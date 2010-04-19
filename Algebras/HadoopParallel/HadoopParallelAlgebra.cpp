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

This algebra implements all relevant operators of integrating
Hadoop and Secondo together to execute some parallel work.
The new operators include:

  * ~doubleexport~.

  * ~paraJoin~.

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
relations into one relation following the (key:string, value:text) pair schema.
The operator extracts the operand field values out as the keys,
and puts the complete original tuple as ~tupleVal~ and
an integer number ~SI~(1 or 2) that is used to denote
which source relation the ~tupleVal~ comes from,
together as the text value of the result tuple.
Hence, after mixing the relations, the MapReduce model can partition
the tuples that may come from different source relation but with a same key value,
together into one reduce function. And in the reduce function, we can call
Secondo to process some queries only for these tuples within a same key range.

---- ((stream (tuple((a1 t1) ... (ai string) ... (an tm))))
(stream (tuple((b1 p1) ... (bj string) ... (bm pm)))) ai bj )
-> ((stream (tuple (key: string) (value: text)))
APPEND (i j))
----

*/

struct doubleExportInfo : OperatorInfo
{
  doubleExportInfo()
  {
    name = "doubleexport";
    signature = "stream (tuple((a1 t1) ... (ai ti) ... (an tm)))"
              "x stream (tuple((b1 p1) ... (bj tj) ... (bm tm)))"
              "x ai x bj -> "
              "stream (tuple (key:string) (value:string))";
    syntax = "_ _ doubleexport[_ , _]";
    meaning = "Mix two relations into (key, value) pairs";
  }
};

/*

2.1 Type Mapping of ~doubleexport~

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
  int attrAIndex = listutils::findAttribute(tupListA,attrAName,attrTypeA);
  CHECK_COND(attrAIndex > 0,
    "Attributename " + attrAName + " not found in the first argument");
  ListExpr tupListB = nl->Second(nl->Second(nl->Second(args)));
  string attrBName = nl->SymbolValue(nl->Fourth(args));
  int attrBIndex = listutils::findAttribute(tupListB,attrBName,attrTypeB);
  CHECK_COND(attrBIndex > 0,
    "Attributename " + attrBName + " not found in the second argument");

  CHECK_COND(nl->Equal(attrTypeA, attrTypeB), "Expect equal key types.");

  ListExpr attrList = nl->TwoElemList(
      nl->TwoElemList(nl->StringAtom("key",false),nl->SymbolAtom(STRING)),
      nl->TwoElemList(nl->StringAtom("value",false),nl->SymbolAtom(TEXT)));
  NList AttrList(attrList, nl);
  NList tupleStreamList = NList(NList().tupleStreamOf(AttrList));

  return nl->ThreeElemList(
               nl->SymbolAtom("APPEND"),
               nl->TwoElemList(nl->IntAtom(attrAIndex),
                               nl->IntAtom(attrBIndex)),
               tupleStreamList.listExpr());
}


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

Get tuples from streamA first, and set their signature number as 1.
After the tuples in streamA are exhausted, then get all tuples from streamB,
and set their signature number as 2.

*/
Tuple* deLocalInfo::nextResultTuple()
{
  Word result(Address(0));

  //Get the tuples from A first;
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
                            ListExpr typeInfo, int sig)
{
  bool yield = false;
  Word result(Address(0));
  Tuple* oldTuple, *newTuple;

  qp->Request(stream.addr, result);
  yield = qp->Received(stream.addr);

  if (yield){
    //Get a tuple from the stream;
    oldTuple = static_cast<Tuple*>(result.addr);
    string key = ((CcString*)(oldTuple->GetAttribute(index)))->GetValue();
    ListExpr oldTupleNL = oldTuple->Out(typeInfo);

    ListExpr valueNL = nl->TwoElemList(nl->IntAtom(sig),oldTupleNL);
    string valueStr = nl->ToString(valueNL);

    newTuple = new Tuple(resultTupleType);
    newTuple->PutAttribute(0,new CcString(key));
    newTuple->PutAttribute(1,new FText(true,valueStr));

    result.setAddr(newTuple);
  }

  return result;
}

/*

2.2 Value Mapping of ~doubleexport~

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

    localInfo = new deLocalInfo(args[0], args[4], args[1], args[5], s);
    local = SetWord(localInfo);
    return 0;
  case REQUEST:

    localInfo = (deLocalInfo*) local.addr;
    result.setAddr(localInfo->nextResultTuple());

    return result.addr !=0 ? YIELD : CANCEL;

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

3 Operator ~parahashjoin~

Together with ~doubleexport~ operator, Hadoop have already automatically
finish the hash partition period of a hash join, only the tuples
from different source relations but have same join attribute value,
i.e. inside a same hash bucket will be processed in one reduce function.
However, the number of the tuples inside a hash bucket may be
very small, calling Secondo every time in reduce functions just to process
a few number of tuples is not an efficient solution.
Therefore, in the reduce function, we decide to just send
the tuples into Secondo, and invoke Secondo only once to process the
join operation at last.
At the same time, since the keys that Hadoop uses to sort and partition
the tuples into different hash buckets are useless in reduce functions,
they will be abandoned, and only the value parts of the tuples outputed
from ~doubleexport~ operation will be sent into Secondo following
the schema: (SI:int, tupleVal:text).
But if we send this kind of tuples back to secondo, the tuples with different
join attributes will be mixed again, though they have already been distinguished
automatically by Hadoop. For avoiding this,
we send a special tuples as separate tuples(SP) to distinguish
the tuples of different hash buckets.

After above procedure, the operator ~parahashjoin~ is used to process
the tuples sent back to Secondo. With the help of SP, it can easy get tuples
inside one hash bucket. And for each hash bucket, since all tuples inside
it have a same join attribute,
the only join operation needed here is a Cartesian product
of tuples from different source relations.

The signature of this operator is:

---- ( (stream (tuple((key:int) (value:text))))
       x (rel(tuple((a1 t1) ... (an tn))))
       x (rel(tuple((b1 p1) ... (bm pm))))
-> stream(tuple((a1 t1) ... (an tn)(b1 p1) ... (bm pm))))
----

The two input rels is used to provide the result schema.

*/

struct paraHashJoinInfo : OperatorInfo
{
  paraHashJoinInfo()
  {
    name = "parahashjoin";
    signature = "stream(tuple((key:int) (value:text)))"
              "x (rel(tuple((a1 t1) ... (an tn))))"
              "x (rel(tuple((b1 p1) ... (bm pm))))"
              "-> stream(tuple((a1 t1) ... (an tn)(b1 p1) ... (bm pm)))";
    syntax = "_ _ _ parahashjoin";
    meaning = "Process tuples from two different relations";
  }
};

/*

3.1 Type Mapping of ~parahashjoin~

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
  CHECK_COND(
       listutils::isSymbol(nl->Second(nl->First(streamTupleList)), INT)
    && listutils::isSymbol(nl->Second(nl->Second(streamTupleList)),TEXT),
    "Expect input stream as (stream(tuple ((key:int) (value:text))))");


  //rename the attributes in both relations to avoid duplication of names
  ListExpr rAtupNList = renameList(nl->Second(nl->Second(relA)), "1");
  ListExpr rBtupNList = renameList(nl->Second(nl->Second(relB)), "2");
  ListExpr resultAttrList = ConcatLists(rAtupNList, rBtupNList);
  ListExpr resultList = nl->TwoElemList(nl->SymbolAtom("stream"),
        nl->TwoElemList(nl->SymbolAtom("tuple"), resultAttrList));

  //return nl->SymbolAtom(INT);
  return resultList;
}

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

3.2 Value Mapping of ~parahashjoin~

*/
int paraHashJoinValueMap(Word* args, Word& result,
                int message, Word& local, Supplier s)
{

/*  result.setAddr(new CcInt(0));
  return 0;*/

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

phjLocalInfo::phjLocalInfo(Word _stream, Supplier s)
{
  mixStream = _stream;

  ListExpr resultType = GetTupleResultType(s);
  resultTupleInfo = nl->OneElemList(nl->Second(resultType));

  joinedTuples = 0;
  count = 0;
  getNewProducts();
}

/*

Make the products of tuples in one bucket.
If the tuples of one bucket all come from a same source relation,
then jump to next bucket. Or else, make the products, and put
the result tuples into the ~joinedTuples~.

*/
bool phjLocalInfo::getNewProducts()
{
  cout << "Called to making new products" << endl;

  TupleBuffer *tbA = 0;
  TupleBuffer *tbB = 0;
  long MaxMem = qp->MemoryAvailableForOperator();

  while(true)
  {
    tbA = new TupleBuffer(MaxMem);
    tbB = new TupleBuffer(MaxMem);

    Word currentTupleWord(Address(0));
    bool isInBucket = true;

    qp->Request(mixStream.addr, currentTupleWord);
    while(qp->Received(mixStream.addr))
    {
      Tuple* currentTuple = static_cast<Tuple*> (currentTupleWord.addr);
      CcInt* SI = static_cast<CcInt*> (currentTuple->GetAttribute(0));

      switch (SI->GetIntval())
      {
      case 0:{
        tbA->AppendTuple(currentTuple);
        break;
      }
      case 1:{
        tbB->AppendTuple(currentTuple);
        break;
      }
      case 2:{
        isInBucket = false;
        break;
      }
      default:{
        //should never be here
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
      // There is no join results in this bucket
      delete tbA;
      delete tbB;
    }
    else
    {
      //compute the products
      if(joinedTuples != 0)
        delete joinedTuples;
      joinedTuples = new TupleBuffer(MaxMem);

      Tuple *tupleA, *tupleB;
      for(int i = 0; i < tbA->GetNoTuples(); i++)
      {
        for(int j = 0; j < tbB->GetNoTuples(); j++)
        {
          tupleA = tbA->GetTuple(i);
          tupleB = tbB->GetTuple(j);

          string tupAStr, tupBStr;
          tupAStr = ((FText*) (tupleA->GetAttribute(1)))->GetValue();
          tupBStr = ((FText*) (tupleB->GetAttribute(1)))->GetValue();
          string resultTupleStr = "(" + tupAStr + " " + tupBStr + ")";
          ListExpr resultTupleNL;
          nl->ReadFromString(resultTupleStr, resultTupleNL);

          int errorPos;
          ListExpr errorInfo;
          bool correct;
          Tuple *resultTuple = Tuple::In(resultTupleInfo, resultTupleNL,
              errorPos, errorInfo, correct);
          joinedTuples->AppendTuple(resultTuple);
        }
      }

      delete tbA;
      delete tbB;

      cout << "result contains: " << joinedTuples->GetNoTuples() << endl;
      tupleIterator = joinedTuples->MakeScan();
      return true;
    }
  }
  return false;
}
/*

If there exists tuples inside ~joinedTuples~,
then just get the next tuple from that.
Or else invoke ~getNewProducts~ to compute new results.

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

3 Class ~HadoopParallelAlgebra~

A new subclass ~HadoopParallelAlgebra~ of class ~Algebra~ is declared.
The only specialization with respect to class ~Algebra~ takes place within
the constructor: all type constructors and operators are registered at the
actual algebra.

After declaring the new class, its only instance ~extendedRelationAlgebra~
is defined.

*/

class HadoopParallelAlgebra : public Algebra
{
public:
  HadoopParallelAlgebra() :
    Algebra()
  {

    AddOperator(doubleExportInfo(),doubleExportValueMap,doubleExportTypeMap);
    AddOperator(paraHashJoinInfo(),paraHashJoinValueMap,paraHashJoinTypeMap);
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
InitializeHadoopParallelAlgebra(NestedList* nlRef, QueryProcessor* qpRef)
{
  nl = nlRef;
  qp = qpRef;
  return (new HadoopParallelAlgebra());
}


