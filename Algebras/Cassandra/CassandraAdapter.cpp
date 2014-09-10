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

#include <string.h>
#include <iostream>
#include <climits>
#include <map>

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
#include <cql/cql_set.hpp>

#include "CqlSingleNodeLoadbalancing.h"
#include "CassandraAdapter.h"
#include "CassandraResult.h"

// Activate debug messages
//#define __DEBUG__

using namespace std;

//namespace to avoid name conflicts
namespace cassandra {

// Init static variables
const string CassandraAdapter::METADATA_TUPLETYPE = "_TUPLETYPE";

void CassandraAdapter::connect(bool singleNodeLoadBalancing) {
    try {

        // Connect to cassandra cluster
        builder -> add_contact_point(
            boost::asio::ip::address::from_string(contactpoint)
        );

        // Set our single node policy
        if(singleNodeLoadBalancing) {
          builder -> with_load_balancing_policy( 
                  boost::shared_ptr< cql::cql_load_balancing_policy_t >( 
                      new SingleNodeLoadBlancing(contactpoint)));
        }
        
        cluster = builder -> build();
        session = cluster -> connect();
        
        // Switch keyspace
        session -> set_keyspace(keyspace);
       
        cout << "Cassandra: Connection successfully established" << endl;
        cout << "You are connected to host " << contactpoint
                 << " keyspace " << keyspace << endl;
        cout << "SingleNodeLoadBalancing: " << singleNodeLoadBalancing << endl;
       
    } catch(std::exception& e) {
        cerr << "Got exception while connection to cassandra: "
             << e.what() << endl;

        disconnect();
    }
}

bool CassandraAdapter::isConnected() {
      if(session) {
        return true;
      } else {
        return false;
      }
}

void CassandraAdapter::writeDataToCassandra(string relation, string partition, 
        string node, string key, string value, string consistenceLevel, 
        bool sync) {

    // Convert consistence level
    cql::cql_consistency_enum consitence 
       = CassandraHelper::convertConsistencyStringToEnum(consistenceLevel);
       
    // Write Data and wait for result
    if(sync) {
      executeCQLSync(
          getInsertCQL(relation, partition, node, key, value),
          consitence
      );
      
    } else {
      executeCQLASync(
          getInsertCQL(relation, partition, node, key, value),
          consitence
      );
    }
}

void CassandraAdapter::writeDataToCassandraPrepared(string relation, 
        string partition, string node, string key, string value, 
        string consistenceLevel, bool sync) {

    // Statement unknown? => Prepare
    if(insertCQLid.empty()) {
       prepareCQLInsert(relation, consistenceLevel);
       
       if(insertCQLid.empty()) {
         cout << "Unable to prepare CQL insert statement" << endl;
         return;
       }
    }
    
    try {
    // Use prepared query for execution
    boost::shared_ptr<cql::cql_execute_t> boundCQLInsert(
            new cql::cql_execute_t(
              insertCQLid,         
              CassandraHelper::convertConsistencyStringToEnum(consistenceLevel)
    ));

    // Bound prameter
    boundCQLInsert -> push_back(partition);
    boundCQLInsert -> push_back(node);
    boundCQLInsert -> push_back(key);
    boundCQLInsert -> push_back(value);
    
    // Build future and execute
    boost::shared_future<cql::cql_future_result_t> future 
       = session->execute(boundCQLInsert);
    
    // Execution sync or async?
    if(sync) {
       executeCQLFutureSync(future);
    } else {
      pendingFutures.push_back(future);
      removeFinishedFutures();
    }
    
    } catch(std::exception& e) {
        cerr << "Got exception executing perpared cql query: " 
             << e.what() << endl;
    }
}

bool CassandraAdapter::prepareCQLInsert(string relation, 
                                        string consistenceLevel) {
     try {
        string cqlQuery = getInsertCQL(relation, "?", "?", "?", "?");
        cout << "Preparing insert query: "  << cqlQuery << endl;
        
        boost::shared_ptr<cql::cql_query_t> unboundInsert(
            new cql::cql_query_t(cqlQuery, 
            CassandraHelper::convertConsistencyStringToEnum
                   (consistenceLevel))
        );
            
        // Prepare CQL
        boost::shared_future<cql::cql_future_result_t> future 
            = session->prepare(unboundInsert);
            
        // Wait for result
        future.wait();
        
        if(future.get().error.is_err()) {
                cerr << "Unable to prepare Insert CQL " << endl;
                cerr << "Error is " << future.get().error.message << endl;
        } else {
            insertCQLid = future.get().result->query_id();
            return true;
        }
    } catch(std::exception& e) {
        cerr << "Got exception while preparing cql query: " 
             << e.what() << endl;
    }
    
    return false;
}

string CassandraAdapter::getInsertCQL(string relation, string partition, 
                                      string node, string key, string value) {
    stringstream ss;
    ss << "INSERT INTO ";
    ss << relation;
    ss << " (partition, node, key, value)";
    ss << " VALUES (";
    
    string quote = "'";
    
    // Prepared statemnt? No quoting! 
    if((key.compare("?") == 0) 
      && (value.compare("?") == 0)
      && (node.compare("?") == 0)
      && (partition.compare("?") == 0)) {
      quote = "";
    }
    
    ss << quote << partition << quote << ", ";
    ss << quote << node << quote << ", ";
    ss << quote << key << quote << ", ";
    ss << quote << value << quote << ");";
    
    return ss.str();
}

CassandraResult* CassandraAdapter::getAllTables(string keyspace) {

    stringstream ss;
    ss << "SELECT columnfamily_name FROM ";
    ss << "system.schema_columnfamilies WHERE keyspace_name='";
    ss << keyspace;
    ss << "';";
    
  return readDataFromCassandra(ss.str(), cql::CQL_CONSISTENCY_ONE);
}


bool CassandraAdapter::getTupleTypeFromTable(string relation, string &result) {
    stringstream ss;
    ss << "SELECT value FROM ";
    ss << relation;
    ss << " WHERE partition = '";
    ss << CassandraAdapter::METADATA_TUPLETYPE;
    ss << "';";
    string query = ss.str();

#ifdef __DEBUG__
    cout << "Query: " << query << endl;
#endif

    // Execute query
    CassandraResult* cassandraResult = 
       readDataFromCassandra(query, cql::CQL_CONSISTENCY_ONE, false);
    
    if(cassandraResult == NULL) {
      return false;
    }
    
    if(! cassandraResult->hasNext() ) {
      return false;
    }
    
    cassandraResult -> getStringValue(result, 0);
    
    // Cleanup result object
    delete cassandraResult;
    cassandraResult = NULL;
    
    return true;
}


CassandraResult* CassandraAdapter::readTable(string relation,
        string consistenceLevel) {

    stringstream ss;
    ss << "SELECT key, value from ";
    ss << relation;
    ss << ";";
    string query = ss.str();
    
    return readDataFromCassandra(query, 
            CassandraHelper::convertConsistencyStringToEnum(consistenceLevel));
}


CassandraResult* CassandraAdapter::readTableRange(string relation,
        string consistenceLevel, string begin, string end) {
  
    stringstream ss;
    ss << "SELECT key, value from ";
    ss << relation;
    ss << " where token(partition) >= " << begin << " ";
    ss << "and token(partition) <= " << end;
    ss << ";";
    string query = ss.str();
    
    cout << "Query is: " << query << endl;
    
    return readDataFromCassandra(query, 
          CassandraHelper::convertConsistencyStringToEnum(consistenceLevel));
  
}

CassandraResult* CassandraAdapter::readTableCreatedByQuery(string relation,
        string consistenceLevel, int queryId) {
   
  vector<TokenRange> ranges;
  if (! getProcessedTokenRangesForQuery(ranges, queryId) ) {
    cerr << "Unable to fetch token ranges for query: " << queryId << endl;
    return NULL;
  }
  

  map<string, string> nodeNames;
  if(! getNodeData(nodeNames) ) {
     cerr << "Unable to fetch node data for query: " << queryId << endl;
     return NULL;
  }
  vector<string> queries;
  
  // Generate token range queries;
  for(vector<TokenRange>::iterator iter = ranges.begin(); 
      iter != ranges.end(); ++iter) {
    
      TokenRange range = *iter;
  
      stringstream ss;
      ss << "SELECT key, value from ";
      ss << relation << " where ";
  
      // Begin of range must be included
      if(range.getStart() == LLONG_MIN) {
        ss << "token(partition) >= " << range.getStart() << " ";
      } else {
        ss << "token(partition) > " << range.getStart() << " ";
      }
  
      // Get the name of the node
      string node = nodeNames[range.getIp()];
  
      ss << "and token(partition) <= " << range.getEnd() << " " ;
      ss << "and node = '" << range.getQueryUUID()  << "' ";
      ss << "ALLOW FILTERING;";
      queries.push_back(ss.str());
  }
    
  MultiCassandraResult* result = new MultiThreadedCassandraResult(queries, 
            this,
            CassandraHelper::convertConsistencyStringToEnum(consistenceLevel));
  
  return result;
}

bool CassandraAdapter::getLocalTokenRanges(
     vector<TokenRange> &localTokenRange, 
     vector <CassandraToken> &localTokens, 
     vector <CassandraToken> &peerTokens) {
  
     vector<TokenRange> allTokenRanges;
     
     bool result = 
        getAllTokenRanges(allTokenRanges, localTokens, peerTokens);
        
    // Do filtering of local intervals
    for(vector<TokenRange>::iterator iter = allTokenRanges.begin(); 
        iter != allTokenRanges.end(); ++iter) {
      
      TokenRange interval = *iter;
      if(interval.isLocalTokenRange()) {
        localTokenRange.push_back(interval);
      }
    }
        
    // Print debug Info
#ifdef __DEBUG__
    cout << "Peer ranges are: ";
    copy(peerTokens.begin(), peerTokens.end(), 
    std::ostream_iterator<CassandraToken>(cout, " "));
    cout << std::endl;
        
    cout << "Local ranges are: ";
    copy(localTokenRange.begin(), localTokenRange.end(), 
    std::ostream_iterator<TokenRange>(cout, " "));
    cout << std::endl;
#endif

    return result;
}

bool CassandraAdapter::getAllTokenRanges(
     vector<TokenRange> &allTokenRange) {
  
     vector <CassandraToken> localTokens;
     vector <CassandraToken> peerTokens;
     
     return getAllTokenRanges(allTokenRange, localTokens, peerTokens);
}

bool CassandraAdapter::getAllTokenRanges(
     vector<TokenRange> &allTokenRange, 
     vector <CassandraToken> &localTokens, 
     vector <CassandraToken> &peerTokens) {
  
    // Calculate local token ranges
    if(! getLocalTokens(localTokens)) {
      return false;
    }
    
    if(! getPeerTokens(peerTokens)) {
      return false;
    }
    
    sort(localTokens.begin(), localTokens.end());
    sort(peerTokens.begin(), peerTokens.end());
    
    // Merge and sort tokens
    vector<CassandraToken> allTokens;
    allTokens.reserve(localTokens.size() + peerTokens.size()); 
    allTokens.insert(allTokens.end(), localTokens.begin(), localTokens.end());
    allTokens.insert(allTokens.end(), peerTokens.begin(), peerTokens.end() );
    sort(allTokens.begin(), allTokens.end());

    // Last position in the vector
    int lastTokenPos = allTokens.size() - 1;
     
    // Is last token in the ring splitted?
    // So the two tokenranges (begin, LLONG_MAX] and (LLONG_MIN, end]
    if((allTokens.at(lastTokenPos)).getToken() != LLONG_MAX) {
      // Add end interval
      TokenRange interval(
        (allTokens.at(lastTokenPos)).getToken(), 
        LLONG_MAX, 
        (allTokens.at(lastTokenPos)).getIp());
      
      allTokenRange.push_back(interval);
    
      // Add start interval
      TokenRange interval2(
        LLONG_MIN, 
        (allTokens.at(0)).getToken(), 
        (allTokens.at(0)).getIp());
      
      allTokenRange.push_back(interval2);
    } else {
      // Add only the end interval
      TokenRange interval(
      (allTokens.at(lastTokenPos - 1)).getToken(), 
      LLONG_MAX, 
      (allTokens.at(lastTokenPos - 1)).getIp());
    }
    
    // Find all local token ranges between nodes and add them
    for(size_t i = 0; i < allTokens.size() - 1; ++i) {
      
      long long currentToken = (allTokens.at(i)).getToken();
      long long nextToken = (allTokens.at(i + 1)).getToken();        
      
      TokenRange interval(currentToken, 
                              nextToken, 
                              (allTokens.at(i)).getIp());
      
      allTokenRange.push_back(interval);
    }
    
    sort(allTokenRange.begin(), allTokenRange.end());
    
    return true;
}


CassandraResult* CassandraAdapter::readTableLocal(string relation,
        string consistenceLevel) {

    // Lokal tokens
    vector<TokenRange> localTokenRange;
    vector <CassandraToken> localTokens;
    vector <CassandraToken> peerTokens;
    
    getLocalTokenRanges(localTokenRange, localTokens, peerTokens);

    vector<string> queries;
    
    // Generate token range queries;
    for(vector<TokenRange>::iterator iter = localTokenRange.begin(); 
        iter != localTokenRange.end(); ++iter) {
      
       stringstream ss;
       ss << "SELECT key, value from ";
       ss << relation << " ";
       ss << "where ";
      
       TokenRange interval = *iter;
       // Include token begin
       if(interval.getStart() == LLONG_MIN) {
         ss << "token(partition) >= " << interval.getStart() << " ";
       } else {
         ss << "token(partition) > " << interval.getStart() << " ";
       }

       ss << "and token(partition) <= " << interval.getEnd();        
       ss << ";";
        
       queries.push_back(ss.str());
    }
    
    MultiCassandraResult* result = new MultiCassandraResult(queries, this, 
            CassandraHelper::convertConsistencyStringToEnum(consistenceLevel));
    
    return result;
}


CassandraResult* CassandraAdapter::readDataFromCassandra(string cql, 
         cql::cql_consistency_enum consistenceLevel, bool printError) {
      
    if(! isConnected() ) {
        cerr << "Cassandra session not ready" << endl;
        return NULL;
    }

    try {
        boost::shared_ptr<cql::cql_query_t> cqlStatement(
            new cql::cql_query_t(cql,
            consistenceLevel
        ));

        boost::shared_future<cql::cql_future_result_t> future
            = session->query(cqlStatement);

        if(future.get().error.is_err()) {
            if(printError) {
              cerr << "Unable to execute " << cqlStatement << endl;
              cerr << "Error is " << future.get().error.message << endl;
            }
        } else {
            return new SingleCassandraResult(future);
        }
    } catch(std::exception& e) {
        cerr << "Got exception while reading data: " << e.what() << endl;
    }

    return NULL;
}


bool CassandraAdapter::createTable(string tablename, string tupleType) {
  
    stringstream ss;
    ss << "CREATE TABLE IF NOT EXISTS ";
    ss << tablename;
    ss << " ( partition text, node text, key text,";
    ss << " value text, PRIMARY KEY(partition, node, key));";

    bool resultCreate = executeCQLSync(ss.str(), cql::CQL_CONSISTENCY_ALL);
    
    sleep(1);
    
    // Write tupletype
    if(resultCreate) {
      bool resultInsert = executeCQLSync(
            // Partition is Tupletype, Key is Tupletype, Node id is tupletype
            getInsertCQL(tablename, CassandraAdapter::METADATA_TUPLETYPE,
                         CassandraAdapter::METADATA_TUPLETYPE, 
                         CassandraAdapter::METADATA_TUPLETYPE, tupleType),
            cql::CQL_CONSISTENCY_ALL
      );
      
      if(resultInsert) {
        // New table is created and the 
        // tuple type is stored successfully
        return true; 
      }
    }
    
    return false;
}

bool CassandraAdapter::dropTable(string tablename) {
    stringstream ss;
    ss << "DROP TABLE IF EXISTS ";
    ss << tablename;
    ss << ";";

    return executeCQLSync(ss.str(), cql::CQL_CONSISTENCY_ALL);
}

void CassandraAdapter::waitForPendingFutures() {
    if(! pendingFutures.empty()) {
      cout << "Wait for pending futures: " << flush;
      
      for(vector < boost::shared_future<cql::cql_future_result_t> >
          ::iterator iter = pendingFutures.begin(); 
          iter != pendingFutures.end(); ++iter) {
        
          boost::shared_future<cql::cql_future_result_t> future = *iter;
      
          if(! future.is_ready() && ! future.has_exception() ) {
            // Assume future error after 100 seconds
            future.timed_wait(boost::posix_time::millisec(100));
            cout << "." << flush;
          }
      }
      
      cout << endl;
      
      // Force removal of finished futures
      removeFinishedFutures(true);
      
      cout << "All futures finished" << endl;
   }
}

void CassandraAdapter::disconnect() {
    if(isConnected()) {
        waitForPendingFutures();
        
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

bool CassandraAdapter::executeCQLSync
(string cql, cql::cql_consistency_enum consistency) {
      try {

        if(! isConnected() ) {
            cerr << "Cassandra session not ready" << endl;
            return false;
        }

        boost::shared_future<cql::cql_future_result_t> future
          = executeCQL(cql, consistency);
          
        return executeCQLFutureSync(future);

      } catch(std::exception& e) {
        cerr << "Got exception while executing cql: " << e.what() << endl;
      }

    return false;
}

bool CassandraAdapter::executeCQLASync
(string cql, cql::cql_consistency_enum consistency) {
      
      try {

        if(! isConnected() ) {
            cerr << "Cassandra session not ready" << endl;
            return false;
        }
        
        usleep(250 + (pendingFutures.size()));
 
        while(pendingFutures.size() > 50) {
          waitForPendingFutures();
          usleep(1000);
          removeFinishedFutures(true);
        }

        boost::shared_future<cql::cql_future_result_t> future
          = executeCQL(cql, consistency);
          
        pendingFutures.push_back(future);
        
        removeFinishedFutures();
        
        return true;
      
      } catch(std::exception& e) {
        cerr << "Got exception while executing cql: " << e.what() << endl;
      }

    return false;
}

void CassandraAdapter::removeFinishedFutures(bool force) {
  
    // The cleanup is not needed everytime
    if(pendingFutures.size() % 10 != 0 && force == false) {
      return;
    }
  
    // Are some futures finished?
    for(vector < boost::shared_future<cql::cql_future_result_t> >
      ::iterator iter = pendingFutures.begin(); 
      iter != pendingFutures.end(); 
      ) {
      
      boost::shared_future<cql::cql_future_result_t> future = *iter;
      
      // Remove finished futures
      if(future.is_ready()) {
        
        if(future.get().error.is_err()) {
          cerr << "Got error while executing future: " 
                << future.get().error.message << endl;
        }
        
        iter = pendingFutures.erase(iter);
      } else {
        ++iter;
      }
    }
}

bool CassandraAdapter::executeCQLFutureSync
(boost::shared_future<cql::cql_future_result_t> cqlFuture) {

    try {

        if(! isConnected() ) {
            cerr << "Cassandra session not ready" << endl;
            return false;
        }

        // Wait for execution
        cqlFuture.wait();

        if(cqlFuture.get().error.is_err()) {
            cerr << "Error is " << cqlFuture.get().error.message << endl;
            return false;
        }

        return true;

    } catch(std::exception& e) {
        cerr << "Got exception while executing cql future: " 
             << e.what() << endl;
    }

    return false;
}

boost::shared_future<cql::cql_future_result_t>
CassandraAdapter::executeCQL
(string cql, cql::cql_consistency_enum consistency) {


    boost::shared_ptr<cql::cql_query_t> cqlStatement(
        new cql::cql_query_t(cql, consistency));

    boost::shared_future<cql::cql_future_result_t> future
    = session->query(cqlStatement);

    return future;
}

bool CassandraAdapter::getTokensFromQuery
    (string query, vector <CassandraToken> &result, string peer) {    
      
    try {
       boost::shared_future< cql::cql_future_result_t > future = 
          executeCQL(query, cql::CQL_CONSISTENCY_ALL);
  
       future.wait();
    
       if(future.get().error.is_err()) {
         cerr << "Unable to fetch local token list" << endl;
         return false;
       }
    
       cql::cql_result_t& cqlResult = *(future.get().result);
    
       while(cqlResult.next()) {
         cql::cql_set_t* setResult = NULL;
         cqlResult.get_set(0, &setResult);
         
         // No peer argument was given, fetch from database
         string currentPeer = peer;
         
         if(currentPeer.empty()) {
           
           // Convert data into ip addresss
           boost::asio::ip::address peerData;
           cqlResult.get_inet("peer", peerData );
           currentPeer = peerData.to_string();
         }
      
         if(setResult != NULL) {
             for (size_t i = 0; i < setResult->size(); ++i) {
                string token;
                setResult->get_string(i, token);
                long long tokenLong = atol(token.c_str());
                result.push_back(CassandraToken(tokenLong, currentPeer));
             }
    
             delete setResult;
         }
       }
     } catch(std::exception& e) {
        cerr << "Got exception while executing cql: " << e.what() << endl;
        return false;
     }
   
     return true;
}

bool CassandraAdapter::getLocalTokens(vector <CassandraToken> &result) {
  return getTokensFromQuery(
    "SELECT tokens FROM system.local", result, string("127.0.0.1"));
}


bool CassandraAdapter::getPeerTokens(vector <CassandraToken> &result) {
  return getTokensFromQuery(
    "SELECT tokens,peer FROM system.peers", result, string(""));
}


bool CassandraAdapter::createMetatables() {
  
  vector<string> queries;
  
  queries.push_back(string(
    "CREATE TABLE IF NOT EXISTS system_queries (id INT, "
    "query TEXT, PRIMARY KEY(id));"
  ));
  
  queries.push_back(string(
    "CREATE TABLE IF NOT EXISTS system_state (ip TEXT, node TEXT, "
    "heartbeat BIGINT, lastquery INT, PRIMARY KEY(ip));"
  ));
  
  queries.push_back(string(
    "CREATE TABLE IF NOT EXISTS system_progress (queryid INT, ip TEXT, "
    "begintoken TEXT, endtoken TEXT, queryuuid TEXT, "
    "PRIMARY KEY(queryid, ip, begintoken));"
  ));
  
  queries.push_back(string(
    "CREATE TABLE IF NOT EXISTS system_tokenranges (begintoken TEXT, "
    "endtoken TEXT, ip TEXT, PRIMARY KEY(begintoken));"
  ));
  
  for(vector<string>::iterator iter = queries.begin(); 
      iter != queries.end(); ++iter) {
    
    string query = *iter;
 
    // Create queries table
    bool result = executeCQLSync(
      query, cql::CQL_CONSISTENCY_ALL
    );
  
    if(! result) {
      cout << "Unable to execute query: " << query << endl;
      return false;
    }  
    
  }
  
  return true;
}

bool CassandraAdapter::dropMetatables() {
  
  vector<string> queries;
  
  queries.push_back(string("TRUNCATE system_queries;"));
  queries.push_back(string("TRUNCATE system_state;"));
  queries.push_back(string("TRUNCATE system_progress;"));
  queries.push_back(string("TRUNCATE system_tokenranges"));
  
  for(vector<string>::iterator iter = queries.begin(); 
      iter != queries.end(); ++iter) {
    
    string query = *iter;
  
    // Create queries table
    bool result = executeCQLSync(
      query, cql::CQL_CONSISTENCY_ALL
    );
  
    if(! result) {
      cout << "Unable to execute query: " << query << endl;
      return false;
    }  
    
  }

  // Wait for drop request to be
  // executed on all cassandra nodes
  sleep(5); 
 
  return true;
}

CassandraResult* CassandraAdapter::getQueriesToExecute() {
  return readDataFromCassandra
            ("SELECT id, query from system_queries", cql::CQL_CONSISTENCY_ALL);
}

CassandraResult* CassandraAdapter::getGlobalQueryState() {
    return readDataFromCassandra
          ("SELECT ip, lastquery FROM system_state", cql::CQL_CONSISTENCY_ALL);
}

void CassandraAdapter::quoteCqlStatement(string &query) {
  size_t startPos = 0;
  string placeholder = "'";
  string value = "''";
    
  while((startPos = query.find(placeholder, startPos)) != std::string::npos) {
         query.replace(startPos, placeholder.length(), value);
         startPos += value.length();
  }
}

bool CassandraAdapter::copyTokenRangesToSystemtable (string localip) {
        vector<TokenRange> allIntervals;
        getAllTokenRanges(allIntervals);

        for(vector<TokenRange>::iterator iter = allIntervals.begin();
            iter != allIntervals.end(); ++iter) {
          
          TokenRange interval = *iter;
        
          // Build CQL query
          stringstream ss;
          ss << "INSERT INTO system_tokenranges(ip, begintoken, endtoken) ";
          ss << "values(";
        
          if(interval.isLocalTokenRange()) {
            ss << "'" << localip << "',";
          } else {
            ss << "'" << interval.getIp() << "',";
          }
          
          ss << "'" << interval.getStart() << "',",
          ss << "'" << interval.getEnd() << "'",
          ss << ");";

          // Update last executed command
          bool result = executeCQLSync(
            ss.str(),
            cql::CQL_CONSISTENCY_ALL
          );
          
          if(! result) {
            return false;
          }
        }
        
        return true;
}

bool CassandraAdapter::getTokenRangesFromSystemtable (
    vector<TokenRange> &result) {
  
      string query 
       = string("SELECT ip, begintoken, endtoken FROM system_tokenranges"); 
      
      return getTokenrangesFromQuery(result, query);
}

bool CassandraAdapter::getProcessedTokenRangesForQuery (
    vector<TokenRange> &result, int queryId) {
  
      stringstream ss;
      ss << "SELECT ip, begintoken, endtoken, queryuuid FROM system_progress ";
      ss << " WHERE queryid = " << queryId;
      
      return getTokenrangesFromQuery(result, ss.str());
}

bool CassandraAdapter::getTokenrangesFromQuery (
    vector<TokenRange> &result, string query) {

    try {   
       boost::shared_future< cql::cql_future_result_t > future = 
          executeCQL(query, cql::CQL_CONSISTENCY_ALL);
  
       future.wait();
    
       if(future.get().error.is_err()) {
         cerr << "Unable to fetch processed tokens for query" << endl;
         return false;
       }
       
       // Does the query return queryuuids?
       bool containsQueryuuid = query.find("queryuuid") != string::npos;
    
       cql::cql_result_t& cqlResult = *(future.get().result);
    
       while(cqlResult.next()) {
         string ip;
         string beginToken;
         string endToken;
         string queryuuid = "";
         
         cqlResult.get_string(0, ip);
         cqlResult.get_string(1, beginToken);
         cqlResult.get_string(2, endToken);
        
         long long beginLong = atol(beginToken.c_str());
         long long endLong = atol(endToken.c_str());
         
         if(containsQueryuuid) {
           cqlResult.get_string(3, queryuuid);
         }
         
         result.push_back(TokenRange(beginLong, endLong, ip, queryuuid));
       }
     } catch(std::exception& e) {
        cerr << "Got exception while executing cql: " << e.what() << endl;
        return false;
     }
     
     // Sort result
     sort(result.begin(), result.end());
   
     return true;
}

bool CassandraAdapter::getHeartbeatData(map<string, time_t> &result) {
    try {   

       boost::shared_future< cql::cql_future_result_t > future = 
          executeCQL(string("SELECT ip, heartbeat FROM system_state"), 
                     cql::CQL_CONSISTENCY_ONE);
  
       future.wait();
    
       if(future.get().error.is_err()) {
         cerr << "Unable to fetch heartbeat data" << endl;
         return false;
       }
    
       cql::cql_result_t& cqlResult = *(future.get().result);
    
       while(cqlResult.next()) {
         string ip;
         boost::int64_t res;
         
         cqlResult.get_string(0, ip);
         cqlResult.get_bigint(1, res);
         
         result.insert(std::pair<string,time_t>(ip,(time_t) res));
       }
     } catch(std::exception& e) {
        cerr << "Got exception while executing cql: " << e.what() << endl;
        return false;
     }
   
     return true;
}

bool CassandraAdapter::getNodeData(map<string, string> &result) {
    try {   

       boost::shared_future< cql::cql_future_result_t > future = 
          executeCQL(string("SELECT ip, node FROM system_state"), 
                     cql::CQL_CONSISTENCY_ALL);
  
       future.wait();
    
       if(future.get().error.is_err()) {
         cerr << "Unable to fetch heartbeat data" << endl;
         return false;
       }
    
       cql::cql_result_t& cqlResult = *(future.get().result);
    
       while(cqlResult.next()) {
         string ip;
         string node;
         
         cqlResult.get_string(0, ip);
         cqlResult.get_string(1, node);
         
         result.insert(std::pair<string,string>(ip, node));
       }
     } catch(std::exception& e) {
        cerr << "Got exception while executing cql: " << e.what() << endl;
        return false;
     }
   
     return true;
}

}

