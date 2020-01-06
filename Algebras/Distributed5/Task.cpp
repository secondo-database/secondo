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

1.1 Default TaskExecutionContext Constructor

*/
TaskExecutionContext::TaskExecutionContext(int _fileTransferPort)
    : fileTransferPort(_fileTransferPort)
{
}

std::optional<boost::unique_lock<boost::recursive_mutex>>
TaskExecutionContext::checkOpenFileTransferator(ConnectionInfo *ci)
{
    boost::recursive_mutex *mutex;
    {
        boost::lock_guard<boost::recursive_mutex> lock(
            openFileTransferatorsMutex);
        auto it = openFileTransferators.find(ci);
        if (it == openFileTransferators.end())
        {
            // this creates a new mutex and locks it
            return std::optional<boost::unique_lock<boost::recursive_mutex>>(
                std::in_place,
                openFileTransferators[ci]);
        }
        mutex = &it->second;
    }
    // this waits until the mutex is unlocked
    boost::lock_guard<boost::recursive_mutex> wait(*mutex);
    return std::optional<boost::unique_lock<boost::recursive_mutex>>();
}

std::pair<std::optional<boost::unique_lock<boost::recursive_mutex>>, Task *>
TaskExecutionContext::checkFileAvailable(
    Task *dataTask, std::string server, Task *resultTask)
{
    std::pair<Task *, std::string> key = make_pair(dataTask, server);
    boost::recursive_mutex *mutex;
    {
        boost::lock_guard<boost::recursive_mutex> lock(
            fileDataAvailableMutex);
        auto it = fileDataAvailable.find(key);
        if (it == fileDataAvailable.end())
        {
            auto &pair = fileDataAvailable[key];
            pair.second = resultTask;
            // this creates a new mutex and locks it
            return make_pair(
                std::optional<boost::unique_lock<boost::recursive_mutex>>(
                    std::in_place,
                    pair.first),
                resultTask);
        }
        mutex = &it->second.first;
        resultTask = it->second.second;
    }
    // this waits until the mutex is unlocked
    boost::lock_guard<boost::recursive_mutex> wait(*mutex);
    return make_pair(
        std::optional<boost::unique_lock<boost::recursive_mutex>>(),
        resultTask);
}

void TaskExecutionContext::debug(std::string message)
{
    boost::lock_guard<boost::recursive_mutex> lock(debugMutex);
    cout << message << endl;
}

/*

2.1 Constructor for specific Task Type
    
Creates a specific Task according to the Task Type

*/

Task::Task(TaskType taskType)
{
    this->taskType = taskType;
    nextId++;
    id = nextId;
}

/*

2.2 Constructor for Data Task Type

 Creates a  Data Task Type
 
 Root task for each slot
 
 This task has no incoming task
 
 Only outgoing tasks    

*/
Task::Task(
    DArrayElement dArrayElement,
    string name,
    size_t slot,
    DataStorageType storageType,
    ListExpr contentType)
{
    this->taskType = Data;
    this->name = name;
    this->server = dArrayElement.getHost();
    this->port = dArrayElement.getPort();
    this->config = dArrayElement.getConfig();
    this->worker = dArrayElement.getNum();
    this->slot = slot;
    this->storageType = storageType;
    this->contentType = contentType;
    nextId++;
    id = nextId;
}

/*

2.3 Function Task Type

This task can have incoming function or data tasks and outgoing function tasks

*/
Task::Task(TaskType taskType, string mapFunction,
           string resultName, ListExpr resultContentType,
           bool isRel, bool isStream)
{
    this->taskType = taskType;
    this->mapFunction = mapFunction;
    this->resultName = resultName;
    this->isRel = isRel;
    this->isStream = isStream;
    this->resultContentType = resultContentType;
    nextId++;
    id = nextId;
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

//executes a function task
void Task::run(TaskExecutionContext &context)
{
    switch (this->taskType)
    {
    case TaskType::Error:
    case TaskType::Data:
        // nothing to do
        break;
    case TaskType::PrepareDataForCopy:
    {
        Task *task = listOfPre.front();
        convertToData(task);

        ConnectionInfo *ci = getWorkerConnection();

        // Save Object in a file
        if (storageType == DataStorageType::Object)
        {
            storageType = DataStorageType::File;
            string oname = getObjectName();
            string fname = getFilePath();
            string cmd = "query " + oname +
                         " saveObjectToFile['" + fname + "']";
            if (!runCommand(ci, cmd, "save object to file"))
                return;
        }

        {
            auto lock = context.checkOpenFileTransferator(ci);
            if (lock)
            {
                string cmd = "query staticFileTransferator(" +
                             std::to_string(context.getFileTransferPort()) +
                             ",10)";
                runCommand(ci, cmd, "open file transferator", false, true);
            }
        }
        break;
    }
    case TaskType::CopyData:
    {
        Task *contentTask = listOfPre.front();
        Task *targetTask = listOfPre.back();
        // Data from contentTask should be made available
        // to targetTask's worker

        // Check if content worker is target worker
        if (contentTask->worker == targetTask->worker)
        {
            convertToData(contentTask);
            break;
        }

        // Check if content is already addressable from target
        // Lock mutex for this content-ip-pair to handle parallelity
        auto pair = context.checkFileAvailable(
            contentTask, targetTask->getServer(), this);
        if (!pair.first)
        {
            // Data is already available in second arguments task
            convertToData(pair.second);
            return;
        }

        // Check if data is on the same server
        // We can reference the data from the other worker
        // via file reference
        if (contentTask->server == targetTask->server)
        {
            convertToData(contentTask);
            return;
        }

        // Copy location info from targetTask and
        // data info from contentTask
        convertToData(targetTask);
        name = contentTask->name;
        slot = contentTask->slot;
        storageType = DataStorageType::File; // copied data is always a file
        contentType = contentTask->contentType;

        // Need to copy content to target
        // TODO choose target via round-robin
        // TODO check if file already exists, e. g. by prev copy
        ConnectionInfo *targetCI = targetTask->getWorkerConnection();
        ConnectionInfo *sourceCI = contentTask->getWorkerConnection();

        string cmd = string("query getFileTCP(") +
                     "'" + contentTask->getFilePath() + "', " +
                     "'" + sourceCI->getHost() + "', " +
                     std::to_string(context.getFileTransferPort()) + ", " +
                     "TRUE, " +
                     "'" + getFilePath() + "')";
        if (!runCommand(targetCI, cmd, "transfer file"))
            return;
        break;
    }
    case TaskType::Function_DMAPSX:
    {
        string resultName = this->resultName;

        Task *firstTask = listOfPre.front();
        convertToData(firstTask);

        //get worker connection for this slot
        ConnectionInfo *ci = getWorkerConnection();

        //Get Data Information from Previous Task to current
        //task and make current
        //task to data task (root of all tasks of the
        // dependency graph for this slot)
        //creates the function of the fun arguments
        vector<string> funargs;
        for (auto t = listOfPre.begin(); t != listOfPre.end(); ++t)
        {
            Task *previousDataTask = *t;
            if (previousDataTask->taskType == TaskType::Error)
            {
                this->taskType = TaskType::Error;
                return;
            }
            funargs.push_back(previousDataTask->getValueArgument() + " ");
        }

        //set name2 from the previous task
        string name2 = getResultObjectName();

        ListExpr funCmdList = fun2cmd(mapFunction, funargs);

        funCmdList = replaceWrite(funCmdList, "write2", name2);
        if (listOfPre.size() == 1)
        {
            string name_slot = firstTask->getObjectName();
            funCmdList = replaceWrite(funCmdList, "write3", name_slot);
        }
        string funcmd = nl->ToString(funCmdList);

        if (!store(ci, funcmd, "dmapS"))
            return;
        break;
    }
    case TaskType::Function_DPRODUCT:
    {
        Task *first = listOfPre.front();
        Task *last = listOfPre.back();

        convertToData(first);

        ConnectionInfo *ci = first->getWorkerConnection();
        string arg1 = first->getValueArgument();
        ListExpr fsrelType = nl->TwoElemList(
            listutils::basicSymbol<fsrel>(),
            nl->Second(last->contentType));
        string arg2 = "(" + nl->ToString(fsrelType) + "( ";
        for (auto it = listOfPre.begin() + 1; it != listOfPre.end(); it++)
        {
            Task *t = *it;
            arg2 += "'" + t->getFilePath() + "' ";
        }
        arg2 += "))";
        string funcall = "( " + mapFunction + " " + arg1 + " " + arg2 + ")";

        if (!store(ci, funcall, "dproductS"))
            return;
        break;
    }
    }
}

//returns the DArray information.
//These informations are:
//server, port, slot and the config
DArrayElement Task::GetDArrayElement()
{
    return DArrayElement(server, port, worker, config);
}

//returns the name of the relation in this slot
string Task::getName()
{
    return name;
}

//returns the slot on which the task has to start
size_t Task::getSlot()
{
    return slot;
}

DataStorageType Task::getStorageType()
{
    return storageType;
}

//returns the task as a string - needed for debugging...
string Task::getFunction()
{
    return mapFunction;
}

//returns the server on which the task has to start
std::string Task::getServer()
{
    return server;
}

//returns the port on which the task has to start
int Task::getPort()
{
    return port;
}

//returns the worker on which the task has to start
int Task::getWorker()
{
    return worker;
}

ListExpr Task::getContentType()
{
    return contentType;
}

//returns if the task is a leaf
bool Task::isLeaf()
{
    return leaf;
}

//returns task type
TaskType Task::getTaskType()
{
    return this->taskType;
}
//sets the attribute leaf on the task
void Task::setLeaf(bool leaf)
{
    this->leaf = leaf;
}

//returns the list of predecessor tasks
std::vector<Task *> &Task::getPredecessor()
{
    return listOfPre;
}

//returns the id of the task
int Task::getId()
{
    return id;
}

//returns the task as a string - needed for debugging...
string Task::toString()
{
    stringstream ss;

    switch (taskType)
    {
    case TaskType::Data:
        ss << "Data " << name << " " << slot << " on " << worker;
        break;
    case TaskType::Function_DMAPSX:
        ss << "Function_DMAPSX " << resultName << " " << mapFunction;
        break;
    case TaskType::Error:
        ss << "Error ";
        break;
    default:
        ss << "???";
        break;
    }
    return ss.str();
}

std::string Task::getFilePath()
{
    ConnectionInfo *ci = getWorkerConnection();
    string dbname = SecondoSystem::GetInstance()->GetDatabaseName();
    return ci->getSecondoHome(false, commandLog) +
           "/dfarrays/" + dbname + "/" +
           name + "/" +
           name + "_" +
           std::to_string(slot) + ".bin";
}

std::string Task::getResultFileDir(ConnectionInfo *ci)
{
    string dbname = SecondoSystem::GetInstance()->GetDatabaseName();
    return ci->getSecondoHome(false, commandLog) +
           "/dfarrays/" + dbname + "/" +
           resultName + "/";
}

std::string Task::getResultFilePath(ConnectionInfo *ci)
{
    return getResultFileDir(ci) +
           resultName + "_" +
           std::to_string(slot) + ".bin";
}

std::string Task::getValueArgument()
{
    switch (storageType)
    {
    case DataStorageType::Object:
        return getObjectName();
    case DataStorageType::File:
    {
        string fname1 = getFilePath();
        ListExpr frelType = nl->TwoElemList(
            listutils::basicSymbol<frel>(),
            nl->Second(contentType));

        return "(" + nl->ToString(frelType) +
               " '" + fname1 + "' )";
        break;
    }
    default:
        throw std::invalid_argument("not implemented storage type");
    }
}

bool Task::runCommand(ConnectionInfo *ci,
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
        cerr << description << " failed, cmd = " << cmd << endl;
        cerr << "message : " << errMsg << endl;
        cerr << "code : " << err << endl;
        cerr << "meaning : " << SecondoInterface::GetErrorMessage(err) << endl;
        writeLog(ci, cmd, errMsg);
        this->taskType = TaskType::Error;
        this->errorMessage = errMsg;
        return false;
    }
    if (!ignoreFailure && r == "(bool FALSE)")
    {
        cerr << description << " failed, cmd = " << cmd << endl;
        cerr << "command returned FALSE" << endl;
        writeLog(ci, cmd, "returned FALSE");
        this->taskType = TaskType::Error;
        this->errorMessage = "returned FALSE";
        return false;
    }
    return true;
}

bool Task::store(ConnectionInfo *ci, std::string value, std::string description)
{
    string name2 = getResultObjectName();
    string cmd;

    storageType = this->isStream || this->isRel
                      ? DataStorageType::File
                      : DataStorageType::Object;

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
        string targetDir = getResultFileDir(ci);

        string cd = "query createDirectory('" + targetDir + "', TRUE)";
        runCommand(ci, cd, "create directory for file", false, true);

        string fname2 = getResultFilePath(ci);

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
        else
        {
            cmd = "(query (count (fconsume5 (feed " + value + " )'" +
                  fname2 + "' " + aa + ")))";
        }

        break;
    }
    default:
        throw std::invalid_argument("not implemented storage type");
    }

    if (!runCommand(ci, cmd, description, true))
        return false;

    this->name = resultName;
    this->contentType = resultContentType;

    return true;
}

ConnectionInfo *Task::getWorkerConnection()
{
    string dbname = SecondoSystem::GetInstance()->GetDatabaseName();
    return algInstance->getWorkerConnection(
        DArrayElement(server, port, worker, config), dbname);
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
    w.addr = (new Task(TaskType::Error));
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
