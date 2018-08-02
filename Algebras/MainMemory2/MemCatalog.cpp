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
#include "MPointer.h"

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
    if(obj){
       usedMemSize += obj->getMemSize();
       obj->incReferences();
    }
    return true;
}

int MemCatalog::renameObject(const std::string& oldName, 
                              const std::string& newName,
                             std::string& errorMessage){

  if(!isMMOnlyObject(oldName)){
      std::stringstream ss;
      ss << oldName << " not part of the memory catalog" 
                << std::endl;
      errorMessage = ss.str();
      return ERR_IDENT_UNKNOWN_OBJ;
  }
  if(isObject(newName)){
     std::stringstream ss;
     ss << newName << " already used as an memory object name" 
               << std::endl;
     return ERR_IDENT_USED;
  }
  std::map<std::string,MemoryObject*>::iterator it = memContents.find(oldName);
  assert(it!=memContents.end());
  MemoryObject* mo = it->second;
  memContents.erase(it);
  memContents[newName] = mo;
  return 0; 
  



}




bool MemCatalog::insert (const std::string& name, MPointer* obj){
    MemoryObject* mo = obj?obj->GetValue():0;
    return insert(name,mo);
}

bool MemCatalog::modify(const std::string& name, MemoryObject* obj){
    MemoryObject* old = getMMObject(name);
    if(old!=0){
        if(old==obj) return true; // nothing to do
        deleteObject(name);
    } else {
     return false;
    }
    return insert(name,obj);
}




bool MemCatalog::deleteObject (const std::string& name){
   if(isMMOnlyObject(name)){
      MemoryObject* mo = getMMObject(name);
      if(mo){
        usedMemSize -= getMMObject(name)->getMemSize();
        mo->deleteIfAllowed();;
      }
      memContents.erase(name);
      return true;
    }
    return false;
}


void MemCatalog::clear (){
    std::map<std::string,MemoryObject*>::iterator it;
    it = memContents.begin();
    for(it = memContents.begin();it!=memContents.end(); it++){
       MemoryObject* mo = it->second;
       if(mo){
         mo->deleteIfAllowed();
       }
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
   return ctlg->IsObjectName(objectName, false);
}


bool MemCatalog::clone(const std::string& name, 
                       MemoryObject* value){
     deleteObject(name);
     return insert(name, value->clone());
}


size_t MemCatalog::getNoReferences(std::string name) const{
       MemoryObject* mo = getMMObject(name);
       return mo?mo->getNoReferences():0;
}


/*
Function returns a pointer to the memory object
*Precondition*: "isMMObject( objectName ) == true"

*/

MemoryObject* MemCatalog::getMMObject(const std::string& objectName)const{
    std::map<std::string,MemoryObject*>::const_iterator it;
    it = memContents.find(objectName);
    if(it==memContents.end()){
      return 0;
    }
    return it->second;
}

ListExpr MemCatalog::getMMObjectTypeExpr(const std::string& oN){
    if (!isMMOnlyObject(oN)){
        return nl->TheEmptyList();
    }
    std::string typeExprString="";
    ListExpr typeExprList=0;
    MemoryObject* object = getMMObject(oN);
    if(!object){
       return nl->SymbolAtom("null");
    }
    typeExprString = object->getObjectTypeExpr();
    if (nl->ReadFromString(typeExprString, typeExprList)){
        //assert(Mem::checkType(typeExprList)); 
        return typeExprList;
    }
    return nl->TheEmptyList();
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

bool MemCatalog::isReserved(const std::string& name)const{
    std::map<std::string,MemoryObject*>::const_iterator it;
    it = memContents.find(name);
    return it!= memContents.end(); 
}

bool MemCatalog::isNull(const std::string& name)const{
    std::map<std::string,MemoryObject*>::const_iterator it;
    it = memContents.find(name);
    if(it!= memContents.end()){
       return it->second == 0; 
    } 
    return false;
}


}  // end of namespace

