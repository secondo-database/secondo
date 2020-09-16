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
#include "LogMsg.h"             // send error messages
#include <cmath>
#include <atomic>
#include <Algebras/FText/FTextAlgebra.h>


using namespace mthreaded;
using namespace std;

extern NestedList* nl;
extern QueryProcessor* qp;

namespace filterGlobal {
constexpr unsigned int DIMENSIONS = 2;
std::atomic<size_t> threadsDone;
std::atomic<size_t> bufferDone;
bool startCalc;
bool startGetNext;
bool endRefinement;
std::mutex endThreads_;
std::mutex stopCalc_;
std::mutex stopGetNext_;
std::mutex qp_;
std::condition_variable stopCalcC;
std::condition_variable stopGetNextC;
std::condition_variable endThreadsC;
}

using namespace filterGlobal;

RefinementWorkerNew::RefinementWorkerNew(
        size_t _coreNoWorker,
        //size_t _bufferSize,
        size_t _streamInNo,
        shared_ptr<SafeQueue<Tuple*>> _tupleBuffer,
        shared_ptr<SafeQueue<Tuple*>> _partBuffer,
        QueryProcessor* _qpThread,
        ListExpr _funList,
        OpTree _funct) :
        coreNoWorker(_coreNoWorker),
        //bufferSize(_bufferSize),
        streamInNo(_streamInNo),
        tupleBuffer(_tupleBuffer), partBuffer(_partBuffer),
        qpThread(_qpThread), funList(_funList), funct(_funct) {
}

RefinementWorkerNew::~RefinementWorkerNew() {
   cout << "destruct candidate: " << streamInNo << endl;
}

// Thread
void RefinementWorkerNew::operator()() {
   endRefinement = false;
   ArgVector &arguments = *qpThread->Argument(funct);
   Tuple* tuple = partBuffer->dequeue();
   while (tuple != nullptr) {
      arguments[0].setAddr(tuple);
      Word funres;
      qpThread->Request(funct, funres);
      bool res = false;
      if (((Attribute*) funres.addr)->IsDefined()) {
         res = ((CcBool*) funres.addr)->GetBoolval();
      }
      if (res) {
         while (tupleBuffer->size() > 50) { usleep(50); };
         tupleBuffer->enqueue(tuple);
      } else {
         tuple->DeleteIfAllowed();
      }
      tuple = partBuffer->dequeue();
   }
   --threadsDone;
   if (threadsDone == 0) {
      tupleBuffer->enqueue(nullptr);
   }
   {
      unique_lock<std::mutex> lock(endThreads_);
      cout << "wait" << endl;
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
   //bufferSize = 10 * coreNoWorker;
   Scheduler();
}


//Destructor
refinementLI::~refinementLI() {
   for (shared_ptr<SafeQueue<Tuple*>> partBuffer : buffer) {
      partBuffer.reset();
   }
   cout << "1";
   buffer.clear();
   tupleBuffer.reset();
   size_t i = 0;
   for (QueryProcessor* qpThread : qpVec) {
      //qpThread->DeleteResultStorage(funct[i]);
      qpThread->Destroy(funct[i], true);
      delete qpThread;
      ++i;
   }
   cout << "2";
   funct.clear();
   cout << "3";
   qpVec.clear();
   cout << "4";
   stream.close();
   for (size_t i = 0; i < coreNoWorker; ++i) {
      if (filterThreads[i].joinable()) {
         filterThreads[i].detach();
      }
   }
   filterThreads.clear();
   cout << "5";
}

//Output
Tuple* refinementLI::getNext() {
   // reading stream phase
   if (phaseStream && !streamDone) {
      Tuple* tuple;
      streamDone = true;
      while ((tuple = stream.request())) {
         while (buffer[countWorker]->size() > 10) { usleep(50); };

         buffer[countWorker]->enqueue(tuple);

         if (countWorker == coreNoWorker - 1) {
            countWorker = 0;
         } else {
            ++countWorker;
         }
         ++fillCounter;
         // switch to readRes
         if (!(tupleBuffer->empty())) {
            fillCounter = 0;
            streamDone = false;
            phaseStream = false;
            break;
         }
      }
      if (streamDone) {
         for (size_t i = 0; i < coreNoWorker; ++i) {
            buffer[i]->enqueue(nullptr);
         }
      }
      // return res phase
   }
   Tuple* res;
   res = tupleBuffer->dequeue();
   if (tupleBuffer->empty()) {
      phaseStream = true;
   }
   if (res != nullptr) {
      return res;
   }
   endRefinement = false;
   {
      lock_guard<std::mutex> lock(endThreads_);
      endRefinement = true;
      endThreadsC.notify_all();
   }
//   for (size_t i = 0; i < coreNoWorker; ++i) {
//      filterThreads[i].join();
//   }
   return 0;
}

void refinementLI::Scheduler() {
   threadsDone = coreNoWorker;
   bufferDone = coreNoWorker;
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
      buffer.push_back(make_shared<SafeQueue<Tuple*>>(i));
      filterThreads.emplace_back(
              RefinementWorkerNew(coreNoWorker, i,
                                  tupleBuffer,
                                  buffer.back(),
                                  qpVec.back(), funList,
                                  funct.back()));
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

   cout << "TM SJ: " << nl->ToString(args) << "##" << nl->ListLength(args)
        << endl;

   string err = "stream(tuple) x fun";
   if (nl->ListLength(args) != 2) {
      return listutils::typeError(err);
   }

   ListExpr stream = nl->First(nl->First(args));
   ListExpr map = nl->First(nl->Second(args));
   ListExpr fun = nl->Second(nl->Second(args));
   cout << "Stream" << nl->ToString(stream) << endl;
   cout << "Map" << nl->ToString(map) << endl;
   cout << "Fun" << nl->ToString(fun) << endl;


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
   cout << nl->ToString(funRes) << endl;
   if (nl->SymbolValue(funRes) == Symbol::TYPEERROR()) {
      return listutils::typeError(
              err + "(typeerror in fun)");
   }

   ListExpr attrList = nl->Second(nl->Second(map));
   if (!nl->Equal(attrList, nl->Second(nl->Second(stream)))) {
      return
              listutils::typeError(err + "(stream not mapped correctly)");
   }
   cout << "attrList" << nl->ToString(attrList) << endl;

   ListExpr mapArg = nl->Second(map);
   cout << nl->ToString(mapArg) << endl;


   ListExpr mapAttr = nl->Third(fun);
   ListExpr function = mapAttr;
   cout << "Formel" << nl->ToString(function) << endl;
   while (nl->ListLength(mapAttr) != 3) {
      mapAttr = nl->Second(mapAttr);
   }
   cout << "Attributes" << nl->ToString(mapAttr) << endl;
//   if (!nl->IsAtom(nl->Second(mapAttr))) {
//      ListExpr attrType;
//      string mapAttrName = nl->SymbolValue(nl->Third(nl->Second(mapAttr)));
//      int index = listutils::findAttribute(attrList, mapAttrName, attrType);
//      if (index == -1) {
//         return
//                 listutils::typeError
//                         (" function attribute not in stream ");
//      }
//   }
//   if (!nl->IsAtom(nl->Third(mapAttr))) {
//      ListExpr attrType;
//      string mapAttrName = nl->SymbolValue(nl->Third(nl->Third(mapAttr)));
//      int index = listutils::findAttribute(attrList, mapAttrName, attrType);
//      if (index == -1) {
//         return
//                 listutils::typeError
//                         (" function attribute not in stream ");
//      }
//   }


//   if (!listutils::isSpatialType(attrType1)) {
//      return listutils::typeError(" first attribute not spatial ");
//   }
//   if (!listutils::isSpatialType(attrType2)) {
//      return listutils::typeError(" second attribute not spatial ");
//   }

   cout << nl->ToString(funRes) << endl;
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


   //ListExpr appendList = nl->OneElemList(nl->IntAtom(index1 - 1));
   ListExpr appendList = nl->OneElemList(funList.listExpr());
   ListExpr last = appendList;
   //last = nl->Append(last, nl->IntAtom(index2 - 1));
   for (size_t i = 2; i < cores - 1; ++i) {
      last = nl->Append(last, funList.listExpr());
   }

   cout << "append: " << nl->ToString(appendList) << endl;
   nl->WriteListExpr(funList.listExpr());
   cout << "fertig TM" << endl;
   //nl->ReadFromString(funStr, funListGlobal );
   cout << "fertig TM" << endl;
   //nl->WriteListExpr(funListGlobal);
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
         //resultTupleType->DeleteIfAllowed();
         return 0;
      case REQUEST:
         result.addr = li ? li->getNext() : 0;
         return result.addr ? YIELD : CANCEL;
      case CLOSE:
         cout << "close" << endl;
         if (li) {
            delete li;
            local.addr = 0;
         }
         cout << "closed" << endl;
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