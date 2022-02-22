#include "KucoinREST.h"


KucoinREST::KucoinREST(boost::asio::io_context &ioc, std::string api_key, std::string passphrase, std::string secret_key)
    : api_key(std::move(api_key)),
      passphrase(std::move(passphrase)),
      secret_key(std::move(secret_key))
{
    https_session = std::make_shared<HTTPSession>(HOST, PORT, ioc);
}


bool KucoinREST::reconnect(boost::asio::io_context &ioc) {
    try {
        https_session = std::make_shared<HTTPSession>(HOST, PORT, ioc);
        return true;
    }
    catch (...) {
        return false;
    }
}


std::string KucoinREST::get_active_orders()
{
    auto endpoint = ORDERS_ENDPOINT + "?status=active";
    auto method = http::verb::get;

    auto signature_params = get_signatures(method, endpoint, {});

    return https_session->request(method,
                                  endpoint,
                                  signature_params);
}

std::string KucoinREST::get_balance(const std::string& id)
{
    auto endpoint = ACCOUNTS_ENDPOINT + '/' + id;
    auto method = http::verb::get;

    auto signature_params = get_signatures(method, endpoint, {});

    return https_session->request(method,
                                  endpoint,
                                  signature_params);
}


std::string KucoinREST::get_accounts() {
    auto endpoint = "/api/v1/accounts";
    auto method = http::verb::get;

    auto signature_params = get_signatures(method, endpoint, {});

    return https_session->request(method,
                                  endpoint,
                                  signature_params);
}


std::string KucoinREST::get_public_bullet()
{
    auto endpoint = PUBLIC_BULLET_ENDPOINT;
    auto method = http::verb::post;

    return https_session->request(http::verb::post,
                                  PUBLIC_BULLET_ENDPOINT);
}

std::string KucoinREST::get_private_bullet()
{
    auto endpoint = PRIVATE_BULLET_ENDPOINT;
    auto method = http::verb::post;

    auto signature_params = get_signatures(method, endpoint, {});

    return https_session->request(method,
                                  endpoint,
                                  signature_params);
}

std::string KucoinREST::send_order(
            std::string clientOid, 
            std::string symbol, 
            std::string side, 
            std::string type, 
            std::string size,
            std::string price)
            {

    auto endpoint = ORDERS_ENDPOINT;
    auto method = http::verb::post;
    auto body = json::serialize(json::value{
            {"clientOid",   clientOid},
            {"side",        side},
            {"symbol",      symbol},
            {"type",        type},
            {"price",       price},
            {"size",        size}
    });

    std::vector<std::pair<std::string, std::string>> params = {
                                               {"Content-Type", "application/json"},
                                               {"Accept", "application/json"},
                                               {"Content-Length", std::to_string(body.length())},
                                               {"Connection", "keep-alive"}
    };

    auto signature_params = get_signatures(method, endpoint, body);
    params.insert(std::end(params), std::begin(signature_params), std::end(signature_params));

    return https_session->request(method,
                                  endpoint,
                                  params, body);
}

std::string KucoinREST::cancel_order(const std::string& orderId) {
    auto endpoint = ORDERS_ENDPOINT + '/' + orderId;
    auto method = http::verb::delete_;

    auto signature_params = get_signatures(method, endpoint, {});

    return https_session->request(method,
                                  endpoint,
                                  signature_params);
}

std::string KucoinREST::cancel_all_orders() {
    auto endpoint = ORDERS_ENDPOINT;
    auto method = http::verb::delete_;

    auto signature_params = get_signatures(method, endpoint, {});

    return https_session->request(method,
                                  endpoint,
                                  signature_params);
}

std::vector<std::pair<std::string, std::string>>
KucoinREST::get_signatures(http::verb method, const std::string& target, const std::string& body) {

    auto timestamp = std::to_string(get_unix_timestamp());
    auto method_as_string = std::string{to_string(method)};
    auto sign = base64_hmac_sha256(timestamp + method_as_string + target + body, secret_key);
    auto encipher_passphrase = base64_hmac_sha256(passphrase, secret_key);


    std::vector<std::pair<std::string, std::string>> signature_params {
            {"KC-API-KEY", api_key},
            {"KC-API-SIGN", sign},
            {"KC-API-TIMESTAMP", timestamp},
            {"KC-API-KEY-VERSION", "2"},
            {"KC-API-passphrase", encipher_passphrase}
    };

    return signature_params;
}


