cmake_minimum_required(VERSION 3.10)

project(anonymizer)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Include directories
include_directories(${PROJECT_SOURCE_DIR}/include)

# Find all source files
file(GLOB SOURCES "src/*.cpp")

# Output directory for binaries
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)

# Create executable
add_executable(anonymizer ${SOURCES})

# Link libraries
target_link_libraries(anonymizer cppkafka clickhouse-cpp-client spdlog boost_system)
