/*
//paragraph [1] Title: [{\Large \bf \begin{center}] [\end{center}}]
//paragraph [10] Footnote: [{\footnote{] [}}]
//[TOC] [\tableofcontents]

[1] Implementation of Module Relation Algebra

June 1996 Claudia Freundorfer

May 2002 Frank Hoffmann port to C++

November 7, 2002 RHG Corrected the type mapping of ~count~.

November 30, 2002 RHG Introduced a function ~RelPersistValue~ instead of
~DefaultPersistValue~ which keeps relations that have been built in memory in a
small cache, so that they need not be rebuilt from then on.

March 2003 Victor Almeida created the new Relational Algebra organization.

[TOC]

1 Overview

The Relational Algebra basically implements two type constructors, namely ~tuple~ and ~rel~.
More information about the Relational Algebra can be found in the RelationAlgebra.h header
file.

2 Defines, includes, and constants

*/
#include "RelationAlgebra.h"
#include "OldRelationAlgebra.h"
#include "CPUTimeMeasurer.h"
#include "NestedList.h"
#include "QueryProcessor.h"
#include "Algebra.h"
#include "StandardTypes.h"

extern NestedList* nl;
extern QueryProcessor* qp;

/*
3 Type constructor ~tuple~

The list representation of a tuple is:

----    (<attrrep 1> ... <attrrep n>)
----

Typeinfo is:

----    (<NumericType(<type exression>)> <number of attributes>)
----


For example, for

----    (tuple
                (
                        (name string)
                        (age int)))
----

the typeinfo is

----    (
                (2 2)
                        (
                                (name (1 4))
                                (age (1 1)))
                2)
----

The typeinfo list consists of three lists. The first list is a
pair (~algebraId~, ~typeId~). The second list represents the
attribute list of the tuple. This list is a sequence of pairs
(~attribute\_name~ (~algebraId~ ~typeId~)). Here the
~typeId~ is the identificator of a standard data type, e.g. int.
The third list is an atom and counts the number of the
tuple's attributes.

3.1 Type property of type constructor ~tuple~

*/
ListExpr TupleProp ()
{
  ListExpr examplelist = nl->TextAtom();
  nl->AppendText(examplelist,"(\"Myers\" 53)");

  return (nl->TwoElemList(
            nl->FourElemList(nl->StringAtom("Signature"),
	                     nl->StringAtom("Example Type List"),
			     nl->StringAtom("List Rep"),
			     nl->StringAtom("Example List")),
            nl->FourElemList(nl->StringAtom("(ident x DATA)+ -> TUPLE"),
	                     nl->StringAtom("(tuple((name string)(age int)))"),
			     nl->StringAtom("(<attr1> ... <attrn>)"),
			     examplelist)));
}

/*
3.2 ~Out~-function of type constructor ~tuple~

The ~out~-function of type constructor ~tuple~ takes as inputs a type
description (~typeInfo~) of the tuples attribute structure in nested list
format and a pointer to a tuple value, stored in main memory.
The function returns the tuple value from main memory storage
in nested list format.

*/
ListExpr
OutTuple (ListExpr typeInfo, Word  value)
{
  return ((Tuple *)value.addr)->Out( typeInfo );
}

/*
3.3 ~SaveToList~-function of type constructor ~tuple~

The ~SaveToList~-function should act as the ~Out~-function
but using internal representation of the objects. It is called
by the default persistence mechanism to store the objects in
the database.

*/
ListExpr
SaveToListTuple (ListExpr typeInfo, Word  value)
{
  return ((Tuple *)value.addr)->SaveToList( typeInfo );
}

/*
3.3 ~In~-function of type constructor ~tuple~

The ~In~-function of type constructor ~tuple~ takes as inputs a type
description (~typeInfo~) of the tuples attribute structure in nested
list format and the tuple value in nested list format. The function
returns a pointer to a tuple value, stored in main memory in accordance to
the tuple value in nested list format.

Error handling in ~InTuple~: ~correct~ is only true if there is the right
number of attribute values and all values have correct list representations.
Otherwise the following error messages are added to ~errorInfo~:

----    (71 tuple 1 <errorPos>)                 atom instead of value list
        (71 tuple 2 <errorPos>)                 not enough values
        (71 tuple 3 <errorPos> <attrno>)        wrong attribute value in
                                                attribute <attrno>
        (71 tuple 4 <errorPos>)                 too many values
----

is added to ~errorInfo~. Here ~errorPos~ is the number of the tuple in the
relation list (passed by ~InRelation~).


*/
Word
InTuple(ListExpr typeInfo, ListExpr value,
        int errorPos, ListExpr& errorInfo, bool& correct)
{
  return SetWord( Tuple::In( typeInfo, value, errorPos, errorInfo, correct ) );
}

/*
3.3 ~RestoreFromList~-function of type constructor ~tuple~

The ~RestoreFromList~-function should act as the ~In~-function
but using internal representation of the objects. It is called
by the default persistence mechanism to retrieve the objects in
the database.

*/
Word
RestoreFromListTuple(ListExpr typeInfo, ListExpr value,
                     int errorPos, ListExpr& errorInfo, bool& correct)
{
  return SetWord( Tuple::RestoreFromList( typeInfo, value, errorPos, errorInfo, correct ) );
}

/*
3.4 ~Delete~-function of type constructor ~tuple~

A type constructor's ~delete~-function is used by the query processor in order
to deallocate memory occupied by instances of Secondo objects. They may have
been created in two ways:

  * as return values of operator calls

  * by calling a type constructor's ~create~-function.

*/
void DeleteTuple(Word& w)
{
  delete (Tuple *)w.addr;
}

/*

3.5 ~Check~-function of type constructor ~tuple~

Checks the specification:

----    (ident x DATA)+         -> TUPLE        tuple
----

with the additional constraint that all identifiers used (attribute names)
must be distinct. Hence a tuple type has the form:

----    (tuple
            (
                (age x)
                (name y)))
----

and ~x~ and ~y~ must be types of kind DATA. Kind TUPLE introduces the
following error codes:

----    (... 1)         Empty tuple type
        (... 2 x)       x is not an attribute list, but an atom
        (... 3 x)       Doubly defined attribute name x
        (... 4 x)       Invalid attribute name x
        (... 5 x)       Invalid attribute definition x (x is not a pair)
        (... 6 x)       Attribute type does not belong to kind DATA
----

*/
bool
CheckTuple(ListExpr type, ListExpr& errorInfo)
{
  vector<string> attrnamelist;
  ListExpr attrlist, pair;
  string attrname;
  bool correct, ckd;
  int unique;
  AlgebraManager* algMgr;

  if ((nl->ListLength(type) == 2) && (nl->IsEqual(nl->First(type), "tuple",
       true)))
  {
    attrlist = nl->Second(type);
    if (nl->IsEmpty(attrlist))
    {
      errorInfo = nl->Append(errorInfo,
        nl->ThreeElemList(nl->IntAtom(61), nl->SymbolAtom("TUPLE"),
          nl->IntAtom(1)));
      return false;
    }
    if (nl->IsAtom(attrlist))
    {
      errorInfo = nl->Append(errorInfo,
        nl->FourElemList(nl->IntAtom(61), nl->SymbolAtom("TUPLE"),
          nl->IntAtom(2),
        attrlist));
      return false;
    }
    algMgr = SecondoSystem::GetAlgebraManager();

    unique = 0;
    correct = true;
    while (!nl->IsEmpty(attrlist))
    {
      pair = nl->First(attrlist);
      attrlist = nl->Rest(attrlist);
      if (nl->ListLength(pair) == 2)
      {
        if ((nl->IsAtom(nl->First(pair))) &&
          (nl->AtomType(nl->First(pair)) == SymbolType))
        {
          attrname = nl->SymbolValue(nl->First(pair));
          unique = std::count(attrnamelist.begin(), attrnamelist.end(),
                         attrname);
          if (unique > 0)
          {
            errorInfo = nl->Append(errorInfo,
             nl->FourElemList(nl->IntAtom(61), nl->SymbolAtom("TUPLE"),
               nl->IntAtom(3), nl->First(pair)));
            correct = false;
          }
          attrnamelist.push_back(attrname);
          ckd =  algMgr->CheckKind("DATA", nl->Second(pair), errorInfo);
          if (!ckd)
          {
            errorInfo = nl->Append(errorInfo,
              nl->FourElemList(nl->IntAtom(61), nl->SymbolAtom("TUPLE"),
                nl->IntAtom(6),nl->Second(pair)));
          }
          correct = correct && ckd;
        }
        else
        {
          errorInfo = nl->Append(errorInfo,
          nl->FourElemList(nl->IntAtom(61), nl->SymbolAtom("TUPLE"),
          nl->IntAtom(4),nl->First(pair)));
          correct = false;
        }
      }
      else
      {
        errorInfo = nl->Append(errorInfo,
          nl->FourElemList(nl->IntAtom(61), nl->SymbolAtom("TUPLE"),
          nl->IntAtom(5),pair ));
        correct = false;
      }
    }
    return correct;
  }
  else
  {
    errorInfo = nl->Append(errorInfo,
      nl->ThreeElemList(nl->IntAtom(60), nl->SymbolAtom("TUPLE"), type));
    return false;
  }
}

/*
3.6 ~Cast~-function of type constructor ~tuple~

Casts a tuple from a stream representation of it. This function is used to read
objects from the disk by the ~TupleManager~. Since tuples are not part of relations
the implementation of this function is not necessary.

*/
void*
CastTuple(void* addr)
{
  return ( 0 );
}

/*
3.7 ~Create~-function of type constructor ~tuple~

This function is used to allocate memory sufficient for keeping one instance
of ~tuple~.

*/
Word
CreateTuple(const ListExpr typeInfo)
{
  Tuple *tup = new Tuple( nl->Second( typeInfo ) );
  return (SetWord(tup));
//  return (SetWord(Address(0)));
}

/*
3.8 ~Close~-function of type constructor ~tuple~

This function is used to destroy the memory allocated by a ~tuple~.

*/
void CloseTuple(Word& w)
{
  delete (Tuple *)w.addr;
}

/*
3.9 ~Clone~-function of type constructor ~tuple~

This function creates a cloned tuple.

*/
Word
CloneTuple(const Word& w)
{
  return SetWord( ((Tuple *)w.addr)->Clone() );
}

/*
3.10 ~Sizeof~-function of type constructor ~tuple~

Returns the size of a tuple's root record to be stored on the disk as a stream.
Since tuples are not part of a relation, the implementation of this function
is not necessary.

*/
int
SizeOfTuple()
{
  return 0;
}

/*
3.11 ~Model~-functions of type constructor ~tuple~

*/
Word
TupleInModel( ListExpr typeExpr, ListExpr list, int objNo )
{
  return (SetWord( Address( 0 ) ));
}

ListExpr
TupleOutModel( ListExpr typeExpr, Word model )
{
  return (0);
}

Word
TupleValueToModel( ListExpr typeExpr, Word value )
{
  return (SetWord( Address( 0 ) ));
}

Word
TupleValueListToModel( const ListExpr typeExpr, const ListExpr valueList,
                       const int errorPos, ListExpr& errorInfo, bool& correct )
{
  correct = true;
  return (SetWord( Address( 0 ) ));
}

/*
3.12 Definition of type constructor ~tuple~

Eventually a type constructor is created by defining an instance of
class ~TypeConstructor~. Constructor's arguments are the type constructor's
name and the eleven functions previously defined.

*/
TypeConstructor cpptuple( "tuple",           	TupleProp,
                          OutTuple,          	InTuple,
                          SaveToListTuple,      RestoreFromListTuple,
                          CreateTuple,		DeleteTuple,
			  0, 			0,
                          CloseTuple, 		CloneTuple,
                          CastTuple,   		SizeOfTuple,
                          CheckTuple,           0,
			  TupleInModel,      	TupleOutModel,
			  TupleValueToModel, 	TupleValueListToModel );
/*
4 TypeConstructor ~rel~

The list representation of a relation is:

----    (<tuplerep 1> ... <tuplerep n>)
----

Typeinfo is:

----    (<NumericType(<type exression>)>)
----

For example, for

----    (rel (tuple ((name string) (age int))))
----

the type info is

----    ((2 1) ((2 2) ((name (1 4)) (age (1 1)))))
----

4.1 Type property of type constructor ~rel~

*/
ListExpr RelProp ()
{
  ListExpr listreplist = nl->TextAtom();
  ListExpr examplelist = nl->TextAtom();
  nl->AppendText(listreplist,"(<tuple>*)where <tuple> is "
  "(<attr1> ... <attrn>)");
  nl->AppendText(examplelist,"((\"Myers\" 53)(\"Smith\" 21))");

  return (nl->TwoElemList(
            nl->FourElemList(nl->StringAtom("Signature"),
	                     nl->StringAtom("Example Type List"),
			     nl->StringAtom("List Rep"),
			     nl->StringAtom("Example List")),
            nl->FourElemList(nl->StringAtom("TUPLE -> REL"),
	               nl->StringAtom("(rel(tuple((name string)(age int))))"),
		       listreplist,
		       examplelist)));
}

/*
4.2 ~Out~-function of type constructor ~rel~

*/
ListExpr
OutRel(ListExpr typeInfo, Word  value)
{
  return ((Relation *)value.addr)->Out( typeInfo );
}
/*
4.3 ~SaveToList~-function of type constructor ~rel~

The ~SaveToList~-function should act as the ~Out~-function
but using internal representation of the objects. It is called
by the default persistence mechanism to store the objects in
the database.

*/
ListExpr
SaveToListRel(ListExpr typeInfo, Word  value)
{
  return ((Relation *)value.addr)->SaveToList( typeInfo );
}

/*
4.3 ~Create~-function of type constructor ~rel~

The function is used to allocate memory sufficient for keeping one instance
of ~rel~.

*/
Word
CreateRel(const ListExpr typeInfo)
{
  Relation* rel = new Relation( typeInfo );
  return (SetWord(rel));
}

/*
4.4 ~In~-function of type constructor ~rel~

~value~ is the list representation of the relation. The structure of
~typeInfol~ and ~value~ are described above. Error handling in ~InRel~:

The result relation will contain all tuples that have been converted
correctly (have correct list expressions). For all other tuples, an error
message containing the position of the tuple within this relation (list) is
added to ~errorInfo~. (This is done by procedure ~InTuple~ called by ~InRel~).
If any tuple representation is wrong, then ~InRel~ will return ~correct~ as
FALSE and will itself add an error message of the form

----    (InRel <errorPos>)
----

to ~errorInfo~. The value in ~errorPos~ has to be passed from the environment;
probably it is the position of the relation object in the list of
database objects.

*/
Word
InRel(ListExpr typeInfo, ListExpr value,
      int errorPos, ListExpr& errorInfo, bool& correct)
{
  return SetWord( Relation::In( typeInfo, value, errorPos, errorInfo, correct ) );
}

/*
4.3 ~RestoreFromList~-function of type constructor ~rel~

The ~RestoreFromList~-function should act as the ~In~-function
but using internal representation of the objects. It is called
by the default persistence mechanism to retrieve the objects in
the database.

*/
Word
RestoreFromListRel(ListExpr typeInfo, ListExpr value,
                   int errorPos, ListExpr& errorInfo, bool& correct)
{
  return SetWord( Relation::RestoreFromList( typeInfo, value, errorPos, errorInfo, correct ) );
}

/*
4.5 ~Delete~-function of type constructor ~rel~

*/
void DeleteRel(Word& w)
{
  return ((Relation *)w.addr)->Delete();
}

/*
4.6 ~Check~-function of type constructor ~rel~

Checks the specification:

----    TUPLE   -> REL          rel
----

Hence the type expression must have the form

----    (rel x)
----

and ~x~ must be a type of kind TUPLE.

*/
bool
CheckRel(ListExpr type, ListExpr& errorInfo)
{
  AlgebraManager* algMgr;

  if ((nl->ListLength(type) == 2) && nl->IsEqual(nl->First(type), "rel"))
  {
    algMgr = SecondoSystem::GetAlgebraManager();
    return (algMgr->CheckKind("TUPLE", nl->Second(type), errorInfo));
  }
  else
  {
    errorInfo = nl->Append(errorInfo,
      nl->ThreeElemList(nl->IntAtom(60), nl->SymbolAtom("REL"), type));
    return false;
  }
}

/*
4.7 ~Cast~-function of type constructor ~rel~

*/
void*
CastRel(void* addr)
{
  return ( 0 );
}

/*
4.8 ~Close~-function of type constructor ~rel~

There is a cache of relations in order to increase performance.
The cache is responsible for closing the relations.
In this case we will implement one function that does nothing, called
~CloseRel~ and another which the cache will execute, called
~CacheCloseRel~.

*/
void CloseRel(Word& w)
{
  return ((Relation *)w.addr)->Close();
}

/*
4.9 ~Open~-function of type constructor ~rel~

This is a slightly modified version of the function ~DefaultOpen~ (from
~Algebra~) which creates the relation from the SmiRecord only if it does not
yet exist.

The idea is to maintain a cache containing the relation representations that
have been built in memory. The cache basically stores pairs (~recordId~,
~relation\_value~). If the ~recordId~ passed to this function is found,
the cached relation value is returned instead of building a new one.

*/
bool
OpenRel( SmiRecord& valueRecord,
         const ListExpr typeInfo,
         Word& value )
{
  return Relation::Open( valueRecord, typeInfo, (Relation *)value.addr );
}

/*
4.10 ~Save~-function of type constructor ~rel~

*/
bool
SaveRel( SmiRecord& valueRecord,
         const ListExpr typeInfo,
         Word& value )
{
  return ((Relation *)value.addr)->Save( valueRecord, typeInfo );
}

/*
4.11 ~Sizeof~-function of type constructor ~rel~

*/
int
SizeOfRel()
{
  return 0;
}

/*
4.12 ~Clone~-function of type constructor ~rel~

*/
Word
CloneRel(const Word& w)
{
  return SetWord( Address(0) );
}

/*
4.13 ~Model~-functions of type constructor ~rel~

*/
Word RelInModel( ListExpr typeExpr, ListExpr list, int objNo )
{
  return (SetWord( Address( 0 ) ));
}

ListExpr RelOutModel( ListExpr typeExpr, Word model )
{
  return (0);
}

Word RelValueToModel( ListExpr typeExpr, Word value )
{
  return (SetWord( Address( 0 ) ));
}

Word RelValueListToModel( const ListExpr typeExpr, const ListExpr valueList,
                       const int errorPos, ListExpr& errorInfo, bool& correct )
{
  correct = true;
  return (SetWord( Address( 0 ) ));
}

/*

4.14 Definition of type constructor ~rel~

Eventually a type constructor is created by defining an instance of
class ~TypeConstructor~. Constructor's arguments are the type constructor's
name and the eleven functions previously defined.

*/

TypeConstructor cpprel( "rel",           RelProp,
                        OutRel,          InRel,
	                SaveToListRel,   RestoreFromListRel,
                        CreateRel, 	 DeleteRel,
			OpenRel, 	 SaveRel,
                        CloseRel,        CloneRel,
                        CastRel,         SizeOfRel,
                        CheckRel,        0,
			RelInModel,      RelOutModel,
			RelValueToModel, RelValueListToModel );

/*

5 Operators

5.2 Selection function for type operators

The selection function of a type operator always returns -1.

*/
int TypeOperatorSelect(ListExpr args)
{
  return -1;
}

/*
5.3 Type Operator ~TUPLE~

Type operators are used only for inferring argument types of parameter
functions. They have a type mapping but no evaluation function.

5.3.1 Type mapping function of operator ~TUPLE~

Extract tuple type from a stream or relation type given as the first argument.

----    ((stream x) ...)                -> x
        ((rel x)    ...)                -> x
----

*/
ListExpr TUPLETypeMap(ListExpr args)
{
  ListExpr first;
  if(nl->ListLength(args) >= 1)
  {
    first = nl->First(args);
    if(nl->ListLength(first) == 2  )
    {
      if ((TypeOfRelAlgSymbol(nl->First(first)) == stream)  ||
          (TypeOfRelAlgSymbol(nl->First(first)) == rel))
        return nl->Second(first);
    }
  }
  return nl->SymbolAtom("typeerror");
}
/*

5.3.2 Specification of operator ~TUPLE~

*/
const string TUPLESpec =
	"( ( \"Signature\" \"Syntax\" \"Meaning\" "
	"\"Remarks\" ) "
	"( <text>((stream x)...) -> x, ((rel x)...) -> "
	"x</text--->"
	"<text>type operator</text--->"
	"<text>Extract tuple type from a stream or "
	"relation type given as the first argument."
	"</text--->"
	"<text>not for use with sos-syntax</text--->"
	"  ) )";
/*

5.3.3 Definition of operator ~TUPLE~

*/
Operator relalgTUPLE (
         "TUPLE",              // name
         TUPLESpec,            // specification
         0,                    // no value mapping
         Operator::DummyModel, // dummy model mapping, defines in Algebra.h
         TypeOperatorSelect,   // trivial selection function
         TUPLETypeMap          // type mapping
);
/*

5.4 Type Operator ~TUPLE2~

5.4.1 Type mapping function of operator ~TUPLE2~

Extract tuple type from a stream or relation type given as the second argument.

----    ((stream x) (stream y) ...)          -> y
        ((rel x) (rel y) ...)                -> y
----

*/
ListExpr TUPLE2TypeMap(ListExpr args)
{
  ListExpr second;
  if(nl->ListLength(args) >= 2)
  {
    second = nl->Second(args);
    if(nl->ListLength(second) == 2  )
    {
      if ((TypeOfRelAlgSymbol(nl->First(second)) == stream)  ||
          (TypeOfRelAlgSymbol(nl->First(second)) == rel))
        return nl->Second(second);
    }
  }
  return nl->SymbolAtom("typeerror");
}
/*

5.4.2 Specification of operator ~TUPLE2~

*/
const string TUPLE2Spec  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                           "\"Remarks\" ) "
                           "( <text><text>((stream x) (stream y) ...) -> y, "
                           "((rel x) (rel y) ...) -> y</text--->"
                           "<text>type operator</text--->"
                           "<text>Extract tuple type from a stream or "
                           "relation"
                           " type given as the second argument.</text--->"
                           "<text>not for use with sos-syntax</text--->"
                           ") )";

/*

5.4.3 Definition of operator ~TUPLE2~

*/
Operator relalgTUPLE2 (
         "TUPLE2",             // name
         TUPLE2Spec,           // specification
         0,                    // no value mapping
         Operator::DummyModel, // dummy model mapping, defines in Algebra.h
         TypeOperatorSelect,   // trivial selection function
         TUPLE2TypeMap         // type mapping
);

/*

5.5 Operator ~feed~

Produces a stream from a relation by scanning the relation tuple by tuple.

5.5.1 Type mapping function of operator ~feed~

A type mapping function takes a nested list as argument. Its contents are
type descriptions of an operator's input parameters. A nested list describing
the output type of the operator is returned.

Result type of feed operation.

----	((rel x))		-> (stream x)
----

*/
ListExpr FeedTypeMap(ListExpr args)
{
  ListExpr first ;

  CHECK_COND(nl->ListLength(args) == 1,
    "Operator feed expects a list of length one.");
  first = nl->First(args);
  CHECK_COND(nl->ListLength(first) == 2,
    "Operator feed expects an argument of type relation.");
  CHECK_COND(TypeOfRelAlgSymbol(nl->First(first)) == rel,
    "Operator feed expects an argument of type relation.");
  return nl->Cons(nl->SymbolAtom("stream"), nl->Rest(first));
}
/*

5.5.2 Value mapping function of operator ~feed~

*/
int
Feed(Word* args, Word& result, int message, Word& local, Supplier s)
{
  GenericRelation* r;
  GenericRelationIterator* rit;
  Word argRelation;


  switch (message)
  {
    case OPEN :
      qp->Request(args[0].addr, argRelation);
      r = ((GenericRelation*)argRelation.addr);
      rit = r->MakeScan();

      local = SetWord(rit);
      return 0;

    case REQUEST :
      rit = (GenericRelationIterator*)local.addr;
      Tuple *t;
      if ((t = rit->GetNextTuple()) != 0)
      {
        result = SetWord(t);
        return YIELD;
      }
      else
      {
        return CANCEL;
      }

    case CLOSE :
      rit = (GenericRelationIterator*)local.addr;
      delete rit;
      return 0;
  }
  return 0;
}
/*

5.5.3 Specification of operator ~feed~

*/
const string FeedSpec  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                         "\"Example\" ) "
                         "( <text>(rel x) -> (stream x)</text--->"
                         "<text>_ feed</text--->"
                         "<text>Produces a stream from a relation by "
                         "scanning the relation tuple by tuple.</text--->"
                         "<text>query cities feed consume</text--->"
                         ") )";

/*

5.5.4 Definition of operator ~feed~

Non-overloaded operators are defined by constructing a new instance of
class ~Operator~, passing all operator functions as constructor arguments.

*/
Operator relalgfeed (
          "feed",                // name
          FeedSpec,              // specification
          Feed,                  // value mapping
          Operator::DummyModel, // dummy model mapping, defines in Algebra.h
          Operator::SimpleSelect,         // trivial selection function
          FeedTypeMap           // type mapping
);

/*
5.6 Operator ~consume~

Collects objects from a stream into a relation.

5.6.1 Type mapping function of operator ~consume~

Operator ~consume~ accepts a stream of tuples and returns a relation.


----    (stream  x)                 -> ( rel x)
----

*/
ListExpr ConsumeTypeMap(ListExpr args)
{
  ListExpr first ;

  if(nl->ListLength(args) == 1)
  {
    first = nl->First(args);
    if ((nl->ListLength(first) == 2) &&
        (TypeOfRelAlgSymbol(nl->First(first)) == stream) &&
        (nl->ListLength(nl->Second(first)) == 2) &&
        (TypeOfRelAlgSymbol(nl->First(nl->Second(first))) == tuple))
      return nl->Cons(nl->SymbolAtom("rel"), nl->Rest(first));
  }
  ErrorReporter::ReportError("Incorrect input for operator consume.");
  return nl->SymbolAtom("typeerror");
}
/*

5.6.2 Value mapping function of operator ~consume~

*/
int
Consume(Word* args, Word& result, int message, Word& local, Supplier s)
{
  Word actual;
  Relation* rel;

  rel = (Relation*)((qp->ResultStorage(s)).addr);
  if(rel->GetNoTuples() > 0)
  {
    rel->Clear();
  }

  qp->Open(args[0].addr);
  qp->Request(args[0].addr, actual);
  while (qp->Received(args[0].addr))
  {
    Tuple* tuple = ((Tuple*)actual.addr)->CloneIfNecessary();
    rel->AppendTuple(tuple);
    if( tuple != actual.addr )
      ((Tuple*)actual.addr)->DeleteIfAllowed();
    tuple->Delete();

    qp->Request(args[0].addr, actual);
  }

  result = SetWord((void*) rel);

  qp->Close(args[0].addr);

  return 0;
}
/*

5.6.3 Specification of operator ~consume~

*/
const string ConsumeSpec  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                            "\"Example\" ) "
                            "( <text>(stream x) -> (rel x)</text--->"
                            "<text>_ consume</text--->"
                            "<text>Collects objects from a stream."
                            "</text--->"
                            "<text>query cities feed consume</text--->"
                            ") )";

/*

5.6.4 Definition of operator ~consume~

*/
Operator relalgconsume (
         "consume",            // name
	 ConsumeSpec,          // specification
	 Consume,              // value mapping
	 Operator::DummyModel, // dummy model mapping, defines in Algebra.h
	 Operator::SimpleSelect,         // trivial selection function
	 ConsumeTypeMap        // type mapping
);
/*

5.7 Operator ~attr~

5.7.1 Type mapping function of operator ~attr~

Result type attr operation.

----
    ((tuple ((x1 t1)...(xn tn))) xi)    -> ti
                            APPEND (i) ti)
----
This type mapping uses a special feature of the query processor, in that if
requests to append a further argument to the given list of arguments, namely,
the index of the attribute within the tuple. This indes is computed within
the type mapping  function. The request is given through the result expression
of the type mapping which has the form, for example,

----

    (APPEND (1) string)

----

The keyword ~APPEND~ occuring as a first element of a returned type expression
tells the query processor to add the elements of the following list - the
second element of the type expression - as further arguments to the operator
(as if they had been written in the query). The third element  of the query
is then used as the real result type. In this case 1 is the index of the
attribute determined in this procedure. The query processor, more precisely
the procedure ~anotate~ there, will produce the annotation for the constant 1,
append it to the list of annotated arguments, and then use "string" as the
result type of the ~attr~ operation.

*/
ListExpr AttrTypeMap(ListExpr args)
{
  ListExpr first, second, attrtype;
  string  attrname;
  int j;
  if(nl->ListLength(args) == 2)
  {
    first = nl->First(args);
    second  = nl->Second(args);

    if((nl->ListLength(first) == 2  ) &&
        (TypeOfRelAlgSymbol(nl->First(first)) == tuple)  &&
        (nl->IsAtom(second)) &&
        (nl->AtomType(second) == SymbolType))
    {
      attrname = nl->SymbolValue(second);
      j = FindAttribute(nl->Second(first), attrname, attrtype);
      if (j)
      return nl->ThreeElemList(nl->SymbolAtom("APPEND"),
                  nl->OneElemList(nl->IntAtom(j)), attrtype);
    }
    ErrorReporter::ReportError("Incorrect input for operator attr.");
    return nl->SymbolAtom("typeerror");
  }
  ErrorReporter::ReportError("Incorrect input for operator attr.");
  return nl->SymbolAtom("typeerror");
}
/*

5.7.2 Value mapping function of operator ~attr~

The argument vector ~arg~ contains in the first slot ~args[0]~ the tuple
and in ~args[2]~ the position of the attribute as a number. Returns as
~result~ the value of an attribute at the given position ~args[2]~ in a
tuple object. The attribute name is argument 2 in the query and is used
in the function ~AttributeTypeMap~ to determine the attribute
number ~args[2]~ .

*/
int
Attr(Word* args, Word& result, int message, Word& local, Supplier s)
{
  Tuple* tupleptr;
  int index;

  tupleptr = (Tuple*)args[0].addr;
  index = ((CcInt*)args[2].addr)->GetIntval();
  assert( 1 <= index && index <= tupleptr->GetNoAttributes() );
  result = SetWord(tupleptr->GetAttribute(index - 1));
  return 0;
}
/*

5.7.3 Specification of operator ~attr~

*/
const string AttrSpec  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                         "\"Remarks\" ) "
                         "( <text>((tuple ((x1 t1)...(xn tn))) xi)  -> "
                         "ti)</text--->"
                         "<text>attr ( _ , _ )</text--->"
                         "<text>Returns the value of an attribute at a "
                         "given position.</text--->"
                         "<text>not for use with sos-syntax</text--->"
                         ") )";

/*

5.7.4 Definition of operator ~attr~

*/
Operator relalgattr (
     "attr",           // name
     AttrSpec,        // specification
     Attr,            // value mapping
     Operator::DummyModel, // dummy model mapping, defines in Algebra.h
     Operator::SimpleSelect,         // trivial selection function
     AttrTypeMap      // type mapping
);
/*

5.8 Operator ~filter~

Only tuples, fulfilling a certain condition are passed on to the output stream.

5.8.1 Type mapping function of operator ~filter~

Result type of filter operation.

----    ((stream (tuple x)) (map (tuple x) bool))       -> (stream (tuple x))
----

*/
ListExpr FilterTypeMap(ListExpr args)
{
  ListExpr first, second;
  if(nl->ListLength(args) == 2)
  {
    first = nl->First(args);
    second  = nl->Second(args);

    if ( (nl->ListLength(first) == 2)
	&& (nl->ListLength(second) == 3)
	&& (nl->ListLength(nl->Second(first)) == 2)
	&& (TypeOfRelAlgSymbol(nl->First(nl->Second(first))) == tuple)
	&& (TypeOfRelAlgSymbol(nl->First(first)) == stream)
	&& (TypeOfRelAlgSymbol(nl->First(second)) == ccmap)
	&& (TypeOfRelAlgSymbol(nl->Third(second)) == ccbool)
	&& (nl->Equal(nl->Second(first),nl->Second(second)))	)
    return first;
  }

  ErrorReporter::ReportError( "Incorrect input for operator filter.");
  return nl->SymbolAtom("typeerror");
}

/*

5.8.2 Value mapping function of operator ~filter~

*/
int
Filter(Word* args, Word& result, int message, Word& local, Supplier s)
{
  bool found;
  Word elem, funresult;
  ArgVectorPointer funargs;
  Tuple* tuple;

  switch ( message )
  {

    case OPEN:

      qp->Open (args[0].addr);
      return 0;

    case REQUEST:

      funargs = qp->Argument(args[1].addr);
      qp->Request(args[0].addr, elem);
      found = false;
      while (qp->Received(args[0].addr) && !found)
      {
        tuple = (Tuple*)elem.addr;
        (*funargs)[0] = elem;
        qp->Request(args[1].addr, funresult);
        if (((StandardAttribute*)funresult.addr)->IsDefined())
        {
          found = ((CcBool*)funresult.addr)->GetBoolval();
        }
        if (!found)
        {
          tuple->DeleteIfAllowed();
          qp->Request(args[0].addr, elem);
        }
      }
      if (found)
      {
        result = SetWord(tuple);
        return YIELD;
      }
      else
        return CANCEL;

    case CLOSE:

      qp->Close(args[0].addr);
      return 0;
  }
  return 0;
}
/*

5.8.3 Specification of operator ~filter~

*/
const string FilterSpec  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                           "\"Example\" ) "
                           "( <text>((stream x) (map x bool)) -> "
                           "(stream x)</text--->"
                           "<text>_ filter [ fun ]</text--->"
                           "<text>Only tuples, fulfilling a certain "
                           "condition are passed on to the output "
                           "stream.</text--->"
                           "<text>query cities feed filter "
                           "[.population > 500000] consume</text--->"
                              ") )";

/*

5.8.4 Definition of operator ~filter~

*/
Operator relalgfilter (
         "filter",            // name
         FilterSpec,           // specification
         Filter,               // value mapping
         Operator::DummyModel, // dummy model mapping, defines in Algebra.h
         Operator::SimpleSelect,         // trivial selection function
         FilterTypeMap         // type mapping
);
/*

5.9 Operator ~project~

5.9.1 Type mapping function of operator ~filter~

Result type of project operation.

----	((stream (tuple ((x1 T1) ... (xn Tn)))) (ai1 ... aik))	->

		(APPEND
			(k (i1 ... ik))
			(stream (tuple ((ai1 Ti1) ... (aik Tik))))
		)
----

The type mapping computes the number of attributes and the list of attribute
numbers for the given projection attributes and asks the query processor to
append it to the given arguments.

*/
ListExpr ProjectTypeMap(ListExpr args)
{
  bool firstcall;
  int noAttrs, j;
  ListExpr first, second, first2, attrtype, newAttrList, lastNewAttrList,
           lastNumberList, numberList, outlist;
  string attrname;

  firstcall = true;
  if (nl->ListLength(args) == 2)
  {
    first = nl->First(args);
    second = nl->Second(args);

    if ((nl->ListLength(first) == 2) &&
        (TypeOfRelAlgSymbol(nl->First(first)) == stream) &&
	(nl->ListLength(nl->Second(first)) == 2) &&
	(TypeOfRelAlgSymbol(nl->First(nl->Second(first))) == tuple) &&
	(!nl->IsAtom(second)) &&
	(nl->ListLength(second) > 0))
    {
      noAttrs = nl->ListLength(second);
      while (!(nl->IsEmpty(second)))
      {
        first2 = nl->First(second);
	second = nl->Rest(second);
	if (nl->AtomType(first2) == SymbolType)
	{
	  attrname = nl->SymbolValue(first2);
	}
	else
        {
          ErrorReporter::ReportError("Incorrect input for operator project.");
          return nl->SymbolAtom("typeerror");
        }
	j = FindAttribute(nl->Second(nl->Second(first)), attrname, attrtype);
	if (j)
	{
	  if (firstcall)
	  {
	    firstcall = false;
	    newAttrList = nl->OneElemList(nl->TwoElemList(first2, attrtype));
	    lastNewAttrList = newAttrList;
	    numberList = nl->OneElemList(nl->IntAtom(j));
	    lastNumberList = numberList;
	  }
	  else
	  {
	    lastNewAttrList =
	      nl->Append(lastNewAttrList, nl->TwoElemList(first2, attrtype));
	    lastNumberList =
	      nl->Append(lastNumberList, nl->IntAtom(j));
	  }
	}
	else
  {
    ErrorReporter::ReportError("Incorrect input for operator project.");
    return nl->SymbolAtom("typeerror");
  }
      }
      // Check whether all new attribute names are distinct
      // - not yet implemented
      outlist = nl->ThreeElemList(
                 nl->SymbolAtom("APPEND"),
		 nl->TwoElemList(nl->IntAtom(noAttrs), numberList),
		 nl->TwoElemList(nl->SymbolAtom("stream"),
		               nl->TwoElemList(nl->SymbolAtom("tuple"),
			                     newAttrList)));
      return outlist;
    }
  }
  ErrorReporter::ReportError("Incorrect input for operator project.");
  return nl->SymbolAtom("typeerror");
}
/*

5.9.2 Value mapping function of operator ~project~

*/
int
Project(Word* args, Word& result, int message, Word& local, Supplier s)
{
  switch (message)
  {
    case OPEN :
    {
      ListExpr resultType = GetTupleResultType( s );
      TupleType *tupleType = new TupleType( nl->Second( resultType ) );
      local.addr = tupleType;

      qp->Open(args[0].addr);
      return 0;
    }
    case REQUEST :
    {
      Word elem1, elem2, arg2;
      int noOfAttrs, index;
      Supplier son;
      Attribute* attr;

      qp->Request(args[0].addr, elem1);
      if (qp->Received(args[0].addr))
      {
        TupleType *tupleType = (TupleType *)local.addr;
        Tuple *t = new Tuple( *tupleType, true );
        assert( t->IsFree() );

        qp->Request(args[2].addr, arg2);
        noOfAttrs = ((CcInt*)arg2.addr)->GetIntval();
        assert( t->GetNoAttributes() == noOfAttrs );

        for( int i = 0; i < noOfAttrs; i++)
        {
          son = qp->GetSupplier(args[3].addr, i);
          qp->Request(son, elem2);
          index = ((CcInt*)elem2.addr)->GetIntval();
          attr = ((Tuple*)elem1.addr)->GetAttribute(index-1);
          t->PutAttribute(i, ((StandardAttribute*)attr->Clone()));
        }
        ((Tuple*)elem1.addr)->DeleteIfAllowed();
        result = SetWord(t);
        return YIELD;
      }
      else return CANCEL;
    }
    case CLOSE :
    {
      delete (TupleType *)local.addr;
      qp->Close(args[0].addr);
      return 0;
    }
  }
  return 0;
}
/*

5.9.3 Specification of operator ~project~

*/
const string ProjectSpec  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                            "\"Example\" ) "
                            "( <text>((stream (tuple ((x1 T1) ... "
                            "(xn Tn)))) (ai1 ... aik)) -> (stream (tuple"
                            " ((ai1 Ti1) ... (aik Tik))))</text--->"
                            "<text>_ project [ list ]</text--->"
                            "<text>Produces a projection tuple for each "
                            "tuple of its input stream.</text--->"
                            "<text>query cities feed project[cityname, "
                            "population] consume</text--->"
                              ") )";

/*

5.9.4 Definition of operator ~project~

*/
Operator relalgproject (
         "project",            // name
         ProjectSpec,          // specification
         Project,              // value mapping
         Operator::DummyModel, // dummy model mapping, defines in Algebra.h
         Operator::SimpleSelect,         // trivial selection function
         ProjectTypeMap        // type mapping
);

/*

5.10 Operator ~product~

5.10.1 Type mapping function of operator ~product~

Result type of product operation.

----	((stream (tuple (x1 ... xn))) (stream (tuple (y1 ... ym))))

	-> (stream (tuple (x1 ... xn y1 ... ym)))
----

*/
ListExpr ProductTypeMap(ListExpr args)
{
  ListExpr first, second, list, list1, list2, outlist;

  if (nl->ListLength(args) == 2)
  {
    first = nl->First(args); second = nl->Second(args);

    // Check first argument and extract list1
    if (nl->ListLength(first) == 2)
    {
      if (TypeOfRelAlgSymbol(nl->First(first)) == stream)
      {
        if (nl->ListLength(nl->Second(first)) == 2)
        {
          if (TypeOfRelAlgSymbol(nl->First(nl->Second(first))) == tuple)
          {
            list1 = nl->Second(nl->Second(first));
          }
          else goto typeerror;
        }
        else goto typeerror;
      }
      else goto typeerror;
    }
    else goto typeerror;

    // Check second argument and extract list2
    if (nl->ListLength(second) == 2)
    {
      if (TypeOfRelAlgSymbol(nl->First(second)) == stream)
      {
        if (nl->ListLength(nl->Second(second)) == 2)
        {
          if (TypeOfRelAlgSymbol(nl->First(nl->Second(second))) == tuple)
          {
            list2 = nl->Second(nl->Second(second));
          }
          else goto typeerror;
        }
        else goto typeerror;
      }
      else goto typeerror;
    }
    else goto typeerror;

    list = ConcatLists(list1, list2);
    // Check whether all new attribute names are distinct
    // - not yet implemented

    if ( CompareNames(list) )
    {
      outlist = nl->TwoElemList(nl->SymbolAtom("stream"),
        nl->TwoElemList(nl->SymbolAtom("tuple"), list));
      return outlist;
    }
    else goto typeerror;
  }
  else goto typeerror;

typeerror:
  ErrorReporter::ReportError("Incorrect input for operator product.");
  return nl->SymbolAtom("typeerror");
}
/*

5.10.2 Value mapping function of operator ~product~

*/

CPUTimeMeasurer productMeasurer;

class NoOrder;

struct ProductLocalInfo
{
  TupleType *resultTupleType;
  Tuple* currentTuple;
  TupleBuffer *rightRel;
  TupleBufferIterator *iter;
};

int
Product(Word* args, Word& result, int message, Word& local, Supplier s)
{
  Word r, u;
  ProductLocalInfo* pli;

  switch (message)
  {
    case OPEN :
    {
      qp->Open(args[0].addr);
      qp->Request(args[0].addr, r);
      pli = new ProductLocalInfo;
      pli->currentTuple = qp->Received(args[0].addr) ? (Tuple*)r.addr : 0;

      /* materialize right stream */
      qp->Open(args[1].addr);
      qp->Request(args[1].addr, u);

      if(qp->Received(args[1].addr))
      {
        pli->rightRel = new TupleBuffer();
      }
      else
      {
        pli->rightRel = 0;
      }

      while(qp->Received(args[1].addr))
      {
        Tuple *t = ((Tuple*)u.addr)->CloneIfNecessary();
        pli->rightRel->AppendTuple( t );
        ((Tuple*)u.addr)->DeleteIfAllowed();
        qp->Request(args[1].addr, u);
      }

      if( pli->rightRel )
      {
        pli->iter = pli->rightRel->MakeScan();
      }
      else
      {
        pli->iter = 0;
      }

      ListExpr resultType = GetTupleResultType( s );
      pli->resultTupleType = new TupleType( nl->Second( resultType ) );

      local = SetWord(pli);
      return 0;
    }
    case REQUEST :
    {
      Tuple *resultTuple, *rightTuple;
      pli = (ProductLocalInfo*)local.addr;

      productMeasurer.Enter();

      if (pli->currentTuple == 0)
      {
        productMeasurer.Exit();
        return CANCEL;
      }
      else
      {
        if( pli->rightRel == 0 ) // second stream is empty
        {
          productMeasurer.Exit();
          return CANCEL;
        }
        else if( (rightTuple = pli->iter->GetNextTuple()) != 0 )
        {
          resultTuple = new Tuple( *(pli->resultTupleType), true );
          assert( resultTuple->IsFree() );

          Concat(pli->currentTuple, rightTuple, resultTuple);
          result = SetWord(resultTuple);
          productMeasurer.Exit();
          return YIELD;
        }
        else
        {
          /* restart iterator for right relation and
             fetch a new tuple from left stream */
          pli->currentTuple->DeleteIfAllowed();
          pli->currentTuple = 0;
          delete pli->iter;
          pli->iter = 0;
          qp->Request(args[0].addr, r);
          if (qp->Received(args[0].addr))
          {
            pli->currentTuple = (Tuple*)r.addr;
            pli->iter = pli->rightRel->MakeScan();
            assert( (rightTuple = pli->iter->GetNextTuple()) != 0 );

            resultTuple = new Tuple( *(pli->resultTupleType), true );
            assert( resultTuple->IsFree() );

            Concat(pli->currentTuple, rightTuple, resultTuple);
            result = SetWord(resultTuple);
            productMeasurer.Exit();
            return YIELD;
          }
          else
          {
            productMeasurer.Exit();
            return CANCEL; // left stream exhausted
          }
        }
      }
    }
    case CLOSE :
    {
      pli = (ProductLocalInfo*)local.addr;
      if(pli->currentTuple != 0)
        pli->currentTuple->DeleteIfAllowed();
      if( pli->iter != 0 )
        delete pli->iter;
      delete pli->resultTupleType;
      if( pli->rightRel )
      {
        pli->rightRel->Clear();
        delete pli->rightRel;
      }
      delete pli;

      qp->Close(args[0].addr);
      qp->Close(args[1].addr);

      productMeasurer.PrintCPUTimeAndReset("Product CPU Time : ");
      return 0;
    }
  }
  return 0;
}
/*

5.10.3 Specification of operator ~product~

*/
const string ProductSpec  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                            "\"Example\" ) "
                            "( <text>((stream (tuple (x1 ... xn))) (stream "
                            "(tuple (y1 ... ym)))) -> (stream (tuple (x1 "
                            "... xn y1 ... ym)))</text--->"
                            "<text>_ _ product</text--->"
                            "<text>Computes a Cartesian product stream from "
                            "its two argument streams.</text--->"
                            "<text>query ten feed twenty feed product count"
                            "</text--->"
                             " ) )";

/*

5.10.4 Definition of operator ~product~

*/
Operator relalgproduct (
         "product",            // name
         ProductSpec,          // specification
         Product,              // value mapping
         Operator::DummyModel, // dummy model mapping, defines in Algebra.h
         Operator::SimpleSelect,         // trivial selection function
         ProductTypeMap        // type mapping
);
/*

5.11 Operator ~count~

Count the number of tuples within a stream of tuples.


5.11.1 Type mapping function of operator ~count~

Operator ~count~ accepts a stream of tuples and returns an integer.

----    (stream  (tuple x))                 -> int
----

*/
ListExpr
TCountTypeMap(ListExpr args)
{
  ListExpr first;

  if( nl->ListLength(args) == 1 )
  {
    first = nl->First(args);
    if ( (nl->ListLength(first) == 2) && nl->ListLength(nl->Second(first)) == 2  )
    {
      if ( ( TypeOfRelAlgSymbol(nl->First(first)) == stream
             || TypeOfRelAlgSymbol(nl->First(first)) == rel )
	   && TypeOfRelAlgSymbol(nl->First(nl->Second(first))) == tuple   )
      return nl->SymbolAtom("int");
    }
  }
  ErrorReporter::ReportError("Incorrect input for operator count.");
  return nl->SymbolAtom("typeerror");
}


/*

5.11.2 Value mapping functions of operator ~count~

*/
int
TCountStream(Word* args, Word& result, int message, Word& local, Supplier s)
{
  Word elem;
  int count = 0;

  qp->Open(args[0].addr);
  qp->Request(args[0].addr, elem);
  while ( qp->Received(args[0].addr) )
  {
    ((Tuple*)elem.addr)->DeleteIfAllowed();
    count++;
    qp->Request(args[0].addr, elem);
  }
  result = qp->ResultStorage(s);
  ((CcInt*) result.addr)->Set(true, count);
  qp->Close(args[0].addr);
  return 0;
}

int
TCountRel(Word* args, Word& result, int message, Word& local, Supplier s)
{
  Relation* rel = (Relation*)args[0].addr;
  result = qp->ResultStorage(s);
  ((CcInt*) result.addr)->Set(true, rel->GetNoTuples());
  return 0;
}


/*

5.11.3 Specification of operator ~count~

*/
const string TCountSpec  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                           "\"Example\" ) "
                           "( <text>((stream/rel (tuple x))) -> int"
                           "</text--->"
                           "<text>_ count</text--->"
                           "<text>Count number of tuples within a stream "
                           "or a relation of tuples.</text--->"
                           "<text>query cities count or query cities "
                           "feed count</text--->"
                              ") )";

/*

5.11.4 Selection function of operator ~count~

*/

int
TCountSelect( ListExpr args )
{
  ListExpr first ;

  if(nl->ListLength(args) == 1)
  {
    first = nl->First(args);
    if(nl->ListLength(first) == 2)
    {
      if (TypeOfRelAlgSymbol(nl->First(first)) == stream)
      {
        return 0;
      }
      else
      {
        if(TypeOfRelAlgSymbol(nl->First(first)) == rel)
        {
          return 1;
        }
      }
    }
  }
  return -1;
}

/*

5.11.5 Definition of operator ~count~

*/
Word
RelNoModelMapping( ArgVector arg, Supplier opTreeNode )
{
  return (SetWord( Address( 0 ) ));
}

ValueMapping countmap[] = {TCountStream, TCountRel };
ModelMapping nomodelmap[] = {RelNoModelMapping, RelNoModelMapping};

Operator relalgcount (
         "count",           // name
         TCountSpec,         // specification
         2,                  // number of value mapping functions
         countmap,          // value mapping functions
         nomodelmap,         // dummy model mapping functions
         TCountSelect,       // trivial selection function
         TCountTypeMap       // type mapping
);

/*

5.11 Operator ~tuplesize~

Reports the average size of the tuples in a relation. This operator is
useful for the optimizer, but it is usable as an operator itself.

5.11.1 Type mapping function of operator ~tuplesize~

Operator ~tuplesize~ accepts a stream of tuples and returns an integer.

----    (stream  (tuple x))                 -> real
----

*/
ListExpr
TupleSizeTypeMap(ListExpr args)
{
  ListExpr first;

  if( nl->ListLength(args) == 1 )
  {
    first = nl->First(args);
    if ( (nl->ListLength(first) == 2) && nl->ListLength(nl->Second(first)) == 2  )
    {
      if ( ( TypeOfRelAlgSymbol(nl->First(first)) == stream
             || TypeOfRelAlgSymbol(nl->First(first)) == rel ) &&
           TypeOfRelAlgSymbol(nl->First(nl->Second(first))) == tuple )
        return nl->SymbolAtom("real");
    }
  }
  ErrorReporter::ReportError("Incorrect input for operator tuplesize.");
  return nl->SymbolAtom("typeerror");
}

/*

5.11.2 Value mapping functions of operator ~count~

*/
int
TupleSizeStream(Word* args, Word& result, int message, Word& local, Supplier s)
{
  Word elem;
  int count = 0;
  float totalSize = 0;

  qp->Open(args[0].addr);
  qp->Request(args[0].addr, elem);
  while ( qp->Received(args[0].addr) )
  {
    totalSize += ((Tuple*)elem.addr)->GetTotalSize();
    count++;
    ((Tuple*)elem.addr)->DeleteIfAllowed();
    qp->Request(args[0].addr, elem);
  }
  result = qp->ResultStorage(s);

  cout << "Total size: " << totalSize << endl
       << "Count: " << count << endl
       << "Average size: " << totalSize/count << endl;

  ((CcReal*) result.addr)->Set(true, totalSize/count);
  qp->Close(args[0].addr);
  return 0;
}

int
TupleSizeRel(Word* args, Word& result, int message, Word& local, Supplier s)
{
  Relation* rel = (Relation*)args[0].addr;
  result = qp->ResultStorage(s);
  ((CcReal*) result.addr)->Set(true, (float)rel->GetTotalSize()/rel->GetNoTuples());
  return 0;
}


/*

5.11.3 Specification of operator ~tuplesize~

*/
const string TupleSizeSpec  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                              "\"Example\" ) "
                              "( <text>((stream/rel (tuple x))) -> real"
                              "</text--->"
                              "<text>_ tuplesize</text--->"
                              "<text>Return the average size of the tuples within a stream "
                              "or a relation.</text--->"
                              "<text>query cities tuplesize or query cities "
                              "feed tuplesize</text--->"
                              ") )";

/*

5.11.4 Selection function of operator ~tuplesize~

This function is the same as for the ~count~ operator.

5.11.5 Definition of operator ~tuplesize~

*/
ValueMapping tuplesizemap[] = {TupleSizeStream, TupleSizeRel };

Operator relalgtuplesize (
         "tuplesize",           // name
         TupleSizeSpec,         // specification
         2,                  // number of value mapping functions
         tuplesizemap,          // value mapping functions
         nomodelmap,         // dummy model mapping functions
         TCountSelect,       // trivial selection function
         TupleSizeTypeMap       // type mapping
);

/*

5.12 Operator ~rename~

Renames all attribute names by adding them with the postfix passed as parameter.

5.12.1 Type mapping function of operator ~rename~

Type mapping for ~rename~ is

----	((stream (tuple([a1:d1, ... ,an:dn)))ar) ->
           (stream (tuple([a1ar:d1, ... ,anar:dn)))
----

*/
ListExpr
RenameTypeMap( ListExpr args )
{
  ListExpr first, first2, second, rest, listn, lastlistn;
  string  attrname;
  string  attrnamen;
  bool firstcall = true;
  if(nl->ListLength(args) == 2)
  {
    first = nl->First(args);
    second  = nl->Second(args);

    if((nl->ListLength(first) == 2  ) &&
    	(TypeOfRelAlgSymbol(nl->First(first)) == stream) &&
	(TypeOfRelAlgSymbol(nl->First(nl->Second(first))) == tuple) &&
       	nl->IsAtom(second) &&
	(nl->AtomType(second) == SymbolType))
    {
      rest = nl->Second(nl->Second(first));
      while (!(nl->IsEmpty(rest)))
      {
	first2 = nl->First(rest);
	rest = nl->Rest(rest);
	nl->SymbolValue(nl->First(first));
	attrname = nl->SymbolValue(nl->First(first2));
	attrnamen = nl->SymbolValue(second);
	attrname.append("_");
	attrname.append(attrnamen);

	if (!firstcall)
	{
	  lastlistn  =
	    nl->Append(lastlistn,
        nl->TwoElemList(nl->SymbolAtom(attrname), nl->Second(first2)));
	}
	else
	{
	  firstcall = false;
 	  listn = nl->OneElemList(nl->TwoElemList(nl->SymbolAtom(attrname),
        nl->Second(first2)));
	  lastlistn = listn;
	}
      }
      return
        nl->TwoElemList(nl->SymbolAtom("stream"),
		nl->TwoElemList(nl->SymbolAtom("tuple"),
		listn));
    }
    ErrorReporter::ReportError("Incorrect input for operator rename.");
    return nl->SymbolAtom("typeerror");
  }
    ErrorReporter::ReportError("Incorrect input for operator rename.");
  return nl->SymbolAtom("typeerror");
}
/*

5.12.2 Value mapping function of operator ~rename~

*/
int
Rename(Word* args, Word& result, int message, Word& local, Supplier s)
{
  Word t;
  Tuple* tuple;

  switch (message)
  {
    case OPEN :

      qp->Open(args[0].addr);
      return 0;

    case REQUEST :

      qp->Request(args[0].addr,t);
      if (qp->Received(args[0].addr))
      {
        tuple = (Tuple*)t.addr;
        result = SetWord(tuple);
        return YIELD;
      }
      else return CANCEL;

    case CLOSE :

      qp->Close(args[0].addr);
      return 0;
  }
  return 0;
}
/*

5.12.3 Specification of operator ~rename~

*/
const string RenameSpec  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                           "\"Example\" ) "
                           "( <text>((stream (tuple([a1:d1, ... ,"
                           "an:dn)))ar) -> (stream (tuple([a1ar:d1, "
                           "... ,anar:dn)))</text--->"
                           "<text>_ rename [ _ ] or just _ { _ }"
                           "</text--->"
                           "<text>Renames all attribute names by adding"
                           " them with the postfix passed as parameter. "
                           "NOTE: parameter must be of symbol type."
                           "</text--->"
                           "<text>query ten feed rename [ r1 ] consume "
                           "or query ten feed {r1} consume, the result "
                           "has format e.g. n_r1</text--->"
                              ") )";

/*

5.12.4 Definition of operator ~rename~

*/
Operator relalgrename (
         "rename",             // name
         RenameSpec,           // specification
         Rename,               // value mapping
         Operator::DummyModel, // dummy model mapping, defines in Algebra.h
         Operator::SimpleSelect,         // trivial selection function
         RenameTypeMap         // type mapping
);


/*
5.13 Operator ~mconsume~

Collects objects from a stream of tuples into a
main memory relation using the ~mrel~ type constructor
of the old relational algebra.

This operator is used to convert from a relation in
the new relational algebra to the old one.

5.6.1 Type mapping function of operator ~mconsume~

Operator ~mconsume~ accepts a stream of tuples and returns a
main memory relation.


----    (stream tuple(x))             -> ( mrel mtuple(x) )
----

*/
ListExpr MConsumeTypeMap(ListExpr args)
{
  ListExpr first ;

  if(nl->ListLength(args) == 1)
  {
    first = nl->First(args);
    if ((nl->ListLength(first) == 2) &&
        (TypeOfRelAlgSymbol(nl->First(first)) == stream) &&
        (nl->ListLength(nl->Second(first)) == 2) &&
        (TypeOfRelAlgSymbol(nl->First(nl->Second(first))) == tuple))
    {
      return nl->TwoElemList(nl->SymbolAtom("mrel"),
                             nl->TwoElemList(nl->SymbolAtom("mtuple"),
                                             nl->Second(nl->Second(first))));
    }
  }
  ErrorReporter::ReportError("Incorrect input for operator consume.");
  return nl->SymbolAtom("typeerror");
}
/*

5.6.2 Value mapping function of operator ~mconsume~

*/
int
MConsume(Word* args, Word& result, int message, Word& local, Supplier s)
{
  Word actual;
  CcRel* rel;

  rel = (CcRel*)((qp->ResultStorage(s)).addr);
  if(rel->GetNoTuples() > 0)
  {
    rel->Empty();
  }

  qp->Open(args[0].addr);
  qp->Request(args[0].addr, actual);
  while (qp->Received(args[0].addr))
  {
    CcTuple* tuple = ((Tuple*)actual.addr)->CloneToMemoryTuple( false );
    rel->AppendTuple(tuple);
    ((Tuple*)actual.addr)->DeleteIfAllowed();

    qp->Request(args[0].addr, actual);
  }

  result = SetWord((void*) rel);

  qp->Close(args[0].addr);

  return 0;
}
/*

5.6.3 Specification of operator ~mconsume~

*/
const string MConsumeSpec  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                             "\"Example\" ) "
                             "( <text>(stream tuple(x)) -> (mrel mtuple(x))</text--->"
                             "<text>_ mconsume</text--->"
                             "<text>Collects objects from a stream into an mrel."
                             "</text--->"
                             "<text>query cities feed mconsume</text--->"
                             ") )";

/*

5.6.4 Definition of operator ~mconsume~

*/
Operator relalgmconsume (
         "mconsume",            // name
	 MConsumeSpec,          // specification
	 MConsume,              // value mapping
	 Operator::DummyModel, // dummy model mapping, defines in Algebra.h
	 Operator::SimpleSelect,         // trivial selection function
	 MConsumeTypeMap        // type mapping
);

/*

6 Class ~RelationAlgebra~

A new subclass ~RelationAlgebra~ of class ~Algebra~ is declared. The only
specialization with respect to class ~Algebra~ takes place within the
constructor: all type constructors and operators are registered at the actual algebra.

After declaring the new class, its only instance ~RelationAlgebra~ is defined.

*/

class RelationAlgebra : public Algebra
{
 public:
  RelationAlgebra() : Algebra()
  {
    AddTypeConstructor( &cpptuple );
    AddTypeConstructor( &cpprel );

    AddOperator(&relalgfeed);
    AddOperator(&relalgconsume);
    AddOperator(&relalgTUPLE);
    AddOperator(&relalgTUPLE2);
    AddOperator(&relalgattr);
    AddOperator(&relalgfilter);
    AddOperator(&relalgproject);
    AddOperator(&relalgproduct);
    AddOperator(&relalgcount);
    AddOperator(&relalgtuplesize);
    AddOperator(&relalgrename);
    AddOperator(&relalgmconsume);

    cpptuple.AssociateKind( "TUPLE" );
    cpprel.AssociateKind( "REL" );

  }
  ~RelationAlgebra() {};
};

RelationAlgebra relationalgebra;
/*

7 Initialization

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
InitializeRelationAlgebra( NestedList* nlRef, QueryProcessor* qpRef )
{
  nl = nlRef;
  qp = qpRef;
  return (&relationalgebra);
}
