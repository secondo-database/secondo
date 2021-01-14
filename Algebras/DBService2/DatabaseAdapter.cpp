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
#include "Algebras/DBService2/DatabaseAdapter.hpp"
#include "Algebras/DBService2/SecondoDatabaseAdapter.hpp"
#include "Algebras/DBService2/DebugOutput.hpp"

#include <string>

using namespace std;

namespace DBService
{

  DatabaseAdapter::DatabaseAdapter()
  {
  }

  // Copy
  DatabaseAdapter::DatabaseAdapter(const DatabaseAdapter &original)
  {
    cout << "Copy DatabaseAdapter::DatabaseAdapter" << endl;
  }

  // Move
  DatabaseAdapter::DatabaseAdapter(const DatabaseAdapter &&original)
  {
    cout << "Move DatabaseAdapter::DatabaseAdapter" << endl;
  }

  //TODO Add additinonal method or paramter to specify the kind of 
  //  DatabaseAdapter to be created.
  // e.g. MockDatabaseAdapter or SecondoDatabaseAdapter
  shared_ptr<DatabaseAdapter> DatabaseAdapter::getInstance() {
    return SecondoDatabaseAdapter::getInstance();
  }

  //TODO Make DatabaseAdapter pure virtual.
  //TODO Create special exception or find other way to reduce the unnecessary 
  //  repetition of strings in this class.

  int DatabaseAdapter::executeInsertQuery(string database, string insertQuery)
  {
    throw SecondoException("executeInsertQuery: Not implemented in \
DatabaseAdapter. Use subclass such as SecondoDatabaseAdapter.");

    return -1;
  }

  void DatabaseAdapter::executeCreateRelationQuery(string database, 
    string query) {

    throw SecondoException("executeCreateRelationQuery: Not implemented in \
DatabaseAdapter. Use subclass such as SecondoDatabaseAdapter.");

  }

  void DatabaseAdapter::executeQueryWithoutResult(string database, 
    string query) {

    throw SecondoException("executeQueryWithoutResult: Not implemented in \
DatabaseAdapter. Use subclass such as SecondoDatabaseAdapter.");

  }

  ListExpr DatabaseAdapter::executeFindQuery(string database, string query)
  {
    throw SecondoException("executeQuery: Not implemented in DatabaseAdapter. \
Use subclass such as SecondoDatabaseAdapter.");

  }

  bool DatabaseAdapter::doesDatabaseExist(string database) {
    throw SecondoException("doesDatabaseExist: Not implemented in \
DatabaseAdapter. Use subclass such as SecondoDatabaseAdapter.");

  }
  void DatabaseAdapter::openDatabase(string database) {
    throw SecondoException("openDatabase: Not implemented in DatabaseAdapter. \
Use subclass such as SecondoDatabaseAdapter.");

  }
  void DatabaseAdapter::closeDatabase() {
    throw SecondoException("closeDatabase: Not implemented in DatabaseAdapter. \
Use subclass such as SecondoDatabaseAdapter.");

  }
  void DatabaseAdapter::createDatabase(string database) {
    throw SecondoException("createDatabase: Not implemented in \
DatabaseAdapter. Use subclass such as SecondoDatabaseAdapter.");

  }

  void DatabaseAdapter::deleteDatabase(string database) {
    throw SecondoException("deleteDatabase: Not implemented in \
DatabaseAdapter. Use subclass such as SecondoDatabaseAdapter.");

  }

  void DatabaseAdapter::createRelation(string database, string relationName, 
    string createRelationStatement)
  {
    throw SecondoException("createRelation: Not implemented in \
DatabaseAdapter. Use subclass such as SecondoDatabaseAdapter.");

  }

  bool DatabaseAdapter::doesRelationExist(string database, 
    string relationName) {
    throw SecondoException("doesRelationExist: Not implemented in \
DatabaseAdapter. Use subclass such as SecondoDatabaseAdapter.");

  }

  bool DatabaseAdapter::isDatabaseOpen() {
    throw SecondoException("isDatabaseOpen: Not implemented in \
DatabaseAdapter. Use subclass such as SecondoDatabaseAdapter.");

  }

  string DatabaseAdapter::getCurrentDatabase() {
    throw SecondoException("getCurrentDatabase: Not implemented in \
DatabaseAdapter. Use subclass such as SecondoDatabaseAdapter.");

  }

} // namespace DBService
