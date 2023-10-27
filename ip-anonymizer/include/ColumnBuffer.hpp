#pragma once

#include <clickhouse/client.h>
#include <cppkafka/buffer.h>

#include "ColumnConfiguration.hpp"

namespace ch = clickhouse;

class ColumnBuffer {
   public:
    ColumnBuffer(const std::vector<ColTriplet>& configs);
    void initBlockWithColumns(ch::Block& block);
    void createLogEntryInBlock(const cppkafka::Buffer& payload);
    void clearColumns();

   private:
    std::vector<ColTriplet> columnConfigs_;
};
