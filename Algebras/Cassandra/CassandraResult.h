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

#ifndef _CASSANDRA_RESULT_H
#define _CASSANDRA_RESULT_H

#include "CassandraAdapter.h"

using namespace std;

//namespace to avoid name conflicts
namespace cassandra {

// Prototype class
class CassandraAdapter;
  
/*
2.1 This class is used as return value for CQL querys
    You can use it to iteratate over the result set

*/
class CassandraResult {
  
public:

     virtual ~CassandraResult() { }
  
     virtual bool hasNext() {
       // To be implemented in subclasses
       return false;
     }
    
     virtual void getStringValue(string &resultString, int pos) {
       // To be implemented in subclasses
     }
     
     virtual int getIntValue(int pos) {
       // To be implemented in subclasses
       return -1;
     }
     
};
     
/*
2.2 Result object for one cql query

*/
class SingleCassandraResult : public CassandraResult {
  
public:

     SingleCassandraResult(cql::cql_result_t& myResult) : result(myResult) {
     }
     
     //virtual ~SingleCassandraResult() { }
     
     virtual bool hasNext();
     virtual void getStringValue(string &resultString, int pos);
     virtual int getIntValue(int pos);
     
private:
     cql::cql_result_t& result;
};

/*
2.3 Result object for >1 cql query

*/
class MultiCassandraResult : public CassandraResult {
  
public:
     
  MultiCassandraResult(vector<string> myQueries, 
                          CassandraAdapter* myCassandraAdapter,
                          cql::cql_consistency_enum myConsistenceLevel);
  
  virtual ~MultiCassandraResult();
  virtual bool setupNextQuery();
  virtual bool hasNext();
  virtual void getStringValue(string &resultString, int pos);
  virtual int getIntValue(int pos);
private:
  vector<string> queries;
  CassandraAdapter* cassandraAdapter;
  cql::cql_consistency_enum consistenceLevel;
  CassandraResult* cassandraResult;
};

}

#endif

