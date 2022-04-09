//
// Created by qod on 15.02.2022.
//

#include "gateway.h"
#include <boost/json/src.hpp>


void send_current_balance() {
    auto btc_balance_json = kucoin.rest->get_balance(config->account.ids.btc_account_id);
    auto btc_balance_object = json::parse(btc_balance_json).as_object();

    auto usdt_balance_json = kucoin.rest->get_balance(config->account.ids.usdt_account_id);
    auto usdt_balance_object = json::parse(usdt_balance_json).as_object();

//    "{\"code\":\"200000\",\"data\":{\"currency\":\"BTC\",\"balance\":\"0.00209\",\"available\":\"0.00209\",\"holds\":\"0\"}}"
    if (btc_balance_object.if_contains("code") && btc_balance_object.at("code") == "200000" &&
        usdt_balance_object.if_contains("code") && usdt_balance_object.at("code") == "200000") {
        auto balance = json::serialize(json::value{
                {"B", json::array({
                                          json::value{
                                                  {"a", btc_balance_object.at("data").at("currency").as_string()},
                                                  {"f", btc_balance_object.at("data").at("available").as_string()},
                                          }, json::value{
                                {"a", usdt_balance_object.at("data").at("currency").as_string()},
                                {"f", usdt_balance_object.at("data").at("available").as_string()}
                        }
                                  }
                )}
        });
        aeron_channels.balance->offer(balance);
        BOOST_LOG_TRIVIAL(trace) << "Sent balances to core: " << balance;
    }
}

void connect_to_aeron_channels() {
    aeron_channels.orderbook = std::make_shared<Publisher>(config->aeron.publishers.orderbook.channel,
                                                    config->aeron.publishers.orderbook.stream_id);
    aeron_channels.balance = std::make_shared<Publisher>(config->aeron.publishers.balance.channel,
                                                  config->aeron.publishers.balance.stream_id);
    aeron_channels.logs = std::make_shared<Publisher>(config->aeron.publishers.logs.channel,
                                               config->aeron.publishers.logs.stream_id);

    aeron_channels.core = std::make_shared<Subscriber>(&orders_aeron_handler,
                                                config->aeron.subscribers.core.channel,
                                                config->aeron.subscribers.core.stream_id);
}



void subscribe_to_ws_channels() {// 5.1 Balance channel
    BOOST_LOG_TRIVIAL(info) << "Sent request to subscribe to balance channel...";
    kucoin.private_ws->subscribe_to_channel("/account/balance", 0, true);

    // 5.2. Orderbook channel
    BOOST_LOG_TRIVIAL(info) << "Sent request to subscribe to orderbook channel...";
    kucoin.public_ws->subscribe_to_channel("/market/ticker:BTC-USDT", 1, false);
}

void connect_to_kucoin() {
    BOOST_LOG_TRIVIAL(info) << "Trying to connect to REST...";
    kucoin.rest = std::make_shared<KucoinREST>(
            io_context,
            config->account.api_key,
            config->account.passphrase,
            config->account.secret_key
    );
    BOOST_LOG_TRIVIAL(info) << "Public websocket connection established.";

    // 3. Соединение с Kucoin Public websocket
    BOOST_LOG_TRIVIAL(info) << "Trying to connect to public websocket...";
    auto public_bullet = bullet_handler(kucoin.rest->get_public_bullet());
    kucoin.public_ws = std::make_shared<KucoinWS>(io_context, public_bullet, orderbook_ws_handler);
    BOOST_LOG_TRIVIAL(info) << "Public websocket connection established.";

    // 4. Соединение с Kucoin Private websocket
    BOOST_LOG_TRIVIAL(info) << "Trying to connect to private websocket...";
    auto private_bullet = bullet_handler(kucoin.rest->get_private_bullet());
    kucoin.private_ws = std::make_shared<KucoinWS>(io_context, private_bullet, balance_ws_handler);
    BOOST_LOG_TRIVIAL(info) << "Private websocket connection established.";
}

// Функция оформляет сообщения для логов, т.е. создает json определенного формата
std::string formulate_log_message(char type, char error_source, int code,
                                  std::basic_string<char, std::char_traits<char>, std::allocator<char>> message) {
    return json::serialize(json::value{
            {"t", std::chrono::duration_cast<std::chrono::seconds>(
                    std::chrono::system_clock::now().time_since_epoch()).count()},
            {"T", type},
            {"p", error_source},
            {"n", config->exchange.name},
            {"c", code},
            {"e", message}
    });
}

// Загрузка конфигурации
void load_configuration(const std::string& path_to_config) {
    config = std::make_shared<gate_config>(path_to_config);
}

