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

#include "Task.h"

using namespace std;
using namespace distributed2;

extern boost::mutex nlparsemtx;
/*

0 Functions from distributed2Algebra

*/
namespace distributed2
{

// Algebra instance
extern Distributed2Algebra *algInstance;

extern DFSType *filesystem;

extern ListExpr replaceWrite(ListExpr list, const string &writeVer,
                             const string &name);

/*

0.1 Auxiliary function fun2cmd.

*/
extern void writeLog(ConnectionInfo *ci, const string &msg);
extern void writeLog(ConnectionInfo *ci, const string &cmd, const string &msg);
extern ListExpr replaceSymbols(ListExpr list,
                               const map<string, string> &replacements);
extern ListExpr replaceSymbols(ListExpr list,
                               const map<string, ListExpr> &replacements);
template <typename T>
ListExpr fun2cmd(ListExpr funlist, const vector<T> &funargs)
{
    if (!nl->HasLength(funlist, 2 + funargs.size()))
    {
        cout << "invalid length of list" << endl;
        return nl->TheEmptyList();
    }
    if (!listutils::isSymbol(nl->First(funlist), "fun"))
    {
        return nl->TheEmptyList();
    }
    funlist = nl->Rest(funlist);
    map<string, T> replacements;
    int pos = 0;
    while (!nl->HasLength(funlist, 1))
    {
        ListExpr first = nl->First(funlist);
        funlist = nl->Rest(funlist);
        if (!nl->HasLength(first, 2))
        {
            cerr << "invalid function argument" << endl;
            return nl->TheEmptyList();
        }
        if (nl->AtomType(nl->First(first)) != SymbolType)
        {
            cerr << "invalid function argument name " << endl;
            return nl->TheEmptyList();
        }
        replacements[nl->SymbolValue(nl->First(first))] = funargs[pos];
        pos++;
    }
    ListExpr rep = replaceSymbols(nl->First(funlist), replacements);
    return rep;
}

template <typename T>
ListExpr fun2cmd(const string &fundef, const vector<T> &funargs)
{
    ListExpr funlist;
    {
        boost::lock_guard<boost::mutex> guard(nlparsemtx);
        if (!nl->ReadFromString(fundef, funlist))
        {
            cerr << "Function is not a nested list" << endl;
            return nl->TheEmptyList();
        }
    }
    return fun2cmd<T>(funlist, funargs);
}

} // namespace distributed2

namespace distributed5
{

CommandLog commandLog;
int Task::nextId = 0;
thread_local TaskStatistics TaskStatistics::local;

/*

2.2 Constructor for Data Task Type

 Creates a  Data Task Type
 
 Root task for each slot
 
 This task has no incoming task
 
 Only outgoing tasks    

*/
DataTask::DataTask(
    const DArrayElement dArrayElement,
    string name,
    size_t slot,
    DataStorageType storageType,
    ListExpr contentType)
    : Task(RunOnReceive),
      dataItem(name, slot, contentType,
               TaskDataLocation(dArrayElement.getHost(),
                                dArrayElement.getPort(),
                                dArrayElement.getConfig(),
                                dArrayElement.getNum(),
                                storageType,
                                false),
               WorkerLocation(dArrayElement.getHost(),
                              dArrayElement.getPort(),
                              dArrayElement.getConfig(),
                              dArrayElement.getNum()))
{
}

WorkerTask::WorkerTask(
    const DArrayElement dArrayElement)
    : WorkerTask(WorkerLocation(dArrayElement.getHost(),
                                dArrayElement.getPort(),
                                dArrayElement.getConfig(),
                                dArrayElement.getNum()))
{
}

/*

2.4 Default Task Destructor

*/
Task::~Task()
{
}

/*

2.5 Basic Type - Task

*/
const string Task::BasicType() { return "task"; }

/*

2.6 Check Type for Type - Task

*/
const bool Task::checkType(const ListExpr list)
{
    if (!nl->HasLength(list, 2))
    {
        return false;
    }
    if (!listutils::isSymbol(nl->First(list), BasicType()))
    {
        return false;
    }
    return true;
}

//Adds a task to the list of predecessor tasks
void Task::addArgument(Task *task, size_t pos)
{
    arguments.emplace_back(task, pos);
}

vector<TaskDataItem *> DataTask::run(
    WorkerLocation &location,
    std::vector<TaskDataItem *> args)
{
    return vector{new TaskDataItem(dataItem)};
}

vector<TaskDataItem *> DmapFunctionTask::run(
    WorkerLocation &location,
    std::vector<TaskDataItem *> args)
{
    string resultName = this->resultName;

    TaskDataItem *firstArg = args.front();
    size_t slot = firstArg->getSlot();
    vector<string> funargs;

    // for all input arguments
    for (auto arg : args)
    {
        funargs.push_back(arg->getValueArgument(location) + " ");
    }
    // create function comand
    ListExpr funCmdList = fun2cmd(mapFunction, funargs);

    funCmdList = replaceWrite(funCmdList, "write2",
                              resultName + "_" + std::to_string(slot));

    // for dmapSX X == 1
    if (args.size() == 1)
    {
        string name_slot = firstArg->getObjectName();
        funCmdList = replaceWrite(funCmdList, "write3", name_slot);
    }
    string funcmd = nl->ToString(funCmdList);
    // store result on the running worker
    return store(location, firstArg->getPreferredLocation(),
                 slot, funcmd, "dmapS");
}

vector<TaskDataItem *> DproductFunctionTask::run(
    WorkerLocation &location,
    std::vector<TaskDataItem *> args)
{
    TaskDataItem *first = args.front();
    TaskDataItem *last = args.back();

    size_t slot = first->getSlot();

    string arg1 = first->getValueArgument(location);
    ListExpr fsrelType = nl->TwoElemList(
        listutils::basicSymbol<fsrel>(),
        nl->Second(last->getContentType()));
    string arg2 = "(" + nl->ToString(fsrelType) + "( ";
    bool isFirst = true;
    for (auto arg : args)
    {
        if (isFirst)
        {
            isFirst = false;
            continue;
        }
        TaskDataLocation loc = arg->findLocation(location,
                                                 DataStorageType::File);
        arg2 += "'" + loc.getFilePath(arg) + "' ";
    }
    arg2 += "))";
    string funcall = "( " + mapFunction + " " + arg1 + " " + arg2 + ")";

    return store(location, first->getPreferredLocation(),
                 slot, funcall, "dproductS");
}

vector<TaskDataItem *> PartitionFunctionTask::run(
    WorkerLocation &location,
    std::vector<TaskDataItem *> args)
{
    TaskDataItem *arg = args.front();

    size_t slot = arg->getSlot();

    TaskDataItem result(resultName, slot, resultContentType,
                        TaskDataLocation(
                            location, DataStorageType::File, false),
                        arg->getPreferredLocation());

    string path = location.getFileBase(&result);
    string argValue = arg->getValueArgument(location);
    string tupleStream = "( " + mapFunction + " " + argValue + ")";
    string distribute = "(fdistribute7 " + tupleStream + " " +
                        "'" + path + "' " +
                        partitionFunction + " " +
                        std::to_string(vslots) + " TRUE" +
                        ")";
    string cmd = "(query (count " + distribute + "))";
    string description = "partitionFS";

    auto *ci = location.getWorkerConnection();

    // create the target directory
    string targetDir = location.getFileDirectory(&result);

    string cd = "query createDirectory('" + targetDir + "', TRUE)";
    double duration = runCommand(ci, cd,
                                 "create directory for file", false, true);

    duration += runCommand(ci, cmd, description, true);
    TaskStatistics::report("remote " + description, duration);

    vector<TaskDataItem *> results;

    for (size_t i = 0; i < vslots; i++)
    {
        results.push_back(new TaskDataItem(
            resultName, slot, i + 1,
            resultContentType,
            TaskDataLocation(location, DataStorageType::File, true),
            arg->getPreferredLocation()));
    }

    return results;
}

vector<TaskDataItem *> CollectFunctionTask::run(
    WorkerLocation &location,
    std::vector<TaskDataItem *> args)
{
    TaskDataItem *first = args.front();
    TaskDataItem *last = args.back();

    size_t slot = first->getVerticalSlot() - 1;

    ListExpr fsrelType = nl->TwoElemList(
        listutils::basicSymbol<fsrel>(),
        nl->Second(last->getContentType()));
    string argQuery = "(" + nl->ToString(fsrelType) + "( ";
    bool isFirst = true;
    for (auto arg : args)
    {
        if (isFirst)
        {
            isFirst = false;
            continue;
        }
        TaskDataLocation loc = arg->findLocation(location,
                                                 DataStorageType::File);
        argQuery += "'" + loc.getFilePath(arg) + "' ";
    }
    argQuery += "))";

    return store(location, first->getPreferredLocation(),
                 slot, argQuery, "collectS");
}

//returns the DArray information.
//These informations are:
//server, port, slot and the config
DArrayElement WorkerLocation::getDArrayElement() const
{
    return DArrayElement(server, port, worker, config);
}

//returns the list of predecessor tasks
std::vector<Task *> Task::getPredecessors()
{
    vector<Task *> list;
    for (auto arg : getArguments())
        list.push_back(arg.first);
    return list;
}

//returns the list of predecessor tasks
std::vector<pair<Task *, size_t>> &Task::getArguments()
{
    return arguments;
}

//returns the id of the task
int Task::getId()
{
    return id;
}

std::string WorkerLocation::getFileBase(const TaskDataItem *data) const
{
    ConnectionInfo *ci = getWorkerConnection();
    string dbname = SecondoSystem::GetInstance()->GetDatabaseName();
    string name = data->getName();
    size_t slot = data->getSlot();
    size_t vslot = data->getVerticalSlot();
    string slotPrefix =
        vslot != 0
            ? "_" + std::to_string(slot) +
                  "_" + std::to_string(vslot - 1)
            : "_" + std::to_string(slot);
    return ci->getSecondoHome(false, commandLog) +
           "/dfarrays/" + dbname + "/" +
           name + "/" +
           name + slotPrefix;
}

std::string WorkerLocation::getFilePath(const TaskDataItem *data) const
{
    return getFileBase(data) + ".bin";
}

std::string WorkerLocation::getFileDirectory(const TaskDataItem *data) const
{
    ConnectionInfo *ci = getWorkerConnection();
    string dbname = SecondoSystem::GetInstance()->GetDatabaseName();
    string name = data->getName();
    return ci->getSecondoHome(false, commandLog) +
           "/dfarrays/" + dbname + "/" +
           name + "/";
}

std::string TaskDataLocation::getValueArgument(const TaskDataItem *data) const
{
    switch (storageType)
    {
    case DataStorageType::Object:
        return data->getObjectName();
    case DataStorageType::File:
    {
        string fname1 = getFilePath(data);

        ListExpr contentType = data->getContentType();
        if (data->isFileRelation())
        {
            return "(" + nl->ToString(contentType) +
                   " '" + fname1 + "' )";
        }
        else if (data->isObjectRelation())
        {
            ListExpr frelType = nl->TwoElemList(
                listutils::basicSymbol<frel>(),
                nl->Second(contentType));

            return "(" + nl->ToString(frelType) +
                   " '" + fname1 + "' )";
        }
        else
        {
            return "(getObjectFromFile '" + fname1 + "')";
        }
    }
    default:
        throw std::invalid_argument("not implemented storage type");
    }
}

double Task::runCommand(ConnectionInfo *ci,
                        std::string cmd,
                        std::string description,
                        bool nestedListFormat,
                        bool ignoreFailure,
                        bool ignoreError)
{
    bool showCommands = false;
    int err;
    string errMsg;
    string r;
    double runtime;
    if (nestedListFormat)
    {
        ci->simpleCommandFromList(cmd, err, errMsg, r, false, runtime,
                                  showCommands, commandLog, false,
                                  algInstance->getTimeout());
    }
    else
    {
        ci->simpleCommand(cmd, err, errMsg, r, false, runtime,
                          showCommands, commandLog, false,
                          algInstance->getTimeout());
    }
    if (err)
    {
        if (ignoreError)
            return 0;
        writeLog(ci, cmd, errMsg);
        throw RemoteException(
            description,
            errMsg +
                " (code: " + std::to_string(err) + " " +
                SecondoInterface::GetErrorMessage(err) + ")",
            cmd);
    }
    if (!ignoreFailure && r == "(bool FALSE)")
    {
        writeLog(ci, cmd, "returned FALSE");
        throw RemoteException(
            description,
            "command returned FALSE",
            cmd);
    }
    return runtime;
}

vector<TaskDataItem *> FunctionTask::store(
    const WorkerLocation &location, const WorkerLocation &preferredLocation,
    size_t slot, std::string value, std::string description)
{
    DataStorageType storageType =
        this->isStream || this->isRel || location != preferredLocation
            ? DataStorageType::File
            : DataStorageType::Object;

    ConnectionInfo *ci = location.getWorkerConnection();
    TaskDataItem result(resultName, slot, resultContentType,
                        TaskDataLocation(location, storageType, true),
                        preferredLocation);
    string name2 = result.getObjectName();
    string cmd;

    double duration = 0;

    switch (storageType)
    {
    case DataStorageType::Object:
    {
        cmd = "(let " + name2 + " = " + value + ")";
        break;
    }
    case DataStorageType::File:
    {
        // create the target directory
        string targetDir = location.getFileDirectory(&result);

        string cd = "query createDirectory('" + targetDir + "', TRUE)";
        duration += runCommand(ci, cd,
                               "create directory for file", false, true);

        string fname2 = location.getFilePath(&result);

        // if the result of the function is a relation, we feed it
        // into a stream to fconsume it
        // if there is a non-temp-name and a dfs is avaiable,
        // we extend the fconsume arguments by boolean values
        // first : create dir, always true
        // second : put result to dfs
        string aa = "";
        if (filesystem)
        {
            aa = " TRUE TRUE ";
        }
        if (this->isStream)
        {
            cmd = "(query (count (fconsume5 " + value + " '" +
                  fname2 + "'" + aa + " )))";
        }
        else if (this->isRel)
        {
            cmd = "(query (count (fconsume5 (feed " + value + " )'" +
                  fname2 + "' " + aa + ")))";
        }
        else
        {
            cmd = "(query (saveObjectToFile " + value + " '" + fname2 + "'))";
        }

        break;
    }
    default:
        throw std::invalid_argument("not implemented storage type");
    }

    duration += runCommand(ci, cmd, description, true);
    TaskStatistics::report("remote " + description, duration);

    return vector{new TaskDataItem(result)};
}

ConnectionInfo *WorkerLocation::getWorkerConnection() const
{
    string dbname = SecondoSystem::GetInstance()->GetDatabaseName();
    ConnectionInfo *ci = algInstance->getWorkerConnection(
        getDArrayElement(), dbname);
    if (ci == 0)
    {
        throw new RemoteException(
            "connect to worker",
            "ConnectionInfo == NULL",
            "(" + toString() + ").getWorkerConnection()");
    }
    return ci;
}

DataDistance TaskDataItem::getDistance(
    WorkerLocation const &location) const
{
    boost::shared_lock_guard<boost::shared_mutex> lock(mutex);
    DataDistance bestDistance = FarAway;
    if (fileLocations > 0)
    {
        bestDistance = DataDistance::FileOverNetwork;
    }
    else if (objectLocations > 0)
    {
        bestDistance = DataDistance::MemoryOverNetwork;
    }
    auto locations = locationsByServer.find(location.getServer());
    if (locations != locationsByServer.end())
    {
        for (auto &loc : locations->second)
        {
            DataDistance dist = loc.getDistance(location, isFileRelation());
            if (dist < bestDistance)
            {
                bestDistance = dist;
            }
        }
    }
    return bestDistance;
}

DataDistance TaskDataItem::getUpcomingDistance(
    WorkerLocation const &location) const
{
    DataDistance bestDistance = getDistance(location);
    boost::shared_lock_guard<boost::shared_mutex> lock(mutex);
    for (auto &loc : upcomingLocations)
    {
        DataDistance dist = loc.getDistance(location, isFileRelation());
        if (dist < bestDistance)
        {
            bestDistance = dist;
        }
    }
    return bestDistance;
}

pair<TaskDataLocation, int> TaskDataItem::findTransferSourceLocation(
    std::map<std::string, pair<bool, int>> activeTransferrators) const
{
    const TaskDataLocation *bestLoc = 0;
    int bestCount = 0;
    for (auto &locations : locationsByServer)
    {
        if (locations.second.empty())
            continue;
        auto transferrator = activeTransferrators[locations.first];
        if (!transferrator.first)
            continue;
        int count = transferrator.second;
        if (bestLoc == 0 || count < bestCount)
        {
            for (auto &loc : locations.second)
            {
                if (loc.getStorageType() != File)
                    continue;
                bestLoc = &loc;
                bestCount = count;
                break;
            }
        }
    }
    if (bestLoc == 0)
        throw NoSourceLocationException(this);
    return make_pair(*bestLoc, bestCount);
}

TaskDataLocation TaskDataItem::findLocation(WorkerLocation const &nearby) const
{
    boost::shared_lock_guard<boost::shared_mutex> lock(mutex);
    const TaskDataLocation *bestLoc = 0;
    DataDistance bestDistance = DataDistance::AccessibleWithCorrectType;
    auto locations = locationsByServer.find(nearby.getServer());
    if (locations != locationsByServer.end())
    {
        for (auto &loc : locations->second)
        {
            if (loc.getWorkerLocation() == nearby)
                return loc;
            DataDistance dist = loc.getDistance(nearby, isFileRelation());
            if (dist < bestDistance)
            {
                bestLoc = &loc;
                bestDistance = dist;
            }
        }
    }
    if (bestLoc == 0)
        throw NoNearbyLocationException(this, nearby);
    return *bestLoc;
}

bool TaskDataItem::hasLocation(TaskDataLocation const &location) const
{
    boost::shared_lock_guard<boost::shared_mutex> lock(mutex);
    auto locations = locationsByServer.find(location.getServer());
    if (locations != locationsByServer.end())
    {
        for (auto &loc : locations->second)
        {
            if (loc == location)
                return true;
        }
    }
    return false;
}

bool TaskDataItem::hasLocation(WorkerLocation const &nearby) const
{
    boost::shared_lock_guard<boost::shared_mutex> lock(mutex);
    auto locations = locationsByServer.find(nearby.getServer());
    if (locations != locationsByServer.end())
    {
        for (auto &loc : locations->second)
        {
            DataDistance dist = loc.getDistance(nearby, isFileRelation());
            if (dist <= DataDistance::AccessibleWithCorrectType)
            {
                return true;
            }
        }
    }
    return false;
}

bool TaskDataItem::hasLocation(WorkerLocation const &nearby,
                               DataStorageType storageType) const
{
    boost::shared_lock_guard<boost::shared_mutex> lock(mutex);
    auto locations = locationsByServer.find(nearby.getServer());
    if (locations != locationsByServer.end())
    {
        for (auto &loc : locations->second)
        {
            if (loc.getStorageType() != storageType)
                continue;
            DataDistance dist = loc.getDistance(nearby, isFileRelation());
            if (dist <= DataDistance::Accessible)
            {
                return true;
            }
        }
    }
    return false;
}

bool TaskDataItem::hasUpcomingLocation(WorkerLocation const &nearby) const
{
    boost::shared_lock_guard<boost::shared_mutex> lock(mutex);
    for (auto &loc : upcomingLocations)
    {
        DataDistance dist = loc.getDistance(nearby, isFileRelation());
        if (dist <= DataDistance::AccessibleWithCorrectType)
        {
            return true;
        }
    }
    auto locations = locationsByServer.find(nearby.getServer());
    if (locations != locationsByServer.end())
    {
        for (auto &loc : locations->second)
        {
            DataDistance dist = loc.getDistance(nearby, isFileRelation());
            if (dist <= DataDistance::AccessibleWithCorrectType)
            {
                return true;
            }
        }
    }
    return false;
}

bool TaskDataItem::hasUpcomingLocation(WorkerLocation const &nearby,
                                       DataStorageType storageType) const
{
    boost::shared_lock_guard<boost::shared_mutex> lock(mutex);
    for (auto &loc : upcomingLocations)
    {
        if (loc.getStorageType() != storageType)
            continue;
        DataDistance dist = loc.getDistance(nearby, isFileRelation());
        if (dist <= DataDistance::Accessible)
        {
            return true;
        }
    }
    auto locations = locationsByServer.find(nearby.getServer());
    if (locations != locationsByServer.end())
    {
        for (auto &loc : locations->second)
        {
            if (loc.getStorageType() != storageType)
                continue;
            DataDistance dist = loc.getDistance(nearby, isFileRelation());
            if (dist <= DataDistance::Accessible)
            {
                return true;
            }
        }
    }
    return false;
}

TaskDataLocation TaskDataItem::findUpcomingLocation(
    WorkerLocation const &nearby) const
{
    boost::shared_lock_guard<boost::shared_mutex> lock(mutex);
    const TaskDataLocation *bestLoc = 0;
    DataDistance bestDistance = DataDistance::AccessibleWithCorrectType;
    for (auto &loc : upcomingLocations)
    {
        DataDistance dist = loc.getDistance(nearby, isFileRelation());
        if (dist < bestDistance)
        {
            bestLoc = &loc;
            bestDistance = dist;
        }
    }
    auto locations = locationsByServer.find(nearby.getServer());
    if (locations != locationsByServer.end())
    {
        for (auto &loc : locations->second)
        {
            DataDistance dist = loc.getDistance(nearby, isFileRelation());
            if (dist < bestDistance)
            {
                bestLoc = &loc;
                bestDistance = dist;
            }
        }
    }
    if (bestLoc == 0)
        throw NoNearbyLocationException(this, nearby);
    return *bestLoc;
}

TaskDataLocation TaskDataItem::findLocation(
    WorkerLocation const &nearby,
    DataStorageType storageType) const
{
    const TaskDataLocation *bestLoc = 0;
    DataDistance bestDistance = DataDistance::Accessible;
    boost::shared_lock_guard<boost::shared_mutex> lock(mutex);
    auto locations = locationsByServer.find(nearby.getServer());
    if (locations != locationsByServer.end())
    {
        for (auto &loc : locations->second)
        {
            if (loc.getStorageType() != storageType)
                continue;
            DataDistance dist = loc.getDistance(nearby, isFileRelation());
            if (dist < bestDistance)
            {
                bestLoc = &loc;
                bestDistance = dist;
            }
        }
    }
    if (bestLoc == 0)
        throw NoNearbyLocationException(this, nearby);
    return *bestLoc;
}

TaskDataLocation TaskDataItem::getFirstLocation() const
{
    boost::shared_lock_guard<boost::shared_mutex> lock(mutex);
    for (auto &locations : locationsByServer)
        if (!locations.second.empty())
            return locations.second[0];
    throw std::invalid_argument("No locations");
}

std::vector<TaskDataLocation> TaskDataItem::getLocations() const
{
    boost::shared_lock_guard<boost::shared_mutex> lock(mutex);
    std::vector<TaskDataLocation> allLocations;
    for (auto &locations : locationsByServer)
    {
        for (auto &loc : locations.second)
            allLocations.push_back(loc);
    }
    return allLocations;
}

void TaskDataItem::merge(TaskDataItem *other)
{
    boost::lock_guard<boost::shared_mutex> lock(mutex);
    for (auto otherLocations : other->locationsByServer)
    {
        auto ownLocations = locationsByServer[otherLocations.first];
        for (auto loc : otherLocations.second)
        {
            bool found = false;
            for (auto ownLoc : ownLocations)
            {
                if (ownLoc == loc)
                {
                    found = true;
                    break;
                }
            }
            if (!found)
            {
                if (loc.getStorageType() == File)
                    fileLocations++;
                if (loc.getStorageType() == Object)
                    objectLocations++;
                ownLocations.push_back(loc);
            }
        }
    }
    other->locationsByServer.clear();
}

//these functions are needed for the type constructor
//sets the size of a task
int SizeOfTask()
{
    return 0;
}

//these functions are needed for the type constructor
//sets the task properties
ListExpr TaskProperty()
{
    return (
        nl->TwoElemList(
            nl->FourElemList(
                nl->StringAtom("Signature"),
                nl->StringAtom("Example Type List"),
                nl->StringAtom("List Rep"),
                nl->StringAtom("Example List")),
            nl->FourElemList(
                nl->StringAtom("-> SIMPLE"),
                nl->StringAtom("(Task, int)"),
                nl->StringAtom("List Rep"),
                nl->StringAtom("Example List"))));
}

//task type check
bool TaskTypeCheck(ListExpr type, ListExpr &errorInfo)
{
    return Task::checkType(type);
}

//creates a task
Word CreateTask(const ListExpr typeInfo)
{
    Word w;
    w.addr = (new ErrorTask());
    return w;
}

//deletes a task
void DeleteTask(const ListExpr typeInfo, Word &w)
{
    Task *k = (Task *)w.addr;
    delete k;
    w.addr = 0;
}

//type constructor
TypeConstructor TaskTC(
    Task::BasicType(),
    TaskProperty,
    0, 0,
    0, 0,
    CreateTask, DeleteTask,
    0, 0,
    0, 0,
    0, SizeOfTask, TaskTypeCheck);
} // namespace distributed5
