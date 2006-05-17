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

//paragraph [1] title: [{\Large \bf ]	[}]
//[ae] [\"{a}]
//[ue] [\"{u}]

[1] ArrayAlgebra

December 2003 Oliver L[ue]ck

This algebra provides a type constructor ~array~, which defines a ["]generic["]
array. The elements of the array must have an internal list representation. The
definition of arrays has been tested for arrays of relations (type constructor
~rel~, only in the main memory version!), b-trees (~btree~) and the standard
types ~int~, ~real~, ~bool~ and ~string~.

The four basic operators ~size~, ~get~, ~put~ and ~makearray~ are used to get
the size of an array, to retrieve an element with a given index, to set a value
to an element with a given index or to construct an array from a given list of
elements.

Note that the first element has the index 0. A precondition for the operators
~get~ and ~put~ is a valid index between 0 and the size of the array minus 1.

The operator ~sortarray~ arranges the elements of an array according to their
integer values of a given functon.

The operator ~tie~ concatenates all elements of an array with a given function,
e.g. it sums up all elements of an integer array. The operator ~cumulate~
calculates the ["]cumulative["] values for each position of the array (see
detailed operator specification).

The operator ~distribute~ creates an array of relations from a tuple stream (by
["]distributing["] the tuples into the relations of the array). The operator
~summarize~ provides a tuple stream containing all tuples stored in an array of
relations (similar to the operator ~feed~ of the relational algebra for a
single relation).

The operator ~loop~ evaluates each element of an array with a given function
and returns an array which contains the resulting values. Based upon this
elementary operator a ["]family["] of more complex ~loop~ operators is defined,
which are capable to work with two arrays or with more than one function. The
aim is to achieve optimization through switching between these functions or the
selection of a function (at best of the most efficient function). Therefore, as
a precondition, all functions have to be equivalent (with regard to the
calculated values).

The operator ~partjoin~ (and its extensions ~partjoinswitch~ and
~partjoinselect~) realizes a special way to calculate ["]joins["] between two
arrays of relations. Nevertheless, the result of this operator is also an
~array~ (see detailed operator specification).

Two additional type operators called ~ELEMENT~ and ~ELEMENT2~ support the
technique of implicit parameter types (for the declaration of parameter
functions). These operators are used by the parser and are not for use with
sos-syntax.

August 2004, M. Spiekermann. Function ~distributeFun~ has been modified. Due to the
fact that a relation creates two files the size of the array is limited by the operatings
systems capability of open files per process. Currently it is possible to create approximately
400 relations. 

August 24, 2004. M. Spiekermann removed a memory leak in the function ~distributeFun~

December 2005, Victor Almeida deleted the deprecated algebra levels
(~executable~, ~descriptive~, and ~hibrid~). Only the executable
level remains. Models are also removed from type constructors.

February 2006, M. Spiekermann. Bug fixes in type mapping of the ~distribute~ and in the
value mapping of the ~summarize~ operator.

1 Preliminaries

1.1 Includes

*/
using namespace std;

#include <sstream>

#include "Algebra.h"
#include "NestedList.h"
#include "NList.h"
#include "QueryProcessor.h"
#include "AlgebraManager.h"
#include "StandardTypes.h"
#include "RelationAlgebra.h"
#include "time.h"
#include "FunVector.h"


extern NestedList* nl;
extern QueryProcessor* qp;
extern AlgebraManager* am;

bool operator<( const FunInfo& f1, const FunInfo& f2 )
{
  return (f1.no < f2.no);
}

std::ostream&
operator<<( std::ostream& os, const FunInfo& f )
{
  os << "function " << f.name << " used " << f.timesUsed
     << " times, used CPU time: " << f.consumedTime << " seconds.";
  return os;
}


namespace {

/*
1.2 Dummy Functions

These functions are needed for the definition of a type constructor (function
~DummyCast~) or for the definition of a non-overloaded operator (function
~simpleSelect~).

*/
static void*
DummyCast( void* addr )
{
  return (0);
}

static int
simpleSelect( ListExpr args )
{
  return 0;
}

/*
1.3 Auxiliary Functions

The function ~toString~ just converts an integer value to a string.

The function ~extractIds~ ["]extracts["] the id-numbers of the algebra and the
type from a given type expression (nested list). This type expression must
already be in the numeric format.

*/
string toString( int number )
{
  ostringstream o;
  o << number << char(0);
  return o.str();
}

void extractIds( const ListExpr numType, int& algebraId, int& typeId )
{
  ListExpr pair;

  if (nl->IsAtom(nl->First(numType))) {
    pair = numType;
  }
  else
  {
    pair = nl->First(numType);
  }

  algebraId = nl->IntValue(nl->First(pair));
  typeId = nl->IntValue(nl->Second(pair));
}

/*
The following ["]generic["] clone function is used by several operators in
order to clone objects. Some types may provide just a dummy clone function. In
this case the list representation for input and output of objects (if defined)
may be used for cloning.

*/
static Word
genericClone( int algebraId, int typeId, ListExpr typeInfo, Word object )
{
  Word clone;

  // Try cloning with the clone function of the appropriate type

  clone = (am->CloneObj(algebraId, typeId))(typeInfo, object);

  if (clone.addr == 0) {

    // Try cloning via the object's list representation

    ListExpr objectLE;

    int errorPos;
    ListExpr errorInfo;
    bool correct;

    objectLE = (am->OutObj(algebraId, typeId))(typeInfo, object);

    clone = (am->InObj(algebraId, typeId))
                   (typeInfo, objectLE, errorPos, errorInfo, correct);

    assert (correct);
  }

  return clone;
}

/*
2 Type Constructor ~array~

2.1 Data Structure - Class ~Array~

At first a data structure for storing an ~array~ in the main memory is defined.
Since an object is represented as a storage Word (which is often a pointer to
the actual object), the data structure of an ~array~ is an array of Word.

*/
class Array
{
  public:
    Array(int, int, int, Word*);
    Array();
    ~Array();
    void initialize(int, int, int, Word*);
    bool isDefined();
    int getSize();
    int getElemAlgId();
    int getElemTypeId();
    Word getElement(int);
    void setElement(int, Word);
  private:
    bool defined;
    int size;
    int elemAlgId;
    int elemTypeId;
    Word* array;
};

Array::Array( int algebraId, int typeId, int n, Word* elements )

// Constructor with complete initialization of the array

{
  defined = true;
  elemAlgId = algebraId;
  elemTypeId = typeId;
  size = n;
  array = new Word[size];

  for (int i=0; i<size; i++) {
    array[i] = elements[i];
  }
}

Array::Array()
{
  defined = false;
  size = 0;
  elemAlgId = 0;
  elemTypeId = 0;
}

Array::~Array()
{
  delete []array;
}

void
Array::initialize( int algebraId, int typeId, int n, Word* elements )

// If the array is already defined, it will be cleared and redefined.

{
  if (defined) {
    delete []array;
  }

  defined = true;
  elemAlgId = algebraId;
  elemTypeId = typeId;
  size = n;
  array = new Word[size];

  for (int i=0; i<size; i++) {
   array[i] = elements[i];
  }
}

bool
Array::isDefined()
{
  return defined;
}

int
Array::getSize()
{
  return size;
}

int
Array::getElemAlgId()
{
  return elemAlgId;
}

int
Array::getElemTypeId()
{
  return elemTypeId;
}

Word
Array::getElement( int index )

// The array has to be defined and index >= 0 and index < size.

{
  if (defined && index>=0 && index<size) {
    return array[index];
  }
  else {
    return SetWord(Address(0));
  }
}

void
Array::setElement( int index, Word element )

// The array has to be defined and index >= 0 and index < size.

{
  if (defined && index>=0 && index<size) {
    array[index] = element;
  }
}

/*
2.2 List Representation

The list representation of an array is:

---- (a1 a2 ... an)
----
The representation of the elements of the array depends from their type. 
So a1 ... an may be nested lists themselves.

2.3 Object ~In~ and ~Out~ Functions

These functions use the ~In~ and ~Out~ functions of the elements of the array.

*/
static Word
InArray( const ListExpr typeInfo, const ListExpr instance,
         const int errorPos, ListExpr& errorInfo, bool& correct )
{
  Array* newarray;

  Word a[nl->ListLength(instance)];
  int algebraId;
  int typeId;

  if (nl->ListLength(instance) > 0) {

  // An array has to consist of at least one element.

    ListExpr typeOfElement = nl->Second(typeInfo);
    ListExpr listOfElements = instance;
    ListExpr element;

    extractIds(typeOfElement, algebraId, typeId);

    int i = 0;
    correct = true;

    do
    {
      element = nl->First(listOfElements);
      listOfElements = nl->Rest(listOfElements);

      a[i++] = ((am->InObj(algebraId, typeId))
                       (typeOfElement, element, errorPos, errorInfo, correct));
    }
    while (!nl->IsEmpty(listOfElements) && correct);

    if (correct) {
      newarray = new Array(algebraId, typeId, i, a);
      return SetWord(newarray);
    }
  }

  correct = false;
  return SetWord(Address(0));
}

static ListExpr
OutArray( ListExpr typeInfo, Word value )
{
  Array* array = (Array*)(value.addr);
  int algebraId = array->getElemAlgId();
  int typeId = array->getElemTypeId();

  ListExpr typeOfElement = nl->Second(typeInfo);

  ListExpr list;
  ListExpr last;
  ListExpr element;

  for (int i=0; i<array->getSize(); i++) {

    element = (am->OutObj(algebraId, typeId))
                             (typeOfElement, array->getElement(i));
    if (i==0) {
      list = nl->OneElemList(element);
      last = list;
    }
    else {
      last = nl->Append(last, element);
    }
  }

  return list;
}

/*
2.4 Object ~RestoreFromList~ and ~SaveToList~ Functions

These functions use the ~RestoreFromList~ and ~SaveToList~ functions of the elements of the array.

*/
static Word
RestoreFromListArray( const ListExpr typeInfo, const ListExpr instance,
                      const int errorPos, ListExpr& errorInfo, bool& correct )
{
  Array* newarray;

  Word a[nl->ListLength(instance)];
  int algebraId;
  int typeId;

  if (nl->ListLength(instance) > 0) {

  // An array has to consist of at least one element.

    ListExpr typeOfElement = nl->Second(typeInfo);
    ListExpr listOfElements = instance;
    ListExpr element;

    extractIds(typeOfElement, algebraId, typeId);

    int i = 0;
    correct = true;

    do
    {
      element = nl->First(listOfElements);
      listOfElements = nl->Rest(listOfElements);
      a[i++] = ((am->RestoreFromListObj(algebraId, typeId))
                       (typeOfElement, element, errorPos, errorInfo, correct));

    }
    while (!nl->IsEmpty(listOfElements) && correct);

    if (correct) {
      newarray = new Array(algebraId, typeId, i, a);
      return SetWord(newarray);
    }
  }

  correct = false;
  return SetWord(Address(0));
}

static ListExpr
SaveToListArray( ListExpr typeInfo, Word value )
{
  Array* array = (Array*)(value.addr);
  int algebraId = array->getElemAlgId();
  int typeId = array->getElemTypeId();

  ListExpr typeOfElement = nl->Second(typeInfo);

  ListExpr list;
  ListExpr last;
  ListExpr element;

  for (int i=0; i<array->getSize(); i++) {
    element = (am->SaveToListObj(algebraId, typeId))
                             (typeOfElement, array->getElement(i));

    if (i==0) {
      list = nl->OneElemList(element);
      last = list;
    }
    else {
      last = nl->Append(last, element);
    }
  }

  return list;
}

/*
2.5 Object ~Open~ and ~Save~ Functions

These functions are similar to the default ~Open~ and ~Save~ functions, but
they are based on the *internal* list representation.

The original aim of this change was to handle arrays of ~btrees~, which do not
have a list representation (for input and output), but which do have an
*internal* list representation (namely a ["]list["] containing a file-id).

*/
bool
OpenArray( SmiRecord& valueRecord, size_t& offset,  
           const ListExpr typeInfo, Word& value )
{
  ListExpr valueList = 0;
  string valueString;
  int valueLength;

  ListExpr errorInfo = nl->OneElemList( nl->SymbolAtom( "ERRORS" ) );
  bool correct;
  valueRecord.Read( &valueLength, sizeof(valueLength), offset );
  offset += sizeof(valueLength);
  char* buffer = new char[valueLength];
  valueRecord.Read( buffer, valueLength, offset );
  offset += valueLength;
  valueString.assign( buffer, valueLength );

  delete []buffer;
  nl->ReadFromString( valueString, valueList );

  value = RestoreFromListArray( typeInfo, 
                                nl->First(valueList), 
                                1, errorInfo, correct );

  if (errorInfo != 0)
  {
    nl->Destroy( errorInfo );
  }
  nl->Destroy( valueList );
  return (true);
}

bool
SaveArray( SmiRecord& valueRecord, size_t& offset,
           const ListExpr typeInfo, Word& value)
{
  ListExpr valueList;
  string valueString;
  int valueLength;

  valueList = SaveToListArray( typeInfo, value );
  valueList = nl->OneElemList( valueList );
  nl->WriteToString( valueString, valueList );
  valueLength = valueString.length();
  valueRecord.Write( &valueLength, sizeof(valueLength), offset );
  offset += sizeof(valueLength);
  valueRecord.Write( valueString.data(), valueString.length(), offset );
  offset += valueString.length();

  nl->Destroy( valueList );
  return (true);
}

/*
2.6 Object ~Creation~, ~Deletion~, ~Close~, ~Clone~ and ~SizeOf~ Functions

The ~Clone~ and the ~Close~ functions use the appropriate functions of the
elements of the array. Additional details are explained within these function.

*/
Word
CreateArray( const ListExpr typeInfo )
{
  return SetWord(new Array());
}

void
DeleteArray( const ListExpr typeInfo, Word& w )
{
  w.addr = 0;
}

void CloseArray( const ListExpr typeInfo, Word& w )
{
  SecondoCatalog* sc = SecondoSystem::GetCatalog();
  Array* array = (Array*)w.addr;

  if (array->isDefined()) {

    string typeName = sc->GetTypeName(array->getElemAlgId(),
                                      array->getElemTypeId());

    for (int i=0; i<array->getSize(); i++) {
      Word element = array->getElement(i);
      (am->CloseObj(array->getElemAlgId(),
                    array->getElemTypeId()))(nl->TheEmptyList(), element);
    }
  }

  delete array;

  w.addr = 0;
}

Word
CloneArray( const ListExpr typeInfo, const Word& w )
{
  Array* array = (Array*)w.addr;
  Array* newarray;

  bool ok = true;

  int n = array->getSize();
  int algebraId = array->getElemAlgId();
  int typeId = array->getElemTypeId();

  Word a[array->getSize()];

  for (int i=0; i < n; i++) {
    a[i] = (am->CloneObj(algebraId, typeId))( nl->TheEmptyList(), 
                                              array->getElement(i) );

    // Check whether cloning was successful

    ok = ok && (a[i].addr != 0);
  }

  if (ok) {
    newarray = new Array(algebraId, typeId, n, a);
  }
  else {

    // If the element's type just provides a dummy clone function, the clone
    // function of the array is also a dummy function.

    newarray = 0;
  }

  return SetWord(newarray);
}

int
SizeOfArray()
{
  return sizeof(Array);
}

/*
2.7 Function Describing the Signature of the Type Constructor

The type of the elements of the array may be described by any valid type
constructor, but the elements must have an internal list representation.

*/
static ListExpr
ArrayProperty()
{
  ListExpr remarkslist = nl->TextAtom();
  nl->AppendText(remarkslist, "The elements of the array must have an "
                              "internal list representation.");
  return (nl->TwoElemList(
            nl->FiveElemList(
              nl->StringAtom("Signature"),
              nl->StringAtom("Example Type List"),
              nl->StringAtom("List Rep"),
              nl->StringAtom("Example List"),
              nl->StringAtom("Remarks")),
            nl->FiveElemList(
              nl->StringAtom("typeconstructor -> ARRAY"),
              nl->StringAtom("(array int)"),
              nl->StringAtom("(a1 a2 ... an)"),
              nl->StringAtom("(0 1 2 3)"),
              remarkslist)));
}

/*
2.8 Kind Checking Function

The type constructor of an array is a list (array type). The first element of
that list is the symbol atom "array" and the second element has to be a valid
type constructor for the elements of the array.

So the second element can be an atom (e.g. int) or - in case of a more complex
type - a nested list itself.

In order to achieve great flexibility, the element's type is not restricted to
the tested types (see introduction). The user of an array has to make sure that
the elements have an internal list representation, because this is not checked
here.

*/
static bool
CheckArray( ListExpr type, ListExpr& errorInfo )
{
  if (nl->ListLength(type) == 2) {

    ListExpr First = nl->First(type);
    ListExpr Second = nl->Second(type);

    if (nl->IsEqual(First, "array")) {

      // Check whether Second is a valid type constructor

      SecondoCatalog* sc = SecondoSystem::GetCatalog();

      if (sc->KindCorrect(Second, errorInfo)) {
        return true;
      }
    }
  }

  return false;
}

/*
2.9 Creation of the Type Constructor Instance

Here an object of the class TypeConstructor is created. The constructor for an
instance of the class TypeConstructor is called with the properties and
functions for the array as parameters. The name of the type constructor is
~array~.

*/
TypeConstructor array (
      "array",
      ArrayProperty,
      OutArray, InArray,
      SaveToListArray, RestoreFromListArray,
      CreateArray, DeleteArray,
      OpenArray, SaveArray,
      CloseArray, CloneArray,
      DummyCast,
      SizeOfArray,
      CheckArray );

/*
3 Implementation of Utility Datastructures for handling parameter functions 

*/

FunInfo::FunInfo() {}

FunInfo::FunInfo( int No, string Name, Supplier s )
{
  no = No;
  name = Name;
  supplier = s;
  timesUsed = 0;
  consumedTime = 0;
}

double
FunInfo::getTime() {
  return consumedTime;
}

void
FunInfo::request( Word* arguments, int n, Word& funresult, string info = "" )
{
  ArgVectorPointer funargs;
  clock_t c1;
  clock_t c2;
  double timediff;

  funargs = qp->Argument( supplier );

  for (int i=0; i<n; i++) {
    (*funargs)[i] = arguments[i];
  }

  if (info != "") {
    cout << info << ", ";
  }

  cout << "function " << name << ", ";

  c1 = clock();
  qp->Request( supplier, funresult );
  c2 = clock();

  timediff = (double)(c2 - c1) / CLOCKS_PER_SEC;

  timesUsed++;
  consumedTime += timediff;

  cout << "used CPU time: " << timediff << " (" << consumedTime
       << ") seconds." << endl;
}


void
FunInfo::open()
{
  qp->Open(supplier);
} 

void
FunInfo::close()
{
  qp->Close(supplier);
} 

void
FunInfo::request( Word argument, Word& funresult, string info = "" )
{
  request(&argument, 1, funresult, info);
}

void
FunInfo::request( Word firstArg, Word secondArg, Word& funresult,
                  string info = "" )
{
  Word arguments[2] = { firstArg, secondArg };
  request(arguments, 2, funresult, info);
}



void
FunVector::addFunction( string name, Supplier s )
{
  //cout << "size: " << funInfos.size() << endl;
  FunInfo f( funInfos.size()+1, name, s );
  funInfos.push_back(f);
}

void
FunVector::load( Word suppl, Word* funNames, const bool doRequest /*= false*/ )
{
  Supplier funSupplier = (Supplier)suppl.addr;
  Supplier supplier1;
  Supplier supplier2;

  int noOfFuns = qp->GetNoSons(funSupplier);

  for (int i=0; i<noOfFuns; i++) {

    Word wfunName = funNames[i];
    if (doRequest)
    {
	qp->Request(funNames[i].addr, wfunName);
    }
    const STRING* name = ((CcString*)wfunName.addr)->GetStringval();

    //cerr << "Function " << i << "/" << noOfFuns << *name << endl;
    supplier1 = qp->GetSupplier(funSupplier, i);
    supplier2 = qp->GetSupplier(supplier1, 1);

    addFunction(*name, supplier2);
  }
}

void
FunVector::requestFun( int funNo, Word argument, Word& funresult,
                       string info = "" )
{
  funInfos[funNo].request(argument, funresult, info);
}

void
FunVector::requestFun( int funNo, Word firstArgument, Word secondArgument,
                       Word& funresult, string info = "" )
{
  funInfos[funNo].request(firstArgument, secondArgument, funresult, info);
}

void
FunVector::requestAll( Word argument, Word& funresult, string info = "" )
{
  for(int i=0; i<(int)funInfos.size(); i++) {
    requestFun(i, argument, funresult, info);
  }
}

void
FunVector::requestAll( Word firstArgument, Word secondArgument,
                       Word& funresult, string info = "" )
{
  for(int i=0; i<(int)funInfos.size(); i++) {
    requestFun(i, firstArgument, secondArgument, funresult, info);
  }
}

void
FunVector::sendMsgForAll(int msg) 
{
  for (size_t i=0; i < funInfos.size(); i++ )
  {
    switch (msg) {
      case OPEN: funInfos[i].open();
      case CLOSE: funInfos[i].close();
      default: assert(false);           
    } 
  }
} 


void
FunVector::reorder()

// Precondition:  All functions between the second and the last element of the
//                vector are ordered by their used CPU time.
// Postcondition: All functions of the vector are ordered by their used CPU
//                time.

{
  int n=funInfos.size();

  if (n > 1) {
    if (funInfos[0].getTime() > funInfos[1].getTime()) {

      // Find the position where to insert the first element, insert the first
      // element at this position and then remove this element from the first
      // position.

      int l = 0;
      int r = n;
      int m;

      do {
        m = (l+r) / 2;

        if (funInfos[m].getTime() <= funInfos[0].getTime()) {
          l = m;
        }
        else {
          r = m;
        }
      }
      while ( (m < (n-1))
              && !((funInfos[m].getTime() <= funInfos[0].getTime())
                   && (funInfos[0].getTime() < funInfos[m+1].getTime())) );

      funInfos.insert( funInfos.begin() + m + 1, funInfos[0] );
      funInfos.erase( funInfos.begin() );
    }
  }
}

int
FunVector::getMin()

// Returns the index of the function with the minimum used CPU time. If there
// are more such functions, the index of the first of these functions is
// returned.

{
  int min=0;

  for (int i=0; i < (int)funInfos.size(); i++) {
    if (funInfos[i].getTime() < funInfos[min].getTime()) {
      min = i;
    }
  }

  return min;
}

void
FunVector::writeSummary()
{
  sort(funInfos.begin(), funInfos.end());

  for (int i=0; i < (int)funInfos.size(); i++) {
    cout << "SUMMARY, " << funInfos[i] << "\n";
  }
}



/*
3.3 Class ~SwitchAlgorithm~

The switch algorithm is implemented as a sub-class of the class ~FunVector~.

An object of the class ~SwitchAlgorithm~ is initialized like an object of the
class ~FunInfo~. For each requests, the switch algorithm chooses the function
with the (so far) lowest total used CPU time.

*/
class SwitchAlgorithm : public FunVector {
  public:
    void request(Word, Word&, string);
    void request(Word, Word, Word&, string);
  private:
    int counter;
};

void
SwitchAlgorithm::request( Word argument, Word& funresult, string info = "" )
{
  requestFun(0, argument, funresult, info);
  reorder();
}

void
SwitchAlgorithm::request( Word firstArgument, Word secondArgument,
                          Word& funresult, string info = "" )
{
  requestFun(0, firstArgument, secondArgument, funresult, info);
  reorder();
}

/*
3.4 Class ~SelectAlgorithm~

The select algorithm is implemented as a sub-class of the class ~FunVector~.

An object of the class ~SelectAlgorithm~ is initialized analogous to an object
of the class ~FunInfo~. In addition to a set of functions, the parameter
~testSize~ has to be set to a number greater than zero. For the first
~testSize~ requests, all functions are used for evaluation. After that, the
function with the (so far) lowest total used CPU time is selected for all
further requests. However, this selection will not be changed later on.

*/
class SelectAlgorithm : public FunVector {
  public:
    SelectAlgorithm();
    void setTestSize(int);
    void request(Word, Word&, string);
    void request(Word, Word, Word&, string);
  private:
    int testSize;
    int selectedFun;
    int counter;
};

SelectAlgorithm::SelectAlgorithm()
{
  testSize = 0;
  selectedFun = -1;
  counter = 0;
}

void
SelectAlgorithm::setTestSize( int n )
{
  testSize = n;
}

void
SelectAlgorithm::request( Word argument, Word& funresult, string info = "" )
{
  if (counter++ < testSize) {
    requestAll(argument, funresult, info);
  }
  else {
    if (selectedFun == -1) {
      selectedFun = getMin();
    }

    requestFun(selectedFun, argument, funresult, info);
  }
}

void
SelectAlgorithm::request( Word firstArgument, Word secondArgument,
                          Word& funresult, string info = "" )
{
  if (counter++ < testSize) {
    requestAll(firstArgument, secondArgument, funresult, info);
  }
  else {
    if (selectedFun == -1) {
      selectedFun = getMin();
    }

    requestFun(selectedFun, firstArgument, secondArgument, funresult, info);
  }
}

/*
4 Operators

4.1 Operator ~size~

The operator ~size~ returns the number of elements of an array.

The type mapping is:

---- ((array t)) -> int
----

*/
static ListExpr
sizeTypeMap( ListExpr args )
{
  if (nl->ListLength(args) == 1)
  {
    ListExpr arg1 = nl->First(args);

    if (!nl->IsAtom(arg1) && nl->IsEqual(nl->First(arg1), "array")) {
      return nl->SymbolAtom("int");
    }
  }

  return nl->SymbolAtom("typeerror");
}

static int
sizeFun ( Word* args, Word& result, int message, Word& local, Supplier s )
{
  Array* array = ((Array*)args[0].addr);

  result = qp->ResultStorage(s);

  ((CcInt*)result.addr)->Set(true, array->getSize());

  return 0;
}

const string sizeSpec =
   "(( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
     "( <text>((array t)) -> int</text--->"
       "<text>size ( _ )</text--->"
       "<text>Returns the size of an array.</text--->"
       "<text>query size(ai)</text---> ))";

Operator size (
      "size",
      sizeSpec,
      sizeFun,
      simpleSelect,
      sizeTypeMap );

/*
4.2 Operator ~get~

The operator ~get~ returns an element at a given index. So the result type of
the operator is the type of the array's elements.

The type mapping is:

---- ((array t) int) -> t
----

Precondition of the value mapping function is a valid index. This means an
index between 0 and the size of the array minus 1. An element is cloned before
returning.

*/
static ListExpr
getTypeMap( ListExpr args )
{
  if (nl->ListLength(args) == 2)
  {
    ListExpr arg1 = nl->First(args);
    ListExpr arg2 = nl->Second(args);

    if (!nl->IsAtom(arg1) && nl->IsEqual(nl->First(arg1), "array")
          && nl->IsEqual(arg2, "int")) {

      // The second item of arg1 is the type of the array's elements.

      ListExpr resultType = nl->Second(arg1);
      return resultType;
    }
  }

  return nl->SymbolAtom("typeerror");
}

static int
getFun ( Word* args, Word& result, int message, Word& local, Supplier s )
{
  Array* array = ((Array*)args[0].addr);
  CcInt* index = ((CcInt*)args[1].addr);

  int i = index->GetIntval();

  if (i<0 || i >= array->getSize()) {

  // error handling

    cout << "*** Error in Operator get: " << endl;
    cout << "Index " << i << " out of range [0;"
         << array->getSize() - 1 << "], ";
    cout << "first element will be returned." << endl;
    i = 0;
  }

  if (i>=0 && i < array->getSize()) {

  // should always be true

    SecondoCatalog* sc = SecondoSystem::GetCatalog();

    Word element = array->getElement(i);

    Word clonedElement;

    int algebraId = array->getElemAlgId();
    int typeId = array->getElemTypeId();

    ListExpr resultType = qp->GetType(s);

    if (nl->ListLength(resultType) > 1) {
      if (nl->IsEqual(nl->First(resultType), "map")) {

        // In case of a mapping only the type of the resulting object of
        // the mapping is relevant.

        while (nl->ListLength(resultType) > 1) {
          resultType = nl->Rest(resultType);
        }
        resultType = nl->First(resultType);
      }
    }

    resultType = sc->NumericType(resultType);

    clonedElement = genericClone(algebraId, typeId, resultType, element);

    // In the next statement the (by the Query Processor) provided place for
    // the result is not used in order to be flexible with regard to the
    // result type.

    result.addr = clonedElement.addr;

    return 0;
  }
  else {
    return 1;
  }
}

const string getSpec =
   "(( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
     "( <text>((array t) int) -> t</text--->"
       "<text>get ( _, _ )</text--->"
       "<text>Returns an element with a given index.</text--->"
       "<text>query get(ai,3)</text---> ))";

Operator get (
      "get",
      getSpec,
      getFun,
      simpleSelect,
      getTypeMap );

/*
4.3 Operator ~put~

The operator ~put~ assigns a value to an element at a given index of an array.

The type mapping is:

---- ((array t) t int) -> (array t)
----

Precondition of the value mapping function is a valid index. This means an
index between 0 and the size of the array minus 1. The function returns a new
array.

*/
static ListExpr
putTypeMap( ListExpr args )
{
  if (nl->ListLength(args) == 3)
  {
    ListExpr arg1 = nl->First(args);
    ListExpr arg2 = nl->Second(args);
    ListExpr arg3 = nl->Third(args);

    if (!nl->IsAtom(arg1) && nl->IsEqual(nl->First(arg1), "array")
        && nl->Equal(nl->Second(arg1), arg2) && nl->IsEqual(arg3, "int")) {
      return arg1;
    }
  }

  return nl->SymbolAtom("typeerror");
}

static int
putFun ( Word* args, Word& result, int message, Word& local, Supplier s )
{
  Array* array = ((Array*)args[0].addr);
  Word newelement = args[1];
  int i = ((CcInt*)args[2].addr)->GetIntval();

  if (i<0 || i >= array->getSize()) {

  // error handling

    cout << "*** Error in Operator put: " << endl;
    cout << "Index " << i << " out of range [0;"
         << array->getSize() - 1 << "], ";
    cout << "first element will be replaced." << endl;
    i = 0;
  }

  if (i>=0 && i < array->getSize()) {

  // should always be true

    SecondoCatalog* sc = SecondoSystem::GetCatalog();

    int n = array->getSize();
    int algebraId = array->getElemAlgId();
    int typeId = array->getElemTypeId();

    Word a[array->getSize()];
    Word element;

    ListExpr resultType = qp->GetType(s);

    if (nl->ListLength(resultType) > 1) {
      if (nl->IsEqual(nl->First(resultType), "map")) {

        // In case of a mapping only the type of the resulting object of
        // the mapping is relevant.

        while (nl->ListLength(resultType) > 1) {
          resultType = nl->Rest(resultType);
        }
        resultType = nl->First(resultType);
      }
    }

    resultType = sc->NumericType(resultType);
    ListExpr typeOfElement = sc->NumericType(nl->Second(resultType));

    for (int l=0; l < n; l++) {
      element = (l!=i) ? array->getElement(l) : newelement;
      a[l] = genericClone(algebraId, typeId, typeOfElement, element);
    }

    result = qp->ResultStorage(s);

    ((Array*)result.addr)->initialize(algebraId, typeId, n, a);

    return 0;
  }
  else {
    return 1;
  }
}

const string putSpec =
   "(( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
     "( <text>((array t) t int) -> (array t)</text--->"
       "<text>put ( _, _, _ )</text--->"
       "<text>Replaces an element at a given index.</text--->"
       "<text>query put(ai,9,3)</text---> ))";

Operator put (
      "put",
      putSpec,
      putFun,
      simpleSelect,
      putTypeMap );

/*
4.4 Operator ~makearray~

This operator creates an array containing the elements of a given list. Note
that all elements must have the same type. The elements are cloned before
creating the array.

The type mapping is:

---- (t t ... t) -> (array t)
----

*/
static ListExpr
makearrayTypeMap( ListExpr args )
{
  bool sameType = true;

  if (nl->ListLength(args) > 0)
  {
    ListExpr typeOfElement = nl->First(args);
    args = nl->Rest(args);

    while (!nl->IsEmpty(args) && sameType) {
      sameType = nl->Equal(nl->First(args), typeOfElement);
      args = nl->Rest(args);
    }

    if (sameType) {
      return nl->TwoElemList(nl->SymbolAtom("array"), typeOfElement);
    }
  }

  return nl->SymbolAtom("typeerror");
}

static int
makearrayFun( Word* args, Word& result, int message, Word& local, Supplier s )
{
  SecondoCatalog* sc = SecondoSystem::GetCatalog();

  ListExpr type = qp->GetType(s);
  ListExpr typeOfElement = sc->NumericType(nl->Second(type));

  int algebraId;
  int typeId;

  extractIds(typeOfElement, algebraId, typeId);

  int n = qp->GetNoSons(s);

  Word a[n];

  for (int i=0; i<n; i++) {
    a[i] = genericClone(algebraId, typeId, typeOfElement, args[i]);
  }

  result = qp->ResultStorage(s);

  ((Array*)result.addr)->initialize(algebraId, typeId, n, a);

  return 0;
}

const string makearraySpec =
   "(( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
     "( <text>(t t ... t) -> (array t)</text--->"
       "<text>makearray ( list )</text--->"
       "<text>Creates an array containing the elements of a given list. "
       "The elements are cloned before creating the array.</text--->"
       "<text>let ai = makearray (0,1,2,3)</text---> ))";

Operator makearray(
      "makearray",
      makearraySpec,
      makearrayFun,
      simpleSelect,
      makearrayTypeMap );

/*
4.5 Operator ~sortarray~

The operator ~sortarray~ arranges the elements of an array according to their
integer values of a given function.

The formal specification of type mapping is:

---- ((array t) (map t int)) -> (array t)
----

First an auxiliary class ~IntPair~ is defined. The aim of this class is to use
the standard sort algorithm in the value mapping function.

*/
class IntPair {
  friend bool operator<(const IntPair&, const IntPair&);
  public : int first, second;
};

bool operator<(const IntPair& p1, const IntPair& p2) {
  return (p1.first < p2.first)
           || (p1.first == p2.first && p1.second < p2.second);
}

/*
Implementation of operator ~sortarray~.

*/
static ListExpr
sortarrayTypeMap( ListExpr args )
{
  if (nl->ListLength(args) == 2)
  {
    ListExpr arrayDesc = nl->First(args);
    ListExpr mapDesc = nl->Second(args);

    if ((nl->ListLength(arrayDesc) == 2)
        && (nl->ListLength(mapDesc) == 3))
    {
      if (nl->IsEqual(nl->First(arrayDesc), "array")
          && nl->IsEqual(nl->First(mapDesc), "map"))
      {
        if (nl->Equal(nl->Second(arrayDesc), nl->Second(mapDesc))
            && nl->IsEqual(nl->Third(mapDesc), "int"))
        {
          return arrayDesc;
        }
      }
    }
  }

  return nl->SymbolAtom("typeerror");
}

static int
sortarrayFun( Word* args, Word& result, int message, Word& local, Supplier s )
{
  SecondoCatalog* sc = SecondoSystem::GetCatalog();

  Array* array = ((Array*)args[0].addr);

  ArgVectorPointer funargs = qp->Argument(args[1].addr);
  Word funresult;

  ListExpr type = qp->GetType(s);
  ListExpr typeOfElement = sc->NumericType(nl->Second(type));

  int algebraId;
  int typeId;

  extractIds(typeOfElement, algebraId, typeId);

  int n = array->getSize();

  vector<IntPair> index(n);
  Word a[n];

  // Calculate and assing function values

  for (int i=0; i<n; i++) {
    (*funargs)[0] = array->getElement(i);

    qp->Request(args[1].addr, funresult);

    index[i].first = ((CcInt*)funresult.addr)->GetIntval();
    index[i].second = i;
  }

  // Sort index-vector according to function values

  sort(index.begin(), index.end());

  // Create resulting array

  for (int i=0; i<n; i++) {

    a[i] = genericClone(algebraId, typeId, typeOfElement,
                            array->getElement(index[i].second));
  }

  result = qp->ResultStorage(s);

  ((Array*)result.addr)->initialize(algebraId, typeId, n, a);

  return 0;
}

const string sortarraySpec =
   "(( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
    "( <text>((array t) (map t int)) -> (array t)</text--->"
      "<text>_ sortarray [ fun ]</text--->"
      "<text>Sorts an array in order of the function values of the "
      "elements.</text--->"
      "<text>query ai sortarray[fun(i:int)i]</text---> ))";

Operator sortarray(
      "sortarray",
      sortarraySpec,
      sortarrayFun,
      simpleSelect,
      sortarrayTypeMap );

/*
4.6 Operator ~tie~

The operator calculates a single "value" of an array by evaluating the elements
of an array with a given function from left to right, e.g.

tie ( (a1, a2, ... , an), + ) = a1 + a2 + ... + an

The formal specification of type mapping is:

---- ((array t) (map t t t)) -> t
----

*/
static ListExpr
tieTypeMap( ListExpr args )
{
  if (nl->ListLength(args) == 2)
  {
    ListExpr arrayDesc = nl->First(args);
    ListExpr mapDesc = nl->Second(args);

    if ((nl->ListLength(arrayDesc) == 2)
        && (nl->ListLength(mapDesc) == 4))
    {
      if (nl->IsEqual(nl->First(arrayDesc), "array")
          && nl->IsEqual(nl->First(mapDesc), "map"))
      {
        ListExpr elementDesc = nl->Second(arrayDesc);

        if (nl->Equal(elementDesc, nl->Second(mapDesc))
            && nl->Equal(elementDesc, nl->Third(mapDesc))
            && nl->Equal(elementDesc, nl->Fourth(mapDesc)))
        {
          return elementDesc;
        }
      }
    }
  }

  return nl->SymbolAtom("typeerror");
}

static int
tieFun( Word* args, Word& result, int message, Word& local, Supplier s )
{
  SecondoCatalog* sc = SecondoSystem::GetCatalog();
  Array* array = ((Array*)args[0].addr);

  ArgVectorPointer funargs = qp->Argument(args[1].addr);
  Word funresult;

  ListExpr typeOfElement = sc->NumericType(qp->GetType(s));

  int algebraId;
  int typeId;

  extractIds(typeOfElement, algebraId, typeId);

  int n = array->getSize();
  Word partResult;

  partResult = array->getElement(0);

  for (int i=1; i<n; i++) {
    (*funargs)[0] = partResult;
    (*funargs)[1] = array->getElement(i);



    qp->Request(args[1].addr, funresult);

    if (funresult.addr != partResult.addr) {
      if (i>1) {
        (am->DeleteObj(algebraId, typeId))(typeOfElement, partResult);
      }
      partResult = genericClone(algebraId, typeId, typeOfElement, funresult);
    }
  }

  // In the next statement the (by the Query Processor) provided place for
  // the result is not used in order to be flexible with regard to the
  // result type.

  result.addr = partResult.addr;

  return 0;
}

const string tieSpec =
   "(( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
    "( <text>((array t) (map t t t)) -> t</text--->"
      "<text>_ tie [ fun ]</text--->"
      "<text>Calculates the \"value\" of an array evaluating the elements of "
      "the array with a given function from left to right.</text--->"
      "<text>query ai tie[fun(i:int,l:int)(i+l)]</text---> ))";

Operator tie(
      "tie",
      tieSpec,
      tieFun,
      simpleSelect,
      tieTypeMap );

/*
4.7 Operator ~cumulate~

This operator ["]cumulates["] the values of the array under a given function.
The i-th element of the resulting array is the concatenation from the first to
the i-th element of the input array under a given function evaluated from left
to right (compare operator ~tie~), e.g.

cumulate ( (a1, a2, ... , an), + ) = (a1, a1 + a2, ... , a1 + a2 + ... + an)

The formal specification of type mapping is:

---- ((array t) (map t t t)) -> (array t)
----

*/
static ListExpr
cumulateTypeMap( ListExpr args )
{
  if (nl->ListLength(args) == 2)
  {
    ListExpr arrayDesc = nl->First(args);
    ListExpr mapDesc = nl->Second(args);

    if ((nl->ListLength(arrayDesc) == 2)
        && (nl->ListLength(mapDesc) == 4))
    {
    if (nl->IsEqual(nl->First(arrayDesc), "array")
        && nl->IsEqual(nl->First(mapDesc), "map"))
      {
        ListExpr elementDesc = nl->Second(arrayDesc);
        if (nl->Equal(elementDesc, nl->Second(mapDesc))
            && nl->Equal(elementDesc, nl->Third(mapDesc))
            && nl->Equal(elementDesc, nl->Fourth(mapDesc)))
        {
          return arrayDesc;
        }
      }
    }
  }

  return nl->SymbolAtom("typeerror");
}

static int
cumulateFun( Word* args, Word& result, int message, Word& local, Supplier s )
{
  SecondoCatalog* sc = SecondoSystem::GetCatalog();
  Array* array = ((Array*)args[0].addr);

  ArgVectorPointer funargs = qp->Argument(args[1].addr);
  Word funresult;

  ListExpr type = qp->GetType(s);


  ListExpr typeOfElement = sc->NumericType(nl->Second(type));

  int algebraId;
  int typeId;

  extractIds(typeOfElement, algebraId, typeId);

  int n = array->getSize();
  Word a[n];
  Word cumResult;

  cumResult = array->getElement(0);

  for (int i=0; i<n; i++) {
    if (i>=1) {
      (*funargs)[0] = cumResult;
      (*funargs)[1] = array->getElement(i);

      qp->Request(args[1].addr, funresult);

      if (funresult.addr != cumResult.addr) {
        (am->DeleteObj(algebraId, typeId))(typeOfElement, cumResult);
        cumResult = genericClone(algebraId, typeId, typeOfElement, funresult);
      }
    }

    a[i] = genericClone(algebraId, typeId, typeOfElement, cumResult);
  }

  result = qp->ResultStorage(s);

  ((Array*)result.addr)->initialize(algebraId, typeId, n, a);

  return 0;
}

const string cumulateSpec =
   "(( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
    "( <text>((array t) (map t t t)) -> (array t)</text--->"
      "<text>_ cumulate [ fun ]</text--->"
      "<text>Cumulates the values of an array under a given "
      "function.</text--->"
      "<text>query ai cumulate[fun(i:int,l:int)(i+l)]</text---> ))";

Operator cumulate (
      "cumulate",
      cumulateSpec,
      cumulateFun,
      simpleSelect,
      cumulateTypeMap );

/*
4.8 Operator ~distribute~

The operator ~distribute~ creates an array of relations from a stream of
tuples. The index of the appropriate relation has to be given by an integer
attribute of the tuple.

This integer attribute is removed from the tuples in the resulting relations.
So the incoming tuples have to consist of at least two attributes.

The formal specification of type mapping is:

---- ((stream (tuple ((x1 t1) ... (xn tn)))) xi)
     -> (array (rel (tuple ((x1 t1) ... (xi-1 ti-1) (xi+1 ti+1) ... (xn tn)))))

     at which n>=2, 1<=i<=n and ti (the type of xi) = int
----

The index of the attribute ai is appended to the result type by the type
mapping function, because this information is needed by the value mapping
function.

Within the value mapping function, an integer constant defines the maximum
number of relations in the resulting array. Tuples with an index smaller than 0
or an index greater than the maximum number of relations are distributed into
the first respectively the last relation.

*/
static ListExpr
distributeTypeMap( ListExpr inArgs )
{
  NList args(inArgs);
  if ( args.length() == 2 )
  {
    NList streamDesc = args.first();
    ListExpr attrNameLE = args.second().listExpr();

    if ( streamDesc.isList() && streamDesc.first().isSymbol("stream") 
         && (streamDesc.length() == 2)
         && (nl->AtomType(attrNameLE) == SymbolType)          )
    {
      ListExpr tupleDesc = streamDesc.second().listExpr();
      string attrName = nl->SymbolValue(attrNameLE);

      if (nl->IsEqual(nl->First(tupleDesc), "tuple")
          && (nl->ListLength(tupleDesc) == 2))
      {
        ListExpr attrList = nl->Second(tupleDesc);

        if (IsTupleDescription(attrList))
        {
          int attrIndex;
          ListExpr attrType;

          attrIndex = FindAttribute(attrList, attrName, attrType);

          if (nl->ListLength(attrList) > 1 && attrIndex > 0
              && nl->IsEqual(attrType, "int"))
          {
            ListExpr attrList2 = nl->TheEmptyList();
            ListExpr last;

            while (!nl->IsEmpty(attrList)) {
              ListExpr attr = nl->First(attrList);

              if (nl->SymbolValue(nl->First(attr)) != attrName) {
                if (nl->IsEmpty(attrList2)) {
                  attrList2 = nl->OneElemList(attr);
                  last = attrList2;
                }
                else {
                  last = nl->Append(last, attr);
                }
              }

              attrList = nl->Rest(attrList);
            }

            return nl->ThreeElemList(
                         nl->SymbolAtom("APPEND"),
                         nl->OneElemList(nl->IntAtom(attrIndex)),
                         nl->TwoElemList(
                           nl->SymbolAtom("array"),
                           nl->TwoElemList(
                             nl->SymbolAtom("rel"),
                             nl->TwoElemList(nl->SymbolAtom("tuple"),
                                             attrList2))));
          }
        }
      }
    }
  }

  return args.typeError("input is not (stream(tuple(y))) x ...");
}


/*

17.08.04 M. Spiekermann

The operating system has an limitation of simultaneously
opend files per process (Linux 1024). Currently each created relation will
open two SMI-Files.

Creating an array containing 512 relations will break these limits. 
However, this limitation can only be removed with a redesign of the relation
class and/or SmiFile. It should be changed to allow multiple relations stored 
in one file. 

*/

static int
distributeFun (Word* args, Word& result, int message, Word& local, Supplier s)
{
  const int MAX_OPEN_RELATIONS = 400; // not the absolute possible maximum

  SecondoCatalog* sc = SecondoSystem::GetCatalog();

  CcInt* indexAttrCcInt = (CcInt*)args[2].addr;
  int pkgAttr = (indexAttrCcInt->GetIntval()) - 1;

  vector<Relation*> relPkg;
	
  ListExpr relType = nl->Second(qp->GetType(s));
  relType = sc->NumericType(relType);

  ListExpr tupleType = nl->Second(relType);

  int n = 0;
  relPkg.push_back( new Relation(relType) );

  CcInt* pkgNrCcInt = 0;
  int pkgNr = 0;
	int outOfRangePkgNr = 0;
	
  Word actual = SetWord( Address(0) );

  qp->Open(args[0].addr);
  qp->Request(args[0].addr, actual);

  bool msgPrinted = false;
	
  while(qp->Received(args[0].addr))
  {
    Tuple* tuple = (Tuple*)actual.addr;
    Tuple* tuple2 = new Tuple(tupleType);
 
    // Copy all attributes except the package number from tuple to tuple2.
    int j = 0;
    for (int i=0; i<tuple->GetNoAttributes(); i++) {
      if (i!=pkgAttr) 
        tuple2->CopyAttribute(i, tuple, j++);
    }

    // Determine package and distribute tuple2 into that package.
    pkgNrCcInt = (CcInt*)(tuple->GetAttribute(pkgAttr));
    pkgNr = pkgNrCcInt->GetIntval();

    tuple->DeleteIfAllowed();
	 
    if ( pkgNr > (MAX_OPEN_RELATIONS - 1) ) { // check if pckNr is valid

      if ( !msgPrinted ) 
      {
         cerr << "Warning: Package number out of Range. "
              << "Open files per process are limited! "
              << "Since every open relation needs to open files "
              << "it is only possible to create at most an array "
              << "with " << MAX_OPEN_RELATIONS << " relations." << endl;
               msgPrinted = true;	
      }	 
           
      pkgNr = outOfRangePkgNr % MAX_OPEN_RELATIONS;
      outOfRangePkgNr++;
    }

    while ( n < pkgNr ) { // enlarge the array if necessary
      
      relPkg.push_back( new Relation(relType) );
			n++;	
    } 
		
    relPkg[pkgNr]->AppendTuple(tuple2);
		
    tuple2->DeleteIfAllowed(); // free memory
	
    qp->Request(args[0].addr, actual);
  }
  qp->Close(args[0].addr);

  result = qp->ResultStorage(s);

  Word a[++n];

  for (int i=0; i<n; i++) {
	
    a[i] = SetWord(relPkg[i]);
  }

  int algebraId = 0;
  int typeId = 0;

  if (sc->GetTypeId("rel", algebraId, typeId)) {
	
    ((Array*)result.addr)->initialize(algebraId, typeId, n, a);
    return 0;
		
  } else {
	
    return 1;
  }
}

const string distributeSpec =
   "(( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
    "( <text>((stream (tuple ((x1 t1) ... (xn tn)))) xi) -> "
      "(array (rel (tuple ((x1 t1) ... (xi-1 ti-1) (xi+1 ti+1) ... "
      "(xn tn)))))</text--->"
      "<text>_ distribute [ _ ]</text--->"
      "<text>Distributes a stream of tuples into an array or relations. The "
      "attribute xi determines the index of the relation, therefore ti must "
      "be int.</text--->"
      "<text>let prel = plz feed distribute [pkg]</text---> ))";

Operator distribute (
      "distribute",
      distributeSpec,
      distributeFun,
      simpleSelect,
      distributeTypeMap );

/*
4.9 Operator ~summarize~

The operator ~summarize~ provides a stream of tuples from an array of
relations. For this purpose, the operator scans all relations beginning with
the first relation of the array.

The formal specification of type mapping is:

---- ((array (rel t))) -> (stream t)

     at which t is of the type tuple
----

Note that the operator ~summarize~ is not exactly inverse to the operator
~distribute~ because the index of the relation is not appended to the
attributes of the outgoing tuples. If the array has been constructed by the
operator ~distribute~ the order of the resulting stream in most cases does not
correspond to the order of the input stream of the operator ~distribute~.

*/
static ListExpr
summarizeTypeMap( ListExpr args )
{
  if (nl->ListLength(args) == 1)
  {
    ListExpr arrayDesc = nl->First(args);

    if (nl->ListLength(arrayDesc) == 2
        && nl->IsEqual(nl->First(arrayDesc), "array"))
    {
      ListExpr relDesc = nl->Second(arrayDesc);

      if (nl->ListLength(relDesc) == 2
          && nl->IsEqual(nl->First(relDesc), "rel"))
      {
        ListExpr tupleDesc = nl->Second(relDesc);
        if (nl->IsEqual(nl->First(tupleDesc), "tuple"))
        {
          return nl->TwoElemList(nl->SymbolAtom("stream"),
                                 nl->Second(relDesc));
        }
      }
    }
  }

  ErrorReporter::ReportError(
     "summarize: Input type array( rel( tuple(...))) expected!");
  return nl->SymbolAtom("typeerror");
}

static int
summarizeFun( Word* args, Word& result, int message, Word& local, Supplier s )
{
  struct ArrayIterator
  {
    private:
    int current; 
    Array* array;
    GenericRelationIterator* rit;
   
    // create an Tuple iterater for the next array element
    bool makeNextRelIter()
    {
      if (rit)
        delete rit;
      
      if ( current < array->getSize() ) 
      {
        Relation* r = static_cast<Relation*>( array->getElement(current).addr );
        current++;
        rit = r->MakeScan();
        return true;
      }
      rit=0;
      return false;  
    } 

    public:
    ArrayIterator(Array* a, const int pos=0) : current(pos), array(a), rit(0)
    {
      makeNextRelIter();
    }
    
    ~ArrayIterator()
    {
      if (rit)
        delete rit;
    }  
    
    Tuple* getNextTuple() // try to get next tuple
    {
      if (!rit)
        return 0;
      
      Tuple* t = rit->GetNextTuple();
      if ( !t )
      { 
         if (!makeNextRelIter())
           return 0;
         else
           return rit->GetNextTuple();
      }   
      return t; 
    } 
  };
 
  ArrayIterator* ait = 0;
  ait = (ArrayIterator*)local.addr;

  switch (message) {
    case OPEN : {
      Word argArray;
      qp->Request(args[0].addr, argArray);
      ait = new ArrayIterator( (Array*)argArray.addr );
      local.addr = ait;
      return 0;
    }
    case REQUEST : {

      Tuple* t = ait->getNextTuple();
      if (t != 0) {
        result = SetWord(t);
        return YIELD;
      }
      return CANCEL;
    }  
    case CLOSE : {
      ait = (ArrayIterator*)local.addr;
      delete ait;
      return 0;
    }  
    default : {
      return 0;              
    }                
  }
  return 0;
}

const string summarizeSpec =
   "(( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
    "( <text>((array (rel t))) -> (stream t)</text--->"
      "<text>_ summarize</text--->"
      "<text>Produces a stream of the tuples from all relations in the "
      "array.</text--->"
      "<text>query prel summarize consume</text---> ))";

Operator summarize (
      "summarize",
      summarizeSpec,
      summarizeFun,
      simpleSelect,
      summarizeTypeMap );

/*
4.10 Operator ~loop~

The Operator ~loop~ evaluates each element of an array with a given function and returns an array which contains the resulting values.

The formal specification of type mapping is:

---- ((array t) (map t r)) -> (array r)
----

*/
static ListExpr
loopTypeMap( ListExpr args )
{
  if (nl->ListLength(args) == 2)
  {
    ListExpr arrayDesc = nl->First(args);
    ListExpr mapDesc = nl->Second(args);

    if ((nl->ListLength(arrayDesc) == 2)
        && (nl->ListLength(mapDesc) == 3))
    {
      if (nl->IsEqual(nl->First(arrayDesc), "array")
          && nl->IsEqual(nl->First(mapDesc), "map")
          && !nl->IsEqual(nl->Third(mapDesc), "typeerror"))
      {
        if (nl->Equal(nl->Second(arrayDesc), nl->Second(mapDesc)))
        {
          return nl->TwoElemList(nl->SymbolAtom("array"),
                                 nl->Third(mapDesc));
        }
      }
    }
  }
  return nl->SymbolAtom("typeerror");
}

static int
loopFun( Word* args, Word& result, int message, Word& local, Supplier s )
{
  SecondoCatalog* sc = SecondoSystem::GetCatalog();

  Array* array = ((Array*)args[0].addr);

  FunInfo f(0, "0", args[1].addr);

  Word funresult;

  ListExpr type = qp->GetType(s);
  ListExpr typeOfElement = sc->NumericType(nl->Second(type));

  int algebraId;
  int typeId;

  extractIds(typeOfElement, algebraId, typeId);

  int n = array->getSize();

  Word a[n];
  string info;

  for (int i=0; i<n; i++) {
    info = "Processing element " + toString(i);
    f.request(array->getElement(i), funresult, info);
    a[i] = genericClone(algebraId, typeId, typeOfElement, funresult);
  }

  result = qp->ResultStorage(s);

  ((Array*)result.addr)->initialize(algebraId, typeId, n, a);

  return 0;
}

const string loopSpec =
   "(( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
    "( <text>((array t) (map t r)) -> (array r)</text--->"
      "<text>_ loop [ fun ]</text--->"
      "<text>Evaluates each element of an array with a given function and "
      "returns an array which contains the resulting values.</text--->"
      "<text>query ai loop [fun(i:int)(i*i)]</text---> ))";

Operator loop (
      "loop",
      loopSpec,
      loopFun,
      simpleSelect,
      loopTypeMap );

/*
4.11 Operator ~loopa~

The operator ~loopa~ evaluates each pair of elements *with the same index* from two arrays with a given function and returns an array which contains the resulting values.

If the two arrays do not have the same size, the remaining elements of the greater array are ignored.

The formal specification of type mapping is:

---- ((array t) (array u) (map t u r)) -> (array r)
----

*/
static ListExpr
loopaTypeMap( ListExpr args )
{
  if (nl->ListLength(args) == 3)
  {
    ListExpr firstArrayDesc = nl->First(args);
    ListExpr secondArrayDesc = nl->Second(args);
    ListExpr mapDesc = nl->Third(args);

    if ((nl->ListLength(firstArrayDesc) == 2)
        && (nl->ListLength(secondArrayDesc) == 2)
        && (nl->ListLength(mapDesc) == 4))
    {
      if (nl->IsEqual(nl->First(firstArrayDesc), "array")
          && nl->IsEqual(nl->First(secondArrayDesc), "array")
          && nl->IsEqual(nl->First(mapDesc), "map")
          && !nl->IsEqual(nl->Fourth(mapDesc), "typeerror"))
      {
        if (nl->Equal(nl->Second(firstArrayDesc), nl->Second(mapDesc))
            && nl->Equal(nl->Second(secondArrayDesc), nl->Third(mapDesc)))
        {
          return nl->TwoElemList(nl->SymbolAtom("array"),
                                 nl->Fourth(mapDesc));
        }
      }
    }
  }

  return nl->SymbolAtom("typeerror");
}

static int
loopaFun( Word* args, Word& result, int message, Word& local, Supplier s )
{
  SecondoCatalog* sc = SecondoSystem::GetCatalog();

  Array* firstArray = ((Array*)args[0].addr);
  Array* secondArray = ((Array*)args[1].addr);

  FunInfo f(0, "0", args[2].addr);

  Word funresult;

  ListExpr type = qp->GetType(s);
  ListExpr typeOfElement = sc->NumericType(nl->Second(type));

  int algebraId;
  int typeId;

  extractIds(typeOfElement, algebraId, typeId);

  int n = min(firstArray->getSize(), secondArray->getSize());

  Word a[n];
  string info;

  for (int i=0; i<n; i++) {
    info = "Processing elements " + toString(i) + " and " + toString(i);
    f.request(firstArray->getElement(i), secondArray->getElement(i),
              funresult, info);
    a[i] = genericClone(algebraId, typeId, typeOfElement, funresult);
  }

  result = qp->ResultStorage(s);

  ((Array*)result.addr)->initialize(algebraId, typeId, n, a);

  return 0;
}

const string loopaSpec =
   "(( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
    "( <text>((array t) (array u) (map t u r)) -> (array r)</text--->"
      "<text>_ _ loopa [ fun ]</text--->"
      "<text>Evaluates each pair of elements with the same index from "
      "two arrays with a given function and returns an array which contains "
      "the resulting values.</text--->"
      "<text>query ai al loopa[fun(i:int,l:int)(i+l)]</text---> ))";

Operator loopa (
      "loopa",
      loopaSpec,
      loopaFun,
      simpleSelect,
      loopaTypeMap );

/*
4.12 Operator ~loopb~


The operator ~loopb~ evaluates each pair of elements from two arrays with a given function and returns an array which contains the resulting values.

The formal specification of type mapping is:

---- ((array t) (array u) (map t u r)) -> (array r)
----

Therefore the type mapping function of the operator ~loopa~ can also be used for the operator ~loopb~.

*/
static int
loopbFun( Word* args, Word& result, int message, Word& local, Supplier s )
{
  SecondoCatalog* sc = SecondoSystem::GetCatalog();

  Array* firstArray = ((Array*)args[0].addr);
  Array* secondArray = ((Array*)args[1].addr);

  FunInfo f(0, "0", args[2].addr);

  Word funresult;

  ListExpr type = qp->GetType(s);
  ListExpr typeOfElement = sc->NumericType(nl->Second(type));

  int algebraId;
  int typeId;

  extractIds(typeOfElement, algebraId, typeId);

  int n = firstArray->getSize();
  int m = secondArray->getSize();

  Word a[n * m];
  string info;

  for (int i=0; i<n; i++) {
    for (int l=0; l<m; l++) {
      info = "Processing elements " + toString(i) + " and " + toString(l);
      f.request(firstArray->getElement(i), secondArray->getElement(l),
                funresult, info);
      a[i * m + l] = genericClone(algebraId, typeId, typeOfElement, funresult);
    }
  }

  result = qp->ResultStorage(s);

  ((Array*)result.addr)->initialize(algebraId, typeId, n * m, a);

  return 0;
}

const string loopbSpec =
   "(( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
    "( <text>((array t) (array u) (map t u r)) -> (array r)</text--->"
      "<text>_ _ loopb [ fun ]</text--->"
      "<text>Evaluates each pair of elements from two arrays with a given "
      "function and returns an array which contains the resulting "
      "values.</text--->"
      "<text>query ai al loopb[fun(i:int,l:int)(i+l)]</text---> ))";

Operator loopb (
      "loopb",
      loopbSpec,
      loopbFun,
      simpleSelect,
      loopaTypeMap );

/*
4.13 Operator ~loopswitch~

The operator ~loopswitch~ evaluates each element of an array with one of the given functions using the switch algorithm. So, under certain conditions, all functions may get (approximately) the same time for calculation and the faster functions will process more elements of the array.

The formal specification of type mapping is:

---- ((array t) ((name1 (map t r)) ... (namen (map t r)))) -> (array r)
----

*/
static ListExpr
loopswitchTypeMap( ListExpr args )
{
  if (nl->ListLength(args) == 2)
  {
    ListExpr arrayDesc = nl->First(args);
    ListExpr funlist = nl->Second(args);

    if ((nl->ListLength(arrayDesc) == 2)
        && (nl->IsEqual(nl->First(arrayDesc), "array")))
    {
      ListExpr firstFunDesc = nl->First(funlist);
      funlist = nl->Rest(funlist);

      ListExpr funNames = nl->TheEmptyList();
      ListExpr last;

      ListExpr firstMapDesc;

      if (nl->ListLength(firstFunDesc) == 2)
      {
        ListExpr firstFunName = nl->First(firstFunDesc);
        firstMapDesc = nl->Second(firstFunDesc);

        if ((nl->AtomType(firstFunName) == SymbolType)
            && (nl->ListLength(firstMapDesc) == 3))
        {
           if ((nl->IsEqual(nl->First(firstMapDesc), "map"))
               && (!nl->IsEqual(nl->Third(firstMapDesc), "typeerror"))
               && (nl->Equal(nl->Second(firstMapDesc),nl->Second(arrayDesc))))
          {
            funNames = nl->OneElemList(
                             nl->StringAtom(nl->SymbolValue(firstFunName)));
            last = funNames;
          }
        }
      }

      if (!nl->IsEmpty(funNames))
      {
        bool ok = true;

        while (!nl->IsEmpty(funlist) && ok)
        {
          ListExpr funDesc = nl->First(funlist);

          if (nl->ListLength(funDesc) == 2)
          {
            ListExpr funName = nl->First(funDesc);
            ListExpr mapDesc = nl->Second(funDesc);

            if ((nl->AtomType(funName) == SymbolType)
                && (nl->Equal(mapDesc, firstMapDesc)))
            {
              last = nl->Append(last,
                                nl->StringAtom(nl->SymbolValue(funName)));
            }
            else { ok = false; }
          }
          else { ok = false; }

          funlist = nl->Rest(funlist);
        }

        if (ok) {
          return nl->ThreeElemList(
                       nl->SymbolAtom("APPEND"),
                       funNames,
                       nl->TwoElemList(
                         nl->SymbolAtom("array"),
                         nl->Third(firstMapDesc)));
        }
      }
    }
  }

  return nl->SymbolAtom("typeerror");
}

static int
loopswitchFun( Word* args, Word& result, int message, Word& local, Supplier s )
{
  SecondoCatalog* sc = SecondoSystem::GetCatalog();

  Array* array = (Array*)args[0].addr;

  SwitchAlgorithm sw;

  sw.load(args[1], &args[2]);

  ListExpr type = qp->GetType(s);
  ListExpr typeOfElement = sc->NumericType(nl->Second(type));

  int algebraId;
  int typeId;

  extractIds(typeOfElement, algebraId, typeId);

  int n = array->getSize();

  Word a[n];
  Word funresult;
  string info;

  for (int i=0; i<n; i++) {
    info = "Processing element " + toString(i);

    sw.request(array->getElement(i), funresult, info);
    a[i] = genericClone(algebraId, typeId, typeOfElement, funresult);
  }

  sw.writeSummary();

  result = qp->ResultStorage(s);

  ((Array*)result.addr)->initialize(algebraId, typeId, n, a);

  return 0;
}

const string loopswitchSpec =
   "(( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
    "( <text>((array t) ((name1 (map t r)) ... (namen (map t r)))) "
      "-> (array r)</text--->"
      "<text>_ loopswitch [ funlist ]</text--->"
      "<text>Evaluates each element of an array with one of the given "
      "functions. All functions may get (approximately) the same time for "
      "calculation, so that the \"faster\" function will process more "
      "elements of the array.</text--->"
      "<text>query ai loopswitch[f:fun(i:int)(i*2), "
      "g:fun(l:int)(l+l)]</text---> ))";

Operator loopswitch (
      "loopswitch",
      loopswitchSpec,
      loopswitchFun,
      simpleSelect,
      loopswitchTypeMap );

/*
4.14 Operator ~loopswitcha~

This operator works like operator ~loopa~ extended by the switch algorithm of operator ~loopswitch~.

The formal specification of type mapping is:

---- ((array t) (array u) ((name1 (map t u r)) ... (namen (map t u r))))
     -> (array r)
----

*/
static ListExpr
loopswitchaTypeMap( ListExpr args )
{
  if (nl->ListLength(args) == 3)
  {
    ListExpr firstArrayDesc = nl->First(args);
    ListExpr secondArrayDesc = nl->Second(args);
    ListExpr funlist = nl->Third(args);

    if ((nl->ListLength(firstArrayDesc) == 2)
        && (nl->IsEqual(nl->First(firstArrayDesc), "array"))
        && (nl->ListLength(secondArrayDesc) == 2)
        && (nl->IsEqual(nl->First(secondArrayDesc), "array")))
    {
      ListExpr firstFunDesc = nl->First(funlist);
      funlist = nl->Rest(funlist);

      ListExpr funNames = nl->TheEmptyList();
      ListExpr last;

      ListExpr firstMapDesc;

      if (nl->ListLength(firstFunDesc) == 2)
      {
        ListExpr firstFunName = nl->First(firstFunDesc);
        firstMapDesc = nl->Second(firstFunDesc);

        if ((nl->AtomType(firstFunName) == SymbolType)
            && (nl->ListLength(firstMapDesc) == 4))
        {
           if ((nl->IsEqual(nl->First(firstMapDesc), "map"))
               && (!nl->IsEqual(nl->Fourth(firstMapDesc), "typeerror"))
               && (nl->Equal(nl->Second(firstMapDesc),
                             nl->Second(firstArrayDesc)))
               && (nl->Equal(nl->Third(firstMapDesc),
                             nl->Second(secondArrayDesc))))
          {
            funNames = nl->OneElemList(
                             nl->StringAtom(nl->SymbolValue(firstFunName)));
            last = funNames;
          }
        }
      }

      if (!nl->IsEmpty(funNames))
      {
        bool ok = true;

        while (!nl->IsEmpty(funlist) && ok)
        {
          ListExpr funDesc = nl->First(funlist);

          if (nl->ListLength(funDesc) == 2)
          {
            ListExpr funName = nl->First(funDesc);
            ListExpr mapDesc = nl->Second(funDesc);

            if ((nl->AtomType(funName) == SymbolType)
                && (nl->Equal(mapDesc, firstMapDesc)))
            {
              last = nl->Append(last,
                                nl->StringAtom(nl->SymbolValue(funName)));
            }
            else { ok = false; }
          }
          else { ok = false; }

          funlist = nl->Rest(funlist);
        }

        if (ok) {
          return nl->ThreeElemList(
                       nl->SymbolAtom("APPEND"),
                       funNames,
                       nl->TwoElemList(
                         nl->SymbolAtom("array"),
                         nl->Fourth(firstMapDesc)));
        }
      }
    }
  }

  return nl->SymbolAtom("typeerror");
}

static int
loopswitchaFun( Word* args, Word& result, int message, Word& local,
                Supplier s )
{
  SecondoCatalog* sc = SecondoSystem::GetCatalog();

  Array* firstArray = (Array*)args[0].addr;
  Array* secondArray = (Array*)args[1].addr;

  SwitchAlgorithm sw;

  sw.load(args[2], &args[3]);

  ListExpr type = qp->GetType(s);
  ListExpr typeOfElement = sc->NumericType(nl->Second(type));

  int algebraId;
  int typeId;

  extractIds(typeOfElement, algebraId, typeId);

  int n = min(firstArray->getSize(), secondArray->getSize());

  Word a[n];
  Word funresult;
  string info;

  for (int i=0; i<n; i++) {
    info = "Processing elements " + toString(i) + " and " + toString(i);

    sw.request(firstArray->getElement(i), secondArray->getElement(i),
               funresult, info);
    a[i] = genericClone(algebraId, typeId, typeOfElement, funresult);
  }

  sw.writeSummary();

  result = qp->ResultStorage(s);

  ((Array*)result.addr)->initialize(algebraId, typeId, n, a);

  return 0;
}

const string loopswitchaSpec =
   "(( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
    "( <text>((array t) (array u) ((name1 (map t u r)) ... "
      "(namen (map t u r)))) -> (array r)</text--->"
      "<text>_ loopswitcha [ funlist ]</text--->"
      "<text>Works like operator loopa extended by the switch algorithm of "
      "operator loopswitch.</text--->"
      "<text>query ai al loopswitcha[f:fun(i1:int,l1:int)(i1 mod l1), "
      "g:fun(i2:int,l2:int)(i2-(i2 div l2)*l2)]</text---> ))";

Operator loopswitcha (
      "loopswitcha",
      loopswitchaSpec,
      loopswitchaFun,
      simpleSelect,
      loopswitchaTypeMap );

/*
4.15 Operator ~loopswitchb~

This operator works like operator ~loopb~ extended by the switch algorithm of operator ~loopswitch~.

The formal specification of type mapping is:

---- ((array t) (array u) ((name1 (map t u r)) ... (namen (map t u r))))
     -> (array r)
----

Therefore the type mapping function of the operator ~loopswitcha~ can also be used for the operator ~loopswitchb~.

*/
static int
loopswitchbFun( Word* args, Word& result, int message, Word& local,
                Supplier s )
{
  SecondoCatalog* sc = SecondoSystem::GetCatalog();

  Array* firstArray = (Array*)args[0].addr;
  Array* secondArray = (Array*)args[1].addr;

  SwitchAlgorithm sw;

  sw.load(args[2], &args[3]);

  ListExpr type = qp->GetType(s);
  ListExpr typeOfElement = sc->NumericType(nl->Second(type));

  int algebraId;
  int typeId;

  extractIds(typeOfElement, algebraId, typeId);

  int n = firstArray->getSize();
  int m = secondArray->getSize();

  Word a[n * m];
  Word funresult;
  string info;

  for (int i=0; i<n; i++) {
    for (int l=0; l<m; l++) {
      info = "Processing elements " + toString(i) + " and " + toString(l);

      sw.request(firstArray->getElement(i), secondArray->getElement(l),
                 funresult, info);
      a[i * m + l] = genericClone(algebraId, typeId, typeOfElement, funresult);

    }
  }

  sw.writeSummary();

  result = qp->ResultStorage(s);

  ((Array*)result.addr)->initialize(algebraId, typeId, n * m, a);

  return 0;
}

const string loopswitchbSpec =
   "(( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
    "( <text>((array t) (array u) ((name1 (map t u r)) ... "
      "(namen (map t u r)))) -> (array r)</text--->"
      "<text>_ loopswitchb [ funlist ]</text--->"
      "<text>Works like operator loopb extended by the switch algorithm of "
      "operator loopswitch.</text--->"
      "<text>query ai al loopswitchb[f:fun(i1:int,l1:int)(i1 mod l1), "
      "g:fun(i2:int,l2:int)(i2-(i2 div l2)*l2)]</text---> ))";

Operator loopswitchb (
      "loopswitchb",
      loopswitchbSpec,
      loopswitchbFun,
      simpleSelect,
      loopswitchaTypeMap );

/*
4.16 Operator ~loopselect~

The operator ~loopselect~ evaluates the first n elements of the array with each of the given functions and sums up the used calculation times for each function. The remaining elements are processed with the (so far) fastest function.

The formal specification of type mapping is:

---- ((array t) ((name1 (map t r)) ... (namen (map t r))) int real)
     -> (array r)
----

The above mentioned parameter n is given by the int- and the real-parameter. Let x the int- and y the real-parameter, than n is calculated similar to the operator ~sample~ of the Relation Algebra:

n := min(arraySize, max(x, y * arraySize))

*/
static ListExpr
loopselectTypeMap( ListExpr args )
{
  if (nl->ListLength(args) == 4)
  {
    ListExpr arrayDesc = nl->First(args);
    ListExpr funlist = nl->Second(args);

    if (nl->IsEqual(nl->Third(args), "int")
        && nl->IsEqual(nl->Fourth(args), "real"))
    {
      return loopswitchTypeMap(nl->TwoElemList(arrayDesc, funlist));
    }
  }

  return nl->SymbolAtom("typeerror");
}

static int
loopselectFun( Word* args, Word& result, int message, Word& local, Supplier s )
{
  SecondoCatalog* sc = SecondoSystem::GetCatalog();

  Array* array = ((Array*)args[0].addr);

  CcInt* absTestCcInt = ((CcInt*)args[2].addr);
  int absTest = absTestCcInt->GetIntval();

  CcReal* relTestCcReal = ((CcReal*)args[3].addr);
  double relTest = relTestCcReal->GetRealval();

  SelectAlgorithm se;

  se.load(args[1], &args[4]);

  ListExpr type = qp->GetType(s);
  ListExpr typeOfElement = sc->NumericType(nl->Second(type));

  int algebraId;
  int typeId;

  extractIds(typeOfElement, algebraId, typeId);

  int n = array->getSize();

  if (absTest < 1) { absTest = 1; }
  if (absTest > n) { absTest = n; }

  if (relTest < 0) { relTest = 0; }
  if (relTest > 1) { relTest = 1; }

  se.setTestSize( max(absTest, (int)(n * relTest + 0.5)) );

  Word a[n];
  Word funresult;
  string info;

  for (int i=0; i<n; i++) {
    info = "Processing element " + toString(i);

    se.request(array->getElement(i), funresult, info);
    a[i] = genericClone(algebraId, typeId, typeOfElement, funresult);
  }

  se.writeSummary();

  result = qp->ResultStorage(s);

  ((Array*)result.addr)->initialize(algebraId, typeId, n, a);

  return 0;
}

const string loopselectSpec =
   "(( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
    "( <text>((array t) ((name1 (map t r)) ... (namen (map t r))) int real) "
      "-> (array r)</text--->"
      "<text>_ loopselect [ funlist; _, _ ]</text--->"
      "<text>Evaluates the first \"n\" elements of the array with each of "
      "the given functions and cumulates the used calculation times. The "
      "remaining elements are processed with the (so far) \"fastest\" "
      "function.</text--->"
      "<text>query ai loopselect[f:fun(i:int)(i*2), g:fun(l:int)(l+l); "
      "10, 0.1]</text---> ))";

Operator loopselect (
      "loopselect",
      loopselectSpec,
      loopselectFun,
      simpleSelect,
      loopselectTypeMap );

/*
4.17 Operator ~loopselecta~

This operator works like operator ~loopa~ extended by the select algorithm of operator ~loopselect~.

The formal specification of type mapping is:

---- ((array t) (array u) ((name1 (map t u r)) ... (namen (map t u r))) int
     real) -> (array r)
----

*/
static ListExpr
loopselectaTypeMap( ListExpr args )
{
  if (nl->ListLength(args) == 5)
  {
    ListExpr firstArrayDesc = nl->First(args);
    ListExpr secondArrayDesc = nl->Second(args);
    ListExpr funlist = nl->Third(args);

    if (nl->IsEqual(nl->Fourth(args), "int")
        && nl->IsEqual(nl->Fifth(args), "real"))
    {
      return loopswitchaTypeMap(nl->ThreeElemList(firstArrayDesc,
                                secondArrayDesc, funlist));
    }
  }

  return nl->SymbolAtom("typeerror");
}

static int
loopselectaFun( Word* args, Word& result, int message, Word& local,
                Supplier s )
{
  SecondoCatalog* sc = SecondoSystem::GetCatalog();

  Array* firstArray = (Array*)args[0].addr;
  Array* secondArray = (Array*)args[1].addr;

  CcInt* absTestCcInt = (CcInt*)args[3].addr;
  int absTest = absTestCcInt->GetIntval();

  CcReal* relTestCcReal = (CcReal*)args[4].addr;
  double relTest = relTestCcReal->GetRealval();

  SelectAlgorithm se;

  se.load(args[2], &args[5]);

  ListExpr type = qp->GetType(s);
  ListExpr typeOfElement = sc->NumericType(nl->Second(type));

  int algebraId;
  int typeId;

  extractIds(typeOfElement, algebraId, typeId);

  int n = min(firstArray->getSize(), secondArray->getSize());

  if (absTest < 1) { absTest = 1; }
  if (absTest > n) { absTest = n; }

  if (relTest < 0) { relTest = 0; }
  if (relTest > 1) { relTest = 1; }

  se.setTestSize( max(absTest, (int)(n * relTest + 0.5)) );

  Word a[n];
  Word funresult;
  string info;

  for (int i=0; i<n; i++) {
    info = "Processing elements " + toString(i) + " and " + toString(i);

    se.request(firstArray->getElement(i), secondArray->getElement(i),
               funresult, info);
    a[i] = genericClone(algebraId, typeId, typeOfElement, funresult);
  }

  se.writeSummary();

  result = qp->ResultStorage(s);

  ((Array*)result.addr)->initialize(algebraId, typeId, n, a);

  return 0;
}

const string loopselectaSpec =
    "(( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
     "( <text>((array t) (array u) ((name1 (map t u r)) ... "
       "(namen (map t u r))) int real) -> (array r)</text--->"
       "<text>_ _ loopselecta [ funlist; _, _ ]</text--->"
       "<text>Works like operator loopa extended by the select algorithm of "
       "operator loopselect.</text--->"
       "<text>query ai al loopselecta[f:fun(i1:int,l1:int)(i1 mod l1), "
       "g:fun(i2:int,l2:int)(i2-(i2 div l2)*l2); 10, 0.1]</text---> ))";

Operator loopselecta (
      "loopselecta",
      loopselectaSpec,
      loopselectaFun,
      simpleSelect,
      loopselectaTypeMap );

/*
4.18 Operator ~loopselectb~

This operator works like operator ~loopb~ extended by the select algorithm of operator ~loopselect~.

The formal specification of type mapping is:

---- ((array t) (array u) ((name1 (map t u r)) ... (namen (map t u r))) int
     real) -> (array r)
----

Therefore the type mapping function of operator ~loopselecta~ can also be used for operator ~loopselectb~.

*/
static int
loopselectbFun( Word* args, Word& result, int message, Word& local,
                Supplier s )
{
  SecondoCatalog* sc = SecondoSystem::GetCatalog();

  Array* firstArray = (Array*)args[0].addr;
  Array* secondArray = (Array*)args[1].addr;

  CcInt* absTestCcInt = (CcInt*)args[3].addr;
  int absTest = absTestCcInt->GetIntval();

  CcReal* relTestCcReal = (CcReal*)args[4].addr;
  double relTest = relTestCcReal->GetRealval();

  SelectAlgorithm se;

  se.load(args[2], &args[5]);

  ListExpr type = qp->GetType(s);
  ListExpr typeOfElement = sc->NumericType(nl->Second(type));

  int algebraId;
  int typeId;

  extractIds(typeOfElement, algebraId, typeId);

  int n = firstArray->getSize();
  int m = secondArray->getSize();
  int r = m * n;

  if (absTest < 1) { absTest = 1; }
  if (absTest > r) { absTest = r; }

  if (relTest < 0) { relTest = 0; }
  if (relTest > 1) { relTest = 1; }

  se.setTestSize( max(absTest, (int)(r * relTest + 0.5)) );

  Word a[r];
  Word funresult;
  string info;

  for (int i=0; i<n; i++) {
    for (int l=0; l<m; l++) {
      info = "Processing elements " + toString(i) + " and " + toString(l);

      se.request(firstArray->getElement(i), secondArray->getElement(l),
                 funresult, info);
      a[i * m + l] = genericClone(algebraId, typeId, typeOfElement, funresult);
    }
  }

  se.writeSummary();

  result = qp->ResultStorage(s);

  ((Array*)result.addr)->initialize(algebraId, typeId, r, a);

  return 0;
}

const string loopselectbSpec =
   "(( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
    "( <text>((array t) (array u) ((name1 (map t u r)) ... "
      "(namen (map t u r))) int real) -> (array r)</text--->"
      "<text>_ _ loopselecta [ funlist; _, _ ]</text--->"
      "<text>Works like operator loopb extended by the select algorithm of "
      "operator loopselect.</text--->"
      "<text>query ai al loopselectb[f:fun(i1:int,l1:int)(i1 mod l1), "
      "g:fun(i2:int,l2:int)(i2-(i2 div l2)*l2); 10, 0.1]</text---> ))";

Operator loopselectb (
      "loopselectb",
      loopselectbSpec,
      loopselectbFun,
      simpleSelect,
      loopselectaTypeMap );

/*
4.19 Operator ~partjoin~

The operator ~partjoin~ allows to calculate joins between two partitioned relations (two arrays of relations) in a special way.


The formal specification of type mapping is:

---- ((array (rel t)) (array (rel u)) (map (rel t) (rel u) r)) -> (array r)

     at which t and u are of the type tuple, r may be any type
----

The following functions appends (the tuples of) a relation to an other relation and is used by the partjoin algorithm.

*/
static void
appendToRel( Word& relation, Word append )

// This auxiliary routine is used by the partjoin algorithm to append a
// relation to an other relation.

{
  GenericRelation* rel = (GenericRelation*)(relation.addr);
  GenericRelation* part = (GenericRelation*)(append.addr);
  GenericRelationIterator* rit = part->MakeScan();
  Tuple* tuple;

  while ((tuple = rit->GetNextTuple()) != 0) 
  {
    if (rel == 0) 
    {
      rel = new Relation(tuple->GetTupleType());
      rel->Clear();
      relation = SetWord(rel);
    }
    rel->AppendTuple(tuple);
    tuple->DeleteIfAllowed();
  }
}

/*
Implementation of the operator.

*/
static ListExpr
partjoinTypeMap( ListExpr args )
{
  if (nl->ListLength(args) == 3)
  {
    ListExpr firstArrayDesc = nl->First(args);
    ListExpr secondArrayDesc = nl->Second(args);
    ListExpr mapDesc = nl->Third(args);

    if ((nl->ListLength(firstArrayDesc) == 2)
        && (nl->ListLength(secondArrayDesc) == 2)
        && (nl->ListLength(mapDesc) == 4))
    {
      if (nl->IsEqual(nl->First(firstArrayDesc), "array")
          && nl->IsEqual(nl->First(secondArrayDesc), "array")
          && !nl->IsAtom(nl->Second(firstArrayDesc))
          && !nl->IsAtom(nl->Second(secondArrayDesc))
          && nl->IsEqual(nl->First(mapDesc), "map"))
      {
        ListExpr firstElementDesc = nl->Second(firstArrayDesc);
        ListExpr secondElementDesc = nl->Second(secondArrayDesc);

        if (nl->IsEqual(nl->First(firstElementDesc), "rel")
            && nl->IsEqual(nl->First(secondElementDesc), "rel")
            && nl->Equal(firstElementDesc, nl->Second(mapDesc))
            && nl->Equal(secondElementDesc, nl->Third(mapDesc)))
        {
          ListExpr firstTupleDesc = nl->Second(firstElementDesc);
          ListExpr secondTupleDesc = nl->Second(secondElementDesc);

          if ((nl->ListLength(firstTupleDesc) == 2)
              && (nl->ListLength(secondTupleDesc) == 2)
              && (nl->IsEqual(nl->First(firstTupleDesc), "tuple"))
              && (nl->IsEqual(nl->First(secondTupleDesc), "tuple")))
          {
            return nl->TwoElemList(nl->SymbolAtom("array"),
                                   nl->Fourth(mapDesc));
          }
        }
      }
    }
  }

  return nl->SymbolAtom("typeerror");
}

static int
partjoinFun( Word* args, Word& result, int message, Word& local, Supplier s )
{

  // INITIALIZATION

  SecondoCatalog* sc = SecondoSystem::GetCatalog();

  Array* firstArray = ((Array*)args[0].addr);
  Array* secondArray = ((Array*)args[1].addr);

  FunInfo f(0, "0", args[2].addr);

  Word funresult;
  string info;

  ListExpr type = qp->GetType(s);
  ListExpr typeOfElement = sc->NumericType(nl->Second(type));

  int algebraId;
  int typeId;

  extractIds(typeOfElement, algebraId, typeId);

  int n = firstArray->getSize();
  int m = secondArray->getSize();

  Word a[n + m - 1];
  int c = 0;

  int i = 0;
  int j = 0;

  Word Acum = SetWord(0);
  Word Bcum = SetWord(0);

  // FIRST PART OF THE PARTJOIN ALGORITHM

  // Process the first elements individually to get an "entry point" for the
  // second part (see below).

  if ((i-j) <= (n-m)) {
    while ((i-j) <= (n-m)) {
      info =  "Processing partitions " + toString(i) + " and " + toString(j);

      f.request(firstArray->getElement(i), secondArray->getElement(j),
                funresult, info);

      a[c++] = genericClone(algebraId, typeId, typeOfElement, funresult);

      if (i > 0) {
        appendToRel(Acum, firstArray->getElement(i-1));
      }
      i++;
    }
    j++;
    appendToRel(Bcum, secondArray->getElement(0));
  }
  else {
    while ((i-j) >= (n-m)) {
      info = "Processing partitions " + toString(i) + " and " + toString(j);

      f.request(firstArray->getElement(i), secondArray->getElement(j),
                funresult, info);
      a[c++] = genericClone(algebraId, typeId, typeOfElement, funresult);

      appendToRel(Bcum, secondArray->getElement(j));
      j++;
    }
    i++;
  }

  // SECOND PART OF THE PARTJOIN ALGORITHM

  // The next loop implements the main concept of the partjoin algorithm:
  // the step by step summarization and evaluation of larger parts of the
  // participating arrays of relations.

  while (i < n) {
    appendToRel(Acum, firstArray->getElement(i-1));
    appendToRel(Bcum, secondArray->getElement(j));

    info = "Processing partitions [0;" + toString(i-1) + "] and "
           + toString(j);

    f.request(Acum, secondArray->getElement(j), funresult, info);
    a[c++] = genericClone(algebraId, typeId, typeOfElement, funresult);

    info = "Processing partitions " + toString(i) + " and [0;"
           + toString(j) + "]";

    f.request(firstArray->getElement(i), Bcum, funresult, info);
    a[c++] = genericClone(algebraId, typeId, typeOfElement, funresult);

    i++;
    j++;
  }

  // SET RESULT AND CLEAN UP

  result = qp->ResultStorage(s);

  ((Array*)result.addr)->initialize(algebraId, typeId, c, a);

  ((GenericRelation*)Acum.addr)->Clear();
  ((GenericRelation*)Bcum.addr)->Clear();

  return 0;
}

const string partjoinSpec =
    "(( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
     "( <text>((array (rel t)) (array (rel u)) (map (rel t) (rel u) r)) "
       "-> (array r)</text--->"
       "<text>_ _ partjoin [ fun ]</text--->"
       "<text>Allows to calculate joins between two arrays of relations "
       "in an efficient way.</text--->"
       "<text>query ar ar partjoin[fun(r1:reltype,r2:reltype)r1 feed r2 feed "
       "rename[A] product count]</text---> ))";

Operator partjoin (
      "partjoin",
      partjoinSpec,
      partjoinFun,
      simpleSelect,
      partjoinTypeMap );

/*
4.20 Operator ~partjoinswitch~

This operator works like operator ~partjoin~ extended by the switch algorithm of operator ~loopswitch~.

The formal specification of type mapping is:

---- ((array (rel t)) (array (rel u)) ((name1 (map (rel t) (rel u) r))
     ... (namen (map (rel t) (rel u) r)))) -> (array r)

     at which t and u are of the type tuple, r may be any type
----

*/
static ListExpr
partjoinswitchTypeMap( ListExpr args )
{
  if (nl->ListLength(args) == 3)
  {
    ListExpr firstArrayDesc = nl->First(args);
    ListExpr secondArrayDesc = nl->Second(args);
    ListExpr funlist = nl->Third(args);

    if ((nl->ListLength(firstArrayDesc) == 2)
        && (nl->ListLength(secondArrayDesc) == 2))
    {
      if ((nl->IsEqual(nl->First(firstArrayDesc), "array"))
          && (nl->IsEqual(nl->First(secondArrayDesc), "array"))
          && !nl->IsAtom(nl->Second(firstArrayDesc))
          && !nl->IsAtom(nl->Second(secondArrayDesc)))
      {
        ListExpr firstElementDesc = nl->Second(firstArrayDesc);
        ListExpr secondElementDesc = nl->Second(secondArrayDesc);

        if ((nl->ListLength(firstElementDesc) == 2)
            && (nl->ListLength(secondElementDesc) == 2)
            && (nl->IsEqual(nl->First(firstElementDesc), "rel"))
            && (nl->IsEqual(nl->First(secondElementDesc), "rel")))
        {
          ListExpr firstTupleDesc = nl->Second(firstElementDesc);
          ListExpr secondTupleDesc = nl->Second(secondElementDesc);

          if ((nl->ListLength(firstTupleDesc) == 2)
              && (nl->ListLength(secondTupleDesc) == 2)
              && (nl->IsEqual(nl->First(firstTupleDesc), "tuple"))
              && (nl->IsEqual(nl->First(secondTupleDesc), "tuple")))
          {
            ListExpr firstFunDesc = nl->First(funlist);
            funlist = nl->Rest(funlist);

            ListExpr funNames = nl->TheEmptyList();
            ListExpr last;

            ListExpr firstMapDesc;

            if (nl->ListLength(firstFunDesc) == 2)
            {
              ListExpr firstFunName = nl->First(firstFunDesc);
              firstMapDesc = nl->Second(firstFunDesc);

              if ((nl->AtomType(firstFunName) == SymbolType)
                  && (nl->ListLength(firstMapDesc) == 4))
              {
                 if ((nl->IsEqual(nl->First(firstMapDesc), "map"))
                     && (!nl->IsEqual(nl->Fourth(firstMapDesc), "typeerror"))
                     && (nl->Equal(firstElementDesc, nl->Second(firstMapDesc)))
                     && (nl->Equal(secondElementDesc,
                                   nl->Third(firstMapDesc))))
                {
                  funNames = nl->OneElemList(
                               nl->StringAtom(nl->SymbolValue(firstFunName)));
                  last = funNames;
                }
              }
            }

            if (!nl->IsEmpty(funNames))
            {
              bool ok = true;

              while (!nl->IsEmpty(funlist) && ok)
              {
                ListExpr funDesc = nl->First(funlist);

                if (nl->ListLength(funDesc) == 2)
                {
                  ListExpr funName = nl->First(funDesc);
                  ListExpr mapDesc = nl->Second(funDesc);

                  if ((nl->AtomType(funName) == SymbolType)
                      && (nl->Equal(mapDesc, firstMapDesc)))
                  {
                    last = nl->Append(last,
                                 nl->StringAtom(nl->SymbolValue(funName)));
                  }
                  else {
                    ok = false;
                  }
                }
                else {
                  ok = false;
                }

                funlist = nl->Rest(funlist);
              }

              if (ok) {
                return nl->ThreeElemList(
                             nl->SymbolAtom("APPEND"),
                             funNames,
                             nl->TwoElemList(
                               nl->SymbolAtom("array"),
                               nl->Fourth(firstMapDesc)));
              }
            }
          }
        }
      }
    }
  }

  return nl->SymbolAtom("typeerror");
}

static int
partjoinswitchFun( Word* args, Word& result, int message, Word& local,
                   Supplier s )
{

  // See value mapping function of operator partjoin for remarks with regard to
  // the partjoin algorithm.

  SecondoCatalog* sc = SecondoSystem::GetCatalog();

  Array* firstArray = ((Array*)args[0].addr);
  Array* secondArray = ((Array*)args[1].addr);

  SwitchAlgorithm sw;

  sw.load(args[2], &args[3]);

  Word funresult;
  string info;

  ListExpr type = qp->GetType(s);
  ListExpr typeOfElement = sc->NumericType(nl->Second(type));

  int algebraId;
  int typeId;

  extractIds(typeOfElement, algebraId, typeId);

  int n = firstArray->getSize();
  int m = secondArray->getSize();

  Word a[n + m - 1];
  int c = 0;

  int i = 0;
  int j = 0;

  Word Acum = SetWord(0);
  Word Bcum = SetWord(0);

  if ((i-j) <= (n-m)) {
    while ((i-j) <= (n-m)) {
      info =  "Processing partitions " + toString(i) + " and " + toString(j);

      sw.request(firstArray->getElement(i), secondArray->getElement(j),
                 funresult, info);
      a[c++] = genericClone(algebraId, typeId, typeOfElement, funresult);

      if (i > 0) {
        appendToRel(Acum, firstArray->getElement(i-1));
      }
      i++;
    }
    j++;
    appendToRel(Bcum, secondArray->getElement(0));
  }
  else {
    while ((i-j) >= (n-m)) {
      info = "Processing partitions " + toString(i) + " and " + toString(j);

      sw.request(firstArray->getElement(i), secondArray->getElement(j),
                 funresult, info);
      a[c++] = genericClone(algebraId, typeId, typeOfElement, funresult);

      appendToRel(Bcum, secondArray->getElement(j));
      j++;
    }
    i++;
  }

  while (i < n) {
    appendToRel(Acum, firstArray->getElement(i-1));
    appendToRel(Bcum, secondArray->getElement(j));

    info = "Processing partitions [0;" + toString(i-1) + "] and "
           + toString(j);

    sw.request(Acum, secondArray->getElement(j), funresult, info);
    a[c++] = genericClone(algebraId, typeId, typeOfElement, funresult);

    info = "Processing partitions " + toString(i) + " and [0;"
           + toString(j) + "]";

    sw.request(firstArray->getElement(i), Bcum, funresult, info);
    a[c++] = genericClone(algebraId, typeId, typeOfElement, funresult);

    i++;
    j++;
  }

  sw.writeSummary();

  result = qp->ResultStorage(s);

  ((Array*)result.addr)->initialize(algebraId, typeId, c, a);

  ((GenericRelation*)Acum.addr)->Clear();
  ((GenericRelation*)Bcum.addr)->Clear();

  return 0;
}

const string partjoinswitchSpec =
   "(( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
    "( <text>((array (rel t)) (array (rel u)) ((name1 (map (rel t) (rel u) r))"
      " ... (namen (map (rel t) (rel u) r)))) -> (array r)</text--->"
      "<text>_ _ partjoinswitch [ funlist ]</text--->"
      "<text>Works like operator partjoin extended by the switch algorithm "
      "of operator loopswitch.</text--->"
      "<text>query ar ar partjoinswitch[f:fun(r11:reltype,r12:reltype)"
      "r11 feed r12 feed rename[A] sortmergejoin[no,no_A] count, "
      "g:fun(r21:reltype,r22:reltype)r21 feed r22 feed rename[A] product "
      "filter[.no=.no_A] count]</text---> ))";

Operator partjoinswitch (
      "partjoinswitch",
      partjoinswitchSpec,
      partjoinswitchFun,
      simpleSelect,
      partjoinswitchTypeMap );

/*
4.21 Operator ~partjoinselect~

This operator works like operator ~partjoin~ extended by the select algorithm of operator ~loopselect~.

The formal specification of type mapping is:

---- ((array (rel t)) (array (rel u)) ((name1 (map (rel t) (rel u) r))
     ... namen (map (rel t) (rel u) r))) int real) -> (array r)

     at which t and u are of the type mtuple, r may be any type
----

*/
static ListExpr
partjoinselectTypeMap( ListExpr args )
{
  if (nl->ListLength(args) == 5)
  {
    ListExpr firstArrayDesc = nl->First(args);
    ListExpr secondArrayDesc = nl->Second(args);
    ListExpr funlist = nl->Third(args);

    if (nl->IsEqual(nl->Fourth(args), "int")
        && nl->IsEqual(nl->Fifth(args), "real"))
    {
      return partjoinswitchTypeMap(nl->ThreeElemList(firstArrayDesc,
                                             secondArrayDesc, funlist));
    }
  }

  return nl->SymbolAtom("typeerror");
}

static int
partjoinselectFun( Word* args, Word& result, int message, Word& local,
                   Supplier s )
{

  // See value mapping function of operator partjoin for remarks with regard to
  // the partjoin algorithm.

  SecondoCatalog* sc = SecondoSystem::GetCatalog();

  Array* firstArray = ((Array*)args[0].addr);
  Array* secondArray = ((Array*)args[1].addr);

  CcInt* absTestCcInt = ((CcInt*)args[3].addr);
  int absTest = absTestCcInt->GetIntval();

  CcReal* relTestCcReal = ((CcReal*)args[4].addr);
  double relTest = relTestCcReal->GetRealval();

  SelectAlgorithm se;

  se.load(args[2], &args[5]);

  Word funresult;
  string info;

  ListExpr type = qp->GetType(s);
  ListExpr typeOfElement = sc->NumericType(nl->Second(type));

  int algebraId;
  int typeId;

  extractIds(typeOfElement, algebraId, typeId);

  int n = firstArray->getSize();
  int m = secondArray->getSize();

  int r = n + m - 1;

  if (absTest < 1) { absTest = 1; }
  if (absTest > r) { absTest = r; }

  if (relTest < 0) { relTest = 0; }
  if (relTest > 1) { relTest = 1; }

  se.setTestSize( max(absTest, (int)(r * relTest + 0.5)) );

  Word a[r];
  int c = 0;

  int i = 0;
  int j = 0;

  Word Acum = SetWord(0);
  Word Bcum = SetWord(0);

  if ((i-j) <= (n-m)) {
    while ((i-j) <= (n-m)) {
      info =  "Processing partitions " + toString(i) + " and " + toString(j);

      se.request(firstArray->getElement(i), secondArray->getElement(j),
                 funresult, info);
      a[c++] = genericClone(algebraId, typeId, typeOfElement, funresult);

      if (i > 0) {
        appendToRel(Acum, firstArray->getElement(i-1));
      }
      i++;
    }
    j++;
    appendToRel(Bcum, secondArray->getElement(0));
  }
  else {
    while ((i-j) >= (n-m)) {
      info = "Processing partitions " + toString(i) + " and " + toString(j);

      se.request(firstArray->getElement(i), secondArray->getElement(j),
                 funresult, info);
      a[c++] = genericClone(algebraId, typeId, typeOfElement, funresult);

      appendToRel(Bcum, secondArray->getElement(j));
      j++;
    }
    i++;
  }

  while (i < n) {
    appendToRel(Acum, firstArray->getElement(i-1));
    appendToRel(Bcum, secondArray->getElement(j));

    info = "Processing partitions [0;" + toString(i-1) + "] and "
           + toString(j);

    se.request(Acum, secondArray->getElement(j), funresult, info);
    a[c++] = genericClone(algebraId, typeId, typeOfElement, funresult);

    info = "Processing partitions " + toString(i) + " and [0;"
           + toString(j) + "]";

    se.request(firstArray->getElement(i), Bcum, funresult, info);
    a[c++] = genericClone(algebraId, typeId, typeOfElement, funresult);

    i++;
    j++;
  }

  se.writeSummary();

  result = qp->ResultStorage(s);

  ((Array*)result.addr)->initialize(algebraId, typeId, c, a);

  ((GenericRelation*)Acum.addr)->Clear();
  ((GenericRelation*)Bcum.addr)->Clear();

  return 0;
}

const string partjoinselectSpec =
   "(( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
    "( <text>((array (rel t)) (array (rel u)) ((name1 (map (rel t) (rel u) r))"
      " ... (namen (map (rel t) (rel u) r))) int real) -> (array r)</text--->"
      "<text>_ _ partjoinselect [ funlist ]</text--->"
      "<text>Works like operator partjoin extended by the select algorithm "
      "of operator loopselect.</text--->"
      "<text>query ar ar partjoinselect[f:fun(r11:reltype,r12:reltype)"
      "r11 feed r12 feed rename[A] sortmergejoin[no,no_A] count, "
      "g:fun(r21:reltype,r22:reltype)r21 feed r22 feed rename[A] product "
      "filter[.no=.no_A] count; 10, 0.1]</text---> ))";

Operator partjoinselect (
      "partjoinselect",
      partjoinselectSpec,
      partjoinselectFun,
      simpleSelect,
      partjoinselectTypeMap );

/*
4.22 Type Operator ~ELEMENT~

Type operators are used only for inferring argument types of parameter functions. They have a type mapping but no evaluation function.

This type operator extracts the type of the elements from an array type given as the first argument.

----    ((array t) ...) -> t
----

*/
ListExpr
ELEMENTTypeMap( ListExpr args )
{
  if(nl->ListLength(args) >= 1)
  {
    ListExpr first = nl->First(args);
    if (nl->ListLength(first) == 2)
    {
      if (nl->IsEqual(nl->First(first), "array")) {
        return nl->Second(first);
      }
    }
  }
  return nl->SymbolAtom("typeerror");
}

const string ELEMENTSpec =
   "(( \"Signature\" \"Syntax\" \"Meaning\" \"Remarks\" )"
    "( <text>((array t) ... ) -> t</text--->"
      "<text>type operator</text--->"
      "<text>Extracts the type of the elements from an array type given "
      "as the first argument.</text--->"
      "<text>not for use with sos-syntax</text---> ))";

Operator ELEMENT (
      "ELEMENT",
      ELEMENTSpec,
      0,
      simpleSelect,
      ELEMENTTypeMap );

/*
4.23 Type Operator ~ELEMENT2~

This type operator extracts the type of the elements from the second array type within a list of argument types.

----    ((array t) (array u) ...) -> u
----

(The first argument must not be an array. It may also be any other type)

*/
ListExpr
ELEMENT2TypeMap( ListExpr args )
{
  if(nl->ListLength(args) >= 2)
  {
    ListExpr second = nl->Second(args);
    if (nl->ListLength(second) == 2)
    {
      if (nl->IsEqual(nl->First(second), "array")) {
        return nl->Second(second);
      }
    }
  }
  return nl->SymbolAtom("typeerror");
}

const string ELEMENT2Spec =
   "(( \"Signature\" \"Syntax\" \"Meaning\" \"Remarks\" )"
    "( <text>((array t) (array u) ... ) -> u</text--->"
      "<text>type operator</text--->"
      "<text>Extracts the type of the elements from an array type given "
      "as the second argument.</text--->"
      "<text>not for use with sos-syntax. The first argument must not "
      "be an array. It may also be any other type.</text---> ))";

Operator ELEMENT2 (
      "ELEMENT2",
      ELEMENT2Spec,
      0,
      simpleSelect,
      ELEMENT2TypeMap );

/*
5 Creating the Algebra

*/
class ArrayAlgebra : public Algebra
{
 public:
  ArrayAlgebra() : Algebra()
  {
    AddTypeConstructor( &array );

    array.AssociateKind("ARRAY");

    AddOperator( &size );
    AddOperator( &get );
    AddOperator( &put );
    AddOperator( &makearray );

    AddOperator( &sortarray );
    AddOperator( &tie );
    AddOperator( &cumulate );

    AddOperator( &distribute );
    AddOperator( &summarize );

    AddOperator( &loop );
    AddOperator( &loopa );
    AddOperator( &loopb );

    AddOperator( &loopswitch );
    AddOperator( &loopswitcha );
    AddOperator( &loopswitchb );

    AddOperator( &loopselect );
    AddOperator( &loopselecta );
    AddOperator( &loopselectb );

    AddOperator( &partjoin );
    AddOperator( &partjoinswitch );
    AddOperator( &partjoinselect );

    AddOperator( &ELEMENT );
    AddOperator( &ELEMENT2 );
  }
  ~ArrayAlgebra() {};
};

ArrayAlgebra arrayAlgebra;

/*
6 Initialization

["]Each algebra module needs an initialization function. The algebra manager has a reference to this function if this algebra is included in the list of required algebras, thus forcing the linker to include this module.

The algebra manager invokes this function to get a reference to the instance of the algebra class and to provide references to the global nested list container (used to store constructor, type, operator and object information) and to the query processor.

The function has a C interface to make it possible to load the algebra dynamically at runtime.["] [Point02]

*/
extern "C"
Algebra*
InitializeArrayAlgebra( NestedList* nlRef, 
                        QueryProcessor* qpRef, 
                        AlgebraManager* amRef )
{
  nl = nlRef;
  qp = qpRef;
  am = amRef;
  return (&arrayAlgebra);
}

}

/*
7 References

[Point02] Algebra Module PointRectangleAlgebra. FernUniversit[ae]t Hagen, Praktische Informatik IV, Secondo System, Directory ["]Algebras/PointRectangle["], file ["]PointRectangleAlgebra.cpp["], since July 2002

*/

