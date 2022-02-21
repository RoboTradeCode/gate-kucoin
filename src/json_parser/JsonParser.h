
#ifndef KUCOIN_GATEWAY_JSONPARSER_H
#define KUCOIN_GATEWAY_JSONPARSER_H

#include "/home/qod/Projects/Research/C++/KucoinGateway/libs/simdjson/simdjson.h"
#include "/home/qod/Projects/Research/C++/KucoinGateway/src/KucoinDataclasses.h"
#include "Exceptions.h"

class JsonParser {
    simdjson::dom::parser parser;
    simdjson::simdjson_result<simdjson::dom::element> current_json;

    /// @brief Структура для описания шагов парсера, т.е. когда нужно погружаться вглубь JSON
    struct JsonStep {
        size_t array_index{};    /// Индекс, если нужно обратиться к элементу массива
        std::string field{};   /// Название поля, если нужно обратиться к содержимому поля
        /// @brief Конструктор, инициализирует поле индекса, оставляет пустым поле field
        JsonStep(size_t index) : array_index(index) {}
        /// @brief Конструктор, инициализирует поле field, делает поле индекса равным 0
        JsonStep(std::string field) : array_index(0), field(std::move(field)) {}
    };

    /// @brief Проследовать по пути вглубь json
    bool follow_path(simdjson::simdjson_result<simdjson::dom::element> &frame, const std::vector<JsonStep>& path);

    /// @brief Спарсить поле из json
    bool get_field(simdjson::simdjson_result<simdjson::dom::element> &element, std::vector<JsonStep> path);


public:
    template <typename T>
    struct Result {
        std::string error;
        bool is_valid{};
        std::string last_field;
        std::size_t last_index{};
        T result;
    };

    /// @brief Конструктор, инициализирует парсер dom
    JsonParser() {
        auto &&error_code = parser.allocate(
                0x1000,    // Размер внутреннего буфера.
                0x20);    // Максимальная глубина вложенности JSON
        if (simdjson::SUCCESS != error_code) {
            throw(InitializationException(error_code));
        }
    }
    /// @brief Установить Json, который будет парсить объект
    bool set_json(const std::string_view &json_frames);

    bool move_in_depth(const std::vector<JsonStep>& path);

    /// @brief Спарсить строку из json
    bool parse_string(std::string &target_string, const std::vector<JsonStep>& path);

    bool parse_double(double &result, const std::vector<JsonStep> &path);

    /// @brief Получить поле, содержащее значение типа long
    ///
    /// @param result переменная типа long, в которую будет помещен результат. Если поле не найдено, переменная не изменится
    /// @param path путь до поля, состоящий из объектов JsonStep. Задавать в формате {{"field1"}, {"array"}, {0}, {"result_field"}}
    bool parse_long(long &result, const std::vector<JsonStep> &path);
};


#endif //KUCOIN_GATEWAY_JSONPARSER_H
