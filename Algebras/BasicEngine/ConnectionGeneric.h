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

    virtual ~ConnectionGeneric() {}

    virtual bool sendCommand(const std::string &command, bool print = true) = 0;

    virtual bool checkConnection() = 0;

    virtual std::string createTabFile(const std::string &tab) = 0;

    virtual std::string getInitSecondoCMD(const std::string &dbname, 
        const std::string &port, const std::string &workerRelation) = 0;

    virtual std::string get_drop_table(const std::string &table) = 0;

    virtual std::string get_drop_index(const std::string &index) = 0;

    virtual std::string create_geo_index(
        const std::string &table, const std::string &geo_col) = 0;

    virtual std::string get_partRoundRobin(const std::string &table, 
        const std::string &key, const std::string &anzSlots, 
        const std::string &targetTab) = 0;

    virtual std::string get_partHash(const std::string &table, 
        const std::string &key, const std::string &anzSlots, 
        const std::string &targetTab) = 0;

    virtual std::string get_partFun(const std::string &table, 
        const std::string &keyS, const std::string &anzSlots, 
        const  std::string &fun, const std::string &targetTab) = 0;

    virtual std::string get_partGrid(const std::string &table, 
            const std::string &key,
            const std::string &geo_col, const std::string &anzSlots, 
            const std::string &x0, const std::string &y0,
            const std::string &size, const std::string &targetTab) = 0;

    virtual std::string get_exportData(const std::string &table, 
            const std::string &join_table, const std::string &key, 
            const std::string &nr, const std::string &path,
            size_t numberOfWorker) = 0;

    virtual std::string get_copy(
        const std::string &table, const std::string &full_path, 
        bool direct) = 0;

    virtual std::string get_partFileName(
        const std::string &table, const std::string &number) = 0;

    virtual std::string getCreateTabSQL(
        const std::string &table, const std::string &query) = 0;

    virtual bool getTypeFromSQLQuery(const std::string &sqlQuery, 
         ListExpr &resultList) = 0;

    virtual ResultIteratorGeneric* performSQLQuery(
        const std::string &sqlQuery) = 0;

    virtual std::string getDatabaseName() = 0;

    virtual int getDatabasePort() = 0;
};


}

#endif