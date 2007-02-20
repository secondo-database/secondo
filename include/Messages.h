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

/*
1 Class ~MessageHandler~

An abstract base class which defines the properties
of a message handler.

*/

class MessageHandler {

  public:
  virtual bool handleMsg(NList msgList) = 0;
 
  MessageHandler() {}
  virtual ~MessageHandler() {}
  
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
  virtual bool handleMsg(NList msgList) {

    //msgList.showNLRefs();
    if ( !msgList.first().isSymbol("simple") )
      return false;
    
    cerr << "Message: " << msgList << endl; 
    return true;
  } 
  SimpleHandler() {};
  ~SimpleHandler() {};
 
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
 
 public:
 
  static MessageCenter* GetInstance() { 
   if (!msg)
     msg = new MessageCenter();
   return msg;
  }
 
  ~MessageCenter() 
  { 
    HandlerList::const_iterator it = msgHandler.begin();
    for (; it != msgHandler.end(); it++)
       delete *it;
  } 
  
  void CallHandler(NList message) 
  {
     HandlerList::const_iterator it = msgHandler.begin();
     for(; it != msgHandler.end(); it++) {
        (*it)->handleMsg(message);
     } 
  } 

  void Send(NList message) 
  {
    //message.showNLRefs();
    CallHandler(message); 
  } 
  
  void AddHandler(MessageHandler* handler) {

     msgHandler.push_back(handler);
  } 
};

#endif
