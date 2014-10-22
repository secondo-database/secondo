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

//paragraph [1] Title: [{\Large \bf \begin{center}] [\end{center}}]
//characters [1] Type: [] []
//characters [2] Type: [] []
//[ae] [\"{a}]
//[oe] [\"{o}]
//[ue] [\"{u}]
//[ss] [{\ss}]
//[Ae] [\"{A}]
//[Oe] [\"{O}]
//[Ue] [\"{U}]
//[x] [$\times $]
//[->] [$\rightarrow $]
//[toc] [\tableofcontents]

[1] Implementation of datatypes vector, set and multiset and operators.

[1] Using Storage Manager Berkeley DB

December 2007 Ingmar G[oe]hr, Nicolai Voget, Peter Spindler, Sascha Bergmann

[toc]

1 Collection class implementation

1.1 Overview

1.1.1 Types

This Algebra provides three different types: vector, set, multiset.

All types are implemented in the class Collection, since they are all nearly
the same.

All types can be created from any number of elements as long as the elements
are all of the same type, which has to be from Kind DATA.

Since vector, set and multisets are of Kind DATA themselves, it is possible
to create a vector of vectors of integers for example.

In a vector, the elements are saved as they are added.

In a set, every element is only saved once, so duplicates are being removed.
Also, the elements will be saved ordered, smallest element first, using the
compare function of the elements. This implies that the type of which a vector,
set or multiset should be created has to implement a compare function.

A multiset is nearly the same as a set with the difference, that an element can
be added more than once.


1.1.2 Operators

Following operators are defined:

  * create\_vector

  * create\_set

  * create\_multiset

    (t)[*] [->] "vector(t)"[1]

    (t)[*] [->] "set(t)"[1]

    (t)[*] [->] "multiset(t)"[1]

t has to be of Kind DATA.


  * insert

    set(t) [x] t [->] "set(t)"[1]

    multiset(t) [x] t [->] "multiset(t)"[1]

If set(t) already contains the element to be inserted, the result is the
original set.


  * + (append)

    vector(t) [x] t [->] "vector(t)"[1]


  * delete

    set(t) [x] t [->] "set(t)"[1]

    multiset(t) [x] t [->] "multiset(t)"[1]

Removes the given element from the set/multiset. If the set/multiset does'nt
contain the element, the result is the original set/multiset. If the multiset
contains the element n times, the result contains the element (n-1) times.


  * contains

    vector(t) [x] t [->] "bool"[1]

    set(t) [x] t [->] "bool"[1]

    multiset(t) [x] t [->] CcInt::BasicType()[1]

Returns whether the collection contains the element. If the collection is a
multiset, the count of the element is returned.


  * in

    t [x] vector(t) [->] "bool"[1]

    t [x] set(t) [->] "bool"[1]

    t [x] multiset [->] CcInt::BasicType()[1]

Same as contains.


  * union

  * intersection

  * difference

    set(t) [x] set(t) [->] "set(t)"[1]

    multiset(t) [x] multiset(t) [->] "multiset(t)"[1]


  * concat

    vector(t) [x] vector(t) [->] "vector(t)"[1]

Appends the elements of the second vector to the first vector.


  * \verb+<+ (proper subset)

  * \verb+<=+ (subset)

  * = (equals)

  * \verb+>=+ (superset)

  * \verb+>+ (proper superset)

    set(t) [x] set(t) [->] "bool"[1]

    multiset(t) [x] multiset(t) [->] "bool"[1]


  * get

    vector(t) [x] int [->] "t"[1]

Returns the nth element of vector. If n is less than 0 or greater than size(
vector), an undefined element is returned.


  * components

    vector(t) [->] "stream(t)"[1]

    set(t) [->] "stream(t)"[1]

    multiset(t) [->] "stream(t)"[1]


  * collect\_vector

  * collect\_set

  * collect\_multiset

    stream(t) [->] "vector(t)"[1]

    stream(t) [->] "set(t)"[1]

    stream(t) [->] "multiset(t)"[1]

t has to be of Kind DATA.


  * size

    vector(t) [->] CcInt::BasicType()[1]

    set(t) [->] CcInt::BasicType()[1]

    multiset(t) [->] CcInt::BasicType()[1]

Returns the number of elements contained in the collection.


  * is\_defined

    vector(t) [->] "bool"[1]

    set(t) [->] "bool"[1]

    multiset(t) [->] "bool"[1]

Necessary to difference between an empty and an undefined collection.

2 Includes

*/


//#define DEBUG
//#define DEBUGHEAD

#include "CollectionAlgebra.h"
#include "ListUtils.h"
#include "Symbols.h"

using namespace std;

using namespace collection;
/*
3.2 Implementation of class functions

*/


/*
Create a Collection of type (vector, set, multiset or undef) with typeInfo.

*/
  Collection::Collection(const CollectionType type, const ListExpr typeInfo,
                                const int buckets /* = 10 */):
    ::Attribute(false),
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
    ::Attribute(false),
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
    ::Attribute(false),
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
cout << "Cast" << endl;
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
    size_t index = elemFLOBDataOffset.Size();
    size_t size = (size_t)(am->SizeOfObj(elemAlgId, elemTypeId))();
    size_t offset = index*size;
    if(elements.getSize()<(offset+size)) {
      if(elements.getSize()<(8*size)) {
        elements.resize(8*size);
      } else {
        elements.resize(elements.getSize()*2);
      }
    }
    elements.write( (char*)(elem), size, offset );

    offset = elementData.getSize();
    elemFLOBDataOffset.Append(offset);
    for(int i=0;i<elem->NumOfFLOBs();i++) {
      Flob* tempFLOB = elem->GetFLOB(i);
      size = tempFLOB->getSize();
      elementData.resize(offset+size);
      char data[size];
      tempFLOB->read(data, size, 0);
      elementData.write(data, size, offset);
      offset += size;
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

/*
3.3 Implementation of Property functions and TypeConstructors

*/
  ListExpr VectorProperty() {
#ifdef DEBUGHEAD
cout << "VectorProperty" << endl;
#endif
    return (nl->TwoElemList(
            nl->FiveElemList(nl->StringAtom("Signature"),
                             nl->StringAtom("Example Type List"),
                             nl->StringAtom("List Rep"),
                             nl->StringAtom("Example List"),
                             nl->StringAtom("Remarks")),
            nl->FiveElemList(nl->StringAtom("-> DATA"),
                             nl->StringAtom("("+Vector::BasicType()+" real)"),
                             nl->StringAtom("(elem1 elem2 .. elem_n)"),
                             nl->StringAtom("(2.839 25.123 3.12 481.2)"),
                             nl->StringAtom("All elements must be of the"
                             "same type."))));
  }

  ListExpr SetProperty() {
#ifdef DEBUGHEAD
cout << "SetProperty" << endl;
#endif
    return (nl->TwoElemList(
            nl->FiveElemList(nl->StringAtom("Signature"),
                             nl->StringAtom("Example Type List"),
                             nl->StringAtom("List Rep"),
                             nl->StringAtom("Example List"),
                             nl->StringAtom("Remarks")),
            nl->FiveElemList(nl->StringAtom("-> DATA"),
                             nl->StringAtom("("+Set::BasicType()+" real)"),
                             nl->StringAtom("(elem1 elem2 .. elem_n)"),
                             nl->StringAtom("(2.839 3.12 25.123 481.2)"),
                             nl->StringAtom("All elements must be of the"
                             "same type."))));
  }

  ListExpr MultisetProperty() {
#ifdef DEBUGHEAD
cout << "MultisetProperty" << endl;
#endif
    return (nl->TwoElemList(
            nl->FiveElemList(nl->StringAtom("Signature"),
                             nl->StringAtom("Example Type List"),
                             nl->StringAtom("List Rep"),
                             nl->StringAtom("Example List"),
                             nl->StringAtom("Remarks")),
            nl->FiveElemList(nl->StringAtom("-> DATA"),
                             nl->StringAtom("("+Multiset::BasicType()+" real)"),
                             nl->StringAtom("((elem1 count1) .. "
                             "(elem_n count_n))"),
                             nl->StringAtom("((2.839 2) (3.12 1) (25.123 1))"),
                             nl->StringAtom("All elements must be of the"
                             "same type."))));
  }


  TypeConstructor vectorTC(
      Vector::BasicType(), VectorProperty,
      Collection::Out, Collection::In,
      0, 0,
      Collection::Create, Collection::Delete,
      Collection::Open, Collection::Save,
      Collection::Close, Collection::Clone,
      Collection::Cast, Collection::SizeOfObj,
      Collection::KindCheck);

  TypeConstructor setTC(
      Set::BasicType(), SetProperty,
      Collection::Out, Collection::In,
      0, 0,
      Collection::Create, Collection::Delete,
      Collection::Open, Collection::Save,
      Collection::Close, Collection::Clone,
      Collection::Cast, Collection::SizeOfObj,
      Collection::KindCheck);

  TypeConstructor multisetTC(
      Multiset::BasicType(), MultisetProperty,
      Collection::Out, Collection::In,
      0, 0,
      Collection::Create, Collection::Delete,
      Collection::Open, Collection::Save,
      Collection::Close, Collection::Clone,
      Collection::Cast, Collection::SizeOfObj,
      Collection::KindCheck);



/*
4 Implementation of operators

4.1 Implementation of contains and in

*/
  template<bool contains> ListExpr ContainsInTypeMap(ListExpr args) {

    ListExpr errorInfo = nl->OneElemList(nl->SymbolAtom("ERROR"));
#ifdef DEBUGHEAD
cout << "ContainsTypeMapping:" << nl->ToString(args) << endl;
#endif
    string opName = (contains?"contains":"in");
    string pos1 = (contains?"first":"second");
    string pos2 = (contains?"second":"first");
    if(nl->ListLength(args)!=2) {
      ErrorReporter::ReportError("Operator " + opName + " expects a list of"
                                  + " length two.");
      return nl->TypeError();
    }
    ListExpr coll = (contains?nl->First(args):nl->Second(args));
    ListExpr elem = (contains?nl->Second(args):nl->First(args));
    if(!Collection::KindCheck(coll, errorInfo)) {
      ErrorReporter::ReportError("Operator " + opName + " expects a vector, "
                                + "set or multiset as " + pos1 + " argument.");
      return nl->TypeError();
    }
    if(!nl->Equal(nl->Second(coll), elem)) {
      ErrorReporter::ReportError("The " + pos2 + " argument of operator "
                                + opName + " has to be of the same type as the"
                                + " elements of the " + pos1 + " argument.");
      return nl->TypeError();
    }
    string type = nl->SymbolValue(nl->First(coll));
    if((type==Vector::BasicType())||(type==Set::BasicType())) {
      return nl->SymbolAtom(CcBool::BasicType());
    } else if(type==Multiset::BasicType()) {
      return nl->SymbolAtom(CcInt::BasicType());
    }
    ErrorReporter::ReportError("Operator " + opName + " expects a vector"
                                + ", set or multiset as " + pos1
                                + " argument.");
    return nl->TypeError();
  }

  template<bool contains> int ContainsInValueMap(Word* args, Word& result,
                                     int message, Word& local, Supplier s) {
#ifdef DEBUGHEAD
cout << "ContainsInValueMap" << endl;
#endif
    result = qp->ResultStorage(s);
    Collection* coll;
    Attribute* elem;
    if(contains) {
      coll = static_cast<Collection*>(args[0].addr);
      elem = static_cast<Attribute*>(args[1].addr);
    } else {
      elem = static_cast<Attribute*>(args[0].addr);
      coll = static_cast<Collection*>(args[1].addr);
    }

    if(!coll->IsDefined()){
      (static_cast<Attribute*>(result.addr))->SetDefined(false);
      return 0;
    }

    int contained = coll->Contains(elem);
    if(coll->GetMyCollType()==collection::multiset) {
      (static_cast<CcInt*>(result.addr))->Set(true, contained);
    } else {
      (static_cast<CcBool*>(result.addr))->Set(true, (contained==1));
    }
    return 0;
  }

  struct containsInfo : OperatorInfo {

    containsInfo()
    {
      name      = "contains";
      signature = Vector::BasicType() + "(t) x t -> " + CcBool::BasicType();
      appendSignature(Set::BasicType() + "(t) x t -> " + CcBool::BasicType());
      appendSignature(Multiset::BasicType() + "(t) x t -> "+CcInt::BasicType());
      syntax    = "_ contains _";
      meaning   = "Contains predicate.";
    }

  };

  struct inInfo : OperatorInfo {

    inInfo()
    {
      name      = "in";
      signature = "t x "+Vector::BasicType() + "(t) -> " + CcBool::BasicType();
      appendSignature("t x "+Set::BasicType() + "(t) -> "+CcBool::BasicType());
      appendSignature("t x "+Multiset::BasicType()+"(t) -> "+
                                                   CcBool::BasicType());
      syntax    = "_ in _";
      meaning   = "Inverted contains predicate.";
    }

  };

/*
4.2 Implementation of add and insert

*/
  template<bool insert> ListExpr InsertTypeMap(ListExpr args) {
    ListExpr errorInfo = nl->OneElemList(nl->SymbolAtom("ERROR"));
#ifdef DEBUGHEAD
cout << "InsertTypeMapping:" << nl->ToString(args) << endl;
#endif
    string opName = (insert?"insert":"+");
    if(nl->ListLength(args)!=2) {
      ErrorReporter::ReportError("Operator " + opName + " expects a list of"
                                  + " length two.");
      return nl->TypeError();
    }
    ListExpr coll = nl->First(args);
    ListExpr elem = nl->Second(args);
    if(!Collection::KindCheck(coll, errorInfo)) {
      if(insert) {
        ErrorReporter::ReportError("Operator " + opName + " expects a "
                                  + "set or multiset as first argument.");
      } else {
        ErrorReporter::ReportError("Operator " + opName + " expects a "
                                  + "vector as first argument.");
      }
      return nl->TypeError();
    }
    if((nl->ListLength(coll)!=2) || !nl->Equal(nl->Second(coll), elem)) {
      ErrorReporter::ReportError("The second argument of operator "
                                + opName + " has to be of the same type as the"
                                + " elements of the first argument.");
      return nl->TypeError();
    }
    string type = nl->SymbolValue(nl->First(coll));
    if((type==Vector::BasicType())==(!insert)) {
      return coll;
    }
    if(insert) {
      ErrorReporter::ReportError("Operator 'insert' expects a "
                                "set or multiset as first argument.");
    } else {
      ErrorReporter::ReportError("Operator '+' expects a "
                                 "vector as first argument.");
    }
    return nl->TypeError();
  }

  template<bool insert> int InsertValueMap(Word* args, Word& result,
                                      int message, Word& local, Supplier s) {
#ifdef DEBUGHEAD
cout << "InsertValueMap" << endl;
#endif
    Collection* coll = static_cast<Collection*>(args[0].addr);
    Attribute* elem = static_cast<Attribute*>(args[1].addr);

    result = qp->ResultStorage(s);
    Collection* resColl = static_cast<Collection*>(result.addr);

    resColl->CopyFrom(static_cast<Attribute*>(coll));
    resColl->Insert(elem, 1);
    return 0;
  }

  struct insertInfo : OperatorInfo {

    insertInfo()
    {
      name      = "insert";
      signature = Set::BasicType() + "(t) x t -> " + Set::BasicType() + "(t)";
      appendSignature(Multiset::BasicType() + "(t) x t -> " +
                                                Multiset::BasicType() + "(t)");
      syntax    = "_ insert _";
      meaning   = "Inserts the second argument in the first.";
    }

  };

  struct addInfo : OperatorInfo {

    addInfo()
    {
      name      = "+";
      signature = Vector::BasicType() + "(t) x t -> " +
                                                  Vector::BasicType() + "(t)";
      syntax    = "_ + _";
      meaning   = "Adds the element to the vector";
    }

  };

/*
4.3 Implementation of operator create

*/
  template<CollectionType collType> ListExpr CreateTypeMap(ListExpr args) {
#ifdef DEBUGHEAD
cout << "CreateTypeMap: " << nl->ToString(args) << endl;
#endif
    string resultType;
    switch(collType) {
    case collection::vector:
        resultType = Vector::BasicType();
        break;
      case collection::set:
        resultType = Set::BasicType();
        break;
      default:
        resultType = Multiset::BasicType();
    }
    string opName = "create_" + resultType;
#ifdef DEBUG
cout << "  Statusbericht CreateTypeMap:" << endl
     << "    resultType: " << resultType << endl;
#endif
    int length = nl->ListLength(args);
    if(length==0) {
      ErrorReporter::ReportError("Operator " + opName + " needs at least one "
          + "element to insert to know the type of the elements.");
      return nl->TypeError();
    }
    ListExpr type = nl->First(args);
#ifdef DEBUG
cout << "  Statusbericht CreateTypeMap:" << endl
     << "    type: " << type << endl;
#endif
    ListExpr errorInfo = nl->OneElemList(nl->SymbolAtom("ERROR"));
    if(!am->CheckKind(Kind::DATA(), type, errorInfo)) {
      ErrorReporter::ReportError("Operator " + opName + " expects elements of "
          + "Kind DATA.");
      return nl->TypeError();
    }
#ifdef DEBUG
cout << "  Statusbericht CreateTypeMap:" << endl
     << "    Wir sind hinter CheckKind!" << endl;
#endif
    ListExpr rest = nl->Rest(args);
    while(!nl->IsEmpty(rest)) {
      if(!nl->Equal(nl->First(rest), type)) {
        ErrorReporter::ReportError("All arguments of operator " + opName
              + " have to be of the same type.");
        return nl->TypeError();
      }
      rest = nl->Rest(rest);
    }
    return nl->TwoElemList(nl->SymbolAtom(resultType), type);
  }

  int CreateValueMap(Word* args, Word& result,
                                      int message, Word& local, Supplier s) {
#ifdef DEBUGHEAD
cout << "CreateValueMap" << endl;
#endif
    result = qp->ResultStorage(s);
    Collection* resultColl = static_cast<Collection*>(result.addr);
    int sons = qp->GetNoSons(s);
    resultColl->Clear();
    resultColl->SetDefined(true);
    int i = 0;
    while(i < sons) {
      Attribute* elem = static_cast<Attribute*>(args[i].addr);
      resultColl->Insert(elem, 1);
      i++;
    }
    resultColl->Finish();
    return 0;
  }

  struct CreateVectorInfo : OperatorInfo {

    CreateVectorInfo()
    {
      name      = "create_" + Vector::BasicType();
      signature = "t+ -> " + Vector::BasicType() + "(t)";
      syntax    = "create_" + Vector::BasicType() + "(_, _)";
      meaning   = "Creates a " + Vector::BasicType() + " of t.";
    }
  };

  struct CreateSetInfo : OperatorInfo {

    CreateSetInfo()
    {
      name      = "create_" + Set::BasicType();
      signature = "t+ -> " + Set::BasicType() + "(t)";
      syntax    = "create_" + Set::BasicType() + "(_, _)";
      meaning   = "Creates a " + Set::BasicType() + " of t.";
    }
  };

  struct CreateMultisetInfo : OperatorInfo {

    CreateMultisetInfo()
    {
      name      = "create_" + Multiset::BasicType();
      signature = "t+ -> " + Multiset::BasicType() + "(t)";
      syntax    = "create_" + Multiset::BasicType() + "(_, _)";
      meaning   = "Creates a " + Multiset::BasicType() + " of t.";
    }
  };

/*
4.4 Implementation of function collect

*/
  template<CollectionType targetType> ListExpr CollectTypeMap(ListExpr args) {
#ifdef DEBUGHEAD
cout << "CollectTypeMap: " << nl->ToString(args) << endl;
#endif
      string operatorName;
      string resultType;

      switch(targetType){
      case collection::vector:
        operatorName = "collect_vector";
        resultType = Vector::BasicType();
        break;
      case collection::set:
        operatorName = "collect_set";
        resultType = Set::BasicType();
        break;
      case collection::multiset:
        operatorName = "collect_multiset";
        resultType = Multiset::BasicType();
        break;
      default:
        operatorName = "";
      }

      const string errMsg = "Operator " + operatorName
                                        + " expects (stream DATA)";

      ListExpr argStream;
      ListExpr argType;
      ListExpr errorInfo = nl->OneElemList(nl->SymbolAtom("ERROR"));

      if ( nl->ListLength(args) == 1 )
      {
        argStream = nl->First(args);

        if ( !nl->IsAtom(argStream) && nl->ListLength(argStream) == 2)
        {
            argType = nl->Second(argStream);

            if ( nl->IsEqual(nl->First(argStream), Symbol::STREAM())
               && am->CheckKind(Kind::DATA(), argType, errorInfo) ){
                return nl->TwoElemList(
                                   nl->SymbolAtom(resultType),
                                   argType);
            }
        }
      }
      ErrorReporter::ReportError(errMsg);
      return nl->TypeError();

  }

  template<CollectionType targetType> int CollectValueMap(
                                     Word* args, Word& result,
                                     int message, Word& local, Supplier s) {

#ifdef DEBUGHEAD
cout << "CollectValueMap" << endl;
#endif
    result = qp->ResultStorage(s);
    Collection* resColl = static_cast<Collection*>(result.addr);
    resColl->Clear();
    resColl->SetDefined(true);
    Attribute* elemToInsert;
    Word elem = SetWord(Address(0));

    qp->Open(args[0].addr);
    qp->Request(args[0].addr, elem);

    while ( qp->Received(args[0].addr) )
    {
        elemToInsert = static_cast<Attribute*>(elem.addr);
        resColl->Insert(elemToInsert, 1);
        elemToInsert->DeleteIfAllowed();
        qp->Request(args[0].addr, elem);
    }
    resColl->Finish();
    qp->Close(args[0].addr);

    return 0;
  }

  struct collectSetInfo : OperatorInfo {

    collectSetInfo()
    {
      name      = "collect_set";
      signature = Symbol::STREAM() + "(t) -> " + Set::BasicType() + "(t)";
      syntax    = "_ collect_set";
      meaning   = "Collects the stream elements into a new set";
    }

  };

  struct collectMultisetInfo : OperatorInfo {

    collectMultisetInfo()
    {
      name      = "collect_multiset";
      signature = Symbol::STREAM() + "(t) -> " + Multiset::BasicType() + "(t)";
      syntax    = "_ collect_multiset";
      meaning   = "Collects the stream elements into a new multiset";
    }

  };

  struct collectVectorInfo : OperatorInfo {

    collectVectorInfo()
    {
      name      = "collect_vector";
      signature = Symbol::STREAM() + "(t) -> " + Vector::BasicType() + "(t)";
      syntax    = "_ collect_vector";
      meaning   = "Collects the stream elements into a new vector";
    }

  };

/*
4.5 Implementation of operator components

*/
  ListExpr ComponentsTypeMap(ListExpr args) {

#ifdef DEBUGHEAD
cout << "ComponentsTypeMap" << endl;
#endif
      const string errMsg = "Operator components expects (vector DATA)"
                                   " or (set DATA)"
                                   " or (multiset DATA)";

      ListExpr argCollection;
      ListExpr argType;
      ListExpr errorInfo = nl->OneElemList(nl->SymbolAtom("ERROR"));

      if ( nl->ListLength(args) == 1 )
      {
        argCollection = nl->First(args);

        if ( !nl->IsAtom(argCollection) && nl->ListLength(argCollection) == 2)
        {
            argType = nl->Second(argCollection);

            if (
                (nl->IsEqual(nl->First(argCollection), Vector::BasicType()) ||
                nl->IsEqual(nl->First(argCollection), Set::BasicType()) ||
                nl->IsEqual(nl->First(argCollection), Multiset::BasicType()))
               && am->CheckKind(Kind::DATA(), argType, errorInfo) ){
                return nl->TwoElemList(
                                   nl->SymbolAtom(Symbol::STREAM()),
                                   argType);
            }
        }
      }
      ErrorReporter::ReportError(errMsg);
      return nl->TypeError();

  }

  struct ComponentsLocalInfo
  {
    int componentIndex;
    int componentCount;
  };

  int ComponentsValueMap( Word* args, Word& result,
                          int message, Word& local, Supplier s) {

#ifdef DEBUGHEAD
cout << "ComponentsValueMap" << endl;
#endif
      ComponentsLocalInfo *linfo;
      Collection* coll = (Collection*)args[0].addr;

      switch( message )
        {
        case OPEN:
          linfo = new ComponentsLocalInfo;
          linfo->componentIndex = 0;
          if(coll->GetNoUniqueComponents()>0) {
            linfo->componentCount = coll->GetComponentCount(0);
          } else {
            linfo->componentCount = 0;
          }
          local = SetWord(linfo);
          return 0;
        case REQUEST:
          if ( local.addr == 0 ){
            result.addr = 0;
            return CANCEL;
          }

          linfo = (ComponentsLocalInfo*)local.addr;
          if (linfo->componentCount <= 0){
            if(linfo->componentIndex + 1 >= coll->GetNoUniqueComponents()){
              result.addr = 0;
              return CANCEL;
            }else{
              linfo->componentIndex++;
              linfo->componentCount =
                            coll->GetComponentCount(linfo->componentIndex);
            }
          }

          result = SetWord(coll->GetComponent(linfo->componentIndex));
          linfo->componentCount--;
          return YIELD;

        case CLOSE:
          if ( local.addr != 0 )
            {
              linfo = ( ComponentsLocalInfo*) local.addr;
              delete linfo;
              local = SetWord(Address(0));
            }
          return 0;
        }
      return -1; // should not be reached
  }

  struct componentsInfo : OperatorInfo {

    componentsInfo()
    {
      name      = "components";
      signature = Vector::BasicType() + "(t) -> " + Symbol::STREAM() + "(t)";
      appendSignature(Set::BasicType() + "(t) -> " + Symbol::STREAM() + "(t)");
      appendSignature(Multiset::BasicType() + "(t) -> "
                                                    + Symbol::STREAM() + "(t)");
      syntax    = "components(_)";
      meaning   = "Takes the elements from the Collection into a stream";
    }

  };

namespace collection {
/*
4.6 Implementation of operator get

*/
  ListExpr GetTypeMap(ListExpr args) {

#ifdef DEBUGHEAD
cout << "GetTypeMap" << endl;
#endif

      const string errMsg = "Operator get expects (vector DATA) x int";

      ListExpr argCollection;
      ListExpr argIndex;
      ListExpr argType;
      ListExpr errorInfo = nl->OneElemList(nl->SymbolAtom("ERROR"));

      if ( nl->ListLength(args) == 2 )
      {
        argCollection = nl->First(args);
        argIndex = nl->Second(args);

        if(!(nl->IsAtom(argIndex) && nl->IsEqual(argIndex, CcInt::BasicType())))
        {
            ErrorReporter::ReportError(errMsg);
            return nl->TypeError();
        }


        if ( !nl->IsAtom(argCollection) && nl->ListLength(argCollection) == 2)
        {
            argType = nl->Second(argCollection);

            if (
                (nl->IsEqual(nl->First(argCollection), Vector::BasicType()))
               && am->CheckKind(Kind::DATA(), argType, errorInfo) ){
                return argType;
            }
        }
      }
      ErrorReporter::ReportError(errMsg);
      return nl->TypeError();

  }

  int GetValueMap( Word* args, Word& result,
                       int message, Word& local, Supplier s) {

#ifdef DEBUGHEAD
cout << "GetValueMap" << endl;
#endif
    Collection* sourceColl = static_cast<Collection*>(args[0].addr);
    CcInt* index = static_cast<CcInt*>( args[1].addr );
    int indexVal = index->GetIntval();

    result = qp->ResultStorage(s);
    Attribute* resAttribute = static_cast<Attribute*>(result.addr);

    if(sourceColl->GetNoComponents() <= indexVal || indexVal < 0){
        ((Attribute*)result.addr)->SetDefined(false);
    }else{
      Attribute* elem = sourceColl->GetComponent(indexVal);
      resAttribute->CopyFrom(elem);
      elem->DeleteIfAllowed(true);
    }

    return 0;
  }

  struct getInfo : OperatorInfo {

    getInfo()
    {
      name      = "get";
      signature = Vector::BasicType() + "(t) x int -> t";
      syntax    = "get( _, _ )";
      meaning   = "Gets a component from the vector and index or undefined"
                    " if the index is invalid";
    }

  };

/*
4.7 Implementation of operator delete

*/
  ListExpr DeleteTypeMap(ListExpr args) {

#ifdef DEBUGHEAD
cout << "DeleteTypeMap" << endl;
#endif
      const string errMsg = "Operator 'deleteelem' "
                            " expects (set DATA) x DATA"
                            " or (multiset DATA) x DATA";

      ListExpr argCollection;
      ListExpr argCollectionType;
      ListExpr argDeleteType;
      ListExpr errorInfo = nl->OneElemList(nl->SymbolAtom("ERROR"));

      if ( nl->ListLength(args) == 2 )
      {
        argCollection = nl->First(args);
        argDeleteType = nl->Second(args);

        if (!nl->IsAtom(argCollection) &&
            nl->ListLength(argCollection) == 2)
        {
            argCollectionType = nl->Second(argCollection);

            if (
                (nl->IsEqual(nl->First(argCollection), Set::BasicType()) ||
                nl->IsEqual(nl->First(argCollection), Multiset::BasicType()))
               && am->CheckKind(Kind::DATA(), argCollectionType, errorInfo)
               && nl->Equal(argCollectionType, argDeleteType)){
                return argCollection;
            }
        }
      }
      ErrorReporter::ReportError(errMsg);
      return nl->TypeError();
  }

  int DeleteValueMap( Word* args, Word& result,
                       int message, Word& local, Supplier s) {

#ifdef DEBUGHEAD
  cout << "DeleteValueMap" << endl;
#endif
    Collection* sourceColl = static_cast<Collection*>(args[0].addr);
    Attribute* elemToDelete = static_cast<Attribute*>( args[1].addr );

    result = qp->ResultStorage( s );
    Collection* resColl = static_cast<Collection*>(result.addr);
    resColl->Clear();

    if(!sourceColl->IsDefined()){
      resColl->SetDefined(false);
    }else{
      for(int i = 0; i < sourceColl->GetNoUniqueComponents(); i++){
        int componentCount = sourceColl->GetComponentCount(i);
        Attribute* elem = sourceColl->GetComponent(i);
        if(elem->Compare(elemToDelete) == 0){
          componentCount--;
        }
        if(componentCount > 0){
          resColl->Insert(elem, componentCount);
        }
        if(elem) {
           elem->DeleteIfAllowed(true);
        }
      }
      resColl->Finish();
    }
    return 0;
  }

  struct deleteInfo : OperatorInfo {

    deleteInfo()
    {
      name      = "deleteelem";
      signature = Set::BasicType() + "(t) x t -> " + Set::BasicType() + "(t)";
      appendSignature(Multiset::BasicType() + "(t) x t -> "
                                            + Multiset::BasicType() + "(t)");
      syntax    = "deleteelem( _, _ )";
      meaning   = "Deletes a component one time from the set or multiset"
                   " if it is present in the collection";
    }

  };

/*
4.8 Implementation of operator concat

*/
  ListExpr ConcatTypeMap(ListExpr args) {

#ifdef DEBUGHEAD
cout << "ConcatTypeMap" << endl;
#endif
      const string errMsg = "Operator concat expects (vector DATA)"
                                   " x (vector DATA)";

      ListExpr argCollection1;
      ListExpr argCollection2;
      ListExpr argCollectionType;

      ListExpr errorInfo = nl->OneElemList(nl->SymbolAtom("ERROR"));

      if ( nl->ListLength(args) == 2 )
      {
        argCollection1 = nl->First(args);
        argCollection2 = nl->Second(args);

        if (!nl->IsAtom(argCollection1) &&
            nl->ListLength(argCollection1) == 2)
        {
            argCollectionType = nl->Second(argCollection1);

            if (
                nl->IsEqual(nl->First(argCollection1), Vector::BasicType())
               && am->CheckKind(Kind::DATA(), argCollectionType, errorInfo)
               && nl->ToString(argCollection1)
                                == nl->ToString(argCollection2)){
                return argCollection1;
            }
        }
      }
      ErrorReporter::ReportError(errMsg);
      return nl->TypeError();
  }

  int ConcatValueMap( Word* args, Word& result,
                       int message, Word& local, Supplier s) {

#ifdef DEBUGHEAD
cout << "ConcatValueMap" << endl;
#endif
    Collection* vector1 = static_cast<Collection*>(args[0].addr);
    Collection* vector2 = static_cast<Collection*>(args[1].addr);

    result = qp->ResultStorage( s );
    Collection *resVector = static_cast<Collection*>(result.addr);
    resVector->CopyFrom(static_cast<Attribute*>(vector1));

    for(int eCnt = 0; eCnt < vector2->GetNoUniqueComponents(); eCnt++){
        Attribute* elem = vector2->GetComponent(eCnt);
        resVector->Insert(elem, 1);
        elem->DeleteIfAllowed(true);
    }
    resVector->Finish();
    return 0;
  }

  struct concatInfo : OperatorInfo {

    concatInfo()
    {
      name      = "concat";
      signature = Vector::BasicType() + "(t) x "
                + Vector::BasicType() + "(t) -> " + Vector::BasicType() + "(t)";
      syntax    = "_ _ concat";
      meaning   = "Concatenates two vectors to a new one";
    }

  };

/*
4.9 Implementation of operators union, intersection and difference

*/
  enum MathSetOperationType {unionOp, intersectionOp, differenceOp};

  template<MathSetOperationType opType>
  ListExpr MathSetOperationTypeMap(ListExpr args) {
    string opName;

#ifdef DEBUGHEAD
cout << "MathSetTypeMap" << endl;
#endif
    switch(opType){
    case unionOp:
        opName = "union";
        break;
    case intersectionOp:
        opName = "intersection";
        break;
    case differenceOp:
        opName = "difference";
        break;
    default:
        break;
    }

    const string errMsg = "Operator " + opName + " expects (set DATA)"
                                   " x (set DATA) or"
                                   " (multiset DATA) x (multiset DATA)";


    ListExpr argCollection1;
    ListExpr argCollection2;
    ListExpr argCollectionType;

    ListExpr errorInfo = nl->OneElemList(nl->SymbolAtom("ERROR"));

    if ( nl->ListLength(args) == 2 )
    {
        argCollection1 = nl->First(args);
        argCollection2 = nl->Second(args);

        if (!nl->IsAtom(argCollection1) &&
            nl->ListLength(argCollection1) == 2)
        {
            argCollectionType = nl->Second(argCollection1);

            if (
                (nl->IsEqual(nl->First(argCollection1), Set::BasicType())
                 || nl->IsEqual(nl->First(argCollection1),
                                Multiset::BasicType()))
               && am->CheckKind(Kind::DATA(), argCollectionType, errorInfo)
               && nl->ToString(argCollection1)
                                == nl->ToString(argCollection2)){
                return argCollection1;
            }
        }
      }
      ErrorReporter::ReportError(errMsg);
      return nl->TypeError();
  }

  template<MathSetOperationType opType>
  int MathSetOperationValueMap( Word* args, Word& result,
                      int message, Word& local, Supplier s) {
#ifdef DEBUGHEAD
    cout << "MathSetValueMap" << endl;
#endif
    Collection* coll1 = static_cast<Collection*>(args[0].addr);
    Collection* coll2 = static_cast<Collection*>(args[1].addr);

    result = qp->ResultStorage( s );
    Collection* resColl = static_cast<Collection*>(result.addr);
    resColl->Clear();

    if( !coll1->IsDefined() || !coll2->IsDefined() ){
      resColl->SetDefined( false );
      return 0;
    }

    int insertCnt;

    int noColl1Components = coll1->GetNoUniqueComponents();
    int noColl2Components = coll2->GetNoUniqueComponents();

    int elementIdx1 = 0;
    int elementIdx2 = 0;

    bool coll1Ended = (noColl1Components <= elementIdx1);
    bool coll2Ended = (noColl2Components <= elementIdx2);

    bool restore1 = !coll1Ended;
    bool restore2 = !coll2Ended;

    Attribute* elem1 = 0;
    Attribute* elem2 = 0;
    while(!coll1Ended && !coll2Ended){
      if(restore1) {
        elem1 = coll1->GetComponent(elementIdx1);
        restore1 = false;
      }
      if(restore2) {
        elem2 = coll2->GetComponent(elementIdx2);
        restore2 = false;
      }

      int compareRes = elem1->Compare(elem2);

      switch(compareRes){
        case -1:
          switch(opType){
            case unionOp:
              resColl->Insert(elem1,coll1->GetComponentCount(elementIdx1));
              elem1->Print(cout) << endl;
              break;
            case differenceOp:
              resColl->Insert(elem1,coll1->GetComponentCount(elementIdx1));
              break;
            default:
              break;
          }
          elementIdx1++; restore1 = true;
          break;
        case 0:
          switch(opType){
            case unionOp:
              resColl->Insert(elem1,  coll1->GetComponentCount(elementIdx1)
                                    + coll2->GetComponentCount(elementIdx2));
              elem1->Print(cout) << endl;
              break;
            case intersectionOp:
              insertCnt =
                    coll1->GetComponentCount(elementIdx1) >
                              coll2->GetComponentCount(elementIdx2) ?
                              coll2->GetComponentCount(elementIdx2) :
                              coll1->GetComponentCount(elementIdx1);
              resColl->Insert(elem1, insertCnt);
              break;
            case differenceOp:
              insertCnt = coll1->GetComponentCount(elementIdx1) -
                        coll2->GetComponentCount(elementIdx2);
              if(insertCnt > 0){
                resColl->Insert(elem1, insertCnt);
              }
              break;
            default:
              break;
          }
          elementIdx1++; restore1 = true;
          elementIdx2++; restore2 = true;
          break;
        case 1:
          switch(opType){
          case unionOp:
            resColl->Insert(elem2, coll2->GetComponentCount(elementIdx2));
            elem2->Print(cout) << endl;
            break;
          default:
            break;
          }
          elementIdx2++;
          restore2 = true;
          break;
        default:
          break;
      }
      coll1Ended = (noColl1Components <= elementIdx1);
      coll2Ended = (noColl2Components <= elementIdx2);
      if(elem1 && restore1) {
        elem1->DeleteIfAllowed(true);
        elem1 = 0;
      }
      if(elem2 && restore2) {
        elem2->DeleteIfAllowed(true);
        elem2 = 0;
      }
    }
    if(elem1) {
       elem1->DeleteIfAllowed(true);
    }
    if(elem2) {
       elem2->DeleteIfAllowed(true);
    }
    for(int iCnt1 = elementIdx1;
                            iCnt1 < coll1->GetNoUniqueComponents(); iCnt1++){
      elem1 = coll1->GetComponent(iCnt1);
      switch(opType){
        case unionOp:
        case differenceOp:
          resColl->Insert(elem1, coll1->GetComponentCount(iCnt1));
          elem1->Print(cout) << endl;
          break;
        default:
          break;
      }
      elem1->DeleteIfAllowed(true);
    }
    for(int iCnt2 = elementIdx2;
                            iCnt2 < coll2->GetNoUniqueComponents(); iCnt2++){
      elem2 = coll2->GetComponent(iCnt2);
      switch(opType){
        case unionOp:
          resColl->Insert(elem2, coll2->GetComponentCount(iCnt2));
          elem2->Print(cout) << endl;
          break;
        default:
          break;
        }
        elem2->DeleteIfAllowed(true);
    }
    resColl->Finish();
    return 0;
  }

  struct unionInfo : OperatorInfo {

    unionInfo()
    {
      name      = "union";
      signature = Set::BasicType() + "(t) x " + Set::BasicType() + "(t) -> "
                                                    + Set::BasicType() + "(t)";
      appendSignature(Multiset::BasicType() + "(t) x "
                                            + Multiset::BasicType() + "(t) -> "
                                            + Multiset::BasicType() + "(t)");
      syntax    = "_ union _";
      meaning   = "assigns the union-operation on two sets or multisets";
    }

  };

  struct intersectionInfo : OperatorInfo {

    intersectionInfo()
    {
      name      = "intersection";
      signature = Set::BasicType() + "(t) x " + Set::BasicType() + "(t) -> "
                                                    + Set::BasicType() + "(t)";
      appendSignature(Multiset::BasicType() + "(t) x " + Multiset::BasicType()
                                            + "(t) -> "
                                            + Multiset::BasicType() + "(t)");
      syntax  = "intersection( _, _)";
      meaning = "assigns the intersection-operation on two sets or multisets";
    }

  };

  struct differenceInfo : OperatorInfo {

    differenceInfo()
    {
      name      = "difference";
      signature = Set::BasicType() + "(t) x " + Set::BasicType() + "(t) -> "
                                                   + Set::BasicType() + "(t)";
      appendSignature(Multiset::BasicType() + "(t) x " + Multiset::BasicType() +
                                  "(t) -> "  + Multiset::BasicType() + "(t)");
      syntax    = "difference( _, _)";
      meaning   = "assigns the difference-operation on two sets or multisets";
    }

  };

/*
4.10 Implementation of operation size

*/
int sizeFun(Word* args, Word& result, int message, Word& local, Supplier s){
  Collection* co = (Collection*)args[0].addr;
  result = qp->ResultStorage(s);
  ((CcInt*)result.addr)->Set(co->IsDefined(), co->GetNoComponents());
return 0;
}

ListExpr sizeTypeMap(ListExpr args){
if( nl->ListLength(args) > 0)
  {
    ListExpr arg1 = nl->First(args);

    if (!nl->IsAtom(arg1) && (nl->IsEqual(nl->First(arg1),
       Set::BasicType()) || nl->IsEqual(nl->First(arg1), Multiset::BasicType())
       || nl->IsEqual(nl->First(arg1), Vector::BasicType()))) {
      return nl->SymbolAtom(CcInt::BasicType());
    }
  }

  return nl->SymbolAtom(Symbol::TYPEERROR());

}


struct sizeInfo : OperatorInfo {

  sizeInfo() : OperatorInfo()
  {
    name      = "size";
    signature = Set::BasicType() + " -> " + CcInt::BasicType();
    appendSignature(Multiset::BasicType() + " -> " + CcInt::BasicType());
    appendSignature(Vector::BasicType() + " -> " + CcInt::BasicType());
    syntax    = "size( _ )";
    meaning   = "Number of contained objects";
  }

};

/*
4.11 Implementation of operators <, <=, >, >= and =

*/
int ltFun(Word* args, Word& result, int message, Word& local, Supplier s){
  bool r = true;
  int tmp;
  Collection* co = (Collection*)args[0].addr;
  Collection* cl = (Collection*)args[1].addr;
  result = qp->ResultStorage(s);
  tmp = co->Compare(cl);
  if(tmp == 0){
    r = false;
  }
  else if(tmp == 1)
    r = false;
  else
    r = true;
  ((CcBool*)result.addr)->Set(true, r);
  return 0;
}

ListExpr compareTypeMap(ListExpr args){
  if (nl->IsAtom(args) || nl->ListLength(args) != 2){
    return nl->SymbolAtom(Symbol::TYPEERROR());
  }
  ListExpr arg1 = nl->First(args);
  ListExpr arg2 = nl->Second(args);
  if (    !nl->IsAtom(arg1)
       && (    nl->IsEqual(nl->First(arg1), Set::BasicType())
            || nl->IsEqual(nl->First(arg1), Multiset::BasicType())
            || nl->IsEqual(nl->First(arg1), Vector::BasicType())
          )
       && nl->Equal(arg1, arg2)
     ) {
    return nl->SymbolAtom(CcBool::BasicType());
  }
  return nl->SymbolAtom(Symbol::TYPEERROR());
}

struct ltInfo : OperatorInfo {

  ltInfo() : OperatorInfo()
  {
    name      = "<";
    signature = Set::BasicType() + " x " + Set::BasicType() + " -> "
                                                        + CcBool::BasicType();
    appendSignature(Multiset::BasicType() + " x " + Multiset::BasicType()
                                              + " -> " + CcBool::BasicType());
    appendSignature(Vector::BasicType() + " x " + Vector::BasicType() + " -> "
                                                        + CcBool::BasicType());
    syntax    = " _ < _ ";
    meaning   = "Object 1 Smaller Than Object 2";
  }

};

int eqFun(Word* args, Word& result, int message, Word& local, Supplier s){
  bool r = true;
  int tmp;
  Collection* co = (Collection*)args[0].addr;
  Collection* cl = (Collection*)args[1].addr;
  result = qp->ResultStorage(s);
  tmp = co->Compare(cl);
  if(tmp == 0){
    r = true;
  }
  else if(tmp == 1)
    r = false;
  else
    r = false;
  ((CcBool*)result.addr)->Set(true, r);
  return 0;
}

struct eqInfo : OperatorInfo {

  eqInfo() : OperatorInfo()
  {
    name      = "=";
    signature = Set::BasicType() + " x " + Set::BasicType() + " -> "
                                                        + CcBool::BasicType();
    appendSignature(Multiset::BasicType() + " x " + Multiset::BasicType()
                                                + " -> " + CcBool::BasicType());
    appendSignature(Vector::BasicType() + " x " + Vector::BasicType()
                                                + " -> " + CcBool::BasicType());
    syntax    = " _ = _ ";
    meaning   = "Object 1 equals Object 2";
  }

};

int neFun(Word* args, Word& result, int message, Word& local, Supplier s){
  bool r = true;
  int tmp;
  Collection* co = (Collection*)args[0].addr;
  Collection* cl = (Collection*)args[1].addr;
  result = qp->ResultStorage(s);
  tmp = co->Compare(cl);
  if(tmp == 0){
    r = false;
  }
  else
    r = true;
  ((CcBool*)result.addr)->Set(true, r);
  return 0;
}

struct neInfo : OperatorInfo {

  neInfo() : OperatorInfo()
  {
    name      = "#";
    signature = Set::BasicType() + " x " + Set::BasicType() + " -> "
                                                          + CcBool::BasicType();
    appendSignature(Multiset::BasicType() + " x " + Multiset::BasicType()
                                                + " -> " + CcBool::BasicType());
    appendSignature(Vector::BasicType() + " x " + Vector::BasicType() + " -> "
                                                        + CcBool::BasicType());
    syntax    = " _ # _ ";
    meaning   = "Object 1 does not equal Object 2";
  }

};

int gtFun(Word* args, Word& result, int message, Word& local, Supplier s){
  bool r = true;
  int tmp;
  Collection* co = (Collection*)args[0].addr;
  Collection* cl = (Collection*)args[1].addr;
  result = qp->ResultStorage(s);
  tmp = co->Compare(cl);
  if(tmp == 0){
    r = false;
  }
  else if(tmp == 1)
    r = true;
  else
    r = false;
  ((CcBool*)result.addr)->Set(true, r);
  return 0;
}

struct gtInfo : OperatorInfo {

  gtInfo() : OperatorInfo()
  {
    name      = ">";
    signature = Set::BasicType() + " x " + Set::BasicType() +
                                                " -> " + CcBool::BasicType();
    appendSignature(Multiset::BasicType() + " x " + Multiset::BasicType() +
                                                " -> " + CcBool::BasicType());
    appendSignature(Vector::BasicType() + " x " + Vector::BasicType() +
                                                " -> " + CcBool::BasicType());
    syntax    = " _ > _ ";
    meaning   = "Object 1 Greater Than Object 2";
  }

};

int leFun(Word* args, Word& result, int message, Word& local, Supplier s){
  bool r = true;
  int tmp;
  Collection* co = (Collection*)args[0].addr;
  Collection* cl = (Collection*)args[1].addr;
  result = qp->ResultStorage(s);
  tmp = co->Compare(cl);
  if(tmp == 0){
    r = true;
  }
  else if(tmp == 1)
    r = false;
  else
    r = true;
  ((CcBool*)result.addr)->Set(true, r);
  return 0;
}

struct leInfo : OperatorInfo {

  leInfo() : OperatorInfo()
  {
    name      = "<=";
    signature = Set::BasicType() + " x " + Set::BasicType() + " -> "
                                                        + CcBool::BasicType();
    appendSignature(Multiset::BasicType() + " x " + Multiset::BasicType()
                                                + " -> " + CcBool::BasicType());
    appendSignature(Vector::BasicType() + " x " + Vector::BasicType()
                                                + " -> " + CcBool::BasicType());
    syntax    = " _ <= _ ";
    meaning   = "Object 1 Smaller equal Object 2";
  }

};

int geFun(Word* args, Word& result, int message, Word& local, Supplier s){
//cout << "Wir sind in der ge Funktion 0" << endl;
  bool r = true;
  int tmp;
  Collection* co = (Collection*)args[0].addr;
  Collection* cl = (Collection*)args[1].addr;
  result = qp->ResultStorage(s);
  tmp = co->Compare(cl);
  if(tmp == 0){
    r = true;
  }
  else if(tmp == 1)
    r = true;
  else
    r = false;
  ((CcBool*)result.addr)->Set(true, r);
  return 0;
}

struct geInfo : OperatorInfo {

  geInfo() : OperatorInfo()
  {
    name      = ">=";
    signature = Set::BasicType() + " x " + Set::BasicType() + " -> "
                                                          + CcBool::BasicType();
    appendSignature(Multiset::BasicType() + " x " + Multiset::BasicType()
                                                + " -> " + CcBool::BasicType());
    appendSignature(Vector::BasicType() + " x " + Vector::BasicType() + " -> "
                                                        + CcBool::BasicType());
    syntax    = " _ >= _ ";
    meaning   = "Object 1 Greater equal Object 2";
  }

};


/*
5 Implementation of class CollectionAlgebra, registration of TypeConstructors
and operators

*/
class CollectionAlgebra : public Algebra {
    public:
    CollectionAlgebra() : Algebra() {
      AddTypeConstructor(&vectorTC);
      AddTypeConstructor(&setTC);
      AddTypeConstructor(&multisetTC);

      vectorTC.AssociateKind(Kind::DATA());
      setTC.AssociateKind(Kind::DATA());
      multisetTC.AssociateKind(Kind::DATA());

      AddOperator(containsInfo(), ContainsInValueMap<true>,
                  ContainsInTypeMap<true>);
      AddOperator(inInfo(), ContainsInValueMap<false>,
                  ContainsInTypeMap<false>);
      AddOperator(insertInfo(), InsertValueMap<true>, InsertTypeMap<true>);
      AddOperator(addInfo(), InsertValueMap<false>, InsertTypeMap<false>);
      AddOperator(CreateVectorInfo(), CreateValueMap,
                  CreateTypeMap<collection::vector>);
      AddOperator(CreateSetInfo(), CreateValueMap,
                  CreateTypeMap<collection::set>);
      AddOperator(CreateMultisetInfo(), CreateValueMap,
                  CreateTypeMap<collection::multiset>);
      AddOperator(collectSetInfo(), CollectValueMap<collection::set>,
                  CollectTypeMap<collection::set>);
      AddOperator(collectMultisetInfo(), CollectValueMap<collection::multiset>,
                  CollectTypeMap<collection::multiset>);
      AddOperator(collectVectorInfo(), CollectValueMap<collection::vector>,
                  CollectTypeMap<collection::vector>);
      AddOperator(componentsInfo(), ComponentsValueMap,
                  ComponentsTypeMap);
      AddOperator(getInfo(), GetValueMap,
                  GetTypeMap);
      AddOperator(deleteInfo(), DeleteValueMap,
                  DeleteTypeMap);
      AddOperator(concatInfo(), ConcatValueMap,
                  ConcatTypeMap);
      AddOperator(unionInfo(), MathSetOperationValueMap<unionOp>,
                  MathSetOperationTypeMap<unionOp>);
      AddOperator(intersectionInfo(), MathSetOperationValueMap<intersectionOp>,
                  MathSetOperationTypeMap<intersectionOp>);
      AddOperator(differenceInfo(), MathSetOperationValueMap<differenceOp>,
                  MathSetOperationTypeMap<differenceOp>);
      AddOperator( sizeInfo(), sizeFun, sizeTypeMap );
      AddOperator( eqInfo(), eqFun, compareTypeMap );
      AddOperator( gtInfo(), gtFun, compareTypeMap );
      AddOperator( ltInfo(), ltFun, compareTypeMap );
      AddOperator( geInfo(), geFun, compareTypeMap );
      AddOperator( leInfo(), leFun, compareTypeMap );
      AddOperator( neInfo(), neFun, compareTypeMap );

    }
    ~CollectionAlgebra() {};
  };
} // end namespace collection
/*
6 Initialization

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

*/
extern "C"
Algebra* InitializeCollectionAlgebra(NestedList* nlRef, QueryProcessor* qpRef) {
  return new (collection::CollectionAlgebra);
}


namespace Vector{
  const string BasicType() {return "vector"; };
  const bool checkType(ListExpr list){
     return collection::Collection::checkType(list, BasicType());
  }
}

namespace Set{
  const string BasicType() {return "set"; };
  const bool checkType(ListExpr list){
     return collection::Collection::checkType(list, BasicType());
  }
}

namespace Multiset{
  const string BasicType() {return "multiset"; };
  const bool checkType(ListExpr list){
     return collection::Collection::checkType(list, BasicType());
  }
}


