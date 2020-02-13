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
    TaskDataItem *result;
    optional<pair<WorkerLocation, int>> reservation;
};

template <typename K>
class OnlyOnceMutexMap
{
public:
    std::optional<boost::unique_lock<boost::mutex>> check(K key)
    {
        boost::mutex *mutex;
        {
            boost::lock_guard<boost::mutex> lock(mapMutex);
            auto it = map.find(key);
            if (it == map.end())
            {
                // this creates a new mutex and locks it
                return std::optional<boost::unique_lock<boost::mutex>>(
                    std::in_place,
                    map[key]);
            }
            mutex = &it->second;
        }
        // this waits until the mutex is unlocked
        boost::lock_guard<boost::mutex> wait(*mutex);
        return std::optional<boost::unique_lock<boost::mutex>>();
    }

private:
    boost::mutex mapMutex;
    std::map<K, boost::mutex> map;
};

class Scheduler
{
public:
    Scheduler(int fileTransferPort, bool resultInObjectForm)
        : fileTransferPort(fileTransferPort),
          resultInObjectForm(resultInObjectForm) {}

    //joins all running threads
    void join()
    {
        boost::unique_lock<boost::mutex> lock(mutex);
        stopWorkers(true);
        while (runningThreads > 0)
        {
            threadSignal.wait(lock);
        }

        // all threads have finished working
        // join them to wait for cleanup
        for (auto &threadPair : threads)
        {
            auto thread = threadPair.second;
            thread->join();
            delete thread;
        }
        threads.clear();
    }

    //When a new task is recieved
    //this methode checks if the task can be started or
    //has to be added to the queue of waiting tasks
    void receiveTask(Task *task)
    {
        // Create a ResultTask for output tasks
        if (task->hasFlag(Output))
        {
            task->clearFlag(Output);
            receiveTask(task);

            ResultTask *result = new ResultTask(*this);
            result->setFlag(RunOnPreferedWorker);
            result->setFlag(CopyArguments);
            result->addPredecessorTask(task);
            receiveTask(result);

            return;
        }

        boost::lock_guard<boost::mutex> lock(mutex);

        if (task->hasFlag(RunOnReceive))
        {
            WorkerLocation emptyLocation("", 0, "", -1);
            vector<TaskDataItem *> emptyArgs;
            TaskDataItem *result = task->run(emptyLocation, emptyArgs);
            setTaskResult(task, result);
            return;
        }

        // update task schedule info structures
        TaskScheduleInfo &info = taskInfo[task];
        std::vector<Task *> &predecessors = task->getPredecessors();
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

    vector<WorkerLocation> getWorkers()
    {
        vector<WorkerLocation> workers;
        for (auto &pair : threads)
        {
            workers.push_back(pair.first);
        }
        return workers;
    }

    vector<DArrayElement> myResult;
    string dArrayName;
    bool isError = false;
    string errorMessage;

private:
    // schedules the task
    // mutex must already be locked
    void scheduleTask(Task *task)
    {
        workPool.push_back(task);
        poolSignal.notify_all();
    }

    void scheduleGarbage(TaskDataItem *data)
    {
        for (auto loc : data->getLocations())
        {
            if (loc.isTemporary())
                garbagePool.push_back(make_pair(data, loc));
        }
        poolSignal.notify_all();
    }

    // makes sure a worker thread for this location is running
    // mutex must already be locked
    void ensureWorker(const WorkerLocation &location)
    {
        auto &slot = threads[location];
        if (slot == 0)
        {
            runningThreads++;
            workerWorking++;
            slot =
                new boost::thread(
                    boost::bind(
                        &Scheduler::worker, this, WorkerLocation(location)));
        }
    }

    // mutex must already be locked
    void stopWorkers(bool waitForWorkDone)
    {
        if (waitForWorkDone)
            stopWorkersWhenWorkDone = true;
        else
            killWorkers = true;
        poolSignal.notify_all();
    }

    void ensureFileTransferrator(WorkerLocation &location)
    {
        auto lock = fileTransferrators.check(location.getServer());
        if (lock)
        {
            string cmd = "query staticFileTransferator(" +
                         std::to_string(fileTransferPort) +
                         ",10)";
            Task::runCommand(location.getWorkerConnection(),
                             cmd, "open file transferator", false, true);
        }
    }

    class WorkerJob
    {
    public:
        WorkerJob(WorkerLocation &location, Scheduler &scheduler)
            : location(location), scheduler(scheduler) {}
        virtual ~WorkerJob() {}

        virtual bool run(boost::unique_lock<boost::mutex> &lock) = 0;

    protected:
        WorkerLocation &location;
        Scheduler &scheduler;
    };

    class ExecuteWorkerJob : public WorkerJob
    {
    public:
        ExecuteWorkerJob(WorkerLocation &location,
                         Scheduler &scheduler,
                         Task *task,
                         vector<TaskDataItem *> args,
                         list<Task *>::const_iterator workPoolIterator)
            : WorkerJob(location, scheduler),
              task(task), args(args), workPoolIterator(workPoolIterator) {}
        virtual ~ExecuteWorkerJob() {}

        virtual bool run(boost::unique_lock<boost::mutex> &lock)
        {
            // remove task from the pool
            scheduler.workPool.erase(workPoolIterator);

            lock.unlock();

            // Execute
            TaskDataItem *result;
            try
            {
                result = task->run(location, args);
            }
            catch (exception &e)
            {
                string message = string(e.what()) + "\n" +
                                 "while running " + task->toString();
                int i = 0;
                for (auto arg : args)
                {
                    message += string("\narg ") + std::to_string(i++) +
                               " = " + arg->toString();
                }
                lock.lock();
                scheduler.addError(message);
                return false;
            }

            lock.lock();

            scheduler.setTaskResult(task, result);
            return true;
        }

    private:
        Task *task;
        vector<TaskDataItem *> args;
        list<Task *>::const_iterator workPoolIterator;
    };

    class TransferDataWorkerJob : public WorkerJob
    {
    public:
        TransferDataWorkerJob(WorkerLocation &location,
                              Scheduler &scheduler,
                              Task *task,
                              int taskCost,
                              TaskDataItem *data,
                              TaskDataLocation sourceLocation)
            : WorkerJob(location, scheduler),
              task(task), taskCost(taskCost),
              data(data), sourceLocation(sourceLocation) {}
        virtual ~TransferDataWorkerJob() {}

        virtual bool run(boost::unique_lock<boost::mutex> &lock)
        {
            string sourceServer = sourceLocation.getServer();

            // set reservation
            scheduler.taskInfo[task].reservation =
                make_pair(location, taskCost);

            // set active transfer and upcoming location
            TaskDataLocation loc(location, File, true);
            data->addUpcomingLocation(loc);
            scheduler.activeTransferrators[sourceServer].second++;

            lock.unlock();

            try
            {
                ConnectionInfo *targetCI = location.getWorkerConnection();

                string cmd = string("query getFileTCP(") +
                             "'" + sourceLocation.getFilePath(data) + "', " +
                             "'" + sourceServer + "', " +
                             std::to_string(scheduler.fileTransferPort) + ", " +
                             "TRUE, " +
                             "'" + location.getFilePath(data) + "')";
                Task::runCommand(targetCI, cmd, "transfer file");
            }
            catch (exception &e)
            {
                lock.lock();
                scheduler.addError(string(e.what()) + "\n" +
                                   "while copying " + data->toString() +
                                   "\n - from " + sourceLocation.toString() +
                                   "\n - to " + location.toString());
                return false;
            }

            lock.lock();

            scheduler.activeTransferrators[sourceServer].second--;
            data->addLocation(loc);

            // unreserve when still reserved by this worker
            auto &reservation = scheduler.taskInfo[task].reservation;
            if (reservation && reservation->first == location)
            {
                reservation.reset();
            }
            scheduler.poolSignal.notify_all();

            return true;
        }

    private:
        Task *task;
        int taskCost;
        TaskDataItem *data;
        TaskDataLocation sourceLocation;
    };

    class ConvertToFileWorkerJob : public WorkerJob
    {
    public:
        ConvertToFileWorkerJob(WorkerLocation &location,
                               Scheduler &scheduler,
                               TaskDataItem *data)
            : WorkerJob(location, scheduler),
              data(data) {}
        virtual ~ConvertToFileWorkerJob() {}

        virtual bool run(boost::unique_lock<boost::mutex> &lock)
        {
            // set active transfer and upcoming location
            TaskDataLocation loc(location, File, true);
            data->addUpcomingLocation(loc);

            lock.unlock();

            try
            {
                ConnectionInfo *ci = location.getWorkerConnection();
                // Save Object in a file
                string oname = data->getObjectName();
                string fname = location.getFilePath(data);
                string cmd = "query " + oname +
                             " saveObjectToFile['" + fname + "']";
                Task::runCommand(ci, cmd, "save object to file");
            }
            catch (exception &e)
            {
                lock.lock();
                scheduler.addError(string(e.what()) + "\n" +
                                   "while converting " + data->toString() +
                                   " to file form on " + location.toString());
                return false;
            }

            lock.lock();

            data->addLocation(loc);

            scheduler.poolSignal.notify_all();

            return true;
        }

    private:
        TaskDataItem *data;
    };

    class WaitForTransferCompletedWorkerJob : public WorkerJob
    {
    public:
        WaitForTransferCompletedWorkerJob(WorkerLocation &location,
                                          Scheduler &scheduler)
            : WorkerJob(location, scheduler) {}
        virtual ~WaitForTransferCompletedWorkerJob() {}

        virtual bool run(boost::unique_lock<boost::mutex> &lock)
        {
            scheduler.poolSignal.wait(lock);
            return true;
        }
    };

    class RemoveDataWorkerJob : public WorkerJob
    {
    public:
        RemoveDataWorkerJob(WorkerLocation &location, Scheduler &scheduler,
                            TaskDataItem *data, TaskDataLocation dataLocation)
            : WorkerJob(location, scheduler),
              data(data), dataLocation(dataLocation) {}
        virtual ~RemoveDataWorkerJob() {}

        virtual bool run(boost::unique_lock<boost::mutex> &lock)
        {
            lock.unlock();

            try
            {
                ConnectionInfo *ci = location.getWorkerConnection();
                switch (dataLocation.getStorageType())
                {
                case File:
                {
                    string file = dataLocation.getFilePath(data);
                    Task::runCommand(
                        ci,
                        "query removeFile('" + file + "')",
                        "remove temporary data file",
                        false, true);
                    break;
                }
                case Object:
                {
                    string objectName = data->getObjectName();
                    Task::runCommand(
                        ci,
                        "delete " + objectName,
                        "remove temporary data object",
                        false, true);
                    break;
                }
                }
            }
            catch (exception &e)
            {
                lock.lock();
                scheduler.addError(string(e.what()) + "\n" +
                                   "while removing " + data->toString() +
                                   " from " + dataLocation.toString());
                return false;
            }

            lock.lock();
            return true;
        }

    private:
        TaskDataItem *data;
        TaskDataLocation dataLocation;
    };

    void
    addError(string message)
    {
        isError = true;
        if (!errorMessage.empty())
            errorMessage += "\n\n";
        errorMessage += message;
    }

    void setTaskResult(Task *task, TaskDataItem *result)
    {
        // make sure a worker is running for the location of the result
        ensureWorker(result->getFirstLocation().getWorkerLocation());

        // TODO when data with the same name has already been referenced
        // merge both data items to one item to keep only a single data item
        // per name

        // store result
        TaskScheduleInfo &info = taskInfo[task];
        info.result = result;
        info.completed = true;

        // For all precessor tasks, decrease the number of remaining tasks
        for (auto preTask : task->getPredecessors())
        {
            TaskScheduleInfo &preInfo = taskInfo[preTask];
            if (--dataReferences[preInfo.result] == 0)
            {
                scheduleGarbage(preInfo.result);
            }
        }

        // For all successor tasks, decrease the number of remaining tasks
        for (Task *succTask : info.successors)
        {
            TaskScheduleInfo &succInfo = taskInfo[succTask];
            succInfo.incompletePrecessors--;

            // each successor keeps a reference to the result
            dataReferences[result]++;

            // scheck if there are now possible tasks which can be started.
            if (succInfo.incompletePrecessors == 0)
            {
                scheduleTask(succTask);
            }
        }

        // When nobody references the result, garbagge collect it
        if (dataReferences[result] == 0)
        {
            scheduleGarbage(result);
        }
    }

    vector<TaskDataItem *> getTaskArguments(Task *task)
    {
        vector<TaskDataItem *> args;
        for (auto t : task->getPredecessors())
        {
            auto info = taskInfo[t];
            args.push_back(info.result);
        }
        return args;
    }

    // the worker thread
    void worker(WorkerLocation location)
    {
        // Connect to the worker
        location.getWorkerConnection();

        // Ensure file transferrator is open
        ensureFileTransferrator(location);

        boost::unique_lock<boost::mutex> lock(mutex);

        auto &transferrator = activeTransferrators[location.getServer()];
        if (!transferrator.first)
        {
            transferrator.first = true;
            poolSignal.notify_all();
        }

        while (true)
        {
            if (killWorkers)
                break;
            // TODO check exit
            WorkerJob *job = selectJob(location);
            if (job != 0)
            {
                if (!job->run(lock))
                {
                    stopWorkers(false);
                    break;
                }
            }
            else
            {
                // check for end of work
                if (workerWorking == 1)
                {
                    if (stopWorkersWhenWorkDone &&
                        workPool.empty() && garbagePool.empty())
                    {
                        poolSignal.notify_all();
                        break;
                    }
                }

                workerWorking--;

                // Go into sleep mode
                poolSignal.wait(lock);

                workerWorking++;
            }
        }

        workerWorking--;
        runningThreads--;
        threadSignal.notify_all();
    }

    // mutex must already be locked
    int computeTaskCost(Task *task, vector<TaskDataItem *> args,
                        WorkerLocation &location)
    {
        int cost = 0;
        int i = 0;
        for (auto arg : args)
        {
            int dist = arg->getDistance(location);
            cost += dist * CostWeightArgument;
            if (i == 0)
            {
                if (task->hasFlag(PreferSlotServer))
                {
                    if (location.getServer() !=
                        arg->getPreferredLocation().getServer())
                    {
                        cost += CostNotPreferredServer;
                    }
                }
                if (task->hasFlag(PreferSlotWorker))
                {
                    if (location != arg->getPreferredLocation())
                    {
                        cost += CostNotPreferredWorker;
                    }
                }
            }
            i++;
        }
        return cost;
    }

    static bool checkBetter(optional<pair<WorkerJob *, int>> &best, int cost)
    {
        if (!best)
            return true;
        if (best->second > cost)
        {
            delete best->first;
            best.reset();
            return true;
        }
        return false;
    }

    // mutex must already be locked
    WorkerJob *selectJob(WorkerLocation &location)
    {
        optional<pair<WorkerJob *, int>> best;
        for (auto it = workPool.begin(); it != workPool.end(); it++)
        {
            auto task = *it;
            vector<TaskDataItem *> args = getTaskArguments(task);

            int cost = computeTaskCost(task, args, location);

            bool wrongWorker =
                task->hasFlag(RunOnPreferedWorker) &&
                args.front()->getPreferredLocation() != location;
            bool wrongServer =
                task->hasFlag(RunOnPreferedServer) &&
                args.front()->getPreferredLocation().getServer() !=
                    location.getServer();
            bool forceFileArguments = task->hasFlag(FileArguments);

            bool canExecute = true;
            bool isHelping = false;

            if (wrongWorker || wrongServer || forceFileArguments)
            {
                for (auto data : args)
                {
                    if (!data->hasFileLocation(location))
                    {
                        if (forceFileArguments)
                        {
                            canExecute = false;
                        }
                        if (data->hasLocation(location))
                        {
                            if (checkBetter(best, cost))
                            {
                                best = make_pair(
                                    new ConvertToFileWorkerJob(
                                        location, *this, data),
                                    cost);
                            }
                        }
                    }
                }
            }

            if (wrongServer)
            {
                continue;
            }

            if (wrongWorker)
            {
                if (args.front()->getPreferredLocation().getServer() !=
                    location.getServer())
                {
                    continue;
                }
                isHelping = true;
            }

            // check if there is already a reservation for this task
            optional<pair<WorkerLocation, int>> reservation =
                taskInfo[task].reservation;
            if (reservation)
            {
                int resCost = reservation->second;
                // it's only allowed to seal a reservation
                // when the cost is smaller
                if (cost >= resCost)
                {
                    // When by this server reserved
                    // We can help to copy files
                    if (reservation->first.getServer() != location.getServer())
                    {
                        continue;
                    }
                    isHelping = true;
                }
                cost += CostReservation;
            }

            for (auto data : args)
            {
                if (!data->hasLocation(location))
                {
                    canExecute = false;
                    if (task->hasFlag(CopyArguments))
                    {
                        if (data->hasUpcomingLocation(location))
                        {
                            int waitingCost = cost + CostWaitingOnTransfer;
                            if (!isHelping && checkBetter(best, waitingCost))
                            {
                                best = make_pair(
                                    new WaitForTransferCompletedWorkerJob(
                                        location, *this),
                                    waitingCost);
                            }
                        }
                        else
                        {
                            try
                            {
                                auto source =
                                    data->findTransferSourceLocation(
                                        activeTransferrators);
                                int transferCost =
                                    cost +
                                    source.second * CostActiveTransfers;
                                if (checkBetter(best, transferCost))
                                {
                                    best = make_pair(
                                        new TransferDataWorkerJob(
                                            location, *this,
                                            task, cost, data, source.first),
                                        transferCost);
                                }
                            }
                            catch (NoSourceLocationException &)
                            {
                                // this can happen when file transferrator is
                                // not ready yet, or data is in object form,
                                // skip this task for now,
                                // as it can't be transferred
                            }
                        }
                    }
                }
            }

            if (canExecute && !isHelping)
            {
                if (checkBetter(best, cost))
                {
                    best = make_pair(
                        new ExecuteWorkerJob(location, *this, task, args, it),
                        cost);
                }
            }
        }

        if (best)
            return best->first;

        // nothing productive to do
        // collect some garbage
        for (auto it = garbagePool.begin(); it != garbagePool.end(); it++)
        {
            auto data = *it;
            if (data.second.getWorkerLocation() == location)
            {
                garbagePool.erase(it);
                return new RemoveDataWorkerJob(location, *this,
                                               data.first, data.second);
            }
        }

        return 0;
    }

    class ResultTask : public Task
    {
    public:
        ResultTask(Scheduler &scheduler)
            : Task(scheduler.resultInObjectForm
                       ? RunOnPreferedWorker | CopyArguments
                       : RunOnPreferedServer | CopyArguments),
              scheduler(scheduler) {}

        virtual std::string getTaskType() const { return "result"; }

        virtual TaskDataItem *run(
            WorkerLocation &location,
            std::vector<TaskDataItem *> args)
        {
            TaskDataItem *result = args.front();

            TaskDataLocation storedLocation = result->findLocation(location);
            WorkerLocation preferredLocation = result->getPreferredLocation();

            if (scheduler.resultInObjectForm)
            {
                if (storedLocation.getStorageType() != Object)
                {
                    ConnectionInfo *ci = location.getWorkerConnection();
                    // materialize file content as object
                    if (Relation::checkType(result->getContentType()))
                    {
                        Task::runCommand(
                            ci,
                            "(let " + result->getObjectName() +
                                " = (consume (feed" +
                                storedLocation.getValueArgument(result) + ")))",
                            "store file relation as object",
                            true);
                    }
                    else
                    {
                        Task::runCommand(
                            ci,
                            "(let " + result->getObjectName() + " = " +
                                storedLocation.getValueArgument(result) + ")",
                            "store file value as object",
                            true);
                    }
                    result->addLocation(
                        TaskDataLocation(location, Object, false));
                }
                else
                {
                    result->persistLocation(storedLocation);
                }
            }
            else
            {
                if (storedLocation.getWorkerLocation() != preferredLocation)
                {
                    ConnectionInfo *ci = location.getWorkerConnection();
                    // copy file into worker
                    Task::runCommand(
                        ci,
                        string("query createDirectory(") +
                            "'" + preferredLocation.getFileDirectory(result) +
                            "', TRUE)",
                        "create directory");
                    Task::runCommand(
                        ci,
                        string("query copyFile(") +
                            "'" + storedLocation.getFilePath(result) + "', " +
                            "'" + preferredLocation.getFilePath(result) + "')",
                        "copy file to correct worker");
                    result->addLocation(
                        TaskDataLocation(preferredLocation, File, false));
                }
                else
                {
                    result->persistLocation(storedLocation);
                }
            }

            scheduler.dArrayName = result->getName();
            while (scheduler.myResult.size() <= result->getSlot())
            {
                scheduler.myResult.push_back(DArrayElement("", 0, 0, ""));
            }
            // TODO Error when already set (multiple leaf tasks)
            DArrayElement &resultItem = scheduler.myResult[result->getSlot()];
            if (resultItem.getHost() != "")
            {
                cout << "Multiple leaf tasks for slot " << result->getSlot()
                     << endl;
            }
            resultItem = preferredLocation.getDArrayElement();

            return result;
        }

    private:
        Scheduler &scheduler;
    };

    int fileTransferPort;
    bool resultInObjectForm;

    boost::mutex mutex;

    std::map<WorkerLocation, boost::thread *> threads;
    size_t runningThreads = 0;
    boost::condition_variable threadSignal;

    std::list<Task *> workPool;
    std::list<std::pair<TaskDataItem *, TaskDataLocation>> garbagePool;
    size_t workerWorking = 0;
    bool stopWorkersWhenWorkDone = false;
    bool killWorkers = false;
    boost::condition_variable poolSignal;

    OnlyOnceMutexMap<std::string> fileTransferrators;

    std::map<string, pair<bool, int>> activeTransferrators;

    std::map<TaskDataItem *, int> dataReferences;

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
    bool resultInObjectForm =
        DArray::checkType(
            Task::innerType(nl->Second(qp->GetType(qp->GetSon(s, 0)))));
    Scheduler scheduler(port, resultInObjectForm);

    while ((task = stream.request()) != 0 && !scheduler.isError)
    {
        scheduler.receiveTask(task);
    }

    scheduler.join();
    stream.close();
    // TODO delete all tasks
    if (scheduler.isError)
    {
        cout << "schedule failed: " << scheduler.errorMessage << endl;
        // TODO report error
    }
    else
    {
        vector<uint32_t> mapping;
        vector<DArrayElement> workers;
        map<DArrayElement, optional<uint32_t>> workersMap;
        for (auto &worker : scheduler.getWorkers())
        {
            auto element = worker.getDArrayElement();
            workersMap.emplace(element, workers.size());
            workers.push_back(element);
        }
        for (auto &element : scheduler.myResult)
        {
            auto &entry = workersMap[element];
            if (!entry)
            {
                entry.emplace(workers.size());
                workers.push_back(element);
            }
            mapping.push_back(entry.value());
        }
        res->set(mapping, scheduler.dArrayName, workers);
    }
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
