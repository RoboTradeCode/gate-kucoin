#pragma once
#include <string>

namespace Kucoin {

/// @brief Базовый класс данных, полученных от Kucoin REST API
    template<typename Data>
    struct MessageREST {
        std::string code;
        Data data;
    };

    template<typename Data>
    struct MessageWebsocket {
        std::string type;
        std::string topic;
        std::string subject;
        Data data;
    };

    struct Balance {
        std::string currencyName;
        std::string allFunds;
        std::string available;
        std::string holds;
    };
    
    struct Account : public Balance {
        std::string id;
        std::string type;
    };

    struct OrderBook {
        std::string sequence;
        std::string time;
        std::string bestAs;
        std::string bestAskSize;
        std::string bestBid;
        std::string bestBidSize;
        std::string price;
        std::string size;
    };

    struct Order {
        std::string id;
        std::string symbol;
        std::string type;
        std::string side;
        std::string price;
        std::string size;
    };

}

struct Uri {
    std::string queryString;
    std::string path;
    std::string protocol;
    std::string host;
    std::string port;

    static Uri parse(std::basic_string_view<char> uri);
};