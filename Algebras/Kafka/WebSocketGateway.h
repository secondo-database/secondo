/*
----
This file is part of SECONDO.

Copyright (C) 2015,
Faculty of Mathematics and Computer Science,
Database Systems for New Applications.

SECONDO is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

SECONDO is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with SECONDO; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
----

*/

#ifndef KAFKA_WEBSOCKETGATEWAY_H
#define KAFKA_WEBSOCKETGATEWAY_H

#include <iostream>
#include <websocketpp/config/asio_client.hpp>
#include "WebSocketClientPrototype.h"

enum ClientType {
    UNDEFINED,
    MOCK,
    NO_TLS,
    TLS
};

class WebSocketGateway {
public:
    ErrorCode Open(std::string uri);
    void Subscribe(std::string body);

    std::string ReadSting();

    void Close();
private:
    ClientType clientType;
    WebsocketClientPPB<websocketpp::config::asio_tls_client> tls_client;
    WebsocketClientPPB<websocketpp::config::asio_client> no_tls_client;
};


#endif //KAFKA_WEBSOCKETGATEWAY_H
