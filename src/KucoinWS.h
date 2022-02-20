#ifndef KUCOIN_GATEWAY_KUCOINPUBLIC_H
#define KUCOIN_GATEWAY_KUCOINPUBLIC_H


#include <boost/beast/core.hpp>
#include <boost/json.hpp>
#include "WSSession.h"
#include "KucoinDataclasses.h"

namespace net = boost::asio;
namespace json = boost::json;

class KucoinWS
{
    std::shared_ptr<WSSession> ws;

public:
    explicit KucoinWS(boost::asio::io_context& ioc, const Bullet& bullet,
                      const std::function<void(std::string)>& event_handler);
    // todo add functions for subscribe to specific channel

    void subscribe_to_channel(const std::string &endpoint, int id, bool is_private_channel);

    void ping();
};


#endif  // KUCOIN_GATEWAY_KUCOINPUBLIC_H
