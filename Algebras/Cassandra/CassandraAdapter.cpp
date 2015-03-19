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
#include <sstream>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <uv.h>

#include <cassert>
#include <cassandra.h>

#include "CassandraHelper.h"
#include "CassandraAdapter.h"
#include "CassandraResult.h"

// Activate debug messages
//#define __DEBUG__

#define MAX_PENDING_FUTURES 30

using namespace std;

//namespace to avoid name conflicts
namespace cassandra {

// Init static variables
const string CassandraAdapter::METADATA_TUPLETYPE = "_TUPLETYPE";

CassError CassandraAdapter::connect_session(CassSession* session, 
    const CassCluster* cluster) {
   
   CassError rc = CASS_OK;
   CassFuture* future = cass_session_connect(session, cluster);
   
   cass_future_wait(future);
   
   rc = cass_future_error_code(future);

   if (rc != CASS_OK) {
      errorFlag = true;
      CassandraHelper::print_error(future);
   } 
   
   cass_future_free(future);
   
   return rc;
}

void CassandraAdapter::connect(bool singleNodeLoadBalancing) {
      
     CassError rc = CASS_OK;
     stringstream ss;

     cluster = cass_cluster_new();
     session = cass_session_new();

     cass_cluster_set_contact_points(cluster, contactpoint.c_str());        
     
     // Switch to single node policy
     if(singleNodeLoadBalancing) {
         cass_cluster_set_load_balance_single(cluster, contactpoint.c_str()); 
     }
      
     rc = connect_session(session, cluster);

     if (rc != CASS_OK) {
        disconnect();
     } else {
        ss << "USE ";
        ss << keyspace;

        // Switch keyspace
        executeCQLSync(ss.str(), CASS_CONSISTENCY_ALL);

        cout << "Cassandra: Connection successfully established" << endl;
        cout << "You are connected to host " << contactpoint
                 << " keyspace " << keyspace << endl;
        cout << "SingleNodeLoadBalancing: " << singleNodeLoadBalancing << endl;
     }
}

bool CassandraAdapter::isConnected() {
      if(session) {
        return true;
      } else {
        return false;
      }
}

bool CassandraAdapter::writeDataToCassandra(string relation, string partition, 
        string node, string key, string value, string consistenceLevel, 
        bool sync) {

    bool result = false;
    
    // Convert consistence level
    CassConsistency consistency
       = CassandraHelper::convertConsistencyStringToEnum(consistenceLevel);
       
    // Write Data and wait for result
    if(sync) {
      result = executeCQLSync(
          getInsertCQL(relation, partition, node, key, value),
          consistency
      );
      
    } else {
      result = executeCQLASync(
          getInsertCQL(relation, partition, node, key, value),
          consistency
      );
    }
    
    return result;
}

bool CassandraAdapter::writeDataToCassandraPrepared(
        const CassPrepared* preparedStatement, 
        string partition, string node, string key, string value, 
        string consistenceLevel, bool sync) {
   
    if(! isConnected() ) {
       cerr << "Cassandra session not ready" << endl;
       return false;
    }
    
    if(preparedStatement == NULL) {
       cerr << "Prepared statement is null" << endl;
       return false;
    }
   
    CassStatement* statement = cass_prepared_bind(preparedStatement);
    cass_statement_set_consistency(statement, 
         CassandraHelper::convertConsistencyStringToEnum(consistenceLevel));
    
    // Bind parameter
    cass_statement_bind_string(statement, 0, 
         cass_string_init(partition.c_str()));
    cass_statement_bind_string(statement, 1, 
         cass_string_init(node.c_str()));
    cass_statement_bind_string(statement, 2, 
         cass_string_init(key.c_str()));
    cass_statement_bind_string(statement, 3, 
         cass_string_init(value.c_str()));
 
    // Build future and execute
    CassFuture* future = cass_session_execute(session, statement);
 
    // Execution sync or async?
    if(sync) {
       executeCQLFutureSync(future);
    } else {
      pendingFutures.push_back(future);
      waitForPendingFuturesIfNeeded();
    }

    cass_statement_free(statement);  
    return true;  
}

void CassandraAdapter::freePreparedStatement(
     const CassPrepared* preparedStatement) {
        
        if(preparedStatement != NULL) {
                cass_prepared_free(preparedStatement);
                preparedStatement = NULL;
        }
}

const CassPrepared* CassandraAdapter::prepareCQLInsert(string relation) {
        
        CassError rc = CASS_OK;
        const CassPrepared* result = NULL;
       
        if(! isConnected() ) {
           cerr << "Cassandra session not ready" << endl;
           return NULL;
        }
 
        string cqlQuery = getInsertCQL(relation, "?", "?", "?", "?");
        cout << "Preparing insert query: "  << cqlQuery << endl;

        CassString query = cass_string_init(cqlQuery.c_str());

        CassFuture* future = cass_session_prepare(session, query);
        cass_future_wait(future);
        rc = cass_future_error_code(future);

        if(rc != CASS_OK) {
           errorFlag = true;
           CassandraHelper::print_error(future);
        } else {
           result = cass_future_get_prepared(future);
        }
    
        cass_future_free(future);

        return result;
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
    
  return readDataFromCassandra(ss.str(), CASS_CONSISTENCY_ONE);
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
       readDataFromCassandra(query, CASS_CONSISTENCY_ONE, false);
    
    if(cassandraResult == NULL) {
      return false;
    }
    
    if(! cassandraResult->hasNext() ) {
      delete cassandraResult;
      cassandraResult = NULL;
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
     
    // Is last token-range splitted?
    //
    // If so, the two token-ranges are 
    // (begin, LLONG_MAX] and [LLONG_MIN, end]
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
         CassConsistency consistenceLevel, bool printError) {

     if(! isConnected() ) {
        cerr << "Cassandra session not ready" << endl;
        return NULL;
     }

     CassStatement* statement = cass_statement_new(
              cass_string_init(cql.c_str()), 0);

     cass_statement_set_consistency(statement, consistenceLevel);

     return new SingleCassandraResult(session, statement, printError);
}


bool CassandraAdapter::createTable(string tablename, string tupleType) {
  
    stringstream ss;
    ss << "CREATE TABLE IF NOT EXISTS ";
    ss << tablename;
    ss << " ( partition text, node text, key text,";
    ss << " value text, PRIMARY KEY(partition, node, key));";

    bool resultCreate = executeCQLSync(ss.str(), CASS_CONSISTENCY_ALL);
    
    // Write tupletype
    if(resultCreate) {
      bool resultInsert = executeCQLSync(
            // Partition is Tupletype, Key is Tupletype, Node id is tupletype
            getInsertCQL(tablename, CassandraAdapter::METADATA_TUPLETYPE,
                         CassandraAdapter::METADATA_TUPLETYPE, 
                         CassandraAdapter::METADATA_TUPLETYPE, tupleType),
            CASS_CONSISTENCY_ALL
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

    return executeCQLSync(ss.str(), CASS_CONSISTENCY_ALL);
}

void CassandraAdapter::waitForPendingFuturesIfNeeded() {
   while(pendingFutures.size() > MAX_PENDING_FUTURES) {
        waitForPendingFutures();
   }
}

void CassandraAdapter::waitForPendingFutures() {
    if(pendingFutures.empty()) {
       return;
    }
      
    for(vector<CassFuture*>::iterator iter = pendingFutures.begin(); 
          iter != pendingFutures.end(); ++iter) {
        
          CassFuture* future = *iter;
          cass_future_wait(future);
     }
      
     // Force removal of finished futures
     removeFinishedFutures(true);
}

void CassandraAdapter::disconnect() {
    if( ! isConnected()) {
       return;
    }
    
    cout << "Disconnecting from cassandra" << endl;
    
    waitForPendingFutures();

    // Close session and cluster
    CassFuture* close_future = cass_session_close(session);
    cass_future_wait(close_future);
    cass_future_free(close_future);
    cass_cluster_free(cluster);
    cass_session_free(session);

    cluster = NULL;
    session = NULL;
}

bool CassandraAdapter::executeCQLSync
   (string cql, CassConsistency consistency) {
        
   if(! isConnected() ) {
      cerr << "Cassandra session not ready" << endl;
      return false;
   }

   CassFuture* future = executeCQL(cql, consistency);
          
   return executeCQLFutureSync(future);
}

bool CassandraAdapter::executeCQLASync
    (string cql, CassConsistency consistency) {
      
     if(! isConnected() ) {
        cerr << "Cassandra session not ready" << endl;
        return false;
     }
        
     CassFuture* future = executeCQL(cql, consistency);
     pendingFutures.push_back(future);
     waitForPendingFuturesIfNeeded();

     return true;
}

void CassandraAdapter::removeFinishedFutures(bool force) {
  
    // The cleanup is not needed everytime
    if(pendingFutures.size() % 10 != 0 && force == false) {
      return;
    }
  
    // Are some futures finished?
    for(vector<CassFuture*>::iterator iter = pendingFutures.begin(); 
      iter != pendingFutures.end(); ) {
      
      CassFuture* future = *iter;
      
      // Remove finished futures
      if(cass_future_ready(future) == cass_true) {
        
        if(cass_future_error_code(future) != CASS_OK) {
           errorFlag = true;
           CassandraHelper::print_error(future); 
        }
        
        cass_future_free(future);
 
        iter = pendingFutures.erase(iter);
      } else {
        ++iter;
      }
    }
}

bool CassandraAdapter::executeCQLFutureSync(CassFuture* future) {

    bool result = true;

     if(! isConnected() ) {
         cerr << "Cassandra session not ready" << endl;
         return false;
     }

     // Wait for execution
     cass_future_wait(future);

     if(cass_future_error_code(future) != CASS_OK) {
         errorFlag = true;
         CassandraHelper::print_error(future); 
         result = false;
     }

     if(future != NULL) {
           cass_future_free(future);
           future = NULL;
     } 
      
     return result;
}

CassFuture* CassandraAdapter::executeCQL
   (string cql, CassConsistency consistency) {

    CassStatement* statement = 
          cass_statement_new(cass_string_init(cql.c_str()), 0);
    cass_statement_set_consistency(statement, consistency);
    CassFuture* future = cass_session_execute(session, statement);
   
    cass_statement_free(statement);

    return future;
}

bool CassandraAdapter::getTokensFromQuery
    (string query, vector <CassandraToken> &result, string peer) {    
     
     CassError rc = CASS_OK;  
     CassFuture* future = executeCQL(query, CASS_CONSISTENCY_QUORUM);
  
     cass_future_wait(future);
     rc = cass_future_error_code(future);

     if (rc != CASS_OK) {
             errorFlag = true;
             CassandraHelper::print_error(future);
             return false;
     }   
     
     const CassResult* cas_result = cass_future_get_result(future);
     CassIterator* iterator = cass_iterator_from_result(cas_result);
    
      while(cass_iterator_next(iterator)) {
         const CassRow* row = cass_iterator_get_row(iterator);
         
         // No peer argument was given, fetch from database
         string currentPeer = peer;
         
         if(currentPeer.empty()) {
           // Convert data into ip addresss
           CassInet peerData;
           cass_value_get_inet(cass_row_get_column_by_name(row, "peer"), 
              &peerData);
           char buf[INET_ADDRSTRLEN];
           uv_inet_ntop(AF_INET, peerData.address, buf, sizeof(buf));
           currentPeer = buf; 
         }
         
         const CassValue* value = cass_row_get_column(row, 0); 
         CassIterator* items_iterator = cass_iterator_from_collection(value);
 
         while(cass_iterator_next(items_iterator)) {
              CassString item_string;
              cass_value_get_string(cass_iterator_get_value(items_iterator), 
                  &item_string);
               
               char value_buffer[128];   
               memcpy(value_buffer, item_string.data, item_string.length);
               value_buffer[item_string.length] = '\0';

              long long tokenLong = atol(value_buffer);
              result.push_back(CassandraToken(tokenLong, currentPeer));
         }
         cass_iterator_free(items_iterator);
       }
       
      if(cas_result != NULL) {
            cass_result_free(cas_result);
            cas_result = NULL;    
       }   

       if(iterator != NULL) {
           cass_iterator_free(iterator);
           iterator = NULL;
       }   

       if(future != NULL) {
           cass_future_free(future);
           future = NULL;
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
      query, CASS_CONSISTENCY_ALL 
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
  vector<TokenRange> ranges;
  int highestQueryId = 0;
  size_t clearTry = 0;

  string getSystemProgress = string(
       "SELECT ip, begintoken, endtoken, queryuuid FROM system_progress");
 
  queries.push_back(string("TRUNCATE system_queries;"));
  queries.push_back(string("TRUNCATE system_state;"));
  queries.push_back(string("TRUNCATE system_progress;"));
  queries.push_back(string("TRUNCATE system_tokenranges"));
  
  do {
     if(clearTry > 20) {
          cerr << "Error: system_progress is not empty after 100 seconds" 
               << endl;
          return false;
     }
  
     for(vector<string>::iterator iter = queries.begin(); 
        iter != queries.end(); ++iter) {
    
       string query = *iter;

       // Create queries table
       bool result = executeCQLSync(
         query, CASS_CONSISTENCY_ALL
       );
  
       if(! result) {
         cout << "Unable to execute query: " << query << endl;
         return false;
       }  
     }

     clearTry++;
     ranges.clear();
     
     sleep(5);
     getTokenrangesFromQuery(ranges, getSystemProgress);
          
     CassandraResult* result = getGlobalQueryState();
     highestQueryId = 0;
     
      // Determine the highest executed query
      if(result != NULL) {
         while(result -> hasNext()) {
           int lastExecutedQuery = result -> getIntValue(1);
           if(lastExecutedQuery > highestQueryId) {
             highestQueryId = lastExecutedQuery;
           }
         }
            
       delete result;
     }
     
  } while(! ranges.empty() || highestQueryId > 0 || clearTry < 2) ;
 
  return true;
}

bool querySortFunction (CassandraQuery a, CassandraQuery b) { 
   return (a.getQueryId() > b.getQueryId()); 
}

void CassandraAdapter::getQueriesToExecute(vector<CassandraQuery> &result) {
  CassandraResult* queries = readDataFromCassandra
            ("SELECT id, query from system_queries", CASS_CONSISTENCY_ALL);
  
  while(queries != NULL && queries->hasNext()) {
     
     // Error while fetching data from cassandra
     if(queries == NULL) {
       return;
     }
     
     size_t id = queries->getIntValue(0);
     string myResult;
     queries -> getStringValue(myResult, 1);
     result.push_back(CassandraQuery(id, myResult));
  }
  
  if(queries != NULL) {
     delete queries;
     queries = NULL;
  }
  
  sort(result.begin(), result.end(), querySortFunction);
}

CassandraResult* CassandraAdapter::getGlobalQueryState(
    CassConsistency consistency, 
    bool printError) {
       
    string cql("SELECT ip, lastquery FROM system_state");
    
    return readDataFromCassandra(cql, consistency, printError);
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
            CASS_CONSISTENCY_ALL
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
    vector<TokenRange> &result, int queryId, 
    CassConsistency consistency, bool printError) {
  
      stringstream ss;
      ss << "SELECT ip, begintoken, endtoken, queryuuid FROM system_progress ";
      ss << " WHERE queryid = " << queryId;
      
      return getTokenrangesFromQuery(result, ss.str(), consistency);
}

bool CassandraAdapter::getTokenrangesFromQuery (
    vector<TokenRange> &result, string query, 
    CassConsistency consistency, bool printError) {
  
  CassError rc = CASS_OK; 
  CassFuture* future = executeCQL(query, consistency);
 
  cass_future_wait(future);
  rc = cass_future_error_code(future);

  if (rc != CASS_OK) {
     if(printError) {
        errorFlag = true;
        CassandraHelper::print_error(future);
     }
     
     return false;
  } 
       
   // Does the query return queryuuids?
   bool containsQueryuuid = query.find("queryuuid") != string::npos;
  
   const CassResult* cas_result = cass_future_get_result(future);
   CassIterator* iterator = cass_iterator_from_result(cas_result);
  
   while(cass_iterator_next(iterator)) {
         
         const CassRow* row = cass_iterator_get_row(iterator);
         CassString result_string;

         string ip;
         string beginToken;
         string endToken;
         string queryuuid = "";
         
         cass_value_get_string(cass_row_get_column(row, 0), &result_string);
         ip.append(result_string.data);

         cass_value_get_string(cass_row_get_column(row, 1), &result_string);
         beginToken.append(result_string.data);
          
         cass_value_get_string(cass_row_get_column(row, 2), &result_string);
         endToken.append(result_string.data);
 
         long long beginLong = atol(beginToken.c_str());
         long long endLong = atol(endToken.c_str());
         
         if(containsQueryuuid) {
           cass_value_get_string(cass_row_get_column(row, 3), &result_string);
           queryuuid.append(result_string.data);
         }
         
         result.push_back(TokenRange(beginLong, endLong, ip, queryuuid));
    }
    
    if(cas_result != NULL) {
       cass_result_free(cas_result);
       cas_result = NULL;    
    }

    if(iterator != NULL) {
       cass_iterator_free(iterator);
       iterator = NULL; 
    }

    if(future != NULL) {
       cass_future_free(future);
       future = NULL;
    }

     // Sort result
     sort(result.begin(), result.end());
     
     return true;
}

bool CassandraAdapter::getHeartbeatData(map<string, time_t> &result) {
 
  CassandraResult *cas_result = readDataFromCassandra(
            string("SELECT ip, heartbeat FROM system_state"), 
            CASS_CONSISTENCY_QUORUM);
  
  while(cas_result->hasNext()) {
         string ip;
         long long res;

         res = cas_result->getBigIntValue(1);
         cas_result->getStringValue(ip, 0); 
         result.insert(std::pair<string,time_t>(ip,(time_t) res));
   }

   delete cas_result;

   return true;
}

bool CassandraAdapter::getNodeData(map<string, string> &result) {

 CassandraResult *cas_result = readDataFromCassandra(
            string("SELECT ip, node FROM system_state"), 
            CASS_CONSISTENCY_QUORUM);
  
  while(cas_result->hasNext()) {
         string ip;
         string node;
         
         cas_result->getStringValue(ip, 0); 
         cas_result->getStringValue(node, 1); 
         
         result.insert(std::pair<string,string>(ip, node));
   }

   delete cas_result;

   return true;
}

}
