/*
---- 
This file is part of SECONDO.

Copyright (C) 2021, University in Hagen, Department of Computer Science, 
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

#include <iostream>
#include <chrono>
#include <ctime>
#include <string>
#include <sstream>
#include <cstdlib>
#include <assert.h>
#include <sys/time.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <db_cxx.h>

using namespace std;

void handleBDBRet(int ret, std::string operation) {
    if(ret != 0) {
        std::cerr << "Error: Operation " << operation 
            << " / ret=" << ret << std::endl;
        exit(1);
    }

    std::cout << "Success: Operation " << operation
            << " / ret=" << ret << std::endl;
}

int main() {
    
    int ret;
    time_t now = time(nullptr);

    std::stringstream ss;
    ss << now;
    std::string prefix = ss.str();

    std::string dbDir = "/tmp/dbenv_" + prefix;
    std::cout << "Init DB dir: " << dbDir << std::endl;

    ret = mkdir(dbDir.c_str(), 0700);
    handleBDBRet(ret, "mkdir");

    DbEnv* dbenv = new DbEnv( DB_CXX_NO_EXCEPTIONS );
    ret = dbenv->set_cachesize( 0, 32*1024*1024, 0);
    handleBDBRet(ret, "set_cachesize");

    ret = dbenv->open( dbDir.c_str(), 
        DB_CREATE   | DB_RECOVER  |
        DB_INIT_LOCK | DB_INIT_LOG | DB_INIT_MPOOL |
        DB_INIT_TXN, 0 );
    handleBDBRet(ret, "open dbenv");

    string dbName = "test";

    while(true) {
        Db* db = new Db(dbenv, DB_CXX_NO_EXCEPTIONS);
        ret = db->open(0, dbName.c_str(), 
            0, DB_BTREE, DB_CREATE | DB_AUTO_COMMIT, 0);
        handleBDBRet(ret, "open btree");

        ret = db->close( 0 );
        handleBDBRet(ret, "close btree");

        std::string dbFile = dbDir + "/" + dbName;
        
        /*ret = std::remove(dbFile.c_str());
        handleBDBRet(ret, "delete btree");
        */

        delete db;
        db = nullptr;

        std::cout << dbFile << endl;
        Db mydb(dbenv, 0);
        ret = mydb.remove(dbFile.c_str(), NULL, 0);
        handleBDBRet(ret, "delete btree");

        usleep(1000000);
    }

    ret = dbenv->close( 0 );
    handleBDBRet(ret, "close");


    return EXIT_SUCCESS;
}