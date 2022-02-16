//
// Created by qod on 03.02.2022.
//

#ifndef OKX_GATEWAY_URI_H
#define OKX_GATEWAY_URI_H


#include <string>

struct Uri
{
public:
    std::string QueryString, Path, Protocol, Host, Port;

    static Uri Parse(const std::string &uri);
};



#endif //OKX_GATEWAY_URI_H
