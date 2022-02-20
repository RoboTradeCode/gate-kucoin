cmake_minimum_required(VERSION 3.15)
project(kucoin_gateway)

set(CMAKE_CXX_STANDARD 20)

set(LIB_AERON /home/qod/Projects/libs/aeron)


find_package(Boost 1.78.0 REQUIRED COMPONENTS log)
find_package(Threads REQUIRED)
find_package(OpenSSL REQUIRED)

include_directories(${LIB_AERON}/aeron-client/src/main/cpp)

#include(libs/aeron_cpp/CMakeLists.txt)

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

        src/KucoinWS.cpp
        src/KucoinWS.h
        src/KucoinREST.cpp
        src/KucoinREST.h

        src/utils.cpp
        src/utils.h
        src/KucoinDataclasses.cpp
        src/KucoinDataclasses.h

        src/config/gate_config.h
        src/config/aeron_channel.h
        src/config/core_config.h
        )

add_executable(
        temp

        src/temp.cpp
        src/KucoinDataclasses.cpp
        src/KucoinDataclasses.h
        libs/simdjson/simdjson.h
        libs/simdjson/simdjson.cpp

)

target_link_libraries(
        ${PROJECT_NAME}
        Threads::Threads
        OpenSSL::SSL
        Boost::log
        ${LIB_AERON}/cppbuild/Release/lib/libaeron_client.a
)
