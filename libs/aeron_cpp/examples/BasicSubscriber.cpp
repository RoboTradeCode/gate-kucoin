#include <atomic>
#include <csignal>
#include <memory>
#include <string>
#include <Aeron.h>
#include "../src/Subscriber.h"

std::atomic<bool> running(true);

/**
 * Функция обратного вызова для обработки каждого сообщения
 *
 * @param message Сообщение
 */
void handler(std::string_view message);

/**
 * Обработчик прерывания SIGINT
 */
void sigint_handler(int);

int main()
{
    std::shared_ptr<Subscriber> subscriber = std::make_shared<Subscriber>(handler, "aeron:ipc");

    // Стратегия ожидания
    aeron::SleepingIdleStrategy idle_strategy(std::chrono::milliseconds(1));

    // Главный цикл
    signal(SIGINT, sigint_handler);
    while (running)
        idle_strategy.idle(subscriber->poll());
}

/**
 * Функция обратного вызова для обработки каждого сообщения
 *
 * @param message Сообщение
 */
void handler(std::string_view message)
{
    std::cout << message << std::endl;
}

/**
 * Обработчик прерывания SIGINT
 */
void sigint_handler(int)
{
    running = false;
}
