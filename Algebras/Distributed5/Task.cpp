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
void Task::addPredecessorTask(Task *t)
{
    listOfPre.push_back(t);
}

TaskDataItem *DataTask::run(
    WorkerLocation &location,
    std::vector<TaskDataItem *> args)
{
    return new TaskDataItem(dataItem);
}

TaskDataItem *DmapFunctionTask::run(
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
        // find shortest distance location
        auto loc = arg->findLocation(location);
        funargs.push_back(loc.getValueArgument(arg) + " ");
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

TaskDataItem *DproductFunctionTask::run(
    WorkerLocation &location,
    std::vector<TaskDataItem *> args)
{
    TaskDataItem *first = args.front();
    TaskDataItem *last = args.back();

    TaskDataLocation firstLoc = first->findLocation(location);
    size_t slot = first->getSlot();

    string arg1 = firstLoc.getValueArgument(first);
    ListExpr fsrelType = nl->TwoElemList(
        listutils::basicSymbol<fsrel>(),
        nl->Second(last->getContentType()));
    string arg2 = "(" + nl->ToString(fsrelType) + "( ";
    for (auto arg : args)
    {
        if (arg == first)
            continue;
        TaskDataLocation loc = arg->findFileLocation(location);
        arg2 += "'" + loc.getFilePath(arg) + "' ";
    }
    arg2 += "))";
    string funcall = "( " + mapFunction + " " + arg1 + " " + arg2 + ")";

    return store(location, first->getPreferredLocation(),
                 slot, funcall, "dproductS");
}

//returns the DArray information.
//These informations are:
//server, port, slot and the config
DArrayElement WorkerLocation::getDArrayElement() const
{
    return DArrayElement(server, port, worker, config);
}

//returns the list of predecessor tasks
std::vector<Task *> &Task::getPredecessors()
{
    return listOfPre;
}

//returns the id of the task
int Task::getId()
{
    return id;
}

std::string WorkerLocation::getFilePath(TaskDataItem *data) const
{
    ConnectionInfo *ci = getWorkerConnection();
    string dbname = SecondoSystem::GetInstance()->GetDatabaseName();
    string name = data->getName();
    size_t slot = data->getSlot();
    return ci->getSecondoHome(false, commandLog) +
           "/dfarrays/" + dbname + "/" +
           name + "/" +
           name + "_" +
           std::to_string(slot) + ".bin";
}

std::string WorkerLocation::getFileDirectory(TaskDataItem *data) const
{
    ConnectionInfo *ci = getWorkerConnection();
    string dbname = SecondoSystem::GetInstance()->GetDatabaseName();
    string name = data->getName();
    return ci->getSecondoHome(false, commandLog) +
           "/dfarrays/" + dbname + "/" +
           name + "/";
}

std::string TaskDataLocation::getValueArgument(TaskDataItem *data) const
{
    switch (storageType)
    {
    case DataStorageType::Object:
        return data->getObjectName();
    case DataStorageType::File:
    {
        string fname1 = getFilePath(data);

        ListExpr contentType = data->getContentType();
        if (Relation::checkType(contentType))
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

void Task::runCommand(ConnectionInfo *ci,
                      std::string cmd,
                      std::string description,
                      bool nestedListFormat,
                      bool ignoreFailure)
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
}

TaskDataItem *FunctionTask::store(
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

    switch (storageType)
    {
    case DataStorageType::Object:
    {
        if (this->isStream)
        {
            cmd = "(let " + name2 + " = (consume " + value + "))";
        }
        else
        {
            cmd = "(let " + name2 + " = " + value + ")";
        }
        break;
    }
    case DataStorageType::File:
    {
        // create the target directory
        string targetDir = location.getFileDirectory(&result);

        string cd = "query createDirectory('" + targetDir + "', TRUE)";
        runCommand(ci, cd, "create directory for file", false, true);

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

    runCommand(ci, cmd, description, true);

    return new TaskDataItem(result);
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
    boost::lock_guard<boost::mutex> lock(mutex);
    DataDistance bestDistance = MemoryOverNetwork;
    for (auto &loc : locations)
    {
        DataDistance dist = loc.getDistance(location);
        if (dist < bestDistance)
        {
            bestDistance = dist;
        }
    }
    return bestDistance;
}

DataDistance TaskDataItem::getUpcomingDistance(
    WorkerLocation const &location) const
{
    DataDistance bestDistance = getDistance(location);
    boost::lock_guard<boost::mutex> lock(mutex);
    for (auto &loc : upcomingLocations)
    {
        DataDistance dist = loc.getDistance(location);
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
    for (auto &loc : locations)
    {
        if (loc.getStorageType() != File)
            continue;
        auto transferrator = activeTransferrators[loc.getServer()];
        if (!transferrator.first)
            continue;
        int count = transferrator.second;
        if (bestLoc == 0 || count < bestCount)
        {
            bestLoc = &loc;
            bestCount = count;
        }
    }
    if (bestLoc == 0)
        throw NoSourceLocationException(this);
    return make_pair(*bestLoc, bestCount);
}

TaskDataLocation TaskDataItem::findLocation(WorkerLocation const &nearby) const
{
    boost::lock_guard<boost::mutex> lock(mutex);
    const TaskDataLocation *bestLoc = 0;
    DataDistance bestDistance = (DataDistance)(FileOnServer + 1);
    for (auto &loc : locations)
    {
        if (loc.getWorkerLocation() == nearby)
            return loc;
        DataDistance dist = loc.getDistance(nearby);
        if (dist < bestDistance)
        {
            bestLoc = &loc;
            bestDistance = dist;
        }
    }
    if (bestLoc == 0)
        throw NoNearbyLocationException(this, nearby);
    return *bestLoc;
}

bool TaskDataItem::hasLocation(WorkerLocation const &nearby) const
{
    boost::lock_guard<boost::mutex> lock(mutex);
    for (auto &loc : locations)
    {
        DataDistance dist = loc.getDistance(nearby);
        if (dist <= FileOnServer)
        {
            return true;
        }
    }
    return false;
}

bool TaskDataItem::hasFileLocation(WorkerLocation const &nearby) const
{
    boost::lock_guard<boost::mutex> lock(mutex);
    for (auto &loc : locations)
    {
        DataDistance dist = loc.getDistance(nearby);
        if (dist == FileOnServer)
        {
            return true;
        }
    }
    return false;
}

bool TaskDataItem::hasUpcomingLocation(WorkerLocation const &nearby) const
{
    boost::lock_guard<boost::mutex> lock(mutex);
    for (auto &loc : upcomingLocations)
    {
        DataDistance dist = loc.getDistance(nearby);
        if (dist <= FileOnServer)
        {
            return true;
        }
    }
    for (auto &loc : locations)
    {
        DataDistance dist = loc.getDistance(nearby);
        if (dist <= FileOnServer)
        {
            return true;
        }
    }
    return false;
}

TaskDataLocation TaskDataItem::findUpcomingLocation(
    WorkerLocation const &nearby) const
{
    boost::lock_guard<boost::mutex> lock(mutex);
    const TaskDataLocation *bestLoc = 0;
    DataDistance bestDistance = (DataDistance)(FileOnServer + 1);
    for (auto &loc : upcomingLocations)
    {
        DataDistance dist = loc.getDistance(nearby);
        if (dist < bestDistance)
        {
            bestLoc = &loc;
            bestDistance = dist;
        }
    }
    for (auto &loc : locations)
    {
        DataDistance dist = loc.getDistance(nearby);
        if (dist < bestDistance)
        {
            bestLoc = &loc;
            bestDistance = dist;
        }
    }
    if (bestLoc == 0)
        throw NoNearbyLocationException(this, nearby);
    return *bestLoc;
}

TaskDataLocation TaskDataItem::findFileLocation(
    WorkerLocation const &nearby) const
{
    boost::lock_guard<boost::mutex> lock(mutex);
    const TaskDataLocation *bestLoc = 0;
    DataDistance bestDistance = (DataDistance)(FileOnServer + 1);
    for (auto &loc : locations)
    {
        if (loc.getStorageType() != File)
            continue;
        DataDistance dist = loc.getDistance(nearby);
        if (dist < bestDistance)
        {
            bestLoc = &loc;
            bestDistance = dist;
        }
    }
    if (bestLoc == 0)
        throw NoNearbyLocationException(this, nearby);
    return *bestLoc;
}

TaskDataLocation TaskDataItem::getFirstLocation() const
{
    boost::lock_guard<boost::mutex> lock(mutex);
    return locations[0];
}

std::vector<TaskDataLocation> TaskDataItem::getLocations() const
{
    boost::lock_guard<boost::mutex> lock(mutex);
    return locations;
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
