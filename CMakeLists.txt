#[[
  mkdir -p ~/freetds & cd ~/freetds & wget ftp://ftp.freetds.org/pub/freetds/stable/freetds-patched.tar.gz
  tar xvzf freetds-patched.tar.gz
  cd freetds-1.00.80 & ./configure & make & sudo make install
]]

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
    /usr/local/include
    "${PROJECT_SOURCE_DIR}/include"
    # "${SPDLOG}"
)

set(CPP_SOURCES
    "${PROJECT_SOURCE_DIR}/src/tds-client.cpp"
)

add_library(tdsclient SHARED ${CPP_SOURCES})

target_link_libraries(
    tdsclient
    libsybdb.so
)

install(FILES ${PROJECT_SOURCE_DIR}/include/tds-client.h DESTINATION include)
install(TARGETS tdsclient  DESTINATION lib)
