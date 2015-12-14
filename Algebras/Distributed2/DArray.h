
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

#ifndef DARRAY_H
#define DARRAY_H

#include <string>
#include "NestedList.h"
#include "RelationAlgebra.h"

#include "Dist2Helper.h"

namespace distributed2{

/*

2 Class ~DArrayElement~

This class represents information about a single worker of a DArray.

*/

class DArrayElement{
  public:
     DArrayElement(const std::string& _server, const int _port,
                    const int _num, const std::string& _config);

     DArrayElement(const DArrayElement& src);

     DArrayElement& operator=(const DArrayElement& src);


     ~DArrayElement();

      inline void setNum(const int num){
         this->num = num;
      }

     void set(const std::string& server, const int port, 
              const int num, const std::string& config);


     bool operator==(const DArrayElement& other) const;
     
     inline bool operator!=(const DArrayElement& other) const{
       return   !((*this) == other);
     }

     bool operator<(const DArrayElement& other) const;
     
     bool operator>(const DArrayElement& other) const;
     
     ListExpr toListExpr();

     bool readFrom(SmiRecord& valueRecord, size_t& offset);

     bool saveTo(SmiRecord& valueRecord, size_t& offset);

     void print(std::ostream& out)const;

     inline std::string getHost()const{ return server; }
     inline int getPort() const {return port; }
     inline std::string getConfig() const{ return config; }
     inline int getNum() const{ return num; }


     template<class H, class C>
     static DArrayElement* createFromTuple(Tuple* tuple, int num, 
                                   int hostPos, int portPos, int configPos){

         if(!tuple || (num < 0) ) {
            return 0;
         }

         H* CcHost = (H*) tuple->GetAttribute(hostPos);
         CcInt* CcPort = (CcInt*) tuple->GetAttribute(portPos);
         C* CcConfig = (C*) tuple->GetAttribute(configPos);

         if(!CcHost->IsDefined() || !CcPort->IsDefined() || 
            !CcConfig->IsDefined()){
             return 0;
         }
         std::string host = CcHost->GetValue();
         int port = CcPort->GetValue();
         std::string config = CcConfig->GetValue();
         if(port<=0){
            return 0;
         }
         return new DArrayElement(host,port,num,config);
     }


  private:
     std::string server;
     uint32_t port;
     uint32_t num;
     std::string config;
};

std::ostream& operator<<(std::ostream& out, const DArrayElement& elem);


bool InDArrayElement(ListExpr list, DArrayElement& result);

/*
3 Class ~DArray~

This class represents the Secondo type ~darray~. It just stores the information
about a connection to a remote server. The actual connections are stored within
the algebra instance.

*/

enum arrayType{DARRAY,DFARRAY,DFMATRIX};

template<arrayType Type>
class DArrayT{
  public:

/*
3.1 Constructors

The constructors create a darray from predefined values.

*/

     DArrayT(const std::vector<uint32_t>& _map, const std::string& _name);

     DArrayT(const size_t _size , const std::string& _name);

     DArrayT(const std::vector<uint32_t>& _map, const std::string& _name, 
               const std::vector<DArrayElement>& _worker);

     DArrayT(const size_t _size, const std::string& _name, 
               const std::vector<DArrayElement>& _worker);

     explicit DArrayT(int dummy) {} // only for cast function

     DArrayT(const DArrayT<Type>& src);

/*
3.2 Assignment Operator

*/
     template<arrayType T>
     DArrayT& operator=(const DArrayT<T>& src);

/*
3.3 Destructor

*/
     ~DArrayT();


/*
3.4 ~getWorkerNum~

This fucntion returns the worker that is responsible for
the given index. This operation cannot applied to a 
DFMATRIX.

*/
    uint32_t getWorkerNum(uint32_t index);

/*
3.5 ~getType~

Returns the template type.

*/
    arrayType getType();

/*
3.6 ~setSize~

Sets a new size. This operation is only allowed for
the DFMATRIX type.

*/

    void setSize(size_t newSize);

/*
3.6 ~set~

This sets the size, the name, and the worker for a 
darray. The map from index to workers is the
standard map.

*/
    void set(const size_t size, const std::string& name, 
              const std::vector<DArrayElement>& worker);


/*
3.7 ~equalMapping~

Checks whether the mappings from indexes to the workers
are equal for two darray types.

*/
     template<class AT>
     bool equalMapping(AT& a, bool ignoreSize );


/*
3.9 ~set~

Sets the mapping, the workers and the name for a darray.
The size is extracted from the mapping.

*/
    void set(const std::vector<uint32_t>& m, const std::string& name, 
              const std::vector<DArrayElement>& worker);

/*
3.10 ~IsDefined~

Checks whether this darray is in a defined state.

*/
     bool IsDefined();

/*
3.11 ~BasicType~

Returns the basic type of a  darray. The result depend on the
template type.

*/

     static const std::string BasicType();

/*
3.12 ~checkType~

Checks wether the argument is complete decsription of a darray.

*/
     static const bool checkType(const ListExpr list);

/*
3.13 Some Getters

*/
     size_t numOfWorkers() const;

     size_t getSize() const;
     
     DArrayElement getWorker(int i);

     std::string getName() const;


/*
3.14 Some setters

*/

     void makeUndefined();

     void setStdMap(size_t size);

     DArrayElement getWorkerForSlot(int i);

     size_t getWorkerIndexForSlot(int i);
     
     void setResponsible(size_t slot, size_t _worker);

     bool setName( const std::string& n);

/*
3.15 ~toListExpr~

Returns the list representation for this darray.

*/

     ListExpr toListExpr();


/*
3.16 ~readFrom~

Read a darray value from a list. If the list is not a valid
description, null is returned. The caller is responsible for 
deleting the return value, if the is one.

*/
     static DArrayT<Type>* readFrom(ListExpr list);

/*
3.17 ~open~

Reads the content of darray from a SmiRecord.

*/

     static bool open(SmiRecord& valueRecord, size_t& offset, 
                      const ListExpr typeInfo, Word& result);


/*
3.18 ~save~

Saves a darray to an SmiRecord.

*/
     static bool save(SmiRecord& valueRecord, size_t& offset,
                      const ListExpr typeInfo, Word& value);

/*
3.19 ~createStdMap~

Returns a vector representing the standard mapping from index to
worker.

*/
     static std::vector<uint32_t> createStdMap(const uint32_t size, 
                                          const int numWorkers);

/*
3.20 ~print~

Writes the content to an output stream.

*/
     void print(std::ostream& out);



/*
3.21 ~equalWorker~

Checks whether the worker definitions are equal.

*/
      template<class TE>
      bool equalWorker(const TE& a) const;


/*
3.22 ~createFromRel ~

Reads the content of a darray from a relation defining 
the workers. The name and the size are explicitely given.
The relation must have at least 3 attributes. The attribute 
at position hostPos must be of type H (CcString or FText) and
describes the host of the worker. At potPos, a ~CcInt~ describes 
the port of the SecondoMonitor. At position configPos, an attribute
of type C (CcString of FText) describes the configuration file
for connecting with the worker. 

*/
      template<class H, class C>
      static DArrayT<Type> createFromRel(Relation* rel, int size, 
                              std::string name,
                              int hostPos, int portPos, int configPos);



  friend class DArrayT<DARRAY>;
  friend class DArrayT<DFARRAY>;
  friend class DArrayT<DFMATRIX>;

  private:
    std::vector<DArrayElement> worker; // connection information
    std::vector<uint32_t> map;  // map from index to worker
    size_t  size; // corresponds with map size except map is empty
    std::string name;  // the basic name used on workers
    bool defined; // defined state of this array


/*
3.23  ~checkMap~

Checks whether the contained map is valid.

*/
   bool checkMap();

/*
3.24 ~isStdMap~

Checks whether the contained map is a standard map.

*/
   bool isStdMap();


/*
3.24 ~equalWorker~

Check for equaliness of workers.

*/
   bool equalWorker(const std::vector<DArrayElement>& w) const;

};



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
    worker(_worker),map(_map), size(map.size),name(_name) {

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
arrayType DArrayT<Type>::getType(){
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
 bool DArrayT<Type>::equalWorker(const TE& a) const{
    return equalWorker(a.worker);
 }


template<arrayType Type>
 template<class H, class C>
 DArrayT<Type> DArrayT<Type>::createFromRel(Relation* rel, int size,
                         std::string name, int hostPos, int portPos, int 
                         configPos){
     std::vector<uint32_t> m;
     DArrayT<Type> result(m,"");
     if(size<=0){
        result.defined = false;
        return result;
     }
     if(!stringutils::isIdent(name)){
        result.defined = false;
        return result;
     }
     result.defined = true;
     result.name = name;

     GenericRelationIterator* it = rel->MakeScan();
     Tuple* tuple;
     while((tuple = it->GetNextTuple())){
        DArrayElement* elem = 
               DArrayElement::createFromTuple<H,C>(tuple,
               result.worker.size(),hostPos, 
               portPos, configPos);
        tuple->DeleteIfAllowed();
        if(elem){
           result.worker.push_back(*elem);
           delete elem;
        }
     } 
     delete it;
     result.setStdMap(size);
     return result;

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





typedef DArrayT<DARRAY> DArray;
typedef DArrayT<DFARRAY> DFArray;
typedef DArrayT<DFMATRIX> DFMatrix;

} // end of namespace distributed2


#endif


