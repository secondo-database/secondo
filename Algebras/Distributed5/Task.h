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
#include "Algebras/Stream/Stream.h"
#include "Algebras/Distributed2/DArray.h"
#include "Algebras/Distributed2/Distributed2Algebra.h"
#include "Algebras/Distributed2/ConnectionInfo.h"

#include <boost/bind.hpp>
#include <boost/ref.hpp>
#include <boost/thread.hpp>

namespace distributed5{

enum TaskType
{
    undefined,
    Data,
    Function,
    Error
};

class Task{
public:
    Task();
    Task(TaskType taskType);
    Task(distributed2::DArrayElement dArrayElement, 
            std::string name, 
            size_t slot);
    Task(std::string dmapFunction, std::string resultName);
    ~Task();
    static const std::string BasicType();
    static const bool checkType(const ListExpr list);
    TaskType getTaskType();
    void addSuccessorTask(Task *t);
    void addPredecessorTask(Task *t);
    int getNumberOfRemainingTasks();
    void run();
    void incNumberOfRemainingTasks();
    void decNumberOfRemainingTasks();
    void decNumberOfRemainingTasksForSuccessors();
    std::vector<Task *> checkSuccessorsTasksToStart();
    bool taskCanBeStarted();
    distributed2::DArrayElement GetDArrayElement();
    std::string getName();
    size_t getSlot();
    bool isLeaf();
    void setLeaf(bool leaf);
    std::string toString();

    private:
    bool taskStarted = false;
    bool taskFinished = false;
    int numberOfRemainingTasks = 0;
    std::vector<Task*> listOfSucc;
    std::vector<Task *> listOfPre;
    std::string slotID;
    TaskType taskType;

    //Relevant information for TaskType Data:
    std::string server;
    int port;
    std::string name;
    size_t slot;
    std::string config;
    int worker;

    //Relevant information for TasksType Function:
    std::string dmapFunction;
    std::string resultName;
    //task attributes
    bool leaf = true;
};

extern TypeConstructor TaskTC;
}

#endif
