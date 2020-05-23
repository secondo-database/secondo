/*
----
This file is part of SECONDO.

Copyright (C) 2019,
Faculty of Mathematics and Computer Science,
Database Systems for New Applications.

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


//[<] [\ensuremath{<}]
//[>] [\ensuremath{>}]

\setcounter{tocdepth}{3}
\tableofcontents



1 Implementation of the multithread HybridJoin Operator

These Operators are:

  * mthreadedHybridJoin

*/

#include <thread>
#include "opSpatialJoin.h"

#include "Attribute.h"          // implementation of attribute types
#include "NestedList.h"         // required at many places
#include "QueryProcessor.h"     // needed for implementing value mappings
#include "AlgebraManager.h"     // e.g., check for a certain kind
#include "Operator.h"           // for operator creation
#include "StandardTypes.h"      // provides int, real, string, bool type
#include "Symbols.h"            // predefined strings
#include "ListUtils.h"          // useful functions for nested lists
#include "LogMsg.h"             // send error messages
#include <cmath>
#include <cfloat>
#include <random>
#include "Algebras/Spatial/SpatialAlgebra.h"
#include "Algebras/Rectangle/RectangleAlgebra.h"
#include "Algebras/SPart/IrregularGrid2D.h"


using namespace mthreaded;
using namespace std;

extern NestedList* nl;
extern QueryProcessor* qp;

namespace spatialJoinGlobal {
static constexpr double MEMFACTOR = 1.2;
static const double BOX_EXPAND = FACTOR;
constexpr unsigned int DIMENSIONS = 3;
vector<shared_ptr<SafeQueue<Tuple*>>> partBufferR;
vector<shared_ptr<SafeQueue<Tuple*>>> partBufferS;
shared_ptr<SafeQueue<Tuple*>> tupleBuffer;
shared_ptr<SafeQueue<std::pair<Tuple*, Tuple*>>> possiblePairBuffer;
mutex mutexFun_;
mutex mutexConcat_;
size_t threadsDone;
}

using namespace spatialJoinGlobal;

CandidateWorker::CandidateWorker(
        size_t _maxMem, size_t _coreNoWorker, size_t _streamInNo,
        pair<size_t, size_t> _joinAttr, Word _fun,
        TupleType* _resultTupleType, const Rect* _gridcell) :
        maxMem(_maxMem),
        coreNoWorker(_coreNoWorker),
        streamInNo(_streamInNo),
        joinAttr(_joinAttr),
        fun(_fun),
        resultTupleType(_resultTupleType),
        gridcell(_gridcell),
        bufferS(maxMem / 2) {

   rtreeR = make_shared<mmrtree::RtreeT<DIM, TupleId>>(2, 4);
   rtreeS = make_shared<mmrtree::RtreeT<DIM, TupleId>>(2, 4);
}

CandidateWorker::~CandidateWorker() {
   cout << "destruct candidate: " << streamInNo << endl;
}


// Thread
void CandidateWorker::operator()() {
   cout << "Worker Candidate Nr. " << streamInNo << endl;

   Tuple* tupleR = partBufferR[streamInNo]->dequeue();
   size_t count = 0;
   while (tupleR != nullptr) {
      bufferR.emplace_back(tupleR);
      ++count;
      tupleR = partBufferR[streamInNo]->dequeue();
   }
   cout << "R in Thread: " << streamInNo << " # " << count << endl;

   cout << "quicksort" << endl;
   quickSort(bufferR, 0, bufferR.size() - 1);

   cout << "Read S" << endl;
   Tuple* tupleS = partBufferS[streamInNo]->dequeue();
   count = 0;
   while (tupleS != nullptr) {
      TupleId id = bufferS.AppendTuple(tupleS);
      rtreeS->insert(((StandardSpatialAttribute<DIM>*)
              tupleS->GetAttribute(joinAttr.second))->BoundingBox(), id);
      ++count;
      tupleS = partBufferS[streamInNo]->dequeue();
   }

   cout << "S in Thread: " << streamInNo << " # " << count << endl;

   cout << "R" << endl;
   for (Tuple* tupleR : bufferR) {
      Rect bboxR = ((StandardSpatialAttribute<DIM>*)
              tupleR->GetAttribute(joinAttr.first))->BoundingBox();
      mmrtree::RtreeT<DIM, TupleId>::iterator* it = rtreeS->find(bboxR);
      const TupleId* id;
      while ((id = it->next())) {
         Tuple* tupleS = bufferS.GetTuple(*id);
         Rect bboxS = ((StandardSpatialAttribute<DIM>*) tupleS->GetAttribute(
                 joinAttr.second))->BoundingBox();
         if (bboxS.Intersects(bboxR) &&
             reportTopright(topright(&bboxR), topright(&bboxS))) {
            auto* result = new Tuple(resultTupleType);
            Concat(tupleR, tupleS, result);
            tupleBuffer->enqueue(result);
         }
      }
   }

//   cout << "refinement" << endl;
//   for (pair<Tuple*, Tuple*> possiblePair : possiblePairs) {
//      assert(possiblePair.first != nullptr);
//      assert(possiblePair.second != nullptr);
//      ArgVector &arguments = *qp->Argument(fun.addr);
//      Tuple* funarg0 = (possiblePair.first);
//      Tuple* funarg1 = (possiblePair.second);
//      if (!funarg0 || !funarg1) {
//         cout << "arg invalid" << endl;
//         return;
//      }
//      arguments[0].setAddr(funarg0);
//      arguments[1].setAddr(funarg1);
//      Word funres;
//      bool res;
//      {
//         std::lock_guard<mutex> lock(spatialJoinGlobal::mutexFun_);
//         qp->Request(fun.addr, funres);
//         res = ((CcBool*) funres.addr)->GetBoolval();
//      }
//      if (res) {
//         auto* result = new Tuple(resultTupleType);
//         Concat(possiblePair.first, possiblePair.second, result);
//         tupleBuffer->enqueue(result);
//      }
//   }

   cout << "Worker Candidate Nr. done " << streamInNo << endl;

   //for (Tuple* tupleR : bufferR) {
   //tupleR->DeleteIfAllowed();
   //}
   bufferR.clear();
   rtreeS.reset();

   cout << "cleaned" << endl;

   --threadsDone;
   if (threadsDone == 0) {
      cout << "nullptr" << endl;
      tupleBuffer->enqueue(nullptr);
   }
}

void CandidateWorker::quickSort(vector<Tuple*> &A, size_t p, size_t q) {
   size_t r;
   if (p < q) {
      r = partition(A, p, q);
      quickSort(A, p, r);
      quickSort(A, r + 1, q);
   }
}


size_t CandidateWorker::partition(vector<Tuple*> &A, size_t p, size_t q) {
   size_t x = ((StandardSpatialAttribute<DIM>*)
           A[p]->GetAttribute(joinAttr.first))->BoundingBox().MinD(0);
   size_t i = p;
   size_t j;
   for (j = p + 1; j < q; j++) {
      if (((StandardSpatialAttribute<DIM>*)
              A[j]->GetAttribute(joinAttr.first))->BoundingBox().MinD(0) <= x) {
         i = i + 1;
         swap(A[i], A[j]);
      }
   }
   swap(A[i], A[p]);
   return i;
}

size_t CandidateWorker::topright(Rect* r1) {
   size_t value = 0;
   if (r1->MaxD(0) >= gridcell->MaxD(0)) { value++; }
   if (r1->MaxD(1) >= gridcell->MaxD(1)) {
      value += 2;
   }
   return value;
}

inline bool CandidateWorker::reportTopright(size_t r1, size_t r2) {
   return ((r1 & r2) == 0) || (r1 + r2 == 3);
}


//Constructor
spatialHashJoinLI::spatialHashJoinLI(Word _streamR, Word _streamS, Word _fun,
                                     pair<size_t, size_t> _joinAttr,
                                     size_t _maxMem,
                                     ListExpr resultType) :
        streamR(_streamR), streamS(_streamS), fun(_fun), joinAttr(_joinAttr),
        maxMem(_maxMem) {
   resultTupleType = new TupleType(nl->Second(resultType));
   resultTupleType->IncReference();
   coreNo = MThreadedSingleton::getCoresToUse();
   coreNoWorker = coreNo - 1;
   streamR.open();
   streamS.open();
   Scheduler();
}


//Destructor
spatialHashJoinLI::~spatialHashJoinLI() {
   cout << "destuctor info-class" << endl;
   partBufferR.clear();
   partBufferS.clear();
   resultTupleType->DeleteIfAllowed();
}

//Output
Tuple* spatialHashJoinLI::getNext() {
   Tuple* res;
   res = tupleBuffer->dequeue();
   if (res != nullptr) {
      return res;
   }
   return 0;
}

void spatialHashJoinLI::Scheduler() {
   // build bbox
   // Stream R
   Tuple* tupleR = streamR.request();
   TupleType* ttR = tupleR->GetTupleType();
   ttR->IncReference();
   size_t tupleSize = tupleR->GetMemSize(); // only first tuple
   vector<Tuple*> bufferR;
   //tupleR->GetTupleId();
   vector<Rectangle<2>> bboxRSample;
   Rect bboxSpan = ((StandardSpatialAttribute<2>*)
           tupleR->GetAttribute(joinAttr.first))->BoundingBox();
   mt19937 rng(chrono::steady_clock::now().time_since_epoch().count());
   size_t count = 0;
   do {
      tupleR->IncReference();
      bufferR.push_back(tupleR);
      size_t replaceNo = uniform_int_distribution<size_t>(0, count)(rng);
      if (count < bboxsample || replaceNo < bboxsample) {
         Rect bbox = ((StandardSpatialAttribute<2>*)
                 tupleR->GetAttribute(joinAttr.first))->BoundingBox();
         if (count < bboxsample) {
            bboxRSample.push_back(bbox);
         } else {
            bboxRSample[replaceNo] = bbox;
         }
         double minB[] = {min(bboxSpan.MinD(0), bbox.MinD(0)),
                          min(bboxSpan.MinD(1), bbox.MinD(1))};
         double maxB[] = {max(bboxSpan.MaxD(0), bbox.MaxD(0)),
                          max(bboxSpan.MaxD(1), bbox.MaxD(1))};
         bboxSpan.Set(true, minB, maxB);
      }
      count++;
      if (count % 1000 == 0) {
         bboxsample += BBOXSAMPLESTEPS;
      }
   } while ((tupleR = streamR.request()));
   streamR.close();
   cout << bboxSpan.MinD(0) << "##" << bboxSpan.MaxD(0) << "Size: " << tupleSize
        << endl;
//   cout << "bbox sample" << endl;
//   for (Rect r : bboxRSample) {
//      cout << r.MinD(0) << " - " << r.MaxD(0) << "##";
//      cout << r.MinD(1) << " - " << r.MaxD(1) << endl;
//   }


   // calc partition for worker to fit in memory
   size_t gridY = 1;
   bool overflow = false;
   size_t memOverflowFactor = ceil(tupleSize * count * MEMFACTOR / maxMem);
   cout << "Factor: " << memOverflowFactor << endl;
   vector<shared_ptr<Buffer>> overflowBufferR;
   if (memOverflowFactor > coreNoWorker) {
      gridY = memOverflowFactor;
      overflow = true;
      for (size_t i = coreNoWorker; i < gridY * coreNoWorker; ++i) {
         overflowBufferR.emplace_back(make_shared<FileBuffer>(ttR));
      }
   }

   auto* irrGrid2d = new IrregularGrid2D(bboxSpan, gridY, coreNoWorker);
   irrGrid2d->SetVector(&bboxRSample, bboxSpan, gridY, coreNoWorker);

   vector<CellInfo*> cellInfoVec = IrregularGrid2D::getCellInfoVector(
           irrGrid2d);
   //resize cells at margin of grid to min/max
   size_t gridSize = cellInfoVec.size();
   for (CellInfo* cellInfo : cellInfoVec) {
      double minB[] = {(cellInfo->cell)->MinD(0), (cellInfo->cell)->MinD(1)};
      double maxB[] = {(cellInfo->cell)->MaxD(0), (cellInfo->cell)->MaxD(1)};
      if ((size_t) cellInfo->cellId <= coreNoWorker) {
         minB[1] = -DBL_MAX;
      }
      if ((size_t) cellInfo->cellId > gridSize - coreNoWorker) {
         maxB[1] = DBL_MAX;
      }
      if ((size_t) cellInfo->cellId % coreNoWorker == 1) {
         minB[0] = -DBL_MAX;
      }
      if ((size_t) cellInfo->cellId % coreNoWorker == 0) {
         maxB[0] = DBL_MAX;
      }
      (cellInfo->cell)->Set(true, minB, maxB);
      cout << (cellInfo->cell)->MinD(0) << " - " << (cellInfo->cell)->MaxD(0)
           << endl;
      cout << (cellInfo->cell)->MinD(1) << " - " << (cellInfo->cell)->MaxD(1)
           << endl;
      cout << (cellInfo->cellId) << " ## " << (cellInfo->statNbrOfPoints)
           << endl;
   }


   // start threads
   joinThreads.reserve(coreNoWorker);
   partBufferR.reserve(coreNoWorker);
   partBufferS.reserve(coreNoWorker);
   tupleBuffer = make_shared<SafeQueue<Tuple*>>(coreNoWorker);
   threadsDone = coreNoWorker;
   for (size_t i = 0; i < coreNoWorker; ++i) {
      partBufferR.push_back(make_shared<SafeQueue<Tuple*>>(i));
      partBufferS.push_back(make_shared<SafeQueue<Tuple*>>(i));
      joinThreads.emplace_back(
              CandidateWorker(maxMem / coreNoWorker, coreNoWorker, i,
                              joinAttr, fun,
                              resultTupleType, cellInfoVec[i]->cell));
      joinThreads.back().detach();
   }

   // Stream R
   count = 0;
   if (!overflow) {
      cout << "not overflow" << endl;
      for (Tuple* tuple : bufferR) {
         Rect bbox = ((StandardSpatialAttribute<2>*)
                 tuple->GetAttribute(joinAttr.first))->BoundingBox();
         for (CellInfo* cellInfo : cellInfoVec) {
            if ((cellInfo->cell)->Intersects(bbox)) {
               partBufferR[cellInfo->cellId - 1]->enqueue(tuple);
            }
         }
         ++count;
      }
   } else {
      cout << "Overflow" << endl;
      for (Tuple* tuple : bufferR) {
         Rect bbox = (((StandardSpatialAttribute<2>*)
                 tuple->GetAttribute(joinAttr.first))->BoundingBox());
         for (CellInfo* cellInfo : cellInfoVec) {
            if ((cellInfo->cell)->Intersects(bbox)) {
               if (cellInfo->cellId < (int) coreNoWorker + 1) {
                  partBufferR[cellInfo->cellId - 1]->enqueue(tuple);
               } else
                  overflowBufferR[cellInfo->cellId - coreNoWorker -
                                  1]->appendTuple(tuple);
            }
         }
         ++count;
      }
   }

   for (size_t i = 0; i < coreNoWorker; ++i) {
      partBufferR[i]->enqueue(nullptr);
   }
   bufferR.clear();

   cout << "Stream S" << endl;
   // grid partitioning S
   count = 0;
   Tuple* tupleS = streamS.request();
   do {
      Rect bbox = ((StandardSpatialAttribute<2>*)
              tupleS->GetAttribute(joinAttr.second))->BoundingBox();
      for (CellInfo* cellInfo : cellInfoVec) {
         if ((cellInfo->cell)->Intersects(bbox)) {
            partBufferS[cellInfo->cellId - 1]->enqueue(tupleS);
         }
      }
      ++count;
   } while ((tupleS = streamS.request()));

   for (size_t i = 0; i < coreNoWorker; ++i) {
      partBufferS[i]->enqueue(nullptr);
   }
   //ttR->DeleteIfAllowed();
   cout << "Schedule Ready" << endl;
}


ListExpr op_spatialHashJoin::spatialHashJoinTM(ListExpr args) {

   // mthreadedHybridJoin has 4 arguments
   // 1: StreamR of Tuple with spatial Attr
   // 2: StreamS of Tuple with spatial Attr
   // 3: fun spatial predicate
   // TODO 4: optional use existing bbox
   // TODO 5: optional define extend of grid

   if (MThreadedSingleton::getCoresToUse() < 3) {
      return listutils::typeError(" only works with >= 3 threads ");
   }

   cout << "TM SJ: " << nl->ToString(args) << "##" << nl->ListLength(args)
        << endl;

   string err = "stream(tuple) x stream(tuple) x fun";
   if (nl->ListLength(args) != 3) {
      return listutils::typeError(err);
   }

//   const ListExpr arg1 = nl->First(nl->First(args)); //tuple-stream
//   ListExpr tupleAttr = nl->Second(args);
//   ListExpr tupleValues = nl->Second(tupleAttr); //values
//   tupleAttr = nl->First(tupleAttr); //attribute list and decr/incr

   ListExpr stream1 = nl->First(nl->First(args));
   ListExpr stream2 = nl->First(nl->Second(args));
   ListExpr fun = nl->First(nl->Third(args));
   cout << nl->ToString(stream1) << endl;
   cout << nl->ToString(stream2) << endl;
   cout << nl->ToString(fun) << endl;

   if (!Stream<Tuple>::checkType(stream1)) {
      return listutils::typeError(err + " (first arg is not a tuple stream)");
   }
   cout << "Stream" << endl;
   if (!Stream<Tuple>::checkType(stream2)) {
      return listutils::typeError(err + " (second arg is not a tuple stream)");
   }
   cout << "Stream" << endl;
   if (!listutils::isMap<2>(fun)) {
      return listutils::typeError(err + "(no map with 2 arguments)");
   }
   cout << "Map" << endl;

   ListExpr attrList1 = nl->Second(nl->Second(stream1));
   ListExpr attrList2 = nl->Second(nl->Second(stream2));
   cout << nl->ToString(attrList1) << endl;
   cout << nl->ToString(attrList2) << endl;

   ListExpr mapArg1 = nl->Second(fun);
   ListExpr mapArg2 = nl->Third(fun);
   cout << nl->ToString(mapArg1) << endl;
   cout << nl->ToString(mapArg2) << endl;

   if (!nl->Equal(nl->Second(mapArg1), attrList1)) {
      return listutils::typeError(" is not an attribute of the first stream");
   }

   if (!nl->Equal(nl->Second(mapArg2), attrList2)) {
      return listutils::typeError(" is not an attribute of the second stream");
   }

//   if(!listutils::disjointAttrNames(al1, al2)){
//      return listutils::typeError("conflicting type names");
//   }

   ListExpr mapAttr = nl->Fourth(nl->Second(nl->Third(args)));
   cout << nl->ToString(mapAttr) << endl;
   while (nl->ListLength(mapAttr) != 3) {
      mapAttr = nl->Second(mapAttr);
   }
   cout << nl->ToString(mapAttr) << endl;
   string mapAttrName1 = nl->SymbolValue(nl->Third(nl->Second(mapAttr)));
   string mapAttrName2 = nl->SymbolValue(nl->Third(nl->Third(mapAttr)));
   cout << mapAttrName1 << "##" << mapAttrName2 << endl;
   ListExpr attrType1;
   ListExpr attrType2;
   int index1 = listutils::findAttribute(attrList1, mapAttrName1, attrType1);
   int index2 = listutils::findAttribute(attrList2, mapAttrName2, attrType2);
   if (!listutils::isSpatialType(attrType1)) {
      return listutils::typeError(" first attribute not spatial ");
   }
   if (!listutils::isSpatialType(attrType2)) {
      return listutils::typeError(" second attribute not spatial ");
   }

   ListExpr funRes = nl->Fourth(fun);
   cout << nl->ToString(funRes) << endl;
   // result of the function must be an attribute again , e . g . in
   // kind DATA
   if (!CcBool::checkType(funRes)) {
      return listutils::typeError(" map result is not a bool ");
   }

   ListExpr resAttrList = listutils::concat(attrList1, attrList2);
   if (!listutils::isAttrList(resAttrList)) {
      return listutils::typeError("Name conflicts in attributes found");
   }
   resAttrList = nl->TwoElemList(nl->SymbolAtom(Symbols::STREAM()),
                                 nl->TwoElemList(
                                         nl->SymbolAtom(Symbols::TUPLE()),
                                         resAttrList));

   ListExpr appendList = nl->TwoElemList(
           nl->IntAtom(index1 - 1),
           nl->IntAtom(index2 - 1));

   cout << "result: " << nl->ToString(resAttrList) << endl;
   cout << "append: " << nl->ToString(appendList) << endl;
   nl->WriteListExpr(appendList);
   cout << "fertig TM" << endl;
   return nl->ThreeElemList(nl->SymbolAtom(Symbols::APPEND()), appendList,
                            resAttrList);
}

// Operator SpatialJoin (firstArg = 1, param = false)
// args[0] : streamR
// args[1] : streamS
// args[2] : index of join attribute R
// args[3] : index of join attribute S

int op_spatialHashJoin::spatialHashJoinVM(Word* args, Word &result, int message,
                                          Word &local, Supplier s) {

   //read append structure
   //(attribute number, sort direction)
   std::pair<size_t, size_t> attr;
   CcInt* attrR = static_cast<CcInt*>(args[3].addr);
   CcInt* attrS = static_cast<CcInt*>(args[4].addr);
   attr = make_pair(attrR->GetIntval(), attrS->GetIntval());

   // create result type
   ListExpr resultType = qp->GetNumType(s);

   spatialHashJoinLI* li = (spatialHashJoinLI*) local.addr;

   switch (message) {

      case OPEN :
         if (li) {
            delete li;
         }

         local.addr = new spatialHashJoinLI(args[0], args[1], args[2], attr,
                                            qp->GetMemorySize(s) *
                                            1024 * 1024,
                                            resultType);
         //resultTupleType->DeleteIfAllowed();
         return 0;
      case REQUEST:
         result.addr = li ? li->getNext() : 0;
         return result.addr ? YIELD : CANCEL;
      case CLOSE:
         cout << "close" << endl;
         if (li) {
            delete li;
            local.addr = 0;
         }
         return 0;
   }
   return 0;
}

std::string op_spatialHashJoin::getOperatorSpec() {
   return OperatorSpec(
           " stream x stream x fun -> stream",
           " streamR streamS mThreadedSpatialJoin(fun)",
           " spatial hash join using >2 cores",
           " R feed {o} S feed {p} mThreadedHybridJoin[X_o comntains Y_p]"
   ).getStr();
}

shared_ptr<Operator> op_spatialHashJoin::getOperator() {
   return std::make_shared<Operator>("mThreadedSpatialJoin",
                                     getOperatorSpec(),
                                     &op_spatialHashJoin::spatialHashJoinVM,
                                     Operator::SimpleSelect,
                                     &op_spatialHashJoin::spatialHashJoinTM);
}