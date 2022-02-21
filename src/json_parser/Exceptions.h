#ifndef KUCOIN_GATEWAY_EXCEPTIONS_H
#define KUCOIN_GATEWAY_EXCEPTIONS_H

#include "/home/qod/Projects/Research/C++/KucoinGateway/libs/simdjson/simdjson.h"

struct BaseJsonException : public std::exception {
    simdjson::error_code error_code;
    explicit BaseJsonException(simdjson::error_code ec) : error_code(ec) {}
};

struct InitializationException : public BaseJsonException {
    explicit InitializationException(simdjson::error_code error_code) : BaseJsonException(error_code) {}
    [[nodiscard]] const char* what() const noexcept override {
        return "Failed BaseJsonException parser initialization.";
    }
};

struct ParsingException : public BaseJsonException {
    explicit ParsingException(simdjson::error_code error_code) : BaseJsonException(error_code) {}
    [[nodiscard]] const char* what() const noexcept override {
        return "Failed BaseJsonException parsing.";
    }
};

#endif //KUCOIN_GATEWAY_EXCEPTIONS_H
