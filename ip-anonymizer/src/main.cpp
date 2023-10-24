#include <iostream>

#include "ClickHouseClientFactory.hpp"

int main() {
    if (ClickHouseClientFactory::testClickhouse()) {
        std::cout << "FAILED: Clickhouse connection test failed\n";
    } else {
        std::cout << "PASSED: Clickhouse connection test passed\n";
    }

    return 0;
}
