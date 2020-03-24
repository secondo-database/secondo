/*
----
This file is part of SECONDO.

Copyright (C) 2015,
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

//[$][\$]

*/

#include "partitionFS.h"
#include "DInputConsumer.h"

using namespace std;
using namespace distributed2;

/*

0 Functions from distributed2Algebras

*/
namespace distributed2
{
// Algebra instance
extern Distributed2Algebra *algInstance;
} // namespace distributed2

namespace distributed5
{

/*

1 partitionFS Operator

Creates a stream of tasks. 
Input Parameter can be

  * a stream of tasks
  
  * D[F]Array

1.1 Type Mapping 

*/

ListExpr partitionFSTM(ListExpr args)
{
    string err =
        "{{d[f]array(rel(X)) / stream(task(d[f]array(rel(X))))} x string x "
        "(fsrel(X) -> stream(tuple(Y))) x "
        "(tuple(Y) -> int) x "
        "int} expected";

    //ensure that exactly 5 argument comes into partitionFS
    if (!nl->HasLength(args, 5))
    {
        return listutils::typeError(err + " (wrong number of args)");
    }

    ListExpr argInput = nl->First(args);

    // check for internal correctness
    if (!nl->HasLength(argInput, 2))
    {
        return listutils::typeError("internal error");
    }

    ListExpr argName = nl->Second(args);
    ListExpr argFunMap = nl->Third(args);
    ListExpr argFunPartition = nl->Fourth(args);
    ListExpr argVSlots = nl->Fifth(args);

    // check for internal correctness (uses Args in type mapping)
    if (!nl->HasLength(argInput, 2) ||
        !nl->HasLength(argName, 2) ||
        !nl->HasLength(argFunMap, 2) ||
        !nl->HasLength(argFunPartition, 2) ||
        !nl->HasLength(argVSlots, 2))
    {
        return listutils::typeError("internal error");
    }

    ListExpr argInputType = nl->First(argInput);
    // i.e. argInputType =
    //   (darray int) or
    //   (dfarray int) or
    //   (stream (task (darray int))) or
    //   (stream (task (dfarray int)))

    bool inputIsDArray = DArray::checkType(argInputType);
    bool inputIsDFArray = DFArray::checkType(argInputType);
    bool inputIsDTaskStream = Stream<Task>::checkType(argInputType) &&
                              DArray::checkType(
                                  Task::innerType(
                                      nl->Second(argInputType)));
    bool inputIsDFTaskStream = Stream<Task>::checkType(argInputType) &&
                               DFArray::checkType(
                                   Task::innerType(
                                       nl->Second(argInputType)));
    bool inputIsStream = inputIsDTaskStream || inputIsDFTaskStream;
    bool inputOk = inputIsDArray ||
                   inputIsDFArray ||
                   inputIsDTaskStream ||
                   inputIsDFTaskStream;
    if (!inputOk)
    {
        return listutils::typeError(err + " (input invalid)");
    }

    ListExpr argNameType = nl->First(argName);
    ListExpr argFunMapType = nl->First(argFunMap);
    ListExpr argFunPartitionType = nl->First(argFunPartition);
    ListExpr argVSlotsType = nl->First(argVSlots);

    if (!CcString::checkType(argNameType))
    {
        return listutils::typeError(err + " (name type invalid)");
    }

    if (
        !listutils::isMap<1>(argFunMapType) ||
        !listutils::isMap<1>(argFunPartitionType))
    {
        return listutils::typeError(err + " (fun type invalid)");
    }

    if (!CcInt::checkType(argVSlotsType))
    {
        return listutils::typeError(err + " (vslots type invalid)");
    }

    ListExpr darrayType = Stream<Task>::checkType(argInputType)
                              ? Task::innerType(nl->Second(argInputType))
                              : argInputType;

    //Function argument type
    ListExpr funMapArg = nl->Second(argFunMapType);

    //expected Function Argument type
    ListExpr expFunMapArg = nl->TwoElemList(
        listutils::basicSymbol<fsrel>(),
        nl->Second(nl->Second(darrayType)));

    if (!nl->Equal(expFunMapArg, funMapArg))
    {
        stringstream ss;
        ss << "type mismatch between map function argument"
           << " and subtype of d[f]array" << endl
           << "subtype is " << nl->ToString(expFunMapArg) << endl
           << "funarg is " << nl->ToString(funMapArg) << endl;

        return listutils::typeError(ss.str());
    }

    //Function return type
    ListExpr funMapRes = nl->Third(argFunMapType);

    if (!Stream<Tuple>::checkType(funMapRes))
    {
        return listutils::typeError(err +
                                    " (map fun must return a tuple stream)");
    }

    // the function definition
    ListExpr funMap = nl->Second(argFunMap);

    // we have to replace the given function arguments
    // by the real function arguments because the
    // given function argument may be a TypeMapOperator
    ListExpr rfunMap = nl->ThreeElemList(
        nl->First(funMap),
        nl->TwoElemList(nl->First(nl->Second(funMap)), funMapArg),
        nl->Third(funMap));

    // compute the subtype of the resulting array
    ListExpr tupleType = nl->Second(funMapRes);
    ListExpr relType = nl->TwoElemList(
        listutils::basicSymbol<Relation>(), tupleType);

    ListExpr funPartitionArg = nl->Second(argFunPartitionType);

    if (!nl->Equal(tupleType, funPartitionArg))
    {
        stringstream ss;
        ss << "type mismatch between partition function argument"
           << " and result of map function" << endl
           << "result is " << nl->ToString(tupleType) << endl
           << "funarg is " << nl->ToString(funPartitionArg) << endl;

        return listutils::typeError(ss.str());
    }

    ListExpr funPartitionRes = nl->Third(argFunPartitionType);

    if (!CcInt::checkType(funPartitionRes))
    {
        return listutils::typeError(err +
                                    " (partition fun must return int)");
    }

    // the function definition
    ListExpr funPartition = nl->Second(argFunPartition);

    // we have to replace the given function arguments
    // by the real function arguments because the
    // given function argument may be a TypeMapOperator
    ListExpr rfunPartition = nl->ThreeElemList(
        nl->First(funPartition),
        nl->TwoElemList(nl->First(nl->Second(funPartition)), funPartitionArg),
        nl->Third(funPartition));

    // i.e. (stream (task (dfmatrix (rel ...))))
    ListExpr resType = nl->TwoElemList(
        listutils::basicSymbol<Stream<Task>>(),
        nl->TwoElemList(
            listutils::basicSymbol<Task>(),
            nl->TwoElemList(
                listutils::basicSymbol<DFMatrix>(),
                relType)));

    ListExpr appendValues = nl->ThreeElemList(
        nl->BoolAtom(inputIsStream),
        nl->TextAtom(nl->ToString(rfunMap)),
        nl->TextAtom(nl->ToString(rfunPartition)));

    return nl->ThreeElemList(
        nl->SymbolAtom(Symbols::APPEND()),
        appendValues,
        resType);
}

/*

1.2 Local Information Class for the partitionFS Operator

*/

class partitionFSLI
{
public:
    partitionFSLI(DInputConsumer &&input,
                  string mapFunction, string partitionFunction,
                  string remoteName, int vslots, ListExpr contentType)
        : input(std::move(input)),
          mapFunction(mapFunction),
          partitionFunction(partitionFunction),
          remoteName(remoteName),
          vslots(vslots),
          contentType(contentType) {}

    //destructor of partitionFSLI
    ~partitionFSLI()
    {
    }

    //returns the next task for the successor operator
    Task *getNext()
    {
        if (!inputConsumed)
        {
            Task *inputTask = input.request();
            if (inputTask != 0)
            {
                if (inputTask->getTaskType() == "worker")
                    workerCount++;
                if (inputTask->hasFlag(Output))
                {
                    inputTask->clearFlag(Output);
                    collectedTasks.push_back(inputTask);
                }
                return inputTask;
            }
            inputConsumed = true;
        }
        if (currentWorker >= workerCount)
            return 0;
        auto *partitionTask = new PartitionFunctionTask(
            mapFunction, partitionFunction,
            remoteName, vslots, contentType);
        partitionTask->setFlag(Output);
        size_t len = collectedTasks.size();
        for (size_t i = currentWorker; i < len; i += workerCount)
        {
            partitionTask->addPredecessorTask(collectedTasks[i]);
        }
        currentWorker++;
        return partitionTask;
    }

private:
    bool inputConsumed = false;
    size_t workerCount = 0;
    size_t currentWorker = 0;
    std::vector<Task *> collectedTasks;
    DInputConsumer input;
    string mapFunction;
    string partitionFunction;
    string remoteName;
    int vslots;
    ListExpr contentType;
};

/*

1.3 Value Mapping for partitionFS

*/
int partitionFSVM(Word *args,
                  Word &result,
                  int message,
                  Word &local,
                  Supplier s)
{

    partitionFSLI *li = (partitionFSLI *)local.addr;

    switch (message)
    {
    case OPEN:
    {
        if (li)
        {
            delete li;
        }
        // Arguments are:
        // input, remoteName, fnMap*, fnPartition*, vslots,
        // isStream, fnMapText, fnPartitionText
        CcString *incomingRemoteName = (CcString *)args[1].addr;
        CcInt *incomingVSlots = (CcInt *)args[4].addr;
        bool isStream = ((CcBool *)args[5].addr)->GetValue();
        string mapFunction = ((FText *)args[6].addr)->GetValue();
        string partitionFunction = ((FText *)args[7].addr)->GetValue();

        // create a new name for the result matrix
        std::string remoteName;
        if (!incomingRemoteName->IsDefined() ||
            incomingRemoteName->GetValue().length() == 0)
        {
            remoteName = algInstance->getTempName();
        }
        else
        {
            remoteName = incomingRemoteName->GetValue();
        }
        // check whether the name is valid
        if (!stringutils::isIdent(remoteName))
        {
            return 0;
        }

        int vslots = 0;
        if (incomingVSlots->IsDefined())
        {
            vslots = incomingVSlots->GetValue();
        }

        //check for all previous tasks
        DInputConsumer input(
            isStream
                ? DInputConsumer(args[0])
                : DInputConsumer(
                      (DArrayBase *)args[0].addr,
                      DInputConsumer::getContentType(
                          qp->GetType(qp->GetSon(s, 0)))));

        local.addr = li =
            new partitionFSLI(std::move(input),
                              mapFunction,
                              partitionFunction,
                              remoteName,
                              vslots,
                              Task::resultType(nl->Second(qp->GetType(s))));

        return 0;
    }
    case REQUEST:
        result.addr = li ? li->getNext() : 0;
        return result.addr ? YIELD : CANCEL;

    case CLOSE:
        if (li)
        {
            delete li;
            local.addr = 0;
        }
        return 0;
    }

    return 0;
}

/*

1.4 Specification for partitionFS

*/
OperatorSpec partitionFSSpec(
    "d[f]array(X)/tasks(d[f]array(X)) x string x fun x fun x int "
    "-> tasks(dfmatrix(Y))",
    "_ partitionFS[_,_]",
    "Partitions distributed data across a vertical partitioning schema",
    "");

/*

1.5 Operator partitionFS

*/

Operator partitionFSOp(
    "partitionFS",
    partitionFSSpec.getStr(),
    partitionFSVM,
    Operator::SimpleSelect,
    partitionFSTM);

} // namespace distributed5
