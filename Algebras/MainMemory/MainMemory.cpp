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
#include "Stream.h"
#include "MainMemory.h"

using namespace std;

namespace mmalgebra {

extern MemCatalog* catalog;


MemCatalog::~MemCatalog(){
    clear();
};

void MemCatalog::setMemSizeTotal(size_t size) {
            memSizeTotal = size;
}

size_t MemCatalog::getMemSizeTotal(){
            return memSizeTotal;
}

unsigned long MemCatalog::getUsedMemSize() {
            return usedMemSize;
}

unsigned long MemCatalog::getAvailabeMemSize() {

    return (memSizeTotal*1024*1024)-(usedMemSize);
}

//only used in memgetcatalog
map<string,MemoryObject*>* MemCatalog::getMemContent(){

            return &memContents;
}


bool MemCatalog::insert (const string& name, MemoryObject* obj){
     if (isMMObject(name)){
        cout<<"identifier already in use"<<endl;
        return false;
    }

    memContents[name] = obj;
    usedMemSize += obj->getMemSize();

    return true;
}

bool MemCatalog::deleteObject (const string& name, const bool erase/*=true*/){
     if(isMMObject(name)){
        usedMemSize -= getMMObject(name)->getMemSize();
        delete getMMObject(name);
        if(erase){
           memContents.erase(name);
        }
        return true;
     }
    return false;
}


void MemCatalog::clear (){
    map<string,MemoryObject*>::iterator it;
    it = memContents.begin();
    while(it!=memContents.end()){
        deleteObject(it->first, false);
        it++;
    }
    memContents.clear();
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
    if(it==memContents.end()){
      return 0;
    }
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
    MemoryObject* obj = getMMObject(name);
    if(!obj){
      return false;
    }  
    if(obj->hasflob()){
      return true;
    }
    SecondoSystem* sys = SecondoSystem::GetInstance();
    if(obj->getDatabase() == sys->GetDatabaseName()){
      return true;
    }
    return false;

}

MemoryObject::~MemoryObject(){}

unsigned long MemoryObject::getMemSize (){
    return memSize;
};

string MemoryObject::getObjectTypeExpr(){
    return objectTypeExpr;
}
void MemoryObject::setObjectTypeExpr(string _oTE){
    objectTypeExpr = _oTE;
}

string MemoryObject::getDatabase(){
    return database;
}

bool MemoryObject::hasflob(){
    return flob;
}


  // MEMORYRELOBJECT

MemoryRelObject::MemoryRelObject(){
    mmrel = new vector<Tuple*>();
};

MemoryRelObject::MemoryRelObject(vector<Tuple*>* _mmrel,
                unsigned long _memSize, string _objectTypeExpr, bool _flob,
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
            //delete tup;
            tup = 0;
            it++;
        }
    delete mmrel;
    }
}

vector<Tuple*>* MemoryRelObject::getmmrel(){
    return mmrel;
}

void MemoryRelObject::addTuple(Tuple* tup){
    if(mmrel==0){
        mmrel = new vector<Tuple*>();
    }

    size_t tupleSize = 0;
    tupleSize = tup->GetMemSize();
    unsigned long availableMemSize =
            catalog->getAvailabeMemSize();
    if ((size_t)tupleSize<availableMemSize){
        tup->SetTupleId(mmrel->size());
        mmrel->push_back(tup);
        memSize += tupleSize;
        catalog->addToUsedMemSize(tupleSize);
    }
    else{
        cout<< "the memSize is not enough, the object"
        " might be usable but not complete"<<endl;
    }
}

bool MemoryRelObject::relToVector(
                        GenericRelation* r, 
                        ListExpr le,
                        string _database, 
                        bool _flob) {

    GenericRelationIterator* rit;
    rit = r->MakeScan();
    Tuple* tup;
    int tupleSize=0;
    unsigned long availableMemSize = catalog->getAvailabeMemSize();
    unsigned long usedMainMemory=0;
    mmrel->clear();
    this->flob = _flob;

    while ((tup = rit->GetNextTuple()) != 0){
        if (flob){
            tup->bringToMemory();
        }
        tupleSize = tup->GetMemSize();
        if ((size_t)tupleSize<availableMemSize){
            tup->SetTupleId(mmrel->size());
            mmrel->push_back(tup);
            usedMainMemory += tupleSize;
            availableMemSize -= tupleSize;
        }
        else{
            if (mmrel->size()==0){
                cout<<"no memory left"<<endl;
                return false;
            }
             cout<< "the available main memory is not enough, the object"
            " might be usable but not complete"<<endl;
            break;
            }
    }
    delete rit;
    memSize = usedMainMemory;
    objectTypeExpr = nl->ToString(nl->TwoElemList( 
                               listutils::basicSymbol<Mem>(),le));
    flob = _flob;
    database = _database;
    return true;
}

bool MemoryRelObject::tupleStreamToRel(Word arg, ListExpr le,
                string _database = "", bool _flob = false){

    Stream<Tuple> stream(arg);
    size_t availableMemSize = catalog->getAvailabeMemSize();
    unsigned long usedMainMemory =0;
    Tuple* tup;
    int tupleSize = 0;

    stream.open();

    while( (tup = stream.request()) != 0){
        if (flob){
            tup->bringToMemory();
        }
        tupleSize = tup->GetMemSize();
        if ((size_t)tupleSize<availableMemSize){
            tup->SetTupleId(mmrel->size());
            mmrel->push_back(tup);
            usedMainMemory += tupleSize;
            availableMemSize -= tupleSize;
        }
        else{
            if (mmrel->size()==0){
                cout<<"no memory left"<<endl;
                return false;
            }
            cout<< "the memSize is not enough, the object"
            " might be usable but not complete"<<endl;
            break;
        }
    }
    memSize = usedMainMemory;
    objectTypeExpr = nl->ToString(nl->TwoElemList(
                                  listutils::basicSymbol<Mem>(),
                                  nl->TwoElemList( 
                                    listutils::basicSymbol<Relation>(),
                                    nl->Second(le))));
    flob = _flob;
    database = _database;

    stream.close();
    return true;
}



ListExpr MemoryRelObject::toListExpr(){

    int vectorSize = mmrel->size();
    ListExpr typeListExpr = 0;
    string typeString = objectTypeExpr;
    nl->ReadFromString(typeString, typeListExpr);
    ListExpr oTE = nl->OneElemList(typeListExpr);
    Tuple* t = mmrel->at(0);
    ListExpr li=nl->OneElemList(t->Out(oTE));
    ListExpr last = li;
    for (int i=1; i<vectorSize; i++){
        t=mmrel->at(i);
        last = nl->Append(last, t->Out(oTE));
    }
    return li;
};

Word MemoryRelObject::In( const ListExpr typeInfo, const ListExpr instance,
                       const int errorPos, ListExpr& errorInfo,
                        bool& correct ){
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
    return result;
}


ListExpr MemoryRelObject::Out( ListExpr typeInfo, Word value ){
    MemoryRelObject* memRel = static_cast<MemoryRelObject*>( value.addr );
    if (memRel->getmmrel()==0 || (memRel->getmmrel())->size() == 0){
    return 0;
    }
    return memRel->toListExpr();
};

bool MemoryRelObject::KindCheck( ListExpr type, ListExpr& errorInfo )
{
    if (nl->ListLength(type)!=2){
            return false;
        }
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
    MemoryRelObject* mmrelObject = new MemoryRelObject
        ( nl->ToString(nl->Second(typeInfo)));
    return (SetWord(mmrelObject));
}

bool MemoryRelObject::Save(SmiRecord& valueRecord, size_t& offset,
                            const ListExpr typeInfo, Word& value){

    size_t size = sizeof(int);
    int v = 7;
    bool ok = valueRecord.Write(&v, size, offset);
    offset += size;
    return ok;
};

bool MemoryRelObject::Open (SmiRecord& valueRecord, size_t& offset,
                            const ListExpr typeInfo, Word& value){
    int i;
    size_t size = sizeof(int);
    bool ok = valueRecord.Read(&i, size, offset);
    offset += size;
    if(ok){
        value.addr = new MemoryRelObject();
    } else {
        value.addr = 0;
    }
    return ok;
};

void MemoryRelObject::Close (const ListExpr typeInfo, Word& w){
    MemoryRelObject *k = (MemoryRelObject *)w.addr;
    delete k;
    w.addr = 0;
};

Word MemoryRelObject::Clone (const ListExpr typeInfo, const Word& w){
    MemoryRelObject* m = (MemoryRelObject*) w.addr;
    MemoryRelObject* n = m;
    Word res;
    res.addr = n;
    return res;
};

void* MemoryRelObject::Cast (void* addr){
    return (new (addr) MemoryRelObject);
};

int MemoryRelObject::SizeOfObj(){

    return sizeof(int);
};

void MemoryRelObject::Delete(const ListExpr typeInfo, Word& w){
    MemoryRelObject* memRelO = (MemoryRelObject*)w.addr;
    delete memRelO;
    w.addr = 0;
};


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
            nl->StringAtom("(<tuple>*)where <tuple> is (<attr1>...<attrn>)"),
            nl->StringAtom("((\"Meyer\" 7),(\"Muller\" 5))")
            )));
}




// MEMORYATTRIBUTEOBJECT

MemoryAttributeObject::MemoryAttributeObject(){
      attributeObject = 0;
        memSize = 0;
        objectTypeExpr = "";
        flob = 0;
        database = "";
};

MemoryAttributeObject::MemoryAttributeObject(Attribute* _attr,
                    unsigned long _memSize, string _objectTypeExpr, bool _flob,
                    string _database){
        attributeObject = _attr;
        memSize = _memSize;
        objectTypeExpr =_objectTypeExpr;
        flob = _flob;
        database = _database;
}

MemoryAttributeObject::~MemoryAttributeObject(){
    if (attributeObject!=0){
        delete attributeObject;
    }
}


Attribute* MemoryAttributeObject::getAttributeObject(){
        return attributeObject;
}

bool MemoryAttributeObject::attrToMM(Attribute* attr,
            ListExpr le, string _database, bool _flob){
    size_t availableMemSize = catalog->getAvailabeMemSize();
    unsigned long usedMainMemory=0;
    if(_flob){
        attr->bringToMemory();
    }
    usedMainMemory = attr->GetMemSize();
    if (usedMainMemory>availableMemSize){
            cout <<"the available main memory size is not enough"<<endl;
            return false;
        }
    attributeObject = attr->Copy();
    memSize = usedMainMemory;
    objectTypeExpr = nl->ToString(nl->TwoElemList(
                                     listutils::basicSymbol<Mem>(),le));
    flob = _flob;
    database = _database;
    return true;
}


MemoryAVLObject::MemoryAVLObject(){};
MemoryAVLObject::MemoryAVLObject( avltree::AVLTree< pair<Attribute*,size_t>,
            KeyComparator >* _tree, size_t _memSize, string _objectTypeExpr,
            bool _flob, string _database){

            tree = _tree;
            memSize = _memSize;
            objectTypeExpr = _objectTypeExpr;
            flob = _flob;
            database = _database;
            };
MemoryAVLObject::~MemoryAVLObject(){
    if (tree){
        delete tree;
    }
}

avltree::AVLTree< pair<Attribute*,size_t>,KeyComparator >*
                                MemoryAVLObject::getAVLtree(){
        return tree;
};


}//end namespace mmalgebra

