/*
----
This file is part of SECONDO.

Copyright (C) 2019,
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


#ifndef DISTRIBUTEDJOB_H
#define DISTRIBUTEDJOB_H

#include "DistributedJob.h"

DistributedJob::DistributedJob(std::string _id, std::string _started,
    std::unordered_map<int, Supplier> _observableNodes,
    std::string _query, std::string _tree, std::string _nestedList,
std::shared_ptr<std::unordered_map<int, std::string>> _observableNodesWithNames,
std::shared_ptr<std::unordered_map<int, std::string>> 
_observableNodesWithArguments)
{
    id = _id;
    started = _started;
    finished = "";
    status = JOB_STATUS_INIT;
    file = getFile();
    observableNodes = _observableNodes;
    observableNodesList = _observableNodes;
    populateObservableNodeIds(_observableNodes);
    query = _query;
    nestedList = _nestedList;
    tree = _tree;
    observableNodesWithNames = _observableNodesWithNames;
    observableNodesWithArguments = _observableNodesWithArguments;
    workerStatus = 
    std::make_shared
    <std::unordered_map<std::string, std::shared_ptr<WorkerStatus>>>();
}

DistributedJob::~DistributedJob()
{
    //
}

std::string DistributedJob::getId()
{
    return id;
}

std::shared_ptr
<std::unordered_map<std::string, std::shared_ptr<WorkerStatus>>> 
DistributedJob::getWorkers()
{
    return workerStatus;
}

void DistributedJob::setWorkers
(std::shared_ptr
<std::unordered_map<std::string, std::shared_ptr<WorkerStatus>>> workers)
{
    workerStatus = workers;
}

std::string DistributedJob::getStarted()
{
    return started;
}

std::string DistributedJob::getFinished()
{
    return finished;
}

std::string DistributedJob::getStatus()
{
    return status;
}

std::string DistributedJob::getFile()
{
    return getJobNameForLog() + ".json";
}

int DistributedJob::getNumOfWorkers()
{
    return workerStatus.get()->size();
}

std::string DistributedJob::getJobNameForLog()
{
    if (!id.empty() && !std::to_string(nowWorkingOnNodeId).empty())
    {
        return id + "_node_" + std::to_string(nowWorkingOnNodeId);
    }
    else
    {
        return "";
    }
}

std::string DistributedJob::getTree()
{
    return tree;
}

void DistributedJob::finishJob(std::string time)
{
    finished = time;
    status = JOB_STATUS_FINISHED;
}

void DistributedJob::setCanceled()
{
    status = JOB_STATUS_CANCELED;
}

void DistributedJob::clearObservableNodes()
{
    if(observableNodes.size() > 0){
        observableNodes.clear();
    }

    if(observableNodesList.size() > 0){
        observableNodesList.clear();
    }
}

bool DistributedJob::setRunning()
{
    if (status == JOB_STATUS_RUNNING)
    {
        return false;
    }
    status = JOB_STATUS_RUNNING;
    return true;
}

std::unordered_map<int, Supplier> DistributedJob::getObservableNodes()
{
    return observableNodes;
}

std::unordered_map<int, Supplier> DistributedJob::getObservableNodesList()
{
    return observableNodesList;
}

std::vector<int> DistributedJob::getReadyNodeIds()
{
    std::vector<int> defs;
    for (auto node : processedNodesList)
    {
        defs.push_back(node.first);
    }
    return defs;
}

std::string DistributedJob::getQuery()
{
    return query;
}

int DistributedJob::getNowWorkingOnNodeId()
{
    return nowWorkingOnNodeId;
}

void DistributedJob::populateObservableNodeIds
(std::unordered_map<int, Supplier> _obsNodes)
{
    for (auto node : _obsNodes)
    {
        observableNodeIds.push_back(node.first);
    }
}

std::vector<int> DistributedJob::getObservableNodeIds()
{
    return observableNodeIds;
}

bool DistributedJob::allNodesProcessed()
{
    return (observableNodesList.size() == processedNodesList.size());
}

bool DistributedJob::allWorkersFinished()
{
    for 
    (auto it = workerStatus.get()->begin(); 
    it != workerStatus.get()->end(); ++it)
    {
        auto &w = *it;
        if (!w.second->allSlotsFinished())
        {
            return false;
            break;
        }
    }
    return true;
}

void DistributedJob::setNumOfSlotsPerWorker(int num)
{
    if (numOfslotsPerWorker == -1)
    {
        numOfslotsPerWorker = num;
    }
}

int DistributedJob::getNumOfSlotsPerWorker()
{
    return numOfslotsPerWorker;
}

std::string DistributedJob::getNestedList()
{
    return nestedList;
}

std::shared_ptr
<std::unordered_map<int, std::string>>
DistributedJob::getObservableNodesWithNames()
{
    return observableNodesWithNames;
}

std::shared_ptr
<std::unordered_map<int, std::string>>
DistributedJob::getObservableNodesWithArguments()
{
    return observableNodesWithArguments;
}

std::vector<std::string> DistributedJob::getFiles()
{
    return files;
}

void DistributedJob::pushFile(std::string file)
{
    files.push_back(file);
}

#endif