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

This header file defines the class MessageServer

2 Defines and includes

*/

#ifndef SECONDO_MESSAGESERVER_H
#define SECONDO_MESSAGESERVER_H


#include <functional>
#include <SocketIO.h>
#include <boost/thread.hpp>
#include "DoubleQueue.h"
#include "ComputeMonitor.h"
#include "../Helpers/typedefs.h"

namespace pregel {
 class MessageServer {
 private:
  enum State {
   READING,
   WAITING
  };

  State state = WAITING;
  boost::mutex stateLock;
  boost::condition_variable stateCondition;

  ComputeMonitor *monitor = nullptr;
  executable initDoneCallback;
  boost::thread *thread = nullptr;
  Socket *socket;

  DoubleQueue messageQueue;
  int round = 0;

 public:
  explicit MessageServer(Socket *socket, executable initDoneCallback);

  virtual ~MessageServer();

  void run();

  void startReading();

  void requestPause();

  void interrupt();

  void setMonitor(ComputeMonitor *monitor);

  void drainBuffer(const consumer<MessageWrapper> &consumer,
                   const int round);

  void healthReport(std::stringstream &sstream);

 private:
  void waitToResumeReading();

  void processMessage();

  void handleEmptyMessage();

  void handleFinishedMessage();

  void handleInitDoneMessage();

  void updateRound();

  void inline addMessage(MessageWrapper *message);

  bool stateIs(State state);

  void setState(State state, bool notify = true);
 };
}


#endif //SECONDO_MESSAGESERVER_H
