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
#include <iostream>
#include "MessageServer.h"
#include <boost/log/trivial.hpp>
#include "../Helpers/Metrics.h"
#include "../PregelContext.h"

namespace pregel {
 MessageServer::MessageServer(std::shared_ptr<Socket>  _socket, 
                              executable _initDoneCallback) :
  stateCondition(),
  initDoneCallback(_initDoneCallback),
  socket(_socket) {
  thread = new boost::thread(boost::bind(&MessageServer::run, this));
 }

 MessageServer::~MessageServer() {
  socket->GetSocketStream().setstate(std::ios_base::failbit);
  socket->Close(); //TODO: need to do more?
  if (!thread->timed_join(boost::posix_time::milliseconds(0))) {
   thread->interrupt();
   if(thread->joinable()){
      thread->join();
   }
  }
  delete thread;
 }

 void MessageServer::run() {
  setState(WAITING, false);
  waitToResumeReading();
  try {
   while (!boost::this_thread::interruption_requested()) {
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
  if(!socket->IsOk()) {
    interrupt();
    return;
  }
  char headerBuffer[MessageWrapper::HEADER_SIZE];
  memset(headerBuffer,0,MessageWrapper::HEADER_SIZE);

  std::string bufferToString(headerBuffer, MessageWrapper::HEADER_SIZE);
  size_t lengthRead = 0;

  char* offset = headerBuffer;

  while (lengthRead < MessageWrapper::HEADER_SIZE) {
   if (!stateIs(READING)) {
    return;
   }
   lengthRead = socket->Read((void *) offset, MessageWrapper::HEADER_SIZE,
                             MessageWrapper::HEADER_SIZE,
                             1);

   if (lengthRead < 0) {
    // problem during reading data from socket
    interrupt();
    return;
   }

   if (lengthRead < MessageWrapper::HEADER_SIZE && lengthRead > 0) {
    BOOST_LOG_TRIVIAL(warning) << "header partially received (Read "
                               << lengthRead << "B / "
                               << MessageWrapper::HEADER_SIZE << "B)";
    BOOST_LOG_TRIVIAL(warning) << "buffer before: " << bufferToString.c_str();
    BOOST_LOG_TRIVIAL(warning) << "buffer after:  " <<
                               std::string(headerBuffer,
                                           MessageWrapper::HEADER_SIZE).c_str();
   }
   offset += lengthRead;
  }
  if(!socket->IsOk()) {
    interrupt();
    return;
  }
  auto header = MessageWrapper::Header::read(headerBuffer);

  const MessageWrapper::MessageType messageType = header.type;

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
    // BOOST_LOG_TRIVIAL(error) << "Received message of unknown type";
    assert(false);
    return;
  }

  const unsigned long bodySize = header.length;
  char* bodyBuffer = new char[bodySize];

  socket->Read((void *) bodyBuffer,
               bodySize); //blocking, but awaiting message body already

  auto message = MessageWrapper::deserialize(bodyBuffer, header);
  delete[] bodyBuffer;
  addMessage(message);
 }

 void MessageServer::addMessage(std::shared_ptr<MessageWrapper> message) {
  messageQueue.push(message, message->getRound());
 }

 void MessageServer::handleEmptyMessage() {
  boost::unique_lock<boost::mutex> lock(monitormtx);
  while(monitor==nullptr){
    monitorCond.wait(lock);
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
  boost::unique_lock<boost::mutex> lock(monitormtx);
  while(monitor==nullptr){
    monitorCond.wait(lock);
  }
  {
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
    boost::lock_guard<boost::mutex> lock(stateLock);
    auto monitorLocal = monitor;
    monitor = nullptr;
    monitorLocal->finish();
  }
  setState(WAITING, false);
  bringAllMessagesToRound(SuperstepCounter::get());
  initDoneCallback();
 }
  

 void MessageServer::bringAllMessagesToRound(const int round) {
   messageQueue.bringMessagesToRound(round);
 }

 void MessageServer::drainBuffer(const consumer2<MessageWrapper> &consumer,
                                 const int round) {
  messageQueue.consume(consumer, round);
 }

 void MessageServer::waitToResumeReading() {
  boost::unique_lock<boost::mutex> lock(stateLock);
  stateCondition.wait(lock, [&]() {
    return state == READING || boost::this_thread::interruption_requested();
  });
 }

 void MessageServer::startReading() {
  setState(READING);
 }

 void MessageServer::requestPause() {
  setState(WAITING);
 }

 bool MessageServer::stateIs(MessageServer::State state) {
  boost::lock_guard<boost::mutex> lock(stateLock);
  return this->state == state;
 }

 void MessageServer::setState(MessageServer::State state, bool notify) {
  {
   boost::lock_guard<boost::mutex> lock(stateLock);
   if(this->state == state){
     return;
   }
   this->state = state;
  }
  if (notify) {
   stateCondition.notify_one();
  }
 }


 std::string MessageServer::stateStr()const{
   switch(state){
     case READING : return "READING";
     case WAITING : return "WAITING"; 
     default      : return "unknown";
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

  sstream << "      State:   " << stateStr()  << std::endl;

  sstream << "      Queue:   " << messageQueue << std::endl;
  sstream << "      Monitor: " << (monitor==nullptr?"NULL":"PRESENT") 
          << std::endl;
  if(monitor!=nullptr){
     monitor->print(sstream, "          ");
  } 

 }

 void MessageServer::setMonitor(std::shared_ptr<Monitor> monitor) {
  this->monitor = monitor;
  monitorCond.notify_all();
 }
}
