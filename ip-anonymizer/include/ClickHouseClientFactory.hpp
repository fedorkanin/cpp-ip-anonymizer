#pragma once

#include <memory>

#include "clickhouse/client.h"

// the class is required to retry db connection in case of first-time connection
// failure, as default clickhouse::Client is not lazy and tries to connect to db
// on creation
class ClickHouseClientFactory {
   public:
    static std::unique_ptr<clickhouse::Client> createClickHouseClient(
        const clickhouse::ClientOptions& options);
};