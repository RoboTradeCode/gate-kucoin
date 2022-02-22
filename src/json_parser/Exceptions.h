#ifndef KUCOIN_GATEWAY_EXCEPTIONS_H
#define KUCOIN_GATEWAY_EXCEPTIONS_H

#include "/home/qod/Projects/Research/C++/KucoinGateway/libs/simdjson/simdjson.h"

struct InitializationException : public std::exception {
    simdjson::error_code error_code;

    explicit InitializationException(simdjson::error_code error_code) : error_code(error_code) {}

    [[nodiscard]] const char *what() const noexcept override {
        return "Failed BaseJsonException parser initialization.";
    }
};
#endif //KUCOIN_GATEWAY_EXCEPTIONS_H
