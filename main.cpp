#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>
#include "src/kucoin_api/kucoin_structures.h"

#include "gateway_running.h"
#include "gateway.h"

#include "KucoinWS.h"
#include "KucoinREST.h"


namespace logging = boost::log;

int main() {
    /* Алгоритм работы шлюза:
     * 1. Загрузка конфигурации шлюза
     * 2. Запуск шлюза
     * 2.1 Соединение с каналами Aeron
     * 2.2 Соединение с Kucoin REST API
     * 2.3 Соединение с Kucoin websocket (Public and Private)
     * 2.4 Подписка на нужные каналы websocket
     * 2.5 Отправка списка активных ордеров и их отмена
     * 2.6 Отправка текущего баланса
     * 3. Вход в цикл обработки данных от ядра (выставление ордеров)
     * и обработки данных от Kucoin. При обрыве соединения автоматически
     * переподключается к Kucoin
     * */

    // Устанавливаю уровень логгирования
    logging::core::get()->set_filter
            (       // для самого подробного логгирования установить logging::trivial::trace
                    logging::trivial::severity >= logging::trivial::trace
            );

    BOOST_LOG_TRIVIAL(info) << "Load configuration...";

    // 1. Получение настроек конфигурации шлюза
    // Указать корректный путь до конфига
    load_configuration("../kucoin_config.toml");

//    2. Запуск шлюза
//    * 2.1 Соединение с каналами Aeron
//    * 2.2 Соединение с Kucoin REST API
//    * 2.3 Соединение с Kucoin websocket (Public and Private)
//    * 2.4 Подписка на нужные каналы websocket
//    * 2.5 Отправка списка активных ордеров и их отмена
//    * 2.6 Отправка текущего баланса
    startup_gateway();

    // 3. Вход в цикл обработки данных от ядра (выставление ордеров)
    // и обработки данных от Kucoin. При обрыве соединения автоматически
    // переподключается к Kucoin
    run_gateway_loop();

    BOOST_LOG_TRIVIAL(info) << "Exit the main loop";
    return EXIT_SUCCESS;
}