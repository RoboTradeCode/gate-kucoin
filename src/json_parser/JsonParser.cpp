#include "JsonParser.h"

bool
JsonParser::follow_path(simdjson::simdjson_result<simdjson::dom::element> &frame, const std::vector<JsonStep> &path) {
    frame = current_json;
    JsonParser::Result<simdjson::simdjson_result<simdjson::dom::element>> result;
    for (const auto& jsonStep : path) {
        if (frame = {jsonStep.field.empty() ? frame.at(jsonStep.array_index) : frame[jsonStep.field]};
                simdjson::SUCCESS != frame.error())
            return false;
    }
    return true;
}

bool JsonParser::get_field(simdjson::simdjson_result<simdjson::dom::element> &element, std::vector<JsonStep> path) {
    // Разбираемые фреймы json.
    follow_path(element, path);
    if ( simdjson::SUCCESS == element.error()) {
        return true;
    }
    return false;
}

bool JsonParser::set_json(const std::string_view &json_frames) {
    current_json = parser.parse(
            json_frames.data(),        // Указатель на разбираемый фрейм.
            json_frames.size(),        // Размер фрейма.
            false);
    if (simdjson::SUCCESS != current_json.error()) {
        return false;
    }
    return true;
}

bool
JsonParser::parse_string(std::string &target_string, const std::vector<JsonStep> &path) {
    simdjson::simdjson_result<simdjson::dom::element> element;
    // Разбираемые фреймы json.

    get_field(element, path);

    auto result = element.get_string();

    if ( simdjson::SUCCESS == result.error()) {
        // Получим значение.
        target_string = result.value();
        return true;
    }
    // Ошибка разбора данных, данные в структуре не действительные.
    return false;
}

bool
JsonParser::parse_double(double &result, const std::vector<JsonStep> &path) {
    simdjson::simdjson_result<simdjson::dom::element> element;
    // Разбираемые фреймы json.

    get_field(element, path);

    auto result_field = element.get_double();

    if ( simdjson::SUCCESS == result_field.error()) {
        // Получим значение.
        result = result_field.value();
        return true;
    }
    // Ошибка разбора данных, данные в структуре не действительные.
    return false;
}

bool JsonParser::parse_long(long &result, const std::vector<JsonStep> &path) {
    simdjson::simdjson_result<simdjson::dom::element> element;
    // Разбираемые фреймы json.

    get_field(element, path);

    auto result_field = element.get_int64();

    if ( simdjson::SUCCESS == result_field.error()) {
        // Получим значение.
        result = result_field.value();
        return true;
    }
    // Ошибка разбора данных, данные в структуре не действительные.
    return false;
}

bool JsonParser::move_in_depth(const std::vector<JsonStep> &path) {
    return get_field(current_json, path);
}
