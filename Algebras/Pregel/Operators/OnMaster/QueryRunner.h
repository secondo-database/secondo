/*
----
This file is part of SECONDO.

Copyright (C) 2019, 
University in Hagen, 
Faculty of Mathematics and  Computer Science,
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

#pragma once
#include <string>
#include "Algebras/Distributed2/ConnectionInfo.h"
#include "../../typedefs.h"

namespace pregel{

 class QueryRunner{
   public:
      QueryRunner(WorkerConnection* _connection, 
                  const std::string& _query);

      ~QueryRunner();

      std::string getResult();

      bool successful();

      std::string errorMessage();

      QueryRunner(const QueryRunner&) = delete;
      QueryRunner(QueryRunner&& ) = delete;

      
      
   private:
      WorkerConnection* connection;
      std::string query;
      boost::thread* runner;
      int err;
      std::string errMsg;
      std::string result;
      
      void run();
 };
}


