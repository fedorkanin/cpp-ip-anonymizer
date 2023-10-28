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
                           const clickhouse::ClientOptions& clickhouse_config)
    : IPAnonymizer(
          std::make_unique<cppkafka::Consumer>(kafka_consumer_config),
          ClickHouseClientFactory::createClickHouseClient(clickhouse_config)) {
    if (!ch_client_)
        throw std::runtime_error("Failed to create ClickHouse client");
}

std::string IPAnonymizer::anonymizeIP(const std::string ip_address) {
    size_t lastDotPos = ip_address.rfind('.');
    if (lastDotPos == std::string::npos) {
        return ip_address;  // Not a valid IP, return as is
    }
    return ip_address.substr(0, lastDotPos) + ".X";
}

void IPAnonymizer::consumeAndBufferLogs(const std::string& topic, int timeout) {
    // create a table if it doesn't exist
    ch_client_->Execute(
        "CREATE TABLE IF NOT EXISTS http_logs ("
        "timestamp DateTime,"
        "resource_id UInt64,"
        "bytes_sent UInt64,"
        "request_time_milli UInt64,"
        "response_status UInt16,"
        "cache_status LowCardinality(String),"
        "method LowCardinality(String),"
        "remote_addr String,"
        "url String"
        ") ENGINE = MergeTree() ORDER BY timestamp");

    // create a materialized view if it doesn't exist
    ch_client_->Execute(
        "CREATE MATERIALIZED VIEW IF NOT EXISTS http_log_aggregated "
        "ENGINE = SummingMergeTree() "
        "ORDER BY (resource_id, response_status, cache_status, remote_addr) "
        "AS "
        "SELECT "
        "    resource_id, "
        "    response_status, "
        "    cache_status, "
        "    remote_addr,"
        "    sum(bytes_sent) as total_bytes_sent,"
        "    count() as request_count "
        "FROM http_logs "
        "GROUP BY resource_id, response_status, cache_status, remote_addr");

    consumer_->subscribe({topic});
    consumer_->set_timeout(std::chrono::milliseconds(timeout));

    ColumnBuffer buffer(std::move(getFreshColumns()));
    auto         last_instert_time = std::chrono::system_clock::from_time_t(0);

    while (true) {
        cppkafka::Message message = consumer_->poll();

        if (!message) continue;
        if (message.get_error()) {
            handleMessageError(message.get_error());
            continue;
        }

        std::cout << "Consumed message with payload size: "
                  << message.get_payload().get_size() << std::endl;
        buffer.append(message.get_payload());
        std::cout << "Appended message to buffer. Should insert? "
                  << shouldInsert(last_instert_time) << std::endl;

        if (shouldInsert(last_instert_time)) {
            std::cout << "Attempting insert" << std::endl;
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
        std::cout << "Insert successful" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error while inserting to ClickHouse: " << e.what()
                  << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}
