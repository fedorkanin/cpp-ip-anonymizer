#pragma once

#include <clickhouse/client.h>
#include <cppkafka/cppkafka.h>

#include <string>
#include <vector>

#include "ClickHouseClientFactory.hpp"

class IPAnonymizer {
   public:
    IPAnonymizer(std::unique_ptr<cppkafka::Consumer>&& consumer,
                 std::unique_ptr<clickhouse::Client>&& chClient)
        : consumer_(std::move(consumer)), ch_client_(std::move(chClient)) {}

    IPAnonymizer(cppkafka::Configuration kafka_consumer_config,
                 std::string             clickhouse_host);

    void consumeAndBufferLogs(const std::string& topic, int timeout);

   private:
    std::unique_ptr<cppkafka::Consumer> consumer_;
    std::unique_ptr<clickhouse::Client> ch_client_;
    clickhouse::Block                   log_buffer_;
    const size_t                        MAX_BUFFER_SIZE = 1;

    template <typename Iter>
    void        clearColumns(Iter first, Iter last);

    std::string anonymizeIP(const std::string ip_address);
};
