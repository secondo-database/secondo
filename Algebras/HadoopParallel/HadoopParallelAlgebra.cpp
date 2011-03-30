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
        "stream(tuple(a1 ... ai ... an)) "
        "x stream(tuple(b1 ... bj ... bm)) "
        "x ai x bj -> stream(tuple"
        "(key:string)(value:string))";
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
    newTuple->PutAttribute(0,new CcString(true, key));
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
        "-> stream(tuple((a1 t1) ... "
        "(an tn)(b1 p1) ... (bm pm)))";
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
        "from the first argument, "
        "and forward it as a stream";
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
  return nl->TwoElemList(nl->SymbolAtom(STREAM),
                         nl->Second(first));
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
  return nl->TwoElemList(nl->SymbolAtom(STREAM),
                         nl->Second(second));
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
        "from the third argument, "
        "and forward it as a stream";
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
  return nl->TwoElemList(nl->SymbolAtom(STREAM),
                         nl->Second(third));
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
        "x(map (stream(T1)) (stream(T2)) "
        "(stream(T1 T2))) ) -> stream(tuple(T1 T2))";
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
        "x (map (stream(T1)) (stream(T2)) "
        "(stream(T1 T2))) ) -> stream(tuple(T1 T2))";
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
   x (stream(tuple((b1 p1) (b2 p2) ... (bj tj) ... (bn pn) )))
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
      if (mapList.first().isSymbol(symbols::MAP)
          && mapList.second().first().isSymbol(symbols::STREAM)
          && mapList.second().
             second().first().isSymbol(symbols::TUPLE)
          && mapList.third().first().isSymbol(symbols::STREAM)
          && mapList.third().
             second().first().isSymbol(symbols::TUPLE)
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
        "((stream(tuple((keyT string)(valueT text))))"
        "-> stream(tuple((keyT string)(valueT text))) )";
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

----   ( stream(tuple(...)) x fileName x path x [fileIndex]
             x [typeNode1] x [typeNode2]
             x [array(string) x selfIndex
                x targetIndex x duplicateTimes] )-> bool
----

Operator ~fconsume~ exports the accepted tuple-stream into files.
The tuples are written into a binary file,
and the type list is written into a separate text file.
Totally it has three different modes:

  * Local mode
  * Type remote mode
  * Data remote mode

Local mode means ~fconsume~ writes both the binary file
and the type file to current node.
The type remote mode means besides writing both files to local disk,
the operator copies the type file to at most two specified remote nodes.
At last the data remote mode means both the type file
and the tuple file are copied into remote nodes,
and if required, delete the tuple file from the local node.

This operator supports at most 10 arguments, the top three are necessary,
then the next three are optional. And the end four arguments are
optional as a whole, i.e., if required, these four arguments must be
asked as a whole.

The first three necessary arguments are:

  * tuple stream
  * file name
  * file path

The file name must not be empty. If the file name is given as "FILE",
then the exported type file's name is FILE\_type.
And the binary tuple file's name is FILE.
The file path could be empty, and then the files are put into
the default path \$SECONDO\_BUILD\_DIR/bin/parallel/.
If it is not empty, the given path must be an absolute Unix path.

The next three optional arguments are:

  * file index
  * type node1
  * type node2

The fourth argument ~file index~ is optional,
it gives an identifiable postfix to the binary file.
If it's given, then the binary tuple file's name is FILE\_index.
The fifth and the sixth arguments denote two remote nodes' names.
If one of them is set, then the operator is changed to type remote mode. .

Note for transporting files between different machines,
we use utility ~scp~ to copy files, and the passwords of these nodes
should not be asked while coping files.

The last four arguments are:

  * machine array
  * self index
  * target index
  * duplicate times

These four arguments should be asked as a whole, and if they are set,
then the operator is changed to data remote mode.
The machine array is an array of strings,
each element is the name of a remote node which
keeps a non-password-required ssh connection with the current node.
The self index, is the local node's index inside the machine array.
The target index, is the array index of the first target node
to which the operator duplicates the binary file.
The last argument duplicate times indicates how many remote
nodes the binary file is copied to.
If it is bigger than 1, then the operator will not only copies the
binary file to the machine where the target index point to,
but also copies the file to next (~duplicate times~ - 1) remote machines.
If the nodes specified by ~target index~ and ~duplicate times~
don't contain the local node,
then the produced binary file will be removed after the replication.


5.15.0 Specification

*/

struct FConsumeInfo : OperatorInfo {

  FConsumeInfo() : OperatorInfo()
  {
    name =      "fconsume";
    signature = "stream(tuple(...)) x fileName x path "
        "x [fileIndex] x [typeNode1] x [typeNode2] "
        "x [ array(string) x selfIndex "
        "x targetIndex x dupTimes ] -> bool";
    syntax =    "_ fconsume[ _ , _ , _ , _ , _ , _, _, _, _ ]";
    meaning =   "Export a stream of tuples into a file, "
        "and may duplicate the file to some remote machines.\n"
        "The number of arguments are changed "
        "along with different mode.";
  }

};

/*
5.14.1 Type mapping

*/
ListExpr FConsumeTypeMap(ListExpr args)
{
  NList l(args);
  string lengthErr = "operator fconsume expects 3 ~ 10 arguments";
  string typeErr = "operator fconsume expects "
               "(stream(tuple(...)) string text "
               "[int] [string] [string] "
               " [ array(string) x int x int x int] )";
  string err1 = "The file name should NOT be empty!";
  string err2 = "ERROR!The file path doesn't exist. \n";
  string err3 = "ERROR!Infeasible evaluation in TM for attribute ";

  //all possible argument length: 3|4|5|6| 7|8|9|10
  int len = l.length();

  //If the data remote mode exists, then the array object
  //is the ~drmPos~ argument.
  int drmPos = (len - 4);

  //First type mapping the operator without considering data remote
  if (drmPos > 2)
    len -= 4;

  if (len < 3 || len > 6)
    return l.typeError(lengthErr);

  NList attr;
  if(!l.first().first().checkStreamTuple(attr) )
    return l.typeError(typeErr);

  if (!l.second().first().isSymbol(symbols::STRING) )
    return l.typeError(typeErr);
  NList fnList;
  if (!QueryProcessor::GetNLArgValueInTM(l.second().second(), fnList))
    return l.typeError(err3 + "fileName");
  string fileName = fnList.str();

  if (0 == fileName.length())
    return l.typeError(err1);

  if (!l.third().first().isSymbol(symbols::TEXT) )
    return l.typeError(typeErr);

  //Create the type file.
  //If path is not specified, then use the default path
  //\$SECONDO\_BUILD\_DIR/bin/parallel/ .
  //And the specified path must be an absolute path
  string typeFileName = fileName + "_type";
  NList fpList;
  if (!QueryProcessor::GetNLArgValueInTM(l.third().second(), fpList))
    return l.typeError(err3 + "filePath");
  string filePath = fpList.str();
  if (0 == filePath.length())
  {
    filePath = FileSystem::GetCurrentFolder();
    FileSystem::AppendItem(filePath, "parallel");
  }
  FileSystem::AppendItem(filePath, typeFileName);
  ofstream typeFile(filePath.c_str());
  NList resultList = NList(NList("rel"),
                           l.first().first().second());
  if (!typeFile.good())
  {
    return l.typeError(err2 + filePath);
  }
  else
  {
    typeFile << resultList.convertToString() << endl;
    typeFile.close();
    cerr << "Created type file " << filePath << endl;
  }

  // Optional Arguments ...
  bool haveIndex = false;
  if (len > 3)
  {
    int nodeArgLoc[2] = {-1 , -1 };
    if (l.fourth().first().isSymbol(symbols::INT))
      haveIndex = true;
    else if (l.fourth().first().isSymbol(symbols::STRING))
      nodeArgLoc[0] = 4;
    else
      return l.typeError(typeErr);

    switch(len)
    {
      case 4:
          break;
      case 5:
        if (l.fifth().first().isSymbol(symbols::STRING)){
          if (haveIndex)
            nodeArgLoc[0] = 5;
          else
            nodeArgLoc[1] = 5;
          break;
        }
      case 6:
        if ((l.fifth().first().isSymbol(symbols::STRING)
            && l.sixth().first().isSymbol(symbols::STRING))){
          nodeArgLoc[0] = 5;
          nodeArgLoc[1] = 6;
        break;
        }
      default:
        return l.typeError(typeErr);
    }

    //Copy the type file to remote nodes if asked
    for(int i = 0; i < 2; i++)
    {
      if (nodeArgLoc[i] > 0)
      {
        NList nnList;
        if (!QueryProcessor::GetNLArgValueInTM(
                l.elem(nodeArgLoc[i]).second(), nnList))
          return l.typeError(err3 + " for node " + int2string(i) );
        string nodeName = nnList.str();
        system(("scp " + filePath + " "
            + nodeName + rmDefaultPath).c_str());
      }
    }
  }

  //Second consider the situation with data remote mode
  if( drmPos > 2 )
  {
    NList arrayList= l.elem(++drmPos).first();
    NList siList = l.elem(++drmPos).first();
    NList tiList = l.elem(++drmPos).first();
    NList dtList = l.elem(++drmPos).first();

    if (!(arrayList.first().isSymbol("array")
        && arrayList.second().isSymbol(symbols::STRING)))
      return l.typeError(typeErr);

    if (!( siList.isSymbol(symbols::INT)
        && tiList.isSymbol(symbols::INT)
        && dtList.isSymbol(symbols::INT)))
      return l.typeError(typeErr);
  }

  NList appList;
  appList.append(NList(haveIndex));

  return NList(NList("APPEND"), appList,
               NList(symbols::BOOL)).listExpr();
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
    int fileIndex = -1;
//    string typeNode[2] = {"",""};  //two remote sftp server name
    Array *machines = 0;
    int si = -1, ti = -1, dt = -1;
    bool drMode = false;

    relName = ((CcString*)args[1].addr)->GetValue();
    path = ((FText*)args[2].addr)->GetValue();

    int len = qp->GetNoSons(s);
    drMode = (len > 8) ? true : false;  //data remote mode
    haveIndex = ((CcBool*) args[len - 1].addr)->GetValue();

    if (haveIndex)
      fileIndex = ((CcInt*) args[3].addr)->GetValue();


    //optional argument number (fileIndex and type nodes)
    //remove top three necessary arguments and the last haveIndex
    if (drMode)
    {
      int drmPos = len - 5;
      machines = (Array*)args[drmPos++].addr;
      si = ((CcInt*)args[drmPos++].addr)->GetValue();
      ti = ((CcInt*)args[drmPos++].addr)->GetValue();
      dt = ((CcInt*)args[drmPos].addr)->GetValue();
    }

    fcli = (fconsumeLocalInfo*) local.addr;
    if (fcli) delete fcli;

    fcli = new fconsumeLocalInfo();
    fcli->state = 0;
    fcli->current = 0;
    local.setAddr(fcli);

    //Write complete tuples into a binary file.
    if (fileIndex >= 0)
    {
      stringstream ss;
      ss << relName << "_" << fileIndex;
      relName = ss.str();
    }

    //create a path for this file.
    string blockFileName = path;
    if (0 == blockFileName.length())
    {
      blockFileName = FileSystem::GetCurrentFolder();
      FileSystem::AppendItem(blockFileName, "parallel");
    }
    FileSystem::AppendItem(blockFileName, relName);
    ofstream blockFile(blockFileName.c_str(), ios::binary);
    if (!blockFile.good())
    {
      cerr << "Create file " << blockFileName << " error!" << endl;
      ((CcBool*)(result.addr))->Set(true, false);
      return 0;
    }

    //Statistic information
    ListExpr relTypeList =
        qp->GetSupplierTypeExpr(qp->GetSon(s,0));
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
    cout << "\nCreated block fileName: " << blockFileName << endl;

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

      string sName =
      ((CcString*)(machines->getElement(si - 1)).addr)->GetValue();
      for(int i = 0; i < aSize; i++)
      {
        if (dupliMachines[i])
          system(("scp " + blockFileName + " " +
           ((CcString*)(machines->getElement(i)).addr)->GetValue()
           + rmDefaultPath + relName + "_" + sName ).c_str());
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
    const double wConsume = 0.001338;//millisecs per byte in FLOB

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
          if ( p1.BTime < 0.1 && pipelinedProgress )
            //non-blocking,
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

Operator fconsumeOp(FConsumeInfo(),
    FConsumeValueMap, FConsumeTypeMap);

/*
5.15 Operator ~ffeed~

This operator maps

----
fileName x path x [fileIndex] x [typeNode]
x [ machineArray x targetIndex x attemptTimes]
-> stream(tuple(...))
----

Operator ~ffeed~ restore a tuple stream from files created
by ~fconsume~ operator.

The first two string arguments ~fileName~ and ~path~ are indispensable.
~fileName~ defines the name of the relation we want to read from,
and it should NOT be empty.
Argument ~path~ defines where the files are.
If it is empty, then the files are assumed in the default path
\$SECONDO\_BUILD\_DIR/. Or else it must be an absolute Unix path.

The third argument ~fileIndex~ is optional,
it defines a postfix of the binary tuple file.
Assume the ~fileName~ is FILE, if the ~fileIndex~ is not defined,
then the binary file's name is FILE,
or else the file's name is FILE\_fileIndex.

The fourth argument ~typeNode~ defines a remote node's
name which contains the type file of the relation.
It's also an optional argument, and if it's not defined,
then the type file must be put into the local default path
SECONDO\_BUILD\_DIR/bin/parallel/.
If it is defined, then the operator first use scp utility to copy
the type file from the remote node to the local default path.

Besides reading binary file from local hard disk,
~ffeed~ also support reading the file from a remote machine if these two
machines are linked by a non-password-required ssh connection.
If so, the following three arguments  ~machineArray~,
~targetIndex~ and ~attemptTimes~ must be given as a whole.

The ~machineArray~ is a Secondo array of strings, which contains the names
of the remote machines that the current node can access to by
non-password-required ssh.
The ~targetIndex~ is used to denote which node in ~machineArray~
contains the binary tuple file.
The ~attemptTimes~ is used when the ~ffeed~ can't copy the binary file
from the node which ~targetIndex~ point to, then it tries to read the
file from the next node (~targetIndex~ + 1),
until the following ~attemptTimes~ nodes are all tried.


5.15.0 Specification

*/

struct FFeedInfo : OperatorInfo {

  FFeedInfo() : OperatorInfo()
  {
    name =      "ffeed";
    signature = "fileName x path x [fileIndex] x [typeNode]"
                "x [ machineArray x targetIndex x attemptTimes]"
                "-> stream(tuple(...))";
    syntax =    "ffeed( _, _, _ , _, _, _, _)";
    meaning =   "restore a relation from a binary file"
                "created by ~fconsume~ operator.";
  }
};

/*
5.15.1 Type mapping

*/

ListExpr FFeedTypeMap(ListExpr args)
{
  NList l(args);
  string typeErr = "ffeed expects(string, text, [int], [string]"
      "[(array string), int, int])";
  string err1 = "ERROR! File name should NOT be empty!";
  string err2 = "ERROR! Type file NOT exist!\n";
  string err3 = "ERROR! A tuple relation type list is "
      "NOT contained in file: ";
  string err4 ="ERROR! Infeasible evaluation in TM for attribute ";

  //operator only accept 2|3|4|5|6|7 arguments
  int len = l.length();

  if ( len < 2 || len > 7)
    return l.typeError(typeErr);

  bool drMode = (len > 4) ? true : false;  //data remote mode

  if (!l.first().first().isSymbol(symbols::STRING))
    return l.typeError(typeErr);
  NList fnList;
  if (!QueryProcessor::GetNLArgValueInTM(l.first().second(), fnList))
    return l.typeError(err4 + "fileName");
  string fileName = fnList.str();

  if (0 == fileName.length())
    return l.typeError(err1);

  //only for test the getNLArgValueInTM
  if (!l.second().first().isSymbol(symbols::TEXT))
    return l.typeError(typeErr);
  NList fpList;
  if (!QueryProcessor::GetNLArgValueInTM(l.second().second(), fpList))
    return l.typeError(err4 + "filePath");
  string filePath = fpList.str();

  if (0 == filePath.length())
  {
    filePath = FileSystem::GetCurrentFolder();
    FileSystem::AppendItem(filePath, "parallel");
  }
  FileSystem::AppendItem(filePath, fileName + "_type");

  //Check the file index and the remote type node
  int argNum = drMode ? (len - 3) : len;
  bool haveIndex = false;
  int rtnPos = -1;  //RTN: remote type node
  switch(argNum)
  {
    case 2:
      break;
    case 3:{
      if (l.third().first().isSymbol(symbols::INT))
        haveIndex = true;
      else if (l.third().first().isSymbol(symbols::STRING))
        rtnPos = 3;
      else
        return l.typeError(typeErr);
      break;
    }
    case 4:{
      if (!(l.third().first().isSymbol(symbols::INT)
          && l.fourth().first().isSymbol(symbols::STRING)))
        return l.typeError(typeErr);
      haveIndex = true;
      rtnPos = 4;
      break;
    }
    default:
      return l.typeError(typeErr);
  }
  if (rtnPos > 0)
  {
    //copy the type file from the remote machine
    NList nnList;
    if (!QueryProcessor::GetNLArgValueInTM(l.elem(rtnPos).second(), nnList))
      return l.typeError(err3 + "node name");
    string nodeName = nnList.str();

    //!!!we assume in every node's home directory, where the sftp
    //server start, exists a link to its \$SECONDO\_BUILD\_DIR !!!
    system(("scp "
        + nodeName + rmDefaultPath + fileName + "_type" + " "
        + filePath).c_str());
  }
  ListExpr relType;
  if(!nl->ReadFromFile(filePath, relType))
    return l.typeError(err2 + filePath);
  if(!listutils::isRelDescription(relType))
    return l.typeError(err3 + filePath);
  if (drMode)
  {
    //remote mode
    int drmPos = len - 3 + 1;  //Three arguments from the end
    if (!(l.elem(drmPos).first().first().isSymbol("array")
        && l.elem(drmPos).first().
           second().isSymbol(symbols::STRING)))
      return l.typeError(typeErr);
    if (!l.elem(++drmPos).first().isSymbol(symbols::INT))
      return l.typeError(typeErr);
    if(!l.elem(++drmPos).first().isSymbol(symbols::INT))
      return l.typeError(typeErr);
  }
  NList streamType =
      NList(NList(symbols::STREAM),
      NList(NList(relType).second()));

  return NList(NList("APPEND"),
               NList(NList(haveIndex),
                     NList(fileName, true)),
               streamType).listExpr();
}

/*
5.15.2 Value mapping

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
      path = ((FText*)args[1].addr)->GetValue();
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

      string filePath = path;
      if (0 == path.length())
      {
        filePath = FileSystem::GetCurrentFolder();
        FileSystem::AppendItem(filePath, "parallel");
      }
      FileSystem::AppendItem(filePath, relName);

      ffli = (FFeedLocalInfo*) local.addr;
      if (ffli) delete ffli;
      ffli = new FFeedLocalInfo(qp->GetType(s));

      ffli->fetchBlockFile(
          relName, filePath, machines, mIndex, attemptTime);
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


Operator ffeedOp(FFeedInfo(), FFeedValueMap, FFeedTypeMap);

/*
5.16 Operator ~hadoopjoin~

This operator carries out a Hadoop join operation in Secondo.

*/

struct HdpJoinInfo : OperatorInfo {

  HdpJoinInfo() : OperatorInfo(){
    name = "hadoopjoin";
    signature =
        "mq1Stream x mq2Stream x machineArray x masterIndex"
        "x reduceNum x resultName x rqMap"
        "-> stream(tuple((mIndex int)(pIndex int)))";
    syntax = "_ _ hadoopjoin[_, _, _, _, _, _; fun]";
    meaning =
        "Evaluating parallel join operation on Secondo platform "
        "deployed in a cluster, "
        "by calling a generic Hadoop join program";
  }
};

/*
5.16.1 Type mapping

The operator maps:

----
   ( (stream(tuple(T1))) x (stream(tuple(T2)))
    x array(string) x int x int x string
    x (map stream(tuple(T1)) stream(tuple(T2))
           stream(tuple(T1 T2))))
    -> stream(tuple((mIndex int)(pIndex int)))
----

This operator evaluates the parallel join operation
in Secondo by calling a generic Hadoop join program.
The operator only works in the Secondo system
which is deployed in a cluster which has a Hadoop system,
and the Secondo Monitors on all nodes that belong to the cluster
have been started already.
The results of the operation are distributed in nodes as files,
with argument ~resultName~ as file name.
And the operator outputs a tuple stream to indicate these files' places.
The tuple stream contains two fields: mIndex and pIndex.
The mIndex denotes which node have the result file,
and the pIndex denotes which part of the complete result is inside
the file.

The operator contains 7 parameters in total:
mq1Stream, mq2Stream, machineArr, masterIndex, rtNum,
resultName and rqMap.

The mq1Stream, mq2Stream and rqMap are Secondo queries.
By using the feature of ~SetUsesArgsInTypeMapping~,
we can get the nested list of these queries, and send them to
Hadoop program as arguments.
Then these queries are merged with some fixed nested list type
queries written in the Hadoop program already, and are sent to multiple
remote Secondo monitors to run.

The machineArr is an array object that is kept
in all Secondo databases of the cluster's nodes.
This array contains the complete list of the nodes' names in the
cluster, and the parameter masterIndex which is also kept in
all Secondo databases, indicates which node in the array is the
master node.

The parameter rtNum is used to define how many reduce tasks we want
to use in the Hadoop job. The number of the map tasks are defined
by the amount of slave nodes in the machineArr.

*/

ListExpr hdpJoinTypeMap(ListExpr args)
{
  NList l(args);
  int len = l.length();

  if (len == 7)
  {
    string typeErr = "operator hadoopjoin expects "
        "stream(tuple(T1)), stream(tuple(T2)), "
        "(array(string), int, int, "
        "(map (stream(tuple(T1))) stream(tuple(T2)) "
        "  stream(tuple(T1 T2))))";
    string err2 = "operator hadoopjoin ecpects "
        "both input streams contain attribute Partition";
    string err3 =
        "ERROR! Infeasible evaluation in TM for attribute ";

    string streamStr[2] = {"", ""};
    for (int argIndex = 1; argIndex <= 2; argIndex++)
    {
      NList mapStream = l.elem(argIndex).first();
      NList attr;
      if (!mapStream.checkStreamTuple(attr))
        return l.typeError(typeErr);
      streamStr[argIndex - 1] =
          l.elem(argIndex).second().convertToString();
    }

    if (!(l.third().first().first().isSymbol("array")
        && l.third().first().second().isSymbol(symbols::STRING)))
      return l.typeError(typeErr);

    if (!l.fourth().first().isSymbol(symbols::INT))
      return l.typeError(typeErr);

    if (!l.fifth().first().isSymbol(symbols::INT))
      return l.typeError(typeErr);

    if (!l.sixth().first().isSymbol(symbols::STRING))
      return l.typeError(typeErr);
    NList rnList;
    if (!QueryProcessor::GetNLArgValueInTM(l.sixth().second(), rnList))
      return l.typeError(err3 + "resultName");
    string resultName = rnList.str();

    string mapStr = l.elem(7).second().fourth().convertToString();
    NList mapList = l.elem(7).first();
    NList attrAB, attr[2];
    if (mapList.first().isSymbol(symbols::MAP)
        && mapList.second().checkStreamTuple(attr[0])
        && mapList.third().checkStreamTuple(attr[1])
        && mapList.fourth().checkStreamTuple(attrAB))
    {
      //Confirmation of Cell and Partition attributes
      bool haveCP[2] = {false, false};
      for (int i = 0; i < 2; i++){
        if (!IsTupleDescription(attr[i].listExpr()))
          return l.typeError(typeErr);
        NList rest = attr[i];
        while (!rest.isEmpty()){
          NList a = rest.first();
          rest.rest();
          if (a.second().isSymbol("int")){
            string aName = a.first().convertToString();
            if ("Partition" ==
                  aName.substr(0,aName.find_first_of("_"))){
                haveCP[i] = true;
            }
          }
        }
      }
      if (!(haveCP[0] && haveCP[1]))
        return l.typeError(err2);

      // Write the join result type into local default path,
      // in case the following operators need.
      NList joinResult = NList(NList(REL),
                       NList(NList(TUPLE), NList(attrAB)));
      string typeFileName = FileSystem::GetCurrentFolder();
      FileSystem::AppendItem(typeFileName, "parallel");
      FileSystem::AppendItem(typeFileName, resultName + "_type");
      ofstream typeFile(typeFileName.c_str());
      if (!typeFile.good())
      {
        cerr << "Create typeInfo file Result_type "
            "in default parallel path error!" << endl;
      }
      else
      {
        //The accepted input is a stream tuple
        typeFile << joinResult.convertToString() << endl;
        typeFile.close();
      }
      cerr << "\nSuccess created type file: "
          << typeFileName << endl;

      // result type
      NList a1(NList("mIndex"), NList(symbols::INT));
      NList a2(NList("pIndex"), NList(symbols::INT));

      NList result(NList(STREAM),
                   NList(NList(TUPLE), NList(a1, a2)));

      return NList(NList("APPEND"),
                   NList(NList(streamStr[0], true, true),
                         NList(streamStr[1], true, true),
                         NList(mapStr, true, true)),
                   result).listExpr();
    }
    else
      return l.typeError(typeErr);
  }
  else
    return l.typeError("Operator hadoop expects a list "
        "of nine arguments");
}

/*
5.16.2 Value mapping

Operator takes the first array parameter,
filter out the master node by the second masterIndex parameter,
and gives a nodes name list separated by slash to the hadoop program.
If the masterIndex is less than 1, all nodes in the array
are involved in the operation.

Besides, take the query strings together with the number of reduce
tasks, and also send them to the hadoop program as arguments,

*/

int hdpJoinValueMap(Word* args, Word& result,
    int message, Word& local, Supplier s)
{
  hdpJoinLocalInfo* hjli = 0;

  switch(message)
  {
    case OPEN:{
      //0 take the parameters
      Array *machines = 0;
      int masterIndex = -1;

      //0.1 make up the nodes list and the master node
      machines = (Array*)args[2].addr;
      masterIndex = ((CcInt*)args[3].addr)->GetIntval();
      int maSize = machines->getSize();
      string masterName;
      stringstream slSS, siSS;
      for (int i = 1; i <= maSize; i++)
      {
        string nodeName =
            ((CcString*)machines->
                getElement(i - 1).addr)->GetValue();
        if (masterIndex == i)
          masterName = nodeName;
        else{
          slSS << nodeName << "/";
          siSS << i << "/";
        }
      }
      string slaveList = slSS.str();
      slaveList = slaveList.substr(0, slaveList.size() - 1);
      string slaveIndexList = siSS.str();
      slaveIndexList =
          slaveIndexList.substr(0, slaveIndexList.size() - 1);

      //0.2 assume the operation happens on
      //all nodes' Secondo databases with a same name
      string dbName =
          SecondoSystem::GetInstance()->GetDatabaseName();

      //0.3 get other arguments
      int rtNum = ((CcInt*)args[4].addr)->GetIntval();
      string rName = ((CcString*)args[5].addr)->GetValue();
      string mrQuery[3] = {
          ((FText*)args[7].addr)->GetValue(),
          ((FText*)args[8].addr)->GetValue(),
          ((FText*)args[9].addr)->GetValue(),
      };

      //1 evaluate the hadoop program
      stringstream queryStr;
      queryStr << "hadoop jar HdpSec.jar dna.HSJoin \\\n"
        << "\"" << masterName << "\" "
        << masterIndex << " "
        << "\"" << slaveList << "\" "
        << "\"" << slaveIndexList << "\" "
        << dbName << " \\\n"
        << "\"" << tranStr(mrQuery[0], "\"", "\\\"") << "\" \\\n"
        << "\"" << tranStr(mrQuery[1], "\"", "\\\"") << "\" \\\n"
        << "\"" << tranStr(mrQuery[2], "\"", "\\\"") << "\" \\\n"
        << rtNum << " " << rName << endl;
      int rtn;
//      cout << queryStr.str() << endl;
      rtn = system("hadoop dfs -rmr OUTPUT");
      rtn = system(queryStr.str().c_str());

      //2 get the result file list
      if (hjli)
        delete hjli;
      hjli = new hdpJoinLocalInfo(s);

      FILE *fs;
      char buf[MAX_STRINGSIZE];
//      fs = popen("cat pjResult", "r");
      fs = popen("hadoop dfs -cat OUTPUT/part*", "r");

      while(fgets(buf, sizeof(buf), fs))
      {
        stringstream ss;
        ss << buf;
        istringstream iss(ss.str());
        int mIndex, pIndex;
        iss >> pIndex >> mIndex;
        hjli->insertPair(make_pair(mIndex, pIndex));
      }
      pclose(fs);
      hjli->setIterator();
      local.setAddr(hjli);

      return 0;
    }
    case REQUEST:{
      if (0 == local.addr)
        return CANCEL;
      hjli = (hdpJoinLocalInfo*)local.addr;
      result.setAddr(hjli->getTuple());

      if (result.addr)
        return YIELD;
      else
        return CANCEL;
    }
    case CLOSE:{
      if (0 == local.addr);
        return CANCEL;
      hjli = (hdpJoinLocalInfo*)local.addr;
      delete hjli;

      return 0;
    }
  }

  //should never be here
  return 0;
}

Operator hadoopjoinOp(HdpJoinInfo(),
                      hdpJoinValueMap,
                      hdpJoinTypeMap);

/*
5.16 Operator ~fdistribute~

~fdistribute~ partitions a tuple stream into several binary files
based on a specific attribute value, along with a linear scan.
These files could be read by ~ffeed~ operator.
This operator is used to replace the expensive ~groupby~ + ~fconsume~ operations,
which need sort the tuple stream first.

The operator accepts at least 4 parameters:
a tuple stream, files' base name, files' path and keyAttributeName.
The first three are same as ~fconsume~ operator,
the fourth parameter defines the key attribute
by whose hash value tuples are partitioned.

If the fifth parameter nBuckets is given, then tuples are
evenly partitioned to buckets based on modulo function,
or else these tuples are partitioned based on
keyAttribute values' hash numbers directly,
which may partitions these tuples NOT evenly.

In both even or uneven partition mode,
the key attribute will be removed from the export tuple files,
just like what ~distribute~ operator does.

5.16.0 Specification

*/

struct FDistributeInfo : OperatorInfo {

  FDistributeInfo() : OperatorInfo()
  {
    name = "fdistribute";
    signature =
        "stream(tuple(...)) x fileName x path "
        "x attrName x [nBuckets]"
        "-> stream(tuple(fileSufix, value))";
    syntax = "_ fconsume[ _ , _ , _ , {_} ]";
    meaning =   "Export a stream of tuples into files that can "
        "be read by ffeed operator. "
        "Files are separated by their suffix index, "
        "which is a hash value based on the given attribute value"
        "and number of set buckets. "
        "If the parameter nBuckets is not given, "
        "tuples may uneven partitioned.";
  }

};

/*
5.16.1 Type mapping

*/
ListExpr FDistributeTypeMap(ListExpr args)
{
  NList l(args);
  string lenErr = "operator fdistribute expects 4/5 arguments.";
  string tpeErr = "operator fdistribute expects "
      "(stream(tuple)) x string x text x symbol x [int]";
  string attErr = "operator fdistribute cannot find the "
      "basis partition attribute: ";
  string err1 = "ERROR!Infeasible evaluation in TM for attribute ";
  string err2 = "The file name should NOT be empty!";
  string err3 = "Fail by openning file: ";

  int len = l.length();
  bool evenMode = true;
  if (4 == len)
    evenMode = false;
  else if (5 != len)
    return l.typeError(lenErr);

  NList attrsList;
  if (!l.first().first().checkStreamTuple(attrsList))
    return l.typeError(tpeErr);

  if (!l.second().first().isSymbol(symbols::STRING))
    return l.typeError(tpeErr);
  NList fnList; //get the file name
  if (!QueryProcessor::GetNLArgValueInTM(l.second().second(), fnList))
    return l.typeError(err1 + "fileName");
  string fileName = fnList.str();
  if (0 == fileName.length())
    return l.typeError(err2);

  if (!l.third().first().isSymbol(symbols::TEXT))
    return l.typeError(tpeErr);

  //Identify attribute
  if (!l.fourth().first().isAtom())
    return l.typeError(tpeErr);
  string attrName = l.fourth().second().str();
  ListExpr attrType;
  int attrIndex = listutils::findAttribute(
      attrsList.listExpr(), attrName, attrType);
  if (attrIndex < 1)
    return l.typeError(attErr + attrName);

  if (evenMode)
    if (!l.fifth().first().isSymbol(symbols::INT))
      return l.typeError(tpeErr);

  //Remove the attribute used for partition the relation
  NList newAL; //new attribute list
  NList rest = attrsList;
  while(!rest.isEmpty())
  {
    NList elem = rest.first();
    rest.rest();

    if (elem.first().str() != attrName)
      newAL.append(elem);
  }

  //Create the type file in local disk
  string typeFileName = fileName + "_type";
  NList fpList;
  if (!QueryProcessor::GetNLArgValueInTM(l.third().second(), fpList))
    return l.typeError(err1 + "filePath");
  string filePath = fpList.str();
  filePath = getFilePath(filePath, typeFileName);
  ofstream typeFile(filePath.c_str());
  NList resultList =
          NList(NList(REL), NList(NList(TUPLE), newAL));
  if (!typeFile.good())
    return l.typeError(err3 + filePath);
  else
  {
    typeFile << resultList.convertToString() << endl;
    cerr << "Created type file " << filePath << endl;
  }
  typeFile.close();

  NList outAttrList =
           NList(NList(NList("suffix"), NList(INT)),
                 NList(NList("tupNum"), NList(INT)));
  NList outList = NList().tupleStreamOf(outAttrList);

  return NList(NList("APPEND"),
               NList(
                 NList(attrIndex),
                 NList(
                   NList(NList(TUPLE), newAL).convertToString(),
                     true, true)),
               outList).listExpr();
}

/*
5.16.2 Value mapping

*/
int FDistributeValueMap(Word* args, Word& result,
    int message, Word& local, Supplier s)
{
  string relName, path;
  FDistributeLocalInfo* fdli = 0;
  Word elem;

  switch(message)
  {
    case OPEN: {
      SecondoCatalog* sc = SecondoSystem::GetCatalog();
      qp->Open(args[0].addr);

      relName = ((CcString*)args[1].addr)->GetValue();
      path = ((FText*)args[2].addr)->GetValue();

      int pos = 5;
      bool evenMode = true;
      if (qp->GetNoSons(s) < 7){
        pos--;
        evenMode = false;
      }
      int nBucket = 0;
      if (evenMode)
        nBucket = ((CcInt*)args[4].addr)->GetValue();
      int attrIndex =
            ((CcInt*)args[pos++].addr)->GetValue() - 1;
      string inTupleTypeStr =
               ((FText*)args[pos].addr)->GetValue();

      ListExpr inTupleTypeList;
      nl->ReadFromString(inTupleTypeStr, inTupleTypeList);
      inTupleTypeList = sc->NumericType(inTupleTypeList);

      ListExpr resultTupleList = GetTupleResultType(s);

      fdli = (FDistributeLocalInfo*) local.addr;
      if (fdli) delete fdli;
      fdli = new FDistributeLocalInfo(
               relName, path, nBucket, attrIndex,
               resultTupleList, inTupleTypeList);
      local.setAddr(fdli);

      //Write tuples to files completely
      qp->Open(args[0].addr);
      qp->Request(args[0].addr, elem);
      while(qp->Received(args[0].addr))
      {
        if (!fdli->insertTuple(elem))
          break;

        qp->Request(args[0].addr, elem);
      }
      qp->Close(args[0].addr);
      fdli->startCloseFiles();
      return 0;
    }
    case REQUEST: {
      fdli = static_cast<FDistributeLocalInfo*>(local.addr);
      if (!fdli)
        return CANCEL;

      //Return the counters of each file
      Tuple* tuple = fdli->closeOneFile();
      if (tuple)
      {
        result.setAddr(tuple);
        return YIELD;
      }
      return CANCEL;
    }
    case CLOSE: {
      fdli = static_cast<FDistributeLocalInfo*>(local.addr);
      if (fdli)
        delete fdli;
      local.addr = 0;
      return 0;
    }
  }
  return 0;
}

FDistributeLocalInfo::FDistributeLocalInfo(
    string _bn, string _pt, int _nb, int _ai,
    ListExpr _rtl, ListExpr _itl)
: nBuckets(_nb), attrIndex(_ai),
  tupleCounter(0)
{
  filePath = getFilePath(_pt, _bn);
  resultTupleType = new TupleType(nl->Second(_rtl));
  exportTupleType = new TupleType(_itl);
}

bool FDistributeLocalInfo::insertTuple(Word tupleWord)
{
  Tuple *tuple = static_cast<Tuple*>(tupleWord.addr);
  size_t fileSfx = HashTuple(tuple);
  bool ok = true;

  map<size_t, fileInfo*>::iterator mit;
  mit = fileList.find(fileSfx);
  fileInfo* fp;
  if (mit != fileList.end())
    fp = (*mit).second;
  else
  {
    fp = new fileInfo(fileSfx, filePath,
        exportTupleType->GetNoAttributes());
    fileList.insert(pair<size_t, fileInfo*>(fileSfx, fp));
  }
  ok = openFile(fp);

  if (!(ok &&
        fp->writeTuple(tuple, tupleCounter,
                       attrIndex, exportTupleType)))
    cerr << "Block File " << fp->getFileName() << " Write Fail.\n";
  tupleCounter++;
  tuple->DeleteIfAllowed();

  return ok;
}

bool FDistributeLocalInfo::openFile(fileInfo* tgtFile)
{
  if (tgtFile->isFileOpen())
    return true;

  if (openFileList.size() >= MAX_FILEHANDLENUM)
  {
    //sort fileInfos according to their last tuples' indices
    sort(openFileList.begin(), openFileList.end(), compFileInfo);
    //The last one of the vector is the idler
    bool poped = false;
    while(!poped)
    {
      if (openFileList.back()->isFileOpen())
      {
        openFileList.back()->closeFile();
        poped = true;
      }
      openFileList.pop_back();
    }
  }

  bool ok = tgtFile->openFile();
  openFileList.push_back(tgtFile);
  return ok;
}

void FDistributeLocalInfo::startCloseFiles()
{
  fit = fileList.begin();
}

Tuple* FDistributeLocalInfo::closeOneFile()
{
  Tuple* tuple = 0;
  if (fit != fileList.end())
  {
    int suffix = (*fit).first;
    fileInfo* fp = (*fit).second;
    bool ok = openFile(fp);

    if ( ok )
    {
      int count = fp->writeLastDscr();
      fp->closeFile();
      tuple = new Tuple(resultTupleType);
      tuple->PutAttribute(0, new CcInt(suffix));
      tuple->PutAttribute(1, new CcInt(count));
    }
    fit++;
  }
  return tuple;
}

Operator fdistributeOp(FDistributeInfo(),
                       FDistributeValueMap,
                       FDistributeTypeMap);

/*
6 Auxiliary functions

*/
string tranStr(const string& s,
                 const string& from, const string& to)
{
  string result = s;

  size_t fLen = from.length();
  size_t tLen = to.length();
  size_t end = s.size();
  size_t p1 = 0;
  size_t p2 = 0;

  while (p1 < end)
  {
    p2 = result.find_first_of(from, p1);

    if ( p2 != string::npos)
    {
      result.replace(p2, fLen, to);
      p1 = p2 + tLen;
    }
    else
      p1 = end;
  }

  return result;
}

string getFilePath(string path, const string fileName)
{
  if (0 == path.length())
  {
    path = FileSystem::GetCurrentFolder();
    FileSystem::AppendItem(path, "parallel");
  }
  FileSystem::AppendItem(path, fileName);
  return path;
}


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
    fconsumeOp.SetUsesArgsInTypeMapping();
    AddOperator(&ffeedOp);
    ffeedOp.SetUsesArgsInTypeMapping();

    AddOperator(&hadoopjoinOp);
    hadoopjoinOp.SetUsesArgsInTypeMapping();

    AddOperator(&fdistributeOp);
    fdistributeOp.SetUsesArgsInTypeMapping();

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

