/*
 ----
 This file is part of SECONDO.

 Copyright (C) 2014, University in Hagen, Faculty of Mathematics and
 Computer Science, Database Systems for New Applications.

 SECONDO is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published
 by
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

 Jan Kristof Nidzwetzki

 //paragraph [1] Title: [{\Large \bf \begin{center}] [\end{center}}]
 //paragraph [10] Footnote: [{\footnote{] [}}]
 //[TOC] [\tableofcontents]

 [TOC]

 0 Overview

 1 Includes and defines

*/
#include "CassandraResult.h"

using namespace std;

//namespace to avoid name conflicts
namespace cassandra {
  
/*
2.2 Result object for one cql query

*/
bool SingleCassandraResult::hasNext() {

  // Wait for result
  if(! futureWaitCalled ) {
      futureWaitCalled = true;      
      try {
          future.wait();
      } catch(std::exception& e) {
          cerr << "Got exception while reading data: " << e.what() << endl;
          return false;
      }
  }
  
  cql::cql_result_t& result = *(future.get().result);
  return result.next();
}

void SingleCassandraResult::getStringValue(string &resultString, int pos) {
  cql::cql_result_t& result = *(future.get().result);
  result.get_string(pos, resultString);
}

int SingleCassandraResult::getIntValue(int pos) {
  int resultInt;
  cql::cql_result_t& result = *(future.get().result);
  result.get_int(pos, resultInt);
  return resultInt;
}

/*
2.3 Result object for multiple cql queries

*/
MultiCassandraResult::MultiCassandraResult(vector<string> myQueries, 
                    CassandraAdapter* myCassandraAdapter,
                    cql::cql_consistency_enum myConsistenceLevel) 
  : queries(myQueries), cassandraAdapter(myCassandraAdapter), 
  consistenceLevel(myConsistenceLevel) { 
    
    cassandraResult = NULL;
}

MultiCassandraResult::~MultiCassandraResult() {
  
  if(cassandraResult != NULL) {
    delete cassandraResult;
    cassandraResult = NULL;
  }
}

bool MultiCassandraResult::setupNextQuery() {
  cout << "Preparing next query" << endl;
  cout << "Size of queries: " << queries.size() << endl;
  
  // Delete old query
  if(cassandraResult != NULL) {
    delete cassandraResult;
    cassandraResult = NULL;
  }
  
  if(queries.empty()) {
    return false;
  }
  
  // Execute next query
  string cql = queries.back();
  queries.pop_back();
  cassandraResult = cassandraAdapter
          ->readDataFromCassandra(cql, consistenceLevel);
  
  return true;
}

void MultiCassandraResult::getStringValue(string &resultString, int pos) {
  cassandraResult -> getStringValue(resultString, pos);
}

int MultiCassandraResult::getIntValue(int pos) {
  return cassandraResult -> getIntValue(pos);
}

bool MultiCassandraResult::hasNext() {
  // No query active and we have a new query to execute
  if((cassandraResult == NULL) && (queries.size() > 0)) {
    
    // Execute the next query
    setupNextQuery();
    
    return hasNext();
  }
  
  // Do we have a next element in current query?
  if(cassandraResult->hasNext()) {
    return true;
  }
  
  // Setup next query and try again
  if(setupNextQuery()) {
    return hasNext();
  }
  
  return false;
}

/*
2.3 Multi Threaded result

*/
class WorkerThreadConfiguration {
  
public:
  int id;
  vector<string>* queries;
  queue< vector<string> >* results;
  pthread_mutex_t* queryMutex;
  pthread_mutex_t* queueMutex;
  pthread_cond_t* queueCondition;
  CassandraAdapter* cassandraAdapter;
  cql::cql_consistency_enum* consistenceLevel;
};

/*
2.3.1 Worker thread helper function
Insert a result into the result list, wakeup
consumer if necessary

*/
void insertIntoResult(WorkerThreadConfiguration* configuration, 
                      vector<string> &result) {
  
  pthread_mutex_lock(configuration->queueMutex);
  
  while(configuration->results->size() >= 100) {
    //cout << "[WorkerThread] wait for condition" << endl;
    // Wait for new elements
    pthread_cond_wait(configuration->queueCondition, 
                      configuration->queueMutex); 
    //cout << "[WorkerThread] wakeup" << endl;
  }
  
  
  bool wasEmpty = configuration->results->empty();
  configuration->results->push(result);
  
  // Wakeup consumer
  if(wasEmpty) {
    pthread_cond_signal(configuration->queueCondition);
  }
  
  pthread_mutex_unlock(configuration->queueMutex);
}

/*
2.3.1 Worker thread main function

*/
void* collectWorkerThread(void *ptr) {

  WorkerThreadConfiguration* configuration = (WorkerThreadConfiguration*) ptr;
  
  cout << "New thread started: " << configuration->id << endl;
  
  while(true) {
    string cql;
    bool empty;
    
    pthread_mutex_lock(configuration->queryMutex);
    empty = configuration->queries->empty();
    if(!empty) {
      cql = configuration->queries->back();
      configuration->queries->pop_back();
    }
    
    pthread_mutex_unlock(configuration->queryMutex);
    
    if(empty) {
      cout << "[Thread-" << configuration->id << "] exiting, "
           << "because all queries are processed" << endl;
      break;
    }
    
    cout << "[Thread-" << configuration->id << "] executing: " << cql << endl;
    
    CassandraResult* cassandraResult = configuration -> cassandraAdapter
          ->readDataFromCassandra(cql, *(configuration -> consistenceLevel));
    
    while(cassandraResult->hasNext()) {
      vector<string> result;
      
      for(size_t i = 0; i < 2; ++i) {
        string resultString;
        cassandraResult->getStringValue(resultString, i);
        result.push_back(resultString);  
      }
      
      insertIntoResult(configuration, result);
    }
          
    if(cassandraResult != NULL) { 
      delete cassandraResult;
    }
  }
    
  // Insert thread finished result 
  vector<string> emptyVector;
  insertIntoResult(configuration, emptyVector);
  
  cout << "[Thread-" << configuration->id << "] exit" << endl;
  delete(configuration);
  pthread_exit(NULL);
}


MultiThreadedCassandraResult::MultiThreadedCassandraResult(
                    vector<string> myQueries, 
                    CassandraAdapter* myCassandraAdapter,
                    cql::cql_consistency_enum myConsistenceLevel) 
  : MultiCassandraResult(myQueries, myCassandraAdapter, myConsistenceLevel), 
  runningThreads(0), firstCall(true) { 

    // Init mutex and condition
    pthread_mutex_init(&queryMutex, NULL);
    pthread_mutex_init(&queueMutex, NULL);
    pthread_cond_init(&queueCondition, NULL);
    
    for(size_t i = 0; i < 5; ++i) {
      cout << "[Threaded] Spawning new thread" << endl;
      pthread_t targetThread;
      
      WorkerThreadConfiguration* config = new WorkerThreadConfiguration();
      config->id = i;
      config->queryMutex = &queryMutex;
      config->queueMutex = &queueMutex;
      config->queueCondition = &queueCondition;
      config->results = &results;
      config->queries = &queries;
      config->cassandraAdapter = cassandraAdapter;
      config->consistenceLevel = &consistenceLevel;
      
      pthread_create(&targetThread, NULL, &collectWorkerThread, config);
      threads.push_back(targetThread);
      ++runningThreads;
    }
    
    cout << "[Threaded] Init complete" << endl;
}

MultiThreadedCassandraResult::~MultiThreadedCassandraResult() {
  
  cout << "[Destructor] Discard pending queries" << endl;
   
   // Discard pending queries
   pthread_mutex_lock(&queryMutex);
   queries.clear();
   pthread_mutex_unlock(&queryMutex);
   
   // Process pending elements
   while(! hasNext() ) {
   }
   
   cout << "[Destructor] Joining threads" << endl;
   
   // Join thrads
   for(size_t i = 0; i < threads.size(); ++i) {
     pthread_join(threads.at(i), NULL);
   }
  
   pthread_mutex_destroy(&queryMutex);
   pthread_mutex_destroy(&queueMutex);
   pthread_cond_destroy(&queueCondition);

   cout << "[Destructor] Deconstructor complete" << endl;
}

bool MultiThreadedCassandraResult::hasNext() {

  pthread_mutex_lock(&queueMutex);
  int oldSize = results.size();
  
  // Remove first element
  if(!firstCall && !results.empty() ) {
    //cout << "[HasNext] pop element" << endl;
    results.pop();
  }
  
  if(firstCall) {
    firstCall = false;
  }
  
  // Wakeup waiting worker threads
  if(oldSize >= 100) {
    //cout << "Wakeup waiting worker threads" << endl;
    pthread_cond_broadcast(&queueCondition);
  }

  // Handle Thread terminations (Vector with 0 entries)
  while(! results.empty() && (results.front()).size() == 0) {
    --runningThreads;
    cout << "[HasNext] Thread termination: " << runningThreads << endl;
    results.pop();
  }
  
  if(! results.empty() ) {
    pthread_mutex_unlock(&queueMutex);
    return true;
  }
  
  while(results.empty() && runningThreads > 0) {
    
    cout << "[HasNext] wait for condition" << endl;
    // Wait for new elements
    pthread_cond_wait(&queueCondition, &queueMutex); 
    cout << "[HasNext] wakeup" << endl;
    
    // Handle Thread terminations (Vector with 0 entries)
    while(! results.empty() && (results.front()).size() == 0) {
      --runningThreads;
      cout << "HasNext Thread termination: " << runningThreads << endl;
      results.pop();
    }
        
    cout << "[HasNext] running" << endl;
  }
  
  // Element found
  if(! results.empty() ) {
    pthread_mutex_unlock(&queueMutex);
    cout << "[HasNext] NEW entries found" << endl;
    return true;
  }
  
  pthread_mutex_unlock(&queueMutex);
  cout << "[HasNext] no new entries found" << endl;
  return false;

}

void MultiThreadedCassandraResult::getStringValue(string &resultString, 
                                                  int pos) {
  
    pthread_mutex_lock(&queueMutex);
    
    if( results.empty() ) {
      pthread_mutex_unlock(&queueMutex);
      return;
    }
    
    if(((int) (results.front()).size()) < pos) {
      cout << "Request for OUT OF RANGE position: " << pos
           << " / " << results.front().size() << endl;
      return;
    }
    
    resultString = (results.front()).at(pos);
    
    pthread_mutex_unlock(&queueMutex);
}

int MultiThreadedCassandraResult::getIntValue(int pos) {
  return -1;
}

  
} // Namespace