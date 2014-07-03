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

#include "CassandraConnectionPool.h"
#include "CassandraAdapter.h"

// Activate debug messages
#define __DEBUG__

//namespace to avoid name conflicts
namespace cassandra {
  
  // Global static pointer used to ensure a single instance of the class.
  CassandraConnectionPool* CassandraConnectionPool::instance = NULL;
  
  CassandraConnectionPool* CassandraConnectionPool::Instance() {
    if (!instance) {
      instance = new CassandraConnectionPool();
    }
    return instance;
  }
  
  void CassandraConnectionPool::destroy() {

#ifdef __DEBUG__
    cout << "[ConnectionPool] Destory Pool called" << endl;
#endif
    
    // Cleanup connections
    for(size_t i = 0; i < connections.size(); ++i) {
      CassandraConnection* connection = connections[i];
      delete connection;
      connections[i] = NULL;
    }

    connections.clear();
  }
  
  CassandraAdapter* CassandraConnectionPool::getConnection(
     string myHostname, string myKeyspace, bool mySingleNode) {

#ifdef __DEBUG__
    cout << "[ConnectionPool] Get Connection called" << endl;
#endif
    
    // Create connection object
    CassandraConnection* connection = new CassandraConnection(mySingleNode, 
                                       myHostname, myKeyspace);
    
    for(size_t i = 0; i < connections.size(); ++i) {
      CassandraConnection* tryConnection = connections[i];
      if(*tryConnection == *connection) {
        cout << "[ConnectionPool] Connection found, reusing" << endl;
        delete connection;
        return tryConnection->getAdapter();
      }
    }
    
#ifdef __DEBUG__    
    cout << "[ConnectionPool] Create new Connection" << endl;
#endif
    
    CassandraAdapter* adapter = new CassandraAdapter(myHostname, myKeyspace);
    
    adapter -> connect(mySingleNode);
    
    // Save connection
    connection -> setAdapter(adapter);
    connections.push_back(connection);
    
    return adapter;   
  }
}
