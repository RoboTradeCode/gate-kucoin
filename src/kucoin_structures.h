#pragma once
#include <string>

struct Bullet {
    std::string token;
    std::string host;
    std::string path;
    int64_t pingInterval = 0;
    int64_t pingTimeout = 0;
};
