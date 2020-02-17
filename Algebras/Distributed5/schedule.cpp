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

// #define DEBUG_JOB_SELECTION

namespace distributed5
{

/*
1 schedule Operator

The operator schedule is responsible for distributing the tasks from a tuple 
stream to the worker.
*/

/*
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
    int successors = 0;
    TaskDataItem *result = 0;
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

class Scheduler;

/*

1.3 WorkerJob

A worker job.

*/

class WorkerJob
{
public:
    WorkerJob(WorkerLocation &location, Scheduler &scheduler)
        : location(location), scheduler(scheduler) {}
    virtual ~WorkerJob() {}

    virtual string toString() const = 0;

    virtual bool run(boost::unique_lock<boost::mutex> &lock) = 0;

protected:
    WorkerLocation &location;
    Scheduler &scheduler;
};

/*
1.2 Scheduler

Scheduler.

*/

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

        cout << "=== Total ===\n"
             << stats.toString();
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
        std::vector<Task *> &predecessors = task->getPredecessors();
        for (auto it = predecessors.begin(); it != predecessors.end(); it++)
        {
            TaskScheduleInfo &preInfo = taskInfo[*it];
            preInfo.successors++;
            if (preInfo.completed)
            {
                dataReferences[preInfo.result]++;
            }
        }
        scheduleTask(task);
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

    void addError(string message)
    {
        isError = true;
        if (!errorMessage.empty())
            errorMessage += "\n\n";
        errorMessage += message;
    }

    void removeTask(list<Task *>::const_iterator iterator)
    {
        workPool.erase(iterator);
    }

    void waitForPoolUpdate(boost::unique_lock<boost::mutex> &lock)
    {
        poolSignal.wait(lock);
    }

    void signalPoolUpdate()
    {
        poolSignal.notify_all();
    }

    int getFileTransferPort()
    {
        return fileTransferPort;
    }

    void reserveTask(Task *task, WorkerLocation &location, int cost)
    {
        taskInfo[task].reservation =
            make_pair(location, cost);
    }

    void unreserveTask(Task *task, WorkerLocation &location, int cost)
    {
        auto &reservation = taskInfo[task].reservation;
        if (reservation && reservation->first == location)
        {
            reservation.reset();
        }
    }

    void updateActiveFileTransfers(string server, int update)
    {
        activeTransferrators[server].second += update;
    }

    void setTaskResult(Task *task, TaskDataItem *result)
    {
        // make sure a worker is running for the location of the result
        ensureWorker(result->getFirstLocation().getWorkerLocation());

        // when data with the same name has already been referenced
        // merge both data items to one item to keep only a single data item
        // per name
        string oname = result->getObjectName();
        TaskDataItem *existing = dataItems[oname];
        if (existing != 0)
        {
            if (existing != result)
            {
                existing->merge(result);
                delete result;
                result = existing;
            }
        }
        else
        {
            dataItems[oname] = result;
        }

        // store result
        TaskScheduleInfo &info = taskInfo[task];
        info.result = result;
        info.completed = true;

        // each successor keeps a reference to the result
        dataReferences[result] += info.successors;

        // For all precessor tasks, decrease the number of remaining tasks
        for (auto preTask : task->getPredecessors())
        {
            TaskScheduleInfo &preInfo = taskInfo[preTask];
            dataReferences[preInfo.result]--;
        }

        // A new result may unlock other tasks
        // Decreased data references may unlock garbagged collecting
        poolSignal.notify_all();
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

    // makes sure a worker thread for this location is running
    // mutex must already be locked
    void ensureWorker(const WorkerLocation &location)
    {
        if (location.getServer() == "")
            return;
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
            double duration = Task::runCommand(location.getWorkerConnection(),
                                               cmd,
                                               "open file transferator",
                                               false,
                                               true);

            TaskStatistics::report("open file transferator", duration);
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
            auto start = std::chrono::high_resolution_clock::now();
            WorkerJob *job = selectJob(location);
            auto duration =
                std::chrono::duration_cast<std::chrono::microseconds>(
                    std::chrono::high_resolution_clock::now() - start);
            TaskStatistics::report("selecting job",
                                   ((double)duration.count()) / 1000000);
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
                    if (stopWorkersWhenWorkDone && workPool.empty())
                    {
                        poolSignal.notify_all();
                        break;
                    }
                }

                workerWorking--;

                // Go into sleep mode
                auto start = std::chrono::high_resolution_clock::now();
                poolSignal.wait(lock);
                auto duration =
                    std::chrono::duration_cast<std::chrono::microseconds>(
                        std::chrono::high_resolution_clock::now() - start);
                TaskStatistics::report("worker idle",
                                       ((double)duration.count()) / 1000000);

                workerWorking++;
            }
        }

        cout << "=== " << location.toString() << " ===\n"
             << TaskStatistics::getThreadLocal().toString();

        stats.merge(TaskStatistics::getThreadLocal());

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
            if (arg == 0)
            {
                cost += CostMissingArgument;
                continue;
            }
            cost += arg->getDistance(location);
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

    // from https://stackoverflow.com/a/57635490
    struct pair_hash
    {
        template <class T1, class T2>
        std::size_t operator()(const std::pair<T1, T2> &pair) const
        {
            return std::hash<T1>()(pair.first) ^ std::hash<T2>()(pair.second);
        }
    };

    // mutex must already be locked
    WorkerJob *selectJob(WorkerLocation &location);

    class ResultTask : public Task
    {
    public:
        ResultTask(Scheduler &scheduler)
            : Task(CopyArguments | ConvertArguments |
                   (scheduler.resultInObjectForm
                        ? RunOnPreferedWorker | PrimaryArgumentAsObject
                        : RunOnPreferedServer | PrimaryArgumentAsFile)),
              scheduler(scheduler) {}

        virtual std::string getTaskType() const { return "result"; }

        virtual TaskDataItem *run(
            WorkerLocation &location,
            std::vector<TaskDataItem *> args)
        {
            TaskDataItem *result = args.front();

            TaskDataLocation storedLocation =
                result->findLocation(
                    location,
                    scheduler.resultInObjectForm ? Object : File);
            WorkerLocation preferredLocation = result->getPreferredLocation();

            if (scheduler.resultInObjectForm)
            {
                result->persistLocation(storedLocation);
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
                        "create directory",
                        false, true);
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
    size_t workerWorking = 0;
    bool stopWorkersWhenWorkDone = false;
    bool killWorkers = false;
    boost::condition_variable poolSignal;

    OnlyOnceMutexMap<std::string> fileTransferrators;

    std::map<string, pair<bool, int>> activeTransferrators;

    std::unordered_map<TaskDataItem *, int> dataReferences;
    std::unordered_map<std::string, TaskDataItem *> dataItems;

    std::unordered_map<Task *, TaskScheduleInfo> taskInfo;

    TaskStatistics stats;
};

/*

1.3 ExecuteWorkerJob

ExecuteWorkerJob.

*/

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

    virtual string toString() const
    {
        string message = string("execute ") + task->toString();
        int i = 0;
        for (auto arg : args)
        {
            message += string("\narg ") + std::to_string(i++) +
                       " = " + arg->toString();
        }
        return message;
    }

    virtual bool run(boost::unique_lock<boost::mutex> &lock)
    {
        // remove task from the pool
        scheduler.removeTask(workPoolIterator);

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
                             "while running " + task->toString() +
                             " on " + location.toString();
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

    virtual string toString() const
    {
        return "transfer " + data->toString() +
               " from " + sourceLocation.toString();
    }

    virtual bool run(boost::unique_lock<boost::mutex> &lock)
    {
        string sourceServer = sourceLocation.getServer();

        // set reservation
        scheduler.reserveTask(task, location, taskCost);

        // set active transfer and upcoming location
        TaskDataLocation loc(location, File, true);
        data->addUpcomingLocation(loc);
        scheduler.updateActiveFileTransfers(sourceServer, 1);

        lock.unlock();

        double duration = 0;

        try
        {
            ConnectionInfo *targetCI = location.getWorkerConnection();

            string cmd = string("query getFileTCP(") +
                         "'" + sourceLocation.getFilePath(data) + "', " +
                         "'" + sourceServer + "', " +
                         std::to_string(scheduler.getFileTransferPort()) +
                         ", TRUE, " +
                         "'" + location.getFilePath(data) + "')";
            duration = Task::runCommand(targetCI, cmd, "transfer file");
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

        TaskStatistics::report("transfer file", duration);

        lock.lock();

        scheduler.updateActiveFileTransfers(sourceServer, -1);
        data->addLocation(loc);

        // unreserve when still reserved by this worker
        scheduler.unreserveTask(task, location, taskCost);

        scheduler.signalPoolUpdate();

        return true;
    }

private:
    Task *task;
    int taskCost;
    TaskDataItem *data;
    TaskDataLocation sourceLocation;
};

class ConvertToObjectWorkerJob : public WorkerJob
{
public:
    ConvertToObjectWorkerJob(WorkerLocation &location,
                             Scheduler &scheduler,
                             Task *task,
                             int taskCost,
                             TaskDataItem *data)
        : WorkerJob(location, scheduler),
          task(task), taskCost(taskCost),
          data(data) {}
    virtual ~ConvertToObjectWorkerJob() {}

    virtual string toString() const
    {
        return "convert to object " + data->toString();
    }

    virtual bool run(boost::unique_lock<boost::mutex> &lock)
    {
        TaskDataLocation sourceLocation =
            data->findLocation(location, File);

        // set reservation
        scheduler.reserveTask(task, location, taskCost);

        // set active transfer and upcoming location
        TaskDataLocation loc(location, Object, true);
        data->addUpcomingLocation(loc);

        lock.unlock();

        double duration = 0;

        try
        {
            ConnectionInfo *ci = location.getWorkerConnection();
            string cmd;
            string description;

            if (data->isObjectRelation() || data->isFileRelation())
            {
                cmd = "(let " + data->getObjectName() +
                      " = (consume (feed" +
                      sourceLocation.getValueArgument(data) + ")))";
                description = "store file relation as object";
            }
            else
            {
                cmd = "(let " + data->getObjectName() + " = " +
                      sourceLocation.getValueArgument(data) + ")";
                description = "store file value as object";
            }

            try
            {
                duration += Task::runCommand(
                    ci,
                    cmd,
                    description,
                    true);
            }
            catch (RemoteException &e)
            {
                // Failed, maybe variable already exists
                // delete variable and retry
                duration += Task::runCommand(
                    ci,
                    "(delete " + data->getObjectName() + ")",
                    "delete existing object",
                    true, true, true);

                duration += Task::runCommand(
                    ci,
                    cmd,
                    description,
                    true);
            }
        }
        catch (exception &e)
        {
            lock.lock();
            scheduler.addError(string(e.what()) + "\n" +
                               "while converting " + data->toString() +
                               " to object form on " + location.toString());
            return false;
        }

        TaskStatistics::report("convert to object", duration);

        lock.lock();

        data->addLocation(loc);

        // unreserve when still reserved by this worker
        scheduler.unreserveTask(task, location, taskCost);

        scheduler.signalPoolUpdate();

        return true;
    }

private:
    Task *task;
    int taskCost;
    TaskDataItem *data;
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

    virtual string toString() const
    {
        return "convert to file " + data->toString();
    }

    virtual bool run(boost::unique_lock<boost::mutex> &lock)
    {
        // set active transfer and upcoming location
        TaskDataLocation loc(location, File, true);
        data->addUpcomingLocation(loc);

        lock.unlock();

        double duration = 0;

        try
        {
            ConnectionInfo *ci = location.getWorkerConnection();
            // Save Object in a file
            string oname = data->getObjectName();
            string fname = location.getFilePath(data);
            string cmd = "query " + oname +
                         " saveObjectToFile['" + fname + "']";
            duration = Task::runCommand(ci, cmd, "save object to file");
        }
        catch (exception &e)
        {
            lock.lock();
            scheduler.addError(string(e.what()) + "\n" +
                               "while converting " + data->toString() +
                               " to file form on " + location.toString());
            return false;
        }

        TaskStatistics::report("convert to file", duration);

        lock.lock();

        data->addLocation(loc);

        scheduler.signalPoolUpdate();

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

    virtual string toString() const
    {
        return "wait for tranfer completed";
    }

    virtual bool run(boost::unique_lock<boost::mutex> &lock)
    {
        auto start = std::chrono::high_resolution_clock::now();
        scheduler.waitForPoolUpdate(lock);
        auto duration =
            std::chrono::duration_cast<std::chrono::microseconds>(
                std::chrono::high_resolution_clock::now() - start);
        TaskStatistics::report("wait for transfer",
                               ((double)duration.count()) / 1000000);
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

    virtual string toString() const
    {
        return "remove " + dataLocation.toString() +
               " from " + data->toString();
    }

    virtual bool run(boost::unique_lock<boost::mutex> &lock)
    {
        data->removeLocation(dataLocation);
        lock.unlock();

        try
        {
            ConnectionInfo *ci = location.getWorkerConnection();
            switch (dataLocation.getStorageType())
            {
            case File:
            {
                string file = dataLocation.getFilePath(data);
                double duration = Task::runCommand(
                    ci,
                    "query removeFile('" + file + "')",
                    "remove temporary data file",
                    false, true);
                TaskStatistics::report("remove file", duration);

                break;
            }
            case Object:
            {
                string objectName = data->getObjectName();
                double duration = Task::runCommand(
                    ci,
                    "(delete " + objectName + ")",
                    "remove temporary data object",
                    true, true);
                TaskStatistics::report("remove object", duration);

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

WorkerJob *Scheduler::selectJob(WorkerLocation &location)
{
    unordered_map<pair<TaskDataItem *, bool>, pair<int, Task *>, pair_hash>
        transferables;

    auto start = std::chrono::high_resolution_clock::now();
    optional<pair<WorkerJob *, int>> best;
    int lastCost = 0;
    list<Task *>::iterator last;
    for (auto it = workPool.begin(); it != workPool.end(); it++)
    {
        auto task = *it;
        auto taskIt = it;

        // TODO this can be removed when task arguments
        // are stored in the taskInfo struct and computed
        // when precedecor is completed resp. on receive

        auto start = std::chrono::high_resolution_clock::now();
        vector<TaskDataItem *> args = getTaskArguments(task);
        auto duration =
            std::chrono::duration_cast<std::chrono::microseconds>(
                std::chrono::high_resolution_clock::now() - start);
        TaskStatistics::report(
            "selectJob getTaskArguments",
            ((double)duration.count()) / 1000000);

        start = std::chrono::high_resolution_clock::now();
        int cost = computeTaskCost(task, args, location);
        duration =
            std::chrono::duration_cast<std::chrono::microseconds>(
                std::chrono::high_resolution_clock::now() - start);
        TaskStatistics::report("selectJob computeTaskCost",
                               ((double)duration.count()) / 1000000);

        start = std::chrono::high_resolution_clock::now();

        if (cost < lastCost)
        {
            // Swap items
            *it = *last;
            *last = task;
            taskIt = last;
        }
        else
        {
            lastCost = cost;
        }
        last = it;

        // Skip early when there is already a better job
        if (best && cost > best->second)
            continue;

        TaskDataItem *firstArg = args.front();

        bool validWorker =
            !task->hasFlag(RunOnPreferedWorker) ||
            (firstArg != 0 &&
             firstArg->getPreferredLocation() == location);
        bool validServer =
            (!task->hasFlag(RunOnPreferedWorker) &&
             !task->hasFlag(RunOnPreferedServer)) ||
            (firstArg != 0 &&
             firstArg->getPreferredLocation().getServer() ==
                 location.getServer());

        bool argumentsAvailable = true;

        if (validServer)
        {
            // check if there is already a reservation for this task
            optional<pair<WorkerLocation, int>> reservation =
                taskInfo[task].reservation;
            if (reservation)
            {
                int resCost = reservation->second;
                // it's only allowed to seal a reservation
                // when the cost is smaller
                cost += CostReservation;

                // Skip early when there is already a better job
                if (best && cost > best->second)
                    continue;

                if (cost >= resCost)
                {
                    // When by this server reserved
                    // We can help to copy files
                    string server = reservation->first.getServer();
                    if (server != location.getServer())
                    {
                        validServer = false;
                    }
                    validWorker = false;
                }
            }
        }

        duration =
            std::chrono::duration_cast<std::chrono::microseconds>(
                std::chrono::high_resolution_clock::now() - start);
        TaskStatistics::report("selectJob prechecks",
                               ((double)duration.count()) / 1000000);

        start = std::chrono::high_resolution_clock::now();

        int i = 0;
        for (auto data : args)
        {
            bool primaryArgument = i == 0;
            i++;

            if (data == 0)
            {
                // When arguments are missing
                // this task can't be executed yet
                argumentsAvailable = false;
                continue;
            }

            bool hasFile = data->hasLocation(location,
                                             DataStorageType::File);
            bool hasObject = data->hasLocation(location,
                                               DataStorageType::Object);

            bool forceFile =
                task->hasFlag(primaryArgument
                                  ? PrimaryArgumentAsFile
                                  : SecondaryArgumentsAsFile);
            bool forceObject =
                task->hasFlag(primaryArgument
                                  ? PrimaryArgumentAsObject
                                  : SecondaryArgumentsAsObject);

            if (!forceFile && data->isObjectRelation())
            {
                forceObject = true;
            }
            if (!forceObject && data->isFileRelation())
            {
                forceFile = true;
            }

            if ((!hasFile || forceObject) && (!hasObject || forceFile))
            {
                // Data is not at the correct location
                // this can't be executed
                argumentsAvailable = false;
            }

            // Is Copy allowed?
            if (task->hasFlag(CopyArguments))
            {
                // This is a valid server for execution
                // and the data is not yet available
                if (validServer &&
                    !hasFile && !hasObject)
                {
                    auto &bestTransferrable =
                        transferables[make_pair(data, validWorker)];
                    if (bestTransferrable.second == 0 ||
                        bestTransferrable.first > cost)
                    {
                        bestTransferrable.first = cost;
                        bestTransferrable.second = task;
                    }
                }
            }

            // Is Convert allowed?
            if (task->hasFlag(TaskFlag::ConvertArguments))
            {
                // Convert from file to object when data need to be in
                // object form
                if (validWorker &&
                    forceObject &&
                    !hasObject && hasFile)
                {
                    int convertCost = cost + CostConvertToObject;
                    if (checkBetter(best, convertCost))
                    {
                        best = make_pair(
                            new ConvertToObjectWorkerJob(
                                location, *this,
                                task, cost, data),
                            convertCost);
                    }
                }
                // Convert from object to file when data need to be in
                // file form
                if (validServer &&
                    forceFile &&
                    !hasFile && hasObject)
                {
                    int convertCost = cost + CostConvertToFile;
                    if (checkBetter(best, convertCost))
                    {
                        best = make_pair(
                            new ConvertToFileWorkerJob(
                                location, *this, data),
                            convertCost);
                    }
                }
                // Convert from object to file so other worker can copy it
                // This helps other workers to execute the task
                // So we reverse task cost logic in a way that far away
                // tasks will get their data converted first, as their as
                // at least likely to be executed by this worker
                if (hasObject && !hasFile)
                {
                    int convertCost = INT_MAX - cost;
                    if (checkBetter(best, convertCost))
                    {
                        best = make_pair(
                            new ConvertToFileWorkerJob(
                                location, *this, data),
                            convertCost);
                    }
                }
            }
        }

        duration =
            std::chrono::duration_cast<std::chrono::microseconds>(
                std::chrono::high_resolution_clock::now() - start);
        TaskStatistics::report("selectJob inner loop",
                               ((double)duration.count()) / 1000000);

        start = std::chrono::high_resolution_clock::now();

        if (argumentsAvailable && validWorker)
        {
            if (checkBetter(best, cost))
            {
                best = make_pair(
                    new ExecuteWorkerJob(location, *this,
                                         task, args, taskIt),
                    cost);
            }
        }
        duration =
            std::chrono::duration_cast<std::chrono::microseconds>(
                std::chrono::high_resolution_clock::now() - start);
        TaskStatistics::report("selectJob execute",
                               ((double)duration.count()) / 1000000);
    }
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(
        std::chrono::high_resolution_clock::now() - start);
    TaskStatistics::report("selectJob loop1",
                           ((double)duration.count()) / 1000000);
    start = std::chrono::high_resolution_clock::now();

    for (auto pair : transferables)
    {
        TaskDataItem *data = pair.first.first;
        bool validWorker = pair.first.second;
        int cost = pair.second.first;
        Task *task = pair.second.second;

        // Check if data is already being transferred here
        if (data->hasUpcomingLocation(location, File))
        {
            // The correct worker can wait for the data transfer
            int waitingCost = cost + CostWaitingOnTransfer;
            if (validWorker && checkBetter(best, waitingCost))
            {
                best = make_pair(
                    new WaitForTransferCompletedWorkerJob(
                        location, *this),
                    waitingCost);
            }
        }
        else
        {
            // Transfer the data to the server
            try
            {
                auto source =
                    data->findTransferSourceLocation(
                        activeTransferrators);
                int transferCost =
                    cost +
                    CostTransfer +
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
    duration = std::chrono::duration_cast<std::chrono::microseconds>(
        std::chrono::high_resolution_clock::now() - start);
    TaskStatistics::report("selectJob loop2",
                           ((double)duration.count()) / 1000000);

    if (best)
    {
#ifdef DEBUG_JOB_SELECTION
        cout << location.toString() << " (cost: " << best->second << ")"
             << " -> " << best->first->toString() << endl;
#endif
        return best->first;
    }

    if (!stopWorkersWhenWorkDone)
        return 0;

    // nothing productive to do
    // collect some garbage
    for (auto pair : dataReferences)
    {
        if (pair.second == 0)
        {
            TaskDataItem *data = pair.first;
            for (auto loc : data->getLocations())
            {
                if (loc.isTemporary() &&
                    loc.getWorkerLocation() == location)
                {
                    WorkerJob *job =
                        new RemoveDataWorkerJob(location, *this,
                                                data, loc);
#ifdef DEBUG_JOB_SELECTION
                    cout << location.toString() << " -> " << job->toString()
                         << endl;
#endif
                    return job;
                }
            }
        }
    }

    return 0;
}

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
