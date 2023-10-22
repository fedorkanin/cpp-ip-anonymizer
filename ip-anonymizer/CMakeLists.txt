cmake_minimum_required(VERSION 3.12)
project(ip-anonymizer)

set(CMAKE_CXX_STANDARD 17)

add_subdirectory(external/clickhouse-cpp)

add_executable(${PROJECT_NAME} "src/main.cpp")

target_include_directories(${PROJECT_NAME} PRIVATE external/clickhouse-cpp/ external/clickhouse-cpp/contrib/absl)

target_link_libraries(${PROJECT_NAME} PRIVATE clickhouse-cpp-lib)