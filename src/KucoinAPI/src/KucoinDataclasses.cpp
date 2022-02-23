//
// Created by qod on 02.02.2022.
//

#include "KucoinDataclasses.h"

#include <utility>

Uri Uri::parse(std::string_view uri) {

    Uri result;

    if (uri.length() == 0)
        return result;

    auto uriEnd = uri.end();

    // get query start
    auto queryStart = std::find(uri.begin(), uriEnd, '?');

    // protocol
    auto protocolStart = uri.begin();
    auto protocolEnd = std::find(protocolStart, uriEnd, ':');            //"://");

    if (protocolEnd != uriEnd)
    {
        std::string prot = &*(protocolEnd);
        if ((prot.length() > 3) && (prot.substr(0, 3) == "://"))
        {
            result.protocol = std::string(protocolStart, protocolEnd);
            protocolEnd += 3;   //      ://
        }
        else
            protocolEnd = uri.begin();  // no protocol
    }
    else
        protocolEnd = uri.begin();  // no protocol

    // host
    auto hostStart = protocolEnd;
    auto pathStart = std::find(hostStart, uriEnd, '/');  // get pathStart

    auto hostEnd = std::find(protocolEnd,
                                   (pathStart != uriEnd) ? pathStart : queryStart,
                                   ':');  // check for port

    result.host = std::string(hostStart, hostEnd);

    // port
    if ((hostEnd != uriEnd) && ((&*(hostEnd))[0] == ':'))  // we have a port
    {
        hostEnd++;
        auto portEnd = (pathStart != uriEnd) ? pathStart : queryStart;
        result.port = std::string(hostEnd, portEnd);
    }

    // path
    if (pathStart != uriEnd)
        result.path = std::string(pathStart, queryStart);

    // query
    if (queryStart != uriEnd)
        result.queryString = std::string(queryStart, uri.end());

    return result;

}