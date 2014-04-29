
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

using namespace std;

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
2.1 Init the cassandra adapter, the 1st parameter
 is the initial contact point to the cassandra cluter.
 The 2nd parameter is the keyspace.

*/
cassandra::CassandraAdapter* getCassandraAdapter(string cassandraHostname, 
                                                 string cassandraKeyspace) {
  
  cassandra::CassandraAdapter* cassandra = 
     new cassandra::CassandraAdapter(cassandraHostname, cassandraKeyspace);
  
  cassandra -> connect(false);
  bool result = cassandra -> executeCQLSync(
    "CREATE TABLE IF NOT EXISTS queries "
        "(id INT, query TEXT, PRIMARY KEY(id));",
    cql::CQL_CONSISTENCY_ALL
  );
 
  if(! result) {
     cout << "Unable to connect to cassadndra" << endl;
     return NULL;
  }
  
  return cassandra;
}

/*
2.2 This is the main loop of the query executor. The method fetches
  queries from cassandra and forward them to secondo.

*/
void mainLoop(SecondoInterface* si, cassandra::CassandraAdapter* cassandra) {
  
  NestedList* nl = si->GetNestedList();
  NList::setNLRef(nl);
  
  int lastCommandId = 0;
  
  while(true) {
        
        cout << "Waiting for commands" << endl;
        
        cassandra::CassandraResult* result = cassandra -> readDataFromCassandra
            ("SELECT id, query from queries", cql::CQL_CONSISTENCY_ONE);
          
        while(result && result -> hasNext()) {
          int id = result->getIntValue(0);
          
          string command;
          result->getStringValue(command, 1);
          
          // Is this the next query to execute
          if(id == lastCommandId + 1) {
            cout << "Executing command " << command << endl;
            
            ListExpr res = nl->TheEmptyList();  // will contain the result
            SecErrInfo err;                  // will contain error information
            
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
              cout << "Command successful processed" << endl;
              cout << "Result is:" << endl;
              cout << nl->ToString(res) << endl << endl;
            }
            
            lastCommandId++;
          }
        }
        
        delete result;
        result = NULL;
        
        sleep(1);
  }
}

int main(int argc, char** argv){

  if(argc != 3) {
     cerr << "Usage: " << argv[0] << " hostname keyspace" << endl;
     return -1;
  }
   
  // Parse commandline args
  string cassandraHostname = string(argv[1]);
  string cassandraKeyspace = string(argv[2]);
   
  SecondoInterface* si = initSecondoInterface();
  if(si == NULL) { 
    return -1;
  }
  
  cout << "SecondoInterface successfull initialized" << endl;
  
  cassandra::CassandraAdapter* cassandra = 
     getCassandraAdapter(cassandraHostname, cassandraKeyspace);
  
  if(cassandra == NULL) { 
    return -1;
  }
  
  cout << "Connection to cassandra successfull" << endl;

  // Query loop
  mainLoop(si, cassandra);
  
  if(cassandra) {
    cassandra -> disconnect();
    delete cassandra;
    cassandra = NULL;
  }
  
  // Shutdown SECONDO interface
  if(si) {
    si->Terminate();
    delete si;
    si = NULL;
  }
 
  return 0;
}