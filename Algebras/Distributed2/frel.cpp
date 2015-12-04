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

#include "frel.h"
#include "NestedList.h"
#include "SecondoSMI.h"
#include "AlgebraTypes.h"
#include "ListUtils.h"
#include "RelationAlgebra.h"
#include "stdint.h"

extern NestedList* nl;

namespace distributed2{

frel::frel():value(""){}
frel::frel(const frel& src): value(src.value){}

frel::frel(const int dummy){}

frel::frel(const std::string& name):
   value(name){}
 
// secondo support 
bool frel::checkType(const ListExpr list){
  if(!nl->HasLength(list,2)){
    return false;
  }
  if(!listutils::isSymbol(nl->First(list),BasicType())){
    return false;
  }
  return Relation::checkType(nl->Second(list));
}


ListExpr frel::Property(){
  return nl->TwoElemList(
     nl->FourElemList(
        nl-> StringAtom("Signature") ,
        nl-> StringAtom("Example Type List") ,
        nl-> StringAtom("List Rep") ,
        nl-> StringAtom("Example List")
     ),
     nl->FourElemList(
       nl->StringAtom("->SIMPLE") ,
       nl->StringAtom("frel(tuple(...))") ,
       nl->StringAtom("<text>" ) ,
       nl->StringAtom(" 'filename' " )
     )
  );
}

Word frel::In(const ListExpr typeInfo, const ListExpr instance,
              const int errorPos, ListExpr& errorInfo, bool& correct){

  Word res((void*)0);
  if(nl->AtomType(instance)!=TextType){
    correct = false;
    return res;
  }
  std::string v = nl->Text2String(instance);
  res.addr = new frel(v);
  correct = true;
  return res;
}


ListExpr frel::Out(ListExpr typeInfo, Word value){
  frel* v = (frel*) value.addr;
  return nl->TextAtom(v->value);
}

Word frel::Create(const ListExpr typeInfo){
  return Word(new frel(""));
}

void frel::Delete(const ListExpr typeInfo, Word& w){
   delete (frel*) w.addr;
   w.addr = 0;
}

bool frel::Save(SmiRecord& valueRecord, size_t& offset,
                 const ListExpr typeInfo, Word& value){
 frel* v = (frel*) value.addr;
 uint32_t length = v->value.length(); 
 valueRecord.Write(&length, sizeof(uint32_t), offset);
 offset += sizeof(uint32_t);
 if(length>0){
    valueRecord.Write(v->value.c_str(), length, offset);
    offset += length;
 }
 return true;
}

bool frel::Open(SmiRecord& valueRecord,
                   size_t& offset, const ListExpr typeInfo,
                   Word& value){

  uint32_t length;
  valueRecord.Read(&length, sizeof(uint32_t), offset);
  offset += sizeof(uint32_t);
  if(length==0){
     value.addr = new frel("");
     return true;
  } 
  char* name = new char[length];
  valueRecord.Read(name, length, offset);
  offset += length;
  value.addr = new frel(string(name, length));
  delete[] name;
  return true;
}
void frel::Close(const ListExpr typeInfo, Word& w){
   delete (frel*) w.addr;
   w.addr = 0;
}

Word frel::Clone(const ListExpr typeInfo, const Word& v){
  frel* src = (frel*) v.addr;
  Word res(new frel(*src));
  return res;
}

void* frel::Cast(void* addr){
  return new (addr) frel(42);
}

bool frel::TypeCheck(ListExpr type, ListExpr& errorInfo){
   return checkType(type);
}

int frel::SizeOf(){
  return sizeof(frel);
}


} // end of namespace distributed2

