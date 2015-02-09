/*
----
This file is part of SECONDO.

Copyright (C) 2007,
Faculty of Mathematics and Computer Science,
Database Systems for New Applications.

SECONDO is free software; you can redistribute it and/or modify
it under the Systems of the GNU General Public License as published by
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

#include "state.h"

#ifndef __QEXECUTOR_WORKER__
#define __QEXECUTOR_WORKER__

//#define QUERY_WORKER_DEBUG

class SecondoWorker {

public:
   SecondoWorker (CassandraAdapter* myCassandra, string mySecondoHost, 
   string mySecondoPort, WorkerQueue *myTokenQueue, size_t myWorkerId, 
   QueryexecutorState *myQueryExecutorState) 
   : cassandra(myCassandra), secondoHost(mySecondoHost), 
   secondoPort(mySecondoPort), queryComplete(false), 
   shutdown(false), query(NULL), tokenQueue(myTokenQueue),
   workerId(myWorkerId), queryExecutorState(myQueryExecutorState) {
   
      si = initSecondoInterface(secondoHost, secondoPort);

      if(si != NULL) { 
          cout << "SecondoInterface successfully initialized: " 
             << secondoHost << " / " << secondoPort << endl;
      }
      
      pthread_mutex_init(&processMutex, NULL);
      pthread_cond_init(&processCondition, NULL);  
      
      queryExecutorState -> setState(workerId, "Idle");
   }
      
   virtual ~SecondoWorker() {
      // Shutdown the SECONDO interface
      if(si) {
         si->Terminate();
         delete si;
         si = NULL;
      }
      
      if(query != NULL) {
         delete query;
         query = NULL;
      }
      
      pthread_mutex_destroy(&processMutex);
      pthread_cond_destroy(&processCondition);
      exit(-1);
   }
   
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
                       config, errMsg, multiUser)) {

        // connection failed, handle error
        cerr << "Cannot initialize secondo system" << endl;
        cerr << "Error message = " << errMsg << endl;
        shutdown = true;

        return NULL;
     }
  
     return si;
   }
   
   WorkerQueue* getTokenQueue() {
      return tokenQueue;
   }
   
   void submitQuery(string &myQuery, size_t myQueryId) {
      
      if(shutdown) {
         cout << "SECONDO worker is down [ " << secondoPort << " ]: " 
              << "ignoring query" << endl;
         return;
      }
      
      pthread_mutex_lock(&processMutex);
      
      queryComplete = false;
      query = new string(myQuery);
      queryId = myQueryId;
      pthread_cond_broadcast(&processCondition);
      
      pthread_mutex_unlock(&processMutex);
   }
   
   bool isQueryComplete() {
      return queryComplete;
   }
   
   void stop() {
      shutdown = true;
      pthread_cond_broadcast(&processCondition);
      pthread_join(workerThread, NULL);
   }
   
   void waitForQueryCompletion() {
      pthread_mutex_lock(&processMutex);
      
      while(! isQueryComplete()) {
           pthread_cond_wait(&processCondition, &processMutex);
      } 
      
      pthread_mutex_unlock(&processMutex);
   }

   
   /*
   2.1 Update global query status

   */
   bool updateLastProcessedToken(TokenRange tokenrange, string &queryuuid) {
  
     // Build CQL query
     stringstream ss;
     ss << "INSERT INTO system_progress ";
     ss << "(queryid, ip, begintoken, endtoken, queryuuid) ";
     ss << "values(";
     ss << "" << queryId << ",",
     ss << "'" << secondoHost << "',";
     ss << "'" << tokenrange.getStart() << "',",
     ss << "'" << tokenrange.getEnd() << "',",
     ss << "'" << queryuuid << "'",
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
   

   void mainLoop() {
      
      if(si == NULL) {
         cout << "---> [ " << secondoPort 
              << " ]: Unable to connect to SECONDO, unable to start MainLoop" 
              << endl;
         return;
      }
      
      nl = si->GetNestedList();
      NList::setNLRef(nl);
      
      while (! shutdown) {
         pthread_mutex_lock(&processMutex);
         while(query == NULL) {
            pthread_cond_wait(&processCondition, &processMutex);
            
            if(shutdown) {
               return;
            }
         }
         
         if(query != NULL) {
            if(QEUtils::containsPlaceholder(*query, "__TOKENRANGE__")) {
               
               while(true) {
                  TokenRange tokenRange = tokenQueue->pop();
                  
                  // special terminal token
                  if(tokenRange.getStart() == 0 && tokenRange.getEnd() == 0) {
                     break;
                  }
                  
                  executeTokenQuery(*query, tokenRange);
               }
               
            } else {
               executeSecondoCommand(*query);
            }
            
            delete query;
            query = NULL;
         }
         
         queryComplete = true;

#ifdef QUERY_WORKER_DEBUG
         cout << "---> [ " << secondoPort << " ]: Query done" << endl;
#endif
         
         pthread_cond_broadcast(&processCondition);
         pthread_mutex_unlock(&processMutex);
      }
   }
   
   
   void setWorkerThread(pthread_t &thread) {
      workerThread = thread;
   }
   
private:
   
   /*
   2.2 Execute a command in SECONDO

   */
   void executeSecondoCommand(string command) {
  
#ifdef QUERY_WORKER_DEBUG
           cout << "Executing command [ " << secondoPort << " ]: " 
                << command << endl;
#endif
           
           ListExpr res = nl->TheEmptyList(); // will contain the result
           SecErrInfo err;                 // will contain error information
        
           si->Secondo(command, res, err); 

           // check whether command was successful
           if(err.code!=0){ 
             cout << "Error during command. Error code [ " << secondoPort 
                  << " ]: " << err.code << " / " << err.msg << endl;
           } else {
#ifdef QUERY_WORKER_DEBUG
             // command was successful
             cout << "Result is [ " << secondoPort << " ]: " 
                  << nl->ToString(res) << endl;
#endif
           }
   }
   
   /*
   2.2 Execute a token query for a given tokenrange

   */
   void executeTokenQuery(string query, 
                         TokenRange &tokenrange) {
  
       stringstream ss;
       ss << "'" << tokenrange.getStart() << "'";
       ss << ", ";
       ss << "'" << tokenrange.getEnd() << "'";
       
       stringstream statestream;
       statestream << "[" << tokenrange.getStart() << "";
       statestream << ", ";
       statestream << "" << tokenrange.getEnd() << "]";
       statestream << " [data node: " << tokenrange.getIp() << "]";
        
       queryExecutorState -> setState(workerId, statestream.str());
  
       // Copy query string, so we can replace the
       // placeholder multiple times
       string ourQuery = string(query);
    
       // Replace token range placeholder
       QEUtils::replacePlaceholder(ourQuery, "__TOKENRANGE__", ss.str());
    
       // Replace Query UUID placeholder
       string myQueryUuid;
       QEUtils::createUUID(myQueryUuid);
       QEUtils::replacePlaceholder(ourQuery, "__QUERYUUID__", myQueryUuid);
    
       executeSecondoCommand(ourQuery);
       
       updateLastProcessedToken(tokenrange, myQueryUuid);
       queryExecutorState -> setState(workerId, "Idle");
   }
   
   CassandraAdapter* cassandra;
   SecondoInterface* si;
   NestedList* nl;
   string secondoHost;
   string secondoPort;
   bool queryComplete;
   bool shutdown;
   string* query;
   size_t queryId;
   WorkerQueue *tokenQueue;
   
   // Thread id
   size_t workerId;
   
   // QueryExecutor state
   QueryexecutorState *queryExecutorState;
   
   // Thread handling
   pthread_t workerThread;
   pthread_mutex_t processMutex;
   pthread_cond_t processCondition;
};

/*
2.4 start the secondo worker thread

*/
void* startSecondoWorkerThreadInternal(void *ptr) {
  SecondoWorker* sw = (SecondoWorker*) ptr;
  sw -> mainLoop();
  
  return NULL;
}

void startSecondoWorkerThread(SecondoWorker *worker) {
  
   pthread_t targetThread;
   pthread_create(&targetThread, NULL, 
                  &startSecondoWorkerThreadInternal, worker);
   

   worker->setWorkerThread(targetThread);
}

#endif