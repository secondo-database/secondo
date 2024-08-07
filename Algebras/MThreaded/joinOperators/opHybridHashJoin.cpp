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

#include "opHybridHashJoin.h"
#include "Attribute.h"          // implementation of attribute types
#include "NestedList.h"         // required at many places
#include "QueryProcessor.h"     // needed for implementing value mappings
#include "Operator.h"           // for operator creation
#include "StandardTypes.h"      // provides int, real, string, bool type
#include "Symbols.h"            // predefined strings
#include "ListUtils.h"          // useful functions for nested lists
#include "LogMsg.h"             // send error messages
#include <cmath>
#include <thread>

using namespace mthreaded;
using namespace std;

extern NestedList* nl;
extern QueryProcessor* qp;

namespace hashJoinGlobal {
size_t threadsDone;
condition_variable workers;
mutex workersDone_;
bool workersDone;
}

using namespace hashJoinGlobal;

HashTablePersist::HashTablePersist(const size_t _bucketsNo,
                                   const size_t _coreNoWorker,
                                   const size_t _maxMem,
                                   TupleType* _ttR, TupleType* _ttS,
                                   const pair<size_t, size_t> _joinAttr) :
        bucketsNo(_bucketsNo), coreNoWorker(_coreNoWorker), maxMem(_maxMem),
        ttR(_ttR), ttS(_ttS), joinAttr(_joinAttr) {
   hashBucketsR.reserve(bucketsNo - 1);
   hashBucketsS.reserve(bucketsNo - 1);
   for (size_t i = 0; i < bucketsNo - 1; ++i) {
      hashBucketsR.push_back(make_shared<MemoryBuffer>(ttR));
      hashBucketsS.push_back(make_shared<MemoryBuffer>(ttS));
      sizeR.push_back(0);
      sizeS.push_back(0);
      hashBucketsOverflowS.push_back(make_shared<FileBuffer>(ttS));
   }
   freeMem = maxMem;
   setSPersist = false;
   lastMemBufferR = bucketsNo - 2;
   lastMemBufferS = bucketsNo - 2;
}

HashTablePersist::~HashTablePersist() {
   hashBucketsR.clear();
   hashBucketsS.clear();
   hashBucketsOverflowS.clear();
}

void HashTablePersist::PushR(Tuple* tuple, const size_t bucket) {
      size_t size = tuple->GetMemSize() + sizeof(void*);
      if (bucket <= lastMemBufferR) {
         freeMem -= size;
      }
      sizeR[bucket] += size;
      hashBucketsR[bucket]->appendTuple(tuple);
      // no memory
      if (freeMem > maxMem && lastMemBufferR <= bucket &&
          lastMemBufferR < bucketsNo) {
         if (!setSPersist) {
            for (size_t i = 0; i < bucketsNo - 1; ++i) {
               hashBucketsS[i] = make_shared<FileBuffer>(ttS);
            }
            setSPersist = true;
         }
         shared_ptr<FileBuffer> tempFileBuffer = make_shared<FileBuffer>(ttR);
         Tuple* tupleNext = hashBucketsR[lastMemBufferR]->readTuple();
         while (tupleNext != nullptr) {
            tempFileBuffer->appendTuple(tupleNext);
            tupleNext = hashBucketsR[lastMemBufferR]->readTuple();
         }
         hashBucketsR[lastMemBufferR] = move(tempFileBuffer);
         freeMem += sizeR[lastMemBufferR];
         --lastMemBufferR;
   }
}

void HashTablePersist::PushS(Tuple* tuple, const size_t bucket) {
   size_t size = tuple->GetMemSize() + sizeof(void*);
   if (!setSPersist && bucket <= lastMemBufferS) {
      freeMem -= size;
   }
   size_t partNo = tuple->HashValue(joinAttr.second) / coreNoWorker /
                   bucketsNo % hashMod;
   if (partNo >= overflowBucketNo[bucket]) {
      // save in overflow
      hashBucketsOverflowS[bucket]->appendTuple(tuple);
   } else {
      sizeS[bucket] += size;
      hashBucketsS[bucket]->appendTuple(tuple);
      if (!setSPersist && freeMem > maxMem && lastMemBufferS < bucketsNo) {
         shared_ptr<FileBuffer> tempFileBuffer = make_shared<FileBuffer>(ttS);
         Tuple* tupleNext = hashBucketsS[lastMemBufferS]->readTuple();
         while (tupleNext != nullptr) {
            tempFileBuffer->appendTuple(tupleNext);
            tupleNext = hashBucketsS[lastMemBufferS]->readTuple();
         }
         hashBucketsS[lastMemBufferS] = move(tempFileBuffer);
         freeMem += sizeS[lastMemBufferS];
         --lastMemBufferS;
      }
   }
}

Tuple* HashTablePersist::PullR(const size_t bucket) const {
   Tuple* tuple = hashBucketsR[bucket]->readTuple();
   return tuple;
}

Tuple* HashTablePersist::PullS(const size_t bucket) const {
   Tuple* tuple = hashBucketsS[bucket]->readTuple();
   return tuple;
}

void HashTablePersist::UseMemHashTable(size_t usedMem) {
   freeMem -= usedMem;
}

void HashTablePersist::SetHashMod(size_t hashMod) {
   this->hashMod = hashMod;
}

void HashTablePersist::CalcOverflow() {
   for (size_t i = 0; i < bucketsNo - 1; ++i) {
      if (sizeR[i] != 0) {
         overflowBucketNo.emplace_back(
                 (size_t) floor(maxMem * 0.8) / floor(sizeR[i] / hashMod));
      } else {
         overflowBucketNo.emplace_back(0);
      }
   }
}

shared_ptr<FileBuffer>
HashTablePersist::GetOverflowS(const size_t bucket) const {
   return hashBucketsOverflowS[bucket];
}

size_t HashTablePersist::GetOverflowBucketNo(const size_t bucket) const {
   return overflowBucketNo[bucket];
}

void HashTablePersist::CloseWrite() {
   for (size_t i = 0; i < bucketsNo - 1; ++i) {
      hashBucketsR[i]->closeWrite();
      hashBucketsS[i]->closeWrite();
      hashBucketsOverflowS[i]->closeWrite();
   }
}

size_t HashTablePersist::OpenRead(size_t bucket) {
   hashBucketsR[bucket]->openRead();
   hashBucketsS[bucket]->openRead();
   return overflowBucketNo[bucket];
}


// Function object for threads


HashJoinWorker::HashJoinWorker(size_t _maxMem,
                               size_t _coreNoWorker,
                               size_t _streamInNo,
                               shared_ptr<SafeQueuePersistent> _tupleBuffer,
                               shared_ptr<SafeQueue<Tuple*>> _partBufferR,
                               shared_ptr<SafeQueue<Tuple*>> _partBufferS,
                               pair<int, int> _joinAttr,
                               TupleType* _resultTupleType,
                               TupleType* _ttS) :
        maxMem(_maxMem),
        coreNoWorker(_coreNoWorker),
        streamInNo(_streamInNo),
        tupleBuffer(_tupleBuffer),
        partBufferR(_partBufferR),
        partBufferS(_partBufferS),
        joinAttr(_joinAttr),
        resultTupleType(_resultTupleType),
        ttS(_ttS) {
}

HashJoinWorker::~HashJoinWorker() {
}

void HashJoinWorker::operator()() {
   // phase 1:
   size_t countOverflow;
   size_t hashMod = phase1(countOverflow);

   if (hashMod != 0) {
      // recursive overflow bucket 0
      if (!overflowBufferR->empty()) {
         recursiveOverflow(overflowBufferR,
                           overflowBufferS, countOverflow, 1);
      }

      // phase 2:
      for (size_t pos = 0; pos < bucketNo - 1; ++pos) {
         // non-overflow
         countOverflow = phase2(hashMod, pos);
         // recursive overflow buckets >0
         if (!overflowBufferR->empty()) {
            recursiveOverflow(overflowBufferR,
                              hashTablePersist->GetOverflowS(pos),
                              countOverflow,
                              1);
         }
      }
      ttR->DeleteIfAllowed();
   }

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

size_t HashJoinWorker::phase1(size_t &countOverflow) {
   vector<vector<Tuple*>> bucketInMemR(bucketsInMem1st);
   size_t memR = maxMem;

   Tuple* tupleNextR;
   tupleNextR = partBufferR->dequeue();
   if (tupleNextR == nullptr) {
      return 0;
   }
   ttR = tupleNextR->GetTupleType();
   ttR->IncReference();

   hashTablePersist = make_shared<HashTablePersist>(bucketNo, coreNoWorker,
                                                    maxMem, ttR, ttS, joinAttr);
   size_t count = 0;
   size_t inMemBucketNo = bucketsInMem1st;
   overflowBufferR = make_shared<FileBuffer>(ttR);
   overflowBufferS = make_shared<FileBuffer>(ttS);
   size_t memROverflow = 0;
   countOverflow = 0;
   while (tupleNextR != nullptr) {
      size_t partNo =
              (tupleNextR->HashValue(joinAttr.first) / coreNoWorker) % bucketNo;
      if (partNo == 0) {
         size_t memRTupleNextR = tupleNextR->GetMemSize() + sizeof(void*);
         partNo = (tupleNextR->HashValue(joinAttr.first) / coreNoWorker /
                   bucketNo) % bucketsInMem1st;
         if (partNo < inMemBucketNo) {
            memR -= memRTupleNextR;
            hashTablePersist->UseMemHashTable(memRTupleNextR);
            bucketInMemR[partNo].push_back(tupleNextR);
         } else {
            memROverflow += memRTupleNextR;
            ++countOverflow;
            overflowBufferR->appendTuple(tupleNextR);
         }
         // overflow
         if (memR > maxMem) {
            size_t memTransfered = 0;
            for (size_t i = inMemBucketNo / 2; i < inMemBucketNo; ++i) {
               for (Tuple* tupleTransfer : bucketInMemR[i]) {
                  memTransfered += tupleTransfer->GetMemSize() + sizeof(void*);
                  ++countOverflow;
                  overflowBufferR->appendTuple(tupleTransfer);
               }
            }
            inMemBucketNo /= 2;
            if (inMemBucketNo == 0) {
               cout << "No bucket of the hash table fits in memory." << endl;
               cout << "Stop calculating this hash table." << endl;
               cout << "!!outcoming tuple stream incomplete!!" << endl;
               break;
            }
            memR -= memTransfered;
            memROverflow += memTransfered;
            hashTablePersist->UseMemHashTable(-memTransfered);
         }
      } else {
         hashTablePersist->PushR(tupleNextR, partNo - 1);
      }
      tupleNextR = partBufferR->dequeue();
      ++count;
   }
   partBufferR.reset();

   size_t hashMod = count / bucketNo;
   if (hashMod == 0) hashMod = 1;

   hashTablePersist->SetHashMod(hashMod);
   hashTablePersist->CalcOverflow();

   Tuple* tupleNextS;
   tupleNextS = partBufferS->dequeue();
   if (tupleNextS == nullptr) {
      return 0;
   }

   count = 0;
   while (tupleNextS != nullptr) {
      size_t partNo =
              (tupleNextS->HashValue(joinAttr.second) / coreNoWorker) %
              bucketNo;
      if (partNo == 0) {
         // join?
         partNo = (tupleNextS->HashValue(joinAttr.second) / coreNoWorker /
                   bucketNo) % bucketsInMem1st;
         if (partNo < inMemBucketNo) {
            if (!bucketInMemR[partNo].empty()) {
               for (auto tupleR : bucketInMemR[partNo]) {
                  if (tupleEqual(tupleR, tupleNextS)) {
                     auto* result = new Tuple(resultTupleType);
                     {
                        Concat(tupleR, tupleNextS, result);
                     }
                     tupleBuffer->enqueue(result);
                  }
               }
            }
            tupleNextS->DeleteIfAllowed();
         } else {
            overflowBufferS->appendTuple(tupleNextS);
         }
      } else {
         hashTablePersist->PushS(tupleNextS, partNo - 1);
      }
      ++count;
      tupleNextS = partBufferS->dequeue();
   }
   partBufferS.reset();
   for (size_t i = 0; i < inMemBucketNo; ++i) {
      for (auto* tupleR : bucketInMemR[i]) {
         tupleR->DeleteIfAllowed();
      }
   }
   bucketInMemR.clear();
   hashTablePersist->CloseWrite();
   overflowBufferR->closeWrite();
   overflowBufferS->closeWrite();
   return hashMod;
}

size_t HashJoinWorker::phase2(size_t hashMod, size_t pos) {
   vector<vector<Tuple*>> bucketInMemR(hashMod);
   size_t memR = maxMem;

   size_t overflow = hashTablePersist->OpenRead(pos);
   Tuple* tupleR = hashTablePersist->PullR(pos);
   size_t countOverflow = 0;
   while (tupleR != nullptr) {
      size_t partNo =
              (tupleR->HashValue(joinAttr.first) / coreNoWorker /
               bucketNo) % hashMod;
      if (partNo < overflow) {
         size_t memRTupleNextR = tupleR->GetMemSize() + sizeof(void*);
         memR -= memRTupleNextR;
         bucketInMemR[partNo].emplace_back(tupleR);
      } else {
         overflowBufferR->appendTuple(tupleR);
         ++countOverflow;
      }
      while (memR > maxMem) {
         for (Tuple* tupleTransfer : bucketInMemR[overflow - 1]) {
            memR += tupleTransfer->GetMemSize() + sizeof(void*);
            overflowBufferR->appendTuple(tupleTransfer);
        }
        --overflow;
      }
      tupleR = hashTablePersist->PullR(pos);
   }
   Tuple* tupleS = hashTablePersist->PullS(pos);
   while (tupleS != nullptr) {
      size_t partNo =
              (tupleS->HashValue(joinAttr.second) / coreNoWorker /
               bucketNo) % hashMod;
      if (!bucketInMemR[partNo].empty()) {
         for (auto tupleR : bucketInMemR[partNo]) {
            if (tupleEqual(tupleR, tupleS)) {
               auto* result = new Tuple(resultTupleType);
               {
                  Concat(tupleR, tupleS, result);
               }
               tupleBuffer->enqueue(result);
            }
         }
      }
      if (tupleS->GetNumOfRefs() == 2) {
         tupleS->DeleteIfAllowed();
      }
      tupleS->DeleteIfAllowed();
      tupleS = hashTablePersist->PullS(pos);
   }
   for (size_t i = 0; i < hashMod; ++i) {
      for (auto* tuple : bucketInMemR[i]) {
         if (tuple->GetNumOfRefs() == 2) {
            tuple->DeleteIfAllowed();
         }
         tuple->DeleteIfAllowed();
      }
   }
   bucketInMemR.clear();
   return countOverflow;
}


void HashJoinWorker::recursiveOverflow(shared_ptr<FileBuffer> overflowR,
                                       shared_ptr<FileBuffer> overflowS,
                                       size_t sizeR, size_t xMod) {
   // phase 1
   size_t inMemBucketNo = sizeR / bucketNo;
   if (inMemBucketNo == 0) {
      inMemBucketNo = 1;
   }
   size_t const hashMod = inMemBucketNo;
   vector<vector<Tuple*>> bucketInMemR(inMemBucketNo);
   size_t memR = maxMem;
   ++xMod;

   overflowR->openRead();
   Tuple* tupleNextR = overflowR->readTuple();
   overflowS->openRead();
   Tuple* tupleNextS = overflowS->readTuple();

   shared_ptr<HashTablePersist> recursiveHashTablePersist =
           make_shared<HashTablePersist>(
                   bucketNo, coreNoWorker,
                   maxMem, ttR, ttS, joinAttr);

   shared_ptr<FileBuffer> recursiveOverflowBufferR = make_shared<FileBuffer>(
           ttR);
   shared_ptr<FileBuffer> recursiveOverflowBufferS = make_shared<FileBuffer>(
           ttS);
   size_t memROverflow = 0;
   size_t countOverflow = 0;
   while (tupleNextR != nullptr) {
      size_t partNo = tupleNextR->HashValue(joinAttr.first) / coreNoWorker;
      for (size_t i = 0; i < xMod; ++i) {
         partNo /= bucketNo;
      }
      size_t partNoBucket = partNo % bucketNo;
      if (partNoBucket == 0) {
         size_t memRTupleNextR = tupleNextR->GetMemSize() + sizeof(void*);
         partNo = partNo / bucketNo % hashMod;
         if (partNo < inMemBucketNo) {
            memR -= memRTupleNextR;
            recursiveHashTablePersist->UseMemHashTable(memRTupleNextR);
            bucketInMemR[partNo].push_back(tupleNextR);
         } else {
            memROverflow += memRTupleNextR;
            ++countOverflow;
            recursiveOverflowBufferR->appendTuple(tupleNextR);
         }
         // overflow
         if (memR > maxMem) {
            size_t memTransfered = 0;
            for (size_t i = inMemBucketNo / 2; i < inMemBucketNo; ++i) {
               for (Tuple* tupleTransfer : bucketInMemR[i]) {
                  ++countOverflow;
                  memTransfered += tupleTransfer->GetMemSize() + sizeof(void*);
                  recursiveOverflowBufferR->appendTuple(tupleTransfer);
               }
            }
            inMemBucketNo /= 2;
            if (inMemBucketNo == 0) {
               cout << "Phase1" << endl;
               cout << "No bucket of the hash table fits in memory." << endl;
               cout << "Stop calculating this hash table." << endl;
               cout << "!!outcoming tuple stream incomplete!!" << endl;
               break;
            }
            memR -= memTransfered;
            memROverflow += memTransfered;
            recursiveHashTablePersist->UseMemHashTable(-memTransfered);
         }
      } else {
         recursiveHashTablePersist->PushR(tupleNextR, partNoBucket - 1);
      }
      tupleNextR = overflowR->readTuple();
   }
   recursiveHashTablePersist->SetHashMod(hashMod);
   recursiveHashTablePersist->CalcOverflow();
   while (tupleNextS != nullptr) {
      size_t partNo = tupleNextS->HashValue(joinAttr.second) / coreNoWorker;
      for (size_t i = 0; i < xMod; ++i) {
         partNo /= bucketNo;
      }
      size_t partNoBucket = partNo % bucketNo;
      if (partNoBucket == 0) {
         // join?
         partNo = partNo / bucketNo % hashMod;
         if (partNo < inMemBucketNo) {
            if (!bucketInMemR[partNo].empty()) {
               for (auto tupleR : bucketInMemR[partNo]) {
                  if (tupleEqual(tupleR, tupleNextS)) {
                     auto* result = new Tuple(resultTupleType);
                     {
                        Concat(tupleR, tupleNextS, result);
                     }
                     tupleBuffer->enqueue(result);
                  }
               }
            }
            tupleNextS->DeleteIfAllowed();
         } else {
            recursiveOverflowBufferS->appendTuple(tupleNextS);
         }
      } else {
         recursiveHashTablePersist->PushS(tupleNextS, partNoBucket - 1);
      }
      tupleNextS = overflowS->readTuple();
   }

   for (size_t i = 0; i < inMemBucketNo; ++i) {
      for (auto* tuple : bucketInMemR[i]) {
         tuple->DeleteIfAllowed();
      }
   }
   bucketInMemR.clear();
   recursiveHashTablePersist->CloseWrite();
   recursiveOverflowBufferR->closeWrite();
   recursiveOverflowBufferS->closeWrite();

   // R0 recursive
   if (!recursiveOverflowBufferR->empty()) {
      recursiveOverflow(recursiveOverflowBufferR, recursiveOverflowBufferS,
                        countOverflow, xMod);
   }

   // phase 2
   for (size_t pos = 0; pos < bucketNo - 1; ++pos) {
      recursiveOverflowBufferR = make_shared<FileBuffer>(
              ttR);
      memR = maxMem;
      vector<vector<Tuple*>> bucketInMemR(inMemBucketNo);
      size_t overflow = recursiveHashTablePersist->OpenRead(pos);
      Tuple* tupleR = recursiveHashTablePersist->PullR(pos);
      countOverflow = 0;
      while (tupleR != nullptr) {
         size_t partNo = tupleR->HashValue(joinAttr.first) / coreNoWorker;
         for (size_t i = 0; i < xMod; ++i) {
            partNo /= bucketNo;
         }
         partNo %= inMemBucketNo;
         if (partNo < overflow) {
            size_t memRTupleNextR = tupleR->GetMemSize() + sizeof(void*);
            memR -= memRTupleNextR;
            bucketInMemR[partNo].emplace_back(tupleR);
         } else {
            recursiveOverflowBufferR->appendTuple(tupleR);
            ++countOverflow;
         }
         while (memR > maxMem) {
            for (Tuple* tupleTransfer : bucketInMemR[overflow - 1]) {
               memR += tupleTransfer->GetMemSize() + sizeof(void*);
               recursiveOverflowBufferR->appendTuple(tupleTransfer);
            }
            --overflow;
         }
         tupleR = recursiveHashTablePersist->PullR(pos);
      }
      Tuple* tupleS = recursiveHashTablePersist->PullS(pos);
      while (tupleS != nullptr) {
         size_t partNo = tupleS->HashValue(joinAttr.second) / coreNoWorker;
         for (size_t i = 0; i < xMod; ++i) {
            partNo /= bucketNo;
         }
         partNo %= inMemBucketNo;
         if (!bucketInMemR[partNo].empty()) {
            for (auto tupleR : bucketInMemR[partNo]) {
               if (tupleEqual(tupleR, tupleS)) {
                  auto* result = new Tuple(resultTupleType);
                  {
                     Concat(tupleR, tupleS, result);
                  }
                  tupleBuffer->enqueue(result);
               }
            }
         }
         tupleS->DeleteIfAllowed();
         tupleS = recursiveHashTablePersist->PullS(pos);
      }
      for (size_t i = 0; i < inMemBucketNo; ++i) {
         for (auto* tuple : bucketInMemR[i]) {
            tuple->DeleteIfAllowed();
         }
      }
      bucketInMemR.clear();
      if (!recursiveOverflowBufferR->empty()) {
         recursiveOverflow(recursiveOverflowBufferR,
                           recursiveHashTablePersist->GetOverflowS(pos),
                           countOverflow, xMod);
      }
   }
}


bool HashJoinWorker::tupleEqual(Tuple* a, Tuple* b) const {
   const auto* aAttr = (const Attribute*) a->GetAttribute(joinAttr.first);
   const auto* bAttr = (const Attribute*) b->GetAttribute(joinAttr.second);
   return aAttr->Compare(bAttr) == 0;
}

//methods of the Local Info Class

hybridHashJoinLI::hybridHashJoinLI(
        Word _streamR,
        Word _streamS,
        const pair<int, int> _joinAttr,
        const size_t _maxMem,
        ListExpr resultType)
        : streamR(_streamR), streamS(_streamS),
          joinAttr(_joinAttr), maxMem(_maxMem),
          coreNo(MThreadedSingleton::getCoresToUse()),
          coreNoWorker(coreNo - 1) {
   resultTupleType = new TupleType(nl->Second(resultType));
   resultTupleType->IncReference();
   streamR.open();
   streamS.open();
   Scheduler();
}

hybridHashJoinLI::~hybridHashJoinLI() {
   partBufferR.clear();
   partBufferS.clear();
   joinThreads.clear();
   resultTupleType->DeleteIfAllowed();
   streamR.close();
   streamS.close();
  }


Tuple* hybridHashJoinLI::getNext() {
   Tuple* res;
   res = tupleBuffer->dequeue();
   if (res != nullptr) {
      if (res->GetNumOfRefs() != 1) {
         res->DeleteIfAllowed();
      }
      return res;
   }
   return 0;
}


void hybridHashJoinLI::Scheduler() {
   tupleBuffer = make_shared<SafeQueuePersistent>(maxMem / coreNoWorker,
                                                  resultTupleType);
   Tuple* tupleR = streamR.request();
   Tuple* tupleS = streamS.request();
   if (tupleS != nullptr && tupleR != nullptr) {
      TupleType* ttS = tupleS->GetTupleType();
      ttS->IncReference();
      // start threads
      joinThreads.reserve(coreNoWorker);
      partBufferR.reserve(coreNoWorker);
      partBufferS.reserve(coreNoWorker);
      threadsDone = coreNoWorker;
      for (size_t i = 0; i < coreNoWorker; ++i) {
         partBufferR.push_back(
                 make_shared<SafeQueue<Tuple*>>(i));
         partBufferS.push_back(
                 make_shared<SafeQueue<Tuple*>>(i));
         resultTupleType->IncReference();
         joinThreads.emplace_back(
                 HashJoinWorker(maxMem / coreNoWorker, coreNoWorker, i,
                                tupleBuffer,
                                partBufferR.back(),
                                partBufferS.back(),
                                joinAttr,
                                resultTupleType, ttS));
      }

      // Stream R
      do {
         size_t partNo = tupleR->HashValue(joinAttr.first) % coreNoWorker;
         partBufferR[partNo]->enqueue(tupleR);
      } while ((tupleR = streamR.request()));

      for (size_t i = 0; i < coreNoWorker; ++i) {
         partBufferR[i]->enqueue(nullptr);
      }

      do {
         size_t partNo = tupleS->HashValue(joinAttr.second) % coreNoWorker;
         partBufferS[partNo]->enqueue(tupleS);
      } while ((tupleS = streamS.request()));

      for (size_t i = 0; i < coreNoWorker; ++i) {
         partBufferS[i]->enqueue(nullptr);
      }
      for (size_t i = 0; i < coreNoWorker; ++i) {
         joinThreads[i].join();
      }
      ttS->DeleteIfAllowed();
   } else {
      tupleBuffer->enqueue(nullptr);
   }
}


/*
1.1 The MThreaded mthreadedMergeSort Operator

*/

ListExpr op_hybridHashJoin::hybridHashJoinTM(ListExpr args) {

   // mthreadedHybridJoin has 4 arguments
   // 1: StreamR of Tuple
   // 2: StreamS of Tuple
   // 3: EquiAttributeR
   // 4: EquiAttributeR

   if (MThreadedSingleton::getCoresToUse() < 3) {
      return listutils::typeError(" only works with >= 3 threads ");
   }

   string err = "stream(tuple) x stream(tuple) x attr1 x "
                "attr2 expected";
   if (!nl->HasLength(args, 4) ) {
      return listutils::typeError(err);
   }
   ListExpr stream1 = nl->First(args);
   ListExpr stream2 = nl->Second(args);
   ListExpr attr1 = nl->Third(args);
   ListExpr attr2 = nl->Fourth(args);

   if (!Stream<Tuple>::checkType(stream1)) {
      return listutils::typeError(err + " (first arg is not a tuple stream)");
   }
   if (!Stream<Tuple>::checkType(stream2)) {
      return listutils::typeError(err + " (second arg is not a tuple stream)");
   }
   if (!listutils::isSymbol(attr1)) {
      return listutils::typeError(err + "(first attrname is not valid)");
   }
   if (!listutils::isSymbol(attr2)) {
      return listutils::typeError(err + "(second attrname is not valid)");
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
      return listutils::typeError(attrname2 +
                                  " is not an attribute of the second stream");
   }

   if (!nl->Equal(attrType1, attrType2)) {
      return listutils::typeError("types of the selected attributes differ");
   }

   ListExpr resAttrList = listutils::concat(attrList1, attrList2);

   if (!listutils::isAttrList(resAttrList)) {
      return listutils::typeError("Name conflicts in attributes found");
   }

   ListExpr indexList = nl->TwoElemList(
           nl->IntAtom(index1 - 1),
           nl->IntAtom(index2 - 1));

   return nl->ThreeElemList(nl->SymbolAtom(Symbols::APPEND()),
                            indexList,
                            nl->TwoElemList(
                                    nl->SymbolAtom(Stream<Tuple>::BasicType()),
                                    nl->TwoElemList(
                                            nl->SymbolAtom(Tuple::BasicType()),
                                            resAttrList)));
}

// Operator sort2 (firstArg = 1, param = false)
// args[0] : streamR
// args[1] : streamS
// args[2] : index of join attribute R
// args[3] : index of join attribute S

int op_hybridHashJoin::hybridHashJoinVM(Word* args, Word &result, int message,
                                        Word &local, Supplier s) {


   //read append structure
   //(attribute number, sort direction)
   std::pair<int, int> attr;
   CcInt* attrR = static_cast<CcInt*>(args[4].addr);
   CcInt* attrS = static_cast<CcInt*>(args[5].addr);
   attr = make_pair(attrR->GetIntval(), attrS->GetIntval());

   // create result type
   ListExpr resultType = qp->GetNumType(s);

   hybridHashJoinLI* li = (hybridHashJoinLI*) local.addr;

   switch (message) {

      case OPEN :
         if (li) {
            delete li;
         }

         local.addr = new hybridHashJoinLI(args[0], args[1],
                                           attr,
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
         usleep(100);
         return 0;
   }
   return 0;
}

std::string op_hybridHashJoin::getOperatorSpec() {
   return OperatorSpec(
           " stream x stream x attr x attr) -> stream",
           " streamR streamS mThreadedHybridJoin(attrR attrS)",
           " hybrid hash join using >2 cores",
           " R feed {o} S feed {p} mThreadedHybridJoin[X_o, Y_p]"
   ).getStr();
}

std::shared_ptr<Operator> op_hybridHashJoin::getOperator() {
   return std::make_shared<Operator>("mThreadedHybridJoin",
                                     getOperatorSpec(),
                                     &op_hybridHashJoin::hybridHashJoinVM,
                                     Operator::SimpleSelect,
                                     &op_hybridHashJoin::hybridHashJoinTM);
}
