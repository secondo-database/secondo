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

//  kvsServerId( {distribution, text}, {int, rect} [, requestOnly]  )

ListExpr kvsServerIdTM(ListExpr args) {
  if (!nl->HasLength(args, 2) && !nl->HasLength(args, 3)) {
    return listutils::typeError(
        "Expecting 2 or 3 arguments [ {distribution, text} x " +
        Rectangle<2>::BasicType() + "," + CcInt::BasicType() +
        "} x bool = ({rect, int}, {distribution, distributionName} "
        "[,requestOnly])].");
  }

  ListExpr firstType = nl->First(nl->First(args));
  if (!Distribution::checkType(firstType) && !FText::checkType(firstType)) {
    return listutils::typeError(
        "1st argument should be distribution OR text. [ {distribution, text} "
        "x " +
        Rectangle<2>::BasicType() + "," + CcInt::BasicType() +
        "} x bool = ({rect, int}, {distribution, distributionName} "
        "[,requestOnly])].");
  }

  ListExpr secondType = nl->First(nl->Second(args));
  if (!Rectangle<2>::checkType(secondType) && !CcInt::checkType(secondType)) {
    return listutils::typeError(
        "2nd argument should be rectangle(bbox) OR int. [ {distribution, text} "
        "x " +
        Rectangle<2>::BasicType() + "," + CcInt::BasicType() +
        "} x bool = ({rect, int}, {distribution, distributionName} "
        "[,requestOnly])].");
  }

  if (nl->HasLength(args, 3) &&
      !CcBool::checkType(nl->First(nl->Third(args)))) {
    return listutils::typeError(
        "3rd argument should be bool. [ {distribution, text} x " +
        Rectangle<2>::BasicType() + "," + CcInt::BasicType() +
        "} x bool = ({rect, int}, {distribution, distributionName} "
        "[,requestOnly])].");
  }

  ListExpr distname;

  if (FText::checkType(firstType)) {
    distname = nl->Second(nl->First(args));  // ((text 'distname')...)
  } else {
    distname = nl->TextAtom(nl->SymbolValue(
        nl->Second(nl->First(args))));  // ( (..) (distribution distname))
  }

  return nl->ThreeElemList(nl->SymbolAtom(Symbol::APPEND()),
                           nl->OneElemList(distname),
                           NList(Symbol::STREAM(),
                                 CcInt::BasicType())
                               .listExpr());  // return int stream
}

int kvsServerIdVM(Word* args, Word& result, int message, Word& local,
                  Supplier s) {
  struct ServerIdResult {
    set<int>::iterator iter;
    set<int> outputServerIds;
  };

  ServerIdResult* res = static_cast<ServerIdResult*>(local.addr);

  switch (message) {
    case OPEN: {
      int noSons = qp->GetNoSons(s);

      // Request Only?
      bool requestOnly = false;
      if (noSons == 4) {
        requestOnly = static_cast<CcBool*>(args[noSons - 2].addr)->GetValue();
      }

      // Distribution
      string distributionName =
          static_cast<FText*>(args[noSons - 1].addr)->GetValue();

      int distRef = kvsIPC->getDistributionRef(distributionName);
      if (distRef < 0) {
        ListExpr distributionType = qp->GetSupplierTypeExpr(qp->GetSon(s, 0));
        if (Distribution::checkType(distributionType)) {
          Distribution* dist = static_cast<Distribution*>(args[0].addr);

          distRef = kvsIPC->getDistributionRef(distributionName, dist->type,
                                               dist->toBin());
        } else {
          cout << "Error: No Distribution specified";
          return CANCEL;
        }
      }

      if (distRef >= 0) {
        // Prepare State
        delete res;
        res = new ServerIdResult;

        set<int>* outputSet = &res->outputServerIds;

        // Handle inputs
        ListExpr inputType = qp->GetSupplierTypeExpr(qp->GetSon(s, 1));
        if (Rectangle<2>::checkType(inputType)) {
          Rectangle<2>* mbb = (Rectangle<2>*)args[1].addr;

          double coords[4];
          coords[0] = mbb->MinD(0);
          coords[1] = mbb->MinD(1);
          coords[2] = mbb->MaxD(0);
          coords[3] = mbb->MaxD(1);

          if (kvsIPC->distAddRect(distRef, 4, coords, outputSet, requestOnly)) {
            if (outputSet->size() > 0) {
              res->iter = res->outputServerIds.begin();
              local.addr = res;
              return 0;
            }
          }
        } else if (CcInt::checkType(inputType)) {
          CcInt* val = static_cast<CcInt*>(args[1].addr);

          if (kvsIPC->distAddInt(distRef, val->GetIntval(), outputSet,
                                 requestOnly)) {
            if (outputSet->size() > 0) {
              res->iter = res->outputServerIds.begin();
              local.addr = res;
              return 0;
            }
          }
        }

        cout << "Error: Handling input type failed.\n";
      } else {
        cout << "Error: Distribution couldn't be loaded.\n";
      }

      // failed?
      delete res;
      return CANCEL;
    }
    case REQUEST: {
      if (!res) {
        return CANCEL;
      } else {
        if (res->iter != res->outputServerIds.end()) {
          CcInt* serverId = new CcInt(true, *(res->iter));
          result.addr = serverId;

          res->iter++;
          return YIELD;
        } else {
          result.addr = 0;
          return CANCEL;
        }
      }
    }
    case CLOSE: {
      delete res;
      local.addr = 0;
    }
      return 0;
  }

  return 0;
}

ListExpr kvsSaveDistTM(ListExpr args) {
  if (!nl->HasLength(args, 1)) {
    return listutils::typeError(
        "Expecting 1 argument [ distribution = (distribution)].");
  }

  ListExpr firstType = nl->First(nl->First(args));
  if (!Distribution::checkType(firstType)) {
    return listutils::typeError(
        "1st argument should be distribution [ distribution = "
        "(distribution)].");
  }

  ListExpr distname;
  distname = nl->TextAtom(nl->SymbolValue(
      nl->Second(nl->First(args))));  // ( (..) (distribution distname))

  return nl->ThreeElemList(nl->SymbolAtom(Symbol::APPEND()),
                           nl->OneElemList(distname),
                           listutils::basicSymbol<CcBool>());
}

int kvsSaveDistVM(Word* args, Word& result, int message, Word& local,
                  Supplier s) {
  result = qp->ResultStorage(s);
  CcBool* res = static_cast<CcBool*>(result.addr);

  // Distribution
  Distribution* dist = static_cast<Distribution*>(args[0].addr);

  int noSons = qp->GetNoSons(s);
  string distributionName =
      static_cast<FText*>(args[noSons - 1].addr)->GetValue();

  int distRef = kvsIPC->getDistributionRef(distributionName);
  if (distRef < 0) {
    distRef =
        kvsIPC->getDistributionRef(distributionName, dist->type, dist->toBin());
  }

  if (distRef >= 0) {
    string data;

    if (kvsIPC->getDistributionData(distRef, &data)) {
      if (dist->fromBin(data)) {
        qp->SetModified(qp->GetSon(s, 0));
        res->Set(true, true);

        return 0;
      }
    }
  }

  cout << "Error: Couldn't load distribution.\n";
  res->Set(true, false);

  return 0;
}
}
