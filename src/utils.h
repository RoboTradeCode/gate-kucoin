#ifndef KUCOIN_GATEWAY_UTILS_H
#define KUCOIN_GATEWAY_UTILS_H

#include <array>
#include <openssl/hmac.h>
#include <boost/beast.hpp>

std::string base64_hmac_sha256(const std::string& message, const std::string& secret_key);

// получение текущего unix timestamp в милисекундах
long get_unix_timestamp();


#endif  // KUCOIN_GATEWAY_UTILS_H
