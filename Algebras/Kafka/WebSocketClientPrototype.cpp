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
#include "WebSocketClientPrototype.h"
#include "log.hpp"

#include <websocketpp/config/asio_client.hpp>
#include <websocketpp/config/asio_no_tls_client.hpp>
#include <websocketpp/client.hpp>

#include <iostream>
#include <string>

static const int SLEEP_INTERVAL = 10;


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

std::string getErrorText(ErrorCode const errorCode) {
    return ErrorNames[errorCode];
}

template<typename ConfigType>
void WebsocketClientPPB<ConfigType>::on_open(websocketpp::connection_hdl hdl) {
    m_status = "Open";
}

template<typename ConfigType>
void WebsocketClientPPB<ConfigType>::on_fail(websocketpp::connection_hdl hdl) {
    m_status = "Failed";

    typename websocketpp::connection<ConfigType>::ptr con
            = this->m_endpoint.get_con_from_hdl(hdl);
    m_error_reason = con->get_ec().message();

    LOG(ERROR) << "WebsocketClientPPB connect failed with reason: "
               << m_error_reason;
}

template<typename ConfigType>
void
WebsocketClientPPB<ConfigType>::on_close(websocketpp::connection_hdl hdl) {
    m_status = "Closed";
    typename websocketpp::connection<ConfigType>::ptr con
            = this->m_endpoint.get_con_from_hdl(hdl);
    std::stringstream s;
    s << "close code: " << con->get_remote_close_code() << " ("
      << websocketpp::close::status::get_string(
              con->get_remote_close_code())
      << "), close reason: " << con->get_remote_close_reason();
    m_error_reason = s.str();
}

template<typename ConfigType>
void WebsocketClientPPB<ConfigType>::on_message(
        websocketpp::connection_hdl,
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
WebsocketClientPPB<ConfigType>::WebsocketClientPPB::WebsocketClientPPB() {
    m_endpoint.clear_access_channels(websocketpp::log::alevel::all);
    m_endpoint.clear_error_channels(websocketpp::log::elevel::all);

    m_endpoint.init_asio();

    setInitHandler<websocketpp::client<ConfigType>>(&m_endpoint);

    m_endpoint.start_perpetual();

    m_thread = websocketpp::lib::make_shared<websocketpp::lib::thread>(
            &websocketpp::client<ConfigType>::run, &m_endpoint);
}

template<typename ConfigType>
WebsocketClientPPB<ConfigType>::~WebsocketClientPPB() {
    m_endpoint.stop_perpetual();

    if (this->m_status.compare("Open") == 0) {
        this->close(websocketpp::close::status::going_away, "");
    }


    m_thread->join();
}

template<typename ConfigType>
ErrorCode WebsocketClientPPB<ConfigType>::connect(std::string const &uri) {
    LOG(DEBUG) << "WebsocketClientPPB: Connecting to " << uri;

    if (m_status.compare("") != 0)
        return CONNECT_ALREADY_CALLED;

    m_status = "Connecting";

    websocketpp::lib::error_code ec;
    typename websocketpp::connection<ConfigType>::ptr con
            = m_endpoint.get_connection(uri, ec);

    if (ec) {
        std::cout << "> Connect initialization error: " << ec.message()
                  << std::endl;
        return INIT_ERROR;
    }

    m_hdl = con->get_handle();

    con->set_open_handler(websocketpp::lib::bind(
            &WebsocketClientPPB<ConfigType>::on_open,
            this,
            websocketpp::lib::placeholders::_1
    ));
    con->set_fail_handler(websocketpp::lib::bind(
            &WebsocketClientPPB<ConfigType>::on_fail,
            this,
            websocketpp::lib::placeholders::_1
    ));
    con->set_close_handler(websocketpp::lib::bind(
            &WebsocketClientPPB<ConfigType>::on_close,
            this,
            websocketpp::lib::placeholders::_1
    ));
    con->set_message_handler(websocketpp::lib::bind(
            &WebsocketClientPPB<ConfigType>::on_message,
            this,
            websocketpp::lib::placeholders::_1,
            websocketpp::lib::placeholders::_2
    ));

    m_endpoint.connect(con);

    int restTime = timeout;
    while (this->get_status().compare("Connecting") == 0 &&
           restTime > 0) {
        kafka::sleepMS(SLEEP_INTERVAL);
        restTime -= SLEEP_INTERVAL;
    }

    if (this->get_status().compare("Connecting") == 0 &&
        restTime <= 0)
        LOG(ERROR) << "WebsocketClientPPB: Time out";

    if (this->get_status().compare("Open") == 0)
        return OK;
    else {
        LOG(ERROR) << "WebsocketClientPPB status: " << this->get_status();
        return CONNECT_FAILED;
    }
}

template<typename ConfigType>
void WebsocketClientPPB<ConfigType>::close(
        websocketpp::close::status::value code, std::string reason) {
    websocketpp::lib::error_code ec;

    std::cout << "> Closing connection " << std::endl;

//    if (this->m_hdl-> == 0) {
//        std::cout << "> No connection found" << std::endl;
//        return;
//    }

    m_endpoint.close(this->m_hdl, code, reason, ec);
    if (ec) {
        std::cout << "> Error initiating close: " << ec.message()
                  << std::endl;
    }

    int restTime = timeout;
    while (this->m_status.compare("Closed") != 0 &&
           restTime > 0) {
        kafka::sleepMS(SLEEP_INTERVAL);
        restTime -= SLEEP_INTERVAL;
    }
}

template<typename ConfigType>
void WebsocketClientPPB<ConfigType>::send(std::string message) {
    websocketpp::lib::error_code ec;

    m_endpoint.send(this->m_hdl, message,
                    websocketpp::frame::opcode::text, ec);
    if (ec) {
        std::cout << "> Error sending message: " << ec.message()
                  << std::endl;
        return;
    }
}


// No need to call this TemporaryFunctions() function,
// it's just to avoid link error.
void prototypeTestTLS() {
    std::string uri = "wss://ws.blockchain.info/inv";
    typedef websocketpp::config::asio_tls_client ConfigTypeTLS;

    WebsocketClientPPB<ConfigTypeTLS> endpoint;

    ErrorCode code = endpoint.connect(uri);
    if (code == OK) {
        endpoint.send(R"({"op":"unconfirmed_sub"})");

        // Wait
        std::this_thread::sleep_for(std::chrono::milliseconds(2000));

        while (endpoint.get_message_count() > 0) {
            std::cout << endpoint.getFrontAndPop() << std::endl;
        }

        endpoint.close(websocketpp::close::status::normal, "Prosto tak");
    } else {
        std::cout << "Connection response:" << code << std::endl
                  << " Connection status: " << endpoint.get_status()
                  << std::endl
                  << "Reason: " << endpoint.get_error_reason() << std::endl;
    }
}

// No need to call this TemporaryFunctions() function,
// it's just to avoid link error.
void prototypeTestPlain() {
    std::string uri = "ws://localhost:9002";
    typedef websocketpp::config::asio_client ConfigTypeC;

    WebsocketClientPPB<ConfigTypeC> endpoint;

    ErrorCode code = endpoint.connect(uri);
    if (code == OK) {
        endpoint.send(R"({"op":"unconfirmed_sub"})");

        // Wait
        std::this_thread::sleep_for(std::chrono::milliseconds(2000));

        while (endpoint.get_message_count() > 0) {
            std::cout << endpoint.getFrontAndPop() << std::endl;
        }

        endpoint.close(websocketpp::close::status::normal, "Prosto tak");
    } else {
        std::cout << "Connection response:" << code << std::endl
                  << " Connection status: " << endpoint.get_status()
                  << std::endl
                  << "Reason: " << endpoint.get_error_reason() << std::endl;
    }
}
