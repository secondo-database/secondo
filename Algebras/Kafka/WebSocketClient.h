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

#ifndef KAFKA_WEBSOCKETCLIENT_H
#define KAFKA_WEBSOCKETCLIENT_H

#include <queue>
#include <websocketpp/client.hpp>
#include <websocketpp/common/thread.hpp>
#include <websocketpp/common/memory.hpp>

enum ErrorCode {
    OK,
    CONNECT_ALREADY_CALLED,
    INIT_ERROR,
    CONNECT_FAILED,
};

std::string getErrorText(ErrorCode const errorCode);

template<typename ConfigType>
class WebsocketClientPPB {
public:
    WebsocketClientPPB();

    ~WebsocketClientPPB();

    ErrorCode connect(std::string const &uri);

    void send(std::string message);

    void close(websocketpp::close::status::value code, std::string reason);

    std::string get_status() const {
        return m_status;
    }

    std::size_t get_message_count() const {
        return m_messages.size();
    }

    std::string getFrontAndPop() {
        std::string res = m_messages.front();
        m_messages.pop();
        return res;
    }

    std::string get_error_reason() const {
        return m_error_reason;
    }

private:

    void on_open(websocketpp::connection_hdl hdl);

    void on_fail(websocketpp::connection_hdl hdl);

    void on_close(websocketpp::connection_hdl hdl);

    void on_message(websocketpp::connection_hdl,
                    typename websocketpp::client<ConfigType>::message_ptr msg);

    websocketpp::connection_hdl m_hdl;
    websocketpp::client<ConfigType> m_endpoint;
    websocketpp::lib::shared_ptr<websocketpp::lib::thread> m_thread;

    int timeout = 10 * 1000;
    std::string m_status;
    std::size_t maxMsgSize = 10000;
    std::queue<std::string> m_messages;
    std::string m_error_reason;
};


#endif //KAFKA_WEBSOCKETCLIENT_H
