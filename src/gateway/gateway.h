//
// Created by qod on 15.02.2022.
//

#ifndef KUCOIN_GATEWAY_GATEWAY_H
#define KUCOIN_GATEWAY_GATEWAY_H

#include <boost/thread/thread.hpp>
#include <boost/algorithm/string.hpp>
#include <algorithm>

#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>

#include "handlers.h"

#include "KucoinWS.h"
#include "kucoin_structures.h"
#include "KucoinREST.h"
#include "Publisher.h"
#include "Subscriber.h"
#include "Uri.h"
#include "utils.h"

static std::shared_ptr<gate_config> config;

static boost::asio::io_context io_context;

struct KucoinAPI {
    std::shared_ptr<KucoinWS> public_ws;
    std::shared_ptr<KucoinWS> private_ws;
    std::shared_ptr<KucoinREST> rest;
};
KucoinAPI kucoin(<#initializer#>, 0, <#initializer#>);

struct AeronChannels {
    std::shared_ptr<Publisher> orderbook;
    std::shared_ptr<Publisher> balance;
    std::shared_ptr<Publisher> logs;
    std::shared_ptr<Subscriber> core;
} static aeron_channels;

// векторы, хранящие id ордеров
struct ActiveOrders {
    std::vector<std::string> to_buy;
    std::vector<std::string> to_sell;
} static orders;


// отвечают за действия шлюза
void send_current_balance();
void connect_to_kucoin();
void connect_to_aeron_channels();
void subscribe_to_ws_channels();

// прочие функции, нужные для работы шлюза
void load_configuration(const std::string& path_to_config);
std::string formulate_log_message(char type, char error_source, int code,
                                  std::basic_string<char, std::char_traits<char>, std::allocator<char>> message);



#endif //KUCOIN_GATEWAY_GATEWAY_H
