#pragma once

#include <clickhouse/client.h>

#include <functional>
#include <memory>
#include <string>
#include <variant>

#include "http_log.capnp.h"

namespace ch = clickhouse;

using ReturnType = std::variant<uint64_t, uint16_t, std::string>;
using Col64Ptr   = std::shared_ptr<ch::ColumnUInt64>;
using Col16Ptr   = std::shared_ptr<ch::ColumnUInt16>;
using ColStrPtr  = std::shared_ptr<ch::ColumnString>;
using ColPtr     = std::shared_ptr<ch::Column>;

struct ColTriplet {
    std::string                                             name;
    std::function<ReturnType(const HttpLogRecord::Reader&)> getter;
    std::shared_ptr<ch::Column>                             col_ptr;
};

std::vector<ColTriplet> getColumnConfigurations();