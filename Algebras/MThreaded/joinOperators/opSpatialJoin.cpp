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



1 Implementation of the multithread sptial join operator

*/

#include <thread>
#include "opSpatialJoin.h"
#include "NestedList.h"         // required at many places
#include "QueryProcessor.h"     // needed for implementing value mappings
#include "Operator.h"           // for operator creation
#include "StandardTypes.h"      // provides int, real, string, bool type
#include "Symbols.h"            // predefined strings
#include "ListUtils.h"          // useful functions for nested lists
#include "LogMsg.h"             // send error messages
#include <cmath>
#include <cfloat>
#include <random>
#include "Algebras/Rectangle/RectangleAlgebra.h"
#include <time.h>

using namespace mthreaded;
using namespace std;

extern NestedList* nl;
extern QueryProcessor* qp;

namespace spatialJoinGlobal {
static constexpr double MEMFACTOR = 1.2;
constexpr unsigned int DIMENSIONS = 2;
condition_variable workers;
mutex workersDone_;
bool workersDone;
size_t threadsDone;
}

using namespace spatialJoinGlobal;

CandidateWorker::CandidateWorker(
        size_t _globalMem, size_t _coreNoWorker,
        size_t _streamInNo,
        shared_ptr<SafeQueuePersistent> _tupleBuffer,
        shared_ptr<SafeQueuePersistent> _partBufferR,
        shared_ptr<SafeQueuePersistent> _partBufferS,
        pair<size_t, size_t> _joinAttr,
        const double _resize,
        TupleType* _resultTupleType, const Rect* _gridcell) :
        maxMem(_globalMem),
        workersMem(_globalMem),
        coreNoWorker(_coreNoWorker),
        streamInNo(_streamInNo),
        tupleBuffer(_tupleBuffer),
        partBufferR(_partBufferR),
        partBufferS(_partBufferS),
        joinAttr(_joinAttr),
        resize(_resize),
        resultTupleType(_resultTupleType),
        gridcell(_gridcell) {
   countInMem = 0;
   if (resize == 0.0) {
      calcBbox = make_shared<std::function<Rect(Tuple*, size_t)>>(
              [](Tuple* t, size_t attr) {
                 Rect rect = ((StandardSpatialAttribute<2>*)
                         t->GetAttribute(attr))->BoundingBox();
                 return rect;

              });
   } else {
      calcBbox = make_shared<std::function<Rect(Tuple*, size_t)>>(
              [=](Tuple* t, size_t attr) {
                 Rect rect = ((StandardSpatialAttribute<2>*)
                         t->GetAttribute(attr))->BoundingBox();
                 return rect.Extend(resize);
              });
   }
   rtreeR = make_shared<mmrtree::RtreeT<DIM, TupleId>>(MINRTREE, MAXRTREE);
}

CandidateWorker::~CandidateWorker() {
   rtreeR.reset();
}


// Thread
void CandidateWorker::operator()() {
   workersDone = false;

   Tuple* tupleR = partBufferR->dequeue();
   ttR = tupleR->GetTupleType();
   ttR->IncReference();
   size_t tupleSize = tupleR->GetMemSize();
   TupleId id = 0;
   bool overflowR = false;
   shared_ptr<FileBuffer> overflowBufferR = make_shared<FileBuffer>(ttR);

   while (tupleR != nullptr) {
      calcRtree(tupleR, id, overflowBufferR, overflowR);
      ++id;
      tupleR = partBufferR->dequeue();
   }
   overflowBufferR->closeWrite();

   Tuple* tupleS = partBufferS->dequeue();
   if (tupleS != nullptr) {
      TupleType* ttS = tupleS->GetTupleType();
      ttS->IncReference();

      shared_ptr<Buffer> overflowBufferS;
      if (overflowR) {
         overflowBufferS = make_shared<FileBuffer>(ttS);
      }
      while (tupleS != nullptr) {
         calcResult(tupleS);
         if (overflowR) {
            overflowBufferS->appendTuple(tupleS);
         } else {
            std::lock_guard<std::mutex> lockT(mutexTupleCounter_);
            tupleS->DeleteIfAllowed();
         }
         //cout << tupleS->GetNumOfRefs() << "*";
         tupleS = partBufferS->dequeue();
      }

      freeRTree();
      rtreeR = make_shared<mmrtree::RtreeT<DIM, TupleId>>(MINRTREE, MAXRTREE);

      if (overflowR) {
         const size_t countOverflow = (size_t) id - countInMem - 1;
         size_t iterationsR = calcIterations(countOverflow, tupleSize);
         cout << "iterations on R: " << iterationsR << endl;
         TupleId oneRun = countOverflow / iterationsR;
         overflowBufferR->openRead();
         for (size_t i = 0; i < iterationsR; ++i) {
            for (TupleId id = 0; id < ((i == 0) ? (oneRun +
                                                   (TupleId) (countOverflow %
                                                              iterationsR))
                                                : oneRun); ++id) {
               Tuple* tupleR = overflowBufferR->readTuple();
               bufferRMem.push_back(tupleR);
               rtreeR->insert(((StandardSpatialAttribute<DIM>*)
                                      tupleR->GetAttribute(joinAttr.first))
                                      ->BoundingBox(),
                              id);
            }
            overflowBufferS->openRead();
            Tuple* tupleS = overflowBufferS->readTuple();
            while (tupleS != nullptr) {
               calcResult(tupleS);
               {
                  std::lock_guard<std::mutex> lockT(mutexTupleCounter_);
                  tupleS->DeleteIfAllowed();
               }
               tupleS = overflowBufferS->readTuple();
            }
            overflowBufferS->closeWrite();
            freeRTree();
            rtreeR = make_shared<mmrtree::RtreeT<DIM, TupleId>>(MINRTREE,
                                                                MAXRTREE);
         }
         overflowBufferR->closeWrite();
      }
      ttS->DeleteIfAllowed();
   }
   ttR->DeleteIfAllowed();
   delete gridcell;
   --threadsDone;
   if (threadsDone == 0) {
      tupleBuffer->enqueue(nullptr);
      lock_guard<std::mutex> lock(workersDone_);
      workersDone = true;
      workers.notify_all();
   } else {
      std::unique_lock<std::mutex> lock(workersDone_);
      workers.wait(lock, [&] { return workersDone; });
   }
}

void CandidateWorker::calcRtree(Tuple* tuple, TupleId id,
                                shared_ptr<Buffer> overflowBufferR,
                                bool &overflowR) {
   if (!overflowR) {
      calcMem(tuple);
      bufferRMem.push_back(tuple);
      rtreeR->insert((*calcBbox)(tuple, joinAttr.first), id);
      workersMem = workersMem - rtreeR->usedMem();
      if (workersMem > maxMem) {
         overflowR = true;
         countInMem = (size_t) id;
      }
   } else {
      overflowBufferR->appendTuple(tuple);
   }
}

void CandidateWorker::calcResult(Tuple* tuple) {
   Rect bboxS = ((StandardSpatialAttribute<DIM>*)
           tuple->GetAttribute(joinAttr.second))->BoundingBox();
   mmrtree::RtreeT<DIM, TupleId>::iterator* it = rtreeR->find(bboxS);
   const TupleId* id;
   while ((id = it->next())) {
      Tuple* tupleR = bufferRMem[(size_t) *id];
      Rect bboxR = (*calcBbox)(tupleR, joinAttr.first);
      if (reportTopright(topright(&bboxR), topright(&bboxS))) {
         auto* result = new Tuple(resultTupleType);
         Concat(tupleR, tuple, result);
         tupleBuffer->enqueue(result);
      }
   }
   delete it;
}

size_t CandidateWorker::topright(Rect* r1) const {
   size_t value = 0;
   if (r1->MaxD(0) >= gridcell->MaxD(0)) { value++; }
   if (r1->MaxD(1) >= gridcell->MaxD(1)) {
      value += 2;
   }
   return value;
}

inline bool CandidateWorker::reportTopright(size_t r1, size_t r2) const {
   return ((r1 & r2) == 0) || (r1 + r2 == 3);
}

inline void CandidateWorker::calcMem(Tuple* tuple) {
   {
      workersMem = workersMem - tuple->GetMemSize() - sizeof(void*) +
                   rtreeR->usedMem();
   }
}

void CandidateWorker::freeRTree() {
   size_t tupleMemSize = 0;
   for (Tuple* tupleR : bufferRMem) {
      tupleMemSize += tupleR->GetMemSize() + sizeof(void*);
      std::lock_guard<std::mutex> lockT(mutexTupleCounter_);
      tupleR->DeleteIfAllowed();
   }
   size_t usedMemRTree = rtreeR->usedMem();
   workersMem = workersMem - tupleMemSize - usedMemRTree;
   bufferRMem.clear();
}

size_t CandidateWorker::calcIterations(const size_t countOverflow,
                                       const size_t tupleSize) const {
   size_t memOverflow = ULONG_MAX;
   size_t iterations = 0;
   while (memOverflow > maxMem) {
      ++iterations;
      memOverflow = rtreeR->guessSize(countOverflow / iterations,
                                      true);
      memOverflow += (countOverflow / iterations) * (sizeof(void*) + tupleSize);
   }
   return iterations;
}

//Constructor
spatialJoinLI::spatialJoinLI(Word _streamR, Word _streamS,
                             pair<size_t, size_t> _joinAttr,
                             double _resize,
                             size_t _maxMem,
                             ListExpr resultType) :
        streamR(_streamR), streamS(_streamS), joinAttr(_joinAttr),
        resize(_resize), maxMem(_maxMem) {
   resultTupleType = new TupleType(nl->Second(resultType));
   resultTupleType->IncReference();
   vector<Tuple*> bufferR = {};
   coreNo = MThreadedSingleton::getCoresToUse();
   coreNoWorker = pow(coreNo - 1, 2);
   bboxsample = BBOXSAMPLESTEPS * coreNoWorker;
   tupleBuffer = make_shared<SafeQueuePersistent>(maxMem / 10,
                                                  resultTupleType);
   if (resize == 0.0) {
      calcBbox = make_shared<std::function<Rect(Tuple*, size_t)>>(
              [](Tuple* t, size_t attr) {
                 Rect rect = ((StandardSpatialAttribute<2>*)
                         t->GetAttribute(attr))->BoundingBox();
                 return rect;
              });
   } else {
      calcBbox = make_shared<std::function<Rect(Tuple*, size_t)>>(
              [=](Tuple* t, size_t attr) {
                 Rect rect = ((StandardSpatialAttribute<2>*)
                         t->GetAttribute(attr))->BoundingBox();
                 return rect.Extend(resize);
              });
   }
   streamR.open();
   streamS.open();
   irrGrid2d = nullptr;
   Scheduler();
}


//Destructor
spatialJoinLI::~spatialJoinLI() {
   for (CellInfo* info : cellInfoVec) {
      delete info;
   }
   cellInfoVec.clear();
   delete irrGrid2d;
   resultTupleType->DeleteIfAllowed();
}

//Output
Tuple* spatialJoinLI::getNext() {
   Tuple* res;
   res = tupleBuffer->dequeue();
   if (res != nullptr) {
      return res;
   }
   // neccecary to secure following operators working well
   this_thread::sleep_for(std::chrono::nanoseconds(100));
   return 0;
}

void spatialJoinLI::Scheduler() {
   // build bbox
   // Stream R
   globalMem = 9 * maxMem / 10;
   workersDone = false;
   Tuple* tupleR = streamR.request();
   Tuple* tupleS = streamS.request();
   if (tupleR != nullptr && tupleS != nullptr) {
      ttR = tupleR->GetTupleType();
      ttR->IncReference();
      ttS = tupleS->GetTupleType();
      ttS->IncReference();
      MultiBuffer bufferR = MultiBuffer(ttR, maxMem);
      vector<Rectangle<2>> bboxRSample;
      Rect bboxSpan = (*calcBbox)(tupleR, joinAttr.first);
      mt19937 rng(chrono::steady_clock::now().time_since_epoch().count());
      size_t count = 0;
      do {
         size_t replaceNo = uniform_int_distribution<size_t>(0, count)(rng);
         if (bboxRSample.size() < bboxsample) {
            Rect bbox = (*calcBbox)(tupleR, joinAttr.first);
            bboxRSample.push_back(bbox);
            double minB[] = {min(bboxSpan.MinD(0), bbox.MinD(0)),
                             min(bboxSpan.MinD(1), bbox.MinD(1))};
            double maxB[] = {max(bboxSpan.MaxD(0), bbox.MaxD(0)),
                             max(bboxSpan.MaxD(1), bbox.MaxD(1))};
            bboxSpan.Set(true, minB, maxB);
         } else if (replaceNo < bboxsample) {
            Rect bbox = (*calcBbox)(tupleR, joinAttr.first);

            bboxRSample[replaceNo] = bbox;
            double minB[] = {min(bboxSpan.MinD(0), bbox.MinD(0)),
                             min(bboxSpan.MinD(1), bbox.MinD(1))};
            double maxB[] = {max(bboxSpan.MaxD(0), bbox.MaxD(0)),
                             max(bboxSpan.MaxD(1), bbox.MaxD(1))};
            bboxSpan.Set(true, minB, maxB);
         }
         bufferR.appendTuple(tupleR);
         count++;
         if (count % CHANGEBOXSAMPLESTEP == 0) {
            bboxsample += BBOXSAMPLESTEPS;
         }
      } while ((tupleR = streamR.request()));
      streamR.close();
      bufferR.closeWrite();

      // not enough MBBs for cores
      if (bboxRSample.size() < coreNoWorker) {
         coreNoWorker = bboxRSample.size();
         irrGrid2d =
                 new IrregularGrid2D(bboxSpan, 1, coreNoWorker);
         irrGrid2d->SetVector(&bboxRSample, bboxSpan, 1,
                              coreNoWorker);
         cellInfoVec = IrregularGrid2D::getCellInfoVector(irrGrid2d);
      } else {
         irrGrid2d =
                 new IrregularGrid2D(bboxSpan, sqrt(coreNoWorker),
                                     sqrt(coreNoWorker));
         irrGrid2d->SetVector(&bboxRSample, bboxSpan,
                              sqrt(coreNoWorker),
                              sqrt(coreNoWorker));
         cellInfoVec = IrregularGrid2D::getCellInfoVector(irrGrid2d);
      }

      //resize cells at margin of grid to min/max
      const size_t gridSize = cellInfoVec.size();
      const size_t edgeSize = sqrt(coreNoWorker);
      for (CellInfo* cellInfo : cellInfoVec) {
         double minB[] =
                 {(cellInfo->cell)->MinD(0), (cellInfo->cell)->MinD(1)};
         double maxB[] =
                 {(cellInfo->cell)->MaxD(0), (cellInfo->cell)->MaxD(1)};
         if ((size_t) cellInfo->cellId <= edgeSize) {
            minB[1] = -DBL_MAX;
         }
         if ((size_t) cellInfo->cellId > gridSize - edgeSize) {
            maxB[1] = DBL_MAX;
         }
         if ((size_t) cellInfo->cellId % edgeSize == 1) {
            minB[0] = -DBL_MAX;
         }
         if ((size_t) cellInfo->cellId % edgeSize == 0) {
            maxB[0] = DBL_MAX;
         }
         (cellInfo->cell)->Set(true, minB, maxB);
      }

      // start threads
      threadsDone = coreNoWorker;
      for (size_t i = 0; i < coreNoWorker; ++i) {
         partBufferR.emplace_back(make_shared<SafeQueuePersistent>(
                 2 * (globalMem / coreNoWorker) / 16, ttR));
         partBufferS.emplace_back(make_shared<SafeQueuePersistent>(
                 (globalMem / coreNoWorker) / 16, ttS));
         joinThreads.emplace_back(
                 CandidateWorker(13 * (globalMem / coreNoWorker) / 16,
                                 coreNoWorker, i,
                                 tupleBuffer,
                                 partBufferR.back(),
                                 partBufferS.back(),
                                 joinAttr,
                                 resize,
                                 resultTupleType,
                                 cellInfoVec[i]->cell));
      }

      // Stream R
      bufferR.openRead();
      Tuple* tuple = bufferR.readTuple();
      while (tuple != nullptr) {
         Rect bbox = (*calcBbox)(tuple, joinAttr.first);
         for (CellInfo* cellInfo : cellInfoVec) {
            if ((cellInfo->cell)->Intersects(bbox)) {
               {
                  std::lock_guard<std::mutex> lockT(mutexTupleCounter_);
                  tuple->IncReference();
               }
               partBufferR[(cellInfo->cellId - 1)]->enqueue(tuple);
            }
         }
         {
            std::lock_guard<std::mutex> lockT(mutexTupleCounter_);
            tuple->DeleteIfAllowed();
         }
         tuple = bufferR.readTuple();
      }
      for (size_t i = 0; i < coreNoWorker; ++i) {
         partBufferR[i]->enqueue(nullptr);
      }

      // grid partitioning S
      while (tupleS != nullptr) {
         Rect bbox = ((StandardSpatialAttribute<2>*)
                 tupleS->GetAttribute(joinAttr.second))->BoundingBox();
         for (CellInfo* cellInfo : cellInfoVec) {
            if ((cellInfo->cell)->Intersects(bbox)) {
               tupleS->IncReference();
               partBufferS[(cellInfo->cellId - 1)]->enqueue(tupleS);
            }
         }
         {
            std::lock_guard<std::mutex> lockT(mutexTupleCounter_);
            tupleS->DeleteIfAllowed();
         }
         tupleS = streamS.request();
      }
      streamS.close();

      for (size_t i = 0; i < coreNoWorker; ++i) {
         partBufferS[i]->enqueue(nullptr);
      }

      for (size_t i = 0; i < coreNoWorker; ++i) {
         joinThreads[i].join();
      }
   } else {
      tupleBuffer->enqueue(nullptr);
   }
}


ListExpr op_spatialJoin::spatialJoinTM(ListExpr args) {

   // mthreadedSpatialJoin has 4 arguments
   // 1: StreamR of Tuple with spatial Attr
   // 2: StreamS of Tuple with spatial Attr
   // 3: Attr R
   // 4: Attr S
   // 5: resize bbox by real

   if (MThreadedSingleton::getCoresToUse() < 3) {
      return listutils::typeError(" only works with >= 3 threads ");
   }

   string err = "stream(tuple) x stream(tuple) x attr1 x "
                "attr2  x real expected";
   if (!nl->HasLength(args, 5)) {
      return listutils::typeError(err);
   }
   ListExpr stream1 = nl->First(args);
   ListExpr stream2 = nl->Second(args);
   ListExpr attr1 = nl->Third(args);
   ListExpr attr2 = nl->Fourth(args);
   ListExpr resize = nl->Fifth(args);

   if (!Stream<Tuple>::checkType(stream1)) {
      return listutils::typeError
              (err + " (first arg is not a tuple stream)");
   }
   if (!Stream<Tuple>::checkType(stream2)) {
      return listutils::typeError
              (err + " (second arg is not a tuple stream)");
   }
   if (!listutils::isSymbol(attr1)) {
      return listutils::typeError
              (err + " (first attrname is not valid)");
   }
   if (!listutils::isSymbol(attr2)) {
      return listutils::typeError
              (err + " (second attrname is not valid)");
   }

   if (!CcReal::checkType(resize)) {
      return listutils::typeError
              (err + " (last argument has to be real)");
   }

   ListExpr attrList1 = nl->Second(nl->Second(stream1));
   ListExpr attrList2 = nl->Second(nl->Second(stream2));
   string attrname1 = nl->SymbolValue(attr1);
   string attrname2 = nl->SymbolValue(attr2);
   ListExpr attrType1;
   ListExpr attrType2;

   int index1 = listutils::findAttribute(attrList1, attrname1, attrType1);
   if (index1 == 0) {
      return listutils::typeError(attrname1 +
                                  " is not an attribute of the first stream");
   }

   int index2 = listutils::findAttribute(attrList2, attrname2, attrType2);
   if (index2 == 0) {
      return listutils::typeError(attrname1 +
                                  " is not an attribute of the second stream");
   }

   if (!listutils::isSpatialType(attrType1)) {
      return listutils::typeError(" first attribute not spatial ");
   }

   if (!listutils::isSpatialType(attrType2)) {
      return listutils::typeError(" second attribute not spatial ");
   }

   ListExpr resAttrList = listutils::concat(attrList1, attrList2);

   if (!listutils::isAttrList(resAttrList)) {
      return listutils::typeError
              ("Name conflicts in attributes found");
   }

   ListExpr indexList = nl->TwoElemList(
           nl->IntAtom(index1 - 1),
           nl->IntAtom(index2 - 1));

   //cout << nl->ToString(indexList) << endl;

   return nl->ThreeElemList(nl->SymbolAtom(Symbols::APPEND()),
                            indexList,
                            nl->TwoElemList(
                                    nl->SymbolAtom(Stream<Tuple>::BasicType()),
                                    nl->TwoElemList(
                                            nl->SymbolAtom(Tuple::BasicType()),
                                            resAttrList)));
}

// Operator SpatialJoin
// args[0] : streamR
// args[1] : streamS
// args[5] : index of join attribute R
// args[6] : index of join attribute S
// args[4] : resize bbox

int op_spatialJoin::spatialJoinVM(Word* args, Word &result, int message,
                                  Word &local, Supplier s) {

   std::pair<size_t, size_t> attr = make_pair(
           (static_cast<CcInt*>(args[5].addr))->GetIntval(),
           (static_cast<CcInt*>(args[6].addr))->GetIntval());
   double resize = (static_cast<CcReal*>(args[4].addr))->GetRealval();


   // create result type
   ListExpr resultType = qp->GetNumType(s);

   spatialJoinLI* li = (spatialJoinLI*) local.addr;

   switch (message) {

      case OPEN :
         if (li) {
            delete li;
         }
         local.addr = new spatialJoinLI(args[0], args[1],
                                        attr, resize,
                                        qp->GetMemorySize(s) *
                                        1024 * 1024,
                                        resultType);
         return 0;
      case REQUEST:
         result.addr = li ? li->getNext() : 0;
         return result.addr ? YIELD : CANCEL;
      case CLOSE:
         if (li) {
            delete li;
            local.addr = 0;
         }
         this_thread::sleep_for(std::chrono::microseconds(20));
         return 0;
   }
   return 0;
}

std::string op_spatialJoin::getOperatorSpec() {
   return OperatorSpec(
           " stream x stream x attr x attr x real -> stream",
           " streamR streamS mThreadedSpatialJoin(attr1, attr2, real)",
           " spatial join using >2 cores",
           " R feed {o} S feed {p} mThreadedSpatialJoin[X_o, Y_p, 0.0]"
   ).getStr();
}

shared_ptr<Operator> op_spatialJoin::getOperator() {
   return std::make_shared<Operator>("mThreadedSpatialJoin",
                                     getOperatorSpec(),
                                     &op_spatialJoin::spatialJoinVM,
                                     Operator::SimpleSelect,
                                     &op_spatialJoin::spatialJoinTM);
}