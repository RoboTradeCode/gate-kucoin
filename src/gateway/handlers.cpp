//
// Created by qod on 15.02.2022.
//

#include "handlers.h"

// Обработчик сообщения, содержащего информацию для подключения к Kucoin websocket
Bullet bullet_handler(const std::string &message) {
    auto object = json::parse(message).as_object();
    auto result = Bullet{};

    BOOST_LOG_TRIVIAL(trace) << object;

    if (object.if_contains("code") &&
        object.at("code") == "200000") {
        Uri uri = Uri::Parse(std::string(object.at("data").at("instanceServers").at(0).at("endpoint").as_string()));
        result = Bullet{
                std::string(object.at("data").at("token").as_string()),
                uri.Host,
                uri.Path,
                object.at("data").at("instanceServers").at(0).at("pingInterval").as_int64(),
                object.at("data").at("instanceServers").at(0).at("pingTimeout").as_int64()
        };
    }

    return result;
}

// Обработчик websocket, оформляет и отправляет сообщения с ордербуком в ядром
void orderbook_ws_handler(const std::string &message) {

    BOOST_LOG_TRIVIAL(trace) << "Received message in private ws_stream handler: " << message;
    auto object = json::parse(message).as_object();

    if (object.at("type").as_string() == "welcome" ||
        object.at("type").as_string() == "ack" ||
        object.at("type").as_string() == "pong")
        return;

    if (object.if_contains("subject") &&
        object.at("subject").as_string() == "trade.ticker") {
        aeron_channels.orderbook->offer(json::serialize(json::value{
                {"u",          object.at("data").at("sequence")},
                {"tcp_stream", "BTCUSDT"}, // todo - сделать замену значений по типу BTC-USDT => BTCUSDT (т.к. будет много тикеров)
                {"b",          object.at("data").at("bestBid")},
                {"B",          object.at("data").at("bestBidSize")},
                {"a",          object.at("data").at("bestAsk")},
                {"A",          object.at("data").at("bestAskSize")},
                {"T",          std::to_string(time(nullptr))}
        }));
    } else {
        BOOST_LOG_TRIVIAL(error) << "Unexpected message: " << message;
        aeron_channels.logs->offer(formulate_log_message(
                'e', 'g', 0, "Unexpected message on websocket: " + message
        ));
    }
}

// Обработчик websocket, оформляет и отправляет сообщения с балансом по ассету в ядром
void balance_ws_handler(const std::string &message) {
    auto object = json::parse(message).as_object();

    // если это системное сообщение, пропускаю
    if (object.at("type").as_string() == "welcome" ||
        object.at("type").as_string() == "ack" ||
        object.at("type").as_string() == "pong")
        return;

    BOOST_LOG_TRIVIAL(debug) << "Received message of balance: " << message;

    // проверяю, относится ли сообщение к изменению баланса
    if (object.if_contains("subject") &&
        object.at("subject") == "account.balance") {
        // проверяю, есть ли нужные поля с балансом валюты
        if (!object.at("data").at("total").as_string().empty() &&
            !object.at("data").at("currency").as_string().empty()) {
            auto balance = json::serialize(json::value{
                    {"B", json::array(
                            {
                                    json::value{
                                            {"a", object.at("data").at("currency").as_string()},
                                            {"f", object.at("data").at("total").as_string()}
                                    }
                            }
                    )}
            });
            aeron_channels.balance->offer(balance);
            BOOST_LOG_TRIVIAL(trace) << "Sent balance to core: " << balance;
        }
    }
        // если это неожиданное сообщение, вывожу ошибку
    else {
        BOOST_LOG_TRIVIAL(error) << "Unexpected message: " << message;
        aeron_channels.logs->offer(formulate_log_message(
                'e', 'g', 0, "Unexpected message on websocket: " + message
        ));
    }
}


// Обработчик сообщений из aeron на выставление и отмену ордеров (BTCUSDT)
void orders_aeron_handler(const std::string &message) {
    throw std::runtime_error("TEST AERON ERROR");
    BOOST_LOG_TRIVIAL(debug) << "Received message in aeron handler: " << message;
    auto object = json::parse(message).as_object();
    std::string result{};

    if (object.at("a") == "+" && object.at("S") == "BTCUSDT") {
        auto id = std::to_string(get_unix_timestamp());
        result = kucoin.rest->send_order(
                id,
                "BTC-USDT",
                object.at("s") == "BUY" ? "to_buy" : "to_sell",
                object.at("t") == "LIMIT" ? "limit" : "market",
                std::string(object.at("q").as_string()),
                format_price_precision(std::string(object.at("p").as_string()))
        );

        BOOST_LOG_TRIVIAL(debug) << "Sent request to create order (id" << id << ")";
        BOOST_LOG_TRIVIAL(debug) << "Result: " << result;

        auto result_object = json::parse(result).as_object();
        if (result_object.at("code") == "200000") {
            auto orderId = std::string(result_object.at("data").at("orderId").as_string());
            if (object.at("s") == "BUY") {
                orders.to_buy.push_back(orderId);
            } else if (object.at("s") == "SELL") {
                orders.to_sell.push_back(orderId);
            }
        }
    } else if (object.at("a") == "-" && object.at("S") == "BTCUSDT") {
        std::string id{};

        if (object.at("s") == "BUY" && !orders.to_buy.empty()) {
            id = orders.to_buy.back();
            orders.to_buy.pop_back();
        } else if (object.at("s") == "SELL" && !orders.to_sell.empty()) {
            id = orders.to_sell.back();
            orders.to_sell.pop_back();
        }

        if (!id.empty()) {
            result = kucoin.rest->cancel_order(id);
            BOOST_LOG_TRIVIAL(debug) << "Sent request to cancel order (id " << id << ")";
            BOOST_LOG_TRIVIAL(debug) << "Result: " << result;
        }
        return;
    } else {
        BOOST_LOG_TRIVIAL(error) << "Unexpected message: " << message;
        aeron_channels.logs->offer(formulate_log_message(
                'e', 'c', 0, "Unexpected message from core: " + message
        ));
    }
}



// Форматирует цену BTC, округляет до десятых
std::string format_price_precision(std::string price) {
    price.resize(7, '0');
    return price;
}
