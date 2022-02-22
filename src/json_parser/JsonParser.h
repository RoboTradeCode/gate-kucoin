
#ifndef KUCOIN_GATEWAY_JSONPARSER_H
#define KUCOIN_GATEWAY_JSONPARSER_H

#include <variant>
#include "/home/qod/Projects/Research/C++/KucoinGateway/libs/simdjson/simdjson.h"
#include "../KucoinAPI/src/KucoinDataclasses.h"
#include "Exceptions.h"

namespace Json {

    struct Error;

/// @brief Парсер JSON, настраивается на файл или строку и получает поля из неё. Работает в режиме dom.
///
/// @details Использует simdjson dom parser. Можно менять JSON, который будет парсить Parser, не создавая нового
/// объекта класса.
    class Parser {
    private:
        simdjson::dom::parser parser;
        simdjson::simdjson_result<simdjson::dom::element> current_json;

    public:

        enum error_code {
            SUCCESS,
            FAILED_MEMORY_ALLOCATE,
            FAILED_PARSING,
            NOT_FOUND,
            INVALID_TYPE,
            UNEXPECTED_TYPE_IN_PATH
        };

        struct Error {
            error_code ec;
            std::string message;
        };

        /// @brief Конструктор, инициализирует парсер dom
        Parser();


        /// @brief Получить элемент внутри JSON
        ///
        /// @param ec структура типа Parser::Error, в которую будет записана ошибка, или SUCCESS в случае успеха
        /// содержит два поля: ec и message. ec содержит значение enum Parser::error_code,  message содержит std::string
        /// с текстом ошибки
        /// @param frame элемент json
        /// @param path путь до поля, включающий название поля. Задавать в формате {"field1", "array", 0, "result_field"}
        /// т.е. массив из std::string и size_t. std::string это название поля, size_t это индекс в массиве
        void follow_path(Error &ec, simdjson::simdjson_result<simdjson::dom::element> &frame,
                         const std::vector<std::variant<size_t, std::string>> &path);
        /// @brief Установить JSON, который будет парсить объект из строки
        ///
        /// @param ec структура типа Parser::Error, в которую будет записана ошибка, или SUCCESS в случае успеха
        /// содержит два поля: ec и message. ec содержит значение enum Parser::error_code,  message содержит std::string
        /// с текстом ошибки
        /// @param json_frames строка JSON
        void load_json_from_string(Error &ec, const std::string_view &json_frames);

        /// @brief Установить JSON, cпарсить объект из файла
        ///
        /// @param ec структура типа Parser::Error, в которую будет записана ошибка, или SUCCESS в случае успеха
        /// содержит два поля: ec и message. ec содержит значение enum Parser::error_code,  message содержит std::string
        /// с текстом ошибки
        /// @param path_to_file путь до файла
        void load_json_from_file(Error &ec, const std::basic_string<char> &path_to_file);

        /// @brief Сделать корневым элементом JSON другой элемент внутри текущего
        /// т.е. при поиске полей поиск будет вестись от нового корневого элемента
        ///
        /// @param ec структура типа Parser::Error, в которую будет записана ошибка, или SUCCESS в случае успеха
        /// содержит два поля: ec и message. ec содержит значение enum Parser::error_code,  message содержит std::string
        /// с текстом ошибки
        /// @param path путь до поля, включающий название поля. Задавать в формате {"field1", "array", 0, "result_field"}
        /// т.е. массив из std::string и size_t. std::string это название поля, size_t это индекс в массиве
        void change_root_element(Error &ec, const std::vector<std::variant<size_t, std::string>> &path);

        /// @brief Получить поле, которое находится внутри JSON
        /// Для получения строки указать тип <const char *>
        /// @param ec структура типа Parser::Error, в которую будет записана ошибка, или SUCCESS в случае успеха
        /// содержит два поля: ec и message. ec содержит значение enum Parser::error_code,  message содержит std::string
        /// с текстом ошибки
        /// @param path путь до поля, включающий название поля. Задавать в формате {"field1", "array", 0, "result_field"}
        /// т.е. массив из std::string и size_t. std::string это название поля, size_t это индекс в массиве
        ///
        /// @result переменная типа std::string, полученная из JSON
        template<typename Type>
        Type parse_field(Error &ec, const std::vector<std::variant<size_t, std::string>> &path) {
            simdjson::simdjson_result<simdjson::dom::element> element;
            // проследую пути, вглубь JSON
            // в результате ожидаю получить конкретное поле внутри JSON
            follow_path(ec, element, path);

            if (ec.ec == SUCCESS) {

                // Пытаюсь преобразовать полученное поле к целевому типу
                auto retrieved_value = element.template get<Type>();


                // если получилось преобразовать к целевому типу...
                if (simdjson::SUCCESS == retrieved_value.error()) {
                    ec.ec = SUCCESS;
                    // Возвращаю полученное значение
                    return retrieved_value.value();
                } else {
                    ec.ec = INVALID_TYPE;
                    ec.message = "Failed converting JSON field";
                }
            }
            // не удалось получить поле
            return {};
        }
    };


}

#endif //KUCOIN_GATEWAY_JSONPARSER_H
