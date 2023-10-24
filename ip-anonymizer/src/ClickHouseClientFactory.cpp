#include "ClickHouseClientFactory.hpp"

#include <chrono>
#include <iostream>
#include <thread>
#include <tuple>
#include <vector>

const unsigned    MAX_RETRIES   = 10;
const unsigned    RETRY_TIMEOUT = 2;
const std::string DEFAULT_HOST  = "clickhouse-server";
using DataRow                   = std::tuple<uint64_t, std::string>;

std::unique_ptr<clickhouse::Client>
ClickHouseClientFactory::createClickHouseClient(const std::string& host) {
    for (int i = 0; i < MAX_RETRIES; ++i) {
        try {
            auto ptr = std::make_unique<clickhouse::Client>(
                clickhouse::ClientOptions().SetHost(host));
            return ptr;
        } catch (const std::exception& e) {
            std::cout << "Failed to connect, retrying in " << RETRY_TIMEOUT
                      << " seconds (attempt " << i + 1 << "/" << MAX_RETRIES
                      << ")\n";
            std::this_thread::sleep_for(std::chrono::seconds(RETRY_TIMEOUT));
        }
    }

    return nullptr;
}

int ClickHouseClientFactory::testClickhouse() {
    auto client_ptr = createClickHouseClient(DEFAULT_HOST);
    if (!client_ptr) {
        std::cout << "Failed to create ClickHouse client\n";
        return 1;
    }

    clickhouse::Client&  client = *client_ptr;

    std::vector<DataRow> inserted_values = {{1, "one"}, {7, "seven"}};
    createAndPopulateTable(client, inserted_values);

    bool check_passed = checkValues(client, inserted_values);

    client.Execute("DROP TABLE default.numbers");

    return !check_passed;
}

void ClickHouseClientFactory::createAndPopulateTable(
    clickhouse::Client& client, const std::vector<DataRow>& values) {
    client.Execute(
        "CREATE TABLE IF NOT EXISTS default.numbers (id UInt64, name String) "
        "ENGINE = Memory");

    clickhouse::Block block;

    auto              id   = std::make_shared<clickhouse::ColumnUInt64>();
    auto              name = std::make_shared<clickhouse::ColumnString>();

    for (const auto& [id_value, name_value] : values) {
        id->Append(id_value);
        name->Append(name_value);
    }

    block.AppendColumn("id", id);
    block.AppendColumn("name", name);

    client.Insert("default.numbers", block);
}

bool ClickHouseClientFactory::checkValues(clickhouse::Client&         client,
                                          const std::vector<DataRow>& values) {
    bool all_values_match = true;
    client.Select("SELECT id, name FROM default.numbers",
                  [&](const clickhouse::Block& block) {
                      for (size_t i = 0; i < block.GetRowCount(); ++i) {
                          uint64_t returned_id =
                              block[0]->As<clickhouse::ColumnUInt64>()->At(i);
                          std::string returned_name = std::string(
                              block[1]->As<clickhouse::ColumnString>()->At(i));

                          if (returned_id != std::get<0>(values[i]) ||
                              returned_name != std::get<1>(values[i])) {
                              all_values_match = false;
                              std::cout << "FAILED: Expected id: "
                                        << std::get<0>(values[i])
                                        << ", name: " << std::get<1>(values[i])
                                        << ". Got id: " << returned_id
                                        << ", name: " << returned_name
                                        << std::endl;
                          }
                      }
                  });

    return all_values_match;
}
