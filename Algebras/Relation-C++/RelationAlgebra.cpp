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
//paragraph [10] Footnote: [{\footnote{] [}}]
//[TOC] [\tableofcontents]

[1] Implementation of Module Relation Algebra

June 1996 Claudia Freundorfer

May 2002 Frank Hoffmann port to C++

November 7, 2002 RHG Corrected the type mapping of ~count~.

November 30, 2002 RHG Introduced a function ~RelPersistValue~ 
instead of ~DefaultPersistValue~ which keeps relations that have
been built in memory in a small cache, so that they need not be 
rebuilt from then on.

March 2003 Victor Almeida created the new Relational Algebra 
organization.

December 2005, Victor Almeida deleted the deprecated algebra levels
(~executable~, ~descriptive~, and ~hibrid~). Only the executable
level remains. Models are also removed from type constructors.

January 2006 Victor Almeida replaced the ~free~ tuples concept to
reference counters. There are reference counters on tuples and also
on attributes. Some assertions were removed, since the code is
stable.

April 2006, M. Spiekermann. New operators ~dumpstream~ and ~sizecounters~
added.

[TOC]

1 Overview

The Relational Algebra basically implements two type constructors, 
namely ~tuple~ and ~rel~.

More information about the Relational Algebra can be found in the 
RelationAlgebra.h header file.

2 Defines, includes, and constants

*/
#include "RelationAlgebra.h"
#include "OldRelationAlgebra.h"
#include "NestedList.h"
#include "QueryProcessor.h"
#include "Algebra.h"
#include "StandardTypes.h"

#include "LogMsg.h"
#include "NList.h"

extern NestedList* nl;
extern QueryProcessor* qp;
extern AlgebraManager *am;

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
            (age (1 1))))
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
            nl->FourElemList(
              nl->StringAtom("Signature"),
              nl->StringAtom("Example Type List"),
              nl->StringAtom("List Rep"),
              nl->StringAtom("Example List")),
            nl->FourElemList(
              nl->StringAtom("(ident x DATA)+ -> TUPLE"),
              nl->StringAtom("(tuple((name string)(age int)))"),
              nl->StringAtom("(<attr1> ... <attrn>)"),
              examplelist)));
}

/*
3.2 ~Out~-function of type constructor ~tuple~

The ~out~-function of type constructor ~tuple~ takes as inputs a 
type description (~typeInfo~) of the tuple's attribute structure in 
nested list format and a pointer to a tuple value, stored in main 
memory. The function returns the tuple value in nested list format.

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

The ~In~-function of type constructor ~tuple~ takes as inputs a 
type description (~typeInfo~) of the tuples attribute structure in 
nested list format and the tuple value in nested list format. The 
function returns a pointer to a tuple value, stored in main memory
in accordance to the tuple value in nested list format.

Error handling in ~InTuple~: ~correct~ is only true if there is the 
right number of attribute values and all values have correct list 
representations.

Otherwise the following error messages are added to ~errorInfo~:

----    (71 tuple 1 <errorPos>)          atom instead of value list
        (71 tuple 2 <errorPos>)          not enough values
        (71 tuple 3 <errorPos> <attrno>) wrong attribute value in
                                         attribute <attrno>
        (71 tuple 4 <errorPos>)          too many values
----

is added to ~errorInfo~. Here ~errorPos~ is the number of the tuple 
in the relation list (passed by ~InRelation~).


*/
Word
InTuple(ListExpr typeInfo, ListExpr value,
        int errorPos, ListExpr& errorInfo, bool& correct)
{
  return SetWord( Tuple::In( typeInfo, value, errorPos, 
                             errorInfo, correct ) );
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
                     int errorPos, ListExpr& errorInfo, 
                     bool& correct)
{
  return 
    SetWord( Tuple::RestoreFromList( typeInfo, value, errorPos, 
                                     errorInfo, correct ) );
}

/*
3.4 ~Delete~-function of type constructor ~tuple~

A type constructor's ~delete~-function is used by the query 
processor in order to deallocate memory occupied by instances 
of Secondo objects. They may have been created in two ways:

  * as return values of operator calls

  * by calling a type constructor's ~create~-function.

*/
void DeleteTuple(const ListExpr typeInfo, Word& w)
{
  ((Tuple *)w.addr)->DeleteIfAllowed();
}

/*

3.5 ~Check~-function of type constructor ~tuple~

Checks the specification:

----    (ident x DATA)+         -> TUPLE        tuple
----

with the additional constraint that all identifiers used (attribute
names) must be distinct. Hence a tuple type has the form:

----    (tuple
            (
                (age x)
                (name y)))
----

and ~x~ and ~y~ must be types of kind DATA. Kind TUPLE introduces 
the following error codes:

----    (... 1)      Empty tuple type
        (... 2 x)    x is not an attribute list, but an atom
        (... 3 x)    Doubly defined attribute name x
        (... 4 x)    Invalid attribute name x
        (... 5 x)    Invalid attribute definition x (x is not a pair)
        (... 6 x)    Attribute type does not belong to kind DATA
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

  if( nl->ListLength(type) == 2 && 
      nl->IsEqual(nl->First(type), "tuple", true))
  {
    attrlist = nl->Second(type);
    if (nl->IsEmpty(attrlist))
    {
      errorInfo = nl->Append(errorInfo,
        nl->ThreeElemList(
          nl->IntAtom(61), 
          nl->SymbolAtom("TUPLE"),
          nl->IntAtom(1)));
      return false;
    }
    if (nl->IsAtom(attrlist))
    {
      errorInfo = nl->Append(errorInfo,
        nl->FourElemList(
          nl->IntAtom(61), 
          nl->SymbolAtom("TUPLE"),
          nl->IntAtom(2),
          attrlist));
      return false;
    }

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
          unique = std::count(attrnamelist.begin(), 
                              attrnamelist.end(),
                              attrname);
          if (unique > 0)
          {
            errorInfo = nl->Append(errorInfo,
             nl->FourElemList(
               nl->IntAtom(61), 
               nl->SymbolAtom("TUPLE"),
               nl->IntAtom(3), 
               nl->First(pair)));
            correct = false;
          }
          attrnamelist.push_back(attrname);
          ckd = am->CheckKind("DATA", nl->Second(pair), 
                              errorInfo);
          if (!ckd)
          {
            errorInfo = nl->Append(errorInfo,
              nl->FourElemList(
                nl->IntAtom(61), 
                nl->SymbolAtom("TUPLE"),
                nl->IntAtom(6),
                nl->Second(pair)));
          }
          correct = correct && ckd;
        }
        else
        {
          errorInfo = nl->Append(errorInfo,
          nl->FourElemList(
            nl->IntAtom(61), 
            nl->SymbolAtom("TUPLE"),
            nl->IntAtom(4),
            nl->First(pair)));
          correct = false;
        }
      }
      else
      {
        errorInfo = nl->Append(errorInfo,
          nl->FourElemList(
            nl->IntAtom(61), 
            nl->SymbolAtom("TUPLE"),
            nl->IntAtom(5),
            pair));
        correct = false;
      }
    }
    return correct;
  }
  else
  {
    errorInfo = nl->Append(errorInfo,
      nl->ThreeElemList(
        nl->IntAtom(60), 
        nl->SymbolAtom("TUPLE"), 
        type));
    return false;
  }
}

/*
3.6 ~Cast~-function of type constructor ~tuple~

Casts a tuple from a stream representation of it. This function is 
used to read objects from the disk by the ~TupleManager~. Since 
tuples are not part of tuples the implementation of this function
is not necessary.

*/
void*
CastTuple(void* addr)
{
  return ( 0 );
}

/*
3.7 ~Create~-function of type constructor ~tuple~

This function is used to allocate memory sufficient for keeping one 
instance of ~tuple~.

*/
Word
CreateTuple(const ListExpr typeInfo)
{
  Tuple *tup = new Tuple( nl->Second( typeInfo ) );
  return (SetWord(tup));
}

/*
3.8 ~Close~-function of type constructor ~tuple~

This function is used to destroy the memory allocated by a ~tuple~.

*/
void CloseTuple(const ListExpr typeInfo, Word& w)
{
  ((Tuple *)w.addr)->DeleteIfAllowed();
}

/*
3.9 ~Clone~-function of type constructor ~tuple~

This function creates a cloned tuple.

*/
Word
CloneTuple(const ListExpr typeInfo, const Word& w)
{
  return SetWord( ((Tuple *)w.addr)->Clone() );
}

/*
3.10 ~Sizeof~-function of type constructor ~tuple~

Returns the size of a tuple's root record to be stored on the disk 
as a stream. Since tuples are not part of tuples, the implementation
of this function is not necessary.

*/
int
SizeOfTuple()
{
  return 0;
}

/*
3.12 Definition of type constructor ~tuple~

Eventually a type constructor is created by defining an instance of
class ~TypeConstructor~. Constructor's arguments are the type 
constructor's name and the eleven functions previously defined.

*/
TypeConstructor cpptuple( "tuple",         TupleProp,
                          OutTuple,        InTuple,
                          SaveToListTuple, RestoreFromListTuple,
                          CreateTuple,     DeleteTuple,
                          0,               0,
                          CloseTuple,      CloneTuple,
                          CastTuple,       SizeOfTuple,
                          CheckTuple );
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

  return nl->TwoElemList(
           nl->FourElemList(
             nl->StringAtom("Signature"),
             nl->StringAtom("Example Type List"),
             nl->StringAtom("List Rep"),
             nl->StringAtom("Example List")),
           nl->FourElemList(
             nl->StringAtom("TUPLE -> REL"),
             nl->StringAtom("(rel(tuple((name string)(age int))))"),
             listreplist,
             examplelist));
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

The function is used to allocate memory sufficient for keeping one 
instance of ~rel~.

*/
Word
CreateRel(const ListExpr typeInfo)
{
  Relation* rel = new Relation( typeInfo );
  return (SetWord(rel));
}

/*
4.4 ~In~-function of type constructor ~rel~

~value~ is the list representation of the relation. The structure 
of ~typeInfo~ and ~value~ are described above. Error handling in 
~InRel~:

The result relation will contain all tuples that have been converted
correctly (have correct list expressions). For all other tuples, an 
error message containing the position of the tuple within this 
relation (list) is added to ~errorInfo~. (This is done by 
procedure ~InTuple~ called by ~InRel~). If any tuple representation 
is wrong, then ~InRel~ will return ~correct~ as FALSE and will 
itself add an error message of the form

----    (InRel <errorPos>)
----

to ~errorInfo~. The value in ~errorPos~ has to be passed from the 
environment; probably it is the position of the relation object in 
the list of database objects.

*/
Word
InRel(ListExpr typeInfo, ListExpr value,
      int errorPos, ListExpr& errorInfo, bool& correct)
{
  return SetWord( Relation::In( typeInfo, value, errorPos, 
                                errorInfo, correct ) );
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
  return 
    SetWord( Relation::RestoreFromList( typeInfo, value, errorPos, 
                                        errorInfo, correct ) );
}

/*
4.5 ~Delete~-function of type constructor ~rel~

*/
void DeleteRel(const ListExpr typeInfo, Word& w)
{
  ((Relation *)w.addr)->Delete();
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
  if ((nl->ListLength(type) == 2) && 
      nl->IsEqual(nl->First(type), "rel"))
  {
    return am->CheckKind("TUPLE", nl->Second(type), errorInfo);
  }
  else
  {
    errorInfo = nl->Append(errorInfo,
      nl->ThreeElemList(
        nl->IntAtom(60), 
        nl->SymbolAtom("REL"), 
        type));
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
In this case we will implement one function that does nothing, 
called ~CloseRel~ and another which the cache will execute, called
~CacheCloseRel~.

*/
void CloseRel(const ListExpr typeInfo, Word& w)
{
  return ((Relation *)w.addr)->Close();
}

/*
4.9 ~Open~-function of type constructor ~rel~

This is a slightly modified version of the function ~DefaultOpen~ 
(from ~Algebra~) which creates the relation from the SmiRecord only 
if it does not yet exist.

The idea is to maintain a cache containing the relation 
representations that have been built in memory. The cache basically 
stores pairs (~recordId~, ~relation\_value~). If the ~recordId~ 
passed to this function is found, the cached relation value is 
returned instead of building a new one.

*/
bool
OpenRel( SmiRecord& valueRecord,
         size_t& offset,
         const ListExpr typeInfo,
         Word& value )
{
  value = SetWord( Relation::Open( valueRecord, offset, typeInfo ) );
  return value.addr != 0;
}

/*
4.10 ~Save~-function of type constructor ~rel~

*/
bool
SaveRel( SmiRecord& valueRecord,
         size_t& offset, 
         const ListExpr typeInfo,
         Word& value )
{
  return 
    ((Relation *)value.addr)->Save( valueRecord, offset, typeInfo );
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
CloneRel(const ListExpr typeInfo, const Word& w)
{
  return SetWord( ((Relation*)w.addr)->Clone() );
}

/*

4.14 Definition of type constructor ~rel~

Eventually a type constructor is created by defining an instance of
class ~TypeConstructor~. Constructor's arguments are the type 
constructor's name and the eleven functions previously defined.

*/
TypeConstructor cpprel( "rel",           RelProp,
                        OutRel,          InRel,
                        SaveToListRel,   RestoreFromListRel,
                        CreateRel,       DeleteRel,
                        OpenRel,         SaveRel,
                        CloseRel,        CloneRel,
                        CastRel,         SizeOfRel,
                        CheckRel );

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

Type operators are used only for inferring argument types of 
parameter functions. They have a type mapping but no evaluation 
function.

5.3.1 Type mapping function of operator ~TUPLE~

Extract tuple type from a stream or relation type given as the 
first argument.

----    ((stream x) ...)                -> x
        ((rel x)    ...)                -> x
----

*/
ListExpr TUPLETypeMap(ListExpr args)
{
  ListExpr first;
  string argstr;

  CHECK_COND(!nl->IsAtom(args),
  "Type operator TUPLE expects a list and not an atom.");
    
  first = nl->First(args);
  nl->WriteToString(argstr, first);
  CHECK_COND( 
    nl->ListLength(first) == 2 &&
    (TypeOfRelAlgSymbol(nl->First(first)) == rel ||
     TypeOfRelAlgSymbol(nl->First(first)) == stream) &&
    nl->ListLength(nl->Second(first)) == 2 &&
    TypeOfRelAlgSymbol(nl->First(nl->Second(first))) == tuple, 
    "Type operator TUPLE expects as argument a list with "
    "structure, (rel(tuple((a1 t1)...(an tn)))) or "
    "(stream(tuple((a1 t1)...(an tn)))).\n"
    "Type operator TUPLE gets an argument '" + argstr + "'." );

  return nl->Second(first);
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
         TypeOperatorSelect,   // trivial selection function
         TUPLETypeMap          // type mapping
);
/*

5.4 Type Operator ~TUPLE2~

5.4.1 Type mapping function of operator ~TUPLE2~

Extract tuple type from a stream or relation type given as the 
second argument.

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
const string TUPLE2Spec  = 
  "( ( \"Signature\" \"Syntax\" \"Meaning\" "
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
         TypeOperatorSelect,   // trivial selection function
         TUPLE2TypeMap         // type mapping
);

/*

5.5 Operator ~feed~

Produces a stream from a relation by scanning the relation tuple by
tuple.

5.5.1 Type mapping function of operator ~feed~

A type mapping function takes a nested list as argument. Its 
contents are type descriptions of an operator's input parameters. 
A nested list describing the output type of the operator is returned.

Result type of feed operation.

----    ((rel x))  -> (stream x)
----

*/
ListExpr FeedTypeMap(ListExpr args)
{
  ListExpr first;
  string argstr;

  CHECK_COND(nl->ListLength(args) == 1,
    "Operator feed expects a list of length one.");
    
  first = nl->First(args);
  nl->WriteToString(argstr, first);
  CHECK_COND( 
    nl->ListLength(first) == 2 &&
    TypeOfRelAlgSymbol(nl->First(first)) == rel &&
    (!(nl->IsAtom(nl->Second(first)) ||
    nl->IsEmpty(nl->Second(first))) &&    
    (TypeOfRelAlgSymbol(nl->First(nl->Second(first))) == tuple)),
  "Operator feed expects an argument of type relation, "
  "(rel(tuple((a1 t1)...(an tn)))).\n"
  "Operator feed gets an argument of type '" + argstr + "'."
  " Relation name not known in the database ?");
     
  return nl->Cons(nl->SymbolAtom("stream"), nl->Rest(first));
}
/*

5.5.2 Value mapping function of operator ~feed~

*/

#define USE_PROGRESS

//comment out to not use progress


#ifndef USE_PROGRESS

// standard version of code

int
Feed(Word* args, Word& result, int message, Word& local, Supplier s)
{
  GenericRelation* r;
  GenericRelationIterator* rit;

  switch (message)
  {
    case OPEN :
      r = (GenericRelation*)args[0].addr;
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


#else

// version with support for progress queries



struct FeedLocalInfo
{
  int current;        	//current number of tuples processed
  int total;          	//total number of tuples in argument relation
  double tuplesize;	//total tuplesize
  double tuplesizeCE;	//size of core and extension part of tuple
  GenericRelationIterator* rit;
};


int
Feed(Word* args, Word& result, int message, Word& local, Supplier s)
{
  GenericRelation* r;
  FeedLocalInfo* fli;


  switch (message)
  {
    case OPEN :
      r = (GenericRelation*)args[0].addr;
      fli = new FeedLocalInfo;
        fli->current = 0;
        fli->total = r->GetNoTuples();
        fli->tuplesize =  r->GetTotalSize() / (fli->total + 1);
        fli->tuplesizeCE =  r->GetTotalExtSize() / (fli->total + 1);
        fli->rit = r->MakeScan();
      local = SetWord(fli);
      return 0;

    case REQUEST :
      fli = (FeedLocalInfo*) local.addr;
      Tuple *t;
      if ((t = fli->rit->GetNextTuple()) != 0)
      {
        fli->current++;
        result = SetWord(t);
        return YIELD;
      }
      else
      {
        return CANCEL;
      }

    case CLOSE :
      fli = (FeedLocalInfo*) local.addr;
      delete fli->rit;
      return 0;

    case PROGRESS :

      ProgressInfo p1;
      ProgressInfo *pRes;
      const double uFeed = 1100.0 / (412670.0 * 80.0);    //seconds per byte
      Supplier sonOfFeed;

     		//cout << "feed was asked for progress" << endl;
 
      pRes = (ProgressInfo*) result.addr;
      fli = (FeedLocalInfo*) local.addr;
      sonOfFeed = qp->GetSupplierSon(s, 0);

      if ( qp->IsObjectNode(sonOfFeed) )
      {
        if ( fli )
        {
          pRes->Card = (double) fli->total;
          pRes->Size = fli->tuplesize;
          pRes->SizeCE = fli->tuplesizeCE;
          pRes->Time = (fli->total + 1) * fli->tuplesizeCE * uFeed; 
 
		//any time value created must be > 0; so we add 1 to the total

          pRes->Progress = (fli->current * fli->tuplesizeCE * uFeed) 
            / pRes->Time;
          return YIELD;
	}
	else return CANCEL;
      }
      else  	//son is an operator, most likely consume
      {

	if ( qp->RequestProgress(sonOfFeed, &p1) )
        {
	  if ( fli == 0 )		//son evaluation not yet finished
          {
            pRes->Card = p1.Card;
            pRes->Size = p1.Size;
            pRes->SizeCE = p1.SizeCE;
            pRes->Time = p1.Time + p1.Card * p1.SizeCE * uFeed;
	    pRes->Progress = (p1.Progress * p1.Time) / pRes->Time;
            return YIELD;
          }
          else				//son evaluation is complete
          {       
            pRes->Card = (double) fli->total;
            pRes->Size = fli->tuplesize;
            pRes->SizeCE = fli->tuplesizeCE;
            pRes->Time = p1.Time + fli->total * fli->tuplesizeCE * uFeed;
            pRes->Progress = 
              (p1.Time + fli->current * fli->tuplesizeCE * uFeed) / pRes->Time;
            return YIELD;
          }
        }
        else return CANCEL;
      }
                 
  }
  return 0;
}

#endif



/*

5.5.3 Specification of operator ~feed~

*/
const string FeedSpec  = 
  "( ( \"Signature\" \"Syntax\" \"Meaning\" "
  "\"Example\" ) "
  "( <text>(rel x) -> (stream x)</text--->"
  "<text>_ feed</text--->"
  "<text>Produces a stream from a relation by "
  "scanning the relation tuple by tuple.</text--->"
  "<text>query cities feed consume</text--->"
  ") )";

/*

5.5.4 Definition of operator ~feed~

Non-overloaded operators are defined by constructing a new instance 
of class ~Operator~, passing all operator functions as constructor
arguments.

*/
Operator relalgfeed (
          "feed",                 // name
          FeedSpec,               // specification
          Feed,                   // value mapping
          Operator::SimpleSelect, // trivial selection function
          FeedTypeMap             // type mapping
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
  string argstr;
  
  CHECK_COND(nl->ListLength(args) == 1,
  "Operator consume expects a list of length one.");
  
  first = nl->First(args);
  nl->WriteToString(argstr, first);
  CHECK_COND(
    ((nl->ListLength(first) == 2) &&
    (TypeOfRelAlgSymbol(nl->First(first)) == stream)) &&
    (!(nl->IsAtom(nl->Second(first)) ||
       nl->IsEmpty(nl->Second(first))) &&    
    (TypeOfRelAlgSymbol(nl->First(nl->Second(first))) == tuple)),
  "Operator consume expects an argument of type (stream(tuple"
  "((a1 t1)...(an tn)))).\n"
  "Operator consume gets an argument of type '" + argstr + "'.");
  
  return nl->Cons(nl->SymbolAtom("rel"), nl->Rest(first));
}

/*
5.6.2 Value mapping function of operator ~consume~

*/

#ifndef USE_PROGRESS

// standard version

int
Consume(Word* args, Word& result, int message, 
        Word& local, Supplier s)
{
  Word actual;

  GenericRelation* rel = (Relation*)((qp->ResultStorage(s)).addr);
  if(rel->GetNoTuples() > 0)
  {
    rel->Clear();
  }

  qp->Open(args[0].addr);
  qp->Request(args[0].addr, actual);
  while (qp->Received(args[0].addr))
  {
    Tuple* tuple = (Tuple*)actual.addr;
    rel->AppendTuple(tuple);
    tuple->DeleteIfAllowed();
    qp->Request(args[0].addr, actual);
  }
  result = SetWord(rel);
  
  qp->Close(args[0].addr);
  return 0;
}

#else 

// Version with support for progress queries

struct consumeLocalInfo
{
  int state; 		//0 = working, 1 = finished
  int current;		//current no of tuples read
};


int
Consume(Word* args, Word& result, int message,
       Word& local, Supplier s)
{
  Word actual;
  consumeLocalInfo* cli;

  if (message != PROGRESS)     //normal evaluation
  {

    cli = new consumeLocalInfo;
      cli->state = 0;
      cli->current = 0;
    local = SetWord(cli);

    GenericRelation* rel = (Relation*)((qp->ResultStorage(s)).addr);
    if(rel->GetNoTuples() > 0)
    {
      rel->Clear();
    }

    qp->Open(args[0].addr);
    qp->Request(args[0].addr, actual);
    while (qp->Received(args[0].addr))
    {
      Tuple* tuple = (Tuple*)actual.addr;
      rel->AppendTuple(tuple);
      tuple->DeleteIfAllowed();
      cli->current++;
      qp->Request(args[0].addr, actual);
    }
    result = SetWord(rel);

    qp->Close(args[0].addr);
    cli->state = 1;
    return 0;
  }

  else //message == PROGRESS
  {

    	//cout << "consume was asked for progress" << endl;

    ProgressInfo p1;
    ProgressInfo* pRes;
    const double uConsume = 28000.0 / (412670.0 * 80.0); //milliseconds per byte

    cli = (consumeLocalInfo*) local.addr;
    pRes = (ProgressInfo*) result.addr;    


    if ( qp->RequestProgress(args[0].addr, &p1) )
    {
      pRes->Card = p1.Card;
      pRes->Size = p1.Size;
      pRes->SizeCE = p1.SizeCE;
      pRes->Time = p1.Time + p1.Card * p1.Size * uConsume;

      if ( cli == 0 )		//not yet working
      {
        pRes->Progress = (p1.Progress * p1.Time) / pRes->Time;
      }
      else 
        if ( cli->state == 0 )	//working
        {
          pRes->Progress = 
            (p1.Progress * p1.Time + cli->current * p1.Size * uConsume ) 
            / pRes->Time;
        }
        else 			//finished
        {
          pRes->Progress = 1.0;
        }
  
      return YIELD;			//successful
    }
    else return CANCEL;			//no progress available
  }
}

#endif


/*
5.6.3 Specification of operator ~consume~

*/
const string ConsumeSpec  = 
  "( ( \"Signature\" \"Syntax\" \"Meaning\" "
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
   "consume",              // name
   ConsumeSpec,            // specification
   Consume,                // value mapping
   Operator::SimpleSelect, // trivial selection function
   ConsumeTypeMap          // type mapping
);

/*
5.7 Operator ~attr~

5.7.1 Type mapping function of operator ~attr~

Result type attr operation.

----
    ((tuple ((x1 t1)...(xn tn))) xi)    -> ti
                            APPEND (i) ti)
----

This type mapping uses a special feature of the query processor, 
in that if requests to append a further argument to the given list 
of arguments, namely, the index of the attribute within the tuple. 
This index is computed within the type mapping function. The 
request is given through the result expression of the type mapping 
which has the form, for example,

---- (APPEND (1) string)
----

The keyword ~APPEND~ occuring as a first element of a returned type 
expression tells the query processor to add the elements of the 
following list - the second element of the type expression - as 
further arguments to the operator (as if they had been written in 
the query). The third element  of the query is then used as the 
real result type. In this case 1 is the index of the attribute 
determined in this procedure. The query processor, more precisely
the procedure ~anotate~ there, will produce the annotation for the 
constant 1, append it to the list of annotated arguments, and then 
use "string" as the result type of the ~attr~ operation.

*/
ListExpr AttrTypeMap(ListExpr args)
{
  ListExpr first, second, attrtype;
  string  attrname, argstr;
  int j;

  nl->WriteToString(argstr, args); 
  CHECK_COND(nl->ListLength(args) == 2,
  "Operator attr expects a list of length two.");
  
  first = nl->First(args);  
  nl->WriteToString(argstr, first);  
  CHECK_COND( (nl->ListLength(first) == 2) &&
              (TypeOfRelAlgSymbol(nl->First(first)) == tuple),
  "Operator attr expects as first argument a list with structure " 
  "(tuple ((a1 t1)...(an tn)))\n"
  "Operator attr gets a list with structure '" + argstr + "'.");
  
  second  = nl->Second(args); 
  nl->WriteToString(argstr, second);   
  CHECK_COND( nl->IsAtom(second) && 
              nl->AtomType(second) == SymbolType,
  "Operator attr expects as second argument a symbol atom "
  "(attributename).\n" 
  "Operator attr gets '" + argstr + "'.");

  attrname = nl->SymbolValue(second);
  j = FindAttribute(nl->Second(first), attrname, attrtype);
  if (j)
    return nl->ThreeElemList(nl->SymbolAtom("APPEND"),
           nl->OneElemList(nl->IntAtom(j)), attrtype);
  else 
  {
    nl->WriteToString( argstr, nl->Second(first) ); 
    ErrorReporter::ReportError(
    "Attribute name '" + attrname + "' is not known in the tuple.\n"
    "Known Attribute(s): " + argstr);
    
    return nl->SymbolAtom("typeerror");
  }   
}
/*
5.7.2 Value mapping function of operator ~attr~

The argument vector ~arg~ contains in the first slot ~args[0]~ the 
tuple and in ~args[2]~ the position of the attribute as a number. 
Returns as ~result~ the value of an attribute at the given 
position ~args[2]~ in a tuple object. The attribute name is 
argument 2 in the query and is used in the function 
~AttributeTypeMap~ to determine the attribute number ~args[2]~ .

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
const string AttrSpec  = 
  "( ( \"Signature\" \"Syntax\" \"Meaning\" "
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
     "attr",                 // name
     AttrSpec,               // specification
     Attr,                   // value mapping
     Operator::SimpleSelect, // trivial selection function
     AttrTypeMap             // type mapping
);

/*
5.8 Operator ~filter~

Only tuples, fulfilling a certain condition are passed on to the 
output stream.

5.8.1 Type mapping function of operator ~filter~

Result type of filter operation.

----    ((stream (tuple x)) (map (tuple x) bool))
               -> (stream (tuple x))
----

*/
ListExpr FilterTypeMap(ListExpr args)
{
  ListExpr first, second;
  string argstr;
  
  CHECK_COND(nl->ListLength(args) == 2,
  "Operator filter expects a list of length two.");
  
  first = nl->First(args);
  second  = nl->Second(args);
    
  nl->WriteToString(argstr, first);    
  CHECK_COND(
    nl->ListLength(first) == 2 &&
    TypeOfRelAlgSymbol(nl->First(first)) == stream &&
    nl->ListLength(nl->Second(first)) == 2 &&
    TypeOfRelAlgSymbol(nl->First(nl->Second(first))) == tuple,
    "Operator filter expects as first argument a list with "
    "structure (stream (tuple ((a1 t1)...(an tn))))\n"
    "Operator filter gets a list with structure '" + argstr + "'.");

  nl->WriteToString(argstr, second);    
  CHECK_COND(nl->ListLength(second) == 3 &&
             TypeOfRelAlgSymbol(nl->First(second)) == ccmap &&
             TypeOfRelAlgSymbol(nl->Third(second)) == ccbool,
    "Operator filter expects a list with structure " 
    "(map (tuple ((a1 t1)...(an tn))) bool)\n"
    "Operator filter gets a list with structure '" + argstr + "'.");
    
  CHECK_COND(nl->Equal(nl->Second(first),nl->Second(second)),
    "Tuple type in stream is not equal to tuple type "
    "in the function.");

  return first;
}

/*
5.8.2 Value mapping function of operator ~filter~

*/


#ifndef USE_PROGRESS

// standard version

int
Filter(Word* args, Word& result, int message, 
       Word& local, Supplier s)
{
  bool found = false;
  Word elem, funresult;
  ArgVectorPointer funargs;
  Tuple* tuple = 0;

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

#else

// progress version

struct FilterLocalInfo
{
  int current;		//tuples read
  int returned;		//tuples returned
};


int
Filter(Word* args, Word& result, int message, 
       Word& local, Supplier s)
{
  bool found = false;
  Word elem, funresult;
  ArgVectorPointer funargs;
  Tuple* tuple = 0;
  FilterLocalInfo* fli;

  switch ( message )
  {

    case OPEN:

      fli = new FilterLocalInfo;
        fli->current = 0;
        fli->returned = 0;
      local = SetWord(fli);

      qp->Open (args[0].addr);
      return 0;

    case REQUEST:

      fli = (FilterLocalInfo*) local.addr;

      funargs = qp->Argument(args[1].addr);
      qp->Request(args[0].addr, elem);
      found = false;
      while (qp->Received(args[0].addr) && !found)
      {
        fli->current++;
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
        fli->returned++;
        result = SetWord(tuple);
        return YIELD;
      }
      else
        return CANCEL;

    case CLOSE:

      qp->Close(args[0].addr);
      return 0;

    case PROGRESS:

      ProgressInfo p1;
      ProgressInfo* pRes;
      double uFilter = 600.0 /  (412670.0 * 0.1);	//to be measured

      pRes = (ProgressInfo*) result.addr;
      fli = (FilterLocalInfo*) local.addr;

      if ( qp->RequestProgress(args[0].addr, &p1) )
      {
        pRes->Size = p1.Size;
        pRes->SizeCE = p1.SizeCE;
        pRes->Time = p1.Time + p1.Card * qp->GetPredCost(s) * uFilter;

        if ( fli )		//filter was started
        {
          pRes->Card = (double) p1.Card * 
            ( (double) fli->returned / (double) (fli->current + 1)); //is > 0 
          pRes->Progress = (p1.Progress * p1.Time 
            + fli->current * qp->GetPredCost(s) * uFilter) / pRes->Time;
        }
        else			//filter not yet started
        {
	  pRes->Card = p1.Card * qp->GetSelectivity(s);
          pRes->Progress = (p1.Progress * p1.Time) / pRes->Time;
        }
        return YIELD;
      }
      else return CANCEL;
  }
  return 0;
}

#endif


/*
5.8.3 Specification of operator ~filter~

*/
const string FilterSpec  = 
  "( ( \"Signature\" \"Syntax\" \"Meaning\" "
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
         "filter",               // name
         FilterSpec,             // specification
         Filter,                 // value mapping
         Operator::SimpleSelect, // trivial selection function
         FilterTypeMap           // type mapping
);


/*
5.8 Operator ~reduce~

A fraction of tuples, fulfilling a certain condition are removed from the output stream.

5.8.1 Type mapping function of operator ~reduce~

Result type of filter operation.

----    ((stream (tuple x)) (map (tuple x) bool) int)
               -> (stream (tuple x))
----

5.12.0 Specification 

*/

struct ReduceInfo : OperatorInfo {
 
  ReduceInfo() : OperatorInfo()
  { 
    name =      "reduce";
    signature = "stream(tuple(y)) x (stream(tuple(y)) -> bool) x int \n"
                "-> stream(tuple(y))";
    syntax =    "_ reduce[ f, n ]";
    meaning =   "Passes all tuples t with f(t)=FALSE to the output stream "
                "but only every k-th tuple [k=max(n,1)] which fulfills f.";
    example =   "plz feed reduce[.PLZ > 50000, 2] count";
  }

};



ListExpr reduce_tm(ListExpr args)
{
  string argstr="";
  
  CHECK_COND(nl->ListLength(args) == 3,
  "Operator filter expects a list of length three.");
  
  ListExpr first = nl->First(args);
  ListExpr second  = nl->Second(args);
  ListExpr third  = nl->Third(args);
    
  nl->WriteToString(argstr, first);    
  CHECK_COND(
    nl->ListLength(first) == 2 &&
    TypeOfRelAlgSymbol(nl->First(first)) == stream &&
    nl->ListLength(nl->Second(first)) == 2 &&
    TypeOfRelAlgSymbol(nl->First(nl->Second(first))) == tuple,
    "Operator filter expects as first argument a list with "
    "structure (stream (tuple ((a1 t1)...(an tn))))\n"
    "Operator filter gets a list with structure '" + argstr + "'.");

  nl->WriteToString(argstr, second);    
  CHECK_COND(nl->ListLength(second) == 3 &&
             TypeOfRelAlgSymbol(nl->First(second)) == ccmap &&
             TypeOfRelAlgSymbol(nl->Third(second)) == ccbool,
    "Operator filter expects a list with structure " 
    "(map (tuple ((a1 t1)...(an tn))) bool)\n"
    "Operator filter gets a list with structure '" + argstr + "'.");
    
  CHECK_COND(nl->Equal(nl->Second(first),nl->Second(second)),
    "Tuple type in stream is not equal to tuple type "
    "in the function.");

  CHECK_COND( (nl->AtomType(third) == SymbolType) 
               && nl->SymbolValue(third) == "int",
               "Third argument is not of type int." );             
  
  return first;
}

/*
5.8.2 Value mapping function of operator ~filter~

*/
int
reduce_vm( Word* args, Word& result, int message, 
           Word& local, Supplier s)
{
  // args[0] : stream(tuple(y))
  // args[1] : stream(tuple(y)) -> bool
  // args[2] : int
 
  struct ReduceInfo {

     int m;
     int n;
     
     ReduceInfo(const int ctr) : m( max(ctr-1,0) ), n(m) {}
     ~ReduceInfo(){}

     bool passOver()
     {
       if (n == 0)
       { 
         n = m;
         return true;
       }  
       else
       { 
         n--;
         return false;
       }  
     } 
  }; 
  
  ReduceInfo* ri = static_cast<ReduceInfo*>( local.addr);
  
  switch ( message )
  {

    case OPEN:

      qp->Open (args[0].addr);
      local.addr = new ReduceInfo(StdTypes::GetInt(args[2]));
      return 0;

    case REQUEST: {

      Tuple* tuple = 0;
      bool ok = false;
      while ( true )
      {
        Word elem, funresult;
        tuple = 0;
        qp->Request(args[0].addr, elem);
        ok = qp->Received(args[0].addr);

        if ( !ok ) // no more tuple in the stream 
          break;
        
        // apply the parameter function   
        ArgVectorPointer funargs;
        funargs = qp->Argument(args[1].addr);
        tuple = (Tuple*)elem.addr;
        (*funargs)[0] = elem;
        qp->Request(args[1].addr, funresult);

        bool found=false;
        if (((StandardAttribute*)funresult.addr)->IsDefined())
        {
          found = ((CcBool*)funresult.addr)->GetBoolval();
        }
        
        // check if the tuple needs to be passed over
        if (found) 
        {
          if ( !ri->passOver() )
            tuple->DeleteIfAllowed();
          else
            break;
        }        
        else
        {
           break;
        } 
      }
      
      if (tuple)
      {
        result.addr = tuple;
        return YIELD;
      }
      else
      { 
        return CANCEL;
      }  
    }
      
    case CLOSE:

      qp->Close(args[0].addr);
      delete ri;
      return 0;
  }
  return 0;
}




/*
5.9 Operator ~project~

5.9.1 Type mapping function of operator ~filter~

Result type of project operation.

----  ((stream (tuple ((x1 T1) ... (xn Tn)))) (ai1 ... aik))  ->
        (APPEND
          (k (i1 ... ik))
          (stream (tuple ((ai1 Ti1) ... (aik Tik))))
        )
----

The type mapping computes the number of attributes and the list of 
attribute numbers for the given projection attributes and asks the 
query processor to append it to the given arguments.

*/
ListExpr ProjectTypeMap(ListExpr args)
{
  bool firstcall = true;
  int noAttrs=0, j=0;
  
  // initialize local ListExpr variables
  ListExpr first=nl->TheEmptyList();
  ListExpr second=first, first2=first, 
           attrtype=first, newAttrList=first;
  ListExpr lastNewAttrList=first, lastNumberList=first, 
           numberList=first, outlist=first;
  string attrname="", argstr="";
  
  CHECK_COND(nl->ListLength(args) == 2,
    "Operator project expects a list of length two.");
  
  first = nl->First(args);
  second = nl->Second(args);
    
  nl->WriteToString(argstr, first);  
  CHECK_COND(
    nl->ListLength(first) == 2 &&
    TypeOfRelAlgSymbol(nl->First(first)) == stream &&
    nl->ListLength(nl->Second(first)) == 2 &&
    TypeOfRelAlgSymbol(nl->First(nl->Second(first))) == tuple,
    "Operator project expects a list with structure " 
    "(stream (tuple ((a1 t1)...(an tn))))\n"
    "Operator project gets a list with structure '" + argstr + "'.");
    
  nl->WriteToString(argstr, second);    
  CHECK_COND(
    nl->ListLength(second) > 0 &&
    !nl->IsAtom(second),
    "Operator project expects a list with attributenames " 
    "(ai...aj)\n"
    "Operator project gets a list '" + argstr + "'.");
 
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
      ErrorReporter::ReportError(
        "Attributename in the list is not of symbol type.");
      return nl->SymbolAtom("typeerror");
    }
    j = FindAttribute(nl->Second(nl->Second(first)), 
                      attrname, attrtype);
    if (j)
    {
      if (firstcall)
      {
        firstcall = false;
        newAttrList = 
          nl->OneElemList(nl->TwoElemList(first2, attrtype));
        lastNewAttrList = newAttrList;
        numberList = nl->OneElemList(nl->IntAtom(j));
        lastNumberList = numberList;
      }
      else
      {
        lastNewAttrList =
          nl->Append(lastNewAttrList, 
                     nl->TwoElemList(first2, attrtype));
        lastNumberList =
          nl->Append(lastNumberList, nl->IntAtom(j));
      }
    }
    else
    {
      ErrorReporter::ReportError(
        "Operator project: Attributename '" + attrname + 
        "' is not a known attributename in the tuple stream.");
          return nl->SymbolAtom("typeerror");
    }
  }
  // Check whether all new attribute names are distinct
  // - not yet implemented
  outlist = 
    nl->ThreeElemList(
      nl->SymbolAtom("APPEND"),
      nl->TwoElemList(
        nl->IntAtom(noAttrs), 
        numberList),
      nl->TwoElemList(
        nl->SymbolAtom("stream"),
        nl->TwoElemList(
          nl->SymbolAtom("tuple"), 
          newAttrList)));
  return outlist;
}

/*
5.9.2 Value mapping function of operator ~project~

*/
int
Project(Word* args, Word& result, int message, 
        Word& local, Supplier s)
{
  switch (message)
  {
    case OPEN :
    {
      ListExpr resultType = GetTupleResultType( s );
      TupleType *tupleType = new TupleType(nl->Second(resultType));
      local.addr = tupleType;

      qp->Open(args[0].addr);
      return 0;
    }
    case REQUEST :
    {
      Word elem1, elem2;
      int noOfAttrs, index;
      Supplier son;

      qp->Request(args[0].addr, elem1);
      if (qp->Received(args[0].addr))
      {
        TupleType *tupleType = (TupleType *)local.addr;
        Tuple *t = new Tuple( tupleType );

        noOfAttrs = ((CcInt*)args[2].addr)->GetIntval();
        assert( t->GetNoAttributes() == noOfAttrs );

        for( int i = 0; i < noOfAttrs; i++)
        {
          son = qp->GetSupplier(args[3].addr, i);
          qp->Request(son, elem2);
          index = ((CcInt*)elem2.addr)->GetIntval();
          t->CopyAttribute(index-1, (Tuple*)elem1.addr, i);
        }
        ((Tuple*)elem1.addr)->DeleteIfAllowed();
        result = SetWord(t);
        return YIELD;
      }
      else return CANCEL;
    }
    case CLOSE :
    {
      ((TupleType *)local.addr)->DeleteIfAllowed();
      qp->Close(args[0].addr);
      return 0;
    }
  }
  return 0;
}

/*
5.9.3 Specification of operator ~project~

*/
const string ProjectSpec  = 
  "( ( \"Signature\" \"Syntax\" \"Meaning\" "
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
         "project",              // name
         ProjectSpec,            // specification
         Project,                // value mapping
         Operator::SimpleSelect, // trivial selection function
         ProjectTypeMap          // type mapping
);

/*
5.10 Operator ~product~

5.10.1 Type mapping function of operator ~product~

Result type of product operation.

----  ((stream (tuple (x1 ... xn))) (stream (tuple (y1 ... ym))))
        -> (stream (tuple (x1 ... xn y1 ... ym)))
----

*/
ListExpr ProductTypeMap(ListExpr args)
{
  ListExpr first, second, list, list1, list2, outlist;
  string argstr;
  
  CHECK_COND(nl->ListLength(args) == 2,
    "Operator product expects a list of length two.");

  first = nl->First(args); second = nl->Second(args);

  nl->WriteToString(argstr, first);  
  CHECK_COND(nl->ListLength(first) == 2 &&
    TypeOfRelAlgSymbol(nl->First(first)) == stream &&
    nl->ListLength(nl->Second(first)) == 2 &&
    TypeOfRelAlgSymbol(nl->First(nl->Second(first))) == tuple,
    "Operator product expects a first list with structure " 
    "(stream (tuple ((a1 t1)...(an tn))))\n"
    "Operator product gets a first list with structure '" + 
    argstr + "'.");
    
  list1 = nl->Second(nl->Second(first));

  nl->WriteToString(argstr, second);  
  CHECK_COND(nl->ListLength(second) == 2 &&
    TypeOfRelAlgSymbol(nl->First(second)) == stream &&
    nl->ListLength(nl->Second(second)) == 2 &&
    TypeOfRelAlgSymbol(nl->First(nl->Second(second))) == tuple,
    "Operator product expects a second list with structure " 
    "(stream (tuple ((a1 t1)...(an tn))))\n"
    "Operator product gets a second list with structure '" + 
    argstr + "'.");

  list2 = nl->Second(nl->Second(second));
  list = ConcatLists(list1, list2);
    // Check whether all new attribute names are distinct
    // - not yet implemented
  if ( CompareNames(list) )
  {
    outlist = nl->TwoElemList(nl->SymbolAtom("stream"),
    nl->TwoElemList(nl->SymbolAtom("tuple"), list));
    return outlist;
  }
  else
  {
    ErrorReporter::ReportError("Operator product: found doubly "
      "defined attributenames in concatenated list.\n");
    return nl->SymbolAtom("typeerror");
  }
}

/*
5.10.2 Value mapping function of operator ~product~

*/
class NoOrder;

struct ProductLocalInfo
{
  TupleType *resultTupleType;
  Tuple* currentTuple;
  TupleBuffer *rightRel;
  TupleBufferIterator *iter;
};

int
Product(Word* args, Word& result, int message, 
        Word& local, Supplier s)
{
  Word r, u;
  ProductLocalInfo* pli;

  switch (message)
  {
    case OPEN :
    {

      long MAX_MEMORY = qp->MemoryAvailableForOperator();
      cmsg.info("RA:ShowMemInfo") 
        << "Product.MAX_MEMORY (" 
        << MAX_MEMORY/1024 << " MB): " << endl;
      cmsg.send();

      qp->Open(args[0].addr);
      qp->Request(args[0].addr, r);
      pli = new ProductLocalInfo;
      pli->currentTuple = 
        qp->Received(args[0].addr) ? (Tuple*)r.addr : 0;

      /* materialize right stream */
      qp->Open(args[1].addr);
      qp->Request(args[1].addr, u);

      if(qp->Received(args[1].addr))
      {
        pli->rightRel = new TupleBuffer( MAX_MEMORY );
      }
      else
      {
        pli->rightRel = 0;
      }

      while(qp->Received(args[1].addr))
      {
        Tuple *t = (Tuple*)u.addr;
        pli->rightRel->AppendTuple( t );
        t->DeleteIfAllowed();
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
      pli->resultTupleType = new TupleType(nl->Second(resultType));

      local = SetWord(pli);
      return 0;
    }
    case REQUEST :
    {
      Tuple *resultTuple, *rightTuple;
      pli = (ProductLocalInfo*)local.addr;

      if (pli->currentTuple == 0)
      {
        return CANCEL;
      }
      else
      {
        if( pli->rightRel == 0 ) // second stream is empty
        {
          return CANCEL;
        }
        else if( (rightTuple = pli->iter->GetNextTuple()) != 0 )
        {
          resultTuple = new Tuple( pli->resultTupleType );
          Concat(pli->currentTuple, rightTuple, resultTuple);
          rightTuple->DeleteIfAllowed();
          result = SetWord(resultTuple);
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
            resultTuple = new Tuple( pli->resultTupleType );
            Concat(pli->currentTuple, rightTuple, resultTuple);
            rightTuple->DeleteIfAllowed();
            result = SetWord(resultTuple);
            return YIELD;
          }
          else
          {
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
      pli->resultTupleType->DeleteIfAllowed();
      if( pli->rightRel )
      {
        pli->rightRel->Clear();
        delete pli->rightRel;
      }
      delete pli;

      qp->Close(args[0].addr);
      qp->Close(args[1].addr);

      return 0;
    }
  }
  return 0;
}

/*
5.10.3 Specification of operator ~product~

*/
const string ProductSpec  = 
  "( ( \"Signature\" \"Syntax\" \"Meaning\" "
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
         "product",              // name
         ProductSpec,            // specification
         Product,                // value mapping
         Operator::SimpleSelect, // trivial selection function
         ProductTypeMap          // type mapping
//         true                  // needs large amounts of memory
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
  string argstr;
  
  CHECK_COND(nl->ListLength(args) == 1,
  "Operator count expects a list of length one.");

  first = nl->First(args);
  
  nl->WriteToString(argstr, first);    
  CHECK_COND( nl->ListLength(first) == 2 && 
    nl->ListLength(nl->Second(first)) == 2 &&
    (TypeOfRelAlgSymbol(nl->First(first)) == stream ||
    TypeOfRelAlgSymbol(nl->First(first)) == rel) &&
    TypeOfRelAlgSymbol(nl->First(nl->Second(first))) == tuple ,
    "Operator count expects a list with structure " 
    "(stream (tuple ((a1 t1)...(an tn)))) or "
    "(rel (tuple ((a1 t1)...(an tn))))\n"
    "Operator count gets a list with structure '" + argstr + "'.");
  
  return nl->SymbolAtom("int");
}


/*
5.11.2 Value mapping functions of operator ~count~

*/

#ifndef USE_PROGRESS

// standard version

int
TCountStream(Word* args, Word& result, int message, 
             Word& local, Supplier s)
{
  Word elem;
  int count = 0;

  qp->Open(args[0].addr);
  qp->Request(args[0].addr, elem);
  while ( qp->Received(args[0].addr) )
  {
    ((Tuple*)elem.addr)->DeleteIfAllowed();
    qp->Request(args[0].addr, elem);
    count++;
  }
  result = qp->ResultStorage(s);
  ((CcInt*) result.addr)->Set(true, count);
  qp->Close(args[0].addr);
  return 0;
}

int
TCountRel(Word* args, Word& result, int message, 
          Word& local, Supplier s)
{
  Relation* rel = (Relation*)args[0].addr;
  result = qp->ResultStorage(s);
  ((CcInt*) result.addr)->Set(true, rel->GetNoTuples());
  return 0;
}

#else

int
TCountStream(Word* args, Word& result, int message, 
             Word& local, Supplier s)
{
  Word elem;
  int count = 0;


  if (message != PROGRESS)
  {

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
  else  //message == PROGRESS
  {
    		//cout << "count was asked for progress" << endl;

    ProgressInfo p1;
    ProgressInfo* pRes;

    pRes = (ProgressInfo*) result.addr; 

    if ( qp->RequestProgress(args[0].addr, &p1) )
    {
      pRes->Card = p1.Card;
      pRes->Size = p1.Size;
      pRes->SizeCE = p1.SizeCE;
      pRes->Time = p1.Time;
      pRes->Progress =  p1.Progress; 
      return YIELD;			//successful
    }
    else return CANCEL;			//no progress available
  }
}



int
TCountRel(Word* args, Word& result, int message, 
          Word& local, Supplier s)
{
  if ( message != PROGRESS ) 		//normal evaluation
  {
    Relation* rel = (Relation*)args[0].addr;
    result = qp->ResultStorage(s);
    ((CcInt*) result.addr)->Set(true, rel->GetNoTuples());
    return 0;
  }

  else	//message == PROGRESS
  {
    Supplier sonOfCount;
    ProgressInfo p1;
    ProgressInfo* pRes;
    
    pRes = (ProgressInfo*) result.addr; 
    sonOfCount = qp->GetSupplierSon(s, 0);

    if ( qp->IsObjectNode(sonOfCount) )
    {
      return CANCEL;
    }
    else
    { 
      if ( qp->RequestProgress(sonOfCount, &p1) )
      {
        pRes->Card = p1.Card;
        pRes->Size = p1.Size;
        pRes->SizeCE = p1.SizeCE;
        pRes->Time = p1.Time;
        pRes->Progress = p1.Progress;
        return YIELD;
      }
      else return CANCEL;
    }
  }
}

#endif



/*
5.11.3 Value mapping function for operator ~count2~ 

This operator generates messages! Refer to "Messages.h".

*/
#include "Messages.h"
int
TCountStream2(Word* args, Word& result, int message, 
              Word& local, Supplier s)
{
  Word elem;
  int count = 0;

  // Get a reference to the message center
  static MessageCenter* msg = MessageCenter::GetInstance();
  
  qp->Open(args[0].addr);
  qp->Request(args[0].addr, elem);
  while ( qp->Received(args[0].addr) )
  {
    if ((count % 100) == 0) {    
      // build a two elem list (simple count)	    
      NList msgList( NList("simple"), NList(count) );
      // send the message, the message center will call 
      // the registered handlers. Normally the client applications
      // will register them. 
      msg->Send(msgList);
    }	      
    ((Tuple*)elem.addr)->DeleteIfAllowed();
    qp->Request(args[0].addr, elem);
    count++;
  }
  result = qp->ResultStorage(s);
  ((CcInt*) result.addr)->Set(true, count);
  qp->Close(args[0].addr);
  return 0;
}




/*
5.11.3 Specification of operator ~count~

*/
const string TCountSpec  = 
"( ( \"Signature\" \"Syntax\" \"Meaning\" "
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
  ListExpr first = nl->First(args);
  if (TypeOfRelAlgSymbol(nl->First(first)) == stream)
    return 0;
  else if(TypeOfRelAlgSymbol(nl->First(first)) == rel)
    return 1;
  return -1;
}

/*

5.11.5 Definition of operator ~count~

*/
ValueMapping countmap[] = {TCountStream, TCountRel };

Operator relalgcount (
         "count",           // name
         TCountSpec,        // specification
         2,                 // number of value mapping functions
         countmap,          // value mapping functions
         TCountSelect,      // selection function
         TCountTypeMap      // type mapping
);


const string TCountSpec2  = 
"( ( \"Signature\" \"Syntax\" \"Meaning\" "
"\"Example\" ) "
"( '((stream (tuple x))) -> int'"
"'_ count2'"
"'Count number of tuples within a stream "
"or a relation of tuples. During computation example messages are sent.'"
"'query plz feed count2'"
") )";

Operator relalgcount2 (
         "count2",                // name
         TCountSpec2,             // specification
         TCountStream2,           // value mapping function
	 Operator::SimpleSelect,
         TCountTypeMap            // type mapping
);
/*
5.11 Operator ~roottuplesize~

Reports the size of the tuples' root part in a relation. This operator 
is useful for the optimizer, but it is usable as an operator itself.

5.11.1 Type mapping function of operator ~roottuplesize~

Operator ~roottuplesize~ accepts a relation or a stream of tuples and 
returns an integer.

----    (rel     (tuple x))                 -> int
        (stream  (tuple x))                 -> int
----

*/
ListExpr
RootTupleSizeTypeMap(ListExpr args)
{
  ListExpr first;
  string argstr;
  
  CHECK_COND(nl->ListLength(args) == 1,
  "Operator tuplesize expects a list of length one.");

  first = nl->First(args);
  
  nl->WriteToString(argstr, first);    
  CHECK_COND( nl->ListLength(first) == 2 && 
    nl->ListLength(nl->Second(first)) == 2 &&
    (TypeOfRelAlgSymbol(nl->First(first)) == stream ||
    TypeOfRelAlgSymbol(nl->First(first)) == rel) &&
    TypeOfRelAlgSymbol(nl->First(nl->Second(first))) == tuple ,
    "Operator roottuplesize expects a list with structure " 
    "(stream (tuple ((a1 t1)...(an tn)))) or "
    "(rel (tuple ((a1 t1)...(an tn))))\n"
    "Operator roottuplesize gets a list with structure '" + 
    argstr + "'.");
  
  return nl->SymbolAtom("int");
}

/*

5.11.2 Value mapping functions of operator ~roottuplesize~

*/
int
RootTupleSizeStream(Word* args, Word& result, int message, 
                    Word& local, Supplier s)
{
  Word elem;
  int count = 0;
  double totalSize = 0;

  qp->Open(args[0].addr);
  qp->Request(args[0].addr, elem);
  while ( qp->Received(args[0].addr) )
  {
    totalSize += ((Tuple*)elem.addr)->GetRootSize();
    count++;
    ((Tuple*)elem.addr)->DeleteIfAllowed();
    qp->Request(args[0].addr, elem);
  }
  result = qp->ResultStorage(s);

  ((CcInt*) result.addr)->Set( true, (int)(totalSize / count) );
  qp->Close(args[0].addr);
  return 0;
}

int
RootTupleSizeRel(Word* args, Word& result, int message, 
                 Word& local, Supplier s)
{
  Relation* rel = (Relation*)args[0].addr;
  result = qp->ResultStorage(s);
  ((CcInt*) result.addr)->
    Set(true, (int)(rel->GetTotalRootSize() / rel->GetNoTuples()) );
  return 0;
}

/*

5.11.3 Specification of operator ~roottuplesize~

*/
const string RootTupleSizeSpec  = 
  "( ( \"Signature\" \"Syntax\" \"Meaning\" "
  "\"Example\" ) "
  "( <text>((stream|rel (tuple x))) -> int"
  "</text--->"
  "<text>_ roottuplesize</text--->"
  "<text>Return the size of the tuples' root part within a "
  "stream or a relation.</text--->"
  "<text>query cities roottuplesize or query cities "
  "feed roottuplesize</text--->"
  ") )";

/*
5.11.4 Selection function of operator ~roottuplesize~

This function is the same as for the ~count~ operator.

5.11.5 Definition of operator ~roottuplesize~

*/
ValueMapping roottuplesizemap[] = {RootTupleSizeStream, 
                                   RootTupleSizeRel };

Operator relalgroottuplesize (
         "roottuplesize",           // name
         RootTupleSizeSpec,         // specification
         2,                     // number of value mapping functions
         roottuplesizemap,          // value mapping functions
         TCountSelect,          // selection function
         RootTupleSizeTypeMap       // type mapping
);

/*
5.11 Operator ~exttuplesize~

Reports the average size of the tuples in a relation taking into
account the extension part, i.e. the small FLOBs. This operator 
is useful for the optimizer, but it is usable as an operator itself.

5.11.1 Type mapping function of operator ~exttuplesize~

Operator ~exttuplesize~ accepts a relation or a stream of tuples 
and returns a real.

----    (rel     (tuple x))                 -> real
        (stream  (tuple x))                 -> real
----

*/
ListExpr
ExtTupleSizeTypeMap(ListExpr args)
{
  ListExpr first;
  string argstr;
  
  CHECK_COND(nl->ListLength(args) == 1,
  "Operator exttuplesize expects a list of length one.");

  first = nl->First(args);
  
  nl->WriteToString(argstr, first);    
  CHECK_COND( nl->ListLength(first) == 2 && 
    nl->ListLength(nl->Second(first)) == 2 &&
    (TypeOfRelAlgSymbol(nl->First(first)) == stream ||
    TypeOfRelAlgSymbol(nl->First(first)) == rel) &&
    TypeOfRelAlgSymbol(nl->First(nl->Second(first))) == tuple ,
    "Operator exttuplesize expects a list with structure " 
    "(stream (tuple ((a1 t1)...(an tn)))) or "
    "(rel (tuple ((a1 t1)...(an tn))))\n"
    "Operator exttuplesize gets a list with structure '" + 
    argstr + "'.");
  
  return nl->SymbolAtom("real");
}

/*

5.11.2 Value mapping functions of operator ~exttuplesize~

*/
int
ExtTupleSizeStream(Word* args, Word& result, int message, 
                   Word& local, Supplier s)
{
  Word elem;
  int count = 0;
  float size = 0;

  qp->Open(args[0].addr);
  qp->Request(args[0].addr, elem);
  while ( qp->Received(args[0].addr) )
  {
    size += ((Tuple*)elem.addr)->GetExtSize();
    count++;
    ((Tuple*)elem.addr)->DeleteIfAllowed();
    qp->Request(args[0].addr, elem);
  }
  result = qp->ResultStorage(s);

  ((CcReal*) result.addr)->Set(true, size/count);
  qp->Close(args[0].addr);
  return 0;
}

int
ExtTupleSizeRel(Word* args, Word& result, int message, 
                Word& local, Supplier s)
{
  Relation* rel = (Relation*)args[0].addr;
  result = qp->ResultStorage(s);
  ((CcReal*) result.addr)->
    Set(true, (float)rel->GetTotalExtSize()/rel->GetNoTuples());
  return 0;
}


/*

5.11.3 Specification of operator ~exttuplesize~

*/
const string ExtTupleSizeSpec  = 
  "( ( \"Signature\" \"Syntax\" \"Meaning\" "
  "\"Example\" ) "
  "( <text>((stream/rel (tuple x))) -> real"
  "</text--->"
  "<text>_ exttuplesize</text--->"
  "<text>Return the average size of the tuples within a stream "
  "or a relation taking into account the small FLOBs.</text--->"
  "<text>query cities exttuplesize or query cities "
  "feed exttuplesize</text--->"
  ") )";

/*
5.11.4 Selection function of operator ~exttuplesize~

This function is the same as for the ~count~ operator.

5.11.5 Definition of operator ~exttuplesize~

*/
ValueMapping exttuplesizemap[] = {ExtTupleSizeStream, 
                                  ExtTupleSizeRel };

Operator relalgexttuplesize (
         "exttuplesize",           // name
         ExtTupleSizeSpec,         // specification
         2,                     // number of value mapping functions
         exttuplesizemap,          // value mapping functions
         TCountSelect,          // selection function
         ExtTupleSizeTypeMap       // type mapping
);

/*
5.11 Operator ~tuplesize~

Reports the average size of the tuples in a relation. This operator 
is useful for the optimizer, but it is usable as an operator itself.

5.11.1 Type mapping function of operator ~tuplesize~

Operator ~tuplesize~ accepts a stream of tuples and returns a 
real.

----    (real    (tuple x))                 -> real
        (stream  (tuple x))                 -> real
----

*/
ListExpr
TupleSizeTypeMap(ListExpr args)
{
  ListExpr first;
  string argstr;
  
  CHECK_COND(nl->ListLength(args) == 1,
  "Operator tuplesize expects a list of length one.");

  first = nl->First(args);
  
  nl->WriteToString(argstr, first);    
  CHECK_COND( nl->ListLength(first) == 2 && 
    nl->ListLength(nl->Second(first)) == 2 &&
    (TypeOfRelAlgSymbol(nl->First(first)) == stream ||
    TypeOfRelAlgSymbol(nl->First(first)) == rel) &&
    TypeOfRelAlgSymbol(nl->First(nl->Second(first))) == tuple ,
    "Operator tuplesize expects a list with structure " 
    "(stream (tuple ((a1 t1)...(an tn)))) or "
    "(rel (tuple ((a1 t1)...(an tn))))\n"
    "Operator tuplesize gets a list with structure '" + 
    argstr + "'.");
  
  return nl->SymbolAtom("real");
}

/*

5.11.2 Value mapping functions of operator ~tuplesize~

*/
int
TupleSizeStream(Word* args, Word& result, int message, 
                Word& local, Supplier s)
{
  Word elem;
  int count = 0;
  float size = 0;

  qp->Open(args[0].addr);
  qp->Request(args[0].addr, elem);
  while ( qp->Received(args[0].addr) )
  {
    size += ((Tuple*)elem.addr)->GetSize();
    count++;
    ((Tuple*)elem.addr)->DeleteIfAllowed();
    qp->Request(args[0].addr, elem);
  }
  result = qp->ResultStorage(s);

  ((CcReal*) result.addr)->Set(true, size/count);
  qp->Close(args[0].addr);
  return 0;
}

int
TupleSizeRel(Word* args, Word& result, int message, 
             Word& local, Supplier s)
{
  Relation* rel = (Relation*)args[0].addr;
  result = qp->ResultStorage(s);
  ((CcReal*) result.addr)->
    Set(true, (float)rel->GetTotalSize()/rel->GetNoTuples());
  return 0;
}


/*

5.11.3 Specification of operator ~tuplesize~

*/
const string TupleSizeSpec  = 
  "( ( \"Signature\" \"Syntax\" \"Meaning\" "
  "\"Example\" ) "
  "( <text>((stream/rel (tuple x))) -> real"
  "</text--->"
  "<text>_ tuplesize</text--->"
  "<text>Return the average size of the tuples within a stream "
  "or a relation taking into account the FLOBs.</text--->"
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
         2,                     // number of value mapping functions
         tuplesizemap,          // value mapping functions
         TCountSelect,          // selection function
         TupleSizeTypeMap       // type mapping
);

/*
5.11 Operator ~rootattrsize~

Reports the size of the attribute's root part in a relation. This operator
is useful for the optimizer, but it is usable as an operator itself.

5.11.1 Type mapping function of operator ~rootattrsize~

Operator ~rootattrsize~ accepts a relation or a stream of tuples and
an attribute name identifier and returns an integer.

----    (rel     (tuple X)) x ident         -> int
        (stream  (tuple X)) x ident         -> int
----

*/
ListExpr
RootAttrSizeTypeMap(ListExpr args)
{
  ListExpr first, second, attrtype;
  string argstr, attrname;

  CHECK_COND(nl->ListLength(args) == 2,
  "Operator tuplesize expects a list of length two.");

  first = nl->First(args);
  second = nl->Second(args);

  nl->WriteToString(argstr, first);
  CHECK_COND( nl->ListLength(first) == 2 &&
    nl->ListLength(nl->Second(first)) == 2 &&
    (TypeOfRelAlgSymbol(nl->First(first)) == stream ||
    TypeOfRelAlgSymbol(nl->First(first)) == rel) &&
    TypeOfRelAlgSymbol(nl->First(nl->Second(first))) == tuple ,
    "Operator rootattrsize expects as first argument "
    "a list with structure "
    "(stream (tuple ((a1 t1)...(an tn)))) or "
    "(rel (tuple ((a1 t1)...(an tn))))\n"
    "Operator rootattrsize gets as first argument "
    "a list with structure '" +
    argstr + "'.");

  nl->WriteToString(argstr, second);
  CHECK_COND(
    nl->IsAtom(second),
    "Operator rootattrsize expects as second argument an attribute name\n"
    "Operator rootattrsize gets a list '" + argstr + "'.");

  if (nl->AtomType(second) == SymbolType)
    attrname = nl->SymbolValue(second);
  else
  {
    ErrorReporter::ReportError(
      "Attribute name in the list is not of symbol type.");
    return nl->SymbolAtom("typeerror");
  }

  int j = FindAttribute(nl->Second(nl->Second(first)),
                        attrname, attrtype);
  if (!j)
  {
    ErrorReporter::ReportError(
      "Operator rootattrsize: Attribute name '" + attrname +
      "' is not a known attribute name in the tuple stream.");
        return nl->SymbolAtom("typeerror");
  }

  // Check whether all new attribute names are distinct
  // - not yet implemented

  return
    nl->ThreeElemList(
      nl->SymbolAtom("APPEND"),
      nl->OneElemList(nl->IntAtom(j)),
      nl->SymbolAtom("int"));
}

/*

5.11.2 Value mapping functions of operator ~rootattrsize~

*/
int
RootAttrSizeStream(Word* args, Word& result, int message,
                   Word& local, Supplier s)
{
  Word elem;
  int count = 0;
  double totalSize = 0;

  qp->Open(args[0].addr);
  qp->Request(args[0].addr, elem);
  int i = ((CcInt*)args[2].addr)->GetIntval() - 1;
  while ( qp->Received(args[0].addr) )
  {
    totalSize += ((Tuple*)elem.addr)->GetRootSize(i);
    count++;
    ((Tuple*)elem.addr)->DeleteIfAllowed();
    qp->Request(args[0].addr, elem);
  }
  result = qp->ResultStorage(s);

  ((CcInt*) result.addr)->Set( true, (int)(totalSize / count) );
  qp->Close(args[0].addr);
  return 0;
}

int
RootAttrSizeRel(Word* args, Word& result, int message,
                 Word& local, Supplier s)
{
  Relation* rel = (Relation*)args[0].addr;
  result = qp->ResultStorage(s);
  int i = ((CcInt*)args[2].addr)->GetIntval() - 1;
  ((CcInt*) result.addr)->
    Set(true, (int)(rel->GetTotalRootSize(i) / rel->GetNoTuples()) );
  return 0;
}

/*

5.11.3 Specification of operator ~rootattrsize~

*/
const string RootAttrSizeSpec  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" "
  "\"Example\" ) "
  "( <text>stream|rel(tuple X) x ident -> int"
  "</text--->"
  "<text>_ rootattrsize[_]</text--->"
  "<text>Return the size of the attributes' root part within a "
  "stream or a relation.</text--->"
  "<text>query cities rootattrsize[loc] or query cities "
  "feed rootattrsize[loc]</text--->"
  ") )";

/*
5.11.4 Selection function of operator ~rootattrsize~

This function is the same as for the ~count~ operator.

5.11.5 Definition of operator ~rootattrsize~

*/
ValueMapping rootattrsizemap[] = {RootAttrSizeStream,
                                  RootAttrSizeRel };

Operator relalgrootattrsize (
         "rootattrsize",           // name
         RootAttrSizeSpec,         // specification
         2,                     // number of value mapping functions
         rootattrsizemap,          // value mapping functions
         TCountSelect,          // selection function
         RootAttrSizeTypeMap       // type mapping
);

/*
5.11 Operator ~extattrsize~

Reports the size of the attribute in a relation taking into account
the extended part, i.e. the small FLOBs. This operator
is useful for the optimizer, but it is usable as an operator itself.

5.11.1 Type mapping function of operator ~extattrsize~

Operator ~extattrsize~ accepts a relation or a stream of tuples and
an attribute name identifier and returns a real.

----    (rel     (tuple X)) x ident         -> real
        (stream  (tuple X)) x ident         -> real
----

*/
ListExpr
ExtAttrSizeTypeMap(ListExpr args)
{
  ListExpr first, second, attrtype;
  string argstr, attrname;

  CHECK_COND(nl->ListLength(args) == 2,
  "Operator tuplesize expects a list of length two.");

  first = nl->First(args);
  second = nl->Second(args);

  nl->WriteToString(argstr, first);
  CHECK_COND( nl->ListLength(first) == 2 &&
    nl->ListLength(nl->Second(first)) == 2 &&
    (TypeOfRelAlgSymbol(nl->First(first)) == stream ||
    TypeOfRelAlgSymbol(nl->First(first)) == rel) &&
    TypeOfRelAlgSymbol(nl->First(nl->Second(first))) == tuple ,
    "Operator extattrsize expects as first argument "
    "a list with structure "
    "(stream (tuple ((a1 t1)...(an tn)))) or "
    "(rel (tuple ((a1 t1)...(an tn))))\n"
    "Operator extattrsize gets as first argument "
    "a list with structure '" +
    argstr + "'.");

  nl->WriteToString(argstr, second);
  CHECK_COND(
    nl->IsAtom(second),
    "Operator extattrsize expects as second argument an attribute name\n"
    "Operator extattrsize gets a list '" + argstr + "'.");

  if (nl->AtomType(second) == SymbolType)
    attrname = nl->SymbolValue(second);
  else
  {
    ErrorReporter::ReportError(
      "Attribute name in the list is not of symbol type.");
    return nl->SymbolAtom("typeerror");
  }

  int j = FindAttribute(nl->Second(nl->Second(first)),
                        attrname, attrtype);
  if (!j)
  {
    ErrorReporter::ReportError(
      "Operator extattrsize: Attribute name '" + attrname +
      "' is not a known attribute name in the tuple stream.");
        return nl->SymbolAtom("typeerror");
  }

  // Check whether all new attribute names are distinct
  // - not yet implemented

  return
    nl->ThreeElemList(
      nl->SymbolAtom("APPEND"),
      nl->OneElemList(nl->IntAtom(j)),
      nl->SymbolAtom("real"));
}

/*

5.11.2 Value mapping functions of operator ~extattrsize~

*/
int
ExtAttrSizeStream(Word* args, Word& result, int message,
                  Word& local, Supplier s)
{
  Word elem;
  int count = 0;
  double totalSize = 0;

  qp->Open(args[0].addr);
  qp->Request(args[0].addr, elem);
  int i = ((CcInt*)args[2].addr)->GetIntval() - 1;
  while ( qp->Received(args[0].addr) )
  {
    totalSize += ((Tuple*)elem.addr)->GetExtSize(i);
    count++;
    ((Tuple*)elem.addr)->DeleteIfAllowed();
    qp->Request(args[0].addr, elem);
  }
  result = qp->ResultStorage(s);

  ((CcReal*) result.addr)->Set( true, totalSize / count );
  qp->Close(args[0].addr);
  return 0;
}

int
ExtAttrSizeRel(Word* args, Word& result, int message,
                 Word& local, Supplier s)
{
  Relation* rel = (Relation*)args[0].addr;
  result = qp->ResultStorage(s);
  int i = ((CcInt*)args[2].addr)->GetIntval() - 1;
  ((CcReal*) result.addr)->
    Set(true, rel->GetTotalExtSize(i) / rel->GetNoTuples() );
  return 0;
}

/*

5.11.3 Specification of operator ~extattrsize~

*/
const string ExtAttrSizeSpec  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" "
  "\"Example\" ) "
  "( <text>stream|rel(tuple X) x ident -> real"
  "</text--->"
  "<text>_ extattrsize[_]</text--->"
  "<text>Return the size of the attribute within a "
  "stream or a relation taking into account the "
  "small FLOBs.</text--->"
  "<text>query cities extattrsize[loc] or query cities "
  "feed extattrsize[loc]</text--->"
  ") )";

/*
5.11.4 Selection function of operator ~extattrsize~

This function is the same as for the ~count~ operator.

5.11.5 Definition of operator ~extattrsize~

*/
ValueMapping extattrsizemap[] = {ExtAttrSizeStream,
                                 ExtAttrSizeRel };

Operator relalgextattrsize (
         "extattrsize",           // name
         ExtAttrSizeSpec,         // specification
         2,                     // number of value mapping functions
         extattrsizemap,          // value mapping functions
         TCountSelect,          // selection function
         ExtAttrSizeTypeMap       // type mapping
);

/*
5.11 Operator ~attrsize~

Reports the size of the attribute in a relation taking into account
the FLOBs. This operator is useful for the optimizer, but it is
usable as an operator itself.

5.11.1 Type mapping function of operator ~attrsize~

Operator ~attrsize~ accepts a relation or a stream of tuples and
an attribute name identifier and returns a real.

----    (rel     (tuple X)) x ident         -> real
        (stream  (tuple X)) x ident         -> real
----

*/
ListExpr
AttrSizeTypeMap(ListExpr args)
{
  ListExpr first, second, attrtype;
  string argstr, attrname;

  CHECK_COND(nl->ListLength(args) == 2,
  "Operator tuplesize expects a list of length two.");

  first = nl->First(args);
  second = nl->Second(args);

  nl->WriteToString(argstr, first);
  CHECK_COND( nl->ListLength(first) == 2 &&
    nl->ListLength(nl->Second(first)) == 2 &&
    (TypeOfRelAlgSymbol(nl->First(first)) == stream ||
    TypeOfRelAlgSymbol(nl->First(first)) == rel) &&
    TypeOfRelAlgSymbol(nl->First(nl->Second(first))) == tuple ,
    "Operator attrsize expects as first argument "
    "a list with structure "
    "(stream (tuple ((a1 t1)...(an tn)))) or "
    "(rel (tuple ((a1 t1)...(an tn))))\n"
    "Operator attrsize gets as first argument "
    "a list with structure '" +
    argstr + "'.");

  nl->WriteToString(argstr, second);
  CHECK_COND(
    nl->IsAtom(second),
    "Operator attrsize expects as second argument an attribute name\n"
    "Operator attrsize gets a list '" + argstr + "'.");

  if (nl->AtomType(second) == SymbolType)
    attrname = nl->SymbolValue(second);
  else
  {
    ErrorReporter::ReportError(
      "Attribute name in the list is not of symbol type.");
    return nl->SymbolAtom("typeerror");
  }

  int j = FindAttribute(nl->Second(nl->Second(first)),
                        attrname, attrtype);
  if (!j)
  {
    ErrorReporter::ReportError(
      "Operator attrsize: Attribute name '" + attrname +
      "' is not a known attribute name in the tuple stream.");
        return nl->SymbolAtom("typeerror");
  }

  // Check whether all new attribute names are distinct
  // - not yet implemented

  return
    nl->ThreeElemList(
      nl->SymbolAtom("APPEND"),
      nl->OneElemList(nl->IntAtom(j)),
      nl->SymbolAtom("real"));
}

/*

5.11.2 Value mapping functions of operator ~attrsize~

*/
int
AttrSizeStream(Word* args, Word& result, int message,
               Word& local, Supplier s)
{
  Word elem;
  int count = 0;
  double totalSize = 0;

  qp->Open(args[0].addr);
  qp->Request(args[0].addr, elem);
  int i = ((CcInt*)args[2].addr)->GetIntval() - 1;
  while ( qp->Received(args[0].addr) )
  {
    totalSize += ((Tuple*)elem.addr)->GetSize(i);
    count++;
    ((Tuple*)elem.addr)->DeleteIfAllowed();
    qp->Request(args[0].addr, elem);
  }
  result = qp->ResultStorage(s);

  ((CcReal*) result.addr)->Set( true, totalSize / count );
  qp->Close(args[0].addr);
  return 0;
}

int
AttrSizeRel(Word* args, Word& result, int message,
            Word& local, Supplier s)
{
  Relation* rel = (Relation*)args[0].addr;
  result = qp->ResultStorage(s);
  int i = ((CcInt*)args[2].addr)->GetIntval() - 1;
  ((CcReal*) result.addr)->
    Set(true, rel->GetTotalSize(i) / rel->GetNoTuples() );
  return 0;
}

/*

5.11.3 Specification of operator ~attrsize~

*/
const string AttrSizeSpec  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" "
  "\"Example\" ) "
  "( <text>stream|rel(tuple X) x ident -> real"
  "</text--->"
  "<text>_ attrsize[_]</text--->"
  "<text>Return the size of the attribute within a "
  "stream or a relation taking into account the "
  "FLOBs.</text--->"
  "<text>query cities attrsize[loc] or query cities "
  "feed attrsize[loc]</text--->"
  ") )";

/*
5.11.4 Selection function of operator ~attrsize~

This function is the same as for the ~count~ operator.

5.11.5 Definition of operator ~attrsize~

*/
ValueMapping attrsizemap[] = {AttrSizeStream,
                              AttrSizeRel };

Operator relalgattrsize (
         "attrsize",           // name
         AttrSizeSpec,         // specification
         2,                     // number of value mapping functions
         attrsizemap,          // value mapping functions
         TCountSelect,          // selection function
         AttrSizeTypeMap       // type mapping
);


/*
5.12 Operator ~sizecounters~

This operator maps

----   (stream  (tuple X)) x string -> (stream (tuple X)) 
----

It sums up the sizes (Root, Extension, Flobs) of the tuples of a given input
stream.  These values are stored as entries in the system table SEC\_COUNTERS
which can be queried afterwards. 

5.12.0 Specification 

*/

struct SizeCountersInfo : OperatorInfo {
 
  SizeCountersInfo() : OperatorInfo()
  { 
    name =      "sizecounters";
    signature = "stream(tuple(y)) x string -> stream(tuple(y))";
    syntax =    "_ sizecounters[ s ]";
    meaning =   "Sums up the size for the tuples root size, the extension "
                "size and the flob size. The results will be stored in "
                "counters which are named \n\n"
                "- RA:RootSize_s\n"
                "- RA:ExtSize_s (Root + Extension)\n"
                "- RA:Size_s (Root + Extension + Flobs)\n"
                "- RA:FlobSizeOnly_s\n"
                "- RA:ExtSizeOnly_s\n";
    example =   "plz feed sizecounters[\"plz\"] count";
  }

};



/*
5.12.1 Type mapping 

The type mapping uses the wrapper class ~NList~ which hides calls
to class NestedList. Moreover, there are some useful functions for
handling streams of tuples.

*/

static ListExpr sizecounters_tm(ListExpr args)
{
  NList l(args);
  
  const string opName = "sizecounters";
  string err1 = opName + "expects (stream(tuple(...)) string)!";
  cout << opName << ": " << l << endl;
  
  if ( !l.checkLength(2, err1) )
    return l.typeError( err1 );
  
  NList attrs;
  if ( !l.first().checkStreamTuple( attrs ) )
    return l.typeError(err1);
  
  if ( !l.second().isSymbol(Symbols::STRING()) ) 
    return l.typeError(err1);

  return l.first().listExpr();
}

/*

5.12.1 Value mapping 

*/

static int sizecounters_vm( Word* args, Word& result, int message, 
                    Word& local, Supplier s)
{
  // args[0]: stream(tuple(...)) 
  // args[1]: string
  
  struct Info {

    long& rootSize;
    long& extSize;
    long& size;
    long& extSizeOnly;
    long& flobSizeOnly;
    long& tuples;

    Info(const string& s) :
     rootSize(Counter::getRef("RA::RootSize_" + s)),
     extSize(Counter::getRef("RA::ExtSize_" + s)),
     size(Counter::getRef("RA::Size_" + s)),
     extSizeOnly(Counter::getRef("RA::ExtSizeOnly_" + s)),
     flobSizeOnly(Counter::getRef("RA::FlobSizeOnly_" + s)),
     tuples(Counter::getRef("RA::Tuples_" + s))
    {}
    ~Info()
    {} 

    void computeSizes()
    {
      extSizeOnly = size - rootSize;
      flobSizeOnly = size - extSize;
    } 

    void addSizes(Tuple* t)
    {
      tuples++;
      rootSize += t->GetRootSize(); 
      extSize += t->GetExtSize(); 
      size += t->GetSize();
    }
    
  };

  Info* pi = static_cast<Info*>( local.addr );
  void* stream = args[0].addr;
  
  switch ( message )
  {

    case OPEN: {
      
      qp->Open(stream); 
      local.addr = new Info( StdTypes::GetString(args[1]) );
      return 0;
    }
               
    case REQUEST: {
     
      Word elem;
      qp->Request(stream, elem);
      if ( qp->Received(stream) ) 
      {
        Tuple* t = static_cast<Tuple*>(elem.addr);
        pi->addSizes(t);
        result = elem; 
        return YIELD;
      }
      else
      {  
        return CANCEL;
      }  
    }
                  
    case CLOSE: {

      pi->computeSizes();           
      delete pi;
      qp->Close(stream);      
      return 0;           
    }
                
    default: {
       assert(false);
    }       
  }
  return 0;
}   

/*
5.13 Operator ~dumpstream~

This operator maps

----   (stream (tuple y)) x string -> (stream (tuple y))
----

It just passes the tuples streamupwards and dumps the tuple values into a file
which is specified by the 2nd parameter.

5.12.0 Specification 

*/

struct DumpStreamInfo : OperatorInfo {
 
  DumpStreamInfo() : OperatorInfo()
  { 
    name =      "dumpstream";
    signature = "stream(tuple(y)) x string -> stream(tuple(y))";
    syntax =    "_ dumpstream[ f, s ]";
    meaning =   "Appends the tuples' values in a file specified by f. "
                "The attribute values are separated by s.";
    example =   "plz feed dumpstream[\"plz.txt\", \"|\"] count";
  }

};


/*
5.12.1 Type mapping 

The type mapping uses the wrapper class ~NList~ which hides calls
to class NestedList. Moreover, there are some useful functions for
handling streams of tuples.

*/

static ListExpr dumpstream_tm(ListExpr args)
{
  NList l(args);
  
  const string opName = "dumpstream";
  string err1 = opName + "expects (stream(tuple(...)) string string)!";
  
  if ( !l.checkLength(3, err1) )
    return l.typeError( err1 );
  
  NList attrs;
  if ( !l.first().checkStreamTuple( attrs ) )
    return l.typeError(err1);
  
  if ( !l.second().isSymbol(Symbols::STRING()) ) 
    return l.typeError(err1);

  if ( !l.third().isSymbol(Symbols::STRING()) ) 
    return l.typeError(err1);

  return l.first().listExpr();
}

/*

5.12.1 Value mapping 

*/

static int dumpstream_vm( Word* args, Word& result, int message, 
                    Word& local, Supplier s)
{
  // args[0]: stream(tuple(...)) 
  // args[1]: string -> file name
  // args[2]: string -> separator
  
  struct Info {

    ofstream os;
    const string colsep;
    int tuples;
    
    Info(const string& fileName, const string& sep) :
      colsep(sep)
    {
      os.open(fileName.c_str(), ios_base::app);
    }
    ~Info()
    {
      os.close();
    } 

    void appendTuple(Tuple& t)
    { 
      for( int i = 0; i < t.GetNoAttributes(); i++)
      {
        os << *t.GetAttribute(i);
        if (i < t.GetNoAttributes() - 1)
          os << colsep;
        else
          os << endl;
      }
    }  
    
  };

  Info* pi = static_cast<Info*>( local.addr );
  void* stream = args[0].addr;
  
  switch ( message )
  {

    case OPEN: {
      
      qp->Open(stream); 
                
       string name = StdTypes::GetString(args[1]);          
       string colsep = StdTypes::GetString(args[2]);
      
       local.addr = new Info(name, colsep);
       return 0;
     }
                
     case REQUEST: {
      
      Word elem;
      qp->Request(stream, elem);
      if ( qp->Received(stream) ) 
      {
        Tuple* t = static_cast<Tuple*>(elem.addr);
        pi->appendTuple(*t);
        cerr << *t << endl;
        result = elem; 
        return YIELD;
      }
      else
      {  
        return CANCEL;
      }  
    }
                  
    case CLOSE: {

      delete pi;
      qp->Close(stream);
      return 0;           
    }
                
    default: {
       assert(false);
    }       
  }
  return 0;
}   


/*
5.12 Operator ~rename~

Renames all attribute names by adding them with the postfix passed
as parameter.

5.12.1 Type mapping function of operator ~rename~

Type mapping for ~rename~ is

----  ((stream (tuple([a1:d1, ... ,an:dn)))ar) ->
           (stream (tuple([a1ar:d1, ... ,anar:dn)))
----

*/
ListExpr
RenameTypeMap( ListExpr args )
{
  ListExpr first=nl->TheEmptyList();
  ListExpr first2=first, second=first, rest=first, 
           listn=first, lastlistn=first;
  string  attrname="", argstr="";
  string  attrnamen="";
  bool firstcall = true;
  
  CHECK_COND(nl->ListLength(args) == 2,
  "Operator rename expects a list of length two.");
  
  first = nl->First(args);
  second  = nl->Second(args);
  
  nl->WriteToString(argstr, first);  
  CHECK_COND( nl->ListLength(first) == 2   &&
    TypeOfRelAlgSymbol(nl->First(first) == stream) &&
    TypeOfRelAlgSymbol(nl->First(nl->Second(first))) == tuple,
    "Operator rename expects as first argument a list with "
    "structure (stream (tuple ((a1 t1)...(an tn))))\n"
    "Operator rename gets a list with structure '" + argstr + "'.");
  
  nl->WriteToString(argstr, second);   
  CHECK_COND( nl->IsAtom(second) && 
    nl->AtomType(second) == SymbolType,
    "Operator rename expects as second argument a symbol "
    "atom (attribute suffix) " 
    "Operator rename gets '" + argstr + "'.");

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
      lastlistn  = nl->Append(lastlistn,
      nl->TwoElemList(nl->SymbolAtom(attrname), nl->Second(first2)));
    }
    else
    {
      firstcall = false;
      listn = 
        nl->OneElemList(nl->TwoElemList(nl->SymbolAtom(attrname),
      nl->Second(first2)));
      lastlistn = listn;
    }
  }
  return
    nl->TwoElemList(nl->SymbolAtom("stream"),
    nl->TwoElemList(nl->SymbolAtom("tuple"),listn));
}
/*

5.12.2 Value mapping function of operator ~rename~

*/
int
Rename(Word* args, Word& result, int message, 
       Word& local, Supplier s)
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
const string RenameSpec  = 
  "( ( \"Signature\" \"Syntax\" \"Meaning\" "
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
         "rename",               // name
         RenameSpec,             // specification
         Rename,                 // value mapping
         Operator::SimpleSelect, // trivial selection function
         RenameTypeMap           // type mapping
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
  ListExpr first;
  string argstr;
  CHECK_COND(nl->ListLength(args) == 1,
  "Operator mconsume expects a list of length one.");
  
  first = nl->First(args);
  
  nl->WriteToString(argstr, first);  
  CHECK_COND( nl->ListLength(first) == 2 &&
    TypeOfRelAlgSymbol(nl->First(first) == stream) &&
    nl->ListLength(nl->Second(first)) == 2 &&
    TypeOfRelAlgSymbol(nl->First(nl->Second(first))) == tuple,
  "Operator mconsume expects as argument a list with structure " 
  "(stream (tuple ((a1 t1)...(an tn))))\n"
  "Operator mconsume gets a list with structure '" + argstr + "'.");

  return nl->TwoElemList(
           nl->SymbolAtom("mrel"),
           nl->TwoElemList(
             nl->SymbolAtom("mtuple"),
             nl->Second(nl->Second(first))));
}
/*

5.6.2 Value mapping function of operator ~mconsume~

*/
int
MConsume(Word* args, Word& result, int message, 
         Word& local, Supplier s)
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
    CcTuple* tuple = ((Tuple*)actual.addr)->CloneToMemoryTuple();
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
const string MConsumeSpec  = 
  "( ( \"Signature\" \"Syntax\" \"Meaning\" "
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
   "mconsume",             // name
   MConsumeSpec,           // specification
   MConsume,               // value mapping
   Operator::SimpleSelect, // trivial selection function
   MConsumeTypeMap         // type mapping
);

/*

6 Class ~RelationAlgebra~

A new subclass ~RelationAlgebra~ of class ~Algebra~ is declared. 
The only specialization with respect to class ~Algebra~ takes place 
within the constructor: all type constructors and operators are 
registered at the actual algebra.

After declaring the new class, its only instance ~RelationAlgebra~ 
is defined.

*/

class RelationAlgebra : public Algebra
{
 public:
  RelationAlgebra() : Algebra()
  {
    AddTypeConstructor( &cpptuple );
    AddTypeConstructor( &cpprel );

#ifndef USE_PROGRESS

// no support for progress queries

    AddOperator(&relalgfeed);
    AddOperator(&relalgconsume);
    AddOperator(&relalgTUPLE);
    AddOperator(&relalgTUPLE2);
    AddOperator(&relalgattr);
    AddOperator(&relalgfilter);
    AddOperator(&relalgproject);
    AddOperator(&relalgproduct);
    AddOperator(&relalgcount);
    AddOperator(&relalgcount2);
    AddOperator(&relalgroottuplesize);
    AddOperator(&relalgexttuplesize);
    AddOperator(&relalgtuplesize);
    AddOperator(&relalgrootattrsize);
    AddOperator(&relalgextattrsize);
    AddOperator(&relalgattrsize);
    AddOperator(&relalgrename);
    AddOperator(&relalgmconsume);

#else

// support for progress queries

    AddOperator(&relalgfeed);		relalgfeed.EnableProgress();
    AddOperator(&relalgconsume);	relalgconsume.EnableProgress();
    AddOperator(&relalgTUPLE);
    AddOperator(&relalgTUPLE2);
    AddOperator(&relalgattr);
    AddOperator(&relalgfilter);		relalgfilter.EnableProgress();
    AddOperator(&relalgproject);
    AddOperator(&relalgproduct);
    AddOperator(&relalgcount);		relalgcount.EnableProgress();
    AddOperator(&relalgcount2);
    AddOperator(&relalgroottuplesize);
    AddOperator(&relalgexttuplesize);
    AddOperator(&relalgtuplesize);
    AddOperator(&relalgrootattrsize);
    AddOperator(&relalgextattrsize);
    AddOperator(&relalgattrsize);
    AddOperator(&relalgrename);
    AddOperator(&relalgmconsume);

#endif

/*
its important to declare the Operator variable below
as static otherwise it is just a temporary variable and
cannot be accessed by the algebra manager.

*/
    static SizeCountersInfo sizecounters_oi;
    static Operator sizecounters_op( sizecounters_oi, 
                                     sizecounters_vm, 
                                     sizecounters_tm );
    AddOperator(&sizecounters_op);


    static DumpStreamInfo dumpstream_oi;
    static Operator dumpstream_op( dumpstream_oi, 
                                   dumpstream_vm, 
                                   dumpstream_tm );
    AddOperator(&dumpstream_op);

    static ReduceInfo reduce_oi;
    static Operator reduce_op( reduce_oi, 
                               reduce_vm, 
                               reduce_tm );
    AddOperator(&reduce_op);

    
    cpptuple.AssociateKind( "TUPLE" );
    cpprel.AssociateKind( "REL" );

  }
  ~RelationAlgebra() {};
};

RelationAlgebra relationalgebra;
/*

7 Initialization

Each algebra module needs an initialization function. The algebra 
manager has a reference to this function if this algebra is 
included in the list of required algebras, thus forcing the linker 
to include this module.

The algebra manager invokes this function to get a reference to the 
instance of the algebra class and to provide references to the 
global nested list container (used to store constructor, type, 
operator and object information) and to the query processor.

The function has a C interface to make it possible to load the 
algebra dynamically at runtime.

*/

extern "C"
Algebra*
InitializeRelationAlgebra( NestedList* nlRef, QueryProcessor* qpRef )
{
  if ( RTFlag::isActive("RA:ShowFLOBstates") )
    FLOB::debug = true;
  if ( RTFlag::isActive("RA:ShowTUPLEstates") )
    PrivateTuple::debug = true;
 
  nl = nlRef;
  qp = qpRef;
  am = SecondoSystem::GetAlgebraManager();
  return (&relationalgebra);
}

