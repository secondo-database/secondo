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

enum TaskType
{
    Error,
    // Data stored on a Worker
    // might by in object and/or file form
    // depending on storageType
    Data,
    // Copy Data from one location (first predecessor task)
    // to another location (second predecessor task)
    // data will always end up in file form if copied
    // might skip copying when data is already accessible
    // from target location
    CopyData,
    // Prepared data to be copied to another location
    // data in object form will be stored to file form
    // file transferators will be created if needed
    PrepareDataForCopy,
    Function_DMAPSX,
    Function_DPRODUCT
};

enum DataStorageType
{
    Object,
    File
};

class Task;

class TaskExecutionContext
{
public:
    TaskExecutionContext(int fileTransferPort);

    int getFileTransferPort() { return fileTransferPort; }

    std::optional<boost::unique_lock<boost::recursive_mutex>>
    checkOpenFileTransferator(distributed2::ConnectionInfo *ci);

    std::pair<std::optional<boost::unique_lock<boost::recursive_mutex>>, Task *>
    checkFileAvailable(Task *dataTask, std::string server, Task *resultTask);

    void debug(std::string message);

private:
    int fileTransferPort;

    boost::recursive_mutex openFileTransferatorsMutex;
    std::map<
        distributed2::ConnectionInfo *,
        boost::recursive_mutex>
        openFileTransferators;

    boost::recursive_mutex fileDataAvailableMutex;
    std::map<
        std::pair<Task *, std::string>,
        std::pair<boost::recursive_mutex, Task *>>
        fileDataAvailable;

    boost::recursive_mutex debugMutex;
};

class Task
{
public:
    Task(TaskType taskType);
    Task(distributed2::DArrayElement dArrayElement,
         std::string name,
         size_t slot,
         DataStorageType storageType,
         ListExpr contentType);
    Task(TaskType taskType,
         std::string mapFunction,
         std::string resultName,
         ListExpr resultContentType,
         bool isRel,
         bool isStream);
    ~Task();
    static const std::string BasicType();
    static const bool checkType(const ListExpr list);
    TaskType getTaskType();
    void addPredecessorTask(Task *t);
    void run(TaskExecutionContext &context);
    distributed2::DArrayElement GetDArrayElement();
    std::string getName();
    size_t getSlot();
    DataStorageType getStorageType();
    std::string getServer();
    int getPort();
    int getWorker();
    ListExpr getContentType();
    bool isLeaf();
    void setLeaf(bool leaf);
    std::string toString();
    std::string getFunction();
    int getId();
    static int nextId;
    std::vector<Task *> &getPredecessor();

    static const ListExpr innerType(const ListExpr list)
    {
        return nl->Second(list);
    }

    static const ListExpr resultType(const ListExpr list)
    {
        return nl->Second(nl->Second(list));
    }

private:
    TaskType taskType;
    std::vector<Task *> listOfPre;

    //Relevant information for TaskType Data:
    std::string server;
    int port;
    std::string config;
    int worker;
    std::string name;
    size_t slot;
    DataStorageType storageType;
    ListExpr contentType;

    //Relevant information for TaskType Function:
    std::string mapFunction;
    std::string resultName;
    ListExpr resultContentType;
    bool isRel;
    bool isStream;

    //Relevant information for TaskType Error:
    std::string errorMessage;

    //task attributes
    bool leaf = true;
    int id;

    //helpers
    void convertToData(Task *sourceTask)
    {
        taskType = sourceTask->taskType;
        server = sourceTask->server;
        port = sourceTask->port;
        slot = sourceTask->slot;
        config = sourceTask->config;
        worker = sourceTask->worker;
        name = sourceTask->name;
        contentType = sourceTask->contentType;
        storageType = sourceTask->storageType;
    }

    distributed2::ConnectionInfo *getWorkerConnection();

    std::string getObjectName()
    {
        return name + "_" + std::to_string(slot);
    }

    std::string getResultObjectName()
    {
        return resultName + "_" + std::to_string(slot);
    }

    std::string getFilePath();

    std::string getResultFileDir(distributed2::ConnectionInfo *ci);

    std::string getResultFilePath(distributed2::ConnectionInfo *ci);

    std::string getValueArgument();

    bool store(distributed2::ConnectionInfo *ci,
               std::string value, std::string description);

    bool runCommand(distributed2::ConnectionInfo *ci,
                    std::string cmd,
                    std::string description,
                    bool nestedListFormat = false,
                    bool ignoreFailure = false);
};

extern TypeConstructor TaskTC;
} // namespace distributed5

#endif
