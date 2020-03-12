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

#include "tasks2tuples.h"

using namespace std;
using namespace distributed2;

namespace distributed5
{

/*

1 tasks2tuples Operator

Converts a stream of tasks to tuples.

1.1 Type Mapping

*/

ListExpr tasks2tuplesTM(ListExpr args)
{
    string err = "stream(tasks({d[f]array, dfmatrix})) expected";

    //ensure that exactly 1 argument comes into schedule
    if (!nl->HasLength(args, 1))
    {
        return listutils::typeError(err + " (wrong number of arguments)");
    }

    //ensure that there comes a Task Stream
    ListExpr arg1Type = nl->First(args);
    if (!Stream<Task>::checkType(arg1Type))
    {
        return listutils::typeError(err + " (tasks expected)");
    }

    ListExpr attrList =
        nl->FiveElemList(
            nl->TwoElemList(nl->SymbolAtom("ID"),
                            listutils::basicSymbol<FText>()),
            nl->TwoElemList(nl->SymbolAtom("TaskType"),
                            listutils::basicSymbol<FText>()),
            nl->TwoElemList(nl->SymbolAtom("TaskInfo"),
                            listutils::basicSymbol<FText>()),
            nl->TwoElemList(nl->SymbolAtom("Flags"),
                            listutils::basicSymbol<FText>()),
            nl->TwoElemList(nl->SymbolAtom("PredecessorTasks"),
                            listutils::basicSymbol<FText>()));

    return Stream<Tuple>::wrap(Tuple::wrap(attrList));
}

/*

1.2 Class for Local Information Service

*/
class tasks2tuplesLI
{
public:
    //constructor for tasks2tuplesLi
    tasks2tuplesLI(Word arg, ListExpr tupleTypeExpr)
        : stream(arg),
          tupleType(new TupleType(tupleTypeExpr))
    {
        stream.open();
    }

    //destructor
    ~tasks2tuplesLI()
    {
        stream.close();
        tupleType->DeleteIfAllowed();
    }

    //Operator for getting the next Task of the stream
    Tuple *getNext()
    {
        Task *task = stream.request();
        if (task == 0)
            return 0;
        Tuple *tuple = new Tuple(tupleType);

        string flags = "";

        flags += task->hasFlag(Output) ? "Output" : "Intermediate";
        if (task->hasFlag(VerticalSlot))
            flags += ", VerticalSlot";
        if (task->hasFlag(CopyArguments))
            flags += ", CopyArguments";
        if (task->hasFlag(ConvertArguments))
            flags += ", ConvertArguments";
        if (task->hasFlag(RunOnPreferedWorker))
            flags += ", RunOnPreferedWorker";
        if (task->hasFlag(RunOnPreferedServer))
            flags += ", RunOnPreferedServer";
        if (task->hasFlag(RunOnReceive))
            flags += ", RunOnReceive";
        if (task->hasFlag(PreferSlotWorker))
            flags += ", PreferSlotWorker";
        if (task->hasFlag(PreferSlotServer))
            flags += ", PreferSlotServer";

        tuple->PutAttribute(0, new FText(true, std::to_string(task->getId())));
        tuple->PutAttribute(1, new FText(true, task->getTaskType()));
        tuple->PutAttribute(2, new FText(true, task->toString()));
        tuple->PutAttribute(3, new FText(true, flags));

        string listOfPreString = "";

        std::vector<Task *> listOfPre = task->getPredecessors();
        for (size_t i = 0; i < listOfPre.size(); i++)
        {
            listOfPreString = listOfPreString.append(
                std::to_string(listOfPre[i]->getId()) + " ");
        }
        tuple->PutAttribute(4, new FText(true, listOfPreString));

        return tuple;
    }

private:
    Stream<Task> stream;
    TupleType *tupleType;
};

/*

1.2 Value Mapping for tasks2tuples 

*/
int tasks2tuplesVM(Word *args, Word &result, int message,
                   Word &local, Supplier s)
{
    tasks2tuplesLI *li = (tasks2tuplesLI *)local.addr;
    switch (message)
    {
    case OPEN:
    {
        if (li)
        {
            delete li;
        }

        ListExpr tupleType = nl->Second(GetTupleResultType(s));
        local.addr = li = new tasks2tuplesLI(args[0], tupleType);
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

1.3 Specification for tasks2tuples 

*/
OperatorSpec tasks2tuplesSpec(
    "stream(task(X)) -> tuples",
    "_ tasks2tuples",
    "Makes tuples from a task stream.",
    "query CabsId dmapS["
    ", . feed filter[.Id = 1039] project[Id] consume] tasks2tuples consume");

/*

1.4 Operator tasks2tuples 

*/
Operator tasks2tuplesOp(
    "tasks2tuples",
    tasks2tuplesSpec.getStr(),
    tasks2tuplesVM,
    Operator::SimpleSelect,
    tasks2tuplesTM);

} //end namespace distributed5
