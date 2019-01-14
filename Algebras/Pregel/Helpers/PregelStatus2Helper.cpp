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
1 Implementation of PregelStatus2Helper.

*/



#include "PregelStatus2Helper.h"
#include "SecondoSystem.h"
#include <assert.h>

template<class T>
ListExpr getAttr(std::string name, const T* t){
  char f = name[0];
  if(f>='a' && f<='z'){
    f = f + 'A'-'a';
    name[0] = f;
  }
  return nl->TwoElemList(nl->SymbolAtom(name), nl->SymbolAtom(T::BasicType()));

}

template<class T>
void setRes(Tuple* res, int attrNo, T* attr){
   if(attr){
      res->PutAttribute(attrNo,attr->Copy());
   } else {
      res->PutAttribute(attrNo, new T(false));           
   }
}

template<class T>
void readFromTuple(Tuple* tuple, const int attrPos, T*& value){
    T* v = (T*) tuple->GetAttribute(attrPos);
    if(value){
       value->CopyFrom(v);
    } else {
       value = (T*) v->Copy();         
    }            
}        


template<class T>
void del(T*& v){
   if(v){        
     v->DeleteIfAllowed();
     v = 0;
   }  
}        


/*
1.1  constructor 

*/
PregelStatus2Helper::PregelStatus2Helper():
  tupleType(0), host(0), port(0), pid(0),
  addressIndex(0), functionText(0),        
  superStep(0), messageType(0), messageTypeNumeric(0),
  messagesSent(0),  messagesDirect(0),  messagesSentPerSuperstep(0), 
  messagesReceived(0),  messagesReceivedPerSuperStep(0), 
  messagesDiscarded(0),  timeProductive(0),  timeIdle(0), 
  productivity(0) {} 

/*
1.2 Destructor 

*/
  PregelStatus2Helper::~PregelStatus2Helper(){
    reset();          
    if(tupleType) tupleType->DeleteIfAllowed();
  }


/*

1.3 Setter

*/

     void PregelStatus2Helper::setHost(const std::string& _host) {
         set(host,_host);
     }
     void PregelStatus2Helper::setPort(const int _port) {
        set(port,_port);
     }             
     void PregelStatus2Helper::setPID(const int _pid) {
        set(pid,_pid);
     }             
     void PregelStatus2Helper::setAddressIndex(const int i){
             set(addressIndex,i);
     }
     void PregelStatus2Helper::setFunctionText(const std::string& f){
             set(functionText,f);
     }

     void PregelStatus2Helper::setSuperStep(const int _superStep){
        set(superStep,_superStep);     
     }
     void PregelStatus2Helper::setMessageType(const std::string& _messageType){
         set(messageType,_messageType);
     }
     void PregelStatus2Helper::setMessageTypeNumeric(const std::string& mtn){
         set(messageTypeNumeric,mtn);
     }
     void PregelStatus2Helper::setMessagesSent(const int ms){
         set(messagesSent,ms);
     }
     void PregelStatus2Helper::setMessagesDirect(const int md){
         set(messagesDirect,md);
     }             
     void PregelStatus2Helper::setMessagesSentPerSuperstep(const double mspss){
         set(messagesSentPerSuperstep,mspss);
     }             
     void PregelStatus2Helper::setMessagesReceived(const int mr){
       set(messagesReceived,mr);
     }
     void PregelStatus2Helper::setMessagesReceivedPerSuperStep(
                     const double mrpss){
       set(messagesReceivedPerSuperStep,mrpss);
     }
     void PregelStatus2Helper::setMessagesDiscarded(const int md){
        set(messagesDiscarded,md);
     }
     void PregelStatus2Helper::setTimeProductive(const double tp){
       set(timeProductive,tp);
     }
     void PregelStatus2Helper::setTimeIdle(const double ti){
       set(timeIdle,ti);
     }
     void PregelStatus2Helper::setProductivity(const double p){
       set(productivity,p);
     }

     ListExpr PregelStatus2Helper::getTupleType(){
        ListExpr attrList = nl->OneElemList( getAttr("Host",host));
        ListExpr last = attrList;
        last = nl->Append(last, getAttr("Port",port));
        last = nl->Append(last, getAttr("Pid", pid));
        last = nl->Append(last, getAttr("addressIndex", addressIndex));
        last = nl->Append(last, getAttr("functionText", functionText));
        last = nl->Append(last, getAttr("SuperStep", superStep));
        last = nl->Append(last, getAttr("MessageType", messageType));
        last = nl->Append(last, getAttr("messageTypeNumeric",
                                        messageTypeNumeric));
        last = nl->Append(last, getAttr("messagesSent",messagesSent));
        last = nl->Append(last, getAttr("messagesDirect", messagesDirect));
        last = nl->Append(last, getAttr("messagesSentPerSuperstep",
                                        messagesSentPerSuperstep));
        last = nl->Append(last, getAttr("messagesReceived",messagesReceived));
        last = nl->Append(last, getAttr("messagesReceivedPerSuperStep",
                                        messagesReceivedPerSuperStep));
        last = nl->Append(last, getAttr("messagesDiscarded",messagesDiscarded));
        last = nl->Append(last, getAttr("timeProductive", timeProductive));
        last = nl->Append(last, getAttr("timeIdle",timeIdle));
        last = nl->Append(last, getAttr("productivity",productivity));
        return Tuple::wrap(attrList);
     }             


     Tuple* PregelStatus2Helper::getTuple(){
        if(!tupleType){
          ListExpr numType = 
                   SecondoSystem::GetCatalog()->NumericType(getTupleType());
          tupleType = new TupleType(numType);
        }                
        Tuple* res = new Tuple(tupleType);
        assert(res->GetNoAttributes()==17);

        setRes(res,0,host);
        setRes(res,1,port);
        setRes(res,2,pid);
        setRes(res,3,addressIndex);
        setRes(res,4,functionText);
        setRes(res,5,superStep);
        setRes(res,6,messageType);
        setRes(res,7,messageTypeNumeric);
        setRes(res,8,messagesSent);
        setRes(res,9,messagesDirect);
        setRes(res,10,messagesSentPerSuperstep);
        setRes(res,11,messagesReceived);
        setRes(res,12,messagesReceivedPerSuperStep);
        setRes(res,13,messagesDiscarded);
        setRes(res,14,timeProductive);
        setRes(res,15,timeIdle);
        setRes(res,16,productivity);

        return res;
     }             

     void PregelStatus2Helper::setHost(Tuple* tuple,const std::string& _host) {
        tuple->PutAttribute(0, new FText(true,_host));
     }
     void PregelStatus2Helper::setPort(Tuple* tuple,const int _port) {
        tuple->PutAttribute(1,new CcInt(true,_port));
     }             
     void PregelStatus2Helper::setPID(Tuple* tuple,const int _pid) {
        tuple->PutAttribute(2,new CcInt(_pid));
     }             

     void PregelStatus2Helper::createFromTuple(Tuple* tuple, 
                                               PregelStatus2Helper*& res) {
         if(res==0){
            res = new PregelStatus2Helper();
         }
         readFromTuple(tuple,0,res->host);
         readFromTuple(tuple,1,res->port);
         readFromTuple(tuple,2,res->pid);
         readFromTuple(tuple,3,res->addressIndex);
         readFromTuple(tuple,4,res->functionText);
         readFromTuple(tuple,5,res->superStep);
         readFromTuple(tuple,6,res->messageType);
         readFromTuple(tuple,7,res->messageTypeNumeric);
         readFromTuple(tuple,8,res->messagesSent);
         readFromTuple(tuple,9,res->messagesDirect);
         readFromTuple(tuple,10,res->messagesSentPerSuperstep);
         readFromTuple(tuple,11,res->messagesReceived);
         readFromTuple(tuple,12,res->messagesReceivedPerSuperStep);
         readFromTuple(tuple,13,res->messagesDiscarded);
         readFromTuple(tuple,14,res->timeProductive);
         readFromTuple(tuple,15,res->timeIdle);
         readFromTuple(tuple,16,res->productivity);
     }             


     PregelStatus2Helper* 
      PregelStatus2Helper::createFromTupleDesc(ListExpr tuple) {
        PregelStatus2Helper* h = new PregelStatus2Helper();
        ListExpr typeInfo = h->getTupleType();
        int errorPos=0;
        ListExpr errorInfo = listutils::emptyErrorInfo();
        bool correct;
        Tuple* t = Tuple::In(typeInfo, tuple, errorPos, errorInfo, correct);
        if(!correct){
          delete h;
          return 0; 
        }                
        createFromTuple(t,h);
        t->DeleteIfAllowed();
        return h;
     }

     void PregelStatus2Helper::reset(){
       del(port);
       del(pid);
       del(addressIndex);
       del(functionText);
       del(superStep);
       del(messageType);
       del(messageTypeNumeric);
       del(messagesSent);
       del(messagesDirect);
       del(messagesSentPerSuperstep);
       del(messagesReceived);
       del(messagesReceivedPerSuperStep);
       del(messagesDiscarded);
       del(timeProductive);
       del(timeIdle);
       del(productivity);
     }             



