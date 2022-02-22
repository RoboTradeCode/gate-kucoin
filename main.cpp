#include <boost/thread/thread.hpp>
#include <boost/json/src.hpp>
#include <boost/algorithm/string.hpp>
#include <algorithm>

#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>
#include "src/KucoinAPI/src/KucoinWS.h"
#include "src/KucoinAPI/src/KucoinDataclasses.h"
#include "src/KucoinAPI/src/KucoinREST.h"
#include "libs/aeron_cpp/src/Publisher.h"
#include "libs/aeron_cpp/src/Subscriber.h"

#include "src/config/gate_config.h"

namespace json = boost::json;
namespace logging = boost::log;

Bullet bullet_handler(const std::string &);

void orderbook_ws_handler(const std::string &message);

void balance_ws_handler(const std::string &message);

void aeron_orders_handler(std::string_view message);

// обрабатывает строку с ордером в определенном формате, их присылает ядро
void handle_order_message(const std::string &order_message);

void sigint_handler(int);

void subscribe_to_ws_channels();

void connect_to_kucoin();

std::string formulate_log_message(char type, char error_source, int code,
                                  std::basic_string<char, std::char_traits<char>, std::allocator<char>> message);

std::string format_price_precision(std::basic_string<char> price);

std::string format_size_precision_btc(std::string price);

// Форматирует объем USDT
std::string format_size_precision_usdt(std::string price);

void ping_ws();

void connect_to_public_ws();

void connect_to_private_ws();

void get_balance();

void enter_main_loop();

// Очередь ордеров, поступающие от ядра ордера добавляются в конец очереди
// В главном цикле шлюза ордера обрабатываются: т.е. отсылаются kucoin
// Обработка ордеров происходит не в обработчике сообщений от ядра, т.к.
// обработчик сообщений запускается в другом потоке. Из-за этого не удается
// обработать исключения, которые возникают во время обработки ордеров
std::list<std::string> orders_deque{};

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

boost::asio::io_context ioc;

std::shared_ptr<gate_config> config;

int main() {
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
     * 8. Получение текущего баланса аккаунта
     * 9. Бесконечный цикл, в нём проверка сообщения от ядра
     * */


    logging::core::get()->set_filter(logging::trivial::severity >= logging::trivial::trace);

    // 0. Получение настроек конфигурации шлюза
    BOOST_LOG_TRIVIAL(info) << "Load configuration...";
    // Указать корректный путь до конфига
    config = std::make_shared<gate_config>("../kucoin_config.toml");

    // 1. Соединение с каналами Aeron
    BOOST_LOG_TRIVIAL(info) << "Trying to connect with aeron...";
    orderbook_channel = std::make_shared<Publisher>(config->aeron.publishers.orderbook.channel,
                                                    config->aeron.publishers.orderbook.stream_id);
    balance_channel = std::make_shared<Publisher>(config->aeron.publishers.balance.channel,
                                                  config->aeron.publishers.balance.stream_id);
    logs_channel = std::make_shared<Publisher>(config->aeron.publishers.logs.channel,
                                               config->aeron.publishers.logs.stream_id, 2500);

    core_channel = std::make_shared<Subscriber>(aeron_orders_handler,
                                                config->aeron.subscribers.core.channel,
                                                config->aeron.subscribers.core.stream_id);

    BOOST_LOG_TRIVIAL(info) << "Aeron connections established";

    // 2. Соединение с Kucoin REST API
    BOOST_LOG_TRIVIAL(info) << "Trying to connect to public websocket...";
    kucoin_rest = std::make_shared<KucoinREST>(
            ioc,
            config->account.api_key,
            config->account.passphrase,
            config->account.secret_key
    );
    BOOST_LOG_TRIVIAL(info) << "Public websocket connection established.";

    // 3. Соединение с Kucoin Public websocket
    BOOST_LOG_TRIVIAL(info) << "Trying to connect to public websocket...";
    connect_to_public_ws();
    BOOST_LOG_TRIVIAL(info) << "Public websocket connection established.";

    // 4. Соединение с Kucoin Private websocket
    BOOST_LOG_TRIVIAL(info) << "Trying to connect to private websocket...";
    connect_to_private_ws();
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
    auto order_object = json::parse(orders).as_object();
    // Если есть ордера (totalNum - общее число активных оредров)
    if (order_object.at("data").at("totalNum").as_int64() > 0) {
        // Если есть активные ордера, логгирую их
        BOOST_LOG_TRIVIAL(info) << "List of active orders: " << orders;
        // todo превышает лимит пересылки сообщений aeron по символам, если есть ордера
        logs_channel->offer(formulate_log_message(
                'i', 'g', 0, "List of active orders: " + orders
                ));
        // 7. Отменяю активные ордера
//        kucoin_rest->cancel_all_orders();
    } else
        BOOST_LOG_TRIVIAL(info) << "No active orders";

    // 8. Получаю текущий баланс аккаунта
    get_balance();

    // Обработчик прерываний, для выхода из цикла.
    signal(SIGINT, sigint_handler);

    // 9. Основной цикл работы шлюза

    /* Шаги работы основного цикла:
     * 1. Обработка сообщений, полученных по websocket
     * 2. Обработка ордеров, полученных от ядра
     * 3. Проверка, пришло ли время пропинговать websocket
     * */


    BOOST_LOG_TRIVIAL(info) << "Enter the main loop";
    // вход в главный цикл шлюза
    enter_main_loop();

    BOOST_LOG_TRIVIAL(info) << "Exit the main loop";
    return EXIT_SUCCESS;
}

/// @brief Главный цикл шлюза
///
/// Выполняется всё время работы шлюза.
/// Выход из цикла означает завершение работы шлюза.
/// Операции внутри цикла выполняются внутри try catch
/// При обрыве соединения шлюз должен переподключиться
///
/// Операции во время итерации цикла:
///  1. Проверка, пришло ли время пропинговать websocket
///  2. Обработка сообщений, полученных по websocket
///  3. получаю ордера от ядра (через aeron), они записываются в orders_deque
///  4. прохожу циклом по всем ордерам
///  5. обрабатываю самый старый ордер в очереди
///   (новые добавляются в конец, самый старый = самый первый)
///  6. удаляю самый старый ордер
void enter_main_loop() {
    while (running) {
        try {
            // 1. Проверка, пришло ли время пропинговать websocket
            ping_ws();
            // 2. Обработка сообщений, полученных по websocket
            ioc.run_for(std::chrono::milliseconds(100));

            // 3. получаю ордера от ядра (через aeron), они записываются в orders_deque
            core_channel->poll();
            // 4. прохожу циклом по всем ордерам
            while (!orders_deque.empty()) {
                // 5. обрабатываю самый старый ордер в очереди
                // (новые добавляются в конец, самый старый = самый первый)
                handle_order_message(orders_deque.front());
                // 6. удаляю самый старый ордер
                orders_deque.pop_front();
            }

        } catch (http::error error) {
            BOOST_LOG_TRIVIAL(error)  << "Http error: " << make_error_code(error).to_string();
            BOOST_LOG_TRIVIAL(warning) << "Problem with connection with Kucoin! Reconnecting...";

            // переподключаюсь к REST API
            if (kucoin_rest->reconnect(ioc))
                BOOST_LOG_TRIVIAL(info) << "Kucoin connections established.";
            else
                BOOST_LOG_TRIVIAL(info) << "Problem with reconnection.";
        } catch (boost::system::system_error error) {
            BOOST_LOG_TRIVIAL(error) << "System error: " << error.code().message();
            BOOST_LOG_TRIVIAL(warning) << "Reconnecting to REST API...";

            // переподключаюсь к REST API
            if (kucoin_rest->reconnect(ioc))
                BOOST_LOG_TRIVIAL(info) << "Kucoin connections established.";
            else
                BOOST_LOG_TRIVIAL(info) << "Problem with reconnection.";
        } catch (websocket::error error) {
            BOOST_LOG_TRIVIAL(error) << "Websocket error: " << make_error_code(error).to_string();
            BOOST_LOG_TRIVIAL(warning) << "Reconnecting to websocket...";

            BOOST_LOG_TRIVIAL(info) << "Trying to connect to public websocket...";
            connect_to_public_ws();
            BOOST_LOG_TRIVIAL(info) << "Public websocket connection established.";

            // 4. Соединение с Kucoin Private websocket
            BOOST_LOG_TRIVIAL(info) << "Trying to connect to private websocket...";
            connect_to_private_ws();
            BOOST_LOG_TRIVIAL(info) << "Private websocket connection established.";

            // 5. Подписка на необходимые каналы websocket

            // 5.1 Balance channel
            BOOST_LOG_TRIVIAL(info) << "Sent request to subscribe to balance channel";
            kucoin_private_ws->subscribe_to_channel("/account/balance", 0, true);

            // 5.2. Orderbook channel
            BOOST_LOG_TRIVIAL(info) << "Sent request to subscribe to orderbook channel";
            kucoin_public_ws->subscribe_to_channel("/market/ticker:BTC-USDT", 1, false);

            BOOST_LOG_TRIVIAL(info) << "Kucoin connections established";
        } catch(const std::runtime_error& re) {
            BOOST_LOG_TRIVIAL(error) << "Runtime error: " << re.what();
        } catch(const std::exception& ex) {
            BOOST_LOG_TRIVIAL(error) << "Error occurred: " << ex.what();
        } catch(...) {
            BOOST_LOG_TRIVIAL(error) << "Unknown failure occurred. Possible memory corruption";
        }
    }
}

/// @brief Отправляет ядру актуальный баланс шлюза по всем аккаунтам, указанным в конфигурации
/// Выполняет много запросов к API, если указано много аккаунтов (запрос на каждый аккаунт)
void get_balance() {
    auto ids{config->account.ids};
    std::vector<json::value> balances{};
    for (auto id : ids) {
        auto balance_message = kucoin_rest->get_balance(id);
        BOOST_LOG_TRIVIAL(trace) << balance_message;
        auto id_balance = json::parse(balance_message).as_object();
        if (id_balance.if_contains("code") && id_balance.at("code") == "200000") {
            balances.push_back(json::value{
                {"a", id_balance.at("data").at("currency").as_string()},
                {"f", id_balance.at("data").at("available").as_string()}
            });
        }
        auto full_balance = json::serialize(json::value{{"B", balances}});
        balance_channel->offer(full_balance);
        BOOST_LOG_TRIVIAL(trace) << "Sent balances to core: " << full_balance;
    }
}

/// @brief подключиться к публичному вебсокету Kucoin
void connect_to_public_ws() {
    auto public_bullet = bullet_handler(kucoin_rest->get_public_bullet());
    kucoin_public_ws = std::make_shared<KucoinWS>(ioc, public_bullet, orderbook_ws_handler);
}

/// @brief подключиться к приватному вебсокету Kucoin
void connect_to_private_ws() {
    auto private_bullet = bullet_handler(kucoin_rest->get_private_bullet());
    kucoin_private_ws = std::make_shared<KucoinWS>(ioc, private_bullet, balance_ws_handler);
}

/// @brief Пропинговать вебсокеты (kucoin_public_ws, kucoin_private_ws)
void ping_ws() {
    using namespace std::literals::chrono_literals;

    static auto last_ping_time = std::chrono::system_clock::now();

    if (std::chrono::duration_cast<std::chrono::seconds>
            (std::chrono::system_clock::now() - last_ping_time) > 15s) {
        BOOST_LOG_TRIVIAL(trace) << "Ping websockets, previous ping time: "
                                 << std::chrono::duration_cast<std::chrono::seconds>(
                                         last_ping_time.time_since_epoch()).count();
        kucoin_public_ws->ping();
        kucoin_private_ws->ping();
        last_ping_time = std::chrono::system_clock::now();
    }
}

/// @brief Оформлить сообщения для логов, создание строки json
/// фугкция создает json определенного формата
/// @param type тип ошибки: i - information, e - error
/// @param error_source где произошла ошибка, по мнению шлюза
/// @param message сообщение об ошибке
/// @return string, оформленный json, с дополнительными полями (timestamp, название биржи)
std::string formulate_log_message(char type, char error_source, int code,
                                  std::basic_string<char, std::char_traits<char>, std::allocator<char>> message) {
    return json::serialize(json::value{{"t", std::chrono::duration_cast<std::chrono::seconds>(
            std::chrono::system_clock::now().time_since_epoch()).count()},
                                       {"T", type},
                                       {"p", error_source},
                                       {"n", config->exchange.name},
                                       {"c", code},
                                       {"e", message}});
}

/// @brief Обработчик сообщения, содержащего информацию для подключения к Kucoin websocket
/// @param message строка json, полученная из Kucoin REST API
/// @return Bullet, структура, содержащая поля json
Bullet bullet_handler(const std::string &message) {
    auto object = json::parse(message).as_object();
    std::unique_ptr<Bullet> result;

    BOOST_LOG_TRIVIAL(trace) << object;

    if (object.if_contains("code") && object.at("code") == "200000") {
        Uri uri = Uri::Parse(std::string(object.at("data").at("instanceServers").at(0).at("endpoint").as_string()));
        result = std::make_unique<Bullet>(std::string(object.at("data").at("token").as_string()), uri.host, uri.path,
                        object.at("data").at("instanceServers").at(0).at("pingInterval").as_int64(),
                        object.at("data").at("instanceServers").at(0).at("pingTimeout").as_int64());
    }

    return *result;
}


/// @brief Обработчик websocket, оформляет и отправляет сообщения с ордербуком в ядром
/// @param message строка json с ордербуком, полученным по kucoin websocket
void orderbook_ws_handler(const std::string &message) {
    BOOST_LOG_TRIVIAL(trace) << "Received message in private ws_stream handler: " << message;
    auto object = json::parse(message).as_object();

    if (object.at("type").as_string() == "welcome" || object.at("type").as_string() == "ack" ||
        object.at("type").as_string() == "pong")
        return;

    if (object.if_contains("subject") && object.at("subject").as_string() == "trade.ticker") {
        orderbook_channel->offer(json::serialize(json::value{{"u",        object.at("data").at("sequence")},
                                                             {"s",        "BTC-USDT"},
                                                             {"b",        object.at("data").at("bestBid")},
                                                             {"B",        object.at("data").at("bestBidSize")},
                                                             {"a",        object.at("data").at("bestAsk")},
                                                             {"A",        object.at("data").at("bestAskSize")},
                                                             {"T",        std::to_string(time(nullptr))},
                                                             {"exchange", config->exchange.name}}));
    } else {
        BOOST_LOG_TRIVIAL(error) << "Unexpected message: " << message;
        logs_channel->offer(formulate_log_message('e', 'g', 0, "Unexpected message on websocket: " + message));
    }
}

/// @brief Обработчик websocket, оформляет и отправляет сообщения с балансом по ассету в ядром
/// @param message строка json с балансом, полученным по kucoin websocket
void balance_ws_handler(const std::string &message) {

    auto object = json::parse(message).as_object();

    if (object.at("type").as_string() == "welcome" || object.at("type").as_string() == "ack" ||
        object.at("type").as_string() == "pong")
        return;

    BOOST_LOG_TRIVIAL(debug) << "Received message of balance: " << message;

    if (object.if_contains("subject") && object.at("subject") == "account.balance") {
        if (!object.at("data").at("available").as_string().empty() &&
            !object.at("data").at("currency").as_string().empty()) {
            auto balance = json::serialize(json::value{{"B", json::array(
                    {json::value{{"a", object.at("data").at("currency").as_string()},
                                 {"f", object.at("data").at("available").as_string()}}})}});
            balance_channel->offer(balance);
            BOOST_LOG_TRIVIAL(trace) << "Sent balance to core: " << balance;
        }
    } else {
        BOOST_LOG_TRIVIAL(error) << "Unexpected message: " << message;
        logs_channel->offer(formulate_log_message('e', 'g', 0, "Unexpected message on websocket: " + message));
    }
}


/// @brief Обработчик сообщений из aeron на выставление и отмену ордеров (BTC-USDT)
/// добавляет ордера в очередь, которые отсылаются в главном цикле
/// @param message string_view, строка json с ордером, полученным от ядра
void aeron_orders_handler(std::string_view message) {
    BOOST_LOG_TRIVIAL(debug) << "Received message in aeron handler: " << message;
    orders_deque.emplace_back(message);
}

/// @brief обрабатывает строку с ордером в определенном формате, их присылает ядро
/// @param message string, строка json с ордером, полученным от ядра
void handle_order_message(const std::string &order_message) {
    auto object = json::parse(order_message).as_object();
    std::string result{};

    if (object.at("a") == "+" && object.at("S") == "BTC-USDT") {
        auto id = std::to_string(get_unix_timestamp());
        result = kucoin_rest->send_order(id, "BTC-USDT", object.at("s") == "BUY" ? "buy" : "sell",
                                         object.at("t") == "LIMIT" ? "limit" : "market",
                                         format_size_precision_btc(std::string(object.at("q").as_string())),
                                         format_price_precision(std::string(object.at("p").as_string())));

        BOOST_LOG_TRIVIAL(debug) << "Sent request to create order (id" << id << ")";
        BOOST_LOG_TRIVIAL(debug) << "Result: " << result;

        auto result_object = json::parse(result).as_object();
        if (result_object.at("code") == "200000") {
            auto orderId = std::string(result_object.at("data").at("orderId").as_string());
            if (object.at("s") == "BUY") {
                buy_orders.push_back(orderId);
            } else if (object.at("s") == "SELL") {
                sell_orders.push_back(orderId);
            }
        // Если баланс недостаточный, посылаю ядру текущий баланс
        } else if (result_object.at("code") == "200004") {
            get_balance();
        }
    } else if (object.at("a") == "-" && object.at("S") == "BTC-USDT") {
        std::string id{};

        if (object.at("s") == "BUY" && !buy_orders.empty()) {
            id = buy_orders.back();
            buy_orders.pop_back();
        } else if (object.at("s") == "SELL" && !sell_orders.empty()) {
            id = sell_orders.back();
            sell_orders.pop_back();
        }

        if (!id.empty()) {
            result = kucoin_rest->cancel_order(id);
            BOOST_LOG_TRIVIAL(debug) << "Sent request to cancel order (id " << id << ")";
            BOOST_LOG_TRIVIAL(debug) << "Result: " << result;
        }
        return;
    } else {
        BOOST_LOG_TRIVIAL(error) << "Unexpected order message: " << order_message;
        logs_channel->offer(formulate_log_message('e', 'c', 0, "Unexpected message from core: " + order_message));
    }
}

/// @brief Форматирует цену BTC, округляет до десятых
/// функция просто обрезает лишние символы
/// @param price string, цена, которую нужно округлить
/// @return string, форматированная цена
std::string format_price_precision(std::string price) {
    price.resize(7, '0');
    return price;
}


/// @brief Форматирует объем BTC
/// функция просто обрезает лишние символы
/// @param size string, объем, который нужно округлить
/// @return string, форматированная цена
std::string format_size_precision_btc(std::string size) {
    size.resize(8, '0');
    return size;
}


/// @brief Форматирует объем USDT
/// функция просто обрезает лишние символы
/// @param size string, объем, который нужно округлить
/// @return string, форматированная цена
std::string format_size_precision_usdt(std::string size) {
    size.resize(10, '0');
    return size;
}

/// @brief обработчик прерывания
void sigint_handler(int) {
    running = false;
}

