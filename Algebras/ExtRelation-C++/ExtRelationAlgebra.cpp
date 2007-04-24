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

December 2005, Victor Almeida deleted the deprecated algebra levels
(~executable~, ~descriptive~, and ~hibrid~). Only the executable
level remains. Models are also removed from type constructors.

January 2006 Victor Almeida replaced the ~free~ tuples concept to
reference counters. There are reference counters on tuples and also
on attributes. Some assertions were removed, since the code is
stable.

June 2006, Corrected a bug caused by improper reference counting of tuples observed in  
operator ~mergesec~.

June 2006, Christian D[ue]ntgen added operators ~symmproduct~ and ~symmproductextend~.

August 2006, Christian D[ue]ntgen added signature ((stream T) int) -> (stream T) to operator ~head~.

January 2007, M. Spiekermann. Reference counting in groupby corrected, since it causes a segmentation fault, 
when the Tuplebuffer needs to be flushed on disk.

[TOC]

1 Includes and defines

*/

#include <vector>
#include <deque>
#include <sstream>
#include <stack>

#include "RelationAlgebra.h"
#include "QueryProcessor.h"
#include "AlgebraManager.h"
#include "CPUTimeMeasurer.h"
#include "StandardTypes.h"
#include "Counter.h"
#include "TupleIdentifier.h"
#include "LogMsg.h"

extern NestedList* nl;
extern QueryProcessor* qp;
extern AlgebraManager* am;

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
    "Type operator group expects a list with minimal length one.");

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
  set<int> drawnNumbers;
  set<int>::iterator iter;
  int drawSize = 0;
  int nDrawn = 0;
  int i = 0;
  int r = 0;
  bool doInvert = false;

  result.resize(0);

  // The variable below defines an offset into the
  // random number sequence. It will be incremented by
  // the number of rand() calls. Hence subsequent calls
  // will avoid to return the same sequence of numbers.
  static unsigned int randCalls = (time(0) % 1000) * 1000;
  srand(randCalls);

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

  // Using Windows RAND_MAX is very small (about 2^15) therefore we 
  // need to limit the drawSize to 3/4 (to avoid long runtimes) of 
  // this size.
  int drawMax = 3*RAND_MAX/4;
  const int randMax = RAND_MAX;
  const int intMax = INT_MAX;
  const int newMax = max(randMax, intMax);
  static const int f = (intMax / randMax) - 1;
  static long& ctr = Counter::getRef("EXT::sample:randPos");
  static long& ctrMax = Counter::getRef("EXT::sample:maxDrawnRand");
  
   
  if ( drawSize > drawMax ) 
  {
    drawSize = drawMax;
    cerr << "Warning: Sample size reduced to 3/4*RAND_MAX." << endl;
  }

  TRACE("*** sample parameters ***")
  SHOW(f)
  SHOW(setSize)
  SHOW(subsetSize)
  SHOW(drawSize)
  
  while(nDrawn < drawSize)
  {
    // 28.04.04 M. Spiekermann.
    // The calculation of random numbers below is recommended in 
    // the man page documentation of the rand() function.
    // Moreover, the factor f is used to retrieve values in the
    // range of LONG_MAX
    long nextRand = rand();
    randCalls++;
    if ( f > 1 )
    {
      nextRand += (f * rand());
      randCalls++;
    } 
    if (nextRand > ctrMax)
      ctrMax = nextRand;
    
    r = (int) ((double)(setSize + 1) * nextRand/(newMax+1.0));
    
    if ( r != 0) {
      if(drawnNumbers.find(r) == drawnNumbers.end())
      {
        drawnNumbers.insert(r);
        ++nDrawn;
      }
    }  
  }
  ctr = randCalls;
  SHOW(drawnNumbers.size())

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
    for(iter = drawnNumbers.begin(); 
        iter != drawnNumbers.end(); 
        ++iter)
      result.push_back(*iter);
  }
  SHOW(result.size())
}

/*
2.4.2 Type mapping function of operator ~sample~

A type mapping function takes a nested list as argument. Its contents are
type descriptions of an operator's input parameters. A nested list describing
the output type of the operator is returned.

Result type of feed operation.

----  ((rel x) int real)    -> (stream x)
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
  RandomRelationIterator* relIter;
};

int Sample(Word* args, Word& result, int message, Word& local, Supplier s)
{
  SampleLocalInfo* localInfo = static_cast<SampleLocalInfo*>( local.addr );
  Word argRelation = SetWord(0);

  Relation* rel = 0;
  Tuple* tuple = 0;

  int sampleSize = 0;
  int relSize = 0;
  float sampleRate = 0;
  int i = 0;
  int currentIndex = 0;

  switch(message)
  {
    case OPEN :
      localInfo = new SampleLocalInfo();
      local.addr = localInfo;

      rel = (Relation*)args[0].addr;
      relSize = rel->GetNoTuples();
      localInfo->relIter = rel->MakeRandomScan();
      
      sampleSize = StdTypes::GetInt(args[1]);
      sampleRate = StdTypes::GetReal(args[2]);

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
      if(localInfo->iter == localInfo->sampleIndices.end())
      {
        return CANCEL;
      }
      else
      {
        currentIndex = *(localInfo->iter);
        int step = currentIndex - localInfo->lastIndex;
        if(!(tuple = localInfo->relIter->GetNextTuple(step)))
        {
          return CANCEL;
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
      ErrorReporter::ReportError(
        "Attributename '" + attrname + "' is not known.\n"
        "Known Attribute(s): " + argstr);
      return nl->SymbolAtom("typeerror");
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
int Remove(Word* args, Word& result, int message, 
           Word& local, Supplier s)
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
      Word elem1, elem2;
      int noOfAttrs, index;
      Supplier son;

      qp->Request(args[0].addr, elem1);
      if (qp->Received(args[0].addr))
      {
        TupleType *tupleType = (TupleType *)local.addr;
        Tuple *t = new Tuple( tupleType );

        noOfAttrs = ((CcInt*)args[2].addr)->GetIntval();
        for (int i=1; i <= noOfAttrs; i++)
        {
          son = qp->GetSupplier(args[3].addr, i-1);
          qp->Request(son, elem2);
          index = ((CcInt*)elem2.addr)->GetIntval();
          t->CopyAttribute(index-1, (Tuple*)elem1.addr, i-1);
        }
        ((Tuple*)elem1.addr)->DeleteIfAllowed();
        result = SetWord(t);
        return YIELD;
      }
      else return CANCEL;
    }
    case CLOSE :
    {
      ((TupleType*)local.addr)->DeleteIfAllowed();
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
    ErrorReporter::ReportError(
      "Attributename '" + attrname + "' is not known.\n"
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
    res->CopyFrom(
      (const StandardAttribute*)tupleptr->GetAttribute(index - 1));
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
         Operator::SimpleSelect, // trivial selection function
         ExtractTypeMap          // type mapping
);

/*
2.8 Operator ~head~

This operator fetches the first n elements (e.g. tuples) from a stream.

2.8.1 Type mapping function of operator ~head~

Type mapping for ~head~ is

----  ((stream (tuple ((x1 t1)...(xn tn))) int)   ->
              ((stream (tuple ((x1 t1)...(xn tn))))

  or ((stream T) int) -> (stream T)  for T in kind DATA 
----

*/
ListExpr HeadTypeMap( ListExpr args )
{
  ListExpr first, second, errorInfo;
  string argstr;

  errorInfo = nl->OneElemList(nl->SymbolAtom("ERROR"));

  CHECK_COND(nl->ListLength(args) == 2,
  "Operator head expects a list of length two.");

  first = nl->First(args);
  second = nl->Second(args);

  // check for first arg == stream of something
  nl->WriteToString(argstr, first);
  CHECK_COND( ( nl->ListLength(first) == 2 ) &&
              ( TypeOfRelAlgSymbol( nl->First(first) ) == stream ), 
    "Operator head expects as first argument a list with structure "
    "(stream (tuple ((a1 t1)...(an tn)))) or "
    "(stream T), where T in kind DATA.\n"
    "Operator head gets as first argument '" + argstr + "'." );

  // check for second argument type == int
  nl->WriteToString(argstr, second);
  CHECK_COND((nl->IsAtom(second)) &&
             (nl->AtomType(second) == SymbolType) &&
       (nl->SymbolValue(second) == "int"),
    "Operator head expects a second argument of type integer.\n"
    "Operator head gets '" + argstr + "'.");


  // check for correct stream input type
  nl->WriteToString(argstr, first);
  CHECK_COND( 
    ( ( nl->ListLength( nl->Second(first) ) == 2) &&
      ( TypeOfRelAlgSymbol(nl->First(nl->Second(first))) == tuple)) ||
    ( (nl->IsAtom(nl->Second(first))) &&
      (am->CheckKind("DATA", nl->Second(first), errorInfo))),
    "Operator head expects as first argument a list with structure "
    "(stream (tuple ((a1 t1)...(an tn)))) or "
    "(stream T), where T in kind DATA.\n"
    "Operator head gets as first argument '" + argstr + "'." );

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
  Word tupleWord;

  switch(message)
  {
    case OPEN:

      qp->Open(args[0].addr);
      localInfo = 
        new HeadLocalInfo( ((CcInt*)args[1].addr)->GetIntval() );
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
                         "( "
                         "<text>((stream (tuple([a1:d1, ... ,an:dn]"
                         "))) x int) -> (stream (tuple([a1:d1, ... ,"
                         "an:dn]))) or \n"
                         "((stream T) int) -> (stream T), "
                         "for T in kind DATA.</text--->"
                         "<text>_ head [ _ ]</text--->"
                         "<text>Returns the first n elements of the input "
                         "stream.</text--->"
                         "<text>query cities feed head[10] consume\n"
                         "query intstream(1,1000) head[10] printstream count"
                         "</text--->"
                              ") )";

/*
2.8.5 Definition of operator ~head~

*/
Operator extrelhead (
         "head",                 // name
         HeadSpec,               // specification
         Head,                   // value mapping
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
MaxMinValueMapping(Word* args, Word& result, int message, 
                   Word& local, Supplier s)
{
  bool definedValueFound = false;
  Word currentTupleWord;
  StandardAttribute* extremum = 
    (StandardAttribute*)(qp->ResultStorage(s)).addr;
  extremum->SetDefined(false);
  result = SetWord(extremum);

  int attributeIndex = ((CcInt*)args[2].addr)->GetIntval() - 1;

  qp->Open(args[0].addr);
  qp->Request(args[0].addr, currentTupleWord);
  while(qp->Received(args[0].addr))
  {
    Tuple* currentTuple = (Tuple*)currentTupleWord.addr;
    const StandardAttribute* currentAttr =
      (const StandardAttribute*)currentTuple->GetAttribute(attributeIndex);
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
AvgSumValueMapping(Word* args, Word& result, int message, 
                   Word& local, Supplier s)
{
  bool definedValueFound = false;
  Word currentTupleWord;
  Attribute* accumulated = 0;
  int nProcessedItems = 0;

  int attributeIndex = ((CcInt*)args[2].addr)->GetIntval() - 1;
  const STRING *attributeType = ((CcString*)args[3].addr)->GetStringval();

  qp->Open(args[0].addr);
  qp->Request(args[0].addr, currentTupleWord);
  while(qp->Received(args[0].addr))
  {
    Tuple* currentTuple = (Tuple*)currentTupleWord.addr;
    Attribute* currentAttr = 
      (Attribute*)currentTuple->GetAttribute(attributeIndex);
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
      accumulated->DeleteIfAllowed();
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
      accumulated->DeleteIfAllowed();
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
         Operator::SimpleSelect,          // trivial selection function
         AvgSumTypeMap<false>         // type mapping
);


/*
2.10 Operator ~krdup~

This operator removes duplicates from a tuple stream. In contrast
to the usual rdup operator, the test of equality of tuples is 
done using only the attributes from the given attribute list.

2.10.1 Type mapping of the operator ~krdup~

*/

ListExpr krdupTM(ListExpr args){
  if(nl->ListLength(args)<2){
    ErrorReporter::ReportError("minimum of 2 arguments required");
    return nl->TypeError(); 
  }

  ListExpr streamList = nl->First(args);
  ListExpr attrnameList = nl->Second(args);
  if(nl->ListLength(attrnameList)<1){
     ErrorReporter::ReportError("invalid number of attribute names");
     return nl->TypeError();
  }

  // extract the attribute names
  set<string> names;
  int pos = 0;
  while(!nl->IsEmpty(attrnameList)){
      ListExpr attr = nl->First(attrnameList);
      pos++;
      attrnameList = nl->Rest(attrnameList);
      if(!nl->AtomType(attr)==SymbolType){
        ErrorReporter::ReportError("argument is not "
                                   " an attribute name");
        return nl->TypeError();
      }
      string n = nl->SymbolValue(attr);
      // insert the name into the set of names
      names.insert(nl->SymbolValue(attr));
  }

  // check the stream argument
  if(nl->ListLength(streamList)!=2 ||
     !nl->IsEqual(nl->First(streamList),"stream")){
     ErrorReporter::ReportError("(stream (tuple(...)) "
                                " expected as first argument");
     return nl->TypeError();
  }

  ListExpr tupleList = nl->Second(streamList);
  if(nl->ListLength(tupleList)!=2 ||
     !nl->IsEqual(nl->First(tupleList),"tuple")){
     ErrorReporter::ReportError("(stream (tuple(...)) "
                                " expected as first argument");
     return nl->TypeError();
  }
  ListExpr attrList = nl->Second(tupleList);
 
  if(names.empty()){ // should never occur
    ErrorReporter::ReportError("internal error, no attr names found.");
    return nl->TypeError();
  }

  pos = 0;
  
  ListExpr PosList = nl->TheEmptyList();
  ListExpr Last;
  bool first = true;

  while(!nl->IsEmpty(attrList)){
     ListExpr pair = nl->First(attrList);
     if((nl->ListLength(pair)!= 2) ||
        (nl->AtomType(nl->First(pair))!=SymbolType)){
        ErrorReporter::ReportError("Error in tuple definition ");
        return nl->TypeError();
     }
     string name = nl->SymbolValue(nl->First(pair));
     if(names.find(name)!= names.end()){
         if(first){
           first = false;
           PosList = nl->OneElemList(nl->IntAtom(pos));
           Last = PosList;
           first = false;
         } else {
           Last = nl->Append(Last,nl->IntAtom(pos));
         }
         names.erase(name);
     } 
     pos++;
     attrList = nl->Rest(attrList);
  }

  if(!names.empty()){
    ErrorReporter::ReportError("at least one given attribute name is "
                               "not part of the stream");
    return nl->TypeError();
  }  

  
  ListExpr result =  nl->ThreeElemList(nl->SymbolAtom("APPEND"),
                                      nl->OneElemList(PosList),
                                      streamList);
  return result;
}

/*
2.10.2 Value Mapping of the ~krdup~ operator

This value mapping is very similar the this one of the 
rdup operator. The only difference is, only specified
attributes are used for the check of equality.

*/ 


class KrdupLocalInfo{
public:
/*
~Constructor~

Creates a localinfo for the krdup operator from the Supplier
containing the information about the indexes which should be used 
for the comparison.

*/
  KrdupLocalInfo(Supplier s):indexes(){
       lastTuple=0;
       int count = qp->GetNoSons(s);
       Word elem;
       for(int i=0;i<count; i++){
          Supplier son = qp->GetSon(s,i);
          qp->Request(son,elem); 
          int index = ((CcInt*)elem.addr)->GetIntval();
          indexes.push_back(index);
       } 
   }

/*
~Destructor~

Destroy this instance.

*/

   ~KrdupLocalInfo(){
       if(lastTuple!=0){
          lastTuple->DecReference();
          lastTuple->DeleteIfAllowed();
       }
       lastTuple = 0; 
   }

/*
~ReplaceIfNonEqual~

If the argument differs from the last stored tuple, the old tuple
is replaced by the argument and the return value is true. Otherwise,
this instance is keep unchanged and the reuslt of the call is false.
The old tuple is removed automatically.

*/
inline bool  ReplaceIfNonEqual(Tuple* newTuple){
   if(lastTuple==0){ // first call
      newTuple->IncReference();
      lastTuple = newTuple;
      return true;
   }

   if(Equals(newTuple)){
     return false;
   } else{ // replace the tuple
     lastTuple->DecReference();
     lastTuple->DeleteIfAllowed();
     newTuple->IncReference();
     lastTuple = newTuple;
     return true;
   }
}

private:
/*
~equals~

Checks the equality of the last tuple with the argument where only the
attributes stored in the vector are of interest.

*/
inline bool Equals(Tuple* tuple){
 int size = indexes.size();
 for(int i=0;i<size;i++){
   int pos = indexes[i];
   int cmp = ( ((Attribute*)lastTuple->GetAttribute(pos))->CompareAlmost(
               ((Attribute*)tuple->GetAttribute(pos))));
   if(cmp!=0){
       return false;
   }
 }
 return true;
}   


   Tuple* lastTuple;
   vector<int> indexes;
};


int KrdupVM(Word* args, Word& result, int message, 
                     Word& local, Supplier s)
{
  Word tuple;

  switch(message)
  {
    case OPEN:
      qp->Open(args[0].addr);
      local = SetWord(new KrdupLocalInfo(qp->GetSon(s,2)));
      return 0;
    case REQUEST:
      qp->Request(args[0].addr,tuple);
      if(qp->Received(args[0].addr)){
         KrdupLocalInfo* localinfo = (KrdupLocalInfo*) local.addr;
         while(!localinfo->ReplaceIfNonEqual((Tuple*)tuple.addr)){
            ((Tuple*)tuple.addr)->DeleteIfAllowed();
            qp->Request(args[0].addr,tuple);
            if(!qp->Received(args[0].addr)){
                return CANCEL;
            }
         }
         result.addr = tuple.addr;
         return YIELD;

      } else {
        return CANCEL;
      }
    case CLOSE:
      KrdupLocalInfo* localinfo = (KrdupLocalInfo*) local.addr;
      delete localinfo;
      local = SetWord(0); 
      qp->Close(args[0].addr);
      return 0;
  }
  return 0;
}


/*
2.13.2 Specification of operator ~krdup~

*/
const string KrdupSpec  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                         "\"Example\" ) "
                         "( <text>((stream (tuple([a1:d1, ... ,an:dn]"
                            " x ai1 .. ain))))"
                         " -> (stream (tuple([a1:d1, ... ,an:dn])))"
                         "</text--->"
                         "<text>_ krdup[ _ , _ , ...]</text--->"
                         "<text>Removes duplicates from a sorted "
                         "stream with respect to the attributes "
                         "given as arguments.</text--->"
                         "<text>query plz feed sortby [Ort] "
                         "krdup[Ort]  consume</text--->"
                              ") )";

/*
2.13.3 Definition of operator ~rdup~

*/
Operator krdup (
         "krdup",             // name
         KrdupSpec,           // specification
         KrdupVM,               // value mapping
         Operator::SimpleSelect,          // trivial selection function
         krdupTM         // type mapping
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

  ListExpr sortOrderDescription = 
    nl->OneElemList(nl->IntAtom(numberOfSortAttrs));
  ListExpr sortOrderDescriptionLastElement = sortOrderDescription;
  ListExpr rest = sortSpecification;
  while(!nl->IsEmpty(rest))
  {
    ListExpr attributeSpecification = nl->First(rest);
    rest = nl->Rest(rest);

    nl->WriteToString(argstr, attributeSpecification);

    int length = nl->ListLength(attributeSpecification);

    CHECK_COND( (nl->IsAtom(attributeSpecification)) || (length == 2),
                 "sortby expects as second argument a list"
                 " of (attrname [asc, desc])|attrname .");

    if(length==2)
    {    
       CHECK_COND((nl->IsAtom(nl->First(attributeSpecification))) &&
           (nl->AtomType(nl->First(attributeSpecification)) == SymbolType) &&
           (nl->IsAtom(nl->Second(attributeSpecification))) &&
           (nl->AtomType(nl->Second(attributeSpecification)) == SymbolType),
           "sortby expects as second argument a list"
           " of (attrname [asc, desc])|attrname .\n"
           "Operator sortby gets a list '" + argstr + "'.");
       attrname = nl->SymbolValue(nl->First(attributeSpecification));
    } else 
    {
         CHECK_COND((nl->AtomType(attributeSpecification) == SymbolType),
            "sortby expects as second argument a list"
            " of (attrname [asc, desc])|attrname .\n"
            "Operator sortby gets a list '" + argstr + "'.");
         attrname = nl->SymbolValue(attributeSpecification);
    }

    int j = FindAttribute(nl->Second(nl->Second(streamDescription)), 
                          attrname, attrtype);
    if (j > 0)
    {
      if(length==2)
      {
         nl->WriteToString(argstr, nl->Second(attributeSpecification));
         CHECK_COND( 
           ((nl->SymbolValue(nl->Second(attributeSpecification)) == 
             sortAscending) || 
            (nl->SymbolValue(nl->Second(attributeSpecification)) == 
             sortDescending)),
            "Operator sortby: sorting criteria must be asc or desc, not '" + 
            argstr + "'!" );
      }

      sortOrderDescriptionLastElement =
        nl->Append(sortOrderDescriptionLastElement, nl->IntAtom(j));
      bool isAscending=true;
      if(length==2)
      {
        isAscending =
          nl->SymbolValue(nl->Second(attributeSpecification)) == sortAscending;
      } else {
         isAscending = true;
      }
      sortOrderDescriptionLastElement =
        nl->Append(sortOrderDescriptionLastElement,
        nl->BoolAtom(isAscending));
    }
    else
    {
      nl->WriteToString( argstr, nl->Second(nl->Second(streamDescription)) );
      ErrorReporter::ReportError(
        "Operator sortby: Attributename '" + attrname +
        "' is not known.\nKnown Attribute(s): " + argstr);
      return nl->SymbolAtom("typeerror");
    }
  }
  return nl->ThreeElemList(nl->SymbolAtom("APPEND"),
    sortOrderDescription, streamDescription);
}
/*
2.11.2 Value mapping function of operator ~sortBy~

The argument vector ~args~ contains in the first slot ~args[0]~ the 
stream and in ~args[2]~ the number of sort attributes. ~args[3]~ 
contains the index of the first sort attribute, ~args[4]~ a boolean 
indicating wether the stream is sorted in ascending order with regard 
to the sort first attribute. ~args[5]~ and ~args[6]~ contain these 
values for the second sort attribute  and so on.

*/
template<bool lexicographically> int
SortBy(Word* args, Word& result, int message, Word& local, Supplier s);
/*

2.11.3 Specification of operator ~sortBy~

*/
const string SortBySpec  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                           "\"Example\" ) "
                           "( <text>((stream (tuple([a1:d1, ... ,an:dn])))"
                           " ((xi1 asc/desc) ... (xij [asc/desc]))) -> "
                           "(stream (tuple([a1:d1, ... ,an:dn])))</text--->"
                           "<text>_ sortby [list]</text--->"
                           "<text>Sorts input stream according to a list "
                           "of attributes ai1 ... aij. \n"
                           "If no order is specified, ascending is assumed."
                           "</text--->"
                           "<text>query employee feed sortby[DeptNo asc] "
                           "consume</text--->"
                              ") )";

/*
2.11.4 Definition of operator ~sortBy~

*/
Operator extrelsortby (
         "sortby",               // name
         SortBySpec,             // specification
         SortBy<false>,          // value mapping
         Operator::SimpleSelect, // trivial selection function
         SortByTypeMap           // type mapping
);

/*
2.12 Operator ~sort~

This operator sorts a stream of tuples lexicographically.

2.12.1 Type mapping function of operator ~sort~

Type mapping for ~sort~ is

----  ((stream (tuple ((x1 t1)...(xn tn))))
        -> (stream (tuple ((x1 t1)...(xn tn)))
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
         SortBy<true>,               // value mapping
         Operator::SimpleSelect,          // trivial selection function
         IdenticalTypeMap<true>         // type mapping
);

/*
2.13 Operator ~rdup~

This operator removes duplicates from a sorted stream.

2.13.1 Value mapping function of operator ~rdup~

*/
int RdupValueMapping(Word* args, Word& result, int message, 
                     Word& local, Supplier s)
{
  Word tuple;
  LexicographicalTupleCompareAlmost cmp;
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
          // stream deliverd a new tuple
          if(local.addr != 0)
          {
            // there is a last tuple
            currentTuple = (Tuple*)tuple.addr;
            lastOutputTuple = (Tuple*)local.addr;
            if(cmp(currentTuple, lastOutputTuple)
              || cmp(lastOutputTuple, currentTuple))
            {
              // tuples are not equal. Return the tuple
              // stored in local info and stor the current one
              // there.
              lastOutputTuple->DecReference();
              lastOutputTuple->DeleteIfAllowed();
              local = SetWord(currentTuple);
              currentTuple->IncReference();
              result = SetWord(currentTuple);
              return YIELD;
            }
            else
            {
              // tuples are equal
              currentTuple->DeleteIfAllowed();
            }
          }
          else
          {
            // no last tuple stored
            currentTuple = (Tuple*)tuple.addr;
            local = SetWord(currentTuple);
            currentTuple->IncReference();
            result = SetWord(currentTuple);
            return YIELD;
          }
        }
        else
        {
          // last tuple of the stream
          lastOutputTuple = (Tuple*)local.addr;
          if(lastOutputTuple != 0)
          {
            lastOutputTuple->DecReference();
            lastOutputTuple->DeleteIfAllowed();
            local = SetWord(0);
          }
          return CANCEL;
        }
      }
    case CLOSE:
      if( local.addr != 0 ){
         ((Tuple*)local.addr)->DecReference();
         ((Tuple*)local.addr)->DeleteIfAllowed();
         local = SetWord(0);
      }
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
    "First argument is '" + argstr + "' and second argument is '" + 
    argstr2 + "'.",
    "Operator mergediff: Tuple type and attribute names of first and "
    "second argument must be equal.\n"
    "First argument is '" + argstr + "' and second argument is '" + 
    argstr2 + "'.",
    "Operator mergeunion: Tuple type and attribute names of first and "
    "second argument must be equal.\n"
    "First argument is '" + argstr + "' and second argument is '" + 
    argstr2 + "'." };

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
    if(deleteOldTuple && (currentATuple != 0))
    {
      currentATuple->DecReference();
      currentATuple->DeleteIfAllowed();
    }

    qp->Request(streamA.addr, tuple);
    if(qp->Received(streamA.addr))
    {
      currentATuple = (Tuple*)tuple.addr;
      currentATuple->IncReference();
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
    if(deleteOldTuple && (currentBTuple != 0))
    {
      currentBTuple->DecReference();
      currentBTuple->DeleteIfAllowed();
    }

    qp->Request(streamB.addr, tuple);
    if(qp->Received(streamB.addr))
    {
      currentBTuple = (Tuple*)tuple.addr;
      currentBTuple->IncReference();
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
    const bool t1 = smallerThan(a, b);
    const bool t2 = smallerThan(b, a);
    return !(t1 || t2);
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
            while(currentBTuple != 0 && 
                  TuplesEqual(result, currentBTuple))
            {
              NextBTuple(true);
            }
            result->DecReference();
          }
          else
          {
            currentBTuple->DecReference();
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
            while(currentATuple != 0 && 
                  TuplesEqual(result, currentATuple))
            {
              NextATuple(true);
            }
            result->DecReference();
          }
          else
          {
            currentATuple->DecReference();
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
            while(currentATuple != 0 && 
                  TuplesEqual(tmp, currentATuple))
            {
              NextATuple(true);
            }
            tmp->DecReference();
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
            while(currentBTuple != 0 && 
                  TuplesEqual(tmp, currentBTuple))
            {
              NextBTuple(true);
            }
            tmp->DecReference();
            if(!outputBWithoutA)
            {
              tmp->DeleteIfAllowed();
            }

          }
          else
          {
            /* found match */
            Tuple* match = currentATuple;
            if(outputMatches)
            {
              result = match;
            }

            NextATuple(false);
            while(currentATuple != 0 && 
                  TuplesEqual(match, currentATuple))
            {
              NextATuple(true);
            }
            while(currentBTuple != 0 && 
                  TuplesEqual(match, currentBTuple))
            {
              NextBTuple(true);
            }
            match->DecReference();
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

template<bool outputAWithoutB, 
         bool outputBWithoutA, 
         bool outputMatches> 
int
SetOpValueMapping(Word* args, Word& result, int message, 
                  Word& local, Supplier s)
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
         Operator::SimpleSelect,          // trivial selection function
         SetOpTypeMap<1>   // type mapping
);

/*
2.14.8 Specification of Operator ~mergeunion~

*/
const string 
MergeUnionSpec = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
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
         Operator::SimpleSelect,          // trivial selection function
         SetOpTypeMap<2>   // type mapping
);

/*
2.15 Operator ~mergejoin~

This operator computes the equijoin two streams.

2.15.1 Type mapping function of operators ~mergejoin~ and ~hashjoin~

Type mapping for ~mergejoin~ is

----  ((stream (tuple ((x1 t1) ... (xn tn)))) 
       (stream (tuple ((y1 d1) ... (ym dm)))) xi yj)
         -> (stream (tuple ((x1 t1) ... (xn tn) (y1 d1) ... (ym dm)))) 
            APPEND (i j)
----

Type mapping for ~hashjoin~ is

----  ((stream (tuple ((x1 t1) ... (xn tn)))) 
       (stream (tuple ((y1 d1) ... (ym dm)))) xi yj int)
         -> (stream (tuple ((x1 t1) ... (xn tn) (y1 d1) ... (ym dm)))) 
            APPEND (i j)
----


*/
template<bool expectIntArgument, int errorMessageIdx> 
ListExpr JoinTypeMap (ListExpr args)
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
  int attrAIndex = 
    FindAttribute(nl->Second(nl->Second(streamA)), attrAName, attrTypeA);
  int attrBIndex = 
    FindAttribute(nl->Second(nl->Second(streamB)), attrBName, attrTypeB);
  nl->WriteToString(argstr, nl->Second(nl->Second(streamA)));
  string joinErrorMessages6[] =
  { "Operator mergejoin: First join attribute '" + 
    attrAName + "' is not in "
    "first argument list '" + argstr +"'.\n",
    "Operator sortmergejoin: First join attribute '" + 
    attrAName + "' is not in "
    "first argument list '" + argstr +"'.\n",
    "Operator hashjoin: First join attribute '" + 
    attrAName + "' is not in "
    "first argument list '" + argstr +"'.\n" };
  CHECK_COND( attrAIndex > 0,
    joinErrorMessages6[errorMessageIdx]);
  nl->WriteToString(argstr, nl->Second(nl->Second(streamB)));
  string joinErrorMessages7[] =
  { "Operator mergejoin: Second join attribute '" + 
    attrBName + "'is not in "
    "second argument list '" + argstr +".\n",
    "Operator sortmergejoin: Second join attribute '" + 
    attrBName + "'is not in "
    "second argument list '" + argstr +".\n",
    "Operator hashjoin: Second join attribute '" + 
    attrBName + "'is not in "
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
      "Operator hashjoin: Parameter 'number of buckets' "
      "must be of type int.\n" );
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
         Operator::SimpleSelect,          // trivial selection function
         JoinTypeMap<false, 1>   // type mapping
);

/*
2.17 Operator ~hashjoin~

This operator computes the equijoin two streams via a hash join.
The user can specify the number of hash buckets.

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
2.17.1 Value Mapping Function of Operator ~hashjoin~

*/
int HashJoin(Word* args, Word& result, int message, Word& local, Supplier s);
/*

2.17.3 Definition of Operator ~hashjoin~

*/
Operator extrelhashjoin(
         "hashjoin",        // name
         HashJoinSpec,     // specification
         HashJoin,         // value mapping
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
                (am->CheckKind("DATA", nl->Third(second2), errorInfo)),
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
  int nooffun;
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
        Tuple *newTuple = new Tuple( resultTupleType );
        for( int i = 0; i < tup->GetNoAttributes(); i++ ) {
          //cout << (void*) tup << endl;
          newTuple->CopyAttribute( i, tup, i );
        }  
        supplier = args[1].addr;
        nooffun = qp->GetNoSons(supplier);
        for (int i=0; i < nooffun;i++)
        {
          supplier2 = qp->GetSupplier(supplier, i);
          supplier3 = qp->GetSupplier(supplier2, 1);
          funargs = qp->Argument(supplier3);
          (*funargs)[0] = SetWord(tup);
          qp->Request(supplier3,value);
          newTuple->PutAttribute( tup->GetNoAttributes()+i, 
                                  ((StandardAttribute*)value.addr)->Clone() );
        }
        tup->DeleteIfAllowed();
        result = SetWord(newTuple);
        return YIELD;
      }
      else
        return CANCEL;

    case CLOSE :

      ((TupleType *)local.addr)->DeleteIfAllowed();
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
         Operator::SimpleSelect,          // trivial selection function
         ExtendTypeMap          // type mapping
);

/*
2.19 Operator ~loopjoin~

This operator will fulfill a join of two relations. Tuples in the 
cartesian product which satisfy certain conditions are passed on 
to the output stream.

For instance,

----    query Staedte feed {s1} loopjoin[ fun(t1: TUPLE) plz feed 
          filter [ attr(t1, SName_s1) = .Ort]] count;

        (query (count (loopjoin (rename (feed Staedte) s1) 
          (fun (t1 TUPLE) (filter (feed plz)
          (fun (tuple1 TUPLE) (= (attr t1 SName_s1) (attr tuple1 Ort))))))))
----

The renaming is necessary whenever the underlying relations have at 
least one common attribute name in order to assure that the output 
tuple stream consists of different named attributes.

The type mapping function of the loopjoin operation is as follows:

----    ((stream (tuple x)) (map (tuple x) (stream (tuple y))))
          -> (stream (tuple x * y))
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

/*
2.19.2 Value mapping function of operator ~loopjoin~

SPM: There is a problem when this operator is requested after
it has returned CANCEL it will chrash since it tryes to do a
request on a NULL pointer.

*/

#define USE_PROGRESS
#ifndef USE_PROGRESS

// standard version

struct LoopjoinLocalInfo
{
  Word tuplex;
  Word streamy;
  TupleType *resultTupleType;
};

int Loopjoin(Word* args, Word& result, int message, 
             Word& local, Supplier s)
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
        localinfo->resultTupleType = 
          new TupleType( nl->Second( resultType ) );
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
      ctuplexy = new Tuple( localinfo->resultTupleType );
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

        if( localinfo->resultTupleType != 0 )
          localinfo->resultTupleType->DeleteIfAllowed();
        delete localinfo;
      }
      qp->Close(args[0].addr);
      return 0;
  }

  return 0;
}


#else

// with support for progress queries



struct LoopjoinLocalInfo
{
  Word tuplex;
  Word streamy;
  TupleType *resultTupleType;
  int k1;		//no of tuples read from first argument
  int kRes;		//no of tuples returned
  int startPhase;	//during the start phase, tuple sizes are measured.
                        //Decremented for each result tuple. 
                        //Init with STARTPHASE.
  int Size;	        //aggregated total size over tuples in start phase
  int SizeExt;	        //aggregated root and ext. size over such tuples
  int noAttrs;		//no of attributes
  int *attrSize;	//for each attribute, the size
  int *attrSizeExt;	//for each attribute, the root and extension size
  double *attrSizeD;	//to be returned to RequestProgress
  double *attrSizeExtD;

  clock_t startTime;	//time at open
};

const int STARTPHASE = 50;

int Loopjoin(Word* args, Word& result, int message, 
             Word& local, Supplier s)
{
  ArgVectorPointer funargs = 0;

  Word tuplex = SetWord(Address(0));
  Word tupley = SetWord(Address(0));
  Word tuplexy = SetWord(Address(0));
  Word streamy = SetWord(Address(0));

  Tuple* ctuplex = 0;
  Tuple* ctupley = 0;
  Tuple* ctuplexy = 0;

  ListExpr resultType;

  LoopjoinLocalInfo *localinfo = 0;

  switch ( message )
  {
    case OPEN:

      localinfo = new LoopjoinLocalInfo;
        resultType = GetTupleResultType( s );
        localinfo->resultTupleType = 
          new TupleType( nl->Second( resultType ) );
        localinfo->k1 = 0;
        localinfo->kRes = 0;
        localinfo->startPhase = STARTPHASE;
        localinfo->Size = 0;
        localinfo->SizeExt = 0;
        localinfo->noAttrs = localinfo->resultTupleType->GetNoAttributes();
        localinfo->attrSize = new int[localinfo->noAttrs];
        localinfo->attrSizeExt = new int[localinfo->noAttrs];
        for ( int i = 0; i < localinfo->noAttrs; i++ )
        {
          localinfo->attrSize[i] = 0;
          localinfo->attrSizeExt[i] = 0;
        }
        localinfo->attrSizeD = new double[localinfo->noAttrs];
        localinfo->attrSizeExtD = new double[localinfo->noAttrs];

        localinfo->startTime = clock();

      local = SetWord(localinfo);

      qp->Open (args[0].addr);
      qp->Request(args[0].addr, tuplex);
      if (qp->Received(args[0].addr))
      {
        localinfo->k1++;
        funargs = qp->Argument(args[1].addr);
        (*funargs)[0] = tuplex;
        streamy=args[1];
        qp->Open (streamy.addr);

        localinfo->tuplex = tuplex;
        localinfo->streamy = streamy;

      }
      else
      {
        localinfo->tuplex = SetWord(Address(0));
        localinfo->streamy = SetWord(Address(0));
      }
      return 0;

    case REQUEST:

      localinfo=(LoopjoinLocalInfo *) local.addr;

      if ( localinfo->tuplex.addr == 0) return CANCEL;

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
            localinfo->k1++;
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
      ctuplexy = new Tuple( localinfo->resultTupleType );
      tuplexy = SetWord(ctuplexy);
      Concat(ctuplex, ctupley, ctuplexy);
      ctupley->DeleteIfAllowed();
      localinfo->kRes++;
      if ( localinfo->startPhase )
      {
        localinfo->Size += ctuplexy->GetSize();
        localinfo->SizeExt += ctuplexy->GetExtSize();
        for ( int i = 0; i < localinfo->noAttrs; i++ )
        {
          localinfo->attrSize[i] += ctuplexy->GetSize(i);
          localinfo->attrSizeExt[i] += ctuplexy->GetExtSize(i);
        }
        localinfo->startPhase--;
      }
      result = tuplexy;
      return YIELD;

    case CLOSE:

      localinfo=(LoopjoinLocalInfo *) local.addr;

      if ( localinfo->tuplex.addr != 0 )
      {
        if( localinfo->streamy.addr != 0 )
          qp->Close( localinfo->streamy.addr );

        if( localinfo->tuplex.addr != 0 )
          ((Tuple*)localinfo->tuplex.addr)->DeleteIfAllowed();

        if( localinfo->resultTupleType != 0 )
          localinfo->resultTupleType->DeleteIfAllowed();

        //delete localinfo;   *not done in the progress case!*
      }

      qp->Close(args[0].addr);
      return 0;


    case PROGRESS:

      ProgressInfo p1;
      ProgressInfo *pRes;
      static clock_t clocksPerMilliSecond = CLOCKS_PER_SEC / 1000 ; 

      pRes = (ProgressInfo*) result.addr;
      localinfo=(LoopjoinLocalInfo *) local.addr;

      if ( localinfo ) 
      {
        if ( !localinfo->startPhase ) 		//stable state assumed
        {
          if ( qp->RequestProgress(args[0].addr, &p1) )
          {
            pRes->Card = ((double) localinfo->kRes) * 
              (p1.Card / (double) localinfo->k1);

	    pRes->Size = localinfo->Size / STARTPHASE;
	    pRes->SizeExt = localinfo->SizeExt / STARTPHASE;
            pRes->noAttrs = localinfo->noAttrs;
            for ( int i = 0; i < localinfo->noAttrs; i++ )
            {
              localinfo->attrSizeD[i] = localinfo->attrSize[i] / STARTPHASE;
              localinfo->attrSizeExtD[i] = localinfo->attrSizeExt[i] 
                / STARTPHASE;
            }
            pRes->attrSize = localinfo->attrSizeD;
            pRes->attrSizeExt = localinfo->attrSizeExtD;

            pRes->Time = 
              ((clock() - localinfo->startTime) / clocksPerMilliSecond)
              * (p1.Card / localinfo->k1);
            pRes->Progress =  localinfo->k1 / p1.Card;
            return YIELD;
          }
        }
      }
        
      return CANCEL;
  }

  return 0;
}


#endif



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
         Operator::SimpleSelect,            // trivial selection function
         LoopjoinTypeMap            // type mapping
);

/*

2.20 Operator ~loopselect~

This operator is similar to the ~loopjoin~ operator except that it 
only returns the inner tuple (instead of the concatination of two 
tuples). Tuples in the cartesian product which satisfy certain conditions 
are passed on to the output stream.

For instance,

----    query Staedte feed loopsel [ fun(t1: TUPLE) plz feed 
          filter [ attr(t1, SName) = .Ort ]] count;

        (query (count (loopsel (feed Staedte) 
          (fun (t1 TUPLE) (filter (feed plz)
          (fun (tuple1 TUPLE) (= (attr t1 SName) (attr tuple1 Ort))))))))

----

7.4.1 Type mapping function of operator ~loopsel~

The type mapping function of the loopsel operation is as follows:

----    ((stream (tuple x)) (map (tuple x) (stream (tuple y))))
          -> (stream (tuple y))
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
Loopselect(Word* args, Word& result, int message, 
           Word& local, Supplier s)
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
        localinfo->resultTupleType = 
          new TupleType( nl->Second( resultType ) );
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
      // prepare tuplex and tupley for processing. 
      // if rely is exausted: fetch next tuplex.
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

        if( localinfo->resultTupleType != 0 ) 
          localinfo->resultTupleType->DeleteIfAllowed();
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
         "loopsel",                // name
         LoopselectSpec,           // specification
         Loopselect,               // value mapping
         Operator::SimpleSelect,   // trivial selection function
         LoopselectTypeMap         // type mapping
);

/*
2.21 Operator ~extendstream~

This operator is implemented by the idea of LOOPJOIN and EXTEND

The type mapping function of the extendstream operation is as follows:

----    ((stream (tuple x)) (map (tuple x) (stream (y))))
          -> (stream (tuple x * y))
  where x = ((x1 t1) ... (xn tn)) and
        y is a simple data type such as string, integer, or real ...
----

----    ((stream (tuple x)) ( (b1 (map (tuple x) (stream (y)))))
          -> (stream (tuple x * y))
  where x = ((x1 t1) ... (xn tn)) and 
        y is a simple data type such as string, integer, or real ...
----

For instance,

----  query people feed extendstream [parts : components (.name) ] consume;
----

*/

ListExpr ExtendStreamTypeMap(ListExpr args)
{
  ListExpr first, second;
  ListExpr listX, listY, list, outlist, errorInfo;
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
    TypeOfRelAlgSymbol(nl->First(nl->Third(nl->Second(nl->First(second)))) == 
      stream),
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
             (am->CheckKind("DATA",
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

int ExtendStream(Word* args, Word& result, int message, 
                 Word& local, Supplier s)
{
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
        //funargs = qp->Argument(args[1].addr);   
        //here should be changed to the following...
        supplier = args[1].addr;
        supplier2 = qp->GetSupplier( supplier, 0 );
        supplier3 = qp->GetSupplier( supplier2, 1 );
        funargs = qp->Argument( supplier3 );

        (*funargs)[0] = wTupleX;
        qp->Open( supplier3 );

        //3. save the local information
        localinfo = new ExtendStreamLocalInfo;
        resultType = GetTupleResultType( s );
        localinfo->resultTupleType = 
          new TupleType( nl->Second( resultType ) );
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

      //2. prepare tupleX and wValueY. 
      //If wValueY is empty, then get next tupleX
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
      tupleXY = new Tuple( localinfo->resultTupleType );

      for( int i = 0; i < localinfo->tupleX->GetNoAttributes(); i++ )
        tupleXY->CopyAttribute( i, localinfo->tupleX, i );

      tupleXY->PutAttribute( localinfo->tupleX->GetNoAttributes(), 
                             (StandardAttribute*)wValueY.addr );

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

        if( localinfo->resultTupleType != 0 )
          localinfo->resultTupleType->DeleteIfAllowed();
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
const string ExtendStreamSpec  = 
  "( ( \"Signature\" \"Syntax\" \"Meaning\" "
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
         Operator::SimpleSelect,            // trivial selection function
         ExtendStreamTypeMap            // type mapping
);

/* 
2.20 Operator ~projectextend~

This operator does the same as a combination of ~extend~ with subsequent
~project~, but in a single operation. This will saves a lot of time, because
attributes are only copied once and attributes to be removed will not be 
copied at all!

The signature of the ~projectextend~ operation is as follows:

----

      stream(tuple((a1 t1) ... (an tn)))
    x (ai1, ... aij)
    x ( (bk1 (tuple((a1 t1) ... (an tn)) --> tk1)) 
        ...
        (bkm (tuple((a1 t1) ... (an tn)) --> tkm))
      )
 --> stream(tuple((ai1 ti1) ... (aij tij) (bk1 tk1) ... (bkm tkm)))

    where ai, ..., aij in {a1, ..., an}.

----

For instance,

----
  query people feed 
  projectext[id; 
             wood_perimeter: perimeter(.wood), 
             wood_size: area(.wood)
            ]
  consume;

----

*/

/*
2.20.1 Type Mapping for operator ~projectextend~

*/
ListExpr ExtProjectExtendTypeMap(ListExpr args)
{
  ListExpr first, second, third,
           rest, errorInfo,
           first2, second2, firstr, attrtype,
           lastNewAttrList, lastNumberList,
           numberList, newAttrList;
  string   argstr, argstr2, attrname="";
  int      noAttrs=0, j=0;
  bool     firstcall = true;

  errorInfo = nl->OneElemList(nl->SymbolAtom("ERROR"));

  CHECK_COND(nl->ListLength(args) == 3,
    "Operator produxtextend expects a list of length three.");

  first = nl->First(args);
  second = nl->Second(args);
  third = nl->Third(args);

  // check whether first is a tuplestream
  nl->WriteToString(argstr, first);
  CHECK_COND(nl->ListLength(first) == 2 &&
             TypeOfRelAlgSymbol(nl->First(first)) == stream &&
             nl->ListLength(nl->Second(first)) == 2 &&
             TypeOfRelAlgSymbol(nl->First(nl->Second(first))) == tuple &&
             nl->ListLength(nl->Second(nl->Second(first))) >0,
    "Operator projecttextend expects a first list with structure "
    "(stream (tuple ((a1 t1)...(an tn))))\n"
    "Operator projectextend gets a first list with structure '" 
    + argstr + "'.");

  // check whether second is a list of attribute names, that all occur in first
  nl->WriteToString(argstr, second);
  CHECK_COND( !nl->IsAtom(second) &&
    nl->ListLength(second) >= 0,
    "Operator projectextend expects as second argument a "
    "list of attribute names\n"
    "Operator projectextend gets as second argument '" + 
    argstr + "'.\n" );

  if( nl->IsEmpty(second) )
  {
    noAttrs = 0;
    numberList = nl->TheEmptyList();
  }
  else
  {
    noAttrs = nl->ListLength(second);
    while ( (noAttrs>0) && !(nl->IsEmpty(second)) )
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
          "Operator projectextend: Attributename '" + attrname + 
          "' is not a known attributename in the tuple stream.");
            return nl->SymbolAtom("typeerror");
      }
    } 
  }
  // Now, we have all projection attrs in newAttrList and their 
  // indexes within the tuple within *numberList

  // check whether third is a list of pairs (attrname map), where attrname is 
  // not in first and map has type (map T Ti) for T is the tupletype from first
  CHECK_COND(!(nl->IsAtom(third)) &&
             (nl->ListLength(third) >= 0),
    "Operator projectextend: Third argument list may not "
    "be an atom" );


  // handle list of new attributes and mapping functions (3rd argument)
  rest = third;
  while (!(nl->IsEmpty(rest))) // for all new attrs
  {
    firstr = nl->First(rest);
    rest = nl->Rest(rest);
    first2 = nl->First(firstr);
    second2 = nl->Second(firstr);

    // check new attribute name
    nl->WriteToString(argstr, first2);
    CHECK_COND( (nl->IsAtom(first2)) &&
                (nl->AtomType(first2) == SymbolType),
      "Operator projectextend: Attribute name '" + argstr +
      "' is not an atom or not of type SymbolType" );

    // check mapping function for new attribute
    nl->WriteToString(argstr, second2);
    CHECK_COND( (nl->ListLength(second2) == 3) &&
                (TypeOfRelAlgSymbol(nl->First(second2)) == ccmap) &&
                (am->CheckKind("DATA", nl->Third(second2), errorInfo)),
      "Operator projectextend expects a mapping function with list "
      "structure (<attrname> (map tuple(X) ti) ).\n"
      "Operator projectextend gets a list '" + argstr + "'.\n" 
      "The third list element ('ti') for such a 'ccmap' must "
      "be of kind 'DATA'.\n" );

    // Do typechecking for first extension mapping function argument:
    nl->WriteToString(argstr, nl->Second(second2));
    nl->WriteToString(argstr2, nl->Second(first));
    CHECK_COND( (nl->Equal(nl->Second((first)),nl->Second(second2))),
      "Operator projectextend: The argument tuple type '" + argstr +
      "' in a mapping function is wrong. It should be '" + argstr2 + "'.\n" );

    // append new attribute (name type)
    if (firstcall)
    {
      firstcall = false;
      newAttrList = 
        nl->OneElemList(nl->TwoElemList(first2, nl->Third(second2)));
      lastNewAttrList = newAttrList;
    }
    else
    { 
      lastNewAttrList =
      nl->Append(lastNewAttrList, 
                  nl->TwoElemList(first2, nl->Third(second2)));
    }
  }

  nl->WriteToString(argstr, newAttrList);
  CHECK_COND( CompareNames(newAttrList),
              "Operator projectextend: found doubly "
              "defined attribute names in concatenated list.\n"
              "The list is '" + argstr + "'\n" );


  ListExpr reslist =
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
  //   nl->WriteToString(argstr, reslist);
  //   cout << "ExtProjectExtendTypeMap(): " 
  //        << "reslist = " << argstr << endl;
  return reslist;
}

/*
2.20.2 Value Mapping for operator ~projectextend~

*/

int
ExtProjectExtendValueMap(Word* args, Word& result, int message, 
                         Word& local, Supplier s)
{
  //  cout << "ExtProjectExtendValueMap() called." << endl;
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
      Word elem1, elem2, value;
      int i=0, noOfAttrs=0, index=0, nooffun=0;
      Supplier son, supplier, supplier2, supplier3;
      ArgVectorPointer extFunArgs;

      qp->Request(args[0].addr, elem1);
      if (qp->Received(args[0].addr))
      {
        TupleType *tupleType = (TupleType *)local.addr;
        Tuple *currTuple     = (Tuple*) elem1.addr;
        Tuple *resultTuple   = new Tuple( tupleType );

        // copy attrs from projection list
        noOfAttrs = ((CcInt*)args[3].addr)->GetIntval();
        for(i = 0; i < noOfAttrs; i++)
        {
          son = qp->GetSupplier(args[4].addr, i);
          qp->Request(son, elem2);
          index = ((CcInt*)elem2.addr)->GetIntval();
          resultTuple->CopyAttribute(index-1, currTuple, i);
        }

        // evaluate and add attrs from extension list
        supplier = args[2].addr;           // get list of ext-functions
        nooffun = qp->GetNoSons(supplier); // get num of ext-functions
        for(i = 0; i< nooffun; i++)
        {
          supplier2 = qp->GetSupplier(supplier, i); // get an ext-function
          supplier3 = qp->GetSupplier(supplier2, 1);
          extFunArgs = qp->Argument(supplier3);
          (*extFunArgs)[0] = SetWord(currTuple);     // pass argument
          qp->Request(supplier3,value);              // call extattr mapping
//  The original implementation tried to avoid copying the function result,
//  but somehow, this results in a strongly growing tuplebuffer on disk:
//           resultTuple->PutAttribute( 
//             noOfAttrs + i, (StandardAttribute*)value.addr );
//           qp->ReInitResultStorage( supplier3 );
          resultTuple->PutAttribute( noOfAttrs + i, 
                ((StandardAttribute*)value.addr)->Clone() );
        }
        currTuple->DeleteIfAllowed();
        result = SetWord(resultTuple);
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
2.20.3 Specification of operator ~projectextend~

*/
const string ExtProjectExtendSpec = 
  "( ( \"Signature\" \"Syntax\" \"Meaning\" "
  "\"Example\" ) "
  "( <text>"
  "stream(tuple((a1 t1) ... (an tn))) \n"
  "  x (ai1, ... aij)\n"
  "  x ( (bk1 (tuple((a1 t1) ... (an tn)) --> tk1))\n"
  "      ...\n"
  "      (bkm (tuple((a1 t1) ... (an tn)) --> tkm))\n"
  "    )\n"
  " --> stream(tuple((ai1 ti1) ... (aij tij) (bk1 tk1) ... (bkm tkm)))\n"
  " where ai, ..., aij in {a1, ..., an}"
  "</text--->"
  "<text>_ projectextend[ list; funlist ]</text--->"
  "<text>Project tuple to list of attributes and extend with "
  "new ones from funlist. The projection list may be empty, "
  "but the extension list must contain at least one entry."
  "You may even 'replace' an attribute (but be careful with that)."
  "</text--->"
  "<text>query Orte feed projectextend"
  "[Ort, Vorwahl; BevT2: .BevT*2, BevT: (.BevT + 30)] consume</text--->"
  ") )";

/*
2.20.4 Definition of operator ~projectextend~

*/
Operator extrelprojectextend (
         "projectextend",           // name
         ExtProjectExtendSpec,      // specification
         ExtProjectExtendValueMap,  // value mapping
         Operator::SimpleSelect,    // trivial selection function
         ExtProjectExtendTypeMap    // type mapping
);

/*
2.21 Operator ~projectextendstream~

This operator does the same as ~extendstream~ with a projection on some 
attributes. It is very often that a big attribute is converted into a 
stream of a small pieces, for example, a text into keywords. When 
applying the ~extendstream~ operator, the big attribute belongs to the 
result type, and is copied for every occurrence of its smaller pieces. 
A projection is a normal operation after such operation. To avoid this 
copying, the projection operation is now built in the operation.

The type mapping function of the ~projectextendstream~ operation is 
as follows:

----  ((stream (tuple ((x1 T1) ... (xn Tn)))) (ai1 ... aik)
        (map (tuple ((x1 T1) ... (xn Tn))) (b stream(Tb))))  ->
      (APPEND
        (k (i1 ... ik))
        (stream (tuple ((ai1 Ti1) ... (aik Tik)(b Tb)))))
----

For instance,

----  query people feed 
        projectextendstream [id, age; parts : .name keywords] 
        consume;
----

*/
ListExpr ProjectExtendStreamTypeMap(ListExpr args)
{
  ListExpr errorInfo;
  errorInfo = nl->OneElemList(nl->SymbolAtom("ERROR"));
  string argstr, argstr2;

  CHECK_COND(nl->ListLength(args) == 3,
    "Operator projectextendstream expects a list of length three.");

  ListExpr first = nl->First(args),
           second = nl->Second(args),
           third = nl->Third(args);

  nl->WriteToString(argstr, first);
  CHECK_COND(nl->ListLength(first) == 2  &&
    (TypeOfRelAlgSymbol(nl->First(first)) == stream) &&
    (nl->ListLength(nl->Second(first)) == 2) &&
    (TypeOfRelAlgSymbol(nl->First(nl->Second(first))) == tuple) &&
    (nl->ListLength(nl->Second(first)) == 2) &&
    (IsTupleDescription(nl->Second(nl->Second(first)))),
    "Operator projectextendstream expects as first argument a "
    "list with structure (stream (tuple ((a1 t1)...(an tn))))\n"
    "Operator projectextendstream gets as first argument '" + 
    argstr + "'." );

  nl->WriteToString(argstr, second);
  CHECK_COND( !nl->IsAtom(second) &&
    nl->ListLength(second) > 0,
    "Operator projectextendstream expects as second argument a "
    "list of attribute names\n"
    "Operator projectextendstream gets as second argument '" + 
    argstr + "'.\n" );

  nl->WriteToString(argstr, third);
  CHECK_COND( !nl->IsAtom(third) &&
    nl->ListLength(third) == 1 &&
    nl->ListLength(nl->First(third)) == 2 &&
    TypeOfRelAlgSymbol(nl->First(nl->Second(nl->First(third)))) == ccmap &&
    nl->ListLength(nl->Third(nl->Second(nl->First(third)))) == 2 &&
    TypeOfRelAlgSymbol(nl->First(nl->Third(nl->Second(nl->First(third)))) == 
      stream),
    "Operator projectextendstream expects as third argument a list "
    "with length two"
    " and structure (<attrname>(map (tuple (...)) (stream <type(DATA)>))).\n"
    "Operator projectextendstream gets as third argument '" + 
    argstr + "'.\n" );
  third = nl->First(third);

  ListExpr secondRest = second;
  ListExpr secondFirst, attrType, newAttrList, numberList;
  secondFirst = attrType = newAttrList = numberList = nl->Empty();
  ListExpr lastNewAttrList, lastNumberList;
  lastNewAttrList = lastNumberList = nl->Empty();

  string attrName = "";
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
        "Operator projectextendstream expects as second argument a "
        "list of attribute names\n"
        "The element '" + argstr + "' is not an attribute name.\n" );
      return nl->SymbolAtom("typeerror");
    }

    int j = FindAttribute( nl->Second(nl->Second(first)), 
                           attrName, attrType );
    if( j )
    {
      if( firstCall )
      {
        firstCall = false;
        newAttrList = 
          nl->OneElemList(nl->TwoElemList(secondFirst, attrType));
        lastNewAttrList = newAttrList;
        numberList = nl->OneElemList(nl->IntAtom(j));
        lastNumberList = numberList;
      }
      else
      {
        lastNewAttrList =
          nl->Append( lastNewAttrList, 
                      nl->TwoElemList(secondFirst, attrType) );
        lastNumberList =
          nl->Append( lastNumberList, nl->IntAtom(j) );
      }
    }
    else
    {
      nl->WriteToString(argstr, first);
      ErrorReporter::ReportError(
        "Operator projectextendstream expects as second argument a "
        "list of attribute names\n"
        " Attribute name '" + attrName + 
        "' does not belong to the tuple stream: \n"
        "'" + argstr + "'.\n" );
      return nl->SymbolAtom("typeerror");
    }
  }


  CHECK_COND((nl->Equal(nl->Second(first), nl->Second(nl->Second(third)))),
    "Operator projectextendstream: Input tuple for mapping "
    "(third argument) and the first argument\n"
    "tuple must have the same description." );

  nl->WriteToString(argstr,
                    nl->Second(nl->Third(nl->Second(third))));
  CHECK_COND((nl->IsAtom(nl->Second(nl->Third(nl->Second(third)))))  &&
             (am->CheckKind("DATA",
               nl->Second(nl->Third(nl->Second(third))), errorInfo)),
    "Operator projectextendstream: the return stream value in the "
    "third argument\n"
    "must implement the kind DATA.\n"
    "The return stream value is of type '" + argstr + "'.\n" );

  ListExpr appendAttr = nl->TwoElemList(
                          nl->First(third),
                          nl->Second(nl->Third(nl->Second(third))));

  nl->WriteToString(argstr, nl->Second(nl->Second(first)));
  nl->WriteToString(argstr2, appendAttr);
  CHECK_COND( AttributesAreDisjoint( nl->Second(nl->Second(first)), 
                                     nl->OneElemList(appendAttr) ),
    "Operator projectextendstream: new attribute name '" + 
    argstr2 + "' must be different\n"
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

int ProjectExtendStream(Word* args, Word& result, int message, 
                        Word& local, Supplier s)
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
        localinfo->resultTupleType = 
          new TupleType( nl->Second( resultType ) );
        localinfo->tupleX = (Tuple*)wTupleX.addr;
        localinfo->streamY = SetWord( supplier3 );

        //4. get the attribute numbers
        int noOfAttrs = ((CcInt*)args[3].addr)->GetIntval();
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

      //2. prepare tupleX and wValueY. 
      //If wValueY is empty, then get next tupleX
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
      tupleXY = new Tuple( localinfo->resultTupleType );

      size_t i;
      for( i = 0; i < localinfo->attrs.size(); i++ )
        tupleXY->CopyAttribute( localinfo->attrs[i], localinfo->tupleX, i );

      tupleXY->PutAttribute( i, (StandardAttribute*)wValueY.addr );

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

        if( localinfo->resultTupleType != 0 )
          localinfo->resultTupleType->DeleteIfAllowed();
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
const string ProjectExtendStreamSpec = 
  "( ( \"Signature\" \"Syntax\" \"Meaning\" "
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
         Operator::SimpleSelect,               // trivial selection function
         ProjectExtendStreamTypeMap            // type mapping
);

/*
2.20 Operator ~concat~

2.20.1 Type mapping function of operator ~concat~

Type mapping for ~concat~ is

----    ((stream (tuple (a1:d1 ... an:dn))) 
         (stream (tuple (b1:d1 ... bn:dn))))
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
      if( local.addr != 0 )
        ((CcInt*)local.addr)->DeleteIfAllowed();
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
         Operator::SimpleSelect, // trivial selection function
         ConcatTypeMap           // type mapping
);

/*
2.21 Operator ~groupby~

2.21.1 Type mapping function of operator ~groupby~

Result type of ~groupby~ operation.

----   Let X = tuple ((x1 t1) ... (xn tn)), R = rel(X):

       ( (stream X) (xi1 ... xik) ( (y1 (map R T1)) ... (ym (map R Tm)) )

        -> ( APPEND (m p1 ... pm) 
               (stream (tuple (xj1 tj1)... (xjl tjl) (y1 T1) ... (ym Tm))))

       with tj,Ti in kind DATA, xi <> xj and k+l=n, pi <> pj and 1 <= pi <= m.
       This means attributes xi ... xik are removed from the stream and 
       attributes y1 ... ym are appended. These new attributes represent 
       aggregated values computed by maps of R -> Ti which must have a 
       result type of kind DATA.
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
    listOk = listOk && !nl->IsAtom(second) && 
             ( nl->ListLength(second) > 0 );

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
    int j = FindAttribute(nl->Second(nl->Second(first)), 
                          attrname, attrtype);
    if (j)
    {
      if (!firstcall)
      {
        lastlistn = nl->Append(lastlistn,nl->TwoElemList(first2,attrtype));
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
      ListExpr mapDef = nl->Second(firstr); 
      ListExpr mapOut = nl->Third(mapDef);

      // check list structure
      bool listOk = true;
      listOk = listOk && ( nl->IsAtom(newAttr) );
      listOk = listOk && ( nl->ListLength(mapDef) == 3 );
      listOk = listOk && ( nl->AtomType(newAttr) == SymbolType );
      listOk = listOk && ( TypeOfRelAlgSymbol(nl->First(mapDef)) == ccmap );
      listOk = listOk && ( nl->Equal(groupType, nl->Second(mapDef)) );

      if( !listOk ) 
        // Todo: there could be more fine grained error messages
      { 
        ErrorReporter::ReportError(
          "groupby: Function definition is not correct!");
        return nl->SymbolAtom("typeerror");
      }

      // Check if mapOut is of kind DATA or 
      // if the function returns a typeerror
      ListExpr typeConstructor = nl->TheEmptyList();
      if ( nl->IsAtom(mapOut) ) // function returns a simple type
      {
        typeConstructor = mapOut;

      } else { // function returns a complex type

        typeConstructor = nl->First(mapOut);
      }

      // check if the Type Constructor belongs to KIND DATA
      // If the functions result type is typeerror this check will also fail
      ListExpr errorInfo = nl->OneElemList(nl->SymbolAtom("ErrorInfo"));
      if ( !am->CheckKind("DATA", typeConstructor, errorInfo) ) {

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


  if ( !CompareNames(listn) ) 
  { // check if attribute names are uniqe
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
  long MAX_MEMORY;
};

int GroupByValueMapping
(Word* args, Word& result, int message, Word& local, Supplier supplier)
{
  Tuple *s = 0;
  Word sWord =SetWord(Address(0));
  TupleBuffer* tp = 0;
  GenericRelationIterator* relIter = 0;
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
    {
      // Get the first tuple pointer and store it in the
      // GroupBylocalInfo structure
      qp->Open (args[0].addr);
      qp->Request(args[0].addr, sWord);
      if (qp->Received(args[0].addr))
      {
        gbli = new GroupByLocalInfo;
        gbli->t = (Tuple*)sWord.addr;
        gbli->t->IncReference();
        ListExpr resultType = GetTupleResultType( supplier );
        gbli->resultTupleType = new TupleType( nl->Second( resultType ) );
        gbli->MAX_MEMORY = qp->MemoryAvailableForOperator();
        local = SetWord(gbli);

        cmsg.info("ERA:ShowMemInfo") 
          << "GroupBy.MAX_MEMORY (" 
          << gbli->MAX_MEMORY / 1024 << " MB): = " 
          << gbli->MAX_MEMORY / gbli->t->GetExtSize() 
          << " Tuples" << endl;
        cmsg.send();
      }
      else
      {
        local = SetWord(0);
      }
      return 0;
    }
    case REQUEST:
    {
      Counter::getRef("GroupBy:Request")++;
      if(local.addr == 0) 
        return CANCEL;
      else
      {
        gbli = (GroupByLocalInfo *)local.addr;
        if( gbli->t == 0 ) // Stream ends
          return CANCEL;

        tp = new TupleBuffer(gbli->MAX_MEMORY);
        tp->AppendTuple(gbli->t);
      }

      // get number of attributes
      numberatt = ((CcInt*)args[indexOfCountArgument].addr)->GetIntval();

      ifequal = true;
      // Get next tuple
      qp->Request(args[0].addr, sWord);
      while ((qp->Received(args[0].addr)) && ifequal)
      {
        s = (Tuple*)sWord.addr;
        for (k = 0; k < numberatt; k++) // check if  tuples t = s
        {
          // loop over all grouping attributes
          attribIdx = 
            ((CcInt*)args[startIndexOfExtraArguments+k].addr)->GetIntval();
          j = attribIdx - 1;
          if (((Attribute*)gbli->t->GetAttribute(j))->
               Compare((Attribute *)s->GetAttribute(j)))
          {
            ifequal = false;
            break;
          }
        }

        if (ifequal) // store in tuple buffer
        {
          tp->AppendTuple(s);
          s->DeleteIfAllowed();
          qp->Request(args[0].addr, sWord); 
          // get next tuple
        }
        else
        { 
          // store tuple pointer in local info
          gbli->t->DecReference();
          gbli->t->DeleteIfAllowed();
          gbli->t = s; 
          gbli->t->IncReference();
        }
      }
      if (ifequal) 
      // last group finished, stream ends
      {
        gbli->t->DecReference();
        gbli->t->DeleteIfAllowed();
        gbli->t = 0;
      }

      // create result tuple
      Tuple *t = new Tuple( gbli->resultTupleType );
      relIter = tp->MakeScan();
      s = relIter->GetNextTuple();

      // copy in grouping attributes
      for(i = 0; i < numberatt; i++)
      {
        attribIdx = 
          ((CcInt*)args[startIndexOfExtraArguments+i].addr)->GetIntval();
        t->CopyAttribute(attribIdx - 1, s, i);
      }
      s->DeleteIfAllowed();
      value2 = (Supplier)args[2].addr; // list of functions
      noOffun  =  qp->GetNoSons(value2);
      delete relIter;

      for(i = 0; i < noOffun; i++)
      {
        // prepare arguments for function i
        supplier1 = qp->GetSupplier(value2, i);
        supplier2 = qp->GetSupplier(supplier1, 1);
        vector = qp->Argument(supplier2);
        // The group was stored in a relation identified by symbol group
        // which is a typemap operator. Here it is stored in the 
        // argument vector
        (*vector)[0] = SetWord(tp);

        // compute value of function i and put it into the result tuple
        qp->Request(supplier2, value);
        t->PutAttribute(numberatt + i, (Attribute*)value.addr);
        qp->ReInitResultStorage(supplier2);
      }
      result = SetWord(t);
      delete tp;
      return YIELD;
    }
    case CLOSE:
    {
      if( local.addr != 0 )
      {
        gbli = (GroupByLocalInfo *)local.addr;
        if( gbli->resultTupleType != 0 )
          gbli->resultTupleType->DeleteIfAllowed();
        if( gbli->t != 0 )
        {
          gbli->t->DecReference();
          gbli->t->DeleteIfAllowed();
        }
        delete gbli;
      }
      qp->Close(args[0].addr);
      return 0;
    }
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
         Operator::SimpleSelect,          // trivial selection function
         GroupByTypeMap         // type mapping
);

/*
2.22 Operator ~aggregate~

2.22.1 Type mapping function of operator ~aggregate~

Type mapping for ~aggregate~ is

----    ((stream (tuple ((x1 T1) ... (xn Tn)))) 
          xi (map Ti Ti Tx) Tx                   ->

        (APPEND i Tx)
----
 
*/
ListExpr AggregateTypeMap( ListExpr args )
{
  string argstr, argstr2;

  CHECK_COND(nl->ListLength(args) == 4,
    "Operator aggregate expects a list of length four.");

  ListExpr first = nl->First(args),
           second = nl->Second(args),
           third = nl->Third(args),
           fourth = nl->Fourth(args);

  nl->WriteToString(argstr, first);
  CHECK_COND(nl->ListLength(first) == 2  &&
    (TypeOfRelAlgSymbol(nl->First(first)) == stream) &&
    (nl->ListLength(nl->Second(first)) == 2) &&
    (TypeOfRelAlgSymbol(nl->First(nl->Second(first))) == tuple) &&
    (nl->ListLength(nl->Second(first)) == 2) &&
    (IsTupleDescription(nl->Second(nl->Second(first)))),
    "Operator aggregate expects as first argument a list with structure "
    "(stream (tuple ((a1 t1)...(an tn))))\n"
    "Operator extend gets as first argument '" + argstr + "'." );

  nl->WriteToString(argstr, second);
  CHECK_COND(nl->IsAtom(second) &&
    nl->AtomType(second) == SymbolType,
    "Operator aggregate expects as second argument an atom "
    "(an attribute name).\n"
    "Operator aggregate gets '" + argstr + "'.\n");

  string attrName = nl->SymbolValue(second);
  ListExpr attrType;
  int j;
  if( !(j = FindAttribute(nl->Second(nl->Second(first)), 
                          attrName, attrType)) )
  {
    nl->WriteToString(argstr, nl->Second(nl->Second(first)));
    ErrorReporter::ReportError(
      "Operator aggregate expects as secong argument an attribute name.\n"
      "Attribute name '" + attrName + 
      "' does not belong to the tuple of the first argument.\n"
      "Known Attribute(s): " + argstr + "\n");
    return nl->SymbolAtom("typeerror");
  }

  nl->WriteToString(argstr, third);
  CHECK_COND( nl->ListLength(third) == 4 &&
    TypeOfRelAlgSymbol(nl->First(third)) == ccmap &&
    (nl->IsAtom(nl->Second(third)) && 
    nl->AtomType(nl->Second(third)) == SymbolType ||
    nl->ListLength(nl->Second(third)) == 1) &&
    (nl->IsAtom(nl->Third(third)) && 
    nl->AtomType(nl->Third(third)) == SymbolType ||
    nl->ListLength(nl->Third(third)) == 1) &&
    nl->Equal(nl->Second(third), nl->Third(third)) &&
    (nl->IsAtom(nl->Fourth(third)) && 
    nl->AtomType(nl->Fourth(third)) == SymbolType ||
    nl->ListLength(nl->Fourth(third)) == 1),
    "Operator aggregate expects as third argument a list with length four"
    " and structure (map t1 t1 t2).\n"
    " Operator aggregate gets as third argument '" + argstr + "'.\n" );

  nl->WriteToString(argstr, nl->TwoElemList(nl->Second(third), 
                    nl->Third(third)) );
  nl->WriteToString(argstr2, attrType);
  CHECK_COND(nl->Equal(nl->Second(third), attrType) &&
    nl->Equal(nl->Third(third), attrType),
    "Operator aggregate expects that the input types for the mapping "
    "and the type of the attribute\n"
    "passed as first argument have the same description.\n"
    "Input types for the mapping: '" + argstr + "'.\n"
    "Attribute type: '" + argstr2 + "'.\n");

  nl->WriteToString(argstr, nl->Fourth(third));
  CHECK_COND(nl->Equal(nl->Fourth(third), attrType),
    "Operator aggregate expects that the result type for the mapping "
    "and the type of the attribute\n"
    "passed as first argument have the same description.\n"
    "Result type for the mapping: '" + argstr + "'.\n"
    "Attribute type: '" + argstr2 + "'.\n");

  nl->WriteToString(argstr, fourth);
  CHECK_COND(nl->IsAtom(fourth) && nl->AtomType(fourth) == SymbolType ||
             nl->ListLength(fourth) == 1,
    "Operator aggregate expects the fourth argument to be a single value.\n"
    "Operator aggregate gets as fourth argument: '" + argstr + "'.\n");

  nl->WriteToString(argstr2, nl->Fourth(third));
  CHECK_COND(nl->Equal(nl->Fourth(third), fourth),
    "Operator aggregate expects that the result type for the mapping in "
    "the third argument\n"
    "and the fourth argument have the same description.\n"
    "Result type for the mapping: '" + argstr2 + "'.\n"
    "Fourth argument: '" + argstr + "'.\n");

  return nl->ThreeElemList(
           nl->SymbolAtom( "APPEND" ),
           nl->OneElemList(nl->IntAtom(j)),
           fourth );
}

/*
2.18.2 Value mapping function of operator ~aggregate~

*/
int Aggregate(Word* args, Word& result, int message, Word& local, Supplier s)
{
  // The argument vector contains the following values:
  // args[0] = stream of tuples
  // args[1] = attribute name
  // args[2] = mapping function
  // args[3] = zero value
  // args[4] = attribute index added by APPEND

  Word t = SetWord( Address(0) ); 
  ArgVectorPointer vector;

  qp->Open(args[0].addr);
  result = qp->ResultStorage(s);
  int index = ((CcInt*)args[4].addr)->GetIntval();

  Word iterWord = args[3];

  qp->Request( args[0].addr, t );
  while( qp->Received( args[0].addr ) )
  {
    vector = qp->Argument(args[2].addr);
    (*vector)[0] = iterWord;
    (*vector)[1] = SetWord( ((Tuple*)t.addr)->GetAttribute( index-1 ) );
    qp->Request(args[2].addr, iterWord);
    qp->ReInitResultStorage( args[2].addr );

    ((Tuple*)t.addr)->DeleteIfAllowed();
    qp->Request( args[0].addr, t );
  }

  ((StandardAttribute*)result.addr)->
    CopyFrom( (const StandardAttribute*)iterWord.addr );  
  qp->Close(args[0].addr);

  return 0;
}

/*
2.18.2 Value mapping function of operator ~aggregateB~

The ~aggregateB~ operator uses a stack to compute the aggregation
balanced. This will have advantages in geomatric aggregation.
It may also help to reduce numeric errors in aggregation using
double values.


2.18.2.1 ~StackEntry~

A stack entry consist of the level within the (simulated)
balanced tree and the corresponding value.
Note:
  The attributes at level 0 come directly from the input stream.
  We have to free them using the deleteIfAllowed function.
  On all other levels, the attribute is computes using the
  parameter function. Because this is outside of stream and
  tuples, no reference counting is available and we have to delete
  them using the usual delete function.

*/
struct AggrStackEntry
{
  inline AggrStackEntry(): level(-1),value(0)
  { }

  inline AggrStackEntry( long level, Attribute* value):
  level( level )
  { this->value = value;}

  inline AggrStackEntry( const AggrStackEntry& a ):
  level( a.level )
  { this->value = a.value;}

  inline AggrStackEntry& operator=( const AggrStackEntry& a )
  { level = a.level; value = a.value; return *this; }

  inline ~AggrStackEntry(){ } // use destroy !!


  inline void destroy(){
     if(level<0){
        return;
     }
     if(level==0){ // original from tuple
        value->DeleteIfAllowed();
     } else {
        delete value;
     }
     value = 0;
     level = -1;
  }

  long level;
  Attribute* value;
};

int AggregateB(Word* args, Word& result, int message,
               Word& local, Supplier s)
{
  // The argument vector contains the following values:
  // args[0] = stream of tuples
  // args[1] = attribute name
  // args[2] = mapping function
  // args[3] = zero value
  // args[4] = attribute index added by APPEND

  Word t1,resultWord;
  ArgVectorPointer vector = qp->Argument(args[2].addr);

  qp->Open(args[0].addr);
  result = qp->ResultStorage(s);
  int index = ((CcInt*)args[4].addr)->GetIntval();

  // read the first tuple
  qp->Request( args[0].addr, t1 );


  if( !qp->Received( args[0].addr ) ){
    // special case: stream is empty
    // use the third argument as result
    ((StandardAttribute*)result.addr)->
      CopyFrom( (const StandardAttribute*)args[3].addr );

  } else {
    // nonempty stream, consume it
    stack<AggrStackEntry> theStack;
    while( qp->Received(args[0].addr)){
       // get the attribute from the current tuple
       Attribute* attr = ((Tuple*)t1.addr)->GetAttribute(index-1)->Copy();
       // the tuple is not longer needed here
       ((Tuple*)t1.addr)->DeleteIfAllowed();
       // put the attribute on the stack merging with existing entries
       // while possible
       int level = 0;
       while(!theStack.empty() && level==theStack.top().level){
         // merging is possible
         AggrStackEntry top = theStack.top();
         theStack.pop();
         // call the parameter function
         (*vector)[0] = SetWord(top.value);
         (*vector)[1] = SetWord(attr);
         qp->Request(args[2].addr, resultWord);
         qp->ReInitResultStorage(args[2].addr);
         top.destroy();  // remove stack content
         if(level==0){ // delete attr;
            attr->DeleteIfAllowed();
         } else {
            delete attr;
         }
         attr = (Attribute*) resultWord.addr;
         level++;
       }
       AggrStackEntry entry(level,attr);
       theStack.push(entry);
       qp->Request(args[0].addr,t1);
   }
   // stream ends, merge stack elements regardless of their level
   assert(!theStack.empty()); // at least one element must be exist
   AggrStackEntry tmpResult = theStack.top();
   theStack.pop();
   while(!theStack.empty()){
     AggrStackEntry top = theStack.top();
     theStack.pop();
     (*vector)[0] = SetWord(tmpResult.value);
     (*vector)[1] = SetWord(top.value);
     qp->Request(args[2].addr, resultWord);
     qp->ReInitResultStorage(args[2].addr);
     tmpResult.destroy(); // destroy temporarly result
     tmpResult.level = 1; // mark as computed
     tmpResult.value = (Attribute*) resultWord.addr;
     top.destroy();
   }
   ((StandardAttribute*)result.addr)->
                          CopyFrom((StandardAttribute*)tmpResult.value);
   tmpResult.destroy();
  }
  // close input stream
  qp->Close(args[0].addr);
  return 0;

}



/*
2.18.3 Specification of operator ~aggregate~

*/
const string AggregateSpec  = 
  "( ( \"Signature\" \"Syntax\" \"Meaning\" "
  "\"Example\" ) "
  "( <text>(stream(tuple((a1 t1) ... (an tn))) ai"
  "((ti ti) -> ti) ti -> ti</text--->"
  "<text>_ aggregate [_; fun; _]</text--->"
  "<text>Aggregates the values from the attribute"
  "in the tuple given the function and the initial"
  " value.</text--->"
  "<text>query ten feed aggregate[no; "
  "fun(i1: int, i2: int) i1+i2; 0]</text--->"
  ") )";

/*
2.18.3 Specification of operator ~aggregateB~

*/
const string AggregateBSpec  = 
  "( ( \"Signature\" \"Syntax\" \"Meaning\" "
  "\"Example\" ) "
  "( <text>(stream(tuple((a1 t1) ... (an tn))) ai"
  "((ti ti) -> ti) ti -> ti</text--->"
  "<text>_ aggregateB [_; fun; _]</text--->"
  "<text>Aggregates the values from the attribute"
  "in the tuple given the function and the initial"
  " value in a balanced fashion.</text--->"
  "<text>query ten feed aggregateB[no; "
  "fun(i1: int, i2: int) i1+i2; 0]</text--->"
  ") )";

/*
2.18.4 Definition of operator ~aggregate~

*/
Operator extrelaggregate (
         "aggregate",              // name
         AggregateSpec,            // specification
         Aggregate,                // value mapping
         Operator::SimpleSelect,          // trivial selection function
         AggregateTypeMap          // type mapping
);

/*
2.18.4 Definition of operator ~aggregate\_new~

*/
Operator extrelaggregateB (
         "aggregateB",              // name
         AggregateBSpec,            // specification
         AggregateB,                // value mapping
         Operator::SimpleSelect,          // trivial selection function
         AggregateTypeMap          // type mapping
);

/*

5.10 Operator ~symmjoin~

5.10.1 Type mapping function of operator ~symmjoin~

Result type of symmjoin operation.

----    ((stream (tuple (x1 ... xn))) (stream (tuple (y1 ... ym))))

        -> (stream (tuple (x1 ... xn y1 ... ym)))
----

*/
ListExpr SymmJoinTypeMap(ListExpr args)
{
  ListExpr first, second, third,
           list, list1, list2;
  string argstr, argstr2;

  CHECK_COND(nl->ListLength(args) == 3,
    "Operator symmjoin expects a list of length three.");

  first = nl->First(args);
  second = nl->Second(args);
  third = nl->Third(args);

  nl->WriteToString(argstr, first);
  CHECK_COND(nl->ListLength(first) == 2 &&
             TypeOfRelAlgSymbol(nl->First(first)) == stream &&
             nl->ListLength(nl->Second(first)) == 2 &&
             TypeOfRelAlgSymbol(nl->First(nl->Second(first))) == tuple,
    "Operator symmjoin expects a first list with structure "
    "(stream (tuple ((a1 t1)...(an tn))))\n"
    "Operator symmjoin gets a first list with structure '" + argstr + "'.");

  list1 = nl->Second(nl->Second(first));

  nl->WriteToString(argstr, second);
  CHECK_COND(nl->ListLength(second) == 2 &&
             TypeOfRelAlgSymbol(nl->First(second)) == stream &&
             nl->ListLength(nl->Second(second)) == 2 &&
             TypeOfRelAlgSymbol(nl->First(nl->Second(second))) == tuple,
    "Operator symmjoin expects a second list with structure "
    "(stream (tuple ((a1 t1)...(an tn))))\n"
    "Operator symmjoin gets a second list with structure '" + argstr + "'.");

  nl->WriteToString(argstr, third);
  CHECK_COND(nl->ListLength(third) == 4 &&
             TypeOfRelAlgSymbol(nl->First(third)) == ccmap &&
             TypeOfRelAlgSymbol(nl->Fourth(third)) == ccbool,
    "Operator symmjoin expects a third list with structure "
    "(map (tuple ((a11 t11)...(a1n t1n)))\n"
    "     (tuple ((a21 t21)...(a2n t2n))) bool)\n"
    "Operator symmjoin gets a third list with structure '" + argstr + "'.");

  list2 = nl->Second(nl->Second(second));
  list = ConcatLists(list1, list2);

  nl->WriteToString(argstr, list1);
  nl->WriteToString(argstr2, list2);
  CHECK_COND( CompareNames(list),
              "Operator symmjoin: found doubly "
              "defined attribute names in concatenated list.\n"
              "The first attribute list is '" + argstr + "'\n"
              "and the second is '" + argstr2 + "'\n" );

  return nl->TwoElemList(nl->SymbolAtom("stream"),
           nl->TwoElemList(nl->SymbolAtom("tuple"),
             list));
}
/*

5.10.2 Value mapping function of operator ~symmjoin~

*/

#ifndef USE_PROGRESS

// standard version


struct SymmJoinLocalInfo
{
  TupleType *resultTupleType;

  TupleBuffer *rightRel;
  GenericRelationIterator *rightIter;
  TupleBuffer *leftRel;
  GenericRelationIterator *leftIter;
  bool right;
  Tuple *currTuple;
  bool rightFinished;
  bool leftFinished;
};

int
SymmJoin(Word* args, Word& result, int message, Word& local, Supplier s)
{
  Word r, l;
  SymmJoinLocalInfo* pli;

  switch (message)
  {
    case OPEN :
    {
      long MAX_MEMORY = qp->MemoryAvailableForOperator();
      cmsg.info("ERA:ShowMemInfo") << "SymmJoin.MAX_MEMORY (" 
                                   << MAX_MEMORY/1024 << " MB): " << endl;
      cmsg.send();
      pli = new SymmJoinLocalInfo;
      pli->rightRel = new TupleBuffer( MAX_MEMORY / 2 );
      pli->rightIter = 0;
      pli->leftRel = new TupleBuffer( MAX_MEMORY / 2 );
      pli->leftIter = 0;
      pli->right = true;
      pli->currTuple = 0;
      pli->rightFinished = false;
      pli->leftFinished = false;

      ListExpr resultType = GetTupleResultType( s );
      pli->resultTupleType = new TupleType( nl->Second( resultType ) );

      qp->Open(args[0].addr);
      qp->Open(args[1].addr);

      local = SetWord(pli);
      return 0;
    }
    case REQUEST :
    {
      pli = (SymmJoinLocalInfo*)local.addr;

      while( 1 )
        // This loop will end in some of the returns.
      {
        if( pli->right )
          // Get the tuple from the right stream and match it with the 
          // left stored buffer
        {
          if( pli->currTuple == 0 )
          {
            qp->Request(args[0].addr, r);
            if( qp->Received( args[0].addr ) )
            {
              pli->currTuple = (Tuple*)r.addr;
              pli->leftIter = pli->leftRel->MakeScan();
            }
            else
            {
              pli->rightFinished = true;
              if( pli->leftFinished )
                return CANCEL;
              else
              {
                pli->right = false;
                continue; // Go back to the loop
              }
            }
          }

          // Now we have a tuple from the right stream in currTuple 
          // and an open iterator on the left stored buffer.
          Tuple *leftTuple = pli->leftIter->GetNextTuple();

          if( leftTuple == 0 )
            // There are no more tuples in the left iterator. We then 
            // store the current tuple in the right buffer and close the 
            // left iterator.
          {
            if( !pli->leftFinished )
              // We only need to keep track of the right tuples if the 
              // left stream is not finished.
            {
              pli->rightRel->AppendTuple( pli->currTuple );
              pli->right = false;
            }

            pli->currTuple->DeleteIfAllowed();
            pli->currTuple = 0;

            delete pli->leftIter;
            pli->leftIter = 0;

            continue; // Go back to the loop
          }
          else
            // We match the tuples.
          {
            ArgVectorPointer funArgs = qp->Argument(args[2].addr);
            (*funArgs)[0] = SetWord( pli->currTuple );
            (*funArgs)[1] = SetWord( leftTuple );
            Word funResult;
            qp->Request(args[2].addr, funResult);
            CcBool *boolFunResult = (CcBool*)funResult.addr;

            if( boolFunResult->IsDefined() &&
                boolFunResult->GetBoolval() )
            {
              Tuple *resultTuple = new Tuple( pli->resultTupleType );
              Concat( pli->currTuple, leftTuple, resultTuple );
              leftTuple->DeleteIfAllowed();
              leftTuple = 0;
              result = SetWord( resultTuple );
              return YIELD;
            }
            else
            {
              leftTuple->DeleteIfAllowed();
              leftTuple = 0;
              continue; // Go back to the loop
            }
          }
        }
        else 
          // Get the tuple from the left stream and match it with the 
          // right stored buffer
        {
          if( pli->currTuple == 0 )
          {
            qp->Request(args[1].addr, l);
            if( qp->Received( args[1].addr ) )
            {
              pli->currTuple = (Tuple*)l.addr;
              pli->rightIter = pli->rightRel->MakeScan();
            }
            else
            {
              pli->leftFinished = true;
              if( pli->rightFinished )
                return CANCEL;
              else
              {
                pli->right = true;
                continue; // Go back to the loop
              }
            }
          }

          // Now we have a tuple from the left stream in currTuple and 
          // an open iterator on the right stored buffer.
          Tuple *rightTuple = pli->rightIter->GetNextTuple();

          if( rightTuple == 0 )
            // There are no more tuples in the right iterator. We then 
            // store the current tuple in the left buffer and close 
            // the right iterator.
          {
            if( !pli->rightFinished )
              // We only need to keep track of the left tuples if the 
              // right stream is not finished.
            {
              pli->leftRel->AppendTuple( pli->currTuple );
              pli->right = true;
            }

            pli->currTuple->DeleteIfAllowed();
            pli->currTuple = 0;

            delete pli->rightIter;
            pli->rightIter = 0;

            continue; // Go back to the loop
          }
          else
            // We match the tuples.
          {
            ArgVectorPointer funArgs = qp->Argument(args[2].addr);
            (*funArgs)[0] = SetWord( rightTuple );
            (*funArgs)[1] = SetWord( pli->currTuple );
            Word funResult;
            qp->Request(args[2].addr, funResult);
            CcBool *boolFunResult = (CcBool*)funResult.addr;

            if( boolFunResult->IsDefined() &&
                boolFunResult->GetBoolval() )
            {
              Tuple *resultTuple = new Tuple( pli->resultTupleType );
              Concat( rightTuple, pli->currTuple, resultTuple );
              rightTuple->DeleteIfAllowed();
              rightTuple = 0;
              result = SetWord( resultTuple );
              return YIELD;
            }
            else
            {
              rightTuple->DeleteIfAllowed();
              rightTuple = 0;
              continue; // Go back to the loop
            }
          }
        }
      }
    }
    case CLOSE :
    {
      pli = (SymmJoinLocalInfo*)local.addr;

      if( pli->currTuple != 0 )
        pli->currTuple->DeleteIfAllowed();

      delete pli->leftIter;
      delete pli->rightIter;
      if( pli->resultTupleType != 0 )
        pli->resultTupleType->DeleteIfAllowed();

      if( pli->rightRel != 0 )
      {
        pli->rightRel->Clear();
        delete pli->rightRel;
      }
  
      if( pli->leftRel != 0 )
      {
        pli->leftRel->Clear();
        delete pli->leftRel;
      }

      delete pli;

      qp->Close(args[0].addr);
      qp->Close(args[1].addr);

      return 0;
    }
  }
  return 0;
}


#else

// with support for progress queries

struct SymmJoinLocalInfo
{
  TupleType *resultTupleType;

  TupleBuffer *rightRel;
  GenericRelationIterator *rightIter;
  TupleBuffer *leftRel;
  GenericRelationIterator *leftIter;
  bool right;
  Tuple *currTuple;
  bool rightFinished;
  bool leftFinished;
  int currentL, currentR, returned;
  bool progressInitialized;
  int noAttrs;
  double *attrSize;
  double *attrSizeExt;
};

int
SymmJoin(Word* args, Word& result, int message, Word& local, Supplier s)
{
  Word r, l;
  SymmJoinLocalInfo* pli;

  switch (message)
  {
    case OPEN :
    {
      long MAX_MEMORY = qp->MemoryAvailableForOperator();
      cmsg.info("ERA:ShowMemInfo") << "SymmJoin.MAX_MEMORY ("
                                   << MAX_MEMORY/1024 << " MB): " << endl;
      cmsg.send();
      pli = new SymmJoinLocalInfo;
      pli->rightRel = new TupleBuffer( MAX_MEMORY / 2 );
      pli->rightIter = 0;
      pli->leftRel = new TupleBuffer( MAX_MEMORY / 2 );
      pli->leftIter = 0;
      pli->right = true;
      pli->currTuple = 0;
      pli->rightFinished = false;
      pli->leftFinished = false;

      ListExpr resultType = GetTupleResultType( s );
      pli->resultTupleType = new TupleType( nl->Second( resultType ) );

      qp->Open(args[0].addr);
      qp->Open(args[1].addr);

      pli->currentL = 0;
      pli->currentR = 0;
      pli->returned = 0;
      pli->progressInitialized = false;

      local = SetWord(pli);
      return 0;
    }
    case REQUEST :
    {
      pli = (SymmJoinLocalInfo*)local.addr;

      while( 1 )
        // This loop will end in some of the returns.
      {
        if( pli->right )
          // Get the tuple from the right stream and match it with the
          // left stored buffer
        {
          if( pli->currTuple == 0 )
          {
            qp->Request(args[0].addr, r);
            if( qp->Received( args[0].addr ) )
            {
              pli->currTuple = (Tuple*)r.addr;
              pli->leftIter = pli->leftRel->MakeScan();
              pli->currentL++;
            }
            else
            {
              pli->rightFinished = true;
              if( pli->leftFinished )
                return CANCEL;
              else
              {
                pli->right = false;
                continue; // Go back to the loop
              }
            }
          }

          // Now we have a tuple from the right stream in currTuple
          // and an open iterator on the left stored buffer.
          Tuple *leftTuple = pli->leftIter->GetNextTuple();

          if( leftTuple == 0 )
            // There are no more tuples in the left iterator. We then
            // store the current tuple in the right buffer and close the
            // left iterator.
          {
            if( !pli->leftFinished )
              // We only need to keep track of the right tuples if the
              // left stream is not finished.
            {
              pli->rightRel->AppendTuple( pli->currTuple );
              pli->right = false;
            }

            pli->currTuple->DeleteIfAllowed();
            pli->currTuple = 0;

            delete pli->leftIter;
            pli->leftIter = 0;

            continue; // Go back to the loop
          }
          else
            // We match the tuples.
          {
            ArgVectorPointer funArgs = qp->Argument(args[2].addr);
            (*funArgs)[0] = SetWord( pli->currTuple );
            (*funArgs)[1] = SetWord( leftTuple );
            Word funResult;
            qp->Request(args[2].addr, funResult);
            CcBool *boolFunResult = (CcBool*)funResult.addr;

            if( boolFunResult->IsDefined() &&
                boolFunResult->GetBoolval() )
            {
              Tuple *resultTuple = new Tuple( pli->resultTupleType );
              Concat( pli->currTuple, leftTuple, resultTuple );
              leftTuple->DeleteIfAllowed();
              leftTuple = 0;
              result = SetWord( resultTuple );
              pli->returned++;
              return YIELD;
            }
            else
            {
              leftTuple->DeleteIfAllowed();
              leftTuple = 0;
              continue; // Go back to the loop
            }
          }
        }
        else
          // Get the tuple from the left stream and match it with the
          // right stored buffer
        {
          if( pli->currTuple == 0 )
          {
            qp->Request(args[1].addr, l);
            if( qp->Received( args[1].addr ) )
            {
              pli->currTuple = (Tuple*)l.addr;
              pli->rightIter = pli->rightRel->MakeScan();
              pli->currentR++;
            }
            else
            {
              pli->leftFinished = true;
              if( pli->rightFinished )
                return CANCEL;
              else
              {
                pli->right = true;
                continue; // Go back to the loop
              }
            }
          }

          // Now we have a tuple from the left stream in currTuple and
          // an open iterator on the right stored buffer.
          Tuple *rightTuple = pli->rightIter->GetNextTuple();

          if( rightTuple == 0 )
            // There are no more tuples in the right iterator. We then
            // store the current tuple in the left buffer and close
            // the right iterator.
          {
            if( !pli->rightFinished )
              // We only need to keep track of the left tuples if the
              // right stream is not finished.
            {
              pli->leftRel->AppendTuple( pli->currTuple );
              pli->right = true;
            }

            pli->currTuple->DeleteIfAllowed();
            pli->currTuple = 0;

            delete pli->rightIter;
            pli->rightIter = 0;

            continue; // Go back to the loop
          }
          else
            // We match the tuples.
          {
            ArgVectorPointer funArgs = qp->Argument(args[2].addr);
            (*funArgs)[0] = SetWord( rightTuple );
            (*funArgs)[1] = SetWord( pli->currTuple );
            Word funResult;
            qp->Request(args[2].addr, funResult);
            CcBool *boolFunResult = (CcBool*)funResult.addr;

            if( boolFunResult->IsDefined() &&
                boolFunResult->GetBoolval() )
            {
              Tuple *resultTuple = new Tuple( pli->resultTupleType );
              Concat( rightTuple, pli->currTuple, resultTuple );
              rightTuple->DeleteIfAllowed();
              rightTuple = 0;
              result = SetWord( resultTuple );
              pli->returned++;
              return YIELD;
            }
            else
            {
              rightTuple->DeleteIfAllowed();
              rightTuple = 0;
              continue; // Go back to the loop
            }
          }
        }
      }
    }
    case CLOSE :
    {
      pli = (SymmJoinLocalInfo*)local.addr;

      if( pli->currTuple != 0 )
        pli->currTuple->DeleteIfAllowed();

      delete pli->leftIter;
      delete pli->rightIter;
      if( pli->resultTupleType != 0 )
        pli->resultTupleType->DeleteIfAllowed();

      if( pli->rightRel != 0 )
      {
        pli->rightRel->Clear();
        delete pli->rightRel;
      }

      if( pli->leftRel != 0 )
      {
        pli->leftRel->Clear();
        delete pli->leftRel;
      }

      qp->Close(args[0].addr);
      qp->Close(args[1].addr);

      return 0;
    }

    case PROGRESS :
    {
      ProgressInfo p1, p2;
      ProgressInfo *pRes;
      const double uSymmJoin = 1.0;   	//millisecs per tuple
      const double vSymmJoin = 0.0001;	//millisecs per tuple and byte
      int i;
      pli = (SymmJoinLocalInfo*)local.addr;
      pRes = (ProgressInfo*) result.addr;

      if (!pli) return CANCEL;

      if (qp->RequestProgress(args[0].addr, &p1)
       && qp->RequestProgress(args[1].addr, &p2))
      {
        if ( !pli->progressInitialized )
        {
          pli->noAttrs = p1.noAttrs + p2.noAttrs;
          pli->attrSize = new double[pli->noAttrs];
          pli->attrSizeExt = new double[pli->noAttrs];
          for (i = 0; i < p1.noAttrs; i++)
          {
            pli->attrSize[i] = p1.attrSize[i];
            pli->attrSizeExt[i] = p1.attrSizeExt[i];
          }
          for (int j = 0; j < p2.noAttrs; j++)
          {
            pli->attrSize[j+i] = p2.attrSize[j];
            pli->attrSizeExt[j+i] = p2.attrSizeExt[j];
          }
          pli->progressInitialized = true;
        }

        pRes->Size = p1.Size + p2.Size;
        pRes->SizeExt = p1.SizeExt + p2.SizeExt;
        pRes->noAttrs = pli->noAttrs;
        pRes->attrSize = pli->attrSize;
        pRes->attrSizeExt = pli->attrSizeExt;
        pRes->Time = p1.Time + p2.Time +
          p1.Card * p2.Card * qp->GetPredCost(s) * uSymmJoin;

        pRes->Progress =
          (p1.Progress * p1.Time + p2.Progress * p2.Time +
          pli->currentL * pli->currentR *
          qp->GetPredCost(s) * uSymmJoin)
          / pRes->Time;

        if (pli->returned > 50 ) 	// stable state assumed now
        {
          pRes->Card = p1.Card * p2.Card *
            ((double) pli->returned / (double) (pli->currentL * pli->currentR));
        }
        else
        {
          pRes->Card = p1.Card * p2.Card * qp->GetSelectivity(s);
        }
        return YIELD;
      }
      else
      {
        return CANCEL;
      }
    }
  }
  return 0;
}

#endif



/*

5.10.3 Specification of operator ~symmjoin~

*/
const string SymmJoinSpec  = 
  "( ( \"Signature\" \"Syntax\" \"Meaning\" "
  "\"Example\" ) "
  "( <text>((stream (tuple (x1 ... xn))) (stream "
  "(tuple (y1 ... ym)))) (map (tuple (x1 ... xn)) "
  "(tuple (y1 ... ym)) -> bool) -> (stream (tuple (x1 "
  "... xn y1 ... ym)))</text--->"
  "<text>_ _ symmjoin[ fun ]</text--->"
  "<text>Computes a Cartesian product stream from "
  "its two argument streams filtering by the third "
  "argument.</text--->"
  "<text>query ten feed {a} twenty feed {b} "
  "symmjoin[.no_a = .no_b] count</text--->"
  " ) )";

/*

5.10.4 Definition of operator ~symmjoin~

*/
Operator extrelsymmjoin (
         "symmjoin",            // name
         SymmJoinSpec,          // specification
         SymmJoin,              // value mapping
         Operator::SimpleSelect,         // trivial selection function
         SymmJoinTypeMap        // type mapping
//         true                   // needs large amounts of memory
);



/* 

5.10.5 Operator ~symmproductextend~

02.06.2006 Christian Duentgen

In queries, it is advantageous to avoid the multiple evaluation of common 
subexpressions (CSE). One way is to evaluate a CSE when it is needed for
the first time, and append a new attribute with the CSE's value to the tuple,
using operator ~extend~. When a CSE first appears within a join condition, and
the CSE depends on attributes from both input streams to join, this method
fails. To cope with such situations and optimize the benefit of CSE 
substitution, we implement the ~symmproductextend~ operator.

The operator has three arguments, the first and second being the input tuple 
streams. The third argument is a function list (i.e. a list of pairs 
(attributename function), where ~attributename~ is a new identifier and 
~function~ is a function calculating the new attribute's value from a pair
of tuples, one taken from each input tuple stream. 

Subsequent to a ~symmproductextend~, a ~filter~ can be applied, e.g. to express
a ``join condition'' using the attributes extended (representing CSEs).

The operator works similar to 

---- stream(tuple(X)) stream(tuple(Y)) symmjoin[TRUE] extend[ funlist ].
----

5.10.5.1 Typemapping for operator ~symmproductextend~

----
    (stream (tuple (A))) 
  x (stream (tuple (B))) 
  x ( ( c1 (map tuple(A) tuple(B) tc1))
          ...

      ( ck (map tuple(A) tuple(B) tck)))
 -> (stream (tuple(A*B*C)))

where A = (ta1 a1) ... (tan an)
      B = (tb1 b1) ... (tbm bm)
      C = (tc1 c1) ... (tck ck)
----

*/

/*
Typemapping operator for operator ~symmproductextend~

*/

ListExpr SymmProductExtendTypeMap(ListExpr args)
{
  ListExpr first, second, third, 
           lastlistn, listn, rest, errorInfo,
           first2, second2, firstr;
  string   argstr, argstr2;

  errorInfo = nl->OneElemList(nl->SymbolAtom("ERROR"));

  CHECK_COND(nl->ListLength(args) == 3,
    "Operator symmproduxtextend expects a list of length three.");

  first = nl->First(args);
  second = nl->Second(args);
  third = nl->Third(args);

  nl->WriteToString(argstr, first);
  CHECK_COND(nl->ListLength(first) == 2 &&
             TypeOfRelAlgSymbol(nl->First(first)) == stream &&
             nl->ListLength(nl->Second(first)) == 2 &&
             TypeOfRelAlgSymbol(nl->First(nl->Second(first))) == tuple,
    "Operator symmproductextend expects a first list with structure "
    "(stream (tuple ((a1 t1)...(an tn))))\n"
    "Operator symmproductextend gets a first list with structure '" 
    + argstr + "'.");

  nl->WriteToString(argstr, second);
  CHECK_COND(nl->ListLength(second) == 2 &&
             TypeOfRelAlgSymbol(nl->First(second)) == stream &&
             nl->ListLength(nl->Second(second)) == 2 &&
             TypeOfRelAlgSymbol(nl->First(nl->Second(second))) == tuple,
    "Operator symmproductextend expects a second list with structure "
    "(stream (tuple ((a1 t1)...(an tn))))\n"
    "Operator symmproductextend gets a second list with structure '" 
    + argstr + "'.");

  CHECK_COND(!(nl->IsAtom(third)) &&
             (nl->ListLength(third) > 0),
    "Operator symmproductextend: Third argument list may not "
    "be empty or an atom" );

  // copy first arg's attributes
  rest = nl->Second(nl->Second(first));
  listn = nl->OneElemList(nl->First(rest));
  lastlistn = listn;
  rest = nl->Rest(rest);
  while (!(nl->IsEmpty(rest)))
  {
     lastlistn = nl->Append(lastlistn,nl->First(rest));
     rest = nl->Rest(rest);
  }

  // copy second arg's attributes
  rest = nl->Second(nl->Second(second));
  while (!(nl->IsEmpty(rest)))
  {
     lastlistn = nl->Append(lastlistn,nl->First(rest));
     rest = nl->Rest(rest);
  }

  // handle list of new attributes and mapping functions (3rd argument)
  rest = third;
  while (!(nl->IsEmpty(rest))) // for all new attrs
  {
    firstr = nl->First(rest);
    rest = nl->Rest(rest);
    first2 = nl->First(firstr);
    second2 = nl->Second(firstr);

    // check new attribute name
    nl->WriteToString(argstr, first2);
    CHECK_COND( (nl->IsAtom(first2)) &&
                (nl->AtomType(first2) == SymbolType),
      "Operator symmproductextend: Attribute name '" + argstr +
      "' is not an atom or not of type SymbolType" );

    // check mapping function for new attribute
    nl->WriteToString(argstr, second2);
    CHECK_COND( (nl->ListLength(second2) == 4) &&
                (TypeOfRelAlgSymbol(nl->First(second2)) == ccmap) &&
                (am->CheckKind("DATA", nl->Fourth(second2), errorInfo)),
      "Operator symmproductextend expects a mapping function with list "
      "structure (<attrname> (map (tuple ( tuple(X) tuple(Y) )) ti) ).\n"
      "Operator symmproductextend gets a list '" + argstr + "'.\n" 
      "The fourth list element ('ti') for such a 'ccmap' must "
      "be of kind 'DATA'.\n" );

    // Do typechecking for first extension mapping function argument:
    nl->WriteToString(argstr, nl->Second(second2));
    nl->WriteToString(argstr2, nl->Second(first));
    CHECK_COND( (nl->Equal(nl->Second((first)),nl->Second(second2))),
      "Operator symmproductextend: The first argument tuple type '" + argstr +
      "' in a mapping function is wrong. It should be '" + argstr2 + "'.\n" );

    // Do typechecking for second extension mapping function argument:
    nl->WriteToString(argstr, nl->Third(second2));
    nl->WriteToString(argstr2, nl->Second(second));
    CHECK_COND( (nl->Equal(nl->Second(second),nl->Third(second2))),
      "Operator symmproductextend: The second argument tuple type '" + argstr +
      "' in a mapping function is wrong. It should be '" + argstr2 + "'.\n" );

    // append new attribute (name type)
    lastlistn = nl->Append(lastlistn,
        (nl->TwoElemList(first2,nl->Fourth(second2))));
  }

  nl->WriteToString(argstr, listn);
  CHECK_COND( CompareNames(listn),
              "Operator symmproductextend: found doubly "
              "defined attribute names in concatenated list.\n"
              "The list is '" + argstr + "'\n" );

  return nl->TwoElemList(nl->SymbolAtom("stream"),
           nl->TwoElemList(nl->SymbolAtom("tuple"),
             listn));
}


/*
5.10.5.2 Value mapping function of operator ~symmproductextend~

*/
struct SymmProductExtendLocalInfo
{
  TupleType *resultTupleType;
  TupleBuffer *rightRel;
  GenericRelationIterator *rightIter;
  TupleBuffer *leftRel;
  GenericRelationIterator *leftIter;
  bool right;
  Tuple *currTuple;
  bool rightFinished;
  bool leftFinished;
};

int
SymmProductExtend(Word* args, Word& result,
                              int message, Word& local, Supplier s)
{
  Word r, l, value;
  SymmProductExtendLocalInfo* pli;
  int i, nooffun, noofsons;
  Supplier supplier, supplier2, supplier3;
  ArgVectorPointer extFunArgs;
  Tuple *resultTuple;

  switch (message)
  {
    case OPEN :
    {
      long MAX_MEMORY = qp->MemoryAvailableForOperator();
      cmsg.info("ERA:ShowMemInfo") << "SymmProductExtend.MAX_MEMORY (" 
                                   << MAX_MEMORY/1024 << " MB): " << endl;
      cmsg.send();
      pli = new SymmProductExtendLocalInfo;
      pli->rightRel = new TupleBuffer( MAX_MEMORY / 2 );
      pli->rightIter = 0;
      pli->leftRel = new TupleBuffer( MAX_MEMORY / 2 );
      pli->leftIter = 0;
      pli->right = true;
      pli->currTuple = 0;
      pli->rightFinished = false;
      pli->leftFinished = false;

      ListExpr resultType = GetTupleResultType( s );
      pli->resultTupleType = new TupleType( nl->Second( resultType ) );
      qp->Open(args[0].addr);
      qp->Open(args[1].addr);

      local = SetWord(pli);
      return 0;
    }
    case REQUEST :
    {
      pli = (SymmProductExtendLocalInfo*)local.addr;

      while( 1 )
        // This loop will end in some of the returns.
      {
        if( pli->right )
          // Get the tuple from the right stream and match it with the 
          // left stored buffer
        {
          if( pli->currTuple == 0 )
          {
            qp->Request(args[1].addr, r);
            if( qp->Received( args[1].addr ) )
            {
              pli->currTuple = (Tuple*)r.addr;
              pli->leftIter = pli->leftRel->MakeScan();
            }
            else
            {
              pli->rightFinished = true;
              if( pli->leftFinished )
                return CANCEL;
              else
              {
                pli->right = false;
                continue; // Go back to the loop
              }
            }
          }

          // Now we have a tuple from the right stream in currTuple 
          // and an open iterator on the left stored buffer.
          Tuple *leftTuple = pli->leftIter->GetNextTuple();

          if( leftTuple == 0 )
            // There are no more tuples in the left iterator. We then 
            // store the current tuple in the right buffer and close the 
            // left iterator.
          {
            if( !pli->leftFinished )
              // We only need to keep track of the right tuples if the 
              // left stream is not finished.
            {
              pli->rightRel->AppendTuple( pli->currTuple );
              pli->right = false;
            }

            pli->currTuple->DeleteIfAllowed();
            pli->currTuple = 0;

            delete pli->leftIter;
            pli->leftIter = 0;

            continue; // Go back to the loop
          }
          else
            // We match the tuples.
          {
            // XRIS: We must calculate the new attributes' values here
            // and extend currTuple with them

            resultTuple = new Tuple( pli->resultTupleType );
            Concat( leftTuple, pli->currTuple, resultTuple );

            supplier = args[2].addr;           // get list of ext-functions
            nooffun = qp->GetNoSons(supplier); // get num of ext-functions
            for(i = 0; i< nooffun; i++)
            {
              supplier2 = qp->GetSupplier(supplier, i); // get an ext-function
              noofsons = qp->GetNoSons(supplier2);
              supplier3 = qp->GetSupplier(supplier2, 1);
              extFunArgs = qp->Argument(supplier3);
              (*extFunArgs)[0] = SetWord(leftTuple);     // pass first argument
              (*extFunArgs)[1] = SetWord(pli->currTuple);// pass second argument
              qp->Request(supplier3,value);              // call extattr mapping
//  The original implementation tried to avoid copying the function result,
//  but somehow, this results in a strongly growing tuplebuffer on disk:
//               resultTuple->PutAttribute( 
//                 leftTuple->GetNoAttributes()
//                 + pli->currTuple->GetNoAttributes()
//                 + i, (StandardAttribute*)value.addr);
//               qp->ReInitResultStorage( supplier3 );
              resultTuple->PutAttribute( 
                leftTuple->GetNoAttributes()
                + pli->currTuple->GetNoAttributes()
                + i, ((StandardAttribute*)value.addr)->Clone() );
              qp->ReInitResultStorage( supplier3 );
            }
            leftTuple->DeleteIfAllowed();
            leftTuple = 0;
            result = SetWord( resultTuple );
            return YIELD;
          }
        }
        else 
          // Get the tuple from the left stream and match it with the 
          // right stored buffer
        {
          if( pli->currTuple == 0 )
          {
            qp->Request(args[0].addr, l);
            if( qp->Received( args[0].addr ) )
            {
              pli->currTuple = (Tuple*)l.addr;
              pli->rightIter = pli->rightRel->MakeScan();
            }
            else
            {
              pli->leftFinished = true;
              if( pli->rightFinished )
                return CANCEL;
              else
              {
                pli->right = true;
                continue; // Go back to the loop
              }
            }
          }

          // Now we have a tuple from the left stream in currTuple and 
          // an open iterator on the right stored buffer.
          Tuple *rightTuple = pli->rightIter->GetNextTuple();

          if( rightTuple == 0 )
            // There are no more tuples in the right iterator. We then 
            // store the current tuple in the left buffer and close 
            // the right iterator.
          {
            if( !pli->rightFinished )
              // We only need to keep track of the left tuples if the 
              // right stream is not finished.
            {
              pli->leftRel->AppendTuple( pli->currTuple );
              pli->right = true;
            }

            pli->currTuple->DeleteIfAllowed();
            pli->currTuple = 0;

            delete pli->rightIter;
            pli->rightIter = 0;

            continue; // Go back to the loop
          }
          else
            // We match the tuples.
          {
            // XRIS: We must calculate the new attribute's values here
            // and extend rightTuple
            resultTuple = new Tuple( pli->resultTupleType );
            Concat( rightTuple, pli->currTuple, resultTuple );
            supplier = args[2].addr;           // get list of ext-functions
            nooffun = qp->GetNoSons(supplier); // get num of ext-functions
            for(i = 0; i< nooffun; i++)
            {
              supplier2 = qp->GetSupplier(supplier, i); // get an ext-function
              noofsons = qp->GetNoSons(supplier2);
              supplier3 = qp->GetSupplier(supplier2, 1);
              extFunArgs = qp->Argument(supplier3);
              (*extFunArgs)[0] = SetWord(pli->currTuple);// pass 1st argument
              (*extFunArgs)[1] = SetWord(rightTuple);    // pass 2nd argument
              qp->Request(supplier3,value);              // call extattr mapping
//  The original implementation tried to avoid copying the function result,
//  but somehow, this results in a strongly growing tuplebuffer on disk:
//               resultTuple->PutAttribute( 
//                 pli->currTuple->GetNoAttributes()
//                 + rightTuple->GetNoAttributes()
//                 + i, (StandardAttribute*)value.addr);
//               // extend effective left tuple
//               qp->ReInitResultStorage( supplier3 );

              resultTuple->PutAttribute( 
                pli->currTuple->GetNoAttributes()
                + rightTuple->GetNoAttributes()
                + i, ((StandardAttribute*)value.addr)->Clone() );
              // extend effective left tuple
            }
            rightTuple->DeleteIfAllowed();
            rightTuple = 0;
            result = SetWord( resultTuple );
            return YIELD;
          }
        }
      }
    }
    case CLOSE :
    {
      pli = (SymmProductExtendLocalInfo*)local.addr;

      if( pli->currTuple != 0 )
        pli->currTuple->DeleteIfAllowed();

      delete pli->leftIter;
      delete pli->rightIter;
      if( pli->resultTupleType != 0 )
        pli->resultTupleType->DeleteIfAllowed();

      if( pli->rightRel != 0 )
      {
        pli->rightRel->Clear();
        delete pli->rightRel;
      }
  
      if( pli->leftRel != 0 )
      {
        pli->leftRel->Clear();
        delete pli->leftRel;
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

5.10.5.3 Specification of operator ~symmproductextend~

*/
const string SymmProductExtendSpec  = 
  "( ( \"Signature\" \"Syntax\" \"Meaning\" "
  "\"Example\" ) "
  "( <text>(stream (tuple(X))) (stream (tuple(Y))) "
  " [(z1, (tuple(X) tuple(Y) -> t1)) "
  "... (zj, (tuple(X) tuple(Y) -> tj))]  -> (stream (tuple(X*Y*Z))) "
  "))</text--->"
  "<text>_ _ symmproductextend[ funlist ]</text--->"
  "<text>Computes a Cartesian product stream from "
  "its two argument streams, extending it with "
  "new attributes respecting mapping rules passed as "
  "third argument.</text--->"
  "<text>query ten feed {a} ten feed {b} "
  "symmproductextend[ [prod: (.no_a * ..no_b)] ] "
  "count</text--->"
  " ) )";

/*

5.10.5.4 Definition of operator ~symmproductextend~

*/
Operator extrelsymmproductextend (
         "symmproductextend",     // name
         SymmProductExtendSpec,   // specification
         SymmProductExtend,       // value mapping
         Operator::SimpleSelect,  // trivial selection function
         SymmProductExtendTypeMap // type mapping
//         true                   // needs large amounts of memory
);


/*

5.10.6 Operator ~symmproduct~

This operator calculates the cartesian product of two tuplestreams in a symmetrical and
non-blocking manner. It behaves like 

---- _ _ symmjoin [ TRUE ]
----
but does not evaluate a predictate. 

5.10.6.1 Typemapping for operator ~symmproduct~

----
    (stream (tuple (A))) 
  x (stream (tuple (B))) 
 -> (stream (tuple(A*B)))

where A = (ta1 a1) ... (tan an)
      B = (tb1 b1) ... (tbm bm)
----

*/

/*
Typemapping operator for operator ~symmproduct~

*/

ListExpr SymmProductTypeMap(ListExpr args)
{
  ListExpr first, second,
           list, list1, list2;
  string argstr, argstr2;

  CHECK_COND(nl->ListLength(args) == 2,
    "Operator symmproduct expects a list of length two.");

  first = nl->First(args);
  second = nl->Second(args);

  nl->WriteToString(argstr, first);
  CHECK_COND(nl->ListLength(first) == 2 &&
             TypeOfRelAlgSymbol(nl->First(first)) == stream &&
             nl->ListLength(nl->Second(first)) == 2 &&
             TypeOfRelAlgSymbol(nl->First(nl->Second(first))) == tuple,
    "Operator symmproduct expects a first list with structure "
    "(stream (tuple ((a1 t1)...(an tn))))\n"
    "Operator symmproduct gets a first list with structure '" + argstr + "'.");

  list1 = nl->Second(nl->Second(first));

  nl->WriteToString(argstr, second);
  CHECK_COND(nl->ListLength(second) == 2 &&
             TypeOfRelAlgSymbol(nl->First(second)) == stream &&
             nl->ListLength(nl->Second(second)) == 2 &&
             TypeOfRelAlgSymbol(nl->First(nl->Second(second))) == tuple,
    "Operator symmproduct expects a second list with structure "
    "(stream (tuple ((a1 t1)...(an tn))))\n"
    "Operator symmproduct gets a second list with structure '" + argstr + "'.");

  list2 = nl->Second(nl->Second(second));
  list = ConcatLists(list1, list2);

  nl->WriteToString(argstr, list1);
  nl->WriteToString(argstr2, list2);
  CHECK_COND( CompareNames(list),
              "Operator symmproduct: found doubly "
              "defined attribute names in concatenated list.\n"
              "The first attribute list is '" + argstr + "'\n"
              "and the second is '" + argstr2 + "'\n" );

  return nl->TwoElemList(nl->SymbolAtom("stream"),
           nl->TwoElemList(nl->SymbolAtom("tuple"),
             list));
}

/*
5.10.5.2 Value mapping function of operator ~symmproduct~

*/

struct SymmProductLocalInfo
{
  TupleType *resultTupleType;

  TupleBuffer *rightRel;
  GenericRelationIterator *rightIter;
  TupleBuffer *leftRel;
  GenericRelationIterator *leftIter;
  bool right;
  Tuple *currTuple;
  bool rightFinished;
  bool leftFinished;
};

int
SymmProduct(Word* args, Word& result, int message, Word& local, Supplier s)
{
  Word r, l;
  SymmProductLocalInfo* pli;

  switch (message)
  {
    case OPEN :
    {
      long MAX_MEMORY = qp->MemoryAvailableForOperator();
      cmsg.info("ERA:ShowMemInfo") << "SymmProduct.MAX_MEMORY (" 
                                   << MAX_MEMORY/1024 << " MB): " << endl;
      cmsg.send();
      pli = new SymmProductLocalInfo;
      pli->rightRel = new TupleBuffer( MAX_MEMORY / 2 );
      pli->rightIter = 0;
      pli->leftRel = new TupleBuffer( MAX_MEMORY / 2 );
      pli->leftIter = 0;
      pli->right = true;
      pli->currTuple = 0;
      pli->rightFinished = false;
      pli->leftFinished = false;

      ListExpr resultType = GetTupleResultType( s );
      pli->resultTupleType = new TupleType( nl->Second( resultType ) );

      qp->Open(args[0].addr);
      qp->Open(args[1].addr);

      local = SetWord(pli);
      return 0;
    }
    case REQUEST :
    {
      pli = (SymmProductLocalInfo*)local.addr;

      while( 1 )
        // This loop will end in some of the returns.
      {
        if( pli->right )
          // Get the tuple from the right stream and match it with the 
          // left stored buffer
        {
          if( pli->currTuple == 0 )
          {
            qp->Request(args[0].addr, r);
            if( qp->Received( args[0].addr ) )
            {
              pli->currTuple = (Tuple*)r.addr;
              pli->leftIter = pli->leftRel->MakeScan();
            }
            else
            {
              pli->rightFinished = true;
              if( pli->leftFinished )
                return CANCEL;
              else
              {
                pli->right = false;
                continue; // Go back to the loop
              }
            }
          }

          // Now we have a tuple from the right stream in currTuple 
          // and an open iterator on the left stored buffer.
          Tuple *leftTuple = pli->leftIter->GetNextTuple();

          if( leftTuple == 0 )
            // There are no more tuples in the left iterator. We then 
            // store the current tuple in the right buffer and close the 
            // left iterator.
          {
            if( !pli->leftFinished )
              // We only need to keep track of the right tuples if the 
              // left stream is not finished.
            {
              pli->rightRel->AppendTuple( pli->currTuple );
              pli->right = false;
            }

            pli->currTuple->DeleteIfAllowed();
            pli->currTuple = 0;

            delete pli->leftIter;
            pli->leftIter = 0;

            continue; // Go back to the loop
          }
          else
            // We match the tuples.
          {
            Tuple *resultTuple = new Tuple( pli->resultTupleType );
            Concat( pli->currTuple, leftTuple, resultTuple );
            leftTuple->DeleteIfAllowed();
            leftTuple = 0;
            result = SetWord( resultTuple );
            return YIELD;
          }
        }
        else 
          // Get the tuple from the left stream and match it with the 
          // right stored buffer
        {
          if( pli->currTuple == 0 )
          {
            qp->Request(args[1].addr, l);
            if( qp->Received( args[1].addr ) )
            {
              pli->currTuple = (Tuple*)l.addr;
              pli->rightIter = pli->rightRel->MakeScan();
            }
            else
            {
              pli->leftFinished = true;
              if( pli->rightFinished )
                return CANCEL;
              else
              {
                pli->right = true;
                continue; // Go back to the loop
              }
            }
          }

          // Now we have a tuple from the left stream in currTuple and 
          // an open iterator on the right stored buffer.
          Tuple *rightTuple = pli->rightIter->GetNextTuple();

          if( rightTuple == 0 )
            // There are no more tuples in the right iterator. We then 
            // store the current tuple in the left buffer and close 
            // the right iterator.
          {
            if( !pli->rightFinished )
              // We only need to keep track of the left tuples if the 
              // right stream is not finished.
            {
              pli->leftRel->AppendTuple( pli->currTuple );
              pli->right = true;
            }

            pli->currTuple->DeleteIfAllowed();
            pli->currTuple = 0;

            delete pli->rightIter;
            pli->rightIter = 0;

            continue; // Go back to the loop
          }
          else
            // We match the tuples.
          {
            Tuple *resultTuple = new Tuple( pli->resultTupleType );
            Concat( rightTuple, pli->currTuple, resultTuple );
            rightTuple->DeleteIfAllowed();
            rightTuple = 0;
            result = SetWord( resultTuple );
            return YIELD;
          }
        }
      }
    }
    case CLOSE :
    {
      pli = (SymmProductLocalInfo*)local.addr;

      if( pli->currTuple != 0 )
        pli->currTuple->DeleteIfAllowed();

      delete pli->leftIter;
      delete pli->rightIter;
      if( pli->resultTupleType != 0 )
        pli->resultTupleType->DeleteIfAllowed();

      if( pli->rightRel != 0 )
      {
        pli->rightRel->Clear();
        delete pli->rightRel;
      }
  
      if( pli->leftRel != 0 )
      {
        pli->leftRel->Clear();
        delete pli->leftRel;
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

5.10.6.3 Specification of operator ~symmproduct~

*/
const string SymmProductSpec  = 
  "( ( \"Signature\" \"Syntax\" \"Meaning\" "
  "\"Example\" ) "
  "( <text>(stream (tuple(X))) (stream (tuple(Y))) "
  " -> (stream (tuple(X*Y))) "
  "))</text--->"
  "<text>_ _ symmproduct</text--->"
  "<text>Computes a Cartesian product stream from "
  "its two argument streams, in a symmetrical and "
  "non-blocking way.</text--->"
  "<text>query ten feed {a} ten feed {b} "
  "symmproduct count</text--->"
  " ) )";

/*

5.10.6.4 Definition of operator ~symmproduct~

*/
Operator extrelsymmproduct (
         "symmproduct",           // name
         SymmProductSpec,         // specification
         SymmProduct,             // value mapping
         Operator::SimpleSelect,  // trivial selection function
         SymmProductTypeMap       // type mapping
//         true                   // needs large amounts of memory
);


/*

3 Class ~ExtRelationAlgebra~

A new subclass ~ExtRelationAlgebra~ of class ~Algebra~ is declared. The only
specialization with respect to class ~Algebra~ takes place within the
constructor: all type constructors and operators are registered at the 
actual algebra.

After declaring the new class, its only instance ~extendedRelationAlgebra~ 
is defined.

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
    AddOperator(&extrelhashjoin);
    AddOperator(&extrelloopjoin);

#ifdef USE_PROGRESS 
    extrelloopjoin.EnableProgress();
#endif

    AddOperator(&extrelextendstream);
    AddOperator(&extrelprojectextendstream);
    AddOperator(&extrelloopsel);
    AddOperator(&extrelgroupby);
    AddOperator(&extrelaggregate);
    AddOperator(&extrelaggregateB);
    AddOperator(&extrelsymmjoin);

#ifdef USE_PROGRESS
    extrelsymmjoin.EnableProgress();
#endif

    AddOperator(&extrelsymmproductextend);
    AddOperator(&extrelsymmproduct);
    AddOperator(&extrelprojectextend);
    AddOperator(&krdup);
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
InitializeExtRelationAlgebra( NestedList* nlRef, 
                              QueryProcessor* qpRef,
                              AlgebraManager* amRef )
{
  nl = nlRef;
  qp = qpRef;
  am = amRef;
  return (&extRelationalgebra);
}

