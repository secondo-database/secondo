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

//paragraph [1] Title: [{\Large \bf] [}]
//paragraph [10] Footnote: [{\footnote{] [}}]
//[TOC] [\tableofcontents]
//[newpage] [\newpage]

[1] Implementation of HadoopParallelAlgebra

April 2010 Jiamin Lu

[TOC]

[newpage]

1 Abstract

HadoopParallelAlgebra implements all relevant operators of integrating
Hadoop and Secondo together to execute some parallel operations.
This algebra includes follow operators:

  * ~doubleexport~. Mix two relations into (key, value) style relation.

  * ~parahashjoin~. Execute join operation on a hash partitioned relation
but includes tuples of different schemes.

1 Includes,  Globals

*/

#include <vector>
#include <iostream>
#include <string>
#include "RelationAlgebra.h"
#include "QueryProcessor.h"
#include "StandardTypes.h"
#include "Counter.h"
#include "TupleIdentifier.h"
#include "LogMsg.h"
#include "RTreeAlgebra.h"
#include "ListUtils.h"
#include "HadoopParallelAlgebra.h"
#include "FTextAlgebra.h"
#include "Symbols.h"
#include "Base64.h"
#include "regex.h"
#include "FileSystem.h"

using namespace symbols;
using namespace std;

extern NestedList* nl;
extern QueryProcessor* qp;

/*
2 Operator ~doubleexport~

This operator is usually used in the map function of MapReduce model.
The main work of this operator is to mix tuples from two different
relations of different schemes into one relation following
the (key:string, value:text) pair schema.
The operator extracts the operand field values out as the keys,
and the value field contains two elements,
one is the complete original tuple as ~tupleVal~
and the other one is an integer number ~SI~(source indicator: 1 or 2)
that is used to denote which source relation the ~tupleVal~ comes from.
At the same time, we use Base 64 code to represent the tuple value,
not the nestedList style, because invoking the Tuple class's ~Out~
function is very expensive.

Since the result relation follows the (key, value) style,
the MapReduce module can read the tuples inside this relation,
and group the tuples come from different
source relations but with a same key value
together into one reduce function.
Therefore in each reduce function,
we can call Secondo to process some queries
only for these tuples with a same key value.

2.1 Specification of Operator ~doubleexport~

*/

struct doubleExportInfo : OperatorInfo
{
  doubleExportInfo()
  {
    name = "doubleexport";
    signature =
        "stream (tuple((a1 t1) ... (ai ti) ... (an tm)))"
        "x stream (tuple((b1 p1) ... (bj tj) ... (bm tm)))"
        "x ai x bj -> "
        "stream (tuple (key:string) (value:string))";
    syntax = "_ _ doubleexport[_ , _]";
    meaning = "Mix two relations into (key, value) pairs";
  }
};

/*
2.1 Type Mapping of Operator ~doubleexport~

---- ((stream (tuple((a1 t1) ... (ai string) ... (an tm))))
     (stream (tuple((b1 p1) ... (bj string) ... (bm pm)))) ai bj )
     -> ((stream (tuple (key: string) (value: text)))
     APPEND (i j))
----

*/
ListExpr doubleExportTypeMap(ListExpr args)
{
  if (nl->ListLength(args) != 4)
  {
    ErrorReporter::ReportError(
      "Operator doubleexport expect a list of four arguments");
    return nl->TypeError();
  }

  if (listutils::isTupleStream(nl->First(args))
      && listutils::isTupleStream(nl->Second(args))
      && listutils::isSymbol(nl->Third(args))
      && listutils::isSymbol(nl->Fourth(args)))
  {
    //Get the indices of two indicated key attributes
    ListExpr tupTypeA, tupTypeB;
    tupTypeA = nl->Second(nl->First(args));
    tupTypeB = nl->Second(nl->Second(args));

    ListExpr attrTypeA, attrTypeB;
    ListExpr tupListA = nl->Second(tupTypeA);
    string attrAName = nl->SymbolValue(nl->Third(args));
    int attrAIndex =
        listutils::findAttribute(tupListA,attrAName,attrTypeA);
    if (attrAIndex <= 0)
    {
      ErrorReporter::ReportError(
        "Attributename " + attrAName
        + " not found in the first argument");
      return nl->TypeError();
    }

    ListExpr tupListB = nl->Second(nl->Second(nl->Second(args)));
    string attrBName = nl->SymbolValue(nl->Fourth(args));
    int attrBIndex =
        listutils::findAttribute(tupListB,attrBName,attrTypeB);
    if (attrBIndex <= 0)
    {
      ErrorReporter::ReportError(
        "Attributename " + attrBName
        + " not found in the second argument");
      return nl->TypeError();
    }

    if (listutils::isDATA(attrTypeA)
      && listutils::isDATA(attrTypeB)
      && nl->Equal(attrTypeA, attrTypeB))
    {
      ListExpr attrList = nl->TwoElemList(
          nl->TwoElemList(nl->StringAtom("keyT",false),
              nl->SymbolAtom(STRING)),
          nl->TwoElemList(nl->StringAtom("valueT",false),
              nl->SymbolAtom(TEXT)));
      NList AttrList(attrList, nl);
      NList tupleStreamList =
          NList(NList().tupleStreamOf(AttrList));

      return nl->ThreeElemList(
                 nl->SymbolAtom("APPEND"),
                 nl->TwoElemList(nl->IntAtom(attrAIndex),
                                 nl->IntAtom(attrBIndex)),
                 tupleStreamList.listExpr());
    }
    else
    {
      ErrorReporter::ReportError(
        "Operator doubleexport expect "
          "two same and DATA kind key types.");
      return nl->TypeError();
    }
  }
  else
  {
    ErrorReporter::ReportError(
      "Operator doubleexport expect: "
      "stream (tuple((a1 t1) ... (ai ti) ... (an tm)))"
      "x stream (tuple((b1 p1) ... (bj tj) ... (bm tm)))"
      "x ai x bj -> stream (tuple (key:text) (value:text))");
    return nl->TypeError();
  }
}

/*
2.2 Value Mapping of Operator ~doubleexport~

*/

int doubleExportValueMap(Word* args, Word& result,
    int message, Word& local, Supplier s)
{
  deLocalInfo *localInfo;

  switch (message)
  {
  case OPEN:
    qp->Open(args[0].addr);
    qp->Open(args[1].addr);

    localInfo =
        new deLocalInfo(args[0], args[4], args[1], args[5], s);
    local = SetWord(localInfo);
    return 0;
  case REQUEST:
    localInfo = (deLocalInfo*) local.addr;
    result.setAddr(localInfo->nextResultTuple());

    return result.addr != 0 ? YIELD : CANCEL;

  case CLOSE:
    qp->Close(args[0].addr);
    qp->Close(args[1].addr);

    localInfo = (deLocalInfo*) local.addr;
    delete localInfo;
    local.addr = 0;
    return 0;
  }
  return 0;
}

/*
2.3 Auxiliary Functions of Operator ~doubleexport~

*/

deLocalInfo::deLocalInfo(Word _streamA, Word wAttrIndexA,
                         Word _streamB, Word wAttrIndexB,
                         Supplier s):resultTupleType(0)
{
  streamA = _streamA;
  streamB = _streamB;
  attrIndexA = StdTypes::GetInt( wAttrIndexA ) - 1;
  attrIndexB = StdTypes::GetInt( wAttrIndexB ) - 1;
  isAEnd = false;

  ListExpr resultType = GetTupleResultType(s);
  resultTupleType = new TupleType( nl->Second( resultType ) );
}

/*
Get tuples from streamA first, and set their ~SI~ as 1.
After traverse the tuples in streamA, get all tuples from streamB,
and set their ~SI~ as 2.

*/

Tuple* deLocalInfo::nextResultTuple()
{
  Tuple* tuple = 0;

  if(!isAEnd){
    tuple = makeTuple(streamA, attrIndexA, 1);
    if (tuple == 0)
      isAEnd = true;
    else
      return tuple;
  }
  tuple = makeTuple(streamB, attrIndexB, 2);
  return tuple;
}

Tuple* deLocalInfo::makeTuple(Word stream, int index,int SI)
{
  bool yield = false;
  Word result;
  Tuple *oldTuple, *newTuple = 0;

  qp->Request(stream.addr, result);
  yield = qp->Received(stream.addr);

  if (yield){
    //Get a tuple from the stream;
    oldTuple = static_cast<Tuple*>(result.addr);

    string key =
        ((Attribute*)(oldTuple->GetAttribute(index)))->getCsvStr();
    string tupStr = oldTuple->WriteToBinStr();
    stringstream vs;
    vs << "(" << SI << " '" << tupStr << "')";

    newTuple = new Tuple(resultTupleType);
    newTuple->PutAttribute(0,new CcString(key));
    newTuple->PutAttribute(1,new FText(true, vs.str()));

    oldTuple->DeleteIfAllowed();
  }

  return newTuple;
}

string binEncode(ListExpr nestList)
{
  stringstream iss, oss;
  nl->WriteBinaryTo(nestList, iss);
  Base64 b64;
  b64.encodeStream(iss, oss);
  string valueStr = oss.str();
  valueStr = replaceAll(valueStr, "\n", "");
  return valueStr;
}

ListExpr binDecode(string binStr)
{
  Base64 b64;
  stringstream iss, oss;
  ListExpr nestList;
  iss << binStr;
  b64.decodeStream(iss, oss);
  nl->ReadBinaryFrom(oss, nestList);
  return nestList;
}

/*
3 Operator ~parahashjoin~

Operator ~parahashjoin~ is used to execute Cartesian product for
a serious of tuples from two different relations
grouped by their join attribute value already
but mixed together in (key, value) schema from Hadoop.

Together with ~doubleexport~ operator, Hadoop has already automatically
finish the hash partition period of a hash join, the tuples
from different source relations but have a same join attribute value,
i.e. inside a same hash bucket will be processed in one reduce function.
However, the number of the tuples inside one hash bucket may be
very small, calling Secondo every time in reduce functions just to process
a few number of tuples is not an efficient solution.
Therefore, in the reduce function, we only send
the tuples into Secondo, and invoke Secondo only once to process the
join operation at last. ~parahashjoin~ is the operator created
to execute the last operation.

At the same time, since the keys that Hadoop uses to partition
tuples into different hash buckets are useless in reduce functions,
they will be abandoned, and only the value parts of the tuples outputed
from ~doubleexport~ operation will be sent into Secondo following
the schema: ((SI + tupleVal) :text).
The ~SI~ is the key field, the ~tupleVal~ is the complete value
of the source tuple in Base 64 code.
And we encapsulate these two value into one text value.

If we only simply send this kind of tuples back to Secondo,
the tuples with different join attributes will be mixed again,
though they have already been grouped automatically by Hadoop.
For avoiding this, in reduce functions,
we send ~OTuple~s whose ~SI~ value is 0 to
separate different hash buckets.

After above procedure, ~parahashjoin~ can easily get tuples
inside one hash bucket with the help of ~OTuple~.
For each hash bucket, ~parahashjoin~ use the key field ~SI~ to
distinguish tuples from different source relations.
Then since all tuples inside have a same join attribute value already,
a simple Cartesian product is caculated for these distinguished tuples.

3.1 Specification of Operator ~parahashjoin~

*/

struct paraHashJoinInfo : OperatorInfo
{
  paraHashJoinInfo()
  {
    name = "parahashjoin";
    signature =
        "stream(tuple((key:int) (value:text)))"
        "x (rel(tuple((a1 t1) ... (an tn))))"
        "x (rel(tuple((b1 p1) ... (bm pm))))"
        "-> stream(tuple((a1 t1) ... (an tn)(b1 p1) ... (bm pm)))";
    syntax = "_ _ _ parahashjoin";
    meaning = "Execute join on a hash partitioned relation";
  }
};

/*
3.1 Type Mapping of Operator ~parahashjoin~

---- ((stream (tuple((key:int) (value:text))))
       x (rel(tuple((a1 t1) ... (an tn))))
       x (rel(tuple((b1 p1) ... (bm pm))))
     -> stream(tuple((a1 t1) ... (an tn)(b1 p1) ... (bm pm))))
----

*/
ListExpr paraHashJoinTypeMap(ListExpr args)
{

  if (nl->ListLength(args) != 3)
  {
    ErrorReporter::ReportError(
      "Operator parahashjoin expect a list of three arguments");
    return nl->TypeError();
  }

  ListExpr stream = nl->First(args);
  ListExpr relA = nl->Second(args);
  ListExpr relB = nl->Third(args);

  if (listutils::isTupleStream(stream)
    && listutils::isRelDescription(relA)
    && listutils::isRelDescription(relB))
  {
    ListExpr streamTupleList = nl->Second(nl->Second(stream));
    if (nl->ListLength(streamTupleList) != 1)
    {
      ErrorReporter::ReportError(
        "Operator parahashjoin only accept tuple stream "
        "with one TEXT type argument");
      return nl->TypeError();
    }
    else if (!listutils::isSymbol(
        nl->Second(nl->First(streamTupleList)),TEXT))
    {
      ErrorReporter::ReportError(
              "Operator parahashjoin only accept tuple stream "
              "with one TEXT type argument");
      return nl->TypeError();
    }

    ListExpr rAtupNList =
        renameList(nl->Second(nl->Second(relA)), "1");
    ListExpr rBtupNList =
        renameList(nl->Second(nl->Second(relB)), "2");
    ListExpr resultAttrList = ConcatLists(rAtupNList, rBtupNList);
    ListExpr resultList = nl->TwoElemList(nl->SymbolAtom("stream"),
          nl->TwoElemList(nl->SymbolAtom("tuple"), resultAttrList));

    return resultList;

  }
  else
  {
    ErrorReporter::ReportError(
      "Operator parahashjoin expect input as "
        "stream(tuple) x rel(tuple) x rel(tuple)");
    return nl->TypeError();
  }

}

/*
Rename the attributes in both relations to avoid duplication of names.

*/

ListExpr renameList(ListExpr oldTupleList, string appendName)
{
  NList newList;
  ListExpr rest = oldTupleList;
  while(!nl->IsEmpty(rest)){
    ListExpr tuple = nl->First(rest);
    string attrname = nl->SymbolValue(nl->First(tuple));
    attrname.append("_" + appendName);

    NList newTuple(nl->TwoElemList(
                    nl->SymbolAtom(attrname),
                    nl->Second(tuple)));
    newList.append(newTuple);
    rest = nl->Rest(rest);
  }
  return newList.listExpr();
}

/*
3.2 Value Mapping of Operator ~parahashjoin~

*/
int paraHashJoinValueMap(Word* args, Word& result,
                int message, Word& local, Supplier s)
{

  phjLocalInfo *localInfo;
  ListExpr aTupleTypeList, bTupleTypeList;

  switch (message)
  {
  case OPEN:
    qp->Open(args[0].addr);

    aTupleTypeList =
        SecondoSystem::GetCatalog()->NumericType(
        nl->Second(qp->GetSupplierTypeExpr(qp->GetSon(s,1))));
    bTupleTypeList =
        SecondoSystem::GetCatalog()->NumericType(
        nl->Second(qp->GetSupplierTypeExpr(qp->GetSon(s,2))));

    localInfo = new phjLocalInfo(args[0], s,
        aTupleTypeList, bTupleTypeList);
    local = SetWord(localInfo);
    return 0;
  case REQUEST:
    localInfo = (phjLocalInfo*) local.addr;
    result = localInfo->nextJoinTuple();

    return result.addr !=0 ? YIELD : CANCEL;
  case CLOSE:
    qp->Close(args[0].addr);

    localInfo = (phjLocalInfo*) local.addr;
    delete localInfo;
    localInfo = 0;
    return 0;
  }
  return 0;
}

/*
3.3 Auxiliary Functions of Operator ~parahashjoin~

*/

phjLocalInfo::phjLocalInfo(Word _stream, Supplier s,
    ListExpr ttA, ListExpr ttB)
{
  mixStream = _stream;

  ListExpr resultType = GetTupleResultType(s);
  resultTupleType = new TupleType(nl->Second(resultType));

  tupleTypeA = new TupleType(ttA);
  tupleTypeB = new TupleType(ttB);

  joinedTuples = 0;
  tupleIterator = 0;
}

/*
Ask for new tuples from ~joinedTuples~.
If there's no more tuples inside ~joinedTuples~,
then invoke ~getNewProducts~ to get new results.

*/
Word phjLocalInfo::nextJoinTuple()
{
  Tuple *tuple;

  if (tupleIterator != 0)
  {
    if ((tuple = tupleIterator->GetNextTuple()) != 0)
      return SetWord(tuple);
    else
    {
      delete tupleIterator;
      tupleIterator = 0;
    }
  }

  if ((tupleIterator = getNewProducts()) != 0)
  {
    tuple = tupleIterator->GetNextTuple();
    return SetWord(tuple);
  }

  return SetWord(Address(0));
}


/*
Collect and distinguish tuples of one bucket.
If the key field value, i.e. ~SI~ is 1, means the tuple comes from rel1,
and if ~SI~ is 2, then means the tuple comes from rel2.
Besides, if ~SI~ is 0, then the tuple is the separator tuple(~ST~).

If the tuples of one bucket all come from a same source relation,
then jump to next bucket because there will be no product results in
this bucket.
Or else, make the products, and put the result tuples into the ~joinedTuples~.

*/

GenericRelationIterator* phjLocalInfo::getNewProducts()
{

  TupleBuffer *tbA = 0;
  TupleBuffer *tbB = 0;
  GenericRelationIterator *iteratorA = 0, *iteratorB = 0;
  Tuple *tupleA = 0, *tupleB = 0;
  string tupStr, sTupStr;
  long MaxMem = qp->MemoryAvailableForOperator();

  //  Traverse the stream, until there is no more tuples exists,
  //  or the ~joinedTuples~ is filled.
  while(true)
  {
    tbA = new TupleBuffer(MaxMem);
    tbB = new TupleBuffer(MaxMem);
    //  Collect tuples in one bucket.
    Word currentTupleWord(Address(0));
    bool isInBucket = true;
    qp->Request(mixStream.addr, currentTupleWord);

    while(qp->Received(mixStream.addr))
    {
      Tuple* currentTuple =
          static_cast<Tuple*> (currentTupleWord.addr);
      tupStr =
          ((FText*) (currentTuple->GetAttribute(0)))->GetValue();
      currentTuple->DeleteIfAllowed();

      int SI = atoi(tupStr.substr(1,1).c_str());
      sTupStr = tupStr.substr(4, tupStr.size() - 6);

      switch (SI)
      {
      case 1:{
        tupleA = new Tuple(tupleTypeA);
        tupleA->ReadFromBinStr(sTupStr);
        tbA->AppendTuple(tupleA);
        tupleA->DeleteIfAllowed();
        break;
      }
      case 2:{
        tupleB = new Tuple(tupleTypeB);
        tupleB->ReadFromBinStr(sTupStr);
        tbB->AppendTuple(tupleB);
        tupleB->DeleteIfAllowed();
        break;
      }
      case 0:{
        isInBucket = false;
        break;
      }
      default:{
        //should never be here
        cerr << "Exist tuples with error SI value" << endl;
        assert(false);
      }
      }

      if (isInBucket)
        qp->Request(mixStream.addr, currentTupleWord);
      else
        break;
    }

    int countA = tbA->GetNoTuples();
    int countB = tbB->GetNoTuples();

    if(countA == 0 && countB == 0)
    {
      // No more data exists
      delete tbA;
      delete tbB;
      return false;
    }
    else if(countA == 0 || countB == 0)
    {
      // All tuples come from one source relation
      delete tbA;
      delete tbB;
    }
    else
    {
      //compute the products
      if (joinedTuples != 0)
        delete joinedTuples;
      joinedTuples = new TupleBuffer(MaxMem);

      int i = 0, j = 0;
      iteratorA = tbA->MakeScan();
      tupleA = iteratorA->GetNextTuple();
      while(tupleA && i++ < countA)
      {
        j = 0;
        iteratorB = tbB->MakeScan();
        tupleB = iteratorB->GetNextTuple();
        while(tupleB && j++ < countB)
        {
          Tuple *resultTuple = new Tuple(resultTupleType);
          Concat(tupleA, tupleB,resultTuple);
          tupleB->DeleteIfAllowed();

          joinedTuples->AppendTuple(resultTuple);
          resultTuple->DeleteIfAllowed();
          tupleB = iteratorB->GetNextTuple();
        }
        delete iteratorB;
        tupleA->DeleteIfAllowed();
        tupleA = iteratorA->GetNextTuple();
      }
      delete iteratorA;

      delete tbA;
      delete tbB;

      return joinedTuples->MakeScan();
    }
  }
  return 0;
}

/*
4. Type Operator ~TUPSTREAM~

This type operator extract the type of the element from a rel type
given as the first argument,
and forwards this type encapsulated in a stream type.

----
    ( (rel(T1)) ... ) -> stream(T1)
----

4.1 Specification of Operator ~TUPSTREAM~

*/

struct TUPSTREAMInfo : OperatorInfo
{
  TUPSTREAMInfo()
  {
    name = "TUPSTREAM";
    signature =
        "( (rel(T1)) ... ) -> stream(T1)";
    syntax = "type operator";
    meaning = "Extract the tuple of a relation "
        "from the first argument, and forward it as a stream";
  }
};

ListExpr TUPSTREAMType( ListExpr args)
{
  ListExpr first;
  CHECK_COND(nl->ListLength(args) >= 1,
      "Expect one argument at least");
  first = nl->First(args);
  CHECK_COND(listutils::isRelDescription(first),
      "rel(tuple(...)) expected");
  return nl->TwoElemList(nl->SymbolAtom(STREAM), nl->Second(first));
}


/*
4. Type Operator ~TUPSTREAM2~

This type operator extract the type of the element from a rel type
given as the second argument,
and forwards this type encapsulated in a stream type.

----
    ( T1 (rel(T2)) ... ) -> stream(T2)
----

4.1 Specification of Operator ~TUPSTREAM2~

*/

struct TUPSTREAM2Info : OperatorInfo
{
  TUPSTREAM2Info()
  {
    name = "TUPSTREAM2";
    signature =
        "( T1 (rel(T2)) ... ) -> stream(T2)";
    syntax = "type operator";
    meaning = "Extract the tuple of a relation "
        "from the second argument, and forward it as a stream";
  }
};

ListExpr TUPSTREAM2Type( ListExpr args)
{
  ListExpr second;
  CHECK_COND(nl->ListLength(args) >= 2,
      "Expect two argument at least");
  second = nl->Second(args);
  CHECK_COND(listutils::isRelDescription(second),
      "rel(tuple(...)) expected");
  return nl->TwoElemList(nl->SymbolAtom(STREAM), nl->Second(second));
}


/*
4. Type Operator ~TUPSTREAM3~

This type operator extract the type of the element from a rel type
given as the third argument,
and forwards this type encapsulated in a stream type.

----
    ( T1 T2 (rel(T3)) ... ) -> stream(T3)
----

4.1 Specification of Operator ~TUPSTREAM3~

*/

struct TUPSTREAM3Info : OperatorInfo
{
  TUPSTREAM3Info()
  {
    name = "TUPSTREAM3";
    signature =
        "( T1 T2 (rel(T3)) ... ) -> stream(T3)";
    syntax = "type operator";
    meaning = "Extract the tuple of a relation "
        "from the third argument, and forward it as a stream";
  }
};

ListExpr TUPSTREAM3Type( ListExpr args)
{
  ListExpr third;
  CHECK_COND(nl->ListLength(args) >= 1,
      "Expect one argument at least");
  third = nl->Third(args);
  CHECK_COND(listutils::isRelDescription(third),
      "rel(tuple(...)) expected");
  return nl->TwoElemList(nl->SymbolAtom(STREAM), nl->Second(third));
}

/*
5 Operator ~parajoin~

Operator ~parahashjoin~ can only execute ~product~ operation for the
tuples belong to different source relation but inside
a same hash bucket.
However, for some specific join operations like spatial operation,
tuples inside one bucket don't means they have an exactly same
join attribute value, and so does the Cartesian product can't be
executed directly for these tuples.

At the same time, ~parahashjoin~ is inefficient for some big join
operations, since we store all result tuples into a temporal
tupleBuffer which will visit the disk if the amount of the
result tuples is too large.

Therefore, we need to create the operator ~parajoin~ that can process
the tuples inside one hash bucket but with different join operations.
Similar with ~parahashjoin~, ~parajoin~ accept the stream mixed with
tuples following two different schemes. These tuples are
partitioned into different buckets according to their join
attribute values, and use ~0Tuple~s to separate these
buckets. At the same time, each tuple contains a ~SI~ value to
indicate which source relations it comes from or dose it a ~OTuple~.
With the ~SI~ values, the operator can get all tuples in one bucket,
and distinguish them into two tuple buffers.

The difference of ~parajoin~ between ~parahashjoin~ is that it can
accept any kind of join operator as its parameter function,
and use this function to execute different join operations for the
tuples inside one hash bucket.
The type of operators can be accepted in ~parajoin~ should be like:

---- stream(T1) x stream(T2) -> stream(T3)
----

The main problem here is that
the function should accept two streams as input, and output
a stream, which doesn't like normal functions which only can
accept DATA object and output DATA or stream.
But thanks to the PartittionedStream algebra, it modify the kernel of
Secondo, and make this kind of function be possible.

For making the functions be possible to accept two streams as input,
we can store the supplier of this operator at the tail two positions
of this function's argument list. Then the query processor knows that
these two inputs are streams, and will use specific messages to drive
the function work.

*/

struct paraJoinInfo : OperatorInfo
{
  paraJoinInfo()
  {
    name = "parajoin";
    signature =
        "( (stream(tuple((key int)(value text))))"
        "x(rel(tuple(T1))) x (rel(tuple(T2)))"
        "x(map (stream(T1)) (stream(T2)) (stream(T1 T2))) )"
        " -> stream(tuple(T1 T2))";
    syntax = "_ _ _ parajoin [fun]";
    meaning = "join mixed tuples from two relations";
  }
};

/*
5.1 Type Mapping of Operator ~parajoin~

----
    (  (stream(tuple((value text))))
     x (rel(tuple(T1))) x (rel(tuple(T2)))
     x ((map (stream(T1)) (stream(T2)) (stream(T1 T2))))  )
     -> stream(tuple(T1 T2))
----

*/

ListExpr paraJoinTypeMap( ListExpr args )
{
  if (nl->ListLength(args) == 4)
  {
    // parajoin for taking mixed streams

    ListExpr streamList = nl->First(args);
    ListExpr relAList = nl->Second(args);
    ListExpr relBList = nl->Third(args);
    ListExpr mapNL = nl->Fourth(args);

    if (listutils::isTupleStream(streamList)
      && listutils::isRelDescription(relAList)
      && listutils::isRelDescription(relBList))
    {
      ListExpr attrList = nl->Second(nl->Second(streamList));
      if (nl->ListLength(attrList) != 1)
      {
        ErrorReporter::ReportError(
          "Operator parajoin only accept tuple stream "
          "with one TEXT type argument");
        return nl->TypeError();
      }
      else if (!listutils::isSymbol(
        nl->Second(nl->First(attrList)),TEXT))
      {
        ErrorReporter::ReportError(
          "Operator parajoin only accept tuple stream "
          "with one TEXT type argument");
        return nl->TypeError();
      }

      if (listutils::isMap<2>(mapNL))
      {
        if (listutils::isTupleStream(nl->Second(mapNL))
          && listutils::isTupleStream(nl->Third(mapNL))
          && listutils::isTupleStream(nl->Fourth(mapNL)))
        {
          ListExpr resultList = nl->TwoElemList(
                nl->SymbolAtom("stream"),
                nl->TwoElemList(nl->SymbolAtom("tuple"),
                    nl->Second(nl->Second(nl->Fourth(mapNL)))));

          return resultList;
        }
        else
        {
          ErrorReporter::ReportError(
              "Operator parajoin expects parameter function "
              "as (map (stream(T1)) (stream(T2)) (stream(T1 T2)))");
          return nl->TypeError();
        }
      }
      else
      {
        ErrorReporter::ReportError(
          "Operator parajoin expects binary function "
            "as the fourth argument.");
        return nl->TypeError();
      }
    }
    else
    {
      ErrorReporter::ReportError(
        "Operator parajoin expect "
          "(stream(tuple((value text))))"
          "x(rel(tuple(T1))) x (rel(tuple(T2)))"
          "x((map (stream(T1)) (stream(T2)) (stream(T1 T2))))");
      return nl->TypeError();
    }
  }
  else
  {
    ErrorReporter::ReportError(
      "Operator parajoin expect a list of four arguments");
    return nl->TypeError();
  }


}

/*
5.2 Value Mapping of Operator ~parajoin~

Here the message like ~(1[*]FUNMSG)+OPEN~ means the function
needs to open its first stream, and ~(1[*]FUNMSG)+REQUEST~ means
the function needs to request its first stream, and so do other
similar messages.

*/
int paraJoinValueMap(Word* args, Word& result,
                int message, Word& local, Supplier s)
{

  pjLocalInfo *localInfo;
  ListExpr aTupleTypeList, bTupleTypeList;

  switch (message)
  {
  case OPEN:{
    qp->Open(args[0].addr);

    aTupleTypeList =
        SecondoSystem::GetCatalog()->NumericType(
        nl->Second(qp->GetSupplierTypeExpr(qp->GetSon(s,1))));
    bTupleTypeList =
        SecondoSystem::GetCatalog()->NumericType(
        nl->Second(qp->GetSupplierTypeExpr(qp->GetSon(s,2))));

    localInfo = new pjLocalInfo(args[0], args[3].addr, s,
        aTupleTypeList, bTupleTypeList,
        qp->MemoryAvailableForOperator());

    local.setAddr(localInfo);
    return 0;
  }
  case REQUEST:{
    // ask the fun to get the result tuple.
    if (local.addr == 0)
      return CANCEL;
    localInfo = (pjLocalInfo*) local.addr;

    result.setAddr(localInfo->getNextTuple());
    if (result.addr)
      return YIELD;
    else
      return CANCEL;
  }
  case (1*FUNMSG)+OPEN:{
    return 0;
  }
  case (2*FUNMSG)+OPEN:{
    return 0;
  }
  case (1*FUNMSG)+REQUEST:{
    if (local.addr == 0)
      return CANCEL;
    localInfo = (pjLocalInfo*) local.addr;

    result.setAddr(localInfo->getNextInputTuple(tupBufferA));
    if ( result.addr != 0)
      return YIELD;
    else
      return CANCEL;
  }
  case (2*FUNMSG)+REQUEST:{
    if (local.addr == 0)
      return CANCEL;
    localInfo = (pjLocalInfo*) local.addr;

    result.setAddr(localInfo->getNextInputTuple(tupBufferB));
    if ( result.addr != 0)
      return YIELD;
    else
      return CANCEL;
  }
  case (1*FUNMSG)+CLOSE:{
    return 0;
  }
  case (2*FUNMSG)+CLOSE:{
    return 0;
  }
  case CLOSE:{
    if (local.addr == 0)
      return CANCEL;
    localInfo = (pjLocalInfo*) local.addr;

    delete localInfo;
    qp->Close(args[0].addr);
    return 0;
  }
  }

  return 0;
}



/*
3.3 Auxiliary Functions of Operator ~parajoin~

Load one bucket tuples from the input tuple stream,
and fill them into two different tupleBuffers according to the
~SI~ value it contains.
If the tuples in that bucket all come from one source relation,
then move to the next bucket directly.

*/
void pjLocalInfo::loadTuples()
{
  if (endOfStream)
  {
    cerr << "The input mixed stream is exhausted." << endl;
    return;
  }

  Word cTupleWord(Address(0));
  bool isInBucket;
  Tuple *cTuple = 0;
  Tuple *tupleA = 0, *tupleB = 0;
  string tupStr, sTupStr;

  if (itrA != 0)
    delete itrA;
  itrA = 0;
  if(tbA != 0)
    delete tbA;
  tbA = 0;

  if (itrB != 0)
    delete itrB;
  itrB = 0;
  if(tbB != 0)
    delete tbB;
  tbB = 0;

  while (!endOfStream)
  {
    tbA = new TupleBuffer(maxMem);
    tbB = new TupleBuffer(maxMem);
    isBufferFilled = false;
    isInBucket = true;

    qp->Request(mixedStream.addr, cTupleWord);
    while (isInBucket && qp->Received(mixedStream.addr))
    {
      cTuple = static_cast<Tuple*> (cTupleWord.addr);
      tupStr = ((FText*) (cTuple->GetAttribute(0)))->GetValue();

      int SI = atoi(tupStr.substr(1,1).c_str());
      sTupStr = tupStr.substr(4, tupStr.size() - 6);

      switch (SI)
      {
      case 1:
      {
        tupleA = new Tuple(tupleTypeA);
        tupleA->ReadFromBinStr(sTupStr);
        tbA->AppendTuple(tupleA);
        tupleA->DeleteIfAllowed();
        break;
      }
      case 2:
      {
        tupleB = new Tuple(tupleTypeB);
        tupleB->ReadFromBinStr(sTupStr);
        tbB->AppendTuple(tupleB);
        tupleB->DeleteIfAllowed();
        break;
      }
      case 0:
      {
        isInBucket = false;
        break;
      }
      default:
      {
        //should never be here
        cerr << "Exist tuples with error SI value" << endl;
        assert(false);
      }
      }

      cTuple->DeleteIfAllowed();
      if (isInBucket)
        qp->Request(mixedStream.addr, cTupleWord);
    }

    int numOfA = tbA->GetNoTuples();
    int numOfB = tbB->GetNoTuples();

    if (numOfA == 0 && numOfB == 0)
    {
      delete tbA;
      delete tbB;
      tbA = tbB = 0;
      endOfStream = true;
      break;
    }
    else if (numOfA == 0 || numOfB == 0)
    {
      delete tbA;
      delete tbB;
      tbA = tbB = 0;
    }
    else
    {
      isBufferFilled = true;
      itrA = tbA->MakeScan();
      itrB = tbB->MakeScan();
      break;
    }
  }

}

/*
Take one tuple from tupleBuffer A or B.
When the operator in the parameter function need one tuple
from the input stream, it gets the tuple from the
filled tuple buffer actually. When both tuple buffers are exhausted,
then continue scan the input stream until the input stream is
exhausted too.

*/
Tuple* pjLocalInfo::getNextInputTuple(tupleBufferType tbt)
{
  Tuple* tuple = 0;

  if(itrA && tbt == tupBufferA){
    tuple = itrA->GetNextTuple();
  }
  else if (itrB){
    tuple = itrB->GetNextTuple();
  }

  return tuple;
}

/*
While the input stream is not exhausted,
keep asking the function to get one result.
If the function's output stream is exhausted,
then load the tuples of one bucket from the input stream.

*/
void* pjLocalInfo::getNextTuple()
{
  Word funResult(Address(0));

  while (!endOfStream)
  {
    qp->Request(JNfun, funResult);
    if (funResult.addr){
      return funResult.addr;
    }
    else if (endOfStream) {
      qp->Close(JNfun);
      return 0;
    }
    else {
      // No more result in current bucket, load the next bucket
      qp->Close(JNfun);
      loadTuples();
      if (isBufferFilled)
        qp->Open(JNfun);
      continue;
    }
  }
  return 0;
}


/*
6 Parajoin2

This is a modified version of *parajoin* operator.
The main difference of the new operator is that,
it accepts two separated sorted tuple stream,
and collect tuples have a same key attribute value,
then use the parameter join function to process them.

*/
struct paraJoin2Info : OperatorInfo
{
  paraJoin2Info()
  {
    name = "parajoin2";
    signature =
        "( (stream(tuple(T1))) x (stream(tuple(T2)))"
        "x (map (stream(T1)) (stream(T2)) (stream(T1 T2))) )"
        " -> stream(tuple(T1 T2))";
    syntax = "_ _ parajoin2 [ _, _ ; fun]";
    meaning = "use parameter join function to merge join two "
              "input sorted streams according to key values.";
  }
};


/*
Take another two sorted stream,
then use the parameter function to execute merge-join operation.

----
   ( (stream(tuple((a1 t1) (a2 t2) ... (ai ti) ... (am tm) )))
   x (stream(tuple((b1 p1) (b2 p2) ... (bj tj) ... (bn pn) ))))
   x ai x bj
   x ((map (stream((a1 t1) (a2 t2) ... (am tm) ))
           (stream((b1 p1) (b2 p2) ... (bn pn) ))
           (stream((a1 t1) (a2 t2) ... (am tm)
                   (b1 p1) (b2 p2) ... (bn pn)))))  )
   -> stream(tuple((a1 t1) (a2 t2) ... (am tm)
                   (b1 p1) (b2 p2) ... (bn pn)))
----

*/

ListExpr paraJoin2TypeMap(ListExpr args)
{
  if(nl->ListLength(args) == 5)
    {
      NList l(args);
      NList streamA = l.first();
      NList streamB = l.second();
      NList keyA = l.third();
      NList keyB = l.fourth();
      NList mapList = l.fifth();

      string err = "parajoin2 expects "
          "(stream(tuple(T1)) x stream(tuple(T2)) "
          "x string x string "
          "x (map (stream(T1)) (stream(T2)) (stream(T1 T2))))";
      string err1 = "parajoin2 can't found key attribute : ";

      NList attrA;
      if (!streamA.checkStreamTuple(attrA))
        return l.typeError(err);

      NList attrB;
      if (!streamB.checkStreamTuple(attrB))
        return l.typeError(err);

      ListExpr keyAType, keyBType;
      int keyAIndex = listutils::findAttribute(
                                 attrA.listExpr(),
                                 keyA.convertToString(),
                                 keyAType);
      if ( keyAIndex <= 0 )
        return l.typeError(err1 + keyA.convertToString());

      int keyBIndex = listutils::findAttribute(
                                 attrB.listExpr(),
                                 keyB.convertToString(),
                                 keyBType);
      if ( keyBIndex <= 0 )
        return l.typeError(err1 + keyB.convertToString());

      if (!nl->Equal(keyAType, keyBType))
        return l.typeError(
            "parajoin2 expects two key attributes with same type.");

      NList attrResult;
      if (mapList.first().isSymbol(Symbols::MAP())
          && mapList.second().first().isSymbol(Symbols::STREAM())
          && mapList.second().second().first().isSymbol(Symbols::TUPLE())
          && mapList.third().first().isSymbol(Symbols::STREAM())
          && mapList.third().second().first().isSymbol(Symbols::TUPLE())
          && mapList.fourth().checkStreamTuple(attrResult)  )
      {
        NList resultStream =
            NList(NList(STREAM, NList(NList(TUPLE), attrResult)));

        return NList(NList("APPEND"),
                     NList(NList(keyAIndex), NList(keyBIndex)),
                     resultStream).listExpr();
      }
      else
        return l.typeError(err);
    }
    else
    {
      ErrorReporter::ReportError(
        "Operator parajoin expect a list of five arguments");
      return nl->TypeError();
    }
}

int paraJoin2ValueMap(Word* args, Word& result,
                int message, Word& local, Supplier s)
{
  pj2LocalInfo* li=0;

  switch(message)
  {
    case OPEN:{
      qp->Open(args[0].addr);
      qp->Open(args[1].addr);

      if (li)
        delete li;
      li = new pj2LocalInfo(args[0], args[1],
                            args[5], args[6],
                            args[4], s);
      local.setAddr(li);
      return 0;
    }
    case REQUEST:{
      if (0 == local.addr)
        return CANCEL;
      li = (pj2LocalInfo*)local.addr;

      result.setAddr(li->getNextTuple());
      if (result.addr)
        return YIELD;
      else
        return CANCEL;
    }
    case (1*FUNMSG)+OPEN:{
      return 0;
    }
    case (2*FUNMSG)+OPEN:{
      return 0;
    }
    case (1*FUNMSG)+REQUEST:{
      if (0 == local.addr)
        return CANCEL;
      li = (pj2LocalInfo*)local.addr;

      result.setAddr(li->getNextInputTuple(tupBufferA));
      if (result.addr)
        return YIELD;
      else
        return CANCEL;
    }
    case (2*FUNMSG)+REQUEST:{
      if (0 == local.addr)
        return CANCEL;
      li = (pj2LocalInfo*)local.addr;

      result.setAddr(li->getNextInputTuple(tupBufferB));
      if (result.addr)
        return YIELD;
      else
        return CANCEL;
    }
    case (1*FUNMSG)+CLOSE:{
      return 0;
    }
    case (2*FUNMSG)+CLOSE:{
      return 0;
    }
    case CLOSE:{
      if (0 == local.addr)
        return CANCEL;
      li = (pj2LocalInfo*)local.addr;

      delete li;
      qp->Close(args[0].addr);
      qp->Close(args[1].addr);
      return 0;
    }
  }

  //should never be here
  return 0;
}

bool pj2LocalInfo::LoadTuples()
{
  bool loaded = false;

  //Clear the buffer
  if (ita)
    delete ita; ita = 0;
  if (tba)
    delete tba; tba = 0;

  if (itb)
    delete itb; itb = 0;
  if (tbb)
    delete tbb; tbb = 0;

  if (moreTuples)
  {
    if (cta == 0)
      cta.setTuple(NextTuple(streamA));
    if (ctb == 0)
      ctb.setTuple(NextTuple(streamB));
  }
  if ( cta == 0 || ctb == 0)
  {
    //one of the stream is exhausted
    endOfStream = true;
    return loaded;
  }

  int cmp = CompareTuples(cta.tuple, keyAIndex,
                          ctb.tuple, keyBIndex);

  // Assume both streams are ordered by asc
  while(0 != cmp)
  {
    if (cmp < 0)
    {
      //a < b, get more a until a >= b
      while (cmp < 0)
      {
        cta.setTuple(NextTuple(streamA));
        if ( cta == 0 )
        {
          endOfStream = true;
          return loaded;
        }
        cmp = CompareTuples(cta.tuple, keyAIndex,
                            ctb.tuple, keyBIndex);
      }
    }
    else if (cmp > 0)
    {
      //a > b, get more b until a <= b
      while (cmp > 0)
      {
        ctb.setTuple(NextTuple(streamB));
        if ( ctb == 0 )
          {
            endOfStream = true;
            return loaded;
          }
        cmp = CompareTuples(cta.tuple, keyAIndex,
                            ctb.tuple, keyBIndex);
      }
    }
  }

  //Take all tuples from streamA, until the next tuple is bigger
  //than the current one.
  tba = new TupleBuffer(maxMem);
  int cmpa = 0;
  RTuple tmpa;
  while ( (cta != 0) && (0 == cmpa) )
  {
    tmpa = cta;
    tba->AppendTuple(tmpa.tuple);
    cta.setTuple(NextTuple(streamA));
    if ( cta != 0 )
      cmpa = CompareTuples(tmpa.tuple, keyAIndex,
                         cta.tuple, keyAIndex);
  }

  tbb = new TupleBuffer(maxMem);
  int cmpb = 0;
  RTuple tmpb;
  while ( (ctb != 0) && (0 == cmpb) )
  {
    tmpb = ctb;
    tbb->AppendTuple(tmpb.tuple);
    ctb.setTuple(NextTuple(streamB));
    if ( ctb != 0 )
      cmpb = CompareTuples(tmpb.tuple, keyBIndex,
                         ctb.tuple, keyBIndex);
  }
  if ((cta == 0) || (ctb == 0))
    moreTuples = false;

  ita = tba->MakeScan();
  itb = tbb->MakeScan();
  loaded = true;

  if ((0 == tba->GetNoTuples()) || (0 == tbb->GetNoTuples()))
    endOfStream = true;
  return loaded;
}

int pj2LocalInfo::CompareTuples(Tuple* ta, int kai,
                                Tuple* tb, int kbi)
{
  Attribute* a = static_cast<Attribute*>(ta->GetAttribute(kai));
  Attribute* b = static_cast<Attribute*>(tb->GetAttribute(kbi));

  if (!a->IsDefined() || !b->IsDefined())
    return -1;

  int cmp = a->Compare(b);
  return cmp;
}

Tuple* pj2LocalInfo::getNextTuple()
{
  Word funResult(Address(0));

  while(!endOfStream)
  {
    qp->Request(pf, funResult);
    if (funResult.addr)
      return (Tuple*)funResult.addr;
    else if (endOfStream)
    {
      qp->Close(pf);
      return 0;
    }
    else
    {
      qp->Close(pf);
      if (LoadTuples())
        qp->Open(pf);
    }
  }

  //should never be here ...
  return 0;
}


/*
6 Operator ~add0Tuple~

The tuples outputed from ~doubleexport~ can't be used directly by
~parahashjoin~ or ~parajoin~, because the MapReduce job is needed
to sort these tuples according to their join attribute values,
and add those ~0Tuple~s to partition those tuples into different
buckets.

For simulating this proceduce in Secondo, we create this operator
called ~add0Tuple~.
This operator must get the outputs from ~doubleexport~, and be used
after a ~sortby~ operator which sort the tuples by their keys.
Then this operator can scan the whole stream, and add the ~0Tuple~s
when the keys values change.

At the same time, this operator also abandon the keyT field of
the input stream, only extract the valueT field to the next operator,
like ~parahashjoin~ or ~parajoin~.

Added in 21th July 2010 -- Jiamin
I changed the ~add0Tuple~ to keep the ~keyT~ attribute,
to reduce the additional overhead of creating new tuples.
Therefore, a ~project~ operator is needed to project the ~valueT~ part
only to the following ~parajoin~ or ~parahashjoin~ operator.

*/

struct add0TupleInfo : OperatorInfo
{
  add0TupleInfo()
  {
    name = "add0Tuple";
    signature =
        "(  (stream(tuple((keyT string)(valueT text))))"
        "-> stream(tuple((keyT string)(valueT text)))  )";
    syntax = "_ add0Tuple";
    meaning = "Separate tuples by inserting 0 tuples";
  }
};

/*
6.1 Type Mapping of Operator ~add0Tuple~

----
    (stream(tuple((keyT string)(valueT text))))
      -> stream(tuple((keyT string)(valueT text)))
----

*/
ListExpr add0TupleTypeMap(ListExpr args)
{
  int len = nl->ListLength(args);
  if (len != 1)
  {
    ErrorReporter::ReportError(
            "Operator add0TupleTypeMap only expect one argument.");
    return nl->TypeError();
  }

  ListExpr streamNL = nl->First(args);
  if (!listutils::isTupleStream(streamNL))
  {
    ErrorReporter::ReportError(
            "Operator add0TupleTypeMap expect a tuple stream.");
    return nl->TypeError();
  }

  ListExpr tupleList = nl->Second(nl->Second(streamNL));
  if (nl->ListLength(tupleList) == 2
  && listutils::isSymbol(nl->Second(nl->First(tupleList)), STRING)
  && listutils::isSymbol(nl->Second(nl->Second(tupleList)), TEXT))
  {
    return streamNL;
//    return nl->TwoElemList(nl->SymbolAtom(STREAM),
//          nl->TwoElemList(nl->SymbolAtom(TUPLE),
//              nl->OneElemList(nl->Second(tupleList))));
  }
  else
  {
    ErrorReporter::ReportError(
           "Operator add0TupleTypeMap expect input "
           "as stream(tuple((string)(text)))");
    return nl->TypeError();
  }
}


/*
6.2 Value Mapping of Operator ~add0Tuple~


*/
int add0TupleValueMap(Word* args, Word& result,
                      int message, Word& local, Supplier s)
{
  a0tLocalInfo *localInfo;
  Word cTupleWord;
  Tuple *oldTuple, *sepTuple;

  switch (message)
  {
  case OPEN:{
    qp->Open(args[0].addr);

    ListExpr resultTupleNL = GetTupleResultType(s);
    localInfo = new a0tLocalInfo(resultTupleNL);

    local.setAddr(localInfo);
    return 0;
  }
  case REQUEST:{
    if (local.addr == 0)
      return CANCEL;
    localInfo = (a0tLocalInfo*)local.addr;

    if(localInfo->needInsert)
    {
      // Output the cached tuple
      result.setAddr(localInfo->cachedTuple);

      localInfo->cachedTuple = 0;
      localInfo->needInsert = false;
      return YIELD;
    }
    else
    {
      qp->Request(args[0].addr, cTupleWord);
      if (qp->Received(args[0].addr))
      {
        oldTuple = (Tuple*)cTupleWord.addr;
        string key =
            ((CcString*)(oldTuple->GetAttribute(0)))->GetValue();

        if ("" == localInfo->key)
          localInfo->key = key;  //Set the initial key value

        if (key == localInfo->key)
        {
          result.setAddr(oldTuple);  //Unchanged key value
          return YIELD;
        }
        else
        {
          //  The key value changes,
          //  cache the current tuple with changed key,
          //  and insert the separate 0Tuple
          localInfo->cachedTuple = oldTuple;
          localInfo->needInsert = true;
          localInfo->key = key;

          sepTuple = new Tuple(localInfo->resultTupleType);
          sepTuple->PutAttribute(0, new CcString(true, "0Tuple"));
          sepTuple->PutAttribute(1, new FText(true, "(0 '')"));

          result.setAddr(sepTuple);
          return YIELD;
        }
      }
      else
        return CANCEL;
    }
  }
  case CLOSE:{
    if (local.addr == 0)
      return CANCEL;
    localInfo = (a0tLocalInfo*)local.addr;
    delete localInfo;
    local.setAddr(0);
    qp->Close(args[0].addr);
    return 0;
  }
  }
  return 0;
}

/*
5.15 Operator ~fconsume~

This operator maps

----   ( stream(tuple(...)) x string x string x [int]
             x [m\_Machine1] x [m\_Machine2]
             x [array(string) x int x int x int] )-> bool
----

Operator ~fconsume~ writes the accepted tuple-stream into a binary file,
and put the nested list of the input stream's schema
into a separate text file.
Totally it has three different modes:

  * Local mode
  * Type remote mode
  * Data remote mode

Local mode means ~fconsume~ writes both binary file and schema file
to local hard disk. Then the type remote mode means it writes both
files to local disk, and copy the schema file to at most two remote
machines' disks. At last the data remote mode means it not only
copies the schema file to remote machine, but also the binary files,
and if required, it also delete these two files on local disk.

This operator supports at most 9 arguments, the top three are necessary,
then the next three are optional. The end four arguments are
optional as a whole, i.e., if required, these four arguments are
asked as a whole.

The first three necessary arguments are the accepted tuple stream,
the name and the path of the output files. The file name must not
be empty, because it decides the output files' names. Assume the
file name is given as "FILE", then the exported schema file name
is FILE\_type. If the optional fourth argument ~index~ is given,
then the binary file name is FILE\_index.
The file path could be empty, and the files will be put into
the default path \$SECONDO\_BUILD\_DIR/bin/parallel/
if the given path is empty. In the contrast, the given path must be
an absolute Unix path if it is not empty.

The fourth argument ~index~ is optional, it gives an identifiable
postfix to the binary file. If it's not given, then the binary file
name is just the one we give as the second argument.

If only set at most above four arguments, then the operator is
used as local mode. And it changes to type remote mode if one of the
next two arguments is given.

The next two arguments are symbol type, and it must start with
prefix 'm\_' to indicate that they are the names of two machines.
These two parameters denotes two remote machines' names,
then ~fconsume~ copies the schema file to them.
It's also possible that only one argument is denoted.

For transporting files between different machines,
we use utility ~scp~ to copy files,
and it must doesn't need any password between these machines.

The last four arguments: ~Machines~, ~sI~, ~tI~ and ~dT~
are viewed as a whole, and are used to process data remote mode.
~Machines~ is a string array, each element is one remote machine's name,
which keeps non-password-required ssh connection with the local machine.
~sI~ means self index, is the array index of the local machine.
~tI~ means target index, is the array index the first target
machine where the operator will duplicate the binary file to.
The last argument ~dT~ means duplicate times, i.e. how many remote
machines will keep the replication of the binary file.
If ~dT~ is bigger than 1, then ~fconsume~ will not only copy the
binary file to the machine where ~tI~ point to in ~Machines~,
but also copy to (~dT~ - 1) remote machines after ~tI~.
If the duplication keeping machines don't contain the local one,
then the produced binary file will be removed after the replication.


5.15.0 Specification

*/

struct FConsumeInfo : OperatorInfo {

  FConsumeInfo() : OperatorInfo()
  {
    name =      "fconsume";
    signature = "stream(tuple(...)) x string x string "
        "x [int] x [m_machine1] x [m_machine2] "
        "x [ array(string) x int x int x int ] -> bool";
    syntax =    "_ fconsume[ _ , _ , _ , _ , _ , _, _, _, _ ]";
    meaning =   "Write a stream of tuples into a binary file, "
        "and may replicate to some remote machines."
        "The number of arguments are changed "
        "along with different mode, details about modes"
        "can be read in source code";
  }

};

/*
5.14.1 Type mapping

*/
ListExpr FConsumeTypeMap(ListExpr args)
{
  NList l(args);
  string err = "operator fconsume expects "
               "(stream(tuple(...)) string string "
               "[int] [m_Machine1] [m_Machine1] "
               " [ array(string) x int x int x int] )";

  int len = l.length();

  //If the data remote mode exist, then the first parameter
  //start from ~drmPos~.
  int drmPos = (len - 4);

  //First type mapping the operator without considering data remote
  if (drmPos > 2)
    len -= 4;

  if(len < 3 || len > 6)
    return l.typeError(err);

  NList attr;
  if(!l.first().checkStreamTuple(attr) )
    return l.typeError(err);

  if(!l.second().isSymbol(Symbols::STRING()) )
    return l.typeError(err);

  if(!l.third().isSymbol(Symbols::STRING()) )
    return l.typeError(err);

  //Optional Arguments ...
  string srvName[2] = {"", ""};
  bool haveIndex = false;
  if (len > 3)
  {
    if (l.fourth().isSymbol(Symbols::INT()))
      haveIndex = true;

    switch(len)
    {
      case 4:
        if (!haveIndex)
          srvName[0] = nl->SymbolValue(l.fourth().listExpr());
        break;
      case 5:
        if (haveIndex)
          srvName[0] = nl->SymbolValue(l.fifth().listExpr());
        else
        {
          srvName[0] = nl->SymbolValue(l.fourth().listExpr());
          srvName[1] = nl->SymbolValue(l.fifth().listExpr());
        }
        break;
      case 6:
        srvName[0] = nl->SymbolValue(l.fifth().listExpr());
        srvName[1] = nl->SymbolValue(l.sixth().listExpr());
        break;
      default:
        break;
    }

    for(int i = 0; i < 2; i++ )
    {
      if ("" != srvName[i])
      {
        if(srvName[i].compare(0, 2, "m_") > 0)
          return l.typeError(err);
        else
          srvName[i] = srvName[i].substr(2);
      }
    }
  }

  //Second consider the situation with data remote mode
  if( drmPos > 2 )
  {
    NList arrayList= l.elem(++drmPos);
    NList siList = l.elem(++drmPos);
    NList tiList = l.elem(++drmPos);
    NList dtList = l.elem(++drmPos);

    if (!(arrayList.first().isSymbol("array")
        && arrayList.second().isSymbol(Symbols::STRING())))
      return l.typeError(err);

    if (!( siList.isSymbol(Symbols::INT())
        && tiList.isSymbol(Symbols::INT())
        && dtList.isSymbol(Symbols::INT())))
      return l.typeError(err);
  }

  return NList(NList("APPEND"),
               NList(NList(haveIndex),
                     NList(srvName[0],true),
                     NList(srvName[1],true)),
               NList(Symbols::BOOL())).listExpr();
}


/*
5.14.2 Value mapping

*/
int FConsumeValueMap(Word* args, Word& result,
    int message, Word& local, Supplier s)
{
  fconsumeLocalInfo* fcli;

  if ( message <= CLOSE)
  {
    result = qp->ResultStorage(s);

    string relName, path;
    bool haveIndex = false;
    int index = -1;
    string machine[2] = {"",""};  //two remote sftp server name
    Array *machines = 0;
    int si = -1, ti = -1, dt = -1;
    bool drMode = false;

    relName = ((CcString*)args[1].addr)->GetValue();
    path = ((CcString*)args[2].addr)->GetValue();

    if("" == relName)
    {
      cerr << "Error: The file name doesn't exit! "<< endl;
      ((CcBool*)(result.addr))->Set(true, false);
      return 0;
    }
/*
The indexes of different parameters are fixed according to
the argument list length.

*/
    const int paraMap[12][8] = {
        { -1, -1, -1, -1, -1, -1, -1, -1 },  //(3)0
        {  4,  3, -1, -1, -1, -1, -1, -1 },  //(4T)1
        {  4, -1,  5, -1, -1, -1, -1, -1 },  //(4F)2
        {  5,  3,  6, -1, -1, -1, -1, -1 },  //(5T)3
        {  5, -1,  6,  7, -1, -1, -1, -1 },  //(5F)4
        { -1,  3,  7,  8, -1, -1, -1, -1 },  //(6)5
        { -1, -1, -1, -1,  3,  4,  5,  6 },  //(7)6
        {  8,  3, -1, -1,  4,  5,  6,  7 },  //(8T)7
        {  8, -1,  9, -1,  4,  5,  6,  7 },  //(8F)8
        {  9,  3, 10, -1,  5,  6,  7,  8 },  //(9T)9
        {  9, -1, 10, 11,  5,  6,  7,  8 },  //(9F)10
        { -1,  3, 11, 12,  6,  7,  8,  9 }   //(10)11
    };

    int len = qp->GetNoSons(s);
    int paraIndex;

    switch(len)
    {
      case 6: paraIndex = 0; break;
      case 7: paraIndex = 1; break;
      case 8: paraIndex = 3; break;
      case 9: paraIndex = 5; break;
      case 10: paraIndex = 6; break;
      case 11: paraIndex = 7; break;
      case 12: paraIndex = 9; break;
      case 13: paraIndex = 11; break;
    }

    //check ~haveIndex~
    if(paraMap[paraIndex][0] > 0)
    {
      haveIndex =
          ((CcBool*)args[paraMap[paraIndex][0]].addr)->GetValue();
      if (!haveIndex)
        paraIndex++;
    }

    //file index
    if(paraMap[paraIndex][1] > 0)
      index = ((CcInt*)args[paraMap[paraIndex][1]].addr)->GetValue();

    //schema machine name
    for(int i = 0; i < 2; i++)
    {
      if (paraMap[paraIndex][2 + i] > 0)
        machine[i] =((CcString*)args[
                     paraMap[paraIndex][2 + i]].addr)->GetValue();
    }

    //machine array
    if (paraMap[paraIndex][4] > 0)
    {
      drMode = true;
      machines = (Array*)args[paraMap[paraIndex][4]].addr;
      si = ((CcInt*)args[paraMap[paraIndex][5]].addr)->GetValue();
      ti = ((CcInt*)args[paraMap[paraIndex][6]].addr)->GetValue();
      dt = ((CcInt*)args[paraMap[paraIndex][7]].addr)->GetValue();
    }

    fcli = (fconsumeLocalInfo*) local.addr;
    if (fcli) delete fcli;

    fcli = new fconsumeLocalInfo();
    fcli->state = 0;
    fcli->current = 0;
    local.setAddr(fcli);

    //If path is not specified, then use the default path
    //\$SECONDO\_BUILD\_DIR/bin/parallel/
    //And the specified path must be an absolute path
    if ("" == path)
    {
      path = FileSystem::GetCurrentFolder();
      FileSystem::AppendItem(path, "parallel");
    }

    //Write the type of the relation into a separated file.
    string typeFileName = path;
    FileSystem::AppendItem(typeFileName, relName + "_type");
    ofstream typeFile(typeFileName.c_str());
    ListExpr relTypeList;
    if (!typeFile.good())
    {
      cerr << "Create typeInfo file"
          << relName + "_type" << " error!\n";
      ((CcBool*)(result.addr))->Set(true, false);
      return 0;
    }
    else
    {
      //The accepted input is a stream tuple
      relTypeList = nl->TwoElemList(
        nl->SymbolAtom("rel"),
        nl->Second(qp->GetSupplierTypeExpr(qp->GetSon(s,0))) );
      typeFile << nl->ToString(relTypeList) << endl;
      typeFile.close();
    }
    cout << "\nCreate type file: " << typeFileName << endl;

    //Put the schema file to other two machines
    for(int i = 0; i < 2; i++)
    {
      if ("" != machine[i])
      {
        system(("scp " + typeFileName + " "
            + machine[i] + rmDefaultPath).c_str());
      }
    }

    //Write complete tuples into a binary file.
    if (index >= 0)
    {
      stringstream ss;
      ss << relName << "_" << index;
      relName = ss.str();
    }

    //create a path for this file.
    string blockFileName = path;
    FileSystem::AppendItem(blockFileName, relName);
    ofstream blockFile(blockFileName.c_str(), ios::binary);
    if (!blockFile.good())
    {
      cerr << "Create file " << blockFileName << " error!" << endl;
      ((CcBool*)(result.addr))->Set(true, false);
      return 0;
    }

    //Statistic information
    TupleType *tt = new TupleType(SecondoSystem::GetCatalog()
                        ->NumericType(nl->Second(relTypeList)));
    vector<double> attrExtSize(tt->GetNoAttributes());
    vector<double> attrSize(tt->GetNoAttributes());
    double totalSize = 0.0;
    double totalExtSize = 0.0;
    int count = 0;

    Word wTuple(Address(0));
    qp->Open(args[0].addr);
    qp->Request(args[0].addr, wTuple);
    while(qp->Received(args[0].addr))
    {
      Tuple* t = static_cast<Tuple*>(wTuple.addr);

      size_t coreSize = 0;
      size_t extensionSize = 0;
      size_t flobSize = 0;
      size_t tupleBlockSize =
          t->GetBlockSize(coreSize, extensionSize, flobSize,
                          &attrExtSize, &attrSize);

      totalSize += (coreSize + extensionSize + flobSize);
      totalExtSize += (coreSize + extensionSize);

      char* tBlock = (char*)malloc(tupleBlockSize);
      t->WriteToBin(tBlock, coreSize, extensionSize, flobSize);
      blockFile.write(tBlock, tupleBlockSize);
      free(tBlock);
      count++;
      fcli->current++;

      t->DeleteIfAllowed();
      qp->Request(args[0].addr, wTuple);
    }

    // write a zero after all tuples to indicate the end.
    u_int32_t endMark = 0;
    blockFile.write((char*)&endMark, sizeof(endMark));

    // build a description list of output tuples
    NList descList;
    descList.append(NList(count));
    descList.append(NList(totalExtSize));
    descList.append(NList(totalSize));
    for(int i = 0; i < tt->GetNoAttributes(); i++)
    {
      descList.append(NList(attrExtSize[i]));
      descList.append(NList(attrSize[i]));
    }

    //put the base64 code of the description list to the file end.
    string descStr = binEncode(descList.listExpr());
    u_int32_t descSize = descStr.size() + 1;
    blockFile.write(descStr.c_str(), descSize);
    blockFile.write((char*)&descSize, sizeof(descSize));

    qp->Close(args[0].addr);
    blockFile.close();
    cout << "\nCreate block fileName: " << blockFileName << endl;

    if (drMode)
    {
      int aSize = machines->getSize();
      bool dupliMachines[aSize];
      bool keepLocal = false;
      memset(dupliMachines, 0, aSize);

      //Mark the machines need to be copied
      for(int i = 0; i < dt; i++)
        dupliMachines[((ti - 1 + i) % aSize)] = true;

      if (dupliMachines[(si -1)])
      {
        keepLocal = true;
        dupliMachines[(si -1)] = false;
      }

      for(int i = 0; i < aSize; i++)
      {
        if (dupliMachines[i])
          system(("scp " + blockFileName + " "
            + ((CcString*)(machines->getElement(i)).addr)->GetValue()
            + rmDefaultPath + relName ).c_str());
      }

      if (!keepLocal)
      {
        system(("rm " + blockFileName).c_str());
        cerr << "Local file '" + blockFileName
            + "' is removed" << endl;
      }

    }

    ((CcBool*)(result.addr))->Set(true, true);
    fcli->state = 1;
    return 0;

  }
  else if ( message == REQUESTPROGRESS )
  {
    ProgressInfo p1;
    ProgressInfo* pRes;
    const double uConsume = 0.024;   //millisecs per tuple
    const double vConsume = 0.0003;  //millisecs per byte in
                                        //  root/extension
    const double wConsume = 0.001338;  //millisecs per byte in FLOB

    fcli = (fconsumeLocalInfo*) local.addr;
    pRes = (ProgressInfo*) result.addr;

    if (qp->RequestProgress(args[0].addr, &p1))
    {
      pRes->Card = p1.Card;
      pRes->CopySizes(p1);

      pRes->Time = p1.Time + p1.Card *
            (uConsume + p1.SizeExt * vConsume
             + (p1.Size - p1.SizeExt) * wConsume);

      if ( fcli == 0 )
      {
        pRes->Progress = (p1.Progress * p1.Time) / pRes->Time;
      }
      else
      {
        if (fcli->state == 0)
        {
          if ( p1.BTime < 0.1 && pipelinedProgress ) //non-blocking,
                                                     //use pipelining
            pRes->Progress = p1.Progress;
          else
            pRes->Progress =
            (p1.Progress * p1.Time +
              fcli->current *  (uConsume + p1.SizeExt * vConsume
                  + (p1.Size - p1.SizeExt) * wConsume) )
                / pRes->Time;
        }
        else
        {
          pRes->Progress = 1.0;
        }
      }

      pRes->BTime = pRes->Time;    //completely blocking
      pRes->BProgress = pRes->Progress;

      return YIELD;      //successful
    }
    else
      return CANCEL;
  }
  else if ( message == CLOSEPROGRESS )
  {
    fcli = (fconsumeLocalInfo*) local.addr;
    if ( fcli ){
       delete fcli;
       local.setAddr(0);
    }
    return 0;
  }

  return 0;
}



const string FConsumeSpec  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" ) "
  "( <text>(stream(tuple(...)) x string x string x [int]"
  " x [m_Machine1] x [m_Machine2]"
  " x [array(string) x int x int x int] ) -> "
  " bool )</text--->"
  "<text>_ fconsume[ _ , _ , _ , _ , _ , _, _, _, _ ]</text--->"
  "<text>Write a stream of tuples into a binary file.</text--->"
  ") )";

Operator fconsumeOp (
    "fconsume",               // name
    FConsumeSpec,             // specification
    FConsumeValueMap,                 // value mapping
    Operator::SimpleSelect, // trivial selection function
    FConsumeTypeMap           // type mapping
);

/*
5.15 Operator ~ffeed~

This operator maps

----
string x string x [int] x [schemaMachine]
x [ array(string) x int x int]
-> rel(tuple(...))
----

This operator restore a relation from a binary file
created by ~fconsume~ operator.

The first two string arguments ~relName~ and ~path~ are indispensable.
~relName~ is used to define the name of the relation that we should read from.
It's composed by two parts, prefix ~f\_~ and ~name~.
The prefix is used to specify that the relation is read from a file.
~path~ could be empty, then the binary file must locate in default
path \$SECONDO\_BUILD\_DIR/. If it's not empty, then the ~path~ must
be an absolute path.

The third argument ~index~ defines a postfix of the binary file.
It's an optional argument, if it's not defined, then the binary file's
name is only ~name~. Or else, the file's name is ~name~ \_index.

The fourth argument ~schemaMachine~ defines a remote machine's
name which may contains the schema file of the relation.
It's also an optional argument, if it's not defined,
then the schema file must be put into the local default path
SECONDO\_BUILD\_DIR/bin/parallel/.
If ~schemaMachine~ is defined, then it must be started
with ~m\_~ prefix to state this is a symbol of a machine.
And it will be first use scp utility to copy the schma file from
the defined remote machine to local default path.

Besides reading binary file from local hard disk,
~ffeed~ also support reading data from another machine if these two
machines are linked by non-password-required ssh connection.
If want to get the relation from a remote machine, then following
three arguments must be given as a whole: ~Machines~, ~ti~ and ~att~.

The ~Machines~ is an Secondo array of strings, which contains the names
of all computers that the current node can access by ssh.
The ~ti~ means target index, it is used to denote which machine
in ~Machines~ contains the binary file.
The ~att~ means attempt times, if ~ffeed~ can't copy the binary file
from the machine which ~ti~ point to, then it will try to read the
file from the machine (~ti~ + 1), until it's totally tried ~att~ times.


5.15.0 Specification

*/

struct FFeedInfo : OperatorInfo {

  FFeedInfo() : OperatorInfo()
  {
    name =      "ffeed";
    signature = "(relName x path x [fileIndex] x [schemaMachine]"
                "x [ array(string) x int x int]"
                "-> stream(tuple(...)))";
    syntax =    "ffeed( _, _, _ , _, _, _, _)";
    meaning =   "restore a relation from a binary file"
                "created by ~fconsume~ operator.";
  }

};

/*
5.14.1 Type mapping

*/
ListExpr FFeedTypeMap(ListExpr args)
{
  NList l(args);
  string err = "ffeed expects(f_relName, string, [int], [m_machine]"
      "[(array string), int, int])";

  //operator only accept 2|3|6|7 arguments
  int len = l.length();

  int drmPos = (len - 3);

  if (drmPos > 1)
    len -= 3;

  if ( len < 2 || len > 4)
    return l.typeError(err);

  if (!(nl->IsAtom(l.first().listExpr())))
    return l.typeError(err);
  string relName = nl->SymbolValue(l.first().listExpr());
  if (relName.compare(0, 2, "f_") > 0)
    return l.typeError(err);
  relName = relName.substr(2);

  if (!l.second().isSymbol(Symbols::STRING()))
    return l.typeError(err);

  //The type file must be put into the default directory
  string typeFileName = FileSystem::GetCurrentFolder();
  FileSystem::AppendItem(typeFileName, "parallel");
  FileSystem::AppendItem(typeFileName, relName + "_type");

  bool haveIndex = false;
  int mPos = 3;  //Position of the remote schema machine argument
  if (len > 2)
  {
    if (l.third().isSymbol(Symbols::INT()))
    {
      haveIndex = true;
      mPos++;
    }

    if (mPos == len )
    {
      if(!l.elem(mPos).isAtom())
        return l.typeError(err);
      string nodeName = nl->SymbolValue(l.elem(mPos).listExpr());
      if (nodeName.compare(0, 2, "m_") > 0)
        return l.typeError(err);
      nodeName = nodeName.substr(2);

      //!!!we assume in every node's home directory, where the sftp
      //server start, exists a link to its \$SECONDO\_BUILD\_DIR !!!
      system(("scp "
          + nodeName + rmDefaultPath + relName + "_type" + " "
          + typeFileName).c_str());
    }
  }

  ListExpr relType;
  if(!nl->ReadFromFile(typeFileName, relType))
  {
    ErrorReporter::ReportError("Can't access file: " + typeFileName);
    return nl->TypeError();
  }

  if(!listutils::isRelDescription(relType))
  {
      ErrorReporter::ReportError("The nested list in file: "
          + typeFileName + " is not a tuple relation type.");
      return nl->TypeError();
  }

  if (drmPos > 1)
  {
    //remote mode
    drmPos++;
    if (!(l.elem(drmPos).first().isSymbol("array")
       && l.elem(drmPos).second().isSymbol(Symbols::STRING())))
      return l.typeError(err);

    if (!l.elem(++drmPos).isSymbol(Symbols::INT()))
      return l.typeError(err);

    if(!l.elem(++drmPos).isSymbol(Symbols::INT()))
      return l.typeError(err);
  }

  ListExpr streamType = nl->TwoElemList(
    nl->SymbolAtom("stream"),
    nl->Second(relType));

  return NList(NList("APPEND"),
               NList(NList(haveIndex),
                     NList(relName, true)),
               NList(streamType)).listExpr();
}

/*
5.14.2 Value mapping

*/
int FFeedValueMap(Word* args, Word& result,
    int message, Word& local, Supplier s)
{
  int index = -1;
  string relName, path;
  FFeedLocalInfo* ffli = 0;
  Supplier sonOfFeed;
  Array *machines = 0;
  int mIndex = -1;
  int attemptTime = 0;

  switch(message)
  {
    case OPEN: {
      path = ((CcString*)args[1].addr)->GetValue();
      int len = qp->GetNoSons(s);
      int pos = len;

      relName = ((CcString*)args[(--pos)].addr)->GetValue();
      bool haveIndex = ((CcBool*)args[(--pos)].addr)->GetValue();

      if (haveIndex)
        index = ((CcInt*)args[2].addr)->GetIntval();

      if ((pos - 4) > 0)
      {
        attemptTime = ((CcInt*)args[(--pos)].addr)->GetIntval();
        mIndex = ((CcInt*)args[(--pos)].addr)->GetIntval() - 1;
        machines = (Array*)args[(--pos)].addr;
      }

      if(index >= 0)
      {
        stringstream ss;
        ss << relName << "_" << index;
        relName = ss.str();
      }

      if (path == "")
      {
        path = FileSystem::GetCurrentFolder();
        FileSystem::AppendItem(path, "parallel");
        FileSystem::AppendItem(path, relName);
      }

      //Reuse the sftp connection
      ffli = (FFeedLocalInfo*) local.addr;
      if (ffli) delete ffli;
      ffli = new FFeedLocalInfo(qp->GetType(s));

      ffli->fetchBlockFile(
          relName, path, machines, mIndex, attemptTime);
      ffli->returned = 0;
      local.setAddr(ffli);
      return 0;
    }
    case REQUEST: {
      ffli = (FFeedLocalInfo*)local.addr;
      Tuple *t = ffli->getNextTuple();
      if (0 == t)
        return CANCEL;
      else
      {
        ffli->returned++;
        result.setAddr(t);
        return YIELD;
      }

    }
    case CLOSE: {
      ffli = (FFeedLocalInfo*)local.addr;
      if (ffli->tupleBlockFile){
        ffli->tupleBlockFile->close();
        delete ffli->tupleBlockFile;
        ffli->tupleBlockFile = 0;
      }
      return 0;  //must return
    }

    case CLOSEPROGRESS: {
      sonOfFeed = qp->GetSupplierSon(s, 0);
      ffli = (FFeedLocalInfo*) local.addr;
      if ( ffli )
      {
         delete ffli;
         local.setAddr(0);
      }
      return 0;
    }
    case REQUESTPROGRESS: {
      ProgressInfo p1;
      ProgressInfo *pRes = 0;
      const double uFeed = 0.00194;    //milliseconds per tuple
      const double vFeed = 0.0000196;  //milliseconds per Byte

      pRes = (ProgressInfo*) result.addr;
      ffli = (FFeedLocalInfo*) local.addr;

      if (ffli)
      {
        ffli->sizesChanged = false;

/*
This operator should always be the first operator of a tuple,
therefore it doesn't have any son operator.

*/
        pRes->Card = (double)ffli->total;
        pRes->CopySizes(ffli);
        pRes->Time =
            (ffli->total + 1) * (uFeed + ffli->SizeExt * vFeed);
        pRes->Progress =
            ffli->returned * (uFeed + ffli->SizeExt * vFeed)
            / pRes->Time;
        pRes->BTime = 0.001;
        pRes->BProgress = 1.0;

        return YIELD;
      }
      else
        return CANCEL;
    }
  }
  return 0;
}

const string FFeedSpec  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" ) "
  "( <text>(relName x path x [fileIndex] x [schemaMachine] "
  "x [ array(string) x int x int]) -> "
  "( stream(tuple(...)) )</text--->"
  "<text>ffeed( _, _, _, _, _, _, _)</text--->"
  "<text>restore a relation from a binary file "
  "created by ~fconsume~ operator.</text--->"
  ") )";

Operator ffeedOp (
    "ffeed",               // name
    FFeedSpec,             // specification
    FFeedValueMap,                 // value mapping
    Operator::SimpleSelect, // trivial selection function
    FFeedTypeMap           // type mapping
);

/*
3 Class ~HadoopParallelAlgebra~

A new subclass ~HadoopParallelAlgebra~ of class ~Algebra~ is declared.
The only specialization with respect to class ~Algebra~ takes place within
the constructor: all type constructors and operators are registered at the
actual algebra.

After declaring the new class, its only instance ~extendedRelationAlgebra~
is defined.

*/

class HadoopParallelAlgebra: public Algebra
{
public:
  HadoopParallelAlgebra() :
    Algebra()
  {

    AddOperator(doubleExportInfo(),
        doubleExportValueMap, doubleExportTypeMap);
    AddOperator(paraHashJoinInfo(),
        paraHashJoinValueMap, paraHashJoinTypeMap);

    AddOperator(paraJoinInfo(),
        paraJoinValueMap, paraJoinTypeMap);

    AddOperator(paraJoin2Info(),
        paraJoin2ValueMap, paraJoin2TypeMap);

    AddOperator(add0TupleInfo(),
        add0TupleValueMap, add0TupleTypeMap);


    AddOperator(TUPSTREAMInfo(), 0, TUPSTREAMType);
    AddOperator(TUPSTREAM2Info(), 0, TUPSTREAM2Type);
    AddOperator(TUPSTREAM3Info(), 0, TUPSTREAM3Type);

    AddOperator(&fconsumeOp);
    AddOperator(&ffeedOp);

#ifdef USE_PROGRESS
    fconsumeOp.EnableProgress();
    ffeedOp.EnableProgress();
#endif

  }
  ~HadoopParallelAlgebra()
  {
  }
  ;
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

extern "C" Algebra*
InitializeHadoopParallelAlgebra(
    NestedList* nlRef, QueryProcessor* qpRef)
{
  nl = nlRef;
  qp = qpRef;
  return (new HadoopParallelAlgebra());
}

/*
[newpage]

*/

