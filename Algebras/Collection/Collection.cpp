/*
----
This file is part of SECONDO.

Copyright (C) 2019, 
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


#include <string>
#include "CollectionAlgebra.h"


using namespace std;

namespace collection{

/*
3.2 Implementation of class functions

*/


/*
Create a Collection of type (vector, set, multiset or undef) with typeInfo.

*/
  Collection::Collection(const CollectionType type, const ListExpr typeInfo,
                                const int buckets /* = 10 */):
    Attribute(false),
    size(0), hashValue(0), collType(type),
    elemFLOBDataOffset(0), elemCount(0), elemArrayIndex(0),
    firstElemHashValue(0), nextElemHashValue(0),
    elements(0), elementData(0)
  {
#ifdef DEBUGHEAD
cout << "Collection(1)" << endl;
#endif
    SetDefined(false);
    GetIds(elemAlgId, elemTypeId, typeInfo);
    numOfBuckets = buckets;
    if(buckets < 10) {
      numOfBuckets = 10;
    }
    for(int i=0;i<numOfBuckets;i++) {
      firstElemHashValue.Append(-1);
    }
  }


/*
Create a Collection by copying all data from coll.

*/
  Collection::Collection(const Collection& coll, const bool empty /* =false */):
    Attribute(false),
    elemFLOBDataOffset(0), elemCount(0), elemArrayIndex(0),
    firstElemHashValue(0), nextElemHashValue(0), elements(0), elementData(0)
  {
#ifdef DEBUGHEAD
    cout << "Collection(2)" << endl;
#endif
    size = 0;
    elemAlgId = coll.elemAlgId;
    elemTypeId = coll.elemTypeId;
    collType = coll.collType;
    if(empty) {
      SetDefined(false);
      hashValue = 0;
      numOfBuckets = 10;
      for(int i=0;i<numOfBuckets;i++) {
        firstElemHashValue.Append(-1);
      }
    } else {
        SetDefined( coll.IsDefined() );
        hashValue = coll.hashValue;
        numOfBuckets = coll.numOfBuckets;
        elemFLOBDataOffset.copyFrom(coll.elemFLOBDataOffset);
        elemCount.copyFrom(coll.elemCount);
        elemArrayIndex.copyFrom(coll.elemArrayIndex);
        firstElemHashValue.copyFrom(coll.firstElemHashValue);
        nextElemHashValue.copyFrom(coll.nextElemHashValue);
        elements.copyFrom(coll.elements);
        elementData.copyFrom(coll.elementData);
    }
  }



/*
Create a Collection of type.

Only to be used by Create function, since we don't know algebraId and typeId
of our subtype there.

*/
  Collection::Collection(CollectionType type):
    Attribute(false),
    elemAlgId(0), elemTypeId(0), size(0), numOfBuckets(0),
    hashValue(0), collType(type), elemFLOBDataOffset(0),
    elemCount(0), elemArrayIndex(0), firstElemHashValue(0),
    nextElemHashValue(0), elements(0), elementData(0)
  {
#ifdef DEBUGHEAD
cout << "Collection(3)" << endl;
#endif
    SetDefined(false);
  }


  Collection::~Collection() {}

  Word Collection::In(const ListExpr typeInfo, const ListExpr instance,
                      const int errorPos, ListExpr& errorInfo, bool& correct) {
#ifdef DEBUGHEAD
cout << "In" << endl << "    TypeInfo: " << nl->ToString(typeInfo) << endl;
#endif
    Word w = SetWord(Address(0));
    bool defined = true;
    correct = true;
    if(nl->IsAtom(typeInfo) || nl->ListLength(typeInfo)!=2) {
      correct = false;
      return w;
    }
    CollectionType collType = GetCollType(nl->First(typeInfo));
    if( collType == undef ) {
        correct = false;
      return w;
    }

    if ( listutils::isSymbolUndefined(instance) ){
      defined = false;
    } else if( nl->IsAtom(instance) ){
      correct = false;
      return w;
    }

    Collection* coll = new Collection(collType, typeInfo,
                                        nl->ListLength(instance));
    coll->SetDefined( defined );
    if(!defined){
      w.addr = coll;
      return w;
    }

    ListExpr first = nl->TheEmptyList();
    ListExpr rest = instance;
    ListExpr subtypeInfo = nl->Second(typeInfo);
    ListExpr elemList;
    int count;

    correct = true;
    Word elemWord;

    while(!nl->IsEmpty(rest) && correct) {
      first = nl->First(rest);
      rest = nl->Rest(rest);
      if(collType==multiset) {
        if(nl->IsAtom(first) || nl->ListLength(first)!=2) {
          correct = false;
        } else {
          elemList = nl->First(first);
          count = nl->IntValue(nl->Second(first));
        }
      } else {
        elemList = first;
        count = 1;
      }
      if(correct) {
        elemWord = (am->InObj(coll->elemAlgId, coll->elemTypeId))
                       (subtypeInfo, elemList, errorPos, errorInfo, correct);
      }
      if(correct) {
        coll->Insert(static_cast<Attribute*>(elemWord.addr), count);
        (static_cast<Attribute*>(elemWord.addr))->DeleteIfAllowed(true);
      }
    }
    if(correct) {
      coll->Finish();
      w.addr = coll;
    } else {
      coll->DeleteIfAllowed(true);
    }
    return w;
  }


/*
Out function of our collection.

Creates a ListExpr consisting of the ListExpressions returned by the
outfunctions of the elements.

*/
  ListExpr Collection::Out(ListExpr typeInfo, Word value) {
#ifdef DEBUGHEAD
cout << "Out" << endl
     << "    TypeInfo: " << nl->ToString(typeInfo) << endl;
#endif
    Collection* coll = static_cast<Collection*>(value.addr);
    if(!coll->IsDefined()) {
      return nl->SymbolAtom(Symbol::UNDEFINED());
    }
    int size = coll->GetNoUniqueComponents();
    if(size == 0) {
      return nl->TheEmptyList();
    }
    ListExpr subtypeInfo = nl->Second(typeInfo);

    Attribute* elem = coll->GetComponent(0);
    ListExpr elemExpr = (am->OutObj(coll->elemAlgId, coll->elemTypeId))
                            (subtypeInfo, SetWord(elem));

    ListExpr ret;
    if(coll->GetMyCollType() == multiset) {
      ret = nl->OneElemList(nl->TwoElemList(elemExpr,
          nl->IntAtom(coll->GetComponentCount(0))));
    } else {
      ret = nl->OneElemList(elemExpr);
    }
    ListExpr last = ret;
    for(int i=1;i<size;i++) {
      elem->DeleteIfAllowed(true);
      elem = coll->GetComponent(i);
      elemExpr = (am->OutObj(coll->elemAlgId, coll->elemTypeId))
                              (subtypeInfo, SetWord(elem));

      ListExpr app;
      if(coll->GetMyCollType() == multiset) {
        app = nl->TwoElemList(elemExpr,
            nl->IntAtom(coll->GetComponentCount(i)));
      } else {
        app = elemExpr;
      }
      last = nl->Append(last, app);
    }
    if(elem){
      elem->DeleteIfAllowed(true);
    }
    return ret;
  }


/*
Creates an empty, undefined collection without subtype for the Query Processor.

*/
  Word Collection::Create(const ListExpr typeInfo) {

#ifdef DEBUGHEAD
cout << "Create: " << nl->ToString(typeInfo) << endl;
#endif

 if (nl -> IsEmpty(typeInfo))
   {
#ifdef DEBUGHEAD
     cout << "returning Empty Collection" << endl;
#endif
//      cout << "returning Empty Collection" << endl;
     return (SetWord(new Collection(undef))); // create an undefined collection
   }

    Word w = SetWord(Address(0));
    if(nl->ListLength(typeInfo)!=2) {
      cout << "  Statusbericht Create: Liste != 2!" << endl;
      assert(false);
      return w;
    }

    ListExpr collTypeInfo;
    if(nl->IsAtom(nl->First(typeInfo))) {
#ifdef DEBUG
      cout << "  Statusbericht Create: ohne Subtyp!" << endl;
#endif
      collTypeInfo = typeInfo;
    } else {
      collTypeInfo = nl->First(typeInfo);
    }

    CollectionType collType = GetCollType(collTypeInfo);
    if(collType==undef) {
#ifdef DEBUG
      cout << "  Statusbericht Create: undefiniert!" << endl;
#endif
      cout << "  Statusbericht Create: undefiniert!" << endl;
      return w;
    }

    Collection* coll;
    if(nl->IsAtom(nl->First(typeInfo))) {
      coll = new Collection(collType);
    } else {
      coll = new Collection(collType, typeInfo);
    }
    coll->SetDefined(true);
    w.addr = coll;
    return w;
  }


  void Collection::Delete(const ListExpr typeInfo, Word& w) {
#ifdef DEBUGHEAD
cout << "Delete" << endl;
#endif
    delete static_cast<Collection*>(w.addr);
    w.addr = 0;
  }


  void Collection::Close(const ListExpr typeInfo, Word& w) {
#ifdef DEBUGHEAD
cout << "Close" << endl;
#endif
    delete static_cast<Collection*>(w.addr);
    w.addr = 0;
  }


  Word Collection::Clone(const ListExpr typeInfo , const Word& w) {
#ifdef DEBUGHEAD
cout << "Clone" << endl;
#endif
    Collection* c = static_cast<Collection*>(w.addr);
    return SetWord(new Collection(*c));
  }


  bool Collection::KindCheck(ListExpr type, ListExpr& errorInfo) {
#ifdef DEBUGHEAD
cout << "KindCheck"<< endl
     << "  typeInfo: " << nl->ToString(type) << endl;
#endif
    string coll;
  /*
      if(nl->IsAtom(type)) {
      coll = nl->SymbolValue(type);
      if((coll==Vector::BasicType())||(coll==Set::BasicType())||
         (coll==Multiset::BasicType())) {
        return true;
      }
    }
  */
    if((nl->ListLength(type)==2) && (nl->IsAtom(nl->First(type)))) {
      coll = nl->SymbolValue(nl->First(type));
      if((coll==Multiset::BasicType()) || (coll==Vector::BasicType()) ||
         (coll==Set::BasicType())) {
        return am->CheckKind(Kind::DATA(), nl->Second(type), errorInfo);
      }
    }
    return false;
  }


  int Collection::SizeOfObj() {
#ifdef DEBUGHEAD
cout << "SizeObj" << endl;
#endif
    return sizeof(Collection);
  }


  size_t Collection::Sizeof() const {
#ifdef DEBUGHEAD
cout << "Size" << endl;
#endif
    return sizeof(*this);
  }


  int Collection::NumOfFLOBs() const {
#ifdef DEBUGHEAD
cout << "NumOfFLOBs" << endl;
#endif
    return 7;
  }


  Flob* Collection::GetFLOB(const int i) {
#ifdef DEBUGHEAD
cout << "GetFLOB: " << i << endl;
#endif
    assert(i>=0 && i<7);
    switch(i){
      case 0:
        return &elemFLOBDataOffset;
      case 1:
        return &elemCount;
      case 2:
        return &elemArrayIndex;
      case 3:
        return &firstElemHashValue;
      case 4:
        return &nextElemHashValue;
      case 5:
        return &elements;
      default: //i == 6
        return &elementData;
    }
  }


  bool Collection::Open(SmiRecord& valueRecord, size_t& offset,
                         const ListExpr typeInfo, Word& value) {
#ifdef DEBUGHEAD
cout << "Open" << endl;
#endif
    Collection* coll = static_cast<Collection*>(Attribute::Open(valueRecord,
                                                offset, nl->First(typeInfo)));
    value = SetWord(coll);
    return true;
  }


  bool Collection::Save(SmiRecord& valueRecord, size_t& offset,
                         const ListExpr typeInfo, Word& value) {
#ifdef DEBUGHEAD
cout << "Save: " << nl->ToString(typeInfo) << endl;
#endif
    Collection* coll = static_cast<Collection*>(value.addr);
    Attribute::Save(valueRecord, offset, nl->First(typeInfo), coll);
    return true;
  }

/*NOCH ZU MACHEN:

Collection* Delete(Attribute* elem) const;

Collection* Union(Collection* coll) const;

Collection* Intersection(Collection* coll) const;

Collection* Difference(Collection* coll) const;*/




/*
Returns -1 if this is less than arg, 0 if arg = this and 1 if this is more than
arg.

*/

  int Collection::Compare(const Attribute* arg) const {
#ifdef DEBUGHEAD
    cout << "Compare" << endl;
#endif
    Collection* collToCompare = (Collection*)arg;

    assert(collType == collToCompare->collType);

    if(!IsDefined() && !collToCompare->IsDefined()){
      return 0;
    }else if(!IsDefined()){
      return -1;
    }else if(!collToCompare->IsDefined()){
      return 1;
    }

    if(collToCompare->GetNoComponents() != GetNoComponents()){
      return   collToCompare->GetNoComponents() > GetNoComponents()
                  ? -1 : 1;
    }else if(collToCompare->GetNoUniqueComponents() != GetNoUniqueComponents()){
      return   collToCompare->GetNoComponents() > GetNoComponents() ? -1 : 1;
    }else{
      int countCompareElem = 0;
      int countOwnElem = 0;
      int compareResult;

      for(int eCnt = 0; eCnt < GetNoUniqueComponents(); eCnt++){
          countCompareElem = collToCompare->GetComponentCount(eCnt);
          countOwnElem = GetComponentCount(eCnt);

          Attribute* elem1 = GetComponent(eCnt);
          Attribute* elem2 = collToCompare->GetComponent(eCnt);
          compareResult = elem1->Compare(elem2);
          elem1->DeleteIfAllowed(true);
          elem2->DeleteIfAllowed(true);
          if(compareResult != 0){
              return compareResult;
          }
          if(countCompareElem != countOwnElem){
              return countCompareElem > countOwnElem ? 1 : -1;
          }
      }
    }
    return 0;
  }


  size_t Collection::HashValue() const {
#ifdef DEBUGHEAD
cout << "HashValue" << endl;
#endif
    return hashValue;
  }


  void Collection::Sort() {
#ifdef DEBUGHEAD
cout << "Sort" << endl;
#endif
    SortMerge(0, GetNoUniqueComponents()-1);
  }


  void Collection::CopyFrom(const Attribute* right) {
#ifdef DEBUGHEAD
cout << "CopyFrom" << endl;
#endif
    const Collection* coll = static_cast<const Collection*>(right);
    this->collType = coll->collType;
    this->elemAlgId = coll->elemAlgId;
    this->elemTypeId = coll->elemTypeId;
    this->SetDefined( coll->IsDefined() );
    if( !coll->IsDefined() ){
      this->Clear();
    } else {
      this->size = coll->size;
      this->numOfBuckets = coll->numOfBuckets;
      this->hashValue = coll->hashValue;
      this->elemFLOBDataOffset.copyFrom(coll->elemFLOBDataOffset);
      this->elemCount.copyFrom(coll->elemCount);
      this->elemArrayIndex.copyFrom(coll->elemArrayIndex);
      this->firstElemHashValue.copyFrom(coll->firstElemHashValue);
      this->nextElemHashValue.copyFrom(coll->nextElemHashValue);
      this->elements.copyFrom(coll->elements);
      this->elementData.copyFrom(coll->elementData);
    }
  }


  bool Collection::Adjacent(const Attribute* arg) const {
#ifdef DEBUGHEAD
cout << "Adjacent" << endl;
#endif
    return false;
  }


  Collection* Collection::Clone() const {
#ifdef DEBUGHEAD
cout << "Clone" << endl;
#endif
    return new Collection(*this);
  }


  void Collection::Finish() {
#ifdef DEBUGHEAD
cout << "Finish" << endl;
#endif
      SetDefined(true);
      if(collType != vector) {
        Sort();
      }
  }


  ostream& Collection::Print( ostream& os ) const {
#ifdef DEBUGHEAD
cout << "Print" << endl;
#endif
    os << "Collection: ";
    switch(collType){
      case vector:
        os << Vector::BasicType();
        break;
      case set:
        os << Set::BasicType();
        break;
      case multiset:
        os << Multiset::BasicType();
        break;
      case undef:
        os << "undef";
        break;
      default:
        os << "ERROR" << endl;
        return (os);
    }
    os << "(" << elemAlgId << "," << elemTypeId << "): ";
    os << "\t" << (IsDefined() ? "defined." : "UNDEFINED." ) << endl;
    os << "Number of contained Elements: " << GetNoComponents() << " ("
       << GetNoUniqueComponents() << " unique)." << endl;
    os << "--- Elements: begin ---" << endl;
    Attribute* elem;
    int times;
    for(int i=0; i<GetNoUniqueComponents(); i++){
      elem = GetComponent(i);
      os << i << ":\t";
      elem->Print(os);
      if(collType == multiset){
        times = GetComponentCount(i);
        os << "(contained " << times << " times).";
      }
      os << endl;
      elem->DeleteIfAllowed(true);
    }
    os << "--- Elements: end ---" << endl;
#ifdef DEBUG
    os << "\tContains size = " << size << " elements." << endl;
    os << "\tUses numOfBuckets = " << numOfBuckets << " buckets." << endl;
    os << "\thashValue = " << hashValue << endl;
    os << "\t\telemCount.Size() = " << elemCount.Size() << endl;
    os << "\t\telemArrayIndex.Size() = " << elemArrayIndex.Size() << endl;
    os << "\t\tfirstElemHashValue.Size() = " << firstElemHashValue.Size()<<endl;
    os << "\t\tnextElemHashValue.Size() = " << nextElemHashValue.Size() << endl;
    os << "\t\telemFLOBDataOffset.Size() = " <<elemFLOBDataOffset.Size()<< endl;
    os << "\t\telements.getSize() = " << elements.getSize() << endl;
    os << "\t\telementData.getSize() = " << elementData.getSize() << endl;
#endif
    return (os);
  }


  void* Collection::Cast(void* addr) {
#ifdef DEBUGHEAD
//cout << "Cast" << endl;
#endif
    return (new (addr) Collection);
  }


  void Collection::Insert(Attribute* elem, const int count) {
#ifdef DEBUGHEAD
cout << "Insert" << endl;
#endif
    int index = GetIndex(elem);
    if(index > -1) {
      if(collType==vector) {
        for(int i=0;i<count;i++) {
          elemArrayIndex.Append(index);
        }
        AddHashValue((static_cast<Attribute*>(elem))->HashValue(),count);
      } else if (collType==multiset) {
        int cnt;
        elemCount.Get(index, &cnt);
        elemCount.Put(index, (cnt+count));
        size += count;
        AddHashValue((static_cast<Attribute*>(elem))->HashValue(),
                              count);
      }
      //there's nothing to do if we've got a set since in a set
      //every element only appears once.
      return;
    }
    SaveComponent(elem, count);
  }


  CollectionType Collection::GetMyCollType() const {
#ifdef DEBUGHEAD
cout << "GetMyCollType" << endl;
#endif
    return collType;
  }


  int Collection::Contains(const Attribute* elem) const {
#ifdef DEBUGHEAD
cout << "Contains" << endl;
#endif
    int index = GetIndex(elem);
    if(index > -1) {
      if(collType == multiset) {
        int count;
        elemCount.Get(index, &count);
        return count;
      } else {
        return 1;
      }
    }
    return 0;
  }


  Attribute* Collection::GetComponent(int pos) const {
#ifdef DEBUGHEAD
cout << "GetComponent" << endl;
#endif
    assert(0<=pos);
    assert(pos<elemArrayIndex.Size());
    int index;
    elemArrayIndex.Get(pos, &index);
    return RestoreComponent(index);
  }


  int Collection::GetComponentCount(const int pos) const {
#ifdef DEBUGHEAD
cout << "GetComponentCount" << endl;
#endif
    assert(0<=pos);
    assert(pos<elemArrayIndex.Size());
    if(collType!=multiset) {
      return 1;
    }
    int index;
    elemArrayIndex.Get(pos, &index);
    int count;
    elemCount.Get(index, &count);
    return count;
  }


  int Collection::GetNoComponents() const {
#ifdef DEBUGHEAD
cout << "GetNo" << endl;
#endif
    if(collType == undef) {
      return 0;
    }
    if(collType==multiset) {
      return size;
    }
    return elemArrayIndex.Size();
  }


  int Collection::GetNoUniqueComponents() const {
    //needed for iterations with GetComponent(i) since for a multiset
    //GetNoComponents returns the number of all components, including
    //duplicates, but GetComponent doesn't regard duplicates.
#ifdef DEBUGHEAD
cout << "GetNoUnique" << endl;
#endif
    if(collType==undef) {
      return 0;
    }
    return elemArrayIndex.Size();
  }


  Collection* Collection::Union(const Collection* coll) const {
#ifdef DEBUGHEAD
cout << "Union" << endl;
#endif
    return new Collection();
  }


  Collection* Collection::Intersection(const Collection* coll) const {
#ifdef DEBUGHEAD
cout << "Intersection" << endl;
#endif
    return new Collection();
  }


  Collection* Collection::Difference(const Collection* coll) const {
#ifdef DEBUGHEAD
cout << "Difference" << endl;
#endif
    return new Collection();
  }


  Collection* Collection::Delete(const Attribute* elem) const {
#ifdef DEBUGHEAD
cout << "Delete" << endl;
#endif
    return new Collection();
  }




  void Collection::GetIds(int& algebraId, int& typeId,
                              const ListExpr typeInfo) {
#ifdef DEBUGHEAD
cout << "GetIds" << endl;
cout << "  " << nl->ToString(typeInfo) << endl;
#endif
    if(nl->IsAtom(typeInfo) || nl->ListLength(typeInfo)!=2) {
      return;
    }
    ListExpr subtypeInfo = nl->Second(typeInfo);
    if(nl->IsAtom(subtypeInfo)) {
      return;
    }

    ListExpr b1 = nl->First(subtypeInfo);
    if(nl->IsAtom(b1)) {
    //subtypeInfo = type = (algId ...)
      if (nl->ListLength(subtypeInfo)!=2) {
        return;
      }
      //list = (algid typeid)
      algebraId = nl->IntValue(nl->First(subtypeInfo));
      typeId = nl->IntValue(nl->Second(subtypeInfo));
    } else {
    //subtypeInfo = (type1 type2).
    //We only need type1 since a collection can only be of one type.
    //type1 is b1 (nl->First(subtypeInfo)), so b1 is (algId typeId).
      if (nl->ListLength(b1)!=2) {
        return;
      }
      //b1 = (algId typeId)
      algebraId = nl->IntValue(nl->First(b1));
      typeId = nl->IntValue(nl->Second(b1));
    }
  }


  void Collection::SortMerge(const int start, const int end) {
#ifdef DEBUGHEAD
cout << "SortMerge" << endl;
#endif
    if(start>=end) {
      return;
    }
    if(collType==vector) {
      return;
    }
    int middle = (start+end-1)/2;
    SortMerge(start, middle);
    SortMerge(middle+1, end);
    int pointer1 = start;
    int pointer2 = middle+1;
    int* help = new int[((end-start)+1)];
    int index;
    int pointer3 = 0;
    Attribute* elem1 = GetComponent(pointer1);
    Attribute* elem2 = GetComponent(pointer2);
    bool finished = false;
    while(!finished) {
      int compare = elem1->Compare(elem2);
      if(compare>0) {
        elemArrayIndex.Get(pointer2, &index);
        pointer2++;
        if(pointer2<=end) {
          elem2->DeleteIfAllowed(true);
          elem2 = GetComponent(pointer2);
        } else {
          finished = true;
        }
      } else {
        elemArrayIndex.Get(pointer1, &index);
        pointer1++;
        if(pointer1<=middle) {
          elem1->DeleteIfAllowed(true);
          elem1 = GetComponent(pointer1);
        } else {
          finished = true;
        }
      }
      help[pointer3] = index;
      pointer3++;
    }
    while(pointer1 <= middle) {
      elemArrayIndex.Get(pointer1, &index);
      help[pointer3] = index;
      pointer1++;
      pointer3++;
    }
    while(pointer2 <= end) {
      elemArrayIndex.Get(pointer2, &index);
      help[pointer3] = index;
      pointer2++;
      pointer3++;
    }
    pointer3 = 0;
    pointer1 = start;
    while(pointer1 <= end) {
      elemArrayIndex.Put(pointer1, help[pointer3]);
      pointer1++;
      pointer3++;
    }
    delete[] help;
    if(elem1){
       elem1->DeleteIfAllowed(true);
    }
    if(elem2){
      elem2->DeleteIfAllowed(true);
    }
  }

  CollectionType Collection::GetCollType(const ListExpr collTypeInfo) {
#ifdef DEBUGHEAD
cout << "GetCollType" << endl;
#endif
    if(nl->ListLength(collTypeInfo) != 2) {
      return undef;
    }
    if(!nl->IsAtom(nl->First(collTypeInfo))
        || !(nl->AtomType(nl->First(collTypeInfo)) == IntType)
        || !nl->IsAtom(nl->Second(collTypeInfo))
        || !(nl->AtomType(nl->Second(collTypeInfo)) == IntType)) {
      return undef;
    }
    int algId = nl->IntValue(nl->First(collTypeInfo));
    int typeId = nl->IntValue(nl->Second(collTypeInfo));
    string collType = am->GetTC(algId, typeId)->Name();
    if(collType==Vector::BasicType()) {
      return vector;
    } else if(collType==Set::BasicType()) {
      return set;
    } else if(collType==Multiset::BasicType()) {
      return multiset;
    }
    return undef;
  }


  void Collection::SaveComponent(Attribute* elem, const int count) {
#ifdef DEBUGHEAD
cout << "SaveComponent" << endl;
#endif

    // step1 backup original flob data from elem into 
    std::vector<Flob> origFlobs;
    for(int i=0;i<elem->NumOfFLOBs();i++){
       Flob* tmpFlob = elem->GetFLOB(i);
       origFlobs.push_back(*tmpFlob);
    }

    SmiFileId  fid = elementData.getFileId();
    SmiRecordId rid = elementData.getRecordId();
    char mode = elementData.getMode();


    // save FLOBs
    size_t offset = elementData.getSize();
    size_t index = elemFLOBDataOffset.Size();
    
    elemFLOBDataOffset.Append(offset);
    for(int i=0;i<elem->NumOfFLOBs();i++) {
      Flob* tempFLOB = elem->GetFLOB(i);
      size = tempFLOB->getSize();
      elementData.resize(offset+size);
      char data[size];
      tempFLOB->read(data, size, 0);
      elementData.write(data, size, offset);
      offset += size;
      // change flob in Elem
      Flob changedFlob = Flob::createFrom(fid,rid,offset, mode, size);
      *elem->GetFLOB(i) = changedFlob;
    }


    // store elements
    size_t size = (size_t)(am->SizeOfObj(elemAlgId, elemTypeId))();
    offset = index*size;
    if(elements.getSize()<(offset+size)) {
      if(elements.getSize()<(8*size)) {
        elements.resize(8*size);
      } else {
        elements.resize(elements.getSize()*2);
      }
    }
    elements.write( (char*)(elem), size, offset );

    // restore flob
    for(int i=0;i<elem->NumOfFLOBs();i++){
       *elem->GetFLOB(i)  = origFlobs[i];
    }




    if(collType == multiset) {
      assert((size_t)elemCount.Size()==index);
      elemCount.Append(count);
      this->size += count;
    }
    elemArrayIndex.Append(index);
    if(collType == vector && count>1) {
      for(int i=0;i<(count-1);i++) {
        elemArrayIndex.Append(index);
      }
    }
    if(collType==set) {
      AddHashValue((static_cast<Attribute*>(elem))->HashValue(), 1);
    } else {
      AddHashValue((static_cast<Attribute*>(elem))->HashValue(), count);
    }
    InsertIndex(elem, index);
  }


  Attribute* Collection::RestoreComponent(const int pos) const {
#ifdef DEBUGHEAD
cout << "RestoreComponent" << endl;
#endif
    assert(pos>=0);
    assert(pos<elemFLOBDataOffset.Size());

    ListExpr typeInfo = nl->TwoElemList(nl->IntAtom(elemAlgId),
                                        nl->IntAtom(elemTypeId));
    Attribute* elem = static_cast<Attribute*>
          ((am->CreateObj(elemAlgId, elemTypeId))(typeInfo).addr);


    /*
    size_t n1 = elem->Sizeof();
    size_t n2 = am->SizeOfObj(elemAlgId, elemTypeId)();
    cout << "n1 = " << n1 << " n2 =" << n2 << endl;
    assert( n1 == n2);
    */

    std::vector<Flob> ff; /* freshflobs */
    for(int i=0;i<elem->NumOfFLOBs();i++) {
      ff.push_back( *elem->GetFLOB(i) );
    }

    size_t offset = (size_t)(pos*(am->SizeOfObj(elemAlgId, elemTypeId))());
    elements.read((char*)(elem), elem->Sizeof(), offset);
    elem = static_cast<Attribute*>((am->Cast(elemAlgId, elemTypeId))(elem));

    for(int i=0;i<elem->NumOfFLOBs();i++) {
      SmiSize fs = elem->GetFLOB(i)->getSize();
      *elem->GetFLOB(i) = ff[i];
      elem->GetFLOB(i)->resize(fs);
    }
    size_t temp = 0;
    elemFLOBDataOffset.Get(pos, &temp);
    offset = temp;
    for(int i=0;i<elem->NumOfFLOBs();i++) {
      Flob* tempFLOB = elem->GetFLOB(i);
      size_t size = tempFLOB->getSize();

//#define X1_DEBUG
#ifdef X1_DEBUG
cout << "  offset: " << offset << endl;
cout << "  size(" << i << "): " << size << endl;
#endif
      char tempData[size];
      elementData.read(tempData, size, offset);
      tempFLOB->write(tempData, size, 0);
      offset += size;
    }

    return elem;
  }


  int Collection::GetIndex(const Attribute* elem) const {
#ifdef DEBUGHEAD
cout << "GetIndex" << endl;
#endif
    size_t tmp = (static_cast<const Attribute*>(elem))->HashValue();
    int hashsum = (int) tmp;
    hashsum = hashsum % numOfBuckets;
    if(hashsum<0) {
      hashsum += numOfBuckets;
    }
    int index = -1;
    firstElemHashValue.Get(hashsum, &index);
#ifdef DEBUG
cout << "  Statusbericht GetIndex-Funktion:" << endl
     << "    Hashsum: " << hashsum << endl
     << "    Index: " << index << endl;
#endif
    while(index>-1) {
      Attribute* elem2 = RestoreComponent(index);
      int comp = elem->Compare(elem2);
      elem2->DeleteIfAllowed(true);
      if(comp==0) {
        return index;
      }
      nextElemHashValue.Get(index, &index);
#ifdef DEBUG
cout << "  Statusbericht GetIndex-Funktion:" << endl
     << "    Index: " << index << endl;
#endif
    }
    return -1;
  }

  void Collection::InsertIndex(const Attribute* elem, const int index) {
#ifdef DEBUGHEAD
cout << "InsertIndex" << endl;
#endif
    assert(nextElemHashValue.Size()==index);
      //nextElemHashValue(index) must not exist!
    int hashsum = (static_cast<const Attribute*>(elem))->HashValue();
    hashsum = hashsum % numOfBuckets;
    if(hashsum<0) {
      hashsum += numOfBuckets;
    }
    int index2 = -1;
    firstElemHashValue.Get(hashsum, &index2);
#ifdef DEBUG
cout << "  Statusbericht InsertIndex-Funktion" << endl
     << "    Hashsum: " << hashsum << endl
     << "    Index: " << index2 << endl;
#endif
    if(index2<0) {
      firstElemHashValue.Put(hashsum, index);
    } else {
      int indexOld(0);
      while(index2>-1) {
#ifdef DEBUG
cout << "  Statusbericht InsertIndex-Funktion" << endl
     << "    Index: " << index2 << endl;
#endif
        indexOld = index2;
        nextElemHashValue.Get(indexOld, &index2);
      }
      nextElemHashValue.Put(indexOld, index);
    }
    nextElemHashValue.Put(index, -1);
  }


  void Collection::AddHashValue(const int value, const int count) {
    hashValue += count*value;
  }

  void Collection::Clear() {
    elemFLOBDataOffset.clean();
    elemCount.clean();
    elemArrayIndex.clean();
    firstElemHashValue.clean();
    nextElemHashValue.clean();
    elements.clean();
    elementData.clean();
    size = 0;
    hashValue = 0;
    numOfBuckets = 10;
    firstElemHashValue.resize(numOfBuckets);
    for(int i=0;i<numOfBuckets;i++) {
        firstElemHashValue.Append(-1);
    }
  }

}

