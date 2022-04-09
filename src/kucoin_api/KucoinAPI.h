//
// Created by qod on 18.02.2022.
//

#ifndef KUCOIN_GATEWAY_KUCOINAPI_H
#define KUCOIN_GATEWAY_KUCOINAPI_H


#include <string>



class KucoinAPI {

public:
    KucoinAPI(const std::string &api_key, const std::string &secret_key, const std::string &passphrase);

};


#endif //KUCOIN_GATEWAY_KUCOINAPI_H
