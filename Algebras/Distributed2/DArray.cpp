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
#include <boost/thread/lock_guard.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/recursive_mutex.hpp>

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

ListExpr DArrayElement::toListExpr() const{
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


std::string getName(const arrayType a){
  switch(a){
    case DARRAY: return "darray";
    case DFARRAY: return "dfarray";
    case DFMATRIXXX: return "dfmatrix";
  }
  assert(false);
  return "unknown";
}


/*
3.1 Implementation of DistTypeBase

3.1.1 Constructors 

*/

   DistTypeBase::DistTypeBase(const std::vector<DArrayElement>& _worker,
              const string& _name): 
      worker(_worker), name(_name), defined(true){}
               
   DistTypeBase::DistTypeBase( const string& _name):
      worker(), name(_name), defined(true){}
   
   DistTypeBase::DistTypeBase( const DistTypeBase& src): worker(src.worker),
            name(src.name), defined(src.defined) {}

   DistTypeBase& DistTypeBase::operator=(const DistTypeBase& src){
      worker = src.worker;
      name = src.name;
      defined = src.defined;
      return *this;
   }


void DistTypeBase::set(const std::string& name, 
         const std::vector<DArrayElement>& worker){
   if(!stringutils::isIdent(name)){ // invalid
      this->name = "";
      this->defined = false;
      return;
   }
   defined = true;
   this->name = name;
   this->worker = worker;
}


 bool DistTypeBase::equalWorkers(const DistTypeBase& other) const{
    return equalWorkers(other.worker);
 }

 bool DistTypeBase::equalWorkers(const std::vector<DArrayElement>& w) const{
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

size_t DistTypeBase::numOfWorkers() const{
  return worker.size();
}

DArrayElement DistTypeBase::getWorker(int i){
   if(i< 0 || i >= (int) worker.size()){
       assert(false);
      // throw "Invalid worker number";
   }
   return worker[i];
}

std::string DistTypeBase::getName() const{
   return name;
}


void DistTypeBase::makeUndefined(){
   worker.clear();
   name = "";
   defined = false;
}

bool DistTypeBase::setName( const std::string& n){
   if(!stringutils::isIdent(n)){
      return false;
   }
   name = n;
   return true;
 }

/*
3.2 Implementation of DArrayT

*/
DArrayBase::DArrayBase(const std::vector<uint32_t>& _map, 
                       const std::string& _name): 
      DistTypeBase(_name),
      map(_map){
   if(!stringutils::isIdent(name) || map.size() ==0 ){ // invalid
      makeUndefined();
      return;
   }
   defined = true;
}


size_t DArrayBase::getSize() const{
  boost::lock_guard<boost::recursive_mutex> guard(mapmtx);
  return map.size();
}


DArrayBase::DArrayBase(const std::vector<uint32_t>& _map, 
                       const std::string& _name, 
          const std::vector<DArrayElement>& _worker):
    DistTypeBase(_worker, _name), map(_map){

   if(!stringutils::isIdent(name) || map.size() ==0 || !checkMap()){ 
      // invalid
      name = "";
      defined = false;
      return;
   }
   defined = true;
}






DArrayBase::DArrayBase(const DArrayBase& src):
        DistTypeBase(src), map(src.map) {}
 

DArrayBase& DArrayBase::operator=(const DArrayBase& src) {
   boost::lock_guard<boost::recursive_mutex> guard(mapmtx);
   DistTypeBase::operator=(src);
   this->map = src.map;
   return *this;
}     

void DArrayBase::set(const std::string& name, 
                    const std::vector<DArrayElement>& worker){
   boost::lock_guard<boost::recursive_mutex> guard(mapmtx);
   DistTypeBase::set(name, worker);
   this->map = createStdMap(worker.size(),worker.size());
}
void DArrayBase::set(const size_t size, const std::string& name, 
         const std::vector<DArrayElement>& worker){
   boost::lock_guard<boost::recursive_mutex> guard(mapmtx);
   DistTypeBase::set(name, worker);
   this->map = createStdMap(size,worker.size());
}

bool DArrayBase::equalMapping(DArrayBase& a, bool ignoreSize )const{
   if(getType()==DFMATRIXXX){
       return false;
   }
   boost::lock_guard<boost::recursive_mutex> guard(mapmtx);
   const vector<uint32_t> m = a.getMap();

   if(!ignoreSize && (map.size()!=m.size())){
      return false;
   }
   size_t minV = std::min(map.size(), m.size());
   for(size_t i=0;i<minV;i++){
      if(map[i]!=m[i]){
         return false;
      }
   }
   return true;
}



void DArrayBase::set(const std::vector<uint32_t>& m, 
         const std::string& name, 
         const std::vector<DArrayElement>& worker){
   if(!stringutils::isIdent(name) || m.size() ==0){ // invalid
      makeUndefined(); 
      return;
   }
   boost::lock_guard<boost::recursive_mutex> guard(mapmtx);
   defined = true;
   this->name = name;
   this->map = m;
   this->worker = worker;
   if(!checkMap()){
        makeUndefined();
   }
}



void DArrayBase::setStdMap(size_t size){
    boost::lock_guard<boost::recursive_mutex> guard(mapmtx);
    map = createStdMap(size, worker.size());
}


DArrayElement DArrayBase::getWorkerForSlot(int i){
   if(i<0 || i>= (int) map.size()){
      cerr << "access for worker " << i << endl;
      cerr << "number of workers is " << map.size() << endl;
      assert(false);
   }
   boost::lock_guard<boost::recursive_mutex> guard(mapmtx);
   return getWorker(map[i]);
}

size_t DArrayBase::getWorkerIndexForSlot(int i){
  boost::lock_guard<boost::recursive_mutex> guard(mapmtx);
   if(i<0 || i>= (int)map.size()){
      assert(false);
   }
   return map[i];
}



void DArrayBase::setResponsible(size_t slot, size_t _worker){
  boost::lock_guard<boost::recursive_mutex> guard(mapmtx);
  assert(slot < map.size());
  assert(_worker < worker.size());
  map[slot] = _worker;
}


ListExpr DArrayBase::toListExpr() const{
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
  boost::lock_guard<boost::recursive_mutex> guard(mapmtx);
  if(isStdMap()){ 
     return nl->ThreeElemList(nl->SymbolAtom(name), 
                              nl->IntAtom(getSize()), 
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

template<class R>
R* DArrayBase::readFrom(ListExpr list){
   if(listutils::isSymbolUndefined(list)){
      std::vector<uint32_t> m;
      return new R(m,"");
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
   if(nl->AtomType(nl->Second(list))==IntType){
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
          if(mv<0 || mv>=(int)v.size()){
             return 0;
          }
          m.push_back(mv);
      }
   }
   R* result = new R(m,name);
   swap(result->worker,v);
   result->defined = true;
   return result;
}


template<class R>
bool DArrayBase::open(SmiRecord& valueRecord, size_t& offset, 
                 const ListExpr __attribute__((unused)) typeInfo,
                 Word& result){
   bool defined;
   result.addr = 0;
   if(!readVar<bool>(defined,valueRecord,offset)){
      return false;
   } 
   if(!defined){
     std::vector<uint32_t> m;
     result.addr = new R(m,"");
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
   DArrayBase* res = 0; 

   for(size_t i=0;i<size;i++){
        if(!readVar<uint32_t>(me,valueRecord,offset)){
           return false;
        }
        m.push_back(me);
    }
    res = new R(m,name);
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

bool DArrayBase::save(SmiRecord& valueRecord, size_t& offset,
                 const ListExpr __attribute__((unused)) typeInfo, 
                 Word& value) {

    DArrayBase* a = (DArrayBase*) value.addr;
    // defined flag
    if(!writeVar(a->defined,valueRecord,offset)){
      return false;
    }
    if(!a->defined){
       return true;
    }
    // size
    size_t s = a->getSize();
    if(!writeVar(s,valueRecord,offset)){
      return false;
    }
    // name
    if(!writeVar(a->name, valueRecord, offset)){
      return false;
    }
    // map
    boost::lock_guard<boost::recursive_mutex> guard(a->mapmtx);
    for(size_t i=0;i<a->map.size();i++){
        if(!writeVar(a->map[i], valueRecord,offset)){
           return false;
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


std::vector<uint32_t> DArrayBase::createStdMap(const uint32_t size, 
                                     const int numWorkers){
   std::vector<uint32_t> map;
   for(uint32_t i=0;i<size;i++){
       map.push_back(i%numWorkers);
   }
   return map;
}


void DArrayBase::print(std::ostream& out)const{
  if(!defined){
     out << "undefined";
     return;
  }

  boost::lock_guard<boost::recursive_mutex> guard(mapmtx);
  out << "Name : " << name <<", size : " << map.size()
      << " workers : [" ;
  for(size_t i =0;i<worker.size();i++){
     if(i>0) out << ", ";
     worker[i].print(out);
  }
  out << "]";
  out << "map = [";
  for(uint32_t i=0;i<map.size();i++){
    if(i>0){
      out << ", ";
    }
    out << i << " -> " << map[i];
  }
  out << "]";
}

void DArrayBase::makeUndefined(){
   boost::lock_guard<boost::recursive_mutex> guard(mapmtx);
   DistTypeBase::makeUndefined();
   map.clear();
}





 bool DArrayBase::checkMap(){
     boost::lock_guard<boost::recursive_mutex> guard(mapmtx);
     for(size_t i=0;i<map.size();i++){
       if(map[i] >= worker.size()){
         return false;
       }
     }
     return true;
   }

bool DArrayBase::isStdMap() const{
  boost::lock_guard<boost::recursive_mutex> guard(mapmtx);
  int s = worker.size();
  for(size_t i=0;i<map.size();i++){
    if(map[i]!= i%s){
      return false;
    }
  }
  return true;
}




ostream& operator<<(ostream& out, const DArrayElement& elem){
  elem.print(out);
  return out;
}


ostream& operator<<(ostream& out, const DArrayBase& a){
  a.print(out);
  return out;
}


template<arrayType T>
bool DArrayT<T>::checkType(const ListExpr list){
    if(!nl->HasLength(list,2)){
       return false;
    }  
    if(!listutils::isSymbol(nl->First(list), BasicType())){
        return false;
    }
    // for other than DARRAY, only relations are allowed as a
    // subtype
    if(T!=DARRAY){
      return Relation::checkType(nl->Second(list));
    }
    // check that second arghument is an valid tyoe
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



/*
Implementation of ~DMatrix~

*/

const std::string DFMatrix::BasicType(){
     return "dfmatrix";
}

DFMatrix::DFMatrix(const size_t _size , const std::string& _name):
      DistTypeBase(_name), size(_size) {
   if(!stringutils::isIdent(name) || size ==0 ){ // invalid
      name = "";
      defined = false;
      size = 0;
      return;
   }
   defined = true;
}


DFMatrix::DFMatrix(const size_t _size, const std::string& _name, 
          const std::vector<DArrayElement>& _worker): 
   DistTypeBase(_worker, _name), size(_size) {
   if(!stringutils::isIdent(name) || size ==0 ){ 
      // invalid
      name = "";
      defined = false;
      size = 0;
      return;
   }
   defined = true;
}

void DFMatrix::setSize(size_t newSize){
  assert(newSize > 0);
  this->size = newSize;
}

bool DFMatrix::open(SmiRecord& valueRecord, size_t& offset, 
                 const ListExpr __attribute__((unused)) typeInfo, 
                 Word& result){
   bool defined;
   result.addr = 0;
   if(!readVar<bool>(defined,valueRecord,offset)){
      return false;
   } 
   if(!defined){
     std::vector<uint32_t> m;
     result.addr = new DFMatrix(0,"");
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

   DFMatrix* res = new DFMatrix(size,name);
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

bool DFMatrix::save(SmiRecord& valueRecord, size_t& offset,
                 const ListExpr __attribute__((unused)) typeInfo, 
                 Word& value) {

    DFMatrix* a = (DFMatrix*) value.addr;
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

bool DFMatrix::checkType(ListExpr e){
  if(!nl->HasLength(e,2)) {
    return false;
  }
  if(!listutils::isSymbol(nl->First(e), BasicType())){
    return false;
  }
  return Relation::checkType(nl->Second(e));
}


ListExpr DFMatrix::toListExpr()const{
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
  return nl->ThreeElemList(nl->SymbolAtom(name), 
                          nl->IntAtom(size), 
                          wl); 
}


DFMatrix* DFMatrix::readFrom(ListExpr list){
   if(listutils::isSymbolUndefined(list)){
      return new DFMatrix(0,"");
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
   size_t s;
   if(nl->AtomType(nl->Second(list))==IntType){
      int size = nl->IntValue(nl->Second(list));
      if(size <=0){
         return 0;
      }
      s = size;
   } else {
      return 0;
   }
   DFMatrix* result = new DFMatrix(s, name, v);
   result->defined = true;
   return result;
}



// template instantiaons

template bool DArrayBase::open<DArray>(SmiRecord&, size_t&, 
                                       const ListExpr, Word&);
template bool DArrayBase::open<DFArray>(SmiRecord&, size_t&, 
                                       const ListExpr, Word&);

template DArray* DArrayBase::readFrom<DArray>(ListExpr);
template DFArray* DArrayBase::readFrom<DFArray>(ListExpr);

template class DArrayT<DARRAY>;
template class DArrayT<DFARRAY>;




} // end of namespace distributed2




