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
     
     virtual cass_int32_t getIntValue(int pos) {
       // To be implemented in subclasses
       return -1;
     }
     
     virtual cass_int64_t getBigIntValue(int pos) {
       // To be implemented in subclasses
       return -1;
     }
     
};
     
/*
2.2 Result object for one cql query

*/
class SingleCassandraResult : public CassandraResult {
  
public:

     SingleCassandraResult(
       CassFuture* myFuture) 
        : future(myFuture), 
          result(NULL), iterator(NULL), row(NULL),
          futureWaitCalled(false) {
     }
     
     virtual ~SingleCassandraResult() { 
         
         if(result != NULL) {
              cass_result_free(result);
              result = NULL;     
         }

         if(iterator != NULL) {
             cass_iterator_free(iterator);
             iterator = NULL;
         }

         if(future != NULL) {
             cass_future_free(future);
             future = NULL;
         }
     }
     
     virtual bool hasNext();
     virtual void getStringValue(string &resultString, int pos);
     virtual cass_int32_t getIntValue(int pos);
     virtual cass_int64_t getBigIntValue(int pos);
     
private:
     CassFuture* future;
     const CassResult* result;
     CassIterator* iterator;
     const CassRow* row;
     bool futureWaitCalled;
};

/*
2.3 Result object for multiple cql queries

*/
class MultiCassandraResult : public CassandraResult {
  
public:
     
  MultiCassandraResult(vector<string> myQueries, 
                          CassandraAdapter* myCassandraAdapter,
                          CassConsistency myConsistenceLevel);
  
  virtual ~MultiCassandraResult();
  virtual bool setupNextQuery();
  virtual bool hasNext();
  virtual void getStringValue(string &resultString, int pos);
  virtual cass_int32_t getIntValue(int pos);
  virtual cass_int64_t getBigIntValue(int pos);
  
protected:
  vector<string> queries;
  CassandraAdapter* cassandraAdapter;
  CassConsistency consistenceLevel;
  CassandraResult* cassandraResult;
};

/*
2.4 Result object for multiple cql queries

Multi Threaded version

*/
class MultiThreadedCassandraResult : public MultiCassandraResult {
  
public:
     
  MultiThreadedCassandraResult(vector<string> myQueries, 
                          CassandraAdapter* myCassandraAdapter,
                          CassConsistency myConsistenceLevel
                      );
  
  virtual ~MultiThreadedCassandraResult();
  virtual bool hasNext();
  virtual void getStringValue(string &resultString, int pos);
  virtual cass_int32_t getIntValue(int pos);
  virtual cass_int64_t getBigIntValue(int pos);
  
private:
  queue< vector<string> > results;
  pthread_mutex_t queryMutex;
  pthread_mutex_t queueMutex;
  pthread_cond_t queueCondition;
  
  size_t runningThreads;
  vector<pthread_t> threads;
  bool firstCall;
};


/*
2.5 Result objects for tokens

*/
class CassandraToken {
  
public:

/*
2.5.1 Default constructor

*/
  CassandraToken(long long myToken, string myIp) 
    : token(myToken), ip(myIp) {
      
  }
  
/*
2.5.2 Is IP local?

*/  
  bool isLocalToken() const {
    return ip.compare("127.0.0.1");
  }
  
/*
2.5.3 Getter for token

*/
  long long getToken() const {
    return token;
  }

/*
2.5.4 Getter for ip

*/
  string getIp() const {
    return ip;
  }

/*
2.5.5 Operator <

*/
  bool operator<( const CassandraToken& val ) const { 
        return getToken() < val.getToken(); 
  }

private:
  long long token;
  string ip;
};

/*
2.5.6 Implementation for "toString"

*/
inline std::ostream& operator<<(std::ostream &strm, 
                         const cassandra::CassandraToken &cassandraToken) {
  
  return strm << "CassandraToken[" << cassandraToken.getToken() 
              << " / " << cassandraToken.getIp()  << "]" << endl;
}

} // Namespace


#endif
