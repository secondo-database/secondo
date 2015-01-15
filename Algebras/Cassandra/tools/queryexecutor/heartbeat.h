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

#ifndef __QEXECUTOR_HEARTBEAT__
#define __QEXECUTOR_HEARTBEAT__

#include <string>
#include <iostream>
#include <map>
#include <algorithm>

#include <unistd.h>
#include <stdlib.h>
#include <time.h>

#include "../../CassandraAdapter.h"
#include "SecondoInterface.h"
#include "NestedList.h"
#include "NList.h"
#include "CassandraAdapter.h"
#include "CassandraResult.h"


/*
1.2 Defines

*/
// Timeout in ms for receiving heartbeat messages from other nodes
#define HEARTBEAT_NODE_TIMEOUT 30000

// Send heartbeat message to cassandra all n seconds
#define HEARTBEAT_UPDATE 5

// Refresh heartbeat status of other nodes all n seconds
#define HEARTBEAT_REFRESH_DATA 15

/*
1.1 Usings

*/
using namespace std;
using namespace cassandra;


/*
2.0 Helper class for sending our own hearbeat
messages to cassandra. This class will be executed
in the background in its own thread.

*/
class HeartbeatUpdater {
 
public:
  
  HeartbeatUpdater(string myCassandraIp, string myCassandraKeyspace) 
     : cassandraIp(myCassandraIp), 
     cassandraKeyspace(myCassandraKeyspace),
     cassandra(NULL), active(true) {
        
       // Connect to cassandra
     cassandra = new CassandraAdapter
        (cassandraIp, cassandraKeyspace);
      
     cout << "[Heartbeat] Connecting to " << cassandraIp;
     cout << " / " << cassandraKeyspace << endl;
  
     if(cassandra != NULL) {
       cassandra -> connect(false);
     } else {
       cerr << "[Heartbeat] Unable to connect to cassandra, ";
       cerr << "exiting thread" << endl;
       active = false;
     }
  }
  
  virtual ~HeartbeatUpdater() {
    cout << "[Heartbeat] Shutdown because destructor called" << endl;
    stop();
  }


bool updateHeartbeat();
bool executeQuery(string query);
void run();
void stop();

private:
  string cassandraIp;
  string cassandraKeyspace;
  CassandraAdapter* cassandra;
  bool active;

};

#endif
