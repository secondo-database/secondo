/*
----
This file is part of SECONDO.

Copyright (C) 2015,
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

*/

#include "AlgebraOperators.h"

using namespace std;

namespace KVS {

extern KeyValueStoreIPC* kvsIPC;

/* **********************
 * Operator: kvsAdd(ip, port)
 *
 */

ListExpr kvsAddTM(ListExpr args) {
  if (!nl->HasLength(args, 3)) {
    return listutils::typeError(
        "3 arguments expected. [text x int x int = (host, interfacePort, "
        "kvsPort)].");
  }

  if (!FText::checkType(nl->First(args))) {
    return listutils::typeError(
        "1st argument should be text. [text x int x int = (host, "
        "interfacePort, kvsPort)].");
  }

  if (!CcInt::checkType(nl->Second(args))) {
    return listutils::typeError(
        "2nd argument should be int. [text x int x int = (host, interfacePort, "
        "kvsPort)].");
  }

  if (!CcInt::checkType(nl->Third(args))) {
    return listutils::typeError(
        "3rd argument should be int. [text x int x int = (host, interfacePort, "
        "kvsPort)].");
  }

  return listutils::basicSymbol<CcBool>();
}

int kvsAddVM(Word* args, Word& result, int message, Word& local, Supplier s) {
  FText* host = static_cast<FText*>(args[0].addr);
  CcInt* interfacePort = static_cast<CcInt*>(args[1].addr);
  CcInt* kvsPort = static_cast<CcInt*>(args[2].addr);

  result = qp->ResultStorage(s);
  CcBool* res = static_cast<CcBool*>(result.addr);

  if (interfacePort->GetIntval() < 1024 || interfacePort->GetIntval() > 65535 ||
      kvsPort->GetIntval() < 1024 || kvsPort->GetIntval() > 65535) {
    res->Set(true, false);
    return 0;
  } else {
    res->Set(true,
             kvsIPC->addConnection(host->GetValue(), interfacePort->GetIntval(),
                                   kvsPort->GetIntval(), "SecondoConfig.ini"));
    return 0;
  }
}

/* **********************
 * Operator: kvsRemove(idx)
 *
 */

ListExpr kvsRemoveTM(ListExpr args) {
  if (!nl->HasLength(args, 1)) {
    return listutils::typeError("1 argument expected. [int = (idx)].");
  }

  if (!CcInt::checkType(nl->First(args))) {
    return listutils::typeError("1st argument should be int. [int = (idx)].");
  }

  return listutils::basicSymbol<CcBool>();
}

int kvsRemoveVM(Word* args, Word& result, int message, Word& local,
                Supplier s) {
  CcInt* idx = static_cast<CcInt*>(args[0].addr);

  result = qp->ResultStorage(s);
  CcBool* res = static_cast<CcBool*>(result.addr);

  res->Set(true, kvsIPC->removeConnection(idx->GetIntval()));

  return 0;
}

/* **********************
 * Operator: kvsUpdateServerList( separatedList )
 *
 */

ListExpr kvsUpdateServerListTM(ListExpr args) {
  if (!nl->HasLength(args, 1)) {
    return listutils::typeError(
        "1 argument expected. [text = (separatedList)].");
  }

  if (!FText::checkType(nl->First(args))) {
    return listutils::typeError(
        "1st argument should be text. [text = (separatedList)].");
  }

  return listutils::basicSymbol<CcBool>();
}

int kvsUpdateServerListVM(Word* args, Word& result, int message, Word& local,
                          Supplier s) {
  FText* separatedList = static_cast<FText*>(args[0].addr);

  result = qp->ResultStorage(s);
  CcBool* res = static_cast<CcBool*>(result.addr);

  res->Set(true, kvsIPC->updateServerList(separatedList->GetValue()));
  return 0;
}

/* **********************
 * Operator: kvsSyncServerList( )
 *
 */

ListExpr kvsSyncServerListTM(ListExpr args) {
  return listutils::basicSymbol<CcBool>();
}

int kvsSyncServerListVM(Word* args, Word& result, int message, Word& local,
                        Supplier s) {
  result = qp->ResultStorage(s);
  CcBool* res = static_cast<CcBool*>(result.addr);

  res->Set(true, kvsIPC->syncServerList());
  return 0;
}

/* **********************
 * Operator: kvsReconnect(idx)
 *
 */

ListExpr kvsReconnectTM(ListExpr args) {
  if (!nl->HasLength(args, 1)) {
    return listutils::typeError("1 argument expected. [int = (idx)].");
  }

  if (!CcInt::checkType(nl->First(args))) {
    return listutils::typeError("1st argument should be int. [int = (idx)].");
  }

  return listutils::basicSymbol<CcBool>();
}

int kvsReconnectVM(Word* args, Word& result, int message, Word& local,
                   Supplier s) {
  CcInt* idx = static_cast<CcInt*>(args[0].addr);

  result = qp->ResultStorage(s);
  CcBool* res = static_cast<CcBool*>(result.addr);

  res->Set(true, kvsIPC->retryConnection(idx->GetIntval()));

  return 0;
}

/* **********************
 * Operator: kvsList( )
 *
 */

ListExpr kvsListTM(ListExpr args) { return listutils::basicSymbol<CcBool>(); }

int kvsListVM(Word* args, Word& result, int message, Word& local, Supplier s) {
  result = qp->ResultStorage(s);
  CcBool* res = static_cast<CcBool*>(result.addr);

  string serverInformation = kvsIPC->getInformationString();
  cout << serverInformation;

  res->Set(true, true);

  return 0;
}

/* **********************
 * Operator: kvsSetDatabase( databaseName )
 *
 */

ListExpr kvsSetDatabaseTM(ListExpr args) {
  if (!nl->HasLength(args, 1)) {
    return listutils::typeError(
        "1 argument expected. [text = (databaseName)].");
  }

  if (!FText::checkType(nl->First(args))) {
    return listutils::typeError(
        "1st argument should be text. [text = (databaseName)].");
  }

  return listutils::basicSymbol<CcBool>();
}

int kvsSetDatabaseVM(Word* args, Word& result, int message, Word& local,
                     Supplier s) {
  FText* databaseName = static_cast<FText*>(args[0].addr);

  result = qp->ResultStorage(s);
  CcBool* res = static_cast<CcBool*>(result.addr);

  res->Set(true, kvsIPC->setDatabase(databaseName->GetValue()));

  return 0;
}

/* **********************
 * Operator: kvsUseDatabase( databaseName )
 *
 */

ListExpr kvsUseDatabaseTM(ListExpr args) {
  if (!nl->HasLength(args, 1)) {
    return listutils::typeError(
        "1 argument expected. [text = (databaseName)].");
  }

  if (!FText::checkType(nl->First(args))) {
    return listutils::typeError(
        "1st argument should be text. [text = (databaseName)].");
  }

  return listutils::basicSymbol<CcBool>();
}

int kvsUseDatabaseVM(Word* args, Word& result, int message, Word& local,
                     Supplier s) {
  FText* databaseName = static_cast<FText*>(args[0].addr);

  result = qp->ResultStorage(s);
  CcBool* res = static_cast<CcBool*>(result.addr);

  string statusMessage = kvsIPC->useDatabase(databaseName->GetValue());
  cout << statusMessage;
  res->Set(true, true);

  return 0;
}
}
