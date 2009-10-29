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

    multiset(t) [x] t [->] "int"[1]

Returns whether the collection contains the element. If the collection is a
multiset, the count of the element is returned.


  * in

    t [x] vector(t) [->] "bool"[1]

    t [x] set(t) [->] "bool"[1]

    t [x] multiset [->] "int"[1]

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

    vector(t) [->] "int"[1]

    set(t) [->] "int"[1]

    multiset(t) [->] "int"[1]

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

#include "Algebra.h"
#include "NestedList.h"
#include "NList.h"
#include "LogMsg.h"
#include "QueryProcessor.h"
#include "ConstructorTemplates.h"
#include "StandardTypes.h"
#include "DBArray.h"
#include "Attribute.h"

extern NestedList* nl;
extern QueryProcessor* qp;

#include "TypeMapUtils.h"
#include "Symbols.h"

using namespace symbols;
using namespace mappings;

#include <string>
using namespace std;

/*
3 Implementation of types

*/
namespace collection {

  enum CollectionType {vector, set, multiset, undef};

/*
3.1 Class header

*/
  class Collection : public Attribute {
    public:

    Collection(CollectionType type, const ListExpr typeInfo,
                  const int buckets = 10);

    Collection(const Collection& coll, bool empty = false);

    Collection(CollectionType type);//only to be used in Create-function

    ~Collection();


    static Word In(const ListExpr typeInfo, const ListExpr instance,
                   const int errorPos, ListExpr& errorInfo, bool& correct);

    static ListExpr Out(ListExpr typeInfo, Word value);

    static Word Create(const ListExpr typeInfo);

    static void Delete(const ListExpr typeInfo, Word& w);

    static void Close(const ListExpr typeInfo, Word& w);

    static Word Clone (const ListExpr typeInfo, const Word& w);

    static bool KindCheck(ListExpr type, ListExpr& errorInfo);

    static int SizeOfObj();

    size_t Sizeof() const;

    int NumOfFLOBs() const;

    FLOB* GetFLOB(const int i);

    static bool Open(SmiRecord& valueRecord, size_t& offset,
                     const ListExpr typeInfo, Word& value);

    static bool Save(SmiRecord& valueRecord, size_t& offset,
                     const ListExpr typeInfo, Word& value);

    int Compare(const Attribute* arg) const;

    size_t HashValue() const;

    void Sort();

    void CopyFrom(const Attribute* right);

    bool Adjacent(const Attribute* arg) const;

    Collection* Clone() const;

    bool IsDefined() const;

    void SetDefined(const bool defined);

    void Finish();

    ostream& Print( ostream& os ) const;

    static void* Cast(void* addr);



    void Insert(Attribute* elem, const int count);
/*
Inserts elem count times.

If the collection is a set, the elem is only inserted once, and only, if it
isn't inserted yet.

If the collection is a multiset, we have two cases:

Case 1: The element is inserted already.

In this case we just increment elemCount of this element with count.

Case 2: The element is not inserted yet.

In this case we save this element in our FLOB and set elemCount of this
element to count.

If the collection is a vector, the element is saved, if it hasn't been saved
yet and the index of this element is inserted in elemArrayIndex count times.

*/

    CollectionType GetMyCollType() const;

//     ListExpr GetMyTypeInfo() const;

    int Contains(const Attribute* elem) const;
/*
If our collection is a vector or set, it returns 1 if elem is contained, else
0. If our collection is a multiset, it returns, how often elem is contained.

*/

    Attribute* GetComponent(const int pos) const;
/*
Returns the component at position pos.

Note that in case of a multiset, we only return every element once, regardless
of how often this element is contained. So the maximum position is not
GetNoComponents(), but GetNoUniqueComponents()!

*/

    int GetComponentCount(const int pos) const;
/*
Returns, how often this element is contained in our collection.

If our collection is a vector or a set, only 0 (is not contained) or 1 (is
contained) is returned.

Consider the remarks about GetComponent(pos)!

*/

    int GetNoComponents() const;
/*
Returns the number of components contained in our collection.

NOT to be used as limit for position of a GetComponent(pos) or
GetComponentCount(pos) call.

Use GetNoUniqueComponents() instead!

*/

    int GetNoUniqueComponents() const;
/*
Returns the number of components contained in our collection.

Since for a multiset it counts every element only once, regardless of how often
it is realy contained, this function is to be used as limit for
GetComponent(pos) or GetComponentCount(pos).

*/

    Collection* Union(const Collection* coll) const;

    Collection* Intersection(const Collection* coll) const;

    Collection* Difference(const Collection* coll) const;

    Collection* Delete(const Attribute* elem) const;


    private:
    Collection() {}

    static void GetIds(int& algebraId, int& typeId, const ListExpr typeInfo);
/*
Sets the algebraId and typeId to the Ids of our subtype, given with
nl->Second(typeInfo).

*/

    void SortMerge(const int start, const int end);
/*
fehlt noch was

*/

    static CollectionType GetCollType(const ListExpr coll);

    int GetIndex(const Attribute* elem) const;
/*
If the result is greater than -1, you can get elem by calling
RestoreComponent(result) or GetComponent(result).
If elem is not saved yet, -1 is returned.

*/

    void InsertIndex(const Attribute* elem, const int index);

    void SaveComponent(Attribute* elem, const int count);

    Attribute* RestoreComponent(const int pos) const;

    void AddHashValue(const int value, const int count);

    bool defined;
    int elemAlgId, elemTypeId;
    int size;
/*
If our collection is a multiset, we use this variable to save the number of
our components.

*/

    int numOfBuckets;
/*
Number of buckets used for our element hashing.
See firstElemHashValue and nextElemHashValue.

*/

    size_t hashValue;
    CollectionType collType;

    DBArray<size_t> elemFLOBDataOffset;
/*
We use this DBArray to save the offsets at which the FLOB contents of the
elements are saved in elementData;

*/

    DBArray<int> elemCount;
/*
This DBArray is used only, if our collection is a multiset, to save the count
of every element.

*/

    DBArray<int> elemArrayIndex;
/*
Returns the index for every element, where index[*]sizeOfObj is the
offset of the desired elem in the elements FLOB.
Also, elemFLOBDataOffset(index) returns the offset, where
the contents of the FLOBs of the desired element are saved.

Used for faster sorting of set and multiset, since we only must sort this array
instead of the arrays elemData, elemDataEnd, elemCount, firstElemHashValue and
nextElemHashValue, which all depend on each other.

Also we only have to save every element once.
If it has to be inserted again, we just append the index belonging to this
element to elemArrayIndex (if the collection is a vector), or we increment
elemCount(index) (if the collection is a multiset).

*/

    DBArray<int> firstElemHashValue;
    DBArray<int> nextElemHashValue;
/*
To allow a fast search of an element, we save the index of the first element
with (hashvalue mod numOfBuckets) in this array at (hashvalue mod
numOfBuckets).
If another element has (hashvalue mod numOfBuckets) its index is saved at
nextElemHashValue(index of first element).
If an element is inserted, at nextElemHashValue(index of this element) a -1 is
saved.

If we choose our numOfBuckets reasonable, searching an element becomes very
fast.

*/

    FLOB elements;
    FLOB elementData;
/*
In this FLOB we save the data of our elements.

*/

  }; //end of class Collection


/*
3.2 Implementation of class functions

*/


/*
Create a Collection of type (vector, set, multiset or undef) with typeInfo.

*/
  Collection::Collection(const CollectionType type, const ListExpr typeInfo,
                                const int buckets /* = 10 */):
    defined(false), size(0), hashValue(0), collType(type),
    elemFLOBDataOffset(0), elemCount(0), elemArrayIndex(0),
    firstElemHashValue(0), nextElemHashValue(0),
    elements(0), elementData(0)
  {
#ifdef DEBUGHEAD
cout << "Collection(1)" << endl;
#endif
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
      SetDefined(coll.IsDefined());
      hashValue = coll.hashValue;
      numOfBuckets = coll.numOfBuckets;

      for(int i=0;i<coll.elemFLOBDataOffset.Size();i++) {
        const size_t* offset;
        coll.elemFLOBDataOffset.Get(i, offset);
        elemFLOBDataOffset.Append(*offset);
      }

      for(int i=0;i<coll.elemCount.Size();i++) {
        const int* count;
        coll.elemCount.Get(i, count);
        elemCount.Append(*count);
      }

      for(int i=0;i<coll.elemArrayIndex.Size();i++) {
        const int* index;
        coll.elemArrayIndex.Get(i, index);
        elemArrayIndex.Append(*index);
      }

      for(int i=0;i<coll.firstElemHashValue.Size();i++) {
        const int* elem;
        coll.firstElemHashValue.Get(i, elem);
        firstElemHashValue.Append(*elem);
      }

      for(int i=0;i<coll.nextElemHashValue.Size();i++) {
        const int* elem;
        coll.nextElemHashValue.Get(i, elem);
        nextElemHashValue.Append(*elem);
      }

      const char* data;
      if(coll.elements.Size()>0) {
        coll.elements.Get(0, &data, false);
        elements.Resize(coll.elements.Size());
        elements.Put(0, coll.elements.Size(), data);
      }
      if(coll.elementData.Size()>0) {
        coll.elementData.Get(0, &data, false);
        elementData.Resize(coll.elementData.Size());
        elementData.Put(0, coll.elementData.Size(), data);
      }
    }
  }


/*
Create a Collection of type.

Only to be used by Create function, since we don't know algebraId and typeId
of our subtype there.

*/
  Collection::Collection(CollectionType type):
    defined(false), elemAlgId(0), elemTypeId(0), size(0), numOfBuckets(0),
    hashValue(0), collType(type), elemFLOBDataOffset(0),
    elemCount(0), elemArrayIndex(0), firstElemHashValue(0),
    nextElemHashValue(0), elements(0), elementData(0)
  {
#ifdef DEBUGHEAD
cout << "Collection(3)" << endl;
#endif
  }


  Collection::~Collection() {}


  Word Collection::In(const ListExpr typeInfo, const ListExpr instance,
                      const int errorPos, ListExpr& errorInfo, bool& correct) {
#ifdef DEBUGHEAD
cout << "In" << endl << "    TypeInfo: " << nl->ToString(typeInfo) << endl;
#endif
    Word w = SetWord(Address(0));
    if(nl->IsAtom(typeInfo) || nl->ListLength(typeInfo)!=2) {
      correct = false;
      return w;
    }
    CollectionType collType = GetCollType(nl->First(typeInfo));
    if((collType == undef) || nl->IsAtom(instance)) {
      correct = false;
      return w;
    }

    Collection* coll = new Collection(collType, typeInfo,
                                        nl->ListLength(instance));

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
        if(nl->ListLength(first)!=2) {
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
      }
    }
    if(correct) {
      coll->Finish();
      w.addr = coll;
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
      return nl->OneElemList(nl->StringAtom("Element ist nicht definiert!"));
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
    return ret;
  }


/*
Creates an empty, undefined collection without subtype for the Query Processor.

*/
  Word Collection::Create(const ListExpr typeInfo) {

#ifdef DEBUGHEAD
cout << "Create: " << nl->ToString(typeInfo) << endl;
#endif
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
/*    if(nl->IsAtom(type)) {
      coll = nl->SymbolValue(type);
      if((coll==VECTOR)||(coll==SET)||(coll==MULTISET)) {
        return true;
      }
    }*/
    if((nl->ListLength(type)==2) && (nl->IsAtom(nl->First(type)))) {
      coll = nl->SymbolValue(nl->First(type));
      if((coll==MULTISET) || (coll==VECTOR) || (coll==SET)) {
        return am->CheckKind("DATA", nl->Second(type), errorInfo);
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


  FLOB* Collection::GetFLOB(const int i) {
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

ostream& Print( ostream& os ) const;

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
  }else{
    int countCompareElem = 0;
    int countOwnElem = 0;
    int compareResult;

    for(int eCnt = 0; eCnt < GetNoUniqueComponents(); eCnt++){
        countCompareElem = collToCompare->GetComponentCount(eCnt);
        countOwnElem = GetComponentCount(eCnt);


        if((compareResult = GetComponent(eCnt)->Compare(
                        collToCompare->GetComponent(eCnt))) != 0){
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
    this->defined = coll->defined;
    this->elemAlgId = coll->elemAlgId;
    this->elemTypeId = coll->elemTypeId;
    this->size = coll->size;
    this->numOfBuckets = coll->numOfBuckets;
    this->hashValue = coll->hashValue;
    this->collType = coll->collType;

    if(coll->elemFLOBDataOffset.Size()>0) {
      this->elemFLOBDataOffset.Resize(coll->elemFLOBDataOffset.Size());
      for(int i=0;i<coll->elemFLOBDataOffset.Size();i++) {
        const size_t* fData;
        coll->elemFLOBDataOffset.Get(i, fData);
        this->elemFLOBDataOffset.Put(i, *fData);
      }
    } else {
      this->elemFLOBDataOffset.Clear();
    }
    if(coll->elemCount.Size()>0) {
      this->elemCount.Resize(coll->elemCount.Size());
      for(int i=0;i<coll->elemCount.Size();i++) {
        const int* count;
        coll->elemCount.Get(i, count);
        this->elemCount.Put(i, *count);
      }
    } else {
      this->elemCount.Clear();
    }
    if(coll->elemArrayIndex.Size()>0) {
      this->elemArrayIndex.Resize(coll->elemArrayIndex.Size());
      for(int i=0;i<coll->elemArrayIndex.Size();i++) {
        const int* index;
        coll->elemArrayIndex.Get(i, index);
        this->elemArrayIndex.Put(i, *index);
      }
    } else {
      this->elemArrayIndex.Clear();
    }
    if(coll->firstElemHashValue.Size()>0) {
      this->firstElemHashValue.Resize(coll->firstElemHashValue.Size());
      for(int i=0;i<coll->firstElemHashValue.Size();i++) {
        const int* fValue;
        coll->firstElemHashValue.Get(i, fValue);
        this->firstElemHashValue.Put(i, *fValue);
      }
    } else {
      this->firstElemHashValue.Clear();
    }
    if(coll->nextElemHashValue.Size()>0) {
      this->nextElemHashValue.Resize(coll->nextElemHashValue.Size());
      for(int i=0;i<coll->nextElemHashValue.Size();i++) {
        const int* nValue;
        coll->nextElemHashValue.Get(i, nValue);
        this->nextElemHashValue.Put(i, *nValue);
      }
    } else {
      this->nextElemHashValue.Clear();
    }

    const char* data;
    if(coll->elements.Size()>0) {
      coll->elements.Get(0, &data, false);
      this->elements.Resize(coll->elements.Size());
      this->elements.Put(0, coll->elements.Size(), data);
    } else {
      this->elements.Clean();
    }
    if(coll->elementData.Size()>0) {
      coll->elementData.Get(0, &data, false);
      this->elementData.Resize(coll->elementData.Size());
      this->elementData.Put(0, coll->elementData.Size(), data);
    } else {
      this->elementData.Clean();
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


  bool Collection::IsDefined() const {
#ifdef DEBUGHEAD
cout << "IsDefined" << endl;
#endif
    return defined;
  }


  void Collection::SetDefined(const bool defined) {
#ifdef DEBUGHEAD
cout << "SetDefined" << endl;
#endif
    this->defined = defined;
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
        AddHashValue((static_cast<Attribute*>(elem))->HashValue(),
                              count);
      } else if (collType==multiset) {
        const int* cnt;
        elemCount.Get(index, cnt);
        elemCount.Put(index, (*cnt+count));
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
        const int* count;
        elemCount.Get(index, count);
        return *count;
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
    const int* index;
    elemArrayIndex.Get(pos, index);
    return RestoreComponent(*index);
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
    const int* index;
    elemArrayIndex.Get(pos, index);
    const int* count;
    elemCount.Get(*index, count);
    return *count;
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
    const int* index;
    int pointer3 = 0;
    Attribute* elem1 = GetComponent(pointer1);
    Attribute* elem2 = GetComponent(pointer2);
    bool finished = false;
    while(!finished) {
      int compare = elem1->Compare(elem2);
      if(compare>0) {
        elemArrayIndex.Get(pointer2, index);
        pointer2++;
        if(pointer2<=end) {
          elem2 = GetComponent(pointer2);
        } else {
          finished = true;
        }
      } else {
        elemArrayIndex.Get(pointer1, index);
        pointer1++;
        if(pointer1<=middle) {
          elem1 = GetComponent(pointer1);
        } else {
          finished = true;
        }
      }
      help[pointer3] = *index;
      pointer3++;
    }
    while(pointer1 <= middle) {
      elemArrayIndex.Get(pointer1, index);
      help[pointer3] = *index;
      pointer1++;
      pointer3++;
    }
    while(pointer2 <= end) {
      elemArrayIndex.Get(pointer2, index);
      help[pointer3] = *index;
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
    if(collType==VECTOR) {
      return vector;
    } else if(collType==SET) {
      return set;
    } else if(collType==MULTISET) {
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
    if(elements.Size()<(offset+size)) {
      if(elements.Size()<(8*size)) {
        elements.Resize(8*size);
      } else {
        elements.Resize(elements.Size()*2);
      }
    }
    elements.Put(offset, size, elem);

    offset = elementData.Size();
    elemFLOBDataOffset.Append(offset);
    for(int i=0;i<elem->NumOfFLOBs();i++) {
      FLOB* tempFLOB = elem->GetFLOB(i);
      size = tempFLOB->Size();
      elementData.Resize(offset+size);
      const char* data;
      tempFLOB->Get(0, &data, false);
      elementData.Put(offset, size, data);
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
    Attribute* elem = (Attribute*)(am->CreateObj(elemAlgId, elemTypeId))
                                    (typeInfo).addr;

    size_t offset = (size_t)(pos*(am->SizeOfObj(elemAlgId, elemTypeId))());
    elements.Get(offset, (const char**)&elem, false);
    elem = (Attribute*)(am->Cast(elemAlgId, elemTypeId))(elem);

    const size_t* temp;
    elemFLOBDataOffset.Get(pos, temp);
    offset = *temp;
    for(int i=0;i<elem->NumOfFLOBs();i++) {
      FLOB* tempFLOB = elem->GetFLOB(i);
      size_t size = tempFLOB->Size();
#ifdef DEBUG
cout << "  size(" << i << "): " << size << endl;
#endif
      const char* tempData;
      elementData.Get(offset, &tempData, false);
      size_t bytes = tempFLOB->ReadFrom((char*)tempData);
      assert(size==bytes);
      offset += bytes;
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
    const int* indexPointer;
    firstElemHashValue.Get(hashsum, indexPointer);
    int index = *indexPointer;
#ifdef DEBUG
cout << "  Statusbericht GetIndex-Funktion:" << endl
     << "    Hashsum: " << hashsum << endl
     << "    Index: " << index << endl;
#endif
    while(index>-1) {
      Attribute* elem2 = RestoreComponent(index);
      int comp = elem->Compare(elem2);
      if(comp==0) {
        return index;
      }
      nextElemHashValue.Get(index, indexPointer);
      index = *indexPointer;
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
    const int* indexPointer;
    int index2;
    firstElemHashValue.Get(hashsum, indexPointer);
    index2 = *indexPointer;
#ifdef DEBUG
cout << "  Statusbericht InsertIndex-Funktion" << endl
     << "    Hashsum: " << hashsum << endl
     << "    Index: " << index2 << endl;
#endif
    if(index2<0) {
      firstElemHashValue.Put(hashsum, index);
    } else {
      int indexOld;
      while(index2>-1) {
#ifdef DEBUG
cout << "  Statusbericht InsertIndex-Funktion" << endl
     << "    Index: " << index2 << endl;
#endif
        nextElemHashValue.Get(index2, indexPointer);
        indexOld = index2;
        index2 = *indexPointer;
      }
      nextElemHashValue.Put(indexOld, index);
    }
    nextElemHashValue.Put(index, -1);
  }


  void Collection::AddHashValue(const int value, const int count) {
    hashValue += count*value;
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
                             nl->StringAtom("("+VECTOR+" real)"),
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
                             nl->StringAtom("("+SET+" real)"),
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
                             nl->StringAtom("("+MULTISET+" real)"),
                             nl->StringAtom("((elem1 count1) .. "
                             "(elem_n count_n))"),
                             nl->StringAtom("((2.839 2) (3.12 1) (25.123 1))"),
                             nl->StringAtom("All elements must be of the"
                             "same type."))));
  }


  TypeConstructor vectorTC(
      VECTOR, VectorProperty,
      Collection::Out, Collection::In,
      0, 0,
      Collection::Create, Collection::Delete,
      Collection::Open, Collection::Save,
      Collection::Close, Collection::Clone,
      Collection::Cast, Collection::SizeOfObj,
      Collection::KindCheck);

  TypeConstructor setTC(
      SET, SetProperty,
      Collection::Out, Collection::In,
      0, 0,
      Collection::Create, Collection::Delete,
      Collection::Open, Collection::Save,
      Collection::Close, Collection::Clone,
      Collection::Cast, Collection::SizeOfObj,
      Collection::KindCheck);

  TypeConstructor multisetTC(
      MULTISET, MultisetProperty,
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
    string opName = (contains?CONTAINS:IN);
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
    if((type==VECTOR)||(type==SET)) {
      return nl->SymbolAtom(BOOL);
    } else if(type==MULTISET) {
      return nl->SymbolAtom(INT);
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
    Collection* coll;
    Attribute* elem;
    if(contains) {
      coll = static_cast<Collection*>(args[0].addr);
      elem = static_cast<Attribute*>(args[1].addr);
    } else {
      coll = static_cast<Collection*>(args[1].addr);
      elem = static_cast<Attribute*>(args[0].addr);
    }
    result = qp->ResultStorage(s);
    int contained = coll->Contains(elem);
    CcInt* count = new CcInt(true, contained);
    CcBool* b = new CcBool(true, (contained==1));
    if(coll->GetMyCollType()==multiset) {
      result.addr = count;
    } else {
      result.addr = b;
    }
    return 0;
  }

  struct containsInfo : OperatorInfo {

    containsInfo()
    {
      name      = CONTAINS;
      signature = VECTOR + "(t) x t -> " + BOOL;
      appendSignature(SET + "(t) x t -> " + BOOL);
      appendSignature(MULTISET + "(t) x t -> " + INT);
      syntax    = "_" + CONTAINS + "_";
      meaning   = "Contains predicate.";
    }

  };

  struct inInfo : OperatorInfo {

    inInfo()
    {
      name      = IN;
      signature = "t x " + VECTOR + "(t) -> " + BOOL;
      appendSignature("t x " + SET + "(t) -> " + BOOL);
      appendSignature("t x " + MULTISET + "(t) -> " + BOOL);
      syntax    = "_" + IN + "_";
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
    string opName = (insert?INSERT:ADD);
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
    if((type==VECTOR)==(!insert)) {
      return coll;
    }
    if(insert) {
      ErrorReporter::ReportError("Operator " + INSERT + " expects a "
                                + "set or multiset as first argument.");
    } else {
      ErrorReporter::ReportError("Operator " + ADD + " expects a "
                                + "vector as first argument.");
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
    Collection* resColl = new Collection(*coll);
    resColl->Insert(elem, 1);
    result = qp->ResultStorage(s);
    result.addr = resColl;
    return 0;
  }

  struct insertInfo : OperatorInfo {

    insertInfo()
    {
      name      = INSERT;
      signature = SET + "(t) x t -> " + SET + "(t)";
      appendSignature(MULTISET + "(t) x t -> " + MULTISET + "(t)");
      syntax    = "_" + INSERT + "_";
      meaning   = "Inserts the second argument in the first.";
    }

  };

  struct addInfo : OperatorInfo {

    addInfo()
    {
      name      = ADD;
      signature = VECTOR + "(t) x t -> " + VECTOR + "(t)";
      syntax    = "_" + ADD + "_";
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
      case vector:
        resultType = VECTOR;
        break;
      case set:
        resultType = SET;
        break;
      default:
        resultType = MULTISET;
    }
    string opName = CREATE_PREFIX + resultType;
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
    if(!am->CheckKind("DATA", type, errorInfo)) {
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
    int sons = qp->GetNoSons(s);
    Collection* coll = static_cast<Collection*>(result.addr);
    Collection* resultColl = new Collection(*coll, true);
    int i = 0;
    while(i < sons) {
      Attribute* elem = static_cast<Attribute*>(args[i].addr);
      resultColl->Insert(elem, 1);
      i++;
    }
    resultColl->Finish();
    result.addr = resultColl;
    return 0;
  }

  struct CreateVectorInfo : OperatorInfo {

    CreateVectorInfo()
    {
      name      = CREATE_PREFIX + VECTOR;
      signature = "t+ -> " + VECTOR + "(t)";
      syntax    = CREATE_PREFIX + VECTOR + "(_, _)";
      meaning   = "Creates a " + VECTOR + " of t.";
    }
  };

  struct CreateSetInfo : OperatorInfo {

    CreateSetInfo()
    {
      name      = CREATE_PREFIX + SET;
      signature = "t+ -> " + SET + "(t)";
      syntax    = CREATE_PREFIX + SET + "(_, _)";
      meaning   = "Creates a " + SET + " of t.";
    }
  };

  struct CreateMultisetInfo : OperatorInfo {

    CreateMultisetInfo()
    {
      name      = CREATE_PREFIX + MULTISET;
      signature = "t+ -> " + MULTISET + "(t)";
      syntax    = CREATE_PREFIX + MULTISET + "(_, _)";
      meaning   = "Creates a " + MULTISET + " of t.";
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
      case vector:
        operatorName = "collect_vector";
        resultType = "vector";
        break;
      case set:
        operatorName = "collect_set";
        resultType = "set";
        break;
      case multiset:
        operatorName = "collect_multiset";
        resultType = "multiset";
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

            if ( nl->IsEqual(nl->First(argStream), STREAM)
               && am->CheckKind("DATA", argType, errorInfo) ){
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
    Collection* coll = static_cast<Collection*>(result.addr);
    Collection* resColl = new Collection(*coll, true);
    Attribute* elemToInsert;
    Word elem = SetWord(Address(0));

    qp->Open(args[0].addr);
    qp->Request(args[0].addr, elem);

    while ( qp->Received(args[0].addr) )
    {
        elemToInsert = (Attribute*) elem.addr;
        resColl->Insert(elemToInsert, 1);
        elemToInsert->DeleteIfAllowed();
        qp->Request(args[0].addr, elem);
    }
    resColl->Finish();
    result.addr = resColl;

    qp->Close(args[0].addr);

    return 0;
  }

  struct collectSetInfo : OperatorInfo {

    collectSetInfo()
    {
      name      = COLLECT_SET;
      signature = STREAM + "(t) -> " + SET + "(t)";
      syntax    = "_ " + COLLECT_SET;
      meaning   = "Collects the stream elements into a new set";
    }

  };

  struct collectMultisetInfo : OperatorInfo {

    collectMultisetInfo()
    {
      name      = COLLECT_MULTISET;
      signature = STREAM + "(t) -> " + COLLECT_MULTISET + "(t)";
      syntax    = "_ " + COLLECT_MULTISET;
      meaning   = "Collects the stream elements into a new multiset";
    }

  };

  struct collectVectorInfo : OperatorInfo {

    collectVectorInfo()
    {
      name      = COLLECT_VECTOR;
      signature = STREAM + "(t) -> " + VECTOR + "(t)";
      syntax    = "_ " + COLLECT_VECTOR;
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
                (nl->IsEqual(nl->First(argCollection), VECTOR) ||
                nl->IsEqual(nl->First(argCollection), SET) ||
                nl->IsEqual(nl->First(argCollection), MULTISET))
               && am->CheckKind("DATA", argType, errorInfo) ){
                return nl->TwoElemList(
                                   nl->SymbolAtom(STREAM),
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
          if ( local.addr == 0 )
            return CANCEL;

          linfo = (ComponentsLocalInfo*)local.addr;
          if (linfo->componentCount <= 0){
            if(linfo->componentIndex + 1 >= coll->GetNoUniqueComponents()){
                return CANCEL;
            }else{
                linfo->componentIndex++;
                linfo->componentCount =
                            coll->GetComponentCount(linfo->componentIndex);
            }
          }

          result = SetWord((coll->GetComponent(
                                    linfo->componentIndex))->Clone());
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
      name      = COMPONENTS;
      signature = VECTOR + "(t) -> " + STREAM + "(t)";
      appendSignature(SET + "(t) -> " + STREAM + "(t)");
      appendSignature(MULTISET + "(t) -> " + STREAM + "(t)");
      syntax    = COMPONENTS + "(_)";
      meaning   = "Takes the elements from the Collection into a stream";
    }

  };

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

        if(!(nl->IsAtom(argIndex) && nl->IsEqual(argIndex, INT)))
        {
            ErrorReporter::ReportError(errMsg);
            return nl->TypeError();
        }


        if ( !nl->IsAtom(argCollection) && nl->ListLength(argCollection) == 2)
        {
            argType = nl->Second(argCollection);

            if (
                (nl->IsEqual(nl->First(argCollection), VECTOR))
               && am->CheckKind("DATA", argType, errorInfo) ){
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

    Attribute* resAttribute;
    result = qp->ResultStorage(s);

    if(sourceColl->GetNoComponents() <= indexVal || indexVal < 0){
        ((Attribute*)result.addr)->SetDefined(false);
    }else{
        resAttribute = sourceColl->GetComponent(indexVal);
        result.addr = resAttribute;
    }

    return 0;
  }

  struct getInfo : OperatorInfo {

    getInfo()
    {
      name      = GET;
      signature = VECTOR + "(t) x int -> t";
      syntax    = GET + "( _, _ )";
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
      const string errMsg = "Operator " + DELETEELEM +
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
                (nl->IsEqual(nl->First(argCollection), SET) ||
                nl->IsEqual(nl->First(argCollection), MULTISET))
               && am->CheckKind("DATA", argCollectionType, errorInfo)
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

    result = qp->ResultStorage(s);
    Collection* coll = static_cast<Collection*>(result.addr);
    Collection* resColl = new Collection(*coll, true);

    for(int eCnt = 0; eCnt < sourceColl->GetNoUniqueComponents(); eCnt++){
        int componentCount = sourceColl->GetComponentCount(eCnt);

        if(sourceColl->GetComponent(eCnt)->Compare(elemToDelete) == 0){
            componentCount--;
        }

        if(componentCount > 0){
            resColl->Insert(sourceColl->GetComponent(eCnt), componentCount);
        }
    }
    resColl->Finish();
    result.addr = resColl;

    return 0;
  }

  struct deleteInfo : OperatorInfo {

    deleteInfo()
    {
      name      = DELETEELEM;
      signature = SET + "(t) x t -> " + SET + "(t)";
      appendSignature(MULTISET + "(t) x t -> " + MULTISET + "(t)");
      syntax    = DELETEELEM + "( _, _ )";
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
                nl->IsEqual(nl->First(argCollection1), VECTOR)
               && am->CheckKind("DATA", argCollectionType, errorInfo)
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

    Collection* resVector = new Collection(*vector1);

    for(int eCnt = 0; eCnt < vector2->GetNoUniqueComponents(); eCnt++){
        resVector->Insert(vector2->GetComponent(eCnt), 1);
    }
    resVector->Finish();
    result = qp->ResultStorage(s);
    result.addr = resVector;

    return 0;
  }

  struct concatInfo : OperatorInfo {

    concatInfo()
    {
      name      = CONCAT;
      signature = VECTOR + "(t) x " + VECTOR + "(t) -> " + VECTOR + "(t)";
      syntax    = "_ _ " + CONCAT;
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
                (nl->IsEqual(nl->First(argCollection1), SET)
                 || nl->IsEqual(nl->First(argCollection1), MULTISET))
               && am->CheckKind("DATA", argCollectionType, errorInfo)
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

    result = qp->ResultStorage(s);
    Collection* coll = static_cast<Collection*>(result.addr);
    Collection* resColl = new Collection(*coll, true);

    int insertCnt;

    int noColl1Components = coll1->GetNoUniqueComponents();
    int noColl2Components = coll2->GetNoUniqueComponents();

    int elementIdx1 = 0;
    int elementIdx2 = 0;

    bool coll1Ended = noColl1Components <= elementIdx1;
    bool coll2Ended = noColl2Components <= elementIdx2;

    while(!coll1Ended && !coll2Ended){
      int compareRes = coll1->GetComponent(elementIdx1)->Compare(
                                coll2->GetComponent(elementIdx2));

      switch(compareRes){
      case -1:
        switch(opType){
        case unionOp:
          resColl->Insert(coll1->GetComponent(elementIdx1),
                          coll1->GetComponentCount(elementIdx1));
          break;
        case differenceOp:
          resColl->Insert(coll1->GetComponent(elementIdx1),
                          coll1->GetComponentCount(elementIdx1));
          break;
        default:
          break;
        }
        elementIdx1++;
        break;
      case 0:
        switch(opType){
        case unionOp:
          resColl->Insert(coll1->GetComponent(elementIdx1),
                            coll1->GetComponentCount(elementIdx1)
                            + coll2->GetComponentCount(elementIdx2));
          break;
        case intersectionOp:
          insertCnt =
                 coll1->GetComponentCount(elementIdx1) >
                          coll2->GetComponentCount(elementIdx2) ?
                          coll2->GetComponentCount(elementIdx2) :
                          coll1->GetComponentCount(elementIdx1);
          resColl->Insert(coll1->GetComponent(elementIdx1), insertCnt);
          break;
        case differenceOp:
          insertCnt = coll1->GetComponentCount(elementIdx1) -
                    coll2->GetComponentCount(elementIdx2);
          if(insertCnt > 0){
            resColl->Insert(coll1->GetComponent(elementIdx1), insertCnt);
          }
          break;
        default:
          break;
        }
        elementIdx1++;
        elementIdx2++;
        break;
      case 1:
        switch(opType){
        case unionOp:
          resColl->Insert(coll2->GetComponent(elementIdx2),
                          coll2->GetComponentCount(elementIdx2));
        default:
          break;
        }
        elementIdx2++;
        break;
      default:
        break;
      }

      coll1Ended = noColl1Components <= elementIdx1;
      coll2Ended = noColl2Components <= elementIdx2;
    }

    for(int iCnt1 = elementIdx1;
                            iCnt1 < coll1->GetNoUniqueComponents(); iCnt1++){
       switch(opType){
        case unionOp:
        case differenceOp:
          resColl->Insert(coll1->GetComponent(iCnt1),
                          coll1->GetComponentCount(iCnt1));
          break;
        default:
          break;
        }
    }

    for(int iCnt2 = elementIdx2;
                            iCnt2 < coll2->GetNoUniqueComponents(); iCnt2++){
       switch(opType){
        case unionOp:
          resColl->Insert(coll2->GetComponent(iCnt2),
                          coll2->GetComponentCount(iCnt2));
          break;
        default:
          break;
        }
    }
    resColl->Finish();
    result.addr = resColl;

    return 0;
  }

  struct unionInfo : OperatorInfo {

    unionInfo()
    {
      name      = UNION;
      signature = SET + "(t) x " + SET + "(t) -> " + SET + "(t)";
      appendSignature(MULTISET + "(t) x " + MULTISET + "(t) -> "
                                            + MULTISET + "(t)");
      syntax    = "_" + UNION + "_";
      meaning   = "assigns the union-operation on two sets or multisets";
    }

  };

  struct intersectionInfo : OperatorInfo {

    intersectionInfo()
    {
      name      = INTERSECTION;
      signature = SET + "(t) x " + SET + "(t) -> " + SET + "(t)";
      appendSignature(MULTISET + "(t) x " + MULTISET + "(t) -> "
                                            + MULTISET + "(t)");
      syntax  = INTERSECTION + "( _, _)";
      meaning = "assigns the intersection-operation on two sets or multisets";
    }

  };

  struct differenceInfo : OperatorInfo {

    differenceInfo()
    {
      name      = DIFFERENCE;
      signature = SET + "(t) x " + SET + "(t) -> " + SET +
      "(t)";
      appendSignature(MULTISET + "(t) x " + MULTISET +
      "(t) -> "  + MULTISET +
        "(t)");
      syntax    = DIFFERENCE + "( _, _)";
      meaning   = "assigns the difference-operation on two sets or multisets";
    }

  };

/*
4.10 Implementation of operation size

*/
int sizeFun_PR(Word* args, Word& result, int message, Word& local, Supplier s){
Collection* co = (Collection*)args[0].addr;
result = qp->ResultStorage(s);
((CcInt*)result.addr)->Set(true, co->GetNoComponents());
return 0;
}

int sizeFun_RR(Word* args, Word& result, int message, Word& local, Supplier s){
Collection* co = (Collection*)args[0].addr;
result = qp->ResultStorage(s);
((CcInt*)result.addr)->Set(true, co->GetNoComponents());
return 0;
}

int sizeFun_SR(Word* args, Word& result, int message, Word& local, Supplier s){
Collection* co = (Collection*)args[0].addr;
result = qp->ResultStorage(s);
((CcInt*)result.addr)->Set(true, co->GetNoComponents());
return 0;
}


ListExpr sizeTypeMap(ListExpr args){
if (!nl->ListLength(args) == 0)
  {
    ListExpr arg1 = nl->First(args);

    if (!nl->IsAtom(arg1) && (nl->IsEqual(nl->First(arg1),
       "set") || nl->IsEqual(nl->First(arg1), "multiset")
       || nl->IsEqual(nl->First(arg1), "vector"))) {
      return nl->SymbolAtom("int");
    }
  }

  return nl->SymbolAtom("typeerror");

}


int sizeSelect(ListExpr args){
  NList list(args);
  if(list.first().isSymbol(SET))
    return 2;
  else if(list.first().isSymbol(MULTISET))
    return 1;
  else
    return 0;
}


struct sizeInfo : OperatorInfo {

  sizeInfo() : OperatorInfo()
  {
    name      = SIZE;
    signature = SET + " -> " + INT;
    appendSignature(MULTISET + " -> " + INT);
    appendSignature(VECTOR + " -> " + INT);
    syntax    = SIZE + "( _ )";
    meaning   = "Size of Object";
  }

};

/*
4.11 Implementation of operators <, <=, >, >= and =

*/
int ltFun_PR(Word* args, Word& result, int message, Word& local, Supplier s){
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

int ltFun_RR(Word* args, Word& result, int message, Word& local, Supplier s){
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

ListExpr ltTypeMap(ListExpr args){
if (!nl->ListLength(args) == 0)
  {
    ListExpr arg1 = nl->First(args);
    ListExpr arg2 = nl->Second(args);

    if (!nl->IsAtom(arg1) &&
      ((nl->IsEqual(nl->First(arg1), "set") ||
      (nl->IsEqual(nl->First(arg1), "multiset")) &&
      nl->Equal(arg1, arg2)))) {
      return nl->SymbolAtom("bool");
    }
  }

  return nl->SymbolAtom("typeerror");

}

int ltSelect(ListExpr args){
  NList list(args);
  if(list.first().isSymbol(SET))
    return 1;
  else
    return 0;
}

struct ltInfo : OperatorInfo {

  ltInfo() : OperatorInfo()
  {
    name      = LT;
    signature = SET + " x " + SET + " -> " + BOOL;
    appendSignature(MULTISET + " x " + MULTISET + " -> " + BOOL);
    syntax    = " _ " + LT + " _ ";
    meaning   = "Object 1 Smaller Than Object 2";
  }

};

int eqFun_PR(Word* args, Word& result, int message, Word& local, Supplier s){
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

int eqFun_RR(Word* args, Word& result, int message, Word& local, Supplier s){
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

ListExpr eqTypeMap(ListExpr args){
if (!nl->ListLength(args) == 0)
  {
    ListExpr arg1 = nl->First(args);
    ListExpr arg2 = nl->Second(args);

    if (!nl->IsAtom(arg1) &&
      ((nl->IsEqual(nl->First(arg1), "set") ||
      (nl->IsEqual(nl->First(arg1), "multiset")) &&
      nl->Equal(arg1, arg2)))) {
      return nl->SymbolAtom("bool");
    }
  }

  return nl->SymbolAtom("typeerror");

}

int eqSelect(ListExpr args){
  NList list(args);
  if(list.first().isSymbol(SET))
    return 1;
  else
    return 0;
}

struct eqInfo : OperatorInfo {

  eqInfo() : OperatorInfo()
  {
    name      = EQ;
    signature = SET + " x " + SET + " -> " + BOOL;
    appendSignature(MULTISET + " x " + MULTISET + " -> " + BOOL);
    syntax    = " _ " + EQ + " _ ";
    meaning   = "Object 1 equals Object 2";
  }

};

int gtFun_PR(Word* args, Word& result, int message, Word& local, Supplier s){
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

int gtFun_RR(Word* args, Word& result, int message, Word& local, Supplier s){
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

ListExpr gtTypeMap(ListExpr args){
if (!nl->ListLength(args) == 0)
  {
    ListExpr arg1 = nl->First(args);
    ListExpr arg2 = nl->Second(args);

    if (!nl->IsAtom(arg1) &&
      ((nl->IsEqual(nl->First(arg1), "set") ||
      (nl->IsEqual(nl->First(arg1), "multiset")) &&
      nl->Equal(arg1, arg2)))) {
      return nl->SymbolAtom("bool");
    }
  }

  return nl->SymbolAtom("typeerror");

}

int gtSelect(ListExpr args){
  NList list(args);
  if(list.first().isSymbol(SET))
    return 1;
  else
    return 0;
}

struct gtInfo : OperatorInfo {

  gtInfo() : OperatorInfo()
  {
    name      = GT;
    signature = SET + " x " + SET + " -> " + BOOL;
    appendSignature(MULTISET + " x " + MULTISET + " -> " + BOOL);
    syntax    = " _ " + GT + " _ ";
    meaning   = "Object 1 Greater Than Object 2";
  }

};

int leFun_PR(Word* args, Word& result, int message, Word& local, Supplier s){
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

int leFun_RR(Word* args, Word& result, int message, Word& local, Supplier s){
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

ListExpr leTypeMap(ListExpr args){
if (!nl->ListLength(args) == 0)
  {
    ListExpr arg1 = nl->First(args);
    ListExpr arg2 = nl->Second(args);

    if (!nl->IsAtom(arg1) &&
      ((nl->IsEqual(nl->First(arg1), "set") ||
      (nl->IsEqual(nl->First(arg1), "multiset")) &&
      nl->Equal(arg1, arg2)))) {
      return nl->SymbolAtom("bool");
    }
  }

  return nl->SymbolAtom("typeerror");

}

int leSelect(ListExpr args){
  NList list(args);
  if(list.first().isSymbol(SET))
    return 1;
  else
    return 0;
}

struct leInfo : OperatorInfo {

  leInfo() : OperatorInfo()
  {
    name      = LE;
    signature = SET + " x " + SET + " -> " + BOOL;
    appendSignature(MULTISET + " x " + MULTISET + " -> " + BOOL);
    syntax    = " _ " + LE + " _ ";
    meaning   = "Object 1 Smaller equal Object 2";
  }

};

int geFun_PR(Word* args, Word& result, int message, Word& local, Supplier s){
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

int geFun_RR(Word* args, Word& result, int message, Word& local, Supplier s){
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

ListExpr geTypeMap(ListExpr args){
if (!nl->ListLength(args) == 0)
  {
    ListExpr arg1 = nl->First(args);
    ListExpr arg2 = nl->Second(args);

    if (!nl->IsAtom(arg1) &&
      ((nl->IsEqual(nl->First(arg1), "set") ||
      (nl->IsEqual(nl->First(arg1), "multiset")) &&
      nl->Equal(arg1, arg2)))) {
      return nl->SymbolAtom("bool");
    }
  }

  return nl->SymbolAtom("typeerror");

}


int geSelect(ListExpr args){
  NList list(args);
  if(list.first().isSymbol(SET))
    return 1;
  else
    return 0;
}

struct geInfo : OperatorInfo {

  geInfo() : OperatorInfo()
  {
    name      = GE;
    signature = SET + " x " + SET + " -> " + BOOL;
    appendSignature(MULTISET + " x " + MULTISET + " -> " + BOOL);
    syntax    = " _ " + GE + " _ ";
    meaning   = "Object 1 Greater equal Object 2";
  }

};

/*
4.12 Implementation of operator is\_defined

*/
int isdefFun(Word* args, Word& result, int message, Word& local, Supplier s){
bool r = false;
Collection* co = (Collection*)args[0].addr;
result = qp->ResultStorage(s);
if(co->IsDefined())
  r = true;
((CcBool*)result.addr)->Set(true, r);
return 0;
}

ListExpr isdefTypeMap(ListExpr args){
if (!nl->ListLength(args) == 0)
  {
    ListExpr arg1 = nl->First(args);

    if (!nl->IsAtom(arg1) && (nl->IsEqual(nl->First(arg1),
        "set") || nl->IsEqual(nl->First(arg1), "multiset")
        || nl->IsEqual(nl->First(arg1), "vector"))) {
      return nl->SymbolAtom("bool");
    }
  }

  return nl->SymbolAtom("typeerror");

}


struct isdefInfo : OperatorInfo {

  isdefInfo() : OperatorInfo()
  {
    name      = ISDEF;
    signature = SET +  " -> " + BOOL;
    appendSignature(MULTISET + " -> " + BOOL);
    appendSignature(VECTOR + " -> " + BOOL);
    syntax    = " _ " + ISDEF;
    meaning   = "isdefined wird abgefragt";
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

      vectorTC.AssociateKind("DATA");
      setTC.AssociateKind("DATA");
      multisetTC.AssociateKind("DATA");

      AddOperator(containsInfo(), ContainsInValueMap<true>,
                  ContainsInTypeMap<true>);
      AddOperator(inInfo(), ContainsInValueMap<false>,
                  ContainsInTypeMap<false>);
      AddOperator(insertInfo(), InsertValueMap<true>,
                  InsertTypeMap<true>);
      AddOperator(addInfo(), InsertValueMap<false>,
                  InsertTypeMap<false>);
      AddOperator(CreateVectorInfo(), CreateValueMap,
                  CreateTypeMap<vector>);
      AddOperator(CreateSetInfo(), CreateValueMap,
                  CreateTypeMap<set>);
      AddOperator(CreateMultisetInfo(), CreateValueMap,
                  CreateTypeMap<multiset>);
      AddOperator(collectSetInfo(), CollectValueMap<set>,
                  CollectTypeMap<set>);
      AddOperator(collectMultisetInfo(), CollectValueMap<multiset>,
                  CollectTypeMap<multiset>);
      AddOperator(collectVectorInfo(), CollectValueMap<vector>,
                  CollectTypeMap<vector>);
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
      ValueMapping sizeFuns[] = { sizeFun_PR, sizeFun_RR, sizeFun_SR, 0 };
      AddOperator( sizeInfo(), sizeFuns, sizeSelect, sizeTypeMap );
      ValueMapping eqFuns[] = { eqFun_PR, eqFun_RR, 0 };
      AddOperator( eqInfo(), eqFuns, eqSelect, eqTypeMap );
      ValueMapping gtFuns[] = { gtFun_PR, gtFun_RR, 0 };
      AddOperator( gtInfo(), gtFuns, gtSelect, gtTypeMap );
      ValueMapping ltFuns[] = { ltFun_PR, ltFun_RR, 0 };
      AddOperator( ltInfo(), ltFuns, ltSelect, ltTypeMap );
      ValueMapping geFuns[] = { geFun_PR, geFun_RR, 0 };
      AddOperator( geInfo(), geFuns, geSelect, geTypeMap );
      ValueMapping leFuns[] = { leFun_PR, leFun_RR, 0 };
      AddOperator( leInfo(), leFuns, leSelect, leTypeMap );
      AddOperator( isdefInfo(), isdefFun, isdefTypeMap);

    }
    ~CollectionAlgebra() {};
  };

} //end of namespace collection


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

