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
#include <time.h>


using namespace mthreaded;
using namespace std;

extern NestedList* nl;
extern QueryProcessor* qp;

namespace spatialJoinGlobal {
static constexpr double MEMFACTOR = 1.2;
static const double BOX_EXPAND = FACTOR;
constexpr unsigned int DIMENSIONS = 3;
//vector<shared_ptr<SafeQueue<Tuple*>>> partBufferR{};
//vector<shared_ptr<SafeQueue<Tuple*>>> partBufferS{};
//shared_ptr<SafeQueue<Tuple*>> tupleBuffer = nullptr;
condition_variable workers;
mutex workersDone_;
bool workersDone;
size_t threadsDone;
mutex globalMem_;
//size_t globalMem;
}

using namespace spatialJoinGlobal;

CandidateWorker::CandidateWorker(
        size_t _maxMem, size_t* _globalMem, size_t _coreNoWorker,
        size_t _streamInNo,
        shared_ptr<SafeQueue<Tuple*>> _tupleBuffer,
        shared_ptr<SafeQueue<Tuple*>> _partBufferR,
        shared_ptr<SafeQueue<Tuple*>> _partBufferS,
        pair<size_t, size_t> _joinAttr,
        TupleType* _resultTupleType, const Rect* _gridcell) :
        maxMem(_maxMem),
        globalMem(_globalMem),
        coreNoWorker(_coreNoWorker),
        streamInNo(_streamInNo),
        tupleBuffer(_tupleBuffer),
        partBufferR(_partBufferR),
        partBufferS(_partBufferS),
        joinAttr(_joinAttr),
        resultTupleType(_resultTupleType),
        gridcell(_gridcell) {
   //bufferR(maxMem) {
   cout << "GlobalMemTh" << maxMem << endl;
   countInMem = 0;
   rtreeR = make_shared<mmrtree::RtreeT<DIM, TupleId>>(2, 4);
}

CandidateWorker::~CandidateWorker() {
   cout << "destruct candidate: " << streamInNo << endl;
//   if (threadsDone == 0)
//   {
//      lock_guard<std::mutex> lock(workersDone_);
//      workersDone = true;
//      workers.notify_all();
//   }
}


// Thread
void CandidateWorker::operator()() {
   cout << "Worker Candidate Nr. " << streamInNo << endl;
   workersDone = false;

   Tuple* tupleR = partBufferR->dequeue();
   ttR = tupleR->GetTupleType();
   ttR->IncReference();
   size_t tupleSize = tupleR->GetMemSize();
   TupleId id = 0;
   bool overflowR = false;
   shared_ptr<Buffer> overflowBufferR = make_shared<MemoryBuffer>(ttR);

   while (tupleR != nullptr) {
      calcRtree(tupleR, id, globalMem, overflowBufferR, overflowR);
      ++id;
      tupleR = partBufferR->dequeue();
   }
   overflowBufferR->closeWrite();
   //ostream &objOstream = cout;
   //if (streamInNo == 0) rtreeR->printAsRel(objOstream);
   cout << "R in Thread: " << streamInNo << " # " << id << endl;
   cout << "global mem" << *globalMem << endl;

   cout << "Read S" << endl;
   Tuple* tupleS = partBufferS->dequeue();
   TupleType* ttS = tupleS->GetTupleType();
   ttS->IncReference();

   shared_ptr<Buffer> overflowBufferS;
   if (overflowR) {
      overflowBufferS = make_shared<FileBuffer>(ttS);
   }
   size_t count = 0;
   //bool firstBuffer = true;
   while (tupleS != nullptr) {
      calcResult(tupleS);
      //cout << "NoRefS" << tupleS->GetNumOfRefs() << "--";
      if (overflowR) {
         overflowBufferS->appendTuple(tupleS);
      } else {
         tupleS->DeleteIfAllowed();
      }
      ++count;
      tupleS = partBufferS->dequeue();
   }

   cout << "clean" << streamInNo << endl;

   for (Tuple* tupleR : bufferRMem) {
      //cout << "NoRefR" << tupleR->GetNumOfRefs() << "--";
      tupleR->DeleteIfAllowed();
   }
   bufferRMem.clear();
   rtreeR = make_shared<mmrtree::RtreeT<DIM, TupleId>>(2, 4);

   //cout << "S in Thread: " << streamInNo << " # " << count << endl;

   cout << "Worker Candidate Nr. done " << streamInNo << endl;

   if (overflowR) {
      cout << "overflow";
      const size_t countOverflow = (size_t) id - countInMem - 1;
      size_t iterationsR = calcIterations(countOverflow, tupleSize);
      cout << "iterationsR: " << iterationsR << endl;
      TupleId oneRun = countOverflow / iterationsR;
      cout << "countOverflow" << countOverflow << "oneRun" << oneRun
           << "countInMem" << countInMem << endl;
      overflowBufferR->openRead();
      for (size_t i = 0; i < iterationsR; ++i) {
         cout << "R: " << i << endl;
         cout << ((i == 0) ? (oneRun + (TupleId) (countOverflow % iterationsR))
                           : oneRun) << endl;
         for (TupleId id = 0; id < ((i == 0) ? (oneRun +
                                                (TupleId) (countOverflow %
                                                           iterationsR))
                                             : oneRun); ++id) {
            //cout << id << "##";
            Tuple* tupleR = overflowBufferR->readTuple();
            assert(tupleR != nullptr);
            bufferRMem.push_back(tupleR);
            rtreeR->insert(((StandardSpatialAttribute<DIM>*)
                    tupleR->GetAttribute(joinAttr.first))->BoundingBox(), id);
         }
         cout << "S in Iter" << endl;
         overflowBufferS->openRead();
         Tuple* tupleS = overflowBufferS->readTuple();
         while (tupleS != nullptr) {
            calcResult(tupleS);
            tupleS->DeleteIfAllowed();
            tupleS = overflowBufferS->readTuple();
         }
         overflowBufferS->closeWrite();
         for (Tuple* tupleR : bufferRMem) {
            //cout << "NoRefR" << tupleR->GetNumOfRefs() << "--";
            tupleR->DeleteIfAllowed();
         }
         bufferRMem.clear();
         rtreeR = make_shared<mmrtree::RtreeT<DIM, TupleId>>(2, 4);
      }
      overflowBufferR->closeWrite();
   }

   ttR->DeleteIfAllowed();
   ttS->DeleteIfAllowed();
   delete gridcell;
   --threadsDone;
   if (threadsDone == 0) {
      cout << "nullptr" << endl;
      tupleBuffer->enqueue(nullptr);
      lock_guard<std::mutex> lock(workersDone_);
      workersDone = true;
      workers.notify_all();
   } else {
      cout << "wait" << endl;
      std::unique_lock<std::mutex> lock(workersDone_);
      workers.wait(lock, [&] { return workersDone; });
      cout << "awake" << endl;
   }


}

void CandidateWorker::calcRtree(Tuple* tuple, TupleId id, size_t* globalMem,
                                shared_ptr<Buffer> overflowBufferR,
                                bool &overflowR) {
   if (!overflowR) {
      calcMem(tuple, globalMem);
      bufferRMem.push_back(tuple);
      rtreeR->insert(((StandardSpatialAttribute<DIM>*)
              tuple->GetAttribute(joinAttr.first))->BoundingBox(), id);
      {
         lock_guard<std::mutex> lock(globalMem_);
         *globalMem = *globalMem - rtreeR->usedMem();
         cout << "TreeMemadd" << rtreeR->usedMem() << endl;
         cout << "GlobalMemafterAdd " << *globalMem << endl;
      }
      if (*globalMem > maxMem) {
         overflowR = true;
         countInMem = (size_t) id;
         overflowBufferR = make_shared<FileBuffer>(ttR);
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
      Rect bboxR = ((StandardSpatialAttribute<DIM>*)
              tupleR->GetAttribute(joinAttr.first))->BoundingBox();
      if (reportTopright(topright(&bboxR), topright(&bboxS))) {
         auto* result = new Tuple(resultTupleType);
         Concat(tupleR, tuple, result);
         tupleBuffer->enqueue(result);
      }
   }
   delete it;
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
              A[j]->GetAttribute(joinAttr.first))
                  ->BoundingBox().MinD(0) <= x) {
         i = i + 1;
         std::swap(A[i], A[j]);
      }
   }
   std::swap(A[i], A[p]);
   return i;
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

inline void CandidateWorker::calcMem(Tuple* tuple, size_t* globalMem) {
   lock_guard<std::mutex> lock(globalMem_);
   *globalMem = *globalMem - tuple->GetMemSize() - sizeof(void*);
   *globalMem += rtreeR->usedMem();
   cout << "TupleMem: " << tuple->GetMemSize() + sizeof(void*) << " TreeMemSub"
        << rtreeR->usedMem() << endl;
}

size_t CandidateWorker::calcIterations(const size_t countOverflow,
                                       const size_t tupleSize) const {
   size_t memOverflow = ULONG_MAX;
   size_t iterations = 0;
   while (memOverflow > maxMem / coreNoWorker) {
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
                             size_t _maxMem,
                             ListExpr resultType) :
        streamR(_streamR), streamS(_streamS), joinAttr(_joinAttr),
        maxMem(_maxMem) {
   resultTupleType = new TupleType(nl->Second(resultType));
   //resultTupleType->IncReference();
   vector<Tuple*> bufferR = {};
   //cellInfoVec = {};
   coreNo = MThreadedSingleton::getCoresToUse();
   coreNoWorker = coreNo - 1;
   tupleBuffer = make_shared<SafeQueue<Tuple*>>(coreNoWorker);
   streamR.open();
   streamS.open();
   Scheduler();
}


//Destructor
spatialJoinLI::~spatialJoinLI() {
   cout << "destuctor info-class" << endl;
   streamR.close();
   streamS.close();
   for (CellInfo* info : cellInfoVec) {
      delete info;
   }
   cellInfoVec.clear();
   delete irrGrid2d;
   //partBufferR.clear();
   //partBufferS.clear();
//   for (CellInfo* info : cellInfoVec){
//      delete info;
//   }
//   cellInfoVec.clear();
   resultTupleType->DeleteIfAllowed();
}

//Output
Tuple* spatialJoinLI::getNext() {
   Tuple* res;
   // terminates without this followed by count but not by consume
   usleep(1);
   res = tupleBuffer->dequeue();
   if (res != nullptr) {
      //cout << "hallo" << endl;
      return res;
   }
   return 0;
}

void spatialJoinLI::Scheduler() {
   // build bbox
   // Stream R
   globalMem = maxMem;
   workersDone = false;
   Tuple* tupleR = streamR.request();
   TupleType* ttR = tupleR->GetTupleType();
   ttR->IncReference();
   std::vector<Tuple*> bufferR;
   //size_t tupleSize = tupleR->GetMemSize(); // only first tuple
   vector<Rectangle<2>> bboxRSample;
   Rect bboxSpan = ((StandardSpatialAttribute<2>*)
           tupleR->GetAttribute(joinAttr.first))->BoundingBox();
   mt19937 rng(chrono::steady_clock::now().time_since_epoch().count());
   size_t count = 0;
   //auto rtree = make_shared<mmrtree::RtreeT<2, TupleId>>(2, 4);
   //auto buffer = make_shared<TupleStore1>(maxMem);
   do {
      //tupleR->IncReference();
      bufferR.push_back(tupleR);
      //TupleId id = buffer->AppendTuple(tupleR);
      //rtree->insert(((StandardSpatialAttribute<2>*)
      //        tupleR->GetAttribute(joinAttr.first))->BoundingBox(), id);
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



   // calc partition for worker to fit in memory
//   size_t gridY = 1;
//   bool overflow = false;
//   size_t memOverflowFactor = ceil(tupleSize * count * MEMFACTOR / maxMem);
//   cout << "Factor: " << memOverflowFactor << endl;
//   vector<shared_ptr<Buffer>> overflowBufferR;
//   if (memOverflowFactor > coreNoWorker) {
//      gridY = memOverflowFactor;
//      overflow = true;
//      for (size_t i = coreNoWorker; i < gridY * coreNoWorker; ++i) {
//         overflowBufferR.emplace_back(make_shared<FileBuffer>(ttR));
//      }
//   }

   irrGrid2d =
           new IrregularGrid2D(bboxSpan, 1, coreNoWorker);
   irrGrid2d->SetVector(&bboxRSample, bboxSpan, 1, coreNoWorker);
   cellInfoVec = IrregularGrid2D::getCellInfoVector(irrGrid2d);


   //resize cells at margin of grid to min/max
   size_t gridSize = cellInfoVec.size();
   for (CellInfo* cellInfo : cellInfoVec) {
      double minB[] =
              {(cellInfo->cell)->MinD(0), (cellInfo->cell)->MinD(1)};
      double maxB[] =
              {(cellInfo->cell)->MaxD(0), (cellInfo->cell)->MaxD(1)};
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
   }


   // start threads
   joinThreads.reserve(coreNoWorker);
   partBufferR.reserve(coreNoWorker);
   partBufferS.reserve(coreNoWorker);
   //tupleBuffer = make_shared<SafeQueue<Tuple*>>(coreNoWorker);
   threadsDone = coreNoWorker;
   for (size_t i = 0; i < coreNoWorker; ++i) {
      partBufferR.push_back(make_shared<SafeQueue<Tuple*>>(i));
      partBufferS.push_back(make_shared<SafeQueue<Tuple*>>(i));
      joinThreads.emplace_back(
              CandidateWorker(maxMem, &globalMem, coreNoWorker, i,
                              tupleBuffer,
                              partBufferR.back(),
                              partBufferS.back(),
                              joinAttr,
                              resultTupleType,
                              cellInfoVec[i]->cell));
      joinThreads.back().detach();
   }

   // Stream R
   count = 0;
   if (true) {
      cout << "not overflow" << endl;
      for (Tuple* tuple : bufferR) {
         Rect bbox = ((StandardSpatialAttribute<2>*)
                 tuple->GetAttribute(joinAttr.first))->BoundingBox();
         for (CellInfo* cellInfo : cellInfoVec) {
            if ((cellInfo->cell)->Intersects(bbox)) {
               tuple->IncReference();
               partBufferR[(cellInfo->cellId - 1)]->enqueue(tuple);
            }
         }
         //cout << "Ref"<<tuple->GetNumOfRefs();
         tuple->DeleteIfAllowed();
         ++count;
      }
   } else {
//      cout << "Overflow" << endl;
//      for (Tuple* tuple : bufferR) {
//         Rect bbox = (((StandardSpatialAttribute<2>*)
//                 tuple->GetAttribute(joinAttr.first))->BoundingBox());
//         for (CellInfo* cellInfo : cellInfoVec) {
//            if ((cellInfo->cell)->Intersects(bbox)) {
//               if (cellInfo->cellId < (int) coreNoWorker + 1) {
//                  tuple->IncReference();
//                  partBufferR[cellInfo->cellId - 1]->enqueue(tuple);
//               } else
//                  overflowBufferR[cellInfo->cellId - coreNoWorker -
//                                  1]->appendTuple(tuple);
//            }
//         }
//         tuple->DeleteIfAllowed();
//         ++count;
//      }
   }
   bufferR.clear();
   for (size_t i = 0; i < coreNoWorker; ++i) {
      partBufferR[i]->enqueue(nullptr);
   }

   cout << "Stream S" << endl;
   // grid partitioning S
   count = 0;
   Tuple* tupleS;
   while ((tupleS = streamS.request())) {
      Rect bbox = ((StandardSpatialAttribute<2>*)
              tupleS->GetAttribute(joinAttr.second))->BoundingBox();
      for (CellInfo* cellInfo : cellInfoVec) {
         if ((cellInfo->cell)->Intersects(bbox)) {
            tupleS->IncReference();
            partBufferS[(cellInfo->cellId - 1)]->enqueue(tupleS);
         }
      }
      tupleS->DeleteIfAllowed();
      ++count;
   }

   // overflow

//   for (CellInfo* info : cellInfoVec) {
//      delete info;
//   }
//   cellInfoVec.clear();
//   delete irrGrid2d;

   for (size_t i = 0; i < coreNoWorker; ++i) {
      partBufferS[i]->enqueue(nullptr);
   }

   ttR->DeleteIfAllowed();
   cout << "Schedule Ready" << endl;
//   std::unique_lock<std::mutex> lock(tbufferfull_);
//   tbufferfull.wait(lock, [&] { return tbufferfullDone; });
   //while (tupleBuffer->empty()) {}
   cout << "tupbuffer init" << endl;
}


ListExpr op_spatialJoin::spatialJoinTM(ListExpr args) {

   // mthreadedHybridJoin has 4 arguments
   // 1: StreamR of Tuple with spatial Attr
   // 2: StreamS of Tuple with spatial Attr
   // 3: Attr R
   // 4: Attr S
   // 5: optional define extend of grid

   if (MThreadedSingleton::getCoresToUse() < 3) {
      return listutils::typeError(" only works with >= 3 threads ");
   }

   cout << "args: " << nl->ToString(args);

   string err = "stream(tuple) x stream(tuple) x attr1 x "
                "attr2 [ x real] expected";
   if (!nl->HasLength(args, 4) && !nl->HasLength(args, 5)) {
      return listutils::typeError(err);
   }
   ListExpr stream1 = nl->First(args);
   ListExpr stream2 = nl->Second(args);
   ListExpr attr1 = nl->Third(args);
   ListExpr attr2 = nl->Fourth(args);

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
   ListExpr optList = nl->Fifth(args);
   if (!nl->IsEmpty(optList) && !CcReal::checkType(nl->First(optList)) &&
       !CcInt::checkType(nl->First(optList))) {
      return
              listutils::typeError(
                      err + " (optional arg is not a real or int)");
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

   cout << nl->ToString(indexList) << endl;

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

int op_spatialJoin::spatialJoinVM(Word* args, Word &result, int message,
                                  Word &local, Supplier s) {
   //cout << "MemOp: " << qp->GetMemorySize(s) *  1024 * 1024 << endl;
   //cout << "VM" << endl;
   //read append structure
   std::pair<size_t, size_t> attr;
   CcInt* attrR = static_cast<CcInt*>(args[5].addr);
   CcInt* attrS = static_cast<CcInt*>(args[6].addr);
   attr = make_pair(attrR->GetIntval(), attrS->GetIntval());
   //cout << attr.first << "##" << attr.second << endl;
//   CcReal* resizeBox = static_cast<CcReal*>(args[4].addr);
//   cout << resizeBox->GetRealval();

   // create result type
   ListExpr resultType = qp->GetNumType(s);

   spatialJoinLI* li = (spatialJoinLI*) local.addr;

   switch (message) {

      case OPEN :
         if (li) {
            delete li;
         }

         local.addr = new spatialJoinLI(args[0], args[1],
                                        attr,
                                        qp->GetMemorySize(s) *
                                        1024 * 1024,
                                        resultType);
         return 0;
      case REQUEST:
         result.addr = li ? li->getNext() : 0;
         return result.addr ? YIELD : CANCEL;
      case CLOSE:
         cout << "CLOSE" << endl;
         if (li) {
            delete li;
            local.addr = 0;
         }
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