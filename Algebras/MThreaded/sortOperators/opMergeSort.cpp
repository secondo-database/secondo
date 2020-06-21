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



1 Implementation of the MergeSort Operator

These Operators are:

  * mthreadedMergeSort

*/
#include <thread>
#include "opMergeSort.h"
//#include "../MThreadedAux.h"

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
#include <climits>


using namespace mthreaded;
using namespace std;

extern NestedList* nl;
extern QueryProcessor* qp;

namespace mergeSortGlobal {
mutex mutexPartition_;
mutex mutexPartitionRemoved_;
mutex mutexStartThreads_;
mutex mutexCompare;
condition_variable condVarData;
condition_variable condVarDataRemoved;
condition_variable conVarStartThreads;

bool dataReady;
bool dataRemoved;
bool streamStop;
bool startThreads;
size_t tupleRemoved = 0;

//vector<std::shared_ptr<Buffer>> bufferTransfer;
//vector<shared_ptr<SafeQueue<Tuple*>>> mergeBuffer;
}

using namespace mergeSortGlobal;

// Function object for threads

Suboptimal::Suboptimal(
        size_t _maxMem,
        std::vector<Tuple*>::iterator _tupleBuffer,
        std::shared_ptr<CompareByVector> _compare,
        TupleType* _tt,
        size_t _threadNumber,
        std::shared_ptr<std::vector<std::shared_ptr<Buffer>>> _bufferTransfer) :
        maxMem(_maxMem),
        tupleBuffer(_tupleBuffer),
        tt(_tt),
        threadNumber(_threadNumber),
        bufferTransfer(_bufferTransfer) {
   compare = std::move(_compare);
   cout << "maxMem" << maxMem << "thread: " << threadNumber << endl;
}

Suboptimal::~Suboptimal() {
}

void Suboptimal::operator()() {

   auto sortTree = make_shared<TournamentTree>(compare, maxMem);
   // replacement selection
   replacementSelectionSort(sortTree);

   // merge bis ein run
   while (runs1.size() > 1) {
      for (const auto &buffer: runs2) {
         buffer->closeWrite();
      }
      runs2.clear();
      const size_t lengthRun2 = ceil(runs1.size() / 2.0);
      runs2.reserve(lengthRun2);
      const size_t lengthEven = floor(runs1.size() / 2.0);
      for (size_t i = 0; i < lengthEven; ++i) {
         runs2.emplace_back(merge(runs1[i * 2], runs1[i * 2 + 1]));
      }
      if (lengthRun2 != lengthEven) {
         runs2.emplace_back(runs1.back());
      }
      runs1.swap(runs2);
   }
   runs1.back()->closeWrite();
   {
      std::lock_guard<std::mutex> lck(mutexPartition_);
      bufferTransfer->operator[](threadNumber) = runs1.back();
   }
}

void Suboptimal::replacementSelectionSort(shared_ptr<TournamentTree> sortTree) {
   // process tuple from stream
   size_t count = 0;
   while (true) {
      Tuple* tuple;
      {
         std::unique_lock<std::mutex> lck(mutexPartition_);
         condVarData.wait(lck, [] { return dataReady; });
         if (!streamStop) { dataReady = false; }
         tuple = *tupleBuffer;
         if (tuple == nullptr) {
            break;
         }
      }

      ++count;
      sortTree->fillLeaves(tuple);

      {
         std::lock_guard<std::mutex> lck(mutexPartitionRemoved_);
         dataRemoved = true;
         tupleRemoved = threadNumber;
         *tupleBuffer = nullptr;
         condVarDataRemoved.notify_all();
      }

      // test memory
      if (!sortTree->testMemSizeFill(tuple)) {
         break;
      }
   }
   //cout << "treesize" << count << endl;
   sortTree->buildTree();

   count = 0;
   if (!streamStop) {
      runs1.emplace_back(make_shared<FileBuffer>(tt));
   } else {
      runs1.emplace_back(make_shared<MemoryBuffer>(tt));
   }

   if (!streamStop) {
      while (true) {
         cout << "notinMem" << endl;
         Tuple* tuple;
         {
            std::unique_lock<std::mutex> lck(mutexPartition_);
            condVarData.wait(lck, [] { return dataReady; });
            if (!streamStop) { dataReady = false; }
            tuple = *tupleBuffer;
            if (tuple == nullptr) {
               break;
            }
         }
         tuple = sortTree->replace(tuple);
         ++count;
         runs1.back()->appendTuple(tuple);

         // if root inactive start new run
         if (!sortTree->isActive()) {
            count = 0;
            sortTree->makeActive();
            runs1.back()->closeWrite();
            runs1.emplace_back(make_shared<FileBuffer>(tt));
         }

         {
            std::lock_guard<std::mutex> lck(mutexPartitionRemoved_);
            dataRemoved = true;
            tupleRemoved = threadNumber;
            *tupleBuffer = nullptr;
            condVarDataRemoved.notify_all();
         }
      }
   }

   //cout << "treesize>1" << count << endl;

   // empty last tree
   count = 0;
   while (!sortTree->isEmpty()) {
      Tuple* tuple = sortTree->replace(nullptr);
      ++count;
      runs1.back()->appendTuple(tuple);
   }
   runs1.back()->closeWrite();
}

shared_ptr<Buffer> Suboptimal::merge(
        shared_ptr<Buffer> run1, shared_ptr<Buffer> run2) {
   shared_ptr<FileBuffer> mergeBuffer = make_shared<FileBuffer>(tt);
   run1->openRead();
   run2->openRead();
   Tuple* run1Tuple = nullptr;
   Tuple* run2Tuple = nullptr;
   TupleEmpty tupleEmpty = both;
   while (true) {
      if (tupleEmpty == first || tupleEmpty == both) {
         run1Tuple = run1->readTuple();
         //cout << run1Tuple->GetNumOfRefs() << "##";
      }
      if (tupleEmpty == second || tupleEmpty == both) {
         run2Tuple = run2->readTuple();
         //cout << run2Tuple->GetNumOfRefs() << "##";
      }
      if (run1Tuple == nullptr || run2Tuple == nullptr) {
         break;
      }
      if (compare->compTuple(run1Tuple, run2Tuple)) {
         mergeBuffer->appendTuple(run1Tuple);
         tupleEmpty = first;
      } else {
         mergeBuffer->appendTuple(run2Tuple);
         tupleEmpty = second;
      }
   }
   while (run1Tuple != nullptr) {
      mergeBuffer->appendTuple(run1Tuple);
      run1Tuple = run1->readTuple();
   }
   while (run2Tuple != nullptr) {
      mergeBuffer->appendTuple(run2Tuple);
      run2Tuple = run2->readTuple();
   }
   mergeBuffer->closeWrite();
   return mergeBuffer;
}

MergeFeeder::MergeFeeder(std::shared_ptr<Buffer> _buf1,
                         std::shared_ptr<Buffer> _buf2,
                         std::shared_ptr<CompareByVector> _compare,
                         std::shared_ptr<SafeQueue<Tuple*>> _mergeBuffer)
        : mergeBuffer(_mergeBuffer) {
   buf1 = move(_buf1);
   buf2 = move(_buf2);
   compare = move(_compare);
   runEmpty = second;
};

void MergeFeeder::operator()() {
   buf1->openRead();
   buf2->openRead();
   Tuple* run1Tuple = buf1->readTuple();
   Tuple* run2Tuple = nullptr;
   size_t count = 0;
   {
      std::unique_lock<std::mutex> lock(mutexStartThreads_);
      conVarStartThreads.wait(lock, [&] { return startThreads; });
   }
   while (true) {
      if (runEmpty == second) {
         run2Tuple = buf2->readTuple();
      } else {
         run1Tuple = buf1->readTuple();
      }
      if (run1Tuple == nullptr || run2Tuple == nullptr) {
         break;
      }
      if (compare->compTuple(run1Tuple, run2Tuple)) {
         {
            if (run1Tuple->GetNumOfRefs() == 2) {
               run1Tuple->DeleteIfAllowed();
            }
            mergeBuffer->enqueue(run1Tuple);
            runEmpty = first;
         }
      } else {
         {
            if (run2Tuple->GetNumOfRefs() == 2) {
               run2Tuple->DeleteIfAllowed();
            }
            mergeBuffer->enqueue(run2Tuple);
            runEmpty = second;
         }
      }
      ++count;
   }
   if (run2Tuple == nullptr) {
      while (run1Tuple != nullptr) {
         {
            if (run1Tuple->GetNumOfRefs() == 2) {
               run1Tuple->DeleteIfAllowed();
            }
            mergeBuffer->enqueue(run1Tuple);
         }
         run1Tuple = buf1->readTuple();
         ++count;
      }
   } else {
      while (run2Tuple != nullptr) {
         {
            if (run2Tuple->GetNumOfRefs() == 2) {
               run2Tuple->DeleteIfAllowed();
            }
            mergeBuffer->enqueue(run2Tuple);
         }
         run2Tuple = buf2->readTuple();
         ++count;
      }
   }
   mergeBuffer->enqueue(nullptr);
}

NoMergeFeeder::NoMergeFeeder(std::shared_ptr<Buffer> _buf,
                             std::shared_ptr<SafeQueue<Tuple*>> _mergeBuffer)
        : mergeBuffer(_mergeBuffer) {
   buf = move(_buf);
};

void NoMergeFeeder::operator()() {

   buf->openRead();
   Tuple* runTuple = buf->readTuple();
   {
      std::unique_lock<std::mutex> lock(mutexStartThreads_);
      conVarStartThreads.wait(lock, [&] { return startThreads; });
   }
   size_t count = 0;
   while (runTuple != nullptr) {
      {
         if (runTuple->GetNumOfRefs() == 2) {
            runTuple->DeleteIfAllowed();
         }
         mergeBuffer->enqueue(runTuple);
      }
      runTuple = buf->readTuple();
      ++count;
   }
   {
      mergeBuffer->enqueue(nullptr);
   }
}

MergePipeline::MergePipeline(
        std::shared_ptr<CompareByVector> _compare,
        std::shared_ptr<SafeQueue<Tuple*>> _mergeBuffer_f1,
        std::shared_ptr<SafeQueue<Tuple*>> _mergeBuffer_f2,
        std::shared_ptr<SafeQueue<Tuple*>> _mergeBuffer)
        : mergeBuffer_f1(_mergeBuffer_f1),
          mergeBuffer_f2(_mergeBuffer_f2),
          mergeBuffer(_mergeBuffer) {
   compare = std::move(_compare);
}

void MergePipeline::operator()() {
   {
      std::unique_lock<std::mutex> lock(mutexStartThreads_);
      conVarStartThreads.wait(lock, [&] { return startThreads; });
   }
   Tuple* tuple1 = nullptr;
   Tuple* tuple2 = nullptr;
   TupleEmpty tupleEmpty = both;
   while (true) {
      if (tupleEmpty == first || tupleEmpty == both) {
         tuple1 = mergeBuffer_f1->dequeue();
      }

      if (tupleEmpty == second || tupleEmpty == both) {
         tuple2 = mergeBuffer_f2->dequeue();
      }
      if (tuple1 != nullptr && tuple2 != nullptr) {
         // compare
         if (compare->compTuple(tuple1, tuple2)) {
            mergeBuffer->enqueue(tuple1);
            tupleEmpty = first;
         } else {
            mergeBuffer->enqueue(tuple2);
            tupleEmpty = second;
         }
      } else if (tuple1 == nullptr && tuple2 == nullptr) {
         break;
      } else if (tuple1 == nullptr) {
         mergeBuffer->enqueue(tuple2);
         tupleEmpty = second;
      } else {
         mergeBuffer->enqueue(tuple1);
         tupleEmpty = first;
      }
   }

   {
      mergeBuffer->enqueue(nullptr);
   }
}

//methods of the Local Info Class

mergeSortLI::mergeSortLI(
        Word _stream,
        const std::vector<std::pair<int, bool>> _sortAttr,
        const size_t _maxMem)
        : stream(_stream), sortAttr(std::move(_sortAttr)), maxMem(_maxMem) {
   coreNo = MThreadedSingleton::getCoresToUse();
   coreNoWorker = coreNo - 1;
   compareLI = std::make_shared<CompareByVector>(sortAttr);
   //mergeBuffer.clear();
   dataReady = false;
   dataRemoved = false;
   streamStop = false;
   startThreads = false;
   stream.open();
   DistributorCollector();
}

mergeSortLI::~mergeSortLI() {
   //mergeBuffer.clear();
   //bufferTransfer.clear();
   tupleBuffer.clear();
   tt->DeleteIfAllowed();
}

Tuple* mergeSortLI::getNext() {
   Tuple* res;
   if (tupleEmpty == first || tupleEmpty == both) {
      ++count;
      tupleNext1 = tupleBufferIn1->dequeue();
   }
   if (tupleEmpty == second || tupleEmpty == both) {
      ++count;
      tupleNext2 = tupleBufferIn2->dequeue();
   }
   // compare
   if (tupleNext1 != nullptr && tupleNext2 != nullptr) {
      if (compareLI->compTuple(tupleNext1, tupleNext2)) {
         res = tupleNext1;
         tupleEmpty = first;
      } else {
         res = tupleNext2;
         tupleEmpty = second;
      }
      if (res->GetNumOfRefs() == 2) cout << "##";
      return res;
   } else if (tupleNext1 == nullptr && tupleNext2 == nullptr) {
      return 0;
   } else {
      if (tupleNext1 == nullptr) {
         tupleEmpty = second;
         res = tupleNext2;
      } else {
         tupleEmpty = first;
         res = tupleNext1;
      }
      if (res->GetNumOfRefs() == 2) cout << "--";
      return res;
   }
   return 0;
}


void mergeSortLI::DistributorCollector() {

   // generate FileBuffer, either in memory or extern
   vector<shared_ptr<Buffer>> tempVec;
   for (size_t i = 0; i < coreNoWorker; ++i) {
      Tuple* tuple = stream.request();
      if (i == 0) {
         tt = tuple->GetTupleType();
         tt->IncReference();
      }
      tempVec.emplace_back(make_shared<MemoryBuffer>(tt));
      cout << "ini" << endl;
      tupleBuffer.emplace_back(tuple);
   }
   mergeFn = make_shared<vector<shared_ptr<Buffer>>>(move(tempVec));
   cout << "ini" << endl;
   vector<vector<Tuple*>::iterator> tupleIter;
   for (auto it = tupleBuffer.begin(); it != tupleBuffer.end(); ++it) {
      tupleIter.push_back(it);
   }

   // start threads
   streamStop = false;
   sortThreads.reserve(coreNoWorker);
   for (size_t i = 0; i < coreNoWorker; ++i) {
      auto compare = make_shared<CompareByVector>(sortAttr);
      thread tempThread(Suboptimal(
              maxMem / coreNoWorker, tupleIter[i], compare, tt, i, mergeFn));
      sortThreads.push_back(std::move(tempThread));
   }

   // divide stream to n-1 cores by round robin
   // start suboptimal as thread
   size_t countStream = coreNoWorker - 1;
   do {
      {
         std::lock_guard<std::mutex> lck(mutexPartition_);
         dataReady = true;
         condVarData.notify_one();
      }
      {
         std::unique_lock<std::mutex> lck(mutexPartitionRemoved_);
         condVarDataRemoved.wait(lck, [] { return dataRemoved; });
         dataRemoved = false;
      }
      ++countStream;
   } while ((tupleBuffer[tupleRemoved] = stream.request()));

   // all tuples of stream processed
   {
      std::lock_guard<std::mutex> lck(mutexPartition_);
      dataReady = true;
      streamStop = true;
      condVarData.notify_all();
   }

   stream.close();

   for (size_t i = 0; i < coreNoWorker; ++i) {
      sortThreads[i].join();
   }
   sortThreads.clear();
   tupleIter.clear();

   //mergeFn = move(bufferTransfer);

   // init buffer
   size_t const feedersDouble = (coreNoWorker == 2) ? 0 : coreNoWorker / 2;
   size_t const feedersSingle = (coreNoWorker == 2) ? 2 : coreNoWorker % 2;
   size_t const feeders = feedersDouble + feedersSingle;


   // start feeder threads
   size_t mergeWorkerNo = 0;
   for (size_t i = 0; i < feedersSingle; ++i) {
      mergeBuffer.push_back(make_shared<SafeQueue<Tuple*>>(mergeWorkerNo));
      thread tempThread(NoMergeFeeder((mergeFn->operator[](i)),
                                      mergeBuffer[mergeWorkerNo]));
      sortThreads.push_back(std::move(tempThread));
      sortThreads.back().detach();
      ++mergeWorkerNo;
   }


   for (size_t i = feedersSingle; i < feeders; ++i) {
      mergeBuffer.push_back(make_shared<SafeQueue<Tuple*>>(mergeWorkerNo));
      auto compare = make_shared<CompareByVector>(sortAttr);
      thread tempThread(
              MergeFeeder((mergeFn->operator[](i * 2 - feedersSingle)),
                          (mergeFn->operator[](i * 2 + 1 - feedersSingle)),
                          compare, mergeBuffer[mergeWorkerNo]));
      sortThreads.push_back(std::move(tempThread));
      sortThreads.back().detach();
      ++mergeWorkerNo;
   }

   // build pipeline
   if (coreNoWorker > 4) {
      size_t pipes = feeders / 2;
      size_t pipeWorkerNo = 0;
      do {
         for (size_t i = mergeWorkerNo; i < mergeWorkerNo + pipes; ++i) {
            mergeBuffer.push_back(make_shared<SafeQueue<Tuple*>>(i));
            auto compare = make_shared<CompareByVector>(sortAttr);
            thread tempThread(MergePipeline(
                    compare, mergeBuffer[pipeWorkerNo],
                    mergeBuffer[pipeWorkerNo + 1],
                    mergeBuffer[i]));
            sortThreads.push_back(std::move(tempThread));
            sortThreads.back().detach();
            pipeWorkerNo += 2;
         }
         mergeWorkerNo += pipes;
         pipes = ceil(pipes / 2.0);
      } while (pipes > 1);
   }

   const size_t mergeBufferNo = mergeBuffer.size();
   tupleBufferIn1 = mergeBuffer[mergeBufferNo - 1];
   tupleBufferIn2 = mergeBuffer[mergeWorkerNo - 2];
   lastWorker = mergeBufferNo - 1;
   tupleEmpty = both;

   {
      lock_guard<std::mutex> lock(mutexStartThreads_);
      startThreads = true;
      conVarStartThreads.notify_all();
   }
   cout << "started threads" << endl;
}


TournamentTree::TournamentTree(shared_ptr<CompareByVector> _compareClass,
                               size_t _maxMem)
        : maxMem(_maxMem) {
   compareClass = std::move(_compareClass);
};

void TournamentTree::fillLeaves(Tuple* tuple) {
   tree.emplace_back(tuple, ULONG_MAX, ULONG_MAX,
                     true);
}

void TournamentTree::buildTree() {
   size_t leaves = tree.size();
   if (leaves <= 2) {
      cout << "no or one leave" << endl;
      return;
   }
   size_t leavesLevel = 1;
   while (leaves > leavesLevel) {
      leavesLevel *= 2;
   }
   size_t upLeaves = (leaves - leavesLevel / 2) * 2;
   size_t nodes = 0;
   for (size_t i = 0; i < upLeaves; i += 2) {
      if (compareClass->compTuple(tree[i].tuple, tree[i + 1].tuple)) {
         tree.emplace_back(tree[i].tuple, i, i + 1, true);
      } else {
         tree.emplace_back(tree[i + 1].tuple,
                           i + 1, i, true);
      }
      nodes += 2;
   }

   upLeaves = leavesLevel / 2;

   while (upLeaves > 1) {
      for (size_t i = nodes; i < (nodes + upLeaves); i += 2) {
         if (compareClass->compTuple(tree[i].tuple, tree[i + 1].tuple)) {
            tree.emplace_back(tree[i].tuple, i, i + 1, true);
         } else {
            tree.emplace_back(tree[i + 1].tuple,
                              i + 1, i, true);
         }
      }
      nodes += upLeaves;
      upLeaves = upLeaves / 2;
   }
}

void
TournamentTree::exchange(Tuple* tuple, const size_t pos, const bool active) {
   //find leaf
   if (tree[pos].leave_small != ULONG_MAX) {
      exchange(tuple, tree[pos].leave_small, active);
   } else {
      tree[pos].tuple = tuple;
      tree[pos].active = active;
      return;
   }

   if (tree[tree[pos].leave_small].tuple == nullptr) {
      std::swap(tree[pos].leave_large, tree[pos].leave_small);
      tree[pos].tuple = tree[tree[pos].leave_small].tuple;
      tree[pos].active = tree[tree[pos].leave_small].active;
      return;
   }
   if (tree[tree[pos].leave_large].tuple == nullptr) {
      tree[pos].tuple = tree[tree[pos].leave_small].tuple;
      tree[pos].active = tree[tree[pos].leave_small].active;
      return;
   }
   if (tree[tree[pos].leave_large].active &&
       tree[tree[pos].leave_small].active) {
      if (compareClass->compTuple(tree[tree[pos].leave_large].tuple,
                                  tree[tree[pos].leave_small].tuple)) {
         tree[pos].tuple = tree[tree[pos].leave_large].tuple;
         std::swap(tree[pos].leave_large, tree[pos].leave_small);
      } else {
         tree[pos].tuple = tree[tree[pos].leave_small].tuple;
      }
   } else if (!tree[tree[pos].leave_large].active &&
              !tree[tree[pos].leave_small].active) {
      if (compareClass->compTuple(tree[tree[pos].leave_large].tuple,
                                  tree[tree[pos].leave_small].tuple)) {
         tree[pos].tuple = tree[tree[pos].leave_large].tuple;
         std::swap(tree[pos].leave_large, tree[pos].leave_small);
      } else {
         tree[pos].tuple = tree[tree[pos].leave_small].tuple;
      }
      tree[pos].active = false;
   } else if (!tree[tree[pos].leave_large].active) {
      tree[pos].tuple = tree[tree[pos].leave_small].tuple;
   } else {
      tree[pos].tuple = tree[tree[pos].leave_large].tuple;
      std::swap(tree[pos].leave_large, tree[pos].leave_small);
   }
}


Tuple* TournamentTree::replace(Tuple* tuple) {
   Tuple* tupleMin = tree.back().tuple;
   bool active = true;
   if (tuple == nullptr || compareClass->compTuple(tuple, tupleMin)) {
      active = false;
   }
   // exchange min value recursively
   size_t pos = tree.size() - 1;
   exchange(tuple, pos, active);
   return tupleMin;
}

void TournamentTree::makeActive() {
   auto it = tree.begin();
   while (it != tree.end()) {
      it->active = true;
      ++it;
   }
}

// DEBUG
void TournamentTree::showTree() const {
   auto it = tree.begin();
   size_t i = 0;
   cout << "Print Tree" << endl;
   while (it != tree.end()) {
      cout << "Pos: " << i << ", hash: " << it->tuple->HashValue()
           << ", kleiner: " << it->leave_small << ", größer: "
           << it->leave_large << endl;
      ++it;
      ++i;
   }
   cout << "Tree end" << endl;
}

bool TournamentTree::isActive() const {
   return tree.back().active;
}

bool TournamentTree::isEmpty() const {
   return (tree.back().tuple == nullptr);
}

bool TournamentTree::testMemSizeFill(const Tuple* tuple) {
   // reserve for tree and tuple
   size_t addMem = tuple->GetMemSize() + memInTree;
   if (addMem <= maxMem) {
      maxMem -= addMem;
      return true;
   }
   return false;
}

bool TournamentTree::testMemSizeExchange(Tuple* tuple) {
   size_t addMem =
           tuple->GetMemSize() + memInTree - tree[0].tuple->GetMemSize();
   if (addMem >= maxMem) {
      maxMem -= addMem;
      return true;
   }
   return false;
}

// true a<b oder a=b
bool CompareByVector::compTuple(const Tuple* a, const Tuple* b) const {
   //assert(a != nullptr && b != nullptr);
   auto it = sortAttr.begin();
   while (it != sortAttr.end()) {
      int cmpValue;
      const int pos = it->first - 1;
      {
         std::lock_guard<mutex> lock(mergeSortGlobal::mutexCompare);
         const auto* aAttr = (const Attribute*) a->GetAttribute(pos);
         const auto* bAttr = (const Attribute*) b->GetAttribute(pos);
         // -1: *this < *rhs
         cmpValue = aAttr->Compare(bAttr);
      }
      if (cmpValue != 0) {
         if (it->second) {
            return cmpValue != 1;
         } else {
            return cmpValue == 1;
         }
      }
      // the current attribute is equal
      it++;
   }
   // all attributes are equal
   return true;
}

/*
1.1 The MThreaded mthreadedMergeSort Operator

*/

ListExpr op_mergeSort::mergeSortTM(ListExpr args) {

   // mthreadedMergeSort has at least 2 arguments
   // 1: Stream of Tuple
   // 2: Sort Attributes
   // 3: Sort Directions Bool -> True Asc, False Desc
   // 4: ...

   if (MThreadedSingleton::getCoresToUse() < 3) {
      return listutils::typeError(" only works with >= 3 threads ");
   }

   const ListExpr arg1 = nl->First(nl->First(args)); //tuple-stream
   ListExpr tupleAttr = nl->Second(args);
   ListExpr tupleValues = nl->Second(tupleAttr); //values
   tupleAttr = nl->First(tupleAttr); //attribute list and decr/incr

   if (nl->ListLength(args) != 2) {
      return listutils::typeError(" has to be 2 args: stream & list ");
   }

   if (nl->ListLength(tupleAttr) <= 0 || nl->ListLength(tupleValues) <= 0 ||
       nl->ListLength(tupleAttr) != nl->ListLength(tupleValues)) {
      return listutils::typeError(" list corrupt ");
   }

   if (nl->ListLength(nl->First(tupleAttr)) != -1) {
      return listutils::typeError(" arg in [] is not single list ");
   }

   if (!Stream<Tuple>::checkType(arg1)) {
      return listutils::typeError(" first arg is not a tuple stream ");
   }

   // extract the attribute list
   ListExpr attrList = nl->Second(nl->Second(arg1));
   ListExpr type = nl->TheEmptyList();
   ListExpr indexes = nl->TheEmptyList();
   ListExpr last = nl->TheEmptyList();
   bool direction;
   string attrName = nl->SymbolValue(nl->First(tupleAttr));

   while (!nl->IsEmpty(tupleAttr)) {
      if (attrName == "typeerror") {
         return listutils::typeError(" list corrupt ");
      }
      int i = listutils::findAttribute(attrList, attrName, type);
      tupleAttr = nl->Rest(tupleAttr);
      tupleValues = nl->Rest(tupleValues);
      if (!nl->IsEmpty(tupleAttr) && CcBool::checkType(nl->First(tupleAttr))) {
         direction = nl->BoolValue(nl->First(tupleValues));
         tupleAttr = nl->Rest(tupleAttr);
         tupleValues = nl->Rest(tupleValues);
      } else {
         direction = true;
      }
      if (i > 0) {
         indexes = nl->OneElemList(nl->IntAtom(i));
         last = indexes;
         last = nl->Append(last, nl->BoolAtom(direction));
         break;
      } else {
         std::cout << "did not find attribute" << attrName << endl;
      }
      attrName = nl->SymbolValue(nl->First(tupleAttr));
   }

   while (!nl->IsEmpty(tupleAttr)) {
      attrName = nl->SymbolValue(nl->First(tupleAttr));
      if (attrName == "typeerror") {
         return listutils::typeError(" list corrupt ");
      }
      int i = listutils::findAttribute(attrList, attrName, type);
      tupleAttr = nl->Rest(tupleAttr);
      tupleValues = nl->Rest(tupleValues);
      if (!nl->IsEmpty(tupleAttr) && CcBool::checkType(nl->First(tupleAttr))) {
         direction = nl->BoolValue(nl->First(tupleValues));
         tupleAttr = nl->Rest(tupleAttr);
         tupleValues = nl->Rest(tupleValues);
      } else {
         direction = true;
      }
      if (i > 0) {
         last = nl->Append(last, nl->IntAtom(i));
         last = nl->Append(last, nl->BoolAtom(direction));
      } else {
         std::cout << "did not find attribute: " << attrName << endl;
      }
   }

   if (nl->ListLength(indexes) == 0) {
      return listutils::typeError(" did not find any attribute ");
   }

   ListExpr append = nl->OneElemList(nl->IntAtom(nl->ListLength(indexes)));
   ListExpr lastAppend = append;
   while (!nl->IsEmpty(indexes)) {
      lastAppend = nl->Append(lastAppend, nl->First(indexes));
      indexes = nl->Rest(indexes);
   }

   //DEBUG
   std::cout << "return: " << nl->ToString(append) << endl;

   return nl->ThreeElemList(
           nl->SymbolAtom(Symbols::APPEND()),
           append,
           arg1);
}

// Operator sort2 (firstArg = 1, param = false)
// args[0] : stream
// args[1] : the number of sort attributes
// args[2] : the index of the first sort attribute
// args[3] : a boolean which indicates if sortorder should
//           be asc (true) or desc (false)
// args[4] : Same as 2 but for the second sort attribute
// args[5] : Same as 3 but for the second sort attribute
// ....
int op_mergeSort::mergeSortVM(Word* args, Word &result, int message,
                              Word &local, Supplier s) {
   //read append structure
   //(attribute number, sort direction)
   const int countIndex = static_cast<CcInt*>(args[2].addr)->GetIntval();
   std::vector<std::pair<int, bool>> index(countIndex / 2);
   index.clear();
   for (int i = 3; i < 3 + countIndex; i += 2) {
      CcInt* attr = static_cast<CcInt*>(args[i].addr);
      CcBool* direct = static_cast<CcBool*>(args[i + 1].addr);
      index.emplace_back(
              std::make_pair(attr->GetIntval(), direct->GetBoolval()));
   }

   mergeSortLI* li = (mergeSortLI*) local.addr;


   switch (message) {

      case OPEN :
         if (li) {
            delete li;
         }
         local.addr = new mergeSortLI(args[0], index,
                                      qp->GetMemorySize(s) *
                                      1024 * 1024);
         return 0;
      case REQUEST:
         result.addr = li ? li->getNext() : 0;
         return result.addr ? YIELD : CANCEL;
      case CLOSE:
         if (li) {
            delete li;
            local.addr = 0;
         }
         return 0;
   }
   return 0;
}

std::string op_mergeSort::getOperatorSpec() {
   return OperatorSpec(
           " stream x (attr x bool ...) -> stream",
           " stream mThreadedMergeSort(attr desc/incr)",
           " Merge Sort using >1 cores",
           " query Orte mThreadedMergeSort(plz TRUE)"
   ).getStr();
}

std::shared_ptr<Operator> op_mergeSort::getOperator() {
   return std::make_shared<Operator>("mThreadedMergeSort",
                                     getOperatorSpec(),
                                     &op_mergeSort::mergeSortVM,
                                     Operator::SimpleSelect,
                                     &op_mergeSort::mergeSortTM);
}