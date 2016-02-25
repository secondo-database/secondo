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

string getDBname() {
    SecondoSystem* sys = SecondoSystem::GetInstance();
    return sys->GetDatabaseName();
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

    CcString* oN = (CcString*) args[0].addr;
          if(!oN->IsDefined()){
             return 0;
          }

    string objectName = oN->GetValue();
    SecondoCatalog* cat = SecondoSystem::GetCatalog();
    bool memloadSucceeded=false;
    Word object; //save the persistent object
    ListExpr objectTypeExpr=0; //type expression of the persistent object
    string objectTypeName=""; //used by cat->GetObjectTypeExpr
    bool defined = false;
    bool hasTypeName = false;

    memloadSucceeded = cat->GetObjectExpr(objectName,
                objectTypeName,objectTypeExpr,object,defined,hasTypeName);


    // object is a relation
    if (Relation::checkType(objectTypeExpr)&&defined){
        GenericRelation* r= static_cast<Relation*>( object.addr );
        MemoryRelObject* mmRelObject = new MemoryRelObject();
        memloadSucceeded = mmRelObject->relToVector(r,
        nl->Second(objectTypeExpr), getDBname(), flob);
        if (memloadSucceeded) {
            catalog->insert(objectName,mmRelObject);
        } else {
            delete mmRelObject;
        }
    }

    // object is attribute
    if (Attribute::checkType(objectTypeExpr)&&defined){
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

    }

    // object is neither an attribute nor a relation
    if (!Attribute::checkType(objectTypeExpr)
            &&!Relation::checkType(objectTypeExpr)){
        memloadSucceeded = false;
        cout<<"a relation or an attribute expected"<<endl;
    }

    result  = qp->ResultStorage(s);
    CcBool* b = static_cast<CcBool*>(result.addr);
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

    if (!CcString::checkType(nl->First(arg))) {
        return listutils::typeError("string expected");
    };

    ListExpr fn = nl->Second(arg);

    if(nl->AtomType(fn)!=StringType){
        return listutils::typeError("error");
    }

    string oN = nl->StringValue(fn);
    ListExpr oTeList = catalog->getMMObjectTypeExpr(oN);

    if(!catalog->isMMObject(oN) ||
            !listutils::isTupleDescription(oTeList))
    {
      return listutils::typeError("not a MainMemory member or not a relation");
    }

    if(!catalog->isAccessible(oN)){
        return listutils::typeError("MainMemory object not accessible");
    }
    return nl->TwoElemList(nl->SymbolAtom(Symbol::STREAM()),oTeList);
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
          CcString* oN = (CcString*) args[0].addr;
          if(!oN->IsDefined()){
             return 0;
          }
          string objectName = oN->GetValue();
          vector<Tuple*>* relation;

          //the type mapping assures that it is a main memory member
          MemoryRelObject* mro =
                    (MemoryRelObject*)catalog->getMMObject(objectName);
          relation = mro->getmmrel();

          local.addr= new mfeedInfo(relation);
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

5.4.4 Description of operator ~mfeed~


*/



OperatorSpec mfeedSpec(
    "string -> stream(Tuple)",
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
    mfeedValMap,
    Operator::SimpleSelect,
    mfeedTypeMap
);


int letmconsume (Word* args, Word& result,
                int message, Word& local, Supplier s, bool flob) {

    result  = qp->ResultStorage(s);
    MemoryRelType* str = (MemoryRelType*)result.addr;

    Supplier t = qp->GetSon( s, 0 );
    ListExpr le = qp->GetType(t);
    bool succeed;
    CcString* oN = (CcString*) args[1].addr;
    if(!oN->IsDefined()){
                return 0;
          }
    string res = oN->GetValue();
    if(catalog->isMMObject(res)){
        succeed = false;
        str->Set(succeed,res);
        return 0;
    };

    MemoryRelObject* mmRelObject = new MemoryRelObject();
    succeed = mmRelObject->tupleStreamToRel(args[0],
        nl->Second(le), getDBname(), flob);
    if (succeed) {
        catalog->insert(res,mmRelObject);
    } else {
        delete mmRelObject;
    }
    str->Set(succeed,res);

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
        (stream(Tuple) x string -> memoryRelType(tuple(X)))

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

    ListExpr tupeldesc = nl->Second(nl->First(args));
    ListExpr l1 = nl->SymbolAtom(MemoryRelType::BasicType());
    return nl->TwoElemList (l1,tupeldesc);;
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
    "stream(Tuple) x string -> memoryRelType(tuple(X))",
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
    "stream(Tuple) x string -> memoryRelType(tuple(X))",
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

    if (!CcString::checkType(nl->First(arg1))) {
        return listutils::typeError("string expected as first argument");
    };


    ListExpr str = nl->Second(arg1);

    if(nl->AtomType(str)!=StringType){
            return listutils::typeError("error");
    }

    string oN = nl->StringValue(str);


    if(catalog->isMMObject(oN) && catalog->isAccessible(oN)){

        ListExpr typeExpr = catalog->getMMObjectTypeExpr(oN);

        if(listutils::isTupleDescription(typeExpr)){
        ListExpr result =
            nl->TwoElemList(nl->SymbolAtom(Relation::BasicType()), typeExpr);
        return result;
        }

        if(listutils::isDATA(typeExpr)) {
          return typeExpr;
        }
    }
return listutils::typeError("string does not belong to a main memory member "
        "or object is not accessible");
}


/*

5.7.3  The Value Mapping Functions of operator ~memobject~

*/



int memobjectValMap (Word* args, Word& result,
                int message, Word& local, Supplier s) {

        CcString* oN = (CcString*) args[0].addr;
        if(!oN->IsDefined()){
            return 0;
        }
        string objectName = oN->GetValue();
        ListExpr typeExpr = catalog->getMMObjectTypeExpr(objectName);


        if (listutils::isTupleDescription(typeExpr)) {
            MemoryRelObject* memObject =
                    (MemoryRelObject*)catalog->getMMObject(objectName);
            GenericRelation* rel =
                    (GenericRelation*)((qp->ResultStorage(s)).addr);
            if(rel->GetNoTuples() > 0) {
                rel->Clear();
            }

            vector<Tuple*>* relation;
            relation = memObject->getmmrel();
            vector<Tuple*>::iterator it;
            it=relation->begin();


            while( it!=relation->end()){
                Tuple* tup = *it;
                rel->AppendTuple(tup);
                it++;
            }

            result.setAddr(rel);

            return 0;

        }

        if (listutils::isDATA(typeExpr)) {

            MemoryAttributeObject* memObject =
                    (MemoryAttributeObject*)catalog->getMMObject(objectName);
            Attribute* attr = (Attribute*)((qp->ResultStorage(s)).addr);
            attr = (memObject->getAttributeObject())->Copy();
            result.setAddr(attr);
        return 0;
        }
    return 0;
}




/*

5.7.4 Description of operator ~memobject~

*/



OperatorSpec memobjectSpec(
    "string -> m:MEMLOADABLE",
    "memobject (_)",
    "returns a persistent Object created from a main memory Object",
    "query memobject ('Trains100')"
);


/*

5.7.5 Instance of operator ~memobject~

*/

Operator memobjectOp (
    "memobject",
    memobjectSpec.getStr(),
    memobjectValMap,
    Operator::SimpleSelect,
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
        "(ObjectType string)(ObjSizeInB string)(ObjSizeInMB real)"
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
        if (listutils::isTupleDescription(objectType)){
            objTyp = MemoryRelObject::BasicType();
        }
        if (listutils::isDATA(objectType)){
            objTyp = "memoryAttributeObject";
        }
        if (nl->ToString(objectType)=="memoryRtreeObject"){
                objTyp = "memoryRtreeObject";
        }
        if (nl->ToString(objectType)=="memoryAVLObject"){
                objTyp = "memoryAVLObject";
        }
        TupleType* tt = new TupleType(nl->Second(resultType));
        Tuple *tup = new Tuple( tt );
        tt->DeleteIfAllowed();

        CcInt* totalMB = new CcInt (true, catalog->getMemSizeTotal());
        CcReal* usedMB =
            new CcReal (true, (double)catalog->getUsedMemSize()/1024.0/1024.0);
        CcString* objectName = new CcString(true,name);
        CcString* oT = new CcString(true,objTyp);
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

    CcString* oN = (CcString*) args[0].addr;
    if(!oN->IsDefined()){
        return 0;
    }
    string objectName = oN->GetValue();
    Supplier t = qp->GetSon( s, 1 );
    ListExpr le = qp->GetType(t);

    if (listutils::isRelDescription(le)){

        GenericRelation* rel= static_cast<Relation*>( args[1].addr );
        MemoryRelObject* mmRelObject = new MemoryRelObject();
        memletsucceed = mmRelObject->relToVector(rel,
        nl->Second(le), getDBname(), flob);
        if (memletsucceed) {
            catalog->insert(objectName,mmRelObject);
        } else {
            delete mmRelObject;
        }
    }

    if (listutils::isDATA(le)){

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
    }

    if (listutils::isTupleStream(le)){

        MemoryRelObject* mmRelObject = new MemoryRelObject();
            memletsucceed = mmRelObject->tupleStreamToRel(args[1],
        nl->Second(le), getDBname(), flob);
        if (memletsucceed) {
            catalog->insert(objectName,mmRelObject);
        } else {
            delete mmRelObject;
        }
    }

    b->Set(true, memletsucceed);

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

    ListExpr arg1 = nl->First(args);
    if (!CcString::checkType(nl->First(arg1))) {
        return listutils::typeError("string expected");
    }
    ListExpr str = nl->Second(arg1);
    string objectName = nl->StringValue(str);

    if (!catalog->isMMObject(objectName)){
        return listutils::typeError("identifier is not a main memory object");
    }
    ListExpr arg2 = nl->Second(args);
    ListExpr memType = catalog->getMMObjectTypeExpr(objectName);
    if (listutils::isRelDescription(nl->First(arg2)) &&
        listutils::isTupleDescription(memType) &&
        nl->Equal(nl->Second(nl->First(arg2)), memType)) {
        return listutils::basicSymbol<CcBool>();
    };
    if (listutils::isDATA(nl->First(arg2)) &&
        listutils::isDATA(memType) &&
        nl->Equal(nl->First(arg2), memType)) {
        return listutils::basicSymbol<CcBool>();
    }
    if (listutils::isTupleStream(nl->First(arg2)) &&
        listutils::isTupleDescription(memType) &&
        nl->Equal(nl->Second(nl->First(arg2)), memType)) {
        return listutils::basicSymbol<CcBool>();
    }
    return listutils::typeError ("the second argument has to "
    " be of kind DATA or a relation or a tuplestream and the type expressions "
    " or the tuple description have to match");
}


/*

5.10.3  The Value Mapping Functions of operator ~memupdate~

*/

int memupdateValMap (Word* args, Word& result,
                int message, Word& local, Supplier s) {

    result  = qp->ResultStorage(s);
    CcBool* b = static_cast<CcBool*>(result.addr);
    bool memupdatesucceed = false;

    CcString* oN = (CcString*) args[0].addr;
    if(!oN->IsDefined()){
        return 0;
    }
    string objectName = oN->GetValue();
    bool flob = catalog->getMMObject(objectName)->hasflob();

    Supplier t = qp->GetSon( s, 1 );
    ListExpr le = qp->GetType(t);


    if (listutils::isRelDescription(le)) {

        catalog->deleteObject(objectName);

        GenericRelation* rel= static_cast<Relation*>( args[1].addr );
        MemoryRelObject* mmRelObject = new MemoryRelObject();
        memupdatesucceed = mmRelObject->relToVector(rel,
        nl->Second(le), getDBname(), flob);
        if (memupdatesucceed) {
            catalog->insert(objectName,mmRelObject);
        } else {
            delete mmRelObject;
        }
    }


    if (listutils::isDATA(le)){

        catalog->deleteObject(objectName);

        Attribute* attr = (Attribute*)args[1].addr;
        MemoryAttributeObject* mmA = new MemoryAttributeObject();
         memupdatesucceed = mmA->attrToMM(attr, le,
                                getDBname(),flob);
         if (memupdatesucceed){
            catalog->insert(objectName, mmA);
         }
         else {
            delete mmA;
         }

    }


    if (listutils::isTupleStream(le)) {

        catalog->deleteObject(objectName);

        MemoryRelObject* mmRelObject = new MemoryRelObject();
            memupdatesucceed = mmRelObject->tupleStreamToRel(args[1],
        nl->Second(le), getDBname(), flob);
        if (memupdatesucceed) {
            catalog->insert(objectName,mmRelObject);
        } else {
            delete mmRelObject;
        }
    }

    b->Set(true, memupdatesucceed);

    return 0;

}


/*

5.10.4 Description of operator ~memupdate~

*/

OperatorSpec memupdateSpec(
    "string x m:MEMLOADABLE -> bool",
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
    memupdateValMap,
    Operator::SimpleSelect,
    memupdateTypeMap
);


/*
5.10 Operator ~mcreateRtree~
creates a an mmRTree over a given main memory relation

*/

/*
5.10.1 Type Mapping Functions of operator ~mcreateRtree~
        (string x string -> string ||
         memoryRelType x string -> string)
        the first parameter identifies the main memory relation, the
        second parameter identifies the attribute

*/

ListExpr mcreateRtreeTypeMap(ListExpr args){

    if(nl->ListLength(args)!=2){
        return listutils::typeError("wrong number of arguments");
    }

// Split argument in two parts
    ListExpr arg1 = nl->First(args);
    ListExpr arg2 = nl->Second(args);
    ListExpr oTE_Rel;

// first argument must be a string or a memoryRelType
    if (!CcString::checkType(nl->First(arg1)) &&
        !MemoryRelType::checkType(nl->First(arg1))){
        return listutils::typeError
        ("string or memoryRelType as first argument expected");
    }


// first argument is a string
    if (CcString::checkType(nl->First(arg1))) {
        string oN_Rel = nl->StringValue(nl->Second(arg1));
        oTE_Rel = catalog->getMMObjectTypeExpr(oN_Rel);
        if (!listutils::isTupleDescription(oTE_Rel)){
            return listutils::typeError
                ("string does not identify a MemoryRelObject");
        }
    }

// first argument is a memoryRelType
    if (MemoryRelType::checkType(nl->First(arg1))){
        oTE_Rel = nl->Second(nl->First(arg1));
    }


// second argument must be a string
    if (!CcString::checkType(nl->First(arg2))) {
        return listutils::typeError("string as second argument expected");
    }

    string attrName = nl->StringValue(nl->Second(arg2));
    ListExpr attrType = 0;
    int attrPos = 0;
    ListExpr attrList = nl->Second(oTE_Rel);

    if (listutils::isAttrList(attrList)){
        attrPos = listutils::findAttribute(attrList, attrName, attrType);
    }


    if (attrPos == 0){
        return listutils::typeError("the second string "
        " does not identify an attribute");
    }

    if (!listutils::isKind(attrType,Kind::SPATIAL2D()) &&
        !listutils::isKind(attrType,Kind::SPATIAL3D()) &&
        !listutils::isKind(attrType,Kind::SPATIAL4D()) &&
        !listutils::isKind(attrType,Kind::SPATIAL8D())){

            return listutils::typeError("Expects key attribute to "
            "be of kind SPATIAL, SPATIAL3D, SPATIAL4D, SPATIAL8D");
       }

    return nl->ThreeElemList(
                nl->SymbolAtom(Symbols::APPEND()),
                nl->TwoElemList
                (nl->IntAtom(attrPos),nl->StringAtom(nl->ToString(attrType))),
                listutils::basicSymbol<CcString>());
}

/*

5.10.3  The Value Mapping Functions of operator ~mcreateRtree~

*/

template<int dim>
bool mcreateRtree(MemoryRelObject* mmrel, int attrPos, string rtreeName){
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
        attr=(StandardSpatialAttribute<dim>*)tup->GetAttribute(attrPos-1);
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
                        "memoryRtreeObject", flob, database);

    if (usedMainMemory>availableMemSize){
        cout<<"there is not enough memory left to create the rtree";
    }
    if (usedMainMemory<=availableMemSize
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
    CcString* str = static_cast<CcString*>(result.addr);
    bool succeed = false;

    // the main memory relation
    T* roN = (T*) args[0].addr;
    if(!roN->IsDefined()){
        return 0;
    }
    string relObjectName = roN->GetValue();

    //the attribute
    CcString* attrN = (CcString*) args[1].addr;
    if(!attrN->IsDefined()){
        return 0;
    }
    string attrName = attrN->GetValue();

    // the appended value attribute Position and attribute type
    CcInt* append = (CcInt*) args[2].addr;
    int attrPos = append->GetValue();
    CcString* aT = (CcString*)args[3].addr;
    string attrType = aT->GetValue();
    ListExpr attrTypeAsList;
    nl->ReadFromString(attrType, attrTypeAsList);
    string res = relObjectName +"_"+attrName;
    MemoryRelObject* mmrel =
        (MemoryRelObject*)catalog->getMMObject(relObjectName);

    if(listutils::isKind(attrTypeAsList,Kind::SPATIAL2D())){
        succeed = mcreateRtree<2>(mmrel, attrPos, res);
    }
    if(listutils::isKind(attrTypeAsList,Kind::SPATIAL3D())){
        succeed= mcreateRtree<3>(mmrel, attrPos, res);
    }
    if(listutils::isKind(attrTypeAsList,Kind::SPATIAL4D())){
        succeed= mcreateRtree<4>(mmrel, attrPos, res);
    }
    if(listutils::isKind(attrTypeAsList,Kind::SPATIAL8D())){
        succeed=mcreateRtree<8>(mmrel, attrPos, res);
    }

    str->Set(succeed, res);
    return 0;
    } //end mcreateRtreeValMap


ValueMapping mcreateRtreeValMap[] =
{
    mcreateRtreeValMapT<CcString>,
    mcreateRtreeValMapT<MemoryRelType>,
};

int mcreateRtreeSelect(ListExpr args){
    // string case at index 0
    if ( CcString::checkType(nl->First(args)) ){
       return 0;
    }
    // MemoryRelType case at index 1
    if ( MemoryRelType::checkType(nl->First(args)) ){
       return 1;
    }
    // should never be reached
    return -1;
  }

/*

5.10.4 Description of operator ~mcreateRtree~

*/

OperatorSpec mcreateRtreeSpec(
    "string x string -> string || memoryRelType x string -> string",
    "_ mcreateRtree [_]",
    "creates an mmrtree over a main memory relation given by the"
    "first string || memoryRelType and an attribute"
    "given by the second string",
    "query 'WFlaechen' mcreateRtree ['GeoData']"
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
    // second Arg ein AttrName?
    if(nl->AtomType(nl->Second(args))!=SymbolType){
        return listutils::typeError("second argument must be an attribute");
    }
    // third arg string (name of the rtree)?
    if(!CcString::checkType(nl->Third(args))){
        return listutils::typeError("third argument must be a string");
    }
    string name = nl->SymbolValue(nl->Second(args));
    // get attributelist
    ListExpr attrList = nl->Second(nl->Second(nl->First(args)));
    ListExpr type;
    int j = listutils::findAttribute(attrList,name,type);
    if(j==0){
        return listutils::typeError("Attr not found");
    }
    if( !Rectangle<2>::checkType(type) &&
        !Rectangle<3>::checkType(type) &&
        !Rectangle<4>::checkType(type) &&
        !Rectangle<8>::checkType(type) &&
        !listutils::isKind(type,Kind::SPATIAL2D()) &&
        !listutils::isKind(type,Kind::SPATIAL3D()) &&
        !listutils::isKind(type,Kind::SPATIAL4D()) &&
        !listutils::isKind(type,Kind::SPATIAL8D()))
        return listutils::typeError("Attr is not a rect or of kind spatial");

    ListExpr tid = listutils::basicSymbol<TupleIdentifier>();
    // find a tid in the attribute list
    string tidn;
    int k = listutils::findType(attrList, tid, tidn);
    if(k==0){
        return listutils::typeError("no tid in tuple");
    }
    return nl->ThreeElemList(nl->SymbolAtom(Symbols::APPEND()),
                   nl->TwoElemList( nl->IntAtom(j-1),
                                    nl->IntAtom(k-1)),
                   listutils::basicSymbol<CcString>());
}



/*

5.10.3  The Value Mapping Functions of operator ~mcreateRtree2~

*/
template <int dim>
int mcreateRtree2ValMapT (Word* args, Word& result,
                int message, Word& local, Supplier s) {

    result  = qp->ResultStorage(s);
    CcString* str = static_cast<CcString*>(result.addr);
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
    MemoryRtreeObject<dim>* mmRtreeObject =
        new MemoryRtreeObject<dim>(rtree, usedMainMemory,
                        "memoryRtreeObject", false, "");

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
    str->Set(succeed, name);
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

    if(listutils::isKind(type,Kind::SPATIAL2D()) ||
        Rectangle<2>::checkType(type)){
        return 0;
    }
    if(listutils::isKind(type,Kind::SPATIAL3D()) ||
        Rectangle<3>::checkType(type)){
        return 1;
    }
    if(listutils::isKind(type,Kind::SPATIAL4D()) ||
        Rectangle<4>::checkType(type)){
        return 2;
    }
    if(listutils::isKind(type,Kind::SPATIAL8D()) ||
        Rectangle<8>::checkType(type)){
        return 3;
    }
  return -1;
 }
/*

5.10.4 Description of operator ~mcreateRtree2~

*/

OperatorSpec mcreateRtree2Spec(
    "stream(Tuple) x T x string -> string",
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

    ListExpr argSec = nl->Second(args); //string + query

    if (!Stream<Tuple>::checkType(stream)) {
        return listutils::typeError
            ("stream(Tuple) as first argument expected");
    }

    if(!nl->HasLength(argSec,2)){
        return listutils::typeError("internal error");
    }

    if (!CcString::checkType(nl->First(argSec))) {
        return listutils::typeError("string expected");
    };

    ListExpr fn = nl->Second(argSec);

    if(nl->AtomType(fn)!=StringType){
        return listutils::typeError("error");
    }

    string oN = nl->StringValue(fn);
    ListExpr oTeList = catalog->getMMObjectTypeExpr(oN);

    if(!catalog->isMMObject(oN) ||
            !listutils::isTupleDescription(oTeList))
    {
      return listutils::typeError("not a MainMemory member or not a relation");
    }

    if (!nl->Equal(nl->Second(stream), oTeList))
    {
      return listutils::typeError("the stream tuple description "
                "does not match the main memory relation ");
    }
  return nl->TwoElemList(nl->SymbolAtom(Symbol::STREAM()),oTeList);

}


class minsertInfo{
  public:
     minsertInfo(vector<Tuple*>* _relation):relation(_relation){
          it = relation->begin();
     }

    ~minsertInfo(){}

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


/*

5.13.3  The Value Mapping Functions of operator ~minsert~

*/

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
            CcString* oN = (CcString*) args[1].addr;
            if(!oN->IsDefined()){
                return 0;
            }
            string objectName = oN->GetValue();
            vector<Tuple*>* relation;
            MemoryRelObject* mro =
                    (MemoryRelObject*)catalog->getMMObject(objectName);

            Stream<Tuple> stream(args[0]);
            Tuple* tup;
            stream.open();
            if (mro->hasflob()){
                while( (tup = stream.request()) != 0){
                    tup->bringToMemory();
                    mro->addTuple(tup);
                }
            } else {
                while( (tup = stream.request()) != 0){
                    mro->addTuple(tup);
                }
            }
        stream.close();
        relation = mro->getmmrel();
        local.addr= new minsertInfo(relation);
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

5.13.4 Description of operator ~minsert~

*/

OperatorSpec minsertSpec(
    "stream(Tuple) x string -> stream(Tuple)",
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
    minsertValMap,
    Operator::SimpleSelect,
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
  ListExpr memoryRtreeDescription = nl->First(args);
  ListExpr memoryRelDescription = nl->Second(args);
  ListExpr searchWindow = nl->Third(args);


    if (!CcString::checkType(nl->First(memoryRtreeDescription))) {
        return listutils::typeError("string as first argument expected");
    }
    string oN_Rtree = nl->StringValue(nl->Second(memoryRtreeDescription));
    ListExpr oTE_Rtree = catalog->getMMObjectTypeExpr(oN_Rtree);

    if (!catalog->isMMObject(oN_Rtree) ||
            !(nl->ToString(oTE_Rtree)=="memoryRtreeObject")){
        return listutils::typeError
                ("first string does not identify a MemoryRTreeObject");
    }

  // second a relation
   if (!CcString::checkType(nl->First(memoryRelDescription))) {
        return listutils::typeError("string as second argument expected");
    }
   string oN_Rel = nl->StringValue(nl->Second(memoryRelDescription));
    ListExpr oTE_Rel = catalog->getMMObjectTypeExpr(oN_Rel);
    if (!catalog->isMMObject(oN_Rel) ||
                !listutils::isTupleDescription(oTE_Rel)){
        return listutils::typeError
                ("second string does not identify a MemoryRelObject");
    }
    if(!catalog->isAccessible(oN_Rel)){
        return listutils::typeError
                ("MemoryRelObject is not accessible");
    }

  // third a type with an MBR
  if(!(    listutils::isSpatialType(nl->First(searchWindow))
        || listutils::isRectangle(nl->First(searchWindow)))){
    return listutils::typeError("expects 3nd argument to be of a type of kind "
      "SPATIAL, SPATIAL3D, SPATIAL4D, SPATIAL8D; or of type 'rect', 'rect3', "
      "'rect4', or 'rect8'.");
  }

return nl->TwoElemList(nl->SymbolAtom(Symbol::STREAM()),oTE_Rel);
}

template<int dim>
class mwiInfo{
  public:
     mwiInfo(mmrtree::RtreeT<dim, size_t>* _rtree,
                vector<Tuple*>* _relation, Rectangle<dim> _searchBox)
     :relation(_relation),rtree(_rtree),searchBox(_searchBox)
     {
        rtree->findAll(searchBox,res);
        it = res.begin();
     }

    ~mwiInfo(){
        res.clear();
    }


     Tuple* next(){
     if(it==res.end()) return 0;
     Tuple* result = relation->at(*it);
     result->IncReference();
     it++;
     return result;

     }

  private:
     vector<Tuple*>* relation;
     mmrtree::RtreeT<dim, size_t>* rtree;
     Rectangle<dim> searchBox;
     set<size_t> res;
     set<size_t>::iterator it;
};

/*

5.14.3  The Value Mapping Functions of operator ~mwindowintersects~

*/
template <int dim>
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
          //first argument MemoryRtreeObject
            CcString* oN_0 = (CcString*) args[0].addr;
          if(!oN_0->IsDefined()){
             return 0;
          }
          string objectName_0 = oN_0->GetValue();
          mmrtree::RtreeT<dim, size_t>* rtree;

          MemoryRtreeObject<dim>* mrTreeO =
                    (MemoryRtreeObject<dim>*)catalog->getMMObject(objectName_0);
          rtree = mrTreeO->getrtree();

          //second argument MemoryRelObject
          CcString* oN_1 = (CcString*) args[1].addr;
          if(!oN_1->IsDefined()){
             return 0;
          }
          string objectName_1 = oN_1->GetValue();
          vector<Tuple*>* relation;

          MemoryRelObject* mro =
                    (MemoryRelObject*)catalog->getMMObject(objectName_1);
          relation = mro->getmmrel();

          //third argument Searchwindow

          StandardSpatialAttribute<dim>* attr =
                    (StandardSpatialAttribute<dim>*)args[2].addr;
          Rectangle<dim> box = attr->BoundingBox();
          local.addr= new mwiInfo<dim>(rtree,relation,box);
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

ValueMapping mwindowintersectsValMap[] =
{
    mwindowintersectsValMapT<2>,
    mwindowintersectsValMapT<3>,
    mwindowintersectsValMapT<4>,
    mwindowintersectsValMapT<8>,
};

/*
1.3 Selection method for value mapping array ~mcreateRtree~

*/
 int mwindowintersectsSelect(ListExpr args)
 {
    ListExpr attr = nl->Third(args);
    if(listutils::isKind(attr,Kind::SPATIAL2D())){
        return 0;
    }
    if(listutils::isKind(attr,Kind::SPATIAL3D())){
        return 1;
    }
    if(listutils::isKind(attr,Kind::SPATIAL4D())){
        return 2;
    }
    if(listutils::isKind(attr,Kind::SPATIAL8D())){
        return 3;
    }

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
    4,
    mwindowintersectsValMap,
    mwindowintersectsSelect,
    mwindowintersectsTypeMap
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
        memoryRelType(tuple(X) x string -> string)

        the first parameter identifies the main memory relation, the
        second parameter identifies the attribute

*/

ListExpr mcreateAVLtreeTypeMap(ListExpr args){

    if(nl->ListLength(args)!=2){
     return listutils::typeError("two arguments expected");
    }

// Split argument in two parts
    ListExpr memoryRelDescription = nl->First(args);
    ListExpr attributeDescription = nl->Second(args);
    ListExpr oTE_Rel;

    if (!CcString::checkType(nl->First(memoryRelDescription)) &&
        !MemoryRelType::checkType(nl->First(memoryRelDescription))){
        return listutils::typeError
        ("string or memoryRelType as first argument expected");
    }

// first argument is a string
    if (CcString::checkType(nl->First(memoryRelDescription))) {
        string oN_Rel = nl->StringValue(nl->Second(memoryRelDescription));
        oTE_Rel = catalog->getMMObjectTypeExpr(oN_Rel);
        if (!listutils::isTupleDescription(oTE_Rel)){
            return listutils::typeError
                ("string does not identify a MemoryRelObject");
        }
    }

// first argument is a memoryRelType
    if (MemoryRelType::checkType(nl->First(memoryRelDescription))){
        oTE_Rel = nl->Second(nl->First(memoryRelDescription));

    }

// second argument must be a string
    if (!CcString::checkType(nl->First(attributeDescription))) {
        return listutils::typeError("string as second argument expected");
    }

    string attrName = nl->StringValue(nl->Second(attributeDescription));
    ListExpr attrType = 0;
    int attrPos = 0;
    ListExpr attrList = nl->Second(oTE_Rel);

    if (listutils::isAttrList(attrList)){
        attrPos = listutils::findAttribute(attrList, attrName, attrType);
    }

    if (attrPos == 0){
        return listutils::typeError
        ("there is no attribute with the given name");
    }

    return nl->ThreeElemList(
                nl->SymbolAtom(Symbols::APPEND()),
                nl->TwoElemList
                (nl->IntAtom(attrPos),nl->StringAtom(nl->ToString(attrType))),
                listutils::basicSymbol<CcString>());
}

/*

5.16.3  The Value Mapping Functions of operator ~mcreateAVLtree~

*/
template<class T>
int mcreateAVLtreeValMapT (Word* args, Word& result,
                int message, Word& local, Supplier s) {

    result  = qp->ResultStorage(s);
    CcString* str = static_cast<CcString*>(result.addr);

// the main memory relation
   T* roN = (T*) args[0].addr;
    if(!roN->IsDefined()){
        return 0;
    }
    string relObjectName = roN->GetValue();

//the attribute
    CcString* attrN = (CcString*) args[1].addr;
    if(!attrN->IsDefined()){
        return 0;
    }
    string attrName = attrN->GetValue();

// the appended value attribute Position and attribute type
    CcInt* append = (CcInt*) args[2].addr;

    int attrPos = append->GetValue();

    CcString* aT = (CcString*)args[3].addr;
    string attrType = aT->GetValue();

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
        attr=tup->GetAttribute(attrPos-1);
        if(attr==0 || !attr->IsDefined()){
            return 0;
        }
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
            return 0;
        }
    }
    string  res = relObjectName +"_"+attrName;
    MemoryAVLObject* avlObject =
        new MemoryAVLObject(tree, usedMainMemory,
            "memoryAVLObject",attrType,flob, getDBname());
    catalog->insert(res,avlObject);

    str->Set(true, res);
    return 0;
    } //end mcreateAVLtreeValMap


ValueMapping mcreateAVLtreeValMap[] =
{
    mcreateAVLtreeValMapT<CcString>,
    mcreateAVLtreeValMapT<MemoryRelType>,
};

int mcreateAVLtreeSelect(ListExpr args){
    // string case at index 0
    if ( CcString::checkType(nl->First(args)) ){
       return 0;
    }
    // MemoryRelType case at index 1
    if ( MemoryRelType::checkType(nl->First(args)) ){
       return 1;
    }
    // should never be reached
    return -1;
  }

/*

5.16.4 Description of operator ~mcreateAVLtree~

*/

OperatorSpec mcreateAVLtreeSpec(
    "string x string -> string || memoryRelType x string -> string",
    "_ mcreateAVLtree [_]",
    "creates an AVLtree over a main memory relation given by the"
    "first string || memoryRelType and an attribute "
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


*/

/*
5.17.1 Type Mapping Functions of operator ~mexactmatch~
    string x string x key -> stream(Tuple)


*/

ListExpr mexactmatchTypeMap(ListExpr args)
{
    if(nl->ListLength(args)!=3){
        return listutils::typeError("three arguments expected");
    }

    /* Split argument in three parts */
    ListExpr memoryAVLDescription = nl->First(args);
    ListExpr memoryRelDescription = nl->Second(args);
    ListExpr keyDescription = nl->Third(args);


    if (!CcString::checkType(nl->First(memoryAVLDescription))) {
        return listutils::typeError("string as first argument expected");
    }
    string oN_AVL = nl->StringValue(nl->Second(memoryAVLDescription));
    ListExpr oTE_AVL = catalog->getMMObjectTypeExpr(oN_AVL);

    if (!catalog->isMMObject(oN_AVL) ||
    !(nl->ToString(oTE_AVL)=="memoryAVLObject")){
        return listutils::typeError
                ("first string does not identify a MemoryAVLObject");
    }

    // second a relation
    if (!CcString::checkType(nl->First(memoryRelDescription))) {
        return listutils::typeError("string as second argument expected");
    }
    string oN_Rel = nl->StringValue(nl->Second(memoryRelDescription));
    ListExpr oTE_Rel = catalog->getMMObjectTypeExpr(oN_Rel);
    if (!catalog->isMMObject(oN_Rel) ||
                !listutils::isTupleDescription(oTE_Rel)){
        return listutils::typeError
                ("second string does not identify a MemoryRelObject");
    }
    if (!catalog->isAccessible(oN_Rel)){
        return listutils::typeError
                ("MemoryRelObject is not accessible");
    }


    // third a key
    string keyTypeAttr = nl->ToString(nl->First(keyDescription));
    if(!listutils::isDATA(nl->First(keyDescription))){
    return listutils::typeError("key attribute expected");
    }

    MemoryAVLObject* avlobj =(MemoryAVLObject*)(catalog->getMMObject(oN_AVL));
    string keyTypeAVL = avlobj->getKeyType();

    if (keyTypeAttr!=keyTypeAVL){
        return listutils::typeError ("type conflict between keys");
    }

    return nl->TwoElemList(nl->SymbolAtom(Symbol::STREAM()),oTE_Rel);
}

class avlOperLI{
    public:

        avlOperLI(avltree::AVLTree< pair<Attribute*,size_t>,
           KeyComparator >* _tree,vector<Tuple*>* _relation,
           Attribute* _attr1, string _keyType)
           :relation(_relation),tree(_tree),attr1(_attr1),keyType(_keyType){
            res = true;
            if (tree->Size()!=0){
     //       tree->Print(cout);
                it = tree->tail(pair<Attribute*,size_t>(attr1,0));
            }
            attr2 = attr1;
        }

        avlOperLI(avltree::AVLTree< pair<Attribute*,size_t>,
           KeyComparator >* _tree,vector<Tuple*>* _relation, Attribute* _attr1,
           Attribute* _attr2, string _keyType)
           :relation(_relation),tree(_tree),attr1(_attr1),attr2(_attr2),
           keyType(_keyType){

            if (tree->Size()!=0){
                it = tree->tail(pair<Attribute*,size_t>(attr1,0));
            }
        }


        ~avlOperLI(){}


        Tuple* next(){
            if(it.onEnd()){
                 return 0;
            }
            hit = it.Get();

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

             if ((hit->first)->Compare(attr2) == 1){
                return 0;
            }
            if ((hit->first)->Compare(attr1) == 0 ||
                (hit->first)->Compare(attr2) == -1 ||
                (hit->first)->Compare(attr2) == 0){

                Tuple* result = relation->at(hit->second);
                result->IncReference();
                it.Next();
                return result;
            }

            return 0;
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

int mexactmatchValMap (Word* args, Word& result,
                    int message, Word& local, Supplier s) {

    avlOperLI* li = (avlOperLI*) local.addr;

    switch (message)
    {
        case OPEN: {
            if(li){
                delete li;
                local.addr=0;
            }
            //first argument MemoryAVLObject
            CcString* oN_0 = (CcString*) args[0].addr;
            if(!oN_0->IsDefined()){
                return 0;
            }
            string objectName_0 = oN_0->GetValue();
            avltree::AVLTree< pair<Attribute*,size_t>, KeyComparator >* tree;
            MemoryAVLObject* avlObject =
                    (MemoryAVLObject*)catalog->getMMObject(objectName_0);
            string keyType = avlObject->getKeyType();
            tree = avlObject->getAVLtree();

            //second argument MemoryRelObject
            CcString* oN_1 = (CcString*) args[1].addr;
            if(!oN_1->IsDefined()){
                return 0;
            }
            string objectName_1 = oN_1->GetValue();
            vector<Tuple*>* relation;
            MemoryRelObject* mro =
                    (MemoryRelObject*)catalog->getMMObject(objectName_1);
            relation = mro->getmmrel();

            // third argument key value
            Attribute* attr = (Attribute*)args[2].addr;
            local.addr= new avlOperLI(tree,relation,attr,attr,keyType);
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
    mexactmatchValMap,
    Operator::SimpleSelect,
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

    /* Split argument in three parts */
    ListExpr memoryAVLDescription = nl->First(args);
    ListExpr memoryRelDescription = nl->Second(args);
    ListExpr keyDescription1 = nl->Third(args);
    ListExpr keyDescription2 = nl->Fourth(args);


    if (!CcString::checkType(nl->First(memoryAVLDescription))) {
        return listutils::typeError("string as first argument expected");
    }
    string oN_AVL = nl->StringValue(nl->Second(memoryAVLDescription));
    ListExpr oTE_AVL = catalog->getMMObjectTypeExpr(oN_AVL);

    if (!catalog->isMMObject(oN_AVL) ||
            !(nl->ToString(oTE_AVL)=="memoryAVLObject")){
        return listutils::typeError
                ("first string does not identify a MemoryAVLObject");
    }

    // second a relation
    if (!CcString::checkType(nl->First(memoryRelDescription))) {
        return listutils::typeError("string as second argument expected");
    }
    string oN_Rel = nl->StringValue(nl->Second(memoryRelDescription));
    ListExpr oTE_Rel = catalog->getMMObjectTypeExpr(oN_Rel);
    if (!catalog->isMMObject(oN_Rel) ||
                !listutils::isTupleDescription(oTE_Rel)){
        return listutils::typeError
                ("second string does not identify a MemoryRelObject");
    }
     if (!catalog->isAccessible(oN_Rel)){
        return listutils::typeError
                ("MemoryRelObject is not accessible");
    }


    // third a key
    string keyTypeAttr1 = nl->ToString(nl->First(keyDescription1));
    string keyTypeAttr2 = nl->ToString(nl->First(keyDescription2));
    if(!listutils::isDATA(nl->First(keyDescription1)) ||
            !listutils::isDATA(nl->First(keyDescription2))){
        return listutils::typeError("key attribute expected");
    }

    MemoryAVLObject* avlobj =(MemoryAVLObject*)(catalog->getMMObject(oN_AVL));
    string keyTypeAVL = avlobj->getKeyType();

    if (keyTypeAttr1!=keyTypeAVL || (keyTypeAttr2!=keyTypeAVL)){
        return listutils::typeError ("type conflict between keys");
    }

    return nl->TwoElemList(nl->SymbolAtom(Symbol::STREAM()),oTE_Rel);
}


/*

5.18.3  The Value Mapping Functions of operator ~mrange~

*/

int mrangeValMap (Word* args, Word& result,
                    int message, Word& local, Supplier s) {

    avlOperLI* li = (avlOperLI*) local.addr;

    switch (message)
    {
        case OPEN: {
            if(li){
                delete li;
                local.addr=0;
            }
            //first argument MemoryAVLObject
            CcString* oN_0 = (CcString*) args[0].addr;
            if(!oN_0->IsDefined()){
                return 0;
            }
            string objectName_0 = oN_0->GetValue();
            avltree::AVLTree< pair<Attribute*,size_t>, KeyComparator >* tree;
            MemoryAVLObject* avlObject =
                    (MemoryAVLObject*)catalog->getMMObject(objectName_0);
            string keyType = avlObject->getKeyType();
            tree = avlObject->getAVLtree();

            //second argument MemoryRelObject
            CcString* oN_1 = (CcString*) args[1].addr;
            if(!oN_1->IsDefined()){
                return 0;
            }
            string objectName_1 = oN_1->GetValue();
            vector<Tuple*>* relation;
            MemoryRelObject* mro =
                    (MemoryRelObject*)catalog->getMMObject(objectName_1);
            relation = mro->getmmrel();

            // third argument key value
            Attribute* attr1 = (Attribute*)args[2].addr;
            Attribute* attr2 = (Attribute*)args[3].addr;
            local.addr= new avlOperLI(tree,relation,attr1, attr2, keyType);
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

5.18.4 Description of operator ~range~

*/

OperatorSpec mrangeSpec(
    "string x string x key x key -> stream(Tuple) ",
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
    mrangeValMap,
    Operator::SimpleSelect,
    mrangeTypeMap
);

/*
5.19 Operator ~matchbelow~

returns for a key X (third argument)
the tuple which contains the biggest attribute value in the 
AVLtree (first argument) which is smaller or equal X.

*/


/*
5.19.1 Type Mapping Functions of operator ~matchbelow~
    string x string x key -> stream(Tuple)


*/

ListExpr matchbelowTypeMap(ListExpr args)
{
    if(nl->ListLength(args)!=3){
        return listutils::typeError("three arguments expected");
    }

    /* Split argument in three parts */
    ListExpr memoryAVLDescription = nl->First(args);
    ListExpr memoryRelDescription = nl->Second(args);
    ListExpr keyDescription = nl->Third(args);


    if (!CcString::checkType(nl->First(memoryAVLDescription))) {
        return listutils::typeError("string as first argument expected");
    }
    string oN_AVL = nl->StringValue(nl->Second(memoryAVLDescription));
    ListExpr oTE_AVL = catalog->getMMObjectTypeExpr(oN_AVL);

    if (!catalog->isMMObject(oN_AVL) ||
        !(nl->ToString(oTE_AVL)=="memoryAVLObject")) {
        return listutils::typeError
                ("first string does not identify a MemoryAVLObject");
    }

    // second a relation
    if (!CcString::checkType(nl->First(memoryRelDescription))) {
        return listutils::typeError("string as second argument expected");
    }
    string oN_Rel = nl->StringValue(nl->Second(memoryRelDescription));
    ListExpr oTE_Rel = catalog->getMMObjectTypeExpr(oN_Rel);
    if (!catalog->isMMObject(oN_Rel) ||
                !listutils::isTupleDescription(oTE_Rel)){
        return listutils::typeError
                ("second string does not identify a MemoryRelObject");
    }

     if (!catalog->isAccessible(oN_Rel)){
        return listutils::typeError
                ("MemoryRelObject is not accessible");
    }

    // third a key
    string keyTypeAttr = nl->ToString(nl->First(keyDescription));
    if(!listutils::isDATA(nl->First(keyDescription))){
    return listutils::typeError("key attribute expected");
    }

    MemoryAVLObject* avlobj =(MemoryAVLObject*)(catalog->getMMObject(oN_AVL));
    string keyTypeAVL = avlobj->getKeyType();

    if (keyTypeAttr!=keyTypeAVL){
        return listutils::typeError ("type conflict between keys");
    }

    return nl->TwoElemList(nl->SymbolAtom(Symbol::STREAM()),oTE_Rel);
}


/*

5.19.3  The Value Mapping Functions of operator ~matchbelow~

*/

int matchbelowValMap (Word* args, Word& result,
                    int message, Word& local, Supplier s) {

    avlOperLI* li = (avlOperLI*) local.addr;

    switch (message)
    {
        case OPEN: {
            if(li){
                delete li;
                local.addr=0;
            }
            //first argument MemoryAVLObject
            CcString* oN_0 = (CcString*) args[0].addr;
            if(!oN_0->IsDefined()){
                return 0;
            }
            string objectName_0 = oN_0->GetValue();
            avltree::AVLTree< pair<Attribute*,size_t>, KeyComparator >* tree;
            MemoryAVLObject* avlObject =
                    (MemoryAVLObject*)catalog->getMMObject(objectName_0);
            string keyType = avlObject->getKeyType();
            tree = avlObject->getAVLtree();

            //second argument MemoryRelObject
            CcString* oN_1 = (CcString*) args[1].addr;
            if(!oN_1->IsDefined()){
                return 0;
            }
            string objectName_1 = oN_1->GetValue();
            vector<Tuple*>* relation;
            MemoryRelObject* mro =
                    (MemoryRelObject*)catalog->getMMObject(objectName_1);
            relation = mro->getmmrel();

            // third argument key value
            Attribute* attr = (Attribute*)args[2].addr;
            local.addr= new avlOperLI(tree,relation,attr,keyType);
            return 0;
        }

        case REQUEST:
            result.addr=(li?li->matchbelow():0);
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

/*

5.19.5 Instance of operator ~matchbelow~

*/

Operator matchbelowOp (
    "matchbelow",
    matchbelowSpec.getStr(),
    matchbelowValMap,
    Operator::SimpleSelect,
    matchbelowTypeMap
);


TypeConstructor MemoryRelTypeTC(
     MemoryRelType::BasicType(),     // name of the type in SECONDO
     MemoryRelType::Property,        // property function describing signature
     MemoryRelType::Out,  MemoryRelType::In,          // out und in functions
    0, 0,                             // SaveToList, RestoreFromList functions
    // object creation and deletion create und delete
     MemoryRelType::create, MemoryRelType::Delete,
     0, 0,        // object open, save
     MemoryRelType::Close,  MemoryRelType::Clone,      // close and clone
     MemoryRelType::Cast,                                // cast function
     MemoryRelType::SizeOfObj,      // sizeof function
     MemoryRelType::KindCheck);      // kind checking


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


/*
6 M-tree support


6.1 mcreateMtree2: Creation of an M-tree for a persistent relation

6.1.1 Type Mapping

*/

/*
4.1 distance functions for supported types

*/
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
     if( t1::checkType(type)){ return 1;}
     if( t3::checkType(type) ){ return 2;}
     if( t4::checkType(type)){ return 3; }
     if( t5::checkType(type)){ return 4;}
     if( t6::checkType(type)){ return 5;}
     if( t7::checkType(type)){ return 6;}
     if( t8::checkType(type)){ return 7;}
     if( t9::checkType(type)){ return 8;}
     return -1;
  }
}





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
  ListExpr result = nl->ThreeElemList(
                     nl->SymbolAtom(Symbols::APPEND()),
                     nl->ThreeElemList( 
                         nl->IntAtom(index1-1),
                         nl->IntAtom(index2-1),
                         nl->StringAtom(nl->ToString(type))),
                     listutils::basicSymbol<CcString>());
  if(     Point::checkType(type)
       || CcString::checkType(type)
       || CcInt::checkType(type)
       || CcReal::checkType(type)   ){
    return result;
  }
  return listutils::typeError("there is no known distance fuction for type "
                               + nl->ToString(type));
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


/*
6.2 Value Mapping template

*/
template <class T>
int mcreateMtree2VMT (Word* args, Word& result, int message,
                    Word& local, Supplier s) {

   result = qp->ResultStorage(s);
   CcString* res = (CcString*) result.addr;

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

   while( (tuple = stream.request())){
      T* attr = (T*) tuple->GetAttribute(index1);
      TupleIdentifier* tid = (TupleIdentifier*) tuple->GetAttribute(index2);
      if(tid->IsDefined()){
        T copy = *attr;
        pair<T,TupleId> p(copy, tid->GetTid());
        tree->insert(p);
      }
      tuple->DeleteIfAllowed();
   }
   stream.close();
   size_t usedMem = tree->memSize();

   MemoryMtreeObject<pair<T, TupleId>,StdDistComp<T> >* mtree = 
          new MemoryMtreeObject<pair<T,TupleId>, StdDistComp<T> > (tree,  
                             usedMem, 
                             string("(") + MEMORYMTREEOBJECT + " " + tn +")", 
                             false, "");
   bool success = catalog->insert(n, mtree);
   res->Set(success, n);
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
  "stream(tuple) x attrname x attrname x string -> string ",
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

  string err = "string x T x real expected";
  if(!nl->HasLength(args,3)){
     return listutils::typeError(err + " ( wrong number of args)");
  }
  ListExpr a1t = nl->First(nl->First(args));
  ListExpr a2t = nl->First(nl->Second(args));
  ListExpr a3t = nl->First(nl->Third(args));

  if( !CcString::checkType(a1t)){
    return  listutils::typeError(err + " (first arg is not a string)");
  } 
  if(   !CcReal::checkType(a3t)
     && !CcInt::checkType(a3t)){
    return listutils::typeError(err + " (third arg is not of type real "
                                "or int)");
  }

  ListExpr a2v = nl->Second(nl->First(args));

  if(nl->AtomType(a2v)!=StringType){
    return listutils::typeError("expected constant string as"
                                " the first argument");
  }

  string mtreename = nl->StringValue(a2v);

  if(!catalog->isMMObject(mtreename)){
     return listutils::typeError("not a memory object : " + mtreename);
  } 

  ListExpr expectedType = nl->TwoElemList(nl->SymbolAtom(MEMORYMTREEOBJECT),
                                a2t);


  ListExpr foundType = catalog->getMMObjectTypeExpr(mtreename);


  if(!nl->Equal(expectedType, foundType)){
     return listutils::typeError("name " + mtreename 
                               + " is not an m-tree over the search key");
  }  

  return nl->TwoElemList(
                listutils::basicSymbol<Stream<TupleIdentifier> >(),
                listutils::basicSymbol<TupleIdentifier> ());
}


template <class T>
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
            string name = ((CcString*) args[0].addr)->GetValue();
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
   return mtreehelper::getTypeNo(type,9);
}

 // note: if adding attributes with flobs, the value mapping must be changed

ValueMapping mdistRange2VM[] = {
  mdistRange2VMT<mtreehelper::t1>,
  mdistRange2VMT<mtreehelper::t2>,
  mdistRange2VMT<mtreehelper::t3>,
  mdistRange2VMT<mtreehelper::t4>,
  mdistRange2VMT<mtreehelper::t5>,
  mdistRange2VMT<mtreehelper::t6>,
  mdistRange2VMT<mtreehelper::t7>,
  mdistRange2VMT<mtreehelper::t8>,
  mdistRange2VMT<mtreehelper::t9>
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
   4,
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

  string err = "string x T  expected";
  if(!nl->HasLength(args,2)){
     return listutils::typeError(err + " ( wrong number of args)");
  }
  ListExpr a1t = nl->First(nl->First(args));
  ListExpr a2t = nl->First(nl->Second(args));

  if( !CcString::checkType(a1t)){
    return  listutils::typeError(err + " (first arg is not a string)");
  } 

  ListExpr a1v = nl->Second(nl->First(args));

  if(nl->AtomType(a1v)!=StringType){
    return listutils::typeError("expected constant string as"
                                " the first argument");
  }

  string mtreename = nl->StringValue(a1v);

  if(!catalog->isMMObject(mtreename)){
     return listutils::typeError("not a memory object : " + mtreename);
  } 

  ListExpr expectedType = nl->TwoElemList(nl->SymbolAtom(MEMORYMTREEOBJECT),
                                a2t);


  ListExpr foundType = catalog->getMMObjectTypeExpr(mtreename);


  if(!nl->Equal(expectedType, foundType)){
     return listutils::typeError("name " + mtreename 
                               + " is not an m-tree over the search key");
  }  

  return nl->TwoElemList(
                listutils::basicSymbol<Stream<TupleIdentifier> >(),
                listutils::basicSymbol<TupleIdentifier> ());
}


template <class T>
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
            string name = ((CcString*) args[0].addr)->GetValue();
            MemoryMtreeObject<pair<T, TupleId>,StdDistComp<T> >* mtreeo  =
               (MemoryMtreeObject<pair<T, TupleId>,StdDistComp<T> >*)
                   catalog->getMMObject(name);
            if(mtreeo){
              MMMTree<pair<T, TupleId>,StdDistComp<T> >* mtree 
                   = mtreeo->getmtree();
              if(mtree){
                  T a = *attr;
                  pair<T,TupleId> p(a,0);
                  local.addr = mtree->nnSearch(p);
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
int mdistScan2Select(ListExpr args){
   ListExpr type = nl->Second(args);
   return mtreehelper::getTypeNo(type,9);
}

 // note: if adding attributes with flobs, the value mapping must be changed

ValueMapping mdistScan2VM[] = {
  mdistScan2VMT<mtreehelper::t1>,
  mdistScan2VMT<mtreehelper::t2>,
  mdistScan2VMT<mtreehelper::t3>,
  mdistScan2VMT<mtreehelper::t4>,
  mdistScan2VMT<mtreehelper::t5>,
  mdistScan2VMT<mtreehelper::t6>,
  mdistScan2VMT<mtreehelper::t7>,
  mdistScan2VMT<mtreehelper::t8>,
  mdistScan2VMT<mtreehelper::t9>
};

OperatorSpec mdistScan2Spec(
  "string x DATA -> stream(tid) ",
  "mem_mtree mdistScan2[keyAttr] ",
  "Scans the tuple ids within an m-tree in increasing "
  "distance of the reference object to the associated "
  "objects.",
  "query \"kinos_mtree\" mdistScan2[ alexanderplatz] count"
);

Operator mdistScan2Op(
   "mdistScan2",
   mdistScan2Spec.getStr(),
   4,
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
  ListExpr a1t = nl->First(nl->First(args));
  ListExpr a2t = nl->First(nl->Second(args));
  ListExpr a3t = nl->First(nl->Third(args));

  if(     !CcString::checkType(a1t)
       || (nl->AtomType(a2t) != SymbolType)
       || !CcString::checkType(a3t)){
    return listutils::typeError(err);
  }

  ListExpr a1v = nl->Second(nl->First(args));
  ListExpr a3v = nl->Second(nl->Third(args));

  if(    (nl->AtomType(a1v)!=StringType)
      || (nl->AtomType(a3v)!=StringType)){
    return listutils::typeError("The first and the third argument "
                                "must be stding constants");
  }

  string relname = nl->StringValue(a1v);
  string treename = nl->StringValue(a3v);

  if(!catalog->isMMObject(relname)){
    return listutils::typeError(relname + " is not a memory object");
  }

  if(catalog->isMMObject(treename)){
     return listutils::typeError(treename 
                         + " already present in main memory catalog");
  }

  ListExpr type = catalog->getMMObjectTypeExpr(relname);


  if(!Tuple::checkType(type)){
     return listutils::typeError(relname + " is not of type relation");
  }

  ListExpr attrList = nl->Second(type);
  ListExpr at;
  int index = listutils::findAttribute(attrList, nl->SymbolValue(a2t), at);

  if(!index){
     return listutils::typeError(nl->ToString(a2t) + " ist noz known in tuple");
  }
  ListExpr result = nl->ThreeElemList(
                          nl->SymbolAtom(Symbols::APPEND()),
                          nl->TwoElemList( 
                                nl->IntAtom(index-1),
                                nl->StringAtom(nl->ToString(at))
                          ),
                          listutils::basicSymbol<CcString>());



  if(     Point::checkType(at)
       || CcString::checkType(at)
       || CcInt::checkType(at)
       || CcReal::checkType(at)   ){
    return result;
  }

  return listutils::typeError("Type " + nl->ToString(at) 
                              + " not supported by m-tree");
}



template <class T>
int mcreateMtreeVMT (Word* args, Word& result, int message,
                    Word& local, Supplier s) {

   result = qp->ResultStorage(s);
   CcString* res = (CcString*) result.addr;

   string relName = ((CcString*) args[0].addr)->GetValue();

   CcString* name = (CcString*) args[2].addr; 
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

   int index = ((CcInt*) args[3].addr)->GetValue(); 
   string tn = ((CcString*) args[4].addr)->GetValue();

   StdDistComp<T> dc;
   MMMTree<pair<T,TupleId>,StdDistComp<T> >* tree = 
           new MMMTree<pair<T,TupleId>,StdDistComp<T> >(4,8,dc);

   MemoryRelObject* mrel = (MemoryRelObject*) catalog->getMMObject(relName);

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
                             string("(") + MEMORYMTREEOBJECT +" " + tn +")",
                             false, "");
   bool success = catalog->insert(n, mtree);
   res->Set(success, n);
   return 0;
}


int mcreateMtreeVM (Word* args, Word& result, int message,
                    Word& local, Supplier s) {

  string tn = ((CcString*) args[4].addr)->GetValue();
  ListExpr tnl;
  if(!nl->ReadFromString(tn,tnl)){
     assert(false);
  }
  if(Point::checkType(tnl)) {
       return mcreateMtreeVMT<Point>(args, result, message, local, s);
  }
  if(CcString::checkType(tnl)) {
       return mcreateMtreeVMT<CcString>(args, result, message, local, s);
  }
  if(CcInt::checkType(tnl)) {
       return mcreateMtreeVMT<CcInt>(args, result, message, local, s);
  }
  if(CcReal::checkType(tnl)) {
       return mcreateMtreeVMT<CcReal>(args, result, message, local, s);
  }
  assert(false);
  return -1;

}

OperatorSpec mcreateMtreeSpec(
  "string x attrname x string -> string ",
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
   string err="string x string x DATA x real expected";
   if(!nl->HasLength(args,4)){
      return listutils::typeError(err + " (wrong number of args)");
   }
   ListExpr a1t = nl->First(nl->First(args));
   ListExpr a2t = nl->First(nl->Second(args));
   ListExpr a3t = nl->First(nl->Third(args));
   ListExpr a4t = nl->First(nl->Fourth(args));

   if(   !CcString::checkType(a1t)
      || !CcString::checkType(a2t)
      || !Attribute::checkType(a3t)
      || !CcReal::checkType(a4t)){
     return listutils::typeError(err);
   }

   ListExpr a1v = nl->Second(nl->First(args));
   ListExpr a2v = nl->Second(nl->Second(args));

   if(    (nl->AtomType(a1v) != StringType)
       || (nl->AtomType(a2v) != StringType)){
     return listutils::typeError("first both arguments must "
                                  "be constant strings");
   }

   string a1 = nl->StringValue(a1v);
   string a2 = nl->StringValue(a2v);

   if(!catalog->isMMObject(a1)){
      return listutils::typeError(a1 + " is not a memory object");
   }

   if(!catalog->isMMObject(a2)){
      return listutils::typeError(a2 + " is not a memory object");
   }

   if(!Tuple::checkType(catalog->getMMObjectTypeExpr(a2))){
     return listutils::typeError( a2 + " is not a memory relation");
   }
   ListExpr treeType = nl->TwoElemList( nl->SymbolAtom(MEMORYMTREEOBJECT),
                                        a3t);


   if(!nl->Equal(catalog->getMMObjectTypeExpr(a1), treeType)){
      return listutils::typeError("first arg is not an mtree over " 
                                  + nl->ToString(a3t));
   }
   
   return nl->TwoElemList(
                 listutils::basicSymbol<Stream<Tuple> >(),
                 catalog->getMMObjectTypeExpr(a2)); 
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

template<class T>
int mdistRangeVMT (Word* args, Word& result, int message,
                    Word& local, Supplier s) {
   distRangeInfo<T>* li = (distRangeInfo<T>*) local.addr;
   switch(message){
     case OPEN : {
               if(li){
                 delete li;
                 local.addr = 0;
               }
               string mt = ((CcString*) args[0].addr)->GetValue();
               string reln =((CcString*) args[1].addr)->GetValue();
               T* key = (T*) args[2].addr;
               CcReal* dist = (CcReal*) args[3].addr;
               if(dist->IsDefined()){
                  double d = dist->GetValue();
                  MemoryMtreeObject<pair<T,TupleId>, StdDistComp<T> >* m =
                       (MemoryMtreeObject<pair<T,TupleId>, StdDistComp<T> >*) 
                         catalog->getMMObject(mt);
                  MemoryRelObject* rel = (MemoryRelObject*) 
                         catalog->getMMObject(reln);
                  if(m && rel){
                     local.addr = new distRangeInfo<T>(m,rel,key,d);
                  }
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
   ListExpr type = nl->Third(args);
   return mtreehelper::getTypeNo(type,9);
}

 // note: if adding attributes with flobs, the value mapping must be changed

ValueMapping mdistRangeVM[] = {
  mdistRangeVMT<mtreehelper::t1>,
  mdistRangeVMT<mtreehelper::t2>,
  mdistRangeVMT<mtreehelper::t3>,
  mdistRangeVMT<mtreehelper::t4>,
  mdistRangeVMT<mtreehelper::t5>,
  mdistRangeVMT<mtreehelper::t6>,
  mdistRangeVMT<mtreehelper::t7>,
  mdistRangeVMT<mtreehelper::t8>,
  mdistRangeVMT<mtreehelper::t9>,
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
   4,
   mdistRangeVM,
   mdistRangeSelect,
   mdistRangeTM
);


/*
Operator mdistScan

*/
ListExpr mdistScanTM(ListExpr args){
   string err="string x string x DATA expected";
   if(!nl->HasLength(args,3)){
      return listutils::typeError(err + " (wrong number of args)");
   }
   ListExpr a1t = nl->First(nl->First(args));
   ListExpr a2t = nl->First(nl->Second(args));
   ListExpr a3t = nl->First(nl->Third(args));

   if(   !CcString::checkType(a1t)
      || !CcString::checkType(a2t)
      || !Attribute::checkType(a3t)) {
     return listutils::typeError(err);
   }

   ListExpr a1v = nl->Second(nl->First(args));
   ListExpr a2v = nl->Second(nl->Second(args));

   if(    (nl->AtomType(a1v) != StringType)
       || (nl->AtomType(a2v) != StringType)){
     return listutils::typeError("first both arguments must "
                                  "be constant strings");
   }

   string a1 = nl->StringValue(a1v);
   string a2 = nl->StringValue(a2v);

   if(!catalog->isMMObject(a1)){
      return listutils::typeError(a1 + " is not a memory object");
   }

   if(!catalog->isMMObject(a2)){
      return listutils::typeError(a2 + " is not a memory object");
   }

   if(!Tuple::checkType(catalog->getMMObjectTypeExpr(a2))){
     return listutils::typeError( a2 + " is not a memory relation");
   }
   ListExpr treeType = nl->TwoElemList( nl->SymbolAtom(MEMORYMTREEOBJECT),
                                        a3t);


   if(!nl->Equal(catalog->getMMObjectTypeExpr(a1), treeType)){
      return listutils::typeError("first arg is not an mtree over " 
                                  + nl->ToString(a3t));
   }
   
   return nl->TwoElemList(
                 listutils::basicSymbol<Stream<Tuple> >(),
                 catalog->getMMObjectTypeExpr(a2)); 
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

template<class T>
int mdistScanVMT (Word* args, Word& result, int message,
                    Word& local, Supplier s) {
   distScanInfo<T>* li = (distScanInfo<T>*) local.addr;
   switch(message){
     case OPEN : {
               if(li){
                 delete li;
                 local.addr = 0;
               }
               string mt = ((CcString*) args[0].addr)->GetValue();
               string reln =((CcString*) args[1].addr)->GetValue();
               T* key = (T*) args[2].addr;
               MemoryMtreeObject<pair<T,TupleId>, StdDistComp<T> >* m =
                       (MemoryMtreeObject<pair<T,TupleId>, StdDistComp<T> >*) 
                        catalog->getMMObject(mt);
               MemoryRelObject* rel = (MemoryRelObject*) 
                        catalog->getMMObject(reln);
               if(m && rel){
                     local.addr = new distScanInfo<T>(m,rel,key);
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
   return mtreehelper::getTypeNo(type,9);
}

 // note: if adding attributes with flobs, the value mapping must be changed

ValueMapping mdistScanVM[] = {
  mdistScanVMT<mtreehelper::t1>,
  mdistScanVMT<mtreehelper::t2>,
  mdistScanVMT<mtreehelper::t3>,
  mdistScanVMT<mtreehelper::t4>,
  mdistScanVMT<mtreehelper::t5>,
  mdistScanVMT<mtreehelper::t6>,
  mdistScanVMT<mtreehelper::t7>,
  mdistScanVMT<mtreehelper::t8>,
  mdistScanVMT<mtreehelper::t9>
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
   4,
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
        AddTypeConstructor (&MemoryRelTypeTC);
        MemoryRelObjectTC.AssociateKind( Kind::SIMPLE() );

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




