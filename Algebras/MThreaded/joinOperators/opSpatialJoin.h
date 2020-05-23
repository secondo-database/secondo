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


namespace mthreaded {


class CandidateWorker {
   private:
   size_t static constexpr DIM = 2;
   size_t maxMem;
   size_t coreNoWorker;
   size_t streamInNo;
   std::pair<size_t, size_t> joinAttr;
   Word fun;
   TupleType* resultTupleType;
   const Rect* gridcell;
   std::shared_ptr<mmrtree::RtreeT<DIM, TupleId>> rtreeR;
   std::shared_ptr<mmrtree::RtreeT<DIM, TupleId>> rtreeS;
   std::vector<Tuple*> bufferR;
   TupleStore1 bufferS;

   public:
   CandidateWorker(
           size_t _maxMem, size_t _coreNoWorker, size_t _streamInNo,
           std::pair<size_t, size_t> _joinAttr, Word _fun,
           TupleType* _resultTupleType, const Rect* _gridcell);

   ~CandidateWorker();

   // Thread
   void operator()();

   private:
   void quickSort(std::vector<Tuple*> &A, size_t p, size_t q);

   size_t partition(std::vector<Tuple*> &A, size_t p, size_t q);

   size_t topright(Rect* r1);

   inline bool reportTopright(size_t r1, size_t r2);
};


class spatialHashJoinLI {
   private:
   Stream<Tuple> streamR;
   Stream<Tuple> streamS;
   Word fun;
   std::pair<size_t, size_t> joinAttr;
   std::vector<std::thread> joinThreads;
   size_t maxMem;
   size_t coreNo;
   size_t coreNoWorker;
   TupleType* resultTupleType;
   const size_t cores = MThreadedSingleton::getCoresToUse();
   size_t bboxsample = 100;
   constexpr static size_t BBOXSAMPLESTEPS = 10;

   public:
   //Constructor
   spatialHashJoinLI(Word _streamR, Word _streamS, Word _fun,
                     std::pair<size_t, size_t> _joinAttr, size_t _maxMem,
                     ListExpr resultType);


   //Destructor
   ~spatialHashJoinLI();

   //Output
   Tuple* getNext();

   private:
   //Scheduler
   void Scheduler();
};


class op_spatialHashJoin {

   static ListExpr spatialHashJoinTM(ListExpr args);

   static int spatialHashJoinVM(Word* args, Word &result, int message,
                                Word &local, Supplier s);

   std::string getOperatorSpec();

   public:

   explicit op_spatialHashJoin() = default;

   ~op_spatialHashJoin() = default;

   std::shared_ptr<Operator> getOperator();

};


} // end of namespace mthreaded

