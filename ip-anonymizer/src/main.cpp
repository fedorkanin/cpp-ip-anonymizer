#include <clickhouse/client.h>

#include <chrono>
#include <iostream>
#include <thread>

using namespace clickhouse;

std::unique_ptr<Client> initializeClickHouseConnection() {
    const int max_retries = 10;
    for (int i = 0; i < max_retries; ++i) {
        try {
            auto client = std::make_unique<Client>(
                ClientOptions().SetHost("clickhouse-server"));
            client->Execute(
                "SELECT 1");  // Simple query to check the connection
            return client;
        } catch (const std::exception& e) {
            std::cerr << "Failed to connect, retrying in 5 seconds... ("
                      << i + 1 << "/" << max_retries << ")\n";
            std::this_thread::sleep_for(std::chrono::seconds(5));
        }
    }

    return nullptr;
}

int testClickhouse() {
    auto client_ptr = initializeClickHouseConnection();
    if (!client_ptr) {
        std::cerr
            << "Failed to connect to ClickHouse after multiple retries.\n";
        return 1;  // Return error code
    }
    Client& client = *client_ptr;

    /// Create a table.
    client.Execute(
        "CREATE TABLE IF NOT EXISTS default.numbers (id UInt64, name String) "
        "ENGINE = Memory");

    /// Insert some values.
    {
        Block block;

        auto  id = std::make_shared<ColumnUInt64>();
        id->Append(1);
        id->Append(7);

        auto name = std::make_shared<ColumnString>();
        name->Append("one");
        name->Append("seven");

        block.AppendColumn("id", id);
        block.AppendColumn("name", name);

        client.Insert("default.numbers", block);
    }

    /// Select values inserted in the previous step.
    client.Select(
        "SELECT id, name FROM default.numbers", [](const Block& block) {
            for (size_t i = 0; i < block.GetRowCount(); ++i) {
                std::cout << block[0]->As<ColumnUInt64>()->At(i) << " "
                          << block[1]->As<ColumnString>()->At(i) << "\n";
            }
        });

    /// Delete table.
    client.Execute("DROP TABLE default.numbers");

    return 0;
}

int main() {
    if (testClickhouse()) {
        std::cout << "Clickhouse connection test failed\n";
    } else {
        std::cout << "Clickhouse connection test passed\n";
    }
    return 0;
}
