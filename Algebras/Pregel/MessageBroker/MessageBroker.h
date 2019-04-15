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

This header file defines the class MessageBroker

2 Defines and includes

*/

#ifndef SECONDO_MESSAGEBROKER_H
#define SECONDO_MESSAGEBROKER_H

#include <list>
#include <functional>
#include <map>
#include "MessageWrapper.h"
#include "MessageServer.h"
#include "MessageClient.h"
#include "../Helpers/RemoteEndpoint.h"
#include <boost/thread.hpp>

namespace pregel {
 class MessageBroker {
 private:
  static MessageBroker broker;

  std::shared_ptr<boost::thread> tcpListener = nullptr;

  std::shared_ptr<Socket> globalSocket = nullptr;

  std::list<std::shared_ptr<MessageServer> > servers 
                    = std::list< std::shared_ptr<MessageServer> >();

  using mt = std::map<int, std::shared_ptr<MessageClient> >;
  mt slotToClient = mt();

  DoubleQueue inbox;

  void acceptConnections(int port);

  void collectFromAllServers(int superstep);

 public:
  MessageBroker();

  virtual ~MessageBroker();

  static MessageBroker &get();

  bool startTcpListener(const int port);

  bool startClient(int slot, RemoteEndpoint host);

  bool startLoopbackProxy(int slot);

  bool tcpListenerRunning();

  void pauseServers();
  
  void startServers();

  void stopServers(bool killThreads);

  void stopClients();

  void reset(const bool killThreads);

  void expectInitMessages();

  void startNewRound(bool &allEmpty, executable &callMeWhenYoureDone);

  unsigned long howManyMessagesInInbox(int superstep);

  supplier2<MessageWrapper>*  inboxSupplier(const int superstep);

  void sendMessage(std::shared_ptr<MessageWrapper> message);

  void broadcastEmptyMessage();

  void broadcastFinishMessage();

  void broadcastInitDoneMessage();

  void healthReport(std::stringstream &);

  unsigned long numberOfClients() const;

  unsigned long numberOfServers() const;

  DoubleQueue& getInBox();

  void clearMessages(); 

 };
}


#endif //SECONDO_MESSAGEBROKER_H
