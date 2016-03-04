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
#include <exception>
#include <string>
#include <map>
#include <vector>
#include <limits>
#include <algorithm>

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
#include "MainMemory.h"
#include "Stream.h"
#include "MMRTree.h"
#include "MMMTree.h"
#include "FTextAlgebra.h"


#include "MovingRegionAlgebra.h"
#include "RectangleAlgebra.h"


std::ostream& operator<<(std::ostream& o, 
                         const std::pair<Attribute*,size_t>& t);

#include "AvlTree.h"

using namespace std;
extern NestedList* nl;
extern QueryProcessor *qp;
extern SecondoSystem* instance;

ostream& operator<<(ostream& o, const pair<Attribute*,size_t>& t){
  o << "(";
  t.first->Print(o);
  o << ", " << t.second   << ")";
  return o;
}


namespace mmalgebra {

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



string getDBname() {
    SecondoSystem* sys = SecondoSystem::GetInstance();
    return sys->GetDatabaseName();
}


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


int memload (Word* args, Word& result,
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
    if(Relation::checkType(objectTypeExpr)&&defined){
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
it breaks up. The created ~MemoryRelObject~ is usable but not complete.

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

~mfeed~ produces a stream of tuples from a main memory relation,
similar to the ~feed~-operator

*/

/*
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
    if(!Relation::checkType(arg)){
      return listutils::typeError("memory object is not a relation");
    }
    return nl->TwoElemList(nl->SymbolAtom(Symbol::STREAM()),
                           nl->Second(arg));
}


/*

5.4.3  The Value Mapping Functions of operator ~mfeed~

*/


class mfeedInfo{
  public:
     mfeedInfo(vector<Tuple*>* _relation):relation(_relation){
          it = relation->begin();
     }

    ~mfeedInfo(){}

     Tuple* next(){
       if(it==relation->end()) return 0;
       Tuple* res = *it;
       it++;
       res->IncReference();
       return res;
     }

  private:
     vector<Tuple*>* relation;
     vector<Tuple*>::iterator it;

};

template<class T>
int mfeedValMap (Word* args, Word& result,
                    int message, Word& local, Supplier s) {


   mfeedInfo* li = (mfeedInfo*) local.addr;

   switch (message)
   {
        case OPEN: {
             if(li){
             delete li;
             local.addr=0;
          }
          T* oN = (T*) args[0].addr;
          if(!oN->IsDefined()){
             return 0;
          }
          string objectName = oN->GetValue();

          MemoryObject* mo = catalog->getMMObject(objectName);
          if(!mo){
            cerr << "mfeed: object " << objectName << " not found" << endl;
            return 0;
          }
          ListExpr rt = catalog->getMMObjectTypeExpr(objectName);
          if(!Mem::checkType(rt)){
              cerr << "MainMemoryAlgebra internal error" << endl;
              return 0;
          }
          rt = nl->Second(rt);
          if(!Relation::checkType(rt)){
              cerr << "Type mismatch in mfeed: not a relation " << endl;
              return 0;
          }
          rt = nl->Second(rt);
          if(!nl->Equal(rt, nl->Second(qp->GetType(s)))){
              cerr << "Type mismatch in mfeed: attribute lists differ" << endl;
              return 0;
          }

          //the type mapping assures that it is a main memory member
          MemoryRelObject* mro = (MemoryRelObject*)mo;
          vector<Tuple*>* relation;
          relation = mro->getmmrel();
          local.addr= new mfeedInfo(relation);
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



ValueMapping mfeedVM[] = {
  mfeedValMap<CcString>,
  mfeedValMap<Mem>
};

int mfeedSelect(ListExpr args){
  return CcString::checkType(nl->First(args))?0:1;
}


/*

5.4.4 Description of operator ~mfeed~


*/



OperatorSpec mfeedSpec(
    "{string, mem(rel(tuple(x))}  -> stream(Tuple)",
    "_ mfeed",
    "produces a stream from a main memory relation",
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


int letmconsume (Word* args, Word& result,
                int message, Word& local, Supplier s, bool flob) {

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
similar to the ~consume~-operator. The name of the main memory relation is given
by the second parameter.

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

5.5.3  The Value Mapping Functions of operator ~letmconsume~

*/

int letmconsumeValMap (Word* args, Word& result,
                int message, Word& local, Supplier s) {

    return letmconsume(args, result, message, local, s, false);
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
    letmconsumeValMap,
    Operator::SimpleSelect,
    letmconsumeTypeMap
);


/*

5.5 Operator ~letmconsumeflob~

~letmconsumeflob~ produces a main memory relation from a stream(tuples),
similar to the ~consume~-operator. The name of the main memory relation is given
by the second parameter. The associated flobs will be loaded to main memory too.

*/

/*

5.5.3  The Value Mapping Functions of operator ~letmconsumeflob~

*/

int letmconsumeflobValMap (Word* args, Word& result,
                int message, Word& local, Supplier s) {

    return letmconsume(args, result, message, local, s, true);
}




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
    letmconsumeflobValMap,
    Operator::SimpleSelect,
    letmconsumeTypeMap
);

/*
5.6 Operator ~memdelete~

~memdelete~ deletes an object from main memory

*/

/*
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

*/

/*
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
        bool isDATA = Attribute::checkType(qp->GetType(s));
        T* oN = (T*) args[0].addr;
        if(!oN->IsDefined()){
            if(isDATA){
               Attribute* res = (Attribute*) result.addr;
               res->SetDefined(0);
            }
            return 0;
        }
        string objectName = oN->GetValue();

        if(   !catalog->isMMObject(objectName) 
           || !catalog->isAccessible(objectName)){
            cerr << "Object " << objectName << " not found in memory catalog"
                 << endl;
            if(isDATA){
               Attribute* res = (Attribute*) result.addr;
               res->SetDefined(0);
            }
            return 0;
        }
        ListExpr typeExpr = catalog->getMMObjectTypeExpr(objectName);
        if(!Mem::checkType(typeExpr)){
            cerr << "invalid type found in catalog" << endl;
            if(isDATA){
               Attribute* res = (Attribute*) result.addr;
               res->SetDefined(0);
            }
            return 0;
        }
        typeExpr = nl->Second(typeExpr);
        if(!nl->Equal(qp->GetType(s), typeExpr)){
            cerr << "expected type and type found in catalog differ" << endl;
            if(isDATA){
               Attribute* res = (Attribute*) result.addr;
               res->SetDefined(0);
            }
            return 0;
        }

        if(isDATA){
            MemoryAttributeObject* memObject =
                    (MemoryAttributeObject*)catalog->getMMObject(objectName);
            Attribute* attr = (Attribute*) result.addr;
            attr->CopyFrom(memObject->getAttributeObject());
            result.setAddr(attr);
            return 0;
        }
    
        if(Relation::checkType(typeExpr)){ 
            MemoryRelObject* memObject =
                    (MemoryRelObject*)catalog->getMMObject(objectName);
            GenericRelation* rel = (GenericRelation*)result.addr;
            if(rel->GetNoTuples() > 0) {
                rel->Clear();
            }

            vector<Tuple*>* relation = memObject->getmmrel();
            vector<Tuple*>::iterator it;
            it=relation->begin();

            while( it!=relation->end()){
                Tuple* tup = *it;
                rel->AppendTuple(tup);
                it++;
            }
            return 0;
        }
        cerr << "memObject: unsupported Type" << endl;
        return 0;
}

/*

5.7.4 Description of operator ~memobject~

*/
OperatorSpec memobjectSpec(
    "string | mem(X)  -> m:MEMLOADABLE , X in {DATA, rel}",
    "memobject (_)",
    "returns a persistent Object created from a main memory Object",
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
    "returns a stream(Tuple) with information of main memory objects",
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


int memlet (Word* args, Word& result,
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

    if(Relation::checkType(le)){
        GenericRelation* rel= static_cast<Relation*>( args[1].addr );
        MemoryRelObject* mmRelObject = new MemoryRelObject();
        memletsucceed = mmRelObject->relToVector(rel,
        le, getDBname(), flob);
        if (memletsucceed) {
            catalog->insert(objectName,mmRelObject);
        } else {
            delete mmRelObject;
        }
    } else if (Attribute::checkType(le)){
        Attribute* attr = (Attribute*)args[1].addr;
         MemoryAttributeObject* mmA = new MemoryAttributeObject();
         memletsucceed = mmA->attrToMM(attr, le,
                                getDBname(),flob);
         if (memletsucceed){
            catalog->insert(objectName, mmA);
         }
         else {
            delete mmA;
         }
    } else if(Stream<Tuple>::checkType(le)){
        MemoryRelObject* mmRelObject = new MemoryRelObject();
            memletsucceed = mmRelObject->tupleStreamToRel(args[1],
        le, getDBname(), flob);
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

*/

/*
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
    if (listutils::isRelDescription(nl->First(arg2)) ||
        listutils::isDATA(nl->First(arg2)) ||
        listutils::isTupleStream(nl->First(arg2))){
            return listutils::basicSymbol<CcBool>();
    }
    return listutils::typeError ("the second argument has to "
    " be of kind DATA or a relation or a tuplestream");
}


/*

5.9.3  The Value Mapping Functions of operator ~memlet~

*/

int memletValMap (Word* args, Word& result,
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
    "string x m:MEMLOADABLE -> bool",
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


*/

/*
5.10.1 Type Mapping Functions of operator ~memupdate~
        (string x m:MEMLOADABLE -> bool)

*/
ListExpr memupdateTypeMap(ListExpr args)
{

    if(nl->ListLength(args)!=2){
        return listutils::typeError("wrong number of arguments");
    }

    ListExpr arg2 = nl->First(nl->Second(args));
    // replace <stream> by <rel> if necessary
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

*/

/*
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
    unsigned long availableMemSize = catalog->getAvailabeMemSize();
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
    "creates an mmrtree over a main memory relation given by the"
    "first string || mem(rel(tuple)) and an attribute"
    "given by the second argument",
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

*/

/*
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
    unsigned long availableMemSize = catalog->getAvailabeMemSize();

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
    "stream(Tuple) x iDENT  x string -> mem(rtree)",
    "_ mcreateRtree2 [_,_]",
    "creates an mmrtree<d>, the key type must be of kind SPATAL2D"
    "SPATIAL3D, SPATIAL4D, SPATIAL8D, or of type rect>d>",
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

*/

/*
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

*/

/*
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
5.13 Operator ~minsert~

inserts the tuple of a stream into a existing main memory relation

*/

/*
5.13.1 Type Mapping Functions of operator ~minsert~
    (stream(tuple(x)) x string -> stream(tuple(x))
    the second argument identifies the main memory relation

*/

ListExpr minsertTypeMap(ListExpr args)
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


class minsertInfo{
  public:
     minsertInfo( Word w, vector<Tuple*>* _relation, bool _flob):
          stream(w),relation(_relation), flob(_flob){
        stream.open();
     }

    ~minsertInfo(){
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

5.13.3  The Value Mapping Functions of operator ~minsert~

*/
template<class T>
int minsertValMap (Word* args, Word& result,
                    int message, Word& local, Supplier s) {

    minsertInfo* li = (minsertInfo*) local.addr;

    switch (message)
    {
        case OPEN: {
            if(li){
               delete li;
               local.addr=0;
            }
            T* oN = (T*) args[1].addr;
            if(!oN->IsDefined()){
                return 0;
            }
            string objectName = oN->GetValue();
            if(!catalog->isMMObject(objectName)){
              return 0;
            }
            ListExpr memType = catalog->getMMObjectTypeExpr(objectName);
            if(!Mem::checkType(memType)){
              return 0;
            }
            memType = nl->Second(memType);
            if(!Relation::checkType(memType)){
               return 0;
            }
            ListExpr st = qp->GetType(qp->GetSon(s,0));
            if(!nl->Equal(nl->Second(st), nl->Second(memType))){
               return 0;
            }
            MemoryRelObject* mro =
                    (MemoryRelObject*)catalog->getMMObject(objectName);

            local.addr = new minsertInfo(args[0], mro->getmmrel(), 
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

ValueMapping minsertVM[] = {
   minsertValMap<CcString>,
   minsertValMap<Mem>
};

int minsertSelect(ListExpr args){
   return CcString::checkType(nl->Second(args))?0:1;
}



/*

5.13.4 Description of operator ~minsert~

*/

OperatorSpec minsertSpec(
    "stream(Tuple) x {string, mem(rel)}  -> stream(Tuple)",
    "minsert(_,_)",
    "inserts the tuple of a stream into a "
    "existing main memory relation",
    "query minsert (ten feed head[5],'ten') count"
);

/*

5.13.5 Instance of operator ~minsert~

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
5.14 Operator ~mwindowintersects~
        Uses the given MemoryRtreeObject (as first argument)to find all tuples
        in the given MemoryRelObject (as second argument)
        with intersects the third argument value's bounding box

*/

/*
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
             T* tree = (T*) args[0].addr;
             R* rel = (R*) args[1].addr;
             typedef StandardSpatialAttribute<dim> W;
             typedef  MemoryRtreeObject<dim> treetype;
             W* window = (W*) args[2].addr;
             if(    !tree->IsDefined() || !rel->IsDefined() 
                 || !window->IsDefined()){
                 return 0;
             }
             string ts = tree->GetValue();
             string rs = rel->GetValue();
             Rectangle<dim> box = window->BoundingBox();
             if(!catalog->isMMObject(ts) || !catalog->isMMObject(rs)
                || !box.IsDefined()){
                return 0;
             }
             ListExpr rt = nl->Second(catalog->getMMObjectTypeExpr(rs));
             if(!Relation::checkType(rt)){
                // not a relation
                return 0;
             }
             if(!nl->Equal(nl->Second(rt) , nl->Second(qp->GetType(s)))){
               // different tuple types
               return 0;
             }
             ListExpr tt = nl->Second(catalog->getMMObjectTypeExpr(ts));
             if(!rtreehelper::checkType(tt)){
                // not an r-tree
                return 0;
             }  
             int rtdim = nl->IntValue(nl->Second(tt));
             if(rtdim != dim){
                 return 0;
             }  
             MemoryRelObject* r = (MemoryRelObject*) catalog->getMMObject(rs);
             treetype* t = (treetype*) catalog->getMMObject(ts);
             local.addr = new mwiInfo<dim>(t, r, box);
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
    "string x string x T -> stream(Tuple) "
    "where T in {rect<d>} U SPATIAL2D U SPATIAL3D U SPATIAL4D U SPATIAL8D",
    "mwindowintersects(_,_,_)",
    "Uses the given rtree to find all tuples"
      " in the given relation which intersects the "
      " argument value's bounding box.",
    "query mwindowintersects"
    "('strassen_GeoData', 'strassen', bbox(thecenter)) count"
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

  string err = " {string, memory(rtree )} x SPATIALxD expected";
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
             typedef StandardSpatialAttribute<dim> boxtype;
             boxtype* o = (boxtype*) args[1].addr;
             if(!name->IsDefined() || !o->IsDefined()){
               return 0;
             }
             string n = name->GetValue();
             Rectangle<dim> box = o->BoundingBox();
             if(!catalog->isMMObject(n) || !box.IsDefined()){
               return 0;
             }
             ListExpr tt = catalog->getMMObjectTypeExpr(n);
             if(!Mem::checkType(tt) || 
                !rtreehelper::checkType(nl->Second(tt))){
               return 0;
             }
             if(nl->IntValue(nl->Second(nl->Second(tt)))!=dim){
                return 0;
             }
             MemoryRtreeObject<dim>* mr = (MemoryRtreeObject<dim>*) 
                                     catalog->getMMObject(n);
             local.addr = mr->getrtree()->find(box);
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
  " {string, memory(rtree)} x SPATIALxD -> stream(tid)",
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

*/

/*

5.4.1 Type Mapping Functions of operator ~mconsume~
        (stream(Tuple) -> memoryRelObject)

*/
ListExpr mconsumeTypeMap(ListExpr args)
{
    if(nl->ListLength(args)!=1){
        return listutils::typeError("(wrong number of arguments)");
    }

    if (!Stream<Tuple>::checkType(nl->First(args))) {
        return listutils::typeError ("stream(Tuple) expected!");
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
    Tuple* tup=0;
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
    "collects the objects from a stream(Tuple)",
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

*/

/*
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
    if(!Mem::checkType(memObjectType)){
       cerr << "invalid object name " << endl;
       str->SetDefined(false);
       return 0;
    }
    
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
       cerr << "not eneogh attributes in tuple";
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

    avltree::AVLTree< pair<Attribute*,size_t>, KeyComparator >* tree
            = new avltree::AVLTree< pair<Attribute*,size_t>, KeyComparator >;
    Attribute* attr;
    pair<Attribute*,size_t> aPair;

    size_t usedMainMemory = 0;
    unsigned long availableMemSize = catalog->getAvailabeMemSize();

    while ( it!=relVec->end()){
        Tuple* tup = *it;
        attr=tup->GetAttribute(attrPos);
        aPair = pair<Attribute*,size_t>(attr,i);
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
    "given by the second string",
    "query 'Staedte' mcreateAVLtree ['SName']"
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
5.17 Operator ~mexactmatch~
        Uses the given MemoryAVLObject (as first argument)to find all tuples
        in the given MemoryRelObject (as second argument)
        with the same key value


5.17.1 Type Mapping Functions of operator ~mexactmatch~
    string x string x key -> stream(Tuple)


*/


ListExpr mexactmatchTypeMap(ListExpr args)
{
    string err ="{string, mem(avltree(t)) } x {string, mem(rel)} x T expected";
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

    if(!MemoryAVLObject::checkType(a1t)){
      return listutils::typeError("first arg is not an avl tree");
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
           avltree::AVLTree< pair<Attribute*,size_t>, KeyComparator >* _tree,
           vector<Tuple*>* _relation, 
           Attribute* _attr1,
           Attribute* _attr2, 
           string _keyType)
           :relation(_relation), tree(_tree), attr1(_attr1), attr2(_attr2),
           keyType(_keyType){
           it = tree->tail(pair<Attribute*,size_t>(attr1,0));
           res = true; 
        }


        ~avlOperLI(){}


        Tuple* next(){
            if(it.onEnd()){
                 return 0;
            }
            hit = it.Get();

            // special treatment for string type , really a good idea???
            if (keyType=="string"){
                string attr1ToString = ((CcString*) attr1)->GetValue();
                string attr2ToString = ((CcString*) attr2)->GetValue();
                string hitString = ((CcString*)(hit->first))->GetValue();
                hitString=trim(hitString);

                if (hitString > attr2ToString) {
                    return 0;
                }
                if (hitString == attr1ToString ||
                    hitString < attr2ToString  ||
                    hitString == attr2ToString ){

                    Tuple* result = relation->at(hit->second);
                    result->IncReference();
                    it.Next();
                    return result;
                }
                return 0;
            } //end keyType string

            if ((hit->first)->Compare(attr2) > 0){ // end reached
                return 0;
            }

            Tuple* result = relation->at(hit->second);
            result->IncReference();
            it.Next();
            return result;
        }




        Tuple* matchbelow(){
            if (res){
                int i = relation->size();
                hit = tree->GetNearestSmallerOrEqual
                            (pair<Attribute*,size_t>(attr1,i));
                if (hit==0){
                    return 0;
                }
                Tuple* result = relation->at(hit->second);
                result->IncReference();
                res = false;
                return result;
            }
            return 0;
            }

    private:
        vector<Tuple*>* relation;
        avltree::AVLTree< pair<Attribute*,size_t>, KeyComparator >* tree;
        Attribute* attr1;
        Attribute* attr2;
        string keyType;
        avltree::AVLTree< pair<Attribute*, size_t>,
                            KeyComparator >::iterator it;
        const pair<Attribute*,size_t>* hit;
        bool res;
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
            T* tree = (T*) args[0].addr;
            R* rel = (R*) args[1].addr;
            Attribute* key = (Attribute*) args[2].addr;

            if(!tree->IsDefined() || !rel->IsDefined()){
               return 0;
            }

            string treeN = tree->GetValue();
            string relN = rel->GetValue();
            if(!catalog->isMMObject(treeN) || ! catalog->isMMObject(relN)){
               cerr << "not a memory object" << endl;
               return 0;
            }
            ListExpr treeT = catalog->getMMObjectTypeExpr(treeN);
            ListExpr relT = catalog->getMMObjectTypeExpr(relN);
            if(!Mem::checkType(treeT) || !Mem::checkType(relT)){
              cerr << "internal error" << endl;
              return 0;
            }
            treeT = nl->Second(treeT);
            relT = nl->Second(relT);
            if(   !MemoryAVLObject::checkType(treeT) 
               || !Relation::checkType(relT)){
               cerr << "invalid type" << endl;
               return 0;
            }
            ListExpr keyT = qp->GetType(qp->GetSon(s,2));
            if(!nl->Equal(keyT, nl->Second(treeT))){
               cerr << "avl type and key type differ" << endl;
               return 0;
            }
            if(!nl->Equal(nl->Second(relT), nl->Second(qp->GetType(s)))){
                cerr << "unexpected tuple description" << endl;
                return 0;
            }
            MemoryAVLObject* avlObject =
                    (MemoryAVLObject*)catalog->getMMObject(treeN);

            MemoryRelObject* mro =
                    (MemoryRelObject*)catalog->getMMObject(relN);

            local.addr= new avlOperLI(avlObject->getAVLtree(),
                                      mro->getmmrel(),
                                      key,key,
                                      nl->ToString(keyT));
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
    "string x string x key -> stream(Tuple) ",
    "_ _ mexactmatch[_]",
    "Uses the given MemoryAVLObject (as first argument)to find all tuples"
        "in the given MemoryRelObject (as second argument)"
        "which have the same attribute value",
    "query 'Staedte_SName', 'Staedte'mexactmatch ['Dortmund'] count"
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
    "string x string x key -> stream(Tuple) ",
    "_ _ matchbelow[_]",
    "returns for a key X (third argument) the tuple which "
    " contains the biggest attribute value in the AVLtree (first argument) "
    " which is smaller or equal X",
    "query 'Staedte_SName' 'Staedte' matchbelow ['Dortmund'] count"

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
        Uses the given MemoryAVLObject (as first argument)to find all tuples
        in the given MemoryRelObject (as second argument)
        which are between the first and the second attribute value
        (as third and fourth argument)

*/

/*
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

    if(!MemoryAVLObject::checkType(a1)){
      return listutils::typeError(err + " (first arg is not an avl tree)");
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
            T* tree = (T*) args[0].addr;
            R* rel = (R*) args[1].addr;
            if(!tree->IsDefined() || !rel->IsDefined()){
               cerr << "undefined object" << endl;
               return 0;
            }
            string treeN = tree->GetValue();
            string relN = rel->GetValue();
            if(!catalog->isMMObject(treeN) || !catalog->isMMObject(relN)){
               cerr << "memory object not found" << endl;
               return 0;
            } 
            ListExpr treeT = nl->Second(catalog->getMMObjectTypeExpr(treeN));
            ListExpr relT = nl->Second(catalog->getMMObjectTypeExpr(relN));
            if(   !MemoryAVLObject::checkType(treeT) 
               || !Relation::checkType(relT)){
               cerr << "detected type problem";
               return 0;
            }
            ListExpr keyT = qp->GetType(qp->GetSon(s,2));
            if(!nl->Equal(nl->Second(treeT), keyT)){
              cerr << "avl key differs to range keys";
              return 0;
            }
            MemoryAVLObject* avlObject =
                    (MemoryAVLObject*)catalog->getMMObject(treeN);

            MemoryRelObject* mro =
                    (MemoryRelObject*)catalog->getMMObject(relN);
            Attribute* key1 = (Attribute*) args[2].addr;
            Attribute* key2 = (Attribute*) args[3].addr;

            local.addr= new avlOperLI(avlObject->getAVLtree(),
                                      mro->getmmrel(),
                                      key1,key2,
                                      nl->ToString(keyT));
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

5.18.4 Description of operator ~range~

*/

OperatorSpec mrangeSpec(
    "{string,mem} x {string,mem} x key x key -> stream(Tuple) ",
    "_ _ mrange[_,_]",
    "Uses the given rtree to find all tuples"
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

   MemoryMtreeObject<pair<T, TupleId>,StdDistComp<T> >* mtree = 
          new MemoryMtreeObject<pair<T,TupleId>, StdDistComp<T> > (tree,  
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
  "query kinos feed addid mcreateMtree2[GeoData, TID, \"kinos_mtree\""
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
            if(!Name->IsDefined()){
               return 0;
            }            
            string name = Name->GetValue();
            if(!catalog->isMMObject(name)){
               return 0;
            }
            ListExpr t = nl->Second(catalog->getMMObjectTypeExpr(name));
            ListExpr expType = nl->TwoElemList(
                                nl->SymbolAtom(mtreehelper::BasicType()),
                                qp->GetType(qp->GetSon(s,1)));
            if(!nl->Equal(t,expType)){
              return 0;
            }

            MemoryMtreeObject<pair<T, TupleId>,StdDistComp<T> >* mtreeo  =
               (MemoryMtreeObject<pair<T, TupleId>,StdDistComp<T> >*)
                   catalog->getMMObject(name);
            if(mtreeo){
              MMMTree<pair<T, TupleId>,StdDistComp<T> >* mtree 
                = mtreeo->getmtree();
              if(mtree){
                  T a = *attr;
                  local.addr = mtree->rangeSearch(pair<T,TupleId>(a,0), d);
              }
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
            if(!Name->IsDefined()){
              return 0;
            }
            string name = Name->GetValue();
            if(!catalog->isMMObject(name)){
              return 0;
            }
            ListExpr treeT = nl->Second(catalog->getMMObjectTypeExpr(name));
            ListExpr keyT = qp->GetType(qp->GetSon(s,1));
            if(!mtreehelper::checkType(treeT, keyT)){
              return 0;
            }

            MemoryMtreeObject<pair<T, TupleId>,StdDistComp<T> >* mtreeo  =
               (MemoryMtreeObject<pair<T, TupleId>,StdDistComp<T> >*)
                   catalog->getMMObject(name);
            MMMTree<pair<T, TupleId>,StdDistComp<T> >* mtree = 
                    mtreeo->getmtree();
            if(mtree){
              T a = *attr;
              pair<T,TupleId> p(a,0);
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

  ListExpr resNameV = nl->Second(nl->Third(args));
  if(nl->AtomType(resNameV)!=StringType){
    return listutils::typeError("third arg must be a constant string");
  }
  if(catalog->isMMObject(nl->StringValue(resNameV))){
    return listutils::typeError("memory object already there.");
  }

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
   MemoryMtreeObject<pair<T, TupleId>,StdDistComp<T> >* mtree = 
          new MemoryMtreeObject<pair<T,TupleId>, StdDistComp<T> > (tree,  
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

     distRangeInfo( MemoryMtreeObject<pair<T,TupleId>, StdDistComp<T> >* mtree, 
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
               T* tree = (T*) args[0].addr;
               R* rel = (R*) args[1].addr;
               CcReal* dist = (CcReal*) args[3].addr;
               if(!tree->IsDefined() || !rel->IsDefined()
                  || !dist->IsDefined()){
                 cerr << "tree, rel or dist undefined" << endl;
                 return 0;
               }          
               string treeN = tree->GetValue();
               string relN = rel->GetValue();

               if(!catalog->isMMObject(treeN) || !catalog->isMMObject(relN)){
                  cerr << "tree or rel not found in memory" << endl;
                  return 0;
               }
               ListExpr relT = nl->Second(catalog->getMMObjectTypeExpr(relN));
               ListExpr treeT = nl->Second(catalog->getMMObjectTypeExpr(treeN));
               if(!Relation::checkType(relT)){
                  cerr << "second arg is not a relation" << endl;
                  return 0;
               }
               if(   !nl->HasLength(treeT,2) 
                  || !listutils::isSymbol(nl->First(treeT), 
                        mtreehelper::BasicType())){
                  cout << "first arg is not a m-tree" << endl;
                  return 0;
               }     
               if(!K::checkType(nl->Second(treeT))){
                  return 0;
               }                
               K* key = (K*) args[2].addr;
               double d = dist->GetValue();
               MemoryMtreeObject<pair<K,TupleId>, StdDistComp<K> >* m =
                    (MemoryMtreeObject<pair<K,TupleId>, StdDistComp<K> >*) 
                      catalog->getMMObject(treeN);
               MemoryRelObject* relo = (MemoryRelObject*) 
                         catalog->getMMObject(relN);
               if(m && relo){
                   local.addr = new distRangeInfo<K>(m,relo,key,d);
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
  "string x string x DATA x real -> stream(tid) ",
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

     distScanInfo( MemoryMtreeObject<pair<T,TupleId> , StdDistComp<T> >* mtree, 
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
               T* tree = (T*) args[0].addr;
               R* rel = (R*) args[1].addr;
               if(!tree->IsDefined() || !rel->IsDefined()){
                 // undefined mem object
                 return 0;
               }
               string relN = rel->GetValue();
               string treeN = tree->GetValue();
               if(   !catalog->isMMObject(relN) 
                  || !catalog->isMMObject(treeN)){
                 // not found in memory
                 return 0;
               } 
               ListExpr relT = nl->Second(catalog->getMMObjectTypeExpr(relN));
               if(!Relation::checkType(relT)){
                 // second arg is not a relation
                 return 0;
               }

               ListExpr treeT = nl->Second(catalog->getMMObjectTypeExpr(treeN));
               if(   !nl->HasLength(treeT,2) 
                  || !listutils::isSymbol(nl->First(treeT),
                                          mtreehelper::BasicType())){
                 // fisrt arg in not an m tree
                 return 0;
               }
               if(!K::checkType(nl->Second(treeT))){
                 // m-tree type and search type differ
                 return 0;
               }
               K* key = (K*) args[2].addr;
               MemoryMtreeObject<pair<K,TupleId>, StdDistComp<K> >* m =
                       (MemoryMtreeObject<pair<K,TupleId>, StdDistComp<K> >*) 
                        catalog->getMMObject(treeN);
               MemoryRelObject* relo = (MemoryRelObject*) 
                        catalog->getMMObject(relN);
               if(m && rel){
                     local.addr = new distScanInfo<K>(m,relo,key);
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
  "string x string x DATA -> stream(tuple) ",
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






class MainMemoryAlgebra : public Algebra
{


    public:
        MainMemoryAlgebra() : Algebra()
        {
            catalog = new MemCatalog();

/*

6.2 Registration of Types


*/

        AddTypeConstructor (&MemoryRelObjectTC);
        MemoryRelObjectTC.AssociateKind( Kind::SIMPLE() );

        AddTypeConstructor (&MemTC);
        MemTC.AssociateKind( Kind::DATA() );
/*
6.3 Registration of Operators

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
        AddOperator (&minsertOp);
        minsertOp.SetUsesArgsInTypeMapping();
        AddOperator (&mwindowintersectsOp);
        mwindowintersectsOp.SetUsesArgsInTypeMapping();
        AddOperator (&mwindowintersectsSOp);
        mwindowintersectsSOp.SetUsesArgsInTypeMapping();
        AddOperator (&mconsumeOp);
        AddOperator (&mcreateAVLtreeOp);
        mcreateAVLtreeOp.SetUsesArgsInTypeMapping();

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


        }
        ~MainMemoryAlgebra() {
        };
};





} // ende namespace mmalgebra

/*
7 Initialization

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
InitializeMainMemoryAlgebra(NestedList* nlRef, QueryProcessor* qpRef)
{

  return (new mmalgebra::MainMemoryAlgebra);
}




