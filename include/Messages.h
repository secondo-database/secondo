/*
---- 
This file is part of SECONDO.

Copyright (C) 2007, University in Hagen, Faculty of Mathematics and Computer Science, 
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

Feb. 2007, M. Spiekermann: Class Listener and MessageHandler introduced 


   
*/   

#ifndef SEC_MESSAGES_H
#define SEC_MESSAGES_H

#include <string>
#include <iostream>
#include <list>

#include "NList.h"
#include "LogMsg.h"
#include "StopWatch.h"

#ifdef THREAD_SAFE
#include <boost/thread.hpp>
#endif




/*
1 Class ~MessageHandler~

An abstract base class which defines the properties
of a message handler.

*/

class MessageHandler {

  public:
  virtual bool handleMsg(NestedList* nl, ListExpr list) = 0;
  virtual void Flush() {} 
  MessageHandler() {}
  virtual ~MessageHandler() {}

protected:
#ifdef THREAD_SAFE
boost::mutex mtx;
#endif

  
}; 

/*
2 Class ~SimpleHandler~
  
A simple dmessage handler. This is an example implementation
of a message handler. It accepts every lists with a structure 

-----
     (simple ...)
-----

Its just for demonstration
purposes. Your own message handler classes can do more complex things. 
   
*/   

class SimpleHandler : public MessageHandler {

  public:
  virtual bool handleMsg(NestedList* nl, ListExpr list){
     #ifdef THREAD_SAFE
     boost::lock_guard<boost::mutex> guard(mtx);
     #endif

     if(!nl->HasMinLength(list,2)){
       return false;
     }
     if(!nl->IsEqual(nl->First(list),"simple")){
        return false;
     }
     cerr << "Message: " << nl->ToString(list);
     return true;
  }
 
  SimpleHandler() {};
  ~SimpleHandler() {};
 
}; 



/*

ProgMesHandler gets progress infos from the QueryProcessor to
control the visualization of the query progress by registered
clients (e.g. to make progress-points).

*/

class ProgMesHandler : public MessageHandler
{

  public:

  ~ProgMesHandler(){
     if(s){
       delete s;
       s = 0;
     }
  }

  //virtual bool handleMsg(NList msgList);
  virtual bool handleMsg(NestedList* nl, ListExpr list);
  ProgMesHandler():total(50),highest(-1),s(0) {};

  int total;  // len of the progress bar 
  int highest; // highest value ever received

  private:
    StopWatch* s;

};

/*
2 Class ~MessageCenter~

This class maintains the handler (or Listener). If someone calls the
~Send~ method all registered handler are called.

*/
class MessageCenter {

 typedef list<MessageHandler*> HandlerList;
 HandlerList msgHandler; 
 
 // There will be only one instance. Hence the constructor
 // is private. ~GetInstance~ will call it if necessary.
 MessageCenter() {} 
 static MessageCenter* msg;
 #ifdef THREAD_SAFE
 static boost::mutex mtx;
 #endif

 
 public:
 
  static MessageCenter* GetInstance() { 
    #ifdef THREAD_SAFE
    boost::lock_guard<boost::mutex> guard(mtx); 
    #endif
    if (!msg){
       msg = new MessageCenter();
    }
    return msg;
  }
 
  ~MessageCenter() 
  { 
    HandlerList::const_iterator it = msgHandler.begin();
    for (; it != msgHandler.end(); it++)
       delete *it;
  } 
  
  void CallHandler(NestedList* nl, ListExpr message) 
  {
    #ifdef THREAD_SAFE
    boost::lock_guard<boost::mutex> guard(mtx); 
    #endif
     
     HandlerList::const_iterator it = msgHandler.begin();
     for(; it != msgHandler.end(); it++) {
        (*it)->handleMsg(nl, message);
     } 
  } 
  

  void Flush() 
  {
    #ifdef THREAD_SAFE
    boost::lock_guard<boost::mutex> guard(mtx); 
    #endif
     HandlerList::const_iterator it = msgHandler.begin();
     for(; it != msgHandler.end(); it++) {
        (*it)->Flush();
     } 
  } 

  void Send(NestedList* nl, ListExpr message) 
  {
    //message.showNLRefs();
    CallHandler(nl, message); 
  } 
  
  void AddHandler(MessageHandler* handler) {

     msgHandler.push_back(handler);
  }

  void RemoveHandler(MessageHandler* handler){
     msgHandler.remove(handler);
  }

 
};

#endif
