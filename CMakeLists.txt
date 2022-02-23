cmake_minimum_required(VERSION 3.15)
project(kucoin_gateway)

set(CMAKE_CXX_STANDARD 20)

set(LIB_AERON /home/qod/Projects/libs/aeron)
set(KUCOIN_API_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/KucoinAPI)


find_package(Boost 1.78.0 REQUIRED COMPONENTS log)
find_package(Threads REQUIRED)
find_package(OpenSSL REQUIRED)

include_directories(${LIB_AERON}/aeron-client/src/main/cpp)

#include(libs/aeron_cpp/CMakeLists.txt)

add_library(
        json_parser
        src/json_parser/JsonParser.h
        src/json_parser/JsonParser.cpp
        src/json_parser/Exceptions.h
)

add_library(
        kucoin_api
        ${KUCOIN_API_DIR}/KucoinAPI.h
        ${KUCOIN_API_DIR}/KucoinAPI.cpp

        ${KUCOIN_API_DIR}/parsers.cpp
        ${KUCOIN_API_DIR}/parsers.h

        ${KUCOIN_API_DIR}/src/KucoinDataclasses.h
        ${KUCOIN_API_DIR}/src/KucoinDataclasses.cpp
        ${KUCOIN_API_DIR}/src/KucoinHTTP.cpp
        ${KUCOIN_API_DIR}/src/KucoinHTTP.h
        ${KUCOIN_API_DIR}/src/KucoinWS.cpp
        ${KUCOIN_API_DIR}/src/KucoinWS.h
)

add_executable(
        ${PROJECT_NAME}

        main.cpp

        libs/aeron_cpp/src/Publisher.cpp
        libs/aeron_cpp/src/Publisher.h
        libs/aeron_cpp/src/Subscriber.cpp
        libs/aeron_cpp/src/Subscriber.h

        libs/simdjson/simdjson.h
        libs/simdjson/simdjson.cpp

        src/WSSession.cpp
        src/WSSession.h
        src/HTTPSession.cpp
        src/HTTPSession.h

        src/KucoinAPI/src/KucoinWS.cpp
        src/KucoinAPI/src/KucoinWS.h
        src/KucoinAPI/src/KucoinHTTP.cpp
        src/KucoinAPI/src/KucoinHTTP.h

        src/utils.cpp
        src/utils.h
        src/KucoinAPI/src/KucoinDataclasses.cpp
        src/KucoinAPI/src/KucoinDataclasses.h

        src/config/gate_config.h
        src/config/aeron_channel.h
        src/config/core_config.h
        src/KucoinAPI/KucoinAPI.cpp src/KucoinAPI/KucoinAPI.h)

add_executable(
        temp

        src/temp.cpp
        src/KucoinAPI/src/KucoinDataclasses.cpp
        src/KucoinAPI/src/KucoinDataclasses.h
        libs/simdjson/simdjson.h
        libs/simdjson/simdjson.cpp

        src/json_parser/JsonParser.cpp src/json_parser/JsonParser.h src/json_parser/Exceptions.h)

target_link_libraries(
        ${PROJECT_NAME}

        json_parser
        kucoin_api
        Threads::Threads
        OpenSSL::SSL
        Boost::log
        ${LIB_AERON}/cppbuild/Release/lib/libaeron_client.a
)
target_link_libraries(
        temp

        json_parser
        kucoin_api
)
