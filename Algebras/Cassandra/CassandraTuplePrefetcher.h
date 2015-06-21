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

#include "CassandraResult.h"
#include "CassandraTuplePrefetcher.h"

using namespace std;

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
       : cassandraResult(NULL) {
          
      cassandraResult = new CassandraResult(mySession, 
             myStatement, myConsistenceLevel);
          
      init();
   }
   
   CassandraTuplePrefetcher(CassSession* mySession, vector<string> myQueries,
      CassConsistency myConsistenceLevel) 
       : cassandraResult(NULL) {
          
      cassandraResult = new CassandraResult(mySession, 
             myQueries, myConsistenceLevel);
             
      init();
   }
         
      
   virtual ~CassandraTuplePrefetcher() {
      if(cassandraResult != NULL) {
         delete cassandraResult;
         cassandraResult = NULL;
      }
      
      pthread_mutex_destroy(&queueMutex);
      pthread_cond_destroy(&queueCondition);
   }
   
   void setProducerThread(pthread_t &thread) {
      producerThread = thread;
   }
   
   void startTuplePrefetch();
   void prefetchTuple();
   string* getNextTuple();
   
private:
   
   void init() {
      // Init mutex and condition
      pthread_mutex_init(&queueMutex, NULL);
      pthread_cond_init(&queueCondition, NULL);
   
      startTuplePrefetch();
   }
   
   void insertToQueue(string *fetchedTuple);

   CassandraResult *cassandraResult;
   queue<string*> tuples;
   pthread_mutex_t queueMutex;
   pthread_cond_t queueCondition;
   pthread_t producerThread;
};

} // Namespace


#endif
