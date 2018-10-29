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

This file defines the members of class MessageServer

*/

#include "MessageServer.h"
#include "../Helpers/LoggerFactory.h"
#include "../Helpers/Metrics.h"
#include "../PregelContext.h"

namespace pregel {
 MessageServer::MessageServer(Socket *socket, executable initDoneCallback) :
  stateCondition(),
  initDoneCallback(initDoneCallback),
  socket(socket) {
  thread = new boost::thread(boost::bind(&MessageServer::run, this));
 }

 MessageServer::~MessageServer() {
  FORCE_LOG
  BOOST_LOG_TRIVIAL(debug) << "Destruct message server";

  if (!thread->timed_join(boost::posix_time::milliseconds(0))) {
   thread->interrupt();
   thread->join();
  }
  delete thread;

  socket->Close(); //TODO: need to do more?
  delete socket;

  delete monitor; // broker owns context
 }

 void MessageServer::run() {
  FORCE_LOG
  setState(WAITING, false);
  waitToResumeReading();
  try {
   while (!thread->interruption_requested()) {
    processMessage();
   }
  } catch (boost::thread_interrupted &e) {
   BOOST_LOG_TRIVIAL(error) << "catching interrupted";
   return;
  }
 }

 void MessageServer::interrupt() {
  thread->interrupt();
 }

 void MessageServer::processMessage() {
  boost::this_thread::interruption_point();
  char headerBuffer[MessageWrapper::HEADER_SIZE];
  std::string bufferToString(headerBuffer, MessageWrapper::HEADER_SIZE);
  size_t lengthRead = 0;

  while (lengthRead < MessageWrapper::HEADER_SIZE) {
   if (!stateIs(READING)) {
    return;
   }
   lengthRead = socket->Read((void *) headerBuffer, MessageWrapper::HEADER_SIZE,
                             MessageWrapper::HEADER_SIZE,
                             1);

   if (lengthRead < 0) {
    interrupt();
    return;
   }

   if (lengthRead < MessageWrapper::HEADER_SIZE && lengthRead > 0) {
    FORCE_LOG
    BOOST_LOG_TRIVIAL(warning) << "header partially received (Read "
                               << lengthRead << "B / "
                               << MessageWrapper::HEADER_SIZE << "B)";
    BOOST_LOG_TRIVIAL(warning) << "buffer before: " << bufferToString.c_str();
    BOOST_LOG_TRIVIAL(warning) << "buffer after:  " <<
                               std::string(headerBuffer,
                                           MessageWrapper::HEADER_SIZE).c_str();
   }
  }
  auto header = MessageWrapper::Header::read(headerBuffer);

  const int messageType = header.type;
  switch (messageType) {
   case MessageWrapper::MessageType::EMPTY:
    handleEmptyMessage();
    return;
   case MessageWrapper::MessageType::FINISH:
    handleFinishedMessage();
    return;
   case MessageWrapper::MessageType::INIT_DONE:
    handleInitDoneMessage();
    return;
   case MessageWrapper::MessageType::DATA:
    RECEIVED_MESSAGE
    break;
   default:
    return;
  }

  const unsigned long bodySize = header.length;
  char bodyBuffer[bodySize];
  socket->Read((void *) bodyBuffer,
               bodySize); //blocking, but awaiting message body already

  auto message = MessageWrapper::deserialize(bodyBuffer, header);
  addMessage(message);
 }

 void MessageServer::addMessage(MessageWrapper *message) {
  messageQueue.push(message, round);
 }

 void MessageServer::handleEmptyMessage() {
  if (monitor == nullptr) {
   BOOST_LOG_TRIVIAL(error) << "Received EMPTY message outside of a round";
   return;
  }
  {
   boost::lock_guard<boost::mutex> lock(stateLock);
   state = WAITING;
   auto monitorLocal = monitor;
   monitor = nullptr;
   monitorLocal->empty();
  }
  // only iff it's the last one to finish,
  // it then collects the messages all together
  waitToResumeReading();
 }

 void MessageServer::handleFinishedMessage() {
  if (monitor == nullptr) {
   BOOST_LOG_TRIVIAL(error) << "Received EMPTY message outside of a round";
   return;
  }
  {
   std::cout << "handle FINISH, wait\n";
   boost::lock_guard<boost::mutex> lock(stateLock);
   this->state = WAITING;
   auto monitorLocal = monitor;
   monitor = nullptr;
   monitorLocal->finish();
  }
  // only iff it's the last one to finish,
  // it also collects the messages all together
  waitToResumeReading();
 }

 void MessageServer::handleInitDoneMessage() {
  if (monitor != nullptr) {
   FORCE_LOG
   BOOST_LOG_TRIVIAL(error) << "Received PAUSE message inside of a round";
   return;
  }

  initDoneCallback();
 }

 void MessageServer::drainBuffer(const consumer<MessageWrapper> &consumer,
                                 const int round) {
  messageQueue.consume(consumer, round);
 }

 void MessageServer::waitToResumeReading() {
  boost::unique_lock<boost::mutex> lock(stateLock);
  std::cout << "wait with reading\n";
  stateCondition.wait(lock, [&]() {
    return state == READING || thread->interruption_requested();
  });
  updateRound();
 }

 void MessageServer::updateRound() {
  round = SuperstepCounter::get();
 }

 void MessageServer::startReading() {
  setState(READING);
 }

 void MessageServer::requestPause() {
//  setState(INTERRUPTED);
  setState(WAITING);
 }

 bool MessageServer::stateIs(MessageServer::State state) {
  boost::lock_guard<boost::mutex> lock(stateLock);
  return this->state == state;
 }

 void MessageServer::setState(MessageServer::State state, bool notify) {
  {
   boost::lock_guard<boost::mutex> lock(stateLock);
   this->state = state;
  }
  if (notify) {
   stateCondition.notify_one();
  }
 }

 void MessageServer::healthReport(std::stringstream &sstream) {
  boost::lock_guard<boost::mutex> guard(stateLock);
  if (socket == nullptr) {
   sstream << "    socket is nullptr" << std::endl;
   return;
  }
  sstream << "      Socket:  " << (socket->IsOk() ? "OK" : "NOT ok")
          << std::endl;
  sstream << "      Address: " << socket->GetSocketAddress().c_str()
          << std::endl;

  sstream << "      State: " << state << std::endl;
 }

 void MessageServer::setMonitor(Monitor *monitor) {
  this->monitor = monitor;
 }
}
