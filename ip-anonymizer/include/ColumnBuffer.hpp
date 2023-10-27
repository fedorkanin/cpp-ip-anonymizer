#pragma once

#include <clickhouse/client.h>
#include <cppkafka/buffer.h>

#include "ColumnConfiguration.hpp"

namespace ch = clickhouse;

class ColumnBuffer {
   public:
    ColumnBuffer(const std::vector<ColTriplet> configs)
        : col_triplet_vec_(configs) {}
    ch::Block     exportToBlockShallow();
    void          append(const cppkafka::Buffer& payload);
    void          clearColumns();
    inline size_t getRowCount() {
        return col_triplet_vec_.empty() ? 0
                                        : col_triplet_vec_[0].col_ptr->Size();
    }

   private:
    std::vector<ColTriplet> col_triplet_vec_;
};
