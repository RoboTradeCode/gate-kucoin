cmake_minimum_required(VERSION 3.21)
project(kucoin_gateway)

set(CMAKE_CXX_STANDARD 20)

set(LIB_AERON /home/qod/Projects/libs/aeron/cppbuild/Release/lib/)


find_package(Boost 1.78.0 REQUIRED COMPONENTS log)
find_package(Threads REQUIRED)
find_package(OpenSSL REQUIRED)

include_directories(/home/qod/Projects/libs/aeron/aeron-client/src/main/cpp)

add_executable(
        main main.cpp
        libs/aeron_cpp/Publisher.cpp
        libs/aeron_cpp/Publisher.h
        libs/aeron_cpp/Subscriber.cpp
        libs/aeron_cpp/Subscriber.h

        src/network_cpp/WSSession.cpp
        src/network_cpp/WSSession.h
        src/network_cpp/HTTPSession.cpp
        src/network_cpp/HTTPSession.h
        src/network_cpp/Uri.cpp
        src/network_cpp/Uri.h

        src/kucoin_api/KucoinWS.cpp
        src/kucoin_api/KucoinWS.h
        src/kucoin_api/KucoinREST.cpp
        src/kucoin_api/KucoinREST.h
        src/kucoin_api/kucoin_structures.cpp
        src/kucoin_api/kucoin_structures.h

        src/utils.h

        src/config/gate_config.h
        src/config/aeron_channel.h
        src/config/core_config.h
        src/utils.cpp src/gateway/gateway_running.cpp src/gateway/gateway_running.h src/gateway/handlers.cpp src/gateway/handlers.h src/gateway/gateway.cpp src/gateway/gateway.h src/kucoin_api/KucoinAPI.cpp src/kucoin_api/KucoinAPI.h)

target_include_directories(
        main
        PUBLIC
        src
        src/config
        src/gateway
        src/kucoin_api
        src/network_cpp
        libs/aeron_cpp
        libs/toml++
)

target_link_libraries(
        main
        Threads::Threads
        OpenSSL::SSL
        Boost::log
        ${LIB_AERON}/libaeron_client.a
)
