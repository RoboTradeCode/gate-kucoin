#include <iostream>
#include "JsonParser.h"

JsonParser::JsonParser() {
    auto &&error_code = parser.allocate(
            0x1000,    // Размер внутреннего буфера.
            0x20);    // Максимальная глубина вложенности JSON
    if (simdjson::SUCCESS != error_code) {
        throw(InitializationException(error_code));
    }
}

bool
JsonParser::follow_path(simdjson::simdjson_result<simdjson::dom::element> &frame,
                        const std::vector<std::variant<size_t, std::string>> &path) {
    frame = current_json;
    for (const auto& jsonStep : path) {
        // проверяю, является ли jsonStep индексом массива внутри json
        if (std::holds_alternative<size_t>(jsonStep)) {
            // пробую получить элемент массива по этому индексу
            frame = frame.at(std::get<size_t>(jsonStep));

        // проверяю, является ли jsonStep именем поля
        } else if (std::holds_alternative<std::string>(jsonStep)) {
            // пробую получить поле JSON по полученному имени поля
            frame = frame[std::get<std::string>(jsonStep)];

        // Если неправильный тип (не size_t или std::string)
        } else {
            throw(std::runtime_error("Invalid type of JSON field (not std::string or size_t)"));
        }

        // проверка, получено ли поле или элемент массива
        if (simdjson::SUCCESS != frame.error())
            // если не получено, возникла ошибка
            return false;
    }
    // если всё ОК и весь путь вглубь json пройден
    return true;
}

bool JsonParser::load_json_from_string(const std::string_view &json_frames) {
    current_json = parser.parse(
            json_frames.data(),        // Указатель на разбираемый фрейм.
            json_frames.size(),        // Размер фрейма.
            false);
    if (simdjson::SUCCESS == current_json.error()) {
        return true;
    }
    return false;
}

bool JsonParser::load_json_from_file(const std::basic_string<char>& path_to_file) {
    current_json = parser.load(path_to_file);
    if (simdjson::SUCCESS == current_json.error()) {
        return true;
    }
    return false;
}

bool JsonParser::change_root_element(const std::vector<std::variant<size_t, std::string>> &path) {
    return follow_path(current_json, path);
}

bool
JsonParser::parse_string(std::string &result, const std::vector<std::variant<size_t, std::string>> &path) {
    simdjson::simdjson_result<simdjson::dom::element> element;
    // проследую пути, вглубь JSON
    // в результате ожидаю получить конкретную строку внутри JSON
    if (follow_path(element, path)) {

        // Пытаюсь преобразовать полученные элемент к строке
        auto retrieved_string = element.get_string();

        // если получилось преобразовать к строке...
        if (simdjson::SUCCESS == retrieved_string.error()) {
            // Записываю полученную строку в ответ
            result = retrieved_string.value();
            // true - т.к. строка успешно получена
            return true;
        }
    }
    // false - не удалось получить строку
    return false;
}

bool
JsonParser::parse_double(double &result, const std::vector<std::variant<size_t, std::string>> &path) {
    simdjson::simdjson_result<simdjson::dom::element> element;
    // проследую пути, вглубь JSON
    // в результате ожидаю получить конкретный double внутри JSON
    if (follow_path(element, path)) {

        // Пытаюсь преобразовать полученный элемент к double
        auto retrieved_double = element.get_double();

        // Если получилось преобразовать к double...
        if (simdjson::SUCCESS == retrieved_double.error()) {
            // Записываю полученный double в ответ
            result = retrieved_double.value();
            // true - т.к. double успешно получен
            return true;
        }
    }
    // false - не удалось получить double
    return false;
}

bool JsonParser::parse_int64_t(int64_t &result, const std::vector<std::variant<size_t, std::string>> &path) {
    simdjson::simdjson_result<simdjson::dom::element> element;
    // проследую пути, вглубь JSON
    // в результате ожидаю получить конкретный int64 внутри JSON
    if (follow_path(element, path)) {

        // Пытаюсь преобразовать полученный элемент к int64
        auto retrieved_int64 = element.get_int64();

        // Если получилось преобразовать к int64...
        if (simdjson::SUCCESS == retrieved_int64.error()) {
            // Записываю полученный int64 в ответ
            result = retrieved_int64.value();
            // true - т.к. int64 успешно получен
            return true;
        }
    }
    // false - не удалось получить int64
    return false;
}

bool JsonParser::parse_bool(bool &result, const std::vector<std::variant<size_t, std::string>> &path) {
    simdjson::simdjson_result<simdjson::dom::element> element;
    // проследую пути, вглубь JSON
    // в результате ожидаю получить конкретный bool внутри JSON
    if (follow_path(element, path)) {

        // Пытаюсь преобразовать полученный элемент к bool
        auto retrieved_bool = element.get_bool();

        // Если получилось преобразовать к bool...
        if (simdjson::SUCCESS == retrieved_bool.error()) {
            // Записываю полученный bool в ответ
            result = retrieved_bool.value();
            // true - т.к. bool успешно получен
            return true;
        }
    }
    // false - не удалось получить bool
    return false;
}
