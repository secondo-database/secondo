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

ListExpr tasks2tuplesTM(ListExpr args)
{
    string err = "stream(tasks(darray)) expected";

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

    ListExpr attrList = nl->SixElemList(
        nl->TwoElemList(nl->SymbolAtom("ID"),
                        listutils::basicSymbol<FText>()),
        nl->TwoElemList(nl->SymbolAtom("Task Type"),
                        listutils::basicSymbol<FText>()),
        nl->TwoElemList(nl->SymbolAtom("Task Function"),
                        listutils::basicSymbol<FText>()),
        nl->TwoElemList(nl->SymbolAtom("Worker"),
                        listutils::basicSymbol<FText>()),
        nl->TwoElemList(nl->SymbolAtom("List of Successors Tasks"),
                        listutils::basicSymbol<FText>()),
        nl->TwoElemList(nl->SymbolAtom("List of Predecessor Tasks"),
                        listutils::basicSymbol<FText>()));

    return Stream<Tuple>::wrap(Tuple::wrap(attrList));
}

class tasks2tuplesLI
{
public:
    tasks2tuplesLI(Word arg, ListExpr tupleTypeExpr)
        : stream(arg),
          tupleType(new TupleType(tupleTypeExpr))
    {
        stream.open();
    }

    ~tasks2tuplesLI()
    {
        stream.close();
        tupleType->DeleteIfAllowed();
    }

    Tuple *getNext()
    {
        Task *task = stream.request();
        if (task == 0)
            return 0;
        Tuple *tuple = new Tuple(tupleType);

        TaskType taskType = task->getTaskType();

        tuple->PutAttribute(0, new FText(true, std::to_string(task->getId())));

        switch (taskType)
        {
        case TaskType::Data:
            tuple->PutAttribute(1, new FText(true, "Data Task"));
            tuple->PutAttribute(2, new FText(true, ""));
            tuple->PutAttribute(
                3,
                new FText(true, std::to_string(task->getWorker())));
            break;
        case TaskType::Function:
            tuple->PutAttribute(1, new FText(true, "Function Task"));
            tuple->PutAttribute(2, new FText(true, task->getFunction()));
            tuple->PutAttribute(
                3,
                new FText(true, std::to_string(task->getWorker())));
            break;
        case TaskType::Error:
            tuple->PutAttribute(1, new FText(true, "Error Task"));
            tuple->PutAttribute(2, new FText(true, ""));
            tuple->PutAttribute(3, new FText(true, ""));
            break;
        default:
            tuple->PutAttribute(1, new FText(true, ""));
            tuple->PutAttribute(2, new FText(true, ""));
            tuple->PutAttribute(3, new FText(true, ""));
        }

        string listOfSuccString = "";

        std::vector<Task *> listOfSucc = task->getSuccessors();
        for (size_t i = 0; i < listOfSucc.size(); i++)
        {
            listOfSuccString = listOfSuccString.append(
                std::to_string(listOfSucc[i]->getId()) + " ");
        }
        tuple->PutAttribute(4, new FText(true, listOfSuccString));

        string listOfPreString = "";

        std::vector<Task *> listOfPre = task->getPredecessor();
        for (size_t i = 0; i < listOfPre.size(); i++)
        {
            listOfPreString = listOfPreString.append(
                std::to_string(listOfPre[i]->getId()) + " ");
        }
        tuple->PutAttribute(5, new FText(true, listOfPreString));

        return tuple;
    }

private:
    Stream<Task> stream;
    TupleType *tupleType;
};

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

OperatorSpec tasks2tuplesSpec(
    "task -> tuples",
    "_ tasks2tuples",
    "Makes tuples from a task stream.",
    "query CabsId dmapS["
    ", . feed filter[.Id = 1039] project[Id] consume] tasks2tuples consume");

Operator tasks2tuplesOp(
    "tasks2tuples",
    tasks2tuplesSpec.getStr(),
    tasks2tuplesVM,
    Operator::SimpleSelect,
    tasks2tuplesTM);

} //end namespace distributed5
