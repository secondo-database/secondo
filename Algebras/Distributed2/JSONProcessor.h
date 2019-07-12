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

#include "Tools/Utilities/json.hpp"
#include "Profiles.h"
#include "DistributedJob.h"
#include <fstream>

using json = nlohmann::json;

std::string const DEFAULT_LOG_DIRECTORY = "distributed_logs";
std::string const DEFAULT_JOURNAL_FILENAME = "DistributedJobsJournal.json";

/*
1 Class JSONProcessor

Writes data to the log files as JSON objects

*/
class JSONProcessor
{

public:
    /*
    1.1 Constructor

    */
    JSONProcessor() {}

    /*
    1.2 Destructor

    */
    ~JSONProcessor();

    /*
    ~insertUpdateJournal~
    
    Updates or creates a journal file from ~job~
     */
    void insertUpdateJournal(std::shared_ptr<DistributedJob> job);

    /*
    ~createWorkerLog~
    
    Creates a worker log file from ~job~
     */
    void createWorkerLog(std::shared_ptr<DistributedJob> job);

    /*
    ~createNodeOperatorMapping~
    
    Creates a JSON mapping for nodes and their operatiors
     */
    json createNodeOperatorMapping(std::shared_ptr<DistributedJob> job);

    /*
    ~createNodeArgumentsMapping~
    
    Creates a JSON mapping for nodes and their arguments
     */
    json createNodeArgumentsMapping(std::shared_ptr<DistributedJob> job);

    /*
    ~createJournalJobStructure~

    Creates JSON structure for ~job~
    */
    json createJournalJobStructure(std::shared_ptr<DistributedJob> job);

    /*
    ~createWorkerLogStructure~

    Creates JSON structure for workers of the ~job~
    */
    json createWorkerLogStructure(std::shared_ptr<DistributedJob> job);

    /*
    ~createWorkerStatusStructure~

    Creates JSON structure for ~workerStatus~
     */
json createWorkerStatusStructure(std::shared_ptr<WorkerStatus> workerStatus);

    /*
    ~createDummyWorkerLogs~

    Creates a worker log file for each node with initial data
     */
    void createDummyWorkerLogs(std::shared_ptr<DistributedJob> job);

    /*
    ~getLogLocation~
    
    Returns location of the journal file
     */
    std::string getLogLocation()
    {
        return DEFAULT_LOG_DIRECTORY + "/" + DEFAULT_JOURNAL_FILENAME;
    }
};
