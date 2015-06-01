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
5.6.1 Type Mapping Functions of operator ~memdelet~

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

             return 0;
        }
    string objectName = oN->GetValue();

    if(isMMObject(objectName)){

        MemoryObject* mem = getMMObject(objectName);
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




class MainMemoryAlgebra : public Algebra
{


    public:
        MainMemoryAlgebra() : Algebra()
        {
/*

6.2 Registration of Types


*/

        AddTypeConstructor (&MemoryRelObjectTC);
        MemoryRelObjectTC.AssociateKind( Kind::SIMPLE() );


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




