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
#include "opFilter.h"

#include "Attribute.h"          // implementation of attribute types
#include "NestedList.h"         // required at many places
#include "QueryProcessor.h"     // needed for implementing value mappings
#include "Operator.h"           // for operator creation
#include "StandardTypes.h"      // provides int, real, string, bool type
#include "Symbols.h"            // predefined strings
#include "ListUtils.h"          // useful functions for nested lists
#include <atomic>
#include <Algebras/FText/FTextAlgebra.h>
#include <chrono>


using namespace mthreaded;
using namespace std;

extern NestedList* nl;
extern QueryProcessor* qp;

namespace filterGlobal {
constexpr unsigned int DIMENSIONS = 2;
std::atomic<size_t> threadsDone;
bool endRefinement;
bool startStream;
std::mutex stream_;
std::mutex endThreads_;
std::condition_variable endThreadsC;
}

using namespace filterGlobal;

RefinementWorker::RefinementWorker(
        size_t _coreNoWorker,
        size_t _streamInNo,
        shared_ptr<SafeQueue<Tuple*>> _tupleBuffer,
        shared_ptr<SafeQueue<Tuple*>> _partBuffer,
        QueryProcessor* _qpThread,
        ListExpr _funList,
        OpTree _funct,
        Stream<Tuple> _stream) :
        coreNoWorker(_coreNoWorker),
        //bufferSize(_bufferSize),
        streamInNo(_streamInNo),
        tupleBuffer(_tupleBuffer), partBuffer(_partBuffer),
        qpThread(_qpThread), funList(_funList), funct(_funct),
        stream(_stream) {
}

RefinementWorker::~RefinementWorker() {
}

// Thread
void RefinementWorker::operator()() {
   endRefinement = false;
   ArgVector &arguments = *qpThread->Argument(funct);
   Tuple* tuple = nullptr;
   {
      lock_guard<std::mutex> lock(endThreads_);
      tuple = stream.request();
   }

   while (tuple != nullptr) {
      arguments[0].setAddr(tuple);
      Word funres;
      qpThread->Request(funct, funres);
      bool res = false;
      if (((Attribute*) funres.addr)->IsDefined()) {
         res = ((CcBool*) funres.addr)->GetBoolval();
      }
      if (res) {
         while (tupleBuffer->size() > 100) {
            this_thread::sleep_for(std::chrono::microseconds(5));
         };
         tupleBuffer->enqueue(tuple);
      } else {
         tuple->DeleteIfAllowed();
      }
      {
         lock_guard<std::mutex> lock(stream_);
         tuple = stream.request();
      }
   }
   --threadsDone;
   if (threadsDone == 0) {
      tupleBuffer->enqueue(nullptr);
   }
   {
      unique_lock<std::mutex> lock(endThreads_);
      endThreadsC.wait(lock, [&] { return endRefinement; });
   }
}


//Constructor
refinementLI::refinementLI(Word _stream, Word _funText) : stream(
        _stream), funText(_funText) {
   coreNo = MThreadedSingleton::getCoresToUse();
   coreNoWorker = coreNo - 1;
   tupleBuffer = make_shared<SafeQueue<Tuple*>>(0);
   stream.open();
   phaseStream = true;
   streamDone = false;
   countWorker = 0;
   fillCounter = 0;
   Scheduler();
}


//Destructor
refinementLI::~refinementLI() {
   for (shared_ptr<SafeQueue<Tuple*>> partBuffer : *buffer) {
      partBuffer.reset();
   }
   buffer->clear();
   tupleBuffer.reset();
   size_t i = 0;
   for (QueryProcessor* qpThread : qpVec) {
      qpThread->Destroy(funct[i], true);
      delete qpThread;
      ++i;
   }
   funct.clear();
   qpVec.clear();
   stream.close();
   for (size_t i = 0; i < coreNoWorker; ++i) {
      if (filterThreads[i].joinable()) {
         filterThreads[i].detach();
      }
   }
}

//Output
Tuple* refinementLI::getNext() {
   // reading stream phase
   Tuple* res;
   res = tupleBuffer->dequeue();

   if (res != nullptr) {
      return res;
   }

   endRefinement = false;
   {
      lock_guard<std::mutex> lock(endThreads_);
      endRefinement = true;
      endThreadsC.notify_all();
   }
   this_thread::sleep_for(std::chrono::microseconds(30));
   return 0;
}

void refinementLI::Scheduler() {
   // start distributor
   filterThreads.reserve(coreNoWorker + 1);
   buffer = make_shared<vector<shared_ptr<SafeQueue<Tuple*>>>>();
   threadsDone = coreNoWorker;
   nl->ReadFromString(((FText*) funText.addr)->GetValue(),
                      funList);
   for (size_t i = 0; i < coreNoWorker; ++i) {
      qpVec.emplace_back(
              new QueryProcessor(nl, SecondoSystem::GetAlgebraManager()));
      funct.emplace_back();
      bool correct = false;
      bool evaluable = false;
      bool defined = false;
      bool isFunction = false;
      ListExpr resultType;
      qpVec.back()->Construct(
              funList, correct, evaluable, defined, isFunction,
              funct.back(), resultType, false);
      buffer->push_back(make_shared<SafeQueue<Tuple*>>(i));
      filterThreads.emplace_back(
              RefinementWorker(coreNoWorker, i,
                               tupleBuffer,
                               buffer->back(),
                               qpVec.back(), funList,
                               funct.back(),
                               stream));
      filterThreads.back().detach();
   }
}


ListExpr op_refinement::refinementTM(ListExpr args) {
   // mthreadedFilter has 2 arguments
   // 1: Stream of Tuple
   // 2: fun

   const size_t cores = MThreadedSingleton::getCoresToUse();
   if (cores < 3) {
      return listutils::typeError(" only works with >= 3 threads ");
   }

   string err = "stream(tuple) x fun";
   if (nl->ListLength(args) != 2) {
      return listutils::typeError(err);
   }

   ListExpr stream = nl->First(nl->First(args));
   ListExpr map = nl->First(nl->Second(args));
   ListExpr fun = nl->Second(nl->Second(args));

   if (!Stream<Tuple>::checkType(stream)) {
      return
              listutils::typeError(err
                                   + " (first arg is not a tuple stream)");
   }

   if (!listutils::isMap<1>(map)) {
      return listutils::typeError(
              err + "(no map with arguments from one tuple)");
   }

   ListExpr funRes = nl->Third(map);
   if (!nl->IsAtom(funRes)) {
      return listutils::typeError(
              err + "(fun corrupt)");
   }
   if (nl->SymbolValue(funRes) == Symbol::TYPEERROR()) {
      return listutils::typeError(
              err + "(typeerror in fun)");
   }

   ListExpr attrList = nl->Second(nl->Second(map));
   if (!nl->Equal(attrList, nl->Second(nl->Second(stream)))) {
      return
              listutils::typeError(err + "(stream not mapped correctly)");
   }

   ListExpr mapAttr = nl->Third(fun);
   ListExpr function = mapAttr;
   while (nl->ListLength(mapAttr) != 3) {
      mapAttr = nl->Second(mapAttr);
   }

   // result of the function must be an attribute again , e . g . in
   // kind DATA
   if (!CcBool::checkType(funRes)) {
      return listutils::typeError(" fun result is not a bool ");
   }

   NList funList(nl);
   funList.append(NList("fun"));
   funList.append(NList(
           NList("streamelem1_1"), NList(nl->Second(stream))));
   funList.append(function);

   ListExpr appendList = nl->OneElemList(funList.listExpr());
   ListExpr last = appendList;
   for (size_t i = 2; i < cores - 1; ++i) {
      last = nl->Append(last, funList.listExpr());
   }

   return nl->ThreeElemList(nl->SymbolAtom(Symbols::APPEND()),
                            nl->OneElemList(nl->TextAtom(
                                    nl->ToString(funList.listExpr()))), stream);
}

// Operator SpatialJoin (firstArg = 1, param = false)
// args[0] : stream
// args[1] : fun1
// args[2] : index of join attribute R
// args[3] : index of join attribute S
// args[4...] : funs for other cores

int op_refinement::refinementVM(Word* args, Word &result, int message,
                                Word &local, Supplier s) {

   refinementLI* li = (refinementLI*) local.addr;

   switch (message) {

      case OPEN :
         if (li) {
            delete li;
         }
         local.addr = new refinementLI(args[0], args[2]);
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

std::string op_refinement::getOperatorSpec() {
   return OperatorSpec(
           " stream x fun -> stream",
           " stream mThreadedFilter(fun)",
           " spatial filter using >2 cores",
           " T feed mThreadedFilter[X_o comntains Y_p]"
   ).getStr();
}

shared_ptr<Operator> op_refinement::getOperator() {
   return std::make_shared<Operator>("mThreadedFilter",
                                     getOperatorSpec(),
                                     &op_refinement::refinementVM,
                                     Operator::SimpleSelect,
                                     &op_refinement::refinementTM);
}