#include <atomic>
#include <csignal>
#include <thread>
#include "../src/Publisher.h"

std::atomic<bool> running(true);

/**
 * Обработчик прерывания SIGINT
 */
void sigint_handler(int);

int main()
{
    Publisher publisher("aeron:ipc");

    // Главный цикл
    signal(SIGINT, sigint_handler);
    while (running)
    {
        publisher.offer("Hello, World!");
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

/**
 * Обработчик прерывания SIGINT
 */
void sigint_handler(int)
{
    running = false;
}
