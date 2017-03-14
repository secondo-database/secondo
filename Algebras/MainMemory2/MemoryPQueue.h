/*

----
This file is part of SECONDO.

Copyright (C) 2017, 
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

*/


#ifndef MEMORYPQUEUE_H
#define MEMORYPQUEUE_H

#include "MainMemoryExt.h"
#include <vector>
#include "Attribute.h"


namespace mm2algebra{

class pqueueentry{
  public:
    pqueueentry(Tuple* tup, double p): tuple(tup),priority(p){
       tuple->IncReference();
    }

    pqueueentry(const pqueueentry& src):
        tuple(src.tuple),priority(src.priority){
       if(tuple){
          tuple->IncReference();
       }
    }
        
    pqueueentry& operator=(const pqueueentry& src){
        if(tuple){
           tuple->DeleteIfAllowed();
        }
        tuple = src.tuple;
        priority = src.priority;
        if(tuple){
          tuple->IncReference();
        }
        return *this;
    }

    ~pqueueentry(){
       if(tuple){
          tuple->DeleteIfAllowed();
       }
    }

    bool operator<(const pqueueentry& rhs)const{
        return priority > rhs.priority;
    }

    bool operator>(const pqueueentry& rhs)const{
        return priority < rhs.priority;
    }

    Tuple* operator()(){
       tuple->IncReference();
       return tuple;
    }

    inline TupleType* getTupleType(){
      return tuple->GetTupleType();
    }

    inline double getPrio() const{
       return priority;
    }

 private:
  Tuple* tuple;
  double priority;

};


class MemoryPQueueObject : public MemoryObject {

    public:

        MemoryPQueueObject(bool _flob, 
                           const std::string& db,
                           const std::string& _type ):
         MemoryObject(_flob, db, _type) {
           flob = _flob;
           ListExpr tl;
           if(!nl->ReadFromString(_type,tl)){
             assert(false);
           }
           ListExpr tln = SecondoSystem::GetCatalog()->NumericType(
                                             nl->Second( nl->Second(tl)));
           tt = new TupleType(tln);
        }


        ~MemoryPQueueObject() {
           while(!queue.empty()){
             queue.pop();
           }
           if(tt){
              tt->DeleteIfAllowed();
           }
        }

        
        static const std::string BasicType() { return "mpqueue"; }

        static bool checkType(ListExpr t){
          return nl->HasLength(t,2) 
                 && listutils::isSymbol(nl->First(t),BasicType())
                 && Tuple::checkType(nl->Second(t));
        }


        size_t size() const{
          return queue.size();
        }

        const pqueueentry& top(){
           return queue.top(); 
        }

        pqueueentry pop(){
           pqueueentry res = queue.top();
           queue.pop();
           return res;
        }

        void push(Tuple* t, double d){
          queue.push(pqueueentry(t,d));
        }

        bool empty() const{
           return queue.empty();
        }

        TupleType* getTupleType(){
            return tt;
        }

        void swapQueue(std::priority_queue<pqueueentry> & q){
           std::swap(q,queue);
        }


        typedef std::priority_queue<pqueueentry> queue_t;

    private:
        queue_t queue;
        TupleType* tt;
};


}


#endif

