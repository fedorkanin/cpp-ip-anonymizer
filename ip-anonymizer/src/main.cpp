#include <capnp/message.h>
#include <capnp/serialize.h>
#include <cppkafka/consumer.h>

#include <chrono>
#include <iostream>

#include "IPAnonymizer.hpp"
#include "http_log.capnp.h"

const std::string       KAFKA_BROKER_LIST     = "broker:29092";
const std::string       KAFKA_TOPIC           = "http_log";
const std::string       KAFKA_GROUP_ID        = "ip-anonymizer-reader";
const std::string       CLICKHOUSE_HOST       = "clickhouse-server";
const uint16_t          CLICKHOUSE_PORT       = 9000;
const size_t            CONSUMER_POLL_RATE_MS = 1000;

cppkafka::Configuration kafka_config{
    {"metadata.broker.list", KAFKA_BROKER_LIST},
    {"group.id", KAFKA_GROUP_ID},
};

clickhouse::ClientOptions clickhouse_config;

int                       main() {
    kafka_config.set_default_topic_configuration(
        {{"auto.offset.reset", "smallest"}});
    clickhouse_config.SetHost(CLICKHOUSE_HOST);
    clickhouse_config.SetPort(CLICKHOUSE_PORT);

    IPAnonymizer ipAnonymizer(kafka_config, clickhouse_config);

    ipAnonymizer.consumeAndBufferLogs(KAFKA_TOPIC, CONSUMER_POLL_RATE_MS);
    return 0;
}
