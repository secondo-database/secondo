/*
----
This file is part of SECONDO.

Copyright (C) 2004, University in Hagen, Department of Computer Science,
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

//paragraph [1] Title: [{\Large \bf \begin {center}] [\end {center}}]
//[TOC] [\tableofcontents]

[1] Header File of the class ~PregelAlgebra~

November 2018, J. Mende


[TOC]

1 Overview

This file defines the members of class NetworkedClient

*/

#include "NetworkedClient.h"
#include "../Helpers/Metrics.h"
#include <StandardTypes.h>

namespace pregel {
 NetworkedClient::NetworkedClient(const RemoteEndpoint &host) noexcept(false) {
  auto socket = Socket::Connect(host.host, std::to_string(host.port),
                                Socket::SockAnyDomain, 10, 1);
  if (socket == nullptr) {
   BOOST_LOG_TRIVIAL(error) << "Couldn't connect to server.";
   throw std::exception();
  }
  if (not socket->IsOk()) {
   delete socket;
   BOOST_LOG_TRIVIAL(error) << "Connection to server is NOT ok.";
   throw std::exception();
  }

  BOOST_LOG_TRIVIAL(info) << "Successfully connect to host: " << host.host
                          << ":" << host.port;
  this->socket = socket;
 }

 NetworkedClient::~NetworkedClient() {
  if (socket != nullptr) {
   socket->Close();
   delete socket;
  }
 }

 void NetworkedClient::sendMessage(MessageWrapper *message) const {
  unsigned long size;
  char *buffer = message->serialize(size);
  socket->Write(buffer, size);

  if (message->getBody() != nullptr) {
   auto value = ((CcReal *) message->getBody()->GetAttribute(1))->GetValue();
   auto target = ((CcInt *) message->getBody()->GetAttribute(0))->GetValue();
   std::cout << "Send message to " << target << " in superstep "
             << message->getRound() << ". It's tuple has VALUE " << value
             << "\n";
   message->getBody()->DeleteIfAllowed(); // When we write,
   // we don't need the tuple anymore
   SENT_MESSAGE
  }

  delete message;
 }

 void NetworkedClient::healthReport(std::stringstream &sstream) const {
  if (socket == nullptr) {
   sstream << "    socket is nullptr" << std::endl;
   return;
  }
  sstream << "    Socket:  " << (socket->IsOk() ? "OK" : "NOT ok") << std::endl;
  sstream << "    Address: " << socket->GetSocketAddress().c_str() << std::endl;
 }
}
