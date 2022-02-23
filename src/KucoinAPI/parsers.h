#ifndef KUCOIN_GATEWAY_PARSERS_H
#define KUCOIN_GATEWAY_PARSERS_H

#include "../json_parser/JsonParser.h"

/// @brief Получить host и path из bullet JSON
///
/// \param json строка, содержащая Json
/// \return std::pair{host, endpoint} хост и эндпоинт. Эндпоинт в формате {path}?token={token}
/// Например {"ws-api.kucoin.com", /endpoint?token="2neAiuYvAU61ZDXANAGAEsEWU64B5bcOlw=="}
std::pair<std::string, std::string> parse_bullet_to_ws_endpoint(std::string_view json) {
    Json::Parser parser;
    Json::Error ec;

    // Распарсить json
    parser.load_json_from_string(ec, json);

    // если успешно распаршен
    if (ec.ec == Json::SUCCESS) {
        // Меняю элемент, от которого буду вести отсчет вглубь JSON
        parser.change_root_element(ec, {"data"});
        // Получаю поле с эндпоинтом
        auto endpoint = parser.get_string_field(ec, {"instanceServers", 0, "endpoint"});
        // если поле с эндпоинтом успешно получено
        if (ec.ec == Json::SUCCESS) {
            // Распарсить эндпоинт (нужно отделить хост и сам эндпоинт)
            Uri uri = Uri::parse(endpoint);
            // Получаю токен веб-сокета
            auto token = parser.get_string_field(ec, {"token"});
            // Если токен успешно получен
            if (ec.ec == Json::SUCCESS) {
                // возвращаю std::pair в формате {хост, эндпоинт?=токен}
                return {uri.host, {uri.path + "?token=" + token}};
            }
        }
    }
    return {};
}

#include "src/KucoinHTTP.h"
#include "src/KucoinWS.h"
#include "src/KucoinDataclasses.h"
#include "../json_parser/JsonParser.h"

#endif //KUCOIN_GATEWAY_PARSERS_H
