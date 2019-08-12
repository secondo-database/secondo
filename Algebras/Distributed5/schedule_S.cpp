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

#include "schedule_S.h"

using namespace std;
using namespace distributed2;


namespace distributed5
{

/*
1 schedule\_S

The operator schedule\_S is responsible for distributing the tasks from a tuple stream to the worker.
*/

/*

1.1 Type Mapping

*/

ListExpr schedule_STM(ListExpr args)
{

    string err = "stream<tasks> expected";

    //ensure that exactly 1 argument comes into schedule
    if (!nl->HasLength(args, 1))
    {
        return listutils::typeError(err + " (wrong number of arguments)");
    }

    //ensure that there comes a Task Stream
    ListExpr arg1Type = nl->First(args);
    if (!Stream<Task>::checkType(arg1Type))
    {
        return listutils::typeError(err);
    }

    //ensure that the stream is of type Tasks
    ListExpr streamType = nl->Second(arg1Type);
    if (!Task::checkType(streamType))
    {
        return listutils::typeError(err);
    }

    //defines the result type
    ListExpr contentType = nl->Second(streamType);

    //result is a DArray of result type
    ListExpr resultType = nl->TwoElemList(
        listutils::basicSymbol<DArray>(),
        contentType);

    return resultType;
}

/*

1.2 Value Mapping

*/

int schedule_SVM(Word *args, Word &result, int message,
                 Word &local, Supplier s)
{
    result = qp->ResultStorage(s);
    Stream<Task> stream(args[0]);
    stream.open();
    Task *task;
    DArray *res = (DArray *)result.addr;
    vector<DArrayElement> myResult;
    string dArrayName;
    vector<Task *> allTasksWaiting;

    //As long as there are still incoming tasks...
    while ((task = stream.request()))
    {
        //create a queue of tasks, which can possibly be started
        std::deque<Task *> allTasksToBeStarted;
        //if incoming task can be started...
        if (task->taskCanBeStarted())
        {
            //... add this task to the queue of possible tasks to start.
            allTasksToBeStarted.push_back(task);
        }
        else
        {
            //... else add this task to the queue of waiting tasks
            //which can be started later.
            allTasksWaiting.push_back(task);
        }

        //As long as there are tasks which can be started...
        while (allTasksToBeStarted.size() != 0)
        {
            //take one out and start the task...
            task = allTasksToBeStarted.back();
            allTasksToBeStarted.pop_back();
            cout << "\n SCHEDULE::" << task->toString() << "\n";
            task->run();
            //For all successor task, decrease the number of remaining tasks
            task->decNumberOfRemainingTasksForSuccessors();
            //check if there are now possible tasks which can be started.
            //If that is the case, check, if this task came already
            //in the schedule_S stream.
            //If that is the case, add this task to the queue of task,
            //which can possibly be started.
            //If that is not the case - the task will came in later and
            // will be checked if it can be started.
            //This is needed, because the tasks themselve are connected.
            //So it can be, that a task is finished, and because they
            //are connected over their successor list, a task should be started,
            //but the task has not came into the schedule operator.
            vector<Task *> tasksPossibleToStart =
                task->checkSuccessorsTasksToStart();
            for (size_t i = 0; i < tasksPossibleToStart.size(); i++)
            {
                std::vector<Task *>::iterator foundPlace =
                    std::find(allTasksWaiting.begin(), allTasksWaiting.end(),
                              tasksPossibleToStart[i]);

                if (foundPlace != allTasksWaiting.end())
                {
                    allTasksToBeStarted.push_back(tasksPossibleToStart[i]);
                    allTasksWaiting.erase(foundPlace);
                }
            }
            //If the task is a leaf the result of the 
            //schedule_S operator is the result of the query for this slot.
            if (task->isLeaf())
            {
                TaskType tt = task->getTaskType();
                if (tt == TaskType::Error)
                {
                    //ToDo Add Errorhandling
                    dArrayName = "Error";
                }
                else
                {
                    dArrayName = task->getName();
                    while (myResult.size() <= task->getSlot())
                    {
                        myResult.push_back(DArrayElement("", 0, 0, ""));
                    }
                    myResult[task->getSlot()] = task->GetDArrayElement();
                }
            }
        }
    }

    res->set(dArrayName, myResult);
    stream.close();

    return 0;
}

OperatorSpec schedule_SSpec(
    "tasks -> darray",
    "_ schedule_S",
    "Computes the result of the query.",
    "");

Operator schedule_SOp(
    "schedule_S",
    schedule_SSpec.getStr(),
    schedule_SVM,
    Operator::SimpleSelect,
    schedule_STM);
} // namespace distributed5
