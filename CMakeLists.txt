cmake_minimum_required(VERSION 3.15)
project(kucoin_gateway)

set(CMAKE_CXX_STANDARD 20)

set(LIB_AERON /home/ubuntu/aeron/)


find_package(Boost 1.78.0 REQUIRED COMPONENTS log)
find_package(Threads REQUIRED)
find_package(OpenSSL REQUIRED)

include_directories(${LIB_AERON}/aeron-client/src/main/cpp)

#include(libs/aeron_cpp/CMakeLists.txt)

add_executable(
        main

        main.cpp

        libs/aeron_cpp/src/Publisher.cpp
        libs/aeron_cpp/src/Publisher.h
        libs/aeron_cpp/src/Subscriber.cpp
        libs/aeron_cpp/src/Subscriber.h

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
        src/kucoin_structures.cpp
        src/kucoin_structures.h
        src/Uri.cpp
        src/Uri.h

        src/config/gate_config.h  s
        rc/config/aeron_channel.h
        src/config/core_config.h
)
target_link_libraries(
        main
        Threads::Threads
        OpenSSL::SSL
        Boost::log
        ${LIB_AERON}/cppbuild/Release/lib/libaeron_client.a
)
