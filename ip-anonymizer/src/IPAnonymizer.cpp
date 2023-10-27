#include "IPAnonymizer.hpp"

#include <capnp/message.h>
#include <capnp/serialize.h>
#include <clickhouse/client.h>

#include <array>
#include <iostream>
#include <sstream>
#include <variant>

#include "http_log.capnp.h"

namespace ch = clickhouse;

IPAnonymizer::IPAnonymizer(cppkafka::Configuration kafka_consumer_config,
                           std::string             clickhouse_host)
    : IPAnonymizer(
          std::make_unique<cppkafka::Consumer>(kafka_consumer_config),
          ClickHouseClientFactory::createClickHouseClient(clickhouse_host)) {}

std::string IPAnonymizer::anonymizeIP(const std::string ip_address) {
    size_t lastDotPos = ip_address.rfind('.');
    if (lastDotPos == std::string::npos) {
        return ip_address;  // Not a valid IP, return as is
    }
    return ip_address.substr(0, lastDotPos) + ".X";
}

using Col64Ptr  = std::shared_ptr<ch::ColumnUInt64>;
using Col16Ptr  = std::shared_ptr<ch::ColumnUInt16>;
using ColStrPtr = std::shared_ptr<ch::ColumnString>;
using ColPtr    = std::shared_ptr<ch::Column>;

using GetterFunc = std::variant<uint64_t    (*)(const HttpLogRecord::Reader&),
                                uint16_t    (*)(const HttpLogRecord::Reader&),
                                std::string (*)(const HttpLogRecord::Reader&)>;

using ReturnType = std::variant<uint64_t, uint16_t, std::string>;
struct ColTripletNew {
    std::string                                             name;
    ColPtr                                                  col_ptr;
    std::function<ReturnType(const HttpLogRecord::Reader&)> getter;
};

#define COL_TRIPLETS_SIZE 9
std::array<ColTripletNew, COL_TRIPLETS_SIZE> getFreshColumnTripletsNew() {
    return {
        ColTripletNew{"timestamp", std::make_shared<ch::ColumnUInt64>(),
                      [](const HttpLogRecord::Reader& log_record) {
                          return log_record.getTimestampEpochMilli();
                      }},
        ColTripletNew{"resource_id", std::make_shared<ch::ColumnUInt64>(),
                      [](const HttpLogRecord::Reader& log_record) {
                          return log_record.getResourceId();
                      }},
        ColTripletNew{"bytes_sent", std::make_shared<ch::ColumnUInt64>(),
                      [](const HttpLogRecord::Reader& log_record) {
                          return log_record.getBytesSent();
                      }},
        ColTripletNew{"request_time_milli",
                      std::make_shared<ch::ColumnUInt64>(),
                      [](const HttpLogRecord::Reader& log_record) {
                          return log_record.getRequestTimeMilli();
                      }},
        ColTripletNew{"response_status", std::make_shared<ch::ColumnUInt16>(),
                      [](const HttpLogRecord::Reader& log_record) {
                          return log_record.getResponseStatus();
                      }},
        ColTripletNew{"cache_status", std::make_shared<ch::ColumnString>(),
                      [](const HttpLogRecord::Reader& log_record) {
                          return log_record.hasCacheStatus()
                                     ? log_record.getCacheStatus().cStr()
                                     : "";
                      }},
        ColTripletNew{"method", std::make_shared<ch::ColumnString>(),
                      [](const HttpLogRecord::Reader& log_record) {
                          return log_record.hasMethod()
                                     ? log_record.getMethod().cStr()
                                     : "";
                      }},
        ColTripletNew{"remote_addr", std::make_shared<ch::ColumnString>(),
                      [](const HttpLogRecord::Reader& log_record) {
                          return log_record.hasRemoteAddr()
                                     ? log_record.getRemoteAddr().cStr()
                                     : "";
                      }},
        ColTripletNew{"url", std::make_shared<ch::ColumnString>(),
                      [](const HttpLogRecord::Reader& log_record) {
                          return log_record.hasUrl()
                                     ? log_record.getUrl().cStr()
                                     : "";
                      }}};
}

void initBlockWithColumns(
    ch::Block&                                   block,
    std::array<ColTripletNew, COL_TRIPLETS_SIZE> col_triplets) {
    for (auto& column : col_triplets)
        block.AppendColumn(column.name, column.col_ptr);
}

// clear columns
template <typename Iter>
void IPAnonymizer::clearColumns(Iter first, Iter last) {
    // use column->clear() to clear the column
    for (auto it = first; it != last; ++it) {
        it->col_ptr->Clear();
    }
}

void createLogEntryInBlock(
    const cppkafka::Buffer&                       payload,
    std::array<ColTripletNew, COL_TRIPLETS_SIZE>& col_triplets) {
    using ColPair = std::pair<std::string, ColPtr>;

    kj::ArrayInputStream array_input_stream(
        {payload.get_data(), payload.get_size()});
    capnp::InputStreamMessageReader message_reader(array_input_stream);
    HttpLogRecord::Reader log_record = message_reader.getRoot<HttpLogRecord>();

    // loop through the columns and add the values to the columns
    for (auto& col_triplet : col_triplets) {
        ReturnType value = col_triplet.getter(log_record);
        std::visit(
            [&](auto&& arg) {
                using T = std::decay_t<decltype(arg)>;
                if constexpr (std::is_same_v<T, uint64_t>) {
                    std::static_pointer_cast<ch::ColumnUInt64>(
                        col_triplet.col_ptr)
                        ->Append(arg);
                } else if constexpr (std::is_same_v<T, uint16_t>) {
                    std::static_pointer_cast<ch::ColumnUInt16>(
                        col_triplet.col_ptr)
                        ->Append(arg);
                } else if constexpr (std::is_same_v<T, std::string>) {
                    std::static_pointer_cast<ch::ColumnString>(
                        col_triplet.col_ptr)
                        ->Append(arg);
                }
            },
            value);
    }
}

void IPAnonymizer::consumeAndBufferLogs(const std::string& topic, int timeout) {
    consumer_->subscribe({topic});
    consumer_->set_timeout(std::chrono::milliseconds(timeout));

    auto col_triplets = getFreshColumnTripletsNew();
    initBlockWithColumns(log_buffer_, col_triplets);

    while (true) {
        cppkafka::Message message = consumer_->poll();
        if (!message) continue;
        if (message.get_error()) {
            std::cerr << "Error while consuming message: "
                      << message.get_error() << std::endl;
            continue;
        }

        createLogEntryInBlock(message.get_payload(), col_triplets);

        if (log_buffer_.GetRowCount() >= MAX_BUFFER_SIZE) {
            ch_client_->Insert("http_logs", log_buffer_);
            log_buffer_ = ch::Block();
            clearColumns(col_triplets.begin(), col_triplets.end());
        }
    }
}
