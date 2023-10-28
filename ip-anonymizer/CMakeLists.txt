cmake_minimum_required(VERSION 3.12)
project(ip-anonymizer)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -fsanitize=address")

# Gather all .cpp files from the src directory
file(GLOB SRC_FILES src/*.cpp)

add_subdirectory(external/clickhouse-cpp)

# Add SRC_FILES to your executable
add_executable(${PROJECT_NAME} ${SRC_FILES})

target_include_directories(${PROJECT_NAME} 
    PRIVATE 
        external/clickhouse-cpp/ 
        external/clickhouse-cpp/contrib/absl 
        include/
)

find_package(CppKafka REQUIRED)

target_link_libraries(${PROJECT_NAME} 
    PRIVATE 
        CppKafka::cppkafka
        clickhouse-cpp-lib
        capnp  
        kj
        #capnp-rpc  # Uncomment for Cap'n Proto's RPC features
)
