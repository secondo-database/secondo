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

#include "ProgressObserver.h"
#include "TreeListener.h"

using namespace distributed2;

ProgressObserver::ProgressObserver(QueryProcessor *qpRef)
{
    qp = qpRef;
    auto treeListener = std::make_shared<TreeListener>(qpRef, this);
    qpRef->registerNewTreeListener(treeListener);
    jsonProcessor = new JSONProcessor();
    saveThreaded = false;
    saveToJSONThreaded();
}

ProgressObserver::~ProgressObserver() {}

void ProgressObserver::observableNodesReadyCallback
(std::unordered_map<int, Supplier> observableNodes,
std::string query,
std::string tree,
std::string nestedList,
std::unordered_map<int, std::string> observableNodesWithNames,
std::unordered_map<int, std::string> observableNodesWithArguments)
{
    initDistributedJob
    (observableNodes, query, tree, 
    nestedList, observableNodesWithNames, 
    observableNodesWithArguments);
}

void ProgressObserver::initDistributedJob
(std::unordered_map<int, Supplier> observableNodes,
std::string query,
std::string tree,
std::string nestedList,
std::unordered_map<int, std::string> observableNodesWithNames,
std::unordered_map<int, std::string> observableNodesWithArguments)
{
    std::string currentTime = getCurrentTime();
    job.reset();
    job = std::make_shared<DistributedJob>("automated_" + currentTime,
                                           currentTime,
                                           observableNodes,
                                           query,
                                           tree,
                                           nestedList,
std::make_shared<std::unordered_map<int, std::string>>
(observableNodesWithNames),
std::make_shared<std::unordered_map<int, std::string>>
(observableNodesWithArguments));
}

void ProgressObserver::mappingCallback(int supplierId, SDArray *sdarray)
{
    if (job->nextNode(supplierId, qp, sdarray))
    {
        initWorkerStatus("", sdarray, supplierId);
        saveThreaded = true;
    }
}

void ProgressObserver::mappingCallback(int supplierId, DArray *darray)
{
    if (job->nextNode(supplierId, qp, darray))
    {
        initWorkerStatus("", darray, supplierId);
        saveThreaded = true;
    }
}

void ProgressObserver::mappingCallback(int supplierId, DFArray *dfarray)
{
    if (job->nextNode(supplierId, qp, dfarray))
    {
        initWorkerStatus("", dfarray, supplierId);
        saveThreaded = true;
    }
}

void ProgressObserver::mappingCallback(int supplierId, DArrayBase *darrayBase)
{
    if (job->nextNode(supplierId, qp, darrayBase))
    {
        initWorkerStatus("", darrayBase, supplierId);
        saveThreaded = true;
    }
}

void ProgressObserver::mappingCallback(int supplierId, DFMatrix *dfmatrix)
{
    if (job->nextNode(supplierId, qp, dfmatrix))
    {
        initWorkerStatus("", dfmatrix, supplierId);
        saveThreaded = true;
    }
}

ProgressListener *ProgressObserver::getProgressListener
(std::string slotObjectName, DArrayElement elem)
{

    std::string workerId = buildWorkerId(elem);

    if(!evaluateProgressListenerRequest(workerId)) {
        cancelJob();
        return nullptr;
    }

    std::shared_ptr<WorkerStatus> ws = job->getWorkers()->at(workerId);
    job->setRunning();
    ws->getSlotStatusMap()->at(slotObjectName) = WorkerStatusEnum::RUNNING;
    ws->updateStatus(WorkerStatusEnum::RUNNING, getCurrentTime());
    return ws->getProgressListener();
}

ProgressListener *ProgressObserver::getProgressListener(DArrayElement elem)
{
    std::string workerId = buildWorkerId(elem);

    if(!evaluateProgressListenerRequest(workerId)) {
        cancelJob();
        return nullptr;
    }

    std::shared_ptr<WorkerStatus> ws = job->getWorkers()->at(workerId);
    job->setRunning();
    ws->updateStatus(WorkerStatusEnum::RUNNING, getCurrentTime());
    return ws->getProgressListener();
}

ProgressListener *ProgressObserver::getProgressListener
(std::string host, int port, int num, std::string part)
{

    std::string workerId = buildWorkerId(host, port, num);

    if(!evaluateProgressListenerRequest(workerId)) {
        cancelJob();
        return nullptr;
    }

    std::shared_ptr<WorkerStatus> ws = job->getWorkers()->at(workerId);
    if (!part.empty())
    {
        ws->getSlotStatusMap()->at(part) = WorkerStatusEnum::RUNNING;
    }
    job->setRunning();
    ws->updateStatus(WorkerStatusEnum::RUNNING, getCurrentTime());
    return ws->getProgressListener();
}

void ProgressObserver::commitWorkingOnSlotFinished
(std::string slotObjectName, DArrayElement elem)
{
    std::string workerId = buildWorkerId(elem);

    if(!evaluateProgressListenerRequest(workerId)) {
        return;
    }

    job->getWorkers()
    ->at(workerId)->getSlotStatusMap()
    ->at(slotObjectName) = 
    WorkerStatusEnum::FINISHED;
    job->getWorkers()->at(workerId)->updateNumOfFinishedSlots(1);
    if (job->allWorkersFinished())
    {
        saveThreaded = false;
        job->markCurrentWorkingAtNodeAsProcessed();
        jsonProcessor->createWorkerLog(job);
        saveThreaded = true;
    }
    if (job->allNodesProcessed())
    {
        finishJob();
    }
}

void ProgressObserver::commitWorkingOnElementFinished(DArrayElement elem)
{
    std::string workerId = buildWorkerId(elem);

    if(!evaluateProgressListenerRequest(workerId)) {
        return;
    }

    job->getWorkers()->at(workerId)
    ->setWorkerFinishedWithAllSlots(getCurrentTime());
    if (job->allWorkersFinished())
    {
        saveThreaded = false;
        job->markCurrentWorkingAtNodeAsProcessed();
        jsonProcessor->createWorkerLog(job);
        saveThreaded = true;
    }
    if (job->allNodesProcessed())
    {
        finishJob();
    }
}

void ProgressObserver::commitWorkingOnElementFinished
(std::string host, int port, int num, std::string part)
{
    std::string workerId = buildWorkerId(host, port, num);

    if(!evaluateProgressListenerRequest(workerId)) {
        return;
    }

    if (!part.empty())
    {
        job->getWorkers()->at(workerId)->getSlotStatusMap()->at(part)
         = WorkerStatusEnum::FINISHED;
    }
    job->getWorkers()->at(workerId)->updateNumOfFinishedSlots(1);
    if (job->allWorkersFinished())
    {
        saveThreaded = false;
        job->markCurrentWorkingAtNodeAsProcessed();
        jsonProcessor->createWorkerLog(job);
        saveThreaded = true;
    }
    if (job->allNodesProcessed())
    {
        finishJob();
    }
}

bool ProgressObserver::evaluateProgressListenerRequest(std::string workerId)
{
    if(job->getObservableNodesList().size() == 0 || !job->getWorkers()) {
        return false;
    }

    if (job->getWorkers()->count(workerId) == 0) {
        return false;
    }

    return true;
}

template <typename A>
void ProgressObserver::initWorkerStatus
(std::string jobName, A *array, int supplierId)
{
    std::shared_ptr
    <std::unordered_map
    <std::string, std::shared_ptr<WorkerStatus>>> workerStatus
     = job->getWorkers();
    workerStatus->clear();
    std::string jobId = jobName;
    size_t numOfWorkers = array->numOfWorkers();
    for (size_t i = 0; i < numOfWorkers; i++)
    {
        std::string workerId = buildWorkerId(array->getWorker(i));
        std::shared_ptr<WorkerStatus> ws
         = std::make_shared<WorkerStatus>(workerId, jobId);
        std::unordered_map<std::string, WorkerStatusEnum> *slotStatusMap = 0;
        if (array->getType() == distributed2::DFMATRIX)
        {
            DFMatrix *dfmatrix = reinterpret_cast<DFMatrix *>(array);
            slotStatusMap =
                initSlotStatusForWorkerStatusDFMatrix(dfmatrix, i);
        }
        else if (array->getType() == distributed2::DARRAY)
        {
            DArray *darray = reinterpret_cast<DArray *>(array);
            slotStatusMap =
                initSlotStatusForWorkerStatus(darray, array->getWorker(i));
        }
        else if (array->getType() == distributed2::DFARRAY)
        {
            DFArray *dfarray = reinterpret_cast<DFArray *>(array);
            slotStatusMap =
                initSlotStatusForWorkerStatus(dfarray, array->getWorker(i));
        }
        else if (array->getType() == distributed2::SDARRAY)
        {
            SDArray *sdarray = reinterpret_cast<SDArray *>(array);
            slotStatusMap =
                initSlotStatusForWorkerStatus(sdarray, array->getWorker(i));
        }
        else
        {
            DArrayBase *darraybase = reinterpret_cast<DArrayBase *>(array);
            slotStatusMap =
                initSlotStatusForWorkerStatus(darraybase, array->getWorker(i));
        }
        ws->setSlotStatusMap(slotStatusMap);
        ws->setNumOfSlots(slotStatusMap->size());
        ProgressListener *pl = new ProgressListener(ws);
        ws->setProgressListener(pl);
        workerStatus->insert(make_pair(workerId, ws));
        ws->setNodeId(supplierId);
        job->setNumOfSlotsPerWorker(slotStatusMap->size());
    }
    jsonProcessor->createDummyWorkerLogs(job);
}

template <typename A>
std::unordered_map
<std::string, WorkerStatusEnum> *ProgressObserver::initSlotStatusForWorkerStatus
(A *array, DArrayElement elem)
{
    std::unordered_map<std::string, WorkerStatusEnum> *slotStatusMap
     = new std::unordered_map<std::string, WorkerStatusEnum>();
    for (size_t i = 0; i < array->getSize(); i++)
    {
        if (elem == array->getWorkerForSlot(i))
        {
            std::string objectName = array->getObjectNameForSlot(i);
            slotStatusMap
            ->insert(make_pair(objectName, WorkerStatusEnum::CREATED));
        }
    }
    return slotStatusMap;
}

std::unordered_map<std::string, WorkerStatusEnum>
 *ProgressObserver::initSlotStatusForWorkerStatusDFMatrix(
    DFMatrix *dfmatrix, int nr)
{

    std::unordered_map<std::string, WorkerStatusEnum> *slotStatusMap
     = new std::unordered_map<std::string, WorkerStatusEnum>();
    std::string objectName = "part_";
    slotStatusMap->insert(
        make_pair(objectName + std::to_string(1), WorkerStatusEnum::CREATED));
    slotStatusMap->insert(
        make_pair(objectName + std::to_string(2), WorkerStatusEnum::CREATED));
    return slotStatusMap;
}

std::string ProgressObserver::buildWorkerId(DArrayElement elem)
{
    return elem.getHost() 
    + ":" + std::to_string(elem.getPort()) 
    + "_" + std::to_string(elem.getNum());
}

std::string ProgressObserver::buildWorkerId(std::string host, int port, int num)
{
    return host + ":" + std::to_string(port) + "_" + std::to_string(num);
}

void ProgressObserver::saveToJSONThreaded()
{
    boost::thread mythread([=] {
        while (true)
        {
            boost::this_thread::sleep_for(boost::chrono::milliseconds(1000));
            if (saveThreaded)
            {
                jsonProcessor->insertUpdateJournal(job);
                jsonProcessor->createWorkerLog(job);
            }
        }
    });
}

void ProgressObserver::finishJob()
{
    saveThreaded = false;
    job->finishJob(getCurrentTime());
    jsonProcessor->createWorkerLog(job);
    jsonProcessor->insertUpdateJournal(job);
}

void ProgressObserver::cancelJob()
{
    if (saveThreaded){
        saveThreaded = false;
        job->setCanceled();
        jsonProcessor->insertUpdateJournal(job);
    }
    if(job){
        job->clearObservableNodes();
    }

}

std::string ProgressObserver::getCurrentTime()
{
    auto currentTimeAuto = std::chrono::system_clock::now();
    time_t currentTimeTime_t 
    = std::chrono::system_clock::to_time_t(currentTimeAuto);
    return std::to_string(currentTimeTime_t);
}
