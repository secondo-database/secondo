/*

----
This file is part of SECONDO.

Copyright (C) 2018, 
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

#include "MemCatalog.h"
#include "MemoryObject.h"
#include "NestedList.h"
#include "ListUtils.h"
#include "SecondoSystem.h"

namespace mm2algebra {

MemCatalog* MemCatalog::instance = 0;

MemCatalog* MemCatalog::getInstance(){
   if(!instance){
      instance = new MemCatalog();
   }
   return instance;
}

void MemCatalog::destroyInstance(){
  if(instance){
     delete instance;
     instance = 0;
  }
}

MemCatalog::~MemCatalog(){
    clear();
};

void MemCatalog::setMemSizeTotal(size_t size) {
            memSizeTotal = size;
}

size_t MemCatalog::getMemSizeTotal(){
            return memSizeTotal;
}

unsigned long MemCatalog::getUsedMemSize() {
            return usedMemSize;
}

unsigned long MemCatalog::getAvailableMemSize() {

    return (memSizeTotal*1024*1024)-(usedMemSize);
}

//only used in memgetcatalog
std::map<std::string,MemoryObject*>* MemCatalog::getMemContent(){

            return &memContents;
}


bool MemCatalog::insert (const std::string& name, MemoryObject* obj){
     if (isObject(name)){
        cout<<"identifier already in use"<<endl;
        return false;
    }

    memContents[name] = obj;
    usedMemSize += obj->getMemSize();

    return true;
}

bool MemCatalog::deleteObject (const std::string& name, 
                               const bool erase/*=true*/){
     if(isMMOnlyObject(name)){
        usedMemSize -= getMMObject(name)->getMemSize();
        delete getMMObject(name);
        if(erase){
           memContents.erase(name);
        }
        return true;
     }
    return false;
}


void MemCatalog::clear (){
    std::map<std::string,MemoryObject*>::iterator it;
    it = memContents.begin();
    while(it!=memContents.end()){
        deleteObject(it->first, false);
        it++;
    }
    memContents.clear();
}

bool MemCatalog::isMMOnlyObject(const std::string& objectName){
    if (memContents.find(objectName)==memContents.end()){
      return false;
    }
    return true;
}

bool MemCatalog::isObject(const std::string& objectName){
   if(isMMOnlyObject(objectName)) return true;
   SecondoCatalog* ctlg = SecondoSystem::GetCatalog();
   return ctlg->IsObjectName(objectName);
}


/*
Function returns a pointer to the memory object
*Precondition*: "isMMObject( objectName ) == true"

*/

MemoryObject* MemCatalog::getMMObject(const std::string& objectName){

    std::map<std::string,MemoryObject*>::iterator it;
    it = memContents.find(objectName);
    if(it==memContents.end()){
      return 0;
    }
    return it->second;
}

ListExpr MemCatalog::getMMObjectTypeExpr(const std::string& oN){
    if (!isMMOnlyObject(oN)){
        return listutils::typeError("not a MainMemory member");
    }
    std::string typeExprString="";
    ListExpr typeExprList=0;
    MemoryObject* object = getMMObject(oN);
    typeExprString = object->getObjectTypeExpr();
    if (nl->ReadFromString(typeExprString, typeExprList)){
        return typeExprList;
    }
    return listutils::typeError();
}

bool MemCatalog::isAccessible(const std::string& name) {
    MemoryObject* obj = getMMObject(name);
    if(!obj){
      return false;
    }  
    if(obj->hasflob()){
      return true;
    }
    SecondoSystem* sys = SecondoSystem::GetInstance();
    if(obj->getDatabase() == sys->GetDatabaseName()){
      return true;
    }
    return false;
}

}  // end of namespace

