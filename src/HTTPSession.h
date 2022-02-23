#ifndef KUCOIN_GATEWAY_HTTPSESSION_H
#define KUCOIN_GATEWAY_HTTPSESSION_H

#include <iostream>
#include <boost/beast/http.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/log/trivial.hpp>
#include "utils.h"

namespace net = boost::asio;
namespace ssl = boost::asio::ssl;
namespace beast = boost::beast;
namespace http = beast::http;
using tcp = boost::asio::ip::tcp;

class HTTPSession
{
    std::string host;
    std::shared_ptr<beast::ssl_stream<beast::tcp_stream>> tcp_stream;
public:
    explicit HTTPSession(net::io_context &ioc, const std::string &host, const std::string &port);

    std::string request(http::verb method, const std::string &target,
                        const std::vector<std::pair<std::string, std::string>> &params = {},
                        const std::string &body = {});
};


#endif  // KUCOIN_GATEWAY_HTTPSESSION_H
