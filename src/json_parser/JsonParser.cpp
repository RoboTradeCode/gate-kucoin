#include <iostream>
#include "JsonParser.h"



Json::Parser::Parser() {
    auto &&error_code = parser.allocate(
            0x1000,    // Размер внутреннего буфера.
            0x20);    // Максимальная глубина вложенности JSON
    if (simdjson::SUCCESS != error_code) {
        throw(InitializationException(error_code));
    }
}

void
Json::Parser::follow_path(Json::Parser::Error &ec, simdjson::simdjson_result<simdjson::dom::element> &frame,
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
            ec.ec = UNEXPECTED_TYPE_IN_PATH;
            ec.message = "Unexpected type in path (not std::string or size_t)";
        }

        // проверка, получено ли поле или элемент массива
        if (simdjson::SUCCESS != frame.error()) {
            // если не получено, возникла ошибка
            ec.ec = NOT_FOUND;
            ec.message = "Non-existent JSON step in path";
        }
    }
    // если всё ОК и весь путь вглубь json пройден
    ec.ec = SUCCESS;
}

void Json::Parser::load_json_from_string(Error &ec, const std::string_view &json_frames) {
    current_json = parser.parse(
            json_frames.data(),        // Указатель на разбираемый фрейм.
            json_frames.size(),        // Размер фрейма.
            false);
    if (simdjson::SUCCESS == current_json.error()) {
        ec.ec = SUCCESS;
    } else {
        ec.ec = FAILED_MEMORY_ALLOCATE;
        ec.message = "Memory for JSON parser doesn't allocated";
    }
}

void Json::Parser::load_json_from_file(Error &ec, const std::basic_string<char> &path_to_file) {
    current_json = parser.load(path_to_file);
    if (simdjson::SUCCESS == current_json.error()) {
        ec.ec = SUCCESS;
    } else {
        ec.ec = FAILED_PARSING;
        ec.message = "Failed to parsing";
    }
}

void Json::Parser::change_root_element(Error &ec, const std::vector<std::variant<size_t, std::string>> &path) {
    return follow_path(ec, current_json, path);
}




