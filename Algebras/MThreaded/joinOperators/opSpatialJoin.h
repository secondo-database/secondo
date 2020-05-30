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



1 Hybrid Hash Join

*/
#pragma once

#include <include/Stream.h>
#include <include/MMRTree.h>
#include <Algebras/MThreaded/MThreadedAux.h>
#include <jmorecfg.h>
#include "Operator.h"
#include "vector"
#include "thread"
#include "condition_variable"
#include "../MThreadedAlgebra.h"
#include <boost/circular_buffer.hpp>
#include <utility>
#include "MMRTree.h"
#include "Algebras/MMRTree/TupleStore1.h"
#include "Algebras/SPart/IrregularGrid2D.h"


namespace mthreaded {


class CandidateWorker {
   private:
   size_t static constexpr DIM = 2;
   size_t maxMem;
   size_t* globalMem;
   size_t coreNoWorker;
   size_t streamInNo;
   std::shared_ptr<SafeQueue<Tuple*>> tupleBuffer;
   std::shared_ptr<SafeQueue<Tuple*>> partBufferR;
   std::shared_ptr<SafeQueue<Tuple*>> partBufferS;
   std::pair<size_t, size_t> joinAttr;
   Word fun;
   TupleType* resultTupleType;
   const Rect* gridcell;
   std::shared_ptr<TupleStore1> buffer;
   //std::shared_ptr<mmrtree::RtreeT<2, TupleId>> rtree;
   std::shared_ptr<mmrtree::RtreeT<DIM, TupleId>> rtreeR;
   //std::shared_ptr<mmrtree::RtreeT<DIM, TupleId>> rtreeS;
   //TupleStore1 bufferR;
   TupleType* ttR;
   std::vector<Tuple*> bufferRMem;
   size_t countInMem;
   //TupleStore1 bufferS;

   public:
   CandidateWorker(
           size_t _maxMem, size_t* _globalMem, size_t _coreNoWorker,
           size_t _streamInNo,
           std::shared_ptr<SafeQueue<Tuple*>> _tupleBuffer,
           std::shared_ptr<SafeQueue<Tuple*>> _partBufferR,
           std::shared_ptr<SafeQueue<Tuple*>> _partBufferS,
           std::pair<size_t, size_t> _joinAttr,
           TupleType* _resultTupleType, const Rect* _gridcell);

   ~CandidateWorker();

   // Thread
   void operator()();

   private:
   void quickSort(std::vector<Tuple*> &A, size_t p, size_t q);

   size_t partition(std::vector<Tuple*> &A, size_t p, size_t q);

   size_t topright(Rect* r1) const;

   inline bool reportTopright(size_t r1, size_t r2) const;

   inline void calcMem(Tuple* tuple, size_t* globalMem);

   void
   calcRtree(Tuple* tuple, TupleId id, size_t* globalMem,
             std::shared_ptr<Buffer> overflowBufferR,
             bool &overflowR);

   void calcResult(Tuple* tuple);

   size_t
   calcIterations(const size_t countOverflow, const size_t tupleSize) const;
};


class spatialJoinLI {
   private:
   Stream<Tuple> streamR;
   Stream<Tuple> streamS;
   std::pair<size_t, size_t> joinAttr;
   std::vector<std::thread> joinThreads;
   size_t maxMem;
   size_t coreNo;
   size_t coreNoWorker;
   TupleType* resultTupleType;
   //std::vector<CellInfo*> cellInfoVec;
   const size_t cores = MThreadedSingleton::getCoresToUse();
   std::shared_ptr<SafeQueue<Tuple*>> tupleBuffer;
   std::vector<std::shared_ptr<SafeQueue<Tuple*>>> partBufferR;
   std::vector<std::shared_ptr<SafeQueue<Tuple*>>> partBufferS;
   size_t bboxsample = 100;
   constexpr static size_t BBOXSAMPLESTEPS = 10;
   size_t globalMem;
   IrregularGrid2D* irrGrid2d;
   std::vector<CellInfo*> cellInfoVec;

   public:
   //Constructor
   spatialJoinLI(Word _streamR, Word _streamS,
                 std::pair<size_t, size_t> _joinAttr, size_t _maxMem,
                 ListExpr resultType);


   //Destructor
   ~spatialJoinLI();

   //Output
   Tuple* getNext();

   private:
   //Scheduler
   void Scheduler();
};


class op_spatialJoin {

   static ListExpr spatialJoinTM(ListExpr args);

   static int spatialJoinVM(Word* args, Word &result, int message,
                            Word &local, Supplier s);

   std::string getOperatorSpec();

   public:

   explicit op_spatialJoin() = default;

   ~op_spatialJoin() = default;

   std::shared_ptr<Operator> getOperator();

};


} // end of namespace mthreaded

