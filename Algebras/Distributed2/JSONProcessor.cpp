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

#include <iostream>
#include <fstream>
#include <iomanip>
#include "JSONProcessor.h"

JSONProcessor::~JSONProcessor() {}

void JSONProcessor::insertUpdateJournal(std::shared_ptr<DistributedJob> job)
{
    json JSON;
    json jobObject = createJournalJobStructure(job);
    JSON[job->getId()] = jobObject;
    std::ofstream o(getLogLocation());
    o << std::setw(4) << JSON << std::endl;
}

void JSONProcessor::createWorkerLog(std::shared_ptr<DistributedJob> job)
{
    if (!job->getFile().empty() && job->getFile() != "")
    {
        std::string logLocation = DEFAULT_LOG_DIRECTORY + "/" + job->getFile();
        json workerLogStructure = createWorkerLogStructure(job);
        std::ofstream o(logLocation);
        o << std::setw(4) << workerLogStructure << std::endl;
    }
}

// structures
json JSONProcessor::createJournalJobStructure
(std::shared_ptr<DistributedJob> job)
{
    json jobStructure = {};

    jobStructure = {
        {"current_file", job->getFile()},
        {"query", job->getQuery()},
        {"nestedList", job->getNestedList()},
        {"finished", job->getFinished()},
        {"id", job->getId()},
        {"numOfWorkers", job->getNumOfWorkers()},
        {"started", job->getStarted()},
        {"status", job->getStatus()},
        {"tree", job->getTree()},
        {"numOfSlotsPerWorker", job->getNumOfSlotsPerWorker()}};

    jobStructure["files"] = job->getFiles();

    if (!job->allNodesProcessed())
    {
        jobStructure["nowWorkingOnNodeId"] = job->getNowWorkingOnNodeId();
    }
    jobStructure["observableNodesIds"] = job->getObservableNodeIds();
    if (!job->getObservableNodeIds().empty())
    {
        jobStructure["readyNodesIds"] = job->getReadyNodeIds();
    }

    jobStructure["nodeOperatorMapping"] = createNodeOperatorMapping(job);

    jobStructure["nodeArgumentsMapping"] = createNodeArgumentsMapping(job);

    return jobStructure;
}

json JSONProcessor::createNodeOperatorMapping
(std::shared_ptr<DistributedJob> job)
{
    json mapping = {};
    for (
        auto it = job->getObservableNodesWithNames()->begin();
         it != job->getObservableNodesWithNames()->end();
          ++it)
    {
        auto &w = *it;
        std::string id = std::to_string(w.first);
        mapping[id] = w.second;
    }

    return mapping;
}

json JSONProcessor::createNodeArgumentsMapping
(std::shared_ptr<DistributedJob> job)
{
    json mapping = {};
    for (auto it = job->getObservableNodesWithArguments()->begin();
     it != job->getObservableNodesWithArguments()->end(); ++it)
    {
        auto &w = *it;
        std::string id = std::to_string(w.first);
        mapping[id] = w.second;
    }

    return mapping;
}

json JSONProcessor::createWorkerLogStructure
(std::shared_ptr<DistributedJob> job)
{
    json workerLog = {};
    workerLog["job"] = {{"id", job->getId()}};

    for (auto it = job->getWorkers()->begin();
     it != job->getWorkers()->end(); ++it)
    {
        auto &w = *it;
        std::string wId = w.first;
        workerLog["node"] = std::to_string(w.second->getNodeId());
        workerLog[wId] = createWorkerStatusStructure(w.second);
    }

    return workerLog;
}
json JSONProcessor::createWorkerStatusStructure
(std::shared_ptr<WorkerStatus> workerStatus)
{
    std::string status = workerStatus->getStatusString();
    int progress = workerStatus->getProgress();
    int runningProgress = workerStatus->getRunningProgress();
    std::string started = workerStatus->getStarted();
    std::string finished = workerStatus->getFinished();
    std::vector<std::string> slotStatus = {};
    for (auto it = workerStatus->getSlotStatusMap()->begin();
     it != workerStatus->getSlotStatusMap()->end(); ++it)
    {
        auto &s = *it;
        slotStatus.push_back(workerStatus->determineStatus(s.second));
    }
    return {
        {"status", status},
        {"progress", progress},
        {"slots", slotStatus},
        {"runningProgress", runningProgress},
        {"started", started},
        {"finished", finished}};
}

void JSONProcessor::createDummyWorkerLogs(std::shared_ptr<DistributedJob> job)
{
    if (job->getFiles().size() == 0)
    {

        for (auto it = job->getObservableNodesWithNames()->begin();
         it != job->getObservableNodesWithNames()->end(); ++it)
        {

            auto &s = *it;

            json dummyWL = {};
            std::string node = std::to_string(s.first);
            dummyWL["job"] = {{"id", job->getId()}};
            dummyWL["node"] = node;
            std::string fileName = job->getId() + "_node_" + node + ".json";

            job->pushFile(fileName);
            std::vector<std::string> slotStatus = {};
            if (s.second == "collect2")
            {
                slotStatus.push_back(WORKER_STATUS_CREATED);
                slotStatus.push_back(WORKER_STATUS_CREATED);
            }
            else
            {
                int num = job->getNumOfSlotsPerWorker();
                for (int i = 0; i < num; i++)
                {
                    slotStatus.push_back(WORKER_STATUS_CREATED);
                }
            }
            for (auto it = job->getWorkers()->begin();
             it != job->getWorkers()->end(); ++it)
            {
                auto &w = *it;
                std::string wId = w.first;
                dummyWL[wId] = {
                    {"status", WORKER_STATUS_CREATED},
                    {"progress", 0},
                    {"slots", slotStatus},
                    {"runningProgress", 0},
                    {"started", ""},
                    {"finished", ""}};
            }

            std::string logLocation = DEFAULT_LOG_DIRECTORY + "/" + fileName;
            std::ofstream o(logLocation);
            o << std::setw(4) << dummyWL << std::endl;
        }
    }
}