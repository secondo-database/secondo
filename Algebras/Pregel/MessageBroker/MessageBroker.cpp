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

This file defines the members of class MessageBroker

*/

#include <iostream>
#include "MessageBroker.h"
#include "NetworkedClient.h"
#include "LoopbackProxy.h"
#include "../typedefs.h"
#include <pthread.h>
#include "../Helpers/LoggerFactory.h"
#include "../Helpers/Metrics.h"
#include "../PregelContext.h"
#include <StandardTypes.h>

namespace pregel {
 MessageBroker MessageBroker::broker;

 MessageBroker::MessageBroker() : inbox() {}

 MessageBroker::~MessageBroker() {
  stopServers();
  stopClients();
 }

 MessageBroker &MessageBroker::get() {
  return broker;
 }

 bool MessageBroker::startServerMother(const int port) {
  try {
   BOOST_LOG_TRIVIAL(info) << "start message server on port " << port;
   serverMother = new boost::thread(
    boost::bind(&MessageBroker::acceptConnections, this, port)
   );
   return true;
  } catch (std::exception &e) {
   BOOST_LOG_TRIVIAL(error) << e.what();
   return false;
  } catch (boost::exception &e) {
   BOOST_LOG_TRIVIAL(error) << "Caught boost exception";
   return false;
  }
 }

 void MessageBroker::acceptConnections(const int port) {
  try {
   globalSocket = Socket::CreateGlobal("localhost", std::to_string(port));

   while (!boost::this_thread::interruption_requested()) {
    auto serverSocket = globalSocket->Accept();
    boost::this_thread::interruption_point();
    if (boost::this_thread::interruption_requested() ||
        serverSocket == nullptr || !serverSocket->IsOk()) {
     continue;
    }

    executable initDoneMessageHandler = [this]() {
      pauseServers();
      collectFromAllServers(SuperstepCounter::get());
    };

    auto messageServer = new MessageServer(serverSocket,
                                           initDoneMessageHandler);
    servers.push_back(messageServer);
    boost::this_thread::interruption_point();
   }
  } catch (boost::thread_interrupted &interrupted) {
   BOOST_LOG_TRIVIAL(debug) << "Interrupted. Return";
  }
  if (globalSocket != nullptr) {
   globalSocket->Close();
  }
 }

 bool MessageBroker::startClient(const int slot, const RemoteEndpoint host) {
  try {
   auto client = new NetworkedClient(host);
   slotToClient.insert(std::pair<int, NetworkedClient *>(slot, client));
   return true;
  } catch (std::exception &e) {
   return false;
  }
 }

 bool MessageBroker::startLoopbackProxy(int slot) {
  try {
   consumer<MessageWrapper> loopbackInsert =
    [this](MessageWrapper *message) {
      if (message->getType() == MessageWrapper::MessageType::DATA) {
      auto value = ((CcReal *) message->getBody()->GetAttribute(1))->GetValue();
      auto target = ((CcInt *) message->getBody()->GetAttribute(0))->GetValue();
      std::cout << "Queue message to " << target << " in superstep "
                << message->getRound() << ". It's tuple has VALUE " << value
                << "\n";
      this->inbox.push(message, message->getRound());
      QUEUED_MESSAGE
      } else {
       delete message;
      }
    };

   auto client = new LoopbackProxy(loopbackInsert);
   slotToClient.insert(std::pair<int, LoopbackProxy *>(slot, client));
   return true;
  } catch (std::exception &e) {
   return false;
  }
 }

 void MessageBroker::reset() {
  stopServers();
  stopClients();
 }

 void MessageBroker::stopServers() {
  if (serverMotherRunning()) {
   serverMother->interrupt();

   const std::string &socketAddress = globalSocket->GetSocketAddress();

   int messageServerPort = PregelContext::get().getMessageServerPort();
   Socket *dummySocket = nullptr;
   while (dummySocket == nullptr) {
    dummySocket = Socket::Connect(socketAddress, std::to_string(
     messageServerPort), Socket::SocketDomain::SockGlobalDomain);
   }
   delete dummySocket;

   serverMother->join();

   if (globalSocket != nullptr) {
    delete globalSocket;
    globalSocket = nullptr;
   }

   delete serverMother;
   serverMother = nullptr;
  }

  for (MessageServer *server : servers) {
   delete server;
  }
  servers.clear();
 }

 void MessageBroker::pauseServers() {
  if (!serverMotherRunning()) {
   return;
  }

  for (MessageServer *server : servers) {
   server->requestPause();
  }
 }

 void MessageBroker::stopClients() {
  std::for_each(slotToClient.begin(),
                slotToClient.end(),
                [](std::pair<int, MessageClient *> slotClientPair) {
                  delete slotClientPair.second;
                }
  );
  slotToClient.clear();
 }

 void MessageBroker::sendMessage(MessageWrapper *message) {
  const int destination = message->getDestination();
  if (slotToClient.find(destination) == slotToClient.end()) {
   BOOST_LOG_TRIVIAL(warning) << "no client set up with destination "
                              << destination << ". Will delete message";
   DISCARDED_MESSAGE
   delete message;
   return;
  }
  MessageClient *client = slotToClient.at(destination);
  client->sendMessage(message);
 }

 void MessageBroker::collectFromAllServers(int superstep) {
  consumer<MessageWrapper> moveToOwnBuffer =
   [this, superstep](MessageWrapper *message) {
    auto value = ((CcReal *) message->getBody()->GetAttribute(1))->GetValue();
    auto target = ((CcInt *) message->getBody()->GetAttribute(0))->GetValue();
     std::cout << "Collect message to " << target << " with value " << value
               << "\n";
    inbox.push(message, superstep);
   };
  std::cout << "Collect messages in superstep " << superstep << "\n";
  for (auto server : servers) {
   server->drainBuffer(moveToOwnBuffer, superstep);
  }
 }

 supplier<MessageWrapper> *MessageBroker::inboxSupplier(const int superstep) {
  return new supplier<MessageWrapper>(inbox.supply(superstep));
 }

 void MessageBroker::startNewRound(bool &allEmpty,
                                   executable &callMeWhenYoureDone) {
  const unsigned long numberOfConnections = servers.size();

  auto callback = (std::function<void(bool)>)
   [this, &allEmpty, &callMeWhenYoureDone](bool empty) {
    int superstep = SuperstepCounter::get();
    collectFromAllServers(superstep);
    allEmpty &= empty; // not synch, but thread is waiting, or still busy anyway
    callMeWhenYoureDone();
  };

  auto monitor = new Monitor(numberOfConnections - 1/*Master*/,
                                    callback);

  for (MessageServer *server : servers) {
   server->setMonitor(monitor);
   server->startReading();
  }
 }

 unsigned long MessageBroker::howManyMessagesInInbox(int round) {
  return inbox.size(round);
 }

 void MessageBroker::broadcastEmptyMessage() {
  const int superstep = SuperstepCounter::get();
  for (auto it = slotToClient.begin(); it != slotToClient.end(); ++it) {
   int destination = (*it).first;
   auto message = MessageWrapper::constructEmptyMessage(destination, superstep);
   sendMessage(message);
  }
 }

 void MessageBroker::broadcastFinishMessage() {
  const int superstep = SuperstepCounter::get();
  for (auto it = slotToClient.begin(); it != slotToClient.end(); ++it) {
   int destination = (*it).first;
   auto message = MessageWrapper::constructFinishMessage(destination,
                                                         superstep);
   sendMessage(message);
  }
 }

 void MessageBroker::broadcastInitDoneMessage() {
  const int superstep = SuperstepCounter::get();
  for (auto it = slotToClient.begin(); it != slotToClient.end(); ++it) {
   int destination = (*it).first;
   auto message = MessageWrapper::constructInitDoneMessage(destination,
                                                           superstep);
   sendMessage(message);
  }
 }

 void MessageBroker::healthReport(std::stringstream &sstream) {
  sstream << "+++Broker" << std::endl;
  sstream << "  Queue: " << inbox << std::endl;
  sstream << "  Clients (" << slotToClient.size() << ") : "
          << (clientsAlive() ? "ready (always true)" : "error")
          << std::endl;

  for (auto client : slotToClient) {
   sstream << "  Client " << client.first << std::endl;
   client.second->healthReport(sstream);
  }

  int number = 0;
  sstream << "  Servers" << std::endl;
  sstream << "    ServerMother: " << (serverMotherRunning() ? "up" : "down")
          << std::endl;
  for (auto server : servers) {
   sstream << "    Server " << number++ << std::endl;
   server->healthReport(sstream);
  }
 }

 bool MessageBroker::serverMotherRunning() {
  if (serverMother == nullptr) {
   return false;
  }
  if (!serverMother->joinable()) {
   return false;
  }
  if (serverMother->try_join_for(boost::chrono::nanoseconds(1))) {
   return false;
  }
  return true;
 }

 void MessageBroker::expectInitMessages() {
  for (MessageServer *server : servers) {
   server->startReading();
  }
 }

 unsigned long MessageBroker::numberOfClients() {
  return slotToClient.size();
 }
}
