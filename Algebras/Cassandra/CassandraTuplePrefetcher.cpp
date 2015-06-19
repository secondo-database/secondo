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
      pthread_mutex_lock(&queueMutex);
      
      while(tuples.size() >= MAX_PREFETCH_TUPLES) {
         pthread_cond_wait(&queueCondition, &queueMutex); 
      }
   
      bool wasEmpty = tuples.empty();
   
      tuples.push(fetchedTuple);
   
      if(wasEmpty) {
         pthread_cond_broadcast(&queueCondition);
      }
      
      pthread_mutex_unlock(&queueMutex);
   }
   
   string* CassandraTuplePrefetcher::getNextTuple() {
      pthread_mutex_lock(&queueMutex);
      int oldSize = tuples.size();
      
      while(tuples.empty()) {
         pthread_cond_wait(&queueCondition, &queueMutex); 
      }
      
      // Remove one element
      string *result = tuples.front();
      tuples.pop();
      
      // Wakeup waiting producer threads
      if(oldSize >= MAX_PREFETCH_TUPLES) {
        //cout << "Wakeup waiting worker threads" << endl;
        pthread_cond_broadcast(&queueCondition);
      }
  
      pthread_mutex_unlock(&queueMutex);
      
      // Producer thread is done and has inserted a NULL result into 
      // the queue
      if(result == NULL) {
         pthread_join(producerThread, NULL);
      }
      
      return result;
   }
   
   void CassandraTuplePrefetcher::prefetchTuple() {
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
     
     insertToQueue(NULL);
   }
   
   void CassandraTuplePrefetcher::startTuplePrefetch() {
      // Create producer thread
      pthread_create(&producerThread, NULL, &startPrefetchProducerThread, this);
   }
}