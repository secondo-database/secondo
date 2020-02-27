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
#include <unordered_set>

using namespace std;
using namespace distributed2;

// #define DEBUG_JOB_SELECTION
// #define REPORT_WORKER_STATS
#define REPORT_TOTAL_STATS

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

// from https://stackoverflow.com/a/57635490
struct pair_hash
{
    template <class T1, class T2>
    std::size_t operator()(const std::pair<T1, T2> &pair) const
    {
        return std::hash<T1>()(pair.first) ^ std::hash<T2>()(pair.second);
    }
};

struct TaskScheduleInfo
{
public:
    boost::shared_mutex mutex;
    std::unordered_set<pair<TaskScheduleInfo *, size_t>, pair_hash> successors;
    vector<TaskDataItem *> arguments;
    TaskDataItem *result = 0;
    // end of mutex protected

    optional<pair<WorkerLocation, int>> reservation;
    bool started = false;
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

    virtual string getType() const = 0;

    virtual bool run() = 0;

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
        boost::unique_lock<boost::mutex> lock(threadsMutex);
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

#ifdef REPORT_TOTAL_STATS
        cout << "=== Total ===\n"
             << stats.toString();
#endif
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

        // update task schedule info structures
        TaskScheduleInfo *info;
        {
            // need to be a exclusive lock, as task is unknown yet and will
            // be added
            boost::lock_guard<boost::shared_mutex> lock(poolMutex);
            info = &taskInfo[task];
        }

        if (task->hasFlag(RunOnReceive))
        {
            WorkerLocation emptyLocation("", 0, "", -1);
            vector<TaskDataItem *> emptyArgs;
            TaskDataItem *result = task->run(emptyLocation, emptyArgs);
            setTaskResult(task, result);
            return;
        }

        std::vector<Task *> &predecessors = task->getPredecessors();
        std::vector<TaskScheduleInfo *> preInfos;
        {
            boost::shared_lock_guard<boost::shared_mutex> lock(poolMutex);
            for (Task *task : predecessors)
            {
                preInfos.push_back(&taskInfo[task]);
            }
        }

        // "info" doesn't need locking as it's not yet in the workPool
        // and receiveTask is only called single-threaded
        for (auto preInfo : preInfos)
        {
            TaskDataItem *result;
            {
                boost::lock_guard<boost::shared_mutex> lock(preInfo->mutex);
                result = preInfo->result;
                if (result == 0)
                    preInfo->successors.emplace(info, info->arguments.size());
                info->arguments.push_back(result);
            }
            if (result != 0)
            {
                boost::lock_guard<boost::shared_mutex>
                    lock(dataReferencesMutex);
                dataReferences[result]++;
            }
        }
        {
            boost::lock_guard<boost::shared_mutex> lock(poolMutex);
            workPool.push_back(task);
            workGeneration++;
        }
        poolSignal.notify_all();
    }

    vector<WorkerLocation> getWorkers()
    {
        boost::lock_guard<boost::mutex> lock(threadsMutex);
        vector<WorkerLocation> workers;
        for (auto &pair : threads)
        {
            workers.push_back(pair.first);
        }
        return workers;
    }

    void addError(string message)
    {
        boost::lock_guard<boost::mutex> lock(resultMutex);
        isError = true;
        if (!errorMessage.empty())
            errorMessage += "\n\n";
        errorMessage += message;
    }

    void waitForPoolUpdateOrLocation(
        TaskDataItem *data, WorkerLocation &nearby)
    {
        boost::shared_lock<boost::shared_mutex> lock(poolMutex);

        if (!data->hasLocation(nearby))
            poolSignal.wait(lock);
    }

    void signalPoolUpdate()
    {
        {
            boost::lock_guard<boost::shared_mutex> lock(poolMutex);
            workGeneration++;
        }
        poolSignal.notify_all();
    }

    int getFileTransferPort()
    {
        return fileTransferPort;
    }

    bool startExecutingTask(Task *task)
    {
        TaskScheduleInfo *info;
        {
            boost::shared_lock_guard<boost::shared_mutex> lock(poolMutex);
            info = &taskInfo[task];
        }
        {
            boost::lock_guard<boost::shared_mutex> lock(info->mutex);
            if (info->started)
            {
                return false;
            }
            info->started = true;
        }
        boost::lock_guard<boost::shared_mutex> lock(poolMutex);
        workPool.remove(task);
        return true;
    }

    bool reserveTask(Task *task, WorkerLocation &location, int cost)
    {
        TaskScheduleInfo *info;
        {
            boost::shared_lock_guard<boost::shared_mutex> lock(poolMutex);
            info = &taskInfo[task];
        }
        boost::lock_guard<boost::shared_mutex> lock(info->mutex);
        if (info->started)
            return false;
        auto &reservation = info->reservation;
        if (reservation && reservation->second <= cost)
        {
            // already reserved with lower (or equal) cost
            if (reservation->first.getServer() != location.getServer())
            {
                // by other server
                // reserve fails
                return false;
            }
            else
            {
                // by other worker on same server
                // reserve succeed, but doesn't update reservation
                return true;
            }
        }
        // not reserved or reserved with higher cost
        // update reservation
        // and reserve succeed
        info->reservation =
            make_pair(location, cost);
        return true;
    }

    void unreserveTask(Task *task, WorkerLocation &location, int cost)
    {
        TaskScheduleInfo *info;
        {
            boost::shared_lock_guard<boost::shared_mutex> lock(poolMutex);
            info = &taskInfo[task];
        }
        boost::lock_guard<boost::shared_mutex> lock(info->mutex);
        auto &reservation = info->reservation;
        if (reservation && reservation->first == location)
        {
            reservation.reset();
        }
    }

    void updateActiveFileTransfers(string server, int update)
    {
        boost::lock_guard<boost::shared_mutex> lock(poolMutex);
        activeTransferrators[server].second += update;
    }

    void setTaskResult(Task *task, TaskDataItem *result)
    {
        // make sure a worker is running for the location of the result
        ensureWorker(result->getFirstLocation().getWorkerLocation());

        string oname = result->getObjectName();
        TaskDataItem *existing = 0;
        {
            boost::lock_guard<boost::mutex> lock(dataItemsMutex);

            // when data with the same name has already been referenced
            // merge both data items to one item to keep only a single data item
            // per name
            auto pair = dataItems.emplace(oname, result);
            if (!pair.second)
            {
                existing = pair.first->second;
            }
        }
        if (existing != 0)
        {
            if (existing != result)
            {
                existing->merge(result);
                delete result;
                result = existing;
            }
        }

        TaskScheduleInfo *info;
        {
            boost::shared_lock_guard<boost::shared_mutex> lock(poolMutex);
            info = &taskInfo[task];
        }

#ifdef DEBUG_JOB_SELECTION
        cout << "Got result from " << task->getId() << ": "
             << result->toString() << endl;
#endif

        size_t refs = 0;
        {
            boost::lock_guard<boost::shared_mutex> lock(info->mutex);

            // store result
            info->result = result;

            // update the arguments of successors
            for (auto pair : info->successors)
            {
                refs++;

                // Here two mutexes are locked, but order of locking is
                // always in direction of result flow
                // So no deadlock can occur
                boost::lock_guard<boost::shared_mutex> lock(pair.first->mutex);
                pair.first->arguments[pair.second] = result;
            }
        }

        // For all precessor tasks, decrease the number of remaining tasks
        // First collect all results from pre tasks
        vector<TaskDataItem *> preResults;
        {
            boost::shared_lock_guard<boost::shared_mutex> lock(poolMutex);
            for (auto preTask : task->getPredecessors())
            {
                TaskScheduleInfo &preInfo = taskInfo[preTask];
                // Here no locking is needed as result is always set
                // and won't change when set once
                preResults.push_back(preInfo.result);
            }
        }

        {
            boost::lock_guard<boost::shared_mutex> lock(dataReferencesMutex);

            // each successor keeps a reference to the result
            dataReferences[result] += refs;

            // Then update data references from pre tasks
            for (auto preResult : preResults)
            {
                dataReferences[preResult]--;
            }
        }

        // A new result may unlock other tasks
        // Decreased data references may unlock garbagged collecting
        {
            boost::lock_guard<boost::shared_mutex> lock(poolMutex);
            workGeneration++;
        }

        poolSignal.notify_all();
    }

    boost::mutex resultMutex;
    vector<DArrayElement> myResult;
    string dArrayName;
    bool isError = false;
    string errorMessage;

private:
    static thread_local optional<set<WorkerLocation>>
        ensureWorkerCheckedLocations;

    // makes sure a worker thread for this location is running
    // mutex must already be locked
    void ensureWorker(const WorkerLocation &location)
    {
        if (location.getServer() == "")
            return;
        // lock-free check of the thread local data
        // to fast exit on already checked locations
        if (ensureWorkerCheckedLocations &&
            !ensureWorkerCheckedLocations->emplace(location).second)
            return;
        {
            boost::lock_guard<boost::mutex> lock(threadsMutex);
            auto &slot = threads[location];
            if (slot != 0)
                return;
            runningThreads++;
            slot =
                new boost::thread(
                    boost::bind(
                        &Scheduler::worker, this, WorkerLocation(location)));
        }
        {
            boost::lock_guard<boost::shared_mutex> lock(poolMutex);
            workerWorking++;
        }
    }

    // mutex must already be locked
    void stopWorkers(bool waitForWorkDone)
    {
        boost::lock_guard<boost::shared_mutex> lock(poolMutex);
        if (waitForWorkDone)
            stopWorkersWhenWorkDone = true;
        else
            killWorkers = true;
        workGeneration++;
        poolSignal.notify_all();
    }

    bool shouldStopWorkersWhenWorkDone()
    {
        boost::shared_lock_guard<boost::shared_mutex> lock(poolMutex);
        return stopWorkersWhenWorkDone;
    }

    bool shouldKillWorkers()
    {
        boost::shared_lock_guard<boost::shared_mutex> lock(poolMutex);
        return killWorkers;
    }

    bool isErrored()
    {
        boost::lock_guard<boost::mutex> lock(resultMutex);
        return isError;
    }

    size_t getWorkGeneration()
    {
        boost::shared_lock_guard<boost::shared_mutex> lock(poolMutex);
        return workGeneration;
    }

    void ensureFileTransferrator(WorkerLocation &location)
    {
        {
            auto lock = fileTransferrators.check(location.getServer());
            if (!lock)
                return;
            string cmd = "query staticFileTransferator(" +
                         std::to_string(fileTransferPort) +
                         ",10)";
            double duration =
                Task::runCommand(location.getWorkerConnection(),
                                 cmd,
                                 "open file transferator",
                                 false,
                                 true);

            TaskStatistics::report("remote open file transferator", duration);
        }

        boost::lock_guard<boost::shared_mutex> lock(poolMutex);

        auto &transferrator = activeTransferrators[location.getServer()];
        if (!transferrator.first)
        {
            transferrator.first = true;
            workGeneration++;
            poolSignal.notify_all();
        }
    }

    // poolMutex already need to be locked
    vector<TaskDataItem *> getTaskArguments(Task *task)
    {
        auto &info = taskInfo[task];
        boost::shared_lock_guard<boost::shared_mutex> lock(info.mutex);
        return info.arguments;
    }

    // the worker thread
    void worker(WorkerLocation location)
    {
        // enable ensureWorker caching for this worker
        ensureWorkerCheckedLocations.emplace();

        // Connect to the worker
        location.getWorkerConnection();

        // Ensure file transferrator is open
        ensureFileTransferrator(location);

        while (true)
        {
            size_t startWorkGeneration = getWorkGeneration();
            if (shouldKillWorkers())
                break;
            auto start = std::chrono::high_resolution_clock::now();
            WorkerJob *job = selectJob(location);
            if (job == 0 && shouldStopWorkersWhenWorkDone() && !isErrored())
            {
                job = selectGarbaggeJob(location);
            }
            auto duration =
                std::chrono::duration_cast<std::chrono::microseconds>(
                    std::chrono::high_resolution_clock::now() - start);
            TaskStatistics::report("selecting job",
                                   ((double)duration.count()) / 1000000);
            if (job != 0)
            {
                auto start = std::chrono::high_resolution_clock::now();
                if (!job->run())
                {
                    stopWorkers(false);
                    break;
                }
                auto duration =
                    std::chrono::duration_cast<std::chrono::microseconds>(
                        std::chrono::high_resolution_clock::now() - start);
                TaskStatistics::report("run job " + job->getType(),
                                       ((double)duration.count()) / 1000000);
            }
            else if (startWorkGeneration == getWorkGeneration())
            {
                // Go into sleep mode
                auto start = std::chrono::high_resolution_clock::now();

                {
                    boost::unique_lock<boost::shared_mutex> lock(poolMutex);

                    if (startWorkGeneration != workGeneration)
                        continue;

                    // check for end of work
                    if (workerWorking == 1)
                    {
                        if (stopWorkersWhenWorkDone && workPool.empty())
                        {
                            workerWorking--;
                            poolSignal.notify_all();
                            break;
                        }
                    }

                    workerWorking--;

                    poolSignal.wait(lock);

                    workerWorking++;
                }

                auto duration =
                    std::chrono::duration_cast<std::chrono::microseconds>(
                        std::chrono::high_resolution_clock::now() - start);
                TaskStatistics::report("worker idle",
                                       ((double)duration.count()) / 1000000);
            }
        }

#ifdef REPORT_WORKER_STATS
        cout << "=== " << location.toString() << " ===\n"
             << TaskStatistics::getThreadLocal().toString();
#endif

#ifdef REPORT_TOTAL_STATS
        {
            boost::lock_guard<boost::mutex> lock(statsMutex);
            stats.merge(TaskStatistics::getThreadLocal());
        }
#endif

        {
            boost::lock_guard<boost::mutex> lock(threadsMutex);
            runningThreads--;
        }
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

    // mutex must already be locked
    void valueJobsForTask(WorkerLocation &location, Task *task,
                          optional<pair<WorkerJob *, int>> &best,
                          unordered_map<
                              pair<TaskDataItem *, bool>,
                              pair<int, Task *>,
                              pair_hash> &transferables);
    void valueJobsForTransferable(
        WorkerLocation &location,
        std::map<string, pair<bool, int>> activeTransferrators,
        TaskDataItem *data,
        bool validWorker,
        int cost, Task *task,
        optional<pair<WorkerJob *, int>> &best);
    WorkerJob *selectJob(WorkerLocation &location);
    WorkerJob *selectGarbaggeJob(WorkerLocation &location);

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

            boost::lock_guard<boost::mutex> lock(scheduler.resultMutex);
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

    boost::mutex threadsMutex;
    std::map<WorkerLocation, boost::thread *> threads;
    size_t runningThreads = 0;
    boost::condition_variable threadSignal;
    // end of threadsMutex protected

    boost::shared_mutex poolMutex;
    size_t workGeneration = 0;
    std::list<Task *> workPool;
    size_t workerWorking = 0;
    bool stopWorkersWhenWorkDone = false;
    bool killWorkers = false;
    boost::condition_variable_any poolSignal;

    std::unordered_map<Task *, TaskScheduleInfo> taskInfo;

    std::map<string, pair<bool, int>> activeTransferrators;
    // end of poolMutex protected

    boost::mutex dataItemsMutex;
    std::unordered_map<std::string, TaskDataItem *> dataItems;
    // end of dataReferencesMutex protected

    boost::shared_mutex dataReferencesMutex;
    std::unordered_map<TaskDataItem *, int> dataReferences;
    // end of dataReferencesMutex protected

    // thread-safe
    OnlyOnceMutexMap<std::string> fileTransferrators;

#ifdef REPORT_TOTAL_STATS
    boost::mutex statsMutex;
    TaskStatistics stats;
#endif
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
                     vector<TaskDataItem *> args)
        : WorkerJob(location, scheduler),
          task(task), args(args) {}
    virtual ~ExecuteWorkerJob() {}

    virtual string getType() const { return "execute"; }

    virtual string toString() const
    {
        string message = string("execute ") +
                         std::to_string(task->getId()) + " " + task->toString();
        int i = 0;
        for (auto arg : args)
        {
            message += string("\narg ") + std::to_string(i++) +
                       " = " + arg->toString();
        }
        return message;
    }

    virtual bool run()
    {
        if (!scheduler.startExecutingTask(task))
            return true;

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
            scheduler.addError(message);
            return false;
        }

        scheduler.setTaskResult(task, result);
        return true;
    }

private:
    Task *task;
    vector<TaskDataItem *> args;
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

    virtual string getType() const { return "transfer"; }

    virtual string toString() const
    {
        return "transfer " + data->toString() +
               " from " + sourceLocation.toString();
    }

    virtual bool run()
    {
        // set reservation
        if (!scheduler.reserveTask(task, location, taskCost))
            return true;

        string sourceServer = sourceLocation.getServer();

        // set active transfer and upcoming location
        TaskDataLocation loc(location, File, true);
        data->addUpcomingLocation(loc);
        scheduler.updateActiveFileTransfers(sourceServer, 1);

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
            scheduler.addError(string(e.what()) + "\n" +
                               "while copying " + data->toString() +
                               "\n - from " + sourceLocation.toString() +
                               "\n - to " + location.toString());
            return false;
        }

        TaskStatistics::report("remote transfer file", duration);

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

    virtual string getType() const { return "convert to object"; }

    virtual string toString() const
    {
        return "convert to object " + data->toString();
    }

    virtual bool run()
    {
        // set reservation
        if (!scheduler.reserveTask(task, location, taskCost))
            return true;

        TaskDataLocation sourceLocation =
            data->findLocation(location, File);

        // set active transfer and upcoming location
        TaskDataLocation loc(location, Object, true);
        data->addUpcomingLocation(loc);

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
            scheduler.addError(string(e.what()) + "\n" +
                               "while converting " + data->toString() +
                               " to object form on " + location.toString());
            return false;
        }

        TaskStatistics::report("remote convert to object", duration);

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

    virtual string getType() const { return "convert to file"; }

    virtual string toString() const
    {
        return "convert to file " + data->toString();
    }

    virtual bool run()
    {
        // set active transfer and upcoming location
        TaskDataLocation loc(location, File, true);
        data->addUpcomingLocation(loc);

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
            scheduler.addError(string(e.what()) + "\n" +
                               "while converting " + data->toString() +
                               " to file form on " + location.toString());
            return false;
        }

        TaskStatistics::report("remote convert to file", duration);

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
                                      Scheduler &scheduler,
                                      TaskDataItem *data)
        : WorkerJob(location, scheduler),
          data(data) {}
    virtual ~WaitForTransferCompletedWorkerJob() {}

    virtual string getType() const { return "wait for transfer"; }

    virtual string toString() const
    {
        return "wait for tranfer completed";
    }

    virtual bool run()
    {
        auto start = std::chrono::high_resolution_clock::now();
        scheduler.waitForPoolUpdateOrLocation(data, location);
        auto duration =
            std::chrono::duration_cast<std::chrono::microseconds>(
                std::chrono::high_resolution_clock::now() - start);
        TaskStatistics::report("wait for transfer",
                               ((double)duration.count()) / 1000000);
        return true;
    }

private:
    TaskDataItem *data;
};

class RemoveDataWorkerJob : public WorkerJob
{
public:
    RemoveDataWorkerJob(WorkerLocation &location, Scheduler &scheduler,
                        TaskDataItem *data, TaskDataLocation dataLocation)
        : WorkerJob(location, scheduler),
          data(data), dataLocation(dataLocation) {}
    virtual ~RemoveDataWorkerJob() {}

    virtual string getType() const { return "remove"; }

    virtual string toString() const
    {
        return "remove " + dataLocation.toString() +
               " from " + data->toString();
    }

    virtual bool run()
    {
        data->removeLocation(dataLocation);

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
                TaskStatistics::report("remote remove file", duration);

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
                TaskStatistics::report("remote remove object", duration);

                break;
            }
            }
        }
        catch (exception &e)
        {
            scheduler.addError(string(e.what()) + "\n" +
                               "while removing " + data->toString() +
                               " from " + dataLocation.toString());
            return false;
        }

        return true;
    }

private:
    TaskDataItem *data;
    TaskDataLocation dataLocation;
};

void Scheduler::valueJobsForTask(WorkerLocation &location, Task *task,
                                 optional<pair<WorkerJob *, int>> &best,
                                 unordered_map<
                                     pair<TaskDataItem *, bool>,
                                     pair<int, Task *>,
                                     pair_hash> &transferables)
{
    vector<TaskDataItem *> args = getTaskArguments(task);

    int cost = computeTaskCost(task, args, location);

    // Skip early when there is already a better job
    if (best && cost > best->second)
        return;

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
        TaskScheduleInfo *info;
        {
            boost::shared_lock_guard<boost::shared_mutex> lock(poolMutex);
            info = &taskInfo[task];
        }
        // check if there is already a reservation for this task
        optional<pair<WorkerLocation, int>> reservation;
        {
            boost::shared_lock_guard<boost::shared_mutex> lock(info->mutex);
            if (info->started)
                return;
            reservation = info->reservation;
        }
        if (reservation)
        {
            int resCost = reservation->second;
            // it's only allowed to seal a reservation
            // when the cost is smaller
            cost += CostReservation;

            // Skip early when there is already a better job
            if (best && cost > best->second)
                return;

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
                auto &bestTransferable =
                    transferables[make_pair(data, validWorker)];
                if (bestTransferable.second == 0 ||
                    bestTransferable.first > cost)
                {
                    bestTransferable.first = cost;
                    bestTransferable.second = task;
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

    if (argumentsAvailable && validWorker)
    {
        if (checkBetter(best, cost))
        {
            best = make_pair(
                new ExecuteWorkerJob(location, *this,
                                     task, args),
                cost);
        }
    }
}

void Scheduler::valueJobsForTransferable(
    WorkerLocation &location,
    std::map<string, pair<bool, int>> activeTransferrators,
    TaskDataItem *data, bool validWorker,
    int cost, Task *task,
    optional<pair<WorkerJob *, int>> &best)
{
    // Check if data is already being transferred here
    if (data->hasUpcomingLocation(location, File))
    {
        // The correct worker can wait for the data transfer
        int waitingCost = cost + CostWaitingOnTransfer;
        if (validWorker && checkBetter(best, waitingCost))
        {
            best = make_pair(
                new WaitForTransferCompletedWorkerJob(
                    location, *this,
                    data),
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

WorkerJob *Scheduler::selectJob(WorkerLocation &location)
{
    unordered_map<pair<TaskDataItem *, bool>, pair<int, Task *>, pair_hash>
        transferables;

    auto start = std::chrono::high_resolution_clock::now();

    vector<Task *> workPoolCopy;
    std::map<string, pair<bool, int>> activeTransferratorsCopy;

    {
        boost::shared_lock_guard<boost::shared_mutex> lock(poolMutex);

        // copy the work pool
        // Tasks may be removed from the work pool
        // All jobs handle this by validation check when they start running
        workPoolCopy.reserve(workPool.size());
        std::copy(std::begin(workPool), std::end(workPool),
                  std::back_inserter(workPoolCopy));
        for (auto &pair : activeTransferrators)
            activeTransferratorsCopy[pair.first] = pair.second;
    }

    if (workPoolCopy.empty())
        return 0;

    optional<pair<WorkerJob *, int>> best;

    // start at a random position
    size_t index = rand() % workPoolCopy.size();
    for (size_t i = index; i < workPoolCopy.size(); i++)
    {
        valueJobsForTask(location, workPoolCopy[i], best, transferables);
    }
    for (size_t i = 0; i < index; i++)
    {
        valueJobsForTask(location, workPoolCopy[i], best, transferables);
    }

    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(
        std::chrono::high_resolution_clock::now() - start);
    TaskStatistics::report("selectJob valueJobsForTask",
                           ((double)duration.count()) / 1000000);

    if (transferables.size() > 0)
    {
        start = std::chrono::high_resolution_clock::now();
        vector<pair<pair<TaskDataItem *, bool>, pair<int, Task *>>> transVec;

        std::copy(std::begin(transferables), std::end(transferables),
                  std::back_inserter(transVec));

        // start at a random position
        index = rand() % transVec.size();
        for (size_t i = index; i < transVec.size(); i++)
        {
            auto &pair = transVec[i];
            valueJobsForTransferable(
                location,
                activeTransferratorsCopy,
                pair.first.first, pair.first.second,
                pair.second.first, pair.second.second,
                best);
        }
        for (size_t i = 0; i < index; i++)
        {
            auto &pair = transVec[i];
            valueJobsForTransferable(
                location,
                activeTransferratorsCopy,
                pair.first.first, pair.first.second,
                pair.second.first, pair.second.second,
                best);
        }
        duration = std::chrono::duration_cast<std::chrono::microseconds>(
            std::chrono::high_resolution_clock::now() - start);
        TaskStatistics::report("selectJob valueJobsForTransferable",
                               ((double)duration.count()) / 1000000);
    }

    if (best)
    {
#ifdef DEBUG_JOB_SELECTION
        cout << location.toString() << " (cost: " << best->second << ")"
             << " -> " << best->first->toString() << endl;
#endif
        return best->first;
    }
    return 0;
}

WorkerJob *Scheduler::selectGarbaggeJob(WorkerLocation &location)
{
    boost::shared_lock_guard<boost::shared_mutex> lock(dataReferencesMutex);

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

thread_local optional<set<WorkerLocation>>
    Scheduler::ensureWorkerCheckedLocations;

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
