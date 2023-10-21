# Build Stage
FROM debian:bookworm-slim as build

WORKDIR /app

RUN apt-get update 
RUN apt-get install -y apt-utils
RUN apt-get install -y build-essential
RUN apt-get install -y git
RUN apt-get install -y cmake

RUN apt-get install -y gfortran-12 libgfortran-12-dev libgfortran5
RUN apt-get install -y gfortran

RUN apt-get install -y libcoarrays-dev
RUN apt-get install -y libcoarrays-openmpi-dev

RUN apt-get install -y libboost-all-dev
RUN apt-get install -y librdkafka-dev
RUN apt-get install -y libssl-dev 

# Clone and build cppkafka
RUN git clone --branch 0.4.0 https://github.com/mfontanini/cppkafka.git && \
    mkdir cppkafka/build && \
    cd cppkafka/build && \
    cmake .. && \
    make && \
    make install

# Clone and build clickhouse-cpp
RUN git clone --branch v2.5.1 https://github.com/ClickHouse/clickhouse-cpp.git && \
    mkdir clickhouse-cpp/build && \
    cd clickhouse-cpp/build && \
    cmake .. && \
    make && \
    make install

# Copy the source code and CMakeLists.txt into the container
COPY src /app/src
COPY include /app/include
COPY CMakeLists.txt /app

# Build the application
RUN cmake . && make

# Run Stage
FROM alpine:latest

WORKDIR /app

# Copy the compiled binary from the build stage
COPY --from=build /app/bin/anonymizer /app

ENTRYPOINT ["./anonymizer"]
