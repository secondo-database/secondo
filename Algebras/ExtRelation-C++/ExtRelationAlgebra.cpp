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

[1] Implementation of Module Extended Relation Algebra

[1] Using Storage Manager Berkeley DB

June 1996 Claudia Freundorfer

May 2002 Frank Hoffmann port to C++

November 7, 2002 RHG Corrected the type mapping of ~tcount~.

November 30, 2002 RHG Introduced a function ~RelPersistValue~ instead of
~DefaultPersistValue~ which keeps relations that have been built in memory in a
small cache, so that they need not be rebuilt from then on.

April 2002 Victor Almeida Separated the relational algebra into two algebras
called Relational Algebra and Extended Relational Algebra.

[TOC]

1 Includes and defines

*/

#include <vector>
#include <deque>
#include <sstream>

#include "RelationAlgebra.h"
#include "QueryProcessor.h"
#include "CPUTimeMeasurer.h"
#include "StandardTypes.h"
#include "Counter.h"

extern NestedList* nl;
extern QueryProcessor* qp;

/*
2 Operators

2.2 Selection function for type operators

The selection function of a type operator always returns -1.

*/
int exttypeOperatorSelect(ListExpr args)
{
  return -1;
}

/*
2.3 Type Operator ~Group~

Type operators are used only for inferring argument types of parameter
functions. They have a type mapping but no evaluation function.

2.3.1 Type mapping function of operator ~group~

----  ((stream x))                -> (rel x)
----

*/
ListExpr GroupTypeMap(ListExpr args)
{
  ListExpr first;
  ListExpr tupleDesc;
  string argstr;

  nl->WriteToString(argstr,args);

  CHECK_COND(nl->ListLength(args) >= 1 && !nl->IsAtom(args),
    "Type operator sample expects a list with minimal length one.");

  first = nl->First(args);

  CHECK_COND(nl->ListLength(first) == 2 && !nl->IsAtom(first),
    "Type operator group: First list in the argument"
    " list must have length two.");

  tupleDesc = nl->Second(first);

  nl->WriteToString(argstr, first);
  CHECK_COND( TypeOfRelAlgSymbol(nl->First(first)) == stream &&
              (!nl->IsAtom(tupleDesc)) &&
              (nl->ListLength(tupleDesc) == 2) &&
              TypeOfRelAlgSymbol(nl->First(tupleDesc)) == tuple &&
              IsTupleDescription(nl->Second(tupleDesc)),
    "Type operator group expects an argument list with structure "
    "( (stream (tuple ((a1 t1)...(an tn))))(ai)...(ak) )\n"
    "Type operator group gets as first argument '" + argstr + "'.");

  return nl->TwoElemList(
          nl->SymbolAtom("rel"),
          tupleDesc);
}
/*
2.3.2 Specification of operator ~Group~

*/
const string GroupSpec  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                          "\"Remarks\" ) "
                          "( <text>((stream x)) -> (rel x)</text--->"
                          "<text>type operator</text--->"
                          "<text>Maps stream type to a rel.</text--->"
                          "<text>not for use with sos-syntax</text--->"
                          ") )";

/*
2.3.3 Definition of operator ~group~

*/
Operator extrelgroup (
         "GROUP",              // name
         GroupSpec,            // specification
         0,                    // no value mapping
         Operator::DummyModel, // dummy model mapping, defines in Algebra.h
         exttypeOperatorSelect,   // trivial selection function
         GroupTypeMap          // type mapping
);

/*
2.4 Operator ~sample~

Produces a stream representing a sample of a relation.

2.4.1 Function ~MakeRandomSubset~

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
MakeRandomSubset(vector<int>& result, int subsetSize, int setSize)
{
  assert(subsetSize >= 1);
  assert(setSize >= 2);
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

  // Using Windows RAND_MAX is very small (about 2^15) therefore we need to limit the drawSize
  // to 3/4 (to avoid long runtimes) of this size.
  int drawMax = 3*RAND_MAX/4;
  if ( drawSize > drawMax ) {
    drawSize = drawMax;
    cerr << "Warning: Sample size reduced to 3/4*RAND_MAX." << endl;
  }

  while(nDrawn < drawSize)
  {
    // 28.04.04 M. Spiekermann.
    // The calculation of random numbers blow is recommended in the man page
    // documentation of the rand() function.
    r = (int) ((double)(setSize + 1) * rand()/(RAND_MAX+1.0));
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
2.4.2 Type mapping function of operator ~sample~

A type mapping function takes a nested list as argument. Its contents are
type descriptions of an operator's input parameters. A nested list describing
the output type of the operator is returned.

Result type of feed operation.

----	((rel x) int real)		-> (stream x)
----

*/
ListExpr SampleTypeMap(ListExpr args)
{
  ListExpr first ;
  ListExpr minSampleSizeLE;
  ListExpr minSampleRateLE;
  string argstr1, argstr2, argstr3;

  CHECK_COND(nl->ListLength(args) == 3,
    "Operator sample expects a list of length three.");

  first = nl->First(args);
  nl->WriteToString(argstr1, first);
  minSampleSizeLE = nl->Second(args);
  nl->WriteToString(argstr2, minSampleSizeLE);
  minSampleRateLE = nl->Third(args);
  nl->WriteToString(argstr3, minSampleSizeLE);

  CHECK_COND(nl->ListLength(first) == 2,
    "Operator sample expects a relation as first argument. "
    "Operator sample gets '" + argstr1 + "' as first argument.");
  CHECK_COND(TypeOfRelAlgSymbol(nl->First(first)) == rel,
    "Operator sample expects a relation as first argument. "
    "Operator sample gets '" + argstr1 + "' as first argument.");

  CHECK_COND(nl->IsAtom(minSampleSizeLE),
    "Operator sample expects an int as second argument."
    "Operator sample gets '" + argstr2 + "' as second argument. ");
  CHECK_COND(nl->AtomType(minSampleSizeLE) == SymbolType,
    "Operator sample expects an int as second argument."
    "Operator sample gets '" + argstr2 + "' as second argument. ");
  CHECK_COND(nl->SymbolValue(minSampleSizeLE) == "int",
    "Operator sample expects an int as second argument."
    "Operator sample gets '" + argstr2 + "' as second argument. ");

  CHECK_COND(nl->IsAtom(minSampleRateLE),
    "Operator sample expects a real as third argument."
    "Operator sample gets '" + argstr3 + "' as third argument. ");
  CHECK_COND(nl->AtomType(minSampleRateLE) == SymbolType,
    "Operator sample expects a real as third argument."
    "Operator sample gets '" + argstr3 + "' as third argument. ");
  CHECK_COND(nl->SymbolValue(minSampleRateLE) == "real",
    "Operator sample expects a real as third argument."
    "Operator sample gets '" + argstr3 + "' as third argument. ");

  return nl->Cons(nl->SymbolAtom("stream"), nl->Rest(first));
}
/*
2.4.3 Value mapping function of operator ~sample~

*/
struct SampleLocalInfo
{
  vector<int> sampleIndices;
  vector<int>::iterator iter;
  int lastIndex;
  RelationIterator* relIter;
};

int Sample(Word* args, Word& result, int message, Word& local, Supplier s)
{
  SampleLocalInfo* localInfo;
  Word argRelation;
  Word sampleSizeWord;
  Word sampleRateWord;

  Relation* rel;
  Tuple* tuple;

  int sampleSize;
  int relSize;
  float sampleRate;
  int i;
  int currentIndex;

  switch(message)
  {
    case OPEN :
      localInfo = new SampleLocalInfo();
      local = SetWord(localInfo);

      qp->Request(args[0].addr, argRelation);
      qp->Request(args[1].addr, sampleSizeWord);
      qp->Request(args[2].addr, sampleRateWord);

      rel = (Relation*)argRelation.addr;
      relSize = rel->GetNoTuples();
      localInfo->relIter = rel->MakeScan();
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
        MakeRandomSubset(localInfo->sampleIndices, sampleSize, relSize);
      }

      localInfo->iter = localInfo->sampleIndices.begin();
      localInfo->lastIndex = 0;
      return 0;

    case REQUEST:
      localInfo = (SampleLocalInfo*)local.addr;
      if(localInfo->iter == localInfo->sampleIndices.end())
      {
        return CANCEL;
      }
      else
      {
        currentIndex = *(localInfo->iter);
        if(!(tuple = localInfo->relIter->GetNextTuple()))
        {
          return CANCEL;
        }

        /* Advance iterator to the the next tuple belonging to the sample */
        for(i = 1; i < currentIndex - localInfo->lastIndex; ++i)
        {
          tuple->DeleteIfAllowed();
          if(!(tuple = localInfo->relIter->GetNextTuple()))
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
      localInfo = (SampleLocalInfo*)local.addr;
      delete localInfo->relIter;
      delete localInfo;
      return 0;
  }
  return 0;
}
/*
2.4.4 Specification of operator ~sample~

*/
const string SampleSpec  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                           "\"Example\" ) "
                           "( <text>(rel x) int real -> (stream x)"
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
2.4.5 Definition of operator ~sample~

Non-overloaded operators are defined by constructing a new instance of
class ~Operator~, passing all operator functions as constructor arguments.

*/
Operator extrelsample (
          "sample",                // name
          SampleSpec,              // specification
          Sample,                  // value mapping
          Operator::DummyModel,    // dummy model mapping, defines in Algebra.h
          Operator::SimpleSelect,  // trivial selection function
          SampleTypeMap            // type mapping
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

The type mapping computes the number of attributes and the list of attribute
numbers for the given left attributes (after removal) and asks the query processor to
append it to the given arguments.

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

  CHECK_COND(nl->ListLength(args) == 2,
    "Operator remove expects a list of length two.");

  first = nl->First(args);
  second = nl->Second(args);

  nl->WriteToString(argstr, first);
  CHECK_COND(nl->ListLength(first) == 2  &&
             (TypeOfRelAlgSymbol(nl->First(first)) == stream) &&
             (nl->ListLength(nl->Second(first)) == 2) &&
             (TypeOfRelAlgSymbol(nl->First(nl->Second(first))) == tuple),
    "Operator remove expects as first argument a list with structure "
    "(stream (tuple ((a1 t1)...(an tn))))\n"
    "Operator remove gets as first argument '" + argstr + "'.");

  nl->WriteToString(argstr, second);
  CHECK_COND((!nl->IsAtom(second)) &&
             (nl->ListLength(second) > 0),
    "Operator remove expects as second argument a list with attribute names "
    "(ai ... ak), not a single atom and not an empty list.\n"
    "Operator remove gets '" + argstr + "'.");

  while (!(nl->IsEmpty(second)))
  {
    first2 = nl->First(second);
    second = nl->Rest(second);
    nl->WriteToString(argstr, first2);
    cout << argstr << endl;

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
      return nl->SymbolAtom("typeerror");
    }

    j = FindAttribute(nl->Second(nl->Second(first)), attrname, attrtype);
    if (j)  removeSet.insert(j);
    else
    {
      nl->WriteToString( argstr, nl->Second(nl->Second(first)) );
      ErrorReporter::ReportError("Attributename '" + attrname + "' is not known.\n"
      "Known Attribute(s): " + argstr);
      return nl->SymbolAtom("typeerror");
    }
  }
  //*****here: we need to generate new attr list according to removeSet*****
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

  if (noAttrs>0)
  {
    outlist = nl->ThreeElemList(
              nl->SymbolAtom("APPEND"),
              nl->TwoElemList(nl->IntAtom(noAttrs), numberList),
              nl->TwoElemList(nl->SymbolAtom("stream"),
              nl->TwoElemList(nl->SymbolAtom("tuple"),
                       newAttrList)));
    return outlist;
  }
  else
  {
    ErrorReporter::ReportError("Do not remove all attributes!");
    return nl->SymbolAtom("typeerror");
  }
}

/*
2.5.2 Value mapping function of operator ~remove~

*/
int Remove(Word* args, Word& result, int message, Word& local, Supplier s)
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
        for (int i=1; i <= noOfAttrs; i++)
        {
          son = qp->GetSupplier(args[3].addr, i-1);
          qp->Request(son, elem2);
          index = ((CcInt*)elem2.addr)->GetIntval();
          attr = ((Tuple*)elem1.addr)->GetAttribute(index-1);
          t->PutAttribute(i-1, ((StandardAttribute*)attr->Clone()));
        }
        ((Tuple*)elem1.addr)->DeleteIfAllowed();
        result = SetWord(t);
        return YIELD;
      }
      else return CANCEL;
    }
    case CLOSE :
    {
      qp->Close(args[0].addr);
      return 0;
    }
  }
  return 0;
}

/*
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
Operator extrelremoval (
         "remove",                // name
         RemoveSpec,              // specification
         Remove,                  // value mapping
         Operator::DummyModel,    // dummy model mapping, defines in Algebra.h
         Operator::SimpleSelect,  // trivial selection function
         RemoveTypeMap            // type mapping
);

/*
2.6 Operator ~cancel~

Transmits tuple from its input stream to its output stream until a tuple
arrives fulfilling some condition.

2.6.1 Type mapping function of operator ~cancel~

Type mapping for ~cancel~ is the same, as type mapping for operator ~filter~.
Result type of cancel operation.

----    ((stream x) (map x bool)) -> (stream x)
----

*/
ListExpr CancelTypeMap(ListExpr args)
{
  ListExpr first, second;
  string argstr;

  CHECK_COND(nl->ListLength(args) == 2,
  "Operator cancel expects a list of length two.");

  first = nl->First(args);
  second  = nl->Second(args);

  nl->WriteToString(argstr, first);
  CHECK_COND(nl->ListLength(first) == 2 &&
             TypeOfRelAlgSymbol(nl->First(first)) == stream &&
	     nl->ListLength(nl->Second(first)) == 2 &&
             TypeOfRelAlgSymbol(nl->First(nl->Second(first))) == tuple,
    "Operator cancel expects as first argument a list with structure "
    "(stream (tuple ((a1 t1)...(an tn))))\n"
    "Operator cancel gets a list with structure '" + argstr + "'.");

  nl->WriteToString(argstr, second);
  CHECK_COND(nl->ListLength(second) == 3 &&
             TypeOfRelAlgSymbol(nl->First(second)) == ccmap &&
             TypeOfRelAlgSymbol(nl->Third(second)) == ccbool,
    "Operator cancel expects as second argument a list with structure "
    "(map (tuple ((a1 t1)...(an tn))) bool)\n"
    "Operator cancel gets a list with structure '" + argstr + "'.");

  CHECK_COND(nl->Equal(nl->Second(first),nl->Second(second)),
    "Tuple type in stream is not equal to tuple type in the function.");

  return first;
}

/*
2.6.2 Value mapping function of operator ~cancel~

*/
int Cancel(Word* args, Word& result, int message, Word& local, Supplier s)
{
  Word t, value;
  Tuple* tuple;
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
        tuple = (Tuple*)t.addr;
        vector = qp->Argument(args[1].addr);
        (*vector)[0] = t;
        qp->Request(args[1].addr, value);
        found =
          ((CcBool*)value.addr)->IsDefined()
          && ((CcBool*)value.addr)->GetBoolval();
        if (found)
        {
          tuple->DeleteIfAllowed();
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
2.6.3 Specification of operator ~cancel~

*/
const string CancelSpec  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
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
2.6.4 Definition of operator ~cancel~

*/
Operator extrelcancel (
         "cancel",               // name
         CancelSpec,             // specification
         Cancel,                 // value mapping
         Operator::DummyModel,   // dummy model mapping, defines in Algebra.h
         Operator::SimpleSelect, // trivial selection function
         CancelTypeMap           // type mapping
);

/*
2.7 Operator ~extract~

This operator has a stream of tuples and the name of an attribut as input and
returns the value of this attribute
from the first tuple of the input stream. If the input stream is empty a run
time error occurs. In this case value -1 will be returned.

2.7.1 Type mapping function of operator ~extract~

Type mapping for ~extract~ is

----  ((stream (tuple ((x1 t1)...(xn tn))) xi)  -> ti
              APPEND (i) ti)
----

*/
ListExpr ExtractTypeMap( ListExpr args )
{
  ListExpr first, second, attrtype;
  string  attrname, argstr;
  int j;

  CHECK_COND(nl->ListLength(args) == 2,
    "Operator extract expects a list of length two.");

  first = nl->First(args);
  second = nl->Second(args);

  nl->WriteToString(argstr, first);
  CHECK_COND(nl->ListLength(first) == 2  &&
             (TypeOfRelAlgSymbol(nl->First(first)) == stream) &&
             (nl->ListLength(nl->Second(first)) == 2) &&
             (TypeOfRelAlgSymbol(nl->First(nl->Second(first))) == tuple),
    "Operator extract expects as first argument a list with structure "
    "(stream (tuple ((a1 t1)...(an tn))))\n"
    "Operator remove gets as first argument '" + argstr + "'.");

  nl->WriteToString(argstr, second);
  CHECK_COND((nl->IsAtom(second)) &&
             (nl->AtomType(second) == SymbolType),
    "Operator extract expects as second argument an atom (attributename).\n"
    "Operator extract gets '" + argstr + "'.\n"
    "Atrributename may not be the name of a Secondo object!");

  attrname = nl->SymbolValue(second);
  j = FindAttribute(nl->Second(nl->Second(first)), attrname, attrtype);
  if (j)
  {
    return nl->ThreeElemList(nl->SymbolAtom("APPEND"),
           nl->OneElemList(nl->IntAtom(j)), attrtype);
  }
  else
  {
    nl->WriteToString( argstr, nl->Second(nl->Second(first)) );
    ErrorReporter::ReportError("Attributename '" + attrname + "' is not known.\n"
      "Known Attribute(s): " + argstr);
    return nl->SymbolAtom("typeerror");
  }
}
/*
2.7.2 Value mapping function of operator ~extract~

The argument vector ~args~ contains in the first slot ~args[0]~ the tuple
and in ~args[2]~ the position of the attribute as a number. Returns as
~result~ the value of an attribute at the given position ~args[2]~ in a
tuple object.

*/
int Extract(Word* args, Word& result, int message, Word& local, Supplier s)
{
  Word t;
  Tuple* tupleptr;
  int index;
  StandardAttribute* res = (StandardAttribute*)((qp->ResultStorage(s)).addr);
  result = SetWord(res);

  qp->Open(args[0].addr);
  qp->Request(args[0].addr,t);

  if (qp->Received(args[0].addr))
  {
    tupleptr = (Tuple*)t.addr;
    index = ((CcInt*)args[2].addr)->GetIntval();
    assert((1 <= index) && (index <= tupleptr->GetNoAttributes()));
    res->CopyFrom((StandardAttribute*)tupleptr->GetAttribute(index - 1));
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
2.7.3 Specification of operator ~extract~

*/
const string ExtractSpec  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                            "\"Example\" ) "
                            "( <text>((stream (tuple([a1:d1, ... ,an:dn]"
                            "))) x ai) -> di</text--->"
                            "<text>_ extract [ _ ]</text--->"
                            "<text>Returns the value of attribute ai of "
                            "the first tuple in the input stream."
                            "</text--->"
                            "<text>query cities feed extract [population]"
                            "</text--->"
                              ") )";

/*
2.7.4 Definition of operator ~extract~

*/
Operator extrelextract (
         "extract",              // name
         ExtractSpec,            // specification
         Extract,                // value mapping
         Operator::DummyModel,   // dummy model mapping, defines in Algebra.h
         Operator::SimpleSelect, // trivial selection function
         ExtractTypeMap          // type mapping
);

/*
2.8 Operator ~head~

This operator fetches the first n tuples from a stream.

2.8.1 Type mapping function of operator ~head~

Type mapping for ~head~ is

----  ((stream (tuple ((x1 t1)...(xn tn))) int)   ->
              ((stream (tuple ((x1 t1)...(xn tn))))
----

*/
ListExpr HeadTypeMap( ListExpr args )
{
  ListExpr first, second;
  string argstr;

  CHECK_COND(nl->ListLength(args) == 2,
  "Operator head expects a list of length two.");

  first = nl->First(args);
  second = nl->Second(args);

  nl->WriteToString(argstr, first);
  CHECK_COND( ( nl->ListLength(first) == 2 ) &&
              ( TypeOfRelAlgSymbol( nl->First(first) ) == stream ) &&
              ( nl->ListLength( nl->Second(first) ) == 2) &&
	      (TypeOfRelAlgSymbol(nl->First(nl->Second(first))) == tuple),
    "Operator head expects as first argument a list with structure "
    "(stream (tuple ((a1 t1)...(an tn))))\n"
    "Operator head gets as first argument '" + argstr + "'." );

  nl->WriteToString(argstr, second);
  CHECK_COND((nl->IsAtom(second)) &&
             (nl->AtomType(second) == SymbolType) &&
	     (nl->SymbolValue(second) == "int"),
    "Operator head expects a second argument of type integer.\n"
    "Operator head gets '" + argstr + "'.");

  return first;
}
/*
2.8.3 Value mapping function of operator ~head~

*/
struct HeadLocalInfo
{
  HeadLocalInfo( const int maxTuples = 0 ):
    numTuples( 0 ),
    maxTuples( maxTuples )
    {}

  int numTuples;
  int maxTuples;
};

int Head(Word* args, Word& result, int message, Word& local, Supplier s)
{
  HeadLocalInfo *localInfo;
  Word maxTuplesWord;
  Word tupleWord;

  switch(message)
  {
    case OPEN:

      qp->Open(args[0].addr);
      qp->Request(args[1].addr, maxTuplesWord);
      localInfo = new HeadLocalInfo( ((CcInt*)maxTuplesWord.addr)->GetIntval() );
      local = SetWord( localInfo );
      return 0;

    case REQUEST:

      localInfo = (HeadLocalInfo*)local.addr;
      if(localInfo->numTuples >= localInfo->maxTuples)
      {
        return CANCEL;
      }

      qp->Request(args[0].addr, tupleWord);
      if(qp->Received(args[0].addr))
      {
        result = tupleWord;
        localInfo->numTuples++;
        return YIELD;
      }
      else
      {
        return CANCEL;
      }
    case CLOSE:

      localInfo = (HeadLocalInfo*)local.addr;
      delete localInfo;
      qp->Close(args[0].addr);
      return 0;
  }
  return 0;
}
/*
2.8.4 Specification of operator ~head~

*/
const string HeadSpec  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                         "\"Example\" ) "
                         "( <text>((stream (tuple([a1:d1, ... ,an:dn]"
                         "))) x int) -> (stream (tuple([a1:d1, ... ,"
                         "an:dn])))</text--->"
                         "<text>_ head [ _ ]</text--->"
                         "<text>Returns the first n tuples in the input "
                         "stream.</text--->"
                         "<text>query cities feed head[10] consume"
                         "</text--->"
                              ") )";

/*
2.8.5 Definition of operator ~head~

*/
Operator extrelhead (
         "head",                 // name
         HeadSpec,               // specification
         Head,                   // value mapping
         Operator::DummyModel,   // dummy model mapping, defines in Algebra.h
         Operator::SimpleSelect, // trivial selection function
         HeadTypeMap             // type mapping
);

/*
2.9 Operators ~max~ and ~min~

2.9.1 Type mapping function of Operators ~max~ and ~min~

Type mapping for ~max~ and ~min~ is

----  ((stream (tuple ((x1 t1)...(xn tn))) xi)  -> ti
              APPEND (i ti)
----

*/
template<bool isMax> ListExpr
MaxMinTypeMap( ListExpr args )
{
  ListExpr first, second, attrtype;
  string  attrname, argstr, argstrtmp;
  int j;

  const char* errorMessage1 =
  isMax ?
    "Operator max expects a list of length two."
  : "Operator min expects a list of length two.";
  CHECK_COND(nl->ListLength(args) == 2,
    errorMessage1);

  first = nl->First(args);
  second = nl->Second(args);

  nl->WriteToString(argstr, first);
  string errorMessage2 =
  isMax ?
    "Operator max expects as first argument a list with structure "
    "(stream (tuple ((a1 t1)...(an tn))))\n"
    "Operator max gets as first argument '" + argstr + "'."
  : "Operator min expects as first argument a list with structure "
    "(stream (tuple ((a1 t1)...(an tn))))\n"
    "Operator min gets as first argument '" + argstr + "'.";
  CHECK_COND(nl->ListLength(first) == 2  &&
             (TypeOfRelAlgSymbol(nl->First(first)) == stream) &&
             (nl->ListLength(nl->Second(first)) == 2) &&
             (TypeOfRelAlgSymbol(nl->First(nl->Second(first))) == tuple) &&
	     (nl->ListLength(nl->Second(first)) == 2) &&
	     (IsTupleDescription(nl->Second(nl->Second(first)))),
	     errorMessage2);

  nl->WriteToString(argstr, second);
  string errorMessage3 =
  isMax ?
    "Operator max expects as second argument an atom (attributename).\n"
    "Operator max gets '" + argstr + "'.\n"
    "Atrributename may not be the name of a Secondo object!"
  : "Operator min expects as second argument an atom (attributename).\n"
    "Operator min gets '" + argstr + "'.\n"
    "Atrributename may not be the name of a Secondo object!";
  CHECK_COND((nl->IsAtom(second)) &&
             (nl->AtomType(second) == SymbolType),
	     errorMessage3);

  attrname = nl->SymbolValue(second);
  nl->WriteToString(argstr, nl->Second(nl->Second(first)));
  j = FindAttribute(nl->Second(nl->Second(first)), attrname, attrtype);
  string errorMessage4 =
    "Attributename '" + attrname + "' is not known.\n"
    "Known Attribute(s): " + argstr;
  string errorMessage5 =
    "Attribute type is not of type real, int, string or bool.";
  if ( j )
  {
    CHECK_COND( (nl->SymbolValue(attrtype) == "real"
          || nl->SymbolValue(attrtype) == "string"
          || nl->SymbolValue(attrtype) == "bool"
          || nl->SymbolValue(attrtype) == "int"),
	  errorMessage5);
    return nl->ThreeElemList(nl->SymbolAtom("APPEND"),
           nl->OneElemList(nl->IntAtom(j)), attrtype);
  }
  else
  {
    nl->WriteToString( argstr, nl->Second(nl->Second(first)) );
    ErrorReporter::ReportError(errorMessage4);
    return nl->SymbolAtom("typeerror");
  }
}
/*
2.9.2 Value mapping function of operators ~max~ and ~min~

*/

template<bool isMax> int
MaxMinValueMapping(Word* args, Word& result, int message, Word& local, Supplier s)
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
    Tuple* currentTuple = (Tuple*)currentTupleWord.addr;
    StandardAttribute* currentAttr =
      (StandardAttribute*)currentTuple->GetAttribute(attributeIndex);
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
2.9.3 Specification of operator ~max~

*/
const string MaxOpSpec  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                          "\"Example\" ) "
                          "( <text>((stream (tuple([a1:d1, ... ,an:dn]"
                          "))) x ai) -> di</text--->"
                          "<text>_ _ mergesec</text--->"
                          "<text>Returns the maximum value of attribute "
                          "ai over the input stream.</text--->"
                          "<text>query cities feed max [ cityname ]"
                          "</text--->"
                              ") )";

/*
2.9.4 Definition of operator ~max~

*/
Operator extrelmax (
         "max",             // name
         MaxOpSpec,           // specification
         MaxMinValueMapping<true>,               // value mapping
         Operator::DummyModel,  // dummy model mapping, defines in Algebra.h
         Operator::SimpleSelect,          // trivial selection function
         MaxMinTypeMap<true>         // type mapping
);

/*
2.9.5 Specification of operator ~min~

*/
const string MinOpSpec  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                          "\"Example\" ) "
                          "( <text>((stream (tuple([a1:d1, ... ,an:dn])))"
                          " x ai) -> di</text--->"
                          "<text>_ min [ _ ]</text--->"
                          "<text>Returns the minimum value of attribute ai "
                          "over the input stream.</text--->"
                          "<text>query cities feed min [ cityname ]"
                          "</text--->"
                              ") )";

/*
2.9.6 Definition of operator ~min~

*/
Operator extrelmin (
         "min",             // name
         MinOpSpec,           // specification
         MaxMinValueMapping<false>,               // value mapping
         Operator::DummyModel,  // dummy model mapping, defines in Algebra.h
         Operator::SimpleSelect,          // trivial selection function
         MaxMinTypeMap<false>         // type mapping
);

/*
2.10 Operators ~avg~ and ~sum~

2.10.1 Type mapping function of Operators ~avg~ and ~sum~

Type mapping for ~avg~ is

----  ((stream (tuple ((x1 t1)...(xn tn))) xi)  -> real
              APPEND (i ti)
----

Type mapping for ~sum~ is

----  ((stream (tuple ((x1 t1)...(xn tn))) xi)  -> ti
              APPEND (i ti)
----

*/

template<bool isAvg> ListExpr
AvgSumTypeMap( ListExpr args )
{
  ListExpr first, second, attrtype;
  string  attrname, argstr;
  int j;

  const char* errorMessage1 =
  isAvg ?
    "Operator avg expects a list of length two."
  : "Operator sum expects a list of length two.";
  CHECK_COND(nl->ListLength(args) == 2,
    errorMessage1);

  first = nl->First(args);
  second = nl->Second(args);

  nl->WriteToString(argstr, first);
  string errorMessage2 =
  isAvg ?
    "Operator avg expects as first argument a list with structure "
    "(stream (tuple ((a1 t1)...(an tn))))\n"
    "Operator avg gets as first argument '" + argstr + "'."
  : "Operator sum expects as first argument a list with structure "
    "(stream (tuple ((a1 t1)...(an tn))))\n"
    "Operator sum gets as first argument '" + argstr + "'.";
  CHECK_COND(nl->ListLength(first) == 2  &&
             (TypeOfRelAlgSymbol(nl->First(first)) == stream) &&
             (nl->ListLength(nl->Second(first)) == 2) &&
             (TypeOfRelAlgSymbol(nl->First(nl->Second(first))) == tuple) &&
	     (nl->ListLength(nl->Second(first)) == 2) &&
	     (IsTupleDescription(nl->Second(nl->Second(first)))),
	     errorMessage2);

  nl->WriteToString(argstr, second);
  string errorMessage3 =
  isAvg ?
    "Operator max expects as second argument an atom (attributename).\n"
    "Operator max gets '" + argstr + "'.\n"
    "Atrributename may not be the name of a Secondo object!"
  : "Operator min expects as second argument an atom (attributename).\n"
    "Operator min gets '" + argstr + "'.\n"
    "Atrributename may not be the name of a Secondo object!";
  CHECK_COND((nl->IsAtom(second)) &&
             (nl->AtomType(second) == SymbolType),
	     errorMessage3);

  attrname = nl->SymbolValue(second);
  nl->WriteToString(argstr, nl->Second(nl->Second(first)));
  j = FindAttribute(nl->Second(nl->Second(first)), attrname, attrtype);
  string errorMessage4 =
    "Attributename '" + attrname + "' is not known.\n"
    "Known Attribute(s): " + argstr;
  string errorMessage5 =
    "Attribute type is not of type real or int.";
  if ( j )
  {
    CHECK_COND( (nl->SymbolValue(attrtype) == "real"
          || nl->SymbolValue(attrtype) == "int"),
	  errorMessage5);
    return nl->ThreeElemList(nl->SymbolAtom("APPEND"),
             nl->TwoElemList(nl->IntAtom(j),
             nl->StringAtom(nl->SymbolValue(attrtype))),
             isAvg ? nl->SymbolAtom("real") : attrtype);
  }
  else
  {
    nl->WriteToString( argstr, nl->Second(nl->Second(first)) );
    ErrorReporter::ReportError(errorMessage4);
    return nl->SymbolAtom("typeerror");
  }
}
/*

2.10.2 Value mapping function of operators ~avg~ and ~sum~

*/
template<bool isAvg> int
AvgSumValueMapping(Word* args, Word& result, int message, Word& local, Supplier s)
{
  bool definedValueFound = false;
  Word currentTupleWord;
  Attribute* accumulated = 0;
  int nProcessedItems = 0;

  assert(args[2].addr != 0);
  assert(args[3].addr != 0);

  int attributeIndex = ((CcInt*)args[2].addr)->GetIntval() - 1;
  STRING *attributeType = ((CcString*)args[3].addr)->GetStringval();

  qp->Open(args[0].addr);
  qp->Request(args[0].addr, currentTupleWord);
  while(qp->Received(args[0].addr))
  {
    Tuple* currentTuple = (Tuple*)currentTupleWord.addr;
    Attribute* currentAttr = (Attribute*)currentTuple->GetAttribute(attributeIndex);
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
2.10.3 Specification of operator ~avg~

*/
const string AvgOpSpec  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                          "\"Example\" ) "
                          "( <text>((stream (tuple([a1:d1, ... ,an:dn]"
                          "))) x ai) -> real</text--->"
                          "<text>_ avg [ _ ]</text--->"
                          "<text>Returns the average value of attribute "
                          "ai over the input stream.</text--->"
                          "<text>query cities feed avg [population]"
                          "</text--->"
                              ") )";

/*
2.10.4 Definition of operator ~avg~

*/
Operator extrelavg (
         "avg",             // name
         AvgOpSpec,           // specification
         AvgSumValueMapping<true>,               // value mapping
         Operator::DummyModel,  // dummy model mapping, defines in Algebra.h
         Operator::SimpleSelect,          // trivial selection function
         AvgSumTypeMap<true>         // type mapping
);

/*
2.10.5 Specification of operator ~sum~

*/
const string SumOpSpec  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                          "\"Example\" ) "
                          "( <text>((stream (tuple([a1:d1, ... ,an:dn]"
                          "))) x ai) -> di</text--->"
                          "<text>_ sum [ _ ]</text--->"
                          "<text>Returns the sum of the values of attribute"
                          " ai over the input stream.</text--->"
                          "<text>query cities feed sum[population]"
                          "</text--->"
                              ") )";

/*
2.10.6 Definition of operator ~sum~

*/
Operator extrelsum (
         "sum",             // name
         SumOpSpec,           // specification
         AvgSumValueMapping<false>,               // value mapping
         Operator::DummyModel,  // dummy model mapping, defines in Algebra.h
         Operator::SimpleSelect,          // trivial selection function
         AvgSumTypeMap<false>         // type mapping
);

/*
2.11 Operator ~sortBy~

This operator sorts a stream of tuples by a given list of attributes.
For each attribute it must be specified wether the list should be sorted
in ascending (asc) or descending (desc) order with regard to that attribute.

2.11.1 Type mapping function of operator ~sortBy~

Type mapping for ~sortBy~ is

----  ((stream (tuple ((x1 t1)...(xn tn))) ((xi1 asc/desc) ... (xij asc/desc)))
              -> (stream (tuple ((x1 t1)...(xn tn)))
                  APPEND (j i1 asc/desc i2 asc/desc ... ij asc/desc)
----

*/

static char* sortAscending = "asc";
static char* sortDescending = "desc";

ListExpr SortByTypeMap( ListExpr args )
{
  ListExpr attrtype;
  string  attrname, argstr;

  CHECK_COND(nl->ListLength(args) == 2,
    "Operator sortby expects a list of length two.");

  ListExpr streamDescription = nl->First(args);
  ListExpr sortSpecification  = nl->Second(args);

  nl->WriteToString(argstr, streamDescription);

  CHECK_COND(nl->ListLength(streamDescription) == 2  &&
             (TypeOfRelAlgSymbol(nl->First(streamDescription)) == stream) &&
             (nl->ListLength(nl->Second(streamDescription)) == 2) &&
             (TypeOfRelAlgSymbol(nl->First(nl->Second(streamDescription))) == tuple),
    "Operator sortby expects as first argument a list with structure "
    "(stream (tuple ((a1 t1)...(an tn))))\n"
    "Operator sortby gets as first argument '" + argstr + "'.");

  int numberOfSortAttrs = nl->ListLength(sortSpecification);

  CHECK_COND(numberOfSortAttrs > 0,
    "Operator sortby: Attribute list may not be enpty!");

  ListExpr sortOrderDescription = nl->OneElemList(nl->IntAtom(numberOfSortAttrs));
  ListExpr sortOrderDescriptionLastElement = sortOrderDescription;
  ListExpr rest = sortSpecification;
  while(!nl->IsEmpty(rest))
  {
    ListExpr attributeSpecification = nl->First(rest);
    rest = nl->Rest(rest);

    nl->WriteToString(argstr, attributeSpecification);
    CHECK_COND(nl->ListLength(attributeSpecification) == 2  &&
               (nl->IsAtom(nl->First(attributeSpecification))) &&
               (nl->AtomType(nl->First(attributeSpecification)) == SymbolType) &&
	       (nl->IsAtom(nl->Second(attributeSpecification))) &&
               (nl->AtomType(nl->Second(attributeSpecification)) == SymbolType),
      "Operator sortby expects as second argument a list "
      "((ai asc/desc)+)\n"
      "Operator sortby gets a list '" + argstr + "'.");

    attrname = nl->SymbolValue(nl->First(attributeSpecification));
    int j = FindAttribute(nl->Second(nl->Second(streamDescription)), attrname, attrtype);

    if (j > 0)
    {
      nl->WriteToString(argstr, nl->Second(attributeSpecification));
      CHECK_COND( ((nl->SymbolValue(nl->Second(attributeSpecification)) == sortAscending)
                  || (nl->SymbolValue(nl->Second(attributeSpecification)) == sortDescending)),
        "Operator sortby: sorting criteria must be asc or desc, not '" + argstr + "'!" );

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
      nl->WriteToString( argstr, nl->Second(nl->Second(streamDescription)) );
      ErrorReporter::ReportError("Operator sortby: Attributename '" + attrname +
        "' is not known.\nKnown Attribute(s): " + argstr);
      return nl->SymbolAtom("typeerror");
    }
  }
  return nl->ThreeElemList(nl->SymbolAtom("APPEND"),
    sortOrderDescription, streamDescription);
}
/*
2.11.2 Value mapping function of operator ~sortBy~

The argument vector ~args~ contains in the first slot ~args[0]~ the stream and
in ~args[2]~ the number of sort attributes. ~args[3]~ contains the index of the first
sort attribute, ~args[4]~ a boolean indicating wether the stream is sorted in
ascending order with regard to the sort first attribute. ~args[5]~ and ~args[6]~
contain these values for the second sort attribute  and so on.

*/
template<bool lexicographically, bool requestArgs> int
SortBy(Word* args, Word& result, int message, Word& local, Supplier s);
/*
This function will be implemented differently for the persistent and for
the main memory relational algebra. Its implementation can be found in
the files ExtRelAlgPersistent.cpp and ExtRelAlgMainMemory.cpp,
respectively.

2.11.3 Specification of operator ~sortBy~

*/
const string SortBySpec  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                           "\"Example\" ) "
                           "( <text>((stream (tuple([a1:d1, ... ,an:dn])))"
                           " ((xi1 asc/desc) ... (xij asc/desc))) -> "
                           "(stream (tuple([a1:d1, ... ,an:dn])))</text--->"
                           "<text>_ sortby [list]</text--->"
                           "<text>Sorts input stream according to a list "
                           "of attributes ai1 ... aij.</text--->"
                           "<text>query employee feed sortby[DeptNo asc] "
                           "consume</text--->"
                              ") )";

/*
2.11.4 Definition of operator ~sortBy~

*/
Operator extrelsortby (
         "sortby",              // name
         SortBySpec,            // specification
         SortBy<false, true>,   // value mapping
         Operator::DummyModel,  // dummy model mapping, defines in Algebra.h
         Operator::SimpleSelect,          // trivial selection function
         SortByTypeMap          // type mapping
);

/*
2.12 Operator ~sort~

This operator sorts a stream of tuples lexicographically.

2.12.1 Type mapping function of operator ~sort~

Type mapping for ~sort~ is

----  ((stream (tuple ((x1 t1)...(xn tn))))   -> (stream (tuple ((x1 t1)...(xn tn)))

----

*/
template<bool isSort> ListExpr
IdenticalTypeMap( ListExpr args )
{
  ListExpr first;
  string argstr;

  const char* errorMessage1 =
  isSort ?
    "Operator sort expects a list of length one."
  : "Operator rdup expects a list of length one.";

  CHECK_COND(nl->ListLength(args) == 1,
    errorMessage1);

  first = nl->First(args);

  nl->WriteToString(argstr, first);
  string errorMessage2 =
  isSort ?
    "Operator sort expects as argument a list with structure "
    "(stream (tuple ((a1 t1)...(an tn))))\n"
    "Operator sort gets as argument '" + argstr + "'."
  : "Operator rdup expects as argument a list with structure "
    "(stream (tuple ((a1 t1)...(an tn))))\n"
    "Operator rdup gets as argument '" + argstr + "'.";
  CHECK_COND(nl->ListLength(first) == 2  &&
             (TypeOfRelAlgSymbol(nl->First(first)) == stream) &&
             (nl->ListLength(nl->Second(first)) == 2) &&
             (TypeOfRelAlgSymbol(nl->First(nl->Second(first))) == tuple) &&
	     (nl->ListLength(nl->Second(first)) == 2) &&
	     (IsTupleDescription(nl->Second(nl->Second(first)))),
	     errorMessage2);

  return first;
}
/*
2.12.2 Specification of operator ~sort~

*/
const string SortSpec  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                            "\"Example\" ) "
                            "( <text>((stream (tuple([a1:d1, ... ,an:dn]"
                            ")))) -> (stream (tuple([a1:d1, ... ,an:dn])))"
                            "</text--->"
                            "<text>_ sort</text--->"
                            "<text>Sorts input stream lexicographically."
                            "</text--->"
	                    "<text>query cities feed sort consume</text--->"
                            ") )";

/*
2.12.3 Definition of operator ~sort~

*/
Operator extrelsort (
         "sort",             // name
         SortSpec,           // specification
         SortBy<true, true>,               // value mapping
         Operator::DummyModel,  // dummy model mapping, defines in Algebra.h
         Operator::SimpleSelect,          // trivial selection function
         IdenticalTypeMap<true>         // type mapping
);

/*
2.13 Operator ~rdup~

This operator removes duplicates from a sorted stream.

2.13.1 Value mapping function of operator ~rdup~

*/
int RdupValueMapping(Word* args, Word& result, int message, Word& local, Supplier s)
{
  Word tuple;
  LexicographicalTupleCompare cmp;
  Tuple* currentTuple;
  Tuple* lastOutputTuple;

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
            currentTuple = (Tuple*)tuple.addr;
            lastOutputTuple = (Tuple*)local.addr;
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
            currentTuple = (Tuple*)tuple.addr;
            local = SetWord(currentTuple->Clone());
            result = SetWord(currentTuple);
            return YIELD;
          }
        }
        else
        {
          lastOutputTuple = (Tuple*)local.addr;
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
2.13.2 Specification of operator ~rdup~

*/
const string RdupSpec  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                         "\"Example\" ) "
                         "( <text>((stream (tuple([a1:d1, ... ,an:dn]))))"
                         " -> (stream (tuple([a1:d1, ... ,an:dn])))"
                         "</text--->"
                         "<text>_ rdup</text--->"
                         "<text>Removes duplicates from a sorted "
                         "stream.</text--->"
                         "<text>query twenty feed ten feed concat sort "
                         "rdup consume</text--->"
                              ") )";

/*
2.13.3 Definition of operator ~rdup~

*/
Operator extrelrdup (
         "rdup",             // name
         RdupSpec,           // specification
         RdupValueMapping,               // value mapping
         Operator::DummyModel,  // dummy model mapping, defines in Algebra.h
         Operator::SimpleSelect,          // trivial selection function
         IdenticalTypeMap<false>         // type mapping
);

/*
2.14 Set Operators

These operators compute set operations on two sorted stream.

2.14.1 Generic Type Mapping for Set Operations

*/
template<int errorMessageIdx> ListExpr
SetOpTypeMap( ListExpr args )
{
  ListExpr first, second;
  string argstr, argstr2;

  string setOpErrorMessages1[] =
  { "Operator mergesec expects a list of length one.",
    "Operator mergediff expects a list of length one.",
    "Operator mergeunion expects a list of length one." };
  CHECK_COND(nl->ListLength(args) == 2,
    setOpErrorMessages1[errorMessageIdx]);

  first = nl->First(args);
  second = nl->Second(args);

  nl->WriteToString(argstr, first);
  string setOpErrorMessages2[] =
  { "Operator mergesec expects as first argument a list with structure "
    "(stream (tuple ((a1 t1)...(an tn))))\n"
    "Operator mergesec gets as first argument '" + argstr + "'.",
    "Operator mergediff expects as first argument a list with structure "
    "(stream (tuple ((a1 t1)...(an tn))))\n"
    "Operator mergediff gets as first argument '" + argstr + "'.",
    "Operator mergeunion expects as first argument a list with structure "
    "(stream (tuple ((a1 t1)...(an tn))))\n"
    "Operator mergeunion gets as first argument '" + argstr + "'." };
  CHECK_COND(nl->ListLength(first) == 2  &&
             (TypeOfRelAlgSymbol(nl->First(first)) == stream) &&
             (nl->ListLength(nl->Second(first)) == 2) &&
             (TypeOfRelAlgSymbol(nl->First(nl->Second(first))) == tuple) &&
           (nl->ListLength(nl->Second(first)) == 2) &&
           (IsTupleDescription(nl->Second(nl->Second(first)))),
           setOpErrorMessages2[errorMessageIdx]);

  nl->WriteToString(argstr, second);
  string setOpErrorMessages3[] =
  { "Operator mergesec expects as second argument a list with structure "
    "(stream (tuple ((a1 t1)...(an tn))))\n"
    "Operator mergesec gets as second argument '" + argstr + "'.",
    "Operator mergediff expects as second argument a list with structure "
    "(stream (tuple ((a1 t1)...(an tn))))\n"
    "Operator mergediff gets as second argument '" + argstr + "'.",
    "Operator mergeunion expects as second argument a list with structure "
    "(stream (tuple ((a1 t1)...(an tn))))\n"
    "Operator mergeunion gets as second argument '" + argstr + "'." };
  CHECK_COND(nl->ListLength(second) == 2  &&
             (TypeOfRelAlgSymbol(nl->First(second)) == stream) &&
             (nl->ListLength(nl->Second(second)) == 2) &&
             (TypeOfRelAlgSymbol(nl->First(nl->Second(second))) == tuple) &&
	     (nl->ListLength(nl->Second(second)) == 2) &&
	     (IsTupleDescription(nl->Second(nl->Second(second)))),
	     setOpErrorMessages3[errorMessageIdx]);

  nl->WriteToString(argstr, first);
  nl->WriteToString(argstr2, second);
  string setOpErrorMessages4[] =
  { "Operator mergesec: Tuple type and attribute names of first"
    " and second argument must be equal.\n"
    "First argument is '" + argstr + "' and second argument is '" + argstr2 + "'.",
    "Operator mergediff: Tuple type and attribute names of first and second argument must be equal.\n"
    "First argument is '" + argstr + "' and second argument is '" + argstr2 + "'.",
    "Operator mergeunion: Tuple type and attribute names of first and second argument must be equal.\n"
    "First argument is '" + argstr + "' and second argument is '" + argstr2 + "'." };
  CHECK_COND((nl->Equal(first, second)),
  	      setOpErrorMessages4[errorMessageIdx]);

  return first;
}
/*
2.14.2 Auxiliary Class for Set Operations

*/

class SetOperation
{
public:
  bool outputAWithoutB;
  bool outputBWithoutA;
  bool outputMatches;

private:
  LexicographicalTupleCompare smallerThan;

  Word streamA;
  Word streamB;

  Tuple* currentATuple;
  Tuple* currentBTuple;

  Tuple* NextATuple(bool deleteOldTuple)
  {
    Word tuple;
    if(deleteOldTuple && currentATuple != 0)
    {
      currentATuple->DeleteIfAllowed();
    }

    qp->Request(streamA.addr, tuple);
    if(qp->Received(streamA.addr))
    {
      currentATuple = (Tuple*)tuple.addr;
      return currentATuple;
    }
    else
    {
      currentATuple = 0;
      return 0;
    }
  }

  Tuple* NextBTuple(bool deleteOldTuple)
  {
    Word tuple;
    if(deleteOldTuple && currentBTuple != 0)
    {
      currentBTuple->DeleteIfAllowed();
    }

    qp->Request(streamB.addr, tuple);
    if(qp->Received(streamB.addr))
    {
      currentBTuple = (Tuple*)tuple.addr;
      return currentBTuple;
    }
    else
    {
      currentBTuple = 0;
      return 0;
    }
  }

  bool TuplesEqual(Tuple* a, Tuple* b)
  {
    return !(smallerThan(a, b) || smallerThan(b, a));
  }

public:

  SetOperation(Word streamA, Word streamB)
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

  virtual ~SetOperation()
  {
    qp->Close(streamA.addr);
    qp->Close(streamB.addr);
  }

  Tuple* NextResultTuple()
  {
    Tuple* result = 0;
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

            Tuple* tmp = currentATuple;
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

            Tuple* tmp = currentBTuple;
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
            Tuple* match = currentATuple;
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
2.14.3 Generic Value Mapping Function for Set Operations

*/

template<bool outputAWithoutB, bool outputBWithoutA, bool outputMatches> int
SetOpValueMapping(Word* args, Word& result, int message, Word& local, Supplier s)
{
  SetOperation* localInfo;

  switch(message)
  {
    case OPEN:
      localInfo = new SetOperation(args[0], args[1]);
      localInfo->outputBWithoutA = outputBWithoutA;
      localInfo->outputAWithoutB = outputAWithoutB;
      localInfo->outputMatches = outputMatches;

      local = SetWord(localInfo);
      return 0;
    case REQUEST:
      localInfo = (SetOperation*)local.addr;
      result = SetWord(localInfo->NextResultTuple());
      return result.addr != 0 ? YIELD : CANCEL;
    case CLOSE:
      localInfo = (SetOperation*)local.addr;
      delete localInfo;
      return 0;
  }
  return 0;
}

/*
2.14.4 Specification of Operator ~mergesec~

*/
const string MergeSecSpec  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                         "\"Example\" ) "
                         "( <text>((stream (tuple ((x1 t1) ... "
                         "(xn tn)))) stream (tuple ((x1 t1) ... (xn tn)"
                         ")))) -> (stream (tuple ((x1 t1) ... (xn tn))))"
                         "</text--->"
                         "<text>_ _ mergesec</text--->"
                         "<text>Computes the intersection of two sorted "
                         "streams.</text--->"
                         "<text>query twenty feed oddtwenty feed mergesec"
                         " consume</text--->"
                         ") )";

/*
2.14.5 Definition of Operator ~mergesec~

*/
Operator extrelmergesec(
         "mergesec",        // name
         MergeSecSpec,     // specification
         SetOpValueMapping<false, false, true>,         // value mapping
         Operator::DummyModel,  // dummy model mapping, defines in Algebra.h
         Operator::SimpleSelect,          // trivial selection function
         SetOpTypeMap<0>   // type mapping
);

/*
2.14.6 Specification of Operator ~mergediff~

*/
const string MergeDiffSpec  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                              "\"Example\" ) "
                             "( <text>((stream (tuple ((x1 t1) ... (xn tn)"
                             "))) stream (tuple ((x1 t1) ... (xn tn))))) ->"
                             " (stream (tuple ((x1 t1) ... (xn tn))))"
                             "</text--->"
                             "<text>_ _ mergediff</text--->"
                             "<text>Computes the difference of two sorted "
                             "streams.</text--->"
                             "<text>query twenty feed oddtwenty feed"
                             " mergediff consume</text--->"
                              ") )";

/*
2.14.7 Definition of Operator ~mergediff~

*/
Operator extrelmergediff(
         "mergediff",        // name
         MergeDiffSpec,     // specification
         SetOpValueMapping<true, false, false>,         // value mapping
         Operator::DummyModel,  // dummy model mapping, defines in Algebra.h
         Operator::SimpleSelect,          // trivial selection function
         SetOpTypeMap<1>   // type mapping
);

/*
2.14.8 Specification of Operator ~mergeunion~

*/
const string MergeUnionSpec  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                               "\"Example\" ) "
                               "( <text>((stream (tuple ((x1 t1) ... (xn tn))))"
                               "stream (tuple ((x1 t1) ... (xn tn))))) -> (stream"
                               " (tuple ((x1 t1) ... (xn tn))))</text--->"
                               "<text>_ _ mergeunion</text--->"
                               "<text>Computes the union of two sorted streams."
                               "</text--->"
                               "<text>query twenty feed oddtwenty feed "
                               "mergeunion consume</text--->"
                               ") )";

/*
2.14.9 Definition of Operator ~mergeunion~

*/
Operator extrelmergeunion(
         "mergeunion",        // name
         MergeUnionSpec,     // specification
         SetOpValueMapping<true, true, true>,         // value mapping
         Operator::DummyModel,  // dummy model mapping, defines in Algebra.h
         Operator::SimpleSelect,          // trivial selection function
         SetOpTypeMap<2>   // type mapping
);

/*
2.15 Operator ~mergejoin~

This operator computes the equijoin two streams.

2.15.1 Type mapping function of operators ~mergejoin~ and ~hashjoin~

Type mapping for ~mergejoin~ is

----  ((stream (tuple ((x1 t1) ... (xn tn)))) (stream (tuple ((y1 d1) ... (ym dm)))) xi yj)

      -> (stream (tuple ((x1 t1) ... (xn tn) (y1 d1) ... (ym dm)))) APPEND (i j)
----

Type mapping for ~hashjoin~ is

----  ((stream (tuple ((x1 t1) ... (xn tn)))) (stream (tuple ((y1 d1) ... (ym dm)))) xi yj int)

      -> (stream (tuple ((x1 t1) ... (xn tn) (y1 d1) ... (ym dm)))) APPEND (i j)
----


*/
template<bool expectIntArgument, int errorMessageIdx> ListExpr JoinTypeMap
(ListExpr args)
{
  ListExpr attrTypeA, attrTypeB, joinAttrDescription;
  ListExpr streamA, streamB, list, list1, list2, outlist;
  string argstr, argstr2;

  nl->WriteToString(argstr, args);
  string joinErrorMessages1[] =
  { "Operator mergejoin expects a list of length four.\n",
    "Operator sortmergejoin expects a list of length four.\n",
    "Operator hashjoin expects a list of length five.\n" };
  CHECK_COND( nl->ListLength(args) == (expectIntArgument ? 5 : 4),
    joinErrorMessages1[errorMessageIdx]);

  streamA = nl->First(args);
  streamB = nl->Second(args);

  nl->WriteToString(argstr, streamA);
  string joinErrorMessages2[] =
  { "Operator mergejoin expects as first argument a list with structure "
    "(stream (tuple ((a1 t1)...(an tn))))\n"
    "Operator mergejoin gets as first argument '" + argstr + "'.",
    "Operator sortmergejoin expects as first argument a list with structure "
    "(stream (tuple ((a1 t1)...(an tn))))\n"
    "Operator sortmergejoin gets as first argument '" + argstr + "'.",
    "Operator hashjoin expects as first argument a list with structure "
    "(stream (tuple ((a1 t1)...(an tn))))\n"
    "Operator hashjoin gets as first argument '" + argstr + "'." };
  CHECK_COND(nl->ListLength(streamA) == 2  &&
             (TypeOfRelAlgSymbol(nl->First(streamA)) == stream) &&
             (nl->ListLength(nl->Second(streamA)) == 2) &&
             (TypeOfRelAlgSymbol(nl->First(nl->Second(streamA))) == tuple) &&
	     (nl->ListLength(nl->Second(streamA)) == 2) &&
	     (IsTupleDescription(nl->Second(nl->Second(streamA)))),
    joinErrorMessages2[errorMessageIdx]);
  list1 = nl->Second(nl->Second(streamA));

  nl->WriteToString(argstr, streamB);
  string joinErrorMessages3[] =
  { "Operator mergejoin expects as second argument a list with structure "
    "(stream (tuple ((a1 t1)...(an tn))))\n"
    "Operator mergejoin gets as second argument '" + argstr + "'.",
    "Operator sortmergejoin expects as second argument a list with structure "
    "(stream (tuple ((a1 t1)...(an tn))))\n"
    "Operator sortmergejoin gets as seond argument '" + argstr + "'.",
    "Operator hashjoin expects as second argument a list with structure "
    "(stream (tuple ((a1 t1)...(an tn))))\n"
    "Operator hashjoin gets as second argument '" + argstr + "'." };
  CHECK_COND(nl->ListLength(streamB) == 2  &&
             (TypeOfRelAlgSymbol(nl->First(streamB)) == stream) &&
             (nl->ListLength(nl->Second(streamB)) == 2) &&
             (TypeOfRelAlgSymbol(nl->First(nl->Second(streamB))) == tuple) &&
	     (nl->ListLength(nl->Second(streamB)) == 2) &&
	     (IsTupleDescription(nl->Second(nl->Second(streamB)))),
    joinErrorMessages3[errorMessageIdx]);
  list2 = nl->Second(nl->Second(streamB));

  nl->WriteToString(argstr, list1);
  nl->WriteToString(argstr2, list2);
  string joinErrorMessages4[] =
  { "Operator mergejoin: Attribute names of first and second argument "
    "list must be disjoint.\n Attribute names of first list are: '" +
    argstr + "'.\n Attribute names of second list are: '" + argstr2 + "'.",
    "Operator sortmergejoin: Attribute names of first and second argument "
    "list must be disjoint.\n Attribute names of first list are: '" +
    argstr + "'.\n Attribute names of second list are: '" + argstr2 + "'.",
    "Operator hashjoin: Attribute names of first and second argument "
    "list must be disjoint.\n Attribute names of first list are: '" +
    argstr + "'.\n Attribute names of second list are: '" + argstr2 + "'." };
  CHECK_COND(AttributesAreDisjoint(list1, list2),
    joinErrorMessages4[errorMessageIdx]);

  list = ConcatLists(list1, list2);
  outlist = nl->TwoElemList(nl->SymbolAtom("stream"),
      nl->TwoElemList(nl->SymbolAtom("tuple"), list));

  string joinErrorMessages5[] =
  { "Operator mergejoin: Join attributes must be of type SymbolType!\n",
    "Operator sortmergejoin: Join attributes must be of type SymbolType!\n",
    "Operator hashjoin: Join attributes must be of type SymbolType!\n" };
  CHECK_COND( (nl->IsAtom(nl->Third(args)) &&
               nl->IsAtom(nl->Fourth(args)) &&
               nl->AtomType(nl->Third(args)) == SymbolType &&
               nl->AtomType(nl->Fourth(args)) == SymbolType),
    joinErrorMessages5[errorMessageIdx]);

  string attrAName = nl->SymbolValue(nl->Third(args));
  string attrBName = nl->SymbolValue(nl->Fourth(args));
  int attrAIndex = FindAttribute(nl->Second(nl->Second(streamA)), attrAName, attrTypeA);
  int attrBIndex = FindAttribute(nl->Second(nl->Second(streamB)), attrBName, attrTypeB);
  nl->WriteToString(argstr, nl->Second(nl->Second(streamA)));
  string joinErrorMessages6[] =
  { "Operator mergejoin: First join attribute '" + attrAName + "' is not in "
    "first argument list '" + argstr +"'.\n",
    "Operator sortmergejoin: First join attribute '" + attrAName + "' is not in "
    "first argument list '" + argstr +"'.\n",
    "Operator hashjoin: First join attribute '" + attrAName + "' is not in "
    "first argument list '" + argstr +"'.\n" };
  CHECK_COND( attrAIndex > 0,
    joinErrorMessages6[errorMessageIdx]);
  nl->WriteToString(argstr, nl->Second(nl->Second(streamB)));
  string joinErrorMessages7[] =
  { "Operator mergejoin: Second join attribute '" + attrBName + "'is not in "
    "second argument list '" + argstr +".\n",
    "Operator sortmergejoin: Second join attribute '" + attrBName + "'is not in "
    "second argument list '" + argstr +".\n",
    "Operator hashjoin: Second join attribute '" + attrBName + "'is not in "
    "second argument list '" + argstr +".\n" };
  CHECK_COND( attrBIndex > 0,
    joinErrorMessages7[errorMessageIdx]);
  string joinErrorMessages8[] =
  { "Operator mergejoin: Type of first join attribute is different"
    " from type of second join argument.\n",
    "Operator sortmergejoin: Type of first join attribute is different"
    " from type of second join argument.\n",
    "Operator hashjoin: Type of first join attribute is different"
    " from type of second join argument.\n" };
  CHECK_COND( nl->Equal(attrTypeA, attrTypeB),
    joinErrorMessages8[errorMessageIdx]);

  if( expectIntArgument )
  {
    CHECK_COND( nl->SymbolValue(nl->Fifth(args)) == "int",
      "Operator hashjoin: Parameter 'number of buckets' must be of type int.\n" );
  }

  joinAttrDescription =
    nl->TwoElemList(nl->IntAtom(attrAIndex), nl->IntAtom(attrBIndex));
  return nl->ThreeElemList(nl->SymbolAtom("APPEND"),
              joinAttrDescription, outlist);
}
/*
2.15.2 Value mapping function of operator ~mergejoin~

*/
template<bool expectSorted> int
MergeJoin(Word* args, Word& result, int message, Word& local, Supplier s);
/*
This function will be implemented differently for the persistent and for
the main memory relational algebra. Its implementation can be found in
the files ExtRelAlgPersistent.cpp and ExtRelAlgMainMemory.cpp,
respectively.

2.15.3 Specification of operator ~mergejoin~

*/
const string MergeJoinSpec  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                              "\"Example\" ) "
                             "( <text>((stream (tuple ((x1 t1) ... "
                             "(xn tn)))) (stream (tuple ((y1 d1) ... "
                             "(ym dm)))) xi yj) -> (stream (tuple ((x1 t1)"
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
2.15.4 Definition of operator ~mergejoin~

*/
Operator extrelmergejoin(
         "mergejoin",        // name
         MergeJoinSpec,     // specification
         MergeJoin<true>,         // value mapping
         Operator::DummyModel,  // dummy model mapping, defines in Algebra.h
         Operator::SimpleSelect,          // trivial selection function
         JoinTypeMap<false, 0>   // type mapping
);

/*
2.16 Operator ~sortmergejoin~

This operator sorts two input streams and computes their equijoin.

2.16.1 Specification of operator ~sortmergejoin~

*/
const string SortMergeJoinSpec  = "( ( \"Signature\" \"Syntax\" "
                                  "\"Meaning\" \"Example\" ) "
                             "( <text>((stream (tuple ((x1 t1) ... "
                             "(xn tn)))) (stream (tuple ((y1 d1) ..."
                             " (ym dm)))) xi yj) -> (stream (tuple "
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
2.16.2 Definition of operator ~sortmergejoin~

*/
Operator extrelsortmergejoin(
         "sortmergejoin",        // name
         SortMergeJoinSpec,     // specification
         MergeJoin<false>,         // value mapping
         Operator::DummyModel,  // dummy model mapping, defines in Algebra.h
         Operator::SimpleSelect,          // trivial selection function
         JoinTypeMap<false, 1>   // type mapping
);

/*
2.17 Operator ~oldhashjoin~

This operator computes the equijoin two streams via a hash join.
The user can specify the number of hash buckets.

2.17.1 Value Mapping Function of Operator ~oldhashjoin~

*/
int OldHashJoin(Word* args, Word& result, int message, Word& local, Supplier s);
/*
This function will be implemented differently for the persistent and for
the main memory relational algebra. Its implementation can be found in
the files ExtRelAlgPersistent.cpp and ExtRelAlgMainMemory.cpp,
respectively.

2.17.2 Specification of Operator ~hashjoin~

*/
const string HashJoinSpec  = "( ( \"Signature\" \"Syntax\" "
                             "\"Meaning\" \"Example\" ) "
                          "( <text>((stream (tuple ((x1 t1) ... "
                          "(xn tn)))) (stream (tuple ((y1 d1) ... "
                          "(ym dm)))) xi yj nbuckets) -> (stream "
                          "(tuple ((x1 t1) ... (xn tn) (y1 d1) ..."
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
2.17.3 Definition of Operator ~hashjoin~

*/
Operator extreloldhashjoin(
         "oldhashjoin",        // name
         HashJoinSpec,     // specification
         OldHashJoin,         // value mapping
         Operator::DummyModel,  // dummy model mapping, defines in Algebra.h
         Operator::SimpleSelect,          // trivial selection function
         JoinTypeMap<true, 2>   // type mapping
);

/*
2.17 Operator ~hashjoin~

This operator computes the equijoin two streams via a hash join.
The user can specify the number of hash buckets.

2.17.1 Value Mapping Function of Operator ~hashjoin~

*/
int HashJoin(Word* args, Word& result, int message, Word& local, Supplier s);
/*
This function will be implemented differently for the persistent and for
the main memory relational algebra. Its implementation can be found in
the files ExtRelAlgPersistent.cpp and ExtRelAlgMainMemory.cpp,
respectively.

2.17.3 Definition of Operator ~newhashjoin~

*/
Operator extrelhashjoin(
         "hashjoin",        // name
         HashJoinSpec,     // specification
         HashJoin,         // value mapping
         Operator::DummyModel,  // dummy model mapping, defines in Algebra.h
         Operator::SimpleSelect,          // trivial selection function
         JoinTypeMap<true, 2>   // type mapping
);

/*
2.18 Operator ~extend~

Extends each input tuple by new attributes as specified in the parameter list.

2.18.1 Type mapping function of operator ~extend~

Type mapping for ~extend~ is

----     ((stream X) ((b1 (map x y1)) ... (bm (map x ym))))

        -> (stream (tuple ((a1 x1) ... (an xn) (b1 y1) ... (bm ym)))))

        where X = (tuple ((a1 x1) ... (an xn)))
----

*/
ListExpr ExtendTypeMap( ListExpr args )
{
  ListExpr first, second, rest, listn, errorInfo,
           lastlistn, first2, second2, firstr, outlist;
  //bool loopok;
  AlgebraManager* algMgr;

  algMgr = SecondoSystem::GetAlgebraManager();
  errorInfo = nl->OneElemList(nl->SymbolAtom("ERROR"));
  string argstr, argstr2;

  CHECK_COND(nl->ListLength(args) == 2,
    "Operator extend expects a list of length two.");

  first = nl->First(args);
  second  = nl->Second(args);

  nl->WriteToString(argstr, first);
  CHECK_COND(nl->ListLength(first) == 2  &&
             (TypeOfRelAlgSymbol(nl->First(first)) == stream) &&
             (nl->ListLength(nl->Second(first)) == 2) &&
             (TypeOfRelAlgSymbol(nl->First(nl->Second(first))) == tuple) &&
	     (nl->ListLength(nl->Second(first)) == 2) &&
	     (IsTupleDescription(nl->Second(nl->Second(first)))),
    "Operator extend expects as first argument a list with structure "
    "(stream (tuple ((a1 t1)...(an tn))))\n"
    "Operator extend gets as first argument '" + argstr + "'." );

  CHECK_COND(!(nl->IsAtom(second)) &&
             (nl->ListLength(second) > 0),
    "Operator extend: Second argument list may not be empty or an atom" );

  rest = nl->Second(nl->Second(first));
  listn = nl->OneElemList(nl->First(rest));
  lastlistn = listn;
  rest = nl->Rest(rest);
  while (!(nl->IsEmpty(rest)))
  {
     lastlistn = nl->Append(lastlistn,nl->First(rest));
     rest = nl->Rest(rest);
  }
  rest = second;
  while (!(nl->IsEmpty(rest)))
  {
    firstr = nl->First(rest);
    rest = nl->Rest(rest);
    first2 = nl->First(firstr);
    second2 = nl->Second(firstr);

    nl->WriteToString(argstr, first2);
    CHECK_COND( (nl->IsAtom(first2)) &&
                (nl->AtomType(first2) == SymbolType),
      "Operator extend: Attribute name '" + argstr +
      "' is not an atom or not of type SymbolType" );

    nl->WriteToString(argstr, second2);
    CHECK_COND( (nl->ListLength(second2) == 3) &&
                (TypeOfRelAlgSymbol(nl->First(second2)) == ccmap) &&
	        (algMgr->CheckKind("DATA", nl->Third(second2), errorInfo)),
      "Operator extend expects a mapping function with list structure"
      " (<attrname> (map (tuple ( (a1 t1)...(an tn) )) ti) )\n. Operator"
      " extend gets a list '" + argstr + "'.\n" );

    nl->WriteToString(argstr, nl->Second(first));
    nl->WriteToString(argstr2, second2);
    CHECK_COND( (nl->Equal(nl->Second(first),nl->Second(second2))),
      "Operator extend: Tuple type in first argument '" + argstr +
      "' is different from the argument tuple type '" + argstr2 +
      "' in the mapping function" );

    lastlistn = nl->Append(lastlistn,
        (nl->TwoElemList(first2,nl->Third(second2))));

    CHECK_COND( (nl->Equal(nl->Second(first),nl->Second(second2))),
      "Operator extend: Tuple type in first argument '" + argstr +
      "' is different from the argument tuple type '" + argstr2 +
      "' in the mapping function" );

    nl->WriteToString(argstr, listn);
    CHECK_COND( (CompareNames(listn)),
     "Operator extend: Attribute names in list '"
     + argstr + "' must be unique." );
  }
  outlist = nl->TwoElemList(nl->SymbolAtom("stream"),
            nl->TwoElemList(nl->SymbolAtom("tuple"),listn));
  return outlist;
}
/*
2.18.2 Value mapping function of operator ~extend~

*/
int Extend(Word* args, Word& result, int message, Word& local, Supplier s)
{
  Word t, value;
  Tuple* tup;
  Supplier supplier, supplier2, supplier3;
  int nooffun, noofsons;
  ArgVectorPointer funargs;
  TupleType *resultTupleType;
  ListExpr resultType;

  switch (message)
  {
    case OPEN :

      qp->Open(args[0].addr);
      resultType = GetTupleResultType( s );
      resultTupleType = new TupleType( nl->Second( resultType ) );
      local = SetWord( resultTupleType );
      return 0;

    case REQUEST :

      resultTupleType = (TupleType *)local.addr;
      qp->Request(args[0].addr,t);
      if (qp->Received(args[0].addr))
      {
        tup = (Tuple*)t.addr;
        Tuple *newTuple = new Tuple( *resultTupleType, true );
        assert( newTuple->GetNoAttributes() > tup->GetNoAttributes() );
        for( int i = 0; i < tup->GetNoAttributes(); i++ )
          newTuple->PutAttribute( i, tup->GetAttribute( i )->Clone() );
        supplier = args[1].addr;
        nooffun = qp->GetNoSons(supplier);
        assert( newTuple->GetNoAttributes() == tup->GetNoAttributes() + nooffun );
        for (int i=0; i < nooffun;i++)
        {
          supplier2 = qp->GetSupplier(supplier, i);
          noofsons = qp->GetNoSons(supplier2);
          supplier3 = qp->GetSupplier(supplier2, 1);
          funargs = qp->Argument(supplier3);
          (*funargs)[0] = SetWord(tup);
          qp->Request(supplier3,value);
          newTuple->PutAttribute( tup->GetNoAttributes()+i, ((StandardAttribute*)value.addr)->Clone() );
        }
        tup->DeleteIfAllowed();
        result = SetWord(newTuple);
        return YIELD;
      }
      else
        return CANCEL;

    case CLOSE :

      resultTupleType = (TupleType *)local.addr;
      delete resultTupleType;
      qp->Close(args[0].addr);
      return 0;
  }
  return 0;
}

/*
2.18.3 Specification of operator ~extend~

*/
const string ExtendSpec  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                           "\"Example\" ) "
                           "( <text>(stream(tuple(x)) x [(a1, (tuple(x)"
                           " -> d1)) ... (an, (tuple(x) -> dn))] -> "
                           "stream(tuple(x@[a1:d1, ... , an:dn])))"
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
2.18.4 Definition of operator ~extend~

*/
Operator extrelextend (
         "extend",              // name
         ExtendSpec,            // specification
         Extend,                // value mapping
         Operator::DummyModel,  // dummy model mapping, defines in Algebra.h
         Operator::SimpleSelect,          // trivial selection function
         ExtendTypeMap          // type mapping
);

/*
2.19 Operator ~loopjoin~

This operator will fulfill a join of two relations. Tuples in the cartesian product which satisfy certain
conditions are passed on to the output stream.

For instance,

----    query Staedte feed {s1} loopjoin[ fun(t1: TUPLE) plz feed filter [ attr(t1, SName_s1) = .Ort]] count;

        (query (count (loopjoin (rename (feed Staedte) s1) (fun (t1 TUPLE) (filter (feed plz)
               (fun (tuple1 TUPLE) (= (attr t1 SName_s1) (attr tuple1 Ort))))))))

----

The renaming is necessary whenever the underlying relations have at least one common attribute name
in order to assure that the output tuple stream consists of different named attributes.

The type mapping function of the loopjoin operation is as follows:

----    ((stream (tuple x)) (map (tuple x) (stream (tuple y))))  -> (stream (tuple x * y))
  where x = ((x1 t1) ... (xn tn)) and y = ((y1 d1) ... (ym dm))
----

*/
ListExpr LoopjoinTypeMap(ListExpr args)
{
  ListExpr first, second;
  ListExpr list1, list2, list, outlist;
  string argstr, argstr2;

  CHECK_COND(nl->ListLength(args) == 2,
    "Operator loopjoin expects a list of length two.");

  first = nl->First(args);
  second  = nl->Second(args);

  nl->WriteToString(argstr, first);
  CHECK_COND(nl->ListLength(first) == 2  &&
             (TypeOfRelAlgSymbol(nl->First(first)) == stream) &&
             (nl->ListLength(nl->Second(first)) == 2) &&
             (TypeOfRelAlgSymbol(nl->First(nl->Second(first))) == tuple) &&
	     (nl->ListLength(nl->Second(first)) == 2) &&
	     (IsTupleDescription(nl->Second(nl->Second(first)))),
    "Operator loopjoin expects as first argument a list with structure "
    "(stream (tuple ((a1 t1)...(an tn))))\n"
    "Operator loopjoin gets as first argument '" + argstr + "'." );

  nl->WriteToString(argstr, second);
  CHECK_COND( (nl->ListLength(second) == 3) &&
             (TypeOfRelAlgSymbol(nl->First(second)) == ccmap) &&
             (nl->ListLength(nl->Third(second)) == 2) &&
             (TypeOfRelAlgSymbol(nl->First(nl->Third(second))) == stream) &&
             (nl->ListLength(nl->Second(nl->Third(second))) == 2) &&
        (TypeOfRelAlgSymbol(nl->First(nl->Second(nl->Third(second)))) == tuple),
    "Operator loopjoin expects as second argument a list with length three"
    " and structure (map (tuple (...)) (stream (tuple (...)))).\n"
    " Operator loopjoin gets as second argument '" + argstr + "'.\n" );

  CHECK_COND((nl->Equal(nl->Second(first), nl->Second(second))),
    "Operator loopjoin: Input tuple for mapping and the first argument "
    "tuple must have the same description. " );

  list1 = nl->Second(nl->Second(first));
  list2 = nl->Second(nl->Second(nl->Third(second)));

  nl->WriteToString(argstr, list1);
  nl->WriteToString(argstr, list2);
  CHECK_COND( (AttributesAreDisjoint(list1, list2)),
  "Attribute names in first and second argument must be disjoint.\n"
  "First argument: '" + argstr + "'.\n"
  "Second argument: '" + argstr2 + "'.\n" );

  list = ConcatLists(list1, list2);
  outlist = nl->TwoElemList(nl->SymbolAtom("stream"),
  nl->TwoElemList(nl->SymbolAtom("tuple"), list));
  return outlist;
}



  /*if(nl->ListLength(args) == 2)
  {
    first = nl->First(args);
    second  = nl->Second(args);

    if( (nl->ListLength(first) == 2) &&
        (TypeOfRelAlgSymbol(nl->First(first)) == stream) &&
        (nl->ListLength(nl->Second(first)) == 2) &&
        (TypeOfRelAlgSymbol(nl->First(nl->Second(first))) == tuple) &&

        (nl->ListLength(second) == 3) &&
        (TypeOfRelAlgSymbol(nl->First(second)) == ccmap) &&

        (nl->Equal(nl->Second(first), nl->Second(second))) &&

        (nl->ListLength(nl->Third(second)) == 2) &&
        (TypeOfRelAlgSymbol(nl->First(nl->Third(second))) == stream) &&
        (nl->ListLength(nl->Second(nl->Third(second))) == 2) &&
        (TypeOfRelAlgSymbol(nl->First(nl->Second(nl->Third(second)))) == tuple) )
    {
      list1 = nl->Second(nl->Second(first));
      list2 = nl->Second(nl->Second(nl->Third(second)));
      if(!AttributesAreDisjoint(list1, list2))
      {
        goto typeerror;
      }
      list = ConcatLists(list1, list2);
      outlist = nl->TwoElemList(nl->SymbolAtom("stream"),
      nl->TwoElemList(nl->SymbolAtom("tuple"), list));
      return outlist;
    }
    else goto typeerror;
  }
  else goto typeerror;

typeerror:
  ErrorReporter::ReportError("Incorrect input for operator loopjoin.");
  return nl->SymbolAtom("typeerror");
}*/

/*
2.19.2 Value mapping function of operator ~loopjoin~

*/

struct LoopjoinLocalInfo
{
  Word tuplex;
  Word streamy;
  TupleType *resultTupleType;
};

int Loopjoin(Word* args, Word& result, int message, Word& local, Supplier s)
{
  ArgVectorPointer funargs = 0;
  
  Word tuplex = SetWord(Address(0));
  Word tupley = SetWord(Address(0));
  Word tuplexy = SetWord(Address(0));
  Word streamy = SetWord(Address(0));
 
  Tuple* ctuplex = 0;
  Tuple* ctupley = 0;
  Tuple* ctuplexy = 0;
 
  LoopjoinLocalInfo *localinfo = 0;

  switch ( message )
  {
    case OPEN:
      qp->Open (args[0].addr);
      qp->Request(args[0].addr, tuplex);
      if (qp->Received(args[0].addr))
      {
        funargs = qp->Argument(args[1].addr);
        (*funargs)[0] = tuplex;
        streamy=args[1];
        qp->Open (streamy.addr);

        localinfo = new LoopjoinLocalInfo;
        ListExpr resultType = GetTupleResultType( s );
        localinfo->resultTupleType = new TupleType( nl->Second( resultType ) );
        localinfo->tuplex = tuplex;
        localinfo->streamy = streamy;
        local = SetWord(localinfo);
      }
      else
      {
        local = SetWord(Address(0));
      }
      return 0;

    case REQUEST:
      if (local.addr ==0) return CANCEL;
      localinfo=(LoopjoinLocalInfo *) local.addr;
      tuplex=localinfo->tuplex;
      ctuplex=(Tuple*)tuplex.addr;
      streamy=localinfo->streamy;
      tupley=SetWord(Address(0));
      while (tupley.addr==0)
      {
        qp->Request(streamy.addr, tupley);
        if (!(qp->Received(streamy.addr)))
        {
          qp->Close(streamy.addr);
          ((Tuple*)tuplex.addr)->DeleteIfAllowed();
          qp->Request(args[0].addr, tuplex);
          if (qp->Received(args[0].addr))
          {
            funargs = qp->Argument(args[1].addr);
            ctuplex=(Tuple*)tuplex.addr;
            (*funargs)[0] = tuplex;
            streamy=args[1];
            qp->Open (streamy.addr);
            tupley=SetWord(Address(0));

            localinfo->tuplex=tuplex;
            localinfo->streamy=streamy;
            local =  SetWord(localinfo);
          }
          else
          {
            localinfo->streamy = SetWord(0);
            localinfo->tuplex = SetWord(0);
            return CANCEL;
          }
        }
        else
        {
          ctupley=(Tuple*)tupley.addr;
        }
      }
      ctuplexy = new Tuple( *localinfo->resultTupleType, true );
      assert( ctuplexy->IsFree() == true );
      tuplexy = SetWord(ctuplexy);
      Concat(ctuplex, ctupley, ctuplexy);
      ctupley->DeleteIfAllowed();
      result = tuplexy;
      return YIELD;

    case CLOSE:
      if( local.addr != 0 )
      {
        localinfo=(LoopjoinLocalInfo *) local.addr;
        if( localinfo->streamy.addr != 0 )
          qp->Close( localinfo->streamy.addr );

        if( localinfo->tuplex.addr != 0 )
          ((Tuple*)localinfo->tuplex.addr)->DeleteIfAllowed();

        delete localinfo->resultTupleType;
        delete localinfo;
      }
      qp->Close(args[0].addr);
      return 0;
  }

  return 0;
}

/*
2.19.3 Specification of operator ~loopjoin~

*/
const string LoopjoinSpec  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                             "\"Example\" ) "
                             "( <text>((stream tuple1) (map tuple1 "
                             "rel(tuple2))) -> (stream tuple1*tuple2)"
                             "</text--->"
                             "<text>_ loopjoin [ fun ]</text--->"
                             "<text>Only tuples in the cartesian product "
                             "which satisfy certain conditions are passed on"
                             " to the output stream. Note: The input tuples must"
                             " have different attribute names, hence renaming may be applied"
                             " to one of the input streams.</text--->"
                             "<text>query Staedte feed {s1} loopjoin[ fun(t1: TUPLE) plz feed"
                             " filter [ attr(t1, SName_s1) = .Ort]] count</text--->"
                             ") )";

/*
2.19.4 Definition of operator ~loopjoin~

*/
Operator extrelloopjoin (
         "loopjoin",                // name
         LoopjoinSpec,              // specification
         Loopjoin,                  // value mapping
         Operator::DummyModel,  // dummy model mapping, defines in Algebra.h
         Operator::SimpleSelect,            // trivial selection function
         LoopjoinTypeMap            // type mapping
);

/*

2.20 Operator ~loopselect~

This operator is similar to the ~loopjoin~ operator except that it only returns the inner tuple
(instead of the concatination of two tuples). Tuples in the cartesian product which satisfy
certain conditions are passed on to the output stream.

For instance,

----    query Staedte feed loopsel [ fun(t1: TUPLE) plz feed filter [ attr(t1, SName) = .Ort ]] count;

        (query (count (loopsel (feed Staedte) (fun (t1 TUPLE) (filter (feed plz)
                    (fun (tuple1 TUPLE) (= (attr t1 SName) (attr tuple1 Ort))))))))

----

7.4.1 Type mapping function of operator ~loopsel~

The type mapping function of the loopsel operation is as follows:

----    ((stream (tuple x)) (map (tuple x) (stream (tuple y))))  -> (stream (tuple y))
        where x = ((x1 t1) ... (xn tn)) and y = ((y1 d1) ... (ym dm))
----

*/
ListExpr
LoopselectTypeMap(ListExpr args)
{
  ListExpr first, second;
  ListExpr list, outlist;
  string argstr;

  CHECK_COND(nl->ListLength(args) == 2,
    "Operator loopsel expects a list of length two.");

  first = nl->First(args);
  second  = nl->Second(args);

  nl->WriteToString(argstr, first);
  CHECK_COND(nl->ListLength(first) == 2  &&
             (TypeOfRelAlgSymbol(nl->First(first)) == stream) &&
             (nl->ListLength(nl->Second(first)) == 2) &&
             (TypeOfRelAlgSymbol(nl->First(nl->Second(first))) == tuple) &&
	     (nl->ListLength(nl->Second(first)) == 2) &&
	     (IsTupleDescription(nl->Second(nl->Second(first)))),
    "Operator loopsel expects as first argument a list with structure "
    "(stream (tuple ((a1 t1)...(an tn))))\n"
    "Operator loopsel gets as first argument '" + argstr + "'." );

  nl->WriteToString(argstr, second);
  CHECK_COND( (nl->ListLength(second) == 3) &&
             (TypeOfRelAlgSymbol(nl->First(second)) == ccmap) &&
             (nl->ListLength(nl->Third(second)) == 2) &&
             (TypeOfRelAlgSymbol(nl->First(nl->Third(second))) == stream) &&
             (nl->ListLength(nl->Second(nl->Third(second))) == 2) &&
        (TypeOfRelAlgSymbol(nl->First(nl->Second(nl->Third(second)))) == tuple),
    "Operator loopsel expects as second argument a list with length three"
    " and structure (map (tuple (...)) (stream (tuple (...)))).\n"
    " Operator loopsel gets as second argument '" + argstr + "'.\n" );

  CHECK_COND((nl->Equal(nl->Second(first), nl->Second(second))),
    "Operator loopsel: Input tuple for mapping and the first argument "
    "tuple must have the same description. " );

  list = nl->Second(nl->Second(nl->Third(second)));
  outlist = nl->TwoElemList(nl->SymbolAtom("stream"),
  nl->TwoElemList(nl->SymbolAtom("tuple"), list));
  return outlist;
}


  /*if(nl->ListLength(args) == 2)
  {
    first = nl->First(args);
    second  = nl->Second(args);

    if( (nl->ListLength(first) == 2) &&
        (TypeOfRelAlgSymbol(nl->First(first)) == stream) &&
        (nl->ListLength(nl->Second(first)) == 2) &&
        (TypeOfRelAlgSymbol(nl->First(nl->Second(first))) == tuple) &&

        (nl->ListLength(second) == 3) &&
        (TypeOfRelAlgSymbol(nl->First(second)) == ccmap) &&
        (nl->Equal(nl->Second(first), nl->Second(second))) &&
        (nl->ListLength(nl->Third(second)) == 2) &&
        (TypeOfRelAlgSymbol(nl->First(nl->Third(second))) == stream) &&
        (nl->ListLength(nl->Second(nl->Third(second))) == 2) &&
        (TypeOfRelAlgSymbol(nl->First(nl->Second(nl->Third(second)))) == tuple) )
    {
      list1 = nl->Second(nl->Second(first));
      list2 = nl->Second(nl->Second(nl->Third(second)));
      outlist = nl->TwoElemList(nl->SymbolAtom("stream"),
      nl->TwoElemList(nl->SymbolAtom("tuple"), list2));
      return outlist;
    }
    else goto typeerror;
  }
  else goto typeerror;

typeerror:
  ErrorReporter::ReportError("Incorrect input for operator loopselect.");
  return nl->SymbolAtom("typeerror");
}*/

/*

4.1.2 Value mapping function of operator ~loopsel~

*/
struct LoopselectLocalInfo
{
  Word tuplex;
  Word streamy;
  TupleType *resultTupleType;
};

int
Loopselect(Word* args, Word& result, int message, Word& local, Supplier s)
{
  ArgVectorPointer funargs;
  Word tuplex, tupley, streamy;
  Tuple* ctuplex;
  Tuple* ctupley;
  LoopselectLocalInfo *localinfo;

  switch ( message )
  {
    case OPEN:
      // open the stream and initiate the variables
      qp->Open (args[0].addr);
      qp->Request(args[0].addr, tuplex);
      if (qp->Received(args[0].addr))
      {
        // compute the rely which corresponding to tuplex
        funargs = qp->Argument(args[1].addr);
        (*funargs)[0] = tuplex;
        streamy = args[1];
        qp->Open(streamy.addr);

        // put the information of tuplex and rely into local
        localinfo = new LoopselectLocalInfo;
        ListExpr resultType = GetTupleResultType( s );
        localinfo->resultTupleType = new TupleType( nl->Second( resultType ) );
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

      // restore localinformation from the local variable.
      localinfo = (LoopselectLocalInfo *) local.addr;
      tuplex = localinfo->tuplex;
      ctuplex = (Tuple*)tuplex.addr;
      streamy = localinfo->streamy;
      // prepare tuplex and tupley for processing. if rely is exausted: fetch next tuplex.
      tupley=SetWord(Address(0));
      while (tupley.addr==0)
      {
        qp->Request(streamy.addr, tupley);
        if (!(qp->Received(streamy.addr)))
        {
          qp->Close(streamy.addr);
          ((Tuple*)tuplex.addr)->DeleteIfAllowed();
          qp->Request(args[0].addr, tuplex);
          if (qp->Received(args[0].addr))
          {
            funargs = qp->Argument(args[1].addr);
            ctuplex = (Tuple*)tuplex.addr;
            (*funargs)[0] = tuplex;
            streamy = args[1];
            qp->Open(streamy.addr);
            tupley = SetWord(Address(0));

            localinfo->tuplex=tuplex;
            localinfo->streamy=streamy;
            local = SetWord(localinfo);
          }
          else
          {
            localinfo->streamy = SetWord(0);
            localinfo->tuplex = SetWord(0);
            return CANCEL;
          }
        }
        else
        {
          ctupley = (Tuple*)tupley.addr;
        }
      }

      result = tupley;
      return YIELD;

    case CLOSE:
      if( local.addr != 0 )
      {
        localinfo=(LoopselectLocalInfo *) local.addr;

        if( localinfo->streamy.addr != 0 )
          qp->Close( localinfo->streamy.addr );

        if( localinfo->tuplex.addr != 0 )
          ((Tuple*)localinfo->tuplex.addr)->DeleteIfAllowed();

        delete localinfo->resultTupleType;
        delete localinfo;
      }
      qp->Close(args[0].addr);
      return 0;
  }

  return 0;
}

/*

4.1.3 Specification of operator ~loopsel~

*/
const string LoopselectSpec  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                               "\"Example\" ) "
                               "( <text>((stream tuple1) (map tuple1 "
                               "rel(tuple2))) -> (stream tuple2)"
                               "</text--->"
                               "<text>_ loopselect [ fun ]</text--->"
                               "<text>Only tuples in the cartesian product "
                               "which satisfy certain conditions are passed on"
                               " to the output stream.</text--->"
                               "<text>query Staedte feed loopsel [ fun(t1: TUPLE) plz feed "
                               "filter [ attr(t1, SName) = .Ort ]] count</text--->"
                               ") )";
/*

4.1.3 Definition of operator ~loopsel~

*/
Operator extrelloopsel (
         "loopsel",                             // name
         LoopselectSpec,                      // specification
         Loopselect,                          // value mapping
         Operator::DummyModel,  // dummy model mapping, defines in Algebra.h
         Operator::SimpleSelect,                        // trivial selection function
         LoopselectTypeMap                    // type mapping
);

/*
2.21 Operator ~extendstream~

This operator is implimented by the idea of LOOPJOIN and EXTEND

The type mapping function of the extendstream operation is as follows:

----    ((stream (tuple x)) (map (tuple x) (stream (y))))  -> (stream (tuple x * y))
  where x = ((x1 t1) ... (xn tn)) and y is a simple data type such as string, integer, or real ...
----

----    ((stream (tuple x)) ( (b1 (map (tuple x) (stream (y)))))  -> (stream (tuple x * y))
  where x = ((x1 t1) ... (xn tn)) and y is a simple data type such as string, integer, or real ...
----

For instance,

----  query people feed extendstream [parts : components (.name) ] consume;
                  ----------------                            --------------------------------------
              [ (stream(tuple x))              (b1 (map (tuple x) (stream (y1)) ) ) ]    -->   (stream (tuple x + (b1 y)) )
	                                                                                                                                          ------------
      where x = ((x1 t1) ... (xn tn)) and y is a simple data type such as string, integer, or real ...
----

*/

ListExpr ExtendStreamTypeMap(ListExpr args)
{
  ListExpr first, second;
  ListExpr listX, listY, list, outlist, errorInfo;
  AlgebraManager* algMgr;
  algMgr = SecondoSystem::GetAlgebraManager();
  errorInfo = nl->OneElemList(nl->SymbolAtom("ERROR"));
  string argstr, argstr2;

  CHECK_COND(nl->ListLength(args) == 2,
    "Operator extendstream expects a list of length two.");

  first = nl->First(args);
  second  = nl->Second(args);

  nl->WriteToString(argstr, first);
  CHECK_COND(nl->ListLength(first) == 2  &&
             (TypeOfRelAlgSymbol(nl->First(first)) == stream) &&
             (nl->ListLength(nl->Second(first)) == 2) &&
             (TypeOfRelAlgSymbol(nl->First(nl->Second(first))) == tuple) &&
	     (nl->ListLength(nl->Second(first)) == 2) &&
	     (IsTupleDescription(nl->Second(nl->Second(first)))),
    "Operator extendstream expects as first argument a list with structure "
    "(stream (tuple ((a1 t1)...(an tn))))\n"
    "Operator extendstream gets as first argument '" + argstr + "'." );

  nl->WriteToString(argstr, second);
  CHECK_COND( !nl->IsAtom(second) &&
     nl->ListLength(second) == 1 &&
     nl->ListLength(nl->First(second)) == 2 &&
     TypeOfRelAlgSymbol(nl->First(nl->Second(nl->First(second)))) == ccmap &&
     nl->ListLength(nl->Third(nl->Second(nl->First(second)))) == 2 &&
     TypeOfRelAlgSymbol(nl->First(nl->Third(nl->Second(nl->First(second)))) == stream),
    "Operator extendstream expects as second argument a list with length two"
    " and structure (<attrname>(map (tuple (...)) (stream <type(DATA)>))).\n"
    " Operator extendstream gets as second argument '" + argstr + "'.\n" );
  second = nl->First(second);

  CHECK_COND((nl->Equal(nl->Second(first), nl->Second(nl->Second(second)))),
    "Operator extendstream: Input tuple for mapping and the first argument "
    "tuple must have the same description. " );

  nl->WriteToString(argstr,
                   nl->Second(nl->Third(nl->Second(second))));
  CHECK_COND((nl->IsAtom(nl->Second(nl->Third(nl->Second(second)))))  &&
             (algMgr->CheckKind("DATA",
	       nl->Second(nl->Third(nl->Second(second))), errorInfo)),
    "Operator extendstream: Objects in the second "
    "stream must be of kind DATA.\n"
    "Operator extendstream: Objects are of type '" + argstr + "'.\n" );

  listX = nl->Second(nl->Second(first));

  listY = nl->OneElemList(nl->TwoElemList(
	  nl->First(second),
	  nl->Second(nl->Third(nl->Second(second)))));

  nl->WriteToString(argstr, listX);
  nl->WriteToString(argstr2, listY);
  CHECK_COND( (AttributesAreDisjoint(listX, listY)),
  "New Attribute name must be different from argument"
  " names in first argument list.\n"
  "First argument list: '" + argstr + "'.\n"
  "New attribute name and type: '" + argstr2 + "'.\n" );

  list = ConcatLists(listX, listY);
  outlist = nl->TwoElemList(
    nl->SymbolAtom("stream"),
    nl->TwoElemList(nl->SymbolAtom("tuple"), list));
  return outlist;
}
/*
2.19.2 Value mapping function of operator ~extendstream~

*/

struct ExtendStreamLocalInfo
{
  Tuple *tupleX;
  Word streamY;
  TupleType *resultTupleType;
};

int ExtendStream(Word* args, Word& result, int message, Word& local, Supplier s)
{
  //cout<<"ExtendStreamValueMap CALLED!!!"<<endl;
  ArgVectorPointer funargs;
  Word wTupleX, wValueY;
  Tuple* tupleXY;
  ExtendStreamLocalInfo *localinfo;

  TupleType *resultTupleType;
  ListExpr resultType;

  Supplier supplier, supplier2, supplier3;

  switch( message )
  {
    case OPEN:
    {
      //1. open the input stream and initiate the arguments
      qp->Open( args[0].addr );
      qp->Request( args[0].addr, wTupleX );
      if( qp->Received( args[0].addr ) )
      {
        //2. compute the result "stream y" from tuple x
        //funargs = qp->Argument(args[1].addr);   //here should be changed to the following...
        supplier = args[1].addr;
        supplier2 = qp->GetSupplier( supplier, 0 );
        supplier3 = qp->GetSupplier( supplier2, 1 );
        funargs = qp->Argument( supplier3 );

        (*funargs)[0] = wTupleX;
        qp->Open( supplier3 ); 

        //3. save the local information
        localinfo = new ExtendStreamLocalInfo;
        resultType = GetTupleResultType( s );
        localinfo->resultTupleType = new TupleType( nl->Second( resultType ) );
        localinfo->tupleX = (Tuple*)wTupleX.addr;
        localinfo->streamY = SetWord( supplier3 );
        local = SetWord(localinfo);
      }
      else
      {
        local = SetWord(Address(0));
      }
      return 0;
    }
    case REQUEST:
    {
      if( local.addr == 0 ) 
        return CANCEL;

      //1. recover local information
      localinfo=(ExtendStreamLocalInfo *) local.addr;
      resultTupleType = (TupleType *)localinfo->resultTupleType;

      //2. prepare tupleX and wValueY. If wValueY is empty, then get next tupleX
      wValueY = SetWord(Address(0));
      while( wValueY.addr == 0 )
      {
        qp->Request( localinfo->streamY.addr, wValueY );
        if( !(qp->Received( localinfo->streamY.addr )) )
        {
          qp->Close( localinfo->streamY.addr );
          localinfo->tupleX->DeleteIfAllowed();
          qp->Request( args[0].addr, wTupleX );
          if( qp->Received(args[0].addr) )
          {
            supplier = args[1].addr;
            supplier2 = qp->GetSupplier(supplier, 0);
            supplier3 = qp->GetSupplier(supplier2, 1);
            funargs = qp->Argument(supplier3);

            localinfo->tupleX = (Tuple*)wTupleX.addr;
            (*funargs)[0] = wTupleX;
            qp->Open( supplier3 );

            localinfo->tupleX = (Tuple*)wTupleX.addr;
            localinfo->streamY = SetWord(supplier3);

            wValueY = SetWord(Address(0));
          }
          else  //streamx is exausted
          {
            localinfo->streamY = SetWord(0);
            localinfo->tupleX = 0;
            return CANCEL;
          }
        }
      }

      //3. compute tupleXY from tupleX and wValueY
      tupleXY = new Tuple( *localinfo->resultTupleType, true );
      assert( tupleXY->IsFree() == true );

      for( int i = 0; i < localinfo->tupleX->GetNoAttributes(); i++ )
        tupleXY->PutAttribute( i, localinfo->tupleX->GetAttribute( i )->Clone() );

      tupleXY->PutAttribute( localinfo->tupleX->GetNoAttributes(), ((StandardAttribute*)wValueY.addr)->Clone() );

      // deleting wValueY
      const AttributeType& yAttributeType = 
        localinfo->resultTupleType->GetAttributeType( localinfo->resultTupleType->GetNoAttributes() - 1 );
      (SecondoSystem::GetAlgebraManager()->DeleteObj( yAttributeType.algId, yAttributeType.typeId ))( wValueY );

      // setting the result
      result = SetWord( tupleXY );
      return YIELD;
    }
    case CLOSE:
    {
      if( local.addr != 0 )
      {
        localinfo = (ExtendStreamLocalInfo *)local.addr;

        if( localinfo->streamY.addr != 0 )
          qp->Close( localinfo->streamY.addr );

        if( localinfo->tupleX != 0 )
          localinfo->tupleX->DeleteIfAllowed();

        delete localinfo->resultTupleType;
        delete localinfo;
      }
      qp->Close( args[0].addr );
      return 0;
    }
  }
  return 0;
}

/*
2.19.3 Specification of operator ~extendstream~

*/
const string ExtendStreamSpec  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                             "\"Example\" ) "
                             "( <text>((stream tuple1) (map tuple1 "
                             "stream(type))) -> (stream tuple1*tuple2)"
                             "</text--->"
                             "<text>_ extendstream [ fun ]</text--->"
                             "<text>This operator do the loopjoin between"
                             "a stream of tuples and a stream of objects of a certain type."
                             " the result is a stream of tuples.</text--->"
	             "<text>query UBahn feed extendstream"
	             "[ newattr:  units(.Trajectory)] consume</text--->"
                             ") )";

/*
2.19.4 Definition of operator ~extendstream~

*/
Operator extrelextendstream (
         "extendstream",                // name
         ExtendStreamSpec,              // specification
         ExtendStream,                  // value mapping
         Operator::DummyModel,  // dummy model mapping, defines in Algebra.h
         Operator::SimpleSelect,            // trivial selection function
         ExtendStreamTypeMap            // type mapping
);

/*
2.21 Operator ~projectextendstream~

This operator does the same as ~extendstream~ with a projection on some attributes. It is
very often that a big attribute is converted into a stream of a small pieces, for example,
a text into keywords. When applying the ~extendstream~ operator, the big attribute belongs
to the result type, and is copied for every occurrence of its smaller pieces. A projection
is a normal operation after such operation. To avoid this copying, the projection operation
is now built in the operation. 

The type mapping function of the ~projectextendstream~ operation is as follows:

----  ((stream (tuple ((x1 T1) ... (xn Tn)))) (ai1 ... aik) 
        (map (tuple ((x1 T1) ... (xn Tn))) (b stream(Tb))))  ->
      (APPEND
        (k (i1 ... ik))
        (stream (tuple ((ai1 Ti1) ... (aik Tik)(b Tb)))))
----

For instance,

----  query people feed projectextendstream [id, age; parts : .name keywords] consume;
----

*/
ListExpr ProjectExtendStreamTypeMap(ListExpr args)
{
  ListExpr errorInfo;
  AlgebraManager* algMgr;
  algMgr = SecondoSystem::GetAlgebraManager();
  errorInfo = nl->OneElemList(nl->SymbolAtom("ERROR"));
  string argstr, argstr2;

  CHECK_COND(nl->ListLength(args) == 3,
    "Operator projectextendstream expects a list of length three.");

  ListExpr first = nl->First(args),
           second = nl->Second(args),
           third = nl->Third(args);

nl->WriteListExpr( first );
cout << endl;
nl->WriteListExpr( second );
cout << endl;
nl->WriteListExpr( third );
cout << endl;

  nl->WriteToString(argstr, first);
  CHECK_COND(nl->ListLength(first) == 2  &&
             (TypeOfRelAlgSymbol(nl->First(first)) == stream) &&
             (nl->ListLength(nl->Second(first)) == 2) &&
             (TypeOfRelAlgSymbol(nl->First(nl->Second(first))) == tuple) &&
       (nl->ListLength(nl->Second(first)) == 2) &&
       (IsTupleDescription(nl->Second(nl->Second(first)))),
    "Operator projectextendstream expects as first argument a list with structure "
    "(stream (tuple ((a1 t1)...(an tn))))\n"
    "Operator projectextendstream gets as first argument '" + argstr + "'." );

  nl->WriteToString(argstr, second);
  CHECK_COND( !nl->IsAtom(second) &&
     nl->ListLength(second) > 0, 
    "Operator projectextendstream expects as second argument a list of attribute names\n"
    " Operator projectextendstream gets as second argument '" + argstr + "'.\n" );

  nl->WriteToString(argstr, third);
  CHECK_COND( !nl->IsAtom(third) &&
     nl->ListLength(third) == 1 &&
     nl->ListLength(nl->First(third)) == 2 &&
     TypeOfRelAlgSymbol(nl->First(nl->Second(nl->First(third)))) == ccmap &&
     nl->ListLength(nl->Third(nl->Second(nl->First(third)))) == 2 &&
     TypeOfRelAlgSymbol(nl->First(nl->Third(nl->Second(nl->First(third)))) == stream),
    "Operator projectextendstream expects as third argument a list with length two"
    " and structure (<attrname>(map (tuple (...)) (stream <type(DATA)>))).\n"
    " Operator projectextendstream gets as third argument '" + argstr + "'.\n" );
  third = nl->First(third);

  ListExpr secondRest = second, secondFirst, attrType, 
           newAttrList, numberList,
           lastNewAttrList, lastNumberList;
  string attrName;
  bool firstCall = true;
  while( !nl->IsEmpty(secondRest) )
  {
    secondFirst = nl->First(secondRest);
    secondRest = nl->Rest(secondRest);
    if( nl->AtomType(secondFirst) == SymbolType )
    {
      attrName = nl->SymbolValue(secondFirst);
    }
    else
    {
      nl->WriteToString(argstr, secondFirst);
      ErrorReporter::ReportError( 
        "Operator projectextendstream expects as second argument a list of attribute names\n"
        " The element '" + argstr + "' is not an attribute name.\n" );
      return nl->SymbolAtom("typeerror");
    }

    int j = FindAttribute( nl->Second(nl->Second(first)), attrName, attrType );
    if( j )
    {
      if( firstCall )
      {
        firstCall = false;
        newAttrList = nl->OneElemList(nl->TwoElemList(secondFirst, attrType));
        lastNewAttrList = newAttrList;
        numberList = nl->OneElemList(nl->IntAtom(j));
        lastNumberList = numberList;
      }
      else
      {
        lastNewAttrList =
          nl->Append( lastNewAttrList, nl->TwoElemList(secondFirst, attrType) );
        lastNumberList =
          nl->Append( lastNumberList, nl->IntAtom(j) );
      }
    }
    else
    {
      nl->WriteToString(argstr, first);
      ErrorReporter::ReportError(
        "Operator projectextendstream expects as second argument a list of attribute names\n"
        " Attribute name '" + attrName + "' does not belong to the tuple stream: \n"
        "'" + argstr + "'.\n" );
      return nl->SymbolAtom("typeerror");
    }
  }


  CHECK_COND((nl->Equal(nl->Second(first), nl->Second(nl->Second(third)))),
    "Operator projectextendstream: Input tuple for mapping (third argument) and the first argument\n"
    "tuple must have the same description." );

  nl->WriteToString(argstr,
                    nl->Second(nl->Third(nl->Second(third))));
  CHECK_COND((nl->IsAtom(nl->Second(nl->Third(nl->Second(third)))))  &&
             (algMgr->CheckKind("DATA",
               nl->Second(nl->Third(nl->Second(third))), errorInfo)),
    "Operator projectextendstream: the return stream value in the third argument\n"
    "must implement the kind DATA.\n"
    "The return stream value is of type '" + argstr + "'.\n" );

  ListExpr appendAttr = nl->TwoElemList(
                          nl->First(third),
                          nl->Second(nl->Third(nl->Second(third))));

  nl->WriteToString(argstr, nl->Second(nl->Second(first)));
  nl->WriteToString(argstr2, appendAttr);
  CHECK_COND( AttributesAreDisjoint( nl->Second(nl->Second(first)), nl->OneElemList(appendAttr) ),
    "Operator projectextendstream: new attribute name '" + argstr2 + "' must be different\n"
    "from the attribute names in first argument list.\n"
    "First argument list: '" + argstr + "'.\n" );


  lastNewAttrList =
          nl->Append( lastNewAttrList, appendAttr );  
  
  return nl->ThreeElemList(
           nl->SymbolAtom("APPEND"),
           nl->TwoElemList(
             nl->IntAtom( nl->ListLength(second) ), 
             numberList),
           nl->TwoElemList(
             nl->SymbolAtom("stream"),
               nl->TwoElemList(
                 nl->SymbolAtom("tuple"),
                 newAttrList)));
}

/*
2.19.2 Value mapping function of operator ~projectextendstream~

*/
struct ProjectExtendStreamLocalInfo
{
  Tuple *tupleX;
  Word streamY;
  TupleType *resultTupleType;
  vector<long> attrs;
};

int ProjectExtendStream(Word* args, Word& result, int message, Word& local, Supplier s)
{
  ArgVectorPointer funargs;
  Word wTupleX, wValueY;
  Tuple* tupleXY;
  ProjectExtendStreamLocalInfo *localinfo;

  TupleType *resultTupleType;
  ListExpr resultType;

  Supplier supplier, supplier2, supplier3;

  switch( message )
  {
    case OPEN:
    {
      //1. open the input stream and initiate the arguments
      qp->Open( args[0].addr );
      qp->Request( args[0].addr, wTupleX );
      if( qp->Received( args[0].addr ) )
      {
        //2. compute the result "stream y" from tuple x
        supplier = args[2].addr;
        supplier2 = qp->GetSupplier( supplier, 0 );
        supplier3 = qp->GetSupplier( supplier2, 1 );
        funargs = qp->Argument( supplier3 );

        (*funargs)[0] = wTupleX;
        qp->Open( supplier3 );

        //3. save the local information
        localinfo = new ProjectExtendStreamLocalInfo;
        resultType = GetTupleResultType( s );
        localinfo->resultTupleType = new TupleType( nl->Second( resultType ) );
        localinfo->tupleX = (Tuple*)wTupleX.addr;
        localinfo->streamY = SetWord( supplier3 );

        //4. get the attribute numbers
        Word arg2;
        qp->Request(args[3].addr, arg2);
        int noOfAttrs = ((CcInt*)arg2.addr)->GetIntval();
        for( int i = 0; i < noOfAttrs; i++)
        {
          Supplier son = qp->GetSupplier(args[4].addr, i);
          Word elem2;
          qp->Request(son, elem2);
          localinfo->attrs.push_back( ((CcInt*)elem2.addr)->GetIntval()-1 );
        }

        local = SetWord(localinfo);
      }
      else
      {
        local = SetWord(Address(0));
      }
      return 0;
    }
    case REQUEST:
    {
      if( local.addr == 0 )
        return CANCEL;

      //1. recover local information
      localinfo=(ProjectExtendStreamLocalInfo *) local.addr;
      resultTupleType = (TupleType *)localinfo->resultTupleType;

      //2. prepare tupleX and wValueY. If wValueY is empty, then get next tupleX
      wValueY = SetWord(Address(0));
      while( wValueY.addr == 0 )
      {
        qp->Request( localinfo->streamY.addr, wValueY );
        if( !(qp->Received( localinfo->streamY.addr )) )
        {
          qp->Close( localinfo->streamY.addr );
          localinfo->tupleX->DeleteIfAllowed();
          qp->Request( args[0].addr, wTupleX );
          if( qp->Received(args[0].addr) )
          {
            supplier = args[2].addr;
            supplier2 = qp->GetSupplier(supplier, 0);
            supplier3 = qp->GetSupplier(supplier2, 1);
            funargs = qp->Argument(supplier3);

            localinfo->tupleX = (Tuple*)wTupleX.addr;
            (*funargs)[0] = wTupleX;
            qp->Open( supplier3 );

            localinfo->tupleX = (Tuple*)wTupleX.addr;
            localinfo->streamY = SetWord(supplier3);

            wValueY = SetWord(Address(0));
          }
          else  //streamx is exausted
          {
            localinfo->streamY = SetWord(0);
            localinfo->tupleX = 0;
            return CANCEL;
          }
        }
      }

      //3. compute tupleXY from tupleX and wValueY
      tupleXY = new Tuple( *localinfo->resultTupleType, true );
      assert( tupleXY->IsFree() == true );

      size_t i;
      for( i = 0; i < localinfo->attrs.size(); i++ )
        tupleXY->PutAttribute( i, localinfo->tupleX->GetAttribute( localinfo->attrs[i] )->Clone() );

      assert( i == (size_t)tupleXY->GetNoAttributes()-1 );
      tupleXY->PutAttribute( i, ((StandardAttribute*)wValueY.addr)->Clone() );

      // deleting wValueY
      const AttributeType& yAttributeType =
        localinfo->resultTupleType->GetAttributeType( localinfo->resultTupleType->GetNoAttributes() - 1 );
      (SecondoSystem::GetAlgebraManager()->DeleteObj( yAttributeType.algId, yAttributeType.typeId ))( wValueY );

      // setting the result
      result = SetWord( tupleXY );
      return YIELD;
    }
    case CLOSE:
    {
      if( local.addr != 0 )
      {
        localinfo = (ProjectExtendStreamLocalInfo *)local.addr;

        if( localinfo->streamY.addr != 0 )
          qp->Close( localinfo->streamY.addr );

        if( localinfo->tupleX != 0 )
          localinfo->tupleX->DeleteIfAllowed();

        delete localinfo->resultTupleType;
        delete localinfo;
      }
      qp->Close( args[0].addr );
      return 0;
    }
  }
  return 0;
}

/*
2.19.3 Specification of operator ~projectextendstream~

*/
const string ProjectExtendStreamSpec = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                             "\"Example\" ) "
                             "( <text>((stream tuple1) (ai1 ... aik) (map tuple1 "
                             "stream(type))) -> (stream tuple1[ai1 ... aik]*type)"
                             "</text--->"
                             "<text>_ projectextendstream [ list; funlist ]</text--->"
                             "<text>This operator does the same as the extendstream does,"
                             " projecting the result stream of tuples to some specified"
                             " list of attribute names.</text--->"
                             "<text>query UBahn feed projectextendstream"
                             "[ Id, Up; newattr:  units(.Trajectory)] consume</text--->"
                             ") )";

/*
2.19.4 Definition of operator ~projectextendstream~

*/
Operator extrelprojectextendstream (
         "projectextendstream",                // name
         ProjectExtendStreamSpec,              // specification
         ProjectExtendStream,                  // value mapping
         Operator::DummyModel,                 // dummy model mapping, defines in Algebra.h
         Operator::SimpleSelect,               // trivial selection function
         ProjectExtendStreamTypeMap            // type mapping
);

/*
2.20 Operator ~concat~

2.20.1 Type mapping function of operator ~concat~

Type mapping for ~concat~ is

----    ((stream (tuple (a1:d1 ... an:dn))) (stream (tuple (b1:d1 ... bn:dn))))

        -> (stream (tuple (a1:d1 ... an:dn)))
----

*/
ListExpr GetAttrTypeList (ListExpr l)
{
  ListExpr first = nl->TheEmptyList();
  ListExpr olist = first, 
           lastolist = first;
           
  ListExpr attrlist = l;
  while (!nl->IsEmpty(attrlist))
  {
    first = nl->First(attrlist);
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

ListExpr ConcatTypeMap( ListExpr args )
{
  ListExpr first, second;
  string argstr, argstr2;

  CHECK_COND(nl->ListLength(args) == 2,
    "Operator concat expects a list of length two.");

  first = nl->First(args);
  second  = nl->Second(args);

  nl->WriteToString(argstr, first);
  CHECK_COND(nl->ListLength(first) == 2  &&
             (TypeOfRelAlgSymbol(nl->First(first)) == stream) &&
             (nl->ListLength(nl->Second(first)) == 2) &&
             (TypeOfRelAlgSymbol(nl->First(nl->Second(first))) == tuple) &&
	     (nl->ListLength(nl->Second(first)) == 2) &&
	     (IsTupleDescription(nl->Second(nl->Second(first)))),
    "Operator concat expects as first argument a list with structure "
    "(stream (tuple ((a1 t1)...(an tn))))\n"
    "Operator concat gets as first argument '" + argstr + "'." );

  nl->WriteToString(argstr, second);
  CHECK_COND(nl->ListLength(second) == 2  &&
             (TypeOfRelAlgSymbol(nl->First(first)) == stream) &&
             (nl->ListLength(nl->Second(second)) == 2) &&
             (TypeOfRelAlgSymbol(nl->First(nl->Second(second))) == tuple) &&
	     (nl->ListLength(nl->Second(second)) == 2) &&
	     (IsTupleDescription(nl->Second(nl->Second(second)))),
    "Operator concat expects as second argument a list with structure "
    "(stream (tuple ((a1 t1)...(an tn))))\n"
    "Operator concat gets as second argument '" + argstr + "'." );

  nl->WriteToString(argstr, GetAttrTypeList(nl->Second(nl->Second(first))));
  nl->WriteToString(argstr2, GetAttrTypeList(nl->Second(nl->Second(second))));
  CHECK_COND((nl->Equal(GetAttrTypeList(nl->Second(nl->Second(first))),
               GetAttrTypeList(nl->Second(nl->Second(second))))),
    "Operator concat: Tuple type of first and second argument "
    "stream must be the same.\n"
    "Tuple type of first stream is '" + argstr + "'.\n"
    "Tuple type of second stream is '" + argstr2 + "'.\n" );

  return first;
}
/*
2.20.2 Value mapping function of operator ~concat~

*/
int Concat(Word* args, Word& result, int message, Word& local, Supplier s)
{
  Word t;
  Tuple* tuple;

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
          tuple = (Tuple*)t.addr;
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
        tuple = (Tuple*)t.addr;
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
2.20.3 Specification of operator ~concat~

*/
const string ConcatSpec  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                           "\"Example\" ) "
                           "( <text>((stream (tuple (a1:d1 ... an:dn))) "
                           "(stream (tuple (b1:d1 ... bn:dn)))) -> (stream"
                           " (tuple (a1:d1 ... an:dn)))</text--->"
                           "<text>_ _ concat</text--->"
                           "<text>Union.</text--->"
                           "<text>query ten feed five feed concat consume"
                           "</text--->"
                           ") )";

/*
2.20.4 Definition of operator ~concat~

*/
Operator extrelconcat (
         "concat",               // name
         ConcatSpec,             // specification
         Concat,                 // value mapping
         Operator::DummyModel,   // dummy model mapping, defines in Algebra.h
         Operator::SimpleSelect, // trivial selection function
         ConcatTypeMap           // type mapping
);

/*
2.21 Operator ~groupby~

2.21.1 Type mapping function of operator ~groupby~

Result type of ~groupby~ operation.

----   Let X = tuple ((x1 t1) ... (xn tn)), R = rel(X):

       ( (stream X) (xi1 ... xik) ( (y1 (map R T1)) ... (ym (map R Tm)) )

        -> ( APPEND (m p1 ... pm) (stream (tuple (xj1 tj1)... (xjl tjl) (y1 T1) ... (ym Tm))))

       with tj,Ti in kind DATA, xi <> xj and k+l=n, pi <> pj and 1 <= pi <= m. 
       This means attributes xi ... xik are removed from the stream and attributes 
       y1 ... ym are appended. These new attributes represent aggregated values computed 
       by maps of R -> Ti which must have a result type of kind DATA.
----

*/
ListExpr GroupByTypeMap2(ListExpr args, const bool memoryImpl = false )
{
  ListExpr first, second, third;     // list used for analysing input
	ListExpr listn, lastlistn, listp;  // list used for constructing output

  first = second = third = nl->TheEmptyList();
	listn = lastlistn = listp = nl->TheEmptyList();

  string relSymbolStr = "rel";
  string tupleSymbolStr = "tuple";

  if ( memoryImpl ) {
    relSymbolStr = "mrel";
    tupleSymbolStr = "mtuple";
  }

  bool listOk = true;
  listOk = listOk && ( nl->ListLength(args) == 3 );

  if ( listOk ) {

    first  = nl->First(args);
    second = nl->Second(args);
    third  = nl->Third(args);

    // check input list structure
    listOk = listOk && (nl->ListLength(first) == 2);
		listOk = listOk && !nl->IsEmpty( third );
		listOk = listOk && !nl->IsAtom(second) && ( nl->ListLength(second) > 0 );

	}

	if( !listOk )
  {
	  stringstream errMsg;
		errMsg << "groupby: Invalid input list structure. "
			     << "The structure should be a three elem list "
					 << "like (stream (" << tupleSymbolStr
					 << "((x1 t1) ... (xn tn)) (xi1 ... xik) "
					 << "( (y1 (map R T1)) ... (ym (map R Tm))!";

    ErrorReporter::ReportError(errMsg.str());
    return nl->SymbolAtom("typeerror");
  }

  ListExpr tuple = nl->First(nl->Second(first));

  listOk = listOk && ( TypeOfRelAlgSymbol(nl->First(first)) == stream );
  listOk = listOk && ( nl->AtomType(tuple) == SymbolType );
  listOk = listOk && ( nl->SymbolValue(tuple) == tupleSymbolStr );

	if ( !listOk ) {

	  ErrorReporter::ReportError( "groupby: Input is not of type (stream "
	                              + tupleSymbolStr + "(...))." );
    return nl->SymbolAtom("typeerror");
	}

	// list seems to be ok. Extract the grouping attributes
	// out of the input stream

	ListExpr rest = second;
        ListExpr lastlistp = nl->TheEmptyList();
	bool firstcall = true;

	while (!nl->IsEmpty(rest))
	{
	  ListExpr attrtype = nl->TheEmptyList();
		ListExpr first2 = nl->First(rest);
		string attrname = nl->SymbolValue(first2);

		// calculate index of attribute in tuple
		int j = FindAttribute(nl->Second(nl->Second(first)), attrname, attrtype);
		if (j)
		{
			if (!firstcall)
			{
				lastlistn  = nl->Append(lastlistn,nl->TwoElemList(first2,attrtype));
				lastlistp = nl->Append(lastlistp,nl->IntAtom(j));
			}
			else
			{
				firstcall = false;
				listn = nl->OneElemList(nl->TwoElemList(first2,attrtype));
				lastlistn = listn;
				listp = nl->OneElemList(nl->IntAtom(j));
				lastlistp = listp;
			}
		}
		else // grouping attribute not in input stream
		{
			string errMsg = "groupby: Attribute " + attrname
				        + " not present in input stream!";

			ErrorReporter::ReportError(errMsg);
			return nl->SymbolAtom("typeerror");
		}
		rest = nl->Rest(rest);
	} // end while



		// compute output tuple with attribute names and their types
		//loopok = true;
		rest = third;
		ListExpr groupType = nl->TwoElemList( nl->SymbolAtom(relSymbolStr),
																          nl->Second(first) );

		while (!(nl->IsEmpty(rest))) // check functions y1 .. ym
		{
  		// iterate over elements of the 3rd input list
			ListExpr firstr = nl->First(rest);
			rest = nl->Rest(rest);

			ListExpr newAttr = nl->First(firstr);
			ListExpr mapDef = nl->Second(firstr); // (map <InTypeExpr> <OutTypeExpr>)
			ListExpr mapOut = nl->Third(mapDef);

			// check list structure
			bool listOk = true;
			listOk = listOk && ( nl->IsAtom(newAttr) );
			listOk = listOk && ( nl->ListLength(mapDef) == 3 );
			listOk = listOk && ( nl->AtomType(newAttr) == SymbolType );
			listOk = listOk && ( TypeOfRelAlgSymbol(nl->First(mapDef)) == ccmap );
			listOk = listOk && ( nl->Equal(groupType, nl->Second(mapDef)) );

			if( !listOk ) { // Todo: there could be more fine grained error messages

				ErrorReporter::ReportError("groupby: Function definition is not correct!");
				return nl->SymbolAtom("typeerror");
			}

			// Check if mapOut is of kind DATA or if the function returns a typeerror

			ListExpr typeConstructor = nl->TheEmptyList();
			if ( nl->IsAtom(mapOut) ) // function returns a simple type
			{
				assert( nl->AtomType(mapOut) == SymbolType );
				typeConstructor = mapOut;

			} else { // function returns a complex type

				typeConstructor = nl->First(mapOut);
				assert( nl->AtomType(typeConstructor) == SymbolType );
			}

			// check if the Type Constructor belongs to KIND DATA
			// If the functions result type is typeerror this check will also fail
			ListExpr errorInfo = nl->OneElemList(nl->SymbolAtom("ErrorInfo"));
			AlgebraManager* algMgr = SecondoSystem::GetAlgebraManager();

			if ( !algMgr->CheckKind("DATA", typeConstructor, errorInfo) ) {

				stringstream errMsg;
				errMsg << "groupby: The aggregate function for attribute \""
							<< nl->SymbolValue(newAttr) << "\""
							<< " returns a type which is not usable in tuples."
							<< " The type constructor \""
							<< nl->SymbolValue(typeConstructor) << "\""
							<< " belongs not to kind DATA!"
							<< ends;

				ErrorReporter::ReportError(errMsg.str());
				return nl->SymbolAtom("typeerror");

			}

			lastlistn = nl->Append(lastlistn,(nl->TwoElemList(newAttr,mapOut)));

	} // end of while check functions


	if ( !CompareNames(listn) ) { // check if attribute names are uniqe

		ErrorReporter::ReportError("groupby: Attribute names are not unique");
		return nl->SymbolAtom("typeerror");
	}

	// Type mapping is correct, return result type.
	return
			nl->ThreeElemList(
				nl->SymbolAtom("APPEND"),
				nl->Cons(nl->IntAtom(nl->ListLength(listp)), listp),
				nl->TwoElemList(
					nl->SymbolAtom("stream"),
					nl->TwoElemList(
						nl->SymbolAtom(tupleSymbolStr),
						listn)));

}


ListExpr GroupByTypeMap(ListExpr args)
{
  return GroupByTypeMap2(args);
}

/*
2.21.2 Value mapping function of operator ~groupby~

*/
struct GroupByLocalInfo
{
  Tuple *t;
  TupleType *resultTupleType;
};

int GroupByValueMapping
(Word* args, Word& result, int message, Word& local, Supplier supplier)
{
  Tuple *t = 0;
  Tuple *s = 0;
  Word sWord =SetWord(Address(0));
  TupleBuffer* tp = 0;
  TupleBufferIterator* relIter = 0;
  int i = 0, j = 0, k = 0;
  int numberatt = 0;
  bool ifequal = false;
  Word value = SetWord(Address(0));
  Supplier  value2;
  Supplier supplier1;
  Supplier supplier2;
  int noOffun = 0;
  ArgVectorPointer vector;
  const int indexOfCountArgument = 3;
  const int startIndexOfExtraArguments = indexOfCountArgument +1;
  int attribIdx = 0;
  Word nAttributesWord = SetWord(Address(0));
  Word attribIdxWord = SetWord(Address(0));
  GroupByLocalInfo *gbli = 0;

  // The argument vector contains the following values:
  // args[0] = stream of tuples 
  // args[1] = list of identifiers 
  // args[2] = list of functions
  // args[3] = Number of extra arguments 
  // args[4 ...] = args added by APPEND

  switch(message)
  {
    case OPEN: 
      // Get the first tuple pointer and store it in the
      // GroupBylocalInfo structure
      qp->Open (args[0].addr);
      qp->Request(args[0].addr, sWord);
      if (qp->Received(args[0].addr))
      {
        gbli = new GroupByLocalInfo;
        gbli->t = (Tuple*)sWord.addr;
        ListExpr resultType = GetTupleResultType( supplier );
        gbli->resultTupleType = new TupleType( nl->Second( resultType ) );
        local = SetWord(gbli);
      }
      else
      {
        local = SetWord(0);
      }
      return 0;

    case REQUEST:
      Counter::getRef("GroupBy:Request")++;
      if(local.addr == 0) // should not happen
        return CANCEL;
      else
      {
        gbli = (GroupByLocalInfo *)local.addr;
        if( gbli->t == 0 ) // Stream ends
          return CANCEL;
         
        //cout << *(gbli->t) << endl;
        t = gbli->t->Clone( true );
        Tuple* copyt = t->Clone( true );
        gbli->t->DeleteIfAllowed();
        gbli->t = 0;
        tp = new TupleBuffer();
        tp->AppendTuple(copyt);
        copyt->DeleteIfAllowed();
      }
      // get number of attributes
      qp->Request(args[indexOfCountArgument].addr, nAttributesWord);
      numberatt = ((CcInt*)nAttributesWord.addr)->GetIntval();

      ifequal = true;

      // Get next tuple
      qp->Request(args[0].addr, sWord);
      //Tuple cmpTup = (Tuple*)sWord.addr;
      //cmpTup->SetFree(false);
      while ((qp->Received(args[0].addr)) && ifequal)
      {
        s = (Tuple*)sWord.addr;
        for (k = 0; k < numberatt; k++) // check if  tuples t = s
        {
          // loop over all grouping attributes
          qp->Request(args[startIndexOfExtraArguments+k].addr, attribIdxWord);
          attribIdx = ((CcInt*)attribIdxWord.addr)->GetIntval();
          j = attribIdx - 1;
          if (((Attribute*)t->GetAttribute(j))->Compare((Attribute *)s->GetAttribute(j)))
            ifequal = false;
        }
        if (ifequal) // store in tuple buffer
        {
          Tuple *auxS = s;
          s = auxS->Clone(true);
          auxS->DeleteIfAllowed();
          tp->AppendTuple( s );
          qp->Request(args[0].addr, sWord); // get next tuple
        }
        else
          gbli->t = (Tuple *)sWord.addr; // store tuple pointer in local info
      }
      if (ifequal) //  last group finished, stream ends
      {
        gbli->t = 0;
      }

      // create result tuple
      t = new Tuple( *gbli->resultTupleType, true );
      assert( t->IsFree() == true );
      relIter = tp->MakeScan();
      s = relIter->GetNextTuple();

      // copy in grouping attributes
      for(i = 0; i < numberatt; i++)
      {
        qp->Request(args[startIndexOfExtraArguments+i].addr, attribIdxWord);
        attribIdx = ((CcInt*)attribIdxWord.addr)->GetIntval();
        t->PutAttribute(i, ((Attribute*)s->GetAttribute(attribIdx - 1))->Clone());
      }
      value2 = (Supplier)args[2].addr; // list of functions 
      noOffun  =  qp->GetNoSons(value2);
      assert( t->GetNoAttributes() == numberatt + noOffun );
      delete relIter;

      for(i = 0; i < noOffun; i++)
      {
        // prepare arguments for function i
        supplier1 = qp->GetSupplier(value2, i);
        supplier2 = qp->GetSupplier(supplier1, 1);
        vector = qp->Argument(supplier2);
        // The group was stored in a relation identified by symbol group 
        // which is a typemap operator. Here it is stored in the argument vector
        (*vector)[0] = SetWord(tp);
        
        // compute value of function i and put it into the result tuple
        qp->Request(supplier2, value); 
        t->PutAttribute(numberatt + i, ((Attribute*)value.addr)->Clone()) ;
      }
      result = SetWord(t);
//      tp->Clear();
      delete tp;
      return YIELD;

    case CLOSE:
      if( local.addr != 0 )
      {
        gbli = (GroupByLocalInfo *)local.addr;
        delete gbli->resultTupleType;
        delete gbli;
      }
      qp->Close(args[0].addr);
      return 0;
  }
  return 0;
}

/*
2.21.3 Specification of operator ~groupby~

*/
const string GroupBySpec  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                            "\"Example\" ) "
                            "( <text>((stream (tuple (a1:d1 ... an:dn))) "
                            "(ai1 ... aik) ((bj1 (fun (rel (tuple (a1:d1"
                            " ... an:dn))) (_))) ... (bjl (fun (rel (tuple"
                            " (a1:d1 ... an:dn))) (_))))) -> (stream (tuple"
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
2.21.4 Definition of operator ~groupby~

*/
Operator extrelgroupby (
         "groupby",             // name
         GroupBySpec,           // specification
         GroupByValueMapping,   // value mapping
         Operator::DummyModel,  // dummy model mapping, defines in Algebra.h
         Operator::SimpleSelect,          // trivial selection function
         GroupByTypeMap         // type mapping
);

/*

3 Class ~ExtRelationAlgebra~

A new subclass ~ExtRelationAlgebra~ of class ~Algebra~ is declared. The only
specialization with respect to class ~Algebra~ takes place within the
constructor: all type constructors and operators are registered at the actual algebra.

After declaring the new class, its only instance ~extendedRelationAlgebra~ is defined.

*/

class ExtRelationAlgebra : public Algebra
{
 public:
  ExtRelationAlgebra() : Algebra()
  {
    AddOperator(&extrelsample);
    AddOperator(&extrelgroup);
    AddOperator(&extrelremoval);
    AddOperator(&extrelcancel);
    AddOperator(&extrelextract);
    AddOperator(&extrelextend);
    AddOperator(&extrelconcat);
    AddOperator(&extrelmin);
    AddOperator(&extrelmax);
    AddOperator(&extrelavg);
    AddOperator(&extrelsum);
    AddOperator(&extrelhead);
    AddOperator(&extrelsortby);
    AddOperator(&extrelsort);
    AddOperator(&extrelrdup);
    AddOperator(&extrelmergesec);
    AddOperator(&extrelmergediff);
    AddOperator(&extrelmergeunion);
    AddOperator(&extrelmergejoin);
    AddOperator(&extrelsortmergejoin);
    AddOperator(&extreloldhashjoin);
    AddOperator(&extrelhashjoin);
    AddOperator(&extrelloopjoin);
    AddOperator(&extrelextendstream);
    AddOperator(&extrelprojectextendstream);
    AddOperator(&extrelloopsel);
    AddOperator(&extrelgroupby);
  }
  ~ExtRelationAlgebra() {};
};

ExtRelationAlgebra extRelationalgebra;

/*

4 Initialization

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
InitializeExtRelationAlgebra( NestedList* nlRef, QueryProcessor* qpRef )
{
  nl = nlRef;
  qp = qpRef;
  return (&extRelationalgebra);
}

