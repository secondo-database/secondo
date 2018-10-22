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

This header file contains definitions of the PregelContext class

2 Defines and includes

*/

#ifndef SECONDO_PREGELCONTEXT_H
#define SECONDO_PREGELCONTEXT_H

#include <GenericTC.h>
#include <ostream>
#include "typedefs.h"
#include "Helpers/WorkerConfig.h"

namespace pregel {
 /*
  * 3 PregelContext class
  *
  * */
 class PregelContext {
 private:
  /*
   * 3.1 Singleton instance
   *
   * */
  static PregelContext context;

  /*
  * 3.2 member variables
  *
  * */
  std::string messageType = "";
  std::string numericMessageType = "";
  int addressIndex = -1;

  int superstep = 0;
  std::string function = "";

  int messageServerPort = 0;
  std::vector<WorkerConfig> workers = std::vector<WorkerConfig>();

 public:
  /*
  * 3.3 Singleton accessor
  *
  * */
  static PregelContext &get();

  /*
  * 3.4 Reset function
  *
  * */
  void reset() {
   messageType = "";
   numericMessageType = "";
   addressIndex = -1;
   superstep = 0;
   function = "";

   workers.clear();
   messageServerPort = 0;
  }

  /*
  * 3.5 Status report
  *
  * */
  void healthReport(std::stringstream &sstream) {
   sstream << "  AddressIndex: " << addressIndex << std::endl;
   sstream << "  FunctionText: " << function << std::endl;
   sstream << "  Superstep: " << superstep << std::endl;
   sstream << "  MessageType          : " << messageType << std::endl;
   sstream << "  MessageType (numeric):" << numericMessageType << std::endl;
   sstream << std::endl;
  }

  /*
  * 3.6 Member Accessors
  *
  * */
  std::string &getMessageType();

  void setMessageType(ListExpr messageTypeAsList);

  void setNumericMessageType(ListExpr messageType);

  std::string &getNumericMessageType() {
   return numericMessageType;
  }

  const int inline getCurrentSuperstep() {
   return superstep;
  }

  void setRoundCounter(const int round);

  void increaseSuperstepCounter();

  int getAddressIndex() const;

  void setAddressIndex(int);

  int getMessageServerPort() const;

  void setMessageServerPort(int);

  const std::string &getFunction() const;

  void setFunction(std::string &);

  /*
  * 3.7 Worker information supplier
  *
  * */
  supplier<WorkerConfig> getWorkers();

  supplier<WorkerConnection> getConnections();

  void addWorker(WorkerConfig worker);

  /*
  * 3.8 Stream concatenation operator
  *
  * */
  friend std::ostream &operator<<(std::ostream &os,
                                  const PregelContext &context);

 };

 /*
  * 4 SuperstepCounter class
  *
  * */
 class SuperstepCounter {
 public:
  static int get();

  static int increment();

  static void invalidate();
 };
}


#endif //SECONDO_PREGELCONTEXT_H
