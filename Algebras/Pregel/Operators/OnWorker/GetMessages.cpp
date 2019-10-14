/*
----
This file is part of SECONDO.

Copyright (C) 2019, 
University in Hagen, 
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

//paragraph [1] Title: [{\Large \bf \begin {center}] [\end {center}}]
//[TOC] [\tableofcontents]

[1] Header File of the class ~PregelAlgebra~



[TOC]

1 Overview

This file contains definitions of the members of class GetMessages 

*/

#include <ListUtils.h>
#include "Stream.h"
#include "Algebras/Relation-C++/RelationAlgebra.h"
#include <StandardTypes.h>
#include "GetMessages.h"
#include "../../PregelAlgebra.h"
#include "../../../FText/FTextAlgebra.h"
#include "Algebras/Distributed2/CommandLog.h"
#include "../../MessageBroker/MessageBroker.h"
#include "../../MessageBroker/MessageWrapper.h"
#include "../../MessageBroker/DoubleQueue.h"


namespace pregel {

 template<class T>
 ListExpr getAttr(const std::string& name){
     return nl->TwoElemList( nl->SymbolAtom(name), listutils::basicSymbol<T>());
 }         


 ListExpr GetMessages::typeMapping(ListExpr args) {
     std::string messageTupleType = PregelContext::get().getMessageType();
     if(messageTupleType.empty()){
        return listutils::typeError("message type is not defined yet");
     }
     ListExpr type = nl->TheEmptyList();
     if(!nl->ReadFromString(messageTupleType,type)){
        return listutils::typeError("message type cannot be parsed");
     }
     if(!Tuple::checkType(type)){
       return listutils::typeError("internal error - not a message "
                                   "type is not tuple :"
                                   + nl->ToString(type));
     }

     ListExpr additionalAttributes = nl->OneElemList(
                                          getAttr<CcInt>("Msg_Destination"));
     ListExpr last = additionalAttributes;
     last = nl->Append(last, getAttr<CcString>("Msg_Type"));
     last = nl->Append(last, getAttr<CcInt>("Msg_Round"));
     last = nl->Append(last, getAttr<FText>("Msg_Host"));
     last = nl->Append(last, getAttr<CcInt>("Msg_Port"));
     last = nl->Append(last, getAttr<CcInt>("Msg_PID"));

     ListExpr attrList = listutils::concat(nl->Second(type), 
                                           additionalAttributes);
     ListExpr resType =  Stream<Tuple>::wrap(Tuple::wrap(attrList));
     return resType;
 }


class GetMessagesInfo{

  public:
      GetMessagesInfo(ListExpr _tupleType, ListExpr _tt){

         tt = new TupleType(_tt);     
         tupleType = _tupleType;
         numTupleType = SecondoSystem::GetCatalog()->NumericType(tupleType);
         numTupleType = nl->TwoElemList( numTupleType, 
                                      nl->ListLength(nl->Second(numTupleType)));
         workers = PregelContext::get().getWorkers();
         msgqueue1 = MessageBroker::get().getInBox().getQueue(0);
         msgqueue2 = MessageBroker::get().getInBox().getQueue(1);
         lastRelValue = nl->TheEmptyList();
         query = "query getPregelMessages() consume";
      }


      ~GetMessagesInfo(){
          tt->DeleteIfAllowed();
      }

      Tuple* next() {
         Tuple* res = getTupleOnWorker();
         if(res) return res;
         return getTupleOnMaster();
      }              

  private:
     ListExpr tupleType;
     ListExpr numTupleType;
     TupleType* tt;
     supplier<WorkerConfig> workers; 
     ListExpr lastRelValue;
     std::string query;
     distributed2::CommandLog cmdLog;
     WorkerConnection* con; // last used connection
     MessageQueue msgqueue1;
     MessageQueue msgqueue2;


     Tuple* getTupleOnMaster(){
         while(nl->IsEmpty(lastRelValue)){
            WorkerConfig* wc = workers();
            if(wc==nullptr) return 0; // no more workers available

            con = wc->connection;
            // send query to worker
            int error;
            std::string errorMsg;
            ListExpr resList;
            double runTime;
            con->simpleCommand(query,
                   error, errorMsg,
                   resList,
                   false,
                   runTime,
                   false,
                   cmdLog,
                   true,
                   0);
            if(!error){
               assert(nl->HasLength(resList,2));
               ListExpr resType = nl->First(resList);
               lastRelValue = nl->Second(resList);
               assert(Relation::checkType(resType));
               assert(nl->Equal(tupleType,nl->Second(resType)));
            } else {
               std::cerr << "error during requesting pregel "
                            "messages from worker"
                         << std::endl;
               std::cerr << errorMsg << std::endl;
            }
         }                
         ListExpr tupleValue = nl->First(lastRelValue);
         lastRelValue = nl->Rest(lastRelValue);
         // read in tuple
         int errorPos=0;
         ListExpr errorInfo = listutils::emptyErrorInfo();
         bool correct;
         Tuple* res = Tuple::In(numTupleType, tupleValue,errorPos,
                                errorInfo, correct);

         // set/overwrite the last three attributes in tuple
         int pos = res->GetNoAttributes() - 3;
         res->PutAttribute(pos, new FText(true, con->getHost()));
         pos++;
         res->PutAttribute(pos, new CcInt(true, con->getPort()));
         pos++;
         res->PutAttribute(pos, new CcInt(true, con->serverPid()));
         pos++;
         return res;  
     }


     Tuple* getTupleOnWorker(){
        if(!msgqueue1.empty()){
          std::shared_ptr<MessageWrapper> mw = msgqueue1.front();
          msgqueue1.pop();
          return getTupleOnWorker(mw);
        }         
        if(!msgqueue2.empty()){
          std::shared_ptr<MessageWrapper> mw = msgqueue2.front();
          msgqueue2.pop();
          return getTupleOnWorker(mw);
        }         
        return 0;
     }

     Tuple* getTupleOnWorker(std::shared_ptr<MessageWrapper> mw){
         Tuple* res = new Tuple(tt);
         Tuple* msg = mw->getBody1();
         int pos = res->GetNoAttributes()-6;
         if(msg){ // overtake attributes from msg to res
           for(int i=0;i<msg->GetNoAttributes();i++){
             res->CopyAttribute(i,msg,i);
           }
           msg->DeleteIfAllowed();
         } else { // create undefined attributes
           ListExpr attrList = nl->Second(tupleType);
           int p = 0;
           AlgebraManager* am = SecondoSystem::GetAlgebraManager();
           while(!nl->IsEmpty(attrList)){
              ListExpr attrType = nl->First(attrList);
              const AttributeType& at = tt->GetAttributeType(p);
              attrList = nl->Rest(attrList);
              p++;
              ObjectCreation cr = am->CreateObj(at.algId, at.typeId);
              Attribute* attr = static_cast<Attribute*> (cr(attrType).addr);
              attr->SetDefined(false);
              res->PutAttribute(p,attr);
           }
         }
         // store the remaining values
         res->PutAttribute(pos,new CcInt(true, mw->getDestination()));
         pos++;
         res->PutAttribute(pos, new CcString(true,
               MessageWrapper::typToString(mw->getType())));
         pos++;
         res->PutAttribute(pos, new CcInt(true,mw->getRound()));
         pos++;
         res->PutAttribute(pos, new FText(false,"")); // host
         pos++;
         res->PutAttribute(pos, new CcInt(false,0)); // port
         pos++;
         res->PutAttribute(pos, new CcInt(false, 0)); // pid
         pos++;
         return res;
     }



};


 int GetMessages::valueMapping(Word*, Word &result, int message,
                                Word & local, Supplier s) {

    GetMessagesInfo* li = (GetMessagesInfo*) local.addr;
    switch(message){
       case OPEN :  if(li){
                       delete li;
                    }
                    local.addr = new GetMessagesInfo(
                                        nl->Second(qp->GetType(s)),
                                        nl->Second(GetTupleResultType(s)));
                    return 0;
       case REQUEST : result.addr = li?li->next():0;
                      return result.addr?YIELD:CANCEL;
       case CLOSE : if(li){
                      delete li;
                      local.addr = 0;
                    }                
                    return 0;            
    }            
    return -1; // should never be reached

 }



 OperatorSpec GetMessages::operatorSpec(
 " -> stream(tuple)",
 "_ ()",
 "Returns a tuple stream containing the current messages of the pregel system",
 "query getPregelMessages() consume;"
 );

 Operator GetMessages::getMessages(
  "getPregelMessages",
  GetMessages::operatorSpec.getStr(),
  GetMessages::valueMapping,
  Operator::SimpleSelect,
  GetMessages::typeMapping
 );
}
