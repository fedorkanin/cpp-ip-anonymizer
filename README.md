# Data Engineering Task

## Task 1 ETL (Anonymizer)

### Requirements

#### Required
* [Docker](https://docs.docker.com/engine/install/)
* [CapNProto](https://capnproto.org/install.html)

#### Recommended
* [Kafka](https://kafka.apache.org/quickstart)
* [clickhouse-client](https://clickhouse.com/docs/en/getting-started/install/)

![](anonymizer.png)

Imagine a logging pipeline of HTTP records, data collected from edge
servers and sent to an Apache Kafka topic called `http_log` as messages. Your task consumes
those messages, performs Cap'N Proto decoding, transforms the data as needed, and inserts them
to a ClickHouse table. The Cap'N Proto schema is
[http_log.capnp](http_log.capnp).


Because of the GDPR regulations you have to anonymize the client IP. For
each record change `remoteAddr`'s last octet to X before sending it over to
ClickHouse (e.g. 1.2.3.`4` -> 1.2.3.`X`).

* Each record must be stored to ClickHouse, even in the event of network or server error. Make sure
that you handle those appropriately.
* Your application should communicate with the ClickHouse server only through the proxy which has
rate limiting for a 1 request per minute limit.
* If there are any limitation about your application write down what they are, and how would you solve them in the future.
  For example
  * What is the latency of the data?
  * Is there any scenario when you will start losing data?
  * Is it possible that there will be a stored duplicate record?
* You can implement the task in any of those languages:
  * Go
  * C/C++
  * Java
  * Rust

### SQL / ClickHouse part

Load those data into ClickHouse, using a new table called `http_log` with the following columns.

```
  timestamp DateTime
  resource_id UInt64
  bytes_sent UInt64
  request_time_milli UInt64
  response_status UInt16
  cache_status LowCardinality(String)
  method LowCardinality(String)
  remote_addr String
  url String
```

Provide a table with ready made totals of served traffic for any combination of resource ID, HTTP status,
cache status and IP address. The totals need to be queried efficiently, in seconds at best, for on-demand
rendering of traffic charts in a front-end such as Grafana.

Characterize the aggregated select query time to show the table architecture is fit for purpose.
Provide an estimate of disk space required given
 1) average incoming message rate
 2) retention of the aggregated data

### Testing environment

You can use included [docker-compose.yml](docker-compose.yml) to setup a local
development environment with every service that you will need for this task.

```bash
$ docker-compose up -d
[+] Running 10/10
 ⠿ Network data-engineering-task_default  Created   0.0s
 ⠿ Container zookeeper                    Started   0.9s
 ⠿ Container prometheus                   Started   0.8s
 ⠿ Container clickhouse                   Started   0.9s
 ⠿ Container ch-proxy                     Started   1.2s
 ⠿ Container broker                       Started   1.3s
 ⠿ Container grafana                      Started   1.2s
 ⠿ Container log-producer                 Started   1.7s
 ⠿ Container kafka-ui                     Started   1.9s
 ⠿ Container jmx-kafka                    Started   1.8s
 ```

After running `docker-compose up -d`, Kafka is available on local port 9092.
You can test it with

```bash
$ kafka-console-consumer --bootstrap-server localhost:9092 \
                         --topic http_log \
                         --group data-engineering-task-reader
```

For testing, a service called `http-log-kafka-producer` is provided, started alongside Kafka.
It produces synthetic Cap'n Proto-encoded records. You can adjust its rate of producing via the
`KAFKA_PRODUCER_DELAY_MS` environment variable inside [docker-compose.yml](./docker-compose.yml)

ClickHouse should be accessed through a proxy running on HTTP port 8124.

For convenience, [Kafka-UI](localhost:4000/) and [Grafana](localhost:3000/) are running
(user `admin` and password `kafka`) that might be useful if you run into any trouble
with Kafka.


# Task completion report

## Research

The initial research was mainly focused on finding and setting up the dependencies of the future project. I chose to complete the project in C++ using the Cap'n Proto, cppkafka, and clickhouse-cpp libraries.
## Environment

The whole project is running with docker compose, so I wanted to maintain the simplicity of launching the project using a single command (via ```docker compose build&&docker compose up -d```). To do that, I created a two-stage Dockerfile, which compiles the projects and it's dependencies, and then passes the compiled binary and shared libraries to the next stage, which runs the executable. 

Developing on Mac with ARM architecture, I faced multiple problems, since some libraries don't compile correctly in ARM docker containers. After tweaking the container for around 8 hours total before and in the process of development, I achieved stable builds with automatic dependency fetching, installation and linking. One of the important features of my Dockerfile is manual caching of main executable build artifacts via ```--mount=type=cache```. It allows for effective recompilation across multiple builds, which is extremely important for development and reduces the time for compilation of the main binary from 30s to around 3s. 

**In the end, my whole code requires no dependencies for building and running, except for Docker.**
## Usage

To run the code, clone the repo with submodules:
`git clone --recurse-submodules https://github.com/fedorkanin/cpp-ip-anonymizer.git`
Then `cd` into cloned repo and run `docker compose build&&docker compose up -d`. 
To see the logs of my module, run `docker compose logs --follow ip-anonymizer`.

## C++ development

### ClickHouse
I began with developing a client wrapper for the cpp-clickhouse client, as I sure would need some specific functionality and automation. I didn't want any proxy restrictions, so I developed a simple client that would connect to the ClickHouse container directly, using the default port 9000. Somehow the cpp-clickhouse client didn't have an option to be lazy and required connection to DB on creation, so I needed to recreate it multiple times, before the DB was up and running, which is not an ideal solution, but it worked. In the end, I achieved a stable connection with ClickHouse and was able to insert or select values and create tables. 

### Kafka client and Cap'n Proto decoding
Configuring the Kafka client was more straightforward than ClickHouse. After successfully receiving raw messages from Kafka broker, I compiled the Cap'n Proto schema into h and cpp files, and successfully decoded fields of the message. 

### Polymorphism of `Columns` and `getters`
At this point I faced a challenge: Cap'n Proto library didn't offer any way to loop through getters of the `HttpLogRecord::Reader`. I always prioritize code maintainability and extensibility, so it should be easily adaptable to new schemas. Moreover, to insert values to ClickHouse client's columns, I required a name and a type of a column. Polymorphism of the `Column` class in clickhouse-cpp is very poor and does not have any polymorphic way to `append`
to an abstract column, except of casting it with a `dynamic-cast`. The same applies to the getters of Cap'n Proto decoder, they are not generalized in any way by default. To handle the problem, I created a following structure:

```cpp
using ReturnType = std::variant<uint64_t, uint16_t, std::string, std::time_t>;
struct ColTriplet {
    std::string                                             name;
    std::function<ReturnType(const HttpLogRecord::Reader&)> getter;
    std::shared_ptr<ch::Column>                             col_ptr;
};
```
This triplet stores everything required to get, cast and append a value from Cap'n Proto message to ClickHouse ```Column```. The config looks like this:
```cpp
std::vector<ColTriplet> getFreshColumns() {
    return {ColTriplet{"timestamp",
                       [](const HttpLogRecord::Reader& log_record) {
                           return static_cast<time_t>(
                               log_record.getTimestampEpochMilli() / 1000);
                       },
                       std::make_shared<ch::ColumnDateTime>()},
            ColTriplet{"resource_id",
                       [](const HttpLogRecord::Reader& log_record) {
                           return log_record.getResourceId();
                       },
                       std::make_shared<ch::ColumnUInt64>()},
                       ...
```

**In the end, I achieved a generalized and flexible solution, that requires minimal changes in the code, in case of Cap'n Proto schema change.** 
The drawback of the solution is that in case of wrong configuration (incorrect column-getter pair), the compiler won't notice the error, which will lead to incorrect cast, and, possibly, to segfault. Fortunately in this case, the error can be discovered right away, when the first message is decoded. In order for the code to be completely type-safe, a non-intrusive double dispatch solution with `std::visitor` is required here, in order to handle `Abstract Value` (`ReturnType`, in my case) and `Abstract Column` interactions (appending `AV` to `AT`, getting `AV` from getters). It is going to be non-intrusive, fast and type-safe, but requires time to code and test.

### Bufferization

The bufferization in my code is pretty straightforward. When receiving Kafka messages, I append their content to a proprietary `ColumnBuffer`, as the default ClickHouse `block` won't allow for an easy management of it's columns. When it's time to insert the data to ClickHouse, I can easily and effectively append to columns to a `block` and insert it via `client->Insert()`. 

### Error handling

In case the insertion is unsuccessful an attempt to insert the buffer is made every 1 second. The data can be lost in case ClickHouse is offline too long and the anonymizer runs out of RAM to store the bufferized data.

### Estimates

Roughly estimating the log record to be $200$ bytes, and the aggregated message to be around $80$ bytes,  the overall disk space occupied will be around $280*N$. The actual number might be lower, as ClickHouse can compress data. 

### DB connection protocols

When I had a mechanism of receiving, decoding, bufferizing and inserting messages, it was time to test it with a proxy restricting my queries to one per minute. Here I found out, that clickhouse-cpp client communicates with ClickHouse via an effective Native protocol, whereas the proxy operates with HTTP requests. There is no embedded functionality in the clickhouse-cpp client to communicate with DB via HTTP, so it has to be implemented manually. The most balanced way of implementing it would be encoding the `ColumnBuffer` rows into Cap'n Proto messages and sending them with HTTP to the ClickHouse, but it requires more time resourses, which I at the moment don't possess, as overall the current solution took me around 10 days (mixed with study, of course).

### Possible improvements

- Implement a retry mechanism with exponential backoff for more efficient and seamless database connections.
- Enhance type safety via double dispatch
- Develop or integrate an HTTP-capable ClickHouse client to cater to both Native and HTTP protocol requirements.
- Run benchmarks to obtain accurate disk space estimates, taking into account ClickHouse's data compression.
- Conduct comprehensive testing, covering functionality, load scenarios, and performance benchmarks.

## Summary

I think my solution is extensible and maintainable, but there are a lot of things to make better, which I attempted to highlight in the report above. Coding it was interesting and I enjoyed it a lot. 
