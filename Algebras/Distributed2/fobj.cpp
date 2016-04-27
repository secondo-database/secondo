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
#include <string>
#include <stdint.h>
#include "fobj.h"
#include "NestedList.h"
#include "SecondoSMI.h"
#include "AlgebraTypes.h"
#include "ListUtils.h"

extern NestedList* nl;

using namespace std;

namespace distributed2{

fobj::fobj():value(""), defined(false){}
fobj::fobj(const fobj& src): value(src.value), defined(src.defined){}

fobj::fobj(const int dummy){}

fobj::fobj(const std::string& name):
   value(name), defined(true){}

void fobj::set(const std::string& value){
   this->value = value;
   defined = true;
}

 
// secondo support 
bool fobj::checkType(const ListExpr list){
  if(!nl->HasLength(list,2)){
    return false;
  }
  if(!listutils::isSymbol(nl->First(list),BasicType())){
    return false;
  }
  return true; // TODO: check whether the second elemnt is a valid
               // secondo type
}


ListExpr fobj::Property(){
  return nl->TwoElemList(
     nl->FourElemList(
        nl-> StringAtom("Signature") ,
        nl-> StringAtom("Example Type List") ,
        nl-> StringAtom("List Rep") ,
        nl-> StringAtom("Example List")
     ),
     nl->FourElemList(
       nl->StringAtom("->SIMPLE") ,
       nl->StringAtom("fobj(X)") ,
       nl->StringAtom("<text>" ) ,
       nl->StringAtom(" 'filename' " )
     )
  );
}

Word fobj::In(const ListExpr typeInfo, const ListExpr instance,
              const int errorPos, ListExpr& errorInfo, bool& correct){

  Word res((void*)0);
  if(listutils::isSymbolUndefined(instance)){
    res.addr = new fobj();
    correct = true;
    return res;
  }


  if(nl->AtomType(instance)!=TextType){
    correct = false;
    return res;
  }
  std::string v = nl->Text2String(instance);
  res.addr = new fobj(v);
  correct = true;
  return res;
}


ListExpr fobj::Out(ListExpr typeInfo, Word value){
  fobj* v = (fobj*) value.addr;
  if(!v->IsDefined()){
    return listutils::getUndefined();
  }
  return nl->TextAtom(v->value);
}

Word fobj::Create(const ListExpr typeInfo){
  return Word(new fobj(""));
}

void fobj::Delete(const ListExpr typeInfo, Word& w){
   delete (fobj*) w.addr;
   w.addr = 0;
}

bool fobj::Save(SmiRecord& valueRecord, size_t& offset,
                 const ListExpr typeInfo, Word& value){
 fobj* v = (fobj*) value.addr;
 if(!v->IsDefined()){
   char def = 0;
   valueRecord.Write(&def, sizeof(char), offset);
   offset += sizeof(char);
   return true; 
 }
 char def = 1;
 valueRecord.Write(&def, sizeof(char), offset);
 offset += sizeof(char);

 uint32_t length = v->value.length(); 
 valueRecord.Write(&length, sizeof(uint32_t), offset);
 offset += sizeof(uint32_t);
 if(length>0){
    valueRecord.Write(v->value.c_str(), length, offset);
    offset += length;
 }
 return true;
}

bool fobj::Open(SmiRecord& valueRecord,
                   size_t& offset, const ListExpr typeInfo,
                   Word& value){

  char def;
  valueRecord.Read(&def, sizeof(char), offset);
  offset+=sizeof(char);
  if(def==0){
     value.addr = new fobj();
     return true;
  }

  uint32_t length;
  valueRecord.Read(&length, sizeof(uint32_t), offset);
  offset += sizeof(uint32_t);
  if(length==0){
     value.addr = new fobj("");
     return true;
  } 
  char* name = new char[length];
  valueRecord.Read(name, length, offset);
  offset += length;
  value.addr = new fobj(string(name, length));
  delete[] name;
  return true;
}
void fobj::Close(const ListExpr typeInfo, Word& w){
   delete (fobj*) w.addr;
   w.addr = 0;
}

Word fobj::Clone(const ListExpr typeInfo, const Word& v){
  fobj* src = (fobj*) v.addr;
  Word res(new fobj(*src));
  return res;
}

void* fobj::Cast(void* addr){
  return new (addr) fobj(42);
}

bool fobj::TypeCheck(ListExpr type, ListExpr& errorInfo){
   return checkType(type);
}

int fobj::SizeOf(){
  return sizeof(fobj);
}


} // end of namespace distributed2

