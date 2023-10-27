#include <cstdint>

#include "http_log.capnp.h"

namespace getters {
uint64_t getTimestampEpochMilli(const HttpLogRecord::Reader& log_record) {
    return log_record.getTimestampEpochMilli();
}

uint64_t getResourceId(const HttpLogRecord::Reader& log_record) {
    return log_record.getResourceId();
}

uint64_t getBytesSent(const HttpLogRecord::Reader& log_record) {
    return log_record.getBytesSent();
}

uint64_t getRequestTimeMilli(const HttpLogRecord::Reader& log_record) {
    return log_record.getRequestTimeMilli();
}

uint16_t getResponseStatus(const HttpLogRecord::Reader& log_record) {
    return log_record.getResponseStatus();
}

std::string getCacheStatus(const HttpLogRecord::Reader& log_record) {
    return log_record.hasCacheStatus() ? log_record.getCacheStatus().cStr()
                                       : "";
}

std::string getMethod(const HttpLogRecord::Reader& log_record) {
    return log_record.hasMethod() ? log_record.getMethod().cStr() : "";
}

std::string getRemoteAddr(const HttpLogRecord::Reader& log_record) {
    return log_record.hasRemoteAddr() ? log_record.getRemoteAddr().cStr() : "";
}

std::string getUrl(const HttpLogRecord::Reader& log_record) {
    return log_record.hasUrl() ? log_record.getUrl().cStr() : "";
}
}  // namespace getters
