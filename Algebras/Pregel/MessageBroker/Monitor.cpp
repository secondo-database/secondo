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

This file defines the members of class ComputeMonitor

*/

#include "Monitor.h"

pregel::Monitor::Monitor(unsigned long numberOfServers,
                                       std::function<void(
                                        bool empty)> finishCallback)
 : finishedWorkerCounter(numberOfServers),
   callback(finishCallback), counterLock() {}

void pregel::Monitor::empty() {
 boost::lock_guard<boost::mutex> guard(counterLock);
 --finishedWorkerCounter;
 bool allFinished;
 allFinished = finishedWorkerCounter == 0;

 if (!allFinished) {
  return;
 }
 callback(allEmpty);
}

void pregel::Monitor::finish() {
 boost::lock_guard<boost::mutex> guard(counterLock);
 allEmpty = false;
 --finishedWorkerCounter;
 bool allFinished;
 allFinished = finishedWorkerCounter == 0;

 if (!allFinished) {
  return;
 }
 callback(false);
}