/*
//paragraph [1] Title: [{\Large \bf \begin{center}] [\end{center}}]
//paragraph [10] Footnote: [{\footnote{] [}}]
//[TOC] [\tableofcontents]

[1] Header-File of SpatialJoin-Algebra

December 2010, Jiamin Lu


[TOC]

1 Overview

This header file defines an auxiliary class SpatialJoinLocalInfo.

2 Defines and Includes

*/
#ifndef SPATIALJOIN_ALGEBRA_H__
#define SPATIALJOIN_ALGEBRA_H__

#include <iostream>

using namespace std;

#include "SpatialAlgebra.h"
#include "RelationAlgebra.h"
#include "Algebra.h"
#include "NestedList.h"
#include "QueryProcessor.h"
#include "RectangleAlgebra.h"
#include "StandardTypes.h"
#include "ListUtils.h"
#include "RectangleAlgebra.h"
#include "TupleBuffer2.h"
#include "Progress.h"

extern NestedList* nl;
extern QueryProcessor* qp;

using namespace extrel2;

typedef enum {leftStream, rightStream} streamType;

template <unsigned dim>
class SpatialJoin2LocalInfo
{
private:

  Rectangle<dim> *joinBox;
  Supplier pf; //parameter function
  bool isSet;  //ensure setting the function's parameters

  struct{
    bool isRel;
    Word streamWord;
    TupleBuffer2 *streamBuffer;
    TupleBuffer2Iterator *tb2Iter;
    int card;
    Rectangle<dim> *MBR;
    double avgSize[dim];
  }r[2];

  void scanStream(int attrIndex, streamType loc, Supplier s);

public:
  SpatialJoin2LocalInfo(Word leftStreamWord, bool isLRel,
                       Word leftAttrIndexWord,
                       Word rightStreamWord, bool isRRel,
                       Word rightAttrIndexWord,
                       Word funWord, Supplier s);

  void openInputStream(streamType loc);
  Tuple* getNextInputTuple(streamType loc);
  void closeInputStream(streamType loc);

  inline void OpenFunction()
  {
    if (isSet)
      qp->Open(pf);
  }
  Tuple* NextResultTuple();

  ~SpatialJoin2LocalInfo(){
    for(int i =  0; i < 2; i++)
    {
      if (r[i].streamBuffer)
      {
        delete r[i].streamBuffer;
        r[i].streamBuffer = 0;
      }
      if (r[i].MBR){
        delete r[i].MBR;
      }
    }
    // delete functions arguments created in the constructor
    ArgVectorPointer funargs = qp->Argument(pf);
    if(dim == 2){
       
       delete (CcReal*)  (*funargs)[2].addr;
       delete (CcReal*)  (*funargs)[3].addr;
       delete (CcReal*)  (*funargs)[4].addr;
       delete (CcReal*)  (*funargs)[5].addr;
       delete (CcInt*)   (*funargs)[6].addr;
       delete (CcReal*)  (*funargs)[7].addr;
       delete (CcReal*)  (*funargs)[8].addr;
    } else { // dim==3
       delete (CcReal*)  (*funargs)[2].addr;
       delete (CcReal*)  (*funargs)[3].addr;
       delete (CcReal*)  (*funargs)[4].addr;
       delete (CcReal*)  (*funargs)[5].addr;
       delete (CcInt*)  (*funargs)[6].addr;
       delete (CcReal*)  (*funargs)[7].addr;
       delete (CcReal*)  (*funargs)[8].addr;
       delete (CcReal*)  (*funargs)[9].addr;
       delete (CcReal*)  (*funargs)[10].addr;
       delete (CcInt*)  (*funargs)[11].addr;
       delete (CcReal*)  (*funargs)[12].addr;
    }
    delete joinBox;


  }

};

/*
1.4 pj2LocalInfo Class

Assists ~parajoin2~ operator.

*/
typedef enum { tupBufferA, tupBufferB } tupleBufferType;

class pj2LocalInfo: public ProgressLocalInfo
{
private:
  Word streamA, streamB;
  Supplier pf;  //parameter function
  int keyAIndex, keyBIndex;

  TupleBuffer *tba, *tbb;
  GenericRelationIterator *ita, *itb;
  RTuple cta, ctb;


  int maxMem;
  bool endOfStream;       //End of the output stream
  bool moreInputTuples;   //End of the input stream

  bool LoadTuples();
  //Load tuples with same key attribute value

  int CompareTuples(Tuple* ta, int kai, Tuple* tb, int kbi);

  inline Tuple* NextTuple(Word stream)
  {
    bool yield = false;
    Word result( Address(0) );

    if(stream.addr)
    {
      qp->Request(stream.addr, result);
      yield = qp->Received(stream.addr);

      if(yield) {
        return static_cast<Tuple*> (result.addr);
      }
    }

    return 0;
  }

public:
  pj2LocalInfo(Word _sa, Word _sb, Word _kai, Word _kbi,
               Word _fun, Supplier s)
  : streamA(_sa), streamB(_sb),
    tba(0), tbb(0), ita(0), itb(0), cta(0), ctb(0),
    endOfStream(false), moreInputTuples(true)
  {
    keyAIndex = StdTypes::GetInt( _kai ) - 1;
    keyBIndex = StdTypes::GetInt( _kbi ) - 1;

    pf = _fun.addr;
    qp->SetupStreamArg(pf, 1, s);
    qp->SetupStreamArg(pf, 2, s);
    qp->Open(pf);

    maxMem = (qp->GetMemorySize(s) * 1024 * 1024);
  }

  ~pj2LocalInfo()
  {
    if (ita){
      delete ita; ita = 0;}
    if (tba){
      delete tba; tba = 0;}

    if (itb){
      delete itb; itb = 0;}
    if (tbb){
      delete tbb; tbb = 0;}
  }

  Tuple* getNextTuple();

  inline Tuple* getNextInputTuple(tupleBufferType tbt)
  {
    if (ita && tupBufferA == tbt)
      return ita->GetNextTuple();
    else if (itb && tupBufferB == tbt)
      return itb->GetNextTuple();
    else
      return 0;
  }
};


#endif /* SPATIALJOIN_ALGEBRA_H__ */
