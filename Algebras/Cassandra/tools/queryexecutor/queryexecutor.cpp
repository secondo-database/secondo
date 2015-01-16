/*
----
This file is part of SECONDO.

Copyright (C) 2007,
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
#include "CassandraAdapter.h"
#include "CassandraResult.h"
#include "heartbeat.h"
#include "workerqueue.h"

/*
1.1 Defines

*/

// Enum for ownership of a token range
// Local Tokenrange   - We have to process the data in the range
// Foreign Tokenrange - An other worker process has to process the data
enum RANGE_MODE {LOCAL_TOKENRANGE, FOREIGN_TOKENRANGE };

// Activate debug messages
#define __DEBUG__

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
  string secondoPort;
};


/*
2.0 Init the secondo c++ api

*/

SecondoInterface* initSecondoInterface(string secondoHost, 
                  string secondoPort) {

   // create an interface to the secondo server
   // the paramneter must be true to communicate as client
   SecondoInterface* si = new SecondoInterface(true);

   // define the name of the configuration file
   string config = "Config.ini";

   // read in runtime flags from the config file
   si->InitRTFlags(config);

  // SECONDO Connection data
  string user = "";
  string passwd = "";
  
  bool multiUser = true;
  string errMsg;          // return parameter
  
  // try to connect
  if(!si->Initialize(user, passwd, secondoHost, secondoPort, 
                    config, errMsg, multiUser)){

     // connection failed, handle error
     cerr << "Cannot initialize secondo system" << endl;
     cerr << "Error message = " << errMsg << endl;

     return NULL;
  }
  
  return si;
}


/*
2.1 Update last executed command in cassandra system table

*/
bool updateLastCommand(CassandraAdapter* cassandra, 
                       size_t lastCommandId, string &ip) {
  
  // Build CQL query
  stringstream ss;
  ss << "UPDATE system_state set lastquery = ";
  ss << lastCommandId;
  ss << " WHERE ip = '";
  ss << ip;
  ss << "';";
  
  // Update last executed command
  bool result = cassandra -> executeCQLSync(
    ss.str(),
    CASS_CONSISTENCY_ONE 
  );
 
  if(! result) {
     cout << "Unable to update last executed query ";
     cout << "in system_state table" << endl;
     cout << "CQL Statement: " << ss.str() << endl;
     return false;
  }

  return true;
}

/*
2.1 Update global query status

*/
bool updateLastProcessedToken(CassandraAdapter* cassandra, 
                       size_t lastCommandId, string &ip, 
                       TokenRange tokenrange, string queryid) {
  
  // Build CQL query
  stringstream ss;
  ss << "INSERT INTO system_progress ";
  ss << "(queryid, ip, begintoken, endtoken, queryuuid) ";
  ss << "values(";
  ss << "" << lastCommandId << ",",
  ss << "'" << ip << "',";
  ss << "'" << tokenrange.getStart() << "',",
  ss << "'" << tokenrange.getEnd() << "',",
  ss << "'" << queryid << "'",
  ss << ");";
  
  // Update last executed command
  bool result = cassandra -> executeCQLSync(
    ss.str(),
    CASS_CONSISTENCY_ONE 
  );
 
  if(! result) {
     cout << "Unable to update last executed query in ";
     cout << "system_progress table" << endl;
     cout << "CQL Statement: " << ss.str() << endl;
     return false;
  }

  return true;
}

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
2.2 Replace placeholder like __NODEID__ in a given string

*/
void replacePlaceholder(string &query, string placeholder, string value) {
  size_t startPos = 0;
    
  while((startPos = query.find(placeholder, startPos)) != std::string::npos) {
         query.replace(startPos, placeholder.length(), value);
         startPos += value.length();
  }
}

/*
2.2 Does the given string contains a placeholder?

*/
bool containsPlaceholder(string searchString, string placeholder) {
  return searchString.find(placeholder) != std::string::npos;
}

/*
2.2 Execute a command in SECONDO

*/
void executeSecondoCommand(SecondoInterface* si, 
                           NestedList* nl, string command) {
  
        cout << "Executing command " << command << endl;
        
        ListExpr res = nl->TheEmptyList(); // will contain the result
        SecErrInfo err;                 // will contain error information
        
        si->Secondo(command, res, err); 

        // check whether command was successful
        if(err.code!=0){ 
          cout << "Error during command. Error code :" << err.code << endl;
          cout << "Error message = " << err.msg << endl;
        } else {
          // command was successful
          cout << "Result is:" << nl->ToString(res) << endl << endl;
        }
}

/*
2.2 Crete a new UUID 

*/
void createUUID(string &uuid) {
   char buffer[128];

   const char *filename = "/proc/sys/kernel/random/uuid";
   FILE *file = fopen(filename, "r");

   // Does the proc file exists?
   if( access(filename, R_OK ) == -1 ) {
       cerr << "Unable to get UUID from kernel" << endl;
       exit(-1);
   }
   
   if (file) {
     while (fscanf(file, "%s", buffer)!=EOF) {
        uuid.append(buffer);
     }
   }  

   fclose(file);
}

/*
2.2 Execute a token query for a given tokenrange

*/
void executeTokenQuery(CassandraAdapter* cassandra, string &query, 
                      size_t queryId, string &ip, 
                      TokenRange tokenrange,
                      SecondoInterface* si, NestedList* nl) {
  
    stringstream ss;
    ss << "'" << tokenrange.getStart() << "'";
    ss << ", ";
    ss << "'" << tokenrange.getEnd() << "'";
  
    // Copy query string, so we can replace the
    // placeholder multiple times
    string ourQuery = string(query);
    
    // Replace token range placeholder
    replacePlaceholder(ourQuery, "__TOKENRANGE__", ss.str());
    
    // Replace Query UUID placeholder
    string myQueryUuid;
    createUUID(myQueryUuid);
    replacePlaceholder(ourQuery, "__QUERYUUID__", myQueryUuid);
    
    executeSecondoCommand(si, nl, ourQuery);
    updateLastProcessedToken(cassandra, queryId, ip, tokenrange, myQueryUuid);
}

/*
2.2 Refresh our heartbeat data

*/
bool updateHeartbeatData(CassandraAdapter* cassandra, 
                         map<string, time_t> &heartbeatData) {


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
bool refreshRingInfo(CassandraAdapter* cassandra, 
                     vector<TokenRange> &allTokenRanges, 
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
  
   if( ! updateHeartbeatData(cassandra, heartbeatData)) {
      return false;
   }  
  
  return true;
}

/*
2.2 Update UUID Entry in global state table

*/
bool updateUuid(CassandraAdapter* cassandra, string uuid, 
                string cassandraIp) {  
  // Build CQL query
  stringstream ss;
  ss << "UPDATE system_state set node = '";
  ss << uuid;
  ss << "' WHERE ip = '";
  ss << cassandraIp;
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
bool executeQueryForTokenrangeIfNeeded(CassandraAdapter* cassandra, 
                                string &query, 
                                size_t queryId, string &ip, 
                                SecondoInterface* si, NestedList* nl,
                                vector<TokenRange> &processedIntervals,
                                TokenRange &tokenrange) {
  
  
        // Its query already processed for tokenrange?
        if( binary_search(processedIntervals.begin(), 
                          processedIntervals.end(), tokenrange))  {
          
#ifdef __DEBUG__
          cout << "[Debug] Skipping already processed TokenRange: " 
               << tokenrange;
#endif
               
          return false;
        } else {
          executeTokenQuery(cassandra, query, queryId, ip, tokenrange, si, nl);
          return true;
        }
}

/*
2.2 Execute the given query for all local token ranges

*/
void executeQueryForTokenranges(CassandraAdapter* cassandra, string &query, 
                                size_t queryId, string &ip, 
                                SecondoInterface* si, NestedList* nl,
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
        executeQueryForTokenrangeIfNeeded(cassandra, query, queryId, ip, si, 
                                          nl, processedIntervals, tokenrange);
      }
    }  
}


/*
2.2 Handle a multitoken query (e.g. query ccollectrange('192.168.1.108', 
     'secondo', 'relation2', 'ONE',__TOKENRANGE__);

*/
void handleTokenQuery(CassandraAdapter* cassandra, string &query, 
                      size_t queryId, string &ip, 
                      SecondoInterface* si, NestedList* nl) {

    // Collecting ring configuration
    vector<TokenRange> allTokenRanges;
    vector<TokenRange> localTokenRanges;
    vector<TokenRange> processedIntervals;
    map<string, time_t> heartbeatData;
    
    if (! refreshRingInfo(cassandra, 
           allTokenRanges, processedIntervals, heartbeatData, queryId) ) {
      
      cerr << "[Error] Unable to collect ring info" << endl;
      return;
    }
    
    // Part 1: Execute query for all local token ranges
    executeQueryForTokenranges(cassandra, query, queryId, ip, si, nl, 
                             allTokenRanges, localTokenRanges, 
                             processedIntervals);

    // Part 2: Process other token ranges
    while( true ) {
      if (! refreshRingInfo(cassandra, 
            allTokenRanges, processedIntervals, heartbeatData, queryId) ) {
        
        cerr << "[Error] Unable to collect ring info" << endl;
        return;
      }
      
      // Reverse the data of the logical ring, so we can interate from
      // MAX to MIN
      reverse(allTokenRanges.begin(), allTokenRanges.end());
           
      // All TokenRanges are processed
      if(processedIntervals.size() == allTokenRanges.size()) {
        printStatusMessage(allTokenRanges, processedIntervals, false);
        return; 
      }

      // Find start offset
      size_t offset = findFirstLocalTokenrange(allTokenRanges, 
                                               localTokenRanges);
      
      RANGE_MODE mode = LOCAL_TOKENRANGE;
      
      for(size_t position = 0; position < allTokenRanges.size(); ++position) {
        size_t realPos = (position + offset) % allTokenRanges.size();
        TokenRange range = allTokenRanges[realPos];

#ifdef __DEBUG__                 
        cout << "[Debug] Handling tokenrange: " << range;
#endif
        
        if(range.isLocalTokenRange(ip)) {

#ifdef __DEBUG__         
          cout << "[Debug] it's a local token range" << endl;
#endif
          
          mode = LOCAL_TOKENRANGE;
        } else {
          
          // refresh heartbeat data
          if( ! updateHeartbeatData(cassandra, heartbeatData)) {
             cout << "[Error] Unable to refresh heartbeat data" << endl;
          }  
          
          time_t now = time(0) * 1000; // Convert to ms
          
          if(heartbeatData[range.getIp()] + HEARTBEAT_NODE_TIMEOUT > now) {
#ifdef __DEBUG__             
            cout << "[Debug] Set to foreign: " << range;
#endif            
            mode = FOREIGN_TOKENRANGE;
          } else {
#ifdef __DEBUG__             
            cout << "[Debug] Treat range as local, because node is dead: " 
                 << range << " last update " << heartbeatData[range.getIp()] 
                 << endl;
#endif
                 
          }
        }
        
        // Process range - it's a local token range
        if(mode == LOCAL_TOKENRANGE) {
          executeQueryForTokenrangeIfNeeded(cassandra, query, queryId, ip, 
                                       si, nl, processedIntervals, range);
        }
      }
      
      printStatusMessage(allTokenRanges, processedIntervals, true);
    }
} 
      
/*
2.3 This is the main loop of the query executor. This method fetches
  queries from cassandra and forward them to secondo.

*/
void mainLoop(SecondoInterface* si, 
              CassandraAdapter* cassandra, string cassandraIp,
              string cassandraKeyspace, string uuid) {
  
  NestedList* nl = si->GetNestedList();
  NList::setNLRef(nl);
  
  size_t lastCommandId = 0;
  updateLastCommand(cassandra, lastCommandId, cassandraIp);
  updateUuid(cassandra, uuid, cassandraIp);
  
  while(true) {
        
        cout << "Waiting for commands" << endl;
        
        CassandraResult* result = cassandra->getQueriesToExecute();
        size_t seenCommands = 0;
        
        while(result != NULL && result -> hasNext()) {
          size_t id = result->getIntValue(0);
          
          string command;
          result->getStringValue(command, 1);
          replacePlaceholder(command, "__NODEID__", uuid);
          replacePlaceholder(command, "__CASSANDRAIP__", cassandraIp);
          replacePlaceholder(command, "__KEYSPACE__", cassandraKeyspace);
          
          // Is this the next query to execute
          if(id == lastCommandId + 1) {

            // Update global status
            ++lastCommandId;
            
            // Simple query or token based query?
            if(containsPlaceholder(command, "__TOKENRANGE__")) {
              handleTokenQuery(cassandra, command, lastCommandId, 
                               cassandraIp, si, nl);
            } else {
              executeSecondoCommand(si, nl, command);
            }
            
            updateLastCommand(cassandra, lastCommandId, cassandraIp);
            updateUuid(cassandra, uuid, cassandraIp);
          }
          
          ++seenCommands;
        }
        
        if(result != NULL)  {
           delete result;
           result = NULL;
        }
        
        // Command list is empty and we have processed 
        // commands in the past. => cqueryreset is executed,
        // clear secondo state and reset lastCommandId
        if(seenCommands < lastCommandId && lastCommandId > 0) {
          sleep(5); // Wait for system tables to be recreated
          lastCommandId = 0;
          updateLastCommand(cassandra, lastCommandId, cassandraIp);
          updateUuid(cassandra, uuid, cassandraIp);
          executeSecondoCommand(si, nl, "close database");
          cout << "[Info] Reset complete, waiting for new queries" << endl;
        }
        
        sleep(5);
  }
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
           cmdline_args.secondoPort = string(optarg);
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
2.8 Main method

*/
int main(int argc, char* argv[]){
  
  cmdline_args_t cmdline_args;
  parseCommandline(argc, argv, cmdline_args);

  SecondoInterface* si = initSecondoInterface(cmdline_args.secondoHost, 
                                              cmdline_args.secondoPort);

  if(si == NULL) { 
    return -1;
  }
  
  cout << "SecondoInterface successfully initialized" << endl;
  
  CassandraAdapter* cassandra = 
     getCassandraAdapter(cmdline_args.cassandraNodeIp, 
                         cmdline_args.cassandraKeyspace);
  
  if(cassandra == NULL) { 
    return -1;
  }
  
  cout << "Connection to cassandra successfull" << endl;

  // Gernerate UUID
  string myUuid;
  createUUID(myUuid);
  cout << "Our id is: " << myUuid << endl;
  
  // Main Programm
  pthread_t heartbeatThread;
  HeartbeatUpdater* heartbeatUpdater;
  
  heartbeatUpdater = startHeartbeatThread(cmdline_args.cassandraNodeIp, 
                       cmdline_args.cassandraKeyspace, heartbeatThread);

  mainLoop(si, cassandra, cmdline_args.cassandraNodeIp, 
           cmdline_args.cassandraKeyspace, myUuid);
  
  // Stop heatbeat thread
  heartbeatUpdater->stop();
  pthread_join(heartbeatThread, NULL);
  
  // Disconnect from cassandra
  if(cassandra) {
    cassandra -> disconnect();
    delete cassandra;
    cassandra = NULL;
  }
  
  // Shutdown the SECONDO interface
  if(si) {
    si->Terminate();
    delete si;
    si = NULL;
  }
 
  return 0;
}

