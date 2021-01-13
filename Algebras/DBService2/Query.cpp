/*

1.1.1 Class Implementation

----
This file is part of SECONDO.

Copyright (C) 2017,
Faculty of Mathematics and Computer Science,
Database Systems for New Applications.

SECONDO is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
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

*/
#include "Algebras/DBService2/Query.hpp"

#include <boost/algorithm/string.hpp>

using namespace std;

namespace DBService {

  Query::Query(string database, string newQuery) { 
    // Stringstream doesn't have a copy constructor.
    query = make_shared<stringstream>();
    *query << newQuery;
    setDatabase(database);
  }

  Query::Query(const Query &original) {
    // *query << original.str();
    query = original.getQuery();
    database = original.getDatabase();
  }

  shared_ptr<stringstream> Query::getQuery() const {
    return query;
  }

  string Query::getDatabase() const {
    return database;
  }

  void Query::setDatabase(string newDatabase) {
    if (newDatabase.empty())
      throw "Can't setDatabase on Query given an empty database string!";

    database = newDatabase;
  }

  Query Query::feed() {
    *query << " " << "feed";
    return Query(database, query->str());
  }  
  
  /*
    filterCondition e.g. ".Name = ?".
    A single questionmark (?) will be replaced with the given string parameter.
    Multiple invocations of filter(...) can be chained.
  */
  Query Query::filter(string filterCondition, string parameter) {

    stringstream quotedParameter;
    quotedParameter << "\"" << parameter << "\"";
    boost::replace_all(filterCondition, "?", quotedParameter.str());

    *query << " " << "filter[" << filterCondition << "]";
    return Query(database, query->str());
  }

  Query Query::filter(string filterCondition, int parameter) {

    stringstream quotedParameter;
    quotedParameter << to_string(parameter);
    boost::replace_all(filterCondition, "?", quotedParameter.str());

    *query << " " << "filter[" << filterCondition << "]";
    return Query(database, query->str());
  }

  Query Query::addid() {
    *query << " " << "addid";
    return Query(database, query->str());
  }

  Query Query::filterByTid(int tid) {
    string filterCondition = ".TID = ?";
    //TODO Reduce code repetition with filter(...)
    stringstream quotedParameter;
    quotedParameter << "tid(" << to_string(tid) << ")";
    boost::replace_all(filterCondition, "?", quotedParameter.str());

    *query << " " << "filter[" << filterCondition << "]";
    return Query(database, query->str());
  }

  //TODO Reduce repetition by using a template function across similar 
  // query methods
  Query Query::deletedirect() {
    *query << " " << "deletedirect";
    return Query(database, query->str());
  }

  Query Query::deletebyid(int tid) {
    *query << " " << "deletebyid[tid(" << tid << ")]";
    return Query(database, query->str());
  }

  Query Query::consume() {
    *query << " " << "consume";
    return Query(database, query->str());
  }

  Query Query::appendString(string stringToAppend) {
    *query << " " << stringToAppend;
    return Query(database, query->str());
  }

  Query Query::relation(string relationName) {
    return appendString(relationName);
  }

  string Query::str() const {
    return query->str();
  }


}