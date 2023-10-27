#include "IPAnonymizer.hpp"

#include <capnp/message.h>
#include <capnp/serialize.h>
#include <clickhouse/client.h>

#include <array>
#include <iostream>
#include <sstream>
#include <variant>

#include "ColumnBuffer.hpp"
#include "ColumnConfiguration.hpp"
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

void initBlockWithColumns(ch::Block&                     block,
                          const std::vector<ColTriplet>& col_triplets) {
    for (auto& column : col_triplets)
        block.AppendColumn(column.name, column.col_ptr);
}

template <typename Iter>
void IPAnonymizer::clearColumns(Iter first, Iter last) {
    for (auto it = first; it != last; ++it) {
        it->col_ptr->Clear();
    }
}

void createLogEntryInBlock(const cppkafka::Buffer&  payload,
                           std::vector<ColTriplet>& col_triplets) {
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

    ColumnBuffer buffer(getColumnConfigurations());
    buffer.initBlockWithColumns(log_buffer_);

    while (true) {
        cppkafka::Message message = consumer_->poll();
        if (!message) continue;
        if (message.get_error()) {
            std::cerr << "Error while consuming message: "
                      << message.get_error() << std::endl;
            continue;
        }
        buffer.createLogEntryInBlock(message.get_payload());

        if (log_buffer_.GetRowCount() >= MAX_BUFFER_SIZE) {
            ch_client_->Insert("http_logs", log_buffer_);
            log_buffer_ = ch::Block();
            buffer.clearColumns();
        }
    }
}
