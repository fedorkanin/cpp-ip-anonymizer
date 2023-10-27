#include "ColumnConfiguration.hpp"

#include <capnp/serialize.h>
#include <kj/io.h>

#include "ColumnBuffer.hpp"

ColumnBuffer::ColumnBuffer(const std::vector<ColTriplet>& configs)
    : columnConfigs_(configs) {}

void ColumnBuffer::initBlockWithColumns(ch::Block& block) {
    for (const auto& config : columnConfigs_) {
        block.AppendColumn(config.name, config.col_ptr);
    }
}

void ColumnBuffer::createLogEntryInBlock(const cppkafka::Buffer& payload) {
    kj::ArrayInputStream array_input_stream(
        {payload.get_data(), payload.get_size()});
    capnp::InputStreamMessageReader message_reader(array_input_stream);
    HttpLogRecord::Reader log_record = message_reader.getRoot<HttpLogRecord>();

    for (auto& config : columnConfigs_) {
        ReturnType value = config.getter(log_record);
        std::visit(
            [&](auto&& arg) {
                using T = std::decay_t<decltype(arg)>;
                if constexpr (std::is_same_v<T, uint64_t>) {
                    std::static_pointer_cast<ch::ColumnUInt64>(config.col_ptr)
                        ->Append(arg);
                } else if constexpr (std::is_same_v<T, uint16_t>) {
                    std::static_pointer_cast<ch::ColumnUInt16>(config.col_ptr)
                        ->Append(arg);
                } else if constexpr (std::is_same_v<T, std::string>) {
                    std::static_pointer_cast<ch::ColumnString>(config.col_ptr)
                        ->Append(arg);
                }
            },
            value);
    }
}

void ColumnBuffer::clearColumns() {
    for (auto& config : columnConfigs_) {
        config.col_ptr->Clear();
    }
}
std::vector<ColTriplet> getColumnConfigurations() {
    return {ColTriplet{"timestamp",
                       [](const HttpLogRecord::Reader& log_record) {
                           return log_record.getTimestampEpochMilli();
                       },
                       std::make_shared<ch::ColumnUInt64>()},
            ColTriplet{"resource_id",
                       [](const HttpLogRecord::Reader& log_record) {
                           return log_record.getResourceId();
                       },
                       std::make_shared<ch::ColumnUInt64>()},
            ColTriplet{"bytes_sent",
                       [](const HttpLogRecord::Reader& log_record) {
                           return log_record.getBytesSent();
                       },
                       std::make_shared<ch::ColumnUInt64>()},
            ColTriplet{"request_time_milli",
                       [](const HttpLogRecord::Reader& log_record) {
                           return log_record.getRequestTimeMilli();
                       },
                       std::make_shared<ch::ColumnUInt64>()},
            ColTriplet{"response_status",
                       [](const HttpLogRecord::Reader& log_record) {
                           return log_record.getResponseStatus();
                       },
                       std::make_shared<ch::ColumnUInt16>()},
            ColTriplet{"cache_status",
                       [](const HttpLogRecord::Reader& log_record) {
                           return log_record.hasCacheStatus()
                                      ? log_record.getCacheStatus().cStr()
                                      : "";
                       },
                       std::make_shared<ch::ColumnString>()},
            ColTriplet{"method",
                       [](const HttpLogRecord::Reader& log_record) {
                           return log_record.hasMethod()
                                      ? log_record.getMethod().cStr()
                                      : "";
                       },
                       std::make_shared<ch::ColumnString>()}};
}
