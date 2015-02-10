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


1 QueryExecutor for Distributed-SECONDO


1.1 Includes

*/

#include <string>
#include <iostream>
#include <map>
#include <algorithm>

#include <unistd.h>
#include <stdlib.h>
#include <time.h>

#include "SecondoInterface.h"
#include "NestedList.h"
#include "NList.h"
#include "qelogger.h"
#include "CassandraAdapter.h"
#include "CassandraResult.h"
#include "heartbeat.h"
#include "workerqueue.h"
#include "qeutils.h"
#include "secondoworker.h"
#include "state.h"

/*
1.1 Defines

*/

// Enum for ownership of a token range
// Local Tokenrange   - We have to process the data in the range
// Foreign Tokenrange - An other worker process has to process the data
enum RANGE_MODE {LOCAL_TOKENRANGE, FOREIGN_TOKENRANGE};

#define CMDLINE_CASSANDRA_NODE          1<<0
#define CMDLINE_CASSANDRA_KEYSPACE      1<<1
#define CMDLINE_SECONDO_HOST            1<<2
#define CMDLINE_SECONDO_PORT            1<<3

/*
1.2 Usings

*/
using namespace std;
using namespace cassandra;

/*
1.3 Structs

*/
struct cmdline_args_t {
  string cassandraNodeIp;
  string cassandraKeyspace;
  string secondoHost;
  vector<string> secondoPorts;
};


class Queryexecutor {

public:
   
   Queryexecutor(vector<SecondoWorker*> *myWorker, 
         CassandraAdapter *myCassandra,
         cmdline_args_t *myCmdline_args, string myInstanceUuid, 
         QueryexecutorState *myQueryExecutorState) 
         : worker(myWorker), cassandra(myCassandra), 
           cmdline_args(myCmdline_args), instanceUuid(myInstanceUuid), 
           queryExecutorState(myQueryExecutorState) {
             
   }
   
   /*
   2.1 Update last executed command in cassandra system table

   */
   bool updateLastCommand(size_t lastCommandId) {
  
     // Build CQL query
     stringstream ss;
     ss << "UPDATE system_state set lastquery = ";
     ss << lastCommandId;
     ss << " WHERE ip = '";
     ss << cmdline_args->cassandraNodeIp;
     ss << "';";

     // Update last executed command
     bool result = cassandra -> executeCQLSync(
       ss.str(),
       CASS_CONSISTENCY_ONE 
     );
 
     if(! result) {
        LOG_ERROR("Unable to update last executed query "
          << "in system_state table" << endl
          << "CQL Statement: " << ss.str() << endl);
        return false;
     }

     return true;
   }
   

   /*
   2.2 Refresh our heartbeat data

   */
   bool updateHeartbeatData(map<string, time_t> &heartbeatData) {


       // Timestamp of the last heartbeat update
       // We update the heartbeat messages only 
       // if the last update is older then
       // HEARTBEAT_REFRESH_DATA
       static time_t lastUpdate = 0;
    
       time_t now = time(0);

       // Heartbeat data outdated, refresh
       if(lastUpdate + HEARTBEAT_REFRESH_DATA < now) {
     
         heartbeatData.clear();

         if (! cassandra -> getHeartbeatData(heartbeatData) ) {
             cerr << "[Error] Unable to heartbeat from system table" << endl;
             return false;
         }

         // Update last heartbeat refresh timestamp
         lastUpdate = time(0); 
       }

       return true;
   }

   /*
   2.2 Refresh our information about the cassandra ring

   */
   bool refreshRingInfo(vector<TokenRange> &allTokenRanges, 
                        vector<TokenRange> &processedIntervals, 
                        map<string, time_t> &heartbeatData, 
                        size_t queryId) {
  

       // Clear transient data
       allTokenRanges.clear();
       processedIntervals.clear();

       // Refresh data
       if (! cassandra -> getTokenRangesFromSystemtable(allTokenRanges) ) {
         cerr << "[Error] Unable to collect token ranges from system table" 
              << endl;
         return false;
       }
   
       if (! cassandra -> getProcessedTokenRangesForQuery(
                  processedIntervals, queryId) ) {
      
         cerr << "[Error] Unable to collect processed token ranges from "
              << "system table" << endl;
         return false;
       }
  
      if( ! updateHeartbeatData(heartbeatData)) {
         return false;
      }  
  
     return true;
   }

   /*
   2.2 Update UUID Entry in global state table

   */
   bool updateUuid() {  
      
     // Build CQL query
     stringstream ss;
     ss << "UPDATE system_state set node = '";
     ss << instanceUuid;
     ss << "' WHERE ip = '";
     ss << cmdline_args->cassandraNodeIp;
     ss << "';";
  
     bool result = cassandra -> executeCQLSync(
         ss.str(),
         CASS_CONSISTENCY_ONE
     );
  
     if(! result) {
       cout << "[Error] Unable to update heartbeat in system_state table" 
            << endl;
       cout << "CQL Statement: " << ss.str() << endl;
       return false;
     }
  
     return true;
   }

   /*
   2.2 Find the first local token range in the logical ring and 
       return its position

   */
   size_t findFirstLocalTokenrange(vector<TokenRange> &allTokenRanges, 
                                 vector<TokenRange> &localTokenRanges) {
         size_t offset = 0;
         for( ; offset < allTokenRanges.size(); ++offset) {
           TokenRange tryTokenRange = allTokenRanges[offset];
           if(binary_search(localTokenRanges.begin(), 
                            localTokenRanges.end(), tryTokenRange)) {
             break;
           }
         }
      
         return offset;
   }

   /*
   2.2 Print total and processed tokens and sleep some time
    
   */
   void printStatusMessage(vector<TokenRange> &allTokenRanges, 
                           vector<TokenRange> &processedIntervals,
                           bool wait) {
        
         cout << "[Info] RESULT: " << processedIntervals.size() << " of "
              << allTokenRanges.size() << " token ranges processed" << endl;
      
         if(wait) {
           cout << "[Info] Sleep 5 seconds and check the ring again" << endl;
           sleep(5);
         }
   }

   /*
   2.2 Execute a query for a tokenrange range, only if it's not 
       already processed 

   */
   bool executeQueryForTokenrangeIfNeeded(string &query, 
                                   size_t queryId, string &ip, 
                                   vector<TokenRange> &processedIntervals,
                                   TokenRange tokenrange) {
  
  
           // Its query already processed for tokenrange?
           if( binary_search(processedIntervals.begin(), 
                             processedIntervals.end(), tokenrange))  {
          
             LOG_DEBUG("Skipping already processed Range: " << tokenrange);
             return false;
           } else {
             WorkerQueue *tokenQueue = (worker->front()) -> getTokenQueue();
             tokenQueue->push(tokenrange);
             return true;
           }
   }

   /*
   2.2 Execute the given query for all local token ranges

   */
   void executeQueryForTokenranges(string &query, 
                                   size_t queryId, string &ip,
                                   vector<TokenRange> &allTokenRanges,
                                   vector<TokenRange> &localTokenRanges,
                                   vector<TokenRange> &processedIntervals) {
  
       // Generate token range queries for local tokenranges;
       for(vector<TokenRange>::iterator 
           iter = allTokenRanges.begin();
           iter != allTokenRanges.end(); ++iter) {
 
         TokenRange tokenrange = *iter;
    
         // It's a local token range
         if(tokenrange.isLocalTokenRange(ip)) {
           localTokenRanges.push_back(tokenrange);
           executeQueryForTokenrangeIfNeeded(query, queryId, ip, 
                processedIntervals, tokenrange);
         }
       }  
   }

   void waitForSecondoWorker() {
      // Wait for query execution
      for(vector<SecondoWorker*>::iterator it = worker->begin(); 
         it != worker->end(); ++it) {
         SecondoWorker *worker = *it;
         worker->waitForQueryCompletion();
      }
   }

   /*
   2.2 Handle a multitoken query (e.g. query ccollectrange('192.168.1.108', 
        'secondo', 'relation2', 'ONE',__TOKENRANGE__);

   */
   void handleTokenQuery(string &query, 
                         size_t queryId, string &ip) {

       // Collecting ring configuration
       vector<TokenRange> allTokenRanges;
       vector<TokenRange> localTokenRanges;
       vector<TokenRange> processedIntervals;
       map<string, time_t> heartbeatData;
    
       queryExecutorState->setQuery(query);
    
       if (! refreshRingInfo(allTokenRanges, processedIntervals, 
            heartbeatData, queryId) ) {
      
         cerr << "[Error] Unable to collect ring info" << endl;
         return;
       }
    
       // Part 1: Execute query for all local token ranges
       executeQueryForTokenranges(query, queryId, ip,
                                allTokenRanges, localTokenRanges, 
                                processedIntervals);

       // Part 2: Process other token ranges
       while( true ) {
         if (! refreshRingInfo(allTokenRanges, processedIntervals, 
               heartbeatData, queryId) ) {
        
           cerr << "[Error] Unable to collect ring info" << endl;
           return;
         }
      
         // Reverse the data of the logical ring, so we can interate from
         // MAX to MIN
         reverse(allTokenRanges.begin(), allTokenRanges.end());
           
         // All TokenRanges are processed
         if(processedIntervals.size() == allTokenRanges.size()) {
           printStatusMessage(allTokenRanges, processedIntervals, false);
        
           WorkerQueue *tokenQueue = (worker->front()) -> getTokenQueue();

           // Add Termination token
           for(size_t i = 0; i < worker->size(); ++i) {
              tokenQueue->push(TokenRange(0, 0, ip));
           }
    
           waitForSecondoWorker();

           return; 
         }

         // Find start offset
         size_t offset = findFirstLocalTokenrange(allTokenRanges, 
                                                  localTokenRanges);
      
         RANGE_MODE mode = LOCAL_TOKENRANGE;
      
         for(size_t position = 0; position < allTokenRanges.size(); 
             ++position) {
                
           size_t realPos = (position + offset) % allTokenRanges.size();
           TokenRange range = allTokenRanges[realPos];              
           LOG_DEBUG("Handling tokenrange: " << range);
        
           if(range.isLocalTokenRange(ip)) {
             LOG_DEBUG("It's a local token range");
             mode = LOCAL_TOKENRANGE;
           } else {
          
             // refresh heartbeat data
             if( ! updateHeartbeatData(heartbeatData)) {
                LOG_ERROR("Unable to refresh heartbeat data");
             }  
          
             time_t now = time(0) * 1000; // Convert to ms
          
             if(heartbeatData[range.getIp()] + HEARTBEAT_NODE_TIMEOUT > now) {
               LOG_DEBUG("Set to foreign: " << range);
               mode = FOREIGN_TOKENRANGE;
             } else {
               LOG_DEBUG("Treat range as local, because node is dead: " 
                 << range << " Hartbeat: " << heartbeatData[range.getIp()]);
             }
           }
        
           // Process range - it's a local token range
           if(mode == LOCAL_TOKENRANGE) {
             executeQueryForTokenrangeIfNeeded(
              query, queryId, ip, processedIntervals, range);
           }
         }
            
         printStatusMessage(allTokenRanges, processedIntervals, true);
       }
   } 

      
   void executeSecondoCommand(string command, size_t queryId, 
       bool wait = true) {
   
      // Execute query on all worker
      for(vector<SecondoWorker*>::iterator it = worker->begin(); 
        it != worker->end(); ++it) {
        
         SecondoWorker *worker = *it;
         worker->submitQuery(command, queryId);
      }
   
      if(wait) {
         waitForSecondoWorker();
      }
   }

   /*
   2.3 This is the main loop of the query executor. This method fetches
     queries from cassandra and forward them to secondo.

   */
   void mainLoop() {
 
     size_t lastCommandId = 0;
     
     updateLastCommand(lastCommandId);       
     updateUuid();
  
     while(true) {
        
           cout << "Waiting for commands...." << endl;
        
           vector<CassandraQuery> result;
           cassandra->getQueriesToExecute(result);
           size_t seenCommands = 0;
        
           while(!result.empty()) {
           
             CassandraQuery &query = result.back();
           
             size_t id = query.getQueryId();
             string command = query.getQuery();
          
             QEUtils::replacePlaceholder(
                command, "__NODEID__", instanceUuid);
             QEUtils::replacePlaceholder(
                command, "__CASSANDRAIP__", cmdline_args -> cassandraNodeIp);
             QEUtils::replacePlaceholder(
                command, "__KEYSPACE__", cmdline_args -> cassandraKeyspace);
          
             // Is this the next query to execute
             if(id == lastCommandId + 1) {

               // Update global status
               ++lastCommandId;
            
               // Simple query or token based query?
               if(QEUtils::containsPlaceholder(command, "__TOKENRANGE__")) {
                 executeSecondoCommand(command, lastCommandId, false);
                 handleTokenQuery(command, lastCommandId, 
                      cmdline_args -> cassandraNodeIp);
               } else {
                  executeSecondoCommand(command, lastCommandId);
               }
            
               updateLastCommand(lastCommandId);
               updateUuid();
             }
          
             result.pop_back();
             ++seenCommands;
           }
        
           // Command list is empty and we have processed 
           // commands in the past. => cqueryreset is executed,
           // clear secondo state and reset lastCommandId
           if(seenCommands < lastCommandId && lastCommandId > 0) {
             cout << "Doing query reset" << endl;
             sleep(5); // Wait for system tables to be recreated
             lastCommandId = 0;
             executeSecondoCommand(string("close database"), seenCommands);
             
             updateLastCommand(lastCommandId);
             updateUuid();
                 
             cout << "[Info] Reset complete, waiting for new queries" << endl;
           }
        
           sleep(5);
     }
   }
   
private:
   vector<SecondoWorker*> *worker;
   CassandraAdapter *cassandra;
   cmdline_args_t *cmdline_args;
   string instanceUuid;
   QueryexecutorState *queryExecutorState;
};


/*
2.1 Init the cassandra adapter, the 1st parameter
 is the initial contact point to the cassandra cluter.
 The 2nd parameter is the keyspace.

*/
CassandraAdapter* getCassandraAdapter(string &cassandraHostname, 
                                      string &cassandraKeyspace) {
  
  CassandraAdapter* cassandra = 
     new CassandraAdapter(cassandraHostname, cassandraKeyspace);
  
  cassandra -> connect(true);
  
  // Connection successfully?
  if(cassandra == NULL) {
    return NULL;
  }
  
  if(! cassandra->createMetatables() ) {
    return NULL;
  }
  
  return cassandra;
}

/*
2.4 start the heartbeat thread

*/
void* startHeartbeatThreadInternal(void *ptr) {
  HeartbeatUpdater* hu = (HeartbeatUpdater*) ptr;
  hu -> run();
  
  return NULL;
}

/*
2.5 start the heartbeat thread

*/
HeartbeatUpdater* startHeartbeatThread(string cassandraIp, 
                         string cassandraKeyspace,
                         pthread_t &targetThread) {
  
   HeartbeatUpdater* heartbeatUpdater 
     = new HeartbeatUpdater(cassandraIp, cassandraKeyspace);
     
   pthread_create(&targetThread, NULL, 
                  &startHeartbeatThreadInternal, heartbeatUpdater);
  
  return heartbeatUpdater;
}

/*
2.6 Print help and exiting

*/
void printHelpAndExit(char *progName) {
  cerr << "Usage: " << progName 
       << " -i <cassandra-ip> -k <keyspace> -s <secondo-ip> -p <secondo-port>" 
       << endl;
          
  cerr << endl;
  cerr << "-i <IP-Address> - The cassandra node to connect to" << endl;
  cerr << "-k <Keyspace>   - The keyspace to open" << endl;
  cerr << "-s <IP-Address> - The IP of the SECONDO Server" << endl;
  cerr << "-p <Port>       - The SECONDO Server port to connect to" << endl;
  exit(EXIT_FAILURE);
}

/*
2.7 parse commandline args

*/
void parseCommandline(int argc, char* argv[], 
                     cmdline_args_t &cmdline_args) {
  
  unsigned int flags = 0;
  int option = 0;
  char* pch;
  
  while ((option = getopt(argc, argv,"i:k:s:p:")) != -1) {
     switch (option) {
      case 'i':
           cmdline_args.cassandraNodeIp = string(optarg);
           flags |= CMDLINE_CASSANDRA_NODE;
           break;
      case 'k':
           cmdline_args.cassandraKeyspace = string(optarg);
           flags |= CMDLINE_CASSANDRA_KEYSPACE;
           break;
      case 's':
           cmdline_args.secondoHost = string(optarg);
           flags |= CMDLINE_SECONDO_HOST;
           break;
      case 'p':
           
           pch = strtok (optarg, ":");
           while (pch != NULL) {
              cmdline_args.secondoPorts.push_back(string(pch));
              pch = strtok (NULL, ":");
           }
        
           flags |= CMDLINE_SECONDO_PORT;
           break;
      default:
        printHelpAndExit(argv[0]);
     }
  }
  
  unsigned int requiredFalgs = CMDLINE_CASSANDRA_NODE | 
                               CMDLINE_CASSANDRA_KEYSPACE |
                               CMDLINE_SECONDO_HOST | 
                               CMDLINE_SECONDO_PORT;
                               
  if(requiredFalgs != flags) {
    printHelpAndExit(argv[0]);
  }
}

/*
2.7 Disconnect from cassandra

*/
void disconnectFromCassandra(CassandraAdapter* cassandra) {
   if(cassandra != NULL) {
     cassandra -> disconnect();
     delete cassandra;
     cassandra = NULL;
   }
}

/*
2.8 Start n SECONDO worker

*/
void startSecondoWorker(vector<SecondoWorker*> &worker, 
   QueryexecutorState &queryExecutorState, 
   cmdline_args_t &cmdline_args, WorkerQueue &tokenQueue, 
   CassandraAdapter* cassandra) {
   
   size_t workerId = 0;
      
   for(vector<string>::iterator it = cmdline_args.secondoPorts.begin(); 
        it != cmdline_args.secondoPorts.end(); it++) {
          
       SecondoWorker *secondoWorker = new SecondoWorker(
            cassandra, cmdline_args.secondoHost, *it, &tokenQueue, 
            workerId, &queryExecutorState);
      
       startSecondoWorkerThread(secondoWorker);
       worker.push_back(secondoWorker);
       
       workerId++;
   }
}

/*
2.9 Stop all SECONDO worker

*/
void stopSeconcoWorker(vector<SecondoWorker*> &worker) {
   for(vector<SecondoWorker*>::iterator it = worker.begin(); 
        it != worker.end(); it++) {
     
           // Stop and delete worker
           (*it)->stop();
           delete *it;
   }
   worker.clear();
}

/*
2.8 Main method

*/
int main(int argc, char* argv[]){
  
  WorkerQueue tokenQueue(2);
  cmdline_args_t cmdline_args;
  vector<SecondoWorker*> worker;
  QueryexecutorState queryExecutorState;
  Logger::open();

  parseCommandline(argc, argv, cmdline_args);
  
  CassandraAdapter* cassandra = 
     getCassandraAdapter(cmdline_args.cassandraNodeIp, 
                         cmdline_args.cassandraKeyspace);
  
  if(cassandra == NULL) { 
    cerr << "Could not connect to cassandra, exiting" << endl;
    exit(EXIT_FAILURE);
  }
  
  // Start SECONDO worker
  startSecondoWorker(worker, queryExecutorState, cmdline_args, 
     tokenQueue, cassandra);

  // Gernerate UUID
  string instanceUuid;
  QEUtils::createUUID(instanceUuid);
  
  // Main Programm
  pthread_t heartbeatThread;
  HeartbeatUpdater* heartbeatUpdater;
  
  heartbeatUpdater = startHeartbeatThread(cmdline_args.cassandraNodeIp, 
                       cmdline_args.cassandraKeyspace, heartbeatThread);


  Queryexecutor queryexecutor(&worker, cassandra, &cmdline_args, 
                instanceUuid, &queryExecutorState);
  queryexecutor.mainLoop();
  
  // Stop SECONDO worker
  stopSeconcoWorker(worker);
   
  // Stop heatbeat thread
  heartbeatUpdater->stop();
  pthread_join(heartbeatThread, NULL);
  delete heartbeatUpdater;
  heartbeatUpdater = NULL;
  
  disconnectFromCassandra(cassandra);
  Logger::close();
   
  return 0;
}
