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

#ifndef _CASSANDRA_CONNECTIONPOOL_H
#define _CASSANDRA_CONNECTIONPOOL_H

#include "CassandraAdapter.h"

using namespace std;

//namespace to avoid name conflicts
namespace cassandra {

// Prototype class
class CassandraAdapter;
 
/*
2.1 Wrapper class for cassandraAdapter objects
This class is used in the connection pool, to hold
and identify connections

*/
class CassandraConnection {
  
public:
  
  CassandraConnection(bool mySingleNode, string myHostname, 
                      string myKeyspace) : 
                      singleNodeLoadBalancing(mySingleNode), 
                      hostname(myHostname), keyspace(myKeyspace), 
                      adapter(NULL) {
  }

/* 
2.1.1 Get the singleNodeLoadBalancing policy for this connection

*/
  bool getSingleNodeLoadBalancing() const {
    return singleNodeLoadBalancing;
  }

/* 
2.1.2 Get the hostnamne for this connection

*/
  string getHostname() const {
    return hostname;
  }

/* 
2.1.3 Get the keyspace for this connection

*/
  string getKeyspace() const {
    return keyspace;
  }

/* 
2.1.4 Associate a cassandra adapter with this object

*/
  void setAdapter(CassandraAdapter* myAdapter) {
    adapter = myAdapter;
  }

/* 
2.1.5 Return the associated cassandra adapter

*/
  CassandraAdapter* getAdapter() {
    return adapter;
  }
  
/* 
2.1.6 Close and destory this connection

*/
  virtual ~CassandraConnection() {

    if(adapter != NULL) {      
      cout << "[ConnectionPool] Destroy connetion to: " 
         << hostname << " / " << keyspace << " / "
         << singleNodeLoadBalancing << endl;
    
      adapter -> disconnect();
      delete adapter;
      adapter = NULL;
    }
  }
  
/*
2.4.7 Operator ==

*/    
 inline bool operator== (const CassandraConnection &connection) {
   
   if((singleNodeLoadBalancing == connection.getSingleNodeLoadBalancing()) &&
     (hostname == connection.getHostname()) &&
     (keyspace == connection.getKeyspace())) {
     return true;
   } 
   
   return false;
 }
  
  
private:
  bool singleNodeLoadBalancing;
  string hostname;
  string keyspace;
  CassandraAdapter* adapter;
};

/*
2.2 This is our cassandra connection pool. It holds open connections for
reusage. It is implemented as a singleton. The method destory is called 
during the destruction of the CassandraAlgebra.

*/
class CassandraConnectionPool {
  
public:
  static CassandraConnectionPool* Instance();

/*
2.2.1 Request a connection the given hostname, keyspace and
with the specified singleNodeLoadBalancing

*/
  CassandraAdapter* getConnection(string myHostname, 
                                  string myKeyspace, bool mySingleNode);

/*
2.2.2 Destory the connection pool

*/
  void destroy();

/*
2.2.3 Destructor

*/
  virtual ~CassandraConnectionPool() {
    destroy();
  }
  
private:
  // We are a singleton, public constructors are not available
  CassandraConnectionPool() {};
  CassandraConnectionPool(CassandraConnectionPool const&) {};
  
  // Our connections
  vector<CassandraConnection*> connections;
  
  // The singleton instance
  static CassandraConnectionPool* instance;
};

}


#endif