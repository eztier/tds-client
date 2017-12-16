cmake_minimum_required(VERSION 3.2)

project(testing VERSION 0.0.1 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_CXX_FLAGS "-Wall -v")

set(SPDLOG ${PROJECT_SOURCE_DIR}/../vendor/spdlog-0.14.0/include)

include_directories(
    /usr/local/include
    "${PROJECT_SOURCE_DIR}/../include"
    "${SPDLOG}"
)

link_directories(
  /usr/local/lib
)

set(CPP_SOURCES
    "${PROJECT_SOURCE_DIR}/index.cpp"
)

add_executable(testing ${CPP_SOURCES})

target_link_libraries(
    testing
    libsybdb.so
    "${PROJECT_SOURCE_DIR}/../build/lib/libtdsclient.so"
)
