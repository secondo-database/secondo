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

using namespace std;

extern NestedList* nl;
extern QueryProcessor* qp;
extern AlgebraManager *am;


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

    ListExpr tupListB = nl->Second(tupTypeB);
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
          nl->TwoElemList(nl->StringAtom("KeyT",false),
              nl->SymbolAtom(CcString::BasicType())),
          nl->TwoElemList(nl->StringAtom("ValueT",false),
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
    local.setAddr(0);
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

Update at Spetemper 2012
Allow to pass stream(tuple(...)) as well, hence to avoid creating relation objects in slave databases.
Now maps:

----
    ( (rel(T1)) ... ) -> stream(T1) |
    ( (stream(T1)) ... ) -> stream(T1)
----

The same update is done for ~TUPLESTREAM2~ and ~TUPLESTREAM3~

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
  if (!listutils::isRelDescription(first)
          && !listutils::isTupleStream(first))
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
  if (!listutils::isRelDescription(second)
          && !listutils::isTupleStream(second))
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
  if (!listutils::isRelDescription(third) && !listutils::isTupleStream(third))
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


Update at September 2012.
Allow the two schema relations can replaced with tuple streams,
hence to be able to accept tuple types from operators like ~ffeed~.

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
      && ( listutils::isRelDescription(relAList)
        || listutils::isTupleStream(relAList))
      && ( listutils::isRelDescription(relBList)
        || listutils::isTupleStream(relBList)))
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
    local.setAddr(0);

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
    dataServers(0),
    localNode(-1), masterNode(0)
{
  available = false;

  //Scan both master and slave lists,
  //and build up a machine list, which insert the master first.
  for (int i = 0; i < 2; i++)
  {
    bool isMaster = ( (0 == i) ? true : false);
    if ( 0 == i )
      dataServers = new vector<dservDesc>();

    char *ev;
    ev = isMaster ?
        getenv(ps_master.c_str()) :
        getenv(ps_slaves.c_str());

    if ( 0 == ev ){
      //cerr << "Environment variable "
      //     << ( isMaster ? ps_master : ps_slaves )
      //     << " is not correctly defined." << endl;
      return;
    }

    string fileName = string(ev);
    if (fileName.length() == 0){
       // cerr << "Environment variable "
       //      << (isMaster ? ps_master : ps_slaves)
       //      << " is set as empty." << endl;
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
      if (dataServers->size() > 0)
      {
        int csn = dataServers->size();  // Current node series number
        for (vector<dservDesc>::iterator dit = dataServers->begin();
             dit != dataServers->end(); dit++)
        {
          if ( 0 == dit->first.compare(ipAddr) &&
               (dit->second.second == port) &&
               0 == dit->second.first.compare(cfPath))
          {
            if ( dit == dataServers->begin())
              masterNode = csn;
            else
              noRepeat = false;
          }
        }
      }
      if (noRepeat){
        dataServers->push_back(dservDesc(
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
      if (dataServers->size() != 1)
      {
        cerr << "Master list requires one line" << endl;
        return;
      }
    }
    else
    {
      if (dataServers->size() < 2)
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
  dataServers = new vector<dservDesc>();
  if(rhg.dataServers){
     vector<dservDesc>::iterator iter = rhg.dataServers->begin();
     while (iter != rhg.dataServers->end()){
       dataServers->push_back(dservDesc(iter->first,
         pair<string, int>(iter->second.first, iter->second.second)));
       iter++;
     }
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

    if ((dataServers->at(index).first.compare(IPAddr) != 0)
     || (dataServers->at(index).second.first.compare(filePath) != 0)
     || (dataServers->at(index).second.second != port))
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
      for(vector<dservDesc>::iterator dit = dataServers->begin();
          dit != dataServers->end(); dit++)
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

  string rfPath = (*dataServers)[loc].second.first;
  if (appendFileName){
    if (attachProducerIP)
    {
      if (producerIP.length() == 0)
        producerIP = getLocalIP();
      fileName += ("_" + producerIP);
    }
    FileSystem::AppendItem(rfPath, fileName);
  }

  string IPAddr = (*dataServers)[loc].first;
  remotePath = (appendTargetIP ? (IPAddr + ":") : "") + rfPath;

  return remotePath;
}

string clusterInfo::getMSECPath(size_t loc,
    bool includeMaster, /*= true*/
    bool round,         /*= false*/
    bool appendIP)      /*= true*/
{
  loc = getInterIndex(loc, includeMaster, round);
  string psfsPath = (*dataServers)[loc].second.first;
  string msecPath = FileSystem::GetParentFolder(psfsPath);
  FileSystem::AppendItem(msecPath, "msec");

  if (appendIP)
  {
    string ip = (*dataServers)[loc].first;
    msecPath = ip + ":" + msecPath;
  }
  return msecPath;
}

string clusterInfo::getIP(size_t loc, bool round /* = false*/)
{
  if ( 0 == loc)
    loc = masterNode;
  loc = getInterIndex(loc, true, round);
  return (*dataServers)[loc].first;
}

size_t clusterInfo::getInterIndex(
    size_t loc, bool includeMaster, bool round){
  assert(dataServers->size() > 1);

  if (!round){
    assert(loc < dataServers->size());
    return loc;
  }
  else{
    if (!includeMaster){
      assert(loc > 0);
      return ((loc - 1) % (dataServers->size() - 1) + 1);
    }
    else{
      return (loc % dataServers->size());
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
    vector<dservDesc>::iterator iter;
    for (iter = dataServers->begin(); iter != dataServers->end(); iter++, cnt++)
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
    vector<dservDesc>::iterator iter;
    for (iter = dataServers->begin(); iter != dataServers->end();
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
    vector<dservDesc>::iterator iter = dataServers->begin();
    int counter = 0;
    while (iter != dataServers->end())
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

On 02/05/2013, let ~fconsume~ and ~fconsume2~ share a same type mapping function.
They are distinguished by the noFlob parameter.
If it is set true, then for an attribute that contain FLOB data by its definition,
its type A should be changed to incomplete(A).


*/
ListExpr FConsumeTypeMap(ListExpr args, bool noFlob)
{
  try{
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
     if (typeFile.good())
     {
       NList resultList;
       if (noFlob)
       {
         //Add the incomplete property on attributes containing Flob.

         NList attrList = tsList.first().second().second();
         NList newAttrList = addIncomplete(attrList);

         resultList = NList(NList(Relation::BasicType()),
             NList(NList(Tuple::BasicType(), newAttrList)));
       }
       else
       {
         resultList =
             NList(NList(Relation::BasicType()), tsList.first().second());
       }
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
  } catch(...){
      return listutils::typeError("invalid input");
  }
}

ListExpr FConsume1TypeMap(ListExpr args)
{
  return FConsumeTypeMap(args, false);
}

/*
5.4 Value mapping

*/
int FConsumeValueMap(Word* args, Word& result,
    int message, Word& local, Supplier s, bool noFlob)
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
    SmiRecordId sourceDS = 0;
    if (drMode || noFlob)
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
      sourceDS = ci->getLocalNode();
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

    SmiFileId flobFileId;
    string flobFilePath;
    ofstream flobFile;
    if (noFlob)
    {
      //Prepare the fileId for the Flob file
      do{
        flobFileId = WinUnix::rand() + WinUnix::getpid();
        flobFilePath = getLocalFilePath(
            "", "flobFile", "_" + int2string(flobFileId));
      }
      while (FileSystem::FileOrFolderExists(flobFilePath));

      flobFile.open(flobFilePath.c_str(), ios::binary);
      if (!flobFile.good())
      {
        cerr << "ERROR!Create Flob file " << flobFilePath << " fail!" << endl;
        ((CcBool*)(result.addr))->Set(true, false);
        return 0;
      }
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
    SmiSize flobBlockOffset = 0;

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

      if (noFlob)
      {
/*
Prepare for ~fconsume3~, split the tuple data into two parts.
The core and extension data is kept in the file named by the given parameters.
This file is stored with the type in the indicated path.

The flob data is kept in another binary file.
The file is only used within the intermediate part of Hadoop qureies,
hence it must be kept in PSFS nodes, and its name is: flobFile\_fileID.

The fileID is a 32bit integer, decided by a {random value} + {the current process id}
If the file with the id exists, then use another random value.

In this case, the flobId keeps the following information:

----
fileId: the id used as the suffix of the flob file
recordId: indicates the sourceDS
mode: 3, indicate it is a remote mode
offset: the offset within the flob file
----

Note here the recordId indicates the source DS, i.e. on which DS the flob is generated.
Regarding the fault-tolerance feature, the flob file can also be duplicated in
several continuous Data Servers.
In this case, the flob file is named: flobFile\_fileID\_sourceIP, like the normal data file.
On how many DSs to fetch the duplicated flob file is decided by the ~fetchFlob~ operator.

*/
        totalSize     += (coreSize + extensionSize);
        totalExtSize  += (coreSize + extensionSize);

        char* tBlock = (char*)malloc(tupleBlockSize);
        t->WriteToDivBlock(tBlock, coreSize, extensionSize, flobSize,
            flobFileId, sourceDS, flobBlockOffset);
        blockFile.write(tBlock, (tupleBlockSize - flobSize));
        size_t flobOffset = tupleBlockSize - flobSize;
        flobFile.write(tBlock + flobOffset, flobSize);
        free(tBlock);
      }
      else
      {
        //Prepare for fconsume, keeping Flob with tuple
        totalSize += (coreSize + extensionSize + flobSize);
        totalExtSize += (coreSize + extensionSize);

        char* tBlock = (char*)malloc(tupleBlockSize);
        t->WriteToBin(tBlock, coreSize, extensionSize, flobSize);
        blockFile.write(tBlock, tupleBlockSize);
        free(tBlock);
      }
      count++;
      fcli->current++;

      t->DeleteIfAllowed();
      qp->Request(args[0].addr, wTuple);
    }

    if (flobFile.is_open()){
      flobFile.close();
      cout << "Flob file: " << flobFilePath << " is created. " << endl;
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
    cout << "\nData file: " << filePath << " is created. " << endl;

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
            if (0 != copyFile(filePath, rPath, true))
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

int FConsume1ValueMap(Word* args, Word& result,
    int message, Word& local, Supplier s)
{
  return FConsumeValueMap(args, result, message, local, s, false);
}

Operator fconsumeOp(FConsumeInfo(),
    FConsume1ValueMap, FConsume1TypeMap);

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
x [producerIndex x targetNodeIndex x attemptTimes]  ;
->stream(tuple(...))
----

Update Dec. 2013 - Jan. 2014
Jiamin Lu

Extend operator ~ffeed2~ and ~ffeed3~ based on the typemapping of ~ffeed~.
Remove the added DS\_IDX attribute.


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

The ~noFlob~ stands for whether exist Flob in the disk file

*/

ListExpr FFeedTypeMap(ListExpr args, bool noFlob)
{
  try{
     NList l(args);
     NList pType, pValue;
   
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
     }
   
     bool delTypeFile = false;
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
//       cerr << "Copy the type file " << filePath
//           << " from <-" << "\t" << rPath << endl;
       int atimes = MAX_COPYTIMES;
       int rc = 0;
       while (atimes-- > 0){
         rc = system((scpCommand + rPath + " " + filePath).c_str());
         if (0 == rc){
           delTypeFile = true;
           break;
         } else {
           WinUnix::sleep(1);
         }
       }
       if (rc != 0)
         return l.typeError(err11);
     }
   
     ListExpr relType;
     if (!nl->ReadFromFile(filePath, relType))
       return l.typeError(err2 + filePath);
     if (delTypeFile){
       FileSystem::DeleteFileOrFolder(filePath);
     }

     //Read type file of DLF flist
     NList resultType;
     string ostStr, nstStr;
     //ost: old stream type, type from the type file, no DS\_IDX
     //nst: new stream type, for ffeed3, with DS\_IDX
     if (noFlob)
     {
       //Ignore the incomplete term
       int count = 0;
       ListExpr realRelType = rmTermNL(relType, "incomplete", count);
       if (!(listutils::isRelDescription(realRelType)
              || listutils::isTupleStream(realRelType)))
         return l.typeError(err3 + filePath);
       NList osType = NList(NList(Symbol::STREAM()),
                       NList(NList(realRelType).second()));
       ostStr = osType.convertToString();

/*
The DS\_IDX attribute is not extended anymore, hence the old and the new
stream type should be the same.

*/
       NList resultAttrList = NList(nl->Second(nl->Second(realRelType)));

       // remove incomplete
       resultType =
           NList(NList(Symbol::STREAM()),
               NList(NList(Tuple::BasicType()), resultAttrList));
       nstStr = resultType.convertToString();
       assert(nstStr.compare(ostStr) == 0);
     }
     else
     {
       if (!(listutils::isRelDescription(relType)
         || listutils::isTupleStream(relType)))
         return l.typeError(err3 + filePath);
       resultType =
           NList(NList(Symbol::STREAM()),
           NList(NList(relType).second()));
       nstStr = ostStr = resultType.convertToString();
     }

     return NList(NList(Symbol::APPEND()),
                  NList(NList(ostStr, true, true), NList(nstStr, true, true)),
                  resultType).listExpr();
   
  } catch(...){
    return listutils::typeError("invalid input");
  }
}

ListExpr FFeed1TypeMap(ListExpr args)
{
  return FFeedTypeMap(args, false);
}


/*
6.4 Value mapping

17th Sept. 2013

There are three modes accepted by this function:

  * 1: Normal mode, reads the complete tuple from data file
  * 2: Part mode, reads tuple from data file, and leave FLOB untouched
  * 3: Separate mode, reads tuple and FLOB from two separate files.

*/
int FFeedValueMap(Word* args, Word& result,
    int message, Word& local, Supplier s, int mode)
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

      if (mode < 3)
        ffli = new FFeedLocalInfo(s, false, prdIndex, filePath);
      else if (mode == 3)
        ffli = new FFeedLocalInfo(s, true, prdIndex, filePath);
      else
      {
        delete ffli;
        ffli = 0;
        local.setAddr(0);
      }

      if (ffli->fetchBlockFile(
          relName , fileSuffix, s,
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

      Tuple *t = 0;
      switch(mode)
      {
        case 1: {t = ffli->getNextTuple(); break;}
        case 2: {t = ffli->getNextTuple2(); break;}
        case 3: {t = ffli->getNextTuple3(); break;}
      }

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

int FFeed1ValueMap(Word* args, Word& result,
    int message, Word& local, Supplier s)
{
  return FFeedValueMap(args, result, message, local, s, 1);
}

Operator ffeedOp(FFeedInfo(), FFeed1ValueMap, FFeed1TypeMap);

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
    string fileName, string fileSuffix, Supplier s,
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

  FileSystem::AppendItem(filePath, fileName + fileSuffix);

/*
Detect whether the file is exist or not.
If the file exists, the fileFound is set as true,
and the filePath contains the complete local path of the file.
Or else, the fileFound is false.

*/
  if (pdi < 0)
  {
    //Fetch the file in the local machine
    fileFound = isLocalFileExist(filePath);
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
        lFilePath = FileSystem::MakeTemp(filePath);
        string cStr = scpCommand + rFilePath +
            " " + lFilePath;
        while (!fileFound && copyTimes-- > 0){
          if (0 == copyFile(rFilePath, lFilePath, true)){
              break;
          }
        }
      }

      fileFound = isLocalFileExist(lFilePath);
      if (fileFound){
        filePath = lFilePath;
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
    cerr << "\nWarning! File " << filePath
         << " is not exist and cannot be remotely fetched.\n\n\n";
    return false;
  }
  tupleBlockFile = new ifstream(filePath.c_str(), ios::binary);
  if (!tupleBlockFile->good())
  {
    cerr << "Warning! Read file " << filePath << " fail.\n\n\n";
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
  noAttrs = rcdTupleType->GetNoAttributes();
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
/*
The structure of the tuple block is:

blockSize | tupleSize | tuple | Flob

blockSize = sizeof(blockSize) + sizeof(tupleSize) + sizeof(tuple) + sizeof(Flob)
tupleSize = sizeof(tuple)

*/
Tuple* FFeedLocalInfo::getNextTuple()
{
  if (!fileFound)
    return 0;
  assert(!noFlob);

  return readTupleFromFile(tupleBlockFile, rcdTupleType, 1);
}

/*

The function getNextTuple2 is prepared for the operator ~ffeed2~

The block contains both the tuple and the FLOB,
but it leaves the FLOB data untouched.

*/
Tuple* FFeedLocalInfo::getNextTuple2()
{
  if (!fileFound)
    return 0;
  assert(!noFlob);

  return readTupleFromFile(tupleBlockFile, rcdTupleType, 2, filePath);
}

/*
The function getNextTuple3 is prepared for the operator ~ffeed3~

The block contains only the tuple data, while the Flob data is
kept either locally or remotely in persistent Flob files.

*/
Tuple* FFeedLocalInfo::getNextTuple3()
{
  if (!fileFound)
    return 0;
  assert(noFlob);

  Tuple* t = readTupleFromFile(tupleBlockFile, rcdTupleType, 3, filePath);
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
  try{
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
   
   }catch(...){
      return listutils::typeError("invalid input");
   }
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
ListExpr FDistributeTypeMap(ListExpr args, bool noFlob){
  try{
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
   
     bool /*evenMode = false, setKPA = false, */ KPA = false;
     NList pmList = l.third();
     //Partition mode (including [nBuckets], [KPA])
     pType = pmList.first();
     pValue = pmList.second();
     int pmLen = pType.length();
     if (pmLen < 0 || pmLen > 2)
       return l.typeError(err11);
     if (1 == pmLen)
     {
       if (pType.first().isSymbol(CcBool::BasicType()))
       {
         KPA = pValue.first().boolval();
       }
       else if (!pType.first().isSymbol(CcInt::BasicType())){
         return l.typeError(err11);
       }
     }
     else if (2 == pmLen)
     {
       if (!pType.first().isSymbol(CcInt::BasicType()) ||
           !pType.second().isSymbol( CcBool::BasicType()))
         return l.typeError(err11);
       else
       {
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
     if (!typeFile.good())
       return l.typeError(err3 + filePath);
     else
     {
       NList resultList;
       if (noFlob)
       {
         //Add the incomplete property on attributes containing Flob.

         NList newAL2 = addIncomplete(newAL);
         resultList = NList(NList(Relation::BasicType()),
                        NList(NList(Tuple::BasicType(), newAL2)));
       }
       else
       {
         resultList = NList(NList(Relation::BasicType()),
                          NList(NList(Tuple::BasicType()), newAL));
       }
       typeFile << resultList.convertToString() << endl;
       cerr << "Type file: " << filePath << " is created. " << endl;
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
  }catch(...){
    return  listutils::typeError("invalid input");
  }
}

/*
8.2 Value mapping

*/
int FDistributeValueMap(Word* args, Word& result,
    int message, Word& local, Supplier s, bool noFlob)
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

      bool kpa = false;
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
          nBucket = ((CcInt*)qp->Request(
              qp->GetSupplierSon(ptmList,0)).addr)->GetValue();
        }
      }
      else if (2 == ptmLen)
      {
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
               dupTgtIndex, dupTimes, noFlob);
      if (!fdli->isOK()){
        delete fdli;
        return CANCEL;
      }
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
    int _di, int _dt, bool _nf)
: nBuckets(_nb), attrIndex(_ai), kpa(_kpa), tupleCounter(0),
  rowNumSuffix(-1), firstDupTarget(_di), dupTimes(_dt),
  localIndex(0), cnIP(""),
  ci(0), copyList(0), noFlob(_nf), sourceDS(-1), ok(true)
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

  if (dupTimes > 0 || noFlob)
  {
    ci = new clusterInfo();

    if(!ci->isOK()){
      cerr << "ERROR!The slave list file does not exist."
      "Is $PARALLEL_SECONDO_SLAVES correctly set up ?" << endl;
      ok = false;
    }

    if(firstDupTarget > (int)ci->getSlaveSize() ){
      cerr << "The first target node index is "
          "out of the range of slave list" << endl;
      ok = false;
    }
    sourceDS = ci->getLocalNode();
  }
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
        exportTupleType->GetNoAttributes(), rowNumSuffix, noFlob, sourceDS);
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
            if ( 0 == copyFile(filePath, rPath, true)){
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

fileInfo::fileInfo(size_t _cs, string _fp, string _fn,
    size_t _an, int _rs, bool _nf/* = false*/, SmiRecordId _sid/* = -1*/):
  cnt(0), totalExtSize(0),totalSize(0), sourceDS(_sid),
  flobBlockOffset(0), lastTupleIndex(0), fileOpen(false), noFlob(_nf)
{
  //\_fn: fileBaseName
  //\_rs: rowNumberSuffix (string "\_X")
  //\_hv: columnSuffix    (integer)
  //\_fn, \_fp: file name and path
  //\_an: attributes number
  if (_rs >= 0){
    _fn += "_" + int2string(_rs);
  }
  blockFileName = _fn + "_" + int2string(_cs);
  blockFilePath = _fp;
  FileSystem::AppendItem(blockFilePath, blockFileName);
  if (noFlob)
  {
    do{
      flobFileId = WinUnix::rand() + WinUnix::getpid();
      flobFileName = "flobFile_" + int2string(flobFileId);
      flobFilePath = _fp;
      FileSystem::AppendItem(flobFilePath, flobFileName);
    } while (FileSystem::FileOrFolderExists(flobFilePath));
  }

  attrExtSize = new vector<double>(_an);
  attrSize = new vector<double>(_an);
}

bool fileInfo::openFile()
{
  if (fileOpen){
    return true;
  }
  else
  {
    bool fileStatus = false;
    ios_base::openmode mode = ios::binary;
    if (lastTupleIndex > 0)
      mode |= ios::app;
    blockFile.open(blockFilePath.c_str(), mode);
    fileStatus = blockFile.good();
    if (noFlob){
      flobFile.open(flobFilePath.c_str(), mode);
      fileStatus &= flobFile.good();
    }
    fileOpen = fileStatus;
    return fileStatus;
  }
}

void fileInfo::closeFile()
{
  if (fileOpen){
    blockFile.close();
    if (noFlob){
      flobFile.close();
    }
    fileOpen = false;
  }
}

bool fileInfo::writeTuple(Tuple* tuple, size_t tupleIndex,
    TupleType* exTupleType, int ai, bool kai, int aj/* = -1*/,
    bool kaj/* = false*/)
{
  if (!fileOpen)
    return false;

  size_t coreSize = 0;
  size_t extensionSize = 0;
  size_t flobSize = 0;

  //The tuple written to the file need remove the key attribute
  Tuple* newTuple;
  bool keepAll = ((ai >= 0) ? kai : true) && ((aj >= 0) ? kaj : true);

  if (keepAll){
    newTuple = tuple;
  }
  else
  {
    newTuple = new Tuple(exTupleType);
    int j = 0;
    for (int i = 0; i < tuple->GetNoAttributes(); i++)
    {
      if ( (i != ai || kai) && ( i != aj || kaj) )
        newTuple->CopyAttribute(i, tuple, j++);
    }
  }

  size_t tupleBlockSize = newTuple->GetBlockSize(
      coreSize, extensionSize, flobSize, attrExtSize, attrSize);
  if (noFlob)
  {
    totalSize += (coreSize + extensionSize);
    totalExtSize += (coreSize + extensionSize);

    char* tBlock = (char*)malloc(tupleBlockSize);
    newTuple->WriteToDivBlock(tBlock, coreSize, extensionSize, flobSize,
        flobFileId, sourceDS, flobBlockOffset);
    blockFile.write(tBlock, (tupleBlockSize - flobSize));
    size_t flobOffset = tupleBlockSize - flobSize;
    flobFile.write(tBlock + flobOffset, flobSize);
    free(tBlock);
  }
  else
  {
    totalSize += (coreSize + extensionSize + flobSize);
    totalExtSize += (coreSize + extensionSize);

    char* tBlock = (char*)malloc(tupleBlockSize);
    newTuple->WriteToBin(tBlock, coreSize,
                         extensionSize, flobSize);
    blockFile.write(tBlock, tupleBlockSize);
    free(tBlock);
  }

  if (!keepAll)
    newTuple->DeleteIfAllowed();
  lastTupleIndex = tupleIndex + 1;
  cnt++;

  return true;
}

int fileInfo::writeLastDscr()
{
  // write a zero after all tuples to indicate the end.
  u_int32_t endMark = 0;
  blockFile.write((char*)&endMark, sizeof(endMark));

  // build a description list of output tuples
  NList descList;
  descList.append(NList(cnt));
  descList.append(NList(totalExtSize));
  descList.append(NList(totalSize));
  int attrNum = attrExtSize->size();
  for(int i = 0; i < attrNum; i++)
  {
    descList.append(NList((*attrExtSize)[i]));
    descList.append(NList((*attrSize)[i]));
  }

  //put the base64 code of the description list to the file end.
  string descStr = binEncode(descList.listExpr());
  u_int32_t descSize = descStr.size() + 1;
  blockFile.write(descStr.c_str(), descSize);
  blockFile.write((char*)&descSize, sizeof(descSize));

  return cnt;
}

ListExpr FDistribute1TypeMap(ListExpr args)
{
  return FDistributeTypeMap(args, false);
}

int FDistribute1ValueMap(Word* args, Word& result,
    int message, Word& local, Supplier s)
{
  return FDistributeValueMap(args, result, message, local, s, false);
}

Operator fdistributeOp(FDistributeInfo(),
                       FDistribute1ValueMap,
                       FDistribute1TypeMap);

/*
4 Operator ~fdistribute3~

Improve the ~fdistribute~ operator by dividing the data file into two parts,
the tuple file and the flob file, like what the ~fconsume3~ operator does.

There is no ~fdistribute3~ file defined.

*/

struct FDistribute3Info : OperatorInfo {

  FDistribute3Info() : OperatorInfo()
  {
    name = "fdistribute3";
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
        "Generate the split data files, by separating the tuple and flob files";
  }

};


ListExpr FDistribute3TypeMap(ListExpr args)
{
  return FDistributeTypeMap(args, true);
}

int FDistribute3ValueMap(Word* args, Word& result,
    int message, Word& local, Supplier s)
{
  return FDistributeValueMap(args, result, message, local, s, true);
}

Operator fdistribute3Op(FDistribute3Info(),
                       FDistribute3ValueMap,
                       FDistribute3TypeMap);



/*
5 Operator ~fconsume3~

2th May 2013

This is the first operator prepared for the distributed F\/R.
This operator also export a tuple stream into two files, type and data.
However, this time the Flob data are not exported,
but still kept in their original Flob file.

It takes the same arguments as the ~fconsume~ operator takes.
However, I have to extend its type mapping function a little bit.
If an attribute contains Flob data,
then its type A is turned to incomplete(A).


The signature of this operator is:

---- (stream(tuple(...))
      x fileName x filePath x [rowNum] x [fileSuffix] ;
      x [typeLoc1] x [typeLoc2]                       ;
      x [targetLoc x dupTimes])                       ;
     -> bool
----

Update on 15th Oct.
Rename the operator from ~fconsume2~ to ~fconsume3~

Therefore, ~ffeed~ and ~ffeed2~ reads data from files created by ~fconsume~,
~ffeed3~ reads data from files created by ~fconsume3~.


5.1 Specification

*/

struct FConsume3Info : OperatorInfo {
  FConsume3Info() : OperatorInfo()
  {
    name = "fconsume3";
    signature = "stream(tuple( ... )) "
        "x string x text x [int] x [int]"
        "x [ [int] x [int] ] "
        "x [ int x int ] "
        "-> bool";
    syntax  = "stream(tuple( ... )) "
        "fconsume[ fileName, filePath, [rowNum], [fileSuffix]; "
        "[typeNode1] x [typeNode2]; "
        "[targetIndex x dupTimes] ] ";
    meaning =
        "Export a stream of tuples into two files. "
        "One is a text file, keeping the schema of relation, "
        "the other is a binary file, keeping the binary tuple data. "
        "Different from the previous fconsume operator, "
        "this operator only exports a tuple's core and extension data "
        "to the data file, and keeps the Flob untouched. "
        "In the mean time, if an attribute belongs to a type that may contain "
        "Flob data, then its type is then surround by a 'incomplete' term.";
  }
};

/*
5.2 Type mapping

*/
ListExpr FConsume3TypeMap(ListExpr args)
{
  return FConsumeTypeMap(args, true);
}

/*
5.3 Value mapping

*/
int FConsume3ValueMap(Word* args, Word& result,
    int message, Word& local, Supplier s)
{
  return FConsumeValueMap(args, result, message, local, s, true);
}

Operator fconsume3Op(FConsume3Info(), FConsume3ValueMap, FConsume3TypeMap);

/*
5 Operator ~ffeed3~

3th May 2013

This operator reads the files created by the ~fconsume~ or ~fconsume2~ operator.
It accepts a relation schema that contains ~incomplete~ type,
and adds an additional attribute DS\_IDX to the output schema,
indicating the source Data Server that the tuple comes from.
Now the operator maps

----
fileName
x path x [rowNum] x [fileIndex]      ;
x [typeNodeIndex]                   ;
x [producerIndex x targetNodeIndex x attemptTimes]  ;
->stream(tuple(...))
----

17th Sept 2013

rename it from ~ffeed2~ to ~ffeed3~

28th Oct. 2013

the ~incomplete~ should be removed from the result,
the result tuples are not completed if they are fed on the remote computer,
where the ~producerIndex~ is indicated and is not the current computer.
In this case, the DS\_IDX is set and the flob mode is set to be 3.
Or else, the DS\_IDX is set as -1 and the flob mode is kept unchanged.


16th Jan. 2014

Remove the DS\_IDX attribute from the output tuples,
since it happens that a tuple contains Flob data coming from different DSs.

5.1 Specification

*/

struct FFeed3Info : OperatorInfo {

  FFeed3Info() : OperatorInfo()
  {
    name =      "ffeed3";
    signature = "string x text x [int] x [int] x [int] x [int x int x int]"
        " -> stream(tuple(...))";
    syntax  = "fileName ffeed[ filePath, [fileSuffix], ; "
        "[remoteTypeNode]; "
        "[producerIndex x targetIndex x attemptTimes] ]";
    meaning =
        "Restore a tuple stream from a pair of type and data files, "
        "which are created by fconsume3 operator. "
        "It accepts the incomplete keyword, which will be removed";
  }
};

/*
5.2 Type Mapping

*/
ListExpr FFeed3TypeMap(ListExpr args)
{
  return FFeedTypeMap(args, true);
}

/*
5.3 Value Mapping

*/
int FFeed3ValueMap(Word* args, Word& result,
    int message, Word& local, Supplier s)
{
  return FFeedValueMap(args, result, message, local, s, 3);
}

Operator ffeed3Op(FFeed3Info(), FFeed3ValueMap, FFeed3TypeMap);

/*
6 Operator ~ffeed2~

17th Sept. 2013

This operator works similiar as the ~ffeed~.

Although the tuple and flob data are kept together in the data file,
it leaves the FLOB data untouched and create the flobId to link it.

6.1 Specification

*/
struct FFeed2Info : OperatorInfo {

  FFeed2Info() : OperatorInfo()
  {
    name =      "ffeed2";
    signature = "string x text x [int] x [int] x [int] x [int x int x int]"
        " -> stream(tuple(...))";
    syntax  = "fileName ffeed[ filePath, [fileSuffix], ; "
        "[remoteTypeNode]; "
        "[producerIndex x targetIndex x attemptTimes] ]";
    meaning =
        "Restore a tuple stream from a pair of type and data files, "
        "but leave the FLOB data untouched. ";
  }
};

/*
5.2 Type Mapping

*/
ListExpr FFeed2TypeMap(ListExpr args)
{
  return FFeedTypeMap(args, false);
}

/*
5.3 Value Mapping

*/
int FFeed2ValueMap(Word* args, Word& result,
    int message, Word& local, Supplier s)
{
  return FFeedValueMap(args, result, message, local, s, 2);
}

Operator ffeed2Op(FFeed2Info(), FFeed2ValueMap, FFeed2TypeMap);

/*
5 Operator ~fetchFlob~

7th May 2013

This operator fetches Flob data, locally and also remotely over the cluster.
The details of this operator are discussed in the attached readme file.
It is used after the ~ffeed3~ and ~pffeed3~ operator for fetching the Flob data.

If the flob are kept locally, it reads the Flob directly.
If the remote Flob is required, FlobOrder is prepared and sent to producer DS
that is indicated by the requring DS\_IDX attribute.

The producer will collect the needed Flobs into a binary file and
then send the file to the local computer.
Afterwards, the flob mode is set to 2 and the flob file is denoted to the
dilvered file.


The operator maps:

----
stream(tuple( ... Ai ... Aj ... DS\_IDX))
  x list (Ai)
\to stream(tuple( ... Ai ...))
----

The Ai and Aj are attributes with Flob data, Aj is removed since it is not asked.

14th Jan 2014
Update by Jiamin Lu

Now ~fetchFlob~ works with both ~ffeed2~ and ~ffeed3~ operators.
In the former case, ~ffeed2~ does not read the Flob data but only create a Flob structure with mode 2.
Then within this operator, the Flob data is read and the Flob mode is changed to 1,
in order to be cached by NativeFlobCache.
If this operator is not used, then the Flob data will be read each time from the disk file directly,
causing bad performance since it increases the disk IO overhead.

In the latter case, ~ffeed3~ also asks the user to use this operator explicitly,
or else system may crush since the getData function in FlobManager for mode 3 is not prepared at all.
The Flob data is stored remotely.

Besides, the DS\_IDX attribute is not required, since in binary operations or with alias operation,
it is difficult to locate this special named attribute.
Therefore, now this operator maps:

----
stream(tuple( ... Ai ... Aj ...))
  x list (Ai)
\to stream(tuple( ... Ai ...))
----

The Ai and Aj are attributes with Flob data, Aj is removed since it is not asked.


5.1 Specification

*/

struct FetchFlobInfo : OperatorInfo{
  FetchFlobInfo() : OperatorInfo()
  {
    name = "fetchFlob";
    signature =
        "stream(tuple( ... Ai ... Aj ...))"
        " x (Ai) -> stream(tuple( ... Ai ...)";
    syntax = "_ op[list]";
    meaning = "Retrieve the Flob data locally or remotely.";
  }
};

/*
5.2 Type Mapping

*/
ListExpr FetchFlobTypeMap(ListExpr args)
{
  try{
    NList l(args);
    NList pType, pValue;

    string lenErr = "ERROR! Operator fetchFlob expects two elements. ";
    string tpeErr = "ERROR! Operator fetchFlob expects "
        "stream(tuple((Ai) ... (DS_IDX int))), dbName and list(Ai)";
    string nfdErr = "ERROR! The input stream doesn't "
        "contain the required attributes ";


    if (l.length() != 2)
      return l.typeError(lenErr);

    NList first = l.first();
    NList instream = first.first();
    if (!listutils::isTupleStream(instream.listExpr()))
      return l.typeError(tpeErr);

    //Collect the names of requited attributes
    vector<string> raNames; //required attribute names
    int raNum;              //number of all required attributes
    NList raList = l.second().second();
    while (!raList.isEmpty()){
      NList attr = raList.first();
      if (! attr.isAtom())
        return l.typeError(tpeErr);

      raNames.push_back(attr.str());
      raList.rest();
    }
    raNum = raNames.size();

    //Check whether the required attribute exist and get its index
    NList raiList;  //required attribute index list
    NList daiList;  //deleted attribute index list
    NList attrList = instream.second().second();
    NList newAttrList;
    int index = 0;
    SecondoCatalog* sc = SecondoSystem::GetCatalog();
    while (!attrList.isEmpty())
    {
      NList elem = attrList.first();
      string aName = elem.first().str();
      NList aTypeList = elem.second();

      ListExpr nmType = sc->NumericType(aTypeList.listExpr());
      int algId, typeId;
      algId = nl->IntValue(nl->First(nmType));
      typeId = nl->IntValue(nl->Second(nmType));
      Attribute* attr = static_cast<Attribute*>
            ((am->CreateObj(algId, typeId))(nmType).addr);
      if (attr->NumOfFLOBs() > 0)
      {
        if (find(raNames.begin(), raNames.end(), aName) != raNames.end()){
          raiList.append(NList(index));
          raNum--;
          newAttrList.append(elem);
        }
        else{
          daiList.append(NList(index));
        }
      }
      else{
        newAttrList.append(elem);
      }
      attrList.rest();
      index++;
    }
    if (raNum > 0){
      return l.typeError(nfdErr);
    }

    NList resultType = NList(NList(Symbol::STREAM()),
        NList(NList(Tuple::BasicType()), newAttrList));

    NList appList;
    appList.append(NList(raiList.convertToString(), true, true));
    appList.append(NList(daiList.convertToString(), true, true));

    return NList(NList(Symbols::APPEND()),
                  appList,
                  resultType).listExpr();

  } catch(...){
    return listutils::typeError("invalid input");
  }
}

/*
5.3 Value Mapping

*/

int FetchFlobValueMap(Word* args, Word& result,
    int message, Word& local, Supplier s)
{
  FetchFlobLocalInfo* ffi;

  switch(message)
  {
    case OPEN: {
      NList resultType = qp->GetType(s);
      string raiStr = ((FText*)args[2].addr)->GetValue();
      string daiStr = ((FText*)args[3].addr)->GetValue();
      ListExpr raiList;  //required attribute index list
      nl->ReadFromString(raiStr, raiList);
      ListExpr daiList;  //deleted attribute index list
      nl->ReadFromString(daiStr, daiList);

      ffi = (FetchFlobLocalInfo*)local.addr;
      if (ffi)
      {
        delete ffi;
        ffi = 0;
      }
      ffi = new FetchFlobLocalInfo
          (s, resultType, NList(raiList), NList(daiList));

      qp->Open(args[0].addr);
      ffi->returned = 0;
      local.setAddr(ffi);

      return 0;
    }
    case REQUEST: {
      ffi = (FetchFlobLocalInfo*)local.addr;
      if (! ffi)
        return CANCEL;

      Tuple *t = ffi->getNextTuple(args[0].addr);
      if ( 0 == t )
        return CANCEL;
      else
      {
        ffi->returned++;
        result.setAddr(t);
        return YIELD;
      }
      return 0;
    }
    case CLOSE: {
      ffi = (FetchFlobLocalInfo*)local.addr;
      if (!ffi)
        return CANCEL;

      qp->Close(args[0].addr);
      return 0; //must return
    }
    case CLOSEPROGRESS: {
      ffi = (FetchFlobLocalInfo*)local.addr;
      if (ffi)
      {
        ffi->clearFetchedFiles();
        delete ffi;
        local.setAddr(0);
      }
      return 0;
    }
    case REQUESTPROGRESS: {

      return 0;
    }
  }

  return 0;
}


Operator fetchFlobOp(FetchFlobInfo(), FetchFlobValueMap, FetchFlobTypeMap);

pthread_mutex_t FetchFlobLocalInfo1::FFLI_mutex1;

FetchFlobLocalInfo1::FetchFlobLocalInfo1(
    const Supplier s, NList resultTypeList, NList _ral,NList _dal):
    faList(_ral), daList(_dal), 
    ci(0), cds(-1), moreInput(true)
{
  LFPath = getLocalFilePath("","","");
  FileSystem::AppendItem(LFPath, "flobFile_");

  resultType = new TupleType(
      SecondoSystem::GetCatalog()->NumericType(
          resultTypeList.second().listExpr()));

  standby = prepared = 0;
  fetchingNum = 0;
  fetchedFiles = 0;
  preparedNum = 0;

  faLen = faList.length();
  faVec = new int[faLen];
  NList rest = faList;
  size_t no = 0;
  while (!rest.isEmpty()){
    faVec[no] = rest.first().intval();
    rest.rest();
    no++;
  }

  maxSheetMem = qp->GetMemorySize(s) * 1024 * 1024;
//  cerr << "Max sheet Memory is: " << qp->GetMemorySize(s) << " MB" << endl;
  pthread_mutex_init(&FFLI_mutex1, NULL);
  pfs = 0;
}

Tuple* FetchFlobLocalInfo1::getNextTuple(const Supplier s)
{
  if (moreInput)
  {
    Word t;
    qp->Request(s, t);
    while (qp->Received(s))
    {
      Tuple* tuple = (Tuple*)t.addr;
      if (faList.isEmpty()){
        //No Flob request, hence fetching no Flob but only removing them
        return setResultTuple(tuple);
      }
      else{
        if (ci == 0){
          //Initialize all components that need to fetch remote Flob
          ci = new clusterInfo();
          cds = ci->getLocalNode();

          int maxSheetNum = getMaxSheetKey();
          standby = new vector<FlobSheet1*>(maxSheetNum);
          fetchingNum = 0;
          prepared = new vector<FlobSheet1*>();
          sheetCounter = new int[maxSheetNum];
          memset(sheetCounter, 0, maxSheetNum * sizeof(int));
          memset(tokenPass, false, PipeWidth);
          fetchedFiles = new vector<string>();
        }

        //check all involved Flob mode
        bool isLocal = true;
        vector<int> sDSs;
        NList rest = faList;
        while(!rest.isEmpty())
        {
          int ai = rest.first().intval();
          Attribute* attr = tuple->GetAttribute(ai);
          for (int k = 0; k < attr->NumOfFLOBs(); k++){
            size_t mode = attr->GetFLOB(k)->getMode();
            int ds = attr->GetFLOB(k)->getRecordId();
            if (mode > 2){ isLocal = false; }
            if (ds >= 0){
              sDSs.push_back(ds);
            }
          }
          rest.rest();
        }

        if (isLocal){
          // set Flob with mode 2 to 1 for caching into NativeFlobCache
          return setResultTuple(readLocalFlob(tuple));
        }
        else{
/*
For every possible Flob request combination, creates a set of flob-sheets
to fetch all needed flobs from the involved Data Servers remotely.
If all needed Flob data is stored locally, the Flob sheet is still created.
The sheetCounter counts the number of flob-sheets created for every Flob request combination.

Note that this method creates a lot of small files, which should be improved in the future. 

*/

          int index;
          while (true)
          {
            index = getSheetKey(sDSs);

            FlobSheet1* fs = standby->at(index);
            if (fs == 0){
              //Initialize flob sheet
              fs = new FlobSheet1(sDSs, index, faList, maxSheetMem);
              sheetCounter[index]++;
              standby->at(index) = fs;
            }

            // standby sheet is full after inserting the new order
            bool full = fs->addOrder(tuple);
            if (full){
              if (sendSheet(fs, sheetCounter[index])){
                standby->at(index) = 0;
              }
            }
            else {
              break;
            }
          }
        }
      }
      qp->Request(s, t);
    }
    moreInput = false;
  }

  if (ci)
  {
    //This loop waits until gets a file
    while (!standby->empty() || fetchingNum > 0 || preparedNum > 0)
    {
      // Copy the files as soon as possible
      while (!standby->empty() && fetchingNum < PipeWidth)
      {
        FlobSheet1* fs = standby->back();
        int index = standby->size() - 1;
        if (fs){
          bool rt = sendSheet(fs, sheetCounter[index]);
          if (!rt){
            continue;
          }
        }
        standby->pop_back();
      }

      while (preparedNum > 0)
      {
        if (pfs == 0)
        {
          pthread_mutex_lock(&FFLI_mutex1);
          for (vector<FlobSheet1*>::iterator cit = prepared->begin();
            cit != prepared->end(); cit++){
            if (! (*cit)->isFinished() ){
              pfs = *cit;
              break;
            }
          }
          pthread_mutex_unlock(&FFLI_mutex1);
        }

        pthread_mutex_lock(&FFLI_mutex1);
        Tuple* t = setResultTuple(pfs->getCachedTuple());
        pthread_mutex_unlock(&FFLI_mutex1);
        if (t){
          return t;
        }
        else {
          //no more tuples in this cache
          //cerr << *pfs << endl;
          preparedNum--;
          pfs = 0;
        }
      }
    }
  }

  return 0;
}

/*
The index for one Flob combination is decided by the weighted sum of all
involved sources.

*/
int FetchFlobLocalInfo1::getSheetKey(const vector<int>& sDSs)
{
  size_t f = faList.length();
  assert(f == sDSs.size());
  assert(ci);

  size_t cSize = ci->getClusterSize();
  int result = 0;
  for(vector<int>::const_iterator it = sDSs.begin(); it != sDSs.end(); it++){
    result += (*it) * pow((double)cSize, (int)(f - 1));
    f--;
  }
  return result;
}

/*
Get the possible maximum sheet index

*/
int FetchFlobLocalInfo1::getMaxSheetKey()
{
  vector<int> sds;
  NList rest = faList;
  while (!rest.isEmpty()){
    sds.push_back(ci->getClusterSize());
    rest.rest();
  }
  return getSheetKey(sds);
}

/*
Remove the useless (unrequired) Flob attribute

*/
Tuple* FetchFlobLocalInfo1::setResultTuple(Tuple* tuple)
{
  Tuple* newTuple = 0;
  if (tuple)
  {
    newTuple = new Tuple(resultType);
    int di = 0;
    //si: source attribute index; di: destination attribute index
    for (int si = 0; si < tuple->GetNoAttributes(); si++)
    {
      NList rest = daList;
      bool remove = false;
      while (!rest.isEmpty()){
        int dai = rest.first().intval();
        if (si == dai){ //remove this attribute
          remove = true;
          break;
        }
        rest.rest();
      }

      if (!remove){
        newTuple->CopyAttribute(si, tuple, di++);
      }
    }
    tuple->DeleteIfAllowed();
  }
  return newTuple;
}

Tuple* FetchFlobLocalInfo1::readLocalFlob(Tuple* tuple)
{
  //Read the Flob from the local disk file and cache it to NativeFlobCache.
  if (tuple){
    tuple->readLocalFlobFile(LFPath);
  }
  return tuple;
}


bool FetchFlobLocalInfo1::sendSheet(FlobSheet1* fs, int times)
{
  if (fetchingNum >= PipeWidth)
  {  
    return false;
  }

  for (int t = 0; t < PipeWidth; t++)
  {
    if (!tokenPass[t] || pthread_kill(threadID[t],0))
    {
      tokenPass[t] = true;
      FFLI_Thread1* ft = new FFLI_Thread1(this, fs, times, t);
      pthread_create(&threadID[t], NULL, sendSheetThread, ft);
      fetchingNum++;
      return true;
    }
  }
  return false;
}

void* FetchFlobLocalInfo1::sendSheetThread(void* ptr)
{
  FFLI_Thread1* ft = (FFLI_Thread1*)ptr;
  FetchFlobLocalInfo1* ffli = ft->ffli;
  FlobSheet1* fs = ft->sheet;
  int times = ft->times;
  int token = ft->token;


  int dest = ffli->getLocalDS();
  vector<int> sources = fs->getSDSs();

  //For one Flob attribute, its data is collected into a separated file
  for (size_t faCounter = 0; faCounter < ffli->faLen; faCounter++)
  {
    int ai = ffli->faVec[faCounter];
    int source = sources[faCounter];

    //Prepare the sheet for this sheet
    pthread_mutex_lock(&FFLI_mutex1);
    string localSheetPath = fs->setSheet(source, dest, times, ai);
    pthread_mutex_unlock(&FFLI_mutex1);
    string sourcePSFS = ffli->getPSFSPath(source);
    int atimes = MAX_COPYTIMES;
    while ( atimes-- > 0){
      if (0 == system(
          (scpCommand + localSheetPath + " " + sourcePSFS).c_str())){
        break;
      }
      else{
        WinUnix::sleep(1);
      }
    }
    if (atimes == 0){
      cerr << "Warning!! Send sheet file " 
           << localSheetPath << " fails" << endl;
    }

/*
Invoke the remote collectFlob program, to prepare the required Flob data.
The program needs the following parameters:

----
flobSheetName : string
PSFSNodePath :  string
ResultFileName :  string
TargetPath :  string
----

*/
    string sourceIP = ffli->getIP(source);
    //Mini Secondo path
    string sourceMSec = ffli->getMSecPath(source, false);
    FileSystem::AppendItem(sourceMSec, "bin/collectFlob");
    string sPSFS = ffli->getPSFSPath(source, false);
    string sSheet = sPSFS;
    string sheetName = localSheetPath.substr(localSheetPath.find_last_of("/"));
    FileSystem::AppendItem(sSheet, sheetName);
    string resultFlobFileName = fs->setResultFile(source, dest, times, ai);
    string localPSFS = ffli->getPSFSPath(dest, true);
    string command = "ssh " + sourceIP + " " + sourceMSec + " "
        + sSheet + " " + sPSFS + " " + resultFlobFileName + " " + localPSFS;

    atimes = MAX_COPYTIMES;
    int rc;
    while (atimes-- > 0){
      rc = system(command.c_str());
      if (rc == 0){
        // Delete the sheet file after the result Flob file is prepared,
        // in case the sheet is sent to the local computer.
        if (FileSystem::FileOrFolderExists(localSheetPath)){
          FileSystem::DeleteFileOrFolder(localSheetPath);
        }
        break;
      } else {
        WinUnix::sleep(1);
      }
    }
    if (atimes == 0){
      cerr << "Warning!! Processing command: " << command << " fails" << endl;
    }
  }

  // after getting all result flob files
  pthread_mutex_lock(&FFLI_mutex1);
  ffli->fetching2prepared(fs);
  ffli->tokenPass[token] = false;
  pthread_mutex_unlock(&FFLI_mutex1);

  return NULL;
}

void FetchFlobLocalInfo1::fetching2prepared(FlobSheet1* fs){
  for (size_t faCounter = 0; faCounter < faLen; faCounter++)
  {
    int ai = faVec[faCounter];
    fetchedFiles->push_back(fs->getResultFilePath(ai));
  }

  prepared->push_back(fs);
  preparedNum++;
  fetchingNum--;
}

void FetchFlobLocalInfo1::clearFetchedFiles(){
  if (fetchedFiles)
  {
    for (vector<string>::iterator it = fetchedFiles->begin();
        it != fetchedFiles->end(); it++){
      string filePath = *it;
      if (!FileSystem::DeleteFileOrFolder(filePath)){
        cerr << "Warning!! File " << filePath << " cannot be deleted. " << endl;
      }
    }
  }
}


string FlobSheet1::setSheet(int source, int dest, int times, int attrId)
{
  stringstream ss;
  ss << "Sheet_"
      << WinUnix::getpid() << "_"
      << sheetIndex << "_"
      << attrId << "_"
      << source << "_"
      << dest << "_"
      << times;
  string sheetName = ss.str();
  sheetName = getLocalFilePath("", sheetName, "");
  ofstream sfout(sheetName.c_str());

  it = buffer->MakeScan();
  Tuple* t = it->GetNextTuple();
  while (t)
  {
    Attribute* attr = t->GetAttribute(attrId);
    for (int k = 0; k < attr->NumOfFLOBs(); k++)
    {
      //output: fileId recordId offset mode size
      Flob* flob = attr->GetFLOB(k);
      if (flob->getSize() > Tuple::extensionLimit){
/*
Note here the Flob may already have been fetched by another thread, 
hence its mode becomes 1. 
However we still write it into the flob order, although the collectFlob 
will fetch no data but only prepare an empty block with its size. 

*/
        sfout << flob->describe();
        toCounter++;
      }
    }
    t = it->GetNextTuple();
  }
  delete it;
  it = 0;
  sfout.close();

  return sheetName;
}

string FlobSheet1::setResultFile(int source, int dest, int times, int attrId)
{
  stringstream ss;
  ss << "ResultFlob_"
      << WinUnix::rand(WinUnix::getpid()) << "_"
      << sheetIndex << "_"
      << attrId << "_"
      << source << "_"
      << dest << "_"
      << times;
  string resultName = ss.str();
  string resultFilePath = getLocalFilePath("", resultName, "");
  flobFiles->find(attrId)->second = make_pair(resultFilePath, 0);
  return resultName;
}

bool FlobSheet1::addOrder(Tuple* tuple)
{
  buffer->AppendTuple(tuple);
  cachedSize += tuple->GetSize();

  return cachedSize >= maxMem;
}

Tuple* FlobSheet1::getCachedTuple()
{
  if (it == 0){
    it = buffer->MakeScan();
  }
  Tuple* tuple = it->GetNextTuple();

  if (tuple){
    rtCounter++;
    for (size_t faCounter = 0; faCounter < faLen; faCounter++)
    {
      int ai = faVec[faCounter];
      for (int k = 0; k < tuple->GetAttribute(ai)->NumOfFLOBs(); k++)
      {
        Flob* flob = tuple->GetAttribute(ai)->GetFLOB(k);
        if (flob->getMode() == 2 || flob->getMode() == 3)
        {
          string flobFile = flobFiles->find(ai)->second.first;
          size_t flobOffset = flobFiles->find(ai)->second.second;
          SmiSize flobSize = flob->getSize();

          Flob::readExFile(*flob, flobFile, flobSize, flobOffset);
          //Record all new created Flob id within this sheet
          newRecIds.insert(flob->getRecordId());
          flobFiles->find(ai)->second.second += flobSize;
        }
        else if (flob->getSize() >= Tuple::extensionLimit){
          SmiRecordId newRecId = flob->getRecordId();
          if (newRecIds.find(newRecId) == newRecIds.end()){
            //Flob listed here but created in another sheet
            flobFiles->find(ai)->second.second += flob->getSize();
            newRecIds.insert(newRecId);
          } else {
          }
        }
      }
    }
  } else {
    delete it; 
    it = 0;
    finished = true;
  }
  return tuple;
}

ostream& operator<<(ostream& os, const FlobSheet1& f){
  return f.print(os);
}

/*
5.1 FetchFlobLocalInfo (2rd version)

*/

pthread_mutex_t FetchFlobLocalInfo::FFLI_mutex;

FetchFlobLocalInfo::FetchFlobLocalInfo(
    const Supplier s, NList resultTypeList, NList _fal, NList _dal)
{
  LFPath = getLocalFilePath("", "", "");
  FileSystem::AppendItem(LFPath, "flobFile_");

  resultType = new TupleType(
      SecondoSystem::GetCatalog()->NumericType(
          resultTypeList.second().listExpr()));

  faLen = _fal.length();
  faVec = new int[faLen];
  size_t no = 0;
  NList rest = _fal;
  while(!rest.isEmpty()){
    faVec[no++] = rest.first().intval();
    rest.rest();
  }

  maxFlobNum = 0;
  for(no = 0; no < faLen; no++){
    int ai = faVec[no];
    int numOfFlobs = resultType->GetAttributeType(ai).numOfFlobs;
    if (numOfFlobs > maxFlobNum)
      maxFlobNum = numOfFlobs;
  }

  daLen = _dal.length();
  daVec = new int[daLen];
  rest = _dal;
  no = 0;
  while(!rest.isEmpty()){
    daVec[no++] = rest.first().intval();
    rest.rest();
  }

  maxBufferSize = qp->GetMemorySize(s) * 1024 * 1024;
  maxSheetSize = 0;
  totalTupleBuffer = 0;
  flobInfo = 0;
  newRecIds = 0;

  ci = 0;
  cds = -1;
  moreInput = true;

  standby = 0;
  prepared = 0;
  fetchingNum = preparedNum = 0;
  fetchedFiles = 0;
  pthread_mutex_init(&FFLI_mutex, NULL);
}

Tuple* FetchFlobLocalInfo::getNextTuple(const Supplier s)
{
  //Read from the input first until there is no more input
  if (moreInput)
  {
    Word t;
    qp->Request(s, t);
    while(qp->Received(s))
    {
      Tuple* tuple = (Tuple*)t.addr;
      if (0 == faLen){
        //No Flob request at all
        return setResultTuple(tuple);
      }
      else
      {
        if (ci == 0)
        {
          //Initialize all components needed for fetching the remote Flob
          ci  = new clusterInfo();
          cds = ci->getLocalNode();
          size_t slaveSize = ci->getSlaveSize();
          maxSheetSize = maxBufferSize / slaveSize;
          totalTupleBuffer = new TupleBuffer(maxBufferSize);
          flobInfo = new vector<TupleFlobInfo>();
          gtbit = 0;

          //Increase the slaveSize with 1 to visit them by the source id
          standby = new vector<FlobSheet*>(slaveSize + 1);
          prepared = new map<pair<int, int>, FlobSheet*>();
          fetchingNum = preparedNum = 0;
          sheetCounter = new int[slaveSize + 1];
          memset(sheetCounter, 0, (slaveSize + 1) * sizeof(int));
          fetchedFiles = new vector<string>();

          memset(tokenPass, false, PipeWidth);
        }

        bool isLocal = true;
        for (size_t no = 0; no < faLen; no++)
        {
          int ai = faVec[no];
          Attribute* attr = tuple->GetAttribute(ai);
          if ( attr->NumOfFLOBs() > 0){
            char mode = attr->GetFLOB(0)->getMode();
            if (mode > 2)
              isLocal = false;
          }
        }

        if (isLocal){
          // Mode 2
          return setResultTuple(readLocalFlob(tuple));
        }
        else
        {
/*
For each Flob with mode 3, its elements mean:

----
* FileId: the integer suffix for its Flob file

* RecordId: the source DS of the cluster

* offset: its offset within the Flob file

* size: the size of the flob
----

*/
          totalTupleBuffer->AppendTuple(tuple);
          TupleFlobInfo tif(faLen, maxFlobNum);
          for (size_t no = 0; no < faLen; no++)
          {
            int ai = faVec[no];
            Attribute* attr = tuple->GetAttribute(ai);
            if (attr->NumOfFLOBs() == 0){
              tif.setFlobInfo(no, 0, 0, 0, 0);
            } else {

              for (int k = 0; k < attr->NumOfFLOBs(); k++)
              {
                Flob* flob = attr->GetFLOB(k);
                if (flob->getMode() < 3){
                  tif.setFlobInfo(no, k, 0, 0, 0);
                  continue;
                }
                int source = flob->getRecordId();

                int times;
                while (true)
                {
                  FlobSheet* fs = standby->at(source);
                  if (fs == 0){
                    sheetCounter[source]++;
                    times = sheetCounter[source];
                    fs = new FlobSheet(source, cds, times, maxSheetSize);
                    standby->at(source) = fs;
                  }
                  else {
                    times = fs->getTimes();
                  }

                  size_t fileOffset;
                  assert(flob->getMode() == 3);
                  bool full = fs->addOrder(*flob, fileOffset);
                  bool ok = false;
                  if (full){
                    if (sendSheet(fs)){
                      standby->at(source) = 0;
                    }
                  } else {
                    ok = true;
                  }

                  if (ok){
                    //The Flob is inserted into the sheet
                    tif.setFlobInfo(no, k, source, times, fileOffset);
                    break;
                  }
                }
              }
            } // set up for one Flob
          } // set up for one attribute
          flobInfo->push_back(tif);
        } // set up for tuple asks Flob
      } // set up for one tuple
      qp->Request(s, t);
    }
    moreInput = false;
  }

  if (ci)
  {
    //This loop waits until getting a file
    while (!standby->empty() || fetchingNum > 0 /*|| preparedNum > 0*/)
    {
      //Send all left sheets
      while (!standby->empty() && fetchingNum < PipeWidth)
      {
        FlobSheet* fs = standby->back();
        if (fs)
        {
          fs->closeSheetFile();
          if (!sendSheet(fs)){
            continue;
          }
        }
        standby->pop_back();
      }
    }

    while (preparedNum > 0){
      if (gtbit == 0){
        gtbit = totalTupleBuffer->MakeScan();
        gtfit = flobInfo->begin();
      }
      
      while (!gtbit->EndOfScan())
      {
        Tuple* tuple = gtbit->GetNextTuple();
        if (tuple == 0)
          break;

        if (!gtfit->isReturned())
        {
          //Whether the Flobs have been fetched
          bool flobAllRead = true;
          for (size_t no = 0; no < faLen; no++){
            int ai = faVec[no];
            Attribute* attr = tuple->GetAttribute(ai);
            for (int k = 0; k < attr->NumOfFLOBs(); k++){
              Flob* flob = attr->GetFLOB(k);
              if (flob->getMode() > 2){
                flobAllRead = false;
                break;
              }
            }
            if (!flobAllRead) break;
          } // detect whether there exists un-read flob
          if (flobAllRead){
            Counter::getRef("FetchFlob")++;
            gtfit->setReturned();
            gtfit++;
            return setResultTuple(tuple);
          }

          //Find its related Flob files, whether they are prepared ?
          bool dataPrepared = true;
          FlobSheet *sheets[faLen][maxFlobNum];
          int source, times;

          for (size_t no = 0; no < faLen; no++){
            int ai = faVec[no];
            Attribute* attr = tuple->GetAttribute(ai);
            for (int k = 0; k < attr->NumOfFLOBs(); k++){
              source = gtfit->getDS(no, k);
              times = gtfit->getSheetTimes(no, k);
              if (source == 0){
                //Flob are kept locally
                continue;
              }
              if (prepared->find(make_pair(source, times)) == prepared->end()){
                dataPrepared = false;
              } else {
                sheets[no][k] =
                    prepared->find(make_pair(source, times))->second;
              }
            }
          }

          if (dataPrepared){
            //Prepare all needed Flob structure.
            for (size_t no = 0; no < faLen; no++)
            {
              int ai = faVec[no];
              Attribute* attr = tuple->GetAttribute(ai);

              for (int k = 0; k < attr->NumOfFLOBs(); k++)
              {
                source = gtfit->getDS(no, k);
                if (source == 0){
                  //Flob is kept within the Tuple
                  continue;
                }

                FlobSheet* sheet = sheets[no][k];
                string flobFile = sheet->getResultFile();

                Flob* flob = attr->GetFLOB(k);
                if (flob->getMode() > 2)
                {
                  Flob::readExFile(*flob, flobFile,
                      flob->getSize(), gtfit->getOffset(no, k));
                }
              }
            }
            Counter::getRef("FetchFlob")++;
            gtfit->setReturned();
            gtfit++;
            return setResultTuple(tuple);
          }
        }
      }

      if (Counter::getRef("FetchFlob") >= totalTupleBuffer->GetNoTuples()){
        //All tuples are fetched
        return 0;
      } else {
        gtbit = 0;
      }
    }
  }

  return 0;
}

void FetchFlobLocalInfo::fetching2prepared(FlobSheet* fs)
{
  string flobFilePath = fs->getResultFile();
  fetchedFiles->push_back(flobFilePath);
  int source = fs->getSource();
  int times = fs->getTimes();

  prepared->insert(make_pair(make_pair(source, times), fs));
  preparedNum++;
  fetchingNum--;
}

void FetchFlobLocalInfo::clearFetchedFiles()
{
  if (fetchedFiles)
  {
    for (vector<string>::iterator it = fetchedFiles->begin();
        it != fetchedFiles->end(); it++){
      string filePath = *it;

      if (!FileSystem::DeleteFileOrFolder(filePath)){
        cerr << "Warning!! File " << filePath << " cannot be deleted. " << endl;
      }
    }
  }
}

bool FetchFlobLocalInfo::sendSheet(FlobSheet* fs)
{
  if (fetchingNum >= PipeWidth){
    return false;
  }

  for (size_t t = 0; t < PipeWidth; t++)
  {
    if (!tokenPass[t] || pthread_kill(threadID[t], 0))
    {
      tokenPass[t] = true;
      FFLI_Thread* ft = new FFLI_Thread(this, fs, t);
      pthread_create(&threadID[t], NULL, sendSheetThread, ft);
      fetchingNum++;
      return true;
    }
  }

  return false;
}


void* FetchFlobLocalInfo::sendSheetThread(void* ptr)
{
  FFLI_Thread* ft = (FFLI_Thread*)ptr;
  FetchFlobLocalInfo* ffli = ft->ffli;
  FlobSheet* fs = ft->sheet;
  int token = ft->token;
  int dest = fs->getDest();

  //start a thread to fetch the flob file.
  string localSheetPath = fs->getSheetPath();
  int source = fs->getSource();
  string sourcePSFS = ffli->getPSFSPath(source);

  //Send the sheet
  int atimes = MAX_COPYTIMES;
  while ( atimes-- > 0){
    if (0 == system(
        (scpCommand + localSheetPath + " " + sourcePSFS).c_str())){
      break;
    }
    else{
      WinUnix::sleep(1);
    }
  }
  if (atimes == 0){
    cerr << "Warning!! Send sheet file "
         << localSheetPath << " fails" << endl;
  }

/*
Invoke the remote collectFlob program, to prepare the required Flob data.
The program needs the following parameters:

----
flobSheetName : string
PSFSNodePath :  string
ResultFileName :  string
TargetPath :  string
----

*/
  string sourceIP = ffli->getIP(source);
  string sourceMSec = ffli->getMSecPath(source, false);
  FileSystem::AppendItem(sourceMSec, "bin/collectFlob");
  string sPSFS = ffli->getPSFSPath(source, false);
  string sSheet = sPSFS;
  string sheetName = localSheetPath.substr(
      localSheetPath.find_last_of("/") + 1);
  FileSystem::AppendItem(sSheet, sheetName);
  string resultFlobFilePath = fs->getResultFile();
  string resultFlobFileName =
      resultFlobFilePath.substr(resultFlobFilePath.find_last_of("/") + 1);
  string localPSFS = ffli->getPSFSPath(dest, true);
  string command = "ssh " + sourceIP + " " + sourceMSec + " "
      + sSheet + " " + sPSFS + " " + resultFlobFileName + " " + localPSFS;

  atimes = MAX_COPYTIMES;
  int rc;
  while (atimes-- > 0){
    rc = system(command.c_str());
    if (rc == 0){
      // Delete the sheet file after the result Flob file is prepared,
      // in case the sheet is sent to the local computer.
      if (FileSystem::FileOrFolderExists(localSheetPath)){
        FileSystem::DeleteFileOrFolder(localSheetPath);
      }
      break;
    } else {
      WinUnix::sleep(1);
    }
  }
  if (atimes == 0){
    cerr << "Warning!! Processing command: " << command << " fails" << endl;
  }

  // after getting all result flob files
  pthread_mutex_lock(&FFLI_mutex);
  ffli->fetching2prepared(fs);
  ffli->tokenPass[token] = false;
  pthread_mutex_unlock(&FFLI_mutex);

  return NULL;
}

/*
Remove the useless (unrequired) Flob attribute

*/
Tuple* FetchFlobLocalInfo::setResultTuple(Tuple* tuple)
{
  Tuple* newTuple = 0;
  if (tuple)
  {
    newTuple = new Tuple(resultType);
    int di = 0;
    //si: source attribute index; di: destination attribute index
    for (int si = 0; si < tuple->GetNoAttributes(); si++)
    {
      bool remove = false;
      for (size_t no = 0; no < daLen; no++)
      {
        int dai = daVec[no];
        if (si == dai){ //remove this attribute
          remove = true;
          break;
        }
      }

      if (!remove){
        newTuple->CopyAttribute(si, tuple, di++);
      }
    }
    tuple->DeleteIfAllowed();
  }
  return newTuple;
}

/*
Read the Flob from the local disk file and cache it to NativeFlobCache.

*/
Tuple* FetchFlobLocalInfo::readLocalFlob(Tuple* tuple)
{
  if (tuple){
    tuple->readLocalFlobFile(LFPath);
  }
  return tuple;
}

FlobSheet::FlobSheet(int source, int dest, int times, int maxMemory):
  sourceDSId(source), destDSId(dest), times(times),
  cachedSize(0), maxMem(maxMemory)
{
  stringstream ss;
  ss << "Sheet_"
      << WinUnix::getpid() << "_"
      << source << "_"
      << dest   << "_"
      << times;
  sheetFilePath = ss.str();
  sheetFilePath = getLocalFilePath("", sheetFilePath, "");
  sheetFile = new ofstream(sheetFilePath.c_str());

/*
The random function makes the result Flob files being different,
within the same process lifetime.
This is because the file names are used in caching the file pointers.

*/

  ss.str("");
  ss.clear();
  ss << "ResultFlob_"
      << WinUnix::rand(WinUnix::getpid()) << "_"
      << source << "_"
      << dest << "_"
      << times;
  resultFlobFilePath = ss.str();
  resultFlobFilePath = getLocalFilePath("", resultFlobFilePath, "");
}

/*
Adds one more order to the current sheet,
then returns whether the sheet is full.

*/
bool FlobSheet::addOrder (const Flob& flob, size_t& offset)
{
  assert(sheetFile);

  if (cachedSize + flob.getSize() > maxMem){
    closeSheetFile();
    return true;
  }

  pair<SmiFileId, SmiSize> mlob(flob.getFileId(), flob.getOffset());
  bool exists = (lobMarkers.find(mlob) != lobMarkers.end());

  if (!exists){
    offset = cachedSize;
    lobMarkers.insert(make_pair(mlob, offset));

    *sheetFile << flob.describe();
    cachedSize += flob.getSize();
  } else {
    offset = lobMarkers.find(mlob)->second;
  }

  return false;
}

void FlobSheet::closeSheetFile()
{
  if (sheetFile){
    sheetFile->close();
    delete sheetFile;
    sheetFile = 0;
  }
}

ostream& operator<<(ostream& os, const TupleFlobInfo& f){
  return f.print(os);
}


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
        arrayalgebra::extractIds(type, algID, typID);
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
Copy a file through the network
~cfn~ means: change (destination) file name

*/
int copyFile(string source, string dest, bool cfn/* = false*/)
{
  bool sRmt = source.find(":") != string::npos ? true : false;
  bool dRmt = dest.find(":") != string::npos ? true : false;

  if (!(sRmt^dRmt)){
    //both sides are remote machines
    //or both sides are local machines.
    return -1;
  }
  
  if (dRmt)
  {  
    string destNode = dest.substr(0, dest.find_first_of(":"));
    dest = dest.substr(dest.find_first_of(":") + 1);
    string sourceFileName = source.substr(source.find_last_of("/") + 1);
    string destFileName= "";
    if (cfn)
    {
      destFileName = dest.substr(dest.find_last_of("/") + 1);
      dest = dest.substr(0, dest.find_last_of("/") + 1);
    }

    int sourceDepth = 0;
    size_t pos = 0;
    while ((pos = source.find("/", pos)) != string::npos){
      sourceDepth++ ;
      pos++;
    }
    sourceDepth = sourceDepth > 0 ? (sourceDepth - 1) : 0;

    stringstream command;
    command << "tar -czf - " << source
      << " | ssh -oCompression=no " << destNode
      << " \"tar -zxf - -C " << dest << " --strip=" << sourceDepth;
    if (cfn){
      command << "; mv " << dest << sourceFileName 
      << " " << dest << destFileName;
    }
    command << "\"";
    return system(command.str().c_str());
  }
  else
  {
   string srcNode = source.substr(0, source.find_first_of(":"));
   source = source.substr(source.find_first_of(":") + 1);
   string srcName = source.substr(source.find_last_of("/") + 1);
   string destName = "";
   if (cfn) {
     destName = dest.substr(dest.find_last_of("/") + 1);
   }
   string destPath = dest.substr(0, dest.find_last_of("/") + 1);

   int sourceDepth = 0;
   size_t pos = 0;
    while ((pos = source.find("/", pos)) != string::npos){
      sourceDepth++ ;
      pos++;
    }
    sourceDepth = sourceDepth > 0 ? (sourceDepth - 1) : 0;

    stringstream command;
    command << "ssh -oCompression=no " << srcNode 
      << " \"tar -czf - " << source << " \" | tar -xzf - -C "
      << destPath << " --strip=" << sourceDepth;
    if (cfn){
      command << "; mv " << destPath << srcName
          << " " << destPath << destName;
    }

    return system(command.str().c_str());
  }
}

/*
Read one tuple from the given data file.

*/
Tuple* readTupleFromFile(ifstream* file, TupleType* type, int mode,
    string flobFile/* = ""*/)
{
  assert((mode == 1) || (mode == 2) || (mode == 3));

  Tuple* t = 0;
  u_int32_t blockSize;
  assert(file->good());

  if (mode == 1)
  {
    file->read(reinterpret_cast<char*>(&blockSize), sizeof(blockSize));
    if (!file->eof() && (blockSize > 0))
    {
      blockSize -= sizeof(blockSize);
      char *tupleBlock = new char[blockSize];
      file->read(tupleBlock, blockSize);

      t = new Tuple(type);
      t->ReadFromBin(tupleBlock, blockSize);
      delete[] tupleBlock;
    }
  }
  else if (mode == 2)
  {
    size_t sizeLen = (sizeof(u_int32_t) + sizeof(u_int16_t));
    char sizes[sizeLen];
    size_t offset = 0;
    file->read(sizes, sizeLen);
    ReadVar<u_int32_t>(blockSize, sizes, offset);

    if (!file->eof() && (blockSize > 0))
    {
      blockSize -= sizeof(blockSize);
      //READ only the tuple data out

      u_int16_t tupleSize;
      ReadVar<u_int16_t>(tupleSize, sizes, offset);

      //read less data
      char *tupleOnlyBlock = new char[tupleSize];
      file->read(tupleOnlyBlock, tupleSize);
      size_t flobOffset = file->tellg();

      t = new Tuple(type);
      t->ReadTupleFromBin(tupleOnlyBlock, tupleSize, flobFile, flobOffset);

      u_int32_t flobLength = blockSize - sizeof(tupleSize) - tupleSize;
      if (flobLength != 0){
        file->seekg(flobLength, ios::cur);
      }
      delete[] tupleOnlyBlock;
    }
  }
  else if (mode == 3)
  {
    size_t sizeLen = (sizeof(u_int32_t) + sizeof(u_int16_t));
    char sizes[sizeLen];
    size_t offset = 0;
    file->read(sizes, sizeLen);
    ReadVar<u_int32_t>(blockSize, sizes, offset);

    if (!file->eof() && (blockSize > 0))
    {
      //READ only the tuple data out
      blockSize -= sizeof(blockSize);
      u_int16_t tupleSize;
      ReadVar<u_int16_t>(tupleSize, sizes, offset);

      char *tupleOnlyBlock = new char[tupleSize];
      file->read(tupleOnlyBlock, tupleSize);
      size_t flobOffset = file->tellg();

      u_int32_t flobLength = blockSize - sizeof(tupleSize) - tupleSize;
      t = new Tuple(type);

      if (flobLength == 0){
        //read as ffeed3
        t->ReadTupleFromBin(tupleOnlyBlock);
      }
      else {
        //read as ffeed2
        t->ReadTupleFromBin(tupleOnlyBlock, tupleSize, flobFile, flobOffset);
      }

      if (flobLength != 0){
        file->seekg(flobLength, ios::cur);
      }
      delete[] tupleOnlyBlock;
    }
  }
  return t;
}

int getRoundRobinIndex(int row, int clusterSize)
{
  return ((row - 1)%clusterSize + 1);
}

/*
remove a term from a given nested-list.
e.g. if the given list is (a (b c)), and the term is b
returns (a c)

*/
ListExpr rmTermNL(ListExpr list, string term, int& count)
{
  if (nl->IsEmpty(list)){
    return list;
  }

  if (nl->IsAtom(list))
  {
    if (nl->IsEqual(list,term))
    {
      return nl->TheEmptyList();
    }
    else{
      return list;
    }
  }
  else
  {
    ListExpr first = nl->First(list);
    if (nl->IsAtom(first))
    {
      if (nl->IsEqual(first, term))
      {
        count++;
        ListExpr rest = rmTermNL(nl->Rest(list), term, count);
        if (nl->ListLength(rest) == 1)
          return nl->First(rest);
      }
    }
  }

  return (nl->Cons(rmTermNL(nl->First(list), term, count),
    rmTermNL(nl->Rest(list), term, count)));
}

/*
Add the incomplete property on attributes containing Flob

*/
NList addIncomplete(const NList& attrList)
{
  SecondoCatalog* sc = SecondoSystem::GetCatalog();
  NList newAttrList;
  NList rest = attrList;
  while (!rest.isEmpty())
  {
    NList attr = rest.first();
    NList name = attr.first();
    NList type = attr.second();

    ListExpr nmType = sc->NumericType(type.listExpr());
    int algId, typeId;
    algId = nl->IntValue(nl->First(nmType));
    typeId = nl->IntValue(nl->Second(nmType));
    Attribute* elem = static_cast<Attribute*>
          ((am->CreateObj(algId, typeId))(nmType).addr);
    if (elem->NumOfFLOBs() > 0)
    {
      newAttrList.append(
          NList(name, NList(NList("incomplete"),type)));
    }
    else
    {
      newAttrList.append(attr);
    }
    rest.rest();
  }

  return newAttrList;
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
//    AddTypeConstructor(&IncompleteTC);
//    IncompleteTC.AssociateKind(Kind::DATA());

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

    AddOperator(&ffeed2Op);
    ffeed2Op.SetUsesArgsInTypeMapping();

    AddOperator(&fconsume3Op);
    fconsume3Op.SetUsesArgsInTypeMapping();
    AddOperator(&ffeed3Op);
    ffeed3Op.SetUsesArgsInTypeMapping();
    AddOperator(&fetchFlobOp);
    fetchFlobOp.SetUsesArgsInTypeMapping();
    fetchFlobOp.SetUsesMemory();

    AddOperator(&fdistribute3Op);
    fdistribute3Op.SetUsesArgsInTypeMapping();


#ifdef USE_PROGRESS
    fconsumeOp.EnableProgress();
    fconsume3Op.EnableProgress();
    fdistribute3Op.EnableProgress();

    ffeedOp.EnableProgress();
    ffeed2Op.EnableProgress();
    ffeed3Op.EnableProgress();
    fetchFlobOp.EnableProgress();

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

