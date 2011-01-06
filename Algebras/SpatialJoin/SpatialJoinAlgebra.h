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

extern NestedList* nl;
extern QueryProcessor* qp;

using namespace extrel2;

typedef enum {leftStream, rightStream} streamType;

template <unsigned dim>
class SpatialJoinLocalInfo
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

  void scanStream(int attrIndex, streamType loc);

public:
  SpatialJoinLocalInfo(Word leftStreamWord, bool isLRel,
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

  ~SpatialJoinLocalInfo(){
    for(int i =  0; i < 2; i++)
    {
      if (r[i].streamBuffer)
      {
        delete r[i].streamBuffer;
        r[i].streamBuffer = 0;
      }
    }
  }

};


#endif /* SPATIALJOIN_ALGEBRA_H__ */
