#include <boost/thread/thread.hpp>
#include <boost/log/trivial.hpp>
#include <boost/json/src.hpp>
#include <boost/algorithm/string.hpp>
#include <algorithm>

#include "src/includes/KucoinWS.h"
#include "src/includes/kucoin_structures.h"
#include "src/includes/KucoinREST.h"
#include "src/includes/Publisher.h"
#include "src/includes/Subscriber.h"
#include "src/includes/Uri.h"

#include "src/config/gate_config.h"

namespace json = boost::json;

Bullet bullet_handler(const std::string&);

void orderbook_ws_handler(const std::string &message);

void balance_ws_handler(const std::string &message);

void orders_aeron_handler(const std::string &message);

void sigint_handler(int);

std::string formulate_log_message(char type, char error_source, int code,
                           std::basic_string<char, std::char_traits<char>, std::allocator<char>> message);

std::string api_key{};
std::string passphrase{};
std::string secret_key{};
std::string exchange_name{};



std::atomic<bool> running(true);
std::shared_ptr<KucoinWS> kucoin_public_ws;
std::shared_ptr<KucoinWS> kucoin_private_ws;
std::shared_ptr<KucoinREST> kucoin_rest;
std::shared_ptr<Publisher> orderbook_channel;
std::shared_ptr<Publisher> balance_channel;
std::shared_ptr<Publisher> logs_channel;
std::shared_ptr<Subscriber> core_channel;

// векторы, хранящие id ордеров
std::vector<std::string> buy_orders;
std::vector<std::string> sell_orders;



int main()
{
    /* Алгоритм работы:
     * 0. Получение настроек конфигурации шлюза
     * 1. Соединение с каналами Aeron
     * 2. Соединение с Kucoin REST API
     * 3. Соединение с Kucoin Public websocket
     * 4. Соединение с Kucoin Private websocket
     * 5. Подписка на необходимые каналы websocket
     * 5.1 Канал для получения баланса
     * 5.2 Канал для получения ордербука
     * 6. Получение списка ордеров
     * 7. Отмена активных ордеров
     * 8. Бесконечный цикл, в нём проверка сообщения от ядра
     * */

    using namespace std::literals::chrono_literals;


    // 0. Получение настроек конфигурации шлюза
    BOOST_LOG_TRIVIAL(info) << "Load configuration...";
    gate_config config("kucoin_config.toml");

    exchange_name = config.exchange.name;
    api_key = config.account.api_key;
    secret_key = config.account.secret_key;
    passphrase = config.account.passphrase;


    // 1. Соединение с каналами Aeron
    BOOST_LOG_TRIVIAL(info) << "Trying to connect with aeron...";
    orderbook_channel = std::make_shared<Publisher>(config.aeron.publishers.orderbook.channel,
                                                    config.aeron.publishers.orderbook.stream_id);
    balance_channel = std::make_shared<Publisher>(config.aeron.publishers.balance.channel,
                                                  config.aeron.publishers.balance.stream_id);
    logs_channel = std::make_shared<Publisher>(config.aeron.publishers.logs.channel,
                                               config.aeron.publishers.logs.stream_id);

    core_channel = std::make_shared<Subscriber>(&orders_aeron_handler,
                                                config.aeron.subscribers.core.channel,
                                                config.aeron.subscribers.core.stream_id);

    BOOST_LOG_TRIVIAL(info) << "Aeron connections established";

    // 2. Соединение с Kucoin REST API
    BOOST_LOG_TRIVIAL(info) << "Trying to connect to public websocket...";
    boost::asio::io_context ioc;
    kucoin_rest = std::make_shared<KucoinREST>(ioc, api_key, passphrase, secret_key);
    BOOST_LOG_TRIVIAL(info) << "Public websocket connection established.";

    // 3. Соединение с Kucoin Public websocket
    BOOST_LOG_TRIVIAL(info) << "Trying to connect to public websocket...";
    auto public_bullet = bullet_handler(kucoin_rest->get_public_bullet());
    kucoin_public_ws = std::make_shared<KucoinWS>(ioc, public_bullet, orderbook_ws_handler);
    BOOST_LOG_TRIVIAL(info) << "Public websocket connection established.";

    // 4. Соединение с Kucoin Private websocket
    BOOST_LOG_TRIVIAL(info) << "Trying to connect to private websocket...";
    auto private_bullet = bullet_handler(kucoin_rest->get_private_bullet());
    kucoin_private_ws = std::make_shared<KucoinWS>(ioc, private_bullet, balance_ws_handler);
    BOOST_LOG_TRIVIAL(info) << "Private websocket connection established.";

    // 5. Подписка на необходимые каналы websocket

    // 5.1 Balance channel
    BOOST_LOG_TRIVIAL(info) << "Sent request to subscribe to balance channel";
    kucoin_private_ws->subscribe_to_channel("/account/balance", 0, true);

    // 5.2. Orderbook channel
    BOOST_LOG_TRIVIAL(info) << "Sent request to subscribe to orderbook channel";
    kucoin_public_ws->subscribe_to_channel("/market/ticker:BTC-USDT", 1, false);

    BOOST_LOG_TRIVIAL(info) << "Kucoin connections established";

    // 6. Получение активных ордеров
    BOOST_LOG_TRIVIAL(info) << "Trying to get list of active orders";
    auto orders = kucoin_rest->get_active_orders();
    if (!orders.empty()) { // todo fix empty, now is always True
        // Если есть активные ордера, логгирую их
        BOOST_LOG_TRIVIAL(info) << "List of active orders: " << orders;
        logs_channel->offer(formulate_log_message(
                'i', 'g', 0, "List of active orders: " + orders
                ));
        // 7. Отменяю активные ордера
        kucoin_rest->cancel_all_orders();
    } else
        BOOST_LOG_TRIVIAL(info) << "No active orders";

    // Обработчик прерываний, для выхода из цикла.
    signal(SIGINT, sigint_handler);

    auto last_ping_time = std::chrono::system_clock::now();


    // 8. Основной цикл работы шлюза
    /* Шаги работы основного цикла:
     * 1. Обработка сообщений, полученных по websocket
     * 2. Обработка ордеров, полученных от ядра
     * 3. Проверка, пришло ли время пропинговать websocket
     * */
    BOOST_LOG_TRIVIAL(info) << "Enter the main loop";
    while (running)
    {
        // 1. Обработка сообщений, полученных по websocket
        ioc.run_for(std::chrono::milliseconds(100));

        // 2. Обработка ордеров, полученных от ядра
        core_channel->poll();

        // 3. Проверка, пришло ли время пропинговать websocket
        if (std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now() - last_ping_time) > 15s) {
            BOOST_LOG_TRIVIAL(trace) << "Ping websockets, previous ping time: "
            << std::chrono::duration_cast<std::chrono::seconds>(
                    last_ping_time.time_since_epoch()).count();
            kucoin_public_ws->ping();
            kucoin_private_ws->ping();
            last_ping_time = std::chrono::system_clock::now();
        }
    }

    BOOST_LOG_TRIVIAL(info) << "Exit the main loop";
    return EXIT_SUCCESS;
}

// Функция оформляет сообщения для логов, т.е. создает json определенного формата
std::string formulate_log_message(char type, char error_source, int code,
                           std::basic_string<char, std::char_traits<char>, std::allocator<char>> message) {
    return json::serialize(json::value{
            { "t", std::chrono::duration_cast<std::chrono::seconds>(
                    std::chrono::system_clock::now().time_since_epoch()).count()},
            { "T", type},
            { "p", error_source},
            { "n", exchange_name},
            { "c", code},
            { "e", message}
    });
}

// Обработчик сообщения, содержащего информацию для подключения к Kucoin websocket
Bullet bullet_handler(const std::string& message)
{
    auto object = json::parse(message).as_object();
    auto result = Bullet{};

    BOOST_LOG_TRIVIAL(trace) << object;

    if (object.if_contains("code") &&
        object.at("code") == "200000")
    {
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

// todo add "welcome" and "ack" is normal ws messages
//[2022-02-12 01:38:55.823138] [0x00007f39278bef00] [error]   Unexpected message: {"id":"VHcY5VAAQC","type":"welcome"}
//[2022-02-12 01:38:55.823348] [0x00007f39278bef00] [info]    {"id":"1","type":"ack"}
//[2022-02-12 01:38:55.823376] [0x00007f39278bef00] [error]   Unexpected message: {"id":"1","type":"ack"}

// Обработчик websocket, оформляет и отправляет сообщения с ордербуком в ядром
void orderbook_ws_handler(const std::string& message)
{

    BOOST_LOG_TRIVIAL(trace) << "Received message in private ws_stream handler: " << message;
    auto object = json::parse(message).as_object();

    if (object.at("type").as_string() == "welcome" ||
        object.at("type").as_string() == "ack" ||
        object.at("type").as_string() == "pong") return;

    if (object.if_contains("subject") &&
        object.at("subject").as_string() == "trade.ticker")
    {
        orderbook_channel->offer(json::serialize(json::value{
            { "u", object.at("data").at("sequence") },
            { "tcp_stream", "BTCUSDT"}, // todo - сделать замену значений по типу BTC-USDT => BTCUSDT (т.к. будет много тикеров)
            { "b", object.at("data").at("bestBid") },
            { "B", object.at("data").at("bestBidSize") },
            { "a", object.at("data").at("bestAsk") },
            { "A", object.at("data").at("bestAskSize") },
            { "T", std::to_string(time(nullptr)) }
        }));
    }
    else {
        BOOST_LOG_TRIVIAL(error) << "Unexpected message: " << message;
        logs_channel->offer(formulate_log_message(
                'e', 'g', 0, "Unexpected message on websocket: " + message
        ));
    }
}

// Обработчик websocket, оформляет и отправляет сообщения с балансом по ассету в ядром
void balance_ws_handler(const std::string& message) {
    BOOST_LOG_TRIVIAL(trace) << "Received message in private ws_stream handler: " << message;
    auto object = json::parse(message).as_object();

    if (object.at("type").as_string() == "welcome" ||
        object.at("type").as_string() == "ack" ||
        object.at("type").as_string() == "pong") return;

    if (object.if_contains("subject") &&
        object.at("subject") == "account.balance") {
        if (!object.at("data").at("total").as_string().empty() &&
            !object.at("data").at("currency").as_string().empty()) {
            auto balance = json::serialize(json::value{
                    {"B", json::array({
                                              json::value{
                                                      "a", object.at("data").at("currency").as_string(),
                                                      "f", object.at("data").at("total").as_string()
                                              }
                                      }
                    )}
            });
            balance_channel->offer(balance);
            BOOST_LOG_TRIVIAL(trace) << "Sent balance to core: " << balance;
        }
    }
    else {
        BOOST_LOG_TRIVIAL(error) << "Unexpected message: " << message;
        logs_channel->offer(formulate_log_message(
                'e', 'g', 0, "Unexpected message on websocket: " + message
        ));
    }
}


// Обработчик сообщений из aeron на выставление и отмену ордеров (BTCUSDT)
void orders_aeron_handler(const std::string& message)
{
    BOOST_LOG_TRIVIAL(trace) << "Received message in aeron handler: " << message;
    auto object = json::parse(message).as_object();
    std::string result{};

    if (object.at("a") == "+" && object.at("S") == "BTCUSDT")
    {
        auto id = std::to_string(get_unix_timestamp());
        result = kucoin_rest->send_order(
            id,
            "BTC-USDT",
            object.at("s") == "BUY" ? "buy" : "sell",
            object.at("t") == "LIMIT" ? "limit" : "market",
            std::string(object.at("q").as_string()),
            std::string(object.at("p").as_string())
        );

        BOOST_LOG_TRIVIAL(trace) << "Sent request to create order (id" << id << ")";
        BOOST_LOG_TRIVIAL(trace) << "Result: " << result;

        if (object.at("s") == "BUY") {
            buy_orders.push_back(id);
        } else if (object.at("s") == "SELL") {
            sell_orders.push_back(id);
        }

        return;
    }
    else if (object.at("a") == "-" && object.at("S") == "BTCUSDT")
    {
        std::string id{};

        if (object.at("s") == "BUY") {
            id = buy_orders.back();
            buy_orders.pop_back();
        } else if (object.at("s") == "SELL") {
            id = sell_orders.back();
            sell_orders.pop_back();
        }

        if (!id.empty()) {
            result = kucoin_rest->cancel_order(id);
            BOOST_LOG_TRIVIAL(trace) << "Sent request to cancel order (id " << id << ")";
            BOOST_LOG_TRIVIAL(trace) << "Result: " << result;
        }
        return;
    }

    BOOST_LOG_TRIVIAL(error) << "Unexpected message: " << message;
    logs_channel->offer(formulate_log_message(
            'e', 'c', 0, "Unexpected message from core: " + message
    ));

}

void sigint_handler(int)
{
    running = false;
}
