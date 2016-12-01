/*

----
This file is part of SECONDO.

Copyright (C) 2016, 
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


/*
1 MPointer

The class MPointer represents a simple Secondo object
holding a pointer to an object in main memory catalog. 
Such an object cannot be stored persistently, because after 
a restart of secondo, the pointer becomes invalid. For
similar  reasons, the IN function returns always a null pointer. 

This class is to accelerate repeated access to main memory 
structures. For example, when accessing a main memory 
index within a loop join, each time the index is accessed,
the index will be get out from the catalog, the type is checked
and so on. This takes a lot of time. To avoid such, the pwrap
operator can be used to wrap the pointer to the object and thus
type checking and retrieving the object from the catalog is only 
made once.


*/


#ifndef MPOINTER_H
#define MPOINTER_H

#include "NestedList.h"
#include "ListUtils.h"
#include "AlgebraTypes.h"
#include "Mem.h"

namespace mm2algebra{
  class MemoryObject;


class MPointer{

public:

/*
1.1 Coinstructors and Destructors

*/
   MPointer(){} 
   MPointer(MemoryObject* v): thePointer(v){} 
   MPointer(const MPointer& p):thePointer(p.thePointer){}
   ~MPointer(){}


/*
1.2 Functions supporting embedding in Secondo

*/
   static const std::string BasicType(){ 
       return "mpointer";
   }
   static const bool checkType(const ListExpr arg){
     if(!nl->HasLength(arg,2)){
       return false;
     }
     if(!listutils::isSymbol(nl->First(arg),BasicType())){
       return false;
     }
     return Mem::checkType(nl->Second(arg));;
   }


   static ListExpr Property(){
    return  nl -> TwoElemList (
       nl -> FourElemList (
       nl -> StringAtom ( " Signature " ) ,
       nl -> StringAtom ( " Example Type List " ) ,
       nl -> StringAtom ( " List Rep " ) ,
       nl -> StringAtom ( " Example List " )) ,
       nl -> FourElemList (
       nl -> StringAtom ( " -> SIMPLE " ) ,
       nl -> StringAtom ( BasicType ()) ,
       nl -> StringAtom ( " () " ) ,
       nl -> StringAtom ( " () " )
     ));
   }

   static Word In(const ListExpr typeInfo, const ListExpr instance,
           const int errorPos, ListExpr& errorInfo, bool& correct){
     // we do not allow an explicit creation of this type
     if(nl->IsEmpty(instance) || listutils::isPtrList(instance)){
        correct = true;
        return SetWord(new MPointer((MemoryObject*)0));
     }
     correct = false;
     return SetWord((void*)0);
   }

   static ListExpr Out(ListExpr typeInfo, Word value){
     MPointer* v = (MPointer*) value.addr;
     return listutils::getPtrList((*v)());
   }

   static Word Create(const ListExpr typeInfo){
     return SetWord(new MPointer((MemoryObject*)0));
   }

   static void Delete(const ListExpr t, Word& v){
     MPointer* p = (MPointer*) v.addr;
     delete p;
     v.addr =0;
   }
 
   static bool Open(SmiRecord& vR,
                    size_t& offset, const ListExpr typeInfo,
                    Word& value){
      // read nothing
      value.addr = new MPointer((MemoryObject*)0);
      return true;
   }

   static bool Save(SmiRecord& valueRecord, size_t& offset,
             const ListExpr typeInfo, Word& value){
     // save nothing
     return true;
   }
   
   static void Close(const ListExpr t, Word& v){
       MPointer* p = (MPointer*) v.addr;
       delete p;
       v.addr =0;
   }

   static Word Clone(const ListExpr typeInfo, const Word& w){
     MPointer* p = (MPointer*) w.addr;
     Word res ;
     res.addr = new MPointer(*p);
     return res;
   }

   static void* Cast(void* addr){
     return new (addr) MPointer;
   }

   static bool TypeCheck(ListExpr type, ListExpr& errorInfo){
     return checkType(type);
   }

   static int SizeOf(){
     return 0;
   }


/*
1.3 Member access

*/
   MemoryObject* operator()() {
     return thePointer;
   }

   MemoryObject* GetValue() const{
     return thePointer; 
   }

   void setPointer(MemoryObject* p){
     thePointer = p;
   }


private:
   MemoryObject* thePointer;


};

} // namespace

#endif
