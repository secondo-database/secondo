/*
----
This file is part of SECONDO.

Copyright (C) 2022,
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

#ifndef BE_INIT_CLUSTER_H
#define BE_INIT_CLUSTER_H

#include "StandardTypes.h"

namespace BasicEngine {

ListExpr be_init_cluster_tm(ListExpr args);

/*
1.12.2 Value Mapping

*/
template <class T>
int init_be_workerSFVM(Word *args, Word &result, int message, Word &local,
                       Supplier s) {

  T *dbtype = (T *)args[0].addr;
  T *dbUser = (T *)args[1].addr;
  T *dbPass = (T *)args[2].addr;
  CcInt *port = (CcInt *)args[3].addr;
  T *dbname = (T *)args[4].addr;
  Relation *worker = (Relation *)args[5].addr;
  FText *workerRelationName = (FText *)args[6].addr;

  result = qp->ResultStorage(s);

  if (be_control != nullptr) {
    std::cerr << "Error: Basic engine is already initialized. "
              << "Please shutdown first, using be_shutdown_cluster()."
              << std::endl
              << std::endl;

    ((CcBool *)result.addr)->Set(true, false);
    return 0;
  }

  if (!dbtype->IsDefined()) {
    std::cerr << "Error: Database type is undefined" << std::endl;
    ((CcBool *)result.addr)->Set(true, false);
    return 0;
  }

  if (!dbUser->IsDefined()) {
    std::cerr << "Error: DBUser is undefined" << std::endl;
    ((CcBool *)result.addr)->Set(true, false);
    return 0;
  }

  if (!dbPass->IsDefined()) {
    std::cerr << "Error: DBPass is undefined" << std::endl;
    ((CcBool *)result.addr)->Set(true, false);
    return 0;
  }

  if (!port->IsDefined()) {
    std::cerr << "Error: Port is undefined" << std::endl;
    ((CcBool *)result.addr)->Set(true, false);
    return 0;
  }

  if (!dbname->IsDefined()) {
    std::cerr << "Error: DBName is undefined" << std::endl;
    ((CcBool *)result.addr)->Set(true, false);
    return 0;
  }

  if (!workerRelationName->IsDefined()) {
    std::cerr << "Error: Worker relation name is undefined" << std::endl;
    ((CcBool *)result.addr)->Set(true, false);
    return 0;
  }

  std::string dbUserValue = dbUser->toText();
  std::string dbPassValue = dbPass->toText();
  int portValue = port->GetIntval();
  std::string dbNameValue = dbname->toText();
  std::string dbTypeValue = dbtype->toText();
  std::string workerRelationNameValue = workerRelationName->toText();

  try {
    ConnectionGeneric *dbConnection = getAndInitDatabaseConnection(
        dbTypeValue, dbUserValue, dbPassValue, portValue, dbNameValue);

    be_control = new BasicEngineControl(dbConnection, worker,
                                        workerRelationNameValue, true);

    bool createConnectionResult = be_control->createAllConnections();

    if (!createConnectionResult) {
      std::cerr << "Error: Connection error, please check the previous messages"
                << " for error messages." << std::endl
                << endl;
      ((CcBool *)result.addr)->Set(true, false);
      return 0;
    }

    bool connectionsAvailable = be_control->checkAllConnections();

    if (!connectionsAvailable) {
      std::cerr << "Error: Not all connections available, please check the"
                << " previous messages for error messages." << std::endl
                << std::endl;
      ((CcBool *)result.addr)->Set(true, false);
      return 0;
    }

    ((CcBool *)result.addr)->Set(true, true);
    return 0;
  } catch (SecondoException &e) {
    BOOST_LOG_TRIVIAL(error) << "Got error while init connections " << e.what();
    ((CcBool *)result.addr)->Set(true, false);
  }

  return 0;
}

/*
1.12.3 Specification

*/
OperatorSpec be_init_cluster_spec(
    "{string, text} x {string, text}  x {string, text} "
    "x int x {string, text} x rel --> bool",
    "be_init_cluster(_,_,_,_,_,_)",
    "Set the dbtype, user, pass, port and the db-name for initialization the "
    "local BE-Worker. Additional you have to specified a Workers-Relation with "
    "all connection information from the worker, including the information "
    "about the second DBMS. The structure of this relation should be "
    "[Host: string, Port: int, Config: string, PGPort: int, DBName: string]",
    "query be_init_cluster('pgsql','user','pass',5432,'gisdb',WorkersPG)");

/*
1.12.4 ValueMapping Array

*/
ValueMapping be_init_cluster_vm[] = {
    init_be_workerSFVM<CcString>,
    init_be_workerSFVM<FText>,
};

/*
1.12.5 Selection Function

*/
int be_init_cluster_select(ListExpr args) {
  return CcString::checkType(nl->First(args)) ? 0 : 1;
}

/*
1.12.6 Operator instance

*/
Operator be_init_cluster_op("be_init_cluster", be_init_cluster_spec.getStr(),
                            sizeof(be_init_cluster_vm), be_init_cluster_vm,
                            be_init_cluster_select, be_init_cluster_tm);

} // namespace BasicEngine

#endif