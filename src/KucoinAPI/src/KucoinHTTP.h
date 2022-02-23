// todo move Kucoin classes to separate dir

#ifndef KUCOIN_GATEWAY_KUCOINHTTP_H
#define KUCOIN_GATEWAY_KUCOINHTTP_H


#include "../../HTTPSession.h"
#include <boost/json.hpp>


namespace json = boost::json;

namespace Kucoin {
    const std::string HOST = "api.kucoin.com";
    const std::string PORT = "443";

    const std::string ACCOUNTS_ENDPOINT = "/api/v1/accounts";
    const std::string PUBLIC_BULLET_ENDPOINT = "/api/v1/bullet-public";
    const std::string PRIVATE_BULLET_ENDPOINT = "/api/v1/bullet-private";
    const std::string ORDERS_ENDPOINT = "/api/v1/orders";
}
class KucoinHTTP
{
    std::shared_ptr<KucoinHTTP> https_session;
    std::string api_key;
    std::string passphrase;
    std::string secret_key;

    std::shared_ptr<boost::asio::io_context> ioc;


    std::vector<std::pair<std::string, std::string>>
    get_signatures(http::verb method, const std::string& target, const std::string& body);

public:

    explicit KucoinHTTP(std::string api_key, std::string passphrase, std::string secret_key);

    std::shared_ptr<boost::asio::io_context> get_ioc() {
        return ioc;
    }

    bool reconnect(boost::asio::io_context &ioc);

    std::string get_active_orders();

    std::string get_balance(const std::string& id);

    std::string get_accounts();

    std::string get_all_balances();

    std::string get_public_bullet();

    std::string get_private_bullet();

    std::string
    send_order(std::string clientOid, std::string symbol, std::string side, std::string type, std::string size,
               std::string price);

    std::string cancel_order(const std::string& orderId);

    std::string cancel_all_orders();
};


#endif  // KUCOIN_GATEWAY_KUCOINHTTP_H
