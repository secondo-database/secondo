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

#ifndef DISTRIBUTE5_TASK_H
#define DISTRIBUTE5_TASK_H
#include "Attribute.h"
#include "Algebra.h"
#include "NestedList.h"
#include "QueryProcessor.h"
#include "AlgebraManager.h"
#include "Operator.h"
#include "StandardTypes.h"
#include "Symbols.h"
#include "ListUtils.h"
#include "Algebras/Distributed2/CommandLogger.h"
#include "Algebras/Array/ArrayAlgebra.h"
#include "SocketIO.h"
#include "Algebras/Distributed2/FileRelations.h"
#include "Algebras/Distributed2/fsrel.h"
#include "Stream.h"
#include "Algebras/Distributed2/DArray.h"
#include "Algebras/Distributed2/Distributed2Algebra.h"
#include "Algebras/Distributed2/ConnectionInfo.h"
#include "Algebras/Distributed2/DFSType.h"

#include <boost/bind.hpp>
#include <boost/ref.hpp>
#include <boost/thread.hpp>

#include <set>
#include <optional>

namespace distributed5
{
enum DataStorageType
{
    Object,
    File
};

enum WorkerDistance
{
    SameProcess,
    SameServer,
    OtherServer
};

enum DataDistance : int
{
    MemoryOnWorker = 1,
    FileOnServer = 2,
    FileOverNetwork = 100,
    MemoryOnServer = 200,
    MemoryOverNetwork = 1000,
};

#define CostWeightArgument 10
#define CostNotPreferredServer 2000
#define CostNotPreferredWorker 100
#define CostActiveTransfers 1
#define CostWaitingOnTransfer 900
#define CostReservation 10

class TaskDataItem;

class WorkerLocation
{
public:
    WorkerLocation(
        std::string server,
        int port,
        std::string config,
        int worker)
        : server(server), port(port), config(config), worker(worker) {}

    const std::string &getServer() const { return server; }
    const std::string &getConfig() const { return config; }
    int getPort() const { return port; }
    int getWorker() const { return worker; }
    distributed2::DArrayElement getDArrayElement() const;

    std::string toString() const
    {
        return server +
               ":" + std::to_string(port) +
               " (" + std::to_string(worker) + " " + config + ")";
    }

    distributed2::ConnectionInfo *getWorkerConnection() const;

    std::string getFilePath(TaskDataItem *data) const;

    std::string getFileDirectory(TaskDataItem *data) const;

    bool operator==(WorkerLocation const &other) const
    {
        return server == other.server &&
               port == other.port &&
               config == other.config &&
               worker == other.worker;
    }

    bool operator!=(WorkerLocation const &other) const
    {
        return !(*this == other);
    }

    bool operator<(WorkerLocation const &other) const
    {
        return worker < other.worker;
    }

    bool operator>(WorkerLocation const &other) const
    {
        return worker > other.worker;
    }

    WorkerDistance getDistance(WorkerLocation const &other) const
    {
        if (server == other.server)
        {
            if (*this == other)
            {
                return WorkerDistance::SameProcess;
            }
            return WorkerDistance::SameServer;
        }
        return WorkerDistance::OtherServer;
    }

private:
    std::string server;
    int port;
    std::string config;
    int worker;
};

class TaskDataLocation
{
public:
    TaskDataLocation(
        std::string server,
        int port,
        std::string config,
        int worker,
        DataStorageType storageType,
        bool temporary)
        : workerLocation(server, port, config, worker),
          storageType(storageType), temporary(temporary) {}

    TaskDataLocation(
        WorkerLocation workerLocation,
        DataStorageType storageType,
        bool temporary)
        : workerLocation(workerLocation),
          storageType(storageType), temporary(temporary) {}

    DataStorageType getStorageType() const { return storageType; }
    bool isTemporary() const { return temporary; }

    const WorkerLocation &getWorkerLocation() const { return workerLocation; }
    const std::string &getServer() const { return workerLocation.getServer(); }
    const std::string &getConfig() const { return workerLocation.getConfig(); }
    int getPort() const { return workerLocation.getPort(); }
    int getWorker() const { return workerLocation.getWorker(); }

    distributed2::DArrayElement getDArrayElement() const
    {
        return workerLocation.getDArrayElement();
    }

    std::string toString() const
    {
        return (storageType == Object ? "object " : "file ") +
               workerLocation.toString();
    }

    distributed2::ConnectionInfo *getWorkerConnection() const
    {
        return workerLocation.getWorkerConnection();
    }

    std::string getFilePath(TaskDataItem *data) const
    {
        return workerLocation.getFilePath(data);
    }

    std::string getFileDirectory(TaskDataItem *data) const
    {
        return workerLocation.getFileDirectory(data);
    }

    std::string getValueArgument(TaskDataItem *data) const;

    DataDistance getDistance(WorkerLocation const &loc) const
    {
        WorkerDistance dist = workerLocation.getDistance(loc);
        switch (storageType)
        {
        case Object:
            switch (dist)
            {
            case SameProcess:
                return DataDistance::MemoryOnWorker;
            case SameServer:
                return DataDistance::MemoryOnServer;
            case OtherServer:
                return DataDistance::MemoryOverNetwork;
            }
            break;
        case File:
            switch (dist)
            {
            case SameProcess:
                return DataDistance::FileOnServer;
            case SameServer:
                return DataDistance::FileOnServer;
            case OtherServer:
                return DataDistance::FileOverNetwork;
            }
            break;
        }
        // This does not happen
        return (DataDistance)-1;
    }

    bool operator==(TaskDataLocation const &other) const
    {
        return workerLocation == other.workerLocation &&
               storageType == other.storageType &&
               temporary == other.temporary;
    }

    bool operator!=(TaskDataLocation const &other) const
    {
        return !(*this == other);
    }

private:
    WorkerLocation workerLocation;
    DataStorageType storageType;
    bool temporary;
};

class TaskDataItem
{
public:
    TaskDataItem(std::string name, size_t slot, ListExpr contentType,
                 TaskDataLocation location, WorkerLocation preferredLocation)
        : preferredLocation(preferredLocation),
          name(name), slot(slot), contentType(contentType)
    {
        locations.push_back(location);
    }

    TaskDataItem(const distributed5::TaskDataItem &copy)
        : preferredLocation(copy.preferredLocation), locations(copy.locations),
          name(copy.name), slot(copy.slot), contentType(copy.contentType) {}

    std::string getName() const { return name; }
    size_t getSlot() const { return slot; }
    ListExpr getContentType() const { return contentType; }

    std::string toString() const
    {
        boost::lock_guard<boost::mutex> lock(mutex);
        std::string str = name + " _ " + std::to_string(slot) +
                          " <3 " + preferredLocation.toString();
        for (auto location : locations)
        {
            str += " @[" + location.toString() + "]";
        }
        return str;
    }

    std::string getObjectName() const
    {
        return name + "_" + std::to_string(slot);
    }

    const WorkerLocation &getPreferredLocation() const
    {
        return preferredLocation;
    }

    TaskDataLocation findLocation(WorkerLocation const &nearby) const;
    bool hasLocation(WorkerLocation const &nearby) const;
    TaskDataLocation findUpcomingLocation(WorkerLocation const &nearby) const;
    bool hasUpcomingLocation(WorkerLocation const &nearby) const;
    bool hasFileLocation(WorkerLocation const &nearby) const;
    TaskDataLocation findFileLocation(WorkerLocation const &nearby) const;
    std::pair<TaskDataLocation, int> findTransferSourceLocation(
        std::map<std::string, std::pair<bool, int>> activeTransferrators) const;
    TaskDataLocation getFirstLocation() const;
    std::vector<TaskDataLocation> getLocations() const;

    DataDistance getDistance(WorkerLocation const &location) const;
    DataDistance getUpcomingDistance(WorkerLocation const &location) const;

    void addLocation(TaskDataLocation location)
    {
        boost::lock_guard<boost::mutex> lock(mutex);
        locations.push_back(location);
        for (auto it = upcomingLocations.begin();
             it != upcomingLocations.end();
             it++)
        {
            if (*it == location)
            {
                upcomingLocations.erase(it);
                break;
            }
        }
    }

    void persistLocation(TaskDataLocation location)
    {
        boost::lock_guard<boost::mutex> lock(mutex);
        for (auto it = locations.begin(); it != locations.end(); it++)
        {
            if (*it == location)
            {
                *it = TaskDataLocation(
                    location.getWorkerLocation(),
                    location.getStorageType(), false);
                return;
            }
        }
    }

    void addUpcomingLocation(TaskDataLocation location)
    {
        boost::lock_guard<boost::mutex> lock(mutex);
        upcomingLocations.push_back(location);
    }

private:
    mutable boost::mutex mutex;
    WorkerLocation preferredLocation;
    std::vector<TaskDataLocation> locations;
    std::list<TaskDataLocation> upcomingLocations;
    std::string name;
    size_t slot;
    ListExpr contentType;
};

enum TaskFlag : int
{
    None = 0x0,
    Output = 0x1,

    CopyArguments = 0x2,
    FileArguments = 0x4,

    RunOnPreferedWorker = 0x10,
    RunOnPreferedServer = 0x20,
    RunOnReceive = 0x40,

    PreferSlotWorker = 0x100,
    PreferSlotServer = 0x200,
};

class Task
{
public:
    Task(int flags = 0)
        : flags(flags), id(nextId++) {}
    virtual ~Task();

    int getId();

    bool hasFlag(TaskFlag flag) { return bool(flags & flag); }
    void setFlag(TaskFlag flag) { flags = flags | flag; }
    void clearFlag(TaskFlag flag) { flags = flags & ~flag; }

    void addPredecessorTask(Task *t);
    std::vector<Task *> &getPredecessors();

    virtual std::string toString() const
    {
        return getTaskType() + " task";
    };

    virtual std::string getTaskType() const = 0;
    virtual TaskDataItem *run(
        WorkerLocation &location,
        std::vector<TaskDataItem *> args) = 0;

    static const std::string BasicType();
    static const bool checkType(const ListExpr list);
    static const ListExpr innerType(const ListExpr list)
    {
        return nl->Second(list);
    }
    static const ListExpr resultType(const ListExpr list)
    {
        return nl->Second(nl->Second(list));
    }

    static void runCommand(distributed2::ConnectionInfo *ci,
                           std::string cmd,
                           std::string description,
                           bool nestedListFormat = false,
                           bool ignoreFailure = false);

private:
    std::vector<Task *> listOfPre;

    int flags;
    size_t slot;
    static int nextId;
    int id;
};

// Data stored on a Worker
// might by in object and/or file form
// depending on storageType
class DataTask : public Task
{
public:
    DataTask(const distributed2::DArrayElement dArrayElement,
             std::string name,
             size_t slot,
             DataStorageType storageType,
             ListExpr contentType);

    virtual std::string getTaskType() const { return "data"; }

    virtual TaskDataItem *run(
        WorkerLocation &location,
        std::vector<TaskDataItem *> args);

private:
    TaskDataItem dataItem;
};

class WorkerTask : public Task
{
public:
    WorkerTask(const WorkerLocation location)
        : Task(RunOnReceive),
          location(location) {}

    WorkerTask(const distributed2::DArrayElement dArrayElement);

    virtual std::string getTaskType() const { return "worker"; }

    virtual std::string toString() const
    {
        return "worker " + location.toString();
    }

    virtual TaskDataItem *run(
        WorkerLocation &location,
        std::vector<TaskDataItem *> args)
    {
        return new TaskDataItem(std::string(""), 0, nl->TheEmptyList(),
                                TaskDataLocation(this->location, Object, false),
                                this->location);
    }

private:
    WorkerLocation location;
};

class ConvertToFileTask : public Task
{
public:
    ConvertToFileTask() : Task(FileArguments) {}

    virtual std::string getTaskType() const { return "convert to file"; }

    virtual TaskDataItem *run(
        WorkerLocation &location,
        std::vector<TaskDataItem *> args)
    {
        return args.front();
    }
};

class FunctionTask : public Task
{
protected:
    FunctionTask(std::string mapFunction,
                 std::string resultName,
                 ListExpr resultContentType,
                 bool isRel,
                 bool isStream)
        : Task(CopyArguments | PreferSlotWorker | PreferSlotServer),
          mapFunction(mapFunction),
          resultName(resultName),
          resultContentType(resultContentType),
          isRel(isRel),
          isStream(isStream) {}

public:
    std::string getFunction() { return mapFunction; }

    virtual std::string toString() const
    {
        return getTaskType() + "[" + resultName + ", " + mapFunction + "] => " +
               nl->ToString(resultContentType);
    }

protected:
    std::string mapFunction;
    std::string resultName;
    ListExpr resultContentType;
    bool isRel;
    bool isStream;

    TaskDataItem *store(
        const WorkerLocation &location, const WorkerLocation &preferredLocation,
        size_t slot, std::string value, std::string description);
};

class DmapFunctionTask : public FunctionTask
{
public:
    DmapFunctionTask(std::string mapFunction,
                     std::string resultName,
                     ListExpr resultContentType,
                     bool isRel,
                     bool isStream)
        : FunctionTask(mapFunction,
                       resultName,
                       resultContentType,
                       isRel,
                       isStream) {}

    virtual std::string getTaskType() const { return "dmap"; }

    virtual TaskDataItem *run(
        WorkerLocation &location,
        std::vector<TaskDataItem *> args);
};

class DproductFunctionTask : public FunctionTask
{
public:
    DproductFunctionTask(std::string mapFunction,
                         std::string resultName,
                         ListExpr resultContentType,
                         bool isRel,
                         bool isStream)
        : FunctionTask(mapFunction,
                       resultName,
                       resultContentType,
                       isRel,
                       isStream)
    {
        setFlag(FileArguments);
    }

    virtual std::string getTaskType() const { return "dproduct"; }

    virtual TaskDataItem *run(
        WorkerLocation &location,
        std::vector<TaskDataItem *> args);
};

class ErrorTask : public Task
{
    virtual std::string getTaskType() const { return "error"; }

    virtual TaskDataItem *run(
        WorkerLocation &location,
        std::vector<TaskDataItem *> args)
    {
        throw std::invalid_argument("Error task should not exit in stream");
    }
};

class RemoteException : public std::exception
{
public:
    RemoteException(std::string description, std::string error, std::string cmd)
        : description(description), error(error), cmd(cmd) {}
    const char *what() const throw()
    {
        if (message.empty())
        {
            message = description + " failed: " + error + "\n" +
                      "command = " + cmd;
        }
        return message.c_str();
    }

private:
    std::string description;
    std::string error;
    std::string cmd;
    mutable std::string message;
};

class NoNearbyLocationException : public std::exception
{
public:
    NoNearbyLocationException(const TaskDataItem *data, WorkerLocation nearby)
        : data(data), nearby(nearby) {}
    const char *what() const throw()
    {
        if (message.empty())
        {
            message = "TaskDataItem (" + data->toString() + ")" +
                      " is not stored nearby " + nearby.toString();
        }
        return message.c_str();
    }

private:
    const TaskDataItem *data;
    WorkerLocation nearby;
    mutable std::string message;
};

class NoSourceLocationException : public std::exception
{
public:
    NoSourceLocationException(const TaskDataItem *data)
        : data(data) {}
    const char *what() const throw()
    {
        if (message.empty())
        {
            message = "TaskDataItem (" + data->toString() + ")" +
                      " is not transferable";
        }
        return message.c_str();
    }

private:
    const TaskDataItem *data;
    mutable std::string message;
};

extern TypeConstructor TaskTC;
} // namespace distributed5

#endif
