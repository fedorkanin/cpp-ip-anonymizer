#include "ClickHouseClientFactory.hpp"

#include <chrono>
#include <iostream>
#include <thread>
#include <tuple>
#include <vector>

const size_t   MAX_RETRIES   = SIZE_MAX;
const unsigned RETRY_TIMEOUT = 2;

std::unique_ptr<clickhouse::Client>
ClickHouseClientFactory::createClickHouseClient(
    const clickhouse::ClientOptions& options) {
    for (size_t i = 0; i < MAX_RETRIES; ++i) {
        std::cout << "Attempting to connect to ClickHouse, options: " << options
                  << " (attempt " << i + 1 << "/" << MAX_RETRIES << ")"
                  << std::endl;
        try {
            auto ptr = std::make_unique<clickhouse::Client>(options);
            return ptr;
        } catch (const std::exception& e) {
            std::cout << "Error connecting to ClickHouse: " << e.what()
                      << std::endl;
            std::this_thread::sleep_for(std::chrono::seconds(RETRY_TIMEOUT));
        }
    }

    return nullptr;
}
