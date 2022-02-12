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
        src/Publisher.cpp         src/includes/Publisher.h
        src/Subscriber.cpp        src/includes/Subscriber.h
        src/WSSession.cpp         src/includes/WSSession.h
        src/HTTPSession.cpp       src/includes/HTTPSession.h
        src/KucoinWS.cpp          src/includes/KucoinWS.h
        src/KucoinREST.cpp        src/includes/KucoinREST.h
        src/utils.cpp             src/includes/utils.h
        src/kucoin_structures.cpp src/includes/kucoin_structures.h
        src/Uri.cpp               src/includes/Uri.h
        src/config/gate_config.h  src/config/aeron_channel.h
        src/config/core_config.h
)
target_link_libraries(
        main
        Threads::Threads
        OpenSSL::SSL
        Boost::log
        ${LIB_AERON}/libaeron_client.a
)
