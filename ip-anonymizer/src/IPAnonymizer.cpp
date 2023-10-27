#include "IPAnonymizer.hpp"

#include <capnp/message.h>
#include <capnp/serialize.h>
#include <clickhouse/client.h>

#include <array>
#include <iostream>
#include <sstream>
#include <variant>

#include "Getters.hpp"
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
// a triplet representing the name of the column, a getter function for
// CapnProto message associated with the column, and a pointer to the column
struct ColumnTriplet {
    std::string name;
    GetterFunc  getter;
    ColPtr      col_ptr;
};

#define COL_TRIPLETS_SIZE 9
std::array<ColumnTriplet, COL_TRIPLETS_SIZE> getFreshColumnTriplets() {
    return {ColumnTriplet{"timestamp", getters::getTimestampEpochMilli,
                          std::make_shared<ch::ColumnUInt64>()},
            ColumnTriplet{"resource_id", getters::getResourceId,
                          std::make_shared<ch::ColumnUInt64>()},
            ColumnTriplet{"bytes_sent", getters::getBytesSent,
                          std::make_shared<ch::ColumnUInt64>()},
            ColumnTriplet{"request_time_milli", getters::getRequestTimeMilli,
                          std::make_shared<ch::ColumnUInt64>()},
            ColumnTriplet{"response_status", getters::getResponseStatus,
                          std::make_shared<ch::ColumnUInt16>()},
            ColumnTriplet{"cache_status", getters::getCacheStatus,
                          std::make_shared<ch::ColumnString>()},
            ColumnTriplet{"method", getters::getMethod,
                          std::make_shared<ch::ColumnString>()},
            ColumnTriplet{"remote_addr", getters::getRemoteAddr,
                          std::make_shared<ch::ColumnString>()},
            ColumnTriplet{"url", getters::getUrl,
                          std::make_shared<ch::ColumnString>()}};
}

void initBlockWithColumns(
    ch::Block&                                   block,
    std::array<ColumnTriplet, COL_TRIPLETS_SIZE> col_triplets) {
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
    std::array<ColumnTriplet, COL_TRIPLETS_SIZE>& col_triplets) {
    using ColPair = std::pair<std::string, ColPtr>;

    kj::ArrayInputStream array_input_stream(
        {payload.get_data(), payload.get_size()});
    capnp::InputStreamMessageReader message_reader(array_input_stream);
    HttpLogRecord::Reader log_record = message_reader.getRoot<HttpLogRecord>();

    // loop through the columns and add the values to the columns
    for (auto& col_triplet : col_triplets) {
        std::visit(
            [&](auto&& getter) {
                using T  = std::decay_t<decltype(getter)>;
                auto val = getter(log_record);
                if constexpr (std::is_same_v<
                                  T,
                                  uint64_t (*)(const HttpLogRecord::Reader&)>) {
                    std::static_pointer_cast<ch::ColumnUInt64>(
                        col_triplet.col_ptr)
                        ->Append(val);
                } else if constexpr (std::is_same_v<
                                         T,
                                         uint16_t (*)(
                                             const HttpLogRecord::Reader&)>) {
                    std::static_pointer_cast<ch::ColumnUInt16>(
                        col_triplet.col_ptr)
                        ->Append(val);
                } else if constexpr (std::is_same_v<
                                         T,
                                         std::string (*)(
                                             const HttpLogRecord::Reader&)>) {
                    std::static_pointer_cast<ch::ColumnString>(
                        col_triplet.col_ptr)
                        ->Append(val);
                }
            },
            col_triplet.getter);
    }
}

void IPAnonymizer::consumeAndBufferLogs(const std::string& topic, int timeout) {
    consumer_->subscribe({topic});
    consumer_->set_timeout(std::chrono::milliseconds(timeout));

    auto col_triplets = getFreshColumnTriplets();
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
