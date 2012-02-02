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
(~executable~, ~descriptive~, and ~hybrid~). Only the executable
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
#include "OrderedRelationAlgebra.h"
#include "NestedList.h"
#include "QueryProcessor.h"
#include "Algebra.h"
#include "StandardTypes.h"
#include "Progress.h"
#include "FTextAlgebra.h"

#include "LogMsg.h"
#include "NList.h"
#include "ListUtils.h"
#include "Symbols.h"
#include "Stream.h"

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
   if(!Tuple::checkType(type)){
      return false;
   }

   if(!listutils::checkAttrListForNamingConventions(nl->Second(type))){
     return false;
   }
   return true;
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
  TupleType* tt = new TupleType(nl->Second(typeInfo));
  Tuple *tup = new Tuple( tt );
  tt->DeleteIfAllowed();
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
TypeConstructor cpptuple( Tuple::BasicType(),         TupleProp,
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
  Relation* rel = ((Relation *)value.addr);
  return rel->Out( typeInfo, rel->MakeScan() );
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
  //cerr << "CreateRel: " << nl->ToString(typeInfo) << endl;
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
  w.addr = 0;
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
      nl->IsEqual(nl->First(type), Relation::BasicType()))
  {
    return am->CheckKind(Kind::TUPLE(), nl->Second(type), errorInfo);
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
  value.setAddr( Relation::Open( valueRecord, offset, typeInfo ) );
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
TypeConstructor cpprel( Relation::BasicType(),           RelProp,
                        OutRel,          InRel,
                        SaveToListRel,   RestoreFromListRel,
                        CreateRel,       DeleteRel,
                        OpenRel,         SaveRel,
                        CloseRel,        CloneRel,
                        CastRel,         SizeOfRel,
                        CheckRel );




/*

4 Type constructor ~trel~

A ~trel~ is represented as a tuple buffer. It re-uses the in and out functions of class
relation.

*/

class TmpRel {

  public:
  TmpRel(){  };
  ~TmpRel(){};

  static GenericRelation* In( ListExpr typeInfo,
                              ListExpr value,
                              int errorPos,
                              ListExpr& errorInfo,
                              bool& correct       ) {

    // the last parameter of Relation::In indicates that a
    // TupleBuffer instance should be created.
    GenericRelation* r =
      (GenericRelation*)Relation::In( typeInfo, value,
                                      errorPos, errorInfo,
                                      correct, true);
    return r;
  }

};


ListExpr
OutTmpRel(ListExpr typeInfo, Word  value)
{
  GenericRelation* rel = ((Relation *)value.addr);
  return Relation::Out( typeInfo, rel->MakeScan() );
}

Word
InTmpRel(ListExpr typeInfo, ListExpr value,
         int errorPos, ListExpr& errorInfo, bool& correct)
{
  return SetWord( TmpRel::In( typeInfo, value, errorPos,
                                errorInfo, correct ) );
}

Word CreateTmpRel( const ListExpr typeInfo )
{
  return (SetWord( new TupleBuffer() ));
}

void DeleteTmpRel( const ListExpr, Word& w )
{
  delete (TupleBuffer *)w.addr;
  w.addr = 0;
}


bool
 TRelKindCheck( ListExpr type, ListExpr& errorInfo ){
  return listutils::isRelDescription(type,true);
}



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
        ((orel x)    ...)                -> x
----

*/
ListExpr TUPLETypeMap(ListExpr args)
{
  if(nl->ListLength(args)<1){
    ErrorReporter::ReportError("One argument expected");
    return nl->TypeError();
  }
  ListExpr first = nl->First(args);
  if(listutils::isTupleStream(first) ||
     listutils::isRelDescription(first) ||
     listutils::isOrelDescription(first) ){
       return nl->Second(first);
  }
  ErrorReporter::ReportError("rel(tuple(...)) or stream(tuple(...))) expected");
  return nl->TypeError();
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
  return nl->SymbolAtom(Symbol::TYPEERROR());
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
  if(nl->ListLength(args)!=1){
    ErrorReporter::ReportError("one argument expected");
    return nl->TypeError();
  }
  ListExpr first = nl->First(args);
  if(listutils::isRelDescription(first,true)||
     listutils::isRelDescription(first,false)){
    return nl->Cons(nl->SymbolAtom(Symbol::STREAM()), nl->Rest(first));
  }
  if(listutils::isOrelDescription(first))
    return nl->TwoElemList(nl->SymbolAtom(Symbol::STREAM()), nl->Second(first));
  ErrorReporter::ReportError("rel(tuple(...)), trel(tuple(...)) or "
                              "orel(tuple(...)) expected");
  return nl->TypeError();
}
/*

5.5.2 Value mapping function of operator ~feed~

*/


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

      local.addr = rit;
      return 0;

    case REQUEST :
      rit = (GenericRelationIterator*)local.addr;
      Tuple *t;
      if ((t = rit->GetNextTuple()) != 0)
      {
        result.setAddr(t);
        return YIELD;
      }
      else
      {
        return CANCEL;
      }

    case CLOSE :
      if(local.addr)
      {
         rit = (GenericRelationIterator*)local.addr;
         delete rit;
         local.addr = 0;
      }
      return 0;
  }
  return 0;
}


#else

// version with support for progress queries


class FeedLocalInfo: public ProgressLocalInfo
{
public:
  FeedLocalInfo() : rit(0) {}
  ~FeedLocalInfo() {
     if (rit) {
        delete rit; rit = 0;
     }
  }

  GenericRelationIterator* rit;
};



int
Feed(Word* args, Word& result, int message, Word& local, Supplier s)
{
  GenericRelation* r=0;
  FeedLocalInfo* fli=0;
  Supplier sonOfFeed;


  switch (message)
  {
    case OPEN :{
      r = (GenericRelation*)args[0].addr;

      fli = (FeedLocalInfo*) local.addr;
      if ( fli ) delete fli;

      fli = new FeedLocalInfo();
      fli->returned = 0;
      fli->total = r->GetNoTuples();
      fli->rit = r->MakeScan();
      local.setAddr(fli);
      return 0;
    }
    case REQUEST :{
      fli = (FeedLocalInfo*) local.addr;
      Tuple *t;
      if ((t = fli->rit->GetNextTuple()) != 0)
      {
        fli->returned++;
        result.setAddr(t);
        return YIELD;
      }
      else
      {
        return CANCEL;
      }
    }
    case CLOSE :{
        // Note: object deletion is handled in OPEN and CLOSEPROGRESS
        // keep the local info structure since it may still be
        // needed for handling progress messages.
      fli = static_cast<FeedLocalInfo*>(local.addr);
      if(fli){
        if(fli->rit){
          delete fli->rit;
          fli->rit=0;
        }
      }
      return 0;
    }
    case CLOSEPROGRESS:{

      sonOfFeed = qp->GetSupplierSon(s, 0);
      fli = (FeedLocalInfo*) local.addr;
      if ( fli )
      {
         delete fli;
         local.setAddr(0);
      }
      return 0;

    }
    case REQUESTPROGRESS :{

      GenericRelation* rr;
      rr = (GenericRelation*)args[0].addr;


      ProgressInfo p1;
      ProgressInfo *pRes=0;

      // Determination of constants in file bin/UpdateProgressConstants

      static const double uFeed = 
        ProgressConstants::getValue("Relation-C++", "feed", "uFeed");
          // 0.00296863;  milliseconds per tuple
      static const double vFeed = 
        ProgressConstants::getValue("Relation-C++", "feed", "vFeed");
          // 0.0010081649; milliseconds per attribute

      pRes = (ProgressInfo*) result.addr;
      fli = (FeedLocalInfo*) local.addr;
      sonOfFeed = qp->GetSupplierSon(s, 0);

      if ( fli )
      {
        fli->sizesChanged = false;

        if ( !fli->sizesInitialized )
        {
          fli->noAttrs = nl->ListLength(nl->Second(nl->Second(qp->GetType(s))));
          fli->attrSize = new double[fli->noAttrs];
          fli->attrSizeExt = new double[fli->noAttrs];

          fli->Size =  0;
          fli->SizeExt =  0;

          for ( int i = 0;  i < fli->noAttrs; i++)
          {
            fli->attrSize[i] = rr->GetTotalSize(i) / (fli->total + 0.001);
            fli->attrSizeExt[i] = rr->GetTotalExtSize(i) / (fli->total + 0.001);

            fli->Size += fli->attrSize[i];
            fli->SizeExt += fli->attrSizeExt[i];
          }
          fli->sizesInitialized = true;
          fli->sizesChanged = true;
        }
      }

      if ( qp->IsObjectNode(sonOfFeed) )
      {
        if ( !fli ) {
           return CANCEL;
        } else  //an object node, fli defined
        {
          pRes->Card = (double) fli->total;
          pRes->CopySizes(fli);  //copy all sizes

          pRes->Time = (fli->total + 1) * (uFeed + fli->noAttrs * vFeed);

          //any time value created must be > 0; so we add 1

          pRes->Progress = fli->returned * (uFeed + fli->noAttrs * vFeed)
            / pRes->Time;

          pRes->BTime = 0.001; //time must not be 0

          pRes->BProgress = 1.0;

          return YIELD;
        }
      }
      else //not an object node
      {
        if ( qp->RequestProgress(sonOfFeed, &p1) )
        {
          pRes->Card = p1.Card;

          pRes->CopySizes(p1);

          pRes->Time = p1.Time + p1.Card * (uFeed + p1.noAttrs * vFeed);

          pRes->Progress =
             ((p1.Progress * p1.Time) +
              (fli ? fli->returned : 0) * (uFeed + p1.noAttrs * vFeed))
              / pRes->Time;

          pRes->BTime = p1.Time;

          pRes->BProgress = p1.Progress;

          return YIELD;
        } else {
          return CANCEL;
        }
      }
    }
  }
  return 0;
}


struct FeedProjectInfo : OperatorInfo {

  FeedProjectInfo() : OperatorInfo()
  {
    name =      "feedproject";

    signature = "rel(tuple(a_1 ... a_n)) x (a_i1 ... a_ik)\n"
                "-> stream(tuple(a_i1 ... a_ik))";
    syntax =    "_ feedproject[ _ ]";
    meaning =   "Creates a stream of tuples from a given relation "
                " and project them to the given list of attributes.";
    example =   "plz feedproject[PLZ] count";

    supportsProgress = true;
  }

};



ListExpr feedproject_tm(ListExpr args)
{
  NList l(args);

  const string opName = "feedproject";
  const string arg1 = "rel(tuple(...)";
  const string arg2 = "list of unique symbols (a_1 ... a_k)";

  string err1("");
  if ( !l.checkLength(2, err1) )
    return l.typeError( err1 );

  NList attrs;
  if ( !l.first().checkRel( attrs ) )
    return l.typeError(1, arg1);

  //cerr << "a1 " << attrs << endl;

  NList atoms = l.second();
  if ( !l.checkSymbolList( atoms ) )
    return l.typeError(2, arg2);

  if ( !l.checkUniqueMembers( atoms ) )
    return l.typeError(2, arg2);


  NList indices;
  NList oldAttrs(attrs);
  NList newAttrs;
  int noAtoms = atoms.length();
  while ( !atoms.isEmpty() )
  {
    string attrname = atoms.first().str();
    ListExpr attrtype = nl->Empty();
    //cerr << "a2 " << attrs << endl;
    int newIndex = FindAttribute( oldAttrs.listExpr(), attrname, attrtype );

    if (newIndex > 0)
    {
      indices.append( NList(newIndex) );
      newAttrs.append( oldAttrs.elem(newIndex) );
    }
    else
    {
      ErrorReporter::ReportError(
        "Attributename '" + attrname +
        "' is not a known attributename in the tuple stream.");
          return nl->SymbolAtom(Symbol::TYPEERROR());
    }
    atoms.rest();
  }

  NList outlist = NList( NList(Symbol::APPEND()),
                         NList( NList( noAtoms ), indices ),
			 NList().tupleStreamOf( newAttrs ) );

  return outlist.listExpr();
}



class FeedProjLocalInfo : public FeedLocalInfo
{
  public:
    double argTupleSize;

    FeedProjLocalInfo(TupleType* ptr) :
      FeedLocalInfo(), argTupleSize(0), tt(ptr)
    {
       returned = 0;
    }
    ~FeedProjLocalInfo()
    {
       if ( tt->DeleteIfAllowed() ) {
	 tt = 0;
       }
    }

  private:
    TupleType* tt;
};


int
feedproject_vm(Word* args, Word& result, int message, Word& local, Supplier s)
{
  GenericRelation* r=0;
  FeedProjLocalInfo* fli=0;
  Supplier sonOfFeed;
  Word elem(Address(0));
  int noOfAttrs= 0;
  int index= 0;
  Supplier son;

  switch (message)
  {
    case OPEN :{
      r = (GenericRelation*)args[0].addr;

      fli = (FeedProjLocalInfo*) local.addr;
      if ( fli ) delete fli;

      TupleType* tt = new TupleType(nl->Second(GetTupleResultType(s)));

      fli = new FeedProjLocalInfo(tt);
      fli->total = r->GetNoTuples();
      fli->rit = r->MakeScan(tt);
      local.setAddr(fli);
      return 0;
    }
    case REQUEST :{
      fli = (FeedProjLocalInfo*) local.addr;
      Tuple *t;

        noOfAttrs = ((CcInt*)args[2].addr)->GetIntval();
        list<int> usedAttrs;

        for( int i = 0; i < noOfAttrs; i++)
        {
          son = qp->GetSupplier(args[3].addr, i);
          qp->Request(son, elem);
          index = ((CcInt*)elem.addr)->GetIntval();
	  //cerr << "ind = " << index << endl;
          usedAttrs.push_back(index-1);
        }


      if ((t = fli->rit->GetNextTuple(usedAttrs)) != 0)
      {
        fli->returned++;
        result.setAddr(t);
        return YIELD;
      }
      else
      {
        return CANCEL;
      }
    }
    case CLOSE :{
        // Note: object deletion is handled in OPEN and CLOSEPROGRESS
        // keep the local info structure since it may still be
        // needed for handling progress messages.
      fli = static_cast<FeedProjLocalInfo*>(local.addr);
      if(fli){
        if(fli->rit){
          delete fli->rit;
          fli->rit=0;
        }
      }
      return 0;
    }
    case CLOSEPROGRESS:{

      sonOfFeed = qp->GetSupplierSon(s, 0);
      fli = (FeedProjLocalInfo*) local.addr;
      if ( fli )
      {
         delete fli;
         local.setAddr(0);
      }
      return 0;

    }
    case REQUESTPROGRESS :{

      GenericRelation* rr;
      rr = (GenericRelation*)args[0].addr;


      ProgressInfo p1;
      ProgressInfo *pRes=0;
      const double uFeedProject = 0.002;     //millisecs per tuple
      const double vFeedProject = 0.000036;  //millisecs per byte input
      const double wFeedProject = 0.0018;    //millisecs per attr



      pRes = (ProgressInfo*) result.addr;
      fli = (FeedProjLocalInfo*) local.addr;
      sonOfFeed = qp->GetSupplierSon(s, 0);

      if ( fli )
      {
        fli->sizesChanged = false;

        if ( !fli->sizesInitialized )
        {
          fli->Size = 0;
          fli->SizeExt = 0;
          fli->noAttrs = ((CcInt*)args[2].addr)->GetIntval();
          fli->attrSize = new double[fli->noAttrs];
          fli->attrSizeExt = new double[fli->noAttrs];


	  fli->argTupleSize = rr->GetTotalExtSize()
		/ (fli->total + 0.001);

          for( int i = 0; i < fli->noAttrs; i++)
          {
            son = qp->GetSupplier(args[3].addr, i);
            qp->Request(son, elem);
            index = ((CcInt*)elem.addr)->GetIntval();
            fli->attrSize[i] = rr->GetTotalSize(index-1)
		/ (fli->total + 0.001);
            fli->attrSizeExt[i] = rr->GetTotalExtSize(index-1)
		/ (fli->total + 0.001);

            fli->Size += fli->attrSize[i];
            fli->SizeExt += fli->attrSizeExt[i];
            fli->sizesInitialized = true;
            fli->sizesChanged = true;
          }

        }
      }

      if ( qp->IsObjectNode(sonOfFeed) )
      {
        if ( !fli ) {
           return CANCEL;
        } else  //an object node, fli defined
        {
          pRes->Card = (double) fli->total;
          pRes->CopySizes(fli);  //copy all sizes

          pRes->Time = (fli->total + 1) *
		(uFeedProject
		+ fli->argTupleSize * vFeedProject
		+ fli->noAttrs * wFeedProject);

          //any time value created must be > 0; so we add 1

          pRes->Progress = fli->returned * (uFeedProject
		+ fli->argTupleSize * vFeedProject
		+ fli->noAttrs * wFeedProject)
            / pRes->Time;

          pRes->BTime = 0.001; //time must not be 0

          pRes->BProgress = 1.0;

          return YIELD;
        }
      } else { //not an object node
        if ( qp->RequestProgress(sonOfFeed, &p1) )
        {
          pRes->Card = p1.Card;

          pRes->CopySizes(p1);

          pRes->Time = p1.Time + p1.Card * (uFeedProject
		+ fli->argTupleSize * vFeedProject
		+ fli->noAttrs * wFeedProject);

          pRes->Progress =
             ((p1.Progress * p1.Time) +
              (fli ? fli->returned : 0) * (uFeedProject
		+ fli->argTupleSize * vFeedProject
		+ fli->noAttrs * wFeedProject))
              / pRes->Time;

          pRes->BTime = p1.Time;

          pRes->BProgress = p1.Progress;

          return YIELD;
        } else {
          return CANCEL;
        }
      }
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
  "( <text>rel(X) | trel(X) | orel(X,Y) -> stream(X)</text--->"
  "<text>_ feed</text--->"
  "<text>Produces a stream from a relation, temporal relation, or ordered "
  "relation by scanning the relation tuple by tuple.</text--->"
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

template<bool isOrel> 
ListExpr ConsumeTypeMap(ListExpr args)
{
  string expected = isOrel?"(stream(tuple(...)) (sortby_Ids)) expected":
                            "stream(tuple(...)) expected";
  if(nl->ListLength(args) != (isOrel?2:1)){
    ErrorReporter::ReportError(expected);
    return nl->TypeError();
  }

  ListExpr first = nl->First(args);

  if(!Stream<Tuple>::checkType(first)){
    return listutils::typeError(expected);
  }

  ListExpr attrlist = nl->Second(nl->Second(first));

  if(!listutils::checkAttrListForNamingConventions(attrlist)){
    return listutils::typeError("Some of the attributes does "
                      "not fit to Secondo's naming conventions");
  }

  if(!isOrel){
    // do not allow an arel  attribute for standard consume
    while(!nl->IsEmpty(attrlist)){
      ListExpr attr = nl->First(attrlist);
      attrlist = nl->Rest(attrlist);
      ListExpr type = nl->Second(attr);
      if( (nl->ListLength(type)==2) &&
          (nl->IsAtom(nl->First(type))) &&
          ( (nl->SymbolValue(nl->First(type))) == "arel")){
        ErrorReporter::ReportError("arel attributes cannot be processed"
                                    " with standard consume");
        return nl->TypeError();
      }
    }
  }

  if(!isOrel) {
    return nl->Cons(nl->SymbolAtom(Relation::BasicType()), nl->Rest(first));
  } else {
    if(!listutils::isKeyDescription(nl->Second(first),nl->Second(args))) {
      ErrorReporter::ReportError("all identifiers of second argument must "
                                  "appear in the first argument");
      return nl->TypeError();
    }
    ListExpr result = nl->ThreeElemList(nl->SymbolAtom(OREL),
                                        nl->Second(first),
                                        nl->Second(args));
    return result;
  }
}


ListExpr tconsume_tm(ListExpr args)
{

  if(nl->ListLength(args)!=1){
    return listutils::typeError("one argument expected");
  }
  if(!Stream<Tuple>::checkType(nl->First(args))){
    ErrorReporter::ReportError("stream(tuple(...)) expecetd");
    return nl->TypeError();
  }

  return nl->Cons(nl->SymbolAtom(TempRelation::BasicType()),
                  nl->Rest(nl->First(args)));
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

  GenericRelation* rel = (GenericRelation*)((qp->ResultStorage(s)).addr);
  if(rel->GetNoTuples() > 0)
  {
    rel->Clear();
  }

  Stream<Tuple> stream(args[0]);

  stream.open;

  Tuple* tup;
  while( (tup = stream.request()) != 0){
      rel->AppendTuple(tup);
      tup->DeleteIfAllowed();
  }
  stream.close();

  result.setAddr(rel);

  return 0;
}

#else


// Version with support for progress queries

struct consumeLocalInfo
{
  consumeLocalInfo(Word& w): state(0), current(0), stream(w){}

  int state;      //0 = working, 1 = finished
  int current;    //current no of tuples read
  Stream<Tuple> stream;

  ostream& print(ostream& o){
    o << "Localinfo (" << (void*) this << "), state=" << state << ", current="
      << current << ", stream=";
    stream.print(o);
    o << ".";
    return o;
  }

  private:
    consumeLocalInfo(): state(0), current(0),stream(0) {}
};


int
Consume(Word* args, Word& result, int message,
       Word& local, Supplier s)
{
  Word actual;
  consumeLocalInfo* cli;

  if ( message <= CLOSE )     //normal evaluation
  {

    cli = (consumeLocalInfo*) local.addr;
    if ( cli ) delete cli;    //needed if consume used in a loop

    cli = new consumeLocalInfo(args[0]);

    local.setAddr(cli);

    GenericRelation* rel = (GenericRelation*)((qp->ResultStorage(s)).addr);
    if(rel->GetNoTuples() > 0)
    {
      rel->Clear();
    }

    cli->stream.open();
    Tuple* tuple;
    while( (tuple = cli->stream.request())!=0){
      rel->AppendTuple(tuple);
      tuple->DeleteIfAllowed();
      cli->current++;
    }
    result.setAddr(rel);

    cli->stream.close();
    cli->state = 1;
    return 0;
  }

  else if ( message == REQUESTPROGRESS )
  {

    if(local.addr == 0){
      return CANCEL;
    }
    ProgressInfo p1;
    ProgressInfo* pRes;
    static const double uConsume = 
      ProgressConstants::getValue("Relation-C++", "consume", "uConsume");
        // 0.024; millisecs per tuple
    static const double vConsume = 
      ProgressConstants::getValue("Relation-C++", "consume", "vConsume");
        // 0.0003;  millisecs per byte in root/extension
    const double wConsume = 
      ProgressConstants::getValue("Relation-C++", "consume", "wConsume");
        // 0.001338; millisecs per byte in FLOB

    cli = (consumeLocalInfo*) local.addr;
    pRes = (ProgressInfo*) result.addr;

    if ( cli->stream.requestProgress( &p1) )
    {
      pRes->Card = p1.Card;
      pRes->CopySizes(p1);

      pRes->Time = p1.Time +
        p1.Card * (uConsume + p1.SizeExt * vConsume
          + (p1.Size - p1.SizeExt) * wConsume);

      if ( cli == 0 )    //not yet working
      {
        pRes->Progress = (p1.Progress * p1.Time) / pRes->Time;
      }
      else
      {
        if ( cli->state == 0 )  //working
        {

          if ( p1.BTime < 0.1 && pipelinedProgress )   //non-blocking,
                                                       //use pipelining
            pRes->Progress = p1.Progress;
          else
            pRes->Progress =
            (p1.Progress * p1.Time +
              cli->current *  (uConsume + p1.SizeExt * vConsume
                  + (p1.Size - p1.SizeExt) * wConsume) )
                / pRes->Time;
        }
        else       //finished
        {
          pRes->Progress = 1.0;
        }
      }

      pRes->BTime = pRes->Time;    //completely blocking
      pRes->BProgress = pRes->Progress;

      return YIELD;      //successful
    } else {
       return CANCEL;      //no progress available
    }
  }
  else if ( message == CLOSEPROGRESS )
  {
    cli = (consumeLocalInfo*) local.addr;
    if ( cli ){
       delete cli;
       local.setAddr(0);
    }
    return 0;
  }

  return 0;
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

const string OConsumeSpec =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" "
  "\"Example\" ) "
  "( <text>(stream (tuple(a1:t1 ... an:tn) x (ai aj ak) -> "
  "(orel (tuple(a1:t1 ... an:tn)) (ai aj ak))</text--->"
  "<text>_ oconsume [list]</text--->"
  "<text>Collects objects from a stream and sorts them according to the second"
  " argument.</text--->"
  "<text>query cities feed oconsume [BevT,Name]</text--->"
  ") )";


const string TConsumeSpec =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" "
  "\"Example\" ) "
  "( <text> stream(tuple(...)) -> trel(tuple(...))</text--->"
  "<text>_ tconsume </text--->"
  "<text> Appends the tuples' values into a tuple buffer."
         "The result cant be materialized, thus don't "
  " </text--->"
  "<text>query cities feed tconsume</text--->"
  ") )";


/*

5.6.4 Definition of operator ~consume~

*/
Operator relalgconsume (
   "consume",              // name
   ConsumeSpec,            // specification
   Consume,                // value mapping
   Operator::SimpleSelect, // trivial selection function
   ConsumeTypeMap<false>   // type mapping
);

Operator relalgoconsume (
   "oconsume",             // name
   OConsumeSpec,           // specification
   Consume,                // value mapping
   Operator::SimpleSelect, // trivial selection function
   ConsumeTypeMap<true>    // type mapping
);


Operator relalgtconsume (
   "tconsume",              // name
   TConsumeSpec,            // specification
   Consume,                // value mapping
   Operator::SimpleSelect, // trivial selection function
   tconsume_tm   // type mapping
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
  if(nl->ListLength(args)!=2){
    return listutils::typeError("two arguments expected");
  }
  ListExpr first = nl->First(args);
  if(!Tuple::checkType(first)){
    return listutils::typeError("First arguments must be  tuple(...)");
  }
  ListExpr second = nl->Second(args);
  if(!listutils::isSymbol(second)){
    return listutils::typeError("second arguments ust be an attribute name");
  }

  string name = nl->SymbolValue(second);
  ListExpr attrtype;
  int j = listutils::findAttribute(nl->Second(first),name,attrtype);
  if(j==0){
    ErrorReporter::ReportError("Attr name " + name +
                               " not found in attribute list");
    return nl->TypeError();
  }
  return nl->ThreeElemList(nl->SymbolAtom(Symbol::APPEND()),
                           nl->OneElemList(nl->IntAtom(j)),
                           attrtype);
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
  result.setAddr(tupleptr->GetAttribute(index - 1));
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

Type mapping function modified to show the possibility of getting
not only types but also arguments in the type mapping. This happens
when an operator
registers "UsesArgsinTypeMapping". Type list now has the form

----	( (type1 arg1) (type2 arg2) )
----

that is

----	(
		  ( (stream (tuple x))  arg1 )
		  ( (map (tuple x) bool)  arg2 )
		)
----

*/
ListExpr FilterTypeMap(ListExpr args)
{

  if(!nl->HasLength(args,2)){
    return listutils::typeError("two arguments expected");
  }

  if(!nl->HasLength(nl->First(args),2)){
    ErrorReporter::ReportError("the first argument "
                               " should be a (type, expression) pair");
    return nl->TypeError();
  }

  if(!listutils::isTupleStream(nl->First(nl->First(args)))){
    ErrorReporter::ReportError("first argument must be a stream of tuples");
    return nl->TypeError();
  }

  if(!nl->HasLength(nl->Second(args),2)){
    ErrorReporter::ReportError("the second argument "
                               " should be a (type, expression) pair");
    return nl->TypeError();
  }

  
  ListExpr map = nl->First(nl->Second(args));
  if(!listutils::isMap<1>(map)){ // checking for a single arg
     ErrorReporter::ReportError("map: tuple -> bool expected as the"
                                " second argument");
     return nl->TypeError();
  }

  ListExpr mapres = nl->Third(map);
  if(!CcBool::checkType(mapres)){
    ErrorReporter::ReportError("map is not a predicate");
    return nl->TypeError();
  } 

  if(!nl->Equal(nl->Second(nl->First(nl->First(args))), nl->Second(map))){
    ErrorReporter::ReportError("map and tuple type are not consistent");
    return nl->TypeError();
  }

  //just for demonstrating "UsesArgsInTypeMapping"

  bool showArguments = false;
  if ( showArguments ) {
        cout << "arguments to the filter operator:" << endl;
	    cout << "first argument: ";
		nl->WriteListExpr( nl->Second(nl->First(args)), cout, 2 );
		cout << endl;
	    cout << "second argument: ";
		nl->WriteListExpr( nl->Second(nl->Second(args)), cout, 2 );
        cout << endl;
		cout << endl;
  }



  return nl->First(nl->First(args));
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
        if (((Attribute*)funresult.addr)->IsDefined())
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
        result.setAddr(tuple);
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
  int current;    //tuples read
  int returned;    //tuples returned
  bool done;            //arg stream exhausted
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

      fli = (FilterLocalInfo*) local.addr;
      if ( fli ) delete fli;

      fli = new FilterLocalInfo;
      fli->current = 0;
      fli->returned = 0;
      fli->done = false;
      local.setAddr(fli);

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
        if (((Attribute*)funresult.addr)->IsDefined())
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
        result.setAddr(tuple);
        return YIELD;
      }
      else
          {
            fli->done = true;
        return CANCEL;
          }

    case CLOSE:

      // Note: object deletion is handled in (repeated) OPEN and CLOSEPROGRESS
      qp->Close(args[0].addr);
      return 0;


    case CLOSEPROGRESS:
      fli = (FilterLocalInfo*) local.addr;
      if ( fli )
      {
         delete fli;
         local.setAddr(0);
      }
      return 0;


    case REQUESTPROGRESS:

      ProgressInfo p1;
      ProgressInfo* pRes;
      const double uFilter = 1.0;

      pRes = (ProgressInfo*) result.addr;
      fli = (FilterLocalInfo*) local.addr;

      if ( qp->RequestProgress(args[0].addr, &p1) )
      {
        pRes->CopySizes(p1);

        if ( fli )    //filter was started
        {
                  if ( fli->done )      //arg stream exhausted, all known
                  {
                    pRes->Card = (double) fli->returned;
                        pRes->Time = p1.Time + (double) fli->current
                          * qp->GetPredCost(s) * uFilter;
                    pRes->Progress = 1.0;
                        pRes->CopyBlocking(p1);
                        return YIELD;
                  }

          if ( fli->returned >= enoughSuccessesSelection ||
		       fli->returned >= p1.Card * qp->GetSelectivity(s) )
            //stable state assumed now - or more returned than cold estimate
          {
            pRes->Card =  p1.Card *
              ( (double) fli->returned / (double) (fli->current));
            pRes->Time = p1.Time + p1.Card * qp->GetPredCost(s) * uFilter;

            if ( p1.BTime < 0.1 && pipelinedProgress )   //non-blocking,
                                                        //use pipelining
              pRes->Progress = p1.Progress;
            else
              pRes->Progress = (p1.Progress * p1.Time
                + fli->current * qp->GetPredCost(s) * uFilter) / pRes->Time;

            pRes->CopyBlocking(p1);
            return YIELD;
          }
        }
    //filter not yet started or not enough seen

        pRes->Card = p1.Card * qp->GetSelectivity(s);
        pRes->Time = p1.Time + p1.Card * qp->GetPredCost(s) * uFilter;

        if ( p1.BTime < 0.1 && pipelinedProgress )   //non-blocking,
                                                        //use pipelining
          pRes->Progress = p1.Progress;
        else
          pRes->Progress = (p1.Progress * p1.Time) / pRes->Time;
          pRes->CopyBlocking(p1);
        return YIELD;
      } else {
        return CANCEL;
      }
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

  if(nl->ListLength(args)!=3){
    ErrorReporter::ReportError("three arguments expected");
    return nl->TypeError();
  }
  ListExpr first = nl->First(args);
  ListExpr second  = nl->Second(args);
  ListExpr third  = nl->Third(args);

  if(!listutils::isTupleStream(first)){
    ErrorReporter::ReportError("first arguments must be a tuple stream");
    return nl->TypeError();
  }

  if(!listutils::isMap<1>(second)){
    ErrorReporter::ReportError("second argument must be a map");
    return nl->TypeError();
  }

  if(!nl->IsEqual(nl->Third(second),CcBool::BasicType())){
    ErrorReporter::ReportError("resulttype of the map (2. arg) must be bool");
    return nl->TypeError();
  }

  if(!nl->Equal(nl->Second(first), nl->Second(second))){
    ErrorReporter::ReportError("different typle types in stream and map");
    return nl->TypeError();
  }

  if(!nl->IsEqual(third,CcInt::BasicType())){
    ErrorReporter::ReportError("third argument must be of type int");
    return nl->TypeError();
  }

  return first;
}

/*
5.8.2 Value mapping function of operator ~reduce~

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
        if (((Attribute*)funresult.addr)->IsDefined())
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
      if(local.addr){
         delete ri;
         local.setAddr(0);
      }
      return 0;
  }
  return 0;
}




/*
5.9 Operator ~project~

5.9.1 Type mapping function of operator ~project~

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

  if(nl->ListLength(args)!=2){
    ErrorReporter::ReportError("tuplestream x arglist expected");
    return nl->TypeError();
  }
  first = nl->First(args);

  if(!listutils::isTupleStream(first)){
    ErrorReporter::ReportError("first argument has to be a tuple stream");
    return nl->TypeError();
  }

  second = nl->Second(args);

  if(nl->ListLength(second)<=0){
    ErrorReporter::ReportError("non empty attribute name list"
                               " expected as second argument");
    return nl->TypeError();
  }

  noAttrs = nl->ListLength(second);
  set<string> attrNames;
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
      return nl->SymbolAtom(Symbol::TYPEERROR());
    }
    if(attrNames.find(attrname)!=attrNames.end()){
       ErrorReporter::ReportError("names within the projection "
                                  "list are not unique");
       return nl->TypeError();
    } else {
       attrNames.insert(attrname);
    }

    j = listutils::findAttribute(nl->Second(nl->Second(first)),
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
          return nl->SymbolAtom(Symbol::TYPEERROR());
    }
  }
  outlist =
    nl->ThreeElemList(
      nl->SymbolAtom(Symbol::APPEND()),
      nl->TwoElemList(
        nl->IntAtom(noAttrs),
        numberList),
      nl->TwoElemList(
        nl->SymbolAtom(Symbol::STREAM()),
        nl->TwoElemList(
          nl->SymbolAtom(Tuple::BasicType()),
          newAttrList)));
  return outlist;
}

/*
5.9.2 Value mapping function of operator ~project~

*/

#ifndef USE_PROGRESS

// standard version


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
        result.setAddr(t);
        return YIELD;
      }
      else return CANCEL;
    }
    case CLOSE :
    {
      qp->Close(args[0].addr);
      if(local.addr)
      {
        ((TupleType *)local.addr)->DeleteIfAllowed();
        local.setAddr(0);
      }
      return 0;
    }
  }
  return 0;
}

# else

// progress version



class ProjectLocalInfo: public ProgressLocalInfo
{
public:

  ProjectLocalInfo() {
    tupleType = 0;
    read = 0;
  }

  ~ProjectLocalInfo() {
    tupleType->DeleteIfAllowed();
    tupleType = 0;
  }

  TupleType *tupleType;
};


int
Project(Word* args, Word& result, int message,
        Word& local, Supplier s)
{
  ProjectLocalInfo *pli=0;
  Word elem1(Address(0));
  Word elem2(Address(0));
  int noOfAttrs= 0;
  int index= 0;
  Supplier son;

  switch (message)
  {
    case OPEN:{

      pli = (ProjectLocalInfo*) local.addr;
      if ( pli ) delete pli;

      pli = new ProjectLocalInfo();
      pli->tupleType = new TupleType(nl->Second(GetTupleResultType(s)));
      local.setAddr(pli);

      qp->Open(args[0].addr);
      return 0;
    }
    case REQUEST:{

      pli = (ProjectLocalInfo*) local.addr;

      qp->Request(args[0].addr, elem1);
      if (qp->Received(args[0].addr))
      {
        pli->read++;
        Tuple *t = new Tuple( pli->tupleType );

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
        result.setAddr(t);
        return YIELD;
      }
      else return CANCEL;
    }
    case CLOSE: {

      // Note: object deletion is done in repeated OPEN or CLOSEPROGRESS
      qp->Close(args[0].addr);
      return 0;

    }
    case CLOSEPROGRESS:{
      pli = (ProjectLocalInfo*) local.addr;
      if ( pli ){
         delete pli;
         local.setAddr(0);
      }
      return 0;
    }

    case REQUESTPROGRESS:{

      ProgressInfo p1;
      ProgressInfo *pRes;
      const double uProject = 0.00073;  //millisecs per tuple
      const double vProject = 0.0004;  //millisecs per tuple and attribute


      pRes = (ProgressInfo*) result.addr;
      pli = (ProjectLocalInfo*) local.addr;

      if ( !pli ) {
         return CANCEL;
      }

      if ( qp->RequestProgress(args[0].addr, &p1) )
      {
        pli->sizesChanged = false;

        if ( !pli->sizesInitialized )
        {
          pli->noAttrs = ((CcInt*)args[2].addr)->GetIntval();
          pli->attrSize = new double[pli->noAttrs];
          pli->attrSizeExt = new double[pli->noAttrs];
        }

        if ( !pli->sizesInitialized || p1.sizesChanged )
        {
          pli->Size = 0;
          pli->SizeExt = 0;

          for( int i = 0; i < pli->noAttrs; i++)
          {
            son = qp->GetSupplier(args[3].addr, i);
            qp->Request(son, elem2);
            index = ((CcInt*)elem2.addr)->GetIntval();
            pli->attrSize[i] = p1.attrSize[index-1];
            pli->attrSizeExt[i] = p1.attrSizeExt[index-1];
            pli->Size += pli->attrSize[i];
            pli->SizeExt += pli->attrSizeExt[i];
          }
          pli->sizesInitialized = true;
          pli->sizesChanged = true;
        }

        pRes->Card = p1.Card;
        pRes->CopySizes(pli);

        pRes->Time = p1.Time + p1.Card * (uProject + pli->noAttrs * vProject);

    //only pointers are copied; therefore the tuple sizes do not
    //matter

        if ( p1.BTime < 0.1 && pipelinedProgress )   //non-blocking,
                                                        //use pipelining
          pRes->Progress = p1.Progress;
        else
          pRes->Progress =
            (p1.Progress * p1.Time +
              pli->read * (uProject + pli->noAttrs * vProject))
            / pRes->Time;

  pRes->CopyBlocking(p1);    //non-blocking operator

        return YIELD;
      } else {
        return CANCEL;
      }
    }
    default : cerr << "unknown message" << endl;
              return 0;

  }
  return 0;
}

#endif



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
2.5 Operator ~remove~

2.5.1 Type mapping function of operator ~remove~

Result type of ~remove~ operation.

----  ((stream (tuple ((x1 T1) ... (xn Tn)))) (ai1 ... aik))  ->

    (APPEND
      (n-k (j1 ... jn-k))
      (stream (tuple ((aj1 Tj1) ... (ajn-k Tjn-k))))
    )
----

The type mapping computes the number of attributes and the list of
attribute numbers for the given left attributes (after removal) and
asks the query processor to append it to the given arguments.

*/
ListExpr RemoveTypeMap(ListExpr args)
{
  bool firstcall = true;
  int noAttrs=0, j=0;

  // initialize all ListExpr with the empty list
  ListExpr first = nl->TheEmptyList();
  ListExpr second = first,
           first2 = first,
           attrtype = first,
           newAttrList = first,
           lastNewAttrList = first,
           lastNumberList = first,
           numberList = first,
           outlist = first;

  string attrname="", argstr="";
  set<int> removeSet;
  removeSet.clear();

  if(nl->ListLength(args)!=2){
    ErrorReporter::ReportError("tuple stream x attrlist expected");
    return nl->TypeError();
  }

  first = nl->First(args);
  second = nl->Second(args);

  if(!listutils::isTupleStream(first)){
    ErrorReporter::ReportError("first argument is not a tuple stream");
    return nl->TypeError();
  }

  if(nl->ListLength(second)<=0){
    ErrorReporter::ReportError("non empty attr list "
                              "expected as second argumnet");
    return nl->TypeError();
  }


  while (!(nl->IsEmpty(second)))
  {
    first2 = nl->First(second);
    second = nl->Rest(second);
    nl->WriteToString(argstr, first2);
    if (nl->AtomType(first2) == SymbolType)
    {
      attrname = nl->SymbolValue(first2);
    }
    else
    {
      nl->WriteToString(argstr, first2);
      ErrorReporter::ReportError("Operator remove gets '" + argstr +
      "' as attributename.\n"
      "Atrribute name may not be the name of a Secondo object!");
      return nl->SymbolAtom(Symbol::TYPEERROR());
    }

    j = listutils::findAttribute(nl->Second(nl->Second(first)),
                                 attrname, attrtype);
    if (j)  removeSet.insert(j);
    else
    {
      nl->WriteToString( argstr, nl->Second(nl->Second(first)) );
      ErrorReporter::ReportError(
        "Attributename '" + attrname + "' is not known.\n"
        "Known Attribute(s): " + argstr);
      return nl->SymbolAtom(Symbol::TYPEERROR());
    }
  }
  // ** here: we need to generate new attr list according to
  // ** removeSet
  ListExpr oldAttrList;
  int i;
  i=0;  // i is the index of the old attriblist
  first = nl->First(args);
  second = nl->Second(args);
  oldAttrList=nl->Second(nl->Second(first));
  noAttrs =0;
  while (!(nl->IsEmpty(oldAttrList)))
  {
    i++;
    first2 = nl->First(oldAttrList);
    oldAttrList = nl->Rest(oldAttrList);

    if (removeSet.find(i)==removeSet.end())
    // the attribute is not in the removal list
    {
      noAttrs++;
      if (firstcall)
      {
        firstcall = false;
        newAttrList = nl->OneElemList(first2);
        lastNewAttrList = newAttrList;
        numberList = nl->OneElemList(nl->IntAtom(i));
        lastNumberList = numberList;
      }
      else
      {
        lastNewAttrList = nl->Append(lastNewAttrList, first2);
        lastNumberList = nl->Append(lastNumberList, nl->IntAtom(i));
      }
    }
  }

  if (noAttrs>0)
  {
    outlist = nl->ThreeElemList(
              nl->SymbolAtom(Symbol::APPEND()),
              nl->TwoElemList(nl->IntAtom(noAttrs), numberList),
              nl->TwoElemList(nl->SymbolAtom(Symbol::STREAM()),
              nl->TwoElemList(nl->SymbolAtom(Tuple::BasicType()),
                       newAttrList)));
    return outlist;
  }
  else
  {
    ErrorReporter::ReportError("Do not remove all attributes!");
    return nl->SymbolAtom(Symbol::TYPEERROR());
  }
}

/*
2.5.2 Value mapping function of operator ~remove~

The value mapping is the same as for project. The difference is treated in the type mapping.



2.5.3 Specification of operator ~remove~

*/
const string RemoveSpec  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                           "\"Example\" ) "
                           "( <text>((stream (tuple ((x1 T1) ... "
                           "(xn Tn)))) (ai1 ... aik)) -> (stream "
                           "(tuple ((aj1 Tj1) ... (ajn-k Tjn-k))))"
                           "</text--->"
                           "<text>_ remove [list]</text--->"
                           "<text>Produces a removal tuple for each "
                           "tuple of its input stream.</text--->"
                           "<text>query cities feed remove[zipcode] "
                           "consume</text--->"
                              ") )";

/*
2.5.4 Definition of operator ~remove~

*/
Operator relalgremove (
         "remove",                // name
         RemoveSpec,              // specification
         Project,                  // value mapping
         Operator::SimpleSelect,  // trivial selection function
         RemoveTypeMap            // type mapping
);




/*
5.10 Operator ~product~

5.10.1 Type mapping function of operator ~product~

Result type of product operation.

----  ((stream (tuple (x1 ... xn))) (stream (tuple (y1 ... ym))))
        -> (stream (tuple (x1 ... xn y1 ... ym)))
----

The right argument stream will be materialized.

*/
ListExpr ProductTypeMap(ListExpr args)
{
  if(nl->ListLength(args)!=2){
    ErrorReporter::ReportError("tweo arguments expected");
    return nl->TypeError();
  }
  ListExpr first = nl->First(args);
  ListExpr second = nl->Second(args);

  if(!listutils::isTupleStream(first) ||
     !listutils::isTupleStream(second)){
    ErrorReporter::ReportError(" stream(tuple(...)) x "
                               "stream(tuple(...)) expected");
    return nl->TypeError();
  }

  ListExpr list1 = nl->Second(nl->Second(first));
  ListExpr list2 = nl->Second(nl->Second(second));

  ListExpr list = ConcatLists(list1, list2);

  if(!listutils::isAttrList(list)){
    ErrorReporter::ReportError("name conflict in concatenated tuples");
    return nl->TypeError();
  }
  ListExpr  outlist = nl->TwoElemList(nl->SymbolAtom(Symbol::STREAM()),
                                      nl->TwoElemList(
                                      nl->SymbolAtom(Tuple::BasicType()),list));
  return outlist;
}

/*
5.10.2 Value mapping function of operator ~product~

*/

#ifndef USE_PROGRESS

// standard version


struct ProductLocalInfo
{
  TupleType *resultTupleType;
  Tuple* currentTuple;
  TupleBuffer *rightRel;
  GenericRelationIterator *iter;
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

      long MAX_MEMORY = (qp->GetMemorySize(s) * 1024 * 1024);
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

      local.setAddr(pli);
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
          result.setAddr(resultTuple);
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
            rightTuple = pli->iter->GetNextTuple();
            assert( rightTuple != 0 );
            resultTuple = new Tuple( pli->resultTupleType );
            Concat(pli->currentTuple, rightTuple, resultTuple);
            rightTuple->DeleteIfAllowed();
            result.setAddr(resultTuple);
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
      if(pli)
      {
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
        local.setAddr(0);
      }

      qp->Close(args[0].addr);
      qp->Close(args[1].addr);

      return 0;
    }
  }
  return 0;
}

# else

// progress version


class ProductLocalInfo: public ProgressLocalInfo
{
public:

  ProductLocalInfo() :
    resultTupleType(0),
    currentTuple(0),
    rightRel(0),
    iter(0)
  {}

  ~ProductLocalInfo()
  {
    if(currentTuple != 0)
      currentTuple->DeleteIfAllowed();
    if( iter != 0 )
      delete iter;
    resultTupleType->DeleteIfAllowed();
    if( rightRel )
    {
      rightRel->Clear();
      delete rightRel;
    }
  }

  TupleType *resultTupleType;
  Tuple* currentTuple;
  TupleBuffer *rightRel;
  GenericRelationIterator *iter;
};

int
Product(Word* args, Word& result, int message,
        Word& local, Supplier s)
{
  Word r, u;
  ProductLocalInfo* pli;

  pli = (ProductLocalInfo*)local.addr;

  switch (message)
  {
    case OPEN:
    {
      long MAX_MEMORY = (qp->GetMemorySize(s) * 1024 * 1024); // in bytes
      cmsg.info("RA:ShowMemInfo")
        << "Product.MAX_MEMORY ("
        << MAX_MEMORY/1024 << " kB): " << endl;
      cmsg.send();

      if ( pli ) delete pli;
      pli = new ProductLocalInfo;

      qp->Open(args[0].addr);
      qp->Request(args[0].addr, r);

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

      pli->readSecond = 0;
      pli->returned = 0;
      local.setAddr(pli);

      while(qp->Received(args[1].addr))
      {
        Tuple *t = (Tuple*)u.addr;
        pli->rightRel->AppendTuple( t );
        t->DeleteIfAllowed();
        qp->Request(args[1].addr, u);
        pli->readSecond++;
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

      return 0;
    }

    case REQUEST:
    {
      Tuple *resultTuple, *rightTuple;

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
          result.setAddr(resultTuple);
          pli->returned++;
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
            rightTuple = pli->iter->GetNextTuple();
            assert( rightTuple != 0 );
            resultTuple = new Tuple( pli->resultTupleType );
            Concat(pli->currentTuple, rightTuple, resultTuple);
            rightTuple->DeleteIfAllowed();
            result.setAddr(resultTuple);
            pli->returned++;
            return YIELD;
          }
          else
          {
            return CANCEL; // left stream exhausted
          }
        }
      }
    }

    case CLOSE:
    {
      // Note: object deletion is done in (repeated) OPEN and CLOSEPROGRESS
      qp->Close(args[0].addr);
      qp->Close(args[1].addr);

      return 0;
    }


    case CLOSEPROGRESS:
    {
      if ( pli ){
         delete pli;
         local.setAddr(0);
      }
      return 0;
    }


    case REQUESTPROGRESS:
    {
      ProgressInfo p1, p2;
      ProgressInfo *pRes;
      const double uProduct = 0.0003; //millisecs per byte (right input)
              //if writing to disk
      const double vProduct = 0.000042; //millisecs per byte (total output)
          //if reading from disk

      pRes = (ProgressInfo*) result.addr;

      if (!pli) {
         return CANCEL;
      }

      if (qp->RequestProgress(args[0].addr, &p1)
       && qp->RequestProgress(args[1].addr, &p2))
      {
        pli->SetJoinSizes(p1, p2);

        pRes->Card = p1.Card * p2.Card;
        pRes->CopySizes(pli);

        pRes->Time = p1.Time + p2.Time +
          p2.Card * p2.Size * uProduct +
          p1.Card * p2.Card * pRes->Size * vProduct;

        pRes->Progress =
          (p1.Progress * p1.Time + p2.Progress * p2.Time +
           pli->readSecond * p2.Size * uProduct +
           pli->returned * pRes->Size * vProduct)
          / pRes->Time;


        pRes->BTime = p1.BTime + p2.BTime +
          p2.Card * p2.Size * uProduct;

        pRes->BProgress =
          (p1.BProgress * p1.BTime + p2.BProgress * p2.BTime +
           pli->readSecond * p2.Size * uProduct)
          / pRes->BTime;

        return YIELD;
      } else {
        return CANCEL;
      }
    }
  }
  return 0;
}

#endif


/*
5.10.3 Specification of operator ~product~

*/
const string ProductSpec  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" "
  "\"Example\" \"Remark\" ) "
  "( <text>((stream (tuple (x1 ... xn))) (stream "
  "(tuple (y1 ... ym)))) -> (stream (tuple (x1 "
  "... xn y1 ... ym)))</text--->"
  "<text>_ _ product</text--->"
  "<text>Computes a Cartesian product stream from "
  "its two argument streams.</text--->"
  "<text>query ten feed twenty feed product count"
  "</text--->"
  "<text>The right argument stream (2nd argument) "
  "will be materialized.</text--->"
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

  if(nl->ListLength(args)!=1){
    ErrorReporter::ReportError("one argument expected");
    return nl->TypeError();
  }
  ListExpr arg = nl->First(args);
  if(!listutils::isRelDescription(arg,true) &&
     !listutils::isRelDescription(arg,false) &&
     !listutils::isOrelDescription(arg) &&
     !listutils::isTupleStream(arg)){
    ErrorReporter::ReportError( "rel(tuple(...)), orel(tuple(...),(...)), "
                                " (trel(tuple(...))"
                                " or stream(tuple(...)) expected");
    return nl->TypeError();
  }
  return nl->SymbolAtom(CcInt::BasicType());
}


struct CountBothInfo : OperatorInfo {

  CountBothInfo() : OperatorInfo()
  {
    name =      "countboth";
    signature = "stream(tuple(y)) x stream(tuple(z)) -> int";
    syntax =    "_ _ countboth";
    meaning =   "Counts the number of tuples of two input streams. "
          "The streams are requested alternately. The purpose of "
    "this operator is to expose the overhead of the seek time.";
    example =   "plz feed orte feed countboth";
  }

};



ListExpr
countboth_tm(ListExpr args)
{
  if(nl->ListLength(args)!=2){
    ErrorReporter::ReportError("two arguments expected");
    return nl->TypeError();
  }

  if(!listutils::isTupleStream(nl->First(args)) ||
     !listutils::isTupleStream(nl->Second(args))){
    ErrorReporter::ReportError(" two tuple stream expected");
    return nl->TypeError();
  }

  return nl->SymbolAtom(CcInt::BasicType());
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
  GenericRelation* rel = (GenericRelation*)args[0].addr;
  result = qp->ResultStorage(s);
  ((CcInt*) result.addr)->Set(true, rel->GetNoTuples());
  return 0;
}

#else

// progress version

int
TCountStream(Word* args, Word& result, int message,
             Word& local, Supplier s)
{
  Word elem;
  int count = 0;


  if ( message <= CLOSE )
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
  else if ( message == REQUESTPROGRESS )
  {
    ProgressInfo p1;
    ProgressInfo* pRes;

    pRes = (ProgressInfo*) result.addr;

    if ( qp->RequestProgress(args[0].addr, &p1) )
    {
      pRes->Copy(p1);
      return YIELD;
    } else {
      return CANCEL;
    }
  }
  else if ( message == CLOSEPROGRESS )
  {
    return 0;
  }

  return 0;
}



int
TCountRel(Word* args, Word& result, int message,
          Word& local, Supplier s)
{
  Supplier sonOfCount;
  sonOfCount = qp->GetSupplierSon(s, 0);

  if ( message <= CLOSE )     //normal evaluation
  {
    GenericRelation* rel = (GenericRelation*)args[0].addr;
    result = qp->ResultStorage(s);
    ((CcInt*) result.addr)->Set(true, rel->GetNoTuples());
    return 0;
  }

  else if ( message == REQUESTPROGRESS )
  {

    ProgressInfo p1;
    ProgressInfo* pRes;

    pRes = (ProgressInfo*) result.addr;

    if ( qp->IsObjectNode(sonOfCount) )
    {
      return CANCEL;
    }
    else
    {
      if ( qp->RequestProgress(sonOfCount, &p1) )
      {
        pRes->Copy(p1);
        return YIELD;
      } else {
        return CANCEL;
      }
    }
  }
  else if ( message == CLOSEPROGRESS && !qp->IsObjectNode(sonOfCount) )
  {
    return 0;
  }
  return 0;
}

int
countboth_vm( Word* args, Word& result, int message,
              Word& local, Supplier s )
{

  struct Counter
  {
    Counter() : count(0) {}

    inline bool request(void* addr)
    {
       Word elem;
       qp->Request(addr, elem);
       bool ok = qp->Received(addr);
       if (ok) {
   count++;
         static_cast<Tuple*>(elem.addr)->DeleteIfAllowed();
       }
       return ok;
    }

    int count;

  };

  qp->Open(args[0].addr);
  qp->Open(args[1].addr);

  Counter c;

  bool recA = c.request(args[0].addr);
  bool recB = c.request(args[1].addr);

  while ( recA || recB )
  {
    if (recA) {
      recA = c.request(args[0].addr);
    }
    if (recB) {
      recB = c.request(args[1].addr);
    }
  }
  result = qp->ResultStorage(s);
  static_cast<CcInt*>(result.addr)->Set(true, c.count);

  qp->Close(args[0].addr);
  qp->Close(args[1].addr);
  return 0;
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
"( <text>stream(tuple X) -> int, \n"
"rel(tuple(X) | orel(tuple(X) | trel(tuple(X) -> int"
"</text--->"
"<text>_ count</text--->"
"<text>Count number of tuples within a stream "
"or a relation (persistent, temporal, or ordered) of tuples.</text--->"
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
  else if( TypeOfRelAlgSymbol(nl->First(first)) == rel
           || TypeOfRelAlgSymbol(nl->First(first)) == trel
           || listutils::isOrelDescription(first))
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
"or a relation of tuples. During computation messages are sent to the "
"client. The purpose of this operator is to demonstrate the "
"programming interfaces only.'"
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

  if(nl->ListLength(args)!=1){
    ErrorReporter::ReportError("one argument expected");
    return nl->TypeError();
  }
  ListExpr first = nl->First(args);
  if(!listutils::isTupleStream(first) &&
     !listutils::isRelDescription(first)){
    ErrorReporter::ReportError("tuple stream or relation expected");
    return nl->TypeError();
  }
  return nl->SymbolAtom(CcInt::BasicType());
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
  if(count > 0){
    ((CcInt*) result.addr)->Set( true, (int)(totalSize / count) );
  } else {
    ((CcInt*) result.addr)->Set( false, 0 );
  }
  qp->Close(args[0].addr);
  return 0;
}

int
RootTupleSizeRel(Word* args, Word& result, int message,
                 Word& local, Supplier s)
{
  Relation* rel = (Relation*)args[0].addr;
  result = qp->ResultStorage(s);
  long count = rel->GetNoTuples();
  double totalsize = rel->GetTotalRootSize();
  if(count > 0){
    ((CcInt*) result.addr)->Set(true, (int)(totalsize / count) );
  } else {
    ((CcInt*) result.addr)->Set(false, 0 );
  }
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
  if(nl->ListLength(args)!=1){
    ErrorReporter::ReportError("one argument expected");
    return nl->TypeError();
  }
  ListExpr first = nl->First(args);
  if(!listutils::isTupleStream(first) &&
     !listutils::isRelDescription(first)){
    ErrorReporter::ReportError("tuple stream or relation expected");
    return nl->TypeError();
  }
  return nl->SymbolAtom(CcReal::BasicType());
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
  if( count > 0 ){
    ((CcReal*) result.addr)->Set(true, (float)(size/count));
  } else {
    ((CcReal*) result.addr)->Set(false, 0);
  }
  qp->Close(args[0].addr);
  return 0;
}

int
ExtTupleSizeRel(Word* args, Word& result, int message,
                Word& local, Supplier s)
{
  Relation* rel = (Relation*)args[0].addr;
  result = qp->ResultStorage(s);
  long count = rel->GetNoTuples();
  double totalsize = rel->GetTotalExtSize();
  if( count > 0 ){
    ((CcReal*) result.addr)->Set(true, (float)(totalsize/count));
  } else {
    ((CcReal*) result.addr)->Set(false, 0.0);
  }
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

----    (rel    (tuple x))                 -> real
        (stream  (tuple x))                 -> real
----

*/
ListExpr
TupleSizeTypeMap(ListExpr args)
{
  if(nl->ListLength(args)!=1){
    ErrorReporter::ReportError("one argument expected");
    return nl->TypeError();
  }
  ListExpr first = nl->First(args);
  if(!listutils::isTupleStream(first) &&
     !listutils::isRelDescription(first,false) &&
     !listutils::isRelDescription(first,true)){
    ErrorReporter::ReportError("tuple stream or relation expected");
    return nl->TypeError();
  }
  return nl->SymbolAtom(CcReal::BasicType());
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
  double size = 0;

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

  if (count > 0){
    ((CcReal*) result.addr)->Set(true, (double)(size/count));
  } else {
    ((CcReal*) result.addr)->Set(false, 0.0);
  }
  qp->Close(args[0].addr);
  return 0;
}

int
TupleSizeRel(Word* args, Word& result, int message,
             Word& local, Supplier s)
{
  Relation* rel = (Relation*)args[0].addr;
  result = qp->ResultStorage(s);
  long count = rel->GetNoTuples();
  double totalsize = rel->GetTotalSize();
  if(count > 0){
    ((CcReal*) result.addr)->Set(true, (double)(totalsize/count) );
  } else {
      ((CcReal*) result.addr)->Set(false, 0.0);
  }
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

  if(nl->ListLength(args)!=2){
    ErrorReporter::ReportError("two arguments expected");
    return nl->TypeError();
  }

  ListExpr first = nl->First(args);
  ListExpr second = nl->Second(args);

  if(!listutils::isRelDescription(first) &&
     !listutils::isTupleStream(first)){
   ErrorReporter::ReportError("rel(tuple(...)) or stream(tuple(...))"
                              " x ident expected");
   return nl->TypeError();
  }

  if(nl->AtomType(second)!=SymbolType){
    ErrorReporter::ReportError("invalid attribut name as second arg");
    return nl->TypeError();
  }
  string attrname = nl->SymbolValue(second);
  ListExpr attrtype;
  int j = listutils::findAttribute(nl->Second(nl->Second(first)),
                                   attrname, attrtype);
  if (!j)
  {
    ErrorReporter::ReportError(
      "Operator rootattrsize: Attribute name '" + attrname +
      "' is not a known attribute name in the attribute list.");
     return nl->TypeError();
  }

  return
    nl->ThreeElemList(
      nl->SymbolAtom(Symbol::APPEND()),
      nl->OneElemList(nl->IntAtom(j)),
      nl->SymbolAtom(CcInt::BasicType()));
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
  if( count > 0 ){
    ((CcInt*) result.addr)->Set( true, (long)(totalSize / count) );
  } else {
    ((CcInt*) result.addr)->Set( false, 0 );
  }
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
  long count = rel->GetNoTuples();
  double totalsize = rel->GetTotalRootSize(i);
  if( count > 0 ){
    ((CcInt*) result.addr)->Set(true, (long)(totalsize/count) );
  } else {
    ((CcInt*) result.addr)->Set(false, 0 );
  }
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
  if(nl->ListLength(args)!=2){
    ErrorReporter::ReportError("two arguments expected");
    return nl->TypeError();
  }

  ListExpr first = nl->First(args);
  ListExpr second = nl->Second(args);

  if(!listutils::isRelDescription(first) &&
     !listutils::isTupleStream(first)){
   ErrorReporter::ReportError("rel(tuple(...)) or stream(tuple(...))"
                              " x ident expected");
   return nl->TypeError();
  }

  if(nl->AtomType(second)!=SymbolType){
    ErrorReporter::ReportError("invalid attribut name as second arg");
    return nl->TypeError();
  }
  string attrname = nl->SymbolValue(second);
  ListExpr attrtype;
  int j = listutils::findAttribute(nl->Second(nl->Second(first)),
                                   attrname, attrtype);
  if (!j)
  {
    ErrorReporter::ReportError(
      "Operator rootattrsize: Attribute name '" + attrname +
      "' is not a known attribute name in the attribute list.");
     return nl->TypeError();
  }

  return
    nl->ThreeElemList(
      nl->SymbolAtom(Symbol::APPEND()),
      nl->OneElemList(nl->IntAtom(j)),
      nl->SymbolAtom(CcReal::BasicType()));
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
  if( count > 0 ){
    ((CcReal*) result.addr)->Set( true, (double)(totalSize / count) );
  } else {
    ((CcReal*) result.addr)->Set( false, 0.0 );
  }
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
  long count = rel->GetNoTuples();
  double totalsize = rel->GetTotalExtSize(i);
  if( count > 0 ){
    ((CcReal*) result.addr)->Set(true, (double)(totalsize / count) );
  } else {
    ((CcReal*) result.addr)->Set(false, 0.0 );
  }
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
  if(nl->ListLength(args)!=2){
    ErrorReporter::ReportError("two arguments expected");
    return nl->TypeError();
  }

  ListExpr first = nl->First(args);
  ListExpr second = nl->Second(args);

  if(!listutils::isRelDescription(first) &&
     !listutils::isTupleStream(first)){
   ErrorReporter::ReportError("rel(tuple(...)) or stream(tuple(...))"
                              " x ident expected");
   return nl->TypeError();
  }

  if(nl->AtomType(second)!=SymbolType){
    ErrorReporter::ReportError("invalid attribut name as second arg");
    return nl->TypeError();
  }
  string attrname = nl->SymbolValue(second);
  ListExpr attrtype;
  int j = listutils::findAttribute(nl->Second(nl->Second(first)),
                                   attrname, attrtype);
  if (!j)
  {
    ErrorReporter::ReportError(
      "Operator rootattrsize: Attribute name '" + attrname +
      "' is not a known attribute name in the attribute list.");
     return nl->TypeError();
  }

  return
    nl->ThreeElemList(
      nl->SymbolAtom(Symbol::APPEND()),
      nl->OneElemList(nl->IntAtom(j)),
      nl->SymbolAtom(CcReal::BasicType()));

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
  if( count > 0){
    ((CcReal*) result.addr)->Set( true, (double)(totalSize / count) );
  } else {
    ((CcReal*) result.addr)->Set( false, 0.0 );
  }
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
  long count = rel->GetNoTuples();
  double totalsize = rel->GetTotalSize(i);
  if( count > 0 ){
    ((CcReal*) result.addr)->Set(true, (double)(totalsize / count) );
  } else {
    ((CcReal*) result.addr)->Set(false, 0.0 );
  }
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
      if(pi)
      {
        pi->computeSizes();
        delete pi;
        local.setAddr(0);
      }
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
    meaning =   "Appends the tuples' values to a file specified by f. "
                "The attribute values are separated by s.";
    example =   "plz feed dumpstream[\"plz.txt\", \"|\"] count";
  }

};


/*
5.12.1 Type mapping

The type mapping uses the wrapper class ~NList~ which hides calls
to class NestedList. Moreover, there are some useful functions for
handling streams of tuples.

(stream(tuple(x))) -> (APPEND (list of attr names) stream(tuple(x))


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

  NList attrNames;
  while ( !attrs.isEmpty() ) {
    cout << attrs.first().first() << endl;
    attrNames.append( attrs.first().first().toStringAtom() );
    attrs.rest();
  }


// the following line can be compiled using newer gcc compilers (4.1.x),
// to be compatible with older ones, this line is replaced by
// two lines.
//  NList result( NList(Symbols::APPEND()), attrNames, l.first() );
   NList tmp(Symbols::APPEND());
   NList result(tmp,attrNames, l.first() );



  return result.listExpr();
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
    int maxAttr;
    int ctr;

    Info(const string& fileName, const string& sep, int max) :
      colsep(sep)
    {
      maxAttr = max;
      ctr = 0;
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
        if (i < maxAttr - 1)
          os << colsep;
        else
          os << endl;
      }
    }

    void appendToHeadLine(Word w)
    {
      if (ctr == maxAttr)
        return;

      string attrStr = StdTypes::GetString( w );
      //cerr << attrStr << endl;
      os << attrStr;
      if (ctr < maxAttr - 1)
        os << colsep;
      else
        os << endl;
      ctr++;
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

       int max = qp->GetNoSons( s );
       pi = new Info(name, colsep, max-3);
       local.addr = pi;

       for (int i=3; i < max; i++)
       {
   pi->appendToHeadLine( args[i] );
       }
       return 0;
     }

     case REQUEST: {

      Word elem;
      qp->Request(stream, elem);
      if ( qp->Received(stream) )
      {
        Tuple* t = static_cast<Tuple*>(elem.addr);
        pi->appendTuple(*t);
        result = elem;
        return YIELD;
      }
      else
      {
        return CANCEL;
      }
    }

    case CLOSE: {
      if(pi)
      {
        delete pi;
        local.setAddr(0);
      }
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
           listn=first, lastlistn=first, tup, tupFirst, type;
  string  attrname="", argstr="";
  string  attrnamen="";
  bool firstcall = true;


  if(nl->ListLength(args)!=2){
   return listutils::typeError("Operator rename expects "
                               "two arguments.");
  }

  first = nl->First(args);
  second  = nl->Second(args);

  nl->WriteToString(argstr, first);
  if (!IsStreamDescription(first)) {
    ErrorReporter::ReportError(
    "Operator rename expects a valid tuple stream "
    "Operator rename gets a list with structure '" + argstr + "'.");
    return nl->TypeError();
  }

  nl->WriteToString(argstr, second);

  if(!listutils::isSymbol(second)){
    return listutils::typeError("the second argument must be a symbol");
  }

  tup = nl->Second(nl->Second(first));
  while (!(nl->IsEmpty(tup)))
  {
    tupFirst = nl->First(tup);
    type = nl->Second(tupFirst);
    if (!(nl->IsAtom(type)))
    {
      type = nl->First(type);
      if (nl->IsAtom(type)&&(nl->SymbolValue(type) == "arel"))
          return nl->SymbolAtom(Symbol::TYPEERROR());
    }
    tup = nl->Rest(tup);
  }

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
    nl->TwoElemList(nl->SymbolAtom(Symbol::STREAM()),
    nl->TwoElemList(nl->SymbolAtom(Tuple::BasicType()),listn));
}

ListExpr RenameAttrTypeMap(ListExpr args){


  if(nl->ListLength(args)!=2){
     return listutils::typeError("Two arguments expected.");
  }

  cerr << __PRETTY_FUNCTION__ << ": args = " << nl->ToString(args) << endl;

  ListExpr firstarg = nl->First(args);
  if(nl->ListLength(firstarg) !=2){
    return listutils::typeError("First argument must consists of 2 parts.");
  }

  ListExpr stream = nl->First(firstarg);
  if(!listutils::isTupleStream(stream)){
    return listutils::typeError("First argument must be a tuple stream.");
  }

  ListExpr secondarg = nl->Second(args);
  if(nl->ListLength(secondarg) !=2){
    return listutils::typeError("Second argument must consists of 2 parts.");
  }

  map<string,string> renameMap;
  ListExpr renamesTypes  = nl->First(secondarg);  // get type list!
  ListExpr renamesValues = nl->Second(secondarg); // get value list!
  ListExpr attrList = nl->Second(nl->Second(stream));

  if(nl->AtomType(renamesTypes)!=NoAtom){
    return listutils::typeError("Second argument must be a list of renamings.");
  }
  if(nl->AtomType(renamesValues)!=NoAtom){
    return listutils::typeError("Second argument must be a list of renamings.");
  }

  while(!nl->IsEmpty(renamesTypes) && !nl->IsEmpty(renamesValues)){
    ListExpr renameType = nl->First(renamesTypes);
    renamesTypes = nl->Rest(renamesTypes);
    ListExpr renameValue = nl->First(renamesValues);
    renamesValues = nl->Rest(renamesValues);
    if(nl->ListLength(renameType)!=2){
      return listutils::typeError("Each rename must consist of the "
                                  "old and the new attribute name.");
    }
    if(nl->ListLength(renameValue)!=2){
      return listutils::typeError("Each rename must consist of the "
      "old and the new attribute name.");
    }
    // determine NewAttrName
    string newname = "";
    if(listutils::isSymbol(nl->First(renameType),CcString::BasicType())){
      newname = nl->StringValue(nl->First(renameValue));
    } else if(listutils::isSymbol(nl->First(renameType))) {
      newname = nl->SymbolValue(nl->First(renameType));
    }
    // determine OldAttrName
    string oldname = "";
    if(listutils::isSymbol(nl->Second(renameType),CcString::BasicType())){
      oldname = nl->StringValue(nl->Second(renameValue));
    } else if(listutils::isSymbol(nl->Second(renameType))) {
      oldname = nl->SymbolValue(nl->Second(renameType));
    }

    // check if new name is a usuable identifier
    if( (oldname=="") || (newname=="") ){
      return listutils::typeError("Empty attribute name detected!");
    }
    string idcheckmsg = "";
    if( !SecondoSystem::GetCatalog()
                                ->IsValidIdentifier(newname,idcheckmsg,true) ) {
      return listutils::typeError("New attribute name "+idcheckmsg+".");
    }
    char f = newname[0];
    if(f<'A' || f>'Z'){
      return listutils::typeError("Attribute name '" + newname + 
                                  "' starts with a lower case");
    }
    if(renameMap.find(oldname)!=renameMap.end()){
      return listutils::typeError(  "Attribute name '"
                                   +oldname+"' renamed twice.");
    } else {
      renameMap[oldname] = newname;
    }
  }

  ListExpr resAttrList;
  string errmsg;
  if(!listutils::replaceAttributes(attrList, renameMap, resAttrList, errmsg)){
    return listutils::typeError(errmsg);
  }
  return nl->TwoElemList( nl->SymbolAtom(Symbol::STREAM()),
                          nl->TwoElemList(nl->SymbolAtom(Tuple::BasicType()),
                                          resAttrList));
}

/*
5.12.1 validateAttrNames

Changes the names attribute names of a tuple stream to start with an upper case.
If there are name conflicts resulting of this renaming, the names are extended by
a sequential number. It returns false, if the names does not start with an character
in a,...,z,A,...,Z. 

*/
bool validateName(string& name){
  char f = name[0];
  if(f<'A' ||  f>'Z'){
     f = (f - 'a') + 'A';
  }
  if(f<'A' || f>'Z'){
     return false;
  }
  name[0] = f;
  return true; 
}

ListExpr validateAttrTM(ListExpr args){
  string err = "tuple stream expected";
  if(!nl->HasLength(args,1)){
    return listutils::typeError(err);
  }
  ListExpr stream = nl->First(args);
  if(!Stream<Tuple>::checkType(stream)){
    return listutils::typeError(err);
  }
  // start building the result
  ListExpr res;
  ListExpr last;
  bool firstElem = true;
  ListExpr attrList = nl->Second(nl->Second(stream));

  set<string> usedNames;


  while(!nl->IsEmpty(attrList)){
     ListExpr first = nl->First(attrList);
     attrList = nl->Rest(attrList);
     string name = nl->SymbolValue(nl->First(first));
     if(!validateName(name)){
        return listutils::typeError("Attribute name found "
                       "which not starts with a character");
     } 
     if(usedNames.find(name)!=usedNames.end()){
        string basic = name + "_";
        int num = 0;
        do{
          name = basic + stringutils::int2str(num);
          num++;
        } while(usedNames.find(name)!=usedNames.end());
     }
     // now, we have a unique name 
     ListExpr at = nl->TwoElemList(nl->SymbolAtom(name), nl->Second(first));
     if(firstElem){
        res = nl->OneElemList(at);
        last = res;
        firstElem=false;
     } else {
        last = nl->Append(last,at); 
     }
  }
  return nl->TwoElemList(nl->SymbolAtom(Stream<Tuple>::BasicType()),
                  nl->TwoElemList(nl->SymbolAtom(Tuple::BasicType()),res));
}



/*

5.12.2 Value mapping function of operator ~rename~

*/
int Rename(Word* args, Word& result, int message,
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
        result.setAddr(tuple);
        return YIELD;
      }
      else return CANCEL;

    case CLOSE :

      qp->Close(args[0].addr);
      return 0;


    case CLOSEPROGRESS:
      return 0;


    case REQUESTPROGRESS:

      ProgressInfo p1;
      ProgressInfo *pRes;

      pRes = (ProgressInfo*) result.addr;

      if ( qp->RequestProgress(args[0].addr, &p1) )
      {
        pRes->Copy(p1);
        return YIELD;
      } else {
        return CANCEL;
      }

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



const string RenameAttrSpec  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" "
  "\"Example\" ) "
  "( <text> stream(tuple) x (NAME NEWNAME)* -> stream(tuple), "
  "where NAME and NEWNAME in {ident, string}"
  "</text--->"
  "<text>_ renameattr [ Newname1 : Oldname1 , .... ] "
  "</text--->"
  "<text>Renames the specified attributes. If string type is used for one of "
  "the NAME or NEWNAME, it must be a constant or database object! This allows "
  "for renaming attributes with forbidden names, such as operator names, type "
  "names or reserved word, as 'value' or 'type'.</text--->"
  "<text>query ten feed renameattr [ No : no ] consume "
  "</text--->"
  ") )";


const string validateAttrSpec  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" "
  "\"Example\" ) "
  "( <text> stream(tuple) -> stream(tuple), "
  "</text--->"
  "<text>_ validateAttr "
  "</text--->"
  "<text>Changes the attribute names to start with an upper case"
  "</text--->"
  "<text>query ten feed validateAttr consume "
  "</text--->"
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


Operator relalgrenameattr (
         "renameattr",               // name
         RenameAttrSpec,             // specification
         Rename,                 // value mapping
         Operator::SimpleSelect, // trivial selection function
         RenameAttrTypeMap           // type mapping
);



Operator relalgvalidateAttr (
         "validateAttr",               // name
         validateAttrSpec,             // specification
         Rename,                 // value mapping
         Operator::SimpleSelect, // trivial selection function
         validateAttrTM           // type mapping
);




/*
5.12 Operator ~buffer~ (Notation: !)

Collects a small initial number of tuples into a buffer before passing them to the successor. Useful for progress estimation.

5.12.1 Type mapping function of operator ~buffer~

Expects a stream of tuples and returns the same type.

----    (stream  x)                 -> ( stream x)
----

*/
ListExpr BufferTypeMap(ListExpr args)
{
  if(nl->ListLength(args)!=1){
     ErrorReporter::ReportError("one argument expected");
     return nl->TypeError();
  }
  ListExpr arg = nl->First(args);

  // tuple stream
  if(IsStreamDescription(arg) ){
    return arg;
  }
  // DATA stream
  ListExpr errorInfo = nl->OneElemList(nl->SymbolAtom("ERROR"));
  if( nl->ListLength(arg)==2 &&
      nl->IsEqual(nl->First(arg),Symbol::STREAM()) &&
      am->CheckKind(Kind::DATA(),nl->Second(arg),errorInfo)) {
     return arg;
  }
  ErrorReporter::ReportError("stream(tuple(x)), stream(DATA)"
                             "expected");
  return nl->TypeError();

}


ListExpr BufferTypeMap2(ListExpr args)
{
  if(nl->ListLength(args)!=1){
    ErrorReporter::ReportError("DATA expected");
    return nl->TypeError();
  }
  ListExpr arg = nl->First(args);
  // DATA
  ListExpr errorInfo = nl->OneElemList(nl->SymbolAtom("ERROR"));
  if(am->CheckKind(Kind::DATA(),arg,errorInfo)){
     return arg;
  }
  ErrorReporter::ReportError("DATA expected");
  return nl->TypeError();
}

/*


5.12.2 Value mapping function of operator ~buffer~

*/



const int BUFFERSIZE = 52;

template <class T>
class BufferLocalInfo
{
 public:
 int state;        //0 = initial, 1 = buffer filled,
       //2 = buffer empty again
 int noInBuffer;   //tuples read into buffer;
 int next;     //index of next tuple to be returned;
 T* buffer[BUFFERSIZE];
};


template<class T>
int Buffer(Word* args, Word& result, int message,
       Word& local, Supplier s)
{
  Word e;
  T* elem;
  BufferLocalInfo<T>* bli;

  bli = (BufferLocalInfo<T>*) local.addr;

  switch (message)
  {
    case OPEN :

      if ( bli ) delete bli;

      bli = new BufferLocalInfo<T>();
        bli->state = 0;
        bli->noInBuffer = 0;
        bli->next = -1;
      local.setAddr(bli);

      qp->Open(args[0].addr);
      return 0;

    case REQUEST :

      if ( bli->state == 2 )      //regular behaviour
          //put first to avoid overhead
      {
        qp->Request(args[0].addr,e);
        if (qp->Received(args[0].addr))
        {
          elem = (T*)e.addr;
          result.setAddr(elem);
          return YIELD;
        }
        else return CANCEL;
      }


      if ( bli->state == 0 )
      {
        //fill the buffer
        qp->Request(args[0].addr, e);
        while (qp->Received(args[0].addr) && bli->noInBuffer < BUFFERSIZE)
        {
          bli->buffer[bli->noInBuffer] = (T*)e.addr;
          bli->noInBuffer++;
          if ( bli->noInBuffer < BUFFERSIZE ) qp->Request(args[0].addr, e);
        }
        if ( bli->noInBuffer > 1 ){
           bli->state = 1;  //next from buffer
        } else {
           bli->state = 3;        //next cancel
        }

        //return first tuple from the buffer, if possible
        if ( bli->noInBuffer > 0 )
        {
          result.setAddr(bli->buffer[0]);
          bli->next = 1;
          return YIELD;
        } else {
          return CANCEL;
        }
      }
      else if ( bli->state == 1 )
      {
        //return one elem from the buffer

        result.setAddr(bli->buffer[bli->next]);
        bli->next++;
        if ( bli->next == bli->noInBuffer )
        {
          if ( bli->noInBuffer == BUFFERSIZE )
            bli->state = 2;      //next from stream
          else bli->state = 3;      //next cancel
        }
        return YIELD;
      }
      else {
        if ( bli->state == 3 ){
          return CANCEL;
        } else {
          cout << "Something terrible happened in the ! operator." << endl;
        }
      }
    case CLOSE :

      // Note: object deletion is done in (repeated) OPEN and CLOSEPROGRESS
      qp->Close(args[0].addr);
      return 0;


    case CLOSEPROGRESS:

      if ( bli )
      {
        delete bli;
        local.setAddr(0);
      }
      return 0;


    case REQUESTPROGRESS:

      ProgressInfo p1;
      ProgressInfo *pRes;

      pRes = (ProgressInfo*) result.addr;

      if ( qp->RequestProgress(args[0].addr, &p1) )
      {
        pRes->Copy(p1);
        return YIELD;
      } else {
        return CANCEL;
      }

  }
  return 0;
}


int Buffer2(Word* args, Word& result, int message,
       Word& local, Supplier s){
   result = qp->ResultStorage(s);
   Attribute* arg = static_cast<Attribute*>(args[0].addr);
   Attribute* res = static_cast<Attribute*>(result.addr);
   res->CopyFrom(arg);
   return 0;
}



/*

5.12.4 SelectionFunction and Value Mapping Array for operator ~buffer~


*/

ValueMapping BufferVM[] = {Buffer<Tuple>,Buffer<Attribute>};

int BufferSelect(ListExpr args){
  ListExpr arg = nl->First(args);
  ListExpr type = nl->Second(arg);
  if(nl->AtomType(type)==NoAtom &&
     nl->IsEqual(nl->First(type),Tuple::BasicType())){
     return 1;
  } else {
     return 0;
  }

}



/*

5.12.4 Specification of operator ~buffer~

*/

const string BufferSpec  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" "
  "\"Example\" ) "
  "( <text>stream(tuple(x)) -> stream(tuple(x)) \n"
  "        stream(DATA) -> stream(DATA>)</text--->"
  "<text>_ !</text--->"
  "<text>Collects a small number of tuples into"
  " a buffer before passing them to the successor."
  " Useful for progress estimation."
  "</text--->"
  "<text>query plz feed filter[.Ort = \"Hagen\"] ! count</text--->"
  ") )";

const string BufferSpec2  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" "
  "\"Example\" ) "
  "( <text>DATA -> DATA </text--->"
  "<text>_ !</text--->"
  "<text>Does nothing</text--->"
  "<text>query 1 ! </text--->"
  ") )";

/*


5.12.5 Definition of operator ~buffer~

*/
Operator relalgbuffer (
         "!",                 // name
         BufferSpec,             // specification
         2,
         BufferVM,                 // value mapping
         BufferSelect,     // trivial selection function
         BufferTypeMap           // type mapping
);


Operator relalgbuffer2 (
         "!",                 // name
         BufferSpec2,             // specification
         Buffer2,                 // value mapping
         Operator::SimpleSelect,     // trivial selection function
         BufferTypeMap2           // type mapping
);









/*
5.13 Operator ~getFileInfo~

Returns a text object with statistical information on all files used by the
relation.

The result has format ~file\_stat\_result~:

----
file_stat_result --> (file_stat_list)
file_stat_list   -->   epsilon
                     | file_statistics file_stat_list
file_statistics  -->   epsilon
                     | file_stat_field file_statistics
file_stat_field  --> ((keyname) (value))
keyname          --> string
value            --> string
----

5.13.1 TypeMapping of operator ~getFileInfo~

*/

ListExpr getFileInfoRelTypeMap(ListExpr args)
{
  if(     nl->ListLength(args)!=1                         // only 1 argument
      || (    !IsRelDescription( nl->First(args), true  )      // is trel
           && !IsRelDescription( nl->First(args), false ) ) ){ // is rel
    return NList::typeError(
          "Expected 1 single argument of type 'rel' or 'trel'.");
  }
  return NList(FText::BasicType()).listExpr();
}

/*

5.13.2 ValueMapping of operator ~getFileInfo~

*/

int getFileInfoRelValueMap(Word* args, Word& result, int message,
                        Word& local, Supplier s)
{
  result = qp->ResultStorage(s);
  FText* restext = (FText*)(result.addr);
  GenericRelation* rel = (GenericRelation*)(args[0].addr);
  SmiStatResultType resVector(0);

  if ( rel != 0 ){

    // start list
    string resString = "[[\n";
    // TupleCoreFile
    rel->GetTupleFileStats(resVector);
    for(SmiStatResultType::iterator i = resVector.begin();
        i != resVector.end(); ){
      resString += "\t[['" + i->first + "'],['" + i->second + "']]";
      if(++i != resVector.end()){
        resString += ",\n";
      } else {
        resString += "\n";
      }
    }
    resString += "],\n";
    resVector.clear();
    // TupleLOBFile
    rel->GetLOBFileStats(resVector);
    resString += "[\n";
    for(SmiStatResultType::iterator i = resVector.begin();
        i != resVector.end(); ){
      resString += "\t[['" + i->first + "'],['" + i->second + "']]";
      if(++i != resVector.end()){
        resString += ",\n";
      } else {
        resString += "\n";
      }
    }
    resString += "]\n";
    restext->Set(true,resString);
    // end of list
    resString += "]\n";
    restext->Set(true,resString);
  } else {
    restext->Set(false,"");
  }
  return 0;
}

/*
5.13.3 Specification of operator ~getFileInfo~

*/
const string getFileInfoRelSpec  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>(rel(tuple(x)) | trel(tuple(X)) -> text)</text--->"
  "<text>getFileInfo( _ )</text--->"
  "<text>Retrieve statistical infomation on the file(s) used by the relation "
  "instance.</text--->"
  "<text>query getFileInfo(plz)</text--->"
  ") )";


/*
5.13.4 Definition of operator ~getFileInfo~

*/
Operator relalggetfileinfo (
         "getFileInfo",            // name
         getFileInfoRelSpec,       // specification
         getFileInfoRelValueMap,   // value mapping
         Operator::SimpleSelect,   // trivial selection function
         getFileInfoRelTypeMap     // type mapping
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
    ConstructorFunctions<TmpRel> cf;
    cf.in = InTmpRel;
    cf.out = OutTmpRel;
    cf.create = CreateTmpRel;
    cf.deletion = DeleteTmpRel;
    cf.kindCheck = TRelKindCheck;

    ConstructorInfo ci;
    ci.name = TempRelation::BasicType();
    ci.signature = "->trel";
    ci.typeExample = "trel((tuple(A int)(B string)))";
    ci.listRep = "A list of tuples";
    ci.valueExample = "((1 \"Hello\") (2 \"World\") (3 \"!\"))";
    ci.remarks = "Type trel represents a temporary relation."
                 " It is only valid during query evaluation."
                 " It cannot be made persistent.";

    static TypeConstructor cpptrel(ci,cf);

    AddTypeConstructor( &cpptuple );
    AddTypeConstructor( &cpprel );
    AddTypeConstructor( &cpptrel );

    AddOperator(&relalgfeed);
    AddOperator(&relalgconsume);
    AddOperator(&relalgoconsume);
    AddOperator(&relalgTUPLE);
    AddOperator(&relalgTUPLE2);
    AddOperator(&relalgattr);
    AddOperator(&relalgfilter); relalgfilter.SetUsesArgsInTypeMapping();
    AddOperator(&relalgproject);
    AddOperator(&relalgremove);
    AddOperator(&relalgproduct); relalgproduct.SetUsesMemory();
    AddOperator(&relalgcount);
    AddOperator(&relalgcount2);
    AddOperator(&relalgroottuplesize);
    AddOperator(&relalgexttuplesize);
    AddOperator(&relalgtuplesize);
    AddOperator(&relalgrootattrsize);
    AddOperator(&relalgextattrsize);
    AddOperator(&relalgattrsize);
    AddOperator(&relalgrename);
    AddOperator(&relalgvalidateAttr);
    AddOperator(&relalgrenameattr);
    relalgrenameattr.SetUsesArgsInTypeMapping();
    AddOperator(&relalgbuffer);
    AddOperator(&relalgbuffer2);
    AddOperator(&relalggetfileinfo);

    AddOperator(&relalgtconsume);

    // More recent programming interface for registering operators
    AddOperator( SizeCountersInfo(), sizecounters_vm, sizecounters_tm );
    AddOperator( DumpStreamInfo(), dumpstream_vm, dumpstream_tm );
    AddOperator( ReduceInfo(), reduce_vm, reduce_tm );
    AddOperator( CountBothInfo(), countboth_vm, countboth_tm );
    AddOperator( FeedProjectInfo(), feedproject_vm, feedproject_tm );

    cpptuple.AssociateKind( Kind::TUPLE() );
    cpprel.AssociateKind( Kind::REL() );
    cpptrel.AssociateKind( Kind::REL() );


/*
Register operators which are able to handle progress messages

*/
#ifdef USE_PROGRESS
    relalgfeed.EnableProgress();
    relalgconsume.EnableProgress();
    relalgoconsume.EnableProgress();
    relalgtconsume.EnableProgress();
    relalgfilter.EnableProgress();
    relalgproject.EnableProgress();
    relalgremove.EnableProgress();
    relalgproduct.EnableProgress();
    relalgcount.EnableProgress();
    relalgrename.EnableProgress();
    relalgrenameattr.EnableProgress();
    relalgvalidateAttr.EnableProgress();
    relalgbuffer.EnableProgress();
#endif

  }
  ~RelationAlgebra() {};
};

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
  nl = nlRef;
  qp = qpRef;
  am = SecondoSystem::GetAlgebraManager();
  return (new RelationAlgebra());
}

