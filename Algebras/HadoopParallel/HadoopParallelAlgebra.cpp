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

Since the result relation follows the (key, value) style,
the MapReduce model can read the tuples inside this relation,
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

  CHECK_COND(nl->ListLength(args) == 4, "Expect 4 arguments");
  CHECK_COND(listutils::isTupleStream(nl->First(args))
    && listutils::isTupleStream(nl->Second(args))
    && listutils::isSymbol(nl->Third(args))
    && listutils::isSymbol(nl->Fourth(args)),
    "Expect stream (tuple((a1 t1) ... (ai ti) ... (an tm)))"
         "x stream (tuple((b1 p1) ... (bj tj) ... (bm tm)))"
         "x ai x bj -> stream (tuple (key:text) (value:text))");

  //Get the indices of two indicated key attributes
  ListExpr tupTypeA, tupTypeB;
  tupTypeA = nl->Second(nl->First(args));
  tupTypeB = nl->Second(nl->Second(args));

  ListExpr attrTypeA, attrTypeB;
  ListExpr tupListA = nl->Second(tupTypeA);
  string attrAName = nl->SymbolValue(nl->Third(args));
  int attrAIndex =
      listutils::findAttribute(tupListA,attrAName,attrTypeA);
  CHECK_COND(attrAIndex > 0,
    "Attributename " + attrAName
    + " not found in the first argument");
  ListExpr tupListB = nl->Second(nl->Second(nl->Second(args)));
  string attrBName = nl->SymbolValue(nl->Fourth(args));
  int attrBIndex =
      listutils::findAttribute(tupListB,attrBName,attrTypeB);
  CHECK_COND(attrBIndex > 0,
    "Attributename " + attrBName
    + " not found in the second argument");
  CHECK_COND(nl->Equal(attrTypeA, attrTypeB),
    "Expect equal key types.");

  ListExpr attrList = nl->TwoElemList(
      nl->TwoElemList(nl->StringAtom("keyT",false),
          nl->SymbolAtom(STRING)),
      nl->TwoElemList(nl->StringAtom("valueT",false),
          nl->SymbolAtom(TEXT)));
  NList AttrList(attrList, nl);
  NList tupleStreamList = NList(NList().tupleStreamOf(AttrList));

  return nl->ThreeElemList(
               nl->SymbolAtom("APPEND"),
               nl->TwoElemList(nl->IntAtom(attrAIndex),
                               nl->IntAtom(attrBIndex)),
               tupleStreamList.listExpr());
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
                         Supplier s)
{
  streamA = _streamA;
  streamB = _streamB;
  attrIndexA = StdTypes::GetInt( wAttrIndexA ) - 1;
  attrIndexB = StdTypes::GetInt( wAttrIndexB ) - 1;
  isAEnd = false;

  //The tupleType here should looks like (tuple(....))
  tupTypeA = nl->OneElemList(
      nl->Second(GetTupleResultType(qp->GetSon(s,0))));
  tupTypeB = nl->OneElemList(
      nl->Second(GetTupleResultType(qp->GetSon(s,1))));

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
  Word result(Address(0));

  if(!isAEnd){
    result = makeTuple(streamA, attrIndexA, tupTypeA, 1);
    if (result.addr != 0)
      return static_cast<Tuple*>( result.addr );
    else
      isAEnd = true;
  }
  result = makeTuple(streamB, attrIndexB, tupTypeB, 2);
  return static_cast<Tuple*> (result.addr);
}

Word deLocalInfo::makeTuple(Word stream, int index,
                            ListExpr typeInfo, int si)
{
  bool yield = false;
  Word result(Address(0));
  Tuple* oldTuple, *newTuple;

  qp->Request(stream.addr, result);
  yield = qp->Received(stream.addr);

  if (yield){
    //Get a tuple from the stream;
    oldTuple = static_cast<Tuple*>(result.addr);
    string key =
        ((CcString*)(oldTuple->GetAttribute(index)))->GetValue();
    ListExpr oldTupleNL = oldTuple->Out(typeInfo);

    ListExpr valueNL = nl->TwoElemList(nl->IntAtom(si),oldTupleNL);
    string valueStr = nl->ToString(valueNL);

    newTuple = new Tuple(resultTupleType);
    newTuple->PutAttribute(0,new CcString(key));
    newTuple->PutAttribute(1,new FText(true,valueStr));

    result.setAddr(newTuple);
  }

  return result;
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
the schema: (SI:int, tupleVal:text).
The ~SI~ is the key field, and the ~tupleVal~ is the value field here.
If we only simply send this kind of tuples back to Secondo,
the tuples with different join attributes will be mixed again,
though they have already been grouped automatically by Hadoop.
For avoiding this, in reduce functions,
we send a special tuples as separate tuples(~ST~)
to divide tuples within different hash buckets.

After above procedure, ~parahashjoin~ can easily get tuples
inside one hash bucket with the help of ~ST~.
For each hash bucket, ~parahashjoin~ use the key field ~SI~ to
distinguish tuples from different source relations.
And since all tuples inside have a same join attribute value already,
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

  CHECK_COND(nl->ListLength(args) == 3, "Expect 3 arguments");

  ListExpr stream = nl->First(args);
  ListExpr relA = nl->Second(args);
  ListExpr relB = nl->Third(args);

  CHECK_COND(listutils::isTupleStream(stream)
    && listutils::isRelDescription(relA)
    && listutils::isRelDescription(relB),
    "Expect input as stream(tuple) x rel(tuple) x rel(tuple)");

  ListExpr streamTupleList = nl->Second(nl->Second(stream));
  CHECK_COND(nl->ListLength(streamTupleList) == 1
  && listutils::isSymbol(nl->Second(nl->First(streamTupleList)),TEXT),
  "Expect input stream as (stream(tuple((value text))))");

  ListExpr rAtupNList =
      renameList(nl->Second(nl->Second(relA)), "1");
  ListExpr rBtupNList =
      renameList(nl->Second(nl->Second(relB)), "2");
  ListExpr resultAttrList = ConcatLists(rAtupNList, rBtupNList);
  ListExpr resultList = nl->TwoElemList(nl->SymbolAtom("stream"),
        nl->TwoElemList(nl->SymbolAtom("tuple"), resultAttrList));

  return resultList;
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
    cout << nl->ToString(tuple) << endl;
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

  switch (message)
  {
  case OPEN:
    qp->Open(args[0].addr);

    localInfo = new phjLocalInfo(args[0], s);
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

phjLocalInfo::phjLocalInfo(Word _stream, Supplier s)
{
  mixStream = _stream;

  ListExpr resultType = GetTupleResultType(s);
  resultTupleType = new TupleType(nl->Second(resultType));
  aTypeInfo =
      SecondoSystem::GetCatalog()->NumericType( nl->OneElemList(
      nl->Second(qp->GetSupplierTypeExpr(qp->GetSon(s,1)))));
  bTypeInfo =
      SecondoSystem::GetCatalog()->NumericType( nl->OneElemList(
      nl->Second(qp->GetSupplierTypeExpr(qp->GetSon(s,2)))));

  joinedTuples = 0;
  getNewProducts();
}

/*
Ask for new tuples from ~joinedTuples~.
If there's no more tuples inside ~joinedTuples~,
then invoke ~getNewProducts~ to get new results.

*/
Word phjLocalInfo::nextJoinTuple()
{
  Tuple *tuple;
  if ((tuple = tupleIterator->GetNextTuple()) != 0)
  {
    return SetWord(tuple);
  }
  else if(getNewProducts())
  {
    tuple = tupleIterator->GetNextTuple();
    return SetWord(tuple);
  }
  else
  {
    return SetWord(Address(0));
  }
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

bool phjLocalInfo::getNewProducts()
{

  TupleBuffer *tbA = 0;
  TupleBuffer *tbB = 0;
  Tuple *tupleA = 0, *tupleB = 0;
  string tupStr;
  ListExpr tupList, valList;
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
      tupStr = ((FText*) (currentTuple->GetAttribute(0)))->GetValue();
      nl->ReadFromString("(" + tupStr + ")", tupList);
      int SI = NList(tupList).first().intval();
      valList = nl->Second(tupList);
      int errorPos;
      ListExpr errorInfo;
      bool correct;
      switch (SI)
      {
      case 1:{
        tupleA = Tuple::In(aTypeInfo, valList, errorPos,
            errorInfo, correct);
        tbA->AppendTuple(tupleA);
        break;
      }
      case 2:{
        tupleB = Tuple::In(bTypeInfo, valList, errorPos,
            errorInfo, correct);
        tbB->AppendTuple(tupleB);
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


    if(tbA->GetNoTuples() == 0 && tbB->GetNoTuples() == 0)
    {
      // No more data exists
      delete tbA;
      delete tbB;
      return false;
    }
    else if(tbA->GetNoTuples() == 0 || tbB->GetNoTuples() == 0)
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

      for (int i = 0; i < tbA->GetNoTuples(); i++)
      {
        for (int j = 0; j < tbB->GetNoTuples(); j++)
        {
          tupleA = tbA->GetTuple(i);
          tupleB = tbB->GetTuple(j);

          Tuple *resultTuple = new Tuple(resultTupleType);
          Concat(tupleA, tupleB,resultTuple);
          joinedTuples->AppendTuple(resultTuple);
        }
      }
      delete tbA;
      delete tbB;

      tupleIterator = joinedTuples->MakeScan();
      return true;
    }
  }
  return false;
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



----
    (  (stream(tuple((value text))))
     x (rel(tuple(T1))) x (rel(tuple(T2)))
     x (map t r)  )
     -> stream(tuple(T1 T2))
----


*/

struct paraJoinInfo : OperatorInfo
{
  paraJoinInfo()
  {
    name = "parajoin";
    signature = "( (stream(tuple((key int)(value text))))"
                 "x(rel(tuple(T1))) x (rel(tuple(T2)))"
                 "x(map t r) )"
                 " -> stream(tuple(T1 T2))";
    syntax = "_ _ _ parajoin [funlist]";
    meaning = "join mixed tuples from two relations";
  }
};

ListExpr paraJoinTypeMap( ListExpr args )
{
  CHECK_COND(nl->ListLength(args) == 4,
    "Expect four arguments");

  ListExpr streamList = nl->First(args);
  ListExpr relAList = nl->Second(args);
  ListExpr relBList = nl->Third(args);
  ListExpr namedMap = nl->Fourth(args);

  CHECK_COND(listutils::isTupleStream(streamList)
    && listutils::isRelDescription(relAList)
    && listutils::isRelDescription(relBList),
    "Expect (stream(tuple((value text))))"
          "x(rel(tuple(T1))) x (rel(tuple(T2)))"
          "x((map t r)))");

  ListExpr attrList = nl->Second(nl->Second(streamList));
  CHECK_COND(nl->ListLength(attrList) == 1
    && listutils::isSymbol(nl->Second(nl->First(attrList)),TEXT),
    "Expect input stream as (stream(tuple((value text))))");

  //The map indicates the join method for tuples in the same bucket
  CHECK_COND(listutils::isMap<2>(namedMap),
      "Expect two arguments in the map.");

  ListExpr rAtupNList =
        renameList(nl->Second(nl->Second(relAList)), "1");
  ListExpr rBtupNList =
        renameList(nl->Second(nl->Second(relBList)), "2");
  ListExpr resultAttrList = ConcatLists(rAtupNList, rBtupNList);
  ListExpr resultList = nl->TwoElemList(nl->SymbolAtom("stream"),
        nl->TwoElemList(nl->SymbolAtom("tuple"), resultAttrList));

  return resultList;
}

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
        SecondoSystem::GetCatalog()->NumericType( nl->OneElemList(
        nl->Second(qp->GetSupplierTypeExpr(qp->GetSon(s,1)))));
    bTupleTypeList =
        SecondoSystem::GetCatalog()->NumericType( nl->OneElemList(
        nl->Second(qp->GetSupplierTypeExpr(qp->GetSon(s,2)))));

    localInfo = new pjLocalInfo(args[0], args[3].addr, s,
        aTupleTypeList, bTupleTypeList);

    local.setAddr(localInfo);
    return 0;
  }
  case REQUEST:{

    // ask the fun to get the result tuple.
    if (local.addr == 0)
      return CANCEL;
    localInfo = (pjLocalInfo*) local.addr;

    result = localInfo->getNextTuple();
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

    result = localInfo->getNextInputTuple(tupBufferA);
    if ( result.addr != 0)
      return YIELD;
    else
      return CANCEL;
  }
  case (2*FUNMSG)+REQUEST:{
    if (local.addr == 0)
      return CANCEL;
    localInfo = (pjLocalInfo*) local.addr;

    result = localInfo->getNextInputTuple(tupBufferB);
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
Load the tuples from the input tuple stream,
and fill the tuples within a same bucket into the tupleBuffers,
if there is only one kind of tuple in that bucket,
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
  string tupStr;
  ListExpr tupList, valList;

  if(tbA != 0)
    delete tbA;
  tbA = 0;
  if(tbB != 0)
    delete tbB;
  tbB = 0;

  while (!endOfStream)
  {
    tbA = new TupleBuffer(maxMem / 2);
    tbB = new TupleBuffer(maxMem / 2);
    isBufferFilled = false;
    isInBucket = true;

    qp->Request(mixedStream.addr, cTupleWord);
    while (isInBucket && qp->Received(mixedStream.addr))
    {
      cTuple = static_cast<Tuple*> (cTupleWord.addr);
      tupStr = ((FText*) (cTuple->GetAttribute(0)))->GetValue();
      nl->ReadFromString("(" + tupStr + ")", tupList);
      int SI = NList(tupList).first().intval();
      valList = nl->Second(tupList);
      int errorPos;
      ListExpr errorInfo;
      bool correct;
      switch (SI)
      {
      case 1:
      {
        tupleA = Tuple::In(aTypeInfo, valList, errorPos,
            errorInfo, correct);
        tbA->AppendTuple(tupleA);
        break;
      }
      case 2:
      {
        tupleB = Tuple::In(bTypeInfo, valList, errorPos,
            errorInfo, correct);
        tbB->AppendTuple(tupleB);
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
      tpIndex_A = tpIndex_B = 0;
      isBufferFilled = true;
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
Word pjLocalInfo::getNextInputTuple(tupleBufferType tbt)
{
  Tuple* tuple = 0;

  if(tbt == tupBufferA){
    if (tpIndex_A >= 0 && tpIndex_A < tbA->GetNoTuples())
      tuple = tbA->GetTuple(tpIndex_A++);
  }
  else{
    if (tpIndex_B >= 0 && tpIndex_B < tbB->GetNoTuples())
      tuple = tbB->GetTuple(tpIndex_B++);
  }

  return SetWord(tuple);
}

Word pjLocalInfo::getNextTuple()
{
  Word funResult(Address(0));

  while (!endOfStream)
  {
    qp->Request(JNfun, funResult);
    if (funResult.addr){
      return funResult;
    }
    else if (endOfStream) {
      qp->Close(JNfun);
      return SetWord(Address(0));
    }
    else {
      // No more result in current bucket, move to the next
      qp->Close(JNfun);
      loadTuples();
      if (isBufferFilled)
        qp->Open(JNfun);
      continue;
    }
  }
  return SetWord(Address(0));
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


    AddOperator(TUPSTREAMInfo(), 0, TUPSTREAMType);
    AddOperator(TUPSTREAM2Info(), 0, TUPSTREAM2Type);
    AddOperator(TUPSTREAM3Info(), 0, TUPSTREAM3Type);

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

