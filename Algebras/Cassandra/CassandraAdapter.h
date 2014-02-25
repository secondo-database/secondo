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

#ifndef _CASSANDRA_H
#define _CASSANDRA_H

#include <string.h>
#include <iostream>

#include <cassert>

#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/thread.hpp>
#include <boost/format.hpp>
#include <boost/algorithm/string.hpp>

#include <cql/cql.hpp>
#include <cql/cql_error.hpp>
#include <cql/cql_event.hpp>
#include <cql/cql_connection.hpp>
#include <cql/cql_session.hpp>
#include <cql/cql_cluster.hpp>
#include <cql/cql_builder.hpp>
#include <cql/cql_execute.hpp>
#include <cql/cql_result.hpp>

using namespace std;

//namespace to avoid name conflicts
namespace cassandra {

/*
2.1 Helper Functions

*/
class CassandraHelper {

public:
    static bool checkConsistenceLevel(string consistenceLevel) {
        if ((consistenceLevel.compare("ANY") == 0)
                || (consistenceLevel.compare("ONE") == 0)
                || (consistenceLevel.compare("QUORUM") == 0)
                || (consistenceLevel.compare("ALL") == 0)) {

            return true;
        }

        return false;
    }
    
    static cql::cql_consistency_enum convertConsistencyStringToEnum
      (string consistenceLevel) {
        
        if(consistenceLevel.compare("ANY") == 0) {
          return cql::CQL_CONSISTENCY_ANY;
        }
        
        if(consistenceLevel.compare("ONE") == 0) {
          return cql::CQL_CONSISTENCY_ONE;
        }
        
        if(consistenceLevel.compare("QUORUM") == 0) {
          return cql::CQL_CONSISTENCY_QUORUM;
        }
        
        if(consistenceLevel.compare("ALL") == 0) {
          return cql::CQL_CONSISTENCY_ONE;
        }
        
        return cql::CQL_CONSISTENCY_ONE;
    }

};

class CassandraResult {
  
public:
     CassandraResult(cql::cql_result_t& myResult) : result(myResult) {
     }
     
     bool hasNext() {
       return result.next();
     }
     
     void getValue(string &resultString) {
        result.get_string(0, resultString);
     }
  
private:
     cql::cql_result_t& result;
};

class CassandraAdapter {

public:
  
    CassandraAdapter(string myContactpoint, string myKeyspace) 
      : contactpoint(myContactpoint), keyspace(myKeyspace) {
    
        builder = cql::cql_cluster_t::builder();
    }
    
    virtual ~CassandraAdapter() {
         disconnect();
    }
    
    void connect() {
      try {
        
       // Connect to cassandra cluster
       builder -> add_contact_point(
          boost::asio::ip::address::from_string(contactpoint)
       );
        
       cluster = builder -> build();
       session = cluster -> connect();
        
       // Switch keyspace
       string keyspaceCql = string("USE ").append(keyspace).append(";");

       boost::shared_future<cql::cql_future_result_t> future 
          = executeCQL(keyspaceCql, cql::CQL_CONSISTENCY_ONE);

       // Wait for execution
       future.wait();
       
       if(future.get().error.is_err()) {
         cerr << "Unable to execute " << keyspaceCql << endl;
         cerr << "Error is " << future.get().error.message << endl;
         
         disconnect();
       } else {
         cout << "Cassandra: Connection successful established" << endl;
         cout << "You are connected to host " << contactpoint 
              << " keyspace " << keyspace << endl;
       }
         
      } catch(std::exception& e) {
        cerr << "Got exception while connection to cassandra: " 
             << e.what() << endl;
             
             disconnect();
      }
    }
    
    void writeDataToCassandra(string key, string value, 
                              string relation, string consistenceLevel) {
        stringstream ss;
        ss << "INSERT INTO ";
        ss << relation;
        ss << " (key, value) ";
        ss << " values ( ";
        ss << "'" << key << "', ";
        ss << "'" << value << "');";
        
        executeCQLSync(
             ss.str(), 
             CassandraHelper::convertConsistencyStringToEnum(consistenceLevel)
           );
    }
    
    CassandraResult* readDataFromCassandra(string relation, 
                                           string consistenceLevel) {
      
        if(! isConnected() ) {
           cerr << "Cassandra session not ready" << endl;
           return NULL;
        }
      
        stringstream ss;
        ss << "SELECT value from ";
        ss << relation;
        ss << ";";
        
        try { 
          boost::shared_ptr<cql::cql_query_t> cqlStatement(
            new cql::cql_query_t(ss.str(), 
               CassandraHelper::convertConsistencyStringToEnum(consistenceLevel)
            ));
          
          boost::shared_future<cql::cql_future_result_t> future 
             = session->query(cqlStatement);
          
          // Wait for result
          future.wait();
          
          if(future.get().error.is_err()) {
             cerr << "Unable to execute " << cqlStatement << endl;
             cerr << "Error is " << future.get().error.message << endl;
          } else {
          
            cql::cql_result_t& result = *(future.get().result);
            
            return new CassandraResult(result);
            
          }
        } catch(std::exception& e) {
            cerr << "Got exception while reading data: " << e.what() << endl;
        } 
        
        return NULL;
    }
    
    bool createTable(string tablename) {
        stringstream ss;
        ss << "CREATE TABLE IF NOT EXISTS ";
        ss << tablename;
        ss << " ( key text PRIMARY KEY, ";
        ss << " value text );";
         
        return executeCQLSync(ss.str(), cql::CQL_CONSISTENCY_ALL);
    }
    
    bool dropTable(string tablename) {
        stringstream ss;
        ss << "DROP TABLE IF EXISTS ";
        ss << tablename;
        ss << ";";
        
        return executeCQLSync(ss.str(), cql::CQL_CONSISTENCY_ALL);
    }
    
    void disconnect() {
      if(isConnected()) {
        
        cout << "Disconnecting from cassandra" << endl;
        
        // Close session and cluster
        session -> close();
        cluster -> shutdown();
        
        // Reset pointer
        session.reset();
        cluster.reset();
        builder.reset();
      }
    }
  
    bool isConnected() {
      if(session) {
        return true;
      } else {
        return false;
      }
    }
    
protected:
  
  bool executeCQLSync(string cql, cql::cql_consistency_enum consistency) {
    try {
      
      if(! isConnected() ) {
        cerr << "Cassandra session not ready" << endl;
        return false;
      }
      
        boost::shared_future<cql::cql_future_result_t> future 
          = executeCQL(cql, consistency);
          
        // Wait for execution
        future.wait();
        
        if(future.get().error.is_err()) {
          cerr << "Unable to execute " << cql << endl;
          cerr << "Error is " << future.get().error.message << endl;
          return false;
        }
        
        return true;
        
      } catch(std::exception& e) {
        cerr << "Got exception while creating table: " << e.what() << endl;
      }
      
      return false;
  }
  
  boost::shared_future<cql::cql_future_result_t> 
     executeCQL(string cql, cql::cql_consistency_enum consistency) {
       
               
      boost::shared_ptr<cql::cql_query_t> cqlStatement(
          new cql::cql_query_t(cql, consistency));
          
      boost::shared_future<cql::cql_future_result_t> future 
          = session->query(cqlStatement);
          
      return future; 
  }
    
private:
  string contactpoint;            // Our cassandra contact point
  string keyspace;                // Our keyspace
  string relation;                // Our relation
  boost::shared_ptr<cql::cql_builder_t> builder;
  boost::shared_ptr<cql::cql_cluster_t> cluster;
  boost::shared_ptr<cql::cql_session_t> session;
};

}

#endif
