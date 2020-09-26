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
#include "Operator.h"
#include "vector"
#include "thread"
#include "condition_variable"

namespace mthreaded {

class RefinementWorker {
   private:
   size_t static constexpr DIM = 2;
   size_t coreNoWorker;
   size_t bufferSize;
   size_t streamInNo;
   std::shared_ptr<SafeQueue<Tuple*>> tupleBuffer;
   std::shared_ptr<SafeQueue<Tuple*>> partBuffer;
   QueryProcessor* qpThread;
   ListExpr funList;
   OpTree funct;
   Stream<Tuple> stream;


   public:
   RefinementWorker(
           size_t _coreNoWorker,
           //size_t _bufferSize,
           size_t _streamInNo,
           std::shared_ptr<SafeQueue<Tuple*>> _tupleBuffer,
           std::shared_ptr<SafeQueue<Tuple*>> _partBuffer,
           QueryProcessor* _qpThread,
           ListExpr _funList,
           OpTree _funct,
           Stream<Tuple> _stream);

   ~RefinementWorker();

   // Thread
   void operator()();

   private:
   void refineNewQP();

   void refineQP();
};


class refinementLI {
   private:

   Stream<Tuple> stream;
   Word funText;
   std::vector<std::thread> filterThreads;
   size_t coreNo;
   size_t coreNoWorker;
   const size_t cores = MThreadedSingleton::getCoresToUse();
   std::shared_ptr<SafeQueue<Tuple*>> tupleBuffer;
   std::shared_ptr<std::vector<std::shared_ptr<SafeQueue<Tuple*>>>> buffer;
   ListExpr funList;
   bool phaseStream;
   bool streamDone;
   size_t countWorker;
   size_t fillCounter;
   std::vector<QueryProcessor*> qpVec;
   std::vector<OpTree> funct;

   public:
   //Constructor
   refinementLI(Word _stream,  Word _funText);


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

