#include <chrono>
#include <iostream>

#include "ClickHouseClientFactory.hpp"
#include "cppkafka/consumer.h"

using namespace std;
using namespace cppkafka;

// Configuration keys
const string KAFKA_BROKER_LIST = "localhost:9092";
const string KAFKA_TOPIC       = "http_log";
const string KAFKA_GROUP_ID    = "data-engineering-task-reader";

std::unique_ptr<cppkafka::Consumer> setupConsumer(
    const cppkafka::Configuration& config) {
    return std::make_unique<cppkafka::Consumer>(config);
}

void consumeAndPrint(cppkafka::Consumer& consumer, const std::string& topic,
                     int timeout) {
    consumer.subscribe({topic});

    // Set the timeout value
    consumer.set_timeout(chrono::milliseconds(timeout));

    while (true) {
        // Poll for messages
        cppkafka::Message message = consumer.poll();
        if (message) {
            // Check for errors
            if (message.get_error()) {
                std::cerr << "Error while consuming message: "
                          << message.get_error() << std::endl;
            } else {
                // Print the consumed message's payload
                std::cout << "Received message: " << message.get_payload()
                          << std::endl;
            }
        }
    }
}

int main() {
    assert(ClickHouseClientFactory::testClickhouse() == 0);
    std::cout << "ClickHouse test passed\n";

    // Kafka configuration
    cppkafka::Configuration config = {
        {"metadata.broker.list",
         "localhost:9092"},             // Replace with your broker list
        {"group.id", "consumer_group"}  // Consumer group ID
    };

    // Setup the consumer
    std::unique_ptr<cppkafka::Consumer> consumer = setupConsumer(config);

    // Consume messages from the "http_log" topic with a 1 second timeout
    std::cout << "Consuming messages from topic "
              << "http_log" << std::endl;
    consumeAndPrint(*consumer, "http_log", 1000);

    return 0;
}
