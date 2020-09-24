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
#ifndef _BasicEngine_Control_H_
#define _BasicEngine_Control_H_

#include "Algebras/Relation-C++/RelationAlgebra.h"
#include "Algebras/Distributed2/CommandLog.h"
#include "BasicEngine_Thread.h"

namespace BasicEngine {

template<class T>
class BasicEngine_Control {

public:
  BasicEngine_Control(T* _dbs_conn){dbs_conn = _dbs_conn; worker = NULL;};
  BasicEngine_Control(T* _dbs_conn, Relation* _worker)
    {dbs_conn = _dbs_conn; worker =_worker;};
  BasicEngine_Control();
  virtual ~BasicEngine_Control(){delete dbs_conn;};

  T* get_conn(){return dbs_conn;};

  bool partTable(std::string tab, std::string key
            , int anzWorker, std::string art);

  bool drop_table(std::string tab)
    {return sendCommand(dbs_conn->get_drop_table(tab),false);};

  bool createTabFile(std::string tab);

  bool copy(std::string tab, std::string full_path, bool direct)
    {return sendCommand(dbs_conn->get_copy(tab, full_path,direct));};

  bool createTab(std::string tab, std::string query)
    {return sendCommand(dbs_conn->get_createTab(tab, query));};

  std::string getFilePath()
    {return std::string("/home/") + getenv("USER") + "/filetransfer/";};

  std::string getparttabname(std::string tab, std::string key);

  bool importData(std::string tab, int anzWorker);

  bool munion(std::string tab, Relation worker);
private:

  T* dbs_conn;
  Relation* worker;
  const int defaultTimeout = 0;

  bool partRoundRobin(std::string tab, std::string key, int anzWorker);

  bool partHash(std::string tab, std::string key, int anzWorker);

  bool partFun(std::string tab, std::string key
         , int anzWorker, std::string fun);

  bool exportData(std::string tab, std::string key, int anzWorker);

  std::string replaceStringAll(std::string str,
                      const std::string& replace,const std::string& with);

  std::string getjoin(std::string key);

  std::string readFile(std::string path);

  std::string createTabFileName(std::string tab)
    {return "create" + tab + ".sql";};

  bool sendCommand(std::string query, bool print= true)
    {return dbs_conn->sendCommand(query,print);};
};
};  /* namespace BasicEngine */

#endif
