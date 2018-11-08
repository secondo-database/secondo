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

This header file defines the class Monitor

2 Defines and includes

*/

#ifndef SECONDO_COMPUTEMESSAGINGCONTEXT_H
#define SECONDO_COMPUTEMESSAGINGCONTEXT_H

#include <utility>
#include <boost/thread/mutex.hpp>
#include <boost/thread/condition_variable.hpp>
#include <boost/log/trivial.hpp>

namespace pregel {
 class Monitor {
 public:
  Monitor(unsigned long numberOfServers,
                 std::function<void(bool empty)> finishCallback);

  void empty();

  void finish();

 private:
  unsigned long finishedWorkerCounter;
  std::function<void(bool empty)> callback;
  bool allEmpty = true;
  boost::mutex counterLock;
 };
}


#endif //SECONDO_COMPUTEMESSAGINGCONTEXT_H
