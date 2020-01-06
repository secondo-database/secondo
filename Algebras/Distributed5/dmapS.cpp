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

#include "dmapS.h"
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

1 dmapS Operator

Creates a stream of tasks. 
Input Parameter can be

  * a stream of tasks
  
  * D[F]Array

1.1 Type Mapping 

*/

template <int x>
ListExpr dmapSTM(ListExpr args)
{
    string err =
        "{{d[f]array(X) / stream(task(d[f]array(X)))} x string x fun} expected";

    //ensure that exactly 2 + x argument comes into dmapS
    if (!nl->HasLength(args, 2 + x))
    {
        return listutils::typeError(err + " (wrong number of args)");
    }

    ListExpr argInputs[x];
    for (int i = 0; i < x; i++)
    {
        argInputs[i] = nl->First(args);
        args = nl->Rest(args);

        // check for internal correctness
        if (!nl->HasLength(argInputs[i], 2))
        {
            return listutils::typeError("internal error");
        }
    }
    ListExpr argName = nl->First(args);
    ListExpr argFun = nl->Second(args);

    // check for internal correctness (uses Args in type mapping)
    if (!nl->HasLength(argName, 2) ||
        !nl->HasLength(argFun, 2))
    {
        return listutils::typeError("internal error");
    }

    ListExpr argInputType[x];
    ListExpr appendValues = 0;
    ListExpr lastAppendValue;
    for (int i = 0; i < x; i++)
    {
        argInputType[i] = nl->First(argInputs[i]);

        // i.e. argInputType[i] =
        //   (darray int) or
        //   (dfarray int) or
        //   (stream (task (darray int))) or
        //   (stream (task (dfarray int)))
        bool inputIsDArray = DArray::checkType(argInputType[i]);
        bool inputIsDFArray = DFArray::checkType(argInputType[i]);
        bool inputIsDTaskStream = Stream<Task>::checkType(argInputType[i]) &&
                                  DArray::checkType(
                                      Task::innerType(
                                          nl->Second(argInputType[i])));
        bool inputIsDFTaskStream = Stream<Task>::checkType(argInputType[i]) &&
                                   DFArray::checkType(
                                       Task::innerType(
                                           nl->Second(argInputType[i])));
        bool inputIsStream = inputIsDTaskStream || inputIsDFTaskStream;
        if (appendValues == 0)
        {
            appendValues = nl->OneElemList(nl->BoolAtom(inputIsStream));
            lastAppendValue = appendValues;
        }
        else
        {
            lastAppendValue = nl->Append(lastAppendValue,
                                         nl->BoolAtom(inputIsStream));
        }
        bool inputOk = inputIsDArray ||
                       inputIsDFArray ||
                       inputIsDTaskStream ||
                       inputIsDFTaskStream;
        if (!inputOk)
        {
            return listutils::typeError(err);
        }
    }
    ListExpr argNameType = nl->First(argName);
    ListExpr argFunType = nl->First(argFun);

    if (
        !CcString::checkType(argNameType) ||
        !listutils::isAnyMap(argFunType) ||
        !nl->HasLength(argFunType, x + 2))
    {
        return listutils::typeError(err);
    }

    ListExpr funArgs[x];
    ListExpr expFunArgs[x];

    ListExpr argFunTypeArgs = nl->Rest(argFunType);

    for (int i = 0; i < x; i++)
    {
        //Function argument type
        funArgs[i] = nl->First(argFunTypeArgs);
        argFunTypeArgs = nl->Rest(argFunTypeArgs);

        ListExpr darrayType = Stream<Task>::checkType(argInputType[i])
                                  ? Task::innerType(nl->Second(argInputType[i]))
                                  : argInputType[i];

        //expected Function Argument type
        if (DFArray::checkType(darrayType))
        {
            expFunArgs[i] = nl->TwoElemList(
                listutils::basicSymbol<frel>(),
                nl->Second(nl->Second(darrayType)));
        }
        else
        {
            expFunArgs[i] = nl->Second(darrayType);
        }

        if (!nl->Equal(expFunArgs[i], funArgs[i]))
        {
            stringstream ss;
            ss << "type mismatch between function argument " << (i + 1)
               << " and subtype of dfarray" << endl
               << "subtype is " << nl->ToString(expFunArgs[i]) << endl
               << "funarg is " << nl->ToString(funArgs[i]) << endl;

            return listutils::typeError(ss.str());
        }
    }

    //Function return type
    ListExpr funRes = nl->First(argFunTypeArgs);

    // the function definition
    ListExpr funq = nl->Second(argFun);

    // we have to replace the given function arguments
    // by the real function arguments because the
    // given function argument may be a TypeMapOperator
    ListExpr rfun = nl->OneElemList(nl->First(funq));
    ListExpr funqRest = nl->Rest(funq);
    ListExpr last = rfun;
    for (int i = 0; i < x; i++)
    {
        ListExpr funarg = nl->First(funqRest);
        funqRest = nl->Rest(funqRest);
        last = nl->Append(last, nl->TwoElemList(
                                    nl->First(funarg),
                                    funArgs[i]));
    }
    last = nl->Append(last, nl->First(funqRest));

    // allowed result types are streams of tuples and
    // non-stream objects
    bool isRel = Relation::checkType(funRes);
    bool isStream = Stream<Tuple>::checkType(funRes);

    if (listutils::isStream(funRes) && !isStream)
    {
        return listutils::typeError(
            "function produces a stream of non-tuples.");
    }

    // compute the subtype of the resulting array
    if (isStream)
    {
        funRes = nl->TwoElemList(
            listutils::basicSymbol<Relation>(),
            nl->Second(funRes));
    }

    // determine the result array type
    // if the origin function result is a tuple stream,
    // the result will be a dfarray, otherwise a darray
    ListExpr resultType = isStream || isRel
                              ? listutils::basicSymbol<DFArray>()
                              : listutils::basicSymbol<DArray>();

    // i.e. (stream (task (darray int)))
    ListExpr resType = nl->TwoElemList(
        listutils::basicSymbol<Stream<Task>>(),
        nl->TwoElemList(
            listutils::basicSymbol<Task>(),
            nl->TwoElemList(
                resultType,
                funRes)));

    lastAppendValue = nl->Append(lastAppendValue,
                                 nl->TextAtom(nl->ToString(rfun)));
    lastAppendValue = nl->Append(lastAppendValue,
                                 nl->BoolAtom(isRel));
    lastAppendValue = nl->Append(lastAppendValue,
                                 nl->BoolAtom(isStream));

    return nl->ThreeElemList(
        nl->SymbolAtom(Symbols::APPEND()),
        appendValues,
        resType);
}

/*

1.2 Local Information Class for the dmapS Operator

*/

template <int x>
class dmapSLI
{
public:
    // constructor of the dmapS Local Information
    // inputs is a vector of DInputConsumer
    //      (could be a d[f]array or a stream of tasks)
    // string is the Function text
    // remoteName is the name of the relation in the slots
    // isStream is a boolean - is required for the result of the dmapS
    dmapSLI(std::vector<DInputConsumer> &&inputs,
            string dmapFunction, string remoteName, ListExpr contentType,
            bool isRel, bool isStream)
        : inputs(std::move(inputs)),
          dmapFunction(dmapFunction),
          remoteName(remoteName), contentType(contentType),
          isRel(isRel), isStream(isStream)
    {
        for (int i = 0; i < x; i++)
        {
            this->inputLeafs[i] = 0;
        }
    }

    //destructor of dmapSLI
    ~dmapSLI()
    {
        for (int i = 0; i < x; i++)
        {
            if (inputLeafs[i])
            {
                delete inputLeafs[i];
                inputLeafs[i] = 0;
            }
        }
    }

    //This methode is called for analysing the incoming tasks.
    //This is done in a while loop, as long as the return value is true.
    //When the incoming task queue is empty, further tasks are requested from
    //the previous task.
    //When all previous tasks for this task are availible, the task can be
    //created and the previous tasks can be set.
    bool combineOutputTaskOrRequestFromInputStream()
    {
        for (int i = 0; i < x; i++)
        {
            if (inputLeafs[i] == 0)
            {
                DInputConsumer &input = inputs[i];
                Task *task = input.request();
                if (task)
                {
                    if (task->isLeaf())
                    {
                        task->setLeaf(false);
                        inputLeafs[i] = task;
                    }
                    outputTasks.push_back(task);
                    return true;
                }
                else
                {
                    return false;
                }
            }
        }
        //create new leaf task for this operator
        Task *a = new Task(TaskType::Function_DMAPSX, dmapFunction,
                           remoteName, contentType, isRel, isStream);
        a->setLeaf(true);

        //check is all previous tasks for this operator are availible
        for (int i = 0; i < x; i++)
        {
            Task *t = inputLeafs[i];
            inputLeafs[i] = 0;
            a->addPredecessorTask(t);
        }
        outputTasks.push_back(a);
        return true;
    }

    //returns the next task for the successor operator
    Task *getNext()
    {
        while (outputTasks.empty())
        {
            if (!combineOutputTaskOrRequestFromInputStream())
            {
                return 0;
            }
        }

        Task *res = outputTasks.front();
        outputTasks.pop_front();

        return res;
    }

private:
    std::deque<Task *> outputTasks;
    std::vector<DInputConsumer> inputs;
    Task *inputLeafs[x];
    string dmapFunction;
    string remoteName;
    ListExpr contentType;
    bool isRel;
    bool isStream;
};

/*

1.3 Value Mapping for dmapS

*/
template <int x>
int dmapSVM(Word *args,
            Word &result,
            int message,
            Word &local,
            Supplier s)
{

    dmapSLI<x> *li = (dmapSLI<x> *)local.addr;

    switch (message)
    {
    case OPEN:
    {
        if (li)
        {
            delete li;
        }
        // Arguments are:
        // a, b, c, remoteName, fn*, aIsStream, bIsStream, cIsStream,
        // |--x--|                   |--------------x--------------|
        // functionText, isRel, isStream
        CcString *incomingRemoteName = (CcString *)args[x].addr;
        FText *incomingFunction = (FText *)args[2 * x + 2].addr;
        bool isRel = ((CcBool *)args[2 * x + 3].addr)->GetValue();
        bool isStream = ((CcBool *)args[2 * x + 4].addr)->GetValue();

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
        std::vector<DInputConsumer> inputs;
        for (int i = 0; i < x; i++)
        {
            bool isStream = ((CcBool *)args[x + 2 + i].addr)->GetValue();
            if (isStream)
            {
                inputs.push_back(DInputConsumer(args[i]));
            }
            else
            {
                DArrayBase *incomingDArray = (DArrayBase *)args[i].addr;
                inputs.push_back(DInputConsumer(
                    incomingDArray,
                    nl->Second(qp->GetType(qp->GetSon(s, i)))));
            }
        }

        local.addr = li =
            new dmapSLI<x>(std::move(inputs),
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

OperatorSpec dmapSSpec(
    "d[f]array(X)/tasks(d[f]array(X)) x string x fun -> tasks(d[f]array(Y))",
    "_ dmapS[_,_]",
    "Creates a stream of tasks",
    "");

Operator dmapSOp(
    "dmapS",
    dmapSSpec.getStr(),
    dmapSVM<1>,
    Operator::SimpleSelect,
    dmapSTM<1>);

Operator dmapS2Op(
    "dmapS2",
    dmapSSpec.getStr(),
    dmapSVM<2>,
    Operator::SimpleSelect,
    dmapSTM<2>);

Operator dmapS3Op(
    "dmapS3",
    dmapSSpec.getStr(),
    dmapSVM<3>,
    Operator::SimpleSelect,
    dmapSTM<3>);

Operator dmapS4Op(
    "dmapS4",
    dmapSSpec.getStr(),
    dmapSVM<4>,
    Operator::SimpleSelect,
    dmapSTM<4>);

Operator dmapS5Op(
    "dmapS5",
    dmapSSpec.getStr(),
    dmapSVM<5>,
    Operator::SimpleSelect,
    dmapSTM<5>);
} // namespace distributed5
