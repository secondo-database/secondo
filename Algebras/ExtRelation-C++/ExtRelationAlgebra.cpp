/*
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

#include "RelationAlgebra.h"
#include "QueryProcessor.h"
#include "CPUTimeMeasurer.h"
#include "StandardTypes.h"

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

  if(!nl->IsAtom(args) && nl->ListLength(args) >= 1)
  {
    first = nl->First(args);
    if(!nl->IsAtom(first) && nl->ListLength(first) == 2  )
    {
      tupleDesc = nl->Second(first);
      if(TypeOfRelAlgSymbol(nl->First(first)) == stream
        && (!nl->IsAtom(tupleDesc))
        && (nl->ListLength(tupleDesc) == 2)
        && TypeOfRelAlgSymbol(nl->First(tupleDesc)) == tuple
        && IsTupleDescription(nl->Second(tupleDesc)))
        return
          nl->TwoElemList(
            nl->SymbolAtom("rel"),
            tupleDesc);
    }
  }
  return nl->SymbolAtom("typeerror");
}
/*
2.3.2 Specification of operator ~Group~

*/
const string GroupSpec =
  "(<text>((stream x)) -> (rel x)</text---><text>Maps stream type to a rel "
  "type.</text--->)";
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

  CHECK_COND(nl->ListLength(args) == 3,
    "Operator sample expects a list of length three.");

  first = nl->First(args);
  minSampleSizeLE = nl->Second(args);
  minSampleRateLE = nl->Third(args);

  CHECK_COND(nl->ListLength(first) == 2,
    "Operator sample expects a relation as first argument.");
  CHECK_COND(TypeOfRelAlgSymbol(nl->First(first)) == rel,
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
const string SampleSpec =
  "(<text>(rel x) int real -> (stream x)</text--->"
  "<text>Produces a random sample of a relation. The sample size is "
  "min(relSize, max(s, t * relSize)), where relSize is the "
  "size of the argument relation, s is the second argument, "
  "and t the third.</text--->)";
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
  bool firstcall;
  int noAttrs, j;
  ListExpr first, second, first2, attrtype, newAttrList, lastNewAttrList,
           lastNumberList, numberList, outlist;
  string attrname;
  set<int> removeSet;
  removeSet.clear();

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

  j = FindAttribute(nl->Second(nl->Second(first)), attrname, attrtype);
  if (j)  removeSet.insert(j);
  else
  {
    ErrorReporter::ReportError("Incorrect input for operator ~remove~.");
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

      // Check whether all new attribute names are distinct
      // - not yet implemented
      //check whether the returning list is null
      if (noAttrs>0)
      {outlist = nl->ThreeElemList(
                 nl->SymbolAtom("APPEND"),
     nl->TwoElemList(nl->IntAtom(noAttrs), numberList),
     nl->TwoElemList(nl->SymbolAtom("stream"),
                   nl->TwoElemList(nl->SymbolAtom("tuple"),
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
2.5.2 Value mapping function of operator ~remove~

*/
int Remove(Word* args, Word& result, int message, Word& local, Supplier s)
{

  switch (message)
  {
    case OPEN :
    {
      ListExpr resultType = SecondoSystem::GetCatalog( ExecutableLevel )->NumericType( qp->GetType( s ) );
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
const string RemoveSpec =
  "(<text>((stream (tuple ((x1 T1) ... (xn Tn)))) (ai1 ... aik)) -> (stream "
  "(tuple ((aj1 Tj1) ... (ajn-k Tjn-k))))</text---><text>Produces a removal "
  "tuple for each tuple of its input stream.</text--->)";
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
  && (nl->Equal(nl->Second(first),nl->Second(second)))  )
    return first;
  }

  ErrorReporter::ReportError( "Incorrect input for operator cancel.");
  return nl->SymbolAtom("typeerror");
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
const string CancelSpec =
  "(<text>((stream x) (map x bool)) -> (stream x)</text---><text>Transmits "
  "tuple from its input stream to its output stream until a tuple arrives "
  "fulfilling some condition.</text--->)";
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
  string  attrname;
  int j;

  if(nl->ListLength(args) == 2)
  {
    first = nl->First(args);
    second  = nl->Second(args);

    if((nl->ListLength(first) == 2  ) &&
       (TypeOfRelAlgSymbol(nl->First(first)) == stream)  &&
       (TypeOfRelAlgSymbol(nl->First(nl->Second(first))) == tuple)  &&
       (nl->IsAtom(second)) &&
       (nl->AtomType(second) == SymbolType))
    {
      attrname = nl->SymbolValue(second);
      j = FindAttribute(nl->Second(nl->Second(first)), attrname, attrtype);
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
    index = (int)((StandardAttribute*)args[2].addr)->GetValue();
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
const string ExtractSpec =
  "(<text>((stream (tuple([a1:d1, ... ,an:dn]))) x ai) -> di</text--->"
  "<text>Returns the value of attribute ai of the first tuple in the "
  "input stream.</text--->)";
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

  if(nl->ListLength(args) == 2)
  {
    first = nl->First(args);
    second  = nl->Second(args);

    if((nl->ListLength(first) == 2  )
      && (TypeOfRelAlgSymbol(nl->First(first)) == stream)
      && (TypeOfRelAlgSymbol(nl->First(nl->Second(first))) == tuple)
      && IsTupleDescription(nl->Second(nl->Second(first)))
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
2.8.3 Value mapping function of operator ~head~

*/
int Head(Word* args, Word& result, int message, Word& local, Supplier s)
{
  int maxTuples;
  Word maxTuplesWord;
  Word tupleWord;
  Tuple* tuple;

  switch(message)
  {
    case OPEN:

      qp->Open(args[0].addr);
      local.ival = 0;
      return 0;

    case REQUEST:

      qp->Request(args[1].addr, maxTuplesWord);
      maxTuples = (int)((StandardAttribute*)maxTuplesWord.addr)->GetValue();
      if(local.ival >= maxTuples)
      {
        return CANCEL;
      }

      qp->Request(args[0].addr, tupleWord);
      if(qp->Received(args[0].addr))
      {
        tuple = (Tuple*)tupleWord.addr;
        result = SetWord(tuple);
        local.ival++;
        return YIELD;
      }
      else
      {
        return CANCEL;
      }
    case CLOSE:

      qp->Close(args[0].addr);
      return 0;
  }
  return 0;
}
/*
2.8.4 Specification of operator ~head~

*/
const string HeadSpec =
  "(<text>((stream (tuple([a1:d1, ... ,an:dn]))) x int) -> (stream "
  "(tuple([a1:d1, ... ,an:dn])))</text---><text>Returns the first n tuples "
  "in the input stream.</text--->)";
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
      && (TypeOfRelAlgSymbol(nl->First(first)) == stream)
      && (TypeOfRelAlgSymbol(nl->First(nl->Second(first))) == tuple)
      && IsTupleDescription(nl->Second(nl->Second(first)))
      && (nl->IsAtom(second))
      && (nl->AtomType(second) == SymbolType))
    {
      attrname = nl->SymbolValue(second);
      j = FindAttribute(nl->Second(nl->Second(first)), attrname, attrtype);

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
  int attributeIndex = (int)((StandardAttribute*)args[2].addr)->GetValue() - 1;

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
const string MaxOpSpec =
  "(<text>((stream (tuple([a1:d1, ... ,an:dn]))) x ai) -> di</text--->"
  "<text>Returns the maximum value of attribute ai over the input "
  "stream.</text--->)";
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
const string MinOpSpec =
  "(<text>((stream (tuple([a1:d1, ... ,an:dn]))) x ai) -> di</text--->"
  "<text>Returns the minimum value of attribute ai over the input "
  "stream.</text--->)";

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
      && (TypeOfRelAlgSymbol(nl->First(first)) == stream)
      && (TypeOfRelAlgSymbol(nl->First(nl->Second(first))) == tuple)
      && IsTupleDescription(nl->Second(nl->Second(first)))
      && (nl->IsAtom(second))
      && (nl->AtomType(second) == SymbolType))
    {
      attrname = nl->SymbolValue(second);
      j = FindAttribute(nl->Second(nl->Second(first)), attrname, attrtype);

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

  int attributeIndex = (int)((StandardAttribute*)args[2].addr)->GetValue() - 1;
  char* attributeType = (char*)((StandardAttribute*)args[3].addr)->GetValue();

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
        if(strcmp(attributeType, "real") == 0)
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
        if(isAvg)
        {
          accumulated = currentAttr->Clone();
        }
        else
        {
          accumulated = (Attribute*)qp->ResultStorage(s).addr;
        }
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

      if(strcmp(attributeType, "real") == 0)
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
      result = SetWord(accumulated);
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
const string AvgOpSpec =
  "(<text>((stream (tuple([a1:d1, ... ,an:dn]))) x ai) -> real</text--->"
  "<text>Returns the average value of attribute ai over the "
  "input stream.</text--->)";

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
const string SumOpSpec =
  "(<text>((stream (tuple([a1:d1, ... ,an:dn]))) x ai) -> di</text--->"
  "<text>Returns the sum of the values of attribute ai over the "
  "input stream.</text--->)";
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
  string  attrname;

  if(nl->ListLength(args) == 2)
  {
    ListExpr streamDescription = nl->First(args);
    ListExpr sortSpecification  = nl->Second(args);

    if((nl->ListLength(streamDescription) == 2  ) &&
      (TypeOfRelAlgSymbol(nl->First(streamDescription)) == stream)  &&
      (TypeOfRelAlgSymbol(nl->First(nl->Second(streamDescription))) == tuple))
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
            int j = FindAttribute(nl->Second(nl->Second(streamDescription)), attrname, attrtype);
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
const string SortBySpec =
  "(<text>((stream (tuple([a1:d1, ... ,an:dn]))) ((xi1 asc/desc) ... "
  "(xij asc/desc))) -> (stream (tuple([a1:d1, ... ,an:dn])))</text--->"
  "<text>Sorts input stream according to a list of attributes "
  "ai1 ... aij.</text--->)";

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
  const char* errorMessage = isSort ?
    "Incorrect input for operator sort."
    : "Incorrect input for operator rdup.";

  if(nl->ListLength(args) == 1)
  {
    first = nl->First(args);

    if((nl->ListLength(first) == 2  )
      && (TypeOfRelAlgSymbol(nl->First(first)) == stream)
      && (TypeOfRelAlgSymbol(nl->First(nl->Second(first))) == tuple)
      && IsTupleDescription(nl->Second(nl->Second(first))))
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
2.12.2 Specification of operator ~sort~

*/
const string SortSpec =
  "(<text>((stream (tuple([a1:d1, ... ,an:dn])))) -> "
  "(stream (tuple([a1:d1, ... ,an:dn])))</text---><text>Sorts input "
  "stream lexicographically.</text--->)";

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
const string RdupSpec =
  "(<text>((stream (tuple([a1:d1, ... ,an:dn])))) -> (stream "
  "(tuple([a1:d1, ... ,an:dn])))</text---><text>Removes duplicates from a "
  "sorted stream.</text--->)";

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

const char* setOpErrorMessages[] =
  { "Incorrect input for operator mergesec.",
    "Incorrect input for operator mergediff.",
    "Incorrect input for operator mergeunion." };

template<int errorMessageIdx> ListExpr
SetOpTypeMap( ListExpr args )
{
  ListExpr first, second;

  if(nl->ListLength(args) == 2)
  {
    first = nl->First(args);
    second = nl->Second(args);

    if((nl->ListLength(first) == 2  )
      && (TypeOfRelAlgSymbol(nl->First(first)) == stream)
      && (TypeOfRelAlgSymbol(nl->First(nl->Second(first))) == tuple)
      && IsTupleDescription(nl->Second(nl->Second(first)))
      && (nl->Equal(first, second)))
    {
      return first;
    }
    ErrorReporter::ReportError(setOpErrorMessages[errorMessageIdx]);
    return nl->SymbolAtom("typeerror");
  }
  ErrorReporter::ReportError(setOpErrorMessages[errorMessageIdx]);
  return nl->SymbolAtom("typeerror");
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
const string MergeSecSpec =
  "(<text>((stream (tuple ((x1 t1) ... (xn tn)))) stream (tuple "
  "((x1 t1) ... (xn tn))))) -> (stream (tuple ((x1 t1) ... (xn tn))))"
  "</text---><text>Computes the intersection of two sorted streams.</text--->)";
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
const string MergeDiffSpec =
  "(<text>((stream (tuple ((x1 t1) ... (xn tn)))) stream (tuple ((x1 t1) "
  "... (xn tn))))) -> (stream (tuple ((x1 t1) ... (xn tn))))</text--->"
  "<text>Computes the difference of two sorted streams.</text--->)";

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
const string MergeUnionSpec =
  "(<text>((stream (tuple ((x1 t1) ... (xn tn)))) stream (tuple "
  "((x1 t1) ... (xn tn))))) -> (stream (tuple ((x1 t1) ... (xn tn))))"
  "</text---><text>Computes the union of two sorted streams.</text--->)";

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

const char* joinErrorMessages[] =
  { "Incorrect input for operator mergejoin.",
    "Incorrect input for operator sortmergejoin.",
    "Incorrect input for operator hashjoin." };

template<bool expectIntArgument, int errorMessageIdx> ListExpr JoinTypeMap
(ListExpr args)
{
  ListExpr attrTypeA, attrTypeB;
  ListExpr streamA, streamB, list, list1, list2, outlist;
  if (nl->ListLength(args) == (expectIntArgument ? 5 : 4))
  {
    streamA = nl->First(args); streamB = nl->Second(args);
    if (nl->ListLength(streamA) == 2)
    {
      if (TypeOfRelAlgSymbol(nl->First(streamA)) == stream)
      {
        if (nl->ListLength(nl->Second(streamA)) == 2)
        {
          if (TypeOfRelAlgSymbol(nl->First(nl->Second(streamA))) == tuple)
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
      if (TypeOfRelAlgSymbol(nl->First(streamB)) == stream)
      {
        if (nl->ListLength(nl->Second(streamB)) == 2)
        {
          if (TypeOfRelAlgSymbol(nl->First(nl->Second(streamB))) == tuple)
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

    if(!AttributesAreDisjoint(list1, list2))
    {
      goto typeerror;
    }

    list = ConcatLists(list1, list2);
    outlist = nl->TwoElemList(nl->SymbolAtom("stream"),
      nl->TwoElemList(nl->SymbolAtom("tuple"), list));

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
    int attrAIndex = FindAttribute(nl->Second(nl->Second(streamA)), attrAName, attrTypeA);
    int attrBIndex = FindAttribute(nl->Second(nl->Second(streamB)), attrBName, attrTypeB);
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
  ErrorReporter::ReportError(joinErrorMessages[errorMessageIdx]);
  return nl->SymbolAtom("typeerror");
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
const string MergeJoinSpec =
  "(<text>((stream (tuple ((x1 t1) ... (xn tn)))) (stream (tuple ((y1 d1) "
  "... (ym dm)))) xi yj) -> (stream (tuple ((x1 t1) ... (xn tn) (y1 d1) ... "
  "(ym dm))))</text---><text>Computes the equijoin two streams. Expects that "
  "input streams are sorted.</text--->)";

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
const string SortMergeJoinSpec =
  "(<text>((stream (tuple ((x1 t1) ... (xn tn)))) (stream (tuple "
  "((y1 d1) ... (ym dm)))) xi yj) -> (stream (tuple ((x1 t1) ... (xn tn) "
  "(y1 d1) ... (ym dm))))</text---><text>Computes the equijoin two "
  "streams.</text--->)";

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

2.17.2 Specification of Operator ~hashjoin~

*/
const string HashJoinSpec =
  "(<text>((stream (tuple ((x1 t1) ... (xn tn)))) (stream (tuple "
  "((y1 d1) ... (ym dm)))) xi yj nbuckets) -> (stream (tuple ((x1 t1) ... "
  "(xn tn) (y1 d1) ... (ym dm))))</text---><text>Computes the equijoin two "
  "streams via a hash join. The number of hash buckets is given by the "
  "parameter nBuckets.</text--->)";

/*
2.17.3 Definition of Operator ~hashjoin~

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

----     ((stream x) ((b1 (map x y1)) ... (bm (map x ym))))

        -> (stream (tuple ((a1 x1) ... (an xn) (b1 y1 ... bm ym))))

        wobei x = (tuple ((a1 x1) ... (an xn)))
----

*/
ListExpr ExtendTypeMap( ListExpr args )
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
      (TypeOfRelAlgSymbol(nl->First(first)) == stream) &&
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
            (TypeOfRelAlgSymbol(nl->First(second2)) == ccmap) &&
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
      if ((loopok) && (CompareNames(listn)))
      {
        outlist =
          nl->TwoElemList(
            nl->SymbolAtom("stream"),
            nl->TwoElemList(nl->SymbolAtom("tuple"),listn));
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
      resultType = SecondoSystem::GetCatalog( ExecutableLevel )->NumericType( qp->GetType( s ) );
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
const string ExtendSpec =
  "(<text>(stream(tuple(x)) x [(a1, (tuple(x) -> d1)) ... (an, (tuple(x) -> "
  "dn))] -> stream(tuple(x@[a1:d1, ... , an:dn])))</text---><text>Extends "
  "each input tuple by new attributes as specified in the parameter "
  "list.</text--->)";

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

----  query Staedte feed loopjoin [plz feed filter [.Ort=.SName] ] consume;

      (query (consume (loopjoin (feed tryrel) (fun (t1 TUPLE) (filter (feed null)
    (fun t2 TUPLE) (= (attr t1 name) (attr t2 pname)))))))
----

2.19.1 Type mapping function of operator ~loopjoin~

The type mapping function of the loopjoin operation is as follows:

----    ((stream (tuple x)) (map (tuple x) (stream (tuple y))))  -> (stream (tuple x * y))
  where x = ((x1 t1) ... (xn tn)) and y = ((y1 d1) ... (ym dm))
----

*/
ListExpr LoopjoinTypeMap(ListExpr args)
{
  ListExpr first, second;
  ListExpr list1, list2, list, outlist;

  if(nl->ListLength(args) == 2)
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
}

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
  ArgVectorPointer funargs;
  Word tuplex, tupley, tuplexy, streamy;
  Tuple* ctuplex;
  Tuple* ctupley;
  Tuple* ctuplexy;
  LoopjoinLocalInfo *localinfo;
  ListExpr resultType;

  switch ( message )
  {
    case OPEN:
      qp->Open (args[0].addr);
      localinfo=new LoopjoinLocalInfo;
      resultType = SecondoSystem::GetCatalog( ExecutableLevel )->NumericType( qp->GetType( s ) );
      localinfo->resultTupleType = new TupleType( nl->Second( resultType ) );

      qp->Request(args[0].addr, tuplex);
      if (qp->Received(args[0].addr))
      {
        funargs = qp->Argument(args[1].addr);
        (*funargs)[0] = tuplex;
        streamy=args[1];
        qp->Open (streamy.addr);
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
          else return CANCEL;
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
      qp->Close(args[0].addr);
      localinfo=(LoopjoinLocalInfo *) local.addr;
      delete localinfo->resultTupleType;
      delete localinfo;
      return 0;
  }

  return 0;
}

/*
2.19.3 Specification of operator ~loopjoin~

*/
const string LoopjoinSpec =
  "(<text>((stream tuple1) (map tuple1 rel(tuple2))) -> (stream tuple1*tuple2)</text---><text> Only"
  " tuples in the cartesian product which satisfy certain conditions are passed "
  "on to the output stream.</text--->)";

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
2.20 Operator ~concat~

2.20.1 Type mapping function of operator ~concat~

Type mapping for ~concat~ is

----    ((stream (tuple (a1:d1 ... an:dn))) (stream (tuple (b1:d1 ... bn:dn))))

        -> (stream (tuple (a1:d1 ... an:dn)))
----

*/
ListExpr GetAttrTypeList (ListExpr l)
{
  ListExpr first, olist, lastolist, attrlist;

  olist = nl->TheEmptyList();
  attrlist = l;
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
  if(nl->ListLength(args)  == 2)
  {
    first = nl->First(args);
    second = nl->Second(args);

    if((nl->ListLength(first) == 2) &&
       (TypeOfRelAlgSymbol(nl->First(first)) == stream) &&
       (nl->ListLength(nl->Second(first)) == 2) &&
       (TypeOfRelAlgSymbol(nl->First(nl->Second(first))) == tuple) &&
       (nl->ListLength(second) == 2) &&
       (TypeOfRelAlgSymbol(nl->First(second)) == stream) &&
       (nl->ListLength(nl->Second(second)) == 2) &&
       (TypeOfRelAlgSymbol(nl->First(nl->Second(second))) == tuple) &&
       (nl->Equal(GetAttrTypeList(nl->Second(nl->Second(first))),
          GetAttrTypeList(nl->Second(nl->Second(second))))))
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
const string ConcatSpec =
  "(<text>((stream (tuple (a1:d1 ... an:dn))) (stream (tuple (b1:d1 ... "
  "bn:dn)))) -> (stream (tuple (a1:d1 ... an:dn)))</text---><text>Union "
  "(without duplicate removal.</text--->)";

/*
2.20.4 Definition of operator ~concat~

*/
Operator extrelconcat (
         "concat",              // name
         ConcatSpec,            // specification
         Concat,                // value mapping
         Operator::DummyModel,  // dummy model mapping, defines in Algebra.h
         Operator::SimpleSelect,          // trivial selection function
         ConcatTypeMap          // type mapping
);

/*
2.21 Operator ~groupby~

2.21.1 Type mapping function of operator ~groupby~

Result type of ~groupby~ operation.

----    ((stream (tuple (xi1 ... xin))) ((namei1(fun x y1)) .. (namein (fun x ym)))

        -> (stream (tuple (xi1 .. xin y1 .. ym)))   APPEND (i1,...in)
----

*/
ListExpr GroupByTypeMap(ListExpr args)
{
  ListExpr first, second, third, rest, listn, lastlistn, first2,
    second2, firstr, attrtype, listp, lastlistp;
  ListExpr groupType;
  bool loopok = false;
  string  attrname;
  int j;
  bool firstcall = true;
  int numberatt;
  string listString;

  if(nl->ListLength(args) == 3)
  {
    first = nl->First(args);
    second  = nl->Second(args);
    third  = nl->Third(args);

    if( nl->IsEmpty( third ) )
    {
      ErrorReporter::ReportError("Incorrect input for operator groupby.");
      return nl->SymbolAtom("typeerror");
    }

    if(nl->ListLength(first) == 2  &&
      (TypeOfRelAlgSymbol(nl->First(first)) == stream) &&
      (TypeOfRelAlgSymbol(nl->First(nl->Second(first))) == tuple) &&
      (!nl->IsAtom(second)) &&
      (nl->ListLength(second) > 0))
    {
      numberatt = nl->ListLength(second);
      rest = second;
      while (!nl->IsEmpty(rest))
      {
        first2 = nl->First(rest);
        rest = nl->Rest(rest);
        attrname = nl->SymbolValue(first2);
        j =   FindAttribute(nl->Second(nl->Second(first)), attrname, attrtype);
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
        else
        {
          ErrorReporter::ReportError("Incorrect input for operator groupby.");
          return nl->SymbolAtom("typeerror");
        }

      }
      loopok = true;
      rest = third;

      groupType =
        nl->TwoElemList(
          nl->SymbolAtom("rel"),
          nl->Second(first));

      while (!(nl->IsEmpty(rest)))
      {
        firstr = nl->First(rest);

        rest = nl->Rest(rest);
        first2 = nl->First(firstr);
        second2 = nl->Second(firstr);

        if((nl->IsAtom(first2)) &&
          (nl->ListLength(second2) == 3) &&
          (nl->AtomType(first2) == SymbolType) &&
          (TypeOfRelAlgSymbol(nl->First(second2)) == ccmap) &&
          (nl->Equal(groupType, nl->Second(second2))))
        {
          lastlistn = nl->Append(lastlistn,
          (nl->TwoElemList(first2,nl->Third(second2))));
        }
        else
          loopok = false;
      }
    }
    if ((loopok) && (CompareNames(listn)))
    {
      return
        nl->ThreeElemList(
          nl->SymbolAtom("APPEND"),
          nl->Cons(nl->IntAtom(nl->ListLength(listp)), listp),
          nl->TwoElemList(
            nl->SymbolAtom("stream"),
            nl->TwoElemList(
              nl->SymbolAtom("tuple"),
              listn)));
    }
  }
  ErrorReporter::ReportError("Incorrect input for operator groupby.");
  return nl->SymbolAtom("typeerror");
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
  Tuple *t;
  Tuple *s;
  Word sWord;
  TupleBuffer* tp;
  TupleBufferIterator* relIter;
  int i, j, k;
  int numberatt;
  bool ifequal;
  Word value;
  Supplier  value2;
  Supplier supplier1;
  Supplier supplier2;
  int ind;
  int noOffun;
  ArgVectorPointer vector;
  const int indexOfCountArgument = 3;
  const int startIndexOfExtraArguments = indexOfCountArgument +1;
  int attribIdx;
  Word nAttributesWord;
  Word attribIdxWord;
  GroupByLocalInfo *gbli;

  switch(message)
  {
    case OPEN:
      qp->Open (args[0].addr);
      qp->Request(args[0].addr, sWord);
      if (qp->Received(args[0].addr))
      {
        gbli = new GroupByLocalInfo;
        gbli->t = (Tuple*)sWord.addr;
        ListExpr resultType = SecondoSystem::GetCatalog( ExecutableLevel )->NumericType( qp->GetType( supplier ) );
        gbli->resultTupleType = new TupleType( nl->Second( resultType ) );
        local = SetWord(gbli);
      }
      else
      {
        local = SetWord(0);
      }
      return 0;

    case REQUEST:
      if(local.addr == 0)
        return CANCEL;
      else
      {
        gbli = (GroupByLocalInfo *)local.addr;
        if( gbli->t == 0 )
          return CANCEL;

        t = gbli->t->Clone( true );
        gbli->t->DeleteIfAllowed();
        gbli->t = 0;
        tp = new TupleBuffer();
        tp->AppendTuple(t);
      }
      qp->Request(args[indexOfCountArgument].addr, nAttributesWord);
      numberatt = ((CcInt*)nAttributesWord.addr)->GetIntval();

      ifequal = true;
      qp->Request(args[0].addr, sWord);
      while ((qp->Received(args[0].addr)) && ifequal)
      {
        s = (Tuple*)sWord.addr;
        for (k = 0; k < numberatt; k++)
        {
          qp->Request(args[startIndexOfExtraArguments+k].addr, attribIdxWord);
          attribIdx = ((CcInt*)attribIdxWord.addr)->GetIntval();
          j = attribIdx - 1;
          if (((Attribute*)t->GetAttribute(j))->Compare((Attribute *)s->GetAttribute(j)))
            ifequal = false;
        }
        if (ifequal)
        {
          Tuple *auxS = s;
          s = s->Clone( true );
          auxS->DeleteIfAllowed();
          tp->AppendTuple( s );
          qp->Request(args[0].addr, sWord);
        }
        else
          gbli->t = (Tuple *)sWord.addr;
      }
      if(ifequal)
      {
        gbli->t = 0;
      }

      t = new Tuple( *gbli->resultTupleType, true );
      assert( t->IsFree() == true );
      relIter = tp->MakeScan();
      s = relIter->GetNextTuple();

      for(i = 0; i < numberatt; i++)
      {
        qp->Request(args[startIndexOfExtraArguments+i].addr, attribIdxWord);
        attribIdx = ((CcInt*)attribIdxWord.addr)->GetIntval();
        t->PutAttribute(i, ((Attribute*)s->GetAttribute(attribIdx - 1))->Clone());
      }
      value2 = (Supplier)args[2].addr;
      noOffun  =  qp->GetNoSons(value2);
      assert( t->GetNoAttributes() == numberatt + noOffun );
      delete relIter;

      for(ind = 0; ind < noOffun; ind++)
      {
        supplier1 = qp->GetSupplier(value2, ind);
        supplier2 = qp->GetSupplier(supplier1, 1);
        vector = qp->Argument(supplier2);
        (*vector)[0] = SetWord(tp);
        qp->Request(supplier2, value);
        t->PutAttribute(numberatt + ind, ((Attribute*)value.addr)->Clone()) ;
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
const string GroupBySpec =
  "(<text>((stream (tuple (a1:d1 ... an:dn))) (ai1 ... aik) ((bj1 (fun "
  "(rel (tuple (a1:d1 ... an:dn))) (_))) ... (bjl (fun (rel (tuple "
  "(a1:d1 ... an:dn))) (_))))) -> (stream (tuple (ai1:di1 ... aik:dik bj1 ... "
  "bjl)))</text---><text>Groups a relation according to attributes "
  "ai1, ..., aik and feeds the groups to other functions. The results of those "
  "functions are appended to the grouping attributes.</text--->)";

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
    AddOperator(&extrelhead);
    AddOperator(&extrelmin);
    AddOperator(&extrelmax);
    AddOperator(&extrelavg);
    AddOperator(&extrelsum);
    AddOperator(&extrelsortby);
    AddOperator(&extrelsort);
    AddOperator(&extrelrdup);
    AddOperator(&extrelmergesec);
    AddOperator(&extrelmergediff);
    AddOperator(&extrelmergeunion);
    AddOperator(&extrelmergejoin);
    AddOperator(&extrelsortmergejoin);
    AddOperator(&extrelhashjoin);
    AddOperator(&extrelextend);
    AddOperator(&extrelloopjoin);
    AddOperator(&extrelconcat);
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

