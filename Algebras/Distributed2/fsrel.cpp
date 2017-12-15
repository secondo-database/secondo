/*
----
This file is part of SECONDO.

Copyright (C) 2015,
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


//[$][\$]

*/


#include "Dist2Helper.h"
#include <string>
#include <vector>
#include "NestedList.h"
#include "SecondoSMI.h"
#include "ListUtils.h"
#include "fsrel.h"
#include "Algebras/Relation-C++/RelationAlgebra.h"


extern NestedList* nl;

namespace distributed2{


fsrel::fsrel(): values(), defined(true){}

fsrel::fsrel(const fsrel& src): values(src.values), defined(src.defined) {}


fsrel::fsrel(const std::vector<std::string>& names):
    values(names), defined(true){};

void fsrel::set(const std::vector<std::string>& values){
  this->values = values;
}
 
  // secondo support 
bool fsrel::checkType(const ListExpr list){
  if(!nl->HasLength(list,2)){
     return false;
  }
  if(!listutils::isSymbol(nl->First(list), BasicType())){
    return false;
  }
  return Tuple::checkType(nl->Second(list));
}

ListExpr fsrel::Property(){
  return nl->TwoElemList(
     nl->FourElemList(
        nl-> StringAtom("Signature") ,
        nl-> StringAtom("Example Type List") ,
        nl-> StringAtom("List Rep") ,
        nl-> StringAtom("Example List")
     ),  
     nl->FourElemList(
       nl->StringAtom("->i SIMPLE") ,
       nl->StringAtom("fsrel(tuple(...))") ,
       nl->StringAtom("(<text> <text> <text>" ) , 
       nl->StringAtom(" 'filename1' 'filename2' " )
     )   
  );  
}


Word fsrel::In(const ListExpr __attribute__((unused))  typeInfo, 
               const ListExpr instance,
               const int __attribute__((unused)) errorPos, 
               ListExpr __attribute__((unused)) & errorInfo, bool& correct) {

  Word res((void*)0);
  if(listutils::isSymbolUndefined(instance)){
    res.addr = new fsrel();
    correct = true;
    return res;
  }
  correct = false;
  if(nl->AtomType(instance) != NoAtom){
    return res;
  }
  std::vector<std::string>  content;
  ListExpr rest = instance;
  while(!nl->IsEmpty(rest)){
     ListExpr name = nl->First(rest);
     rest = nl->Rest(rest);
     if(nl->AtomType(name)!=TextType){
        return res; 
     }
     content.push_back(nl->Text2String(name));
  }
  res.addr = new fsrel(content);
  correct = true;
  return res;
}

ListExpr fsrel::Out(ListExpr __attribute__((unused)) typeInfo, 
                    Word value){
   fsrel* r = (fsrel*) value.addr;
   if(!r->IsDefined()){
      return listutils::getUndefined();
   }
   if(r->values.empty()){
      return  nl->TheEmptyList();
   }
   ListExpr res = nl->OneElemList( nl->TextAtom(r->values[0]));
   ListExpr last = res;
   for(size_t i=1;i<r->values.size(); i++){
     last = nl->Append(last, nl->TextAtom(r->values[i]));
   }
   return res;
}


Word fsrel::Create(const ListExpr __attribute__((unused)) typeInfo) {
   return SetWord(new fsrel(false));
}

void fsrel::Delete(const ListExpr __attribute__((unused)) typeInfo, Word& w){
    delete (fsrel*) w.addr;
    w.addr = 0;
}


bool fsrel::Save(SmiRecord& valueRecord, size_t& offset,
          const ListExpr __attribute__((unused)) typeInfo, 
          Word& value){
  fsrel* r = (fsrel*) value.addr;
  char def = r->IsDefined()?1:0;
  bool ok = true;
  ok = writeVar(def,valueRecord,offset);
  if(!r->IsDefined()){
    return ok;
  }
  size_t size = r->size();
  ok = ok && writeVar(size, valueRecord, offset);
  for(size_t i=0;i<r->size();i++){
    ok = ok && writeVar((*r)[i], valueRecord,offset);
  }
  return ok;
}


bool fsrel::Open(SmiRecord& valueRecord,
                 size_t& offset, 
                 const ListExpr __attribute__((unused)) typeInfo,
          Word& value){
   value.addr = 0;
   char def;
   bool ok = true;
   ok = ok && readVar(def,valueRecord, offset);
   if(!ok){
      return false;
   }
   if(!def){
      value.addr = new fsrel();
      return true;
   }
   size_t size;
   readVar(size,valueRecord, offset);
   std::vector<std::string> content;
   std::string v;
   for(size_t i=0;i<size;i++){
      ok = ok && readVar(v, valueRecord,offset);
      if(!ok){
         return false;
      }
      content.push_back(v);
   }
   value.addr = new fsrel(content);
   return true;
}


void fsrel::Close(const ListExpr __attribute__((unused)) typeInfo, Word& w){
    delete (fsrel*) w.addr;
    w.addr = 0;
}
Word fsrel::Clone(const ListExpr __attribute__((unused)) typeInfo, 
                  const Word& v){
  Word res;
  res.addr = new fsrel( *((fsrel*) v.addr));
  return res;
}

void* fsrel::Cast(void* addr) {
   return new (addr) fsrel(0);
}

bool fsrel::TypeCheck(ListExpr __attribute__((unused)) type, 
                      ListExpr __attribute__((unused)) & errorInfo){
  return checkType(type);
}

int fsrel::SizeOf(){
   return 512;  
}
  


} // end of namespace distributed2

