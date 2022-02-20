#pragma once
#include <string>

/// @brief Базовый класс данных, полученных от Kucoin REST API
struct KucoinMessageREST {
    std::string code;
};

struct KucoinMessageWebsocket {
    std::string type;
    std::string topic;
    std::string subject;
};

struct Bullet : public KucoinMessageREST {
    Bullet(std::string token, std::string host, std::string path,
           int64_t pingInterval, int64_t pingTimeout);

    std::string token;
    std::string host;
    std::string path;
    int64_t pingInterval;
    int64_t pingTimeout;
};

struct Balance {
    std::string currencyName;
    std::string allFunds;
    std::string available;
    std::string holds;
};

struct BalanceRESTMessage : public KucoinMessageREST {
    std::string currencyName;
    std::string allFunds;
    std::string available;
    std::string holds;

    BalanceRESTMessage(std::string_view json);
};

struct BalanceWebsocketMessage : public KucoinMessageWebsocket {
    std::string currencyName;
    std::string allFunds;
    std::string available;
    std::string holds;
};

struct OrderBook : public KucoinMessageREST  {
    std::string sequence;
    std::string time;
    std::string bestAs;
    std::string bestAskSize;
    std::string bestBid;
    std::string bestBidSize;
    std::string price;
    std::string size;
};

struct Uri {
    std::string queryString;
    std::string path;
    std::string protocol;
    std::string host;
    std::string port;

    static Uri Parse(const std::string &uri);
};

