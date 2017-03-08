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


#ifndef MEMORYSTACK

#include "MainMemoryExt.h"
#include <stack>
#include "Attribute.h"
#include "PointerWrapper.h"


namespace mm2algebra{



class MemoryStackObject : public MemoryObject {

    public:

        typedef PointerWrap<Tuple> mstackentry;

        MemoryStackObject(bool _flob, 
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


        ~MemoryStackObject() {
           while(!s.empty()){
             s.pop();
           }
           if(tt){
              tt->DeleteIfAllowed();
           }
        }

        
        static const std::string BasicType() { return "mstack"; }

        static bool checkType(ListExpr t){
          return nl->HasLength(t,2) 
                 && listutils::isSymbol(nl->First(t),BasicType())
                 && Tuple::checkType(nl->Second(t));
        }


        size_t size() const{
          return s.size();
        }

        const Tuple* top(){
           Tuple* t = s.top().getPointer(); 
           t->IncReference();
           return t;
        }

        Tuple* pop(){
           Tuple* t = s.top().getPointer();
           t->IncReference(); 
           s.pop();
           return t;
        }

        void push(Tuple* t){
          if(flob){
              t->bringToMemory();
          }
          s.push(mstackentry(t));
        }

        bool empty() const{
           return s.empty();
        }

        TupleType* getTupleType(){
            return tt;
        }


    private:
        std::stack<mstackentry> s;
        TupleType* tt;
};


}


#endif

