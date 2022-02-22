//
// Created by qod on 22.02.2022.
//

#ifndef KUCOIN_GATEWAY_KUCOINAPI_H
#define KUCOIN_GATEWAY_KUCOINAPI_H


#include "src/KucoinREST.h"
#include "src/KucoinWS.h"

class KucoinAPI {
    std::unique_ptr<KucoinREST> rest_api;
    std::unique_ptr<KucoinWS> websocket;

    std::unique_ptr<boost::asio::io_context> ioc;

    std::string api_key;
    std::string passphrase;
    std::string secret_key;

    bool InitHTTP() {
        ioc = std::make_unique<boost::asio::io_context>();
        rest_api = std::make_unique<KucoinREST>(*ioc, api_key, passphrase, secret_key);
    }

    bool InitWSS() {
        if (rest_api == nullptr) {
            return false;
        }
        auto string_bullet = rest_api->get_private_bullet();
        Bullet bullet = Bullet(string_bullet);
    }
public:
};


#endif //KUCOIN_GATEWAY_KUCOINAPI_H
