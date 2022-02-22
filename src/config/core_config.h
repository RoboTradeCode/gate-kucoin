//
// Created by user on 02.02.2022.
//
#pragma once
#include <iostream>
#include <filesystem>
#include <vector>

#include "aeron_channel.h"

#include "../toml++/toml.h"

struct core_config{
    struct{
        // данные которые принимает ядро
        struct{
            //от куда и в каком режиме принимаем ордербуки
            struct {
                //# subscription - это то, что передается в констукртор subscriber
                //# new AeronSubscriber('handler', 'aeron:udp?control-mode=manual');
                aeron_channel subscription;
                // destinations - тут указывается список publishers'ов от которых мы будем принимать данные.
                // $subscriber->addDestination("aeron:udp?endpoint=172.31.4.173:40457|control=172.31.4.173:40456");
                // список указывается вот так - ["channel_11", "channel_2"],
                std::vector<std::string> destinations;
            } orderbooks;

            //от куда и в каком режиме принимаем балансы
            struct {
                aeron_channel subscription;
                std::vector<std::string> destinations;
            } balances;

        } subscribers;

        // данные которые ядро отправляет
        struct{
            // канал в котором передаются ордера(команды) от ядра шлюзу.
            aeron_channel commands;
            // канал в который отправляем логи работы шлюза (ошибки в работе, оповещения и т.д.)
            aeron_channel logs;
            // канал в который отправляем статистические данные(метрики)
            aeron_channel metrics;
        } publishers;
    } aeron;

    explicit core_config(const std::string& config_file_path_){
        // проверим валидность указанного пути
        namespace fs = std::filesystem;
        fs::path path(config_file_path_);

        if (not fs::exists(path) && not fs::is_regular_file(path))
            throw std::invalid_argument("File " + config_file_path_ + " doesn't exists");

        // получим дерево конфига
        auto config = toml::parse_file(config_file_path_);

        /// - считаем настройки необходимые для приема данных

        /// 1. прием orderbooks
        aeron.subscribers.orderbooks.subscription.channel = config["aeron"]["subscribers"]["orderbooks"]["subscription"][0].value_or("");
        aeron.subscribers.orderbooks.subscription.stream_id = config["aeron"]["subscribers"]["orderbooks"]["subscription"][1].value_or(0);
        // прочитаем узлы к которым подключимся для получения данных
        get_destinations(aeron.subscribers.orderbooks.destinations, config["aeron"]["subscribers"]["orderbooks"]["destinations"]);

        /// 2. прием балансов
        aeron.subscribers.balances.subscription.channel = config["aeron"]["subscribers"]["balances"]["subscription"][0].value_or("");
        aeron.subscribers.balances.subscription.stream_id = config["aeron"]["subscribers"]["balances"]["subscription"][1].value_or(0);
        // прочитаем узлы к которым подключимся для получения данных
        get_destinations(aeron.subscribers.balances.destinations, config["aeron"]["subscribers"]["balances"]["destinations"]);

        /// - считаем настройки необходимые для отправки данных
        /// 1. канал в который будем передавать команды ордера(команды) от ядра шлюзу.
        aeron.subscribers.balances.subscription.channel = config["aeron"]["subscribers"]["balances"]["subscription"][0].value_or("");

    }

    /// Считаем список destinations
    static bool get_destinations(std::vector<std::string>& destinations,       ///< вектор куда будем записывать
                                 toml::node_view<toml::node> config_node){     ///< узел конфига от куда будем читать
        // проверим что указан массив(список) и то, что он содержит данные одного типа - is_homogeneous()
        if (toml::array* array = config_node.as_array(); array->is_homogeneous())
            // добавив в список destinations не пустые элементы списка
            for (toml::node &elem: *array)
                if(std::string item_value = elem.value_or(""); not item_value.empty())
                    destinations.emplace_back(item_value);

        return true;
    }
};

