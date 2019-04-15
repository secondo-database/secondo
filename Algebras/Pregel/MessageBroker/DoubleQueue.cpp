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

This file defines the members of class DoubleQueue

*/

#include "DoubleQueue.h"
#include <boost/log/trivial.hpp>

namespace pregel {
 std::ostream &
 operator<<(std::ostream &os, DoubleQueue &buffer) {
  os << "buffersize[0]: " << buffer.getQueue(0).size() 
     << " buffersize[1]: " << buffer.getQueue(1).size();
  return os;
 }

 DoubleQueue::DoubleQueue() : buffers(), lock() {}

 unsigned long DoubleQueue::size(const int round) {
  boost::lock_guard<boost::mutex> guard(lock[(round + 2) % 2]);
  return getQueue(round).size();
 }

 std::shared_ptr<MessageWrapper> DoubleQueue::pop(const int round) {
  boost::lock_guard<boost::mutex> guard(lock[(round + 2) % 2]);
  MessageQueue &queue = getQueue(round);
  if (queue.empty()) {
   return nullptr;
  }
  std::shared_ptr<MessageWrapper> message = queue.front();
  queue.pop();
  return message;
 }

 void DoubleQueue::consume(const consumer2<MessageWrapper> &callback,
                           const int round) {
  auto supplier = supply(round);
  std::shared_ptr<MessageWrapper> message;
  while ((message = supplier()) != nullptr) {
   callback(message);
  }
//  Forwarder<MessageWrapper>(supplier, callback).consumeAll();
 }

 supplier2<MessageWrapper> DoubleQueue::supply(const int round) {
  MessageQueue &queue = getQueue(round);
  return supplier2<MessageWrapper>([&queue, this, round]() 
     -> std::shared_ptr<MessageWrapper>  {
    boost::lock_guard<boost::mutex> guard(this->lock[(round + 2) % 2]);
    if (queue.empty()) {
     return nullptr;
    }
    std::shared_ptr<MessageWrapper> message = queue.front();
    queue.pop();
    return message;
  });
 }

 void DoubleQueue::push(std::shared_ptr<MessageWrapper> message, 
                        const int round) {
  boost::lock_guard<boost::mutex> guard(lock[(round + 2) % 2]);
  MessageQueue &queue = getQueue(round);
  queue.push(message);
 }

 MessageQueue &DoubleQueue::getQueue(const int round) {
  return buffers[(round + 2) % 2];
 }

 void DoubleQueue::bringMessagesToRound(const int round){
    MessageQueue& source = getQueue(round+1);
    MessageQueue& target = getQueue(round);
    while(!source.empty()){
       auto msg = source.front();
       msg->setRound(round);
       target.push(msg);
       source.pop();     
    }
 }

 void DoubleQueue::clear(){
    MessageQueue& q1 = buffers[0];
    while(!q1.empty()){
      q1.pop();
    }
    MessageQueue& q2 = buffers[1];
    while(!q2.empty()){
      q2.pop();
    }
 }


}
