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


#include "NestedList.h"
#include "ListUtils.h"
#include <vector>

using namespace std;

#include "MainMemory.h"
namespace mmalgebra {

extern MemCatalog catalog;


MemCatalog::~MemCatalog(){
cout<<"START Destruktor MemCatalog!"<<endl;
   clear();
    //delete memContents;
cout<<"ENDE Destruktor MemCatalog!"<<endl;
};

void MemCatalog::setMemSizeTotal(size_t size) {
            memSizeTotal = size;
}

size_t MemCatalog::getMemSizeTotal(){
            return memSizeTotal;
}

size_t MemCatalog::getUsedMemSize() {
            return usedMemSize;
}

size_t MemCatalog::getAvailabeMemSize() {

    return (memSizeTotal*1024*1024)-(usedMemSize);
}

//only used in memgetcatalog
map<string,MemoryObject*>* MemCatalog::getMemContent(){

            return &memContents;
}
//void MemCatalog::setUsedMemSize(size_t size) {
//            usedMemSize = size;
//    }

bool MemCatalog::insert (const string& name, MemoryObject* obj){
     if (isMMObject(name)){
        cout<<"identifier already in use"<<endl;
        return false;
    }

    memContents[name] = obj;
    usedMemSize += obj->getMemSize();

    return true;
}

bool MemCatalog::deleteObject (const string& name){

     if(isMMObject(name)){
        usedMemSize -= getMMObject(name)->getMemSize();
        delete getMMObject(name);
        memContents.erase(name);
        return true;
     }
    return false;
}

void MemCatalog::clear (){
    map<string,MemoryObject*>::iterator it;
    it = memContents.begin();
    while (it!=memContents.end()){
cout<<"Objekt: "<<it->first<<endl;
        deleteObject(it->first);
        it++;
    }
}

bool MemCatalog::isMMObject(const string& objectName){
    if (memContents.find(objectName)==memContents.end()){
            return false;
    }
    return true;
}

/*
Function returns a pointer to the memory object
*Precondition*: "isMMObject( objectName ) == true"

*/

MemoryObject* MemCatalog::getMMObject(const string& objectName){

    map<string,MemoryObject*>::iterator it;
    it = memContents.find(objectName);
    return it->second;
}

ListExpr MemCatalog::getMMObjectTypeExpr(const string& oN){

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

MemoryObject::~MemoryObject(){}

void MemoryObject::setMemSize(size_t i){
    memSize = i;
}
size_t MemoryObject::getMemSize (){
    return memSize;
};

string MemoryObject::getObjectTypeExpr(){
    return objectTypeExpr;
}
void MemoryObject::setObjectTypeExpr(string oTE){
    objectTypeExpr=oTE;
};

//Testweise
ListExpr MemoryObject::Out( ListExpr typeInfo, Word value ){

    ListExpr li = nl->IntAtom(23);
    return li;

}


//nochmal!!!
ListExpr MemoryObject::Property(){
    return (nl->TwoElemList (
        nl->FourElemList (
            nl->StringAtom("Signature"),
            nl->StringAtom("Example Type List"),
            nl->StringAtom("List Rep"),
            nl->StringAtom("Example List")),
        nl->FourElemList (
            nl->StringAtom("-> SIMPLE"), //nicht doch eher rel -> ja nach was??
            nl->StringAtom(MemoryObject::BasicType()),
            nl->StringAtom("??A List of tuples"),
            nl->StringAtom(("Meyer, 7"),("Muller, 5"))
            )));
}



TypeConstructor MemoryObjectTC(
    MemoryObject::BasicType(),     // name of the type in SECONDO
    MemoryObject::Property,        // property function describing signature
    MemoryObject::Out, 0,          // out und in functions
    0, 0,                             // SaveToList, RestoreFromList functions
    0,0,                             // object creation and deletion
    0, 0,                            // object open, save
    0,0,                             // close and clone
    0,                                // cast function
    0,                        // sizeof function
    0);                          // kind checking function



MemoryRelObject::MemoryRelObject(){};
MemoryRelObject::MemoryRelObject(vector<Tuple*>* _mmrel,
                size_t _memSize, string _objectTypeExpr){
        mmrel = _mmrel;
        memSize = _memSize;
        objectTypeExpr =_objectTypeExpr;
};

MemoryRelObject::~MemoryRelObject(){
cout<<"start destruktor memoryrel:"<<endl;
    vector<Tuple*>::iterator it = mmrel->begin();
    while (it!=mmrel->end()){
        Tuple* tup = *it;
        tup->DeleteIfAllowed();
        //delete tup;
        tup = 0;
        it++;
    }
    delete mmrel;
cout<<"ende destruktor MemoryRelObject"<< endl;
}

vector<Tuple*>* MemoryRelObject::getmmrel(){
    return mmrel;
};

void MemoryRelObject::setmmrel(vector<Tuple*>* _mmrel){
    mmrel = _mmrel;
};



void MemoryRelObject::addTuple(Tuple* tup){

    int tupleSize = 0;
    tupleSize = tup->GetMemSize();
    size_t availableMemSize =
            (catalog.getMemSizeTotal()*1024*1024)-catalog.getUsedMemSize();

    if ((size_t)tupleSize<availableMemSize){
                mmrel->push_back(tup);
                memSize += tupleSize;
                //catalog.usedMemSize += tupleSize;

           }
    else{
    cout<< "the memSize is not enough, the object"
    "is usable but not complete"<<endl;
                                                }
}




//
// Word MemoryRelObject::In( const ListExpr typeInfo, const ListExpr instance,
//                       const int errorPos, ListExpr& errorInfo,
//                        bool& correct ){
//
//  correct = false;
//    Word result = SetWord(Address(0));
//    const string errMsg = "Leider ist in noch nicht implementiert";
//    return result;
//
//}

ListExpr MemoryRelObject::Out( ListExpr typeInfo, Word value ){

    MemoryRelObject* memRel = static_cast<MemoryRelObject*>( value.addr );
    int vectorSize = memRel->mmrel->size();
    ListExpr objectTypeExpr = 0;
    string type = memRel->getObjectTypeExpr();
    nl->ReadFromString(type, objectTypeExpr);

    Tuple* t = memRel->mmrel->at(0);
    ListExpr l=0;
    l=t->Out(objectTypeExpr);
    ListExpr last = l;
    ListExpr temp = 0;;

    cout << "erstes Tupel: "<<nl->ToString(l);
    cout << "VectorGrösse"<< vectorSize << endl;

    for (int i=1; i<vectorSize; i++){
        t=memRel->mmrel->at(i);
        temp=t->Out(objectTypeExpr);
        last = nl->Append(last,temp);
    }
    cout<< "meine MemoryRelObject out-Funktion..."<<nl->ToString(last)<< endl;
    return last;
};




//nochmal!!!
ListExpr MemoryRelObject::Property(){
    return (nl->TwoElemList (
        nl->FourElemList (
            nl->StringAtom("Signature"),
            nl->StringAtom("Example Type List"),
            nl->StringAtom("List Rep"),
            nl->StringAtom("Example List")),
        nl->FourElemList (
            nl->StringAtom("-> SIMPLE"), //nicht doch eher rel -> ja nach was??
            nl->StringAtom(MemoryRelObject::BasicType()),
            nl->StringAtom("??A List of tuples"),
            nl->StringAtom(("Meyer, 7"),("Muller, 5"))
            )));
}



TypeConstructor MemoryRelObjectTC(
    MemoryRelObject::BasicType(),     // name of the type in SECONDO
    MemoryRelObject::Property,        // property function describing signature
    MemoryRelObject::Out, 0,          // out und in functions
    0, 0,                             // SaveToList, RestoreFromList functions
    0,0,                             // object creation and deletion
    0, 0,                            // object open, save
    0,0,                             // close and clone
    0,                                // cast function
    0,      // sizeof function
    0);      // kind checking function

MemoryAttributeObject::MemoryAttributeObject(Attribute* _attr,
                    size_t _memSize, string _objectTypeExpr){
        attributeObject = _attr;
        memSize = _memSize;
        objectTypeExpr =_objectTypeExpr;
}

MemoryAttributeObject::~MemoryAttributeObject(){

cout<<"Destruktor von MemoryAttributObject - nicht fertig "<<endl;

}


void MemoryAttributeObject::setAttributeObject(Attribute* attr){
            attributeObject=attr;
        }
Attribute* MemoryAttributeObject::getAttributeObject(){
        return attributeObject;
}


MemoryRtreeObject::MemoryRtreeObject(){};
MemoryRtreeObject::MemoryRtreeObject (mmrtree::RtreeT<2, size_t>* _rtree,
            size_t _memSize, string _objectTypeExpr){

    rtree = _rtree;
    memSize = _memSize;
    objectTypeExpr =_objectTypeExpr;
};
MemoryRtreeObject::~MemoryRtreeObject(){
cout<<"Destruktor von MemoryRtreeObject - nicht fertig "<<endl;
}

void MemoryRtreeObject::setRtree(mmrtree::RtreeT<2, size_t>* _rtree){
            rtree=_rtree;
        }
}//end namespace mmalgebra

