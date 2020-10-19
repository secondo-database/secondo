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

@author
c. Behrndt

@note
Checked - 2020

@history
Version 1.0 - Created - C.Behrndt - 2020

*/
#ifndef _ConnectionPG_H_
#define _ConnectionPG_H_

#include <string>
#include "libpq-fe.h"

namespace BasicEngine {

/*
5 Class ~ConnectionPG~

This class represents the controling from the system.

*/
class ConnectionPG {

public:
  /*
  5.1 Public Methods

  */
  ConnectionPG(int port, std::string dbname);
  virtual ~ConnectionPG(){};

  bool sendCommand(std::string* command, bool print = true);

  bool checkConn();

  std::string createTabFile(std::string* tab);

  std::string get_init()
    {return "query init_pg(" + std::to_string(port) + ",'"+ dbname +"');";}

  std::string get_init(std::string* _dbname, std::string* _port)
    {return "query init_pg(" + *_port + ",'"+ *_dbname +"');";}

  std::string get_drop_table(std::string* tab)
    {return "DROP TABLE IF EXISTS " + *tab + ";";}

  std::string get_partRoundRobin(std::string* tab, std::string* key
            , std::string* anzSlots, std::string* targetTab);

  std::string get_partHash(std::string* tab, std::string* key
            , std::string* anzSlots, std::string* targetTab);

  std::string get_partFun(std::string* tab, std::string* key
            ,std::string* anzSlots,std::string* fun,std::string* targetTab);

  std::string get_exportData(std::string* tab, std::string* join_tab
            ,std::string* join_stat,std::string* nr,std::string* path
            ,long unsigned int* anzWorker);

  std::string get_copy(std::string* tab, std::string* full_path, bool* direct);

  std::string get_partTabName(std::string* tab, std::string* number)
    {return *tab + "_" + *number +".bin";};

  std::string get_createTab(std::string* tab, std::string* query)
    {return "CREATE TABLE " + *tab + " AS ("+ *query + ")";};

private:

  /*
  5.2 Members

  5.2.1 ~conn~

  The connection to PostgreSQL

  */
  PGconn* conn;

  /*
  5.2.2 ~port~

  The port from the PostgreSQL DB.

  */
  int port;

  /*
  5.2.3 ~dbname~

  The Name of the Database.

  */
  std::string dbname;

  /*
  5.3 Private Methods

  */
  int get_port()
    {return port;};

  std::string get_dbname()
    {return dbname;};

  PGresult* sendQuery(std::string* query);

  bool createFunctionRandom(std::string* tab, std::string* key
            , std::string* anzWorker, std::string* select);

  bool createFunctionDDRandom(std::string* tab, std::string* key
            , std::string* anzWorker, std::string* select);

  void getFieldInfoFunction(std::string* tab, std::string* key
             ,std::string* fields,std::string* valueMap,std::string* select);
};

}; /* namespace BasicEngine */
#endif //_ConnectionPG_H_
