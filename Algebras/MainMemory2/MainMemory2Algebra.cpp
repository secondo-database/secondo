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


//[_] [\_]

*/

#include "MemCatalog.h"
#include "Algebra.h"
#include "NestedList.h"
#include "QueryProcessor.h"
#include "ListUtils.h"
#include "Attribute.h"
#include "AlgebraManager.h"
#include "Operator.h"
#include "StandardTypes.h"
#include "NList.h"
#include "Symbols.h"
#include "SecondoCatalog.h"
#include "SecondoSystem.h"
#include "Algebras/Relation-C++/RelationAlgebra.h"
#include "Algebras/OrderedRelation/OrderedRelationAlgebra.h"
//#include "GraphAlgebra.h"
#include "Stream.h"
#include "MMRTree.h"
#include "MMMTree.h"
#include "Algebras/FText/FTextAlgebra.h"
//#include "MovingRegionAlgebra.h"
#include "Algebras/Rectangle/RectangleAlgebra.h"
#include "Algebras/Spatial/Point.h"
#include "Algebras/Spatial/SpatialAlgebra.h"
#include "Algebras/Temporal/TemporalAlgebra.h"
#include "AvlTree.h"
#include "StopWatch.h"

#include "PersistentNTree.h"
#include "NTree.h"
#include "ttree.h"

#include "MPointer.h"
// #include "mapmatch.h"


#include <ctime>
#include <exception>
#include <string>
#include <map>
#include <vector>
#include <limits>
#include <algorithm>

#include <ctime>


#include "MemoryVectorObject.h"
#include "MemoryPQueue.h"
#include "MemoryStack.h"

#include "MGraph2.h"
#include "MGraph3.h"
#include "Algebras/Standard-C++/LongInt.h"

#include <chrono>
#include <unordered_map>


using namespace std;
using namespace temporalalgebra;

extern NestedList* nl;
extern QueryProcessor *qp;


ostream& operator<<(ostream& o, const mm2algebra::AttrIdPair& t) {
  o << "(";
  t.getAttr()->Print(o);
  o << ", " << t.getTid()   << ")";
  return o;
}


namespace mm2algebra {

MemCatalog* catalog = 0;

MemCatalog* getMemCatalog(){ 
   return catalog;
}



#define MEMORYMTREEOBJECT "memoryMTreeObject"



bool checkUsesArgs(ListExpr args){
  ListExpr tmp = args;
  while(!nl->IsEmpty(tmp)){
     if(!nl->HasLength(nl->First(args),2)){
        return false;
     }
     tmp = nl->Rest(tmp);
  }
  return true;
}


bool getMemSubType(ListExpr type, ListExpr& res){
  if(MPointer::checkType(type)){
     res = nl->Second(nl->Second(type));
     return true;
  }
  if(!Mem::checkType(type)){
    return false;
  }
  res = nl->Second(type);
  return true;
}

/*

4 Auxiliary functions

*/
namespace rtreehelper{


   int getDimension(ListExpr type){
      if(listutils::isKind(type,Kind::SPATIAL2D())) return 2;
      if(listutils::isKind(type,Kind::SPATIAL3D())) return 3;
      if(listutils::isKind(type,Kind::SPATIAL4D())) return 4;
      if(listutils::isKind(type,Kind::SPATIAL8D())) return 8;
      return -1;   
   }

   string BasicType(){
     return "rtree";
   }

   bool checkType(ListExpr type){
      if(!nl->HasLength(type,2)){
        return false;
      }
      if(!listutils::isSymbol(nl->First(type),BasicType())){
         return false;
      }
      if(nl->AtomType(nl->Second(type))!=IntType){
         return false;
      }
      int dim = nl->IntValue(nl->Second(type));
      return (dim==2) || (dim==3) || (dim==4) || (dim==8);
   }

   bool checkType(ListExpr type, int dim){
     if(!checkType(type)){
       return false;
     }
     return nl->IntValue(nl->Second(type))==dim;
   }

   ListExpr getType(int dim){
     return nl->TwoElemList( nl->SymbolAtom(BasicType()),
                             nl->IntAtom(dim));
   }





}

/*

Some functions for getting a certain memory object by name.
The name may be given as a CcString or as a Mem instance. 
A lot of checkes ensure correctness of operations. If one of 
the test failes, the result will be 0 , otherwise a 
pointer to the memory object.

*/

template<class T>
memAVLtree* getAVLtree(T* treeN, ListExpr subtype){
   if(!treeN->IsDefined()){
      return 0;
   }
   string treen = treeN->GetValue();
   if(!catalog->isMMOnlyObject(treen) || !catalog->isAccessible(treen)){
      return 0;
   }
   ListExpr treet = nl->Second(catalog->getMMObjectTypeExpr(treen));
   if(!MemoryAVLObject::checkType(treet)){
       return 0;
   }
   if(!nl->Equal(nl->Second(treet),subtype)){
      return 0;
   }
   MemoryAVLObject* mao = (MemoryAVLObject*)catalog->getMMObject(treen);
   return mao->getAVLtree();
}

template<class T>
memAVLtree* getAVLtree(T* treeN){
   if(!treeN->IsDefined()){
      return 0;
   }
   string treen = treeN->GetValue();
   if(!catalog->isMMOnlyObject(treen) || !catalog->isAccessible(treen)){
      return 0;
   }
   ListExpr treet = nl->Second(catalog->getMMObjectTypeExpr(treen));
   if(!MemoryAVLObject::checkType(treet)){
       return 0;
   }
   MemoryAVLObject* mao = (MemoryAVLObject*)catalog->getMMObject(treen);
   return mao->getAVLtree();
}

template<>
memAVLtree* getAVLtree(MPointer* tree, ListExpr subtype){
  MemoryObject* ptr = (*tree)();
  return ((MemoryAVLObject*)ptr)->getAVLtree();
}

template<>
memAVLtree* getAVLtree(MPointer* tree){
  MemoryObject* ptr = (*tree)();
  return ((MemoryAVLObject*)ptr)->getAVLtree();
}


template<class T>
MemoryRelObject* getMemRel(T* relN){

  if(!relN->IsDefined()) { return 0; }
  string reln = relN->GetValue();
  if(!catalog->isMMOnlyObject(reln) || !catalog->isAccessible(reln)) {
    return 0;
  }
  ListExpr relType = nl->Second(catalog->getMMObjectTypeExpr(reln));
    
  if(!Relation::checkType(relType)) { return 0; }
  
  return (MemoryRelObject*) catalog->getMMObject(reln);
}

template<>
MemoryRelObject* getMemRel( MPointer* rel){
   return (MemoryRelObject*) ((*rel)());
}



template<class T>
MemoryRelObject* getMemRel(T* relN, ListExpr tupleType, string& error) {
  if(!relN->IsDefined()) { 
     error = "Relation name not defined";
     return 0;
  }
  
  string reln = relN->GetValue();
  if(!catalog->isMMOnlyObject(reln) || !catalog->isAccessible(reln)){
    error ="relation '"+reln+"' not known or not accessible";
    return 0;
  }
  ListExpr relType = nl->Second(catalog->getMMObjectTypeExpr(reln));
    
  if(!Relation::checkType(relType)){
     error = "main memory object '"+reln+"' is not a relation";
     return 0;
  }
  if(!nl->Equal(nl->Second(relType), tupleType)){
     error = string("memory relation has an unexpected tuple type \n")
             + "expected: " + nl->ToString(tupleType) +"\n"
             + "found   : " + nl->ToString(nl->Second(relType));
     return 0;
  }
  error="";
  return (MemoryRelObject*) catalog->getMMObject(reln);
}


template<>
MemoryRelObject* getMemRel(MPointer* rel, ListExpr tupleType, string& error) {
   return (MemoryRelObject*) ((*rel)());
}



template<class T>
MemoryRelObject* getMemRel(T* relN, ListExpr tupleType) {
  string dummy;
  return getMemRel(relN, tupleType, dummy); 
}

template<>
MemoryRelObject* getMemRel(MPointer* rel, ListExpr tupleType) {
  string dummy;
  return getMemRel(rel, tupleType, dummy); 
}

template<class T>
MemoryORelObject* getMemORel(T* relN, ListExpr tupleType){
  if(!relN->IsDefined()){
     return 0;
  }
  string reln = relN->GetValue();
  if(!catalog->isMMOnlyObject(reln) || !catalog->isAccessible(reln)){
    return 0;
  }
  ListExpr relType = nl->Second(catalog->getMMObjectTypeExpr(reln));

  if(!listutils::isOrelDescription(relType)){
     return 0;
  }
  return (MemoryORelObject*) catalog->getMMObject(reln);
}

template<>
MemoryORelObject* getMemORel(MPointer* rel, ListExpr tupleType){
  return (MemoryORelObject*) ((*rel)());
}


template<class T>
MemoryGraphObject* getMemGraph(T* aN){
  if(!aN->IsDefined()){
     return 0;
  }
  std::string an = aN->GetValue();
  if(!catalog->isMMOnlyObject(an) || !catalog->isAccessible(an)){
    return 0;
  }
  ListExpr type = nl->Second(catalog->getMMObjectTypeExpr(an));
  if(!MemoryGraphObject::checkType(type)){
    return 0;
  }
  
  return (MemoryGraphObject*) catalog->getMMObject(an);
}

template<>
MemoryGraphObject* getMemGraph(MPointer* a){
  return (MemoryGraphObject*) ((*a)());
}

template<class T>
MemoryAttributeObject* getMemAttribute(T* aN, ListExpr _type){
  if(!aN->IsDefined()){
     return 0;
  }
  string an = aN->GetValue();
  if(!catalog->isMMOnlyObject(an) || !catalog->isAccessible(an)){
    return 0;
  }
  ListExpr type = nl->Second(catalog->getMMObjectTypeExpr(an));
  if(!Attribute::checkType(type)){
    return 0;
  }
  if(!nl->Equal(type,_type)){
    return 0;
  }
  return (MemoryAttributeObject*) catalog->getMMObject(an);
}

template<>
MemoryAttributeObject* getMemAttribute(MPointer* a, ListExpr _type){
  return (MemoryAttributeObject*) ((*a)());
}

template<class T, int dim>
MemoryRtreeObject<dim>* getRtree(T* tN){
  if(!tN->IsDefined()){
    return 0;
  }
  string tn = tN->GetValue();
  if(!catalog->isMMOnlyObject(tn)){ 
    // because we store only rectangle, we do'nt have to check
    // accessibility
    return 0;
  }
  ListExpr type = nl->Second(catalog->getMMObjectTypeExpr(tn));
  if(!rtreehelper::checkType(type,dim)){
    return 0;
  }
  return (MemoryRtreeObject<dim>*) catalog->getMMObject(tn);
}


template<class T, int dim>
MemoryRtreeObject<dim>* getRtree(MPointer* p){
   return (MemoryRtreeObject<dim>*) ((*p)());
}

template<int dim>
MemoryRtreeObject<dim>* getRtree(MPointer* t){
  return (MemoryRtreeObject<dim>*) ((*t)());
}


template<class T, class K>
MemoryMtreeObject<K, StdDistComp<K> >* getMtree(T* name){
  if(!name->IsDefined()){
    return 0;
  }
  string n = name->GetValue();
  if(!catalog->isMMOnlyObject(n)){
    return 0;
  } 
  ListExpr type = nl->Second(catalog->getMMObjectTypeExpr(n));
  if(!MemoryMtreeObject<K, StdDistComp<K> >::checkType(type)){
    return 0;
  }
  if(!K::checkType(nl->Second(type))){
     return 0;
  }
  return (MemoryMtreeObject<K, StdDistComp<K> >*) 
          catalog->getMMObject(n);
}

template<class T, class K, class L>
MemoryMtreeObject<pair<K, L>, StdDistCompExt<K, L> >* getMtree(T* name) {
  if (!name->IsDefined()) {
    return 0;
  }
  string n = name->GetValue();
  if (!catalog->isMMOnlyObject(n)) {
    return 0;
  } 
  ListExpr type = nl->Second(catalog->getMMObjectTypeExpr(n));
  cout << "type is " << nl->ToString(type);
  if (!MemoryMtreeObject<Tuple, StdDistCompExt<K, L> >::checkType(type)) {
    return 0;
  }
  if (!Tuple::checkType(nl->Second(type))) {
     return 0;
  }
  return (MemoryMtreeObject<pair<K, L>, StdDistCompExt<K, L> >*) 
          catalog->getMMObject(n);
}

template<class T, class K>
MemoryMtreeObject<K, StdDistComp<K> >* getMtree(MPointer* t){
   return (MemoryMtreeObject<K, StdDistComp<K> >*) ((*t)());
}

template<class T, class K, class L>
MemoryMtreeObject<pair<K, L>, StdDistCompExt<K, L> >* getMtree(MPointer* t) {
  return (MemoryMtreeObject<pair<K, L>, StdDistCompExt<K, L> >*)((*t)());
}

template<class T, class K, int variant>
MemoryNtreeObject<K, StdDistComp<K>, variant>* getNtreeX(T* name) {
  if (!name->IsDefined()) {
    return 0;
  }
  string n = name->GetValue();
  if (!catalog->isMMOnlyObject(n)) {
    return 0;
  } 
  ListExpr type = nl->Second(catalog->getMMObjectTypeExpr(n));
  if (!MemoryNtreeObject<K, StdDistComp<K>, variant>::checkType(type)) {
    return 0;
  }
  if (!K::checkType(nl->Second(type))) {
     return 0;
  }
  return (MemoryNtreeObject<K,StdDistComp<K>, variant>*)catalog->getMMObject(n);
}

template<class T, class K, class L, int variant>
MemoryNtreeObject<pair<K,L>, StdDistCompExt<K,L>, variant>* getNtreeX(T* name) {
  if (!name->IsDefined()) {
    return 0;
  }
  string n = name->GetValue();
  if (!catalog->isMMOnlyObject(n)) {
    return 0;
  } 
  ListExpr type = nl->Second(catalog->getMMObjectTypeExpr(n));
  cout << "type is " << nl->ToString(type);
  if (!MemoryNtreeObject<Tuple, StdDistCompExt<K,L>, variant>::checkType(type)){
    return 0;
  }
  if (!Tuple::checkType(nl->Second(type))) {
     return 0;
  }
  return (MemoryNtreeObject<pair<K, L>, StdDistCompExt<K, L>, variant>*) 
          catalog->getMMObject(n);
}

template<class T, class K, int variant>
MemoryNtreeObject<K, StdDistComp<K>, variant>* getNtreeX(MPointer* t) {
   return (MemoryNtreeObject<K, StdDistComp<K>, variant>*) ((*t)());
}

template<class T, class K, class L, int variant>
MemoryNtreeObject<pair<K, L>, StdDistCompExt<K, L>, variant>* 
                                                        getNtreeX(MPointer* t) {
  return (MemoryNtreeObject<pair<K,L>, StdDistCompExt<K, L>, variant>*)((*t)());
}

template<class T>
MemoryTTreeObject* getTtree(T* treeN){
   if(!treeN->IsDefined()){
      return 0;
   }
   string treen = treeN->GetValue();
   if(!catalog->isMMOnlyObject(treen) || !catalog->isAccessible(treen)){
     return 0;
   }
   ListExpr treet = nl->Second(catalog->getMMObjectTypeExpr(treen));
   if(!MemoryTTreeObject::checkType(treet)){
     return 0;
   }
   return (MemoryTTreeObject*)catalog->getMMObject(treen);
}

template<>
MemoryTTreeObject* getTtree(MPointer* tree){
  return (MemoryTTreeObject*) ((*tree)());
}


template<class T>
MemoryVectorObject* getMVector(T* vecN){

  if(!vecN->IsDefined()) { return 0; }
  string vecn = vecN->GetValue();
  if(!catalog->isMMOnlyObject(vecn) || !catalog->isAccessible(vecn)) {
    return 0;
  }
  ListExpr vecType = nl->Second(catalog->getMMObjectTypeExpr(vecn));
    
  if(!MemoryVectorObject::checkType(vecType)) { return 0; }
  
  return (MemoryVectorObject*) catalog->getMMObject(vecn);
}

template<>
MemoryVectorObject* getMVector( MPointer* vec){
   return (MemoryVectorObject*) ((*vec)());
}



template<class T>
MemoryPQueueObject* getMemoryPQueue(T* qN){
   if(!qN->IsDefined()){
      return 0;
   }
   string qn = qN->GetValue();
   if(!catalog->isMMOnlyObject(qn) || !catalog->isAccessible(qn)){
      return 0;
   }
   ListExpr qt = nl->Second(catalog->getMMObjectTypeExpr(qn));
   if(!MemoryPQueueObject::checkType(qt)){
       return 0;
   }
   return (MemoryPQueueObject*)catalog->getMMObject(qn);
}

template<>
MemoryPQueueObject* getMemoryPQueue(MPointer* q){
   return (MemoryPQueueObject*) ((*q)());
}

template<class T>
MemoryStackObject* getMemoryStack(T* qN){
   if(!qN->IsDefined()){
      return 0;
   }
   string qn = qN->GetValue();
   if(!catalog->isMMOnlyObject(qn) || !catalog->isAccessible(qn)){
      return 0;
   }
   ListExpr qt = nl->Second(catalog->getMMObjectTypeExpr(qn));
   if(!MemoryStackObject::checkType(qt)){
       return 0;
   }
   return (MemoryStackObject*)catalog->getMMObject(qn);
}

template<>
MemoryStackObject* getMemoryStack(MPointer* q){
   return (MemoryStackObject*) ((*q)());
}


template<class M, class S>
M* getMemObject(S* qN){
   if(!qN->IsDefined()){
      return 0;
   }
   string qn = qN->GetValue();
   if(!catalog->isMMOnlyObject(qn) || !catalog->isAccessible(qn)){
      return 0;
   }
   ListExpr qt = nl->Second(catalog->getMMObjectTypeExpr(qn));
   if(!M::checkType(qt)){
       return 0;
   }
   return (M*)catalog->getMMObject(qn);
}


// same function but without type checking of the graph
template<class M, class S>
M* getMemObject2(S* qN){
   if(!qN->IsDefined()){
      return 0;
   }
   string qn = qN->GetValue();
   if(!catalog->isMMOnlyObject(qn) || !catalog->isAccessible(qn)){
      return 0;
   }
   return (M*)catalog->getMMObject(qn);
}


template<class M>
M* getMemObject(MPointer* t){
   return (M*) ((*t)());
}

template<class M>
M* getMemObject2(MPointer* t){
   return (M*) ((*t)());
}

/*
Function returning the current database name.

*/

string getDBname() {
    SecondoSystem* sys = SecondoSystem::GetInstance();
    return sys->GetDatabaseName();
}

template<class T>
T* computeValue(ListExpr expression){
   Word queryResult;
   string typeString = "";
   string errorString = "";
   bool correct;
   bool evaluable;
   bool defined;
   bool isFunction;
   // use the queryprocessor for executing the expression
   try{
     qp->ExecuteQuery(expression, queryResult,
                      typeString, errorString, correct,
                      evaluable, defined, isFunction);
     // check correctness of the expression
     if(!correct || !evaluable || !defined || isFunction){
        assert(queryResult.addr == 0);
        return 0;
     }
     T* fn = (T*) queryResult.addr;
     return fn;
   } catch(...){
     return 0;
   }
}



/*
This function extract the type information from a 
memory object. If the type is given as mem object,
just this type is set to be the result. Otherwise type
has to be a CcString having the given value.
In case of an error (missing objects or whatever), the
result will be false and some errorsmessage is set. 
In the other case, the reuslt is true, errMsg is keept unchanged
and result will have the type in the memory.

*/

bool getMemTypeFromString(ListExpr type, ListExpr value, 
                ListExpr & result, string& error, 
                bool allowComplex){
    if(!CcString::checkType(type)){
       error = "not of type string";
       return false;
    }
    string n = "";
    if(nl->AtomType(value)==StringType){
      n = nl->StringValue(value);
    } else {
      if(!allowComplex){
        error = "only constant strings supported";
        return false;
      }
      CcString* N = computeValue<CcString>(value);
      if(!N){
        error = "could not compute result for " +nl->ToString(value);
        return false;
      }
      if(!N->IsDefined()){
        N->DeleteIfAllowed();
        error = "Undefined name for memory object";
        return false;
      }
      n = N->GetValue();
      N->DeleteIfAllowed();
    }

    if(!catalog->isMMOnlyObject(n)){
       error = n + " is not a memory object";
       return false;
    }
    if(!catalog->isAccessible(n)){
       error = n + " is not accessible";
       return false;
    }
    result = catalog->getMMObjectTypeExpr(n);
    if(!Mem::checkType(result)){
      error = "internal error: memory object " + n + " not of type mem";
      return false;
    }
    return true;
}

inline bool getMemTypeFromString(ListExpr typevalue, 
                ListExpr & result, string& error, 
                bool allowComplex = false){
  return getMemTypeFromString(nl->First(typevalue), nl->Second(typevalue),
                    result,error,allowComplex);
}
/*
some functions related to strings.

*/

string rtrim(string s, const string& delim = " \t\r\n")
{
  string::size_type last = s.find_last_not_of(delim.c_str());
  return last == string::npos ? "" : s.erase(last + 1);
}

string ltrim(string s, const string& delim = " \t\r\n")
{
  return s.erase(0, s.find_first_not_of(delim.c_str()));
}

string trim(string s, const string& delim = " \t\r\n")
{
  return ltrim(rtrim(s, delim), delim);
}

/*

5 Creating Operators

5.1 Operators ~memload~ and ~memloadflob~

Load a persistent relation into main memory. If there is not enough space
it breaks up. The created ~MemoryRelObject~ or ~MemoryORelOBject~
is usable but not complete.

5.1.1 Type Mapping Functions of operator ~memload~ 


*/
ListExpr memloadTM(ListExpr args) {

    if (nl->ListLength(args)!=1){
       return listutils::typeError("wrong number of arguments");
    }
    ListExpr sourceType = nl->First(args);
    // check for allowed types
    if(   !Attribute::checkType(sourceType)
       && !Relation::checkType(sourceType)
       && !OrderedRelation::checkType(sourceType)){
       return listutils::typeError("only rel, orel, and DATA are supported");
    }
    return MPointer::wrapType(Mem::wrapType(sourceType));
}


/*
5.1.3  The Value Mapping Functions of operator ~memload~

*/
template<bool flob>
int memloadVMRel (Word* args, Word& result,
                    int message, Word& local, Supplier s) {

    GenericRelation* r= static_cast<Relation*>( args[0].addr );
    MemoryRelObject* mmRelObject = new MemoryRelObject();
    ListExpr objectTypeExpr = nl->Second(nl->Second( qp->GetType(s)));
    bool memloadSucceeded = mmRelObject->relToVector(r,
                                      objectTypeExpr,
                                      getDBname(), 
                                      flob);
    result = qp->ResultStorage(s);
    MPointer* mp = (MPointer*) result.addr;
    if(memloadSucceeded){
       mp->setPointer(mmRelObject);
    } else {
       mp->setPointer(0);
    }
    // deletion control now by mpointer
    mmRelObject->deleteIfAllowed();
    return 0;
}

template<bool flob>
int memloadVMORel (Word* args, Word& result,
                    int message, Word& local, Supplier s) {

    GenericRelation* r= static_cast<OrderedRelation*>(args[0].addr);
    MemoryORelObject* mmORelObject = new MemoryORelObject();
    // expected orel description 
    ListExpr objectTypeExpr = nl->Second(nl->Second(qp->GetType(s)));
    bool memloadSucceeded = mmORelObject->relToTree(
                                      r,
                                      objectTypeExpr,
                                      getDBname(), flob);
    result = qp->ResultStorage(s);
    MPointer* mp = (MPointer*) result.addr;
    if(memloadSucceeded){
       mp->setPointer(mmORelObject);
    } else {
       mp->setPointer(0);
    }
    mmORelObject->deleteIfAllowed();
    return 0;
}


template<bool flob>
int memloadVMAttr (Word* args, Word& result,
                   int message, Word& local, Supplier s) {

   Attribute* attr = (Attribute*)args[0].addr;
   MemoryAttributeObject* mmA = new MemoryAttributeObject();
   ListExpr objectTypeExpr = nl->Second(nl->Second(qp->GetType(s)));
   bool memloadSucceeded = mmA->attrToMM(attr, objectTypeExpr,
                                getDBname(),flob);

   result = qp->ResultStorage(s);
   MPointer* mp = (MPointer*) result.addr;
   if(memloadSucceeded){
      mp->setPointer(mmA);
   } else {
      mp->setPointer(0);
   }
   mmA->deleteIfAllowed();
   return 0;
}

ValueMapping memloadVM[] = {
   memloadVMAttr<false>,
   memloadVMRel<false>,
   memloadVMORel<false>
};

int memloadSelect(ListExpr args){
  ListExpr a1 = nl->First(args);
  if(Attribute::checkType(a1)) return 0;
  if(Relation::checkType(a1)) return 1;
  if(OrderedRelation::checkType(a1)) return 2;
  return -1;
}

OperatorSpec memloadSpec(
  " P -> mpointer(mem(P)) , P in {DATA, rel, orel} ",
  " memload(_)",
  " Converts a peristent object into a memory representation. "
  " Flob will keept persistent.",
  " let mten = memload(ten)"
);

Operator memloadOp(
  "memload",
  memloadSpec.getStr(),
  3,
  memloadVM,
  memloadSelect,
  memloadTM
);

ValueMapping memloadflobVM[] = {
   memloadVMAttr<true>,
   memloadVMRel<true>,
   memloadVMORel<true>
};

OperatorSpec memloadflobSpec(
  " P -> mpointer(mem(P)) , P in {DATA, rel, orel} ",
  " memloadflob(_)",
  " Converts a peristent object into a memory representation. "
  " Flobs will also transferred into main memory.",
  " let mten = memloadflob(ten)"
);


Operator memloadflobOp(
  "memloadflob",
  memloadflobSpec.getStr(),
  3,
  memloadflobVM,
  memloadSelect,
  memloadTM
);


/*
5.3 Operator ~meminit~

Initialises the main memory which is used within the main memory algebra.
The default value is 256MB.
The maximum value is limited by the value set in ~SecondoConfig.ini~.
If the wanted value is smaller then the memory that is already in use,
the value will be set to the smallest possible value
without deleting any main memory objects.

*/

/*
5.3.1 Type Mapping Functions of operator ~meminit~ (int->int)

*/

ListExpr meminitTypeMap(ListExpr args)
{
  if(nl->ListLength(args)!=1){
     return listutils::typeError("wrong number of arguments");
  }
  if (!CcInt::checkType(nl->First(args))) {
  return listutils::typeError("int expected");
  }
  return listutils::basicSymbol<CcInt>();
}


/*

5.3.3  The Value Mapping Functions of operator ~meminit~

*/

int meminitValMap (Word* args, Word& result,
                    int message, Word& local, Supplier s) {

    int maxSystemMainMemory = qp->GetMemorySize(s);
    int newMainMemorySize = ((CcInt*)args[0].addr)->GetIntval();
    int res=0;

    if (newMainMemorySize<0){
        res=catalog->getMemSizeTotal();
    }
    else if ((double)newMainMemorySize <
                    (catalog->getUsedMemSize()/1024.0/1024.0)){
            res = (catalog->getUsedMemSize()/1024.0/1024.0)+1;
            catalog->setMemSizeTotal
                        (catalog->getUsedMemSize()/1024.0/1024.0+1);
        }
    else if (newMainMemorySize>maxSystemMainMemory){
            res = maxSystemMainMemory;
            catalog->setMemSizeTotal(maxSystemMainMemory);
    }
    else {
        res = newMainMemorySize;
        catalog->setMemSizeTotal(newMainMemorySize);
    }

    result  = qp->ResultStorage(s);
    CcInt* b = static_cast<CcInt*>(result.addr);
    b->Set(true, res);
    return 0;
}

/*

5.3.4 Description of operator ~meminit~

*/

OperatorSpec meminitSpec(
    "int -> int",
    "meminit(_)",
    "initialises the main memory, the maximum size "
    " is limited by the global memory which is set in SecondoConfig.ini",
    "query meminit(256)"
);

/*

5.3.5 Instance of operator ~meminit~

*/

Operator meminitOp (
    "meminit",
    meminitSpec.getStr(),
    meminitValMap,
    Operator::SimpleSelect,
    meminitTypeMap
);


/*
5.4 Operator ~mfeed~

~mfeed~ produces a stream of tuples from a main memory relation or
ordered relation, similar to the ~feed~-operator

5.4.1 Type Mapping Functions of operator ~mfeed~ (string -> stream(Tuple))

*/

ListExpr mfeedTypeMap(ListExpr args) {

    if(nl->ListLength(args)!=1){
        return listutils::typeError("wrong number of arguments");
    }
    ListExpr a1 = nl->First(args);
    if(MPointer::checkType(a1)){
      a1 = nl->Second(a1);
    }
    if(!Mem::checkType(a1)){
      return listutils::typeError("argument is not a memory object");
    }
    a1 = nl->Second(a1); // remove mem
    if(   !Relation::checkType(a1) 
       && !listutils::isOrelDescription(a1)){
      return listutils::typeError(
        "memory object is not a relation or ordered relation");
    }
    return nl->TwoElemList(nl->SymbolAtom(Symbol::STREAM()),
                           nl->Second(a1));
}


/*

5.4.3  The Value Mapping Functions of operator ~mfeed~

*/


class mfeedInfo{
  public:
    mfeedInfo(vector<Tuple*>* _rel) : rel(_rel), orel(0), pos(0) { }
    
    mfeedInfo(ttree::TTree<TupleWrap,TupleComp>* _orel) : 
         rel(0),orel(_orel),pos(0) {
      iter = orel->begin();
    }
    
    ~mfeedInfo() {}

    Tuple* next() {
      Tuple* res;
      if(rel){
        while(pos < rel->size()){
          res = (*rel)[pos];
          pos++;
          if(res){
            res->SetTupleId(pos);
            res->IncReference();
            return res;
          }
        }  
        return 0;
      } else {
        while(!iter.end()) {
          res = (*iter).getPointer();
          iter++;
          if(res){
            res->IncReference();
            return res;
          }
        }
        return 0;
      }
   }

  private:
     vector<Tuple*>* rel;
     ttree::TTree<TupleWrap,TupleComp>* orel;
     size_t pos;
     ttree::Iterator<TupleWrap,TupleComp> iter;
};


template<bool ordered>
int mfeedValMapMem (Word* args, Word& result,
                    int message, Word& local, Supplier s) {

  mfeedInfo* li = (mfeedInfo*) local.addr;

  switch (message) {
    
    case OPEN: {
      
      if(li) {
        delete li;
        local.addr=0;
      }
      
      Mem* oN = (Mem*) args[0].addr;
      if(!oN->IsDefined()){
        return 0;
      }
      if(ordered){
        MemoryORelObject* orel = getMemORel(oN, nl->Second(qp->GetType(s)));
        if(!orel) {
          return 0;
        }
        local.addr= new mfeedInfo(orel->getmmorel());
        return 0;
      } else {
        MemoryRelObject* rel = getMemRel(oN);
        if(!rel) {
          return 0;
        }
        local.addr= new mfeedInfo(rel->getmmrel());
        return 0;
      }
    }

    case REQUEST:
        result.addr=li?li->next():0;
        return result.addr?YIELD:CANCEL;

    case CLOSE:
        if(li) {
          delete li;
          local.addr = 0;
        }
        return 0;
  }

  return -1;
}



template<bool ordered>
int mfeedValMapMPointer (Word* args, Word& result,
                    int message, Word& local, Supplier s) {

  mfeedInfo* li = (mfeedInfo*) local.addr;
  switch(message){
    case OPEN:{
           if(li){
             delete li;
           }
           MPointer* p = (MPointer*) args[0].addr;
           if(ordered){
              MemoryORelObject* orel = (MemoryORelObject*) (*p)() ;
              if(!orel) {
               return 0;
              }
              local.addr= new mfeedInfo(orel->getmmorel());
           } else {
              MemoryRelObject* a =(MemoryRelObject*) (*p)(); 
              if(!a){
                 return 0;
              }
              local.addr= new mfeedInfo(a->getmmrel());
           }
           return 0;
         }
           
    case REQUEST:
            result.addr = li?li->next():0;
            return result.addr?YIELD:CANCEL;
    case CLOSE:{
             if(li){
               delete li;
               local.addr=0;
             }
             return 0;
          }
  
  }
  return -1;
}


ValueMapping mfeedVM[] = {
  mfeedValMapMem<false>,
  mfeedValMapMem<true>,
  mfeedValMapMPointer<false>,
  mfeedValMapMPointer<true>,
};

int mfeedSelect(ListExpr args) {
  ListExpr a1 = nl->First(args);
  int o1 = 0;
  if(MPointer::checkType(a1)){
     o1 = 2;
     a1 = nl->Second(a1);
  }
  a1 = nl->Second(a1);
  int o2 = Relation::checkType(a1)?0:1;
  return o1 + o2;
}


/*

5.4.4 Description of operator ~mfeed~


*/

OperatorSpec mfeedSpec(
    "{MREL, MOREL} -> stream(Tuple), ",
    "_ mfeed",
    "produces a stream from a main memory (ordered) relation",
    "query mten mfeed"
);

/*

5.4.5 Instance of operator ~mfeed~

*/

Operator mfeedOp (
    "mfeed",
    mfeedSpec.getStr(),
    4,
    mfeedVM,
    mfeedSelect,
    mfeedTypeMap
);


/*
5.5 Operator gettuples

This operator gets a stream of tuple ids, 
and a main memory relation and returns the
tuples from the ids out of the relation.

*/
ListExpr gettuplesTM(ListExpr args){
  string err ="stream(X) x {string, mem(rel(tuple))} expected";
  if(!nl->HasLength(args,2)){
    return listutils::typeError(err + " (wrong number of args)");
  }
  ListExpr a2;
  if(!getMemSubType(nl->Second(args),a2)){
    return listutils::typeError("second argument is not a memory object");
  }
  if(!Relation::checkType(a2)){
    return listutils::typeError(err + " (second arg is not a "
                                "memory relation.)");
  }
  ListExpr a1 = nl->First(args);
  if(Stream<TupleIdentifier>::checkType(a1)){
     return nl->TwoElemList( listutils::basicSymbol<Stream<Tuple> >(),
                             nl->Second(a2));
  }
  // second variant accepts a stream of tuples containing a tupleID
  if(!Stream<Tuple>::checkType(a1)){
    return listutils::typeError("1st arg is neither a stream of "
                                "tids nor a stream of tuples");
  }
  ListExpr attrList = nl->Second(nl->Second(a1));
  int index = 0;
  int tidIndex = 0;
  ListExpr iattr = nl->TheEmptyList();
  ListExpr last = nl->TheEmptyList();
  set<string> usednames;
  bool first = true;
  

  while(!nl->IsEmpty(attrList)){
    ListExpr attr = nl->First(attrList); 
    attrList = nl->Rest(attrList);
    index++;
    if(TupleIdentifier::checkType(nl->Second(attr))){
       if(tidIndex!=0){
          return listutils::typeError("incoming tuple stream contains "
                                      "more than one tid attribute");
       }
       tidIndex = index; 
    } else {
      if(first){
         iattr = nl->OneElemList(attr);
         last = iattr;
         first = false;
      } else {
         last = nl->Append(last, attr); 
      }
    }
  }  
  if(!tidIndex){
     return listutils::typeError("incoming tuple stream does not "
                                 "contains an attribute of type tid");
  }
  ListExpr relAttrList = nl->Second(nl->Second(a2));
  ListExpr resAttrList = listutils::concat(iattr, relAttrList);

  if(!listutils::isAttrList(resAttrList)){
     return listutils::typeError("found name conflicts");
  }

  ListExpr ret = nl->ThreeElemList(nl->SymbolAtom(Symbols::APPEND()),
                           nl->OneElemList(nl->IntAtom(tidIndex-1)),
                           nl->TwoElemList(
                               listutils::basicSymbol<Stream<Tuple> >(),
                               nl->TwoElemList(
                                   listutils::basicSymbol<Tuple>(),
                                   resAttrList)));
  return  ret;
}


class gettuplesInfo{
  public:
    gettuplesInfo(Word& _stream, MemoryRelObject* _rel): 
        stream(_stream), rel(_rel->getmmrel()){
      stream.open();
    }

    ~gettuplesInfo(){
        stream.close();
     }

     Tuple* next(){
        TupleIdentifier* tid;
        while((tid=stream.request())){
           if(!tid->IsDefined()){
             tid->DeleteIfAllowed();
           } else {
              TupleId id = tid->GetTid();
              tid->DeleteIfAllowed();
              if(id>0 && id<=rel->size()){
                 Tuple* res = (*rel)[id-1];
                 if(res){ // ignore deleted tuples
                    res->IncReference();
                    res->SetTupleId(id);
                    return res;
                 } else {
                    // cout << "ignore deleted tuple" << endl;
                    ;
                 }
              } else {
                  // cout << "ignore id " << id << endl;
                  ;
              }
           }
        }
        return 0;
     }

  private:
     Stream<TupleIdentifier> stream;
     vector<Tuple*>* rel;
};

template<class T>
int gettuplesVMT (Word* args, Word& result,
                    int message, Word& local, Supplier s) {

    gettuplesInfo* li = (gettuplesInfo*) local.addr;
    switch(message){
         case OPEN :{
             if(li){
                delete li;
                local.addr = 0;
             }
             T* n = (T*) args[1].addr;
             MemoryRelObject* rel = getMemRel(n,nl->Second(qp->GetType(s)));
             if(!rel){
               return 0;
             }
             local.addr = new gettuplesInfo(args[0], rel);
             return 0; 

         }
         case REQUEST: 
                result.addr = li?li->next():0;
                return result.addr?YIELD:CANCEL;
         case CLOSE: {
               if(li){
                 delete li;
                 local.addr = 0;
               }
               return 0;
         }
    }    
    return -1;
}


class gettuplesInfoUnwrap{
  public:
    gettuplesInfoUnwrap(Word& _stream, MemoryRelObject* _rel, int _tidpos,
                        ListExpr resTupleType): 
        stream(_stream), rel(_rel->getmmrel()), tidpos(_tidpos){
      stream.open();
      tt = new TupleType(resTupleType);
    }

    ~gettuplesInfoUnwrap(){
        stream.close();
        tt->DeleteIfAllowed();
     }

     Tuple* next(){
        Tuple* inTuple;
        while((inTuple=stream.request())){
           TupleIdentifier* tid = (TupleIdentifier*) 
                                  inTuple->GetAttribute(tidpos);
           if(!tid->IsDefined()){
              inTuple->DeleteIfAllowed();
           } else {
              TupleId id = tid->GetTid();
              if(id<1 || id > rel->size()){
                inTuple->DeleteIfAllowed();
              } else {
                Tuple* relTuple = rel->at(id-1);
                if(!relTuple){
                   inTuple->DeleteIfAllowed();
                } else {
                  Tuple* resTuple = createResTuple(inTuple,relTuple);
                  inTuple->DeleteIfAllowed();
                  return resTuple;
                }
              }
           }
        }
        return 0;
     }

  private:
     Stream<Tuple> stream;
     vector<Tuple*>* rel;
     int tidpos;
     TupleType* tt;

     Tuple* createResTuple(Tuple* inTuple, Tuple* relTuple){
        Tuple* resTuple = new Tuple(tt);
        // copy attributes before tid from intuple to restuple
        for(int i=0; i < tidpos; i++){
           resTuple->CopyAttribute(i,inTuple,i);
        }
        // copy attributes after tidpos from intuple to restuple
        for(int i=tidpos+1;i<inTuple->GetNoAttributes(); i++){
           resTuple->CopyAttribute(i,inTuple,i-1);
        }
        // copy attributes from relTuple to resTuple
        for(int i=0;i<relTuple->GetNoAttributes(); i++){
           resTuple->CopyAttribute(i, relTuple, 
                                   i + inTuple->GetNoAttributes()-1);
        }
        return resTuple;
     }
};


template<class T>
int gettuplesUnwrapVMT (Word* args, Word& result,
                        int message, Word& local, Supplier s) {

    gettuplesInfoUnwrap* li = (gettuplesInfoUnwrap*) local.addr;
    switch(message){
         case OPEN :{
             if(li){
                delete li;
                local.addr = 0;
             }
             T* n = (T*) args[1].addr;
             MemoryRelObject* rel = getMemRel(n);
             if(!rel){
               return 0;
             }
             int tidpos = ((CcInt*)args[2].addr)->GetValue();
             ListExpr tt = nl->Second(GetTupleResultType(s));
             local.addr = new gettuplesInfoUnwrap(args[0], rel,tidpos,tt);
             return 0; 

         }
         case REQUEST: 
                result.addr = li?li->next():0;
                return result.addr?YIELD:CANCEL;
         case CLOSE: {
               if(li){
                 delete li;
                 local.addr = 0;
               }
               return 0;
         }
    }    
    return -1;
}



ValueMapping gettuplesVM[] =  {
   gettuplesVMT<Mem>,
   gettuplesVMT<MPointer>,
   gettuplesUnwrapVMT<Mem>,
   gettuplesUnwrapVMT<MPointer>

};

int gettuplesSelect(ListExpr args){
  int n2 = -1; 
  ListExpr a2 = nl->Second(args);
  if(Mem::checkType(a2)) n2 = 0;
  if(MPointer::checkType(a2)) n2 = 1;
  if(n2<0) return -1;
   
  int n1 = Stream<TupleIdentifier>::checkType(nl->First(args))?0:2;
  return n1+n2;
}

OperatorSpec gettuplesSpec(
  "stream({tid,tuple}) x MREL -> stream(tuple(X)), "
  "MREL represented as mem or mpointer",
  "_ _ gettuples",
  "Retrieves tuples from a main memory relation whose ids are "
  "specified in the incoming stream. If the incoming stream is a tuple stream,"
  "exactly one attribute must be of type tid. Thius attribute is used to "
  "extract the tuples from the main memory relation. The tid-attribute is "
  "removed from the incoming stream and the tuple from the relation is "
  "appended to the remaining tuples",
  "query strassen_Name mdistScan2[mstr] mstrassen gettuples consume"
);

Operator gettuplesOp(
  "gettuples",
  gettuplesSpec.getStr(),
  4,
  gettuplesVM,
  gettuplesSelect,
  gettuplesTM
);


/*
5.7 Operator ~memobject~

~memobject~ gets a name of a main memory object and return a persistent version

5.7.1 Type Mapping Functions of operator ~memobject~ (string -> m:MEMLOADABLE)

*/
ListExpr memobjectTypeMap(ListExpr args) {
    if(nl->ListLength(args)!=1){
        return listutils::typeError("wrong number of arguments");
    }
    ListExpr arg1 = nl->First(args);
    if(MPointer::checkType(arg1)){
      arg1 = nl->Second(arg1);
    }
    if(!Mem::checkType(arg1)){
      return listutils::typeError("Argument is not a memory object");
    }
    ListExpr subtype = nl->Second(arg1);
    if(   !Attribute::checkType(subtype)
       && !Relation::checkType(subtype)){
         return listutils::typeError("only rel and DATA are supported");
    }
    return subtype;
}


/*

5.7.3  The Value Mapping Functions of operator ~memobject~

*/
template<class T>
int memobjectValMap (Word* args, Word& result,
                int message, Word& local, Supplier s) {
        result = qp->ResultStorage(s);
        ListExpr resType = qp->GetType(s);
        T* oN = (T*) args[0].addr;
        if(Attribute::checkType(resType)){  // Attribute
           MemoryAttributeObject* mao = getMemAttribute(oN, resType);
           if(!mao){
              ((Attribute*) result.addr)->SetDefined(false);
              return 0;
           }
           ((Attribute*) result.addr)->CopyFrom(mao->getAttributeObject());
           return 0;
        } else if(Relation::checkType(resType)){ // relation
           GenericRelation* r = (GenericRelation*) result.addr;
            if(r->GetNoTuples() > 0) {
                r->Clear();
            }

           MemoryRelObject* mro = getMemRel(oN, nl->Second(resType));
           if(!mro){
              return 0;
           }
           vector<Tuple*>* relation = mro->getmmrel();
           vector<Tuple*>::iterator it;
           it=relation->begin();

           while( it!=relation->end()){
                Tuple* tup = *it;
                r->AppendTuple(tup);
                it++;
            }
            return 0;
        }
        assert(false); // unsupported type
        return 0;
}

/*

5.7.4 Description of operator ~memobject~

*/
OperatorSpec memobjectSpec(
    " mem(X) | mpointer(mem(X))  ->X  , X in {DATA, rel}",
    "memobject (_)",
    "returns a persistent object created from a main memory object",
    "query memobject (mTrains100)"
);

/*
5.7.5 Value Mapping Array and Selection

*/
ValueMapping memobjectVM[] = {
   memobjectValMap<Mem>,
   memobjectValMap<MPointer>
};

int memobjectSelect(ListExpr args){
   ListExpr a = nl->First(args);
   if(Mem::checkType(a)) return 0;
   if(MPointer::checkType(a)) return 1;
   return -1;
}

/*

5.7.5 Instance of operator ~memobject~

*/

Operator memobjectOp (
    "memobject",
    memobjectSpec.getStr(),
    2,
    memobjectVM,
    memobjectSelect,
    memobjectTypeMap
);

/*

5.8 Operator ~memgetcatalog~

Returns a ~stream(Tuple)~.
Each tuple describes one element of the main memory catalog.


5.8.1 Type Mapping Functions of operator ~memgetcatalog~ (  -> stream(Tuple) )

*/


ListExpr memgetcatalogTypeMap(ListExpr args)
{
    if(nl->ListLength(args)!=0){
        return listutils::typeError("no argument expected");
    }

    string stringlist = "(stream(tuple((TotalMB int)"
        "(UsedMB real)(Name string)"
        "(ObjectType text)(ObjSizeInB string)(ObjSizeInMB real)"
            "(Database string)(Accessible bool)(FlobsLoaded bool)"
            "(NoReferences int ))))";

    ListExpr res =0;
    if(nl->ReadFromString(stringlist, res)){};
    return res;
}


/*

5.8.3  The Value Mapping Functions of operator ~memgetcatalog~

*/

class memgetcatalogInfo{
    public:

        memgetcatalogInfo(ListExpr _resultType){
        
        tt = new TupleType(nl->Second(_resultType));
        memContents = catalog->getMemContent();
        it = memContents->begin();
        };
        ~memgetcatalogInfo(){
           tt->DeleteIfAllowed();
         }

        string long_to_string(unsigned long value) {
            std::ostringstream stream;
            stream << value;
            return stream.str();
        }

        Tuple* next(){
          if(it==memContents->end()) {
            return 0;
          }
          string name = it->first;
          MemoryObject* memobj = it->second;
          string objTyp ="nn";

          ListExpr objectType = catalog->getMMObjectTypeExpr(name);
          objTyp = nl->ToString(objectType);

          Tuple *tup = new Tuple( tt );

          CcInt* totalMB = new CcInt (true, catalog->getMemSizeTotal());
          CcReal* usedMB =
              new CcReal(true, (double)catalog->getUsedMemSize()/1024.0/1024.0);
          CcString* objectName = new CcString(true,name);
          FText* oT = new FText(true,objTyp);
          int64_t objectMem = memobj?memobj->getMemSize():0;
          CcString* memSizeB = new CcString(true, long_to_string(objectMem));
          CcReal* memSizeMB =
              new CcReal(true, (double)objectMem/1024.0/1024.0);
          string db = memobj?memobj->getDatabase():"none";
          CcString* database = new CcString(true,db);
          bool access = memobj 
                  ? (memobj->getDatabase()==getDBname()) || (memobj->hasflob())
                  : true;
          CcBool* accessible = new CcBool(true, access);
          bool flob = memobj?memobj->hasflob():false;
          CcBool* flobs = new CcBool(true, flob);

          int norefs = memobj?memobj->getNoReferences():0;
         

          tup->PutAttribute(0,totalMB);
          tup->PutAttribute(1,usedMB);
          tup->PutAttribute(2,objectName);
          tup->PutAttribute(3,oT);
          tup->PutAttribute(4,memSizeB);
          tup->PutAttribute(5,memSizeMB);
          tup->PutAttribute(6,database);
          tup->PutAttribute(7,accessible);
          tup->PutAttribute(8,flobs);
          tup->PutAttribute(9,new CcInt(norefs));
          it++;
          return tup;
    }
 private:

       map<string, MemoryObject*>* memContents;
       map<string, MemoryObject*>::iterator it;
       TupleType* tt;

};


int memgetcatalogValMap (Word* args, Word& result,
            int message, Word& local, Supplier s) {


   memgetcatalogInfo* li = (memgetcatalogInfo*) local.addr;

   switch (message)
   {
        case OPEN: {
             if(li){
             delete li;
             local.addr=0;

          }
        ListExpr resultType;
        resultType = GetTupleResultType( s );
        local.addr= new memgetcatalogInfo(resultType);
        return 0;
        }

        case REQUEST:
            result.addr=(li?li->next():0);
            return result.addr?YIELD:CANCEL;


        case CLOSE:
            if(li)
            {
            delete li;
            local.addr = 0;
            }
            return 0;
   }

    return 0;

}



/*

5.8.4 Description of operator ~memgetcatalog~

*/

OperatorSpec memgetcatalogSpec(
    " -> stream(Tuple)",
    "memgetcatalog()",
    "returns a stream(Tuple) with information about  main memory objects",
    "query memgetcatalog()"
);

/*

5.8.5 Instance of operator ~memgetcatalog~

*/

Operator memgetcatalogOp (
    "memgetcatalog",
    memgetcatalogSpec.getStr(),
    memgetcatalogValMap,
    Operator::SimpleSelect,
    memgetcatalogTypeMap
);



/*
5.10 Operator ~mcreateRtree~
creates a an mmRTree over a given main memory relation
or a stream of tuples

5.10.1 TypeMapping

   mpointer(mem(REL(...))) x IDENT -> mpointer(rtree(dim)), REL in rel,orel
   stream(tuple(... TID)) x IDENT -> mpointer(rtree(dim))

*/

ListExpr mcreatertreeTM(ListExpr args){

   if(!nl->HasLength(args,2)){
     return listutils::typeError("two arguments expected");
   } 
   ListExpr arg2 = nl->Second(args);
   if(nl->AtomType(arg2)!=SymbolType){
     return listutils::typeError("second argument is not a valid "
                                 "attribute name");
   }

   ListExpr arg1 = nl->First(args);
   ListExpr attrList= nl->TheEmptyList();
   int tidIndex = -1;
   if(Stream<Tuple>::checkType(arg1)){
      attrList = nl->Second(nl->Second(arg1));
      ListExpr tid = listutils::basicSymbol<TupleIdentifier>();
      string tidn;
      tidIndex = listutils::findType(attrList, tid, tidn);
      if(!tidIndex){
        return listutils::typeError("no tid found in tuple of stream");
      }
      tidIndex--;
   } else {
      if(!MPointer::checkType(arg1)){
         return listutils::typeError("mcreatertree expects an mpointer to "
                                     "a relation or a stream of tuples");
      }
      arg1 = nl->Second(arg1); // remove mpointer
      if(!Mem::checkType(arg1)){
        return listutils::typeError("internal error");
      }
      arg1 = nl->Second(arg1); // remove mem
      if(!Relation::checkType(arg1)){
         return listutils::typeError("mcreatertree expects an mpointer to "
                                     "a relation or a stream of tuples");
          
      }
      attrList = nl->Second(nl->Second(arg1));
   }
    
   ListExpr attrType;
   string aname = nl->SymbolValue(arg2);
   int attrIndex = listutils::findAttribute(attrList, aname, attrType);
   if(!attrIndex){
       return listutils::typeError("attribute " + aname+ " not found");
   }
   attrIndex--;
   int dim = rtreehelper::getDimension(attrType);
   if(dim < 0){
     return listutils::typeError("cannot determine dimension of attribute " 
                                 + aname);
   }
   ListExpr resType = MPointer::wrapType(Mem::wrapType(
                                   rtreehelper::getType(dim)));
   ListExpr appendList;
   if(tidIndex < 0){
     appendList = nl->OneElemList(nl->IntAtom(attrIndex));
   } else {
     appendList = nl->TwoElemList( nl->IntAtom(attrIndex),
                                   nl->IntAtom(tidIndex));
   }
   return nl->ThreeElemList( nl->SymbolAtom(Symbols::APPEND()),
                             appendList,
                             resType);

}


template<int dim>
int mcreateRtreeVMMP(Word* args, Word& result,
                int message, Word& local, Supplier s) {

   result = qp->ResultStorage(s);
   MPointer* res = (MPointer*) result.addr;
   MPointer* arg = (MPointer*) args[0].addr;
   MemoryRelObject* mrel = (MemoryRelObject*) arg->GetValue();
   if(!mrel){
     return 0;
   }

   std::vector<Tuple*>* v = mrel->getmmrel();
   if(!v){
     return 0;
   }

   int attrPos = ((CcInt*) args[2].addr)->GetValue();

   mmrtree::RtreeT<dim, size_t>* tree = new mmrtree::RtreeT<dim, size_t>(4,8);
   for(size_t i = 0;i<v->size();i++){
      Tuple* t = v->at(i);
      if(t){
         Rectangle<dim> r  =  ((StandardSpatialAttribute<dim>*)
                                t->GetAttribute(attrPos))->BoundingBox();
         if(r.IsDefined()){
            tree->insert(r,i);
         }
      }
   }
   MemoryRtreeObject<dim>* mrt = new MemoryRtreeObject<dim>(tree, 
                                      tree->usedMem(),
                                      nl->ToString(nl->Second(qp->GetType(s))),
                                      getDBname());
   res->setPointer(mrt);
   mrt->deleteIfAllowed();
   return 0;
}


template<int dim>
int mcreateRtreeVMStream(Word* args, Word& result,
                int message, Word& local, Supplier s) {

   result = qp->ResultStorage(s);
   MPointer* res = (MPointer*) result.addr;

   int attrPos = ((CcInt*) args[2].addr)->GetValue();
   int tidPos = ((CcInt*) args[3].addr)->GetValue();
   Stream<Tuple> stream(args[0]);
   stream.open();
   mmrtree::RtreeT<dim, size_t>* tree = new mmrtree::RtreeT<dim, size_t>(4,8);
   Tuple* t;
   while( (t = stream.request())){
      Rectangle<dim> r  =  ((StandardSpatialAttribute<dim>*)
                                 t->GetAttribute(attrPos))->BoundingBox();
      TupleIdentifier* tid = (TupleIdentifier*) t->GetAttribute(tidPos);
      if(r.IsDefined() && tid->IsDefined()){
        tree->insert(r,tid->GetTid());
      }
      t->DeleteIfAllowed();
   }
   stream.close();
   MemoryRtreeObject<dim>* mrt = new MemoryRtreeObject<dim>(tree, 
                                      tree->usedMem(),
                                      nl->ToString(nl->Second(qp->GetType(s))),
                                      getDBname());
   res->setPointer(mrt);
   mrt->deleteIfAllowed();
   return 0;
}


ValueMapping mcreatertreeVM[] = {
   mcreateRtreeVMStream<2>,
   mcreateRtreeVMStream<3>,
   mcreateRtreeVMStream<4>,
   mcreateRtreeVMStream<8>,
   mcreateRtreeVMMP<2>,
   mcreateRtreeVMMP<3>,
   mcreateRtreeVMMP<4>,
   mcreateRtreeVMMP<8>
};

int mcreatertreeSelect(ListExpr args){
   int o1 = 0;
   ListExpr attrList;
   if(MPointer::checkType(nl->First(args))){
     o1 = 4;
     attrList = nl->Second(nl->Second(nl->Second(nl->Second(nl->First(args)))));
   } else {
     o1 = 0;
     attrList = nl->Second(nl->Second(nl->First(args)));
   }
   string attrName = nl->SymbolValue(nl->Second(args));
   ListExpr attrType;
   listutils::findAttribute(attrList, attrName, attrType);
   int dim = rtreehelper::getDimension(attrType);
   int o2 = 0;
   switch(dim){
     case 2 : o2 = 0; break;
     case 3 : o2 = 1; break;
     case 4 : o2 = 2; break;
     case 8 : o2 = 3; break;
     default: assert(false);
   }
   return o1 + o2;
}

OperatorSpec mcreatertreeSpec(
  " { mpointer(mem(rel(...))), stream(tuple(...)) } x IDENT "
  "-> mpointer(rtree X)",
  " _ createmmrtree[_]",
  " Creates a main memory based rtree over an attribute of a  relation "
  "represented as either main memory relation or as a tuple stream.",
  "query mstrassen createmmrtree[GeoData]"
);


Operator mcreatertreeOp(
  "mcreatertree",
  mcreatertreeSpec.getStr(),
  8,
  mcreatertreeVM,
  mcreatertreeSelect,
  mcreatertreeTM
);


/*
5.11 Operator ~memsize~

returns the currently set main memory size

5.11.1 Type Mapping Functions of operator ~memsize~ (->int)

*/

ListExpr memsizeTypeMap(ListExpr args)
{

  if(nl->ListLength(args)!=0){
     return listutils::typeError("no argument expected");
  }

  return listutils::basicSymbol<CcInt>();
}


/*

5.11.3  The Value Mapping Functions of operator ~memsize~

*/

int memsizeValMap (Word* args, Word& result,
                    int message, Word& local, Supplier s) {

    int res = catalog->getMemSizeTotal();

    result  = qp->ResultStorage(s);
    CcInt* b = static_cast<CcInt*>(result.addr);
    b->Set(true, res);


    return 0;
}

/*

5.11.4 Description of operator ~memsize~

*/

OperatorSpec memsizeSpec(
    "-> int",
    "memsize()",
    "returns the currently set main memory size ",
    "query memsize()"
);

/*

5.11.5 Instance of operator ~memsize~

*/

Operator memsizeOp (
    "memsize",
    memsizeSpec.getStr(),
    memsizeValMap,
    Operator::SimpleSelect,
    memsizeTypeMap
);



/*
5.12 Operator ~memclear~

deletes all main memory objects

5.12.1 Type Mapping Functions of operator ~memclear~ (-> bool)

*/

ListExpr memclearTypeMap(ListExpr args)
{

  if(nl->ListLength(args)!=0){
     return listutils::typeError("no argument expected");
  }

  return listutils::basicSymbol<CcBool>();
}


/*

5.12.3  The Value Mapping Functions of operator ~memclear~

*/

int memclearValMap (Word* args, Word& result,
                    int message, Word& local, Supplier s) {

    bool res = false;
    catalog->clear();
    res = true;
    result  = qp->ResultStorage(s);
    CcBool* b = static_cast<CcBool*>(result.addr);
    b->Set(true, res);
    return 0;
}

/*

5.12.4 Description of operator ~memclear~

*/

OperatorSpec memclearSpec(
    "-> bool",
    "memclear()",
    "deletes all main memory objects",
    "query memclear()"
);

/*

5.12.5 Instance of operator ~memclear~

*/

Operator memclearOp (
    "memclear",
    memclearSpec.getStr(),
    memclearValMap,
    Operator::SimpleSelect,
    memclearTypeMap
);


/*
5.13 Operator ~meminsert~

inserts the tuple of a stream into a existing main memory relation

5.13.1 Type Mapping Functions of operator ~meminsert~
    (stream(tuple(x)) x string -> stream(tuple(x))
    the second argument identifies the main memory relation

*/

ListExpr meminsertTypeMap(ListExpr args)
{
    if(nl->ListLength(args)!=2){
        return listutils::typeError("two arguments expected");
    }

    ListExpr stream = nl->First(args);

    if (!Stream<Tuple>::checkType(stream)) {
        return listutils::typeError
            ("stream(Tuple) as first argument expected");
    }
    
    ListExpr argSec = nl->Second(args);
    if(MPointer::checkType(argSec)){
      argSec = nl->Second(argSec);
    } 
    if(!Mem::checkType(argSec)){
       return listutils::typeError("second arg is not a memory object");
    }
    ListExpr subtype = nl->Second(argSec);
    if(!Relation::checkType(subtype)){
       return listutils::typeError("mem relation expected ");
    }
    if(!nl->Equal(nl->Second(stream), nl->Second(subtype))){
       return listutils::typeError("stream type and mem relation type differ");
    }
    return stream;

}


class meminsertInfo{
  public:
     meminsertInfo( Word& w, vector<Tuple*>* _relation, bool _flob):
          stream(w),relation(_relation), flob(_flob){
        stream.open();
     }

    ~meminsertInfo(){
       stream.close();
     }

     Tuple* next(){
       Tuple* res = stream.request();
       if(!res){
         return 0;
       }
       if(flob){
         res->bringToMemory();
       }
       res->IncReference();
       relation->push_back(res);
       res->SetTupleId(relation->size());
       return res;
     }

  private:
     Stream<Tuple> stream;
     vector<Tuple*>* relation;
     bool flob;

};


/*

5.13.3  The Value Mapping Functions of operator ~meminsert~

*/
template<class T, bool tc>
int meminsertValMap (Word* args, Word& result,
                    int message, Word& local, Supplier s) {

    meminsertInfo* li = (meminsertInfo*) local.addr;

    switch (message)
    {
        case OPEN: {
            if(li){
               delete li;
               local.addr=0;
            }
            T* oN = (T*) args[1].addr;
            MemoryRelObject* mro = 0;
            if(tc){
               ListExpr tt = nl->Second(qp->GetType(qp->GetSon(s,0)));
               mro = getMemRel(oN,tt);
            } else {
               mro = getMemRel(oN);
            }
            if(!mro){
              return 0;
            }
            local.addr = new meminsertInfo(args[0], mro->getmmrel(), 
                                         mro->hasflob()); 
            return 0;
        }

        case REQUEST:
            result.addr=(li?li->next():0);
            return result.addr?YIELD:CANCEL;

        case CLOSE:
            if(li) {
              delete li;
              local.addr = 0;
            }
            return 0;
   }

   return -1;
}

ValueMapping meminsertVM[] = {
   meminsertValMap<Mem,true>,
   meminsertValMap<MPointer,false>
};

int meminsertSelect(ListExpr args){
   ListExpr a = nl->Second(args);
   if(Mem::checkType(a)) return 0;
   if(MPointer::checkType(a)) return 1;
   return -1;
}



/*

5.13.4 Description of operator ~meminsert~

*/

OperatorSpec meminsertSpec(
    "stream(Tuple) x {string, mem(rel), mpointer(rel)}  -> stream(Tuple)",
    "meminsert(_,_)",
    "inserts the tuples of a stream into an "
    "existing main memory relation",
    "query meminsert (ten feed head[5],'ten') count"
);

/*

5.13.5 Instance of operator ~meminsert~

*/

Operator meminsertOp (
    "meminsert",
    meminsertSpec.getStr(),
    3,
    meminsertVM,
    meminsertSelect,
    meminsertTypeMap
);

/*
5.14 Operator ~mwindowintersects~
        Uses the given MemoryRtreeObject (as first argument)to find all tuples
        in the given MemoryRelObject (as second argument)
        with intersects the third argument value's bounding box

5.14.1 Type Mapping Functions of operator ~mwindowintersects~
    string x string x T -> stream(Tuple)
    where T in {rect<d>} U SPATIAL2D U SPATIAL3D U SPATIAL4D U SPATIAL8D

*/

ListExpr mwindowintersectsTypeMap(ListExpr args)
{
  if(nl->ListLength(args)!=3){
    return listutils::typeError("three arguments expected");
  }

  /* Split argument in three parts */
  ListExpr a1 = nl->First(args);
  ListExpr a2 = nl->Second(args);
  ListExpr a3 = nl->Third(args);

  string err="MRTREE x MREL x rectX expected";

  if(MPointer::checkType(a1)){
    a1 = nl->Second(a1);
  }
  if(!Mem::checkType(a1)){
    return listutils::typeError("first arg is not a memory object");
  }

  if(MPointer::checkType(a2)){
    a2 = nl->Second(a2);
  }
  if(!Mem::checkType(a2)){
    return listutils::typeError("second arg is not a memory object");
  }
  
  
  ListExpr rtreetype = nl->Second(a1); // remove leading mem
  
  if(!rtreehelper::checkType(rtreetype)){
    return listutils::typeError(err + " (first arg is not a mem rtree)");
  }

  int rtreedim = nl->IntValue(nl->Second(rtreetype));

  ListExpr relType = nl->Second(a2);
  if(!Relation::checkType(relType)){
    return listutils::typeError(err + " (second argument is not "
                                       "a memory relation)");
  }
 
  int dim2 = rtreehelper::getDimension(a3);

  if(dim2 < 0){
    return listutils::typeError("third argument not in kind SPATIALxD");
  }

  if(rtreedim != dim2){
     return listutils::typeError("dimensions of rtree and "
                                 "search object differ");
  }
  
  ListExpr res = nl->TwoElemList( 
                 listutils::basicSymbol<Stream<Tuple> >(),
                 nl->Second(relType)); 
  return res;
}



template<int dim>
class mwiInfo{
  public:
     mwiInfo( MemoryRtreeObject<dim>* _tree,
              MemoryRelObject* _rel,
              Rectangle<dim>& _box) {
        mmrtree::RtreeT<dim, size_t>* t = _tree->getrtree();
        rel = _rel->getmmrel();
        it = t->find(_box);
     }

    ~mwiInfo(){
        delete it;
    }

     Tuple* next(){
        Tuple* res = 0;
        while(!res){
          const size_t* indexp = it->next();
          if(!indexp){
             return 0;
          }
          if(*indexp < rel->size()){ // ignores index outside vector
             res = (*rel)[*indexp];
             if(res){ // may be deleted
                res->IncReference();
             }
          }
        }
        return res;
     }

  private:
     vector<Tuple*>* rel;
     typename mmrtree::RtreeT<dim, size_t>::iterator* it;
};

/*

5.14.3  The Value Mapping Functions of operator ~mwindowintersects~

*/
template <class T, class R, int dim>
int mwindowintersectsValMapT (Word* args, Word& result,
                    int message, Word& local, Supplier s) {

   mwiInfo<dim>* li = (mwiInfo<dim>*) local.addr;

   switch (message)
   {
        case OPEN: {
             if(li){
               delete li;
               local.addr=0;
             }
             T* treeN = (T*) args[0].addr;
             R* relN = (R*) args[1].addr;
             typedef StandardSpatialAttribute<dim> W;
             typedef  MemoryRtreeObject<dim> treetype;
             W* window = (W*) args[2].addr;
             ListExpr tupleT = nl->Second(qp->GetType(s));
             MemoryRelObject* mro = getMemRel(relN,tupleT);
             if(!mro){
               return 0;
             }
             treetype* tree = getRtree<T,dim>(treeN);
             if(!tree){
               return 0;
             }
             if(!window->IsDefined()){
                 return 0;
             }
             Rectangle<dim> box = window->BoundingBox();
             if(!box.IsDefined()){ // empty spatial object
                return 0;
             }
             local.addr = new mwiInfo<dim>(tree, mro, box);
             return 0;
        }

        case REQUEST:
            result.addr=(li?li->next():0);
            return result.addr?YIELD:CANCEL;


        case CLOSE:
            if(li) {
              delete li;
              local.addr = 0;
            }
            return 0;
   }

    return -1;
}

ValueMapping mwindowintersectsValMap[] =
{
    mwindowintersectsValMapT<Mem, Mem,2>,
    mwindowintersectsValMapT<Mem, Mem,3>,
    mwindowintersectsValMapT<Mem, Mem,4>,
    mwindowintersectsValMapT<Mem, Mem,8>,
    
    mwindowintersectsValMapT<Mem, MPointer,2>,
    mwindowintersectsValMapT<Mem, MPointer,3>,
    mwindowintersectsValMapT<Mem, MPointer,4>,
    mwindowintersectsValMapT<Mem, MPointer,8>,
    
    mwindowintersectsValMapT<MPointer, Mem,2>,
    mwindowintersectsValMapT<MPointer, Mem,3>,
    mwindowintersectsValMapT<MPointer, Mem,4>,
    mwindowintersectsValMapT<MPointer, Mem,8>,
    
    mwindowintersectsValMapT<MPointer, MPointer,2>,
    mwindowintersectsValMapT<MPointer, MPointer,3>,
    mwindowintersectsValMapT<MPointer, MPointer,4>,
    mwindowintersectsValMapT<MPointer, MPointer,8>,
};

/*
1.3 Selection method for value mapping array ~mcreateRtree~

*/
 int mwindowintersectsSelect(ListExpr args)
 {
   int dim = rtreehelper::getDimension(nl->Third(args));
   int n3 = -1;
   switch (dim){
     case 2 : n3 = 0;break;
     case 3 : n3 = 1;break;
     case 4 : n3 = 2;break;
     case 8 : n3 = 3;break;
     default : return -1;
   }

   int n1 = -1;
   ListExpr a1 = nl->First(args);
   if(Mem::checkType(a1)) n1 = 0;
   if(MPointer::checkType(a1)) n1 = 1;

   int n2 = -1;
   ListExpr a2 = nl->Second(args);
   if(Mem::checkType(a2)) n2 = 0;
   if(MPointer::checkType(a2)) n2 = 1;

   int res =  8*n1 + 4*n2 + n3;

   return res;
 }


/*

5.14.4 Description of operator ~mwindowintersects~

*/

OperatorSpec mwindowintersectsSpec(
   "MRTREE x MREL x rectX -> stream(tuple(X)) , where MRTREE, MREL"
   " are represented as mem or mpointer",
   "_ _ mwindowintersects[_]",
   "Uses the given rtree to find all tuples"
   " in the given relation which intersects the "
   " argument value's bounding box.",
   "query mstrassen_GeoData mstrassen mwindowintersects[thecenter] count"
);

/*

5.14.5 Instance of operator ~mwindowintersects~

*/

Operator mwindowintersectsOp (
    "mwindowintersects",
    mwindowintersectsSpec.getStr(),
    16,
    mwindowintersectsValMap,
    mwindowintersectsSelect,
    mwindowintersectsTypeMap
);


/*
5.16 Operator mwindowintersectsS 

*/
template<bool wrap>
ListExpr mwindowintersectsSTM(ListExpr args){

  string err = " {string, memory(rtree <dim> )} x SPATIAL<dim>D expected";
  if(!nl->HasLength(args,2)){
     return listutils::typeError(err + " (wrong number of args)");
  }
  ListExpr a2t = nl->Second(args);
  int dim = rtreehelper::getDimension(a2t);
  if(dim < 0){
     return listutils::typeError(err + " (second arg is not in "
                                 "kind SPATIALxD)");
  }
  ListExpr a1 = nl->First(args);
  if(MPointer::checkType(a1)){
     a1 = nl->Second(a1);
  }
  if(!Mem::checkType(a1)){
    return listutils::typeError("first arg is not a memory object");
  }
  ListExpr tree = nl->Second(a1);
  if(!rtreehelper::checkType(tree)){
      return listutils::typeError(err + " (first arg is not an r-tree");
  }
  int tdim = nl->IntValue(nl->Second(tree));
  if(dim!=tdim){
     return listutils::typeError(err + "tree and query object have "
                                       "different dimensions");
  }
  if(!wrap){
     return nl->TwoElemList(
                    listutils::basicSymbol<Stream<TupleIdentifier> >(),
                    listutils::basicSymbol<TupleIdentifier> ());
  } else {
    ListExpr attrList = nl->OneElemList(
                            nl->TwoElemList( 
                              nl->SymbolAtom("TID"),
                              listutils::basicSymbol<TupleIdentifier>()));

    return nl->TwoElemList( listutils::basicSymbol<Stream<Tuple> >(),
                            nl->TwoElemList( listutils::basicSymbol<Tuple>(),
                            attrList));
  }
}

template<int dim>
struct mwindowintersectsSInfo{
    typename mmrtree::RtreeT<dim,size_t>::iterator* it;
    TupleType* tt;
};


template <int dim, class T, bool wrap>
int mwindowintersectsSVMT (Word* args, Word& result,
                    int message, Word& local, Supplier s) {


   mwindowintersectsSInfo<dim>* li = (mwindowintersectsSInfo<dim>*)local.addr;

   switch(message){
      case OPEN:{
             if(li){
               delete li->it;
               if(li->tt){
                  li->tt->DeleteIfAllowed();
               }
               delete li;
               local.addr = 0;
             }
             T* name = (T*) args[0].addr;
             MemoryRtreeObject<dim>* tree = getRtree<T,dim>(name);
             if(!tree){
                return 0;
             }
             typedef StandardSpatialAttribute<dim> boxtype;
             boxtype* o = (boxtype*) args[1].addr;
             if(!o->IsDefined()){
               return 0;
             }
             Rectangle<dim> box = o->BoundingBox();
             if(!box.IsDefined()){ // empty spatial object
               return 0;
             }
             li = new mwindowintersectsSInfo<dim>();
             li->it = tree->getrtree()->find(box);
             if(wrap){
               li->tt = new TupleType( nl->Second(GetTupleResultType(s)));
             } else {
               li->tt = 0;
             }
             local.addr = li;
             return 0;
      }
      case REQUEST : {
               if(!li){
                  return CANCEL;
               }
               const size_t* index = li->it->next();
               if(!index){
                 result.addr=0;
               } else { // iterator exhausted
                 TupleIdentifier* tid = new TupleIdentifier(true,*index);
                 if(!wrap){
                    result.addr = tid;
                 } else {
                   Tuple* tuple = new Tuple(li->tt);
                   tuple->PutAttribute(0,tid);
                   result.addr = tuple;
                 }
               }
               return result.addr?YIELD:CANCEL;
        }
       case CLOSE:
                 if(li){
                    delete li->it;
                    if(li->tt){
                       li->tt->DeleteIfAllowed();
                    }
                    delete li;
                    local.addr = 0;
                 }
                 return 0;
   }
   return -1;
}

ValueMapping mwindowintersectsSVM[] = {
   mwindowintersectsSVMT<2, Mem, true>,
   mwindowintersectsSVMT<3, Mem, true>,
   mwindowintersectsSVMT<4, Mem, true>,
   mwindowintersectsSVMT<8, Mem, true>,
   mwindowintersectsSVMT<2, MPointer, true>,
   mwindowintersectsSVMT<3, MPointer, true>,
   mwindowintersectsSVMT<4, MPointer, true>,
   mwindowintersectsSVMT<8, MPointer, true>
};

int mwindowintersectsSSelect(ListExpr args){
   int n1 = -1;
   ListExpr a1 = nl->First(args);

   if(Mem::checkType(a1)) n1=0;
   if(MPointer::checkType(a1)) n1=4;


   int dim = rtreehelper::getDimension(nl->Second(args));
   int n2;
   switch(dim){
      case 2 : n2 = 0; break;
      case 3 : n2 = 1; break;
      case 4 : n2 = 2; break;
      case 8 : n2 = 3; break;
      default : assert(false);
   }
   return n1 + n2;
}


OperatorSpec mwindowintersectsSSpec(
  "MRTREE x SPATIAL<dim>D -> stream(tuple((TID tid))), MRTREE "
  "represented as mem, or mpointer",
  " _ mwindowintersectsS[_]",
  "Returns the tuple ids belonging to rectangles intersecting the "
  "bounding box of the second argument wraped in a tuple. ",
  "query mstrassen_GeoData mwindowintersectsS[ thecenter ] count" 
);

Operator mwindowintersectsSOp(
   "mwindowintersectsS",
   mwindowintersectsSSpec.getStr(),
   8,
   mwindowintersectsSVM,
   mwindowintersectsSSelect,
   mwindowintersectsSTM<true>
);



/*

5.15 Operator ~mconsume~

~mconsume~ Collects objects from a stream in a ~MemoryRelObject~

5.4.1 Type Mapping Functions of operator ~mconsume~
        (stream(Tuple) -> memoryRelObject)

*/
ListExpr mconsumeTypeMap(ListExpr args)
{
    if(nl->ListLength(args)!=1){
        return listutils::typeError("(wrong number of arguments)");
    }
    if (!Stream<Tuple>::checkType(nl->First(args))) {
        return listutils::typeError ("stream(tuple) expected!");
    }
    ListExpr ttype = nl->Second(nl->First(args));
    ListExpr relType = nl->TwoElemList( 
                           nl->SymbolAtom(Relation::BasicType()), 
                           ttype);
    return MPointer::wrapType(Mem::wrapType(relType));
}


/*

5.15.3  The Value Mapping Functions of operator ~mconsume~

*/
template<bool flob>
int mconsumeValMap (Word* args, Word& result,
                int message, Word& local, Supplier s){
    result  = qp->ResultStorage(s);
    MPointer* mp = (MPointer*)result.addr;
    ListExpr tupleType = nl->Second(nl->Second(qp->GetType(s)));
    MemoryRelObject* mrel = new MemoryRelObject();
    mrel->tupleStreamToRel(args[0], tupleType, getDBname(),flob);
    mp->setPointer(mrel); 
    mrel->deleteIfAllowed();
    return 0;
}


/*

5.15.4 Description of operator ~mconsume~

*/

OperatorSpec mconsumeSpec(
    "stream(Tuple) -> mpointer(mem(rel(TUPLE)))",
    "_ mconsume",
    "collects the objects from a stream(tuple) into a memory relation",
    "query ten feed mconsume"
);

/*

5.15.5 Instance of operator ~mconsume~

*/

Operator mconsumeOp (
    "mconsume",
    mconsumeSpec.getStr(),
    mconsumeValMap<false>,
    Operator::SimpleSelect,
    mconsumeTypeMap
);

/*

5.15.4 Description of operator ~mconsumeflob~

*/

OperatorSpec mconsumeflobSpec(
    "stream(Tuple) -> mpointer(mem(rel(TUPLE)))",
    "_ mconsumeflob",
    "collects the objects from a stream(tuple) including flobs "
    "into a memory relation",
    "query ten feed mconsumeflob"
);

/*

5.15.5 Instance of operator ~mconsume~

*/

Operator mconsumeflobOp (
    "mconsumeflob",
    mconsumeflobSpec.getStr(),
    mconsumeValMap<true>,
    Operator::SimpleSelect,
    mconsumeTypeMap
);





/*
5.16 Operator ~mcreateAVLtree~
creates a an AVLTree over a given main memory relation

5.16.1 Type Mapping Functions of operator ~mcreateAVLtree~

   the first parameter identifies the main memory relation, the
   second parameter identifies the attribute

   another variant accpects a stream of tuple and two 
   attribute names. The first one identifies the attribute to
   index, the second one a TID attribute.

*/

ListExpr mcreateAVLtreeTypeMap(ListExpr args){

    if(!nl->HasLength(args,2) && !nl->HasLength(args,3)){
     return listutils::typeError("two arguments expected");
    }
    bool isStream = nl->HasLength(args,3);

    if(nl->AtomType(nl->Second(args))!=SymbolType){
      return listutils::typeError("second arg does not represent a valid "
                                  "attribute name");
    }    
    string err = "expected stream(tuple) x IDENT x INDENT or MREL x IDENT";

    ListExpr tupleType = nl->TheEmptyList();
    ListExpr attrList = nl->TheEmptyList();
    ListExpr first = nl->First(args);
    int tidIndex = -1;
    if(isStream){
       if(!Stream<Tuple>::checkType(first)){
          return listutils::typeError(err);
       }
       ListExpr third = nl->Third(args);
       if(nl->AtomType(third) != SymbolType){
         return listutils::typeError("third argument is not a valid "
                                     "attribute name");
       } 
       string tidname = nl->SymbolValue(third);
       ListExpr tidType;
       tupleType = nl->Second(first);
       attrList = nl->Second(tupleType);
       tidIndex = listutils::findAttribute(attrList, tidname, tidType);
       if(!tidIndex){
         return listutils::typeError("attribute " + tidname 
                                   + " is not a member of tuple");
       } 
       if(!TupleIdentifier::checkType(tidType)){
         return listutils::typeError("attribute " + tidname 
                                     + " not of type tid");
       }
    } else {
       if(MPointer::checkType(first)){
          first = nl->Second(first);
       }
       if(!Mem::checkType(first)){
         return listutils::typeError(err);
       }
       ListExpr subType = nl->Second(first);
       if(!Relation::checkType(subType)){
          return listutils::typeError("first arg is not a MREL");
       }
       tupleType = nl->Second(subType);
       attrList = nl->Second(tupleType);
    }

    string attrName = nl->SymbolValue(nl->Second(args));
    ListExpr attrType;
    int attrIndex = listutils::findAttribute(attrList,attrName,attrType);

    if(!attrIndex){
      return listutils::typeError(attrName + " is not a member of the tuple");
    }

    ListExpr resType = MPointer::wrapType(
                          Mem::wrapType(
                             MemoryAVLObject::wrapType(attrType)));
 
    ListExpr appendList;
    if(isStream){
       appendList = nl->TwoElemList( nl->IntAtom(attrIndex-1),
                                     nl->IntAtom(tidIndex-1));
    } else {
       appendList = nl->OneElemList( nl->IntAtom(attrIndex-1));
    }
   

    return nl->ThreeElemList(
                nl->SymbolAtom(Symbols::APPEND()),
                appendList,
                resType);
}

/*

5.16.3  The Value Mapping Functions of operator ~mcreateAVLtree~

*/
MemoryAVLObject* createAVLTree(MemoryRelObject* mmrel, int attrPos,
                               ListExpr type){
   bool flob = mmrel->hasflob();
   vector<Tuple*>* relVec = mmrel->getmmrel();
   vector<Tuple*>::iterator it;
   it=relVec->begin();
   unsigned int i=1; // start tuple counting with 1

   memAVLtree* tree = new memAVLtree();
   Attribute* attr;
   AttrIdPair aPair;

   size_t usedMainMemory = 0;
   unsigned long availableMemSize = catalog->getAvailableMemSize();

   while ( it!=relVec->end()){
       Tuple* tup = *it;
       if(tup){
         attr=tup->GetAttribute(attrPos);
         aPair = AttrIdPair(attr,i);
         // size for a pair is 16 bytes, plus an additional pointer 8 bytes
         size_t entrySize = 24;
         if (entrySize<availableMemSize){
             tree->insert(aPair);
             usedMainMemory += (entrySize);
             availableMemSize -= (entrySize);
             i++;
         } else {
            cout<<"there is not enough main memory available"
            " to create an AVLTree"<<endl;
            delete tree;
            return 0;
         }
       }
       it++;
    }
    MemoryAVLObject* avlObject =
        new MemoryAVLObject(tree, usedMainMemory,
            nl->ToString(type),flob, getDBname());
    return avlObject;
}



int mcreateAVLtreeVMMem (Word* args, Word& result,
                int message, Word& local, Supplier s) {

   result = qp->ResultStorage(s);
   MPointer* res = (MPointer*) result.addr;

   // the main memory relation
   Mem* roN = (Mem*) args[0].addr;
   if(!roN->IsDefined()){
        res->setPointer(0);
        return 0;
   }
   string relObjectName = roN->GetValue();

   int attrPos = ((CcInt*) args[2].addr)->GetValue();

   ListExpr memObjectType = catalog->getMMObjectTypeExpr(relObjectName);

   ListExpr argType = qp->GetType(qp->GetSon(s,0));
  
   if(!nl->Equal(memObjectType, argType)){
      cerr << "type stored in catalog and type of mem object differ" << endl;
      cerr << "type of memobject " << argType << endl;
      cerr << "type in catalog " << memObjectType << endl;
      res->setPointer(0);
      return 0;
   }

   MemoryRelObject* mmrel =
        (MemoryRelObject*)catalog->getMMObject(relObjectName);
   if(!mmrel){
     res->setPointer(0);
     return 0;
   }
   MemoryAVLObject* obj = createAVLTree(mmrel, attrPos, 
                                       nl->Second(qp->GetType(s))); 
   res->setPointer(obj);
   obj->deleteIfAllowed();
   return 0;

} //end mcreateAVLtreeValMap

int mcreateAVLtreeVMMP (Word* args, Word& result,
                int message, Word& local, Supplier s) {

  MPointer* arg1 = (MPointer*)(args[0].addr);
  result = qp->ResultStorage(s);
  MPointer* res = (MPointer*) result.addr;
  MemoryRelObject* mmrel = (MemoryRelObject*) arg1->GetValue();
  ListExpr attrPos = ((CcInt*)args[2].addr)->GetValue();
  if(!mmrel){
    res->setPointer(0);
    return 0;
  }
  MemoryAVLObject* obj = createAVLTree(mmrel, attrPos, 
                                       nl->Second(qp->GetType(s))); 
  res->setPointer(obj);
  obj->deleteIfAllowed();
  return 0;
}
int mcreateAVLtreeVMStream (Word* args, Word& result,
                int message, Word& local, Supplier s) {

    result = qp->ResultStorage(s);
    MPointer* res = (MPointer*) result.addr;

    int keyIndex = ((CcInt*) args[3].addr)->GetValue();
    int tidIndex = ((CcInt*) args[4].addr)->GetValue();

    Stream<Tuple> stream(args[0]);
    Tuple* tuple;
    memAVLtree* tree = new memAVLtree();

    stream.open();
    size_t availableMem = catalog->getAvailableMemSize();
    size_t usedMem = 0;

    while((tuple=stream.request()) && (usedMem < availableMem)){
      TupleIdentifier* tid = (TupleIdentifier*) tuple->GetAttribute(tidIndex);
      if(tid->IsDefined()){
          Attribute* key = tuple->GetAttribute(keyIndex);
          key->bringToMemory();
          AttrIdPair p(key, tid->GetTid());
          usedMem += key->GetMemSize() + sizeof(size_t);
          tree->insert(p); 
      }
      tuple->DeleteIfAllowed();
    }
    stream.close();
    if(usedMem > availableMem){
       delete tree;         
       res->setPointer(0);
       return 0;
    }
    MemoryAVLObject* mo = new MemoryAVLObject(tree, usedMem, 
                       nl->ToString(nl->Second(qp->GetType(s))), 
                       true, getDBname());
    res->setPointer(mo);
    mo->deleteIfAllowed();
    return 0;
}


ValueMapping mcreateAVLtreeVM[] =
{
    mcreateAVLtreeVMMem,
    mcreateAVLtreeVMMP,
    mcreateAVLtreeVMStream
};

int mcreateAVLtreeSelect(ListExpr args){
   if(nl->HasLength(args,3)){
     return 2;
   }
   return Mem::checkType(nl->First(args))?0:1;
}

/*

5.16.4 Description of operator ~mcreateAVLtree~

*/

OperatorSpec mcreateAVLtreeSpec(
    "   MREL x IDENT  -> mpointer(mem(avltree)) "
    "|| stream(tuple)) x IDENT x IDENT -> mpointer(mem(avltree))",
    "_ mcreateAVLtree[_]",
    "Creates an AVLtree over an attribute of a relation. " 
    "This relation may be represented as main memory relation "
    " (mem or mpointer) or as a stream of tuples. " 
    " The second argument represents the attribute to index. "
    " If the relation is represented as a tuple stream, a third "
    " argument specifies the name of some TID attribute. ", 
    "query mStaedte mcreateAVLtree [SName]"
);

/*

5.16.5 Instance of operator ~mcreateAVLtree~

*/

Operator mcreateAVLtreeOp (
    "mcreateAVLtree",
    mcreateAVLtreeSpec.getStr(),
    3,
    mcreateAVLtreeVM,
    mcreateAVLtreeSelect,
    mcreateAVLtreeTypeMap
);



/*
5.17 Operator ~mexactmatch~
        Uses the given MemoryAVLObject or MemoryTTreeObject (as first argument)
        to find all tuples in the given MemoryRelObject (as second argument)
        with the same key value


5.17.1 Type Mapping Functions of operator ~mexactmatch~
    tree x rel x key -> stream(Tuple)


*/
ListExpr mexactmatchTM(ListExpr args) {
    string err ="{MAVLTREE(V),MTTREE}  x REL(T) x V expected ";
    
   if(nl->ListLength(args)!=3){
        return listutils::typeError("three arguments expected");
    }

    // process first argument
    ListExpr a1 = nl->First(args);
    if(MPointer::checkType(a1)){
      a1 = nl->Second(a1);
    }
    if(!Mem::checkType(a1)){
      return listutils::typeError("first arg is not a memory object");
    }
    a1 = nl->Second(a1);

    ListExpr a2 = nl->Second(args);
    if(MPointer::checkType(a2)){
      a2 = nl->Second(a2);
    }
    if(!Mem::checkType(a2)){
      return listutils::typeError("second argument is not a memory object");
    }
    a2 = nl->Second(a2);

    bool avl = MemoryAVLObject::checkType(a1);
    bool ttree = MemoryTTreeObject::checkType(a1);
    
    if(!avl && !ttree){
      return listutils::typeError("first arg is not an avl tree or a t tree");
    }

    if(!Relation::checkType(a2)){
      return listutils::typeError("second arg is not a memory relation");
    }

    ListExpr a3 = nl->Third(args);

    if(!nl->Equal(a3, nl->Second(a1))){
      return listutils::typeError("type managed by tree and key type differ "
                                  );
    }

    ListExpr res = nl->TwoElemList( listutils::basicSymbol<Stream<Tuple> >(),
                            nl->Second(a2));

    return nl->ThreeElemList(nl->SymbolAtom(Symbols::APPEND()),
                             nl->OneElemList(
                                nl->BoolAtom(avl)),
                             res);

}

class avlOperLI{
    public:
        avlOperLI(
           memAVLtree* _tree,
           vector<Tuple*>* _relation, 
           Attribute* _attr1,
           Attribute* _attr2, 
           bool _below)
           :relation(_relation), avltree(_tree), attr1(_attr1), attr2(_attr2),
           below(_below){

           isAvl = true;
           if(!below){
              avlit = avltree->tail(AttrIdPair(attr1,0));    
           }
           res = true; 
        }
        
        avlOperLI(
          memttree* _tree,
          vector<Tuple*>* _relation, 
          Attribute* _attr1,
          Attribute* _attr2, 
          bool _below)
          :relation(_relation), ttree(_tree), attr1(_attr1), attr2(_attr2),
          below(_below){
          isAvl = false;
          if(!_below){
            tit = ttree->tail(AttrIdPair(attr1,0)); 
          }
          res = true; 
        }


        ~avlOperLI(){}


        Tuple* next(){
          assert(!below);
          // T-Tree
          if(!isAvl) {
            while(!tit.end()) {
              thit = *tit; 
              if ((thit.getAttr())->Compare(attr2) > 0){ // end reached
                  return 0;
              }
              Tuple* result = relation->at(thit.getTid()-1);
              if(result){
                 result->IncReference();
                 result->SetTupleId(thit.getTid());
                 tit++;
                 return result;
              } else {
                 tit++;
              }
            } // end of iterator reached
             return 0;
          }  else { // avl tree
            while(!avlit.onEnd()){
              avlhit = avlit.Get();
              if ((avlhit->getAttr())->Compare(attr2) > 0){ // end reached
                 return 0;
               }

               Tuple* result = relation->at(avlhit->getTid()-1);
               if(result){ // tuple exist
                 result->IncReference();
                 avlit.Next();
                 result->SetTupleId(avlhit->getTid());
                 return result;
               } else { // tuple deleted
                 avlit.Next();
               }
            }
            return 0;     // end of iterator reached
          }
          return 0; // should never be reached
        }


        Tuple* matchbelow(){
          assert(below);
          if (res) {
            // AVL-Tree
            if(isAvl) {
              size_t i = relation->size();
              avlhit = avltree->GetNearestSmallerOrEqual
                          (AttrIdPair(attr1,i));
              if (avlhit==0) {
                  return 0;
              }
              Tuple* result = relation->at(avlhit->getTid()-1);
              if(result){ // present in index, but deleted in rel
                result->IncReference();
                result->SetTupleId(avlhit->getTid());
              }
              res = false;
              return result;
            } else { // ttree
              size_t i = relation->size();
              AttrIdPair s(attr1,i);
              const AttrIdPair* t = ttree->GetNearestSmallerOrEqual(s, 0);
              if(!t){
                return 0;
              }
              Tuple* result = relation->at(t->getTid()-1);
              if(result){
                 result->IncReference();
                 result->SetTupleId(t->getTid());
              }
              res = false;
              return result;
            }
          }
          return 0;
        }

    private:
        vector<Tuple*>* relation;
        memAVLtree* avltree;
        memttree* ttree;
        Attribute* attr1;
        Attribute* attr2;
        string keyType;
        bool below;
        avlIterator avlit;
        ttree::Iterator<AttrIdPair,AttrComp> tit;
        const AttrIdPair* avlhit;
        AttrIdPair thit;
        bool res;
        bool isAvl;
};


/*

5.17.3  The Value Mapping Functions of operator ~mexactmatch~

*/

template<class T, class R, bool below>
int mexactmatchVMT (Word* args, Word& result,
                    int message, Word& local, Supplier s) {

    avlOperLI* li = (avlOperLI*) local.addr;


    switch (message)
    {
        case INIT: { // arguments required, create once in open
            return 0;
        }
        case FINISH : {
            qp->GetLocal2(s).addr = 0; // do not delete here
            return 0;
        }

        case OPEN: {
            if(li){
                delete li;
                local.addr=0;
            }
            MemoryRelObject* mro = (MemoryRelObject*) qp->GetLocal2(s).addr;
            if(!mro){  // retrieve relation only once          
               R* relN = (R*) args[1].addr;
               mro = getMemRel(relN, nl->Second(qp->GetType(s)));
               qp->GetLocal2(s).addr = mro;
            }
            if(!mro){
                return 0;
            }

            Attribute* key = (Attribute*) args[2].addr;

            bool avl = ((CcBool*) args[3].addr)->GetValue();
            
            T* treeN = (T*) args[0].addr;
            //ListExpr subtype = qp->GetType(qp->GetSon(s,2));


            if(avl) {
              memAVLtree* avltree = getAVLtree(treeN);
              local.addr= new avlOperLI(avltree,
                                      mro->getmmrel(),
                                      key,key,
                                      below);
            } else {
              MemoryTTreeObject* ttree = getTtree(treeN);
              if(ttree) {
                local.addr= new avlOperLI(ttree->gettree(),
                                      mro->getmmrel(),
                                      key,key,
                                      below);
              } else {
                return 0;
              }
            }
            return 0;
        }

        case REQUEST:
            if(below){
               result.addr=li?li->matchbelow():0;
            } else {
               result.addr=li?li->next():0;
            }
            return result.addr?YIELD:CANCEL;

        case CLOSE:
            if(li) {
              delete li;
              local.addr = 0;
            }
            return 0;
   }

    return 0;
}


ValueMapping mexactmatchVM[] = {
   mexactmatchVMT<Mem, Mem, false>,
   mexactmatchVMT<Mem, MPointer, false>,
   mexactmatchVMT<MPointer, Mem, false>,
   mexactmatchVMT<MPointer, MPointer, false>,
};

ValueMapping matchbelowVM[] = {
   mexactmatchVMT<Mem, Mem, true>,
   mexactmatchVMT<Mem, MPointer, true>,
   mexactmatchVMT<MPointer, Mem, true>,
   mexactmatchVMT<MPointer, MPointer, true>,
};

int mexactmatchSelect(ListExpr args){
  int n1 = Mem::checkType(nl->First(args))?0:2;
  int n2 = Mem::checkType(nl->Second(args))?0:1;
  return n1 + n2;
}

/*

5.17.4 Description of operator ~mexactmatch~

*/

OperatorSpec mexactmatchSpec(
    "MTREE x MREL x T -> stream(Tuple), MTREE is an avltree or ttree, "
    "MTREE, MRel represented as string, mem, or mpointer",
    "_ _ mexactmatch[_]",
    "Uses the given MemoryAVLObject (as first argument) to find all tuples "
    "in the given MemoryRelObject (as second argument) "
    "that have the same attribute value",
    "query mStaedte_SName, mStaedte mexactmatch [\"Dortmund\"] count"
);

/*

5.17.5 Instance of operator ~mexactmatch~

*/
Operator mexactmatchOp (
    "mexactmatch",
    mexactmatchSpec.getStr(),
    9,
    mexactmatchVM,
    mexactmatchSelect,
    mexactmatchTM
);


/*

5.19.4 Description of operator ~matchbelow~

*/

OperatorSpec matchbelowSpec(
    " MTREE x MREL x T -> stream(tuple(X)), MTREE  is an avl-tree or an ttree, "
    "MTree, MRel represented as string, mem, or mpointer",
    "_ _ matchbelow[_]",
    "returns for a key  (third argument) the tuple which "
    " contains the biggest attribute value in the AVLtree (first argument) "
    " which is smaller than key  or equal to key",
    "query mStaedte_SNamem mStaedte matchbelow [\"Dortmund\"] count"
);

Operator matchbelowOp (
    "matchbelow",
    matchbelowSpec.getStr(),
    9,
    matchbelowVM,
    mexactmatchSelect,
    mexactmatchTM
);


/*
5.18 Operator ~mrange~
        Uses the given MemoryAVLObject or MemoryTTreeObject (as first argument)
        to find all tuples in the given MemoryRelObject (as second argument)
        which are between the first and the second attribute value
        (as third and fourth argument)

5.18.1 Type Mapping Functions of operator ~mrange~
    MAVLTREE(V)  x MREL(T)  x key x key -> stream(T)


*/
ListExpr mrangeTypeMap(ListExpr args)
{
    if(nl->ListLength(args)!=4){
        return listutils::typeError("four arguments expected");
    }
    ListExpr a1 = nl->First(args);
    ListExpr a2 = nl->Second(args);
    ListExpr a3 = nl->Third(args);
    ListExpr a4 = nl->Fourth(args);

    if(MPointer::checkType(a1)){
      a1 = nl->Second(a1);
    }
    if(!Mem::checkType(a1)){
      return listutils::typeError("first argument is not a memory object");
    }
    if(MPointer::checkType(a2)){
       a2 = nl->Second(a2);
    }
    if(!Mem::checkType(a2)){
      return listutils::typeError("second argument is not a memory object");
    }


    a1 = nl->Second(a1); // remove mem
    a2 = nl->Second(a2); 

    if(!MemoryAVLObject::checkType(a1) && !MemoryTTreeObject::checkType(a1)){
      return listutils::typeError( "(first arg is not an avl tree or a ttree)");
    }

    if(!Relation::checkType(a2)){
      return listutils::typeError(" (second arg is not a memory relation)");
    }

    ListExpr treeType = nl->Second(a1);
    if(!nl->Equal(treeType, a3)){
      return listutils::typeError("treetype and type of arg 3 differ");
    }
    if(!nl->Equal(treeType, a4)){
      return listutils::typeError("treetype and type of arg 4 differ");
    }
    return nl->TwoElemList(nl->SymbolAtom(Symbol::STREAM()),
                           nl->Second(a2));
}

/*

5.18.3  The Value Mapping Functions of operator ~mrange~

*/

template<class T, class R, bool isAVL>
int mrangeVMT (Word* args, Word& result,
                    int message, Word& local, Supplier s) {

    avlOperLI* li = (avlOperLI*) local.addr;

    switch (message)
    {
        case OPEN: {
            if(li){
                delete li;
                local.addr=0;
            }
            
            R* relN = (R*) args[1].addr;
            MemoryRelObject* mro;
            if(R::requiresTypeCheckInVM()){
               mro = getMemRel(relN, nl->Second(qp->GetType(s)));
            } else {
               mro = getMemRel(relN);
            }
            if(!mro){
              return 0;
            }
            Attribute* key1 = (Attribute*) args[2].addr;
            Attribute* key2 = (Attribute*) args[3].addr;
            T* treeN = (T*) args[0].addr;

            if(isAVL) {
              memAVLtree* avltree;
              if(T::requiresTypeCheckInVM()){           
                  ListExpr subtype = qp->GetType(qp->GetSon(s,2));
                  avltree = getAVLtree(treeN, subtype);
              } else {
                  avltree = getAVLtree(treeN);
              }
              local.addr= new avlOperLI(avltree,
                                      mro->getmmrel(),
                                      key1,key2,
                                      false);
            } else {
              MemoryTTreeObject* ttree;
              if(T::requiresTypeCheckInVM()){
                 //ListExpr subtype = qp->GetType(qp->GetSon(s,2));
                 ttree = getTtree(treeN); //, subtype);
              } else {
                 ttree = getTtree(treeN);
              }
              if(ttree) {
                local.addr= new avlOperLI(ttree->gettree(),
                                      mro->getmmrel(),
                                      key1,key2,
                                      false);
              }
            }
            return 0;
        }

        case REQUEST:
            result.addr=li?li->next():0;
            return result.addr?YIELD:CANCEL;

        case CLOSE:
            if(li) {
              delete li;
              local.addr = 0;
            }
            return 0;
   }

   return -1;
}




ValueMapping mrangeVM[] = {
    mrangeVMT<Mem,Mem,true>,
    mrangeVMT<Mem,Mem,false>,
    mrangeVMT<Mem,MPointer,true>,
    mrangeVMT<Mem,MPointer,false>,
    mrangeVMT<MPointer,Mem, true>,
    mrangeVMT<MPointer,Mem, false>,
    mrangeVMT<MPointer,MPointer,true>,
    mrangeVMT<MPointer,MPointer,false>
};

int mrangeSelect(ListExpr args){
   int n1 = 0;
   ListExpr a1 = nl->First(args);
   if(MPointer::checkType(a1)){
     n1 = 4;
     a1 = nl->Second(a1);
   }
   ListExpr tt = nl->Second(a1);
   int n3 = MemoryAVLObject::checkType(tt)?0:1;
   ListExpr a2 = nl->Second(args);
   int n2 = MPointer::checkType(a2)?2:0;
   return n1+n2+n3;
}

/*

5.18.4 Description of operator ~mrange~

*/

OperatorSpec mrangeSpec(
    "MTREE x MREL x T x T -> stream(Tuple(X)), MTREE is an avl-tree "
    "or a t-tree, MTREE, MREL represented as mem or mpointer ",
    "_ _ mrange[_,_]",
    "Uses the given avl-tree or a ttree to find all tuples"
    " in the given relation which are between "
     "the first and the second attribute value.",
    "query Staedte_SName Staedte mrange [\"Aachen\",\"Dortmund\"] count"
);

/*

5.18.5 Instance of operator ~mrange~

*/
Operator mrangeOp (
    "mrange",
    mrangeSpec.getStr(),
    8,
    mrangeVM,
    mrangeSelect,
    mrangeTypeMap
);



/*
Operators providing a tid-stream

mexactzmatchS
mrangeS
matchbelowS

*/
template<bool wrap>
ListExpr mexactmatchSTM(ListExpr args){
  string err =" MTREE(T)  x T  expected";
  if(!nl->HasLength(args,2)){
     return listutils::typeError(err + " (wrong number of args)");
  }
  ListExpr a1 = nl->TheEmptyList();
  if(!getMemSubType(nl->First(args),a1)){
    return listutils::typeError("first arg is not a memory object");
  }
  if(!MemoryAVLObject::checkType(a1)){
    return listutils::typeError(err + " (1st arg i not an avl tree)");
  }
  if(!nl->Equal(nl->Second(a1), nl->Second(args))){
     return listutils::typeError("avl type and search type differ");
  }
  if(!wrap){
      return nl->TwoElemList( 
                   listutils::basicSymbol<Stream<TupleIdentifier> >(),
                   listutils::basicSymbol<TupleIdentifier>());
  } else {
      ListExpr attrList = nl->OneElemList(
                            nl->TwoElemList(
                                nl->SymbolAtom("TID"),
                                listutils::basicSymbol<TupleIdentifier>()));
      return nl->TwoElemList(
                     listutils::basicSymbol<Stream<Tuple> >(),
                     nl->TwoElemList(
                         listutils::basicSymbol<Tuple>(),
                         attrList));
  }
}


template<bool wrap>
ListExpr mrangeSTM(ListExpr args){
  string err =" MAVLTREE(T) x T x T  expected";
  if(!nl->HasLength(args,3)){
     return listutils::typeError(err + " (wrong number of args)");
  }
  string errMsg;
  ListExpr a1;
  if(!getMemSubType(nl->First(args),a1)){
    return listutils::typeError("first arg is not a memory object");
  }
  if(!MemoryAVLObject::checkType(a1)){
    return listutils::typeError(err + " (1st arg i not an avl tree)");
  }
  if(!nl->Equal(nl->Second(a1), nl->Second(args))){
     return listutils::typeError("avl type and search type  1 differ");
  }
  if(!nl->Equal(nl->Second(a1), nl->Third(args))){
     return listutils::typeError("avl type and search type 2 differ");
  }
  if(!wrap){
      return nl->TwoElemList( 
                   listutils::basicSymbol<Stream<TupleIdentifier> >(),
                   listutils::basicSymbol<TupleIdentifier>());
  } else {
      ListExpr attrList = nl->OneElemList(
                            nl->TwoElemList(
                                nl->SymbolAtom("TID"),
                                listutils::basicSymbol<TupleIdentifier>()));
      return nl->TwoElemList(
                     listutils::basicSymbol<Stream<Tuple> >(),
                     nl->TwoElemList(
                         listutils::basicSymbol<Tuple>(),
                         attrList));
  }
}

class AVLOpS{
  public:
     AVLOpS(memAVLtree* _tree, 
            Attribute* _beg, Attribute* _end): 
        tree(_tree), end(_end){
        it = tree->tail(AttrIdPair(_beg,0));
        tt = 0;
     }

     AVLOpS(memAVLtree* _tree, 
            Attribute* _beg, 
            Attribute* _end,
            ListExpr tupleType): 
        tree(_tree), end(_end){
        it = tree->tail(AttrIdPair(_beg,0));
        tt = new TupleType(tupleType);
     }
  
     ~AVLOpS(){
       if(tt){
         tt->DeleteIfAllowed();
       }
     }
     

     TupleIdentifier* nextTID(){
         if(it.onEnd()){
           return 0;
         } 
         const AttrIdPair* p = *it;
         it++;
         int cmp = p->getAttr()->Compare(end);
         if(cmp>0){
           return 0;
         }
         return new TupleIdentifier(true,p->getTid());
     }

     Tuple* nextTuple(){
        assert(tt);
        TupleIdentifier* tid = nextTID();
        if(!tid){
          return 0;
        }
        Tuple* res = new Tuple(tt);
        res->PutAttribute(0,tid);
        return res; 
     }



   private:
       memAVLtree* tree;
       Attribute* end;
       avlIterator it;
       TupleType* tt;
};


template<class T, bool wrap>
int mrangeSVMT (Word* args, Word& result,
                    int message, Word& local, Supplier s) {


  AVLOpS* li = (AVLOpS*) local.addr;
  switch(message){
     case OPEN:{
        if(li){
          delete li;
          local.addr = 0;
        }
        T* treeN = (T*) args[0].addr;
        memAVLtree* tree = getAVLtree(treeN, qp->GetType(qp->GetSon(s,1)));
        if(!tree){
           return 0;
        }
        Attribute* beg = (Attribute*) args[1].addr;
        Attribute* end = (Attribute*) args[qp->GetNoSons(s)-1].addr;
        if(!wrap){
           local.addr = new AVLOpS(tree,beg,end);
        } else {
           local.addr = new AVLOpS(tree,beg,end,
                                   nl->Second(GetTupleResultType(s)));
        }
        return 0;
     }
     case REQUEST: {
        if(!li){
           result.addr = 0;
        } else {
           if(wrap){
              result.addr = li->nextTuple();
           } else {
              result.addr = li->nextTID();
           }
        }
        return result.addr?YIELD:CANCEL;
     }
          
     case CLOSE:{
         if(li){
            delete li;
            local.addr = 0;
         }
         return 0;
     }
  }
  return -1;
}


ValueMapping mrangeSVM[] = {
   mrangeSVMT<Mem, true>,
   mrangeSVMT<MPointer, true>
};

int mrangeSSelect(ListExpr args){
  ListExpr a1 = nl->First(args);
  if(Mem::checkType(a1)) return 0;
  if(MPointer::checkType(a1)) return 1;
  return -1;
}


OperatorSpec mexactmatchSSpec(
  "MTREE x MREL  x T -> stream(tuple((TID tid))), MTREE is an avl-tree "
  "or a t-tree, MTREE, MREL represented as string, mem, or mpointer ",
  "_ memexactmatchS[_]",
  "Retrieves the tuple ids from an avl-tree whose "
  "keys have the value given by the second arg.",
  "query mstrassen_Name mexactmatch[\"Hirzerweg\"] count"
);

OperatorSpec mrangeSSpec(
  "MTREE x T x T -> stream(tuple((TID tid))), MTREE is an avl-tree "
  " represented as mem or mpointer",
  "_ mrangeS[_,_] ",
  "Retrieves the tuple ids from an avl-tree whose key "
  "is within the range defined by the last two arguments.",
  "query mstrassen_Name mrangeS[\"A\",\"B\"] count"
);



Operator mexactmatchSOp(
  "mexactmatchS",
  mexactmatchSSpec.getStr(),
  2,
  mrangeSVM,
  mrangeSSelect,
  mexactmatchSTM<true>
);

Operator mrangeSOp(
  "mrangeS",
  mrangeSSpec.getStr(),
  2,
  mrangeSVM,
  mrangeSSelect,
  mrangeSTM<true>
);

template<bool wrap>
ListExpr matchbelowSTM(ListExpr args){
  string err =" (mem (avl T)) x T  expected";
  if(!nl->HasLength(args,2)){
     return listutils::typeError(err + " (wrong number of args)");
  }
  ListExpr a1;
  if(!getMemSubType(nl->First(args),a1)){
    return listutils::typeError("1st arg is not a memory object");
  }
  if(!MemoryAVLObject::checkType(a1)){
    return listutils::typeError(err + " (1st arg i not an avl tree)");
  }
  if(!nl->Equal(nl->Second(a1), nl->Second(args))){
     return listutils::typeError("avl type and search type differ");
  }
  if(!wrap){
      return nl->TwoElemList( 
                   listutils::basicSymbol<Stream<TupleIdentifier> >(),
                   listutils::basicSymbol<TupleIdentifier>());
  } else {
      ListExpr attrList = nl->OneElemList(
                            nl->TwoElemList(
                                nl->SymbolAtom("TID"),
                                listutils::basicSymbol<TupleIdentifier>()));
      return nl->TwoElemList(
                     listutils::basicSymbol<Stream<Tuple> >(),
                     nl->TwoElemList(
                         listutils::basicSymbol<Tuple>(),
                         attrList));
  }
}


template<class T, bool wrap>
int matchbelowSVMT (Word* args, Word& result,
                    int message, Word& local, Supplier s) {

   switch (message){
      case OPEN: {
           if(local.addr){
             if(wrap){
                ((Tuple*)local.addr)->DeleteIfAllowed();
             } else {
                ((TupleIdentifier*)local.addr)->DeleteIfAllowed();
             }
             local.addr=0;
           }
           T* treeN = (T*) args[0].addr;
           memAVLtree* tree = getAVLtree(treeN, qp->GetType(qp->GetSon(s,1)));
           if(!tree){
             return 0;
           }
           Attribute* attr = (Attribute*) args[1].addr;
           AttrIdPair searchP(attr, numeric_limits<size_t>::max()); 
           const AttrIdPair* p = tree->GetNearestSmallerOrEqual(searchP);
           if(p){
              TupleIdentifier* tid =  new TupleIdentifier(true,p->getTid());
              if(wrap){
                 TupleType* tt = new TupleType(nl->Second(
                                               GetTupleResultType(s)));
                 Tuple* res = new Tuple(tt);
                 res->PutAttribute(0,tid);
                 local.addr = res; 
                 tt->DeleteIfAllowed();
              } else {
                 local.addr = tid;
              }
           } 
           return 0;
      }
      case REQUEST : {
             result.addr = local.addr;
             local.addr = 0;
             return result.addr?YIELD:CANCEL;
      }
      case CLOSE :{
           if(local.addr){
             if(wrap){
                ((Tuple*)local.addr)->DeleteIfAllowed();
             } else {
                ((TupleIdentifier*)local.addr)->DeleteIfAllowed();
             }
             local.addr=0;
           }
             return 0;
      }

   }
   return -1;
}

ValueMapping matchbelowSVM[] = {
   matchbelowSVMT<Mem, true>,
   matchbelowSVMT<MPointer, true>
};

int matchbelowSSelect(ListExpr args){
  ListExpr a1 = nl->First(args);
  if(Mem::checkType(a1)) return 0;
  if(MPointer::checkType(a1)) return 1;
  return -1;
}

OperatorSpec matchbelowSSpec(
  "MTREE x T -> stream(tuple((TID tid))) , where MTREE in an avl-tree "
  " represented as  mem or as an mpointer ",
  "_ matchbelowS[_]",
  "Returns the tid of the entry in the avl tree whose key "
  "is smaller or equal to the key attribute. If the given "
  "key is smaller than all stored values, the resulting stream "
  "will be empty.",
  "query mstrassen_Name matchbelowS[\"B\"] count"
);

Operator matchbelowSOp(
  "matchbelowS",
  matchbelowSSpec.getStr(),
  2,
  matchbelowSVM,
  matchbelowSSelect,
  matchbelowSTM<true>
);


/*
6 M-tree support

6.1 template class that enables the use of Flob types in an M-tree

*/

/*
6.1 ~mcreatemtree~: Creation of an M-tree 

6.1.1 Type Mapping

We support two variants. The first variant gets a tuple stream,
the name of the attribute to index and the attribute name of a 
tid attribute. The second gets a main memory relation and the 
attribute that should be indexed.

*/


ListExpr mcreatemtreeTM(ListExpr args){
  string err="expected: stream(tuple) x attrname x attrname [x geoid]"
             " or MREL x attrname [x geoid]";

  // ensure at least 2 arguments
  if(!nl->HasMinLength(args,2)){
    return listutils::typeError(err+" (less than 2 arguments)");
  }
  bool isTS = Stream<Tuple>::checkType(nl->First(args));
  bool isMP = MPointer::checkType(nl->First(args));
  if(!isTS && !isMP) {
    return listutils::typeError(err + " (first arg is not a tuple stream "
                                         "and not an mpointer)");
  }
  if(nl->AtomType(nl->Second(args)) != SymbolType){
    return listutils::typeError(err + " (second argument is not a valid "
                                "attribute name)");
  }
  bool geoidPresent = false;
  if(isTS){
    if(!nl->HasLength(args,3) && !nl->HasLength(args,4)){
      return listutils::typeError("for a tuplestream, "
                                  "3 or 4 arguments are required");
    }
    if(nl->AtomType(nl->Third(args)) != SymbolType){
      return listutils::typeError(err + " (third argument is not a valid "
                                "attribute name)");
    }
    if(nl->HasLength(args,4)){
      if(!Geoid::checkType(nl->Fourth(args))){
         return listutils::typeError("fourth argument is not a geoid");
      }
      geoidPresent = true;
    }
  } 
  else {
    if(!nl->HasLength(args,2) && !nl->HasLength(args,3)){
      return listutils::typeError("for an mpointer, 2 or 3 "
                                  " arguments are required");
    }
    if(nl->HasLength(args,3)){
       if(!Geoid::checkType(nl->Third(args))){
         return listutils::typeError("last argument is not an geoid");
       }
       geoidPresent = true;
    }
  }

  // extract tuple type from first argument
  ListExpr tupletype;
  if(isMP){
     ListExpr mpt = nl->Second(nl->Second(nl->First(args)));
     if(!Relation::checkType(mpt)){
       return listutils::typeError("mpointer to a non-relation");
     }
     tupletype = nl->Second(mpt);
  } else {
     tupletype = nl->Second(nl->First(args));
  }

  ListExpr attrList =nl->Second(tupletype);

  string name = nl->SymbolValue(nl->Second(args));
  ListExpr type;
  int index1 = listutils::findAttribute(attrList, name, type);
  if(!index1){
    return listutils::typeError("attribute " + name 
                                + " not part of the tuple");
  }
  // support geoid only for point, mpoint, cupoint, cmpoint
  if(geoidPresent){
    if(!Point::checkType(type) && !temporalalgebra::MPoint::checkType(type) &&
       !temporalalgebra::CUPoint::checkType(type) &&
       !temporalalgebra::CMPoint::checkType(type)) {
      return listutils::typeError("geoid support only for (m|cu|cm|eps)point");
    }
  }


  int index2 = -1;
  if(isTS){
     string tidname = nl->SymbolValue(nl->Third(args));
     ListExpr tidtype;
     index2 = listutils::findAttribute(attrList, tidname, tidtype);
     if(!index2){
        return listutils::typeError("attribute " + tidname 
                                 + "not known in tuple");
     }
     if(!TupleIdentifier::checkType(tidtype)){
       return listutils::typeError("attribute " + tidname 
                                   + " not of type tid");
    }
  }

  // check for supported type, extend if required
  int no = mtreehelper::getTypeNo(type,12);
  if(no <0){
     return listutils::typeError("there is no known distance function for type "
                               + nl->ToString(type));
  }

  ListExpr resType = MPointer::wrapType( 
                        Mem::wrapType(
                          nl->TwoElemList(
                             listutils::basicSymbol<
                               MemoryMtreeObject<Point, 
                                             StdDistComp<Point> > >(),
                             type
                          )));
  ListExpr appendList;
  if(isTS){
    if(geoidPresent){
       appendList = nl->ThreeElemList( 
                         nl->IntAtom(index1-1),
                         nl->IntAtom(index2-1),
                         nl->StringAtom(nl->ToString(type)));
    } else {
       appendList = nl->FourElemList(
                         nl->TwoElemList(listutils::basicSymbol<Geoid>(),
                                         listutils::getUndefined()), 
                         nl->IntAtom(index1-1),
                         nl->IntAtom(index2-1),
                         nl->StringAtom(nl->ToString(type)));

    }
  } else {
    if(geoidPresent){
        appendList = nl->TwoElemList( nl->IntAtom(index1-1), 
                                      nl->StringAtom(nl->ToString(type)));
     } else {
        appendList = nl->ThreeElemList( 
                               nl->TwoElemList(listutils::basicSymbol<Geoid>(),
                                               listutils::getUndefined()), 
                               nl->IntAtom(index1-1), 
                               nl->StringAtom(nl->ToString(type)));
     }
  }

  ListExpr result = nl->ThreeElemList(
                     nl->SymbolAtom(Symbols::APPEND()),
                     appendList,
                     resType);

  return result;
}



/*
6.2 Value Mapping template

*/
template <class T>
int mcreatemtreeVMTStream (Word* args, Word& result, int message,
                    Word& local, Supplier s) {

   result = qp->ResultStorage(s);
   MPointer* res = (MPointer*) result.addr;
   Geoid* geoid = (Geoid*)  args[3].addr;
   int index1 = ((CcInt*)   args[4].addr)->GetValue(); 
   int index2 = ((CcInt*)   args[5].addr)->GetValue(); 

   if(!geoid->IsDefined()){
      geoid = 0;
   }

   StdDistComp<T> dc(geoid);
   MMMTree<MTreeEntry<T>, StdDistComp<T> >* tree =
     new MMMTree<MTreeEntry<T>, StdDistComp<T> >(4, 8, dc);
//    typedef pair<T,TupleId> treeentry_t;
//    MMMTree<treeentry_t,StdDistComp<T>,MemCloner<treeentry_t> >* tree = 
//            new MMMTree<treeentry_t,StdDistComp<T>,
//                        MemCloner<treeentry_t>>(4,8,dc);

   Stream<Tuple> stream(args[0]);
   stream.open();
   Tuple* tuple;
   bool flobused = false;
   while( (tuple = stream.request())){
      T* attr = (T*) tuple->GetAttribute(index1);
      TupleIdentifier* tid = (TupleIdentifier*) tuple->GetAttribute(index2);
      if(tid->IsDefined()){
//         T copy = *attr;
        flobused = flobused || (attr->NumOfFLOBs()>0);
        MTreeEntry<T> entry(*attr, tid->GetTid());
//         pair<T,TupleId> p(copy, tid->GetTid());
        tree->insert(entry);
      }
      tuple->DeleteIfAllowed();
   }
   stream.close();
   size_t usedMem = tree->memSize();
   ListExpr typeList = nl->Second(qp->GetType(s));
   MemoryMtreeObject<T, StdDistComp<T> >* mtree = 
          new MemoryMtreeObject<T, StdDistComp<T> > (tree,  
                             usedMem, 
                             nl->ToString(typeList), 
                             !flobused, getDBname());
//    MemoryMtreeObject<T,StdDistComp<T> >* mtree = 
//           new MemoryMtreeObject<T, StdDistComp<T> > (tree,  
//                              usedMem, 
//                              nl->ToString(typeList), 
//                              !flobused, getDBname());
   mtreehelper::increaseCounter("counterMCreateMTree", 
                          mtree->getmtree()->getDistComp().getNoDistFunCalls());
   res->setPointer(mtree);
   mtree->deleteIfAllowed();
   return 0;
}


template <class T>
int mcreatemtreeVMTMP (Word* args, Word& result, int message,
                    Word& local, Supplier s) {

   result = qp->ResultStorage(s);
   MPointer* res = (MPointer*) result.addr;
   MPointer* mrelp = (MPointer*) args[0].addr;
   if(mrelp->isNull()){
     res->setPointer(0);
     return 0;
   }
   Geoid* geoid = (Geoid*) args[2].addr;
   int index1 = ((CcInt*) args[3].addr)->GetValue(); 
   string tn = ((CcString*) args[4].addr)->GetValue();

   if(!geoid->IsDefined()){
     geoid = 0;
   }

   MemoryRelObject* mrel = (MemoryRelObject*) mrelp->GetValue();
   vector<Tuple*>* v = mrel->getmmrel();


   StdDistComp<T> dc(geoid);
//    typedef pair<T,TupleId> treeentry_t;
   MMMTree<MTreeEntry<T>, StdDistComp<T> >* tree =
     new MMMTree<MTreeEntry<T>, StdDistComp<T> >(4, 8, dc);
//    MMMTree<treeentry_t,StdDistComp<T>,MemCloner<treeentry_t>  >* tree = 
//            new MMMTree<treeentry_t,StdDistComp<T>, 
//                        MemCloner<treeentry_t> >(4,8,dc);

   Tuple* tuple;
   bool flobused = false;
   if(v){
     for(size_t i=0;i<v->size();i++){
       tuple = v->at(i);
       if(tuple){
         T* attr = (T*) tuple->GetAttribute(index1);
//          T copy = *attr;
         flobused = flobused || (attr->NumOfFLOBs()>0);
         MTreeEntry<T> entry(*attr, i+1);
//          pair<T,TupleId> p(copy, i+1);
         tree->insert(entry);
         entry.Destroy();
       }
     }
   }
   size_t usedMem = tree->memSize();
   ListExpr typeList = nl->Second(qp->GetType(s));
   MemoryMtreeObject<T, StdDistComp<T> >* mtree = 
          new MemoryMtreeObject<T, StdDistComp<T> > (tree,  
                             usedMem, 
                             nl->ToString(typeList), 
                             !flobused, getDBname());
//    MemoryMtreeObject<T,StdDistComp<T> >* mtree = 
//           new MemoryMtreeObject<T, StdDistComp<T> > (tree,  
//                              usedMem, 
//                              nl->ToString(typeList), 
//                              !flobused, getDBname());
   mtreehelper::increaseCounter("counterMCreateMTree", 
                          mtree->getmtree()->getDistComp().getNoDistFunCalls());
   res->setPointer(mtree);
   mtree->deleteIfAllowed();
   return 0;
}


/*
6.3 Selection and  Value Mapping Array

*/
int mcreatemtreeSelect(ListExpr args){
   int o1;
   string attrName = nl->SymbolValue(nl->Second(args));
   ListExpr attrList;
   if(Stream<Tuple>::checkType(nl->First(args))){   
     o1 = 0;
     attrList = nl->Second(nl->Second(nl->First(args)));
   } else {
     o1 = 12;
     attrList=nl->Second(nl->Second(nl->Second(nl->Second(nl->First(args)))));
            //  mpointer   mem         rel       tuple 
   }

   ListExpr type;
   listutils::findAttribute(attrList, attrName, type);
   return o1 + mtreehelper::getTypeNo(type,12);
}

 // note: if adding attributes with flobs, the value mapping must be changed

ValueMapping mcreatemtreeVM[] = {
  mcreatemtreeVMTStream<mtreehelper::t1>,
  mcreatemtreeVMTStream<mtreehelper::t2>,
  mcreatemtreeVMTStream<mtreehelper::t3>,
  mcreatemtreeVMTStream<mtreehelper::t4>,
  mcreatemtreeVMTStream<mtreehelper::t5>,
  mcreatemtreeVMTStream<mtreehelper::t6>,
  mcreatemtreeVMTStream<mtreehelper::t7>,
  mcreatemtreeVMTStream<mtreehelper::t8>,
  mcreatemtreeVMTStream<mtreehelper::t9>,
  mcreatemtreeVMTStream<mtreehelper::t10>,
  mcreatemtreeVMTStream<mtreehelper::t11>,
  mcreatemtreeVMTStream<mtreehelper::t12>,
  mcreatemtreeVMTMP<mtreehelper::t1>,
  mcreatemtreeVMTMP<mtreehelper::t2>,
  mcreatemtreeVMTMP<mtreehelper::t3>,
  mcreatemtreeVMTMP<mtreehelper::t4>,
  mcreatemtreeVMTMP<mtreehelper::t5>,
  mcreatemtreeVMTMP<mtreehelper::t6>,
  mcreatemtreeVMTMP<mtreehelper::t7>,
  mcreatemtreeVMTMP<mtreehelper::t8>,
  mcreatemtreeVMTMP<mtreehelper::t9>,
  mcreatemtreeVMTMP<mtreehelper::t10>,
  mcreatemtreeVMTMP<mtreehelper::t11>,
  mcreatemtreeVMTMP<mtreehelper::t12>
};

OperatorSpec mcreatemtreeSpec(
  "stream(tuple) x attrname x attrname [x geoid] -> mpointer(mem(mtree X))||\n"
  "MREL(tuple) x attrname [x geoid] -> , mpointer(mem(mtree X))",
  "tuplestream  mcreatemtree[ indexAttr, TID_attr [, geoid] ] ||\n"
  "mrel mcreatemtree[indexAttr [, geoid] ]",
  "This operator creates an m-tree in main memory. "
  "The first argument is a stream or a main memory relation containing the "
  "tuples to be indexed. The second argument refers to the attribute "
  "over that the index is built. If the tuples are provided as a stream, "
  "the third argument refers to an attribute inside the tuple containg its "
  "tuple id. The last argument is optional. It must be of type geoid and "
  "can only be used if the index-attribute is of type point, mpoint, cupoint, "
  "or cmpoint. If this argument is present, the distance between two objects "
  "is computed as geographic distance on this geoid instead of using the "
  "Euclidean distance.\n In detail, the following types are supported:\n\n"
  "  * point:   p1->Distance(*p2, geoid)\n"
  "  * string:  stringutils::ld->(s1->GetValue(), s2->GetValue())\n"
  "  * int:     abs(i1->GetValue() - i2->GetValue())\n"
  "  * real:    abs(r1->GetValue() - r2->GetValue())\n"
  "  * rect<d>: r1->Distance(*r2)\n"
  "  * mpoint:  mp1->DistanceAvg(*mp2, geoid)\n"
  "  * cupoint: cup1->DistanceAvg(*cup2, false, geoid)\n"
  "  * cmpoint: cmp1->DistanceAvg(*cmp2, false, geoid)\n",
  "let kinos_mtree_GeoData =  kinos feed addid mcreatemtree[GeoData, TID]"
);

Operator mcreatemtreeOp(
   "mcreatemtree",
   mcreatemtreeSpec.getStr(),
   24,
   mcreatemtreeVM,
   mcreatemtreeSelect,
   mcreatemtreeTM
);

/*
Operator ~mcreatemtree2~

6.1 Type Mapping

*/
ListExpr mcreatemtree2TM(ListExpr args) {
  string err= "expected: stream(tuple) x attrname x attrname x attrname x real "
              "[x geoid]\n or MREL x attrname x attrname x real [x geoid]";
  if (!nl->HasMinLength(args, 4)) {
    return listutils::typeError(err+" (less than 4 arguments)");
  }
  bool isTS = Stream<Tuple>::checkType(nl->First(args));
  bool isMP = MPointer::checkType(nl->First(args));
  if (!isTS && !isMP) {
    return listutils::typeError(err + " (first arg is neither a tuple stream "
                                         "nor an mpointer)");
  }
  if (nl->AtomType(nl->Second(args)) != SymbolType){
    return listutils::typeError(err + " (second argument is not a valid "
                                "attribute name)");
  }
  if (nl->AtomType(nl->Third(args)) != SymbolType) {
    return listutils::typeError(err + " (third argument is not a valid "
                                "attribute name)");
  }
  if (isTS) {
    if(!nl->HasLength(args, 5) && !nl->HasLength(args, 6)) {
      return listutils::typeError("for a tuple stream, "
                                  "5 or 6 arguments are required");
    }
    if (nl->AtomType(nl->Fourth(args)) != SymbolType) {
      return listutils::typeError(err + " (fourth argument is not a valid "
                                "attribute name)");
    }
    if (!CcReal::checkType(nl->Fifth(args))) {
      return listutils::typeError(err + " (fifth argument is not a real)");
    }
    if (nl->HasLength(args, 6)) {
      if (!Geoid::checkType(nl->Sixth(args))) {
         return listutils::typeError(" (sixth argument is not a geoid)");
      }
    }
  } 
  else {
    if (!nl->HasLength(args, 4) && !nl->HasLength(args, 5)) {
      return listutils::typeError("for an mpointer, 4 or 5 "
                                  " arguments are required");
    }
    if (nl->HasLength(args, 5)) {
      if (!Geoid::checkType(nl->Fifth(args))) {
         return listutils::typeError("fifth argument is not a geoid");
       }
    }
  }
  // extract tuple type from first argument
  ListExpr tupletype;
  if (isMP) {
    ListExpr mpt = nl->Second(nl->Second(nl->First(args)));
    if (!Relation::checkType(mpt)) {
      return listutils::typeError("mpointer to a non-relation");
    }
    tupletype = nl->Second(mpt);
  }
  else {
    tupletype = nl->Second(nl->First(args));
  }
  ListExpr attrList = nl->Second(tupletype);
  string mpName = nl->SymbolValue(nl->Second(args));
  ListExpr type;
  int index1 = listutils::findAttribute(attrList, mpName, type);
  if (!index1) {
    return listutils::typeError("attr " + mpName + " not part of the tuple");
  }
  if(!temporalalgebra::MPoint::checkType(type) &&
     !temporalalgebra::CUPoint::checkType(type) &&
     !temporalalgebra::CMPoint::checkType(type)) {
    return listutils::typeError(" support only for (m|cu|cm)point");
  }
  string mlName = nl->SymbolValue(nl->Third(args));
  int index2 = listutils::findAttribute(attrList, mlName, type);
  if (!index2) {
    return listutils::typeError("attr " + mlName + " not part of the tuple");
  }
  if (!stj::isSymbolicType(type)) {
    return listutils::typeError(" support only for mlabel(s) or mplace(s)");
  }
  int index3 = -1;
  if (isTS) {
    string tidname = nl->SymbolValue(nl->Fourth(args));
    ListExpr tidtype;
    index3 = listutils::findAttribute(attrList, tidname, tidtype);
    if (!index3) {
      return listutils::typeError("attribute " + tidname + " does not exist");
    }
    if (!TupleIdentifier::checkType(tidtype)) {
      return listutils::typeError("attribute " + tidname + " not of type tid");
    }
  }
  ListExpr resType = MPointer::wrapType( 
                        Mem::wrapType(
                          nl->TwoElemList(
                             listutils::basicSymbol<
                               MemoryMtreeObject<Point, 
                                             StdDistComp<Point> > >(),
                             nl->SymbolAtom("tuple")
                          )));
  ListExpr appendList;
  if (isTS) {
    appendList = nl->ThreeElemList(nl->IntAtom(index1 - 1),
                                   nl->IntAtom(index2 - 1),
                                   nl->IntAtom(index3 - 1));
  }
  else {
    appendList = nl->TwoElemList(nl->IntAtom(index1 - 1), 
                                 nl->IntAtom(index2 - 1));
  }
  ListExpr result = nl->ThreeElemList(nl->SymbolAtom(Symbols::APPEND()),
                                      appendList,
                                      resType);
  return result;
}

/*
6.2 Value Mapping functions

*/
template<class Spa, class Sym>
int mcreatemtree2StreamVM(Word* args, Word& result, int message, Word& local, 
                          Supplier s) {
  result = qp->ResultStorage(s);
  MPointer* res = (MPointer*)result.addr;
  Geoid *geoid = 0;
  int offset = 0;
  if (qp->GetNoSons(s) == 9) { // 6 given, 3 additional
    geoid = (Geoid*)args[5].addr;
    if (!geoid->IsDefined()) {
      geoid = 0;
    }
    offset++;
  }
  CcReal* alphacc = (CcReal*)args[4].addr;
  if (!alphacc->IsDefined()) {
    return 0;
  }
  double alpha = alphacc->GetValue();
  if (alpha < 0.0 || alpha > 1.0) {
    cout << "alpha must be in [0, 1]" << endl;
    return 0;
  }
  int indexSpa = ((CcInt*)args[5 + offset].addr)->GetValue(); 
  int indexSym = ((CcInt*)args[6 + offset].addr)->GetValue();
  int indexTID = ((CcInt*)args[7 + offset].addr)->GetValue();
  StdDistCompExt<Spa, Sym> dc(geoid, alpha);
  MMMTree<MTreeEntry<pair<Spa, Sym> >, StdDistCompExt<Spa, Sym> >* tree =
    new MMMTree<MTreeEntry<pair<Spa, Sym> >, StdDistCompExt<Spa, Sym> >(4,8,dc);
//   typedef pair<pair<Spa, Sym>, TupleId>  treeentry_t; 
//   MMMTree<treeentry_t, StdDistCompExt<Spa, Sym>,
//           MemCloner<treeentry_t>  >* tree = 
//       new MMMTree<treeentry_t,
//                   StdDistCompExt<Spa, Sym>,
//                   MemCloner<treeentry_t>  >(4, 8, dc);
  Stream<Tuple> stream(args[0]);
  stream.open();
  Tuple* tuple;
  bool flobused = false;
  Spa* spa = 0;
  Sym *sym = 0;
  while ((tuple = stream.request())) {
    spa = (Spa*)(tuple->GetAttribute(indexSpa));
    sym = (Sym*)(tuple->GetAttribute(indexSym));
    TupleIdentifier* tid = (TupleIdentifier*)(tuple->GetAttribute(indexTID));
    if (tid->IsDefined()) {
      Spa copySpa = *spa;
      Sym copySym = *sym;
      flobused = flobused || copySpa.NumOfFLOBs() > 0 ||
        copySym.NumOfFLOBs() > 0;
      pair<Spa, Sym> spasym(copySpa, copySym);
      MTreeEntry<pair<Spa, Sym> > entry(spasym, tid->GetTid());
//       pair<pair<Spa, Sym>, TupleId> p(spasym, tid->GetTid());
      tree->insert(entry);
      entry.Destroy();
    }
    tuple->DeleteIfAllowed();
  }
  stream.close();
  size_t usedMem = tree->memSize();
  ListExpr typeList = nl->Second(qp->GetType(s));
  MemoryMtreeObject<pair<Spa, Sym>, StdDistCompExt<Spa, Sym> >*
     mtree = new MemoryMtreeObject<pair<Spa, Sym>, StdDistCompExt<Spa, Sym> > 
                (tree, usedMem, nl->ToString(typeList), !flobused, getDBname());
  mtreehelper::increaseCounter("counterMCreateMTree", 
                          mtree->getmtree()->getDistComp().getNoDistFunCalls());
  res->setPointer(mtree);
  mtree->deleteIfAllowed();
  return 0;
}

template<class Spa, class Sym>
int mcreatemtree2MRelVM(Word* args, Word& result, int message, Word& local, 
                        Supplier s) {
  result = qp->ResultStorage(s);
  MPointer* res = (MPointer*)result.addr;
  MPointer* mrelp = (MPointer*)args[0].addr;
  if (mrelp->isNull()) {
    res->setPointer(0);
    return 0;
  }
  Geoid *geoid = 0;
  int offset = 0;
  if (qp->GetNoSons(s) == 7) { // 5 given, 2 additional
    geoid = (Geoid*)args[4].addr;
    if (!geoid->IsDefined()) {
      geoid = 0;
    }
    offset++;
  }
  CcReal* alphacc = (CcReal*)args[3].addr;
  if (!alphacc->IsDefined()) {
    return 0;
  }
  double alpha = alphacc->GetValue();
  if (alpha < 0.0 || alpha > 1.0) {
    cout << "alpha must be in [0, 1]" << endl;
    return 0;
  }
  int indexSpa = ((CcInt*)args[4 + offset].addr)->GetValue();
  int indexSym = ((CcInt*)args[5 + offset].addr)->GetValue();
  MemoryRelObject* mrel = (MemoryRelObject*) mrelp->GetValue();
  vector<Tuple*>* v = mrel->getmmrel();
  StdDistCompExt<Spa, Sym> dc(geoid, alpha);
  MMMTree<MTreeEntry<pair<Spa, Sym> >, StdDistCompExt<Spa, Sym> >* tree =
    new MMMTree<MTreeEntry<pair<Spa, Sym> >, StdDistCompExt<Spa, Sym> >(4,8,dc);
//   typedef pair<pair<Spa, Sym>, TupleId> treeentry_t;
//   MMMTree<treeentry_t, StdDistCompExt<Spa, Sym>,
//           MemCloner<treeentry_t>  >* tree = 
//     new MMMTree<treeentry_t, 
//                 StdDistCompExt<Spa, Sym>,
//                 MemCloner<treeentry_t>  >(4, 8, dc);
  Tuple* tuple;
  bool flobused = false;
  Spa* spa = 0;
  Sym* sym = 0;
  if (v) {
    for(size_t i = 0; i < v->size(); i++) {
      tuple = v->at(i);
      if (tuple) {
        spa = (Spa*)tuple->GetAttribute(indexSpa);
        sym = (Sym*)tuple->GetAttribute(indexSym);
        Spa copySpa = *spa;
        Sym copySym = *sym;
        pair<Spa, Sym> spasym(copySpa, copySym);
        MTreeEntry<pair<Spa, Sym> > entry(spasym, i + 1);
//         pair<pair<Spa, Sym>, TupleId> p(spasym, i + 1);
        tree->insert(entry);
        entry.Destroy();
      }
    }
  }
  size_t usedMem = tree->memSize();
  ListExpr typeList = nl->Second(qp->GetType(s));
  MemoryMtreeObject<pair<Spa, Sym>, StdDistCompExt<Spa, Sym> >*
     mtree = new MemoryMtreeObject<pair<Spa, Sym>, StdDistCompExt<Spa, Sym> > 
                (tree, usedMem, nl->ToString(typeList), !flobused, getDBname());
  mtreehelper::increaseCounter("counterMCreateMTree", 
                          mtree->getmtree()->getDistComp().getNoDistFunCalls());
  res->setPointer(mtree);
  mtree->deleteIfAllowed();
  return 0;
}

/*
6.3 Selection Function and Value Mapping Array

*/
int mcreatemtree2Select(ListExpr args) {
  bool isStream = Stream<Tuple>::checkType(nl->First(args));
  ListExpr tupletype;
  int noSymTypes = 4;
  int noSpaTypes = 3;
  int offset = 0;
  if (!isStream) {
    tupletype = nl->Second(nl->Second(nl->Second(nl->First(args))));
    offset = noSymTypes * noSpaTypes;
  }
  else {
    tupletype = nl->Second(nl->First(args));
  }
  ListExpr attrList = nl->Second(tupletype);
  string symName = nl->SymbolValue(nl->Third(args));
  ListExpr type;
  listutils::findAttribute(attrList, symName, type);
  int symbolicType;
  if (stj::MLabel::checkType(type)) {
    symbolicType = 0;
  }
  if (stj::MLabels::checkType(type)) {
    symbolicType = 1;
  }
  if (stj::MPlace::checkType(type)) {
    symbolicType = 2;
  }
  if (stj::MPlaces::checkType(type)) {
    symbolicType = 3;
  }
  string spaName = nl->SymbolValue(nl->Second(args));
  listutils::findAttribute(attrList, spaName, type);
  if (temporalalgebra::MPoint::checkType(type)) {
    return symbolicType + offset;
  }
  if (temporalalgebra::CUPoint::checkType(type)) {
    return noSymTypes + symbolicType + offset;
  }
  if (temporalalgebra::CMPoint::checkType(type)) {
    return 2 * noSymTypes + symbolicType + offset;
  }
  return -1;
}

ValueMapping mcreatemtree2VMs[] = {
  mcreatemtree2StreamVM<temporalalgebra::MPoint, stj::MLabel>,
  mcreatemtree2StreamVM<temporalalgebra::MPoint, stj::MLabels>,
  mcreatemtree2StreamVM<temporalalgebra::MPoint, stj::MPlace>,
  mcreatemtree2StreamVM<temporalalgebra::MPoint, stj::MPlaces>,
  mcreatemtree2StreamVM<temporalalgebra::CUPoint, stj::MLabel>,
  mcreatemtree2StreamVM<temporalalgebra::CUPoint, stj::MLabels>,
  mcreatemtree2StreamVM<temporalalgebra::CUPoint, stj::MPlace>,
  mcreatemtree2StreamVM<temporalalgebra::CUPoint, stj::MPlaces>,
  mcreatemtree2StreamVM<temporalalgebra::CMPoint, stj::MLabel>,
  mcreatemtree2StreamVM<temporalalgebra::CMPoint, stj::MLabels>,
  mcreatemtree2StreamVM<temporalalgebra::CMPoint, stj::MPlace>,
  mcreatemtree2StreamVM<temporalalgebra::CMPoint, stj::MPlaces>,
  mcreatemtree2MRelVM<temporalalgebra::MPoint, stj::MLabel>,
  mcreatemtree2MRelVM<temporalalgebra::MPoint, stj::MLabels>,
  mcreatemtree2MRelVM<temporalalgebra::MPoint, stj::MPlace>,
  mcreatemtree2MRelVM<temporalalgebra::MPoint, stj::MPlaces>,
  mcreatemtree2MRelVM<temporalalgebra::CUPoint, stj::MLabel>,
  mcreatemtree2MRelVM<temporalalgebra::CUPoint, stj::MLabels>,
  mcreatemtree2MRelVM<temporalalgebra::CUPoint, stj::MPlace>,
  mcreatemtree2MRelVM<temporalalgebra::CUPoint, stj::MPlaces>,
  mcreatemtree2MRelVM<temporalalgebra::CMPoint, stj::MLabel>,
  mcreatemtree2MRelVM<temporalalgebra::CMPoint, stj::MLabels>,
  mcreatemtree2MRelVM<temporalalgebra::CMPoint, stj::MPlace>,
  mcreatemtree2MRelVM<temporalalgebra::CMPoint, stj::MPlaces>
};

OperatorSpec mcreatemtree2Spec(
  "stream(tuple) x attrname x attrname x attrname x real [x geoid] -> "
  "mpointer(mem(mtree tuple))||\n"
  "MREL(tuple) x attrname x attrname x real [x geoid] -> "
  "mpointer(mem(mtree tuple))",
  "tuplestream mcreatemtree2[Spatiotemp_attr, Symbolic_attr, TID_attr, alpha "
  "[, geoid] ] ||\n"
  "mrel mcreatemtree2[MPoint_attr, MLabel_attr, alpha [, geoid] ]",
  "This operator creates an m-tree in main memory. "
  "The first argument is a stream or a main memory relation containing the "
  "tuples to be indexed. The second and third arguments refer to the attributes"
  " of the types mpoint/cupoint/cmpoint and mlabel(s)/mplace(s), respectively. "
  "If the tuples are provided as a stream, the fourth argument refers to a "
  "tuple id attribute. The fifth argument is a real number in [0,1] controlling"
  "the distance function ratio. The sixth argument is optional and must be of "
  "type geoid. If it is present, the distance between two objects is computed "
  "as geographic distance on this geoid instead of using the Euclidean "
  "distance.\n The following types are supported with respective functions:\n\n"
  "  Spatial Types (first attribute name):\n"
  "  * mpoint:  DistanceAvg (geoid supported)\n"
  "  * cupoint: DistanceAvgLB (geoid supported)\n"
  "  * cmpoint: DistanceAvgLB (geoid supported)\n\n"
  "  Symbolic Types (second attribute name):\n"
  "  * mlabel:  Jaccard Similarity\n"
  "  * mlabels: Jaccard Similarity\n"
  "  * mplace:  Jaccard Similarity\n"
  "  * mplaces: Jaccard Similarity\n",
  "let kinos_mtree_GeoData =  kinos feed addid mcreatemtree[GeoData, TID]"
);

Operator mcreatemtree2Op(
   "mcreatemtree2",
   mcreatemtree2Spec.getStr(),
   24,
   mcreatemtree2VMs,
   mcreatemtree2Select,
   mcreatemtree2TM
);


/*
Operator ~minsertmtree~

*/

ListExpr minsertmtreeTM(ListExpr args){
  if(!nl->HasLength(args,3)){
    return listutils::typeError("three arguments required");
  }
  ListExpr a1 = nl->First(args);
  if(!Stream<Tuple>::checkType(a1)){
    return listutils::typeError("first argument is not a tuple stream");
  }
  ListExpr a2 = nl->Second(args);

  if(MPointer::checkType(a2)){
    a2 = nl->Second(a2);
  }
  if(!Mem::checkType(a2)){
     return listutils::typeError("arg 2 is not a memory object");
  }
  a2 = nl->Second(a2); // remove mem
 
  ListExpr a3 = nl->Third(args);
  if(!listutils::isSymbol(a3)){
    return listutils::typeError("third arg is not a valid attribute name");
  }
 
  ListExpr attrList = nl->Second(nl->Second(a1));
  ListExpr attrType;
  string attrName = nl->SymbolValue(a3);
  int attrIndex = listutils::findAttribute(attrList, attrName, attrType);
  if(!attrIndex){
    return listutils::typeError("attribute name " + attrName 
                                + " not part of the tuple");
  }
  ListExpr tt = nl->TwoElemList(nl->SymbolAtom(mtreehelper::BasicType()),
                                attrType);
  if(!nl->Equal(a2,tt)){
    return listutils::typeError("second arg is not an memory mtree over " 
                                + nl->ToString(attrType));
  }
  ListExpr tidType;
  int tidIndex = listutils::findAttribute(attrList, "TID", tidType);
  if(tidIndex==0){
     return listutils::typeError("no TID attribute found");
  }
  if(!TupleIdentifier::checkType(tidType)){
    return listutils::typeError("TID attribute not of tpye tid");
  }


  ListExpr appendList = nl->TwoElemList(nl->IntAtom(attrIndex -1),
                                        nl->IntAtom(tidIndex -1));
  return nl->ThreeElemList(
                  nl->SymbolAtom(Symbols::APPEND()),
                  appendList,
                 nl->First(args));
}


template<class T>
class minsertmtreeInfo{
  typedef StdDistComp<T> D;

  public:
     minsertmtreeInfo(Word _stream, 
                      MemoryMtreeObject<T,D >* _tree,
                      int _index, int _tidIndex): stream(_stream) {
        tree = _tree->getmtree();
        index = _index;
        tidIndex = _tidIndex;
        stream.open();
     }

     ~minsertmtreeInfo(){
       stream.close();
     }

     Tuple* next(){
        Tuple* in = stream.request();
        if(!in) return 0;
        TupleIdentifier* tid = (TupleIdentifier*) in->GetAttribute(tidIndex);
        if(!tid->IsDefined()){
           return in;
        }
        TupleId id = tid->GetTid();
        if(id!=0){ // do not insert 0 id
           T* attr = (T*) in->GetAttribute(index);
           MTreeEntry<T> p(*attr, id);
//            pair<T,TupleId> p(*attr,id);
           tree->insert(p);
        }
        return in;
     }


  private:
     Stream<Tuple> stream;
     MMMTree<MTreeEntry<T>, StdDistComp<T> >* tree;
//      MMMTree<std::pair<T, TupleId>, StdDistComp<T>,
//             MemCloner<std::pair<T, TupleId> >  >* tree;
     int index;
     int tidIndex;

};


template<class T, class N>
int minsertmtreeVMT (Word* args, Word& result, int message,
                    Word& local, Supplier s) {

   typedef StdDistComp<T> D;
   minsertmtreeInfo<T >* li = (minsertmtreeInfo<T>*) local.addr;
   switch(message){
      case  OPEN : { if(li){
                        delete li;
                     }
                     int index = ((CcInt*) args[3].addr)->GetValue();
                     int tidIndex = ((CcInt*) args[4].addr)->GetValue(); 
                     N* n = (N*) args[1].addr;
                     MemoryMtreeObject<T,D>* tree = getMtree<N,T>(n);
                     local.addr = new minsertmtreeInfo<T>(args[0], tree, 
                                                          index, tidIndex);
                     return 0;
                   }
       case REQUEST : result.addr = li?li->next():0;
                      return result.addr?YIELD:CANCEL;
       case CLOSE : { 
                     if(li){
                       delete li;
                       local.addr = 0;
                     }
                     return 0;
                    }

   }
   return -1;
}


ValueMapping minsertmtreeVM[] = {
  minsertmtreeVMT<mtreehelper::t1,Mem>,
  minsertmtreeVMT<mtreehelper::t2,Mem>,
  minsertmtreeVMT<mtreehelper::t3,Mem>,
  minsertmtreeVMT<mtreehelper::t4,Mem>,
  minsertmtreeVMT<mtreehelper::t5,Mem>,
  minsertmtreeVMT<mtreehelper::t6,Mem>,
  minsertmtreeVMT<mtreehelper::t7,Mem>,
  minsertmtreeVMT<mtreehelper::t8,Mem>,
  minsertmtreeVMT<mtreehelper::t9,Mem>,
  minsertmtreeVMT<mtreehelper::t10,Mem>,
  minsertmtreeVMT<mtreehelper::t11,Mem>,
  minsertmtreeVMT<mtreehelper::t12,Mem>,
  minsertmtreeVMT<mtreehelper::t1,MPointer>,
  minsertmtreeVMT<mtreehelper::t2,MPointer>,
  minsertmtreeVMT<mtreehelper::t3,MPointer>,
  minsertmtreeVMT<mtreehelper::t4,MPointer>,
  minsertmtreeVMT<mtreehelper::t5,MPointer>,
  minsertmtreeVMT<mtreehelper::t6,MPointer>,
  minsertmtreeVMT<mtreehelper::t7,MPointer>,
  minsertmtreeVMT<mtreehelper::t8,MPointer>,
  minsertmtreeVMT<mtreehelper::t9,MPointer>,
  minsertmtreeVMT<mtreehelper::t10,MPointer>,
  minsertmtreeVMT<mtreehelper::t11,MPointer>,
  minsertmtreeVMT<mtreehelper::t12,MPointer>
};

int minsertmtreeSelect(ListExpr args){
   int o1 = Mem::checkType(nl->Second(args))?0:12;
   string attrName = nl->SymbolValue(nl->Third(args));
   ListExpr attrList = nl->Second(nl->Second(nl->First(args)));
   ListExpr type;
   listutils::findAttribute(attrList, attrName, type);
   return  o1 + mtreehelper::getTypeNo(type,12);
}

OperatorSpec minsertmtreeSpec(
  "stream(tuple(x@TID)) x MMTREE x IDENT -> stream(tuple(x@TID))",
  "_ minsertmtree[ _ , _ ]",
  "Inserts the tuples from a stream into an existing main memory mtree."
  "TupleIDs are extracted from the TID attribute expected in the tuple. "
  "Tuples with an undefined id or an id of value 0 are not inserted "
  "into the tree. All incoming tuples are put into the output stream.",
  "query plz feed addid minsertmtree[plz_PLZ_mtree, PLZ] count"
);

Operator minsertmtreeOp(
  "minsertmtree",
   minsertmtreeSpec.getStr(),
   24,
   minsertmtreeVM,
   minsertmtreeSelect,
   minsertmtreeTM
);

/*
Operator ~mdistRange2~

This operator creates a stream of TupleIDs that are inside a 
given distance to a reference object. The used index is a 
main memory based mtree.

*/
template<bool wrap>
ListExpr mdistRange2TM(ListExpr args){

  string err = "MTREE(T) x T x real expected";
  if(!nl->HasLength(args,3)){
     return listutils::typeError(err + " ( wrong number of args)");
  }
  ListExpr a1 = nl->First(args);
  if(MPointer::checkType(a1)){
    a1 = nl->Second(a1);
  }
  if(!Mem::checkType(a1)){
     return listutils::typeError("arg 1 is not a memory object");
  }
  a1 = nl->Second(a1); // remove mem
  ListExpr mt = nl->TwoElemList(
                          nl->SymbolAtom(mtreehelper::BasicType()),
                          nl->Second(args));

  if(!nl->Equal(a1, mt)){
    return listutils::typeError("arg 1 is not an mtree over arg 2 (" 
                                + nl->ToString(nl->First(nl->Second(args)))
                                + ")");
  }
  if(!CcReal::checkType(nl->Third(args))){
     return listutils::typeError(err + " (third arg is not a real)");
  }
  if(!wrap){
      return nl->TwoElemList( 
                   listutils::basicSymbol<Stream<TupleIdentifier> >(),
                   listutils::basicSymbol<TupleIdentifier>());
  } else {
      ListExpr attrList = nl->OneElemList(
                            nl->TwoElemList(
                                nl->SymbolAtom("TID"),
                                listutils::basicSymbol<TupleIdentifier>()));
      return nl->TwoElemList(
                     listutils::basicSymbol<Stream<Tuple> >(),
                     nl->TwoElemList(
                         listutils::basicSymbol<Tuple>(),
                         attrList));
  }
}

template<class T>
struct mdistrange2Info{
   RangeIterator<MTreeEntry<T>, StdDistComp<T> >* it;
   TupleType* tt;
};


template <class T, class N, bool wrap>
int mdistRange2VMT (Word* args, Word& result, int message,
                    Word& local, Supplier s) {

  mdistrange2Info<T>* li = (mdistrange2Info<T>*) local.addr;
  switch(message){
    case OPEN: {
            if(li) {
              delete li->it;
              if(li->tt){
                li->tt->DeleteIfAllowed();
              }
              local.addr = 0;
            }

            T* attr = (T*) args[1].addr;
            CcReal* dist = (CcReal*) args[2].addr;
            if(!dist->IsDefined()){
               return 0;
            }
            double d = dist->GetValue();
            if(d < 0){
               return 0;
            }
            N* tree = (N*) args[0].addr;
            typedef  MemoryMtreeObject<T,StdDistComp<T> > mtot;
            mtot* mtreeo = getMtree<N,T>(tree);
            if(!mtreeo){
              return 0;
            }            
            typedef MMMTree<MTreeEntry<T>, StdDistComp<T> > mtt;
//             typedef MMMTree<pair<T,TupleId>,StdDistComp<T>,
//                             MemCloner<pair<T,TupleId>> > mtt;
            mtt* mtree = mtreeo->getmtree();
            if(mtree){
                T a = *attr;
                mdistrange2Info<T>* info = new mdistrange2Info<T>();
                MTreeEntry<T> entry(a, 0);
                info->it = mtree->rangeSearch(entry, d);
//                 info->it = mtree->rangeSearch(pair<T,TupleId>(a,0), d);
                if(!wrap){
                   info->tt = 0;
                } else {
                   info->tt = new TupleType(nl->Second(GetTupleResultType(s)));
                }
                local.addr = info;
            }
            return 0;
          }
     case REQUEST: {
               if(!li){
                 result.addr=0;
                 return CANCEL;
               }
               const MTreeEntry<T>* n = li->it->next();
               TupleIdentifier* res = n ?
                                     new TupleIdentifier(true, n->getTid()) : 0;
               if(!res){
                 result.addr=0;
                 return CANCEL;
               }
               if(!wrap){
                 result.addr = res;
               } else {
                  Tuple* tuple = new Tuple(li->tt);
                  tuple->PutAttribute(0,res);
                  result.addr = tuple;
               }
               return result.addr?YIELD:CANCEL;
            }
     case CLOSE:
               if(li){
                   delete li->it;
                   if(li->tt){
                      li->tt->DeleteIfAllowed();
                   }
                   delete li;
                   local.addr = 0;
               }
               return 0;
     }
     return -1;            
}

/*
6.3 Selection and  Value Mapping Array

*/
int mdistRange2Select(ListExpr args){
   ListExpr type = nl->Second(args);
   int n;
   if(Mem::checkType(nl->First(args))){
      n = 0;
   } else { // MPointer variant
      n = 12;  
   }
   int res =  mtreehelper::getTypeNo(type,12) + n;
   return res;
}

 // note: if adding attributes with flobs, the value mapping must be changed

ValueMapping mdistRange2VM[] = {
  mdistRange2VMT<mtreehelper::t1, Mem, true>,
  mdistRange2VMT<mtreehelper::t2, Mem, true>,
  mdistRange2VMT<mtreehelper::t3, Mem, true>,
  mdistRange2VMT<mtreehelper::t4, Mem, true>,
  mdistRange2VMT<mtreehelper::t5, Mem, true>,
  mdistRange2VMT<mtreehelper::t6, Mem, true>,
  mdistRange2VMT<mtreehelper::t7, Mem, true>,
  mdistRange2VMT<mtreehelper::t8, Mem, true>,
  mdistRange2VMT<mtreehelper::t9, Mem, true>,
  mdistRange2VMT<mtreehelper::t10, Mem, true>,
  mdistRange2VMT<mtreehelper::t11, Mem, true>,
  mdistRange2VMT<mtreehelper::t12, Mem, true>,
  
  mdistRange2VMT<mtreehelper::t1, MPointer, true>,
  mdistRange2VMT<mtreehelper::t2, MPointer, true>,
  mdistRange2VMT<mtreehelper::t3, MPointer, true>,
  mdistRange2VMT<mtreehelper::t4, MPointer, true>,
  mdistRange2VMT<mtreehelper::t5, MPointer, true>,
  mdistRange2VMT<mtreehelper::t6, MPointer, true>,
  mdistRange2VMT<mtreehelper::t7, MPointer, true>,
  mdistRange2VMT<mtreehelper::t8, MPointer, true>,
  mdistRange2VMT<mtreehelper::t9, MPointer, true>,
  mdistRange2VMT<mtreehelper::t10, MPointer, true>,
  mdistRange2VMT<mtreehelper::t11, MPointer, true>,
  mdistRange2VMT<mtreehelper::t12, MPointer, true>
};

OperatorSpec mdistRange2Spec(
  "MTREE x DATA x real -> stream(tuple((TID tid))), "
  "MTREE represented as a mem or mpointer ",
  "mem_mtree mdistRange2[keyAttr, maxDist] ",
  "Retrieves those tuple ids from an mtree those key value has "
  "a maximum distaance of the given dist",
  "query mkinos_mtree mdistRange2[ alexanderplatz , 2000.0] count"
);

Operator mdistRange2Op(
   "mdistRange2",
   mdistRange2Spec.getStr(),
   24,
   mdistRange2VM,
   mdistRange2Select,
   mdistRange2TM<true>
);


/*
Operator ~mdistScan2~

This operator creates a stream of TupleIDs 
whose associated objects are in increasing order
to the reference object.

*/
template<bool wrap>
ListExpr mdistScan2TM(ListExpr args){
  string err = "MTREE  x T  expected";
  if(!nl->HasLength(args,2)){
     return listutils::typeError(err + " ( wrong number of args)");
  }
  ListExpr a1 = nl->First(args);
  ListExpr a2 = nl->Second(args);
  if(MPointer::checkType(a1)){
    a1 = nl->Second(a1);
  }
  if(!Mem::checkType(a1)){
    return listutils::typeError("1st arg is not a memory object");
  }
  a1 = nl->Second(a1); // remove mem
  if(!mtreehelper::checkType(a1,a2)){
     return listutils::typeError(err+ "( first arg is not an "
                                      "mtree over key type)");
  }
  if(!wrap){
      return nl->TwoElemList( 
                   listutils::basicSymbol<Stream<TupleIdentifier> >(),
                   listutils::basicSymbol<TupleIdentifier>());
  } else {
      ListExpr attrList = nl->OneElemList(
                            nl->TwoElemList(
                                nl->SymbolAtom("TID"),
                                listutils::basicSymbol<TupleIdentifier>()));
      return nl->TwoElemList(
                     listutils::basicSymbol<Stream<Tuple> >(),
                     nl->TwoElemList(
                         listutils::basicSymbol<Tuple>(),
                         attrList));
  }
}

template<class T>
struct mdistScan2Info{
  NNIterator<MTreeEntry<T>, StdDistComp<T> >* it;
//    NNIterator<pair<T,TupleId>, StdDistComp<T>, 
//               MemCloner<pair<T,TupleId> >   >* it;
   TupleType* tt;
};


template <class T, class N, bool wrap>
int mdistScan2VMT (Word* args, Word& result, int message,
                    Word& local, Supplier s) {
  mdistScan2Info<T>* li = (mdistScan2Info<T>*) local.addr;
  int noDistFunCalls = 0;
  switch(message){
    case OPEN: {
            if(li) {
              delete li->it;
              if(li->tt){
                li->tt->DeleteIfAllowed();
              }
              delete li;
              local.addr = 0;
            }
            T* attr = (T*) args[1].addr;
            N* Name = (N*) args[0].addr;
            typedef MemoryMtreeObject<T,StdDistComp<T> > mtot;
            mtot* tree = getMtree<N,T>(Name);
            if(!tree){
              return 0;
            }
            MMMTree<MTreeEntry<T>, StdDistComp<T> >* mtree = tree->getmtree();
//             MMMTree<pair<T, TupleId>,StdDistComp<T>,
//                     MemCloner<pair<T, TupleId>> >* mtree = 
//                     tree->getmtree();
            if(mtree){
              MTreeEntry<T> p(*attr, 0);
//               pair<T,TupleId> p(*attr,0);
              li = new mdistScan2Info<T>();
              li->it =  mtree->nnSearch(p);
              if(!wrap){
                li->tt=0;
              } else {
                li->tt = new TupleType(nl->Second(GetTupleResultType(s)));
              }
              local.addr = li;
            }
            return 0;
          }
     case REQUEST: {
               if(!li){
                 result.addr=0;
                 return CANCEL;
               }
               const MTreeEntry<T>* n = li->it->next();
               TupleIdentifier* tid = n ? 
                                     new TupleIdentifier(true, n->getTid()) : 0;
//                const pair<T,TupleId>* n = li->it->next();
//              TupleIdentifier* tid = n? new TupleIdentifier(true,n->second):0;
               if(!tid){
                 result.addr=0;
                 return CANCEL;
               } 
               if(!wrap){
                  result.addr = tid;
               } else {
                  Tuple* tuple = new Tuple(li->tt);
                  tuple->PutAttribute(0,tid);
                  result.addr = tuple;
               }
                
               return result.addr?YIELD:CANCEL;
            }
     case CLOSE:
               cout << "Number of distance function calls: " << noDistFunCalls
                    << endl;
               if(li){
                 delete li->it;
                 if(li->tt){
                    li->tt->DeleteIfAllowed();
                 }
                 delete li;
                 local.addr = 0;
               }
               return 0;
     }
     return -1;            
}

/*
6.3 Selection and  Value Mapping Array

*/
int mdistScan2Select(ListExpr args){
   ListExpr type = nl->Second(args);
   int n;
   int m = 12;
   if(Mem::checkType(nl->First(args))){
     n = 0;
   } else {  // mpointer
     n = m;
   }
   return mtreehelper::getTypeNo(type,m) + n;
}

 // note: if adding attributes with flobs, the value mapping must be changed

ValueMapping mdistScan2VM[] = {
  mdistScan2VMT<mtreehelper::t1, Mem, true>,
  mdistScan2VMT<mtreehelper::t2, Mem, true>,
  mdistScan2VMT<mtreehelper::t3, Mem, true>,
  mdistScan2VMT<mtreehelper::t4, Mem, true>,
  mdistScan2VMT<mtreehelper::t5, Mem, true>,
  mdistScan2VMT<mtreehelper::t6, Mem, true>,
  mdistScan2VMT<mtreehelper::t7, Mem, true>,
  mdistScan2VMT<mtreehelper::t8, Mem, true>,
  mdistScan2VMT<mtreehelper::t9, Mem, true>,
  mdistScan2VMT<mtreehelper::t10, Mem, true>,
  mdistScan2VMT<mtreehelper::t11, Mem, true>,
  mdistScan2VMT<mtreehelper::t12, Mem, true>,

  mdistScan2VMT<mtreehelper::t1, MPointer, true>,
  mdistScan2VMT<mtreehelper::t2, MPointer, true>,
  mdistScan2VMT<mtreehelper::t3, MPointer, true>,
  mdistScan2VMT<mtreehelper::t4, MPointer, true>,
  mdistScan2VMT<mtreehelper::t5, MPointer, true>,
  mdistScan2VMT<mtreehelper::t6, MPointer, true>,
  mdistScan2VMT<mtreehelper::t7, MPointer, true>,
  mdistScan2VMT<mtreehelper::t8, MPointer, true>,
  mdistScan2VMT<mtreehelper::t9, MPointer, true>,
  mdistScan2VMT<mtreehelper::t10, MPointer, true>,
  mdistScan2VMT<mtreehelper::t11, MPointer, true>,
  mdistScan2VMT<mtreehelper::t12, MPointer, true>
};

OperatorSpec mdistScan2Spec(
  "MTREE  x DATA -> stream(tuple((TID tid))), "
  "MTREE represented as string, me, or mpointer ",
  "mem_mtree mdistScan2[keyAttr] ",
  "Scans the tuple ids within an m-tree in increasing "
  "distance of the reference object to the associated "
  "objects.",
  "query mkinos_mtree mdistScan2[ alexanderplatz] count"
);

Operator mdistScan2Op(
   "mdistScan2",
   mdistScan2Spec.getStr(),
   36,
   mdistScan2VM,
   mdistScan2Select,
   mdistScan2TM<true>
);


/*
Operator mdistRange

*/
ListExpr mdistRangeTM(ListExpr args) {
  string err = "MTREE(T) x MREL x T (x U) x real expected";
  if (!nl->HasLength(args,4) && !nl->HasLength(args, 5)) {
    return listutils::typeError(err + " (wrong number of args)");
  }
  ListExpr a1 = nl->First(args);
  ListExpr a2 = nl->Second(args);
  if (MPointer::checkType(a1)) { 
    a1 = nl->Second(a1);
  }
  if (!Mem::checkType(a1)) {
    return listutils::typeError("first arg is not a memory object");
  }
  if (MPointer::checkType(a2)) {
    a2 = nl->Second(a2);
  }
  if (!Mem::checkType(a2)) {
    return listutils::typeError("2nd arg is not a memory object");
  }
  a1 = nl->Second(a1);
  a2 = nl->Second(a2);
  if (!Relation::checkType(a2)) {
    return listutils::typeError("second arg is not a relation");
  }
  ListExpr a3 = nl->Third(args);
  ListExpr a4 = nl->Fourth(args);
  if (nl->HasLength(args, 4)) {
    if (!mtreehelper::checkType(a1, a3)) {
      return listutils::typeError("first arg is not a mtree over " 
                                  + nl->ToString(a3));
    }
  }
  else {
    if (!mtreehelper::checkType(a1, nl->SymbolAtom(Tuple::BasicType()))) {
      return listutils::typeError("first arg is not a mtree over tuples");
    }
    if (!temporalalgebra::MPoint::checkType(a3) &&
        !temporalalgebra::CUPoint::checkType(a3) &&
        !temporalalgebra::CMPoint::checkType(a3)) {
      return listutils::typeError("third arg is not a (m|cu|cm)point");
    }
    if (!stj::isSymbolicType(a4)) {
      return listutils::typeError("fourth arg is not an mlabel(s) / mplace(s)");
    }
    a4 = nl->Fifth(args);
  }
  if (!CcReal::checkType(a4)) {
    return listutils::typeError(err + "final arg is not a real");
  }
  return nl->TwoElemList(
                listutils::basicSymbol<Stream<Tuple> >(),
                nl->Second(a2)); 
}

template<class T, class Dist>
class distRangeInfo{
  public:

     distRangeInfo( MemoryMtreeObject<T, Dist>* mtree, 
                    MemoryRelObject* mrel, 
                    T* ref, 
                    double dist){
                 
                rel = mrel->getmmrel();
                MTreeEntry<T> p(*ref, 0);
//                 pair<T,TupleId> p(*ref,0);
                it = mtree->getmtree()->rangeSearch(p, dist);
              }

     ~distRangeInfo() {
       delete it;
     }

     Tuple* next() {
        while(true) {
            const MTreeEntry<T>* p = it->next();
//             const pair<T,TupleId>* p = it->next();
            if(!p) {
              return 0;
            }
            if (p->getTid() <= rel->size()) {
               Tuple* res = (*rel)[p->getTid() - 1];
               if(res) { // ignore deleted tuples
                 res->IncReference();
                 return res;
               }
            }
        }
        return 0;
     }
     
     size_t getNoDistFunCalls() {
       return it->getNoDistFunCalls();
     }
     

  private:
     vector<Tuple*>* rel;
     RangeIterator<MTreeEntry<T>, Dist>* it;
//      RangeIterator<pair<T,TupleId> , StdDistComp<T>,
//                    MemCloner<pair<T,TupleId> > >* it;

};

template<class K, class T, class R>
int mdistRangeVMT (Word* args, Word& result, int message, Word& local,
                   Supplier s) {
  distRangeInfo<K, StdDistComp<K> >* li = 
                              (distRangeInfo<K, StdDistComp<K> >*) local.addr;
  switch (message) {
    case OPEN : {
      if (li) {
        delete li;
        local.addr = 0;
      }
      R* relN = (R*)args[1].addr;
      MemoryRelObject* rel = getMemRel(relN, nl->Second(qp->GetType(s)));
      if (!rel) {
        return 0;
      }
      CcReal* dist = (CcReal*)args[3].addr;
      if (!dist->IsDefined()) {
        return 0;
      }
      double d = dist->GetValue();
      if (d < 0) {
        return 0;
      }
      T* treeN = (T*)args[0].addr;
      MemoryMtreeObject<K, StdDistComp<K> >* m = getMtree<T, K>(treeN);
      if (!m) {
        return 0;
      }
      K* key = (K*)args[2].addr;
      local.addr = new distRangeInfo<K, StdDistComp<K> >(m, rel, key, d);
      return 0;
    }
    case REQUEST: {
      result.addr = li ? li->next() : 0;
      return result.addr ? YIELD : CANCEL;
    }
    case CLOSE : {
      if (li) {
        mtreehelper::increaseCounter("counterMDistRange", 
                                     li->getNoDistFunCalls());
        delete li;
        local.addr = 0;
      }
      return 0;
    }
   }
   return -1;
}

template<class K, class L, class T, class R>
int mdistRangeVMT2(Word* args, Word& result, int message, Word& local, 
                   Supplier s) {
  distRangeInfo<pair<K, L>, StdDistCompExt<K, L> >* li = 
                 (distRangeInfo<pair<K, L>, StdDistCompExt<K, L> >*) local.addr;
  switch (message) {
    case OPEN : {
      if (li) {
        delete li;
        local.addr = 0;
      }
      R* relN = (R*)args[1].addr;
      MemoryRelObject* rel = getMemRel(relN, nl->Second(qp->GetType(s)));
      if (!rel) {
        return 0;
      }
      CcReal* dist = (CcReal*)args[4].addr;
      if (!dist->IsDefined()) {
        return 0;
      }
      double d = dist->GetValue();
      if (d < 0) {
        return 0;
      }
      T* treeN = (T*)args[0].addr;
      MemoryMtreeObject<pair<K, L>, StdDistCompExt<K, L> >* m = 
                                                      getMtree<T, K, L >(treeN);
      if (!m) {
        return 0;
      }
      K* key1 = (K*)args[2].addr;
      L* key2 = (L*)args[3].addr;
      pair<K, L> key(*key1, *key2);
      local.addr = new distRangeInfo<pair<K, L>, StdDistCompExt<K, L> >(m, rel,
                                                                       &key, d);
      return 0;
    }
    case REQUEST: {
      result.addr = li ? li->next() : 0;
      return result.addr ? YIELD : CANCEL;
    }
    case CLOSE : {
      if (li) {
        mtreehelper::increaseCounter("counterMDistRange2", 
                                     li->getNoDistFunCalls());
        delete li;
        local.addr = 0;
      }
      return 0;
    }
  }
  return -1;
}

template<int minNoArgs>
int mdistRangeScanSelect(ListExpr args){
  int o1 = -1;
  int o2 = -1;
  ListExpr a1 = nl->First(args);
  ListExpr a2 = nl->Second(args);
  if (nl->HasLength(args, minNoArgs)) {
    ListExpr typeL = nl->Third(args);
    int type =  mtreehelper::getTypeNo(typeL,12);
    
    if(Mem::checkType(a1)) o1=0;
    if(MPointer::checkType(a1)) o1=24;
    if(o1<0) return -1; 

    if(Mem::checkType(a2)) o2=0;
    if(MPointer::checkType(a2)) o2=12;
    if(o2<0) return -1; 
   
    int res =  type + o1 + o2;
    return res;
  }
  else {
    ListExpr typeList1 = nl->Third(args);
    ListExpr typeList2 = nl->Fourth(args);
    int type1 = mtreehelper::getTypeNo(typeList1, 12) - 9;
    int type2 = stj::getTypeNo(typeList2);
    if (Mem::checkType(a1)) {
      o1 = 48;
    }
    if (MPointer::checkType(a1)) {
      o1 = 72;
    }
    if (o1 < 0) {
      return -1;
    }

    if (Mem::checkType(a2)) {
      o2 = 0;
    }
    if (MPointer::checkType(a2)) {
      o2 = 12;
    }
    if (o2 < 0) {
      return -1;
    }
    int res = type1 + 3 * type2 + o1 + o2;
    return res;
  }
  
}

// note: if adding attributes with flobs, the value mapping must be changed

ValueMapping mdistRangeVM[] = {
  mdistRangeVMT<mtreehelper::t1,Mem,Mem>,
  mdistRangeVMT<mtreehelper::t2,Mem,Mem>,
  mdistRangeVMT<mtreehelper::t3,Mem,Mem>,
  mdistRangeVMT<mtreehelper::t4,Mem,Mem>,
  mdistRangeVMT<mtreehelper::t5,Mem,Mem>,
  mdistRangeVMT<mtreehelper::t6,Mem,Mem>,
  mdistRangeVMT<mtreehelper::t7,Mem,Mem>,
  mdistRangeVMT<mtreehelper::t8,Mem,Mem>,
  mdistRangeVMT<mtreehelper::t9,Mem,Mem>,
  mdistRangeVMT<mtreehelper::t10,Mem,Mem>,
  mdistRangeVMT<mtreehelper::t11,Mem,Mem>,
  mdistRangeVMT<mtreehelper::t12,Mem,Mem>,
  
  mdistRangeVMT<mtreehelper::t1,Mem,MPointer>,
  mdistRangeVMT<mtreehelper::t2,Mem,MPointer>,
  mdistRangeVMT<mtreehelper::t3,Mem,MPointer>,
  mdistRangeVMT<mtreehelper::t4,Mem,MPointer>,
  mdistRangeVMT<mtreehelper::t5,Mem,MPointer>,
  mdistRangeVMT<mtreehelper::t6,Mem,MPointer>,
  mdistRangeVMT<mtreehelper::t7,Mem,MPointer>,
  mdistRangeVMT<mtreehelper::t8,Mem,MPointer>,
  mdistRangeVMT<mtreehelper::t9,Mem,MPointer>,
  mdistRangeVMT<mtreehelper::t10,Mem,MPointer>,
  mdistRangeVMT<mtreehelper::t11,Mem,MPointer>,
  mdistRangeVMT<mtreehelper::t12,Mem,MPointer>,
  
  mdistRangeVMT<mtreehelper::t1,MPointer,Mem>,
  mdistRangeVMT<mtreehelper::t2,MPointer,Mem>,
  mdistRangeVMT<mtreehelper::t3,MPointer,Mem>,
  mdistRangeVMT<mtreehelper::t4,MPointer,Mem>,
  mdistRangeVMT<mtreehelper::t5,MPointer,Mem>,
  mdistRangeVMT<mtreehelper::t6,MPointer,Mem>,
  mdistRangeVMT<mtreehelper::t7,MPointer,Mem>,
  mdistRangeVMT<mtreehelper::t8,MPointer,Mem>,
  mdistRangeVMT<mtreehelper::t9,MPointer,Mem>,
  mdistRangeVMT<mtreehelper::t10,MPointer,Mem>,
  mdistRangeVMT<mtreehelper::t11,MPointer,Mem>,
  mdistRangeVMT<mtreehelper::t12,MPointer,Mem>,
  
  mdistRangeVMT<mtreehelper::t1,MPointer,MPointer>,
  mdistRangeVMT<mtreehelper::t2,MPointer,MPointer>,
  mdistRangeVMT<mtreehelper::t3,MPointer,MPointer>,
  mdistRangeVMT<mtreehelper::t4,MPointer,MPointer>,
  mdistRangeVMT<mtreehelper::t5,MPointer,MPointer>,
  mdistRangeVMT<mtreehelper::t6,MPointer,MPointer>,
  mdistRangeVMT<mtreehelper::t7,MPointer,MPointer>,
  mdistRangeVMT<mtreehelper::t8,MPointer,MPointer>,
  mdistRangeVMT<mtreehelper::t9,MPointer,MPointer>,
  mdistRangeVMT<mtreehelper::t10,MPointer,MPointer>,
  mdistRangeVMT<mtreehelper::t11,MPointer,MPointer>,
  mdistRangeVMT<mtreehelper::t12,MPointer,MPointer>,
  
  mdistRangeVMT2<temporalalgebra::MPoint, stj::MLabel, Mem, Mem>,
  mdistRangeVMT2<temporalalgebra::CUPoint, stj::MLabel, Mem, Mem>,
  mdistRangeVMT2<temporalalgebra::CMPoint, stj::MLabel, Mem, Mem>,
  mdistRangeVMT2<temporalalgebra::MPoint, stj::MLabels, Mem, Mem>,
  mdistRangeVMT2<temporalalgebra::CUPoint, stj::MLabels, Mem, Mem>,
  mdistRangeVMT2<temporalalgebra::CMPoint, stj::MLabels, Mem, Mem>,
  mdistRangeVMT2<temporalalgebra::MPoint, stj::MPlace, Mem, Mem>,
  mdistRangeVMT2<temporalalgebra::CUPoint, stj::MPlace, Mem, Mem>,
  mdistRangeVMT2<temporalalgebra::CMPoint, stj::MPlace, Mem, Mem>,
  mdistRangeVMT2<temporalalgebra::MPoint, stj::MPlaces, Mem, Mem>,
  mdistRangeVMT2<temporalalgebra::CUPoint, stj::MPlaces, Mem, Mem>,
  mdistRangeVMT2<temporalalgebra::CMPoint, stj::MPlaces, Mem, Mem>,
  
  mdistRangeVMT2<temporalalgebra::MPoint, stj::MLabel, Mem, MPointer>,
  mdistRangeVMT2<temporalalgebra::CUPoint, stj::MLabel, Mem, MPointer>,
  mdistRangeVMT2<temporalalgebra::CMPoint, stj::MLabel, Mem, MPointer>,
  mdistRangeVMT2<temporalalgebra::MPoint, stj::MLabels, Mem, MPointer>,
  mdistRangeVMT2<temporalalgebra::CUPoint, stj::MLabels, Mem, MPointer>,
  mdistRangeVMT2<temporalalgebra::CMPoint, stj::MLabels, Mem, MPointer>,
  mdistRangeVMT2<temporalalgebra::MPoint, stj::MPlace, Mem, MPointer>,
  mdistRangeVMT2<temporalalgebra::CUPoint, stj::MPlace, Mem, MPointer>,
  mdistRangeVMT2<temporalalgebra::CMPoint, stj::MPlace, Mem, MPointer>,
  mdistRangeVMT2<temporalalgebra::MPoint, stj::MPlaces, Mem, MPointer>,
  mdistRangeVMT2<temporalalgebra::CUPoint, stj::MPlaces, Mem, MPointer>,
  mdistRangeVMT2<temporalalgebra::CMPoint, stj::MPlaces, Mem, MPointer>,
  
  mdistRangeVMT2<temporalalgebra::MPoint, stj::MLabel, MPointer, Mem>,
  mdistRangeVMT2<temporalalgebra::CUPoint, stj::MLabel, MPointer, Mem>,
  mdistRangeVMT2<temporalalgebra::CMPoint, stj::MLabel, MPointer, Mem>,
  mdistRangeVMT2<temporalalgebra::MPoint, stj::MLabels, MPointer, Mem>,
  mdistRangeVMT2<temporalalgebra::CUPoint, stj::MLabels, MPointer, Mem>,
  mdistRangeVMT2<temporalalgebra::CMPoint, stj::MLabels, MPointer, Mem>,
  mdistRangeVMT2<temporalalgebra::MPoint, stj::MPlace, MPointer, Mem>,
  mdistRangeVMT2<temporalalgebra::CUPoint, stj::MPlace, MPointer, Mem>,
  mdistRangeVMT2<temporalalgebra::CMPoint, stj::MPlace, MPointer, Mem>,
  mdistRangeVMT2<temporalalgebra::MPoint, stj::MPlaces, MPointer, Mem>,
  mdistRangeVMT2<temporalalgebra::CUPoint, stj::MPlaces, MPointer, Mem>,
  mdistRangeVMT2<temporalalgebra::CMPoint, stj::MPlaces, MPointer, Mem>,
  
  mdistRangeVMT2<temporalalgebra::MPoint, stj::MLabel, MPointer, MPointer>,
  mdistRangeVMT2<temporalalgebra::CUPoint, stj::MLabel, MPointer, MPointer>,
  mdistRangeVMT2<temporalalgebra::CMPoint, stj::MLabel, MPointer, MPointer>,
  mdistRangeVMT2<temporalalgebra::MPoint, stj::MLabels, MPointer, MPointer>,
  mdistRangeVMT2<temporalalgebra::CUPoint, stj::MLabels, MPointer, MPointer>,
  mdistRangeVMT2<temporalalgebra::CMPoint, stj::MLabels, MPointer, MPointer>,
  mdistRangeVMT2<temporalalgebra::MPoint, stj::MPlace, MPointer, MPointer>,
  mdistRangeVMT2<temporalalgebra::CUPoint, stj::MPlace, MPointer, MPointer>,
  mdistRangeVMT2<temporalalgebra::CMPoint, stj::MPlace, MPointer, MPointer>,
  mdistRangeVMT2<temporalalgebra::MPoint, stj::MPlaces, MPointer, MPointer>,
  mdistRangeVMT2<temporalalgebra::CUPoint, stj::MPlaces, MPointer, MPointer>,
  mdistRangeVMT2<temporalalgebra::CMPoint, stj::MPlaces, MPointer, MPointer>
};

OperatorSpec mdistRangeSpec(
  "MTREE x MREL  x T (x U) x real -> stream(tuple) , MTREE, "
  "MREL represented as string, mem, or mpointer",
  "mem_mtree mem_rel mdistRange[keyAttr, maxDist] ",
  "Retrieves those tuples from a memory relation "
  "having a distance smaller or equals to a given dist "
  "to a key value (or pair of key values). This operation is aided by a memory "
  "based m-tree.",
  "query mkinos_mtree mKinos mdistRange[ alexanderplatz , 2000.0] count"
);

Operator mdistRangeOp(
   "mdistRange",
   mdistRangeSpec.getStr(),
   96,
   mdistRangeVM,
   mdistRangeScanSelect<4>,
   mdistRangeTM
);

/*
Operator ~mdistRangeNx~

Type Mapping, used for mdistRangeN, mdistRangeN2, mdistRangeN5, mdistRangeN6,
mdistRange7, and mdistRangeN8

*/
template<int variant>
ListExpr mdistRangeNTM(ListExpr args) {
  string err = "NTREE(T) x MREL x T (x U) x real expected";
  if (!nl->HasLength(args, 4) && !nl->HasLength(args, 5)) {
    return listutils::typeError(err + " (wrong number of args)");
  }
  ListExpr a1 = nl->First(args);
  ListExpr a2 = nl->Second(args);
  if (MPointer::checkType(a1)) { 
    a1 = nl->Second(a1);
  }
  if (!Mem::checkType(a1)) {
    return listutils::typeError("first arg is not a memory object");
  }
  if (MPointer::checkType(a2)) {
    a2 = nl->Second(a2);
  }
  if (!Mem::checkType(a2)) {
    return listutils::typeError("2nd arg is not a memory object");
  }
  a1 = nl->Second(a1);
  a2 = nl->Second(a2);
  if (!Relation::checkType(a2)) {
    return listutils::typeError("second arg is not a relation");
  }
  ListExpr a3 = nl->Third(args);
  ListExpr a4 = nl->Fourth(args);
  if (nl->HasLength(args, 4)) {
    if (!mtreehelper::checkTypeN(a1, a3, variant)) {
      return listutils::typeError("first arg is not an ntree" + 
                              to_string(variant) + " over " + nl->ToString(a3));
    }
  }
  else {
    if (!mtreehelper::checkTypeN(a1, nl->SymbolAtom(Tuple::BasicType()), 
                                 variant)) {
      return listutils::typeError("first arg is not an ntree" +
                                  to_string(variant) + " over tuples");
    }
    if (!temporalalgebra::MPoint::checkType(a3) &&
        !temporalalgebra::CUPoint::checkType(a3) &&
        !temporalalgebra::CMPoint::checkType(a3)) {
      return listutils::typeError("third arg is not a (m|cu|cm)point");
    }
    if (!stj::isSymbolicType(a4)) {
      return listutils::typeError("fourth arg is not an mlabel(s) / mplace(s)");
    }
    a4 = nl->Fifth(args);
  }
  if (!CcReal::checkType(a4)) {
    return listutils::typeError(err + "final arg is not a real");
  }
  return nl->TwoElemList(listutils::basicSymbol<Stream<Tuple> >(),
                         nl->Second(a2)); 
}

template<class T, class Dist, int variant>
class distRangeNInfo {
 public:
  distRangeNInfo(MemoryNtreeObject<T, Dist, variant>* ntreeX,
                 MemoryRelObject* mrel, T* ref, double range) {
    rel = mrel->getmmrel();
    MTreeEntry<T> p(*ref, 0);
    it = ntreeX->getNtreeX()->rangeSearch(p, range);
  }

  ~distRangeNInfo() {
    delete it;
  }

  Tuple* next() {
    while (true) {
      const TupleId tid = it->next();
      if ((int)tid == -1) {
        return 0;
      }
      if (tid <= rel->size()) {
        Tuple* res = (*rel)[tid - 1];
        if (res) { // ignore deleted tuples
          res->IncReference();
          return res;
        }
      }
    }
    return 0;
  }
  
  size_t getNoDistFunCalls() {
    return it->getNoDistFunCalls();
  }
  
  NTreeStat getStat() const {
    return it->getStat();
  }
     
 private:
  vector<Tuple*>* rel;
  RangeIteratorN<MTreeEntry<T>, Dist, variant>* it;
};

template<class K, class T, class R, int variant>
int mdistRangeNVMT(Word* args, Word& result, int message, Word& local,
                   Supplier s) {
  distRangeNInfo<K, StdDistComp<K>, variant>* li = 
                        (distRangeNInfo<K, StdDistComp<K>, variant>*)local.addr;
  switch (message) {
    case OPEN : {
      if (li) {
        delete li;
        local.addr = 0;
      }
      R* relN = (R*)args[1].addr;
      MemoryRelObject* rel = getMemRel(relN, nl->Second(qp->GetType(s)));
      if (!rel) {
        return 0;
      }
      CcReal* range = (CcReal*)args[3].addr;
      if (!range->IsDefined()) {
        return 0;
      }
      double r = range->GetValue();
      if (r < 0.0) {
        return 0;
      }
      T* treeN = (T*)args[0].addr;
      MemoryNtreeObject<K, StdDistComp<K>, variant>* n = 
                                                getNtreeX<T, K, variant>(treeN);
      if (!n) {
        return 0;
      }
      K* key = (K*)args[2].addr;
      local.addr = new distRangeNInfo<K, StdDistComp<K>, variant>(n, rel,key,r);
      return 0;
    }
    case REQUEST: {
      result.addr = li ? li->next() : 0;
      return result.addr ? YIELD : CANCEL;
    }
    case CLOSE : {
      if (li) {
        string prefix = "counterMDistRangeN" + 
                        (variant > 1 ? to_string(variant) : "");        
        NTreeStat stat = li->getStat();
        mtreehelper::increaseCounter(prefix,
                                     stat.noDCInnerNodes + stat.noDCLeaves);
//         string counterINName = prefix + "InnerNodes";
//         mtreehelper::increaseCounter(counterINName, stat.noDCInnerNodes);
//         string counterLName = prefix + "Leaves";
//         mtreehelper::increaseCounter(counterLName, stat.noDCLeaves);
        mtreehelper::increaseCounter("noVisitedInnerNodes", stat.noInnerNodes);
        mtreehelper::increaseCounter("noVisitedLeaves", stat.noLeaves);
        delete li;
        local.addr = 0;
      }
      return 0;
    }
   }
   return -1;
}

int mdistRangeNSelect(ListExpr args) {
  int o1 = -1;
  int o2 = -1;
  ListExpr a1 = nl->First(args);
  ListExpr a2 = nl->Second(args);
  if (nl->HasLength(args, 4)) {
    ListExpr typeL = nl->Third(args);
    int type =  mtreehelper::getTypeNo(typeL,12);
    if(Mem::checkType(a1)) o1=0;
    if(MPointer::checkType(a1)) o1=24;
    if(o1<0) return -1; 

    if(Mem::checkType(a2)) o2=0;
    if(MPointer::checkType(a2)) o2=12;
    if(o2<0) return -1; 
   
    int res =  type + o1 + o2;
    return res;
  }
  return -1;
//   else { TODO!
//     ListExpr typeList1 = nl->Third(args);
//     ListExpr typeList2 = nl->Fourth(args);
//     int type1 = mtreehelper::getTypeNo(typeList1, 12) - 9;
//     int type2 = stj::getTypeNo(typeList2);
//     if (Mem::checkType(a1)) {
//       o1 = 48;
//     }
//     if (MPointer::checkType(a1)) {
//       o1 = 72;
//     }
//     if (o1 < 0) {
//       return -1;
//     }
// 
//     if (Mem::checkType(a2)) {
//       o2 = 0;
//     }
//     if (MPointer::checkType(a2)) {
//       o2 = 12;
//     }
//     if (o2 < 0) {
//       return -1;
//     }
//     int res = type1 + 3 * type2 + o1 + o2;
//     return res;
//   }
}

template<int variant>
ValueMapping mdistRangeNVM[] = {
  mdistRangeNVMT<mtreehelper::t1,Mem,Mem,variant>,
  mdistRangeNVMT<mtreehelper::t2,Mem,Mem,variant>,
  mdistRangeNVMT<mtreehelper::t3,Mem,Mem,variant>,
  mdistRangeNVMT<mtreehelper::t4,Mem,Mem,variant>,
  mdistRangeNVMT<mtreehelper::t5,Mem,Mem,variant>,
  mdistRangeNVMT<mtreehelper::t6,Mem,Mem,variant>,
  mdistRangeNVMT<mtreehelper::t7,Mem,Mem,variant>,
  mdistRangeNVMT<mtreehelper::t8,Mem,Mem,variant>,
  mdistRangeNVMT<mtreehelper::t9,Mem,Mem,variant>,
  mdistRangeNVMT<mtreehelper::t10,Mem,Mem,variant>,
  mdistRangeNVMT<mtreehelper::t11,Mem,Mem,variant>,
  mdistRangeNVMT<mtreehelper::t12,Mem,Mem,variant>,
  
  mdistRangeNVMT<mtreehelper::t1,Mem,MPointer,variant>,
  mdistRangeNVMT<mtreehelper::t2,Mem,MPointer,variant>,
  mdistRangeNVMT<mtreehelper::t3,Mem,MPointer,variant>,
  mdistRangeNVMT<mtreehelper::t4,Mem,MPointer,variant>,
  mdistRangeNVMT<mtreehelper::t5,Mem,MPointer,variant>,
  mdistRangeNVMT<mtreehelper::t6,Mem,MPointer,variant>,
  mdistRangeNVMT<mtreehelper::t7,Mem,MPointer,variant>,
  mdistRangeNVMT<mtreehelper::t8,Mem,MPointer,variant>,
  mdistRangeNVMT<mtreehelper::t9,Mem,MPointer,variant>,
  mdistRangeNVMT<mtreehelper::t10,Mem,MPointer,variant>,
  mdistRangeNVMT<mtreehelper::t11,Mem,MPointer,variant>,
  mdistRangeNVMT<mtreehelper::t12,Mem,MPointer,variant>,
  
  mdistRangeNVMT<mtreehelper::t1,MPointer,Mem,variant>,
  mdistRangeNVMT<mtreehelper::t2,MPointer,Mem,variant>,
  mdistRangeNVMT<mtreehelper::t3,MPointer,Mem,variant>,
  mdistRangeNVMT<mtreehelper::t4,MPointer,Mem,variant>,
  mdistRangeNVMT<mtreehelper::t5,MPointer,Mem,variant>,
  mdistRangeNVMT<mtreehelper::t6,MPointer,Mem,variant>,
  mdistRangeNVMT<mtreehelper::t7,MPointer,Mem,variant>,
  mdistRangeNVMT<mtreehelper::t8,MPointer,Mem,variant>,
  mdistRangeNVMT<mtreehelper::t9,MPointer,Mem,variant>,
  mdistRangeNVMT<mtreehelper::t10,MPointer,Mem,variant>,
  mdistRangeNVMT<mtreehelper::t11,MPointer,Mem,variant>,
  mdistRangeNVMT<mtreehelper::t12,MPointer,Mem,variant>,
  
  mdistRangeNVMT<mtreehelper::t1,MPointer,MPointer,variant>,
  mdistRangeNVMT<mtreehelper::t2,MPointer,MPointer,variant>,
  mdistRangeNVMT<mtreehelper::t3,MPointer,MPointer,variant>,
  mdistRangeNVMT<mtreehelper::t4,MPointer,MPointer,variant>,
  mdistRangeNVMT<mtreehelper::t5,MPointer,MPointer,variant>,
  mdistRangeNVMT<mtreehelper::t6,MPointer,MPointer,variant>,
  mdistRangeNVMT<mtreehelper::t7,MPointer,MPointer,variant>,
  mdistRangeNVMT<mtreehelper::t8,MPointer,MPointer,variant>,
  mdistRangeNVMT<mtreehelper::t9,MPointer,MPointer,variant>,
  mdistRangeNVMT<mtreehelper::t10,MPointer,MPointer,variant>,
  mdistRangeNVMT<mtreehelper::t11,MPointer,MPointer,variant>,
  mdistRangeNVMT<mtreehelper::t12,MPointer,MPointer,variant>
};

OperatorSpec mdistRangeNSpec(
  "NTREE x MREL  x T (x U) x real -> stream(tuple) , NTREE, "
  "MREL represented as string, mem, or mpointer",
  "mem_ntree mem_rel mdistRangeN[keyAttr, maxDist] ",
  "Retrieves those tuples from a memory relation "
  "having a distance smaller or equals to a given dist "
  "to a key value (or pair of key values). This operation is aided by a memory "
  "based n-tree.",
  "query mkinos_ntree mKinos mdistRangeN[alexanderplatz, 2000.0] count"
);

Operator mdistRangeNOp(
   "mdistRangeN",
   mdistRangeNSpec.getStr(),
   48,
   mdistRangeNVM<1>,
   mdistRangeNSelect,
   mdistRangeNTM<1>
);

/*
Operator ~mdistRangeN2~

*/

OperatorSpec mdistRangeN2Spec(
  "NTREE2 x MREL  x T (x U) x real -> stream(tuple) , NTREE2, "
  "MREL represented as string, mem, or mpointer",
  "mem_ntree2 mem_rel mdistRangeN2[keyAttr, maxDist] ",
  "Retrieves those tuples from a memory relation "
  "having a distance smaller or equal to a given distance "
  "to a key value (or pair of key values). This operation is aided by a memory "
  "based ntree2.",
  "query mkinos_ntree2 mKinos mdistRangeN2[alexanderplatz, 2000.0] count"
);

Operator mdistRangeN2Op(
   "mdistRangeN2",
   mdistRangeN2Spec.getStr(),
   48,
   mdistRangeNVM<2>,
   mdistRangeNSelect,
   mdistRangeNTM<2>
);

/*
Operator ~mdistRangeN5~

*/
OperatorSpec mdistRangeN5Spec(
  "NTREE5 x MREL  x T (x U) x real -> stream(tuple) , NTREE2, "
  "MREL represented as string, mem, or mpointer",
  "mem_ntree5 mem_rel mdistRangeN5[keyAttr, maxDist] ",
  "Retrieves those tuples from a memory relation "
  "having a distance smaller or equal to a given distance "
  "to a key value (or pair of key values). This operation is aided by a memory "
  "based ntree5.",
  "query mkinos_ntree5 mKinos mdistRangeN5[alexanderplatz, 2000.0] count"
);

Operator mdistRangeN5Op(
   "mdistRangeN5",
   mdistRangeN5Spec.getStr(),
   48,
   mdistRangeNVM<5>,
   mdistRangeNSelect,
   mdistRangeNTM<5>
);

/*
Operator ~mdistRangeN6~

*/
OperatorSpec mdistRangeN6Spec(
  "NTREE6 x MREL  x T (x U) x real -> stream(tuple) , NTREE6, "
  "MREL represented as string, mem, or mpointer",
  "mem_ntree2 mem_rel mdistRangeN6[keyAttr, maxDist] ",
  "Retrieves those tuples from a memory relation "
  "having a distance smaller or equal to a given distance "
  "to a key value (or pair of key values). This operation is aided by a memory "
  "based ntree6.",
  "query mkinos_ntree6 mKinos mdistRangeN6[alexanderplatz, 2000.0] count"
);

Operator mdistRangeN6Op(
   "mdistRangeN6",
   mdistRangeN6Spec.getStr(),
   48,
   mdistRangeNVM<6>,
   mdistRangeNSelect,
   mdistRangeNTM<6>
);

/*
Operator ~mdistRangeN7~

*/
OperatorSpec mdistRangeN7Spec(
  "NTREE7 x MREL  x T (x U) x real -> stream(tuple) , NTREE7, "
  "MREL represented as string, mem, or mpointer",
  "mem_ntree7 mem_rel mdistRangeN7[keyAttr, maxDist] ",
  "Retrieves those tuples from a memory relation "
  "having a distance smaller or equal to a given distance "
  "to a key value (or pair of key values). This operation is aided by a memory "
  "based ntree7.",
  "query mkinos_ntree7 mKinos mdistRangeN7[alexanderplatz, 2000.0] count"
);

Operator mdistRangeN7Op(
   "mdistRangeN7",
   mdistRangeN7Spec.getStr(),
   48,
   mdistRangeNVM<7>,
   mdistRangeNSelect,
   mdistRangeNTM<7>
);

/*
Operator ~mdistRangeN8~

*/
OperatorSpec mdistRangeN8Spec(
  "NTREE8 x MREL  x T (x U) x real -> stream(tuple) , NTREE8, "
  "MREL represented as string, mem, or mpointer",
  "mem_ntree8 mem_rel mdistRangeN8[keyAttr, maxDist] ",
  "Retrieves those tuples from a memory relation "
  "having a distance smaller or equal to a given distance "
  "to a key value (or pair of key values). This operation is aided by a memory "
  "based ntree8.",
  "query mkinos_ntree8 mKinos mdistRangeN8[alexanderplatz, 2000.0] count"
);

Operator mdistRangeN8Op(
   "mdistRangeN8",
   mdistRangeN8Spec.getStr(),
   48,
   mdistRangeNVM<8>,
   mdistRangeNSelect,
   mdistRangeNTM<8>
);

/*
Operator ~mnearestNeighborN7~, ~mnearestNeighborN8~

*/
template<int variant>
ListExpr mnearestNeighborNTM(ListExpr args) {
  string err = "NTREE(T) x MREL x T (x U) expected";
  if (!nl->HasLength(args, 3) && !nl->HasLength(args, 4)) {
    return listutils::typeError(err + " (wrong number of args)");
  }
  ListExpr a1 = nl->First(args);
  ListExpr a2 = nl->Second(args);
  if (MPointer::checkType(a1)) { 
    a1 = nl->Second(a1);
  }
  if (!Mem::checkType(a1)) {
    return listutils::typeError("first arg is not a memory object");
  }
  if (MPointer::checkType(a2)) {
    a2 = nl->Second(a2);
  }
  if (!Mem::checkType(a2)) {
    return listutils::typeError("2nd arg is not a memory object");
  }
  a1 = nl->Second(a1);
  a2 = nl->Second(a2);
  if (!Relation::checkType(a2)) {
    return listutils::typeError("second arg is not a relation");
  }
  ListExpr a3 = nl->Third(args);
  string treeType = "ntree" + to_string(variant);
  if (nl->HasLength(args, 3)) {
    if (!mtreehelper::checkTypeN(a1, a3, variant)) {
      return listutils::typeError("first arg is not an " + treeType + " over " +
                                  nl->ToString(a3));
    }
  }
  else {
    ListExpr a4 = nl->Fourth(args);
    if (!mtreehelper::checkTypeN(a1, nl->SymbolAtom(Tuple::BasicType()), 
                                 variant)) {
      return listutils::typeError("first arg is not an " + treeType +
                                  " over tuples");
    }
    if (!temporalalgebra::MPoint::checkType(a3) &&
        !temporalalgebra::CUPoint::checkType(a3) &&
        !temporalalgebra::CMPoint::checkType(a3)) {
      return listutils::typeError("third arg is not a (m|cu|cm)point");
    }
    if (!stj::isSymbolicType(a4)) {
      return listutils::typeError("fourth arg is not an mlabel(s) / mplace(s)");
    }
  }
  // copy attribute list and append distance attribute
  set<string> attrNames;
  ListExpr attrList = nl->Second(nl->Second(a2));
  ListExpr newAttrList = nl->OneElemList(nl->First(attrList));
  attrNames.insert(nl->SymbolValue(nl->First(nl->First(attrList))));
  ListExpr lastList = newAttrList;
  attrList = nl->Rest(attrList);
  while (!nl->IsEmpty(attrList)) {
    attrNames.insert(nl->SymbolValue(nl->First(nl->First(attrList))));
    lastList = nl->Append(lastList, nl->First(attrList));
    attrList = nl->Rest(attrList);
  }
  string distAttrName = "QueryObjectDistance";
  bool isPresent = (attrNames.find(distAttrName) != attrNames.end());
  int suffix = 0;
  while (isPresent) {
    suffix++;
    distAttrName += to_string(suffix);
    isPresent = (attrNames.find(distAttrName) != attrNames.end());
  }
  lastList = nl->Append(lastList, nl->TwoElemList(nl->SymbolAtom(distAttrName),
                                        nl->SymbolAtom(CcReal::BasicType())));
  return nl->TwoElemList(listutils::basicSymbol<Stream<Tuple> >(),
              nl->TwoElemList(nl->SymbolAtom(Tuple::BasicType()), newAttrList));
}

template<class T, class DistComp, int variant>
class mnearestNeighborNInfo {
 public:
  mnearestNeighborNInfo(MemoryNtreeObject<T, DistComp, variant>* ntreeX,
                        MemoryRelObject* mrel, T* ref, ListExpr typeList, 
                        const int _k = 0) : tupleTypeList(typeList), k(_k) {
    rel = mrel->getmmrel();
    MTreeEntry<T> p(*ref, 0);
    it = ntreeX->getNtreeX()->nnSearch(p, k);
    sc = SecondoSystem::GetCatalog();
    numTupleTypeList = sc->NumericType(tupleTypeList);
  }

  ~mnearestNeighborNInfo() {
    delete it;
  }

  Tuple* next() {
    TidDist td(0, -1.0);
    while (true) {
      td = it->next();
      if (((int)td.tid) == -1) {
        return 0;
      }
      if (td.dist < 0.0) {
        return 0;
      }
      if (td.tid <= rel->size()) {
        Tuple* src = (*rel)[td.tid - 1];
        if (src) { // ignore deleted tuples
          Tuple* res = new Tuple(numTupleTypeList);
          for (int i = 0; i < src->GetNoAttributes(); i++) {
            res->CopyAttribute(i, src, i);
          }
          res->PutAttribute(res->GetNoAttributes() - 1,
                            new CcReal(true, td.dist));
          return res;
        }
      }
    }
    return 0;
  }
  
  NTreeStat getStat() {
    return it->getStat();
  }
  
  int getNoDistFunCalls() {
    return it->getNoDistFunCalls();
  }
  
  int getNoDistFunCallsInnerNodes() {
    return it->getNoDistFunCallsInnerNodes();
  }
  
  int getNoDistFunCallsLeaves() {
    return it->getNoDistFunCallsLeaves();
  }


 private:
  vector<Tuple*>* rel;
  NNIteratorN<MTreeEntry<T>, DistComp, variant>* it;
  ListExpr tupleTypeList, numTupleTypeList;
  int k;
  SecondoCatalog *sc;
};

template<class K, class T, class R, int variant, int k>
int mnearestNeighborNVMT(Word* args, Word& result, int message, Word& local,
                         Supplier s) {
  mnearestNeighborNInfo<K, StdDistComp<K>, variant>* li = 
                 (mnearestNeighborNInfo<K, StdDistComp<K>, variant>*)local.addr;
  switch (message) {
    case OPEN : {
      if (li) {
        delete li;
        local.addr = 0;
      }
      R* relN = (R*)args[1].addr;
      MemoryRelObject* rel = getMemRel(relN, nl->Second(qp->GetType(s)));
      if (!rel) {
        return 0;
      }
      T* treeN = (T*)args[0].addr;
      MemoryNtreeObject<K, StdDistComp<K>, variant>* n = 
                                                getNtreeX<T, K, variant>(treeN);
      if (!n) {
        return 0;
      }
      K* key = (K*)args[2].addr;
      local.addr = new mnearestNeighborNInfo<K, StdDistComp<K>, variant>(n,
                           rel, key, nl->Second(qp->GetSupplierTypeExpr(s)), k);
      return 0;
    }
    case REQUEST: {
      result.addr = li ? li->next() : 0;
      return result.addr ? YIELD : CANCEL;
    }
    case CLOSE : {
      if (li) {
        string prefix = "counterM" + (k == 0 ? "" : to_string(k)) + 
                        "NearestNeighborN" + to_string(variant);
        mtreehelper::increaseCounter(prefix, li->getNoDistFunCalls());
//         string counterINName = prefix + "InnerNodes";
//         mtreehelper::increaseCounter(counterINName, 
//                                      li->getNoDistFunCallsInnerNodes());
//         string counterLName = prefix + "Leaves";
//         mtreehelper::increaseCounter(counterLName, 
//                                      li->getNoDistFunCallsLeaves());
        NTreeStat stat = li->getStat();
        mtreehelper::increaseCounter("noVisitedInnerNodes", stat.noInnerNodes);
        mtreehelper::increaseCounter("noVisitedLeaves", stat.noLeaves);
        delete li;
        local.addr = 0;
      }
      return 0;
    }
   }
   return -1;   
}

// TODO: adapt this function
// template<class K, class L, class T, class R>
// int mdistScanVMT2(Word* args, Word& result, int message, Word& local,
//                   Supplier s) {
//   distScanInfo<pair<K, L>, StdDistCompExt<K ,L> >* li = 
//               (distScanInfo<pair<K, L>, StdDistCompExt<K, L> >*) local.addr;
//   switch (message) {
//     case OPEN : {
//       if (li) {
//         delete li;
//         local.addr = 0;
//       }
//       R* relN = (R*) args[1].addr;
//       MemoryRelObject* mro;
//       if (R::requiresTypeCheckInVM()) {
//         mro = getMemRel(relN, nl->Second(qp->GetType(s)));
//       } else {
//         mro = getMemRel(relN);
//       }
//       if (!mro) {
//         return 0;
//       }
//       K* key1 = (K*)args[2].addr;
//       L* key2 = (L*)args[3].addr;
//       T* tree = (T*)args[0].addr;
//       pair<K, L> key(*key1, *key2);
//       MemoryMtreeObject<pair<K, L>, StdDistCompExt<K, L> >* m = 
//                                                     getMtree<T, K, L>(tree);
//       if (m) {
//         local.addr = new distScanInfo<pair<K, L>, StdDistCompExt<K, L> >(m,
//                                                                 mro, &key);
//       }
//       return 0;
//     }
//     case REQUEST : {
//       result.addr = li ? li->next() : 0;
//       return result.addr ? YIELD : CANCEL;
//     }
//     case CLOSE : {
//       if (li) {
//         mtreehelper::increaseCounter("counterMDistScan", 
//                                      li->getNoDistFunCalls());
//         delete li;
//         local.addr = 0;
//       }
//       return 0;
//     }
//   }
//   return -1;
// }

 // note: if adding attributes with flobs, the value mapping must be changed

template<int variant, int k>
ValueMapping mnearestNeighborNVM[] = {
  mnearestNeighborNVMT<mtreehelper::t1, Mem, Mem, variant, k>,
  mnearestNeighborNVMT<mtreehelper::t2, Mem, Mem, variant, k>,
  mnearestNeighborNVMT<mtreehelper::t3, Mem, Mem, variant, k>,
  mnearestNeighborNVMT<mtreehelper::t4, Mem, Mem, variant, k>,
  mnearestNeighborNVMT<mtreehelper::t5, Mem, Mem, variant, k>,
  mnearestNeighborNVMT<mtreehelper::t6, Mem, Mem, variant, k>,
  mnearestNeighborNVMT<mtreehelper::t7, Mem, Mem, variant, k>,
  mnearestNeighborNVMT<mtreehelper::t8, Mem, Mem, variant, k>,
  mnearestNeighborNVMT<mtreehelper::t9, Mem, Mem, variant, k>,
  mnearestNeighborNVMT<mtreehelper::t10, Mem, Mem, variant, k>,
  mnearestNeighborNVMT<mtreehelper::t11, Mem, Mem, variant, k>,
  mnearestNeighborNVMT<mtreehelper::t12, Mem, Mem, variant, k>,
  
  mnearestNeighborNVMT<mtreehelper::t1, Mem, MPointer, variant, k>,
  mnearestNeighborNVMT<mtreehelper::t2, Mem, MPointer, variant, k>,
  mnearestNeighborNVMT<mtreehelper::t3, Mem, MPointer, variant, k>,
  mnearestNeighborNVMT<mtreehelper::t4, Mem, MPointer, variant, k>,
  mnearestNeighborNVMT<mtreehelper::t5, Mem, MPointer, variant, k>,
  mnearestNeighborNVMT<mtreehelper::t6, Mem, MPointer, variant, k>,
  mnearestNeighborNVMT<mtreehelper::t7, Mem, MPointer, variant, k>,
  mnearestNeighborNVMT<mtreehelper::t8, Mem, MPointer, variant, k>,
  mnearestNeighborNVMT<mtreehelper::t9, Mem, MPointer, variant, k>,
  mnearestNeighborNVMT<mtreehelper::t10, Mem, MPointer, variant, k>,
  mnearestNeighborNVMT<mtreehelper::t11, Mem, MPointer, variant, k>,
  mnearestNeighborNVMT<mtreehelper::t12, Mem, MPointer, variant, k>,

  mnearestNeighborNVMT<mtreehelper::t1, MPointer, Mem, variant, k>,
  mnearestNeighborNVMT<mtreehelper::t2, MPointer, Mem, variant, k>,
  mnearestNeighborNVMT<mtreehelper::t3, MPointer, Mem, variant, k>,
  mnearestNeighborNVMT<mtreehelper::t4, MPointer, Mem, variant, k>,
  mnearestNeighborNVMT<mtreehelper::t5, MPointer, Mem, variant, k>,
  mnearestNeighborNVMT<mtreehelper::t6, MPointer, Mem, variant, k>,
  mnearestNeighborNVMT<mtreehelper::t7, MPointer, Mem, variant, k>,
  mnearestNeighborNVMT<mtreehelper::t8, MPointer, Mem, variant, k>,
  mnearestNeighborNVMT<mtreehelper::t9, MPointer, Mem, variant, k>,
  mnearestNeighborNVMT<mtreehelper::t10, MPointer, Mem, variant, k>,
  mnearestNeighborNVMT<mtreehelper::t11, MPointer, Mem, variant, k>,
  mnearestNeighborNVMT<mtreehelper::t12, MPointer, Mem, variant, k>,
  
  mnearestNeighborNVMT<mtreehelper::t1, MPointer, MPointer, variant, k>,
  mnearestNeighborNVMT<mtreehelper::t2, MPointer, MPointer, variant, k>,
  mnearestNeighborNVMT<mtreehelper::t3, MPointer, MPointer, variant, k>,
  mnearestNeighborNVMT<mtreehelper::t4, MPointer, MPointer, variant, k>,
  mnearestNeighborNVMT<mtreehelper::t5, MPointer, MPointer, variant, k>,
  mnearestNeighborNVMT<mtreehelper::t6, MPointer, MPointer, variant, k>,
  mnearestNeighborNVMT<mtreehelper::t7, MPointer, MPointer, variant, k>,
  mnearestNeighborNVMT<mtreehelper::t8, MPointer, MPointer, variant, k>,
  mnearestNeighborNVMT<mtreehelper::t9, MPointer, MPointer, variant, k>,
  mnearestNeighborNVMT<mtreehelper::t10, MPointer, MPointer, variant, k>,
  mnearestNeighborNVMT<mtreehelper::t11, MPointer, MPointer, variant, k>,
  mnearestNeighborNVMT<mtreehelper::t12, MPointer, MPointer, variant, k>/*,
  
  mnearestNeighborN7VMT2<MPoint, stj::MLabel, Mem, Mem>,
  mnearestNeighborN7VMT2<CUPoint, stj::MLabel, Mem, Mem>,
  mnearestNeighborN7VMT2<CMPoint, stj::MLabel, Mem, Mem>,
  mnearestNeighborN7VMT2<MPoint, stj::MLabels, Mem, Mem>,
  mnearestNeighborN7VMT2<CUPoint, stj::MLabels, Mem, Mem>,
  mnearestNeighborN7VMT2<CMPoint, stj::MLabels, Mem, Mem>,
  mnearestNeighborN7VMT2<MPoint, stj::MPlace, Mem, Mem>,
  mnearestNeighborN7VMT2<CUPoint, stj::MPlace, Mem, Mem>,
  mnearestNeighborN7VMT2<CMPoint, stj::MPlace, Mem, Mem>,
  mnearestNeighborN7VMT2<MPoint, stj::MPlaces, Mem, Mem>,
  mnearestNeighborN7VMT2<CUPoint, stj::MPlaces, Mem, Mem>,
  mnearestNeighborN7VMT2<CMPoint, stj::MPlaces, Mem, Mem>,
  
  mnearestNeighborN7VMT2<MPoint, stj::MLabel, Mem, MPointer>,
  mnearestNeighborN7VMT2<CUPoint, stj::MLabel, Mem, MPointer>,
  mnearestNeighborN7VMT2<CMPoint, stj::MLabel, Mem, MPointer>,
  mnearestNeighborN7VMT2<MPoint, stj::MLabels, Mem, MPointer>,
  mnearestNeighborN7VMT2<CUPoint, stj::MLabels, Mem, MPointer>,
  mnearestNeighborN7VMT2<CMPoint, stj::MLabels, Mem, MPointer>,
  mnearestNeighborN7VMT2<MPoint, stj::MPlace, Mem, MPointer>,
  mnearestNeighborN7VMT2<CUPoint, stj::MPlace, Mem, MPointer>,
  mnearestNeighborN7VMT2<CMPoint, stj::MPlace, Mem, MPointer>,
  mnearestNeighborN7VMT2<MPoint, stj::MPlaces, Mem, MPointer>,
  mnearestNeighborN7VMT2<CUPoint, stj::MPlaces, Mem, MPointer>,
  mnearestNeighborN7VMT2<CMPoint, stj::MPlaces, Mem, MPointer>,
  
  mnearestNeighborN7VMT2<MPoint, stj::MLabel, MPointer, Mem>,
  mnearestNeighborN7VMT2<CUPoint, stj::MLabel, MPointer, Mem>,
  mnearestNeighborN7VMT2<CMPoint, stj::MLabel, MPointer, Mem>,
  mnearestNeighborN7VMT2<MPoint, stj::MLabels, MPointer, Mem>,
  mnearestNeighborN7VMT2<CUPoint, stj::MLabels, MPointer, Mem>,
  mnearestNeighborN7VMT2<CMPoint, stj::MLabels, MPointer, Mem>,
  mnearestNeighborN7VMT2<MPoint, stj::MPlace, MPointer, Mem>,
  mnearestNeighborN7VMT2<CUPoint, stj::MPlace, MPointer, Mem>,
  mnearestNeighborN7VMT2<CMPoint, stj::MPlace, MPointer, Mem>,
  mnearestNeighborN7VMT2<MPoint, stj::MPlaces, MPointer, Mem>,
  mnearestNeighborN7VMT2<CUPoint, stj::MPlaces, MPointer, Mem>,
  mnearestNeighborN7VMT2<CMPoint, stj::MPlaces, MPointer, Mem>,
  
  mnearestNeighborN7VMT2<MPoint, stj::MLabel, MPointer, MPointer>,
  mnearestNeighborN7VMT2<CUPoint, stj::MLabel, MPointer, MPointer>,
  mnearestNeighborN7VMT2<CMPoint, stj::MLabel, MPointer, MPointer>,
  mnearestNeighborN7VMT2<MPoint, stj::MLabels, MPointer, MPointer>,
  mnearestNeighborN7VMT2<CUPoint, stj::MLabels, MPointer, MPointer>,
  mnearestNeighborN7VMT2<CMPoint, stj::MLabels, MPointer, MPointer>,
  mnearestNeighborN7VMT2<MPoint, stj::MPlace, MPointer, MPointer>,
  mnearestNeighborN7VMT2<CUPoint, stj::MPlace, MPointer, MPointer>,
  mnearestNeighborN7VMT2<CMPoint, stj::MPlace, MPointer, MPointer>,
  mnearestNeighborN7VMT2<MPoint, stj::MPlaces, MPointer, MPointer>,
  mnearestNeighborN7VMT2<CUPoint, stj::MPlaces, MPointer, MPointer>,
  mnearestNeighborN7VMT2<CMPoint, stj::MPlaces, MPointer, MPointer>*/
};

OperatorSpec mnearestNeighborN7Spec(
  "MTREE x MREL x T (x U) -> stream(tuple) , "
  "MTREE, MREL represented as string, mem, or mpointer",
  "mem_mtree mem_rel mnearestNeighborN7[keyAttr] ",
  "Sorts the relation by its distance to the reference object aided by an "
  "N-tree7. The original tuples are extended by an attribute containing the "
  "distance to the reference object.",
  "query mkinos_mtree mKinos mnearestNeighborN7[alexanderplatz] consume"
);

Operator mnearestNeighborN7Op(
   "mnearestNeighborN7",
   mnearestNeighborN7Spec.getStr(),
   96,
   mnearestNeighborNVM<7, 0>,
   mdistRangeScanSelect<3>,
   mnearestNeighborNTM<7>
);



OperatorSpec m1nearestNeighborN7Spec(
  "NTREE7 x MREL x T (x U) -> stream(tuple) , "
  "NTREE7, MREL represented as string, mem, or mpointer",
  "mem_ntree7 mem_rel m1nearestNeighborN7[keyAttr] ",
  "Retrieves the nearest neighbor to a reference object aided by an N-tree7. "
  "The original tuple is extended by an attribute containing the distance to "
  "the reference object.",
  "query mkinos_ntree7 mKinos m1nearestNeighborN7[alexanderplatz] consume"
);

Operator m1nearestNeighborN7Op(
   "m1nearestNeighborN7",
   m1nearestNeighborN7Spec.getStr(),
   96,
   mnearestNeighborNVM<7, 1>,
   mdistRangeScanSelect<3>,
   mnearestNeighborNTM<7>
);

OperatorSpec m1nearestNeighborN8Spec(
  "NTREE8 x MREL x T (x U) -> stream(tuple) , "
  "NTREE8, MREL represented as string, mem, or mpointer",
  "mem_ntree8 mem_rel m1nearestNeighborN8[keyAttr] ",
  "Retrieves the nearest neighbor to a reference object aided by an N-tree8. "
  "The original tuple is extended by an attribute containing the distance to "
  "the reference object.",
  "query mkinos_ntree8 mKinos m1nearestNeighborN8[alexanderplatz] consume"
);

Operator m1nearestNeighborN8Op(
   "m1nearestNeighborN8",
   m1nearestNeighborN8Spec.getStr(),
   96,
   mnearestNeighborNVM<8, 1>,
   mdistRangeScanSelect<3>,
   mnearestNeighborNTM<8>
);

/*
Operator ~mdistScan~

*/
ListExpr mdistScanTM(ListExpr args) {
  string err="MTREE(T) x MREL x T (x U) expected";
  if (!nl->HasLength(args, 3) && !nl->HasLength(args, 4)) {
    return listutils::typeError(err + " (wrong number of args)");
  }
  ListExpr a1 = nl->First(args);
  ListExpr a2 = nl->Second(args);
  if (MPointer::checkType(a1)) {
    a1 = nl->Second(a1);
  }
  if (!Mem::checkType(a1)) {
    return listutils::typeError("first arg is not a memory object");
  }
  if (MPointer::checkType(a2)) {
    a2 = nl->Second(a2);
  }
  if (!Mem::checkType(a2)) {
    return listutils::typeError("2nd arg is not a memory object");
  }
  a1 = nl->Second(a1);
  a2 = nl->Second(a2);
  if (!Relation::checkType(a2)) {
    return listutils::typeError("second arg is not a relation");
  }
  ListExpr a3 = nl->Third(args);
  if (nl->HasLength(args, 3)) {
    if (!mtreehelper::checkType(a1, a3)) {
      return listutils::typeError("first arg is not an mtree over " 
                                  + nl->ToString(a3));
    }
  }
  else {
    ListExpr a4 = nl->Fourth(args);
    if (!mtreehelper::checkType(a1, nl->SymbolAtom(Tuple::BasicType()))) {
      return listutils::typeError("first arg is not an mtree over tuples");
    }
    if (!temporalalgebra::MPoint::checkType(a3) &&
        !temporalalgebra::CUPoint::checkType(a3) &&
        !temporalalgebra::CMPoint::checkType(a3)) {
      return listutils::typeError("third arg is not a (m|cu|cm)point");
    }
    if (!stj::isSymbolicType(a4)) {
      return listutils::typeError("fourth arg is not an mlabel(s) / mplace(s)");
    }
  }
  return nl->TwoElemList(listutils::basicSymbol<Stream<Tuple> >(),
                         nl->Second(a2)); 
}

template<class T, class DistComp>
class distScanInfo {
  public:
    distScanInfo(MemoryMtreeObject<T, DistComp>* mtree, MemoryRelObject* mrel, 
                 T* ref) {                 
      rel = mrel->getmmrel();
      MTreeEntry<T> p(*ref, 0);
//       pair<T,TupleId> p(*ref,0);
      it = mtree->getmtree()->nnSearch(p);
    }

    ~distScanInfo() {
      delete it;
    }

    Tuple* next() {
      while(true) {
        const MTreeEntry<T>* p = it->next();
//           const pair<T,TupleId>* p = it->next();
        if (!p) {
          return 0;
        }
        if ((p->getTid() <= rel->size()) && (p->getTid() > 0)) {
          Tuple* res = (*rel)[p->getTid() - 1];
          if (res) { // ignore deleted tuples
            res->IncReference();
            return res;
          }
        }
      }
      return 0;
    }
    
    int getNoDistFunCalls() {
      return it->getNoDistFunCalls();
    }


  private:
    vector<Tuple*>* rel;
    NNIterator<MTreeEntry<T>, DistComp>* it;
//      NNIterator<pair<T,TupleId> , StdDistComp<T>,
//                 MemCloner<pair<T,TupleId> >  >* it;
};

template<class K, class T, class R>
int mdistScanVMT(Word* args, Word& result, int message, Word& local,
                 Supplier s) {
   distScanInfo<K, StdDistComp<K> >* li = 
                                 (distScanInfo<K, StdDistComp<K> >*) local.addr;
   switch(message) {
     case OPEN : {
        if(li){
          delete li;
          local.addr = 0;
        }
        R* relN = (R*) args[1].addr;
        MemoryRelObject* mro;
        if (R::requiresTypeCheckInVM()) {
          mro = getMemRel(relN, nl->Second(qp->GetType(s)));
        } else {
          mro = getMemRel(relN);
        }
        if (!mro) {
          return 0;
        }
        K* key = (K*)args[2].addr;
        T* tree = (T*)args[0].addr;
        MemoryMtreeObject<K, StdDistComp<K> >* m = getMtree<T, K>(tree);
        if (m) {
          local.addr = new distScanInfo<K, StdDistComp<K> >(m, mro, key);
        }
        return 0;
      }
      case REQUEST : {
        result.addr = li ? li->next() : 0;
        return result.addr ? YIELD : CANCEL;
      }
      case CLOSE : {
        if (li) {
          mtreehelper::increaseCounter("counterMDistScan", 
                                       li->getNoDistFunCalls());
          delete li;
          local.addr = 0;
        }
        return 0;
      }
   }
   return -1;
}

template<class K, class L, class T, class R>
int mdistScanVMT2(Word* args, Word& result, int message, Word& local,
                  Supplier s) {
  distScanInfo<pair<K, L>, StdDistCompExt<K ,L> >* li = 
                  (distScanInfo<pair<K, L>, StdDistCompExt<K, L> >*) local.addr;
  switch (message) {
    case OPEN : {
      if (li) {
        delete li;
        local.addr = 0;
      }
      R* relN = (R*) args[1].addr;
      MemoryRelObject* mro;
      if (R::requiresTypeCheckInVM()) {
        mro = getMemRel(relN, nl->Second(qp->GetType(s)));
      } else {
        mro = getMemRel(relN);
      }
      if (!mro) {
        return 0;
      }
      K* key1 = (K*)args[2].addr;
      L* key2 = (L*)args[3].addr;
      T* tree = (T*)args[0].addr;
      pair<K, L> key(*key1, *key2);
      MemoryMtreeObject<pair<K, L>, StdDistCompExt<K, L> >* m = 
                                                        getMtree<T, K, L>(tree);
      if (m) {
        local.addr = new distScanInfo<pair<K, L>, StdDistCompExt<K, L> >(m, mro,
                                                                         &key);
      }
      return 0;
    }
    case REQUEST : {
      result.addr = li ? li->next() : 0;
      return result.addr ? YIELD : CANCEL;
    }
    case CLOSE : {
      if (li) {
        mtreehelper::increaseCounter("counterMDistScan", 
                                     li->getNoDistFunCalls());
        delete li;
        local.addr = 0;
      }
      return 0;
    }
  }
  return -1;
}

 // note: if adding attributes with flobs, the value mapping must be changed

ValueMapping mdistScanVM[] = {
  mdistScanVMT<mtreehelper::t1, Mem, Mem>,
  mdistScanVMT<mtreehelper::t2, Mem, Mem>,
  mdistScanVMT<mtreehelper::t3, Mem, Mem>,
  mdistScanVMT<mtreehelper::t4, Mem, Mem>,
  mdistScanVMT<mtreehelper::t5, Mem, Mem>,
  mdistScanVMT<mtreehelper::t6, Mem, Mem>,
  mdistScanVMT<mtreehelper::t7, Mem, Mem>,
  mdistScanVMT<mtreehelper::t8, Mem, Mem>,
  mdistScanVMT<mtreehelper::t9, Mem, Mem>,
  mdistScanVMT<mtreehelper::t10, Mem, Mem>,
  mdistScanVMT<mtreehelper::t11, Mem, Mem>,
  mdistScanVMT<mtreehelper::t12, Mem, Mem>,
  
  mdistScanVMT<mtreehelper::t1, Mem, MPointer>,
  mdistScanVMT<mtreehelper::t2, Mem, MPointer>,
  mdistScanVMT<mtreehelper::t3, Mem, MPointer>,
  mdistScanVMT<mtreehelper::t4, Mem, MPointer>,
  mdistScanVMT<mtreehelper::t5, Mem, MPointer>,
  mdistScanVMT<mtreehelper::t6, Mem, MPointer>,
  mdistScanVMT<mtreehelper::t7, Mem, MPointer>,
  mdistScanVMT<mtreehelper::t8, Mem, MPointer>,
  mdistScanVMT<mtreehelper::t9, Mem, MPointer>,
  mdistScanVMT<mtreehelper::t10, Mem, MPointer>,
  mdistScanVMT<mtreehelper::t11, Mem, MPointer>,
  mdistScanVMT<mtreehelper::t12, Mem, MPointer>,

  mdistScanVMT<mtreehelper::t1, MPointer, Mem>,
  mdistScanVMT<mtreehelper::t2, MPointer, Mem>,
  mdistScanVMT<mtreehelper::t3, MPointer, Mem>,
  mdistScanVMT<mtreehelper::t4, MPointer, Mem>,
  mdistScanVMT<mtreehelper::t5, MPointer, Mem>,
  mdistScanVMT<mtreehelper::t6, MPointer, Mem>,
  mdistScanVMT<mtreehelper::t7, MPointer, Mem>,
  mdistScanVMT<mtreehelper::t8, MPointer, Mem>,
  mdistScanVMT<mtreehelper::t9, MPointer, Mem>,
  mdistScanVMT<mtreehelper::t10, MPointer, Mem>,
  mdistScanVMT<mtreehelper::t11, MPointer, Mem>,
  mdistScanVMT<mtreehelper::t12, MPointer, Mem>,
  
  mdistScanVMT<mtreehelper::t1, MPointer, MPointer>,
  mdistScanVMT<mtreehelper::t2, MPointer, MPointer>,
  mdistScanVMT<mtreehelper::t3, MPointer, MPointer>,
  mdistScanVMT<mtreehelper::t4, MPointer, MPointer>,
  mdistScanVMT<mtreehelper::t5, MPointer, MPointer>,
  mdistScanVMT<mtreehelper::t6, MPointer, MPointer>,
  mdistScanVMT<mtreehelper::t7, MPointer, MPointer>,
  mdistScanVMT<mtreehelper::t8, MPointer, MPointer>,
  mdistScanVMT<mtreehelper::t9, MPointer, MPointer>,
  mdistScanVMT<mtreehelper::t10, MPointer, MPointer>,
  mdistScanVMT<mtreehelper::t11, MPointer, MPointer>,
  mdistScanVMT<mtreehelper::t12, MPointer, MPointer>,
  
  mdistScanVMT2<temporalalgebra::MPoint, stj::MLabel, Mem, Mem>,
  mdistScanVMT2<temporalalgebra::CUPoint, stj::MLabel, Mem, Mem>,
  mdistScanVMT2<temporalalgebra::CMPoint, stj::MLabel, Mem, Mem>,
  mdistScanVMT2<temporalalgebra::MPoint, stj::MLabels, Mem, Mem>,
  mdistScanVMT2<temporalalgebra::CUPoint, stj::MLabels, Mem, Mem>,
  mdistScanVMT2<temporalalgebra::CMPoint, stj::MLabels, Mem, Mem>,
  mdistScanVMT2<temporalalgebra::MPoint, stj::MPlace, Mem, Mem>,
  mdistScanVMT2<temporalalgebra::CUPoint, stj::MPlace, Mem, Mem>,
  mdistScanVMT2<temporalalgebra::CMPoint, stj::MPlace, Mem, Mem>,
  mdistScanVMT2<temporalalgebra::MPoint, stj::MPlaces, Mem, Mem>,
  mdistScanVMT2<temporalalgebra::CUPoint, stj::MPlaces, Mem, Mem>,
  mdistScanVMT2<temporalalgebra::CMPoint, stj::MPlaces, Mem, Mem>,
  
  mdistScanVMT2<temporalalgebra::MPoint, stj::MLabel, Mem, MPointer>,
  mdistScanVMT2<temporalalgebra::CUPoint, stj::MLabel, Mem, MPointer>,
  mdistScanVMT2<temporalalgebra::CMPoint, stj::MLabel, Mem, MPointer>,
  mdistScanVMT2<temporalalgebra::MPoint, stj::MLabels, Mem, MPointer>,
  mdistScanVMT2<temporalalgebra::CUPoint, stj::MLabels, Mem, MPointer>,
  mdistScanVMT2<temporalalgebra::CMPoint, stj::MLabels, Mem, MPointer>,
  mdistScanVMT2<temporalalgebra::MPoint, stj::MPlace, Mem, MPointer>,
  mdistScanVMT2<temporalalgebra::CUPoint, stj::MPlace, Mem, MPointer>,
  mdistScanVMT2<temporalalgebra::CMPoint, stj::MPlace, Mem, MPointer>,
  mdistScanVMT2<temporalalgebra::MPoint, stj::MPlaces, Mem, MPointer>,
  mdistScanVMT2<temporalalgebra::CUPoint, stj::MPlaces, Mem, MPointer>,
  mdistScanVMT2<temporalalgebra::CMPoint, stj::MPlaces, Mem, MPointer>,
  
  mdistScanVMT2<temporalalgebra::MPoint, stj::MLabel, MPointer, Mem>,
  mdistScanVMT2<temporalalgebra::CUPoint, stj::MLabel, MPointer, Mem>,
  mdistScanVMT2<temporalalgebra::CMPoint, stj::MLabel, MPointer, Mem>,
  mdistScanVMT2<temporalalgebra::MPoint, stj::MLabels, MPointer, Mem>,
  mdistScanVMT2<temporalalgebra::CUPoint, stj::MLabels, MPointer, Mem>,
  mdistScanVMT2<temporalalgebra::CMPoint, stj::MLabels, MPointer, Mem>,
  mdistScanVMT2<temporalalgebra::MPoint, stj::MPlace, MPointer, Mem>,
  mdistScanVMT2<temporalalgebra::CUPoint, stj::MPlace, MPointer, Mem>,
  mdistScanVMT2<temporalalgebra::CMPoint, stj::MPlace, MPointer, Mem>,
  mdistScanVMT2<temporalalgebra::MPoint, stj::MPlaces, MPointer, Mem>,
  mdistScanVMT2<temporalalgebra::CUPoint, stj::MPlaces, MPointer, Mem>,
  mdistScanVMT2<temporalalgebra::CMPoint, stj::MPlaces, MPointer, Mem>,
  
  mdistScanVMT2<temporalalgebra::MPoint, stj::MLabel, MPointer, MPointer>,
  mdistScanVMT2<temporalalgebra::CUPoint, stj::MLabel, MPointer, MPointer>,
  mdistScanVMT2<temporalalgebra::CMPoint, stj::MLabel, MPointer, MPointer>,
  mdistScanVMT2<temporalalgebra::MPoint, stj::MLabels, MPointer, MPointer>,
  mdistScanVMT2<temporalalgebra::CUPoint, stj::MLabels, MPointer, MPointer>,
  mdistScanVMT2<temporalalgebra::CMPoint, stj::MLabels, MPointer, MPointer>,
  mdistScanVMT2<temporalalgebra::MPoint, stj::MPlace, MPointer, MPointer>,
  mdistScanVMT2<temporalalgebra::CUPoint, stj::MPlace, MPointer, MPointer>,
  mdistScanVMT2<temporalalgebra::CMPoint, stj::MPlace, MPointer, MPointer>,
  mdistScanVMT2<temporalalgebra::MPoint, stj::MPlaces, MPointer, MPointer>,
  mdistScanVMT2<temporalalgebra::CUPoint, stj::MPlaces, MPointer, MPointer>,
  mdistScanVMT2<temporalalgebra::CMPoint, stj::MPlaces, MPointer, MPointer>
};

OperatorSpec mdistScanSpec(
  "MTREE x MREL x T (x U) -> stream(tuple) , "
  "MTREE, MREL represented as string, mem, or mpointer",
  "mem_mtree mem_rel mdistScan[keyAttr] ",
  "Retrieves tuples from an memory relation in increasing "
  "distance to a reference object (or pair of reference objects) aided by a "
  "memory based m-tree.",
  "query mkinos_mtree mKinos mdistScan[ alexanderplatz] consume"
);

Operator mdistScanOp(
   "mdistScan",
   mdistScanSpec.getStr(),
   96,
   mdistScanVM,
   mdistRangeScanSelect<3>,
   mdistScanTM
);




////////////////////////////////////////////////////////////////////////
// NEW
///////////////////////////////////////////////////////////////////////




/*

Operators of MainMemory2Algebra

7.1 Operator ~mwrap~

Converts a string into an mem(X) according to the
stored type in the memory catalog

7.1.1 Type Mapping Functions of operator ~mwrap~
    string -> mem(X)

*/
ListExpr mwrapTM(ListExpr args){
   if(!nl->HasLength(args,1)){
      return listutils::typeError("one argument expected");
   }
   ListExpr first = nl->First(args);
   if(!nl->HasLength(first,2)) {
      return listutils::typeError("internal error");
   }
   ListExpr type = nl->First(first);
   ListExpr value = nl->Second(first);
   if(!CcString::checkType(type)){
      return listutils::typeError("string expected");
   }
   ListExpr res;
   string error;
   if(!getMemTypeFromString(type, value,res, error, true)){
     return listutils::typeError(error);
   }
   return res;
}

/*

7.1.2 Value Mapping Function of operator ~mwrap~

*/
template<int strpos>
int mwrapVM(Word* args, Word& result,
             int message, Word& local, Supplier s) {
   CcString* arg = (CcString*) args[strpos].addr;
   result = qp->ResultStorage(s);
   Mem* res = (Mem*) result.addr;
   bool def = arg->IsDefined();
   string v = def?arg->GetValue():"";
   res->set(def,v);
   return 0;
}

/*

7.1.3 Description of operator ~mwrap~

*/
OperatorSpec mwrapSpec(
    "string -> mem(X)",
    "mwrap(_)",
    "Converts a string into a mem object"
    " according to the type from the memory catalog",
    "query mwrap(\"ten\")"
);

/*

7.1.4 Instance of operator ~mwrap~

*/
Operator mwrapOp(
   "mwrap",
   mwrapSpec.getStr(),
   mwrapVM<0>,
   Operator::SimpleSelect,
   mwrapTM
);


/*
Operator ~mwrap2~

This operator may create an mem(x) object without accessing the
memory catalog to get the type. Instead of this, the type is
given explicitely in the first argument. The type must be a 
constant text but the string may be any expression that evaluates
to string. The value mapping checks the equqlity between the
given type and the actual type. If not, the mem object will be
undefined.

*/
ListExpr mwrap2TM(ListExpr args){
  if(!nl->HasLength(args,2)){
    return listutils::typeError("2 args expected");
  }
  if(!checkUsesArgs(args)){
    return listutils::typeError("internal error");
  }
  if(!FText::checkType(nl->First(nl->First(args)))){
    return listutils::typeError("first arg is not a text");
  }
  if(!CcString::checkType(nl->First(nl->Second(args)))){
    return listutils::typeError("second arg is not a string");
  }
  ListExpr typeList = nl->Second(nl->First(args));
  if(nl->AtomType(typeList)!=TextType){
    return listutils::typeError("the text is not constant");
  }
  string type = nl->Text2String(typeList);
  if(!nl->ReadFromString(type,typeList)){
   return listutils::typeError("the type is not a valid nested list");
  }
  ListExpr res = nl->TwoElemList(
            listutils::basicSymbol<Mem>(),
            typeList
         );
  if(!Mem::checkType(res)){
    return listutils::typeError("type is not a valid mem type");
  }
  return res;
}

OperatorSpec mwrap2Spec(
  "text x string -> mem(X)",
  "mwrap2(_,_)",
  "Creates a mem object whose type is given "
  "as a text in nested list format",
  "query mwrap2('string', \"myName\")" 
);

Operator mwrap2Op(
  "mwrap2",
  mwrap2Spec.getStr(),
  mwrapVM<1>,
  Operator::SimpleSelect,
  mwrap2TM
);


/*
Another variant determining the subtype from an expression.

*/
ListExpr mwrap3TM(ListExpr args){
  if(!nl->HasLength(args,2)){
    return listutils::typeError("two args expected");
  }
  if(!CcString::checkType(nl->Second(args))){
    return listutils::typeError("second arg is not a string");
  }
  ListExpr res =  nl->TwoElemList(
                    listutils::basicSymbol<Mem>(),
                    nl->First(args));
  if(!Mem::checkType(res)){
    return listutils::typeError("first arg is not a valid mem subtype");
  }
  return res;
}


OperatorSpec mwrap3Spec(
  "X x string -> mem(X)",
  "mwrap3(_,_)",
  "Creates a mem object whose type is given "
  "by the first argument. The value of the first "
  "argument is not used",
  "query mwrap2('string', \"myName\")" 
);

Operator mwrap3Op(
  "mwrap3",
  mwrap3Spec.getStr(),
  mwrapVM<1>,
  Operator::SimpleSelect,
  mwrap3TM
);




template<int pos>
ListExpr MTUPLETM(ListExpr args){

   if(!nl->HasMinLength(args,pos)){
      return listutils::typeError("too less arguments, expected "
               + stringutils::int2str(pos) + " but got " 
               + stringutils::int2str(nl->ListLength(args)));
   }
   // remove elements before pos
   for(int i=1; i<pos;i++){
     args = nl->Rest(args);
   }
   ListExpr subtype;
   if(!getMemSubType(nl->First(args), subtype)) {
    return listutils::typeError("the specified argument ist not "
                                "a memory object");
   }
   ListExpr res;
   if(nl->HasMinLength(subtype,2)){
     res = nl->Second(subtype); // enclosed tuple: stream, rel, orel ...
   } else {
     return listutils::typeError("unsupported subtype");
   }
   if(!Tuple::checkType(res)){
     return listutils::typeError("no tuple found");
   }
   return res;
}

OperatorSpec MTUPLESpec(
   "... MREL  x ... -> X or string -> tuple, MREL in {tring,mem,mpointer} ",
   "MTUPLE<X>(_)",
   "Retrieves the tuple type of a memory relation at position "
   "<X> in the argument list",
   "query mten mupdatebyid[[const tid value 5]; No: .No + 10000] count"
);

Operator MTUPLEOp(
    "MTUPLE",
    MTUPLESpec.getStr(),
    0,
    Operator::SimpleSelect,
    MTUPLETM<1>
);

Operator MTUPLE2Op(
    "MTUPLE2",
    MTUPLESpec.getStr(),
    0,
    Operator::SimpleSelect,
    MTUPLETM<2>
);




/*
7.2 Operator ~mcreatettree~

The operator creates a TTree over a given main memory relation.

7.2.1 Type Mapping Functions of operator ~mcreatettree~
        {mpointer , mem(rel(...))} x IDENT -> mpointer(mem(ttree X)) 

        the first parameter identifies the main memory relation, 
        the second parameter identifies the attribute

*/
ListExpr mcreatettreeTypeMap(ListExpr args){
  
    if(!nl->HasLength(args,2)){
     return listutils::typeError("two arguments expected");
    }

    ListExpr second = nl->Second(args);
    if(nl->AtomType(second)!=SymbolType){
       return listutils::typeError("second argument is not a valid "
                                   "attribute name");
    } 
    ListExpr first = nl->First(args);
    if(MPointer::checkType(first)){
       first = nl->Second(first); // just remove the mpointer
    }
    if(!Mem::checkType(first)){
      return listutils::typeError("first argument not of type mpointer "
                                  "or mem");
    }
    ListExpr rel = nl->Second(first); // remove mem
    if(!Relation::checkType(rel)){
       return listutils::typeError("first argument is not a memory relation");
    }
    ListExpr attrList = nl->Second(nl->Second(rel));
    string aname = nl->SymbolValue(second);
    ListExpr attrType;
    int index = listutils::findAttribute(attrList, aname, attrType);
    if(!index){
      return listutils::typeError("attribute " + aname 
                                  + " not part of the relation");
    }
    ListExpr resType = MPointer::wrapType(Mem::wrapType(
                          MemoryTTreeObject::wrapType(attrType)));
    return nl->ThreeElemList( nl->SymbolAtom(Symbols::APPEND()),
                              nl->OneElemList( nl->IntAtom(index-1)),
                              resType);
     
}

MemoryTTreeObject*  createMTTreeOver(MemoryRelObject* mmrel,int  attrPos,
                                     ListExpr resType) {

  vector<Tuple*>* relation = mmrel->getmmrel();
  vector<Tuple*>::iterator it = relation->begin();
  unsigned int i = 1;

  memttree* tree = new memttree(8,10);
  size_t usedMainMemory = 0;
  unsigned long availableMemSize = catalog->getAvailableMemSize();
  while(it != relation->end()) {
      Tuple* tup = *it;
      if(tup){
         Attribute* attr = tup->GetAttribute(attrPos);
         AttrIdPair tPair(attr, i);

         // size for a pair is 16 bytes, plus an additional pointer 8 bytes
         size_t entrySize = 24;
         if (entrySize<availableMemSize){
            tree->insert(tPair);
            usedMainMemory += (entrySize);
            availableMemSize -= (entrySize);
         } else {
           cout << "there is not enough main memory available"
                   " to create an TTree" << endl;
           delete tree;
           return 0;
         }
      }
      i++;
      it++;
  }
  MemoryTTreeObject* ttreeObject = 
        new MemoryTTreeObject(tree, 
                              usedMainMemory,
                              nl->ToString(resType),
                              mmrel->hasflob(), 
                              getDBname());
  return ttreeObject;
}
/*

7.2.3  Value Mapping Functions of operator ~mcreatettree~

*/
int mcreatettreeVMMem(
                Word* args, Word& result,
                int message, Word& local, Supplier s) {
  
  result  = qp->ResultStorage(s);
  MPointer* res = static_cast<MPointer*>(result.addr);

  // the main memory relation
  Mem* rel = (Mem*) args[0].addr;
  if(!rel->IsDefined()){
      res->setPointer(0);
      return 0;
  }
  string name = rel->GetValue();
  int attrPos = ((CcInt*) args[2].addr)->GetValue();

  ListExpr type = catalog->getMMObjectTypeExpr(name);

  ListExpr memType = qp->GetType(qp->GetSon(s,0));
  if(!nl->Equal(type,memType)){
    cerr << "Types of memory representation and type in catalog differ";
    cerr << "Type in TM was " << nl->ToString(memType) << endl; 
    cerr << "Type in catalkog is " << nl->ToString(type) << endl;
    res->setPointer(0);
    return 0;
  }

  // get main memory relation
  MemoryRelObject* mmrel =
      (MemoryRelObject*)catalog->getMMObject(name);
  if(!mmrel){
    res->setPointer(0);
    return 0;
  }

  MemoryTTreeObject* mmtree = createMTTreeOver(mmrel, attrPos, 
                                               nl->Second(qp->GetType(s)));
  res->setPointer(mmtree);
  mmtree->deleteIfAllowed();
  return 0;
}


int mcreatettreeVMMP(
                Word* args, Word& result,
                int message, Word& local, Supplier s) {
  result  = qp->ResultStorage(s);
  MPointer* res = static_cast<MPointer*>(result.addr);
  int attrPos = ((CcInt*) args[2].addr)->GetValue();
  MPointer* rel = (MPointer*) args[0].addr;
  MemoryRelObject* mmrel = (MemoryRelObject*) rel->GetValue();
  if(!mmrel){
    res->setPointer(0);
    return 0;
  }
  MemoryTTreeObject* mmtree = createMTTreeOver(mmrel, attrPos, 
                                               nl->Second(qp->GetType(s)));
  res->setPointer(mmtree);
  mmtree->deleteIfAllowed();
  return 0;
}

/*

7.2.4 Value Mapping Array and Selection

*/
ValueMapping mcreatettreeVM[] =
{
    mcreatettreeVMMP,
    mcreatettreeVMMem
};

int mcreatettreeSelect(ListExpr args){
    return MPointer::checkType(nl->First(args))?0:1;
}

/*

7.2.5 Description of operator ~mcreatettree~

*/
OperatorSpec mcreatettreeSpec(
    "MREL x IDENT -> mpointer(mem(ttree X)) ",
    "_ mcreatettree [_]",
    "Creates an T-Tree over a main memory relation given by the"
    "first argument and an attribute given by the second argument",
    "query mStaedte mcreatettree [SName]"
);

/*

7.1.6 Instance of operator ~mcreatettree~

*/

Operator mcreatettreeOp (
    "mcreatettree",
    mcreatettreeSpec.getStr(),
    2,
    mcreatettreeVM,
    mcreatettreeSelect,
    mcreatettreeTypeMap
);


/*

7.3 Operator ~minsertttree~, ~mdeletettree~

These operators insert, delete or update objects in a main memory ttree index.
They receive a stream of tuples (including an attribute tid), whose attributes,
determined by the third argument, will be either inserted, deleted or updated
in the tree.

7.3.1 Type Mapping Functions of operator ~minsertttree~,
      ~mdeletettree~.

*/
enum ChangeTypeTree {
  MInsertTree,
  MDeleteTree
};

template<class TreeType>
ListExpr minsertdeletetreeTypeMap(ListExpr args){
  
  if(nl->ListLength(args)!=3) {
    return listutils::typeError("three arguments expected");
  }
  
  ListExpr stream = nl->First(args); //stream + query

  if(!listutils::isTupleStream(stream)){
    return listutils::typeError("first argument must be a tuple stream");
  }

  // check if last attribute is of type 'tid'
  ListExpr rest = nl->Second(nl->Second(stream));
  ListExpr next;
  while (!(nl->IsEmpty(rest))) {
    next = nl->First(rest);
    rest = nl->Rest(rest);
  }
  if(!TupleIdentifier::checkType(nl->Second(next))) {
    return listutils::typeError("last attribute in the tuple must be a tid");
  }
  
  // allow ttree or avl ttree as second argument
  ListExpr second = nl->Second(args);
  ListExpr tree;
  if(!getMemSubType(second,tree)){
     return listutils::typeError("second arg is not a memory object");
  } 
  if(!TreeType::checkType(tree)) {
    return listutils::typeError("second arg is not a memory tree");
  }

  // check attribute
  ListExpr third = nl->Third(args);
  if(nl->AtomType(third)!=SymbolType) {
      return listutils::typeError("third argument is not a valid "
                                  "attribute name");
  }

  string attrName = nl->SymbolValue(third);
  ListExpr attrType = 0;
  int attrPos = 0;
  ListExpr attrList = nl->Second(nl->Second(stream));
  attrPos = listutils::findAttribute(attrList, attrName, attrType);

  if (attrPos == 0){
      return listutils::typeError
      ("there is no attribute having name " + attrName);
  }

  
  ListExpr append = nl->OneElemList(nl->IntAtom(attrPos));
  
  return nl->ThreeElemList(
                nl->SymbolAtom(Symbol::APPEND()),
                append,
                stream);
}


template<ChangeTypeTree ct, class TreeType, class TreeElem>
class minsertdeleteInfo {
  public:

    typedef TreeType treetype;

    minsertdeleteInfo(Word& w, TreeType* _tree, int _attrPos)
      : stream(w), tree(_tree), attrPos(_attrPos) {
      stream.open();
    }

    ~minsertdeleteInfo(){
      stream.close();
    }

    
    Tuple* next(){
      Tuple* res = stream.request();
      if(!res) {
        return 0; 
      }
      
      Attribute* tidAttr = res->GetAttribute(res->GetNoAttributes() - 1);
      TupleId oldTid = ((TupleIdentifier*)tidAttr)->GetTid();
      Attribute* attr = res->GetAttribute(attrPos-1);
      TreeElem  elem = TreeElem(attr,oldTid);
      switch(ct){
        case MInsertTree : tree->insert(elem); break;
        case MDeleteTree : tree->remove(elem); break;
        default: assert(false);
      }
      return res;
    }
  private:
     Stream<Tuple> stream;
     TreeType* tree;
     int attrPos;
};


/*

7.3.2  The Value Mapping Functions of operator ~minsertttree~
       and ~mdeletettree~

*/
template<class T, class LocalInfo, class MemTree>
int minserttreeValueMap (Word* args, Word& result,
                    int message, Word& local, Supplier s) {
  
  LocalInfo* li = (LocalInfo*) local.addr;


  switch(message) {

    
    case OPEN : {
      if(li) {
        delete li;
        local.addr = 0;
      }

      T* tree = (T*) args[1].addr;
      if(!tree->IsDefined()){
         // cout << "undefined" << endl;
          return 0;
      }
      string name = tree->GetValue();
      // cut blank from the front
      stringutils::trim(name);


      
      MemTree* mmtree =(MemTree*) catalog->getMMObject(name);
      if(!mmtree) {
        return 0;
      }
     
      int attrPos = ((CcInt*) args[3].addr)->GetValue(); 


      cout << "*********************************" << endl;
      cout << "AttrPos = " << attrPos << endl;
      cout << "*********************************" << endl;

        
      local.addr = new LocalInfo(args[0],
                                 mmtree->gettree(),
                                 attrPos);
      return 0;
    }

    case REQUEST : {
      result.addr=li?li->next():0;
      return result.addr?YIELD:CANCEL;
    }
    
    case CLOSE :
      if(li) {
        delete li;
        local.addr = 0;
      }
      return 0;
      
  }    
  return 0;

}

template<class LocalInfo, class MemTree>
int minserttreeMPointerValueMap (Word* args, Word& result,
                    int message, Word& local, Supplier s) {
  
  LocalInfo* li = (LocalInfo*) local.addr;
  
  switch(message) {
    
    case OPEN : {
      if(li) {
        delete li;
        local.addr = 0;
      }

      MPointer* tree = (MPointer*) args[1].addr;
      MemTree* mmtree =(MemTree*) (*tree)();
      if(!mmtree) {
        return 0;
      }
      int attrPos = ((CcInt*) args[3].addr)->GetValue(); 
      local.addr = new LocalInfo(args[0],
                                 mmtree->gettree(),
                                 attrPos);
      return 0;
    }

    case REQUEST : {
      result.addr=li?li->next():0;
      return result.addr?YIELD:CANCEL;
    }
    
    case CLOSE :
      if(li) {
        delete li;
        local.addr = 0;
      }
      return 0;
      
  }    
  return 0;

}

/*
7.3.3 Value Mapping Array and Selection

*/
ValueMapping minsertttreeVM[] = {

    // insert
    minserttreeValueMap<Mem,minsertdeleteInfo<MInsertTree, 
                                 memttree, AttrIdPair>, MemoryTTreeObject >,
    minserttreeMPointerValueMap<minsertdeleteInfo<MInsertTree, 
                                 memttree, AttrIdPair>, MemoryTTreeObject >,

    // delete
    minserttreeValueMap<Mem,minsertdeleteInfo<MDeleteTree, 
                                 memttree, AttrIdPair>, MemoryTTreeObject >,
    minserttreeMPointerValueMap<minsertdeleteInfo<MDeleteTree, 
                                 memttree, AttrIdPair>, MemoryTTreeObject >
};

template<ChangeTypeTree ct>
int minsertttreeSelect(ListExpr args){
  ListExpr a1 = nl->Second(args);
  int n1 = -1;
  if(Mem::checkType(a1)) n1 = 0;
  if(MPointer::checkType(a1)) n1 = 1;
  if(n1<0) return  -1;

  if(ct==MInsertTree) 
    return n1;
  else if(ct==MDeleteTree) 
    return n1+2;
}

/*
7.3.4 Description of operator ~minsertttree~

*/

OperatorSpec minsertttreeSpec(
    "stream(tuple(x@[TID:tid])) x MTTREE x IDENT "
    "-> stream(tuple(x@[TID:tid])) , MTTREE in {string, mem, mpointer}",
    "_ op [_,_]",
    "inserts an object into a main memory ttree",
    "query ten feed head[5] minsert[mten] "
    "minsertttree[mten_No, No] count"
);

/*
7.3.5 Instance of operator ~minsertttree~

*/
Operator minsertttreeOp (
    "minsertttree",
    minsertttreeSpec.getStr(),
    4,
    minsertttreeVM,
    minsertttreeSelect<MInsertTree>,
    minsertdeletetreeTypeMap<MemoryTTreeObject>
);

/*
7.4.4 Description of operator ~mdeletettree~

*/

OperatorSpec mdeletettreeSpec(
    "stream(tuple(x@[TID:tid])) x MTTREExIDENT "
    "-> stream(tuple(x@[TID:tid])) , MTTREE in {mem, mpointer}",
    "_ op [_,_]",
    "deletes an object identified by tupleid from a main memory ttree",
    "query ten feed head[5] mdelete[mten] " 
    "mdeletettree[mten_No, No] count"
);

/*
7.4.5 Instance of operator ~mdeletettree~

*/
Operator mdeletettreeOp (
    "mdeletettree",
    mdeletettreeSpec.getStr(),
    4,
    minsertttreeVM,
    minsertttreeSelect<MDeleteTree>,
    minsertdeletetreeTypeMap<MemoryTTreeObject>
);

/*
7.5 ~insert~ and ~delete~ for AVL trees

*/

OperatorSpec minsertavltreeSpec(
    "stream(tuple(x@[TID:tid])) x AVLTREE x IDENT "
    "-> stream(tuple(x@[TID:tid])), AVLTREE iun {string, mem, mpointer}",
    "_ op [_,_]",
    "inserts an object into a main memory avltree",
    "query ten feed head[5] minsert[mten] "
    "minsertavltree[mten_No, No] count"
);


ValueMapping minsertAVLtreeVM[] = {
    minserttreeValueMap<Mem,minsertdeleteInfo<MInsertTree, 
                                 memAVLtree, AttrIdPair>, MemoryAVLObject >,
    minserttreeMPointerValueMap<minsertdeleteInfo<MInsertTree, 
                                 memAVLtree, AttrIdPair>, MemoryAVLObject >,
};

int minsertavltreeSelect(ListExpr args){
  ListExpr a1 = nl->Second(args);
  if(Mem::checkType(a1)) return 0;
  if(MPointer::checkType(a1)) return 1;
  return -1;
}


Operator minsertavltreeOp (
    "minsertavltree",
    minsertavltreeSpec.getStr(),
    2,
    minsertAVLtreeVM,
    minsertavltreeSelect,
    minsertdeletetreeTypeMap<MemoryAVLObject>
);


OperatorSpec mdeleteavltreeSpec(
    "stream(tuple(x@[TID:tid])) x AVLTREE x ident "
    "-> stream(tuple(x@[TID:tid])), AVLTREE in {mem, mpointer}",
    "_ op [_,_]",
    "Removes objects from  a main memory avltree",
    "query ten feed head[5] modelete[mten] "
    "mdeleteavltree[mten_No, No] count"
);


ValueMapping mdeleteAVLtreeVM[] = {
    minserttreeValueMap<Mem,minsertdeleteInfo<MDeleteTree, 
                                    memAVLtree, AttrIdPair>, MemoryAVLObject >,
    minserttreeMPointerValueMap<minsertdeleteInfo<MDeleteTree, 
                                    memAVLtree, AttrIdPair>, MemoryAVLObject >
};

int mdeleteavltreeSelect(ListExpr args){
  ListExpr a1 = nl->Second(args);
  if(Mem::checkType(a1)) return 0;
  if(MPointer::checkType(a1)) return 1;
  return -1;
}


Operator mdeleteavltreeOp (
    "mdeleteavltree",
    mdeleteavltreeSpec.getStr(),
    2,
    mdeleteAVLtreeVM,
    mdeleteavltreeSelect,
    minsertdeletetreeTypeMap<MemoryAVLObject>
);


/*
7.6 Operators ~mcreateinsertrel~, ~mcreatedeleterel~ and
~mcreateupdaterel~

These operators create an auxiliary relation with the same tuple schema
as the given relation including an attribute tid. In case of 
~mcreateupdaterel~, additionally all attributes with an appended sting 
'old' are added to the schema.

7.6.1 General Type mapping function of operators ~mcreateinsertrel~,
~mcreatedeleterel~ and ~mcreateupdaterel~

*/
ListExpr mcreateAuxiliaryRelTM(const ListExpr& args,
                                    const string opName) {

  if(nl->HasLength(args,2) && !nl->HasLength(args,1)) {
    return listutils::typeError("one argument expected");
  }
  if(nl->HasLength(args,2)){
    if(!CcBool::checkType(nl->Second(args))){
      return listutils::typeError("second arg is not of type bool");
    }
  } 
  
  ListExpr first = nl->First(args);
  if(!MPointer::checkType(first)){
    return listutils::typeError("mpointer(mem(P)) expected");
  } 
  // remove mpointer and mem from type
  first = nl->Second(nl->Second(first));

  // check for rel
  bool isOrel;
  if(Relation::checkType(first)) {
    isOrel = false;
  } else if(listutils::isOrelDescription(first)) {
    isOrel = true;
  } else {
    return listutils::typeError(
    "first arg is not a memory relation or ordered relation");
  }

  // build first part of the result-tupletype
  ListExpr rest =  nl->Second(nl->Second(first));
  ListExpr listn = nl->OneElemList(nl->First(rest));
  ListExpr lastlistn = listn;
  rest = nl->Rest(rest);
  while(!(nl->IsEmpty(rest))) {
     lastlistn = nl->Append(lastlistn,nl->First(rest));
     rest = nl->Rest(rest);
  }
  
  if(opName == "mcreateupdaterel") {
    // Append once again all attributes from the argument-relation
    // to the result-tupletype but the names of the attributes
    // extendend by 'old'
  
    rest = nl->Second(nl->Second(first));
    string oldName;
    ListExpr oldAttribute;
    while (!(nl->IsEmpty(rest))) {
      nl->WriteToString(oldName, nl->First(nl->First(rest)));
      oldName += "_old";
      oldAttribute =
        nl->TwoElemList(
          nl->SymbolAtom(oldName),
          nl->Second(nl->First(rest)));
      lastlistn = nl->Append(lastlistn,oldAttribute);
      rest = nl->Rest(rest);
    }
  }

  //Append last attribute for the tupleidentifier
  lastlistn = nl->Append(lastlistn,
                         nl->TwoElemList(
                           nl->SymbolAtom("TID"),
                           nl->SymbolAtom(TupleIdentifier::BasicType())));
  
  ListExpr outlist;
  

  if(!isOrel) {
    outlist = nl->TwoElemList(
                listutils::basicSymbol<Mem>(),
                nl->TwoElemList(
                  listutils::basicSymbol<Relation>(),
                  nl->TwoElemList(
                    nl->SymbolAtom(Tuple::BasicType()),
                    listn)));
  } else {
    outlist = nl->TwoElemList(
                listutils::basicSymbol<Mem>(),
                nl->ThreeElemList(
                  nl->SymbolAtom(OREL),
                  nl->TwoElemList(  
                    nl->SymbolAtom(Tuple::BasicType()),
                    listn),
                  nl->Third(first)));
  }
  ListExpr resType = MPointer::wrapType(outlist);
  if(nl->HasLength(args,1)){
     return nl->ThreeElemList( nl->SymbolAtom(Symbols::APPEND()),
                               nl->OneElemList(nl->BoolAtom(false)),
                               resType);

  } else {
    return resType;
  }
}

ListExpr mcreateinsertrelTM(ListExpr args) {
  return mcreateAuxiliaryRelTM(args, "mcreateinsertrel");
}

ListExpr mcreatedeleterelTM(ListExpr args) {
  return mcreateAuxiliaryRelTM(args, "mcreatedeleterel");
}

ListExpr mcreateupdaterelTM(ListExpr args) {
  return mcreateAuxiliaryRelTM(args, "mcreateupdaterel");
}

/*
7.6.2 General Value mapping function of operators ~mcreateinsertrel~, 
       ~mcreatedeleterel~ and ~mcreateupdaterel~

*/
template<bool isORel>
int mcreateauxiliaryrelVMT(Word* args, Word& result, int message,
                             Word& local, Supplier s) {
  result = qp->ResultStorage(s);
  MPointer* mp = (MPointer*) result.addr;
  ListExpr resType = qp->GetType(s);
  MPointer* arg = (MPointer*) args[0].addr;
  if(arg->isNull()) return 0;

  resType = nl->Second(resType); // remove mpointer
  CcBool* Flob = (CcBool*) args[1].addr;
  bool flob = Flob->IsDefined() && Flob->GetValue();
  if(!isORel){
    vector<Tuple*>* v = new vector<Tuple*>();
    MemoryRelObject* mrel = new MemoryRelObject(v,0,nl->ToString(resType),
                                               flob, getDBname());
    mp->setPointer(mrel);
    mrel->deleteIfAllowed();
  } else {
    ttree::TTree<TupleWrap,TupleComp>* t;
    t  = new ttree::TTree<TupleWrap,TupleComp>(16,18);
    MemoryORelObject* moa = (MemoryORelObject*) arg->GetValue(); 
    vector<int>* va = moa->getAttrPos();
    vector<int>* vr = new vector<int>();
    for(size_t i=0;i<vr->size();i++){
       vr->push_back(va->at(i));
    }
    MemoryORelObject* mor = new MemoryORelObject(t,vr,0, 
                                    nl->ToString(resType),flob, 
                                    getDBname());
    mp->setPointer(mor);
    mor->deleteIfAllowed();
  }
  return 0;
}

ValueMapping mcreateauxiliaryrelVM[] = {
   mcreateauxiliaryrelVMT<false>,
   mcreateauxiliaryrelVMT<true>
};

int mcreateauxiliaryrelSelect(ListExpr args){
  ListExpr rel = nl->Second(nl->Second(nl->First(args)));
  return Relation::checkType(rel)?0:1;
}

/*
7.6.4 Specification of operator ~mcreateinsertrel~

*/
OperatorSpec mcreateinsertrelSpec(
    "mpointer(mem(rel)) [x bool]  -> mpointer(mem(rel(tuple(x@[TID:tid]))))",
    "mcreateinsertrel(_)",
    "creates an auxiliary relation",
    "query mcreateinsertrel(mten))"
);


/*
7.6.5 Definition of operator ~mcreateinsertrel~

*/
Operator mcreateinsertrelOp (
  "mcreateinsertrel",             // name
  mcreateinsertrelSpec.getStr(),  // specification
  2,
  mcreateauxiliaryrelVM,        // value mapping
  mcreateauxiliaryrelSelect,    // trivial selection function
  mcreateinsertrelTM       // type mapping
);


/*
7.7.4  Specification of operator ~mcreatedeleterel~

*/
OperatorSpec mcreatedeleterelSpec(
    "MREL -> mem(rel(tuple(x@[TID:tid]))) ",
    "mcreatedeleterel(_) , MREL in {string, mem, mpointer}",
    "creates an auxiliary relation",
    "let  fuenf =  mcreatedeleterel(mten)"
);

/*
7.7.5  Definition of operator ~mcreatedeleterel~

*/
Operator mcreatedeleterelOp(
  "mcreatedeleterel",             // name
  mcreatedeleterelSpec.getStr(),  // specification
  2,
  mcreateauxiliaryrelVM,             // value mapping
  mcreateauxiliaryrelSelect,         // trivial selection function
  mcreatedeleterelTM         // type mapping
);


/*
7.8.4 Specification of operator ~mcreateupdaterel~

*/

OperatorSpec mcreateupdaterelSpec(
    " MREL(tuple(X)) -> mem(rel(tuple(x@[(a1_old x1)..."
    "(an_old xn)(TID:tid)]))), MREL in {string, mem, mpointer}",
    "mcreateupdaterel(_)",
    "creates an auxiliary relation",
    "let fuenf = mcreateupdaterel(mten))"
);

    
/*
7.8.5 Definition of operator ~mcreateupdaterel~

*/
Operator mcreateupdaterelOp (
  "mcreateupdaterel",             // name
  mcreateupdaterelSpec.getStr(),  // specification
  2,
  mcreateauxiliaryrelVM,             // value mapping
  mcreateauxiliaryrelSelect,         // selection function
  mcreateupdaterelTM         // type mapping
);




/*
7.9 Operator ~minsert~

Inserts each tuple of the inputstream into the memory relation. Returns 
a stream of tuples which is  basically the same as the inputstream 
but each tuple extended by an attribute of type 'tid' which is the 
tupleidentifier of the inserted tuple in the extended relation.

7.9.1 General type mapping function of operators ~minsert~, ~minsertsave~,
      ~mdelete~ and ~mdeletesave~
      
   stream(tuple(x)) x {string, mem(rel(...))} -> stream(tuple(x@[TID:tid])) or
   
   stream(tuple(x)) x {string, mem(rel(...))} x {string, mem(rel(...))} 
     -> stream(tuple(x@[TID:tid])) 
     
   
 
*/
template<int noargs, bool ordered>
ListExpr minsertTypeMap(ListExpr args) {
  
  if(!nl->HasLength(args,noargs)) {
    return listutils::typeError("wrong number of arguments");
  }

  ListExpr stream = nl->First(args);

  // process stream
  if(!Stream<Tuple>::checkType(stream)){
    return listutils::typeError("first argument must be a tuple stream");
  }
  
  string errMsg;
  
  // process second argument (mem(rel))
  ListExpr second;
  if(!getMemSubType(nl->Second(args), second)){
    return listutils::typeError("second arg is not a memory object");
  }

  if(ordered){
    if(!listutils::isOrelDescription(second)){
      return listutils::typeError(
         "second arg is not an ordered memory relation");
    }
  } else {
    if((!Relation::checkType(second))){
      return listutils::typeError(
         "second arg is not a memory relation");
    }
  }

  if(!nl->Equal(nl->Second(stream), nl->Second(second))){
      return listutils::typeError("stream type and mem relation type differ\n"
                               "stream: " + nl->ToString(stream) + "\n"
                               + "rel   : " + nl->ToString(second));
  }
  
  // append tupleidentifier
  ListExpr rest = nl->Second(nl->Second(second));
  ListExpr at;
  if(listutils::findAttribute(rest,"TID",at)>0){
    return listutils::typeError("There is already an TID attribute");
  }
  ListExpr listn = nl->OneElemList(nl->First(rest));
  ListExpr lastlistn = listn;
  rest = nl->Rest(rest);
  while (!(nl->IsEmpty(rest))) {
    lastlistn = nl->Append(lastlistn,nl->First(rest));
    rest = nl->Rest(rest);
  }
  

  lastlistn = nl->Append(lastlistn,
                         nl->TwoElemList(
                           nl->SymbolAtom("TID"),
                           nl->SymbolAtom(TupleIdentifier::BasicType())));
  
  // process third argument (minsertsave only)
  if(nl->ListLength(args)==3) {
    ListExpr third;
    if(!getMemSubType(nl->Third(args),third)){
      return listutils::typeError("second arg is not a Memory object");
    }
    if(!Relation::checkType(third)){
      return listutils::typeError(
       "third arg is not a memory relation");
    }
    // the scheme of this relation must be equal to the output tuple scheme
    if(!nl->Equal(listn, nl->Second(nl->Second(third)))){
       return listutils::typeError("Scheme of the auxiliary relation is not "
                  "equal to the main relation + TID");
    }
  }
  return nl->TwoElemList(nl->SymbolAtom(Symbol::STREAM()),
                         nl->TwoElemList(
                           nl->SymbolAtom(Tuple::BasicType()),
                           listn));
}


class minsertInfo {
  public:
     minsertInfo(Word& w, vector<Tuple*>* _relation, 
                 bool _flob, TupleType* _type)
        : stream(w),relation(_relation), flob(_flob) {
        type = _type;
        type->IncReference();
        stream.open();
     }

    ~minsertInfo(){
       stream.close();
       type->DeleteIfAllowed();     
     }

     Tuple* next(){
       Tuple* res = stream.request();

       if(!res) {return 0; }

       if(flob) {
         res->bringToMemory();
       }
       
       Tuple* newtup = new Tuple(type); 
       for(int i = 0; i < res->GetNoAttributes(); i++) {
         newtup->CopyAttribute(i,res,i);
       }
      
       
       // add the main memory tuple id to the output tuple
       Attribute* tidAttr = new TupleIdentifier(true,
                                       relation->size()+1);
       newtup->PutAttribute(res->GetNoAttributes(), tidAttr);
       relation->push_back(res);
       newtup->SetTupleId(relation->size());
    
       return newtup;
     }

  private:
     Stream<Tuple> stream;
     vector<Tuple*>* relation;
     bool flob;
     TupleType* type;
};


/*

7.9.2  Value Mapping Function of operator ~minsert~

*/
template<class T>
int minsertValMap (Word* args, Word& result,
                    int message, Word& local, Supplier s) {

  typedef pair<MemoryRelObject*,TupleType*> globalInfo;
  minsertInfo* li = (minsertInfo*) local.addr;
  globalInfo* gi = (globalInfo*) qp->GetLocal2(s).addr;
  
  switch (message) {

    case INIT: {
       TupleType* tt = new TupleType(nl->Second(GetTupleResultType(s)));
       gi = new globalInfo(0,tt);
       qp->GetLocal2(s).addr=gi;
       return 0;
    }

    case FINISH: {
       if(gi){
         gi->first=0;
         gi->second->DeleteIfAllowed();
         delete gi;
         qp->GetLocal2(s).addr = 0;
       }
       return 0;
    }
    
    case OPEN : {
      if(li) {
        delete li;
        local.addr=0;
      }
      

      assert(gi);

      if(!gi->first){      
        T* oN = (T*) args[1].addr;   
        MemoryRelObject* rel = getMemRel(oN);
        if(!rel) return 0;
        gi->first = rel;
      }
      MemoryRelObject*  mmr=gi->first;
      TupleType* tt = gi->second; 
      
      local.addr = new minsertInfo(args[0],mmr->getmmrel(),mmr->hasflob(),tt);
      return 0;
    }
    
    case REQUEST : {
      result.addr=li?li->next():0;
      return result.addr?YIELD:CANCEL;
    }
    
    case CLOSE :
      if(li) {
        delete li;
        local.addr = 0;
      }
      return 0;
      
  }
  return 0;

}

ValueMapping minsertVM[] = {
   minsertValMap<Mem>,
   minsertValMap<MPointer>,

};

int minsertSelect(ListExpr args){
  ListExpr t = nl->Second(args);
  if(Mem::checkType(t)) return 0;
  if(MPointer::checkType(t)) return 1;
  return -1;
}

/*
7.9.4 Description of operator ~minsert~

*/

OperatorSpec minsertSpec(
    "stream(tuple(x)) x MREAL -> stream(tuple(x@[TID:tid])), "
    "MREL in {string, mem, mpointer}",
    "_ minsert [_]",
    "inserts the tuples of a stream into an "
    "existing main memory relation. All tuples get an additional "
    "attribute of type 'tid'",
    "query minsert (ten feed head[5],mten) count"
);

/*
7.9.5 Instance of operator ~minsert~

*/
Operator minsertOp (
    "minsert",
    minsertSpec.getStr(),
    2,
    minsertVM,
    minsertSelect,
    minsertTypeMap<2,false>
);


/*
7.10 Operator ~minsertsave~

Inserts each tuple of the inputstream into the memory relation. Returns 
a stream of tuples which is basically the same as the inputstream 
but each tuple extended by an attribute of type 'tid' which is the 
tupleidentifier of the inserted tuple in the extended relation.
Additionally the tuple of the input stream are saved in an auxiliary 
main memory realtion.

*/
class minsertsaveInfo {
  public:
     minsertsaveInfo(Word& w, vector<Tuple*>* _relation, 
         vector<Tuple*>* _auxrel, 
                 bool _flob, ListExpr _type)
        : stream(w),relation(_relation), auxrel(_auxrel), 
          flob(_flob) {
        stream.open();
        type = new TupleType(_type);
     }
     
    ~minsertsaveInfo(){
       stream.close();
       type->DeleteIfAllowed();
     }

    Tuple* next(){
      Tuple* res = stream.request();
      if(!res){
         return 0;
      }
      
      if(flob){ 
        res->bringToMemory();
      }
      
      //get tuple id and append it to tuple
      Tuple* newtup = new Tuple(type); 
      for(int i = 0; i < res->GetNoAttributes(); i++){
        newtup->CopyAttribute(i,res,i);
      }
      Attribute* tidAttr = new TupleIdentifier(true,relation->size()+1);
      newtup->PutAttribute(res->GetNoAttributes(), tidAttr);
      
      // insert tuple in memory relation 
      relation->push_back(res);
      // insert tuple in auxrel
      auxrel->push_back(newtup);
      newtup->IncReference();
       
      return newtup;
    }

  private:
     Stream<Tuple> stream;
     vector<Tuple*>* relation;
     vector<Tuple*>* auxrel;
     bool flob;
     TupleType* type;
};


/*
7.10.2  Value Mapping Functions of operator ~minsertsave~

*/
template<class R,class A>
int minsertsaveValueMap (Word* args, Word& result,
                    int message, Word& local, Supplier s) {
  
  minsertsaveInfo* li = (minsertsaveInfo*) local.addr;
  
  switch (message) {
    
    case OPEN : {
      
      if(li) {
        delete li;
        local.addr=0;
      }
      
      qp->Open(args[0].addr);
      
      R* oN = (R*) args[1].addr;
      A* aux = (A*) args[2].addr;
      
      MemoryRelObject* rel = getMemRel(oN);;
      if(!rel) { return 0; }
      MemoryRelObject* auxrel = getMemRel(aux);;
      if(!auxrel) { return 0; }
      
      ListExpr tt = nl->Second(GetTupleResultType(s));
      
      local.addr = new minsertsaveInfo(args[0],rel->getmmrel(),
                                       auxrel->getmmrel(),
                                       rel->hasflob(),tt);
      return 0;
    }
    
    case REQUEST : {
      result.addr=li?li->next():0;
      return result.addr?YIELD:CANCEL;
    }
    
    case CLOSE :
      if(li) {
        delete li;
        local.addr = 0;
      }
      return 0;     
  }
  return 0;

}

ValueMapping minsertsaveVM[] = {
   minsertsaveValueMap<Mem,Mem>,
   minsertsaveValueMap<Mem,MPointer>,
   minsertsaveValueMap<MPointer,Mem>,
   minsertsaveValueMap<MPointer,MPointer>
};

int minsertsaveSelect(ListExpr args){
   ListExpr t = nl->Second(args);
   ListExpr a = nl->Third(args);
   
   int nt = -1;
   if(Mem::checkType(t)) nt = 0;
   if(MPointer::checkType(t)) nt = 2;
   if(nt<0) return -1;
       
   int na = -1;
   if(Mem::checkType(a)) na = 0;
   if(MPointer::checkType(a)) na = 1;
   if(na<0) return -1;
   
   return nt + na;
    
   
}

/*
7.10.4 Description of operator ~minsertsave~

*/
OperatorSpec minsertsaveSpec(
    "stream(tuple(x)) x MREL1 x MREL2, MREL1,MREL2 in {string, mem, mpointer} "
    "-> stream(tuple(x@[TID:tid]))",
    "_ minsertsave [_,_]",
    "insert all tuple of an input stream into two main memory relations",
    "query ten feed head[5] minsertsave['ten','fuenf'] count"
);

/*
7.10.5 Instance of operator ~minsertsave~

*/
Operator minsertsaveOp (
    "minsertsave",
    minsertsaveSpec.getStr(),
    4,
    minsertsaveVM,
    minsertsaveSelect,
    minsertTypeMap<3,false>
);


/*
7.11 Operator ~minserttuple~

This Operator inserts the given list of attributes as a new tuple into 
a main memory relation.

7.11.1 General type mapping function of operators ~minserttuple~
      and ~minserttuplesave~.
      
*/
template<int noArgs>
ListExpr minserttupleTypeMap(ListExpr args) {

  
  if((nl->ListLength(args)!=noArgs)) {
    return listutils::typeError(stringutils::int2str(noArgs) 
                                + " arguments expected");
  }
  // process first argument (mem(rel))
  ListExpr first;
  if(!getMemSubType(nl->First(args),first)){
    return listutils::typeError("first argument is not a memory object");
  }
  if(!Relation::checkType(first)){
    return listutils::typeError("first arg is not a memory relation");
  }
    
  // check tuple
  ListExpr second = nl->Second(args);
  if(nl->AtomType(second)!=NoAtom){
    return listutils::typeError("list as second arg expected");
  }

  if(nl->ListLength(nl->Second(nl->Second(first)))!=nl->ListLength(second)){
    return listutils::typeError("different lengths in tuple and update");
  }
  
  // check whether types of relation matches types of attributes  
  ListExpr restrel = nl->Second(nl->Second(first));
  ListExpr resttuple = second;

  while(!(nl->IsEmpty(restrel))) {
    if(!nl->Equal(nl->Second(nl->First(restrel)),nl->First(resttuple))){
      return listutils::typeError("type mismatch in attribute "
                                  "list and update list");
    }
    restrel = nl->Rest(restrel);
    resttuple = nl->Rest(resttuple);
  }
  
  // append tupleid
  restrel = nl->Second(nl->Second(first));

  ListExpr at;
  if(listutils::findAttribute(restrel,"TID",at)>0){
    return listutils::typeError("TID attribute already present in "
                                "argument relation.");
  }

  ListExpr listn = nl->OneElemList(nl->First(restrel));
  ListExpr lastlistn = listn;
  restrel = nl->Rest(restrel);
  while (!(nl->IsEmpty(restrel))) {
    lastlistn = nl->Append(lastlistn,nl->First(restrel));
    restrel = nl->Rest(restrel);
  }
  
  lastlistn = nl->Append(lastlistn,
                          nl->TwoElemList(
                            nl->SymbolAtom("TID"),
                            nl->SymbolAtom(TupleIdentifier::BasicType())));

  // process third argument (minserttuplesave only)
  if(noArgs==3) {
    ListExpr third;
    if(!getMemSubType(nl->Third(args),third)){
      return listutils::typeError("third arg is not a memory object");
    }
    if(!Relation::checkType(third)){
      return listutils::typeError(
        "third arg is not a memory relation");
    }
    // check whether this relations corresponds to the output tuples
    ListExpr attrList = nl->Second(nl->Second(third));
    if(!nl->Equal(attrList, listn)){
      return listutils::typeError("auxiliary relation type differs to " 
                                  "main relationtype + TID");
    }

  }

 
  return nl->TwoElemList(nl->SymbolAtom(Symbol::STREAM()),
                          nl->TwoElemList(
                            nl->SymbolAtom(Tuple::BasicType()),
                            listn));
}


class minserttupleInfo{
  public:
    minserttupleInfo(vector<Tuple*>* _relation, bool _flob,
                     ListExpr _listType, TupleType* _type, Word& _tupleList)
      : relation(_relation), flob(_flob), 
        listType(_listType) ,type(_type), tupleList(_tupleList) {
      
      firstcall = true;
    }
    
    ~minserttupleInfo() {
      type->DeleteIfAllowed();
    }
    
    Tuple* next() {
      
      if(!firstcall) {
         return 0;
      } else {
         firstcall = false;
      }
      
      Tuple* res = new Tuple(type);
      
      ListExpr rest = nl->Second(nl->Second(listType));
      ListExpr insertType = nl->OneElemList(nl->First(rest));
      ListExpr last = insertType;

      rest = nl->Rest(rest);
      while(nl->ListLength(rest)>1) {
        last = nl->Append(last, nl->First(rest));
        rest = nl->Rest(rest);
      }
      insertType = nl->TwoElemList(nl->SymbolAtom(Tuple::BasicType()),
                                   insertType);

      TupleType* insertTupleType = new TupleType(insertType);  
      Tuple* tup = new Tuple(insertTupleType);
      insertTupleType->DeleteIfAllowed();
      Supplier supplier = tupleList.addr;
      
      Supplier s;
      attrValue.addr = 0;
      for(int i=0; i<res->GetNoAttributes()-1; i++) {
        s = qp->GetSupplier(supplier, i);
        qp->Request(s,attrValue);
        Attribute* attr = (Attribute*) attrValue.addr;
        res->PutAttribute(i,attr->Clone());
        tup->CopyAttribute(i,res,i);
      }
      const TupleId& tid = relation->size()+1;
      tup->SetTupleId(tid);
      // add TID
      Attribute* tidAttr = new TupleIdentifier(true,tid);
      res->PutAttribute(res->GetNoAttributes()-1, tidAttr);
      
      if(flob){
        tup->bringToMemory();
      }
      // add Tuple to memory relation
      relation->push_back(tup);
      return res;
    }

private:
    vector<Tuple*>* relation;
    bool flob;
    ListExpr listType;
    TupleType* type;
    Word tupleList;
    bool firstcall;
    Word attrValue;
};


/*

7.11.2  Value Mapping Function of operator ~minserttuple~

*/
template<class T>
int minserttupleValMap (Word* args, Word& result,
                    int message, Word& local, Supplier s) {

  minserttupleInfo* li = (minserttupleInfo*) local.addr;

  switch (message) {
    case OPEN: {
      
      if(li) {
        delete li;
        local.addr=0;
      }
      
      T* oN = (T*) args[0].addr;
      
      MemoryRelObject* rel = getMemRel(oN);
      if(!rel) { 
        return 0; }
      
      ListExpr tupleType = GetTupleResultType(s);
      TupleType* tt = new TupleType(nl->Second(tupleType));
      
      local.addr = new minserttupleInfo(rel->getmmrel(),rel->hasflob(),
                                        tupleType,tt,args[1]);
      return 0;
    }

    case REQUEST:
      result.addr=li?li->next():0;
      return result.addr?YIELD:CANCEL;

    case CLOSE:
      if(li) {
        delete li;
        local.addr = 0;
      }
      return 0;
  }
  return 0;
}


ValueMapping minserttupleVM[] = {
  minserttupleValMap<Mem>,
  minserttupleValMap<MPointer>
};

int minserttupleSelect(ListExpr args){
  ListExpr t = nl->First(args);
  if(Mem::checkType(t)) return 0;
  if(MPointer::checkType(t)) return 1;
  return -1;
}


/*
7.11.4 Description of operator ~minserttuple~

*/
OperatorSpec minserttupleSpec(
    "MREL x [t1 ... tn] -> stream(tuple(x@[TID:tid])) , "
    "MREL in {string, mem, mpointer}",
    "_ minserttuple [list]",
    "inserts a tuple into a main memory relation",
    "query 'Staedte' minserttuple['AA',34,5666,'899','ZZ'] count"
);

/*
7.11.5 Instance of operator ~minserttuple~

*/
Operator minserttupleOp (
    "minserttuple",
    minserttupleSpec.getStr(),
    2,
    minserttupleVM,
    minserttupleSelect,
    minserttupleTypeMap<2>
);




/*
7.12 Operator ~minserttuplesave~

The Operator ~minserttuplesave~ adds a tuple to a given main memory relation
(argument 1) and saves it additionally in an auxiliary main memory 
relation (argument 3). 
   
*/
class minserttuplesaveInfo{
  public:
    minserttuplesaveInfo(vector<Tuple*>* _relation, vector<Tuple*>* _auxrel, 
                         bool _flob, ListExpr _listType, TupleType* _type, 
                         Word& _tupleList)
      : relation(_relation), auxrel(_auxrel), flob(_flob), 
        listType(_listType) ,type(_type), tupleList(_tupleList) {
      
      firstcall = true;
    }
    
    ~minserttuplesaveInfo() {
      type->DeleteIfAllowed();
    }
    
    Tuple* next() {
      
      if(!firstcall){
          return 0;
      } else {
          firstcall = false;
      }
      
      Tuple* res = new Tuple(type);
      
      ListExpr rest = nl->Second(nl->Second(listType));
      ListExpr insertType = nl->OneElemList(nl->First(rest));
      ListExpr last = insertType;
      
      rest = nl->Rest(rest);
      while(nl->ListLength(rest)>1) {
        last = nl->Append(last, nl->First(rest));
        rest = nl->Rest(rest);
      }
      insertType = nl->TwoElemList(nl->SymbolAtom(Tuple::BasicType()),
                                   insertType);


      TupleType* insertTupleType = new TupleType(insertType);
      Tuple* tup = new Tuple(insertTupleType);
      insertTupleType->DeleteIfAllowed();
      Supplier supplier = tupleList.addr;
      attrValue.addr = 0;      
      Supplier s;
      for(int i=0; i<res->GetNoAttributes()-1; i++) {
        s = qp->GetSupplier(supplier, i);
        qp->Request(s,attrValue);
        Attribute* attr = (Attribute*) attrValue.addr;
        res->PutAttribute(i,attr->Clone());
        tup->CopyAttribute(i,res,i);
      }
      const TupleId tid = relation->size()+1;
      tup->SetTupleId(tid);
      
      // add TID
      Attribute* tidAttr = new TupleIdentifier(true,tid);
      res->PutAttribute(res->GetNoAttributes()-1, tidAttr);
      
      // flob
      if(flob) {
        tup->bringToMemory();
      }
      
      // add Tuple to memory relation
      relation->push_back(tup);

      // add tuple to auxrel
      res->IncReference();
      auxrel->push_back(res);
      return res;
    }

  private:
    vector<Tuple*>* relation;
    vector<Tuple*>* auxrel;
    bool flob;
    ListExpr listType;
    TupleType* type;
    Word tupleList;
    bool firstcall;
    Word attrValue;
};

/*
7.12.2  Value Mapping Function of operator ~minserttuplesave~

*/
template<class R, class A>
int minserttuplesaveValueMap (Word* args, Word& result,
                    int message, Word& local, Supplier s) {

  minserttuplesaveInfo* li = (minserttuplesaveInfo*) local.addr;

  switch (message) {
    case OPEN: {
      
      if(li) {
        delete li;
        local.addr=0;
      }
      
      R* oN = (R*) args[0].addr;

      ListExpr tupleType = GetTupleResultType(s);
      TupleType* tt = new TupleType(nl->Second(tupleType));
      
      A* aux = (A*) args[2].addr;
      
      MemoryRelObject* rel = getMemRel(oN);
      if(!rel) { return 0; }
      MemoryRelObject* auxrel = getMemRel(aux);
      if(!auxrel) { return 0; }
      local.addr = new minserttuplesaveInfo(rel->getmmrel(),
                                            auxrel->getmmrel(),
                                            rel->hasflob(),
                                            tupleType,tt,args[1]);
      return 0;
    }

    case REQUEST:
      result.addr=li?li->next():0;
      return result.addr?YIELD:CANCEL;

    case CLOSE:
      if(li) {
        delete li;
        local.addr = 0;
      }
      return 0;
  }
  return 0;
}


ValueMapping minserttuplesaveVM[] = {
  minserttuplesaveValueMap<Mem,Mem>,
  minserttuplesaveValueMap<Mem,MPointer>,
  minserttuplesaveValueMap<MPointer,Mem>,
  minserttuplesaveValueMap<MPointer,MPointer>,
};

int minserttuplesaveSelect(ListExpr args){
   ListExpr t = nl->First(args);
   ListExpr a = nl->Third(args);
   
   int nt = -1;
   if(Mem::checkType(t)) nt = 0;
   if(MPointer::checkType(t)) nt = 2;
   if(nt<0) return -1;
       
   int na = -1;
   if(Mem::checkType(a)) na = 0;
   if(MPointer::checkType(a)) na = 1;
   if(na<0) return -1;
   return nt + na;
    
}


/*
7.12.4 Description of operator ~minserttuplesave~

*/
OperatorSpec minserttuplesaveSpec(
    "MREL1 x [t1 ... tn] x "
    "MREL2 -> stream(tuple(x@[TID:tid])), MREL1, MREL2 "
    "in {string, mem, mpointer}",
    "_ minserttuplesave [list; _]",
    "inserts a tuple into a main memory relation and an auxiliary main "
    "memory relation",
    "query 'Staedte' "
    "minserttuplesave['AusgedachtDorf',34,5666,899,'ZZ'; 'Stadt'] count"
);

/*
7.12.5 Instance of operator ~minserttuplesave~

*/
Operator minserttuplesaveOp (
    "minserttuplesave",
    minserttupleSpec.getStr(),
    4,
    minserttuplesaveVM,
    minserttuplesaveSelect,
    minserttupleTypeMap<3>
);



/*
7.13 Operator ~mdelete~


7.13.1 Auxiliary function ~remove~

This function removes the first tuple in a relation having the
same attribute values as the given tuple. The tuple id of
res is set to the former tuple id of the removed tuple.
If the tuple is not found, its tuple id is set to null.

*/
void remove(vector<Tuple*>* relation, Tuple* res) {
    
  for (size_t i = 0; i < relation->size(); i++) {
    Tuple* tup = relation->at(i);
    if(tup){
      bool equal = true;
      for(int j=0; j<tup->GetNoAttributes() && equal; j++) {
        int cmp = ((Attribute*)tup->GetAttribute(j))->Compare(
                  ((Attribute*)res->GetAttribute(j)));
        if(cmp!=0){
          equal = false;
        }
      }
      if(equal){ // hit
        tup->DeleteIfAllowed();  
        (*relation)[i] = 0;
        res->SetTupleId(i+1);
        return;
      }
    }
  }
  res->SetTupleId(0);
}

class mdeleteInfo {
public:
mdeleteInfo(Word& w, 
            vector<Tuple*>* _mainRelation, 
            vector<Tuple*>* _auxRelation,
            ListExpr _type)
  : stream(w),
    mainRelation(_mainRelation),
    auxRelation(_auxRelation) {
    type = new TupleType(_type);
    stream.open();
  }

  ~mdeleteInfo(){
     stream.close();
     type->DeleteIfAllowed();
   }
     
  Tuple* next() {
     TupleIdentifier* tid;
     while((tid=stream.request())){
       //tid->Print(cout) << endl; 
       if(!tid->IsDefined()){
         tid->DeleteIfAllowed();
       } else {
         TupleId id = tid->GetTid();
         if(id<1 || id > mainRelation->size()){
           tid->DeleteIfAllowed();
         } else {
           Tuple* tuple = mainRelation->at(id-1);
           if(!tuple){
             tid->DeleteIfAllowed();
           } else {
             (*mainRelation)[id-1]=0;
             return createResultTuple(tuple,tid);
           }
         }
       }

     }
     return 0;
  }

  private:
     Stream<TupleIdentifier> stream;
     vector<Tuple*>* mainRelation;
     vector<Tuple*>* auxRelation;
     TupleType* type;


     Tuple* createResultTuple(Tuple* orig, TupleIdentifier* tid){
       Tuple* res = new Tuple(type);
       assert(res->GetNoAttributes()==orig->GetNoAttributes()+1);


       for(int i=0;i<orig->GetNoAttributes(); i++){
          res->CopyAttribute(i,orig,i);
       }
       res->PutAttribute(orig->GetNoAttributes(), tid);
       orig->DeleteIfAllowed();
       res->SetTupleId(tid->GetTid());
       // save tuple if required
       if(auxRelation){
         res->IncReference();
         auxRelation->push_back(res);
       }
       return res;
     }
};

/*

7.13 Operator ~mdelete~

This Operator deletes all tuples of an input stream from a main memory 
relation. All tuple of the input stream are returned in an output 
stream with an additional Attribut containing the tupleid.

7.13.2 Value Mapping Function of operator ~mdelete~

*/
template<bool save>
ListExpr mdeleteTM(ListExpr args){
  int noargs =  save?3:2;
  if(!nl->HasLength(args,noargs)){
    return listutils::typeError("wrong number of arguments");
  }
  if(!Stream<TupleIdentifier>::checkType(nl->First(args))){
    return listutils::typeError("first argument mut be a stream of tids");
  }
  string errMsg;
  ListExpr mainRel;
  if(!getMemSubType(nl->Second(args), mainRel)){
    return listutils::typeError("Second argument is not a memory type");
  }

  if(!Relation::checkType(mainRel)){
    return listutils::typeError("second arg is not a memory relation");
  }
  ListExpr mat = nl->Second(nl->Second(mainRel));
  // append TID to mtt
  ListExpr at = listutils::concat(mat, nl->OneElemList(
                               nl->TwoElemList(nl->SymbolAtom("TID"),
                                  listutils::basicSymbol<TupleIdentifier>())));
  ListExpr tt = nl->TwoElemList( listutils::basicSymbol<Tuple>(), at);
  ListExpr res = nl->TwoElemList(listutils::basicSymbol<Stream<Tuple> >(),
                           tt);
  if(!save){
     return res;
  }
  // check the aux relation
  ListExpr auxRel;
  if(!getMemSubType(nl->Third(args), auxRel)){
    return listutils::typeError("third argument is not a memory object");
  }
  if(!Relation::checkType(auxRel)){
    return listutils::typeError("third arg is not a memory relation");
  }
  if(!nl->Equal(nl->Second(auxRel),tt)){
    return listutils::typeError("auxiliary relation is not main "
                                "relation + tid");
  } 
  return res;
}


template<class T, class S, bool save>
int mdeleteValMap (Word* args, Word& result,
                    int message, Word& local, Supplier s) {
  
  mdeleteInfo* li = (mdeleteInfo*) local.addr;
  
  switch (message) {
    case OPEN : {
      
      if(li) {
        delete li;
        local.addr=0;
      }
      
      T* oN = (T*) args[1].addr;
      MemoryRelObject* rel = getMemRel(oN);
      if(!rel) { 
        return 0;
      }
      
      ListExpr  tt = nl->Second(GetTupleResultType(s));

      vector<Tuple*>* aux = 0;
      if(save){
         S* an = (S*) args[2].addr;
         MemoryRelObject* arel = getMemRel(an);
         if(!arel){
           return 0;
         }
         aux = arel->getmmrel();
      }

      local.addr = new mdeleteInfo(args[0],
                                   rel->getmmrel(),
                                   aux, 
                                   tt);
      return 0;
    }
    
    case REQUEST : {
      result.addr=li?li->next():0;
      return result.addr?YIELD:CANCEL;
    }
    
    case CLOSE :
      if(li) {
        delete li;
        local.addr = 0;
      }
      return 0;  
  }
  return 0;
}

ValueMapping mdeleteVM[] = {
   // the second template argument is ignored in this case
   mdeleteValMap<Mem, Mem, false>,
   mdeleteValMap<MPointer, Mem, false>
};

int mdeleteSelect(ListExpr args){
   ListExpr s = nl->Second(args);
   if(Mem::checkType(s)) return 0;
   if(MPointer::checkType(s)) return 1;
   return -1;
}

/*
7.13.4 Description of operator ~mdelete~

*/
OperatorSpec mdeleteSpec(
    "stream(tid) x MREL -> stream(tuple(x@[TID:tid])), "
    "MREL in {mem, mpointer}",
    "_ mdelete [_]",
    "deletes the tuple of a stream from an "
    "existing main memory relation",
    "query ten feed filter [.No = 2] tids mdelete['ten'] count"
);

/*
7.13.5 Instance of operator ~mdelete~

*/
Operator mdeleteOp (
    "mdelete",
    mdeleteSpec.getStr(),
    2,
    mdeleteVM,
    mdeleteSelect,
    mdeleteTM<false>
);


/*
7.14 Operator ~mdeletesave~

The operator ~mdeletesave~ deletes all tuples of an input stream from
a main memory relation. It returns the stream, all tuples now containing an
additional attribut of type 'tid' and adds the tuples to an auxiliary 
main memory relation.

*/

/*
7.14.2 Value Mapping of operator ~mdeletesave~

*/
ValueMapping mdeletesaveVM[] = {
  mdeleteValMap<Mem, Mem, true>,
  mdeleteValMap<Mem, MPointer,true>,
  mdeleteValMap<MPointer, Mem, true>,
  mdeleteValMap<MPointer, MPointer,true>,
};

int mdeletesaveSelect(ListExpr args){
  ListExpr a2 = nl->Second(args);
  int n1 = -1;
  if(Mem::checkType(a2)) n1 = 0;
  if(MPointer::checkType(a2)) n1 = 2;
  
  ListExpr a3 = nl->Third(args);
  int n2 = -1;
  if(Mem::checkType(a3)) n2 = 0;
  if(MPointer::checkType(a3)) n2 = 1;

  if(n1<0 || n2<0) return  -1; 
  return n1+n2;
}



/*
7.14.4 Description of operator ~mdeletesave~

*/

OperatorSpec mdeletesaveSpec(
    "stream(tid) x {mem(rel),mpointer} "
    "x {mem(rel),mpointer} -> stream(Tuple)",
    "_ mdeletesave[_,_]",
    "Deletes tuples with given ids from a relation and stores "
    "deleted tuple into a auxiliary relation. ",
    "query mten mfeed filter[.No > 6] tids "
    "mdeletesave[mtenm, mtenDel]  count"
);

/*
7.1.5 Instance of operator ~mdeletesave~

*/
Operator mdeletesaveOp (
    "mdeletesave",
    mdeletesaveSpec.getStr(),
    4,
    mdeletesaveVM,
    mdeletesaveSelect,
    mdeleteTM<true>
);


  
/*
7.2 Operator ~mdeletebyid~

The Operator ~mdeletebyid~ deletes all tuples possessing a given 
TupleIdentifier from a main memory relation. It returns the deleted
Tuple including a new attribute of type `tid` as a stream.

7.15.1 Type Mapping Funktion for operator ~mdeletebyid~

{string, mem(rel(tuple(x)))} x (tid) -> stream(tuple(x@[TID:tid]))

*/
ListExpr mdeletebyidTypeMap(ListExpr args) {

  if(nl->ListLength(args) != 2) {
    return listutils::typeError("wrong number of arguments");
  }

  ListExpr first;
  if(!getMemSubType(nl->First(args),first)){
    return listutils::typeError("first argument is not a memory object");
  }
  if(!Relation::checkType(first)){
    return listutils::typeError("first arg is not a memory relation");
  }

  // process second arg (tid)
  ListExpr second = nl->Second(args);
  if(!TupleIdentifier::checkType(second)){
    return listutils::typeError("second argument must be a tid");
  }
  
  ListExpr rest = nl->Second(nl->Second(first));
  ListExpr at;
  if(listutils::findAttribute(rest,"TID",at)){
    return listutils::typeError("Relation has a TID attribute");
  }
  ListExpr attrlist = nl->OneElemList(nl->First(rest));
  ListExpr currentattr = attrlist;
  rest = nl->Rest(rest);
  while (!(nl->IsEmpty(rest))) {
    currentattr = nl->Append(currentattr,nl->First(rest));
    rest = nl->Rest(rest);
  }
  
  currentattr = nl->Append(currentattr,
                           nl->TwoElemList(
                             nl->SymbolAtom("TID"),
                             nl->SymbolAtom(TupleIdentifier::BasicType())));
  
  return nl->TwoElemList(nl->SymbolAtom(Symbol::STREAM()),
                         nl->TwoElemList(
                           nl->SymbolAtom(Tuple::BasicType()),
                           attrlist));
}


class mdeletebyidInfo{
  public:
    mdeletebyidInfo(vector<Tuple*>* _relation, 
                    TupleIdentifier* _tid, 
                    ListExpr _type)
      : relation(_relation), tid(_tid) {
        type = new TupleType(_type);
        firstcall = true;
      }
      
    ~mdeletebyidInfo() {
         type->DeleteIfAllowed();
     }
     
    Tuple* next() {
      if(firstcall) {
        firstcall = false;
        if(!tid->IsDefined()){
          return 0;
        }
        size_t index = tid->GetTid();
        if(index==0 || index>relation->size()){
          return 0;
        }
        Tuple* res = relation->at(index-1);
        if(!res ) {
          return 0;
        }
        (*relation)[index-1] = 0;
        // overtake the attributes from the removed tuple
        Tuple* newtup = new Tuple(type); 
        for(int i=0; i<res->GetNoAttributes(); i++) {
           newtup->CopyAttribute(i,res,i);
        }
        Attribute* tidAttr = new TupleIdentifier(true,index);
        newtup->PutAttribute(res->GetNoAttributes(), tidAttr);
        res->DeleteIfAllowed();
        return newtup;
      }
      return 0;
    }

  private:
    vector<Tuple*>* relation;
    TupleIdentifier* tid;
    TupleType* type;
    bool firstcall;
};

/*

7.15.2  Value Mapping Function of operator ~mdeletebyid~

*/
template<class T>
int mdeletebyidValMap (Word* args, Word& result,
                    int message, Word& local, Supplier s) {

  mdeletebyidInfo* li = (mdeletebyidInfo*) local.addr;

    switch (message) {
      case OPEN: {
          
        if(li){
          delete li;
          local.addr=0;
        }
        
        T* oN = (T*) args[0].addr;
      
        MemoryRelObject* rel = getMemRel(oN);
        if(!rel) { return 0; }
        
        TupleIdentifier* tid = (TupleIdentifier*)(args[1].addr);
        
        local.addr = new mdeletebyidInfo(rel->getmmrel(),
                                         tid,
                                         nl->Second(GetTupleResultType(s)));
        return 0;
      }

      case REQUEST:
        result.addr=li?li->next():0;
        return result.addr?YIELD:CANCEL;

      case CLOSE:
      if(li) {
        delete li;
        local.addr = 0;
      }
      return 0;
   }

   return 0;
}


ValueMapping mdeletebyidVM[] = {
  mdeletebyidValMap<Mem>,
  mdeletebyidValMap<MPointer>
  
};

int mdeleteelect(ListExpr args){
   ListExpr a = nl->First(args);
   if(Mem::checkType(a)) return 0;
   if(MPointer::checkType(a)) return 1;
   return -1;
}


/*
7.15.4 Description of operator ~mdeletebyid~

*/
OperatorSpec mdeletepec(
    "{mem(rel(tuple(x))), mpointer} x tid "
    "-> stream(tuple(x@[TID:tid]))] ",
    "_ mdeletebyid [_]",
    "deletes the tuples possessing a given tupleid from the main memory "
    "relation",
    "query 'Staedte' mdeletebyid[[const tid value 5]] count"
);

/*
7.15.5 Instance of operator ~mdeletebyid~

*/
Operator mdeletebyidOp (
    "mdeletebyid",
    mdeletepec.getStr(),
    2,
    mdeletebyidVM,
    mdeleteelect,
    mdeletebyidTypeMap
);


/*
7.16 Operator ~mdeletedirect~

*/
ListExpr mdeletedirectTM(ListExpr args){
   string err = "stream(tuple(X)) x (mem(rel(tuple(X)))) expected";
   if(!nl->HasLength(args,2)){
      return listutils::typeError("two arguments expected");
   }
   ListExpr stream = nl->First(args);
   if(!Stream<Tuple>::checkType(stream)){
     return listutils::typeError("first arg is not a tuple stream");
   }
   ListExpr mrel;
   if(!getMemSubType(nl->Second(args),mrel)){
     return listutils::typeError("Second arg is not a memory object");
   }
   ListExpr streamAttrList = nl->Second(nl->Second(stream));
   ListExpr relAttrList = nl->Second(nl->Second(mrel));
   if(!nl->Equal(streamAttrList, relAttrList)){
     return listutils::typeError("attributes of stream and relation differ");
   }  
   ListExpr append = nl->OneElemList(nl->TwoElemList(
                            nl->SymbolAtom("TID"),
                            listutils::basicSymbol<TupleIdentifier>()));

   ListExpr resAttrList = listutils::concat( streamAttrList, append);
   if(!listutils::isAttrList(resAttrList)){
     return listutils::typeError("attribute TID already present. ");
   }
   return nl->TwoElemList(listutils::basicSymbol<Stream<Tuple> >(),
                          nl->TwoElemList(
                              listutils::basicSymbol<Tuple>(),
                              resAttrList));

}


class mdeletedirectInfo{

  public:
    mdeletedirectInfo(Word& _stream, vector<Tuple*>* _rel,
                      vector<Tuple*>* _saverel,
                      ListExpr resultTupleType):
     stream(_stream), rel(_rel), saverel(_saverel) {
       tt = new TupleType(resultTupleType);
       stream.open();
   }
  
   ~mdeletedirectInfo(){
       stream.close();
       tt->DeleteIfAllowed();
    }

    Tuple* next(){
       Tuple* inTuple = stream.request();
       if(!inTuple){
          return 0;
       }
       TupleId tid = inTuple->GetTupleId();
       if(tid>0 && tid <= rel->size()){
          Tuple* victim = rel->at(tid-1);
          if(victim){
             rel->at(tid-1) = 0;
             victim->DeleteIfAllowed();
          }
       }
       Tuple* resTuple = createResTuple(inTuple);
       inTuple->DeleteIfAllowed();
       if(saverel){
         resTuple->IncReference();
         saverel->push_back(resTuple);
       }
       return resTuple;
    }
  
  private:
     Stream<Tuple> stream;
     vector<Tuple*>* rel;
     vector<Tuple*>* saverel;
     TupleType* tt;

     Tuple* createResTuple(Tuple* inTuple){
        Tuple* res = new Tuple(tt);
        for(int i=0;i<inTuple->GetNoAttributes(); i++){
           res->CopyAttribute(i,inTuple,i);
        }
        res->PutAttribute(inTuple->GetNoAttributes(),
                          new TupleIdentifier(true,inTuple->GetTupleId()));
        return res;
     }

};


template<class T>
int mdeletedirectVMT(Word* args, Word& result,
                    int message, Word& local, Supplier s) {

  mdeletedirectInfo* li = (mdeletedirectInfo*) local.addr;

    switch (message) {
      case OPEN: {
        if(li){
          delete li;
          local.addr=0;
        }
        
        T* oN = (T*) args[1].addr;
      
        MemoryRelObject* rel = getMemRel(oN);
        if(!rel) { return 0; }
       
        local.addr = new  mdeletedirectInfo(
                            args[0],
                            rel->getmmrel(),
                            0,
                            nl->Second(GetTupleResultType(s)));
        return 0;
      }

     case REQUEST:
            result.addr = li?li->next():0;
            return result.addr?YIELD:CANCEL;

     case CLOSE :
             if(li){
                delete li;
                local.addr = 0;
             }
             return 0;
   }
   return -1;
}

ValueMapping mdeletedirectVM[] = {
  mdeletedirectVMT<Mem>,
  mdeletedirectVMT<MPointer>
};

int mdeletedirectSelect(ListExpr args){
  ListExpr s = nl->Second(args);
  if(Mem::checkType(s)) return 0;
  if(MPointer::checkType(s)) return 1;
  return -1;
}

OperatorSpec mdeletedirectSpec(
  "stream(tuple(X)) x MREL(tuple(X)) -> stream(tuple(X@(TID tid))),"
  " MREL in {mem, mpointer} ",
  "_ _ mdeletedirect",
  "This function extracts the tuple ids from the incoming stream "
  "and removes the tuples having this id from the main memory relation "
  "which can be given as a string, a mem(rel) or an mpointer(mem(rel)). "
  "The output are the incoming tuples extended by this id. Note that "
  "the values of the result tuples have nothing to do with the values "
  "stored in the relation. ",
  "query ten feed mten mdeletedirect consume"
);

Operator mdeletedirectOp(
  "mdeletedirect",
  mdeletedirectSpec.getStr(),
  2,
  mdeletedirectVM,
  mdeletedirectSelect,
  mdeletedirectTM
);

/*
7.17 Operator ~mdeletedirectsave~

*/
ListExpr mdeletedirectsaveTM(ListExpr args){
   string err = "stream(tuple(X)) x (mem(rel(tuple(X)))) x "
                "mem(rel(tuple(X@TID))) expected";
   if(!nl->HasLength(args,3)){
      return listutils::typeError("three arguments expected");
   }
   ListExpr stream = nl->First(args);
   if(!Stream<Tuple>::checkType(stream)){
     return listutils::typeError("first arg is not a tuple stream");
   }

   ListExpr mrel;
   if(!getMemSubType(nl->Second(args), mrel)){
     return listutils::typeError("Second argument is not a memory object");
   }
   ListExpr streamAttrList = nl->Second(nl->Second(stream));
   ListExpr relAttrList = nl->Second(nl->Second(mrel));
   if(!nl->Equal(streamAttrList, relAttrList)){
     return listutils::typeError("attributes of stream and relation differ");
   }  
   ListExpr append = nl->OneElemList(nl->TwoElemList(
                            nl->SymbolAtom("TID"),
                            listutils::basicSymbol<TupleIdentifier>()));

   ListExpr resAttrList = listutils::concat( streamAttrList, append);
   if(!listutils::isAttrList(resAttrList)){
     return listutils::typeError("attribute TID already present. ");
   }
   // check the saverel
   ListExpr saverel;
   if(!getMemSubType(nl->Third(args), saverel)){
     return listutils::typeError("third arg is not a memory object");
   }
   ListExpr saveAttrList = nl->Second(nl->Second(saverel));
   if(!nl->Equal(saveAttrList, resAttrList)){
      return listutils::typeError("save relation scheme does not fit "
                                  "the result scheme");
   } 

   return nl->TwoElemList(listutils::basicSymbol<Stream<Tuple> >(),
                          nl->TwoElemList(
                              listutils::basicSymbol<Tuple>(),
                              resAttrList));
}

template<class T, class S>
int mdeletedirectsaveVMT(Word* args, Word& result,
                    int message, Word& local, Supplier s) {

  mdeletedirectInfo* li = (mdeletedirectInfo*) local.addr;

    switch (message) {
      case OPEN: {
        if(li){
          delete li;
          local.addr=0;
        }
        
        T* oN = (T*) args[1].addr;
      
        MemoryRelObject* rel = getMemRel(oN);
        if(!rel) { return 0; }
        
        S* sN = (S*) args[2].addr;
      
        MemoryRelObject* srel = getMemRel(sN);
        if(!srel) { return 0; }
       
        local.addr = new  mdeletedirectInfo(
                            args[0],
                            rel->getmmrel(),
                            srel->getmmrel(),
                            nl->Second(GetTupleResultType(s)));
        return 0;
      }

     case REQUEST:
            result.addr = li?li->next():0;
            return result.addr?YIELD:CANCEL;

     case CLOSE :
             if(li){
                delete li;
                local.addr = 0;
             }
             return 0;
   }
   return -1;
}

ValueMapping mdeletedirectsaveVM[] = {
  mdeletedirectsaveVMT<Mem, Mem>,
  mdeletedirectsaveVMT<Mem, MPointer>,
  mdeletedirectsaveVMT<MPointer, Mem>,
  mdeletedirectsaveVMT<MPointer, MPointer>
};

int mdeletedirectsaveSelect(ListExpr args){
  ListExpr s = nl->Second(args);
  int n1 = 0;
  if(Mem::checkType(s)) n1 = 0;
  else if(MPointer::checkType(s)) n1 = 2;

  s = nl->Third(args);
  int n2 = 0;
  if(Mem::checkType(s)) n2 = 0;
  else if(MPointer::checkType(s)) n2 = 1;

  return n1+n2;
}

OperatorSpec mdeletedirectsaveSpec(
  "stream(tuple(X)) x MREL1(tuple(X)) x MREL2(rel(tuple(X@TID))"
  " -> stream(tuple(X@(TID tid))), MREL1, MREL2 in { mem, mpointer}",
  "_ _ _ mdeletedirectsave",
  "This function extracts the tuple ids from the incoming stream "
  "and removes the tuples having this id from the main memory relation "
  "which can be given as a string, a mem(rel) or an mpointer(mem(rel)). "
  "The output tuples are the incoming tuples extended by this id. "
  "Beside giving the result tuples into the output stream, these "
  "tuples are saved into an auxiliary relation given as the third argument. "
  "Note that "
  "the values of the result tuples have nothing to do with the values "
  "stored in the relation. ",
  "query ten feed mten mten_aux mdeletedirectsave consume"
);

Operator mdeletedirectsaveOp(
  "mdeletedirectsave",
  mdeletedirectsaveSpec.getStr(),
  4,
  mdeletedirectsaveVM,
  mdeletedirectsaveSelect,
  mdeletedirectsaveTM
);


/*
7.19 Operator ~mupdate~

*/


ListExpr mupdateTM(ListExpr args){
  string err = "stream(tid) x {string, mem(rel) } x funlist expected";
  if(!nl->HasLength(args,3)){
    return listutils::typeError(err + " (wrong number of args)");
  }
  ListExpr stream = nl->First(args);
  if(!Stream<TupleIdentifier>::checkType(stream)){
    return listutils::typeError(err+ " (first arg is not a tid stream)");
  }
  ListExpr mem;
  if(!getMemSubType(nl->Second(args),mem)){
    return  listutils::typeError("second argument is not a memory object");
  }

  if(!Relation::checkType(mem)){
     return listutils::typeError("Second arg is not a memory relation");
  }
  ListExpr funList  = nl->Third(args);
  ListExpr tupleList = nl->Second(mem);
  ListExpr attrList = nl->Second(tupleList);

  ListExpr at;
  if(listutils::findAttribute(attrList,"TID", at)){
      return listutils::typeError("attribute TID already present");
  }


  // each element of funlist must be a list consisting
  // of an attribute name in attrList and a function mapping
  // tupleList into the attribute's type
  set<string> used; // used attribute names
  ListExpr appendList = nl->TheEmptyList();
  ListExpr last;
  bool first = true;

  while(!nl->IsEmpty(funList)){
    ListExpr fun = nl->First(funList);
    funList = nl->Rest(funList);
    if(!nl->HasLength(fun,2)){ //(name map)
       return listutils::typeError("found element in typelist having "
                                   "invalid length");
    }
    ListExpr name = nl->First(fun);
    if(nl->AtomType(name)!=SymbolType){
       return listutils::typeError("invalid attribute name");
    } 
    string attr = nl->SymbolValue(nl->First(fun));
    if(used.find(attr)!=used.end()){
      return listutils::typeError("used attribute name " + attr + " twice");
    }
    used.insert(attr);
    ListExpr map = nl->Second(fun);
    ListExpr attrType;
    int attrIndex = listutils::findAttribute(attrList, attr, attrType);
    if(!attrIndex){
      return listutils::typeError("Attribute " + attr 
                                  + " not part of the relation");
    }
    if(first){
      appendList = nl->OneElemList( nl->IntAtom(attrIndex-1));     
      last = appendList;
    } else {
      last = nl->Append(last, nl->IntAtom(attrIndex-1));
    }
    if(!listutils::isMap<1>(map)){
      return listutils::typeError("invalid map definition");
    }
    if(!nl->Equal(nl->Second(map), tupleList)){
      return listutils::typeError("function argument for " + attr +
                              + " does not corresponds to the relation type");
    }
    if(!nl->Equal(nl->Third(map), attrType)){
      return listutils::typeError("function result for " + attr
                             + " does not corresponds to the attr type");
    }
  }
  if(nl->IsEmpty(appendList)){
    return listutils::typeError("function list is empty");     
  }
  
  // create result tuple list
  // step 1: overtake attributes
  ListExpr tmp = attrList;
  ListExpr resAttrList = nl->OneElemList(nl->First(tmp));
  last = resAttrList;
  tmp = nl->Rest(tmp); 
  while(!nl->IsEmpty(tmp)){
     last = nl->Append(last, nl->First(tmp));
     tmp = nl->Rest(tmp);
  }
  // append _old attributes
  tmp = attrList;
  while(!nl->IsEmpty(tmp)){
    ListExpr attr = nl->First(tmp);
    tmp = nl->Rest(tmp);
    string newName = nl->SymbolValue(nl->First(attr))+"_old";
    if(listutils::findAttribute(attrList,newName,at)){
      return listutils::typeError("Attribute " + newName 
                                  + "already known in tuple");
    }
    ListExpr na = nl->TwoElemList(nl->SymbolAtom(newName),
                                  nl->Second(attr));
    last = nl->Append(last, na);
  }
  // append TID
  last = nl->Append(last, nl->TwoElemList( nl->SymbolAtom("TID"),
                                 listutils::basicSymbol<TupleIdentifier>()));

  tupleList = nl->TwoElemList(listutils::basicSymbol<Tuple>(),
                              resAttrList);
  return nl->ThreeElemList(
                 nl->SymbolAtom(Symbols::APPEND()),
                 appendList,
                 nl->TwoElemList( listutils::basicSymbol<Stream<Tuple> >(),
                                  tupleList));
}


class mupdateInfoN{

 public:
   mupdateInfoN(Word& _stream, vector<Tuple*>* _rel,
               vector<int> _attrs,Supplier _funs,
               ListExpr tt ) :stream(_stream),
                   rel(_rel), attrs(_attrs){
       stream.open();
       assert(qp->GetNoSons(_funs) == (int) attrs.size());
       for(int i=0;i<qp->GetNoSons(_funs); i++){
         Supplier s = qp->GetSupplier(qp->GetSupplier(_funs,i),1);
         funs.push_back(s);
         funargs.push_back(qp->Argument(s));
       }
       resType = new TupleType(tt);
   }
   ~mupdateInfoN(){
     stream.close();
     resType->DeleteIfAllowed();
   }

   Tuple* next(){
    TupleIdentifier* tid;
     while( (tid=stream.request()) ){
       if(tid->IsDefined()){
         TupleId id = tid->GetTid();
         if(id>0 && id <= rel->size()){
            Tuple* res = rel->at(id-1);
            if(res){
               res->SetTupleId(id);
               tid->DeleteIfAllowed();
               return updateTuple(res);
            }
         }
       }
       tid->DeleteIfAllowed();
     } // stream exhausted
     return 0;
   }



 private:
    Stream<TupleIdentifier> stream;
    vector<Tuple*>* rel;
    vector<int> attrs;
    vector<Supplier> funs;
    vector<ArgVectorPointer> funargs;
    TupleType* resType;
    Word funres;

    Tuple* updateTuple(Tuple* orig){
       Tuple* res = new Tuple(resType);
       // copy all attributes twice
       for(int i=0;i<orig->GetNoAttributes();i++){
         res->CopyAttribute(i,orig,i);
         res->CopyAttribute(i,orig, i + orig->GetNoAttributes());
       }
       res->PutAttribute(res->GetNoAttributes()-1, 
                         new TupleIdentifier(true, orig->GetTupleId()));

       // update according to the functions
       for(size_t i=0;i<funs.size(); i++){
          Supplier s = funs[i];
          ArgVectorPointer a = funargs[i];
          (*a)[0].setAddr(orig);
          funres.addr = 0;
          qp->Request(s,funres);
          Attribute* resAttr = ((Attribute*)funres.addr)->Clone();
          orig->PutAttribute(attrs[i], resAttr);
          res->PutAttribute(attrs[i], resAttr->Copy());
       }
       return res;

    }
};



template<class T>
int mupdateVMT(Word* args, Word& result,
               int message, Word& local, Supplier s) {

   mupdateInfoN* li = (mupdateInfoN*) local.addr;
   switch(message){
     case OPEN : {
                   if(li){
                     delete li;
                     local.addr = 0;
                   }
                   T* mrel = (T*) args[1].addr;
                   MemoryRelObject* rel = getMemRel(mrel);
                   if(!rel) { 
                      return 0;
                   }
                   vector<int> attrs;
                   for(int i=3; i<qp->GetNoSons(s); i++){
                     attrs.push_back(((CcInt*)args[i].addr)->GetValue());
                   }
                   ListExpr tt = nl->Second(GetTupleResultType(s));
                   local.addr = new mupdateInfoN(args[0],
                                                rel->getmmrel(),
                                                attrs,
                                                qp->GetSon(s,2), tt);
                   return 0;
                 }
      case REQUEST: result.addr = li?li->next():0;
                    return result.addr?YIELD:CANCEL;
      case CLOSE : if(li){
                      delete li;
                      local.addr=0;
                   }
                   return 0; 
   }
   return -1;
}

ValueMapping mupdateVMN[] = {
   mupdateVMT<Mem>,
   mupdateVMT<MPointer>
};

int mupdateSelectN(ListExpr args){
  return Mem::checkType(nl->Second(args))?0:1;
}

OperatorSpec mupdateSpecN(
  "stream(tid) x MREL x funlist -> stream(tuple), "
  "MREL in {mem, mpointer}",
  "<stream> mupdate[ <memrel> ; funs]",
  "Applies functions to the tuples in <memrel> specified by "
  "their id's in the incomming stream and updates the tuples "
  "in the relation. ",
  "query mten mfeed  filter[.No = 6] mupdate[mten, No : .No + 23] count"
);


Operator mupdateNOp(
  "mupdate",
  mupdateSpecN.getStr(),
  2,
  mupdateVMN,
  mupdateSelectN,
  mupdateTM
);


enum UpdateOp {
  MUpdate,
  MUpdateSave,
  MUpdateByID
};

void update(vector<Tuple*>* relation, Tuple* res, 
            int changedIndex, Attribute* attr) {
      
  vector<int> attrPos;
  
  attrPos.push_back(changedIndex+1);
  
  vector<Tuple*>::iterator it = relation->begin();
  
  while(it != relation->end()) {
    Tuple* tup = *it;
    if(tup){
      if(TupleComp::equal(res,tup,&attrPos)) {
        tup->PutAttribute(changedIndex, attr->Clone());
        return;
      }
    }
    it++;
  }
}

Tuple* copyAttributes(Tuple* res, TupleType* type) {
  Tuple* tup = new Tuple(type);
  assert(tup->GetNoAttributes() == 2 * res->GetNoAttributes() + 1);
  for (int i=0; i<res->GetNoAttributes(); i++)
    tup->PutAttribute(res->GetNoAttributes()+i,
                      res->GetAttribute(i)->Clone());
  return tup;
}
    
/*
7.2 Operator ~mupdate~

The Operator ~mupdate~ updates all attributes in the tuples of a main 
memory relation. The tuple to update are given by an input stream, the new
values of the attributes a calculated with a set of given functions.
The updated tuple, including a new attribute of type `tid`, 
are appended to an output stream.

7.16.1 General Type Mapping Funktions for operator ~mupdate~ and
       ~mupdatesave~.

  stream(tid) x mrel x funlist

  stream(tid) x mrel x mrel x funlist 

*/
template<bool save>    
ListExpr mupdateTypeMap(ListExpr args) {

  int noargs = save?4:3;
  if(!nl->HasLength(args,noargs)){
    return listutils::typeError("wrong number of arguments");
  }  

  // process stream
  ListExpr stream = nl->First(args);
  if(!Stream<TupleIdentifier>::checkType(stream)){
    return listutils::typeError("first argument must be a tuple id's");
  }
  
  // process second argument (mem(rel))
  string errMsg;
  ListExpr second;
  if(!getMemSubType(nl->Second(args),second)) {
    return listutils::typeError("2nd argument is not a memory object");
  }
  if(!Relation::checkType(second)) {
    return listutils::typeError("second arg is not a memory relation");
  }
  ListExpr at;
  if(listutils::findAttribute(nl->Second(nl->Second(second)),"TID",at)){
    return listutils::typeError("Relation already contains a TID attribute");
  }

  ListExpr maps = save?nl->Fourth(args):nl->Third(args);

  if(   (nl->AtomType(maps)!=NoAtom)
     || nl->IsEmpty(maps)){
    return listutils::typeError("last arg must be a non-empty list of maps");
  } 

  // check maps
  ListExpr tuple = nl->Second(second);
  ListExpr attrList = nl->Second(tuple);

  ListExpr indexList = nl->TheEmptyList();
  ListExpr lastIndex = indexList;

  // create the result list as a copy from attrlist
  ListExpr newAttrList = nl->OneElemList(nl->First(attrList));
  ListExpr last = newAttrList;
  ListExpr tmp = nl->Rest(attrList);
  while(!nl->IsEmpty(tmp)){
     last = nl->Append(last,nl->First(tmp));
     tmp = nl->Rest(tmp);
  }
  // append the same attributes with additional _old
  tmp = attrList;
  while(!nl->IsEmpty(tmp)){
     ListExpr a = nl->First(tmp);
     tmp = nl->Rest(tmp);
     last = nl->Append(last, nl->TwoElemList( 
                                nl->SymbolAtom(
                                   nl->SymbolValue(nl->First(a))+"_old"),
                                nl->Second(a)));
  } 
  last = nl->Append(last,nl->TwoElemList(
                             nl->SymbolAtom("TID"),
                             listutils::basicSymbol<TupleIdentifier>())); 
  if(!listutils::isAttrList(newAttrList)){
    return listutils::typeError("Found name conflict in result type");
  }

  set<string> used;
  while(!nl->IsEmpty(maps)){
    ListExpr map = nl->First(maps);
    maps = nl->Rest(maps);
    if(!nl->HasLength(map,2)){  // (name map)
       return listutils::typeError("funlist contains not a named function");
    }
    if(  !listutils::isSymbol(nl->First(map))
       ||!listutils::isMap<1>(nl->Second(map))){
       return listutils::typeError("funlist contains not a named function");
    }
    string attrName = nl->SymbolValue(nl->First(map));
    if(used.find(attrName)!=used.end()){
      return listutils::typeError("used " + attrName + " twice");
    }
    used.insert(attrName);
    if(listutils::findAttribute(attrList, attrName+"_old", at)){
      return listutils::typeError(attrName
                   + "_old already part of the relation");
    }
    int index = listutils::findAttribute(attrList, attrName, at);
    if(index==0){
      return listutils::typeError(  "Attribute " + attrName 
                                  + " not part of the relation");
    }
    ListExpr fun = nl->Second(map);
    if(!nl->Equal(nl->Second(fun), tuple)){
      return listutils::typeError("function argument for " + attrName 
                    + " differs from the tuple type");
    } 
    if(!nl->Equal(nl->Third(fun), at)){
      return listutils::typeError("function result for " + attrName 
                    + " differs from the attribute type");
    } 
    // checks successful, extend lists
    if(nl->IsEmpty(indexList)){
       indexList = nl->OneElemList(nl->IntAtom(index-1));
       lastIndex = indexList;
    } else {
       lastIndex = nl->Append(lastIndex,nl->IntAtom(index-1));
    }
  }
  ListExpr newTuple = nl->TwoElemList(
                         listutils::basicSymbol<Tuple>(),
                         newAttrList);


  if(save){ // check third argument
     ListExpr auxRel;
     if(!getMemSubType(nl->Third(args),auxRel)){
       return listutils::typeError("third arg is not a memory object");
     }
     if(!Relation::checkType(auxRel)){
        return listutils::typeError("third arg is not a memory relation");
     }
     if(!nl->Equal(nl->Second(auxRel), newTuple)){
        return listutils::typeError("auxiliarary relation has another type "
                         "than the result");
     }
  }

  return nl->ThreeElemList(
                nl->SymbolAtom(Symbols::APPEND()),
                indexList,
                nl->TwoElemList(
                      listutils::basicSymbol<Stream<Tuple> >(),
                      newTuple));
}

class mupdateInfo {
  public:
    mupdateInfo(Word& _stream, 
                vector<Tuple*>* _mainRelation, 
                bool _flob, 
                Supplier _funs, 
                vector<Tuple*>* _auxRelation,
                vector<int>& _indexes, 
                ListExpr _type) :
       stream(_stream), mainRelation(_mainRelation),
       flob(_flob),  auxRelation(_auxRelation),
       indexes(_indexes) {
         type = new TupleType(_type);
         stream.open();
         for(int i=0;i<qp->GetNoSons(_funs); i++){
            Supplier s = qp->GetSupplier(qp->GetSupplier(_funs,i),1);
            ArgVectorPointer p = qp->Argument(s);
            funs.push_back(s);
            funargs.push_back(p); 
         } 
         assert(indexes.size()==funs.size());

    }


    ~mupdateInfo(){
      stream.close();
      type->DeleteIfAllowed();
    }
    
    Tuple* next() {
       TupleIdentifier* t;
       while( (t=stream.request())){
          if(!t->IsDefined()){
            t->DeleteIfAllowed();
          } else {
            TupleId id = t->GetTid();
            t->DeleteIfAllowed();
            if(id>0 && id <=mainRelation->size()){
               Tuple* tuple = mainRelation->at(id-1);
               if(tuple){
                  return createResultTuple(id, tuple);
               }
            }
          }
       }
       return 0;
    }

  private:
     Stream<TupleIdentifier> stream;
     vector<Tuple*>* mainRelation; 
     bool flob;
     vector<Tuple*>* auxRelation;
     vector<int> indexes;
     vector<Supplier> funs;
     vector<ArgVectorPointer> funargs;
     TupleType* type;
     Word v;

     Tuple* createResultTuple(TupleId id, Tuple* tuple){
         Tuple* res = new Tuple(type);
         // copy all attributes twice into the new tuple
         for(int i=0;i<tuple->GetNoAttributes(); i++){
           res->CopyAttribute(i,tuple,i);
           res->CopyAttribute(i,tuple, i + tuple->GetNoAttributes());
         }
         // put tid into tuple
         res->PutAttribute(res->GetNoAttributes()-1, 
                           new TupleIdentifier(true,id));
         res->SetTupleId(id);
         // update attributes
         for(size_t i=0;i<funs.size();i++){
            (*(funargs[i]))[0] = tuple;
            v.addr = 0;
            qp->Request(funs[i],v);
            Attribute* newAttr = ((Attribute*)v.addr)->Clone();
            if(flob){
             newAttr->bringToMemory();
            }
            res->PutAttribute(indexes[i], newAttr); // put to res
            tuple->PutAttribute(indexes[i], newAttr->Copy()); // update relation
         }
         // save if necessary
         if(auxRelation){
           res->IncReference();
           auxRelation->push_back(res); 
         }
         return res;
     }
};


/*

7.16.3  Value Mapping Function of operator ~mupdate~

*/
template<class T, class S, bool save>
int mupdateValMap (Word* args, Word& result,
                    int message, Word& local, Supplier s) {
  
  mupdateInfo* li = (mupdateInfo*) local.addr;

  switch (message) {
    
    case OPEN : {
      
      if(li) {
        delete li;
        local.addr=0;
      }
      T* oN = (T*) args[1].addr; 
      MemoryRelObject* rel = getMemRel(oN);
      ListExpr tt = nl->Second(GetTupleResultType(s));
      int funindex = 2;
      vector<Tuple*>* auxRel = 0;
      if(save){
         funindex = 3;
         S* aRel = (S*) args[2].addr;
         MemoryRelObject* arel = getMemRel(aRel);
         auxRel = arel->getmmrel();
      }      

      vector<int> indexes;
      for(int i=funindex+1;i<qp->GetNoSons(s);i++){
        indexes.push_back(((CcInt*)args[i].addr)->GetValue());
      }
      local.addr = new mupdateInfo(args[0], // stream
                                   rel->getmmrel(), // main relation
                                   rel->hasflob(), // flob?
                                   qp->GetSon(s,funindex), // funlist
                                   auxRel, // auxiliary relation
                                   indexes,  // attribute indexes
                                   tt); // result tuple type
      return 0;
    }
    
    case REQUEST : {
      result.addr=li?li->next():0;
      return result.addr?YIELD:CANCEL;
    }
    
    case CLOSE :
      if(li) {
        delete li;
        local.addr = 0;
      }
      return 0;
  }
  return 0;
}

ValueMapping mupdateVM[] = {
   mupdateValMap<Mem, Mem, false>,
   mupdateValMap<MPointer, Mem, false>
};

int mupdateSelect(ListExpr args){
   ListExpr t = nl->Second(args);
   if(Mem::checkType(t)) return 0;
   if(MPointer::checkType(t)) return 1;
   return -1;
}



/*
7.16.4 Description of operator ~mupdate~

*/
OperatorSpec mupdateSpec(
    "stream(tuple(x)) x MREL x [(a1, (tuple(x) -> d1)) "
    "... (an, (tuple(x) -> dn))] -> "
    "stream(tuple(x @ [x1_old t1] @...[xn_old tn] @ [TID tid]))),"
    " MREL in {mem, mpointer}",
    "_ mupdate[_; funlist] implicit parameter tuple type TUPLE",
    "updates the tuple of a stream in an "
    "existing main memory relation",
    "query Staedte feed filter[.SName = 'Hannover'] "
    "mupdate['Staedte';Bev: .Bev + 1000] count"
);

/*
7.16.5 Instance of operator ~mupdate~

*/
Operator mupdateOp (
    "mupdate",
    mupdateSpec.getStr(),
    2,
    mupdateVM,
    mupdateSelect,
    mupdateTypeMap<false>
);


ValueMapping mupdatesaveVM[] = {
   mupdateValMap<Mem, Mem, true>,
   mupdateValMap<Mem, MPointer, true>,
   mupdateValMap<MPointer, Mem, true>,
   mupdateValMap<MPointer, MPointer, true>
};






int mupdatesaveSelect(ListExpr args){
   int n1 = Mem::checkType(nl->Second(args))?0:2;
   int n2 = Mem::checkType(nl->Third(args))?0:1;
   return n1+n2;
}



/*
7.17.4 Description of operator ~mupdatesave~

*/
OperatorSpec mupdatesaveSpec(
    "stream(tuple(x)) x MREL1 "
    "x [(a1, (tuple(x) -> d1)) ... (an,(tuple(x) -> dn))] "
    "x MREL2 -> "
    "stream(tuple(x @ [x1_old t1] @...[xn_old tn] @ [TID tid]))),"
    "MREL1, MREL2 in {mem,mpointer}",
    "_ mupdatesave[_,_; funlist] implicit parameter tuple type TUPLE",
    "updates the tuple of a stream in an "
    "existing main memory relation and saves the tuples of the output "
    "stream in an additional main memory relation",
    "query Staedte feed filter[.SName = 'Hannover'] "
    "mupdatesave['Staedte','Stadt';Bev: .Bev + 1000] count"
);

/*
7.17.5 Instance of operator ~mupdatesave~

*/
Operator mupdatesaveOp (
    "mupdatesave",
    mupdatesaveSpec.getStr(),
    4,
    mupdatesaveVM,
    mupdatesaveSelect,
    mupdateTypeMap<true>
);


/*
7.18 Operator ~mupdatebyid~

The Operator ~mupdatebyid~ updates the tuple with the given tupleidentifier
in a main memory relation, using functions to calculate the new attribute 
values. The updated tuple is returned in an output stream with all attributes
copied and extended by the suffix 'old'. Also an attribute of type 'tid' is
added.

7.18.1 Type Mapping Function of operator


*/
ListExpr mupdatebyidTypeMap(ListExpr args) {
  if(nl->ListLength(args) != 3) {
    return listutils::typeError("wrong number of arguments");
  }

  // process first arg
  ListExpr first;
  if(!getMemSubType(nl->First(args), first)){
    return listutils::typeError("first argument is not a memory object");
  }
  if(!Relation::checkType(first)){
    return listutils::typeError("first arg is not a memory relation");
  }

  // process second arg (tid)
  ListExpr second = nl->Second(args);
  if(!listutils::isSymbol(second,TupleIdentifier::BasicType())){
    return listutils::typeError("second argument must be a tid");
  }
    
  // process update function
  ListExpr map = nl->Third(args);
  
  // argument is not a map
  ListExpr maprest = map;
  if(nl->ListLength(maprest<1)) {
    return listutils::typeError("arg must be a list of maps");
  }
  int noAttrs = nl->ListLength(maprest);


  // copy all original attributes to resAttrList
  ListExpr inAttrList = nl->Second(nl->Second(first));
  ListExpr resAttrList = nl->TheEmptyList();
  ListExpr resAttrListLast= nl->TheEmptyList();
  bool firstcall = true;
  while(!nl->IsEmpty(inAttrList)){
    ListExpr attr = nl->First(inAttrList);
    inAttrList = nl->Rest(inAttrList);
    if(firstcall){
       resAttrList = nl->OneElemList(attr);
       resAttrListLast = resAttrList;
       firstcall = false;
    } else {
       resAttrListLast = nl->Append(resAttrListLast, attr);
    }  
  } 
  
  // Go through all functions
  ListExpr mapfirst, mapsecond;
  ListExpr attrType;
  ListExpr indices=nl->TheEmptyList(), indicescurrent = nl->TheEmptyList();
  
  firstcall = true;
  while (!(nl->IsEmpty(maprest))) {
    map = nl->First(maprest);
    maprest = nl->Rest(maprest);
    mapfirst = nl->First(map);
    mapsecond = nl->Second(map);
    
    // check if argument map is a function
    if(!listutils::isMap<1>(mapsecond)){
      return listutils::typeError("not a map found");
    }
    if(!nl->Equal(nl->Second(first),nl->Second(mapsecond))) {
      return listutils::typeError("argument of map is wrong");
    }

    // check for valid attribute name in function
    if(!listutils::isSymbol(mapfirst)){
      return listutils::typeError("not a valid attribute name:" +
                                  nl->ToString(mapfirst));
    }
    string argstr;
    int attrIndex;
    nl->WriteToString(argstr, mapfirst);
    attrIndex = listutils::findAttribute(nl->Second(nl->Second(first)),
                              argstr, attrType);
    if(attrIndex==0){
      return listutils::typeError("attribute " + argstr + " not known");
    }
    if(!nl->Equal(attrType, nl->Third(mapsecond))){
      return listutils::typeError("result of the map and attribute"
                                  " type differ");
    }
    
    // construct list with all indices of the changed attributes
    // in the inputstream to be appended to the resultstream
    if(firstcall) {
      indices = nl->OneElemList(nl->IntAtom(attrIndex));
      indicescurrent = indices;
      firstcall = false;
    } else {
      indicescurrent = nl->Append(indicescurrent,
                                  nl->IntAtom(attrIndex));
    }
    argstr += "_old";
    ListExpr newAttr = nl->TwoElemList( nl->SymbolAtom(argstr),
                                        attrType);
    resAttrListLast= nl->Append(resAttrListLast, newAttr);
  }
  

  // append TID as the last attribute for result
  ListExpr newAttr = nl->TwoElemList(nl->SymbolAtom("TID"),
                                     listutils::basicSymbol<TupleIdentifier>());
  resAttrListLast = nl->Append(resAttrListLast, newAttr);

  
  ListExpr resType = nl->TwoElemList(nl->SymbolAtom(Symbol::STREAM()),
                                     nl->TwoElemList(
                                       nl->SymbolAtom(Tuple::BasicType()),
                                       resAttrList));


  ListExpr res =  nl->ThreeElemList(nl->SymbolAtom(Symbols::APPEND()),
                           nl->TwoElemList(nl->IntAtom(noAttrs),
                                           indices),
                           resType);
  return res;
 
}

class mupdatebyidInfo {
  public:
    mupdatebyidInfo(vector<Tuple*>* _relation,
                    TupleIdentifier* _tid, 
                    Word& _funlist,
                    Word& _indices, 
                    TupleType* _type)
      : relation(_relation), tid(_tid), 
        funlist(_funlist), 
        indices(_indices), 
        type(_type) {
        type->IncReference();
        firstcall = true;
      }
    
    ~mupdatebyidInfo(){
      type->DeleteIfAllowed();
    }
    
    Tuple* next() {
      // returns only a single tuple
      if(!firstcall) return 0;
      firstcall = false;
      // special case tid undefined
      if(!tid->IsDefined()){
          return 0;
      } 
      TupleId id = tid->GetTid();
      // special case tid ouside the allowed range
      if(id<1 || id>relation->size()){
          return 0;
      } 

      Tuple* orig = relation->at(id-1);
      // special case: tuple already removed
      if(!orig) return 0;
      return computeResult(orig);

    }


      
  private:
     vector<Tuple*>* relation;
     TupleIdentifier* tid;
     Word funlist;
     Word indices;
     TupleType* type;
     bool firstcall;
     Word elem, value; 


     Tuple* computeResult(Tuple* orig) {

        Tuple* res = new Tuple(type);

        int noAttr = res->GetNoAttributes() - (orig->GetNoAttributes() + 1);

        // copy all attributes from orig to res
        for(int i=0;i<orig->GetNoAttributes();i++){
          res->CopyAttribute(i,orig,i);
        }

        // collect changed indexes and 
        // computes attributes into vectors

        Word Index;
        vector<int> indexv;
        vector<Attribute*> attrv;


        for(int i=0;i<noAttr;i++){
           // get the attribute id
           Supplier son = qp->GetSupplier(indices.addr,i);
           qp->Request(son,Index);
           int index = ((CcInt*)Index.addr)->GetIntval()-1;
           res->CopyAttribute(index, orig, orig->GetNoAttributes()+i);

           Attribute* replacement = computeFun(orig, i);
           indexv.push_back(index);
           attrv.push_back(replacement);
        }

        // replace attributes in orig and result
        for(int i=0;i<noAttr; i++){
           orig->PutAttribute(indexv[i], attrv[i]);
           res->PutAttribute(indexv[i], attrv[i]->Copy());
        }
        // finally copy the tid into result and change its tid
        res->PutAttribute(res->GetNoAttributes()-1, tid->Copy());
        res->SetTupleId(tid->GetTid());

        return res;
     }

     Attribute* computeFun( Tuple* orig, int funNo){
        Supplier funs = funlist.addr;
        Supplier s1   = qp->GetSupplier(funs, funNo);
        Supplier fun  = qp->GetSupplier(s1, 1);
        ArgVectorPointer funargs = qp->Argument(fun);
        ((*funargs)[0]).setAddr(orig);
        Word value;
        qp->Request(fun,value);
        return  ((Attribute*)value.addr)->Clone();
     }
};


/*

7.18.3  Value Mapping Function of operator ~mupdatebyid~

*/
template<class T>
int mupdatebyidValueMap (Word* args, Word& result,
                    int message, Word& local, Supplier s) {
  
  mupdatebyidInfo* li = (mupdatebyidInfo*) local.addr;
  TupleType*  tt = (TupleType*) qp->GetLocal2(s).addr;

  switch (message) {

   case INIT: {
       tt = new TupleType(nl->Second(GetTupleResultType(s)));
       qp->GetLocal2(s).addr=tt;
       return 0;
    }

    case FINISH: {
       if(tt){
           tt->DeleteIfAllowed();
       }     
       return 0;
    }
    
    case OPEN : {
      if(li){
        delete li;
        local.addr=0;
      }
      
      T* oN = (T*) args[0].addr;
      MemoryRelObject* rel = getMemRel(oN);

      if(!rel) {return 0;}
      
      TupleIdentifier* tid = (TupleIdentifier*)(args[1].addr);
      //int noAttr = ((CcInt*) args[3].addr)->GetValue();
      local.addr = new mupdatebyidInfo(rel->getmmrel(),tid,args[2],
                                       args[4],tt);
      return 0;
    }
    
    case REQUEST : {
      result.addr=li?li->next():0;
      return result.addr?YIELD:CANCEL;
    }
    
    case CLOSE :
      if(li) {
        delete li;
        local.addr = 0;
      }
      return 0;
      
  }
  return 0;

}

ValueMapping mupdatebyidVM[] = {
   mupdatebyidValueMap<Mem>,
   mupdatebyidValueMap<MPointer>
};

int mupdatebyidSelect(ListExpr args){
   return Mem::checkType(nl->First(args))?0:1;
}

/*
7.18.4 Description of operator ~mupdatebyid~

*/
OperatorSpec mupdatebyidSpec(
    "MREL x (tid) x "
    "[(a1, (tuple(x) -> d1)) ... (an,(tuple(x) -> dn))] "
    "-> stream(tuple(x @ [x1_old t1] @...[xn_old tn] @ [TID tid]))),"
    " MREL in{mem, mpointer}",
    "_ mupdatebyid[_; funlist] implicit parameter tuple type MTUPLE",
    "updates the tuple with the given tupleidentifier from "
    "a main memory relation",
    "query Staedte feed filter[.SName = 'Hannover'] "
    "mupdate['Staedte'; Bev: .Bev + 1000] count"
);

/*

7.18.5 Instance of operator ~mupdatebyid~

*/
Operator mupdatebyidOp (
    "mupdatebyid",
    mupdatebyidSpec.getStr(),
    2,
    mupdatebyidVM,
    mupdatebyidSelect,
    mupdatebyidTypeMap
);

/*
7.19 Operator ~mupdatedirect2~

*/
ListExpr mupdatedirect2TM(ListExpr args){
    if(!nl->HasLength(args,4)){
      return listutils::typeError("4 arguments expected");
    }
    ListExpr stream = nl->First(args);
    if(!Stream<Tuple>::checkType(stream)){
      return listutils::typeError("first argument is not a tuple stream");
    }
    ListExpr tt1 = nl->Second(stream);

    ListExpr  mrel;
    if(!getMemSubType(nl->Second(args),mrel)){
      return listutils::typeError("second arg is not a memory object");
    }

    if(!Relation::checkType(mrel)){
      return listutils::typeError("Second argument is not a memory relation");
    }

    ListExpr Attr = nl->Third(args);
    if(nl->AtomType(Attr) != SymbolType){
      return listutils::typeError("Third arg is not a valid attribute name");
    } 
    ListExpr funlist = nl->Fourth(args);
    if(nl->AtomType(funlist) != NoAtom){
       return listutils::typeError("fourth arg is not a list of functions");
    }
    int noFuns = nl->ListLength(funlist);
    if(noFuns < 1){
       return listutils::typeError("at least 1 function required");
    }
    ListExpr type;
    ListExpr attrList = nl->Second(nl->Second(mrel));
    string attrName = nl->SymbolValue(Attr);
    
    int tidIndex = listutils::findAttribute(nl->Second(nl->Second(stream)),
                                            attrName, type);
    if(!tidIndex){
       return listutils::typeError("Attribute " + attrName 
                                   + " not part of the tuple stream");
    } 
    if(!TupleIdentifier::checkType(type)){
       return listutils::typeError("Attribute " + attrName
                                   + " not of type tid");
    }

    
    ListExpr tt2 = nl->Second(mrel);
    attrList = nl->Second(tt2);

    ListExpr funindexes=nl->TheEmptyList();
    ListExpr last = nl->TheEmptyList();
    set<int> used;
    bool first = true;
    while(!nl->IsEmpty(funlist)){
       ListExpr namedFun = nl->First(funlist);
       funlist = nl->Rest(funlist);
       if(!nl->HasLength(namedFun,2)){
         return listutils::typeError("invalid named function in list");
       }
       Attr = nl->First(namedFun);
       if(nl->AtomType(Attr) != SymbolType){
          return listutils::typeError("invalid function name found");
       }
       attrName = nl->SymbolValue(Attr);
       int attrIndex = listutils::findAttribute(attrList, attrName, type);
       if(used.find(attrIndex)!=used.end()){
          return listutils::typeError("found more than one function for " 
                                      + attrName);
       }
       used.insert(attrIndex);
       if(!attrIndex){
         return listutils::typeError("Attribute " + attrName 
                                     + " not found in Relation");
       }
       ListExpr fun = nl->Second(namedFun);
       if(!listutils::isMap<2>(fun)){
         return listutils::typeError("no binary fun for attribute" + attrName);
       }
       // check that the funtion argument ist the relation tuple
       if(!nl->Equal(tt1, nl->Second(fun))){
          return listutils::typeError("first function argument and tuple"
                                      " type of stream differ");
       }
       if(!nl->Equal(tt2, nl->Third(fun))){
          return listutils::typeError("second function argument and tuple"
                                      " type of relation differ");
       }
       // check that the function result corresponds to the attribute type
       if(!nl->Equal(type, nl->Fourth(fun))){
         return listutils::typeError("function result and type of " 
                                     + attrName + " differ");
       }
       // append attrIndex to indexes
       if(first){
          funindexes = nl->OneElemList(nl->IntAtom(attrIndex - 1 ));
          last = funindexes;
          first = false;
       } else {
          last = nl->Append(last,nl->IntAtom(attrIndex - 1));
       }
    }

    ListExpr resType = nl->TwoElemList(
                          listutils::basicSymbol<Stream<Tuple> >(),
                          tt2);    

    ListExpr res =  nl->ThreeElemList(
             nl->SymbolAtom(Symbols::APPEND()),
             nl->ThreeElemList(nl->IntAtom(tidIndex-1), 
                               nl->IntAtom(noFuns),
                               funindexes),
             resType  
           );
    return res; 

}


class mupdatedirect2LocalInfo{

  public:
     mupdatedirect2LocalInfo(Word _stream, vector<Tuple*>* _relation,
                        int _tidIndex, Word _funs, vector<int>& _funindexes):
         stream(_stream), relation(_relation), tidIndex(_tidIndex), 
         funindexes(_funindexes)
     {

        stream.open();
        for(size_t i=0;i<_funindexes.size();i++){
          Supplier s1 = qp->GetSupplier(_funs.addr,i);
          Supplier fun = qp->GetSupplier(s1,1);
          ArgVectorPointer avp = qp->Argument(fun);
          funs.push_back(fun);
          funargs.push_back(avp); 
        }
     }
     ~mupdatedirect2LocalInfo(){
        stream.close();
     }

   Tuple* next(){
      Tuple* tuple = stream.request();
      while(tuple) {
         TupleIdentifier* Tid = (TupleIdentifier*)tuple->GetAttribute(tidIndex);
         if(Tid->IsDefined()){
            TupleId tid = Tid->GetTid();
            if((tid > 0) && (tid <=  relation->size())){
              Tuple* relTuple = relation->at(tid-1);
              if(relTuple){
                Tuple* resTuple = updateTuple(tuple, relTuple);
                tuple->DeleteIfAllowed();
                resTuple->SetTupleId(tid);
                return resTuple;
              }
            }
         }
         tuple->DeleteIfAllowed();
         tuple = stream.request();
      }
      return 0;
   }


   private:
      Stream<Tuple> stream;
      vector<Tuple*>* relation;
      int tidIndex;
      vector<int> funindexes;
      vector<Supplier> funs;
      vector<ArgVectorPointer> funargs; 
      Word result;

      Tuple* updateTuple(Tuple* streamTuple, Tuple* relTuple){
         vector<Attribute*> uattrs;
         for(size_t i=0;i<funs.size();i++){
            ArgVectorPointer avp = funargs[i];
            (*avp)[0].addr = streamTuple;
            (*avp)[1].addr = relTuple;
            qp->Request(funs[i], result);
            uattrs.push_back( ((Attribute*) result.addr)->Clone());
         }
         for(size_t i=0;i<funindexes.size();i++){
            relTuple->PutAttribute(funindexes[i], uattrs[i]);
         }
         Tuple* resTuple = new Tuple(relTuple->GetTupleType());
         for(int i=0;i<relTuple->GetNoAttributes();i++){
            resTuple->CopyAttribute(i,relTuple,i);
         }
         return resTuple; 
      }
};



template<class T>
int mupdatedirect2VMT (Word* args, Word& result,
                    int message, Word& local, Supplier s) {
  
  mupdatedirect2LocalInfo* li = (mupdatedirect2LocalInfo*) local.addr;

  switch (message) {

    
    case OPEN : {
      if(li){
        delete li;
        local.addr=0;
      }
      
      T* oN = (T*) args[1].addr;
      MemoryRelObject* rel = getMemRel(oN);

      if(!rel) {return 0;}

      int tidIndex = ((CcInt*) args[4].addr)->GetValue();
      int noFuns = ((CcInt*) args[5].addr)->GetValue();
      vector<int> funindexes;
      Word elem;
      for(int i=0;i<noFuns;i++){
         Supplier s = qp->GetSupplier(args[6].addr, i);
         qp->Request(s,elem);
         int index = ((CcInt*) elem.addr)->GetValue();
         funindexes.push_back(index);
      } 
 
      local.addr = new mupdatedirect2LocalInfo(args[0], rel->getmmrel(), 
                                              tidIndex, 
                                              args[3], funindexes);  
    
      return 0;
    }
    
    case REQUEST : {
      result.addr=li?li->next():0;
      return result.addr?YIELD:CANCEL;
    }
    
    case CLOSE :
      if(li) {
        delete li;
        local.addr = 0;
      }
      return 0;
      
  }
  return 0;

}


ValueMapping mupdatedirect2VM[] = {
   mupdatedirect2VMT<Mem>,
   mupdatedirect2VMT<MPointer>
};

int mupdatedirect2Select(ListExpr args){
   return Mem::checkType(nl->Second(args))?0:1;
}

/*
7.18.4 Description of operator ~mupdatedirect2~

*/
OperatorSpec mupdatedirect2Spec(
    "stream(tuple(X)  x mrel(tuple(Y) x IDENT x funlist -> stream(tuple(Y))",
    "_ _ mupdatedirect2[_; funlist]",
    "The first argument represents a stream of tuple containing a TID "
    "attribute. "
    "The second represents a main memory relation. " 
    "The third argument specifies the name of the tid attribute in the stream."
    "The last argument is a list of named functions taking a tuple(X) and "
    "a tuple(Y) producing an attribute according to the name's type "
    " in the relation. The operator updates the tuples in the relation "
    " with the tid coming from the tuple stream by replacing the attributes "
    " in the funlist by the function results. The result is the incoming tuple "
    " stream. Tuples having containing a tid that point to non existing tuples" 
    " in the relation are removed from the output stream.",
    "query ten feed filter[.No > 3] addid mten "
    "mupdatedirect2[TID; No : .No + ..No * 10] consume"
);

/*

7.18.5 Instance of operator ~mupdatebyid~

*/
Operator mupdatedirect2Op (
    "mupdatedirect2",
    mupdatedirect2Spec.getStr(),
    2,
    mupdatedirect2VM,
    mupdatedirect2Select,
    mupdatedirect2TM
);





/////////////////////////////////////// ORDERED RELATION ////
enum ChangeType{
  MOInsert,
  MODelete,
  MOUpdate
};



/*
7.19 Operator ~moinsert~

The Operator ~moinsert~ insert all tuples of an input stream into an 
ordered main memory relation. The tuples of the input stream are given 
an additional attribute of type 'tid' and added to the output stream.

*/
template<ChangeType ct>
class moinsertInfo {
  public:
         
    moinsertInfo(Word& w, ttree::TTree<TupleWrap,TupleComp>* _orel, 
                 bool _flob, TupleType* _type,vector<int>* _attrPos) 
      : stream(w),orel(_orel),flob(_flob),type(_type),attrPos(_attrPos) {
      stream.open();
    }

  ~moinsertInfo() {
      stream.close();
      type->DeleteIfAllowed();
    }

    Tuple* next() {
      Tuple* res = stream.request();
      if(!res) { return 0; }
      
      if((ct==MOInsert) && flob) {
        res->bringToMemory();
      }
      
      // copy attributes of tuple from input stream
      Tuple* newtup = new Tuple(type); 
      for(int i = 0; i < res->GetNoAttributes(); i++){
        newtup->CopyAttribute(i,res,i);
      }
      
      TupleId tid = res->GetTupleId();
      switch(ct){
        case MOInsert : {
                          TupleWrap tw(res);
                          orel->insert(tw,attrPos); 
                        }
                        break;
        case MODelete : {
                          TupleWrap tw(res);
                          orel->remove(tw, attrPos); 
                        }
                        break;
        default : assert(false);
      }
      res->DeleteIfAllowed(); // remove old tuple
      
      //get tuple id and append it to the stored tuple
      Attribute* tidAttr = new TupleIdentifier(true,tid);
      newtup->PutAttribute(newtup->GetNoAttributes()-1, tidAttr);
     
       
      return newtup;
    }

  private:
     Stream<Tuple> stream;
     ttree::TTree<TupleWrap,TupleComp>* orel;
     bool flob;
     TupleType* type;
     vector<int>* attrPos;
};


/*

7.19.2  Value Mapping Functions of operator ~moinsert~

*/
template<class T, ChangeType ct>
int moinsertValueMap (Word* args, Word& result,
                    int message, Word& local, Supplier s) {
  
  moinsertInfo<ct>* li = (moinsertInfo<ct>*) local.addr;

  switch (message) {
    
    case OPEN : {
      
      if(li) {
        delete li;
        local.addr=0;
      }
      
      qp->Open(args[0].addr);
      
      T* oN = (T*) args[1].addr;
      ListExpr ttype = nl->Second(qp->GetType(qp->GetSon(s,0)));
      MemoryORelObject* orel = getMemORel(oN,ttype);
      if(!orel) { return 0; }
      
      TupleType* tt = new TupleType(nl->Second(GetTupleResultType(s)));
      
      local.addr = new moinsertInfo<ct>(args[0],orel->getmmorel(),
                                        orel->hasflob(),tt,
                                        orel->getAttrPos());
      
      return 0;
    }
    
    case REQUEST : {
      result.addr=li?li->next():0;
      return result.addr?YIELD:CANCEL;
    }
    
    case CLOSE :
      if(li) {
        delete li;
        local.addr = 0;
      }
      return 0;
      
  }
  return 0;

}

ValueMapping moinsertVM[] = {
   moinsertValueMap<Mem,MOInsert>,
   moinsertValueMap<MPointer,MOInsert>,
   moinsertValueMap<Mem,MODelete>,
   moinsertValueMap<MPointer,MODelete>
};


template<ChangeType ct>
int moinsertSelect(ListExpr args){
  ListExpr t = nl->Second(args);
  int n = -1;
  if(Mem::checkType(t)) n = 0;
  if(MPointer::checkType(t)) n = 1;
  if(n<0) return -1;
  if(ct == MOInsert) 
    return n;
  if(ct == MODelete)
    return n+2;
  return -1;
}



/*
7.19.4 Description of operator ~moinsert~

*/
OperatorSpec moinsertSpec(
    "stream(tuple(x)) x iMREL -> "
    "stream(tuple(x@[TID:tid])), MREL in {string, mem, mpointer}",
    "_ moinsert [_]",
    "inserts the tuple of a stream into an "
    "existing main memory ordered relation",
    "query moinsert (ten feed head[5],moten) count"
);

/*
7.19.5 Instance of operator ~moinsert~

*/
Operator moinsertOp (
    "moinsert",
    moinsertSpec.getStr(),
    4,
    moinsertVM,
    moinsertSelect<MOInsert>,
    minsertTypeMap<2,true>
);


/*
7.20 Operator ~modelete~

The operator ~modelete~ deletes all tuple of the input stream from the given
main memory ordered relation. The tuples of the input stream including an 
attribute of type 'tid' are appended to the output stream.

7.20.4 Description of operator ~modelete~

*/
OperatorSpec modeleteSpec(
    "stream(tuple(x)) x MREL "
    "-> stream(tuple(x@[TID:tid])), MREL in {string, mem, mpointer}",
    "_ modelete [_]",
    "all tuples of an input stream will be deleted from an "
    "main memory ordered relation",
    "query ten feed head[5] modelete['oten'] count"
);

/*
7.20.5 Instance of operator ~modelete~

*/
Operator modeleteOp (
    "modelete",
    modeleteSpec.getStr(),
    4,
    moinsertVM,
    moinsertSelect<MODelete>,
    minsertTypeMap<2,true>
);


/*
7.21 Operator ~moconsume~

  The Operator ~moconsume~ creates an MemoryORelOBject for output on
  an user interface, i.e. the console from
  a input stream and the attribute identifiers over which the main
  memory ordered relation will be sorted.

7.21.1 Type Mapping Functions of operator ~moconsume~

stream(tuple(x)) x (ident1 ... identn) -> mem(orel(tuple(tuple(x))))

*/
ListExpr moconsumeTypeMap(ListExpr args) {
  
    if(nl->ListLength(args)!=2){
        return listutils::typeError("(wrong number of arguments)");
    }
    if(!Stream<Tuple>::checkType(nl->First(args))) {
        return listutils::typeError ("stream(tuple) expected!");
    }
    ListExpr tupleDescr  = nl->Second(nl->First(args));
    ListExpr attrList = nl->Second(tupleDescr);
    ListExpr keyList = nl->Second(args);
    if(nl->AtomType(keyList)!=NoAtom){
      return listutils::typeError("expected list of attribute names "
                                  "as the second argument");
    }
    if(nl->IsEmpty(keyList)){
      return listutils::typeError("keylist must not be empty");
    } 
    ListExpr appendList = nl->TheEmptyList();
    ListExpr last = nl->TheEmptyList();
    bool first = true;
    while(!nl->IsEmpty(keyList)){
       ListExpr currentKey = nl->First(keyList);
       keyList = nl->Rest(keyList);
       if(nl->AtomType(currentKey) != SymbolType){
         return listutils::typeError("found invalid attribute name in"
                                     " key list");
       }
       string name = nl->SymbolValue(currentKey);
       ListExpr attrType;
       int index = listutils::findAttribute(attrList, name, attrType);
       if(!index){
         return listutils::typeError(name + " is not a member of the tuple");
       }
       if(first){
          appendList = nl->OneElemList(nl->IntAtom(index));
          last = appendList;
          first = false;
       } else {
          last = nl->Append(last, nl->IntAtom(index));
       }
    }


    ListExpr orel = nl->ThreeElemList(
                           listutils::basicSymbol<OrderedRelation>(),
                           tupleDescr,
                           nl->Second(args));

    ListExpr resType =  MPointer::wrapType(Mem::wrapType(orel));
    return nl->ThreeElemList( nl->SymbolAtom(Symbols::APPEND()),
                              appendList,
                              resType);
}


/*
7.21.2  Value Mapping Function of operator ~moconsume~

*/
template<bool flob>
int moconsumeVM (Word* args, Word& result,
                int message, Word& local, Supplier s) {
    
   
    vector<int>* pos = new vector<int>();
    for(int i=2;i<qp->GetNoSons(s);i++){
      pos->push_back( ((CcInt*) args[i].addr)->GetValue());
    }
    ttree::TTree<TupleWrap,TupleComp>* tree;
    tree  = new ttree::TTree<TupleWrap,TupleComp>(16,18);
    ListExpr objectType = nl->Second(qp->GetType(s));

    MemoryORelObject* mor = new MemoryORelObject(tree,pos,0,
                                   nl->ToString(objectType), flob,
                                   getDBname());
 
    Stream<Tuple> stream(args[0]);
    stream.open();
    Tuple* tup = 0;
    bool ok = true;
    while( ((tup = stream.request()) != 0) && ok) {
      ok = mor->addTuple(tup);
      tup->DeleteIfAllowed();
    }
    stream.close();
    result = qp->ResultStorage(s);
    MPointer* res = (MPointer*) result.addr;
    res->setPointer(mor);
    mor->deleteIfAllowed();
    return 0;
}

/*
7.21.4 Description of operator ~moconsume~

*/
OperatorSpec moconsumeSpec(
    "stream(Tuple) x (ident1 ... identn) "
    "->(mpointer (mem(orel (tuple(x)) (ident1 ... identn))))",
    "_ moconsume[list]",
    "collects the objects from a stream(tuple) into a memory ordered relation",
    "query mten mfeed head[2] moconsume[No]"
);


/*
7.21.5 Instance of operator ~moconsume~

*/
Operator moconsumeOp (
    "moconsume",
    moconsumeSpec.getStr(),
    moconsumeVM<false>,
    Operator::SimpleSelect,
    moconsumeTypeMap
);


/*
7.21.4 Description of operator ~moconsumeflob~

*/
OperatorSpec moconsumeflobSpec(
    "stream(Tuple) x (ident1 ... identn) "
    "->(mpointer (mem(orel (tuple(x)) (ident1 ... identn))))",
    "_ moconsumeflob[list]",
    "collects the objects from a stream(tuple) into a memory ordered relation",
    "query mten mfeed head[2] moconsumeflob[No]"
);


/*
7.21.5 Instance of operator ~moconsumeflob~

*/
Operator moconsumeflobOp (
    "moconsumeflob",
    moconsumeflobSpec.getStr(),
    moconsumeVM<true>,
    Operator::SimpleSelect,
    moconsumeTypeMap
);


/*
7.24 Operators ~morange~, ~moleftrange~, ~morightrange~
        In case of ~morange~ finds all tuples in the given MemoryORelObject 
        (as first argument) which are between the first and the second 
        attribute value (as second and third argument).
        In case of ~moleftrange~, ~morightrange~ finds all tuples in 
        the given MemoryORelObject (as first argument) which are left or 
        right of the given value (as second argument).

7.24.1 General Type Mapping Functions of operators ~morange~, ~moleftrange~ 
       and ~morightrange~

{string, mem(orel(tuple(x)))}  x T x T -> stream(tuple(x)) or

{string, mem(orel(tuple(x)))}  x T -> stream(tuple(x))

*/
enum RangeKind {
  LeftRange,
  RightRange,
  Range
};

template<RangeKind rk>
ListExpr morangeTypeMap(ListExpr args) {
    // if morange
    if(rk==Range && nl->ListLength(args)!=3){
        return listutils::typeError("three arguments expected");
    }
    // if moleftrange or morightrange
    if((rk==LeftRange || rk==RightRange) && nl->ListLength(args)!=2){
        return listutils::typeError("two arguments expected");
    }

    ListExpr a1 = nl->First(args);      //mem(orel)
    ListExpr a2 = nl->Second(args);
    // if morange
    ListExpr a3 = nl->TheEmptyList();        

    string err;
    if(rk==Range) {
      err = "{mpointer, mem(orel)} x T x T expected";
    } else {
      err = "{mpointer, mem(orel)} x T expected"; 
    }
  
    string errMsg;
    if(!getMemSubType(nl->First(args), a1)){
      return listutils::typeError("first argument is not a memory object");
    }

    // process MemoryORelObject
    if(!listutils::isOrelDescription(a1)) {
      return listutils::typeError(err + " (first arg is not an orel)");
    }
    
    // extract type of key2 if morange
    if(rk==Range) {
      a3 = nl->Third(args);  
    } else {
      a3 = a2;
    }
    if(!nl->Equal(a2,a3)){
      return listutils::typeError("third argument and second argument "
                                  "have different types");
    }

  
    ListExpr attrList = nl->Second(nl->Second(a1));
    ListExpr attrNames = nl->Third(a1);
    ListExpr typeList = nl->TheEmptyList();
    ListExpr lastType = nl->TheEmptyList();
    ListExpr indexList = nl->TheEmptyList();
    ListExpr lastIndex = nl->TheEmptyList();
    bool first = true;

    // extract the types from the key attributes and collect their positions
    while(!nl->IsEmpty(attrNames)){
       ListExpr attrName = nl->First(attrNames);
       attrNames = nl->Rest(attrNames);
       if(nl->AtomType(attrName)!=SymbolType){
         return listutils::typeError("error in orel description");
       }
       string name = nl->SymbolValue(attrName);
       ListExpr type;
       int index = listutils::findAttribute(attrList, name, type);
       if(!index){
          return listutils::typeError("invalid orel description");
       }
       if(first){
         typeList = nl->OneElemList(type);
         lastType = typeList;
         indexList = nl->OneElemList( nl->IntAtom(index-1));
         lastIndex = indexList;
         first = false;
       } else {
         lastType = nl->Append(lastType, type);
         lastIndex = nl->Append(lastIndex, nl->IntAtom(index-1));
       }
    }

    if(!nl->Equal(typeList, a2)){
       return listutils::typeError("key attribute type of orel does not fit "
                                   "the boundary attributes");
    }
    if(!nl->Equal(typeList, a3)){
       return listutils::typeError("key attribute type of orel does not fit "
                                   "the max boundary attributes (1)");
    }

    ListExpr resType =  nl->TwoElemList(nl->SymbolAtom(Symbol::STREAM()),
                                        nl->Second(a1)); 
    return nl->ThreeElemList(nl->SymbolAtom(Symbols::APPEND()),
                             indexList,
                             resType);
}

class morangeInfo{
    public:
      
      morangeInfo(ttree::TTree<TupleWrap,TupleComp>* _tree, 
                  TupleWrap _min,
                  TupleWrap _max, 
                  vector<int> _attrPos)
          : tree(_tree),max(_max),attrPos(_attrPos) {
        
        if(_min()==0){ // no left boundary, start from beginning
           iter = tree->begin();
        } else {
           iter = tree->tail(_min, &attrPos);
        }
      }
      
      ~morangeInfo(){}


      Tuple* next() {
         if(iter.end()){ // scan finished
            return 0;
         }
         Tuple* result = (*iter).getPointer();
         if(!result){
           return 0;             
         }
         if(max()==0){ // no right boundary 
            result->IncReference();
            iter++;
            return result;
         }
         // check right boundary
         if(TupleComp::smaller(max(), result, &attrPos)){
           return 0;
         } 
         result->IncReference();
         iter++;
         return result;
      }
    
     private:
        ttree::TTree<TupleWrap,TupleComp>* tree;
        TupleWrap max;
        vector<int> attrPos;
        ttree::Iterator<TupleWrap,TupleComp> iter;
};

/*
7.24.2 The Value Mapping Function of operator ~mrange~, ~moleftrange~ and 
       ~morightrange~

*/
template<class T, RangeKind rk>
int morangeVMT (Word* args, Word& result,
                    int message, Word& local, Supplier s) {

   
    morangeInfo* li = (morangeInfo*) local.addr;
      
    switch (message) {
       
      case OPEN: {
        
            if(li){
                delete li;
                local.addr=0;
            }

            T* orelN = (T*) args[0].addr;
            MemoryORelObject* orel = 
                getMemORel(orelN, nl->Second(qp->GetType(s)));
            if(!orel){
              return 0; 
            }
            Tuple* minTuple = 0;
            Tuple* maxTuple = 0;           
            vector<int> positions; 
            // collect minTuple, maxTuple and 
            // positions            
            TupleType* tt = new TupleType(nl->Second(GetTupleResultType(s)));
            int keyPos = rk==Range?3:2;
            for(int i=keyPos;i<qp->GetNoSons(s); i++){
               positions.push_back(((CcInt*)args[i].addr)->GetValue());
            }
            // collect minTuple
            int maxpos = 1;
            if(rk!=LeftRange){ // minValue is required for rightrange and range
               maxpos = 2;
               minTuple = new Tuple(tt);
               Word aw;
               for(size_t i=0;i<positions.size();i++){ 
                  Supplier son = qp->GetSupplier(args[1].addr,i);
                  qp->Request(son,aw);
                  Attribute* attr = (Attribute*) aw.addr;
                  minTuple->PutAttribute(positions[i], attr->Copy());
               }
            }

            // collect maxTuple
            if(rk!=RightRange){ // maxValue is required for leftrange and range
               maxTuple = new Tuple(tt);
               Word aw;
               for(size_t i=0;i<positions.size();i++){ 
                  Supplier son = qp->GetSupplier(args[maxpos].addr,i);
                  qp->Request(son,aw);
                  Attribute* attr = (Attribute*) aw.addr;
                  maxTuple->PutAttribute(positions[i], attr->Copy());
               }
            }
            // the tuple comparator vector starts counting with 1
            for(size_t i=0;i<positions.size();i++){ 
               positions[i]++;
            }  

            local.addr = new morangeInfo(orel->getmmorel(),
                                         minTuple,maxTuple, 
                                         positions); 
            if(minTuple){
               minTuple->DeleteIfAllowed();
            }
            if(maxTuple){
               maxTuple->DeleteIfAllowed();
            }
            tt->DeleteIfAllowed();
            return 0;
        }

        case REQUEST:
            result.addr=li?li->next():0;
            return result.addr?YIELD:CANCEL;

        case CLOSE:
            if(li) {
              delete li;
              local.addr = 0;
            }
            return 0;
   }
   return -1;
}


ValueMapping morangeVM[] = {
    morangeVMT<Mem,Range>,
    morangeVMT<MPointer,Range>,

    morangeVMT<Mem,LeftRange>,
    morangeVMT<MPointer,LeftRange>,

    morangeVMT<Mem,RightRange>,
    morangeVMT<MPointer,RightRange>
};

template<RangeKind rk>
int morangeSelect(ListExpr args){
  int n = Mem::checkType(nl->First(args))?0:1;
  if(n<0) return -1;
  switch(rk){
    case Range : return n;
    case LeftRange : return n + 2;
    case RightRange : return n + 4;
  }


}

/*
7.24.4 Description of operator ~morange~

*/
OperatorSpec morangeSpec(
    "MOREL  x T x T -> stream(tuple(x)), MOREL in{mem, mpointer}",
    "_ morange [_,_]",
    "returns all tuples in a main memory ordered relation whose attributes "
    "are lying between the two given values",
    "query 'oten' morange[2,2] count"
);


/*
7.25.4 Description of operator ~moleftrange~

*/
OperatorSpec moleftrangeSpec(
    "MOREL x T -> stream(tuple(x)), MOREL in {mem, mpointer}",
    "_ moleftrange [_]",
    "returns all tuples in a main memory ordered relation whose attributes "
    "are smaller than the given value",
    "query 'oten' moleftrange[2] count"
);

/*
7.26.4 Description of operator ~morightrange~

*/
OperatorSpec morightrangeSpec(
    "MOREL  x T -> stream(tuple(x)), MOREL in {mem, mpointer}",
    "_ morightrange [_]",
    "returns all tuples in a main memory ordered relation whose attributes "
    "are larger than the given value",
    "query 'oten' morightrange[8] count"
);



/*
7.24.5 Instance of operator ~morange~

*/
Operator morangeOp (
    "morange",
    morangeSpec.getStr(),
    6,
    morangeVM,
    morangeSelect<Range>,
    morangeTypeMap<Range>
);

/*
7.25.5 Instance of operator ~moleftrange~

*/
Operator moleftrangeOp (
    "moleftrange",
    moleftrangeSpec.getStr(),
    6,
    morangeVM,
    morangeSelect<LeftRange>,
    morangeTypeMap<LeftRange>
);

/*
7.26.5 Instance of operator ~morightrange~

*/
Operator morightrangeOp (
    "morightrange",
    morightrangeSpec.getStr(),
    6,
    morangeVM,
    morangeSelect<RightRange>,
    morangeTypeMap<RightRange>
);


enum SPType{
  DIJKSTRA,
  ASTAR
};

Tuple* findTuple(ttree::TTree<TupleWrap,TupleComp>* mmorel, 
                 int nr, int prev) {
    
  CcInt* startNodeInt = new CcInt(true,prev);
  CcInt* endNodeInt = new CcInt(true,nr);

  ttree::Iterator<TupleWrap,TupleComp> it = mmorel->begin();
  
  Tuple* tup=0;
  while(!it.end()) {
    tup = (*it).getPointer();
    if(!tup)
      return 0;
    // find all tuples with startnode as first argument
    if(tup->GetAttribute(0)->Compare(startNodeInt) < 0) 
      it++;
    // find EndNode in second argument
    else if(tup->GetAttribute(1)->Compare(endNodeInt) < 0)
      it++;
    else break;
  }

  startNodeInt->DeleteIfAllowed();
  endNodeInt->DeleteIfAllowed();
  return tup;
}


bool appendTuple(Tuple* tup,TupleType* tt, 
                 int seqNo,
                 ttree::TTree<TupleWrap,TupleComp>* result) {
  Tuple* newTuple = new Tuple(tt);
  int i = 0;
  for(; i<tup->GetNoAttributes(); i++) {
    newTuple->CopyAttribute(i,tup,i);
  }
  int s = seqNo;
  CcInt* noOfNodes = new CcInt(true, s);
  newTuple->PutAttribute(i,noOfNodes);
  TupleWrap tw(newTuple);
  result->insert(tw,i+1);   // sort by SeqNo
  newTuple->DeleteIfAllowed();
  return true;
}

QueueEntryWrap findNextNode(
           ttree::TTree<QueueEntryWrap,EntryComp>* visitedNodes,
           QueueEntryWrap& current) {
    
  ttree::Iterator<QueueEntryWrap,EntryComp> iter = visitedNodes->begin();
  while(!iter.end()) {
    QueueEntryWrap entry = *iter;

    if(entry.getPointer()->nodeNumber == current()->prev) {
      return entry;
    }
    iter++;
  }
  return 0;
}


/*

7.27 Operator ~moshortestpathd~ and ~moshortestpatha~

The Operator ~moshortestpathd~ and ~moshortestpatha~ calculate the shortest
path between two given nodes of a graph. The graph is in this case represented
by a main memory ordered relation. The tuples of such a relation need to
have two integer values as their first two attributes identifying the start and
end node of an edge and at least one double value containing the cost
the edge.
The main memory relation must be ordered first by the start nodes of the tuples
then by the end nodes.
The operator expects an main memory ordered relation, two integers describing
the start and goal node of the path, an integer choosing the resulting output
of the shortest path calculation, as well as an cost function, which maps
the values of the tuples to a double value. The Operator ~moshortestpatha~
expects an additional function calculating the distance to the goal node.
The following results can be selected by integer: 0 shortest path,
1 remaining priority queue at end of computation, 2 visited sections of 
shortest path search, 3 shortest path tree.
For all cases an additional attribute 'seqNo' of type integer will be appended
to the tuples of the output stream.
The Operator ~moshortestpathd~ uses Dijkstras Algorithm and the Operator
~moshortestpatha~ uses the AStar-Algorithm.

7.27.1 General Type Mapping for Operators ~moshortestpathd~ and
       ~moshortestpatha~

{string, mem(orel(tuple(X)))} x int x int x int x (tuple->real)
-> stream(tuple(a1:t1,...an+1:tn+1)) and

{string, mem(orel(tuple(X)))} x int x int x int x (tuple->real) 
x (tuple->real) -> 
stream(tuple(a1:t1,...an+1:tn+1))

*/
template<SPType sptype>
ListExpr moshortestpathTypeMap(ListExpr args) {
  
  if(sptype == DIJKSTRA && nl->ListLength(args) != 5) {
    return listutils::typeError("moshortestpath expects 5 arguments");
  }
  
  if(sptype == ASTAR && nl->ListLength(args) != 6) {
    return listutils::typeError("moshortestpath expects 6 arguments");
  }

  // process first arg
  ListExpr a1;
  if(!getMemSubType(nl->First(args),a1)){
    return listutils::typeError("first arg is not a memory object");
  }

  //MemoryORelObject
  if(!listutils::isOrelDescription(a1)) {
    return listutils::typeError("first arg is not an orel");
  }
  
  ListExpr startNodeList = nl->Second(args);
  ListExpr endNodeList = nl->Third(args);
  ListExpr resultSelect = nl->Fourth(args);
  ListExpr functionWeightMap = nl->Fifth(args);
  ListExpr functionDistMap;
  if(sptype == ASTAR){
    functionDistMap = nl->Sixth(args);
  }

  ListExpr orelTuple = nl->Second(a1);    

  ListExpr orelAttrList(nl->Second(orelTuple));

  if(!nl->HasMinLength(orelAttrList, 3)){
    return listutils::typeError("Ordered relation must have at "
                                "least 3 attributes");
  }


  ListExpr firstAttr = nl->First(orelAttrList);
  if(!CcInt::checkType(nl->Second(firstAttr))){
    return listutils::typeError("first attribute must be of type int");
  }
  ListExpr secondAttr = nl->Second(orelAttrList);
  if(!CcInt::checkType(nl->Second(secondAttr))){
    return listutils::typeError("second attribute must be of type int");
  }

  //Check of second argument
  if (!CcInt::checkType(startNodeList)) {
    return listutils::typeError("Second argument must be int");
  }

  //Check of third argument
  if (!CcInt::checkType(endNodeList)) {
    return listutils::typeError("Third argument should be int");
  }

  //Check of fourth argument
  if(!CcInt::checkType(resultSelect) ) {
    return listutils::typeError("Fourth argument should be int");
  }

  //Check of fifth argument
  if(!listutils::isMap<1>(functionWeightMap)) {
    return listutils::typeError("Fifth argument should be a map");
  }

  ListExpr mapTuple = nl->Second(functionWeightMap);

  if(!nl->Equal(orelTuple,mapTuple)) {
    return listutils::typeError("Tuple of map function must match orel tuple");
  }

  ListExpr mapres = nl->Third(functionWeightMap);

  if(!CcReal::checkType(mapres)) {
    return listutils::typeError(
                "Wrong mapping result type for oshortestpathd");
  }
  
  // check of sixth argument if ASTAR
  if(sptype == ASTAR) {
    if (!listutils::isMap<1>(functionDistMap)) {
      return listutils::typeError(
                      "Sixth argument must be a map");
    }
    ListExpr mapTuple2 = nl->Second(functionDistMap);

    if (!nl->Equal(orelTuple,mapTuple2)) {
      return listutils::typeError(
                       "Tuple of map function must match orel tuple");
    }
    ListExpr mapres2 = nl->Third(functionDistMap);

    if(!CcReal::checkType(mapres2)) {
      return listutils::typeError(
                            "Wrong mapping result type for oshortestpath");
    }
  }

  // appends Attribute SeqNo to Attributes in orel
  NList extendAttrList(nl->TwoElemList(nl->SymbolAtom("SeqNo"),
                                       nl->SymbolAtom(CcInt::BasicType())));
  NList extOrelAttrList(nl->TheEmptyList());


  for(int i = 0; i < nl->ListLength(orelAttrList); i++) {
    NList attr(nl->Nth(i+1,orelAttrList));
    extOrelAttrList.append(attr);
  }

  extOrelAttrList.append(extendAttrList);

  ListExpr outlist = nl->TwoElemList(nl->SymbolAtom(Symbol::STREAM()),
                                     nl->TwoElemList(
                                       nl->SymbolAtom(Tuple::BasicType()),
                                       extOrelAttrList.listExpr()));
  return outlist;
}


class moshortestpathdInfo {
public:
  
  moshortestpathdInfo(ttree::TTree<TupleWrap,TupleComp>* _tree,  
                     int _startNode, int _endNode, int _resultSelect,
                     Word& _arg, TupleType* _tt) 
      : mmorel(_tree),startNode(_startNode),
        endNode(_endNode),resultSelect(_resultSelect),
        arg(_arg),tt(_tt),seqNo(1) {
          
    queue = new Queue();
    visitedNodes = new ttree::TTree<QueueEntryWrap,EntryComp>(16,18);
    sptree = new ttree::TTree<TupleWrap,TupleComp>(16,18);
    result = new ttree::TTree<TupleWrap,TupleComp>(16,18);
    
    bool found = shortestPath();
    if(resultSelect<0 || resultSelect>3){ // bring to allowed values
       resultSelect = 0;
    }
    if(found || resultSelect == 3) {
      resultSelection();
    } else  {
      cerr << "no path found" << endl;
    }
    iterator = result->begin();
  }

  ~moshortestpathdInfo() {
    while(!queue->empty())
      queue->pop();
    delete queue;
    
    tt->DeleteIfAllowed();
    tt = 0;
    
    delete visitedNodes;
    delete result;
    delete sptree;
  };
  
  Tuple* next() {
    if(iterator.end()) 
      return 0;
    Tuple* res = (*iterator).getPointer();
    if(res==NULL) {
      return 0;
    }
    iterator++;
    res->IncReference();
    return res;
  }

  private:
    Word funResult;
  
  
/*
~shortestpath~
 
*/
  bool shortestPath() {
  
  // INIT
    int toNode = startNode;
    
    QueueEntry* startEntry = new QueueEntry(startNode,-1,0.0,0.0);
    QueueEntryWrap qw(startEntry);
    startEntry->DeleteIfAllowed();
    queue->push(qw);
    visitedNodes->insert(qw);
    
    double dist = 0.0;
    
  // SEARCH SHORTESTPATH
    while(!queue->empty()) {
      current = queue->top();
      current()->visited = true;
      queue->pop();
      
      // goal node found
      if (resultSelect<3 && current()->nodeNumber == endNode) {
        return true;
      }
      // process next node
      else {
        CcInt* currentNodeNumber = new CcInt(true,current()->nodeNumber);
        // get tuple with currentNodeNumber as startnode
        ttree::Iterator<TupleWrap,TupleComp> it = mmorel->begin();
        while(!it.end()) {
          Tuple* tup = (*it).getPointer();
          if(tup->GetAttribute(0)->Compare(currentNodeNumber) < 0) {
            it++;
          }
          // tuple found
          else break;
        }
        // process edges
        while(!it.end()) {
          
          Tuple* currentTuple = (*it).getPointer();
          // all edges processed
          if(currentTuple->GetAttribute(0)->Compare(currentNodeNumber) > 0) {
            break;
          }
          if(resultSelect!=3) {
            TupleWrap tw(currentTuple);
            sptree->insert(tw);   
          }
          toNode = ((CcInt*)currentTuple->GetAttribute(1))->GetIntval();
          
          if(current()->nodeNumber != toNode) {
            ArgVectorPointer funArgs = qp->Argument(arg.addr);
            funResult.addr = 0;
            ((*funArgs)[0]).setAddr(currentTuple);
            qp->Request(arg.addr,funResult);
            double edgeCost = ((CcReal*)funResult.addr)->GetRealval();
            if (edgeCost < 0.0) {
              cerr << "Found negativ edge cost computation aborted." << endl;
              return 0;
            }
            dist = current()->dist + edgeCost;
            bool contained = false;
            ttree::Iterator<QueueEntryWrap,EntryComp> it = 
                                            visitedNodes->begin();
            
            // check if shortening of path possible
            while(!it.end()) {
              QueueEntryWrap entry = *it;
              if(entry.getPointer()->nodeNumber == toNode) {
                if(entry.getPointer()->dist > dist) {
                  QueueEntry* prevEntry = new QueueEntry(entry()->nodeNumber,
                                                          entry()->prev,
                                                          entry()->dist,
                                                          entry()->priority);
                  prevEntry->visited = entry()->visited;
                  entry()->dist = dist;
                  entry()->prev = current()->nodeNumber;
                  entry()->priority = dist;
                  if(entry()->visited){
                    QueueEntryWrap  qw(prevEntry);
                    visitedNodes->update(entry,qw);
                  }
                  prevEntry->DeleteIfAllowed();
                }
                contained = true;
                break;
              }
              it++;
            }

            if(!contained) {
              QueueEntry* to = 
                  new QueueEntry(toNode,current()->nodeNumber,dist,dist);
              QueueEntryWrap qw(to);
              to->DeleteIfAllowed();
              visitedNodes->insert(qw,true);
              queue->push(qw);
              if(resultSelect==3){
                TupleWrap tw(currentTuple);
                sptree->insert(tw);
              }
              currentTuple->IncReference();
            }
          }
          it++;
        } 
        currentNodeNumber->DeleteIfAllowed();
      }
    } // end while search shortestpath
    return false;
  }
  
  
  void resultSelection() {
    switch(resultSelect) {
      case 0: {  // shortest path  
      
        Tuple* currentTuple = 
                   findTuple(mmorel,current()->nodeNumber,current()->prev);
        bool found = false;
    
        while (!found && currentTuple != 0) {

          appendTuple(currentTuple,tt,seqNo,result);
          seqNo++;
          
          if(current()->prev != startNode) {
            current = findNextNode(visitedNodes,current);

            currentTuple = 
                    findTuple(mmorel,current()->nodeNumber,current()->prev);
          }
          else {
            found = true;
            if(currentTuple != 0) {
              currentTuple->DeleteIfAllowed();
              currentTuple = 0;
            }
          }
        }
        break;
      }
      case 1: { //Remaining elements in priority queue
        while(queue->size() > 0) {
          current = queue->top();
          queue->pop();
          Tuple* currentTuple = findTuple(mmorel,
                                          current()->nodeNumber,
                                          current()->prev);
          appendTuple(currentTuple,tt,seqNo,result);
          seqNo++;
        }
        break;
      }
      case 2: { //visited sections
        ttree::Iterator<TupleWrap,TupleComp> iter = sptree->begin();
         while(!iter.end()) {
           Tuple* currentTuple = (*iter).getPointer();
           appendTuple(currentTuple,tt,seqNo,result);
           iter++;
           seqNo++;
         }
         break;
      }
      case 3: { //shortest path tree     
        ttree::Iterator<TupleWrap,TupleComp> iter = sptree->begin();
        while(!iter.end()) {
          Tuple* currentTuple = (*iter).getPointer();
          appendTuple(currentTuple,tt,seqNo,result);
          iter++;
          seqNo++;
        }
        break;
      }

      default: { //should have never been reached
        break;
      }
    }
      
  }


protected:

  ttree::TTree<TupleWrap,TupleComp>* mmorel;     
  ttree::Iterator<TupleWrap,TupleComp> iterator;
  int startNode;
  int endNode;
  int resultSelect;
  Word arg; 
  TupleType* tt;
  int seqNo;
  Queue* queue;
  
  ttree::TTree<QueueEntryWrap,EntryComp>* visitedNodes;
  ttree::TTree<TupleWrap,TupleComp>* sptree;
  ttree::TTree<TupleWrap,TupleComp>* result;
  
  
  QueueEntryWrap current;
  int seqNoAttrIndex;
  
};



/*

7.27.2 Value Mapping Function of operator ~moshortestpathd~

*/
template<class T>
int moshortestpathdValueMap(Word* args, Word& result, int message,
                          Word& local, Supplier s) {

  moshortestpathdInfo* li = (moshortestpathdInfo*) local.addr;

  switch(message) {
    
    case OPEN: {
      
      if(li) {
        delete li;
        local.addr=0;
      }

      int resultSelect = ((CcInt*)args[3].addr)->GetIntval();
      if(resultSelect<0 || resultSelect>3) {
        cerr << "Selected result value does not exist. Enter 0 for shortest "
        << "path, 1 for remaining priority queue elements, 2 for visited "
        << "edges, 3 for shortest path tree." << endl;
        return 0;
      }

      // Check for simplest Case
      int startNode = ((CcInt*)args[1].addr)->GetIntval();
      int endNode = ((CcInt*)args[2].addr)->GetIntval();
      if(resultSelect<3) {
        if(startNode == endNode) {
          cerr << "source and target node are equal, no path";
          return 0;
        }
      }

      ListExpr tupleType = GetTupleResultType(s);
      TupleType* tt = new TupleType(nl->Second(tupleType));

      T* orelN = (T*) args[0].addr;
      MemoryORelObject* orel = getMemORel(orelN, nl->Second(qp->GetType(s)));
      if(!orel){
        return 0; 
      }

      local.addr = new moshortestpathdInfo(orel->getmmorel(),
                                          startNode, 
                                          endNode, 
                                          resultSelect, 
                                          args[4], tt);
      return 0;
    }

    case REQUEST: 
      result.addr=li?li->next():0;
      return result.addr?YIELD:CANCEL;
    
    case CLOSE: 
      if(li) {
        delete li;
        local.addr = 0;
      }
      return 0;
  }
  return 0;
}


ValueMapping moshortestpathdVM[] = {
    moshortestpathdValueMap<Mem>,
    moshortestpathdValueMap<MPointer>
};


int moshortestpathSelect(ListExpr args){
  return Mem::checkType(nl->First(args))?0:1;
}


/*
7.27.4 Specification of operator ~moshortestpathd~

*/
OperatorSpec moshortestpathdSpec(
    "MOREL x int x int x int x "
    "(tuple->real) -> stream(tuple(a1:t1,...an+1:tn+1)), "
    "MOREL in {mem, mpointer}",
    "morel moshortestpathd [startNode,endNode,resultSelect; fun] ",
    "Caculates the shortest path from startNode to endNode in a graph "
    "representent in a ordered memory relation using dijkstras algorithm."
    "The ordered relation must have SourceNodes as the first attribute and "
    "target nodes as second attribute. " 
    "Using the argument resultselect, the output of this operator is "
    "controlled. 0 : shortest path, 1 : remaining elements in queue, "
    " 2 : visited sections, 3 : shortest path tree ",
    "query mwrap('otestrel') moshortestpathd"
    "[1,3,0; distance(.GeoData_s1,.GeoData_s2)] count"
);

/*
7.27.4 Instance of operator ~moshortestpathd~

*/
Operator moshortestpathdOp (
    "moshortestpathd",
    moshortestpathdSpec.getStr(),
    2,
    moshortestpathdVM,
    moshortestpathSelect,
    moshortestpathTypeMap<DIJKSTRA>
);


class moshortestpathaInfo {
public:
  
  moshortestpathaInfo(ttree::TTree<TupleWrap,TupleComp>* _tree,  
                     int _startNode, int _endNode, int _resultSelect,
                     Word& _arg, Word& _arg2, TupleType* _tt) 
      : mmorel(_tree),startNode(_startNode),
        endNode(_endNode),resultSelect(_resultSelect),
        arg(_arg),arg2(_arg2),tt(_tt),seqNo(1) {
          
          queue = new Queue();
          visitedNodes = new ttree::TTree<QueueEntryWrap,EntryComp>(16,18);
          sptree = new ttree::TTree<TupleWrap,TupleComp>(16,18);
          result = new ttree::TTree<TupleWrap,TupleComp>(16,18);
          
          bool found = shortestPath();
          if(found || resultSelect == 3) {
            resultSelection();
          }
          else  
            cerr << "no path found" << endl;
          
          iterator = result->begin();
        }

  ~moshortestpathaInfo() {
    while(!queue->empty())
      queue->pop();
    delete queue;
    
    tt->DeleteIfAllowed();
    tt = 0;
    
    delete visitedNodes;
    delete result;
    delete sptree;
  };
  
  Tuple* next() {
    if(iterator.end()) 
      return 0;
    Tuple* res = (*iterator).getPointer();
    if(res==NULL) {
      return 0;
    }
    iterator++;
    res->IncReference();
    return res;
  }
  
  
/*
~shortestpath~
 
*/
  bool shortestPath() {
  
  // INIT
    int toNode = startNode;
    
    QueueEntry* startEntry = new QueueEntry(startNode,-1,0.0,0.0);
    QueueEntryWrap qw(startEntry);
    queue->push(qw);
    visitedNodes->insert(qw);
    startEntry->DeleteIfAllowed();
    
    double dist = 0.0;
    
  // SEARCH SHORTESTPATH
    while(!queue->empty()) {
      current = queue->top();
      current()->visited = true;
      queue->pop();
      
      if (resultSelect<3 && current()->nodeNumber == endNode) {
        return true;
      }
      else {
        CcInt* currentNodeNumber = new CcInt(true,current()->nodeNumber);
        
        ttree::Iterator<TupleWrap,TupleComp> it = mmorel->begin();
        while(!it.end()) {
          Tuple* tup = (*it).getPointer();
          // get tuple with currentNodeNumber as startnode
          if(tup->GetAttribute(0)->Compare(currentNodeNumber) < 0) {
            it++;
          }
          else break;
        }
        
        while(!it.end()) {
          Tuple* currentTuple = (*it).getPointer();            
          if(currentTuple->GetAttribute(0)->Compare(currentNodeNumber) > 0) {
            break;
          }
          toNode = ((CcInt*)currentTuple->GetAttribute(1))->GetIntval();
          if(resultSelect!=3) {
            TupleWrap tw(currentTuple); 
            sptree->insert(tw);      
          }
          if(current()->nodeNumber != toNode) {
            ArgVectorPointer funArgs = qp->Argument(arg.addr);
            funResult.addr =0;
            ((*funArgs)[0]).setAddr(currentTuple);
            qp->Request(arg.addr,funResult);
            double edgeCost = ((CcReal*)funResult.addr)->GetRealval();
            if (edgeCost < 0.0) {
              cerr << "Found negativ edge cost computation aborted." << endl;
              return 0;
            }
            dist = current()->dist + edgeCost;
            
            ArgVectorPointer funArgs2 = qp->Argument(arg2.addr);
            funResult2.addr = 0;
            ((*funArgs2)[0]).setAddr(currentTuple);
            qp->Request(arg2.addr,funResult2);
            double restCost = ((CcReal*)funResult2.addr)->GetRealval();
            if (restCost < 0) restCost = 0;
            double prio = dist + restCost;
            
            bool contained = false;
            ttree::Iterator<QueueEntryWrap,EntryComp> it = 
                                                      visitedNodes->begin();
            while(!it.end()) {
              QueueEntryWrap entry = *it;
              // found node before
              if(entry()->nodeNumber == toNode) {
                if(entry()->priority > prio) {
                  QueueEntry* prevEntry = new QueueEntry(entry()->nodeNumber,
                                                          entry()->prev,
                                                          entry()->dist,
                                                          entry()->priority);
                  prevEntry->visited = entry()->visited;
                  entry()->prev = current()->nodeNumber;
                  entry()->priority = prio;
                  entry()->dist = dist;
                  if(entry()->visited){
                    QueueEntryWrap qw(prevEntry);
                    visitedNodes->update(entry,qw);
                  }
                  prevEntry->DeleteIfAllowed();
                }
                contained = true;
                break;
              }
              it++;
            }

            if(!contained) {
              QueueEntry* to = 
                  new QueueEntry(toNode,current()->nodeNumber,dist,prio);
              QueueEntryWrap qw(to);
              visitedNodes->insert(qw,true);
              to->DeleteIfAllowed();
              queue->push(qw);
              if(resultSelect==3) {
                TupleWrap tw(currentTuple);
                sptree->insert(tw);
              }
              currentTuple->IncReference();
            }
          }
          it++;
        } 
        currentNodeNumber->DeleteIfAllowed();
      }
    } // end while search shortestpath
    return false;
  }
  
  
  void resultSelection() {
    switch(resultSelect) {
      case 0: {  // shortest path  
        Tuple* currentTuple = findTuple(mmorel,
                                        current()->nodeNumber,
                                        current()->prev);
        bool found = false;
    
        int i=0;
        while (!found && currentTuple != 0) {
          
          i++;
          appendTuple(currentTuple,tt,seqNo,result);
          seqNo++;
          
          if(current()->prev != startNode) {
            current = findNextNode(visitedNodes,current);
            currentTuple = findTuple(mmorel,
                                     current()->nodeNumber,
                                     current()->prev);
          }
          else {
            found = true;
            if(currentTuple != 0) {
              currentTuple->DeleteIfAllowed();
              currentTuple = 0;
            }
          }
        }
        break;
      }
      case 1: { //Remaining elements in priority queue
        while(queue->size() > 0) {
          current = queue->top();
          queue->pop();
          Tuple* currentTuple = findTuple(mmorel,
                                          current()->nodeNumber,
                                          current()->prev);
          appendTuple(currentTuple,tt,seqNo,result);
          seqNo++;
        }
        break;
      }
      case 2: { //visited sections
        ttree::Iterator<TupleWrap,TupleComp> it = sptree->begin();
        while(!it.end()) {
          Tuple* currentTuple = (*it).getPointer();;
          appendTuple(currentTuple,tt,seqNo,result);
          seqNo++;
          it++;
        }
        break;
      }
      case 3: { //shortest path tree     
        ttree::Iterator<TupleWrap,TupleComp> iter = sptree->begin();
        while(!iter.end()) {
          Tuple* currentTuple = (*iter).getPointer();
          appendTuple(currentTuple,tt,seqNo,result);
          iter++;
          seqNo++;
        }
        break;
      }

      default: { //should have never been reached
        break;
      }
    }
      
  }


protected:

  ttree::TTree<TupleWrap,TupleComp>* mmorel;     
  ttree::Iterator<TupleWrap,TupleComp> iterator;
  int startNode;
  int endNode;
  int resultSelect;
  Word arg; 
  Word arg2;
  TupleType* tt;
  int seqNo;
  Queue* queue;
  
  ttree::TTree<QueueEntryWrap,EntryComp>* visitedNodes;
  ttree::TTree<TupleWrap,TupleComp>* sptree;
  ttree::TTree<TupleWrap,TupleComp>* result;


  QueueEntryWrap current;
  int seqNoAttrIndex;

private:
  Word funResult;
  Word funResult2;
};



/*

7.28.2  Value Mapping Functions of operator ~moshortestpatha~

*/
template<class T>
int moshortestpathaValueMap(Word* args, Word& result, int message,
                          Word& local, Supplier s) {

  moshortestpathaInfo* li = (moshortestpathaInfo*) local.addr;

  switch(message) {

    case OPEN: {

      if(li) {
        delete li;
        local.addr=0;
      }

      int resultSelect = ((CcInt*) args[3].addr)->GetIntval();
      if(resultSelect<0 || resultSelect>3) {
       cerr << "Selected result value does not exist. Enter 0 for shortest "
       << "path, 1 for remaining priority queue elements, 2 for visited "
       << "edges, 3 for shortest path tree." << endl;
        return 0;
      }

      // Check for simplest Case
      int startNode = ((CcInt*)args[1].addr)->GetIntval();
      int endNode = ((CcInt*)args[2].addr)->GetIntval();
      if(resultSelect<3) {
        if(startNode == endNode) {
          cerr << "source and target node are equal, no path";
          return 0;
        }
      }

      ListExpr tupleType = GetTupleResultType(s);
      TupleType* tt = new TupleType(nl->Second(tupleType));

      T* orelN = (T*) args[0].addr;
      MemoryORelObject* orel = getMemORel(orelN, nl->Second(qp->GetType(s)));
      if(!orel){
        return 0;
      }

      local.addr = new moshortestpathaInfo(orel->getmmorel(),
                                          startNode,
                                          endNode,
                                          resultSelect,
                                          args[4], args[5], tt);
      return 0;
    }

    case REQUEST:
      result.addr=li?li->next():0;
      return result.addr?YIELD:CANCEL;

    case CLOSE:
      if(li) {
        delete li;
        local.addr = 0;
      }
      return 0;
  }
  return 0;
}


ValueMapping moshortestpathaVM[] = {
    moshortestpathaValueMap<Mem>,
    moshortestpathaValueMap<MPointer>,
    
};

/*

7.28.4 Description of operator ~moshortestpatha~

*/
OperatorSpec moshortestpathaSpec(
    "MOREL x int x int x int x "
    "(tuple->real) x (tuple->real)-> stream(tuple(a1:t1,...an+1:tn+1)), "
    "MOREL in {mem, mpointer}",
    "_ moshortestpatha [_,_,_; fun,fun] implicit parameter tuple type MTUPLE",
    "calculates the shortest path for a given start and goal node in a main "
    "memory ordered relation using the astar algorithm",
    "query mwrap('otestrel') moshortestpatha [1,3,0; "
    "distance(.GeoData_s1,.GeoData_s2), "
    "distance(.GeoData_s1,.GeoData_s2) * 0.0] count"
);

/*

7.28.5 Instance of operator ~moshortestpatha~

*/
Operator moshortestpathaOp (
    "moshortestpatha",
    moshortestpathaSpec.getStr(),
    2,
    moshortestpathaVM,
    moshortestpathSelect,
    moshortestpathTypeMap<ASTAR>
);




/*
7.30 Operator ~moconnectedcomponents~

The operator ~moconnectedcomponents~ calculates the strongly connected
components in a graph given here as a main memory ordered relation.
The operator expects a main memory ordered relation with its tuples
containing at least two integers. They have to be ordered first by the start
node of each edge and secondly bey the end node.

The output is a tuple stream containing all tuples of the main memory
ordered relation with an additional attribute 'compNo' of type integer.


7.30.1 Type Mapping Funktion for operator ~moconnectedcomponents~

{string, mem(orel(tuple(x)))} -> stream(tuple(x@[compNo:int]))

*/
ListExpr moconnectedcomponentsTypeMap(ListExpr args) {

    if(nl->ListLength(args) != 1) {
        return listutils::typeError("one arguments expected");
    }

    ListExpr arg;
    if(!getMemSubType(nl->First(args),arg)){
      return listutils::typeError("first arg is not a memory object");
    }
    if(!listutils::isOrelDescription(arg)){
      return listutils::typeError("memory object is not a relation");
    }

    ListExpr rest = nl->Second(nl->Second(arg));
    ListExpr listn = nl->OneElemList(nl->First(rest));
    ListExpr lastlistn = listn;
    rest = nl->Rest(rest);
    while (!(nl->IsEmpty(rest))) {
      lastlistn = nl->Append(lastlistn,nl->First(rest));
      rest = nl->Rest(rest);
    }

    lastlistn = nl->Append(lastlistn,
                          nl->TwoElemList(
                            nl->SymbolAtom("CompNo"),
                            listutils::basicSymbol<CcInt>()));

    ListExpr outList = nl->TwoElemList(nl->SymbolAtom(Symbol::STREAM()),
                                       nl->TwoElemList(
                                         nl->SymbolAtom(Tuple::BasicType()),
                                         listn));
    return outList;
}

class Vertex {
public:
  Vertex(int _nr)
    : nr(_nr), inStack(false) {}
  
  size_t operator()(const Vertex &v) const {
    return v.nr;
  }
  
  void setIndex(int _index) {
    index = _index; 
  }
  
  void setLowlink(int _lowlink) {
    lowlink = _lowlink; 
  }
 
  int nr;
  int index;
  int lowlink;
  int compNo;
  bool inStack;
};

class VertexComp {
public:
  bool operator()(const Vertex& v1, const Vertex& v2) const {
    return v1.nr < v2.nr;
  }
};

// geaendert 9.9.2016
class moconnectedComponentsInfo{
  public:
    moconnectedComponentsInfo(ttree::TTree<TupleWrap,TupleComp>* _tree,
                              ListExpr _tt)
        : tree(_tree) {
      tt = new TupleType(_tt);

      scctree = new ttree::TTree<TupleWrap,TupleComp>(16,18);
      nodes = new set<Vertex,VertexComp>();
      scc(tree);
      appendTuples();
      iter = scctree->begin();
    }


    ~moconnectedComponentsInfo(){
      delete scctree;
      //set<Vertex,VertexComp>::iterator it = nodes->begin();
      delete nodes;
      tt->DeleteIfAllowed();
    }


    Tuple* next() {
      if(iter.end()) 
        return 0;
      Tuple* res = (*iter).getPointer();
      if(!res) {
        return 0;
      }
      iter++;
      res->IncReference();
      return res;
    }

    // appends all tuples including their component number to scctree
    bool appendTuples() {
      set<Vertex,VertexComp>::iterator iter;
      ttree::Iterator<TupleWrap,TupleComp> it = tree->begin();
      while(!it.end()) {
        Tuple* t = (*it).getPointer();
        Vertex v(((CcInt*)t->GetAttribute(1))->GetIntval());
        iter = nodes->find(v);
        if(iter != nodes->end()) {
          v = *iter;
          Tuple* newTuple = new Tuple(tt);
          for(int i=0; i<t->GetNoAttributes(); i++) {
            newTuple->CopyAttribute(i,t,i);
          }
          CcInt* noOfNodes = new CcInt(true, v.compNo);
          newTuple->PutAttribute(t->GetNoAttributes(),noOfNodes);
          TupleWrap tw(newTuple);
          scctree->insert(tw,t->GetNoAttributes());   // sort by CompNo
          newTuple->DeleteIfAllowed();
        }
        else return false; 
        it++;
      }
      return true;
    }


    void scc(ttree::TTree<TupleWrap,TupleComp>* tree) {
      int compNo = 1;
      int index = 0;
      ttree::Iterator<TupleWrap,TupleComp> it = tree->begin();
      std::stack<Vertex>* stack = new std::stack<Vertex>(); 
      // process first node
      Tuple* tuple = (*it).getPointer();
      scc(tuple,index,stack,compNo);  

      // process nodes
      while(!it.end()) {
        // find next node
        Tuple* t = (*it).getPointer();
        while(t->GetAttribute(0)->Compare(tuple->GetAttribute(0)) == 0) {
          it++;
          if(!it.end()) {
            tuple = (*it).getPointer();
          }
          // all nodes processed
          else {
            delete stack;
            return;
          }
        }
        // find scc for node
        scc(tuple,index,stack,compNo);
      }

    }

    // caculates the scc for each node
    void scc(Tuple* tuple, int& index, 
             std::stack<Vertex>* stack, int& compNo) {

      set<Vertex,VertexComp>::iterator iter;
      Vertex v(((CcInt*)tuple->GetAttribute(0))->GetIntval());
      iter = nodes->find(v);
      
      // v already seen
      if(iter != nodes->end()) {
        return;
      }

      // v not yet seen
      v.index = index;
      v.lowlink = index;
      index++;
      stack->push(v);
      v.inStack = true;
      nodes->insert(v);
      ttree::Iterator<TupleWrap,TupleComp> it = tree->begin();
      Tuple* t = (*it).getPointer();
      
      // find node in orel
      while(t->GetAttribute(0)->Compare(tuple->GetAttribute(0)) < 0) {
        it++;
        if(!it.end())
          t = (*it).getPointer(); 
        else return;
      }

      // while node has adjacent nodes
      while(((CcInt*)t->GetAttribute(0))->GetIntval() == v.nr) {
        Vertex w(((CcInt*)t->GetAttribute(1))->GetIntval());
        iter = nodes->find(w);
        // w not seen yet
        if(iter == nodes->end()) { 
          it = tree->begin();
          t = (*it).getPointer();
          while(((CcInt*)t->GetAttribute(0))->GetIntval() != w.nr) {
            it++;
            if(!it.end())
              t = (*it).getPointer();     
            // no adjacent nodes for this node
            else {
              t = tuple;
              w.index = index;
              w.lowlink = index;
              w.compNo = compNo;
              compNo++;
              index++;
              nodes->insert(w);
              break;
            }
          }
          // recursive call
          scc(t,index,stack,compNo);   
          iter = nodes->find(w);
          w = *iter;
          v.setLowlink(min(v.lowlink,w.lowlink));
          // find next adjacent edge for v
          it = tree->begin();
          t = (*it).getPointer();
          while(t->GetAttribute(0)->Compare(tuple->GetAttribute(0)) < 0) {
            it++;
            if(!it.end())
              t = (*it).getPointer(); 
            else return;
          }
        }
        // w already seen
        else {
          w = *iter;
          if(w.inStack) {
            v.setLowlink(min(v.lowlink,w.index));  
          }
        }
        it++;
        if(!it.end())
          t = (*it).getPointer();
        else break;
      }
      
      // root of scc found
      if (v.lowlink == v.index) { 
        v.compNo = compNo;
        while(true) {
          Vertex w = stack->top();
          stack->pop();
          w.inStack = false;
          if(v.nr == w.nr)
            break;
          w.compNo = compNo;
        }
        compNo++;
      }
    }

  private:
     ttree::TTree<TupleWrap,TupleComp>* tree;
     ttree::TTree<TupleWrap,TupleComp>* scctree;
     set<Vertex,VertexComp>* nodes;
     ttree::Iterator<TupleWrap,TupleComp> iter;
     TupleType* tt;
};


/*

7.30.2 Value Mapping Functions of operator ~moconnectedcomponents~

*/
template<class T>
int moconnectedcomponentsValMap (Word* args, Word& result,
                    int message, Word& local, Supplier s) {

  moconnectedComponentsInfo* li = (moconnectedComponentsInfo*) local.addr;

  switch (message) {
    case OPEN: {
      
      if(li){
        delete li;
        local.addr=0;
      }
      T* oN = (T*) args[0].addr;

      ListExpr ttype = nl->Second(nl->Second(qp->GetType(s)));

      MemoryORelObject* orel = getMemORel(oN,ttype);
      if(!orel){
        return listutils::typeError
          ("could not find memory orel object");
      }

      ListExpr tupleType = GetTupleResultType(s);
      ListExpr tt = nl->Second(tupleType);

      local.addr= new moconnectedComponentsInfo(orel->getmmorel(),tt);
      return 0;
    }

    case REQUEST:
      result.addr=li?li->next():0;
      return result.addr?YIELD:CANCEL;

    case CLOSE:
      if(li) {
        delete li;
        local.addr = 0;
      }
      return 0;
  }

  return -1;
}


ValueMapping moconnectedcomponentsVM[] = {
  moconnectedcomponentsValMap<Mem>,
  moconnectedcomponentsValMap<MPointer>

};

int moconnectedcomponentsSelect(ListExpr args){
  return Mem::checkType(nl->First(args))?0:1;;
}


/*

7.30.4 Description of operator ~moconnectedcomponents~

*/
OperatorSpec moconnectedcomponentsSpec(
    "MOREL  -> stream(Tuple)",
    "_ moconnectedcomponents",
    "",
    "query motestrel moconnectedcomponents"
);

/*

7.30.5 Instance of operator ~moconnectedcomponents~

*/
Operator moconnectedcomponentsOp (
    "moconnectedcomponents",
    moconnectedcomponentsSpec.getStr(),
    2,
    moconnectedcomponentsVM,
    moconnectedcomponentsSelect,
    moconnectedcomponentsTypeMap
);


/*
7.31 Operators ~mquicksort~ and ~mquicksortby~

The Operator ~mquicksort~ sorts all tuples of a main memory relation
over all of their attributes, as long as they are atomic. It returns 
the tuples with an additional attribut of type 'tid' in an output stream.

The Operator ~mquicksortby~ sorts all the elements in a main memory relation
by one or more given attributes and returns a stream of the sorted tuples 
and their original tupleid's.

7.31.1 Type Mapping Functions of operator ~mquicksort~ and ~mquicksortby~

{string, mem(rel(tuple(x)))} -> stream(tuple(x)) or 
{string, mem(rel(tuple(x)))} x (ident1 ... identn) -> stream(tuple(x))

*/
enum SORTTYPE{
  Sort,
  SortBy
};

template<SORTTYPE st>
ListExpr mquicksortTypeMap(ListExpr args) {

  if(st == Sort) {
    if(nl->ListLength(args) != 1) 
        return listutils::typeError("one argument expected");
  }
  if(st == SortBy) {
    if(nl->ListLength(args) != 2) 
        return listutils::typeError("two arguments expected");
  }

  // check relation
  ListExpr first;
  if(!getMemSubType(nl->First(args),first)){
    return listutils::typeError("first arg is not a memory object");
  }
  if(!Relation::checkType(first)){
    return listutils::typeError("memory object is not a relation");
  }

    // check if second argument attribute of relation
  if(st == SortBy) {
    int numberOfSortAttrs = nl->ListLength(nl->Second(args));
    if(numberOfSortAttrs <= 0){
      return listutils::typeError("Attribute list may not be empty!");
    }

    if(!listutils::isKeyDescription(nl->Second(first),
                                    nl->Second(args))) {
      return listutils::typeError("all identifiers of second argument must "
                                  "appear in the first argument");
    }
  }

  return nl->TwoElemList(nl->SymbolAtom(Symbol::STREAM()),
                         nl->Second(first));
}


/*

7.31.2  The Value Mapping Functions of operators ~mquicksort~ and 
        ~mquicksortby~
        
*/
class mquicksortInfo{
  public:
    mquicksortInfo(vector<Tuple*>* _relation, vector<int> _pos) 
    : relation(_relation), pos(_pos) {
        
      quicksort(0,relation->size()-1,pos);
      it = relation->begin();
    }
    
    ~mquicksortInfo(){}
    
    void quicksort(int left, int right, vector<int> pos) {
      
      int i = left, j = right;
      Tuple* tmp;
      Tuple* pivot = relation->at(((left + right) / 2)); 
            
      /* partition */
      while (i <= j) {
        
        while(TupleComp::smaller(relation->at(i),pivot,&pos))
          i++;
        while (TupleComp::greater(relation->at(j),pivot,&pos))
          j--;
        
        if (i <= j) {
          tmp = relation->at(i);
          relation->at(i) = relation->at(j);
          relation->at(j) = tmp;
          
          i++;
          j--;
        }
      }      
      /* recursion */
      if (left < j)
        quicksort(left, j, pos);
      if (i < right)
        quicksort(i, right, pos);
    }

    
    Tuple* next() {
      while(it != relation->end()){ 
         Tuple* res = *it;
         it++;
         if(res) {
           res->IncReference();
           return res;
         }
      }
      return 0;
    }

  private:
     vector<Tuple*>* relation;
     vector<int> pos;
     vector<Tuple*>::iterator it;
};

/*

7.31.2 Value Mapping Function of operators ~mquicksort~ and ~mquicksortby~

*/
template<class T, SORTTYPE st>
int mquicksortValMap (Word* args, Word& result,
                    int message, Word& local, Supplier s) {

  mquicksortInfo* li = (mquicksortInfo*) local.addr;

  switch (message) {
    case OPEN: {
      
      if(li){
        delete li;
        local.addr=0;
      }
      vector<int> pos; 
      T* oN = (T*) args[0].addr;
      
      MemoryRelObject* rel = getMemRel(oN, nl->Second(qp->GetType(s)));
      if(!rel){
        return listutils::typeError
          ("could not find memory rel object");
      }
      
      if(st == SortBy) {
        Supplier t = qp->GetSon(s,0);
        ListExpr listrel = nl->Second(qp->GetType(s));
        t = qp->GetSon(s,1);
        ListExpr attrToSort = qp->GetType(t);
        ListExpr attr;
        while (!(nl->IsEmpty(attrToSort))) {
          attr = nl->First(attrToSort);
          attrToSort = nl->Rest(attrToSort);
          
          ListExpr attrType = 0;
          int attrPos = 0;
          ListExpr attrList = nl->Second(listrel);
          
          attrPos = listutils::findAttribute(attrList,
                                             nl->ToString(attr), 
                                             attrType);
          if(attrPos == 0) {
            return listutils::typeError
              ("there is no attribute having name " + nl->ToString(attr));
          }
          pos.push_back(attrPos);
        }
      }
      else if(st == Sort) {
        ListExpr listrel = nl->Second(nl->Second(qp->GetType(s)));
        for(int i=0; i<nl->ListLength(listrel); i++)
          pos.push_back(i+1);
      }
      
      local.addr= new mquicksortInfo(rel->getmmrel(),pos); 
      return 0;
    }

    case REQUEST:
      result.addr=li?li->next():0;
      return result.addr?YIELD:CANCEL;

    case CLOSE:
      if(li) {
        delete li;
        local.addr = 0;
      }
      return 0;
  }

  return -1;
}


ValueMapping mquicksortVM[] = {
  mquicksortValMap<Mem,Sort>,
  mquicksortValMap<MPointer,Sort>,
  mquicksortValMap<Mem,SortBy>,
  mquicksortValMap<MPointer,SortBy>,
};

template<SORTTYPE st>
int mquicksortSelect(ListExpr args){
  int num = Mem::checkType(nl->First(args))?0:1;;
  if(num<0) return -1;
  return st==Sort?num:num+2;
}


/*
7.31.4 Description of operator ~mquicksort~

*/
OperatorSpec mquicksortSpec(
    "MREL -> stream(Tuple), MREL in {mem, mpointer}",
    "_ mquicksort",
    "sorts all elements of a main memory relation over all attributes",
    "query 'Staedte' mquicksort count"
);

/*
7.31.5 Instance of operator ~mquicksort~

*/
Operator mquicksortOp (
    "mquicksort",
    mquicksortSpec.getStr(),
    4,
    mquicksortVM,
    mquicksortSelect<Sort>,
    mquicksortTypeMap<Sort>
);

/*

7.32.4 Description of operator ~mquicksortby~

*/
OperatorSpec mquicksortbySpec(
    "MREL x  ID -> stream(Tuple), MREL on {mem, mpointer}",
    "_ mquicksortby[]",
    "sorts the main memory over the given attributes",
    "query 'Staedte' mquicksortby[Bev,SName] count"
);

/*

7.32.5 Instance of operator ~mquicksortby~

*/
Operator mquicksortbyOp (
    "mquicksortby",
    mquicksortbySpec.getStr(),
    4,
    mquicksortVM,
    mquicksortSelect<SortBy>,
    mquicksortTypeMap<SortBy>
);

/////////////////////////////////////// GRAPH ////


/*

~dijkstra~

Calculates the shortest path between the vertices __start__ and
__dest__ in the __graph__ using dijkstra's algorithm.

*/
bool dijkstra(graph::Graph* graph, Word& arg, 
              graph::Vertex* start, graph::Vertex* dest) {
  
  graph::Queue queue;

  set<graph::Vertex*,graph::Vertex::EqualVertex>::iterator it = 
                                                graph->getGraph()->begin();
  // init all nodes
  while(it != graph->getGraph()->end()) {
    graph::Vertex* v = *it;
    v->setCost(numeric_limits<double>::max());
    v->isSeen(false);
    v->setDist(0);
    v->setPrev(0);
    it++;
  }

  // cost to start node
  start->setCost(0);
  queue.push(start);

  Word funResult;
  
  // as long as queue has entries
  while(!queue.empty()) {
      graph::Vertex* v = queue.top();
      queue.pop();
      if(v->wasSeen()) continue;
      v->isSeen(true);

      // process edges
      for(size_t i=0; i<v->getEdges()->size(); i++) {
        
        vector<graph::EdgeWrap>* edges = v->getEdges();
        
        graph::Vertex* w = graph->getVertex(edges
                                      ->at(i).getPointer()->getDest());
        
        ArgVectorPointer funArgs = qp->Argument(arg.addr);
        funResult.addr = 0;
        ((*funArgs)[0]).setAddr(v->getEdges()->at(i).getPointer()->getTuple());
        qp->Request(arg.addr,funResult);
        double cost = ((CcReal*)funResult.addr)->GetRealval(); 

        if(cost<0) {
            std::cout << "Error: cost is negative" << std::endl;
            return false;
        }

        // shortening of path possible
        if(w->getCost() > v->getCost()+cost) {
            w->setCost(v->getCost()+cost);
            w->setDist(w->getCost());
            w->setPrev(v);
            queue.push(w);
        }
          
      }
      if(v->getNr() == dest->getNr()) {
        return true;
      }
  }
  return false;
}

/*

~astar~

Calculates the shortest path between the vertices __start__ and
__dest__ in the __graph__ using the AStar-algorithm.

*/
bool astar(graph::Graph* graph, Word& arg, Word& arg2,
           graph::Vertex* start, graph::Vertex* dest) {
  
  graph::Queue openlist;

  set<graph::Vertex*,graph::Vertex::EqualVertex>::iterator it = 
                                                graph->getGraph()->begin();
  while(it != graph->getGraph()->end()) {
    graph::Vertex* v = *it;
    v->setCost(numeric_limits<double>::max());
    v->isSeen(false);
    v->setDist(numeric_limits<double>::max());
    v->setPrev(0);
    it++;
  }

  // cost to start node
  start->setCost(0.0);
  openlist.push(start);

  Word funResult;
  Word funResult2;


  // as long as queue has entries
  while(!openlist.empty()) {
    graph::Vertex* v = openlist.top();
    openlist.pop();
    if(v->wasSeen()) continue;
    v->isSeen(true);
    
    if(v->getNr() == dest->getNr()) {
      return true;
    }
    
    // for every adjacent edge of v
    for(size_t i=0; i<v->getEdges()->size(); i++) {
      graph::Vertex* w = graph->getVertex(v->getEdges()
                                 ->at(i).getPointer()->getDest());
      ArgVectorPointer funArgs = qp->Argument(arg.addr);
      funResult.addr = 0;
      ((*funArgs)[0]).setAddr(v->getEdges()->at(i).getPointer()->getTuple());
      qp->Request(arg.addr,funResult);
      double edgecost = ((CcReal*)funResult.addr)->GetRealval();      
      
      if(edgecost<0.0) {
        return false;
      }
      
      ArgVectorPointer funArgs2 = qp->Argument(arg2.addr);
      funResult2.addr = 0;
      ((*funArgs2)[0]).setAddr(v->getEdges()->at(i).getPointer()->getTuple());
      qp->Request(arg2.addr,funResult2);
      double prio = ((CcReal*)funResult2.addr)->GetRealval();    

      if(prio<0.0) {
        prio = 0.0;
      }

      // shortening of path possible
      if(w->getDist() > v->getCost()+edgecost+prio) {
          w->setCost(v->getCost()+edgecost);
          w->setDist(prio+w->getCost());
          
          if(!w->wasSeen())
            w->setPrev(v);
          openlist.push(w);
      }
     }
      
  }
  return false;
}

bool shortestPath(graph::Graph* graph, int startNode, 
                  int endNode, Word& arg, Word& arg2, 
                  bool isAstar) {
  bool found = false;
  if(!isAstar) {
    found = dijkstra(graph, arg,
                     graph->getVertex(startNode), 
                     graph->getVertex(endNode));
  }
  if(isAstar) {
    found = astar(graph,arg,arg2,
                  graph->getVertex(startNode), 
                  graph->getVertex(endNode));
  }
  if(found) {
    graph->getPath(graph->getVertex(endNode));
  }
  return found;        
}

/*
7.33 Operator ~mgshortestdpathd~

The Operator ~mgshortestpathd~ and ~mgshortestpatha~ calculate the shortest
path between two given nodes of a graph. The graph is in this case represented
by a main memory graph. The tuples of such a graph need to 
have two integer values as their first two attributes identifying the start and
end node of an edge and at least one double value containing the cost
the edge.

The operator expects an main memory graph, two integers describing 
the start and goal node of the path, an integer choosing the resulting output
of the shortest path calculation, as well as an cost function, which maps 
the values of the tuples to a double value. The Operator ~moshortestpatha~ 
expects an additional function calculating the distance to the goal node.

The following results can be selected by integer:
0 shortest path
1 remaining priority queue at end of computation
2 visited sections of shortest path search
3 shortest path tree

For all cases an additional attribute 'seqNo' of type integer will be appended
to the tuples of the output stream.

The Operator ~mgshortestpathd~ uses Dijkstras Algorithm and the Operator 
~mgshortestpatha~ uses the AStar-Algorithm.


7.33.1 General Type Mapping for Operators ~mgshortestpathd~ and 
       ~mgshortestpatha~
   
{string, mem(rel(tuple(X)))} x int x int x int -> 
stream(tuple(a1:t1,...an+1:tn+1))  and

{string, mem(graph(tuple(X)))} x int x int x int x (tuple->real) 
x (tuple->real) -> 
stream(tuple(a1:t1,...an+1:tn+1))
   
*/
template<bool isAstar>
ListExpr mgshortestpathdTypeMap(ListExpr args) {

  if(!isAstar && nl->ListLength(args) != 5) {
      return listutils::typeError("five  arguments expected");
  }
  
  else if(isAstar && nl->ListLength(args) != 6) {
      return listutils::typeError("six arguments expected");
  }

  ListExpr graph;
  if(!getMemSubType(nl->First(args), graph)){
    return listutils::typeError("first argument is not a memory object");
  }
  if(!MemoryGraphObject::checkType(graph)) {
    return listutils::typeError("first arg is not a mem graph");
  }
  
  ListExpr startNodeList = nl->Second(args);
  ListExpr endNodeList = nl->Third(args);
  ListExpr resultSelect = nl->Fourth(args);
  ListExpr map = nl->Fifth(args);
  ListExpr distMap=nl->TheEmptyList();
  if(isAstar)
    distMap = nl->Sixth(args);

  graph = nl->Second(graph);

  if(!listutils::isTupleDescription(graph)) {
    return listutils::typeError(
                       "second value of graph is not of type tuple");
  }
  
  ListExpr relAttrList(nl->Second(graph));

  if(!listutils::isAttrList(relAttrList)) {
    return listutils::typeError("Error in rel attrlist.");
  }

  if(!(nl->ListLength(relAttrList) >= 3)) {
    return listutils::typeError("rel has less than 3 attributes.");
  }

  //Check of second argument
  if (!listutils::isSymbol(startNodeList,CcInt::BasicType())) {
    return listutils::typeError("Second argument should be int");
  }

  //Check of third argument
  if (!listutils::isSymbol(endNodeList,CcInt::BasicType())) {
    return listutils::typeError("Third argument should be int");
  }

  //Check of fourth argument
  if(!listutils::isSymbol(resultSelect,CcInt::BasicType())) {
    return listutils::typeError("Fourth argument should be int");
  }
  
  //Check of fifth argument
  if(!listutils::isMap<1>(map)) {
    return listutils::typeError("Fifth argument should be a map");
  }

  ListExpr mapTuple = nl->Second(map);

  if(!nl->Equal(graph,mapTuple)) {
    return listutils::typeError(
                       "Tuple of map function must match graph tuple");
  }

  ListExpr mapres = nl->Third(map);

  if(!listutils::isSymbol(mapres,CcReal::BasicType())) {
    return listutils::typeError(
                 "Wrong mapping result type for mgshortestpatha");
  }
  
  //Check of sixth argument if AStar
  if(isAstar) {
    if(!listutils::isMap<1>(distMap)) {
      return listutils::typeError(
                      "Sixth argument should be a map");
    }

    ListExpr distmapTuple = nl->Second(distMap);

    if(!nl->Equal(graph,distmapTuple)) {
      return listutils::typeError(
             "Tuple of map function must match graph tuple");
    }

    ListExpr distmapres = nl->Third(distMap);

    if(!listutils::isSymbol(distmapres,CcReal::BasicType())) {
      return listutils::typeError(
             "Wrong mapping result type for mgshortestpatha");
    }
  }

  // appends Attribute SeqNo to Attributes in orel
  ListExpr rest = nl->Second(graph);
  ListExpr listn = nl->OneElemList(nl->First(rest));
  ListExpr lastlistn = listn;
  rest = nl->Rest(rest);
  while (!(nl->IsEmpty(rest))) {
    lastlistn = nl->Append(lastlistn,nl->First(rest));
    rest = nl->Rest(rest);
  }
  lastlistn = nl->Append(lastlistn,
                        nl->TwoElemList(
                          nl->SymbolAtom("SeqNo"),
                          nl->SymbolAtom(TupleIdentifier::BasicType())));
  
  return nl->TwoElemList(nl->SymbolAtom(Symbol::STREAM()),
                                      nl->TwoElemList(
                                        nl->SymbolAtom(Tuple::BasicType()),
                                        listn));
}


class mgshortestpathdInfo {
public:
  
  mgshortestpathdInfo(graph::Graph* _graph, 
                      int _startNode, int _endNode, 
                      int _resultSelect, Word& _arg,
                      TupleType* _tt) 
      : graph(_graph),startNode(_startNode),
        endNode(_endNode), resultSelect(_resultSelect),
        arg(_arg), tt(_tt),seqNo(1) {
          
          result = new ttree::TTree<TupleWrap,TupleComp>(16,18); 
          bool found = shortestPath(graph,startNode,endNode,arg,arg,false);
          
          if(found || resultSelect == 3) {
            resultSelection();
          }
          else { 
            cerr << "no path found" << endl;
          }
          iterator = result->begin();

        }

  ~mgshortestpathdInfo() {
    graph->reset();
    delete result;
    tt->DeleteIfAllowed();
  }
  
  Tuple* next() {
    if(iterator.end()) 
      return 0;
    Tuple* res = (*iterator).getPointer();
    if(res==NULL) {
      return 0;
    }
    iterator++;
    res->IncReference();
    return res;
  }
  
 
  
/*
~resultSelection~
 
*/  
  void resultSelection() { 
  
    switch(resultSelect) {
      case 0: {  // shortest path  
      
        seqNo = 1;
        int source = startNode;
        int dest = 0;
        
        for(size_t i=1; i<graph->getResult()->size(); i++) {
          dest = graph->getResult()->at(i);
          Tuple* tup = graph->getEdge(source,dest);          
          if(!tup)
            break;

          appendTuple(tup,tt,seqNo,result);
          seqNo++;
          source = dest;
        }
        graph->reset();
        break;
      }
      case 1: { //Remaining elements in priority queue
        
        break;
      }
      
      case 2: { //visited sections
      
        break;
      }
  
      case 3: { //shortest path tree     
      
        break;
      }

      default: { //should have never been reached
        break;
      }
    }
      
  }


protected:

  graph::Graph* graph;
  int startNode;
  int endNode;
  int resultSelect;
  Word arg;
  TupleType* tt;  
  int seqNoAttrIndex;
  int seqNo;
  ttree::TTree<TupleWrap,TupleComp>* result;
  ttree::Iterator<TupleWrap,TupleComp> iterator;  
};


/*

7.33.2 Value Mapping Function of operator ~mgshortestpathd~ 

*/
template<class T>
int mgshortestpathdValMap (Word* args, Word& result,
                    int message, Word& local, Supplier s) {

  mgshortestpathdInfo* li = (mgshortestpathdInfo*) local.addr;

  switch (message) {
    case OPEN: {
      
      if(li){
        delete li;
        local.addr=0;
      }
      
      int resultSelect = ((CcInt*)args[3].addr)->GetIntval();
      if(resultSelect<0 || resultSelect>3) {
        cerr << "Selected result value does not exist. Enter 0 for shortest "
        << "path, 1 for remaining priority queue elements, 2 for visited "
        << "edges, 3 for shortest path tree." << endl;
        return 0;
      }

      // Check for simplest Case
      int startNode = ((CcInt*)args[1].addr)->GetIntval();
      int endNode = ((CcInt*)args[2].addr)->GetIntval();
      if(resultSelect<3) {
        if(startNode == endNode) {
          cerr << "source and target node are equal, no path";
          return 0;
        }
      }
      
      T* oN = (T*) args[0].addr;
      
      MemoryGraphObject* memgraph = getMemGraph(oN);
      if(!memgraph) {
        return 0;
      }

      ListExpr tupleType = GetTupleResultType(s);
      TupleType* tt = new TupleType(nl->Second(tupleType));

      local.addr = new mgshortestpathdInfo(memgraph->getgraph(),
                                           startNode, 
                                           endNode, 
                                           resultSelect,
                                           args[4],tt);
      return 0;
    }

    case REQUEST:
      result.addr=li?li->next():0;
      return result.addr?YIELD:CANCEL;

    case CLOSE:
      if(li) {
        delete li;
        local.addr = 0;
      }
      return 0;
  }

  return -1;
}


ValueMapping mgshortestpathdVM[] = {
  mgshortestpathdValMap<Mem>,
  mgshortestpathdValMap<MPointer>,

};

int mgshortestpathdSelect(ListExpr args){
  return Mem::checkType(nl->First(args))?0:1;;
}


/*

7.33.4 Description of operator ~mgshortestpathd~

*/
OperatorSpec mgshortestpathdSpec(
    "MGRAPH x int x int x int -> "
    "stream(tuple(a1:t1,...an+1:tn+1))",
    "_ mgshortestpathd [_,_,_; fun] implicit parameter tuple type MTUPLE",
    "finds the shortest path between to given nodes in a main memory "
    "graph using dijkstras algorithm",
    "query graph mgshortestpathd "
    "[1,3,0; distance(.GeoData_s1,.GeoData_s2)] count"
);


/*

7.33.5 Instance of operator ~mgshortestpathd~

*/
Operator mgshortestpathdOp (
    "mgshortestpathd",
    mgshortestpathdSpec.getStr(),
    2,
    mgshortestpathdVM,
    mgshortestpathdSelect,
    mgshortestpathdTypeMap<false>
);



class mgshortestpathaInfo {
public:
  
  mgshortestpathaInfo(graph::Graph* _graph, 
                     int _startNode, int _endNode, int _resultSelect,
                     Word& _arg, Word& _arg2, TupleType* _tt) 
      : graph(_graph),startNode(_startNode),
        endNode(_endNode),resultSelect(_resultSelect),
        arg(_arg),arg2(_arg2), tt(_tt),seqNo(1) {

          result = new ttree::TTree<TupleWrap,TupleComp>(16,18); 
        
          bool found = shortestPath(graph,startNode,endNode,arg,arg2,true);
          if(found || resultSelect == 3) {
            resultSelection();
          }
          else 
            cerr << "no path found" << endl;
          
          iterator = result->begin();

        }
        
  ~mgshortestpathaInfo() {
    graph->reset();
    delete result;
    result = 0;
    tt->DeleteIfAllowed();
  }
  
  Tuple* next() {
    if(iterator.end()) 
      return 0;
    Tuple* res = (*iterator).getPointer();
    if(res==NULL) {
      return 0;
    }
    iterator++;
    res->IncReference();
    return res;
  }
  


/*
~resultSelection~
 
*/  
  void resultSelection() { 
  
    switch(resultSelect) {
      case 0: {  // shortest path  
      
        seqNo = 1;
        int source = startNode;
        int dest = 0;
        
        for(size_t i=1; i<graph->getResult()->size(); i++) {
          dest = graph->getResult()->at(i);
          Tuple* tup = graph->getEdge(source,dest);          
          if(!tup)
            break;
          appendTuple(tup,tt,seqNo,result);
          seqNo++;
          source = dest;
        }
        graph->reset();
        break;
      }
      case 1: { //Remaining elements in priority queue
        
        break;
      }
      
      case 2: { //visited sections
      
        break;
      }
  
      case 3: { //shortest path tree     
      
        break;
      }

      default: { //should have never been reached
        break;
      }
    }
      
  }


protected:

  graph::Graph* graph;
  int startNode;
  int endNode;
  int resultSelect;
  Word arg;
  Word arg2;
  TupleType* tt;  
  int seqNoAttrIndex;
  int seqNo;
  ttree::TTree<TupleWrap,TupleComp>* result;
  ttree::Iterator<TupleWrap,TupleComp> iterator;
};


/*

7.34.2 Value Mapping Function of operator ~mgshortestpatha~

*/
template<class T>
int mgshortestpathaValMap (Word* args, Word& result,
                    int message, Word& local, Supplier s) {

  mgshortestpathaInfo* li = (mgshortestpathaInfo*) local.addr;

  switch (message) {
    case OPEN: {
      
      if(li){
        delete li;
        local.addr=0;
      }
      
      int resultSelect = ((CcInt*)args[3].addr)->GetIntval();
      if(resultSelect<0 || resultSelect>3) {
        cerr << "Selected result value does not exist. Enter 0 for shortest "
        << "path, 1 for remaining priority queue elements, 2 for visited "
        << "edges, 3 for shortest path tree." << endl;
        return 0;
      }

      // Check for simplest Case
      int startNode = ((CcInt*)args[1].addr)->GetIntval();
      int endNode = ((CcInt*)args[2].addr)->GetIntval();
      if(resultSelect<3) {
        if(startNode == endNode) {
          cerr << "source and target node are equal, no path";
          return 0;
        }
      }
      
      T* oN = (T*) args[0].addr;
      
      MemoryGraphObject* memgraph = getMemGraph(oN);
      if(!memgraph) {
        return 0;
      }
      
      ListExpr tupleType = GetTupleResultType(s);
      TupleType* tt = new TupleType(nl->Second(tupleType));

      local.addr = new mgshortestpathaInfo(memgraph->getgraph(),
                                           startNode, 
                                           endNode, 
                                           resultSelect,
                                           args[4],args[5],tt);
      return 0;
    }

    case REQUEST:
      result.addr=li?li->next():0;
      return result.addr?YIELD:CANCEL;

    case CLOSE:
      if(li) {
        delete li;
        local.addr = 0;
      }
      return 0;
  }

  return -1;
}


ValueMapping mgshortestpathaVM[] = {
  mgshortestpathaValMap<Mem>,
  mgshortestpathaValMap<MPointer>,
};

int mgshortestpathaSelect(ListExpr args){
  return Mem::checkType(nl->First(args))?0:1;
}


/*

7.34.4 Description of operator ~mgshortestpatha~

*/
OperatorSpec mgshortestpathaSpec(
    "MGRAPH x int x int x int x (tuple->real) x "
    "(tuple->real) -> stream(tuple(a1:t1,...an+1:tn+1))",
    "_ mgshortestpatha [_,_,_; fun, fun] implicit parameter tuple type MTUPLE",
    "finds the shortest path between to given nodes in a main memory "
    "graph using the a*-algorithm",
    "query mwrap('graph') mgshortestpatha "
    "[1,40,0; distance(.SourcePos,.TargetPos), "
    "distance(.SourcePos,.TargetPos) * 2.0] count"
);


/*

7.34.5 Instance of operator ~mgshortestpatha~

*/
Operator mgshortestpathaOp (
    "mgshortestpatha",
    mgshortestpathaSpec.getStr(),
    2,
    mgshortestpathaVM,
    mgshortestpathaSelect,
    mgshortestpathdTypeMap<true>
);





/*
7.35 Operator ~mcreatemgraph~ and ~mcreatemgraphflob~

The operator ~mcreatemgraph~ creates a main memory graph using adjacency list
from an ordered relation which represents a graph as well.
of the main memory graph is successful the operator returns true.
In addition the operator ~mcreatemgraphflob~ loads the flobs of the attributes
into the main memory.

7.35.1 Type Mapping Functions of operator ~mcreatemgraph~ and
        ~mcreatemgraphflob~

orel(tuple(x)) ->  mpointer(mem(graph(orel)))

*/
ListExpr mcreatemgraphTypeMap(ListExpr args) {
  
 if(nl->ListLength(args)!=1){
    return listutils::typeError("wrong number of arguments");
 }
 ListExpr arg = nl->First(args);
 if(!OrderedRelation::checkType(arg)){
   return listutils::typeError("argument is not an ordered relation");
 }
    
 // check if tuples contain at least to integer values
 ListExpr orelTuple = nl->Second(arg);    
 ListExpr attrlist(nl->Second(orelTuple));
    
 ListExpr rest = attrlist;
 bool foundSource = false;
 bool foundTarget = false;

 if(!nl->HasMinLength(attrlist,2)){
   return listutils::typeError("orel has less than "
                               "two integer attributes.");

 }
    
 while(!nl->IsEmpty(rest)) {
    ListExpr listn = nl->OneElemList(nl->Second(nl->First(rest)));  
    if(listutils::isSymbol(nl->First(listn),CcInt::BasicType())) {    
      if(!foundSource) 
         foundSource = true;
      else {
         foundTarget = true;
         break;
      }
    }
    rest = nl->Rest(rest);
  }

  if(!foundSource || !foundTarget) {
      return listutils::typeError("orel has less than 2 int attributes.");
  }

  return MPointer::wrapType(Mem::wrapType(
                     MemoryGraphObject::wrapType(orelTuple)));
    
}


/*

7.35.2  The Value Mapping Functions of operator ~mcreatemgraph~

*/
template<bool flob>
int mcreatemgraphValMap(Word* args, Word& result,
                 int message, Word& local, Supplier s) {
    result  = qp->ResultStorage(s);
    MPointer* mp = static_cast<MPointer*>(result.addr);
    bool mcreatemgraphsucceed = false;
    Supplier t = qp->GetSon( s, 0 );
    ListExpr le = qp->GetType(t);
    GenericRelation* rel = static_cast<GenericRelation*>(args[0].addr);
    MemoryGraphObject* memgraph = new MemoryGraphObject();
    mcreatemgraphsucceed = memgraph->relToGraph(rel,le,getDBname(),flob);
    if(mcreatemgraphsucceed) {
       mp->setPointer(memgraph);
       memgraph->deleteIfAllowed();
    } else {
      memgraph->deleteIfAllowed(); // it managed by the catalog
      mp->setPointer(0);
    }
    return 0;
}


/*

7.35.4 Description of operator ~mcreatemgraph~

*/
OperatorSpec createmgrepahSpec(
    "orel(tuple(x)) -> bool",
    "mcreatemgraph(_)",
    "creates a main memory graph object from an ordered relation",
    "query mcreatemgraph (otestrel)"
);


/*

7.35.5 Instance of operator ~mcreatemgraph~

*/
Operator mcreatemgraphOp (
    "mcreatemgraph",
    createmgrepahSpec.getStr(),
    mcreatemgraphValMap<false>,
    Operator::SimpleSelect,
    mcreatemgraphTypeMap
);

/*

7.36.4 Description of operator ~mcreatemgraphflob~

*/
OperatorSpec mcreatemgraphflobSpec(
    "orel(tuple(x)) -> mpointer(mem(graph(tuple(x))))",
    "mcreatemgraphflob (_)",
    "creates a main memory graph object from a given ordered relation "
    "and loads the flobs into the main memory",
    "query mcreatemgraphflob (otestrel)"
);


/*

7.36.5 Instance of operator ~mcreatemgraphflob~

*/
Operator mcreatemgraphflobOp (
    "mcreatemgraphflob",
    mcreatemgraphflobSpec.getStr(),
    mcreatemgraphValMap<true>,
    Operator::SimpleSelect,
    mcreatemgraphTypeMap
);


/*

7.37 Operator ~mgconnectedcomponents~

The operator ~mgconnectedcomponents~ computes the strongly connected 
components in a given main meory graph. the tuples of the graph
with the appended component number are returned in an output stream.

7.37.1 Type Mapping Functions of operator ~mgconnectedcomponents~

{string, mem(graph(tuple(x)))} -> stream(tuple(a1:t1,...an+1:tn+1))
    
*/
template<bool perNode>
ListExpr mgconnectedcomponentsTypeMap(ListExpr args) {
  
    if(nl->ListLength(args) != 1) {
        return listutils::typeError("one argument expected");
    }
    ListExpr graph;
    if(!getMemSubType(nl->First(args), graph)){
      return listutils::typeError("argument is not a memory object");
    }

    if(!MemoryGraphObject::checkType(graph))
      return listutils::typeError("first arg is not a mem graph");
    
    // check if second argument attribute of relation
    graph = nl->Second(graph);
    ListExpr relAttrList(nl->Second(graph));

    if(!listutils::isAttrList(relAttrList)) {
      return listutils::typeError("Error in rel attrlist.");
    }
    
    // appends Attribute CompNo to Attributes in orel
    ListExpr rest = nl->Second(graph);
    ListExpr listn = nl->OneElemList(nl->First(rest));
    ListExpr lastlistn = listn;
    rest = nl->Rest(rest);
    while (!(nl->IsEmpty(rest))) {
      lastlistn = nl->Append(lastlistn,nl->First(rest));
      rest = nl->Rest(rest);
    }
    string n1 = perNode?"SourceComp":"CompNo";
    lastlistn = nl->Append(lastlistn,
                          nl->TwoElemList(
                            nl->SymbolAtom(n1),
                            listutils::basicSymbol<CcInt>()));
    if(perNode){
       lastlistn = nl->Append(lastlistn,
                          nl->TwoElemList(
                            nl->SymbolAtom("TargetComp"),
                            listutils::basicSymbol<CcInt>()));
    }

    
    return nl->TwoElemList(nl->SymbolAtom(Symbol::STREAM()),
                                        nl->TwoElemList(
                                          nl->SymbolAtom(Tuple::BasicType()),
                                          listn));
}

template<bool iter, bool perNode>
class mgconnectedComponentsInfo{
  public:
    mgconnectedComponentsInfo(graph::Graph* _graph,
                              TupleType* _tt)
        : graph(_graph),tt(_tt) {
      
      // compute the strongly connected components
      graph->resetComp();

      if(iter){
         graph->tarjan2();
      } else {
         graph->tarjan();
      }
      graphit = graph->getGraph()->begin();
      nextVertex();
    }
    
    ~mgconnectedComponentsInfo(){
      tt->DeleteIfAllowed();
    }
    
    
    Tuple* next() {
      while(graphit != graph->getGraph()->end()) {
         if(vertexit!=vertex->getEdges()->end()){
            Tuple* result = createResultTuple(
                                  vertexit->getPointer()->getTuple());
            vertexit++;
            return result; 
         }
         graphit++;
         nextVertex();
      }      
      return 0;
   }

    
  private:
    graph::Graph* graph;
    TupleType* tt;
    set<graph::Vertex*,graph::Vertex::EqualVertex>::iterator graphit;
    vector<graph::EdgeWrap>::iterator vertexit;
    int compNo;
    graph::Vertex* vertex;

    void nextVertex(){
      if(graphit!= graph->getGraph()->end()){
         vertex = *graphit;
         compNo = vertex->getCompNo();
         vertexit = vertex->getEdges()->begin();
      } else {
         vertex = 0;
         compNo = -1;
      }
    }

    Tuple* createResultTuple(Tuple* tup){
      // copy original attributes
      Tuple* res = new Tuple(tt);
      for(int i=0; i<tup->GetNoAttributes(); i++) {
        res->CopyAttribute(i,tup,i);
      }     
      // append new Attributes
      graph::Vertex* w = graph->getVertex(vertexit->getPointer()->getDest());
      if(!perNode){
        int comp = w->getCompNo()==compNo?compNo:-2;
        res->PutAttribute(tup->GetNoAttributes(),new CcInt(true,comp));
        return res;  
      }
      res->PutAttribute(tup->GetNoAttributes(),new CcInt(true,compNo));
      res->PutAttribute(tup->GetNoAttributes()+1,
                        new CcInt(true,w->getCompNo()));
      return res;
   }
};

/*

7.37.2 Value Mapping Function of operator ~mgconnectedcomponents~

*/
template<class T,bool iter, bool perNode>
int mgconnectedcomponentsValMap (Word* args, Word& result,
                    int message, Word& local, Supplier s) {

  mgconnectedComponentsInfo<iter, perNode>* li =
        (mgconnectedComponentsInfo<iter, perNode>*) local.addr;

  switch (message) {
    case OPEN: {
      
      if(li){
        delete li;
        local.addr=0;
      }
      T* oN = (T*) args[0].addr;
      

      MemoryGraphObject* memgraph = getMemGraph(oN);
      if(!memgraph) {
        return 0;
      }
      
      ListExpr tupleType = GetTupleResultType(s);
      TupleType* tt = new TupleType(nl->Second(tupleType));
      
      local.addr= new mgconnectedComponentsInfo<iter,perNode>(
                                                 memgraph->getgraph(),tt); 
      return 0;
    }

    case REQUEST:
      result.addr=li?li->next():0;
      return result.addr?YIELD:CANCEL;

    case CLOSE:
      if(li) {
        delete li;
        local.addr = 0;
      }
      return 0;
  }

  return -1;
}


ValueMapping mgconnectedcomponentsVM[] = {
  mgconnectedcomponentsValMap<Mem,false, false>,
  mgconnectedcomponentsValMap<MPointer,false, false>,
};


ValueMapping mgconnectedcomponentsVM2[] = {
  mgconnectedcomponentsValMap<Mem,true, false>,
  mgconnectedcomponentsValMap<MPointer,true, false>,
};

ValueMapping mgconnectedcomponentsVM3[] = {
  mgconnectedcomponentsValMap<Mem,true, true>,
  mgconnectedcomponentsValMap<MPointer,true, true>
};


int mgconnectedcomponentsSelect(ListExpr args){
  return Mem::checkType(nl->First(args))?0:1;
}


/*

7.37.4 Description of operator ~mgconnectedcomponents~

*/
OperatorSpec mgconnectedcomponentsSpec(
    "MGRAPH -> stream(Tuple)",
    "_ mgconnectedcomponents",
    "computes the scc's for a given main memory graph",
    "query graph mgconnectedcomponents"
);

/*

7.37.5 Instance of operator ~mgconnectedcomponents~

*/
Operator mgconnectedcomponents_oldOp (
    "mgconnectedcomponents_old",
    mgconnectedcomponentsSpec.getStr(),
    2,
    mgconnectedcomponentsVM,
    mgconnectedcomponentsSelect,
    mgconnectedcomponentsTypeMap<false>
);

Operator mgconnectedcomponentsOp (
    "mgconnectedcomponents",
    mgconnectedcomponentsSpec.getStr(),
    2,
    mgconnectedcomponentsVM2,
    mgconnectedcomponentsSelect,
    mgconnectedcomponentsTypeMap<false>
);


Operator mgconnectedcomponentsNOp (
    "mgconnectedcomponentsN",
    mgconnectedcomponentsSpec.getStr(),
    2,
    mgconnectedcomponentsVM3,
    mgconnectedcomponentsSelect,
    mgconnectedcomponentsTypeMap<true>
);

/*
7.38 ~momapmatchmht~

7.38.1 Operator-Info
    map matching with a main memory ordered relation
    Result: Tuples of matched edges with timestamps

*/
// struct MOMapMatchMHTInfo : OperatorInfo {
// MOMapMatchMHTInfo() {
//   name      = "momapmatchmht";
//   signature = MemoryORelObject::BasicType() + " x " +
//               rtreehelper::BasicType() + " x " +
//               MemoryRelObject::BasicType() + " x " +
//               temporalalgebra::MPoint::BasicType() + " -> " +
//               "stream(tuple([<attributes of tuple of edge>,"
//                             "StartTime:DateTime, EndTime:DateTime]))";
// 
//   appendSignature(MemoryORelObject::BasicType() + " x " +
//                   rtreehelper::BasicType() + " x " +
//                   MemoryRelObject::BasicType() + " x " +
//                   FText::BasicType()  + " -> " +
//                   "stream(tuple([<attributes of tuple of edge>,"
//                                 "StartTime:DateTime, EndTime:DateTime]))");
// 
//   appendSignature(MemoryORelObject::BasicType() + " x " +
//                   rtreehelper::BasicType() + " x " +
//                   MemoryRelObject::BasicType() + " x " +
//                   "(stream (tuple([Lat:real, Lon:real, Time:DateTime "
//                                   "[,Fix:int] [,Sat:int] [,Hdop : real]"
//                                   "[,Vdop:real] [,Pdop:real] "
//                                   "[,Course:real] [,Speed(m/s):real]])))"
//                   + " -> " +
//                   "stream(tuple([<attributes of tuple of edge>,"
//                                 "StartTime:DateTime, EndTime:DateTime]))");
// 
// 
//   syntax    = "momapmatchmht ( _ , _ , _ , _ )";
//   meaning   = "The operation maps the MPoint or "
//               "the data of a gpx-file or "
//               "the data of a tuple stream "
//               "to the given network, which is based on a "
//               "main memory ordered relation."
//               "Result is a stream of tuples with the matched edges "
//               "of the network with timestamps.";
//   example   = "momapmatchmht('Edges', 'EdgeIndex_Box_rtree', "
//                              "'EdgeIndex', 'Trk_Dortmund.gpx')";
// }
// };


// static ListExpr GetORelNetworkAttrIndexes(ListExpr ORelAttrList);


// static ListExpr GetMMDataIndexesOfTupleStream(ListExpr TupleStream);


// enum MOMapMatchingMHTResultType {
//     OMM_RESULT_STREAM,
//     OMM_RESULT_GPX,
//     OMM_RESULT_MPOINT
// };


// static ListExpr AppendLists(ListExpr List1, ListExpr List2) {
//     
//   ListExpr ResultList = nl->TheEmptyList();
//   ListExpr last = nl->TheEmptyList();
// 
//   ListExpr Lauf = List1;
// 
//   while (!nl->IsEmpty(Lauf)) {
//     ListExpr attr = nl->First(Lauf);
// 
//     if (nl->IsEmpty(ResultList)) {
//       ResultList = nl->OneElemList(attr);
//       last = ResultList;
//     }
//     else
//       last = nl->Append(last, attr);
//     
//     Lauf = nl->Rest(Lauf);
//   }
// 
//   Lauf = List2;
// 
//   while (!nl->IsEmpty(Lauf)) {
//     ListExpr attr = nl->First(Lauf);
// 
//     if (nl->IsEmpty(ResultList)) {
//         ResultList = nl->OneElemList(attr);
//         last = ResultList;
//     }
//     else 
//       last = nl->Append(last, attr);
// 
//     Lauf = nl->Rest(Lauf);
//   }
// 
//   return ResultList;
// }


// template<class T>
// static typename mapmatching::MmORelNetwork<T>::OEdgeAttrIndexes 
//                         GetOEdgeAttrIndexes(Word* args, int nOffset) {
//     // s. GetORelNetworkAttrIndexes
// 
//     typename mapmatching::MmORelNetwork<T>::OEdgeAttrIndexes Indexes;
// 
//    const CcInt* pIdxSource    = static_cast<CcInt*>(args[nOffset + 0].addr);
//    const CcInt* pIdxTarget    = static_cast<CcInt*>(args[nOffset + 1].addr);
//    const CcInt* pIdxSourcePos = static_cast<CcInt*>(args[nOffset + 2].addr);
//    const CcInt* pIdxTargetPos = static_cast<CcInt*>(args[nOffset + 3].addr);
//    const CcInt* pIdxCurve     = static_cast<CcInt*>(args[nOffset + 4].addr);
//    const CcInt* pIdxRoadName  = static_cast<CcInt*>(args[nOffset + 5].addr);
//     const CcInt* pIdxRoadType  = static_cast<CcInt*>(args[nOffset + 6].addr);
//    const CcInt* pIdxMaxSpeed  = static_cast<CcInt*>(args[nOffset + 7].addr);
//    const CcInt* pIdxWayId     = static_cast<CcInt*>(args[nOffset + 8].addr);
// 
// 
//     Indexes.m_IdxSource    = pIdxSource->GetValue();
//     Indexes.m_IdxTarget    = pIdxTarget->GetValue();
//     Indexes.m_IdxSourcePos = pIdxSourcePos->GetValue();
//     Indexes.m_IdxTargetPos = pIdxTargetPos->GetValue();
//     Indexes.m_IdxCurve     = pIdxCurve->GetValue();
//     Indexes.m_IdxRoadName  = pIdxRoadName->GetValue();
//     Indexes.m_IdxRoadType  = pIdxRoadType->GetValue();
//     Indexes.m_IdxMaxSpeed  = pIdxMaxSpeed->GetValue();
//     Indexes.m_IdxWayId     = pIdxWayId->GetValue();
// 
//     if (Indexes.m_IdxSource < 0 || Indexes.m_IdxTarget < 0 ||
//         Indexes.m_IdxSourcePos < 0 || Indexes.m_IdxTargetPos < 0 ||
//         Indexes.m_IdxCurve < 0 || Indexes.m_IdxWayId < 0)
//     {
//         assert(false);
//     }
// 
//     return Indexes;
// }


// static ListExpr GetORelNetworkAttrIndexes(ListExpr ORelAttrList) {
//     ListExpr attrType;
//     int nAttrSourceIndex = 
//                 listutils::findAttribute(ORelAttrList, "Source", attrType);
//     if (nAttrSourceIndex == 0) 
//         return listutils::typeError("'Source' not found in attr list");
//     else {
//         if (!CcInt::checkType(attrType) &&
//             !LongInt::checkType(attrType)) {
//             return listutils::typeError(
//                                "'Source' must be " + CcInt::BasicType() +
//                                " or " + LongInt::BasicType());
//         }
//     }
// 
//     int nAttrTargetIndex = 
//                 listutils::findAttribute(ORelAttrList, "Target", attrType);
//     if (nAttrTargetIndex == 0) {
//       return listutils::typeError("'Target' not found in attr list");
//     }
//     else {
//         if (!CcInt::checkType(attrType) &&
//             !LongInt::checkType(attrType)) {
//             return listutils::typeError(
//                                    "'Target' must be " + CcInt::BasicType()
//                                    + " or " + LongInt::BasicType()) ;
//         }
//     }
// 
//     int nAttrSourcePosIndex = listutils::findAttribute(
//                                       ORelAttrList, "SourcePos", attrType);
//     if (nAttrSourcePosIndex == 0) {
//         return listutils::typeError("'SourcePos' not found in attr list");
//     }
//     else {
//         if (!listutils::isSymbol(attrType, Point::BasicType())) {
//             return listutils::typeError(
//                               "'SourcePos' must be " + Point::BasicType());
//         }
//     }
// 
//     int nAttrTargetPosIndex = listutils::findAttribute(
//                                       ORelAttrList, "TargetPos", attrType);
//     if (nAttrTargetPosIndex == 0) {
//         return listutils::typeError("'TargetPos' not found in attr list");
//     }
//     else {
//         if (!listutils::isSymbol(attrType, Point::BasicType())) {
//             return listutils::typeError(
//                                "'TargetPos' must be " + Point::BasicType());
//         }
//     }
// 
//     int nAttrCurveIndex = listutils::findAttribute(
//                                           ORelAttrList, "Curve", attrType);
//     if (nAttrCurveIndex == 0) {
//         return listutils::typeError("'Curve' not found in attr list");
//     }
//     else {
//         if (!listutils::isSymbol(attrType, SimpleLine::BasicType())) {
//             return listutils::typeError(
//                              "'Curve' must be " + SimpleLine::BasicType());
//         }
//     }
// 
//     int nAttrRoadNameIndex = listutils::findAttribute(
//                                         ORelAttrList, "RoadName", attrType);
//     if (nAttrRoadNameIndex != 0)
//     {
//         if (!listutils::isSymbol(attrType, FText::BasicType()))
//         {
//             return listutils::typeError(
//                                 "'RoadName' must be " + FText::BasicType());
//         }
//     }
// 
//     int nAttrRoadTypeIndex = listutils::findAttribute(
//                                        ORelAttrList, "RoadType", attrType);
//     if (nAttrRoadTypeIndex != 0)
//     {
//         if (!listutils::isSymbol(attrType, FText::BasicType()))
//         {
//             return listutils::typeError(
//                                "'RoadType' must be " + FText::BasicType());
//         }
//     }
// 
//     int nAttrMaxSpeedTypeIndex = listutils::findAttribute(
//                                         ORelAttrList, "MaxSpeed", attrType);
//     if (nAttrMaxSpeedTypeIndex != 0)
//     {
//         if (!listutils::isSymbol(attrType, FText::BasicType()))
//         {
//             return listutils::typeError(
//                                 "'MaxSpeed' must be " + FText::BasicType());
//         }
//     }
// 
//     int nAttrWayIdTypeIndex = listutils::findAttribute(
//                                           ORelAttrList, "WayId", attrType);
//     if (nAttrWayIdTypeIndex == 0)
//     {
//         return listutils::typeError("'WayId' not found in attr list");
//     }
//     else
//     {
//         if (!CcInt::checkType(attrType) &&
//             !LongInt::checkType(attrType))
//         {
//             return listutils::typeError( 
//                                     "'WayId' must be " + CcInt::BasicType()
//                                     + " or " + LongInt::BasicType());
//         }
//     }
// 
//     --nAttrSourceIndex;
//     --nAttrTargetIndex;
//     --nAttrSourcePosIndex;
//     --nAttrTargetPosIndex;
//     --nAttrCurveIndex;
//     --nAttrRoadNameIndex;
//     --nAttrRoadTypeIndex;
//     --nAttrMaxSpeedTypeIndex;
//     --nAttrWayIdTypeIndex;
// 
//     ListExpr Ind = nl->OneElemList(nl->IntAtom(nAttrSourceIndex));
//     ListExpr Last = Ind;
//     Last = nl->Append(Last, nl->IntAtom(nAttrTargetIndex));
//     Last = nl->Append(Last, nl->IntAtom(nAttrSourcePosIndex));
//     Last = nl->Append(Last, nl->IntAtom(nAttrTargetPosIndex));
//     Last = nl->Append(Last, nl->IntAtom(nAttrCurveIndex));
//     Last = nl->Append(Last, nl->IntAtom(nAttrRoadNameIndex));
//     Last = nl->Append(Last, nl->IntAtom(nAttrRoadTypeIndex));
//     Last = nl->Append(Last, nl->IntAtom(nAttrMaxSpeedTypeIndex));
//     Last = nl->Append(Last, nl->IntAtom(nAttrWayIdTypeIndex));
// 
//     return Ind;
// }


// static shared_ptr<mapmatch::MapMatchDataContainer> 
//                     GetMMDataFromTupleStream(Supplier Stream,
//                                              Word* args,
//                                              int nOffset) {
//     
//     shared_ptr<mapmatch::MapMatchDataContainer> 
//                             pContData(new mapmatch::MapMatchDataContainer);
// 
//     const CcInt* pIdxLat    = static_cast<CcInt*>(args[nOffset + 0].addr);
//     const CcInt* pIdxLon    = static_cast<CcInt*>(args[nOffset + 1].addr);
//     const CcInt* pIdxTime   = static_cast<CcInt*>(args[nOffset + 2].addr);
//     const CcInt* pIdxFix    = static_cast<CcInt*>(args[nOffset + 3].addr);
//     const CcInt* pIdxSat    = static_cast<CcInt*>(args[nOffset + 4].addr);
//     const CcInt* pIdxHdop   = static_cast<CcInt*>(args[nOffset + 5].addr);
//     const CcInt* pIdxVdop   = static_cast<CcInt*>(args[nOffset + 6].addr);
//     const CcInt* pIdxPdop   = static_cast<CcInt*>(args[nOffset + 7].addr);
//     const CcInt* pIdxCourse = static_cast<CcInt*>(args[nOffset + 8].addr);
//     const CcInt* pIdxSpeed  = static_cast<CcInt*>(args[nOffset + 9].addr);
//   //const CcInt* pIdxEle    = static_cast<CcInt*>(args[nOffset + 10].addr);
// 
//     const int nIdxLat    = pIdxLat->GetValue();
//     const int nIdxLon    = pIdxLon->GetValue();
//     const int nIdxTime   = pIdxTime->GetValue();
//     const int nIdxFix    = pIdxFix->GetValue();
//     const int nIdxSat    = pIdxSat->GetValue();
//     const int nIdxHdop   = pIdxHdop->GetValue();
//     const int nIdxVdop   = pIdxVdop->GetValue();
//     const int nIdxPdop   = pIdxPdop->GetValue();
//     const int nIdxCourse = pIdxCourse->GetValue();
//     const int nIdxSpeed  = pIdxSpeed->GetValue();
//     //const int nIdxEle    = pIdxEle->GetValue();
// 
//     if (nIdxLat < 0 || nIdxLon < 0 || nIdxTime < 0) {
//         assert(false);
//         return pContData;
//     }
// 
//     Word wTuple;
//     qp->Open(Stream);
//     qp->Request(Stream, wTuple);
//     while (qp->Received(Stream)) {
//       Tuple* pTpl = (Tuple*)wTuple.addr;
// 
//       CcReal* pLat    = static_cast<CcReal*>(pTpl->GetAttribute(nIdxLat));
//       CcReal* pLon    = static_cast<CcReal*>(pTpl->GetAttribute(nIdxLon));
//       DateTime* pTime = 
//                      static_cast<DateTime*>(pTpl->GetAttribute(nIdxTime));
// 
//       CcInt* pFix     = NULL;
//       if (nIdxFix >= 0)
//           pFix = static_cast<CcInt*>(pTpl->GetAttribute(nIdxFix));
// 
//       CcInt* pSat     = NULL;
//       if (nIdxSat >= 0)
//           pSat = static_cast<CcInt*>(pTpl->GetAttribute(nIdxSat));
// 
//       CcReal* pHdop   = NULL;
//       if (nIdxHdop >= 0)
//           pHdop = static_cast<CcReal*>(pTpl->GetAttribute(nIdxHdop));
// 
//       CcReal* pVdop   = NULL;
//       if (nIdxVdop >= 0)
//           pVdop = static_cast<CcReal*>(pTpl->GetAttribute(nIdxVdop));
// 
//       CcReal* pPdop   = NULL;
//       if (nIdxPdop >= 0)
//           pPdop = static_cast<CcReal*>(pTpl->GetAttribute(nIdxPdop));
// 
//       CcReal* pCourse = NULL;
//       if (nIdxCourse >= 0)
//           pCourse = static_cast<CcReal*>(pTpl->GetAttribute(nIdxCourse));
// 
//       CcReal* pSpeed  = NULL;
//       if (nIdxSpeed >= 0)
//           pSpeed = static_cast<CcReal*>(pTpl->GetAttribute(nIdxSpeed));
// 
//       if (pLat != NULL && pLon != NULL && pTime != NULL &&
//           pLat->IsDefined() && pLon->IsDefined() && pTime->IsDefined()) {
//         //cout << "found valid data point" << endl;
//           mapmatch::MapMatchData Data(pLat->GetValue(),
//                             pLon->GetValue(),
//                             pTime->millisecondsToNull());
// 
//           if (pFix != NULL && pFix->IsDefined())
//               Data.m_nFix = pFix->GetValue();
// 
//           if (pSat != NULL && pSat->IsDefined())
//               Data.m_nSat = pSat->GetValue();
// 
//           if (pHdop != NULL && pHdop->IsDefined())
//               Data.m_dHdop = pHdop->GetValue();
// 
//           if (pVdop != NULL && pVdop->IsDefined())
//               Data.m_dVdop = pVdop->GetValue();
// 
//           if (pPdop != NULL && pPdop->IsDefined())
//               Data.m_dPdop = pPdop->GetValue();
// 
//           if (pCourse != NULL && pCourse->IsDefined())
//               Data.m_dCourse = pCourse->GetValue();
// 
//           if (pSpeed != NULL && pSpeed->IsDefined())
//               Data.m_dSpeed = pSpeed->GetValue();
//           
//           pContData->Append(Data);
//       }
// 
//       pTpl->DeleteIfAllowed();
//       pTpl = NULL;
// 
//       qp->Request(Stream, wTuple);
//   }
//   qp->Close(Stream);
// 
//   return pContData;
// }


// static ListExpr GetMMDataIndexesOfTupleStream(ListExpr TupleStream) {
//     ListExpr attrType;
//     int nAttrLatIndex = listutils::findAttribute(
//                     nl->Second(nl->Second(TupleStream)), "Lat", attrType);
//     if(nAttrLatIndex==0)
//     {
//         return listutils::typeError("'Lat' not found in attr list");
//     }
//     else
//     {
//         if (!listutils::isSymbol(attrType, CcReal::BasicType()))
//         {
//             return listutils::typeError("'Lat' must be " +
//                                         CcReal::BasicType());
//         }
//     }
// 
//     int nAttrLonIndex = listutils::findAttribute(
//                      nl->Second(nl->Second(TupleStream)), "Lon", attrType);
//     if (nAttrLonIndex == 0)
//     {
//         return listutils::typeError("'Lon' not found in attr list");
//     }
//     else
//     {
//         if (!listutils::isSymbol(attrType, CcReal::BasicType()))
//         {
//             return listutils::typeError("'Lon' must be " +
//                                         CcReal::BasicType());
//         }
//     }
// 
//     int nAttrTimeIndex = listutils::findAttribute(
//                     nl->Second(nl->Second(TupleStream)), "Time", attrType);
//     if (nAttrTimeIndex == 0)
//     {
//         return listutils::typeError("'Time' not found in attr list");
//     }
//     else
//     {
//         if (!listutils::isSymbol(attrType, DateTime::BasicType()))
//         {
//             return listutils::typeError("'Time' must be " +
//                                         DateTime::BasicType());
//         }
//     }
// 
//     int nAttrFixIndex = listutils::findAttribute(
//                       nl->Second(nl->Second(TupleStream)), "Fix", attrType);
//     if (nAttrFixIndex != 0)
//     {
//         if (!listutils::isSymbol(attrType, CcInt::BasicType()))
//         {
//             return listutils::typeError("'Fix' must be " +
//                                         CcInt::BasicType());
//         }
//     }
// 
//     int nAttrSatIndex = listutils::findAttribute(
//                      nl->Second(nl->Second(TupleStream)), "Sat", attrType);
//     if (nAttrSatIndex != 0)
//     {
//         if (!listutils::isSymbol(attrType, CcInt::BasicType()))
//         {
//             return listutils::typeError("'Sat' must be " +
//                                         CcInt::BasicType());
//         }
//     }
// 
//     int nAttrHdopIndex = listutils::findAttribute(
//                     nl->Second(nl->Second(TupleStream)), "Hdop", attrType);
//     if (nAttrHdopIndex != 0)
//     {
//         if (!listutils::isSymbol(attrType, CcReal::BasicType()))
//         {
//             return listutils::typeError("'Hdop' must be " +
//                                         CcReal::BasicType());
//         }
//     }
// 
//     int nAttrVdopIndex = listutils::findAttribute(
//                     nl->Second(nl->Second(TupleStream)), "Vdop", attrType);
//     if (nAttrVdopIndex != 0)
//     {
//         if (!listutils::isSymbol(attrType, CcReal::BasicType()))
//         {
//             return listutils::typeError("'Vdop' must be " +
//                                         CcReal::BasicType());
//         }
//     }
// 
//     int nAttrPdopIndex = listutils::findAttribute(
//                     nl->Second(nl->Second(TupleStream)), "Pdop", attrType);
//     if (nAttrPdopIndex != 0)
//     {
//         if (!listutils::isSymbol(attrType, CcReal::BasicType()))
//         {
//             return listutils::typeError("'Pdop' must be " +
//                                         CcReal::BasicType());
//         }
//     }
// 
//     int nAttrCourseIndex = listutils::findAttribute(
//                   nl->Second(nl->Second(TupleStream)), "Course", attrType);
//     if (nAttrCourseIndex != 0)
//     {
//         if (!listutils::isSymbol(attrType, CcReal::BasicType()))
//         {
//             return listutils::typeError(
//                     "'Course' must be " + CcReal::BasicType());
//         }
//     }
// 
//     int nAttrSpeedIndex = listutils::findAttribute(
//                    nl->Second(nl->Second(TupleStream)), "Speed", attrType);
//     if (nAttrSpeedIndex != 0)
//     {
//         if (!listutils::isSymbol(attrType, CcReal::BasicType()))
//         {
//             return listutils::typeError(
//                     "'Speed' must be " + CcReal::BasicType());
//         }
//     }
// 
//     int nAttrEleIndex = listutils::findAttribute(
//                       nl->Second(nl->Second(TupleStream)), "Ele", attrType);
//     if (nAttrEleIndex != 0)
//     {
//         if (!listutils::isSymbol(attrType, CcReal::BasicType()))
//         {
//             return listutils::typeError(
//                     "'Ele' must be " + CcReal::BasicType());
//         }
//     }
// 
//     --nAttrLatIndex;
//     --nAttrLonIndex;
//     --nAttrTimeIndex;
//     --nAttrFixIndex;
//     --nAttrSatIndex;
//     --nAttrHdopIndex;
//     --nAttrVdopIndex;
//     --nAttrPdopIndex;
//     --nAttrCourseIndex;
//     --nAttrSpeedIndex;
//     --nAttrEleIndex;
// 
//     ListExpr Ind = nl->OneElemList(nl->IntAtom(nAttrLatIndex));
//     ListExpr Last = Ind;
//     Last = nl->Append(Last, nl->IntAtom(nAttrLonIndex));
//     Last = nl->Append(Last, nl->IntAtom(nAttrTimeIndex));
//     Last = nl->Append(Last, nl->IntAtom(nAttrFixIndex));
//     Last = nl->Append(Last, nl->IntAtom(nAttrSatIndex));
//     Last = nl->Append(Last, nl->IntAtom(nAttrHdopIndex));
//     Last = nl->Append(Last, nl->IntAtom(nAttrVdopIndex));
//     Last = nl->Append(Last, nl->IntAtom(nAttrPdopIndex));
//     Last = nl->Append(Last, nl->IntAtom(nAttrCourseIndex));
//     Last = nl->Append(Last, nl->IntAtom(nAttrSpeedIndex));
//     Last = nl->Append(Last, nl->IntAtom(nAttrEleIndex));
// 
//     return Ind;
// }


/*

7.38.2 General Type Mapping for Operators ~momapmatchmht~

*/
// ListExpr momapmatchmhtTypeMap_Common(ListExpr args,
//                                 MOMapMatchingMHTResultType eResultType) {
//     
//     if(nl->ListLength(args) != 4) {
//       return listutils::typeError("four arguments expected");
//     }
// 
//     // Check Network - OrderedRelation, RTree, Relation
// 
//     // mem(orel)
//     ListExpr arg1 = nl->First(args);
//     
//     if(!nl->HasLength(arg1,2)){
//         return listutils::typeError("internal error");
//     }
//     
//     string errMsg;
// 
//     if(!getMemType(nl->First(arg1), nl->Second(arg1), arg1, errMsg)){
//              return listutils::typeError("string or mem(orel) expected : " 
//                                           + errMsg);
//     }
// 
//     arg1 = nl->Second(arg1); // remove mem
//     if(!listutils::isOrelDescription(arg1)){
//     return listutils::typeError("memory object is not an ordered relation");
//     }
//     
//     ListExpr orelTuple = nl->Second(arg1);
//     if (!listutils::isTupleDescription(orelTuple))
//     {
//         return listutils::typeError
//                     ("2nd value of mem orel is not of type tuple");
//     }
// 
//     ListExpr orelAttrList = nl->Second(orelTuple);
//     if (!listutils::isAttrList(orelAttrList)) {
//         return listutils::typeError("Error in orel attrlist");
//     }
// 
//     // Check attributes of orel
//     ListExpr IndNetwAttr = GetORelNetworkAttrIndexes(orelAttrList);
// 
//     if(nl->Equal(IndNetwAttr, nl->TypeError()))
//       return IndNetwAttr;
// 
//     // mem(rtree 2)
//     ListExpr arg2 = nl->Second(args);
//     if(!getMemType(nl->First(arg2), nl->Second(arg2), arg2, errMsg)){
//       return listutils::typeError("\n problem in second arg: " + errMsg);
//     } 
//     
//     ListExpr rtreetype = nl->Second(arg2); // remove leading mem
//     if(!rtreehelper::checkType(rtreetype)){
//       return listutils::typeError("second arg is not a mem rtree");
//     }
//    
//     int rtreedim = nl->IntValue(nl->Second(rtreetype));
//     
//     if (rtreedim != 2) {
//         return listutils::typeError("rtree with dim==2 expected");
//     }
// 
//     // mem(rel)
//     ListExpr arg3 = nl->Third(args);
//     
//     if(!nl->HasLength(arg3,2)){
//         return listutils::typeError("internal error");
//     }
//     
//     if(!getMemType(nl->First(arg3), nl->Second(arg3), arg3, errMsg)){
//     return listutils::typeError("string or mem(rel) expected : " + errMsg);
//     }
// 
//     arg3 = nl->Second(arg3); // remove mem
//     if(!Relation::checkType(arg3)){
//       return listutils::typeError("memory object is not a relation");
//     }
// 
//     // GPS-Data (MPoint, FileName, TupleStream)
//     ListExpr arg4 = nl->First(nl->Fourth(args));
//     if (!listutils::isSymbol(arg4,temporalalgebra::MPoint::BasicType()) &&
//         !listutils::isSymbol(arg4,FText::BasicType()) &&
//         !listutils::isTupleStream(arg4)) {
//       
//         return listutils::typeError("4th argument must be " +
//                                     temporalalgebra::MPoint::BasicType() 
//                                     + " or " +
//                                     FText::BasicType() + " or " +
//                                     "tuple stream");
//     }
//    
//     // Result
//     ListExpr ResultType = nl->TheEmptyList();
// 
//     switch (eResultType) {
//       case OMM_RESULT_STREAM: {
//         ListExpr addAttrs = nl->TwoElemList(
//                     nl->TwoElemList(nl->SymbolAtom("StartTime"),
//                                     nl->SymbolAtom(DateTime::BasicType())),
//                     nl->TwoElemList(nl->SymbolAtom("EndTime"),
//                                     nl->SymbolAtom(DateTime::BasicType())));
// 
//         ListExpr ResAttr = AppendLists(orelAttrList, addAttrs);
// 
//         ResultType = nl->TwoElemList(nl->SymbolAtom(Symbol::STREAM()),
//                                       nl->TwoElemList(
//                                         nl->SymbolAtom(Tuple::BasicType()),
//                                         ResAttr));
//       }
//       break;
// 
//       case OMM_RESULT_GPX: {
//         ListExpr addAttrs = nl->SixElemList(
//                     nl->TwoElemList(nl->SymbolAtom("StartTime"),
//                                     nl->SymbolAtom(DateTime::BasicType())),
//                     nl->TwoElemList(nl->SymbolAtom("EndTime"),
//                                     nl->SymbolAtom(DateTime::BasicType())),
//                     nl->TwoElemList(nl->SymbolAtom("StartPos"),
//                                     nl->SymbolAtom(Point::BasicType())),
//                     nl->TwoElemList(nl->SymbolAtom("EndPos"),
//                                     nl->SymbolAtom(Point::BasicType())),
//                     nl->TwoElemList(nl->SymbolAtom("StartLength"),
//                                     nl->SymbolAtom(CcReal::BasicType())),
//                     nl->TwoElemList(nl->SymbolAtom("EndLength"),
//                                     nl->SymbolAtom(CcReal::BasicType())));
// 
//         ListExpr ResAttr = AppendLists(orelAttrList, addAttrs);
// 
//         ResultType = nl->TwoElemList(nl->SymbolAtom(Symbol::STREAM()),
//                                      nl->TwoElemList(
//                                         nl->SymbolAtom(Tuple::BasicType()),
//                                         ResAttr));
//       }
//       break;
// 
//       case OMM_RESULT_MPOINT: {
//           ResultType = nl->SymbolAtom(temporalalgebra::MPoint::BasicType());
//       }
//       break;
//     }
// 
// 
//     if (listutils::isTupleStream(arg4)) {
//       ListExpr IndMMData = GetMMDataIndexesOfTupleStream(arg4);
// 
//       if (nl->Equal(IndMMData, nl->TypeError()))
//           return IndMMData;
//       else {
//           return nl->ThreeElemList(nl->SymbolAtom(Symbol::APPEND()),
//                                     AppendLists(IndNetwAttr, IndMMData),
//                                     ResultType);
//       }
//     }
//     else {
//       return nl->ThreeElemList(nl->SymbolAtom(Symbol::APPEND()),
//                                 IndNetwAttr,
//                                 ResultType);
//     }
// }


// ListExpr momapmatchmhtTypeMap(ListExpr in_xArgs) {
//     return momapmatchmhtTypeMap_Common(in_xArgs, OMM_RESULT_STREAM);
// }

/*

7.38.3 Value Mapping Function of operator ~momapmatchmht~

*/
// template<class T, class R, class Q>
// int momapmatchmhtMPointValueMap(Word* args,
//                                 Word& result,
//                                 int message,
//                                 Word& local,
//                                 Supplier s,
//           typename mapmatching::MmORelEdgeStreamCreator<T>::EMode eMode) {
//   
// //     cout << "momapmatchmhtMPointValueMap called" << endl;
// 
//     mapmatching::MmORelEdgeStreamCreator<T>* pCreator =
//           static_cast<mapmatching::MmORelEdgeStreamCreator<T>*>(local.addr);
//     switch (message) {
//       
//       case OPEN: {
//         if(pCreator) {
//             delete pCreator;
//             pCreator = NULL;
//         }
// 
//         // mem(orel)
//         R* orelN = (R*) args[0].addr;
//         MemoryORelObject* orel = 
//                  getMemORel(orelN,nl->Second(qp->GetType(s)));
//         if(!orel) 
//           return 0;
//         
//         // mem(rtree)
//         Q* treeN = (Q*) args[1].addr;
//         string name = treeN->GetValue();
//         // cut blank from the front of the string
//         name = name.substr(1, name.size()-1);
//         MemoryRtreeObject<2>* tree =
//               (MemoryRtreeObject<2>*)catalog->getMMObject(name);
//         if(!tree) 
//           return 0;
//            
//         // mem(rel)
//         R* relN = (R*) args[2].addr;
//         MemoryRelObject* rel = getMemRel(relN);
//         if(!rel)
//           return 0;
//        
//         typename mapmatching::MmORelNetwork<T>::OEdgeAttrIndexes Indexes = 
//                                            GetOEdgeAttrIndexes<T>(args, 4);
// 
//         mapmatching::MmORelNetwork<T> Network(orel, tree, rel, Indexes);
// 
//         temporalalgebra::MPoint* pMPoint = 
//                     static_cast<temporalalgebra::MPoint*>(args[3].addr);
// 
//         // Do Map Matching
//         mapmatching::MmORelNetworkAdapter<T> NetworkAdapter(&Network);
//         mapmatch::MapMatchingMHT MapMatching(&NetworkAdapter, pMPoint);
//                     
//         mapmatching::MmORelEdgeStreamCreator<T>* pCreator =
//         new mapmatching::MmORelEdgeStreamCreator<T>(s,NetworkAdapter,eMode);
//         if (!MapMatching.DoMatch(pCreator)) {
//             // Error
//             delete pCreator;
//             pCreator = NULL;
//         }
// 
//         local.setAddr(pCreator);
//           return 0;
//       }
//       case REQUEST: {
//         if (pCreator == NULL) {
//           return CANCEL;
//         }
//         else {
//           result.addr = pCreator->GetNextTuple();
//           return result.addr ? YIELD : CANCEL;
//         }
//       }
//       case CLOSE: {
//         if (pCreator) {
//           delete pCreator;
//           local.addr = NULL;
//         }
//           return 0;
//       }
//       default: {
//         return 0;
//       }
//     }
// }


// template<class T, class R, class Q>
// int momapmatchmhtTextValueMap(Word* args,
//                               Word& result,
//                               int message,
//                               Word& local,
//                               Supplier s,
//          typename mapmatching::MmORelEdgeStreamCreator<T>::EMode eMode) {
// 
// //     cout << "momapmatchmhtTextValueMap called" << endl;
// 
//     mapmatching::MmORelEdgeStreamCreator<T>* pCreator =
//          static_cast<mapmatching::MmORelEdgeStreamCreator<T>*>(local.addr);
// 
//     switch (message) {
//       case OPEN: {
// 
//         if(pCreator) {
//             delete pCreator;
//             pCreator = NULL;
//         }
// 
//         // mem(orel)
//         R* orelN = (R*) args[0].addr;
//         MemoryORelObject* orel = 
//                              getMemORel(orelN,nl->Second(qp->GetType(s)));
//         if(!orel) 
//           return 0;
// 
//         //mem(rtree)
//         Q* treeN = (Q*) args[1].addr;
//         string name = treeN->GetValue();
//         // cut blank from the front of the string
//         name = name.substr(1, name.size()-1);
//         MemoryRtreeObject<2>* tree =
//               (MemoryRtreeObject<2>*)catalog->getMMObject(name);
//         if(!tree) 
//           return 0;
// 
//         // mem(rel)
//         R* relN = (R*) args[2].addr;
//         MemoryRelObject* rel = getMemRel(relN);
//         if(!rel)
//           return 0;;
//          
//         typename mapmatching::MmORelNetwork<T>::OEdgeAttrIndexes Indexes = 
//                                           GetOEdgeAttrIndexes<T>(args, 4);
// 
//         mapmatching::MmORelNetwork<T> Network(orel, tree, rel, Indexes);
//         FText* pFileName = static_cast<FText*>(args[3].addr);
//         std::string strFileName = pFileName->GetValue();
// 
//         // Do Map Matching
//         mapmatching::MmORelNetworkAdapter<T> NetworkAdapter(&Network);
//         mapmatch::MapMatchingMHT MapMatching(&NetworkAdapter, strFileName);
//         mapmatching::MmORelEdgeStreamCreator<T>* pCreator =
//         new mapmatching::MmORelEdgeStreamCreator<T>(s,NetworkAdapter,eMode);
//         
//         if (!MapMatching.DoMatch(pCreator)) {
//             //Error
//             delete pCreator;
//             pCreator = NULL;
//         }
// 
//         local.setAddr(pCreator);
//         return 0;
//       }
//       case REQUEST: {
//         if(pCreator == NULL) {
//           return CANCEL;
//         }
//         else {
//           result.addr = pCreator->GetNextTuple();
//           return result.addr ? YIELD : CANCEL;
//         }
//       }
//       case CLOSE: {
//         if (pCreator) {
//           delete pCreator;
//           local.addr = NULL;
//         }
//         return 0;
//       }
//       default: {
//         return 0;
//       }
//     }
// }


// template<class T, class R, class Q>
// int momapmatchmhtStreamValueMap(Word* args,
//                                 Word& result,
//                                 int message,
//                                 Word& local,
//                                 Supplier s,
//             typename mapmatching::MmORelEdgeStreamCreator<T>::EMode eMode) {
//   
// //     cout << "momapmatchmhtStreamValueMap called" << endl;
// 
//     mapmatching::MmORelEdgeStreamCreator<T>* pCreator =
//           static_cast<mapmatching::MmORelEdgeStreamCreator<T>*>(local.addr);
//     
//     switch (message) {
//     case OPEN: {
//       
//       if (pCreator != NULL) {
//           delete pCreator;
//           pCreator = NULL;
//       }
// 
//       // mem(orel)
//       R* orelN = (R*) args[0].addr;
//       MemoryORelObject* orel = getMemORel(orelN,nl->Second(qp->GetType(s)));
//       if(!orel) 
//         return 0;
//       
//       // mem(rtree)
//       Q* treeN = (Q*) args[1].addr;
//       string name = treeN->GetValue();
//       // cut blank from the front of the string
//       name = name.substr(1, name.size()-1);
//       MemoryRtreeObject<2>* tree =
//             (MemoryRtreeObject<2>*)catalog->getMMObject(name);
//       if(!tree) 
//         return 0;
//           
//       // mem(rel)
//       R* relN = (R*) args[2].addr;
//       MemoryRelObject* rel = getMemRel(relN);
//       if(!rel)
//         return 0;
// 
//       typename mapmatching::MmORelNetwork<T>::OEdgeAttrIndexes Indexes = 
//                                            GetOEdgeAttrIndexes<T>(args, 4);
// 
//       mapmatching::MmORelNetwork<T> Network(orel, tree, rel, Indexes);
// 
//       shared_ptr<mapmatch::MapMatchDataContainer> pContData = 
//                   GetMMDataFromTupleStream(args[3].addr, args, 4 + 9);
//                   // 9 OEdge-Attr-Indexes
//       // Do Map Matching
//       mapmatching::MmORelNetworkAdapter<T> NetworkAdapter(&Network);
//       mapmatch::MapMatchingMHT MapMatching(&NetworkAdapter, pContData); 
//       mapmatching::MmORelEdgeStreamCreator<T>* pCreator =
//        new mapmatching::MmORelEdgeStreamCreator<T>(s,NetworkAdapter,eMode);
// 
//       if (!MapMatching.DoMatch(pCreator)) {
//           //Error
//           delete pCreator;
//           pCreator = NULL;
//       }
// 
//       local.setAddr(pCreator);
//       return 0;
//     }
//     case REQUEST: {
//       if (pCreator == NULL) {
//         return CANCEL;
//       }
//       else {
//         result.addr = pCreator->GetNextTuple();
//         return result.addr ? YIELD : CANCEL;
//       }
//     }
//     case CLOSE: {
//       if (pCreator) {
//         delete pCreator;
//         local.addr = NULL;
//       }
//       return 0;
//     }
//     default: {
//       return 0;
//     }
//   }
// }
 

// template<class T, class R, class Q>
// int momapmatchmhtMPointVMT(Word* args,
//                            Word& result,
//                            int message,
//                            Word& local,
//                            Supplier in_xSupplier) {
//   
//     return momapmatchmhtMPointValueMap<T,R,Q>(
//                   args,
//                   result,
//                   message,
//                   local,
//                   in_xSupplier,
//                   mapmatching::MmORelEdgeStreamCreator<T>::MODE_EDGES);
// }
// 
// 
// template<class T, class R, class Q>
// int momapmatchmhtTextVMT(Word* args,
//                         Word& result,
//                         int message,
//                         Word& local,
//                         Supplier in_xSupplier) {
//   
//     return momapmatchmhtTextValueMap<T,R,Q>(
//                     args,
//                     result,
//                     message,
//                     local,
//                     in_xSupplier,
//                     mapmatching::MmORelEdgeStreamCreator<T>::MODE_EDGES);
// }


// template<class T, class R, class Q>
// int momapmatchmhtStreamVMT(Word* args,
//                            Word& result,
//                            int message,
//                            Word& local,
//                            Supplier in_xSupplier) {
//   
//     return momapmatchmhtStreamValueMap<T,R,Q>(
//                       args,
//                       result,
//                       message,
//                       local,
//                       in_xSupplier,
//                       mapmatching::MmORelEdgeStreamCreator<T>::MODE_EDGES);
// }


/*

7.38.4 Selection Function ~momapmatchmht~

*/
// int momapmatchmhtSelect(ListExpr args) {
//   
//     NList type(args);
//     if (type.length() == 4) {
//       
//         if (type.fourth().isSymbol(temporalalgebra::MPoint::BasicType())) 
//             return 0/* + offset*/;
//         
//         else if (type.fourth().isSymbol(FText::BasicType())) 
//             return 1 /*+ offset*/;
//         
//         else if (listutils::isTupleStream(type.fourth().listExpr())) 
//             return 2/* + offset*/;
//         
//         else 
//             return -1;        
//     }
//     else 
//       return -1;
// }


// ValueMapping momapmatchmhtVM[] = {
//   momapmatchmhtMPointVMT<CcInt,CcString,Mem>,
//   momapmatchmhtTextVMT<CcInt,CcString,Mem>,
//   momapmatchmhtStreamVMT<CcInt,CcString,Mem>
// };

 
/*

7.38.5 Description of operator ~momapmatchmht~

*/
// OperatorSpec momapmatchmhtSpec(
//     "mem(orel(tuple(x))) x mem(rtree) x mem(rel(tuple(x)) x var "
//     "-> stream(tuple(x))",
//     "momapmatchmht (_,_,_,_)",
//     "map matching with a main memory ordered relation "
//     "Result: Tuples of matched edges with timestamps",
//     "query momapmatchmht('Edges','EdgeIndex_Box_rtree', "
//     "'EdgeIndex','Trk_MapMatchTest.gpx') count"
// );


/*
7.38.5 Instance of operator ~momapmatchmht~

*/
// Operator momapmatchmhtOp (
//     "momapmatchmht",
//     momapmatchmhtSpec.getStr(),
//     3,
//     momapmatchmhtVM,
//     momapmatchmhtSelect,
//     momapmatchmhtTypeMap
// );



GenTC<Mem> MemTC;



TypeConstructor MPointerTC(
   MPointer::BasicType(),
   MPointer::Property,
   MPointer::Out, MPointer::In,
   0,0,
   MPointer::Create, MPointer::Delete,
   MPointer::Open, MPointer::Save,
   MPointer::Close, MPointer::Clone,
   MPointer::Cast,
   MPointer::SizeOf,
   MPointer::TypeCheck
);


/*

9 Operations on Attribute vectors

9.1 collect[_]mvector

stream(DATA) x string x bool -> bool;

*/
ListExpr collect_mvectorTM(ListExpr args){
  if(!nl->HasLength(args,2)){
    return listutils::typeError("2 arguments expected");
  }
  string err = "stream(DATA) x bool expected";
  if(  !Stream<Attribute>::checkType(nl->First(args))
     ||!CcBool::checkType(nl->Second(args))){
    return listutils::typeError(err);
  }
  return MPointer::wrapType(Mem::wrapType(MemoryVectorObject::wrapType(
                           nl->Second(nl->First(args)))));
}

int collect_mvectorVM(Word* args, Word& result, int message,
                Word& local, Supplier s){
    CcBool* flob = (CcBool*) args[1].addr;
    result = qp->ResultStorage(s);
    MPointer* res = (MPointer*) result.addr;
    if(!flob->IsDefined()){
      res->setPointer(0);
      return 0;
    }
    string db =  SecondoSystem::GetInstance()->GetDatabaseName();
    ListExpr st = qp->GetType(qp->GetSon(s,0));
    st = nl->Second(qp->GetType(s));
    string type = nl->ToString(st);
    MemoryVectorObject* v = new MemoryVectorObject(flob->GetValue(),db,type);
    Stream<Attribute> stream(args[0]);
    stream.open();
    Attribute* a;
    while((a = stream.request())){
        v->add(a,true);
        a->DeleteIfAllowed();
    }
    stream.close();
    res->setPointer(v);
    v->deleteIfAllowed(); 
    return 0;                            
}

OperatorSpec collect_mvectorSpec(
  "stream(X) x bool -> mpointer(mem(mvector(X))) , X in DATA",
  " _ collect_mvector[_]",
  " Collects a stream of attributes into a main memory"
  " vector. The second argument specifies whether "
  " flobs should be loaded into memory too.",
  " query plz feed projecttransformstream[PLZ] collect_mvector[TRUE] "
);

Operator collect_mvectorOp(
  "collect_mvector",
  collect_mvectorSpec.getStr(),
  collect_mvectorVM,
  Operator::SimpleSelect,
  collect_mvectorTM
);


/*
9.2 sizemv

Returns the size of an mvector.

*/


ListExpr sizemvTM(ListExpr args){
   if(!nl->HasLength(args,1)){
     return listutils::typeError("one arg expected");
   }
   ListExpr a;
   if(!getMemSubType(nl->First(args),a)){
     return listutils::typeError("first argument is not a memory object");
   }
   if(!MemoryVectorObject::checkType(a)){
     return listutils::typeError("argument is not an mvector");
   }
   return  listutils::basicSymbol<CcInt>();
}


template<class T>
int sizemvVMT(Word* args, Word& result, int message,
                Word& local, Supplier s){

   T* arg = (T*) args[0].addr;
   result = qp->ResultStorage(s);
   CcInt* res = (CcInt*) result.addr;
   MemoryVectorObject* mv = getMVector(arg);
   if(!mv){
     res->SetDefined(false);
   } else {
     res->Set(true, mv->size());
   }
   return 0;
}

ValueMapping sizemvVM[] = {
   sizemvVMT<Mem>,
   sizemvVMT<MPointer>
};


int sizemvSelect(ListExpr args){
  ListExpr a = nl->First(args);
  if(Mem::checkType(a)) return 0;
  if(MPointer::checkType(a)) return 1;
  return -1;
}

OperatorSpec sizemvSpec(
  "{mem,mpointer} -> int ",
  "sizemv(_)",
  "Returns the size of a memory vector.",
  "query sizemv(v1)"

);

Operator sizemvOp(
  "sizemv",
  sizemvSpec.getStr(),
  2,
  sizemvVM,
  sizemvSelect,
  sizemvTM

);



/*
9.3 getmv

Returns an element of an mvector.

*/


ListExpr getmvTM(ListExpr args){
   if(!nl->HasLength(args,2)){
     return listutils::typeError("one arg expected");
   }
   if(!CcInt::checkType(nl->Second(args))){
     return listutils::typeError("Second arg has to be an int");
   }
   ListExpr a;
   if(!getMemSubType(nl->First(args),a)){
     return listutils::typeError("first arg is not a memory object");
   }
   if(!MemoryVectorObject::checkType(a)){
     return listutils::typeError("argument is not an mvector");
   }
   return  listutils::basicSymbol<CcInt>();
}


template<class T>
int getmvVMT(Word* args, Word& result, int message,
                Word& local, Supplier s){

   result = qp->ResultStorage(s);
   Attribute* res = (Attribute*) result.addr;
   CcInt* spos = (CcInt*) args[1].addr;
   if(!spos->IsDefined() ){
     res->SetDefined(false);
     return 0;
   }
   int pos = spos->GetValue();

   T* arg = (T*) args[0].addr;

   MemoryVectorObject* mv = getMVector(arg);
   if(!mv){
     res->SetDefined(false);
   } else {
     if(pos<0 || (size_t)pos>=mv->size()){
        res->SetDefined(0);
     } else {
        res->CopyFrom(mv->get(pos));
     }
   }
   return 0;
}

ValueMapping getmvVM[] = {
   getmvVMT<Mem>,
   getmvVMT<MPointer>
};


int getmvSelect(ListExpr args){
  ListExpr a = nl->First(args);
  if(Mem::checkType(a)) return 0;
  if(MPointer::checkType(a)) return 1;
  return -1;
}

OperatorSpec getmvSpec(
  "{mem,mpointer} x int -> DATA ",
  "_ getmv[_ ]",
  "Returns an specified element of an mvector",
  "query v1 getmv[42]"
);

Operator getmvOp(
  "getmv",
  getmvSpec.getStr(),
  2,
  getmvVM,
  getmvSelect,
  getmvTM
);


/*
9.3 putmv

Changes a element of an mvector. Returns the old element. 

*/


ListExpr putmvTM(ListExpr args){
   if(!nl->HasLength(args,3)){
     return listutils::typeError("one arg expected");
   }
   if(!CcInt::checkType(nl->Second(args))){
     return listutils::typeError("Second arg has to be an int");
   }
   ListExpr newValue = nl->Third(args);
   if(!Attribute::checkType(newValue)){
     return listutils::typeError("thir arg not in kind DATA");
   }

   ListExpr a;
   if(!getMemSubType(nl->First(args),a)){
    return listutils::typeError("fisrt arg is not a memory object");
   }
   if(!MemoryVectorObject::checkType(a)){
     return listutils::typeError("argument is not an mvector");
   }
   if(!nl->Equal(nl->Second(a),newValue)){
     return listutils::typeError("new value has another type than the vector");
   }
   return  listutils::basicSymbol<CcInt>();
}


template<class T>
int putmvVMT(Word* args, Word& result, int message,
                Word& local, Supplier s){

   result = qp->ResultStorage(s);
   Attribute* res = (Attribute*) result.addr;
   CcInt* spos = (CcInt*) args[1].addr;
   if(!spos->IsDefined() ){
     res->SetDefined(false);
     return 0;
   }
   int pos = spos->GetValue();

   Attribute* attr = (Attribute*) args[2].addr;

   T* arg = (T*) args[0].addr;

   MemoryVectorObject* mv = getMVector(arg);
   if(!mv){
     res->SetDefined(false);
   } else {
     if(pos<0 || (size_t)pos>=mv->size()){
        res->SetDefined(0);
     } else {
        Attribute* old = mv->get(pos);
        mv->put(pos, attr, true);
        res->CopyFrom(old);
        old->DeleteIfAllowed();
     }
   }
   return 0;
}

ValueMapping putmvVM[] = {
   putmvVMT<Mem>,
   putmvVMT<MPointer>
};


int putmvSelect(ListExpr args){
  ListExpr a = nl->First(args);
  if(Mem::checkType(a)) return 0;
  if(MPointer::checkType(a)) return 1;
  return -1;
}

OperatorSpec putmvSpec(
  "{mem,mpointer} x int x DATA -> DATA ",
  "_ getmv[_ ]",
  "Replaces an specified element in an mvector, returns the old element.",
  "query v1  putmv[17, 'newValue'] "
);

Operator putmvOp(
  "putmv",
  putmvSpec.getStr(),
  2,
  putmvVM,
  putmvSelect,
  putmvTM
);

/*
9.5 sizemv

Check whether an mvector is sorted.

*/


ListExpr isSortedmvTM(ListExpr args){
   if(!nl->HasLength(args,1)){
     return listutils::typeError("one arg expected");
   }
   ListExpr a;
   if(!getMemSubType(nl->First(args),a)){
     return listutils::typeError("first argument is not a memory object");
   }
   if(!MemoryVectorObject::checkType(a)){
     return listutils::typeError("argument is not an mvector");
   }
   return  listutils::basicSymbol<CcBool>();
}


template<class T>
int isSortedmvVMT(Word* args, Word& result, int message,
                Word& local, Supplier s){

   T* arg = (T*) args[0].addr;
   result = qp->ResultStorage(s);
   CcBool* res = (CcBool*) result.addr;
   MemoryVectorObject* mv = getMVector(arg);
   if(!mv){
     res->SetDefined(false);
   } else {
     res->Set(true, mv->isSorted());
   }
   return 0;
}

ValueMapping isSortedmvVM[] = {
   isSortedmvVMT<Mem>,
   isSortedmvVMT<MPointer>
};


int isSortedmvSelect(ListExpr args){
  ListExpr a = nl->First(args);
  if(Mem::checkType(a)) return 0;
  if(MPointer::checkType(a)) return 1;
  return -1;
}

OperatorSpec isSortedmvSpec(
  "{mem,mpointer} -> bool ",
  "issortedmv(_)",
  "Checks whether an mvector is known as sorted",
  "query isSortedmv(v1)"

);

Operator isSortedmvOp(
  "isSortedmv",
  isSortedmvSpec.getStr(),
  2,
  isSortedmvVM,
  isSortedmvSelect,
  isSortedmvTM
);



/*
9.5 sortmv

Check whether an mvector is sorted.

*/


ListExpr sortmvTM(ListExpr args){
   if(!nl->HasLength(args,1)){
     return listutils::typeError("one arg expected");
   }
   ListExpr arg1 = nl->First(args);
   if(MPointer::checkType(arg1)){
     arg1 = nl->Second(arg1);
   }
   if(!Mem::checkType(arg1)){
     return listutils::typeError("argument not of type mpointer or mem");
   }
   arg1 = nl->Second(arg1);
   if(!MemoryVectorObject::checkType(arg1)){
     return listutils::typeError("argument is not an mvector");
   }
   return MPointer::wrapType(Mem::wrapType(arg1)); 

}


template<class T>
int sortmvVMT(Word* args, Word& result, int message,
                Word& local, Supplier s){

   T* arg = (T*) args[0].addr;
   result = qp->ResultStorage(s);
   MPointer* res = (MPointer*) result.addr;
   MemoryVectorObject* mv = getMVector(arg);
   if(mv){
     mv->sort();
   }
   res->setPointer(mv);
   return 0;
}

ValueMapping sortmvVM[] = {
   sortmvVMT<Mem>,
   sortmvVMT<MPointer>
};


int sortmvSelect(ListExpr args){
  ListExpr a = nl->First(args);
  if(Mem::checkType(a)) return 0;
  if(MPointer::checkType(a)) return 1;
  return -1;
}

OperatorSpec sortmvSpec(
  "{mem,string,mpointer} -> mpointer ",
  "sort(_)",
  "Sorts an mvector.",
  "query sort(v1)"

);

Operator sortmvOp(
  "sortmv",
  sortmvSpec.getStr(),
  2,
  sortmvVM,
  sortmvSelect,
  sortmvTM
);

/*
9.8 feedmv

Feeds the elements of an mvector into a stream.

*/


ListExpr feedmvTM(ListExpr args){
   if(!nl->HasLength(args,1)){
     return listutils::typeError("one arg expected");
   }
   ListExpr a;
   if(!getMemSubType(nl->First(args),a)){
     return listutils::typeError("first arg is not a memory object");
   }
   if(!MemoryVectorObject::checkType(a)){
     return listutils::typeError("argument is not an mvector");
   }
   return nl->TwoElemList( listutils::basicSymbol<Stream<Attribute> >(),
                           nl->Second(a)); 

}

class feedmvInfo{
  public:
    feedmvInfo(MemoryVectorObject* _v): v(_v), pos(0), max(_v->size()){}

    ~feedmvInfo(){}

    Attribute* next(){
      if(pos>=max){
        return 0;
      }
      return v->get(pos++)->Copy();
    }

  private:
    MemoryVectorObject* v;
    size_t pos;
    size_t max;

};


template<class T>
int feedmvVMT(Word* args, Word& result, int message,
                Word& local, Supplier s){

   feedmvInfo* li = (feedmvInfo*) local.addr;
   switch(message){
      case OPEN: {
           if(li){
             delete li;
             local.addr = 0;
           }
           T* arg = (T*) args[0].addr;
           MemoryVectorObject* mv = getMVector(arg);
           if(mv){
             local.addr = new feedmvInfo(mv);
           }
           return 0;
        }
      case REQUEST:
           result.addr = li?li->next():0;
           return result.addr?YIELD:CANCEL;
      case CLOSE:
           if(li){
             delete li;
             local.addr = 0;
           }  
    }
    return -1;
}

ValueMapping feedmvVM[] = {
   feedmvVMT<Mem>,
   feedmvVMT<MPointer>
};


int feedmvSelect(ListExpr args){
  ListExpr a = nl->First(args);
  if(Mem::checkType(a)) return 0;
  if(MPointer::checkType(a)) return 1;
  return -1;
}

OperatorSpec feedmvSpec(
  "{mem,mpointer} -> stream(DATA) ",
  "_ feedmv",
  "feeds an mvector.",
  "query v1 feedmv count"
);

Operator feedmvOp(
  "feedmv",
  feedmvSpec.getStr(),
  2,
  feedmvVM,
  feedmvSelect,
  feedmvTM
);

/*
9.8 findmv

Returns the position within an sorted mvector where
an element is located or would be inserted (moving 
elements to right).

*/


ListExpr findmvTM(ListExpr args){
 if(!nl->HasLength(args,2)){
   return listutils::typeError("two arg expected");
 }
 ListExpr v = nl->Second(args);
 if(!Attribute::checkType(v)){
   return listutils::typeError("second arg is not in kind DATA");
 }
 ListExpr a;
 if(!getMemSubType(nl->First(args),a)){
   return listutils::typeError("first arg is not a memory object");
 }

 if(!MemoryVectorObject::checkType(a)){
   return listutils::typeError("argument is not an mvector");
 }
 ListExpr sa = nl->Second(a);
 if(!nl->Equal(sa,v)){
   return listutils::typeError("subtype of vector and search type differ");
 }

 return  listutils::basicSymbol<CcInt>();
}


template<class T>
int findmvVMT(Word* args, Word& result, int message,
              Word& local, Supplier s){

 T* arg = (T*) args[0].addr;
 result = qp->ResultStorage(s);
 CcInt* res = (CcInt*) result.addr;
 MemoryVectorObject* mv = getMVector(arg);
 if(!mv){
   res->SetDefined(false);
   return 0;
 }
 if(!mv->isSorted()){
   res->SetDefined(false);
   return 0;
 }
 if(mv->size()<1){
   res->Set(true,0);
   return 0;
 }
 Attribute* attr = (Attribute*) args[1].addr;
 res->Set(true,mv->binSearch(attr));
 return 0;
}

ValueMapping findmvVM[] = {
 findmvVMT<Mem>,
 findmvVMT<MPointer>
};


int findmvSelect(ListExpr args){
ListExpr a = nl->First(args);
if(Mem::checkType(a)) return 0;
if(MPointer::checkType(a)) return 1;
return -1;
}

OperatorSpec findmvSpec(
"{mem, mpointer} x  DATA -> int ",
"findmv(_)",
"Returns the position (or a potential insertion position) "
"of a value within an ordered  memory vector.",
"query findmv(v1, 'Value' )"
);

Operator findmvOp(
"findmv",
findmvSpec.getStr(),
2,
findmvVM,
findmvSelect,
findmvTM
);


/*
9.8 matchbelow

Returns the element of an mvector that is smaller or equal than
a given object.


*/


ListExpr matchbelowmvTM(ListExpr args){
 if(!nl->HasLength(args,2)){
   return listutils::typeError("two arg expected");
 }
 ListExpr v = nl->Second(args);
 if(!Attribute::checkType(v)){
   return listutils::typeError("second arg is not in kind DATA");
 }
 ListExpr a;
 if(!getMemSubType(nl->First(args),a)){
   return listutils::typeError("first arg is not a memory object");
 }
 if(!MemoryVectorObject::checkType(a)){
   return listutils::typeError("argument is not an mvector");
 }
 
 ListExpr sa = nl->Second(a);
 if(!nl->Equal(sa,v)){
   return listutils::typeError("subtype of vector and search type differ");
 }
 return  sa;
}


template<class T>
int matchbelowmvVMT(Word* args, Word& result, int message,
              Word& local, Supplier s){

 T* arg = (T*) args[0].addr;
 result = qp->ResultStorage(s);
 Attribute* res = (Attribute*) result.addr;
 MemoryVectorObject* mv = getMVector(arg);
 if(!mv){
   res->SetDefined(false);
   return 0;
 }
 Attribute* attr = (Attribute*) args[1].addr;
 Attribute* resv = mv->matchBelow(attr);
 if(!resv){
     res->SetDefined(false);
     return 0;
 }
 res->CopyFrom(resv);
 return 0;
}

ValueMapping matchbelowmvVM[] = {
 matchbelowmvVMT<Mem>,
 matchbelowmvVMT<MPointer>
};


int matchbelowmvSelect(ListExpr args){
  ListExpr a = nl->First(args);
  if(Mem::checkType(a)) return 0;
  if(MPointer::checkType(a)) return 1;
  return -1;
}

OperatorSpec matchbelowmvSpec(
"{mem, mpointer} x  DATA -> DATA ",
" _ matchbelowmv[_]",
"Returns the element of an mvector that is  "
"smaller or equal to agiven object.",
"query v1 matchbelowmv( 'Value' )"

);

Operator matchbelowmvOp(
  "matchbelowmv",
  matchbelowmvSpec.getStr(),
  2,
  matchbelowmvVM,
  matchbelowmvSelect,
  matchbelowmvTM
);


/*
9.9 Operator insertmv

*/
ListExpr insertmvTM(ListExpr args){
  if(!nl->HasLength(args,2) && !nl->HasLength(args,3)){
    return listutils::typeError("two or three args expected");
  }
  
  ListExpr stream = nl->First(args);
  ListExpr attrType = nl->TheEmptyList();
  ListExpr vector = nl->TheEmptyList();
  int index = 0;
  if(nl->HasLength(args,2)){
    if(!Stream<Attribute>::checkType(stream)){
       return listutils::typeError("If two arguments are given, the first one "
                                   "must be a stream of attributes");
    }
    attrType = nl->Second(stream);
    vector = nl->Second(args);
  } else { // three arguments
    if(!Stream<Tuple>::checkType(stream)){
       return listutils::typeError("If 3 arguments are given, the first one "
                                   "must be a stream of tuples");
    }
    ListExpr attrNameList = nl->Second(args);
    if(!listutils::isSymbol(attrNameList)){
      return listutils::typeError("invalid attribute name as 2nd arg");
    }
    string attrName = nl->SymbolValue(attrNameList);
    ListExpr attrList = nl->Second(nl->Second(stream));
    index = listutils::findAttribute(attrList,attrName,attrType);
    if(!index){
      return listutils::typeError("Attribute " + attrName 
                                 + " not part of the tuple in stream");
    }
    vector = nl->Third(args);
  }

  if(!getMemSubType(vector,vector)){
    return listutils::typeError("the last element is not a memory object");
  }
  if(!MemoryVectorObject::checkType(vector)){
    return listutils::typeError("last element is not a memory vector");
  }  
  if(!nl->Equal(attrType, nl->Second(vector))){
    return listutils::typeError("stream type and vector type differ");
  }
  if(nl->HasLength(args,2)){
    return stream;
  } else {
    return nl->ThreeElemList(nl->SymbolAtom(Symbols::APPEND()),
                             nl->OneElemList( nl->IntAtom(index-1)),
                             stream);
  }
}


class insertmvAttrInfo{
  public:
    insertmvAttrInfo(Word& _stream, MemoryVectorObject* _v): 
      stream(_stream), v(_v){
       stream.open();
    }
    ~insertmvAttrInfo(){
      stream.close();
    }

    Attribute* next(){
      Attribute* a = stream.request();
      if(a){
        v->add(a,true);
      }
      return a;
    }

  private:
     Stream<Attribute> stream;
     MemoryVectorObject* v; 
};


template<class T>
int insertmvAttrVMT(Word* args, Word& result, int message,
                    Word& local, Supplier s){

   insertmvAttrInfo* li = (insertmvAttrInfo*) local.addr;
   switch(message){
     case OPEN: {
            if(li){
              delete li;
              local.addr = 0;
            }
            T* vn = (T*) args[1].addr;
            MemoryVectorObject* v = getMVector(vn);
            if(v==0){
             return 0;
            }
            local.addr = new insertmvAttrInfo(args[0],v);
            return 0;
          }
     case REQUEST:
           result.addr = li?li->next():0;
           return result.addr?YIELD:CANCEL;

     case CLOSE:
          if(li){
            delete li;
            local.addr = 0;
          }
          return 0;
   }
   return -1;
}

class insertmvTupleInfo{
  public: 
      insertmvTupleInfo(Word& _stream, MemoryVectorObject* _v, int _index):
         stream(_stream), v(_v), index(_index){
         stream.open();
      }
      ~insertmvTupleInfo(){
        stream.close();
      }

      Tuple* next(){
         Tuple* t = stream.request();
         if(t){
           v->add(t->GetAttribute(index), true);
         }
         return t;
      }

  private:
      Stream<Tuple> stream;
      MemoryVectorObject* v; 
      int index;
};


template<class T>
int insertmvTupleVMT(Word* args, Word& result, int message,
                    Word& local, Supplier s){

   insertmvTupleInfo* li = (insertmvTupleInfo*) local.addr;
   switch(message){
     case OPEN: {
            if(li){
              delete li;
              local.addr = 0;
            }
            T* vn = (T*) args[2].addr;
            MemoryVectorObject* v = getMVector(vn);
            if(v==0){
             return 0;
            }
            int index = ((CcInt*) args[3].addr)->GetValue();
            local.addr = new insertmvTupleInfo(args[0],v,index);
            return 0;
          }
     case REQUEST:
           result.addr = li?li->next():0;
           return result.addr?YIELD:CANCEL;

     case CLOSE:
          if(li){
            delete li;
            local.addr = 0;
          }
          return 0;
   }
   return -1;
}

ValueMapping insertmvVM[] = {
   insertmvAttrVMT<Mem>,
   insertmvAttrVMT<MPointer>,
   insertmvTupleVMT<Mem>,
   insertmvTupleVMT<MPointer>
};

int insertmvSelect(ListExpr args){

  int n1 = -2;
  ListExpr vt;
  if(nl->HasLength(args,2)){
    n1 = 0;
    vt = nl->Second(args);
  } else {
    n1 = 2;
    vt = nl->Third(args);
  }
  if(Mem::checkType(vt)){
    return n1;
  }
  if(MPointer::checkType(vt)){
    return n1+1;
  }
  return -1;
}

OperatorSpec insertmvSpec(
  "stream(A) x mvector -> stream(A) | "
  "stream(tuple(...)) x ID x mvector -> stream(tuple)",
  " _ insertmv[_,_] ",
  "Appends elements from a stream to a main memory vector.",
  " query ten feed insertmv[No, tv] count"
);


Operator insertmvOp(
  "insertmv",
   insertmvSpec.getStr(),
   4,
   insertmvVM,
   insertmvSelect,
   insertmvTM
);


/*
10 MPointer Creation

*/
ListExpr pwrapTM(ListExpr args){
  if(!nl->HasLength(args,1)){
    return listutils::typeError("pwrap needs one argument");
  }
  ListExpr arg = nl->First(args);
  if(!nl->HasLength(arg,2)){
    return listutils::typeError("internal error");
  }
  ListExpr t = nl->First(arg);
  if(  !CcString::checkType(t)
     &&!Mem::checkType(t)){
    return listutils::typeError("string or mem(...) expected");
  } 

  ListExpr motype;
  if(CcString::checkType(t)) {
     ListExpr v = nl->Second(arg);
     if(nl->AtomType(v)!=StringType){
       return listutils::typeError("only constant objects allowed");
     }
     string n = nl->StringValue(v);
     MemoryObject* mo = catalog->getMMObject(n);
     if(mo==0){
        return listutils::typeError("No mm object with name " + n + " found");
     }
     motype = mo->getType();
  } else {
     motype = t;
  }
  ListExpr res = nl->TwoElemList(listutils::basicSymbol<MPointer>(),
                                 motype);

  return res; 
}


template<class T>
int pwrapVMT(Word* args, Word& result, int message,
                Word& local, Supplier s){


  result = qp->ResultStorage(s);
  MPointer* mp = (MPointer*) result.addr;

  if((*mp)()){ // evaluate only once
    return 0;
  }
   
  T* t = (T*) args[0].addr;
  if(!t->IsDefined()){ // keep 0 pointer
    return 0;
  }
  MemoryObject* ptr = catalog->getMMObject(t->GetValue());
  mp->setPointer(ptr);
  return 0;
}

int pwrapSelect(ListExpr args){
  return CcString::checkType(nl->First(args))?0:1;
}

ValueMapping pwrapVM[] = {
  pwrapVMT<CcString>,
  pwrapVMT<Mem>
};

OperatorSpec pwrapSpec(
  "{string, mem} -> mpointer",
  "pwrap(_)",
  "converts a name into a memory pointer",
  "query pwrap(\"mten\")"
);

Operator pwrapOp(
  "pwrap",
  pwrapSpec.getStr(),
  2,
  pwrapVM,
  pwrapSelect,
  pwrapTM
);

/*
11 Variant of matchbelow

*/

ListExpr matchbelow2TM(ListExpr args){

  if(!nl->HasLength(args,5)){
    return listutils::typeError("wrong number of arguments, expected 5");
  }
  ListExpr tree = nl->First(args);
  if(MPointer::checkType(tree)){
     tree = nl->Second(tree);
  }
  if(!Mem::checkType(tree)){
    return listutils::typeError("first arg is not of type mem or mpointer");
  }
  // remove mem from tree description
  tree = nl->Second(tree);
  if(   !MemoryAVLObject::checkType(tree)
     && !MemoryTTreeObject::checkType(tree)){
     return listutils::typeError("first arg must be a main memory "
                                 "avl-tree or t-tree.");
  }

  bool isAVL = MemoryAVLObject::checkType(tree);
  ListExpr treeType = nl->Second(tree);

  ListExpr rel = nl->Second(args);

  if(MPointer::checkType(rel)){
    rel = nl->Second(rel); // remove mpointer
  }
  if(!Mem::checkType(rel)){
    return listutils::typeError("second argument not of type mpointer or mem");
  }
  rel = nl->Second(rel); // remove mem
  
  if(!Relation::checkType(rel)){
     return listutils::typeError("Second arg is not a memory relation");
  }

  ListExpr searchType = nl->Third(args);

  if(!nl->Equal(searchType,treeType)){
     return listutils::typeError("search key type differs from tree type");
  }  

  ListExpr attrName = nl->Fourth(args);
  if(nl->AtomType(attrName)!=SymbolType){
    return listutils::typeError("fourth arg must be an attribute name");
  }
  string aname = nl->SymbolValue(attrName);
  ListExpr attrType;
  ListExpr attrList = nl->Second(nl->Second(rel));
  int index = listutils::findAttribute(attrList,aname,attrType);
  if(!index){
    return listutils::typeError("attribute " + aname 
                                + " not found in relation");
  }
  ListExpr defaultType = nl->Fifth(args);
  if(!nl->Equal(attrType,defaultType)){
    return listutils::typeError("default value type differs "
                                "from attribute type");
  }
  return nl->ThreeElemList(
               nl->SymbolAtom(Symbols::APPEND()),
               nl->TwoElemList(nl->BoolAtom(isAVL),
                               nl->IntAtom(index-1)),
               defaultType
             );
}


template<class T, class R>
int matchbelow2VMT(Word* args, Word& result, int message,
                Word& local, Supplier s){

   result = qp->ResultStorage(s);
   Attribute* res = (Attribute*) result.addr;

   R* relN = (R*) args[1].addr;
   MemoryRelObject* mro = getMemRel(relN);
   if(!mro){
     res->SetDefined(false);
     return 0;      
   }

   vector<Tuple*>* rel = mro->getmmrel();
   size_t size = rel->size();
   T* treeN = (T*) args[0].addr;
   bool isAVL = ((CcBool*) args[5].addr)->GetValue();
   
   Attribute* searchValue = (Attribute*) args[2].addr;
   Attribute* defaultValue = (Attribute*) args[4].addr;
   int attrIndex = ((CcInt*)args[6].addr)->GetValue();
   AttrIdPair const* hit=0; 

   AttrIdPair p(searchValue,size);
   if(isAVL){
      memAVLtree* tree = getAVLtree(treeN);    
      if(!tree){
        res->SetDefined(0);
        return 0;
      }
      hit = tree->GetNearestSmallerOrEqual(p);
   } else {
      MemoryTTreeObject*  tree = getTtree(treeN);
      if(!tree){
        res->SetDefined(0);
        return 0;
      } 
      hit = tree->gettree()->GetNearestSmallerOrEqual(p,0);
   }
   if(!hit){ // tree empty or searchvalue smaller than all entries
      res->CopyFrom(defaultValue);
      return 0;
   }
   TupleId tid = hit->getTid();
   if(tid>size || tid<1){
     res->SetDefined(false);
     return 0;
   } 
   Tuple* rt = rel->at(tid-1);
   if(!rt){ // deleted tuple
     res->SetDefined(false);
     return 0;
   }
   res->CopyFrom(rt->GetAttribute(attrIndex));

   return 0;
}

ValueMapping matchbelow2VM[] = {
  matchbelow2VMT<Mem,Mem>,
  matchbelow2VMT<Mem,MPointer>,
  matchbelow2VMT<MPointer,Mem>,
  matchbelow2VMT<MPointer,MPointer>
};

int matchbelow2Select(ListExpr args){
   ListExpr tree = nl->First(args);
   int n1 =   Mem::checkType(tree)?0 : 2;
   
   ListExpr rel = nl->Second(args);
   int n2 =  Mem::checkType(rel)?0:1;
   return n1+n2;
}

OperatorSpec matchbelow2Spec(
   "{AVLTREE, TTREE} x MREL x T x Ident x V -> V , "
   "AVLTREE, TTREE, MREL in {string, mem, mpointer}",
   "tree rel matchbelow2[searchV, attrName, defaultV]",
   "Retrieves from an avl tree the entry which is "
   "less or equal to searchV. From the returned tuple id, "
   "the tuple in the relation is retrieved and the "
   "attribute with given name is extracted. " 
   "If the avl tree does not returns a hit, the defaultV is "
   "used as the result. If the tuple id is not present in the "
   "relation, the result is undefined. ",
   "query avl rel matchbelow2[ 23, Name, \"Unknown\"] "
);


Operator matchbelow2Op(
  "matchbelow2",
  matchbelow2Spec.getStr(),
  4,
  matchbelow2VM,
  matchbelow2Select,
  matchbelow2TM
);


/*
12 Operator count

*/
ListExpr countTM(ListExpr args){
  if(!nl->HasLength(args,1)){
    return listutils::typeError("one arg expected");
  }
  ListExpr a1 = nl->First(args);
  if(MPointer::checkType(a1)){
    a1 = nl->Second(a1);
  }  
  if(!Mem::checkType(a1)){
    return listutils::typeError("arg not of type mpointer or mem");
  }
  a1 = nl->Second(a1); // remove leading mem
  if(   !Relation::checkType(a1) 
     && !OrderedRelation::checkType(a1)){
     return listutils::typeError("not a memory (o)rel");
  }
  return listutils::basicSymbol<CcInt>();
}


int countVMMem(Word* args, Word& result, int message,
                Word& local, Supplier s){

   Mem* arg = (Mem*)args[0].addr;
   result = qp->ResultStorage(s);
   CcInt* res = (CcInt*) result.addr;
   if(!arg->IsDefined()){
     res->SetDefined(false);
     return 0;
   }
   string name = arg->GetValue();
   if(!catalog->isObject(name) ) {
     res->SetDefined(false);
     return 0;
   }
   ListExpr relType = nl->Second(catalog->getMMObjectTypeExpr(name));
   
   if(Relation::checkType(relType)){
     res->Set(true, ((MemoryRelObject*)catalog->getMMObject(name))->getSize());
   } else if(OrderedRelation::checkType(relType)){
     res->Set(true,((MemoryORelObject*)catalog->getMMObject(name))->getSize());
   } else {
     res->SetDefined(false);
   }
   return 0;
}

template<class RelType>
int countVMMPointer(Word* args, Word& result, int message,
                Word& local, Supplier s){
  MPointer* arg = (MPointer*) args[0].addr;
  RelType* rel = (RelType*) arg->GetValue();
  int c = rel?rel->getSize():0;
  result = qp->ResultStorage(s);
  ((CcInt*) result.addr)->Set(true,c);
  return 0;
}

   
ValueMapping countVM[] = {
  countVMMem,
  countVMMPointer<MemoryRelObject>,
  countVMMPointer<MemoryORelObject>
}; 

int countSelect(ListExpr args){
  ListExpr a1 = nl->First(args);
  if(Mem::checkType(a1)) return 0;
  // an mpointer
  a1 = nl->Second(nl->Second(a1));
  return Relation::checkType(a1)?1:2;
}

OperatorSpec countSpec(
  " {MREL, MOREL} -> int",
  "_ count",
  "returns the size of an (ordered) memory relation",
  " query mten count"
);

Operator countOp(
  "count",
  countSpec.getStr(),
  3,
  countVM,
  countSelect,
  countTM
);



/*
8 Operators on priority queues

8.1 mcreatepqueue

Creates a new priority queue in main  memory.

stream(tuple) x AttrName  -> mpointer(mem(mpqueue(tuple))) 

*/
ListExpr mcreatepqueueTM(ListExpr args){

   if(!nl->HasLength(args,2)){
     return listutils::typeError("two arguments expected");
   }
   if(!Stream<Tuple>::checkType(nl->First(args))){
     return listutils::typeError("first arg is not a tuple stream");
   }
   if(nl->AtomType(nl->Second(args))!=SymbolType){
     return listutils::typeError("second arg is not a valid identifier");
   }
   ListExpr tupleType = nl->Second(nl->First(args));
   ListExpr attrList = nl->Second(tupleType);
   string attrName = nl->SymbolValue(nl->Second(args));
   ListExpr attrType;
   int index = listutils::findAttribute(attrList, attrName, attrType);
   if(!index){
     return listutils::typeError(attrName + " is not a member of the tuple");
   }
   if(!CcReal::checkType(attrType)){
    return listutils::typeError("Attribute " + attrName + " not of type real");
   }
   ListExpr res = MPointer::wrapType(Mem::wrapType(
                    MemoryPQueueObject::wrapType(tupleType)));

   return nl->ThreeElemList( nl->SymbolAtom(Symbols::APPEND()),
                             nl->OneElemList(nl->IntAtom(index-1)),
                             res);
}


template<bool flob>
int mcreatepqueueVM(Word* args, Word& result, int message,
                Word& local, Supplier s){
    result = qp->ResultStorage(s);
    MPointer* mp = (MPointer*) result.addr;
    string dbname = getDBname();
    ListExpr tt = nl->Second(nl->Second(nl->Second(qp->GetType(s)))); 
    ListExpr type = Mem::wrapType(MemoryPQueueObject::wrapType(tt));
    int prioIndex = ((CcInt*) args[2].addr)->GetValue();
    MemoryPQueueObject* q = new MemoryPQueueObject(flob, dbname,
                                               nl->ToString(type));
    Stream<Tuple> stream(args[0]);
    stream.open();
    Tuple* tuple;
    while( (tuple= stream.request())){
      CcReal* prio = (CcReal*) tuple->GetAttribute(prioIndex);
      if(prio->IsDefined()){
        q->push(tuple, prio->GetValue());
      }
      tuple->DeleteIfAllowed();
    }
    stream.close();
    mp->setPointer(q);
    q->deleteIfAllowed();
    return 0;
}


OperatorSpec mcreatepqueueSpec(
   "stream(tuple) x IDENT -> mpointer(mem(pqueue)) ",
   "_ mcreatepqueue[_]",
   "Creates a priority in memory storing the  "
   " tuples of the stream. The priority comes from the "
   "attribute that's name is given in the second argument. "
   "This attribute has to be of type real. " 
   "If this attribute is undefined, the values are not inserted "
   "into the queue but nevertheless part of the output stream.",
   "query strassen extend[L : size(.GeoData)] mcreatepqueue[L] count"

);

Operator mcreatepqueueOp(
  "mcreatepqueue",
  mcreatepqueueSpec.getStr(),
  mcreatepqueueVM<false>,
  Operator::SimpleSelect,
  mcreatepqueueTM
);

Operator mcreatepqueueflobOp(
  "mcreatepqueueflob",
  mcreatepqueueSpec.getStr(),
  mcreatepqueueVM<true>,
  Operator::SimpleSelect,
  mcreatepqueueTM
);



template<class T>
ListExpr sizeTM( ListExpr args){
   if(!nl->HasLength(args,1)){
     return listutils::typeError("one argument expected");
   }
   ListExpr a1;
   if(!getMemSubType(nl->First(args),a1)){
     return listutils::typeError("first arg is not a memory object");
   }
   if(!T::checkType(a1)){
     return listutils::typeError("argument is not of type " + T::BasicType());
   }
   return listutils::basicSymbol<CcInt>();
}


template<class T, class C>
int sizeVMT(Word* args, Word& result, int message,
                Word& local, Supplier s){
  result = qp->ResultStorage(s);
  CcInt* res = (CcInt*) result.addr;
  C* q = getMemObject<C>((T*) args[0].addr);
  if(!q){
    res->SetDefined(false);
  } else {
    res->Set(true, q->size());
  }
  return 0;
}

ValueMapping sizeVM[] = {
   sizeVMT<Mem, MemoryPQueueObject>,
   sizeVMT<MPointer, MemoryPQueueObject>,
};

int sizeSelect(ListExpr args){
  ListExpr a = nl->First(args);
  if(Mem::checkType(a)) return 0;
  if(MPointer::checkType(a)) return 1;
  return -1;
}


OperatorSpec sizeSpec(
  "mpqueue -> int",
  "size(_)",
  "Returns the number of entries in an mpqueue.",
  "query size(strassen_L)"
);

Operator sizeOp(
  "size",
  sizeSpec.getStr(),
  2,
  sizeVM,
  sizeSelect,
  sizeTM<MemoryPQueueObject>
);


/*
Operator mfeedpq

*/
ListExpr mfeedpqTM(ListExpr args){
  if(!nl->HasLength(args,1)){
    return listutils::typeError("one argument expected");
  }
  ListExpr a1;
  if(!getMemSubType(nl->First(args),a1)){
    return listutils::typeError("argument is not a memory object");
  }
  if(!MemoryPQueueObject::checkType(a1)){
    return listutils::typeError("expected mpqueue");
  }
  return nl->TwoElemList(
                listutils::basicSymbol<Stream<Tuple> >(),
                nl->Second(a1));  

}

class mfeedpqInfo{
   public:
      mfeedpqInfo(MemoryPQueueObject* _obj): obj(_obj),minSize(0){}
      mfeedpqInfo(MemoryPQueueObject* _obj,
                  int _minSize): obj(_obj),minSize(_minSize){}

      ~mfeedpqInfo(){}

      Tuple* next(){
          if(obj->empty()){
             return 0;
          }
          pqueueentry e = obj->pop();
          Tuple* t =  e();
          return t;
      }

      Tuple* nextMinSize(){
         if(obj->empty() || obj->size()<=minSize){
           return 0;
         }  
         pqueueentry e = obj->pop();
         Tuple* t =  e();
         return t;
      }


   private:
      MemoryPQueueObject* obj;
      size_t minSize;
};


template<class T>
int mfeedpqVMT(Word* args, Word& result, int message,
                Word& local, Supplier s){

   mfeedpqInfo* li = (mfeedpqInfo*) local.addr;
   switch(message){
     case OPEN:{
            if(li){
                delete li;
                local.addr = 0;
            }
            MemoryPQueueObject* q = getMemoryPQueue((T*) args[0].addr);
            if(q){
              local.addr = new mfeedpqInfo(q);
            }
            return 0;
     }
     case REQUEST:
             result.addr = li?li->next():0;
             return result.addr?YIELD:CANCEL;
     case CLOSE:
             if(li){
               delete li;
               local.addr = 0;
             }
             return 0;
     
 
   }

  return -1;
}

ValueMapping mfeedpqVM[] = {
   mfeedpqVMT<Mem>,
   mfeedpqVMT<MPointer>
};

int mfeedpqSelect(ListExpr args){
  ListExpr a = nl->First(args);
  if(Mem::checkType(a)) return 0;
  if(MPointer::checkType(a)) return 1;
  return -1;
}


OperatorSpec mfeedpqSpec(
  "MPQUEUE -> stream(tuple)",   
  " _ mfeedpq ",
  "Feeds the constent of a priority queue into a tuple stream. "
  "In contrast to a 'normal' feed, the queue is eat up.",
  "query strassen_L mfeedpq count"
);


Operator mfeedpqOp(
  "mfeedpq",
  mfeedpqSpec.getStr(),
  2,
  mfeedpqVM,
  mfeedpqSelect,
  mfeedpqTM
);


/*
Operator mfeedpqSize

*/
ListExpr mfeedpqSizeTM(ListExpr args){
  if(!nl->HasLength(args,2)){
    return listutils::typeError("two arguments expected");
  }
  ListExpr a1;
  if(!getMemSubType(nl->First(args),a1)){
    return listutils::typeError("first arg is not a memory object");
  }
  if(!MemoryPQueueObject::checkType(a1)){
    return listutils::typeError("expected mpqueue");
  }
  if(!CcInt::checkType(nl->Second(args))){
    return listutils::typeError("expected int as second argument");
  }

  return nl->TwoElemList(
                listutils::basicSymbol<Stream<Tuple> >(),
                nl->Second(a1));  

}

template<class T>
int mfeedpqSizeVMT(Word* args, Word& result, int message,
                Word& local, Supplier s){

   mfeedpqInfo* li = (mfeedpqInfo*) local.addr;
   switch(message){
     case OPEN:{
            if(li){
                delete li;
                local.addr = 0;
            }
            MemoryPQueueObject* q = getMemoryPQueue((T*) args[0].addr);
            if(q){
              int s = 0;
              CcInt* S = (CcInt*) args[1].addr;
              if(S->IsDefined()){
                s = S->GetValue();
                if(s<0) s = 0;
              }
              local.addr = new mfeedpqInfo(q,s);
            }
            return 0;
     }
     case REQUEST:
             result.addr = li?li->nextMinSize():0;
             return result.addr?YIELD:CANCEL;
     case CLOSE:
             if(li){
               delete li;
               local.addr = 0;
             }
             return 0;
     
 
   }

  return -1;
}

ValueMapping mfeedpqSizeVM[] = {
   mfeedpqSizeVMT<Mem>,
   mfeedpqSizeVMT<MPointer>
};

int mfeedpqSizeSelect(ListExpr args){
  ListExpr a = nl->First(args);
  if(Mem::checkType(a)) return 0;
  if(MPointer::checkType(a)) return 1;
  return -1;
}


OperatorSpec mfeedpqSizeSpec(
  "MPQUEUE x int -> stream(tuple)",   
  " _ mfeedpq ",
  "Feeds the content of a priority queue into a tuple stream "
  "until the queue is empty or a minimum size of the queue "
  "is reached."
  "In contrast to a 'normal' feed, the queue is eat up.",
  "query strassen_L mfeedpqSize[2] count"
);


Operator mfeedpqSizeOp(
  "mfeedpqSize",
  mfeedpqSizeSpec.getStr(),
  2,
  mfeedpqSizeVM,
  mfeedpqSizeSelect,
  mfeedpqSizeTM
);



/*
Operator mfeedpqAbort

*/
ListExpr mfeedpqAbortTM(ListExpr args){
  if(!nl->HasLength(args,2)){
    return listutils::typeError("one argument expected");
  }
  ListExpr a1;
  if(!getMemSubType(nl->First(args),a1)){
    return listutils::typeError("first argument ist not a memory object");
  }
  if(!MemoryPQueueObject::checkType(a1)){
    return listutils::typeError("expected mpqueue");
  }
  ListExpr max = nl->Second(args);
  if(   !CcReal::checkType(max) 
     && !CcInt::checkType(max)){
    return listutils::typeError("The second argument must be of "
                                "type int or real");
  }
  return nl->TwoElemList(
                listutils::basicSymbol<Stream<Tuple> >(),
                nl->Second(a1));  

}

class mfeedpqAbortInfo{
   public:
      mfeedpqAbortInfo(MemoryPQueueObject* _obj, double _m): obj(_obj), m(_m){}

      ~mfeedpqAbortInfo(){}

      Tuple* next(){
          if(obj->empty()){
             return 0;
          }
          if(obj->top().getPrio()>=m){ // maximum prio reached
            return 0;
          }
          pqueueentry e = obj->pop();
          Tuple* t =  e();
          return t;
      }

   private:
      MemoryPQueueObject* obj;
      double m;
};


template<class T, class M>
int mfeedpqAbortVMT(Word* args, Word& result, int message,
                Word& local, Supplier s){

   mfeedpqAbortInfo* li = (mfeedpqAbortInfo*) local.addr;
   switch(message){
     case OPEN:{
            if(li){
                delete li;
                local.addr = 0;
            }
            MemoryPQueueObject* q = getMemoryPQueue((T*) args[0].addr);
            M* max = (M*) args[1].addr;
            if(!max->IsDefined()){
              return 0;
            }
            if(q){
              local.addr = new mfeedpqAbortInfo(q, max->GetValue());
            }
            return 0;
     }
     case REQUEST:
             result.addr = li?li->next():0;
             return result.addr?YIELD:CANCEL;
     case CLOSE:
             if(li){
               delete li;
               local.addr = 0;
             }
             return 0;
   }
  return -1;
}

ValueMapping mfeedpqAbortVM[] = {
   mfeedpqAbortVMT<Mem,CcInt>,
   mfeedpqAbortVMT<Mem,CcReal>,
   mfeedpqAbortVMT<MPointer,CcInt>,
   mfeedpqAbortVMT<MPointer,CcReal>,
};

int mfeedpqAbortSelect(ListExpr args){
   int n1 = Mem::checkType(nl->First(args))?0:2;
   int n2 = CcInt::checkType(nl->Second(args))?0:1;
   return n1 + n2;
}


OperatorSpec mfeedpqAbortSpec(
  "MPQUEUE x {int,real} -> stream(tuple)",   
  " _ mfeedpqAbort[_] ",
  "Feeds the constent of a priority queue into a tuple stream. "
  "In contrast to a 'normal' feed, the queue is eat up. If "
  "the maximum priority (2nd arg) ist reached, feeding is stopped",
  "query strassen_L mfeedpqAbort[230.0] count"
);


Operator mfeedpqAbortOp(
  "mfeedpqAbort",
  mfeedpqAbortSpec.getStr(),
  4,
  mfeedpqAbortVM,
  mfeedpqAbortSelect,
  mfeedpqAbortTM
);


/*
Operator ~minsertTuplepq~

*/

ListExpr minsertTuplepq(ListExpr args){
   if(!nl->HasLength(args,3) && !nl->HasLength(args,4)){
     return listutils::typeError("3 or 4 arguments expected");
   }
   ListExpr a1;
   if(!getMemSubType(nl->First(args),a1)){
     return listutils::typeError("first argument is not a memory object");
   }
   if(!MemoryPQueueObject::checkType(a1)){
     return listutils::typeError("first arg is not a mpqueue");
   }


   ListExpr tuple = nl->Second(args);
   if(!Tuple::checkType(tuple)){
     return listutils::typeError("second arg is not a tuple");
   }
   if(!nl->Equal(nl->Second(a1), tuple)){
     return listutils::typeError("tuple type does not fit the tuple "
                                 "stored in queue");
   }

   if(!CcReal::checkType(nl->Third(args))){
     return listutils::typeError("third arg is not a real");
   }
   if(nl->HasLength(args,3)){
     return listutils::basicSymbol<CcBool>();
   }
   ListExpr attrName = nl->Fourth(args);
   if(!listutils::isSymbol(attrName)){
     return listutils::typeError("4th arg is not a valid attribute name");
   }
   string an = nl->SymbolValue(attrName);
   ListExpr attrType;
   ListExpr attrList = nl->Second(tuple);
   int index = listutils::findAttribute(attrList,an,attrType);
   if(!index){
      return listutils::typeError("attribute " + an + "not part of the tuple");
   }
   if(!CcReal::checkType(attrType)){
      return listutils::typeError("attribute " + an + " of type real");
   }
   return nl->ThreeElemList(
                 nl->SymbolAtom(Symbols::APPEND()),
                 nl->OneElemList(nl->IntAtom(index-1)),
                 listutils::basicSymbol<CcBool>());
}

template<class T>
int minserttuplepqVMT(Word* args, Word& result, int message,
                Word& local, Supplier s){

   result = qp->ResultStorage(s);
   CcBool* res = (CcBool*) result.addr; 

   MemoryPQueueObject* q = getMemoryPQueue((T*) args[0].addr);
   if(!q){
      res->SetDefined(false);
      return 0;
   }
   Tuple* tuple = (Tuple*) args[1].addr;
   CcReal* prioS = (CcReal*) args[2].addr;
   if(!prioS->IsDefined()){
     res->SetDefined(false);
     return 0;
   }
   double prio = prioS->GetValue();
   if(qp->GetNoSons(s)>4){ // update tuple entry if desired
      int prioIndex = ((CcInt*) args[4].addr)->GetValue();
      ((CcReal*) tuple->GetAttribute(prioIndex))->Set(true,prio);
   }
   q->push(tuple,prio);
   res->Set(true,true);
   return 0;
}


ValueMapping minsertTuplepqVM[] = {
   minserttuplepqVMT<Mem>,
   minserttuplepqVMT<MPointer>
};

int minsertTuplepqSelect(ListExpr args){
  ListExpr a = nl->First(args);
  if(Mem::checkType(a)) return 0;
  if(MPointer::checkType(a)) return 1;
  return -1;
}

OperatorSpec minsertTupleSpec(
  "MPQUEUE x tuple x real [x IDENT] -> bool",
  "minserttuplepq(_,_,_,_)",
  "Inserts a tuple into a memory priority queue with a "
  "given priority. If the optional attribute name is given, "
  "the specified attribute of the tuple is changed to the "
  "priority value before insertion. Of course the attribute "
  "has to be of type real.",
  "query strassen feed extend[L : size(.GeoData)] "
  "extend[ Ok : minserttuplepq(mstrassen_PQ_L, . , .L * 2.0,L)] "
  " count");


Operator minsertTuplepqOp(
   "minserttuplepq",
   minsertTupleSpec.getStr(),
   2,
   minsertTuplepqVM,
   minsertTuplepqSelect,
   minsertTuplepq
);



/*
Operator ~minsertTuplepqProject~

This operator works similar to minsertTuplepq but allowes to
project the incoming tuple to a set of attributes before insertion.

Type Mapping

  queue tuple real [IDENT] <list of IDENT>

*/

ListExpr minserttuplepqprojectTM(ListExpr args){
   if(!nl->HasLength(args,4) && !nl->HasLength(args,5)){
      return listutils::typeError("4 or 5 arguments required");
   }
   ListExpr a1;
   if(!getMemSubType(nl->First(args),a1)){
     return listutils::typeError("first argument is not a memory object");
   }
   if(!MemoryPQueueObject::checkType(a1)){
     return listutils::typeError("first arg is not a mpqueue");
   }

   ListExpr tuple = nl->Second(args);
   if(!Tuple::checkType(tuple)){
     return listutils::typeError("second arg is not a tuple");
   }


   ListExpr prio = nl->Third(args);
   if(!CcReal::checkType(prio)){
     return listutils::typeError("third arg is not a real");
   }

   ListExpr projectList;
   string updateAttr = "";
   ListExpr attrList = nl->Second(tuple);
   int updateIndex = -1;

   if(nl->HasLength(args,4)){
     projectList = nl->Fourth(args);
   } else {
     projectList = nl->Fifth(args);
     ListExpr update = nl->Fourth(args);
     if(!listutils::isSymbol(update)){
       return listutils::typeError("fourth arg is not a valid attribute name");
     }
     updateAttr = nl->SymbolValue(update);
     ListExpr attrType;
     updateIndex = listutils::findAttribute(attrList,updateAttr,attrType);
     if(!updateIndex){
        return listutils::typeError("Attribute name " + updateAttr 
                                    + " not part of the tuple");
     }
     if(!CcReal::checkType(attrType)){
        return listutils::typeError("attribute " + updateAttr 
                                    + " not of type real");
     }
   }

   if(nl->AtomType(projectList)!=NoAtom){
     return listutils::typeError("last argument must be a list "
                                 "of attribute names");
   }
   set<string> project;
   int updatePos = -1;
   ListExpr resAttrList = nl->TheEmptyList();
   ListExpr lastAttrList = nl->TheEmptyList();
   ListExpr projectPos = nl->TheEmptyList();
   ListExpr lastProjectPos=nl->TheEmptyList();
   bool first = true;
   int pos = 0;

   while(!nl->IsEmpty(projectList)){
     ListExpr attr = nl->First(projectList);
     projectList = nl->Rest(projectList);

     if(!listutils::isSymbol(attr)){
       return listutils::typeError("projection list contains invalid "
                                   "attribute name");
     }
     string attrName = nl->SymbolValue(attr);
     if(attrName == updateAttr){
       updatePos = pos;
     }
     pos++;
     if(project.find(attrName)!=project.end()){
       return listutils::typeError(  "attribute name " + attrName 
                                   + " found twice in projection list");
     }

     project.insert(attrName);


     ListExpr attrType;
     int index =  listutils::findAttribute(attrList, attrName, attrType);
     if(!index){
       return listutils::typeError(  "attribute " + attrName 
                                   + " not part of the tuple");
     }

     // attrname is ok
     if(first){
        first = false;
        resAttrList = nl->OneElemList(nl->TwoElemList(attr, attrType));
        lastAttrList = resAttrList;
        projectPos = nl->OneElemList( nl->IntAtom(index-1));
        lastProjectPos = projectPos;
     } else {
        lastAttrList = nl->Append(lastAttrList, 
                                  nl->TwoElemList(attr, attrType));
        lastProjectPos = nl->Append(lastProjectPos, nl->IntAtom(index-1));
     } 

   }


   if( (updateAttr!="") && (updatePos<0)){
     return listutils::typeError("update attribute not part of "
                                 "projection list");
   }

   if(updatePos>=0){
     lastProjectPos = nl->Append(lastProjectPos, nl->IntAtom(updatePos));
   }

   ListExpr queueAttrList = nl->Second(nl->Second(a1));
   if(!nl->Equal(resAttrList,queueAttrList)){
     return listutils::typeError("projected tuples does not fit the "
                                 "queue tuple type");
   }

   ListExpr res =  nl->ThreeElemList(
                 nl->SymbolAtom(Symbols::APPEND()),
                 projectPos,
                 listutils::basicSymbol<CcBool>());

   return res; 

}


template<class T, bool update>
int minserttuplepqprojectVMT(Word* args, Word& result, int message,
                Word& local, Supplier s){

   result = qp->ResultStorage(s);
   CcBool* res = (CcBool*) result.addr; 

   MemoryPQueueObject* q = getMemoryPQueue((T*) args[0].addr);
   if(!q){
      res->SetDefined(false);
      return 0;
   }
   Tuple* tuple = (Tuple*) args[1].addr;
   CcReal* prioS = (CcReal*) args[2].addr;
   if(!prioS->IsDefined()){
     res->SetDefined(false);
     return 0;
   }
   double prio = prioS->GetValue();
   TupleType* tt = q->getTupleType();
   assert(tt);
   
   int min, max, updatePos;
   if(update){
     min = 5;
     max = qp->GetNoSons(s) -1;
     updatePos = ((CcInt*) args[max].addr)->GetValue();
   } else {
     min = 4;
     max = qp->GetNoSons(s);
     updatePos = -1;
   }

   Tuple* insertTuple = new Tuple(tt);
   for(int i=min;i<max;i++){
      int p = ((CcInt*) args[i].addr)->GetValue();
      if(p!=updatePos){
         insertTuple->CopyAttribute(p,tuple,i-min);
      } else {
         insertTuple->PutAttribute(i-min, new CcReal(true,prio));
      }      
   }

   q->push(insertTuple,prio);
   insertTuple->DeleteIfAllowed();
   res->Set(true,true);  
   return 0;
}


ValueMapping minserttuplepqprojectVM[] = {
   minserttuplepqprojectVMT<Mem,false>,
   minserttuplepqprojectVMT<Mem,true>,
   minserttuplepqprojectVMT<MPointer,false>,
   minserttuplepqprojectVMT<MPointer,true>
};

int minserttuplepqprojectSelect(ListExpr args){
  int n1 = Mem::checkType(nl->First(args))?0:2;
  int n2 = nl->HasLength(args,4)?0:1;
  return n1+n2;
}

OperatorSpec minserttuplepqprojectUSpec(
   "mpqueue x tuple x real  x IDENT x <IDENTLIST> -> bool, "
   "mpqueue in {mem, mpointer",
   "minserttuplepqproject(_,_,_,_;list)",
   "projects a tuple to the attributes enumerated in the identifier list "
   "with a specified priority into a main memory priority queue. "
   "The attribute in the tuple specified by the 4th argument, "
   "is updated to the value of the priority before inserting"
   " the projected tuple into the queue.",
   "query strassen feed extend[L : size(.GeoData), K : 1.0] "
   "extend[ Ok : mstrassen_PQ_L "
   "minserttuplepqprojectU[ . , .L * 2.0, L;Name,Typ,GeoData,L]] count"
);

Operator minserttuplepqprojectUOp(
  "minserttuplepqprojectU",
  minserttuplepqprojectUSpec.getStr(),
  4,
  minserttuplepqprojectVM,
  minserttuplepqprojectSelect,
  minserttuplepqprojectTM
);


OperatorSpec minserttuplepqprojectSpec(
   "mpqueue x tuple x real  x <IDENTLIST> -> bool "
   "mpqueue in { mem, mpointer",
   "minserttuplepqproject(_,_,_,_;list)",
   "projects a tuple to the attributes enumerated in the identifier list "
   "with a specified priority into a main memory priority queue. ",
   "query strassen feed extend[L : size(.GeoData), K : 1.0] "
   "extend[ Ok : mstrassen_PQ_L "
   "minserttuplepqproject[ . , .L * 2.0;Name,Typ,GeoData,L]] count"
);

Operator minserttuplepqprojectOp(
  "minserttuplepqproject",
  minserttuplepqprojectSpec.getStr(),
  4,
  minserttuplepqprojectVM,
  minserttuplepqprojectSelect,
  minserttuplepqprojectTM
);




/*
8.8 mpwreorder

*/
template<int numArgs>
ListExpr mpqreorderTM(ListExpr args){

  // mpqueue x fun [x AttrName]
  if(!nl->HasLength(args,numArgs)){
     return listutils::typeError("invalid number of arguments");
  }
  ListExpr q;
  if(!getMemSubType(nl->First(args),q)){
    return listutils::typeError("first arg is not a memory object");
  }
  if(!MemoryPQueueObject::checkType(q)){
    return listutils::typeError("first arg is not a memory priority queue");
  }
  ListExpr tt = nl->Second(q); 
  ListExpr fun = nl->Second(args);
  if(!listutils::isMap<1>(fun)){
    return listutils::typeError("second arg is not an unary function");
  }
  if(!nl->Equal(nl->Second(fun),tt)){
     return listutils::typeError("function argument and tuple type in "
                                 "queue differ");
  }
  if(!CcReal::checkType(nl->Third(fun))){
    return listutils::typeError("function result is not a real");
  }
  if(numArgs==2){
     return listutils::basicSymbol<CcInt>();
  }
  ListExpr an = nl->Third(args);
  if(nl->AtomType(an) != SymbolType){
    return listutils::typeError("third arg is not a valid attribute name ");
  }
  string ann = nl->SymbolValue(an);
  ListExpr attrType;
  int index = listutils::findAttribute(nl->Second(tt),ann,attrType);
  if(!index){
     return  listutils::typeError("attribute " + ann
                                  + " not part of the tuple");
  }
  if(!CcReal::checkType(attrType)){
     return listutils::typeError("attribute " + ann 
                                  + " not of type real");
  }
  return nl->ThreeElemList(
            nl->SymbolAtom(Symbols::APPEND()),
            nl->OneElemList(nl->IntAtom(index-1)),
            listutils::basicSymbol<CcInt>()
         );
}

class mpqreorderInfo{

  public:
     mpqreorderInfo(MemoryPQueueObject* _q, Word& _fun, int _attrPos):
       q(_q), fun(_fun.addr), attrPos(_attrPos)
     {
       arg = qp->Argument(fun);  
     }
 
     int compute(){

        // cout << "Reorder queue entries" << endl;
        // StopWatch w;

        MemoryPQueueObject::queue_t nq;
        int fails = 0;
        while(!q->empty()){
          pqueueentry e = q->pop();
          if(!insertnewentry(e,nq)){
             fails++;
          }
        }
        q->swapQueue(nq);

        //cout << "reordering has taken " << w.diffTimes() << endl;

        return fails;
     }


  private:
     MemoryPQueueObject* q;
     Supplier fun;
     int attrPos;
     ArgVectorPointer arg;
     Word funres;

     bool insertnewentry(pqueueentry& e, MemoryPQueueObject::queue_t& q){
        Tuple* t = e();
        ((*arg)[0]).addr = t;
        qp->Request(fun,funres);
        CcReal* prio = (CcReal*) funres.addr;
        if(!prio->IsDefined()){
           t->DeleteIfAllowed();
           return false;
        }
        double d = prio->GetValue();
        if(attrPos>=0){
          t->PutAttribute(attrPos,new CcReal(true,d));
        }
        q.push(pqueueentry(t,d));
        t->DeleteIfAllowed();
        return true;
     }
};


template<class Q>
int mpqreorderVMT(Word* args, Word& result, int message,
                Word& local, Supplier s){

   MemoryPQueueObject* q = getMemObject<MemoryPQueueObject>((Q*) args[0].addr);
   result = qp->ResultStorage(s);
   CcInt* res = (CcInt*) result.addr;
   if(!q){
      res->SetDefined(false);
      return 0;
   }
   int attrPos = -1;
   if(qp->GetNoSons(s)==4){
     attrPos = ((CcInt*) args[3].addr)->GetValue();
   }
   mpqreorderInfo li(q, args[1], attrPos);
   res->Set(true,li.compute());

   return 0;
}

ValueMapping mpqreorderVM[] = {
   mpqreorderVMT<Mem>,
   mpqreorderVMT<MPointer>
};

int mpqreorderSelect(ListExpr args){
  return Mem::checkType(nl->First(args))?0:1;
}

OperatorSpec mpqreorderSpec(
   "mpqueue x fun -> int",
   " _ mpqreorder [_] ",
   "Changes the priorities of the contents of a memory "
   "priority queue. Returns the number of failed reinsertions, "
   "i.e., entries having an undefined function result.",
   " query mpq mpwreorder[ 1/.Prio] count."
);

Operator mpqreorderOp(
  "mpqreorder",
  mpqreorderSpec.getStr(),
  2,
  mpqreorderVM,
  mpqreorderSelect,
  mpqreorderTM<2>
);


OperatorSpec mpqreorderupdateSpec(
   "mpqueue x fun -> int",
   " _ mpqreorderupdate [_,_] ",
   "Changes the priorities of the contents of a memory "
   "priority queue. Returns the number of failed reinsertions, "
   "i.e., entries having an undefined function result. The "
   "attribute that's name is given as the last argument is "
   "changed to the new priority value.",
   " query mpq mpwreorder[ 1/.Prio] count."
);

Operator mpqreorderupdateOp(
  "mpqreorderupdate",
  mpqreorderupdateSpec.getStr(),
  2,
  mpqreorderVM,
  mpqreorderSelect,
  mpqreorderTM<3>
);




/*
9 Operators on Memory Stacks

9.1 mcreatestack

9.1.1 Type Mapping

Stream<Tuple> -> mpointer(mem(mstack(Tuple))) 

*/
ListExpr mcreatestackTM(ListExpr args){
  if(!nl->HasLength(args,1)){
    return listutils::typeError("two arguments expected");
  }
  if(!Stream<Tuple>::checkType(nl->First(args))){
    return listutils::typeError("stream(tuple) expected");
  }
  return MPointer::wrapType( Mem::wrapType( 
            MemoryStackObject::wrapType(nl->Second(nl->First(args)))));
}


template<bool flob>
int mcreatestackVM(Word* args, Word& result, int message,
                Word& local, Supplier s){

   result = qp->ResultStorage(s);
   MPointer* mp = (MPointer*) result.addr;
   string dbname = getDBname(); 
   ListExpr mtype = nl->Second(qp->GetType(s));
   MemoryStackObject* stack = new MemoryStackObject(flob,
                                   dbname,nl->ToString(mtype));
   Stream<Tuple> stream(args[0]);
   stream.open();
   Tuple* tuple;
   while((tuple=stream.request())){
       stack->push(tuple);
       tuple->DeleteIfAllowed();
   }
   stream.close();
   mp->setPointer(stack);
   stack->deleteIfAllowed();
   return 0;
}

OperatorSpec mcreatestackSpec(
  "stream<Tuple> -> mpointer(mem(mstack(Tuple)))",
  " _ mcreatestack",
  "Inserts incoming tuples into a newly created "
  "memory stack.",
  "query plz feed mcreatestack"
);

Operator mcreatestackOp(
  "mcreatestack",
  mcreatestackSpec.getStr(),
  mcreatestackVM<false>,
  Operator::SimpleSelect,
  mcreatestackTM
);


Operator mcreatestackflobOp(
  "mcreatestackflob",
  mcreatestackSpec.getStr(),
  mcreatestackVM<true>,
  Operator::SimpleSelect,
  mcreatestackTM
);



/*
9.2 Operator mfeedstack

*/
ListExpr mfeedstackTM(ListExpr args){
  if(!nl->HasLength(args,1)){
    return listutils::typeError("one argument expected");
  }
  ListExpr a1;
  if(!getMemSubType(nl->First(args),a1)){
    return listutils::typeError("first argument is not a memory object");
  }
  if(!MemoryStackObject::checkType(a1)){
    return listutils::typeError("expected mstack");
  }
  return nl->TwoElemList(
                listutils::basicSymbol<Stream<Tuple> >(),
                nl->Second(a1));  

}

class mfeedstackInfo{
   public:
      mfeedstackInfo(MemoryStackObject* _obj): obj(_obj){}

      ~mfeedstackInfo(){}

      Tuple* next(){
          if(obj->empty()){
             return 0;
          }
          Tuple* t =  obj->pop();
          return t;
      }

   private:
      MemoryStackObject* obj;
};


template<class T>
int mfeedstackVMT(Word* args, Word& result, int message,
                Word& local, Supplier s){

   mfeedstackInfo* li = (mfeedstackInfo*) local.addr;
   switch(message){
     case OPEN:{
            if(li){
                delete li;
                local.addr = 0;
            }
            MemoryStackObject* q = getMemoryStack((T*) args[0].addr);
            if(q){
              local.addr = new mfeedstackInfo(q);
            }
            return 0;
     }
     case REQUEST:
             result.addr = li?li->next():0;
             return result.addr?YIELD:CANCEL;
     case CLOSE:
             if(li){
               delete li;
               local.addr = 0;
             }
             return 0;
     
 
   }

  return -1;
}

ValueMapping mfeedstackVM[] = {
   mfeedstackVMT<Mem>,
   mfeedstackVMT<MPointer>
};

int mfeedstackSelect(ListExpr args){
  ListExpr a = nl->First(args);
  if(Mem::checkType(a)) return 0;
  if(MPointer::checkType(a)) return 1;
  return -1;
}


OperatorSpec mfeedstackSpec(
  "MSTACK -> stream(tuple)",   
  " _ mfeedstack ",
  "Feeds the content of a memory stack into a tuple stream. "
  "In contrast to a 'normal' feed, the stack is eat up.",
  "query plzstack mfeedstack count"
);


Operator mfeedstackOp(
  "mfeedstack",
  mfeedstackSpec.getStr(),
  2,
  mfeedstackVM,
  mfeedstackSelect,
  mfeedstackTM
);


/*
9.3 Operator ~stacksize~

We have to use a special name because in the selection
function there is no chance to get the type. Fortunately,
a lot of functions are implemented as templates.

*/


OperatorSpec stacksizeSpec(
  "MSTACK -> int",
  "stacksize(_)",
  "Retrieves the size of a memeory stack",
  "query stacksize(plzstack)"
);

ValueMapping stacksizeVM[] = {
  sizeVMT<Mem, MemoryStackObject>,
  sizeVMT<MPointer, MemoryStackObject>,
};

int stacksizeSelect(ListExpr args){
  return Mem::checkType(nl->First(args))?0:1;
}


Operator stacksizeOp(
  "stacksize",
   stacksizeSpec.getStr(),
   2,
   stacksizeVM,
   stacksizeSelect,
   sizeTM<MemoryStackObject>
);


/*
9.4 insertmstack

Inserts a tuple stream into an existing memory stack

*/

ListExpr insertmstackTM(ListExpr args){
  if(!nl->HasLength(args,2)){
    return listutils::typeError("two arguments expected");
  }
  ListExpr ts = nl->First(args);
  if(!Stream<Tuple>::checkType(ts)){
    return listutils::typeError("first argument is not a tuple stream");
  }
  ListExpr s;
  if(!getMemSubType(nl->Second(args),s)){
    return listutils::typeError("second arg is not a memory object");
  }
  if(!MemoryStackObject::checkType(s)){
    return listutils::typeError("second arg is not a memory stack");
  }
  ListExpr st = nl->Second(s);
  if(!nl->Equal(st, nl->Second(ts))){
    return listutils::typeError("tuple type in stack and stream differ");
  }
  return ts; 
}

class insertmstackInfo{

 public:
    insertmstackInfo(Word& _stream, MemoryStackObject* _s):
      stream(_stream), s(_s){
      stream.open();
    }

    ~insertmstackInfo(){
       stream.close();
     }

     Tuple* next(){
        Tuple* t = stream.request();
        if(t){
            s->push(t);
        }
        return t;
     }

 private:
   Stream<Tuple> stream;
   MemoryStackObject* s;
};



template<class T>
int insertmstackVMT(Word* args, Word& result, int message,
                    Word& local, Supplier s){

  insertmstackInfo* li = (insertmstackInfo*) local.addr;
  switch(message){
    case OPEN: {
        if(li){
          delete li;
          local.addr = 0;
        }
        MemoryStackObject* o = getMemObject<MemoryStackObject>((T*) 
                                  args[1].addr);
        if(o){
          local.addr = new insertmstackInfo(args[0],o);
        }
        return 0;
    }
    case REQUEST:{
        result.addr = li?li->next():0;
        return result.addr?YIELD:CANCEL;
    }
    case CLOSE:{
        if(li){
           delete li;
           local.addr = 0;
        }
        return 0;
    }
  }
  return -1;  
}

ValueMapping insertmstackVM[] = {
  insertmstackVMT<Mem>,
  insertmstackVMT<MPointer>
};

int insertmstackSelect(ListExpr args){
  return Mem::checkType(nl->Second(args))?0:1;
}

OperatorSpec insertmstackSpec(
  "stream(tuple) x MSTACK -> stream(tuple)",
  "_ insertmstack[_]",
  "Inserts incoming tuples into a memory stack.",
  "query plz feed insertmstack[plzstack1] count"
);

Operator insertmstackOp(
  "insertmstack",
  insertmstackSpec.getStr(),
  2,
  insertmstackVM,
  insertmstackSelect,
  insertmstackTM
);



/*
Operator ~mblock~

*/

ListExpr mblockTM(ListExpr args){
  if(!nl->HasLength(args,1)){
    return listutils::typeError("one argument expected");
  }
  ListExpr a = nl->First(args);
  if(!Stream<Tuple>::checkType(a)
     &&!Stream<Attribute>::checkType(a)){
   return listutils::typeError("strean(tuple) or stream(DATA) expected");
  }
  return a;
}

template<class E>
class mblockInfo{
   public:
      mblockInfo(Word& _stream): stream(_stream){
         collect();
      }

      ~mblockInfo(){
          for(size_t i=pos;i<v.size();i++){
              v[i]->DeleteIfAllowed();
          }
       }

       E* next(){
          if(pos>=v.size()){
             return 0;
          }
          E* res = v[pos];
          v[pos] = 0;
          pos++;
          return res;
       }

    private:
        Stream<E> stream;
        vector<E*> v;
        size_t pos;

      void collect(){
         stream.open();
         E* elem;
         while((elem=stream.request())){
            v.push_back(elem);
         }
         stream.close();
         pos = 0;
      }
};



template<class T>
int mblockVMT(Word* args, Word& result, int message,
              Word& local, Supplier s){
    mblockInfo<T>* li = (mblockInfo<T>*) local.addr;
    switch(message){
       case OPEN:
              if(li) delete li;
              local.addr = new mblockInfo<T>(args[0]);
              return 0;
       case REQUEST:
              result.addr = li?li->next():0;
              return result.addr?YIELD:CANCEL;
       case CLOSE:
               if(li){
                 delete li;
                 local.addr = 0;
               }
    }
    return -1;
}

ValueMapping mblockVM[] = {
   mblockVMT<Tuple>,
   mblockVMT<Attribute>
};

int mblockSelect(ListExpr args){
  return Stream<Attribute>::checkType(nl->First(args))?1:0;
}

OperatorSpec mblockSpec(
  "stream(X) -> stream(X), X in tuple, DATA",
  "_ mblock",
  "Collects the complete stream within a single "
  "step before feeding the output stream. ",
  "query ten feed mblock head[3] count"
);

Operator mblockOp(
  "mblock",
  mblockSpec.getStr(),
  2,
  mblockVM,
  mblockSelect,
  mblockTM
);


/*
10 Operators on mgraph2


*/



/*

10.1 Creation of the graph

10.1.1 Type mapping

stream(tuple) x attrname x attrname x fun  

edges           source     target     costs  

*/
ListExpr createmgraph2TM(ListExpr args){
  if(!nl->HasLength(args,4)){
    return listutils::typeError("five arguments expected");
  }
  // first arg: tuple stream
  ListExpr ts = nl->First(args);
  if(!Stream<Tuple>::checkType(ts)){
    return listutils::typeError("first argument is not a tuple stream");
  }
  // second arg: attribute name of source
  ListExpr source_a = nl->Second(args);
  if(nl->AtomType(source_a)!=SymbolType){
    return listutils::typeError("second arg is not a valid symbol value");
  }
  string source_n = nl->SymbolValue(source_a);
  ListExpr source_t;
  ListExpr attrList = nl->Second(nl->Second(ts));
  int source_index = listutils::findAttribute(attrList,source_n,source_t);
  if(!source_index){
    return listutils::typeError("Attribute " + source_n 
                                 + " not found in tuple");
  }
  // check for valid node type, (int or longint)
  if(   !CcInt::checkType(source_t) 
     && !LongInt::checkType(source_t)){
    return listutils::typeError("Invalid source type required int or longint");
  }
  // third arg : attribute name of target
  ListExpr target_a = nl->Third(args);
  if(nl->AtomType(target_a)!=SymbolType){
    return listutils::typeError("third arg is not a valid symbol value");
  }
  string target_n = nl->SymbolValue(target_a);
  ListExpr target_t;
  int target_index = listutils::findAttribute(attrList,target_n,target_t);
  if(!target_index){
    return listutils::typeError("Attribute " + target_n 
                                + " not found in tuple");
  }
  if(!nl->Equal(source_t, target_t)){
    return listutils::typeError("types of source and target differ");
  }
  // fourth arg: cost function
  ListExpr costfun = nl->Fourth(args);
  if(!listutils::isMap<1>(costfun)){
    return listutils::typeError("fourth arg is not an unary function");
  }
  if(!nl->Equal(nl->Second(costfun), nl->Second(ts))){
    return listutils::typeError("tuple type and function argument of "
                                "cost function differ");
  }
  if(!CcReal::checkType(nl->Third(costfun))){
    return listutils::typeError("result of cost function is not a real");
  }

  ListExpr newAttr = nl->ThreeElemList(
    nl->TwoElemList(
       nl->SymbolAtom("MG_Source"),
       listutils::basicSymbol<CcInt>()),
    nl->TwoElemList(
       nl->SymbolAtom("MG_Target"),
       listutils::basicSymbol<CcInt>()),
    nl->TwoElemList(
       nl->SymbolAtom("MG_Cost"),
       listutils::basicSymbol<CcReal>())
  );

  attrList = listutils::concat(attrList, newAttr);
  ListExpr tt = nl->TwoElemList(
                     listutils::basicSymbol<Tuple>(),
                     attrList);
  if(!Tuple::checkType(tt)){
    return listutils::typeError("attribute MG_source, MG_Target, "
                                "or MG_Cost already present in tuple");
  }
  ListExpr resType = MPointer::wrapType(Mem::wrapType(
                        MGraph2::wrapType(tt)));
  return nl->ThreeElemList(
              nl->SymbolAtom(Symbols::APPEND()),
              nl->TwoElemList(
                     nl->IntAtom(source_index-1),
                     nl->IntAtom(target_index-1)),
              resType);
}


 // T : type of node (int, longint)
template<class T, bool flob>
int createmgraph2VMT(Word* args, Word& result, int message,
              Word& local, Supplier s){

  result = qp->ResultStorage(s);
  MPointer* mp = (MPointer*) result.addr;
  
  ListExpr mtype  = nl->Second(qp->GetType(s));
  string type = nl->ToString(mtype);
  string dbname = getDBname(); 
  MGraph2* graph = new MGraph2(flob, dbname,type);
  int sourcePos = ((CcInt*) args[4].addr)->GetValue();
  int targetPos = ((CcInt*) args[5].addr)->GetValue();
  Word costFun = args[3];
  ArgVectorPointer costArg = qp->Argument(costFun.addr);
  size_t lostTuples = 0;
  Stream<Tuple> stream(args[0]);
  stream.open();
  Tuple* orig;
  Word value;
  while( (orig= stream.request())){
     T* Source = (T*) orig->GetAttribute(sourcePos);
     T* Target = (T*) orig->GetAttribute(targetPos);
     (*costArg)[0] = orig;
     qp->Request(costFun.addr,value);
     CcReal* Costs = (CcReal*) value.addr;
     if(Source->IsDefined() && Target->IsDefined() && Costs->IsDefined()){
        Tuple* nt = graph->insertOrigEdge<T>(orig,Source->GetValue(), 
                                             Target->GetValue(), 
                                             Costs->GetValue());
        nt->DeleteIfAllowed();
     } else {
        lostTuples++;
     }
     orig->DeleteIfAllowed();
  }
  stream.close();
  if(lostTuples>0){
    cerr << " lost " << lostTuples << " during creation of an mgraph2" << endl;
  }
  mp->setPointer(graph);
  graph->deleteIfAllowed();
  return 0;
}


ValueMapping createmgraph2VM[] ={
   createmgraph2VMT<CcInt,false>,
   createmgraph2VMT<LongInt,false>
};

ValueMapping createmgraph2flobVM[] ={
   createmgraph2VMT<CcInt,true>,
   createmgraph2VMT<LongInt,true>
};

int createmgraph2Select(ListExpr args){
  ListExpr nodeType;
  listutils::findAttribute( nl->Second(nl->Second(nl->First(args))),
                            nl->SymbolValue(nl->Second(args)),
                            nodeType);
  return CcInt::checkType(nodeType)?0:1;
}

OperatorSpec createmgraph2Spec(
  "stream(tuple) x IDENT x INDENT x fun -> mpointer(mem(mgraph2(tuple@..)))",
  " edges createmgraph2[SourceAttr, TargetAttr, CostFun]",
  "Creates an mgraph2 from an stream of edges. Edges with undefined "
  "source, target or costs are ignored. " 
  "All other edges are inserted into the graph and extended by "
  "new source, target, and a cost attribute.",
  "query otestrel feed createmgraph2[S1_id, S2_id, "
  "distance(.S1_Pos, .S2_Pos)] "
);

Operator createmgraph2Op(
  "createmgraph2",
  createmgraph2Spec.getStr(),
  2,
  createmgraph2VM,
  createmgraph2Select,
  createmgraph2TM  
);

Operator createmgraph2flobOp(
  "createmgraph2flob",
  createmgraph2Spec.getStr(),
  2,
  createmgraph2flobVM,
  createmgraph2Select,
  createmgraph2TM  
);


/*
10.2 mg2insertorig

This operator inserts new edges into an existing graph
in the same way as in the graph creation. The only difference
is that the graph has to exist with the correct tuple type.
Note that as in the creation double edges are allowed.

*/

ListExpr mg2insertorigTM(ListExpr args){
  if(!nl->HasLength(args,5)){
    return listutils::typeError("five arguments expected");
  }
  // first arg: tuple stream
  ListExpr ts = nl->First(args);
  if(!Stream<Tuple>::checkType(ts)){
    return listutils::typeError("first argument is not a tuple stream");
  }
  // second arg: attribute name of source
  ListExpr source_a = nl->Second(args);
  if(nl->AtomType(source_a)!=SymbolType){
    return listutils::typeError("second arg is not a valid symbol value");
  }
  string source_n = nl->SymbolValue(source_a);
  ListExpr source_t;
  ListExpr attrList = nl->Second(nl->Second(ts));
  int source_index = listutils::findAttribute(attrList,source_n,source_t);
  if(!source_index){
    return listutils::typeError("Attribute " + source_n 
                                 + " not found in tuple");
  }
  // check for valid node type, (int or longint)
  if(   !CcInt::checkType(source_t) 
     && !LongInt::checkType(source_t)){
    return listutils::typeError("Invalid source type required int or longint");
  }
  // third arg : attribute name of target
  ListExpr target_a = nl->Third(args);
  if(nl->AtomType(target_a)!=SymbolType){
    return listutils::typeError("third arg is not a valid symbol value");
  }
  string target_n = nl->SymbolValue(target_a);
  ListExpr target_t;
  int target_index = listutils::findAttribute(attrList,target_n,target_t);
  if(!target_index){
    return listutils::typeError("Attribute " + target_n 
                                + " not found in tuple");
  }
  if(!nl->Equal(source_t, target_t)){
    return listutils::typeError("types ofd source and target differ");
  }

  // fourth arg: cost function
  ListExpr costfun = nl->Fourth(args);
  if(!listutils::isMap<1>(costfun)){
    return listutils::typeError("fourth arg is not an unary function");
  }
  if(!nl->Equal(nl->Second(costfun), nl->Second(ts))){
    return listutils::typeError("tuple type and function argument of "
                                "cost function differ");
  }
  if(!CcReal::checkType(nl->Third(costfun))){
    return listutils::typeError("result of cost function is not a real");
  }

  // fifth argument: the graph
  ListExpr graph;
  if(!getMemSubType(nl->Fifth(args),graph)){
    return listutils::typeError("5th argument is not a memory object");
  }
  if(!MGraph2::checkType(graph)){
    return listutils::typeError("5th arg is not an mgraph2");
  }
  ListExpr sAttrList = nl->Second(nl->Second(ts));
  ListExpr gAttrList = nl->Second(nl->Second(graph));
  if(nl->ListLength(sAttrList)+3 != nl->ListLength(gAttrList)){
    return listutils::typeError("invalid number of attributes in tuple");
  }
  while(!nl->IsEmpty(sAttrList)){
     ListExpr sf = nl->First(sAttrList);
     ListExpr gf = nl->First(gAttrList);
     sAttrList = nl->Rest(sAttrList);
     gAttrList = nl->Rest(gAttrList);
     if(!nl->Equal(sf,gf)){
       return listutils::typeError("invalid tuple type (differs to graph)");
     }
  }

  ListExpr gTuple = nl->Second(graph);

  return nl->ThreeElemList(
              nl->SymbolAtom(Symbols::APPEND()),
              nl->TwoElemList(
                     nl->IntAtom(source_index-1),
                     nl->IntAtom(target_index-1)),
              nl->TwoElemList(
                 listutils::basicSymbol<Stream<Tuple> >(),
                 gTuple)   
              );
}

template<class T>
class insertmgraph2Info{

  public:
     insertmgraph2Info( Word& _stream, MGraph2* _graph,int _sourcePos, 
       int _targetPos, Word& _costFun):
       stream(_stream), graph(_graph), sourcePos(_sourcePos),
       targetPos(_targetPos), costFun(_costFun.addr){
       stream.open();
       costArg = qp->Argument(costFun);
       lost = 0; 
     }
     ~insertmgraph2Info(){
       if(lost>0){
          cout << "Lost : " << lost 
               << " edges during inserting edges into a graph" << endl;
       }
       stream.close();
     }

     Tuple* next(){
       Tuple* orig;

       while( (orig = stream.request())) {
          T* Source = (T*) orig->GetAttribute(sourcePos);
          T* Target = (T*) orig->GetAttribute(targetPos);
          (*costArg)[0] = orig;
          qp->Request(costFun,value);
          CcReal* Costs = (CcReal*) value.addr;
          if(Source->IsDefined() && Target->IsDefined() && Costs->IsDefined()){
            Tuple* res = graph->insertOrigEdge<T>(orig,Source->GetValue(), 
                                               Target->GetValue(), 
                                               Costs->GetValue());
            orig->DeleteIfAllowed();
            return res;
          }
          // ignore nonsense tuples
          lost++;
          orig->DeleteIfAllowed();
       }
       return 0;
     }
  private:
    Stream<Tuple> stream;
    MGraph2* graph;
    int sourcePos;
    int targetPos;
    Supplier costFun;
    ArgVectorPointer costArg;
    Word value;
    size_t lost;
};



template<class T, class G>
int mg2insertorigVMT(Word* args, Word& result, int message,
              Word& local, Supplier s){

  insertmgraph2Info<T>* li = (insertmgraph2Info<T>*)local.addr;

  switch(message){

     case OPEN:{
       if(li){
          delete li;
          local.addr = 0;
       }
       MGraph2* graph = getMemObject<MGraph2>((G*) args[4].addr); 
       if(!graph){
         return 0;
       }
       local.addr = new insertmgraph2Info<T>(
                          args[0], graph,
                          ((CcInt*) args[5].addr)->GetValue(),
                          ((CcInt*) args[6].addr)->GetValue(),
                          args[3]);
        return 0;
     }
     case REQUEST:{
          result.addr = li?li->next():0;
          return result.addr?YIELD:CANCEL;
     }                     
     case CLOSE: {
         if(li){
           delete li;
           local.addr = 0;
         }
         return 0;
     }
  }
  return -1; 
}

ValueMapping mg2insertorigVM[] = {
  mg2insertorigVMT<CcInt,Mem>,
  mg2insertorigVMT<CcInt,MPointer>,
  mg2insertorigVMT<LongInt,Mem>,
  mg2insertorigVMT<LongInt,MPointer>
};

int mg2insertorigSelect(ListExpr args){
  ListExpr nodeType;
  listutils::findAttribute( nl->Second(nl->Second(nl->First(args))),
                            nl->SymbolValue(nl->Second(args)),
                            nodeType);
  int n1 =  CcInt::checkType(nodeType)?0:2;
  int n2 = Mem::checkType(nl->Fifth(args))?0:1;
  return n1 + n2; 
}

OperatorSpec mg2insertorigSpec(
  "stream(tuple) x IDENT x INDENT x fun x MGRAPH2 -> stream(tuple)",
  " edges mg2insertorig[SourceName, TargetName, CostFun, Graph] ",
  "Inserts new edges into an existing graph using the original "
  "tuple representation that was used during the graph creation.",
  " query edges feed mg2insertgraph[Source, Target, .Cost, mg2] count"
);

Operator mg2insertorigOp(
  "mg2insertorig",
  mg2insertorigSpec.getStr(),
  4,
  mg2insertorigVM,
  mg2insertorigSelect,
  mg2insertorigTM
);

/*
10.3 Operator mg2insert

*/
template<class G>
ListExpr mginsertTM(ListExpr args){
   if(!nl->HasLength(args,2)){
     return listutils::typeError("two arguments expected");
   }
   ListExpr ts = nl->First(args);
   if(!Stream<Tuple>::checkType(ts)){
     return listutils::typeError("first arg is not a tuple stream");
   }
   ListExpr graph;
   if(!getMemSubType(nl->Second(args),graph)){
     return listutils::typeError("second argument is not a memory object");
   }
   if(!G::checkType(graph)){
     return listutils::typeError("second argument is not an " +
                                 G::BasicType());
   }
   if(!nl->Equal(nl->Second(ts), nl->Second(graph))){
     return listutils::typeError("tuple types of stream and graph differ");
   }
   return ts;
}

template<class G>
class mginsertInfo{
  public:
    mginsertInfo(Word& _stream, G* _graph): stream(_stream), graph(_graph){
       stream.open();
    }

    ~mginsertInfo(){
      stream.close();
    }

    Tuple* next(){
       Tuple* t;
       while( (t = stream.request())){
         if(!graph->insertGraphEdge(t)){
           t->DeleteIfAllowed();
         } else {
           return t;
         }
       }
       return 0;
    }


  private:
    Stream<Tuple> stream;
    G* graph; 
};

template<class GN, class Graph>
int mginsertVMT(Word* args, Word& result, int message,
                 Word& local, Supplier s){
  mginsertInfo<Graph>* li = (mginsertInfo<Graph>*) local.addr;
  switch(message){
    case OPEN: {
       if(li) {
         delete li;
         local.addr =0;
       }
       Graph* g = getMemObject<Graph>((GN*)args[1].addr);
       if(!g) return 0;
       local.addr = new mginsertInfo<Graph>(args[0],g);
       return 0;
    }
    case REQUEST:
       result.addr = li?li->next():0;
       return result.addr?YIELD:CANCEL;
    case CLOSE:
        if(li){
          delete li;
          local.addr = 0;
        }
        return 0;
  } 
  return -1;
}

ValueMapping mg2insertVM[] = {
   mginsertVMT<Mem, MGraph2>,
   mginsertVMT<MPointer, MGraph2>
};

int mginsertSelect(ListExpr args){
  return Mem::checkType(nl->Second(args))?0:1;
}

OperatorSpec mg2insertSpec(
  "stream<tuple> x MGRAPH2 -> stream(tuple)",
  "_ mg2insert[_]",
  "Inserts graph tuples into a graph."
  "Tuples with invalid values are ignored and are "
  "not passed to the output stream. "
  "Invalid tuples may have source or target node "
  "outside the graph or negative costs.",
  "query newEdges feed mginsert[mg2] count"
);


Operator mg2insertOp(
  "mg2insert",
  mg2insertSpec.getStr(),
  2,
  mg2insertVM,
  mginsertSelect,
  mginsertTM<MGraph2>
);

/*
10.5 mgfeed

*/
template<class G>
ListExpr mgfeedTM(ListExpr args){
  if(!nl->HasLength(args,1)){
    return listutils::typeError("one argument expected");
  }
  ListExpr g = nl->First(args);
  string err;
  if(MPointer::checkType(g)){
    g = nl->Second(g);
  }
  if(!Mem::checkType(g)){
    return listutils::typeError("first arg must be of type mpointer or mem");
  }
  g = nl->Second(g);
  if(!G::checkType(g)){
    return listutils::typeError("argument is not a memory graph");
  }
  return nl->TwoElemList(
             listutils::basicSymbol<Stream<Tuple> >(),
             nl->Second(g));
  
}


template<class GN, class Graph>
int mg2feedVMT(Word* args, Word& result, int message,
                 Word& local, Supplier s){

  typedef typename Graph::edgeIterator iterator;
  iterator* li = (iterator*) local.addr;
  switch(message){
    case OPEN: {
      if(li){
         delete li;
         local.addr = 0;
      }
      Graph* g = getMemObject<Graph>((GN*) args[0].addr);
      if(!g) return 0;
      local.addr = g->getEdgeIt();
      return 0;
    }
    case REQUEST:
      result.addr = li?li->next():0;
      return result.addr?YIELD:CANCEL;
    case CLOSE:
      if(li){
         delete li;
         local.addr = 0;
      }
      return 0;
  }
  return -1; 
}

ValueMapping mg2feedVM[] = {
   mg2feedVMT<Mem, MGraph2>,
   mg2feedVMT<MPointer, MGraph2>
};

int mgfeedSelect(ListExpr args){
 return Mem::checkType(nl->First(args))?0:1;
}

OperatorSpec mg2feedSpec(
  "MGRAPH -> stream(tuple)",
  " _ mg2feed",
  "Extract the edges from a mgraph2 object",
  "query mg2 mgfeed count"
);


Operator mg2feedOp(
  "mg2feed",
  mg2feedSpec.getStr(),
  2,
  mg2feedVM,
  mgfeedSelect,
  mgfeedTM<MGraph2>
);


/*
10.5 Operator mg2nodemap

*/
ListExpr mg2nodemapTM(ListExpr args){
  if(!nl->HasLength(args,2)){
    return listutils::typeError("two arguments required");
  }
  ListExpr g;
  if(!getMemSubType(nl->First(args),g)){
    return listutils::typeError("first arg is not a memory object");
  }
  if(!MGraph2::checkType(g)){
    return listutils::typeError("argument is not a memory graph");
  }
  // check second argument
  ListExpr node = nl->Second(args);
  if(!CcInt::checkType(node) && !LongInt::checkType(node)){
    return listutils::typeError("second arg has to be of type int or longint");
  }
  return listutils::basicSymbol<CcInt>();
}


template<class G, class N>
int mg2nodemapVMT(Word* args, Word& result, int message,
                  Word& local, Supplier s){
  result = qp->ResultStorage(s);
  CcInt*  res = (CcInt*) result.addr;
  MGraph2* g = getMemObject<MGraph2>((G*) args[0].addr);
  if(!g){
     res->SetDefined(false);
     return 0;
  }
  N* v = (N*) args[1].addr;
  if(!v->IsDefined()){
     res->SetDefined(false);
     return 0;
  }

  int r = g->mapNode(v->GetValue());
  if(r<0){
     res->SetDefined(false);
     return 0;
  }
  res->Set(true,r);
  return 0;
}

ValueMapping mg2nodemapVM[] = {
  mg2nodemapVMT<Mem,CcInt>,
  mg2nodemapVMT<Mem,LongInt>,
  mg2nodemapVMT<MPointer,CcInt>,
  mg2nodemapVMT<MPointer,LongInt>
};

int mg2nodemapSelect(ListExpr args){
  int n1 = Mem::checkType(nl->First(args))?0:2;
  int n2 = CcInt::checkType(nl->Second(args))?0:1;
  return n1+n2;
}

OperatorSpec mg2nodemapSpec(
  "MGRAPH2 x {int, longint}  -> int",
  "_ mg2nodemap[_]",
  "Returns the internal node number for a node having "
  "a nodenumber corresponding to the argument.",
  "query mg2 nodemap[40]"
);

Operator mg2nodemapOp(
  "mg2nodemap",
  mg2nodemapSpec.getStr(),
  4,
  mg2nodemapVM,
  mg2nodemapSelect,
  mg2nodemapTM
);

/*
10.6 Operator mg2numvertices

*/
template<class Graph>
ListExpr mgnumverticesTM(ListExpr args){
  if(!nl->HasLength(args,1)){
    return listutils::typeError("one argument expected");
  }
  ListExpr g;
  if(!getMemSubType(nl->First(args),g)){
    return listutils::typeError("argument is not a memory object");
  }
  if(!Graph::checkType(g)){
    return listutils::typeError("argument is not a" + Graph::BasicType());
  }
  return listutils::basicSymbol<CcInt>();
}

template<class GN, class Graph>
int mgnumverticesVMT(Word* args, Word& result, int message,
                 Word& local, Supplier s){
  result = qp->ResultStorage(s);
  CcInt*  res = (CcInt*) result.addr;
  Graph* g = getMemObject<Graph>((GN*) args[0].addr);
  if(!g){
     res->SetDefined(false);
     return 0;
  }
  res->Set(true,g->numVertices());
  return 0; 
}

ValueMapping mg2numverticesVM[] = {
  mgnumverticesVMT<Mem, MGraph2>,
  mgnumverticesVMT<MPointer, MGraph2>
};

int mgnumverticesSelect(ListExpr args){
  return Mem::checkType(nl->First(args))?0:1;
}

OperatorSpec mg2numverticesSpec(
  "MGRAPH2 -> int",
  "mg2numvertices(_)",
  "Returns the number of nodes of a mgraph2 object",
  "query mgenumvertices(mg2)"
);

Operator mg2numverticesOp(
  "mg2numvertices",
  mg2numverticesSpec.getStr(),
  2,
  mg2numverticesVM,
  mgnumverticesSelect,
  mgnumverticesTM<MGraph2>
);

/*
10.7 mgsuccessors mgpredecessors

*/
template<class Graph>
ListExpr mgsuccpredTM(ListExpr args){
  if(!nl->HasLength(args,2) &&!nl->HasLength(args,3)){
    return listutils::typeError("two or three arguments required");
  }
  ListExpr g;
  if(!getMemSubType(nl->First(args),g)){
    return listutils::typeError("first arg is not a memory object");
  }
  if(!Graph::checkType(g)){
    return listutils::typeError("argument is not a " + Graph::BasicType());
  }
  // check second argument
  ListExpr node = nl->Second(args);
  if(!CcInt::checkType(node) ){
    return listutils::typeError("second arg has to be of type int");
  }
  ListExpr resType = Stream<Tuple>::wrap(nl->Second(g));
  if(nl->HasLength(args,3)){
     if(!CcBool::checkType(nl->Third(args))){
       return listutils::typeError("third argument is not of type bool");
     }
     return resType;
  }
  // optional bool argument missing, add default 
  return nl->ThreeElemList(nl->SymbolAtom(Symbols::APPEND()),
                           nl->OneElemList(nl->BoolAtom(false)),
                           resType);

}



template<class GN, class Graph, bool isSucc>
int mgsuccpredVMT(Word* args, Word& result, int message,
                 Word& local, Supplier s){
 
   typedef typename Graph::singleNodeIterator iterator;
   iterator* li = (iterator*) local.addr;

   switch(message){
     case OPEN: {
        if(li){
          delete li;
          local.addr = 0;
        }
        // graph
        Graph* g = getMemObject<Graph>((GN*) args[0].addr);
        if(!g){
          return 0;
        }
        // node
        CcInt* v = (CcInt*) args[1].addr;
        if(!v->IsDefined()){
          return 0;
        }
        // delete option
        CcBool* DelOption = (CcBool*) args[2].addr;
        if(!DelOption->IsDefined()){
           return 0;
        } 
        bool delOption = DelOption->GetValue();
        if(isSucc){
           local.addr = g->getSuccessors(v->GetValue(),delOption);
        } else {
           local.addr = g->getPredecessors(v->GetValue(),delOption);
        }
        return 0;
     }
     case REQUEST:{
        result.addr = li?li->next():0;
        return result.addr?YIELD:CANCEL;
     }
     case CLOSE: {
         if(li){
           delete li;
           local.addr = 0;
         }
         return 0;
     }
   }
   return -1;
}

ValueMapping mg2successorsVM[] = {
  mgsuccpredVMT<Mem,MGraph2,true>,
  mgsuccpredVMT<MPointer,MGraph2,true>
};

ValueMapping mg2predecessorsVM[] = {
  mgsuccpredVMT<Mem,MGraph2,false>,
  mgsuccpredVMT<MPointer,MGraph2,false>
};

int mgsuccpredSelect(ListExpr args){
  return Mem::checkType(nl->First(args))?0:1;
}

OperatorSpec mg2successorsSpec(
"MGRAPH2 x int [x bool] -> stream(tuple)",
"_ mg2successors [_]",
"returns the successors of a specified node."
"if the optional boolean argument exist with value true,"
" all outgoing edges of this node are removed. This holds "
"also in the case that only a part of these edges are "
"put into result stream (e.g., restricted by a head).",
"query mg2 mg2successors[0] count"
);

OperatorSpec mg2predecessorsSpec(
"MGRAPH2 x int [x bool] -> stream(tuple)",
"_ mg2predecessors [_]",
"returns the predecessors of a specified node.",
"if the optional boolean argument exist with value true,"
" all incoming edges of this node are removed. This holds "
"also in the case that only a part of these edges are "
"put into result stream (e.g., restricted by a head).",
"query mg2 mg2predecessors[0] count"
);

Operator mg2successorsOp(
"mg2successors",
mg2successorsSpec.getStr(),
2,
mg2successorsVM,
mgsuccpredSelect,
mgsuccpredTM<MGraph2>
);


Operator mg2predecessorsOp(
"mg2predecessors",
mg2predecessorsSpec.getStr(),
2,
mg2predecessorsVM,
mgsuccpredSelect,
mgsuccpredTM<MGraph2>
);

/*
10.7 number of predecessors and successors

*/
template<class Graph>
ListExpr mgnumsuccpredTM(ListExpr args){
  if(!nl->HasLength(args,2)){
    return listutils::typeError("two arguments required");
  }
  ListExpr g;
  if(!getMemSubType(nl->First(args),g)){
   return listutils::typeError("first arg is not a memory object");
  }
  if(!Graph::checkType(g)){
    return listutils::typeError("argument is not a " + Graph::BasicType());
  }
  // check second argument
  ListExpr node = nl->Second(args);
  if(!CcInt::checkType(node) ){
    return listutils::typeError("second arg has to be of type int");
  }
  return listutils::basicSymbol<CcInt>();
}

template<class GN, class Graph, bool isSucc>
int mgnumsuccpredVMT(Word* args, Word& result, int message,
                 Word& local, Supplier s){
  result = qp->ResultStorage(s);
  CcInt*  res = (CcInt*) result.addr;
  Graph* g = getMemObject<Graph>((GN*) args[0].addr);
  if(!g){
     res->SetDefined(false);
     return 0;
  }
  CcInt* v = (CcInt*) args[1].addr;
  if(!v->IsDefined()){
     res->SetDefined(false);
     return 0;
  }
  int c = isSucc?g->succCount(v->GetValue()):g->predCount(v->GetValue());
  if(c<0){
    res->SetDefined(false);
    return 0;
  }
  res->Set(true,c);
  return 0;
}


ValueMapping mg2numsuccessorsVM[] = {
   mgnumsuccpredVMT<Mem,MGraph2,true>,
   mgnumsuccpredVMT<MPointer,MGraph2,true>
};

ValueMapping mg2numpredecessorsVM[] = {
   mgnumsuccpredVMT<Mem,MGraph2,false>,
   mgnumsuccpredVMT<MPointer,MGraph2,false>
};

int mgnumsuccpredSelect(ListExpr args){
  return Mem::checkType(nl->First(args))?0:1;
}

OperatorSpec mg2numsuccessorsSpec(
  "MGRAPH2 x int -> int",
  "_ mg2numsuccessors[_]",
  "Return the count of successors of a vertex.",
  "query mg2 mg2numsuccessors[1] "
);

OperatorSpec mg2numpredecessorsSpec(
  "MGRAPH2 x int -> int",
  "_ mg2numpredecessors[_]",
  "Return the count of predecessors of a vertex.",
  "query mg2 mg2numpredecessors[1] "
);

Operator mg2numsuccessorsOp(
  "mg2numsuccessors",
  mg2numsuccessorsSpec.getStr(),
  2,
  mg2numsuccessorsVM,
  mgnumsuccpredSelect,
  mgnumsuccpredTM<MGraph2>
);

Operator mg2numpredecessorsOp(
  "mg2numpredecessors",
  mg2numpredecessorsSpec.getStr(),
  2,
  mg2numpredecessorsVM,
  mgnumsuccpredSelect,
  mgnumsuccpredTM<MGraph2>
);

/*
10.8 mg2disconnect

*/
template<class Graph>
ListExpr mgdisconnectTM(ListExpr args){
  if(!nl->HasLength(args,2)){
    return listutils::typeError("two arguments required");
  }
  ListExpr g;
  if(!getMemSubType(nl->First(args),g)){
   return listutils::typeError("first arg is not a memory object");
  }
  if(!Graph::checkType(g)){
    return listutils::typeError("argument is not a " + Graph::BasicType());
  }
  // check second argument
  ListExpr node = nl->Second(args);
  if(!CcInt::checkType(node) ){
    return listutils::typeError("second arg has to be of type int");
  }
  return listutils::basicSymbol<CcBool>();
}

template<class GN, class Graph>
int mgdisconnectVMT(Word* args, Word& result, int message,
                 Word& local, Supplier s){
  result = qp->ResultStorage(s);
  CcBool*  res = (CcBool*) result.addr;
  Graph* g = getMemObject<Graph>((GN*) args[0].addr);
  if(!g){
     res->SetDefined(false);
     return 0;
  }
  CcInt* v = (CcInt*) args[1].addr;
  if(!v->IsDefined()){
     res->SetDefined(false);
     return 0;
  }
  res->Set(true,g->disconnect(v->GetValue()));
  return 0;
}

ValueMapping mg2disconnectVM[] = {
   mgdisconnectVMT<Mem, MGraph2>,
   mgdisconnectVMT<MPointer, MGraph2>
};

int mgdisconnectSelect(ListExpr args){
  return Mem::checkType(nl->First(args))?0:1;
}


OperatorSpec mg2disconnectSpec(
  "MGRAPH2 x int -> bool",
  "_ mg2disconnect[_]",
  "Disconnects a specified node from the remainder of the graph",
  "query mg2 mg2disconnect[6] "
);


Operator mg2disconnectOp(
  "mg2disconnect",
  mg2disconnectSpec.getStr(),
  2,
  mg2disconnectVM,
  mgdisconnectSelect,
  mgdisconnectTM<MGraph2>
);


/*
deleteEdges

*/
template<class G>
ListExpr mgdeleteEdgesTM(ListExpr args){
  if(!nl->HasLength(args,3)){
    return listutils::typeError("three arguments expected");
  }
  ListExpr g;
  if(!getMemSubType(nl->First(args),g)){
   return listutils::typeError("first arg is not a memory object");
  }
  if(!G::checkType(g)){
    return listutils::typeError("argument is not a " + G::BasicType());
  }
  if(!CcInt::checkType(nl->Second(args))){
    return listutils::typeError("second argument is not an int");
  }
  if(!CcInt::checkType(nl->Third(args))){
    return listutils::typeError("third argument is not an int");
  }
  return listutils::basicSymbol<CcBool>();
}


template<class GN, class Graph>
int mgdeleteEdgesVMT(Word* args, Word& result, int message,
                 Word& local, Supplier s){
  result = qp->ResultStorage(s);
  CcBool*  res = (CcBool*) result.addr;
  Graph* g = getMemObject<Graph>((GN*) args[0].addr);
  if(!g){
     res->SetDefined(false);
     return 0;
  }
  CcInt* source = (CcInt*) args[1].addr;
  CcInt* target = (CcInt*) args[2].addr;
  if(!source->IsDefined() || !target->IsDefined()){
     res->SetDefined(false);
     return 0;
  }
  g->removeAllEdges(source->GetValue(), target->GetValue());
  res->Set(true,true);
  return 0;
}


ValueMapping mg2deleteEdgesVM[] = {
   mgdeleteEdgesVMT<Mem, MGraph2>,
   mgdeleteEdgesVMT<MPointer, MGraph2>
};

int mgdeleteEdgesSelect(ListExpr args){
  return Mem::checkType(nl->First(args))?0:1;
}


OperatorSpec mg2deleteEdgesSpec(
  "MGRAPH2 x int x int -> bool",
  "graph  mg2deleteEdges[source, target]",
  "Removes all edges from source to target in graph.",
  "query mg2 mg2deleteEdges[2,7] "
);


Operator mg2deleteEdgesOp(
  "mg2deleteEdges",
  mg2deleteEdgesSpec.getStr(),
  2,
  mg2deleteEdgesVM,
  mgdeleteEdgesSelect,
  mgdeleteEdgesTM<MGraph2>
);


/*
11 Operators for mgraph3

*/
ListExpr createmgraph3TM(ListExpr args){
  // stream(tuple) x SourceName x TargetName x CostName x Size 
  if(!nl->HasLength(args,5)){
     return listutils::typeError("5 args expected");
  }
  // first arg: tuple stream
  ListExpr ts = nl->First(args);
  if(!Stream<Tuple>::checkType(ts)){
    return listutils::typeError("First arg is not a tuple stream");
  }
  ListExpr tupleType = nl->Second(ts);
  // second arg: name of src
  ListExpr sn = nl->Second(args);
  if(nl->AtomType(sn)!=SymbolType){
    return listutils::typeError("second arg is not a valid attribute name");
  }
  string src = nl->SymbolValue(sn);
  ListExpr attrType;
  ListExpr attrList = nl->Second(tupleType);
  int srcIndex = listutils::findAttribute(attrList, src,attrType);
  if(!srcIndex){
     return listutils::typeError("attribute " + src + " not part of the tuple");
  }
  if(!CcInt::checkType(attrType)){
     return listutils::typeError("attribute " + src + " not of type int");
  }
  // third arg: name of target
  ListExpr tn = nl->Third(args);
  if(nl->AtomType(tn)!=SymbolType){
    return listutils::typeError("third arg is not a valid attribute name");
  }
  string target = nl->SymbolValue(tn);
  int targetIndex = listutils::findAttribute(attrList, target,attrType);
  if(!targetIndex){
     return listutils::typeError("attribute " + target 
                                  + " not part of the tuple");
  }
  if(!CcInt::checkType(attrType)){
     return listutils::typeError("attribute " + target + " not of type int");
  }
  // fourth  arg: name of cost
  ListExpr cn = nl->Fourth(args);
  if(nl->AtomType(cn)!=SymbolType){
    return listutils::typeError("fourth arg is not a valid attribute name");
  }
  string cost = nl->SymbolValue(cn);
  int costIndex = listutils::findAttribute(attrList, cost,attrType);
  if(!costIndex){
     return listutils::typeError("attribute " + cost 
                                 + " not part of the tuple");
  }
  if(!CcReal::checkType(attrType)){
     return listutils::typeError("attribute " + cost + " not of type real");
  }
  // fifth arg: graph size
  ListExpr size = nl->Fifth(args);
  if(!CcInt::checkType(size)){
     return listutils::typeError("fifth attribute is not of type int");
  }

  ListExpr resType = MPointer::wrapType( Mem::wrapType(
                       MGraph3::wrapType(tupleType)));


  cout << "result type = " << nl->ToString(resType) << endl;

  return nl->ThreeElemList(
            nl->SymbolAtom(Symbols::APPEND()),
            nl->ThreeElemList( nl->IntAtom(srcIndex-1),
                               nl->IntAtom(targetIndex-1),
                               nl->IntAtom(costIndex-1)),
            resType
         );
}

             
template<bool flob>
int createmgraph3VMT(Word* args, Word& result, int message,
                 Word& local, Supplier s){

   result = qp->ResultStorage(s);
   MPointer* mp = (MPointer*) result.addr;
   CcInt* Size = (CcInt*) args[4].addr;
   if(!Size->IsDefined()){
      mp->setPointer(0);
      return 0;
   }
   int size = Size->GetValue();
   if(size<=0){
     mp->setPointer(0);
     return 0;
   }
   int srcPos = ((CcInt*) args[5].addr)->GetValue();
   int targetPos = ((CcInt*) args[6].addr)->GetValue();
   int costPos = ((CcInt*) args[7].addr)->GetValue();
   ListExpr memType = nl->Second(qp->GetType(s));
   MGraph3* mg3 = new MGraph3(flob, getDBname(),
                              nl->ToString(memType),
                              srcPos, targetPos, costPos,
                              size);

   Stream<Tuple> stream(args[0]);
   stream.open();
   Tuple* tuple;
   size_t lost = 0;
   while( (tuple = stream.request())){
      if(!mg3->insertGraphEdge(tuple)){
         lost++;
      }
      tuple->DeleteIfAllowed();
   }
   stream.close();
   mp->setPointer(mg3);
   mg3->deleteIfAllowed();
   return 0;
}

OperatorSpec createmgraph3Spec(
  "stream(tuple) x IDENT x IDENT x IDENT x int -> "
  "mpointer(mem(mgraph3(tuple)))",
  "edges createmgraph3[Src, Target, Cost, Size] ",
  "Creates an mgraph3 object in main memory using the given size. The graph "
  "will have Size vertices numbered from 0..Size-1. "
  "The Edges are defined by the incoming tuples. Src and Target must be "
  "attributes of the edges of type int. Cost is an attribute of the edges "
  "of type real. "
  "Edges having undefined or negative costs are "
  "not inserted into the graph. The same holds for edges having invalid "
  "Source or Target (undefined or outside the range [0,Size-1]).",
  "query rel feed createmgraph3[Id_s1, Id_s2, Cost, 1024"
);

Operator createmgraph3Op(
  "createmgraph3",
  createmgraph3Spec.getStr(),
  createmgraph3VMT<false>,
  Operator::SimpleSelect,
  createmgraph3TM
);

Operator createmgraph3flobOp(
  "createmgraph3flob",
  createmgraph3Spec.getStr(),
  createmgraph3VMT<true>,
  Operator::SimpleSelect,
  createmgraph3TM
);

/*
11.2 inserting new edges using mg3insert

*/

ValueMapping mg3insertVM[] = {
   mginsertVMT<Mem, MGraph3>,
   mginsertVMT<MPointer, MGraph3>
};


OperatorSpec mg3insertSpec(
  "stream<tuple> x MGRAPH3 -> stream(tuple)",
  "_ mg2insert[_]",
  "Inserts tuples into an existing graph."
  "Tuples with invalid values are ignored and are "
  "not passed to the output stream. "
  "Invalid tuples may have source or target node "
  "outside the graph or negative costs.",
  "query newEdges feed mginsert[mg3] count"
);


Operator mg3insertOp(
  "mg3insert",
  mg3insertSpec.getStr(),
  2,
  mg3insertVM,
  mginsertSelect,
  mginsertTM<MGraph3>
);


/*
11.3 Operator mg3feed

*/

ValueMapping mg3feedVM[] = {
   mg2feedVMT<Mem, MGraph3>,
   mg2feedVMT<MPointer, MGraph3>
};


OperatorSpec mg3feedSpec(
  "MGRAPH3 -> stream(tuple)",
  " _ mg3feed",
  "Extract the edges from a mgraph3 object",
  "query mg3 mgfeed count"
);


Operator mg3feedOp(
  "mg3feed",
  mg3feedSpec.getStr(),
  2,
  mg3feedVM,
  mgfeedSelect,
  mgfeedTM<MGraph3>
);


/*
Number of vertices

*/

ValueMapping mg3numverticesVM[] = {
  mgnumverticesVMT<Mem, MGraph3>,
  mgnumverticesVMT<MPointer, MGraph3>
};

OperatorSpec mg3numverticesSpec(
  "MGRAPH3 -> int",
  "mg3numvertices(_)",
  "Returns the number of nodes of a mgraph3 object",
  "query mg3numvertices(mg3)"
);

Operator mg3numverticesOp(
  "mg3numvertices",
  mg3numverticesSpec.getStr(),
  2,
  mg3numverticesVM,
  mgnumverticesSelect,
  mgnumverticesTM<MGraph3>
);


/*
Successors, Predecessors

*/

ValueMapping mg3successorsVM[] = {
  mgsuccpredVMT<Mem,MGraph3,true>,
  mgsuccpredVMT<MPointer,MGraph3,true>
};

ValueMapping mg3predecessorsVM[] = {
  mgsuccpredVMT<Mem,MGraph3,false>,
  mgsuccpredVMT<MPointer,MGraph3,false>
};


OperatorSpec mg3successorsSpec(
"MGRAPH3 x int [x bool] -> stream(tuple)",
"_ mg3successors [_]",
"returns the successors of a specified node.",
"if the optional boolean argument exist with value true,"
" all outgoing edges of this node are removed. This holds "
"also in the case that only a part of these edges are "
"put into result stream (e.g., restricted by a head).",
"query mg3 mg3successors[0] count"
);

OperatorSpec mg3predecessorsSpec(
"MGRAPH3 x int [x bool] -> stream(tuple)",
"_ mg3predecessors [_]",
"returns the predecessors of a specified node.",
"if the optional boolean argument exist with value true,"
" all incoming edges of this node are removed. This holds "
"also in the case that only a part of these edges are "
"put into result stream (e.g., restricted by a head).",
"query mg3 mg3predecessors[0,FALSE] count"
);

Operator mg3successorsOp(
"mg3successors",
mg3successorsSpec.getStr(),
2,
mg3successorsVM,
mgsuccpredSelect,
mgsuccpredTM<MGraph3>
);


Operator mg3predecessorsOp(
"mg3predecessors",
mg3predecessorsSpec.getStr(),
2,
mg3predecessorsVM,
mgsuccpredSelect,
mgsuccpredTM<MGraph3>
);


/*
Number of successors / predecessors

*/
ValueMapping mg3numsuccessorsVM[] = {
   mgnumsuccpredVMT<Mem,MGraph3,true>,
   mgnumsuccpredVMT<MPointer,MGraph3,true>
};

ValueMapping mg3numpredecessorsVM[] = {
   mgnumsuccpredVMT<Mem,MGraph3,false>,
   mgnumsuccpredVMT<MPointer,MGraph3,false>
};


OperatorSpec mg3numsuccessorsSpec(
  "MGRAPH3 x int -> int",
  "_ mg3numsuccessors[_]",
  "Return the count of successors of a vertex.",
  "query mg3 mg3numsuccessors[1] "
);

OperatorSpec mg3numpredecessorsSpec(
  "MGRAPH3 x int -> int",
  "_ mg3numpredecessors[_]",
  "Return the count of predecessors of a vertex.",
  "query mg3 mg3numpredecessors[1] "
);

Operator mg3numsuccessorsOp(
  "mg3numsuccessors",
  mg3numsuccessorsSpec.getStr(),
  2,
  mg3numsuccessorsVM,
  mgnumsuccpredSelect,
  mgnumsuccpredTM<MGraph3>
);

Operator mg3numpredecessorsOp(
  "mg3numpredecessors",
  mg3numpredecessorsSpec.getStr(),
  2,
  mg3numpredecessorsVM,
  mgnumsuccpredSelect,
  mgnumsuccpredTM<MGraph3>
);

/*
disconect

*/

ValueMapping mg3disconnectVM[] = {
   mgdisconnectVMT<Mem, MGraph3>,
   mgdisconnectVMT<MPointer, MGraph3>
};

OperatorSpec mg3disconnectSpec(
  "MGRAPH3 x int -> bool",
  "_ mg3disconnect[_]",
  "Disconnects a specified node from the rest of the graph",
  "query mg3 mg3disconnect[6] "
);


Operator mg3disconnectOp(
  "mg3disconnect",
  mg3disconnectSpec.getStr(),
  3,
  mg3disconnectVM,
  mgdisconnectSelect,
  mgdisconnectTM<MGraph3>
);


/*
mg3deleteEdges

*/

ValueMapping mg3deleteEdgesVM[] = {
   mgdeleteEdgesVMT<Mem, MGraph3>,
   mgdeleteEdgesVMT<MPointer, MGraph3>
};


OperatorSpec mg3deleteEdgesSpec(
  "MGRAPH3 x int x int -> bool",
  "graph  mg3deleteEdges[source, target]",
  "Removes all edges from source to target in graph.",
  "query mg3 mg3deleteEdges[2,7] "
);


Operator mg3deleteEdgesOp(
  "mg3deleteEdges",
  mg3deleteEdgesSpec.getStr(),
  2,
  mg3deleteEdgesVM,
  mgdeleteEdgesSelect,
  mgdeleteEdgesTM<MGraph3>
);



/*
Operator mgconnectedcomponents

*/
template<class G,bool perNode>
ListExpr mg23connectedcomponentsTM(ListExpr args){
  if(!nl->HasLength(args,1)){
    return listutils::typeError("one argument expected");
  }
  ListExpr graph;
  if(!getMemSubType(nl->First(args), graph)){
    return listutils::typeError("argument is not a memory object");
  }
  if(!G::checkType(graph)){
    return listutils::typeError("arg is not of type " + G::BasicType());
  }
  ListExpr attrList = nl->Second(nl->Second(graph));
  ListExpr d;
  
  if(!perNode){
    if(listutils::findAttribute(attrList,"CompNo",d)){
      return listutils::typeError("Graph representation contains an "
                               "attribute CompNo");
    }
    ListExpr cl = nl->OneElemList(nl->TwoElemList(
                           nl->SymbolAtom("CompNo"),
                           listutils::basicSymbol<CcInt>()));
    attrList = listutils::concat(attrList,cl);
  } else {
    if(listutils::findAttribute(attrList,"SourceComp",d)){
      return listutils::typeError("Graph representation contains an "
                               "attribute SourceComp");
    }
    if(listutils::findAttribute(attrList,"TargetComp",d)){
      return listutils::typeError("Graph representation contains an "
                               "attribute TargetComp");
    }
    ListExpr cl = nl->TwoElemList(
                     nl->TwoElemList(
                           nl->SymbolAtom("SourceComp"),
                           listutils::basicSymbol<CcInt>()),
                     nl->TwoElemList(
                           nl->SymbolAtom("TargetComp"),
                           listutils::basicSymbol<CcInt>())
                  );
    attrList = listutils::concat(attrList,cl);
  }
  
  return Stream<Tuple>::wrap(Tuple::wrap(attrList));
}

template<bool perNode>
class mg23connectedcomponentsInfo{
  public:
     mg23connectedcomponentsInfo(MGraphCommon* _g, TupleType* _tt): 
     graph(_g),tt(_tt){
        tt->IncReference();
        graph->components(compInfo); 
        pos = 0;
        if(pos<graph->numVertices()){
          it = graph->getSuccList(pos).begin(); 
        }
     }

     ~mg23connectedcomponentsInfo(){
        tt->DeleteIfAllowed();
     }
    
     Tuple* next(){
        while(pos < graph->numVertices()){
           if(it != graph->getSuccList(pos).end()){
              Tuple* t = createResultTuple(*it);
              it++;
              return t;
           }
           pos++;
           if(pos<graph->numVertices()){
             it = graph->getSuccList(pos).begin(); 
           }
        }
        return 0;
     }

  private:
     MGraphCommon* graph;
     TupleType* tt;
     vector<int> compInfo;
     size_t pos;
     list<MEdge>::const_iterator it;

     Tuple* createResultTuple(const MEdge& e){
        const Tuple* src = e.info;
        Tuple* res = new Tuple(tt);
        for(int i=0;i<src->GetNoAttributes();i++){
           res->CopyAttribute(i,src,i);
        }
        if(!perNode){
           int comp = compInfo[pos] == compInfo[e.target]?compInfo[pos]:-2;
           res->PutAttribute(src->GetNoAttributes(),new CcInt(true,comp));
        } else {
           int sourceComp = compInfo[pos];
           int targetComp = compInfo[e.target];
           res->PutAttribute(src->GetNoAttributes(),new CcInt(true,sourceComp));
           res->PutAttribute(src->GetNoAttributes()+1,
                             new CcInt(true,targetComp));
        }
        return res;
     }

};

template<class GN, class Graph, bool perNode>
int mg23connectedcomponentsVMT(Word* args, Word& result, int message,
                 Word& local, Supplier s){

   mg23connectedcomponentsInfo<perNode>* li = 
                      (mg23connectedcomponentsInfo<perNode>*) local.addr;
   switch(message){
      case INIT: {
         qp->GetLocal2(s).addr = new TupleType( 
                                         nl->Second(GetTupleResultType(s)));
         return 0;
      }
      case FINISH: {
         TupleType* tt = (TupleType*) qp->GetLocal2(s).addr;
         if(tt){
            tt->DeleteIfAllowed();
            qp->GetLocal2(s).addr=0;
         }
         return 0;
      }
      case OPEN: {
          if(li){
              delete li;
              local.addr = 0;
          }
          MGraphCommon* g = getMemObject<Graph>((GN*) args[0].addr);
          if(!g){
             return 0;
          }
          TupleType* tt = (TupleType*) qp->GetLocal2(s).addr;
          local.addr = new mg23connectedcomponentsInfo<perNode>(g,tt);
          return 0;
      }
      case REQUEST: {
        result.addr = li?li->next():0;
        return result.addr?YIELD:CANCEL;
      }
      case CLOSE : {
         if(li){
           delete li;
           local.addr = 0;
         }
         return 0;
      }
   }
   return -1;
}

int mg23connectedcomponentsSelect(ListExpr args){
   return Mem::checkType(nl->First(args))?0:1;
}

ValueMapping mg2connectedcomponentsVM [] = {
  mg23connectedcomponentsVMT<Mem,MGraph2, false>,
  mg23connectedcomponentsVMT<MPointer,MGraph2, false>
};

ValueMapping mg3connectedcomponentsVM [] = {
  mg23connectedcomponentsVMT<Mem,MGraph3, false>,
  mg23connectedcomponentsVMT<MPointer,MGraph3, false>
};

ValueMapping mg2connectedcomponents2VM [] = {
  mg23connectedcomponentsVMT<Mem,MGraph2, true>,
  mg23connectedcomponentsVMT<MPointer,MGraph2, true>
};

ValueMapping mg3connectedcomponents2VM [] = {
  mg23connectedcomponentsVMT<Mem,MGraph3, true>,
  mg23connectedcomponentsVMT<MPointer,MGraph3, true>
};


OperatorSpec mg23connctedcomponentsSpec(
  "mgraph{2,3} -> stream(tuple)",
  "mgconnectedcomponents(_)",
  "Append a component number to the graph edges",
  "query mgconnectedcomponents(mg2)"
);

Operator mg2connectedcomponentsOp(
  "mg2connectedcomponents",
  mg23connctedcomponentsSpec.getStr(),
  2,
  mg2connectedcomponentsVM,
  mg23connectedcomponentsSelect,
  mg23connectedcomponentsTM<MGraph2,false>
);

Operator mg3connectedcomponentsOp(
  "mg3connectedcomponents",
  mg23connctedcomponentsSpec.getStr(),
  2,
  mg3connectedcomponentsVM,
  mg23connectedcomponentsSelect,
  mg23connectedcomponentsTM<MGraph3,false>
);


OperatorSpec mg23connctedcomponentsNSpec(
  "mgraph{2,3} -> stream(tuple)",
  "mgconnectedcomponents(_)",
  "Append a component number to the graph nodes",
  "query graph23 mgXconnectedcomponentsN count"
);


Operator mg2connectedcomponentsNOp(
  "mg2connectedcomponentsN",
  mg23connctedcomponentsNSpec.getStr(),
  2,
  mg2connectedcomponents2VM,
  mg23connectedcomponentsSelect,
  mg23connectedcomponentsTM<MGraph2,true>
);

Operator mg3connectedcomponentsNOp(
  "mg3connectedcomponentsN",
  mg23connctedcomponentsNSpec.getStr(),
  2,
  mg3connectedcomponents2VM,
  mg23connectedcomponentsSelect,
  mg23connectedcomponentsTM<MGraph3,true>
);


/*
~ Computing contraction ~

*/
template<class G, bool onlySize>
ListExpr mgcontractTM(ListExpr args){
  if(!nl->HasLength(args,8)){
    return listutils::typeError("8 arguments expected");
  }
  ListExpr graph;
  if(!getMemSubType(nl->First(args),graph)){
    return listutils::typeError("first arg is not a memory object");
  }
  if(!G::checkType(graph)){
    return listutils::typeError("argument not of type " + G::BasicType());
  }

  if(!CcInt::checkType(nl->Second(args))){
    return listutils::typeError("expected graph x int^7");
  }
  if(!CcInt::checkType(nl->Third(args))){
    return listutils::typeError("expected graph x int^7");
  }
  if(!CcInt::checkType(nl->Fourth(args))){
    return listutils::typeError("expected graph x int^7");
  }
  if(!CcInt::checkType(nl->Fifth(args))){
    return listutils::typeError("expected graph x int^7");
  }
  if(!CcInt::checkType(nl->Sixth(args))){
    return listutils::typeError("expected graph x int^7");
  }
  if(!CcInt::checkType(nl->Sixth(nl->Rest(args)))){
    return listutils::typeError("expected graph x int^7");
  }
  if(!CcInt::checkType(nl->Sixth(nl->Rest(nl->Rest(args))))){
    return listutils::typeError("expected graph x int^7");
  }
  if(onlySize){
     return listutils::basicSymbol<CcInt>();
  }
}


template<class GN, class Graph>
int mgcontractVMT(Word* args, Word& result, int message,
                 Word& local, Supplier s){

  result = qp->ResultStorage(s);
  CcInt*  res = (CcInt*) result.addr;
  Graph* g = getMemObject<Graph>((GN*) args[0].addr);
  if(!g){
     res->SetDefined(false);
     return 0;
  }
  int maxPrio = 30;
  CcInt* V = (CcInt*) args[1].addr;
  if(V->IsDefined()){
    maxPrio = V->GetValue();
  }
  int minBlockSize = 100;
  V = (CcInt*) args[2].addr;
  if(V->IsDefined()){
    minBlockSize = V->GetValue();
    if(minBlockSize<1) minBlockSize = 1;
  } 
  int maxHopsF = 4;
  V = (CcInt*) args[3].addr;
  if(V->IsDefined()){
    maxHopsF = V->GetValue();
    if(maxHopsF<1) maxHopsF = 1;
  }
  int maxHopsB = 0;
  V = (CcInt*) args[4].addr;
  if(V->IsDefined()){
    maxHopsB = V->GetValue();
  }
  int variant = 1;
  V = (CcInt*) args[5].addr;
  if(V->IsDefined()){
    variant = V->GetValue();
  }
  if( (variant !=1) && (variant!=2)){
    res->SetDefined(false);
    return 0;
  }
  int skipReinsert = 0;
  V = (CcInt*) args[6].addr;
  if(V->IsDefined()){
     skipReinsert =  V->GetValue();
     if(skipReinsert<0) skipReinsert=0;
  }
  size_t maxEdges = std::numeric_limits<size_t>::max();
  V = (CcInt*) args[7].addr;
  if(V->IsDefined()){
     int e =  V->GetValue();
     if(e>0) maxEdges = e; 
  }
   


  std::vector<shortCutInfo> shortcuts; 
  res->Set(true, g->contract(maxPrio, minBlockSize, maxHopsF, 
                             maxHopsB,shortcuts,variant,
                             skipReinsert, maxEdges));
  return 0;
}

ValueMapping mg2contractVM[] = {
   mgcontractVMT<Mem, MGraph2>,
   mgcontractVMT<MPointer, MGraph2>
};

ValueMapping mg3contractVM[] = {
   mgcontractVMT<Mem, MGraph3>,
   mgcontractVMT<MPointer, MGraph3>
};

int mgcontractSelect(ListExpr args){
  return Mem::checkType(nl->First(args))?0:1;
}

OperatorSpec mg2contractSpec(
  "MGRAPH2 x int x int x int x int x int x int x int -> int",
  "mg2contract(graph, maxPrio, minBlockSIze, maxHopsF, maxHopsB, "
  "variant, skipReinsert, maxEdges)",
  "Computes the number of contraction edges created during contraction "
  "of a graph. \n"
  "graph : the graph to contract\n"
  "maxPrio : do not reorganize the queue until maxPrio is reached\n"
  "minBlockSize: do not reorganize the queue until minBlocksize nodes "
  "have been contracted\n"
  "maxHopsF : maximum number of hops (forward) for searching shortest paths\n"
  "maxHopsB : maximum number of hops for backward search. If <=0, the "
  "multitarget variant is used\n"
  "variant: 1 - two step variant as in ContractN, 2 - use EdgeDifference "
  "in single step\n"
  "skipReinsert: if the queue is smaller than this value, reinsertion of "
  "nodes into the queue is omitted\n"
  "maxEdges : maximum number of edges for a single shortest path search.",
  "query mg2contract(g2,20,800,3,1,2)"
);

OperatorSpec mg3contractSpec(
  "MGRAPH3 x int x int x int x int x int x int x int -> int",
  "mg3contract(graph, maxPrio, minBlockSIze, maxHopsF, maxHopsB, "
  "variant, skipReinsert, maxEdges)",
  "Computes the number of contraction edges created during contraction "
  "of a graph. \n"
  "graph : the graph to contract\n"
  "maxPrio : do not reorganize the queue until maxPrio is reached\n"
  "minBlockSize: do not reorganize the queue until minBlocksize nodes "
  "have been contracted\n"
  "maxHopsF : maximum number of hops (forward) for searching shortest paths\n"
  "maxHopsB : maximum number of hops for backward search. If <=0, the "
  "multitarget variant is used\n"
  "variant: 1 - two step variant as in ContractN, 3 - use EdgeDifference "
  "in single step\n"
  "skipReinsert: if the queue is smaller than this value, reinsertion of "
  "nodes into the queue is omitted"
  "maxEdges : maximum number of edges for a single shortest path search.",
  "query mg3contract(g3,30,800,3,1,3)"
);

Operator mg2contractOp(
  "mg2contract",
  mg2contractSpec.getStr(),
  2,
  mg2contractVM,
  mgcontractSelect,
  mgcontractTM<MGraph2, true>
);

Operator mg3contractOp(
  "mg3contract",
  mg3contractSpec.getStr(),
  2,
  mg3contractVM,
  mgcontractSelect,
  mgcontractTM<MGraph3,true>
);


/*
Operator minPathCosts for mgraph2 and mgraph3

*/
template<class G>
ListExpr mgminPathCostTM(ListExpr args){
  if(!nl->HasLength(args,5) && !nl->HasLength(args,7)){
    return listutils::typeError("5 arguments expected");
  }
  ListExpr graph;
  if(!getMemSubType(nl->First(args),graph)){
    return listutils::typeError("first argument is not a memory object");
  }
  if(!G::checkType(graph)){
    return listutils::typeError("argument not of type " + G::BasicType());
  }

  if(!CcInt::checkType(nl->Second(args))){
    return listutils::typeError("expected graph x int x int x int 2");
  }
  if(!CcInt::checkType(nl->Third(args))){
    return listutils::typeError("expected graph x int x int x int 3");
  }
  if(!CcInt::checkType(nl->Fourth(args))){
    return listutils::typeError("expected graph x int x int x int 4");
  }
  if(!CcInt::checkType(nl->Fifth(args))){
    return listutils::typeError("expected graph x int x int x int 5");
  }
  if(nl->HasLength(args,5)){
    return nl->ThreeElemList( nl->SymbolAtom(Symbols::APPEND()),
                              nl->TwoElemList(nl->IntAtom(-1), nl->RealAtom(0)),
                              listutils::basicSymbol<CcReal>());
  }
  if(!CcInt::checkType(nl->Sixth(args))){
    return listutils::typeError("expected graph x int x int x int 6");
  }
  if(!CcReal::checkType(nl->Sixth(nl->Rest(args)))){
    return listutils::typeError("maxCosts not of type real");
  }
  return listutils::basicSymbol<CcReal>();
}


template<class GN, class Graph>
int mgminPathCostVMT(Word* args, Word& result, int message,
                 Word& local, Supplier s){

  result = qp->ResultStorage(s);
  CcReal*  res = (CcReal*) result.addr;
  Graph* g = getMemObject<Graph>((GN*) args[0].addr);
  if(!g){
     res->SetDefined(false);
     return 0;
  }
  int source = 0;
  CcInt* V = (CcInt*) args[1].addr;
  if(V->IsDefined()){
    source = V->GetValue();
  }
  int target = 1;
  V = (CcInt*) args[2].addr;
  if(V->IsDefined()){
    target = V->GetValue();
  } 
  int maxHopsForward = 4;
  V = (CcInt*) args[3].addr;
  if(V->IsDefined()){
    maxHopsForward = V->GetValue();
  }
  int maxHopsBackward = 4;
  V = (CcInt*) args[4].addr;
  if(V->IsDefined()){
    maxHopsBackward = V->GetValue();
  }
  int forbidden = -1;
  V = (CcInt*) args[5].addr;
  if(V->IsDefined()){
    forbidden = V->GetValue();
  }
  double maxCosts = 0;
  CcReal* CV = (CcReal*) args[6].addr;
  if(CV->IsDefined()){
    maxCosts = CV->GetValue();
  }
   
  
  res->Set(true, g->pathCosts(source, target, maxHopsForward, 
                              maxHopsBackward,forbidden, maxCosts));
  return 0;
}

ValueMapping mg2minPathCostVM[] = {
   mgminPathCostVMT<Mem, MGraph2>,
   mgminPathCostVMT<MPointer, MGraph2>
};

ValueMapping mg3minPathCostVM[] = {
   mgminPathCostVMT<Mem, MGraph3>,
   mgminPathCostVMT<MPointer, MGraph3>
};

int mgminPathCostSelect(ListExpr args){
  return Mem::checkType(nl->First(args))?0:1;
}

OperatorSpec mg2minPathCostSpec(
  "MGRAPH2 x int x int x int x int [x int x real] -> real",
  "mg2minpathcists(graph, source, target, maxHopsForward, maxHopsBackward)"
  " [, forbidden, maxCosts]",
  "Comutes the minimum path length from source to target using a "
  "maximum number of hops in each direction",
  "query mg2minPathCost(g2, 1,42,10,10)"
);

OperatorSpec mg3minPathCostSpec(
  "MGRAPH3 x int x int x int x int [x int x real] -> real",
  "mg3minpathcists(graph, source, target, maxHopsForward, maxHopsBackward)"
  " [, forbidden, maxCosts]",
  "Comutes the minimum path length from source to target using a "
  "maximum number of hops in each direction",
  "query mg3minPathCost(g3, 1,42,10,10)"
);

Operator mg2minPathCostOp(
  "mg2minPathCost",
  mg2minPathCostSpec.getStr(),
  2,
  mg2minPathCostVM,
  mgminPathCostSelect,
  mgminPathCostTM<MGraph2>
);

Operator mg3minPathCostOp(
  "mg3minPathCost",
  mg3minPathCostSpec.getStr(),
  2,
  mg3minPathCostVM,
  mgminPathCostSelect,
  mgminPathCostTM<MGraph3>
);

/*
2.4  exportddsg

*/
template<class G>
ListExpr exportddsgTM(ListExpr args){
   if(!nl->HasLength(args,3)){
     return listutils::typeError("three arguments expected");
   }
   ListExpr graph;
   if(!getMemSubType(nl->First(args), graph)){
     return listutils::typeError("first arg is not a memory object");
   }
   if(!G::checkType(graph)){
     return listutils::typeError("argument not of type " + G::BasicType());
   }
   ListExpr fn = nl->Second(args);
   if(!FText::checkType(fn) && !CcString::checkType(fn)){
     return listutils::typeError("expected text or string as second argument");
   }
   ListExpr scale = nl->Third(args);
   if(!CcInt::checkType(scale) && !CcReal::checkType(scale)){
     return listutils::typeError("expected int or real as third argument");
   }
   return listutils::basicSymbol<CcBool>();
}


template<class GN, class FN, class SF, class Graph>
int exportddsgVMT(Word* args, Word& result, int message,
                 Word& local, Supplier s){

  result = qp->ResultStorage(s);
  CcBool*  res = (CcBool*) result.addr;
  Graph* g = getMemObject<Graph>((GN*) args[0].addr);
  FN* fileName = (FN*) args[1].addr;
  SF* scaleFactor = (SF*) args[2].addr;
  if(!g || !fileName->IsDefined() || !scaleFactor->IsDefined()){
     res->SetDefined(false);
     return 0;
  }
  string fn = fileName->GetValue();
  double sf = scaleFactor->GetValue();
  if(sf<=0){
     res->Set(true,false);
     return 0;
  }
  res->Set(true,g->exportDDSG(fn,sf));
  return 0;
}

ValueMapping mg2exportddsgVM[] = {
  exportddsgVMT<Mem, CcString, CcInt, MGraph2>,
  exportddsgVMT<Mem, CcString, CcReal, MGraph2>,
  exportddsgVMT<Mem, FText, CcInt, MGraph2>,
  exportddsgVMT<Mem, FText, CcReal, MGraph2>,
  
  exportddsgVMT<MPointer, CcString, CcInt, MGraph2>,
  exportddsgVMT<MPointer, CcString, CcReal, MGraph2>,
  exportddsgVMT<MPointer, FText, CcInt, MGraph2>,
  exportddsgVMT<MPointer, FText, CcReal, MGraph2>
};

ValueMapping mg3exportddsgVM[] = {
  exportddsgVMT<Mem, CcString, CcInt, MGraph3>,
  exportddsgVMT<Mem, CcString, CcReal, MGraph3>,
  exportddsgVMT<Mem, FText, CcInt, MGraph3>,
  exportddsgVMT<Mem, FText, CcReal, MGraph3>,
  
  exportddsgVMT<MPointer, CcString, CcInt, MGraph3>,
  exportddsgVMT<MPointer, CcString, CcReal, MGraph3>,
  exportddsgVMT<MPointer, FText, CcInt, MGraph3>,
  exportddsgVMT<MPointer, FText, CcReal, MGraph3>
};

int exportddsgSelect(ListExpr args){
  int n1 = Mem::checkType(nl->First(args))?0:4;
  int n2 = CcString::checkType(nl->Second(args))?0:2;
  int n3 = CcInt::checkType(nl->Third(args))?0:1;
  return n1+n2+n3;
}

OperatorSpec mg2exportddsgSpec(
  "MGRAPH2 x {string,text} x {int,real} -> bool",
  "graph mg2exportddsg[filename, scalefactor] ",
  "Export an mgraph2 to a file using the ddsg format. "
  "DDSG supports only integers as costs. All costs in the "
  "graph a multiplied with the scale factor and rounded "
  " to the next integer value. " 
  "The return value gives the success of this operation.",
  "query mg2 exportddsg['graph.txt', 1000]"
);

OperatorSpec mg3exportddsgSpec(
  "MGRAPH3 x {string,text} x {int,real} -> bool",
  "graph mg3exportddsg[filename, scalefactor] ",
  "Export an mgraph3 to a file using the ddsg format. "
  "DDSG supports only integers as costs. All costs in the "
  "graph a multiplied with the scale factor and rounded "
  " to the next integer value. " 
  "The return value gives the success of this operation.",
  "query mg3 exportddsg['graph.txt', 1000]"
);

Operator mg2exportddsgOp(
  "mg2exportddsg",
  mg2exportddsgSpec.getStr(),
  8,
  mg2exportddsgVM,
  exportddsgSelect,
  exportddsgTM<MGraph2>
);

Operator mg3exportddsgOp(
  "mg3exportddsg",
  mg3exportddsgSpec.getStr(),
  8,
  mg3exportddsgVM,
  exportddsgSelect,
  exportddsgTM<MGraph3>
);


/*
mgraphPrint

*/
ListExpr mgraphPrintTM(ListExpr args){
   if(!nl->HasLength(args,3)){
     return listutils::typeError("three argument expected");
   }
   ListExpr graph;
   if(!getMemSubType(nl->First(args), graph)){
     return listutils::typeError("first arg is not a memory object");
   }
   if(!MGraph2::checkType(graph) && !MGraph3::checkType(graph)){
     return listutils::typeError("argument not of type mgraph2 or mgraph3");
   }
   if(!CcBool::checkType(nl->Second(args))){
     return listutils::typeError("second arg not of type bool");
   }
   if(!CcBool::checkType(nl->Third(args))){
     return listutils::typeError("third arg not of type bool");
   }
   return listutils::basicSymbol<CcBool>();
}

template< class GN>
int mgraphPrintVMT(Word* args, Word& result, int message,
                Word& local, Supplier s){
  
  MGraphCommon* g = getMemObject2<MGraphCommon>((GN*) args[0].addr);
  CcBool* printInfo = (CcBool*) args[1].addr;
  CcBool* printBackward = (CcBool*) args[2].addr;
  result = qp->ResultStorage(s);
  CcBool* res = (CcBool*) result.addr;
  if(!printInfo->IsDefined() || !printBackward->IsDefined()){
    res->SetDefined(false);
    return 0; 
  }
  res->Set(true,true);
  std::vector<std::string>* names = nullptr;
  if(printInfo->GetValue()){
    ListExpr gT = qp->GetType(qp->GetSon(s,0));
    if(nl->HasLength(gT,2) && nl->IsEqual(nl->First(gT),MPointer::BasicType())){
      gT = nl->Second(gT);
    } // remove potential mpointer
    if(nl->HasLength(gT,2) && nl->IsEqual(nl->First(gT),Mem::BasicType())){
      gT = nl->Second(gT);
    } // remove mem
    if(nl->HasLength(gT,2) ){
      gT = nl->Second(gT);
    } // remove mgraphX -> tupleType left 

    names = new std::vector<std::string>(Tuple::getAttrNames(gT));
    // bring all names to the same length
    size_t m=0;
    for(size_t i=0;i<names->size();i++){
      if(m< (*names)[i].length()) m = (*names)[i].length();
    }
    for(size_t i=0;i<names->size();i++){
       while((*names)[i].length() < m){
         (*names)[i] += " ";
       }
    }
  }
  g->print(cout, names, printBackward->GetValue());
  if(names != nullptr){
     delete names;
  }
  return 0;
}

ValueMapping mgraphPrintVM[] = {
   mgraphPrintVMT<Mem>,
   mgraphPrintVMT<MPointer>
};

int mgraphPrintSelect(ListExpr args){
   return Mem::checkType(nl->First(args))?0:1;
}

OperatorSpec mgraphPrintSpec(
   "MGRAPH x bool x bool -> bool ",
   "_ mgraphPrint[_,_] ",
   "Prints out the graph that is given as the first argument. "
   "The second argument specifies whether the tuple info of "
   " each edge should be printed out. The third argment specifies "
   "whether backward edges should be printed out. ",
   "query mg2 print[FALSE,FALSE] "
);

Operator mgraphPrintOp(
   "mgraphPrint",
   mgraphPrintSpec.getStr(),
   2,
   mgraphPrintVM,
   mgraphPrintSelect,
   mgraphPrintTM   
);


/*
mgraph2text

*/
ListExpr mgraph2textTM(ListExpr args){
   if(!nl->HasLength(args,3)){
     return listutils::typeError("three argument expected");
   }
   ListExpr graph;
   if(!getMemSubType(nl->First(args), graph)){
     return listutils::typeError("first arg is not a memory object");
   }
   if(!MGraph2::checkType(graph) && !MGraph3::checkType(graph)){
     return listutils::typeError("argument not of type mgraph2 or mgraph3");
   }
   if(!CcBool::checkType(nl->Second(args))){
     return listutils::typeError("second arg not of type bool");
   }
   if(!CcBool::checkType(nl->Third(args))){
     return listutils::typeError("third arg not of type bool");
   }
   return listutils::basicSymbol<FText>();
}

template< class GN>
int mgraph2textVMT(Word* args, Word& result, int message,
                Word& local, Supplier s){
  
  MGraphCommon* g = getMemObject2<MGraphCommon>((GN*) args[0].addr);
  CcBool* printInfo = (CcBool*) args[1].addr;
  CcBool* printBackward = (CcBool*) args[2].addr;
  result = qp->ResultStorage(s);
  FText* res = (FText*) result.addr;
  if(!printInfo->IsDefined() || !printBackward->IsDefined()){
    res->SetDefined(false);
    return 0; 
  }
  stringstream ss;
  std::vector<std::string>* names = nullptr;
  if(printInfo->GetValue()){
    ListExpr gT = qp->GetType(qp->GetSon(s,0));
    if(nl->HasLength(gT,2) && nl->IsEqual(nl->First(gT),MPointer::BasicType())){
      gT = nl->Second(gT);
    } // remove potential mpointer
    if(nl->HasLength(gT,2) && nl->IsEqual(nl->First(gT),Mem::BasicType())){
      gT = nl->Second(gT);
    } // remove mem
    if(nl->HasLength(gT,2) ){
      gT = nl->Second(gT);
    } // remove mgraphX -> tupleType left 

    names = new std::vector<std::string>(Tuple::getAttrNames(gT));
    // bring all names to the same length
    size_t m=0;
    for(size_t i=0;i<names->size();i++){
      if(m< (*names)[i].length()) m = (*names)[i].length();
    }
    for(size_t i=0;i<names->size();i++){
       while((*names)[i].length() < m){
         (*names)[i] += " ";
       }
    }
  }
  g->print(ss, names, printBackward->GetValue());
  if(names != nullptr){
     delete names;
  }
  res->Set(true, ss.str());
  return 0;
}

ValueMapping mgraph2textVM[] = {
   mgraph2textVMT<Mem>,
   mgraph2textVMT<MPointer>
};

int mgraph2textSelect(ListExpr args){
   return Mem::checkType(nl->First(args))?0:1;
}

OperatorSpec mgraph2textSpec(
   "MGRAPH x bool x bool -> bool ",
   "_ mgraph2text[_,_] ",
   "Converts a main memory grapg to a text. "
   "The second argument specifies whether the tuple info of "
   " each edge should be part of the result. The third argment specifies "
   "whether backward edges should be in the result. ",
   "query mgraph2text[FALSE,FALSE] "
);

Operator mgraph2textOp(
   "mgraph2text",
   mgraph2textSpec.getStr(),
   2,
   mgraph2textVM,
   mgraph2textSelect,
   mgraph2textTM   
);


ListExpr MGroupTM(ListExpr args)
{
  if(nl->ListLength(args)<1){
     ErrorReporter::ReportError("one argument expected");
     return nl->TypeError();
  }

  ListExpr first = nl->First(args);

  if(!Stream<Tuple>::checkType(first)){
    return listutils::typeError("tuple stream expected");
  }

  ListExpr relType = nl->TwoElemList( listutils::basicSymbol<Relation>(),
                                      nl->Second(first));
  return MPointer::wrapType(Mem::wrapType(relType));

}
/*
2.3.2 Specification of operator ~MGroup~

*/
const string MGroupSpec  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                          "\"Remarks\" ) "
                          "( <text>((stream x)) -> (mem (rel x))</text--->"
                          "<text>type operator</text--->"
                          "<text>Maps stream type to a mem rel.</text--->"
                          "<text>not for use with sos-syntax</text--->"
                          ") )";




/*
2.3.3 Definition of operator ~group~

*/
Operator mgroupOp (
         "MGROUP",              // name
         MGroupSpec,            // specification
         0,                    // no value mapping
         Operator::SimpleSelect,   // trivial selection function
         MGroupTM          // type mapping
);




/*
Operator memgroupby

*/
ListExpr memgroupbyTM(ListExpr args){

  if(!nl->HasLength(args,3)){
    return listutils::typeError("stream x attrlist x funlist expected");
  }
  if(!Stream<Tuple>::checkType(nl->First(args))){
    return listutils::typeError("first arg is not a tuple stream");
  }
  if(nl->AtomType(nl->Second(args))!=NoAtom){
    return listutils::typeError("second arg is not a list of attributes");
  }
  if(nl->AtomType(nl->Third(args))!=NoAtom){
    return listutils::typeError("third arg is not a list of functions");
  }
  vector<int> groupAttr;
  vector<ListExpr> groupTypes;
  vector<string> groupNames;
  ListExpr attrList = nl->Second(nl->Second(nl->First(args)));
  ListExpr groupAttrList = nl->Second(args);
  while(!nl->IsEmpty( groupAttrList)){
    ListExpr ga = nl->First(groupAttrList);
    groupAttrList = nl->Rest(groupAttrList);
    if(nl->AtomType(ga)!=SymbolType){
      return listutils::typeError("Invalid attr ID in attr list");
    }
    string aname = nl->SymbolValue(ga);
    ListExpr at;
    int index = listutils::findAttribute(attrList, aname, at);
    if(!index){
      return listutils::typeError("attribute " + aname 
      + " not part of the incoming stream");
    }
    groupAttr.push_back(index - 1);
    groupTypes.push_back(at);
    groupNames.push_back(aname);
  }
  

   ListExpr reltype  = nl->TwoElemList(
                         listutils::basicSymbol<Relation>(),
                         nl->Second(nl->First(args)));

   ListExpr funarg = MPointer::wrapType(Mem::wrapType(reltype));

  // check funtion list
  ListExpr funlist = nl->Third(args);
  while(!nl->IsEmpty(funlist)){
    ListExpr nfun = nl->First(funlist);
    funlist = nl->Rest(funlist);
    if(!nl->HasLength(nfun,2)){
       return listutils::typeError("found problem in function list");
    }    
    ListExpr aname = nl->First(nfun);
    string err;
    if(!listutils::isValidAttributeName(aname, err)){
      return listutils::typeError("invalid attr name " + err);
    }
    ListExpr fun = nl->Second(nfun);
    if(!listutils::isMap<1>(fun)){
      return listutils::typeError("found something that is not "
                                  "an unary function");
    }
    if(!nl->Equal(funarg, nl->Second(fun))){
      return listutils::typeError("invalid function argument");
    } 
    if(!Attribute::checkType(nl->Third(fun))){
      return listutils::typeError("function result is not an attribute");
    }
    groupNames.push_back(nl->SymbolValue(aname));
    groupTypes.push_back(nl->Third(fun));
  } 

  if(groupNames.empty()){
    return listutils::typeError("grouping attributes and functions are empty");
  }
  assert(groupNames.size() == groupTypes.size());
 
  ListExpr resAttrList = nl->OneElemList( 
                         nl->TwoElemList(nl->SymbolAtom(groupNames[0]),
                                         groupTypes[0]));
  ListExpr last = resAttrList;
  for(size_t i=1;i<groupNames.size();i++){
    last = nl->Append(last, nl->TwoElemList(nl->SymbolAtom(groupNames[i]),
                                         groupTypes[i]));
  }
  ListExpr resType = nl->TwoElemList(
                        listutils::basicSymbol<Stream<Tuple> >(),
                        nl->TwoElemList(
                           listutils::basicSymbol<Tuple>(),
                           resAttrList));



  if(groupAttr.empty()){
    return resType;
  }

  ListExpr appendList = nl->OneElemList( nl->IntAtom(groupAttr[0]));
  last = appendList;
  for(size_t i=1;i<groupAttr.size();i++){
    last = nl->Append(last, nl->IntAtom(groupAttr[i]));
  }


  ListExpr r =  nl->ThreeElemList(
               nl->SymbolAtom(Symbols::APPEND()),
               appendList,
               resType);
  return r;
}


class memgroupbyLocalInfo{
  public:

    memgroupbyLocalInfo(Word _stream, 
                      vector<int>* _groupAttr, 
                      vector<Supplier>* _funs,
                      vector<ArgVectorPointer>* _funargs,
                      TupleType* _tt): stream(_stream),
                      groupAttr(_groupAttr), funs(_funs),
                      funargs(_funargs), tt(_tt) {
        tt->IncReference();
        stream.open();
        start = stream.request();
        mrel = new MemoryRelObject();
        mp = new MPointer(mrel,true);
    }

    ~memgroupbyLocalInfo(){
        tt->DeleteIfAllowed();
        stream.close();
        delete mp; // mrel is deleted automatically
    }

    Tuple* next() {
       if(!start){
         return 0;
       }
       updateMRel();
       return createResultTuple();
    }  


    private:
      Stream<Tuple> stream;
      vector<int>* groupAttr;
      vector<Supplier>* funs;
      vector<ArgVectorPointer>* funargs;
      TupleType* tt;
      Tuple* start;
      MemoryRelObject* mrel; // represents the current group
      MPointer* mp;
      Word funres;
      

      bool matches(Tuple* tuple){
        for(size_t i=0;i<groupAttr->size();i++){
          int a = (*groupAttr)[i];
          Attribute* a1 = start->GetAttribute(a);
          Attribute* a2 = tuple->GetAttribute(a);
          if(a1->Compare(a2)!=0){
            return false;
          }
        }
        return true;
      }

      void updateMRel(){
         mrel->clear();
         mrel->addTuple(start);
         Tuple* tuple;
         while( (tuple=stream.request())){
           if(matches(tuple)){
              mrel->addTuple(tuple);
           } else {
             start = tuple;
             return;
           }
         }
         start = 0;
      }

      Tuple* createResultTuple(){
         vector<Tuple*>* rel = mrel->getmmrel();
         assert(rel->size()>0);

         Tuple* t1 = (*rel)[0];
         Tuple* res = new Tuple(tt);
         // copy group attributes
         for(size_t i=0;i<groupAttr->size();i++){
            res->CopyAttribute((*groupAttr)[i],t1,i);
         }
         // append the function results
         int n1 = groupAttr->size();
         for(size_t i=0;i<funs->size();i++){
           ArgVectorPointer avp = (*funargs)[i];
           ((*avp)[0]).setAddr(mp); 
           Supplier fun = (*funs)[i];
           qp->Request(fun, funres);
           res->PutAttribute(n1+i, ((Attribute*)funres.addr)->Clone());
         }

         return res;
      }
};


struct memgroupbyGlobalInfo{
  TupleType* tt;
  vector<int> groupAttr;
  vector<Supplier> funs;
  vector<ArgVectorPointer> funargs;
};

int memgroupbyVM(Word* args, Word& result, int message,
                 Word& local, Supplier s){

   memgroupbyLocalInfo* li = (memgroupbyLocalInfo*) local.addr;
   memgroupbyGlobalInfo* gi = (memgroupbyGlobalInfo*) qp->GetLocal2(s).addr;

   switch(message){
     case INIT:{
        return 0;
     }

     case FINISH: {
        if(gi){
           gi->tt->DeleteIfAllowed();
           delete gi;
           qp->GetLocal2(s).addr=0;
        } 
        return 0;
     }

     case OPEN:{
        if(!gi){
                    gi = new memgroupbyGlobalInfo();
                    gi->tt = new TupleType(nl->Second(GetTupleResultType(s)));
          // collect attribute positions
                    for(int i=3; i<qp->GetNoSons(s);i++){
                        gi->groupAttr.push_back(
                                 ((CcInt*)args[i].addr)->GetValue());
                    }
          // collect functions and theire argument vectors
                    Supplier funlist = qp->GetSon(s,2); 
                    for(int i=0;i<qp->GetNoSons(funlist);i++){
            Supplier namedfun = qp->GetSupplier(funlist,i);
            Supplier fun = qp->GetSupplier(namedfun,1);
                        gi->funs.push_back(fun);  
            ArgVectorPointer avp = qp->Argument(fun);
            gi->funargs.push_back(avp); 
                    }
                    qp->GetLocal2(s).addr = gi;

        }


        if(li) delete li;
        local.addr = new memgroupbyLocalInfo(args[0], 
                                   &(gi->groupAttr),
                                   &(gi->funs),
                                   &(gi->funargs),
                                   gi->tt);
        return 0;

     }

     case REQUEST:
            result.addr = li?li->next():0;
            return result.addr?YIELD:CANCEL;

     case CLOSE:{
         if(li){
            delete li;
            local.addr = 0;
         }
         return 0;
     }

   }
  return -1;
}


OperatorSpec memgroupbySpec(
  "stream(T) x attrlist x funlist -> stream(B) ",
  " _ memgroupby[_ , _ ] ",
  "Groupes a sorted stream of tuples by a set of "
  "attributes and extend each group by "
  "evaluating functions on this group. "
  "For each group, one result tuple is created",
  "query plz feed sortby[PLZ] memgroupby[PLZ; C : group count] consume"
);

Operator memgroupbyOp(
   "memgroupby",
   memgroupbySpec.getStr(),
   memgroupbyVM,
   Operator::SimpleSelect,
   memgroupbyTM

);

/*
2.5 Operator importCH

*/
ListExpr importCHTM(ListExpr args){
  if(!nl->HasLength(args,2)){
    return listutils::typeError("one argument expected");
  }
  if(!checkUsesArgs(args)){
    return listutils::typeError("internal error");
  }

  ListExpr arg = nl->First(nl->First(args));
  if(!FText::checkType(arg) && !CcString::checkType(arg)){
    return listutils::typeError("string or text expected as first arg");
  }
  if(!CcBool::checkType(nl->First(nl->Second(args)))){
     return listutils::typeError("second argument not of type bool");
  }  
  ListExpr b = nl->Second(nl->Second(args));
  if(nl->AtomType(b) != BoolType){
    return listutils::typeError("second argument must be a constant bool");
  }
  ListExpr attrList;
  if(nl->BoolValue(b)){
    attrList = nl->TwoElemList(
                nl->TwoElemList(nl->SymbolAtom("Node"),
                               listutils::basicSymbol<CcInt>()),
                nl->TwoElemList(nl->SymbolAtom("Number"),
                                listutils::basicSymbol<CcInt>())
              );
  } else {
     attrList = nl->OneElemList(
                    nl->TwoElemList(nl->SymbolAtom("Source"),
                                    listutils::basicSymbol<CcInt>()));
     ListExpr last = attrList;
     last = nl->Append(last, 
                         nl->TwoElemList(nl->SymbolAtom("Target"),
                                         listutils::basicSymbol<CcInt>()));
     last = nl->Append(last, 
                         nl->TwoElemList(nl->SymbolAtom("Weight"),
                                         listutils::basicSymbol<CcInt>()));
     last = nl->Append(last, 
                         nl->TwoElemList(nl->SymbolAtom("Middle"),
                                         listutils::basicSymbol<CcInt>()));
     last = nl->Append(last, 
                         nl->TwoElemList(nl->SymbolAtom("Forward"),
                                         listutils::basicSymbol<CcBool>()));
     last = nl->Append(last, 
                         nl->TwoElemList(nl->SymbolAtom("Backward"),
                                         listutils::basicSymbol<CcBool>()));
     last = nl->Append(last, 
                         nl->TwoElemList(nl->SymbolAtom("Shortcut"),
                                         listutils::basicSymbol<CcBool>()));
  }
  ListExpr res = nl->TwoElemList(
                    listutils::basicSymbol<Stream<Tuple> >(),
                    nl->TwoElemList(
                       listutils::basicSymbol<Tuple>(),
                       attrList));
  return res;

}


class importCHInfo{
   public:

     importCHInfo(string _fn, bool _order, TupleType* _tt):
        fn(_fn), order(_order), tt(_tt){
        tt->IncReference();
        in.open(fn.c_str(), ios::in | ios::binary);
        size_t buffersize = 4*1024*1024;
        inbuf = new char[buffersize];
        in.rdbuf()->pubsetbuf(inbuf,buffersize);
        error = false;
        endChecked = false;
        if(!in){
           error = true;
           cerr << "cannot open file " << fn << endl; 
           return; 
        }
        char id[4];
        in.read(id,4);   
        if(!in){
           error = true;
           cerr << "cannot read id " << endl;
           return;
        }
        if(id[0]!='C' || id[1]!='H' || id[2]!='\r' || id[3]!='\n'){
          cerr << "invalid id" << endl;
          error = true;
          return;
        }
        uint32_t  version;
        in.read((char*)&version,4);
        if(version!=1){
           cerr << "invalid version" << endl;
           return;
        }
        in.read((char*)&noNodes,4);
        in.read((char*)&m1,4);
        in.read((char*)&m2,4);
        cout << "noNodes " << noNodes << endl;
        cout << "no original edges " << m1 << endl;
        cout << "no shortcut edges " << m2 << endl;
        cout << "expected file size " << (4*5 + noNodes*4 + m1*16 + m2*20 + 4) 
             << " bytes " << endl;
        currentNode =0;
        currentM1 = 0;
        currentM2 = 0;
        if(!order){ // ignore node level block
           cout << " ignore " << (noNodes*4) << " bytes " << endl;
           char* buf = new char[noNodes*4];
           in.read(buf,noNodes*4);
           delete[] buf;
        }
        
     }
     ~importCHInfo(){
         in.close();
         delete[] inbuf;
         tt->DeleteIfAllowed();
      }

     Tuple* next(){
        return order?nextOrder():nextEdge();
     }



   private:
      string fn;
      bool order;
      TupleType* tt;
      char* inbuf;
      ifstream in; 
      bool error;
      uint32_t noNodes;
      uint32_t m1;
      uint32_t m2;
      uint32_t currentNode;
      uint32_t currentM1;
      uint32_t currentM2;
      bool endChecked;

     Tuple* nextOrder(){
        if(error) return 0;
        if(!in) {
          error = true;
          return 0;
        }
        if(currentNode>=noNodes){
           error = true; // not really an error
           return 0;
        }
        uint32_t level;
        in.read((char*) &level,4);
        if(!in){
           error = true;
           return 0;
        }
        Tuple* res = new Tuple(tt);
        res->PutAttribute(0, new CcInt(true,currentNode));
        res->PutAttribute(1, new CcInt(true, level));
        currentNode++;
        return res;
     }

     Tuple* nextEdge(){
        if(error) return 0;
        if(currentM1>=m1 && currentM2 >= m2){
          if(!endChecked){
             uint32_t end;
             in.read((char*)&end,4);
             cout << "postion in file is " << in.tellg() << endl; 
             if(end!=0x12345678){
               cerr << "end check failed" << endl;
               cerr << "end code is " << ios::hex << end << endl;
             } else {
               cout << "end recognized" << endl;
             }
             endChecked = true;  
          }
          return 0;
        } 

        uint32_t source;
        uint32_t target;
        uint32_t weight;
        uint32_t flags;
        uint32_t middle1;
        int middle;
        in.read((char*) &source,4);
        in.read((char*) &target,4);
        in.read((char*) &weight,4);
        in.read((char*) &flags,4);
        bool shortcut = currentM1>=m1;
        if(shortcut){
           in.read((char*) &middle1,4);
           middle = middle1;
        } else {
           middle = -1;
        }
        Tuple* res = new Tuple(tt);
        res->PutAttribute(0, new CcInt(true,source));
        res->PutAttribute(1, new CcInt(true,target));
        res->PutAttribute(2, new CcInt(true,weight));
        res->PutAttribute(3, new CcInt(true,middle));
        uint32_t mask = 1;
        res->PutAttribute(4, new CcBool(true, flags & mask));
        mask = 2;
        res->PutAttribute(5, new CcBool(true, flags & mask));
        mask = 4;
        res->PutAttribute(6, new CcBool(true, flags & mask));
        if(shortcut){
           currentM2++;
        } else {
          currentM1++;
        }
        return res;
     }


};


template<class F>
int importCHVMT(Word* args, Word& result, int message,
                 Word& local, Supplier s){

  importCHInfo* li = (importCHInfo*) local.addr;
  switch(message){
   case OPEN: {
     if(li){
       delete li;
       local.addr = 0;
     }
     F* filename = (F*) args[0].addr;
     CcBool* order = (CcBool*) args[1].addr;
     if(!filename->IsDefined() || !order->IsDefined()){
        return 0;
     }
     TupleType* tt = new TupleType(nl->Second(GetTupleResultType(s)));
     local.addr = new importCHInfo(filename->GetValue(), order->GetValue(), tt);
     tt->DeleteIfAllowed();
     return 0;
   }
   case REQUEST: {
      result.addr = li?li->next():0;
      return result.addr?YIELD:CANCEL;
   }
   case CLOSE: {
      if(li){
        delete li;
        local.addr = 0;
      }
      return 0;
   }

   }
   return -1;
    
}


ValueMapping importCHVM[] = {
  importCHVMT<CcString>,
  importCHVMT<FText>
};

int importCHSelect(ListExpr args){
  return CcString::checkType(nl->First(args))?0:1;
}

OperatorSpec importCHSpec(
  "{string,text} x bool -> stream(tuple)",
  "importCH(fileName, onlyOrder)",
  "Import a file in CH format (binary contraction hierarchie). "
  "The first argument specifies the file name. If the second "
  "argument is true, only the levels of the nodes are returned "
  " as a tuple stream (Node : int, Level : int). "
  "If this argument is set to false, the edges are returned in "
  "a stream of tuple (Source : int, Target : int, Weight : int, Middle : int, "
  "Forward : bool Backward : bool, Shortcut : bool).",
  " query importCH('edges.hc', TRUE) count"
);

Operator importCHOp(
  "importCH",
   importCHSpec.getStr(),
   2,
   importCHVM,
   importCHSelect,
   importCHTM
);



/*
Operator mmergejoinproject


*/

ListExpr mmergejoinprojectTM(ListExpr args){
  // rel1 x rel2 x attr1 x attr2  projectlist
  if(!nl->HasLength(args,5)){
    return listutils::typeError("55555 arguments expected");
  }
  ListExpr rel1;
  if(!getMemSubType(nl->First(args),rel1)){
    return listutils::typeError("first arg is not a memory object");
  }
  if(!Relation::checkType(rel1)){
    return listutils::typeError("first arg is not a memory relation");
  }
  ListExpr tt1 = nl->Second(rel1);
  ListExpr attrList1 = nl->Second(tt1);

  // second argument: an mrel
  ListExpr rel2;
  if(!getMemSubType(nl->Second(args),rel2)){
    return listutils::typeError("Second argument is not a memory object");
  }
  if(!Relation::checkType(rel2)){
    return listutils::typeError("first arg is not a memory relation");
  }
  ListExpr tt2 = nl->Second(rel2);
  ListExpr attrList2 = nl->Second(tt2);

  // third argument: an attribute name of rel1 
  ListExpr attr1 = nl->Third(args);
  if(nl->AtomType(attr1) != SymbolType){
    return listutils::typeError("third arg is not a valid attribute name");
  }
  string attrName1 = nl->SymbolValue(attr1);
  ListExpr attrType1;
  int attrIndex1 = listutils::findAttribute(attrList1, attrName1, attrType1);
  if(!attrIndex1){
    return listutils::typeError("Attribute " + attrName1 
                                 + " not part of the first relation");
  }
  // fourth argument: an attribute name of rel2
  ListExpr attr2 = nl->Fourth(args);
  if(nl->AtomType(attr2) != SymbolType){
    return listutils::typeError("fourth arg is not a valid attribute name");
  }
  string attrName2 = nl->SymbolValue(attr2);
  ListExpr attrType2;
  int attrIndex2 = listutils::findAttribute(attrList2, attrName2, attrType2);
  if(!attrIndex2){
    return listutils::typeError("Attribute " + attrName2 
                                 + " not part of the second relation");
  }
  // check for equal types
  if(!nl->Equal(attrType1, attrType2)){
    return listutils::typeError("Types of attributes " + attrName1 + " and "
                                + attrName2 + " differ.");
  }
  // fifth argument: the projection list
  ListExpr prjList = nl->Fifth(args);
  if(nl->AtomType(prjList) != NoAtom){
    return listutils::typeError("fifth argument must be a list "
                                "of attribute names");
  }
  ListExpr completeAttrList = listutils::concat(attrList1, attrList2); 
  ListExpr prjIndexes = nl->TheEmptyList();
  ListExpr prjIndexesLast = nl->TheEmptyList();
  ListExpr resAttrList = nl->TheEmptyList();
  ListExpr resAttrListLast = nl->TheEmptyList();
  bool first = true;
  while(!nl->IsEmpty(prjList)){
    ListExpr attr = nl->First(prjList);
    prjList = nl->Rest(prjList); 
    if(nl->AtomType(attr)!=SymbolType){
      return listutils::typeError("projection list contains an "
                                  "invalid attribute");
    }
    ListExpr attrType;
    string attrName = nl->SymbolValue(attr);
    int attrIndex = listutils::findAttribute(completeAttrList, 
                                             attrName, attrType);
    if(!attrIndex){
      return listutils::typeError("attribute " + attrName 
                                  + " not part of the relations");
    }
    if(first){
      prjIndexes = nl->OneElemList(nl->IntAtom(attrIndex-1));
      resAttrList = nl->OneElemList(nl->TwoElemList(attr, attrType));
      prjIndexesLast = prjIndexes;
      resAttrListLast = resAttrList;
      first = false;
    } else {
       prjIndexesLast = nl->Append(prjIndexesLast, nl->IntAtom(attrIndex-1));
       resAttrListLast = nl->Append(resAttrListLast, 
                                    nl->TwoElemList(attr, attrType));
    }
  }
  if(!listutils::isAttrList(resAttrList)){
    return listutils::typeError("There are name conflicts in projection list");
  }
  ListExpr resType = MPointer::wrapType(
                       Mem::wrapType(
                              nl->TwoElemList(
                                 listutils::basicSymbol<Relation>(),
                                 nl->TwoElemList(
                                    listutils::basicSymbol<Tuple>(),
                                    resAttrList)))); 

  return nl->ThreeElemList(
           nl->SymbolAtom(Symbols::APPEND()),
           nl->ThreeElemList(nl->IntAtom(attrIndex1-1),
                             nl->IntAtom(attrIndex2-1),
                             prjIndexes),
           resType
         );
}

template<class R1, class R2>
int mmergejoinprojectVMT(Word* args, Word& result, int message,
                        Word& local, Supplier s){

   result = qp->ResultStorage(s);
   MPointer* res = (MPointer*) result.addr;
   R1* r1 = (R1*) args[0].addr;
   R2* r2 = (R2*) args[1].addr;
   
   MemoryRelObject* rel1 = getMemRel(r1);
   MemoryRelObject* rel2 = getMemRel(r2);
   if(!rel1 || !rel2){
     res->setPointer(0);
     return 0;      
   }
   // create result and insert into catalog
   ListExpr resType = nl->Second(qp->GetType(s));
   MemoryRelObject* resRel = new MemoryRelObject(nl->ToString(resType));

   // do the join
   vector<Tuple*>* v1 = rel1->getmmrel();
   vector<Tuple*>* v2 = rel2->getmmrel();
   
   size_t s1 = v1->size();
   size_t s2 = v2->size();
   size_t pos1 = 0;
   size_t pos2 = 0;
   size_t posm = 0;

   int ai1 = ((CcInt*)args[5].addr)->GetValue();
   int ai2 = ((CcInt*)args[6].addr)->GetValue();

   vector<int> prjList;
   Supplier sup2 = qp->GetSon(s,7);
   for( int i=0;i<qp->GetNoSons(sup2);i++){
      Word elem;
      Supplier s3 = qp->GetSupplier(sup2,i);
      qp->Request(s3,elem);
      prjList.push_back(((CcInt*) elem.addr)->GetValue());
   }


   TupleType* tt = new TupleType(nl->Second(nl->Second(
                                 nl->Second(qp->GetNumType(s)))));

   while((pos1 < s1) && (pos2 < s2)){
       Tuple* t1 = v1->at(pos1);
       Tuple* t2 = v2->at(pos2);
       if(!t1){
          pos1++;
       } else if(!t2){
          pos2++;
       } else {
          Attribute* a1 = t1->GetAttribute(ai1);
          Attribute* a2 = t2->GetAttribute(ai2);
          int cmp = a1->Compare(a2);
          if(cmp < 0){
             pos1++;
          } else if(cmp>0){
             pos2++;
          } else {
             // we scan v2 until the attribute is bigger or v2 is finished
             // after that, we reset pos2 and increase
             posm = pos2;
             while(pos2 < s2 && cmp==0){
                 // create result tuple
                 Tuple* rt = new Tuple(tt);
                 for(size_t i=0;i<prjList.size();i++){
                    int a = prjList[i];
                    if(a< t1->GetNoAttributes()){
                       rt->CopyAttribute(a,t1,i);
                     } else {
                        a = a - t1->GetNoAttributes();
                        rt->CopyAttribute(a,t2,i);
                    }
                 }
                 resRel->addTuple(rt);
                 // next undeleted tuple
                 pos2++;
                 while( (pos2<s2)  && ( (t2 = v2->at(pos2))==0)){
                    pos2++;
                 }
                 if(pos2 < s2){
                     a2 = t2->GetAttribute(ai2);
                     cmp = a1->Compare(a2);
                 }
             }
             pos2 = posm;
             pos1++;
          }
       }
   }
   tt->DeleteIfAllowed();
   res->setPointer(resRel);
   resRel->deleteIfAllowed();
   return 0;
}

ValueMapping mmergejoinprojectVM[] = {
    mmergejoinprojectVMT<Mem,Mem>,
    mmergejoinprojectVMT<Mem,MPointer>,
    mmergejoinprojectVMT<MPointer,Mem>,
    mmergejoinprojectVMT<MPointer,MPointer>
  };

OperatorSpec mmergejoinprojectSpec(
  " MREL x MREL x IDENT x IDENT x INDENT^+  -> mpointer ",
  " rel1 rel2 mmergejoinproject[attr1, attr2, prjlist] ",
  " joins the relations rel1 and rel2 on attributes attr1 and attr2,"
  " respectively." 
  "It's assumed rel1 and rel2 are sorted by the join attributes. "
  "The concatenated tuples are projected to the prjlist. ",
  "query mten mplz mmergerjoinproject[No, PLZ; "
  "No,PLZ,Ort] count" 
);

int mmergejoinprojectSelect(ListExpr args){
  int n1 = Mem::checkType(nl->First(args))?0:2;
  int n2 = Mem::checkType(nl->Second(args))?0:1;
  return n1+n2;
}

Operator mmergejoinprojectOp(
   "mmergejoinproject",
   mmergejoinprojectSpec.getStr(),
   4, 
   mmergejoinprojectVM,
   mmergejoinprojectSelect,
   mmergejoinprojectTM
);

/*
9 N-tree support

6.1 ~mcreatentree~: Creation of an N-tree 

6.1.1 Type Mapping

Applied for operators ~mcreatentree~, ~mcreatentree2~, ~mcreatentree5~,
~mcreatentree6~, and ~mcreatentree7~.

*/
template<int variant>
ListExpr mcreatentreeTM(ListExpr args) {
  bool useParams = (variant == 2); // two integer args for pivots and pruning
  string err = (!useParams ? "expected: MREL x attrname x int x int [x geoid]" :
                 "expected: MREL x attrname x int x int x int x int [x geoid]");
  if (useParams) {
    if (!nl->HasLength(args, 6) && !nl->HasLength(args, 7)) {
      return listutils::typeError(" (6 or 7 arguments are required");
    }
  }
  else {
    if (!nl->HasLength(args, 4) && !nl->HasLength(args, 5)) {
      return listutils::typeError(" (4 or 5 arguments are required");
    }
  }
  if (!MPointer::checkType(nl->First(args))) {
    return listutils::typeError(err + " (first arg is not an mpointer)");
  }
  if (nl->AtomType(nl->Second(args)) != SymbolType) {
    return listutils::typeError(err + " (second argument is not a valid "
                                      "attribute name)");
  }
  if (!CcInt::checkType(nl->Fourth(args))) {
    return listutils::typeError(err + " (fourth argument is not an integer)");
  }
  bool geoidPresent = false;
  if (!CcInt::checkType(nl->Third(args))) {
    return listutils::typeError(err + " (third argument is not an integer)");
  }
  if (useParams) {
    if (!CcInt::checkType(nl->Fifth(args))) {
      return listutils::typeError(err + " (fifth argument is not an integer)");
    }
    if (!CcInt::checkType(nl->Sixth(args))) {
      return listutils::typeError(err + " (sixth argument is not an integer)");
    }
    if (nl->HasLength(args, 7)) {
      if (!Geoid::checkType(nl->Seventh(args))) {
        return listutils::typeError("seventh argument is not a geoid");
      }
      geoidPresent = true;
    }
  }
  else {
    if (nl->HasLength(args, 5)) {
      if (!Geoid::checkType(nl->Fifth(args))) {
        return listutils::typeError("fifth argument is not a geoid");
      }
      geoidPresent = true;
    }
  }
  // extract tuple type from first argument
  ListExpr tupletype;
  ListExpr mpt = nl->Second(nl->Second(nl->First(args)));
  if (!Relation::checkType(mpt)) {
    return listutils::typeError(" mpointer to a non-relation");
  }
  tupletype = nl->Second(mpt);
  ListExpr attrList = nl->Second(tupletype);
  string name = nl->SymbolValue(nl->Second(args));
  ListExpr type;
  int index1 = listutils::findAttribute(attrList, name, type);
  if (!index1) {
    return listutils::typeError("attribute " + name + " not part of the tuple");
  }
  // support geoid only for point, mpoint, cupoint, cmpoint
  if (geoidPresent) {
    if (!Point::checkType(type) && !temporalalgebra::MPoint::checkType(type) &&
        !temporalalgebra::CUPoint::checkType(type) &&
        !temporalalgebra::CMPoint::checkType(type)) {
      return listutils::typeError("geoid support only for (m|cu|cm|eps)point");
    }
  }
  // check for supported type, extend if required
  int no = mtreehelper::getTypeNo(type, 12);
  if (no < 0) {
     return listutils::typeError("no distance function available for type "
                                 + nl->ToString(type));
  }
  ListExpr resType;
  if (useParams) {
    resType = MPointer::wrapType(Mem::wrapType(nl->TwoElemList(
       listutils::basicSymbol<MemoryNtreeObject<Point, StdDistComp<Point>, 
                                                variant> >(),
       type)));
  }
  else {
    resType = MPointer::wrapType(Mem::wrapType(nl->TwoElemList(
        listutils::basicSymbol<MemoryNtreeObject<Point, StdDistComp<Point>,
                                                 variant> >(),
        type)));
  }
  ListExpr appendList;
  if (geoidPresent) {
    appendList = nl->TwoElemList(nl->IntAtom(index1 - 1), 
                                 nl->StringAtom(nl->ToString(type)));
  }
  else {
    appendList = nl->ThreeElemList( 
                   nl->TwoElemList(listutils::basicSymbol<Geoid>(),
                                   listutils::getUndefined()), 
                   nl->IntAtom(index1 - 1), 
                   nl->StringAtom(nl->ToString(type)));
  }
  ListExpr result = nl->ThreeElemList(nl->SymbolAtom(Symbols::APPEND()),
                                      appendList, resType);
  return result;
}

/*
6.2 Value Mapping template

*/
template<class T, int variant>
int mcreatentreeVMT(Word* args, Word& result, int message, Word& local,
                    Supplier s) {
  result = qp->ResultStorage(s);
  MPointer* res = (MPointer*)result.addr;
  MPointer* mrelp = (MPointer*)args[0].addr;
  if (mrelp->isNull()) {
    res->setPointer(0);
    return 0;
  }
  Geoid* geoid = (Geoid*)args[4].addr;
  int index = ((CcInt*)args[5].addr)->GetValue(); 
//    string tn = ((CcString*) args[4].addr)->GetValue();
  if (!geoid->IsDefined()) {
    geoid = 0;
  }
  MemoryRelObject* mrel = (MemoryRelObject*) mrelp->GetValue();
  int degree = ((CcInt*)args[2].addr)->GetValue();
  int maxLeafSize = ((CcInt*)args[3].addr)->GetValue();
  if (degree < 1 || maxLeafSize < 1 || degree > maxLeafSize) {
    cout << "invalid parameters: degree=" << degree << ", maxLeafSize=" 
        << maxLeafSize << endl;
    res->setPointer(0);
    return 0;
  }
  PartitionMethod partMethod = (variant == 8 ? RANDOMOPT : RANDOMONLY);
  vector<Tuple*>* v = mrel->getmmrel();
  vector<MTreeEntry<T> > contents;
  bool flobused = false;
  T* attr = 0;
  for (size_t i = 0; i < v->size(); i++) {
    attr = (T*)(v->at(i)->GetAttribute(index));
    MTreeEntry<T> entry(*attr, i + 1);
    contents.push_back(entry);
    flobused = flobused || (attr->NumOfFLOBs() > 0);
  }
  StdDistComp<T> dc(geoid);
  NTree<MTreeEntry<T>, StdDistComp<T>, variant>* tree =
     new NTree<MTreeEntry<T>, StdDistComp<T>, variant>(degree, maxLeafSize, dc,
                                                       partMethod, index);
  tree->build(contents);
//   cout << "entries: " << tree->getNoEntries() << ", nodes: " 
//        << tree->getNoNodes() << ", leaves: " << tree->getNoLeaves() << endl;
  size_t usedMem = tree->memSize();
  ListExpr typeList = nl->Second(qp->GetType(s));
  MemoryNtreeObject<T, StdDistComp<T>, variant>* ntree = 
      new MemoryNtreeObject<T, StdDistComp<T>, variant>(tree, usedMem, 
                                nl->ToString(typeList), !flobused, getDBname());
  NTreeStat stat = ntree->getNtreeX()->getStat();
  string counterName = "counterMCreateNTree" + 
                       (variant > 1 ? to_string(variant) : "");
  mtreehelper::increaseCounter(counterName,
                               stat.noDCInnerNodes + stat.noDCLeaves);
//   string counterINName = counterName + "InnerNodes";
//   mtreehelper::increaseCounter(counterINName, stat.noDCInnerNodes);
//   string counterLName = counterName + "Leaves";
//   mtreehelper::increaseCounter(counterLName, stat.noDCLeaves);
  mtreehelper::increaseCounter("noVisitedInnerNodes", stat.noInnerNodes);
  mtreehelper::increaseCounter("noVisitedLeaves", stat.noLeaves);
  res->setPointer(ntree);
  ntree->deleteIfAllowed();
  return 0;
}

/*
6.3 Selection Function and Value Mapping Array

*/
int mcreatentreeSelect(ListExpr args) {
  string attrName = nl->SymbolValue(nl->Second(args));
  ListExpr attrList = nl->Second(nl->Second(nl->Second(nl->Second(nl->First(
                                                                      args)))));
          //  mpointer   mem         rel       tuple 
  ListExpr type;
  listutils::findAttribute(attrList, attrName, type);
  return mtreehelper::getTypeNo(type, 12);
}

 // note: if adding attributes with flobs, the value mapping must be changed

 template<int variant>
ValueMapping mcreatentreeVM[] = {
  mcreatentreeVMT<mtreehelper::t1, variant>,
  mcreatentreeVMT<mtreehelper::t2, variant>,
  mcreatentreeVMT<mtreehelper::t3, variant>,
  mcreatentreeVMT<mtreehelper::t4, variant>,
  mcreatentreeVMT<mtreehelper::t5, variant>,
  mcreatentreeVMT<mtreehelper::t6, variant>,
  mcreatentreeVMT<mtreehelper::t7, variant>,
  mcreatentreeVMT<mtreehelper::t8, variant>,
  mcreatentreeVMT<mtreehelper::t9, variant>,
  mcreatentreeVMT<mtreehelper::t10, variant>,
  mcreatentreeVMT<mtreehelper::t11, variant>,
  mcreatentreeVMT<mtreehelper::t12, variant>
};

OperatorSpec mcreatentreeSpec(
  "MREL(tuple) x attrname x int x int [x geoid] -> , mpointer(mem(mtree X))\n",
  "mrel mcreatentree[indexAttr, degree, maxLeafSize, [, geoid] ]\n",
  "This operator creates an N-tree in main memory. "
  "The first argument is a main memory relation containing the "
  "tuples to be indexed. The second argument refers to the attribute "
  "over that the index is built. The next two arguments represent the degree of"
  " the tree and and maximum number of entries in a leaf.\n"
  "The last argument is optional. It must be of type geoid and "
  "can only be used if the index-attribute is of type point, mpoint, cupoint, "
  "or cmpoint. If this argument is present, the distance between two objects "
  "is computed as geographic distance on this geoid instead of using the "
  "Euclidean distance.\n In detail, the following types are supported:\n\n"
  "  * point:   p1->Distance(*p2, geoid)\n"
  "  * string:  stringutils::ld->(s1->GetValue(), s2->GetValue())\n"
  "  * int:     abs(i1->GetValue() - i2->GetValue())\n"
  "  * real:    abs(r1->GetValue() - r2->GetValue())\n"
  "  * rect<d>: r1->Distance(*r2)\n"
  "  * mpoint:  mp1->DistanceAvg(*mp2, geoid)\n"
  "  * cupoint: cup1->DistanceAvg(*cup2, true, geoid)\n"
  "  * cmpoint: cmp1->DistanceAvg(*cmp2, true, geoid)\n",
  "let kinosM_ntree_GeoData =  kinosM mcreatentree[GeoData, 5, 8]"
);

Operator mcreatentreeOp(
   "mcreatentree",
   mcreatentreeSpec.getStr(),
   12,
   mcreatentreeVM<1>,
   mcreatentreeSelect,
   mcreatentreeTM<1>
);

/*
Operator ~mcreatentree2~ 

6.2 Value Mapping template

*/
template<class T>
int mcreatentree2VMT(Word* args, Word& result, int message, Word& local,
                     Supplier s) {
  result = qp->ResultStorage(s);
  MPointer* res = (MPointer*)result.addr;
  MPointer* mrelp = (MPointer*)args[0].addr;
  if (mrelp->isNull()) {
    res->setPointer(0);
    return 0;
  }
  Geoid* geoid = (Geoid*)args[6].addr;
  int index = ((CcInt*)args[7].addr)->GetValue(); 
//    string tn = ((CcString*) args[4].addr)->GetValue();
  if (!geoid->IsDefined()) {
    geoid = 0;
  }
  MemoryRelObject* mrel = (MemoryRelObject*) mrelp->GetValue();
  CcInt *ccDegree = ((CcInt*)args[2].addr);
  CcInt *ccMaxLeafSize = ((CcInt*)args[3].addr);
  CcInt *ccCandOrder = ((CcInt*)args[4].addr);
  CcInt *ccPMethod = ((CcInt*)args[5].addr);
  if (!ccDegree->IsDefined() || !ccMaxLeafSize->IsDefined() || 
      !ccCandOrder->IsDefined() || !ccPMethod->IsDefined()) {
    cout << "at least one undefined input parameter" << endl;
    res->setPointer(0);
    return 0;
  }  
  int degree = ccDegree->GetValue();
  int maxLeafSize = ccMaxLeafSize->GetValue();
  if (degree < 1 || maxLeafSize < 1 || degree > maxLeafSize) {
    cout << "invalid parameter(s): degree=" << degree << ", maxLeafSize=" 
        << maxLeafSize << endl;
    res->setPointer(0);
    return 0;
  }
  if (ccCandOrder->GetValue() < 0 || ccCandOrder->GetValue() > 2) {
    cout << "invalid parameter: candOrder must be in {0, 1, 2}" << endl;
    res->setPointer(0);
    return 0;
  }
  CandOrder candOrder = static_cast<CandOrder>(ccCandOrder->GetValue());
  if (ccPMethod->GetValue() < 0 || ccPMethod->GetValue() > 1) {
    cout << "invalid parameter: pruningMethod must be in {0, 1}" << endl;
    res->setPointer(0);
    return 0;
  }
  PruningMethod pMethod = static_cast<PruningMethod>(ccPMethod->GetValue());
  PartitionMethod partMethod = RANDOMONLY; //TODO: currently fixed
  vector<Tuple*>* v = mrel->getmmrel();
  vector<MTreeEntry<T> > contents;
  bool flobused = false;
  T* attr = 0;
  for (size_t i = 0; i < v->size(); i++) {
    attr = (T*)(v->at(i)->GetAttribute(index));
    MTreeEntry<T> entry(*attr, i + 1);
    contents.push_back(entry);
    flobused = flobused || (attr->NumOfFLOBs() > 0);
  }
  StdDistComp<T> dc(geoid);
  NTree<MTreeEntry<T>, StdDistComp<T>, 2>* tree =
      new NTree<MTreeEntry<T>, StdDistComp<T>, 2>(degree, maxLeafSize, 
                                     candOrder, pMethod, dc, partMethod, index);
  tree->build(contents);
//   cout << "entries: " << tree->getNoEntries() << ", nodes: " 
//        << tree->getNoNodes() << ", leaves: " << tree->getNoLeaves() << endl;
  size_t usedMem = tree->memSize();
  ListExpr typeList = nl->Second(qp->GetType(s));
  MemoryNtreeObject<T, StdDistComp<T>, 2>* ntree2 = 
      new MemoryNtreeObject<T, StdDistComp<T>, 2>(tree, usedMem, 
                                nl->ToString(typeList), !flobused, getDBname());
  mtreehelper::increaseCounter("counterMCreateNTree2", 
                        ntree2->getNtreeX()->getDistComp().getNoDistFunCalls());
  res->setPointer(ntree2);
  ntree2->deleteIfAllowed();
  return 0;
}

/*
6.3 Selection Function and Value Mapping Array

*/
int mcreatentree2Select(ListExpr args) {
  string attrName = nl->SymbolValue(nl->Second(args));
  ListExpr attrList = nl->Second(nl->Second(nl->Second(nl->Second(nl->First(
                                                                      args)))));
          //  mpointer   mem         rel       tuple 
  ListExpr type;
  listutils::findAttribute(attrList, attrName, type);
  return mtreehelper::getTypeNo(type, 12);
}

 // note: if adding attributes with flobs, the value mapping must be changed

ValueMapping mcreatentree2VM[] = {
  mcreatentree2VMT<mtreehelper::t1>,
  mcreatentree2VMT<mtreehelper::t2>,
  mcreatentree2VMT<mtreehelper::t3>,
  mcreatentree2VMT<mtreehelper::t4>,
  mcreatentree2VMT<mtreehelper::t5>,
  mcreatentree2VMT<mtreehelper::t6>,
  mcreatentree2VMT<mtreehelper::t7>,
  mcreatentree2VMT<mtreehelper::t8>,
  mcreatentree2VMT<mtreehelper::t9>,
  mcreatentree2VMT<mtreehelper::t10>,
  mcreatentree2VMT<mtreehelper::t11>,
  mcreatentree2VMT<mtreehelper::t12>
};

OperatorSpec mcreatentree2Spec(
  "MREL(tuple) x attrname x int x int x int x int [x geoid] -> "
  "mpointer(mem(mtree X))\n",
  "mrel mcreatentree[indexAttr, degree, maxLeafSize, candOrder, pruningMethod "
  "[, geoid]]\n",
  "This operator creates an N-tree2 in main memory. "
  "The first argument is a main memory relation containing the "
  "tuples to be indexed. The second argument refers to the attribute "
  "over that the index is built. The next two arguments represent the degree of"
  " the tree and and maximum number of entries in a leaf.\n"
  "The fifth argument represents the candidate ordering (0 <=> random order, "
  "1 <=> use two reference points, 2 <=> use three reference points).\n"
  "The sixth argument is for selecting the pruning method (0 <=> simple, 1 <=> "
  "minDist-based).\n"
  "The last argument is optional. It must be of type geoid and "
  "can only be used if the index-attribute is of type point, mpoint, cupoint, "
  "or cmpoint. If this argument is present, the distance between two objects "
  "is computed as geographic distance on this geoid instead of using the "
  "Euclidean distance.\n In detail, the following types are supported:\n\n"
  "  * point:   p1->Distance(*p2, geoid)\n"
  "  * string:  stringutils::ld->(s1->GetValue(), s2->GetValue())\n"
  "  * int:     abs(i1->GetValue() - i2->GetValue())\n"
  "  * real:    abs(r1->GetValue() - r2->GetValue())\n"
  "  * rect<d>: r1->Distance(*r2)\n"
  "  * mpoint:  mp1->DistanceAvg(*mp2, geoid)\n"
  "  * cupoint: cup1->DistanceAvg(*cup2, true, geoid)\n"
  "  * cmpoint: cmp1->DistanceAvg(*cmp2, true, geoid)\n",
  "let kinosM_ntree_GeoData =  kinosM mcreatentree2[GeoData, 5, 8, 1, 0]"
);

Operator mcreatentree2Op(
   "mcreatentree2",
   mcreatentree2Spec.getStr(),
   12,
   mcreatentree2VM,
   mcreatentree2Select,
   mcreatentreeTM<2>
);

/*
Operator ~mcreatentree5~

*/
OperatorSpec mcreatentree5Spec(
  "MREL(tuple) x attrname x int x int [x geoid] -> , mpointer(mem(mtree X))\n",
  "mrel mcreatentree5[indexAttr, degree, maxLeafSize, [, geoid] ]\n",
  "This operator creates an N-tree5 in main memory. "
  "The first argument is a main memory relation containing the "
  "tuples to be indexed. The second argument refers to the attribute "
  "over that the index is built. The next two arguments represent the degree of"
  " the tree and and maximum number of entries in a leaf.\n"
  "The last argument is optional. It must be of type geoid and "
  "can only be used if the index-attribute is of type point, mpoint, cupoint, "
  "or cmpoint. If this argument is present, the distance between two objects "
  "is computed as geographic distance on this geoid instead of using the "
  "Euclidean distance.\n In detail, the following types are supported:\n\n"
  "  * point:   p1->Distance(*p2, geoid)\n"
  "  * string:  stringutils::ld->(s1->GetValue(), s2->GetValue())\n"
  "  * int:     abs(i1->GetValue() - i2->GetValue())\n"
  "  * real:    abs(r1->GetValue() - r2->GetValue())\n"
  "  * rect<d>: r1->Distance(*r2)\n"
  "  * mpoint:  mp1->DistanceAvg(*mp2, geoid)\n"
  "  * cupoint: cup1->DistanceAvg(*cup2, true, geoid)\n"
  "  * cmpoint: cmp1->DistanceAvg(*cmp2, true, geoid)\n",
  "let kinosM_ntree_GeoData =  kinosM mcreatentree5[GeoData, 5, 8]"
);

Operator mcreatentree5Op(
   "mcreatentree5",
   mcreatentree5Spec.getStr(),
   12,
   mcreatentreeVM<5>,
   mcreatentreeSelect,
   mcreatentreeTM<5>
);

/*
Operator ~mcreatentree6~

*/
OperatorSpec mcreatentree6Spec(
  "MREL(tuple) x attrname x int x int [x geoid] -> , mpointer(mem(mtree X))\n",
  "mrel mcreatentree6[indexAttr, degree, maxLeafSize, [, geoid] ]\n",
  "This operator creates an N-tree6 in main memory. "
  "The first argument is a main memory relation containing the "
  "tuples to be indexed. The second argument refers to the attribute "
  "over that the index is built. The next two arguments represent the degree of"
  " the tree and and maximum number of entries in a leaf.\n"
  "The last argument is optional. It must be of type geoid and "
  "can only be used if the index-attribute is of type point, mpoint, cupoint, "
  "or cmpoint. If this argument is present, the distance between two objects "
  "is computed as geographic distance on this geoid instead of using the "
  "Euclidean distance.\n In detail, the following types are supported:\n\n"
  "  * point:   p1->Distance(*p2, geoid)\n"
  "  * string:  stringutils::ld->(s1->GetValue(), s2->GetValue())\n"
  "  * int:     abs(i1->GetValue() - i2->GetValue())\n"
  "  * real:    abs(r1->GetValue() - r2->GetValue())\n"
  "  * rect<d>: r1->Distance(*r2)\n"
  "  * mpoint:  mp1->DistanceAvg(*mp2, geoid)\n"
  "  * cupoint: cup1->DistanceAvg(*cup2, true, geoid)\n"
  "  * cmpoint: cmp1->DistanceAvg(*cmp2, true, geoid)\n",
  "let kinosM_ntree_GeoData =  kinosM mcreatentree6[GeoData, 5, 8]"
);

Operator mcreatentree6Op(
   "mcreatentree6",
   mcreatentree6Spec.getStr(),
   12,
   mcreatentreeVM<6>,
   mcreatentreeSelect,
   mcreatentreeTM<6>
);

/*
Operator ~mcreatentree7~

*/
OperatorSpec mcreatentree7Spec(
  "MREL(tuple) x attrname x int x int [x geoid] -> , mpointer(mem(mtree X))\n",
  "mrel mcreatentree7[indexAttr, degree, maxLeafSize, [, geoid] ]\n",
  "This operator creates an N-tree7 in main memory. "
  "The first argument is a main memory relation containing the "
  "tuples to be indexed. The second argument refers to the attribute "
  "over that the index is built. The next two arguments represent the degree of"
  " the tree and and maximum number of entries in a leaf.\n"
  "The last argument is optional. It must be of type geoid and "
  "can only be used if the index-attribute is of type point, mpoint, cupoint, "
  "or cmpoint. If this argument is present, the distance between two objects "
  "is computed as geographic distance on this geoid instead of using the "
  "Euclidean distance.\n In detail, the following types are supported:\n\n"
  "  * point:   p1->Distance(*p2, geoid)\n"
  "  * string:  stringutils::ld->(s1->GetValue(), s2->GetValue())\n"
  "  * int:     abs(i1->GetValue() - i2->GetValue())\n"
  "  * real:    abs(r1->GetValue() - r2->GetValue())\n"
  "  * rect<d>: r1->Distance(*r2)\n"
  "  * mpoint:  mp1->DistanceAvg(*mp2, geoid)\n"
  "  * cupoint: cup1->DistanceAvg(*cup2, true, geoid)\n"
  "  * cmpoint: cmp1->DistanceAvg(*cmp2, true, geoid)\n",
  "let kinosM_ntree_GeoData =  kinosM mcreatentree7[GeoData, 5, 8]"
);

Operator mcreatentree7Op(
   "mcreatentree7",
   mcreatentree7Spec.getStr(),
   12,
   mcreatentreeVM<7>,
   mcreatentreeSelect,
   mcreatentreeTM<7>
);

/*
Operator ~mcreatentree8~

*/
OperatorSpec mcreatentree8Spec(
  "MREL(tuple) x attrname x int x int [x geoid] -> , mpointer(mem(mtree X))\n",
  "mrel mcreatentree8[indexAttr, degree, maxLeafSize, [, geoid] ]\n",
  "This operator creates an N-tree8 in main memory. "
  "The first argument is a main memory relation containing the "
  "tuples to be indexed. The second argument refers to the attribute "
  "over that the index is built. The next two arguments represent the degree of"
  " the tree and and maximum number of entries in a leaf.\n"
  "The last argument is optional. It must be of type geoid and "
  "can only be used if the index-attribute is of type point, mpoint, cupoint, "
  "or cmpoint. If this argument is present, the distance between two objects "
  "is computed as geographic distance on this geoid instead of using the "
  "Euclidean distance.\n In detail, the following types are supported:\n\n"
  "  * point:   p1->Distance(*p2, geoid)\n"
  "  * string:  stringutils::ld->(s1->GetValue(), s2->GetValue())\n"
  "  * int:     abs(i1->GetValue() - i2->GetValue())\n"
  "  * real:    abs(r1->GetValue() - r2->GetValue())\n"
  "  * rect<d>: r1->Distance(*r2)\n"
  "  * mpoint:  mp1->DistanceAvg(*mp2, geoid)\n"
  "  * cupoint: cup1->DistanceAvg(*cup2, true, geoid)\n"
  "  * cmpoint: cmp1->DistanceAvg(*cmp2, true, geoid)\n",
  "let kinosM_ntree_GeoData = kinosM mcreatentree8[GeoData, 5, 8]"
);

Operator mcreatentree8Op(
   "mcreatentree8",
   mcreatentree8Spec.getStr(),
   12,
   mcreatentreeVM<8>,
   mcreatentreeSelect,
   mcreatentreeTM<8>
);

/*
operator ~makeNtreePersistent~

*/
ListExpr exportntreeTM(ListExpr args) {
  if (!nl->HasLength(args, 4) && !nl->HasLength(args, 5)) {
    return listutils::typeError("four or five arguments expected");
  }  
  ListExpr a1 = nl->First(args);
  if (MPointer::checkType(a1)) { 
    a1 = nl->Second(a1);
  }
  if (!Mem::checkType(a1)) {
    return listutils::typeError("first arg is not a memory object");
  }
  if (nl->ToString(nl->First(nl->Second(a1))) != "ntree7" &&
      nl->ToString(nl->First(nl->Second(a1))) != "ntree8") {
    return listutils::typeError("first arg must be an ntree7 or an ntree8");
  }
  if (!MPointer::checkType(nl->Second(args))) {
    return listutils::typeError("second arg is not an mpointer");
  }
  // extract tuple type from first argument
  ListExpr mpt = nl->Second(nl->Second(nl->Second(args)));
  if (!Relation::checkType(mpt)) {
    return listutils::typeError("mpointer to a non-relation");
  }
  if (!CcString::checkType(nl->Third(args))) {
    return listutils::typeError("third arg must be a string");  
  }
  if (!CcInt::checkType(nl->Fourth(args))) {
    return listutils::typeError("fourth arg must be an int");
  }
  if (nl->HasLength(args, 5)) {
    if (!CcInt::checkType(nl->Fifth(args))) {
      return listutils::typeError("fifth arg must be an int");
    }
  }
  return nl->SymbolAtom(CcBool::BasicType());
}

template<class T, int variant>
int exportntreeVMT(Word* args, Word& result, int message, Word& local,
                   Supplier s) {
  result = qp->ResultStorage(s);
  CcBool* res = (CcBool*)result.addr;
  MPointer* treeMem = (MPointer*)args[0].addr;
  MPointer* relMem = (MPointer*)args[1].addr;
  if (treeMem->isNull() || relMem->isNull()) {
    res->Set(true, false);
    return 0;
  }
  MemoryRelObject* mrel = (MemoryRelObject*)relMem->GetValue();
  vector<Tuple*>* relVector = mrel->getmmrel();
  CcString *ccPrefix = (CcString*)args[2].addr;
  if (!ccPrefix->IsDefined()) {
    cout << "undefined prefix" << endl;
    res->Set(true, false);
    return 0;
  }
  string prefix = ccPrefix->GetValue();
  CcInt *ccFirstNodeId = (CcInt*)args[3].addr;
  if (!ccFirstNodeId->IsDefined()) {
    cout << "undefined first node id" << endl;
    res->Set(true, false);
    return 0;
  }
  int suffix = -1;
  if (qp->GetNoSons(s) == 5) {
    CcInt *ccSuffix = (CcInt*)args[4].addr;
    if (!ccSuffix->IsDefined()) {
      cout << "undefined suffix" << endl;
      res->Set(true, false);
      return 0;
    }
    suffix = ccSuffix->GetValue();
    if (suffix < 0) {
      cout << "negative suffix invalid" << endl;
      res->Set(true, false);
      return 0;
    }
  }
  Supplier s0 = qp->GetSon(s, 1);
  ListExpr relTypeList = nl->Second(qp->GetType(s0));
  MemoryNtreeObject<T, StdDistComp<T>, variant>* treeObj = 
                                       getNtreeX<MPointer, T, variant>(treeMem);
  NTree<MTreeEntry<T>, StdDistComp<T>, variant> *ntree = treeObj->getNtreeX();  
  PersistentNTree<T, StdDistComp<T>, variant> pntree(ntree, relVector, 
                        relTypeList, prefix, ccFirstNodeId->GetValue(), suffix);
  res->Set(true, pntree.getStatus());  
  return 0;
}

ValueMapping exportntreeVM[] = {
  exportntreeVMT<mtreehelper::t1, 7>,
  exportntreeVMT<mtreehelper::t2, 7>,
  exportntreeVMT<mtreehelper::t3, 7>,
  exportntreeVMT<mtreehelper::t4, 7>,
  exportntreeVMT<mtreehelper::t5, 7>,
  exportntreeVMT<mtreehelper::t6, 7>,
  exportntreeVMT<mtreehelper::t7, 7>,
  exportntreeVMT<mtreehelper::t8, 7>,
  exportntreeVMT<mtreehelper::t9, 7>,
  exportntreeVMT<mtreehelper::t10, 7>,
  exportntreeVMT<mtreehelper::t11, 7>,
  exportntreeVMT<mtreehelper::t12, 7>,
  exportntreeVMT<mtreehelper::t1, 8>,
  exportntreeVMT<mtreehelper::t2, 8>,
  exportntreeVMT<mtreehelper::t3, 8>,
  exportntreeVMT<mtreehelper::t4, 8>,
  exportntreeVMT<mtreehelper::t5, 8>,
  exportntreeVMT<mtreehelper::t6, 8>,
  exportntreeVMT<mtreehelper::t7, 8>,
  exportntreeVMT<mtreehelper::t8, 8>,
  exportntreeVMT<mtreehelper::t9, 8>,
  exportntreeVMT<mtreehelper::t10, 8>,
  exportntreeVMT<mtreehelper::t11, 8>,
  exportntreeVMT<mtreehelper::t12, 8>
};

int exportntreeSelect(ListExpr args) {
  string treeType = nl->SymbolValue(nl->First(nl->Second(nl->Second(nl->First(
                                                                      args)))));
  int offset = (treeType == "ntree7" ? 0 : 12);
  ListExpr a1 = nl->Second(nl->Second(nl->Second(nl->First(args))));
  return mtreehelper::getTypeNo(a1, 12) + offset;
}

OperatorSpec exportntreeSpec(
  "NTREEx(T) x MREL x string x int [x int] -> bool",
  "ntree mrel exportntree[relNamePrefix, firstNodeId]",
  "Creates a persistent structure from an existing main memory N-tree. Four "
  "database relations representing tree information, the tree structure, "
  "distance information, and pivot information are computed. From these, the "
  "tree can be fully reconstructed. The desired id of the first node has to be "
  "specified (3rd argument). The last argument representing a suffix that is "
  "appended to the relation names (e.g., \"kinosNodeInfo_73\") is optional.",
  "query mKinos mcreatentree8[GeoData, 4, 8] mKinos exportntree[\"kinos\", 1]"
);


Operator exportntreeOp(
   "exportntree",
   exportntreeSpec.getStr(),
   24,
   exportntreeVM,
   exportntreeSelect,
   exportntreeTM
);

/*
operator ~importntree~

*/
template<int variant>
ListExpr importntreeTM(ListExpr args) {
  if (!nl->HasLength(args, 2) && !nl->HasLength(args, 3)) {
    return listutils::typeError("two or three arguments expected");
  }
  if (!CcString::checkType(nl->First(args))) {
    return listutils::typeError("first arg must be a string");  
  }
  if (nl->HasLength(args, 3)) {
    if (!CcInt::checkType(nl->Third(args))) {
      return listutils::typeError("third arg must be an int");
    }
  }
  return MPointer::wrapType(Mem::wrapType(nl->TwoElemList(
     listutils::basicSymbol<MemoryNtreeObject<Point, StdDistComp<Point>, 
                                              variant> >(), nl->Second(args))));
}


/*
6.2 Value Mapping template

*/
template<class T, int variant>
int importntreeVMT(Word* args, Word& result, int message, Word& local, 
                   Supplier s) {
  result = qp->ResultStorage(s);
  MPointer* res = (MPointer*)result.addr;
  CcString* ccprefix = (CcString*)args[0].addr;
  if (!ccprefix->IsDefined()) {
    res->setPointer(0);
    return 0;
  }
  string prefix = ccprefix->GetValue();
  int suffix = -1;
  if (qp->GetNoSons(s) == 3) {
    CcInt* ccsuffix = (CcInt*)args[2].addr;
    if (!ccsuffix->IsDefined()) {
      res->setPointer(0);
      return 0;
    }
    suffix = ccsuffix->GetValue();
  }
  PersistentNTree<T, StdDistComp<T>, variant> persNTree(prefix, suffix);
  NTree<MTreeEntry<T>, StdDistComp<T>, variant>* tree = persNTree.getNTree();
  if (tree == 0) {
    res->setPointer(0);
    return 0;
  }
  cout << "entries: " << tree->getNoEntries() << ", nodes: " 
       << tree->getNoNodes() << ", leaves: " << tree->getNoLeaves() << endl;
  size_t usedMem = tree->memSize();
  ListExpr typeList = nl->Second(qp->GetType(s));
  bool flobused = (nl->ToString(typeList) == "mpoint" || 
    nl->ToString(typeList) == "cupoint" || nl->ToString(typeList) == "cmpoint");
  MemoryNtreeObject<T, StdDistComp<T>, variant>* ntree = 
      new MemoryNtreeObject<T, StdDistComp<T>, variant>(tree, usedMem, 
                                nl->ToString(typeList), !flobused, getDBname());
  NTreeStat stat = ntree->getNtreeX()->getStat();
  string counterName = "counterMCreateNTree" + 
                       (variant > 1 ? to_string(variant) : "");
  mtreehelper::increaseCounter(counterName,
                               stat.noDCInnerNodes + stat.noDCLeaves);
  mtreehelper::increaseCounter("noVisitedInnerNodes", stat.noInnerNodes);
  mtreehelper::increaseCounter("noVisitedLeaves", stat.noLeaves);
  if (persNTree.getStatus()) {
    res->setPointer(ntree);
  }
  else {
    res->setPointer(0);
  }
  ntree->deleteIfAllowed();
  return 0;
}

template<int variant>
ValueMapping importntreeVM[] = {
  importntreeVMT<mtreehelper::t1, variant>,
  importntreeVMT<mtreehelper::t2, variant>,
  importntreeVMT<mtreehelper::t3, variant>,
  importntreeVMT<mtreehelper::t4, variant>,
  importntreeVMT<mtreehelper::t5, variant>,
  importntreeVMT<mtreehelper::t6, variant>,
  importntreeVMT<mtreehelper::t7, variant>,
  importntreeVMT<mtreehelper::t8, variant>,
  importntreeVMT<mtreehelper::t9, variant>,
  importntreeVMT<mtreehelper::t10, variant>,
  importntreeVMT<mtreehelper::t11, variant>,
  importntreeVMT<mtreehelper::t12, variant>
};

int importntreeSelect(ListExpr args) {
  return mtreehelper::getTypeNo(nl->Second(args), 12);
}

OperatorSpec importntree7Spec(
  "string x T [x int] -> NTREE7(T)",
  "importntree8(prefix, object [, suffix])",
  "Reconstructs an Ntree8 from previously exported DB relations with the given "
  " prefix and optional suffix. The second argument must be an object of the "
  "same type as the tree objects, whose value is irrelevant.",
  "query importntree7(\"kinos\", [const point value undef])"
);

Operator importntree7Op (
    "importntree7",
    importntree7Spec.getStr(),
    12,
    importntreeVM<7>,
    importntreeSelect,
    importntreeTM<7>
);

OperatorSpec importntree8Spec(
  "string x T -> NTREE8(T)",
  "importntree8(prefix, object)",
  "Reconstructs an Ntree8 from previously exported DB relations with the given "
  " prefix. The second argument must be an object of the same type as the tree "
  " objects, whose value is irrelevant.",
  "query importntree8(\"kinos\", [const point value undef])"
);

Operator importntree8Op (
    "importntree8",
    importntree8Spec.getStr(),
    12,
    importntreeVM<8>,
    importntreeSelect,
    importntreeTM<8>
);


/*
23 Algebra Definition

*/


class MainMemory2Algebra : public Algebra {

    public:
        MainMemory2Algebra() : Algebra() {
          if(!catalog){
             catalog = MemCatalog::getInstance();
          }

/*

8.2 Registration of Types


*/
          AddTypeConstructor (&MPointerTC);
          MPointerTC.AssociateKind( Kind::SIMPLE() );

          AddTypeConstructor (&MemTC);
          MemTC.AssociateKind( Kind::DATA() );
          
/*
8.3 Registration of Operators

*/
          AddOperator (&memloadOp);
          AddOperator (&memloadflobOp);

          AddOperator (&meminitOp);
          meminitOp.SetUsesMemory();

          AddOperator (&mfeedOp);

          AddOperator (&memobjectOp);

          AddOperator (&memgetcatalogOp);
          
          AddOperator (&mcreatertreeOp);

          AddOperator (&memsizeOp);
          AddOperator (&memclearOp);

          AddOperator (&meminsertOp);

          AddOperator (&mwindowintersectsOp);


          AddOperator (&mwindowintersectsSOp);

          AddOperator (&mconsumeOp);
          AddOperator (&mconsumeflobOp);


          AddOperator (&mcreateAVLtreeOp);

          AddOperator (&mexactmatchOp);
          mexactmatchOp.enableInitFinishSupport();

          AddOperator (&mrangeOp);

          AddOperator (&matchbelowOp);

          AddOperator (&matchbelow2Op);

          AddOperator(&mcreatemtreeOp);
          AddOperator(&mcreatemtree2Op);
          AddOperator(&minsertmtreeOp);
          AddOperator(&mdistRange2Op);

          AddOperator(&mdistScan2Op);

          AddOperator(&mdistRangeOp);
          AddOperator(&mdistRangeNOp);
          AddOperator(&mdistRangeN2Op);
          AddOperator(&mdistRangeN5Op);
          AddOperator(&mdistRangeN6Op);
          AddOperator(&mdistRangeN7Op);
          AddOperator(&mdistRangeN8Op);
          AddOperator(&mnearestNeighborN7Op);
          AddOperator(&m1nearestNeighborN7Op);
          AddOperator(&m1nearestNeighborN8Op);

          AddOperator(&mdistScanOp);

          AddOperator(&mexactmatchSOp);

          AddOperator(&mrangeSOp);

          AddOperator(&matchbelowSOp);

          AddOperator(&gettuplesOp);
          
          AddOperator(&mcreatentreeOp);
          AddOperator(&mcreatentree2Op);
          AddOperator(&mcreatentree5Op);
          AddOperator(&mcreatentree6Op);
          AddOperator(&mcreatentree7Op);
          AddOperator(&mcreatentree8Op);
          
          AddOperator(&exportntreeOp);
          AddOperator(&importntree7Op);
          AddOperator(&importntree8Op);
          
  ////////////////////// MainMemory2Algebra////////////////////////////
          
          AddOperator(&mwrapOp);
          mwrapOp.SetUsesArgsInTypeMapping();
          AddOperator(&mwrap2Op);
          mwrap2Op.SetUsesArgsInTypeMapping();
          AddOperator(&mwrap3Op);


          AddOperator(&MTUPLEOp);
          AddOperator(&MTUPLE2Op);
          
          AddOperator(&mcreatettreeOp);

          AddOperator(&minsertttreeOp);

          AddOperator(&mdeletettreeOp);
          AddOperator(&minsertavltreeOp);
          AddOperator(&mdeleteavltreeOp);

          AddOperator(&mcreateinsertrelOp);
          AddOperator(&minsertOp);
          minsertOp.enableInitFinishSupport();
          AddOperator(&minsertsaveOp);
          AddOperator(&minserttupleOp);
          AddOperator(&minserttuplesaveOp);
          
          AddOperator(&mcreatedeleterelOp);
          AddOperator(&mdeleteOp);

          AddOperator(&mdeletesaveOp);
          AddOperator(&mdeletebyidOp);
          AddOperator(&mdeletedirectOp);
          AddOperator(&mdeletedirectsaveOp);

          AddOperator(&mcreateupdaterelOp);
          AddOperator(&mupdateNOp);
          AddOperator(&mupdatesaveOp);

          AddOperator(&mupdatebyidOp);
          mupdatebyidOp.enableInitFinishSupport();
          
          AddOperator(&mupdatedirect2Op);
          
          AddOperator(&moinsertOp);
          AddOperator(&modeleteOp);
          AddOperator(&moconsumeOp);
          AddOperator(&moconsumeflobOp);
          AddOperator(&morangeOp);
          AddOperator(&moleftrangeOp);
          AddOperator(&morightrangeOp);
          
          AddOperator(&moshortestpathdOp);
          AddOperator(&moshortestpathaOp);
          
          AddOperator(&moconnectedcomponentsOp);
          
          AddOperator(&mquicksortOp);
          AddOperator(&mquicksortbyOp);
          
          AddOperator (&mcreatemgraphOp);
          AddOperator (&mcreatemgraphflobOp);
          
          AddOperator(&mgshortestpathdOp);
          AddOperator(&mgshortestpathaOp);
          AddOperator(&mgconnectedcomponentsOp);
          AddOperator(&mgconnectedcomponentsNOp);
          AddOperator(&mgconnectedcomponents_oldOp);
        
          AddOperator(&pwrapOp);
          pwrapOp.SetUsesArgsInTypeMapping();
          

     // operations on mvector
          AddOperator(&collect_mvectorOp);
          AddOperator(&sizemvOp);
          AddOperator(&getmvOp);
          AddOperator(&putmvOp);
          AddOperator(&isSortedmvOp);
          AddOperator(&sortmvOp);
          AddOperator(&feedmvOp);
          AddOperator(&findmvOp);
          AddOperator(&matchbelowmvOp);
          AddOperator(&insertmvOp);
          AddOperator(&countOp);

    // operators on priority queues
          AddOperator(&mcreatepqueueOp);
          AddOperator(&mcreatepqueueflobOp);

          AddOperator(&sizeOp);
          AddOperator(&mfeedpqOp);
          AddOperator(&mfeedpqSizeOp);
          AddOperator(&mfeedpqAbortOp);

          AddOperator(&minsertTuplepqOp);
          AddOperator(&minserttuplepqprojectOp);
          AddOperator(&minserttuplepqprojectUOp);

          AddOperator(&mpqreorderOp);
          AddOperator(&mpqreorderupdateOp);

     // operators on stack
          AddOperator(&mfeedstackOp);

          AddOperator(&mcreatestackOp);
          AddOperator(&mcreatestackflobOp);
          AddOperator(&stacksizeOp);
          AddOperator(&insertmstackOp);

          AddOperator(&mblockOp);

     // operators on mgraph2

          AddOperator(&createmgraph2Op);
          AddOperator(&createmgraph2flobOp);
          AddOperator(&mg2insertorigOp);
          AddOperator(&mg2insertOp);
          AddOperator(&mg2feedOp);
          AddOperator(&mg2nodemapOp);
          AddOperator(&mg2numverticesOp);
          AddOperator(&mg2successorsOp);
          AddOperator(&mg2predecessorsOp);
          AddOperator(&mg2numsuccessorsOp);
          AddOperator(&mg2numpredecessorsOp);
          AddOperator(&mg2disconnectOp);
          AddOperator(&mg2deleteEdgesOp);
          AddOperator(&mg2contractOp);
          AddOperator(&mg2minPathCostOp);
          AddOperator(&mg2exportddsgOp);
          AddOperator(&mg2connectedcomponentsOp);
          mg2connectedcomponentsOp.enableInitFinishSupport();
          AddOperator(&mg2connectedcomponentsNOp);
          mg2connectedcomponentsNOp.enableInitFinishSupport();
           
          // operators on mgraph3
          AddOperator(&createmgraph3Op);
          AddOperator(&createmgraph3flobOp);

          AddOperator(&mg3insertOp);
          AddOperator(&mg3feedOp);
          AddOperator(&mg3numverticesOp);
          AddOperator(&mg3successorsOp);
          AddOperator(&mg3predecessorsOp);
          AddOperator(&mg3numsuccessorsOp);
          AddOperator(&mg3numpredecessorsOp);
          AddOperator(&mg3disconnectOp);
          AddOperator(&mg3deleteEdgesOp);
          AddOperator(&mg3connectedcomponentsOp);
          mg3connectedcomponentsOp.enableInitFinishSupport();
          AddOperator(&mg3connectedcomponentsNOp);
          mg3connectedcomponentsNOp.enableInitFinishSupport();
          
          AddOperator(&mg3contractOp);
          AddOperator(&mg3minPathCostOp);
          AddOperator(&mg3exportddsgOp);

          AddOperator(&mgraphPrintOp);
          AddOperator(&mgraph2textOp);

          AddOperator(&mgroupOp);
          AddOperator(&memgroupbyOp);
          memgroupbyOp.enableInitFinishSupport();

          AddOperator(&importCHOp),
          importCHOp.SetUsesArgsInTypeMapping();

          AddOperator(&mmergejoinprojectOp);

        }
        
        ~MainMemory2Algebra() {
            MemCatalog::destroyInstance();
        };
};

} // end of namespace mmalgebra

/*
9 Initialization

Each algebra module needs an initialization function. The algebra manager
has a reference to this function if this algebra is included in the list
of required algebras, thus forcing the linker to include this module.

The algebra manager invokes this function to get a reference to the instance
of the algebra class and to provide references to the global nested list
container (used to store constructor, type, operator and object information)
and to the query processor.

The function has a C interface to make it possible to load the algebra
dynamically at runtime (if it is built as a dynamic link library). The name
of the initialization function defines the name of the algebra module. By
convention it must start with "Initialize<AlgebraName>".

To link the algebra together with the system you must create an
entry in the file "makefile.algebra" and to define an algebra ID in the
file "Algebras/Management/AlgebraList.i.cfg".

*/

extern "C"
Algebra*
InitializeMainMemory2Algebra(NestedList* nlRef, QueryProcessor* qpRef)
{
  return (new mm2algebra::MainMemory2Algebra);
}

