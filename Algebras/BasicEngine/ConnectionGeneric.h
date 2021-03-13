/*
----
This file is part of SECONDO.

Copyright (C) 2021,
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

//[$][\$]

*/
#ifndef _CONNECTION_GENERIC_H_
#define _CONNECTION_GENERIC_H_

#include <string>
#include "ResultIteratorGeneric.h"

namespace BasicEngine {

class ConnectionGeneric {

    public:

    ConnectionGeneric(const std::string &_dbUser, const std::string &_dbPass, 
       const int _dbPort, const std::string &_dbName) : dbUser(_dbUser),
       dbPass(_dbPass), dbPort(_dbPort), dbName(_dbName) {

    }

    virtual ~ConnectionGeneric() {

    }

    virtual std::string getDbType() = 0;

    virtual bool createConnection() = 0;

    virtual bool checkConnection() = 0;

    virtual bool validateQuery(const std::string &query) = 0;

    virtual bool sendCommand(const std::string &command, bool print = true) = 0;

    virtual std::string getCreateTableSQL(const std::string &tab) = 0;

    virtual std::string getDropTableSQL(const std::string &table) = 0;

    virtual std::string getDropIndexSQL(const std::string &index) = 0;

    virtual std::string getCreateGeoIndexSQL(
        const std::string &table, const std::string &geo_col) = 0;

    virtual bool partitionRoundRobin(const std::string &table, 
        const std::string &key, const size_t anzSlots, 
        const std::string &targetTab) = 0;

    virtual std::string getPartitionHashSQL(const std::string &table, 
        const std::string &key, const size_t anzSlots, 
        const std::string &targetTab) = 0;

    virtual std::string getPartitionSQL(const std::string &table, 
        const std::string &keyS, const size_t anzSlots, 
        const  std::string &fun, const std::string &targetTab) = 0;

    virtual std::string getPartitionGridSQL(const std::string &table, 
            const std::string &key,
            const std::string &geo_col, const size_t anzSlots, 
            const std::string &x0, const std::string &y0,
            const std::string &size, const std::string &targetTab) = 0;

    virtual std::string getExportDataSQL(const std::string &table, 
            const std::string &join_table, const std::string &key, 
            const std::string &nr, const std::string &path,
            size_t numberOfWorker) = 0;

    virtual std::string getImportTableSQL(
        const std::string &table, const std::string &full_path) = 0;

    virtual std::string getExportTableSQL(
        const std::string &table, const std::string &full_path) = 0;

    virtual std::string getFilenameForPartition(
        const std::string &table, const std::string &number) = 0;

    virtual std::string getCreateTabSQL(
        const std::string &table, const std::string &query) = 0;

    virtual std::string getCopySchemaSQL(const std::string &table) = 0;

    virtual std::string getRenameTableSQL(const std::string &source, 
        const std::string &destination) = 0;

    virtual bool getTypeFromSQLQuery(const std::string &sqlQuery, 
         ListExpr &resultList) = 0;

    virtual ResultIteratorGeneric* performSQLSelectQuery(
        const std::string &sqlQuery) = 0;


    /*
    5.3 

    5.3.1 ~getDbPort~ Return the Port of the database

    */
    int getDbPort() {
        return dbPort;
    }

    /*
    5.3.2 ~getDbName~

    Return the name of the database.

    */
    std::string getDbName() {
        return dbName;
    }

    /*
    5.3.3 ~getDbUser~

    Return the user of the database.

    */
    std::string getDbUser() {
        return dbUser;
    }

    /*
    5.3.4 ~getDbPass~

    Return the password of the database.

    */
    std::string getDbPass() {
        return dbPass;
    }

    protected:
    /*
    5.2.0 Try to limit the given SQL query to 1 result. 
        
    Needed to determine the type of the query result.
    */
    std::string limitSQLQuery(const std::string &query);

    /*
    5.2.1 ~dbUser~

    The username for the database connection

    */
    std::string dbUser;

    /*
    5.2.2 ~dbPass~

    The password of the postgress connection

    */
    std::string dbPass;

    /*
    5.2.3 ~port~

    The port of the database

    */
    int dbPort;

    /*
    5.2.4 ~dbname~

    The Name of the Database.

    */
    std::string dbName;
};
}

#endif