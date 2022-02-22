cmake_minimum_required(VERSION 3.21)
project(AeronUtils)

set(CMAKE_CXX_STANDARD 20)

find_package(Threads REQUIRED)

# Aeron Client
include_directories(/home/qod/Projects/libs/aeron EXCLUDE_FROM_ALL)
include_directories(/home/qod/Projects/libs/aeron/aeron-client/src/main/cpp)

SET(SOURCE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Subscriber.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Publisher.cpp)

SET(HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Subscriber.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Publisher.h)

add_library(aeron_utils ${SOURCE} ${HEADERS})
add_library(aeron_utils::aeron_utils ALIAS aeron_utils)

add_executable(BasicPublisher ${CMAKE_CURRENT_SOURCE_DIR}/examples/BasicPublisher.cpp)
target_link_libraries(BasicPublisher Threads::Threads aeron_utils::aeron_utils aeron_client)

add_executable(BasicSubscriber ${CMAKE_CURRENT_SOURCE_DIR}/examples/BasicSubscriber.cpp)
target_link_libraries(BasicSubscriber Threads::Threads aeron_utils::aeron_utils aeron_client)
