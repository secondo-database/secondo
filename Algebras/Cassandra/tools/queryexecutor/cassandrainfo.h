/*
----
This file is part of SECONDO.

Copyright (C) 2015,
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


1 Encapsulates details about the logical ring of cassandra


1.1 Includes

*/

#ifndef _cassandrainfo_h
#define _cassandrainfo_h

#include <stdlib.h>

/*
1.2 defines

*/

// Refresh heartbeat status all n seconds
#define HEARTBEAT_REFRESH_INTERVAL 5 

// Timeout in ms for receiving heartbeat messages from other nodes
#define HEARTBEAT_NODE_TIMEOUT 30000

// Try to refresh data n times
#define CASSANDRA_REFRESH_RETRY 15

/*
2.0 The class CassandraInfo encapsulates details
about the logical ring and the heartbeat data
of the QPNs.

*/
class CassandraInfo {
public:
   
   CassandraInfo(CassandraAdapter* myCassandra) 
     : cassandra(myCassandra) {
      
   }
   
   /*
   2.1 Refresh the ring info and the heartbeat info; only executed 
       when lastUpdate is older then HEARTBEAT_REFRESH_INTERVAL
   
   */
   bool refreshData(size_t queryId) {
      bool ringInfoResult = false;
      bool heartbeatDataResult = false;
      
      // Timestamp of the last heartbeat refresh
      static time_t lastUpdate = 0;
   
      time_t now = time(0);

      // Cache version is up-to-date
      if(lastUpdate + HEARTBEAT_REFRESH_INTERVAL >= now) {
         return true;
      }
      
      for(size_t i = 0; i < CASSANDRA_REFRESH_RETRY; i++) {
         if(! ringInfoResult) {
            ringInfoResult = refreshRingInfo(queryId);
         }
         
         if( ! heartbeatDataResult ) {
            heartbeatDataResult = refreshHeartbeatData();
         }
         
         if(ringInfoResult && heartbeatDataResult) {
            break;
         }
         
         // Wait one second and try again
         sleep(1);
      }
      
      // Update last refresh timestamp
      lastUpdate = time(0); 

      return ringInfoResult && heartbeatDataResult;
   }
   
   void refreshDataOrExit(size_t queryId) {
      bool result = refreshData(queryId);
      
      if(! result) {
         exit(EXIT_FAILURE);
      }
   }
   
   bool isNodeAlive(string ip) {
      refreshHeartbeatData();
      time_t now = time(0) * 1000; // Convert to ms
   
      if(heartbeatData[ip] + HEARTBEAT_NODE_TIMEOUT > now) {
         return true;
      }
      
      return false;
   }
   
   bool isQueryExecutedCompletely() {
      return allTokenRanges.size() == processedTokenRanges.size();
   }
   
   vector<TokenRange> getAllTokenRanges() {
      return allTokenRanges;
   }
   
   vector<TokenRange> getProcessedTokenRanges() {
      return processedTokenRanges;
   } 
   
   map<string, time_t> getHeartbeatData() {
      return heartbeatData;
   }
   
private:
   
   /*
   2.2 Refresh heartbeat data

   */
   bool refreshHeartbeatData() {

     heartbeatData.clear();

     if (! cassandra -> getHeartbeatData(heartbeatData) ) {
        cerr << "[Error] Unable to heartbeat from system table" << endl;
        return false;
     }

     return true;
   }

   /*
   2.2 Refresh our information about the cassandra ring

   */
   bool refreshRingInfo(size_t queryId) {
  
       // Clear transient data
       allTokenRanges.clear();
       processedTokenRanges.clear();

       // Refresh data
       if (! cassandra -> getTokenRangesFromSystemtable(allTokenRanges) ) {
         cerr << "[Error] Unable to collect token ranges from system table" 
              << endl;
         return false;
       }
       
       // Reverse the data of the logical ring, so we can interate from
       // MAX to MIN
       reverse(allTokenRanges.begin(), allTokenRanges.end());
   
       if (! cassandra -> getProcessedTokenRangesForQuery(
                  processedTokenRanges, queryId) ) {
      
         cerr << "[Error] Unable to collect processed token ranges from "
              << "system table" << endl;
         return false;
       }
     
     return true;
   }
   
   
   CassandraAdapter* cassandra;
   vector<TokenRange> allTokenRanges;
   vector<TokenRange> processedTokenRanges;
   map<string, time_t> heartbeatData;
};

#endif
