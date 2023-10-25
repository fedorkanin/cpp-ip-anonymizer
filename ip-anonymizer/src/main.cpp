#include <capnp/message.h>
#include <capnp/serialize.h>
#include <cppkafka/consumer.h>

#include <chrono>
#include <iostream>

#include "ClickHouseClientFactory.hpp"
#include "http_log.capnp.h"

const std::string                   KAFKA_BROKER_LIST = "broker:29092";
const std::string                   KAFKA_TOPIC       = "http_log";
const std::string                   KAFKA_GROUP_ID    = "ip-anonymizer-reader";

std::unique_ptr<cppkafka::Consumer> SetupConsumer(
    const cppkafka::Configuration& config) {
    return std::make_unique<cppkafka::Consumer>(config);
}

std::string DecodeMessage(const cppkafka::Buffer& payload) {
    kj::ArrayInputStream input_stream(
        kj::arrayPtr(payload.get_data(), payload.get_size()));

    capnp::InputStreamMessageReader message_reader(input_stream);

    HttpLogRecord::Reader log_reader = message_reader.getRoot<HttpLogRecord>();

    // Extract data from the log here
    // For brevity, let's just get remoteAddr (more fields can be added
    // similarly)
    return log_reader.getRemoteAddr();
}

void ConsumeAndPrint(cppkafka::Consumer& consumer, const std::string& topic,
                     int timeout) {
    consumer.subscribe({topic});
    consumer.set_timeout(std::chrono::milliseconds(timeout));

    while (true) {
        cppkafka::Message message = consumer.poll();
        if (message) {
            if (message.get_error()) {
                std::cerr << "Error while consuming message: "
                          << message.get_error() << std::endl;
            } else {
                std::string decoded_message =
                    DecodeMessage(message.get_payload());
                std::cout << "Decoded message: " << decoded_message
                          << std::endl;
            }
        }
    }
}

int main() {
    cppkafka::Configuration config = {
        {"metadata.broker.list", KAFKA_BROKER_LIST},
        {"group.id", KAFKA_GROUP_ID},
    };

    cppkafka::TopicConfiguration topic_config = {
        {"auto.offset.reset", "smallest"}};
    config.set_default_topic_configuration(topic_config);

    std::unique_ptr<cppkafka::Consumer> consumer = SetupConsumer(config);

    std::cout << "Consuming messages from topic " << KAFKA_TOPIC << std::endl;
    ConsumeAndPrint(*consumer, KAFKA_TOPIC, 1000);

    return 0;
}
