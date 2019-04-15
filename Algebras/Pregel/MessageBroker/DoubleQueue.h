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

This header file defines the class DoubleQueue

2 Defines and includes

*/

#ifndef SECONDO_SWITCHABLEBUFFER_H
#define SECONDO_SWITCHABLEBUFFER_H

#include <ostream>
#include <queue>
#include <functional>
#include "MessageWrapper.h"
#include "../typedefs.h"
#include <boost/thread.hpp>

namespace pregel {
 using MessageQueue = std::queue<std::shared_ptr<MessageWrapper> >;

 class DoubleQueue {
 public:
  DoubleQueue();

  friend std::ostream &
  operator<<(std::ostream &os, DoubleQueue &buffer);

  MessageQueue &getQueue(const int round);

  unsigned long size(const int round);

  void push(std::shared_ptr<MessageWrapper> message, const int round);

  std::shared_ptr<MessageWrapper> pop(const int round);

  void consume(const consumer2<MessageWrapper> &callback,
               const int round);

  supplier2<MessageWrapper> supply(const int round);

  void bringMessagesToRound(const int round);

  void clear();

 private:
  std::queue< std::shared_ptr<MessageWrapper> > buffers[2];
  boost::mutex lock[2];
 };
}


#endif //SECONDO_SWITCHABLEBUFFER_H
