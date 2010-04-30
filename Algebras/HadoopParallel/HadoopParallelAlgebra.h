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

[1] Definition of Auxiliary Classes in HadoopParallelAlgebra

April 2010 Jiamin Lu

[newpage]

1 Auxiliary Classes in HadoopParallelAlgebra

This file claims all relevant classes and methods that is
used in HadoopParallelAlgebra. Includes follow classes:

  * ~deLocalInfo~. Assists ~doubleexport~ operator.

  * ~phjLocalInfo~. Assists ~parahashjoin~ operator.

And includes one method:

  * ~renameList~. Assists ~parahashjoin~ operator

*/
#ifndef HADOOPPARALLELALGEBRA_H_
#define HADOOPPARALLELALGEBRA_H_

#include "FTextAlgebra.h"

/*
1.1 deLocalInfo Class

Assists ~doubleexport~ operator.
First traverse tuples from streamA to the end, then traverse streamB.

*/
class deLocalInfo
{
private:

  Word streamA;
  Word streamB;
  bool isAEnd;
  //Check out whether gets to the end of streamA.

  int attrIndexA;
  int attrIndexB;

  ListExpr tupTypeA;
  ListExpr tupTypeB;

  TupleType *resultTupleType;

  Word makeTuple(Word stream, int index, ListExpr typeInfo, int sig);

public:
  deLocalInfo(Word _streamA, Word wAttrIndexA,
              Word _streamB, Word wAttrIndexB,
              Supplier s);

  Tuple* nextResultTuple();
};

/*
1.2 renameList Method

Assists ~parahashjoin~ operator.
Append an ~appendName~ after each attribute name of ~oldTupleList~,
to avoid the situation that joined relations have a same attribute name.

*/
ListExpr renameList(ListExpr oldTupleList, string appendName);

/*
1.3 phjLocalInfo Class

Assists ~parahashjoin~ operator.
~getNewProducts~ collects tuples in each buckets,
and make products if the tuples come from different sources.
Put the product results in the TupleBuffer ~joinedTuples~.


*/
class phjLocalInfo
{
private:

  Word mixStream;
  ListExpr resultTupleInfo;

  TupleBuffer *joinedTuples;
  GenericRelationIterator *tupleIterator;

  bool getNewProducts();

public:
  phjLocalInfo(Word _stream, Supplier s);

  Word nextJoinTuple();
};

/*
Below we define a class which helps to maintain stream in value
mappings.

Copied from PartitionedStreamAlgebra

*/

template<class T>
struct StreamBase
{
  typedef enum {opened, finished, closed} StreamState;

  private:
  const T& child() const {
    return static_cast<const T&>( *this );
  }

  public:
  mutable StreamState state;


  inline void open() {
    return child().open();
  }

  inline void close() {
    return child().close();
  }

  inline void* getNext() const
  {
    return child().getNext();
  }

  StreamBase() : state(closed) {}
  StreamBase(const StreamBase& rhs) : state(rhs.state) {}

};

struct StreamOpAddr : public StreamBase<StreamOpAddr>
{
  public:

  void* stream;

  StreamOpAddr( ) : StreamBase<StreamOpAddr>() {}

  StreamOpAddr( Supplier s ) : stream(s) {}

  StreamOpAddr( const StreamOpAddr& rhs ) :
    StreamBase<StreamOpAddr>(rhs),
    stream(rhs.stream)
  {}

  inline void open()
  {
    if (state == closed) {
      TRACE("StreamOpAddr()::open");
      qp->Open(stream);
      state = opened;
    }
  }

  inline void close()
  {
    if (state != closed) {
      TRACE("StreamOpAddr()::close");
      qp->Close(stream);
      state = closed;
    }
  }

/*
The next function gets a tuple of the input stream and returns
the tuple or a marker. At the first call a marker will be produced
This ensures that operators stream-downwards will always know the
requested partsize. Afterwards when the amount of ~tuples~ requests are made
a marker will be injected.

*/


  inline void* getNext() const
  {
    static Word element;
    if (state != finished) {
    qp->Request(stream, element);
    if( qp->Received(stream) ) {
      return element.addr;
    }
    else {
      state = finished;
      return 0;
    }
    }
    return 0;
  }

};


/*
1.4 dpLocalInfo Class


*/
typedef enum { tupBufferA, tupBufferB } tupleBufferType;

class pjLocalInfo
{
private:

  Word mixedStream;
  StreamOpAddr joinFun;
  ListExpr aTypeInfo;
  ListExpr bTypeInfo;

  TupleBuffer *tbA;
  TupleBuffer *tbB;
  int tpIndex_A, tpIndex_B;
  const int maxMem;
  bool endOfStream;
  bool isBufferFilled;

  int bucketNum;
  int tupNum;


  //Get the tuples within one bucket, and fill them into tupleBuffers
  void loadTuples()
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

        endOfStream = true;
        break;
      }
      else if (numOfA == 0 || numOfB == 0)
      {
        delete tbA;
        delete tbB;
      }
      else
      {
        tpIndex_A = tpIndex_B = 0;
        isBufferFilled = true;
        break;
      }
    }

  }

public:
  pjLocalInfo(Word inputStream, Supplier fun, Supplier s,
      ListExpr ttA, ListExpr ttB,
      int mem = 1024 * 1024) :
    mixedStream(inputStream),
    aTypeInfo(ttA),
    bTypeInfo(ttB),
    tbA(0),
    tbB(0),
    tpIndex_A(-1),
    tpIndex_B(-1),
    maxMem(mem),
    endOfStream(false),
    isBufferFilled(false)
  {
    //Indicator both arguments of the function accept stream
    qp->SetupStreamArg(fun, 1, s);
    qp->SetupStreamArg(fun, 2, s);
    joinFun = StreamOpAddr(fun);
    joinFun.open();
  }

  ~pjLocalInfo()
  {
    if (tbA != 0)
      delete tbA;
    tbA = 0;

    if (tbB != 0)
      delete tbB;
    tbB = 0;
  }

  // Get the next tuple from tupleBuffer A or tuppleBuffer B.
  Word getNextInputTuple(tupleBufferType tbt)
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

  // Get the next result tuple
  Word getNextTuple()
  {
    Word funResult(Address(0));

    while (!endOfStream)
    {
      //qp->Request(joinFun, funResult);
      Tuple *t = (Tuple*)joinFun.getNext();
      if (t){
        return SetWord(t);
      }
      else if (endOfStream) {
        //qp->Close(joinFun);
        joinFun.close();
        return SetWord(Address(0));
      }
      else {
        // No more result in current bucket, need move to next bucket
        //qp->Close(joinFun);
        joinFun.close();
        loadTuples();
        if (isBufferFilled)
          joinFun.open();
          //qp->Open(joinFun);
        continue;
      }
    }

    return SetWord(Address(0));
  }

};


#endif /* HADOOPPARALLELALGEBRA_H_ */
