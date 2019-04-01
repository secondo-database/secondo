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

#include "MemoryObject.h"
#include <string>

extern NestedList* nl;

namespace mm2algebra{

MemoryObject::~MemoryObject(){}

unsigned long MemoryObject::getMemSize (){
    return memSize;
};

std::string MemoryObject::getObjectTypeExpr(){
    return objectTypeExpr;
}
void MemoryObject::setObjectTypeExpr(std::string _oTE){
    objectTypeExpr = _oTE;
}

std::string MemoryObject::getDatabase(){
    return database;
}

bool MemoryObject::hasflob(){
    return flob;
}


ListExpr MemoryObject::getType() const{
  ListExpr res;
  if(!nl->ReadFromString(objectTypeExpr,res)){
     return nl->TheEmptyList();
  } else {
     return res;
  }
}


ListExpr MemoryObject::out() {
  return nl->SymbolAtom("NO_OUT");
}

MemoryObject* MemoryObject::in(ListExpr type, bool& correct){
   correct = false;
   return 0;
}



} // end of namespace 

