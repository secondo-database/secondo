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

#include "dproductS.h"
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

1 dproductS Operator

Creates a stream of tasks. 

1.1 Type Mapping 

*/

ListExpr dproductSTM(ListExpr args)
{
    if (!nl->HasLength(args, 4))
    {
        return listutils::typeError("4 arguments expected");
    }
    string err = "d[f]array(X)/tasks(d[f]array(X)) x "
                 "d[f]array(rel(Y))/tasks(d[f]array(rel(Y))) x string x "
                 "(X x fsrel(Y) -> Z) -> tasks(d[f]array(Z))  excpected";

    ListExpr c = args;
    // check for args in type mapping
    while (!nl->IsEmpty(c))
    {
        if (!nl->HasLength(nl->First(c), 2))
        {
            return listutils::typeError("internal error");
        }
        c = nl->Rest(c);
    }

    ListExpr firstArg = nl->First(args);
    ListExpr secondArg = nl->Second(args);
    ListExpr nameArg = nl->Third(args);
    ListExpr funArg = nl->Fourth(args);

    ListExpr firstArgValue = nl->First(firstArg);
    ListExpr secondArgValue = nl->First(secondArg);
    ListExpr nameArgValue = nl->First(nameArg);
    ListExpr funArgValue = nl->First(funArg);

    ListExpr firstArgInnerType;
    bool firstArgIsStream = false;
    ListExpr firstArgArrayType = firstArgValue;
    if (Stream<Task>::checkType(firstArgValue))
    {
        firstArgIsStream = true;
        firstArgArrayType = Task::innerType(nl->Second(firstArgValue));
    }
    if (DArray::checkType(firstArgArrayType))
    {
        firstArgInnerType = nl->Second(firstArgArrayType);
    }
    else if (DFArray::checkType(firstArgArrayType))
    {
        firstArgInnerType = nl->TwoElemList(
            listutils::basicSymbol<frel>(),
            nl->Second(nl->Second(firstArgArrayType)));
    }
    else
    {
        return listutils::typeError(
            err + " (first arg is not a d[f]array or a stream of tasks)");
    }
    if (!Relation::checkType(firstArgInnerType) &&
        !frel::checkType(firstArgInnerType))
    {
        return listutils::typeError(
            err + " (only relations are allowed for the "
                  "subtypes of the d[f]arrays, first arg)");
    }

    ListExpr secondArgInnerType;
    bool secondArgIsStream = false;
    ListExpr secondArgArrayType = secondArgValue;
    if (Stream<Task>::checkType(secondArgValue))
    {
        secondArgIsStream = true;
        secondArgArrayType = Task::innerType(nl->Second(secondArgValue));
    }
    if (DArray::checkType(secondArgArrayType) ||
        DFArray::checkType(secondArgArrayType))
    {
        secondArgInnerType = nl->Second(secondArgArrayType);
    }
    else
    {
        return listutils::typeError(
            err + " (second arg is not a d[f]array or a stream of tasks)");
    }
    if (!Relation::checkType(secondArgInnerType))
    {
        return listutils::typeError(
            err + " (only relations are allowed for the "
                  "subtypes of the d[f]arrays, second arg)");
    }

    if (!CcString::checkType(nameArgValue))
    {
        return listutils::typeError(err + " (third arg is not a string)");
    }

    if (!listutils::isMap<2>(funArgValue))
    {
        return listutils::typeError(
            err + " (4th arg is not a binary function)");
    }

    ListExpr secondArgAsFsrel = nl->TwoElemList(listutils::basicSymbol<fsrel>(),
                                                nl->Second(secondArgInnerType));

    ListExpr funArg1 = nl->Second(funArgValue);
    if (!nl->Equal(firstArgInnerType, funArg1))
    {
        return listutils::typeError(err + " (first function argument " +
                                    nl->ToString(funArg1) +
                                    "does not fit the d[f]array type " +
                                    nl->ToString(firstArgInnerType) +
                                    ")");
    }
    ListExpr funArg2 = nl->Third(funArgValue);
    if (!nl->Equal(secondArgAsFsrel, funArg2))
    {
        return listutils::typeError(err + " (second function argument " +
                                    nl->ToString(funArg2) +
                                    "does not fit the d[f]array type " +
                                    nl->ToString(secondArgAsFsrel) +
                                    ")");
    }

    // check the resulting type of the function. if it is a stream, it has to
    // be a tuple stream
    ListExpr funResult = nl->Fourth(funArgValue);
    bool isStream = false;
    ListExpr resType;
    if (listutils::isStream(funResult))
    {
        isStream = true;
        if (!Stream<Tuple>::checkType(funResult))
        {
            return listutils::typeError("invalid function result, stream, "
                                        "but not a tuple stream");
        }
        resType = nl->TwoElemList(
            listutils::basicSymbol<Relation>(),
            nl->Second(funResult));
    }
    else
    {
        resType = funResult;
    }
    bool isRel = Relation::checkType(funResult);

    // replace function arguments
    ListExpr funQuery = nl->Second(funArg);
    ListExpr newFunArg1 = nl->TwoElemList(
        nl->First(nl->Second(funQuery)),
        funArg1);
    ListExpr newFunArg2 = nl->TwoElemList(
        nl->First(nl->Third(funQuery)),
        funArg2);
    ListExpr rfunQuery = nl->FourElemList(
        nl->First(funQuery),
        newFunArg1,
        newFunArg2,
        nl->Fourth(funQuery));

    ListExpr resultType = isStream || isRel
                              ? listutils::basicSymbol<DFArray>()
                              : listutils::basicSymbol<DArray>();

    ListExpr streamResType = nl->TwoElemList(
        listutils::basicSymbol<Stream<Task>>(),
        nl->TwoElemList(
            listutils::basicSymbol<Task>(),
            nl->TwoElemList(resultType, resType)));

    ListExpr appendList =
        nl->FiveElemList(
            nl->BoolAtom(firstArgIsStream),
            nl->BoolAtom(secondArgIsStream),
            nl->TextAtom(nl->ToString(rfunQuery)),
            nl->BoolAtom(isRel),
            nl->BoolAtom(isStream));

    return nl->ThreeElemList(nl->SymbolAtom(Symbols::APPEND()),
                             appendList,
                             streamResType);
}

/*

1.2 Local Info Class for dproductS 

*/

class dproductSLI
{
public:
    dproductSLI(DInputConsumer &&a1, DInputConsumer &&a2,
                string dproductFunction,
                string remoteName, ListExpr contentType,
                bool isRel, bool isStream)
        : input1(std::move(a1)), input2(std::move(a2)),
          dproductFunction(dproductFunction),
          remoteName(remoteName), contentType(contentType),
          isRel(isRel), isStream(isStream)
    {
    }

    Task *getNext()
    {
        // return queued tasks first
        if (!outputTasks.empty())
        {
            Task *res = outputTasks.front();
            outputTasks.pop_front();

            return res;
        }

        // return tasks from input1 until leaf is found
        if (input1Leaf == 0)
        {
            Task *task = input1.request();
            if (task == 0)
            {
                // end of stream
                return 0;
            }
            if (task->hasFlag(Output))
            {
                task->clearFlag(Output);
                input1Leaf = task;
            }
            return task;
        }

        // when input2 has not be consumed yet
        // read all of it and prepare leaf tasks for copy
        if (!input2Consumed)
        {
            Task *task = input2.request();
            if (task != 0)
            {
                if (task->hasFlag(Output))
                {
                    task->clearFlag(Output);
                    // Prepare Task for copy
                    preparedTasks.push_back(task);
                }
                return task;
            }
            input2Consumed = true;
            if (preparedTasks.empty())
            {
                // no leafs from input2
                // that should not happen
                // we can't combine
                return 0;
            }
        }

        // Final step: combine prepared tasks with leaf task from input1

        // create a dproduct task
        Task *dproductTask = new DproductFunctionTask(
            dproductFunction, remoteName, contentType, isRel, isStream);
        dproductTask->setFlag(Output);
        dproductTask->addPredecessorTask(input1Leaf);
        for (Task *t : preparedTasks)
        {
            dproductTask->addPredecessorTask(t);
        }
        outputTasks.push_back(dproductTask);

        // reset leaf variable to get next leaf task
        input1Leaf = 0;

        // return first output task from queue
        Task *res = outputTasks.front();
        outputTasks.pop_front();

        return res;
    }

private:
    DInputConsumer input1;
    DInputConsumer input2;
    std::deque<Task *> outputTasks;
    Task *input1Leaf = 0;
    bool input2Consumed = false;
    std::vector<Task *> preparedTasks;
    string dproductFunction;
    string remoteName;
    ListExpr contentType;
    bool isRel;
    bool isStream;
};

/*

1.3 Value Mapping for dproductS 

*/

int dproductSVM(Word *args, Word &result, int message,
                Word &local, Supplier s)
{

    dproductSLI *li = (dproductSLI *)local.addr;

    switch (message)
    {
    case OPEN:
    {
        if (li)
        {
            delete li;
        }
        // Arguments are:
        // a, b, remoteName, fn*,
        // aIsStream, bIsStream, functionText, isRel, isStream
        CcString *incomingRemoteName = (CcString *)args[2].addr;
        bool aIsStream = ((CcBool *)args[4].addr)->GetValue();
        bool bIsStream = ((CcBool *)args[5].addr)->GetValue();
        FText *incomingFunction = (FText *)args[6].addr;
        bool isRel = ((CcBool *)args[7].addr)->GetValue();
        bool isStream = ((CcBool *)args[8].addr)->GetValue();

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

        //check for all previous tasks
        DInputConsumer a(
            aIsStream
                ? DInputConsumer(args[0])
                : DInputConsumer(
                      (DArrayBase *)args[0].addr,
                      DInputConsumer::getContentType(
                          qp->GetType(qp->GetSon(s, 0)))));
        DInputConsumer b(
            bIsStream
                ? DInputConsumer(args[1])
                : DInputConsumer(
                      (DArrayBase *)args[1].addr,
                      DInputConsumer::getContentType(
                          qp->GetType(qp->GetSon(s, 1)))));

        local.addr = li =
            new dproductSLI(std::move(a), std::move(b),
                            incomingFunction->GetValue(),
                            remoteName,
                            Task::resultType(nl->Second(qp->GetType(s))),
                            isRel, isStream);

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

1.4 Specification for dproductS 

*/

OperatorSpec dproductSSpec(
    "d[f]array(rel(X))/tasks(d[f]array(rel(X))) x "
    "d[f]array(rel(Y))/tasks(d[f]array(rel(Y))) x "
    "string x (frel(X) x fsrel(Y) -> Z) -> tasks(d[f]array(Z))",
    "_ _ dproductS[_,_]",
    "Creates a stream of tasks",
    "");

/*

1.5 Operator dproductS 

*/
Operator dproductSOp(
    "dproductS",
    dproductSSpec.getStr(),
    dproductSVM,
    Operator::SimpleSelect,
    dproductSTM);
} // namespace distributed5
