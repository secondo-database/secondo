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

#ifndef KAFKA_WEBSOCKETCLIENTPROTOTYPE_H
#define KAFKA_WEBSOCKETCLIENTPROTOTYPE_H

#include <websocketpp/client.hpp>
#include <websocketpp/common/thread.hpp>
#include <websocketpp/common/memory.hpp>

enum ErrorCode {
    OK,
    CONNECT_ALREADY_CALLED,
    INIT_ERROR,
    CONNECT_FAILED,
};

template<typename ConfigType>
class Connection {
public:

    Connection(websocketpp::connection_hdl hdl, std::string uri);

    void on_open(websocketpp::client<ConfigType> *c,
                 websocketpp::connection_hdl hdl);

    void on_fail(websocketpp::client<ConfigType> *c,
                 websocketpp::connection_hdl hdl);

    void on_close(websocketpp::client<ConfigType> *c,
                  websocketpp::connection_hdl hdl);

    void on_message(websocketpp::connection_hdl,
                    typename websocketpp::client<ConfigType>::message_ptr msg);

    websocketpp::connection_hdl get_hdl() const {
        return m_hdl;
    }

    std::string get_status() const {
        return m_status;
    }

private:
    websocketpp::connection_hdl m_hdl;
    std::string m_status;
    std::string m_uri;
    std::string m_server;
    std::size_t maxMsgSize = 10;
public:
    std::queue<std::string> m_messages;
    std::string m_error_reason;
};


template<typename ConfigType>
class WebsocketClientPPB {
public:
    WebsocketClientPPB();

    ~WebsocketClientPPB();

    ErrorCode connect(std::string const &uri);
    void send(std::string message);
    void close(websocketpp::close::status::value code, std::string reason);

    websocketpp::lib::shared_ptr<Connection<ConfigType>> get_metadata() {
        return metadata_ptr;
    }

private:
    websocketpp::client<ConfigType> m_endpoint;
    websocketpp::lib::shared_ptr<websocketpp::lib::thread> m_thread;
    websocketpp::lib::shared_ptr<Connection<ConfigType>> metadata_ptr = nullptr;
    int timeout = 10 * 1000;

};


#endif //KAFKA_WEBSOCKETCLIENTPROTOTYPE_H
