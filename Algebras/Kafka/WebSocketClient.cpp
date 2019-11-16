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

#include "WebSocketClient.h"
#include "log.hpp"

#define USE_MOCK_DATA 1


void WebSocketClient::Open(std::string uri) {
    LOG(DEBUG) << "WebSocketClient: Connecting to " << uri;
    if (USE_MOCK_DATA)
        return;

}

void WebSocketClient::Subscribe(std::string body) {
    LOG(DEBUG) << "WebSocketClient: Sending subscribe request " << body;
    if (USE_MOCK_DATA)
        return;

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

std::string WebSocketClient::ReadSting() {
    if (USE_MOCK_DATA)
        return data;


}

void WebSocketClient::Close() {
    LOG(DEBUG) << "WebSocketClient: Close connection";
    if (USE_MOCK_DATA)
        return;

}
