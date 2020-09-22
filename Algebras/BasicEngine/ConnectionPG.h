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
#include <string>
#include "libpq-fe.h"

namespace BasicEngine {

class ConnectionPG {

public:
  ConnectionPG(int port, std::string dbname);
  ConnectionPG();
  virtual ~ConnectionPG(){};

  int get_port()
    {return port;};

  std::string get_dbname()
    {return dbname;};

  bool sendCommand(std::string command, bool print = true);

  PGresult* sendQuery(std::string query);

  std::string createTabFile(std::string tab);

  bool checkConn();

  std::string get_init()
    {return "query init_pg(" + std::to_string(port) + ",'"+ dbname +"');";}

  std::string get_init(std::string _dbname, std::string _port)
    {return "query init_pg(" + _port + ",'"+ _dbname +"');";}

  std::string get_drop_table(std::string tab)
    {return "DROP TABLE IF EXISTS " + tab + ";";}

  std::string get_partRoundRobin(std::string tab, std::string key
            , std::string anzWorker, std::string targetTab);

  std::string get_partHash(std::string tab, std::string key
            , std::string anzWorker, std::string targetTab);

  std::string get_partFun(std::string tab, std::string key
            ,std::string anzWorker,std::string fun,std::string targetTab);

  std::string get_exportData(std::string tab, std::string join_tab
            , std::string join_stat, std::string nr, std::string path);

  std::string get_copy(std::string tab, std::string full_path, bool direct);

  std::string get_partTabName(std::string tab, std::string number)
    {return tab + "_" + number +".bin";};

  std::string get_createTab(std::string tab, std::string query)
    {return "CREATE TABLE " + tab + " AS ("+ query + ")";};
private:
  PGconn* conn;
  int port;
  std::string dbname;

  bool createFunctionRandom(std::string* tab, std::string* key
            , std::string* anzWorker, std::string* select);

  bool createFunctionDDRandom(std::string* tab, std::string* key
            , std::string* anzWorker, std::string* select);

  void getFieldInfoFunction(std::string* tab, std::string* key
             ,std::string *fields,std::string *valueMap,std::string* select);
};

}; /* namespace BasicEngine */
