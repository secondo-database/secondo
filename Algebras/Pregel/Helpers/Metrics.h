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

This header file defines the class Metrics

2 Defines and includes

*/

#pragma once

#define GATHER_PREGEL_METRICS

#ifdef GATHER_PREGEL_METRICS
#define SENT_MESSAGE Metrics::get().sentMessage();
#define QUEUED_MESSAGE Metrics::get().queuedMessage();
#define RECEIVED_MESSAGE Metrics::get().receivedMessage();
#define DISCARDED_MESSAGE Metrics::get().discardedMessage();

#define PRODUCTIVE Metrics::get().startClock(true);
#define IDLE Metrics::get().startClock(false);
#define STOP_CLOCK Metrics::get().stopClock();
#else
#define SENT_MESSAGE do {} while(0);
#define QUEUED_MESSAGE do {} while(0);
#define RECEIVED_MESSAGE do {} while(0);
#define DISCARDED_MESSAGE do {} while(0);

#define PRODUCTIVE do {} while(0);
#define IDLE do {} while(0);
#define STOP_CLOCK do {} while(0);
#endif

#include "../PregelContext.h"
#include "../Helpers/PregelStatus2Helper.h"
#include <chrono>

namespace pregel {
#define TIME_ZERO \
std::chrono::duration_values<std::chrono::milliseconds>::zero()

 class Metrics {
 private:
  static Metrics singleton;

  unsigned int messagesSentOverNetwork;
  unsigned int messagesQueuedDirectly;
  unsigned int messagesReceived;
  unsigned int messagesDiscarded;
  std::chrono::milliseconds timeIdle;
  std::chrono::milliseconds timeProductive;

  struct StopWatch {
   std::chrono::milliseconds clock = TIME_ZERO;
   bool countingProductiveTime;
  };
  
  StopWatch stopWatch;

 public:
  struct Report {
   unsigned int messagesSentOverNetwork;
   unsigned int messagesQueuedDirectly;
   double messagesSentPerSuperstep;
   unsigned int messagesReceived;
   double messagesReceivedPerSuperstep;
   unsigned int messagesDiscarded;
   double timeIdle;
   double timeProductive;
   double productivity;

   void print(std::stringstream &sstream);
   void fill(PregelStatus2Helper& ps2h);
  };

  static Metrics &get();

  Report report();

  void sentMessage();

  void queuedMessage();

  void receivedMessage();

  void discardedMessage();

  void startClock(bool productive = true);

  void stopClock();
 };
}


