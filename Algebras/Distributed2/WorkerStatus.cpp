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

#ifndef WORKERSTATUS_H
#define WORKERSTATUS_H

#include "WorkerStatus.h"
#include "ProgressListener.h"
#include <chrono>

WorkerStatus::WorkerStatus(std::string _id, std::string _jobId)
{
    id = _id;
    jobId = _jobId;
    status = WorkerStatusEnum::CREATED;
    progress = 0;
    runningProgress = 0;
    numOfSlots = 0;
    numOfFinishedSlots = 0;
    finished = "";
    started = "";
    node = -1;
}

WorkerStatus::WorkerStatus()
{
    //
}

WorkerStatus::~WorkerStatus()
{
    //
}

void WorkerStatus::updateStatus(WorkerStatusEnum _status, std::string timestamp)
{
    if (_status == WorkerStatusEnum::FINISHED)
    {
        finished = timestamp;
    }
    if (_status == WorkerStatusEnum::RUNNING && started == "")
    {
        started = timestamp;
    }
    status = _status;
}

void WorkerStatus::updateProgress(int _progress)
{
    if (_progress == 0)
    {
        return;
    }
    double offset = 0.0;
    if (numOfSlots > 0)
    {
        offset = (double)numOfFinishedSlots / (double)numOfSlots;
    }

    progress = (offset * 100) + _progress / numOfSlots;
    runningProgress = _progress;
}

std::string WorkerStatus::getId()
{
    return id;
}

std::string WorkerStatus::getStatusString()
{
    return determineStatus(status);
}

int WorkerStatus::getProgress()
{
    return progress;
}

std::string WorkerStatus::getJobId()
{
    return jobId;
}

ProgressListener *WorkerStatus::getProgressListener()
{
    return pl;
}

void WorkerStatus::setProgressListener(ProgressListener *_pl)
{
    pl = _pl;
}

void WorkerStatus::setSlotStatusMap
(std::unordered_map<std::string, WorkerStatusEnum> *_slotStatusMap)
{
    slotStatusMap = _slotStatusMap;
}

std::unordered_map
<std::string, WorkerStatusEnum> *WorkerStatus::getSlotStatusMap()
{
    return slotStatusMap;
}

void WorkerStatus::setNumOfSlots(int num)
{
    numOfSlots = num;
}

bool WorkerStatus::allSlotsFinished()
{
    return (numOfFinishedSlots == numOfSlots);
}

int WorkerStatus::getNumOfRunningSlots()
{
    int runningSlots = 0;
    std::unordered_map<std::string, WorkerStatusEnum>::iterator it
    = slotStatusMap->begin();
    while (it != slotStatusMap->end())
    {
        if (it->second == WorkerStatusEnum::RUNNING)
        {
            runningSlots++;
        }
        it++;
    }
    return runningSlots;
}

std::string WorkerStatus::getStarted()
{
    return started;
}

std::string WorkerStatus::getFinished()
{
    return finished;
}

void WorkerStatus::updateNumOfFinishedSlots(int num)
{
    numOfFinishedSlots = numOfFinishedSlots + num;
    if (numOfFinishedSlots > numOfSlots)
    {
        numOfFinishedSlots = numOfSlots;
    }
    if (numOfFinishedSlots > 0)
    {
        double offset = 0.0;
        if (numOfSlots > 0)
        {
            offset = (double)numOfFinishedSlots / (double)numOfSlots;
        }
        progress = offset * 100;
        runningProgress = 0;
    }
    if (numOfFinishedSlots == numOfSlots)
    {

        auto currentTimeAuto = std::chrono::system_clock::now();
        time_t currentTimeTime_t
        = std::chrono::system_clock::to_time_t(currentTimeAuto);
        updateStatus
        (WorkerStatusEnum::FINISHED, std::to_string(currentTimeTime_t));
    }
}

void WorkerStatus::setWorkerFinishedWithAllSlots(std::string timestamp)
{
    updateStatus(WorkerStatusEnum::FINISHED, timestamp);
    std::unordered_map<std::string, WorkerStatusEnum>::iterator it
     = slotStatusMap->begin();
    progress = 100;
    while (it != slotStatusMap->end())
    {
        it->second = WorkerStatusEnum::FINISHED;
        it++;
    }
    numOfFinishedSlots = numOfSlots;
}

int WorkerStatus::getNumOfFinishedSlots()
{
    return numOfFinishedSlots;
}

int WorkerStatus::getRunningProgress()
{
    return runningProgress;
}

void WorkerStatus::setNodeId(int id)
{
    if (node == -1)
    {
        node = id;
    }
}

int WorkerStatus::getNodeId()
{
    return node;
}

std::string WorkerStatus::determineStatus(WorkerStatusEnum statusEnum)
{
    switch (statusEnum)
    {
    case WorkerStatusEnum::RUNNING:
        return WORKER_STATUS_RUNNING;

    case WorkerStatusEnum::FINISHED:
        return WORKER_STATUS_FINISHED;

    default:
        return WORKER_STATUS_CREATED;
    }
}

#endif