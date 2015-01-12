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

#include "heartbeat.h"

 
/*
2.1 Update Heartbeat timestamp

*/
  bool HeartbeatUpdater::updateHeartbeat() {  
    // Build CQL query
    stringstream ss;
    ss << "UPDATE system_state set heartbeat = unixTimestampOf(now()) ";
    ss << "WHERE ip = '";
    ss << cassandraIp;
    ss << "';";
    
    return executeQuery(ss.str());
  } 

/*
2.1 Execute a query in cassandra

*/
  bool HeartbeatUpdater::executeQuery(string query) {
    // Update last executed command
    bool result = cassandra -> executeCQLSync(
      query,
      CASS_CONSISTENCY_ONE 
    );
  
    if(! result) {
      cout << "Unable to update heartbeat in system_state table" << endl;
      cout << "CQL Statement: " << query << endl;
      return false;
    }

    return true;
  }

/*
2.1 Main loop - send heartbeat messages all n seconds

*/
  void HeartbeatUpdater::run() {
    while(active) {
      updateHeartbeat();
      sleep(HEARTBEAT_UPDATE);
    }
  }

/*
2.1 Stop the main loop

*/
  void HeartbeatUpdater::stop() {
    active = false;
    
    // Disconnect from cassandra
    if(cassandra != NULL) {
      cassandra -> disconnect();
      delete cassandra;
      cassandra = NULL;
    }
    
  }
  
