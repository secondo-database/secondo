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

[1] Implementation of Module Relation Algebra Main Memory

[1] Using Storage Manager Berkeley DB

June 1996 Claudia Freundorfer

May 2002 Frank Hoffmann port to C++

November 7, 2002 RHG Corrected the type mapping of ~tcount~.

November 30, 2002 RHG Introduced a function ~RelPersistValue~ instead of
~DefaultPersistValue~ which keeps relations that have been built in memory in a
small cache, so that they need not be rebuilt from then on.

December 2005, Victor Almeida deleted the deprecated algebra levels
(~executable~, ~descriptive~, and ~hibrid~). Only the executable
level remains. Models are also removed from type constructors.

[TOC]

2 Auxilary Functions

1.2 Function ~TypeOfRelAlgSymbol~

Transforms a list expression ~symbol~ into one of the values of
type ~RelationType~. ~Symbol~ is allowed to be any list. If it is not one
of these symbols, then the value ~error~ is returned.

*/

#include <set>
#include <time.h>

#include "OldRelationAlgebra.h"
#include "CPUTimeMeasurer.h"
#include "QueryProcessor.h"
#include "LogMsg.h"

extern NestedList* nl;
extern QueryProcessor* qp;

extern int ccTuplesCreated;
extern int ccTuplesDeleted;

extern TypeConstructor ccreltuple;
extern TypeConstructor ccrelprel;

// type mapping functions implemented in ExtRelationAlgebra.cpp
extern ListExpr GroupByTypeMap2(ListExpr args, const bool memoryImpl = false);

static CcRelationType CcTypeOfRelAlgSymbol (ListExpr symbol) 
{
  string s;

  if (nl->AtomType(symbol) == SymbolType)
  {
    s = nl->SymbolValue(symbol);
    if (s == "mrel"   ) return mrel;
    if (s == "mtuple" ) return mtuple;
    if (s == "stream" ) return mstream;
    if (s == "map"    ) return mmap;
    if (s == "bool"   ) return mbool;
  }
  return merror;
}
/*

1.3 Macro CHECK\_COND

This macro makes reporting errors in type mapping functions more convenient.

*/
#define CHECK_COND(cond, msg) \
  if(!(cond)) \
  {\
    ErrorReporter::ReportError(msg);\
    return nl->SymbolAtom("typeerror");\
  };
/*

5.6 Function ~FindAttribute~

Here ~list~ should be a list of pairs of the form (~name~,~datatype~).
The function ~FindAttribute~ determines whether ~attrname~ occurs as one of
the names in this list. If so, the index in the list (counting from 1)
is returned and the corresponding datatype is returned in ~attrtype~.
Otherwise 0 is returned. Used in operator ~attr~.

*/
int CcFindAttribute( ListExpr list, string attrname, ListExpr& attrtype, NestedList* nl)
{
  ListExpr first, rest;
  int j;
  string  name;

  if (nl->IsAtom(list))
    return 0;
  rest = list;
  j = 1;
  while (!nl->IsEmpty(rest))
  {
    first = nl->First(rest);
    rest = nl->Rest(rest);
    if ((nl->ListLength(first) == 2) &&
       (nl->AtomType(nl->First(first)) == SymbolType))
    {
      name = nl->SymbolValue(nl->First(first));
      if (name == attrname)
      {
        attrtype = nl->Second(first);
        return j;
      }
    }
    else
      return 0; // typeerror
    j++;
  }
  return 0; // attrname not found
}

/*

5.6 Function ~ConcatLists~

Concatenates two lists.

*/
ListExpr CcConcatLists( ListExpr list1, ListExpr list2)
{
  if (nl->IsEmpty(list1))
  {
    return list2;
  }
  else
  {
    return nl->Cons(nl->First(list1), CcConcatLists(nl->Rest(list1), list2));
  }
}

/*

5.6 Function ~IsTupleDescription~

Checks wether a ListExpression is of the form
((a1 t1) ... (ai ti)).

*/
bool CcIsTupleDescription(ListExpr a, NestedList* nl)
{
  ListExpr rest = a;
  ListExpr current;

  while(!nl->IsEmpty(rest))
  {
    current = nl->First(rest);
    rest = nl->Rest(rest);
    if((nl->ListLength(current) == 2)
      && (nl->IsAtom(nl->First(current)))
      && (nl->AtomType(nl->First(current)) == SymbolType)
      && (nl->IsAtom(nl->Second(current)))
      && (nl->AtomType(nl->Second(current)) == SymbolType))
    {
    }
    else
    {
      return false;
    }
  }
  return true;
}

/*

5.6 Function ~AttributesAreDisjoint~

Checks wether two ListExpressions are of the form
((a1 t1) ... (ai ti)) and ((b1 d1) ... (bj dj))
and wether the ai and the bi are disjoint.

*/
bool CcAttributesAreDisjoint(ListExpr a, ListExpr b)
{
  set<string> aNames;
  ListExpr rest = a;
  ListExpr current;

  while(!nl->IsEmpty(rest))
  {
    current = nl->First(rest);
    rest = nl->Rest(rest);
    if((nl->ListLength(current) == 2)
      && (nl->IsAtom(nl->First(current)))
      && (nl->AtomType(nl->First(current)) == SymbolType)
      && (nl->IsAtom(nl->Second(current)))
      && (nl->AtomType(nl->Second(current)) == SymbolType))
    {
      aNames.insert(nl->SymbolValue(nl->First(current)));
    }
    else
    {
      return false;
    }
  }
  rest = b;
  while(!nl->IsEmpty(rest))
  {
    ListExpr current = nl->First(rest);
    rest = nl->Rest(rest);
    if((nl->ListLength(current) == 2)
      && (nl->IsAtom(nl->First(current)))
      && (nl->AtomType(nl->First(current)) == SymbolType)
      && (nl->IsAtom(nl->Second(current)))
      && (nl->AtomType(nl->Second(current)) == SymbolType))
    {
      if(aNames.find(nl->SymbolValue(nl->First(current))) != aNames.end())
      {
        return false;
      }
    }
    else
    {
      return false;
    }
  }
  return true;
}

/*

4 Operators

4.2 Selection function for type operators

The selection function of a type operator always returns -1.

*/
static int CcTypeOperatorSelect(ListExpr args) 
{ 
  return -1; 
}


/*

6.1 Type Operator ~TUPLE~

Type operators are used only for inferring argument types of parameter
functions. They have a type mapping but no evaluation function.

6.1.1 Type mapping function of operator ~TUPLE~

Extract tuple type from a stream or relation type given as the first argument.

----    ((stream x) ...)                -> x
        ((mrel x)   ...)                -> x
----

*/
ListExpr CcTUPLETypeMap(ListExpr args)
{
  ListExpr first;
  if(nl->ListLength(args) >= 1)
  {
    first = nl->First(args);
    if(nl->ListLength(first) == 2  )
    {
      if ((CcTypeOfRelAlgSymbol(nl->First(first)) == mstream)  ||
          (CcTypeOfRelAlgSymbol(nl->First(first)) == mrel))
        return nl->Second(first);
    }
  }
  return nl->SymbolAtom("typeerror");
}
/*

4.1.3 Specification of operator ~TUPLE~

*/
const string CcTUPLESpec  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                          "\"Remarks\" ) "
                          "( <text>((stream x)...) -> x, ((mrel x)...) -> "
		  	  "x</text--->"
			  "<text>type operator</text--->"
			  "<text>Extract tuple type from a stream or "
			  "relation type given as the first argument."
			  "</text--->"
			  "<text>not for use with sos-syntax</text--->"
			  "  ) )";
/*

4.1.3 Definition of operator ~TUPLE~

*/
Operator ccrelTUPLE (
         "TUPLE",              // name
         CcTUPLESpec,            // specification
         0,                    // no value mapping
         CcTypeOperatorSelect,   // trivial selection function
         CcTUPLETypeMap          // type mapping
);
/*

6.1 Type Operator ~TUPLE2~

6.1.1 Type mapping function of operator ~TUPLE2~

Extract tuple type from a stream or relation type given as the second argument.

----    ((stream x) (stream y) ...)          -> y
        ((mrel x) (mrel y)     ...)          -> y
----

*/
ListExpr CcTUPLE2TypeMap(ListExpr args)
{
  ListExpr second;
  if(nl->ListLength(args) >= 2)
  {
    second = nl->Second(args);
    if(nl->ListLength(second) == 2  )
    {
      if ((CcTypeOfRelAlgSymbol(nl->First(second)) == mstream)  ||
          (CcTypeOfRelAlgSymbol(nl->First(second)) == mrel))
        return nl->Second(second);
    }
  }
  return nl->SymbolAtom("typeerror");
}
/*

4.1.3 Specification of operator ~TUPLE2~

*/
const string CcTUPLE2Spec  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                           "\"Remarks\" ) "
                           "( <text><text>((stream x) (stream y) ...) -> y, "
			   "((mrel x) (mrel y) ...) -> y</text--->"
			   "<text>type operator</text--->"
			   "<text>Extract tuple type from a stream or "
			   "relation"
			   " type given as the second argument.</text--->"
			   "<text>not for use with sos-syntax</text--->"
			   ") )";
/*

4.1.3 Definition of operator ~TUPLE2~

*/
Operator ccrelTUPLE2 (
         "TUPLE2",             // name
         CcTUPLE2Spec,           // specification
         0,                    // no value mapping
         CcTypeOperatorSelect,   // trivial selection function
         CcTUPLE2TypeMap         // type mapping
);

/*

6.1 Type Operator ~Group~

Type operators are used only for inferring argument types of parameter
functions. They have a type mapping but no evaluation function.

6.1.1 Type mapping function of operator ~group~

----  ((stream x))                -> (mrel x)
----

*/
ListExpr CcGroupTypeMap(ListExpr args)
{
  ListExpr first;
  ListExpr tupleDesc;

  if(!nl->IsAtom(args) && nl->ListLength(args) >= 1)
  {
    first = nl->First(args);
    if(!nl->IsAtom(first) && nl->ListLength(first) == 2  )
    {
      tupleDesc = nl->Second(first);
      if(CcTypeOfRelAlgSymbol(nl->First(first)) == mstream
        && (!nl->IsAtom(tupleDesc))
        && (nl->ListLength(tupleDesc) == 2)
        && CcTypeOfRelAlgSymbol(nl->First(tupleDesc)) == mtuple
        && CcIsTupleDescription(nl->Second(tupleDesc), nl))
        return
          nl->TwoElemList(
            nl->SymbolAtom("mrel"),
            tupleDesc);
    }
  }
  return nl->SymbolAtom("typeerror");
}
/*

4.1.3 Specification of operator ~Group~

*/
const string CcGroupSpec  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                          "\"Remarks\" ) "
                          "( <text>((stream x)) -> (mrel x)</text--->"
			  "<text>type operator</text--->"
			  "<text>Maps stream type to a relation.</text--->"
			  "<text>not for use with sos-syntax</text--->"
			  ") )";
/*

4.1.3 Definition of operator ~group~

*/
Operator ccrelgroup (
         "GROUP",              // name
         CcGroupSpec,            // specification
         0,                    // no value mapping
         CcTypeOperatorSelect,   // trivial selection function
         CcGroupTypeMap          // type mapping
);

/*

4.1 Operator ~feed~

Produces a stream from a relation by scanning the relation tuple by tuple.

4.1.1 Type mapping function of operator ~feed~

A type mapping function takes a nested list as argument. Its contents are
type descriptions of an operator's input parameters. A nested list describing
the output type of the operator is returned.

Result type of feed operation.

----	((mrel x))		-> (stream x)
----

*/
static ListExpr CcFeedTypeMap(ListExpr args)
{
  ListExpr first ;

  CHECK_COND(nl->ListLength(args) == 1,
    "Operator feed expects a list of length one.");
  first = nl->First(args);
  CHECK_COND(nl->ListLength(first) == 2,
    "Operator feed expects an argument of type mrel(...)");
  CHECK_COND(CcTypeOfRelAlgSymbol(nl->First(first)) == mrel,
    "Operator feed expects an argument of type mrel");
  return nl->Cons(nl->SymbolAtom("stream"), nl->Rest(first));
}
/*

4.1.2 Value mapping function of operator ~feed~

*/
static int
CcFeed(Word* args, Word& result, int message, Word& local, Supplier s)
{
  CcRel* r;
  CcRelIT* rit;
  Word argRelation;


  switch (message)
  {
    case OPEN :
      qp->Request(args[0].addr, argRelation);
      r = ((CcRel*)argRelation.addr);
      rit = r->MakeNewScan();

      local.addr = rit;
      return 0;

    case REQUEST :
      rit = (CcRelIT*)local.addr;
      if (!(rit->EndOfScan()))
      {
        result = SetWord(rit->GetTuple());
        rit->Next();
        return YIELD;
      }
      else
      {
        return CANCEL;
      }

    case CLOSE :
      rit = (CcRelIT*)local.addr;
      delete rit;
      return 0;
  }
  return 0;
}
/*

4.1.3 Specification of operator ~feed~

*/
const string CcFeedSpec  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                         "\"Example\" ) "
                         "( <text>(mrel x) -> (stream x)</text--->"
			 "<text>_ feed</text--->"
			 "<text>Produces a stream from a relation by "
			 "scanning the relation tuple by tuple.</text--->"
			 "<text>query cities feed consume</text--->"
			 ") )";
/*

4.1.3 Definition of operator ~feed~

Non-overloaded operators are defined by constructing a new instance of
class ~Operator~, passing all operator functions as constructor arguments.

*/
Operator ccrelfeed (
          "feed",                // name
          CcFeedSpec,              // specification
          CcFeed,                  // value mapping
          Operator::SimpleSelect,         // trivial selection function
          CcFeedTypeMap           // type mapping
);

/*

4.1 Operator ~sample~

Produces a stream representing a sample of a relation.

4.1.1 Function ~MakeRandomSubset~

Generates a random subset of the numbers 1 ... ~setSize~, the size
of which is ~subsetSize~. This function is needed for operator ~sample~.

The strategy for generating a random subset works as follows: The algorithm
maintains a set of already drawn numbers. The algorithm draws a new random
number (using the libc random number generator) and adds it to the set of
already drawn numbers, if it has not been
already drawn. This is repeated until the size of the drawn set equals
~subsetSize~. If ~subsetSize~ it not considerably smaller than ~setSize~, e.g.
~subsetSize~ = ~setSize~ - 1, this approach becomes very inefficient
or may even not terminate, because towards the end of the algorithm
it may take a very long (or infinitely long)
time until the random number generator hits one of the few numbers,
which have not been already drawn. Therefore, if ~subsetSize~ is more
than half of ~subSet~, we simple draw a subset of size
~setSize~ - ~subsetSize~ and take the complement of that set as result set.

*/
void
CcMakeRandomSubset(vector<int>& result, int subsetSize, int setSize)
{
  assert(subsetSize >= 1);
  assert(setSize >= 2);
  assert(setSize <= RAND_MAX);
  assert(setSize > subsetSize);

  set<int> drawnNumbers;
  set<int>::iterator iter;
  int drawSize;
  int nDrawn = 0;
  int i;
  int r;
  bool doInvert;

  result.resize(0);
  srand(time(0));

  if(((double)setSize) / ((double)subsetSize) <= 2)
  {
    doInvert = true;
    drawSize = setSize - subsetSize;
  }
  else
  {
    doInvert = false;
    drawSize = subsetSize;
  }

  while(nDrawn < drawSize)
  {
    r = rand();
    r = r % (setSize + 1);
    if(r == 0)
    {
      continue;
    }

    if(drawnNumbers.find(r) == drawnNumbers.end())
    {
      drawnNumbers.insert(r);
      ++nDrawn;
    }
  }

  if(doInvert)
  {
    for(i = 1; i <= setSize; ++i)
    {
      if(drawnNumbers.find(i) == drawnNumbers.end())
      {
        result.push_back(i);
      }
    }
  }
  else
  {
    for(iter = drawnNumbers.begin(); iter != drawnNumbers.end(); ++iter)
    {
      result.push_back(*iter);
    }
  }
}

/*

4.1.1 Type mapping function of operator ~sample~

A type mapping function takes a nested list as argument. Its contents are
type descriptions of an operator's input parameters. A nested list describing
the output type of the operator is returned.

Result type of feed operation.

----	((mrel x) int real)		-> (stream x)
----

*/
static ListExpr CcSampleTypeMap(ListExpr args)
{
  ListExpr first ;
  ListExpr minSampleSizeLE;
  ListExpr minSampleRateLE;

  CHECK_COND(nl->ListLength(args) == 3,
    "Operator sample expects a list of length three.");

  first = nl->First(args);
  minSampleSizeLE = nl->Second(args);
  minSampleRateLE = nl->Third(args);

  CHECK_COND(nl->ListLength(first) == 2,
    "Operator sample expects a relation as first argument.");
  CHECK_COND(CcTypeOfRelAlgSymbol(nl->First(first)) == mrel,
    "Operator sample expects a relation as first argument.");

  CHECK_COND(nl->IsAtom(minSampleSizeLE),
    "Operator sample expects an int as second argument.")
  CHECK_COND(nl->AtomType(minSampleSizeLE) == SymbolType,
    "Operator sample expects an int as second argument.")
  CHECK_COND(nl->SymbolValue(minSampleSizeLE) == "int",
    "Operator sample expects an int as second argument.");
  CHECK_COND(nl->IsAtom(minSampleRateLE),
    "Operator sample expects a real as third argument.")
  CHECK_COND(nl->AtomType(minSampleRateLE) == SymbolType,
    "Operator sample expects a real as third argument.")
  CHECK_COND(nl->SymbolValue(minSampleRateLE) == "real",
    "Operator sample expects a real as third argument.");

  return nl->Cons(nl->SymbolAtom("stream"), nl->Rest(first));
}
/*

4.1.2 Value mapping function of operator ~sample~

*/
struct CcSampleLocalInfo
{
  vector<int> sampleIndices;
  vector<int>::iterator iter;
  int lastIndex;
  CcRelIT* relIT;
};

static int
CcSample(Word* args, Word& result, int message, Word& local, Supplier s)
{
  CcSampleLocalInfo* localInfo;
  Word argRelation;
  Word sampleSizeWord;
  Word sampleRateWord;

  CcRel* rel;
  CcTuple* tuple;

  int sampleSize;
  int relSize;
  float sampleRate;
  int i;
  int currentIndex;

  switch(message)
  {
    case OPEN :
      localInfo = new CcSampleLocalInfo();
      local = SetWord(localInfo);

      qp->Request(args[0].addr, argRelation);
      qp->Request(args[1].addr, sampleSizeWord);
      qp->Request(args[2].addr, sampleRateWord);

      rel = (CcRel*)argRelation.addr;
      relSize = rel->GetNoTuples();
      localInfo->relIT = rel->MakeNewScan();
      sampleSize = ((CcInt*)sampleSizeWord.addr)->GetIntval();
      sampleRate = ((CcReal*)sampleRateWord.addr)->GetRealval();

      if(sampleSize < 1)
      {
        sampleSize = 1;
      }
      if(sampleRate <= 0.0)
      {
        sampleRate = 0.0;
      }
      else if(sampleRate > 1.0)
      {
        sampleRate = 1.0;
      }
      if((int)(sampleRate * (float)relSize) > sampleSize)
      {
        sampleSize = (int)(sampleRate * (float)relSize);
      }

      if(relSize <= sampleSize)
      {
        for(i = 1; i <= relSize; ++i)
        {
          localInfo->sampleIndices.push_back(i);
        }
      }
      else
      {
        CcMakeRandomSubset(localInfo->sampleIndices, sampleSize, relSize);
      }

      localInfo->iter = localInfo->sampleIndices.begin();
      localInfo->lastIndex = 0;
      return 0;

    case REQUEST:
      localInfo = (CcSampleLocalInfo*)local.addr;
      if(localInfo->iter == localInfo->sampleIndices.end())
      {
        return CANCEL;
      }
      else
      {
        currentIndex = *(localInfo->iter);
        if(!localInfo->relIT->EndOfScan())
        {
          tuple = localInfo->relIT->GetTuple();
          localInfo->relIT->Next();
        }
        else
        {
          return CANCEL;
        }

        /* Advance iterator to the the next tuple belonging to the sample */
        for(i = 1; i < currentIndex - localInfo->lastIndex; ++i)
        {
          tuple->DeleteIfAllowed();
          if(!localInfo->relIT->EndOfScan())
          {
            tuple = localInfo->relIT->GetTuple();
            localInfo->relIT->Next();
          }
          else
          {
            return CANCEL;
          }
        }

        result = SetWord(tuple);
        localInfo->lastIndex = *(localInfo->iter);
        localInfo->iter++;
        return YIELD;
      }

    case CLOSE :
      localInfo = (CcSampleLocalInfo*)local.addr;
      delete localInfo->relIT;
      delete localInfo;
      return 0;
  }
  return 0;
}
/*

4.1.3 Specification of operator ~sample~

*/
const string CcSampleSpec  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                           "\"Example\" ) "
                           "( <text>(mrel x) int real -> (stream x)"
			   "</text--->"
			   "<text>_ sample [ _  , _ ]</text--->"
			   "<text>Produces a random sample of a relation."
			   " The sample size is min(relSize, "
			   "max(s, t * relSize)), where relSize is the size"
			   " of the argument relation, s is the second "
			   "argument, and t the third.</text--->"
			  "<text>query cities sample[0, 0.45] count</text--->"
			   ") )";

/*

4.1.3 Definition of operator ~sample~

Non-overloaded operators are defined by constructing a new instance of
class ~Operator~, passing all operator functions as constructor arguments.

*/
Operator ccrelsample (
          "sample",                // name
          CcSampleSpec,              // specification
          CcSample,                  // value mapping
          Operator::SimpleSelect,         // trivial selection function
          CcSampleTypeMap           // type mapping
);

/*
4.1 Operator ~consume~

Collects objects from a stream into a relation.

4.1.1 Type mapping function of operator ~consume~

Operator ~consume~ accepts a stream of tuples and returns a relation.


----    (stream  x)                 -> (mrel x)
----

*/
ListExpr CcConsumeTypeMap(ListExpr args)
{
  ListExpr first ;

  if(nl->ListLength(args) == 1)
  {
    first = nl->First(args);
    if ((nl->ListLength(first) == 2) &&
        (CcTypeOfRelAlgSymbol(nl->First(first)) == mstream) &&
        (nl->ListLength(nl->Second(first)) == 2) &&
        (CcTypeOfRelAlgSymbol(nl->First(nl->Second(first))) == mtuple))
      return nl->Cons(nl->SymbolAtom("mrel"), nl->Rest(first));
  }
  ErrorReporter::ReportError("Incorrect input for operator consume.");
  return nl->SymbolAtom("typeerror");
}
/*

4.1.2 Value mapping function of operator ~consume~

*/
static int
CcConsume(Word* args, Word& result, int message, Word& local, Supplier s)
{
  Word actual;
  CcRel* rel;

  rel = (CcRel*)((qp->ResultStorage(s)).addr);
  if( rel->GetNoTuples() > 0 )
    rel->Empty();

  qp->Open(args[0].addr);
  qp->Request(args[0].addr, actual);
  while (qp->Received(args[0].addr))
  {
    CcTuple* tuple = (CcTuple*)actual.addr;
    tuple = tuple->CloneIfNecessary();
    tuple->SetFree(false);
    rel->AppendTuple(tuple);
    qp->Request(args[0].addr, actual);
  }

  result = SetWord((void*) rel);

  qp->Close(args[0].addr);

  return 0;
}
/*

4.1.3 Specification of operator ~consume~

*/
const string CcConsumeSpec  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                            "\"Example\" ) "
                            "( <text>(stream x) -> (mrel x)</text--->"
			    "<text>_ consume</text--->"
			    "<text>Collects objects from a stream."
			    "</text--->"
			    "<text>query cities feed consume</text--->"
			    ") )";
/*

4.1.3 Definition of operator ~consume~

*/
Operator ccrelconsume (
         "consume",            // name
	 CcConsumeSpec,          // specification
	 CcConsume,              // value mapping
	 Operator::SimpleSelect,         // trivial selection function
	 CcConsumeTypeMap        // type mapping
);
/*

7.1 Operator ~attr~

7.1.1 Type mapping function of operator ~attr~

Result type attr operation.

----
    ((mtuple ((x1 t1)...(xn tn))) xi)    -> ti
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
ListExpr CcAttrTypeMap(ListExpr args)
{
  ListExpr first, second, attrtype;
  string  attrname;
  int j;
  if(nl->ListLength(args) == 2)
  {
    first = nl->First(args);
    second  = nl->Second(args);

    if((nl->ListLength(first) == 2  ) &&
        (CcTypeOfRelAlgSymbol(nl->First(first)) == mtuple)  &&
        (nl->IsAtom(second)) &&
        (nl->AtomType(second) == SymbolType))
    {
      attrname = nl->SymbolValue(second);
      j = CcFindAttribute(nl->Second(first), attrname, attrtype, nl);
      if (j)
      return nl->ThreeElemList(nl->SymbolAtom("APPEND"),
                  nl->OneElemList(nl->IntAtom(j)), attrtype);
    }
  }
  ErrorReporter::ReportError("Incorrect input for operator attr.");
  return nl->SymbolAtom("typeerror");
}
/*

4.1.2 Value mapping function of operator ~attr~

The argument vector ~arg~ contains in the first slot ~args[0]~ the tuple
and in ~args[2]~ the position of the attribute as a number. Returns as
~result~ the value of an attribute at the given position ~args[2]~ in a
tuple object. The attribute name is argument 2 in the query and is used
in the function ~AttributeTypeMap~ to determine the attribute
number ~args[2]~ .

*/
static int
CcAttr(Word* args, Word& result, int message, Word& local, Supplier s)
{
  CcTuple* tupleptr;
  int index;

  tupleptr = (CcTuple*)args[0].addr;
  index = ((CcInt*)args[2].addr)->GetIntval();
  assert( 1 <= index && index <= tupleptr->GetNoAttrs() );
  result = SetWord(tupleptr->Get(index - 1));
  return 0;
}
/*

4.1.3 Specification of operator ~attr~

*/
const string CcAttrSpec  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                         "\"Remarks\" ) "
                         "( <text>((tuple ((x1 t1)...(xn tn))) xi)  -> "
			 "ti)</text--->"
			 "<text>attr ( _ , _ )</text--->"
			 "<text>Returns the value of an attribute at a "
			 "given position.</text--->"
			 "<text>not for use with sos-syntax</text--->"
			 ") )";
/*

4.1.3 Definition of operator ~attr~

*/
Operator ccrelattr (
     "attr",           // name
     CcAttrSpec,        // specification
     CcAttr,            // value mapping
     Operator::SimpleSelect,         // trivial selection function
     CcAttrTypeMap      // type mapping
);
/*

7.3 Operator ~filter~

Only tuples, fulfilling a certain condition are passed on to the output stream.

7.3.1 Type mapping function of operator ~filter~

Result type of filter operation.

----    ((stream (mtuple x)) (map (mtuple x) bool))       -> (stream (mtuple x))
----

*/
template<bool isFilter> 
ListExpr 
CcFilterTypeMap(ListExpr args)
{
  ListExpr first, second;
  if(nl->ListLength(args) == 2)
  {
    first = nl->First(args);
    second  = nl->Second(args);

    if ( (nl->ListLength(first) == 2)
	&& (nl->ListLength(second) == 3)
	&& (nl->ListLength(nl->Second(first)) == 2)
	&& (CcTypeOfRelAlgSymbol(nl->First(nl->Second(first))) == mtuple)
	&& (CcTypeOfRelAlgSymbol(nl->First(first)) == mstream)
	&& (CcTypeOfRelAlgSymbol(nl->First(second)) == mmap)
	&& (CcTypeOfRelAlgSymbol(nl->Third(second)) == mbool)
	&& (nl->Equal(nl->Second(first),nl->Second(second)))	)
    return first;
  }

  ErrorReporter::ReportError(
    isFilter ?
      "Incorrect input for operator filter." :
      "Incorrect input for operator cancel.");
  return nl->SymbolAtom("typeerror");
}

/*

4.1.2 Value mapping function of operator ~filter~

*/
static int
CcFilter(Word* args, Word& result, int message, Word& local, Supplier s)
{
  Word elem = SetWord(0);
  Word funresult = SetWord(0);

  bool found = false;
  CcTuple* tuple = 0;

  ArgVectorPointer funargs;

  switch ( message )
  {

    case OPEN:

	//cout << "tfilter OPEN " << endl;

      qp->Open (args[0].addr);
      return 0;

    case REQUEST:

	//cout << "tfilter REQUEST " << endl;

      funargs = qp->Argument(args[1].addr);
      qp->Request(args[0].addr, elem);
      found = false;
      while (qp->Received(args[0].addr) && !found)
      {
        tuple = (CcTuple*)elem.addr;
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

	//cout << "tfilter CLOSE " << endl;

      qp->Close(args[0].addr);
      return 0;
  }
  return 0;
}
/*

4.1.3 Specification of operator ~filter~

*/
const string CcFilterSpec  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
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

4.1.3 Definition of operator ~filter~

*/
Operator ccrelfilter (
         "filter",            // name
         CcFilterSpec,           // specification
         CcFilter,               // value mapping
         Operator::SimpleSelect,         // trivial selection function
         CcFilterTypeMap<true>         // type mapping
);
/*

7.3 Operator ~project~

7.3.1 Type mapping function of operator ~filter~

Result type of project operation.

----	((stream (mtuple ((x1 T1) ... (xn Tn)))) (ai1 ... aik))	->

		(APPEND
			(k (i1 ... ik))
			(stream (mtuple ((ai1 Ti1) ... (aik Tik))))
		)
----

The type mapping computes the number of attributes and the list of attribute
numbers for the given projection attributes and asks the query processor to
append it to the given arguments.

*/
ListExpr 
CcProjectTypeMap(ListExpr args)
{
  bool firstcall = false;
  int noAttrs = 0, j = 0;

  ListExpr first, second, first2;
  ListExpr attrtype, newAttrList, lastNewAttrList;
  ListExpr lastNumberList, numberList, outlist;
  
  first = second = first2 = nl->TheEmptyList();
  attrtype = newAttrList = lastNewAttrList = nl->TheEmptyList();
  lastNumberList = numberList = outlist = nl->TheEmptyList();

  string attrname = "";

  firstcall = true;
  if (nl->ListLength(args) == 2)
  {
    first = nl->First(args);
    second = nl->Second(args);

    if ((nl->ListLength(first) == 2) &&
        (CcTypeOfRelAlgSymbol(nl->First(first)) == mstream) &&
	(nl->ListLength(nl->Second(first)) == 2) &&
	(CcTypeOfRelAlgSymbol(nl->First(nl->Second(first))) == mtuple) &&
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
        j = CcFindAttribute(nl->Second(nl->Second(first)), attrname, attrtype, nl);
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
		               nl->TwoElemList(nl->SymbolAtom("mtuple"),
			                     newAttrList)));
      // cout << nl->WriteToFile("/dev/tty",outlist) << endl;
      return outlist;
    }
  }
  ErrorReporter::ReportError("Incorrect input for operator project.");
  return nl->SymbolAtom("typeerror");
}
/*

4.1.2 Value mapping function of operator ~project~

*/
static int
CcProject(Word* args, Word& result, int message, Word& local, Supplier s)
{
  Word elem1, elem2, arg2;
  int noOfAttrs, index;
  Supplier son;
  Attribute* attr;
  CcTuple* t;


  switch (message)
  {
    case OPEN :

	//cout << "project OPEN" << endl;


      qp->Open(args[0].addr);
      return 0;

    case REQUEST :

	//cout << "project REQUEST" << endl;

      qp->Request(args[0].addr, elem1);
      if (qp->Received(args[0].addr))
      {
        t = new CcTuple();
        t->SetFree(true);

	qp->Request(args[2].addr, arg2);
        noOfAttrs = ((CcInt*)arg2.addr)->GetIntval();
        t->SetNoAttrs(noOfAttrs);
        for (int i=1; i <= noOfAttrs; i++)
        {
          son = qp->GetSupplier(args[3].addr, i-1);
          qp->Request(son, elem2);
          index = ((CcInt*)elem2.addr)->GetIntval();
          attr = ((CcTuple*)elem1.addr)->Get(index-1);
          t->Put(i-1, ((StandardAttribute*)attr->Clone()));
        }
        ((CcTuple*)elem1.addr)->DeleteIfAllowed();
        result = SetWord(t);
        return YIELD;
      }
      else return CANCEL;

    case CLOSE :

	//cout << "project CLOSE" << endl;

      qp->Close(args[0].addr);
      return 0;
  }
  return 0;
}
/*

4.1.3 Specification of operator ~project~

*/
const string CcProjectSpec  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                            "\"Example\" ) "
                            "( <text>((stream (mtuple ((x1 T1) ... "
			    "(xn Tn)))) (ai1 ... aik)) -> (stream (mtuple"
			    " ((ai1 Ti1) ... (aik Tik))))</text--->"
			    "<text>_ project [ list ]</text--->"
			    "<text>Produces a projection tuple for each "
			    "tuple of its input stream.</text--->"
			    "<text>query cities feed project[cityname, "
			    "population] consume</text--->"
			      ") )";

/*

4.1.3 Definition of operator ~project~

*/
Operator ccrelproject (
         "project",            // name
         CcProjectSpec,          // specification
         CcProject,              // value mapping
         Operator::SimpleSelect,         // trivial selection function
         CcProjectTypeMap        // type mapping
);

/*

7.3 Operator ~remove~

7.3.1 Type mapping function of operator ~remove~

Result type of ~remove~ operation.

----	((stream (mtuple ((x1 T1) ... (xn Tn)))) (ai1 ... aik))	->

		(APPEND
			(n-k (j1 ... jn-k))
			(stream (mtuple ((aj1 Tj1) ... (ajn-k Tjn-k))))
		)
----

The type mapping computes the number of attributes and the list of attribute
numbers for the given left attributes (after removal) and asks the query processor to
append it to the given arguments.

*/
ListExpr 
CcRemoveTypeMap(ListExpr args)
{
  bool firstcall = false;
  int noAttrs = 0, j = 0;

  ListExpr first, second, first2;
  ListExpr attrtype, newAttrList, lastNewAttrList;
  ListExpr lastNumberList, numberList, outlist;

  first = second = first2 = nl->TheEmptyList();
  attrtype = newAttrList = lastNewAttrList = nl->TheEmptyList();
  lastNumberList = numberList = outlist = nl->TheEmptyList();

  string attrname = "";
  set<int> removeSet;
  removeSet.clear();

  firstcall = true;
  if (nl->ListLength(args) == 2)
  {
    first = nl->First(args);
    second = nl->Second(args);

    if ((nl->ListLength(first) == 2) &&
        (CcTypeOfRelAlgSymbol(nl->First(first)) == mstream) &&
	(nl->ListLength(nl->Second(first)) == 2) &&
	(CcTypeOfRelAlgSymbol(nl->First(nl->Second(first))) == mtuple) &&
	(!nl->IsAtom(second)) &&
	(nl->ListLength(second) > 0))
    {
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
	  ErrorReporter::ReportError("Incorrect input for operator ~remove~.");
	  return nl->SymbolAtom("typeerror");
	}

	j = CcFindAttribute(nl->Second(nl->Second(first)), attrname, attrtype, nl);
	if (j)  removeSet.insert(j);
	else
	{
	  ErrorReporter::ReportError("Incorrect input for operator ~remove~.");
	  return nl->SymbolAtom("typeerror");
	}
      }
      /* here we need to generate new attr list according to removeSet */
      ListExpr oldAttrList;
      int i;
      i=0;  // i is the index of the old attriblist
      first = nl->First(args);
      second = nl->Second(args);
      oldAttrList=nl->Second(nl->Second(first));
      //noAttrs = nl->ListLength(oldAttrList) - nl->ListLength(second);  // n-k
      noAttrs =0;
      while (!(nl->IsEmpty(oldAttrList)))
      {
	i++;
	first2 = nl->First(oldAttrList);
	oldAttrList = nl->Rest(oldAttrList);

	if (removeSet.find(i)==removeSet.end())  //the attribute is not in the removal list
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

      // Check whether all new attribute names are distinct
      // - not yet implemented
      //check whether the returning list is null
      if (noAttrs>0)
      {outlist = nl->ThreeElemList(
                 nl->SymbolAtom("APPEND"),
		 nl->TwoElemList(nl->IntAtom(noAttrs), numberList),
		 nl->TwoElemList(nl->SymbolAtom("stream"),
		               nl->TwoElemList(nl->SymbolAtom("mtuple"),
			                     newAttrList)));
      return outlist;
      }
      else
      {
      ErrorReporter::ReportError(
	"Incorrect input for operator ~remove~ - trying to remove all attributes.");
      return nl->SymbolAtom("typeerror");
      }
    }
  }
  ErrorReporter::ReportError("Incorrect input for operator ~remove~.");
  return nl->SymbolAtom("typeerror");
}

/*

4.1.2 Value mapping function of operator ~remove~

*/
static int
CcRemove(Word* args, Word& result, int message, Word& local, Supplier s)
{
  Word elem1, elem2, arg2;
  int noOfAttrs, index;
  Supplier son;
  Attribute* attr;
  CcTuple* t;


  switch (message)
  {
    case OPEN :
      qp->Open(args[0].addr);
      return 0;

    case REQUEST :
      qp->Request(args[0].addr, elem1);
      if (qp->Received(args[0].addr))
      {
        t = new CcTuple();
        t->SetFree(true);

	qp->Request(args[2].addr, arg2);
        noOfAttrs = ((CcInt*)arg2.addr)->GetIntval();
        t->SetNoAttrs(noOfAttrs);
        for (int i=1; i <= noOfAttrs; i++)
        {
          son = qp->GetSupplier(args[3].addr, i-1);
          qp->Request(son, elem2);
          index = ((CcInt*)elem2.addr)->GetIntval();
          attr = ((CcTuple*)elem1.addr)->Get(index-1);
          t->Put(i-1, ((StandardAttribute*)attr->Clone()));
        }
        ((CcTuple*)elem1.addr)->DeleteIfAllowed();
        result = SetWord(t);
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

4.1.3 Specification of operator ~remove~

*/
const string CcRemoveSpec  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                           "\"Example\" ) "
                           "( <text>((stream (mtuple ((x1 T1) ... "
			   "(xn Tn)))) (ai1 ... aik)) -> (stream "
			   "(mtuple ((aj1 Tj1) ... (ajn-k Tjn-k))))"
			   "</text--->"
			   "<text>_ remove [list]</text--->"
			   "<text>Produces a removal tuple for each "
			   "tuple of its input stream.</text--->"
			   "<text>query cities feed remove[zipcode] "
			   "consume</text--->"
			      ") )";

/*

4.1.3 Definition of operator ~remove~

*/
Operator ccrelremove (
         "remove",                             // name
         CcRemoveSpec,                        // specification
         CcRemove,                               // value mapping
         Operator::SimpleSelect,                       // trivial selection function
         CcRemoveTypeMap               // type mapping
);
/*

7.3 Operator ~product~

5.6.1 Help Function ~Concat~

Copies the attribute values of two tuples
(words) ~r~ and ~s~ into tuple (word) ~t~.

*/
void CcConcat (Word r, Word s, Word& t)
{
  int rnoattrs, snoattrs, tnoattrs;
  Attribute* attr;

  rnoattrs = ((CcTuple*)r.addr)->GetNoAttrs();
  snoattrs = ((CcTuple*)s.addr)->GetNoAttrs();
  if ((rnoattrs + snoattrs) > MaxSizeOfAttr)
  {
    tnoattrs = MaxSizeOfAttr;
  }
  else
  {
    tnoattrs = rnoattrs + snoattrs;
  }

  ((CcTuple*)t.addr)->SetNoAttrs(tnoattrs);
  for (int i = 1; i <= rnoattrs; i++)
  {
    attr = ((CcTuple*)r.addr)->Get(i - 1);
    ((CcTuple*)t.addr)->Put((i - 1), ((StandardAttribute*)attr)->Clone());
  }
  for (int j = (rnoattrs + 1); j <= tnoattrs; j++)
  {
    attr = ((CcTuple*)s.addr)->Get(j - rnoattrs - 1);
    ((CcTuple*)t.addr)->Put((j - 1), ((StandardAttribute*)attr)->Clone());
  }
}
/*

7.3.1 Type mapping function of operator ~product~

Result type of product operation.

----	((stream (mtuple (x1 ... xn))) (stream (mtuple (y1 ... ym))))

	-> (stream (mtuple (x1 ... xn y1 ... ym)))
----

*/

bool CcCompareNames(ListExpr list)
{
  vector<string> attrnamestrlist;
  vector<string>::iterator it;
  ListExpr attrnamelist;
  int unique;
  string attrname;

  attrnamelist = list;
  attrnamestrlist.resize(nl->ListLength(list));
  it = attrnamestrlist.begin();
  while (!nl->IsEmpty(attrnamelist))
  {
    attrname = nl->SymbolValue(nl->First(nl->First(attrnamelist)));
    attrnamelist = nl->Rest(attrnamelist);
    unique = std::count(attrnamestrlist.begin(), attrnamestrlist.end(),
	                       attrname);
    *it =  attrname;
    if (unique) return false;
    it++;
  }
  return true;
}

ListExpr 
CcProductTypeMap(ListExpr args)
{
  ListExpr first, second, list, list1, list2, outlist;

  if (nl->ListLength(args) == 2)
  {
    first = nl->First(args); second = nl->Second(args);

    // Check first argument and extract list1
    if (nl->ListLength(first) == 2)
    {
      if (CcTypeOfRelAlgSymbol(nl->First(first)) == mstream)
      {
        if (nl->ListLength(nl->Second(first)) == 2)
        {
          if (CcTypeOfRelAlgSymbol(nl->First(nl->Second(first))) == mtuple)
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
      if (CcTypeOfRelAlgSymbol(nl->First(second)) == mstream)
      {
        if (nl->ListLength(nl->Second(second)) == 2)
        {
          if (CcTypeOfRelAlgSymbol(nl->First(nl->Second(second))) == mtuple)
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

    list = CcConcatLists(list1, list2);
    // Check whether all new attribute names are distinct
    // - not yet implemented

    if ( CcCompareNames(list) )
    {
      outlist = nl->TwoElemList(nl->SymbolAtom("stream"),
        nl->TwoElemList(nl->SymbolAtom("mtuple"), list));
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

4.1.2 Value mapping function of operator ~product~

*/

CPUTimeMeasurer ccProductMeasurer;

struct CcProductLocalInfo
{
  CcTuple* currentTuple;
  vector<CcTuple*> rightRel;
  vector<CcTuple*>::iterator iter;
};

static int
CcProduct(Word* args, Word& result, int message, Word& local, Supplier s)
{
  Word r, u, t;
  CcTuple* tuple;
  CcProductLocalInfo* pli;

  switch (message)
  {
    case OPEN :

      qp->Open(args[0].addr);
      qp->Request(args[0].addr, r);
      pli = new CcProductLocalInfo;
      pli->currentTuple = qp->Received(args[0].addr) ? (CcTuple*)r.addr : 0;

      /* materialize right stream */
      qp->Open(args[1].addr);
      qp->Request(args[1].addr, u);
      while(qp->Received(args[1].addr))
      {
        pli->rightRel.push_back((CcTuple*)u.addr);
        qp->Request(args[1].addr, u);
      }

      pli->iter = pli->rightRel.begin();
      local = SetWord(pli);
      return 0;

    case REQUEST :
      pli = (CcProductLocalInfo*)local.addr;

      ccProductMeasurer.Enter();

      if (pli->currentTuple == 0)
      {
        ccProductMeasurer.Exit();
        return CANCEL;
      }
      else
      {
        if(pli->iter != pli->rightRel.end())
        {
          tuple = new CcTuple();
          tuple->SetFree(true);
          t = SetWord(tuple);
          CcConcat(SetWord(pli->currentTuple), SetWord(*(pli->iter)), t);
          result = t;
          ++(pli->iter);

          ccProductMeasurer.Exit();
          return YIELD;
        }
        else
        {
          /* restart iterator for right relation and
             fetch a new tuple from left stream */
          pli->currentTuple->DeleteIfAllowed();
          pli->currentTuple = 0;
          qp->Request(args[0].addr, r);
          if (qp->Received(args[0].addr))
          {
            pli->currentTuple = (CcTuple*)r.addr;
            pli->iter = pli->rightRel.begin();
            if(pli->iter == pli->rightRel.end()) // second stream is empty
            {
              ccProductMeasurer.Exit();
              return CANCEL;
            }
            else
            {
              tuple = new CcTuple();
              tuple->SetFree(true);
              t = SetWord(tuple);
              CcConcat(SetWord(pli->currentTuple), SetWord(*(pli->iter)), t);
              result = t;
              ++(pli->iter);

              ccProductMeasurer.Exit();
              return YIELD;
            }
          }
          else
          {
            ccProductMeasurer.Exit();
            return CANCEL; // left stream exhausted
          }
        }
      }

    case CLOSE :
      pli = (CcProductLocalInfo*)local.addr;
      if(pli->currentTuple != 0)
      {
        pli->currentTuple->DeleteIfAllowed();
      }

      for(pli->iter = pli->rightRel.begin();
        pli->iter != pli->rightRel.end();
        ++(pli->iter))
      {
        (*(pli->iter))->DeleteIfAllowed();
      }
      delete pli;

      qp->Close(args[0].addr);
      qp->Close(args[1].addr);

      ccProductMeasurer.PrintCPUTimeAndReset("Product CPU Time : ");
      return 0;
  }
  return 0;
}
/*

4.1.3 Specification of operator ~product~

*/
const string CcProductSpec  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                            "\"Example\" ) "
                            "( <text>((stream (mtuple (x1 ... xn))) (stream "
			    "(mtuple (y1 ... ym)))) -> (stream (mtuple (x1 "
			    "... xn y1 ... ym)))</text--->"
			    "<text>_ _ product</text--->"
			    "<text>Computes a Cartesian product stream from "
			    "its two argument streams.</text--->"
			    "<text>query ten feed twenty feed product count"
			    "</text--->"
			     " ) )";
/*

4.1.3 Definition of operator ~product~

*/
Operator ccrelproduct (
         "product",            // name
         CcProductSpec,          // specification
         CcProduct,              // value mapping
         Operator::SimpleSelect,         // trivial selection function
         CcProductTypeMap        // type mapping
);
/*

7.3 Operator ~cancel~

Transmits tuple from its input stream to its output stream until a tuple
arrives fulfilling some condition.

7.3.1 Type mapping function of operator ~cancel~

Type mapping for ~cancel~ is the same, as type mapping for operator ~filter~.
Result type of cancel operation.

----    ((stream x) (map x bool)) -> (stream x)
----

4.1.2 Value mapping function of operator ~cancel~

*/
static int
CcCancel(Word* args, Word& result, int message, Word& local, Supplier s)
{
  Word t, value;
  CcTuple* tuple;
  bool found;
  ArgVectorPointer vector;

  switch (message)
  {
    case OPEN :

      qp->Open(args[0].addr);
      return 0;

    case REQUEST :

      qp->Request(args[0].addr, t);
      found= false;
      if (qp->Received(args[0].addr))
      {
        tuple = (CcTuple*)t.addr;
        vector = qp->Argument(args[1].addr);
        (*vector)[0] = t;
        qp->Request(args[1].addr, value);
        found =
          ((CcBool*)value.addr)->IsDefined()
          && ((CcBool*)value.addr)->GetBoolval();
        if (found)
        {
          qp->Close(args[0].addr);
          return CANCEL;
        }
        else
        {
          result = SetWord(tuple);
          return YIELD;
        }
      }
      else return CANCEL;

    case CLOSE :

      qp->Close(args[0].addr);
      return 0;
  }
  return 0;
}
/*

4.1.3 Specification of operator ~cancel~

*/
const string CcCancelSpec  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                           "\"Example\" ) "
                           "( <text>((stream x) (map x bool)) -> "
			   "(stream x)</text--->"
			   "<text>_ cancel [ fun ]</text--->"
			   "<text>Transmits tuple from its input stream "
			   "to its output stream until a tuple arrives "
			   "fulfilling some condition.</text--->"
			   "<text>query cities feed cancel [.cityname = "
			   "\"Dortmund\"] consume</text--->"
			      ") )";
/*

4.1.3 Definition of operator ~cancel~

*/
Operator ccrelcancel (
         "cancel",             // name
         CcCancelSpec,           // specification
         CcCancel,               // value mapping
         Operator::SimpleSelect,         // trivial selection function
         CcFilterTypeMap<false>         // type mapping
);
/*

7.3 Operator ~count~

Count the number of tuples within a stream of tuples.


7.3.1 Type mapping function of operator ~count~

Operator ~count~ accepts a stream of tuples and returns an integer.

----    (stream  (mtuple x))                 -> int
----

*/
ListExpr
CcCountTypeMap(ListExpr args)
{
  ListExpr first;

  if( nl->ListLength(args) == 1 )
  {
    first = nl->First(args);
    if ( (nl->ListLength(first) == 2) && nl->ListLength(nl->Second(first)) == 2  )
    {
      if ( ( CcTypeOfRelAlgSymbol(nl->First(first)) == mstream
             || CcTypeOfRelAlgSymbol(nl->First(first)) == mrel )
	   && CcTypeOfRelAlgSymbol(nl->First(nl->Second(first))) == mtuple )
      return nl->SymbolAtom("int");
    }
  }
  ErrorReporter::ReportError("Incorrect input for operator count.");
  return nl->SymbolAtom("typeerror");
}


/*

4.1.2 Value mapping functions of operator ~count~

*/
static int
CcCountStream(Word* args, Word& result, int message, Word& local, Supplier s)
{
  Word elem;
  int count = 0;

	//cout << "tcount" << endl;

  qp->Open(args[0].addr);
  qp->Request(args[0].addr, elem);
  while ( qp->Received(args[0].addr) )
  {
    ((CcTuple*)elem.addr)->DeleteIfAllowed();
    count++;
    qp->Request(args[0].addr, elem);
  }
  result = qp->ResultStorage(s);
  ((CcInt*) result.addr)->Set(true, count);
  qp->Close(args[0].addr);
  return 0;
}

static int
CcCountRel(Word* args, Word& result, int message, Word& local, Supplier s)
{
  CcRel* rel = (CcRel*)args[0].addr;
  result = qp->ResultStorage(s);
  ((CcInt*) result.addr)->Set(true, rel->GetNoTuples());
  return 0;
}


/*

4.1.3 Specification of operator ~count~

*/
const string CcCountSpec  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                           "\"Example\" ) "
                           "( <text>((stream/mrel (mtuple x))) -> int"
			   "</text--->"
			   "<text>_ count</text--->"
			   "<text>Count number of tuples within a stream "
			   "or a relation of tuples.</text--->"
			   "<text>query cities count or query cities "
			   "feed count</text--->"
			      ") )";
/*

4.3.1 Selection function of operator ~tcount~

*/

static int
CcCountSelect( ListExpr args )
{
  if(nl->ListLength(args) == 1)
  {
     ListExpr first = nl->First(args);
    if(nl->ListLength(first) == 2)
    {
      if (CcTypeOfRelAlgSymbol(nl->First(first)) == mstream)
      {
        return 0;
      }
      else
      {
        if(CcTypeOfRelAlgSymbol(nl->First(first)) == mrel)
        {
          return 1;
        }
      }
    }
  }
  return -1;
}

/*

4.1.3 Definition of operator ~count~

*/
ValueMapping ccrelcountmap[] = {CcCountStream, CcCountRel };

Operator ccrelcount (
         "count",           // name
         CcCountSpec,         // specification
         2,                  // number of value mapping functions
         ccrelcountmap,          // value mapping functions
         CcCountSelect,       // trivial selection function
         CcCountTypeMap       // type mapping
);
/*

7.3 Operator ~rename~

Renames all attribute names by adding them with the postfix passed as parameter.

7.3.1 Type mapping function of operator ~rename~

Type mapping for ~rename~ is

----	((stream (mtuple([a1:d1, ... ,an:dn)))ar) ->
           (stream (mtuple([a1ar:d1, ... ,anar:dn)))
----

*/
static ListExpr
CcRenameTypeMap( ListExpr args )
{
  ListExpr first, first2, second;
  ListExpr rest, listn, lastlistn;

  first = first2 = second = nl->TheEmptyList();
  rest = listn = lastlistn = nl->TheEmptyList();

  string  attrname = "";
  string  attrnamen = "";
  bool firstcall = true;

  if(nl->ListLength(args) == 2)
  {
    first = nl->First(args);
    second  = nl->Second(args);

    if((nl->ListLength(first) == 2  ) &&
    	(CcTypeOfRelAlgSymbol(nl->First(first)) == mstream) &&
	(CcTypeOfRelAlgSymbol(nl->First(nl->Second(first))) == mtuple) &&
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
		nl->TwoElemList(nl->SymbolAtom("mtuple"),
		listn));
    }
    ErrorReporter::ReportError("Incorrect input for operator rename.");
    return nl->SymbolAtom("typeerror");
  }
    ErrorReporter::ReportError("Incorrect input for operator rename.");
  return nl->SymbolAtom("typeerror");
}
/*

4.1.2 Value mapping function of operator ~rename~

*/
static int
CcRename(Word* args, Word& result, int message, Word& local, Supplier s)
{
  Word t;
  CcTuple* tuple;

  switch (message)
  {
    case OPEN :

      qp->Open(args[0].addr);
      return 0;

    case REQUEST :

      qp->Request(args[0].addr,t);
      if (qp->Received(args[0].addr))
      {
        tuple = (CcTuple*)t.addr;
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

4.1.3 Specification of operator ~rename~

*/
const string CcRenameSpec  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                           "\"Example\" ) "
                           "( <text>((stream (mtuple([a1:d1, ... ,"
			   "an:dn)))ar) -> (stream (mtuple([a1ar:d1, "
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

4.1.3 Definition of operator ~rename~

*/
Operator ccrelrename (
         "rename",             // name
         CcRenameSpec,           // specification
         CcRename,               // value mapping
         Operator::SimpleSelect,         // trivial selection function
         CcRenameTypeMap         // type mapping
);
/*

7.3 Operator ~extract~

This operator has a stream of tuples and the name of an attribut as input and
returns the value of this attribute
from the first tuple of the input stream. If the input stream is empty a run
time error occurs. In this case value -1 will be returned.

7.3.1 Type mapping function of operator ~extract~

Type mapping for ~extract~ is

----	((stream (mtuple ((x1 t1)...(xn tn))) xi) 	-> ti
							APPEND (i) ti)
----

*/
static ListExpr
CcExtractTypeMap( ListExpr args )
{
  ListExpr first, second, attrtype;
  string  attrname;
  int j;

  if(nl->ListLength(args) == 2)
  {
    first = nl->First(args);
    second  = nl->Second(args);

    if((nl->ListLength(first) == 2  ) &&
       (CcTypeOfRelAlgSymbol(nl->First(first)) == mstream)  &&
       (CcTypeOfRelAlgSymbol(nl->First(nl->Second(first))) == mtuple)  &&
       (nl->IsAtom(second)) &&
       (nl->AtomType(second) == SymbolType))
    {
      attrname = nl->SymbolValue(second);
      j = CcFindAttribute(nl->Second(nl->Second(first)), attrname, attrtype, nl);
      if (j)
        return nl->ThreeElemList(nl->SymbolAtom("APPEND"),
          nl->OneElemList(nl->IntAtom(j)), attrtype);
    }
    ErrorReporter::ReportError("Incorrect input for operator extract.");
    return nl->SymbolAtom("typeerror");
  }
  ErrorReporter::ReportError("Incorrect input for operator extract.");
  return nl->SymbolAtom("typeerror");
}
/*

4.1.2 Value mapping function of operator ~extract~

The argument vector ~args~ contains in the first slot ~args[0]~ the tuple
and in ~args[2]~ the position of the attribute as a number. Returns as
~result~ the value of an attribute at the given position ~args[2]~ in a
tuple object.

*/
static int
CcExtract(Word* args, Word& result, int message, Word& local, Supplier s)
{
  Word t;
  CcTuple* tupleptr;
  int index;
  StandardAttribute* res = (StandardAttribute*)((qp->ResultStorage(s)).addr);
  result = SetWord(res);

  qp->Open(args[0].addr);
  qp->Request(args[0].addr,t);

  if (qp->Received(args[0].addr))
  {
    tupleptr = (CcTuple*)t.addr;
    index = ((CcInt*)args[2].addr)->GetIntval();
    assert((1 <= index) && (index <= tupleptr->GetNoAttrs()));
    res->CopyFrom((const StandardAttribute*)tupleptr->Get(index - 1));
    tupleptr->DeleteIfAllowed();
  }
  else
  {
    res->SetDefined(false);
  }

  qp->Close(args[0].addr);
  return 0;
}
/*

4.1.3 Specification of operator ~extract~

*/
const string CcExtractSpec  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                            "\"Example\" ) "
                            "( <text>((stream (mtuple([a1:d1, ... ,an:dn]"
			    "))) x ai) -> di</text--->"
			    "<text>_ extract [ _ ]</text--->"
			    "<text>Returns the value of attribute ai of "
			    "the first tuple in the input stream."
			    "</text--->"
			    "<text>query cities feed extract [population]"
			    "</text--->"
			      ") )";
/*

4.1.3 Definition of operator ~extract~

*/
Operator ccrelextract (
         "extract",             // name
         CcExtractSpec,           // specification
         CcExtract,               // value mapping
         Operator::SimpleSelect,          // trivial selection function
         CcExtractTypeMap         // type mapping
);

/*

7.3 Operator ~head~

This operator fetches the first n tuples from a stream.

7.3.1 Type mapping function of operator ~head~

Type mapping for ~head~ is

----	((stream (mtuple ((x1 t1)...(xn tn))) int) 	->
							((stream (mtuple ((x1 t1)...(xn tn))))
----

*/
static ListExpr
CcHeadTypeMap( ListExpr args )
{
  ListExpr first, second;

  if(nl->ListLength(args) == 2)
  {
    first = nl->First(args);
    second  = nl->Second(args);

    if((nl->ListLength(first) == 2  )
      && (CcTypeOfRelAlgSymbol(nl->First(first)) == mstream)
      && (CcTypeOfRelAlgSymbol(nl->First(nl->Second(first))) == mtuple)
      && CcIsTupleDescription(nl->Second(nl->Second(first)), nl)
      && (nl->IsAtom(second))
      && (nl->AtomType(second) == SymbolType)
      && nl->SymbolValue(second) == "int")
    {
      return first;
    }
    ErrorReporter::ReportError("Incorrect input for operator head.");
    return nl->SymbolAtom("typeerror");
  }
  ErrorReporter::ReportError("Incorrect input for operator head.");
  return nl->SymbolAtom("typeerror");
}
/*

4.1.2 Value mapping function of operator ~head~

*/
static int
CcHead(Word* args, Word& result, int message, Word& local, Supplier s)
{
  int maxTuples;
  Word maxTuplesWord;
  Word tupleWord;
  CcTuple* tuple;

  switch(message)
  {
    case OPEN:

	//cout << "head OPEN" << endl;

      qp->Open(args[0].addr);
      local.ival = 0;
      return 0;

    case REQUEST:

	//cout << "head REQUEST" << endl;

      qp->Request(args[1].addr, maxTuplesWord);
      maxTuples = ((CcInt*)maxTuplesWord.addr)->GetIntval();
      if(local.ival >= maxTuples)
      {
        return CANCEL;
      }

      qp->Request(args[0].addr, tupleWord);
      if(qp->Received(args[0].addr))
      {
        tuple = (CcTuple*)tupleWord.addr;
        result = SetWord(tuple);
        local.ival++;
        return YIELD;
      }
      else
      {
        return CANCEL;
      }
    case CLOSE:

	//cout << "head CLOSE" << endl;

      qp->Close(args[0].addr);
      return 0;
  }
  return 0;
}
/*

4.1.3 Specification of operator ~head~

*/
const string CcHeadSpec  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                         "\"Example\" ) "
                         "( <text>((stream (mtuple([a1:d1, ... ,an:dn]"
			 "))) x int) -> (stream (mtuple([a1:d1, ... ,"
			 "an:dn])))</text--->"
			 "<text>_ head [ _ ]</text--->"
			 "<text>Returns the first n tuples in the input "
			 "stream.</text--->"
			 "<text>query cities feed head[10] consume"
			 "</text--->"
			      ") )";
/*

4.1.3 Definition of operator ~head~

*/
Operator ccrelhead (
         "head",             // name
         CcHeadSpec,           // specification
         CcHead,               // value mapping
         Operator::SimpleSelect,          // trivial selection function
         CcHeadTypeMap         // type mapping
);

/*

7.3 Operators ~max~ and ~min~


7.3.1 Type mapping function of Operators ~max~ and ~min~

Type mapping for ~max~ and ~min~ is

----	((stream (mtuple ((x1 t1)...(xn tn))) xi) 	-> ti
							APPEND (i ti)
----

*/
template<bool isMax> ListExpr
CcMaxMinTypeMap( ListExpr args )
{
  ListExpr first, second, attrtype;
  string  attrname;
  int j;
  const char* errorMessage =
    isMax ?
      "Incorrect input for operator max."
      : "Incorrect input for operator min.";

  if(nl->ListLength(args) == 2)
  {
    first = nl->First(args);
    second  = nl->Second(args);

    if((nl->ListLength(first) == 2  )
      && (CcTypeOfRelAlgSymbol(nl->First(first)) == mstream)
      && (CcTypeOfRelAlgSymbol(nl->First(nl->Second(first))) == mtuple)
      && CcIsTupleDescription(nl->Second(nl->Second(first)), nl)
      && (nl->IsAtom(second))
      && (nl->AtomType(second) == SymbolType))
    {
      attrname = nl->SymbolValue(second);
      j = CcFindAttribute(nl->Second(nl->Second(first)), attrname, attrtype, nl);

      if (j > 0
        && (nl->SymbolValue(attrtype) == "real"
          || nl->SymbolValue(attrtype) == "string"
          || nl->SymbolValue(attrtype) == "bool"
          || nl->SymbolValue(attrtype) == "int"))
      {
        return nl->ThreeElemList(nl->SymbolAtom("APPEND"),
          nl->OneElemList(nl->IntAtom(j)), attrtype);
      }
    }
    ErrorReporter::ReportError(errorMessage);
    return nl->SymbolAtom("typeerror");
  }
  ErrorReporter::ReportError(errorMessage);
  return nl->SymbolAtom("typeerror");
}
/*

4.1.2 Value mapping function of operators ~max~ and ~min~

*/

template<bool isMax> int
CcMaxMinValueMapping(Word* args, Word& result, int message, Word& local, Supplier s)
{
  bool definedValueFound = false;
  Word currentTupleWord;
  StandardAttribute* extremum = (StandardAttribute*)(qp->ResultStorage(s)).addr;
  extremum->SetDefined(false);
  result = SetWord(extremum);

  assert(args[2].addr != 0);
  int attributeIndex = ((CcInt*)args[2].addr)->GetIntval() - 1;

  qp->Open(args[0].addr);
  qp->Request(args[0].addr, currentTupleWord);
  while(qp->Received(args[0].addr))
  {
    CcTuple* currentTuple = (CcTuple*)currentTupleWord.addr;
    const StandardAttribute* currentAttr =
      (const StandardAttribute*)currentTuple->Get(attributeIndex);
    if(currentAttr->IsDefined())
    {
      if(definedValueFound)
      {
        if(isMax)
        {
          if(currentAttr->Compare(extremum) > 0)
          {
            extremum->CopyFrom(currentAttr);
          }
        }
        else
        {
          if(currentAttr->Compare(extremum) < 0)
          {
            extremum->CopyFrom(currentAttr);
          }
        }
      }
      else
      {
        definedValueFound = true;
        extremum->CopyFrom(currentAttr);
      }
    }
    currentTuple->DeleteIfAllowed();
    qp->Request(args[0].addr, currentTupleWord);
  }
  qp->Close(args[0].addr);

  return 0;
}
/*

4.1.3 Specification of operator ~max~

*/
const string CcMaxOpSpec  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                          "\"Example\" ) "
                          "( <text>((stream (mtuple([a1:d1, ... ,an:dn]"
			  "))) x ai) -> di</text--->"
			  "<text>_ _ mergesec</text--->"
			  "<text>Returns the maximum value of attribute "
			  "ai over the input stream.</text--->"
			  "<text>query cities feed max [ cityname ]"
			  "</text--->"
			      ") )";
/*

4.1.3 Definition of operator ~max~

*/
Operator ccrelmax (
         "max",             // name
         CcMaxOpSpec,           // specification
         CcMaxMinValueMapping<true>,               // value mapping
         Operator::SimpleSelect,          // trivial selection function
         CcMaxMinTypeMap<true>         // type mapping
);

/*

4.1.3 Specification of operator ~min~

*/
const string CcMinOpSpec  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                          "\"Example\" ) "
                          "( <text>((stream (mtuple([a1:d1, ... ,an:dn])))"
			  " x ai) -> di</text--->"
			  "<text>_ min [ _ ]</text--->"
			  "<text>Returns the minimum value of attribute ai "
			  "over the input stream.</text--->"
			  "<text>query cities feed min [ cityname ]"
			  "</text--->"
			      ") )";
/*

4.1.3 Definition of operator ~min~

*/
Operator ccrelmin (
         "min",             // name
         CcMinOpSpec,           // specification
         CcMaxMinValueMapping<false>,               // value mapping
         Operator::SimpleSelect,          // trivial selection function
         CcMaxMinTypeMap<false>         // type mapping
);

/*

7.3 Operators ~avg~ and ~sum~


7.3.1 Type mapping function of Operators ~avg~ and ~sum~

Type mapping for ~avg~ is

----	((stream (mtuple ((x1 t1)...(xn tn))) xi) 	-> real
							APPEND (i ti)
----

Type mapping for ~sum~ is

----	((stream (mtuple ((x1 t1)...(xn tn))) xi) 	-> ti
							APPEND (i ti)
----

*/

template<bool isAvg> ListExpr
CcAvgSumTypeMap( ListExpr args )
{
  ListExpr first, second, attrtype;
  string  attrname;
  int j;
  const char* errorMessage =
    isAvg ?
      "Incorrect input for operator avg."
      : "Incorrect input for operator sum.";


  if(nl->ListLength(args) == 2)
  {
    first = nl->First(args);
    second  = nl->Second(args);

    if((nl->ListLength(first) == 2  )
      && (CcTypeOfRelAlgSymbol(nl->First(first)) == mstream)
      && (CcTypeOfRelAlgSymbol(nl->First(nl->Second(first))) == mtuple)
      && CcIsTupleDescription(nl->Second(nl->Second(first)), nl)
      && (nl->IsAtom(second))
      && (nl->AtomType(second) == SymbolType))
    {
      attrname = nl->SymbolValue(second);
      j = CcFindAttribute(nl->Second(nl->Second(first)), attrname, attrtype, nl);

      if (j > 0
        && (nl->SymbolValue(attrtype) == "real"
          || nl->SymbolValue(attrtype) == "int"))
      {
        return nl->ThreeElemList(nl->SymbolAtom("APPEND"),
          nl->TwoElemList(nl->IntAtom(j),
            nl->StringAtom(nl->SymbolValue(attrtype))),
            isAvg ? nl->SymbolAtom("real") : attrtype);
      }
    }
    ErrorReporter::ReportError(errorMessage);
    return nl->SymbolAtom("typeerror");
  }
  ErrorReporter::ReportError(errorMessage);
  return nl->SymbolAtom("typeerror");
}

/*

4.1.2 Value mapping function of operators ~avg~ and ~sum~

*/
template<bool isAvg> int
CcAvgSumValueMapping(Word* args, Word& result, int message, Word& local, Supplier s)
{
  bool definedValueFound = false;
  Word currentTupleWord;
  Attribute* accumulated = 0;
  int nProcessedItems = 0;

  assert(args[2].addr != 0);
  assert(args[3].addr != 0);

  int attributeIndex = ((CcInt*)args[2].addr)->GetIntval() - 1;
  const STRING *attributeType = ((CcString*)args[3].addr)->GetStringval();

  qp->Open(args[0].addr);
  qp->Request(args[0].addr, currentTupleWord);
  while(qp->Received(args[0].addr))
  {
    CcTuple* currentTuple = (CcTuple*)currentTupleWord.addr;
    Attribute* currentAttr = (Attribute*)currentTuple->Get(attributeIndex);
    if(currentAttr->IsDefined())
    {
      nProcessedItems++;

      if(definedValueFound)
      {
        if(strcmp(*attributeType, "real") == 0)
        {
          CcReal* accumulatedReal = (CcReal*)accumulated;
          CcReal* currentReal = (CcReal*)currentAttr;
          accumulatedReal->Set(currentReal->GetRealval()
            + accumulatedReal->GetRealval());
        }
        else
        {
          CcInt* accumulatedInt = (CcInt*)accumulated;
          CcInt* currentInt = (CcInt*)currentAttr;
          accumulatedInt->Set(currentInt->GetIntval()
            + accumulatedInt->GetIntval());
        }
      }
      else
      {
        definedValueFound = true;
        accumulated = currentAttr->Clone();
      }
    }
    currentTuple->DeleteIfAllowed();
    qp->Request(args[0].addr, currentTupleWord);
  }
  qp->Close(args[0].addr);

  if(definedValueFound)
  {
    if(isAvg)
    {
      CcReal* resultAttr = (CcReal*)(qp->ResultStorage(s).addr);
      float nItems = (float)nProcessedItems;

      if(strcmp(*attributeType, "real") == 0)
      {
        CcReal* accumulatedReal = (CcReal*)accumulated;
        resultAttr->Set(accumulatedReal->GetRealval() / nItems);
      }
      else
      {
        CcInt* accumulatedInt = (CcInt*)accumulated;
        resultAttr->Set(((float)accumulatedInt->GetIntval()) / nItems);
      }
      delete accumulated;
      result = SetWord(resultAttr);
    }
    else
    {
      if(strcmp(*attributeType, "real") == 0)
      {
        CcReal* resultAttr = (CcReal*)(qp->ResultStorage(s).addr);
        CcReal* accumulatedReal = (CcReal*)accumulated;
        resultAttr->Set(accumulatedReal->GetRealval());
        result = SetWord(resultAttr);
      }
      else
      {
        CcInt* resultAttr = (CcInt*)(qp->ResultStorage(s).addr);
        CcInt* accumulatedInt = (CcInt*)accumulated;
        resultAttr->Set(accumulatedInt->GetIntval());
        result = SetWord(resultAttr);
      }
      delete accumulated;
    }
    return 0;
  }
  else
  {
    ((StandardAttribute*)qp->ResultStorage(s).addr)->SetDefined(false);
    result = qp->ResultStorage(s);
    return 0;
  }
}

/*

4.1.3 Specification of operator ~avg~

*/
const string CcAvgOpSpec  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                          "\"Example\" ) "
                          "( <text>((stream (mtuple([a1:d1, ... ,an:dn]"
			  "))) x ai) -> real</text--->"
			  "<text>_ avg [ _ ]</text--->"
			  "<text>Returns the average value of attribute "
			  "ai over the input stream.</text--->"
			  "<text>query cities feed avg [population]"
			  "</text--->"
			      ") )";
/*

4.1.3 Definition of operator ~avg~

*/
Operator ccrelavg (
         "avg",             // name
         CcAvgOpSpec,           // specification
         CcAvgSumValueMapping<true>,               // value mapping
         Operator::SimpleSelect,          // trivial selection function
         CcAvgSumTypeMap<true>         // type mapping
);

/*

4.1.3 Specification of operator ~sum~

*/
const string CcSumOpSpec  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                          "\"Example\" ) "
                          "( <text>((stream (mtuple([a1:d1, ... ,an:dn]"
			  "))) x ai) -> di</text--->"
			  "<text>_ sum [ _ ]</text--->"
			  "<text>Returns the sum of the values of attribute"
			  " ai over the input stream.</text--->"
			  "<text>query cities feed sum[population]"
			  "</text--->"
			      ") )";

/*

4.1.3 Definition of operator ~sum~

*/
Operator ccrelsum (
         "sum",             // name
         CcSumOpSpec,           // specification
         CcAvgSumValueMapping<false>,               // value mapping
         Operator::SimpleSelect,          // trivial selection function
         CcAvgSumTypeMap<false>         // type mapping
);

/*

7.3 Operator ~sortBy~

This operator sorts a stream of tuples by a given list of attributes.
For each attribute it must be specified wether the list should be sorted
in ascending (asc) or descending (desc) order with regard to that attribute.

7.3.1 Type mapping function of operator ~sortBy~

Type mapping for ~sortBy~ is

----	((stream (mtuple ((x1 t1)...(xn tn))) ((xi1 asc/desc) ... (xij asc/desc)))
              -> (stream (mtuple ((x1 t1)...(xn tn)))
                  APPEND (j i1 asc/desc i2 asc/desc ... ij asc/desc)
----

*/

const char* sortAscending = "asc";
const char* sortDescending = "desc";

static ListExpr
CcSortByTypeMap( ListExpr args )
{
  ListExpr attrtype;
  string  attrname;

  if(nl->ListLength(args) == 2)
  {
    ListExpr streamDescription = nl->First(args);
    ListExpr sortSpecification  = nl->Second(args);

    if((nl->ListLength(streamDescription) == 2  ) &&
      (CcTypeOfRelAlgSymbol(nl->First(streamDescription)) == mstream)  &&
      (CcTypeOfRelAlgSymbol(nl->First(nl->Second(streamDescription))) == mtuple))
    {
      int numberOfSortAttrs = nl->ListLength(sortSpecification);
      if(numberOfSortAttrs > 0)
      {
        ListExpr sortOrderDescription = nl->OneElemList(nl->IntAtom(numberOfSortAttrs));
        ListExpr sortOrderDescriptionLastElement = sortOrderDescription;
        ListExpr rest = sortSpecification;
        while(!nl->IsEmpty(rest))
        {
          ListExpr attributeSpecification = nl->First(rest);
          rest = nl->Rest(rest);
          if((nl->ListLength(attributeSpecification) == 2)
            && (nl->IsAtom(nl->First(attributeSpecification)))
            && (nl->AtomType(nl->First(attributeSpecification)) == SymbolType)
            && (nl->IsAtom(nl->Second(attributeSpecification)))
            && (nl->AtomType(nl->Second(attributeSpecification)) == SymbolType))
          {
            attrname = nl->SymbolValue(nl->First(attributeSpecification));
            int j = CcFindAttribute(nl->Second(nl->Second(streamDescription)), attrname, attrtype, nl);
            if ((j > 0)
              && ((nl->SymbolValue(nl->Second(attributeSpecification)) == sortAscending)
                  || (nl->SymbolValue(nl->Second(attributeSpecification)) == sortDescending)))
            {
              sortOrderDescriptionLastElement =
                nl->Append(sortOrderDescriptionLastElement, nl->IntAtom(j));
              bool isAscending =
                nl->SymbolValue(nl->Second(attributeSpecification)) == sortAscending;
              sortOrderDescriptionLastElement =
                nl->Append(sortOrderDescriptionLastElement,
                  nl->BoolAtom(isAscending));
            }
            else
            {
              ErrorReporter::ReportError("Incorrect input for operator sortby.");
              return nl->SymbolAtom("typeerror");
            }
          }
          else
          {
            ErrorReporter::ReportError("Incorrect input for operator sortby.");
            return nl->SymbolAtom("typeerror");
          }
        }
        return nl->ThreeElemList(nl->SymbolAtom("APPEND"),
              sortOrderDescription, streamDescription);
      };
      ErrorReporter::ReportError("Incorrect input for operator sortby.");
      return nl->SymbolAtom("typeerror");
    }
    ErrorReporter::ReportError("Incorrect input for operator sortby.");
    return nl->SymbolAtom("typeerror");
  }
  ErrorReporter::ReportError("Incorrect input for operator sortby.");
  return nl->SymbolAtom("typeerror");
}

/*

4.1.2 Value mapping function of operator ~sortBy~

The argument vector ~args~ contains in the first slot ~args[0]~ the stream and
in ~args[2]~ the number of sort attributes. ~args[3]~ contains the index of the first
sort attribute, ~args[4]~ a boolean indicating wether the stream is sorted in
ascending order with regard to the sort first attribute. ~args[5]~ and ~args[6]~
contain these values for the second sort attribute  and so on.

*/

typedef vector< pair<int, bool> > CcSortOrderSpecification;

class CcTupleCmp
{
public:
  CcSortOrderSpecification spec;
  bool operator()(const CcTuple* aConst, const CcTuple* bConst) const
  {
    CcTuple* a = (CcTuple*)aConst;
    CcTuple* b = (CcTuple*)bConst;

    CcSortOrderSpecification::const_iterator iter = spec.begin();
    while(iter != spec.end())
    {
      if(((Attribute*)a->Get(iter->first - 1))->
        Compare(((Attribute*)b->Get(iter->first - 1))) < 0)
      {
        return iter->second;
      }
      else
      {
        if(((Attribute*)a->Get(iter->first - 1))->
          Compare(((Attribute*)b->Get(iter->first - 1))) > 0)
        {
          return !(iter->second);
        }
      }
      iter++;
    }
    return false;
  }
};

struct CcSortByLocalInfo
{
  vector<CcTuple*>* tuples;
  size_t currentIndex;
};

CPUTimeMeasurer ccSortMeasurer;

template<bool lexicographically, bool requestArgs> int
CcSortBy(Word* args, Word& result, int message, Word& local, Supplier s)
{
  Word tuple;
  vector<CcTuple*>* tuples;
  CcSortByLocalInfo* localInfo;
  CcSortOrderSpecification spec;
  int i;
  Word intWord;
  Word boolWord;
  size_t j;
  int sortAttrIndex;
  int nSortAttrs;
  bool sortOrderIsAscending;
  CcTupleCmp ccCmp;
  LexicographicalCcTupleCmp lCcCmp;
  CcTuple* t;

  switch(message)
  {
    case OPEN:
      tuples = new vector<CcTuple*>;
      qp->Open(args[0].addr);
      qp->Request(args[0].addr,tuple);
      while(qp->Received(args[0].addr))
      {
        t =(CcTuple*)tuple.addr;
        tuples->push_back(t);
        qp->Request(args[0].addr,tuple);
      }
      qp->Close(args[0].addr);

      if(lexicographically)
      {
        ccSortMeasurer.Enter();
        sort(tuples->begin(), tuples->end(), lCcCmp);
        ccSortMeasurer.Exit();
      }
      else
      {
	if(requestArgs)
        {
          qp->Request(args[2].addr, intWord);
        }
        else
        {
          intWord = SetWord(args[2].addr);
        }
        nSortAttrs = ((CcInt*)intWord.addr)->GetIntval();
        for(i = 1; i <= nSortAttrs; i++)
        {
	  if(requestArgs)
          {
            qp->Request(args[2 * i + 1].addr, intWord);
          }
          else
          {
            intWord = SetWord(args[2 * i + 1].addr);
          }
          sortAttrIndex = ((CcInt*)intWord.addr)->GetIntval();

          if(requestArgs)
          {
            qp->Request(args[2 * i + 2].addr, boolWord);
          }
          else
          {
            boolWord = SetWord(args[2 * i + 2].addr);
          }
          sortOrderIsAscending = ((CcBool*)boolWord.addr)->GetBoolval();
          spec.push_back(pair<int, bool>(sortAttrIndex, sortOrderIsAscending));
        };
        ccCmp.spec = spec;

        ccSortMeasurer.Enter();
        sort(tuples->begin(), tuples->end(), ccCmp);
        ccSortMeasurer.Exit();
      }

      ccSortMeasurer.PrintCPUTimeAndReset("CPU Time for Sorting Tuples : ");

      localInfo = new CcSortByLocalInfo;
      localInfo->tuples = tuples;
      localInfo->currentIndex = 0;
      local = SetWord(localInfo);
      return 0;
    case REQUEST:
      localInfo = (CcSortByLocalInfo*)local.addr;
      tuples = localInfo->tuples;
      if(localInfo->currentIndex  + 1 <= tuples->size())
      {
        result = SetWord((*tuples)[localInfo->currentIndex]);
        localInfo->currentIndex++;
        return YIELD;
      }
      else
      {
        return CANCEL;
      }
    case CLOSE:
      localInfo = (CcSortByLocalInfo*)local.addr;

      for(j = localInfo->currentIndex;
        j + 1 <= localInfo->tuples->size(); j++)
      {
        (*(localInfo->tuples))[j]->DeleteIfAllowed();
      }

      delete localInfo->tuples;
      delete localInfo;
      return 0;
  }
  return 0;
}
/*

4.1.3 Specification of operator ~sortBy~

*/
const string CcSortBySpec  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                           "\"Example\" ) "
                           "( <text>((stream (mtuple([a1:d1, ... ,an:dn])))"
			   " ((xi1 asc/desc) ... (xij asc/desc))) -> "
			   "(stream (mtuple([a1:d1, ... ,an:dn])))</text--->"
			   "<text>_ sortby [list]</text--->"
			   "<text>Sorts input stream according to a list "
			   "of attributes ai1 ... aij.</text--->"
			   "<text>query employee feed sortby[DeptNo asc] "
			   "consume</text--->"
			      ") )";

/*

4.1.3 Definition of operator ~sortBy~

*/
Operator ccrelsortby (
         "sortby",              // name
         CcSortBySpec,            // specification
         CcSortBy<false, true>,   // value mapping
         Operator::SimpleSelect,          // trivial selection function
         CcSortByTypeMap          // type mapping
);

/*

7.3 Operator ~sort~

This operator sorts a stream of tuples lexicographically.

7.3.1 Type mapping function of operator ~sort~

Type mapping for ~sort~ is

----	((stream (mtuple ((x1 t1)...(xn tn)))) 	-> (stream (mtuple ((x1 t1)...(xn tn)))

----

*/
template<bool isSort> ListExpr
CcIdenticalTypeMap( ListExpr args )
{
  ListExpr first;
  const char* errorMessage = isSort ?
    "Incorrect input for operator sort."
    : "Incorrect input for operator rdup.";

  if(nl->ListLength(args) == 1)
  {
    first = nl->First(args);

    if((nl->ListLength(first) == 2  )
      && (CcTypeOfRelAlgSymbol(nl->First(first)) == mstream)
      && (CcTypeOfRelAlgSymbol(nl->First(nl->Second(first))) == mtuple)
      && CcIsTupleDescription(nl->Second(nl->Second(first)), nl))
    {
      return first;
    }
    ErrorReporter::ReportError(errorMessage);
    return nl->SymbolAtom("typeerror");
  }
  ErrorReporter::ReportError(errorMessage);
  return nl->SymbolAtom("typeerror");
}

/*

4.1.3 Specification of operator ~sort~

*/
const string CcSortSpec  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                         "\"Example\" ) "
                         "( <text>((stream (mtuple([a1:d1, ... ,an:dn]"
			 ")))) -> (stream (mtuple([a1:d1, ... ,an:dn])))"
			 "</text--->"
			 "<text>_ sort</text--->"
			 "<text>Sorts input stream lexicographically."
			 "</text--->"
		    "<text>query cities feed sort consume</text--->"
			      ") )";

/*

4.1.3 Definition of operator ~sort~

*/
Operator ccrelsort (
         "sort",             // name
         CcSortSpec,           // specification
         CcSortBy<true, true>,               // value mapping
         Operator::SimpleSelect,          // trivial selection function
         CcIdenticalTypeMap<true>         // type mapping
);

/*

7.3 Operator ~rdup~

This operator removes duplicates from a sorted stream.

4.1.2 Value mapping function of operator ~rdup~

*/

static int
CcRdupValueMapping(Word* args, Word& result, int message, Word& local, Supplier s)
{
  Word tuple;
  LexicographicalCcTupleCmp cmp;
  CcTuple* currentTuple;
  CcTuple* lastOutputTuple;

  switch(message)
  {
    case OPEN:
      qp->Open(args[0].addr);
      local = SetWord(0);
      return 0;
    case REQUEST:
      while(true)
      {
        qp->Request(args[0].addr, tuple);
        if(qp->Received(args[0].addr))
        {
          if(local.addr != 0)
          {
            currentTuple = (CcTuple*)tuple.addr;
            lastOutputTuple = (CcTuple*)local.addr;
            if(cmp(currentTuple, lastOutputTuple)
              || cmp(lastOutputTuple, currentTuple))
            {
              lastOutputTuple->DeleteIfAllowed();
              local = SetWord(currentTuple->Clone());
              result = SetWord(currentTuple);
              return YIELD;
            }
            else
            {
              currentTuple->DeleteIfAllowed();
            }
          }
          else
          {
            currentTuple = (CcTuple*)tuple.addr;
            local = SetWord(currentTuple->Clone());
            result = SetWord(currentTuple);
            return YIELD;
          }
        }
        else
        {
	  lastOutputTuple = (CcTuple*)local.addr;
          if(lastOutputTuple != 0)
          {
           lastOutputTuple->DeleteIfAllowed();
         }
         return CANCEL;
        }
      }
    case CLOSE:
      qp->Close(args[0].addr);
      return 0;
  }
  return 0;
}
/*

4.1.3 Specification of operator ~rdup~

*/
const string CcRdupSpec  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                         "\"Example\" ) "
                         "( <text>((stream (mtuple([a1:d1, ... ,an:dn]))))"
			 " -> (stream (mtuple([a1:d1, ... ,an:dn])))"
			 "</text--->"
			 "<text>_ rdup</text--->"
			 "<text>Removes duplicates from a sorted "
			 "stream.</text--->"
			 "<text>query twenty feed ten feed concat sort "
			 "rdup consume</text--->"
			      ") )";
/*

4.1.3 Definition of operator ~rdup~

*/
Operator ccrelrdup (
         "rdup",             // name
         CcRdupSpec,           // specification
         CcRdupValueMapping,               // value mapping
         Operator::SimpleSelect,          // trivial selection function
         CcIdenticalTypeMap<false>         // type mapping
);

/*

7.3 Set Operators

These operators compute set operations on two sorted stream.

7.3.1 Generic Type Mapping for Set Operations

*/

const char* ccSetOpErrorMessages[] =
  { "Incorrect input for operator mergesec.",
    "Incorrect input for operator mergediff.",
    "Incorrect input for operator mergeunion." };

template<int errorMessageIdx> ListExpr
CcSetOpTypeMap( ListExpr args )
{
  ListExpr first, second;

  if(nl->ListLength(args) == 2)
  {
    first = nl->First(args);
    second = nl->Second(args);

    if((nl->ListLength(first) == 2  )
      && (CcTypeOfRelAlgSymbol(nl->First(first)) == mstream)
      && (CcTypeOfRelAlgSymbol(nl->First(nl->Second(first))) == mtuple)
      && CcIsTupleDescription(nl->Second(nl->Second(first)), nl)
      && (nl->Equal(first, second)))
    {
      return first;
    }
    ErrorReporter::ReportError(ccSetOpErrorMessages[errorMessageIdx]);
    return nl->SymbolAtom("typeerror");
  }
  ErrorReporter::ReportError(ccSetOpErrorMessages[errorMessageIdx]);
  return nl->SymbolAtom("typeerror");
}

/*

7.3.2 Auxiliary Class for Set Operations

*/

class CcSetOperation
{
public:
  bool outputAWithoutB;
  bool outputBWithoutA;
  bool outputMatches;

private:
  LexicographicalCcTupleCmp smallerThan;

  Word streamA;
  Word streamB;

  CcTuple* currentATuple;
  CcTuple* currentBTuple;

  CcTuple* NextATuple(bool deleteOldTuple)
  {
    Word tuple;
    if(deleteOldTuple && currentATuple != 0)
    {
      currentATuple->DeleteIfAllowed();
    }

    qp->Request(streamA.addr, tuple);
    if(qp->Received(streamA.addr))
    {
      currentATuple = (CcTuple*)tuple.addr;
      return currentATuple;
    }
    else
    {
      currentATuple = 0;
      return 0;
    }
  }

  CcTuple* NextBTuple(bool deleteOldTuple)
  {
    Word tuple;
    if(deleteOldTuple && currentBTuple != 0)
    {
      currentBTuple->DeleteIfAllowed();
    }

    qp->Request(streamB.addr, tuple);
    if(qp->Received(streamB.addr))
    {
      currentBTuple = (CcTuple*)tuple.addr;
      return currentBTuple;
    }
    else
    {
      currentBTuple = 0;
      return 0;
    }
  }

  bool TuplesEqual(CcTuple* a, CcTuple* b)
  {
    return !(smallerThan(a, b) || smallerThan(b, a));
  }

public:

  CcSetOperation(Word streamA, Word streamB)
  {
    this->streamA = streamA;
    this->streamB = streamB;

    currentATuple = 0;
    currentBTuple = 0;

    qp->Open(streamA.addr);
    qp->Open(streamB.addr);

    NextATuple(false);
    NextBTuple(false);
  }

  virtual ~CcSetOperation()
  {
    qp->Close(streamA.addr);
    qp->Close(streamB.addr);
  }

  CcTuple* NextResultTuple()
  {
    CcTuple* result = 0;
    while(result == 0)
    {
      if(currentATuple == 0)
      {
        if(currentBTuple == 0)
        {
          return 0;
        }
        else
        {
          if(outputBWithoutA)
          {
            result = currentBTuple;
            NextBTuple(false);
            while(currentBTuple != 0 && TuplesEqual(result, currentBTuple))
            {
              NextBTuple(true);
            }
          }
          else
          {
            currentBTuple->DeleteIfAllowed();
            return 0;
          }
        }
      }
      else
      {
        if(currentBTuple == 0)
        {
          if(outputAWithoutB)
          {
            result = currentATuple;
            NextATuple(false);
            while(currentATuple != 0 && TuplesEqual(result, currentATuple))
            {
              NextATuple(true);
            }
          }
          else
          {
            currentATuple->DeleteIfAllowed();
            return 0;
          }
        }
        else
        {
          /* both current tuples != 0 */
          if(smallerThan(currentATuple, currentBTuple))
          {
            if(outputAWithoutB)
            {
              result = currentATuple;
            }

            CcTuple* tmp = currentATuple;
            NextATuple(false);
            while(currentATuple != 0 && TuplesEqual(tmp, currentATuple))
            {
              NextATuple(true);
            }
            if(!outputAWithoutB)
            {
              tmp->DeleteIfAllowed();
            }
          }
          else if(smallerThan(currentBTuple, currentATuple))
          {
            if(outputBWithoutA)
            {
              result = currentBTuple;
            }

            CcTuple* tmp = currentBTuple;
            NextBTuple(false);
            while(currentBTuple != 0 && TuplesEqual(tmp, currentBTuple))
            {
              NextBTuple(true);
            }
            if(!outputBWithoutA)
            {
              tmp->DeleteIfAllowed();
            }

          }
          else
          {
            /* found match */
            assert(TuplesEqual(currentATuple, currentBTuple));
            CcTuple* match = currentATuple;
            if(outputMatches)
            {
              result = match;
            }

            NextATuple(false);
            while(currentATuple != 0 && TuplesEqual(match, currentATuple))
            {
              NextATuple(true);
            }
            while(currentBTuple != 0 && TuplesEqual(match, currentBTuple))
            {
              NextBTuple(true);
            }
            if(!outputMatches)
            {
              match->DeleteIfAllowed();
            }
          }
        }
      }
    }
    return result;
  }
};

/*

7.3.2 Generic Value Mapping Function for Set Operations

*/

template<bool outputAWithoutB, bool outputBWithoutA, bool outputMatches> int
CcSetOpValueMapping(Word* args, Word& result, int message, Word& local, Supplier s)
{
  CcSetOperation* localInfo;

  switch(message)
  {
    case OPEN:
      localInfo = new CcSetOperation(args[0], args[1]);
      localInfo->outputBWithoutA = outputBWithoutA;
      localInfo->outputAWithoutB = outputAWithoutB;
      localInfo->outputMatches = outputMatches;

      local = SetWord(localInfo);
      return 0;
    case REQUEST:
      localInfo = (CcSetOperation*)local.addr;
      result = SetWord(localInfo->NextResultTuple());
      return result.addr != 0 ? YIELD : CANCEL;
    case CLOSE:
      localInfo = (CcSetOperation*)local.addr;
      delete localInfo;
      return 0;
  }
  return 0;
}

/*

4.1.3 Specification of Operator ~mergesec~

*/
const string CcMergeSecSpec  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                         "\"Example\" ) "
                         "( <text>((stream (mtuple ((x1 t1) ... "
			 "(xn tn)))) stream (mtuple ((x1 t1) ... (xn tn)"
			 ")))) -> (stream (mtuple ((x1 t1) ... (xn tn))))"
			 "</text--->"
			 "<text>_ _ mergesec</text--->"
			 "<text>Computes the intersection of two sorted "
			 "streams.</text--->"
			 "<text>query twenty feed oddtwenty feed mergesec"
			 " consume</text--->"
			 ") )";
/*

4.1.3 Definition of Operator ~mergesec~

*/
Operator ccrelmergesec(
         "mergesec",        // name
         CcMergeSecSpec,     // specification
         CcSetOpValueMapping<false, false, true>,         // value mapping
         Operator::SimpleSelect,          // trivial selection function
         CcSetOpTypeMap<0>   // type mapping
);

/*

4.1.3 Specification of Operator ~mergediff~

*/
const string CcMergeDiffSpec  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                              "\"Example\" ) "
                             "( <text>((stream (mtuple ((x1 t1) ... (xn tn)"
			     "))) stream (mtuple ((x1 t1) ... (xn tn))))) ->"
			     " (stream (mtuple ((x1 t1) ... (xn tn))))"
			     "</text--->"
			     "<text>_ _ mergediff</text--->"
			     "<text>Computes the difference of two sorted "
			     "streams.</text--->"
			     "<text>query twenty feed oddtwenty feed"
			     " mergediff consume</text--->"
			      ") )";
/*

4.1.3 Definition of Operator ~mergediff~

*/
Operator ccrelmergediff(
         "mergediff",        // name
         CcMergeDiffSpec,     // specification
         CcSetOpValueMapping<true, false, false>,         // value mapping
         Operator::SimpleSelect,          // trivial selection function
         CcSetOpTypeMap<1>   // type mapping
);

/*

4.1.3 Specification of Operator ~mergeunion~

*/
const string CcMergeUnionSpec  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                               "\"Example\" ) "
                         "( <text>((stream (mtuple ((x1 t1) ... (xn tn))))"
			 "stream (mtuple ((x1 t1) ... (xn tn))))) -> (stream"
			 " (mtuple ((x1 t1) ... (xn tn))))</text--->"
			 "<text>_ _ mergeunion</text--->"
			 "<text>Computes the union of two sorted streams."
			 "</text--->"
			 "<text>query twenty feed oddtwenty feed "
			 "mergeunion consume</text--->"
			      ") )";
/*

4.1.3 Definition of Operator ~mergeunion~

*/
Operator ccrelmergeunion(
         "mergeunion",        // name
         CcMergeUnionSpec,     // specification
         CcSetOpValueMapping<true, true, true>,         // value mapping
         Operator::SimpleSelect,          // trivial selection function
         CcSetOpTypeMap<2>   // type mapping
);

/*

7.3 Operator ~mergejoin~

This operator computes the equijoin two streams.

7.3.1 Type mapping function of operators ~mergejoin~ and ~hashjoin~

Type mapping for ~mergejoin~ is

----	((stream (mtuple ((x1 t1) ... (xn tn)))) (stream (mtuple ((y1 d1) ... (ym dm)))) xi yj)

      -> (stream (mtuple ((x1 t1) ... (xn tn) (y1 d1) ... (ym dm)))) APPEND (i j)
----

Type mapping for ~hashjoin~ is

----	((stream (mtuple ((x1 t1) ... (xn tn)))) (stream (mtuple ((y1 d1) ... (ym dm)))) xi yj int)

      -> (stream (mtuple ((x1 t1) ... (xn tn) (y1 d1) ... (ym dm)))) APPEND (i j)
----


*/

const char* ccJoinErrorMessages[] =
  { "Incorrect input for operator mergejoin.",
    "Incorrect input for operator sortmergejoin.",
    "Incorrect input for operator hashjoin." };

template<bool expectIntArgument, int errorMessageIdx> 
ListExpr 
CcJoinTypeMap(ListExpr args)
{
  ListExpr attrTypeA, attrTypeB;
  ListExpr streamA, streamB, list, list1, list2, outlist;
  if (nl->ListLength(args) == (expectIntArgument ? 5 : 4))
  {
    streamA = nl->First(args); streamB = nl->Second(args);
    if (nl->ListLength(streamA) == 2)
    {
      if (CcTypeOfRelAlgSymbol(nl->First(streamA)) == mstream)
      {
        if (nl->ListLength(nl->Second(streamA)) == 2)
        {
          if (CcTypeOfRelAlgSymbol(nl->First(nl->Second(streamA))) == mtuple)
          {
            list1 = nl->Second(nl->Second(streamA));
          }
          else goto typeerror;
        }
        else goto typeerror;
      }
      else goto typeerror;
    }
    else goto typeerror;

    if (nl->ListLength(streamB) == 2)
    {
      if (CcTypeOfRelAlgSymbol(nl->First(streamB)) == mstream)
      {
        if (nl->ListLength(nl->Second(streamB)) == 2)
        {
          if (CcTypeOfRelAlgSymbol(nl->First(nl->Second(streamB))) == mtuple)
          {
            list2 = nl->Second(nl->Second(streamB));
          }
          else goto typeerror;
        }
        else goto typeerror;
      }
      else goto typeerror;
    }
    else goto typeerror;

    if(!CcAttributesAreDisjoint(list1, list2))
    {
      goto typeerror;
    }

    list = CcConcatLists(list1, list2);
    outlist = nl->TwoElemList(nl->SymbolAtom("stream"),
      nl->TwoElemList(nl->SymbolAtom("mtuple"), list));

    ListExpr joinAttrDescription;
    if(!(nl->IsAtom(nl->Third(args))
        && nl->IsAtom(nl->Fourth(args))
        && nl->AtomType(nl->Third(args)) == SymbolType
        && nl->AtomType(nl->Fourth(args)) == SymbolType))
    {
      goto typeerror;
    }

    string attrAName = nl->SymbolValue(nl->Third(args));
    string attrBName = nl->SymbolValue(nl->Fourth(args));
    int attrAIndex = CcFindAttribute(nl->Second(nl->Second(streamA)), attrAName, attrTypeA, nl);
    int attrBIndex = CcFindAttribute(nl->Second(nl->Second(streamB)), attrBName, attrTypeB, nl);
    if(attrAIndex <= 0 || attrBIndex <= 0 || !nl->Equal(attrTypeA, attrTypeB))
    {
      goto typeerror;
    }

    if(expectIntArgument && nl->SymbolValue(nl->Fifth(args)) != "int")
    {
      goto typeerror;
    }

    joinAttrDescription =
      nl->TwoElemList(nl->IntAtom(attrAIndex), nl->IntAtom(attrBIndex));
    return nl->ThreeElemList(nl->SymbolAtom("APPEND"),
              joinAttrDescription, outlist);
  }
  else goto typeerror;

typeerror:
  ErrorReporter::ReportError(ccJoinErrorMessages[errorMessageIdx]);
  return nl->SymbolAtom("typeerror");
}

/*

4.1.2 Auxiliary definitions for value mapping function of operator ~mergejoin~

*/

static CcInt oneCcInt(true, 1);
static CcBool trueCcBool(true, true);

CPUTimeMeasurer ccMergeMeasurer;

class CcMergeJoinLocalInfo
{
private:
  vector<CcTuple*> bucketA;
  vector<CcTuple*> bucketB;
  deque<CcTuple*> resultBucket;

  Word aResult;
  Word bResult;

  Word streamALocalInfo;
  Word streamBLocalInfo;

  Word streamA;
  Word streamB;

  ArgVector aArgs;
  ArgVector bArgs;

  int attrIndexA;
  int attrIndexB;

  bool expectSorted;

  int CompareCcTuples(CcTuple* a, CcTuple* b)
  {
    /* tuples with NULL-Values in the join attributes
       are never matched with other tuples. */
    if(!((Attribute*)a->Get(attrIndexA))->IsDefined())
    {
      return -1;
    }
    if(!((Attribute*)b->Get(attrIndexB))->IsDefined())
    {
      return 1;
    }

    return ((Attribute*)a->Get(attrIndexA))->Compare((Attribute*)b->Get(attrIndexB));
  }

  void SetArgs(ArgVector& args, Word stream, Word attrIndex)
  {
    args[0] = SetWord(stream.addr);
    args[2] = SetWord(&oneCcInt);
    args[3] = SetWord(attrIndex.addr);
    args[4] = SetWord(&trueCcBool);
  }

  CcTuple* nextATuple()
  {
    bool yield;
    if(expectSorted)
    {
      qp->Request(streamA.addr, aResult);
      yield = qp->Received(streamA.addr);
    }
    else
    {
      int errorCode = CcSortBy<false, false>(aArgs, aResult, REQUEST, streamALocalInfo, 0);
      yield = (errorCode == YIELD);
    }

    if(yield)
    {
      return (CcTuple*)aResult.addr;
    }
    else
    {
      aResult = SetWord((void*)0);
      return 0;
    }
  }

  CcTuple* nextBTuple()
  {
    bool yield;
    if(expectSorted)
    {
      qp->Request(streamB.addr, bResult);
      yield = qp->Received(streamB.addr);
    }
    else
    {
      int errorCode = CcSortBy<false, false>(bArgs, bResult, REQUEST, streamBLocalInfo, 0);
      yield = (errorCode == YIELD);
    }

    if(yield)
    {
      return (CcTuple*)bResult.addr;
    }
    else
    {
      bResult = SetWord((void*)0);
      return 0;
    }
  }

  bool FetchNextMatch()
  {
    CcTuple* aCcTuple = (CcTuple*)aResult.addr;
    CcTuple* bCcTuple = (CcTuple*)bResult.addr;
    if(aCcTuple == 0 || bCcTuple == 0)
    {
      if(aCcTuple != 0)
        aCcTuple->DeleteIfAllowed();
      if(bCcTuple != 0)
        bCcTuple->DeleteIfAllowed();

      return false;
    }
    else
    {
      int cmpResult = CompareCcTuples((CcTuple*)aResult.addr, (CcTuple*)bResult.addr);
      while(cmpResult != 0)
      {
        if(cmpResult < 0)
        {
          ((CcTuple*)aResult.addr)->DeleteIfAllowed();
          if(nextATuple() == 0)
          {
            ((CcTuple*)bResult.addr)->DeleteIfAllowed();
            return false;
          }
        }
        else
        {
          ((CcTuple*)bResult.addr)->DeleteIfAllowed();
          if(nextBTuple() == 0)
          {
            ((CcTuple*)aResult.addr)->DeleteIfAllowed();
            return false;
          }
        }
        cmpResult = CompareCcTuples((CcTuple*)aResult.addr, (CcTuple*)bResult.addr);
      }
      return true;
    }
  }

  void ComputeProductOfBuckets()
  {
    assert(!bucketA.empty());
    assert(!bucketB.empty());

    vector<CcTuple*>::iterator iterA = bucketA.begin();
    vector<CcTuple*>::iterator iterB = bucketB.begin();
    for(; iterA != bucketA.end(); iterA++)
    {
      for(iterB = bucketB.begin(); iterB != bucketB.end(); iterB++)
      {
        CcTuple* resultTuple = new CcTuple;
        resultTuple->SetFree(true);
        Word resultWord = SetWord(resultTuple);
        Word aWord = SetWord(*iterA);
        Word bWord = SetWord(*iterB);
        CcConcat(aWord, bWord, resultWord);
        resultBucket.push_back(resultTuple);
      }
    }
  }

  void ClearBuckets()
  {
    vector<CcTuple*>::iterator iterA = bucketA.begin();
    vector<CcTuple*>::iterator iterB = bucketB.begin();

    for(; iterA != bucketA.end(); iterA++)
    {
      (*iterA)->DeleteIfAllowed();
    }

    for(; iterB != bucketB.end(); iterB++)
    {
      (*iterB)->DeleteIfAllowed();
    }

    bucketA.clear();
    bucketB.clear();
  }

  void FillResultBucket()
  {
    assert((CcTuple*)aResult.addr != 0);
    assert((CcTuple*)bResult.addr != 0);

    CcTuple* aMatch = (CcTuple*)aResult.addr;
    CcTuple* bMatch = (CcTuple*)bResult.addr;
    assert(CompareCcTuples(aMatch, bMatch) == 0);

    CcTuple* currentA = aMatch;
    CcTuple* currentB = bMatch;

    while(currentA != 0 && CompareCcTuples(currentA, bMatch) == 0)
    {
      bucketA.push_back(currentA);
      currentA = nextATuple();
    }

    while(currentB != 0 && CompareCcTuples(aMatch, currentB) == 0)
    {
      bucketB.push_back(currentB);
      currentB = nextBTuple();
    }

    ComputeProductOfBuckets();
    ClearBuckets();
  }

public:
  CcMergeJoinLocalInfo(Word streamA, Word attrIndexA,
    Word streamB, Word attrIndexB, bool expectSorted)
  {
    assert(streamA.addr != 0);
    assert(streamB.addr != 0);
    assert(attrIndexA.addr != 0);
    assert(attrIndexB.addr != 0);
    assert(((CcInt*)attrIndexA.addr)->GetIntval() > 0);
    assert(((CcInt*)attrIndexB.addr)->GetIntval() > 0);

    aResult = SetWord(0);
    bResult = SetWord(0);

    this->expectSorted = expectSorted;
    this->streamA = streamA;
    this->streamB = streamB;
    this->attrIndexA = ((CcInt*)attrIndexA.addr)->GetIntval() - 1;
    this->attrIndexB = ((CcInt*)attrIndexB.addr)->GetIntval() - 1;

    if(expectSorted)
    {
      qp->Open(streamA.addr);
      qp->Open(streamB.addr);
    }
    else
    {
      SetArgs(aArgs, streamA, attrIndexA);
      SetArgs(bArgs, streamB, attrIndexB);
      CcSortBy<false, false>(aArgs, aResult, OPEN, streamALocalInfo, 0);
      CcSortBy<false, false>(bArgs, bResult, OPEN, streamBLocalInfo, 0);
    }

    nextATuple();
    nextBTuple();
  }

  ~CcMergeJoinLocalInfo()
  {
    if(expectSorted)
    {
      qp->Close(streamA.addr);
      qp->Close(streamB.addr);
    }
    else
    {
      CcSortBy<false, false>(aArgs, aResult, CLOSE, streamALocalInfo, 0);
      CcSortBy<false, false>(bArgs, bResult, CLOSE, streamBLocalInfo, 0);
    };
  }

  CcTuple* NextResultTuple()
  {
    if(resultBucket.empty())
    {
      if(FetchNextMatch())
      {
        FillResultBucket();
      }
      else
      {
        return 0;
      }
    }
    CcTuple* next = resultBucket.front();
    resultBucket.pop_front();
    return next;
  }
};

/*

4.1.2 Value mapping function of operator ~mergejoin~

*/

template<bool expectSorted> int
CcMergeJoin(Word* args, Word& result, int message, Word& local, Supplier s)
{
  CcMergeJoinLocalInfo* localInfo;
  Word attrIndexA;
  Word attrIndexB;

  switch(message)
  {
    case OPEN:
      qp->Request(args[4].addr, attrIndexA);
      qp->Request(args[5].addr, attrIndexB);
      localInfo = new CcMergeJoinLocalInfo
        (args[0], attrIndexA, args[1], attrIndexB, expectSorted);
      local = SetWord(localInfo);
      return 0;
    case REQUEST:
      ccMergeMeasurer.Enter();
      localInfo = (CcMergeJoinLocalInfo*)local.addr;
      result = SetWord(localInfo->NextResultTuple());
      ccMergeMeasurer.Exit();
      return result.addr != 0 ? YIELD : CANCEL;
    case CLOSE:
      ccMergeMeasurer.PrintCPUTimeAndReset("CPU Time for Merging Tuples : ");

      localInfo = (CcMergeJoinLocalInfo*)local.addr;
      delete localInfo;
      return 0;
  }
  return 0;
}

/*

4.1.3 Specification of operator ~mergejoin~

*/
const string CcMergeJoinSpec  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                              "\"Example\" ) "
                             "( <text>((stream (mtuple ((x1 t1) ... "
			     "(xn tn)))) (stream (mtuple ((y1 d1) ... "
			     "(ym dm)))) xi yj) -> (stream (mtuple ((x1 t1)"
			     " ... (xn tn) (y1 d1) ... (ym dm))))"
			     "</text--->"
			     "<text>_ _ mergejoin [_, _]</text--->"
			     "<text>Computes the equijoin two streams. "
			     "Expects that input streams are sorted."
			     "</text--->"
			     "<text>query duplicates feed ten feed "
			     "rename[A] mergejoin[no, no_A] sort rdup "
			     "consume</text--->"
			      ") )";
/*

4.1.3 Definition of operator ~mergejoin~

*/
Operator ccrelmergejoin (
         "mergejoin",        // name
         CcMergeJoinSpec,     // specification
         CcMergeJoin<true>,         // value mapping
         Operator::SimpleSelect,          // trivial selection function
         CcJoinTypeMap<false, 0>   // type mapping
);

/*

7.3 Operator ~sortmergejoin~

This operator sorts two input streams and computes their equijoin.

4.1.3 Specification of operator ~sortmergejoin~

*/
const string CcSortMergeJoinSpec  = "( ( \"Signature\" \"Syntax\" "
                                  "\"Meaning\" \"Example\" ) "
                             "( <text>((stream (mtuple ((x1 t1) ... "
			     "(xn tn)))) (stream (mtuple ((y1 d1) ..."
			     " (ym dm)))) xi yj) -> (stream (mtuple "
			     "((x1 t1) ... (xn tn) (y1 d1) ... (ym dm)"
			     ")))</text--->"
			     "<text>_ _ sortmergejoin [ _ , _ ]"
			     "</text--->"
			     "<text>Computes the equijoin two streams."
			     "</text--->"
			     "<text>query duplicates feed ten feed "
			     "mergejoin[no, nr] consume</text--->"
			      ") )";

/*

4.1.3 Definition of operator ~sortmergejoin~

*/
Operator ccrelsortmergejoin (
         "sortmergejoin",        // name
         CcSortMergeJoinSpec,     // specification
         CcMergeJoin<false>,         // value mapping
         Operator::SimpleSelect,          // trivial selection function
         CcJoinTypeMap<false, 1>   // type mapping
);


/*

7.3 Operator ~hashjoin~

This operator computes the equijoin two streams via a hash join.
The user can specify the number of hash buckets.

7.3.1 Auxiliary Class for Operator ~hashjoin~

*/

CPUTimeMeasurer ccHashMeasurer;  // measures cost of distributing into buckets and
                               // of computing products of buckets
CPUTimeMeasurer ccBucketMeasurer;// measures the cost of producing the tuples in
                               // the result set

class CcHashJoinLocalInfo
{
private:
  static const size_t MAX_BUCKETS = 6151;
  static const size_t MIN_BUCKETS = 1;
  static const size_t DEFAULT_BUCKETS = 97;
  size_t nBuckets;
  size_t currentBucket;

  int attrIndexA;
  int attrIndexB;

  Word streamA;
  Word streamB;

  vector<vector< CcTuple*> > bucketsA;
  vector<vector< CcTuple*> > bucketsB;
  vector<CcTuple*> resultBucket;

  int CompareCcTuples(CcTuple* a, CcTuple* b)
  {
    /* tuples with NULL-Values in the join attributes
       are never matched with other tuples. */
    if(!((Attribute*)a->Get(attrIndexA))->IsDefined())
    {
      return -1;
    }
    if(!((Attribute*)b->Get(attrIndexB))->IsDefined())
    {
      return 1;
    }

    return ((Attribute*)a->Get(attrIndexA))->
      Compare((Attribute*)b->Get(attrIndexB));
  }

  size_t HashTuple(CcTuple* tuple, int attrIndex)
  {
    return (((StandardAttribute*)tuple->Get(attrIndex))->HashValue() % nBuckets);
  }

  void FillHashBuckets(Word stream, int attrIndex,
    vector<vector< CcTuple*> >& buckets)
  {
    Word tupleWord;
    qp->Open(stream.addr);
    qp->Request(stream.addr, tupleWord);
    while(qp->Received(stream.addr))
    {
      ccHashMeasurer.Enter();

      CcTuple* tuple = (CcTuple*)tupleWord.addr;
      buckets[HashTuple(tuple, attrIndex)].push_back(tuple);

      ccHashMeasurer.Exit();

      qp->Request(stream.addr, tupleWord);
    }
    qp->Close(stream.addr);
  }

  void ClearBucket(vector<CcTuple*>& bucket)
  {
    vector<CcTuple*>::iterator iter = bucket.begin();
    while(iter != bucket.end())
    {
      (*iter)->DeleteIfAllowed();
      iter++;
    }
  }

  bool FillResultBucket()
  {
    ccHashMeasurer.Enter();

    while(resultBucket.empty() && currentBucket < nBuckets)
    {
      vector<CcTuple*>& a = bucketsA[currentBucket];
      vector<CcTuple*>& b = bucketsB[currentBucket];

      vector<CcTuple*>::iterator iterA = a.begin();
      vector<CcTuple*>::iterator iterB;
      for(; iterA != a.end(); iterA++)
      {
        for(iterB = b.begin(); iterB != b.end(); iterB++)
        {
          if(CompareCcTuples(*iterA, *iterB) == 0)
          {
            ccHashMeasurer.Exit();
            ccBucketMeasurer.Enter();

            CcTuple* resultTuple = new CcTuple;
            resultTuple->SetFree(true);
            Word resultWord = SetWord(resultTuple);
            Word aWord = SetWord(*iterA);
            Word bWord = SetWord(*iterB);
            CcConcat(aWord, bWord, resultWord);
            resultBucket.push_back(resultTuple);

            ccBucketMeasurer.Exit();
            ccHashMeasurer.Enter();
          };
        }
      }

      ClearBucket(a);
      ClearBucket(b);
      currentBucket++;
    }

    ccHashMeasurer.Exit();
    return !resultBucket.empty();
  };

public:
  CcHashJoinLocalInfo(Word streamA, Word attrIndexAWord,
    Word streamB, Word attrIndexBWord, Word nBucketsWord)
  {
    this->streamA = streamA;
    this->streamB = streamB;
    currentBucket = 0;

    attrIndexA = ((CcInt*)attrIndexAWord.addr)->GetIntval() - 1;
    attrIndexB = ((CcInt*)attrIndexBWord.addr)->GetIntval() - 1;
    nBuckets = ((CcInt*)nBucketsWord.addr)->GetIntval();
    if(nBuckets < MIN_BUCKETS)
    {
      nBuckets = MIN_BUCKETS;
    }
    else if(nBuckets > MAX_BUCKETS)
    {
      nBuckets = MAX_BUCKETS;
    }

    ccHashMeasurer.Enter();

    bucketsA.resize(nBuckets);
    bucketsB.resize(nBuckets);

    ccHashMeasurer.Exit();

    FillHashBuckets(streamA, attrIndexA, bucketsA);
    FillHashBuckets(streamB, attrIndexB, bucketsB);
  }

  ~CcHashJoinLocalInfo()
  {
  }

  CcTuple* NextResultTuple()
  {
    if(resultBucket.empty())
    {
      if(!FillResultBucket())
      {
        return 0;
      }
    }
    CcTuple* result = resultBucket.back();
    resultBucket.pop_back();
    return result;
  }
};

/*

7.3.2 Value Mapping Function of Operator ~hashjoin~

*/

static int
CcHashJoin(Word* args, Word& result, int message, Word& local, Supplier s)
{
  CcHashJoinLocalInfo* localInfo;
  Word attrIndexA;
  Word attrIndexB;
  Word nHashBuckets;

  switch(message)
  {
    case OPEN:
      qp->Request(args[5].addr, attrIndexA);
      qp->Request(args[6].addr, attrIndexB);
      qp->Request(args[4].addr, nHashBuckets);
      localInfo = new CcHashJoinLocalInfo(args[0], attrIndexA,
        args[1], attrIndexB, nHashBuckets);
      local = SetWord(localInfo);
      return 0;
    case REQUEST:
      localInfo = (CcHashJoinLocalInfo*)local.addr;
      result = SetWord(localInfo->NextResultTuple());
      return result.addr != 0 ? YIELD : CANCEL;
    case CLOSE:
      ccHashMeasurer.PrintCPUTimeAndReset("CPU Time for Hashing Tuples : ");
      ccBucketMeasurer.PrintCPUTimeAndReset(
        "CPU Time for Computing Products of Buckets : ");

      localInfo = (CcHashJoinLocalInfo*)local.addr;
      delete localInfo;
      return 0;
  }
  return 0;
}

/*

4.1.3 Specification of Operator ~hashjoin~

*/
const string CcHashJoinSpec  = "( ( \"Signature\" \"Syntax\" "
                             "\"Meaning\" \"Example\" ) "
                          "( <text>((stream (mtuple ((x1 t1) ... "
			  "(xn tn)))) (stream (mtuple ((y1 d1) ... "
			  "(ym dm)))) xi yj nbuckets) -> (stream "
			  "(mtuple ((x1 t1) ... (xn tn) (y1 d1) ..."
			  " (ym dm))))</text--->"
			  "<text> _ _ hashjoin [ _ , _ , _ ]"
			  "</text--->"
			  "<text>Computes the equijoin two streams "
			  "via a hash join. The number of hash buckets"
			  " is given by the parameter nBuckets."
			  "</text--->"
			  "<text>query Employee feed Dept feed "
			  "rename[A] hashjoin[DeptNr, DeptNr_A, 17] "
			  "sort consume</text--->"
			      ") )";
/*

4.1.3 Definition of Operator ~hashjoin~

*/
Operator ccrelhashjoin (
         "hashjoin",        // name
         CcHashJoinSpec,     // specification
         CcHashJoin,         // value mapping
         Operator::SimpleSelect,          // trivial selection function
         CcJoinTypeMap<true, 2>   // type mapping
);

/*

7.3 Operator ~extend~

Extends each input tuple by new attributes as specified in the parameter list.

7.3.1 Type mapping function of operator ~extend~

Type mapping for ~extend~ is

----     ((stream x) ((b1 (map x y1)) ... (bm (map x ym))))

        -> (stream (mtuple ((a1 x1) ... (an xn) (b1 y1 ... bm ym))))

        where x = (mtuple ((a1 x1) ... (an xn)))
----

*/
static ListExpr
CcExtendTypeMap( ListExpr args )
{
  ListExpr first, second, rest, listn, errorInfo,
           lastlistn, first2, second2, firstr, outlist;
  bool loopok;
  AlgebraManager* algMgr;

  algMgr = SecondoSystem::GetAlgebraManager();
  errorInfo = nl->OneElemList(nl->SymbolAtom("ERROR"));
  if(nl->ListLength(args) == 2)
  {
    first = nl->First(args);
    second  = nl->Second(args);
    if((nl->ListLength(first) == 2)  &&
      (CcTypeOfRelAlgSymbol(nl->First(first)) == mstream) &&
      (!nl->IsAtom(second)) &&
      (nl->ListLength(second) > 0))
    {
      rest = nl->Second(nl->Second(first));
      listn = nl->OneElemList(nl->First(rest));
      lastlistn = listn;
      rest = nl->Rest(rest);
      while (!(nl->IsEmpty(rest)))
      {
        lastlistn = nl->Append(lastlistn,nl->First(rest));
        rest = nl->Rest(rest);
      }
      loopok = true;
      rest = second;
      while (!(nl->IsEmpty(rest)))
      {
        firstr = nl->First(rest);
        rest = nl->Rest(rest);
        first2 = nl->First(firstr);
        second2 = nl->Second(firstr);
        if ((nl->IsAtom(first2)) &&
            (nl->ListLength(second2) == 3) &&
            (nl->AtomType(first2) == SymbolType) &&
            (CcTypeOfRelAlgSymbol(nl->First(second2)) == mmap) &&
            (algMgr->CheckKind("DATA", nl->Third(second2), errorInfo)) &&
                  (nl->Equal(nl->Second(first),nl->Second(second2))))
        {
          lastlistn = nl->Append(lastlistn,
            (nl->TwoElemList(first2,nl->Third(second2))));
        }
        else
        {
          loopok = false;
        }
      }
      if ((loopok) && (CcCompareNames(listn)))
      {
        outlist =
          nl->TwoElemList(
            nl->SymbolAtom("stream"),
            nl->TwoElemList(nl->SymbolAtom("mtuple"),listn));
        return outlist;
      }
      else
      {
        ErrorReporter::ReportError("Incorrect input for operator extend.");
        return nl->SymbolAtom("typeerror");
      }
    }
    else
    {
      ErrorReporter::ReportError("Incorrect input for operator extend.");
      return nl->SymbolAtom("typeerror");
    }
  }
  ErrorReporter::ReportError("Incorrect input for operator extend.");
  return nl->SymbolAtom("typeerror");
}
/*

4.1.2 Value mapping function of operator ~extend~

*/
static int
CcExtend(Word* args, Word& result, int message, Word& local, Supplier s)
{
  Word t, value;
  CcTuple* tup;
  Supplier supplier, supplier2, supplier3;
  int noofoldattrs, nooffun, noofsons;
  ArgVectorPointer funargs;

  switch (message)
  {
    case OPEN :

      qp->Open(args[0].addr);
      return 0;

    case REQUEST :

      qp->Request(args[0].addr,t);
      if (qp->Received(args[0].addr))
      {
        tup = (CcTuple*)t.addr;
        tup = tup->CloneIfNecessary();
        noofoldattrs = tup->GetNoAttrs();
        supplier = args[1].addr;
        nooffun = qp->GetNoSons(supplier);
        for (int i=0; i < nooffun;i++)
        {
          supplier2 = qp->GetSupplier(supplier, i);
          noofsons = qp->GetNoSons(supplier2);
          supplier3 = qp->GetSupplier(supplier2, 1);
          funargs = qp->Argument(supplier3);
          (*funargs)[0] = SetWord(tup);
          qp->Request(supplier3,value);
          tup->Put(noofoldattrs+i,((StandardAttribute*)value.addr)->Clone());
        }

        tup->SetNoAttrs(noofoldattrs + nooffun);
        result = SetWord(tup);
        return YIELD;
      }
      else
        return CANCEL;

    case CLOSE :

      qp->Close(args[0].addr);
      return 0;
  }
  return 0;
}
/*

4.1.3 Specification of operator ~extend~

*/
const string CcExtendSpec  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                           "\"Example\" ) "
                           "( <text>(stream(mtuple(x)) x [(a1, (mtuple(x)"
			   " -> d1)) ... (an, (mtuple(x) -> dn))] -> "
			   "stream(mtuple(x@[a1:d1, ... , an:dn])))"
			   "</text--->"
			   "<text>_ extend [funlist]</text--->"
			   "<text>Extends each input tuple by new "
			   "attributes as specified in the parameter"
			   " list.</text--->"
			   "<text>query ten feed extend [mult5 : "
			   ".nr * 5, mod2 : .nr mod 2] consume"
			   "</text--->"
			      ") )";
/*

4.1.3 Definition of operator ~extend~

*/
Operator ccrelextend (
         "extend",              // name
         CcExtendSpec,            // specification
         CcExtend,                // value mapping
         Operator::SimpleSelect,          // trivial selection function
         CcExtendTypeMap          // type mapping
);

/*

7.3 Operator ~loopjoin~

This operator will fulfill a join of two relations. Tuples in the cartesian product which satisfy certain
conditions are passed on to the output stream.

For instance,

----	query Staedte feed loopjoin [plz feed filter [.Ort=.SName] ] consume;

    	(query (consume (loopjoin (feed tryrel) (fun (t1 TUPLE) (filter (feed null)
		(fun t2 TUPLE) (= (attr t1 name) (attr t2 pname)))))))
----

7.3.1 Type mapping function of operator ~loopjoin~

The type mapping function of the loopjoin operation is as follows:

----    ((stream (mtuple x)) (map (mtuple x) (stream (mtuple y))))  -> (stream (mtuple x * y))
	where x = ((x1 t1) ... (xn tn)) and y = ((y1 d1) ... (ym dm))
----

*/
ListExpr 
CcLoopjoinTypeMap(ListExpr args)
{
  ListExpr first, second;
  ListExpr list1, list2, list, outlist;

  if(nl->ListLength(args) == 2)
  {
    first = nl->First(args);
    second  = nl->Second(args);

    if 	    ( (nl->ListLength(first) == 2)
	&& (CcTypeOfRelAlgSymbol(nl->First(first)) == mstream)
	&& (nl->ListLength(nl->Second(first)) == 2)
	&& (CcTypeOfRelAlgSymbol(nl->First(nl->Second(first))) == mtuple)
	&& (nl->ListLength(second) == 3)
	&& (CcTypeOfRelAlgSymbol(nl->First(second)) == mmap)
	&& (nl->Equal(nl->Second(first), nl->Second(second)))
	&& (nl->ListLength(nl->Third(second)) == 2)
	&& (CcTypeOfRelAlgSymbol(nl->First(nl->Third(second))) == mstream)
	&& (nl->ListLength(nl->Second(nl->Third(second))) == 2)
	&& (CcTypeOfRelAlgSymbol(nl->First(nl->Second(nl->Third(second)))) == mtuple) )
	{
                   list1 = nl->Second(nl->Second(first));
                   list2 = nl->Second(nl->Second(nl->Third(second)));
	   if(!CcAttributesAreDisjoint(list1, list2))
	   {
	       goto typeerror;
	   }
	   list = CcConcatLists(list1, list2);
	   outlist = nl->TwoElemList(nl->SymbolAtom("stream"),
		nl->TwoElemList(nl->SymbolAtom("mtuple"), list));
  	   return outlist;
	}
    else goto typeerror;
  }
 else goto typeerror;

typeerror:
  ErrorReporter::ReportError("Incorrect input for operator loopjoin.");
  return nl->SymbolAtom("typeerror");
}

/*

4.1.2 Value mapping function of operator ~loopjoin~

*/

struct CcLoopjoinLocalInfo
{
    Word tuplex;
    Word streamy;
};

static int
CcLoopjoin(Word* args, Word& result, int message, Word& local, Supplier s)
{
  ArgVectorPointer funargs;
  Word tuplex, tupley, tuplexy, streamy;
  CcTuple* ctuplex;
  CcTuple* ctupley;
  CcTuple* ctuplexy;
  CcLoopjoinLocalInfo *localinfo;

  switch ( message )
  {
    case OPEN:
      //1.open the stream and initiate the variables
      qp->Open (args[0].addr);
      qp->Request(args[0].addr, tuplex);
      if (qp->Received(args[0].addr))
      {
      //2>>> here: compute the rely which corresponding to tuplex
          funargs = qp->Argument(args[1].addr);
          (*funargs)[0] = tuplex;
          streamy=args[1];
          qp->Open (streamy.addr);
     //3>>> here: put the information of tuplex and rely into local
           localinfo=new CcLoopjoinLocalInfo;
           localinfo->tuplex=tuplex;
           localinfo->streamy=streamy;
           local = SetWord(localinfo);
      }
      else
      {
           local = SetWord(Address(0));
       }
      return 0;

    case REQUEST:
      if (local.addr ==0) return CANCEL;
      //1>>>>>>restore localinformation from the local variable.
      localinfo=(CcLoopjoinLocalInfo *) local.addr;
      tuplex=localinfo->tuplex;
      ctuplex=(CcTuple*)tuplex.addr;
      streamy=localinfo->streamy;
      //2>>>>>> prepare tuplex and tupley for processing. if rely is exausted: fetch next tuplex.
      tupley=SetWord(Address(0));
      while (tupley.addr==0)
      {
           qp->Request(streamy.addr, tupley);
           if (!(qp->Received(streamy.addr)))
             {
	    qp->Close(streamy.addr);
	    ((CcTuple*)tuplex.addr)->DeleteIfAllowed();
	    qp->Request(args[0].addr, tuplex);
	    if (qp->Received(args[0].addr))
	    {
               funargs = qp->Argument(args[1].addr);
	       ctuplex=(CcTuple*)tuplex.addr;
	       (*funargs)[0] = tuplex;
               streamy=args[1];
               qp->Open (streamy.addr);
	       tupley=SetWord(Address(0));

 	       localinfo->tuplex=tuplex;
 	       localinfo->streamy=streamy;
	       local =  SetWord(localinfo);
	   }
	   else return CANCEL;
           }
          else
           {
	    ctupley=(CcTuple*)tupley.addr;
            }
      }
      //3>>>>>> compute tuplexy.
      ctuplexy = new CcTuple();
      ctuplexy->SetFree(true);
      tuplexy = SetWord(ctuplexy);
      CcConcat(tuplex, tupley, tuplexy);
      ((CcTuple*)tupley.addr)->DeleteIfAllowed();
      result = tuplexy;
      return YIELD;

    case CLOSE:
      qp->Close(args[0].addr);
      localinfo=(CcLoopjoinLocalInfo *) local.addr;
      delete localinfo;
      return 0;
  }

  return 0;
}
/*

4.1.3 Specification of operator ~loopjoin~

*/
const string CcLoopjoinSpec  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                             "\"Example\" ) "
                        "( <text>((stream mtuple1) (map mtuple1 "
			"mrel(mtuple2))) -> (stream mtuple1*mtuple2)"
			"</text--->"
			"<text>_ loopjoin [ fun ]</text--->"
			"<text>Only tuples in the cartesian product "
			"which satisfy certain conditions are passed on"
			" to the output stream.</text--->"
			"<text>query cities feed loopjoin [five feed "
			"filter [.no > 2]] consume</text--->"
			      ") )";
/*

4.1.3 Definition of operator ~loopjoin~

*/
Operator ccrelloopjoin (
         "loopjoin",	           		// name
         CcLoopjoinSpec,          		// specification
         CcLoopjoin,               		// value mapping
         Operator::SimpleSelect,         		// trivial selection function
         CcLoopjoinTypeMap	         	// type mapping
);

/*

7.4 Operator ~loopselect~

This operator is similar to the ~loopjoin~ operator except that it only returns the inner tuple
(instead of the concatination of two tuples). Tuples in the cartesian product which satisfy
certain conditions are passed on to the output stream.

For instance,

----	query Staedte feed loopselect [plz feed filter [.Ort=.SName] ] consume;

    	(query (consume (loopselect (feed tryrel) (fun (t1 TUPLE) (filter (feed null)
		(fun t2 TUPLE) (= (attr t1 name) (attr t2 pname)))))))
----

7.4.1 Type mapping function of operator ~loopselect~

The type mapping function of the loopjoin operation is as follows:

----    ((stream (mtuple x)) (map (mtuple x) (stream (mtuple y))))  -> (stream (mtuple y))
	where x = ((x1 t1) ... (xn tn)) and y = ((y1 d1) ... (ym dm))
----

*/
ListExpr 
CcLoopselectTypeMap(ListExpr args)
{
  ListExpr first, second;
  ListExpr list1, list2, outlist;

  if(nl->ListLength(args) == 2)
  {
    first = nl->First(args);
    second  = nl->Second(args);

    if 	    ( (nl->ListLength(first) == 2)
	&& (CcTypeOfRelAlgSymbol(nl->First(first)) == mstream)
	&& (nl->ListLength(nl->Second(first)) == 2)
	&& (CcTypeOfRelAlgSymbol(nl->First(nl->Second(first))) == mtuple)
	&& (nl->ListLength(second) == 3)
	&& (CcTypeOfRelAlgSymbol(nl->First(second)) == mmap)
	&& (nl->Equal(nl->Second(first), nl->Second(second)))
	&& (nl->ListLength(nl->Third(second)) == 2)
	&& (CcTypeOfRelAlgSymbol(nl->First(nl->Third(second))) == mstream)
	&& (nl->ListLength(nl->Second(nl->Third(second))) == 2)
	&& (CcTypeOfRelAlgSymbol(nl->First(nl->Second(nl->Third(second)))) == mtuple) )
	{
                   list1 = nl->Second(nl->Second(first));
                   list2 = nl->Second(nl->Second(nl->Third(second)));
	   //if(!CcAttributesAreDisjoint(list1, list2))
	   //{
	   //    goto typeerror;
	   //}
	   //list = CcConcatLists(list1, list2);
	   outlist = nl->TwoElemList(nl->SymbolAtom("stream"),
		nl->TwoElemList(nl->SymbolAtom("mtuple"), list2));
  	   return outlist;
	}
    else goto typeerror;
  }
 else goto typeerror;

typeerror:
  ErrorReporter::ReportError("Incorrect input for operator loopselect.");
  return nl->SymbolAtom("typeerror");
}

/*

4.1.2 Value mapping function of operator ~loopjoin~

*/

struct CcLoopselectLocalInfo
{
    Word tuplex;
    Word streamy;
};

static int
CcLoopselect(Word* args, Word& result, int message, Word& local, Supplier s)
{
  ArgVectorPointer funargs;
  Word tuplex, tupley, streamy;  //tuplexy
  CcTuple* ctuplex;
  CcTuple* ctupley;
  //CcTuple* ctuplexy;
  CcLoopselectLocalInfo *localinfo;

  switch ( message )
  {
    case OPEN:
      //1.open the stream and initiate the variables
      qp->Open (args[0].addr);
      qp->Request(args[0].addr, tuplex);
      if (qp->Received(args[0].addr))
      {
      //2>>> here: compute the rely which corresponding to tuplex
          funargs = qp->Argument(args[1].addr);
          (*funargs)[0] = tuplex;
          streamy=args[1];
          qp->Open (streamy.addr);
     //3>>> here: put the information of tuplex and rely into local
           localinfo=new CcLoopselectLocalInfo;
           localinfo->tuplex=tuplex;
           localinfo->streamy=streamy;
           local = SetWord(localinfo);
      }
      else
      {
           local = SetWord(Address(0));
       }
      return 0;

    case REQUEST:
      if (local.addr ==0) return CANCEL;
      //1>>>>>>restore localinformation from the local variable.
      localinfo=(CcLoopselectLocalInfo *) local.addr;
      tuplex=localinfo->tuplex;
      ctuplex=(CcTuple*)tuplex.addr;
      streamy=localinfo->streamy;
      //2>>>>>> prepare tuplex and tupley for processing. if rely is exausted: fetch next tuplex.
      tupley=SetWord(Address(0));
      while (tupley.addr==0)
      {
           qp->Request(streamy.addr, tupley);
           if (!(qp->Received(streamy.addr)))
             {
	    qp->Close(streamy.addr);
	    ((CcTuple*)tuplex.addr)->DeleteIfAllowed();
	    qp->Request(args[0].addr, tuplex);
	    if (qp->Received(args[0].addr))
	    {
                       funargs = qp->Argument(args[1].addr);
	       ctuplex=(CcTuple*)tuplex.addr;
	       (*funargs)[0] = tuplex;
                       streamy=args[1];
                       qp->Open (streamy.addr);
	       tupley=SetWord(Address(0));

 	       localinfo->tuplex=tuplex;
 	       localinfo->streamy=streamy;
	       local =  SetWord(localinfo);
	   }
	   else return CANCEL;
           }
          else
           {
	    ctupley=(CcTuple*)tupley.addr;
            }
      }
      //3>>>>>> compute tuplexy.
      //ctuplexy = new CcTuple();
      //ctuplexy->SetFree(true);
      //tuplexy = SetWord(ctuplexy);
      //CcConcat(tuplex, tupley, tuplexy);
      //((CcTuple*)tupley.addr)->DeleteIfAllowed();
      result = tupley;
      return YIELD;

    case CLOSE:
      qp->Close(args[0].addr);
      localinfo=(CcLoopselectLocalInfo *) local.addr;
      delete localinfo;
      return 0;
  }

  return 0;
}
/*

4.1.3 Specification of operator ~loopjoin~

*/
const string CcLoopselectSpec  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                             "\"Example\" ) "
                        "( <text>((stream mtuple1) (map mtuple1 "
			"mrel(mtuple2))) -> (stream mtuple2)"
			"</text--->"
			"<text>_ loopselect [ fun ]</text--->"
			"<text>Only tuples in the cartesian product "
			"which satisfy certain conditions are passed on"
			" to the output stream.</text--->"
			"<text>query cities feed loopselect [five feed "
			"filter [.no > 2]] consume</text--->"
			      ") )";
/*

4.1.3 Definition of operator ~loopjoin~

*/
Operator ccrelloopselect (
         "loopsel",	           		// name
         CcLoopselectSpec,         		// specification
         CcLoopselect,               		// value mapping
         Operator::SimpleSelect,         		// trivial selection function
         CcLoopselectTypeMap	         	// type mapping
);


/*

7.3 Operator ~loopjoinrel~

This operator will fulfill a join of two relations. Tuples in the cartesian product which satisfy certain
conditions are passed on to the output stream.

For instance,

----    query Staedte feed loopjoinrel [plz feed filter [.Ort=.SName] consume] consume;

    	(query (consume (loopjoinrel (feed tryrel) (fun (t1 TUPLE)
		(consume filter (feed null) (fun t2 TUPLE) (= (attr t1 name) (attr t2 pname)))))))
----

7.3.1 Type mapping function of operator ~loopjoinrel~

The type mapping function of the loopjoinrel operation is as follows:

----    ((stream (mtuple x)) (map (mtuple x) (mrel (mtuple y))))  -> (stream (mtuple x * y))
	where x = ((x1 t1) ... (xn tn)) and y = ((y1 d1) ... (ym dm))
----

*/
ListExpr 
CcLoopjoinrelTypeMap(ListExpr args)
{
  ListExpr first, second;
  ListExpr list1, list2, list, outlist;

  if(nl->ListLength(args) == 2)
  {
    first = nl->First(args);
    second  = nl->Second(args);

    if 	    ( (nl->ListLength(first) == 2)
	&& (CcTypeOfRelAlgSymbol(nl->First(first)) == mstream)
	&& (nl->ListLength(nl->Second(first)) == 2)
	&& (CcTypeOfRelAlgSymbol(nl->First(nl->Second(first))) == mtuple)
	&& (nl->ListLength(second) == 3)
	&& (CcTypeOfRelAlgSymbol(nl->First(second)) == mmap)
	&& (nl->Equal(nl->Second(first), nl->Second(second)))
	&& (nl->ListLength(nl->Third(second)) == 2)
	&& (CcTypeOfRelAlgSymbol(nl->First(nl->Third(second))) == mrel)
	&& (nl->ListLength(nl->Second(nl->Third(second))) == 2)
	&& (CcTypeOfRelAlgSymbol(nl->First(nl->Second(nl->Third(second)))) == mtuple) )
	{
                   list1 = nl->Second(nl->Second(first));
                   list2 = nl->Second(nl->Second(nl->Third(second)));
	   if(!CcAttributesAreDisjoint(list1, list2))
	   {
	       goto typeerror;
	   }
	   list = CcConcatLists(list1, list2);
	   outlist = nl->TwoElemList(nl->SymbolAtom("stream"),
		nl->TwoElemList(nl->SymbolAtom("mtuple"), list));
  	   return outlist;
	}
    else goto typeerror;
  }
 else goto typeerror;

typeerror:
  ErrorReporter::ReportError("Incorrect input for operator loopjoinrel.");
  return nl->SymbolAtom("typeerror");
}

/*

4.1.2 Value mapping function of operator ~loopjoinrel~

*/

struct CcLoopjoinrelLocalInfo
{
    Word tuplex;
    Word rely;
    Word relyit;
};

static int
CcLoopjoinrel(Word* args, Word& result, int message, Word& local, Supplier s)
{
  ArgVectorPointer funargs;
  Word tuplex, tupley, tuplexy, funresult, rely;
  CcTuple* ctuplex;
  CcTuple* ctupley;
  CcTuple* ctuplexy;
  CcRel* crely;
  CcRelIT* crelyit;
  CcLoopjoinrelLocalInfo *localinfo;

  switch ( message )
  {
    case OPEN:
      //1.open the stream and initiate the variables
      qp->Open (args[0].addr);
      qp->Request(args[0].addr, tuplex);
      if (qp->Received(args[0].addr))
      {
      //2>>> here: compute the rely which corresponding to tuplex
          funargs = qp->Argument(args[1].addr);
          ctuplex=(CcTuple*)tuplex.addr;
          (*funargs)[0] = tuplex;
          qp->Request(args[1].addr, funresult);
          rely=funresult;
          crely = (CcRel*)(funresult.addr);
          crelyit=crely->MakeNewScan();
     //3>>> here: put the information of tuplex and rely into local
           localinfo=new CcLoopjoinrelLocalInfo;
           localinfo->tuplex=tuplex;
           localinfo->rely=rely;
           localinfo->relyit=SetWord(crelyit);
           local = SetWord(localinfo);
      }
      else
      {
           local = SetWord(Address(0));
       }
      return 0;

    case REQUEST:
      if (local.addr ==0) return CANCEL;
      //1>>>>>>restore localinformation from the local variable.
      localinfo=(CcLoopjoinrelLocalInfo *) local.addr;
      tuplex=localinfo->tuplex;
      ctuplex=(CcTuple*)tuplex.addr;
      rely=localinfo->rely;
      crely=((CcRel*)rely.addr);
      crelyit=(CcRelIT*)((localinfo->relyit).addr);
      //2>>>>>> prepare tuplex and tupley for processing. if rely is exausted: fetch next tuplex.
      tupley=SetWord(Address(0));
      while (tupley.addr==0)
      {
           crely=((CcRel*)rely.addr);
           if (crelyit->EndOfScan())
           {
	    ((CcTuple*)tuplex.addr)->DeleteIfAllowed();
	    qp->Request(args[0].addr, tuplex);
	    if (qp->Received(args[0].addr))
	    {
                       funargs = qp->Argument(args[1].addr);
	       ctuplex=(CcTuple*)tuplex.addr;
	       (*funargs)[0] = tuplex;
	       qp->Request(args[1].addr, funresult);
	       rely=SetWord(funresult.addr);
	       crely = (CcRel*)(funresult.addr);
                       crelyit=crely->MakeNewScan();
	       tupley=SetWord(Address(0));
	       //cout<<"number of tuples in rel y:"<<((CcRel*)rely.addr)->GetNoTuples()<<endl;

 	       localinfo->tuplex=tuplex;
 	       localinfo->rely=rely;
	       localinfo->relyit=SetWord(crelyit);
	       local =  SetWord(localinfo);
	   }
	   else return CANCEL;
           }
          else
           {
	    tupley=SetWord(crelyit->GetTuple());
	    ctupley=(CcTuple*)tupley.addr;
	    crelyit->Next();
	    localinfo->relyit=SetWord(crelyit);
	    local =  SetWord(localinfo);
            }
      }
      //3>>>>>> compute tuplexy.
      ctuplexy = new CcTuple();
      ctuplexy->SetFree(true);
      tuplexy = SetWord(ctuplexy);
      CcConcat(tuplex, tupley, tuplexy);
      ((CcTuple*)tupley.addr)->DeleteIfAllowed();
      result = tuplexy;
      return YIELD;

    case CLOSE:
      qp->Close(args[0].addr);
      localinfo=(CcLoopjoinrelLocalInfo *) local.addr;
      delete localinfo;
      return 0;
  }

  return 0;
}
/*

4.1.3 Specification of operator ~loopjoinrel~

*/
const string CcLoopjoinrelSpec  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                                "\"Example\" ) "
                      "( <text>((stream mtuple1) (map mtuple1 mrel(mtuple2))) "
		      "-> (stream mtuple1*mtuple2)</text--->"
		      "<text>_ loopjoin [ fun ]</text--->"
		      "<text>Only tuples in the cartesian product which "
		      "satisfy certain conditions are passed on to the "
		      "output stream.</text--->"
		      "<text>query cities feed loopjoin [ five ] consume"
		      "</text--->"
			     " ) )";
/*

4.1.3 Definition of operator ~loopjoinrel~

*/
Operator ccrelloopjoinrel (
         "loopjoinrel",	           		// name
         CcLoopjoinrelSpec,        		// specification
         CcLoopjoinrel,               		// value mapping
         Operator::SimpleSelect,         		// trivial selection function
         CcLoopjoinrelTypeMap	         	// type mapping
);

/*

7.3 Operator ~concat~

7.3.1 Type mapping function of operator ~concat~

Type mapping for ~concat~ is

----    ((stream (mtuple (a1:d1 ... an:dn))) (stream (mtuple (b1:d1 ... bn:dn))))

        -> (stream (mtuple (a1:d1 ... an:dn)))
----

*/
ListExpr CcGetAttrTypeList (ListExpr l)
{
  ListExpr olist = nl->TheEmptyList();
  ListExpr lastolist = nl->TheEmptyList();
  ListExpr attrlist = l;

  while (!nl->IsEmpty(attrlist))
  {
    ListExpr first = nl->First(attrlist);
    attrlist = nl->Rest(attrlist);
    if (olist == nl->TheEmptyList())
    {
      olist = nl->Cons(nl->Second(first), nl->TheEmptyList());
      lastolist = olist;
    }
    else
    {
      lastolist = nl->Append(lastolist, nl->Second(first));
    }
  }
  return olist;
}

static ListExpr
CcConcatTypeMap( ListExpr args )
{
  ListExpr first, second;
  if(nl->ListLength(args)  == 2)
  {
    first = nl->First(args);
    second = nl->Second(args);

    if((nl->ListLength(first) == 2) &&
       (CcTypeOfRelAlgSymbol(nl->First(first)) == mstream) &&
       (nl->ListLength(nl->Second(first)) == 2) &&
       (CcTypeOfRelAlgSymbol(nl->First(nl->Second(first))) == mtuple) &&
       (nl->ListLength(second) == 2) &&
       (CcTypeOfRelAlgSymbol(nl->First(second)) == mstream) &&
       (nl->ListLength(nl->Second(second)) == 2) &&
       (CcTypeOfRelAlgSymbol(nl->First(nl->Second(second))) == mtuple) &&
       (nl->Equal(CcGetAttrTypeList(nl->Second(nl->Second(first))),
          CcGetAttrTypeList(nl->Second(nl->Second(second))))))
       return first;
    else
    {
      ErrorReporter::ReportError("Incorrect input for operator concat.");
      return nl->SymbolAtom("typeerror");
    }
  }
  ErrorReporter::ReportError("Incorrect input for operator concat.");
  return nl->SymbolAtom("typeerror");
}
/*

4.1.2 Value mapping function of operator ~concat~

*/
static int
CcConcat(Word* args, Word& result, int message, Word& local, Supplier s)
{
  Word t;
  CcTuple* tuple;

  switch (message)
  {
    case OPEN :

      qp->Open(args[0].addr);
      qp->Open(args[1].addr);
      local = SetWord(new CcInt(true, 0));
      return 0;

    case REQUEST :
      if ( (((CcInt*)local.addr)->GetIntval()) == 0)
      {
        qp->Request(args[0].addr, t);
        if (qp->Received(args[0].addr))
        {
          tuple = (CcTuple*)t.addr;
          result = SetWord(tuple);
          return YIELD;
        }
        else
        {
          ((CcInt*)local.addr)->Set(1);
        }
      }
      qp->Request(args[1].addr, t);
      if (qp->Received(args[1].addr))
      {
        tuple = (CcTuple*)t.addr;
        result = SetWord(tuple);
        return YIELD;
      }
      else
        return CANCEL;

    case CLOSE :

      qp->Close(args[0].addr);
      qp->Close(args[1].addr);
      delete (CcInt*)local.addr;
      return 0;
  }
  return 0;
}
/*

4.1.3 Specification of operator ~concat~

*/
const string CcConcatSpec  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                           "\"Example\" ) "
                     "( <text>((stream (mtuple (a1:d1 ... an:dn))) "
		     "(stream (mtuple (b1:d1 ... bn:dn)))) -> (stream"
		     " (mtuple (a1:d1 ... an:dn)))</text--->"
		     "<text>_ _ concat</text--->"
		     "<text>Union.</text--->"
		     "<text>query ten feed five feed concat consume"
		     "</text--->"
			      ") )";
/*

4.1.3 Definition of operator ~concat~

*/
Operator ccrelconcat (
         "concat",              // name
         CcConcatSpec,            // specification
         CcConcat,                // value mapping
         Operator::SimpleSelect,          // trivial selection function
         CcConcatTypeMap          // type mapping
);

/*

7.20 Operator ~groupby~


7.20.1 Type mapping function of operator ~groupby~

Result type of ~groupby~ operation.



*/
ListExpr 
CcGroupByTypeMap(ListExpr args)
{
  // To avoid redundant code the type mapping is 
  // implemented in the file ExtRelationAlgebra.cpp
  return GroupByTypeMap2(args, true);
}

/*

7.20.2 Value mapping function of operator ~groupby~

Remark this implementation causes a SIGSEGV - to be fixed!

*/



int 
CcGroupByValueMapping(Word* args, Word& result, int message, Word& local, Supplier supplier)
{
  CcTuple *t = 0;
  CcTuple *s = 0;
  Word sWord = SetWord(0);
  Word relWord = SetWord(0);
  CcRel* tp = 0;
  CcRelIT* relIter = 0;
  int i=0, j=0, k=0;
  int numberatt = 0;
  bool ifequal = false;
  Word value = SetWord(0);
  Supplier  value2;
  Supplier supplier1;
  Supplier supplier2;
  int ind = 0;
  int noOffun = 0;
  ArgVectorPointer vector;
  const int indexOfCountArgument = 3;
  const int startIndexOfExtraArguments = indexOfCountArgument +1;
  int attribIdx = 0;
  Word nAttributesWord = SetWord(0);
  Word attribIdxWord = SetWord(0);

  switch(message)
  {
    case OPEN: 
      qp->Open (args[0].addr);
      qp->Request(args[0].addr, sWord);
      if (qp->Received(args[0].addr))
      {
        local = SetWord((CcTuple*)sWord.addr);
      }
      else
      {
        local = SetWord(0);
      }
      return 0;

    case REQUEST:
      tp = new CcRel;
      SHOW(tp)
      if(local.addr == 0)
      {
        delete tp;
        return CANCEL;
      }
      else
      {
        t = (CcTuple*)local.addr;
        t = t->Clone();
        t->SetFree(false);
        SHOW(t)
        SHOW(*t)
        tp->AppendTuple(t);
      }
      qp->Request(args[indexOfCountArgument].addr, nAttributesWord);
      numberatt = ((CcInt*)nAttributesWord.addr)->GetIntval();

      ifequal = true;
      qp->Request(args[0].addr, sWord);
      while ((qp->Received(args[0].addr)) && ifequal)
      {
        s = (CcTuple*)sWord.addr;
        for (k = 0; k < numberatt; k++)
        {
          qp->Request(args[startIndexOfExtraArguments+k].addr, attribIdxWord);
          attribIdx = ((CcInt*)attribIdxWord.addr)->GetIntval();
          j = attribIdx - 1;
          if (((Attribute*)t->Get(j))->Compare((Attribute *)s->Get(j)))
            ifequal = false;
        }
        if (ifequal)
        {
          s = s->Clone();
          s->SetFree(false);
          SHOW(s)
          SHOW(*s)
          tp->AppendTuple(s);
          qp->Request(args[0].addr, sWord);
        }
        else
          local = SetWord((CcTuple*)sWord.addr);
      }
      if(ifequal)
      {
        local = SetWord(0);
      }

      t = new CcTuple;
      t->SetFree(true);
      SHOW(tp)
      relIter = tp->MakeNewScan();
      TRACE("GetNextTuple")
      s = relIter->GetNextTuple();
      
      SHOW(s)
      SHOW(*s)
      for(i = 0; i < numberatt; i++)
      {
        qp->Request(args[startIndexOfExtraArguments+i].addr, attribIdxWord);
        attribIdx = ((CcInt*)attribIdxWord.addr)->GetIntval();
        t->Put(i, ((Attribute*)s->Get(attribIdx - 1))->Clone());
      }
      value2 = (Supplier)args[2].addr;
      noOffun  =  qp->GetNoSons(value2);
      t->SetNoAttrs(numberatt + noOffun);
      delete relIter;

      SHOW(noOffun) 
      for(ind = 0; ind < noOffun; ind++)
      {
        supplier1 = qp->GetSupplier(value2, ind);
        supplier2 = qp->GetSupplier(supplier1, 1);
        vector = qp->Argument(supplier2);
        (*vector)[0] = SetWord(tp);
        qp->Request(supplier2, value);
        t->Put(numberatt + ind, ((Attribute*)value.addr)->Clone()) ;
      }
      SHOW(*t)
      result = SetWord(t);
      relWord = SetWord(tp);
      //DeleteCcRel(relWord);
      return YIELD;

    case CLOSE:
      qp->Close(args[0].addr);
      return 0;
  }
  return 0;
}

/*

4.1.3 Specification of operator ~groupby~

*/
const string CcGroupBySpec  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                            "\"Example\" ) "
                      "( <text>((stream (mtuple (a1:d1 ... an:dn))) "
		      "(ai1 ... aik) ((bj1 (fun (mrel (mtuple (a1:d1"
		      " ... an:dn))) (_))) ... (bjl (fun (mrel (mtuple"
		      " (a1:d1 ... an:dn))) (_))))) -> (stream (mtuple"
		      " (ai1:di1 ... aik:dik bj1 ... bjl)))</text--->"
		      "<text>_ groupby [list; funlist]</text--->"
		      "<text>Groups a relation according to attributes "
		      "ai1, ..., aik and feeds the groups to other "
		      "functions. The results of those functions are "
		      "appended to the grouping attributes.</text--->"
		      "<text>query Employee feed sortby[DeptNr asc] "
		      "groupby[DeptNr; anz : group feed count] consume"
		      "</text--->"
			      ") )";
/*

4.1.3 Definition of operator ~groupby~

*/
Operator ccrelgroupby (
         "groupby",             // name
         CcGroupBySpec,           // specification
         CcGroupByValueMapping,   // value mapping
         Operator::SimpleSelect,          // trivial selection function
         CcGroupByTypeMap         // type mapping
);

/*

5 Defnition of type constructor ~mtuple~

Eventually a type constructor is created by defining an instance of
class ~TypeConstructor~. Constructor's arguments are the type constructor's
name and the eleven functions previously defined.

*/
void DummyDelete(const ListExpr, Word& w) {};
void DummyClose(const ListExpr, Word& w) {};
Word DummyClone(const ListExpr, const Word& w) { return SetWord( Address(0) ); };
int DummySizeOf() { return 0; }

TypeConstructor ccreltuple( "mtuple",             CcTupleProp,
                          OutCcTuple,          InCcTuple,
                          SaveToListCcTuple,   RestoreFromListCcTuple,
                          CreateCcTuple,       DummyDelete,
                          0,                   0,
                          DummyClose,          DummyClone,
                          CastCcTuple,         DummySizeOf,
                          CheckCcTuple );
/*

5 Definition of type constructor ~mrel~

Eventually a type constructor is created by defining an instance of
class ~TypeConstructor~. Constructor's arguments are the type constructor's
name and the eleven functions previously defined.

*/
TypeConstructor ccrelrel( "mrel",            CcRelProp,
                       OutCcRel,          InCcRel,
                       0,                 0,
                       CreateCcRel, 	   DummyDelete,
		       OpenCcRel, 	   SaveCcRel,
                       DummyClose,        DummyClone,
                       CastCcRel,         DummySizeOf,
                       CheckCcRel );

/*

6 Class ~OldRelationAlgebra~

A new subclass ~OldRelationAlgebra~ of class ~Algebra~ is declared. The only
specialization with respect to class ~Algebra~ takes place within the
constructor: all type constructors and operators are registered at the actual algebra.

After declaring the new class, its only instance ~RelationAlgebra~ is defined.

*/

class OldRelationAlgebra : public Algebra
{
 public:
  OldRelationAlgebra() : Algebra()
  {
    AddTypeConstructor( &ccreltuple );
    AddTypeConstructor( &ccrelrel );

    AddOperator(&ccrelfeed);
    AddOperator(&ccrelsample);
    AddOperator(&ccrelconsume);
    AddOperator(&ccrelTUPLE);
    AddOperator(&ccrelgroup);
    AddOperator(&ccrelTUPLE2);
    AddOperator(&ccrelattr);
    AddOperator(&ccrelfilter);
    AddOperator(&ccrelproject);
    AddOperator(&ccrelremove);
    AddOperator(&ccrelproduct);
    AddOperator(&ccrelcancel);
    AddOperator(&ccrelcount);
    AddOperator(&ccrelrename);
    AddOperator(&ccrelextract);
    AddOperator(&ccrelextend);
    AddOperator(&ccrelconcat);
    AddOperator(&ccrelmax);
    AddOperator(&ccrelmin);
    AddOperator(&ccrelavg);
    AddOperator(&ccrelsum);
    AddOperator(&ccrelhead);
    AddOperator(&ccrelsortby);
    AddOperator(&ccrelsort);
    AddOperator(&ccrelrdup);
    AddOperator(&ccrelmergesec);
    AddOperator(&ccrelmergediff);
    AddOperator(&ccrelmergeunion);
    AddOperator(&ccrelmergejoin);
    AddOperator(&ccrelloopjoin);
    AddOperator(&ccrelloopselect);
    AddOperator(&ccrelloopjoinrel);
    AddOperator(&ccrelsortmergejoin);
    AddOperator(&ccrelhashjoin);
    AddOperator(&ccrelgroupby);

    ccreltuple.AssociateKind( "MTUPLE" );
    ccrelrel.AssociateKind( "MREL" );

  }
  ~OldRelationAlgebra() {};
};

OldRelationAlgebra oldrelationalgebra;
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
InitializeOldRelationAlgebra( NestedList* nlRef, QueryProcessor* qpRef )
{
  nl = nlRef;
  qp = qpRef;
  return (&oldrelationalgebra);
}

