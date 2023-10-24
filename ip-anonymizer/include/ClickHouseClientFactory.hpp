#pragma once

#include <memory>

#include "clickhouse/client.h"

// the class is required to retry db connection in case of first-time connection
// failure, as default clickhouse::Client is not lazy and tries to connect to db
// on creation
class ClickHouseClientFactory {
   public:
    static int                                 testClickhouse();

    static std::unique_ptr<clickhouse::Client> createClickHouseClient(
        const std::string& host);

   private:
    using DataRow = std::tuple<uint64_t, std::string>;
    static void createAndPopulateTable(clickhouse::Client&         client,
                                       const std::vector<DataRow>& values);
    static bool checkValues(clickhouse::Client&         client,
                            const std::vector<DataRow>& values);
};