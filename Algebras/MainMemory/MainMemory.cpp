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
    clear();
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

bool MemCatalog::isAccessible(const string& name) {
    SecondoSystem* sys = SecondoSystem::GetInstance();
    return   (isMMObject(name) &&
        ((getMMObject(name)->getDatabase()==sys->GetDatabaseName())
            || getMMObject(name)->hasflob()) );
}

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

string MemoryObject::getDatabase(){
    return database;
}

bool MemoryObject::hasflob(){
    return flob;
}


//void MemoryObject::setObjectTypeExpr(string oTE){
//    objectTypeExpr=oTE;
//};



// MEMORYRELOBJECT

MemoryRelObject::MemoryRelObject(){};
MemoryRelObject::MemoryRelObject(vector<Tuple*>* _mmrel,
                size_t _memSize, string _objectTypeExpr, bool _flob,
                string _database){
        mmrel = _mmrel;
        memSize = _memSize;
        objectTypeExpr = _objectTypeExpr;
        flob = _flob;
        database = _database;
};
MemoryRelObject::MemoryRelObject (string _objectTypeExpr){
        objectTypeExpr = _objectTypeExpr;
        mmrel = 0;
        memSize = 0;
};

MemoryRelObject::~MemoryRelObject(){
    if (!mmrel==0){
        vector<Tuple*>::iterator it = mmrel->begin();
        while (it!=mmrel->end()){
            Tuple* tup = *it;
            tup->DeleteIfAllowed();
            tup = 0;
            it++;
        }
    delete mmrel;
    }
}
vector<Tuple*>* MemoryRelObject::getmmrel(){
    return mmrel;
};

void MemoryRelObject::setmmrel(vector<Tuple*>* _mmrel){
    mmrel = _mmrel;
};



void MemoryRelObject::addTuple(Tuple* tup){

    if(mmrel==0){
    mmrel = new vector<Tuple*>();
    }

    int tupleSize = 0;
    tupleSize = tup->GetMemSize();
    size_t availableMemSize =
            (catalog.getMemSizeTotal()*1024*1024)-catalog.getUsedMemSize();
    if ((size_t)tupleSize<availableMemSize){
                mmrel->push_back(tup);
                memSize += tupleSize;

            //Die usedMemSize des Katalogs wird noch nicht angepasst!!
            // soll das immer neu berechnet werden??
             //   catalog->usedMemSize += tupleSize;

           }
    else{
    cout<< "the memSize is not enough, the object"
    "is usable but not complete"<<endl;
                                                }
}

ListExpr MemoryRelObject::toListExpr(){
    int vectorSize = mmrel->size();
    ListExpr typeListExpr = 0;
    string typeString = objectTypeExpr;
    nl->ReadFromString(typeString, typeListExpr);
    //speichere ich insgesamt falsche objectTypeExpr????
    //ohne folgende Zeile Fehler in der Tupel out Methode
    ListExpr oTE = nl->OneElemList(typeListExpr);
    Tuple* t = mmrel->at(0);
    ListExpr li=nl->OneElemList(t->Out(oTE));
    ListExpr last = li;
    for (int i=1; i<vectorSize; i++){
        t=mmrel->at(i);
        last = nl->Append(last, t->Out(oTE));
    }
    ListExpr memoryObjectdescription = nl->TwoElemList(nl->SymbolAtom
                    (MemoryRelObject::BasicType()), typeListExpr);

    return nl->TwoElemList(memoryObjectdescription, li);
};



 Word MemoryRelObject::In( const ListExpr typeInfo, const ListExpr instance,
                       const int errorPos, ListExpr& errorInfo,
                        bool& correct ){
cout<<"Start MemoryRelObject::in"<<endl;
    // ToDO list is correct?

    Word result;

    correct = true;
    Tuple* tup;
    ListExpr tupleList;
    ListExpr tupleTypeInfo;
    ListExpr first;
    int tupleno;
    bool tupleCorrect;
    MemoryRelObject* mmrelObj =
            new MemoryRelObject(nl->ToString(nl->Second(typeInfo)));


    tupleList = instance;
    tupleTypeInfo = nl->TwoElemList(nl->Second(typeInfo),
        nl->IntAtom(nl->ListLength(nl->Second(nl->Second(typeInfo)))));
    tupleno = 0;

    while (!nl->IsEmpty(tupleList)){
        first = nl->First(tupleList);
        tupleList = nl->Rest(tupleList);
        tupleno++;
        tup = Tuple::In(tupleTypeInfo, first, tupleno,
                            errorInfo, tupleCorrect);

        if (tupleCorrect){
            mmrelObj->addTuple(tup);

           // tup->DeleteIfAllowed();             ????????????
    //        count++;
        }
        else {
            correct = false;
        }
    }

    if(!correct){
        result = SetWord(Address(0));
    } else {

        result.addr = mmrelObj;
    }
    cout<<"Ende MemoryRelObject::in"<<endl;
    return result;
}


ListExpr MemoryRelObject::Out( ListExpr typeInfo, Word value ){
    MemoryRelObject* memRel = static_cast<MemoryRelObject*>( value.addr );
    return memRel->toListExpr();

};

bool MemoryRelObject::KindCheck( ListExpr type, ListExpr& errorInfo )
{
    ListExpr first = nl->First(type);
    ListExpr second = nl->Second(type);
    return (listutils::isTupleDescription(second) &&
        (nl ->IsEqual(first,BasicType())));
}

const bool MemoryRelObject::checkType(const ListExpr type){
    ListExpr first = nl->First(type);
    ListExpr second = nl->Second(type);
    return (listutils::isTupleDescription(second) &&
        (nl ->IsEqual(first,BasicType())));
        }


Word MemoryRelObject::create(const ListExpr typeInfo) {
    //muss eventuell Second(typeInfo) sein??
    MemoryRelObject* mmrelObject = new MemoryRelObject
        ( nl->ToString(nl->Second(typeInfo)));
    return (SetWord(mmrelObject));
}

int MemoryRelObject::SizeOfObj(){

    return 1111111111;
};

void MemoryRelObject::deleteMemoryRelObject(const ListExpr typeInfo, Word& w){
    MemoryRelObject* memRelO = (MemoryRelObject*)w.addr;
    delete memRelO;
    w.addr = 0;
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
            nl->StringAtom("-> SIMPLE"),
            nl->StringAtom(MemoryRelObject::BasicType()),
            nl->StringAtom("List of tuples"),
            nl->StringAtom(("Meyer, 7"),("Muller, 5"))
            )));
}




// MEMORYATTRIBUTEOBJECT


MemoryAttributeObject::MemoryAttributeObject(Attribute* _attr,
                    size_t _memSize, string _objectTypeExpr, bool _flob,
                    string _database){
        attributeObject = _attr;
        memSize = _memSize;
        objectTypeExpr =_objectTypeExpr;
        flob = _flob;
        database = _database;
}

MemoryAttributeObject::~MemoryAttributeObject(){

cout<<"Destruktor von MemoryAttributObject"<<endl;

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
cout<<"Destruktor von MemoryRtreeObject"<<endl;
}

void MemoryRtreeObject::setRtree(mmrtree::RtreeT<2, size_t>* _rtree){
            rtree=_rtree;
        }

mmrtree::RtreeT<2, size_t>* MemoryRtreeObject::getrtree(){
        return rtree;
}

MemoryAVLObject::MemoryAVLObject(){};
MemoryAVLObject::MemoryAVLObject( avltree::AVLTree< pair<Attribute*,size_t>,
            KeyComparator >* _tree, size_t _memSize, string _objectTypeExpr,
            string _keyType ){

            tree = _tree;
            memSize = _memSize;
            objectTypeExpr = _objectTypeExpr;
            keyType = _keyType;
            };
MemoryAVLObject::~MemoryAVLObject(){
cout<<"Destruktor von MemoryAVLObject"<<endl;
}

avltree::AVLTree< pair<Attribute*,size_t>,KeyComparator >*
                                MemoryAVLObject::getAVLtree(){
        return tree;
};

string MemoryAVLObject::getKeyType(){

        return keyType;
};

}//end namespace mmalgebra

