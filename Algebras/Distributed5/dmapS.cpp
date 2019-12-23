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
Input Parameter can be a stream of tasks coming from a previous dmap\_S Operator or a DArray.

1.1 Type Mapping 

*/

template <int x>
ListExpr dmapSTM(ListExpr args)
{
    string err =
        "{{darray(X) / stream(task(darray(X)))}  x string x fun} expected";

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

    ListExpr resultType;

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
        if (i == 0)
        {
            resultType = inputIsDArray || inputIsDTaskStream
                             ? listutils::basicSymbol<DArray>()
                             : listutils::basicSymbol<DFArray>();
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

        //expected Function Argument type
        expFunArgs[i] = Stream<Task>::checkType(argInputType[i])
                            ? Task::resultType(nl->Second(argInputType[i]))
                            : nl->Second(argInputType[i]);

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

1.1 Local Information Class for the dmap\_S Operator

*/

template <int x>
class dmapSLI
{
public:
    //constructor of the dmapS Local Information
    //string is the Function text
    //remoteName is the name of the relation in the slots
    dmapSLI(string dmapFunction, string remoteName, bool isRel)
    {
        for (int i = 0; i < x; i++)
        {
            this->inputStreams[i] = 0;
        }
        this->dmapFunction = dmapFunction;
        this->remoteName = remoteName;
        this->isRel = isRel;
    }

    ~dmapSLI()
    {
        for (int i = 0; i < x; i++)
        {
            if (inputStreams[i])
            {
                inputStreams[i]->close();
                delete inputStreams[i];
                inputStreams[i] = 0;
            }
        }
    }

    void setStream(Word stream, int inputIndex)
    {
        int i = inputIndex;
        this->inputStreams[i] = new Stream<Task>(stream);
        this->inputStreams[i]->open();
    }

    //adds the task t to the list of all tasks
    //manages also the predecessor and successor tasks of this tasks
    //if task is not a leaf (so there are some other
    // dmapS operators in between),
    //the task can just be forwarded.
    //if task is the leaf, the next incoming task is the successor of the
    // task and visa versa.
    void addTask(Task *t, int inputIndex)
    {
        if (!t->isLeaf())
        {
            outputTasks.push_back(t);
        }
        else
        {
            inputLeafQueues[inputIndex].push_back(t);
        }
    }

    // TODO
    bool combineOutputTaskOrRequestFromInputStream()
    {
        for (int i = 0; i < x; i++)
        {
            if (inputLeafQueues[i].empty())
            {
                Stream<Task> *stream = inputStreams[i];
                if (stream == 0)
                {
                    return false;
                }
                Task *task = stream->request();
                if (task)
                {
                    addTask(task, i);
                    return true;
                }
                else
                {
                    stream->close();
                    delete stream;
                    inputStreams[i] = 0;
                    return false;
                }
            }
        }

        Task *a = new Task(dmapFunction, remoteName, isRel);
        a->setLeaf(true);

        for (int i = 0; i < x; i++)
        {
            Task *t = inputLeafQueues[i].front();
            inputLeafQueues[i].pop_front();
            t->setLeaf(false);
            t->addSuccessorTask(a);
            a->addPredecessorTask(t);
            outputTasks.push_back(t);
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
    Stream<Task> *inputStreams[x];
    std::deque<Task *> inputLeafQueues[x];
    string dmapFunction;
    string remoteName;
    bool isRel;
};

/*

1.2 Value Mapping for dmap


dmap\_S Value Mapping

*/
template <int x>
int dmapSVM(Word *args,
            Word &result,
            int message,
            Word &local,
            Supplier s)
{

    dmapSLI<x> *li = (dmapSLI<x> *)local.addr;

    FText *incomingFunction;
    CcString *incomingRemoteName;
    string remoteName;
    bool isRel;
    int i;

    switch (message)
    {
    case OPEN:
        if (li)
        {
            delete li;
        }
        // Arguments are:
        // a, b, c, remoteName, fn*, aIsStream, bIsStream, cIsStream,
        // |--x--|                   |--------------x--------------|
        // functionText, isRel, isStream
        incomingRemoteName = (CcString *)args[x].addr;
        incomingFunction = (FText *)args[2 * x + 2].addr;
        isRel = ((CcBool *)args[2 * x + 3].addr)->GetValue();

        // create a new name for the result array
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

        local.addr = li =
            new dmapSLI<x>(incomingFunction->GetValue(), remoteName, isRel);

        for (i = 0; i < x; i++)
        {
            bool isStream = ((CcBool *)args[x + 2 + i].addr)->GetValue();
            if (isStream)
            {
                li->setStream(args[i], i);
            }
            else
            {
                DArrayBase *incomingDArray = (DArrayBase *)args[i].addr;
                for (size_t j = 0; j < incomingDArray->getSize(); j++)
                {
                    //create list of Data Tasks
                    li->addTask(
                        new Task(
                            incomingDArray->getWorkerForSlot(j),
                            incomingDArray->getName(),
                            j,
                            incomingDArray->getType(),
                            qp->GetType(qp->GetSon(s, i))),
                        i);
                }
            }
        }

        return 0;

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
    "darray(X)/tasks(darray(X)) x string x fun -> tasks(darray(Y))",
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
