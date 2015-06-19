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
#include "CassandraTuplePrefetcher.h"
#include "CassandraResult.h"

// Activate debug messages
//#define __DEBUG__

using namespace std;

//namespace to avoid name conflicts
namespace cassandra {
  
/*
2.2 Result object for one cql query

*/
bool CassandraResult::hasNext() {

  CassError rc = CASS_OK;
  bool hasNextResult = true;

  // Future could not be prepared
  if(future == NULL) {
      return false;
  }

  // Get Result from Future 
  if(result == NULL || iterator == NULL) {
           
      // Wait max 30 seconds
      if(! cass_future_wait_timed(future, 30 * 1000000)) {
         cerr << "Future wait timed out!" << endl;
         return false;
      }
          
      rc = cass_future_error_code(future);

      if (rc != CASS_OK) {
         CassandraHelper::print_error(future);
         return false;
      }
  
      result = cass_future_get_result(future);
      iterator = cass_iterator_from_result(result);
  }

  hasNextResult = cass_iterator_next(iterator);

  if(hasNextResult == true) {
     row = cass_iterator_get_row(iterator);
  } else {
     // Get next page
     bool hasMorePages = cass_result_has_more_pages(result);

     if(hasMorePages) {
        // Set position to next page
        cass_statement_set_paging_state(statement, result);
        
        // Cleanup data from previous page
        freeIterator();
        freeResult();
        freeFuture();

        future = cass_session_execute(session, statement);
        return hasNext();
     } else {
        if(queries.size() > 0) {
           setupNextQuery();
           return hasNext();
        }
     }
  }

  return hasNextResult;
}

void CassandraResult::setupNextQuery() {
   
#ifdef __DEBUG__
  cout << "Preparing next query" << endl;
  cout << "Size of queries: " << queries.size() << endl;
#endif
  
   freeAll();
   
   if(queries.size() == 0) {
      return;
   }
   
   string query = queries.back();
   queries.pop_back();
      
   statement = cass_statement_new(query.c_str(), 0);
   
   cass_statement_set_paging_size(statement, 100);

   future = cass_session_execute(session, statement);

   CassError rc = cass_future_error_code(future);

   if(rc != CASS_OK) {

      if(printError) {
         cerr << "Unable to execute statement " << endl;
         CassandraHelper::print_error(future);
      }   

      future = NULL;
   }
}

void CassandraResult::getStringValue(string &resultString, int pos) {
   const char* item;
   size_t item_length;   
   cass_value_get_string(cass_row_get_column(row, pos), &item, &item_length);
   resultString.append(item, item_length);
}

cass_int32_t CassandraResult::getIntValue(int pos) {
  cass_int32_t resultInt;
  cass_value_get_int32(cass_row_get_column(row, pos), &resultInt);
  return resultInt;
}

cass_int64_t CassandraResult::getBigIntValue(int pos) {
  cass_int64_t resultInt;
  cass_value_get_int64(cass_row_get_column(row, pos), &resultInt);
  return resultInt;
}
  
} // Namespace
