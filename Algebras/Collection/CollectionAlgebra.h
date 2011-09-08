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

[toc]

1 Collection class implementation
see CollectionAlgebra.cpp for details.

*/
#ifndef _COLLECTIONALGEBRA_H_
#define _COLLECTIONALGEBRA_H_

#include "Algebra.h"
#include "NestedList.h"
#include "NList.h"
#include "LogMsg.h"
#include "QueryProcessor.h"
#include "ConstructorTemplates.h"
#include "StandardTypes.h"
#include "Attribute.h"
#include "../../Tools/Flob/Flob.h"
#include "../../Tools/Flob/DbArray.h"


extern NestedList* nl;
extern QueryProcessor* qp;

#include "TypeMapUtils.h"
#include "Symbols.h"

using namespace mappings;

#include <vector>
#include <string>

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

    static bool checkType(const ListExpr list, const string& basicType){
       return nl->HasLength(list,2) &&
              listutils::isSymbol(nl->First(list), basicType) &&
              listutils::isDATA(nl->Second(list));
    }


    static int SizeOfObj();

    size_t Sizeof() const;

    int NumOfFLOBs() const;

    Flob* GetFLOB(const int i);

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

    void Clear();

/*
Resets the Collection to well-defined settings.
Does NOT change the ~defined~ flag!

*/


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

    DbArray<size_t> elemFLOBDataOffset;
/*
We use this DBArray to save the offsets at which the FLOB contents of the
elements are saved in elementData;

*/

    DbArray<int> elemCount;
/*
This DBArray is used only, if our collection is a multiset, to save the count
of every element.

*/

    DbArray<int> elemArrayIndex;
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

    DbArray<int> firstElemHashValue;
    DbArray<int> nextElemHashValue;
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

    Flob elements;
    Flob elementData;
/*
In this FLOB we save the data of our elements.

*/

  }; //end of class Collection


} //end of namespace collection

/*
Functions returning the Secondo type names

*/
namespace Vector{
  const string BasicType();
  const bool checkType(ListExpr list);
}

namespace Set{
  const string BasicType();
  const bool checkType(ListExpr list);
}

namespace Multiset{
  const string BasicType();
  const bool checkType(ListExpr list);
}


#endif // _COLLECTIONALGEBRA_H_
