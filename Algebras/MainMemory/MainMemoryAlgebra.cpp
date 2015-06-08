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

using namespace std;
extern NestedList* nl;
extern QueryProcessor *qp;
// extern AlgebraManager *am;

namespace mmalgebra {


    MemCatalog catalog;


/*

4 Auxiliary functions

4.1 ~tmStringBool~

Function checks the type mapping string-> bool.
It returns a list expression for the result type,
otherwise the symbol ~typeerror~.

*/

ListExpr tmStringBool(ListExpr args) {
string err = "string expected";
  if(nl->ListLength(args)!=1){
     return listutils::typeError(err + " (wrong number of arguments)");
  }
  if (!CcString::checkType(nl->First(args))) {
  return listutils::typeError(err);
    }
  return listutils::basicSymbol<CcBool>();


}
/*

4.2 ~tmStringStringString~

Function checks the type mapping string x string -> string.
It returns a list expression for the result type,
otherwise the symbol ~typeerror~.

*/
ListExpr tmStringStringString (ListExpr args) {

string err = "string expected";
  if(nl->ListLength(args)!=2){
     return listutils::typeError(err + " (wrong number of arguments)");
  }

  if (!CcString::checkType(nl->First(args))
            ||(!CcString::checkType(nl->Second(args)))) {
      return listutils::typeError(err);
  }
  return listutils::basicSymbol<CcString>();

}


ListExpr tmStringMemloadBool (ListExpr args){

    if(nl->ListLength(args)!=2){
        return listutils::typeError("wrong number of arguments");
    }
    if (!CcString::checkType(nl->First(args))) {
        return listutils::typeError("string expected as first argument");
    };

    if (listutils::isRelDescription(nl->Second(args))) {
        return listutils::basicSymbol<CcBool>();
    };

    if (listutils::isDATA(nl->Second(args))) {
        return listutils::basicSymbol<CcBool>();
    }
    if (listutils::isTupleStream(nl->Second(args))){
        return listutils::basicSymbol<CcBool>();
    }

    return listutils::typeError ("the second argument has to "
    "be of kind DATA or a relation or a tuplestream");

}

/*

4.3 ~isMMObject~

Function that checks if there is a main memory object with the name of the given string

*/
bool isMMObject(string objectName){
    if (catalog.memContents.find(objectName)==catalog.memContents.end()){
    return false;
    }
    return true;
}


/*

4.4 ~getMMObject~

Function returns a pointer to the memory object
*Precondition*: "isMMObject( objectName ) == true"

*/


MemoryObject* getMMObject(string objectName){

    map<string,MemoryObject*>::iterator it;
    it = catalog.memContents.find(objectName);
    return it->second;
}

/*

4.5 ~getMMObjectTypeExpr~

Function that return the typeExpression in nested List format.
Parameter ist the object name.


*/

ListExpr getMMObjectTypeExpr(string oN){

    if (!isMMObject(oN)){
        return listutils::typeError("not a MainMemory member");
    }
    string typeExprString="";
    ListExpr typeExprList=0;
    MemoryObject* object = getMMObject(oN);
    typeExprString = object->getObjectTypeExpr();
    if (nl->ReadFromString(typeExprString, typeExprList)){
        return typeExprList;
    };
    return listutils::typeError();
};

/*

4.6 ~swap~


*/

void swap() {
    cout<< "Der HS reicht nicht aus, Laden der Rel in den HS wird abg.!"<<endl;
}


/*

4.7 ~relToVector~

Function to fill a ~vector<tuple>~ with the tuples of a given relation

*/

MemoryRelObject* relToVector(GenericRelation* r, ListExpr le = 0) {

        GenericRelationIterator* rit;
        rit = r->MakeScan();
        Tuple* tup;
        int tupleSize=0;

//die Gesamtspeichergroesse in MB umgerechnet in Byte - usedMemsize
        size_t availableMemSize =
                    (catalog.memSizeTotal*1024*1024)-catalog.usedMemSize;
        size_t usedMainMemory=0;
        bool extStorage=false;
        size_t extStorageSize=0;

        vector<Tuple*>* mmrel = new vector<Tuple*>();

        while ((tup = rit->GetNextTuple()) != 0)
            {
                tupleSize = tup->GetSize();
                if ((size_t)tupleSize<availableMemSize){
                        mmrel->push_back(tup);
                       // tup->IncReference(); ???
                        usedMainMemory += tupleSize;
                        availableMemSize -= tupleSize;
                }
                else{

                    swap();
                    extStorage=true;
                    extStorageSize=1;
                break;

                // nur solange swap() nicht richtig implementiert ist!
                }
            }

        //das eigentliche HS-objekt wird angelegt und mit Werten gefüllt
        MemoryRelObject* mmRelObject = new MemoryRelObject();
        mmRelObject->setmmrel(mmrel);
        mmRelObject->setmmrelDiskpart(0);
        mmRelObject->setExtStorage(extStorage);
        mmRelObject->setMemSize(usedMainMemory);
        mmRelObject->setExtStorageSize(extStorageSize);
        mmRelObject->
            setObjectTypeExpr(nl->ToString(le));

    return mmRelObject;

}

MemoryAttributeObject* attrToMM(Attribute* attr, ListExpr le){
        // wird noch nicht verwendet, ich brauche erst die Attributgröße
        //size_t availableMemSize =
          //          (catalog.memSizeTotal*1024*1024)-catalog.usedMemSize;
        size_t usedMainMemory=0;
        bool extStorage=false;
        size_t extStorageSize=0;

usedMainMemory = 25; //nur damit irgendwas steht

        MemoryAttributeObject* mmA = new MemoryAttributeObject();
        mmA->setAttributeObject(attr);
        mmA->setExtStorage(extStorage);
        mmA->setMemSize(usedMainMemory);
        mmA->setExtStorageSize(extStorageSize);
        mmA->setObjectTypeExpr(nl->ToString(le));

    return mmA;

}


MemoryRelObject* tupelStreamToRel(Word arg, ListExpr le){

 vector<Tuple*>* mmrel = new vector<Tuple*>();

    Stream<Tuple> stream(arg);
    size_t availableMemSize =
                (catalog.memSizeTotal*1024*1024)-catalog.usedMemSize;
    size_t usedMainMemory =0;
    bool extStorage=false;
    size_t extStorageSize=0;

    stream.open();

    Tuple* tup;
    int tupleSize = 0;
    while( (tup = stream.request()) != 0){
        tupleSize = tup->GetSize();
            if ((size_t)tupleSize<availableMemSize){
                mmrel->push_back(tup);
                usedMainMemory += tupleSize;
                availableMemSize -= tupleSize;
                }
            else{
                swap();
                extStorage=true;
                extStorageSize=1;
                //break nur solange swap() nicht richtig implementiert ist!
                break;
                }
            }

    MemoryRelObject* mem = new MemoryRelObject();
    mem->setmmrel(mmrel);
        mem->setmmrelDiskpart(0);
        mem->setExtStorage(extStorage);
        mem->setMemSize(usedMainMemory);
        mem->setExtStorageSize(extStorageSize);
        mem->setObjectTypeExpr(nl->ToString(le));


    stream.close();

return mem;

}

/*

5 Creating Operators

5.1 Operator ~memload~

load a persistent relation into main memory

5.1.1 Type Mapping Functions of operator ~memload~

A type mapping function checks whether the correct argument types are supplied
for an operator; if so, it returns a list expression for the result type,
otherwise the symbol ~typeerror~.

*/
ListExpr memloadTypeMap(ListExpr args)
{
 return tmStringBool(args);
}


/*
5.1.2 Selection Function of operator ~memload~

A selection function is quite similar to a type mapping function. The only
difference is that it doesn't return a type but the index of a value
mapping function being able to deal with the respective combination of
input parameter types.

Note that a selection function does not need to check the correctness of
argument types; this has already been checked by the type mapping function.
A selection function is only called if the type mapping was successful. This
makes programming easier as one can rely on a correct structure of the list
~args~.

*/

/*
5.1.3  The Value Mapping Functions of operator ~memload~



*/

int memloadValMap (Word* args, Word& result,
                    int message, Word& local, Supplier s) {


    //Prüfung ob der der Operation übergebene
    //string zu einem Objekt im Secondokatalog gehört
    CcString* oN = (CcString*) args[0].addr;
          if(!oN->IsDefined()){
             return 0;
          }
    string objectName = oN->GetValue();
    SecondoCatalog* cat = SecondoSystem::GetCatalog();
    // wird verwendet festzuhalten, ob das Laden in den HS erfolgreich war
    bool memloadSucceeded=false;
    // wird verwendet um das DB-Objekt darin zu speichern
    Word object;
    // objectTypeExpr des Objekts
    ListExpr objectTypeExpr=0;
    //wird von der Funktion cat->GetObjectTypeExpr verwendet
    //und speichert darin den objectTypeName (meistens leer???)
    string objectTypeName="";
    //die Gesamtspeichergroesse in MB umgerechnet in Byte - usedMemsize
    size_t availableMemSize =
                    (catalog.memSizeTotal*1024*1024)-catalog.usedMemSize;
    // der verbrauchte Speicherplatz für das zu ladende Objekt - in Byte
    size_t usedMainMemory=0;
    bool extStorage=false;
    size_t extStorageSize=0;

    // das Laden in den HS findet nur statt, wenn der string zu einem Objekt
    // des Systemkatalogs gehört und das Objekt noch nicht im HS steht
    if (!cat->IsObjectName(objectName)){
        cout<<"Der Name gehört zu keinem persistenten Datenbankobjekt"<<endl;
        }
    if (isMMObject(objectName)){
        memloadSucceeded =true;
        cout<<"Das Objekt ist schon in den Hauptspeicher geladen"<<endl;
    }
    if (cat->IsObjectName(objectName) && (!isMMObject(objectName))){
        bool defined = false;
        bool hasTypeName = false;
        memloadSucceeded = cat->GetObjectExpr(objectName,
                objectTypeName,objectTypeExpr,object,defined,hasTypeName);


//Das übergebene Object ist eine Relation
    if (Relation::checkType(objectTypeExpr)&&defined){
        GenericRelation* r= static_cast<Relation*>( object.addr );
        GenericRelationIterator* rit;
        rit = r->MakeScan();
        Tuple* tup;
        int tupleSize=0;

        vector<Tuple*>* mmrel = new vector<Tuple*>();

        while ((tup = rit->GetNextTuple()) != 0)
            {
                //bekomme ich so die richtige Tupelgröße??
                tupleSize = tup->GetSize();
                if ((size_t)tupleSize<availableMemSize){
                        mmrel->push_back(tup);
                        usedMainMemory += tupleSize;
                        availableMemSize -= tupleSize;
                }
                else{

                    swap();
                    extStorage=true;
                    //nur so zum ausprobieren
                    extStorageSize=1;
                break;

                // nur solange swap() nicht richtig implementiert ist!
                }
            }

        //das eigentliche HS-objekt wird angelegt und mit Werten gefüllt
        MemoryRelObject* mmRelObject = new MemoryRelObject();
        mmRelObject->setmmrel(mmrel);
        mmRelObject->setmmrelDiskpart(0);
        mmRelObject->setExtStorage(extStorage);
        mmRelObject->setMemSize(usedMainMemory);
        mmRelObject->setExtStorageSize(extStorageSize);
        mmRelObject->
            setObjectTypeExpr(nl->ToString(nl->Second(objectTypeExpr)));

        //der Hauptspeicherkatalog wird aktualisiert
        catalog.memContents[objectName] = mmRelObject;
        catalog.setUsedMemSize(catalog.getUsedMemSize() + usedMainMemory);


        //die nicht mehr benötigten Variablen werden gelöscht
        // ---  was muss hier gelöscht werden????

        //delete rit;
        //delete r;
        //delete tup;

    } //Ende Objekt ist eine Relation

// das Objekt ist ein Attribute
    if (Attribute::checkType(objectTypeExpr)&&defined){

        //Objektgröße???, überprüfung ob genug HS-Platz etc...
        usedMainMemory = 25;

        cout<<"mit objectTypeExpr: "<<nl->ToString(objectTypeExpr)<<endl;
        cout<<"Wert von objectName: "<<objectName<<endl;
        cout<<"Wert von objectTypeName: "<<objectTypeName<<endl;
        Attribute* attr = (Attribute*)object.addr;

        MemoryAttributeObject* mmA = new MemoryAttributeObject();
        mmA->setAttributeObject(attr);
        mmA->setExtStorage(extStorage);
        mmA->setMemSize(usedMainMemory);
        mmA->setExtStorageSize(extStorageSize);
        mmA->setObjectTypeExpr(nl->ToString(objectTypeExpr));

        //der Hauptspeicherkatalog wird aktualisiert
        catalog.memContents[objectName] = mmA;
        catalog.setUsedMemSize(catalog.getUsedMemSize() + usedMainMemory);
        }



 // das Objekt ist weder ein Attributetyp noch eine Relation
    if (!Attribute::checkType(objectTypeExpr)
            &&!Relation::checkType(objectTypeExpr)){

        memloadSucceeded = false;
        cout<<"das Objekt ist weder eine Relation noch ein Attribut"<<endl;

    }



}  // Ende  (cat->IsObjectName(objectName) && (!isMMObject(objectName)))



    //ab hier nichts ändern ausser zweiten Parameter von Set(_, ..)

    result  = qp->ResultStorage(s);
    CcBool* b = static_cast<CcBool*>(result.addr);
    //the first argument says the boolean value is defined,
    //the second is the real boolean value
    b->Set(true, memloadSucceeded);


    return 0;   //bei nicht Stromoperatoren stets 0
}


/*

5.1.4 Description of operator ~memload~

Similar to the ~property~ function of a type constructor, an operator needs to
be described, e.g. for the ~list operators~ command.  This is now done by
creating a subclass of class ~OperatorInfo~.

*/

OperatorSpec memloadSpec(
    "string -> bool",
    "memload(_)",
    "lädt ein persistentes Objekt in den Hauptspeicher",
    "query memload('plz')"
);

/*

5.1.5 Instance of operator ~memload~

*/

Operator memloadOp (
    "memload",
    memloadSpec.getStr(),
    memloadValMap,
    Operator::SimpleSelect, //nicht ueberladener Operator
    memloadTypeMap
);

/*
5.2 Operator ~meminit~

Initialisert den zu verwaltenden Hauptspeicherbereich.
Es wird die Größe des gewünschten Hauptspeicherbereichs in MB übergeben
und zurück erhält man die tatsächlich genehmigte
bzw. eingerichtete Hauptscheichergröße in MB.
Der default Wert ohne Initialisierung ist 256MB

*/

/*
5.2.1 Type Mapping Functions of operator ~meminit~

A type mapping function checks whether the correct argument types are supplied
for an operator; if so, it returns a list expression for the result type,
otherwise the symbol ~typeerror~. (int->int)

*/
ListExpr meminitTypeMap(ListExpr args)
{
string err = "int expected";
  if(nl->ListLength(args)!=1){
     return listutils::typeError(err + " (wrong number of arguments)");
  }
  if (!CcInt::checkType(nl->First(args))) {
  return listutils::typeError(err);
  }
  return listutils::basicSymbol<CcInt>();

}


/*

5.2.3  The Value Mapping Functions of operator ~meminit~

*/

int meminitValMap (Word* args, Word& result,
                    int message, Word& local, Supplier s) {

    int maxSystemMainMemory = qp->GetMemorySize(s);
    int newMainMemorySize = ((CcInt*)args[0].addr)->GetIntval();
    int res=0;

    if (newMainMemorySize<0){
    cout<< "die Größe des HS-bereichs darf nicht negativ sein:"<<endl;
    res=catalog.getMemSizeTotal();
    }
    else if ((size_t)newMainMemorySize<catalog.getUsedMemSize()/1024/1024){
            res = catalog.getUsedMemSize()/1024/1024;
            catalog.setMemSizeTotal(catalog.getUsedMemSize()/1024/1024);
        }
    else if (newMainMemorySize>maxSystemMainMemory){
            res = maxSystemMainMemory;
            catalog.setMemSizeTotal(maxSystemMainMemory);
    }
    else {
        res = newMainMemorySize;
        catalog.setMemSizeTotal(newMainMemorySize);
    }

    //nur zum Test bis memgetcatalog implementiert ist Katalogausgabe


     cout <<"memcatolog beinhaltet: " <<endl;
     map<string, MemoryObject*>::iterator itera = catalog.memContents.begin();
     while (itera!=catalog.memContents.end())
          {
            cout<< "memcatalog[" <<itera->first << "]: ",
            cout<<itera->second<<endl;
            ++itera;
          }
    cout <<"benutzterHauptspeicher: "<<catalog.getUsedMemSize()<<endl;
    cout <<"gesamterHauptspeicher: "<<catalog.getMemSizeTotal()<<endl;



    result  = qp->ResultStorage(s);
    CcInt* b = static_cast<CcInt*>(result.addr);
    b->Set(true, res);


    return 0;   //bei nicht Stromoperatoren stets 0
}


/*

5.2.4 Description of operator ~meminit~

Similar to the ~property~ function of a type constructor, an operator needs to
be described, e.g. for the ~list operators~ command.  This is now done by
creating a subclass of class ~OperatorInfo~.

*/



OperatorSpec meminitSpec(
    "int -> int",
    "meminit(_)",
    "initialisiert den von der MainMemoraAlgebra benutzten Hauptspeicher",
    "query memload(256)"
);

/*

5.2.5 Instance of operator ~meminit~

*/

Operator meminitOp (
    "meminit",
    meminitSpec.getStr(),
    meminitValMap,
    Operator::SimpleSelect,
    meminitTypeMap
);


/*
5.3 Operator ~mfeed~

Analog zum Operator ~feed~ wandelt ~mfeed~
eine Hauptspeicherrelation in einen Strom von Tupeln um.
Dafuer wird ihm der Name der Relation als String uebergeben.

*/

/*
5.3.1 Type Mapping Functions of operator ~mfeed~

A type mapping function checks whether the correct argument types are supplied
for an operator; if so, it returns a list expression for the result type,
otherwise the symbol ~typeerror~. (string -> stream(tuple))

*/

ListExpr mfeedTypeMap(ListExpr args) {

    // mfeedOp.SetUsesArgsInTypeMapping() benutzt;
    // es wird auch die query mit übertragen...nochmal genau

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


    if(!isMMObject(oN) ||
            !listutils::isTupleDescription(getMMObjectTypeExpr(oN)))
    {
      return listutils::typeError("not a MainMemory member or not a relation");
    }

    ListExpr oTeList = getMMObjectTypeExpr(oN);

    return nl->TwoElemList(nl->SymbolAtom(Symbol::STREAM()),oTeList);
}


/*
s
5.3.3  The Value Mapping Functions of operator ~mfeed~

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
          map<string,MemoryObject*>::iterator it;
          it = catalog.memContents.find(objectName);
          if(it==catalog.memContents.end()){
             return 0;
          }

          MemoryRelObject* mro = (MemoryRelObject*)it->second;
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

    return 0;   //bei nicht Stromoperatoren stets 0
}




/*

5.3.4 Description of operator ~mfeed~

Similar to the ~property~ function of a type constructor, an operator needs to
be described, e.g. for the ~list operators~ command.  This is now done by
creating a subclass of class ~OperatorInfo~.

*/



OperatorSpec mfeedSpec(
    "string -> stream(tuple)",
    "_ mfeed",
    "produces a stream from a main memory relation, when the name is given",
    "query 'ten' mfeed"
);

/*

5.3.5 Instance of operator ~mfeed~

*/

Operator mfeedOp (
    "mfeed",
    mfeedSpec.getStr(),
    mfeedValMap,
    Operator::SimpleSelect,
    mfeedTypeMap
);




/*
5.4 Operator ~letmconsume~


*/

/*
5.4.1 Type Mapping Functions of operator ~letmconsume~

A type mapping function checks whether the correct argument types are supplied
for an operator; if so, it returns a list expression for the result type,
otherwise the symbol ~typeerror~.

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
    return listutils::basicSymbol<CcString>();
}


/*

5.4.3  The Value Mapping Functions of operator ~letmconsume~

*/



int letmconsumeValMap (Word* args, Word& result,
                int message, Word& local, Supplier s) {

    //hier greife ich auf den ersten Sohn zu, enthält TypeExpression vom Stream
    Supplier t = qp->GetSon( s, 0 );
    ListExpr le = qp->GetType(t);
    vector<Tuple*>* mmrel = new vector<Tuple*>();
   // MemoryRelObject* mem = new MemoryRelObject();
    Stream<Tuple> stream(args[0]);
    size_t availableMemSize =
                (catalog.memSizeTotal*1024*1024)-catalog.usedMemSize;
    size_t usedMainMemory =0;
    bool extStorage=false;
    size_t extStorageSize=0;
    stream.open();

    Tuple* tup;
    int tupleSize = 0;
    while( (tup = stream.request()) != 0){
        tupleSize = tup->GetSize();
            if ((size_t)tupleSize<availableMemSize){
                mmrel->push_back(tup);
                usedMainMemory += tupleSize;
                availableMemSize -= tupleSize;
                }
            else{
                swap();
                extStorage=true;
                //nur so zum ausprobieren
                extStorageSize=1;
                //break nur solange swap() nicht richtig implementiert ist!
                break;
                }
            }

    MemoryRelObject* mem = new MemoryRelObject();
    mem->setmmrel(mmrel);
        mem->setmmrelDiskpart(0);
        mem->setExtStorage(extStorage);
        mem->setMemSize(usedMainMemory);
        mem->setExtStorageSize(extStorageSize);
        mem->setObjectTypeExpr(nl->ToString(le));


    stream.close();

    CcString* oN = (CcString*) args[1].addr;
    if(!oN->IsDefined()){
                return 0;
          }
    string res = oN->GetValue();

    //der Katalog wird aktualisiert
    catalog.memContents[res] = mem;
    catalog.setUsedMemSize(catalog.getUsedMemSize() + usedMainMemory);



    result  = qp->ResultStorage(s);
    CcString* str = static_cast<CcString*>(result.addr);
    str->Set(true, res);
    return 0;   //bei nicht Stromoperatoren stets 0
}




/*

5.4.4 Description of operator ~letmconsume~

Similar to the ~property~ function of a type constructor, an operator needs to
be described, e.g. for the ~list operators~ command.  This is now done by
creating a subclass of class ~OperatorInfo~.

*/



OperatorSpec letmconsumeSpec(
    "stream(tuple) x string -> string",
    "(_) mconsume [_]",
    "produces a main memory relation from a stream(tuple)",
    "query ... mconsume ['Trains100']"
);



/*

5.4.5 Instance of operator ~letmconsume~

*/

Operator letmconsumeOp (
    "letmconsume",
    letmconsumeSpec.getStr(),
    letmconsumeValMap,
    Operator::SimpleSelect,
    letmconsumeTypeMap
);





/*
5.6 Operator ~memdelete~


*/

/*
5.6.1 Type Mapping Functions of operator ~memdelete~

*/
ListExpr memdeleteTypeMap(ListExpr args)
{
   return tmStringBool(args);
}


/*

5.6.3  The Value Mapping Functions of operator ~memdelete~

*/



int memdeleteValMap (Word* args, Word& result,
                int message, Word& local, Supplier s) {

    bool deletesucceed = false;
    CcString* oN = (CcString*) args[0].addr;
    if(!oN->IsDefined()){
            result  = qp->ResultStorage(s);
            CcBool* b = static_cast<CcBool*>(result.addr);
            b->Set(true, deletesucceed);
            return 0;
        }
    string objectName = oN->GetValue();

    if(isMMObject(objectName)){

        MemoryObject* mem = getMMObject(objectName);
        catalog.setUsedMemSize(catalog.getUsedMemSize()-mem->getMemSize());

        delete mem;
        deletesucceed=true;
        mem=0;
        catalog.memContents.erase(objectName);

    }

    result  = qp->ResultStorage(s);
    CcBool* b = static_cast<CcBool*>(result.addr);
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


*/

/*
5.7.1 Type Mapping Functions of operator ~memobject~

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

    // mfeedOp.SetUsesArgsInTypeMapping() benutzt;
    // es wird auch die query mit übertragen

    ListExpr str = nl->Second(arg1);

    if(nl->AtomType(str)!=StringType){
            return listutils::typeError("error");
    }

    string oN = nl->StringValue(str);


    if(isMMObject(oN)){

        ListExpr typeExpr = getMMObjectTypeExpr(oN);

        if(listutils::isTupleDescription(typeExpr)){
        ListExpr result =
            nl->TwoElemList(nl->SymbolAtom(Relation::BasicType()), typeExpr);
        return result;
        }

        if(listutils::isDATA(typeExpr)) {
          return typeExpr;
        }
    }
return listutils::typeError("string does not belong to a main memory member");
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
        ListExpr typeExpr = getMMObjectTypeExpr(objectName);


        if (listutils::isTupleDescription(typeExpr)) {
            MemoryRelObject* memObject =
                    (MemoryRelObject*)getMMObject(objectName);
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
                //tup->IncReference();
                //tup->DeleteIfAllowed();
                it++;
            }

            result.setAddr(rel);

            return 0;

        }

        if (listutils::isDATA(typeExpr)) {

            MemoryAttributeObject* memObject =
                    (MemoryAttributeObject*)getMMObject(objectName);
            Attribute* attr = (Attribute*)((qp->ResultStorage(s)).addr);
            attr = memObject->getAttributeObject();
            result.setAddr(attr);
        return 0;
        }
    return 0;
}




/*

5.7.4 Description of operator ~memobject~

Similar to the ~property~ function of a type constructor, an operator needs to
be described, e.g. for the ~list operators~ command.  This is now done by
creating a subclass of class ~OperatorInfo~.

*/



OperatorSpec memobjectSpec(
    "string -> memoryObject",
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

Returns a ~stream(tuple)~.
Each tuple describes one element of the mainmemory catalog.


5.8.1 Type Mapping Functions of operator ~memgetcatalog~

A type mapping function checks whether the correct argument types are supplied
for an operator; if so, it returns a list expression for the result type,
otherwise the symbol ~typeerror~. (-> stream(tuple))

*/


ListExpr memgetcatalogTypeMap(ListExpr args)
{



    string stringlist = "(stream(tuple((TotMemSizInMB int)"
            "(TotUsedMemSizInB int)(Name string)"
            "(ObjectType string)(MemSizeInB int)"
            "(ExtStor bool)(ExtStorSizeinB int))))";
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
       it = catalog.memContents.begin();
       };
       ~memgetcatalogInfo(){}


    Tuple* next(){
        if(it==catalog.memContents.end()) {
            return 0;
        }
        string name = it->first;
        MemoryObject* memobj = it->second;
        string objTyp ="nn";

        ListExpr objectType = getMMObjectTypeExpr(name);
        if (listutils::isTupleDescription(objectType)){
            objTyp = MemoryRelObject::BasicType();
        }
        if (listutils::isDATA(objectType)){
            objTyp = MemoryAttributeObject::BasicType();
        }

        TupleType* tt = new TupleType(nl->Second(resultType));
        Tuple *tup = new Tuple( tt );
        tt->DeleteIfAllowed();

        CcInt* totalMemSize = new CcInt (true, catalog.getMemSizeTotal());
        CcInt* totalUsedMemSize = new CcInt (true, catalog.getUsedMemSize());
        CcString* objectName = new CcString(true,name);
        CcString* oT = new CcString(true,objTyp);
        CcInt* memSize = new CcInt(true, (int)memobj->getMemSize());
        CcBool* extSto = new CcBool(true, memobj->getExtStorage());
        CcInt* extStoSize = new CcInt(true, (int)memobj->getExtStorageSize());

        tup->PutAttribute(0,totalMemSize);
        tup->PutAttribute(1,totalUsedMemSize);
        tup->PutAttribute(2,objectName);
        tup->PutAttribute(3,oT);
        tup->PutAttribute(4,memSize);
        tup->PutAttribute(5,extSto);
        tup->PutAttribute(6,extStoSize);

        it++;
        return tup;
    }
 private:
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

Similar to the ~property~ function of a type constructor, an operator needs to
be described, e.g. for the ~list operators~ command.  This is now done by
creating a subclass of class ~OperatorInfo~.

*/



OperatorSpec memgetcatalogSpec(
    " -> stream(tuple)",
    "memgetcatalog",
    "returns a stream(tuple) of the members of the mainmemory catalog",
    "query memgetcatalog"
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
5.9 Operator ~memlet~


*/

/*
5.9.1 Type Mapping Functions of operator ~memlet~

*/
ListExpr memletTypeMap(ListExpr args)
{
    return tmStringMemloadBool(args);
}


/*

5.9.3  The Value Mapping Functions of operator ~memlet~

*/



int memletValMap (Word* args, Word& result,
                int message, Word& local, Supplier s) {


    bool memletsucceed = false;

    //der Name unter dem das Object im Hauptkatalog
    // auftauchen soll ist objectName
    CcString* oN = (CcString*) args[0].addr;
    if(!oN->IsDefined()){
        return 0;
    }
    string objectName = oN->GetValue();

    if (isMMObject(objectName)){
        cout<< "unter diesem Namen gibt es schon ein HS-objekt."<<endl;

        result  = qp->ResultStorage(s);
        CcBool* b = static_cast<CcBool*>(result.addr);
        b->Set(true, memletsucceed);
        return 0;
    }

    //jetzt brauche ich die Typbeschreibung des zweiten Objects:
     //der zweite Sohn, enthält TypeExpression des Objects
    Supplier t = qp->GetSon( s, 1 );
    ListExpr le = qp->GetType(t);

    if (listutils::isRelDescription(le)){

        GenericRelation* rel= static_cast<Relation*>( args[1].addr );
        //die Hilfsfunktion relToVector wird mit
        //der Relation und der Tupelbeschreibung aufgerufen
        MemoryRelObject* mmRelObject = relToVector(rel,nl->Second(le));

        catalog.memContents[objectName] = mmRelObject;
        catalog.setUsedMemSize(catalog.getUsedMemSize() +
                mmRelObject->getMemSize());

        memletsucceed =true;


    }

    if (listutils::isDATA(le)){

        Attribute* attr = (Attribute*)args[1].addr;

        MemoryAttributeObject* mmA = attrToMM(attr, le);

        catalog.memContents[objectName] = mmA;
        catalog.setUsedMemSize(catalog.getUsedMemSize() + mmA->getMemSize());

        memletsucceed = true;
    }

    if (listutils::isTupleStream(le)){

        MemoryRelObject* mem = tupelStreamToRel(args[1], nl->Second(le));

        catalog.memContents[objectName] = mem;
        catalog.setUsedMemSize(catalog.getUsedMemSize() + mem->getMemSize());

        memletsucceed = true;
    }


    result  = qp->ResultStorage(s);
    CcBool* b = static_cast<CcBool*>(result.addr);
    b->Set(true, memletsucceed);

    return 0;

}




/*

5.9.4 Description of operator ~memlet~

Similar to the ~property~ function of a type constructor, an operator needs to
be described, e.g. for the ~list operators~ command.  This is now done by
creating a subclass of class ~OperatorInfo~.

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
5.10 Operator ~memupdate~


*/

/*
5.10.1 Type Mapping Functions of operator ~memupdate~

*/
ListExpr memupdateTypeMap(ListExpr args)
{
    return tmStringMemloadBool(args);
}


/*

5.10.3  The Value Mapping Functions of operator ~memupdate~

*/



int memupdateValMap (Word* args, Word& result,
                int message, Word& local, Supplier s) {


    bool memupdatesucceed = false;

    CcString* oN = (CcString*) args[0].addr;
    if(!oN->IsDefined()){
        return 0;
    }
    string objectName = oN->GetValue();

    if (!isMMObject(objectName)){
        cout<< "there is no main memory object: "<<objectName<<endl;

        result  = qp->ResultStorage(s);
        CcBool* b = static_cast<CcBool*>(result.addr);
        b->Set(true, memupdatesucceed);
        return 0;
    }


    ListExpr memType = getMMObjectTypeExpr(objectName);

    Supplier t = qp->GetSon( s, 1 );
    ListExpr le = qp->GetType(t);

    if (listutils::isRelDescription(le) &&
                listutils::isTupleDescription(memType) &&
                nl->Equal(nl->Second(le), memType)){

        MemoryRelObject* mem = (MemoryRelObject*)getMMObject(objectName);
        catalog.setUsedMemSize(catalog.getUsedMemSize()-mem->getMemSize());

        delete mem;
        mem = 0;

        GenericRelation* rel= static_cast<Relation*>( args[1].addr );
        mem = relToVector(rel,nl->Second(le));

        catalog.memContents[objectName] = mem;
        catalog.setUsedMemSize(catalog.getUsedMemSize() +
                mem->getMemSize());
        memupdatesucceed =true;

    }


    if (listutils::isDATA(le) && listutils::isDATA(memType) &&
        nl->Equal(le,memType)){

        MemoryAttributeObject* mem =
                (MemoryAttributeObject*)getMMObject(objectName);
        catalog.setUsedMemSize(catalog.getUsedMemSize()-mem->getMemSize());

        delete mem;
        mem = 0;

        Attribute* attr = (Attribute*)args[1].addr;
        mem = attrToMM(attr, le);

        catalog.memContents[objectName] = mem;
        catalog.setUsedMemSize(catalog.getUsedMemSize() + mem->getMemSize());
        memupdatesucceed = true;
        }


    if (listutils::isTupleStream(le) && listutils::isTupleDescription(memType)
                && nl->Equal(nl->Second(le), memType)){

        MemoryRelObject* mem = (MemoryRelObject*)getMMObject(objectName);
        catalog.setUsedMemSize(catalog.getUsedMemSize()-mem->getMemSize());

        delete mem;
        mem = 0;

        mem = tupelStreamToRel(args[1], nl->Second(le));

        catalog.memContents[objectName] = mem;
        catalog.setUsedMemSize(catalog.getUsedMemSize() + mem->getMemSize());
        memupdatesucceed = true;
    } else {
    cout << "type of expression is different from type of object" << endl;
    }



    result  = qp->ResultStorage(s);
    CcBool* b = static_cast<CcBool*>(result.addr);
    b->Set(true, memupdatesucceed);

    return 0;

}




/*

5.10.4 Description of operator ~memupdate~

Similar to the ~property~ function of a type constructor, an operator needs to
be described, e.g. for the ~list operators~ command.  This is now done by
creating a subclass of class ~OperatorInfo~.

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


*/

/*
5.10.1 Type Mapping Functions of operator ~mcreateRtree~

*/
ListExpr mcreateRtreeTypeMap(ListExpr args)
{
    return tmStringStringString(args);
}


/*

5.10.3  The Value Mapping Functions of operator ~mcreateRtree~

*/



int mcreateRtreeValMap (Word* args, Word& result,
                int message, Word& local, Supplier s) {







    string res ="ergebnis";

    result  = qp->ResultStorage(s);
    CcString* str = static_cast<CcString*>(result.addr);
    str->Set(true, res);
    return 0;

}




/*

5.10.4 Description of operator ~mcreateRtree~

Similar to the ~property~ function of a type constructor, an operator needs to
be described, e.g. for the ~list operators~ command.  This is now done by
creating a subclass of class ~OperatorInfo~.

*/



OperatorSpec mcreateRtreeSpec(
    "string x string -> string",
    "mcreateRtree (_,_)",
    "creates an mrtree???? the first string describes the object,"
    "the second string describs the attribute",
    "query memupdate ('fuenf', ten feed head[7])"
);



/*

5.10.5 Instance of operator ~mcreateRtree~

*/

Operator mcreateRtreeOp (
    "mcreateRtree",
    mcreateRtreeSpec.getStr(),
    mcreateRtreeValMap,
    Operator::SimpleSelect,
    mcreateRtreeTypeMap
);


class MainMemoryAlgebra : public Algebra
{


    public:
        MainMemoryAlgebra() : Algebra()
        {
/*

6.2 Registration of Types


*/

//        AddTypeConstructor (&MemoryRelObjectTC);
//        MemoryRelObjectTC.AssociateKind( Kind::SIMPLE() );
//        AddTypeConstructor (&MemoryObjectTC);
//        MemoryObjectTC.AssociateKind( Kind::SIMPLE() );

/*
6.3 Registration of Operators

*/
        AddOperator (&memloadOp);
        AddOperator (&meminitOp);
        meminitOp.SetUsesMemory();
        AddOperator (&mfeedOp);
        mfeedOp.SetUsesArgsInTypeMapping();
        AddOperator (&letmconsumeOp);
        AddOperator (&memdeleteOp);
        AddOperator (&memobjectOp);
        memobjectOp.SetUsesArgsInTypeMapping();
        AddOperator (&memgetcatalogOp);
        AddOperator (&memletOp);
        AddOperator (&memupdateOp);
        AddOperator (&mcreateRtreeOp);


        }
        ~MainMemoryAlgebra() {};
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




