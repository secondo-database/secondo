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

@author
c. Behrndt

@history
Version 1.0 - Created - C.Behrndt - 2020

*/


#include <boost/log/core.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/trivial.hpp>
#include <boost/algorithm/string/predicate.hpp>

#include "Algebras/FText/FTextAlgebra.h"
#include "Algebras/Distributed2/DArray.h"
#include "StandardTypes.h"
#include "BasicEngine.h"
#include "BasicEngineControl.h"
#include "GridManager.h"

#include "operators/be_command.h"
#include "operators/be_query.h"
#include "operators/be_copy.h"
#include "operators/be_mquery.h"
#include "operators/be_mcommand.h"
#include "operators/be_shutdown.h"
#include "operators/be_shutdown_cluster.h"
#include "operators/be_union.h"
#include "operators/be_struct.h"
#include "operators/be_init_cluster.h"
#include "operators/be_init.h"
#include "operators/be_run_sql.h"
#include "operators/be_collect.h"
#include "operators/be_share.h"
#include "operators/be_validate_query.h"
#include "operators/be_grid_create.h"
#include "operators/be_grid_delete.h"
#include "operators/be_part_random.h"
#include "operators/be_part_rr.h"
#include "operators/be_part_hash.h"
#include "operators/be_part_fun.h"
#include "operators/be_part_grid.h"
#include "operators/be_repart_random.h"
#include "operators/be_repart_rr.h"
#include "operators/be_repart_hash.h"
#include "operators/be_repart_fun.h"
#include "operators/be_repart_grid.h"

#include "ConnectionPostgres.h"
#include "ConnectionMySQL.h"

using namespace distributed2;
using namespace std;

namespace BasicEngine {

/*
0 Declaring variables

dbs\_con is a pointer to a connection, for example to postgres

*/
BasicEngineControl* be_control = nullptr;

/*
1 Operators

1.1 Operator  ~be\_init~

Establishes a connection to a running postgres System.
The result of this operator is a boolean indicating the success
of the operation.

1.1.1 Type Mapping of be\_init\_worker is used

*/

/*
1.1.2 Generic database connection factory

*/
ConnectionGeneric* getAndInitDatabaseConnection(const string &dbType, 
     const string &dbUser, const string &dbPass, 
     const int dbPort, const string &dbName) {

    ConnectionGeneric* connection = nullptr;

    if(ConnectionPostgres::DBTYPE == dbType) {
      connection = new ConnectionPostgres(dbUser, dbPass, dbPort, dbName);
    } else if(ConnectionMySQL::DBTYPE == dbType) {
      connection = new ConnectionMySQL(dbUser, dbPass, dbPort, dbName);
    } else {
      throw SecondoException("Unsupported database type: " + dbType);
    }

    if(connection == nullptr) {
      throw SecondoException("Unable to establish database connection");
    }

    bool connectionResult = connection->createConnection();

    if(! connectionResult) {
      throw SecondoException("Database connection check failed");
    }

    return connection;
}


/*
1.15 Implementation of the Algebra

*/
class BasicEngineAlgebra : public Algebra
{
 public:
  BasicEngineAlgebra() : Algebra()
  {
    AddOperator(&be_init_op);
    be_init_op.SetUsesArgsInTypeMapping();
    AddOperator(&be_init_cluster_op);
    be_init_cluster_op.SetUsesArgsInTypeMapping();
    AddOperator(&be_shutdown);
    AddOperator(&be_shutdown_cluster);
    AddOperator(&be_queryOp);
    AddOperator(&be_commandOp);
    AddOperator(&be_copyOp);
    AddOperator(&be_mqueryOp);
    AddOperator(&be_mcommandOp);
    AddOperator(&be_unionOp);
    AddOperator(&be_structOp);
    AddOperator(&be_runsqlOp);
    AddOperator(&be_collect_op);
    be_collect_op.SetUsesArgsInTypeMapping();

    AddOperator(&be_partRandomOp);
    AddOperator(&be_partRROp);
    AddOperator(&be_partHashOp);
    AddOperator(&be_partGridOp);
    AddOperator(&be_partFunOp);
    AddOperator(&be_repartRandomOp);
    AddOperator(&be_repartRROp);
    AddOperator(&be_repartHashOp);
    AddOperator(&be_repartGridOp);
    AddOperator(&be_repartFunOp);

    AddOperator(&be_shareOp);
    AddOperator(&be_validateQueryOp);
    AddOperator(&be_gridCreateOp);
    AddOperator(&be_gridDeleteOp);

    // configure boost logger
    // TODO: Move to SECONDO core
    boost::log::core::get()->set_filter
    (
         boost::log::trivial::severity >= boost::log::trivial::debug
//         boost::log::trivial::severity >= boost::log::trivial::info
    );
  }

  ~BasicEngineAlgebra() {

    if(be_control != nullptr) {

      if(be_control->isMaster()) {
        be_control->shutdownWorker();
      }

      delete be_control;
      be_control = nullptr;
    }
  };
};

} // end of namespace BasicEngine

/*
1.16 Initialization

*/
extern "C"
Algebra*
InitializeBasicEngineAlgebra( NestedList* nlRef, QueryProcessor* qpRef )
{
  return (new BasicEngine::BasicEngineAlgebra);
}
