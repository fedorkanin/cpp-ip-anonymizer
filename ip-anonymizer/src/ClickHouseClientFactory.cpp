#include "ClickHouseClientFactory.hpp"

#include <chrono>
#include <iostream>
#include <thread>

const unsigned    MAX_RETRIES   = 10;
const unsigned    RETRY_TIMEOUT = 2;
const std::string DEFAULT_HOST  = "clickhouse-server";

std::unique_ptr<clickhouse::Client>
ClickHouseClientFactory::createClickHouseClient(const std::string& host) {
    std::cout << "Creating ClickHouse client" << std::endl;
    for (int i = 0; i < MAX_RETRIES; ++i) {
        try {
            std::cout << "Trying to create pointer to ClickHouse client"
                      << std::endl;
            auto ptr = std::make_unique<clickhouse::Client>(
                clickhouse::ClientOptions().SetHost(host));
            std::cout << "Successfully created pointer to ClickHouse client"
                      << std::endl;
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

    clickhouse::Client& client = *client_ptr;

    std::cout << "Creating table" << std::endl;

    /// Create a table.
    client.Execute(
        "CREATE TABLE IF NOT EXISTS default.numbers (id UInt64, name "
        "String) ENGINE = Memory");

    std::cout << "Inserting values" << std::endl;
    /// Insert some values.
    {
        clickhouse::Block block;

        std::cout << "Creating columns" << std::endl;
        auto id = std::make_shared<clickhouse::ColumnUInt64>();
        id->Append(1);
        id->Append(7);

        std::cout << "Creating columns 2" << std::endl;
        auto name = std::make_shared<clickhouse::ColumnString>();
        name->Append("one");
        name->Append("seven");

        std::cout << "Appending columns" << std::endl;
        block.AppendColumn("id", id);
        block.AppendColumn("name", name);

        std::cout << "Inserting block" << std::endl;
        client.Insert("default.numbers", block);
    }

    std::cout << "Selecting values" << std::endl;
    /// Select values inserted in the previous step.
    client.Select("SELECT id, name FROM default.numbers",
                  [](const clickhouse::Block& block) {
                      for (size_t i = 0; i < block.GetRowCount(); ++i) {
                          std::cout
                              << block[0]->As<clickhouse::ColumnUInt64>()->At(
                                     i)  // NOTE: Corrected type here
                              << " "
                              << block[1]->As<clickhouse::ColumnString>()->At(i)
                              << "\n";
                      }
                  });

    std::cout << "Dropping table" << std::endl;
    /// Delete table.
    client.Execute("DROP TABLE default.numbers");

    std::cout << "Test passed, returning 0" << std::endl;
    return 0;
}
