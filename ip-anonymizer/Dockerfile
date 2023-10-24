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
    # apt-get install -y capnproto

# Clone and build capnproto
RUN git clone --branch v1.0.1 https://github.com/capnproto/capnproto.git && \
    cd capnproto/c++ && \
    autoreconf -i && \
    ./configure && \
    make -j6 check && \
    make install

# Clone and build cppkafka
RUN git clone --branch 0.4.0 https://github.com/mfontanini/cppkafka.git -4 && \
    mkdir cppkafka/build && \
    cd cppkafka/build && \
    cmake .. && \
    make && \
    make install

# Copy the source code and CMakeLists.txt into the container
COPY external /app/external
COPY src /app/src
COPY include /app/include
COPY CMakeLists.txt /app

# Build the application
RUN mkdir build && \
    cd build && \
    cmake .. && \
    make

# Find and copy required shared libraries
RUN mkdir -p /app/libs && \
    ldd /app/build/ip-anonymizer | grep "=> /" | awk '{print $3}' | xargs -I '{}' cp -v '{}' /app/libs


# ====================RUN STAGE====================
FROM debian:bookworm-slim

WORKDIR /app

COPY --from=build /app/libs /usr/local/lib
COPY --from=build /app/build/ip-anonymizer /app

# Reconfigure the dynamic linker run-time bindings
RUN ldconfig

CMD ["./ip-anonymizer"]


