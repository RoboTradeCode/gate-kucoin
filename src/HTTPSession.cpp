
#include "HTTPSession.h"

HTTPSession::HTTPSession(net::io_context &ioc, const std::string &host, const std::string &port)
    : host(host)
{
    ssl::context ctx{ ssl::context::tls_client };
    ctx.set_verify_mode(ssl::context::verify_peer | ssl::context::verify_fail_if_no_peer_cert);
    ctx.set_default_verify_paths();

    tcp_stream = std::make_shared<beast::ssl_stream<beast::tcp_stream>>(ioc, ctx);

    tcp::resolver resolver(ioc);
    auto const results = resolver.resolve(host, port);
    beast::get_lowest_layer(*tcp_stream).connect(results);

    SSL_set_tlsext_host_name(tcp_stream->native_handle(), host.c_str());
    tcp_stream->handshake(ssl::stream_base::client);
}

// todo add methods for different http method (POST, GET, DELETE), make request() private
std::string HTTPSession::request(http::verb method, const std::string &target,
                                 const std::vector<std::pair<std::string, std::string>> &params,
                                 const std::string &body) {

    http::request<http::string_body> req{ method, target, 11 };
    req.set(http::field::host, host);


    for (const auto& param : params) {
        req.set(param.first, param.second);
    }

    // Если есть тело запроса, задаю его
    if (!body.empty())
        req.body() = body;

    // Отправка запроса
    http::write(*tcp_stream, req);
//    std::cout << req << std::endl;

    beast::flat_buffer buffer;
    http::response<http::string_body> res;
    http::read(*tcp_stream, buffer, res);

    return res.body();
}
