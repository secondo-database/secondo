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



1 Basic Operators Header File

*/
#pragma once

#include <include/Stream.h>
#include <Algebras/MThreaded/MThreadedAux.h>
#include <jmorecfg.h>
#include "Operator.h"
#include "vector"
#include "thread"
#include "condition_variable"
#include "../MThreadedAlgebra.h"
#include <boost/circular_buffer.hpp>
#include <utility>

namespace mthreaded {

constexpr size_t BUFFERSIZE = 10;

/*
1.1 The MThreaded MThreadedMergeSort Operator
stream x attr x const (desc, incr) -> stream

*/

enum TupleEmpty {
   both, first, second
};

class CompareByVector {
   private:
   const std::vector<std::pair<int, bool>> sortAttr;

   public:
   // Constructor: Set compare Vector
   explicit CompareByVector(const std::vector<std::pair<int, bool>> _sortAttr)
           : sortAttr(_sortAttr) {}

   ~CompareByVector() {}

   // Compare 2 tuples and return true of 1>=2
   bool compTuple(const Tuple* a, const Tuple* b) const;

   // Debug: first attr index
   int firstAttr() const;
};

class TournamentTree {
   private:
   struct node {
      Tuple* tuple;
      size_t leave_small;
      size_t leave_large;
      bool active;

      node(Tuple* _tuple, size_t _leave_small, size_t _leave_large,
           bool _active)
              : tuple(_tuple),
                leave_small(_leave_small), leave_large(_leave_large),
                active(_active) {}
   };

   bool treeComplete;
   size_t countInactive;
   size_t maxMem;
   std::vector<node> tree;
   std::shared_ptr<CompareByVector> compareClass;

   static constexpr size_t memInTree =
           2 * sizeof(void*) + 4 * sizeof(int) + 2 * sizeof(bool);

   // recursive insert tuple in complete tree
   void exchange(Tuple* tuple, const size_t pos, const bool active);

   public:
   // constructor
   TournamentTree(
           std::shared_ptr<CompareByVector> _compareClass,
           size_t _maxMem);

   ~TournamentTree() { tree.clear(); }

   // fill leaves
   void fillLeaves(Tuple* tuple);

   // build tree from leaves
   void buildTree();

   // insert new tuple in tree and pull
   Tuple* replace(Tuple* tuple);

   // make all nodes active
   void makeActive();

   // test if active
   bool isActive() const;

   bool isEmpty() const;

   // test if next tuple fits in memory
   bool testMemSizeFill(const Tuple* tuple) ;

   // test if next tuple fits in memory
   bool testMemSizeExchange(Tuple* tuple);

   // DEBUG
   void showTree() const;
};

class MergeFeeder {
   private:
   std::shared_ptr<CompareByVector> compare;
   std::shared_ptr<Buffer> buf1;
   std::shared_ptr<Buffer> buf2;
   std::shared_ptr<SafeQueue<Tuple*>> mergeBuffer;
   //size_t n;
   TupleEmpty runEmpty;

   public:
   // Constructor: 2 incoming Buffer
   MergeFeeder(std::shared_ptr<Buffer> _buf1,
               std::shared_ptr<Buffer> _buf2,
               std::shared_ptr<CompareByVector> _compare,
               std::shared_ptr<SafeQueue<Tuple*>> _mergeBuffer);

   ~MergeFeeder() {}

   // Thread
   void operator()();
};

class NoMergeFeeder {
   private:
   std::shared_ptr<Buffer> buf;
   //std::shared_ptr<boost::circular_buffer<Tuple*>> tupleBufferOut;
   //size_t n;
   std::shared_ptr<SafeQueue<Tuple*>> mergeBuffer;

   public:
   // Constructor: 2 incoming Buffer
   NoMergeFeeder(std::shared_ptr<Buffer> _buf,
                 std::shared_ptr<SafeQueue<Tuple*>> _mergeBuffer);

   ~NoMergeFeeder() {}

   // Thread
   void operator()();
};

class MergePipeline {
   private:
   std::shared_ptr<CompareByVector> compare;
   std::shared_ptr<SafeQueue<Tuple*>> mergeBuffer_f1;
   std::shared_ptr<SafeQueue<Tuple*>> mergeBuffer_f2;
   std::shared_ptr<SafeQueue<Tuple*>> mergeBuffer;
   //size_t feederNo;
   //size_t n;

   public:
   MergePipeline(
           std::shared_ptr<CompareByVector> _compare,
           std::shared_ptr<SafeQueue<Tuple*>> _mergeBuffer_f1,
           std::shared_ptr<SafeQueue<Tuple*>> _mergeBuffer_f2,
           std::shared_ptr<SafeQueue<Tuple*>> _mergeBuffer);

   ~MergePipeline() {}

   // Thread
   void operator()();
};

class Suboptimal {
   private:
   size_t maxMem;
   std::vector<Tuple*>::iterator tupleBuffer;
   TupleType* tt;
   std::shared_ptr<CompareByVector> compare;
   std::vector<std::shared_ptr<Buffer>> runs1;
   std::vector<std::shared_ptr<Buffer>> runs2;
   size_t threadNumber;
   std::shared_ptr<std::vector<std::shared_ptr<Buffer>>> bufferTransfer;

   public:
   explicit Suboptimal(
           size_t _maxMem,
           std::vector<Tuple*>::iterator _tupleBuffer,
           std::shared_ptr<CompareByVector> _compare,
           TupleType* _tt,
           size_t _threadNumber,
           std::shared_ptr<std::vector<std::shared_ptr<Buffer>>>
                   _bufferTransfer);

   ~Suboptimal();

   // Thread
   void operator()();

   // Replacement Selection Sort
   void replacementSelectionSort(std::shared_ptr<TournamentTree> sortTree);

   std::shared_ptr<Buffer> merge(
           std::shared_ptr<Buffer> run1, std::shared_ptr<Buffer> run2);
};


class mergeSortLI {
   private:
   Stream<Tuple> stream;
   const std::vector<std::pair<int, bool>> sortAttr;
   std::shared_ptr<std::vector<std::shared_ptr<Buffer>>> mergeFn;
   std::vector<Tuple*> tupleBuffer;
   size_t lastWorker;
   TupleType* tt;
   std::vector<std::thread> sortThreads;
   std::shared_ptr<CompareByVector> compareLI;
   Tuple* tupleNext1;
   Tuple* tupleNext2;
   TupleEmpty tupleEmpty;
   std::shared_ptr<SafeQueue<Tuple*>> tupleBufferIn1;
   std::shared_ptr<SafeQueue<Tuple*>> tupleBufferIn2;
   //std::vector<std::shared_ptr<Buffer>> bufferTransfer;
   std::vector<std::shared_ptr<SafeQueue<Tuple*>>> mergeBuffer;

   const size_t maxMem;
   size_t coreNo;
   size_t coreNoWorker;
   const size_t cores = MThreadedSingleton::getCoresToUse();
   bool streamEmpty;

   size_t count = 0;

   public:
   //Constructor
   mergeSortLI(
           Word _stream,
           const std::vector<std::pair<int, bool>> _sortAttr,
           const size_t _maxMem);


   //Destructor
   ~mergeSortLI();

   //Output
   Tuple* getNext();

   private:
   //Scheduler
   void DistributorCollector();
};


class op_mergeSort {

   static ListExpr mergeSortTM(ListExpr args);

   static int mergeSortVM(Word* args, Word &result, int message,
                          Word &local, Supplier s);

   std::string getOperatorSpec();

   public:

   explicit op_mergeSort() = default;

   ~op_mergeSort() = default;

   std::shared_ptr<Operator> getOperator();

};


} // end of namespace mthreaded

