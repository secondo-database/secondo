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

    ListExpr attrList = nl->Cons(
        nl->TwoElemList(nl->SymbolAtom("ID"),
                        listutils::basicSymbol<FText>()),
        nl->Cons(
            nl->TwoElemList(nl->SymbolAtom("TaskType"),
                            listutils::basicSymbol<FText>()),
            nl->FiveElemList(
                nl->TwoElemList(nl->SymbolAtom("TaskFunction"),
                                listutils::basicSymbol<FText>()),
                nl->TwoElemList(nl->SymbolAtom("Worker"),
                                listutils::basicSymbol<FText>()),
                nl->TwoElemList(nl->SymbolAtom("Location"),
                                listutils::basicSymbol<FText>()),
                nl->TwoElemList(nl->SymbolAtom("ContentType"),
                                listutils::basicSymbol<FText>()),
                nl->TwoElemList(nl->SymbolAtom("PredecessorTasks"),
                                listutils::basicSymbol<FText>()))));

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

        TaskType taskType = task->getTaskType();

        tuple->PutAttribute(0, new FText(true, std::to_string(task->getId())));

        switch (taskType)
        {
        case TaskType::Data:
        {
            std::string storageType = "?";
            switch (task->getStorageType())
            {
            case DataStorageType::Object:
                storageType = "object";
                break;
            case DataStorageType::File:
                storageType = "file";
                break;
            }
            tuple->PutAttribute(1, new FText(true, "Data Task"));
            tuple->PutAttribute(2, new FText(true, "N/A"));
            tuple->PutAttribute(
                3,
                new FText(true, task->getServer() + ":" +
                                    std::to_string(task->getPort()) + " @ " +
                                    std::to_string(task->getWorker())));
            tuple->PutAttribute(
                4,
                new FText(true, task->getName() + " @ " +
                                    std::to_string(task->getSlot()) + " as " +
                                    storageType));
            tuple->PutAttribute(
                5,
                new FText(true, nl->ToString(task->getContentType())));
            break;
        }
        case TaskType::Function_DMAPSX:
            tuple->PutAttribute(1, new FText(true, "Function DMAPSX Task"));
            tuple->PutAttribute(2, new FText(true, task->getFunction()));
            tuple->PutAttribute(3, new FText(true, "N/A"));
            tuple->PutAttribute(4, new FText(true, "N/A"));
            tuple->PutAttribute(5, new FText(true, "N/A"));
            break;
        case TaskType::Function_DPRODUCT:
            tuple->PutAttribute(1, new FText(true, "Function DPRODUCT Task"));
            tuple->PutAttribute(2, new FText(true, task->getFunction()));
            tuple->PutAttribute(3, new FText(true, "N/A"));
            tuple->PutAttribute(4, new FText(true, "N/A"));
            tuple->PutAttribute(5, new FText(true, "N/A"));
            break;
        case TaskType::PrepareDataForCopy:
            tuple->PutAttribute(1, new FText(true, "Prepare for Copy Task"));
            tuple->PutAttribute(2, new FText(true, "N/A"));
            tuple->PutAttribute(3, new FText(true, "N/A"));
            tuple->PutAttribute(4, new FText(true, "N/A"));
            tuple->PutAttribute(5, new FText(true, "N/A"));
            break;
        case TaskType::CopyData:
            tuple->PutAttribute(1, new FText(true, "Copy Data Task"));
            tuple->PutAttribute(2, new FText(true, "N/A"));
            tuple->PutAttribute(3, new FText(true, "N/A"));
            tuple->PutAttribute(4, new FText(true, "N/A"));
            tuple->PutAttribute(5, new FText(true, "N/A"));
            break;
        case TaskType::Error:
            tuple->PutAttribute(1, new FText(true, "Error Task"));
            tuple->PutAttribute(2, new FText(true, "-"));
            tuple->PutAttribute(3, new FText(true, "-"));
            tuple->PutAttribute(4, new FText(true, "-"));
            tuple->PutAttribute(5, new FText(true, "-"));
            break;
        }

        string listOfPreString = "";

        std::vector<Task *> listOfPre = task->getPredecessor();
        for (size_t i = 0; i < listOfPre.size(); i++)
        {
            listOfPreString = listOfPreString.append(
                std::to_string(listOfPre[i]->getId()) + " ");
        }
        tuple->PutAttribute(6, new FText(true, listOfPreString));

        return tuple;
    }

private:
    Stream<Task> stream;
    TupleType *tupleType;
};

/*

1.2 Value Mapping

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
