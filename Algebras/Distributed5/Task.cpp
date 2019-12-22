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

0 Functions from distributed2Algebras

*/
namespace distributed2
{

// Algebra instance
extern Distributed2Algebra *algInstance;

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

1.1 Default Task Constructor

*/
Task::Task()
{
    this->taskType = undefined;
}

/*

1.2 Constructor for specific Task Type
    Creates a specific Task according to the Task Type

*/

Task::Task(TaskType taskType)
{
    this->taskType = taskType;
    nextId++;
    id = nextId;
}

/*

1.3 Constructor for Data Task Type

 creates a  Data Task Type
 Root task of each slot
 This task has no incoming task
 Only outgoing tasks    

*/
Task::Task(DArrayElement dArrayElement, string name, size_t slot)
{
    this->taskType = Data;
    this->name = name;
    this->server = dArrayElement.getHost();
    this->port = dArrayElement.getPort();
    this->config = dArrayElement.getConfig();
    this->worker = dArrayElement.getNum();
    this->slot = slot;
    nextId++;
    id = nextId;
}

/*

1.4 Function Task Type
this task can have incoming function or data tasks and outgoing function tasks

*/
Task::Task(string dmapFunction, string resultName)
{
    this->taskType = Function;
    this->dmapFunction = dmapFunction;
    this->resultName = resultName;
    nextId++;
    id = nextId;
}

/*

1.5 Default Task Destructor

*/
Task::~Task()
{
}

/*

1.5 Basic Type - Task

*/
const string Task::BasicType() { return "task"; }

/*

1.6 Check Type for Type - Task

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

//Adds a task to the list of successor tasks
void Task::addSuccessorTask(Task *t)
{
    listOfSucc.push_back(t);
    t->incNumberOfRemainingTasks();
}

//Adds a task to the list of predecessor tasks
void Task::addPredecessorTask(Task *t)
{
    listOfPre.push_back(t);
}

//returns the number of the remaining function tasks before
//this task can be executed
//if the number is 0 this task can be executed
int Task::getNumberOfRemainingTasks()
{
    return numberOfRemainingTasks;
}

//executes a function task
void Task::run()
{
    if (this->taskType == TaskType::Function)
    {
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
            if (t == listOfPre.begin())
            {
                this->server = previousDataTask->server;
                this->port = previousDataTask->port;
                this->config = previousDataTask->config;
                this->slot = previousDataTask->slot;
                this->worker = previousDataTask->worker;
            }
            funargs.push_back(previousDataTask->name + "_" +
                              std::to_string(slot) + " ");
        }

        //get dbname
        string dbname = SecondoSystem::GetInstance()->GetDatabaseName();
        //set name2 from the previous task
        string name2 = resultName + "_" +
                       stringutils::int2str(this->slot);

        //get worker connection for this slot
        ConnectionInfo *ci = algInstance->getWorkerConnection(
            DArrayElement(server, port, worker, config), dbname);

        ListExpr funCmdList = fun2cmd(dmapFunction, funargs);

        funCmdList = replaceWrite(funCmdList, "write2", name2);
        if (listOfPre.size() == 1)
        {
            string name_slot = listOfPre.front()->name + "_" +
                               std::to_string(slot);
            funCmdList = replaceWrite(funCmdList, "write3", name_slot);
        }
        string funcmd = nl->ToString(funCmdList);

        string cmd = "(let " + name2 + " = " + funcmd + ")";

        int err = 0;
        string errMsg;
        string r;
        double runtime;
        bool showCommands = false;
        if (!ci)
        {
            return;
        }
        //executes the simple command
        ci->simpleCommandFromList(cmd, err, errMsg, r, false, runtime,
                                  showCommands, commandLog, false,
                                  algInstance->getTimeout());

        if ((err != 0))
        {
            std::cout << "ERROR FROM TASK\n";
            showError(ci, cmd, err, errMsg);
            std::cout << "\nLOG FROM TASK\n";
            writeLog(ci, cmd, errMsg);
            std::cout << "\nfeddisch\n";
            this->taskType = TaskType::Error;
            return;
        }
        this->taskType = TaskType::Data;
        this->name = this->resultName;
        ;
    }
}

//increases the number or remaining tasks
//when a predecessor task is added this function is needed
void Task::incNumberOfRemainingTasks()
{
    numberOfRemainingTasks = numberOfRemainingTasks + 1;
}

//decreases the number or remaining tasks
//when a predecessor task is executed this function is needed
void Task::decNumberOfRemainingTasks()
{
    numberOfRemainingTasks = numberOfRemainingTasks - 1;
}

//decreases the number of remaining tasks for all successor tasks
//this is needed when the task is finished  an all successor tasks
//needs to be informed, that this task is finished.
void Task::decNumberOfRemainingTasksForSuccessors()
{
    for (size_t i = 0; i < listOfSucc.size(); i++)
    {
        listOfSucc[i]->decNumberOfRemainingTasks();
    }
}

//checks if a successor task can be started
//this is needed when the task is finished.
//All successor tasks need to be checked if they can be started.
vector<Task *> Task::checkSuccessorsTasksToStart()
{
    vector<Task *> allPossibleSuccessorTasksToStart;
    for (size_t i = 0; i < listOfSucc.size(); i++)
    {
        if (listOfSucc[i]->numberOfRemainingTasks == 0)
        {
            allPossibleSuccessorTasksToStart.push_back(listOfSucc[i]);
        }
    }
    return allPossibleSuccessorTasksToStart;
}

//checks if this task can be started.
bool Task::taskCanBeStarted()
{
    return (numberOfRemainingTasks == 0);
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

//returns the task as a string - needed for debugging...
string Task::getFunction()
{
    return dmapFunction;
}

//returns the task as a string - needed for debugging...
std::vector<Task *> Task::getSuccessors()
{
    return listOfSucc;
}

std::vector<Task *> Task::getPredecessor()
{
    return listOfPre;
}

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
    case TaskType::Function:
        ss << "Function " << resultName << " " << dmapFunction;
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
    w.addr = (new Task());
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
