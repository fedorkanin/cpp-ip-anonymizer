#include <capnp/message.h>
#include <capnp/serialize.h>
#include <cppkafka/consumer.h>

#include <chrono>
#include <iostream>

#include "IPAnonymizer.hpp"
#include "http_log.capnp.h"

const std::string KAFKA_BROKER_LIST = "broker:29092";
const std::string KAFKA_TOPIC       = "http_log";
const std::string KAFKA_GROUP_ID    = "ip-anonymizer-reader";

const std::string CLICKHOUSE_HOST = "clickhouse-server";

int               main() {
    cppkafka::Configuration kafka_config{
                      {"metadata.broker.list", KAFKA_BROKER_LIST},
                      {"group.id", KAFKA_GROUP_ID},
    };
    kafka_config.set_default_topic_configuration(
        {{"auto.offset.reset", "smallest"}});

    IPAnonymizer ipAnonymizer(kafka_config, CLICKHOUSE_HOST);
    ipAnonymizer.consumeAndBufferLogs(KAFKA_TOPIC, 1000);
}
