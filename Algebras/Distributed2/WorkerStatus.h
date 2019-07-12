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

#include <string>
#include <iostream>
#include "WorkerStatusEnum.cpp"
#include <unordered_map>

std::string const WORKER_STATUS_CREATED = "created";
std::string const WORKER_STATUS_RUNNING = "running";
std::string const WORKER_STATUS_FINISHED = "finished";

class ProgressListener;

/*
1 Class WorkerStatus

Represents status of a worker

*/
class WorkerStatus
{

public:
    /*
    1.1 Constructor

    */
    WorkerStatus();

    /*
    1.1.1 Constructor

    */
    WorkerStatus(std::string _id, std::string _jobId);

    /*
    1.2 Destructor

    */
    ~WorkerStatus();

    /*
    ~updateStatus~

    Updates status according ot ~_status~
    */
    void updateStatus(WorkerStatusEnum _status, std::string timestamp);

    /*
    ~updateProgress~

    Updates progress according ot ~_progress~
    */
    void updateProgress(int _progress);

    /*
    ~getId~

    Returns the ID of the worker
    */
    std::string getId();

    /*
    ~getStatusString~
    
    Returns the status of the worker as string
     */
    std::string getStatusString();

    /*
    ~getProgress~

    Returns progress of the worker in percent
     */
    int getProgress();

    /*
    ~getJobId~
    
    Returns the id of the owning job
     */
    std::string getJobId();

    /*
    ~setSlotStatusMap~
    
    Sets the map of slots ant their status
     */
    void setSlotStatusMap
    (std::unordered_map<std::string, WorkerStatusEnum> *_slotStatusMap);

    /*
    ~getSlotStatusMap~
    
    Returns the map of slots ant their status
     */
    std::unordered_map<std::string, WorkerStatusEnum> *getSlotStatusMap();

    /*
    ~getProgressListener~

    Returns the progress listener for the worker object
     */
    ProgressListener *getProgressListener();

    /*
    ~setProgressListener~

    Sets the progress listener for the worker object
     */
    void setProgressListener(ProgressListener *_pl);

    /*
    ~determineStatus~

    Returns the ~statusEnum~ as string
     */
    static std::string determineStatus(WorkerStatusEnum statusEnum);

    /*
    ~setNumOfSlots~

    Sets the number of slots for the worker
     */
    void setNumOfSlots(int num);

    /*
    ~setWorkerFinishedWithAllSlots~

    Sets the status of the worker to FINISHED with ~timestamp~
     */
    void setWorkerFinishedWithAllSlots(std::string timestamp);

    /*
    ~updateNumOfFinishedSlots~
    
    Updates the number of finished slots to ~num~
     */
    void updateNumOfFinishedSlots(int num);

    /*
    ~getNumOfFinishedSlots~

    Returns the number of finished slots
     */
    int getNumOfFinishedSlots();

    /*
    ~allSlotsFinished~

    Returns true if all slots of the worker finished, false else
     */
    bool allSlotsFinished();

    /*
    ~getRunningProgress~

    Returns the progress of currently processed slots. 
    Not available in some constellations. In this case returns 0
     */
    int getRunningProgress();

    /*
    ~getNumOfRunningSlots~
    
    Returns number of running slots of the worker
     */
    int getNumOfRunningSlots();

    /*
    ~getStarted~

    Returns the timestamp when the status was changed to RUNNING
     */
    std::string getStarted();

    /*
    ~getFinished~

    Returns the timestamp when the status was changed to FINISHED
     */
    std::string getFinished();

    /*
    ~setNodeId~
    
    Sets node id for the worker
     */
    void setNodeId(int id);

    /*
    ~getNodeId~
    
    Returns node id of the worker
     */
    int getNodeId();

private:
    std::string id;
    WorkerStatusEnum status;
    int progress;
    int runningProgress;
    std::string jobId;
    ProgressListener *pl;
    std::unordered_map<std::string, WorkerStatusEnum> *slotStatusMap;
    int numOfSlots;
    int numOfFinishedSlots;
    std::string started;
    std::string finished;
    int node;
};