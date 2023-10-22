FROM debian:bookworm-slim as build

WORKDIR /app

RUN apt-get update && \
    apt-get install -y apt-utils && \
    apt-get install -y build-essential && \
    apt-get install -y git && \
    apt-get install -y cmake && \
    apt-get install -y libboost-all-dev && \
    apt-get install -y librdkafka-dev && \
    apt-get install -y libssl-dev

# Clone and build cppkafka
RUN git clone --branch 0.4.0 https://github.com/mfontanini/cppkafka.git -4 && \
    mkdir cppkafka/build && \
    cd cppkafka/build && \
    cmake .. && \
    make && \
    make install

# Copy the source code and CMakeLists.txt into the container
COPY src /app/src
COPY include /app/include
COPY external /app/external
COPY CMakeLists.txt /app

# Build the application
RUN mkdir build && \
    cd build && \
    cmake .. && \
    make

# Run Stage
FROM debian:bookworm-slim

WORKDIR /app

# Copy the compiled binary from the build stage
COPY --from=build /app/build/ip-anonymizer /app

CMD ["./ip-anonymizer"]