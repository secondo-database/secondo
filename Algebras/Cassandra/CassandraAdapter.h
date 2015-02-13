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

 This file contains an abstraction layer for the cassandra algebra. The 
 cassandra algebra does not communicate with cassandra directly. The algebra
 will only use the functions defined here. 
 
 In addition, this file contains some helper classes: E.g. for representing
 token-ranges or parsing consistency levels.
 
 1 Includes and defines

*/

#ifndef _CASSANDRA_H
#define _CASSANDRA_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <iostream>
#include <map>
#include <stdlib.h>
#include <cassert>
#include <cassandra.h>
#include <algorithm>
#include <limits.h>
#include <vector>

#include "CassandraHelper.h"
#include "CassandraResult.h"

using namespace std;

//namespace to avoid name conflicts
namespace cassandra {

// Prototype classes
class CassandraResult;
class CassandraToken;

/*
2.4 Helper Class Token Interval

*/

class TokenRange {
  
public:
  
/*
2.4.1 Construct a new token interval

*/
  TokenRange(long long myStart, long long myEnd, string myIp) :
    start(myStart), end(myEnd), ip(myIp) {}
    
  TokenRange(long long myStart, long long myEnd, 
             string myIp, string myQueryuuid) :
    start(myStart), end(myEnd), ip(myIp), queryuuid(myQueryuuid) {}
  
  
/*
2.4.2 Get interval start

*/  
  long long getStart() const {
    return start;
  }

/*
2.4.3 Get interval end

*/
  long long getEnd() const {
    return end;
  }

/*
2.4.4 Get interval end

*/
  string getIp() const {
    return ip;
  }
  
/*
2.4.5 Get interval end

*/
  string getQueryUUID() const {
    return queryuuid;
  }  

/*
2.4.6 Is this a local interval?

*/
  bool isLocalTokenRange(string myIp = "127.0.0.1") const {
    return (ip.compare(myIp) == 0);
  }

/*
2.4.7 Get interval end

*/  
  long long getSize() {
      if(start >= 0 && end >= 0) {
          return end - start;
      }
      
      if(start <= 0 && end <= 0) {
          return llabs(start - end);
      }
      
      return llabs(start) + llabs(end);
  }

/*
2.4.8 Operator <

*/  
  bool operator<( const TokenRange& val ) const { 
        return start < val.getStart(); 
  }

/*
2.4.9 Operator >

*/  
  bool operator>( const TokenRange& val ) const { 
        return start > val.getStart(); 
  }
    
/*
2.4.10 Operator ==

*/    
 inline bool operator== (const TokenRange &interval) {
   
   if((interval.getStart() == getStart()) &&
     (interval.getEnd() == getEnd())) {
     return true;
   } 
   
   return false;
 }
  
private:
  // Interval start
  long long start;
  
  // Interval end
  long long end;
  
  // Ip assigned to this interval
  string ip;
  
  // QueryUUID that processed this token range
  string queryuuid;
};

/*
2.4.4 Implementation for "toString"

*/
inline std::ostream& operator<<(std::ostream &strm, 
                         const cassandra::TokenRange &tokenInterval) {
  
  return strm << "TokenRange[" << tokenInterval.getStart()
              << ";" << tokenInterval.getEnd() << "; " 
              << "ip=" << tokenInterval.getIp() << "]";
}

/*
2.4 CassandraQuery wrapper class

*/
class CassandraQuery {
public:
   CassandraQuery(size_t myQueryId, string &myQuery) 
      : queryId(myQueryId), query(myQuery) 
   {}

   size_t getQueryId() {
      return queryId;
   }
   
   string getQuery() {
      return query;
   }

private:
   size_t queryId;
   string query;
};

/*
2.3 Adapter for cassandra

*/
class CassandraAdapter {

public:

/*
2.3.0 Static variables


*/
    static const string METADATA_TUPLETYPE;


/*
2.3.1 Constructor

1. Parameter the contactpoint of the cassadra cluster
2. Parameter the keyspace to use (e.g. secondo)

*/
    CassandraAdapter(string myContactpoint, string myKeyspace) 
      : contactpoint(myContactpoint), keyspace(myKeyspace),
        insertCQLid(NULL) {
          
    }
    
    virtual ~CassandraAdapter() {
         disconnect();
    }
    

/*
2.3.2 Open a connection to the cassandra cluster. If the 1st parameter
   is set to ~true~, the load balancing feature of the driver will be
   disabled. Only connections to the specified cassandra node will 
   be established. If set to ~false~, the loadbalancing feature
   of the driver will be enabled.

*/
    void connect(bool singleNodeLoadBalancing);

/*
2.3.3 Write a tuple to the cluster

1. Parameter is the name of the relation (e.g. plz)
2. Parameter is the partiton value of the tuple
3. Parameter is the name of the node
4. Parameter is the unique key of the data
5. Parameter is the data
6. Parameter is the consistence level used for writing
7. Parameter specifies to use synchronus or asynchronus writes

*/
    void writeDataToCassandra(string relation, 
        string partition, string node, string key, string value, 
        string consistenceLevel, bool sync);
  
/* 
2.3.4 Same as writeDataToCassandra, but with prepared statements

*/
    void writeDataToCassandraPrepared(string relation, 
        string partition, string node, string key, string value, 
        string consistenceLevel, bool sync
                             );

/*
2.3.4 Inform the CassandraAdapter about the fact that the last tuple for 
   the relaion is written. Batch writes can be commited and prepared 
   statements can be freed. 

*/
    void relationCompleteCallback(string relation);

/*
2.3.5 Fetch a full table from cassandra

1. Parameter is the relation to read
2. Parameter is the consistence level used for writing

*/
    CassandraResult* readTable(string relation,
        string consistenceLevel);

/*
2.3.5 Fetch partial table from cassandra. Read only
      the tuples stored on the local node

1. Parameter is the relation to read
2. Parameter is the consistence level used for writing

*/
    CassandraResult* readTableLocal(string relation,
        string consistenceLevel);

/*
2.3.5 Fetch partial table from cassandra. Read only
      the tuples inside the given token interval

1. Parameter is the relation to read
2. Parameter is the consistence level used for writing
3. Parameter is the begin token range
4. Parameter is the end token range

*/
    CassandraResult* readTableRange(string relation,
        string consistenceLevel, string begin, string end);


/*
2.3.6 Fetch partial table from cassandra. The table is
      produced by the query ~queryId~. Fetch only data
      from fault free nodes.

1. Parameter is the relation to read
2. Parameter is the consistence level used for writing
3. Parameter is the queryId

*/
    CassandraResult* readTableCreatedByQuery(string relation,
        string consistenceLevel, int queryId);

    
/*
2.3.6 Create a new relation in cassandra and write some 
      metadata (e.g. tupletype) for the table. 
      
 Returns true if the relation could be created, false otherwise.

1. Parameter is the name of the relation
2. Parameter is the tuple type of the stored tuples 
   in nested list representation

*/
    bool createTable(string tablename, string tupletype);

/*
2.3.7 Remove a relation from the cassandra cluster. Returns true if
      the relation could be successfully removed, false otherwise. 

1. Parameter is the name of the relation

*/
    bool dropTable(string tablename);
    
/*
2.3.8 Disconnect from cassandra cluster. This method waits for
      all pending requests to finish before the connection is 
      closed. So the call can take some time.
      
*/
    void disconnect();
  
/*
2.3.9 Is the connection to the cluster open? Return true if the 
      connection is open. False otherweise.
      
*/    
    bool isConnected();

/*
2.3.10 Get the token list of the current node
1. Parameter is the token result list

*/
  bool getLocalTokens(vector <cassandra::CassandraToken> &result);
  
/*
2.3.11 Get the token list of all peer nodes
1. Parameter is the token result list

*/
  bool getPeerTokens(vector <cassandra::CassandraToken> &result);
  
/*
2.3.12 Get all tables from the keyspace specified 
       by the 1th paramter

*/
  CassandraResult* getAllTables(string keyspace);

/*
2.3.12 Get the TupleType (in nested list format) from
       the table specified by the 1th paramter. The 
       result will be stored in the 2nd parameter. 
       
Returns true, if the 2nd parameter contains
a valid result. False otherweise

*/
   bool getTupleTypeFromTable(string table, string &result);

/*
2.3.12 Execute the cql statement and return the result

*/    
  CassandraResult* readDataFromCassandra(string cql, 
         CassConsistency consistenceLevel,
         bool printError = true);

/*
2.3.12 Get a list with all token ranges

1st parameter is a vector with the token ranges
2nd parameter is a vector with the tokens of the local system (optional)
3rd parameter is a vector with the tokens of the other systems (optional)

*/
   bool getAllTokenRanges(vector<TokenRange> &allTokenRange);

   bool getAllTokenRanges(vector<TokenRange> &allTokenRange, 
     vector <CassandraToken> &localTokens, 
     vector <CassandraToken> &peerTokens);
 
/*
2.3.12 Get a list with local token ranges

1st parameter is a vector with the local token ranges
2nd parameter is a vector with the tokens of the local system
3rd parameter is a vector with the tokens of the other systems

*/

   bool getLocalTokenRanges(vector<TokenRange> &localTokenRange, 
     vector <CassandraToken> &localTokens, 
     vector <CassandraToken> &peerTokens);
   
/*
2.3.12 Execute the cql statement with a given consistence level synchronus

*/    
  bool executeCQLSync(string cql, CassConsistency consistency);

/*
2.3.13 Execute the cql statement with a given consistence level asynchronus

*/    
  bool executeCQLASync(string cql, CassConsistency consistency);

/*
2.3.14 Create meta tables for queries and status information

*/    
  bool createMetatables();

/*
2.3.15 Drop metatables

*/    
  bool dropMetatables();
  
  
/*
2.3.16 Get a cassandraResult with queries to execute

*/    
  void getQueriesToExecute(vector<CassandraQuery> &result);

/*
2.3.17 Quote CQL Statement

Replace all single quotes with double quotes

*/      
  void quoteCqlStatement(string &query);  

/*
2.3.18 Get the global query state

*/    
  CassandraResult* getGlobalQueryState();

/*
2.3.19 Get processed token ranges for query

*/
  bool getProcessedTokenRangesForQuery (
      vector<TokenRange> &result, int queryId);

/*
2.3.20 Get tokenranges from query

1. parameter result
2. parameter the query

*/  
  bool getTokenrangesFromQuery (
    vector<TokenRange> &result, string query);

/*
2.3.21 Get tokenranges from system table

1. parameter result

*/  
  bool getTokenRangesFromSystemtable (
    vector<TokenRange> &result);
  
/*
2.3.22 Get heartbeat data

Result is a map:

IP to Lastheatbeat message

*/
  bool getHeartbeatData(map<string, time_t> &result);

/*
2.3.23 Get node data

Result is a map:

IP to Noodename

*/
  bool getNodeData(map<string, string> &result);
  
/*
2.3.24 Copy tokenranges to systemtable

*/
  bool copyTokenRangesToSystemtable(string localip);
          
  
/*
2.3.25 Wait for pending futures

*/
  void waitForPendingFutures();
protected:

/*
2.3.26 Execute the given cql future and check for errors. Returns
       true if the future is executed successfully. False otherwise.
       
*/    
  bool executeCQLFutureSync(CassFuture* cqlFuture);

/*
2.3.27 Execute the given cql. Returns a future containing the
       Query result.
       
*/    
  CassFuture* 
     executeCQL(string cql, CassConsistency consistency);

/*
2.3.28 Returns a CQL statement for inserting a new row. The
       first parameter is the relation for this request. The 
       second parameter is the parition key, the third parameter 
       is the node id. The fourth parameter is the key of the tuple.
       The fith parameter is the value of the tuple. 
       
*/    
  string getInsertCQL(string relation, string partition, 
                      string node, string key, string value);

/*
2.3.29 Create a pepared statement for inserting data into the 
       relation spoecified in the first parameter.
       
*/    
  bool prepareCQLInsert(string relation);

/*
2.3.30 Iterate over all pending futures (e.g. writes), report
       errors and remove finished futures from the future list.
       Normally a cleanup is started only if the condition
       list.length % 100 = 0 is true. 
       With the parameter force, you can override this policy.
       
*/    
  void removeFinishedFutures(bool force = false);

/*
2.3.31 Execute a CQL query and extract result tokens

1. Parameter is the query to execute
2. Parameter is the token result list
3. Parameter is the default peer

*/
  bool getTokensFromQuery (string query, vector <CassandraToken> &result, 
                           string peer);

/*
2.3.32 Connect to the cassandra cluster 

1. Parameter is the session instance
2. Parameter is the cassandra cluster 

*/
  CassError connect_session(CassSession* session, 
                                 const CassCluster* cluster);   

 
private:

  // Our cassandra contact point
  string contactpoint;            
  
  // Our keyspace
  string keyspace;
  
  // Our relation
  string relation;                
  
  // Cassandra cluster
  CassCluster* cluster;
  
  // Cassandra session
  CassSession* session;
  
  // Query ID for prepared insert statement
  const CassPrepared* insertCQLid;  

  // Pending futures (e.g. write requests)
  std::vector<CassFuture*> pendingFutures;             
      
};



} // Namespace

#endif
