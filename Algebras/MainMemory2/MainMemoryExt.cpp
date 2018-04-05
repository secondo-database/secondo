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

*/


#include "NestedList.h"
#include "ListUtils.h"
#include <vector>
#include "Algebras/Stream/Stream.h"
#include "MainMemoryExt.h"

#include "MemoryObject.h"
#include "MemCatalog.h"


//#include "GraphAlgebra.h"

using namespace std;

namespace mm2algebra {

extern MemCatalog* catalog;




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
    if (mmrel!=0){
        vector<Tuple*>::iterator it = mmrel->begin();
        while (it!=mmrel->end()){
            Tuple* tup = *it;
            if(tup){
               tup->DeleteIfAllowed();
            }
            it++;
        }
        delete mmrel;
    }
}

vector<Tuple*>* MemoryRelObject::getmmrel(){
    return mmrel;
}

Tuple* MemoryRelObject::getTuple(TupleId tid) {
  vector<Tuple*>::iterator it = mmrel->begin();
  Tuple* tup;
  
  while(it != mmrel->end()) {
    tup = *it;
    if(tid == tup->GetTupleId()) {
      return tup;
    }
    it++;
  }
  return 0;
}

void MemoryRelObject::addTuple(Tuple* tup){
    if(mmrel==0){
        mmrel = new vector<Tuple*>();
    }

    size_t tupleSize = 0;
    tupleSize = tup->GetMemSize();
    unsigned long availableMemSize =
            catalog->getAvailableMemSize();
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
    unsigned long availableMemSize = catalog->getAvailableMemSize();
    unsigned long usedMainMemory=0;
    // remove old content
    vector<Tuple*>::iterator it;
    for(it = mmrel->begin(); it!=mmrel->end(); it++){
      if(*it){
         (*it)->DeleteIfAllowed();
      }
    }
    mmrel->clear();
    this->flob = _flob;

    while ((tup = rit->GetNextTuple()) != 0){
        if (flob){
            tup->bringToMemory();
        }
        tupleSize = tup->GetMemSize();
        if ((size_t)tupleSize<availableMemSize){
            tup->SetTupleId(mmrel->size()+1);
            mmrel->push_back(tup);
            usedMainMemory += tupleSize;
            availableMemSize -= tupleSize;
        }else{
            if (mmrel->size()==0){
                cout<<"no memory left"<<endl;
                return false;
            }
             cout<< "the available main memory is not enough, the object"
            " might be usable but not complete"<<endl;
            tup->DeleteIfAllowed();
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

// TODO
bool MemoryRelObject::mmrelToVector(
                        std::vector<Tuple*>* r, 
                        ListExpr le,
                        string _database, 
                        bool _flob) {

    
    Tuple* tup;
    int tupleSize=0;
    unsigned long availableMemSize = catalog->getAvailableMemSize();
    unsigned long usedMainMemory=0;
    mmrel->clear();
    this->flob = _flob;

    for(size_t i=0; i<r->size(); i++) {
      tup = r->at(i);
        if (flob){
          tup->bringToMemory();
        }
        tupleSize = tup->GetMemSize();
        if ((size_t)tupleSize<availableMemSize){
            tup->SetTupleId(mmrel->size()+1);
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
    memSize = usedMainMemory;
    objectTypeExpr = nl->ToString(le);
    flob = _flob;
    database = _database;
    return true;
}


bool MemoryRelObject::tupleStreamToRel(Word arg, ListExpr le,
                string _database, bool _flob){

    Stream<Tuple> stream(arg);
    flob = _flob;
    size_t availableMemSize = catalog->getAvailableMemSize();
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
            tup->SetTupleId(mmrel->size()+1);
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

    ListExpr li = nl->OneElemList(t->Out(oTE));
    ListExpr last = li;
    for(int i=1; i<vectorSize; i++) {
        t = mmrel->at(i);
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
    if(!nl->HasLength(type,2)){
      return false;
    }
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
    size_t availableMemSize = catalog->getAvailableMemSize();
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
MemoryAVLObject::MemoryAVLObject( memAVLtree* _tree, 
                                  size_t _memSize, 
                                  const string& _objectTypeExpr,
                                  bool _flob, 
                                  const string& _database){

            tree = _tree;
            memSize = _memSize;
            objectTypeExpr = _objectTypeExpr;
            flob = _flob;
            database = _database;
            };


void pairdestroy(AttrIdPair& p){
  //p.first->DeleteIfAllowed();
  //p.first = 0;
}

MemoryAVLObject::~MemoryAVLObject(){
    if (tree){
        tree->destroy(pairdestroy);
        delete tree;
    }
}

memAVLtree* MemoryAVLObject::getAVLtree(){
    return tree;
}



// MEMORYORELOBJECT ///////////////////////////////////////////////
MemoryORelObject::MemoryORelObject(){
    mmorel = new ttree::TTree<TupleWrap,TupleComp>(16,18);
    pos = new std::vector<int>();
};

MemoryORelObject::MemoryORelObject(ttree::TTree<TupleWrap,TupleComp>* _mmorel,
                                   std::vector<int>* _pos, 
                                   unsigned long _memSize, 
                                   std::string _objectTypeExpr, bool _flob,
                                   std::string _database) {
    mmorel = _mmorel;
    pos = _pos;
    memSize = _memSize;
    objectTypeExpr = _objectTypeExpr;
    flob = _flob;
    database = _database;
};

MemoryORelObject::MemoryORelObject (string _objectTypeExpr){
    objectTypeExpr = _objectTypeExpr;
    pos = 0;
    mmorel = 0;
    memSize = 0;
};


MemoryORelObject::~MemoryORelObject() {
  if(mmorel!=0) {
    delete mmorel;
  }
  pos->clear();
  delete pos;
}

ttree::TTree<TupleWrap,TupleComp>* MemoryORelObject::getmmorel(){
  return mmorel;
}


std::vector<int>* MemoryORelObject::getAttrPos() {
return pos;
}


void MemoryORelObject::setAttrPos(int attrPos, bool keep) {
if(!pos)
  pos = new std::vector<int>();
if(pos && !keep)
  pos->clear();

pos->push_back(attrPos);
}


void MemoryORelObject::setAttrPos() {
if(pos)
  pos->clear();
else 
  pos = new std::vector<int>();
pos->push_back(1);
pos->push_back(2);
}


void MemoryORelObject::addTuple(Tuple* tup){
  
  if(mmorel==0) {
      cout << "crete new tree" << endl;
      mmorel = new ttree::TTree<TupleWrap,TupleComp>(4,8);
  }

  size_t tupleSize = 0;
  tupleSize = tup->GetMemSize();
  unsigned long availableMemSize = catalog->getAvailableMemSize();

  if((size_t)tupleSize<availableMemSize) {
      size_t tid = mmorel->noEntries()+1;
      tup->SetTupleId(tid);
      TupleWrap tw(tup);
      mmorel->insert(tw,pos);
      memSize += tupleSize;
      catalog->addToUsedMemSize(tupleSize);
  } else {
      cout << "the memSize is not enough, the object"
              " might be usable but not complete" << endl;
  }
}


std::ostream& MemoryORelObject::print(std::ostream& out) const {
out << endl << "MMOREL: " << endl;

ttree::Iterator<TupleWrap,TupleComp> it = mmorel->begin();

while(it.hasNext()) {
  Tuple* tup = (*it).getPointer();
    tup->Print(out); 
    it++;
  }
  out << endl <<"PRINT END MemoryORelObject" << endl<< endl;
  return out;
}

bool MemoryORelObject::relToTree(
                        GenericRelation* r, 
                        ListExpr le,
                        string _database, 
                        bool _flob) {
    
    GenericRelationIterator* rit;
    rit = r->MakeScan();
    Tuple* tup;
    int tupleSize=0;
    unsigned long availableMemSize = catalog->getAvailableMemSize();
    unsigned long usedMainMemory = 0;
    
    mmorel->clear(true); 
    this->flob = _flob;
    
    ListExpr list = nl->Third(le);
    pos->clear();

    while(!nl->IsEmpty(list)) {
      string attrName = nl->SymbolValue(nl->First(list));
      ListExpr attrType = 0;
      int attrPos = 0;
      ListExpr attrList = nl->Second(nl->Second(le));
      attrPos = listutils::findAttribute(attrList, attrName, attrType);

      if (attrPos == 0) {
          return listutils::typeError
          ("there is no attribute having name " + attrName);
      }
      
      pos->push_back(attrPos);
      list = nl->Rest(list);
    }
    
    while ((tup = rit->GetNextTuple()) != 0) {
      if (flob) {
        tup->bringToMemory();
      }
      tupleSize = tup->GetMemSize();
      if ((size_t)tupleSize<availableMemSize) {
        tup->SetTupleId(mmorel->noEntries()+1);
        TupleWrap tw(tup);
        mmorel->insert(tw,pos);
        usedMainMemory += tupleSize;
        availableMemSize -= tupleSize;
        tup->DeleteIfAllowed();
      } else {
        if(mmorel->isEmpty()){
          cout << "no memory left" << endl;
          tup->DeleteIfAllowed();
          return false;
        }
        cout << "the available main memory is not enough, the object"
                " might be usable but not complete" << endl;
        tup->DeleteIfAllowed();
        break;
      }
    }
    
    delete rit;
    memSize = usedMainMemory;
    objectTypeExpr = 
          nl->ToString(nl->TwoElemList(listutils::basicSymbol<Mem>(),le));
    flob = _flob;
    database = _database;
    return true;
}


bool MemoryORelObject::tupleStreamToORel(Word arg, ListExpr le, ListExpr type,
                                        string _database, bool _flob) {

    Stream<Tuple> stream(arg);
    flob = _flob;
    size_t availableMemSize = catalog->getAvailableMemSize();
    unsigned long usedMainMemory =0;
    Tuple* tup;
    int tupleSize = 0;

    stream.open();

    
    while((tup = stream.request()) != 0){
      if(flob) {
        tup->bringToMemory();
      }
      tupleSize = tup->GetMemSize();
      if((size_t)tupleSize<availableMemSize) {
        tup->SetTupleId(mmorel->noEntries()+1);  
        TupleWrap tw(tup);
        mmorel->insert(tw,pos);
        usedMainMemory += tupleSize;
        availableMemSize -= tupleSize;
        tup->DeleteIfAllowed();
      } else {
        if(mmorel->isEmpty()) {
            cout << "no memory left" << endl;
            return false;
        }
        cout << "the memSize is not enough, the object"
                " might be usable but not complete" <<endl;
        tup->DeleteIfAllowed();
        break;
      }
    }
    memSize = usedMainMemory;
    objectTypeExpr = nl->ToString(nl->TwoElemList(
                        listutils::basicSymbol<Mem>(),
                        nl->ThreeElemList( 
                          nl->SymbolAtom(OREL),
                          nl->Second(le),
                          type)));
    flob = _flob;
    database = _database;

    stream.close();
    return true;
}


Word MemoryORelObject::In( const ListExpr typeInfo, const ListExpr instance,
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
    MemoryORelObject* mmorelObj =
            new MemoryORelObject(nl->ToString(nl->Second(typeInfo)));
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
            mmorelObj->addTuple(tup);
        }
        else {
            correct = false;
        }
    }
    if(!correct){
        result = SetWord(Address(0));
    } else {
        result.addr = mmorelObj;
    }
    return result;
}


ListExpr MemoryORelObject::Out(ListExpr typeInfo, Word value) {
    MemoryORelObject* mmorel = static_cast<MemoryORelObject*>(value.addr);
    if(mmorel->getmmorel()==0 || (mmorel->getmmorel())->isEmpty()) {
      return 0;
    }
    
    ListExpr result = nl->TheEmptyList();
    ListExpr last = result, tupleList = result;
    
    Tuple* t = 0;
    ListExpr tupleTypeInfo = nl->TwoElemList(
      nl->Second(typeInfo),
      nl->IntAtom(nl->ListLength(nl->Second(nl->Second(typeInfo)))));

    ttree::Iterator<TupleWrap,TupleComp> it = mmorel->getmmorel()->begin();
    while (!it.end()){
      t = (*it).getPointer();
      tupleList = t->Out(tupleTypeInfo);
      if (result == nl->TheEmptyList()) {
        result = nl->Cons(tupleList, nl->TheEmptyList());
        last = result;
      } else {
        last = nl->Append(last,tupleList);
      }
      it++;
    }

    return result;
};

bool MemoryORelObject::KindCheck(ListExpr type, ListExpr& errorInfo)
{
    if (nl->ListLength(type)!=2){
            return false;
        }
    ListExpr first = nl->First(type);
    ListExpr second = nl->Second(type);
    return (listutils::isTupleDescription(second) &&
        (nl ->IsEqual(first,BasicType())));
}

const bool MemoryORelObject::checkType(const ListExpr type){
    ListExpr first = nl->First(type);
    ListExpr second = nl->Second(type);
    return (listutils::isTupleDescription(second) &&
        (nl ->IsEqual(first,BasicType())));
        }

Word MemoryORelObject::create(const ListExpr typeInfo) {
    MemoryORelObject* mmorelObject = new MemoryORelObject
        ( nl->ToString(nl->Second(typeInfo)));
    return (SetWord(mmorelObject));
}

bool MemoryORelObject::Save(SmiRecord& valueRecord, size_t& offset,
                            const ListExpr typeInfo, Word& value){

    size_t size = sizeof(int);
    int v = 7;
    bool ok = valueRecord.Write(&v, size, offset);
    offset += size;
    return ok;
};

bool MemoryORelObject::Open (SmiRecord& valueRecord, size_t& offset,
                            const ListExpr typeInfo, Word& value){
    int i;
    size_t size = sizeof(int);
    bool ok = valueRecord.Read(&i, size, offset);
    offset += size;
    if(ok){
        value.addr = new MemoryORelObject();
    } else {
        value.addr = 0;
    }
    return ok;
};

void MemoryORelObject::Close (const ListExpr typeInfo, Word& w){
    MemoryORelObject *k = (MemoryORelObject *)w.addr;
    delete k;
    w.addr = 0;
};

Word MemoryORelObject::Clone (const ListExpr typeInfo, const Word& w){
    MemoryORelObject* m = (MemoryORelObject*) w.addr;
    MemoryORelObject* n = m;
    Word res;
    res.addr = n;
    return res;
};

void* MemoryORelObject::Cast (void* addr){
    return (new (addr) MemoryORelObject);
};

int MemoryORelObject::SizeOfObj(){

    return sizeof(int);
};

void MemoryORelObject::Delete(const ListExpr typeInfo, Word& w){
    MemoryORelObject* memORelO = (MemoryORelObject*)w.addr;
    delete memORelO;
    w.addr = 0;
};


ListExpr MemoryORelObject::Property(){
    return (nl->TwoElemList (
        nl->FourElemList (
            nl->StringAtom("Signature"),
            nl->StringAtom("Example Type List"),
            nl->StringAtom("List Rep"),
            nl->StringAtom("Example List")),
        nl->FourElemList (
            nl->StringAtom("-> SIMPLE"),
            nl->StringAtom(MemoryORelObject::BasicType()),
            nl->StringAtom("(<tuple>*) where <tuple> is (<attr1>...<attrn>)"),
            nl->StringAtom("((\"Meyer\" 5),(\"Muller\" 7))")
            )));
    
}


// MEMORYGRAPHOBJECT ////////////////////////////////////////////////
MemoryGraphObject::MemoryGraphObject() {
  memgraph = new graph::Graph();
  source = -1;
  target = -1;
}

MemoryGraphObject::MemoryGraphObject(graph::Graph* _graph, int _source, 
                                     int _target, size_t _memSize, 
                                     const std::string& _objectTypeExpr, 
                                     bool _flob,
                                     const std::string& _database) {
    memgraph = _graph;
    source = _source;
    target = _target;
    memSize = _memSize;
    objectTypeExpr = _objectTypeExpr;
    flob = _flob;
    database = _database;    
}

MemoryGraphObject::MemoryGraphObject (string _objectTypeExpr) {    
    memgraph = 0;
    source = -1;
    target = -1;
    memSize = 0;
    objectTypeExpr = _objectTypeExpr;
};
  
MemoryGraphObject::~MemoryGraphObject() {
  delete memgraph;
}

graph::Graph* MemoryGraphObject::getgraph() {
  return memgraph;
}


bool MemoryGraphObject::relToGraph(
      GenericRelation* r, 
      ListExpr le,
      string _database, 
      bool _flob) {
    
    GenericRelationIterator* rit;
    rit = r->MakeScan();
    Tuple* tup;
    int tupleSize = 0;
    unsigned long availableMemSize = catalog->getAvailableMemSize();
    unsigned long usedMainMemory=0;
     
    memgraph->clear();  
    flob = _flob;

    // find indexes of integer attributes
    ListExpr attrList = nl->Second(nl->Second(le));
    
    ListExpr rest = attrList;
    int i = 0;
    bool foundSource = false;
    
    while(!nl->IsEmpty(rest)) {
      ListExpr listn = nl->OneElemList(nl->Second(nl->First(rest)));  
      if(listutils::isSymbol(nl->First(listn),CcInt::BasicType())) {    
        
        if(!foundSource) {
          source = i;
          foundSource = true;
        }
        else {
          target = i;
          break;
        }
      }
      rest = nl->Rest(rest);
      i++;
    }
    
    while((tup = rit->GetNextTuple()) != 0) {
      if(flob) 
          tup->bringToMemory();
      
      tupleSize = tup->GetMemSize();
      
      if((size_t)tupleSize < availableMemSize) {
        tup->SetTupleId(memgraph->size()+1);        
        
        memgraph->addEdge(tup,source,target,0.0,0.0);
        tup->DeleteIfAllowed();
        usedMainMemory += tupleSize;
        availableMemSize -= tupleSize;
      }
      else {
        if(memgraph->isEmpty()) {
            cout << "no memory left" << endl;
            tup->DeleteIfAllowed();
            return false;
        }
        cout << "the available main memory is not enough, the object"
                " might be usable but not complete" << endl;
        tup->DeleteIfAllowed();
        break;
      }
    }
    
    delete rit;
    memSize = usedMainMemory;
    objectTypeExpr = nl->ToString(nl->TwoElemList(
                          listutils::basicSymbol<Mem>(),
                          nl->TwoElemList(
                             nl->SymbolAtom(BasicType()),
                             nl->Second(le)
                       )));
    flob = _flob;
    database = _database;
    return true;
}

void MemoryGraphObject::addTuple(Tuple* tup, double cost, double dist){
    
    if(memgraph==0) 
        memgraph = new graph::Graph();

    size_t tupleSize = 0;
    tupleSize = tup->GetMemSize();
    unsigned long availableMemSize = catalog->getAvailableMemSize();
 
    if((size_t)tupleSize<availableMemSize) {
        tup->SetTupleId(memgraph->size()+1);        

        memgraph->addEdge(tup,source,target,cost,dist);
//         tup->IncReference();
        memSize += tupleSize;
        catalog->addToUsedMemSize(tupleSize);
    }
    else 
        cout << "the memSize is not enough, the object"
                " might be usable but not complete" << endl;
}


const bool MemoryGraphObject::checkType(ListExpr type){
  ListExpr first = nl->First(type);
    ListExpr second = nl->Second(type);
    return (listutils::isTupleDescription(second) &&
        (nl ->IsEqual(first,BasicType())));
        
}



}//end namespace mm2algebra
