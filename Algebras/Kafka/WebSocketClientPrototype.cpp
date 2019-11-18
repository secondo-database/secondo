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

#include "Utils.h"

#include <websocketpp/config/asio_client.hpp>
#include <websocketpp/config/asio_no_tls_client.hpp>
#include <websocketpp/client.hpp>

#include <websocketpp/common/thread.hpp>
#include <websocketpp/common/memory.hpp>

#include <iostream>
#include <string>

static const int SLEEP_INTERVAL = 10;

template<typename ConfigType>
class Connection {
public:
    Connection(websocketpp::connection_hdl hdl,
               std::string uri)
            : m_hdl(hdl), m_status("Connecting"), m_uri(uri),
              m_server("N/A") {}

    void on_open(websocketpp::client<ConfigType> *c,
                 websocketpp::connection_hdl hdl) {
        m_status = "Open";
        m_server = c->get_con_from_hdl(hdl)->get_response_header("Server");
    }

    void on_fail(websocketpp::client<ConfigType> *c,
                 websocketpp::connection_hdl hdl) {
        m_status = "Failed";

        typename websocketpp::connection<ConfigType>::ptr con
                = c->get_con_from_hdl(hdl);
        m_server = con->get_response_header("Server");
        m_error_reason = con->get_ec().message();
    }

    void on_close(websocketpp::client<ConfigType> *c,
                  websocketpp::connection_hdl hdl) {
        m_status = "Closed";
        typename websocketpp::connection<ConfigType>::ptr con
                = c->get_con_from_hdl(hdl);
        std::stringstream s;
        s << "close code: " << con->get_remote_close_code() << " ("
          << websocketpp::close::status::get_string(
                  con->get_remote_close_code())
          << "), close reason: " << con->get_remote_close_reason();
        m_error_reason = s.str();
    }

    void on_message(websocketpp::connection_hdl,
                    typename websocketpp::client<ConfigType>::message_ptr msg) {
        if (msg->get_opcode() == websocketpp::frame::opcode::text) {
            if (m_messages.size() > maxMsgSize) {
                std::cout << "Max buffer size exided" << std::endl;
            } else {
                m_messages.push(msg->get_payload());
            }
        } else {
            std::cout << "Not textual message, ignoring" << std::endl;
        }
    }

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

enum ErrorCode {
    OK,
    CONNECT_ALREADY_CALLED,
    INIT_ERROR,
    CONNECT_FAILED,
};

std::string ErrorNames[] =
        {
                "OK",
                "CONNECT_ALREADY_CALLED",
                "INIT_ERROR",
                "CONNECT_FAILED",
        };

std::ostream &operator<<(std::ostream &out, const ErrorCode value) {
    return out << ErrorNames[value];
}


typedef std::shared_ptr<boost::asio::ssl::context> context_ptr;

static context_ptr on_tls_init() {
    // establishes a SSL connection
    context_ptr ctx = std::make_shared<boost::asio::ssl::context>(
            boost::asio::ssl::context::sslv23);

    try {
        ctx->set_options(boost::asio::ssl::context::default_workarounds |
                         boost::asio::ssl::context::no_sslv2 |
                         boost::asio::ssl::context::no_sslv3 |
                         boost::asio::ssl::context::single_dh_use);
    } catch (std::exception &e) {
        std::cout << "Error in context pointer: " << e.what() << std::endl;
    }
    return ctx;
}

//partial template specialization
template<class T>
void setInitHandler(T *endpoint) {

}

template<>
void setInitHandler(
        websocketpp::client<websocketpp::config::asio_tls_client> *tls_endpoint
) {
    tls_endpoint->set_tls_init_handler(bind(&on_tls_init));
}
// end partial template specialization


template<typename ConfigType>
class WebsocketGateway {
public:
    WebsocketGateway() {
        m_endpoint.clear_access_channels(websocketpp::log::alevel::all);
        m_endpoint.clear_error_channels(websocketpp::log::elevel::all);

        m_endpoint.init_asio();

        setInitHandler<websocketpp::client<ConfigType>>(&m_endpoint);

        m_endpoint.start_perpetual();

        m_thread = websocketpp::lib::make_shared<websocketpp::lib::thread>(
                &websocketpp::client<ConfigType>::run, &m_endpoint);
    }

    ~WebsocketGateway() {
        m_endpoint.stop_perpetual();

        if (metadata_ptr != nullptr && metadata_ptr->get_status() == "Open") {
            this->close(websocketpp::close::status::going_away, "");
        }


        m_thread->join();
    }

    ErrorCode connect(std::string const &uri) {

        if (metadata_ptr != nullptr)
            return CONNECT_ALREADY_CALLED;

        websocketpp::lib::error_code ec;

        typename websocketpp::connection<ConfigType>::ptr con
                = m_endpoint.get_connection(uri, ec);

        if (ec) {
            std::cout << "> Connect initialization error: " << ec.message()
                      << std::endl;
            return INIT_ERROR;
        }

        metadata_ptr = websocketpp::lib::make_shared<Connection<ConfigType>>(
                con->get_handle(), uri);

        con->set_open_handler(websocketpp::lib::bind(
                &Connection<ConfigType>::on_open,
                metadata_ptr,
                &m_endpoint,
                websocketpp::lib::placeholders::_1
        ));
        con->set_fail_handler(websocketpp::lib::bind(
                &Connection<ConfigType>::on_fail,
                metadata_ptr,
                &m_endpoint,
                websocketpp::lib::placeholders::_1
        ));
        con->set_close_handler(websocketpp::lib::bind(
                &Connection<ConfigType>::on_close,
                metadata_ptr,
                &m_endpoint,
                websocketpp::lib::placeholders::_1
        ));
        con->set_message_handler(websocketpp::lib::bind(
                &Connection<ConfigType>::on_message,
                metadata_ptr,
                websocketpp::lib::placeholders::_1,
                websocketpp::lib::placeholders::_2
        ));

        m_endpoint.connect(con);

        int restTime = timeout;
        while (metadata_ptr->get_status().compare("Connecting") == 0 &&
               restTime > 0) {
            kafka::sleepMS(SLEEP_INTERVAL);
            restTime -= SLEEP_INTERVAL;
        }

        if (metadata_ptr->get_status().compare("Open") == 0)
            return OK;
        else
            return CONNECT_FAILED;
    }

    void close(websocketpp::close::status::value code, std::string reason) {
        websocketpp::lib::error_code ec;

        std::cout << "> Closing connection " << std::endl;

        if (metadata_ptr == nullptr) {
            std::cout << "> No connection found" << std::endl;
            return;
        }

        m_endpoint.close(metadata_ptr->get_hdl(), code, reason, ec);
        if (ec) {
            std::cout << "> Error initiating close: " << ec.message()
                      << std::endl;
        }

        int restTime = timeout;
        while (metadata_ptr->get_status().compare("Closed") != 0 &&
               restTime > 0) {
            kafka::sleepMS(SLEEP_INTERVAL);
            restTime -= SLEEP_INTERVAL;
        }
    }

    void send(std::string message) {
        websocketpp::lib::error_code ec;

        m_endpoint.send(metadata_ptr->get_hdl(), message,
                        websocketpp::frame::opcode::text, ec);
        if (ec) {
            std::cout << "> Error sending message: " << ec.message()
                      << std::endl;
            return;
        }
    }

    websocketpp::lib::shared_ptr<Connection<ConfigType>> get_metadata() {
        return metadata_ptr;
    }

private:
    websocketpp::client<ConfigType> m_endpoint;
    websocketpp::lib::shared_ptr<websocketpp::lib::thread> m_thread;
    websocketpp::lib::shared_ptr<Connection<ConfigType>> metadata_ptr = nullptr;
    int timeout = 10 * 1000;
};

void prototypeTest() {
//    std::string uri = "ws://localhost:9002";
    std::string uri = "wss://ws.blockchain.info/inv";
    typedef websocketpp::config::asio_tls_client ConfigTypeC;
//    typedef websocketpp::config::asio_client ConfigTypeC;

    WebsocketGateway<ConfigTypeC> endpoint;

    ErrorCode code = endpoint.connect(uri);
    if (code == OK) {
        endpoint.send(R"({"op":"unconfirmed_sub"})");

        // Wait
        std::this_thread::sleep_for(std::chrono::milliseconds(2000));

        websocketpp::lib::shared_ptr<Connection<ConfigTypeC>> metadata
                = endpoint.get_metadata();

        while (metadata->m_messages.size() > 0) {
            std::cout << metadata->m_messages.front() << std::endl;
            metadata->m_messages.pop();
        }

        endpoint.close(websocketpp::close::status::normal, "Prosto tak");
    } else {
        std::cout << "Connection response:" << code << std::endl;
        websocketpp::lib::shared_ptr<Connection<ConfigTypeC>> metadata
                = endpoint.get_metadata();
        std::cout << "Connection status:" << metadata->get_status() << std::endl
                  << "Reason: " << metadata->m_error_reason << std::endl;
    }
}
