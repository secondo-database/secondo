/*

----
This file is part of SECONDO.

Copyright (C) 2004, University in Hagen, Department of Computer Science,
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
#include "RelationAlgebra.h"
#include "OrderedRelationAlgebra.h"
#include "GraphAlgebra.h"
#include "Stream.h"
#include "MMRTree.h"
#include "MMMTree.h"
#include "FTextAlgebra.h"
#include "MovingRegionAlgebra.h"
#include "RectangleAlgebra.h"
#include "AvlTree.h"

#include "MainMemoryExt.h"
#include "ttree.h"
// #include "mapmatch.h"


#include <ctime>
#include <exception>
#include <string>
#include <map>
#include <vector>
#include <limits>
#include <algorithm>

#include <ctime>

using namespace std;

extern NestedList* nl;
extern QueryProcessor *qp;
extern SecondoSystem* instance;


ostream& operator<<(ostream& o, const mm2algebra::avlPair& t) {
  o << "(";
  t.first->Print(o);
  o << ", " << t.second   << ")";
  return o;
}


namespace mm2algebra {

MemCatalog* catalog;

#define MEMORYMTREEOBJECT "memoryMTreeObject"


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



}

namespace mtreehelper{

  double distance(const Point* p1, const Point* p2) {
     if(!p1->IsDefined() && !p2->IsDefined()){
       return 0;
     }
     if(!p1->IsDefined() || !p2->IsDefined()){
         return std::numeric_limits<double>::max();
     } 
     return p1->Distance(*p2);
  }

  double distance(const CcString* s1, const CcString* s2) {
     if(!s1->IsDefined() && !s2->IsDefined()){
        return 0;
     }
     if(!s1->IsDefined() || !s2->IsDefined()){
        return std::numeric_limits<double>::max();
     }
     return stringutils::ld(s1->GetValue() ,s2->GetValue());
  }

  double distance(const CcInt* i1, const CcInt* i2) {
     if(!i1->IsDefined() && !i2->IsDefined()){
        return 0;
     }
     if(!i1->IsDefined() || !i2->IsDefined()){
        return std::numeric_limits<double>::max();
     }
     return abs(i1->GetValue() - i2->GetValue());
  }
  
  double distance(const CcReal* r1, const CcReal* r2) {
     if(!r1->IsDefined() && !r2->IsDefined()){
        return 0;
     }
     if(!r1->IsDefined() || !r2->IsDefined()){
        return std::numeric_limits<double>::max();
     }
     return abs(r1->GetValue() - r2->GetValue());
  }

  template<unsigned int dim>
  double distance(const Rectangle<dim>* r1, 
                  const Rectangle<dim>* r2) {
     if(!r1->IsDefined() && !r2->IsDefined()){
        return 0;
     }
     if(!r1->IsDefined() || !r2->IsDefined()){
        return std::numeric_limits<double>::max();
     }
     return r1->Distance(*r2);
  }



/*
   6.4 ~getTypeNo~

   Returns a number for supported types, -1 if not supported.

*/
  typedef Point t1;
  typedef CcString t2;
  typedef CcInt t3;
  typedef CcReal t4;
  typedef Rectangle<1> t5;
  typedef Rectangle<2> t6;
  typedef Rectangle<3> t7;
  typedef Rectangle<4> t8;
  typedef Rectangle<8> t9;

  int getTypeNo(ListExpr type, int expectedNumbers){
     assert(expectedNumbers==9);
     if( t1::checkType(type)){ return 0;}
     if( t2::checkType(type)){ return 1;}
     if( t3::checkType(type) ){ return 2;}
     if( t4::checkType(type)){ return 3; }
     if( t5::checkType(type)){ return 4;}
     if( t6::checkType(type)){ return 5;}
     if( t7::checkType(type)){ return 6;}
     if( t8::checkType(type)){ return 7;}
     if( t9::checkType(type)){ return 8;}
     return -1;
  }

  string BasicType(){
    return  "mtree";
  }

  bool checkType(ListExpr type, ListExpr subtype){
    if(!nl->HasLength(type,2)){
       return  false;
    }
    if(!listutils::isSymbol(nl->First(type), BasicType())){
       return false;
    }
    if(getTypeNo(subtype, 9) < 0){
      return false;
    }
    return nl->Equal(nl->Second(type), subtype);
  }


}


template<class T>
class StdDistComp{
  public:
    double  operator()(const pair<T,TupleId>&  o1, 
                       const pair<T,TupleId>& o2){
       return mtreehelper::distance(&o1.first,&o2.first);
    }

    ostream& print( const pair<T, TupleId> & p,  ostream& o){
       o << "<"; p.first.Print(o); o << p.second << ">";
       return o;
    }
  
    void reset(){} // not sure
};


namespace ttreehelper{

  string BasicType(){
    return  "ttree";
  }
  
  bool checkType(ListExpr type){
    if(!nl->HasLength(type,2)){
      return false;
    }
    if(!listutils::isSymbol(nl->First(type),BasicType())){
      return false;
    }
    return true;
  }
} // end namespace ttreehelper


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
   if(!catalog->isMMObject(treen) || !catalog->isAccessible(treen)){
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
MemoryRelObject* getMemRel(T* relN){
  if(!relN->IsDefined()) { return 0; }
  string reln = relN->GetValue();
  if(!catalog->isMMObject(reln) || !catalog->isAccessible(reln)) {
    return 0;
  }
  ListExpr relType = nl->Second(catalog->getMMObjectTypeExpr(reln));
    
  if(!Relation::checkType(relType)) { return 0; }
  
  return (MemoryRelObject*) catalog->getMMObject(reln);
}


template<class T>
MemoryRelObject* getMemRel(T* relN, ListExpr tupleType) {
  if(!relN->IsDefined()) { return 0; }
  
  string reln = relN->GetValue();
  if(!catalog->isMMObject(reln) || !catalog->isAccessible(reln)){
    return 0;
  }
  ListExpr relType = nl->Second(catalog->getMMObjectTypeExpr(reln));
    
  if(!Relation::checkType(relType)){
     return 0;
  }
  if(!nl->Equal(nl->Second(relType), tupleType)){
     return 0;
  }
  return (MemoryRelObject*) catalog->getMMObject(reln);
}


template<class T>
MemoryORelObject* getMemORel(T* relN, ListExpr tupleType){
  if(!relN->IsDefined()){
     return 0;
  }
  string reln = relN->GetValue();
  if(!catalog->isMMObject(reln) || !catalog->isAccessible(reln)){
    return 0;
  }
  ListExpr relType = nl->Second(catalog->getMMObjectTypeExpr(reln));

  if(!listutils::isOrelDescription(relType)){
     return 0;
  }
  return (MemoryORelObject*) catalog->getMMObject(reln);
}

template<class T>
MemoryGraphObject* getMemGraph(T* aN){
  if(!aN->IsDefined()){
     return 0;
  }
  string an = aN->GetValue();
  if(!catalog->isMMObject(an) || !catalog->isAccessible(an)){
    return 0;
  }
  ListExpr type = nl->Second(catalog->getMMObjectTypeExpr(an));
  if(!MemoryGraphObject::checkType(type)){
    return 0;
  }
  
  return (MemoryGraphObject*) catalog->getMMObject(an);
}


template<class T>
MemoryAttributeObject* getMemAttribute(T* aN, ListExpr _type){
  if(!aN->IsDefined()){
     return 0;
  }
  string an = aN->GetValue();
  if(!catalog->isMMObject(an) || !catalog->isAccessible(an)){
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


template<class T, int dim>
MemoryRtreeObject<dim>* getRtree(T* tN){
  if(!tN->IsDefined()){
    return 0;
  }
  string tn = tN->GetValue();
  if(!catalog->isMMObject(tn)){ 
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

template<class T, class K>
MemoryMtreeObject<K, StdDistComp<K> >* getMtree(T* name){
  if(!name->IsDefined()){
    return 0;
  }
  string n = name->GetValue();
  if(!catalog->isMMObject(n)){
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

template<class T>
MemoryTTreeObject* getTtree(T* treeN){
   if(!treeN->IsDefined()){
      return 0;
   }
   string treen = treeN->GetValue();
   if(!catalog->isMMObject(treen) || !catalog->isAccessible(treen)){
     return 0;
   }
   ListExpr treet = nl->Second(catalog->getMMObjectTypeExpr(treen));
   if(!MemoryTTreeObject::checkType(treet)){
     return 0;
   }
   return (MemoryTTreeObject*)catalog->getMMObject(treen);
}


/*
Function returning the current database name.

*/

string getDBname() {
    SecondoSystem* sys = SecondoSystem::GetInstance();
    return sys->GetDatabaseName();
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
bool getMemType(ListExpr type, ListExpr value, 
                ListExpr & result, string& error){

    if(Mem::checkType(type)){
        result = type;
        return true;
    }
    if(!CcString::checkType(type)){
       error = "not of type mem or string";
       return false;
    }
    if(nl->AtomType(value)!=StringType){
       error="only constant strings are supported";
       return false;
    }
    string n = nl->StringValue(value);
    if(!catalog->isMMObject(n)){
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
2 Operator ~memload~

This operator transfers a DB object into the main memory.
The object can be of type rel, orel or must be in kind DATA.


The operator supports two variants. One variant loads also the 
flobs into the memory, the other variant does not touches the flobs.
The next function supports both variants.

*/
int memload(Word* args, Word& result,
            int message, Word& local, Supplier s, bool flob) {

    result  = qp->ResultStorage(s);
    CcBool* b = static_cast<CcBool*>(result.addr);

    CcString* oN = (CcString*) args[0].addr;
    if(!oN->IsDefined()){
       b->Set(true,false);
       return 0;
    }

    string objectName = oN->GetValue();
    if(catalog->isMMObject(objectName)){
       // name already exist
       b->Set(true,false);
       return 0;
    }

    SecondoCatalog* cat = SecondoSystem::GetCatalog();
    bool memloadSucceeded=false;
    Word object; //save the persistent object
    ListExpr objectTypeExpr=0; //type expression of the persistent object
    string objectTypeName=""; //used by cat->GetObjectTypeExpr
    bool defined = false;
    bool hasTypeName = false;

    memloadSucceeded = cat->GetObjectExpr(
                objectName,
                objectTypeName,
                objectTypeExpr,
                object,
                defined,
                hasTypeName);

     if(!memloadSucceeded){
       // object not found
       b->Set(true,false);
       return 0;
     }

    // object is a relation
    if(Relation::checkType(objectTypeExpr)&&defined) {
        GenericRelation* r= static_cast<Relation*>( object.addr );
        MemoryRelObject* mmRelObject = new MemoryRelObject();
        memloadSucceeded = mmRelObject->relToVector(
                                          r,
                                          objectTypeExpr,
                                          getDBname(), flob);
        if (memloadSucceeded) {
            catalog->insert(objectName,mmRelObject);
        } else {
            delete mmRelObject;
        }
        delete r;

    // object is an ordered relation
    } else if(listutils::isOrelDescription(objectTypeExpr)&&defined) {
        GenericRelation* r= static_cast<OrderedRelation*>(object.addr);
        MemoryORelObject* mmORelObject = new MemoryORelObject();
        memloadSucceeded = mmORelObject->relToTree(
                                          r,
                                          objectTypeExpr,
                                          getDBname(), flob);
        if (memloadSucceeded) {
            catalog->insert(objectName,mmORelObject);
        } else {
            delete mmORelObject;
        }
        delete r;
        
    } else if (Attribute::checkType(objectTypeExpr)&&defined){
        Attribute* attr = (Attribute*)object.addr;
        MemoryAttributeObject* mmA = new MemoryAttributeObject();
        memloadSucceeded = mmA->attrToMM(attr, objectTypeExpr,
                                getDBname(),flob);
        if (memloadSucceeded){
          catalog->insert(objectName, mmA);
        }
        else {
          delete mmA;
        }
        attr->DeleteIfAllowed();
    } else {
       // only attributes and relations are supported
       b->Set(true,false);
       return 0;
    }

    b->Set(true, memloadSucceeded);
    return 0;
}



/*

5 Creating Operators

5.1 Operator ~memload~

Load a persistent relation into main memory. If there is not enough space
it breaks up. The created ~MemoryRelObject~ or ~MemoryORelOBject~
is usable but not complete.

5.1.1 Type Mapping Functions of operator ~memload~ (string -> bool)


*/
ListExpr memloadTypeMap(ListExpr args) {

    if (nl->ListLength(args)!=1){
        return listutils::typeError("wrong number of arguments");
    }
    ListExpr arg1 = nl->First(args);
    if (!CcString::checkType(nl->First(arg1))) {
        return listutils::typeError("string expected");
    }
    //check for database object
    ListExpr str = nl->Second(arg1);
    string objectName = nl->StringValue(str);
    SecondoCatalog* cat = SecondoSystem::GetCatalog();

    if (!cat->IsObjectName(objectName)){
        return listutils::typeError("identifier is not in use");
    }

    //already main memory object?
    if (catalog->isMMObject(objectName)){
        return listutils::typeError("identifier is "
        " already used for a main memory object");
    }
    return listutils::basicSymbol<CcBool>();
}


/*
5.1.3  The Value Mapping Functions of operator ~memload~

*/
int memloadValMap (Word* args, Word& result,
                    int message, Word& local, Supplier s) {

    return memload(args, result, message, local, s, false);

}


/*

5.1.4 Description of operator ~memload~

*/
OperatorSpec memloadSpec(
    "string -> bool",
    "memload(_)",
    "loads a persistent object to main memory (without flobs) "
    "if there is not enough space, the loaded object may be not complete "
    "but usable",
    "query memload('plz')"
);

/*

5.1.5 Instance of operator ~memload~

*/

Operator memloadOp (
    "memload",
    memloadSpec.getStr(),
    memloadValMap,
    Operator::SimpleSelect,
    memloadTypeMap
);

/*
5.2 Operator ~memloadflob~

Like ~memload~ but loads also the flob part into the main memory

*/

/*
5.2.3  The Value Mapping Functions of operator ~memloadflob~

*/


int memloadflobValMap (Word* args, Word& result,
                    int message, Word& local, Supplier s) {

    return memload(args, result, message, local, s, true);

}


/*

5.2.4 Description of operator ~memloadflob~

*/

OperatorSpec memloadflobSpec(
    "string -> bool",
    "memloadflob(_)",
    "loads a persistent object together with the associated flobs to  "
    "main memory. If there is not enough space, the loaded object "
    "may be not complete but usable",
    "query memloadflob('Trains')"
);

/*

5.2.5 Instance of operator ~memloadflob~

*/

Operator memloadflobOp (
    "memloadflob",
    memloadflobSpec.getStr(),
    memloadflobValMap,
    Operator::SimpleSelect,
    memloadTypeMap
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
        cout<< "the size must be >0"<<endl;
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

    ListExpr arg = nl->First(args);
    
    if(!nl->HasLength(arg,2)){
        return listutils::typeError("internal error");
    }
    string errMsg;

    if(!getMemType(nl->First(arg), nl->Second(arg), arg, errMsg)){
      return listutils::typeError("string or mem(rel) expected : " + errMsg);
    }

    arg = nl->Second(arg); // remove mem
    if((!Relation::checkType(arg)) && (!listutils::isOrelDescription(arg))){
      return listutils::typeError(
        "memory object is not a relation or ordered relation");
    }
    return nl->TwoElemList(nl->SymbolAtom(Symbol::STREAM()),
                           nl->Second(arg));
}


/*

5.4.3  The Value Mapping Functions of operator ~mfeed~

*/


class mfeedInfo{
  public:
    mfeedInfo(vector<Tuple*>* _rel) : rel(_rel) {
        isOrel = false;
        it = rel->begin();
    }
    
    mfeedInfo(ttree::TTree<Tuple*,TupleComp>* _orel) : orel(_orel) {
      iter = orel->begin();
      isOrel = true;
    }
    
    ~mfeedInfo() {}

    Tuple* next() {
      
      Tuple* res;
      if(!isOrel) {
        if(it==rel->end()) return 0;
        res = *it;
        it++;
      }
      else {
        if(!iter.hasNext()) return 0;
        res = *iter;
        iter++;
      }
       if(res==NULL) {
        return 0;
       }
       res->IncReference();
       return res;
     }

  private:
     vector<Tuple*>* rel;
     vector<Tuple*>::iterator it;
     ttree::TTree<Tuple*,TupleComp>* orel;
     ttree::Iterator<Tuple*,TupleComp> iter;
     bool isOrel;
};


template<class T>
int mfeedValMap (Word* args, Word& result,
                    int message, Word& local, Supplier s) {

  mfeedInfo* li = (mfeedInfo*) local.addr;

  switch (message) {
    
    case OPEN: {
      
      if(li) {
        delete li;
        local.addr=0;
      }
      
      T* oN = (T*) args[0].addr;
      
      ListExpr type = catalog->getMMObjectTypeExpr(oN->GetValue());
      
      // check for rel
      if(Relation::checkType(nl->Second(type))) {  
        MemoryRelObject* rel = getMemRel(oN);
        if(!rel) {
          return 0;
        }
        local.addr= new mfeedInfo(rel->getmmrel());
        return 0;
      }
      // check for orel
      else if(listutils::isOrelDescription(nl->Second(type))) { 
        MemoryORelObject* orel = getMemORel(oN, nl->Second(qp->GetType(s)));
        if(!orel) {
          return 0;
        }
        local.addr= new mfeedInfo(orel->getmmorel());
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


ValueMapping mfeedVM[] = {
  mfeedValMap<CcString>,
  mfeedValMap<Mem>
};

int mfeedSelect(ListExpr args) {
  return CcString::checkType(nl->First(args))?0:1;
}


/*

5.4.4 Description of operator ~mfeed~


*/

OperatorSpec mfeedSpec(
    "{string, mem(rel(tuple(x))}  -> stream(Tuple)",
    "_ mfeed",
    "produces a stream from a main memory (ordered) relation",
    "query 'ten' mfeed"
);

/*

5.4.5 Instance of operator ~mfeed~

*/

Operator mfeedOp (
    "mfeed",
    mfeedSpec.getStr(),
    2,
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
  string err ="stream(tid) x {string, mem(rel(tuple))} expected";
  if(!nl->HasLength(args,2)){
    return listutils::typeError(err + " (wrong number of args)");
  }
  ListExpr a1 = nl->First(nl->First(args));
  if(!Stream<TupleIdentifier>::checkType(a1)){
    return listutils::typeError(err + " (1st arg is not a tid stream)");
  }
  ListExpr a2 = nl->Second(args);
  string errMsg;
  if(!getMemType(nl->First(a2), nl->Second(a2), a2, errMsg)){
     return listutils::typeError(err + "\nproblem in 2nd arg: " + errMsg);
  }
  a2 = nl->Second(a2);
  if(!Relation::checkType(a2)){
    return listutils::typeError(err + " (second arg is not a "
                                "memory relation.)");
  }
  return nl->TwoElemList( listutils::basicSymbol<Stream<Tuple> >(),
                          nl->Second(a2));
  
}


class gettuplesInfo{
  public:
    gettuplesInfo(Word _stream, MemoryRelObject* _rel): 
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
             cout << "ignore undefined tuple id" << endl;
             tid->DeleteIfAllowed();
           } else {
              TupleId id = tid->GetTid();
              tid->DeleteIfAllowed();
              if(id>=0 && id<rel->size()){
                 Tuple* res = (*rel)[id];
                 res->IncReference();
                 res->SetTupleId(id);
                 return res;
              } else {
                 cout << "ignore id " << id << endl;
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

ValueMapping gettuplesVM[] =  {
   gettuplesVMT<CcString>,
   gettuplesVMT<Mem>
};

int gettuplesSelect(ListExpr args){
  return CcString::checkType(nl->Second(args))?0:1;
}

OperatorSpec gettuplesSpec(
  "stream(tid) x {string, mem(rel(tuple(X)))} -> stream(tuple(X))",
  "_ _ gettuples",
  "Retrieves tuples from a main memory relation whose ids are "
  "specified in the incoming stream.",
  "query \"strassen_Name\" mdistScan2[\"str\"] \"strassen\" gettuples consume"
);

Operator gettuplesOp(
  "gettuples",
  gettuplesSpec.getStr(),
  2,
  gettuplesVM,
  gettuplesSelect,
  gettuplesTM
);


template<bool flob>
int letmconsumeValMap(Word* args, Word& result,
                int message, Word& local, Supplier s) {

    result  = qp->ResultStorage(s);
    Mem* str = (Mem*)result.addr;

    Supplier t = qp->GetSon( s, 0 );
    ListExpr le = qp->GetType(t);
    bool succeed;
    CcString* oN = (CcString*) args[1].addr;
    if(!oN->IsDefined()){
        str->SetDefined(false);
        return 0;
    }
    string res = oN->GetValue();
    if(catalog->isMMObject(res)){
        str->SetDefined(false);
        return 0;
    };

    MemoryRelObject* mmRelObject = new MemoryRelObject();
    succeed = mmRelObject->tupleStreamToRel(args[0],
        le, getDBname(), flob);
    if (succeed) {
        catalog->insert(res,mmRelObject);
    } else {
        delete mmRelObject;
    }
    str->set(succeed,res);

   return 0;
}


/*

5.5 Operator ~letmconsume~

~letmconsume~ produces a main memory relation from a stream(tuples),
similar to the ~consume~-operator. The name of the main memory relation 
is given by the second parameter.

*/

/*

5.5.1 Type Mapping Functions of operator ~letmconsume~
        (stream(Tuple) x string -> mem(rel(tuple(X))))

*/
ListExpr letmconsumeTypeMap(ListExpr args)
{
    if(nl->ListLength(args)!=2){
        return listutils::typeError("(wrong number of arguments)");
    }

    if (!Stream<Tuple>::checkType(nl->First(args))
        || !CcString::checkType(nl->Second(args)) ) {
        return listutils::typeError ("stream(Tuple) x string expected!");
    }
    return nl->TwoElemList(
                listutils::basicSymbol<Mem>(),
                nl->TwoElemList(
                    listutils::basicSymbol<Relation>(),
                    nl->Second(nl->First(args))));
}


/*

5.5.4 Description of operator ~letmconsume~

*/

OperatorSpec letmconsumeSpec(
    "stream(Tuple) x string -> mem(rel(tuple(X)))",
    "_ letmconsume [_]",
    "produces a main memory relation from a stream(Tuple)",
    "query ten feed letmconsume ['zehn']"
);



/*

5.5.5 Instance of operator ~letmconsume~

*/

Operator letmconsumeOp (
    "letmconsume",
    letmconsumeSpec.getStr(),
    letmconsumeValMap<false>,
    Operator::SimpleSelect,
    letmconsumeTypeMap
);


/*

5.5.4 Description of operator ~letmconsume~

*/

OperatorSpec letmconsumeflobSpec(
    "stream(Tuple) x string -> mem(relType(tuple(X)))",
    "_ letmconsumeflob [_]",
    "produces a main memory relation from a stream(Tuple)"
    "and load the associated flobs",
    "query trains feed letmconsumeflob ['trains1']"
);



/*

5.5.5 Instance of operator ~letmconsumeflob~

*/

Operator letmconsumeflobOp (
    "letmconsumeflob",
    letmconsumeflobSpec.getStr(),
    letmconsumeValMap<true>,
    Operator::SimpleSelect,
    letmconsumeTypeMap
);

/*
5.6 Operator ~memdelete~

~memdelete~ deletes an object from main memory

5.6.1 Type Mapping Functions of operator ~memdelete~ (string -> bool)

*/
ListExpr memdeleteTypeMap(ListExpr args)
{
 if(nl->ListLength(args)!=1){
        return listutils::typeError("wrong number of arguments");
    }

    ListExpr arg = nl->First(args);

    if(!nl->HasLength(arg,2)){
        return listutils::typeError("internal error");
    }

    if (!CcString::checkType(nl->First(arg))) {
        return listutils::typeError("string expected");
    };

    ListExpr fn = nl->Second(arg);

    if(nl->AtomType(fn)!=StringType){
        return listutils::typeError("error");
    }

    string oN = nl->StringValue(fn);

    if(!catalog->isMMObject(oN))
    {
      return listutils::typeError("not a MainMemory member");
    }
     return listutils::basicSymbol<CcBool>();
}


/*

5.6.3  The Value Mapping Functions of operator ~memdelete~

*/


int memdeleteValMap (Word* args, Word& result,
                int message, Word& local, Supplier s) {
    result  = qp->ResultStorage(s);
    CcBool* b = static_cast<CcBool*>(result.addr);
    bool deletesucceed = false;
    CcString* oN = (CcString*) args[0].addr;
    if(!oN->IsDefined()){
            b->Set(true, deletesucceed);
            return 0;
        }
    string objectName = oN->GetValue();
    deletesucceed = catalog->deleteObject(objectName);

    b->Set(true, deletesucceed);
    return 0;

}


/*

5.6.4 Description of operator ~memdelete~

Similar to the ~property~ function of a type constructor, an operator needs to
be described, e.g. for the ~list operators~ command.  This is now done by
creating a subclass of class ~OperatorInfo~.

*/

OperatorSpec memdeleteSpec(
    "string -> bool",
    "memdelete (_)",
    "deletes a main memory object",
    "query memdelete ('ten')"
);



/*

5.6.5 Instance of operator ~memdelete~

*/

Operator memdeleteOp (
    "memdelete",
    memdeleteSpec.getStr(),
    memdeleteValMap,
    Operator::SimpleSelect,
    memdeleteTypeMap
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

    if(!nl->HasLength(arg1,2)){
        return listutils::typeError("internal error");
    }

    string err = "string or mem(X) expected";
     
    string errMsg;
    if(!getMemType(nl->First(arg1), nl->Second(arg1), arg1, errMsg)){
      return listutils::typeError(err + "(" + errMsg +")");
    }
    ListExpr subtype = nl->Second(arg1);
    if(   !Attribute::checkType(subtype)
       && !Relation::checkType(subtype)){
         return listutils::typeError("only rel and DATA supported");
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
    "string | mem(X)  -> m:MEMLOADABLE , X in {DATA, rel}",
    "memobject (_)",
    "returns a persistent object created from a main memory object",
    "query memobject (\"Trains100\")"
);

/*
5.7.5 Value Mapping Array and Selection

*/
ValueMapping memobjectVM[] = {
   memobjectValMap<CcString>,
   memobjectValMap<Mem>
};

int memobjectSelect(ListExpr args){
  return CcString::checkType(nl->First(args))?0:1;
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
            "(Database string)(Accessible bool)(FlobsLoaded bool))))";

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
        resultType = _resultType;
        memContents = catalog->getMemContent();
        it = memContents->begin();
        };
        ~memgetcatalogInfo(){}

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

        TupleType* tt = new TupleType(nl->Second(resultType));
        Tuple *tup = new Tuple( tt );
        tt->DeleteIfAllowed();

        CcInt* totalMB = new CcInt (true, catalog->getMemSizeTotal());
        CcReal* usedMB =
            new CcReal (true, (double)catalog->getUsedMemSize()/1024.0/1024.0);
        CcString* objectName = new CcString(true,name);
        FText* oT = new FText(true,objTyp);
        CcString* memSizeB = new CcString
                            (true, long_to_string(memobj->getMemSize()));
        CcReal* memSizeMB =
            new CcReal(true, (double)memobj->getMemSize()/1024.0/1024.0);
        CcString* database = new CcString(true,(string)memobj->getDatabase());
        CcBool* accessible =
                new CcBool(true, (bool)(memobj->getDatabase()==getDBname()
                    || memobj->hasflob()));
        CcBool* flobs = new CcBool(true, (bool)memobj->hasflob());

        tup->PutAttribute(0,totalMB);
        tup->PutAttribute(1,usedMB);
        tup->PutAttribute(2,objectName);
        tup->PutAttribute(3,oT);
        tup->PutAttribute(4,memSizeB);
        tup->PutAttribute(5,memSizeMB);
        tup->PutAttribute(6,database);
        tup->PutAttribute(7,accessible);
        tup->PutAttribute(8,flobs);

        it++;
        return tup;
    }
 private:

       map<string, MemoryObject*>* memContents;
       map<string, MemoryObject*>::iterator it;
       ListExpr resultType;

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



int memlet(Word* args, Word& result,
           int message, Word& local, Supplier s, bool flob) {

    result  = qp->ResultStorage(s);
    CcBool* b = static_cast<CcBool*>(result.addr);
    bool memletsucceed = false;
    bool correct = true;

    CcString* oN = (CcString*) args[0].addr;
    if(!oN->IsDefined()){
        return 0;
    }
    string objectName = oN->GetValue();
    Supplier t = qp->GetSon( s, 1 );
    ListExpr le = qp->GetType(t);

    // relation
    if(Relation::checkType(le)) {
        GenericRelation* rel = static_cast<Relation*>(args[1].addr);
        MemoryRelObject* mmRelObject = new MemoryRelObject();
        memletsucceed = mmRelObject->relToVector(rel,le, 
                                                 getDBname(),flob);
        if(memletsucceed) 
          catalog->insert(objectName,mmRelObject);
        else 
          delete mmRelObject;
        
    }
    // main memory relation
    else if(Relation::checkType(nl->Second(le))) {
        CcString* oN = static_cast<CcString*>(args[1].addr);
        MemoryRelObject* rel = getMemRel(oN);
        if(!rel){
          cerr << "no relation with by that name in main memory catalog" 
               << endl;
          rel = new MemoryRelObject(); 
        }
        MemoryRelObject* mmRelObject = new MemoryRelObject();
        memletsucceed = mmRelObject->mmrelToVector(rel->getmmrel(),le, 
                                                   getDBname(),flob);
        if(memletsucceed) 
          catalog->insert(objectName,mmRelObject);
        else 
          delete mmRelObject;
        delete rel;
    } 
    // ordered relation
    else if(listutils::isOrelDescription(le)) {
        GenericRelation* orel= static_cast<OrderedRelation*>(args[1].addr);
        MemoryORelObject* mmORelObject = new MemoryORelObject();
        memletsucceed = mmORelObject->relToTree(orel,le,
                                                getDBname(),flob);
        if(memletsucceed) 
            catalog->insert(objectName,mmORelObject);
        else 
          delete mmORelObject;
        
    }
    // attribute
    else if(Attribute::checkType(le)) {
      Attribute* attr = (Attribute*)args[1].addr;
      MemoryAttributeObject* mmA = new MemoryAttributeObject();
      memletsucceed = mmA->attrToMM(attr, le,
                            getDBname(),flob);
      if(memletsucceed) {
        catalog->insert(objectName, mmA);
      }
      else {
        delete mmA;
      }
    } 
    // stream
    else if(Stream<Tuple>::checkType(le)){
        MemoryRelObject* mmRelObject = new MemoryRelObject();
        memletsucceed = mmRelObject->tupleStreamToRel(args[1],
                                                      le, 
                                                      getDBname(), 
                                                      flob);
        if (memletsucceed) {
            catalog->insert(objectName,mmRelObject);
        } else {
            delete mmRelObject;
        }
    } else {
      correct = false;
    }

    b->Set(correct, memletsucceed);

    return 0;

}


/*
5.9 Operator ~memlet~
creates a new main memory object. The first parameter is the
name of the new main memory object, the second is the query/the
MEMLOADABLE object from which the mm-object will be created.

5.9.1 Type Mapping Functions of operator ~memlet~

        (string X m:MEMLOADABLE -> bool)

*/
ListExpr memletTypeMap(ListExpr args)
{
    if(nl->ListLength(args)!=2){
        return listutils::typeError("wrong number of arguments");
    }
    ListExpr arg1 = nl->First(args);
    if (!CcString::checkType(nl->First(arg1))) {
        return listutils::typeError("string expected");
    }
    ListExpr str = nl->Second(arg1);
    string objectName = nl->StringValue(str);
    if (catalog->isMMObject(objectName)){
        return listutils::typeError("identifier already in use");
    }
    ListExpr arg2 = nl->Second(args);
    if(listutils::isRelDescription(nl->First(arg2)) ||
       listutils::isOrelDescription(nl->First(arg2)) ||
       listutils::isDATA(nl->First(arg2)) ||
       listutils::isTupleStream(nl->First(arg2))) {
      return listutils::basicSymbol<CcBool>();
    }
    return listutils::typeError(
             "the second argument has to "
             "be of kind DATA or a relation or an ordered relation "
             "or a tuplestream");
}


/*

5.9.3  The Value Mapping Functions of operator ~memlet~

*/

int memletValMap(Word* args, Word& result,
                 int message, Word& local, Supplier s) {

   return memlet(args, result, message, local,s, false);
}


/*

5.9.4 Description of operator ~memlet~

*/



OperatorSpec memletSpec(
    "string x m:MEMLOADABLE -> bool",
    "memlet (_,_)",
    "creates a main memory object from a given MEMLOADABLE",
    "query memlet ('Trains100', Trains feed head[100])"
);



/*

5.9.5 Instance of operator ~memlet~

*/

Operator memletOp (
    "memlet",
    memletSpec.getStr(),
    memletValMap,
    Operator::SimpleSelect,
    memletTypeMap
);

/*
5.9 Operator ~memletflob~

creates a new main memory object. The first parameter is the
name of the new main memory object, the second is the query/the
MEMLOADABLE object from which the mm-object will be created.
The associated flobs will be loaded to main memory too.

*/


/*

5.9.3  The Value Mapping Functions of operator ~memletflob~

*/

int memletflobValMap (Word* args, Word& result,
                int message, Word& local, Supplier s) {

   return memlet(args, result, message, local, s,true);
}


/*

5.9.4 Description of operator ~memletflob~

*/

OperatorSpec memletflobSpec(
    "string x MEMLOADABLE -> bool",
    "memletflob (_,_)",
    "creates a main memory object from a given MEMLOADABLE."
    "the associated flobs will be loaded",
    "query memletflob ('Trains100', Trains feed head[100])"
);



/*

5.9.5 Instance of operator ~memletflob~

*/

Operator memletflobOp (
    "memletflob",
    memletflobSpec.getStr(),
    memletflobValMap,
    Operator::SimpleSelect,
    memletTypeMap
);


/*
5.10 Operator ~memupdate~
updates a main memory object. The tuple description for a stream or a relation
must be the same as the one of the main memory object.


5.10.1 Type Mapping Functions of operator ~memupdate~
        (string x m:MEMLOADABLE -> bool)

*/
ListExpr memupdateTypeMap(ListExpr args)
{

    if(nl->ListLength(args)!=2){
        return listutils::typeError("wrong number of arguments");
    }

    ListExpr arg2 = nl->First(nl->Second(args));
    // replace <stream> by <rel> if necessary. this enables unique handling
    if(Stream<Tuple>::checkType(arg2)){
       arg2 = nl->TwoElemList( listutils::basicSymbol<Relation>(),
                               nl->Second(arg2));
    }
    string errMsg;
    ListExpr arg1 = nl->First(args);
    if(!getMemType(nl->First(arg1), nl->Second(arg1), arg1, errMsg)){
      return listutils::typeError("error in 1st arg: " + errMsg);
    }

    ListExpr subtype = nl->Second(arg1);
    if(!nl->Equal(subtype,arg2)){
       return listutils::typeError("Update type and memory type differ");
    }
    if(   !Attribute::checkType(subtype)
       && !Relation::checkType(subtype)){
       return listutils::typeError("unsupported subtype");
    }
    return  listutils::basicSymbol<CcBool>();

}


/*

5.10.3  The Value Mapping Functions of operator ~memupdate~

*/
template<class T>
int memupdateValMap (Word* args, Word& result,
                int message, Word& local, Supplier s) {

    result  = qp->ResultStorage(s);
    CcBool* b = static_cast<CcBool*>(result.addr);
    bool memupdatesucceed = false;
    bool correct = true;
    Supplier t = qp->GetSon( s, 1 );
    ListExpr le = qp->GetType(t);

    bool isDATA = Attribute::checkType(le);

    T* oN = (T*) args[0].addr;
    if(!oN->IsDefined()){
        cerr << "memupdate: undefined object name" << endl;
        if(isDATA){
           ((Attribute*) result.addr)->SetDefined(false);
        }
        return 0;
    }
    string objectName = oN->GetValue();

    // check whether object is present
    if(!catalog->isMMObject(objectName)){
        cerr << "memupdate: object not found" << endl;
        if(isDATA){
           ((Attribute*) result.addr)->SetDefined(false);
        }
        return 0;
    }

    // check whether memory object has expected type
    ListExpr expType = Stream<Tuple>::checkType(le) 
               ? nl->TwoElemList( listutils::basicSymbol<Relation>(),
                                  nl->Second(le))
               : le;

    
     ListExpr memType = catalog->getMMObjectTypeExpr(objectName);
     if(!Mem::checkType(memType)){
       cerr << "internal error: memory object not of type mem" << endl;
       return 0;
     }
     memType = nl->Second(memType);
     if(!nl->Equal(memType,expType)){
        cerr << "memupdate: object has not the expected type" << endl;
        if(isDATA){
           ((Attribute*) result.addr)->SetDefined(false);
        }
        return 0;
     }

     bool flob = catalog->getMMObject(objectName)->hasflob();
     if(Relation::checkType(le)) {
        catalog->deleteObject(objectName);
        GenericRelation* rel= static_cast<Relation*>( args[1].addr );
        MemoryRelObject* mmRelObject = new MemoryRelObject();
        memupdatesucceed = mmRelObject->relToVector(rel, le, 
                                         getDBname(), flob);
        if (memupdatesucceed) {
            catalog->insert(objectName,mmRelObject);
        } else {
            delete mmRelObject;
        }
     } else if(isDATA){
        catalog->deleteObject(objectName);
        Attribute* attr = (Attribute*)args[1].addr;
        MemoryAttributeObject* mmA = new MemoryAttributeObject();
        memupdatesucceed = mmA->attrToMM(attr, le,
                                getDBname(),flob);
        if (memupdatesucceed){
           catalog->insert(objectName, mmA);
        } else {
            delete mmA;
        }
    } else if (listutils::isTupleStream(le)) {
        catalog->deleteObject(objectName);
        MemoryRelObject* mmRelObject = new MemoryRelObject();
            memupdatesucceed = mmRelObject->tupleStreamToRel(args[1],
        le, getDBname(), flob);
        if (memupdatesucceed) {
            catalog->insert(objectName,mmRelObject);
        } else {
            delete mmRelObject;
        }
    } else {
       cerr << "memupdate: unsupported type" << endl;
       correct = false;
    }

    b->Set(correct, memupdatesucceed);

    return 0;
}

ValueMapping memupdateVM[] = {
   memupdateValMap<CcString>,
   memupdateValMap<Mem>
};


int memupdateSelect(ListExpr args){
  return CcString::checkType(nl->First(args))?0:1;
}



/*

5.10.4 Description of operator ~memupdate~

*/

OperatorSpec memupdateSpec(
    "{string, mem} x m:MEMLOADABLE -> bool",
    "memupdate (_,_)",
    "updates a main memory object with a given MEMLOADABLE",
    "query memupdate ('fuenf', ten feed head[7])"
);

/*

5.10.5 Instance of operator ~memupdate~

*/

Operator memupdateOp (
    "memupdate",
    memupdateSpec.getStr(),
    2,
    memupdateVM,
    memupdateSelect,
    memupdateTypeMap
);


/*
5.10 Operator ~mcreateRtree~
creates a an mmRTree over a given main memory relation

5.10.1 Type Mapping Functions of operator ~mcreateRtree~
        (string x Ident -> string ||
         mem(rel(tuple)) x Ident -> mem(rtree dim))
        the first parameter identifies the main memory relation, the
        second parameter identifies the attribute

*/

ListExpr mcreateRtreeTypeMap(ListExpr args){

    if(nl->ListLength(args)!=2){
        return listutils::typeError("wrong number of arguments");
    }

    // Split argument in two parts
    ListExpr arg1 = nl->First(args);
    ListExpr oTE_Rel;
    string errMsg;
    if(!getMemType(nl->First(arg1), nl->Second(arg1), oTE_Rel, errMsg)){
      return listutils::typeError("problem in 1st arg:" + errMsg);
    }
    oTE_Rel = nl->Second(oTE_Rel); // remove leading mem
    if(!Relation::checkType(oTE_Rel)){
       return listutils::typeError("memory object is not a relation");
    }

    ListExpr a = nl->First(nl->Second(args));
    if(nl->AtomType(a)!=SymbolType){
       return listutils::typeError("second argument must be an identifier");
    }
    string attrName = nl->SymbolValue(a);

    ListExpr attrType = 0;
    int attrPos = 0;
    ListExpr attrList = nl->Second(nl->Second(oTE_Rel));

    attrPos = listutils::findAttribute(attrList, attrName, attrType);

    if (attrPos == 0){
        return listutils::typeError("attribute  " + attrName
           + " not present in tuple");
    }

    int dim = rtreehelper::getDimension(attrType);

    if(dim<0){
      return listutils::typeError("referenced attribute not "
                                  "in kind SPATIAL<X>D");
    }


    ListExpr resType = nl->TwoElemList(
                         listutils::basicSymbol<Mem>(),
                         nl->TwoElemList(
                             nl->SymbolAtom(rtreehelper::BasicType()),
                             nl->IntAtom(dim)));

   return nl->ThreeElemList(
               nl->SymbolAtom(Symbol::APPEND()),
               nl->ThreeElemList( nl->IntAtom(attrPos-1), 
                                nl->IntAtom(dim),
                                nl->StringAtom(attrName)),
               resType);

}

/*

5.10.3  The Value Mapping Functions of operator ~mcreateRtree~

*/

template<int dim>
bool mcreateRtree(MemoryRelObject* mmrel, 
                  int attrPos, 
                  const string& rtreeName,
                  ListExpr typeList){


    bool flob = mmrel->hasflob();
    string database = mmrel->getDatabase();
    vector<Tuple*>* relVec = mmrel->getmmrel();
    vector<Tuple*>::iterator it;
    it=relVec->begin();
    unsigned int i=0;
    mmrtree::RtreeT<dim, size_t>* rtree =
                    new mmrtree::RtreeT<dim, size_t>(4,8);

    StandardSpatialAttribute<dim>* attr=0;
    size_t usedMainMemory=0;
    unsigned long availableMemSize = catalog->getAvailableMemSize();
    while( it!=relVec->end()){
        Tuple* tup = *it;
        attr=(StandardSpatialAttribute<dim>*)tup->GetAttribute(attrPos);
        if (attr==0 || !attr->IsDefined()){
            return 0;
        }
        Rectangle<dim> box = attr->BoundingBox();
        rtree->insert(box, i);
        it++;
        i++;
    } // end while

    usedMainMemory = rtree->usedMem();
    MemoryRtreeObject<dim>* mmRtreeObject =
        new MemoryRtreeObject<dim>(rtree, usedMainMemory,
                        nl->ToString(typeList), flob, database);

    if (usedMainMemory>availableMemSize){
        cout<<"there is not enough memory left to create the rtree";
    }
    if(    usedMainMemory<=availableMemSize
        && catalog->insert(rtreeName,mmRtreeObject)){
        return true;
    }else {
       delete mmRtreeObject;
       return false;
    }
}

template<class T>
int mcreateRtreeValMapT (Word* args, Word& result,
                int message, Word& local, Supplier s) {

    result  = qp->ResultStorage(s);
    ListExpr resType = qp->GetType(s);
    Mem* str = static_cast<Mem*>(result.addr);
    bool succeed = false;

    // the main memory relation
    T* roN = (T*) args[0].addr;
    if(!roN->IsDefined()){
        str->SetDefined(false);
        return 0;
    }
    string relObjectName = roN->GetValue();
    if(!catalog->isMMObject(relObjectName)){
       cerr << "memory object " << relObjectName << " not found" << endl;
       str->SetDefined(false);
       return 0;
    }
    ListExpr memObjectType = catalog->getMMObjectTypeExpr(relObjectName);
    if(!Mem::checkType(memObjectType)){
       cerr << "internal error: memory object not of type mem" << endl;
       str->SetDefined(false);
       return 0;
    }
    memObjectType = nl->Second(memObjectType);
    if(!Relation::checkType(memObjectType)){
       cerr << "memory object is not a relation" << endl;
       str->SetDefined(false);
       return 0;
    }

    ListExpr attrList = nl->Second(nl->Second(memObjectType));
    int attrPos = ((CcInt*) args[2].addr)->GetValue();
    int dim = ((CcInt*) args[3].addr)->GetValue();
    while(!nl->IsEmpty(attrList) && attrPos > 0){
       attrList = nl->Rest(attrList);
       attrPos--;
    }
    if(nl->IsEmpty(attrList)){
       cerr << "Attribute not present" << endl;
       str->SetDefined(false);
       return 0;
    }
    attrPos = ((CcInt*) args[2].addr)->GetValue();
    ListExpr attr = nl->Second(nl->First(attrList));
    int dim2 = rtreehelper::getDimension(attr);
    if(dim!=dim2){
      cerr << "dimension change between type mapping and value mapping" 
           << endl;
      str->SetDefined(false);
      return 0;
    }
     
    MemoryRelObject* mmrel =
        (MemoryRelObject*)catalog->getMMObject(relObjectName);

    if(!mmrel){
       cerr << "internal error, rel is not present" << endl;
       str->SetDefined(false);
       return 0;
    }
    
    string attrName = ((CcString*)args[4].addr)->GetValue();
    string res = relObjectName + "_" + attrName;

    switch (dim){
       case 2: 
               succeed = mcreateRtree<2>(mmrel, attrPos, res, resType);
               break;
       case 3: succeed = mcreateRtree<3>(mmrel, attrPos, res, resType);
               break;
       case 4: succeed = mcreateRtree<4>(mmrel, attrPos, res, resType);
               break;
       case 8: succeed = mcreateRtree<8>(mmrel, attrPos, res, resType);
               break;
    }
    str->set(succeed, res);
    return 0;
 } //end mcreateRtreeValMap


ValueMapping mcreateRtreeValMap[] =
{
    mcreateRtreeValMapT<CcString>,
    mcreateRtreeValMapT<Mem>,
};

int mcreateRtreeSelect(ListExpr args){
    // string case at index 0
    if ( CcString::checkType(nl->First(args)) ){
       return 0;
    }
    // Mem(rel(tuple)) case at index 1
    if ( Mem::checkType(nl->First(args)) ){
       return 1;
    }
    // should never be reached
    return -1;
  }

/*

5.10.4 Description of operator ~mcreateRtree~

*/

OperatorSpec mcreateRtreeSpec(
    "string x string -> string || mem(rel(tuple)) x string -> mem(rtree)",
    "_ mcreateRtree [_]",
    "creates an main memory r-tree over a main memory relation given by the "
    "first string || mem(rel(tuple)) and an attribute "
    "given by the second argument.",
    "query 'WFlaechen' mcreateRtree [GeoData]"
);



/*

5.10.5 Instance of operator ~mcreateRtree~

*/

Operator mcreateRtreeOp (
    "mcreateRtree",             //operator's name
    mcreateRtreeSpec.getStr(),  //specification
    2,
    mcreateRtreeValMap,         // value mapping array
    mcreateRtreeSelect,    //selection function
    mcreateRtreeTypeMap         //type mapping
);

/*

5.10 Operator ~mcreateRtree2~
creates a an mmRTree the keytype must be of Kind SPATIAL2D,
SPATIAL3D, SPATIAL4D, SPATIAL8D, or of type rect

5.10.1 Type Mapping Functions of operator ~mcreateRtree2~
        (stream(Tuple) x T x string-> string) mit T of KIND SPATIAL2D,
        SPATIAL3D, SPATIAL4D, SPATIAL8D, or of type rect

*/

ListExpr mcreateRtree2TypeMap(ListExpr args){
    string err = "stream(Tuple) x attrName x string expected";
    if(!nl->HasLength(args,3)){
        return listutils::typeError("wrong number of arguments");
    }
    // first arg stream(Tuple)?
    if(!Stream<Tuple>::checkType(nl->First(args))){
        return listutils::typeError("first argument must be a stream(Tuple)");
    }
    // second Arg is AttrName
    if(nl->AtomType(nl->Second(args))!=SymbolType){
        return listutils::typeError("second argument must be an attribute");
    }
    // third arg string (name of the rtree)
    if(!CcString::checkType(nl->Third(args))){
        return listutils::typeError("third argument must be a string");
    }
    string name = nl->SymbolValue(nl->Second(args));
    // get attributelist
    ListExpr attrList = nl->Second(nl->Second(nl->First(args)));
    ListExpr type;
    int j = listutils::findAttribute(attrList,name,type);
    if(j==0){
        return listutils::typeError("Attr " + name +" not found");
    }
    int dim = rtreehelper::getDimension(type);
    if(dim < 0){
       return listutils::typeError("type " + nl->ToString(type) 
                                   + " not supported");
    }


    ListExpr tid = listutils::basicSymbol<TupleIdentifier>();
    // find a tid in the attribute list
    string tidn;
    int k = listutils::findType(attrList, tid, tidn);
    if(k==0){
        return listutils::typeError("no tid in tuple");
    }

    ListExpr resType = nl->TwoElemList(
                          listutils::basicSymbol<Mem>(),
                          nl->TwoElemList(
                             nl->SymbolAtom(rtreehelper::BasicType()),
                             nl->IntAtom(dim)));
   

    return nl->ThreeElemList(
                   nl->SymbolAtom(Symbols::APPEND()),
                   nl->TwoElemList( nl->IntAtom(j-1),
                                    nl->IntAtom(k-1)),
                   resType);
}


/*

5.10.3  The Value Mapping Functions of operator ~mcreateRtree2~

*/
template <int dim>
int mcreateRtree2ValMapT (Word* args, Word& result,
                int message, Word& local, Supplier s) {

    result  = qp->ResultStorage(s);
    Mem* str = static_cast<Mem*>(result.addr);
    bool succeed = false;
    size_t usedMainMemory=0;
    unsigned long availableMemSize = catalog->getAvailableMemSize();

    //get r-tree name
    string name = ((CcString*)args[2].addr)->GetValue();

    // create mainmemory rtrees
    mmrtree::RtreeT<dim, size_t>* rtree =
                    new mmrtree::RtreeT<dim, size_t>(4,8);

    // get attribute-index
    int MBRIndex = ((CcInt*) args[3].addr)->GetValue();
    int TIDIndex = ((CcInt*) args[4].addr)->GetValue();

    Stream<Tuple> stream(args[0]);
    stream.open();
    Tuple* t;

    while( (t=stream.request())!=0){
        int tid = ((TupleIdentifier*)t->GetAttribute(TIDIndex))->GetTid();
        StandardSpatialAttribute<dim>* attr
            =(StandardSpatialAttribute<dim>*) t->GetAttribute(MBRIndex);

        Rectangle<dim> rect = attr->BoundingBox();
        // insert rect,id -pair into the tree
        rtree->insert(rect, tid);
        t->DeleteIfAllowed();
    }

    stream.close();
    usedMainMemory = rtree->usedMem();

    ListExpr le = nl->TwoElemList(
                      listutils::basicSymbol<Mem>(),
                      nl->TwoElemList(
                        nl->SymbolAtom(rtreehelper::BasicType()),
                        nl->IntAtom(dim)
                      ));

    MemoryRtreeObject<dim>* mmRtreeObject =
        new MemoryRtreeObject<dim>(rtree, usedMainMemory,
                        nl->ToString(le), false, "");

    if (usedMainMemory>availableMemSize){
        cout<<"there is not enough memory left to create the rtree";
    }
    if (usedMainMemory<=availableMemSize &&
                catalog->insert(name,mmRtreeObject)){
        succeed = true;
    }else {
       delete mmRtreeObject;
       succeed = false;
    }
    str->set(succeed, name);
    return 0;

}

ValueMapping mcreateRtree2ValMap[] =
{
    mcreateRtree2ValMapT<2>,
    mcreateRtree2ValMapT<3>,
    mcreateRtree2ValMapT<4>,
    mcreateRtree2ValMapT<8>,
};

/*
1.3 Selection method for value mapping array ~mcreateRtree2~

*/
 int mcreateRtree2Select(ListExpr args)
 {
    ListExpr attrList = nl->Second(nl->Second(nl->First(args)));
    ListExpr type;
    string name = nl->SymbolValue(nl->Second(args));
    listutils::findAttribute(attrList,name,type);

    int dim = rtreehelper::getDimension(type);
    switch(dim){
         case 2: return 0;
         case 3: return 1;
         case 4: return 2;
         case 8: return 3;
    }
    return -1;
}

/*

5.10.4 Description of operator ~mcreateRtree2~

*/

OperatorSpec mcreateRtree2Spec(
    "stream(Tuple) x Ident  x string -> mem(rtree)",
    "_ mcreateRtree2 [_,_]",
    "creates a main memory r-tree over a tuple stream (1st argument)."
    "The second argument specifies the attribute over that the index "
    "is created. The type of this attribute must be in kind SPATIALxD. "
    "The third argument specifies the name of the main memory object.",
    "query strassen feed head[5] mcreateRtree2 [GeoData, 'strassen_GeoData']"
);



/*

5.10.5 Instance of operator ~mcreateRtree2~

*/

Operator mcreateRtree2Op (
    "mcreateRtree2",             //operator's name
    mcreateRtree2Spec.getStr(),  //specification
    4,
    mcreateRtree2ValMap,         // value mapping array
    mcreateRtree2Select,    //selection function
    mcreateRtree2TypeMap         //type mapping
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

    ListExpr argFir = nl->First(args); //stream + query
    ListExpr stream = nl->First(argFir);

    if (!Stream<Tuple>::checkType(stream)) {
        return listutils::typeError
            ("stream(Tuple) as first argument expected");
    }
    
    ListExpr argSec = nl->Second(args); //string + query

    string errMsg;
    ListExpr a2t;
    if(!getMemType(nl->First(argSec), nl->Second(argSec), a2t, errMsg)){
      return listutils::typeError("problem in 2nd arg: " + errMsg);
    }

    ListExpr subtype = nl->Second(a2t);
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
     meminsertInfo( Word w, vector<Tuple*>* _relation, bool _flob):
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
template<class T>
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
            ListExpr tt = nl->Second(qp->GetType(qp->GetSon(s,0)));
            MemoryRelObject* mro = getMemRel(oN,tt);
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
   meminsertValMap<CcString>,
   meminsertValMap<Mem>
};

int meminsertSelect(ListExpr args){
   return CcString::checkType(nl->Second(args))?0:1;
}



/*

5.13.4 Description of operator ~meminsert~

*/

OperatorSpec meminsertSpec(
    "stream(Tuple) x {string, mem(rel)}  -> stream(Tuple)",
    "meminsert(_,_)",
    "inserts the tuple of a stream into an "
    "existing main memory relation",
    "query meminsert (ten feed head[5],'ten') count"
);

/*

5.13.5 Instance of operator ~meminsert~

*/

Operator meminsertOp (
    "meminsert",
    meminsertSpec.getStr(),
    2,
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

  string err=" {string, mem(rtree)} x {string, mem(rel)} x SPATIALXD expected";

  ListExpr a1t;
  string errMsg;

  if(!getMemType(nl->First(a1), nl->Second(a1), a1t, errMsg)){
    return listutils::typeError(err + "\n problem in arg 1: " + errMsg);
  } 
  
  ListExpr rtreetype = nl->Second(a1t); // remove leading mem
  
  if(!rtreehelper::checkType(rtreetype)){
    return listutils::typeError(err + " (first arg is not a mem rtree)");
  }

  int rtreedim = nl->IntValue(nl->Second(rtreetype));
  ListExpr a2t;
  if(!getMemType(nl->First(a2), nl->Second(a2), a2t, errMsg)){
    return listutils::typeError(err + "\n problem in arg 2: " + errMsg);
  }
  ListExpr relType = nl->Second(a2t);
  if(!Relation::checkType(relType)){
    return listutils::typeError(err + " (second argument is not "
                                "a memory relation)");
  }
 
  int dim2 = rtreehelper::getDimension(nl->First(a3));

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
             res->IncReference();
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
template <int dim, class T, class R>
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
    mwindowintersectsValMapT<2,CcString, CcString>,
    mwindowintersectsValMapT<3,CcString, CcString>,
    mwindowintersectsValMapT<4,CcString, CcString>,
    mwindowintersectsValMapT<8,CcString, CcString>,

    mwindowintersectsValMapT<2,CcString, Mem>,
    mwindowintersectsValMapT<3,CcString, Mem>,
    mwindowintersectsValMapT<4,CcString, Mem>,
    mwindowintersectsValMapT<8,CcString, Mem>,
    
    mwindowintersectsValMapT<2,Mem, CcString>,
    mwindowintersectsValMapT<3,Mem, CcString>,
    mwindowintersectsValMapT<4,Mem, CcString>,
    mwindowintersectsValMapT<8,Mem, CcString>,

    mwindowintersectsValMapT<2,Mem, Mem>,
    mwindowintersectsValMapT<3,Mem, Mem>,
    mwindowintersectsValMapT<4,Mem, Mem>,
    mwindowintersectsValMapT<8,Mem, Mem>,
};

/*
1.3 Selection method for value mapping array ~mcreateRtree~

*/
 int mwindowintersectsSelect(ListExpr args)
 {
   int n1 = CcString::checkType(nl->First(args))?0:1;
   int n2 = CcString::checkType(nl->Second(args))?0:1;
   int dim = rtreehelper::getDimension(nl->Third(args));
   int n3 = -1;
   switch (dim){
     case 2 : n3 = 0;break;
     case 3 : n3 = 1;break;
     case 4 : n3 = 2;break;
     case 8 : n3 = 3;break;
   }
   return 8*n1 + 4*n2 + n3;

  return -1;
 }


/*

5.14.4 Description of operator ~mwindowintersects~

*/

OperatorSpec mwindowintersectsSpec(
   "{string, mem(rtree <dim>) x {string, mem(rel(X)) x T -> stream(tuple(X)) "
   "where T in SPATIAL<dim>D",
   "_ _ mwindowintersects[_]",
   "Uses the given rtree to find all tuples"
   " in the given relation which intersects the "
   " argument value's bounding box.",
   "query \"strassen_GeoData\" \"strassen\" mwindowintersects[thecenter] count"
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

ListExpr mwindowintersectsSTM(ListExpr args){

  string err = " {string, memory(rtree <dim> )} x SPATIAL<dim>D expected";
  if(!nl->HasLength(args,2)){
     return listutils::typeError(err + " (wrong number of args)");
  }
  ListExpr a2t = nl->First(nl->Second(args));
  int dim = rtreehelper::getDimension(a2t);
  if(dim < 0){
     return listutils::typeError(err + " (second arg is not in "
                                 "kind SPATIALxD)");
  }
  ListExpr a1 = nl->First(args);
  ListExpr a1t;
  string errMsg;
  if(!getMemType(nl->First(a1), nl->Second(a1), a1t, errMsg)){
     return listutils::typeError(err +"\n error in 1st arg: " + errMsg);
  }
  ListExpr tree = nl->Second(a1t);
  if(!rtreehelper::checkType(tree)){
      return listutils::typeError(err + " (first arg is not an r-tree");
  }
  int tdim = nl->IntValue(nl->Second(tree));
  if(dim!=tdim){
     return listutils::typeError(err + "tree and query object have "
                                       "different dimensions");
  }
  return nl->TwoElemList(
                    listutils::basicSymbol<Stream<TupleIdentifier> >(),
                    listutils::basicSymbol<TupleIdentifier> ());
}


template <int dim, class T>
int mwindowintersectsSVMT (Word* args, Word& result,
                    int message, Word& local, Supplier s) {

   typedef typename mmrtree::RtreeT<dim,size_t>::iterator it;
   it* li = (it*) local.addr;

   switch(message){
      case OPEN:{
             if(li){
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
             local.addr = tree->getrtree()->find(box);
             return 0;
      }
      case REQUEST : {
               const size_t* index = li->next();
               if(index){
                  result.addr = new TupleIdentifier(true,*index);
               } else {
                  result.addr=0;
               }
               return result.addr?YIELD:CANCEL;
        }
       case CLOSE:
                 if(li){
                    delete li;
                    local.addr = 0;
                 }
                 return 0;
   }
   return -1;
}

ValueMapping mwindowintersectsSVM[] = {
   mwindowintersectsSVMT<2, CcString>,
   mwindowintersectsSVMT<3, CcString>,
   mwindowintersectsSVMT<4, CcString>,
   mwindowintersectsSVMT<8, CcString>,
   mwindowintersectsSVMT<2, Mem>,
   mwindowintersectsSVMT<3, Mem>,
   mwindowintersectsSVMT<4, Mem>,
   mwindowintersectsSVMT<8, Mem>
};

int mwindowintersectsSSelect(ListExpr args){
   int n1 = CcString::checkType(nl->First(args))?0:1;
   int dim = rtreehelper::getDimension(nl->Second(args));
   int n2;
   switch(dim){
      case 2 : n2 = 0; break;
      case 3 : n2 = 1; break;
      case 4 : n2 = 2; break;
      case 8 : n2 = 3; break;
      default : assert(false);
   }
   return 4*n1 + n2;
}


OperatorSpec mwindowintersectsSSpec(
  "{string, memory(rtree <dim>)} x SPATIAL<dim>D -> stream(tid)",
  " _ mwindowintersectsS[_]",
  "Returns the tuple ids belonging to rectangles intersecting the "
  "bounding box of the second argument. ",
  "query \"strassen_GeoData\" mwindowintersectsS[ thecenter ] count" 
);

Operator mwindowintersectsSOp(
   "mwindowintersectsS",
   mwindowintersectsSSpec.getStr(),
   8,
   mwindowintersectsSVM,
   mwindowintersectsSSelect,
   mwindowintersectsSTM
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
    ListExpr l1 = nl->Second(nl->First(args));
    ListExpr l2 = nl->SymbolAtom(MemoryRelObject::BasicType());
    return nl->TwoElemList (l2,l1);;
}


/*

5.15.3  The Value Mapping Functions of operator ~mconsume~

*/

int mconsumeValMap (Word* args, Word& result,
                int message, Word& local, Supplier s) {
    Supplier t = qp->GetSon( s, 0 );
    ListExpr le = qp->GetType(t);
    result  = qp->ResultStorage(s);
    MemoryRelObject* mrel = (MemoryRelObject*)result.addr;
    Stream<Tuple> stream(args[0]);
    stream.open();
    Tuple* tup = 0;
    while( (tup = stream.request()) != 0){
        mrel->addTuple(tup);
    }
    mrel->setObjectTypeExpr(nl->ToString(nl->Second(le)));
    stream.close();
    return 0;
}


/*

5.15.4 Description of operator ~mconsume~

*/

OperatorSpec mconsumeSpec(
    "stream(Tuple) -> memoryrelobject",
    "_ mconsume",
    "collects the objects from a stream(tuple) into a memory relation",
    "query 'ten' mfeed mconsume"
);



/*

5.15.5 Instance of operator ~mconsume~

*/

Operator mconsumeOp (
    "mconsume",
    mconsumeSpec.getStr(),
    mconsumeValMap,
    Operator::SimpleSelect,
    mconsumeTypeMap
);




/*
5.16 Operator ~mcreateAVLtree~
creates a an AVLTree over a given main memory relation

5.16.1 Type Mapping Functions of operator ~mcreateAVLtree~
        (string x string -> string  ||
        mem(rel(tuple)) x string -> string)

        the first parameter identifies the main memory relation, the
        second parameter identifies the attribute

*/

ListExpr mcreateAVLtreeTypeMap(ListExpr args){

    if(nl->ListLength(args)!=2){
     return listutils::typeError("two arguments expected");
    }

    // Split argument in two parts
    ListExpr a1 = nl->First(args);
    ListExpr a2 = nl->Second(args);
    ListExpr oTE_Rel;

    string errMsg;
    if(!getMemType(nl->First(a1), nl->Second(a1), oTE_Rel, errMsg)){
      return listutils::typeError("problem in 1st arg: " + errMsg);
    }
    oTE_Rel = nl->Second(oTE_Rel);

    if(!Relation::checkType(oTE_Rel)){
       return listutils::typeError("memory object is not a relation");
    }

    if(nl->AtomType(nl->First(a2))!=SymbolType){
       return listutils::typeError("second argument is not a valid "
                                   "attribute name");
    }

    string attrName = nl->SymbolValue(nl->First(a2));
    ListExpr attrType = 0;
    int attrPos = 0;
    ListExpr attrList = nl->Second(nl->Second(oTE_Rel));
    attrPos = listutils::findAttribute(attrList, attrName, attrType);

    if (attrPos == 0){
        return listutils::typeError
        ("there is no attribute having  name " + attrName);
    }

    ListExpr resType = nl->TwoElemList(
                          listutils::basicSymbol<Mem>(),
                          nl->TwoElemList(
                             listutils::basicSymbol<MemoryAVLObject>(),
                             attrType
                       ));


    return nl->ThreeElemList(
                nl->SymbolAtom(Symbols::APPEND()),
                nl->TwoElemList(nl->IntAtom(attrPos - 1),
                                nl->StringAtom(attrName)),
                resType);
}

/*

5.16.3  The Value Mapping Functions of operator ~mcreateAVLtree~

*/
template<class T>
int mcreateAVLtreeValMapT (Word* args, Word& result,
                int message, Word& local, Supplier s) {

    result  = qp->ResultStorage(s);
    Mem* str = static_cast<Mem*>(result.addr);

   // the main memory relation
   T* roN = (T*) args[0].addr;
   if(!roN->IsDefined()){
        str->SetDefined(false);
        return 0;
   }
   string relObjectName = roN->GetValue();

   int attrPos = ((CcInt*) args[2].addr)->GetValue();

   ListExpr memObjectType = catalog->getMMObjectTypeExpr(relObjectName);
   memObjectType = nl->Second(memObjectType);

   if(!Relation::checkType(memObjectType)){
     cerr << "object " << relObjectName  << "is not a relation" << endl;
     str->SetDefined(false);
     return false;
   } 

   // extract attribute 
   ListExpr attrList = nl->Second(nl->Second(memObjectType));
   int ap = attrPos;
   while(!nl->IsEmpty(attrList) && ap>0){
      attrList = nl->Rest(attrList);
      ap--;
   }
   if(nl->IsEmpty(attrList)){
     cerr << "not enough attributes in tuple";
     str->SetDefined(false);
     return 0;
   }
   ListExpr relattrType = nl->Second(nl->First(attrList));
   ListExpr avlattrType = nl->Second(nl->Second(qp->GetType(s)));

   if(!nl->Equal(relattrType, avlattrType)){
     cerr << "expected type and type in relation differ" << endl;
     str->SetDefined(false);
     return 0;
   }

   string attrName = ((CcString*)args[3].addr)->GetValue();

   string resName = relObjectName + "_" + attrName;
   if(catalog->isMMObject(resName)){
      cerr << "object " << resName << "already exists" << endl;
      str->SetDefined(false);
      return 0;
   }

   MemoryRelObject* mmrel =
        (MemoryRelObject*)catalog->getMMObject(relObjectName);
   bool flob = mmrel->hasflob();
   vector<Tuple*>* relVec = mmrel->getmmrel();
   vector<Tuple*>::iterator it;
   it=relVec->begin();
   unsigned int i=0;

   memAVLtree* tree = new memAVLtree();
   Attribute* attr;
   avlPair aPair;

   size_t usedMainMemory = 0;
   unsigned long availableMemSize = catalog->getAvailableMemSize();

   while ( it!=relVec->end()){
       Tuple* tup = *it;
       attr=tup->GetAttribute(attrPos);
       aPair = avlPair(attr->Copy(),i);
       // size for a pair is 16 bytes, plus an additional pointer 8 bytes
       size_t entrySize = 24;
       if (entrySize<availableMemSize){
           tree->insert(aPair);
           usedMainMemory += (entrySize);
           availableMemSize -= (entrySize);
           it++;
           i++;
       } else {
          cout<<"there is not enough main memory available"
          " to create an AVLTree"<<endl;
          delete tree;
          str->SetDefined(false);
          return 0;
        }
    }
    MemoryAVLObject* avlObject =
        new MemoryAVLObject(tree, usedMainMemory,
            nl->ToString(qp->GetType(s)),flob, getDBname());
    catalog->insert(resName,avlObject);

    str->set(true, resName);
    return 0;
} //end mcreateAVLtreeValMap


ValueMapping mcreateAVLtreeValMap[] =
{
    mcreateAVLtreeValMapT<CcString>,
    mcreateAVLtreeValMapT<Mem>,
};

int mcreateAVLtreeSelect(ListExpr args){
    // string case at index 0
    if ( CcString::checkType(nl->First(args)) ){
       return 0;
    }
    // Mem(rel(tuple))case at index 1
    if ( Mem::checkType(nl->First(args)) ){
       return 1;
    }
    // should never be reached
    return -1;
  }

/*

5.16.4 Description of operator ~mcreateAVLtree~

*/

OperatorSpec mcreateAVLtreeSpec(
    "string x string -> string || mem(rel(tuple(X))) x string -> string",
    "_ mcreateAVLtree [_]",
    "creates an AVLtree over a main memory relation given by the"
    "first string || mem(rel(tuple))  and an attribute "
    "given by the second iargument",
    "query \"Staedte\" mcreateAVLtree [SName]"
);

/*

5.16.5 Instance of operator ~mcreateAVLtree~

*/

Operator mcreateAVLtreeOp (
    "mcreateAVLtree",
    mcreateAVLtreeSpec.getStr(),
    2,
    mcreateAVLtreeValMap,
    mcreateAVLtreeSelect,
    mcreateAVLtreeTypeMap
);


/*
5.17 Operator createAVLtree2

This operator creates an AVL tree from a tuple stream.

*/
ListExpr mcreateAVLtree2TM(ListExpr args){
  string err = "stream(tuple) x Ident x Indent x string expected";
  if(!nl->HasLength(args,4)){
    return listutils::typeError(err + " (wrong number of args)");
  }

  if(!Stream<Tuple>::checkType(nl->First(args))){
    return listutils::typeError(err + " ( first arg is not a tuple stream)");
  }
  if(nl->AtomType(nl->Second(args)) != SymbolType){
    return listutils::typeError(err + " (second arg is not a "
                                "valid identifier)");
  }
  if(nl->AtomType(nl->Third(args)) != SymbolType){
    return listutils::typeError(err + " (third arg is not a "
                                "valid identifier)");
  }
  if(!CcString::checkType(nl->Fourth(args))){
    return listutils::typeError(err+ " (fourth arg is not a string)");
  }

  ListExpr attrList = nl->Second(nl->Second(nl->First(args)));
  string keyName = nl->SymbolValue(nl->Second(args));
  string tidName = nl->SymbolValue(nl->Third(args));
  ListExpr keyType;
  int keyIndex = listutils::findAttribute(attrList, keyName, keyType);
  if(!keyIndex){
     return listutils::typeError("attribute " + keyName 
                                 + " not found in tuple");
  }
  ListExpr tidType;
  int tidIndex = listutils::findAttribute(attrList, tidName, tidType);
  if(!tidIndex){
     return listutils::typeError("attribute " + tidName 
                                 + " not found in tuple");
  }
  if(!TupleIdentifier::checkType(tidType)){
     return listutils::typeError("attribute " + tidName + " is not a tid");
  }
  ListExpr resType = nl->TwoElemList(
                          listutils::basicSymbol<Mem>(),
                          nl->TwoElemList(
                               listutils::basicSymbol<MemoryAVLObject>(),
                               keyType));

  return nl->ThreeElemList(
                  nl->SymbolAtom(Symbols::APPEND()),
                  nl->TwoElemList( nl->IntAtom(keyIndex-1),
                                   nl->IntAtom(tidIndex-1)),
                  resType);
}


int mcreateAVLtree2VM (Word* args, Word& result,
                int message, Word& local, Supplier s) {

    result = qp->ResultStorage(s);
    Mem* res = (Mem*) result.addr;

    CcString* memName = (CcString*) args[3].addr;
    if(!memName->IsDefined()){
      res->SetDefined(false);
      return 0;
    }
    string name = memName->GetValue();
    if(catalog->isMMObject(name)){
      res->SetDefined(false);
      return 0;
    }

    int keyIndex = ((CcInt*) args[4].addr)->GetValue();
    int tidIndex = ((CcInt*) args[5].addr)->GetValue();

    Stream<Tuple> stream(args[0]);
    Tuple* tuple;
    memAVLtree* tree = new memAVLtree();

    stream.open();
    size_t availableMem = catalog->getAvailableMemSize();
    size_t usedMem = 0;
    size_t count = 0;

    while((tuple=stream.request()) && (usedMem < availableMem)){
      TupleIdentifier* tid = (TupleIdentifier*) tuple->GetAttribute(tidIndex);
      if(tid->IsDefined()){
          count++;
          Attribute* key = tuple->GetAttribute(keyIndex);
          key->bringToMemory();
          key = key->Copy();
          avlPair p(key, tid->GetTid());
          usedMem += key->GetMemSize() + sizeof(size_t);
          tree->insert(p); 
      }
      tuple->DeleteIfAllowed();
    }
    stream.close();
    if(usedMem > availableMem){
       delete tree;         
       res->SetDefined(false);
       return 0;
    }
    MemoryAVLObject* mo = new MemoryAVLObject(tree, usedMem, 
                       nl->ToString(qp->GetType(s)), true, getDBname());

    bool success = catalog->insert(name, mo);
    if(!success){
       delete mo;
    }
    res->set(success,name);
    return 0;
}


OperatorSpec mcreateAVLtree2Spec(
  "stream(tuple) x Ident x Ident x string -> mem(avltree ...)",
  "_ mcreateAVLtree2[_,_,_] ",
  "creates an avl tree over an tuple stream. The second argument "
  "specifies the attribute over that the index is build, the "
  "third argument specifies a TID attribute within the tuple. "
  "The last argument specifies the name in the memory representation.",
  "query strassen feed addid createAVLtree2[Name, TID, \"strassen_Name\"]"
);


Operator mcreateAVLtree2Op (
    "mcreateAVLtree2",
    mcreateAVLtree2Spec.getStr(),
    mcreateAVLtree2VM,
    Operator::SimpleSelect,
    mcreateAVLtree2TM
);


/*
5.17 Operator ~mexactmatch~
        Uses the given MemoryAVLObject or MemoryTTreeObject (as first argument)
        to find all tuples in the given MemoryRelObject (as second argument)
        with the same key value


5.17.1 Type Mapping Functions of operator ~mexactmatch~
    string x string x key -> stream(Tuple)


*/


ListExpr mexactmatchTypeMap(ListExpr args)
{
    string err ="{string, mem(avltree(T)) } x {string, mem(rel)} x T expected";
    if(nl->ListLength(args)!=3){
        return listutils::typeError("three arguments expected");
    }

    // process first argument
    ListExpr a1t = nl->First(args);
    string errMsg;

    if(!getMemType(nl->First(a1t), nl->Second(a1t), a1t, errMsg)){
       return listutils::typeError(  err + "(problem in first arg : " 
                                   + errMsg+")");
    }
   // process second argument
    ListExpr a2t = nl->Second(args);
    if(!getMemType(nl->First(a2t), nl->Second(a2t), a2t, errMsg)){
       return listutils::typeError(  err + "(problem in first arg : " 
                                   + errMsg+")");
    }

    
    a1t = nl->Second(a1t); // remove mem
    a2t = nl->Second(a2t);

    if(!MemoryAVLObject::checkType(a1t) && !MemoryTTreeObject::checkType(a1t)){
      return listutils::typeError("first arg is not an avl tree or a t tree");
    }
    if(!Relation::checkType(a2t)){
      return listutils::typeError("second arg is not a memory relation");
    }

    ListExpr a3t = nl->First(nl->Third(args));

    if(!nl->Equal(a3t, nl->Second(a1t))){
      return listutils::typeError("type managed by tree and key type differ");
    }

    return nl->TwoElemList( listutils::basicSymbol<Stream<Tuple> >(),
                            nl->Second(a2t));

}

class avlOperLI{
    public:
        avlOperLI(
           memAVLtree* _tree,
           vector<Tuple*>* _relation, 
           Attribute* _attr1,
           Attribute* _attr2, 
           string _keyType)
           :relation(_relation), avltree(_tree), attr1(_attr1), attr2(_attr2),
           keyType(_keyType){
           isAvl = true;
           avlit = avltree->tail(avlPair(attr1,0));    
           res = true; 
           
        }
        
        avlOperLI(
          memttree* _tree,
          vector<Tuple*>* _relation, 
          Attribute* _attr1,
          Attribute* _attr2, 
          string _keyType)
          :relation(_relation), ttree(_tree), attr1(_attr1), attr2(_attr2),
          keyType(_keyType){
          isAvl = false;
          AttrComp cmp;
          tit = ttree->tail(ttreePair(attr1,0), cmp); 
          res = true; 
        }


        ~avlOperLI(){}


        Tuple* next(){
          // T-Tree
          if(!isAvl) {
            if(tit.end()) 
              return 0;
            else {
              thit = *tit; 
              if (keyType=="string"){
                string attr1ToString = ((CcString*) attr1)->GetValue();
                string attr2ToString = ((CcString*) attr2)->GetValue();
                string hitString = ((CcString*)(thit.first))->GetValue();
                hitString=trim(hitString);

                if (hitString > attr2ToString) {
                    return 0;
                }
                if (hitString == attr1ToString ||
                    hitString < attr2ToString  ||
                    hitString == attr2ToString ){

                    Tuple* result = relation->at(thit.second-1);
                    result->IncReference();
                    tit++;
                    return result;
                }
                return 0;
              } //end keyType string
              
              if ((thit.first)->Compare(attr2) > 0){ // end reached
                return 0;
              }
              Tuple* result = relation->at(thit.second-1);
              result->IncReference();
              tit++;
              return result;
            }
          }
          // AVL-Tree
          else {
            if(avlit.onEnd()){
                 return 0;
            }
            avlhit = avlit.Get();

            // special treatment for string type , really a good idea???
            if (keyType=="string"){
                string attr1ToString = ((CcString*) attr1)->GetValue();
                string attr2ToString = ((CcString*) attr2)->GetValue();
                string hitString = ((CcString*)(avlhit->first))->GetValue();
                hitString=trim(hitString);

                if (hitString > attr2ToString) {
                    return 0;
                }
                if (hitString == attr1ToString ||
                    hitString < attr2ToString  ||
                    hitString == attr2ToString ){

                    Tuple* result = relation->at(avlhit->second);
                    result->IncReference();
                    avlit.Next();
                    return result;
                }
                return 0;
            } //end keyType string

            if ((avlhit->first)->Compare(attr2) > 0){ // end reached
                return 0;
            }

            Tuple* result = relation->at(avlhit->second);
            result->IncReference();
            avlit.Next();
            return result;
          }
          return 0;     // shouldn't be reached
        }


        Tuple* matchbelow(){
          if (res) {
            // AVL-Tree
            if(isAvl) {
              size_t i = relation->size();
              avlhit = avltree->GetNearestSmallerOrEqual
                          (avlPair(attr1,i));
              if (avlhit==0) {
                  return 0;
              }
              Tuple* result = relation->at(avlhit->second);
              result->IncReference();
              res = false;
              return result;
            }
            // T-Tree
            else {
              if(!tit.end()) {
                thit.first->Print(cout);
                while(tit.hasNext()) {
                  ttreePair pair = *tit;
                  if(pair.first->Compare(attr1) > 0) {
                    break;
                  }
                  else if(pair.first->Compare(attr1) < 0) {
                    thit = *tit;
                    tit++;
                  }
                  else {
                    thit = *tit;
                    // there are more than one entries with the same value
                    if(thit.first->Compare(attr1) == 0) {
                      tit++;
                    }
                    else {
                      thit = *tit;
                      break;
                    }
                  }
                }
              }

              Tuple* result = relation->at(thit.second-1);
              result->IncReference();
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
        avlIterator avlit;
        ttree::Iterator<ttreePair,AttrComp> tit;
        const avlPair* avlhit;
        ttreePair thit;
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
        case OPEN: {
            if(li){
                delete li;
                local.addr=0;
            }
            
            R* relN = (R*) args[1].addr;
            MemoryRelObject* mro = getMemRel(relN, nl->Second(qp->GetType(s)));
            if(!mro){
              return 0;
            }
            Attribute* key = (Attribute*) args[2].addr;
            
            T* treeN = (T*) args[0].addr;
            ListExpr subtype = qp->GetType(qp->GetSon(s,2));
            memAVLtree* avltree = getAVLtree(treeN, subtype);
            if(avltree) {
              local.addr= new avlOperLI(avltree,
                                      mro->getmmrel(),
                                      key,key,
                                      nl->ToString(subtype));
            }
            else {
              MemoryTTreeObject* ttree = getTtree(treeN);
              if(ttree) {
                local.addr= new avlOperLI(ttree->getttree(),
                                      mro->getmmrel(),
                                      key,key,
                                      nl->ToString(subtype));
              }
              else {
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
   mexactmatchVMT<CcString, CcString, false>,
   mexactmatchVMT<CcString, Mem, false>,
   mexactmatchVMT<Mem, CcString, false>,
   mexactmatchVMT<Mem, Mem, false>
};

ValueMapping matchbelowVM[] = {
   mexactmatchVMT<CcString, CcString, true>,
   mexactmatchVMT<CcString, Mem, true>,
   mexactmatchVMT<Mem, CcString, true>,
   mexactmatchVMT<Mem, Mem, true>
};

int mexactmatchSelect(ListExpr args){
  int n1 = CcString::checkType(nl->First(args))?0:2;
  int n2 = CcString::checkType(nl->Second(args))?0:1;
  return n1 + n2;
}

/*

5.17.4 Description of operator ~mexactmatch~

*/

OperatorSpec mexactmatchSpec(
    "{string, mem(avltree T) x {string, mem(rel(tuple(X)))} "
    " x T -> stream(Tuple(X))",
    "_ _ mexactmatch[_]",
    "Uses the given MemoryAVLObject (as first argument) to find all tuples "
    "in the given MemoryRelObject (as second argument) "
    "that have the same attribute value",
    "query \"Staedte_SName\", \"Staedte\" mexactmatch [\"Dortmund\"] count"
);

/*

5.17.5 Instance of operator ~mexactmatch~

*/
Operator mexactmatchOp (
    "mexactmatch",
    mexactmatchSpec.getStr(),
    4,
    mexactmatchVM,
    mexactmatchSelect,
    mexactmatchTypeMap
);


/*

5.19.4 Description of operator ~matchbelow~

*/

OperatorSpec matchbelowSpec(
    "{string, mem(avltree T)}  x {string, mem(rel(tuple(X)))}  "
    "x T -> stream(tuple(X)) ",
    "_ _ matchbelow[_]",
    "returns for a key  (third argument) the tuple which "
    " contains the biggest attribute value in the AVLtree (first argument) "
    " which is smaller than key  or equal to key",
    "query \"Staedte_SName\" \"Staedte\" matchbelow [\"Dortmund\"] count"
);

Operator matchbelowOp (
    "matchbelow",
    matchbelowSpec.getStr(),
    4,
    matchbelowVM,
    mexactmatchSelect,
    mexactmatchTypeMap
);


/*
5.18 Operator ~mrange~
        Uses the given MemoryAVLObject or MemoryTTreeObject (as first argument)
        to find all tuples in the given MemoryRelObject (as second argument)
        which are between the first and the second attribute value
        (as third and fourth argument)

5.18.1 Type Mapping Functions of operator ~mrange~
    string x string x key x key -> stream(Tuple)


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

    string err = "{string, mem(avltree)}  x "
                 "{string, mem(rel)} x T x T expected";

    string errMsg;
    if(!getMemType(nl->First(a1), nl->Second(a1), a1, errMsg)){
      return listutils::typeError(err + "\n problem in first arg:" + errMsg);
    }

    if(!getMemType(nl->First(a2), nl->Second(a2), a2, errMsg)){
      return listutils::typeError(err + "\n problem in first arg:" + errMsg);
    }

    a1 = nl->Second(a1); // remove mem
    a2 = nl->Second(a2); 
    a3 = nl->First(a3);  // extract type
    a4 = nl->First(a4);

    if(!MemoryAVLObject::checkType(a1) && !MemoryTTreeObject::checkType(a1)){
      return listutils::typeError(err 
                    + " (first arg is not an avl tree or a ttree)");
    }
    if(!Relation::checkType(a2)){
      return listutils::typeError(err + " (second arg is not a relation)");
    }
    ListExpr avlType = nl->Second(a1);
    if(!nl->Equal(avlType, a3)){
      return listutils::typeError("avltype and type of arg 3 differ");
    }
    if(!nl->Equal(avlType, a4)){
      return listutils::typeError("avltype and type of arg 4 differ");
    }
    return nl->TwoElemList(nl->SymbolAtom(Symbol::STREAM()),
                           nl->Second(a2));
}

/*

5.18.3  The Value Mapping Functions of operator ~mrange~

*/

template<class T, class R>
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
            MemoryRelObject* mro = getMemRel(relN, nl->Second(qp->GetType(s)));
            if(!mro){
              return 0;
            }
            Attribute* key1 = (Attribute*) args[2].addr;
            Attribute* key2 = (Attribute*) args[3].addr;
            T* treeN = (T*) args[0].addr;

            ListExpr subtype = qp->GetType(qp->GetSon(s,2));
            memAVLtree* avltree = getAVLtree(treeN, subtype);
            if(avltree) {
              local.addr= new avlOperLI(avltree,
                                      mro->getmmrel(),
                                      key1,key2,
                                      nl->ToString(subtype));
            }
            else {
              MemoryTTreeObject* ttree = getTtree(treeN);
              if(ttree) {
                local.addr= new avlOperLI(ttree->getttree(),
                                      mro->getmmrel(),
                                      key1,key2,
                                      nl->ToString(subtype));
              }
              else {
                return 0;
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
    mrangeVMT<CcString, CcString>,
    mrangeVMT<CcString, Mem>,
    mrangeVMT<Mem, CcString>,
    mrangeVMT<Mem, Mem>
};

int mrangeSelect(ListExpr args){
   int n1 = CcString::checkType(nl->First(args))?0:2;
   int n2 = CcString::checkType(nl->Second(args))?0:1;
   return n1+n2;
}

/*

5.18.4 Description of operator ~mrange~

*/

OperatorSpec mrangeSpec(
    "{string,mem(avltree T) } x {string, mem(rel(tuple(X)))} "
    "x T x T -> stream(Tuple(X)) ",
    "_ _ mrange[_,_]",
    "Uses the given avl-tree to find all tuples"
    " in the given relation which are between "
     "the first and the second attribute value.",
    "query 'Staedte_SName' 'Staedte' mrange ['Aachen','Dortmund'] count"
);

/*

5.18.5 Instance of operator ~mrange~

*/
Operator mrangeOp (
    "mrange",
    mrangeSpec.getStr(),
    4,
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
ListExpr mexactmatchSTM(ListExpr args){
  string err =" (mem (avl T)) x T  expected";
  if(!nl->HasLength(args,2)){
     return listutils::typeError(err + " (wrong number of args)");
  }
  string errMsg;
  ListExpr a1 = nl->First(args);
  if(!getMemType(nl->First(a1), nl->Second(a1), a1, errMsg)){
    return listutils::typeError(err + "\n problem in 1st arg: " + errMsg);
  }
  a1 = nl->Second(a1);
  if(!MemoryAVLObject::checkType(a1)){
    return listutils::typeError(err + " (1st arg i not an avl tree)");
  }
  if(!nl->Equal(nl->Second(a1), nl->First(nl->Second(args)))){
     return listutils::typeError("avl type and search type differ");
  }
  return nl->TwoElemList( listutils::basicSymbol<Stream<TupleIdentifier> >(),
                          listutils::basicSymbol<TupleIdentifier>());
}

ListExpr mrangeSTM(ListExpr args){
  string err =" (mem (avl T)) x T x T  expected";
  if(!nl->HasLength(args,3)){
     return listutils::typeError(err + " (wrong number of args)");
  }
  string errMsg;
  ListExpr a1 = nl->First(args);
  if(!getMemType(nl->First(a1), nl->Second(a1), a1, errMsg)){
    return listutils::typeError(err + "\n problem in 1st arg: " + errMsg);
  }
  a1 = nl->Second(a1);
  if(!MemoryAVLObject::checkType(a1)){
    return listutils::typeError(err + " (1st arg i not an avl tree)");
  }
  if(!nl->Equal(nl->Second(a1), nl->First(nl->Second(args)))){
     return listutils::typeError("avl type and search type  1 differ");
  }
  if(!nl->Equal(nl->Second(a1), nl->First(nl->Third(args)))){
     return listutils::typeError("avl type and search type 2 differ");
  }
  
  return nl->TwoElemList( listutils::basicSymbol<Stream<TupleIdentifier> >(),
                          listutils::basicSymbol<TupleIdentifier>());
}


class AVLOpS{

  public:
     AVLOpS(memAVLtree* _tree, 
            Attribute* _beg, Attribute* _end): 
        tree(_tree), end(_end){
        it = tree->tail(avlPair(_beg,0));
     }

     TupleIdentifier* next(){
         if(it.onEnd()){
           return 0;
         } 
         const avlPair* p = *it;
         it++;
         int cmp = p->first->Compare(end);
         if(cmp>0){
           return 0;
         }
         return new TupleIdentifier(true,p->second);
     }


   private:
       memAVLtree* tree;
       Attribute* end;
       avlIterator it;
};


template<class T>
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
        local.addr = new AVLOpS(tree,beg,end);
        return 0;
     }
     case REQUEST: {
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


ValueMapping mrangeSVM[] = {
   mrangeSVMT<CcString>,
   mrangeSVMT<Mem>
};

int mrangeSSelect(ListExpr args){
  return CcString::checkType(nl->First(args))?0:1;
}


OperatorSpec mexactmatchSSpec(
  "{string, mem(avltree T) x T -> stream(tid) ",
  "_ memexactmatchS[_]",
  "Retrieves the tuple ids from an avl-tree whose "
  "keys have the value given by the second arg.",
  "query \"strassen_Name\" mexactmatch[\"Hirzerweg\"] count"
);

OperatorSpec mrangeSSpec(
  "{string, mem(avltree T) x T x T -> stream(tid)",
  "_ mrangeS[_,_] ",
  "Retrieves the tuple ids from an avl-tree whose key "
  "is within the range defined by the last two arguments.",
  "query \"strassen_Name\" mrangeS[\"A\",\"B\"] count"
);

Operator mexactmatchSOp(
  "mexactmatchS",
  mexactmatchSpec.getStr(),
  2,
  mrangeSVM,
  mrangeSSelect,
  mexactmatchSTM
);

Operator mrangeSOp(
  "mrangeS",
  mrangeSpec.getStr(),
  2,
  mrangeSVM,
  mrangeSSelect,
  mrangeSTM
);


ListExpr matchbelowSTM(ListExpr args){
  string err =" (mem (avl T)) x T  expected";
  if(!nl->HasLength(args,2)){
     return listutils::typeError(err + " (wrong number of args)");
  }
  string errMsg;
  ListExpr a1 = nl->First(args);
  if(!getMemType(nl->First(a1), nl->Second(a1), a1, errMsg)){
    return listutils::typeError(err + "\n problem in 1st arg: " + errMsg);
  }
  a1 = nl->Second(a1);
  if(!MemoryAVLObject::checkType(a1)){
    return listutils::typeError(err + " (1st arg i not an avl tree)");
  }
  if(!nl->Equal(nl->Second(a1), nl->First(nl->Second(args)))){
     return listutils::typeError("avl type and search type differ");
  }
  return nl->TwoElemList( listutils::basicSymbol<Stream<TupleIdentifier> >(),
                          listutils::basicSymbol<TupleIdentifier>());
}


template<class T>
int matchbelowSVMT (Word* args, Word& result,
                    int message, Word& local, Supplier s) {

   TupleIdentifier* tid = (TupleIdentifier*) local.addr;
   switch (message){
      case OPEN: {
           if(tid){
             delete tid;
             local.addr=0;
           }
           T* treeN = (T*) args[0].addr;
           memAVLtree* tree = getAVLtree(treeN, qp->GetType(qp->GetSon(s,1)));
           if(!tree){
             return 0;
           }
           Attribute* attr = (Attribute*) args[1].addr;
           avlPair searchP(attr, numeric_limits<size_t>::max()); 
           const avlPair* p = tree->GetNearestSmallerOrEqual(searchP);
           if(p){
              local.addr = new TupleIdentifier(true,p->second);
           } 
           return 0;
      }
      case REQUEST : {
             result.addr = tid;
             local.addr = 0;
             return result.addr?YIELD:CANCEL;
      }
      case CLOSE :{
             if(tid){
                delete tid;
                local.addr = 0;
             }
             return 0;
      }

   }
   return -1;
}

ValueMapping matchbelowSVM[] = {
   matchbelowSVMT<CcString>,
   matchbelowSVMT<Mem>
};

int matchbelowSSelect(ListExpr args){
  return CcString::checkType(nl->First(args))?0:1;
}

OperatorSpec matchbelowSSpec(
  "{string, mem(avltree T) x T -> stream(TID) ",
  "_ matchbelowS[_]",
  "Returns the tid of the entry in the avl tree whose key "
  "is smaller or equal to the key attribute. If the given "
  "key is smaller than all stored values, the resulting stream "
  "will be empty.",
  "query \"strassen_Name\" matchbelowS[\"B\"] count"
);

Operator matchbelowSOp(
  "matchbelowS",
  matchbelowSSpec.getStr(),
  2,
  matchbelowSVM,
  matchbelowSSelect,
  matchbelowSTM
);


/*
6 M-tree support


6.1 mcreateMtree2: Creation of an M-tree for a persistent relation

6.1.1 Type Mapping

*/


ListExpr mcreateMtree2TM(ListExpr args){
  string err="expected: stream(tuple) x attrname x attrname x memory name ";
  if(!nl->HasLength(args,4)){
    return listutils::typeError(err+" (wrong number of args)");
  }
  if(!Stream<Tuple>::checkType(nl->First(args))){
    return listutils::typeError(err + " (first arg is not a tuple stream)");
  }
  if(nl->AtomType(nl->Second(args)) != SymbolType){
    return listutils::typeError(err + " (second argument is not a valid "
                                "attribute name)");
  }
  if(nl->AtomType(nl->Third(args)) != SymbolType){
    return listutils::typeError(err + " (third argument is not a valid "
                                "attribute name)");
  }
  if(!CcString::checkType(nl->Fourth(args))){
    return listutils::typeError(err + " (third argument is not a string");
  }

  ListExpr attrList = nl->Second(nl->Second(nl->First(args)));
  string name = nl->SymbolValue(nl->Second(args));
  ListExpr type;
  int index1 = listutils::findAttribute(attrList, name, type);
  if(!index1){
    return listutils::typeError("attribute " + name 
                                + " not part of the tuple");
  }
  string tidname = nl->SymbolValue(nl->Third(args));
  ListExpr tidtype;
  int index2 = listutils::findAttribute(attrList, tidname, tidtype);
  if(!index2){
     return listutils::typeError("attribute " + tidname 
                                 + "not known in tuple");
  }
  if(!TupleIdentifier::checkType(tidtype)){
     return listutils::typeError("attribute " + tidname 
                                 + " not of type tid");
  }

  // check for supported type, extend if required
  ListExpr resType = nl->TwoElemList(
                        listutils::basicSymbol<Mem>(),
                        nl->TwoElemList(
                           listutils::basicSymbol<
                             MemoryMtreeObject<Point, StdDistComp<Point> > >(),
                           type
                        ));


  ListExpr result = nl->ThreeElemList(
                     nl->SymbolAtom(Symbols::APPEND()),
                     nl->ThreeElemList( 
                         nl->IntAtom(index1-1),
                         nl->IntAtom(index2-1),
                         nl->StringAtom(nl->ToString(type))),
                     resType);

  int no = mtreehelper::getTypeNo(type,9);
  if(no <0){
     return listutils::typeError("there is no known distance fuction for type "
                               + nl->ToString(type));
  }
  return result;
}





/*
6.2 Value Mapping template

*/
template <class T>
int mcreateMtree2VMT (Word* args, Word& result, int message,
                    Word& local, Supplier s) {

   result = qp->ResultStorage(s);
   Mem* res = (Mem*) result.addr;

   CcString* name = (CcString*) args[3].addr;
   if(!name->IsDefined()){
      // invalid name
      res->SetDefined(false);
      return 0;
   }
   string n = name->GetValue();
   if(catalog->isMMObject(n)){
      // name already used
      res->SetDefined(false);
      return 0;
   }

   int index1 = ((CcInt*) args[4].addr)->GetValue(); 
   int index2 = ((CcInt*) args[5].addr)->GetValue(); 
   string tn = ((CcString*) args[6].addr)->GetValue();

   StdDistComp<T> dc;
   MMMTree<pair<T,TupleId>,StdDistComp<T> >* tree = 
           new MMMTree<pair<T,TupleId>,StdDistComp<T> >(4,8,dc);

   Stream<Tuple> stream(args[0]);
   stream.open();
   Tuple* tuple;

   bool flobused = false;
   while( (tuple = stream.request())){
      T* attr = (T*) tuple->GetAttribute(index1);
      TupleIdentifier* tid = (TupleIdentifier*) tuple->GetAttribute(index2);
      if(tid->IsDefined()){
        T copy = *attr;
        flobused = flobused || (copy.NumOfFLOBs()>0);
        pair<T,TupleId> p(copy, tid->GetTid());
        tree->insert(p);
      }
      tuple->DeleteIfAllowed();
   }
   stream.close();
   size_t usedMem = tree->memSize();
   ListExpr typeList = nl->TwoElemList( 
                            listutils::basicSymbol<Mem>(),
                            nl->TwoElemList(
                                 nl->SymbolAtom(mtreehelper::BasicType()),
                                 nl->SymbolAtom(tn)));

   MemoryMtreeObject<T,StdDistComp<T> >* mtree = 
          new MemoryMtreeObject<T, StdDistComp<T> > (tree,  
                             usedMem, 
                             nl->ToString(typeList), 
                             !flobused, getDBname());
   bool success = catalog->insert(n, mtree);
   res->set(success, n);
   return 0;
}

/*
6.3 Selection and  Value Mapping Array

*/
int mcreateMtree2Select(ListExpr args){
   ListExpr attrList = nl->Second(nl->Second(nl->First(args)));
   string attrName = nl->SymbolValue(nl->Second(args));
   ListExpr type;
   listutils::findAttribute(attrList, attrName, type);
   return mtreehelper::getTypeNo(type,9);
   assert(false);
   return -1; // invalid
}

 // note: if adding attributes with flobs, the value mapping must be changed

ValueMapping mcreateMtree2VM[] = {
  mcreateMtree2VMT<mtreehelper::t1>,
  mcreateMtree2VMT<mtreehelper::t2>,
  mcreateMtree2VMT<mtreehelper::t3>,
  mcreateMtree2VMT<mtreehelper::t4>,
  mcreateMtree2VMT<mtreehelper::t5>,
  mcreateMtree2VMT<mtreehelper::t6>,
  mcreateMtree2VMT<mtreehelper::t7>,
  mcreateMtree2VMT<mtreehelper::t8>,
  mcreateMtree2VMT<mtreehelper::t9>
};

OperatorSpec mcreateMtree2Spec(
  "stream(tuple) x attrname x attrname x string -> mem(mtree X) ",
  "elements mcreateMtreeSpec[ indexAttr, TID_attr, mem_name]",
  "creates an main memory m tree from a tuple stream",
  "query kinos feed addid mcreateMtree2[GeoData, TID, \"kinos_mtree\"]"
);

Operator mcreateMtree2Op(
   "mcreateMtree2",
   mcreateMtree2Spec.getStr(),
   4,
   mcreateMtree2VM,
   mcreateMtree2Select,
   mcreateMtree2TM
);


/*
Operator ~mdistRange2~

This operator creates a stream of TupleIDs that are inside a 
given distance to a reference object. The used index is a 
main memory based mtree.

*/
ListExpr mdistRange2TM(ListExpr args){

  string err = "{string, mem(mtree T)}  x T x real expected";
  if(!nl->HasLength(args,3)){
     return listutils::typeError(err + " ( wrong number of args)");
  }

  string errMsg;
  ListExpr a1 = nl->First(args);
  if(!getMemType(nl->First(a1), nl->Second(a1), a1, errMsg)){
    return listutils::typeError(err + "\n error in first arg: " + errMsg);
  }
  a1 = nl->Second(a1);
  ListExpr mt = nl->TwoElemList(
                          nl->SymbolAtom(mtreehelper::BasicType()),
                          nl->First(nl->Second(args)));

  if(!nl->Equal(a1, mt)){
    return listutils::typeError("arg 1 is not an mtree over arg 2 (" 
                                + nl->ToString(nl->First(nl->Second(args)))
                                + ")");
  }
  if(!CcReal::checkType(nl->First(nl->Third(args)))){
     return listutils::typeError(err + " (third arg is not a real)");
  }

  return nl->TwoElemList(
                listutils::basicSymbol<Stream<TupleIdentifier> >(),
                listutils::basicSymbol<TupleIdentifier> ());
}


template <class T, class N>
int mdistRange2VMT (Word* args, Word& result, int message,
                    Word& local, Supplier s) {

  RangeIterator<pair<T,TupleId>, StdDistComp<T>  >* li 
              = (RangeIterator<pair<T,TupleId>, StdDistComp<T> >*) local.addr;
  switch(message){
    case OPEN: {
            if(li) {
              delete li;
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
            N* Name = (N*) args[0].addr;
            typedef  MemoryMtreeObject<T,StdDistComp<T> > mtot;
            mtot* mtreeo = getMtree<N,T>(Name);
            if(!mtreeo){
              return 0;
            }            
            typedef MMMTree<pair<T,TupleId>,StdDistComp<T> > mtt;
            mtt* mtree = mtreeo->getmtree();
            if(mtree){
                T a = *attr;
                local.addr = mtree->rangeSearch(pair<T,TupleId>(a,0), d);
            }
            return 0;
          }
     case REQUEST: {
               if(!li){
                 result.addr=0;
                 return CANCEL;
               }
               const pair<T,TupleId>* n = li->next();
               result.addr = n? new TupleIdentifier(true,n->second) : 0;
               return result.addr?YIELD:CANCEL;
            }
     case CLOSE:
               if(li){
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
   int m = 9;
   int n;
   if(CcString::checkType(nl->First(args))){
      n = 0;
   } else {
      n = m;
   }

   int res =  mtreehelper::getTypeNo(type,m) + n;

   return res;

}

 // note: if adding attributes with flobs, the value mapping must be changed

ValueMapping mdistRange2VM[] = {
  mdistRange2VMT<mtreehelper::t1, CcString>,
  mdistRange2VMT<mtreehelper::t2, CcString>,
  mdistRange2VMT<mtreehelper::t3, CcString>,
  mdistRange2VMT<mtreehelper::t4, CcString>,
  mdistRange2VMT<mtreehelper::t5, CcString>,
  mdistRange2VMT<mtreehelper::t6, CcString>,
  mdistRange2VMT<mtreehelper::t7, CcString>,
  mdistRange2VMT<mtreehelper::t8, CcString>,
  mdistRange2VMT<mtreehelper::t9, CcString>,

  mdistRange2VMT<mtreehelper::t1, Mem>,
  mdistRange2VMT<mtreehelper::t2, Mem>,
  mdistRange2VMT<mtreehelper::t3, Mem>,
  mdistRange2VMT<mtreehelper::t4, Mem>,
  mdistRange2VMT<mtreehelper::t5, Mem>,
  mdistRange2VMT<mtreehelper::t6, Mem>,
  mdistRange2VMT<mtreehelper::t7, Mem>,
  mdistRange2VMT<mtreehelper::t8, Mem>,
  mdistRange2VMT<mtreehelper::t9, Mem>
};

OperatorSpec mdistRange2Spec(
  "string x DATA x real -> stream(tid) ",
  "mem_mtree mdistRange2[keyAttr, maxDist] ",
  "Retrieves those tuple ids from an mtree those key value has "
  "a maximum distaance of the given dist",
  "query \"kinos_mtree\" mdistRange2[ alexanderplatz , 2000.0] count"
);

Operator mdistRange2Op(
   "mdistRange2",
   mdistRange2Spec.getStr(),
   18,
   mdistRange2VM,
   mdistRange2Select,
   mdistRange2TM
);


/*
Operator ~mdistScan2~

This operator creates a stream of TupleIDs 
whose associated objects are in increasing order
to the reference object.

*/
ListExpr mdistScan2TM(ListExpr args){
  string err = "{string, mem(mtree (T))}  x T  expected";
  if(!nl->HasLength(args,2)){
     return listutils::typeError(err + " ( wrong number of args)");
  }
  ListExpr a1 = nl->First(args);
  ListExpr a2 = nl->Second(args);
  string errMsg;
  if(!getMemType(nl->First(a1), nl->Second(a1), a1,  errMsg)){
    return listutils::typeError(err + "\n problem in 1st arg: " + errMsg);
  }
  a1 = nl->Second(a1); // remove mem
  a2 = nl->First(a2);  // extract type
  if(!mtreehelper::checkType(a1,a2)){
     return listutils::typeError(err+ "( first arg is not an "
                                      "mtree over key type)");
  }
  return nl->TwoElemList( listutils::basicSymbol<Stream<TupleIdentifier> >(),
                          listutils::basicSymbol<TupleIdentifier>());


}


template <class T, class N>
int mdistScan2VMT (Word* args, Word& result, int message,
                    Word& local, Supplier s) {

  NNIterator<pair<T,TupleId>, StdDistComp<T>  >* li 
                = (NNIterator<pair<T,TupleId>, StdDistComp<T> >*) local.addr;
  switch(message){
    case OPEN: {
            if(li) {
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
            MMMTree<pair<T, TupleId>,StdDistComp<T> >* mtree = 
                    tree->getmtree();
            if(mtree){
              pair<T,TupleId> p(*attr,0);
              local.addr = mtree->nnSearch(p);
            }
            return 0;
          }
     case REQUEST: {
               if(!li){
                 result.addr=0;
                 return CANCEL;
               }
               const pair<T,TupleId>* n = li->next();
               result.addr = n? new TupleIdentifier(true,n->second) : 0;
               return result.addr?YIELD:CANCEL;
            }
     case CLOSE:
               if(li){
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
   int m = 9;
   if(CcString::checkType(nl->First(args))){
     n = 0;
   } else {
     n = m;
   }

   return mtreehelper::getTypeNo(type,m) + n;
}

 // note: if adding attributes with flobs, the value mapping must be changed

ValueMapping mdistScan2VM[] = {
  mdistScan2VMT<mtreehelper::t1, CcString>,
  mdistScan2VMT<mtreehelper::t2, CcString>,
  mdistScan2VMT<mtreehelper::t3, CcString>,
  mdistScan2VMT<mtreehelper::t4, CcString>,
  mdistScan2VMT<mtreehelper::t5, CcString>,
  mdistScan2VMT<mtreehelper::t6, CcString>,
  mdistScan2VMT<mtreehelper::t7, CcString>,
  mdistScan2VMT<mtreehelper::t8, CcString>,
  mdistScan2VMT<mtreehelper::t9, CcString>,
  
  mdistScan2VMT<mtreehelper::t1, Mem>,
  mdistScan2VMT<mtreehelper::t2, Mem>,
  mdistScan2VMT<mtreehelper::t3, Mem>,
  mdistScan2VMT<mtreehelper::t4, Mem>,
  mdistScan2VMT<mtreehelper::t5, Mem>,
  mdistScan2VMT<mtreehelper::t6, Mem>,
  mdistScan2VMT<mtreehelper::t7, Mem>,
  mdistScan2VMT<mtreehelper::t8, Mem>,
  mdistScan2VMT<mtreehelper::t9, Mem>
};

OperatorSpec mdistScan2Spec(
  "{string, (mem (mtree DATA))}  x DATA -> stream(tid) ",
  "mem_mtree mdistScan2[keyAttr] ",
  "Scans the tuple ids within an m-tree in increasing "
  "distance of the reference object to the associated "
  "objects.",
  "query \"kinos_mtree\" mdistScan2[ alexanderplatz] count"
);

Operator mdistScan2Op(
   "mdistScan2",
   mdistScan2Spec.getStr(),
   18,
   mdistScan2VM,
   mdistScan2Select,
   mdistScan2TM
);



/*
Operator ~mcreateMTree~

This operator creates an m-tree over a main memory relation.

*/
ListExpr mcreateMtreeTM(ListExpr args){

  string err = "string x Ident  x string expected";
  if(!nl->HasLength(args,3)){
    return listutils::typeError(err + " (wrong number of args)");
  }
  ListExpr a1 = nl->First(args);
  ListExpr a2 = nl->Second(args);
  ListExpr a3 = nl->Third(args);
  string errMsg;
  if(!getMemType(nl->First(a1), nl->Second(a1), a1, errMsg)){
    return listutils::typeError("problem in 1st arg : " + errMsg);
  }

  a1 = nl->Second(a1);
  a2 = nl->First(a2);
  a3 = nl->First(a3);

  if(!Relation::checkType(a1)){
    return listutils::typeError(err + " (first arg is not a mem rel");
  }

  if(nl->AtomType(a2)!=SymbolType){
    return listutils::typeError(err + " (second arg is not a valid Id");
  }
  if(!CcString::checkType(a3)){
    return listutils::typeError(err + " (third arg is not a string)");
  }

  // ListExpr resNameV = nl->Second(nl->Third(args));
  // if(nl->AtomType(resNameV)!=StringType){
  //  return listutils::typeError("third arg must be a constant string");
  // }
  // if(catalog->isMMObject(nl->StringValue(resNameV))){
  //  return listutils::typeError("memory object already there.");
  // }

  ListExpr attrList = nl->Second(nl->Second(a1));
  ListExpr at;
  string attrName = nl->SymbolValue(a2);
  int index = listutils::findAttribute(attrList, attrName, at);
  if(!index){
     return listutils::typeError( attrName+ " is not known in tuple");
  }
  int typeNo = mtreehelper::getTypeNo(at,9);
  if(typeNo < 0){
    return listutils::typeError("Type " + nl->ToString(at) + " not supported");
  }

  ListExpr resType = nl->TwoElemList(
                        nl->SymbolAtom(Mem::BasicType()),
                        nl->TwoElemList(
                             nl->SymbolAtom(mtreehelper::BasicType()),
                             at));

  ListExpr result = nl->ThreeElemList(
                          nl->SymbolAtom(Symbols::APPEND()),
                          nl->TwoElemList(nl->IntAtom(index-1),
                                          nl->IntAtom(typeNo)),
                          resType);
  
  return result;
}



template <class T, class R>
int mcreateMtreeVMT (Word* args, Word& result, int message,
                    Word& local, Supplier s) {

   result = qp->ResultStorage(s);
   Mem* res = (Mem*) result.addr;
   R* RelName = (R*) args[0].addr;
   CcString* Name = (CcString*) args[2].addr; 
   if(!RelName->IsDefined() || !Name->IsDefined()){
      res->SetDefined(false);
      return 0;
   }
   string relName = RelName->GetValue();
   string name = Name->GetValue();
   MemoryObject* mmobj = catalog->getMMObject(relName);
   if(!mmobj){
      res->SetDefined(false);
      return 0;
   }
   ListExpr mmType = nl->Second(catalog->getMMObjectTypeExpr(relName));
   if(!Relation::checkType(mmType)){
      res->SetDefined(false);
      return 0;
   }
   if(catalog->isMMObject(name)){
      // name already used
      res->SetDefined(false);
      return 0;
   }

   int index = ((CcInt*) args[3].addr)->GetValue();
   // extract attribute type
   ListExpr attrList = nl->Second(nl->Second(mmType));
   int i2 = index;
   while(!nl->IsEmpty(attrList) && i2>0){
     attrList = nl->Rest(attrList);
     i2--;
   }
   if(nl->IsEmpty(attrList)){
      res->SetDefined(false);
      return 0;
   }
   ListExpr attrType = nl->Second(nl->First(attrList));
   if(!T::checkType(attrType)){
      res->SetDefined(false);
      return 0;
   }
 

   StdDistComp<T> dc;
   MMMTree<pair<T,TupleId>,StdDistComp<T> >* tree = 
           new MMMTree<pair<T,TupleId>,StdDistComp<T> >(4,8,dc);

   MemoryRelObject* mrel = (MemoryRelObject*) mmobj;

   vector<Tuple*>* rel = mrel->getmmrel();

   // insert attributes

   for(size_t i=0;i<rel->size();i++){
       T* attr = (T*) (*rel)[i]->GetAttribute(index);
       pair<T,TupleId> p(*attr,i);
       tree->insert(p); 
   }
   size_t usedMem = tree->memSize();
   MemoryMtreeObject<T,StdDistComp<T> >* mtree = 
          new MemoryMtreeObject<T, StdDistComp<T> > (tree,  
                             usedMem, 
                             nl->ToString(qp->GetType(s)),
                             mrel->hasflob(), getDBname());
   bool success = catalog->insert(name, mtree);
   res->set(success, name);
   
   return 0;
}

ValueMapping mcreatetreeVMA[] = {
  mcreateMtreeVMT<mtreehelper::t1,CcString>,
  mcreateMtreeVMT<mtreehelper::t2,CcString>,
  mcreateMtreeVMT<mtreehelper::t3,CcString>,
  mcreateMtreeVMT<mtreehelper::t4,CcString>,
  mcreateMtreeVMT<mtreehelper::t5,CcString>,
  mcreateMtreeVMT<mtreehelper::t6,CcString>,
  mcreateMtreeVMT<mtreehelper::t7,CcString>,
  mcreateMtreeVMT<mtreehelper::t8,CcString>,
  mcreateMtreeVMT<mtreehelper::t9,CcString>,
  mcreateMtreeVMT<mtreehelper::t1,Mem>,
  mcreateMtreeVMT<mtreehelper::t2,Mem>,
  mcreateMtreeVMT<mtreehelper::t3,Mem>,
  mcreateMtreeVMT<mtreehelper::t4,Mem>,
  mcreateMtreeVMT<mtreehelper::t5,Mem>,
  mcreateMtreeVMT<mtreehelper::t6,Mem>,
  mcreateMtreeVMT<mtreehelper::t7,Mem>,
  mcreateMtreeVMT<mtreehelper::t8,Mem>,
  mcreateMtreeVMT<mtreehelper::t9,Mem>
};


int mcreateMtreeVM (Word* args, Word& result, int message,
                    Word& local, Supplier s) {

  int typeNo = ((CcInt*) args[4].addr)->GetValue();
  int offset = CcString::checkType(qp->GetType(qp->GetSon(s,0)))?0:9;

  return mcreatetreeVMA[typeNo+ offset](args,result,message,local,s);
}

OperatorSpec mcreateMtreeSpec(
  "(string, mem(rel))  x attrname x string -> string ",
  "memrel  mcreateMtree[ indexAttr, mem_name]",
  "creates an main memory m tree from a main memory relation",
  "query \"mkkinos\"  mcreateMtree[GeoData, \"kinos_mtree\"]"
);

Operator mcreateMtreeOp(
   "mcreateMtree",
   mcreateMtreeSpec.getStr(),
   mcreateMtreeVM,
   Operator::SimpleSelect,
   mcreateMtreeTM
);

/*
Operator mdistRange

*/
ListExpr mdistRangeTM(ListExpr args){
   string err="{string, (mem mtree)} x {string x DATA x real expected";
   if(!nl->HasLength(args,4)){
      return listutils::typeError(err + " (wrong number of args)");
   }

   ListExpr a1 = nl->First(args);
   ListExpr a2 = nl->Second(args);
   string errMsg;
   if(!getMemType(nl->First(a1), nl->Second(a1), a1, errMsg)){
     return listutils::typeError(err + "\n problen in 1st arg: " + errMsg);
   }
   if(!getMemType(nl->First(a2), nl->Second(a2), a2, errMsg)){
     return listutils::typeError(err + "\n problen in 2nd arg: " + errMsg);
   }

   a1 = nl->Second(a1);
   a2 = nl->Second(a2);
   ListExpr a3 = nl->First(nl->Third(args));
   ListExpr a4 = nl->First(nl->Fourth(args));

   if(!mtreehelper::checkType(a1,a3)){
     return listutils::typeError("first arg is not a mtree over " 
                                 + nl->ToString(a1));
   }
   if(!Relation::checkType(a2)){
     return listutils::typeError("second arg is not a relation");
   }

   if(!CcReal::checkType(a4)){
     return listutils::typeError(err + "(4th arg is not a real)");
   }

   return nl->TwoElemList(
                 listutils::basicSymbol<Stream<Tuple> >(),
                 nl->Second(a2)); 
}

template<class T>
class distRangeInfo{
  public:

     distRangeInfo( MemoryMtreeObject<T, StdDistComp<T> >* mtree, 
                    MemoryRelObject* mrel, 
                    T* ref, 
                    double dist){
                 
                rel = mrel->getmmrel();
                pair<T,TupleId> p(*ref,0);
                it = mtree->getmtree()->rangeSearch(p,dist);
              }

     ~distRangeInfo(){
       delete it;
     }

     Tuple* next(){
        while(true){
            const pair<T,TupleId>* p = it->next();
            if(!p){ return 0;}
            if(p->second < rel->size()){
               Tuple* res = (*rel)[p->second];
               res->IncReference();
               return res;
            }
        }
        return 0;
     }


  private:
     vector<Tuple*>* rel;
     RangeIterator<pair<T,TupleId> , StdDistComp<T> >* it;

};

template<class K, class T, class R>
int mdistRangeVMT (Word* args, Word& result, int message,
                    Word& local, Supplier s) {
   distRangeInfo<K>* li = (distRangeInfo<K>*) local.addr;
   switch(message){
     case OPEN : {
               if(li){
                 delete li;
                 local.addr = 0;
               }
               R* relN = (R*) args[1].addr;
               MemoryRelObject* rel = getMemRel(relN, 
                                         nl->Second(qp->GetType(s)));
               if(!rel){
                 return 0;
               }
               CcReal* dist = (CcReal*) args[3].addr;
               if(!dist->IsDefined()){
                 return 0;
               }
               double d = dist->GetValue();
               if(d<0){
                  return 0;
               }
               
               T* treeN = (T*) args[0].addr;
               MemoryMtreeObject<K, StdDistComp<K> >* m = getMtree<T,K>(treeN);
               if(!m){
                 return 0;
               }
               K* key = (K*) args[2].addr;
               local.addr = new distRangeInfo<K>(m,rel,key,d);
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

int mdistRangeSelect(ListExpr args){
   ListExpr typeL = nl->Third(args);
   int type =  mtreehelper::getTypeNo(typeL,9);
   int o1 = CcString::checkType(nl->First(args))?0:18;
   int o2 = CcString::checkType(nl->Second(args))?0:9;
   return type + o1 + o2;
}

 // note: if adding attributes with flobs, the value mapping must be changed

ValueMapping mdistRangeVM[] = {
  mdistRangeVMT<mtreehelper::t1,CcString,CcString>,
  mdistRangeVMT<mtreehelper::t2,CcString,CcString>,
  mdistRangeVMT<mtreehelper::t3,CcString,CcString>,
  mdistRangeVMT<mtreehelper::t4,CcString,CcString>,
  mdistRangeVMT<mtreehelper::t5,CcString,CcString>,
  mdistRangeVMT<mtreehelper::t6,CcString,CcString>,
  mdistRangeVMT<mtreehelper::t7,CcString,CcString>,
  mdistRangeVMT<mtreehelper::t8,CcString,CcString>,
  mdistRangeVMT<mtreehelper::t9,CcString,CcString>,

  mdistRangeVMT<mtreehelper::t1,CcString,Mem>,
  mdistRangeVMT<mtreehelper::t2,CcString,Mem>,
  mdistRangeVMT<mtreehelper::t3,CcString,Mem>,
  mdistRangeVMT<mtreehelper::t4,CcString,Mem>,
  mdistRangeVMT<mtreehelper::t5,CcString,Mem>,
  mdistRangeVMT<mtreehelper::t6,CcString,Mem>,
  mdistRangeVMT<mtreehelper::t7,CcString,Mem>,
  mdistRangeVMT<mtreehelper::t8,CcString,Mem>,
  mdistRangeVMT<mtreehelper::t9,CcString,Mem>,
  
  mdistRangeVMT<mtreehelper::t1,Mem,CcString>,
  mdistRangeVMT<mtreehelper::t2,Mem,CcString>,
  mdistRangeVMT<mtreehelper::t3,Mem,CcString>,
  mdistRangeVMT<mtreehelper::t4,Mem,CcString>,
  mdistRangeVMT<mtreehelper::t5,Mem,CcString>,
  mdistRangeVMT<mtreehelper::t6,Mem,CcString>,
  mdistRangeVMT<mtreehelper::t7,Mem,CcString>,
  mdistRangeVMT<mtreehelper::t8,Mem,CcString>,
  mdistRangeVMT<mtreehelper::t9,Mem,CcString>,

  mdistRangeVMT<mtreehelper::t1,Mem,Mem>,
  mdistRangeVMT<mtreehelper::t2,Mem,Mem>,
  mdistRangeVMT<mtreehelper::t3,Mem,Mem>,
  mdistRangeVMT<mtreehelper::t4,Mem,Mem>,
  mdistRangeVMT<mtreehelper::t5,Mem,Mem>,
  mdistRangeVMT<mtreehelper::t6,Mem,Mem>,
  mdistRangeVMT<mtreehelper::t7,Mem,Mem>,
  mdistRangeVMT<mtreehelper::t8,Mem,Mem>,
  mdistRangeVMT<mtreehelper::t9,Mem,Mem>,


};

OperatorSpec mdistRangeSpec(
  "{string, mem(mtree T)} x {string, mem(rel(tuple))}  x T  "
  "x real -> stream(tuple) ",
  "mem_mtree mem_rel mdistRange[keyAttr, maxDist] ",
  "Retrieves those tuples from a memory relation "
  "having a distance smaller or equals to a given dist "
  "to a key value. This operation is aided by a memory "
  "based m-tree.",
  "query \"kinos_mtree\" \"Kinos\" mdistRange[ alexanderplatz , 2000.0] count"
);

Operator mdistRangeOp(
   "mdistRange",
   mdistRangeSpec.getStr(),
   36,
   mdistRangeVM,
   mdistRangeSelect,
   mdistRangeTM
);


/*
Operator mdistScan

*/
ListExpr mdistScanTM(ListExpr args){
   string err="{string, mem} x {string,mem} x DATA expected";
   if(!nl->HasLength(args,3)){
      return listutils::typeError(err + " (wrong number of args)");
   }
   ListExpr a1 = nl->First(args);
   string errMsg;
   if(!getMemType(nl->First(a1), nl->Second(a1), a1, errMsg)){
     return listutils::typeError(err + "\n problem in first arg: " + errMsg);
   }
   ListExpr a2 = nl->Second(args);
   if(!getMemType(nl->First(a2), nl->Second(a2), a2, errMsg)){
     return listutils::typeError(err + "\n problem in second arg: " + errMsg);
   }
   a1 = nl->Second(a1);
   a2 = nl->Second(a2);
   ListExpr a3 = nl->First(nl->Third(args));

   if(!mtreehelper::checkType(a1,a3)){
     return listutils::typeError(err + "(first arg is not a mtree over " 
                                 + nl->ToString(a3) + ")");
   }
   if(!Relation::checkType(a2)){
     return listutils::typeError(err + "( second arg is not a mem relation)");
   }
   
   return nl->TwoElemList(
                 listutils::basicSymbol<Stream<Tuple> >(),
                 nl->Second(a2)); 
}

template<class T>
class distScanInfo{
  public:

     distScanInfo( MemoryMtreeObject<T, StdDistComp<T> >* mtree, 
                    MemoryRelObject* mrel, 
                    T* ref){
                 
                rel = mrel->getmmrel();
                pair<T,TupleId> p(*ref,0);
                it = mtree->getmtree()->nnSearch(p);
              }

     ~distScanInfo(){
       delete it;
     }

     Tuple* next(){
        while(true){
            const pair<T,TupleId>* p = it->next();
            if(!p){ return 0;}
            if(p->second < rel->size()){
               Tuple* res = (*rel)[p->second];
               res->IncReference();
               return res;
            }
        }
        return 0;
     }


  private:
     vector<Tuple*>* rel;
     NNIterator<pair<T,TupleId> , StdDistComp<T> >* it;

};

template<class K, class T, class R>
int mdistScanVMT (Word* args, Word& result, int message,
                    Word& local, Supplier s) {
   distScanInfo<K>* li = (distScanInfo<K>*) local.addr;
   switch(message){
     case OPEN : {
        if(li){
          delete li;
          local.addr = 0;
        }
        R* relN = (R*) args[1].addr;
        MemoryRelObject* mro=getMemRel(relN, nl->Second(qp->GetType(s)));
        if(!mro){
          return 0;
        }

        K* key = (K*) args[2].addr;

        T* tree = (T*) args[0].addr;
        MemoryMtreeObject<K, StdDistComp<K> >*m = getMtree<T,K>(tree);
        if(m){
          local.addr = new distScanInfo<K>(m,mro,key);
        }
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

int mdistScanSelect(ListExpr args){
   ListExpr type = nl->Third(args);
   int m = 9;
   int keyTypeNo = mtreehelper::getTypeNo(type,m);
   int n1 = CcString::checkType(nl->First(args))?0:2*m;
   int n2 = CcString::checkType(nl->Second(args))?0:m;

   return keyTypeNo + n1 + n2;

}

 // note: if adding attributes with flobs, the value mapping must be changed

ValueMapping mdistScanVM[] = {
  mdistScanVMT<mtreehelper::t1, CcString, CcString>,
  mdistScanVMT<mtreehelper::t2, CcString, CcString>,
  mdistScanVMT<mtreehelper::t3, CcString, CcString>,
  mdistScanVMT<mtreehelper::t4, CcString, CcString>,
  mdistScanVMT<mtreehelper::t5, CcString, CcString>,
  mdistScanVMT<mtreehelper::t6, CcString, CcString>,
  mdistScanVMT<mtreehelper::t7, CcString, CcString>,
  mdistScanVMT<mtreehelper::t8, CcString, CcString>,
  mdistScanVMT<mtreehelper::t9, CcString, CcString>,

  mdistScanVMT<mtreehelper::t1, CcString, Mem>,
  mdistScanVMT<mtreehelper::t2, CcString, Mem>,
  mdistScanVMT<mtreehelper::t3, CcString, Mem>,
  mdistScanVMT<mtreehelper::t4, CcString, Mem>,
  mdistScanVMT<mtreehelper::t5, CcString, Mem>,
  mdistScanVMT<mtreehelper::t6, CcString, Mem>,
  mdistScanVMT<mtreehelper::t7, CcString, Mem>,
  mdistScanVMT<mtreehelper::t8, CcString, Mem>,
  mdistScanVMT<mtreehelper::t9, CcString, Mem>,

  mdistScanVMT<mtreehelper::t1, Mem, CcString>,
  mdistScanVMT<mtreehelper::t2, Mem, CcString>,
  mdistScanVMT<mtreehelper::t3, Mem, CcString>,
  mdistScanVMT<mtreehelper::t4, Mem, CcString>,
  mdistScanVMT<mtreehelper::t5, Mem, CcString>,
  mdistScanVMT<mtreehelper::t6, Mem, CcString>,
  mdistScanVMT<mtreehelper::t7, Mem, CcString>,
  mdistScanVMT<mtreehelper::t8, Mem, CcString>,
  mdistScanVMT<mtreehelper::t9, Mem, CcString>,

  mdistScanVMT<mtreehelper::t1, Mem, Mem>,
  mdistScanVMT<mtreehelper::t2, Mem, Mem>,
  mdistScanVMT<mtreehelper::t3, Mem, Mem>,
  mdistScanVMT<mtreehelper::t4, Mem, Mem>,
  mdistScanVMT<mtreehelper::t5, Mem, Mem>,
  mdistScanVMT<mtreehelper::t6, Mem, Mem>,
  mdistScanVMT<mtreehelper::t7, Mem, Mem>,
  mdistScanVMT<mtreehelper::t8, Mem, Mem>,
  mdistScanVMT<mtreehelper::t9, Mem, Mem>

};

OperatorSpec mdistScanSpec(
  "{string, mem(mtree T)} x {string, mem(rel(tuple))}  x T -> stream(tuple) ",
  "mem_mtree mem_rel mdistScan[keyAttr] ",
  "Retrieves tuples from an memory relation in increasing "
  "distance to a reference object aided by a memory based "
  "m-tree.",
  "query \"kinos_mtree\" \"Kinos\" mdistScan[ alexanderplatz] consume"
);

Operator mdistScanOp(
   "mdistScan",
   mdistScanSpec.getStr(),
   36,
   mdistScanVM,
   mdistScanSelect,
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
   if(!getMemType(type, value,res, error)){
     return listutils::typeError(error);
   }
   return res;
}

/*

7.1.2 Value Mapping Function of operator ~mwrap~

*/
int mwrapVM(Word* args, Word& result,
             int message, Word& local, Supplier s) {
   CcString* arg = (CcString*) args[0].addr;
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
   mwrapVM,
   Operator::SimpleSelect,
   mwrapTM
);




ListExpr MTUPLETM(ListExpr args){
   if(!nl->HasMinLength(args,1)){
      return listutils::typeError("too less arguments");
   }
   ListExpr first = nl->First(args);
   if(!Mem::checkType(first)){
     return listutils::typeError("not a mem object");
   }
   ListExpr res = nl->Second(first);
   if(Relation::checkType(res)){
     return nl->Second(res);
   }
   if(listutils::isOrelDescription(res)){
     return nl->Second(res);
   }
   if(MemoryGraphObject::checkType(res)){
     return nl->Second(res);
   }
   return listutils::typeError(
        "Memory object is not a relation or ordered relation");
}

OperatorSpec MTUPLESpec(
   "mem(rel(X)) -> X or string -> tuple",
   "MTUPLE(_)",
   "Retrieves the tuple type of a memory relation.",
   "query \"ten\" mupdatebyid[[const tid value 5]; No: .No + 10000] count"
);

Operator MTUPLEOp(
    "MTUPLE",
    MTUPLESpec.getStr(),
    0,
    Operator::SimpleSelect,
    MTUPLETM
);





/*
7.2 Operator ~mcreatettree~

The operator creates a TTree over a given main memory relation.

7.2.1 Type Mapping Functions of operator ~mcreatettree~
        {string, mem(rel(...))} x ident -> mem(ttree string)

        the first parameter identifies the main memory relation, 
        the second parameter identifies the attribute

*/

// TODO memory leak
ListExpr mcreatettreeTypeMap(ListExpr args){
  
    if(nl->ListLength(args)!=2){
     return listutils::typeError("two arguments expected");
    }

    ListExpr first = nl->First(args);
    ListExpr second = nl->Second(args);
    ListExpr rel;

    string errMsg;
    if(!getMemType(nl->First(first), nl->Second(first), rel, errMsg)){
      return listutils::typeError("problem in 1st arg: " + errMsg);
    }
    rel = nl->Second(rel);

    if(!Relation::checkType(rel)){
       return listutils::typeError("memory object is not a relation");
    }
    
    
    if(nl->AtomType(nl->First(second))!=SymbolType){
       return listutils::typeError("second argument is not a valid "
                                   "attribute name");
    }

    string attrName = nl->SymbolValue(nl->First(second));
    ListExpr attrType = 0;
    int attrPos = 0;
    ListExpr attrList = nl->Second(nl->Second(rel));
    attrPos = listutils::findAttribute(attrList, attrName, attrType);

    if (attrPos == 0){
        return listutils::typeError
        ("there is no attribute having  name " + attrName);
    }

    ListExpr resType = nl->TwoElemList(
                          listutils::basicSymbol<Mem>(),
                          nl->TwoElemList(
                             nl->SymbolAtom(ttreehelper::BasicType()),
                             attrType
                       ));
    
    return nl->ThreeElemList(
                nl->SymbolAtom(Symbols::APPEND()),
                nl->TwoElemList(nl->IntAtom(attrPos - 1),
                                nl->StringAtom(attrName)),
                resType);
}

/*

7.2.3  Value Mapping Functions of operator ~mcreatettree~

*/
template<class T>
int mcreatettreeValueMap(Word* args, Word& result,
                int message, Word& local, Supplier s) {
  
  result  = qp->ResultStorage(s);
  Mem* str = static_cast<Mem*>(result.addr);

  // the main memory relation
  T* rel = (T*) args[0].addr;
  if(!rel->IsDefined()){
      str->SetDefined(false);
      return 0;
  }
  string name = rel->GetValue();

  int attrPos = ((CcInt*) args[2].addr)->GetValue();

  ListExpr type = catalog->getMMObjectTypeExpr(name);
  type = nl->Second(type);

  if(!Relation::checkType(type)){
    cerr << "object " << name  << "is not a relation" << endl;
    str->SetDefined(false);
    return false;
  } 

  // extract attribute 
  ListExpr attrList = nl->Second(nl->Second(type));
  int pos = attrPos;
  while(!nl->IsEmpty(attrList) && pos>0){
    attrList = nl->Rest(attrList);
    pos--;
  }
  if(nl->IsEmpty(attrList)){
    cerr << "not enough attributes in tuple";
    str->SetDefined(false);
    return 0;
  }
  ListExpr relattrType = nl->Second(nl->First(attrList));
  ListExpr ttreeattrType = nl->Second(nl->Second(qp->GetType(s)));

  if(!nl->Equal(relattrType, ttreeattrType)){
    cerr << "expected type and type in relation differ" << endl;
    str->SetDefined(false);
    return 0;
  }
  // extract name of relation
  string attrName = ((CcString*)args[3].addr)->GetValue();
  // create name for ttree object
  string resName = name + "_" + attrName;
  if(catalog->isMMObject(resName)){
    cerr << "object " << resName << "already exists" << endl;
    str->SetDefined(false);
    return 0;
  }
  // get main memory relation
  MemoryRelObject* mmrel =
      (MemoryRelObject*)catalog->getMMObject(name);
  
  vector<Tuple*>* relation = mmrel->getmmrel();
  vector<Tuple*>::iterator it = relation->begin();
  unsigned int i = 1;

  memttree* tree = new memttree(8,10);
  size_t usedMainMemory = 0;
  unsigned long availableMemSize = catalog->getAvailableMemSize();

  // insert (Attribute, TupleId)-pairs into ttree
  while(it != relation->end()) {
      Tuple* tup = *it;
      Attribute* attr = tup->GetAttribute(attrPos);
      ttreePair tPair = ttreePair(attr->Copy(),i);

      // size for a pair is 16 bytes, plus an additional pointer 8 bytes
      size_t entrySize = 24;
      if (entrySize<availableMemSize){
          tree->insert(tPair);
//           tPair.first->DeleteIfAllowed();
          usedMainMemory += (entrySize);
          availableMemSize -= (entrySize);
          it++;
          i++;
      } 
      else {
        cout << "there is not enough main memory available"
                " to create an TTree" << endl;
        delete tree;
        str->SetDefined(false);
        return 0;
      }
  }
  
  MemoryTTreeObject* ttreeObject = 
        new MemoryTTreeObject(tree, 
                              usedMainMemory,
                              nl->ToString(qp->GetType(s)),
                              mmrel->hasflob(), 
                              getDBname());
  catalog->insert(resName,ttreeObject);

  str->set(true, resName);
  return 0;
}

/*

7.2.4 Value Mapping Array and Selection

*/
ValueMapping mcreatettreeVM[] =
{
    mcreatettreeValueMap<CcString>,
    mcreatettreeValueMap<Mem>,
};

int mcreatettreeSelect(ListExpr args){
    // string case at index 0
    if ( CcString::checkType(nl->First(args)) ){
       return 0;
    }
    // Mem(rel(tuple))case at index 1
    if (Mem::checkType(nl->First(args)) ){
       return 1;
    }
    // should never be reached
    return -1;
  }

/*

7.2.5 Description of operator ~mcreatettree~

*/
OperatorSpec mcreatettreeSpec(
    "string x string -> string || mem(rel(tuple(X))) x string -> string",
    "_ mcreatettree [_]",
    "creates an T-Tree over a main memory relation given by the"
    "first argument string || mem(rel(tuple)) and an attribute "
    "given by the second argument",
    "query \"Staedte\" mcreatettree [SName]"
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
  MInsertTTree,
  MDeleteTTree
};

template<ChangeTypeTree ct>
ListExpr minsertttreeTypeMap(ListExpr args){
  
  if(nl->ListLength(args)!=3)
    return listutils::typeError("three arguments expected");
  
  // process stream
  ListExpr first = nl->First(args); //stream + query
  ListExpr stream = nl->First(first);

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
  if(!listutils::isSymbol(nl->Second(next),TupleIdentifier::BasicType()))
    return listutils::typeError("last attribute in the tuple must be a tid");
  
  // process ttree
  ListExpr second = nl->Second(args);
  string errMsg;

  if(!getMemType(nl->First(second), nl->Second(second), second, errMsg))
    return listutils::typeError("\n problem in second arg: " + errMsg);
  
  ListExpr ttree = nl->Second(second); // remove leading mem
  if(!ttreehelper::checkType(ttree))
    return listutils::typeError("second arg is not a mem ttree");

  
  
  // check attribute
  ListExpr third = nl->Third(args);
  if(nl->AtomType(nl->First(third))!=SymbolType)
      return listutils::typeError("third argument is not a valid "
                                  "attribute name");

  string attrName = nl->SymbolValue(nl->First(third));
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
            nl->OneElemList(append),
            stream);
}


template<ChangeTypeTree ct>
class minsertttreeInfo {
  public:
    minsertttreeInfo(Word w, memttree* _ttree, int _attrPos)
      : stream(w), ttree(_ttree), attrPos(_attrPos) {
      stream.open();
    }

    ~minsertttreeInfo(){
      stream.close();
    }

    
    Tuple* next(){
      Tuple* res = stream.request();
      if(!res) { 
        return 0; 
      }
      
      Attribute* tidAttr = res->GetAttribute(res->GetNoAttributes() - 1);
      TupleId oldTid = ((TupleIdentifier*)tidAttr)->GetTid();
      
      if(ct==MInsertTTree) {
        Attribute* attr = res->GetAttribute(attrPos-1);
        ttreePair tPair = ttreePair(attr->Copy(),oldTid);
        ttree->insert(tPair);
//         tPair.first->DeleteIfAllowed();
      }
      
      if(ct==MDeleteTTree) {
        Attribute* attr = res->GetAttribute(attrPos-1);
        ttreePair tPair = ttreePair(attr->Copy(),oldTid);
        ttree->remove(tPair);
//         tPair.first->DeleteIfAllowed();
      }
      return res;
    }

  private:
     Stream<Tuple> stream;
     memttree* ttree;
     int attrPos;
};


/*

7.3.2  The Value Mapping Functions of operator ~minsertttree~
       and ~mdeletettree~

*/
template<class T, ChangeTypeTree ct>
int minsertttreeValueMap (Word* args, Word& result,
                    int message, Word& local, Supplier s) {
  
minsertttreeInfo<ct>* li = (minsertttreeInfo<ct>*) local.addr;
  
  switch(message) {
    
    case OPEN : {
      
      if(li) {
        delete li;
        local.addr = 0;
      }

      T* tree = (T*) args[1].addr;
      if(!tree->IsDefined()){
          cout << "undefined" << endl;
          return 0;
      }
      string name = tree->GetValue();
      // cut blank from the front
      name = name.substr(1, name.size()-1);
      
      MemoryTTreeObject* mmttree =
      (MemoryTTreeObject*)catalog->getMMObject(name);
      if(!mmttree) {
        return 0;
      }
      
      ListExpr attrlist = nl->Second(nl->Second(qp->GetType(s)));  // stream
      ListExpr attrToSort = qp->GetType(qp->GetSon(s,2));
      ListExpr attrType = 0;
      int attrPos = 0;
      
      attrPos = listutils::findAttribute(attrlist, 
                                         nl->ToString(attrToSort), 
                                         attrType);
      if(attrPos == 0) {
        return listutils::typeError
          ("there is no attribute having name " + nl->ToString(attrToSort));
      }
        
      local.addr = new minsertttreeInfo<ct>(args[0],
                                            mmttree->getttree(),
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
    minsertttreeValueMap<CcString,MInsertTTree>,
    minsertttreeValueMap<Mem,MInsertTTree>,
    minsertttreeValueMap<CcString,MDeleteTTree>,
    minsertttreeValueMap<Mem,MDeleteTTree>
};

template<ChangeTypeTree ct>
int minsertttreeSelect(ListExpr args){
  if(ct==MInsertTTree) 
    return CcString::checkType(nl->First(args))?0:1;
  else if(ct==MDeleteTTree) 
    return CcString::checkType(nl->First(args))?2:3;
}

/*
7.3.4 Description of operator ~minsertttree~

*/

OperatorSpec minsertttreeSpec(
    "stream(tuple(x@[TID:tid])) x {string, mem(ttree)} x ident "
    "-> stream(tuple(x@[TID:tid]))",
    "_ op [_,_]",
    "inserts an object into a main memory ttree",
    "query ten feed head[5] minsert[\"ten\"] "
    "minsertttree[\"ten_No\", No] count"
);

/*
7.3.5 Instance of operator ~minsertttree~

*/
Operator minsertttreeOp (
    "minsertttree",
    minsertttreeSpec.getStr(),
    6,
    minsertttreeVM,
    minsertttreeSelect<MInsertTTree>,
    minsertttreeTypeMap<MInsertTTree>
);

/*
7.4.4 Description of operator ~mdeletettree~

*/

OperatorSpec mdeletettreeSpec(
    "stream(tuple(x@[TID:tid])) x {string, mem(ttree)} x ident "
    "-> stream(tuple(x@[TID:tid]))",
    "_ op [_,_]",
    "deletes an object identified by tupleid from a main memory ttree",
    "query ten feed head[5] mdelete[\"ten\"] " 
    "mdeletettree[\"ten_No\", No] count"
);

/*
7.4.5 Instance of operator ~mdeletettree~

*/
Operator mdeletettreeOp (
    "mdeletettree",
    mdeletettreeSpec.getStr(),
    6,
    minsertttreeVM,
    minsertttreeSelect<MDeleteTTree>,
    minsertttreeTypeMap<MDeleteTTree>
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
ListExpr mcreateAuxiliaryRelTypeMap(const ListExpr& args,
                                    const string opName) {

  if(nl->ListLength(args) != 1) {
    return listutils::typeError("one argument expected");
  }
  
  ListExpr first = nl->First(args);
  if(!nl->HasLength(first,2))
    return listutils::typeError("internal error");
  
  string errMsg;

  if(!getMemType(nl->First(first), nl->Second(first), first, errMsg)){
    return listutils::typeError(
      "string or mem(rel) or mem(orel) expected : " + errMsg);
  }

  // check for rel
  bool isOrel;
  first = nl->Second(first); // remove mem
  if(Relation::checkType(first)) {
    isOrel = false;
  }
  else if(listutils::isOrelDescription(first)) {
    isOrel = true;
  }
  else {
    return listutils::typeError(
    "first arg is not a memory relation or ordered relation");
  }

  // build first part of the result-tupletype
  ListExpr rest = nl->Second(nl->Second(first));
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
  }
  else {
    outlist = nl->TwoElemList(
                listutils::basicSymbol<Mem>(),
                nl->ThreeElemList(
                  nl->SymbolAtom(OREL),
                  nl->TwoElemList(  
                    nl->SymbolAtom(Tuple::BasicType()),
                    listn),
                  nl->Third(first)));
  }
  return outlist;
}

ListExpr mcreateinsertrelTypeMap(ListExpr args) {
  return mcreateAuxiliaryRelTypeMap(args, "mcreateinsertrel");
}

ListExpr mcreatedeleterelTypeMap(ListExpr args) {
  return mcreateAuxiliaryRelTypeMap(args, "mcreatedeleterel");
}

ListExpr mcreateupdaterelTypeMap(ListExpr args) {
  return mcreateAuxiliaryRelTypeMap(args, "mcreateupdaterel");
}

/*
7.6.2 General Value mapping function of operators ~mcreateinsertrel~, 
       ~mcreatedeleterel~ and ~mcreateupdaterel~

*/
template<class T>
int mcreateinsertrelValueMap(Word* args, Word& result, int message,
                             Word& local, Supplier s) {
  result = qp->ResultStorage(s);
  return 0;
}

/*
7.6.3 Value Mapping Array and Selection

*/
ValueMapping mcreateinsertrelVM[] = {
  mcreateinsertrelValueMap<CcString>,
  mcreateinsertrelValueMap<Mem>
};

int mcreateinsertrelSelect(ListExpr args){
  return CcString::checkType(nl->First(args))?0:1;
}


/*
7.6.4 Specification of operator ~mcreateinsertrel~

*/
OperatorSpec mcreateinsertrelSpec(
    "{string, mem(rel(...))} -> mem(rel(tuple(x@[TID:tid]))) ",
    "mcreateinsertrel(_)",
    "creates an auxiliary relation",
    "query memlet (\"fuenf\", mcreateinsertrel(\"ten\"))"
);


/*
7.6.5 Definition of operator ~mcreateinsertrel~

*/
Operator mcreateinsertrelOp (
  "mcreateinsertrel",             // name
  mcreateinsertrelSpec.getStr(),  // specification
  2,
  mcreateinsertrelVM,             // value mapping
  mcreateinsertrelSelect,         // trivial selection function
  mcreateinsertrelTypeMap         // type mapping
);


/*
7.7.4  Specification of operator ~mcreatedeleterel~

*/
OperatorSpec mcreatedeleterelSpec(
    "{string, mem(rel(...))} -> mem(rel(tuple(x@[TID:tid]))) ",
    "mcreatedeleterel(_)",
    "creates an auxiliary relation",
    "query memlet (\"fuenf\", mcreatedeleterel(\"ten\"))"
);

/*
7.7.5  Definition of operator ~mcreatedeleterel~

*/
Operator mcreatedeleterelOp(
  "mcreatedeleterel",             // name
  mcreatedeleterelSpec.getStr(),  // specification
  2,
  mcreateinsertrelVM,             // value mapping
  mcreateinsertrelSelect,         // trivial selection function
  mcreatedeleterelTypeMap         // type mapping
);


/*
7.8.4 Specification of operator ~mcreateupdaterel~

*/

OperatorSpec mcreateupdaterelSpec(
    "{string, mem(rel(...))} -> "
    "mem(rel(tuple(x@[(a1_old x1)...(an_old xn)(TID:tid)]))) ",
    "mcreateupdaterel(_)",
    "creates an auxiliary relation",
    "query memlet (\"fuenf\", mcreateupdaterel(\"ten\"))"
);

    
/*
7.8.5 Definition of operator ~mcreateupdaterel~

*/
Operator mcreateupdaterelOp (
  "mcreateupdaterel",             // name
  mcreateupdaterelSpec.getStr(),  // specification
  2,
  mcreateinsertrelVM,             // value mapping
  mcreateinsertrelSelect,         // selection function
  mcreateupdaterelTypeMap         // type mapping
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

ListExpr minsertTypeMap(ListExpr args) {
  
  if((nl->ListLength(args)!=2) && (nl->ListLength(args)!=3)) {
    return listutils::typeError("wrong number of arguments");
  }
  
  ListExpr first = nl->First(args); //stream + query
  ListExpr stream = nl->First(first);

  // process stream
  if(!listutils::isTupleStream(stream)){
    return listutils::typeError("first argument must be a tuple stream");
  }
  
  string errMsg;
  
  // process second argument (mem(rel))
  ListExpr second  = nl->Second(args);
  if(!getMemType(nl->First(second), nl->Second(second), second, errMsg)) {
      return listutils::typeError("(problem in second arg: " + errMsg + ")");
  }
  second = nl->Second(second);
  if((!Relation::checkType(second)) && 
     (!listutils::isOrelDescription(second))){
    return listutils::typeError(
      "second arg is not a memory relation");
  }
  if(!nl->Equal(nl->Second(stream), nl->Second(second))){
      return listutils::typeError("stream type and mem relation type differ");
  }
  
  // process third argument (minsertsave only)
  if(nl->ListLength(args)==3) {
    ListExpr third  = nl->Third(args);
    if(!getMemType(nl->First(third), nl->Second(third), third, errMsg)) {
      return listutils::typeError("(problem in third arg: " + errMsg + ")");
    }
    third = nl->Second(third);
    if(!Relation::checkType(third)){
      return listutils::typeError(
	"third arg is not a memory relation");
    }
  }
  
  // append tupleidentifier
  ListExpr rest = nl->Second(nl->Second(second));
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
  
  return nl->TwoElemList(nl->SymbolAtom(Symbol::STREAM()),
                         nl->TwoElemList(
                           nl->SymbolAtom(Tuple::BasicType()),
                           listn));
}


class minsertInfo {
  public:
     minsertInfo(Word w, vector<Tuple*>* _relation, 
                 bool _flob, TupleType* _type)
        : stream(w),relation(_relation), flob(_flob), type(_type) {
        stream.open();
     }

    ~minsertInfo(){
       stream.close();
//        type->DeleteIfAllowed;     TODO
     }

     Tuple* next(){
       Tuple* res = stream.request();
       if(!res) { return 0; }
       if(flob)
         res->bringToMemory();
       
       Tuple* newtup = new Tuple(type); 
       for(int i = 0; i < res->GetNoAttributes(); i++)
         newtup->CopyAttribute(i,res,i);
       res->IncReference();
       
       //get tuple id and append it to tuple
       const TupleId& tid = res->GetTupleId();
       Attribute* tidAttr = new TupleIdentifier(true,tid);
       newtup->PutAttribute(res->GetNoAttributes(), tidAttr);
       relation->push_back(res);
      
       res->DeleteIfAllowed();
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
  
  minsertInfo* li = (minsertInfo*) local.addr;
  
  switch (message) {
    
    case OPEN : {
      
      if(li) {
        delete li;
        local.addr=0;
      }
      
      qp->Open(args[0].addr);
      
      T* oN = (T*) args[1].addr;   
      MemoryRelObject* rel = getMemRel(oN);
      if(!rel) return 0;
      
      TupleType* tt = new TupleType(nl->Second(GetTupleResultType(s)));
      
      local.addr = new minsertInfo(args[0],rel->getmmrel(),rel->hasflob(),tt);
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
   minsertValMap<CcString>,
   minsertValMap<Mem>
};

int minsertSelect(ListExpr args){
   return CcString::checkType(nl->Second(args))?0:1;
}

/*
7.9.4 Description of operator ~minsert~

*/

OperatorSpec minsertSpec(
    "stream(tuple(x)) x {string, mem(rel(...))} -> stream(tuple(x@[TID:tid]))",
    "_ minsert [_]",
    "inserts the tuples of a stream into an "
    "existing main memory relation. All tuples get an additional "
    "attribute of type 'tid'",
    "query minsert (ten feed head[5],\"ten\") count"
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
    minsertTypeMap
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
     minsertsaveInfo(Word w, vector<Tuple*>* _relation, 
		     vector<Tuple*>* _auxrel, 
                 bool _flob, TupleType* _type)
        : stream(w),relation(_relation), auxrel(_auxrel), 
          flob(_flob), type(_type) {
        stream.open();
     }
     
    ~minsertsaveInfo(){
       stream.close();
     }

    Tuple* next(){
      Tuple* res = stream.request();
      if(!res) return 0;
      
      if(flob)
        res->bringToMemory();
      
      //get tuple id and append it to tuple
      Tuple* newtup = new Tuple(type); 
      for(int i = 0; i < res->GetNoAttributes(); i++)
        newtup->CopyAttribute(i,res,i);
      const TupleId& tid = res->GetTupleId();
      Attribute* tidAttr = new TupleIdentifier(true,tid);
      newtup->PutAttribute(res->GetNoAttributes(), tidAttr);
      
      // insert tuple in memory relation 
      relation->push_back(res);
      res->IncReference();
      // insert tuple in auxrel
      auxrel->push_back(newtup);
      newtup->IncReference();
       
      res->DeleteIfAllowed();
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
// TODO memory leaks
template<class T>
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
      
      T* oN = (T*) args[1].addr;
      T* aux = (T*) args[2].addr;
      
      MemoryRelObject* rel = getMemRel(oN);;
      if(!rel) { return 0; }
      MemoryRelObject* auxrel = getMemRel(aux);;
      if(!auxrel) { return 0; }
      
      TupleType* tt = new TupleType(nl->Second(GetTupleResultType(s)));
      
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
   minsertsaveValueMap<CcString>,
   minsertsaveValueMap<Mem>
};

int minsertsaveSelect(ListExpr args){
   return CcString::checkType(nl->Second(args))?0:1;
}

/*
7.10.4 Description of operator ~minsertsave~

*/
OperatorSpec minsertsaveSpec(
    "stream(tuple(x)) x {string, mem(rel(...))} x {string, mem(rel(...))} "
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
    2,
    minsertsaveVM,
    minsertsaveSelect,
    minsertTypeMap
);


/*
7.11 Operator ~minserttuple~

This Operator inserts the given list of attributes as a new tuple into 
a main memory relation.

7.11.1 General type mapping function of operators ~minserttuple~
      and ~minserttuplesave~.
      
*/
ListExpr minserttupleTypeMap(ListExpr args) {

  
  if((nl->ListLength(args)!=2) && (nl->ListLength(args)!=3)) {
    return listutils::typeError("two arguments expected");
  }

  // process first argument (mem(rel))
  ListExpr first = nl->First(args);
  if(!nl->HasLength(first,2)){
    return listutils::typeError("internal error");
  }
  string errMsg;
  if(!getMemType(nl->First(first), nl->Second(first), first, errMsg)){
    return listutils::typeError("string or mem(rel) expected : " + errMsg);
  }
  first = nl->Second(first); // remove mem
  if(!Relation::checkType(first)){
    return listutils::typeError("first arg is not a memory relation");
  }
    
  // check tuple
  ListExpr second = nl->Second(args);
  if(nl->AtomType(second)!=NoAtom){
    return listutils::typeError("list as second arg expected");
  }
  if(nl->ListLength(nl->Second(first))!=nl->ListLength(second)){
    return listutils::typeError("different lengths in tuple and update");
  }
  
  // process third argument (minserttuplesave only)
  if(nl->ListLength(args)==3) {
    ListExpr third  = nl->Third(args);
    if(!getMemType(nl->First(third), nl->Second(third), third, errMsg)) {
      return listutils::typeError("(problem in third arg: " + errMsg + ")");
    }
    third = nl->Second(third);
    if(!Relation::checkType(third)){
      return listutils::typeError(
        "third arg is not a memory relation");
    }
  }
    
  ListExpr restrel = nl->Second(nl->Second(first));
  ListExpr resttuple = nl->First(second);

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
  
  return nl->TwoElemList(nl->SymbolAtom(Symbol::STREAM()),
                          nl->TwoElemList(
                            nl->SymbolAtom(Tuple::BasicType()),
                            listn));
}


class minserttupleInfo{
  public:
    minserttupleInfo(vector<Tuple*>* _relation, bool _flob,
                     ListExpr _listType, TupleType* _type, Word _tupleList)
      : relation(_relation), flob(_flob), 
        listType(_listType) ,type(_type), tupleList(_tupleList) {
      
      firstcall = true;
    }
    
    ~minserttupleInfo() {
      type->DeleteIfAllowed();
    }
    
    Tuple* next() {
      
      if(!firstcall) return 0;
      else firstcall = false;
      
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
      Supplier supplier = tupleList.addr;
      
      Word attrValue;
      Supplier s;
      for(int i=0; i<res->GetNoAttributes()-1; i++) {
        s = qp->GetSupplier(supplier, i);
        qp->Request(s,attrValue);
        Attribute* attr = (Attribute*) attrValue.addr;
        res->PutAttribute(i,attr->Clone());
        tup->CopyAttribute(i,res,i);
      }
      tup->SetTupleId(relation->size()+1);
      // add TID
      const TupleId& tid = tup->GetTupleId();
      Attribute* tidAttr = new TupleIdentifier(true,tid);
      res->PutAttribute(res->GetNoAttributes()-1, tidAttr);
      res->IncReference();
      
      if(flob)
        tup->bringToMemory();
      
      // add Tuple to memory relation
      relation->push_back(tup);
      tup->IncReference();
      return res;
    }

private:
    vector<Tuple*>* relation;
    bool flob;
    ListExpr listType;
    TupleType* type;
    Word tupleList;
    bool firstcall;
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
      qp->SetModified(qp->GetSon(s, 0));
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
  minserttupleValMap<CcString>,
  minserttupleValMap<Mem>
};

int minserttupleSelect(ListExpr args){
  return CcString::checkType(nl->First(args))?0:1;
}


/*
7.11.4 Description of operator ~minserttuple~

*/
OperatorSpec minserttupleSpec(
    "{string, mem(rel(...))} x [t1 ... tn] -> stream(tuple(x@[TID:tid]))",
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
    minserttupleTypeMap
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
                         Word _tupleList)
      : relation(_relation), auxrel(_auxrel), flob(_flob), 
        listType(_listType) ,type(_type), tupleList(_tupleList) {
      
      firstcall = true;
    }
    
    ~minserttuplesaveInfo() {
      type->DeleteIfAllowed();
    }
    
    Tuple* next() {
      
      if(!firstcall) return 0;
      else firstcall = false;
      
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
      Supplier supplier = tupleList.addr;
      
      Word attrValue;
      Supplier s;
      for(int i=0; i<res->GetNoAttributes()-1; i++) {
        s = qp->GetSupplier(supplier, i);
        qp->Request(s,attrValue);
        Attribute* attr = (Attribute*) attrValue.addr;
        res->PutAttribute(i,attr->Clone());
        tup->CopyAttribute(i,res,i);
      }
      tup->SetTupleId(relation->size()+1);
      
      // add TID
      const TupleId& tid = tup->GetTupleId();
      Attribute* tidAttr = new TupleIdentifier(true,tid);
      res->PutAttribute(res->GetNoAttributes()-1, tidAttr);
      res->IncReference();
      
      // flob
      if(flob)
        tup->bringToMemory();
      
      // add Tuple to memory relation
      relation->push_back(tup);
      tup->IncReference();
      // add tuple to auxrel
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
};

/*
7.12.2  Value Mapping Function of operator ~minserttuplesave~

*/
template<class T>
int minserttuplesaveValueMap (Word* args, Word& result,
                    int message, Word& local, Supplier s) {

  minserttuplesaveInfo* li = (minserttuplesaveInfo*) local.addr;

  switch (message) {
    case OPEN: {
      
      if(li) {
        delete li;
        local.addr=0;
      }
      
      T* oN = (T*) args[0].addr;

      ListExpr tupleType = GetTupleResultType(s);
      TupleType* tt = new TupleType(nl->Second(tupleType));
      
      T* aux = (T*) args[2].addr;
      
      MemoryRelObject* rel = getMemRel(oN);
      if(!rel) { return 0; }
      MemoryRelObject* auxrel = getMemRel(aux);
      if(!auxrel) { return 0; }
      local.addr = new minserttuplesaveInfo(rel->getmmrel(),
                                            auxrel->getmmrel(),
                                            rel->hasflob(),
                                            tupleType,tt,args[1]);
      qp->SetModified(qp->GetSon(s, 0));  // correct ?
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
  minserttuplesaveValueMap<CcString>,
  minserttuplesaveValueMap<Mem>
};

int minserttuplesaveSelect(ListExpr args){
  return CcString::checkType(nl->First(args))?0:1;
}


/*
7.12.4 Description of operator ~minserttuplesave~

*/
OperatorSpec minserttuplesaveSpec(
    "{string, mem(rel(tuple(x)))} x [t1 ... tn] x "
    "{string, mem(rel(tuple(x@[TID:tid])))} -> stream(tuple(x@[TID:tid]))",
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
    2,
    minserttuplesaveVM,
    minserttuplesaveSelect,
    minserttupleTypeMap
);



/*
7.13 Operator ~mdelete~

  stream(tuple(x)) x {string, mem(rel(...))} -> stream(tuple(x@[TID:tid]))

*/
void remove(vector<Tuple*>* relation, Tuple* res) {
    
  for (size_t i = 0; i < relation->size(); i++) {
    Tuple* tup = relation->at(i);
    if(tup){
      for(int j=0; j<tup->GetNoAttributes(); j++) {
        int cmp = ((Attribute*)tup->GetAttribute(j))->Compare(
                  ((Attribute*)res->GetAttribute(j)));
        if(cmp!=0) 
          break;
        else {
          relation->erase(relation->begin()+i);
          return;
        }
      }
    }
  }
}

class mdeleteInfo {
public:
mdeleteInfo(Word w, vector<Tuple*>* _relation, 
            bool _flob, TupleType* _type)
  : stream(w),relation(_relation), flob(_flob), type(_type) {
  stream.open();
}

    ~mdeleteInfo(){
       stream.close();
     }
     
     Tuple* next() {
       Tuple* res = stream.request();
       if(!res) return 0;
       
       Tuple* newtup = new Tuple(type); 
       for(int i = 0; i < res->GetNoAttributes(); i++)
         newtup->CopyAttribute(i,res,i);
       
       //get tuple id and append it to tuple
       const TupleId& tid = res->GetTupleId();
       Attribute* tidAttr = new TupleIdentifier(true,tid);
       newtup->PutAttribute(res->GetNoAttributes(), tidAttr);
       newtup->IncReference();  // ?
       
       // delete from relation
       remove(relation,res);     
       res->DeleteIfAllowed();  
       
       return newtup;
     }

  private:
     Stream<Tuple> stream;
     vector<Tuple*>* relation;
     bool flob;
     TupleType* type;
};

/*

7.13 Operator ~mdelete~

This Operator deletes all tuples of an input stream from a main memory 
relation. All tuple of the input stream are returned in an output 
stream with an additional Attribut containing the tupleid.

7.13.2 Value Mapping Function of operator ~mdelete~

*/
// TODO memory leak
template<class T>
int mdeleteValMap (Word* args, Word& result,
                    int message, Word& local, Supplier s) {
  
  mdeleteInfo* li = (mdeleteInfo*) local.addr;
  
  switch (message) {
    case OPEN : {
      
      if(li) {
        delete li;
        local.addr=0;
      }
      
      qp->Open(args[0].addr);
      
      T* oN = (T*) args[1].addr;
      MemoryRelObject* rel = getMemRel(oN);;
      if(!rel) { return 0; }
      
      TupleType* tt = new TupleType(nl->Second(GetTupleResultType(s)));
      
      local.addr = new mdeleteInfo(args[0],rel->getmmrel(),
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

ValueMapping mdeleteVM[] = {
   mdeleteValMap<CcString>,
   mdeleteValMap<Mem>
};

int mdeleteSelect(ListExpr args){
   return CcString::checkType(nl->Second(args))?0:1;
}

/*
7.13.4 Description of operator ~mdelete~

*/
OperatorSpec mdeleteSpec(
    "stream(tuple(x)) x {string, mem(rel(...))} -> stream(tuple(x@[TID:tid]))",
    "_ mdelete [_]",
    "deletes the tuple of a stream from an "
    "existing main memory relation",
    "query ten feed filter [.No = 2] mdelete['ten'] count"
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
    minsertTypeMap
);


/*
7.14 Operator ~mdeletesave~

The operator ~mdeletesave~ deletes all tuples of an input stream from
a main memory relation. It returns the stream, all tuples now containing an
additional attribut of type 'tid' and adds the tuples to an auxiliary 
main memory relation.

*/
class mdeletesaveInfo {
  public:
    mdeletesaveInfo(Word w, vector<Tuple*>* _relation, 
                    vector<Tuple*>* _auxrel, 
                bool _flob, TupleType* _type)
      : stream(w),relation(_relation), auxrel(_auxrel), 
        flob(_flob), type(_type) {
      stream.open();
    }
    
  ~mdeletesaveInfo(){
      stream.close();
    }

    
     
    Tuple* next() {
      Tuple* res = stream.request();
      if(!res) { return 0; }
      if(flob)
        res->bringToMemory();
      
      Tuple* newtup = new Tuple(type); 
      for(int i = 0; i < res->GetNoAttributes(); i++)
        newtup->CopyAttribute(i,res,i);
      const TupleId& tid = res->GetTupleId();
      Attribute* tidAttr = new TupleIdentifier(true,tid);
      newtup->PutAttribute(res->GetNoAttributes(), tidAttr);
      res->IncReference();
      
      // delete from relation
      remove(relation,res);
      
      // add to auxrel
      auxrel->push_back(newtup);
      newtup->IncReference();
              
      res->DeleteIfAllowed(); 
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
7.14.2 Value Mapping Function of operator ~mdeletesave~

*/
// TODO memory leaks
template<class T>
int mdeletesaveValueMap (Word* args, Word& result,
                         int message, Word& local, Supplier s) {
  
  mdeletesaveInfo* li = (mdeletesaveInfo*) local.addr;
  
  switch (message) {
    
    case OPEN : {
      
      if(li) {
        delete li;
        local.addr=0;
      }
      
      qp->Open(args[0].addr);
      T* oN = (T*) args[1].addr;
      T* aux = (T*) args[2].addr;
      
      MemoryRelObject* rel = getMemRel(oN);
      if(!rel) { return 0; }
      MemoryRelObject* auxrel = getMemRel(aux);
      if(!auxrel) { return 0; }
      
      TupleType* tt = new TupleType(nl->Second(GetTupleResultType(s)));
      
      local.addr = new mdeletesaveInfo(args[0],rel->getmmrel(),
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

ValueMapping mdeletesaveVM[] = {
   mdeletesaveValueMap<CcString>,
   mdeletesaveValueMap<Mem>
};

int mdeletesaveSelect(ListExpr args){
   return CcString::checkType(nl->Second(args))?0:1;
}

/*
7.14.4 Description of operator ~mdeletesave~

*/
OperatorSpec mdeletesaveSpec(
    "stream(Tuple) x {string, mem(rel)}  -> stream(Tuple)",
    "mdelete(_,_)",
    "deletes the tuple of a stream from an "
    "existing main memory relation",
    "query mdelete (ten feed head[5],\"ten\") count"
);

/*
7.1.5 Instance of operator ~mdeletesave~

*/
Operator mdeletesaveOp (
    "mdeletesave",
    mdeletesaveSpec.getStr(),
    2,
    mdeletesaveVM,
    mdeletesaveSelect,
    minsertTypeMap
);



// find tuple in memory relation 
Tuple* find(vector<Tuple*>* relation, TupleIdentifier* tid) {
  vector<Tuple*>::iterator it = relation->begin();
  Tuple* tup;
  
  while(it != relation->end()) {
    tup = *it;
    if(tid->GetTid() == tup->GetTupleId()) {
      return tup;
    }
    it++;
  }
  return 0;
}
    
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

  // process first arg
  ListExpr first = nl->First(args);
  if(!nl->HasLength(first,2)){
      return listutils::typeError("internal error");
  }
  string errMsg;
  if(!getMemType(nl->First(first), nl->Second(first), first, errMsg)){
    return listutils::typeError("string or mem(rel) expected : " + errMsg);
  }
  first = nl->Second(first); // remove mem
  if(!Relation::checkType(first)){
    return listutils::typeError("first arg is not a memory relation");
  }

  // process second arg (tid)
  ListExpr second = nl->First(nl->Second(args));
  if(!listutils::isSymbol(second,TupleIdentifier::BasicType())){
    return listutils::typeError("second argument must be a tid");
  }
  
  ListExpr rest = nl->Second(nl->Second(first));
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
                    TupleType* _type)
      : relation(_relation), tid(_tid), type(_type) {
        firstcall = true;
      }
      
    ~mdeletebyidInfo() {}
     
    Tuple* next() {
      
      if(firstcall) {
        firstcall = false;
      Tuple* res = find(relation,tid);
      if(!res) return 0;
      
      Tuple* newtup = new Tuple(type); 
      for(int i=0; i<res->GetNoAttributes(); i++)
        newtup->CopyAttribute(i,res,i);
      res->IncReference();
      const TupleId& tid = res->GetTupleId();
      Attribute* tidAttr = new TupleIdentifier(true,tid);
      newtup->PutAttribute(res->GetNoAttributes(), tidAttr);
      
      remove(relation,res);
      
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
        TupleType* tt = new TupleType(nl->Second(GetTupleResultType(s)));
        
        local.addr = new mdeletebyidInfo(rel->getmmrel(),tid,tt);
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
  mdeletebyidValMap<CcString>,
  mdeletebyidValMap<Mem>
};

int mdeletebyidSelect(ListExpr args){
  return CcString::checkType(nl->First(args))?0:1;
}


/*
7.15.4 Description of operator ~mdeletebyid~

*/
OperatorSpec mdeletebyidSpec(
    "{string, mem(rel(tuple(x)))} x (tid) -> stream(tuple(x@[TID:tid]))] ",
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
    mdeletebyidSpec.getStr(),
    2,
    mdeletebyidVM,
    mdeletebyidSelect,
    mdeletebyidTypeMap
);


enum UpdateOp {
  MUpdate,
  MUpdateSave,
  MUpdateByID
};

template<UpdateOp uo>
void update(vector<Tuple*>* relation, Tuple* res, 
            int changedIndex, Attribute* attr) {
      
  vector<int>* attrPos = new vector<int>();
  attrPos->push_back(changedIndex+1);
  
  vector<Tuple*>::iterator it = relation->begin();
  
  while(it != relation->end()) {
    Tuple* tup = *it;
    if(TupleComp::equal(res,tup,attrPos)) {

      for(int i=0; i<res->GetNoAttributes(); i++) {
        if(i==changedIndex) {
          tup->PutAttribute(i,attr->Clone());
          return;
        }
      }
    }
    it++;
  }
  attrPos->clear();
  delete attrPos;
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

*/
    
ListExpr mupdateTypeMap(ListExpr args) {
  
  if((nl->ListLength(args)!=3) && (nl->ListLength(args)!=4)) {
    return listutils::typeError("wrong number of arguments");
  }
  
  // process stream
  ListExpr first = nl->First(args); //stream + query
  ListExpr stream = nl->First(first);
  if(!listutils::isTupleStream(stream)){
    return listutils::typeError("first argument must be a tuple stream");
  }
  
  // process second argument (mem(rel))
  string errMsg;
  ListExpr second = nl->Second(args);
  if(!getMemType(nl->First(second), nl->Second(second), second, errMsg)) {
      return listutils::typeError("(problem in second arg : " 
                                  + errMsg + ")");
  }
  second = nl->Second(second);
  if(!Relation::checkType(second)) {
    return listutils::typeError("second arg is not a memory relation");
  }
  if(!nl->Equal(nl->Second(stream), nl->Second(second))){
      return listutils::typeError("stream type and mem relation type differ");
  }
  
  // process third argument (only for mupdatesave)
  if(nl->ListLength(args)==4) {
    ListExpr third = nl->Third(args);
    if(!getMemType(nl->First(third), nl->Second(third), third, errMsg)){
       return listutils::typeError( "(problem in third arg : " 
                                    + errMsg + ")");
    }
    
    third = nl->Second(third);
    if(!Relation::checkType(third)) {
      return listutils::typeError("third arg is not a memory relation");
    }
  }
 
  // process update function
  ListExpr map;
  // mupdate
  if(nl->ListLength(args)==3) 
    map = nl->Third(args);
  // mupdatesave
  else
    map = nl->Fourth(args);
  // argument is not a map
  if(nl->ListLength(map<1)) {
    return listutils::typeError("arg must be a list of maps");
  }
  ListExpr maprest = nl->First(map);
  int noAttrs = nl->ListLength(map);
  
  // Go through all functions
  ListExpr mapfirst, mapsecond;
  ListExpr attrType;
  ListExpr indices, indicescurrent;
  
  while (!(nl->IsEmpty(maprest))) {
    map = nl->First(maprest);
    maprest = nl->Rest(maprest);
    mapfirst = nl->First(map);
    mapsecond = nl->Second(map);
    
    // check if argument map is a function
    if(!listutils::isMap<1>(mapsecond)){
      return listutils::typeError("not a map found");
    }
    if(!nl->Equal(nl->Second(stream),nl->Second(mapsecond))) {
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
    attrIndex = listutils::findAttribute(nl->Second(nl->Second(stream)),
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
    bool firstcall = true;
    if(firstcall) {
      indices = nl->OneElemList(nl->IntAtom(attrIndex));
      indicescurrent = indices;
      firstcall = false;
    }
    else 
      indicescurrent = nl->Append(indicescurrent,
                                  nl->IntAtom(attrIndex));
  }
  
  maprest = nl->Second(nl->Second(stream));
  
  ListExpr attrlist = nl->OneElemList(nl->First(maprest));
  ListExpr currentattr = attrlist;
  maprest = nl->Rest(maprest);
  
  while (!(nl->IsEmpty(maprest))) {
    currentattr = nl->Append(currentattr,nl->First(maprest));
    maprest = nl->Rest(maprest);
  }
  
  // build second part of the resultstream
  maprest = nl->Second(nl->Second(stream));
  string oldName;
  ListExpr oldattr;
  while (!(nl->IsEmpty(maprest))) {
    nl->WriteToString(oldName, nl->First(nl->First(maprest)));
    oldName += "_old";
    oldattr = nl->TwoElemList(nl->SymbolAtom(oldName),
                              nl->Second(nl->First(maprest)));
    currentattr = nl->Append(currentattr,oldattr);
    maprest = nl->Rest(maprest);
  }
  currentattr = nl->Append(currentattr,
                           nl->TwoElemList(
                             nl->SymbolAtom("TID"),
                             nl->SymbolAtom(TupleIdentifier::BasicType())));
  
  ListExpr resType = nl->TwoElemList(nl->SymbolAtom(Symbol::STREAM()),
                                     nl->TwoElemList(
                                       nl->SymbolAtom(Tuple::BasicType()),
                                       attrlist));
  
  return nl->ThreeElemList(nl->SymbolAtom(Symbols::APPEND()),
                           nl->TwoElemList(nl->IntAtom(noAttrs),
                                           indices),
                           resType);
}

// TODO support for more than one function
class mupdateInfo {
  public:
    mupdateInfo(Word w, vector<Tuple*>* _relation, 
                bool _flob, Word _arg, 
                Word _arg2, TupleType* _type)
      : stream(w),relation(_relation), flob(_flob), arg(_arg), 
        arg2(_arg2), type(_type) {
      stream.open();
    }

    ~mupdateInfo(){
      stream.close();
    }
    
    Tuple* next() {
      Tuple* res = stream.request();
      if(!res) { return 0; }
      
      // Supplier for the functions
      Supplier supplier = arg.addr;

      Word elem;  
      Supplier son = qp->GetSupplier(arg2.addr, 0);
      qp->Request(son, elem);
      int changedIndex = ((CcInt*)elem.addr)->GetIntval()-1;

      // Get next appended index
      Supplier s1 = qp->GetSupplier(supplier, 0);
      // Get the function
      Supplier s2 = qp->GetSupplier(s1, 1);
      ArgVectorPointer funargs = qp->Argument(s2);
      ((*funargs)[0]).setAddr(res);
      Word value;
      qp->Request(s2,value);
      Attribute* newAttr = ((Attribute*)value.addr)->Clone();
      
      if(flob)
        res->bringToMemory();
      
      Tuple* tup = copyAttributes(res,type);
      res->IncReference();
      
      
      for(int i = 0; i < res->GetNoAttributes(); i++) {
        if(i!=changedIndex)
          tup->CopyAttribute(i,res,i);
        else
          tup->PutAttribute(i,newAttr->Clone());
      }
      
      //get tuple id and append it to tuple
      const TupleId& tid = res->GetTupleId();
      Attribute* tidAttr = new TupleIdentifier(true,tid);
      tup->PutAttribute(tup->GetNoAttributes()-1,tidAttr);
      
      update<MUpdate>(relation,res,changedIndex,newAttr);
      
      return tup;
    }

  private:
     Stream<Tuple> stream;
     vector<Tuple*>* relation; 
     bool flob;
     Word arg;
     Word arg2;
     TupleType* type;
};


/*

7.16.3  Value Mapping Function of operator ~mupdate~

*/
template<class T>
int mupdateValMap (Word* args, Word& result,
                    int message, Word& local, Supplier s) {
  
  mupdateInfo* li = (mupdateInfo*) local.addr;
  
  switch (message) {
    
    case OPEN : {
      
      if(li) {
        delete li;
        local.addr=0;
      }
      
      qp->Open(args[0].addr);
      
      T* oN = (T*) args[1].addr;
      ListExpr ttype = nl->Second(qp->GetType(qp->GetSon(s,0)));
      MemoryRelObject* rel = getMemRel(oN,ttype);
      if(!rel) { return 0; }
        
      TupleType* tt = new TupleType(nl->Second(GetTupleResultType(s)));
      
      local.addr = new mupdateInfo(args[0],rel->getmmrel(),
                                   rel->hasflob(),
                                   args[2],args[4],tt);
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
   mupdateValMap<CcString>,
   mupdateValMap<Mem>
};

int mupdateSelect(ListExpr args){
   return CcString::checkType(nl->Second(args))?0:1;
}



/*
7.16.4 Description of operator ~mupdate~

*/
OperatorSpec mupdateSpec(
    "stream(tuple(x)) x string x [(a1, (tuple(x) -> d1)) "
    "... (an, (tuple(x) -> dn))] -> "
    "stream(tuple(x @ [x1_old t1] @...[xn_old tn] @ [TID tid])))",
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
    mupdateTypeMap
);


/*
7.17 Operator ~mupdatesave~

The Operator ~mupdatesave~ has the same functionality as ~mupdate~, but
saves the tuples of the output stream in an additional auxiliary
main memory relation.

*/
class mupdatesaveInfo {
  public:
    mupdatesaveInfo(Word w, vector<Tuple*>* _relation, 
                    vector<Tuple*>* _auxrel, 
                    bool _flob, Word _arg, 
                    Word _arg2, TupleType* _type)
      : stream(w),relation(_relation), auxrel(_auxrel),
        flob(_flob), arg(_arg), 
        arg2(_arg2), type(_type) {
      stream.open();
    }
    
    ~mupdatesaveInfo(){
      stream.close();
    }

    Tuple* next() {
      
      Tuple* res = stream.request();
      if(!res) { return 0; }
      
      // Copy the attributes from the old tuple
      Tuple* tup = copyAttributes(res,type);

      // Supplier for the functions
      Supplier supplier = arg.addr;
      Word elem, value;  
      Supplier son = qp->GetSupplier(arg2.addr, 0);
      qp->Request(son, elem);
      int changedIndex = ((CcInt*)elem.addr)->GetIntval()-1;

      // Get next appended index
      Supplier s1 = qp->GetSupplier(supplier, 0);
      // Get the function
      Supplier s2 = qp->GetSupplier(s1, 1);
      ArgVectorPointer funargs = qp->Argument(s2);
      ((*funargs)[0]).setAddr(res);
      qp->Request(s2,value);
      Attribute* newAttr = ((Attribute*)value.addr)->Clone();

      if(flob){
        res->bringToMemory();
      }
      res->IncReference();
      
      for(int i = 0; i < res->GetNoAttributes(); i++) {
        if(i!=changedIndex)
          tup->CopyAttribute(i,res,i);
        else
          tup->PutAttribute(i,newAttr->Clone());
      }
      
      //get tuple id and append it to tuple
      const TupleId& tid = res->GetTupleId();
      Attribute* tidAttr = new TupleIdentifier(true,tid);
      tup->PutAttribute(tup->GetNoAttributes()-1,tidAttr);
      
      // append tuple to auxrel
      auxrel->push_back(tup);
//       tup->IncReference();
      
      update<MUpdateSave>(relation,res,changedIndex,newAttr);
      
//       res->DeleteIfAllowed();
      return tup;
    }

  private:
     Stream<Tuple> stream;
     vector<Tuple*>* relation;
     vector<Tuple*>* auxrel;
     bool flob;
     Word arg;
     Word arg2;
     TupleType* type;
};


/*
7.17.2 Value Mapping Function of operator ~mupdatesave~

*/
template<class T>
int mupdatesaveValueMap (Word* args, Word& result,
                    int message, Word& local, Supplier s) {
  
  mupdatesaveInfo* li = (mupdatesaveInfo*) local.addr;
  
  switch (message) {
    
    case OPEN : {
      
      if(li) {
        delete li;
        local.addr=0;
      }
      
      qp->Open(args[0].addr);
      
      T* oN = (T*) args[1].addr;
      T* aux = (T*) args[2].addr;
      
      MemoryRelObject* rel = getMemRel(oN);
      if(!rel) { return 0; }
      MemoryRelObject* auxrel = getMemRel(aux);
      if(!auxrel) { return 0; }
      
      TupleType* tt = new TupleType(nl->Second(GetTupleResultType(s)));
      
      local.addr = new mupdatesaveInfo(args[0],rel->getmmrel(),
                                       auxrel->getmmrel(),
                                       rel->hasflob(),
                                       args[3],args[5],tt);
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

ValueMapping mupdatesaveVM[] = {
   mupdatesaveValueMap<CcString>,
   mupdatesaveValueMap<Mem>
};

int mupdatesaveSelect(ListExpr args){
   return CcString::checkType(nl->Second(args))?0:1;
}



/*
7.17.4 Description of operator ~mupdatesave~

*/
OperatorSpec mupdatesaveSpec(
    "stream(tuple(x)) x {string, mem(rel(tuple(x)))} "
    "x [(a1, (tuple(x) -> d1)) ... (an,(tuple(x) -> dn))] "
    "x {string, mem(rel((tuple(x@[TID:tid]))))} -> "
    "stream(tuple(x @ [x1_old t1] @...[xn_old tn] @ [TID tid])))",
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
    2,
    mupdatesaveVM,
    mupdatesaveSelect,
    mupdateTypeMap
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
  ListExpr first = nl->First(args);
  if(!nl->HasLength(first,2)){
      return listutils::typeError("internal error");
  }
  string errMsg;
  if(!getMemType(nl->First(first), nl->Second(first), first, errMsg)){
    return listutils::typeError("string or mem(rel) expected : " + errMsg);
  }
  first = nl->Second(first); // remove mem
  if(!Relation::checkType(first)){
    return listutils::typeError("first arg is not a memory relation");
  }

  // process second arg (tid)
  ListExpr second = nl->First(nl->Second(args));
  if(!listutils::isSymbol(second,TupleIdentifier::BasicType())){
    return listutils::typeError("second argument must be a tid");
  }
    
  // process update function
  ListExpr map = nl->Third(args);
  
  // argument is not a map
  if(nl->ListLength(map<1)) {
    return listutils::typeError("arg must be a list of maps");
  }
  ListExpr maprest = nl->First(map);
  int noAttrs = nl->ListLength(map);
  
  // Go through all functions
  ListExpr mapfirst, mapsecond;
  ListExpr attrType;
  ListExpr indices, indicescurrent;
  
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
    bool firstcall = true;
    if(firstcall) {
      indices = nl->OneElemList(nl->IntAtom(attrIndex));
      indicescurrent = indices;
      firstcall = false;
    }
    else 
      indicescurrent = nl->Append(indicescurrent,
                                  nl->IntAtom(attrIndex));
  }
  
  maprest = nl->Second(nl->Second(first));
  
  ListExpr attrlist = nl->OneElemList(nl->First(maprest));
  ListExpr currentattr = attrlist;
  maprest = nl->Rest(maprest);
  
  while (!(nl->IsEmpty(maprest))) {
    currentattr = nl->Append(currentattr,nl->First(maprest));
    maprest = nl->Rest(maprest);
  }
  
  // build second part of the resultstream
  maprest = nl->Second(nl->Second(first));
  string oldName;
  ListExpr oldattr;
  while (!(nl->IsEmpty(maprest))) {
    nl->WriteToString(oldName, nl->First(nl->First(maprest)));
    oldName += "_old";
    oldattr = nl->TwoElemList(nl->SymbolAtom(oldName),
                              nl->Second(nl->First(maprest)));
    currentattr = nl->Append(currentattr,oldattr);
    maprest = nl->Rest(maprest);
  }
  currentattr = nl->Append(currentattr,
                           nl->TwoElemList(
                             nl->SymbolAtom("TID"),
                             nl->SymbolAtom(TupleIdentifier::BasicType())));
  
  ListExpr resType = nl->TwoElemList(nl->SymbolAtom(Symbol::STREAM()),
                                     nl->TwoElemList(
                                       nl->SymbolAtom(Tuple::BasicType()),
                                       attrlist));
  
  return nl->ThreeElemList(nl->SymbolAtom(Symbols::APPEND()),
                           nl->TwoElemList(nl->IntAtom(noAttrs),
                                           indices),
                           resType);
}

class mupdatebyidInfo {
  public:
    mupdatebyidInfo(vector<Tuple*>* _relation,
                    TupleIdentifier* _tid, Word _arg,
                    Word _arg2, TupleType* _type)
      : relation(_relation), tid(_tid), arg(_arg), 
        arg2(_arg2), type(_type) {
        firstcall = true;
      }
    
    ~mupdatebyidInfo(){
      type->DeleteIfAllowed();
    }
    
    Tuple* next() {
      if(!firstcall) return 0;
      firstcall = false;
      Tuple* res = find(relation,tid);
      if(!res) return 0;
      
      Tuple* newtup = copyAttributes(res,type);
        
      // Supplier for the functions
      Supplier supplier = arg.addr;

      Word elem, value;  
      Supplier son = qp->GetSupplier(arg2.addr, 0);
      qp->Request(son, elem);
      int changedIndex = ((CcInt*)elem.addr)->GetIntval()-1;

      // Get next appended index
      Supplier s1 = qp->GetSupplier(supplier, 0);
      // Get the function
      Supplier s2 = qp->GetSupplier(s1, 1);
      ArgVectorPointer funargs = qp->Argument(s2);
      ((*funargs)[0]).setAddr(res);
      qp->Request(s2,value);
      Attribute* newAttr = ((Attribute*)value.addr)->Clone();
      
      update<MUpdateByID>(relation,res,changedIndex,newAttr); 
      res->SetTupleId(tid->GetTid());
      for(int i = 0; i < res->GetNoAttributes(); i++) {
        if(i!=changedIndex)
          newtup->CopyAttribute(i,res,i);
        else
          newtup->PutAttribute(i,newAttr->Clone());
      }
      
      //get tuple id and append it to tuple
      Attribute* tidAttr = new TupleIdentifier(true,tid->GetTid());
      newtup->PutAttribute(newtup->GetNoAttributes()-1,tidAttr);
 
      return newtup;
    }

  private:
     vector<Tuple*>* relation;
     TupleIdentifier* tid;
     Word arg;
     Word arg2;
     TupleType* type;
     bool firstcall;
};


/*

7.18.3  Value Mapping Function of operator ~mupdatebyid~

*/
template<class T>
int mupdatebyidValueMap (Word* args, Word& result,
                    int message, Word& local, Supplier s) {
  
  mupdatebyidInfo* li = (mupdatebyidInfo*) local.addr;
  
  switch (message) {
    
    case OPEN : {
      
      if(li){
        delete li;
        local.addr=0;
      }
      
      T* oN = (T*) args[0].addr;
      MemoryRelObject* rel = getMemRel(oN);
      if(!rel) {return 0;}
      
      TupleIdentifier* tid = (TupleIdentifier*)(args[1].addr);
      TupleType* tt = new TupleType(nl->Second(GetTupleResultType(s)));
      
      local.addr = new mupdatebyidInfo(rel->getmmrel(),tid,args[2],args[4],tt);
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
   mupdatebyidValueMap<CcString>,
   mupdatebyidValueMap<Mem>
};

int mupdatebyidSelect(ListExpr args){
   return CcString::checkType(nl->Second(args))?0:1;
}

/*
7.18.4 Description of operator ~mupdatebyid~

*/
OperatorSpec mupdatebyidSpec(
    "{string, mem(rel(tuple))} x (tid) x "
    "[(a1, (tuple(x) -> d1)) ... (an,(tuple(x) -> dn))] "
    "-> stream(tuple(x @ [x1_old t1] @...[xn_old tn] @ [TID tid])))",
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
         
    moinsertInfo(Word w, ttree::TTree<Tuple*,TupleComp>* _orel, 
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
      
      if((ct==MOInsert) && flob)
        res->bringToMemory();
      
      // copy attributes of tuple from input stream
      Tuple* newtup = new Tuple(type); 
      for(int i = 0; i < res->GetNoAttributes(); i++)
        newtup->CopyAttribute(i,res,i);
      
      if(ct == MOInsert)
        orel->insert(res,attrPos);  
      if(ct == MODelete)
        orel->remove(res,attrPos);  
      res->IncReference();
      
      //get tuple id and append it to tuple
      const TupleId& tid = res->GetTupleId();
      Attribute* tidAttr = new TupleIdentifier(true,tid);
      newtup->PutAttribute(res->GetNoAttributes(), tidAttr);
      
      res->DeleteIfAllowed();
      return newtup;
    }

  private:
     Stream<Tuple> stream;
     ttree::TTree<Tuple*,TupleComp>* orel;
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
   moinsertValueMap<CcString,MOInsert>,
   moinsertValueMap<Mem,MOInsert>,
   moinsertValueMap<CcString,MODelete>,
   moinsertValueMap<Mem,MODelete>
};


template<ChangeType ct>
int moinsertSelect(ListExpr args){
  if(ct == MOInsert) 
    return CcString::checkType(nl->Second(args))?0:1;
  if(ct == MODelete)
    return CcString::checkType(nl->Second(args))?2:3;
}



/*
7.19.4 Description of operator ~moinsert~

*/
OperatorSpec moinsertSpec(
    "stream(tuple(x)) x {string, mem(orel(tuple(x)))} -> "
    "stream(tuple(x@[TID:tid]))",
    "_ moinsert [_]",
    "inserts the tuple of a stream into an "
    "existing main memory ordered relation",
    "query moinsert (ten feed head[5],\"oten\") count"
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
    minsertTypeMap
);


/*
7.20 Operator ~modelete~

The operator ~modelete~ deletes all tuple of the input stream from the given
main memory ordered relation. The tuples of the input stream including an 
attribute of type 'tid' are appended to the output stream.

7.20.4 Description of operator ~modelete~

*/
OperatorSpec modeleteSpec(
    "stream(tuple(x)) x {string, mem(orel(tuple(x)))} "
    "-> stream(tuple(x@[TID:tid]))",
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
    minsertTypeMap
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
    
    ListExpr attrlist = nl->Second(nl->Second(nl->First(args)));
    if(!listutils::checkAttrListForNamingConventions(attrlist)){
      return listutils::typeError("Some of the attributes do "
                   "not fit into Secondo's naming conventions");
    }
    if(!listutils::isKeyDescription(nl->Second(nl->First(args)),
                                    nl->Second(args))) {
      return listutils::typeError("all identifiers of second argument must "
                                  "appear in the first argument");
    }
    ListExpr l1 = nl->Second(nl->First(args));
    ListExpr l2 = nl->SymbolAtom(MemoryORelObject::BasicType());
    return nl->TwoElemList (l2,l1);
}


/*
7.21.2  Value Mapping Function of operator ~moconsume~

*/
int moconsumeValueMap (Word* args, Word& result,
                int message, Word& local, Supplier s) {
    
    Supplier t = qp->GetSon(s,0);
    ListExpr rel = qp->GetType(t);

    t = qp->GetSon(s,1);
    ListExpr attrToSort = qp->GetType(t);
    ListExpr type = attrToSort;
    
    result = qp->ResultStorage(s); 
    MemoryORelObject* morel = (MemoryORelObject*)result.addr;
    
    ListExpr attr;
    while (!(nl->IsEmpty(attrToSort))) {
      attr = nl->First(attrToSort);
      attrToSort = nl->Rest(attrToSort);
      
      ListExpr attrType = 0;
      int attrPos = 0;
      ListExpr attrList = nl->Second(nl->Second(rel));
      
      attrPos = listutils::findAttribute(attrList, 
                                         nl->ToString(attr), 
                                         attrType);
      if(attrPos == 0) {
        return listutils::typeError
          ("there is no attribute having name " + nl->ToString(attr));
      }
      morel->setAttrPos(attrPos,true);
    }
    
    Stream<Tuple> stream(args[0]);
    stream.open();
    Tuple* tup = 0;
    while((tup = stream.request()) != 0) {
      morel->addTuple(tup);
    }
    
    morel->setObjectTypeExpr(nl->ToString(nl->TwoElemList(nl->Second(rel),
                                                          type)));
    stream.close();
    return 0;
}

/*
7.21.4 Description of operator ~moconsume~

*/
OperatorSpec moconsumeSpec(
    "stream(Tuple) x (ident1 ... identn) "
    "-> (mem(orel (tuple(x)) (ident1 ... identn)))",
    "_ moconsume[list]",
    "collects the objects from a stream(tuple) into a memory ordered relation",
    "query 'ten' mfeed head[2] moconsume[No]"
);


/*
7.21.5 Instance of operator ~moconsume~

*/
Operator moconsumeOp (
    "moconsume",
    moconsumeSpec.getStr(),
    moconsumeValueMap,
    Operator::SimpleSelect,
    moconsumeTypeMap
);


/*
7.22 Operator ~letmoconsume~ and ~letmoconsumeflob~

  The Operator ~letmoconsume~ creates a new MemoryORelOBject from an 
  input stream. It's name is given in the second argument. The third
  argument provides a list of attribute identifiers over which the relation
  will be sorted. The Operator ~letmoconsumeflob~ additionally loads the 
  flobs of all tuple of the stream into main memory.
  
7.21.1 General Type Mapping Functions of operator ~letmoconsume~
       and ~letmoconsumeflob~

  stream(tuple(x)) x string x (ident1 ... identn) -> 
  mem(orel(tuple(x)) (ident1 ... identn)) 

*/
ListExpr letmoconsumeTypeMap(ListExpr args) {
    if(nl->ListLength(args)!=3){
      return listutils::typeError("(wrong number of arguments)");
    }
    if (!Stream<Tuple>::checkType(nl->First(args)) || 
        !CcString::checkType(nl->Second(args)) ) {
      return listutils::typeError ("stream(Tuple) x string expected!");
    }
    
    ListExpr attrlist = nl->Second(nl->Second(nl->First(args)));
    if(!listutils::checkAttrListForNamingConventions(attrlist)){
      return listutils::typeError("Some of the attributes do "
                   "not fit into Secondo's naming conventions");
    }

    if(!listutils::isKeyDescription(nl->Second(nl->First(args)),
                                    nl->Third(args))) {
      return listutils::typeError("all identifiers of third argument must "
                                  "appear in the first argument");
    }
    
    return nl->TwoElemList(
                  listutils::basicSymbol<Mem>(),
                  nl->ThreeElemList(
                      nl->SymbolAtom(OREL),
                      nl->Second(nl->First(args)),
                      nl->Third(args)));
}

/*
7.22.2 Value Mapping Functions of operator ~letmoconsume~
       and ~letmoconsumeflob~

*/
template<bool flob>
int letmoconsumeValMap(Word* args, Word& result,
                int message, Word& local, Supplier s) {
  
  result  = qp->ResultStorage(s);
  Mem* str = (Mem*)result.addr;

  CcString* oN = (CcString*) args[1].addr;
  if(!oN->IsDefined()){
      str->SetDefined(false);
      return 0;
  }
  
  string res = oN->GetValue();
  if(catalog->isMMObject(res)) {
      str->SetDefined(false);
      return 0;
  }
  
  ListExpr rel = qp->GetType(qp->GetSon(s,0));
  ListExpr attrToSort = qp->GetType(qp->GetSon(s,2));
  MemoryORelObject* mmorel = new MemoryORelObject();
  
  ListExpr attrs = attrToSort;
  ListExpr attr;
 
  while (!(nl->IsEmpty(attrToSort))) {
    attr = nl->First(attrToSort);
    attrToSort = nl->Rest(attrToSort);
    
    ListExpr attrType = 0;
    int attrPos = 0;
    ListExpr attrList = nl->Second(nl->Second(rel));
    
    attrPos = listutils::findAttribute(attrList, nl->ToString(attr), attrType);
    if(attrPos == 0) {
      return listutils::typeError
        ("there is no attribute having name " + nl->ToString(attr));
    }
    mmorel->setAttrPos(attrPos,true);
  }
  
  bool succeed = mmorel->tupleStreamToORel(args[0],
                                           rel, attrs,
                                           getDBname(), flob);
  if(succeed) 
    catalog->insert(res,mmorel);
  else 
    delete mmorel;
  
  str->set(succeed,res);
  return 0;
}


/*
7.22.4 Description of operator ~letmoconsume~

*/
OperatorSpec letmoconsumeSpec(
    "stream(tuple(x)) x string x (ident1 ... identn) -> "
    "mem(orel(tuple(x)) (ident1 ... identn)) ",
    "_ letmoconsume [_;list]",
    "produces a main memory ordered relation from a stream(tuple)",
    "query ten feed head[5] minsert['ten'] letmoconsume['fuenf'; No]"
);

/*
7.22.5 Instance of operator ~letmoconsume~

*/
Operator letmoconsumeOp (
    "letmoconsume",
    letmoconsumeSpec.getStr(),
    letmoconsumeValMap<false>,
    Operator::SimpleSelect,
    letmoconsumeTypeMap
);


/*
7.23.4 Description of operator ~letmoconsumeflob~

*/
OperatorSpec letmoconsumeflobSpec(
    "stream(tuple(x)) x string x (ident1 ... identn) "
    "-> mem(orel(tuple(x)) (ident1 ... identn))",
    "_ letmoconsumeflob [_; list]",
    "produces a main memory ordered relation from a stream(tuple)"
    "and loads the associated flobs",
    "query 'oTrains' mfeed letmoconsumeflob ['moTrains'; Id]"
);


/*
7.23.5 Instance of operator ~letmoconsumeflob~

*/
Operator letmoconsumeflobOp (
    "letmoconsumeflob",
    letmoconsumeflobSpec.getStr(),
    letmoconsumeValMap<true>,
    Operator::SimpleSelect,
    letmoconsumeTypeMap
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
    ListExpr a3;        
    if(rk==Range)
      a3 = nl->Third(args);

    string err;
    if(rk==Range)
      err = "{string, mem(orel)} x T x T expected";
    //if moleftrange or morightrange
    else 
      err = "{string, mem(orel)} x T expected"; 
  
    string errMsg;
    if(!getMemType(nl->First(a1), nl->Second(a1), a1, errMsg)){
      return listutils::typeError(err + "\n problem in first arg:" + errMsg);
    }
    // remove mem from mem(orel) 
    a1 = nl->Second(a1); 
   
    // process MemoryORelObject
    if(!listutils::isOrelDescription(a1)) {
      return listutils::typeError(err + " (first arg is not an orel)");
    }
    // extract type of key1
    a2 = nl->First(a2);  
    // extract type of key2 if morange
    if(rk==Range)
      a3 = nl->First(a3);  
      
    ListExpr key = nl->First(nl->Third(a1));
    
    string attrName = nl->SymbolValue(key);
    ListExpr attrType = 0;
    int attrPos = 0;
    ListExpr attrList = nl->Second(nl->Second(a1));
    attrPos = listutils::findAttribute(attrList, attrName, attrType);

    if(attrPos == 0){
        return listutils::typeError
        ("there is no attribute having name " + attrName);
    }

    if(!nl->Equal(attrType, a2)){
      return listutils::typeError("oreltype and type of arg 2 differ");
    }
    if(rk==Range) {
      if(!nl->Equal(attrType, a3)){
        return listutils::typeError("oreltype and type of arg 3 differ");
      }
    }
    return nl->TwoElemList(nl->SymbolAtom(Symbol::STREAM()),
                           nl->Second(a1)); 
}

template<RangeKind rk>
class morangeInfo{
    public:
      
      morangeInfo(ttree::TTree<Tuple*,TupleComp>* _tree, 
                  Attribute* _attr1,
                  Attribute* _attr2, 
                  int _attrPos)
          : tree(_tree),attr1(_attr1),attr2(_attr2),attrPos(_attrPos) {
        
        iter = tree->begin();
        res = false;
        
        if(rk==LeftRange) 
          res = true;
        
        while((rk== Range || rk==RightRange) && 
               !res && !iter.end()) {
          Tuple* result = *iter;
          Attribute* attr;
          if(result)
            attr = result->GetAttribute(attrPos-1);
          
          if(attr->Compare(attr1) < 0) 
            iter++;
          else
            res = true;
        }
      }
      
      ~morangeInfo(){}


      Tuple* next() {

        if(iter.end() || !res)
          return 0;
        
        Tuple* result = *iter;
        if(!result)
          return 0;
        
        Attribute* attr = result->GetAttribute(attrPos-1);
        
        if(rk==LeftRange && attr->Compare(attr1) > 0) {
          return 0;
        }
        if(rk==Range && attr->Compare(attr2) > 0) {
          return 0;
        }
          
        result->IncReference();
        iter++;
        return result;
      }


    private:
        ttree::TTree<Tuple*,TupleComp>* tree;
        Attribute* attr1;
        Attribute* attr2;
        int attrPos;
        ttree::Iterator<Tuple*,TupleComp> iter;
        bool res;
};

/*
7.24.2 The Value Mapping Function of operator ~mrange~, ~moleftrange~ and 
       ~morightrange~

*/
template<class T, RangeKind rk>
int morangeVMT (Word* args, Word& result,
                    int message, Word& local, Supplier s) {

   
    morangeInfo<rk>* li = (morangeInfo<rk>*) local.addr;
      
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
            
            Attribute* key1 = (Attribute*) args[1].addr;
            // if morange
            Attribute* key2 = 0;        
            if(rk==Range)
              key2 = (Attribute*) args[2].addr;
            
            local.addr = new morangeInfo<rk>(orel->getmmorel(),
                                             key1,key2, 
                                             orel->getAttrPos()->at(0)); 
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
    morangeVMT<CcString,Range>,
    morangeVMT<Mem,Range>,
    morangeVMT<CcString,LeftRange>,
    morangeVMT<Mem,LeftRange>,
    morangeVMT<CcString,RightRange>,
    morangeVMT<Mem,RightRange>
};

template<RangeKind rk>
int morangeSelect(ListExpr args){
  if(rk==Range) {
    return CcString::checkType(nl->First(args))?0:1;
  }
  else if(rk==LeftRange) {
    return CcString::checkType(nl->First(args))?2:3;
  }
  else if(rk==RightRange)
    return CcString::checkType(nl->First(args))?4:5;
}

/*
7.24.4 Description of operator ~morange~

*/
OperatorSpec morangeSpec(
    "{string, mem(orel(tuple(x)))}  x T x T -> stream(tuple(x))",
    "_ morange [_,_]",
    "returns all tuples in a main memory ordered relation whose attributes "
    "are lying between the two given values",
    "query 'oten' morange[2,2] count"
);


/*
7.25.4 Description of operator ~moleftrange~

*/
OperatorSpec moleftrangeSpec(
    "{string, mem(orel(tuple(x)))}  x T -> stream(tuple(x))",
    "_ moleftrange [_]",
    "returns all tuples in a main memory ordered relation whose attributes "
    "are smaller than the given value",
    "query 'oten' moleftrange[2] count"
);

/*
7.26.4 Description of operator ~morightrange~

*/
OperatorSpec morightrangeSpec(
    "{string, mem(orel(tuple(x)))}  x T -> stream(tuple(x))",
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

Tuple* findTuple(ttree::TTree<Tuple*,TupleComp>* mmorel, 
                 int nr, int prev) {
    
  CcInt* startNodeInt = new CcInt(true,prev);
  CcInt* endNodeInt = new CcInt(true,nr);

  ttree::Iterator<Tuple*,TupleComp> it = mmorel->begin();
  
  Tuple* tup;
  while(it.hasNext()) {
    tup = *it;
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
                 ttree::TTree<Tuple*,TupleComp>* result) {
  Tuple* newTuple = new Tuple(tt);
  int i = 0;
  for(; i<tup->GetNoAttributes(); i++) {
    newTuple->CopyAttribute(i,tup,i);
  }
  int s = seqNo;
  CcInt* noOfNodes = new CcInt(true, s);
  newTuple->PutAttribute(i,noOfNodes);
  result->insert(newTuple,i+1);   // sort by SeqNo
  newTuple->IncReference();
  return true;
}

QueueEntry* findNextNode(ttree::TTree<QueueEntry*,EntryComp>* visitedNodes,
                         QueueEntry* current) {
    
  ttree::Iterator<QueueEntry*,EntryComp> iter = visitedNodes->begin();
  while(iter.hasNext()) {
    QueueEntry* entry = *iter;

    if(entry->nodeNumber == current->prev) {
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
  ListExpr a1 = nl->First(args);
  if(!nl->HasLength(a1,2)){
      return listutils::typeError("internal error");
  }
  string errMsg;
  if(!getMemType(nl->First(a1), nl->Second(a1), a1, errMsg)){
    return listutils::typeError("string or mem(orel) expected : " + errMsg);
  }
  
  a1 = nl->Second(a1); // remove mem from mem(orel) 
  //MemoryORelObject
  if(!listutils::isOrelDescription(a1)) {
    return listutils::typeError("first arg is not an orel");
  }
  
  ListExpr startNodeList = nl->First(nl->Second(args));
  ListExpr endNodeList = nl->First(nl->Third(args));
  ListExpr resultSelect = nl->First(nl->Fourth(args));
  ListExpr functionWeightMap = nl->First(nl->Fifth(args));
  ListExpr functionDistMap;
  if(sptype == ASTAR)
    functionDistMap = nl->First(nl->Sixth(args));

  ListExpr orelTuple = nl->Second(a1);    

  if(!listutils::isTupleDescription(orelTuple)) {
    return listutils::typeError("second value of orel is not of type tuple");
  }

  ListExpr orelAttrList(nl->Second(orelTuple));

  if(!listutils::isAttrList(orelAttrList)) {
    return listutils::typeError("Error in orel attrlist.");
  }

  if(nl->ListLength(orelAttrList) >= 3) {
    ListExpr firstAttr = nl->First(orelAttrList);

    if(nl->ListLength(firstAttr) != 2 ||
       nl->SymbolValue(nl->Second(firstAttr)) != CcInt::BasicType()) {
      return listutils::typeError("First attribute of orel should be int");
    }

    ListExpr secondAttr = nl->Second(orelAttrList);
    if (nl->ListLength(secondAttr) != 2 ||
        nl->SymbolValue(nl->Second(secondAttr)) != CcInt::BasicType()) {
      return listutils::typeError("Second attribute of orel should be int");
    }
  }
  else {
    return listutils::typeError("orel has less than 3 attributes.");
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
  if(!listutils::isMap<1>(functionWeightMap)) {
    return listutils::typeError("Fifth argument should be a map");
  }

  ListExpr mapTuple = nl->Second(functionWeightMap);

  if(!nl->Equal(orelTuple,mapTuple)) {
    return listutils::typeError("Tuple of map function must match orel tuple");
  }

  ListExpr mapres = nl->Third(functionWeightMap);

  if(!listutils::isSymbol(mapres,CcReal::BasicType())) {
    return listutils::typeError(
                "Wrong mapping result type for oshortestpathd");
  }
  
  // check of sixth argument if ASTAR
  if(sptype == ASTAR) {
    if (!listutils::isMap<1>(functionDistMap)) {
      return listutils::typeError(
                      "Sixth argument should be a map");
    }
    ListExpr mapTuple2 = nl->Second(functionDistMap);

    if (!nl->Equal(orelTuple,mapTuple2)) {
      return listutils::typeError(
                       "Tuple of map function must match orel tuple");
    }
    ListExpr mapres2 = nl->Third(functionDistMap);

    if(!listutils::isSymbol(mapres2,CcReal::BasicType())) {
      return listutils::typeError(
                            "Wrong mapping result type for oshortestpath");
    }
  }

  // appends Attribute SeqNo to Attributes in orel
  NList extendAttrList(nl->TwoElemList(nl->SymbolAtom("SeqNo"),
                                       nl->SymbolAtom(CcInt::BasicType())));
  NList extOrelAttrList(nl->TheEmptyList());


  for (int i = 0; i < nl->ListLength(orelAttrList); i++) {
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
  
  moshortestpathdInfo(ttree::TTree<Tuple*,TupleComp>* _tree,  
                     int _startNode, int _endNode, int _resultSelect,
                     Word _arg, TupleType* _tt) 
      : mmorel(_tree),startNode(_startNode),
        endNode(_endNode),resultSelect(_resultSelect),
        arg(_arg),tt(_tt),seqNo(1) {
          
    queue = new Queue();
    visitedNodes = new ttree::TTree<QueueEntry*,EntryComp>(16,18);
    sptree = new ttree::TTree<Tuple*,TupleComp>(16,18);
    result = new ttree::TTree<Tuple*,TupleComp>(16,18);
    
    bool found = shortestPath();
    if(found || resultSelect == 3) {
      resultSelection();
    }
    else  
      cerr << "no path found" << endl;
    iterator = result->begin();
  }

  ~moshortestpathdInfo() {
    while(!queue->empty())
      queue->pop();
    delete queue;
    
    ttree::Iterator<QueueEntry*,EntryComp> it = visitedNodes->begin();
    while(it.hasNext()) {
      QueueEntry* entry = *it;
      if(entry) {
        delete entry;
        entry = 0;
      }
      it++;
    }
    
    tt->DeleteIfAllowed();
    tt = 0;
    
    delete visitedNodes;
    delete result;
    delete sptree;
  };
  
  Tuple* next() {
    if(iterator.end()) 
      return 0;
    Tuple* res = *iterator;
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
    queue->push(startEntry);
    visitedNodes->insert(startEntry);
    
    double dist = 0.0;
    
  // SEARCH SHORTESTPATH
    while(!queue->empty()) {
      current = queue->top();
      current->visited = true;
      queue->pop();
      
      // goal node found
      if (resultSelect<3 && current->nodeNumber == endNode) {
        return true;
      }
      // process next node
      else {
        CcInt* currentNodeNumber = new CcInt(true,current->nodeNumber);
        // get tuple with currentNodeNumber as startnode
        ttree::Iterator<Tuple*,TupleComp> it = mmorel->begin();
        while(it.hasNext()) {
          Tuple* tup = *it;
          if(tup->GetAttribute(0)->Compare(currentNodeNumber) < 0) {
            it++;
          }
          // tuple found
          else break;
        }
        // process edges
        while(it.hasNext()) {
          
          Tuple* currentTuple = *it;
          // all edges processed
          if(currentTuple->GetAttribute(0)->Compare(currentNodeNumber) > 0) {
            break;
          }
          if(resultSelect!=3) {
            sptree->insert(currentTuple);   
          }
          toNode = ((CcInt*)currentTuple->GetAttribute(1))->GetIntval();
          
          if(current->nodeNumber != toNode) {
            ArgVectorPointer funArgs = qp->Argument(arg.addr);
            Word funResult;
            ((*funArgs)[0]).setAddr(currentTuple);
            qp->Request(arg.addr,funResult);
            double edgeCost = ((CcReal*)funResult.addr)->GetRealval();
            if (edgeCost < 0.0) {
              cerr << "Found negativ edge cost computation aborted." << endl;
              return 0;
            }
            dist = current->dist + edgeCost;
            bool contained = false;
            ttree::Iterator<QueueEntry*,EntryComp> it = 
                                            visitedNodes->begin();
            
            // check if shortening of path possible
            while(it.hasNext()) {
              QueueEntry* entry = *it;
              if(entry->nodeNumber == toNode) {
                if(entry->dist > dist) {
                  QueueEntry* prevEntry = new QueueEntry(entry->nodeNumber,
                                                          entry->prev,
                                                          entry->dist,
                                                          entry->priority);
                  prevEntry->visited = entry->visited;
                  entry->dist = dist;
                  entry->prev = current->nodeNumber;
                  entry->priority = dist;
                  if(entry->visited)
                    visitedNodes->update(entry,prevEntry);
                }
                contained = true;
                break;
              }
              it++;
            }

            if(!contained) {
              QueueEntry* to = 
                  new QueueEntry(toNode,current->nodeNumber,dist,dist);
              visitedNodes->insert(to,true);
              queue->push(to);
              if(resultSelect==3)
                sptree->insert(currentTuple);
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
                   findTuple(mmorel,current->nodeNumber,current->prev);
        bool found = false;
    
        while (!found && currentTuple != 0) {

          appendTuple(currentTuple,tt,seqNo,result);
          seqNo++;
          
          if(current->prev != startNode) {
            current = findNextNode(visitedNodes,current);

            currentTuple = 
                    findTuple(mmorel,current->nodeNumber,current->prev);
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
                                          current->nodeNumber,
                                          current->prev);
          appendTuple(currentTuple,tt,seqNo,result);
          seqNo++;
        }
        break;
      }
      case 2: { //visited sections
        ttree::Iterator<Tuple*,TupleComp> iter = sptree->begin();
         while(iter.hasNext()) {
           Tuple* currentTuple = *iter;
           appendTuple(currentTuple,tt,seqNo,result);
           iter++;
           seqNo++;
         }
         break;
      }
      case 3: { //shortest path tree     
        ttree::Iterator<Tuple*,TupleComp> iter = sptree->begin();
        while(iter.hasNext()) {
          Tuple* currentTuple = *iter;
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

  ttree::TTree<Tuple*,TupleComp>* mmorel;     
  ttree::Iterator<Tuple*,TupleComp> iterator;
  int startNode;
  int endNode;
  int resultSelect;
  Word arg; 
  TupleType* tt;
  int seqNo;
  Queue* queue;
  
  ttree::TTree<QueueEntry*,EntryComp>* visitedNodes;
  ttree::TTree<Tuple*,TupleComp>* sptree;
  ttree::TTree<Tuple*,TupleComp>* result;
  
  
  QueueEntry* current;
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
    moshortestpathdValueMap<CcString>,
    moshortestpathdValueMap<Mem>
};


int moshortestpathSelect(ListExpr args){
  return CcString::checkType(nl->First(args))?0:1;
}


/*
7.27.4 Specification of operator ~moshortestpathd~

*/
OperatorSpec moshortestpathdSpec(
    "{string, mem(orel(tuple(x)))} x int x int x int x "
    "(tuple->real) -> stream(tuple(a1:t1,...an+1:tn+1))",
    "_ moshortestpathd [_,_,_; fun] implicit parameter tuple type MTUPLE",
    "claculates the shortest path for a given start and goal node in a main "
    "memory ordered relation using dijkstras algorithm",
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
  
  moshortestpathaInfo(ttree::TTree<Tuple*,TupleComp>* _tree,  
                     int _startNode, int _endNode, int _resultSelect,
                     Word _arg, Word _arg2, TupleType* _tt) 
      : mmorel(_tree),startNode(_startNode),
        endNode(_endNode),resultSelect(_resultSelect),
        arg(_arg),arg2(_arg2),tt(_tt),seqNo(1) {
          
          queue = new Queue();
          visitedNodes = new ttree::TTree<QueueEntry*,EntryComp>(16,18);
          sptree = new ttree::TTree<Tuple*,TupleComp>(16,18);
          result = new ttree::TTree<Tuple*,TupleComp>(16,18);
          
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
    
    ttree::Iterator<QueueEntry*,EntryComp> it = visitedNodes->begin();
    while(it.hasNext()) {
      QueueEntry* entry = *it;
      if(entry) {
        delete entry;
        entry = 0;
      }
      it++;
    }
    
    tt->DeleteIfAllowed();
    tt = 0;
    
    delete visitedNodes;
    delete result;
    delete sptree;
  };
  
  Tuple* next() {
    if(iterator.end()) 
      return 0;
    Tuple* res = *iterator;
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
    queue->push(startEntry);
    visitedNodes->insert(startEntry);
    
    double dist = 0.0;
    
  // SEARCH SHORTESTPATH
    while(!queue->empty()) {
      current = queue->top();
      current->visited = true;
      queue->pop();
      
      if (resultSelect<3 && current->nodeNumber == endNode) {
        return true;
      }
      else {
        CcInt* currentNodeNumber = new CcInt(true,current->nodeNumber);
        
        ttree::Iterator<Tuple*,TupleComp> it = mmorel->begin();
        while(it.hasNext()) {
          Tuple* tup = *it;
          // get tuple with currentNodeNumber as startnode
          if(tup->GetAttribute(0)->Compare(currentNodeNumber) < 0) {
            it++;
          }
          else break;
        }
        
        while(it.hasNext()) {
          Tuple* currentTuple = *it;            
          if(currentTuple->GetAttribute(0)->Compare(currentNodeNumber) > 0) {
            break;
          }
          toNode = ((CcInt*)currentTuple->GetAttribute(1))->GetIntval();
          if(resultSelect!=3) {
            sptree->insert(currentTuple);      
          }
          if(current->nodeNumber != toNode) {
            ArgVectorPointer funArgs = qp->Argument(arg.addr);
            Word funResult;
            ((*funArgs)[0]).setAddr(currentTuple);
            qp->Request(arg.addr,funResult);
            double edgeCost = ((CcReal*)funResult.addr)->GetRealval();
            if (edgeCost < 0.0) {
              cerr << "Found negativ edge cost computation aborted." << endl;
              return 0;
            }
            dist = current->dist + edgeCost;
            
            ArgVectorPointer funArgs2 = qp->Argument(arg2.addr);
            Word funResult2;
            ((*funArgs2)[0]).setAddr(currentTuple);
            qp->Request(arg2.addr,funResult2);
            double restCost = ((CcReal*)funResult2.addr)->GetRealval();
            if (restCost < 0) restCost = 0;
            double prio = dist + restCost;
            
            bool contained = false;
            ttree::Iterator<QueueEntry*,EntryComp> it = 
                                                      visitedNodes->begin();
            while(it.hasNext()) {
              QueueEntry* entry = *it;
              // found node before
              if(entry->nodeNumber == toNode) {
                if(entry->priority > prio) {
                  QueueEntry* prevEntry = new QueueEntry(entry->nodeNumber,
                                                          entry->prev,
                                                          entry->dist,
                                                          entry->priority);
                  prevEntry->visited = entry->visited;
                  entry->prev = current->nodeNumber;
                  entry->priority = prio;
                  entry->dist = dist;
                  if(entry->visited)
                    visitedNodes->update(entry,prevEntry);
                }
                contained = true;
                break;
              }
              it++;
            }

            if(!contained) {
              QueueEntry* to = 
                  new QueueEntry(toNode,current->nodeNumber,dist,prio);
              visitedNodes->insert(to,true);
              queue->push(to);
              if(resultSelect==3)
                sptree->insert(currentTuple);
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
                                        current->nodeNumber,
                                        current->prev);
        bool found = false;
    
        int i=0;
        while (!found && currentTuple != 0) {
          
          i++;
          appendTuple(currentTuple,tt,seqNo,result);
          seqNo++;
          
          if(current->prev != startNode) {
            current = findNextNode(visitedNodes,current);
            currentTuple = findTuple(mmorel,
                                     current->nodeNumber,
                                     current->prev);
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
                                          current->nodeNumber,
                                          current->prev);
          appendTuple(currentTuple,tt,seqNo,result);
          seqNo++;
        }
        break;
      }
      case 2: { //visited sections
        ttree::Iterator<Tuple*,TupleComp> it = sptree->begin();
        while(it.hasNext()) {
          Tuple* currentTuple = *it;
          appendTuple(currentTuple,tt,seqNo,result);
          seqNo++;
          it++;
        }
        break;
      }
      case 3: { //shortest path tree     
        ttree::Iterator<Tuple*,TupleComp> iter = sptree->begin();
        while(iter.hasNext()) {
          Tuple* currentTuple = *iter;
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

  ttree::TTree<Tuple*,TupleComp>* mmorel;     
  ttree::Iterator<Tuple*,TupleComp> iterator;
  int startNode;
  int endNode;
  int resultSelect;
  Word arg; 
  Word arg2;
  TupleType* tt;
  int seqNo;
  Queue* queue;
  
  ttree::TTree<QueueEntry*,EntryComp>* visitedNodes;
  ttree::TTree<Tuple*,TupleComp>* sptree;
  ttree::TTree<Tuple*,TupleComp>* result;


  QueueEntry* current;
  int seqNoAttrIndex;
  
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
    moshortestpathaValueMap<CcString>,
    moshortestpathaValueMap<Mem>
};

/*

7.28.4 Description of operator ~moshortestpatha~

*/
OperatorSpec moshortestpathaSpec(
    "{string, mem(orel(tuple(x)))} x int x int x int x "
    "(tuple->real) x (tuple->real)-> stream(tuple(a1:t1,...an+1:tn+1))",
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

    ListExpr arg = nl->First(args);

    if(!nl->HasLength(arg,2)){
        return listutils::typeError("internal error");
    }

    string errMsg;

    if(!getMemType(nl->First(arg), nl->Second(arg), arg, errMsg)){
      return listutils::typeError("string or mem(rel) expected : " + errMsg);
    }

    arg = nl->Second(arg); // remove mem
    if(!listutils::isOrelDescription(arg)){
      return listutils::typeError("memory object is not a relation");
    }

    // TODO check if second argument attribute of relation

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
  bool operator()(const Vertex* v1, const Vertex* v2) const {
    return v1->nr < v2->nr;
  }
};

// geaendert 9.9.2016
class moconnectedComponentsInfo{
  public:
    moconnectedComponentsInfo(ttree::TTree<Tuple*,TupleComp>* _tree,
                              TupleType* _tt)
        : tree(_tree),tt(_tt) {

      scctree = new ttree::TTree<Tuple*,TupleComp>(16,18);
      nodes = new set<Vertex*,VertexComp>();
      scc(tree);
      appendTuples();
      iter = scctree->begin();
    }


    ~moconnectedComponentsInfo(){
      delete scctree;
      set<Vertex*,VertexComp>::iterator it = nodes->begin();
      while(it!=nodes->end()) {
        Vertex* v = *it;
        delete v;
        it++;
      }
      delete nodes;
    }


    Tuple* next() {
      if(!iter.hasNext()) 
        return 0;
      Tuple* res = *iter;
      if(!res) {
        return 0;
      }
      iter++;
      res->IncReference();
      return res;
    }

    // appends all tuples including their component number to scctree
    bool appendTuples() {
      set<Vertex*,VertexComp>::iterator iter;
      ttree::Iterator<Tuple*,TupleComp> it = tree->begin();
      while(it.hasNext()) {
        Tuple* t = *it;
        Vertex* v = new Vertex(((CcInt*)t->GetAttribute(1))->GetIntval());
        iter = nodes->find(v);
        if(iter != nodes->end()) {
          delete v;
          v = *iter;
          Tuple* newTuple = new Tuple(tt);
          int i = 0;
          for(; i<t->GetNoAttributes(); i++) {
            newTuple->CopyAttribute(i,t,i);
          }
          CcInt* noOfNodes = new CcInt(true, v->compNo);
          newTuple->PutAttribute(i,noOfNodes);
          scctree->insert(newTuple,i+1);   // sort by CompNo
        }
        else return false; 
        it++;
      }
      return true;
    }


    void scc(ttree::TTree<Tuple*,TupleComp>* tree) {
      int compNo = 1;
      int index = 0;
      ttree::Iterator<Tuple*,TupleComp> it = tree->begin();
      std::stack<Vertex*>* stack = new std::stack<Vertex*>(); 
      // process first node
      Tuple* tuple = *it;
      scc(tuple,index,stack,compNo);  

      // process nodes
      while(it.hasNext()) {
        // find next node
        Tuple* t = *it;
        while(t->GetAttribute(0)->Compare(tuple->GetAttribute(0)) == 0) {
          it++;
          if(it.hasNext()) {
            tuple = *it;
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
             std::stack<Vertex*>* stack, int& compNo) {

      set<Vertex*,VertexComp>::iterator iter;
      Vertex* v = new Vertex(((CcInt*)tuple->GetAttribute(0))->GetIntval());
      iter = nodes->find(v);
      
      // v already seen
      if(iter != nodes->end()) {
        delete v;
        return;
      }

      // v not yet seen
      v->index = index;
      v->lowlink = index;
      index++;
      stack->push(v);
      v->inStack = true;
      nodes->insert(v);
      ttree::Iterator<Tuple*,TupleComp> it = tree->begin();
      Tuple* t = *it;
      Vertex* w;
      
      // find node in orel
      while(t->GetAttribute(0)->Compare(tuple->GetAttribute(0)) < 0) {
        it++;
        if(it.hasNext())
          t = *it; 
        else return;
      }

      // while node has adjacent nodes
      while(((CcInt*)t->GetAttribute(0))->GetIntval() == v->nr) {
        w = new Vertex(((CcInt*)t->GetAttribute(1))->GetIntval());
        iter = nodes->find(w);
        // w not seen yet
        if(iter == nodes->end()) { 
          it = tree->begin();
          t = *it;
          while(((CcInt*)t->GetAttribute(0))->GetIntval() != w->nr) {
            it++;
            if(it.hasNext())
              t = *it;     
            // no adjacent nodes for this node
            else {
              t = tuple;
              w->index = index;
              w->lowlink = index;
              w->compNo = compNo;
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
          v->setLowlink(min(v->lowlink,w->lowlink));
          // find next adjacent edge for v
          it = tree->begin();
          t = *it;
          while(t->GetAttribute(0)->Compare(tuple->GetAttribute(0)) < 0) {
            it++;
            if(it.hasNext())
              t = *it; 
            else return;
          }
        }
        // w already seen
        else {
          delete w;
          w = *iter;
          if(w->inStack) {
            v->setLowlink(min(v->lowlink,w->index));  
          }
        }
        it++;
        if(it.hasNext())
          t = *it;
        else break;
      }
      
      // root of scc found
      if (v->lowlink == v->index) { 
        v->compNo = compNo;
        while(true) {
          w = stack->top();
          stack->pop();
          w->inStack = false;
          if(v->nr == w->nr)
            break;
          w->compNo = compNo;
        }
        compNo++;
      }
    }

  private:
     ttree::TTree<Tuple*,TupleComp>* tree;
     ttree::TTree<Tuple*,TupleComp>* scctree;
     set<Vertex*,VertexComp>* nodes;
     ttree::Iterator<Tuple*,TupleComp> iter;
     TupleType* tt;
     int compNr;
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
      TupleType* tt = new TupleType(nl->Second(tupleType));

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
  moconnectedcomponentsValMap<CcString>,
  moconnectedcomponentsValMap<Mem>
};

int moconnectedcomponentsSelect(ListExpr args){
  return CcString::checkType(nl->First(args))?0:1;
}


/*

7.30.4 Description of operator ~moconnectedcomponents~

*/
OperatorSpec moconnectedcomponentsSpec(
    "{string, mem(orel(tuple(x))}  -> stream(Tuple)",
    "_ moconnectedcomponents",
    "",
    "query \"otestrel\" moconnectedcomponents"
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
  ListExpr first = nl->First(args);    
  if(!nl->HasLength(first,2)){
      return listutils::typeError("internal error");
  }
  string errMsg;
  if(!getMemType(nl->First(first), nl->Second(first), first, errMsg)){
    return listutils::typeError("string or mem(rel) expected : " + errMsg);
  }
  first = nl->Second(first); // remove mem
  if(!Relation::checkType(first)){
    return listutils::typeError("memory object is not a relation");
  }

    // check if second argument attribute of relation
  if(st == SortBy) {
    int numberOfSortAttrs = nl->ListLength(nl->Second(args));
    if(numberOfSortAttrs < 0){
      return listutils::typeError("Attribute list may not be empty!");
    }

    if(!listutils::isKeyDescription(nl->Second(first),
                                    nl->First(nl->Second(args)))) {
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
    mquicksortInfo(vector<Tuple*>* _relation, vector<int>* _pos) 
    : relation(_relation), pos(_pos) {
        
      quicksort(0,relation->size()-1,pos);
      it = relation->begin();
    }
    
    ~mquicksortInfo(){}
    
    void quicksort(int left, int right, vector<int>* pos) {
      
      int i = left, j = right;
      Tuple* tmp;
      Tuple* pivot = relation->at(((left + right) / 2)); 
            
      /* partition */
      while (i <= j) {
        
        while(TupleComp::smaller(relation->at(i),pivot,pos))
          i++;
        while (TupleComp::greater(relation->at(j),pivot,pos))
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
      if(it == relation->end()) 
        return 0;
      Tuple* res = *it;
      if(!res) {
        return 0;
      }
      it++;
      res->IncReference();
      return res;
    }

  private:
     vector<Tuple*>* relation;
     vector<int>* pos;
     vector<Tuple*>::iterator it;
};

/*

7.31.2 Value Mapping Function of operators ~mquicksort~ and ~mquicksortby~

*/
template<class T, SORTTYPE st>
int mquicksortValMap (Word* args, Word& result,
                    int message, Word& local, Supplier s) {

  mquicksortInfo* li = (mquicksortInfo*) local.addr;
  vector<int>* pos = new vector<int>();

  switch (message) {
    case OPEN: {
      
      if(li){
        delete li;
        local.addr=0;
      }
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
          pos->push_back(attrPos);
        }
      }
      else if(st == Sort) {
        ListExpr listrel = nl->Second(nl->Second(qp->GetType(s)));
        for(int i=0; i<nl->ListLength(listrel); i++)
          pos->push_back(i+1);
      }
      
      local.addr= new mquicksortInfo(rel->getmmrel(),pos); 
      return 0;
    }

    case REQUEST:
      result.addr=li?li->next():0;
      return result.addr?YIELD:CANCEL;

    case CLOSE:
      if(li) {
        delete pos;
        delete li;
        local.addr = 0;
      }
      return 0;
  }

  return -1;
}


ValueMapping mquicksortVM[] = {
  mquicksortValMap<CcString,Sort>,
  mquicksortValMap<Mem,Sort>,
  mquicksortValMap<CcString,SortBy>,
  mquicksortValMap<Mem,SortBy>,
};

template<SORTTYPE st>
int mquicksortSelect(ListExpr args){
  if(st == Sort)
    return CcString::checkType(nl->First(args))?0:1;
  else 
    return CcString::checkType(nl->First(args))?2:3;
}


/*
7.31.4 Description of operator ~mquicksort~

*/
OperatorSpec mquicksortSpec(
    "{string, mem(rel(tuple(x))} -> stream(Tuple)",
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
    "{string, mem(rel(tuple(x))} x ID -> stream(Tuple)",
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
bool dijkstra(graph::Graph* graph, Word arg, 
              graph::Vertex* start, graph::Vertex* dest) {
  
  graph::Queue* queue = new graph::Queue();

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
  queue->push(start);
  
  // as long as queue has entries
  while(!queue->empty()) {
      graph::Vertex* v = queue->top();
      queue->pop();
      if(v->wasSeen()) continue;
      v->isSeen(true);

      // process edges
      for(size_t i=0; i<v->getEdges()->size(); i++) {
        
        vector<graph::Edge*>* edges = v->getEdges();
        
        graph::Vertex* w = graph->getVertex(((CcInt*)edges->at(i)->
                    getEdge()->GetAttribute(edges->at(i)->
                    getPosDest()))->GetIntval());
        
        ArgVectorPointer funArgs = qp->Argument(arg.addr);
        Word funResult;
        ((*funArgs)[0]).setAddr(v->getEdges()->at(i)->getEdge());
        qp->Request(arg.addr,funResult);
        double cost = ((CcReal*)funResult.addr)->GetRealval(); 

        if(cost<0) {
            std::cout << "Error: cost is negative" << std::endl;
            queue->clear();
            delete queue;
            return false;
        }

        // shortening of path possible
        if(w->getCost() > v->getCost()+cost) {
            w->setCost(v->getCost()+cost);
            w->setDist(w->getCost());
            w->setPrev(v);
            queue->push(w);
        }
          
      }
      if(v->getNr() == dest->getNr()) {
        return true;
      }
  }
  
  queue->clear();
  delete queue;
  return false;
}

/*

~astar~

Calculates the shortest path between the vertices __start__ and
__dest__ in the __graph__ using the AStar-algorithm.

*/
bool astar(graph::Graph* graph, Word arg, Word arg2,
           graph::Vertex* start, graph::Vertex* dest) {
  
  graph::Queue* openlist = new graph::Queue();

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
  openlist->push(start);


  // as long as queue has entries
  while(!openlist->empty()) {
    graph::Vertex* v = openlist->top();
    openlist->pop();
    if(v->wasSeen()) continue;
    v->isSeen(true);
    
    if(v->getNr() == dest->getNr()) {
      return true;
    }
    
    // for every adjacent edge of v
    for(size_t i=0; i<v->getEdges()->size(); i++) {
      graph::Vertex* w = graph->getVertex(((CcInt*)v->getEdges()->at(i)->
                  getEdge()->GetAttribute(v->getEdges()->at(i)->getPosDest()))
                  ->GetIntval());
      ArgVectorPointer funArgs = qp->Argument(arg.addr);
      Word funResult;
      ((*funArgs)[0]).setAddr(v->getEdges()->at(i)->getEdge());
      qp->Request(arg.addr,funResult);
      double edgecost = ((CcReal*)funResult.addr)->GetRealval();      
      
      if(edgecost<0.0) {
        openlist->clear();
        delete openlist;
        return false;
      }
      
      ArgVectorPointer funArgs2 = qp->Argument(arg2.addr);
      Word funResult2;
      ((*funArgs2)[0]).setAddr(v->getEdges()->at(i)->getEdge());
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
          openlist->push(w);
      }
     }
      
  }
  openlist->clear();
  delete openlist;
  return false;
}

bool shortestPath(graph::Graph* graph, int startNode, 
                  int endNode, Word arg, Word arg2, 
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
      return listutils::typeError("four arguments expected");
  }
  
  else if(isAstar && nl->ListLength(args) != 6) {
      return listutils::typeError("five arguments expected");
  }

  ListExpr first = nl->First(args);
    
  if(!nl->HasLength(first,2)){
      return listutils::typeError("internal error");
  }
  
  string errMsg;

  if(!getMemType(nl->First(first), nl->Second(first), first, errMsg))
    return listutils::typeError("\n problem in first arg: " + errMsg);
  
  ListExpr graph = nl->Second(first); // remove leading mem
  if(!MemoryGraphObject::checkType(graph))
    return listutils::typeError("first arg is not a mem graph");
  

  ListExpr startNodeList = nl->First(nl->Second(args));
  ListExpr endNodeList = nl->First(nl->Third(args));
  ListExpr resultSelect = nl->First(nl->Fourth(args));
  ListExpr map = nl->First(nl->Fifth(args));
  ListExpr distMap;
  if(isAstar)
    distMap = nl->First(nl->Sixth(args));

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
                      int _resultSelect, Word _arg,
                      TupleType* _tt) 
      : graph(_graph),startNode(_startNode),
        endNode(_endNode), resultSelect(_resultSelect),
        arg(_arg), tt(_tt),seqNo(1) {
          
          result = new ttree::TTree<Tuple*,TupleComp>(16,18); 
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
    Tuple* res = *iterator;
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
  ttree::TTree<Tuple*,TupleComp>* result;
  ttree::Iterator<Tuple*,TupleComp> iterator;  
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
  mgshortestpathdValMap<CcString>,
  mgshortestpathdValMap<Mem>
};

int mgshortestpathdSelect(ListExpr args){
  return CcString::checkType(nl->First(args))?0:1;
}


/*

7.33.4 Description of operator ~mgshortestpathd~

*/
OperatorSpec mgshortestpathdSpec(
    "{string, mem(graph(tuple(x)))} x int x int x int -> "
    "stream(tuple(a1:t1,...an+1:tn+1))",
    "_ mgshortestpathd [_,_,_; fun] implicit parameter tuple type MTUPLE",
    "finds the shortest path between to given nodes in a main memory "
    "graph using dijkstras algorithm",
    "query mwrap('graph') mgshortestpathd "
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
                     Word _arg, Word _arg2, TupleType* _tt) 
      : graph(_graph),startNode(_startNode),
        endNode(_endNode),resultSelect(_resultSelect),
        arg(_arg),arg2(_arg2), tt(_tt),seqNo(1) {

          result = new ttree::TTree<Tuple*,TupleComp>(16,18); 
        
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
    Tuple* res = *iterator;
    if(res==NULL) {
      return 0;
    }
    iterator++;
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
  ttree::TTree<Tuple*,TupleComp>* result;
  ttree::Iterator<Tuple*,TupleComp> iterator;
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
  mgshortestpathaValMap<CcString>,
  mgshortestpathaValMap<Mem>
};

int mgshortestpathaSelect(ListExpr args){
  return CcString::checkType(nl->First(args))?0:1;
}


/*

7.34.4 Description of operator ~mgshortestpatha~

*/
OperatorSpec mgshortestpathaSpec(
    "{string, mem(graph(tuple(x)))} x int x int x int x (tuple->real) x "
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




int memglet(Word* args, Word& result,
            int message, Word& local, Supplier s, bool flob) {

    result  = qp->ResultStorage(s);
    CcBool* b = static_cast<CcBool*>(result.addr);
    bool memletsucceed = false;
    bool correct = true;

    CcString* oN = (CcString*) args[0].addr;
    if(!oN->IsDefined()){
        return 0;
    }
    string objectName = oN->GetValue();
    Supplier t = qp->GetSon( s, 1 );
    ListExpr le = qp->GetType(t);
    
    if(listutils::isOrelDescription(le)) {
        GenericRelation* rel = static_cast<Relation*>(args[1].addr);
        MemoryGraphObject* memgraph = new MemoryGraphObject();
        memletsucceed = memgraph->relToGraph(rel,le,getDBname(),flob);
        if(memletsucceed) {
          catalog->insert(objectName,memgraph);
//           memgraph->getgraph()->print(cout);
        }
        else 
          delete memgraph;
    }    
    else {
      correct = false;
    }

    b->Set(correct, memletsucceed);

    return 0;

}

/*
7.35 Operator ~memglet~ and ~memgletflob~

The operator ~memglet~ creates a main memory graph using adjacency list
from an ordered relation which represents a graph as well. As an additional
argument the name of the graph is required. If the creation
of the main memory graph is successful the operator returns true.
In addition the operator ~memgletflob~ loads the flobs of the attributes into
the main memory.

7.35.1 Type Mapping Functions of operator ~memlet~ and ~memgletflob~

string x orel(tuple(x)) -> bool

*/
ListExpr memgletTypeMap(ListExpr args) {
  
  cout << "memgletTypeMap" << endl;
    if(nl->ListLength(args)!=2){
        return listutils::typeError("wrong number of arguments");
    }
    ListExpr arg1 = nl->First(args);
    if (!CcString::checkType(nl->First(arg1))) {
        return listutils::typeError("string expected");
    }
    ListExpr str = nl->Second(arg1);
    string objectName = nl->StringValue(str);
    if (catalog->isMMObject(objectName)){
        return listutils::typeError("identifier already in use");
    }
    ListExpr arg2 = nl->Second(args);
    
    if(!listutils::isOrelDescription(nl->First(arg2))) {
      return listutils::typeError("the second argument has to be "
                                  "an ordered relation");
    }
    
    // check if tuples contain at least to integer values
    ListExpr orelTuple = nl->Second(nl->First(arg2));    

    if(!listutils::isTupleDescription(orelTuple)) {
      return listutils::typeError("second value of orel is not of type tuple");
    }

    ListExpr attrlist(nl->Second(orelTuple));

    if(!listutils::isAttrList(attrlist)) {
      return listutils::typeError("Error in orel attrlist.");
    }
    
    ListExpr rest = attrlist;
    bool foundSource = false;
    bool foundTarget = false;
    
    if(nl->ListLength(attrlist) >= 2) {
      
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
      if(!foundSource || !foundTarget)
        return listutils::typeError("orel has less than "
                                    "two integer attributes.");
    }
    else {
      return listutils::typeError("orel has less than 2 attributes.");
    }
    
    return listutils::basicSymbol<CcBool>();
}


/*

7.35.2  The Value Mapping Functions of operator ~memglet~

*/
int memgletValMap(Word* args, Word& result,
                 int message, Word& local, Supplier s) {

   return memglet(args, result, message, local,s, false);
}


/*

7.35.4 Description of operator ~memglet~

*/
OperatorSpec memgletSpec(
    "string x orel(tuple(x)) -> bool",
    "memglet (_,_)",
    "creates a main memory graph object from a given ordered relation",
    "query memglet ('graph', otestrel)"
);


/*

7.35.5 Instance of operator ~memglet~

*/
Operator memgletOp (
    "memglet",
    memgletSpec.getStr(),
    memgletValMap,
    Operator::SimpleSelect,
    memgletTypeMap
);

/*

7.36.2  The Value Mapping Functions of operator ~memgletflob~

*/
int memgletflobValMap (Word* args, Word& result,
                int message, Word& local, Supplier s) {

   return memglet(args, result, message, local, s,true);
}

/*

7.36.4 Description of operator ~memgletflob~

*/
OperatorSpec memgletflobSpec(
    "string x orel(tuple(x)) -> bool",
    "memgletflob (_,_)",
    "creates a main memory graph object from a given ordered relation "
    "and loads the flobs into the main memory",
    "query memgletflob ('graph', otestrel)"
);


/*

7.36.5 Instance of operator ~memgletflob~

*/
Operator memgletflobOp (
    "memgletflob",
    memgletflobSpec.getStr(),
    memgletflobValMap,
    Operator::SimpleSelect,
    memgletTypeMap
);


/*

7.37 Operator ~mgconnectedcomponents~

The operator ~mgconnectedcomponents~ computes the strongly connected 
components in a given main meory graph. the tuples of the graph
with the appended component number are returned in an output stream.

7.37.1 Type Mapping Functions of operator ~mgconnectedcomponents~

{string, mem(graph(tuple(x)))} -> stream(tuple(a1:t1,...an+1:tn+1))
    
*/
ListExpr mgconnectedcomponentsTypeMap(ListExpr args) {
  
    if(nl->ListLength(args) != 1) {
        return listutils::typeError("one argument expected");
    }

    ListExpr first = nl->First(args);
    
    if(!nl->HasLength(first,2)){
        return listutils::typeError("internal error");
    }
    
    string errMsg;

    if(!getMemType(nl->First(first), nl->Second(first), first, errMsg))
      return listutils::typeError("\n problem in first arg: " + errMsg);
    
    ListExpr graph = nl->Second(first); // remove leading mem
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
    lastlistn = nl->Append(lastlistn,
                          nl->TwoElemList(
                            nl->SymbolAtom("CompNo"),
                            listutils::basicSymbol<CcInt>()));
    
    return nl->TwoElemList(nl->SymbolAtom(Symbol::STREAM()),
                                        nl->TwoElemList(
                                          nl->SymbolAtom(Tuple::BasicType()),
                                          listn));
}


class mgconnectedComponentsInfo{
  public:
    mgconnectedComponentsInfo(graph::Graph* _graph,
                              TupleType* _tt)
        : graph(_graph),tt(_tt) {
      
      // compute the strongly connected components
      graph->tarjan();
      it = graph->getGraph()->begin();
      j = 0;
    }
    
    ~mgconnectedComponentsInfo(){
      tt->DeleteIfAllowed();
      graph->clearScc();
    }
    
    
    Tuple* next() {
      if(it == graph->getGraph()->end()) {
        graph->print(cout);
        return 0;
      }      
      // get next edge from graph
      graph::Vertex* v = *it;
      if(j>=v->getEdges()->size()) {
        it++;
        j=0;
        if(it != graph->getGraph()->end()) {
          v = *it;
        }
        else return 0;
      }
      while(v->getEdges()->size() == 0) {
        it++;
        if(it != graph->getGraph()->end()) {
          v = *it;
        }
        else return 0;
      }

      // get the tuple
      graph::Edge* e = v->getEdges()->at(j);
      Tuple* tup = e->getEdge();
      graph::Vertex* u = graph->getVertex(((CcInt*)tup->
                  GetAttribute(e->getPosDest()))->GetIntval()); 
      int compNo = u->getCompNo();
      j++;

      // add component number to output tuple
      CcInt* comp = new CcInt(true,compNo);
      Tuple* res = new Tuple(tt);
      int i = 0;
      for(; i<tup->GetNoAttributes(); i++) {
        res->CopyAttribute(i,tup,i);
      }     
      res->PutAttribute(i,comp);
      return res;  
    }
    
  private:
    graph::Graph* graph;
    TupleType* tt;
    size_t j;
    set<graph::Vertex*,graph::Vertex::EqualVertex>::iterator it;
};

/*

7.37.2 Value Mapping Function of operator ~mgconnectedcomponents~

*/
template<class T>
int mgconnectedcomponentsValMap (Word* args, Word& result,
                    int message, Word& local, Supplier s) {

  mgconnectedComponentsInfo* li = (mgconnectedComponentsInfo*) local.addr;

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
      
      local.addr= new mgconnectedComponentsInfo(memgraph->getgraph(),tt); 
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
  mgconnectedcomponentsValMap<CcString>,
  mgconnectedcomponentsValMap<Mem>
};

int mgconnectedcomponentsSelect(ListExpr args){
  return CcString::checkType(nl->First(args))?0:1;
}


/*

7.37.4 Description of operator ~mgconnectedcomponents~

*/
OperatorSpec mgconnectedcomponentsSpec(
    "(mem(graph)) -> stream(Tuple)",
    "_ mgconnectedcomponents",
    "computes the scc's for a given main memory graph",
    "query 'graph' mgconnectedcomponents"
);

/*

7.37.5 Instance of operator ~mgconnectedcomponents~

*/
Operator mgconnectedcomponentsOp (
    "mgconnectedcomponents",
    mgconnectedcomponentsSpec.getStr(),
    2,
    mgconnectedcomponentsVM,
    mgconnectedcomponentsSelect,
    mgconnectedcomponentsTypeMap
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




TypeConstructor MemoryRelObjectTC(
    MemoryRelObject::BasicType(),     // name of the type in SECONDO
    MemoryRelObject::Property,        // property function describing signature
    MemoryRelObject::Out, MemoryRelObject::In,          // out und in functions
    0, 0,                             // SaveToList, RestoreFromList functions
    // object creation and deletion create und delete
    MemoryRelObject::create,MemoryRelObject::Delete,
    MemoryRelObject::Open, MemoryRelObject::Save,        // object open, save
    MemoryRelObject::Close, MemoryRelObject::Clone,      // close and clone
    MemoryRelObject::Cast,                                // cast function
    MemoryRelObject::SizeOfObj,      // sizeof function
    MemoryRelObject::KindCheck);      // kind checking

GenTC<Mem> MemTC;


TypeConstructor MemoryORelObjectTC(
    MemoryORelObject::BasicType(),    // name of the type in SECONDO
    MemoryORelObject::Property,       // property function describing signature
    MemoryORelObject::Out, MemoryORelObject::In,        // out und in functions
    0, 0,                             // SaveToList, RestoreFromList functions
    // object creation and deletion create und delete
    MemoryORelObject::create,MemoryORelObject::Delete,
    MemoryORelObject::Open, MemoryORelObject::Save,        // object open, save
    MemoryORelObject::Close, MemoryORelObject::Clone,      // close and clone
    MemoryORelObject::Cast,                                // cast function
    MemoryORelObject::SizeOfObj,      // sizeof function
    MemoryORelObject::KindCheck);     // kind checking


class MainMemory2Algebra : public Algebra {

    public:
        MainMemory2Algebra() : Algebra() {
          catalog = new MemCatalog();

/*

8.2 Registration of Types


*/

          AddTypeConstructor (&MemoryRelObjectTC);
          MemoryRelObjectTC.AssociateKind( Kind::SIMPLE() );

          AddTypeConstructor (&MemTC);
          MemTC.AssociateKind( Kind::DATA() );
          
          AddTypeConstructor (&MemoryORelObjectTC);
          MemoryORelObjectTC.AssociateKind( Kind::SIMPLE() );
/*
8.3 Registration of Operators

*/
          AddOperator (&memloadOp);
          memloadOp.SetUsesArgsInTypeMapping();
          AddOperator (&memloadflobOp);
          memloadflobOp.SetUsesArgsInTypeMapping();
          AddOperator (&meminitOp);
          meminitOp.SetUsesMemory();
          AddOperator (&mfeedOp);
          mfeedOp.SetUsesArgsInTypeMapping();
          AddOperator (&letmconsumeOp);
          AddOperator (&letmconsumeflobOp);
          AddOperator (&memdeleteOp);
          memdeleteOp.SetUsesArgsInTypeMapping();
          AddOperator (&memobjectOp);
          memobjectOp.SetUsesArgsInTypeMapping();
          AddOperator (&memgetcatalogOp);
          AddOperator (&memletOp);
          memletOp.SetUsesArgsInTypeMapping();
          AddOperator (&memletflobOp);
          memletflobOp.SetUsesArgsInTypeMapping();
          AddOperator (&memupdateOp);
          memupdateOp.SetUsesArgsInTypeMapping();
          AddOperator (&mcreateRtreeOp);
          mcreateRtreeOp.SetUsesArgsInTypeMapping();
          AddOperator (&mcreateRtree2Op);
          AddOperator (&memsizeOp);
          AddOperator (&memclearOp);
          AddOperator (&meminsertOp);
          meminsertOp.SetUsesArgsInTypeMapping();
          AddOperator (&mwindowintersectsOp);
          mwindowintersectsOp.SetUsesArgsInTypeMapping();
          AddOperator (&mwindowintersectsSOp);
          mwindowintersectsSOp.SetUsesArgsInTypeMapping();
          AddOperator (&mconsumeOp);
          AddOperator (&mcreateAVLtreeOp);
          mcreateAVLtreeOp.SetUsesArgsInTypeMapping();

          AddOperator (&mcreateAVLtree2Op);
          AddOperator (&mexactmatchOp);
          mexactmatchOp.SetUsesArgsInTypeMapping();

          AddOperator (&mrangeOp);
          mrangeOp.SetUsesArgsInTypeMapping();
          AddOperator (&matchbelowOp);
          matchbelowOp.SetUsesArgsInTypeMapping();
          AddOperator(&mcreateMtree2Op);
          AddOperator(&mdistRange2Op);
          mdistRange2Op.SetUsesArgsInTypeMapping();
          AddOperator(&mdistScan2Op);
          mdistScan2Op.SetUsesArgsInTypeMapping();

          AddOperator(&mcreateMtreeOp);
          mcreateMtreeOp.SetUsesArgsInTypeMapping();
          AddOperator(&mdistRangeOp);
          mdistRangeOp.SetUsesArgsInTypeMapping();
          AddOperator(&mdistScanOp);
          mdistScanOp.SetUsesArgsInTypeMapping();

          AddOperator(&mexactmatchSOp);
          mexactmatchSOp.SetUsesArgsInTypeMapping();
          AddOperator(&mrangeSOp);
          mrangeSOp.SetUsesArgsInTypeMapping();
          AddOperator(&matchbelowSOp);
          matchbelowSOp.SetUsesArgsInTypeMapping();

          AddOperator(&gettuplesOp);
          gettuplesOp.SetUsesArgsInTypeMapping();
          
  ////////////////////// MainMemory2Algebra////////////////////////////
          
          AddOperator(&mwrapOp);
          mwrapOp.SetUsesArgsInTypeMapping();
          AddOperator(&MTUPLEOp);
          
          AddOperator(&mcreatettreeOp);
          mcreatettreeOp.SetUsesArgsInTypeMapping();
          AddOperator(&minsertttreeOp);
          minsertttreeOp.SetUsesArgsInTypeMapping();
          AddOperator(&mdeletettreeOp);
          mdeletettreeOp.SetUsesArgsInTypeMapping();
          
          AddOperator(&mcreateinsertrelOp);
          mcreateinsertrelOp.SetUsesArgsInTypeMapping();
          AddOperator(&minsertOp);
          minsertOp.SetUsesArgsInTypeMapping();
          AddOperator(&minsertsaveOp);
          minsertsaveOp.SetUsesArgsInTypeMapping();
          AddOperator(&minserttupleOp);
          minserttupleOp.SetUsesArgsInTypeMapping();
          AddOperator(&minserttuplesaveOp);
          minserttuplesaveOp.SetUsesArgsInTypeMapping();
          
          AddOperator(&mcreatedeleterelOp);
          mcreatedeleterelOp.SetUsesArgsInTypeMapping();
          AddOperator(&mdeleteOp);
          mdeleteOp.SetUsesArgsInTypeMapping();
          AddOperator(&mdeletesaveOp);
          mdeletesaveOp.SetUsesArgsInTypeMapping();
          AddOperator(&mdeletebyidOp);
          mdeletebyidOp.SetUsesArgsInTypeMapping();

          AddOperator(&mcreateupdaterelOp);
          mcreateupdaterelOp.SetUsesArgsInTypeMapping();
          AddOperator(&mupdateOp);
          mupdateOp.SetUsesArgsInTypeMapping();
          AddOperator(&mupdatesaveOp);
          mupdatesaveOp.SetUsesArgsInTypeMapping();
          AddOperator(&mupdatebyidOp);
          mupdatebyidOp.SetUsesArgsInTypeMapping();
          
          AddOperator(&moinsertOp);
          moinsertOp.SetUsesArgsInTypeMapping();
          AddOperator(&modeleteOp);
          modeleteOp.SetUsesArgsInTypeMapping();
          AddOperator(&moconsumeOp);
          AddOperator (&letmoconsumeOp);
          AddOperator (&letmoconsumeflobOp);
          AddOperator(&morangeOp);
          morangeOp.SetUsesArgsInTypeMapping();
          AddOperator(&moleftrangeOp);
          moleftrangeOp.SetUsesArgsInTypeMapping();
          AddOperator(&morightrangeOp);
          morightrangeOp.SetUsesArgsInTypeMapping();
          
          AddOperator(&moshortestpathdOp);
          moshortestpathdOp.SetUsesArgsInTypeMapping();
          AddOperator(&moshortestpathaOp);
          moshortestpathaOp.SetUsesArgsInTypeMapping();
          
          AddOperator(&moconnectedcomponentsOp);
          moconnectedcomponentsOp.SetUsesArgsInTypeMapping();
          
          AddOperator(&mquicksortOp);
          mquicksortOp.SetUsesArgsInTypeMapping();
          AddOperator(&mquicksortbyOp);
          mquicksortbyOp.SetUsesArgsInTypeMapping();
          
          AddOperator (&memgletOp);
          memgletOp.SetUsesArgsInTypeMapping();
          AddOperator (&memgletflobOp);
          memgletflobOp.SetUsesArgsInTypeMapping();
          
          AddOperator(&mgshortestpathdOp);
          mgshortestpathdOp.SetUsesArgsInTypeMapping();       
          AddOperator(&mgshortestpathaOp);
          mgshortestpathaOp.SetUsesArgsInTypeMapping();
          AddOperator(&mgconnectedcomponentsOp);
          mgconnectedcomponentsOp.SetUsesArgsInTypeMapping();
        
//           AddOperator(&momapmatchmhtOp);
//           momapmatchmhtOp.SetUsesArgsInTypeMapping(); 
        }
        
        ~MainMemory2Algebra() {};
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

