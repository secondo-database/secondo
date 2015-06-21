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

 This class implements the consumer producer pattern and prefetches tuples
 from the SN. 
 
 1 Includes and defines

*/

#include "CassandraTuplePrefetcher.h"

namespace cassandra {

/*
2.2 Helper function to start the producer thread

*/
void* startPrefetchProducerThread(void *ptr) {
   CassandraTuplePrefetcher* prefetcher = (CassandraTuplePrefetcher*) ptr;
   prefetcher -> prefetchTuple();

   return NULL;
}

void CassandraTuplePrefetcher::insertToQueue(string *fetchedTuple) {
      pthread_mutex_lock(&tupleQueueMutex);
      
      while(tuples.size() >= MAX_PREFETCH_TUPLES) {
         pthread_cond_wait(&tupleQueueCondition, &tupleQueueMutex); 
      }
   
      bool wasEmpty = tuples.empty();
   
      tuples.push(fetchedTuple);
   
      if(wasEmpty) {
         pthread_cond_broadcast(&tupleQueueCondition);
      }
      
      pthread_mutex_unlock(&tupleQueueMutex);
   }
   
   void CassandraTuplePrefetcher::joinThreads() {
      for(vector<pthread_t*>::iterator iter = producerThreads.begin(); 
          iter != producerThreads.end(); iter++) {
             pthread_t *thread = *iter;
             pthread_join(*thread, NULL);
             delete thread;
      }
      
      producerThreads.clear();
   }
   
   string* CassandraTuplePrefetcher::getNextTuple() {
      pthread_mutex_lock(&tupleQueueMutex);
      int oldSize = tuples.size();
      
      while(tuples.empty()) {
         pthread_cond_wait(&tupleQueueCondition, &tupleQueueMutex); 
      }
      
      // Remove one element
      string *result = tuples.front();
      tuples.pop();
      
      // Wakeup waiting producer threads
      if(oldSize >= MAX_PREFETCH_TUPLES) {
        //cout << "Wakeup waiting worker threads" << endl;
        pthread_cond_broadcast(&tupleQueueCondition);
      }
  
      pthread_mutex_unlock(&tupleQueueMutex);
      
      // One Producer thread is done and has inserted a NULL result into 
      // the queue
      if(result == NULL) {
         receivedTerminals++;
         
         if(receivedTerminals == producerThreads.size()) {
            joinThreads();
         } else {
            return getNextTuple(); // Fetch an other tuple
         }
      }
      
      return result;
   }
   
   CassandraResult* CassandraTuplePrefetcher::getNextResult() {
      CassandraResult *cassandraResult = NULL;
      
      pthread_mutex_lock(&queryQueueMutex);
      
      if(queries.size() > 0) {
         string query = queries.back();
         queries.pop_back();
         cassandraResult = new CassandraResult(session, 
                query, consistenceLevel);
      }
  
      pthread_mutex_unlock(&queryQueueMutex);
      
      return cassandraResult;
   } 
   
   void CassandraTuplePrefetcher::prefetchTuple() {
          
      CassandraResult *cassandraResult = getNextResult();
      
      while(cassandraResult != NULL) {
      
         while(cassandraResult->hasNext()) {
      
           string key;
           string *fetchedTuple = new string();
       
           cassandraResult -> getStringValue(key, 0);
           cassandraResult -> getStringValue(*fetchedTuple, 1);
      
           // Metadata? Skip tuple
           if(key.at(0) == '_') {
  #ifdef __DEBUG__
             cout << "Skipping key: " << key << " value " << value << endl;
  #endif
             continue;
           }
           insertToQueue(fetchedTuple);
        }
     
        delete cassandraResult;
        cassandraResult = getNextResult();
     }
     
     // Notify consumer - all work is done
     insertToQueue(NULL);
   }
   
   void CassandraTuplePrefetcher::startTuplePrefetch() {
      int totalQueries = queries.size();
      size_t threads = min(MAX_PREFETCH_THREADS, totalQueries);
      
      // Create producer threads
      for(size_t i = 0; i < threads; i++) {
         pthread_t *thread = new pthread_t();
         pthread_create(thread, NULL, &startPrefetchProducerThread, this);
         producerThreads.push_back(thread);
      }
   }
}