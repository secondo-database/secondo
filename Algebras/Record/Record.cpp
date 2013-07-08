/*
----
This file is part of SECONDO.

Copyright (C) 2004, University in Hagen, Department of Computer Science,
Database Systems for New Applications.

SECONDO is free software{} you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation{} either version 2 of the License, or
(at your option) any later version.

SECONDO is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY{} without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with SECONDO{} if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
----

//paragraph [1] Title: [{\Large \bf \begin {center}] [\end {center}}]
//[TOC] [\tableofcontents]

[1] .cpp file of type Record

December 2009,  B. Franzkowiak

[TOC]

1 Overview

Within this class a new structured data type named ~Record~ is implemented into
SECONDO. Elements of already existing attributes of kind DATA can be summerized
to a new data type likewise a structure. A Record object is of kind DATA as
well and therefore a record can contain other records.

In particular records should be used to import geographical data respectively
of data type given by EOO and NMEA0183 data protocolls. An object of type Record
should have the ability to hold the specified data given by a E00 or NMEA0183
geodata object.

Additionally the fact that elements that belong to a record can contain FLOBs
has to be considered.
To avoid uncontrolled groth of such FLOBs they have to be handled
by the record administration.

4 Concept

                Figure 1: Representation of a ~Record~ [Record.cpp.Figure1.eps]

4.1 Record type structure

A ~Record~ is derived from ~Attribute~ as it has to be used in relations.
Therefore it
is of type ~DATA~ as well and contains amongst others information about:

  * Its hash value ~size\_t Hashvalue~ for the whole record

  * Its numbers of elements ~int noElements~ in the record

  * A structure ~struct ElemInfo~ to describe the record elements in detail
(see below)

  * An array ~DBArray elemInfoArray~ contains all structured meta information
about the elements (see below)

  * Two ~FLOB~s (see below)

    1 ~elemData~ containing in fact the data of the elements

    2 ~elemExData~ containing in fact all Flobs of all elements as far as
elements hold FLOBs

4.2 Structure ~struct ElemInfo~

Using a structure to describe the specific details of each element a flexible
and generic build up of a record is possible. For each element a related
structure is allocated and saved in the DBArray ~elemInfoArray~.
The structure
contains the following detailed information:

  * ~char elemName~ name of the element, e.g. age

  * ~char elemType~ type of the element, e.g. int

  * ~int algebraId~ id of the algebra of the given type, e.g. 0

  * ~int typeId~ id of typeId of the given type, e.g. 3

  * ~bool hasData~ information if the element contains data or is empty

  * ~size\_t dataOffSet~ position where the saved data is located in the FLOB
~elemData~

  * ~size\_t extDataOffSet~ position where the FLOBS owned by the element
are located in the FLOB ~elemExData~


4.3 Array ~DBArray elemInfoArray~

This array is used to save the element information data. For each element a new
structure element ~ElemenInfo~ is created and saved in the array
~elemInfoArray~. So the record itself always knows about its metadata.
This information is saved in an own array independend of the data by itself.
Therefore it is possible to administrate the data FLOBs.


4.4 Element FLOB ~elemData~

The element data in fact is stored in a FLOB called ~elemData~.
To assure that the
element or respective the ~elemInfoArray~ knows where the related data is
saved in the FLOB, the position in the FLOB the so called offset is stored
in the structure ~ElemInfo~ in ~dataOffSet~. Together with the
~dataOffSet~ and the ~elemData~ FLOB the element can be saved
and rebuilt.

4.5 External element FLOB ~elemExData~

Apparently an element can consist of FLOBs as well.
To administrate an effective
repository management fot all given FLOBs of all existing
record elements another FLOB
~elemExData~is required. Within this additional FLOB all occurring
FLOBs are saved. To assure that the element or respective the
~elemInfoArray~ knows where the related FLOBs are saved, the offset
of the beginning of the elements FLOB area is stored in the structure
~ElemInfo~ in ~extDataOffSet~.

3 handy Record example

List expression definition of the Record:

Type Description: (record (name1 typ1) (name2 type2) ... (nameN typeN))

Elements:         ($<elem1> <elem2> ... <elemN>$)

Handy example:

Description: (record (name string) (age int) (amount real))

Elements:    ("Mayer" 42 1023.08)

let rec3 = [const record (name: string, age: int, amount: real)
            value ("Schulz" 23 28.0)]


4 Source code area

4.1 Defines and includes

The following includes are neccessary:

*/

#include "Record.h"
#include "Algebra.h"
#include "NestedList.h"
#include "NList.h"
#include "LogMsg.h"
#include "QueryProcessor.h"
#include "StandardTypes.h"
#include "SecondoCatalog.h"
#include "ListIterator.h"
#include <cctype>
#include "Symbols.h"
#include "ListUtils.h"


using namespace std;

// enable/disable debug output to standard error
#undef RECORD_DEBUG
//#define RECORD_DEBUG

/*
External references to a nested list and query processor instance are needed:

*/
extern NestedList* nl;
extern QueryProcessor* qp;

/*
To assure a correct hashing two constants are needed:

*/
static const int HASH_INIT = 17;
static const int HASH_MULTI = 59;


/*
5.1 Area with Record constructor and destructor

The class Record provides the following constructors and deconstructor:

  * Record

  * Record(int initSize)

  * Record()

Standard constructor:

*/
Record::Record()
{
#ifdef RECORD_DEBUG
  cerr << "Record::Record()" << endl;
#endif
}

/*
Constructor ~Record(int initSize)~ initialises the record with a given
constant hash value HASHINIT = 17 for a given number of elements.

*/
Record::Record(int initSize)
  : Attribute(true)
  , hashValue(HASH_INIT)
  , noElements(0)
  , elemInfoArray(initSize)
  , elemData(0)
  , elemExtData(0)
{
#ifdef RECORD_DEBUG
  cerr << "Record::Record(" << initSize << ")" << endl;
#endif

  // set as defined
  this->SetDefined(true);
}

/*
Deconstructor ~[~]Record()~

Destroys the Record and cleans up the memory of ~elemInfoArray~,
~elemData~, ~elemExtData~.

*/
Record::~Record() { }

/*
6.1 Public record method area

The class Record  provides the following public methods:

  * GetNoElements

  * GetElement

  * GetElementName

  * GetElementTypeName

  * SetElement

  * AppendElement

*/

/*
~GetNoElements~ returns the number of elements in this record.

*/
int
Record::GetNoElements() const
{
#ifdef RECORD_DEBUG
  cerr << "Record::GetNoElements(): noElements=" << this->noElements << endl;
#endif

  return this->noElements;
}

/*

~GetElement~ returns the element at position pos.

With the help of the element info ~elemInfo~ it is possible to rebuild the
attribute that is stored at a given record position. First the ~elemInfo~
is refilled with the information of ~elemInfoArray~ at the given position.
It is now possible to create the attribute with the correct type and name.
As the position in ~elemData~ is known as well the data can be assigned to
the attribute. Finally the external data if available can be assigned to the
attribute as well.

*/
Attribute*
Record::GetElement(int pos) const
{

  assert(pos >=0);
  assert(pos < this->elemInfoArray.Size());

  // get element info by position
  ElemInfo elemInfo;
  elemInfoArray.Get(pos, elemInfo);

  if(!elemInfo.hasData){
     return 0;
  } else {
    // create requested element
    // TODO: It's not a good idea to have no type information during
    //       object creation for some types
    Attribute* elem = static_cast<Attribute*>
            ((am->CreateObj(elemInfo.algebraId, elemInfo.typeId))(0).addr);

    // save the flob states
    vector<Flob> savedFlobs;
    for(int i=0;i<elem->NumOfFLOBs();i++){
      savedFlobs.push_back(*elem->GetFLOB(i));
    }

    this->elemData.read((char*)elem, elem->Sizeof(),elemInfo.dataOffset);
    // assign retrieved data to element
    elem = static_cast<Attribute*>(
         (am->Cast(elemInfo.algebraId, elemInfo.typeId))((void*) elem));

    // restore the flob states
    for(int i=0;i<elem->NumOfFLOBs();i++){
      int size = elem->GetFLOB(i)->getSize();
      elem->GetFLOB(i)->kill();
      *(elem->GetFLOB(i)) = savedFlobs[i];
      elem->GetFLOB(i)->resize(size);
    }


    // assign external data offset
    size_t offset = elemInfo.extDataOffset;

    // restore flob data
    for (int i = 0; i < elem->NumOfFLOBs(); i++) {
      Flob* flob = elem->GetFLOB(i);
      char* buffer = new char[flob->getSize()];
      bool ok = elemExtData.read(buffer, flob->getSize(), offset);
      assert(ok);

      // assign retrieved data to target flob
      flob->write(buffer,  flob->getSize(), 0);
      delete[] buffer;
      // update external data offset
      offset += flob->getSize();
    }

    return elem;
  }
}

/*
~GetElementName~ returns the element name at position pos.

With the help of the element info ~elemInfo~ it is possible to select the
element name of an element given at a certain record position.

*/
const string
Record::GetElementName(int pos) const
{
#ifdef RECORD_DEBUG
  cerr << "Record::GetElementName(" << pos << ")";
#endif

  // check precondition
  assert(pos >=0);
  assert(pos < this->elemInfoArray.Size());

  // get element info by position
  ElemInfo  elemInfo;
  this->elemInfoArray.Get(pos, elemInfo);

#ifdef RECORD_DEBUG
  cerr << ": elemName=" << elemInfo.elemName << endl;
#endif

  return elemInfo.elemName;
}

/*
~GetElementTypeName~ returns the element type name at position pos.

With the help of the element info ~elemInfo~ it is possible to select the
type name of an element given at a certain record position.

*/
const string
Record::GetElementTypeName(int pos) const
{
#ifdef RECORD_DEBUG
  cerr << "Record::GetElementTypeName(" << pos << ")";
#endif

  // check precondition
  assert(pos >=0);
  assert(pos < this->elemInfoArray.Size());

  // get element info by position
  ElemInfo  elemInfo;
  this->elemInfoArray.Get(pos, elemInfo);

#ifdef RECORD_DEBUG
  cerr << ": typeName=" << elemInfo.typeName << endl;
#endif

  return elemInfo.typeName;
}

/*
~SetElement~ stores the given element at the given position
in this record.

The element name has to be filled in and it has to be a known type name.
Otherwise the action will be rejected.
If everything is correct the new element info has to be saved in
~elemInfoArray~. Therefore the size of the array needs potentially
to be resized to assure that the new ~elemInfo~ fits in the array. After that
the element info data is set and the element data is stored in the
~elemData~ FLOB. If the element contains FLOBS oneself these FLOBs will be
stored in the elemExData FLOB.

*/
bool
Record::SetElement(int pos, Attribute* elem,
                   const string& typeName, const string& elemName)
{
#ifdef RECORD_DEBUG
  cerr << "Record::SetElement(" << pos
       << ", " << (elem == NULL ? "NULL" : typeid(*elem).name())
       << ", " << typeName << ", " << elemName << ")" << endl;
#endif

  // check preconditons
  assert(pos >= 0);
  assert(elem != NULL);

  // check of empty element name
  if (elemName.size() == 0) {
#ifdef RECORD_DEBUG
    cerr << "Record::SetElement: empty element name" << endl;
#endif
    cmsg.typeError("Record::SetElement: empty element name is not allowed.");
    return false;
  }

  // get a secondo catalog instance
  SecondoCatalog* sc = SecondoSystem::GetCatalog();

  // check given type name
  if (sc->IsTypeName(typeName) == false) {
#ifdef RECORD_DEBUG
    cerr << "Record::SetElement: unknown type name" << endl;
#endif
    cmsg.typeError("Record::SetElement: unknown type name " + typeName);
    return false;
  }

 // the element name has to start with a capital letter
  if (isupper(elemName[0]) == 0) {
#ifdef RECORD_DEBUG
    cerr << "Record::SetElement: element name has to start with a "
         << "capital letter: " << elemName << endl;
#endif
    cmsg.inFunError("Record::SetElement: element name has to start with a "
                    "capital letter: " + elemName);
    return false;
  }

  // create an empty element info instance
  ElemInfo elemInfo;

  // check if the requested position fits into the element info array
  if (pos >= this->elemInfoArray.Size()) {

    // save size of array (before resize operation)
    int size = this->elemInfoArray.Size();

    // resize array
    this->elemInfoArray.resize(pos + 1);

    // padding the array with empty element info objects to fill the gap
    for (; size < (this->elemInfoArray.Size() - 1); size++) {
      this->elemInfoArray.Put(size, elemInfo);
    }
  }

  // assign element name and type
  strncpy(elemInfo.elemName, elemName.c_str(), MAX_STRINGSIZE);
  strncpy(elemInfo.typeName, typeName.c_str(), MAX_STRINGSIZE);

  // assign element type related ids to the element info object
  sc->GetTypeId(typeName, elemInfo.algebraId, elemInfo.typeId);

#ifdef RECORD_DEBUG
  cerr << "Record::SetElement: pos=" <<  pos
       << ", algebraId=" << elemInfo.algebraId
       << ", typeId=" << elemInfo.typeId << endl;
#endif

  // mark element as available and assign remaining offset values
  elemInfo.hasData = true;
  elemInfo.dataOffset = this->elemData.getSize();
  elemInfo.extDataOffset = this->elemExtData.getSize();

#ifdef RECORD_DEBUG
  cerr << "Record::SetElement: pos=" <<  pos
       << ", elemInfo.dataOffset=" << elemInfo.dataOffset
       << ", elemInfo.extDataOffset=" << elemInfo.extDataOffset
       << ", elemData.Size=" << elemData.Size()
       << ", elemData.Info=" << this->elemData.ToString()
       << ", elem->Sizeof()=" << elem->Sizeof()
       << endl;
#endif

  // store element
  this->elemData.write((char*) elem, elem->Sizeof(), elemInfo.dataOffset);

#ifdef RECORD_DEBUG
  cerr << "Record::SetElement: pos=" << pos
       << ", elemData.Size=" << elemData.Size()
       << ", elemData.Info=" << this->elemData.ToString() << endl;
#endif

  // store element flob(s)
  // first of all, assign starting point of external data as offset
  size_t offset = elemInfo.extDataOffset;

  // iterate througth all flobs of this element
  for (int i = 0; i < elem->NumOfFLOBs(); i++) {
    // get current flob
    Flob* flob = elem->GetFLOB(i);

    // get current flob size
    size_t size = flob->getSize();
    // resize elem external data flob to the required size
    this->elemExtData.resize(offset + size);
#ifdef RECORD_DEBUG
    cerr << "Record::SetElement: pos=" << pos
         << ", FLOB #" << i << ", offset=" << offset
         << ", FLOB size=" << size
         << ", elemExtData size=" << this->elemExtData.Size() << endl;
#endif

    // get data from current flob and store it
    char* buffer = new char[size];
    flob->read(buffer, size, 0);
    this->elemExtData.write(buffer, size, offset);
    delete[] buffer;
    // update offset
    offset += size;
  }

  // store element info into the array
  this->elemInfoArray.Put(pos, elemInfo);

  // update number of elements
  this->noElements = pos + 1;

  // update hash value
  this->hashValue = (this->hashValue * HASH_MULTI) + elem->HashValue();

  return true;
}

/*
~AppendElement~ appends the given element at the end of this record.

It is possible to append new elements at the end of the record. Therefore the
method ~SetElement~ is used. The position of the new element equates
the number of already existing elements.

*/
bool
Record::AppendElement(Attribute* elem,
                      const string& typeName, const string& elemName)
{
#ifdef RECORD_DEBUG
  cerr << "Record::AppendElement("
       << (elem == NULL ? "NULL" : typeid(*elem).name())
       << ", " << typeName << ", " << elemName << ")" << endl;
#endif

  // delegate work to the set element methode
  return (this->SetElement(this->noElements, elem, typeName, elemName));
}

/*
7.1 Area with only abstract(virtual) ~Attribute~ methods

These methods need to be implemented to assure that the new data type can
be used in Secondo algebras. This includes the following methods:

  * Sizeof

  * Compare

  * Adjacent

  * Clone

  * Hashvalue

  * CopyFrom

*/

/*
~Sizeof~ returns the memory size that is needen to store a Record.

*/

size_t
Record::Sizeof() const
{
#ifdef RECORD_DEBUG
  cerr << "Record::Sizeof(): sizeof=" << sizeof(Record) << endl;
#endif

  return sizeof(Record);
}

/*
~Compare~ compares to records and returns an appropriate int value.

        0 = objects are equal

        $>$0= argument is smaller than ~this~

        $<$0 = ~this~ is smaller than the argument

*/
int
Record::Compare(const Attribute* rhs) const
{
#ifdef RECORD_DEBUG
  cerr << "Record::Compare()";
#endif

  int cmp;

  // validate the address of the right handle side
  if (rhs == NULL) {
    cmp = 1;
  }
  // compare the instance adresses
  else if (this == rhs) {
    cmp = 0;
  } else {
    // try to cast the given attribute type into a record type
    const Record* record = dynamic_cast<const Record*>(rhs);

    if (record == NULL) {
      // cast failure, given attribute type isn't of type record
      cmp = 1;
    } else {
      // compare by element size
      cmp = this->GetNoElements() - record->GetNoElements();

      // compare the elements if both have the same count of elements
      // (loop until cmp != 0 or all elements are compared)
      for (int i = 0; i < this->noElements && cmp == 0; i++) {
        Attribute* attr1 = this->GetElement(i);
        Attribute* attr2 = record->GetElement(i);

        cmp = attr1->Compare(attr2);
        attr1->DeleteIfAllowed();
        attr2->DeleteIfAllowed();
      }
    }
  }

#ifdef RECORD_DEBUG
  cerr << ": cmp=" << cmp << endl;
#endif

  // return the comare result
  return cmp;
}

/*
~Adjacent~ returns true if the current record instance is adjecent to
annother instance of the same type otherwise returns false.

*/
bool
Record::Adjacent(const Attribute* attr) const
{
  // this method returns always false as it is senseless in this context
  return false;
}

/*
~Clone~ returns a depth copy of the record object.

*/
Attribute *
Record::Clone() const
{
#ifdef RECORD_DEBUG
  cerr << "Record::Clone()" << endl;
#endif

  Record* clone = new Record(0);

  // copy all values from this to clone
  clone->hashValue = this->hashValue;
  clone->noElements = this->noElements;
  clone->elemInfoArray.copyFrom(elemInfoArray);
  clone->elemData.copyFrom(elemData);
  clone->elemExtData.copyFrom(elemExtData);

  return clone;
}

/*
~HashValue~ returns the hash value of the record.

The ~HashValue~ of the record starts with HASH\_INIT = 17 and changes
with every added element in this way of doing that the old ~HashValue~
is multiplied with a hash constant HASH\_MULTI = 57 and added to the
elements HashValue.

*/
size_t
Record::HashValue() const
{
#ifdef RECORD_DEBUG
  cerr << "Record::NumOfFLOBs(): hashValue=" << this->hashValue << endl;
#endif

  return this->hashValue;
}

/*
~CopyFrom~ copies the value of a given attribute into this object.

*/
void
Record::CopyFrom(const Attribute* attr)
{
#ifdef RECORD_DEBUG
  cerr << "Record::CopyFrom("
       << (attr == NULL ? "NULL" : typeid(*attr).name()) << ")" << endl;
#endif

  const Record* record = dynamic_cast<const Record*>(attr);

  if (record != NULL) {
    // copy all values from given record to this
    this->hashValue = record->hashValue;
    this->noElements = record->noElements;
    elemInfoArray.copyFrom(record->elemInfoArray);
    elemData.copyFrom(record->elemData);
    elemExtData.copyFrom(record->elemExtData);
  }

  return;
}

/*

8.1 Area with only virtual ~Attribute~ methods

These methods need to be implemented for this data type because
a record owns FLOBs. This includes the following methods:

  * NumOfFLOBs

  * GetFLOB

  * Print

*/

/*
~NumOfFLOBs~ returns the number of FLOBs used in this record class.
As a Record type needs a DBArray ~elemInfoArray~, a FLOB ~elemData~ and
a FLOB ~elemExData~, 3 is returned.

*/
int
Record::NumOfFLOBs() const
{
#ifdef RECORD_DEBUG
  cerr << "Record::NumOfFLOBs(): numOfFLOBs=3" << endl;
#endif

  return 3;
}

/*
~GetFLOB~ returns a pointer to one of the three available FLOB objects.

        0 = ~elemInfoArray~

        1 = ~elemData~

        2 = ~elemExData~

*/
Flob*
Record::GetFLOB(const int i)
{
#ifdef RECORD_DEBUG
  cerr << "Record::GetFLOB(" << i << ")" << endl;
#endif

  // check preconditions
  assert(i >= 0 &&  i < 3);

  switch(i) {
    case 0:
      return &elemInfoArray;
    case 1:
      return &elemData;
    case 2:
    default:
      return &elemExtData;
  }
}

/*
~Print~ prints out useful information about the record for debugging purpose
as there are the ~hashValue~, ~noElements~, current elements.

*/
ostream&
Record::Print(ostream& os) const
{
  os << "Record["
     << "addr=" << (void*) this
     << ", hashValue=" << this->hashValue
     << ", noElements=" << this->noElements;

  for (int pos = 0; pos < this->noElements; pos++) {
    // get element info by position
    ElemInfo  elemInfo;
    this->elemInfoArray.Get(pos, elemInfo);
    // print record element
    os << ", elem" << (pos + 1)
       << "=(" << elemInfo.typeName
       << " " << elemInfo.elemName
       << ")";
  }

  os << "]";

  return os;
}

/*
9.1 Area with only static ~SECONDO~ methods

This area includes the standard ~SECONDO~ methods as there are:


  * In

  * Out

  * Create

  * Delete

  * Clone

  * Cast

  * KindCheck

  * SizeOfObj

  * Property

*/


/*
~In~ function to create a record instance through a nested list.

According to the assigned type and value list the record is created.
Before that the lists are checked regarding there correctness in
the following order:

  1 check whether the given type list is a record type list at all

  2 check if in case of a non empty value list the two lists
  have the same number of elements.

  3 If the value list is empty an empty record with only the given
  element types is created

In all other cases an appropriate error message is populated and
the creation aborts.

After preliminary research both lists will be run through parallel
in order to create the single elements. To simplify the parallel
run through a list iterator ~ListIterator~ class has been created.
More information about the iterator can be found in ~ListIterator.h~
document.

At the run through the following cases have to be differentiated:

First of all the current type list has to contain 2 elements. Example:

  * case 1: (string (int int))

    1 element name

    1 element type list, with two integer values

      1.1 algebraId

      1.2 typeId

or

  * case 2: (string ((int int) ...))

    2 element name

    2 element type as list, first list item contains
    two integer values (see above)

The first possibility reflects a simple Secondo type whereas the second
illustrates a complex type likewise a ~record~.

The element name has to start with a capital letter.

The list elements have to conform these guidelines otherwise an error is
detected.

Once the list item is correct the new element is created. Therefore the
belonging algebraId and typeId is read out from the
Secondo catalogue.

Elements that are marked with NULL are created as well but are ~undefined~.

After the successful element creation this element is appended to the
record by AppendElement method.

The whole procedure is repeated as long as element information is
available in the given ~typeInfo~ list.

*/
Word
Record::In(const ListExpr typeInfo, const ListExpr instance,
           const int errorPos, ListExpr& errorInfo, bool& correct)
{
#ifdef RECORD_DEBUG
  cerr << "Record::In(" << nl->ToString(typeInfo) << ", "
       << nl->ToString(instance) << ", ..., ..., " << correct << ")" << endl;
#endif

  Word w = SetWord(Address(0));
  correct = false;
  const string nullSymbol = "NULL";

  if (Record::IsRecordTypeList(typeInfo)) {
    // create an empty record instance
    Record* record = new Record(0);

    bool hasValueList;

    if(listutils::isSymbolUndefined(instance)){ // an undefined record:
      record->SetDefined(false);
      correct = true;
      return w;
    }
    // in case of a not empty value list:
    // case 1: value list has to be a list
    // case 2: type list and value list have to be of the same length
    if (nl->ListLength(instance) == 0) {
      hasValueList = false;
    } else {
      hasValueList = true;

      // case 1
      if (nl->IsAtom(instance)){
#ifdef RECORD_DEBUG
        cerr << "Record::In: value list is of kind atom but "
                "a list is expected!" << endl;
#endif
        cmsg.inFunError("Record::In: Value list is of kind atom but "
                        "a list is expected!  ");
        return w;
      }

      // case 2
      if (nl->ListLength(instance) != nl->ListLength(nl->Rest(typeInfo))) {
#ifdef RECORD_DEBUG
        cerr << "Record::In: different number of elements in "
                "type list and value list " << endl;
#endif
        cmsg.inFunError("Record::In: different number of elements in "
                        "type list and Value list ");
        return w;
      }
    }

    // create type and value list iteratoren
    ListIterator typeIter = nl->Rest(typeInfo);
    ListIterator valueIter = instance;

    // variables used inside the iteration loop
    string elemName;
    string elemTypeName;
    int elemAlgebraId;
    int elemTypeId;
    Word elemWord;
    Attribute* elem;

    // iterate synchrone through the type and value list elements
    while(typeIter.HasNext() && valueIter.HasNext()) {
      // assign the current type list element
      NList curType = typeIter.NextNList();

      // assign the current value list element
      ListExpr curValue;
      if (hasValueList) {
        curValue = valueIter.NextListExpr();
      } else {
        curValue = nl->OneElemList(nl->SymbolAtom(nullSymbol));
      }

#ifdef RECORD_DEBUG
      cerr << "Record::In: curType=" << curType.convertToString() << endl;
      cerr << "Record::In: curValue=" << nl->ToString(curValue) << endl;
#endif

      // the current type list has to contain 2 elements
      // case 1: (string (int int))
      //         1. element name
      //         2. element type list with two integer values
      //           2.1 algebraId
      //           2.2 typeId
      // case 2: (string ((int int) ...))
      //         1. element name
      //         2. element type as list, first list item contains
      //            two integer values
      if (   !(   curType.length() == 2
               && curType.second().length() == 2
               && curType.second().first().isInt()
               && curType.second().second().isInt())
          && !(  curType.length() == 2
               && curType.second().length() > 0
               && curType.second().first().length() == 2
               && curType.second().first().first().isInt()
               && curType.second().first().second().isInt()))
      {
#ifdef RECORD_DEBUG
        cerr << "Record::In: wrong subtype info" << endl;
#endif
        cmsg.inFunError("Record::In: wrong subtype info: " +
                        curType.convertToString());
        return w;
      }

      // extraxt the element name from current type list element
      elemName = curType.first().convertToString();

      // extraxt the element type ids from current type list element
      if (curType.second().first().isAtom()) {
        // case 1
        elemAlgebraId = curType.second().first().intval();
        elemTypeId = curType.second().second().intval();
      } else {
        // case 2
        elemAlgebraId = curType.second().first().first().intval();
        elemTypeId = curType.second().first().second().intval();
      }

      // retrieve the type name of the ids from secondo catalog
      SecondoCatalog* sc = SecondoSystem::GetCatalog();
      elemTypeName = sc->GetTypeName(elemAlgebraId, elemTypeId);


      // the element name has to start with a capital letter
      if (isupper(elemName[0]) == 0) {
        cmsg.inFunError("Record::In: element name has to start with a "
                        "capital letter: " + elemName);
        return w;
      }

      // check if curValue is a Atom of value NULL (TEXT or Symbol)
      // if true -> create a default object
     if (nullSymbol.compare(nl->ToString(curValue)) == 0) {
       elemWord = (am->CreateObj(elemAlgebraId, elemTypeId))
                  (curType.second().listExpr());

       // cast the read type instance to an attribute
       elem = static_cast<Attribute*>(elemWord.addr);

       // make elem as undefined
       elem->SetDefined(false);
     } else {
       // read element by registered IN function of the current type
       elemWord = (am->InObj(elemAlgebraId, elemTypeId))
                  (curType.second().listExpr(),
                   curValue,
                   errorPos,
                   errorInfo,
                   correct);

       // cast the read type instance to an attribute
       elem = static_cast<Attribute*>(elemWord.addr);
      }

      // check of existing object elem
      if (elem == NULL) {
        cmsg.inFunError("Record::In: In function of type "
                        + elemTypeName
                        +" for element "
                        + elemName
                        + " has delivered a NULL pointer for value "
                        + nl->ToString(curValue));
        return w;
      }

      // append the read attribute to the record and check the result
      if (record->AppendElement(elem, elemTypeName, elemName) == false) {
        cmsg.inFunError("Record::In: Cannot append element "
                        + elemName
                        + " of type "
                        + elemTypeName);
        elem->DeleteIfAllowed();
        return w;
      }
      elem->DeleteIfAllowed();
      elem=0;
    } // End of: iterate synchrone through the type and value list elements


    // set the created record as return value
    w = SetWord(record);
    correct = true;
  } //IsRecordTypeList

#ifdef RECORD_DEBUG
  cerr << "Record::In: correct=" << correct
       << ", w.addr=" << w.addr << endl;
#endif

  return w;
}


/*
~Out~ function to create a nested list representation for a record type
object.

First of all the given ~typeInfo~ list has to be verified whether
the list is correct. Therfore an private method ~IsRecordTypeList~ (see below)
is used. After that it is checked if the record
is defined at all. If not an adequate information is given back as a
nested list.

If the record does not contain any element that is to say the record
is empty, an empty list is returned.

Finally the private method ~GetElementValueList~ (see below)
is called for every
remaining elment. This method creates a proper ~ListExpr~ for the
element which is added to the related record nested list.

*/
ListExpr
Record::Out(ListExpr typeInfo, Word value)
{
#ifdef RECORD_DEBUG
  cerr << "Record::Out(" << nl->ToString(typeInfo) << ", ...)" << endl;
#endif

  ListExpr result;

  // check type info list expression first
  if (Record::IsRecordTypeList(typeInfo) == false) {
    // result = nl->GetErrorList();
    result = nl->OneElemList(nl->StringAtom("type info failure"));
  } if (value.addr == NULL) {
    // result = nl->GetErrorList();
    result = nl->OneElemList(nl->StringAtom("value is NULL failure"));
  } else {
    // cast the given address to a record instance
    Record* record = static_cast<Record*>(value.addr);

#ifdef RECORD_DEBUG
    cerr << "Record::Out: " << *record << endl;
#endif

    if(record->IsDefined() == false) {
      // record isn't defined
      // result = nl->GetErrorList();
      result = nl->SymbolAtom(Symbol::UNDEFINED());
    } else if (record->noElements == 0) {
      // record has no elements
      result = nl->TheEmptyList();
    } else {
      // create list with value of the first element
      result = nl->OneElemList(record->GetElementValueList(0));

      // iterate through the remaining elements
      ListExpr last = result;
      for(int i = 1; i < record->noElements; i++) {
        // append value of current element to the list
        last = nl->Append(last, record->GetElementValueList(i));
      }
    }
  }

#ifdef RECORD_DEBUG
  cerr << "Record::Out: result=" << nl->ToString(result) << endl;
#endif

  return result;
}

/*
~Create~ creates an empty, undefined ~record~ without subtype for
the Query Processor.

*/
Word
Record::Create(const ListExpr typeInfo)
{
#ifdef RECORD_DEBUG
  cerr << "Record::Create(" << nl->ToString(typeInfo) << ")" << endl;
#endif

  Word w = SetWord(Address(0));

  // create a record instance
  w = SetWord(Address(new Record(0)));

  return w;
}

/*
~Delete~

*/
void
Record::Delete(const ListExpr typeInfo, Word& w)
{
#ifdef RECORD_DEBUG
  cerr << "Record::Delete(" << nl->ToString(typeInfo)
       << ", " << w.addr << ")" << endl;
#endif

  if (w.addr != NULL) {
    Record* record = static_cast<Record*>(w.addr);
    delete record;
    w.addr = 0;
  }
}

/*
~Close~

*/
void
Record::Close(const ListExpr typeInfo, Word& w)
{
#ifdef RECORD_DEBUG
  cerr << "Record::Close(" << nl->ToString(typeInfo)
       << ", " << w.addr << ")" << endl;
#endif

  if (w.addr != NULL) {
    Record* record = static_cast<Record*>(w.addr);
    delete record;
    w.addr = 0;
  }
}

/*
~Clone~

*/
Word
Record::Clone(const ListExpr typeInfo, const Word& w)
{
#ifdef RECORD_DEBUG
  cerr << "Record::Clone(" << nl->ToString(typeInfo)
       << ", " << w.addr << ")" << endl;
#endif

  Word clone = SetWord(Address(0));

  if (w.addr != NULL) {
    Record* record = static_cast<Record*>(w.addr);
    clone = SetWord(record->Clone());
  }

  return clone;
}

/*
~Cast~

*/
void*
Record::Cast(void* addr)
{
#ifdef RECORD_DEBUG
  cerr << "Record::Cast(" << addr << ")" << endl;
#endif

  return new (addr) Record();
}

/*
Kind Checking Function

This function checks whether the type constructor is correct applied.

A Record ~typeInfo~ has to look like: (record (...)) thus a list of minimum
two elements where the first element has to be keyword ~record~.

All remaining elements in the ~typeInfo~ list can be simple elements with a
simple type or can be records again. Thus they have to look like

  * case 1: (string string)

    1 element name as string

    2 element type name as string

  * case 2: (string (list)), e.g. record in record

    1 element name as string

    2 element type as list

The type name has to be of kind ~DATA~.

*/
bool
Record::KindCheck(ListExpr typeInfo, ListExpr& errorInfo)
{

 if(nl->ListLength(typeInfo)<2){ // symbol record and at least one element
   return false;
 } 
 if(!listutils::isSymbol(nl->First(typeInfo), Record::BasicType())){
   return false;
 }
 ListExpr rest = nl->Rest(typeInfo);
 // rest must be a list of pairs (symbol DATA)
 set<string> usedNames;
 while(!nl->IsEmpty(rest)){
   ListExpr attr = nl->First(rest);
   rest = nl->Rest(rest);
   if(!nl->HasLength(attr,2)){
      cmsg.typeError("invalid field description in record");
      return false; 
   }
   ListExpr attrName = nl->First(attr);
   if(!listutils::isSymbol(attrName)){
      cmsg.typeError("invalid field name in record");
      return false; 
   }
   string name = nl->SymbolValue(attrName);
   if(usedNames.find(name)!=usedNames.end()){
      cmsg.typeError("field name " + name + " is used twice ");
      return false; 
   }
   usedNames.insert(name);
   string errmsg;
   if(!SecondoSystem::GetCatalog()->IsValidIdentifier(name,errmsg,true)){
      cmsg.typeError("field name " + name + " is not allowed: " + errmsg);
      return false; 
   }
   // ok, check the type of this field
   if(!listutils::isDATA(nl->Second(attr))){
      cmsg.typeError("field  " + name + " is not in kind DATA");
      return false; 
   }
 } 
 return true;

}

/*
~SizeOfObj~

*/
int
Record::SizeOfObj()
{
#ifdef RECORD_DEBUG
  cerr << "Record::SizeOfObj(): sizeOfObj=" << sizeof(Record) << endl;
#endif

  return sizeof(Record);
}

/*
~Property~

*/
ListExpr
Record::Property()
{
#ifdef RECORD_DEBUG
  cerr << "Record::Property()" << endl;
#endif

  return (nl->TwoElemList(
         nl->FiveElemList(nl->StringAtom("Signature"),
                          nl->StringAtom("Example Type List"),
                          nl->StringAtom("List Rep"),
                          nl->StringAtom("Example List"),
                          nl->StringAtom("Remarks")),

         nl->FiveElemList(nl->StringAtom("-> DATA"),
                          nl->StringAtom("(" + Record::BasicType() +
                                                          "(name string) "
                                         "(age int) (amount real))"),
                          nl->StringAtom("(stringElem intElem realElem)"),
                          nl->StringAtom("(\"Meyer\" 42 1023.08)"),
                          nl->StringAtom("All elements have to be of type"
                                         " attribute."))));
}


/*
10.1 Area with private record methods

This area includes all private record methods as there are:

  * CopyFlob

  * CopyElemInfos

  * GetElementValueList

  * IsRecordTypeList

*/

/*
~GetElementValueList~ returns a proper ~ListExpr~ for the element
at the given record position.
This is needed in ~Out~ (see above).

*/
ListExpr
Record::GetElementValueList(int pos) const
{
  // check preconditions
  assert(pos >=0);
  assert(pos < this->elemInfoArray.Size());

  // get element info by position
  ElemInfo elemInfo;
  this->elemInfoArray.Get(pos, elemInfo);

  // get element by position
  Attribute* elem = this->GetElement(pos);

  // create type info list for the element
  ListExpr subtypeInfo = nl->TwoElemList(nl->IntAtom(elemInfo.algebraId),
                                         nl->IntAtom(elemInfo.typeId));

  // return list expression for the requested element
  ListExpr res =  (am->OutObj(elemInfo.algebraId, elemInfo.typeId))
           (subtypeInfo, SetWord(elem));
  delete elem;
  return res;
}

/*
~IsRecordTypeList~ checks if the given type list is a record type list.
This is needed in several places like ~In~, ~Out~, ~Save~, ~Open~....

Three type lists a common nowadays:

  1 (72 0)

  2 ((72 0))

  3 ((72 0) ...)

With this information a crosscheck is done with the Secondo catalogue.

*/
bool
Record::IsRecordTypeList(ListExpr typeInfo)
{
  bool isRecord = false;
  NList list = typeInfo;

#ifdef RECORD_DEBUG
  cerr << "Record::IsRecordTypeList(" << nl->ToString(typeInfo) << ")" << endl;
#endif


  // check type info list expression, it has to be
  //    "(72 0)"
  // or "((72 0))"
  // or "((72 0) ...)"
  if (list.length() >= 1) {
    // extact the algebra and type id from the given list
    int listAlgebraId;
    int listTypeId;

    if (list.first().isAtom()) {
      // case: "(72 0)"
      listAlgebraId = list.first().intval();
      listTypeId = list.second().intval();
    } else {
      // case: "((72 0))" or "((72 0) ...)"
      listAlgebraId = list.first().first().intval();
      listTypeId = list.first().second().intval();
    }

    // retrieve record ids from secondo cataloge
    int scAlgebraId;
    int scTypeId;
    SecondoCatalog* sc =  SecondoSystem::GetCatalog();
    sc->GetTypeId(Record::BasicType(), scAlgebraId,  scTypeId);

    // compare the list ids with secondo record ids
    if (listAlgebraId == scAlgebraId && listTypeId == scTypeId) {
      isRecord = true;
    }
  }

  return isRecord;
}

