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
#include "Helpers/PregelStatus2Helper.h"
#include "Algebras/Relation-C++/RelationAlgebra.h"

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
  TupleType* tupleType = 0;
  int addressIndex = -1;

  int superstep = 0;
  std::string function = "";

  int messageServerPort = 0;
  std::vector<WorkerConfig> workers = std::vector<WorkerConfig>();

  enum Phase {
   INITIAL,
   SET_UP,
   READY
  };

  Phase phase = INITIAL;

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
   if(tupleType){
     tupleType->DeleteIfAllowed();
     tupleType = 0;
   }
   addressIndex = -1;
   superstep = 0;
   function = "";

   workers.clear();
   messageServerPort = 0;

   phase = INITIAL;
  }

  ~PregelContext(){
     if(tupleType){
       tupleType->DeleteIfAllowed();
     }
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
   sstream << std::endl;
  }
  
  void healthReport(PregelStatus2Helper& ps2h) {
     ps2h.setAddressIndex(addressIndex);
     ps2h.setFunctionText(function);
     ps2h.setSuperStep(superstep); 
     ps2h.setMessageType(messageType);
     ps2h.setMessageTypeNumeric(messageType);  
  }

  /*
  * 3.6 Member Accessors
  *
  * */
  std::string &getMessageType();

  void setMessageType(ListExpr messageTypeAsList);

  void setNumericMessageType(ListExpr messageType);

  TupleType* &getTupleType() {
   return tupleType;
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

  bool inline isSetUp() {
   return phase == SET_UP;
  }

  void inline setUp() {
   phase = SET_UP;
  }

  bool inline isReady() {
   return phase == READY;
  }

  void inline ready() {
   phase = READY;
  }

  /*
  * 3.7 Worker information supplier
  *
  * */
  supplier<WorkerConfig> getWorkers();

  supplier<WorkerConnection> getConnections();

  void addWorker(WorkerConfig worker) noexcept(false);

  bool workerExists(RemoteEndpoint &endpoint,
                    int messageServerPort);


  std::ostream& print(std::ostream& os) const{
    os << " function: " << function 
       << " routes: " << workers.size() << std::endl;
    for (auto worker : workers) {
      worker.print(os) << std::endl;
    }
    return os;
  }


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
