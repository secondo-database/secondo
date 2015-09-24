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

namespace KVS {

extern KeyValueStoreIPC* kvsIPC;

/* **********************
 * Operator: kvsStartAppTM()
 *
 */

ListExpr kvsStartAppTM(ListExpr args) {
  return listutils::basicSymbol<CcBool>();
}

int kvsStartAppVM(Word* args, Word& result, int message, Word& local,
                  Supplier s) {
  result = qp->ResultStorage(s);
  CcBool* res = static_cast<CcBool*>(result.addr);

  string kvsAppPath = Application::Instance()->GetApplicationPath() +
                      PATH_SLASH + kvsIPC->getAppName();

  res->Set(true, kvsIPC->connect(kvsAppPath));
  return 0;
}

/* **********************
 * Operator: kvsTransferId()
 *
 */

ListExpr kvsTransferIdTM(ListExpr args) {
  return listutils::basicSymbol<CcInt>();
}

int kvsTransferIdVM(Word* args, Word& result, int message, Word& local,
                    Supplier s) {
  result = qp->ResultStorage(s);
  CcInt* res = static_cast<CcInt*>(result.addr);

  res->Set(kvsIPC->getTransferId());

  return 0;
}

/* **********************
 * Operator: kvsGlobalId()
 *
 */

ListExpr kvsGlobalIdTM(ListExpr args) {
  return listutils::basicSymbol<CcInt>();
}

int kvsGlobalIdVM(Word* args, Word& result, int message, Word& local,
                  Supplier s) {
  result = qp->ResultStorage(s);
  CcInt* res = static_cast<CcInt*>(result.addr);

  res->Set(kvsIPC->getGlobalTupelId());

  return 0;
}

/* **********************
 * Operator: kvsInitClients(host, interfacePort, kvsPort)
 *
 */

ListExpr kvsInitClientsTM(ListExpr args) {
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

int kvsInitClientsVM(Word* args, Word& result, int message, Word& local,
                     Supplier s) {
  FText* host = static_cast<FText*>(args[0].addr);
  CcInt* interfacePort = static_cast<CcInt*>(args[1].addr);
  CcInt* kvsPort = static_cast<CcInt*>(args[2].addr);

  result = qp->ResultStorage(s);
  CcBool* res = static_cast<CcBool*>(result.addr);

  res->Set(true,
           kvsIPC->initClients(host->GetValue(), interfacePort->GetIntval(),
                               kvsPort->GetIntval()));
  return 0;
}

/* **********************
 * Operator: kvsStartClient(port)
 *
 */

ListExpr kvsStartClientTM(ListExpr args) {
  // string err = "text x int = (host, port) expected";

  if (!nl->HasLength(args, 1)) {
    return listutils::typeError("1 argument expected. [int = (port)].");
  }

  if (!CcInt::checkType(nl->First(args))) {
    return listutils::typeError("1st argument should be int. [int = (port)].");
  }

  return listutils::basicSymbol<CcBool>();
}

int kvsStartClientVM(Word* args, Word& result, int message, Word& local,
                     Supplier s) {
  CcInt* port = static_cast<CcInt*>(args[0].addr);

  result = qp->ResultStorage(s);
  CcBool* res = static_cast<CcBool*>(result.addr);

  res->Set(true, kvsIPC->startClient(port->GetIntval()));
  return 0;
}

/* **********************
 * Operator: kvsStopClient(port)
 *
 */
ListExpr kvsStopClientTM(ListExpr args) {
  // string err = "text x int = (host, port) expected";

  if (!nl->HasLength(args, 1)) {
    return listutils::typeError("1 argument expected. [int = (port)].");
  }

  if (!CcInt::checkType(nl->First(args))) {
    return listutils::typeError("1st argument should be int. [int = (port)].");
  }

  return listutils::basicSymbol<CcBool>();
}

int kvsStopClientVM(Word* args, Word& result, int message, Word& local,
                    Supplier s) {
  CcInt* port = static_cast<CcInt*>(args[0].addr);

  result = qp->ResultStorage(s);
  CcBool* res = static_cast<CcBool*>(result.addr);

  res->Set(true, kvsIPC->stopClient(port->GetIntval()));

  return 0;
}

/* **********************
 * Operator: kvsIndex(id)
 *
 */

ListExpr kvsSetIdTM(ListExpr args) {
  if (!nl->HasLength(args, 1)) {
    return listutils::typeError("1 argument expected. [int = (id)].");
  }

  if (!CcInt::checkType(nl->First(args))) {
    return listutils::typeError("1st argument should be int. [int = (id)].");
  }

  return listutils::basicSymbol<CcBool>();
}

int kvsSetIdVM(Word* args, Word& result, int message, Word& local, Supplier s) {
  CcInt* id = static_cast<CcInt*>(args[0].addr);

  result = qp->ResultStorage(s);
  CcBool* res = static_cast<CcBool*>(result.addr);

  res->Set(true, kvsIPC->setId(id->GetIntval()));
  return 0;
}

/* **********************
 * Operator: kvsSetMaster(host, interfacePort, kvsPort)
 *
 */

ListExpr kvsSetMasterTM(ListExpr args) {
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

int kvsSetMasterVM(Word* args, Word& result, int message, Word& local,
                   Supplier s) {
  FText* host = static_cast<FText*>(args[0].addr);
  CcInt* interfacePort = static_cast<CcInt*>(args[1].addr);
  CcInt* kvsPort = static_cast<CcInt*>(args[2].addr);

  result = qp->ResultStorage(s);
  CcBool* res = static_cast<CcBool*>(result.addr);

  res->Set(true, kvsIPC->setMaster(host->GetValue(), interfacePort->GetIntval(),
                                   kvsPort->GetIntval()));
  return 0;
}

/* **********************
 * Operator:  kvsRetrieve(serverId)
 *        kvsRetrieve(serverId [, allowedId's])
 *        kvsRetrieve(distribution, region (attribute name that refers to a
 * region?) [,allowedId's])
 *
 */

ListExpr kvsRetrieveTM(ListExpr args) {
  ListExpr attr_desc;
  ListExpr dist_name;

  string distributionName("");

  if (nl->HasLength(args, 2)) {
    ListExpr firstType = nl->First(nl->First(args));
    if (!Stream<Tuple>::checkType(firstType)) {
      return listutils::typeError(
          "1st argument should be stream (tuple (...)). [stream (tuple (...)) "
          "x symbol = _ (serverIdAttribute)]");
    }

    ListExpr secondType = nl->First(nl->Second(args));
    if (nl->AtomType(secondType) != SymbolType) {
      return listutils::typeError(
          "2nd argument should be symbol. [stream (tuple (...)) x symbol = _ "
          "(serverIdAttribute)]");
    }

    attr_desc = secondType;
    dist_name = nl->TextAtom("");

  } else if (nl->HasLength(args, 3)) {
    ListExpr firstType = nl->First(nl->First(args));
    if (!Stream<Tuple>::checkType(firstType)) {
      return listutils::typeError(
          "1st argument should be stream (tuple (...)). [stream (tuple (...)) "
          "x {distribution, text} x symbol = _ ({distribution, "
          "distributionName}, regionAttribute]");
    }

    ListExpr secondType = nl->First(nl->Second(args));
    if (!FText::checkType(secondType) && !Distribution::checkType(secondType)) {
      return listutils::typeError(
          "2nd argument should be distribution OR text. [stream (tuple (...)) "
          "x {distribution, text} x symbol = _ ({distribution, "
          "distributionName}, regionAttribute]");
    }

    ListExpr thirdType = nl->First(nl->Third(args));
    if (nl->AtomType(thirdType) != SymbolType) {
      return listutils::typeError(
          "3rd argument should be symbol. [stream (tuple (...)) x "
          "{distribution, text} x symbol = _ ({distribution, "
          "distributionName}, regionAttribute]");
    }

    attr_desc = thirdType;

    if (FText::checkType(secondType)) {
      dist_name = nl->Second(nl->Second(args));  // ((text 'distname')...)
    } else {
      dist_name = nl->TextAtom(nl->SymbolValue(
          nl->Second(nl->Second(args))));  // ( (..) (distribution distname))
    }

  } else {
    return listutils::typeError(
        "2 OR 3 arguments expected. [stream (tuple (...)) x {symbol, "
        "distribution, text} [x symbol] = _ ({serverIdAttribute, distribution, "
        "distributionName} [,region] )].");
  }

  string attr_name = nl->SymbolValue(attr_desc);

  // cout<<"Debug:"<<nl->ToString(args)<<endl;
  // Debug:(((stream (tuple ((Osm_id string) (Name string) (Type string)
  // (GeoData region) (ServerId int)))) (extendstream (head (feed Buildings) 1)
  // ((ServerId (fun (tuple1 TUPLE) (kvsServerId testdist4 (bbox (attr tuple1
  // GeoData)) TRUE)))))) (ServerId ServerId))

  ListExpr tuple_desc = nl->Second(nl->First(nl->First(args)));
  if (nl->IsEqual(nl->First(tuple_desc), Tuple::BasicType()) &&
      nl->ListLength(tuple_desc) == 2) {
    ListExpr attrL = nl->Second(tuple_desc);

    // cout<<"tuple_desc: "<<nl->ToString(tuple_desc)<<"\n";
    // cout<<"attrL:"<<nl->ToString(attrL)<<"\n";

    if (IsTupleDescription(attrL)) {
      int attrIndex;
      ListExpr attrType;

      attrIndex = FindAttribute(attrL, attr_name, attrType);

      if (attrIndex > 0) {
        return nl->ThreeElemList(
            nl->SymbolAtom(Symbol::APPEND()),
            nl->TwoElemList(nl->IntAtom(attrIndex), dist_name),
            nl->First(nl->First(args)));  // return input stream

      } else {
        return listutils::typeError(
            "input not as expected: Attribute not found in tuple...");
      }
    } else {
      return listutils::typeError(
          "input not as expected: Tuple description not recognized...");
    }
  } else {
    return listutils::typeError(
        "input not as expected: TupleType not recognized...");
  }
}

int kvsRetrieveSelect(ListExpr args) {
  if (nl->HasLength(args, 2)) {
    return 0;
  } else {
    return 1;
  }
}

int kvsRetrieveByIdVM(Word* args, Word& result, int message, Word& local,
                      Supplier s) {
  class RetrieveLocal {
   public:
    RetrieveLocal(Address streamAddr)
        : inTupleStream(streamAddr), lastUpdate(time(NULL)) {}

    Stream<Tuple> inTupleStream;
    time_t lastUpdate;
    const double interval = 10;  // in seconds
  };

  RetrieveLocal* retrieve = static_cast<RetrieveLocal*>(local.addr);

  switch (message) {
    case OPEN: {
      delete retrieve;

      // connect to master
      //-1 = error, 0 = occupied (currently restructuring), 1 = success
      int requestResult = kvsIPC->tryRestructureLock();

      while (requestResult == 0) {
        boost::this_thread::sleep(boost::posix_time::milliseconds(500));
        requestResult = kvsIPC->tryRestructureLock();
      }

      if (requestResult == 1) {
        retrieve = new RetrieveLocal(args[0].addr);
        retrieve->inTupleStream.open();

        local.addr = retrieve;
        return 0;
      } else {
        local.addr = 0;
        return CANCEL;
      }
    }
    case REQUEST: {
      if (retrieve != 0) {
        int attrIndex = ((CcInt*)(args[2].addr))->GetIntval() - 1;
        Tuple* tuple;

        if (difftime(time(NULL), retrieve->lastUpdate) > retrieve->interval) {
          kvsIPC->updateRestructureLock();
          time(&retrieve->lastUpdate);
        }

        while ((tuple = retrieve->inTupleStream.request()) != 0) {
          int serverId =
              static_cast<CcInt*>(tuple->GetAttribute(attrIndex))->GetIntval();
          if (serverId == kvsIPC->localId) {
            // result = SetWord(tuple);
            result.addr = tuple;
            return YIELD;
          }
        }

        kvsIPC->unlockRestructureLock();
        retrieve->inTupleStream.close();

        delete retrieve;
        local.addr = 0;
      }
      result.addr = 0;
      return CANCEL;
    }
    case CLOSE: {
      delete retrieve;
      local.addr = 0;
    }
      return 0;
  }
  return 0;
}

int kvsRetrieveByRegionVM(Word* args, Word& result, int message, Word& local,
                          Supplier s) {
  class RetrieveLocal {
   public:
    RetrieveLocal(Address streamAddr, int distRef)
        : distRef(distRef), inTupleStream(streamAddr), lastUpdate(time(NULL)) {}

    int distRef;
    Stream<Tuple> inTupleStream;
    time_t lastUpdate;
    const double interval = 10;  // in seconds
  };

  RetrieveLocal* retrieve = static_cast<RetrieveLocal*>(local.addr);

  switch (message) {
    case OPEN: {
      delete retrieve;

      // Distribution:
      string distributionName = static_cast<FText*>(args[4].addr)->GetValue();
      int distRef = kvsIPC->getDistributionRef(distributionName);
      if (distRef < 0) {
        ListExpr distributionType = qp->GetSupplierTypeExpr(qp->GetSon(s, 1));
        if (Distribution::checkType(distributionType)) {
          Distribution* dist = static_cast<Distribution*>(args[1].addr);

          distRef = kvsIPC->getDistributionRef(distributionName, dist->type,
                                               dist->toBin());
        } else {
          cout << "Error: No Distribution specified";
          return CANCEL;
        }
      }

      // connect to master
      //-1 = error, 0 = occupied (currently restructuring), 1 = success
      int requestResult = kvsIPC->tryRestructureLock();

      while (requestResult == 0) {
        boost::this_thread::sleep(boost::posix_time::milliseconds(500));
        requestResult = kvsIPC->tryRestructureLock();
      }

      if (requestResult == 1 && distRef >= 0) {
        retrieve = new RetrieveLocal(args[0].addr, distRef);
        retrieve->inTupleStream.open();

        local.addr = retrieve;
        return 0;
      } else {
        local.addr = 0;
        return CANCEL;
      }
    }
    case REQUEST: {
      if (retrieve != 0) {
        int attrIndex = ((CcInt*)(args[3].addr))->GetIntval() - 1;
        Tuple* tuple;

        if (difftime(time(NULL), retrieve->lastUpdate) > retrieve->interval) {
          kvsIPC->updateRestructureLock();
          time(&retrieve->lastUpdate);
        }

        while ((tuple = retrieve->inTupleStream.request()) != 0) {
          const Rectangle<2> mbb =
              static_cast<Region*>(tuple->GetAttribute(attrIndex))
                  ->BoundingBox();

          double coords[4];

          coords[0] = mbb.MinD(0) * 1000;
          coords[1] = mbb.MinD(1) * 1000;
          coords[2] = mbb.MaxD(0) * 1000;
          coords[3] = mbb.MaxD(1) * 1000;

          set<int> results;
          if (kvsIPC->qtDistRequest(retrieve->distRef, 4, coords, &results)) {
            if (results.find(kvsIPC->localId) != results.end()) {
              result.addr = tuple;
              return YIELD;
            }
          }
        }

        kvsIPC->unlockRestructureLock();
        retrieve->inTupleStream.close();

        delete retrieve;
        local.addr = 0;
      }
      result.addr = 0;
      return CANCEL;
    }
    case CLOSE: {
      delete retrieve;
      local.addr = 0;
    }
      return 0;
  }
  return 0;
}

ListExpr kvsExecTM(ListExpr args) {
  if (!nl->HasLength(args, 1)) {
    return listutils::typeError("1 arguments expected. [text = (command)].");
  }

  if (!FText::checkType(nl->First(args))) {
    return listutils::typeError(
        "1st argument should be text. [text = (command)].");
  }

  return listutils::basicSymbol<CcBool>();
}

int kvsExecVM(Word* args, Word& result, int message, Word& local, Supplier s) {
  FText* commandText = static_cast<FText*>(args[0].addr);

  result = qp->ResultStorage(s);
  CcBool* res = static_cast<CcBool*>(result.addr);

  res->Set(true, kvsIPC->execCommand(commandText->GetValue()));

  return 0;
}
}
