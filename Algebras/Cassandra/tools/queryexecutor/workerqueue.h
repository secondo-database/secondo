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


1 WorkerQueue for Distributed-SECONDO


1.1 Includes

*/
#include <string>
#include <queue>

#include "CassandraAdapter.h"
#include "CassandraResult.h"

/*
1.2 usings

*/

using namespace std;

class WorkerQueue {

public:
   
   WorkerQueue(size_t myMaxElements) : maxElements(myMaxElements) {
      pthread_mutex_init(&queueMutex, NULL);
      pthread_cond_init(&queueCondition, NULL);
   }

   virtual ~WorkerQueue() {
      pthread_mutex_destroy(&queueMutex);
      pthread_cond_destroy(&queueCondition);
   }

/*
2.1 Push a tokenrange into the queue

*/
   void push(TokenRange* range) {
      pthread_mutex_lock(&queueMutex);      
      
      // Queue is full, wait for element removal
      while(isFull()) {
           pthread_cond_wait(&queueCondition, &queueMutex);
      }

      bool wasEmpty = isEmpty();
      myQueue.push(range);

      // Wakeup sleeping consumers
      if(wasEmpty) {
         pthread_cond_broadcast(&queueCondition);
      }

      pthread_mutex_unlock(&queueMutex);
   }

/*
2.2 Get the next tokenrange from the queue

*/
   TokenRange* pop() {
       pthread_mutex_lock(&queueMutex);
      
       while(myQueue.empty()) {
           pthread_cond_wait(&queueCondition, &queueMutex);
       }       

       bool wasFull = isFull();

       TokenRange* range = myQueue.front();
       myQueue.pop();

       // Wakeup sleeping consumers
       if(wasFull) {
         pthread_cond_broadcast(&queueCondition);
       }

       pthread_mutex_unlock(&queueMutex);
       
       return range;
   }

/*
2.3 is the queue empty?

*/
   bool isEmpty() {
       return myQueue.empty();
   }

/*
2.4 is the queue full?

*/
   bool isFull() {
      return myQueue.size() >= maxElements;
   }

private:
   size_t maxElements;
   queue<TokenRange*> myQueue;
   pthread_mutex_t queueMutex;
   pthread_cond_t queueCondition;
};

