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

#include "dmap_S.h"

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

1 dmap\_S Operator

Creates a stream of tasks. 
Input Parameter can be a stream of tasks coming from a previous dmap\_S Operator or a DArray.

1.1 Type Mapping 

*/

ListExpr dmap_STM(ListExpr args)
{
    string err = "{{darray(X)}  x string x fun} expected";

    //ensure that exactly 3 argument comes into dmap_S
    if (!nl->HasLength(args, 3))
    {
        return listutils::typeError(err + " (wrong number of args)");
    }

    // check for internal correctness (uses Args in type mapping)
    if (!nl->HasLength(nl->First(args), 2) 
        || !nl->HasLength(nl->Second(args), 2) 
        || !nl->HasLength(nl->Third(args), 2))
    {
        return listutils::typeError("internal error");
    }

    ListExpr arg1Type = nl->First(nl->First(args));
    ListExpr arg2Type = nl->First(nl->Second(args));
    ListExpr arg3Type = nl->First(nl->Third(args));

    bool arg1Ok = DArray::checkType(arg1Type) 
    || (Stream<Task>::checkType(arg1Type) 
    && Task::checkType(nl->Second(arg1Type)));

    if (
        !arg1Ok ||
        !CcString::checkType(arg2Type) ||
        !listutils::isMap<1>(arg3Type))
    {
        return listutils::typeError(err);
    }

    //Function argument type
    ListExpr funArg = nl->Second(arg3Type);
    //Function return type
    ListExpr funRes = nl->Third(arg3Type);
    //expected Function Argument type
    ListExpr expFunArg = Stream<Task>::checkType(arg1Type)
                             ? nl->Second(nl->Second(arg1Type))
                             : nl->Second(arg1Type);

    if (!nl->Equal(expFunArg, funArg))
    {
        stringstream ss;
        ss << "type mismatch between function argument and "
           << " subtype of dfarray" << endl
           << "subtype is " << nl->ToString(expFunArg) << endl
           << "funarg is " << nl->ToString(funArg) << endl;

        return listutils::typeError(ss.str());
    }

    // the function definition
    ListExpr funq = nl->Second(nl->Third(args));

    // we have to replace the given function arguments
    // by the real function arguments because the
    // given function argument may be a TypeMapOperator
    ListExpr funargs = nl->Second(funq);
    ListExpr rfunargs = nl->TwoElemList(
        nl->First(funargs),
        funArg);

    ListExpr rfun = nl->ThreeElemList(
        nl->First(funq),
        rfunargs,
        nl->Third(funq));

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
    // ----We only support DARRAY---
    ListExpr resType = nl->TwoElemList(
        listutils::basicSymbol<Stream<Task>>(),
        nl->TwoElemList(
            listutils::basicSymbol<Task>(),
            funRes));

    return nl->ThreeElemList(
        nl->SymbolAtom(Symbols::APPEND()),
        nl->ThreeElemList(nl->TextAtom(nl->ToString(rfun)),
                          nl->BoolAtom(isRel),
                          nl->BoolAtom(isStream)),
        resType);
}

/*

1.1 Local Information Class for the dmap\_S Operator

*/

class dmap_SLI
{
public:
    //constructor of the dmap_S Local Information
    //string is the Function text
    //remoteName is the name of the relation in the slots
    dmap_SLI(string t, string remoteName)
    {
        this->stream = 0;
        this->dmapFunction = t;
        this->remoteName = remoteName;
    }

    dmap_SLI(Word stream, string dmapFunction, string remoteName)
    {
        this->stream = new Stream<Task>(stream);
        this->dmapFunction = dmapFunction;
        this->remoteName = remoteName;
        this->stream->open();
    }

    ~dmap_SLI()
    {
        if (stream)
        {
            stream->close();
            delete stream;
            stream = 0;
        }
    }

    //adds the task t to the list of all tasks
    //manages also the predecessor and successor tasks of this tasks
    //if task is not a leaf (so there are some other
    // dmap_S operators in between), 
    //the task can just be forwarded.
    //if task is the leaf, the next incoming task is the successor of the
    // task and visa versa.
    void addTask(Task *t)
    {
        if (!t->isLeaf())
        {
            allTasks.push_back(t);
        }
        else
        {
            t->setLeaf(false);
            Task *a = new Task(dmapFunction, remoteName);
            a->setLeaf(true);
            t->addSuccessorTask(a);
            a->addPredecessorTask(t);
            allTasks.push_back(t);
            allTasks.push_back(a);
        }
    }

    //returns the next task for the successor operator
    Task *getNext()
    {
        if (allTasks.empty())
        {
            if (stream)
            {
                Task *task = stream->request();
                if (task)
                {
                    addTask(task);
                }
                else
                {
                    stream->close();
                    delete stream;
                    stream = 0;
                    return 0;
                }
            }
            else
            {
                return 0;
            }
        }

        Task *res = allTasks.front();
        allTasks.pop_front();

        return res;
    }

private:
    std::deque<Task *> allTasks;
    Stream<Task> *stream;
    string dmapFunction;
    string remoteName;
};

/*

1.2 Value Mapping for DArray


dmap\_S Value Mapping when a DARRAY comes as in put

*/
int dmap_SVM_DArray(Word *args, 
            Word &result, 
            int message, 
            Word &local, 
            Supplier s)
{

    dmap_SLI *li = (dmap_SLI *)local.addr;
    DArray *incomingDArray;

    FText *incomingFunction;
    CcString *incomingRemoteName;
    string remoteName;

    switch (message)
    {
    case OPEN:
        if (li)
        {
            delete li;
        }
        incomingDArray = (DArray *)args[0].addr;
        incomingRemoteName = (CcString *)args[1].addr;

        incomingFunction = (FText *)args[3].addr;

        // create a new name for the result array
        if (!incomingRemoteName->IsDefined() 
            || incomingRemoteName->GetValue().length() == 0)
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
            new dmap_SLI(incomingFunction->GetValue(), remoteName);

        for (size_t i = 0; i < incomingDArray->getSize(); i++)
        {
            //create list of Data Tasks
            li->addTask(
                new Task(
                    incomingDArray->getWorkerForSlot(i), 
                    incomingDArray->getName(), i));
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

/*

1.3 Value Mapping for Task Stream

dmap\_S Value Mapping when a stream of tasks comes as in put

*/
int dmap_SVM_Task(Word *args, 
                    Word &result, 
                    int message, 
                    Word &local, 
                    Supplier s)
{

    dmap_SLI *li = (dmap_SLI *)local.addr;
    Word incomingStream;

    FText *incomingFunction;
    CcString *incomingRemoteName;
    string remoteName;

    switch (message)
    {
    case OPEN:
        if (li)
        {
            delete li;
        }
        incomingStream = args[0];
        incomingRemoteName = (CcString *)args[1].addr;
        incomingFunction = (FText *)args[3].addr;

        // create a new name for the result array
        if (!incomingRemoteName->IsDefined() 
                || incomingRemoteName->GetValue().length() == 0)
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
                new dmap_SLI(incomingStream, 
                    incomingFunction->GetValue(), 
                    remoteName);
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

ValueMapping dmap_SVM[] = {
    dmap_SVM_DArray,
    dmap_SVM_Task};

//Checks which type of Value Mapping is required.  DArray vs. Task Stream
int dmap_SSelect(ListExpr args)
{
    if (DArray::checkType(nl->First(args)))
    {
        return 0;
    }
    if (
        Stream<Task>::checkType(nl->First(args)) &&
        Task::checkType(nl->Second(nl->First(args))))
    {
        return 1;
    }
    return -1;
}

OperatorSpec dmap_SSpec(
    "d[f]array/tasks(darray) x string x fun -> stream(task)",
    "_ dmap_S[_,_]",
    "Creates a stream of tasks",
    "");

Operator dmap_SOp(
    "dmap_S",
    dmap_SSpec.getStr(),
    2,
    dmap_SVM,
    dmap_SSelect,
    dmap_STM);
} // namespace distributed5
