/*
//paragraph [1] title: [{\Large \bf ]	[}]



[1] Array Algebra

Apr 2003 Oliver Lueck

The algebra provides a type constructor ~array~, which defines a generic
array. The elements of the array must have a list representation.

The algebra has three basic operators ~size~, ~get~ and ~put~. The operators
are used to get the size of the array, to retrieve an element with a given
index or to set a value to an element with a given index.

Note that the first element has the index 0. Precondition for the operators
~get~ and ~set~ is a valid index between 0 and the size of the array minus 
1.

The operator ~loop~ evaluates each element of an array with a given function
and returns an array which contains the resulting values.

Additionally the algebra provides two special operators ~distribute~ and
~summarize~ which help to work with arrays of relations.

1 Preliminaries

1.1 Includes

*/
using namespace std;

#include "Algebra.h"
#include "NestedList.h"
#include "QueryProcessor.h"
#include "StandardTypes.h"
#include "RelationAlgebra.h"
#include "TimeTest.h"

namespace {

NestedList* nl;
QueryProcessor* qp;

/*
1.2 Dummy Functions

Not interesting, but needed in the definition of a type constructor.

*/
//static Word
//NoSpace(int size) { return (SetWord(Address(0))); }

//static void
//DoNothing(Word& w) { w.addr = 0; }
 
static void* DummyCast(void* addr) { return (0); }

/*
2 Type Constructor ~array~

2.1 Data Structure - Class ~Array~

*/
class Array
{
  public:
    Array(int, int, int, Word*);
    Array();
    ~Array();
    void initialize(int, int, int, Word*);
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

Array::Array(int algebraId, int typeId, int n, Word* elements) { 
// Constructor with complete initialization of the array

  defined = true;
  elemAlgId = algebraId;
  elemTypeId = typeId;
  size = n;
  array = new Word[size];

  for (int i=0; i<size; i++) {
    array[i] = elements[i];
  }
}

Array::Array() {

  defined = false;
  size = 0;
  elemAlgId = 0;
  elemTypeId = 0;
}

Array::~Array() {

  delete []array;
}

void Array::initialize(int algebraId, int typeId, int n, Word* elements) {
// Precondition: Array is undefined

  if (!defined) {
    defined = true;
    elemAlgId = algebraId;
    elemTypeId = typeId;
    size = n;
    array = new Word[size];

    for (int i=0; i<size; i++) {
     array[i] = elements[i];
    } 
  }
}

int Array::getSize() { return size; }

int Array::getElemAlgId() { return elemAlgId; }

int Array::getElemTypeId() { return elemTypeId; }

Word Array::getElement(int index) { 
// Precondition: Array is defined and index>=0 and index<size

  if (defined && index>=0 && index<size) {
    return array[index];
  } 
  else {
    return SetWord(Address(0));
  }
}

void Array::setElement(int index, Word element) {
// Precondition: Array is defined and index>=0 and index<size

  if (defined && index>=0 && index<size) {
    array[index] = element;
  }
}

/*
2.2 Auxiliary Function

*/
void extractIds(const ListExpr numType, int& algebraId, int& typeId) {

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
2.3 List Representation

The list representation of an array is

----	(a1 a2 ... an)
----
The representation of the elements of the array depends from their type.
So a1 ... an may be nested lists themselves.

2.4 ~In~ and ~Out~ Functions

These functions use the ~In~ and ~Out~ Functions for the elements of 
the array.

*/
static ListExpr
OutArray( ListExpr typeInfo, Word value )
{
//cout << "In OutArray Funktion!" << endl;

  AlgebraManager* am = SecondoSystem::GetAlgebraManager();

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

static Word 
InArray( const ListExpr typeInfo, const ListExpr instance,
         const int errorPos, ListExpr& errorInfo, bool& correct )
{
//cout << "In InArray Funktion!" << endl;

  AlgebraManager* am = SecondoSystem::GetAlgebraManager();

  Array* newarray;

  Word a[nl->ListLength(instance)];
  int algebraId;
  int typeId;

  if (nl->ListLength(instance) > 0) {
  // Array has to consist of at least one element.

    ListExpr typeOfElement = nl->Second(typeInfo);
    ListExpr listOfElements = instance;
    ListExpr element;

    extractIds(typeOfElement, algebraId, typeId);

    int i = 0;

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

/*
2.5 Object ~Creation~, ~Deletion~, ~Close~ and ~Clone~ Functions

The ~Deletion~ and the ~Clone~ functions use the ~Delete~ and ~Clone~ 
functions of the elements of the array.

*/
Word CreateArray(const ListExpr typeInfo) {
//cout << "In CreateArray Function!" << endl;

  return SetWord(new Array());
}

void DeleteArray(Word& w) {
//cout << "In DeleteArray Function!" << endl;

  AlgebraManager* am = SecondoSystem::GetAlgebraManager();
  Array* array = (Array*)w.addr;

  for (int i=0; i<array->getSize(); i++) {
    Word element = array->getElement(i);
    (am->DeleteObj(array->getElemAlgId(),array->getElemTypeId()))(element);
  }

  delete array;
  w.addr = 0;
}

Word CloneArray(const Word& w) {
//cout << "In CloneArray Funktion!" << endl;

  AlgebraManager* am = SecondoSystem::GetAlgebraManager();

  Array* array = (Array*)w.addr;
  Array* newarray;

  int n = array->getSize();
  int algebraId = array->getElemAlgId();
  int typeId = array->getElemTypeId();

  Word a[array->getSize()];

  for (int i=0; i < n; i++) {
    a[i] = (am->CloneObj(algebraId, typeId))(array->getElement(i));
  }

  newarray = new Array(algebraId, typeId, n, a);
  return SetWord(newarray);
}

void CloseArray( Word& w ) {
//cout << "In CloseArray Function!" << endl;

  w.addr = 0;
}
/*
2.6 Function Describing the Signature of the Type Constructor

The type of the elements of the array may be described by any valid Type 
Constructor, but the elements must have a list representation.

*/
static ListExpr
ArrayProperty()
{
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
              nl->StringAtom("The elements of the array must have a list "
                             "representation."))));
}

/*
2.7 Kind Checking Function

The Type Constructor of an array is a list (array type). The first element
of that list is the symbol "array" and the second element has to be a valid
Type Constructor for the elements of the array.

So the second element can be a symbol (e.g. int) or - in case of a more 
complex type - a Nested List itself.

*/
static bool
CheckArray( ListExpr type, ListExpr& errorInfo )
{
  if (nl->ListLength(type) == 2) {

    ListExpr First = nl->First(type);
    ListExpr Second = nl->Second(type);

    if (nl->IsEqual(First, "array")) {
      // Check whether Second is a valid Type Constructor

      SecondoCatalog* sc = SecondoSystem::GetCatalog(ExecutableLevel);

      if (sc->KindCorrect(Second, errorInfo)) {
        return true;
      } 
    }
  }

  return false;
}

/*
2.8 Creation of the Type Constructor Instance

Here an object of the type TypeConstructor is created. The constructor for
an instance of the class TypeConstructor is called with the properties and 
functions for the array as parameters.

*/
TypeConstructor array(
	"array",                      // name		
	ArrayProperty,                // property function describing signature
	OutArray, InArray,            // out and in functions
	CreateArray, DeleteArray,     // object creation and deletion
	0, 0,                         // default object open and save
	CloseArray, CloneArray,       // opject close and clone
	DummyCast,                    // cast function
	CheckArray,                   // kind checking function
	0,                            // predef. pers. function for model
	TypeConstructor::DummyInModel, 	
	TypeConstructor::DummyOutModel,
	TypeConstructor::DummyValueToModel,
	TypeConstructor::DummyValueListToModel );

/*
3 Creating Operators

3.1 Trivial Selection Function

This selection function can be used for the non-overloaded operators 
of the algebra. Currently all operators of this algebra can use the
trivial selection function.

*/
static int
simpleSelect (ListExpr args) { return 0; }

/*
3.1 Operator ~size~

The operator ~size~ returns the number of elements of an array.

The type mapping is

---- ((array x)) -> int
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
sizeFun (Word* args, Word& result, int message, Word& local, Supplier s)
{
  Array* array = ((Array*)args[0].addr);

  result = qp->ResultStorage(s);

  ((CcInt*)result.addr)->Set(true, array->getSize());

  return 0;
}

const string sizeSpec = 
    "(( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
      "( <text>((array x)) -> int</text--->"
        "<text>size( _ )</text--->"
        "<text>Returns the size of an array.</text--->"
        "<text>query size(ai)</text---> ))";

Operator size (
	"size", 	       	   //name
	sizeSpec,                  //specification
	sizeFun,	      	   //value mapping
	Operator::DummyModel,      //dummy model mapping, defined in Algebra.h
	simpleSelect,              //trivial selection function 
	sizeTypeMap                //type mapping 
);

/*
3.2 Operator ~get~

The operator ~get~ returns the element with a given index. So the result type 
of the operator is the type of the array's elements.

The type mapping is

---- ((array x) int) -> x
----

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

/*
Precondition of the value mapping function is a valid index. This means
an index between 0 and the size of the array minus 1. 

The list representation of the elements is used for cloning them because 
some types may have a dummy implementation of their ~Clone~ function.

*/
static int
getFun (Word* args, Word& result, int message, Word& local, Supplier s)
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

    SecondoCatalog* sc = SecondoSystem::GetCatalog(ExecutableLevel);
    AlgebraManager* am = SecondoSystem::GetAlgebraManager();

    Word element = array->getElement(i);

    Word clonedElement;
    ListExpr elemLE;

    int errorPos;
    ListExpr errorInfo;
    bool correct;

    int algebraId = array->getElemAlgId();
    int typeId = array->getElemTypeId();

    ListExpr resultType = qp->GetType(s);
    resultType = sc->NumericType(resultType);

    elemLE = (am->OutObj(algebraId, typeId))(resultType, element);
    clonedElement = (am->InObj(algebraId, typeId))
                           (resultType, elemLE, errorPos, errorInfo, correct);

    result.addr = clonedElement.addr;

    return 0;
  }
  else {
    return 1;
  }
}

const string getSpec = 
    "(( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
      "( <text>((array x) int) -> x</text--->"
        "<text>_ get [ _ ]</text--->"
        "<text>Returns an element with a given index.</text--->"
        "<text>query ai get [3]</text---> ))";

Operator get (
	"get",
	getSpec,
	getFun,
	Operator::DummyModel,
	simpleSelect,
	getTypeMap
);
/*
3.3 Operator ~put~

The operator ~put~ assigns a value to an element of an array.

The type mapping is

---- ((array x) x int) -> (array x) 
----

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

/*
Precondition of the value mapping function is a valid index. This means
an index between 0 and the size of the array minus 1.

The list representation of the elements is used for cloning them because 
some types may have a dummy implementation of their ~Clone~ function.

*/
static int
putFun (Word* args, Word& result, int message, Word& local, Supplier s)
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

    SecondoCatalog* sc = SecondoSystem::GetCatalog(ExecutableLevel);
    AlgebraManager* am = SecondoSystem::GetAlgebraManager();

    int n = array->getSize();
    int algebraId = array->getElemAlgId();
    int typeId = array->getElemTypeId();

    Word a[array->getSize()];
    Word element;    
    ListExpr elemLE;

    int errorPos;
    ListExpr errorInfo;
    bool correct;

    ListExpr resultType = qp->GetType(s);
    ListExpr typeOfElement = sc->NumericType(nl->Second(resultType));

    for (int l=0; l < n; l++) {
      element = (l!=i) ? array->getElement(l) : newelement;
      elemLE = (am->OutObj(algebraId, typeId))(typeOfElement, element);
      a[l] = (am->InObj(algebraId, typeId))
                    (typeOfElement, elemLE, errorPos, errorInfo, correct);
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
      "( <text>((array x) x int) -> (array x)</text--->"
        "<text>_ _ put [ _ ]</text--->"
        "<text>Replaces an element at a given index.</text--->"
        "<text>query ai 9 put [3]</text---> ))";

Operator put (
	"put",
	putSpec,
	putFun,
	Operator::DummyModel,
	simpleSelect,
	putTypeMap
);

/*
3.4 Operator ~makearray~

This Operator creates an array containing the elements of a given list. 
Note that all elements must have the same type. The elements are cloned
before building the array.

The type mapping is

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
makearrayFun (Word* args, Word& result, int message, Word& local, Supplier s)
{
  SecondoCatalog* sc = SecondoSystem::GetCatalog(ExecutableLevel);
  AlgebraManager* am = SecondoSystem::GetAlgebraManager();

  ListExpr type = qp->GetType(s);
  ListExpr typeOfElement = sc->NumericType(nl->Second(type));

  int algebraId;
  int typeId;

  extractIds(typeOfElement, algebraId, typeId);

  int n = qp->GetNoSons(s);

  Word a[n];
  ListExpr elemLE;

  int errorPos;
  ListExpr errorInfo;
  bool correct;

  for (int i=0; i<n; i++) {

    elemLE = (am->OutObj(algebraId, typeId))(typeOfElement, args[i]);
    a[i] = (am->InObj(algebraId, typeId))
                  (typeOfElement, elemLE, errorPos, errorInfo, correct);
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
        "The elements are cloned before building the array.</text--->"
        "<text>let ai = makearray (0,1,2,3)</text---> ))";

Operator makearray (
	"makearray",
	makearraySpec,
	makearrayFun,
	Operator::DummyModel,
	simpleSelect,
	makearrayTypeMap
);

/*
3.5 Operator ~distribute~

The operator ~distribute~ builds an array of relations from an incoming stream 
of tuples. The index of the appropriate relation has to be given by an integer 
attribute of the tuple.

This integer attribute is removed from the tuples in the resulting relations. 
So the incoming tuples have to consist of at least two attributes.

The formal specification of type mapping is:

---- ((stream (tuple ((x1 t1) ... (xn tn)))) xi) 

     -> (array (rel (tuple ((x1 t1) ... (xi-1 ti-1) (xi+1 ti+1) ... (xn tn)))))

     at which n>=2, 1<=i<=n and ti (the type of xi) = int
----

The index of the attribute ai is appended to the result type, because this
information is needed by the value mapping function.

*/
static ListExpr
distributeTypeMap( ListExpr args )
{
  if (nl->ListLength(args) == 2)
  {
    ListExpr streamDesc = nl->First(args);
    ListExpr attrNameLE = nl->Second(args);

    if (nl->IsEqual(nl->First(streamDesc), "stream") 
        && (nl->ListLength(streamDesc) == 2)
        && (nl->AtomType(attrNameLE) == SymbolType))
    {
      ListExpr tupleDesc = nl->Second(streamDesc);
      string attrName = nl->SymbolValue(attrNameLE);

      if (nl->IsEqual(nl->First(tupleDesc), "tuple")
          && (nl->ListLength(tupleDesc) == 2))
      {
        ListExpr attrList = nl->Second(tupleDesc);

        if (IsTupleDescription(attrList, nl))
        {
          int attrIndex;
          ListExpr attrType;

          attrIndex = findattr(attrList, attrName, attrType, nl);

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

  return nl->SymbolAtom("typeerror");
}

/*
The value mapping function implements the operator ~distribute~. An integer 
constant defines the maximum number of relations in the resulting array.

Tuples with an index smaller than 0 or an index greater than the maximum number 
of relations are distributed to the first respectively the last relation.

*/
static int
distributeFun (Word* args, Word& result, int message, Word& local, Supplier s) {

  const int MAX_PKG = 256;

  CcInt* indexAttrCcInt = (CcInt*)args[2].addr;
  int pkgAttr = (indexAttrCcInt->GetIntval()) - 1;

  CcRel* relPkg[MAX_PKG] = { 0 };

  int n = 0;

  relPkg[0] = new CcRel();
  relPkg[0]->Empty();

  CcInt* pkgNrCcInt;
  int pkgNr;

  Word actual;

  qp->Open(args[0].addr);
  qp->Request(args[0].addr, actual);

  while(qp->Received(args[0].addr))
  {
    CcTuple* tuple = (CcTuple*)actual.addr;
    tuple = tuple->CloneIfNecessary();
    tuple->SetFree(false);

    CcTuple* tuple2 = new CcTuple();
    tuple2->SetNoAttrs(tuple->GetNoAttrs() - 1);

    int j = 0;
    for (int i=0; i<tuple->GetNoAttrs(); i++) {
      if (i!=pkgAttr) {
       tuple2->Put(j++, tuple->Get(i));
      }
    }
    tuple2->SetFree(false);

    tuple->DeleteIfAllowed();

    pkgNrCcInt = (CcInt*)(tuple->Get(pkgAttr));
    pkgNr = pkgNrCcInt->GetIntval();

    if (pkgNr < 0) { pkgNr = 0; }
    if (pkgNr > MAX_PKG - 1) { pkgNr = MAX_PKG - 1; }

    while (n < pkgNr) {
      relPkg[++n] = new CcRel();
      relPkg[n]->Empty();
    }
    relPkg[pkgNr]->AppendTuple(tuple2);

    qp->Request(args[0].addr, actual);
  }

  qp->Close(args[0].addr);

  result = qp->ResultStorage(s);

  Word a[++n];

  for (int i=0; i<n; i++) {
    a[i] = SetWord(relPkg[i]);
  }

  int algebraId;
  int typeId;

  SecondoCatalog* sc = SecondoSystem::GetCatalog(ExecutableLevel);

  if (sc->GetTypeId("rel", algebraId, typeId)) {
    Array* newarray = new Array(algebraId, typeId, n, a);
    result = SetWord(newarray);

    return 0;
  }
  else {
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
	Operator::DummyModel,
	simpleSelect,
	distributeTypeMap
);

/*
3.6 Operator ~summarize~

The operator ~summarize~ produces a stream of tuples from an array of 
relations. The operator reads the tuples of all relations beginning
with the first relation of the array.

The formal specification of type mapping is:

---- ((array (rel x))) -> (stream x)
----

Note that the operator ~summarize~ is not exactly inverse to the operator
~distribute~ because the index of the relation is not appended to the
attributes of the outgoing tuples.

If the array has been constructed by the operator ~distribute~ the order
of the resulting stream in most cases does not correspond to the order of
the input stream of the operator ~distribute~.

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
        return nl->TwoElemList(nl->SymbolAtom("stream"), nl->Second(relDesc));
      }
    }
  }

  return nl->SymbolAtom("typeerror");
}

static int
summarizeFun (Word* args, Word& result, int message, Word& local, Supplier s) {

  struct ArrayIterator{int current; CcRelIT* rit;}* ait;

  Array* array;
  CcRel* r;
  Word argArray;
  Word element;

  switch (message) {
    case OPEN :
      ait = new ArrayIterator;
      ait->current = -1;
      local.addr = ait;
      return 0;

    case REQUEST : 
      ait = (ArrayIterator*)local.addr;

      if (ait->current < 0 || ait->rit->EndOfScan()) {
        qp->Request(args[0].addr, argArray);
        array = (Array*)argArray.addr;
      
        while (ait->current < 0 
           || (ait->rit->EndOfScan() && ait->current < array->getSize()-1)) {
          element = array->getElement(++(ait->current));
          r = (CcRel*)element.addr;
          ait->rit = r->MakeNewScan();
        }
      }

      if (!(ait->rit->EndOfScan())) {
        result = SetWord(ait->rit->GetTuple());
        ait->rit->Next();
        return YIELD;
      }
      else {
        return CANCEL;
      }

    case CLOSE : 
      ait = (ArrayIterator*)local.addr;
      delete ait->rit;
      return 0;
  }
  return 0;
}

const string summarizeSpec = 
    "(( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
     "( <text>((array (rel x))) -> (stream x)</text--->"
       "<text>_ summarize</text--->"
       "<text>Produces a stream of the tuples from all relations in the "
       "array.</text--->"
       "<text>query prel summarize consume</text---> ))";

Operator summarize (
	"summarize",
	summarizeSpec,
	summarizeFun,
	Operator::DummyModel,
	simpleSelect,
	summarizeTypeMap
);

/*

3.7 Type Operator ~ELEMENT~

Type operators are used only for inferring argument types of parameter
functions. They have a type mapping but no evaluation function.

This type operator extracts the type of the elements from an array type 
given as the first argument.

----    ((array x) ...) -> x
----

*/
ListExpr ELEMENTTypeMap(ListExpr args)
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
     "( <text>((array x) ... ) -> (x)</text--->"
       "<text>type operator</text--->"
       "<text>Extracts the type of the elements from an array type given "
       "as the first argument.</text--->"
       "<text>not for use with sos-syntax</text---> ))";

Operator ELEMENT (
      "ELEMENT",
      ELEMENTSpec,
      0,
      Operator::DummyModel,
      simpleSelect,
      ELEMENTTypeMap
);

/*

3.8 Type Operator ~ELEMENT2~

Type operators are used only for inferring argument types of parameter
functions. They have a type mapping but no evaluation function.

This type operator extracts the type of the elements from the second 
array type within a list of argument types.

----    ((array x) (array y) ...) -> y
----

(The first argument must not be an array. It may also be any other 
type)
*/
ListExpr ELEMENT2TypeMap(ListExpr args)
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
     "( <text>((array x) (array y) ... ) -> (y)</text--->"
       "<text>type operator</text--->"
       "<text>Extracts the type of the elements from an array type given "
       "as the second argument.</text--->"
       "<text>not for use with sos-syntax. The first argument must not "
       "be an array. It may also be any other type.</text---> ))";

Operator ELEMENT2 (
      "ELEMENT2",
      ELEMENT2Spec,
      0,
      Operator::DummyModel,
      simpleSelect,
      ELEMENT2TypeMap
);

/*
3.9 Operator ~loop~

The Operator ~loop~ evaluates each element of an array with a given function and
returns an array which contains the resulting values.

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

    if (nl->IsEqual(nl->First(arrayDesc), "array") 
        && nl->IsEqual(nl->First(mapDesc), "map")
        && nl->ListLength(mapDesc) == 3)
    {
      if (nl->Equal(nl->Second(arrayDesc), nl->Second(mapDesc)))
      {
        return nl->TwoElemList(nl->SymbolAtom("array"),
                               nl->Third(mapDesc));
      }
    }
  }
  return nl->SymbolAtom("typeerror");
}

static int
loopFun (Word* args, Word& result, int message, Word& local, Supplier s) {
  SecondoCatalog* sc = SecondoSystem::GetCatalog(ExecutableLevel);
  AlgebraManager* am = SecondoSystem::GetAlgebraManager();

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

  ListExpr elemLE;

  int errorPos;
  ListExpr errorInfo;
  bool correct;

  for (int i=0; i<n; i++) {
    (*funargs)[0] = array->getElement(i);

    qp->Request(args[1].addr, funresult);

    elemLE = (am->OutObj(algebraId, typeId))(typeOfElement, funresult);
    a[i] = (am->InObj(algebraId, typeId))
                  (typeOfElement, elemLE, errorPos, errorInfo, correct);
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
	Operator::DummyModel,
	simpleSelect,
	loopTypeMap
);

/*
3.10 Operator ~loop2~

The operator ~loop2~ evaluates each pair of elements from two arrays with 
a given function and returns an array which contains the resulting values.

The formal specification of type mapping is:

---- ((array t) (array u) (map t u r)) -> (array r)
----

*/
static ListExpr
loop2TypeMap( ListExpr args )
{
  if (nl->ListLength(args) == 3) 
  {
    ListExpr firstArrayDesc = nl->First(args);
    ListExpr secondArrayDesc = nl->Second(args);
    ListExpr mapDesc = nl->Third(args); 

    if (nl->IsEqual(nl->First(firstArrayDesc), "array") 
        && nl->IsEqual(nl->First(secondArrayDesc), "array")
        && nl->IsEqual(nl->First(mapDesc), "map")
        && nl->ListLength(mapDesc) == 4)
    {
      if (nl->Equal(nl->Second(firstArrayDesc), nl->Second(mapDesc))
          && nl->Equal(nl->Second(secondArrayDesc), nl->Third(mapDesc)))
      {
        return nl->TwoElemList(nl->SymbolAtom("array"),
                               nl->Fourth(mapDesc));
      }
    }
  }

  return nl->SymbolAtom("typeerror");
}

static int
loop2Fun (Word* args, Word& result, int message, Word& local, Supplier s) {

  SecondoCatalog* sc = SecondoSystem::GetCatalog(ExecutableLevel);
  AlgebraManager* am = SecondoSystem::GetAlgebraManager();

  Array* firstArray = ((Array*)args[0].addr);
  Array* secondArray = ((Array*)args[1].addr);

  ArgVectorPointer funargs = qp->Argument(args[2].addr);
  Word funresult;

  ListExpr type = qp->GetType(s);
  ListExpr typeOfElement = sc->NumericType(nl->Second(type));

  int algebraId;
  int typeId;

  extractIds(typeOfElement, algebraId, typeId);

  int n = firstArray->getSize();
  int m = secondArray->getSize();

  Word a[n * m];

  ListExpr elemLE;

  int errorPos;
  ListExpr errorInfo;
  bool correct;

  for (int i=0; i<n; i++) {
    for (int l=0; l<m; l++) {
      (*funargs)[0] = firstArray->getElement(i);
      (*funargs)[1] = secondArray->getElement(l);

      qp->Request(args[2].addr, funresult);

      elemLE = (am->OutObj(algebraId, typeId))(typeOfElement, funresult);

      a[i * m + l] = (am->InObj(algebraId, typeId))
                          (typeOfElement, elemLE, errorPos, errorInfo, correct);
    }
  }

  result = qp->ResultStorage(s);

  ((Array*)result.addr)->initialize(algebraId, typeId, n * m, a);

  return 0;
}

const string loop2Spec = 
    "(( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
     "( <text>((array t) (array u) (map t u r)) -> (array r)</text--->"
       "<text>_ _ loop2 [ fun ]</text--->"
       "<text>Evaluates each pair of elements from two arrays with a given "
       "function and returns an array which contains the resulting "
       "values.</text--->"
       "<text>query ai al loop2[fun(i:int,l:int)(i+l)]</text---> ))";

Operator loop2 (
	"loop2",
	loop2Spec,
	loop2Fun,
	Operator::DummyModel,
	simpleSelect,
	loop2TypeMap
);

/*
3.11 Operator ~sortarray~

The operator ~sortarray~ sorts an array in order of the function values
of the elements.

The formal specification of type mapping is:

---- ((array t) (map t int)) -> (array t)
----

First an auxiliary class ~IntPair~ is defined. The aim of this class is to 
use the standard sort algorithm in the value mapping function.

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
Implementation of operator ~sortarray~

*/
static ListExpr
sortarrayTypeMap( ListExpr args )
{
  if (nl->ListLength(args) == 2) 
  {
    ListExpr arrayDesc = nl->First(args);
    ListExpr mapDesc = nl->Second(args); 

    if (nl->IsEqual(nl->First(arrayDesc), "array") 
        && nl->IsEqual(nl->First(mapDesc), "map")
        && nl->ListLength(mapDesc) == 3)
    {
      if (nl->Equal(nl->Second(arrayDesc), nl->Second(mapDesc))
          && nl->IsEqual(nl->Third(mapDesc), "int"))
      {
        return arrayDesc;
      }
    }
  }

  return nl->SymbolAtom("typeerror");
}

static int
sortarrayFun (Word* args, Word& result, int message, Word& local, Supplier s) {

  SecondoCatalog* sc = SecondoSystem::GetCatalog(ExecutableLevel);
  AlgebraManager* am = SecondoSystem::GetAlgebraManager();

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

  ListExpr elemLE;

  int errorPos;
  ListExpr errorInfo;
  bool correct;

  for (int i=0; i<n; i++) {
    (*funargs)[0] = array->getElement(i);

    qp->Request(args[1].addr, funresult);

    index[i].first = ((CcInt*)funresult.addr)->GetIntval();
    index[i].second = i;
  }

  sort(index.begin(), index.end());

  for (int i=0; i<n; i++) {

    elemLE = (am->OutObj(algebraId, typeId))
                     (typeOfElement, array->getElement(index[i].second));
    a[i] = (am->InObj(algebraId, typeId))
                  (typeOfElement, elemLE, errorPos, errorInfo, correct);
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

Operator sortarray (
	"sortarray",
	sortarraySpec,
	sortarrayFun,
	Operator::DummyModel,
	simpleSelect,
	sortarrayTypeMap
);

/*
4 Creating the Algebra

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

    AddOperator( &distribute );
    AddOperator( &summarize );

    AddOperator( &ELEMENT );
    AddOperator( &ELEMENT2 );

    AddOperator( &loop );
    AddOperator( &loop2 );

    AddOperator( &sortarray );
  }
  ~ArrayAlgebra() {};
};

ArrayAlgebra arrayAlgebra; 

/*
5 Initialization

Each algebra module needs an initialization function. The algebra manager
has a reference to this function if this algebra is included in the list
of required algebras, thus forcing the linker to include this module.

The algebra manager invokes this function to get a reference to the instance
of the algebra class and to provide references to the global nested list
container (used to store constructor, type, operator and object information)
and to the query processor.

The function has a C interface to make it possible to load the algebra
dynamically at runtime.

*/
extern "C"
Algebra*
InitializeArrayAlgebra( NestedList* nlRef, QueryProcessor* qpRef )
{
  nl = nlRef;
  qp = qpRef;
  return (&arrayAlgebra);
}

}
