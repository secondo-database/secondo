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
#include <string>
#include <unordered_map>
#include <vector>
#include "AlgebraTypes.h"
#include "WorkerStatus.h"
#include "QueryProcessor.h"

std::string const JOB_STATUS_INIT = "created";
std::string const JOB_STATUS_RUNNING = "running";
std::string const JOB_STATUS_FINISHED = "finished";
std::string const JOB_STATUS_CANCELED = "canceled";
/*
1
Class DistributedJob

Represents the currently running distributed job. Coordinates the ~WorkerStatus~ objects.

*/
class DistributedJob {
std::string DEFAULT_JOURNAL_FILENAME = "DistributedJobsJournal.json";
public:

/*
Constructor

*/
DistributedJob(std::string _id, std::string _started,
std::unordered_map<int, Supplier> _observableNodes,
std::string _query, std::string _tree, std::string _nestedList,
std::shared_ptr<std::unordered_map<int, std::string>> _observableNodesWithNames,
std::shared_ptr<std::unordered_map<int, std::string>>
_observableNodesWithArguments);
/*
Destructor

*/
    ~DistributedJob();

/*
~getId~

Returns the ID of the job

*/
    std::string getId();

    /*
    ~getWorkers~

    Returns the corresponding worker status objects in a unordered map
     */
    std::shared_ptr
    <std::unordered_map
    <std::string, std::shared_ptr<WorkerStatus>>> getWorkers();

    /*
    ~setWorkers~

    Sets the corresponding worker status objects in a unordered map
     */
    void setWorkers
    (std::shared_ptr<std::unordered_map
    <std::string, std::shared_ptr<WorkerStatus>>> workers);

    /*
    ~getStarted~

    Returns the timestamp of the start
     */
    std::string getStarted();

    /*
    ~getStarted~

    Returns the timestamp of the finish (blank if job is not finished yet)
     */
    std::string getFinished();

    /*
    ~getStatus~

    Returns the status of the job
     */
    std::string getStatus();

    /*
    ~getFile~

    Returns the correspondig file name
     */
    std::string getFile();

    /*
    ~getNumOfWorkers~

    Returns the number of involved workers
     */
    int getNumOfWorkers();

    /*
    ~getJobNameForLog~

    Returns the correspondig name meant to be used in the log
     */
    std::string getJobNameForLog();

    /*
    ~finishJob~

    Sets the status to FINISHED with timestamp
     */
    void finishJob(std::string time);

    /*
    ~getObservableNodes~

    Returns the nodes to be observed in the job
     */
    std::unordered_map<int, Supplier> getObservableNodes();

    /*
    ~getObservableNodesList~

    Returns the nodes to be observed in the job
     */
    std::unordered_map<int, Supplier> getObservableNodesList();

    /*
    ~getReadyNodeIds~

    Returns IDs of finished nodes
     */
    std::vector<int> getReadyNodeIds();

    /*
    ~getQuery~

    Returns the entered query
     */
    std::string getQuery();

    /*
    ~getTree~

    Returns the tree of the job (JSON) as a string
     */
    std::string getTree();

    /*
    ~getNowWorkingOnNodeId~

    Returns the ID of the currently processed node
     */
    int getNowWorkingOnNodeId();

    /*
    ~populateObservableNodeIds~
    
    Populates observable node IDs
     */
    void populateObservableNodeIds(std::unordered_map<int, Supplier> _obsNodes);

    /*
    ~getObservableNodeIds~

    Returns IDs of observable nodes
     */
    std::vector<int> getObservableNodeIds();

    /*
    ~allWorkersFinished~

    Returns ~true~ if all nodes finished
     */
    bool allWorkersFinished();

    /*
    ~setRunning~

    Returns true at change, 
    false if the status was not changed (was already running)
     */
    bool setRunning();

    /*
    ~setNumOfSlotsPerWorker~

    Sets number of slots per worker
     */
    void setNumOfSlotsPerWorker(int num);

    /*
    ~getNumOfSlotsPerWorker~

    Returns number of slots per worker
     */
    int getNumOfSlotsPerWorker();

    /*
    ~getFiles~

    Returns an array of log file names per worker
     */
    std::vector<std::string> getFiles();

    /*
    ~pushFile~

    Adds a file name to the array of file names per worker
     */
    void pushFile(std::string file);

    /*
    ~getNestedList~
    
    Returns corresponding nested list
     */
    std::string getNestedList();

    /*
    ~getObservableNodesWithNames~
    
    Returns observalbe nodes with their names
     */
    std::shared_ptr<std::unordered_map<int, std::string>> 
    getObservableNodesWithNames();

    /*
    ~getObservableNodesWithArguments~
    
    Returns observalbe nodes with their arguments separated by colon
     */
    std::shared_ptr<std::unordered_map<int, std::string>> 
    getObservableNodesWithArguments();

    /*
    ~setCanceled~

    Sets the status to ~CANCELED~
     */
    void setCanceled();

    /*
    ~clearObservableNodes~
    
    Clears the list of observable nodes so no mapping can be accepted
    anymore until a new tree is created.
     */
    void clearObservableNodes();

    /*
    ~nextNode~

    Marks currently processed node as finished 
    and moves to the next node in the list. Returns true at success and false
    if there are no more nodes to move to
     */
    template <typename A>
    bool nextNode(int supplierId, QueryProcessor *qp, A *array)
    {
        markCurrentWorkingAtNodeAsProcessed();
        if (observableNodes.size() > 0 && observableNodes.count(supplierId) > 0)
        {
            prepareNextNode(supplierId, qp, array);
            return true;
        }
        return false;
    }

    /*
    ~markCurrentWorkingAtNodeAsProcessed~
    
    Marks currently processed node as finished
     */
    void markCurrentWorkingAtNodeAsProcessed()
    {
        if (nowWorkingOnNodeId != -1 
        && !(processedNodesList.count(nowWorkingOnNodeId) > 0))
        {
            processedNodesList
            .insert(std::make_pair(nowWorkingOnNodeId, nowWorkingOnNode));
        }
    }

    /*
    ~allNodesProcessed~

    Returns true if there are no more nodes to process and false else
     */
    bool allNodesProcessed();

private:
    std::string id;
    std::string started;
    std::string finished;
    std::string status;
    std::string file;
    std::shared_ptr<std::unordered_map
    <std::string, std::shared_ptr<WorkerStatus>>> workerStatus;
    std::unordered_map<int, Supplier> observableNodes;
    Supplier nowWorkingOnNode;
    std::string query;
    std::string nestedList;
    std::string tree;
    std::unordered_map<int, Supplier> observableNodesList;
    std::unordered_map<int, Supplier> processedNodesList;
    int nowWorkingOnNodeId = -1;
    std::vector<int> observableNodeIds{};
    std::vector<std::string> files{};
    std::shared_ptr<std::unordered_map<int, std::string>> 
    observableNodesWithNames;
    std::shared_ptr<std::unordered_map<int, std::string>> 
    observableNodesWithArguments;
    int numOfslotsPerWorker = -1;

    /*
    ~prepareNextNode~

    Prepares the next node to be processed
     */
    template <typename A>
    void prepareNextNode(int supplierId, QueryProcessor *qp, A *array)
    {
        nowWorkingOnNode = observableNodes.at(supplierId);
        nowWorkingOnNodeId = supplierId;
        removeSupplierFromList(supplierId);
    }

    /*
    ~removeSupplierFromList~

    Removes ~supplierId~ from the list of observable nodes
     */
    void removeSupplierFromList(int supplierId)
    {
        observableNodes.erase(supplierId);
    }
};