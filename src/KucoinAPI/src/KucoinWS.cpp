#include "KucoinWS.h"

KucoinWS::KucoinWS(net::io_context& ioc, std::pair<std::string, std::string> url, const std::function<void(std::string)>& event_handler)
{
    ws = std::make_shared<WSSession>(url.first, "443", url.second, ioc, event_handler);
    ws->async_read();
}



// todo make subscribe_to_channel private
void KucoinWS::subscribe_to_channel(const std::string& endpoint, int id, bool is_private_channel=false)
{                     //Whether the server needs to return the receipt information of this subscription or not. Set as false by default.
//}
    ws->write(json::serialize(json::value{
        { "id", id },
        { "type", "subscribe"},
        { "topic", endpoint},  //Topic needs to be subscribed. Some topics support to divisional subscribe the informations of multiple trading pairs through ",".
        { "privateChannel", is_private_channel ? "true" : "false"},                      //Adopted the private channel or not. Set as false by default.
        { "response", true}
    }));
}

void KucoinWS::ping() {
    ws->write(json::serialize(json::value{
            {"type", "ping"}
    }));
}
