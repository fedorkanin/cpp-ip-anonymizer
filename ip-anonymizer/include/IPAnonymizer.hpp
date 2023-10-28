#pragma once

#include <clickhouse/client.h>
#include <cppkafka/cppkafka.h>

#include <string>
#include <vector>

#include "ClickHouseClientFactory.hpp"
#include "ColumnBuffer.hpp"

class IPAnonymizer {
   public:
    IPAnonymizer(cppkafka::Configuration          kafka_consumer_config,
                 const clickhouse::ClientOptions& clickhouse_config);

    void consumeAndBufferLogs(const std::string& topic, int timeout);

   private:
    IPAnonymizer(std::unique_ptr<cppkafka::Consumer>&& consumer,
                 std::unique_ptr<clickhouse::Client>&& chClient)
        : consumer_(std::move(consumer)), ch_client_(std::move(chClient)) {}

    std::unique_ptr<cppkafka::Consumer> consumer_;
    std::unique_ptr<clickhouse::Client> ch_client_;

    std::string anonymizeIP(const std::string ip_address);
    void        handleMessageError(const cppkafka::Error& error);
    bool        shouldInsert(
               const std::chrono::system_clock::time_point& lastInsertTime) const;
    void attemptInsert(ColumnBuffer&                          buffer,
                       std::chrono::system_clock::time_point& lastInsertTime);
};
