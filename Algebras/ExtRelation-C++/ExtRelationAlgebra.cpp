/*
----
This file is part of SECONDO.

Copyright (C) 2004-2008, University in Hagen, Faculty of Mathematics and
Computer Science, Database Systems for New Applications.

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
//[_] [\_]
//[x] [\ensuremath{\times}]
//[->] [\ensuremath{\rightarrow}]
//[>] [\ensuremath{>}]
//[<] [\ensuremath{<}]



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

June 2006, Corrected a bug caused by improper reference counting of tuples
observed in operator ~mergesec~.

June 2006, Christian D[ue]ntgen added operators ~symmproduct~ and
~symmproductextend~.

August 2006, Christian D[ue]ntgen added signature ((stream T) int) -> (stream T)
to operator ~head~.

January 2007, M. Spiekermann. Reference counting in groupby corrected, since it
causes a segmentation fault, when the Tuplebuffer needs to be flushed on disk.

July 2007, C. Duentgen. Changed groupbyTypeMap. It will now accept an empty
list of grouping attributes (argument 1)). The result will then be constructed
from a single group containing ALL tuples from the input stream (argument 0).
Logically, the result tuples contain no original attributes from the input
stream, but only those created by aggregation functions from argument 2.

October 2007, M. Spiekermann, T. Behr. Reimplementation of the operators ~avg~
and ~sum~ in order to provide better example code since these should be used as
examples for the practical course.

January 2008, C. D[ue]ntgen adds aggregation operator ~var~, computing the
variance on a stream.

[TOC]

1 Includes and defines

*/

#include <vector>
#include <deque>
#include <sstream>
#include <stack>
#include <limits.h>
#include <set>

//#define TRACE_ON
#undef TRACE_ON
#include "LogMsg.h"
#define TRACE_OFF

#include "RelationAlgebra.h"
#include "QueryProcessor.h"
#include "AlgebraManager.h"
#include "CPUTimeMeasurer.h"
#include "StandardTypes.h"
#include "Counter.h"
#include "TupleIdentifier.h"
#include "Progress.h"
#include "RTuple.h"
#include "Symbols.h"
#include "ListUtils.h"
#include "Outerjoin.h"
#include "DateTime.h"
#include "Stream.h"

extern NestedList* nl;
extern QueryProcessor* qp;
extern AlgebraManager* am;
extern Operator extrelsmouterjoin;
extern Operator extrelsymmouterjoin;

using namespace listutils;

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
  if(nl->ListLength(args)<1){
     ErrorReporter::ReportError("one argument expected");
     return nl->TypeError();
  }

  ListExpr first = nl->First(args);

  if(!Stream<Tuple>::checkType(first)){
    return listutils::typeError("tuple stream expected");
  }

  return nl->TwoElemList(
          nl->SymbolAtom(Relation::BasicType()),
          nl->Second(first));
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

If the optional parameter ~subsetSize~ is set to ~true~, the optional parameter
~randSeed~ (defaults to 0) is used as the starting sequence offset for the
random
number generator. Otherwise, the offset is calculated from the current time and
will differ for each call.

*/
void
MakeRandomSubset(vector<int>& result, int subsetSize, int setSize,
     bool useSeed = false, unsigned int randSeed = 1)
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
  // will avoid to return the same sequence of numbers,
  // unless useSeed is true and both calls provide the
  // same randSeed.
  static unsigned int randCalls = (time(0) % 1000) * 1000;
  // For some experiments, one would like to get "known" samples.
  // To that end, useSeed can be set to true and an explicit random seed can be
  // passed.
  // To allow for "real" random sequences after such a seeding, we need to
  // remember the fact whether the current randCalls is due to an explicit
  // seed or not (otherwise the sample-result would be random)
  static bool lastWasSeeded = useSeed;
  if(useSeed){
    randCalls = randSeed;
    lastWasSeeded = true; // time-based seed overwritten!
  } else {
    if(lastWasSeeded){
      randCalls = (time(0) % 1000) * 1000; // re-init seed
    }
    lastWasSeeded = false;
  }
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
  int drawMax = 3*(RAND_MAX/4);
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

----  ((rel x) int real [int] )    -> (stream x)
----

*/
ListExpr SampleTypeMap(ListExpr args)
{

  int len = nl->ListLength(args);
  if(len!=3 && len!=4){
    ErrorReporter::ReportError("three or four arguments expected");
    return nl->TypeError();
  }

  ListExpr rel = nl->First(args);
  ListExpr minSampleSize = nl->Second(args);
  ListExpr minSampleRate = nl->Third(args);

  if( !Relation::checkType(rel) ||
      !CcInt::checkType(minSampleSize)  ||
      !CcReal::checkType(minSampleRate)){
    ErrorReporter::ReportError("rel x int x real [ x int] expected");
    return nl->TypeError();
  }

  ListExpr streamDescription =
          nl->Cons(nl->SymbolAtom(Symbol::STREAM()), nl->Rest(rel));

  if(len==4){
    ListExpr randSeed = nl->Fourth(args);
    if(!CcInt::checkType(randSeed)){
      ErrorReporter::ReportError("rel x int x real [ x int] expected");
      return nl->TypeError();
    }
    return nl->ThreeElemList(nl->SymbolAtom(Symbol::APPEND()),
                             nl->OneElemList(nl->BoolAtom(true)),
                             streamDescription);
  }

  return nl->ThreeElemList(nl->SymbolAtom(Symbol::APPEND()),
                           nl->TwoElemList(nl->IntAtom(0),
                                           nl->BoolAtom(false)),
                           streamDescription);
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
  bool useSeed;
  unsigned int randSeed;
};

int Sample(Word* args, Word& result, int message, Word& local, Supplier s)
{
  SampleLocalInfo* localInfo = static_cast<SampleLocalInfo*>( local.addr );

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

      localInfo->randSeed = (unsigned int)((CcInt*)(args[3].addr))->GetIntval();
      localInfo->useSeed = ((CcBool*)(args[4].addr))->GetBoolval();

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
        MakeRandomSubset(localInfo->sampleIndices, sampleSize, relSize,
                         localInfo->useSeed, localInfo->randSeed);
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

        result.setAddr(tuple);
        localInfo->lastIndex = *(localInfo->iter);
        localInfo->iter++;
        return YIELD;
      }

    case CLOSE :
      if(local.addr){
         localInfo = (SampleLocalInfo*)local.addr;
         delete localInfo->relIter;
         delete localInfo;
         local.setAddr(0);
      }
      return 0;
  }
  return 0;
}
/*
2.4.4 Specification of operator ~sample~

*/
const string SampleSpec  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                           "\"Example\" ) "
                           "( <text>rel(X) int x real [x int ] "
                           "-> (stream x)"
                           "</text--->"
                           "<text>_ sample [ Size , Fraction , Seed ]</text--->"
                           "<text>Produces a random sample of a relation."
                           " The sample size is min(relSize, "
                           "max(s, t * relSize)), where relSize is the size"
                           " of the argument relation, s is Size "
                           "argument, and t is Fraction. The optional fourth "
                           "parameter selects a seed for the used random "
                           "number generator."
                           "The sample has the same ordering as the original "
                           "relation. </text--->"
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
  if(nl->ListLength(args)!=2){
    return listutils::typeError("two arguments expected");
  }
  ListExpr stream = nl->First(args);
  ListExpr fun = nl->Second(args);

  string err = " stream(tuple(x)) x (tuple(x) -> bool) expected";
  if(!listutils::isTupleStream(stream) ||
     !listutils::isMap<1>(fun)){
    return listutils::typeError(err);
  }

  if(!listutils::isSymbol(nl->Third(fun),CcBool::BasicType())){
    return listutils::typeError(err);
  }
  if(!nl->Equal(nl->Second(stream),nl->Second(fun))){
    return listutils::typeError(err);
  }
  return stream;
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
          result.setAddr(tuple);
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

  if(nl->ListLength(args)!=2){
   return listutils::typeError("two arguments expected");
  }

  ListExpr stream = nl->First(args);
  ListExpr attrname = nl->Second(args);
  string err = "(stream( tuple[ a1 : t1, .., an : tn ])) x a_i expected";

  if(!listutils::isTupleStream(stream) ||
     nl->AtomType(attrname)!=SymbolType){
    return listutils::typeError(err);
  }

  string attr = nl->SymbolValue(attrname);
  ListExpr attrList = nl->Second(nl->Second(stream));
  ListExpr attrType;
  int j = listutils::findAttribute(attrList, attr, attrType);
  if (j) {
    if (nl->ListLength(attrType) == 2 && nl->SymbolValue
          (nl->First(attrType)) == "arel"){
      ErrorReporter::ReportError("Standard extract is not defined for arel");
      return nl->TypeError();
    }
    else{
      return nl->ThreeElemList(nl->SymbolAtom(Symbol::APPEND()),
           nl->OneElemList(nl->IntAtom(j)), attrType);
    }
  } else {
    return listutils::typeError("Attribute name " + attr +
                                " not known in the tuple");
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
  int index;
  Attribute* res = (Attribute*)((qp->ResultStorage(s)).addr);
  result.setAddr(res);

  Stream<Tuple> stream(args[0]);
  stream.open();
  Tuple* tuple = stream.request(); 

  if(tuple) {
    index = ((CcInt*)args[2].addr)->GetIntval();
    res->CopyFrom(
      (const Attribute*)tuple->GetAttribute(index - 1));
    tuple->DeleteIfAllowed();
  }
  else
  {
    res->SetDefined(false);
  }

  stream.close();
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

  if(nl->ListLength(args)!=2){
    return listutils::typeError("two arguments expected");
  }

  string err = " stream(tuple(...) x int or stream(DATA) x int expected";

  ListExpr stream = nl->First(args);
  ListExpr count = nl->Second(args);

  if(( !Stream<Tuple>::checkType(stream) &&
       !Stream<Attribute>::checkType(stream) ) ||
     !CcInt::checkType(count) ){
    return listutils::typeError(err);
  }
  return stream;
}
/*
2.8.3 Value mapping function of operator ~head~

*/

#ifndef USE_PROGRESS

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
      local.setAddr( localInfo );
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
      if(local.addr)
      {
        localInfo = (HeadLocalInfo*)local.addr;
        delete localInfo;
        local.setAddr(0);
      }
      qp->Close(args[0].addr);
      return 0;
  }
  return 0;
}

#else

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
  HeadLocalInfo *hli;
  Word tupleWord;

  hli = (HeadLocalInfo*)local.addr;

  switch(message)
  {
    case OPEN:

      if ( hli ) delete hli;

      hli =
        new HeadLocalInfo( ((CcInt*)args[1].addr)->GetIntval() );
      local.setAddr( hli );

      qp->Open(args[0].addr);
      return 0;

    case REQUEST:


      if(hli->numTuples >= hli->maxTuples)
      {
        return CANCEL;
      }

      qp->Request(args[0].addr, tupleWord);
      if(qp->Received(args[0].addr))
      {
        result = tupleWord;
        hli->numTuples++;
        return YIELD;
      }
      else
      {
        return CANCEL;
      }

    case CLOSE:
      qp->Close(args[0].addr);
      return 0;

    case CLOSEPROGRESS:
      if ( hli )
      {
        delete hli;
        local.setAddr(0);
      }
      return 0;

    case REQUESTPROGRESS:

      ProgressInfo p1;
      ProgressInfo* pRes;
      const double uHead = 0.00056;

      pRes = (ProgressInfo*) result.addr;

      if ( !hli )  {
         return CANCEL;
      }

      if ( qp->RequestProgress(args[0].addr, &p1) )
      {
        pRes->Card = (p1.Card < hli->maxTuples ? p1.Card : hli->maxTuples);
        pRes->CopySizes(p1);

        double perTupleTime =
    (p1.Time - p1.BTime) / (p1.Card + 1);

        pRes->Time = p1.BTime + (double) pRes->Card * (perTupleTime + uHead);

        pRes->Progress =
    (p1.BProgress * p1.BTime
          + (double) hli->numTuples * (perTupleTime + uHead))
          / pRes->Time;

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
  if(nl->ListLength(args)!=2){
   return listutils::typeError("two arguments expected");
  }

  string err = "stream(tuple(...)) x attrname expected";
  ListExpr stream = nl->First(args);
  ListExpr attr = nl->Second(args);

  if(!listutils::isTupleStream(stream) ||
     nl->AtomType(attr)!=SymbolType){
   return listutils::typeError(err);
  }

  ListExpr attrtype;
  string attrname = nl->SymbolValue(attr);
  ListExpr attrlist = nl->Second(nl->Second(stream));
  int j = listutils::findAttribute(attrlist, attrname, attrtype);
  if ( j ) {
    if(!listutils::isSymbol(attrtype,CcReal::BasicType()) &&
       !listutils::isSymbol(attrtype,CcString::BasicType()) &&
       !listutils::isSymbol(attrtype,CcBool::BasicType()) &&
       !listutils::isSymbol(attrtype,CcInt::BasicType()) &&
       !listutils::isSymbol(attrtype,Instant::BasicType()) &&
       !listutils::isSymbol(attrtype,Duration::BasicType()) ){
      return listutils::typeError("result type not in {real, string, "
                                          "bool, int, instant, duration}");
    }
    return nl->ThreeElemList(nl->SymbolAtom(Symbol::APPEND()),
           nl->OneElemList(nl->IntAtom(j)), attrtype);
  } else {
    return listutils::typeError("attribute name " + attrname +
                                "not known in the tuple");
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
  Attribute* extremum =
    (Attribute*)(qp->ResultStorage(s)).addr;
  extremum->SetDefined(false);
  result.setAddr(extremum);

  int attributeIndex = ((CcInt*)args[2].addr)->GetIntval() - 1;

  qp->Open(args[0].addr);
  qp->Request(args[0].addr, currentTupleWord);
  while(qp->Received(args[0].addr))
  {
    Tuple* currentTuple = (Tuple*)currentTupleWord.addr;
    const Attribute* currentAttr =
      (const Attribute*)currentTuple->GetAttribute(attributeIndex);
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
2.10 Operators ~avg~, ~sum~, and ~var~

2.10.1 Type mapping function of Operators ~avg~, ~sum~, and ~var~

Type mapping for ~avg~ is

----  ((stream (tuple ((x1 t1)...(xn tn))) xi)  -> real)
              APPEND (i ti)
----

Type mapping for ~sum~ is

----  ((stream (tuple ((x1 t1)...(xn tn))) xi)  -> ti)
              APPEND (i ti)
----

Type mapping for ~var~ is

----  ((stream (tuple ((x1 t1)...(xn tn))) xi)  -> real)
              APPEND (i ti)
----

*/

template<bool isAvg>
ListExpr
AvgSumTypeMap( ListExpr args )
{

  NList type(args);
  if ( !type.hasLength(2) )
  {
    return NList::typeError("Expecting two arguments.");
  }

  NList first = type.first();
  if (
    !first.hasLength(2)  ||
    !first.first().isSymbol(Symbol::STREAM()) ||
    !first.second().hasLength(2) ||
    !first.second().first().isSymbol(Tuple::BasicType()) ||
    !IsTupleDescription( first.second().second().listExpr() ) )
  {
    return NList::typeError("Error in first argument!");
  }

  NList second = type.second();
  if ( !second.isSymbol() ) {
    return NList::typeError( "Second argument must be an attribute name. "
                             "Perhaps the attribute's name may be the name "
           "of a Secondo object!" );
  }

  string attrname = type.second().str();
  ListExpr attrtype = nl->Empty();
  int j = FindAttribute(first.second().second().listExpr(), attrname, attrtype);

  if ( j != 0 )
  {
    if ( nl->SymbolValue(attrtype) != CcReal::BasicType() &&
   nl->SymbolValue(attrtype) != CcInt::BasicType() )
    {
      return NList::typeError("Attribute type is not of type real or int.");
    }
    NList resType = isAvg ? NList(CcReal::BasicType()) : NList(attrtype);
    return NList( NList(Symbol::APPEND()),
                  NList(j).enclose(), resType ).listExpr();
  }
  else
  {
    return NList::typeError( "Attribute name '" + attrname + "' is not known.");
  }
}

/*
Since the list structure has been already checked the selection function can
trust to have a list of the correct format. Hence we can ommit many checks and
access elements directly.

*/

int
AvgSumSelect( ListExpr args )
{
  NList type(args);
  NList first = type.first();
  string attrname = type.second().str();
  ListExpr attrtype = nl->Empty();
  FindAttribute(first.second().second().listExpr(), attrname, attrtype);

  if ( nl->SymbolValue(attrtype) == CcInt::BasicType() )
  {
    return 0;
  }
  return 1;
}
/*

2.10.2 Value mapping function of operators ~avg~, ~sum~, and ~var~

Here we use template functions which may be instantiated with the
following values:

----
    T = int, real
    R = CcInt, CcReal
----

*/
template<class T, class R> int
SumValueMapping(Word* args, Word& result, int message,
                   Word& local, Supplier s)
{
  T sum = 0;
  int number = 0;

  Word currentTupleWord(Address(0));
  int attributeIndex = static_cast<CcInt*>( args[2].addr)->GetIntval() - 1;

  qp->Open(args[0].addr);
  qp->Request(args[0].addr, currentTupleWord);
  while(qp->Received(args[0].addr))
  {
    Tuple* currentTuple = static_cast<Tuple*>( currentTupleWord.addr );
    R* currentAttr =
      static_cast<R*>( currentTuple->GetAttribute(attributeIndex) );

    if( currentAttr->IsDefined() ) // process only defined elements
    {
      number++;
      sum += currentAttr->GetValue();
    }
    currentTuple->DeleteIfAllowed();
    qp->Request(args[0].addr, currentTupleWord);
  }
  qp->Close(args[0].addr);

  result = qp->ResultStorage(s);
  static_cast<R*>( result.addr )->Set(true, sum);
  return 0;
}

template<class T, class R> int
AvgValueMapping(Word* args, Word& result, int message,
                   Word& local, Supplier s)
{
  T sum = 0;
  int number = 0;

  Word currentTupleWord(Address(0));
  int attributeIndex = static_cast<CcInt*>( args[2].addr )->GetIntval() - 1;

  qp->Open(args[0].addr);
  qp->Request(args[0].addr, currentTupleWord);
  while(qp->Received(args[0].addr))
  {
    Tuple* currentTuple = static_cast<Tuple*>( currentTupleWord.addr );
    R* currentAttr =
      static_cast<R*>( currentTuple->GetAttribute(attributeIndex) );

    if( currentAttr->IsDefined() ) // process only defined elements
    {
      number++;
      sum += currentAttr->GetValue();
    }
    currentTuple->DeleteIfAllowed();
    qp->Request(args[0].addr, currentTupleWord);
  }
  qp->Close(args[0].addr);

  result = qp->ResultStorage(s);
  if ( number == 0 ) {
    static_cast<Attribute*>( result.addr )->SetDefined(false);
  }
  else
  {
    SEC_STD_REAL sumreal = sum;
    static_cast<CcReal*>( result.addr )->Set(true, sumreal / number);
  }
  return 0;
}


template<class T, class R> int
VarValueMapping(Word* args, Word& result, int message,
                Word& local, Supplier s)
{
  TupleBuffer *tp = 0;
  GenericRelationIterator *relIter = 0;
  long MaxMem = qp->MemoryAvailableForOperator();

  T sum = 0;
  int number = 0;
  int counter = 0;
  SEC_STD_REAL mean = 0.0;
  SEC_STD_REAL diffsum = 0.0;

  result = qp->ResultStorage(s);
  Word currentTupleWord(Address(0));
  int attributeIndex = static_cast<CcInt*>( args[2].addr )->GetIntval() - 1;

  tp = new TupleBuffer(MaxMem);

  // In a first scan, we compute the stream's MEAN:
  qp->Open(args[0].addr);
  qp->Request(args[0].addr, currentTupleWord);
  while(qp->Received(args[0].addr))
  {
    Tuple* currentTuple = static_cast<Tuple*>( currentTupleWord.addr );
    R* currentAttr =
        static_cast<R*>( currentTuple->GetAttribute(attributeIndex) );

    if( currentAttr->IsDefined() ) // process only defined elements
    {
      number++;
      sum += currentAttr->GetValue();
    }
    tp->AppendTuple(currentTuple);
    currentTuple->DeleteIfAllowed();
    qp->Request(args[0].addr, currentTupleWord);
  }
  qp->Close(args[0].addr);

  // if there was no defined value, we are finished. Otherwise we need a secomd
  // scan to compute the stream's VARIANCE
  if ( number < 2 ) {
    static_cast<Attribute*>( result.addr )->SetDefined(false);
  }
  else
  {
    mean = (sum * 1.0) / number;

    Tuple* currentTuple = 0;
    relIter = tp->MakeScan();
    currentTuple = relIter->GetNextTuple();

    while( currentTuple && (counter <= number) )
    {
      R* currentAttr =
          static_cast<R*>( currentTuple->GetAttribute(attributeIndex) );

      if( currentAttr->IsDefined() ) // process only defined elements
      {
        counter++;
        SEC_STD_REAL Diff = ( (currentAttr->GetValue() * 1.0) - mean );
        diffsum += Diff * Diff;
      }
      currentTuple->DeleteIfAllowed();
      currentTuple = relIter->GetNextTuple();
    }
    delete relIter;
    static_cast<CcReal*>( result.addr )->Set(true, diffsum / (counter - 1));
  }
  delete tp;
  return 0;
}


/*
2.10.3 Operator Descriptions for ~avg~, ~sum~, and ~var~

*/

struct avgInfo : OperatorInfo {

  avgInfo() : OperatorInfo() {

    name      = "avg";
    signature = "((stream (tuple([a1:d1, ...,"
                          " ai:int, ..., an:dn]))) x ai) -> real";
    appendSignature( "((stream (tuple([a1:d1, ...,"
                          " ai:real, ..., an:dn]))) x ai) -> real");
    syntax    = "_ avg [ _ ]";
    meaning   = "Returns the average value of attribute "
                "ai over the input stream. ";
  }

};

struct sumInfo : OperatorInfo {

  sumInfo() : OperatorInfo() {

    name      = "sum";
    signature = "((stream (tuple([a1:d1, ..., "
                            "ai:int, ..., an:dn]))) x ai) -> int";
    appendSignature( "((stream (tuple([a1:d1, ...,"
                         " ai:real, ..., an:dn]))) x ai) -> real" );
    syntax    = "_ sum [ _ ]";
    meaning   = "Returns the sum of the values of attribute "
                "ai over the input stream. ";
  }

};

struct varInfo : OperatorInfo {

  varInfo() : OperatorInfo() {

    name      = "var";
    signature = "((stream (tuple([a1:d1, ..., "
        "ai:int, ..., an:dn]))) x ai) -> real";
    appendSignature( "((stream (tuple([a1:d1, ...,"
        " ai:real, ..., an:dn]))) x ai) -> real" );
    syntax    = "_ var [ _ ]";
    meaning   = "Returns the variance of the values of attribute "
        "ai over the input stream. Needs to perform 2 scans!";
  }

};

/*
10.5 Operator ~stats~

This operator calculates several aggregation functions and statistics on a
stream of tuples containing two mumeric attributes. And returns a single tuple
with
all the results:

  * CountX - Number of defined instances for the first attribute X

  * MinX   - minimum instance of the first attribute

  * MaxX   - maximum instance of the first attribute

  * SumX   - sum for all defined instance of the first attribute

  * AvgX   - mean for all defined instance of the first attribute

  * VarX   - variance for all defined instance of the first attribute

  * CountY - Number of defined instances for the second attribute Y

  * MinY   - minimum instance of the second attribute

  * MaxY   - maximum instance of the second attribute

  * SumY   - sum for all defined instance of the second attribute

  * AvgY   - mean for all defined instance of the second attribute

  * VarY   - variance for all defined instance of the second attribute

  * Count  - Cardinality of the stream

  * CountXY - Number of tuples, where both arguments X and Y are defined

  * CovXY  - the covariance for X and Y

  * Corr   - the Pearson product-moment correlation coefficient for X and Y

*/

/*
10.5.1 Type mapping for ~stats~

Type mapping for ~stats~ is

----  ((stream (tuple ((x1 t1)...(xn tn))) xi xj)  -> stream(tuple(
       (CountX int) (MinX real) (MaxX real) (SumX real) (AvgX real) (VarX real)
       (CountY int) (MinY real) (MaxY real) (SumY real) (AvgY real) (VarY real)
             (Count int) (CountXY int) (CovXY real) (CorrXY real)))
              APPEND ((i ti) (j tj))

      where ti, ty in {int, real}
----

*/

ListExpr  StatsTypeMap( ListExpr args )
{

  NList type(args);
  if ( !type.hasLength(3) )
  {
    return NList::typeError("Expecting three arguments.");
  }

  NList first = type.first();
  if (
      !first.hasLength(2)  ||
      !first.first().isSymbol(Symbol::STREAM()) ||
      !first.second().hasLength(2) ||
      !first.second().first().isSymbol(Tuple::BasicType()) ||
      !IsTupleDescription( first.second().second().listExpr() ) )
  {
    return NList::typeError("First argument must be of type stream(tuple(X))");
  }

  NList second = type.second();
  if ( !second.isSymbol() ) {
    return NList::typeError( "Second argument must be an attribute name. "
        "Perhaps the attribute's name may be the name "
        "of a Secondo object!" );
  }

  NList third = type.third();
  if ( !second.isSymbol() ) {
    return NList::typeError( "Third argument must be an attribute name. "
        "Perhaps the attribute's name may be the name "
        "of a Secondo object!" );
  }

  string attrnameX = type.second().str();
  ListExpr attrtypeX = nl->Empty();
  int jX = FindAttribute(first.second().second().listExpr(),
                         attrnameX, attrtypeX);

  string attrnameY = type.third().str();
  ListExpr attrtypeY = nl->Empty();
  int jY = FindAttribute(first.second().second().listExpr(),
                         attrnameY, attrtypeY);

  if ( (jX != 0) && (jY != 0) )
  {
    if ( nl->SymbolValue(attrtypeX) != CcReal::BasicType() &&
         nl->SymbolValue(attrtypeX) != CcInt::BasicType() )
    {
      return NList::typeError(
          "Attribute type of 2nd argument is not of type real or int.");
    }
    if ( nl->SymbolValue(attrtypeY) != CcReal::BasicType() &&
         nl->SymbolValue(attrtypeY) != CcInt::BasicType() )
    {
      return NList::typeError(
          "Attribute type of 3rd argument is not of type real or int.");
    }
    NList resTupleType = NList(NList("CountX"),
                               NList(CcInt::BasicType())).enclose();
    resTupleType.append(NList(NList("MinX"),NList(CcReal::BasicType())));
    resTupleType.append(NList(NList("MaxX"),NList(CcReal::BasicType())));
    resTupleType.append(NList(NList("SumX"),NList(CcReal::BasicType())));
    resTupleType.append(NList(NList("AvgX"),NList(CcReal::BasicType())));
    resTupleType.append(NList(NList("VarX"),NList(CcReal::BasicType())));
    resTupleType.append(NList(NList("CountY"),NList(CcInt::BasicType())));
    resTupleType.append(NList(NList("MinY"),NList(CcReal::BasicType())));
    resTupleType.append(NList(NList("MaxY"),NList(CcReal::BasicType())));
    resTupleType.append(NList(NList("SumY"),NList(CcReal::BasicType())));
    resTupleType.append(NList(NList("AvgY"),NList(CcReal::BasicType())));
    resTupleType.append(NList(NList("VarY"),NList(CcReal::BasicType())));
    resTupleType.append(NList(NList("Count"),NList(CcInt::BasicType())));
    resTupleType.append(NList(NList("CountXY"),NList(CcInt::BasicType())));
    resTupleType.append(NList(NList("CovXY"),NList(CcReal::BasicType())));
    resTupleType.append(NList(NList("CorrXY"),NList(CcReal::BasicType())));
    NList resType = NList( NList(Symbol::APPEND()), NList(NList(jX), NList(jY)),
                           NList(NList(Symbol::STREAM()),
                                 NList(NList(Tuple::BasicType()),resTupleType))
                         );
    return resType.listExpr();
  }
  else
  {
    if (jX == 0)
      return NList::typeError( "Attribute '" + attrnameX + "' is not known.");
    else
      return NList::typeError( "Attribute '" + attrnameY + "' is not known.");
  }
}

/*
Since the list structure has been already checked the selection function can
trust to have a list of the correct format. Hence we can ommit many checks and
access elements directly.

*/

int StatsSelect( ListExpr args )
{
  NList type(args);
  NList first = type.first();
  string attrnameX = type.second().str();
  string attrnameY = type.third().str();
  ListExpr attrtypeX = nl->Empty();
  ListExpr attrtypeY = nl->Empty();
  FindAttribute(first.second().second().listExpr(), attrnameX, attrtypeX);
  FindAttribute(first.second().second().listExpr(), attrnameY, attrtypeY);

  if ((nl->SymbolValue(attrtypeX) == CcInt::BasicType())&&(
    nl->SymbolValue(attrtypeY) == CcInt::BasicType()))
    return 0;
  if ((nl->SymbolValue(attrtypeX) == CcInt::BasicType())&&
    (nl->SymbolValue(attrtypeY) == CcReal::BasicType()))
    return 1;
  if ((nl->SymbolValue(attrtypeX) == CcReal::BasicType())&&
    (nl->SymbolValue(attrtypeY) == CcInt::BasicType()))
    return 2;
  if ((nl->SymbolValue(attrtypeX) == CcReal::BasicType())&&
    (nl->SymbolValue(attrtypeY)==CcReal::BasicType()))
    return 3;
  assert( false );
  return -1;
}

/*
10.5.2 Value mapping for ~stats~

There are 4 template class parameters.
Tx and Ty are ~int~ or ~real~,
Rx and Ry are the corresponding types ~CcInt~ resp. ~CcReal~.

*/

template<class Tx, class Rx, class Ty, class Ry> int
    StatsValueMapping(Word* args, Word& result, int message,
                      Word& local, Supplier s)
{

  TupleBuffer *tp = 0;
  GenericRelationIterator *relIter = 0;
  long MaxMem = qp->MemoryAvailableForOperator();

  bool *finished = 0;
  TupleType *resultTupleType = 0;
  Tuple *newTuple = 0;
  CcInt *CCountX = 0;
  CcReal *CMinX = 0;
  CcReal *CMaxX = 0;
  CcReal *CSumX = 0;
  CcReal *CAvgX = 0;
  CcReal *CVarX = 0;
  CcInt *CCountY = 0;
  CcReal *CMinY = 0;
  CcReal *CMaxY = 0;
  CcReal *CSumY = 0;
  CcReal *CAvgY = 0;
  CcReal *CVarY = 0;
  CcInt *CCount = 0;
  CcInt *CCountXY = 0;
  CcReal *CCovXY = 0;
  CcReal *CCorrXY = 0;

  switch(message)
  {
    case OPEN:
    {
      finished = new bool(false);
      local.addr = finished;
      return 0;
    }

    case REQUEST:
    {
      if(local.addr == 0)
        return CANCEL;
      finished = (bool*) local.addr;
      if(*finished)
        return CANCEL;

      int countAll = 0, countX = 0, countY = 0, countXY = 0;
      SEC_STD_REAL minX = 0, maxX = 0, sumX = 0;
      SEC_STD_REAL minY = 0, maxY = 0, sumY = 0;
      SEC_STD_REAL meanX = 0, meanY  = 0;
      SEC_STD_REAL diffsumX = 0, diffsumY = 0, sumXY = 0;
      SEC_STD_REAL sumX2 = 0, sumY2 = 0, sumsqX = 0, sumsqY = 0;

      Tx currX = 0;
      Ty currY = 0;

      result = qp->ResultStorage(s);
      Word currentTupleWord(Address(0));
      int attributeIndexX = static_cast<CcInt*>(args[3].addr)->GetIntval() - 1;
      int attributeIndexY = static_cast<CcInt*>(args[4].addr)->GetIntval() - 1;

      tp = new TupleBuffer(MaxMem);

      // In a first scan, we compute the stream's MEANs for X,Y
      // and count defined values for X,Y:
      qp->Open(args[0].addr);
      qp->Request(args[0].addr, currentTupleWord);
      while(qp->Received(args[0].addr))
      {
        countAll++;
        Tuple* currentTuple = static_cast<Tuple*>( currentTupleWord.addr );
        Rx* currentAttrX =
            static_cast<Rx*>( currentTuple->GetAttribute(attributeIndexX) );
        Ry* currentAttrY =
            static_cast<Ry*>( currentTuple->GetAttribute(attributeIndexY) );

        if( currentAttrX->IsDefined() )
        {
          countX++;
          currX = currentAttrX->GetValue();
          if (countX == 1)
          {
            minX = currX;
            maxX = currX;
          }
          else
          {
            if (currX < minX)
              minX = currX;
            if (currX > maxX)
              maxX = currX;
          }
          sumX += currX;
        }
        if( currentAttrY->IsDefined() )
        {
          countY++;
          currY = currentAttrY->GetValue();
          if (countY == 1)
          {
            minY = currY;
            maxY = currY;
          }
          else
          {
            if (currY < minY)
              minY = currY;
            if (currY > maxY)
              maxY = currY;
          }
          sumY += currY;
        }
        tp->AppendTuple(currentTuple);
        if(currentTuple->DeleteIfAllowed()){
            cout << "stats :: deleting a tuple stored in a buffer !!!" << endl;
        }
        qp->Request(args[0].addr, currentTupleWord);
      }
      qp->Close(args[0].addr);

      if ( (countX + countY) > 1 )
      {
        meanX = (sumX * 1.0) / countX;
        meanY = (sumY * 1.0) / countY;

        // A second Scan to calculate the emprical covariance and
        // emprical correlation coefficient. Therefore, we recalculate the means
        // using only those tuples, where both attributes are defined...
        Tuple* currentTuple = 0;
        relIter = tp->MakeScan();
        currentTuple = relIter->GetNextTuple();

        while( currentTuple )
        {
          Rx* currentAttrX =
              static_cast<Rx*>( currentTuple->GetAttribute(attributeIndexX) );
          Ry* currentAttrY =
              static_cast<Ry*>( currentTuple->GetAttribute(attributeIndexY) );
          if( currentAttrX->IsDefined() )
          { // for var(X):
            currX = currentAttrX->GetValue();
            diffsumX += (currX - meanX) * (currX - meanX);
          }
          if( currentAttrY->IsDefined() )
          { // for var(Y):
            currY = currentAttrY->GetValue();
            diffsumY += (currY - meanY) * (currY - meanY);
          }
          if( currentAttrX->IsDefined() && currentAttrY->IsDefined() )
          { // for cov(X,Y) and corr(X,Y):
            countXY++;
            sumX2 += currX;
            sumY2 += currY;
            sumXY += currX * currY;
            sumsqX += currX * currX;
            sumsqY += currY * currY;
          }
          currentTuple->DeleteIfAllowed();
          currentTuple = relIter->GetNextTuple();
        }
        delete relIter;
      }
      delete tp;

      // create the resulttuple:
      resultTupleType = new TupleType(nl->Second(GetTupleResultType(s)));
      newTuple = new Tuple( resultTupleType );

      CCountX = new CcInt(true, countX);
      CMinX   = new CcReal((countX > 0), minX);
      CMaxX   = new CcReal((countX > 0), maxX);
      CSumX   = new CcReal(true, sumX);
      if (countX > 0){
        CAvgX = new CcReal(true, sumX / countX);
        CVarX = new CcReal(true, diffsumX / (countX - 1));
      }
      else{
        CAvgX = new CcReal(false, 0);
        CVarX = new CcReal(false, 0);
      }
      CCountY = new CcInt(true, countY);
      CMinY   = new CcReal((countY > 0), minY);
      CMaxY   = new CcReal((countY > 0), maxY);
      CSumY   = new CcReal(true, sumY);
      if (countY > 0){
        CAvgY = new CcReal(true, sumY / countY);
        CVarY = new CcReal(true, diffsumY / (countY - 1));
      }
      else {
        CAvgY = new CcReal(false, 0);
        CVarY = new CcReal(false, 0);
      }
      CCount  = new CcInt(true, countAll);
      CCountXY = new CcInt(true, countXY);
      if(countXY > 1){
        SEC_STD_REAL covXY =
            (sumXY - (sumX2 * sumY2 / countXY)) / (countXY - 1);
        CCovXY = new CcReal(true, covXY);
        SEC_STD_REAL meanX2 = sumX2/countXY;
        SEC_STD_REAL meanY2 = sumY2/countXY;
        SEC_STD_REAL denom =   sqrt(sumsqX - (countXY * (meanX2 * meanX2)))
                             * sqrt(sumsqY - (countXY * (meanY2 * meanY2)));
        if (denom == 0){
            CCorrXY = new CcReal(false, 0);
        }
        else{
          CCorrXY = new
              CcReal(true, (sumXY - (countXY * meanX2 * meanY2) ) / denom);
        }
      }
      else {
        CCovXY  = new CcReal(false, 0);
        CCorrXY = new CcReal(false, 0);
      }
      newTuple->PutAttribute(  0,(Attribute*)CCountX);
      newTuple->PutAttribute(  1,(Attribute*)CMinX);
      newTuple->PutAttribute(  2,(Attribute*)CMaxX);
      newTuple->PutAttribute(  3,(Attribute*)CSumX);
      newTuple->PutAttribute(  4,(Attribute*)CAvgX);
      newTuple->PutAttribute(  5,(Attribute*)CVarX);
      newTuple->PutAttribute(  6,(Attribute*)CCountY);
      newTuple->PutAttribute(  7,(Attribute*)CMinY);
      newTuple->PutAttribute(  8,(Attribute*)CMaxY);
      newTuple->PutAttribute(  9,(Attribute*)CSumY);
      newTuple->PutAttribute( 10,(Attribute*)CAvgY);
      newTuple->PutAttribute( 11,(Attribute*)CVarY);
      newTuple->PutAttribute( 12,(Attribute*)CCount);
      newTuple->PutAttribute( 13,(Attribute*)CCountXY);
      newTuple->PutAttribute( 14,(Attribute*)CCovXY);
      newTuple->PutAttribute( 15,(Attribute*)CCorrXY);

      result.setAddr(newTuple);
      resultTupleType->DeleteIfAllowed();

      *finished = true;
      return YIELD;
    }

    case CLOSE:
    {
      if(local.addr == 0)
        return 0;
      finished = (bool*) local.addr;
      delete(finished);
      local.setAddr(0);
      return 0;
    }
  } // end switch
  return -1;
}

/*
10.5.2 Operator Descriptions for ~stats~

*/

struct statsInfo : OperatorInfo {

  statsInfo() : OperatorInfo() {

    name      = "stats";
    signature = "((stream (tuple([a1:d1, ... ,an:dn]))) x ai x aj) -> "
    "stream(tuple( \n"
    "(CountX int) (MinX real) (MaxX real) (SumX real) (AvgX real) (VarX real)\n"
    "(CountY int) (MinY real) (MaxY real) (SumY real) (AvgY real) (VarY real)\n"
    "(Count int) (CountXY int) (CovXY real) (CorrXY real))\n, "
    "where ti, tj in {int,real}";
    syntax    = "_ stats [ attrnameX , attrnameY ]";
    meaning   = "Returns a tuple containing several statistics for 2 numerical "
        "attributes 'attrnameX', 'attrnameY' on the stream. The first 6 "
        "attributes contain the number of tuples where X is defined, the "
        "minimum and maximum value for X, the sum for X, the mean for X, and "
        "the variance of X. \n"
        "The 7th to 12th attribute hold the appropriate statistics for Y.\n"
        "the last 4 attributes hold the total count of tuples in the stream, "
        "the number of tuples, where both attributes X and Y contain defined "
        "values, the covariance of X and X, and finally Pearson's "
        "correlation coefficient";
  }

};


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
      if(nl->AtomType(attr)!=SymbolType){
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
     !nl->IsEqual(nl->First(streamList),Symbol::STREAM())){
     ErrorReporter::ReportError("(stream (tuple(...)) "
                                " expected as first argument");
     return nl->TypeError();
  }

  ListExpr tupleList = nl->Second(streamList);
  if(nl->ListLength(tupleList)!=2 ||
     !nl->IsEqual(nl->First(tupleList),Tuple::BasicType())){
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


  ListExpr result =  nl->ThreeElemList(nl->SymbolAtom(Symbol::APPEND()),
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
      local.setAddr(new KrdupLocalInfo(qp->GetSon(s,2)));
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
      if(local.addr) {
        KrdupLocalInfo* localinfo = (KrdupLocalInfo*) local.addr;
        delete localinfo;
        local.setAddr(0);
      }
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
2.13.3 Definition of operator ~krdup~

*/
Operator krdup (
         "krdup",             // name
         KrdupSpec,           // specification
         KrdupVM,               // value mapping
         Operator::SimpleSelect,          // trivial selection function
         krdupTM         // type mapping
);

/*

2.10 Operator k-smallest

2.10.1 Type Mapping

   stream(tuple(...)) [x] int [x] attr[_]1 ... attr[_]n [->] tuple(...)


*/

ListExpr ksmallestTM(ListExpr args){

  string err = "stream(tuple(...)) x int x attr_1 x ... x attr_n   expected";
  if(nl->ListLength(args) < 3 ){
     ErrorReporter::ReportError(err);
     return nl->TypeError();
  }
  ListExpr Stream = nl->First(args);
  args = nl->Rest(args);
  ListExpr Int = nl->First(args);
  ListExpr AttrList = nl->Rest(args);

  if(nl->ListLength(AttrList)!=1 ){
     ErrorReporter::ReportError(err);
     return nl->TypeError();
  }
  AttrList = nl->First(AttrList);

  if(!IsStreamDescription(Stream)){
    ErrorReporter::ReportError(err);
    return nl->TypeError();
  }
  if(!nl->IsEqual(Int,CcInt::BasicType())){
    ErrorReporter::ReportError(err);
    return nl->TypeError();
  }

  ListExpr StreamList = nl->Second(nl->Second(Stream));

  int attrNo = 0;
  ListExpr NumberList;
  ListExpr Last;
  ListExpr attrType = nl->TheEmptyList();

  if(nl->AtomType(AttrList)!=NoAtom){
    return listutils::typeError(err);
  }


  while(!nl->IsEmpty(AttrList)){

     ListExpr Attr = nl->First(AttrList);
     if(nl->AtomType(Attr)!=SymbolType){
        ErrorReporter::ReportError(err);
        return nl->TypeError();
     }
     string attrName = nl->SymbolValue(Attr);
     int j = FindAttribute(StreamList,attrName, attrType);
     if(j==0){
        ErrorReporter::ReportError("Attribute name " + attrName + "not found");
        return nl->TypeError();
     }
     if(attrNo==0){
      NumberList = nl->OneElemList(nl->IntAtom(j-1));
      Last = NumberList;
     } else {
       Last = nl->Append(Last, nl->IntAtom(j-1));
     }
     attrNo++;
     AttrList = nl->Rest(AttrList);
  }

  return nl->ThreeElemList(
              nl->SymbolAtom(Symbol::APPEND()),
              nl->TwoElemList(nl->IntAtom(attrNo),
                              NumberList),
              Stream);
}


/*
2.10.2 LocalInfo


*/

#ifndef USE_PROGRESS
class KSmallestLocalInfo{
 public:

/*
~Constructor~

Constructs a localinfo tor given k and the attribute indexes by ~attrnumbers~.

*/
    KSmallestLocalInfo(int ak, vector<int>& attrnumbers,bool Smallest):
       elems(0),numbers(attrnumbers),pos(0),smallest(Smallest),
       firstRequest(true){
       if(ak<0){
          k = 0;
       } else {
          k = ak;
       }
    }


/*
~Destructor~

Destroy this instance and calls DeleteIfAllowed for all
non processed tuples.


*/
    ~KSmallestLocalInfo(){
        for(unsigned int i=0;i<elems.size();i++){
          if(elems[i]){
            elems[i]->DeleteIfAllowed();
            elems[i] = 0;
          }
        }
    }


/*
~nextTuple~

Returns the next tuple within the buffer, or 0 if no tuple is
available.

*/
   Tuple* nextTuple(Word& stream){
      if(firstRequest){
         Word elem;
         qp->Request(stream.addr,elem);
         Tuple* tuple;
         while(qp->Received(stream.addr)){
           tuple = static_cast<Tuple*>(elem.addr);
           insertTuple(tuple);
           qp->Request(stream.addr,elem);
           firstRequest = false;
         }
      }

      if(pos==0){
         // sort the elements
         if(elems.size()< k){
            initializeHeap();
         }
         for(unsigned int i=elems.size()-1; i>0; i--){
            Tuple* top = elems[0];
            Tuple* last = elems[i];
            elems[0] = last;
            elems[i] = top;
            sink(0,i);
         }
      }
      if(pos<elems.size()){
         Tuple* res = elems[pos];
         elems[pos] = 0;
         pos++;
         return res;
      } else {
        return 0;
      }
   }

 private:
    vector<Tuple*> elems;
    vector<int> numbers;
    unsigned int k;
    unsigned int pos;
    bool smallest;
    bool firstRequest;


/*
~insertTuple~

Inserts a tuple into the local buffer. If the buffer would be
overflow (size [>] k) , the maximum element is removed from the buffer.

*/
    void insertTuple(Tuple* tuple){
       if(elems.size() < k){
          elems.push_back(tuple);
          if(elems.size()==k){
            initializeHeap();
          }
       } else {
         Tuple* maxTuple = elems[0];

         int cmp = compareTuples(tuple,maxTuple);
         if(cmp>=0){ // tuple >= maxTuple
            tuple->DeleteIfAllowed();
         } else {
            maxTuple->DeleteIfAllowed();
            elems[0] = tuple;
            sink(0,elems.size());
         }
       }
    }

    inline int compareTuples(Tuple* t1, Tuple* t2){
      return smallest?compareTuplesSmaller(t1,t2): compareTuplesSmaller(t2,t1);
    }

    int compareTuplesSmaller(Tuple* t1, Tuple* t2){
       for(unsigned int i=0;i<numbers.size();i++){
          Attribute* a1 = t1->GetAttribute(numbers[i]);
          Attribute* a2 = t2->GetAttribute(numbers[i]);
          int cmp = a1->Compare(a2);
          if(cmp!=0){
            return cmp;
          }
       }
       return 0;
    }

    void initializeHeap(){
       int s = elems.size()/2;
       for(int i=s; i>=0; i--){
          sink(i,elems.size());
       }
    }

    void sink(unsigned int i, unsigned int max){
      unsigned int root = i;
      unsigned int son1 = ((i+1)*2) - 1;
      unsigned int son2 = ((i+1)*2+1) - 1;
      unsigned int swapWith;

      bool done = false;
      do{
        swapWith = root;
        son1 = ((root+1)*2) - 1;
        son2 = ((root+1)*2+1) - 1;
        if(son1<max){
          int cmp = compareTuples(elems[root],elems[son1]);
          if(cmp<0){
             swapWith = son1;
          }
        }
        if(son2 < max){
          int cmp = compareTuples(elems[swapWith],elems[son2]);
          if(cmp<0){
            swapWith = son2;
          }
        }
        if(swapWith!=root){
          Tuple* t1 = elems[root];
          Tuple* t2 = elems[swapWith];
          elems[swapWith] = t1;
          elems[root] = t2;
        }
        done = (swapWith == root);
        root = swapWith;

      }while(!done);
    }
};

template<bool smaller>
int ksmallestVM(Word* args, Word& result,
                           int message, Word& local, Supplier s)
{
  switch( message )
  {
    case OPEN:{
      qp->Open(args[0].addr);
      CcInt* cck = static_cast<CcInt*>(args[1].addr);
      int attrNum = (static_cast<CcInt*>(args[3].addr))->GetIntval();
      Supplier son;
      Word elem;
      vector<int> attrPos;
      for(int i=0;i<attrNum;i++){
         son = qp->GetSupplier(args[4].addr,i);
         qp->Request(son,elem);
         int anum = (static_cast<CcInt*>(elem.addr))->GetIntval();
         attrPos.push_back(anum);
      }
      int k = cck->IsDefined()?cck->GetIntval():0;
      KSmallestLocalInfo* linfo = new   KSmallestLocalInfo(k,attrPos,smaller);
      local.addr = linfo;
      return 0;
    }
    case REQUEST:{
       KSmallestLocalInfo* linfo;
       linfo = static_cast<KSmallestLocalInfo*>(local.addr);
       if(linfo){
         Tuple* tuple = linfo->nextTuple(args[0]);
         if(tuple){
           result.setAddr(tuple);
           return YIELD;
         }
       }
       return CANCEL;

    }
    case CLOSE:{
       qp->Close(args[0].addr);
       KSmallestLocalInfo* linfo;
       linfo = static_cast<KSmallestLocalInfo*>(local.addr);
       if(linfo){
          delete linfo;
          local.addr=0;
       }
       return 0;
    }
    default:{
       return  0;
    }
  }

}

#else

class KSmallestLocalInfo: public ProgressLocalInfo{
  public:

/*
~Constructor~

Constructs a localinfo tor given k and the attribute indexes by ~attrnumbers~.

*/

    KSmallestLocalInfo(int ak, vector<int>& attrnumbers,bool Smallest):
      elems(0),numbers(attrnumbers),pos(0),smallest(Smallest),
            firstRequest(true){
              if(ak<0){
                k = 0;
              } else {
                k = ak;
              }
            }


/*
            ~Destructor~

            Destroy this instance and calls DeleteIfAllowed for all
            non processed tuples.


*/
            ~KSmallestLocalInfo(){
              for(unsigned int i=0;i<elems.size();i++){
                if(elems[i]){
                  elems[i]->DeleteIfAllowed();
                  elems[i] = 0;
                }
              }
            }

            int getK()
            {
              return k;
            }

/*
            ~nextTuple~

            Returns the next tuple within the buffer, or 0 if no tuple is
            available.

*/
            Tuple* nextTuple(Word& stream){
              if(firstRequest){
                Word elem;
                qp->Request(stream.addr,elem);
                Tuple* tuple;
                while(qp->Received(stream.addr)){
                  tuple = static_cast<Tuple*>(elem.addr);
                  insertTuple(tuple);
                  qp->Request(stream.addr,elem);
                  firstRequest = false;
                }
              }

              if(pos==0){
         // sort the elements
                if(elems.size()< k){
                  initializeHeap();
                }
                for(unsigned int i=elems.size()-1; i>0; i--){
                  Tuple* top = elems[0];
                  Tuple* last = elems[i];
                  elems[0] = last;
                  elems[i] = top;
                  sink(0,i);
                }
              }
              if(pos<elems.size()){
                Tuple* res = elems[pos];
                elems[pos] = 0;
                pos++;
                return res;
              } else {
                return 0;
              }
            }

  private:
    vector<Tuple*> elems;
    vector<int> numbers;
    unsigned int k;
    unsigned int pos;
    bool smallest;
    bool firstRequest;


/*
~insertTuple~

Inserts a tuple into the local buffer. If the buffer would be
overflow (size [>] k) , the maximum element is removed from the buffer.

*/
    void insertTuple(Tuple* tuple){
      if(elems.size() < k){
        elems.push_back(tuple);
        if(elems.size()==k){
          initializeHeap();
        }
      } else {
        Tuple* maxTuple = elems[0];

        int cmp = compareTuples(tuple,maxTuple);
        if(cmp>=0){ // tuple >= maxTuple
          tuple->DeleteIfAllowed();
        } else {
          maxTuple->DeleteIfAllowed();
          elems[0] = tuple;
          sink(0,elems.size());
        }
      }
    }

    inline int compareTuples(Tuple* t1, Tuple* t2){
      return smallest?compareTuplesSmaller(t1,t2): compareTuplesSmaller(t2,t1);
    }

    int compareTuplesSmaller(Tuple* t1, Tuple* t2){
      for(unsigned int i=0;i<numbers.size();i++){
        Attribute* a1 = t1->GetAttribute(numbers[i]);
        Attribute* a2 = t2->GetAttribute(numbers[i]);
        int cmp = a1->Compare(a2);
        if(cmp!=0){
          return cmp;
        }
      }
      return 0;
    }

    void initializeHeap(){
      int s = elems.size()/2;
      for(int i=s; i>=0; i--){
        sink(i,elems.size());
      }
    }

    void sink(unsigned int i, unsigned int max){
      unsigned int root = i;
      unsigned int son1 = ((i+1)*2) - 1;
      unsigned int son2 = ((i+1)*2+1) - 1;
      unsigned int swapWith;

      bool done = false;
      do{
        swapWith = root;
        son1 = ((root+1)*2) - 1;
        son2 = ((root+1)*2+1) - 1;
        if(son1<max){
          int cmp = compareTuples(elems[root],elems[son1]);
          if(cmp<0){
            swapWith = son1;
          }
        }
        if(son2 < max){
          int cmp = compareTuples(elems[swapWith],elems[son2]);
          if(cmp<0){
            swapWith = son2;
          }
        }
        if(swapWith!=root){
          Tuple* t1 = elems[root];
          Tuple* t2 = elems[swapWith];
          elems[swapWith] = t1;
          elems[root] = t2;
        }
        done = (swapWith == root);
        root = swapWith;

      }while(!done);
    }
};

template<bool smaller>
    int ksmallestVM(Word* args, Word& result,
                    int message, Word& local, Supplier s)
{
  switch( message )
  {
    case OPEN:{
      qp->Open(args[0].addr);
      CcInt* cck = static_cast<CcInt*>(args[1].addr);
      int attrNum = (static_cast<CcInt*>(args[3].addr))->GetIntval();
      Supplier son;
      Word elem;
      vector<int> attrPos;
      for(int i=0;i<attrNum;i++){
        son = qp->GetSupplier(args[4].addr,i);
        qp->Request(son,elem);
        int anum = (static_cast<CcInt*>(elem.addr))->GetIntval();
        attrPos.push_back(anum);
      }
      int k = cck->IsDefined()?cck->GetIntval():0;
      KSmallestLocalInfo* linfo = new   KSmallestLocalInfo(k,attrPos,smaller);
      local.addr = linfo;
      return 0;
    }
    case REQUEST:{
      KSmallestLocalInfo* linfo;
      linfo = static_cast<KSmallestLocalInfo*>(local.addr);
      if(linfo){
        Tuple* tuple = linfo->nextTuple(args[0]);
        if(tuple){
          result.setAddr(tuple);
          return YIELD;
        }
      }
      return CANCEL;

    }
    case CLOSE:{
      qp->Close(args[0].addr);
#ifndef USE_PROGRESS
      KSmallestLocalInfo* linfo;
      linfo = static_cast<KSmallestLocalInfo*>(local.addr);
      if(linfo){
        delete linfo;
        local.addr=0;
      }
#endif
      return 0;
    }

    case CLOSEPROGRESS: {
      KSmallestLocalInfo* linfo;
      linfo = static_cast<KSmallestLocalInfo*>(local.addr);
      if(linfo){
        delete linfo;
        local.addr=0;
      }
      return 0;
    }
    case REQUESTPROGRESS: {

      ProgressInfo p1;
      ProgressInfo *pRes;
      KSmallestLocalInfo* linfo;

      const double uKsmallest = 0.0001287348; //millisecs per tuple
      const double vKsmallest = 0.0000021203; //millisecs per tuple
      const double wKsmallest = 0.0000000661; //millisecs per k

      pRes = (ProgressInfo*) result.addr;
      linfo = (KSmallestLocalInfo*) local.addr;
      if(!linfo){
         return CANCEL;
      }

      if (qp->RequestProgress(args[0].addr, &p1))
      {
        pRes->CopySizes(p1);
        int k = linfo->getK();
        if (k < p1.Card)
        {
          pRes->Card = k;
        }
        else
        {
          pRes->Card = p1.Card;
        }
        pRes->Time = p1.Time + p1.Card *
            (uKsmallest + p1.SizeExt * vKsmallest + k * wKsmallest);
        pRes->Progress = (p1.Progress*p1.Time + linfo->read *
            (uKsmallest + p1.SizeExt * vKsmallest + k * wKsmallest))/pRes->Time;
        pRes->BTime=pRes->Time;
        pRes->BProgress=pRes->Progress;

        return YIELD;
      } else {
         return CANCEL;
      }
    }

    default:{
      return  0;
    }
  }

}
#endif
const string ksmallestSpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "( <text>stream(tuple([a1:d1, ... ,an:dn])))"
    " x int x a_k x  ... a_m -> "
    "(stream (tuple(...)))</text--->"
    "<text>_ ksmallest [k ; list ]</text--->"
    "<text>returns the k smallest elements from the input stream"
    "</text--->"
    "<text>query employee feed ksmallest[10; Salary] consume "
    "</text--->"
    ") )";

const string kbiggestSpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "( <text>stream(tuple([a1:d1, ... ,an:dn])))"
    " x int x a_k x  ... a_m -> "
    "(stream (tuple(...)))</text--->"
    "<text>_ kbiggest [k ; list ]</text--->"
    "<text>returns the k biggest elements from the input stream"
    "</text--->"
    "<text>query employee feed kbiggest[10; Salary] consume "
    "</text--->"
    ") )";

Operator ksmallest (
         "ksmallest",
         ksmallestSpec,
         ksmallestVM<true>,
         Operator::SimpleSelect,
         ksmallestTM
);

Operator kbiggest (
         "kbiggest",
         kbiggestSpec,
         ksmallestVM<false>,
         Operator::SimpleSelect,
         ksmallestTM
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

static const char* sortAscending = "asc";
static const char* sortDescending = "desc";

ListExpr SortByTypeMap( ListExpr args )
{

  if(nl->ListLength(args)!=2){
    return listutils::typeError("two arguments expected");
  }

  ListExpr stream = nl->First(args);
  if(!listutils::isTupleStream(stream)){
    return listutils::typeError("first argument must be a tuple stream");
  }
  ListExpr attrtype;
  string  attrname, argstr;
  ListExpr sortSpecification  = nl->Second(args);
  int numberOfSortAttrs = nl->ListLength(sortSpecification);

  if(numberOfSortAttrs < 0){
    return listutils::typeError("Attribute list may not be enpty!");
  }

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
    string err = "second argument must be a list with"
                 " elements of form: (attrname [asc, desc])|attrname";
    if(!nl->IsAtom(attributeSpecification) &&  (length != 2)){
     return listutils::typeError(err);
    }
    bool isAscending=true;
    if(length==2) {
      if(nl->AtomType(nl->First(attributeSpecification))!=SymbolType){
         return listutils::typeError(err);
      }
      attrname = nl->SymbolValue(nl->First(attributeSpecification));
      ListExpr order = nl->Second(attributeSpecification);
      if(listutils::isSymbol(order, sortAscending)){
         isAscending = true;
      } else if(listutils::isSymbol(order, sortDescending)){
         isAscending = false;
      } else {
        return listutils::typeError("invalid sorting criterion");
      }
    } else  {
      if(nl->AtomType(attributeSpecification)!=SymbolType){
         return listutils::typeError(err);
      }
      attrname = nl->SymbolValue(attributeSpecification);
    }

    int j = FindAttribute(nl->Second(nl->Second(stream)),
                          attrname, attrtype);
    if (j > 0) {
      sortOrderDescriptionLastElement =
        nl->Append(sortOrderDescriptionLastElement, nl->IntAtom(j));
      sortOrderDescriptionLastElement =
        nl->Append(sortOrderDescriptionLastElement,
        nl->BoolAtom(isAscending));
    } else {
      return listutils::typeError("Unknown attribute name found");
    }
  }
  return nl->ThreeElemList(nl->SymbolAtom(Symbol::APPEND()),
    sortOrderDescription, stream);
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
sortby_vm(Word* args, Word& result, int message, Word& local, Supplier s);
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
         "sortby_old",               // name
         SortBySpec,             // specification
         sortby_vm<false>,       // value mapping
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
  if(nl->ListLength(args)!=1){
    return listutils::typeError("one argument expected");
  }
  ListExpr arg = nl->First(args);
  if(!listutils::isTupleStream(arg)){
    return listutils::typeError("tuple stream expected");
  }
  return arg;

}
/*
2.12.2 Specification of operator ~sort\_old~

*/
const string SortSpec  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                         "\"Example\" ) "
                         "( <text>((stream (tuple([a1:d1, ... ,an:dn]"
                         ")))) -> (stream (tuple([a1:d1, ... ,an:dn])))"
                         "</text--->"
                         "<text>_ sort_old</text--->"
                         "<text>Sorts input stream lexicographically."
                         "</text--->"
                         "<text>query cities feed sort_old consume</text--->"
                         ") )";

/*
2.12.3 Definition of operator ~sort\_old~

*/
Operator extrelsort (
         "sort_old",             // name
         SortSpec,           // specification
         sortby_vm<true>,               // value mapping
         Operator::SimpleSelect,          // trivial selection function
         IdenticalTypeMap<true>         // type mapping
);

/*
2.13 Operator ~rdup~

This operator removes duplicates from a sorted stream.

2.13.1 Value mapping function of operator ~rdup~

*/

#ifndef USE_PROGRESS

// standard version

int RdupValueMapping(Word* args, Word& result, int message,
                     Word& local, Supplier s)
{
  Word tuple(Address(0));
  LexicographicalTupleCompareAlmost cmp;
  Tuple* current = 0;
  RTuple* lastOutput = 0;

  switch(message)
  {
    case OPEN: {
      qp->Open(args[0].addr);
      local.addr = 0;
      return 0;
    }
    case REQUEST: {
      while(true)
      {
        qp->Request(args[0].addr, tuple);
        if(qp->Received(args[0].addr))
        {
          // stream delivered a new tuple
          if(local.addr != 0)
          {
            // there is a last tuple
            current = static_cast<Tuple*>(tuple.addr);
            lastOutput = static_cast<RTuple*>(local.addr);
            if(cmp(current, lastOutput->tuple)
              || cmp(lastOutput->tuple, current))
            {
              // tuples are not equal. Return the tuple
              // stored in local info and store the current one
              // there.

        *lastOutput = RTuple( current );
              result = tuple;
              return YIELD;
            }
            else
            {
              // tuples are equal
              current->DeleteIfAllowed();
            }
          }
          else
          {
            // no last tuple stored
      local.addr = new RTuple( static_cast<Tuple*>(tuple.addr) );
      result = tuple;
            return YIELD;
          }
        }
  else
        {
          result.addr = 0;
          return CANCEL;
        }
      }
    }
    case CLOSE: {
      if( local.addr != 0 ){ // check if local is present
         lastOutput = static_cast<RTuple*>(local.addr);
         delete lastOutput;
         local.setAddr(0);
      }
      qp->Close(args[0].addr);
      return 0;
    }
  }
  return 0;
}

# else

// progress version


struct RdupLocalInfo
{
  RdupLocalInfo(): localTuple(0),read(0),returned(0),stableValue(50),cmp1(){}
  ~RdupLocalInfo(){
    if(localTuple){
      localTuple->DeleteIfAllowed();
      localTuple=0;
    }
  }
  Tuple* localTuple;
  int read, returned, stableValue;
  LexicographicalTupleCmpAlmost cmp1;
};


int RdupValueMapping(Word* args, Word& result, int message,
                     Word& local, Supplier s)
{

  RdupLocalInfo* rli = (RdupLocalInfo*) local.addr;

  switch(message)
  {
    case OPEN: {

      if ( rli ) {
         delete rli;
      }
      rli = new RdupLocalInfo();
      local.setAddr(rli);
      qp->Open(args[0].addr);
      return 0;
    } case REQUEST: {
      if(!rli){
         return CANCEL;
      }
      Tuple* currentTuple;
      Tuple* lastOutputTuple;
      Word tuple;

      while(true)
      {
        qp->Request(args[0].addr, tuple);
        if(qp->Received(args[0].addr))
        {
          // stream deliverd a new tuple

          rli->read++;
          if(rli->localTuple != 0)
          {
            // there is a last tuple

            currentTuple = (Tuple*)tuple.addr;
            lastOutputTuple = rli->localTuple;
            if(rli->cmp1(currentTuple, lastOutputTuple)!=0)
            {
              // tuples are not equal. Return the tuple
              // stored in local info and store the current one
              // there.
              lastOutputTuple->DeleteIfAllowed();

              rli->returned++;
              rli->localTuple = currentTuple;

              currentTuple->IncReference();
              result.setAddr(currentTuple);
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
            rli->returned++;
            rli->localTuple = currentTuple;
            currentTuple->IncReference();
            result.setAddr(currentTuple);
            return YIELD;
          }
        }
        else
        {
          // last tuple of the stream
          lastOutputTuple = rli->localTuple;
          if(lastOutputTuple != 0)
          {
            lastOutputTuple->DeleteIfAllowed();
            rli->localTuple = 0;
          }
          return CANCEL;
        }
      }
    } case CLOSE: {
      qp->Close(args[0].addr);
      return 0;

    } case CLOSEPROGRESS: {
      if ( rli )
      {
        delete rli;
        local.setAddr(0);
      }
      return 0;


    } case REQUESTPROGRESS: {
      ProgressInfo p1;
      ProgressInfo* pRes;
      const double uRdup = 0.01;  // time per tuple
      const double vRdup = 0.1;   // time per comparison
      const double wRdup = 0.9;   // default selectivity (= 10% duplicates)

      pRes = (ProgressInfo*) result.addr;

      if ( qp->RequestProgress(args[0].addr, &p1) )
      {
        pRes->CopySizes(p1);
        pRes->Time = p1.Time + p1.Card * uRdup * vRdup;

        pRes->CopyBlocking(p1);    //non-blocking operator

        if (rli) {
          if (rli->returned > rli->stableValue) {
            pRes->Card =  p1.Card *
              ((double) rli->returned / (double) (rli->read));

            if ( p1.BTime < 0.1 && pipelinedProgress ){  //non-blocking,
                                                        //use pipelining
              pRes->Progress = p1.Progress;
            } else {
              pRes->Progress = (p1.Progress * p1.Time
                + rli->read * uRdup * vRdup) / pRes->Time;
            }

            return YIELD;
          }
        }

        pRes->Card = p1.Card * wRdup;

        if ( p1.BTime < 0.1 && pipelinedProgress ){      //non-blocking,
                                                        //use pipelining
          pRes->Progress = p1.Progress;
        } else {
          pRes->Progress = (p1.Progress * p1.Time) / pRes->Time;
        }

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
  string err = " stream(tuple(X) x stream(tuple(X)) expected";
  if(nl->ListLength(args)!=2){
    return listutils::typeError(err);
  }
  if(!listutils::isTupleStream(nl->First(args)) ||
     !nl->Equal(nl->First(args), nl->Second(args))){
    return listutils::typeError(err);
  }
  return nl->First(args);
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
  LexicographicalTupleCmp tupleCmp;

  Word streamA;
  Word streamB;

  Tuple* currentATuple;
  Tuple* currentBTuple;

/*
~NextATuple~

Stores the next tuple from stream A into currentATuple which is
unequal to the currently store A tuple.

*/
  void NextATuple()
  {
    Word tuple;
    qp->Request(streamA.addr,tuple);
    if(!currentATuple){ // first tuple
       if(qp->Received(streamA.addr)){
          currentATuple = static_cast<Tuple*>(tuple.addr);
       }
    } else { // currentAtuple already exists
       while(qp->Received(streamA.addr) &&
             TuplesEqual(currentATuple, static_cast<Tuple*>(tuple.addr))){
          (static_cast<Tuple*>(tuple.addr))->DeleteIfAllowed();
          qp->Request(streamA.addr,tuple);
       }
       currentATuple->DeleteIfAllowed(); // remove from inputstream or buffer
       if(qp->Received(streamA.addr)){
         currentATuple = static_cast<Tuple*>(tuple.addr);
       } else {
         currentATuple=0;
       }
    }
  }

  void NextBTuple()
  {
    Word tuple;
    if(!currentBTuple){ // first tuple
       qp->Request(streamB.addr,tuple);
       if(qp->Received(streamB.addr)){
          currentBTuple = static_cast<Tuple*>( tuple.addr);
       }
    } else { // currentAtuple already exists
       qp->Request(streamB.addr,tuple);
       while(qp->Received(streamB.addr) &&
             TuplesEqual(currentBTuple, static_cast<Tuple*>(tuple.addr))){
          (static_cast<Tuple*>(tuple.addr))->DeleteIfAllowed();
          qp->Request(streamB.addr,tuple);
       }
       currentBTuple->DeleteIfAllowed();
       if(qp->Received(streamB.addr)){
         currentBTuple = static_cast<Tuple*>(tuple.addr);
       } else {
         currentBTuple=0;
       }
    }
  }


  bool TuplesEqual(Tuple* a, Tuple* b)
  {
    return (tupleCmp(a,b)==0);
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
    NextATuple();
    NextBTuple();
  }

  virtual ~SetOperation()
  {
    qp->Close(streamA.addr);
    qp->Close(streamB.addr);
    if(currentATuple){
      currentATuple->DeleteIfAllowed();
      currentATuple=0;
    }
    if(currentBTuple){
      currentBTuple->DeleteIfAllowed();
      currentBTuple=0;
    }
  }

  Tuple* NextResultTuple()
  {
    Tuple* result = 0;
    while(result == 0)
    {
      if(currentATuple == 0)
      {
        if(currentBTuple == 0)
        { // stream exhausted
          return 0;
        }
        else
        {
          if(outputBWithoutA)
          {
            result = currentBTuple;
            result->IncReference();
            NextBTuple();
          }
          else
          {
            currentBTuple->DeleteIfAllowed();
            currentBTuple = 0;
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
            result->IncReference();
            NextATuple();
          }
          else
          {
            currentATuple->DeleteIfAllowed();
            currentATuple=0;
            return 0;
          }
        }
        else
        {
          /* both current tuples != 0 */
          int cmp = tupleCmp(currentATuple, currentBTuple);
          if(cmp < 0)
          {
            if(outputAWithoutB)
            {
              result = currentATuple;
              result->IncReference();
            }
            NextATuple();
          }
          else if(cmp > 0)
          {
            if(outputBWithoutA)
            {
              result = currentBTuple;
              result->IncReference();
            }
            NextBTuple();
          }
          else
          {
            /* found match */
            Tuple* match = currentATuple;
            if(outputMatches)
            {
              result = match;
              result->IncReference();
            }
            NextATuple();
            NextBTuple();
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

      local.setAddr(localInfo);
      return 0;
    case REQUEST:
      localInfo = (SetOperation*)local.addr;
      result.setAddr(localInfo->NextResultTuple());
      return result.addr != 0 ? YIELD : CANCEL;
    case CLOSE:
      if(local.addr){
         localInfo = (SetOperation*)local.addr;
         delete localInfo;
         local.setAddr(0);
      }
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
bool arelTypeEqual (NList& t1, NList& t2)
{
  NList firstA, firstB;
  NList attrA = t1.second().second();
  NList attrB = t2.second().second();
  if (attrA.length() != attrB.length())
    return false;
  else
  {
    bool equal = true;
    while (!attrA.isEmpty() && equal)
    {
      firstA = attrA.first().second();
      firstB = attrB.first().second();
       if (firstA.hasLength(2) && firstB.hasLength(2) &&
           firstA.first().isSymbol() && firstB.first().isSymbol() &&
           firstA.first().str() == "arel" && firstB.first().str() == "arel")
        equal = arelTypeEqual(firstA, firstB);
      else
        equal = nl->Equal(firstA.listExpr(), firstB.listExpr());
      attrA.rest();
      attrB.rest();
    }
    return equal;
  }
}

template<bool OptionalIntAllowed, int defaultValue>
ListExpr JoinTypeMap (ListExpr args)
{
  string err = "stream(tuple[y1 : d1, ..., yn : dn]) x "
               "stream(tuple[z1 : e1, ..., zn : en]) x di x e1 ";



  if(OptionalIntAllowed){
    err += " [ x int]";
  }

  int len = nl->ListLength(args);

  // check for correct number of arguments
  if( (len!=4) && (len!=5)){
     return listutils::typeError(err);
  }
  if((len==5) && !OptionalIntAllowed){
     return listutils::typeError(err);
  }

  // check the first 4 arguments stream x stream x attrname x attrname
  ListExpr stream1 = nl->First(args);
  ListExpr stream2 = nl->Second(args);
  ListExpr attr1 = nl->Third(args);
  ListExpr attr2 = nl->Fourth(args);
  if(!listutils::isTupleStream(stream1) ||
     !listutils::isTupleStream(stream2) ||
     nl->AtomType(attr1)!=SymbolType ||
     nl->AtomType(attr2)!=SymbolType){
    return listutils::typeError(err);
  }

  // check the last element if present
  if(len==5){
     ListExpr size = nl->Fifth(args);
     if(!listutils::isSymbol(size,CcInt::BasicType())){
       return listutils::typeError(err + "(last arg is not an int");
     }
  }

  // check for correct naming of attributes
  ListExpr list1 = nl->Second(nl->Second(stream1));
  ListExpr list2 = nl->Second(nl->Second(stream2));
  if(!listutils::disjointAttrNames(list1,list2)){
    return listutils::typeError("Attribute lists are not disjoint");
  }


  ListExpr list = ConcatLists(list1, list2);
  ListExpr outlist = nl->TwoElemList(nl->SymbolAtom(Symbol::STREAM()),
      nl->TwoElemList(nl->SymbolAtom(Tuple::BasicType()), list));

  string attrAName = nl->SymbolValue(attr1);
  string attrBName = nl->SymbolValue(attr2);

  ListExpr attrTypeA, attrTypeB;

  int attrAIndex = listutils::findAttribute(list1, attrAName, attrTypeA);
  if(attrAIndex<1){
    return listutils::typeError("Attributename " + attrAName +
                                " not found in the first argument");
  }

  int attrBIndex = listutils::findAttribute(list2, attrBName, attrTypeB);
  if(attrBIndex<1){
    return listutils::typeError("Attributename " + attrBName +
                                " not found in the second argument");
  }

  NList attrA(attrTypeA);
  NList attrB(attrTypeB);
  if (attrA.hasLength(2) && attrB.hasLength(2) &&
      attrA.first().isSymbol() && attrB.first().isSymbol() &&
      attrA.first().str() == "arel" && attrB.first().str() == "arel")
  {
   if (!arelTypeEqual(attrA, attrB))
    return listutils::typeError("different types selected for operation");
  }
  else
  {
    if(!nl->Equal(attrTypeA, attrTypeB)){
      return listutils::typeError("different types selected for operation");
    }
  }

  ListExpr joinAttrDescription;

  if(!OptionalIntAllowed || len == 5){
      joinAttrDescription  = nl->TwoElemList(nl->IntAtom(attrAIndex),
                                             nl->IntAtom(attrBIndex));
  } else { // additionally add the default value
      joinAttrDescription = nl->ThreeElemList( nl->IntAtom(defaultValue),
                                               nl->IntAtom(attrAIndex),
                                               nl->IntAtom(attrBIndex));
  }
  return nl->ThreeElemList(nl->SymbolAtom(Symbol::APPEND()),
              joinAttrDescription, outlist);
}

/*
Some explicit instantiations in order to use them also
outside of the implementation file.

*/

template ListExpr
JoinTypeMap<false,1>(ListExpr args);


template ListExpr
JoinTypeMap<true,1>(ListExpr args);



/*
2.15.2 Value mapping functions of operator ~mergejoin~

*/

template<bool expectSorted> int
mergejoin_vm( Word* args, Word& result,
              int message, Word& local, Supplier s );

template<bool expectSorted> int
mergejoin_vm( Word* args, Word& result,
                  int message, Word& local, Supplier s );


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
         "mergejoin",             // name
         MergeJoinSpec,           // specification
         mergejoin_vm<true>,      // value mapping
         Operator::SimpleSelect,  // trivial selection function
         JoinTypeMap<false,0>    // type mapping
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
         "sortmergejoin_old",            // name
         SortMergeJoinSpec,          // specification
         mergejoin_vm<false>,        // value mapping
         Operator::SimpleSelect,     // trivial selection function
         JoinTypeMap<false,0>       // type mapping
);

/*
2.17 Operator ~hashjoin~

This operator computes the equijoin two streams via a hash join.
The user can specify the number of hash buckets.

2.17.2 Specification of Operator ~hashjoin~

*/
const string HashJoinSpec  = "( ( \"Signature\" \"Syntax\" "
                             "\"Meaning\" \"Example\" \"Remark\" ) "
                          "( <text>((stream (tuple ((x1 t1) ... "
                          "(xn tn)))) (stream (tuple ((y1 d1) ... "
                          "(ym dm)))) xi yj [nbuckets]) -> (stream "
                          "(tuple ((x1 t1) ... (xn tn) (y1 d1) ..."
                          " (ym dm))))</text--->"
                          "<text> _ _ hashjoin [ _ , _ , _ ]"
                          "</text--->"
                          "<text>Computes the equijoin two streams "
                          "via a hash join. The number of hash buckets"
                          " is given by the parameter nBuckets. If the number"
                          " of buckets is omitted, a default value of 99997"
                          " is used"
                          "</text--->"
                          "<text>query Employee feed Dept feed "
                          "rename[A] hashjoin[DeptNr, DeptNr_A, 17] "
                          "sort consume</text--->"
                          "<text>The hash table is created from the 2nd "
                          "argument. If it does not fit into memory, "
                          "the 1st argument will also be materialized."
                          "</text--->"
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
         JoinTypeMap<true, 99997>   // type mapping
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
  ListExpr errorInfo = nl->OneElemList(nl->SymbolAtom("ERROR"));

  if(nl->ListLength(args)!=2){
    ErrorReporter::ReportError("two elements expected");
    return nl->TypeError();
  }

  ListExpr stream = nl->First(args);

  if(!IsStreamDescription(stream)){
    ErrorReporter::ReportError("first argument is not a tuple stream");
    return nl->TypeError();
  }

  ListExpr tuple = nl->Second(stream);

  ListExpr functions = nl->Second(args);
  if(nl->ListLength(functions)<1){
    ErrorReporter::ReportError("at least one function expected");
    return nl->TypeError();
  }

  // copy attrlist to newattrlist
  ListExpr attrList = nl->Second(nl->Second(stream));
  ListExpr newAttrList = nl->OneElemList(nl->First(attrList));
  ListExpr lastlistn = newAttrList;
  attrList = nl->Rest(attrList);
  while (!(nl->IsEmpty(attrList)))
  {
     lastlistn = nl->Append(lastlistn,nl->First(attrList));
     attrList = nl->Rest(attrList);
  }

  // reset attrList
  attrList = nl->Second(nl->Second(stream));
  ListExpr typeList;


  // check functions
  set<string> usedNames;

  while (!(nl->IsEmpty(functions)))
  {
    ListExpr function = nl->First(functions);
    functions = nl->Rest(functions);
    if(nl->ListLength(function)!=2){
      ErrorReporter::ReportError("invalid extension found");
      return nl->TypeError();
    }
    ListExpr name = nl->First(function);
    ListExpr map  = nl->Second(function);

    if(nl->AtomType(name)!=SymbolType){
      ErrorReporter::ReportError("invalid attribute name");
      return nl->TypeError();
    }

    string namestr = nl->SymbolValue(name);
    int pos = FindAttribute(attrList,namestr,typeList);
    if(pos!=0){
       ErrorReporter::ReportError("Attribute "+ namestr +
                                  " already member of the tuple");
       return nl->TypeError();
    }

    string symcheckmsg = "";
    if(!SecondoSystem::GetCatalog()->IsValidIdentifier(namestr,symcheckmsg)){
      return listutils::typeError("attribute name "+ symcheckmsg +".");
    }

    if(nl->ListLength(map)!=3){
      ErrorReporter::ReportError("invalid function");
      return nl->TypeError();
    }

    if(!nl->IsEqual(nl->First(map),Symbol::MAP())){
      ErrorReporter::ReportError("invalid function");
      return nl->TypeError();
    }

    ListExpr funResType = nl->Third(map);
    if(!am->CheckKind(Kind::DATA(),funResType, errorInfo)){
      ErrorReporter::ReportError("requested attribute " + namestr +
                                 "not in kind DATA");
      return nl->TypeError();
    }

    ListExpr funArgType = nl->Second(map);

    if(!nl->Equal(funArgType, tuple)){
      ErrorReporter::ReportError("function type different to the tuple type");
      return nl->TypeError();
    }

    if(usedNames.find(namestr)!=usedNames.end()){
      ErrorReporter::ReportError("Name "+ namestr + "occurs twice");
      return nl->TypeError();
    }
    usedNames.insert(namestr);
    // append attribute
    lastlistn = nl->Append(lastlistn, (nl->TwoElemList(name, funResType )));
  }

  return nl->TwoElemList(nl->SymbolAtom(Symbol::STREAM()),
            nl->TwoElemList(nl->SymbolAtom(Tuple::BasicType()),newAttrList));
}
/*
2.18.2 Value mapping function of operator ~extend~

*/

#ifndef USE_PROGRESS

// standard version

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
      local.setAddr( resultTupleType );
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
          ((*funargs)[0]).setAddr(tup);
          qp->Request(supplier3,value);
          newTuple->PutAttribute( tup->GetNoAttributes()+i,
                                  ((Attribute*)value.addr)->Clone() );
        }
        tup->DeleteIfAllowed();
        result.setAddr(newTuple);
        return YIELD;
      }
      else
        return CANCEL;

    case CLOSE :
      if(local.addr)
      {
         ((TupleType *)local.addr)->DeleteIfAllowed();
         local.setAddr(0);
      }
      qp->Close(args[0].addr);
      return 0;
  }
  return 0;
}


# else

// progress version


class ExtendLocalInfo: public ProgressLocalInfo
{
public:

  ExtendLocalInfo():resultTupleType(0),stableValue(0),
                    sizesFinal(false), noOldAttrs(0),
                    noNewAttrs(0), attrSizeTmp(0),
                    attrSizeExtTmp(0) {};

  ~ExtendLocalInfo() {
    if(resultTupleType){
      resultTupleType->DeleteIfAllowed();
    }
    if(attrSizeTmp){
       delete [] attrSizeTmp;
    }
    if(attrSizeExtTmp){
       delete [] attrSizeExtTmp;
    }
  }

  TupleType *resultTupleType;
  int stableValue;
  bool sizesFinal;        //true after stableValue reached
  int noOldAttrs, noNewAttrs;
  double *attrSizeTmp;
  double *attrSizeExtTmp;
};


int Extend(Word* args, Word& result, int message, Word& local, Supplier s)
{
  Word t, value;
  Tuple* tup;
  Supplier supplier, supplier2, supplier3;
  int nooffun;
  ArgVectorPointer funargs;
  ExtendLocalInfo *eli;

  eli = (ExtendLocalInfo*) local.addr;


  switch (message)
  {
    case OPEN :

      if ( eli ) {
         delete eli;
      }

      eli = new ExtendLocalInfo;
      eli->resultTupleType = new TupleType(nl->Second(GetTupleResultType(s)));
      eli->read = 0;
      eli->stableValue = 50;
      eli->sizesFinal = false;
      eli->noNewAttrs = qp->GetNoSons(args[1].addr);
      eli->attrSizeTmp = new double[eli->noNewAttrs];
      eli->attrSizeExtTmp = new double[eli->noNewAttrs];
      for (int i = 0; i < eli->noNewAttrs; i++)
      {
        eli->attrSizeTmp[i] = 0.0;
        eli->attrSizeExtTmp[i] = 0.0;
      }

      local.setAddr(eli);

      qp->Open(args[0].addr);

      return 0;

    case REQUEST :
      if(!eli){
        return CANCEL;
      }
      qp->Request(args[0].addr,t);
      if (qp->Received(args[0].addr))
      {
        tup = (Tuple*)t.addr;
        Tuple *newTuple = new Tuple( eli->resultTupleType );
        eli->read++;
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
          ((*funargs)[0]).setAddr(tup);
          qp->Request(supplier3,value);
          newTuple->PutAttribute( tup->GetNoAttributes()+i,
                                  ((Attribute*)value.addr)->Clone() );

          if (eli->read <= eli->stableValue)
          {
            eli->attrSizeTmp[i] += newTuple->GetSize(tup->GetNoAttributes()+i);
            eli->attrSizeExtTmp[i] +=
              newTuple->GetExtSize(tup->GetNoAttributes()+i);
          }

        }
        tup->DeleteIfAllowed();
        result.setAddr(newTuple);
        return YIELD;
      }
      else
        return CANCEL;

    case CLOSE :
      qp->Close(args[0].addr);
      return 0;

    case CLOSEPROGRESS:
      if ( eli ){
         delete eli;
         local.setAddr(0);
      }
      return 0;


    case REQUESTPROGRESS:

      ProgressInfo p1;
      ProgressInfo *pRes;
      const double uExtend = 0.0012;   //millisecs per tuple
      const double vExtend = 0.00085;   //millisecs per tuple and attribute

      pRes = (ProgressInfo*) result.addr;

      if ( !eli ) {
         return CANCEL;
      }

      if ( qp->RequestProgress(args[0].addr, &p1) )
      {
        eli->sizesChanged = false;

        if ( !eli->sizesInitialized )
        {
          eli->noOldAttrs = p1.noAttrs;
          eli->noAttrs = eli->noOldAttrs + eli->noNewAttrs;
          eli->attrSize = new double[eli->noAttrs];
          eli->attrSizeExt = new double[eli->noAttrs];
        }

        if ( !eli->sizesInitialized || p1.sizesChanged ||
             ( eli->read >= eli->stableValue && !eli->sizesFinal ) )
        {
          eli->Size = 0.0;
          eli->SizeExt = 0.0;

        //old attrs
          for (int i = 0; i < eli->noOldAttrs; i++)
          {
            eli->attrSize[i] = p1.attrSize[i];
            eli->attrSizeExt[i] = p1.attrSizeExt[i];
            eli->Size += eli->attrSize[i];
            eli->SizeExt += eli->attrSizeExt[i];
          }

            //new attrs
          if ( eli->read < eli->stableValue )
          {
            for (int j = 0; j < eli->noNewAttrs; j++)
            {
              eli->attrSize[j + eli->noOldAttrs] = 12;
              eli->attrSizeExt[j + eli->noOldAttrs] = 12;
              eli->Size += eli->attrSize[j + eli->noOldAttrs];
              eli->SizeExt += eli->attrSizeExt[j + eli->noOldAttrs];
            }
          }
          else
          {
            for (int j = 0; j < eli->noNewAttrs; j++)
            {
              eli->attrSize[j + eli->noOldAttrs] = eli->attrSizeTmp[j] /
                eli->stableValue;
              eli->attrSizeExt[j + eli->noOldAttrs] = eli->attrSizeExtTmp[j] /
                eli->stableValue;
              eli->Size += eli->attrSize[j + eli->noOldAttrs];
              eli->SizeExt += eli->attrSizeExt[j + eli->noOldAttrs];
            }
          }
          eli->sizesInitialized = true;
          eli->sizesChanged = true;
        }
        //ensure sizes are updated only once for passing the threshold
        if ( eli->read >= eli->stableValue ) eli->sizesFinal = true;


        pRes->Card = p1.Card;

        pRes->CopySizes(eli);

        pRes->Time = p1.Time + p1.Card * (uExtend + eli->noNewAttrs * vExtend);


        if ( p1.BTime < 0.1 && pipelinedProgress )      //non-blocking,
                                                        //use pipelining
          pRes->Progress = p1.Progress;
        else
          pRes->Progress =
            (p1.Progress * p1.Time +
              eli->read * (uExtend + eli->noNewAttrs * vExtend))
            / pRes->Time;

        pRes->CopyBlocking(p1);    //non-blocking operator

        return YIELD;
      } else {
        return CANCEL;
      }
  }
  return 0;
}

#endif


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

 if(nl->ListLength(args)!=2){
   return listutils::typeError("two arguments expected");
 }
 ListExpr stream = nl->First(args);
 ListExpr map = nl->Second(args);
 string err = "stream(tuple (a)) x ( tuple(a) -> stream(tuple(b))) expected";
 if(!listutils::isTupleStream(stream) ||
    !listutils::isMap<1>(map)){
   return listutils::typeError(err);
 }

 ListExpr maparg = nl->Second(map);
 ListExpr mapres = nl->Third(map);

 if(!nl->Equal(nl->Second(stream), maparg)){
   return listutils::typeError( err +
                               " (function argument differs from tuple)");
 }

 if(!listutils::isTupleStream(mapres)){
   return listutils::typeError(err +
                               " (function result is not a tuple stream");
 }

 ListExpr alist1 = nl->Second(nl->Second(stream));
 ListExpr alist2 = nl->Second(nl->Second(mapres));

 if(!listutils::disjointAttrNames(alist1, alist2)){
   return listutils::typeError(err +
                               " ( name conflict in  tuples");
 }
 ListExpr list = ConcatLists(alist1,alist2);

 return nl->TwoElemList(nl->SymbolAtom(Symbol::STREAM()),
                        nl->TwoElemList(nl->SymbolAtom(Tuple::BasicType()),
                                        list));
}

/*
2.19.2 Value mapping function of operator ~loopjoin~

SPM: There is a problem when this operator is requested after
it has returned CANCEL it will chrash since it tryes to do a
request on a NULL pointer.

*/

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

  Word tuplex(Address(0));
  Word tupley(Address(0));
  Word tuplexy(Address(0));
  Word streamy(Address(0));

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
        local.setAddr(localinfo);
      }
      else
      {
        local.setAddr(0);
      }
      return 0;

    case REQUEST:
      if (local.addr ==0) return CANCEL;
      localinfo=(LoopjoinLocalInfo *) local.addr;
      tuplex=localinfo->tuplex;
      ctuplex=(Tuple*)tuplex.addr;
      streamy=localinfo->streamy;
      tupley.setAddr(0);
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
            tupley.setAddr(0);

            localinfo->tuplex=tuplex;
            localinfo->streamy=streamy;
            local.setAddr(localinfo);
          }
          else
          {
            localinfo->streamy.setAddr(0);
            localinfo->tuplex.setAddr(0);
            return CANCEL;
          }
        }
        else
        {
          ctupley=(Tuple*)tupley.addr;
        }
      }
      ctuplexy = new Tuple( localinfo->resultTupleType );
      tuplexy.setAddr(ctuplexy);
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
        local.setAddr(0);
      }
      qp->Close(args[0].addr);
      return 0;
  }

  return 0;
}


#else

// with support for progress queries



class LoopjoinLocalInfo: public ProgressLocalInfo
{
public:
  LoopjoinLocalInfo():tuplex(Address(0)),streamy(Address(0)),
                      resultTupleType(0) {}
  ~LoopjoinLocalInfo() {
     if(resultTupleType){
       resultTupleType->DeleteIfAllowed();
       resultTupleType=0;
     }
  }

  Word tuplex;
  Word streamy;
  TupleType *resultTupleType;

};


int Loopjoin(Word* args, Word& result, int message,
             Word& local, Supplier s)
{
  ArgVectorPointer funargs = 0;

  Word tuplex(Address(0));
  Word tupley(Address(0));
  Word tuplexy(Address(0));
  Word streamy(Address(0));

  Tuple* ctuplex = 0;
  Tuple* ctupley = 0;
  Tuple* ctuplexy = 0;

  ListExpr resultType;

  LoopjoinLocalInfo* lli;

  lli = (LoopjoinLocalInfo *) local.addr;

  switch ( message )
  {
    case OPEN:

      if ( lli ) {
         delete lli;
      }

      lli = new LoopjoinLocalInfo();
      resultType = GetTupleResultType( s );
      lli->resultTupleType =
           new TupleType( nl->Second( resultType ) );

      local.setAddr(lli);

      qp->Open (args[0].addr);
      qp->Request(args[0].addr, tuplex);
      if (qp->Received(args[0].addr))
      {
        lli->readFirst++;
        funargs = qp->Argument(args[1].addr);
        (*funargs)[0] = tuplex;
        streamy=args[1];
        qp->Open (streamy.addr);

        lli->tuplex = tuplex;
        lli->streamy = streamy;

      }
      else
      {
        lli->tuplex.setAddr(0);
        lli->streamy.setAddr(0);
      }
      return 0;

    case REQUEST:
      if(!lli){
        return CANCEL;
      }
      if ( lli->tuplex.addr == 0) {
         return CANCEL;
      }

      tuplex=lli->tuplex;
      ctuplex=(Tuple*)tuplex.addr;
      streamy=lli->streamy;
      tupley.setAddr(0);
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
            lli->readFirst++;
            funargs = qp->Argument(args[1].addr);
            ctuplex=(Tuple*)tuplex.addr;
            (*funargs)[0] = tuplex;
            streamy=args[1];
            qp->Open (streamy.addr);
            tupley.setAddr(0);

            lli->tuplex=tuplex;
            lli->streamy=streamy;
          }
          else
          {
            lli->streamy.setAddr(0);
            lli->tuplex.setAddr(0);
            return CANCEL;
          }
        }
        else
        {
          ctupley=(Tuple*)tupley.addr;
        }
      }
      ctuplexy = new Tuple( lli->resultTupleType );
      tuplexy.setAddr(ctuplexy);
      Concat(ctuplex, ctupley, ctuplexy);
      ctupley->DeleteIfAllowed();

      lli->returned++;
      result = tuplexy;
      return YIELD;

    case CLOSE:
      qp->Close(args[0].addr);
      return 0;

    case CLOSEPROGRESS:
      if ( lli )
      {
         if ( lli->tuplex.addr != 0 )
         {
           if( lli->streamy.addr != 0 )
            qp->Close( lli->streamy.addr );

           if( lli->tuplex.addr != 0 )
             ((Tuple*)lli->tuplex.addr)->DeleteIfAllowed();

         }
         delete lli;
         local.setAddr(0);
      }
      return 0;


    case REQUESTPROGRESS:
    {
      ProgressInfo p1, p2;
      ProgressInfo *pRes;
      //const double uLoopjoin = 0.2;  //not used

      pRes = (ProgressInfo*) result.addr;

      if (!lli) {
         return CANCEL;
      }

      if (qp->RequestProgress(args[0].addr, &p1)
       && qp->RequestProgress(args[1].addr, &p2))
      {
        lli->SetJoinSizes(p1, p2);



        if (lli->returned > enoughSuccessesJoin)
        {
          pRes->Card = p1.Card  *
            ((double) (lli->returned) /
              (double) (lli->readFirst));
        }
        else
          pRes->Card = p1.Card  * p2.Card;

        pRes->CopySizes(lli);

        pRes->Time = p1.Time + p1.Card * p2.Time;


        if ( p1.BTime < 0.1 && pipelinedProgress )      //non-blocking,
                                                        //use pipelining
          pRes->Progress = p1.Progress;
        else
          pRes->Progress =
            (p1.Progress * p1.Time + (double) lli->readFirst * p2.Time)
          / pRes->Time;

        pRes->CopyBlocking(p1);  //non-blocking operator;
        //second argument assumed not to block

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
2.19.3 Specification of operator ~loopjoin~

*/
const string LoopjoinSpec  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
  "\"Example\" ) "
  "( <text>((stream tuple1) (fun (tuple1 -> "
  "stream(tuple2)))) -> (stream tuple1*tuple2)"
  "</text--->"
  "<text>_ loopjoin [ fun ]</text--->"
  "<text>Creates the product of all tuples from the first argument "
  "with all tuples from the stream created by the second (function) argument. "
  "Note: The input tuples must "
  "have different attribute names, hence renaming may be applied "
  "to one of the input streams.</text--->"
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
 if(nl->ListLength(args)!=2){
   return listutils::typeError("two arguments expected");
 }
 ListExpr stream = nl->First(args);
 ListExpr map = nl->Second(args);
 string err = "stream(tuple (a)) x ( tuple(a) -> stream(tuple(b))) expected";
 if(!listutils::isTupleStream(stream) ||
    !listutils::isMap<1>(map)){
   return listutils::typeError(err);
 }

 ListExpr maparg = nl->Second(map);
 ListExpr mapres = nl->Third(map);

 if(!nl->Equal(nl->Second(stream), maparg)){
   return listutils::typeError( err +
                               " (function argument differs from tuple)");
 }

 if(!listutils::isTupleStream(mapres)){
   return listutils::typeError(err +
                               " (function result is not a tuple stream");
 }

 return mapres;
}

/*

4.1.2 Value mapping function of operator ~loopsel~

*/

#ifndef USE_PROGRESS

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
        local.setAddr(localinfo);
      }
      else
      {
        local.setAddr(0);
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
      tupley.setAddr(0);
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
            tupley.setAddr(0);

            localinfo->tuplex=tuplex;
            localinfo->streamy=streamy;
            local.setAddr(localinfo);
          }
          else
          {
            localinfo->streamy.setAddr(0);
            localinfo->tuplex.setAddr(0);
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
        local.setAddr(0);
      }
      qp->Close(args[0].addr);
      return 0;
  }

  return 0;
}
#else

struct LoopselectLocalInfo: public ProgressLocalInfo
{
  LoopselectLocalInfo(): ::ProgressLocalInfo(),tuplex(Address(0)),
                         streamy(Address(0)), resultTupleType(0){}

  ~LoopselectLocalInfo(){
     if(resultTupleType){
       resultTupleType->DeleteIfAllowed();
       resultTupleType = 0;
     }
  }

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
      localinfo = static_cast<LoopselectLocalInfo*>(local.addr);
      if(localinfo){
        delete localinfo;
        localinfo=0;
      }
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
        localinfo->readFirst=1;   // first tuple read
        localinfo->returned=0;
        local.setAddr(localinfo);
      }
      else
      {
        local.setAddr(0);
      }
      return 0;

    case REQUEST:
      if (local.addr ==0){
          return CANCEL;
      }

      // restore localinformation from the local variable.
      localinfo = (LoopselectLocalInfo *) local.addr;
      tuplex = localinfo->tuplex;
      ctuplex = (Tuple*)tuplex.addr;
      streamy = localinfo->streamy;
      // prepare tuplex and tupley for processing.
      // if rely is exausted: fetch next tuplex.
      tupley.setAddr(0);
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
            localinfo->readFirst++;         // another tuple read
            funargs = qp->Argument(args[1].addr);
            ctuplex = (Tuple*)tuplex.addr;
            (*funargs)[0] = tuplex;
            streamy = args[1];
            qp->Open(streamy.addr);
            tupley.setAddr(0);

            localinfo->tuplex=tuplex;
            localinfo->streamy=streamy;
            local.setAddr(localinfo);
          }
          else
          {
            localinfo->streamy.setAddr(0);
            localinfo->tuplex.setAddr(0);
            return CANCEL;
          }
        }
        else
        {
          ctupley = (Tuple*)tupley.addr;
        }
      }
      localinfo->returned++;
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

        delete localinfo;
        local.setAddr(0);
      }
      qp->Close(args[0].addr);
      return 0;
    case CLOSEPROGRESS:
      return 0;
    case REQUESTPROGRESS:
      localinfo = (LoopselectLocalInfo *) local.addr;
      if (localinfo)
      {
        ProgressInfo p1, p2;
        ProgressInfo *pRes;
        pRes = (ProgressInfo*) result.addr;
        if (qp->RequestProgress(args[0].addr, &p1) &&
            qp->RequestProgress(args[1].addr, &p2))
        {
          if (localinfo->readFirst>1)
          {
       // cardinality is estimated from the returned tuples and the read tuples
            pRes->Card = p1.Card * localinfo->returned / localinfo->readFirst;
          }
          else
          {
        // default guess is the multiplication of cardinalities
            pRes->Card = p1.Card * p2.Card;
          }

          pRes->CopySizes(p2);  // tuples are axtracted from fun relation
          // for each tuplex, all matching tupley will be collected,
          // therefore the time for the query for tupley is multiplied
          pRes->Time = p1.Time + p1.Card * p2.Time;

          pRes->Progress =
              (p1.Progress * p1.Time + (double) localinfo->readFirst * p2.Time)
              / pRes->Time;
          // progress depends on how many tuplex have been done (with readFirst)

          pRes->CopyBlocking(p1);  //non-blocking operator;
          return YIELD;
        }

      } else {
        return CANCEL;
      }

  }

  return -1;
}
#endif
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
  NList Args(args);
  if(!Args.hasLength(2)){
     return NList::typeError("expecting two arguments");
  }
  NList Stream = Args.first();


  // check the tuple stream
  if(!Stream.hasLength(2) ||
     !Stream.first().isSymbol(Symbol::STREAM()) ||
     !Stream.second().hasLength(2) ||
     !Stream.second().first().isSymbol(Tuple::BasicType()) ||
     !IsTupleDescription(Stream.second().second().listExpr())
     ){
     return NList::typeError("first argument is not a tuple stream");
  }

  // second argument must be a list of length one
  NList Second = Args.second();
  if(!Second.isNoAtom() ||
     !Second.hasLength(1)){
     return NList::typeError("error in second argument");
  }

  // check for ( <attrname> <map> )
  NList  NameMap = Second.first();
  if(!NameMap.hasLength(2)){
    return NList::typeError("expecting (attrname map) as second argument");
  }

  // attrname must be an identifier
  if(!NameMap.first().isSymbol()){
    return NList::typeError("invalid representation of <attrname>");
  }

  // check map
  NList Map = NameMap.second();

  // Map must have format (map <tuple> <stream>)
  if(!Map.hasLength(3) ||
     !Map.first().isSymbol(Symbol::MAP()) ||
     !Map.second().hasLength(2) ||
     !Map.second().first().isSymbol(Tuple::BasicType()) ||
     !Map.third().hasLength(2) ||
     !Map.third().first().isSymbol(Symbol::STREAM())){
     return NList::typeError("expecting (map ( (tuple(...) (stream DATA))))"
                             " as second argument");
  }

  // the argumenttype for map must be equal to the tuple type from the stream
  if(Stream.second() != Map.second()){
     return NList::typeError("different tuple descriptions detected");
  }

  NList  typelist  = Map.third().second();

  ListExpr errorInfo = nl->OneElemList(nl->SymbolAtom("ERROR"));
  // check for Kind Data
  if(!am->CheckKind(Kind::DATA(),typelist.listExpr(),errorInfo)){
    return NList::typeError("stream elements not in kind DATA");
  }

  // check if argname is already used in the original tuple

  string attrname = NameMap.first().str();
  ListExpr attrtype=nl->TheEmptyList();
  int index = FindAttribute(Stream.second().second().listExpr(),
                            attrname,attrtype);
  if(index){ // attrname already used
    return NList::typeError("attrname already used in the original stream");
  }

  ListExpr attrlist = ConcatLists(Stream.second().second().listExpr(),
                                  nl->OneElemList(
                                    nl->TwoElemList(
                                      nl->SymbolAtom(attrname),
                                      Map.third().second().listExpr())));

  return  nl->TwoElemList(nl->SymbolAtom(Symbol::STREAM()),
                nl->TwoElemList( nl->SymbolAtom(Tuple::BasicType()),attrlist));
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
        localinfo->streamY.setAddr( supplier3 );
        local.setAddr(localinfo);
      }
      else
      {
        local.setAddr(0);
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
      wValueY.setAddr(0);
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
            localinfo->streamY.setAddr(supplier3);

            wValueY.setAddr(0);
          }
          else  //streamx is exausted
          {
            localinfo->streamY.setAddr(0);
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
                             (Attribute*)wValueY.addr );

      // setting the result
      result.setAddr( tupleXY );
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
        local.setAddr(0);
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

 string err = "stream(tuple(...)) x (a1,..., an) x "
              "( (b1  map1) ... (bm : mapm)) expected";

 if(nl->ListLength(args)!=3){
   return listutils::typeError("wrong number of arguments, expected: 3");
 }

 ListExpr stream = nl->First(args);
 ListExpr attrList = nl->Second(args);
 ListExpr extList = nl->Third(args);

 if(!listutils::isTupleStream(stream)){
   return listutils::typeError(err + " first arg is not a tuple stream");
 }

 // check whether all elements of attrlist are attrnames in the
 // first argument
 if(nl->AtomType(attrList)!=NoAtom){
   return listutils::typeError(err +
                          " second arg is not a list of attribute names");
 }

 ListExpr streamattr = nl->Second(nl->Second(stream));
 set<string> names;
 int noAttrs;
 ListExpr numberList;

 ListExpr newAttrList;
 ListExpr lastNewAttrList;
 ListExpr lastNumberList;
 bool firstCall = true;

 if(nl->IsEmpty(attrList)){
   noAttrs = 0;
   numberList = nl->TheEmptyList();
   newAttrList = nl->TheEmptyList();
 } else {
   noAttrs = nl->ListLength(attrList);
   ListExpr attrType;
   while(!nl->IsEmpty(attrList)){
     ListExpr attr = nl->First(attrList);
     attrList = nl->Rest(attrList);
     if(nl->AtomType(attr)!=SymbolType){
       return listutils::typeError(err + "(invalid attr name found)");
     }
     string attrName = nl->SymbolValue(attr);
     if(names.find(attrName)!=names.end()){
       return listutils::typeError(err + "(attr name found twice)");
     }
     names.insert(attrName);
     int j = listutils::findAttribute(streamattr, attrName, attrType);
     if(j==0){
       return listutils::typeError(err + "(attr name "+
                                   attrName + " not found)");
     }
     if (firstCall) {
       firstCall = false;
       newAttrList = nl->OneElemList(nl->TwoElemList(attr, attrType));
       lastNewAttrList = newAttrList;
       numberList = nl->OneElemList(nl->IntAtom(j));
       lastNumberList = numberList;
     } else {
       lastNewAttrList = nl->Append(lastNewAttrList,
                                    nl->TwoElemList(attr, attrType));
       lastNumberList = nl->Append(lastNumberList, nl->IntAtom(j));
     }
   }
 }

 // check the third argument (must be a list of pairs (attrname map))
 // the argument of map must be the tuple type in the stream and the
 // result must be in kind DATA
 if(nl->IsAtom(extList)){
   return listutils::typeError(err + " (wrong extension list)");
 }

 ListExpr tupletype = nl->Second(stream);
 while(!nl->IsEmpty(extList)){
   ListExpr ext = nl->First(extList);
   extList = nl->Rest(extList);
   if(nl->ListLength(ext)!=2){
     return listutils::typeError(err + " (problem in extension list");
   }
   ListExpr nameL = nl->First(ext);
   ListExpr map = nl->Second(ext);
   if(nl->AtomType(nameL)!=SymbolType ||
      !listutils::isMap<1>(map)){
     return listutils::typeError(err + " (problem in extension list");
   }
   string name = nl->SymbolValue(nameL);
   if(names.find(name)!=names.end()){
     return listutils::typeError(err +
                              "( conflicting names found " + name+")");
   }
   names.insert(name);
   if(!nl->Equal(tupletype,nl->Second(map))){
    return listutils::typeError(err + "( invalid argument type for map)");
   }
   ListExpr mapres = nl->Third(map);
   if(!listutils::isDATA(mapres)){
     return listutils::typeError(err + " (map result for " + name
                                 + " not in kind DATA)");
   }
   if(firstCall) {
     firstCall = false;
     newAttrList = nl->OneElemList(nl->TwoElemList(nameL, mapres));
     lastNewAttrList = newAttrList;
   } else {
     lastNewAttrList =
     nl->Append(lastNewAttrList, nl->TwoElemList(nameL,mapres));
   }
 }

 if(nl->IsEmpty(newAttrList)){
   return listutils::typeError(err +  "(resulting tuple would be empty");
 }


  return nl->ThreeElemList(
               nl->SymbolAtom(Symbol::APPEND()),
               nl->TwoElemList( nl->IntAtom(noAttrs),
                                numberList),
               nl->TwoElemList( nl->SymbolAtom(Symbol::STREAM()),
                            nl->TwoElemList( nl->SymbolAtom(Tuple::BasicType()),
                                                 newAttrList)));

}

/*
2.20.2 Value Mapping for operator ~projectextend~

*/

#ifndef USE_PROGRESS

// standard version

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
          ((*extFunArgs)[0]).setAddr(currTuple);     // pass argument
          qp->Request(supplier3,value);              // call extattr mapping
//  The original implementation tried to avoid copying the function result,
//  but somehow, this results in a strongly growing tuplebuffer on disk:
//           resultTuple->PutAttribute(
//             noOfAttrs + i, (Attribute*)value.addr );
//           qp->ReInitResultStorage( supplier3 );
          resultTuple->PutAttribute( noOfAttrs + i,
                ((Attribute*)value.addr)->Clone() );
        }
        currTuple->DeleteIfAllowed();
        result.setAddr(resultTuple);
        return YIELD;
      }
      else return CANCEL;
    }
    case CLOSE :
    {
      if(local.addr)
      {
        ((TupleType *)local.addr)->DeleteIfAllowed();
        qp->Close(args[0].addr);
        local.setAddr(0);
      }
      return 0;
    }
  }
  return 0;
}

# else

// progress version

class ProjectExtendLocalInfo: public ProgressLocalInfo
{
public:

  ProjectExtendLocalInfo():
        resultTupleType(0),stableValue(0), sizesFinal(false),
        noOldAttrs(0),noNewAttrs(0),attrSizeTmp(0), attrSizeExtTmp(0) {};

  ~ProjectExtendLocalInfo() {
    if(resultTupleType){
       resultTupleType->DeleteIfAllowed();
       resultTupleType=0;
    }
    if(attrSizeTmp){
       delete [] attrSizeTmp;
       attrSizeTmp =0;
    }
    if(attrSizeExtTmp){
       delete [] attrSizeExtTmp;
       attrSizeExtTmp =0;
    }
  }

  TupleType *resultTupleType;
  int stableValue;
  bool sizesFinal;
  int noOldAttrs, noNewAttrs;
  double *attrSizeTmp;
  double *attrSizeExtTmp;
};

int
ExtProjectExtendValueMap(Word* args, Word& result, int message,
                         Word& local, Supplier s)
{
  //  cout << "ExtProjectExtendValueMap() called." << endl;
  Word elem1, elem2, value;
  int index=0;
  Supplier son, supplier, supplier2, supplier3;
  ArgVectorPointer extFunArgs;

  ProjectExtendLocalInfo *eli;
  eli = (ProjectExtendLocalInfo*) local.addr;

  switch (message)
  {
    case OPEN :
    {
      if ( eli ) {
        delete eli;
      }

      eli = new ProjectExtendLocalInfo;
      eli->resultTupleType = new TupleType(nl->Second(GetTupleResultType(s)));
      eli->read = 0;
      eli->stableValue = 50;
      eli->sizesFinal = false;
      eli->noOldAttrs = ((CcInt*)args[3].addr)->GetIntval();
      eli->noNewAttrs = qp->GetNoSons(args[2].addr);
      eli->attrSizeTmp = new double[eli->noNewAttrs];
      eli->attrSizeExtTmp = new double[eli->noNewAttrs];
      for (int i = 0; i < eli->noNewAttrs; i++)
      {
        eli->attrSizeTmp[i] = 0.0;
        eli->attrSizeExtTmp[i] = 0.0;
      }

      local.setAddr(eli);

      qp->Open(args[0].addr);

      return 0;
    }
    case REQUEST :
    {
      if(!eli){
        return CANCEL;
      }
      qp->Request(args[0].addr, elem1);
      if (qp->Received(args[0].addr))
      {
        eli->read++;

        Tuple *currTuple     = (Tuple*) elem1.addr;
        Tuple *resultTuple   = new Tuple( eli->resultTupleType );

        // copy attrs from projection list

        for(int i = 0; i < eli->noOldAttrs; i++)
        {
          son = qp->GetSupplier(args[4].addr, i);
          qp->Request(son, elem2);
          index = ((CcInt*)elem2.addr)->GetIntval();
          resultTuple->CopyAttribute(index-1, currTuple, i);
        }

        // evaluate and add attrs from extension list
        supplier = args[2].addr;           // get list of ext-functions
        for(int i = 0; i < eli->noNewAttrs; i++)
        {
          supplier2 = qp->GetSupplier(supplier, i); // get an ext-function
          supplier3 = qp->GetSupplier(supplier2, 1);
          extFunArgs = qp->Argument(supplier3);
          ((*extFunArgs)[0]).setAddr(currTuple);     // pass argument
          qp->Request(supplier3,value);              // call extattr mapping

          resultTuple->PutAttribute( eli->noOldAttrs + i,
                ((Attribute*)value.addr)->Clone() );

          if (eli->read <= eli->stableValue)
          {
            eli->attrSizeTmp[i] += resultTuple->GetSize( eli->noOldAttrs + i );
            eli->attrSizeExtTmp[i] +=
              resultTuple->GetExtSize( eli->noOldAttrs + i );
          }

        }
        currTuple->DeleteIfAllowed();
        result.setAddr(resultTuple);
        return YIELD;
      } else {
        return CANCEL;
      }
    }
    case CLOSE :
    {
      qp->Close(args[0].addr);
      return 0;
    }

    case CLOSEPROGRESS:
    {
      if ( eli ) {
         delete eli;
         local.setAddr(0);
      }
      return 0;
    }

    case REQUESTPROGRESS:

      ProgressInfo p1;
      ProgressInfo *pRes;
      const double uProjectExtend = 0.0012;    //millisecs per tuple
      const double vProjectExtend = 0.00085;   //millisecs per tuple
                                               //and attribute

      pRes = (ProgressInfo*) result.addr;

      if ( !eli ) {
        return CANCEL;
      }

      if ( qp->RequestProgress(args[0].addr, &p1) )
      {
        eli->sizesChanged = false;

        if ( !eli->sizesInitialized )
        {
          eli->noAttrs = eli->noOldAttrs + eli->noNewAttrs;
          eli->attrSize = new double[eli->noAttrs];
          eli->attrSizeExt = new double[eli->noAttrs];
        }

        if ( !eli->sizesInitialized || p1.sizesChanged ||
           ( eli->read >= eli->stableValue && !eli->sizesFinal ) )
        {
      eli->Size = 0.0;
      eli->SizeExt = 0.0;

          for( int i = 0; i < eli->noOldAttrs; i++)    //old attrs
          {
            son = qp->GetSupplier(args[4].addr, i);
            qp->Request(son, elem2);
            index = ((CcInt*)elem2.addr)->GetIntval();
            eli->attrSize[i] = p1.attrSize[index-1];
            eli->attrSizeExt[i] = p1.attrSizeExt[index-1];
            eli->Size += eli->attrSize[i];
            eli->SizeExt += eli->attrSizeExt[i];
          }

          if ( eli->read < eli->stableValue )        //new attrs
          {
            for (int j = 0; j < eli->noNewAttrs; j++)
            {
              eli->attrSize[j + eli->noOldAttrs] = 12;   //size yet unknown
              eli->attrSizeExt[j + eli->noOldAttrs] = 12;
              eli->Size += eli->attrSize[j + eli->noOldAttrs];
              eli->SizeExt += eli->attrSizeExt[j + eli->noOldAttrs];
            }
          }
          else
          {
            for (int j = 0; j < eli->noNewAttrs; j++)
            {
              eli->attrSize[j + eli->noOldAttrs] = eli->attrSizeTmp[j] /
                eli->stableValue;
              eli->attrSizeExt[j + eli->noOldAttrs] = eli->attrSizeExtTmp[j] /
                eli->stableValue;
              eli->Size += eli->attrSize[j + eli->noOldAttrs];
              eli->SizeExt += eli->attrSizeExt[j + eli->noOldAttrs];
            }
          }
          eli->sizesInitialized = true;
          eli->sizesChanged = true;;
        }
        if ( eli->read >= eli->stableValue ) eli->sizesFinal = true;

        pRes->Card = p1.Card;

        pRes->CopySizes(eli);

        pRes->Time = p1.Time +
          p1.Card * (uProjectExtend + eli->noAttrs * vProjectExtend);


        if ( p1.BTime < 0.1 && pipelinedProgress )      //non-blocking,
                                                        //use pipelining
          pRes->Progress = p1.Progress;
        else
          pRes->Progress =
            (p1.Progress * p1.Time +
              eli->read * (uProjectExtend + eli->noAttrs * vProjectExtend))
            / pRes->Time;

        pRes->CopyBlocking(p1);    //non-blocking operator

        return YIELD;
      } else {
        return CANCEL;
      }
  }
  return 0;
}

#endif

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

 if(nl->ListLength(args)!=3){
   return listutils::typeError("three arguments expected");
 }

 ListExpr stream = nl->First(args);
 ListExpr attrList = nl->Second(args);
 ListExpr namedMapL = nl->Third(args);
 string err ="stream(tuple(K) x (a1..an) x "
             "(b (tuple(K) -> stream(L))) expected";

 if(!listutils::isTupleStream(stream) ||
    nl->IsAtom(attrList) ||
    nl->ListLength(namedMapL)!=1){
   return listutils::typeError(err);
 }

 ListExpr namedMap = nl->First(namedMapL);
 if(nl->ListLength(namedMap)!=2){
   return listutils::typeError(err +
                        "(third argument must be a pair of name , map)");
 }

 // process the projection list
 ListExpr streamattr = nl->Second(nl->Second(stream));
 set<string> names;
 int noAttrs;
 ListExpr numberList;

 ListExpr newAttrList;
 ListExpr lastNewAttrList;
 ListExpr lastNumberList;
 bool firstCall = true;

 int attrno = nl->ListLength(attrList);
 if(nl->IsEmpty(attrList)){
   noAttrs = 0;
   numberList = nl->TheEmptyList();
   newAttrList = nl->TheEmptyList();
 } else {
   noAttrs = nl->ListLength(attrList);
   ListExpr attrType;
   while(!nl->IsEmpty(attrList)){
     ListExpr attr = nl->First(attrList);
     attrList = nl->Rest(attrList);
     if(nl->AtomType(attr)!=SymbolType){
       return listutils::typeError(err + "(invalid attr name found)");
     }
     string attrName = nl->SymbolValue(attr);
     if(names.find(attrName)!=names.end()){
       return listutils::typeError(err + "(attr name found twice)");
     }
     names.insert(attrName);
     int j = listutils::findAttribute(streamattr, attrName, attrType);
     if(j==0){
       return listutils::typeError(err + "(attr name "+
                                   attrName + " not found)");
     }
     if (firstCall) {
       firstCall = false;
       newAttrList = nl->OneElemList(nl->TwoElemList(attr, attrType));
       lastNewAttrList = newAttrList;
       numberList = nl->OneElemList(nl->IntAtom(j));
       lastNumberList = numberList;
     } else {
       lastNewAttrList = nl->Append(lastNewAttrList,
                                    nl->TwoElemList(attr, attrType));
       lastNumberList = nl->Append(lastNumberList, nl->IntAtom(j));
     }
   }
 }

 ListExpr tupletype = nl->Second(stream);
 ListExpr mapname = nl->First(namedMap);
 ListExpr map = nl->Second(namedMap);
 if(nl->AtomType(mapname)!=SymbolType ||
    !listutils::isMap<1>(map)){
   return listutils::typeError(err +
          "(third argument must be a pair of name and map)");
 }

 if(!nl->Equal(tupletype,nl->Second(map))){
   return listutils::typeError(err + "(map argument differs from tuple type");
 }
 if(!listutils::isDATAStream(nl->Third(map))){
   return listutils::typeError(err + " (result of mpa is not a DATA stream");
 }

 ListExpr mapData = nl->Second(nl->Third(map));
 if(firstCall){
   newAttrList = nl->OneElemList(nl->TwoElemList(mapname,mapData));
 } else {
   lastNewAttrList = nl->Append(lastNewAttrList,
                                nl->TwoElemList(mapname,mapData));
 }


 return nl->ThreeElemList( nl->SymbolAtom(Symbol::APPEND()),
                           nl->TwoElemList( nl->IntAtom( attrno ),
                                             numberList),
                           nl->TwoElemList( nl->SymbolAtom(Symbol::STREAM()),
                                            nl->TwoElemList(
                                            nl->SymbolAtom(Tuple::BasicType()),
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
        localinfo->streamY.setAddr( supplier3 );

        //4. get the attribute numbers
        int noOfAttrs = ((CcInt*)args[3].addr)->GetIntval();
        for( int i = 0; i < noOfAttrs; i++)
        {
          Supplier son = qp->GetSupplier(args[4].addr, i);
          Word elem2;
          qp->Request(son, elem2);
          localinfo->attrs.push_back( ((CcInt*)elem2.addr)->GetIntval()-1 );
        }

        local.setAddr(localinfo);
      }
      else
      {
        local.setAddr(0);
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
      wValueY.setAddr(0);
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
            localinfo->streamY.setAddr(supplier3);

            wValueY.setAddr(0);
          }
          else  //streamx is exausted
          {
            localinfo->streamY.setAddr(0);
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

      tupleXY->PutAttribute( i, (Attribute*)wValueY.addr );

      // setting the result
      result.setAddr( tupleXY );
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
        local.setAddr(0);
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
  if(nl->ListLength(args)!=2){
    return listutils::typeError("two streams expected");
  }
  if(!nl->Equal(nl->First(args),nl->Second(args))){
      return listutils::typeError("both arguments must be of the same type");
  }
  if(!listutils::isStream(nl->First(args))){
    return listutils::typeError("arguments are no streams");
  }
  return nl->First(args);
}
/*
2.20.2 Value mapping function of operator ~concat~

*/
int Concat(Word* args, Word& result, int message, Word& local, Supplier s)
{
  Word t;

  switch (message)
  {
    case OPEN :

      qp->Open(args[0].addr);
      qp->Open(args[1].addr);
      // start using 1st stream
      if(local.addr){
        *(static_cast<bool*>(local.addr)) = false;
      } else {
        local.setAddr(new bool(false));
      }
      return 0;

    case REQUEST :
      if(!local.addr) {
        result.setAddr(0);
        return CANCEL;
      }
      if ( !(*static_cast<bool*>(local.addr)) ){
        qp->Request(args[0].addr, t);
        if (qp->Received(args[0].addr)){
          result.setAddr(t.addr);
          return YIELD;
        } else {
          *(static_cast<bool*>(local.addr)) = true; // start using 2nd stream
        }
      }
      qp->Request(args[1].addr, t);
      if (qp->Received(args[1].addr)){
        result.setAddr(t.addr);
        return YIELD;
      } else {
        return CANCEL;
      }

    case CLOSE :

      qp->Close(args[0].addr);
      qp->Close(args[1].addr);
      if( local.addr ) {
        delete (static_cast<bool*>(local.addr));
        local.setAddr(0);
      }
      return 0;
  }
  return 0;
}

/*
2.20.3 Specification of operator ~concat~

*/
const string ConcatSpec  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                           "\"Example\" ) "
                           "(<text>stream(X) x stream(X) -> stream(X)</text--->"
                           "<text>_ _ concat</text--->"
                           "<text>Returns all elements of the first argument "
                           "followed by all elements from the second argument."
                           "</text--->"
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

       ( (stream X) (xi1 ... xik) ( (y1 (map R T1)) ... (ym (map R Tm)) ) )

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

  string relSymbolStr = Relation::BasicType();
  string tupleSymbolStr = Tuple::BasicType();

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
// Original implementation:
//     listOk = listOk && !nl->IsAtom(second) &&
//              ( nl->ListLength(second) > 0 );
    // Also allow for an empty grouping attr-list:
    listOk = listOk && !nl->IsAtom(second);

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
    return nl->SymbolAtom(Symbol::TYPEERROR());
  }


  // check for tuple stream
  listOk = listOk &&
           (nl->ListLength(first) == 2) &&
           (TypeOfRelAlgSymbol(nl->First(first) == stream)) &&
           (nl->ListLength(nl->Second(first)) == 2 ) &&
           (nl->IsEqual(nl->First(nl->Second(first)),tupleSymbolStr)) &&
           (IsTupleDescription(nl->Second(nl->Second(first))));

  if ( !listOk ) {

    ErrorReporter::ReportError( "groupby: Input is not of type (stream "
        + tupleSymbolStr + "(...))." );
    return nl->SymbolAtom(Symbol::TYPEERROR());
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
    if(nl->AtomType(first2)!=SymbolType){
      ErrorReporter::ReportError("Wrong format for an attribute name");
      return nl->TypeError();
    }

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
      return nl->SymbolAtom(Symbol::TYPEERROR());
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
      return nl->SymbolAtom(Symbol::TYPEERROR());
    }

    // check if the Type Constructor belongs to KIND DATA
    // If the functions result type is typeerror this check will also fail
    ListExpr errorInfo = nl->OneElemList(nl->SymbolAtom("ErrorInfo"));
    if ( !am->CheckKind(Kind::DATA(), mapOut, errorInfo) ) {

      stringstream errMsg;
      errMsg << "groupby: The aggregate function for attribute \""
          << nl->SymbolValue(newAttr) << "\""
          << " returns a type which is not usable in tuples."
          << " The type constructor \""
          <<  nl->ToString(mapOut) << "\""
          << " belongs not to kind DATA!"
          << ends;

      ErrorReporter::ReportError(errMsg.str());
      return nl->SymbolAtom(Symbol::TYPEERROR());

    }

    if (    (nl->EndOfList( lastlistn ) == true)
         && (nl->IsEmpty( lastlistn ) == false)
         && (nl->IsAtom( lastlistn ) == false)
       )
    { // list already contains group-attributes (not empty)
      lastlistn = nl->Append(lastlistn,(nl->TwoElemList(newAttr,mapOut)));
    }
    else
    { // no group attribute (list is still empty)
      listn = nl->OneElemList(nl->TwoElemList(newAttr,mapOut));
      lastlistn = listn;
    }
  } // end of while check functions

  if ( !CompareNames(listn) )
  { // check if attribute names are unique
    ErrorReporter::ReportError("groupby: Attribute names are not unique");
    return nl->SymbolAtom(Symbol::TYPEERROR());
  }

  // Type mapping is correct, return result type.

  ListExpr result =
    nl->ThreeElemList(
      nl->SymbolAtom(Symbol::APPEND()),
      nl->Cons(nl->IntAtom(nl->ListLength(listp)), listp),
      nl->TwoElemList(
        nl->SymbolAtom(Symbol::STREAM()),
        nl->TwoElemList(
          nl->SymbolAtom(tupleSymbolStr),
          listn
        )
      )
    );

//  string resstring;
//  nl->WriteToString(resstring, result);
//  cout << "groupbyTypeMap: result = " << resstring << endl;
  return result;
}

ListExpr GroupByTypeMap(ListExpr args)
{
  return GroupByTypeMap2(args);
}

/*
2.22 Operator ~slidingwindow~

2.21.1 Type mapping function of operator ~slidingwindow~

Result type of ~slidingwindow~ operation.

----   Let X = tuple ((x1 t1) ... (xn tn)), R = rel(X):

       ( (stream X) (xi1 ... xik) ( (y1 (map R T1)) ... (ym (map R Tm)) ) )

        -> ( APPEND (m p1 ... pm)
               (stream (tuple (xj1 tj1)... (xjl tjl) (y1 T1) ... (ym Tm))))

       with tj,Ti in kind DATA, xi <> xj and k+l=n, pi <> pj and 1 <= pi <= m.
       This means attributes xi ... xik are removed from the stream and
       attributes y1 ... ym are appended. These new attributes represent
       aggregated values computed by maps of R -> Ti which must have a
       result type of kind DATA.
----

*/
ListExpr SlidingWindowTypeMap(ListExpr args)
{
  bool debugme= true;
  ListExpr first, second, third, fourth;     // list used for analysing input
  ListExpr listn, lastlistn, listp;  // list used for constructing output

  first = second = third = nl->TheEmptyList();
  listn = lastlistn = listp = nl->TheEmptyList();

  string tupleSymbolStr = Tuple::BasicType();

  bool listOk = true;
  listOk = listOk && ( nl->ListLength(args) == 4 );

  if ( listOk ) {
    first  = nl->First(args);
    second = nl->Second(args);
    third  = nl->Third(args);
    fourth= nl->Fourth(args);

    // check input list structure
    listOk = listOk && (nl->ListLength(first) == 2);
    listOk = listOk && !nl->IsEmpty( fourth );
  }

  if( !listOk )
  {
    stringstream errMsg;
    errMsg << "slidingwindow: Invalid input list structure. "
        << "The structure should be a four elem list "
        << "like (stream (" << tupleSymbolStr
        << "((x1 t1) ... (xn tn)) int int "
        << "( (y1 (map R T1)) ... (ym (map R Tm))!";

    ErrorReporter::ReportError(errMsg.str());
    return nl->SymbolAtom(Symbol::TYPEERROR());
  }


  // check for tuple stream
  listOk = listOk &&
           (nl->ListLength(first) == 2) &&
           (TypeOfRelAlgSymbol(nl->First(first) == stream)) &&
           (nl->ListLength(nl->Second(first)) == 2 ) &&
           (nl->IsEqual(nl->First(nl->Second(first)),tupleSymbolStr)) &&
           (IsTupleDescription(nl->Second(nl->Second(first))));

  if ( !listOk ) {

    ErrorReporter::ReportError( "slidingwindow: Input is not of type (stream "
        + tupleSymbolStr + "(...))." );
    return nl->SymbolAtom(Symbol::TYPEERROR());
  }

  // list seems to be ok. Check the second and the third arguments
  if(! nl->IsEqual(second, CcInt::BasicType()) )
  {
    ErrorReporter::ReportError(
        "slidingwindow: expected int as a second argument");
    return nl->TypeError();
  }

  if(! nl->IsEqual(third, CcInt::BasicType()) )
  {
    ErrorReporter::ReportError(
        "slidingwindow: expected int as a third argument");
    return nl->TypeError();
  }

  // Check the last argument
  ListExpr rest = fourth;

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
//    listOk = listOk && ( nl->Equal(groupType, nl->Second(mapDef)) );

    if( !listOk )
        // Todo: there could be more fine grained error messages
    {
      ErrorReporter::ReportError(
          "slidingwindow: Function definition is not correct!");
      return nl->SymbolAtom(Symbol::TYPEERROR());
    }

    // check if the Type Constructor belongs to KIND DATA
    // If the functions result type is typeerror this check will also fail
    ListExpr errorInfo = nl->OneElemList(nl->SymbolAtom("ErrorInfo"));
    if ( !am->CheckKind(Kind::DATA(), mapOut, errorInfo) ) {

      stringstream errMsg;
      errMsg << "slidingwindow: The aggregate function for attribute \""
          << nl->SymbolValue(newAttr) << "\""
          << " returns a type which is not usable in tuples."
          << " The type constructor \""
          <<  nl->ToString(mapOut) << "\""
          << " belongs not to kind DATA!"
          << ends;

      ErrorReporter::ReportError(errMsg.str());
      return nl->SymbolAtom(Symbol::TYPEERROR());

    }

    if (    (nl->EndOfList( lastlistn ) == true)
         && (nl->IsEmpty( lastlistn ) == false)
         && (nl->IsAtom( lastlistn ) == false)
       )
    { // list already contains group-attributes (not empty)
      lastlistn = nl->Append(lastlistn,(nl->TwoElemList(newAttr,mapOut)));
    }
    else
    { // no group attribute (list is still empty)
      listn = nl->OneElemList(nl->TwoElemList(newAttr,mapOut));
      lastlistn = listn;
    }
  } // end of while check functions

  if ( !CompareNames(listn) )
  { // check if attribute names are unique
    ErrorReporter::ReportError("slidingwindow: Attribute names are not unique");
    return nl->SymbolAtom(Symbol::TYPEERROR());
  }

  // Type mapping is correct, return result type.
  ListExpr groupType = nl->TwoElemList( nl->SymbolAtom(Symbol::STREAM()),
      nl->TwoElemList(nl->SymbolAtom(Tuple::BasicType()), listn) );

  if(debugme)
  {
    string resstring;
    nl->WriteToString(resstring, groupType);
    cerr << "groupbyTypeMap: result = " << resstring << endl;
  }
  return groupType;
}


/*
2.21.2 Value mapping function of operator ~groupby~

*/


#ifndef USE_PROGRESS


struct GroupByLocalInfo
{
  Tuple *t;
  TupleType *resultTupleType;
  long MAX_MEMORY;
  GroupByLocalInfo() : t(0), resultTupleType(0), MAX_MEMORY(0) {}
};

int GroupByValueMapping
(Word* args, Word& result, int message, Word& local, Supplier supplier)
{
  Tuple *s = 0;
  Word sWord(Address(0));
  TupleBuffer* tp = 0;
  GenericRelationIterator* relIter = 0;
  int i = 0, j = 0, k = 0;
  int numberatt = 0;
  bool ifequal = false;
  Word value(Address(0));
  Supplier  value2;
  Supplier supplier1;
  Supplier supplier2;
  int noOffun = 0;
  ArgVectorPointer vector;
  const int indexOfCountArgument = 3;
  const int startIndexOfExtraArguments = indexOfCountArgument +1;
  int attribIdx = 0;
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
        gbli = new GroupByLocalInfo();
        gbli->t = (Tuple*)sWord.addr;
        gbli->t->IncReference();
        ListExpr resultType = GetTupleResultType( supplier );
        gbli->resultTupleType = new TupleType( nl->Second( resultType ) );
        gbli->MAX_MEMORY = qp->MemoryAvailableForOperator();
        local.setAddr(gbli);

        cmsg.info("ERA:ShowMemInfo")
          << "GroupBy.MAX_MEMORY ("
          << gbli->MAX_MEMORY / 1024 << " MB): = "
          << gbli->MAX_MEMORY / gbli->t->GetExtSize()
          << " Tuples" << endl;
        cmsg.send();
      }
      else
      {
        local.setAddr(0);
      }
      return 0;
    }
    case REQUEST:
    {
      Counter::getRef("GroupBy:Request")++;
      if(local.addr == 0)
      {
        return CANCEL;
      }
      gbli = (GroupByLocalInfo *)local.addr;
      if( gbli->t == 0 ) { // Stream ends
          return CANCEL;
      }
      tp = new TupleBuffer(gbli->MAX_MEMORY);
      tp->AppendTuple(gbli->t);


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
        ((*vector)[0]).setAddr(tp);

        // compute value of function i and put it into the result tuple
        qp->Request(supplier2, value);
        t->PutAttribute(numberatt + i, (Attribute*)value.addr);
        qp->ReInitResultStorage(supplier2);
      }
      result.setAddr(t);
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
        local.setAddr(0);
      }
      qp->Close(args[0].addr);
      return 0;
    }
  }
  return 0;
}

#else

//progress version


class GroupByLocalInfo: public ProgressLocalInfo
{
public:
  Tuple *t;
  TupleType *resultTupleType;
  long MAX_MEMORY;

  //new for progress

  int stableValue;
  bool sizesFinal;

  int noGroupAttrs;
  int noAggrAttrs;
  double *attrSizeTmp;
  double *attrSizeExtTmp;

  // initialization
  GroupByLocalInfo() : ProgressLocalInfo(),
    t(0), resultTupleType(0), MAX_MEMORY(0),
    stableValue(0),sizesFinal(false),
    noGroupAttrs(0), noAggrAttrs(0),
    attrSizeTmp(0), attrSizeExtTmp(0)
  {}

  ~GroupByLocalInfo(){
     if(t){
       t->DeleteIfAllowed();
       t = 0;
     }
     if(resultTupleType){
       resultTupleType->DeleteIfAllowed();
       resultTupleType = 0;
     }
     if(attrSizeTmp){
       delete[] attrSizeTmp;
       attrSizeTmp=0;
     }
     if(attrSizeExtTmp){
       delete[] attrSizeExtTmp;
       attrSizeExtTmp = 0;
     }
  }

};

int GroupByValueMapping
(Word* args, Word& result, int message, Word& local, Supplier supplier)
{
  Tuple *s = 0;
  Word sWord(Address(0));
  TupleBuffer* tp = 0;
  GenericRelationIterator* relIter = 0;
  int i = 0, j = 0, k = 0;
  int numberatt = 0;
  bool ifequal = false;
  Word value(Address(0));
  Supplier  value2;
  Supplier supplier1;
  Supplier supplier2;
  int noOffun = 0;
  ArgVectorPointer vector;
  const int indexOfCountArgument = 3;
  const int startIndexOfExtraArguments = indexOfCountArgument +1;
  int attribIdx = 0;
  GroupByLocalInfo *gbli = (GroupByLocalInfo *)local.addr;


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

      if (gbli) {
        delete gbli;
      }
      gbli = new GroupByLocalInfo();
      gbli->stableValue = 5;
      gbli->sizesFinal = false;

      numberatt = ((CcInt*)args[indexOfCountArgument].addr)->GetIntval();
      value2 = (Supplier)args[2].addr; // list of functions
      noOffun  =  qp->GetNoSons(value2);

      gbli->noAttrs = numberatt + noOffun;
      gbli->noGroupAttrs = numberatt;
      gbli->noAggrAttrs = noOffun;
      gbli->attrSizeTmp = new double[gbli->noAttrs];
      gbli->attrSizeExtTmp = new double[gbli->noAttrs];

      for (int i = 0; i < gbli->noAttrs; i++)
      {
        gbli->attrSizeTmp[i] = 0.0;
        gbli->attrSizeExtTmp[i] = 0.0;
      }

      local.setAddr(gbli);  //from now, progress queries possible

      // Get the first tuple pointer and store it in the
      // GroupBylocalInfo structure
      qp->Open (args[0].addr);
      qp->Request(args[0].addr, sWord);
      if (qp->Received(args[0].addr))
      {
        gbli->read++;
        gbli->t = (Tuple*)sWord.addr;
        //gbli->t->IncReference();
        ListExpr resultType = GetTupleResultType( supplier );
        gbli->resultTupleType = new TupleType( nl->Second( resultType ) );
        gbli->MAX_MEMORY = qp->MemoryAvailableForOperator();


        cmsg.info("ERA:ShowMemInfo")
          << "GroupBy.MAX_MEMORY ("
          << gbli->MAX_MEMORY / 1024 << " MB): = "
          << gbli->MAX_MEMORY / gbli->t->GetExtSize()
          << " Tuples" << endl;
        cmsg.send();
      }

      return 0;
    }
    case REQUEST:
    {
      Counter::getRef("GroupBy:Request")++;
      if(!gbli){
        return CANCEL;
      }

      if(gbli->t == 0){
        return CANCEL;
      } else {
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
        gbli->read++;
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
          gbli->t->DeleteIfAllowed();
          gbli->t = s;
          //gbli->t->IncReference();
        }
      }
      if (ifequal)
      // last group finished, stream ends
      {
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
        ((*vector)[0]) = SetWord(tp);

        // compute value of function i and put it into the result tuple
        qp->Request(supplier2, value);
        t->PutAttribute(numberatt + i, (Attribute*)value.addr);
        qp->ReInitResultStorage(supplier2);
      }

      if ( gbli->returned < gbli->stableValue )
      {
        for (int i = 0; i < gbli->noAttrs; i++)
        {
          gbli->attrSizeTmp[i] += t->GetSize(i);
          gbli->attrSizeExtTmp[i] += t->GetExtSize(i);
        }
      }

      gbli->returned++;
      result.setAddr(t);
      if(tp){
         delete tp;
      }
      tp = 0;
      return YIELD;
    }
    case CLOSE:
    {
      qp->Close(args[0].addr);
      return 0;
    }

    case CLOSEPROGRESS:
      if (gbli)
      {
        delete gbli;
        local.setAddr(0);
      }
      return 0;


    case REQUESTPROGRESS:

      ProgressInfo p1;
      ProgressInfo *pRes;
      const double uGroup = 0.013;  //millisecs per tuple and new attribute

      pRes = (ProgressInfo*) result.addr;

      if ( !gbli ){
         return CANCEL;
      }

      if ( qp->RequestProgress(args[0].addr, &p1) )
      {
        gbli->sizesChanged = false;

        if ( !gbli->sizesInitialized )
        {
          gbli->attrSize = new double[gbli->noAttrs];
          gbli->attrSizeExt = new double[gbli->noAttrs];
        }

        if ( !gbli->sizesInitialized || p1.sizesChanged ||
           ( gbli->returned >= gbli->stableValue && !gbli->sizesFinal ) )
        {

          if ( gbli->returned < gbli->stableValue )
          {
            for (int i = 0; i < gbli->noGroupAttrs; i++)
            {
              gbli->attrSize[i] = 56;  //guessing string attributes
              gbli->attrSizeExt[i] = 56;
            }
            for (int j = 0; j < gbli->noAggrAttrs; j++)
            {        //guessing int attributes
              gbli->attrSize[j + gbli->noGroupAttrs] = 12;
              gbli->attrSizeExt[j + gbli->noGroupAttrs] = 12;
            }
          }
          else
          {
            for (int i = 0; i < gbli->noAttrs; i++)
            {
              gbli->attrSize[i] = gbli->attrSizeTmp[i] / gbli->stableValue;
              gbli->attrSizeExt[i] = gbli->attrSizeExtTmp[i] /
                gbli->stableValue;
            }
          }
          gbli->Size = 0.0;
          gbli->SizeExt = 0.0;
          for (int i = 0; i < gbli->noAttrs; i++)
          {
            gbli->Size += gbli->attrSize[i];
            gbli->SizeExt += gbli->attrSizeExt[i];
          }
          gbli->sizesInitialized = true;
          gbli->sizesChanged = true;
        }
        if ( gbli->returned >= gbli->stableValue ) gbli->sizesFinal = true;



        //As long as we have not seen 5 result tuples (groups), we guess groups
        //of size 10

        pRes->Card = (gbli->returned < 5 ? p1.Card/10.0 :
          p1.Card * (double) gbli->returned / (double) gbli->read);

        pRes->CopySizes(gbli);

        pRes->Time = p1.Time + p1.Card * gbli->noAggrAttrs * uGroup;

        pRes->Progress = (p1.Progress * p1.Time
          + gbli->read * gbli->noAggrAttrs * uGroup)
          / pRes->Time;

        pRes->CopyBlocking(p1);    //non-blocking operator

        return YIELD;
      } else {
        return CANCEL;
      }

  }
  return 0;
}

#endif



struct SlidingWindowLocalInfo
{
  TupleType *resultTupleType;
  bool firstWindow;
  CircularTupleBuffer* tb;

  SlidingWindowLocalInfo() : resultTupleType(0), firstWindow(true), tb(0) {}
};

int SlidingWindowValueMapping
(Word* args, Word& result, int message, Word& local, Supplier supplier)
{
  bool debugme= false;
  if(debugme)
    qp->ListOfTree(supplier, cerr);
  Tuple *s = 0;
  Word sWord(Address(0));
  Word value(Address(0));
  Supplier  value2;
  Supplier supplier1;
  Supplier supplier2;
  int noOffun = 0;
  ArgVectorPointer vector;
  SlidingWindowLocalInfo *gbli = 0;

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
      gbli = new SlidingWindowLocalInfo();
      ListExpr resultType = GetTupleResultType( supplier );
      gbli->resultTupleType = new TupleType( nl->Second( resultType ) );
      gbli->firstWindow= true;
      local.setAddr(gbli);
      qp->Open (args[0].addr);

      return 0;
    }
    case REQUEST:
    {
      int windowSize, stepSize;
      windowSize = static_cast<CcInt*>(args[1].addr)->GetIntval();
      stepSize = static_cast<CcInt*>(args[2].addr)->GetIntval();

      if(local.addr == 0)
      {
        return CANCEL;
      }
      gbli = (SlidingWindowLocalInfo *)local.addr;

      int i= 0;
      if(gbli->firstWindow)
      {
        gbli->firstWindow= false;
        gbli->tb= new CircularTupleBuffer(
            true, gbli->resultTupleType, windowSize);
        qp->Request(args[0].addr, sWord);
        while (i++ < windowSize && qp->Received(args[0].addr))
        {
          s= static_cast<Tuple*>(sWord.addr);
          gbli->tb->AppendTuple(s);
          qp->Request(args[0].addr, sWord);
          s->DeleteIfAllowed();
        }
      }
      else
      {
        qp->Request(args[0].addr, sWord);
        while (i++ < stepSize && qp->Received(args[0].addr))
        {
          s= static_cast<Tuple*>(sWord.addr);
          gbli->tb->AppendTuple(s);
          qp->Request(args[0].addr, sWord);
          s->DeleteIfAllowed();
        }
      }
      if(i == 1) //Steam end
        return CANCEL;

      // create result tuple
      Tuple *t = new Tuple( gbli->resultTupleType );
      value2 = (Supplier)args[3].addr; // list of functions
      noOffun  =  qp->GetNoSons(value2);

      for(i = 0; i < noOffun; i++)
      {
        // prepare arguments for function i
        supplier1 = qp->GetSupplier(value2, i);
        supplier2 = qp->GetSupplier(supplier1, 1);
        vector = qp->Argument(supplier2);
        // The group was stored in a relation identified by symbol group
        // which is a typemap operator. Here it is stored in the
        // argument vector
        ((*vector)[0]).setAddr(gbli->tb);

        // compute value of function i and put it into the result tuple
        qp->Request(supplier2, value);
        t->PutAttribute(i, (Attribute*)value.addr);
        qp->ReInitResultStorage(supplier2);
      }
      result.setAddr(t);
      return YIELD;
    }
    case CLOSE:
    {
      if( local.addr != 0 )
      {
        gbli = (SlidingWindowLocalInfo *)local.addr;
        if( gbli->resultTupleType != 0 )
          gbli->resultTupleType->DeleteIfAllowed();
        if( gbli->tb != 0 )
        {
          gbli->tb->Clear();
          delete gbli->tb;
        }
        delete gbli;
        local.setAddr(0);
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
                            "appended to the grouping attributes. The empty "
                            "list is allowed for the grouping attributes (this "
                            "results in a single group with all input tuples)."
                            "</text--->"
                            "<text>query Employee feed sortby[DeptNr asc] "
                            "groupby[DeptNr; anz : group feed count] consume"
                            "</text--->"
                            ") )";

/*
2.21.6 Specification of operator ~slidingwindow~

*/
const string SlidingWindowSpec  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                            "\"Example\" ) "
                            "( <text>((stream (tuple (a1:d1 ... an:dn))) "
                            "int int ((bj1 (fun (rel (tuple (a1:d1"
                            " ... an:dn))) (_))) ... (bjl (fun (rel (tuple"
                            " (a1:d1 ... an:dn))) (_))))) -> (stream (tuple"
                            " (bj1 ... bjl)))</text--->"
                            "<text>_ slidingwindow [int, int; funlist]"
                            "</text--->"
                            "<text>Apply a sliding window on a stream of tuples"
                            " grouping together the tuples within the window."
                            " The window size and the step size are arguments."
                            " The groups are fed to the functions. The result"
                            " tuples are constructed from the results of those"
                            " functions. If the stream has tuples less than the"
                            " window size, these tuples are considered one "
                            " group.</text--->"
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
2.21.5 Definition of operator ~slidingwindow~

*/
Operator extrelslidingwindow (
         "slidingwindow",             // name
         SlidingWindowSpec,           // specification
         SlidingWindowValueMapping,   // value mapping
         Operator::SimpleSelect,          // trivial selection function
         SlidingWindowTypeMap         // type mapping
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
  if(nl->ListLength(args)!=4){
    return listutils::typeError("4 arguments expected");
  }

  ListExpr stream = nl->First(args);
  ListExpr attrNameList = nl->Second(args);
  ListExpr map = nl->Third(args);
  ListExpr defaultValue = nl->Fourth(args);

  if(!listutils::isTupleStream(stream)){
    return listutils::typeError("first argument must be a tuple stream");
  }
  if(nl->AtomType(attrNameList)!=SymbolType){
    return listutils::typeError("second argument is not a valid attr name");
  }
  if(!listutils::isMap<2>(map)){
    return listutils::typeError("third argument is not a map");
  }

  ListExpr attrType;
  string attrName = nl->SymbolValue(attrNameList);
  int j = listutils::findAttribute(nl->Second(nl->Second(stream)),
                                   attrName, attrType);
  if(j==0){
    return listutils::typeError("attrname " + attrName +
                                " not known in the tuple");
  }
  if(!nl->Equal(defaultValue,attrType)){
    return listutils::typeError("default value has another"
                                " type than the selected attribute");
  }

  if(nl->ListLength(map)!=4){
   return listutils::typeError("map has the wrong number of arguments");
  }

  ListExpr maparg1 = nl->Second(map);
  ListExpr maparg2 = nl->Third(map);
  ListExpr mapres  = nl->Fourth(map);
  string err = "stream(tuple([a1 : t1, ..., an : tn])) x "
               "ai x ( ti x ti -> ti) x ti expected";

  if(!nl->Equal(maparg1, maparg2) ||
     !nl->Equal(maparg1, mapres) ||
     !nl->Equal(attrType,maparg1)){
     return listutils::typeError(err);
  }

  return nl->ThreeElemList(
           nl->SymbolAtom( Symbol::APPEND() ),
           nl->OneElemList(nl->IntAtom(j)),
           defaultValue );
}

/*
2.18.2 Value mapping function of operator ~aggregate~

*/
#ifndef USE_PROGRESS


int Aggregate(Word* args, Word& result, int message, Word& local, Supplier s)
{
  // The argument vector contains the following values:
  // args[0] = stream of tuples
  // args[1] = attribute name
  // args[2] = mapping function
  // args[3] = zero value
  // args[4] = attribute index added by APPEND

  Word t( Address(0) );
  ArgVectorPointer vector;

  qp->Open(args[0].addr);
  int index = ((CcInt*)args[4].addr)->GetIntval();
  result = qp->ResultStorage(s);
  // Get the initial value
  Attribute* tmpres = ((Attribute*)args[3].addr)->Clone();
  qp->Request( args[0].addr, t );
  Word fctres;
  while( qp->Received( args[0].addr ) )
  {
    vector = qp->Argument(args[2].addr);
    ((*vector)[0]).setAddr(tmpres);
    ((*vector)[1]).setAddr(((Tuple*)t.addr)->GetAttribute( index-1 ) );
    qp->Request(args[2].addr, fctres);

    delete tmpres; //delete intermediate result
    tmpres = (Attribute*) fctres.addr;

    qp->ReInitResultStorage( args[2].addr );

    ((Tuple*)t.addr)->DeleteIfAllowed();
    qp->Request( args[0].addr, t );
  }

  ((Attribute*)result.addr)->
    CopyFrom( (const Attribute*)tmpres );

  delete tmpres;
  qp->Close(args[0].addr);

  return 0;
}
#else
int Aggregate(Word* args, Word& result, int message, Word& local, Supplier s)
{
  // The argument vector contains the following values:
  // args[0] = stream of tuples
  // args[1] = attribute name
  // args[2] = mapping function
  // args[3] = zero value
  // args[4] = attribute index added by APPEND

  Word t( Address(0) );
  ArgVectorPointer vector;
  ProgressLocalInfo* ali=0;
  ali = (ProgressLocalInfo*) local.addr;
  switch ( message )
  {

    case OPEN :
    case REQUEST:
    case CLOSE: {
      if(ali){
         delete ali;
      }
      ali = new ProgressLocalInfo();
      local.setAddr(ali);
      qp->Open(args[0].addr);
      int index = ((CcInt*)args[4].addr)->GetIntval();
      result = qp->ResultStorage(s);
      // Get the initial value
      Attribute* tmpres = ((Attribute*)args[3].addr)->Clone();
      qp->Request( args[0].addr, t );
      Word fctres;
      while( qp->Received( args[0].addr ) )
      {
        vector = qp->Argument(args[2].addr);
        ((*vector)[0]).setAddr(tmpres);
        ((*vector)[1]).setAddr(((Tuple*)t.addr)->GetAttribute( index-1 ) );
        qp->Request(args[2].addr, fctres);

        delete tmpres; //delete intermediate result
        tmpres = (Attribute*) fctres.addr;

        qp->ReInitResultStorage( args[2].addr );

        ((Tuple*)t.addr)->DeleteIfAllowed();
        qp->Request( args[0].addr, t );
        if (ali) ali->read++;
      }
      ((Attribute*)result.addr)->CopyFrom( (const Attribute*)tmpres );
      delete tmpres;
      qp->Close(args[0].addr);
      return 0;
    }
    case CLOSEPROGRESS:{
      if ( ali )        // if local info structure exists
      {
        delete ali;     // remove it
        local.setAddr(0); // and set adress to 0
      }
      return 0;
    }

    case REQUESTPROGRESS :
    {
      ProgressInfo p1;
      ProgressInfo *pRes;
      pRes = (ProgressInfo*) result.addr;
      double uAggregate=0.0121937626;
      if (qp->RequestProgress(args[0].addr, &p1))
      {
        pRes->Card=1;
        pRes->Time = p1.Time + p1.Card * uAggregate;
        pRes->Progress = (p1.Progress * p1.Time + ali->read * uAggregate)/
            pRes->Time;
        return YIELD;
      } else{
        return CANCEL;
      }
    }
  }
  return -1;
}

#endif
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
    ((Attribute*)result.addr)->
      CopyFrom( (const Attribute*)args[3].addr );

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
         ((*vector)[0]).setAddr(top.value);
         ((*vector)[1]).setAddr(attr);
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
     ((*vector)[0]).setAddr(top.value);
     ((*vector)[1]).setAddr(tmpResult.value);
     qp->Request(args[2].addr, resultWord);
     qp->ReInitResultStorage(args[2].addr);
     tmpResult.destroy(); // destroy temporarly result
     tmpResult.level = 1; // mark as computed
     tmpResult.value = (Attribute*) resultWord.addr;
     top.destroy();
   }
   ((Attribute*)result.addr)->
                          CopyFrom((Attribute*)tmpResult.value);
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
  "<text>_ aggregate [ AttrName; fun; InitialValue]</text--->"
  "<text>Aggregates the values from the attribute 'AttrName'"
  "in the tuplestream using function 'fun' and starting "
  "with 'InitialValue' as first left argument for 'fun'. "
  "Returns 'InitialValue', if the stream is empty.</text--->"
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
  "<text>_ aggregateB [ AttrName ; fun; ZeroValue ]</text--->"
  "<text>Aggregates the values of attribute 'AttrName' "
  "from the tuple stream using the function 'fun' in a balanced "
  "fashion. Returns 'ZeroValue', if the stream is empty.</text--->"
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

----    ((stream (tuple (x1 ... xn))) (stream (tuple (y1 ... ym))) (map tuple tuple bool)

        -> (stream (tuple (x1 ... xn y1 ... ym)))
----

*/
ListExpr SymmJoinTypeMap(ListExpr args)
{
  if(nl->ListLength(args)!=3){
    return listutils::typeError("three arguments expected");
  }
  ListExpr stream1 = nl->First(args);
  ListExpr stream2 = nl->Second(args);
  ListExpr map = nl->Third(args);

  string err = "stream(tuple1) x stream(tuple2) x "
               "( tuple1 x tuple2 -> bool) expected";
  if(!listutils::isTupleStream(stream1) ||
     !listutils::isTupleStream(stream2) ||
     !listutils::isMap<2>(map)){
    return listutils::typeError(err);
  }

  if(!nl->Equal(nl->Second(stream1), nl->Second(map)) ||
     !nl->Equal(nl->Second(stream2), nl->Third(map)) ||
     !listutils::isSymbol(nl->Fourth(map),CcBool::BasicType())){
    return listutils::typeError(err +"(wrong mapping)");
  }

  ListExpr a1List = nl->Second(nl->Second(stream1));
  ListExpr a2List = nl->Second(nl->Second(stream2));

  if(!listutils::disjointAttrNames(a1List,a2List)){
    return listutils::typeError(err + "(name conflict in tuples");
  }
  ListExpr list = ConcatLists(a1List, a2List);

  return nl->TwoElemList(nl->SymbolAtom(Symbol::STREAM()),
           nl->TwoElemList(nl->SymbolAtom(Tuple::BasicType()),
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

      local.setAddr(pli);
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
            ((*funArgs)[0]).setAddr( pli->currTuple );
            ((*funArgs)[1]).setAddr( leftTuple );
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
              result.setAddr( resultTuple );
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
            ((*funArgs)[0]).setAddr( rightTuple );
            ((*funArgs)[1]).setAddr( pli->currTuple );
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
              result.setAddr( resultTuple );
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
      if(pli)
      {
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
         local.setAddr(0);
      }

      qp->Close(args[0].addr);
      qp->Close(args[1].addr);

      return 0;
    }
  }
  return 0;
}


#else

// with support for progress queries

class SymmJoinLocalInfo: public ProgressLocalInfo
{
public:

   SymmJoinLocalInfo(): resultTupleType(0), rightRel(0),
       rightIter(0),leftRel(0),leftIter(0),right(false),
       currTuple(0),rightFinished(false),leftFinished(false) {}

   ~SymmJoinLocalInfo() {
      if(resultTupleType){
        resultTupleType->DeleteIfAllowed();
        resultTupleType =0;
      }
      if(currTuple){
        currTuple->DeleteIfAllowed();
        currTuple =0;
      }
      if(rightRel){
        delete rightRel;
        rightRel=0;
      }
      if(rightIter){
        delete rightIter;
        rightIter=0;
      }
      if(leftRel){
        delete leftRel;
        leftRel=0;
      }
      if(leftIter){
        delete leftIter;
        leftIter=0;
      }
   }

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

  pli = (SymmJoinLocalInfo*) local.addr;

  switch (message)
  {
    case OPEN :
    {

      long MAX_MEMORY = qp->MemoryAvailableForOperator();
      cmsg.info("ERA:ShowMemInfo") << "SymmJoin.MAX_MEMORY ("
                                   << MAX_MEMORY/1024 << " MB): " << endl;
      cmsg.send();


      if ( pli ){
         delete pli;
      }
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
      pli->readFirst = 0;
      pli->readSecond = 0;
      pli->returned = 0;
      local.setAddr(pli);

      qp->Open(args[0].addr);
      qp->Open(args[1].addr);

      return 0;
    }

    case REQUEST :
    {
      if(!pli){
        return CANCEL;
      }
      while( true )
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
              pli->readFirst++;
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
            ((*funArgs)[0]).setAddr( pli->currTuple );
            ((*funArgs)[1]).setAddr( leftTuple );
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
              result.setAddr( resultTuple );
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
              pli->readSecond++;
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
            ((*funArgs)[0]).setAddr( rightTuple );
            ((*funArgs)[1]).setAddr( pli->currTuple );
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
              result.setAddr( resultTuple );
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
      if(pli)
      {
        if( pli->currTuple != 0 ){
          pli->currTuple->DeleteIfAllowed();
          pli->currTuple=0;
        }

        if(pli->leftIter){
          delete pli->leftIter;
          pli->leftIter = 0;
        }
        if(pli->rightIter){
           delete pli->rightIter;
           pli->rightIter=0;
        }
        if( pli->resultTupleType != 0 ){
          pli->resultTupleType->DeleteIfAllowed();
          pli->resultTupleType=0;
        }

        if( pli->rightRel != 0 )
        {
          pli->rightRel->Clear();
          delete pli->rightRel;
          pli->rightRel=0;
        }

        if( pli->leftRel != 0 )
        {
          pli->leftRel->Clear();
          delete pli->leftRel;
          pli->leftRel=0;
        }
      }

      qp->Close(args[0].addr);
      qp->Close(args[1].addr);

      return 0;
    }


    case CLOSEPROGRESS:
      if ( pli )
      {
         delete pli;
         local.setAddr(0);
      }
      return 0;


    case REQUESTPROGRESS :
    {
      ProgressInfo p1, p2;
      ProgressInfo *pRes;
      const double uSymmJoin = 0.2;  //millisecs per tuple pair


      pRes = (ProgressInfo*) result.addr;

      if (!pli){
          return CANCEL;
      }

      if (qp->RequestProgress(args[0].addr, &p1)
       && qp->RequestProgress(args[1].addr, &p2))
      {
        pli->SetJoinSizes(p1, p2);

        pRes->CopySizes(pli);

        double predCost =
          (qp->GetPredCost(s) == 0.1 ? 0.004 : qp->GetPredCost(s));

        //the default value of 0.1 is only suitable for selections

        pRes->Time = p1.Time + p2.Time +
          p1.Card * p2.Card * predCost * uSymmJoin;

        pRes->Progress =
          (p1.Progress * p1.Time + p2.Progress * p2.Time +
          pli->readFirst * pli->readSecond *
          predCost * uSymmJoin)
          / pRes->Time;

        if (pli->returned > enoughSuccessesJoin )   // stable state assumed now
        {
          pRes->Card = p1.Card * p2.Card *
            ((double) pli->returned /
              (double) (pli->readFirst * pli->readSecond));
        }
        else
        {
          pRes->Card = p1.Card * p2.Card * qp->GetSelectivity(s);
        }

        pRes->CopyBlocking(p1, p2);  //non-blocking oprator

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
  if(nl->ListLength(args)!=3){
    return listutils::typeError("three arguments expected");
  }

  ListExpr stream1 = nl->First(args);
  ListExpr stream2 = nl->Second(args);
  ListExpr mapList = nl->Third(args);

  string err = "stream(tupleA) x stream(tupleB) x "
                "((name (tupleA x tupleB -> DATA))+) expected";

  if(!listutils::isTupleStream(stream1) ||
     !listutils::isTupleStream(stream2) ||
     nl->IsAtom(mapList)){
    return listutils::typeError(err);
  }


  ListExpr TType1 = nl->Second(stream1);
  ListExpr TType2 = nl->Second(stream2);

  ListExpr attrL1 = nl->Second(TType1);
  // copy all attributes from the first list
  ListExpr newAttrList = nl->OneElemList(nl->First(attrL1));
  ListExpr lastAttrList = newAttrList;
  attrL1 = nl->Rest(attrL1);
  while(!nl->IsEmpty(attrL1)){
    lastAttrList = nl->Append(lastAttrList,nl->First(attrL1));
    attrL1 = nl->Rest(attrL1);
  }
  // copy the attributes of the second tuple stream
  ListExpr attrL2 = nl->Second(TType2);
  while(!nl->IsEmpty(attrL2)){
   lastAttrList = nl->Append(lastAttrList, nl->First(attrL2));
   attrL2 = nl->Rest(attrL2);
  }

  // for each mapping

  while(!nl->IsEmpty(mapList)){
    ListExpr namedMap = nl->First(mapList);
    mapList = nl->Rest(mapList);

    if(nl->ListLength(namedMap)!=2){
      return listutils::typeError(err +"( invalid named map found)");
    }

    if(nl->AtomType(nl->First(namedMap))!=SymbolType){
      return listutils::typeError(err+ "(invalid attribute name found");
    }
    ListExpr map = nl->Second(namedMap);

    if(!listutils::isMap<2>(map)){
     return listutils::typeError(err+" ( found a incorrect map in list)");
    }
    if(!nl->Equal(TType1, nl->Second(map)) ||
       !nl->Equal(TType2, nl->Third(map))){
      return listutils::typeError(err +"( ivalid argument in mapping)");
    }
    lastAttrList = nl->Append(lastAttrList,
                              nl->TwoElemList(nl->First(namedMap),
                                              nl->Fourth(map)));
  }

  if(!listutils::isAttrList(newAttrList)){
    return listutils::typeError(err + " ( error in result stream: "
                               "conflicting names or non-DATA attributes)");
  }

  return nl->TwoElemList(nl->SymbolAtom(Symbol::STREAM()),
                         nl->TwoElemList(nl->SymbolAtom(Tuple::BasicType()),
                                         newAttrList));

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

      local.setAddr(pli);
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
              ((*extFunArgs)[0]).setAddr(leftTuple);     // pass first argument
              ((*extFunArgs)[1]).setAddr(pli->currTuple);// pass second argument
              qp->Request(supplier3,value);              // call extattr mapping
//  The original implementation tried to avoid copying the function result,
//  but somehow, this results in a strongly growing tuplebuffer on disk:
//               resultTuple->PutAttribute(
//                 leftTuple->GetNoAttributes()
//                 + pli->currTuple->GetNoAttributes()
//                 + i, (Attribute*)value.addr);
//               qp->ReInitResultStorage( supplier3 );
              resultTuple->PutAttribute(
                leftTuple->GetNoAttributes()
                + pli->currTuple->GetNoAttributes()
                + i, ((Attribute*)value.addr)->Clone() );
            }
            leftTuple->DeleteIfAllowed();
            leftTuple = 0;
            result.setAddr( resultTuple );
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
            Concat( pli->currTuple, rightTuple, resultTuple );
            supplier = args[2].addr;           // get list of ext-functions
            nooffun = qp->GetNoSons(supplier); // get num of ext-functions
            for(i = 0; i< nooffun; i++)
            {
              supplier2 = qp->GetSupplier(supplier, i); // get an ext-function
              noofsons = qp->GetNoSons(supplier2);
              supplier3 = qp->GetSupplier(supplier2, 1);
              extFunArgs = qp->Argument(supplier3);
              ((*extFunArgs)[0]).setAddr(pli->currTuple);// pass 1st argument
              ((*extFunArgs)[1]).setAddr(rightTuple);    // pass 2nd argument
              qp->Request(supplier3,value);              // call extattr mapping
//  The original implementation tried to avoid copying the function result,
//  but somehow, this results in a strongly growing tuplebuffer on disk:
//               resultTuple->PutAttribute(
//                 pli->currTuple->GetNoAttributes()
//                 + rightTuple->GetNoAttributes()
//                 + i, (Attribute*)value.addr);
//               // extend effective left tuple
//               qp->ReInitResultStorage( supplier3 );

              resultTuple->PutAttribute(
                pli->currTuple->GetNoAttributes()
                + rightTuple->GetNoAttributes()
                + i, ((Attribute*)value.addr)->Clone() );
              // extend effective left tuple
            }
            rightTuple->DeleteIfAllowed();
            rightTuple = 0;
            result.setAddr( resultTuple );
            return YIELD;
          }
        }
      }
    }
    case CLOSE :
    {
      pli = (SymmProductExtendLocalInfo*)local.addr;
      if(pli)
      {
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
        local.setAddr(0);
      }

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
  if(nl->ListLength(args)!=2){
    return listutils::typeError("two arguments expected");
  }
  ListExpr stream1 = nl->First(args);
  ListExpr stream2 = nl->Second(args);
  string err = "stream(tuple1) x stream(tuple2) expected";
  if(!listutils::isTupleStream(stream1) ||
     !listutils::isTupleStream(stream2)){
    return listutils::typeError(err +
                 "(one of the arguments is not a tuple stream)");
  }
  ListExpr newAttrList = ConcatLists(nl->Second(nl->Second(stream1)),
                                     nl->Second(nl->Second(stream2)));
  if(!listutils::isAttrList(newAttrList)){
    return listutils::typeError(err + "( name conflict )");
  }
  return nl->TwoElemList(nl->SymbolAtom(Symbol::STREAM()),
                         nl->TwoElemList(nl->SymbolAtom(Tuple::BasicType()),
                                         newAttrList));

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

      local.setAddr(pli);
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
            Tuple *resultTuple = new Tuple( pli->resultTupleType );
            Concat( leftTuple, pli->currTuple, resultTuple );
            leftTuple->DeleteIfAllowed();
            leftTuple = 0;
            result.setAddr( resultTuple );
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
            Tuple *resultTuple = new Tuple( pli->resultTupleType );
            Concat( pli->currTuple, rightTuple, resultTuple );
            rightTuple->DeleteIfAllowed();
            rightTuple = 0;
            result.setAddr( resultTuple );
            return YIELD;
          }
        }
      }
    }
    case CLOSE :
    {
      pli = (SymmProductLocalInfo*)local.addr;
      if(pli)
      {
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
        local.setAddr(0);
      }

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
5.10.7 Operator ~addCounter~

5.10.7.1 Type Mapping function

This operator receives a tuple stream, an attribute name
as well as an initial value. It extends each tuple by
a counter initialized with the given value and the given name.

*/
ListExpr AddCounterTypeMap(ListExpr args){

if(nl->ListLength(args)!=3){
   ErrorReporter::ReportError("three arguments required.");
   return nl->TypeError();
}
ListExpr stream = nl->First(args);
ListExpr nameL   = nl->Second(args);
ListExpr init   = nl->Third(args);

// check init
if(!nl->IsEqual(init,CcInt::BasicType())){
  ErrorReporter::ReportError("the third argument has to be of type int");
  return nl->TypeError();
}

if(nl->AtomType(nameL)!=SymbolType){
  ErrorReporter::ReportError("The second argument can't be used as an"
                             "attribute name.");
  return nl->TypeError();
}
string name = nl->SymbolValue(nameL);
 // check for usualibity

string symcheckmsg = "";
if(!SecondoSystem::GetCatalog()->IsValidIdentifier(name,symcheckmsg)){
  return listutils::typeError("Symbol unusable: "+symcheckmsg+".");
}


// check streamlist
if(nl->ListLength(stream)!=2 ||
   !nl->IsEqual(nl->First(stream),Symbol::STREAM())){
 ErrorReporter::ReportError("first argument is not a stream");
 return nl->TypeError();
}
ListExpr tuple = nl->Second(stream);
if(nl->ListLength(tuple)!=2 ||
   !nl->IsEqual(nl->First(tuple),Tuple::BasicType())){
 ErrorReporter::ReportError("first argument is not a tuple stream");
 return nl->TypeError();
}
set<string> usednames;
ListExpr attributes = nl->Second(tuple);
if(nl->AtomType(attributes)!=NoAtom){
   ErrorReporter::ReportError("invalid representation in tuple stream"
                              "(not a list)");
   return nl->TypeError();
}
ListExpr last = nl->TheEmptyList();
while(!nl->IsEmpty(attributes)){
   last = nl->First(attributes);
   if(nl->ListLength(last)!=2){
      ErrorReporter::ReportError("invalid representation in tuple stream"
                                 " wrong listlength");
      return nl->TypeError();
   }
   if(nl->AtomType(nl->First(last))!=SymbolType){
      ErrorReporter::ReportError("invalid representation in tuple stream"
                                 "(syntax of attribute name)");
      return nl->TypeError();
   }
   usednames.insert(nl->SymbolValue(nl->First(last)));
   attributes = nl->Rest(attributes);
}

if(usednames.find(name)!=usednames.end()){
  ErrorReporter::ReportError("Name" + name +" is already used.");
  return  nl->TypeError();
}

// all fine, construct the result

if(nl->IsEmpty(last)){ // stream without attributes
  return nl->TwoElemList( nl->SymbolAtom(Symbol::STREAM()),
                          nl->TwoElemList(
                              nl->SymbolAtom(Tuple::BasicType()),
                              nl->OneElemList(
                                 nl->TwoElemList(
                                     nl->SymbolAtom(name),
                                     nl->SymbolAtom(CcInt::BasicType())))));
}

// make a copy of the attributes
  attributes = nl->Second(tuple);
  ListExpr reslist = nl->OneElemList(nl->First(attributes));
  ListExpr lastlist = reslist;
  attributes = nl->Rest(attributes);
  while (!(nl->IsEmpty(attributes))) {
    lastlist = nl->Append(lastlist,nl->First(attributes));
    attributes = nl->Rest(attributes);
  }
  lastlist = nl->Append(lastlist,nl->TwoElemList(
                        nl->SymbolAtom(name),
                        nl->SymbolAtom(CcInt::BasicType())));

  return nl->TwoElemList( nl->SymbolAtom(Symbol::STREAM()),
                nl->TwoElemList( nl->SymbolAtom(Tuple::BasicType()),
                                 reslist));

}

/*
5.10.7.2 Value mapping of the ~addcounter~ operator

*/
class AddCounterLocalInfo{
public:
  AddCounterLocalInfo(CcInt* init, Supplier s){
     defined = init->IsDefined();
     value = init->GetIntval();
     tupleType = new TupleType(nl->Second(GetTupleResultType(s)));
  }
  ~AddCounterLocalInfo(){
     tupleType->DeleteIfAllowed();
     tupleType = 0;
  }
  Tuple* createTuple(Tuple* orig){
    if(!defined){
       return 0;
    }
    Tuple* result = new Tuple(tupleType);
    int size = orig->GetNoAttributes();
    for(int i=0;i<size;i++){
       result->CopyAttribute(i,orig,i);
    }
    result->PutAttribute(size, new CcInt(true,value));
    value++;
    return result;
  }


private:
  bool  defined;
  int value;
  TupleType* tupleType;
};


int AddCounterValueMap(Word* args, Word& result, int message,
               Word& local, Supplier s){

   Word orig;
   switch(message){
    case OPEN: {
       CcInt* Init = ((CcInt*)args[2].addr);
       qp->Open(args[0].addr);
       local.addr = new AddCounterLocalInfo(Init,s);
       break;
    }
    case REQUEST: {
       qp->Request(args[0].addr,orig);
       if(!qp->Received(args[0].addr)){
          return CANCEL;
       }  else {
          Tuple* tmp = ((AddCounterLocalInfo*)local.addr)->
                          createTuple((Tuple*)orig.addr);
          ((Tuple*)orig.addr)->DeleteIfAllowed();
          result.setAddr(tmp);
          return YIELD;
       }

    }
    case CLOSE: {
       qp->Close(args[0].addr);
       if(local.addr)
       {
         AddCounterLocalInfo* acli = (AddCounterLocalInfo*)local.addr;
         delete acli;
         local.addr=0;
       }
       return 0;
    }
    default: assert(false); // unknown message
   }
   return 0;
}


/*

5.10.7.3 Specification of operator ~addcounter~

*/
const string AddCounterSpec  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" "
  "\"Example\" ) "
  "( <text>(stream (tuple(X))) x name x int "
  " -> (stream (tuple(X (name int)))) "
  "))</text--->"
  "<text>_ symmproduct [_, _]</text--->"
  "<text>Adds a counter with the given name to the stream "
  "starting at the initial value </text--->"
  "<text>query ten feed addCounter[ Cnt , 1] consume</text--->"
  " ) )";

/*

5.10.7.4 Definition of operator ~addcounter~

*/
Operator extreladdcounter (
         "addcounter",           // name
         AddCounterSpec,         // specification
         AddCounterValueMap,      // value mapping
         Operator::SimpleSelect,  // trivial selection function
         AddCounterTypeMap       // type mapping
);


struct printrefsInfo : OperatorInfo {

  printrefsInfo()
  {
    name      = "printrefs";

    signature = "stream(tuple(...)) -> stream(tuple(...))";
    syntax    = "_ printrefs _";
    meaning   = "Prints out the values of the tuple's "
                "and attribute's reference counter";
  }
};

static ListExpr printrefs_tm(ListExpr args)
{
  NList l(args);

  static const string err1 = "printrefs expects stream(tuple(...))!";

  if ( !l.hasLength(1) )
    return l.typeError(err1);

  NList attrs;
  if ( l.checkStreamTuple( attrs ) )
    return NList::typeError(err1);

  return l.first().listExpr();
}


int printrefs_vm( Word* args, Word& result, int message,
                 Word& local, Supplier s)
{
  Word w(Address(0));

  switch (message)
  {
    case OPEN :

      qp->Open(args[0].addr);
      return 0;

    case REQUEST :

      qp->Request(args[0].addr, w);
      if (qp->Received(args[0].addr))
      {
        Tuple* t = static_cast<Tuple*>( w.addr );
        int tRefs = t->GetNumOfRefs();
        cout << (void*)t << ": " << tRefs << "(";
        for(int i = 0; i < t->GetNoAttributes(); i++)
        {
          cout << " " << t->GetAttribute(i)->NoRefs();
        }
        cout << " )" << endl;

        result.addr = t;
        return YIELD;
      }
      result.addr = 0;
      return CANCEL;

    case CLOSE :

      qp->Close(args[0].addr);
      return 0;
  }
  return 0;
}



/*
2.113 Operator extend[_]aggr

This operator computes the aggregation of an attribute of a tuple stream.
Each provisonal result is appended to the original tuple.


2.113.1 Type Mapping

The Signature is:

  tuplestream x attrName x newAttrName x fun

*/

ListExpr extend_aggrTM(ListExpr args){

  string err = "stream(tuple(...)) x attr x newAttr x fun expected";
  if(!nl->HasLength(args,3)){
    return listutils::typeError(err);
  }
  ListExpr stream = nl->First(args);
  ListExpr AttrNameList = nl->Second(args);
  ListExpr FunList1 = nl->Third(args);

  if(!nl->HasLength(FunList1,1)){ // only one function is allowed
     return listutils::typeError(err + " (only one fun allowed)");
  }
  ListExpr FunList2 = nl->First(FunList1);
  if(!nl->HasLength(FunList2,2)){
     return listutils::typeError(err + " invalid function");
  }


  ListExpr NewAttrName = nl->First(FunList2);
  ListExpr Fun = nl->Second(FunList2);

  if(!Stream<Tuple>::checkType(stream)){
    return listutils::typeError(err + 
                                " (first arg is not a tuple stream) ");
  }

  int AttrNameListLength = nl->ListLength(AttrNameList);
  if(AttrNameListLength!=1 && AttrNameListLength!=2){
     return listutils::typeError(err);
  }
  ListExpr AttrName1 = nl->First(AttrNameList);
  
  if(!listutils::isSymbol(AttrName1)){
    return listutils::typeError(err + 
                        " (second arg is not a valid attribute name ");
  }
  if(!listutils::isSymbol(NewAttrName)){
    return listutils::typeError(err + 
                        " (third arg is not a valid attribute name ");
  }
  if(!listutils::isMap<2>(Fun)){
    return listutils::typeError(err + 
                        " (third arg is not a function");
  }
  ListExpr FunArgType1 = nl->Second(Fun);
  ListExpr FunArgType2 = nl->Third(Fun);
  ListExpr FunResType = nl->Fourth(Fun);
  ListExpr AttrType;
  string attrName1 = nl->SymbolValue(AttrName1);

  ListExpr AttrList = nl->Second(nl->Second(stream));

  int pos = listutils::findAttribute(AttrList, attrName1, AttrType);
  if(pos==0){
    return listutils::typeError("Attribute name " + attrName1 + 
                                " not present in stream");
  }

  if(!nl->Equal(FunArgType1, AttrType)){
    return listutils::typeError("Attribute type doesn't match "
                                "function argument type");
  }
  if(!nl->Equal(FunArgType2, AttrType)){
    return listutils::typeError("Attribute type doesn't match "
                                "function argument type");
  }
  if(!nl->Equal(FunResType, AttrType)){
    return listutils::typeError("Attribute type doesn't match "
                                "function result type");
  }

ListExpr NewAttr = nl->OneElemList(nl->TwoElemList(NewAttrName, AttrType));
ListExpr ResAttrList = listutils::concat(AttrList, NewAttr);

if(!listutils::isAttrList(ResAttrList)){
    return listutils::typeError("Attribute " + nl->ToString(NewAttr) +
                                " already present in stream");
}

ListExpr Res = nl->TwoElemList(
                             nl->SymbolAtom(Stream<Tuple>::BasicType()),
                             nl->TwoElemList(
                                    nl->SymbolAtom(Tuple::BasicType()),
                                    ResAttrList)) ;

int resetPos = -1;
if(AttrNameListLength==2){
  ListExpr AttrName2 = nl->Second(AttrNameList);
  if(!listutils::isSymbol(AttrName2)){
    return listutils::typeError(err);
  }
  string attrName2=nl->SymbolValue(AttrName2);
  ListExpr at;
  int index = listutils::findAttribute(AttrList,attrName2, at);
  if(index==0){
     return listutils::typeError("Attributre " + attrName2 + " not found");
  }
  if(!CcBool::checkType(at)){
    return listutils::typeError("Attrbute " + attrName2 + " not of type bool");
  } 
  resetPos = index -1;
}

return nl->ThreeElemList(nl->SymbolAtom(Symbol::APPEND()),
                         nl->TwoElemList(nl->IntAtom(pos-1),
                         nl->IntAtom(resetPos)
                        ),
                        Res);
}



class ExtendAggrInfo{

public: 
     ExtendAggrInfo(Word& _stream, Supplier _fun, int _attrPos, 
                  int _resetPos, ListExpr type):
     stream(_stream),first(true), fun(_fun), attrPos(_attrPos),
   resetPos(_resetPos), value(0) {
         tupleType = new TupleType(type);
         stream.open();
     }
    
     ~ExtendAggrInfo(){
            stream.close();
            tupleType->DeleteIfAllowed();
            if(value){
                 value->DeleteIfAllowed();
            }
     }

     Tuple* getNextTuple(){

            Tuple* srcTuple = stream.request();
            if(srcTuple==0){
                return 0;
            }

            Tuple* resTuple = new Tuple(tupleType); 
            // copy old attributes
            for(int i=0;i<srcTuple->GetNoAttributes(); i++){
                resTuple->CopyAttribute(i,srcTuple,i);
            }
      bool reset=false;
      if(resetPos>=0){
        CcBool* b = (CcBool*) srcTuple->GetAttribute(resetPos);
        reset = b->IsDefined() && b->GetValue();
      }
            if(first){ 
                value = srcTuple->GetAttribute(attrPos)->Clone();
                first = false;
            } else if(reset){
        value->DeleteIfAllowed();
        value = srcTuple->GetAttribute(attrPos)->Clone();
      } else {
                // compute value from current value and next attribute
                Attribute* attr = srcTuple->GetAttribute(attrPos);
                ArgVectorPointer funargs = qp->Argument(fun);
                (*funargs)[0] = value;
                (*funargs)[1] = attr;
                Word funres;
                qp->Request(fun,funres);
                value->DeleteIfAllowed();
                value = ((Attribute*)funres.addr)->Copy(); 
            }
            resTuple->PutAttribute(srcTuple->GetNoAttributes(), value->Clone());
            srcTuple->DeleteIfAllowed();
            return resTuple;
     }

private:
    Stream<Tuple> stream;
    bool first;
    Supplier fun;
    int attrPos;
  int resetPos;
    Attribute* value; 
    TupleType* tupleType;   
};


int
extend_aggrVM(Word* args, Word& result, int message,
                 Word& local, Supplier s1){

   ExtendAggrInfo* li = (ExtendAggrInfo*) local.addr;
   switch(message){
    case OPEN : {
                  if(li){
                     delete li;
                  }
                  int attrPos = ((CcInt*)args[3].addr)->GetValue();
                  int resetPos = ((CcInt*)args[4].addr)->GetValue();
                  Supplier s = args[2].addr;
                  Supplier s2 = qp->GetSupplier(s,0);
                  Supplier s3 = qp->GetSupplier(s2,1);
                  local.addr = new ExtendAggrInfo(args[0], s3, attrPos,
                                      resetPos, 
                                      nl->Second(GetTupleResultType(s1)));
                  return 0;
                  }
    case REQUEST: {
                     if(!li){
                        return CANCEL;
                     }
                     result.addr = li->getNextTuple();
                     return result.addr?YIELD:CANCEL;
                   }
    case CLOSE: {
                   if(li){
                      delete li;
                      local.addr=0;
                    }
                    return 0;
                }

   }
   return -1;
}

const string extend_aggrSpec  =
"( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
"( <text>stream(tuple(x)) x attrName x newAttrName x fun -> "
  "stream(tuple(x @ newAttrName))</text--->"
  "<text>stream extend_aggr [ attrName ; newAttrName : fun ] )</text--->"
  "<text>Compute an aggrgation over an attribute of a tuple and stored all "
  "provisional results into the" 
  "  result tuples"
  "</text--->"
  "<text>query ten feed extend_aggr[no; aNo : "
  "fun(i1: int, i2: int) i1 + i2 ] </text--->"
  ") )";



/*
2.113.4 Operator instance

*/

Operator extend_aggr(
          "extend_aggr",
          extend_aggrSpec,
          extend_aggrVM,
          Operator::SimpleSelect,
          extend_aggrTM);



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
    AddOperator(&extrelcancel);
    AddOperator(&extrelextract);
    AddOperator(&extrelextend);
    AddOperator(&extrelconcat);
    AddOperator(&extrelmin);
    AddOperator(&extrelmax);

    ValueMapping avgFuns[] = { AvgValueMapping<int,CcInt>,
                         AvgValueMapping<SEC_STD_REAL,CcReal>, 0 };
    ValueMapping sumFuns[] = { SumValueMapping<int,CcInt>,
                         SumValueMapping<SEC_STD_REAL,CcReal>, 0 };
    ValueMapping varFuns[] = { VarValueMapping<int,CcInt>,
                         VarValueMapping<SEC_STD_REAL,CcReal>, 0 };

    AddOperator(avgInfo(), avgFuns, AvgSumSelect, AvgSumTypeMap<true>);
    AddOperator(sumInfo(), sumFuns, AvgSumSelect, AvgSumTypeMap<false>);

    AddOperator(printrefsInfo(), printrefs_vm, printrefs_tm);

    AddOperator(varInfo(), varFuns, AvgSumSelect, AvgSumTypeMap<true>);

    ValueMapping statsFuns[] =
    {
      StatsValueMapping<int,CcInt,int,CcInt>,
      StatsValueMapping<int,CcInt,SEC_STD_REAL,CcReal>,
      StatsValueMapping<SEC_STD_REAL,CcReal,int,CcInt>,
      StatsValueMapping<SEC_STD_REAL,CcReal,SEC_STD_REAL,CcReal>, 0
    };
    AddOperator(statsInfo(), statsFuns, StatsSelect, StatsTypeMap);

    AddOperator(&extrelhead);
    AddOperator(&extrelsortby);
    AddOperator(&extrelsort);
    AddOperator(&extrelrdup);
    AddOperator(&extrelmergesec);
    AddOperator(&extrelmergediff);
    AddOperator(&extrelmergeunion);
    AddOperator(&extrelmergejoin);

    AddOperator(&extrelsortmergejoin);
    AddOperator(&extrelsmouterjoin);
    AddOperator(&extrelhashjoin);
    AddOperator(&extrelloopjoin);
    AddOperator(&extrelextendstream);
    AddOperator(&extrelprojectextendstream);
    AddOperator(&extrelloopsel);
    AddOperator(&extrelgroupby);
    AddOperator(&extrelaggregate);
    AddOperator(&extrelaggregateB);
    AddOperator(&extrelsymmjoin);
    AddOperator(&extrelsymmouterjoin);
    AddOperator(&extrelsymmproductextend);
    AddOperator(&extrelsymmproduct);
    AddOperator(&extrelprojectextend);
    AddOperator(&krdup);
    AddOperator(&extreladdcounter);
    AddOperator(&ksmallest);
    AddOperator(&kbiggest);
    AddOperator(&extrelslidingwindow);
    AddOperator(&extend_aggr);

#ifdef USE_PROGRESS
// support for progress queries
   extrelextend.EnableProgress();
   extrelhead.EnableProgress();
   extrelsortby.EnableProgress();
   extrelsort.EnableProgress();
   extrelrdup.EnableProgress();
   extrelmergejoin.EnableProgress();
   extrelsortmergejoin.EnableProgress();
   extrelsmouterjoin.EnableProgress();
   extrelsymmouterjoin.EnableProgress();
   extrelhashjoin.EnableProgress();
   extrelloopjoin.EnableProgress();
   extrelgroupby.EnableProgress();
   extrelsymmjoin.EnableProgress();
   extrelprojectextend.EnableProgress();
   ksmallest.EnableProgress();
   kbiggest.EnableProgress();
   extrelloopsel.EnableProgress();
   extrelaggregate.EnableProgress();
#endif
  }

  ~ExtRelationAlgebra() {};
};

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
  return (new ExtRelationAlgebra());
}

