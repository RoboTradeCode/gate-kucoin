#ifndef KUCOIN_GATEWAY_HANDLERS_H
#define KUCOIN_GATEWAY_HANDLERS_H

#include <boost/log/trivial.hpp>

#include "gateway_running.h"
#include "gateway.h"
#include "Uri.h"

#include "KucoinWS.h"
#include "kucoin_structures.h"
#include "KucoinREST.h"

namespace json = boost::json;
namespace logging = boost::log;

extern KucoinAPI kucoin(<#initializer#>, 0, <#initializer#>);

Bullet bullet_handler(const std::string &);

void orderbook_ws_handler(const std::string &message);

void balance_ws_handler(const std::string &message);

void orders_aeron_handler(const std::string &message);

std::string format_price_precision(std::basic_string<char> price);

#endif //KUCOIN_GATEWAY_HANDLERS_H
