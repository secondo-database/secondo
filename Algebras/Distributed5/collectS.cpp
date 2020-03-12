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

#include "collectS.h"
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

1 collectS Operator

Creates a stream of tasks. 
Input Parameter can be

  * a stream of tasks
  
  * D[F]Array

1.1 Type Mapping 

*/

ListExpr collectSTM(ListExpr args)
{
    string err =
        "{{dfmatrix(X) / stream(task(dfmatrix(X)))} x string} expected";

    //ensure that exactly 2 arguments comes into collectS
    if (!nl->HasLength(args, 2))
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

    // check for internal correctness (uses Args in type mapping)
    if (!nl->HasLength(argName, 2))
    {
        return listutils::typeError("internal error");
    }

    ListExpr argInputType = nl->First(argInput);
    // i.e. argInputType =
    //   (dfmatrix X) or
    //   (stream (task (dfmatrix int)))

    bool inputIsDFMatrix = DFMatrix::checkType(argInputType);
    bool inputIsDFTaskStream = Stream<Task>::checkType(argInputType) &&
                               DFMatrix::checkType(
                                   Task::innerType(
                                       nl->Second(argInputType)));
    bool inputIsStream = inputIsDFTaskStream;
    bool inputOk = inputIsDFMatrix || inputIsDFTaskStream;
    if (!inputOk)
    {
        return listutils::typeError(err + " (input invalid)");
    }

    ListExpr argNameType = nl->First(argName);

    if (!CcString::checkType(argNameType))
    {
        return listutils::typeError(err + " (name type invalid)");
    }

    ListExpr dfmatrixType = Stream<Task>::checkType(argInputType)
                                ? Task::innerType(nl->Second(argInputType))
                                : argInputType;

    ListExpr relType = nl->Second(dfmatrixType);
    string relTypeString = nl->ToString(relType);

    // i.e. (stream (task (dfarray (rel ...))))
    ListExpr resType = nl->TwoElemList(
        listutils::basicSymbol<Stream<Task>>(),
        nl->TwoElemList(
            listutils::basicSymbol<Task>(),
            nl->TwoElemList(
                listutils::basicSymbol<DFArray>(),
                relType)));

    ListExpr appendValues = nl->OneElemList(
        nl->BoolAtom(inputIsStream));

    return nl->ThreeElemList(
        nl->SymbolAtom(Symbols::APPEND()),
        appendValues,
        resType);
}

/*

1.2 Local Information Class for the collectS Operator

*/

class collectSLI
{
public:
    collectSLI(DInputConsumer &&input, string remoteName, ListExpr contentType)
        : input(std::move(input)),
          remoteName(remoteName),
          contentType(contentType) {}

    //destructor of collectSLI
    ~collectSLI()
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
                if (inputTask->hasFlag(Output))
                {
                    inputTask->clearFlag(Output);
                    collectedTasks.push_back(inputTask);
                }
                return inputTask;
            }
            inputConsumed = true;
        }

        if (collectedTasks.empty())
            return 0;

        auto &first = collectedTasks[0];

        if (currentVSlot >= first->getNumberOfResults())
            return 0;

        auto *collectTask = new CollectFunctionTask(remoteName, contentType);
        collectTask->setFlag(Output);
        size_t vslot = currentVSlot++;
        // The primary argument is the task where vslot == slot
        // It only influences the preferred locations
        // and it not part of the collecting process
        collectTask->addArgument(
            collectedTasks[vslot % collectedTasks.size()], vslot);
        // The secondary arguments are collected
        // and merged
        for (Task *task : collectedTasks)
        {
            collectTask->addArgument(task, vslot);
        }

        return collectTask;
    }

private:
    size_t currentVSlot = 0;
    std::vector<Task *> collectedTasks;
    bool inputConsumed = false;
    DInputConsumer input;
    string remoteName;
    ListExpr contentType;
};

/*

1.3 Value Mapping for collectS

*/
int collectSVM(Word *args,
               Word &result,
               int message,
               Word &local,
               Supplier s)
{

    collectSLI *li = (collectSLI *)local.addr;

    switch (message)
    {
    case OPEN:
    {
        if (li)
        {
            delete li;
        }
        // Arguments are:
        // input, remoteName, isStream
        CcString *incomingRemoteName = (CcString *)args[1].addr;
        bool isStream = ((CcBool *)args[2].addr)->GetValue();

        // create a new name for the result array
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

        DInputConsumer input(
            isStream
                ? DInputConsumer(args[0])
                : DInputConsumer(
                      (DArrayBase *)args[0].addr,
                      DInputConsumer::getContentType(
                          qp->GetType(qp->GetSon(s, 0)))));

        local.addr = li =
            new collectSLI(std::move(input),
                           remoteName,
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

1.4 Specification for collectS

*/

OperatorSpec collectSSpec(
    "dfmatrix(X)/tasks(dfmatrix(X)) x string -> tasks(dfarray(X))",
    "_ collectS[_,_]",
    "Collects the slots of a matrix into a "
    "dfarray. The string is the name of the "
    "resulting array.",
    "");

/*

1.5 Operator for collectS

*/
Operator collectSOp(
    "collectS",
    collectSSpec.getStr(),
    collectSVM,
    Operator::SimpleSelect,
    collectSTM);

} // namespace distributed5
