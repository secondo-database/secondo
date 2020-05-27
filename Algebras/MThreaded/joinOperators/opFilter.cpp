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
#include <random>
#include <Algebras/FText/FTextAlgebra.h>


using namespace mthreaded;
using namespace std;

extern NestedList* nl;
extern QueryProcessor* qp;

namespace filterGlobal {
constexpr unsigned int DIMENSIONS = 2;
vector<shared_ptr<SafeQueue<Tuple*>>> partBuffer;
shared_ptr<SafeQueue<Tuple*>> tupleBuffer;
//shared_ptr<extrel2::TupleBuffer2> buffer;
size_t threadsDone;
}

using namespace filterGlobal;


RefinementWorker::RefinementWorker(
        size_t _coreNoWorker, size_t _streamInNo, ListExpr _funList) :
        coreNoWorker(_coreNoWorker),
        streamInNo(_streamInNo),
        funList(_funList) {
}

RefinementWorker::RefinementWorker(
        size_t _coreNoWorker, size_t _streamInNo, OpTree _fun) :
        coreNoWorker(_coreNoWorker), streamInNo(_streamInNo) {
   funct = _fun;
}

RefinementWorker::~RefinementWorker() {
   cout << "destruct candidate: " << streamInNo << endl;
}


// Thread
void RefinementWorker::operator()() {
   if (streamInNo == 0) {
      refineQP();
   } else {
      refineNewQP();
   }

   cout << "Worker Candidate Nr. done " << streamInNo << endl;
   --threadsDone;
   if (threadsDone == 0) {
      cout << "nullptr" << endl;
      tupleBuffer->enqueue(nullptr);
   }
}

void RefinementWorker::refineNewQP() {
   QueryProcessor* qproc =
           new QueryProcessor(SecondoSystem::GetNestedList(),
                   SecondoSystem::GetAlgebraManager());
   bool correct = false;
   bool evaluable = false;
   bool defined = false;
   bool isFunction = false;
   ListExpr resultType;
   qproc->Construct(
           funList, correct, evaluable, defined,isFunction,
           funct,resultType, true);
   Tuple* tuple = partBuffer[streamInNo]->dequeue();
   while (tuple != nullptr) {
      ArgVector &arguments = *qproc->Argument(funct);
      arguments[0].setAddr(tuple);
      Word funres;
      qproc->Request(funct, funres);
      bool res = ((CcBool*) funres.addr)->GetBoolval();
      if (res) {
         cout << "found" << streamInNo << endl;
         tupleBuffer->enqueue(tuple);
      } else {
         tuple->DeleteIfAllowed();
      }
      tuple = partBuffer[streamInNo]->dequeue();
   }
   qproc->Destroy(funct, true);
   delete qproc;
}

void RefinementWorker::refineQP() {
   Tuple* tuple = partBuffer[streamInNo]->dequeue();
   while (tuple != nullptr) {
      ArgVector &arguments = *qp->Argument(funct);
      arguments[0].setAddr(tuple);
      Word funres;
      qp->Request(funct, funres);
      bool res = ((CcBool*) funres.addr)->GetBoolval();
      if (res) {
         cout << "found" << streamInNo << endl;
         tupleBuffer->enqueue(tuple);
      } else {
         tuple->DeleteIfAllowed();
      }
      tuple = partBuffer[streamInNo]->dequeue();
   }
}

//Constructor
refinementLI::refinementLI(Word* _args) : args(_args), stream(_args[0]) {
   coreNo = MThreadedSingleton::getCoresToUse();
   coreNoWorker = coreNo - 1;
   stream.open();
   //buffer = make_shared<extrel2::TupleBuffer2>();
   Scheduler();
}


//Destructor
refinementLI::~refinementLI() {
   partBuffer.clear();
}

//Output
Tuple* refinementLI::getNext() {
   Tuple* res;
   res = tupleBuffer->dequeue();
   if (res != nullptr) {
      return res;
   }
   return 0;
}

void refinementLI::Scheduler() {
   // start threads
   joinThreads.reserve(coreNoWorker);
   partBuffer.reserve(coreNoWorker);
   tupleBuffer = make_shared<SafeQueue<Tuple*>>(coreNoWorker);
   threadsDone = coreNoWorker;
   partBuffer.push_back(make_shared<SafeQueue<Tuple*>>(0));
   joinThreads.emplace_back(
           RefinementWorker(coreNoWorker,
                   0, (OpTree) args[1].addr));
   //joinThreads.back().detach();
   for (size_t i = 1; i < coreNoWorker; ++i) {
      partBuffer.push_back(make_shared<SafeQueue<Tuple*>>(i));
      ListExpr funList;
      nl->ReadFromString(((FText*) args[2].addr)->GetValue(),
              funList);
      joinThreads.emplace_back(
              RefinementWorker(coreNoWorker, i, funList));
      //joinThreads.back().detach();
   }

   // Stream
   Tuple* tuple = stream.request();
   size_t countWorker = 0;
   do {
      partBuffer[countWorker]->enqueue(tuple);
      if (countWorker == coreNoWorker - 1) {
         countWorker = 0;
      } else {
         ++countWorker;
      }
   } while ((tuple = stream.request()));
   stream.close();

   cout << "Stream Ready" << endl;

   for (size_t i = 0; i < coreNoWorker; ++i) {
      partBuffer[i]->enqueue(nullptr);
      joinThreads[i].join();
   }

   cout << "Schedule Ready" << endl;
}


ListExpr op_refinement::refinementTM(ListExpr args) {

   // mthreadedHybridJoin has 2 arguments
   // 1: Stream of Tuple with spatial Attr
   // 2: fun spatial predicate

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
              listutils::typeError(err + " (first arg is not a tuple stream)");
   }
   cout << "Stream" << endl;

   if (!listutils::isMap<1>(map)) {
      return listutils::typeError(
              err + "(no map with arguments from one tuple)");
   }
   cout << "Map" << endl;

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
   string mapAttrName1 = nl->SymbolValue(nl->Third(nl->Second(mapAttr)));
   string mapAttrName2 = nl->SymbolValue(nl->Third(nl->Third(mapAttr)));
   cout << mapAttrName1 << "##" << mapAttrName2 << endl;
   ListExpr attrType1;
   ListExpr attrType2;
   int index1 = listutils::findAttribute(attrList, mapAttrName1, attrType1);
   int index2 = listutils::findAttribute(attrList, mapAttrName2, attrType2);
   cout << "index" << index1 << "##" << index2 << endl;
   if (index1 == -1 || index2 == -1) {
      return
              listutils::typeError(" function attribute not in stream ");
   }
   if (!listutils::isSpatialType(attrType1)) {
      return listutils::typeError(" first attribute not spatial ");
   }
   if (!listutils::isSpatialType(attrType2)) {
      return listutils::typeError(" second attribute not spatial ");
   }

   ListExpr funRes = nl->Third(map);
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

         local.addr = new refinementLI(args);
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