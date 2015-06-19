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

 The classes, contained in this file, encapsulates a cassandra result 
 statement into a CassandraResult object. These objects can be lazy 
 evaluated or evaluated in multiple threads.
 
 1 Includes and defines

*/

#ifndef _CASSANDRA_RESULT_H
#define _CASSANDRA_RESULT_H

#include <queue>
#include "CassandraTuplePrefetcher.h"
#include "CassandraHelper.h"
#include "CassandraAdapter.h"
#include "CassandraToken.h"

using namespace std;

//namespace to avoid name conflicts
namespace cassandra {


// Prototype class
class CassandraAdapter;
     
/*
2.2 Result object for one cql query

*/
class CassandraResult {
  
public:

     // Constructor with one query
     CassandraResult(CassSession* mySession, string myStatement,
        CassConsistency myConsistenceLevel, bool myPrintError = true)
        : session(mySession), statement(NULL), future(NULL),
          result(NULL), iterator(NULL), row(NULL), printError(myPrintError) {

             queries.push_back(myStatement);
             setupNextQuery();
     }

     // Constructor with query vector
     CassandraResult(CassSession* mySession, vector<string> myQueries,
        CassConsistency myConsistenceLevel, bool myPrintError = true)
        : session(mySession), statement(NULL), future(NULL),
          result(NULL), iterator(NULL), row(NULL),
          queries(myQueries), printError(myPrintError) {

         setupNextQuery();
     }

     void freeIterator() {
        if(iterator != NULL) {
            cass_iterator_free(iterator);
            iterator = NULL;
        }
     }

     void freeResult() {
        if(result != NULL) {
             cass_result_free(result);
             result = NULL;     
        }
     }

     void freeFuture() {
        if(future != NULL) {
            cass_future_free(future);
            future = NULL;
        }
     }
     
     void freeStatement() {
        if(statement != NULL) {
           cass_statement_free(statement);
           statement = NULL;
        }
     }
     
     void freeAll() {
        freeIterator();
        freeResult();
        freeFuture();
        freeStatement();
     }
     
     virtual ~CassandraResult() {
        freeAll();
     }
     
     virtual bool hasNext();
     virtual void getStringValue(string &resultString, int pos);
     virtual cass_int32_t getIntValue(int pos);
     virtual cass_int64_t getBigIntValue(int pos);
     void setupNextQuery();
     
private:
     CassSession* session;
     CassStatement* statement;
     CassFuture* future;
     const CassResult* result;
     CassIterator* iterator;
     const CassRow* row;
     vector<string> queries;
     queue< vector< CassRow*> > results;
     bool printError;
};

} // Namespace


#endif
