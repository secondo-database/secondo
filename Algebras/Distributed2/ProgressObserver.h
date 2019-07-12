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
#include "DArray.h"
#include <iostream>
#include <chrono>
#include <ctime>
#include "JSONProcessor.h"
#include "ProgressListener.h"
#include <unordered_map>
#include <utility>

#include <boost/bind.hpp>
#include <boost/ref.hpp>


/*
1 Class ProgressObserver

This class coordinates the observing of progress

*/
class ProgressObserver
{
public:
    /*
    1.1 Constructor

    */
    ProgressObserver(QueryProcessor *qpRef);

    /*
    1.2 Destructor

    */
    ~ProgressObserver();

    /*
    ~observableNodesReadyCallback~

    Callback to be called after all obervable nodes
    are collected
     */
    void observableNodesReadyCallback(
        std::unordered_map<int, Supplier> observableNodes,
        std::string query,
        std::string tree,
        std::string nestedList,
        std::unordered_map<int, std::string> 
        observableNodesWithNames,
        std::unordered_map<int, std::string>
        observableNodesWithArguments);

    /*
    ~operatorValueMappingCalled~

    Callback to be called from value mapping. 
    Indicates for which node id it was called.
     */
    void operatorValueMappingCalled(int nodeId);

    /*
    ~getProgressListener~

    Returns a progress listener object
    
     */
    ProgressListener *getProgressListener
    (std::string slotObjectName, distributed2::DArrayElement elem);

    /*
    ~getProgressListener~

    Returns a progress listener object
    
     */
    ProgressListener *getProgressListener(distributed2::DArrayElement elem);

    /*
    ~getProgressListener~

    Returns a progress listener object
    
     */
    ProgressListener *getProgressListener
    (std::string host, int port, int num, std::string part);

    /*
     ~commitWorkingOnSlotFinished~

    Callback to becalled when working on a particular slot was finished
    
     */
    void commitWorkingOnSlotFinished
    (std::string slotObjectName, distributed2::DArrayElement elem);

    /*
     ~commitWorkingOnSlotFinished~
     
    Callback to becalled when working on a particular slot was finished
    
     */
    void commitWorkingOnElementFinished(distributed2::DArrayElement elem);

    /*
     ~commitWorkingOnSlotFinished~
     
    Callback to becalled when working on a particular slot was finished
    
     */
    void commitWorkingOnElementFinished
    (std::string host, int port, int num, std::string part);

    /*
    ~buildWorkerId~

    Returns a unique worker id to identify a worker
    
     */
    std::string buildWorkerId(distributed2::DArrayElement elem);

    /*
    ~buildWorkerId~
    
    Returns a unique worker id to identify a worker
    
     */
    std::string buildWorkerId(std::string host, int port, int num);

    /*
    ~mappingCallback~
    
    Callback to be called after the mapping 
    for a supplier of type ~*sdarray~ was done
     */
    void mappingCallback(int supplierId, distributed2::SDArray *sdarray);

    /*
    ~mappingCallback~
    
    Callback to be called after the mapping 
    for a supplier of type ~*darray~ was done
     */
    void mappingCallback(int supplierId, distributed2::DArray *darray);

    /*
    ~mappingCallback~
    
    Callback to be called after the mapping 
    for a supplier of type ~*dfarray~ was done
     */
    void mappingCallback(int supplierId, distributed2::DFArray *dfarray);

    /*
    ~mappingCallback~
    
    Callback to be called after the mapping 
    for a supplier of type ~*darrayBase~ was done
     */
    void mappingCallback(int supplierId, distributed2::DArrayBase *darrayBase);

    /*
    ~mappingCallback~
    
    Callback to be called after the mapping 
    for a supplier of type ~*dfmatrix~ was done
     */
    void mappingCallback(int supplierId, distributed2::DFMatrix *dfmatrix);

    /*
    ~getCurrentTime~
    
    Returns current time as string
     */
    std::string getCurrentTime();

    /*
    ~initSlotStatusForWorkerStatus~

    Returns an undordered map of workers and their status
     */
    template <typename A>
    std::unordered_map<std::string, WorkerStatusEnum>
    *initSlotStatusForWorkerStatus(A *array, distributed2::DArrayElement elem);

    /*
    ~initSlotStatusForWorkerStatusDFMatrix~

    Returns an undordered map of workers and their status for ~dfmatrix~
     */
    std::unordered_map<std::string, WorkerStatusEnum> 
    *initSlotStatusForWorkerStatusDFMatrix(distributed2::DFMatrix *dfmatrix,
                                           int nr);

    /*
    ~initWorkerStatus~
    
    Initiates the ~workerStatus~ attribute for ~job~
     */
    template <typename A>
    void initWorkerStatus(std::string jobName, A *array, int supplierId);

    /*
    ~initDistributedJob~

    Inits the ~DistributedJob~ object
    */
    void initDistributedJob(std::unordered_map<int, Supplier> observableNodes,
                            std::string query,
                            std::string tree,
                            std::string nestedList,
            std::unordered_map<int, std::string> observableNodesWithNames,
            std::unordered_map<int, std::string> observableNodesWithArguments);

    /*
    ~updateJobStatus~

    Updates the status of the job
    
     */
    void updateJobStatus();

    /*
    ~finishJob~
    
    Called when no more nodes to work available
     */
    void finishJob();

    /*
    ~cancelJob~
    
    Called when a fatal error occurs
     */
    void cancelJob();

    /*
    ~evaluateProgressListenerRequest~
    
    Evaluates the request for a progress listener for a worker id.
    Returns true if there is a ~WorkerStatus~ object for
    the request and false else.
     */
    bool evaluateProgressListenerRequest(std::string workerId);

    /*
    ~saveToJSONThreaded~

    Saves the status of the job and his workers 
    to corresponding JSON files in a separate thread
     */
    void saveToJSONThreaded();

    JSONProcessor *jsonProcessor;

private:
    QueryProcessor *qp;
    std::shared_ptr<DistributedJob> job;
    bool saveThreaded = false;
};
