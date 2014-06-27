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
#include "CassandraResult.h"

using namespace std;

//namespace to avoid name conflicts
namespace cassandra {
  
/*
2.2 Result object for one cql query

*/
bool SingleCassandraResult::hasNext() {

  // Wait for result
  if(! futureWaitCalled ) {
      futureWaitCalled = true;      
      try {
          future.wait();
      } catch(std::exception& e) {
          cerr << "Got exception while reading data: " << e.what() << endl;
          return false;
      }
  }
  
  cql::cql_result_t& result = *(future.get().result);
  return result.next();
}

void SingleCassandraResult::getStringValue(string &resultString, int pos) {
  cql::cql_result_t& result = *(future.get().result);
  result.get_string(pos, resultString);
}

int SingleCassandraResult::getIntValue(int pos) {
  int resultInt;
  cql::cql_result_t& result = *(future.get().result);
  result.get_int(pos, resultInt);
  return resultInt;
}

/*
2.3 Result object for >1 cql query

*/
MultiCassandraResult::MultiCassandraResult(vector<string> myQueries, 
                    CassandraAdapter* myCassandraAdapter,
                    cql::cql_consistency_enum myConsistenceLevel) 
  : queries(myQueries), cassandraAdapter(myCassandraAdapter), 
  consistenceLevel(myConsistenceLevel) { 
    
    cassandraResult = NULL;
}

MultiCassandraResult::~MultiCassandraResult() {
  
  if(cassandraResult != NULL) {
    delete cassandraResult;
    cassandraResult = NULL;
  }
  
  while(! pendingResults.empty()) {
    cassandraResult = pendingResults.front();
    pendingResults.pop();
    delete cassandraResult;
  }
}

bool MultiCassandraResult::setupNextQuery() {
  cout << "Preparing next query" << endl;
  cout << "Size of queue: " << pendingResults.size() << endl;
  cout << "Size of queries: " << queries.size() << endl;
  
  // Delete old query
  if(cassandraResult != NULL) {
    delete cassandraResult;
    cassandraResult = NULL;
  }
  
  while(pendingResults.size() < 10 && queries.size() > 0) {
    string cql = queries.back();
    queries.pop_back();
    cout << "[Executing] Query is " << cql << endl;
    CassandraResult* myCassandraResult = cassandraAdapter
          ->readDataFromCassandra(cql, consistenceLevel);
    cout << "[Running]" << endl;          
    pendingResults.push(myCassandraResult);
  }

  if(! pendingResults.empty()) {
    cassandraResult = pendingResults.front();
    pendingResults.pop();
    return true;
  }

  return false;
}

void MultiCassandraResult::getStringValue(string &resultString, int pos) {
  cassandraResult -> getStringValue(resultString, pos);
}

int MultiCassandraResult::getIntValue(int pos) {
  return cassandraResult -> getIntValue(pos);
}

bool MultiCassandraResult::hasNext() {
  // No query active and we have a new query to execute
  if(cassandraResult == NULL && 
     (queries.size() > 0 || pendingResults.size() > 0)) {
    
    // Execute the next query
    setupNextQuery();
    
    return hasNext();
  }
  
  // Do we have a next element in current query?
  if(cassandraResult->hasNext()) {
    return true;
  }
  
  // Setup next query and try again
  if(setupNextQuery()) {
    return hasNext();
  }
  
  return false;
}
  
  
} // Namespace