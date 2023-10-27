#include "IPAnonymizer.hpp"

#include <capnp/message.h>
#include <capnp/serialize.h>
#include <clickhouse/client.h>

#include <array>
#include <iostream>
#include <sstream>
#include <variant>

#include "ColumnBuffer.hpp"
#include "ColumnConfiguration.hpp"
#include "http_log.capnp.h"

namespace ch = clickhouse;

IPAnonymizer::IPAnonymizer(cppkafka::Configuration kafka_consumer_config,
                           std::string             clickhouse_host)
    : IPAnonymizer(
          std::make_unique<cppkafka::Consumer>(kafka_consumer_config),
          ClickHouseClientFactory::createClickHouseClient(clickhouse_host)) {}

std::string IPAnonymizer::anonymizeIP(const std::string ip_address) {
    size_t lastDotPos = ip_address.rfind('.');
    if (lastDotPos == std::string::npos) {
        return ip_address;  // Not a valid IP, return as is
    }
    return ip_address.substr(0, lastDotPos) + ".X";
}

void IPAnonymizer::consumeAndBufferLogs(const std::string& topic, int timeout) {
    consumer_->subscribe({topic});
    consumer_->set_timeout(std::chrono::milliseconds(timeout));

    ColumnBuffer buffer(std::move(getFreshColumns()));
    auto         last_instert_time = std::chrono::system_clock::now();

    while (true) {
        cppkafka::Message message = consumer_->poll();

        if (!message) continue;
        if (message.get_error()) {
            handleMessageError(message.get_error());
            continue;
        }

        buffer.append(message.get_payload());

        if (shouldInsert(last_instert_time)) {
            attemptInsert(buffer, last_instert_time);
        }
    }
}

void IPAnonymizer::handleMessageError(const cppkafka::Error& error) {
    std::cerr << "Error while consuming message: " << error << std::endl;
}

bool IPAnonymizer::shouldInsert(
    const std::chrono::system_clock::time_point& last_insert_time) const {
    using namespace std::chrono;
    auto currentTime = system_clock::now();
    return duration_cast<seconds>(currentTime - last_insert_time).count() > 60;
}

void IPAnonymizer::attemptInsert(
    ColumnBuffer&                          buffer,
    std::chrono::system_clock::time_point& lastInsertTime) {
    try {
        ch_client_->Insert("http_logs", buffer.exportToBlockShallow());
        buffer.clearColumns();
        lastInsertTime = std::chrono::system_clock::now();
    } catch (const std::exception& e) {
        std::cerr << "Error while inserting to ClickHouse: " << e.what()
                  << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}
