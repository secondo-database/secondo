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
//[<] [$<$]
//[>] [$>$]
//[INSET] [$\in$]

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

#include "HadoopParallelAlgebra.h"
#include <vector>
#include <iostream>
#include <string>
#include "RelationAlgebra.h"
#include "QueryProcessor.h"
#include "StandardTypes.h"
#include "Counter.h"
#include "TupleIdentifier.h"
#include "LogMsg.h"
#include "ListUtils.h"
#include "FTextAlgebra.h"
#include "Symbols.h"
#include "Base64.h"
#include "regex.h"
#include "FileSystem.h"
#include "StringUtils.h"
#include "Symbols.h"
#include "Application.h"

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
              nl->SymbolAtom(CcString::BasicType())),
          nl->TwoElemList(nl->StringAtom("valueT",false),
              nl->SymbolAtom(FText::BasicType())));
      NList AttrList(attrList, nl);
      NList tupleStreamList =
          NList(NList().tupleStreamOf(AttrList));

      return nl->ThreeElemList(
                 nl->SymbolAtom(Symbol::APPEND()),
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
  valueStr = stringutils::replaceAll(valueStr, "\n", "");
  return valueStr;
}

ListExpr binDecode(string binStr)
{
  Base64 b64;
  stringstream iss, oss;
  ListExpr nestList;
  iss << binStr;
  b64.decodeStream(iss, oss);
  if (nl->ReadBinaryFrom(oss, nestList))
    return nestList;
  else
    return nl->TheEmptyList();
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
        nl->Second(nl->First(streamTupleList)),FText::BasicType()))
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
    ListExpr resultList = nl->TwoElemList(
        nl->SymbolAtom(Symbol::STREAM()),
        nl->TwoElemList(
            nl->SymbolAtom(Tuple::BasicType()),
            resultAttrList));

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
    result = localInfo->nextJoinTuple( s );

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
Word phjLocalInfo::nextJoinTuple( Supplier s )
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

  if ((tupleIterator = getNewProducts( s )) != 0)
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

GenericRelationIterator* phjLocalInfo::getNewProducts( Supplier s) 
{

  TupleBuffer *tbA = 0;
  TupleBuffer *tbB = 0;
  GenericRelationIterator *iteratorA = 0, *iteratorB = 0;
  Tuple *tupleA = 0, *tupleB = 0;
  string tupStr, sTupStr;
  long MaxMem = (qp->GetMemorySize(s) * 1024 * 1024);

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

  if (nl->ListLength(args) < 1)
    return listutils::typeError("Expect one argument at least");
  ListExpr first = nl->First(args);
  if (!listutils::isRelDescription(first))
    return listutils::typeError("rel(tuple(...)) expected");
  return nl->TwoElemList(nl->SymbolAtom(Symbol::STREAM()),
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
  if (nl->ListLength(args) < 2)
    return listutils::typeError("Expect two argument at least");
  ListExpr second = nl->Second(args);
  if (!listutils::isRelDescription(second))
    return listutils::typeError("rel(tuple(...)) expected");
  return nl->TwoElemList(nl->SymbolAtom(Symbol::STREAM()),
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
  if (nl->ListLength(args) < 3)
    return listutils::typeError("Expect 3 arguments at least");
  ListExpr third = nl->Third(args);
  if (!listutils::isRelDescription(third))
      return listutils::typeError("rel(tuple(...)) expected");
  return nl->TwoElemList(nl->SymbolAtom(Symbol::STREAM()),
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
        nl->Second(nl->First(attrList)),FText::BasicType()))
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
                nl->SymbolAtom(Symbol::STREAM()),
                nl->TwoElemList(nl->SymbolAtom(Tuple::BasicType()),
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
        (qp->GetMemorySize(s) * 1024 * 1024));

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
  && listutils::isSymbol(
      nl->Second(nl->First(tupleList)), CcString::BasicType())
  && listutils::isSymbol(nl->Second(
      nl->Second(tupleList)), FText::BasicType()))
  {
    return streamNL;
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

//Set up the remote copy command options uniformly.
const string scpCommand = "scp -q ";



/*
7 Implementation of clusterInfo class

The clusterInfo class is used to read two line-based text files that
describe the distribution of a cluster.

The locations of these files are denoted by two environment
variables: master and slave lists.
The first one is used to describe the master node of the cluster,
and can only contains one line.
The second one lists all possible locations within the cluster that
can hold type and data files, which can be written and read by
fconsume and ffeed operators respectively.

Each line of the files are composed by three parts, which are separated
by colons:

----
IP:location:port
----

The IP indicates the network position of a node inside the cluster,
and the location which must be a absolute path, describes the disk
position of the files.
The port is used to tell through which port to access to the Secondo
monitor that is allocated with this file location.

Each Secondo monitor reads its configurations from a file
that is denoted by SECONDO\_CONFIG, which also describes above three
parameters. And all functions inside this class of getting local
information like ~getLocalIP~ reads the configuration from this file.
In principle, the information of the config file should be
in conformity with the list files, or else operators like ~fconsume~
may goes wrong.


Updated in 12th Sept.
The clusterInfo reads both master and slave files together,
and mixed the master and slaves in one list.
The master only has one machine, while its series number is 0.
The slaves contains several machines, and their series numbers
start from 1.
The master node's IP can be repeatable in slaves, in case we want
to use a master as slaves too.

Nodes are separated from each other according to their series numbers,
a same node may exist several times during the list,
especially when a node is viewed as a master and slave at a same time.
We increase the checking before inserting a new node into the list,
and forbid using repeated slave nodes in the lists.
If a non-master node repeats in the slaves, then the construction fails.
But if the master node is viewed as a slave node too,
then it will be viewed as two different nodes,
files can be copied to its local disk, attached with its IP address
as file names' postfixes.

Usually, master node doesn't involved into the parallel processing,
we set it only in case we need to gather the parallel results into
the master node.

*/
clusterInfo::clusterInfo() :
    ps_master("PARALLEL_SECONDO_MASTER"),
    ps_slaves("PARALLEL_SECONDO_SLAVES"),
    localNode(-1), masterNode(0)
{
  available = false;

  //Scan both master and slave lists,
  //and build up a machine list, which insert the master first.
  for (int i = 0; i < 2; i++)
  {
    bool isMaster = ( (0 == i) ? true : false);
    if ( 0 == i )
      disks = new vector<diskDesc>();

    char *ev;
    ev = isMaster ?
        getenv(ps_master.c_str()) :
        getenv(ps_slaves.c_str());

    if ( 0 == ev ){
      cerr << "Environment variable "
           << ( isMaster ? ps_master : ps_slaves )
           << " is not correctly defined." << endl;
      return;
    }

    string fileName = string(ev);
    if (fileName.length() == 0){
        cerr << "Environment variable "
             << (isMaster ? ps_master : ps_slaves)
             << " is set as empty." << endl;
      return;
    }
    else if (
      !FileSystem::FileOrFolderExists(fileName) ||
      FileSystem::IsDirectory(fileName)){
      cerr << "Node list file: " << fileName << endl
          << " does NOT exist." << endl;
      return;
    }

    ifstream fin(fileName.c_str());
    string line;
    while (getline(fin, line))
    {
      if (line.length() == 0)
        continue;  //Avoid warning message for an empty line
      istringstream iss(line);
      string ipAddr, cfPath, sport;
      getline(iss, ipAddr, ':');
      getline(iss, cfPath, ':');
      getline(iss, sport, ':');
      if ((ipAddr.length() == 0) ||
          (cfPath.length() == 0) ||
          (sport.length() == 0))
      {
        cerr << "Format in file " << fileName
            << " is not correct.\n";
        break;
      }

      //TODO verify the correctness of the IP address

      //Remove the slash tail
      if (cfPath.find_last_of("/") == cfPath.length() - 1)
        cfPath = cfPath.substr(0, cfPath.length() - 1);

      int port = atoi(sport.c_str());

      //TODO require a light method to remove duplicated records
      bool noRepeat = true;
      if (disks->size() > 0)
      {
        int csn = disks->size();  // Current node series number
        for (vector<diskDesc>::iterator dit = disks->begin();
             dit != disks->end(); dit++)
        {
          if ( 0 == dit->first.compare(ipAddr) &&
               (dit->second.second == port) &&
               0 == dit->second.first.compare(cfPath))
          {
            if ( dit == disks->begin())
              masterNode = csn;
            else
              noRepeat = false;
          }
        }
      }
      if (noRepeat){
        disks->push_back(diskDesc(
            ipAddr, pair<string, int>(cfPath, port)));
      }
      else{
        cerr << "Exist repeated slave nodes in the list" << endl;
        return;
      }
    }
    fin.close();

// Master list must contain one fully defined node
// Slaves list must contain at least one fully defined node
// Fully defined means all three elements are correctly indicated.
    if (isMaster)
    {
      if (disks->size() != 1)
      {
        cerr << "Master list requires one line" << endl;
        return;
      }
    }
    else
    {
      if (disks->size() < 2)
      {
        cerr << "Slave list should not be empty" << endl;
        return;
      }
    }
  }

  // The node list is built up correctly.
  available = true;
}

clusterInfo::clusterInfo(clusterInfo& rhg):
    ps_master(rhg.ps_master),
    ps_slaves(rhg.ps_slaves),
    available(rhg.available),
    localNode(rhg.localNode),
    masterNode(rhg.masterNode)
{
  disks = new vector<diskDesc>();
  vector<diskDesc>::iterator iter = rhg.disks->begin();
  while (iter != rhg.disks->end()){
    disks->push_back(diskDesc(iter->first,
      pair<string, int>(iter->second.first, iter->second.second)));
    iter++;
  }
}

/*
Read a clusterInfo from a nested list,
which must be a sub set of the current cluster.

*/
bool clusterInfo::covers(NList& clusterList)
{

  NList newCluster(clusterList);
  while (!newCluster.isEmpty()){
    NList slave = newCluster.first();

    if (slave.length() != 4 )
      return false;

    int index       = slave.first().intval();
    string IPAddr   = slave.second().str();
    string filePath = slave.third().str();
    int port        = slave.fourth().intval();

    if ((disks->at(index).first.compare(IPAddr) != 0)
     || (disks->at(index).second.first.compare(filePath) != 0)
     || (disks->at(index).second.second != port))
    {
      cerr << "The import cluster is not "
          "a sub set of the current cluster" << endl;
      return false;
    }

    newCluster.rest();
  }

  return true;

}

/*
The local IP address can be set inside the SecondoConfig file,
but if the setting value doesn't match with any available IP
addresses of the current machine, then an error message will be given.

If it's not defined, then we use all available IP addresses
to match with the slave list.
If nothing is matched, then an error message will be given.

If the error message is given, then the return an empty string.

*/
string clusterInfo::getLocalIP()
{
  string localIP;

  string confPath = string(getenv("SECONDO_CONFIG"));
  localIP = SmiProfile::GetParameter("ParallelSecondo",
      "localIP","", confPath);

  bool match = false;
  vector<string> *aIPs = getAvailableIPAddrs();
  for (vector<string>::iterator it = aIPs->begin();
       it != aIPs->end(); it++)
  {
    string aIP = (*it);
    if (localIP != "")
    {
      if (localIP.compare(aIP) == 0)
        match = true;
    }
    else
    {
      for(vector<diskDesc>::iterator dit = disks->begin();
          dit != disks->end(); dit++)
      {
        if (dit->first.compare(aIP) == 0)
        {
          localIP = aIP;
          match = true;
        }
      }
    }

    if (match) break;
  }

  if (!match)
    cerr << "Host's IP address is "
        "undefined in PARALLEL_SLAVES list. \n" << endl;

  return localIP;
}

vector<string>* clusterInfo::getAvailableIPAddrs()
{
  vector<string>* IPList = new vector<string>();
  struct ifaddrs * ifAddrStruct = 0;
  struct ifaddrs * ifa = 0;
  void * tmpAddrPtr = 0;

  getifaddrs(&ifAddrStruct);
  for (ifa = ifAddrStruct; ifa != 0; ifa = ifa->ifa_next)
  {
    if (0 == ifa->ifa_addr)
      continue;
    if (ifa->ifa_addr->sa_family == AF_INET)
    {
      // IPv4 Address
      tmpAddrPtr = &((struct sockaddr_in*)ifa->ifa_addr)->sin_addr;
      char addressBuffer[INET_ADDRSTRLEN];
      inet_ntop(AF_INET, tmpAddrPtr,
          addressBuffer, INET_ADDRSTRLEN);
      IPList->push_back(addressBuffer);
    }
    else if (ifa->ifa_addr->sa_family == AF_INET6)
    {
      // IPv6 Address
      tmpAddrPtr = &((struct sockaddr_in*)ifa->ifa_addr)->sin_addr;
      char addressBuffer[INET6_ADDRSTRLEN];
      inet_ntop(AF_INET6, tmpAddrPtr,
          addressBuffer, INET6_ADDRSTRLEN);
      IPList->push_back(addressBuffer);
    }
  }

  if (ifAddrStruct)
    freeifaddrs(ifAddrStruct);

  return IPList;
}

/*
Get the remote file path, make sure the remote is accessible.

All files are divided into sub-folders according to their prefix names.
If the sub-folder is not exit, then create it.

Update:
This feature is disabled, as it's too expensive.
The accessible of the remote path is not guaranteed.

*/

string clusterInfo::getRemotePath(
    size_t loc,
    bool includeMaster    /*= true*/,
    bool round            /*= false*/,
    bool appendTargetIP   /*= true */,
    bool appendFileName   /*= false*/,
    string fileName,
    bool attachProducerIP /*= false*/,
    string producerIP     /*= "" */)
{
  string remotePath = "";
  loc = getInterIndex(loc, includeMaster, round);

  string rfPath = (*disks)[loc].second.first;
  if (appendFileName){
    if (attachProducerIP)
    {
      if (producerIP.length() == 0)
        producerIP = getLocalIP();
      fileName += ("_" + producerIP);
    }
    FileSystem::AppendItem(rfPath, fileName);
  }

  string IPAddr = (*disks)[loc].first;
  remotePath = (appendTargetIP ? (IPAddr + ":") : "") + rfPath;

  return remotePath;
}

string clusterInfo::getIP(size_t loc, bool round /* = false*/)
{
  if ( 0 == loc)
    loc = masterNode;
  loc = getInterIndex(loc, true, round);
  return (*disks)[loc].first;
}

size_t clusterInfo::getInterIndex(
    size_t loc, bool includeMaster, bool round){
  assert(disks->size() > 1);

  if (!round){
    assert(loc < disks->size());
    return loc;
  }
  else{
    if (!includeMaster){
      assert(loc > 0);
      return ((loc - 1) % (disks->size() - 1) + 1);
    }
    else{
      return (loc % disks->size());
    }
  }
}

int clusterInfo::searchLocalNode()
{
  string localPath = getLocalPath();
  string localIP = getLocalIP();

  int local = -1, cnt = 0;
  if (localIP.length() != 0 &&
      localPath.length() != 0)
  {
    vector<diskDesc>::iterator iter;
    for (iter = disks->begin(); iter != disks->end(); iter++, cnt++)
    {
      if ((localIP.compare((*iter).first) == 0) &&
          (localPath.compare((*iter).second.first)) == 0)
      {
        local = cnt;
        break;
      }
    }
  }
  else
    cerr << "\nThe local IP or Path is not correctly defined. \n"
      "They should match one line in PARALLEL_SECONDO_SLAVES list.\n"
      "Check the SECONDO_CONFIG file please." << endl;

/*
If a master is used as a slave too,
then it will be viewed as a normal slave node.

*/
  if ( 0 == local)
    local = masterNode;

  return local;
}

void clusterInfo::print()
{
  if (available)
  {
    int counter = 0;
    cout << "\n---- PARALLEL_SECONDO_SLAVES list ----" << endl;
    vector<diskDesc>::iterator iter;
    for (iter = disks->begin(); iter != disks->end();
        iter++, counter++)
    {
      cout << counter << ":\t" << iter->first
          << "\t" << iter->second.first
          << "\t" << iter->second.second << endl;
    }
    cout << "---- PARALLEL_SECONDO_SLAVES ends ----\n" << endl;
  }
}

NList clusterInfo::toNestedList()
{
  if (available)
  {
    NList output;
    vector<diskDesc>::iterator iter = disks->begin();
    int counter = 0;
    while (iter != disks->end())
    {
      NList slave(
          NList(counter),
          NList(iter->first, true, false),
          NList(iter->second.first, true, true),
          NList(iter->second.second));
      output.append(slave);

      iter++;
      counter++;
    }
    return output;
  }
  else
  {
    return NList();
  }
}

/*
5 Operator ~fconsume~

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
the default path SECONDO\_BUILD\_DIR/bin/parallel/.
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

5.1 Update the format of fconsume

In 14/05/2011, remove the machine array parameter of in data remote mode,
as building a Secondo array object that describes the whole structure
of the cluster in every database, limits the flexibility of the whole system.
Therefore, we use a text file list that is denoted by PARALLEL\_SECONDO\_SLAVES
to take the place of the machine array.

In both type remote and data remote modes, target nodes that are
used to backup type file and data files must be registered
in the node list file specified by PARALLEL\_SECONDO\_SLAVES.

Now the operator maps

---- (stream(tuple(...))
      x fileName x filePath x [fileSuffix]
      x [typeLoc1] x [typeLoc2]
      x [targetLoc x dupTimes])
     -> bool
----

Besides the input tuple stream, all the left parameters are divided
into three parts, separated by semicolons, and correspond to the
three modes of the operator.

The basic functions of the operator and its different modes don't
change, only the locations of remote type nodes and remote data nodes
are not given by users explicitly, but are denoted by giving
serial numbers of the PARALLEL\_SECONDO\_SLAVES.
The format of the list file is described in the ~clusterInfo~ section.

Besides, during the data remote mode, the operator should knows
the serial number of the current location before duplicating files,
which requires the operator to get to know the current IP address.
However, I didn't a suitable method to get the local IP address
in different platforms, therefore this IP address must be set inside
the configure file denoted by SECONDO\_CONFIG, as localIP value.

The location of the files is also set up inside the configure file,
as SecondoFilePath, in case the PARALLEL\_SECONDO\_SLAVES is not
required within an individual computer.

In 8/6/2011, increase another parameter into the fconsume operator, rowNum.
As ~ffeed~, we put it in the front of the fileSuffix paramter.
However, we don't strictly distinguish these two parameters.
If both of them is available, then we set the data files' names
with two successive integers connected by a underscore.
If only one number shows up, then only one integer suffix is set
after data files' names.

Now the operator maps

---- (stream(tuple(...))
      x fileName x filePath x [rowNum] x [fileSuffix] ;
      x [typeLoc1] x [typeLoc2]                       ;
      x [targetLoc x dupTimes])                       ;
     -> bool
----


5.2 Specification

*/

struct FConsumeInfo : OperatorInfo {
  FConsumeInfo() : OperatorInfo()
  {
    name =      "fconsume";
    signature = "stream(tuple( ... )) "
        "x string x text x [int] "
        "x [ [int] x [int] ] "
        "x [ int x int ] "
        "-> bool";
    syntax  = "stream(tuple( ... )) "
        "fconsume[ fileName, filePath, [rowNum], [fileSuffix]; "
        "[typeNode1] x [typeNode2]; "
        "[targetIndex x dupTimes] ] ";
    meaning =
        "Export a stream of tuples' data into a binary data file, "
        "and its type nested list into a text type file. "
        "The given file name is used as the data file's name. "
        "If the optional integer value fileSuffix is given, "
        "then the data file's name will be 'fileName_fileSuffix'."
        "The type file name is 'fileName_type'. "
        "Both type file and data file can be duplicated "
        "to some remote machines, which are listed in a list file "
        "indicated by PARALLEL_SECONDO_SLAVES environment variable. "
        "Detail explanations are described in the attached "
        "README.pdf along with the HadoopParallel algebra.";
  }
};

/*
5.3 Type mapping

*/
ListExpr FConsumeTypeMap(ListExpr args)
{
  NList l(args);
  string lengthErr =
      "ERROR!Operator fconsume expects 4 parameter groups, "
      "separated by semicolons";
  string typeErr = "ERROR!Operator fconsume expects "
               "(stream(tuple(...)) "
               "fileName: string, filePath: text, "
               "[rowNum: int] x [fileSuffix: int]; "
               "[typeNodeIndex: int] [typeNodeIndex2: int]; "
               "[targetNodeIndex: int, duplicateTimes: int])";
  string typeErr2 =
      "ERROR!The basic parameters expects "
      "[fileName: string, filePath: text, "
      "[rowNum: int], [fileSuffix: int]]";
  string typeErr3 = "ERROR!Type remote nodes expects "
      "[[typeNodeIndex: int], [typeNodeIndex2: int]]";
  string typeErr4 = "ERROR!Data remote nodes expects "
      "[targetNode:int, duplicateTimes: int]";
  string err1 = "ERROR!The file name should NOT be empty!";
  string err2 = "ERROR!Cannot create type file: \n";
  string err3 = "ERROR!Infeasible evaluation in TM for attribute: ";
  string err4 = "ERROR!Expect the file name and path.";
  string err5 = "ERROR!Expect the file suffix.";
  string err6 = "ERROR!Expect the target index and dupTimes.";
  string err7 = "ERROR!Remote node for type file is out of range";
  string err8 = "ERROR!Building up master and slave list fails, "
      "is $PARALLEL_SECONDO_SLAVES and $PARALLEL_SECONDO_MASTERS "
      "correctly set up ?";
  string err9 = "ERROR!Remote copy type file fail.";

  int len = l.length();
  if ( len != 4)
    return l.typeError(lengthErr);

  string filePreName, filePath;
  bool trMode, drMode;
  drMode = trMode = false;
  int tNode[2] = {-1, -1};

  NList tsList = l.first(); //input tuple stream
  NList bsList = l.second(); //basic parameters
  NList trList = l.third();  //type remote parameters
  NList drList = l.fourth(); //data remote parameters

  NList attr;
  if(!tsList.first().checkStreamTuple(attr) )
    return l.typeError(typeErr);

  //Basic parameters
  //The first list contains all parameters' types
  NList pType = bsList.first();
  //The second list contains all parameter's values
  NList pValue = bsList.second();
  if (pType.length() < 2 || pType.length() > 4)
    return l.typeError(typeErr2);

  if (pType.first().isSymbol(CcString::BasicType()) &&
      pType.second().isSymbol(FText::BasicType()))
  {
    if (pType.length() > 2)
    {
      if (!pType.third().isSymbol(CcInt::BasicType()))
        return l.typeError(err5);

      if ((4 == pType.length()) &&
          !pType.fourth().isSymbol(CcInt::BasicType()))
        return l.typeError(err5);
    }

    NList fnList;
    if (!QueryProcessor::GetNLArgValueInTM(pValue.first(), fnList))
      return l.typeError(err3 + "file prefix name");
    filePreName = fnList.str();
    if (0 == filePreName.length())
      return l.typeError(err1);
    NList fpList;
    if (!QueryProcessor::GetNLArgValueInTM(pValue.second(), fpList))
      return l.typeError(err3 + "filePath");
    filePath = fpList.str();
  }
  else
    return l.typeError(err4);

  pType = trList.first();
  if (!pType.isEmpty())
  {
    if (pType.length() > 2)
      return l.typeError(typeErr3);
    while (!pType.isEmpty())
    {
      if (!pType.first().isSymbol(CcInt::BasicType()))
        return l.typeError(typeErr3);
      pType.rest();
    }

    pValue = trList.second();
    trMode = true;
    int cnt = 0;
    while (!pValue.isEmpty())
    {
      NList nList;
      if (!QueryProcessor::GetNLArgValueInTM(pValue.first(), nList))
        return l.typeError(err3 + " type node index");
      tNode[cnt++] = nList.intval();
      pValue.rest();
    }
  }

  pType = drList.first();
  if (!pType.isEmpty())
  {
    if (pType.length() != 2)
      return l.typeError(err6);
    if (!pType.first().isSymbol(CcInt::BasicType()) ||
        !pType.second().isSymbol(CcInt::BasicType()))
      return l.typeError(typeErr4);
    drMode = true;
  }

  //Type Checking is done, create the type file.
  filePath = getLocalFilePath(filePath, filePreName, "_type");
  if (filePath.length() == 0)
    return l.typeError(err2 +
      "Type file path is unavailable, check the SecondoConfig.ini.");
  ofstream typeFile(filePath.c_str());
  NList resultList = NList(NList(Relation::BasicType()),
                           tsList.first().second());
  if (typeFile.good())
  {
    typeFile << resultList.convertToString() << endl;
    typeFile.close();
    cerr << "Type file: " << filePath << " is created. " << endl;
  }
  else
    return l.typeError(
        err2 + "Type file path is unavailable: " + filePath);

  //Verify the existence of the PARALLEL\_SECONDO\_SLAVES file
  if (trMode || drMode)
  {
    clusterInfo *ci = new clusterInfo();

    if (!ci->isOK())
      return l.typeError(err8);
    int sLen = ci->getSlaveSize();
    //Copy type files to remote location
    for (int i = 0; i < 2; i++)
    {
      if (tNode[i] >= 0)
      {
        if ( tNode[i] > sLen )
        {
          ci->print();
          return l.typeError(err7);
        }
        else
        {
/*
Copy the type file to a remote path without changing the file name.
The master node is also included.

*/
          string rPath = ci->getRemotePath(tNode[i]);
          cerr << "Copy type file to -> \t" << rPath << endl;
          if ( 0 != (system
              ((scpCommand + filePath + " " + rPath).c_str())))
            return l.typeError(err9);
        }
      }
    }
  }

  return NList(NList(CcBool::BasicType())).listExpr();
}


/*
5.4 Value mapping

*/
int FConsumeValueMap(Word* args, Word& result,
    int message, Word& local, Supplier s)
{
  fconsumeLocalInfo* fcli = 0;

  if ( message <= CLOSE)
  {
    result = qp->ResultStorage(s);
    Supplier bspList = args[1].addr,
             drpList = args[3].addr;
    string relName, fileSuffix = "", filePath;
    int fileIndex = -1;

    relName = ((CcString*)
      qp->Request(qp->GetSupplierSon(bspList, 0)).addr)->GetValue();
    filePath = ((FText*)
      qp->Request(qp->GetSupplierSon(bspList, 1)).addr)->GetValue();
    int bspLen = qp->GetNoSons(bspList);
    int idx = 2;
    while (idx < bspLen)
    {
      fileIndex = ((CcInt*)
        qp->Request(qp->GetSupplier(bspList, idx)).addr)->GetValue();
      if (fileIndex >= 0)
        fileSuffix += ("_" + int2string(fileIndex));
      idx++;
    }

    int ti = -1, dt = -1;
    bool drMode = false;

    int drpLen = qp->GetNoSons(drpList);
    if (drpLen == 2)
    {
      drMode = true;
      ti = ((CcInt*)
          qp->Request(qp->GetSupplier(drpList, 0)).addr)->GetValue();
      dt = ((CcInt*)
          qp->Request(qp->GetSupplier(drpList, 1)).addr)->GetValue();
    }

    //Check whether the duplicate parameters are available
    clusterInfo *ci = 0;
    if (drMode)
    {
      ci = new clusterInfo();
      if ((ti > (int)ci->getSlaveSize()))
      {
        ci->print();
        cerr <<
            "ERROR! The first target node for backing up duplicate "
            "data files is out of the range of the slave list.\n";
        ((CcBool*)(result.addr))->Set(true, false);
        return 0;
      }
    }

    fcli = (fconsumeLocalInfo*) local.addr;
    if (fcli) delete fcli;

    fcli = new fconsumeLocalInfo();
    fcli->state = 0;
    fcli->current = 0;
    local.setAddr(fcli);

    //Write complete tuples into a binary file.
    //create a path for this file.
    filePath = getLocalFilePath(filePath, relName, fileSuffix);
    ofstream blockFile(filePath.c_str(), ios::binary);
    if (!blockFile.good())
    {
      cerr << "ERROR!Create file " << filePath << " fail!" << endl;
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
    cout << "\nData file: " << filePath << " is created" << endl;

    if (drMode)
    {
      bool keepLocal = false;
      int localNode = ci->getLocalNode();
      if (localNode < 0)
      {
        cerr
          << "ERROR! Cannot find the local position " << endl
          << ci->getLocalIP() << ":" << ci->getLocalPath() << endl
          << "in the slave list, backup files fail. " << endl;
        ci->print();

        ((CcBool*) (result.addr))->Set(true, false);
        return 0;
      }

      //Attach producer's IP to file's name if it's duplicated.
      string pdrIP = ci->getIP(localNode);

      //Avoid copying file to a same node repeatedly
      int cLen = ci->getClusterSize();
      bool copyList[cLen];
      memset(copyList, false, cLen);
      for (int i = 0; i < dt; i++, ti++)
        copyList[ ti % cLen ] = true;
      //Synchronize the copy status of master node
      if ( (ci->getMasterNode() != 0) &&
           (copyList[0] || copyList[ci->getMasterNode()] ))
        copyList[0] = copyList[ci->getMasterNode()] = true;

      for (int i = 0; i < cLen; i++)
      {
        if (copyList[i])
        {
          if ((localNode == i) ||  //slaves
              ((0 == i) &&         //master
               (localNode == ci->getMasterNode())))
          {
            keepLocal = true;
            continue;
          }
          else
          {
/*
Copy the data file into a remote path,
The data file is possible duplicated to the master node.
The series number of slaves may be round.
the remote file name may be changed in order to denote the producer.
the target IP is appended, for using the scp command.

*/
            string rPath = ci->getRemotePath(
                i, true, true, true, true, relName, true);
            cerr << "Copy " << filePath
                << "\n->\t" << rPath << endl;
            if ( 0 != ( system(
                (scpCommand + filePath + " " + rPath).c_str())))
            {
              cerr << "Copy remote file fail." << endl;
              ((CcBool*)(result.addr))->Set(true, false);
              return 0;
            }
          }
        }
      }
      if (!keepLocal)
      {
        if ( 0 != (system(("rm " + filePath).c_str())))
        {
          cerr << "Delete local file " << filePath << " fail.\n";
          ((CcBool*)(result.addr))->Set(true, false);
          return 0;
        }
        cerr << "Local file '" + filePath + "' is deleted.\n";
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
6 Operator ~ffeed~

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
SECONDO\_BUILD\_DIR/. Or else it must be an absolute Unix path.

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

6.1 Update the format of ffeed

In 18/05/2011, adjust ~ffeed~ operator to read the remote locations
from list files, not from the string array.

Now the operator maps

----
fileName x path x [fileIndex] x [typeNodeIndex]
x [targetNodeIndex x attemptTimes]
->stream(tuple(...))
----

Similar as the ~fconsume~ operator, parameters of this operator
are also divided into three parts, and are separated by semicolons.
Because of that, we change ~ffeed~ from a prefix operator to
a post operator, since the prefix operators don't accept semicolons.
The prefix parameter is the file name of string type.

The ~typeNodeIndex~ and ~targetNodeIndex~ denote the locations of
some specific nodes inside the cluster, which are listed
inside the PARALLEL\_SECONDO\_SLAVES file.

In 8/6/2011, add the row number into the ffeed operator.
The rowNumber doesn't affect the type file, only the data type.
If it's defined, then the ffeed will fetch a file with two
successive suffices. Now the operator maps

----
fileName
x path x [rowNum] x [fileIndex]     ;
x [typeNodeIndex]                   ;
x [targetNodeIndex x attemptTimes]  ;
->stream(tuple(...))
----


6.2 Specification

*/

struct FFeedInfo : OperatorInfo {

  FFeedInfo() : OperatorInfo()
  {
    name =      "ffeed";
    signature = "string x text x [int] x [int] x [int x int x int]"
        " -> stream(tuple(...))";
    syntax  = "fileName ffeed[ filePath, [fileSuffix]; "
        "[remoteTypeNode]; "
        "[producerIndex x targetIndex x attemptTimes] ]";
    meaning =
        "Restore a tuple stream from the binary data and "
        "text type files that are created by "
        "fconsume or fdistribute operator. "
        "Both type and data file can be fetched from "
        "remote machines which are listed in the "
        "PARALLEL_SECONDO_SLAVES file."
        "Detail explanations are described in the attached "
        "README.pdf along with the HadoopParallel algebra.";
  }
};

/*
6.3 Type mapping

*/

ListExpr FFeedTypeMap(ListExpr args)
{
  NList l(args);
  NList pType, pValue;
  bool haveIndex, trMode, drMode;
  haveIndex = trMode = drMode = false;

  string lenErr = "ERROR! Operator ffeed expects "
      "four parts parameters, separated by semicolons";
  string typeErr = "ERROR! Operator ffeed expects "
      "fileName: string, filePath: text, "
      "[rowNum: int] [fileSuffix: int]; "
      "[typeNodeIndex: int]; "
      "[producerIndex: int, targetIndex: int, attemptTimes: int]";
  string err1 = "ERROR! File name should NOT be empty!";
  string err2 = "ERROR! Type file is NOT exist!\n";
  string err3 = "ERROR! A tuple relation type list is "
      "NOT contained in file: ";
  string err4 = "ERROR! Infeasible evaluation in TM for attribute ";
  string err5 = "ERROR! Prefix parameter expects fileName: string";
  string err6 = "ERROR! Basic parameters expect "
      "filePath: text, [rowNum: int] [fileSuffix: int] ";
  string err7 = "ERROR! Type remote parameter expects "
      "[typeNodeIndex: int]; ";
  string err8 = "ERROR! Remote node for type file is out of range.";
  string err9 = "ERROR! Data remote parameters expect "
      "[producerIndex: int, targetIndex: int, attemptTimes: int]";
  string err10 = "ERROR! The slave list file does not exist."
      "Is $PARALLEL_SECONDO_SLAVES correctly set up ?";
  string err11 = "ERROR! Copy remote type file fail.";


  if (l.length() != 4)
    return l.typeError(lenErr);

  NList fn = l.first();
  pType = fn.first();
  pValue = fn.second();
  if (!pType.isSymbol(CcString::BasicType()))
    return l.typeError(err5);
  NList fnList;
  if (!QueryProcessor::GetNLArgValueInTM(pValue, fnList))
    return l.typeError(err4 + "fileName");
  string fileName = fnList.str();
  if (0 == fileName.length())
    return l.typeError(err1);

  NList bp = l.second();  //basic parameters
  pType = bp.first();
  pValue = bp.second();
  int bpLen = pType.length();
  if (bpLen < 1 || bpLen > 3)
    return l.typeError(err6);
  if (!pType.first().isSymbol(FText::BasicType()))
    return l.typeError(err6);
  if (bpLen > 1)
  {
    if (!pType.second().isSymbol(CcInt::BasicType()))
      return l.typeError(err6);
    if (bpLen == 3 &&
        !pType.third().isSymbol(CcInt::BasicType()))
      return l.typeError(err6);
  }

  NList fpList;
  if (!QueryProcessor::GetNLArgValueInTM(pValue.first(), fpList))
    return l.typeError(err4 + "filePath");
  string filePath = fpList.str();
  filePath = getLocalFilePath(filePath, fileName, "_type");


  NList tr = l.third();
  pType = tr.first();
  int tnIndex = -1;
  if (!pType.isEmpty())
  {
    if (pType.length() > 1 ||
        !pType.first().isSymbol(CcInt::BasicType()))
      return l.typeError(err7);
    trMode = true;
    pValue = tr.second();
    tnIndex = pValue.first().intval();
  }

  NList dr = l.fourth();
  pType = dr.first();
  if (!pType.isEmpty())
  {
    if (pType.length() != 3)
      return l.typeError(err9);
    if (!pType.first().isSymbol(CcInt::BasicType()) ||
        !pType.second().isSymbol(CcInt::BasicType()) ||
        !pType.third().isSymbol(CcInt::BasicType()))
      return l.typeError(err9);
    drMode = true;
  }

  if (tnIndex >= 0)
  {
    //copy the type file from remote to here
    clusterInfo *ci = new clusterInfo();
    if (!ci->isOK())
      return l.typeError(err10);

    int sLen = ci->getSlaveSize();
    if (tnIndex > sLen)
    {
      ci->print();
      return l.typeError(err8);
    }

    string rPath = ci->getRemotePath(tnIndex, true, false, true,
        true, (fileName + "_type"));
    //put the type file into a temporary file
    filePath = FileSystem::MakeTemp(filePath);
    cerr << "Copy the type file " << filePath
        << " from <-" << "\t" << rPath << endl;
    if (0 != system((scpCommand + rPath + " " + filePath).c_str()))
      return l.typeError(err11);
  }

  ListExpr relType;
  if (!nl->ReadFromFile(filePath, relType))
    return l.typeError(err2 + filePath);
  //Read type file of DLF flist
  if (!(listutils::isRelDescription(relType)
    || listutils::isTupleStream(relType)))
    return l.typeError(err3 + filePath);
  NList streamType =
      NList(NList(Symbol::STREAM()),
      NList(NList(relType).second()));

  return streamType.listExpr();
}

/*
6.4 Value mapping

*/
int FFeedValueMap(Word* args, Word& result,
    int message, Word& local, Supplier s)
{
  string relName, path, fileSuffix = "";
  FFeedLocalInfo* ffli = 0;
  int prdIndex = -1, tgtIndex = -1;
  int attTimes = 0;

  switch(message)
  {
    case OPEN: {
      if (!((CcString*)args[0].addr)->IsDefined()){
        cerr << "File Name string is undefined." << endl;
        return 0;
      }
      else{
        relName = ((CcString*)args[0].addr)->GetValue();
      }

      Supplier bspNode = args[1].addr,
               drpNode = args[3].addr;

      path =  ((FText*)qp->Request(
          qp->GetSupplierSon(bspNode, 0)).addr)->GetValue();
      int bspLen = qp->GetNoSons(bspNode);
      int idx = 1;
      while (idx < bspLen )
      {
        int index = ((CcInt*)qp->Request(
            qp->GetSupplierSon(bspNode, idx)).addr)->GetValue();
        if (index >= 0)
          fileSuffix += ("_" + int2string(index));
        idx++;
      }

      if (qp->GetNoSons(drpNode) == 3)
      {
        prdIndex = ((CcInt*)qp->Request(
            qp->GetSupplierSon(drpNode, 0)).addr)->GetValue();
        tgtIndex = ((CcInt*)qp->Request(
            qp->GetSupplierSon(drpNode, 1)).addr)->GetValue();
        attTimes = ((CcInt*)qp->Request(
            qp->GetSupplierSon(drpNode, 2)).addr)->GetValue();
      }

      string filePath = path;
      filePath =
          getLocalFilePath(filePath, relName, fileSuffix, false);

      ffli = (FFeedLocalInfo*) local.addr;
      if (ffli)
      {
        delete ffli;
        ffli = 0;
      }
      ffli = new FFeedLocalInfo(s);
      if (ffli->fetchBlockFile(
          relName , fileSuffix, filePath, s,
          prdIndex, tgtIndex, attTimes))
      {
        ffli->returned = 0;
        local.setAddr(ffli);
      }
      else
      {
        delete ffli;
        ffli = 0;
        local.setAddr(0);
      }

      return 0;
    }
    case REQUEST: {
      ffli = (FFeedLocalInfo*)local.addr;

      if (!ffli)
        return CANCEL;

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
      if (!ffli)
        return CANCEL;
      else
      {
        if (ffli->tupleBlockFile){

          ffli->tupleBlockFile->close();
          delete ffli->tupleBlockFile;
          ffli->tupleBlockFile = 0;
        }
      }

      return 0;  //must return
    }

    case CLOSEPROGRESS: {
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
6.5 Implementation of FFeedLocalInfo methods

*/

bool FFeedLocalInfo::isLocalFileExist(string fp)
{
  if (fp.length() != 0)
  {
    if (FileSystem::FileOrFolderExists(fp)){
      return !FileSystem::IsDirectory(fp);
    }
  }
  return false;
}

bool FFeedLocalInfo::fetchBlockFile(
    string fileName, string fileSuffix, string filePath, Supplier s,
    int pdi, int tgi, int att)
{
/*

  * pdi: producer node index
  * tgi: target node index
  * att: attempt times

*/

  //Fetch the binary file from remote machine.
  string pdrIP = "", tgtIP = "";
  clusterInfo *ci = 0;

  string targetFilePath = filePath;
  FileSystem::AppendItem(targetFilePath, fileName + fileSuffix);

/*
Detect whether the file is exist or not.
If the file exists, the fileFound is set as true,
and the filePath contains the complete local path of the file.
Or else, the fileFound is false.

*/
  if (pdi < 0)
  {
    //Fetch the file in the local machine
    fileFound = isLocalFileExist(targetFilePath);
  }
  else
  {
    //Fetch the file in a remote machine
    ci = new clusterInfo();
    if(!ci->isOK())
    {
      cerr << "ERROR!The PARALLEL_SECONDO_SLAVES list is not "
          "correctly set up." << endl;
      return false;
    }
    if ((tgi > (int)ci->getSlaveSize())
     || (pdi > (int)ci->getSlaveSize()))
    {
      cerr << "ERROR!Producer index " << pdi
          << " or target index " << tgi
          << " is out of the range of the slave list: "
          << ci->getSlaveSize() << endl;
      ci->print();
      return false;
    }

    if ( 0 == pdi ){
      pdi = ci->getMasterNode();
    }
    pdrIP = ci->getIP(pdi);

    while (!fileFound && (att-- > 0))
    {
      string rFilePath;  //The remote file
      string lFilePath;  //The local file
      int targetIndex = ((tgi == 0) ? ci->getMasterNode() : tgi);
      tgtIP = ci->getIP(targetIndex, true);

/*
remoteFileName adds the producer IP address as suffix
if the target machine is not the producer.

*/
      bool attachProducerIP = !(targetIndex == pdi);
      rFilePath = ci->getRemotePath(targetIndex,
          true,   // may copy to master node
          true,   // may traverse the whole array
          true,   // attach with target node IP
          true,
          fileName + fileSuffix, // attpen file name
          attachProducerIP,
          pdrIP);

      if (ci->getLocalIP().compare(tgtIP) == 0){
        //looking at the target at local disk
        lFilePath = rFilePath.substr(rFilePath.find(":") + 1);
      }
      else{
        //use scp to copy the file to a temporary file,
        //in case several processes both want to copy a same file.
        int copyTimes = MAX_COPYTIMES;
        lFilePath = FileSystem::MakeTemp(targetFilePath);
        string cStr = scpCommand + rFilePath +
            " " + lFilePath;
        while (!fileFound && copyTimes-- > 0){
          if (0 == system(cStr.c_str())){
              break;
          }
        }
      }

      fileFound = isLocalFileExist(lFilePath);
      if (fileFound){
        targetFilePath = lFilePath;
      }
      if (!fileFound) {
        cerr << "Warning! Cannot fetch file at : "
            << rFilePath << endl;
        tgi++;
      }
    }
  }

  if (!fileFound)
  {
    cerr << "\nERROR! File " << targetFilePath
         << " is not exist and cannot be remotely fetched.\n\n\n";
    return false;
  }
  tupleBlockFile = new ifstream(targetFilePath.c_str(), ios::binary);
  if (!tupleBlockFile->good())
  {
    cerr << "ERROR! Read file " << targetFilePath << " fail.\n\n\n";
    tupleBlockFile = 0;
    return false;
  }

  //Catch the file, and read the description list
  u_int32_t descSize;
  size_t fileLength;
  tupleBlockFile->seekg(0, ios::end);
  fileLength = tupleBlockFile->tellg();
  tupleBlockFile->seekg(
      (fileLength - sizeof(descSize)), ios::beg);
  tupleBlockFile->read((char*)&descSize, sizeof(descSize));

  char descStr[descSize];
  tupleBlockFile->seekg(
      (fileLength - (descSize + sizeof(descSize))), ios::beg);
  tupleBlockFile->read(descStr, descSize);
  tupleBlockFile->seekg(0, ios::beg);

  NList descList = NList(binDecode(string(descStr)));
  if (descList.isEmpty())
  {
    cerr << "\nERROR! Reading ending description list fail." << endl;
    return false;
  }

  //Initialize the sizes of progress local info
  noAttrs = tupleType->GetNoAttributes();
  total = descList.first().intval();
  attrSize = new double[noAttrs];
  attrSizeExt = new double[noAttrs];
  for(int i = 0; i < noAttrs; i++)
  {
    attrSizeExt[i] =
        descList.elem(4 + i*2).realval() / total;
    attrSize[i] =
        descList.elem(4 + (i*2 + 1)).realval() / total;

    SizeExt += attrSizeExt[i]; //average sizeExt of a tuple
    Size += attrSize[i];
  }

  sizesInitialized = true;
  sizesChanged = true;

  return true;
}

Tuple* FFeedLocalInfo::getNextTuple(){

  if (!fileFound)
    return 0;

  Tuple* t = 0;
  u_int32_t blockSize;

  assert(tupleBlockFile->good());
  tupleBlockFile->read(
      reinterpret_cast<char*>(&blockSize),
      sizeof(blockSize));
  if (!tupleBlockFile->eof() && (blockSize > 0))
  {
    blockSize -= sizeof(blockSize);
    char *tupleBlock = new char[blockSize];
    TupleId tid = tupleBlockFile->tellg();
    tupleBlockFile->seekg(tid);
    tupleBlockFile->read(tupleBlock, blockSize);

    t = new Tuple(tupleType);
    t->ReadFromBin(tupleBlock, blockSize);
    t->SetTupleId(tid);
    delete[] tupleBlock;
  }

  return t;
}

/*
7 Operator ~hadoopjoin~

This operator carries out a Hadoop join operation in Secondo.

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


Update in 10/06/2010
Replace the requirement for Partition attribute
by denoting a partition basis attribute,
if this attribute's type provides the HashValue function
required by ~fdistribute~ operator.

At the same time,
the machineArray and masterIndex are not required any more,
since we use the PARALLEL\_SECONDO\_SLAVES list.

Now the operator maps

----
stream(tuple(T1) x stream(tuple(T2))
x partAttr1 x partAttr2 x partitionNum x resultName
x (map stream(tuple(T1)) stream(tuple(T2) stream(tuple(T1 T2)))))
-> stream(tuple((MIndex int)(PIndex int)))
----

Update in 23/10/2011
Add a new optional parameter, named data path.
Usually, the hadoop job of hadoop join operator reads data
in map step, from Secondo databases in every slave.
But in the CLOUD environment, like the Amazon EC2,
data are kept as disk files in additional storage devices,
as to re-use them in different scale clusters.
In this case, hadoop join should tell each map task to read data
from files, not from databases.

The parameter is named as dataFilePath, in text type.
It's an optional parameter, if it's not indicated, then the hadoop
job reads data from Secondo databases on each slave.
Or else data are read from disk files.

Now the operator maps

----
stream(tuple(T1) x stream(tuple(T2))
x partAttr1 x partAttr2 x partitionNum x resultName x dataFilePath
x (map stream(tuple(T1)) stream(tuple(T2) stream(tuple(T1 T2)))))
-> stream(tuple((MIndex int)(PIndex int)))
----

Update in 31/10/2011
Replace the optional parameter ~dataFilePath~ to be an optional
parameter ~dataLocRel~, which is a relation that contains two fields:
SIndex:int and DPath:text.

Through this change, ~hadoopjoin~ can read data in a more flexible way,
in the past, every slave is assumed to have the data in its own Secondo database.
After this improvement, they can either read the data from Secondo database
or local file system, and may don't need to perform the map tasks.

The SIndex indicates the series number of a slave which contains the
data that ~hadoopjoin~ need read. And the DPath indicates the location
of the data. Normally it's the data file path, when it's denoted as
a special string <READ DB/>, then the data is kept in the slave's
Secondo database. When a slave is not included in this relation,
then it doesn't need to execute the map task.

This parameter is an optional parameter too, when it's not given,
then all slaves read data from their own Secondo databases.

Now the operator maps

----
stream(tuple(T1) x stream(tuple(T2))
x partAttr1 x partAttr2 x partitionNum x resultName x dataLocRel
x (map stream(tuple(T1)) stream(tuple(T2) stream(tuple(T1 T2)))))
-> stream(tuple((MIndex int)(PIndex int)))
----

*/

struct HdpJoinInfo : OperatorInfo {

  HdpJoinInfo() : OperatorInfo(){
    name = "hadoopjoin";
    signature =
        "(stream(tuple(T1)) x stream(tuple(T2)) x "
        "rel(tuple(SIndex:int DPath:text)) x "
        " partAttr1 x partAttr2 x int x string x "
        " (map stream(tuple(T1)) "
        "  stream(tuple(T2) stream(tuple(T1 T2)))))"
        "-> stream(tuple(int int))";
    syntax = "stream(tuple(T1) stream(tuple(T2)) "
        "hadoopjoin[partAttr1, partAttr2, dataLocRel"
        "partitionNum, resultName; "
        "joinQuery]";
    meaning =
        "Evaluating a join operation on parallel Secondo "
        "by invoking a generic Hadoop join job. "
        "The join procedure is processed by several computers "
        "within a cluster simultaneously. "
        "The result tuples are encapsulated into several files, "
        "stored on different nodes. "
        "The output stream of this operator denotes the locations "
        "of these result data files."
        "The third relation parameter is an optional parameter. ";
  }
};

/*
7.1 Type mapping

*/

ListExpr hdpJoinTypeMap(ListExpr args)
{
  string lengErr = "Operator hadoopjoin expects a list "
      "of seven arguments. ";
  string typeErr = "operator hadoopjoin expects "
      "(stream(tuple(T1)), stream(tuple(T2)), "
      "[rel(tuple(int, text))] "
      "partAttr1, partAttr2, int, string, "
      "(map (stream(tuple(T1))) stream(tuple(T2)) "
      "  stream(tuple(T1 T2))) )";
  string err1 =
      "ERROR! Infeasible evaluation in TM for attribute ";

  NList l(args);
  bool dre = true;  // Dataloc Relation Exist
  if (l.length() != 8)
    return l.typeError(lengErr);

  string ss[2] = {"", ""};  // nested list of input streams
  string an[2] = {"", ""};  // attribute name
  int attrOffset = 2;       //The offset of argument parameter
  //Check both input are tuple streams,
  //and the partition attribute is included in respective stream
  for (int argIndex = 1; argIndex <= 2; argIndex++)
  {
    NList attrList;
    NList streamList = l.elem(argIndex).first();
    if (!streamList.checkStreamTuple(attrList))
      return l.typeError(typeErr);

    NList partAttr = l.elem(argIndex + attrOffset).first();
    if (!partAttr.isAtom())
      return l.typeError(typeErr);

    ListExpr attrType;
    string attrName = partAttr.str();
    int attrIndex = listutils::findAttribute(
        attrList.listExpr(), attrName, attrType);
    if (attrIndex <= 0)
      return l.typeError(typeErr);

    ss[argIndex - 1] = l.elem(argIndex).second().convertToString();
    an[argIndex - 1] = attrName;
  }

  // Partition scale number
  if (!l.fifth().first().isSymbol(CcInt::BasicType()))
    return l.typeError(typeErr);

  // Result file name
  if (!l.sixth().first().isSymbol(CcString::BasicType()))
    return l.typeError(typeErr);

  NList rnList;
  if (!QueryProcessor::GetNLArgValueInTM(
        l.sixth().second(), rnList))
    return l.typeError(err1 + " resultName");
  string resultName = rnList.str();

  // Check for the data location
  NList drList = l.elem(7).first();
  if (!drList.isEmpty())
  {
    // Check the dataLocRel
    dre = true;
    NList drType = drList.first();
    NList drAttrList;
    if (!drType.checkRel(drAttrList)){
      return l.typeError(typeErr);
    }

    if (!(drAttrList.first().second().isSymbol(CcInt::BasicType())
       && drAttrList.second().second().isSymbol(FText::BasicType())))
    {
      return l.typeError(typeErr);
    }
  }

  string mapStr = l.elem(8).second().fourth().convertToString();
  NList mapList = l.elem(8).first();

  NList attrAB;
  if (! (mapList.first().isSymbol(Symbol::MAP())
      && mapList.fourth().checkStreamTuple(attrAB)))
    return l.typeError(typeErr);

    // Write the join result type into local default path,
    // in case the following operators need.
    NList joinResult =
        NList(NList(Relation::BasicType()),
              NList(NList(Tuple::BasicType()), NList(attrAB)));
    string typeFileName =
        getLocalFilePath("", resultName, "_type", true);
    ofstream typeFile(typeFileName.c_str());
    if (!typeFile.good())
      cerr << "Create typeInfo file Result_type "
          "in default parallel path error!" << endl;
    else
    {
      //The accepted input is a stream tuple
      typeFile << joinResult.convertToString() << endl;
      typeFile.close();
    }
    cerr << "\nSuccess created type file: "
        << typeFileName << endl;

    // result type
    NList a1(NList("MIndex"), NList(CcInt::BasicType()));
    NList a2(NList("PIndex"), NList(CcInt::BasicType()));

    NList result(
        NList(Symbols::STREAM()),
          NList(NList(Tuple::BasicType()),
            NList(
              NList(NList("MIndex"), NList(CcInt::BasicType())),
              NList(NList("PIndex"), NList(CcInt::BasicType())))));

    NList appList;
    appList.append(NList(ss[0], true, true));
    appList.append(NList(ss[1], true, true));
    appList.append(NList(mapStr, true, true));
    appList.append(NList(an[0], true, false));
    appList.append(NList(an[1], true, false));
    appList.append(NList(dre, false));

    return NList(NList(Symbol::APPEND()),
                 appList, result).listExpr();
}

/*
7.2 Value mapping


The dataFilePath is an optional parameter.
If it's not defined, then a special string <READ DB\/> is
sent to the hadoop job, which will read data from Secondo
databases on each slave, during the map step.
If it's defined, then the hadoop job reads data from file system.


*/

int hdpJoinValueMap(Word* args, Word& result,
    int message, Word& local, Supplier s)
{
  hdpJoinLocalInfo* hjli = 0;

  switch(message)
  {
    case OPEN:{
      if (hjli)
      {
        delete hjli;
        hjli = 0;
      }

      //0 Set the parameters
      //0.1 assume the operation happens on
      //all nodes' Secondo databases with a same name
      string dbName =
          SecondoSystem::GetInstance()->GetDatabaseName();

      //0.2 set other arguments
      int rtNum = ((CcInt*)args[4].addr)->GetIntval();
      string rName = ((CcString*)args[5].addr)->GetValue();

      string mrQuery[3] = {
          ((FText*)args[8].addr)->GetValue(),
          ((FText*)args[9].addr)->GetValue(),
          ((FText*)args[10].addr)->GetValue()
      };
      string attrName[2] = {
          ((CcString*)args[11].addr)->GetValue(),
          ((CcString*)args[12].addr)->GetValue()
      };

      bool dre = ((CcBool*)args[13].addr)->GetValue();
      NList dlList;
      if (dre){
        // Build the fileLocList based on the given relation
        GenericRelation* dlr =
            (GenericRelation*)(qp->Request(
                qp->GetSupplierSon(args[6].addr, 0)).addr);
        GenericRelationIterator *iter = dlr->MakeScan();
        Tuple* nextTuple = iter->GetNextTuple();
        while(!iter->EndOfScan()){
          int sIndex =
              ((CcInt*)nextTuple->GetAttribute(0))->GetValue();
          string dLoc =
              ((FText*)nextTuple->GetAttribute(1))->GetValue();
          dlList.append(
              NList(NList(sIndex), NList(dLoc, true, true)));

          nextTuple->DeleteIfAllowed();
          nextTuple = iter->GetNextTuple();
        }
        delete iter;
      }else{
        // Build the fileLocList based on the slave list
        clusterInfo *ci = new clusterInfo();
        if (!ci->isOK()){
          cerr <<
        "\n\nERROR!\n====================\n"
        "The parallel Secondo environment is not correctly set up."
        "Check whether $PARALLEL_SECONDO_SLAVES is defined ? \n"
        "-----------------" << endl;
          return 0;
        }
        size_t sIndex = 1;
        while (sIndex < ci->getSlaveSize()){
          dlList.append(NList(NList((int)sIndex),
                              NList("<READ DB/>", true, true)));
          sIndex++;
        }
      }
      string drlStr = dlList.convertToString();

      //1 evaluate the hadoop program
      stringstream queryStr;
      queryStr << "hadoop jar HdpSec.jar dna.HSJoin \\\n"
        << dbName << " \"" << drlStr << "\"" << " \\\n"
        << "\"" << tranStr(mrQuery[0], "\"", "\\\"") << "\" \\\n"
        << "\"" << attrName[0] << "\" \\\n"
        << "\"" << tranStr(mrQuery[1], "\"", "\\\"") << "\" \\\n"
        << "\"" << attrName[1] << "\" \\\n"
        << "\"" << tranStr(mrQuery[2], "\"", "\\\"") << "\" \\\n"
        << rtNum << " " << rName << endl;
      int rtn;
      cout << queryStr.str() << endl;
      rtn = system("hadoop dfs -rmr OUTPUT");
      rtn = system(queryStr.str().c_str());

      if (rtn != 0)
      {
        cerr <<
        "\n\nERROR!\n====================\n"
        "The hadoop job cannot be successfully executed, "
        "check whether the Hadoop Runtime is correctly installed "
        "and started up.\n"
        "-----------------" << endl;
        return 0;
      }

      hjli = new hdpJoinLocalInfo(s);

      FILE *fs;
      char buf[MAX_STRINGSIZE];
//      fs = popen("cat pjResult", "r");  //Used for debug only
      fs = popen("hadoop dfs -cat OUTPUT/part*", "r");
      if (NULL != fs)
      {
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
      }
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
8 Operator ~fdistribute~

The operator maps

----
stream(tuple(...))
x fileName x path x attrName
x [nBuckets] x [KPA]
-> stream(tuple(fileSufix, value))
----

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

It's also possible to accept the sixth parameter,
KPA (Keep Partition Attribute),
which indicates whether the key attribute is removed.
By default it's false, i.e. remove that key attribute,
just like what the ~distribute~ operator does.
But if it's set to be true, then the key attribute will
stay in the result files.

In 13/5/2011, enable ~fdistribute~ operation with duplication function.
As we need use ~fdistribute~ in the generic hadoop operation's map step,
it's necessary to use ~fdistribute~ operator to duplicate its result
files into candidate nodes, to meet the requirement of fault-tolerance
feature.

Same as ~fconsume~ and ~ffeed~ operators, the duplicate parameters
are optional, and are separated from the basic parameters by semicolons.
Now the operator maps

----
stream(tuple(...))
x fileName x path x attrName
x [nBuckets] x [KPA]
x [typeNodeIndex1] x [typeNodeIndex2]
x [targetIndex x dupTimes ]
-> stream(tuple(fileSufix, value))
----

In 8/6/2011, extend the fdistribute with another new parameter, rowNum.
Since this should also be an optional parameter,
and it may be confused with the nBuckets,
I decided to further divide the parameter list to 5 parts,
set the rowNum after the attrName, as an optional parameter,
and group nBuckeets and KPA as another group of parameters.
Now the operator maps

----
stream(tuple(...))
x fileName x path x attrName x [rowNum] ;
x [nBuckets] x [KPA] ;
x [typeNodeIndex1] x [typeNodeIndex2] ;
x [targetIndex x dupTimes ]
-> stream(tuple(fileSufix, value))
----


8.0 Specification

*/

struct FDistributeInfo : OperatorInfo {

  FDistributeInfo() : OperatorInfo()
  {
    name = "fdistribute";
    signature = "stream(tuple(a1 ... ai ... aj)) "
        "x string x text x symbol x [int] x [int] x [bool] "
        "x [int] x [int] x [ int x int ]"
        "-> stream(tuple( ... )) ";
    syntax =
        "stream(tuple(a1 ... ai ... aj)) "
        " fdistribute[ fileName, path, partitionAttr, [rowNum];"
        " [bucketNum], [KPA]; "
        " [typeNode1], [typeNode2]; "
        " [targetIndex,  dupTimes] ]";
    meaning =
        "Export a stream of tuples into binary data files "
        "that can be read by ffeed operator, and write the schema "
        "of the stream into a text type file. "
        "Tuples are distributed into different data files "
        "based on the hash value of the given partition attribute, "
        "if the attribute's type provides the HashValue function. "
        "Data files are distinguished from each other "
        "by using these hash values as their name's suffices. "
        "If the bucketNum is given, then the tuples are re-hashed "
        "by the bucketNum again to achieve an even partition. "
        "Users can optionally keeping the partition attribute value "
        "by setting the value of KPA(Keep Partition Attribute) "
        "as true,  which is set as false by default. "
        "Both type file and data file can be duplicated "
        "to some remote machines, which are listed in a list file "
        "indicated by PARALLEL_SECONDO_SLAVES environment variable. "
        "Detail explanations are described in the attached "
        "README.pdf along with the HadoopParallel algebra.";
  }

};

/*
8.1 Type mapping

*/
ListExpr FDistributeTypeMap(ListExpr args)
{
  NList l(args);
  string lenErr = "ERROR!Operator expects 5 parts arguments.";
  string typeErr = "ERROR!Operator expects "
      "(stream(tuple(a1, a2, ..., an))) "
      "x string x text x ai x [int] x [int] x [bool] "
      "x [int] x [int] x [ int x int ] ";
  string attErr = "ERROR!Operator cannot find the "
      "partition attribute: ";
  string err4 = "ERROR!Basic arguments expect "
      "fileName: string, filePath: text, attrName: ai"
      "[rowNum: int]";
  string err11 = "ERROR!Parition mode expects "
      "{nBuckets: int}, {keepPartAttr: bool}";
  string err5 = "ERROR!Type remote nodes expects "
      "[[typeNodeIndex: int], [typeNodeIndex2: int]]";
  string err6 = "ERROR!Data remote nodes expects "
        "[targetNode:int, duplicateTimes: int]";

  string err1 = "ERROR!Infeasible evaluation in TM for attribute ";
  string err2 = "ERROR!The file name should NOT be empty!";
  string err3 = "ERROR!Fail by openning file: ";
  string err7 = "ERROR!Infeasible evaluation in TM for attribute: ";
  string err8 = "ERROR!The slave list file does not exist."
        "Is $PARALLEL_SECONDO_SLAVES correctly set up ?";
  string err9 = "ERROR!Remote node for type file is out of range";
  string err10 = "ERROR!Remote duplicate type file fail.";

  if (l.length() != 5)
    return l.typeError(lenErr);

  NList pType, pValue;

  //First part argument (including stream(tuple(...)) )
  NList attrsList;
  if (!l.first().first().checkStreamTuple(attrsList))
    return l.typeError(typeErr);

  NList bpList = l.second();
  //Basic parameters (including string, text, symbol, [int])
  pType = bpList.first();
  pValue = bpList.second();
  int bpLen = pType.length();

  if (bpLen < 3 || bpLen > 4)
    return l.typeError(err4);

  // File name
  if (!pType.first().isSymbol(CcString::BasicType()))
    return l.typeError(err4);
  NList fnList;
  if (!QueryProcessor::GetNLArgValueInTM(pValue.first(), fnList))
    return l.typeError(err1 + "fileName");
  string filePrefix = fnList.str();
  if (0 == filePrefix.length())
    return l.typeError(err2);

  // File path
  if (!pType.second().isSymbol(FText::BasicType()))
    return l.typeError(err4);
  NList fpList;
  if (!QueryProcessor::GetNLArgValueInTM(pValue.second(), fpList))
    return l.typeError(err1 + "filePath");
  string filePath = fpList.str();

  // Partition attribute
  if (!pType.third().isSymbol())
    return l.typeError(typeErr + "\n" + err4);
  string attrName = pValue.third().convertToString();
  ListExpr attrType;
  int attrIndex = listutils::findAttribute(
      attrsList.listExpr(), attrName, attrType);
  if (attrIndex < 1)
    return l.typeError(attErr + attrName);

  //Optional row number
  if ( bpLen == 4 )
    if (!pType.fourth().isSymbol(CcInt::BasicType()))
      return l.typeError(err4);

  bool evenMode = false;
  bool setKPA = false, KPA = false;
  NList pmList = l.third();
  //Partition mode (including [nBuckets], [KPA])
  pType = pmList.first();
  pValue = pmList.second();
  int pmLen = pType.length();
  if (pmLen < 0 || pmLen > 2)
    return l.typeError(err11);
  if (1 == pmLen)
  {
    if (pType.first().isSymbol(CcInt::BasicType()))
      evenMode = true;
    else if (pType.first().isSymbol(CcBool::BasicType()))
    {
      setKPA = true;
      KPA = pValue.first().boolval();
    }
    else
      return l.typeError(err11);
  }
  else if (2 == pmLen)
  {
    if (!pType.first().isSymbol(CcInt::BasicType()) ||
        !pType.second().isSymbol( CcBool::BasicType()))
      return l.typeError(err11);
    else
    {
      evenMode = true;
      setKPA = true;
      KPA = pValue.second().boolval();
    }
  }

  //Remove the attribute used for partition the relation
  NList newAL; //new attribute list
  if (KPA)
    newAL = attrsList;
  else
  {
    NList rest = attrsList;
    while (!rest.isEmpty())
    {
      NList elem = rest.first();
      rest.rest();
      if (elem.first().str() != attrName)
        newAL.append(elem);
    }
  }

  //Create the type file in local disk
  string typeFileName = filePrefix + "_type";
  filePath = getLocalFilePath(filePath, typeFileName, "");
  ofstream typeFile(filePath.c_str());
  NList resultList =
          NList(NList(Relation::BasicType()),
                NList(NList(Tuple::BasicType()), newAL));
  if (!typeFile.good())
    return l.typeError(err3 + filePath);
  else
  {
    typeFile << resultList.convertToString() << endl;
    cerr << "Created type file " << filePath << endl;
  }
  typeFile.close();

  clusterInfo* ci = 0;
  NList trList = l.fourth();
  pType = trList.first();
  int tNode[2] = {-1, -1};
  if (!pType.isEmpty())
  {
    ci = new clusterInfo();
    if (!ci->isOK())
      return l.typeError(err8);

    //Get the type index and duplicate the type file.
    if (pType.length() > 2)
      return l.typeError(err5);
    while (!pType.isEmpty())
    {
      if (!pType.first().isSymbol(CcInt::BasicType()))
        return l.typeError(err5);
      pType.rest();
    }

    pValue = trList.second();
    int cnt = 0;
    while(!pValue.isEmpty())
    {
      NList nList;
      if (!QueryProcessor::GetNLArgValueInTM(pValue.first(), nList))
        return l.typeError( err7 + " type node index");
      tNode[cnt++] = nList.intval();
      pValue.rest();
    }

    //scp filePath .. IP:loc/typeFileName
    int sLen = ci->getSlaveSize();
    for (int i = 0; i < 2; i++)
    {
      if (tNode[i] >= 0)
      {
        if (tNode[i] > sLen)
        {
          ci->print();
          return l.typeError(err9);
        }
        else
        {
          string rPath = ci->getRemotePath(tNode[i]);
          cerr << "Copy the type file to -> \t" << rPath << endl;
          if (0 != system(
               (scpCommand + filePath + " " + rPath).c_str()))
            return l.typeError(err10);
        }
      }
    }
  }

  NList drList = l.fifth();
  pType = drList.first();
  if (!pType.isEmpty())
  {
    if(pType.length() != 2)
      return l.typeError(err6);
    if (!pType.first().isSymbol(CcInt::BasicType()) ||
        !pType.second().isSymbol(CcInt::BasicType()))
      return l.typeError(err6);
  }

  NList outAttrList =
           NList(NList(NList("Suffix"), NList(CcInt::BasicType())),
                 NList(NList("TupNum"), NList(CcInt::BasicType())));
  NList outList = NList().tupleStreamOf(outAttrList);

  return NList(NList(Symbol::APPEND()),
               NList(
                 NList(attrIndex),
                 NList(
                   NList(NList(
                     Tuple::BasicType()), newAL).convertToString(),
                     true, true)),
               outList).listExpr();
}

/*
8.2 Value mapping

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

      Supplier bspList = args[1].addr,
               ptmList = args[2].addr,
               drpList = args[4].addr;

      relName = ((CcString*)qp->Request(
          qp->GetSupplierSon(bspList,0)).addr)->GetValue();
      path = ((FText*)qp->Request(
          qp->GetSupplierSon(bspList,1)).addr)->GetValue();

      int rowNum = -1;
      int bspLen = qp->GetNoSons(bspList);
      if (4 == bspLen)
        rowNum = ((CcInt*)qp->Request(
            qp->GetSupplier(bspList,3)).addr)->GetValue();

      bool evenMode = false, kpa = false;
      int nBucket = 0;
      int ptmLen = qp->GetNoSons(ptmList);
      if (1 == ptmLen)
      {
        ListExpr ptList = qp->GetType(qp->GetSupplierSon(ptmList,0));
        if (nl->IsEqual(ptList, CcBool::BasicType()))
          kpa = ((CcBool*)qp->Request(
              qp->GetSupplierSon(ptmList,0)).addr)->GetValue();
        else
        {
          evenMode = true;
          nBucket = ((CcInt*)qp->Request(
              qp->GetSupplierSon(ptmList,0)).addr)->GetValue();
        }
      }
      else if (2 == ptmLen)
      {
        evenMode = true;
        nBucket = ((CcInt*)qp->Request(
            qp->GetSupplierSon(ptmList,0)).addr)->GetValue();
        kpa = ((CcBool*)qp->Request(
            qp->GetSupplierSon(ptmList,1)).addr)->GetValue();
      }

      int attrIndex =
            ((CcInt*)args[5].addr)->GetValue() - 1;

      string inTupleTypeStr =
               ((FText*)args[6].addr)->GetValue();
      ListExpr inTupleTypeList;
      nl->ReadFromString(inTupleTypeStr, inTupleTypeList);
      inTupleTypeList = sc->NumericType(inTupleTypeList);

      int drpLen = qp->GetNoSons(drpList);
      int dupTgtIndex = -1, dupTimes = -1;
      if (2 == drpLen)
      {
        dupTgtIndex = ((CcInt*)qp->Request(
            qp->GetSupplierSon(drpList, 0)).addr)->GetValue();
        dupTimes    = ((CcInt*)qp->Request(
            qp->GetSupplierSon(drpList, 1)).addr)->GetValue();
      }

      fdli = (FDistributeLocalInfo*) local.addr;
      if (fdli) delete fdli;
      ListExpr resultTupleList = GetTupleResultType(s);
      fdli = new FDistributeLocalInfo(
               relName, rowNum, path, nBucket, attrIndex, kpa,
               resultTupleList, inTupleTypeList,
               dupTgtIndex, dupTimes);
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
      if (!fdli->startCloseFiles())
        return CANCEL;
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

/*
8.3 Implementation of FDistributeLocalInfo methods

*/

FDistributeLocalInfo::FDistributeLocalInfo(
    string _bn, int _rn, string _pt, int _nb, int _ai, bool _kpa,
    ListExpr _rtl, ListExpr _itl,
    int _di, int _dt)
: nBuckets(_nb), attrIndex(_ai), kpa(_kpa), tupleCounter(0),
  /*rowNumSuffix(""),*/
  rowNumSuffix(-1), firstDupTarget(_di), dupTimes(_dt),
  localIndex(0), cnIP(""),
  ci(0), copyList(0)
{
  string fnSfx = "";

  if ( _rn >= 0 ){
    rowNumSuffix = _rn;
    fnSfx = "_" + int2string(_rn);
  }
  fileBaseName = _bn;
  filePath = getLocalFilePath(_pt, _bn, fnSfx, false);
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
    fp = new fileInfo(fileSfx, filePath, fileBaseName,
        exportTupleType->GetNoAttributes(), rowNumSuffix);
    fileList.insert(pair<size_t, fileInfo*>(fileSfx, fp));
  }
  ok = openFile(fp);

  if (ok)
  {
    if (!fp->writeTuple(tuple, tupleCounter,exportTupleType,
        attrIndex, kpa))
    {
      cerr << "Block file " << fp->getFilePath()
          << " write fail." << endl;
      ok = false;
    }
    else
    {
      tupleCounter++;
      tuple->DeleteIfAllowed();
    }
  }
  return ok;
}

bool FDistributeLocalInfo::openFile(fileInfo* tgtFile)
{
  if (tgtFile->isFileOpen())
    return true;

  if (openFileList.size() >= MAX_OPENFILE_NUM)
  {
    //sort fileInfos according to their last tuples' indices
    sort(openFileList.begin(), openFileList.end(), compFileInfo);
    //The last one of the vector is the idler
    bool poped = false;
    while(!poped && openFileList.size() > 0)
    {
      fileInfo* oldestFile = openFileList.back();
      if (oldestFile)
      {
        if (oldestFile->isFileOpen())
        {
          oldestFile->closeFile();
          poped = true;
        }
      }
      openFileList.pop_back();
    }
  }

  bool ok = tgtFile->openFile();
  if (ok){
    // Only opened file are inserted into list
    openFileList.push_back(tgtFile);
  }
  return ok;
}

bool FDistributeLocalInfo::startCloseFiles()
{
  fit = fileList.begin();

  if (dupTimes > 0)
  {
    ci = new clusterInfo();
    if(!ci->isOK())
    {
      cerr << "ERROR!The slave list file does not exist."
      "Is $PARALLEL_SECONDO_SLAVES correctly set up ?" << endl;
      return false;
    }

    if(firstDupTarget > (int)ci->getSlaveSize() )
    {
      cerr << "The first target node index is "
          "out of the range of slave list" << endl;
      return false;
    }

    int cLen = ci->getClusterSize();
    copyList = new bool[cLen];
    memset(copyList, false, cLen);
    int ti = firstDupTarget;
    for (int i = 0; i < dupTimes; i++, ti++)
      copyList[ ti % cLen ] = true;
    //Synchronize the copy status of master node
    if ( (ci->getMasterNode() != 0) &&
         (copyList[0] || copyList[ci->getMasterNode()] ))
      copyList[0] = copyList[ci->getMasterNode()] = true;

    localIndex = ci->getLocalNode();
    if (localIndex < 0)
    {
      cerr << "ERROR! Cannot find the local position " << endl
          << ci->getLocalIP() << ":" << ci->getLocalPath() << endl
          << "in the slave list, backup files will fail." << endl;
      ci->print();
      return false;
    }
    cnIP = ci->getIP(localIndex);
  }

  return true;
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

    if (!duplicateOneFile(fp))
    {
      tuple->DeleteIfAllowed();
      return 0;
    }
    fit++;
  }
  return tuple;
}

bool FDistributeLocalInfo::duplicateOneFile(fileInfo* fi)
{
  //Duplicate a file after close it.
  //if the duplicating goes wrong, it can tell the stream to stop.

  if (copyList)
  {
    if (fi->isFileOpen())
      fi->closeFile();
    string filePath = fi->getFilePath();
    int cLen = ci->getClusterSize();
    bool keepLocal = false;
    for (int i = 0; i < cLen; i++)
    {
      if (copyList[i])
      {
        if (( localIndex == i) ||  //slave
            ( (0 == i) &&          //master
               (localIndex == ci->getMasterNode())))
        {
          keepLocal = true;
          continue;
        }
        else
        {
          string rPath =
              ci->getRemotePath(i,
                  true, // include master
                  true, // round
                  true, // attachProducerIP
                  true, // attach file name
                  (fi->getFileName()),
                  true // attachIP
                  );
          int copyTimes = MAX_COPYTIMES;
          while(copyTimes-- > 0)
          {
            if ( 0 == system(
                (scpCommand + filePath + " " + rPath).c_str())){
              break;
            }else{
            cerr << "Warning! Duplicate file "
                << filePath << " fail."
                << strerror(errno) << endl;
            }
          }
          if (copyTimes <= 0)
          {
            cerr << "Error! Duplicate remote file fail." << endl;
            return false;
          }
        }
      }
    }
    if (!keepLocal)
    {
      if ( 0 != (system(("rm " + filePath).c_str())))
      {
        cerr << "Delete local file " << filePath << " fail.\n";
        return false;
      }
      cerr << "Local file " << filePath << " is deleted.\n";
    }

  }
  return true;
}

Operator fdistributeOp(FDistributeInfo(),
                       FDistributeValueMap,
                       FDistributeTypeMap);

/*
9 Data Type fList

During the parallel processing, we need to indicate Secondo objects
distributed in slave nodes of the cluster. There are two situations
need to be considered:

  * Object's pieces are kept in Secondo databases, with a same name.

  * Object's pieces are kept in a series of binary files, start with a same name.

The fList is used to process both situations.

Assume a Secondo relation is divided to ~r~ * ~c~ a matrix relation,
with ~r~ rows and ~c~ columns. All part(cell) inside this metrix
are all either stored in slaves' Secondo databases,
or exported as data and type files, kept in slaves' file systems.
If they are kept in database system, it must be a  ~n~ * 1 matrix,
~n~ is the number of the slaves,
as each slave database only has one part of the object with a same name.
If the data are kept in partition files, then each slave may contain
several rows of partition files.

For each fList, during the procedures with Hadoop jobs,
each map task process one row data in the matrix relation,
and divide it into columns of partition files after the internal processing.
Each reduce task process one column partition files,
and transpose it into one row of the output fList.

A fList contains following variables:

  * objectName. The name of the distributed Secondo object. If data is
distributed as Secondo objects in slaves, then all objects
are named as this value. If data is distributed as partition files,
then all these partition file names start with this value.

  * objectType. The schema of this distributed Secondo Object.
At present, it must be a relation.

  * nodesList. Indicate the locations of slaves where the data are distributed on.
It's used to be set by user manually, but now can be read from
the PARALLEL\_SECONDO\_SLAVES list when the fList is built up at the first time,
and then is kept independently in the fList.

While reading an exist fList, and its kept nodesList is different from the
current slave list, then it will be marked as unavailable.
The master node doesn't take part in the distribution of data.

  * fileLocList. Indicate the location of the objects.

    * [<] READ DB / [>] : Here indicate the object is distributed as Secondo Objects in every slave Secondo databases.

    * [<] file path [>] : Here indicate the file location of a partition file, and it must be a absolute file path.

    * null value : Here indicate the partition file is kept in the default file path, which is defined in master or slave list.

  * duplicateTimes. Indicate the duplication times for each partition file.
If data are distributed as Secondo objects, then this value must be 1.

  * Available. Denotes whether the last operator which created this list is successfully performed.

  * Distributed. A boolean used to indicate whether data are distributed.

The *Distributed* variable only can be set by operators like ~spread~
which create a flist data, together with data files.
When a flist is undistributed, then it cannot be read data from it,
like what the operator ~collect~ does.


Update at 6th Feb. 2012
For building up the generic parallel Secondo,
a fList object should be used to describe three different kinds of objects:

  * DGO : Distributed global object. Its value is kept on the master node only.

  * DLO : Distributed local object. Its value is distributed on slaves, as Secondo objects.

  * DLF : Distributed local file list. Its value is distributed on slaves, as disk files, only for relation.

Update at 19th Mar. 2012

Remove the DGO kind, since I decided to not create special flist object,
only for pointing out some objects that have already been created is a flist.
Besides, I also need to remove two useless attributes from the value list:

  * available:  when the objectKind field is set as UNDEF kind
or the distributed field is set as false, then available is false, or else it is true.

  * inDB: it can be replaced by the objectKind too.

*/
fList::fList(string _on, NList _tl, clusterInfo* _ci,
    NList _fll, size_t _dp, size_t _mrn, size_t _mcn,
    bool _idd, fListKind _fkd ):
    subName(_on), objectType(_tl),
    interCluster(new clusterInfo(*_ci)),
    fileLocList(_fll),
    dupTimes(_dp),
    mrNum(_mrn), mcNum(_mcn),
    isDistributed(_idd),
    objKind(_fkd)
{}

fList::fList(fList& rhg):
    subName(rhg.getSubName()),
    objectType(rhg.objectType),
    interCluster(new clusterInfo(*rhg.interCluster)),
    fileLocList(rhg.getLocList()),
    dupTimes(rhg.getDupTimes()),
    mrNum(rhg.getMtxRowNum()),
    mcNum(rhg.getMtxColNum()),
    isDistributed(rhg.isDistributed),
    objKind(rhg.objKind)
{}


/*
9.1 fList::In Function

The ~In~ function accepts following parameters

  * A Secondo object name.
This is a string value express the name of an exist Secondo object.
We use it to get the ~objectName~ and its type expression.
In some cases, the object may not exist in the current database,
then it must has been exported into the file system,
and its type file must be kept in the local *parallel* directory.
If neither the object or the type file exists,
then set the ~correct~ as FALSE.

  * A nodesList.
This is a list of string values, each specifies a IP address of
a node in the cluster. And the first one is viewed as the master
node by default.

  * A fileLocMatrix
This is a nested list that composed by integer numbers,
which denotes the matrix of cell files
E.g., it may looks like this:

---- (  (1 ( 1 2 3 4 5))
        (2 ( 1 2 3 4 5))
        (3 ( 1 2 3 4 5))
        (4 ( 1 2 3 4 5)) )
----

The above example shows that a Secondo objects is divided into
a 4x5 matrix file, and is distributed to a cluster with 4 nodes.
Each node, including the master node, contains five cell files.

  * A duplicate times
This is a integer number used to tell how many duplications of a
cell file are kept inside the cluster.
At present, we adopt a simple chained declustering mechanism to
backup the duplications of the cell files.
Besides the primary node that is denoted in the fileLoc matrix,
it will be copied to (~dupTimes~ - 1) nodes that are listed after
the primary node within the nodesList.

Update at 26/12/2011
The ~nodesList~  is set up at the first time when the fList is built,
by creating a clusterInfo object, and doesn't need to be manually indicated.
After reading, the node list is kept inside the fList,
in case it need to be reloaded into another database.
Therefore it also can be manually indicated,
but the given nodelist must be a subset of the current node list,
so as to keep a fList object while the cluster scale increases.

In clusters like ours, that each node contains two hard disks,
and contains two independent miniSecondo databases,
these databases will be viewed as different slaves inside the cluster.

For the fileLocMatrix, it's possible that one slave may contains
several rows of files, some partition files may don't exist.
All partition files belong to one row must be stored at one slave,
and also in one file path, which is indicated as the last text of each row.
The row number of these partition files have nothing to do with the
slave nodes that store these files.
Hence it may looks like:

----(  (2 (1 2 3 4 5) '')
       (2 (1 2 3 4 5) '')
       (1 (1 2 5)     '\/mnt\/diskb')
       (4 (1 2 3 4 5) '') )
----

The above example also shows a 4x5 matrix relation,
distributed on a cluster at least has 4 nodes.
Each row data has been partitioned into at most 5 pieces.
The 2th node has two rows partition files, while the 3th node doesn't have any one of it.
And in the 1th node, the third and fourth column partition files don't exist.
The first row partition files are kept in the 2th node,
while the third row partition files are kept in the 1th node.
All files are kept in slaves' default parallel location,
except the third row, which are kept in a specific path of 1th node.

Update at 01/09/12

As some relation may cannot produce partitions for all rows,
we left an empty row inside the fileLocMatrix,
to denote a row without any partition files.
Hence now the fileLocMatrix should looks like:

----(  (1 (1 2 3 4 5) '')
       (2 (1 2 3 4 5) '')
       (1 () '')
       (2 (1 2 5)     '\/mnt\/diskb')
       (1 (1 2 3 4 5) '') )
----

Here the example indicates a 5x5 matrix relation,
distributed on a cluster with 2 slaves.
The third row is an empty row, it's column list is empty,
since there is no partition files produced for this row.

Update at 01/13/12

Since ~flist~ describes the distribution of tuples,
the type of a flist should be changed from simply flist
to flist(tuple(....)).
At the same time, there is no necessary to indicate an exist
object or type file anymore, since the type is given by users.
The anonymous of flist objects are checked by comparing the
type files if they exist.
The type file keeps the schema as tuple relation,
since it may be required by file-relevant operators.

Updated when ~spread2~ operator is created.
The type for DLF kind flist is set as

----
flist(stream(tuple(....)))
----

The type for DLO kind flist with relation type object is set as

----
flist(rel(tuple(....)))
----

So we can distinguish these two different types.

Update 23th Mar. 2012

I disable the ~In~ function of the fList,
since all the fileLocation is set by operators.

But it may still be needed, in case we want to reload
a fList to another cluster.
Therefore, I still keep the code for the old function,
called Backup\_In.

*/


Word fList::Backup_In(const ListExpr typeInfo,
    const ListExpr instance, const int errorPos,
    ListExpr& errorInfo, bool& correct)
{
  Word result = SetWord(Address(0));
  string typeErr = "expect "
      "(fileName [nodesList] fileLocList "
      "dupTimes isInDB [isDistributed]) ";
  string flocErr = "incorrect file location list, "
      "refer to source document.";
  clusterInfo *ci = new clusterInfo();

  NList il(instance);
  correct = true;
  NList onl, nll, fml, dpl,idl, dbl, dkd;
  string objName = "";
  ListExpr objType;
  bool idb = false;
  bool dbd = false;
  int maxRNum, maxCNum;

  if (6 == il.length())
  {
    //Read the complete nestedlist created by the OUT function
    onl = il.first();   //object name
    nll = il.second();  //node list
    fml = il.third();   //fileloc list
    dpl = il.fourth();  //duplicate times
    dbl = il.fifth();   //data are distributed
    dkd = il.sixth();   //data kind
  }
  else if (4 == il.length())
  {
    //Set by users manually
    onl = il.first();
    fml = il.second();
    dpl = il.third();
//    idl = il.fourth();
  }
  else{
    cerr << "The current list length is: " << il.length() << endl;
    cmsg.inFunError(typeErr);
    correct = false;
  }


/*
It's not allowed to create reduplicated flist inside a cluster,
hence each time when a new flist is created,
it checks whether exists a homonymous type file with a different type list.
If it does, then abort the creation, or else create the type file.

*/


  if (correct)
  {
    if (!onl.isString())
    {
      cmsg.inFunError(typeErr);
      correct = false;
    }
    else
    {
      objType = AntiNumericType(typeInfo);
      objName = onl.str();
      if (0 == objName.length())
      {
        cmsg.inFunError("Object Name cannot be empty.");
        correct = false;
      }
      else
      {
        string typeFilePath = getLocalFilePath("", objName, "_type");
        ListExpr inType = nl->Second(objType);
        if (FileSystem::FileOrFolderExists(typeFilePath))
        {
          //type file exists, compare the tuple type
          ListExpr exeType;
          bool ok = false;
          if (nl->ReadFromFile(typeFilePath, exeType)){
              if (nl->Equal(exeType,inType)){
                ok = true;
              }
          }
          if (!ok){
            cmsg.inFunError(
                "Homonymous type file in " + typeFilePath);
            correct = false;
          }
        }
        else
        {
          //type file not exists, write the tuple type
          if (nl->IsAtom(inType)){
            inType = nl->OneElemList(inType);
          }
          if (!nl->WriteToFile(typeFilePath, inType))
          {
            cmsg.inFunError("Write object Type to file " +
                typeFilePath + " failed! ");
            correct = false;
          }
        }
      }
    }
  }

  //Check nodesList
  if (correct && !nll.isEmpty())
  {

/*
If a fList is imported from another cluster,
then the old cluster must be a subset of the current one.

*/


    if (!ci->covers(nll))
    {
      cmsg.inFunError("Expect a subset of the current cluster.");
      correct = false;
    }
  }

  //Check the type of the fileLocList
  if (correct)
  {
    if (fml.isEmpty())
      idb = true;
    else
    {
      bool isOK = true;
      NList rows = fml;

      // Check the type of the list
      while (!rows.isEmpty())
      {
        NList aRow = rows.first();
        if (3 == aRow.length())
        {
          if (!aRow.first().isInt()){
            cerr << "Each row number expects integer" << endl;
            isOK = false;
            break;
          }

          NList CFs = aRow.second();
          bool crow = true;  //Correctness of a row
          while(!CFs.isEmpty())
          {
            NList aCF = CFs.first();
            if (!aCF.isInt()){
              crow = false;
              break;
            }
            CFs.rest();
          }
          if (!crow){
            cerr << "Each column number expects integer" << endl;
            isOK = false;
            break;
          }

          NList PFP = aRow.third();
          if(!PFP.isText()){
            cerr << "Each file path expects text" << endl;
            isOK = false;
            break;
          }
        }
        else{
          cerr << "Each row expects three elements "
              "(slaveIndex, columnList, filePath)" << endl;
          isOK = false;
          break;
        }
        rows.rest();
      }
      if (isOK){
//        isOK = verifyLocList(fml, maxRNum, maxCNum);
      }

      if (!isOK){
        cmsg.inFunError(flocErr);
        correct = false;
      }
    }
  }

  //Check the duplicate times
  int dpTime = 0;
  if (correct)
  {
    correct = false;
    if (dpl.isInt())
    {
      dpTime = dpl.intval();
      if ( (idb && dpTime == 1) || (!idb && dpTime >= 1)){
        correct = true;
      }
    }

    if (!correct)
      cmsg.inFunError(
          "Expect a positive integer duplicate times value.");
  }

  //Check whether data are kept in databases

  if (correct)
  {
    if (idl.isBool())
    {
      idb = idl.boolval();
    }
    else
    {
      correct = false;
      cmsg.inFunError("Expect a bool value for inDB.");
    }
  }


  //Check whether data have been distributed.
  if (correct && !dbl.isEmpty())
  {
    if (dbl.isBool()){
      dbd = dbl.boolval();
    }
    else{
      correct = false;
      cmsg.inFunError("Expect a bool value for Distributed.");
    }
  }

  //Check whether the object kind is set
  fListKind objKind = UNDEF;
  if (correct && !dkd.isEmpty())
  {
    string kindErr = "Expect data kind of DGO, DLO or DLF";
    if (!dkd.isAtom())
    {
      correct = false;
      cmsg.inFunError(kindErr);
    }
    else
    {
      if (!dkd.isInt()){
        correct = false;
        cmsg.inFunError(kindErr);
      }
      else{
        objKind = (fListKind)dkd.intval();
        if (objKind < UNDEF || objKind > DLF){
          correct = false;
          cmsg.inFunError(kindErr);
        }
      }
    }
  }

  if (correct)
  {
    fList *FLL = new fList(objName, NList(objType),
        ci, fml, dpTime, maxRNum, maxCNum);

    return SetWord(FLL);
  }

  return SetWord(Address(0));
}


Word fList::In(const ListExpr typeInfo, const ListExpr instance,
            const int errorPos, ListExpr& errorInfo, bool& correct)
{
  correct = false;
  return SetWord(Address(0));
}

ListExpr fList::Out(ListExpr typeInfo, Word value)
{
  if (value.addr)
  {
    fList* fl = static_cast<fList*>(value.addr);
    NList outList;

    outList.append(NList(fl->getSubName(), true, false));
    outList.append(fl->getNodeList());
    outList.append(fl->getLocList());
    outList.append(NList(fl->getMtxRowNum()));
    outList.append(NList(fl->getMtxColNum()));
    outList.append(NList(fl->getDupTimes()));
    outList.append(NList(fl->isDistributed, false));
    outList.append(NList(fl->objKind));
    return outList.listExpr();
  }
  else
    return nl->SymbolAtom("undefined");
}

Word fList::Create(const ListExpr typeInfo)
{
  return SetWord(
    new fList("", NList(),new clusterInfo(), NList(), 1));
}

void fList::Delete(const ListExpr typeInfo, Word& w)
{
  fList* data = (fList*)w.addr;

  if (data)
  {
    string objName = data->getSubName();
    string typeFilePath = getLocalFilePath("", objName, "_type");
    if (FileSystem::FileOrFolderExists(typeFilePath)){
      FileSystem::DeleteFileOrFolder(typeFilePath);
    }
    delete data;
  }
  w.addr = 0;
}

void fList::Close(const ListExpr typeInfo, Word& w)
{
  delete (fList*)w.addr;
  w.addr = 0;
}


Word fList::Clone(const ListExpr typeInfo, const Word& w)
{
  return SetWord(new fList(*(fList*)w.addr));
}

bool fList::Save(SmiRecord& valueRecord, size_t& offset,
                 const ListExpr typeInfo, Word& w)
{
  bool ok = true;

  ListExpr valueList = Out(typeInfo, w);
  valueList = nl->OneElemList(valueList);
  string valueStr;
  nl->WriteToString(valueStr, valueList);
  int valueLength = valueStr.length();
  ok = ok && valueRecord.Write(&valueLength, sizeof(int), offset);
  offset += sizeof(int);
  ok = ok && valueRecord.Write(valueStr.data(), valueLength, offset);
  offset += valueLength;

  return ok;
}

bool fList::Open(SmiRecord& valueRecord,
                 size_t& offset,
                 const ListExpr typeInfo,
                 Word& value)
{
  int valueLength;
  string valueStr = "";
  ListExpr valueList = 0;
  char *buf = 0;
  ListExpr errorInfo = nl->OneElemList(nl->SymbolAtom("ERRORS"));
  bool correct;

  bool ok = true;
  ok = ok && valueRecord.Read(&valueLength, sizeof(int), offset);
  offset += sizeof(int);
  buf = new char[valueLength];
  ok = ok && valueRecord.Read(buf, valueLength, offset);
  offset += valueLength;
  valueStr.assign(buf, valueLength);
  delete []buf;
  nl->ReadFromString(valueStr, valueList);

  value = RestoreFromList(typeInfo, nl->First(valueList),
      1, errorInfo, correct);

  if (errorInfo != 0)
    nl->Destroy(errorInfo);
  nl->Destroy(valueList);
  return ok;
}

Word fList::RestoreFromList(
    const ListExpr typeInfo, const ListExpr instance,
    const int errorPos, ListExpr& errorInfo, bool& correct )
{
  NList il = NList(instance);
  string objName = il.first().str();
  NList typeList = NList(AntiNumericType(typeInfo));
  NList nodeList = il.second();
  NList locList = il.third();
  size_t maxRNum = il.fourth().intval();
  size_t maxCNum = il.fifth().intval();
  size_t dupTimes = il.sixth().intval();
  bool distributed = il.seventh().boolval();
  fListKind kind = (fListKind)il.eigth().intval();

  clusterInfo *ci = new clusterInfo();
  fList* fl = 0;
  if (ci->covers(nodeList))
  {
    fl = new fList(objName, typeList, ci, locList, dupTimes,
        maxRNum, maxCNum, distributed, kind);
    return SetWord(fl);
  }
  else{
    correct = false;
    return SetWord(Address(0));
  }
}

bool CheckFList(ListExpr type, ListExpr& errorInfo)
{
  if (  (nl->ListLength(type) == 2)
      &&(nl->IsEqual(nl->First(type), fList::BasicType()))){
    return true;
  }
  else{
    cmsg.otherError("FList Data Type Check Fails!");
    return false;
  }
}

/*
The ~In~ function only checks the type of the fileLocList,
but doesn't check the availability of the matrix relation.
This function checks whether the value of the flist is available,
by checking following conditions:

  * each slave index must be less than nodesNum

  * each column number must be a positive number

  * duplicate number is less than nodesNum

  * data path is set as [<]READ DB\/[>] while the data are not in database,
or the other way round.

Check the available of a file location list, also get the maximum row and column number.

*/
bool fList::verifyLocList()
{

  if (isAvailable())
    return true;

  if (!fileLocList.isEmpty())
  {
    mrNum = fileLocList.length();
    mcNum = 0;

    NList fll = fileLocList;
    while (!fll.isEmpty())
    {
      NList aRow = fll.first();
      int nodeNum = aRow.first().intval();
      if (nodeNum >= (int)interCluster->getClusterSize())
      {
        cerr << "Improper row number: " << nodeNum << endl;
        return false;
      }
      NList cfList = aRow.second();
      while (!cfList.isEmpty())
      {
        NList aPF = cfList.first();  //A partition file suffix
        int partNum = aPF.intval();
        if (partNum < 1)
        {
          cerr << "Improper column number: " << partNum << endl;
          return false;
        }
        mcNum = (partNum > (int)mcNum) ? partNum : mcNum;
        cfList.rest();
      }

      string dataLoc = aRow.third().str();
      if (dataLoc.length() > 0){
        cerr << "Non-empty data location "
            "while data are kept in databases. " << endl;
        return false;
      }
      else if (dataLoc.compare(dbLoc) == 0){
        cerr << "It's not acceptable that only part data are "
            "kept in databases. " << endl;
        return false;
      }

      fll.rest();
    }

    if ((isInDB() && dupTimes > 1)
        || (dupTimes >= interCluster->getClusterSize()) )
    {
      cerr << "Improper duplication times: " << dupTimes
          << (isInDB() ? " , when data are kept in databases " : "")
          << endl;
      return false;
    }
    return true;
  }
  return true;
}


bool fList::verifyLocList(NList fileLocList, size_t clusterScale,
      int& maxRNum, int& maxCNum)
{
  maxCNum = maxRNum = 0;
  if (!fileLocList.isEmpty())
  {
    maxRNum = fileLocList.length();

    NList fll = fileLocList;
    while (!fll.isEmpty())
    {
      NList aRow = fll.first();
      size_t nodeNum = aRow.first().intval();
      if (nodeNum >= clusterScale )
      {
        cerr << "Improper row number: " << nodeNum << endl;
        return false;
      }
      NList cfList = aRow.second();
      while (!cfList.isEmpty())
      {
        NList aPF = cfList.first();  //A partition file suffix
        int partNum = aPF.intval();
        if (partNum < 1)
        {
          cerr << "Improper column number: " << partNum << endl;
          return false;
        }
        maxCNum = (partNum > (int)maxCNum) ? partNum : maxCNum;
        cfList.rest();
      }

      string dataLoc = aRow.third().str();
/*
If the fileLocList is not empty, then it must belong to DLF kind
If the path value is ~dbLoc~, or is not an absolute path
then it returns false
It is impossible to check the available of these paths,
since they exist on remote machines

*/
      if (dataLoc.length() > 0)
      {
        if (  dataLoc.find('/') != 0
           || dataLoc.compare(dbLoc) == 0){
          cerr << "Improper file path" << dataLoc << endl;
          return false;
        }
      }

      fll.rest();
    }
  }
  return true;
}

size_t fList::getPartitionFileLoc(
    size_t row, vector<string>& locations)
{
  if (!isAvailable()){
    return 0;
  }

  if ( row > mrNum ){
    cerr << "The row (" << row << ":" << mrNum << ") "
        << " is illegal" << endl;
    return 0;
  }

  NList rowLoc = fileLocList.elem(row);
  if (rowLoc.second().length() == 0){
    return 0; //The current row is empty.
  }

  locations.resize(0);
  stringstream ss;
  int ssIndex = rowLoc.first().intval();  // start slave index
  for (size_t i = 0; i < dupTimes; i++){
    string sIPAddr = interCluster->getIP(ssIndex + i, true);
    string dataLoc = rowLoc.third().str();
    if (objKind == DLO){
      dataLoc = dbLoc;
    }
    else if ((dataLoc.length() == 0) || (i > 0)){
      // duplicated files are kept at remote node's default path
      // Only output the remote folder, not complete file path.
      dataLoc = interCluster->getRemotePath(ssIndex + i,
          false, true, false);
    }
    string remotePath = sIPAddr + ":" + dataLoc;
    locations.push_back(remotePath);
  }
  return dupTimes;
}

NList fList::getColumnList(size_t row)
{
  if (!isAvailable() || row > mrNum){
    return NList();
  }
  else{
    return fileLocList.elem(row).second();
  }
}


void fList::appendFileLocList(NList elem)
{
  if (isDistributed){
    //cannot append new loc information to distributed flist
    cerr << "Error! Cannot append new locations to "
        "a distributed flist."
        << endl;
    return;
  }
  fileLocList.append(elem);
}

struct fListInfo: ConstructorInfo
{
  fListInfo()
  {
    name = "flist";
    signature = " ANY -> FLIST ";
    typeExample = "(flist(rel(tuple((PLZ int)(Ort string)))))";
    listRep = "( objName fileLocList dupTimes inDB )";
    valueExample =
      "( \"plz\" ( (1 (1 2) '') (2 (1 2) '') (1 () '') ) 2 FALSE) ";
    remarks = "Describe distributed data over computer clusters.";
  }
};
struct fListFunctions: ConstructorFunctions<fList>
{
  fListFunctions()
  {
    in = fList::In;
    out = fList::Out;

    create = fList::Create;
    deletion = fList::Delete;
    close = fList::Close;
    clone = fList::Clone;
    kindCheck = CheckFList;

    save = fList::Save;
    open = fList::Open;
  }
};

fListInfo fli;
fListFunctions flf;
TypeConstructor flTC(fli, flf);

/*
5 Operator ~spread~

This operator accepts a tuple stream,
distributes tuples into a matrix relation based on their values of
a given attribute and returns a flist.
The map of the ~spread~ operator is:

----
stream(tuple(a1 ... ai ... aj ... an))
x fileName x filePath x [dupTime]
x ai x [scale] x [keepAI]
x [aj] x [scale2] x [keepAJ]
  -> flist
----

All paratition files are kept as disk files in slave nodes,
and their file names are built up as: fileName\_row\_column,
row in [1,Scale],column in[1,Scale2].
Both fileName and filePath parameters are indispensable.
But the filePath can be set as an empty string,
and its default value listed in the SecondoConfig.ini is then used.

For the purpose of fault-tolerance,  each partition file is
duplicated on ~dupTime~ continuous slave nodes,
the default value of ~dupTime~ is 1.
All duplicated files are kepts in nodes' default paths.

By default, a tuple stream is divided into ~sn~ * 1 partition files,
based on the value of the indispensable parameter ~ai~.
The ~sn~ is the number of slave nodes indicated in the PARALLEL\_SECONDO\_SLAVES.
The partition attribute ~ai~ will be removed after the operation, except the ~keepAI~ is set as true.


The ~sn~ can also be replaced by the optional parameter ~scale~ parameter,
and files of each row are distributed into a slave node,
based on the order denoted in the PARALLEL\_SECONDO\_SLAVES.

On each row, the data can be further divided into several column partition files,
if the second key-attribute ~aj~ is given.
The ~aj~ must be different from ~ai~, to avoid producing empty partition files.
The number of columns is decided by the number of values of ~aj~,
and also can be indicated by the optional parameter ~scale2~.
~aj~ also will be removed by default after the operation, except the ~keepAJ~ is set as true.


*/
struct SpreadInfo : OperatorInfo {

  SpreadInfo() : OperatorInfo()
  {
    name = "spread";
    signature = "stream(tuple(a1 ... ai ... aj ... an)) "
                " x string x text x [int] "
                " x ai x [int] x [bool] "
                " x [aj] x [int] x [bool] "
                " -> flist(tuple(a1 ... ai ... aj ... an))";
    syntax = "stream(tuple(a1 ... ai ... aj ... an)) "
            " x fileName x filePath x [dupTime] "
            " x ai x [scale] x [keepAI] "
            " x [aj] x [scale2] x [keepAJ] "
            "  -> flist(tuple(a1 ... ai ... aj ... an))";
    meaning = "This operator accepts a tuple stream, "
        "distributes its tuples into a matrix relation "
        "based on their values of a given attribute "
        "and returns a flist.";
  }

};

/*

5.1 Type Mapping

----
stream(tuple(a1 ... ai ... aj ... an))
x string x text x [int]
x ai x [int] x [bool]
x [aj] x [int] x [bool]
  -> flist(tuple(a1 ... ai ... aj ... an))
----

During the type mapping function, as we use several optional parameters,
hence it's better to divide these parameters into lists.
The specification of this operator is:

----
_ op[ list;list;list]
----

The first list denotes the fileName and filePath, and the optional ~dupTime~.
The second list denotes the first keyAttribute, together with its optional scale.
The third list denotes the optional second keyAttribute, together with its optional scale.

The type mapping function produces the text type file for the result files,
and duplicate it to every node's default pathlisted in PARALLEL\_SECONDO\_SLAVES\/MASTER.

21th Mar. 2012
Set the file name and path as optional parameters too,
they can be set by rules, without always set by users.

Also the type of the output flist becomes flist(stream(tuple(T))),
used to distinguish with the DLO flists for tuple relations,
which have the type of flist(rel(tuple(T)))
Now the map becomes:

----
stream(tuple(a1 ... ai ... aj ... an))
x string x text x [int]
x ai x [int] x [bool]
x [aj] x [int] x [bool]
  -> flist( stream ( tuple (a1 ... ai ... aj ... an)))
----

*/

ListExpr SpreadTypeMap(ListExpr args){

  NList l(args);
  string err[] = {
      // 0
      "ERROR! Operator expects 4 lists arguments.",

      "ERROR! Operator expects (stream(tuple(a1, a2, ..., an)))"
      "x ( [string] x [text] x [int] ) "
      "x ( ai x [int] x [bool] ) "
      "x ( [aj] x [int] x [bool] )",

      "ERROR! Infeasible evaluation in TM for attribute: ",

      "ERROR! Unavailable file name: ",

      "ERROR! Operator cannot find the dividing attribute: ",

      // 5
      "ERROR! Two keyAttributes must be different from each other.",

      "ERROR! The result stream tuple type cannot be empty.",

      "ERROR! Cannot create homonymous flists. ",

      "ERROR! Cannot open file at ",

      "ERROR! PARALLEL_SECONDO_SLAVES/MASTER is not set up. ",

      // 10
      "ERROR! Remote copy fails to path: ",

      "ERROR! This Secondo database is not listed inside "
      "PARALLEL_SECONDO_SLAVES/MASTER "
  };
  bool keepAI = false, keepAJ = false;

  if (l.length() != 4)
    return l.typeError(err[0]);

  NList pType, pValue;

  //First list, stream(tuple())
  string fileName = "", filePath = "";
  NList attrList;
  if (!l.first().first().checkStreamTuple(attrList)){
    return l.typeError(err[1]);
  }
  NList inStream = l.first().second();

  //Second list, ([string] [text] [int] )
  NList bpList = l.second();  //basic parameters
  pType = bpList.first();
  pValue = bpList.second();
  if (pType.length() > 3){
    return l.typeError(err[1]);
  }

  int len = pType.length();
  for (int i = 1 ; i <= len ; i++)
  {
    NList pp = pType.elem(i);
    NList pv = pValue.elem(i);

    if (pp.isSymbol(CcString::BasicType()))
    {
      //Set the file name
      NList fnList;
      if (!QueryProcessor::GetNLArgValueInTM(pv, fnList)){
        return l.typeError(err[2] + "fileName");
      }
      fileName = fnList.str();
    }
    else if (pp.isSymbol(FText::BasicType()))
    {
      //Set the file path
      NList fpList;
      if (!QueryProcessor::GetNLArgValueInTM(pv, fpList)){
        return l.typeError(err[2] + "filePath");
      }
      filePath = fpList.str();
    }
    else if (!pp.isSymbol(CcInt::BasicType())){
      return l.typeError(err[1]);
    }
  }

  //Third list, (keyAttr1 [int] [bool])
  NList scList1 = l.third(); //scale list 1
  pType = scList1.first();
  pValue= scList1.second();
  if (pType.length() < 1 || pType.length() > 3){
    return l.typeError(err[1]);
  }
  if (!pType.first().isSymbol()){
    return l.typeError(err[1]);
  }
  string keyAI = pType.first().convertToString();
  ListExpr attrType;
  int attrIndex1 =
    listutils::findAttribute(attrList.listExpr(), keyAI, attrType);
  if (attrIndex1 < 1){
    return l.typeError(err[4] + keyAI);
  }

  if (pType.length() == 2){
    if (pType.second().isSymbol(CcBool::BasicType())){
      keepAI = pValue.second().boolval();
    }
    else if (!pType.second().isSymbol(CcInt::BasicType())){
      return l.typeError(err[1]);
    }
  }
  else if (pType.length() == 3){
    if (pType.second().isSymbol(CcInt::BasicType())
        && pType.third().isSymbol(CcBool::BasicType())){
      keepAI = pValue.third().boolval();
    }
    else{
      return l.typeError(err[1]);
    }
  }

  //Fourth list, ([keyAttr2] [int] [bool])
  NList scList2 = l.fourth(); //scale list 1
  pType = scList2.first();
  pValue= scList2.second();
  string keyAJ = "";
  int attrIndex2 = -1;
  if (pType.length() > 3){
    return l.typeError(err[1]);
  }
  if (pType.length() > 0){

    if (!pType.first().isSymbol()){
      return l.typeError(err[1]);
    }
    keyAJ = pType.first().convertToString();
    attrIndex2 =
      listutils::findAttribute(attrList.listExpr(), keyAJ, attrType);
    if (attrIndex2 < 1){
      return l.typeError(err[4] + keyAJ);
    }
    else if (attrIndex2 == attrIndex1){
      return l.typeError(err[5]);
    }

    if (pType.length() == 2){
      if (pType.second().isSymbol(CcBool::BasicType())){
        keepAJ = pValue.second().boolval();
      }
      else if (!pType.second().isSymbol(CcInt::BasicType())){
        return l.typeError(err[1]);
      }
    }
    else if (pType.length() == 3){
      if (pType.second().isSymbol(CcInt::BasicType())
          && pType.third().isSymbol(CcBool::BasicType())){
        keepAJ = pValue.third().boolval();
      }
      else{
        return l.typeError(err[1]);
      }
    }
  }

  NList newAttrList;
  if (keepAI && keepAJ){
    newAttrList = attrList;
  }
  else{
    NList rest = attrList;
    while (!rest.isEmpty()){
      NList elem = rest.first();
      if (   ((elem.first().str() != keyAI) || (keepAI))
          && ((elem.first().str() != keyAJ) || (keepAJ)) ){
        newAttrList.append(elem);
      }
      rest.rest();
    }
  }
  if (newAttrList.length() == 0){
    return l.typeError(err[6]);
  }
  //Create the type file
  if (fileName.length() == 0)
    fileName = fList::tempName(false);
  NList resultList =
      NList(NList(fList::BasicType(),
            NList(NList(Stream<Tuple>::BasicType()),
            NList(NList(Tuple::BasicType()), newAttrList))));
  filePath = getLocalFilePath(filePath,
      (fileName + "_type"), "", true);
  if (FileSystem::FileOrFolderExists(filePath)){
    ListExpr exeType;
    bool ok = false;
    if (nl->ReadFromFile(filePath, exeType)){
      if (listutils::isTupleStream(exeType)){
        if (nl->Equal(exeType, nl->Second(resultList.listExpr()))){
          ok = true;
        }
      }
    }
    if (!ok)
      return l.typeError(err[7] + filePath);
  }
  else{
    ListExpr expList = nl->Second(resultList.listExpr());
    if (!nl->WriteToFile(filePath, expList)){
      return l.typeError(err[8] + filePath);
    }
  }

  //Duplicate the type to master and all slave nodes
  clusterInfo* ci = new clusterInfo();
  if (!ci->isOK()){
    return l.typeError(err[9]);
  }
  if (ci->getLocalNode() < 0){
    return l.typeError(err[11]);
  }
  string masterPath;
  if (!ci->isLocalTheMaster()){
    masterPath = ci->getRemotePath(0);
    if ( 0 != system(
        (scpCommand + filePath + " " + masterPath).c_str())){
      return l.typeError(err[10] + masterPath);
    }
  }
  for (size_t i = 1; i <= ci->getSlaveSize(); i++){
    //Copy the type file to every slave
    string rPath = ci->getRemotePath(i, false);
    if ( 0 != system(
        (scpCommand + filePath + " " + rPath).c_str())){
      return l.typeError(err[10] + rPath);
    }
  }
  return NList(NList(Symbol::APPEND()),
               NList(NList(attrIndex1),
                     NList(attrIndex2),
                     NList(fileName,true, false)),
               resultList).listExpr();
}

/*
5.2 Value Mapping

Each partition inside the flist produced by ~spread~ operator
is also composed by two files, the type file and the data file.
All partition files in a same row, i.e. kept in a same slave node, share a same type file.

Both kinds files are as same as the files created by ~fconsume~ or ~fdistribute~ files,
so that they can be read by using ~ffeed~ operator as normal.
The reason is that, during the parallel processing,
the slave nodes don't contain the flist object,
but only read files from their local or neighbors' disks.

The master node deonted by PARALLEL\_SECONDO\_MASTER must contain the type file,
in order to avoid dirty data by checking these type files' names on the master node.
Every slave node has one type file. If one slave node has several rows partition files,
then all rows share a same type file.

where to put the data file? Partition files are produced at the disk where the spread operation is executed,
and then is copied to target after the production is finished.

*/
int SpreadValueMap(Word* args, Word& result,
    int message, Word& local, Supplier s){

  SpreadLocalInfo *lif = 0;

  if ( message <= CLOSE )
  {
    //Get the parameters
    result = qp->ResultStorage(s);
    Supplier bspList = args[1].addr,
             partList1 = args[2].addr,
             partList2 = args[3].addr;
    string fileName = "", filePath = "";
    int keyIdxI = -1, keyIdxJ = -1;
    int scaleI = 0, scaleJ = 0;
    bool keepAI = false, keepAJ = false;
    size_t dupTimes = 1;

    keyIdxI = ((CcInt*)args[4].addr)->GetValue() - 1;
    keyIdxJ = ((CcInt*)args[5].addr)->GetValue() - 1;

    int blLen = qp->GetNoSons(bspList);
    for (int i = 0; i < blLen; i++)
    {
      ListExpr pp = qp->GetType(qp->GetSupplierSon(bspList,i));
      if (nl->IsEqual(pp, CcString::BasicType())){
        //File Name is set
        fileName = ((CcString*)qp->Request(
            qp->GetSupplierSon(bspList, i)).addr)->GetValue();
      }
      else if (nl->IsEqual(pp, FText::BasicType())){
        //File Path is set
        filePath = ((FText*)qp->Request(
            qp->GetSupplierSon(bspList, i)).addr)->GetValue();
      }
      else if (nl->IsEqual(pp, FText::BasicType())){
        //duplicate time is set
        dupTimes = ((CcInt*)qp->Request(
            qp->GetSupplier(bspList,i)).addr)->GetValue();
      }
    }
    if (fileName.length() == 0){
      //The file name is not set
      fileName = ((CcString*)args[6].addr)->GetValue();
    }

    int plLen1 = qp->GetNoSons(partList1);
    if (plLen1 == 2){
      ListExpr argType = qp->GetType(qp->GetSupplier(partList1,1));
      if (nl->IsEqual(argType, CcBool::BasicType())){
        keepAI = ((CcBool*)qp->Request(
            qp->GetSupplier(partList1, 1)).addr)->GetValue();
      }
      else{
        scaleI = ((CcInt*)qp->Request(
            qp->GetSupplier(partList1, 1)).addr)->GetValue();
      }
    }
    else if (plLen1 == 3){
      scaleI = ((CcInt*)qp->Request(
          qp->GetSupplier(partList1, 1)).addr)->GetValue();
      keepAI = ((CcBool*)qp->Request(
          qp->GetSupplier(partList1, 2)).addr)->GetValue();
    }
    int plLen2 = qp->GetNoSons(partList2);
    if (plLen2 == 2){
      ListExpr argType = qp->GetType(qp->GetSupplier(partList2,1));
      if (nl->IsEqual(argType, CcBool::BasicType())){
        keepAJ = ((CcBool*)qp->Request(
            qp->GetSupplier(partList2, 1)).addr)->GetValue();
      }
      else{
        scaleJ = ((CcInt*)qp->Request(
            qp->GetSupplier(partList2, 1)).addr)->GetValue();
      }
    }
    else if (plLen2 == 3){
      scaleJ = ((CcInt*)qp->Request(
          qp->GetSupplier(partList2, 1)).addr)->GetValue();
      keepAJ = ((CcBool*)qp->Request(
          qp->GetSupplier(partList2, 2)).addr)->GetValue();
    }
      lif = (SpreadLocalInfo*)local.addr;
      if (lif) delete lif;
      lif = new SpreadLocalInfo(fileName, filePath, dupTimes,
         keyIdxI, scaleI, keepAI, keyIdxJ, scaleJ, keepAJ);
      if (!lif->isAvailable()){ return 0; }
      local.setAddr(lif);

      Word wTuple(Address(0));
      qp->Open(args[0].addr);
      qp->Request(args[0].addr, wTuple);
      while (qp->Received(args[0].addr))
      {
        if (!lif->insertTuple(wTuple)){
          cerr << "Inserting tuple to files fail. " << endl;
          break;
        }
        qp->Request(args[0].addr, wTuple);
      }

      if (lif->closeAllPartFiles()){
        result.addr = new fList(*(lif->getResultList()));
      }
      else{
        cerr << "Closing partition files fails" << endl;
      }

      delete lif;
      local.setAddr(0);
  }
//TODO add the progress estimation in the future
//  else if ( message == REQUESTPROGRESS )
//  else if ( message == CLOSEPROGRESS )


  return 0;
}

SpreadLocalInfo::SpreadLocalInfo(
    string fileName, string filePath, int _dp,
    int _ai1, int _rn, bool _kai,
    int _ai2, int _cn, bool _kaj):
    partFileName(fileName),
    attrIndex1(_ai1), attrIndex2(_ai2),
    rowAmount(_rn),colAmount(_cn),
    keepA1(_kai), keepA2(_kaj), done(false), tupleCounter(0),
    dupTimes(_dp)
{
  partFilePath = getLocalFilePath(filePath, fileName, "", false);

  // Read the schema from the type file created in type mapping
  string typeFilePath = getLocalFilePath(
      filePath, fileName, "_type");
  ListExpr resultTypeList;

  if (!nl->ReadFromFile(typeFilePath, resultTypeList)){
    cerr << "Reading result schema from the type file fails. "
        << endl;
    return;
  }

  ci = new clusterInfo();
  if (ci->isOK())
  {
    if (rowAmount == 0){
//By default, the row number of a flist is the number of slaves
      rowAmount = ci->getSlaveSize();
    }

    //This operator only creates DLF kind distributed relations
    resultList = new fList(fileName,
        NList(resultTypeList), ci, NList(),
        dupTimes, rowAmount, colAmount, false, DLF);

    SecondoCatalog* sc = SecondoSystem::GetCatalog();
    resultTypeList = sc->NumericType(nl->Second(resultTypeList));
    exportTupleType = new TupleType(resultTypeList);
  }
}

bool SpreadLocalInfo::insertTuple(Word wTuple)
{
  // Insert a tuple into a proper data file.
  // row and column number are calculated based on
  // tuple's key attributes value.
  // If the file has not been created, then create it,
  // and insert the file pointer to the matrix
  // Or else, find the file pointer from the matrix,

  Tuple *tuple = static_cast<Tuple*>(wTuple.addr);
  size_t row    = hashValue(tuple, attrIndex1, rowAmount) + 1;
  size_t column = 1;
  if ( attrIndex2 >= 0 ){
    column = hashValue(tuple, attrIndex2, colAmount) + 1;
  }

  fileInfo* fp = 0;
  map<size_t, rowFile*>::iterator mit = matrixRel.find(row);
  if (mit != matrixRel.end()){
    rowFile::iterator rit = mit->second->find(column);
    if (rit != mit->second->end()){
      fp = rit->second;
    }else{
      //create a new partition file.
      fp = new fileInfo(column, partFilePath, partFileName,
          exportTupleType->GetNoAttributes(), row);
      mit->second->insert(pair<size_t, fileInfo*>(column, fp));
    }
  }else
  {
    // Create a new rowFile
    fp = new fileInfo(column, partFilePath, partFileName,
      exportTupleType->GetNoAttributes(), row);
    rowFile *newRow = new rowFile();
    newRow->insert(pair<size_t, fileInfo*>(column, fp));
    matrixRel.insert(pair<size_t, rowFile*>(row, newRow));
  }

  bool ok = openFile(fp);

  if (ok)
  {
    if (!fp->writeTuple(tuple, tupleCounter, exportTupleType,
        attrIndex1, keepA1, attrIndex2, keepA2)){
      cerr << "Block file " << fp->getFilePath()
          << " write failes." << endl;
      ok = false;
    }
    else{
      tupleCounter++;
      tuple->DeleteIfAllowed();
    }
  }
  return ok;
}

bool SpreadLocalInfo::openFile(fileInfo *fp){
  //Control the amount of opening file handles

  if (fp->isFileOpen()){
    return true;
  }

  if (openFileList.size() >= MAX_OPENFILE_NUM)
  {
    //sort fileInfos according to their last tuples' indices
    sort(openFileList.begin(), openFileList.end(), compFileInfo);
    //The last one of the vector is the idler
    bool poped = false;
    //It's possible that fileInfos kept in the stack,
    //are closed from other functions.
    while(!poped && openFileList.size() > 0)
    {
      fileInfo* oldestFile = openFileList.back();
      if (oldestFile->isFileOpen())
      {
        oldestFile->closeFile();
        poped = true;
      }
      openFileList.pop_back();
    }
  }


  bool ok = fp->openFile();
  if (ok){
    openFileList.push_back(fp);
  }
  return ok;
}

bool SpreadLocalInfo::closeAllPartFiles()
{
  //traverse the whole matrix,
  //to add the last description list on all part files.
  //Then close and duplicate them.

  size_t lastRow = 0;

  map<size_t, rowFile*>::iterator mit = matrixRel.begin();
  while (mit != matrixRel.end()){
    size_t row = mit->first;
    if (row > (lastRow + 1)){
      //Insert empty rows
      for (size_t erow = (lastRow + 1); erow < row; erow++)
      {
        NList emptyRowList = NList(
            NList((int)ci->getInterIndex(erow, false, true)),
            NList(),
            NList("", true, true));
        resultList->appendFileLocList(emptyRowList);
      }
    }
    lastRow = row;
    rowFile::iterator rit = mit->second->begin();
    NList columnList;
    string filePaths = "";
    while ( rit!= mit->second->end()){
      size_t column = rit->first;
      columnList.append(NList((int)column));
      fileInfo* fp = rit->second;
      if (openFile(fp))
      {
        fp->writeLastDscr();
        fp->closeFile();
      }
      else
      {
        cerr << "Part file " << fp->getFilePath()
            << " Cannot be correctly opened, "
                "when writing the last description list. " << endl;
        return false;
      }
      filePaths += (fp->getFilePath() + " ");
      rit++;
    }

    bool *copyList = new bool[ci->getClusterSize()];
    memset(copyList, 0, ci->getClusterSize());
    size_t startNode = row;
    for (size_t i = 0; i < dupTimes; i++){
      size_t dupNode = ci->getInterIndex(
          (startNode + i), false, true);
      copyList[dupNode] = true;
    }

    for(size_t i = 0; i < ci->getClusterSize(); i++){
      if (copyList[i]){
        string remotePath =
            ci->getRemotePath(i, false, true, true);
        int copyTime = MAX_COPYTIMES;
        while (copyTime-- > 0)
        {
          if (system(
              (scpCommand + filePaths + " " + remotePath).c_str())
              != 0 ){
            cerr << "Warning! Duplicate files "
                << filePaths << " fails. "
                << strerror(error) << endl;
          }
          else
            break;
        }
        if (copyTime <= 0){
          cerr << "Error! Duplicate remote files fail." << endl;
          return false;
        }
      }
    }

    //add the first duplicate location to the fileLocList
    string remoteLocalPath =
        ci->getRemotePath(row, false, true, false);
    NList rowList = NList(
        NList((int)ci->getInterIndex(row, false, true)),
        columnList,
        NList(remoteLocalPath, true, true));
    resultList->appendFileLocList(rowList);

    mit++;
  }

  resultList->setDistributed();
  done = true;
  return true;
}

size_t SpreadLocalInfo::hashValue(
    Tuple *t, int attrIndex, int scale){
  size_t hashValue =
      ((Attribute*)t->GetAttribute(attrIndex))->HashValue();

  if (scale > 0){
    hashValue %= scale;
  }

  return hashValue;
}


Operator spreadOp(SpreadInfo(), SpreadValueMap, SpreadTypeMap);

/*
5 Operator ~collect~

This operator is used to collect the data from the partition files denoted by the given flist.

----
flist(tuple) x [row] x [column] -> stream(tuple)
----

*/

struct CollectInfo : OperatorInfo {

  CollectInfo() : OperatorInfo()
  {
    name = "collect";
    signature = "flist(tuple) x [int] x [int] -> stream(tuple)";
    syntax = "flist(tuple) x [row] x [column] -> stream(tuple)";
    meaning = "This operator is used to collect the data "
        "from the partition files denoted by the given flist.";
  }

};

/*

5.1 Type Mapping

First ensure the distributed data in flist is a rel(tuple) type.

----
flist(tuple) x [int] x [int] -> stream(tuple)
----

If only one optional parameter is given, then it's viewed as a row number.
The optional parameters only accept non-negative integer numbers.

Any operators create new flist objects,
cannot be used before the ~collect~ operator,
or else the creation will be done twice since we use the
GetNLArgValueInTM function in query processor,


*/
ListExpr CollectTypeMap(ListExpr args)
{

  NList l(args);
  string err[] = {
      //0
      "ERROR! Operator expects flist x [int] x [int]. ",

      "ERROR! Unavailable optional parameters.",

      "ERROR! Operator expects row and column numbers "
      "are non-negative values.",
  };
  NList pType, pValue;

  if (l.length() != 2)
    return l.typeError(err[0]);

  //First flist
  pType = l.first().first();
  pValue = l.first().second();
  if (!isFListStreamDescription(pType)){
    return l.typeError(err[0]);
  }
  NList tupleType = pType.second().second();

  //Optional parameters
  pType = l.second().first();
  pValue = l.second().second();
  if (pType.length() > 2){
    return l.typeError(err[0]);
  }
  else if (pType.length() > 0){
    if (!pType.first().isSymbol(CcInt::BasicType())){
      return l.typeError(err[0]);
    }

    NList opVal;
    if (!qp->GetNLArgValueInTM(pValue.first(), opVal)){
      return l.typeError(err[1]);
    }else{
      int rowNum = opVal.intval();
      if (rowNum < 0){
        return l.typeError(err[2]);
      }
    }

    if (pType.length() > 1){
      if (!pType.first().isSymbol(CcInt::BasicType())){
        return l.typeError(err[0]);
      }
      if (!qp->GetNLArgValueInTM(pValue.second(), opVal)){
        return l.typeError(err[1]);
      }else{
        int columnNum = opVal.intval();
        if (columnNum < 0){
          return l.typeError(err[2]);
        }
      }
    }
  }

  NList streamType =
      NList(NList(Symbol::STREAM()),
              NList(tupleType));

  return streamType.listExpr();
}


/*

5.2 Value Mapping

By default, it reads all partition files denoted in the given flist, and returns the tuples in a stream.
If the optional parameters are given, then this operator only reads part partition files.
For any partition files listed a flist, both it's row and column numbers are non-zero positive integer numbers.
If the row number is 0, then it denotes a complete row partition files, while a complete column part files are denoted when the column number is 0. E.g,

  * collect[1] or collect[1,0]  means to collect all partitions files in the first row,

  * collect[0,2] means to collect all partition files in the second column,

  * collect[1,2] means to collect one partition file in the 1th row and 2th column,

  * collect[0,0] means to collect all files inside the matrix.


*/
int CollectValueMap(Word* args, Word& result,
    int message, Word& local, Supplier s){
  CollectLocalInfo* cli = 0;

  switch(message){
    case OPEN: {

      fList* partitionFileList = (fList*)args[0].addr;

      NList currentCluster = clusterInfo().toNestedList();
      if (!partitionFileList->isCollectable(currentCluster)){
        cerr << "This flist is not collectable" << endl;
        return CANCEL;
      }

      size_t row = 0, column = 0;
      Supplier optList = args[1].addr;
      int optLen = qp->GetNoSons(optList);
      if (1 == optLen){
        row = ((CcInt*)
          qp->Request(qp->GetSupplier(optList,0)).addr)->GetValue();
      }else if (2 == optLen){
        row = ((CcInt*)
          qp->Request(qp->GetSupplier(optList,0)).addr)->GetValue();
        column = ((CcInt*)
          qp->Request(qp->GetSupplier(optList,1)).addr)->GetValue();
      }

      cli = (CollectLocalInfo*)local.addr;
      if (cli){
        delete cli;
        cli = 0;
      }
      cli = new CollectLocalInfo(partitionFileList, row, column);

      if (cli->fetchAllPartFiles()){
        local.setAddr(cli);
      }
      else{
        delete cli;
        cli = 0;
        local.setAddr(Address(0));
      }

      return 0;
    }
    case REQUEST: {
      if (0 == local.addr)
        return CANCEL;

      cli = (CollectLocalInfo*)local.addr;
      result.setAddr(cli->getNextTuple());
      if ( 0 == result.addr)
      {
        return CANCEL;
      }
      else
      {
        return YIELD;
      }
    }
    case CLOSE: {
      cli = (CollectLocalInfo*)local.addr;
      if (!cli)
        return CANCEL;
      else{
        delete cli;
        cli = 0;
        local.setAddr(Address(0));
      }
      return 0;  //must return
    }
  }


  return 0;
}

CollectLocalInfo::CollectLocalInfo(fList* _fl, size_t _r, size_t _c):
    fileList(_fl), row(_r), column(_c), inputFile(0)
{
  NList typeList = fileList->getInterTypeList();

  //Get the resultType
  SecondoCatalog* sc = SecondoSystem::GetCatalog();
  ListExpr resultTypeList =
      sc->NumericType(typeList.second().listExpr());
  resultType = new TupleType(resultTypeList);
}

bool CollectLocalInfo::fetchAllPartFiles()
{
  //According to the accepted row and column number,
  //fetch data files and fullfill their names to the partFiles vector

  size_t rBegin = ( row == 0 ) ? 1 : row,
         rEnd   = ( row == 0 ) ? fileList->getMtxRowNum() : row;

  clusterInfo* cluster = new clusterInfo();
  string localIP = cluster->getLocalIP();

  for ( size_t ri = rBegin; ri <= rEnd; ri++)
  {
    //Files of a same row are always kept together
    vector<string> remotePaths;
    size_t cand =
        fileList->getPartitionFileLoc(ri, remotePaths);
    if (cand > 0)
    {
      NList newColList = fileList->getColumnList(ri);
      if (column != 0)
      {
        //find one file of each row
        //check the column number is exist in the column list
        NList rest = newColList;
        bool find = false;
        while (!rest.isEmpty()){
          int elem = rest.first().intval();
          if ( (int)column == elem){
            find = true;
            break;
          }
          rest.rest();
        }
        if (find){
          newColList = NList();
          newColList.append(NList((int)column));
        }
        else{
          cerr << "The partition ("
              << ri << "," << column <<") is unavailable. " << endl;
          return false;
        }
      }

      NList rest = newColList;
      while (!rest.isEmpty()){
        size_t ci = rest.first().intval();
        string fileName = fileList->getSubName()
            + "_" + int2string(ri) + "_" + int2string(ci);
        string localPath = getLocalFilePath("", fileName, "", true);
        vector<string>::iterator pit = remotePaths.begin();
        while (pit != remotePaths.end()){
          string rPath = *pit;
          FileSystem::AppendItem(rPath, fileName);
          string remoteIP, remoteLocalPath;
          istringstream iss(rPath);
          getline(iss, remoteIP, ':');
          getline(iss, remoteLocalPath, ':');

          bool found = false;
          if (remoteIP.compare(localIP) == 0)
          {
            localPath = remoteLocalPath;
            found = true;
          }
          else
          {
            int copyTime = MAX_COPYTIMES;
            while (copyTime-- > 0){
              if (0 != system(
                  (scpCommand + rPath + " " + localPath).c_str()))
              {
                cerr << "Warning! Copy remote file from " << rPath
                    << " doesn't work yet." << endl;
              }else
              {
                break;
              }
            }

            if (copyTime <= 0){
              cerr << "ERROR! Copy remote file from " << rPath
                  << " fails" << endl;
              return false;
            }
            else{
              found = true;
            }
          }

          if (found)
          {
            partFiles.push_back(localPath);
            break;
          }
          pit++;
        }
        rest.rest();
      }
    }
  }
  return true;
}

bool CollectLocalInfo::partFileOpened()
{
  if (0 == inputFile){
    if (partFiles.size() != 0){
      string partFileName = partFiles.back();
      //open the file to inputFiles
      inputFile = new ifstream(partFileName.c_str(), ios::binary);
      if (!inputFile->good()){
        inputFile = 0;
        return false;
      }

      //Read the tail description list
      u_int32_t descSize;
      size_t fileLength;
      inputFile->seekg(0, ios::end);
      fileLength = inputFile->tellg();
      inputFile->seekg(
          (fileLength - sizeof(descSize)), ios::beg);
      inputFile->read((char*)&descSize, sizeof(descSize));

      char descStr[descSize];
      inputFile->seekg(
          (fileLength - (descSize + sizeof(descSize))), ios::beg);
      inputFile->read(descStr, descSize);
      inputFile->seekg(0, ios::beg);

      NList descList = NList(binDecode(string(descStr)));
      if (descList.isEmpty())
      {
        cerr << "\nERROR! Tail description list read in "
            << partFileName << "fail." << endl;
        return false;
      }
      return true;
    }
    else{
      return false;  //No more file in the partFiles list
    }
  }
  else{
    return true;  //Exist an opened file
  }

}

Tuple* CollectLocalInfo::getNextTuple()
{
  Tuple* t = 0;

  //Read one tuple from the present opened file
  //if there is no more data in the current file,
  //then remove the file from the partFiles list,
  //and open the new one.
  while (partFileOpened()){
    u_int32_t blockSize = 0;
    inputFile->read( reinterpret_cast<char*>(&blockSize),
        sizeof(blockSize));
    if (blockSize > 0)
    {
      //read and return the tuple
      blockSize -= sizeof(blockSize);
      char *tupleBlock = new char[blockSize];
      inputFile->read(tupleBlock, blockSize);

      t = new Tuple(resultType);
      t->ReadFromBin(tupleBlock, blockSize);
      delete[] tupleBlock;
      break;
    }
    else
    {
      //close the opened file,
      //pop the file name from the file list.
      inputFile->close();
      delete inputFile;
      inputFile = 0;
      partFiles.pop_back();
    }
  }
  return t;
}

Operator collectOp(CollectInfo(), CollectValueMap, CollectTypeMap);

/*
4. Type Operator ~PARA~

*/

struct ParaInfo : OperatorInfo
{
  ParaInfo()
  {
    name = "para";
    signature =
        "( flist(T) ) -> T \n"
        "T -> T, T in DELIVERABLE";
    syntax = "type operator";
    meaning = "Extract the data type from a flist object";
  }
};

ListExpr ParaTypeMapping( ListExpr args)
{
  NList l(args);
  string tpeErr = "Eexpect one flist or DELIVERABLE input";

  if (l.length() != 1)
    return l.typeError(tpeErr);

  NList inType;
  if (l.first().isAtom())
  {
    //non flist input
    if (!listutils::isKind(
        l.first().listExpr(), Kind::DELIVERABLE())){
      return l.typeError(tpeErr);
    }
    inType = l.first();
  }
  else
  {
    if (!l.first().first().isSymbol(fList::BasicType()))
      return l.typeError(tpeErr);
    inType = l.first().second();
  }

  return inType.listExpr();
}

int ParaValueMapping(Word* args, Word& result,
    int message, Word& local, Supplier s){

  cerr << "\nOps... It is not allowed to use para operator in "
      "any directly evaluable queries."
      << endl;
  return 0;
}

Operator paraOp(ParaInfo(), ParaValueMapping, ParaTypeMapping);

struct TParaInfo : OperatorInfo
{
  TParaInfo()
  {
    name = "TPARA";
    signature =
        "( flist(ANY) ) -> ANY";
    syntax = "type operator";
    meaning = "Extract the data type from a flist object";
  }
};

ListExpr TParaTypeMapping( ListExpr args)
{
  NList l(args);

  if (l.length() < 1)
    return l.typeError("Expect at least one argument.");
  NList ffListType = l.first();

  if (ffListType.isAtom()){
    return ffListType.listExpr();
  }
  else
  {
    if (ffListType.first().isSymbol(fList::BasicType())){
      return ffListType.second().listExpr();
    }
    else{
      return ffListType.listExpr();
    }
  }

}

/*
4 Operator ~hadoopMap~

Create DGO DLO kind flist after being queried
within the map step of the embedded Hadoop job.

Update at 22th Mar. 2012
Jiamin

Remove the DGO kind, but it can create either DLO or DLF kind flist.
The operator name is changed to ~hadoopMap~,
which means it creates a new flist only through the map step of
the preset Hadoop job.

The map step in MR works similar as a ~filter~ operation,
hence this operator only accepts one flist as the input,
but in its internal function, several other flists can be used too.

It is not necessary to indicate name to created sub-objects or sub-files,
which can be set by the system.
However, for DLF kind flist, it is better to keep the tridition.
Therefore, file name and file path is prepared as optional argument.
Also, an optional symbol is prepared, used to indicate the kind of the result flist.
By default, it is DLO, but also can be set as DLF.
I didn't use an integer or a bool for this value,
since it is possible for us to create more kinds for the flist.

*/
struct hadoopMapInfo : OperatorInfo
{
  hadoopMapInfo()
  {
    name = "hadoopMap";
    signature =
        "flist(T) x [string] x [text] x [(DLO):DLF] "
        "x ( map T T1 ) -> flist(T1)";
    meaning = "Create DLO or DLF kind flist after the map step";
  }
};

/*
4.1 Type Mapping of ~hadoopMap~

This operator maps

----
T x string -> flist
----

The T is the data created by the prepositive query.
If this query is only an exist Secondo object name,
then this operator creates a DGO flist data.
Or else if the query contains at least one ~para~ operator,
then this operator creates a DLO flist data,
and creates local Secondo objects in slave databases through
delivering the prepositive query by a generic Hadoop job.

The string is the object name created on every slave database,
and it must be start with a capital character.
If the object name is given as an empty string,
then it creates a DGO kind flist by default.


Update at 21th Mar. 2012
Jiamin

Now this operator maps

----
flist(T) x [string] x [text] x [(DLO):DLF]
  x ( map ( T -> T1) ) -> flist(T1)
----

*/

ListExpr hadoopMapTypeMap(ListExpr args){
  NList l(args);

  string lenErr = "ERROR! Operator hadoopMap expects 3 arguments. ";
  string typErr = "ERROR! Operator hadoopMap expects "
      "flist(T) x [string] x [text] x [(DLO):DLF] x (map T T1))";

  string ifaErr = "ERROR! Infeasible evaluation in TM of attribute:";
  string nprErr = "ERROR! Operator hadoopMap expects "
      "creating a new DLO or DLF kind flist.";
  string onmErr = "ERROR! Operator hadoopMap expects the created "
      "object name starts with upper case. ";
  string uafErr = "ERROR!! The internal function is unavailable.";
  string hnmErr = "ERROR! Exists homonymous flist type file in: ";
  string fwtErr = "ERROR! Failed writing type into file: ";
  string expErr = "ERROR! Improper output type for DLF flist";

  string objName, filePath, qStr;
  fListKind kind = DLO;

  if (l.length() != 3)
    return l.typeError(lenErr);

  //The input must be a flist type data.
  NList inputType = l.first().first();
  if (!inputType.first().isSymbol(fList::BasicType())){
    return l.typeError(nprErr);
  }

  //Optional parameters
  int len = l.second().first().length();
  if (len > 0)
  {
    for (int i = 1; i <= len; i++)
    {
      NList pp = l.second().first().elem(i);
      NList pv = l.second().second().elem(i);

      if (pp.isSymbol(CcString::BasicType()))
      {
        //ObjectName defined.
        NList fnList;
        if (!QueryProcessor::GetNLArgValueInTM(pv, fnList)){
          return l.typeError(ifaErr + "objName");
        }
        objName = fnList.str();
      }
      else if (pp.isSymbol(FText::BasicType()))
      {
        //FilePath defined
        NList fpList;
        if (!QueryProcessor::GetNLArgValueInTM(pv, fpList)){
          return l.typeError(ifaErr + "filePath");
        }
        filePath = fpList.str();
      }
      else if (pp.isSymbol("DLF")){
        kind = DLF;
      }
      else if (!pp.isSymbol("DLO")){
        return l.typeError(nprErr);
      }
    }
  }

  //Encapsulated object type
  NList coType = l.third().first().third();   //Create Type
  NList coQuery = l.third().second().third(); //Create Query
  NList coBType;                              //Create Base Type
  if (coType.isAtom()){
    coBType = coType;
  }
  else{
    coBType = coType.first();
  }
  if (coBType.isEqual("typeerror"))
    return l.typeError(uafErr);
  qStr = coQuery.convertToString();

  //Query Parameter
  string coParaName = l.third().second().second().first().str();

  //If it is a DLF, then the output type must be tuple stream or DATA
  //Several different export type for creating DLF kind flist:
  //0 : DLO, non-DLF kind
  //1 : stream(tuple)
  //2 : stream(T)      < T in DATA kind >
  //3 : T              < T in DATA kind >
  int dlfType = 0;
  if (kind == DLF)
  {
    if (listutils::isStream(coType.listExpr()))
    {
      if (listutils::isTupleStream(coType.listExpr())){
        dlfType = 1;
      }
      else{
        NList exType = coType.second().first();
        if (!listutils::isKind(exType.listExpr(), Kind::DATA())){
          return l.typeError(expErr);
        }
        dlfType = 2;
      }
    }
    else
    {
      NList exType = coType.first();
      if (!listutils::isKind(exType.listExpr(), Kind::DATA())){
        return l.typeError(expErr);
      }
      dlfType = 3;
    }
  }

  NList resultType = NList(NList(fList::BasicType()), NList(coType));
  if (objName.length() == 0)
    objName = fList::tempName(false);
  else
  {
    char f = objName[0];
    if (f<'A' || f>'Z'){
      return l.typeError(onmErr);
    }
    //If the name of sub-objects or sub-files are denoted by users,
    //Then it must be kept in a text type file,
    //in case of the homonymous problem
    //also this file must be kept in the default file path.

    string filePath =
        getLocalFilePath("", (objName + "_type"),"", true);
    if (FileSystem::FileOrFolderExists(filePath)){
      ListExpr exeType;
      bool ok = false;
      if (nl->ReadFromFile(filePath, exeType)){
          if (nl->Equal(exeType, nl->Second(resultType.listExpr()))){
            ok = true;
          }
      }
      if (!ok)
        return l.typeError(hnmErr + filePath);
    }
    else{
      ListExpr expList = nl->Second(resultType.listExpr());
      if (nl->IsAtom(expList)){
        expList = nl->OneElemList(expList);
      }
      if (!nl->WriteToFile(filePath, expList)){
        return l.typeError(fwtErr + filePath);
      }
    }
  }

  return NList(NList(Symbol::APPEND()),
      NList(NList(qStr, true, true),
            NList(objName, true, false),
            NList(dlfType),
            NList(coParaName, true, false)),
      resultType).listExpr();

}

int hadoopMapValueMap(Word* args, Word& result,
    int message, Word& local, Supplier s){

  if ( message <= CLOSE )
  {
    //Appended parameters
    //Query string
    string CreateQuery =
        ((FText*)args[3].addr)->GetValue();
    //Get the object name.
    string CreateObjectName =
        ((CcString*)args[4].addr)->GetValue();
    //Get the export file type
    int CreateObjectType =
        ((CcInt*)args[5].addr)->GetValue();
    string inParaName =
        ((CcString*)args[6].addr)->GetValue();

    //Optional parameters
    string CreateFilePath = "";
    fListKind kind = DLO;
    if (CreateObjectType > 0)
      kind = DLF;
    Supplier oppList = args[1].addr;
    int opLen = qp->GetNoSons(oppList);
    for (int i = 0; i < opLen; i++)
    {
      //Only the file path need to be get
      ListExpr pp = qp->GetType(qp->GetSupplierSon(oppList,i));
      if (nl->IsEqual(pp, FText::BasicType())){
        CreateFilePath = ((FText*)
          qp->Request(qp->GetSupplier(oppList,i)).addr)->GetValue();
      }
    }

    //Result type
    ListExpr resultType = qp->GetType(s);
    fList* resultFList;

    {
      //Create DLO flist
      fList* inputFList = (fList*)args[0].addr;
      int dupTimes = inputFList->getDupTimes();

      //Parameters required by the Hadoop job are:
      string dbName = fList::tempName(true);
      ListExpr CreateQueryList;
      nl->ReadFromString(CreateQuery, CreateQueryList);
      vector<string> flistParaList;
      vector<fList*> flistObjList;
      flistParaList.push_back(inParaName); //The input parameter
      flistObjList.push_back(inputFList);
      vector<string> DLF_NameList, DLF_fileLocList;

      //Scan para operations in the internal function list
      //If the ~para~ operator contains a DELIEVERABLE object,
      //then extract its value in nested-list format,
      //and embed inside the query.
      //Or else, replace the para operation with the flist name,
      //then left to the next function.
      bool ok = true;
      CreateQueryList = replaceParaOp(CreateQueryList, flistParaList,
          flistObjList, ok);
      if (!ok){
        cerr << "Replace para operation fails" << endl;
      }
      else{
        //Replace parameter value according to their flist value
        for (size_t i = 0; i < flistParaList.size(); i++)
        {
          CreateQueryList =
              replaceFList(CreateQueryList,
                  flistParaList[i], flistObjList[i],
                  DLF_NameList, DLF_fileLocList, ok);
          if (!ok)
            break;
        }
      }

      NList dlfNameList, dlfLocList;
      ListExpr sidList;
      if (!ok){
        cerr << "Reading flist data fails." << endl;
      }
      else
      {
        for (size_t i = 0; i < DLF_NameList.size(); i++)
        {
          dlfNameList.append(NList(DLF_NameList[i], true, false));
          ListExpr locList;
          nl->ReadFromString(DLF_fileLocList[i], locList);
          dlfLocList.append(NList(locList));
        }

        //Call the Hadoop job
        stringstream queryStr;
        queryStr
          << "hadoop jar ParallelSecondo.jar "
              "ParallelSecondo.PS_HadoopMap \\\n"
          << dbName << " " << CreateObjectName << " "
          << " \"" << tranStr(nl->ToString(CreateQueryList),
              "\"", "\\\"") << "\" \\\n"
          << " \"" << tranStr(dlfNameList.convertToString(),
              "\"", "\\\"") << "\" \\\n"
          << " \"" << tranStr(dlfLocList.convertToString(),
              "\"", "\\\"") << "\" \\\n"
          << dupTimes  << " " << CreateObjectType << " "
          << " \"" << CreateFilePath << "\"" << endl;
        int rtn = -1;
//        cout << queryStr.str() << endl;
        rtn = system(queryStr.str().c_str());
        ok = (rtn == 0);

        if (ok)
        {
          FILE *fs;
          char buf[MAX_STRINGSIZE];
          fs = popen("hadoop dfs -cat OUTPUT/part*", "r");
          if (NULL != fs)
          {
            if (fgets(buf, sizeof(buf),fs))
            {
              stringstream ss;
              ss << buf;
              string locListStr = ss.str();
              locListStr = locListStr.substr(locListStr.find_first_of(' '));
              nl->ReadFromString(locListStr, sidList);
            }
            else
              ok = false;
          }
          else
            ok = false;
        }
      }

      if (ok){
        clusterInfo *ci = new clusterInfo();
        int slaveSize = (int)ci->getSlaveSize();
        NList fileLocList;

        if (CreateObjectType == 0)
          CreateFilePath = dbLoc;

        //Create file location list
        ListExpr rest = sidList;
        while (!nl->IsEmpty(rest))
        {
          int slaveIdx = nl->IntValue(nl->First(rest));
          cerr << "Get one slave Index as: " << slaveIdx << endl;

          if (fileLocList.isEmpty()){
            fileLocList.makeHead(
                NList(NList(slaveIdx), NList(1).enclose(),
                    NList(CreateFilePath, true, true)));
          }
          else{
            fileLocList.append(
                NList(NList(slaveIdx), NList(1).enclose(),
                NList(CreateFilePath, true, true)));
          }

          rest = nl->Rest(rest);
        }

        resultFList = new fList(CreateObjectName, NList(resultType),
          ci, fileLocList, 1, slaveSize, 1, true, kind);
      }
      else{
        //If the creation job is not successfully returned,
        //Then an empty flist is returned as the result
        resultFList = 0;
      }
    }

    result.setAddr(resultFList);
  }
  //TODO add the progress estimation in the future
  //  else if ( message == REQUESTPROGRESS )
  //  else if ( message == CLOSEPROGRESS )

    return 0;

}

Operator hadoopMapOP(
    hadoopMapInfo(), hadoopMapValueMap, hadoopMapTypeMap);


/*
5 Operator ~hadoop~

This operator should performs various kinds of binary sequence
operators in parallel. Its mapping is:

----
flist(T1) x flist(T2)
x (map stream(T1) stream(T3(... ai ... )) )   # mapQuery 1
x ai                                          # PartitionAttribute 1
x (map stream(T2) stream(T4(... aj ... )) )   # mapQuery 2
x aj                                          # PartitionAttribute 2
x (map stream(T3) stream(T4) stream(T5))      # reduceQuery
x string                                      # resultName
x int                                         # reduce scale
-> flist(T5)
----

And its specification is:

----
_ _ hadoop[_, _;_,_;_;_,_]
flist flist hadoop[map1, Part1; map2, Part2; reduce; result, scale]
----

*/

/*
4 Operator ~createFList~

Create an undistributed flist,
used to debug the implementation of the flist operator.

T -> flist

I disable the In and Out function of the flist,
like what BTree does, because we abandon the DGO kind object,
hence it is impossible to create a flist without running a hadoop job.


*/
struct createFListInfo : OperatorInfo
{
  createFListInfo()
  {
    name = "createFList";
    signature = "T -> flist";
    meaning = "";
  }
};

/*
4.1 Type Mapping

*/
ListExpr createFListTypeMap(ListExpr args){

  NList l(args);
  string tpeErr = "ERROR! createFList expects "
    "T -> flist Debuging ... ";
  if (l.length() != 1){
    return l.typeError(tpeErr);
  }

  NList pType, pValue;
  pType = l.first().first();
  pValue = l.first().second();

  string objName = pValue.str();
  if (!SecondoSystem::GetCatalog()->IsObjectName(objName))
    return l.typeError("Object doesn't exist");

  NList resultType = NList(NList(fList::BasicType()), pType);

  return resultType.listExpr();

}

/*
4.2 Value Mapping

*/
int createFListValueMap(Word* args, Word& result,
    int message, Word& local, Supplier s){

  string dbName = fList::tempName(true);

  string objName = fList::tempName(false);
  NList resultType = NList(qp->GetType(s));
  clusterInfo* ci = new clusterInfo();
  NList fileLocList = NList();
  size_t dupTime = 1;
  fList* rl =
      new fList(objName, resultType, ci, fileLocList, dupTime);

  result.setAddr(rl);
  return 0;
}


Operator cflOp(
    createFListInfo(), createFListValueMap, createFListTypeMap);


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

/*
The ~getLocalFilePath~ function is used to set the path of the type and
data files produced by ~fconsume~, ~ffeed~ and ~fdistribute~ operators.

If a specified file path is not given, then it reads the
~SecondoFilePath~ variable set in the SecondoConfig.ini that is
denoted by SECONDO\_CONFIG parameter.
The path must be an absolute path.
By default, the path will be set to SECONDO\_BUILD\_DIR/bin/parallel

If an non-default path is unavailable or not exist,
then a warning message will be given, and the default value is used.

*/
string getLocalFilePath(string filePath, const string fileName,
                   string suffix, bool appendFileName)
{

  bool pathOK = false, alarm = false;
  string path = "";
  int candidate = 0;
  while (!pathOK && candidate < 3)
  {
    if (0 == candidate) {
      path = filePath;
    }
    else if (1 == candidate) {
      path = SmiProfile::GetParameter("ParallelSecondo",
          "SecondoFilePath","", string(getenv("SECONDO_CONFIG")));
    }
    else {
      path = FileSystem::GetCurrentFolder();
      FileSystem::AppendItem(path, "parallel");
    }

    if (path.length() > 0)
    {
      if (path.find_last_of("/") == (path.length() - 1))
        path = path.substr(0, path.length() - 1);

      //In case the parent folder doesn't exist.
      if ( FileSystem::IsDirectory(path) ) {
        pathOK = true;
      }
      else {
        pathOK = FileSystem::CreateFolder(path);
        alarm = true;
      }
    }
    candidate++;
  }

  if (pathOK)
  {
    if (appendFileName){
      FileSystem::AppendItem(path, fileName + suffix);
    }

    // When there is no specific path is given,
    // then no warning messages.
    if (alarm){
      cerr << "Warning! The given path is unavailable or not exit, "
          "\n then the default path " << path << " is used.\n\n";
    }
  }

  return (pathOK ? path : "");
}

/*
Convert a numeric type list back to the normal type list.

*/
ListExpr AntiNumericType(ListExpr type)
{
  if (nl->IsEmpty(type)){
    return type;
  } else if (nl->ListLength(type) == 2 ) {
      if (  nl->IsAtom(nl->First(type)) &&
            nl->IsAtom(nl->Second(type)) &&
            nl->AtomType(nl->First(type)) == IntType &&
            nl->AtomType(nl->Second(type)) == IntType){

        int algID, typID;
        extractIds(type, algID, typID);
        SecondoCatalog* sc = SecondoSystem::GetCatalog();
        if(algID < 0 || typID < 0)
          return nl->SymbolAtom("ERROR");
        return nl->SymbolAtom(sc->GetTypeName(algID,typID));
      }
      else
        return (nl->Cons(AntiNumericType(nl->First(type)),
            AntiNumericType(nl->Rest(type))));
  }
  else if (nl->IsAtom(type)){
    return type;
  }
  else{
    return (nl->Cons(AntiNumericType(nl->First(type)),
                     AntiNumericType(nl->Rest(type))));
  }
}

/*
Checks for valid description of a flist

*/
bool isFListStreamDescription(const NList& typeInfo)
{
  if (typeInfo.length() != 2){
    return false;
  }
  if (!( typeInfo.first().isSymbol(fList::BasicType()) &&
      listutils::isTupleStream(typeInfo.second().listExpr()))){
    return false;
  }

  return true;
}

/*
Scans the query list, and replace parameter values by fList data.
DGO kind data replace the parameter with their value list,
while DLO kind data replace with object name.
If it is a DLF type list, then the file name is appended into the DLF\_NameList,
while the fileLocList is appended into the DLF\_fileLocList

*/
ListExpr replaceFList(ListExpr createQuery, string listName,
    fList* listObject, vector<string>& DLF_NameList,
    vector<string>& DLF_fileLocList, bool& ok)
{
  if (!ok)
  {
    return nl->OneElemList(nl->SymbolAtom("error"));
  }

  if (nl->IsEmpty(createQuery))
    return createQuery;

  if (nl->IsAtom(createQuery))
  {
    if ((nl->AtomType(createQuery) == SymbolType) &&
        (nl->SymbolValue(createQuery) == listName))
    {
      if (listObject->isAvailable())
      {
        string objectName = listObject->getSubName();
        switch (listObject->getKind())
        {
          case DLO:{
            return nl->SymbolAtom(objectName);
          }
          case DLF:{
            stringstream ss;
            ss << "<DLFMark:" << objectName << "/>";
            DLF_NameList.push_back(objectName);
            DLF_fileLocList.push_back(
                listObject->getLocList().convertToString());
            return nl->StringAtom(ss.str(),true);
            ;
          }
          default:{
            ok = false;
            return nl->OneElemList(nl->SymbolAtom("error"));
          }
        }
      }
      ok = false;
      return nl->OneElemList(nl->SymbolAtom("error"));
    }
    else
      return createQuery;
  }
  else
  {
    return (nl->Cons(replaceFList(nl->First(createQuery),
        listName, listObject, DLF_NameList, DLF_fileLocList, ok),
                     replaceFList(nl->Rest(createQuery),
        listName, listObject, DLF_NameList, DLF_fileLocList, ok)));
  }
}

/*
Find all nested lists of ~para~ operations,

If the operator contains a T type data, T [INSET] DATA,
then replace it with the data's value directly.
It it contains a flist(T) data,
then add its name and value to these two vectors.

*/
ListExpr replaceParaOp(
    ListExpr createQuery, vector<string>& flistParaList,
    vector<fList*>& flistObjList, bool& ok)
{
  if (!ok){
    return nl->OneElemList(nl->SymbolAtom("error"));
  }

  if (nl->IsEmpty(createQuery))
    return createQuery;

  if (nl->ListLength(createQuery) == 2)
  {
    ListExpr first = nl->First(createQuery);
    if (nl->IsAtom(first))
    {
      if (nl->IsEqual(first, "para"))
      {
        ListExpr second = nl->Second(createQuery);
        string paraName = nl->ToString(second);
        ListExpr paraType =
            SecondoSystem::GetCatalog()->GetObjectTypeExpr(paraName);

        if (nl->ListLength(paraType) > 1){
          if(nl->IsEqual(nl->First(paraType), fList::BasicType())){
            flistParaList.push_back(paraName);
            Word listValue;
            bool defined;
            ok = SecondoSystem::GetCatalog()->
                GetObject(paraName, listValue, defined);
            if (ok){
              flistObjList.push_back((fList*)listValue.addr);
              return nl->SymbolAtom(paraName);
            }
            else
              return nl->OneElemList(nl->SymbolAtom("error"));
          }
        }
        else{
          ListExpr DGOValue =
              SecondoSystem::GetCatalog()->GetObjectValue(paraName);
          return DGOValue;
        }
      }
    }
  }

  if (nl->ListLength(createQuery) > 0){
    return (nl->Cons(
      replaceParaOp(nl->First(createQuery),
           flistParaList, flistObjList, ok),
      replaceParaOp(nl->Rest(createQuery),
           flistParaList, flistObjList, ok) ));
  }
  else{
    return createQuery;
  }
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
    AddTypeConstructor(&flTC);

    AddOperator(doubleExportInfo(),
        doubleExportValueMap, doubleExportTypeMap);

    Operator* parahashjoin =
    AddOperator(paraHashJoinInfo(),
        paraHashJoinValueMap, paraHashJoinTypeMap);
      parahashjoin->SetUsesMemory();

    Operator* parajoin =
    AddOperator(paraJoinInfo(),
        paraJoinValueMap, paraJoinTypeMap);
      parajoin->SetUsesMemory();

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

    AddOperator(&spreadOp);
    spreadOp.SetUsesArgsInTypeMapping();

    AddOperator(&collectOp);
    collectOp.SetUsesArgsInTypeMapping();

    //AddOperator(ParaInfo(), ParaValueMapping, ParaTypeMapping);
    AddOperator(&paraOp);

    AddOperator(TParaInfo(), 0, TParaTypeMapping);

    AddOperator(&hadoopMapOP);
    hadoopMapOP.SetUsesArgsInTypeMapping();

    AddOperator(&cflOp);
    cflOp.SetUsesArgsInTypeMapping();




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

