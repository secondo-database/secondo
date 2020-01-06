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

#include "schedule.h"

using namespace std;
using namespace distributed2;

namespace distributed5
{

/*
1 schedule Operator

The operator schedule is responsible for distributing the tasks from a tuple 
stream to the worker.


1.1 Type Mapping

Type Mapping for the schedule Operator.

*/

ListExpr scheduleTM(ListExpr args)
{

    string err = "stream(task(d[f]array), int) expected";

    //ensure that exactly 2 arguments comes into schedule
    if (!nl->HasLength(args, 2))
    {
        return listutils::typeError(err + " (wrong number of arguments)");
    }

    //ensure that there comes a Task Stream
    ListExpr arg1Type = nl->First(args);
    if (!Stream<Task>::checkType(arg1Type))
    {
        return listutils::typeError(err + " (tasks expected)");
    }

    //ensure that the stream is of type Tasks
    ListExpr taskType = Task::innerType(nl->Second(arg1Type));
    if (!(DArray::checkType(taskType) || DFArray::checkType(taskType)))
    {
        return listutils::typeError(err + " (d[f]array expected)");
    }

    // ensure that second argument is a port number
    ListExpr arg2Type = nl->Second(args);
    if (!CcInt::checkType(arg2Type))
    {
        return listutils::typeError(err + " (port number expected)");
    }

    return taskType;
}

struct TaskScheduleInfo
{
public:
    bool completed = false;
    int incompletePrecessors = 0;
    std::vector<Task *> successors;
};

class Scheduler
{
public:
    Scheduler(int fileTransferPort) : context(fileTransferPort) {}

    //joins all running threads
    void join()
    {
        mutex.lock();
        while (!allThreads.empty())
        {

            boost::thread *a = allThreads.front();
            allThreads.pop_front();
            mutex.unlock();
            a->join();
            delete a;
            mutex.lock();
        }

        mutex.unlock();
    }

    //When a new task is recieved
    //this methode checks if the task can be started or
    //has to be added to the queue of waiting tasks
    void receiveTask(Task *task)
    {
        boost::lock_guard<boost::recursive_mutex> lock(mutex);

        // update task schedule info structures
        TaskScheduleInfo &info = taskInfo[task];
        std::vector<Task *> &predecessors = task->getPredecessor();
        for (auto it = predecessors.begin(); it != predecessors.end(); it++)
        {
            TaskScheduleInfo &preInfo = taskInfo[*it];
            if (!preInfo.completed)
            {
                info.incompletePrecessors++;
                preInfo.successors.push_back(task);
            }
        }

        //if incoming task can be started...
        if (info.incompletePrecessors == 0)
        {
            scheduleTask(task);
        }
    }

    vector<DArrayElement> myResult;
    string dArrayName;

private:
    //schedules the task
    void scheduleTask(Task *task)
    {
        allThreads.push_back(
            new boost::thread(
                boost::bind(
                    &Scheduler::runTask, this, task)));
    }

    //executes the task
    void runTask(Task *task)
    {
        task->run(context);
        boost::lock_guard<boost::recursive_mutex> lock(mutex);

        TaskScheduleInfo &info = taskInfo[task];
        info.completed = true;
        //For all successor task, decrease the number of remaining tasks
        std::vector<Task *> &successors = info.successors;
        for (auto it = successors.begin(); it != successors.end(); it++)
        {
            Task *succTask = *it;
            TaskScheduleInfo &succInfo = taskInfo[succTask];
            succInfo.incompletePrecessors--;

            //check if there are now possible tasks which can be started.
            if (succInfo.incompletePrecessors == 0)
            {
                scheduleTask(succTask);
            }
        }
        //If the task is a leaf the result of the
        //schedule operator is the result of the query for this slot.
        if (task->isLeaf())
        {
            TaskType tt = task->getTaskType();
            if (tt == TaskType::Error)
            {
                //ToDo Add Errorhandling
                cout << "Leaf task at slot " << task->getSlot() << " is Error";
                dArrayName = "Error";
            }
            else
            {
                dArrayName = task->getName();
                while (myResult.size() <= task->getSlot())
                {
                    myResult.push_back(DArrayElement("", 0, 0, ""));
                }
                // TODO Error when already set (multiple leaf tasks)
                DArrayElement &resultItem = myResult[task->getSlot()];
                if (resultItem.getHost() != "")
                {
                    cout << "Multiple leaf tasks for slot " << task->getSlot()
                         << endl;
                }
                resultItem = task->GetDArrayElement();
            }
        }
    }
    deque<boost::thread *> allThreads;
    boost::recursive_mutex mutex;
    TaskExecutionContext context;

    std::map<Task *, TaskScheduleInfo> taskInfo;
};

/*

1.2 Value Mapping

Value Mapping for schedule Operator.

*/

int scheduleVM(Word *args, Word &result, int message,
               Word &local, Supplier s)
{
    result = qp->ResultStorage(s);
    Stream<Task> stream(args[0]);
    stream.open();
    int port = ((CcInt *)args[1].addr)->GetValue();
    Task *task;
    DArrayBase *res = (DArrayBase *)result.addr;
    Scheduler scheduler(port);

    while ((task = stream.request()) != 0)
    {
        scheduler.receiveTask(task);
    }

    scheduler.join();
    // TODO delete all tasks
    res->set(scheduler.dArrayName, scheduler.myResult);
    stream.close();

    return 0;
}

OperatorSpec scheduleSpec(
    "tasks(darray(X), int) -> darray(X)",
    "_ schedule[_]",
    "Computes the result of the query.",
    "");

Operator scheduleOp(
    "schedule",
    scheduleSpec.getStr(),
    scheduleVM,
    Operator::SimpleSelect,
    scheduleTM);
} // namespace distributed5
