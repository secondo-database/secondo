
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

#include "../../CassandraAdapter.h"
#include "SecondoInterface.h"
#include "NestedList.h"
#include "NList.h"
#include "CassandraAdapter.h"
#include "CassandraResult.h"

#include <boost/uuid/uuid.hpp>      
#include <boost/uuid/uuid_generators.hpp> 
#include <boost/uuid/uuid_io.hpp>  


/*
1.1 Defines

*/

// Local Tokenrange - We have to process the data in the range
// Foreign Tokenrange - An other worker process has to process the data
enum RANGE_MODE {LOCAL_TOKENRANGE, FOREIGN_TOKENRANGE };

// Timeout in ms for receiving heartbeat messages from other nodes
#define NODE_TIMEOUT 30000

/*
1.2 Using

*/
using namespace std;
using namespace cassandra;


/*
2.0 Helper class for sending our own hearbeat
messages to cassandra. This class will be executed
in its own thread

*/
class HeartbeatUpdater {
 
public:
  
  HeartbeatUpdater(string myCassandraIp, string myCassandraKeyspace) 
     : cassandraIp(myCassandraIp), 
     cassandraKeyspace(myCassandraKeyspace),
     cassandra(NULL), active(true) {
        
       // Connect to cassandra
     cassandra = new CassandraAdapter
        (cassandraIp, cassandraKeyspace);
      
     cout << "[Heartbeat] Connecting to " << cassandraIp;
     cout << " / " << cassandraKeyspace << endl;
  
     if(cassandra != NULL) {
       cassandra -> connect(false);
     } else {
       cerr << "[Heartbeat] Unable to connect to cassandra, ";
       cerr << "exiting thread" << endl;
       active = false;
     }
  }
  
  virtual ~HeartbeatUpdater() {
    cout << "[Heartbeat] Shutdown because destructor called" << endl;
    stop();
  }
  
/*
2.1 Update Heartbeat timestamp

*/
  bool updateHeartbeat() {  
    // Build CQL query
    stringstream ss;
    ss << "UPDATE system_state set heartbeat = unixTimestampOf(now()) ";
    ss << "WHERE ip = '";
    ss << cassandraIp;
    ss << "';";
    
    return executeQuery(ss.str());
  } 

/*
2.1 Execute a query

*/
  bool executeQuery(string query) {
    // Update last executed command
    bool result = cassandra -> executeCQLSync(
      query,
      cql::CQL_CONSISTENCY_ONE
    );
  
    if(! result) {
      cout << "Unable to update heartbeat in system_state table" << endl;
      cout << "CQL Statement: " << query << endl;
      return false;
    }

    return true;
  }

/*
2.1 Main loop

*/
  void run() {
    while(active) {
      updateHeartbeat();
      sleep(5);
    }
  }

/*
2.1 Stop the main loop

*/
  void stop() {
    active = false;
    
    // Disconnect from cassandra
    if(cassandra != NULL) {
      cassandra -> disconnect();
      delete cassandra;
      cassandra = NULL;
    }
    
  }
  
private:
  string cassandraIp;
  string cassandraKeyspace;
  CassandraAdapter* cassandra;
  bool active;
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
    cql::CQL_CONSISTENCY_ONE
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
                       TokenRange tokenrange) {
  
  // Build CQL query
  stringstream ss;
  ss << "INSERT INTO system_progress(queryid, ip, begintoken, endtoken) ";
  ss << "values(";
  ss << "" << lastCommandId << ",",
  ss << "'" << ip << "',";
  ss << "'" << tokenrange.getStart() << "',",
  ss << "'" << tokenrange.getEnd() << "'",
  ss << ");";
  
  // Update last executed command
  bool result = cassandra -> executeCQLSync(
    ss.str(),
    cql::CQL_CONSISTENCY_ONE
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
2.2 Replace placeholder like __NODEID__ in Queries

*/
void replacePlaceholder(string &query, string placeholder, string value) {
  size_t startPos = 0;
    
  while((startPos = query.find(placeholder, startPos)) != std::string::npos) {
         query.replace(startPos, placeholder.length(), value);
         startPos += value.length();
  }
}

/*
2.2 Does the given sting contains a placeholder?

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
          // if the error code is different to zero, an error is occurred
          cout << "Error during command. Error code :" << err.code << endl;
          cout << "Error message = " << err.msg << endl;
        } else {
          // command was successful
          // do what ever you want to de with the result list
          // in this little example, the result is just printed out
          cout << "Result is:" << nl->ToString(res) << endl << endl;
        }
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
    replacePlaceholder(ourQuery, "__TOKENRANGE__", ss.str());
    executeSecondoCommand(si, nl, ourQuery);
    updateLastProcessedToken(cassandra, queryId, ip, tokenrange);
}

/*
2.2 Refresh our heartbeat data

*/
bool updateHeartbeatData(CassandraAdapter* cassandra, 
                         map<string, time_t> &heartbeatData) {


    static time_t lastUpdate = 0;
    time_t now = time(0) * 1000;               // Convert to ms

    // Data older then 15 seconds, refresh
    if(lastUpdate + 15000 < now) {
      // Clear old hearbeat data
      heartbeatData.clear();

      if (! cassandra -> getHeartbeatData(heartbeatData) ) {
          cerr << "[Error] Unable to heartbeat from system table" << endl;
          return false;
      }

      lastUpdate = time(0) * 1000; // Convert to ms
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
      cql::CQL_CONSISTENCY_ONE
  );
  
  if(! result) {
    cout << "Unable to update heartbeat in system_state table" << endl;
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
void printStatusMessageAndWait(vector<TokenRange> &allTokenRanges, 
                              vector<TokenRange> &processedIntervals) {
        
      cout << "[Info] We have " << processedIntervals.size() << " of "
           << allTokenRanges.size() << " token ranges processed" << endl;
      cout << "[Info] Sleep 5 seconds and check the ring again" << endl;
      
      sleep(5);
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
          
          cout << "[Info] Skipping already processed TokenRange: " 
               << tokenrange;
          
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
        return; 
      }

      // Find start offset
      size_t offset = findFirstLocalTokenrange(allTokenRanges, 
                                               localTokenRanges);
      
      RANGE_MODE mode = LOCAL_TOKENRANGE;
      
      for(size_t position = 0; position < allTokenRanges.size(); ++position) {
        size_t realPos = (position + offset) % allTokenRanges.size();
        TokenRange range = allTokenRanges[realPos];
        cout << "[Debug] Handling tokenrange: " << range;
        
        if(range.isLocalTokenRange(ip)) {
          cout << "Set to local" << endl;
          mode = LOCAL_TOKENRANGE;
        } else {
          
          // refresh heartbeat data
          if( ! updateHeartbeatData(cassandra, heartbeatData)) {
             cout << "[Error] Unable to refresh heartbeat data" << endl;
          }  
          
          time_t now = time(0) * 1000; // Convert to ms
          
          if(heartbeatData[range.getIp()] + NODE_TIMEOUT > now) {  
            cout << "[Info] Set to foreign: " << range;
              mode = FOREIGN_TOKENRANGE;
          } else {
            cout << "[Info] Treat range as local, because node is dead: " 
                 << range;
          }
        }
        
        // Process range - it's a local token range
        if(mode == LOCAL_TOKENRANGE) {
          executeQueryForTokenrangeIfNeeded(cassandra, query, queryId, ip, 
                                       si, nl, processedIntervals, range);
        }
      }
      
      printStatusMessageAndWait(allTokenRanges, processedIntervals);
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
          cout << "[Info] Reset complete, wait for new queries" << endl;
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
       << " -i <cassandra-ip> -k <keyspace> -p <secondo-port>"  << endl;
          
  cerr << endl;
  cerr << "-i <IP-Address> - The cassandra node to connect to" << endl;
  cerr << "-k <Keyspace> - The keyspace to open" << endl;
  cerr << "-p <Port> - The SECONDO Server port to connect to" << endl;
  exit(EXIT_FAILURE);
}

/*
2.7 Main method

*/
int main(int argc, char* argv[]){

  if(argc != 7) {
     printHelpAndExit(argv[0]);
  }
  
  // Parse commandline args
  string cassandraNodeIp = "";
  string cassandraKeyspace = "";
  string secondoPort = "";
  
  int option = 0;
  while ((option = getopt(argc, argv,"i:k:p:")) != -1) {
     switch (option) {
      case 'i':
           cassandraNodeIp = string(optarg);
           break;
      case 'k':
           cassandraKeyspace = string(optarg);
           break;
      case 'p':
           secondoPort = string(optarg);
           break;
      default:
        printHelpAndExit(argv[0]);
     }
  }
   
  string secondoHost = string("127.0.0.1");
  SecondoInterface* si = initSecondoInterface(secondoHost, secondoPort);

  if(si == NULL) { 
    return -1;
  }
  
  cout << "SecondoInterface successfully initialized" << endl;
  
  CassandraAdapter* cassandra = 
     getCassandraAdapter(cassandraNodeIp, cassandraKeyspace);
  
  if(cassandra == NULL) { 
    return -1;
  }
  
  cout << "Connection to cassandra successfull" << endl;

  // Gernerate UUID
  boost::uuids::uuid uuid = boost::uuids::random_generator()();
  const string myUuid = boost::lexical_cast<std::string>(uuid);
  cout << "Our id is: " << myUuid << endl;
  
  // Main Programm
  pthread_t targetThread;

  startHeartbeatThread(cassandraNodeIp, cassandraKeyspace, targetThread);

  mainLoop(si, cassandra, cassandraNodeIp, cassandraKeyspace, myUuid);
  
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

