
#ifndef KUCOIN_GATEWAY_JSONPARSER_H
#define KUCOIN_GATEWAY_JSONPARSER_H

#include <variant>
#include "/home/qod/Projects/Research/C++/KucoinGateway/libs/simdjson/simdjson.h"
#include "../KucoinAPI/src/KucoinDataclasses.h"
#include "Exceptions.h"


/// @brief Парсер JSON, настраивается на файл или строку и получает поля из неё. Работает в режиме dom.
///
/// @details Использует simdjson dom parser. Можно менять JSON, который будет парсить JsonParser, не создавая нового
/// объекта класса.
class JsonParser {
private:
    simdjson::dom::parser parser;
    simdjson::simdjson_result<simdjson::dom::element> current_json;

    /// @brief Получить элемент внутри JSON
    bool follow_path(simdjson::simdjson_result<simdjson::dom::element> &frame, const std::vector<std::variant<size_t, std::string>>& path);

public:

    /// @brief Конструктор, инициализирует парсер dom
    JsonParser();

    /// @brief Установить JSON, который будет парсить объект из строки
    ///
    /// @param json_frames строка JSON
    bool load_json_from_string(const std::string_view &json_frames);

    /// @brief Установить JSON, cпарсить объект из файла
    ///
    /// @param path_to_file путь до файла
    bool load_json_from_file(const std::basic_string<char>& path_to_file);

    /// @brief Сделать корневым элементом JSON другой элемент внутри текущего
    /// т.е. при поиске полей поиск будет вестись от нового корневого элемента
    bool change_root_element(const std::vector<std::variant<size_t, std::string>>& path);

    /// @brief Получить поле, содержащее значение типа std::string
    ///
    /// @param result переменная типа std::string, в которую будет помещен результат. Если поле не найдено, переменная не изменится
    /// @param path путь до поля, включающий название поля. Задавать в формате {"field1", "array", 0, "result_field"}
    /// т.е. массив из std::string и size_t. std::string это название поля, size_t это индекс в массиве
    ///
    /// @result true, если успешно получено поле. false, если поле не получено (произошла ошибка)
    bool parse_string(std::string &target_string, const std::vector<std::variant<size_t, std::string>>& path);

    /// @brief Получить поле, содержащее значение типа double
    ///
    /// @param result переменная типа double, в которую будет помещен результат. Если поле не найдено, переменная не изменится
    /// @param path путь до поля, включающий название поля. Задавать в формате {"field1", "array", 0, "result_field"}
    /// т.е. массив из std::string и size_t. std::string это название поля, size_t это индекс в массиве
    ///
    /// @result true, если успешно получено поле. false, если поле не получено (произошла ошибка)
    bool parse_double(double &result, const std::vector<std::variant<size_t, std::string>> &path);

    /// @brief Получить поле, содержащее значение типа long
    ///
    /// @param result переменная типа long, в которую будет помещен результат. Если поле не найдено, переменная не изменится
    /// @param path путь до поля, включающий название поля. Задавать в формате {"field1", "array", 0, "result_field"}
    /// т.е. массив из std::string и size_t. std::string это название поля, size_t это индекс в массиве
    ///
    /// @result true, если успешно получено поле. false, если поле не получено (произошла ошибка)
    bool parse_int64_t(int64_t &result, const std::vector<std::variant<size_t, std::string>> &path);

    /// @brief Получить поле, содержащее значение типа bool
    ///
    /// @param result переменная типа bool, в которую будет помещен результат. Если поле не найдено, переменная не изменится
    /// @param path путь до поля, включающий название поля. Задавать в формате {"field1", "array", 0, "result_field"}
    /// т.е. массив из std::string и size_t. std::string это название поля, size_t это индекс в массиве
    ///
    /// @result true, если успешно получено поле. false, если поле не получено (произошла ошибка)
    bool parse_bool(bool &result, const std::vector<std::variant<size_t, std::string>> &path);
};


#endif //KUCOIN_GATEWAY_JSONPARSER_H
