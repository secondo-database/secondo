
/*
----
This file is part of SECONDO.

Copyright (C) 2017,
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

#ifndef TIMEOUT_H
#define TIMEOUT_H


#include "Messages.h"
#include "NestedList.h"


namespace distributed2
{

template<class T>
class TimeoutNotifier {
  public:
     TimeoutNotifier(T* _timeoutReceiver){
        timeoutReceiver = _timeoutReceiver;
        runner = 0;
     }

    virtual ~TimeoutNotifier(){
      end();
    }

     void stop(){
        quit = true;
        if(runner){
           runner->interrupt();
        }
        end();
     }

     virtual void start( int seconds){
        if(runner) return;
        waiting_time = seconds;
        runner = new boost::thread(&TimeoutNotifier::run, this);
     }

  protected:
     size_t waiting_time;
     bool quit;
     T* timeoutReceiver;
     boost::thread* runner;
     

     virtual void run(){
        boost::this_thread::interruption_point();
        quit = false;
        boost::this_thread::interruption_point();
        sleep(waiting_time);
        boost::this_thread::interruption_point();
        if(quit) return;  
        if(timeoutReceiver){
           timeoutReceiver->timeout();
        }
     }

     void end(){
       if(runner){
          runner->detach();
          delete runner;
          runner = 0;
       }
     }

};


//  class for observing heartbeat messages
template<class T>
class HeartbeatObserver : public MessageHandler,  public TimeoutNotifier<T>{
  public:
     HeartbeatObserver(T* _timeoutReceiver): 
            TimeoutNotifier<T>(_timeoutReceiver){
     }

    virtual ~HeartbeatObserver(){
      TimeoutNotifier<T>::end();
    }

    bool handleMsg(NestedList* nl, ListExpr msg, int source){

       if(! nl->HasLength(msg,2)) {  return false; }
       if(    (nl->AtomType(nl->First(msg))!=SymbolType)
           || (nl->AtomType(nl->Second(msg))!=IntType)){
           return false;
       }
       if(  (nl->SymbolValue(nl->First(msg))!="heartbeat")) {
          return false;
       } 
       int v = nl->IntValue(nl->Second(msg));
       beatReceived = true;
       if(v==0){ TimeoutNotifier<T>::quit = true; }
       return true;
     }

     void start( int seconds){
        if(TimeoutNotifier<T>::runner) return;
        TimeoutNotifier<T>::waiting_time = seconds*1000000;
        TimeoutNotifier<T>::runner = 
                   new boost::thread(&HeartbeatObserver::run, this);
     }

  private:
     bool beatReceived;
     
     virtual void run(){
        beatReceived = true;
        boost::this_thread::interruption_point();
        TimeoutNotifier<T>::quit = false;
        boost::this_thread::interruption_point();
        while(beatReceived){
           beatReceived = false;
           usleep(TimeoutNotifier<T>::waiting_time);
           if(TimeoutNotifier<T>::quit) return;  
        } 
        boost::this_thread::interruption_point();
        // timeout recognized
        if(TimeoutNotifier<T>::timeoutReceiver){
           TimeoutNotifier<T>::timeoutReceiver->timeout();
        }
     }

};

} // end of namespace

#endif

