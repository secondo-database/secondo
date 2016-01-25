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
#include "DArray.h"


using namespace std;

namespace distributed2{


/*

1.1 Implementation of  Class ~DArrayElement~

This class represents information about a single worker of a DArray.

*/



DArrayElement::DArrayElement(const string& _server, const int _port,
               const int _num, const string& _config):
   server(_server), port(_port), num(_num),config(_config) {}

DArrayElement::DArrayElement(const DArrayElement& src):
 server(src.server), port(src.port), num(src.num),
 config(src.config) {}

DArrayElement& DArrayElement::operator=(const DArrayElement& src){
   this->server = src.server;
   this->port = src.port;
   this->num = src.num;
   this->config = src.config;
   return *this;
}     

DArrayElement::~DArrayElement() {}

void DArrayElement::set(const string& server, const int port, 
         const int num, const string& config){
   this->server = server;
   this->port = port;
   this->num = num;
   this->config = config;
}


bool DArrayElement::operator==(const DArrayElement& other) const{
  return    (this->port == other.port)
         && (this->num == other.num)
         && (this->server == other.server)
         && (this->config == other.config); 
}

bool DArrayElement::operator<(const DArrayElement& other) const {
   if(this->port < other.port) return true;
   if(this->port > other.port) return false;
   if(this->num < other.num) return true;
   if(this->num > other.num) return false;
   if(this->server < other.server) return true;
   if(this->server > other.server) return false;
   if(this->config < other.config) return true;
   // equal or greater
   return false;
}

bool DArrayElement::operator>(const DArrayElement& other) const {
   if(this->port > other.port) return true;
   if(this->port < other.port) return false;
   if(this->num > other.num) return true;
   if(this->num < other.num) return false;
   if(this->server > other.server) return true;
   if(this->server < other.server) return false;
   if(this->config > other.config) return true;
   // equal or greater
   return false;
}

ListExpr DArrayElement::toListExpr(){
   return nl->ThreeElemList(
              nl->TextAtom(server),
              nl->IntAtom(port),
              nl->TextAtom(config));   
}

bool DArrayElement::readFrom(SmiRecord& valueRecord, size_t& offset){
   string s;
   if(!readVar(s,valueRecord,offset)){
       return false;
   }
   uint32_t p;
   if(!readVar(p,valueRecord,offset)){
      return false;
   }
   string c;
   if(!readVar(c,valueRecord,offset)){
      return false;
   }

   server = s;
   port = p;
   num = -1;
   config = c;
   return true;
}

bool DArrayElement::saveTo(SmiRecord& valueRecord, size_t& offset){
   if(!writeVar(server,valueRecord,offset)){
      return false;
   }
   if(!writeVar(port,valueRecord,offset)){
       return false;
   }
   if(!writeVar(config,valueRecord,offset)){
      return false;
   }
   return true;
}

void DArrayElement::print(ostream& out)const{
    out << "( S: " << server << ", P : " << port 
        << "Num : " << num
        << ", C : " << config << ")";
}


bool InDArrayElement(ListExpr list, DArrayElement& result){
   if(!nl->HasLength(list,3)){
     return false;
   }
   ListExpr e1 = nl->First(list);
   ListExpr e2 = nl->Second(list);
   ListExpr e4 = nl->Third(list);
   string server;
   int port; 
   int num;
   string config;

   if(nl->AtomType(e1) == StringType){
      server = nl->StringValue(e1);     
   } else if(nl->AtomType(e1) == TextType){
      server = nl->Text2String(e1);
   } else {
      return false;
   }
   stringutils::trim(server);
   if(server.empty()){
     return false;
   }
   if(nl->AtomType(e2) != IntType){
     return false;
   }
   port = nl->IntValue(e2);
   if(port <=0){
     return false;
   }

   if(nl->AtomType(e4) == StringType){
      config = nl->StringValue(e4);
   } else if(nl->AtomType(e4) == TextType){
      config = nl->Text2String(e4);
   } else {
      return false;
   }
   stringutils::trim(config);
   if(config.empty()){
      return false;
   }
   num = -1;
   result.set(server,port,num,config);
   return true;
}


/*
3.2 Implementation of DArrayT

*/
template<arrayType Type>
DArrayT<Type>::DArrayT(const std::vector<uint32_t>& _map, 
                       const std::string& _name): 
      worker(),map(_map), size(map.size()),name(_name) {
   if(!stringutils::isIdent(name) || map.size() ==0 ){ // invalid
      name = "";
      defined = false;
      map.clear();
      size = 0;
      return;
   }
   defined = true;
}

template<arrayType Type>
DArrayT<Type>::DArrayT(const size_t _size , const std::string& _name): 
      worker(),map(), size(_size),name(_name) {
   assert(Type == DFMATRIX);
   if(!stringutils::isIdent(name) || size ==0 ){ // invalid
      name = "";
      defined = false;
      map.clear();
      size = 0;
      return;
   }
   defined = true;
}

template<arrayType Type>
DArrayT<Type>::DArrayT(const std::vector<uint32_t>& _map, 
                       const std::string& _name, 
          const std::vector<DArrayElement>& _worker): 
    worker(_worker),map(_map), size(map.size()),name(_name) {

   if(!stringutils::isIdent(name) || map.size() ==0 || !checkMap()){ 
      // invalid
      name = "";
      defined = false;
      map.clear();
      size = 0;
      return;
   }
   defined = true;
}

template<arrayType Type>
DArrayT<Type>::DArrayT(const size_t _size, const std::string& _name, 
          const std::vector<DArrayElement>& _worker): 
    worker(_worker),map(), size(_size),name(_name) {

   assert(Type == DFMATRIX);
   if(!stringutils::isIdent(name) || size ==0 ){ 
      // invalid
      name = "";
      defined = false;
      map.clear();
      size = 0;
      return;
   }
   defined = true;
}





template<arrayType Type>
DArrayT<Type>::DArrayT(const DArrayT<Type>& src): 
        worker(src.worker), map(src.map), name(src.name), 
        defined(src.defined)
  {
  }

template<arrayType Type>
template<arrayType T>
DArrayT<Type>& DArrayT<Type>::operator=(const DArrayT<T>& src) {
   this->worker = src.worker;
   this->map = src.map;
   this->size = src.size;
   this->name = src.name;
   this->defined = src.defined;
   return *this;
}     

template<arrayType Type>
DArrayT<Type>::~DArrayT() {
}


template<arrayType Type>
uint32_t DArrayT<Type>::getWorkerNum(uint32_t index){
  assert(Type!=DFMATRIX);
  return map[index];
}


template<arrayType Type>
arrayType DArrayT<Type>::getType() const{
  return Type;
}


template<arrayType Type>
void DArrayT<Type>::setSize(size_t newSize){
  assert(newSize > 0);
  assert(Type == DFMATRIX);
  this->size = newSize;
}


template<arrayType Type>
void DArrayT<Type>::set(const size_t size, const std::string& name, 
         const std::vector<DArrayElement>& worker){
   if(!stringutils::isIdent(name) || size ==0){ // invalid
      this->name = "";
      this->defined = false;
      return;
   }
   defined = true;
   this->name = name;
   this->size = size;
   if(Type!=DFMATRIX){
        this->map = createStdMap(size,worker.size());
   } else {
        map.clear();
   }
   this->worker = worker;
}

template<arrayType Type>
template<class AT>
bool DArrayT<Type>::equalMapping(AT& a, bool ignoreSize ){
   if(Type==DFMATRIX){ // mapping does not exist in DFMATRIX
      return true;
   }
   if(!ignoreSize && (map.size()!=a.map.size())){
      return false;
   }
   size_t minV = std::min(map.size(), a.map.size());
   for(size_t i=0;i<minV;i++){
      if(map[i]!=a.map[i]){
         return false;
      }
   }
   return true;
}



template<arrayType Type>
void DArrayT<Type>::set(const std::vector<uint32_t>& m, 
         const std::string& name, 
         const std::vector<DArrayElement>& worker){
   if(!stringutils::isIdent(name) || m.size() ==0){ // invalid
      makeUndefined(); 
      return;
   }
   defined = true;
   this->name = name;
   this->map = m;
   this->size = m.size();
   this->worker = worker;
   if(Type==DFMATRIX){
      map.clear();
   }
   if(!checkMap()){
        makeUndefined();
   }
}


template<arrayType Type>
bool DArrayT<Type>::IsDefined(){
   return defined;
}

template<arrayType Type>
const std::string DArrayT<Type>::BasicType() { 
  switch(Type){
     case DARRAY : return "darray";
     case DFARRAY : return "dfarray";
     case DFMATRIX : return "dfmatrix";
  }
  assert(false);
  return "typeerror";
}

template<arrayType Type>
const bool DArrayT<Type>::checkType(const ListExpr list){
    if(!nl->HasLength(list,2)){
       return false;
    }  
    if(!listutils::isSymbol(nl->First(list), BasicType())){
        return false;
    }
    // for dfarrays and dmatrices, only relations 
    // are allowed as subtype
    if((Type == DFARRAY) || (Type==DFMATRIX)){
      return Relation::checkType(nl->Second(list));
    }

    // for a darray, each type is allowed
    SecondoCatalog* ctl = SecondoSystem::GetCatalog();
    std::string name;
    int algid, type;
    if(!ctl->LookUpTypeExpr(nl->Second(list), name, algid, type)){
       return false;
    }
    AlgebraManager* am = SecondoSystem::GetAlgebraManager();
    ListExpr errorInfo = listutils::emptyErrorInfo();
    if(!am->TypeCheck(algid,type,nl->Second(list),errorInfo)){
       return false;
    }
    return true;
}

template<arrayType Type>
size_t DArrayT<Type>::numOfWorkers() const{
  return worker.size();
}
template<arrayType Type>
size_t DArrayT<Type>::getSize() const{
   return size;
}

template<arrayType Type>
void DArrayT<Type>::setStdMap(size_t size){
    if(Type!=DFMATRIX){
       map = createStdMap(size, worker.size());
       this->size = size;
    } else {
       this->size = size;
       map.clear();
    }
}


template<arrayType Type>
DArrayElement DArrayT<Type>::getWorkerForSlot(int i){
   assert(Type!=DFMATRIX);
   if(i<0 || i>= map.size()){
      assert(false);
   }
   return getWorker(map[i]);
}

template<arrayType Type>
size_t DArrayT<Type>::getWorkerIndexForSlot(int i){
   assert(Type!=DFMATRIX);
   if(i<0 || i>= map.size()){
      assert(false);
   }
   return map[i];
}


template<arrayType Type>
DArrayElement DArrayT<Type>::getWorker(int i){
   if(i< 0 || i >= (int) worker.size()){
       assert(false);
      // throw "Invalid worker number";
   }
   return worker[i];
}

template<arrayType Type>
void DArrayT<Type>::setResponsible(size_t slot, size_t _worker){

  assert(size == map.size());
  assert(slot < size);
  assert(_worker < worker.size());
  assert(Type!=DFMATRIX);
  map[slot] = _worker;
}


template<arrayType Type>
ListExpr DArrayT<Type>::toListExpr(){
  if(!defined){
    return listutils::getUndefined();
  }

  ListExpr wl;
  if(worker.empty()){
    wl =  nl->TheEmptyList();
  } else {
      wl = nl->OneElemList(
                   worker[0].toListExpr());
      ListExpr last = wl;
      for(size_t i=1;i<worker.size();i++){
        last = nl->Append(last, worker[i].toListExpr());
      }
  }
  if(isStdMap() || (Type==DFMATRIX)){ 
     return nl->ThreeElemList(nl->SymbolAtom(name), 
                              nl->IntAtom(size), 
                              wl); 
  } else if(map.empty()) {
     return nl->ThreeElemList(nl->SymbolAtom(name), 
                              nl->TheEmptyList(), 
                              wl);
  } else {
     ListExpr lmap = nl->OneElemList(nl->IntAtom(map[0]));
     ListExpr last = lmap;
     for(size_t i=1;i<map.size();i++){
        last = nl->Append(last, nl->IntAtom(map[i]));
     }
     return nl->ThreeElemList(nl->SymbolAtom(name), lmap, wl);
  }
}


template<arrayType Type>
DArrayT<Type>* DArrayT<Type>::readFrom(ListExpr list){
   if(listutils::isSymbolUndefined(list)){
      std::vector<uint32_t> m;
      return new DArrayT<Type>(m,"");
   }
   if(!nl->HasLength(list,3)){
      return 0;
   }
   ListExpr Name = nl->First(list);
   ListExpr Workers = nl->Third(list);
   if(   (nl->AtomType(Name) != SymbolType)
       ||(nl->AtomType(Workers)!=NoAtom)){
      return 0;
   }
   std::string name = nl->SymbolValue(Name);
   if(!stringutils::isIdent(name)){
      return 0;
   }
   std::vector<DArrayElement> v;
   int wn = 0;
   while(!nl->IsEmpty(Workers)){
      DArrayElement elem("",0,0,"");
      if(!InDArrayElement(nl->First(Workers), elem)){
         return 0;
      }
      elem.setNum(wn);
      wn++;
      v.push_back(elem);
      Workers = nl->Rest(Workers);
   }
   std::vector<uint32_t> m;
   if(nl->AtomType(nl->Second(list)==IntType)){
      int size = nl->IntValue(nl->Second(list));
      if(size <=0){
         return 0;
      }
      m = createStdMap(size,v.size());
   } else if(nl->AtomType(nl->Second(list))!=NoAtom){
      return 0;
   } else {
      ListExpr lm = nl->Second(list);
      while(!nl->IsEmpty(lm)){
          ListExpr first = nl->First(lm);
          lm = nl->Rest(lm);
          if(nl->AtomType(first)!=IntType){
             return 0;
          }
          int mv = nl->IntValue(first);
          if(mv<0 || mv>=v.size()){
             return 0;
          }
          m.push_back(mv);
      }
   }
   DArrayT<Type>* result = new DArrayT<Type>(m,name);
   swap(result->worker,v);
   result->defined = true;
   return result;
}

template<arrayType Type>
bool DArrayT<Type>::open(SmiRecord& valueRecord, size_t& offset, 
                 const ListExpr typeInfo, Word& result){
   bool defined;
   result.addr = 0;
   if(!readVar<bool>(defined,valueRecord,offset)){
      return false;
   } 
   if(!defined){
     std::vector<uint32_t> m;
     result.addr = new DArrayT<Type>(m,"");
     return true;
   }
   // array in smirecord is defined, read size
   size_t size;
   if(!readVar<size_t>(size,valueRecord,offset)){
      return false;
   }
   // read name
   std::string name;
   if(!readVar<std::string>(name,valueRecord, offset)){
       return false;
   }

   // read  map
   uint32_t me;
   std::vector<uint32_t> m;
   DArrayT<Type>* res = 0; 

   if(Type!=DFMATRIX){
      for(size_t i=0;i<size;i++){
          if(!readVar<uint32_t>(me,valueRecord,offset)){
             return false;
          }
          m.push_back(me);
      }
      res = new DArrayT<Type>(m,name);
   } else {
      res = new DArrayT<Type>(size,name);
   }
   int wn = 0;
   // append workers
   size_t numWorkers;
   if(!readVar<size_t>(numWorkers,valueRecord, offset)){
     return false;
   }
   for(size_t i=0; i< numWorkers; i++){
      DArrayElement elem("",0,0,"");
      if(!elem.readFrom(valueRecord, offset)){
          delete res;
          return false;
      }
      elem.setNum(wn);
      wn++;
      res->worker.push_back(elem);
   }

   result.addr = res;
   return true;
}

template<arrayType Type>
bool DArrayT<Type>::save(SmiRecord& valueRecord, size_t& offset,
                 const ListExpr typeInfo, Word& value) {

    DArrayT<Type>* a = (DArrayT<Type>*) value.addr;
    // defined flag
    if(!writeVar(a->defined,valueRecord,offset)){
      return false;
    }
    if(!a->defined){
       return true;
    }
    // size
    size_t s = a->size;
    if(!writeVar(s,valueRecord,offset)){
      return false;
    }
    // name
    if(!writeVar(a->name, valueRecord, offset)){
      return false;
    }
    if(Type!=DFMATRIX){
      // map
      for(size_t i=0;i<a->map.size();i++){
          if(!writeVar(a->map[i], valueRecord,offset)){
             return false;
          }
      }
    }
    // workers
    if(!writeVar(a->worker.size(), valueRecord, offset)){
      return false;
    }
    for(size_t i=0;i<a->worker.size();i++){
         if(!a->worker[i].saveTo(valueRecord,offset)){
            return false;
         }
    }
    return true; 
}


template<arrayType Type>
std::vector<uint32_t> DArrayT<Type>::createStdMap(const uint32_t size, 
                                     const int numWorkers){
   std::vector<uint32_t> map;
   if(Type!=DFMATRIX){
      for(uint32_t i=0;i<size;i++){
         map.push_back(i%numWorkers);
      }
   } 
   return map;
}


template<arrayType Type>
void DArrayT<Type>::print(std::ostream& out){
  if(!defined){
     out << "undefined";
     return;
  }

  out << "Name : " << name <<", size : " << map.size()
      << " workers : [" ;
  for(size_t i =0;i<worker.size();i++){
     if(i>0) out << ", ";
     worker[i].print(out);
  }
  out << "]";
  if(Type != DFMATRIX){
    out << "map = [";
    for(uint32_t i=0;i<map.size();i++){
      if(i>0){
        out << ", ";
      }
      out << i << " -> " << map[i];
    }
    out << "]";
  }
}

template<arrayType Type>
void DArrayT<Type>::makeUndefined(){
   worker.clear();
   map.clear();
   size = 0;
   name = "";
   defined = false;
}

template<arrayType Type>
std::string DArrayT<Type>::getName() const{
   return name;
}

template<arrayType Type>
 bool DArrayT<Type>::setName( const std::string& n){
   if(!stringutils::isIdent(n)){
      return false;
   }
   name = n;
   return true;
 }

template<arrayType Type>
 template<class TE>
 bool DArrayT<Type>::equalWorkers(const TE& a) const{
    return equalWorker(a.worker);
 }




template<arrayType Type>
   bool DArrayT<Type>::checkMap(){
     if(Type!=DFMATRIX){
        for(int i=0;i<map.size();i++){
           if(map[i] >= worker.size()){
              return false;
           }
        }
        return true;
     }
     return map.empty();
   }

template<arrayType Type>
bool DArrayT<Type>::isStdMap(){
     if(Type!=DFMATRIX){
        int s = worker.size();
        for(size_t i=0;i<map.size();i++){
             if(map[i]!= i%s){
                 return false;
             }
        }
        return true;
     }
     return map.empty();
   }



template<arrayType Type>
bool DArrayT<Type>::equalWorker(const std::vector<DArrayElement>& w) const{
      if(worker.size() != w.size()){
          return false;
      }
      for(size_t i=0;i<worker.size();i++){
           if(worker[i] != w[i]){
              return false;
           }
      }
      return true;
   }


ostream& operator<<(ostream& out, const DArrayElement& elem){
  elem.print(out);
  return out;
}

// the classes
template class DArrayT<DARRAY>;
template class DArrayT<DFARRAY>;
template class DArrayT<DFMATRIX>;

// all combinations of assignment
template DArray& DArray::operator=(const DArray&);
template DArray& DArray::operator=(const DFArray&);
template DArray& DArray::operator=(const DFMatrix&);
template DFArray& DFArray::operator=(const DArray&);
template DFArray& DFArray::operator=(const DFArray&);
template DFArray& DFArray::operator=(const DFMatrix&);
template DFMatrix& DFMatrix::operator=(const DArray&);
template DFMatrix& DFMatrix::operator=(const DFArray&);
template DFMatrix& DFMatrix::operator=(const DFMatrix&);

// all combinations of equalMapping
template bool DArray::equalMapping(DArray&, bool);
template bool DArray::equalMapping(DFArray&, bool);
template bool DArray::equalMapping(DFMatrix&, bool);
template bool DFArray::equalMapping(DArray&, bool);
template bool DFArray::equalMapping(DFArray&, bool);
template bool DFArray::equalMapping(DFMatrix&, bool);
template bool DFMatrix::equalMapping(DArray&, bool);
template bool DFMatrix::equalMapping(DFArray&, bool);
template bool DFMatrix::equalMapping(DFMatrix&, bool);

// the same game for equalWorker
template bool DArray::equalWorkers(const DArray&) const;
template bool DArray::equalWorkers(const DFArray&) const;
template bool DArray::equalWorkers(const DFMatrix&) const;
template bool DFArray::equalWorkers(const DArray&) const;
template bool DFArray::equalWorkers(const DFArray&) const;
template bool DFArray::equalWorkers(const DFMatrix&) const;
template bool DFMatrix::equalWorkers(const DArray&) const;
template bool DFMatrix::equalWorkers(const DFArray&) const;
template bool DFMatrix::equalWorkers(const DFMatrix&) const;




} // end of namespace distributed2




