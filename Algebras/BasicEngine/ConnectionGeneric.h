/*
----
This file is part of SECONDO.

Copyright (C) 2018,
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

namespace BasicEngine {

class ConnectionGeneric {

    public:

    virtual ~ConnectionGeneric() {}

    virtual bool sendCommand(std::string* command, bool print = true) = 0;

    virtual bool checkConn() = 0;

    virtual std::string createTabFile(std::string* tab) = 0;

    virtual std::string get_init() = 0;

    virtual std::string get_init(std::string* _dbname, std::string* _port) = 0;

    virtual std::string get_drop_table(std::string* tab) = 0;

    virtual std::string get_drop_index(std::string* index) = 0;

    virtual std::string create_geo_index(
        std::string* tab, std::string* geo_col) = 0;

    virtual std::string get_partRoundRobin(std::string* tab, std::string* key
            , std::string* anzSlots, std::string* targetTab) = 0;

    virtual std::string get_partHash(std::string* tab, std::string* key
            , std::string* anzSlots, std::string* targetTab) = 0;

    virtual std::string get_partFun(std::string* tab, std::string* keyS
            ,std::string* anzSlots,std::string* fun,std::string* targetTab) = 0;

    virtual std::string get_partGrid(std::string* tab,std::string* key,
            std::string* geo_col, std::string* anzSlots, 
            std::string* x0, std::string* y0,
            std::string* size, std::string* targetTab) = 0;

    virtual std::string get_exportData(std::string* tab, std::string* join_tab
            ,std::string* key,std::string* nr,std::string* path
            ,long unsigned int* anzWorker) = 0;

    virtual std::string get_copy(
        std::string* tab, std::string* full_path, bool* direct) = 0;

    virtual std::string get_partFileName(
        std::string* tab, std::string* number) = 0;

    virtual std::string get_createTab(
        std::string* tab, std::string* query) = 0;

    virtual bool getTypeFromSQLQuery(std::string sqlQuery, 
         ListExpr &resultList) = 0;
};


}

#endif