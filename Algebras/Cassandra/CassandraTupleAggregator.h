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
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with SECONDO; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
----


1 Tuple aggregator for the cspread operator. This class
consumes tuples, aggregate tuple for the same key
to one aggregated chunk and send this chunk completely 
to cassandra.


1.1 Includes

*/

#ifndef __CASSANDRA_TUPLE_AGGREGATOR_
#define __CASSANDRA_TUPLE_AGGREGATOR_

#include "CassandraAdapter.h"

#define SEPARATOR "|"
#define MAX_TUPLE_IN_BUFFER 1

namespace cassandra {
   
class CassandraTupleAggrerator {

public:
   
   CassandraTupleAggrerator(CassandraAdapter *myCassandra, 
       string &myRelationName, string &myConsistence, 
       string &mySystemname) 
       : cassandra(myCassandra), relationName(myRelationName), 
       consistence(myConsistence), systemname(mySystemname), 
       lastpartitionKey(""), statement(NULL), receivedTuple(0), 
       sendTuple(0), tupleInBuffer(0) {
          
       statement = cassandra->prepareCQLInsert(relationName);
      
   }
   
   virtual ~CassandraTupleAggrerator() {
     freePreparedStatement();
   }
   
   void printTupleRatio() {
      cout << "Received " << receivedTuple << " / Send " 
           << sendTuple << " ratio: " <<  
              (float) sendTuple / (float) receivedTuple 
           << endl;
   }
   
   bool processTuple(string &partitionKey, string tuple) {

      bool result = true;
      receivedTuple++;
      
      if(statement == NULL) {
          return false;
      }

      if((partitionKey != lastpartitionKey) 
         || (tupleInBuffer >= MAX_TUPLE_IN_BUFFER)) {
            
         result = sendCurrentChunk();
         lastpartitionKey = partitionKey;
         
         ss << tuple;
      } else {
         ss << SEPARATOR << tuple;
      }
      
      tupleInBuffer++;
      
      return result;
   }
   
   void freePreparedStatement() {
      if(statement != NULL) {
         cassandra -> freePreparedStatement(statement);
         statement = NULL;
      }
   }
      
   bool sendCurrentChunk() {
      bool result = true;
      
      if(tupleInBuffer > 0) {
         stringstream tss;
         tss << sendTuple;
         string tupleNumberStr = tss.str();
      
         // Emit data
         result = cassandra->writeDataToCassandraPrepared(
                           statement,
                           lastpartitionKey,
                           systemname,
                           tupleNumberStr, 
                           ss.str(), 
                           consistence, false);
                           
         sendTuple++;                  
      }
                        
      // Reset stringstream
      ss.clear();
      ss.str(std::string());
      
      tupleInBuffer = 0;
      
      return result;
   }
   
   size_t getReceivedTuple() {
      return receivedTuple;
   }
   
protected:
   
private:
   CassandraAdapter *cassandra; // Cassandra adapter
   string relationName;         // Relation name to delete
   string consistence;          // Consistence
   string systemname;           // Name of our system
   string lastpartitionKey;          // Id of the last emited chunk
   stringstream ss;             // Stringstream for storing tuple
   const CassPrepared *statement; // The prepared insert statement
   size_t receivedTuple;        // Number of received tuple
   size_t sendTuple;            // Number of send tuple
   size_t tupleInBuffer;        // Number of tuples in the buffer
};

} // Namespace

#endif
