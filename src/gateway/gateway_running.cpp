#include "gateway_running.h"
#include "gateway.h"

[[noreturn]] void run_gateway_loop() {

    using namespace std::literals::chrono_literals;

    // время последнего пинга
// веб-сокет закрывается, если его не пинговать раз в 15-30 секунд,
// для этого нужно отсчитывать, сколько времени прошло с последнего пинга
    auto last_ping_time = std::chrono::system_clock::now();


    /* Шаги работы основного цикла:
     * 1. Обработка сообщений, полученных по websocket
     * 1.1 Переподключение, если произошел обрыв соединения
     * 2. Обработка ордеров, полученных от ядра
     * 3. Проверка, пришло ли время пропинговать websocket
     * */
    BOOST_LOG_TRIVIAL(info) << "Enter the main loop";
    while (true) {
        try {

            // 1. Обработка сообщений, полученных по websocket
            io_context.run_for(std::chrono::milliseconds(100));

            // 2. Обработка ордеров, полученных от ядра
            aeron_channels.core->poll();

            // 3. Проверка, пришло ли время пропинговать websocket
            if (std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now() - last_ping_time) >
                15s) {
                BOOST_LOG_TRIVIAL(trace) << "Ping websockets, previous ping time: "
                                         << std::chrono::duration_cast<std::chrono::seconds>(
                                                 last_ping_time.time_since_epoch()).count();
                kucoin.public_ws->ping();
                kucoin.private_ws->ping();
                last_ping_time = std::chrono::system_clock::now();
            }
        }
            // 1.1 Переподключение, если произошел обрыв соединения
        catch (...) {
            BOOST_LOG_TRIVIAL(warning) << "Problem with connection with Kucoin! ";
            BOOST_LOG_TRIVIAL(warning) << "Reconnection...";

            // todo debug option
            aeron_channels.logs->offer("Reconnection with kucoin.");

            io_context.restart();

            // Подключение к Kucoin REST API и Kucoin Websocket
            connect_to_kucoin();

            BOOST_LOG_TRIVIAL(info) << "Kucoin connections established.";

            // Подписка на необходимые каналы websocket
            subscribe_to_ws_channels();

            BOOST_LOG_TRIVIAL(info) << "Connection to desired websockets established.";

            kucoin.rest->cancel_all_orders();

        }
    }
}

void startup_gateway() {


    // 2. Соединение с каналами aeron_cpp (берутся из конфига)
    BOOST_LOG_TRIVIAL(info) << "Trying to connect with aeron...";
    connect_to_aeron_channels();
    BOOST_LOG_TRIVIAL(info) << "aeron_cpp connections established.";

    // 3. Соединение с Kucoin REST API и Kucoin Websocket
    BOOST_LOG_TRIVIAL(info) << "Trying to connect with Kucoin...";
    connect_to_kucoin();
    BOOST_LOG_TRIVIAL(info) << "Kucoin connections established.";

    // 4. Подписываюсь на нужные каналы Kucoin websocket
// каналы для получения баланса и для полуения ордербука
    subscribe_to_ws_channels();
    BOOST_LOG_TRIVIAL(info) << "Connection to desired websockets established.";

    // 5. Получаю активные ордера
    BOOST_LOG_TRIVIAL(info) << "Trying to get list of active orders";
    auto active_orders = kucoin.rest->get_active_orders();
    if (!active_orders.empty()) { // todo fix empty, now is always True
        // Если есть активные ордера, логгирую их
        BOOST_LOG_TRIVIAL(info) << "List of active orders: " << active_orders;
        // todo превышает лимит пересылки сообщений aeron по символам, если есть ордера
//        logs_channel->offer(formulate_log_message(
//                'i', 'g', 0, "List of active orders: " + orders
//                ));
        // 7. Отменяю активные ордера
        kucoin.rest->cancel_all_orders();
    } else
        BOOST_LOG_TRIVIAL(info) << "No active orders";

    // 8. Отправляю текущий баланс аккаунта
    send_current_balance();
}
