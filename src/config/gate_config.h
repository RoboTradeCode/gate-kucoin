//
// Header-only класс для чтения конфига шлюза.
//
#pragma once
#include <iostream>
#include <filesystem>
#include <stdexcept>

#include "aeron_channel.h"

#include "../toml++/toml.h"

struct gate_config{
    struct {
        std::string name{};
    } exchange;




    struct{
        std::string api_key{};
        std::string secret_key{};
        std::string passphrase{};

        std::vector<std::string> ids;
    } account;

    struct {
        // Настройки относящиеся к отправке данных
        struct {
            // канала для отправки order_book(маркет дата)
            aeron_channel orderbook{};
            // канала для отправки логов работы шлюза
            aeron_channel logs;
            // канала для отправки баланса аккаунта
            aeron_channel balance;
        } publishers;

        // Настройки относящиеся к приему данных
        struct {
            // канал в котором приходят команды от ядра.
            aeron_channel core;
        } subscribers;
    } aeron;

    // вызовем конструктор родительского класса
    explicit gate_config(const std::string& config_file_path_){
        // проверим валидность указанного пути
        namespace fs = std::filesystem;
        fs::path path(config_file_path_);

        if (not fs::exists(path) && not fs::is_regular_file(path)){
            throw std::invalid_argument("File " + config_file_path_ + " doesn't exists");
        }

        // получим дерево конфига
        auto config = toml::parse_file(config_file_path_);

        exchange.name = config["exchange"]["name"].value_or("");

        account.api_key = config["account"]["api_key"].value_or("");
        account.secret_key = config["account"]["secret_key"].value_or("");
        account.passphrase = config["account"]["passphrase"].value_or("");

        get_vector(account.ids, config["account"]["ids"]);

        aeron.publishers.orderbook.channel = config["aeron"]["publishers"]["orderbook"][0].value_or("");
        aeron.publishers.orderbook.stream_id = config["aeron"]["publishers"]["orderbook"][1].value_or(0);

        aeron.publishers.logs.channel = config["aeron"]["publishers"]["logs"][0].value_or("");
        aeron.publishers.logs.stream_id = config["aeron"]["publishers"]["logs"][1].value_or(0);
        aeron.publishers.balance.channel = config["aeron"]["publishers"]["balance"][0].value_or("");
        aeron.publishers.balance.stream_id = config["aeron"]["publishers"]["balance"][1].value_or(0);

        aeron.subscribers.core.channel = config["aeron"]["subscribers"]["core"][0].value_or("");
        aeron.subscribers.core.stream_id = config["aeron"]["subscribers"]["core"][1].value_or(0);
    }

    /// Считаем список элементов
    static bool get_vector(std::vector<std::string>& vector,       ///< вектор куда будем записывать
                           toml::node_view<toml::node> config_node){     ///< узел конфига от куда будем читать
        // проверим что указан массив(список) и то, что он содержит данные одного типа - is_homogeneous()
        if (toml::array* array = config_node.as_array(); array->is_homogeneous())
            // добавив в список destinations не пустые элементы списка
            for (toml::node &elem: *array)
                if(std::string item_value = elem.value_or(""); not item_value.empty())
                    vector.emplace_back(item_value);

        return true;
    }

};