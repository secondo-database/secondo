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

#include "WebSocketGateway.h"
#include "log.hpp"

#define USE_MOCK_DATA 1


ErrorCode WebSocketGateway::Open(std::string uri) {
    LOG(DEBUG) << "WebSocketGateway: Connecting to " << uri;
    if (uri.rfind("mock://data", 0)==0) {
        clientType = MOCK;
        return OK;
    } else if (uri.rfind("wss://", 0)==0) {
        clientType = TLS;
        return tls_client.connect(uri);
    } else if (uri.rfind("ws://", 0)==0) {
        clientType = NO_TLS;
        return no_tls_client.connect(uri);
    } else {
        clientType = UNDEFINED;
        return CONNECT_FAILED;
    }
}

void WebSocketGateway::Subscribe(std::string body) {
    LOG(DEBUG) << "WebSocketGateway: Sending subscribe request " << body;
    switch(clientType) {
        case MOCK :
            break;
        case TLS :
            tls_client.send(body);
            break;
        case NO_TLS :
            no_tls_client.send(body);
            break;
        default:
            break;
    }
}


std::string data = R"(
    {
       "application": "hiking",
       "reputons": [
       {
           "rater": "HikingAsylum",
           "assertion": "advanced",
           "rated": "Marilyn C",
           "rating": 0.90
         }
       ]
    }
)";

std::string WebSocketGateway::ReadSting() {
    if (USE_MOCK_DATA)
        return data;


}

void WebSocketGateway::Close() {
    LOG(DEBUG) << "WebSocketGateway: Close connection";
    if (USE_MOCK_DATA)
        return;

}
