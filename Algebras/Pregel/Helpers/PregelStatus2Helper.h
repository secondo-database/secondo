/*
----
This file is part of SECONDO.

Copyright (C) 2019,
University of Hagen,
Faculty of Mathematics and Computer Science,
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

*/

/*

1 PregelStatus2Helper

This class helps to create the result tuples of the 
pregelStatus2 Operator.

Here, all information is collected independently whether it
comes from the master or from a worker. Unused attributes are set to 
be undefined.


1.1 Includes

*/



#pragma once

#include <string>
#include "Algebras/Relation-C++/RelationAlgebra.h"
#include "Algebras/FText/FTextAlgebra.h"
#include "StandardTypes.h"
#include "NestedList.h"

/*
1.2 Class Definition

*/

class PregelStatus2Helper{
  public:
/*
1.2.1 Constructor, Destructor

*/
     PregelStatus2Helper();
     ~PregelStatus2Helper();

/*
1.2.2 Setter

*/
     void setHost(const std::string& host);
     void setPort(const int port);
     void setPID(const int pid);

     void setAddressIndex(const int i);
     void setFunctionText(const std::string& f);
     void setSuperStep(const int superStep);
     void setMessageType(const std::string& messageType);
     void setMessageTypeNumeric(const std::string& mtn);
     void setMessagesSent(const int ms);
     void setMessagesDirect(const int md);
     void setMessagesSentPerSuperstep(const double mspss);
     void setMessagesReceived(const int mr);
     void setMessagesReceivedPerSuperStep(const double mrpss);
     void setMessagesDiscarded(const int md);
     void setTimeProductive(const double tp);
     void setTimeIdle(const double ti);
     void setProductivity(const double p);

/*
1.2.3  Functions setting values in result tuple directly 

*/
     static void setHost(Tuple* tuple, const std::string& host);
     static void setPort(Tuple* tuple, const int port);
     static void setPID(Tuple* tuple, const int pid);
     
/*
1.2.4 ~getTuple~

Returns this instance in a tuple representation.

*/
     Tuple* getTuple();

/*
1.2.5 ~getTupleType~

Returns the type of the tuple for the tuple representation.

*/     
     ListExpr getTupleType();

/*
1.2.6 ~reset~

Clears the content of this instance.

*/
     void reset();

/*
1.2.7 Reading out tuples.

These functions create a new instance using the valuse from a tuple represnetation.

*/
     static void createFromTuple(Tuple* tuple, PregelStatus2Helper*& res);
     static PregelStatus2Helper* createFromTupleDesc(ListExpr tuple);

  private:
     TupleType* tupleType;
     // Information about the worker
     FText* host;
     CcInt* port;
     CcInt* pid;
     // general information
     CcInt* addressIndex;
     FText* functionText;
     CcInt* superStep;
     FText* messageType;
     FText* messageTypeNumeric;
     CcInt* messagesSent;
     CcInt* messagesDirect;
     CcReal* messagesSentPerSuperstep;
     CcInt* messagesReceived;
     CcReal* messagesReceivedPerSuperStep;
     CcInt*  messagesDiscarded;
     CcReal* timeProductive;
     CcReal* timeIdle;
     CcReal* productivity;
     // to be continued
     //
     //
     template<class M, typename V>
     void set(M*& m, const V& v){
        if(m) {
          m->Set(true,v);
        } else {
          m = new M(true,v);
        }    
     }       
};  


