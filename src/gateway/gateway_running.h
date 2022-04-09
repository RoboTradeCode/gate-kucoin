//
// Created by qod on 15.02.2022.
//

#ifndef KUCOIN_GATEWAY_GATEWAY_RUNNING_H
#define KUCOIN_GATEWAY_GATEWAY_RUNNING_H


#include <boost/json.hpp>
#include <boost/log/trivial.hpp>
#include "gate_config.h"
#include "KucoinWS.h"
#include "KucoinREST.h"
#include "gateway.h"

namespace json = boost::json;
namespace logging = boost::log;

// отвечают за состояния шлюза
void startup_gateway();

[[noreturn]] void run_gateway_loop();
#endif //KUCOIN_GATEWAY_GATEWAY_RUNNING_H
