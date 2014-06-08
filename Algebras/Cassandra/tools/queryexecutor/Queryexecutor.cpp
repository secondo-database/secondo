
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
#include <stdlib.h>
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


/*
1.2 Using

*/
using namespace std;


class HartbeatUpdater {
 
public:
  
  HartbeatUpdater(string myIp, cassandra::CassandraAdapter* myCassandra) 
     : ip(myIp), cassandra(myCassandra), active(true) {
  }
  
/*
2.1 Update Hartbeat timestamp

*/
  bool updateHartbeat() {  
    // Build CQL query
    stringstream ss;
    ss << "UPDATE status set hartbeat = unixTimestampOf(now()) ";
    ss << "WHERE ip = '";
    ss << ip;
    ss << "';";
    
    // Update last executed command
    bool result = cassandra -> executeCQLSync(
      ss.str(),
      cql::CQL_CONSISTENCY_ONE
    );
  
    if(! result) {
      cout << "Unable to update hartbeat in status table" << endl;
      cout << "CQL Statement: " << ss.str() << endl;
      return false;
    }

    return true;
  }
  
  void run() {
    while(active) {
      updateHartbeat();
      sleep(5);
    }
  }
  
  void stop() {
    active = false;
  }
  
private:
  string ip;
  cassandra::CassandraAdapter* cassandra;
  bool active;
};


/*
2.0 Init the secondo c++ api

*/

SecondoInterface* initSecondoInterface() {

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
  string host = "localhost";
  string port = "1234";
  bool multiUser = true;
  string errMsg;          // return parameter
  
  // try to connect
  if(!si->Initialize(user,passwd,host,port,config,errMsg,multiUser)){
     // connection failed, handle error
     cerr << "Cannot initialize secondo system" << endl;
     cerr << "Error message = " << errMsg << endl;
     return NULL;
  }
  
  return si;
}

/*
2.1 Create meta tables queries and status

*/
bool createMetatables(cassandra::CassandraAdapter* cassandra) {
  
  // Create queries table
  bool result = cassandra -> executeCQLSync(
    "CREATE TABLE IF NOT EXISTS queries "
        "(id INT, query TEXT, PRIMARY KEY(id));",
    cql::CQL_CONSISTENCY_ALL
  );
 
  if(! result) {
     cout << "Unable to create queries table" << endl;
     return false;
  }
  
  // Create state table
  result = cassandra -> executeCQLSync(
    "CREATE TABLE IF NOT EXISTS status "
        "(ip TEXT, hartbeat BIGINT, lastquery INT, PRIMARY KEY(ip));",
    cql::CQL_CONSISTENCY_ALL
  );
  
  if(! result) {
     cout << "Unable to create status table" << endl;
     return false;
  }
  
  return true;
}


/*
2.1 Update last executed command

*/
bool updateLastCommand(cassandra::CassandraAdapter* cassandra, 
                       size_t lastCommandId, string ip) {
  
  // Build CQL query
  stringstream ss;
  ss << "UPDATE status set lastquery = ";
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
     cout << "Unable to update last executed query in status table" << endl;
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
cassandra::CassandraAdapter* getCassandraAdapter(string cassandraHostname, 
                                                 string cassandraKeyspace) {
  
  cassandra::CassandraAdapter* cassandra = 
     new cassandra::CassandraAdapter(cassandraHostname, cassandraKeyspace);
  
  cassandra -> connect(true);
  
  // Connection successfully?
  if(cassandra == NULL) {
    return NULL;
  }
  
  if(! createMetatables(cassandra) ) {
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
          cout << "Command successfully processed" << endl;
          cout << "Result is:" << endl;
          cout << nl->ToString(res) << endl << endl;
        }
}

/*
2.2 Handle a multitoken query (e.g. query ccollectrange('192.168.1.108', 
     'secondo', 'relation2', 'ONE',__TOKEN__);

*/
void handleTokenQuery(string &query, 
                      vector<cassandra::TokenInterval> &localTokenRange,
                      SecondoInterface* si, NestedList* nl) {
  
  cout << "Handle Token Query called" << endl;
  
    // Generate token range queries;
    for(vector<cassandra::TokenInterval>::iterator 
        iter = localTokenRange.begin();
        iter != localTokenRange.end(); ++iter) {
 
      cassandra::TokenInterval interval = *iter;
    
      stringstream ss;
      ss << "'" << interval.getStart() << "'";
      ss << ", ";
      ss << "'" << interval.getEnd() << "'";
    
      replacePlaceholder(query, "__TOKEN__", ss.str());
    
      cout << "Query is is "  << query << endl;
      
      executeSecondoCommand(si, nl, query);

    }  
}

/*
2.3 This is the main loop of the query executor. This method fetches
  queries from cassandra and forward them to secondo.

*/
void mainLoop(SecondoInterface* si, 
              cassandra::CassandraAdapter* cassandra, string cassandraIp,
              string cassandraKeyspace) {
  
  NestedList* nl = si->GetNestedList();
  NList::setNLRef(nl);
  
  // Gernerate UUID
  boost::uuids::uuid uuid = boost::uuids::random_generator()();
  const string myUuid = boost::lexical_cast<std::string>(uuid);
  cout << "Our id is: " << myUuid << endl;
  
  // Collect logical ring configuration
  vector<long long> localTokens;
  vector<long long> peerTokens;
  vector<cassandra::TokenInterval> localTokenRange;
     
  if(! cassandra->getLocalTokens(localTokens) ) {
    cerr << "Unable to determine local tokens" << endl;
    exit(-1);
  }
  
  if (! cassandra->getPeerTokens(peerTokens) ) {
    cerr << "Unable to determine peer tokens" << endl;
    exit(-1);
  }
  
  if(! cassandra->getLokalTokenRanges(localTokenRange)) {
    cerr << "Unable to determine lokal token ranges" << endl;
    exit(-1);
  }
  
  cout << "Collecting logical ring configuration done" << endl;
  
  size_t lastCommandId = 0;
  
  while(true) {
        
        cout << "Waiting for commands" << endl;
        
        cassandra::CassandraResult* result = cassandra -> readDataFromCassandra
            ("SELECT id, query from queries", cql::CQL_CONSISTENCY_ONE);
          
        while(result != NULL && result -> hasNext()) {
          size_t id = result->getIntValue(0);
          
          string command;
          result->getStringValue(command, 1);
          replacePlaceholder(command, "__NODEID__", myUuid);
          replacePlaceholder(command, "__CASSANDRAIP__", cassandraIp);
          replacePlaceholder(command, "__KEYSPACE__", cassandraKeyspace);
          
          // Is this the next query to execute
          if(id == lastCommandId + 1) {
            
            // Simple query or token based query?
            if(containsPlaceholder(command, "__TOKEN__")) {
              handleTokenQuery(command, localTokenRange, si, nl);
            } else {
              executeSecondoCommand(si, nl, command);
            }
            
            // Update global status
            updateLastCommand(cassandra, lastCommandId, cassandraIp);
            ++lastCommandId;
          }
        }
        
        delete result;
        result = NULL;
        
        sleep(1);
  }
}

/*
2.4 start the hartbeat thread

*/
void* startHartbeatThreadInternal(void *ptr) {
  HartbeatUpdater* hu = (HartbeatUpdater*) ptr;
  hu -> run();
  
  return NULL;
}

/*
2.5 start the hartbeat thread

*/
bool startHartbeatThread(cassandra::CassandraAdapter* cassandra,
                            string cassandraIp, pthread_t &targetThread) {
  
   HartbeatUpdater hartbeatUpdater(cassandraIp, cassandra);
   pthread_create(&targetThread, NULL, 
                  &startHartbeatThreadInternal, &hartbeatUpdater);
  
  return true;
}

/*
2.6 Main method

*/
int main(int argc, char** argv){

  if(argc != 3) {
     cerr << "Usage: " << argv[0] << " ip keyspace" << endl;
     return -1;
  }
   
  // Parse commandline args
  string cassandraNodeIp = string(argv[1]);
  string cassandraKeyspace = string(argv[2]);
   
  SecondoInterface* si = initSecondoInterface();
  if(si == NULL) { 
    return -1;
  }
  
  cout << "SecondoInterface successfully initialized" << endl;
  
  cassandra::CassandraAdapter* cassandra = 
     getCassandraAdapter(cassandraNodeIp, cassandraKeyspace);
  
  if(cassandra == NULL) { 
    return -1;
  }
  
  cout << "Connection to cassandra successfull" << endl;

  // Main Programm
  pthread_t targetThread;
  startHartbeatThread(cassandra, cassandraNodeIp, targetThread);
  mainLoop(si, cassandra, cassandraNodeIp, cassandraKeyspace);
  
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

