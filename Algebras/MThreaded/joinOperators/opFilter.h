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
#include <Algebras/MThreaded/MThreadedAux.h>
#include <jmorecfg.h>
#include "Operator.h"
#include "vector"
#include "thread"
#include "condition_variable"
#include "../MThreadedAlgebra.h"
#include <utility>


namespace mthreaded {

class DistributWorker {
   private:
   Stream<Tuple> stream;
   std::shared_ptr<std::vector<std::shared_ptr<SafeQueue<Tuple*>>>> buffer;
   size_t coreNoWorker;

   public:
   DistributWorker(
           Stream<Tuple> _stream,
           std::shared_ptr<std::vector<std::shared_ptr<SafeQueue<Tuple*>>>>
           _buffer,
           size_t _coreNoWorker);

   ~DistributWorker();

   // Thread
   void operator()();
};


class RefinementWorker {
   private:
   size_t static constexpr DIM = 2;
   size_t coreNoWorker;
   size_t streamInNo;
   std::shared_ptr<SafeQueue<Tuple*>> tupleBuffer;
   std::shared_ptr<SafeQueue<Tuple*>> partBuffer;
   //std::pair<size_t, size_t> joinAttr;
   ListExpr funList;
   OpTree funct;


   public:
   RefinementWorker(
           size_t _coreNoWorker,
           size_t _streamInNo,
           std::shared_ptr<SafeQueue<Tuple*>> _tupleBuffer,
           std::shared_ptr<SafeQueue<Tuple*>> _partBuffer,
           ListExpr _funList);

   RefinementWorker(
           size_t _coreNoWorker,
           size_t _streamInNo,
           std::shared_ptr<SafeQueue<Tuple*>> _tupleBuffer,
           std::shared_ptr<SafeQueue<Tuple*>> _partBuffer,
           OpTree _fun);

   ~RefinementWorker();

   // Thread
   void operator()();

   private:
   void refineNewQP();

   void refineQP();
};


class refinementLI {
   private:

   //std::vector<Word> fun;
   Word* args;
   Stream<Tuple> stream;
   //std::pair<size_t, size_t> joinAttr;
   std::vector<std::thread> filterThreads;
   //std::thread distributor;
   size_t coreNo;
   size_t coreNoWorker;
   const size_t cores = MThreadedSingleton::getCoresToUse();
   std::shared_ptr<std::vector<std::shared_ptr<SafeQueue<Tuple*>>>> buffer;
   ListExpr funList;
   //extrel2::TupleBuffer2Iterator* tb2Iter;

   public:
   //Constructor
   refinementLI(Word* _args);


   //Destructor
   ~refinementLI();

   //Output
   Tuple* getNext();

   private:
   //Scheduler
   void Scheduler();
};


class op_refinement {
   static ListExpr refinementTM(ListExpr args);

   static int refinementVM(Word* args, Word &result, int message,
                           Word &local, Supplier s);

   std::string getOperatorSpec();

   public:
   explicit op_refinement() = default;

   ~op_refinement() = default;

   std::shared_ptr<Operator> getOperator();
};


} // end of namespace mthreaded

