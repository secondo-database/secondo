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
#ifndef _BasicEngine_Control_H_
#define _BasicEngine_Control_H_

#include "Algebras/Relation-C++/RelationAlgebra.h"
#include "BasicEngine_Thread.h"
#include "BasicEngineHelper.cpp"

namespace BasicEngine {

/*
2 Class ~BasicEngine\_Control~

This class represents the controling from the system.

*/
template<class T>
class BasicEngine_Control {

public:

/*
2.1 Public Methods

*/
  BasicEngine_Control(T* _dbs_conn)
  {
    dbs_conn = _dbs_conn;
    worker = NULL;
    anzWorker = 0;
  };

  BasicEngine_Control(T* _dbs_conn, Relation* _worker)
  {
    dbs_conn = _dbs_conn;
    worker =_worker->Clone();
    anzWorker = worker->GetNoTuples();
    createAllConnection();
  };

  virtual ~BasicEngine_Control(){delete dbs_conn;};

  T* get_conn(){return dbs_conn;};

  bool checkConn();

  bool partTable(std::string tab, std::string key
            , std::string art, int slotSize);

  bool drop_table(std::string tab)
    {return sendCommand(dbs_conn->get_drop_table(&tab),false);};

  bool createTabFile(std::string tab);

  bool copy(std::string tab, std::string full_path, bool direct)
    {return sendCommand(dbs_conn->get_copy(&tab,&full_path,&direct));};

  bool createTab(std::string tab, std::string query)
    {return sendCommand(dbs_conn->get_createTab(&tab, &query));};

  bool munion(std::string tab);

  bool mquery(std::string query, std::string tab);

  bool mcommand(std::string query);

  bool runsql(std::string filepath);

  bool sendCommand(std::string query, bool print= true)
    {return dbs_conn->sendCommand(&query,print);};

private:

/*
2.2 Members

2.2.1 ~dbs\_conn~

In this template variable were stores the connection,
to a secondary dbms (for example postgresql)

*/
T* dbs_conn;

/*
2.2.2 ~worker~

The worker is a relation with all informations about the
worker connection like port, connection-file, ip

*/
Relation* worker;

/*
2.2.3 ~vec\_ci~

In this vector all connection to the worker are stored.

*/
std::vector<distributed2::ConnectionInfo*> vec_ci;

/*
2.2.4 ~importer~

In this vector all informations for starting the thread
are stored.

*/
std::vector<BasicEngine_Thread*> importer;

/*
2.2.4 ~anzWorker~

The anzWorker counts the number of worker.

*/
long unsigned int anzWorker;

/*
2.3 Private Methods

*/
  bool createAllConnection();

  bool createConnection(long unsigned int* index);

  bool partRoundRobin(std::string* tab, std::string* key, int* slotsize);

  bool partHash(std::string* tab, std::string* key, int* slotsize);

  bool partFun(std::string* tab, std::string* key
         , std::string* fun, int* slotsize);

  bool exportData(std::string* tab, std::string* key
         , long unsigned int* slotsize);

  bool importData(std::string* tab);

  bool exportToWorker(std::string* tab);

  std::string createTabFileName(std::string* tab)
    {return "create" + *tab + ".sql";}

  std::string get_partFileName(std::string* tab, std::string* nr)
    {return dbs_conn->get_partFileName(tab,nr);}

  std::string getFilePath()
    {return std::string("/home/") + getenv("USER") + "/filetransfer/";};

  std::string getparttabname(std::string* tab, std::string* key);

};
};  /* namespace BasicEngine */

#endif //_BasicEngine_Control_H_
