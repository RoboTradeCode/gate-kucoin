//
// Created by qod on 22.02.2022.
//

#ifndef KUCOIN_GATEWAY_KUCOINAPI_H
#define KUCOIN_GATEWAY_KUCOINAPI_H

#include "src/KucoinHTTP.h"
#include "src/KucoinWS.h"
#include "src/KucoinDataclasses.h"
#include "../json_parser/JsonParser.h"



class KucoinAPI {
//    https_session = std::make_shared<HTTPSession>(HOST, PORT, ioc);
    std::unique_ptr<KucoinHTTP> https_session;
    std::unique_ptr<KucoinWS> websocket;

    std::string api_key;
    std::string passphrase;
    std::string secret_key;


    bool InitHTTP() {
        https_session = std::make_unique<KucoinHTTP>(api_key, passphrase, secret_key);
    }
public:
    KucoinAPI(std::string api_key, std::string passphrase, std::string secret_key) {
        InitHTTP();
    }


    std::vector<Kucoin::Order> get_active_orders() {
        auto json_string = https_session->get_active_orders();
        if (std::vector<Kucoin::Order> res = parse_active_orders(json_string); ) {

        }
    }

    Kucoin::Balance get_balance(const std::string& id);

    std::vector<Kucoin::Account> get_accounts();

    std::vector<Kucoin::Account> get_all_balances();

    /// @brief Выставление ордера на биржу
    ///
    /// \param clientOid
    /// \param symbol
    /// \param side
    /// \param type
    /// \param size
    /// \param price
    /// \return
    std::string
    send_order(std::string clientOid, std::string symbol, std::string side, std::string type, std::string size,
               std::string price);


    std::string cancel_order(const std::string& orderId);

    std::string cancel_all_orders();

//    bool InitWSS() {
//        if (https_session == nullptr) {
//            return false;
//        }
//        auto string_bullet = https_session->get_private_bullet();
//        // Получаю хост и эндпоинт. Например {"ws-api.kucoin.com", /endpoint?token="2neAiuYvAU61ZDXANAGAEsEWU64B5bcOlw=="}
//        std::pair<std::string, std::string> url = parse_bullet_to_ws_endpoint(string_bullet);
//
//
//    }
};


#endif //KUCOIN_GATEWAY_KUCOINAPI_H
