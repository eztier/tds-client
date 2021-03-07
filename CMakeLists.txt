cmake_minimum_required(VERSION 3.2)

project(tdsclient VERSION 0.1.1 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_BUILD_TYPE RelWithDebInfo)

# find_package(Boost 1.62.0 COMPONENTS filesystem REQUIRED)

# set(SPDLOG ${PROJECT_SOURCE_DIR}/vendor/spdlog-0.14.0/include)

include_directories(
    "${PROJECT_SOURCE_DIR}/include"
    /usr/local/include
    # "${SPDLOG}"
)

link_directories(
  /usr/local/lib
  /usr/local/opt/freetds/lib
)

set(CPP_SOURCES
    "${PROJECT_SOURCE_DIR}/src/tds-client.cpp"
)

add_library(tdsclient SHARED ${CPP_SOURCES})

target_link_libraries(
    tdsclient
    sybdb
)

# install(FILES ${PROJECT_SOURCE_DIR}/include/tds-client.h DESTINATION include)
install(DIRECTORY  ${PROJECT_SOURCE_DIR}/include DESTINATION include
    FILES_MATCHING PATTERN "*.h")
install(TARGETS tdsclient  DESTINATION lib)
