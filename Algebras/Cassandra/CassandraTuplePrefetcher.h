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

#ifndef _CASSANDRA_TUPLEPREFETCH_H
#define _CASSANDRA_TUPLEPREFETCH_H

#define MAX_PREFETCH_TUPLES 200
#define MAX_PREFETCH_THREADS 5

#include "CassandraResult.h"
#include "CassandraTuplePrefetcher.h"

/*
1.1 Namspace

*/
namespace cassandra {

/*
2.1 Tuple prefetcher class 

*/
class CassandraTuplePrefetcher {
  
public:
   
   CassandraTuplePrefetcher(CassSession* mySession, 
           string myStatement, CassConsistency myConsistenceLevel) 
       : session(mySession), consistenceLevel(myConsistenceLevel), 
           receivedTerminals(0), shutdown(false), workdone(false),
           queueEmpty(0), queueFull(0) {
          
      queries.push_back(myStatement);
      init();
   }
   
   CassandraTuplePrefetcher(CassSession* mySession, vector<string> myQueries,
      CassConsistency myConsistenceLevel) 
       : session(mySession), consistenceLevel(myConsistenceLevel), 
         queries(myQueries), receivedTerminals(0), queueEmpty(0),
         queueFull(0) {
          
      init();
   }
      
   virtual ~CassandraTuplePrefetcher() {
      pthread_mutex_destroy(&tupleQueueMutex);
      pthread_cond_destroy(&tupleQueueCondition);
      pthread_mutex_destroy(&queryQueueMutex);
   }

/*
2.1.1 Start the prefetching of tuples in an other thread.

The prefetched tuples are placed in the tuples queue 

*/   
   void startTuplePrefetch();
   
/*
2.1.2 Get the next prefetched tuple

Returns NULL if all tuples a emmited
   
*/
   string* getNextTuple();
   
/*
2.1.3 Call back function after the prefetch thread is
created

*/  
void prefetchTuple();

/*
2.1.4 Shutdown the prefetcher

*/
void shutdownQueue();

   
private:
   
   void init() {
      // Init mutexes and conditions
      pthread_mutex_init(&tupleQueueMutex, NULL);
      pthread_cond_init(&tupleQueueCondition, NULL);
      pthread_mutex_init(&queryQueueMutex, NULL);
      
      startTuplePrefetch();
   }
   
/*
2.1.4 Insert a tuple into the tuple queue
   
*/    
   void insertToQueue(string *fetchedTuple);
   
/*
2.1.5 Get the next cassandra result to process
   
*/    
   CassandraResult* getNextResult();

/*
2.1.6 Join all prefetching threads
   
*/       
   void joinThreads();

   queue<string*> tuples;
   pthread_mutex_t tupleQueueMutex;
   pthread_cond_t tupleQueueCondition;
   vector<pthread_t*> producerThreads;
   CassSession* session;
   CassConsistency consistenceLevel;
   vector<string> queries;   
   pthread_mutex_t queryQueueMutex;
   size_t receivedTerminals;
   volatile bool shutdown;
   volatile bool workdone;
   size_t queueEmpty;   // How often was the queue empty
   size_t queueFull;    // How often was the queue full
};

} // Namespace


#endif
