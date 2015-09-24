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
 * Operator: qtcreatedist
 *
 */

ListExpr qtcreatedistTM(ListExpr inargs) {
  NList args(inargs);
  if (args.length() == 3) {
    cout << "1:" << args.second().str() << " - "
         << args.second().isSymbol(CcString::BasicType()) << " - "
         << args.second().isAtom() << " - " << args.second().isEmpty() << " - "
         << args.second().length() << "\n";
    cout << "2:" << args.third().str() << "\n";

    if (args.third().isSymbol(QuadTreeDistributionType::BasicType())) {
      NList stream_desc = args.first();
      ListExpr attr_desc = args.second().listExpr();

      cout << "stream_desc: " << nl->ToString(stream_desc.listExpr()) << "\n";

      if (stream_desc.isList() &&
          stream_desc.first().isSymbol(Symbol::STREAM()) &&
          (stream_desc.length() == 2) &&
          (nl->AtomType(attr_desc) == SymbolType)) {
        ListExpr tuple_desc = stream_desc.second().listExpr();
        string attr_name = nl->SymbolValue(attr_desc);

        cout << "tuple_desc: " << nl->ToString(tuple_desc) << "\n";

        if (nl->IsEqual(nl->First(tuple_desc), Tuple::BasicType()) &&
            nl->ListLength(tuple_desc) == 2) {
          ListExpr attrL = nl->Second(tuple_desc);

          if (IsTupleDescription(attrL)) {
            int attrIndex;
            ListExpr attrType;

            attrIndex = FindAttribute(attrL, attr_name, attrType);

            if (attrIndex > 0) {
              return nl->ThreeElemList(
                  nl->SymbolAtom(Symbol::APPEND()),
                  nl->OneElemList(nl->IntAtom(attrIndex)),
                  nl->SymbolAtom(QuadTreeDistributionType::BasicType()));
              // NList(QuadTreeDistributionType::BasicType()).listExpr());
            } else {
              return args.typeError(
                  "input not as expected: Attribute not found in tuple...");
            }
          } else {
            return args.typeError(
                "input not as expected: unexpected stream / tuple format");
          }
        } else {
          return args.typeError(
              "input not as expected: unexpected tuple format");
        }
      }
    }
  }

  return args.typeError("input is not (stream(tuple(y))) x ...");
}

int qtcreatedistVM(Word* args, Word& result, int message, Word& local,
                   Supplier s) {
  QuadTreeDistributionType* baseQtd =
      static_cast<QuadTreeDistributionType*>(args[2].addr);
  int attrIndex = ((CcInt*)(args[3].addr))->GetIntval() - 1;

  QuadTreeDistributionType* resultQtd =
      (QuadTreeDistributionType*)(qp->ResultStorage(s)).addr;

  resultQtd->init(baseQtd);

  Stream<Tuple> inTupleStream(args[0]);
  inTupleStream.open();

  Tuple* tuple;

  cout << "Reading Data ..." << endl;

  double coords[4];

  while ((tuple = inTupleStream.request()) != 0) {
    Rectangle<2>* mbb =
        static_cast<Rectangle<2>*>(tuple->GetAttribute(attrIndex));

    coords[0] = mbb->MinD(0) * 1000;
    coords[1] = mbb->MinD(1) * 1000;
    coords[2] = mbb->MaxD(0) * 1000;
    coords[3] = mbb->MaxD(1) * 1000;

    // cout<<"MBB: x:"<<mbb->MinD(0)<<" y:"<<mbb->MinD(1)<<"
    // w:"<<mbb->MaxD(0)-mbb->MinD(0)<<" h:"<<mbb->MaxD(1)-mbb->MinD(1)<<"\n";

    // initialized?
    if (resultQtd->root == 0) {
      resultQtd->root =
          new QuadNode(mbb->MinD(0), mbb->MinD(1), resultQtd->initialWidth,
                       resultQtd->initialHeight);
    }

    // fully inside tree?
    resultQtd->expand(coords);

    // result server ids
    set<int> results;

    //
    resultQtd->insert(resultQtd->root, coords, &results);

    delete tuple;
  }

  inTupleStream.close();

  cout << "Reading Data done ..." << endl;

  result.addr = resultQtd;
  return 0;
}

/* **********************
 * Operator: qtserverid
 *
 */

ListExpr qtserveridLocalTM(ListExpr args) {
  NList type(args);
  if (type !=
      NList(Rectangle<2>::BasicType(), QuadTreeDistributionType::BasicType())) {
    return NList::typeError("Expecting rect x qtdistribution");
  }

  return NList(Symbol::STREAM(), CcInt::BasicType()).listExpr();
}

int qtserveridLocalVM(Word* args, Word& result, int message, Word& local,
                      Supplier s) {
  struct ServerIdResult {
    set<int>::iterator iter;
    set<int> outputServerIds;
  };

  ServerIdResult* res = static_cast<ServerIdResult*>(local.addr);

  switch (message) {
    case OPEN: {
      cout << "qtserverid: OPEN\n";
      delete res;
      res = new ServerIdResult;

      Rectangle<2>* mbb = (Rectangle<2>*)args[0].addr;
      QuadTreeDistributionType* qtd =
          static_cast<QuadTreeDistributionType*>(args[1].addr);

      set<int>* outputSet = &res->outputServerIds;

      double coords[4];

      coords[0] = mbb->MinD(0) * 1000;
      coords[1] = mbb->MinD(1) * 1000;
      coords[2] = mbb->MaxD(0) * 1000;
      coords[3] = mbb->MaxD(1) * 1000;

      if (qtd->root == 0) {
        qtd->root = new QuadNode(mbb->MinD(0), mbb->MinD(1), qtd->initialWidth,
                                 qtd->initialHeight);
      }

      qtd->expand(coords);
      qtd->insert(qtd->root, coords, outputSet);

      qp->SetModified(qp->GetSon(s, 1));

      /*set<int>* outputSet = &res->outputServerIds;
      propagateDownB(qtd->root,  [outputSet, mbb] (QuadNode* node) -> bool {
        if(node->isOverlapping(mbb) && node->serverId != 0) {
          outputSet->insert(node->serverId);
          return true;
        } else {
          return false;
        }});*/

      if (outputSet->size() > 0) {
        res->iter = res->outputServerIds.begin();
        local.addr = res;
        return 0;
      } else {
        delete res;
        return CANCEL;
      }
    }
    case REQUEST: {
      cout << "qtserverid: REQUEST\n";
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
      cout << "qtserverid: CLOSE\n";
      delete res;
      local.addr = 0;
    }
      return 0;
  }

  return 0;
}

ListExpr qtserveridTM(ListExpr args) {
  if (!nl->HasLength(args, 2)) {
    return listutils::typeError(
        "Expecting 2 arguments [ " + Rectangle<2>::BasicType() +
        " x {distribution, text} = (rect, {distribution, distributionName})].");
  }

  if (!Rectangle<2>::checkType(nl->First(args))) {
    return listutils::typeError(
        "1st argument should be rectangle(bbox). [ " +
        Rectangle<2>::BasicType() +
        " x {distribution, text} = (rect, {distribution, distributionName})].");
  }

  if (!Distribution::checkType(nl->Second(args)) &&
      !FText::checkType(nl->Second(args))) {
    return listutils::typeError(
        "2nd argument should be distribution OR text. [ " +
        Rectangle<2>::BasicType() +
        " x {distribution, text} = (rect, {distribution, distributionName})].");
  }

  ListExpr distname;

  if (FText::checkType(nl->Second(args))) {
    distname = nl->Second(args);
  } else {
    distname = nl->TextAtom(nl->SymbolValue(nl->Second(args)));
  }

  return nl->ThreeElemList(nl->SymbolAtom(Symbol::APPEND()),
                           nl->OneElemList(distname),
                           NList(Symbol::STREAM(),
                                 CcInt::BasicType())
                               .listExpr());  // return int stream
}

int qtserveridVM(Word* args, Word& result, int message, Word& local,
                 Supplier s) {
  struct ServerIdResult {
    set<int>::iterator iter;
    set<int> outputServerIds;
  };

  ServerIdResult* res = static_cast<ServerIdResult*>(local.addr);

  switch (message) {
    case OPEN: {
      string distributionName = static_cast<FText*>(args[2].addr)->GetValue();

      int distRef = kvsIPC->getDistributionRef(distributionName);
      if (distRef < 0) {
        ListExpr distributionType = qp->GetSupplierTypeExpr(qp->GetSon(s, 2));
        if (!FText::checkType(distributionType)) {
          Distribution* dist = static_cast<Distribution*>(args[2].addr);

          distRef = kvsIPC->getDistributionRef(distributionName, dist->type,
                                               dist->toBin());
        } else {
          cout << "Error: No Distribution specified";
          return CANCEL;
        }
      }

      cout << "qtserverid: OPEN\n";
      delete res;
      res = new ServerIdResult;

      Rectangle<2>* mbb = (Rectangle<2>*)args[0].addr;
      set<int>* outputSet = &res->outputServerIds;

      double coords[4];

      coords[0] = mbb->MinD(0) * 1000;
      coords[1] = mbb->MinD(1) * 1000;
      coords[2] = mbb->MaxD(0) * 1000;
      coords[3] = mbb->MaxD(1) * 1000;

      // TODO: Still specific to QuadTree-Distribution. Should be generalized to
      // addMbb / addInt and than have Distributions either have support it or
      // return false
      kvsIPC->qtDistAdd(distRef, 4, coords, outputSet);

      if (outputSet->size() > 0) {
        res->iter = res->outputServerIds.begin();
        local.addr = res;
        return 0;
      } else {
        delete res;
        return CANCEL;
      }
    }
    case REQUEST: {
      cout << "qtserverid: REQUEST\n";
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
      cout << "qtserverid: CLOSE\n";
      delete res;
      local.addr = 0;
    }
      return 0;
  }

  return 0;
}

/* **********************
 * Operator: qtintersectsLocal
 *
 */

ListExpr qtintersectsLocalTM(ListExpr args) {
  if (!nl->HasLength(args, 3)) {
    return listutils::typeError(
        "3 arguments expected. [rect x rect x qtdistribution  = (rectA, rectB, "
        "qtdistribution)].");
  }

  if (!Rectangle<2>::checkType(nl->First(args))) {
    return listutils::typeError(
        "1st argument should be rectangle. [rect x rect x qtdistribution  = "
        "(rectA, rectB, qtdistribution)].");
  }

  if (!Rectangle<2>::checkType(nl->Second(args))) {
    return listutils::typeError(
        "2nd argument should be rectangle. [rect x rect x qtdistribution  = "
        "(rectA, rectB, qtdistribution)].");
  }

  if (!QuadTreeDistributionType::checkType(nl->Third(args))) {
    return listutils::typeError(
        "3rd argument should be distribution. [rect x rect x qtdistribution  = "
        "(rectA, rectB, qtdistribution)].");
  }

  return listutils::basicSymbol<CcBool>();
}

int qtintersectsLocalVM(Word* args, Word& result, int message, Word& local,
                        Supplier s) {
  Rectangle<2>* rectA = static_cast<Rectangle<2>*>(args[0].addr);
  Rectangle<2>* rectB = static_cast<Rectangle<2>*>(args[1].addr);
  QuadTreeDistributionType* qtd =
      static_cast<QuadTreeDistributionType*>(args[2].addr);

  result = qp->ResultStorage(s);
  CcBool* res = static_cast<CcBool*>(result.addr);

  if (!rectA->Intersects(*rectB)) {
    res->Set(true, false);
  } else {
    double min[2];
    double max[2];

    min[0] = rectA->MinD(0) * 1000;
    min[1] = rectA->MinD(1) * 1000;
    max[0] = rectA->MaxD(0) * 1000;
    max[1] = rectA->MaxD(1) * 1000;

    rectA->Set(rectA->IsDefined(), min, max);

    min[0] = rectB->MinD(0) * 1000;
    min[1] = rectB->MinD(1) * 1000;
    max[0] = rectB->MaxD(0) * 1000;
    max[1] = rectB->MaxD(1) * 1000;

    rectB->Set(rectB->IsDefined(), min, max);

    double interx = std::max(rectA->MinD(0), rectB->MinD(0));
    double intery = std::max(rectA->MinD(1), rectB->MinD(1));

    res->Set(true, kvsIPC->localId == qtd->pointId(interx, intery));
  }

  return 0;
}

/* **********************
 * Operator: qtintersects
 *
 */

ListExpr qtintersectsTM(ListExpr args) {
  if (!nl->HasLength(args, 3)) {
    return listutils::typeError(
        "3 arguments expected. [rect x rect x {distribution,text}  = (rectA, "
        "rectB, distribution)].");
  }

  if (!Rectangle<2>::checkType(nl->First(args))) {
    return listutils::typeError(
        "1st argument should be rectangle. [rect x rect x {distribution,text}  "
        "= (rectA, rectB, distribution)].");
  }

  if (!Rectangle<2>::checkType(nl->Second(args))) {
    return listutils::typeError(
        "2nd argument should be rectangle. [rect x rect x {distribution,text}  "
        "= (rectA, rectB, distribution)].");
  }

  if (!Distribution::checkType(nl->Third(args)) &&
      !FText::checkType(nl->Third(args))) {
    return listutils::typeError(
        "3rd argument should be distribution OR text. [rect x rect x "
        "{distribution,text}  = (rectA, rectB, distribution)].");
  }

  ListExpr distName = nl->Second(args);

  return nl->ThreeElemList(
      nl->SymbolAtom(Symbol::APPEND()),
      nl->OneElemList(nl->TextAtom(nl->SymbolValue(distName))),
      listutils::basicSymbol<CcBool>());  // return bool
}

int qtintersectsVM(Word* args, Word& result, int message, Word& local,
                   Supplier s) {
  Rectangle<2>* rectA = static_cast<Rectangle<2>*>(args[0].addr);
  Rectangle<2>* rectB = static_cast<Rectangle<2>*>(args[1].addr);

  result = qp->ResultStorage(s);
  CcBool* res = static_cast<CcBool*>(result.addr);

  if (!rectA->Intersects(*rectB)) {
    res->Set(true, false);
  } else {
    string distributionName = static_cast<FText*>(args[3].addr)->GetValue();

    int distRef = kvsIPC->getDistributionRef(distributionName);
    if (distRef < 0) {
      ListExpr distributionType = qp->GetSupplierTypeExpr(qp->GetSon(s, 2));
      if (!FText::checkType(distributionType)) {
        Distribution* dist = static_cast<Distribution*>(args[2].addr);

        distRef = kvsIPC->getDistributionRef(distributionName, dist->type,
                                             dist->toBin());
      } else {
        cout << "Error: No Distribution specified";
        res->Set(false, false);
        return 0;
      }
    }

    double min[2];
    double max[2];

    min[0] = rectA->MinD(0) * 1000;
    min[1] = rectA->MinD(1) * 1000;
    max[0] = rectA->MaxD(0) * 1000;
    max[1] = rectA->MaxD(1) * 1000;

    rectA->Set(rectA->IsDefined(), min, max);

    min[0] = rectB->MinD(0) * 1000;
    min[1] = rectB->MinD(1) * 1000;
    max[0] = rectB->MaxD(0) * 1000;
    max[1] = rectB->MaxD(1) * 1000;

    rectB->Set(rectB->IsDefined(), min, max);

    double interx = std::max(rectA->MinD(0), rectB->MinD(0));
    double intery = std::max(rectA->MinD(1), rectB->MinD(1));

    res->Set(true,
             kvsIPC->localId == kvsIPC->qtDistPointId(distRef, interx, intery));
  }

  return 0;
}

ListExpr qtDistinctTM(ListExpr args) {
  if (!nl->HasLength(args, 2)) {
    return listutils::typeError(
        "2 arguments expected. [{distribution,text} x rect  = (distribution, "
        "rect)].");
  }

  if (!QuadTreeDistributionType::checkType(nl->First(nl->First(args))) &&
      !FText::checkType(nl->First(nl->First(args)))) {
    return listutils::typeError(
        "1st argument should be qtdistribution OR text.  [{distribution,text} "
        "x rect  = (distribution, rect)].");
  }

  if (!Rectangle<2>::checkType(nl->First(nl->Second(args)))) {
    return listutils::typeError(
        "2nd argument should be rectangle. [{distribution,text} x rect  = "
        "(distribution, rect)].");
  }

  if (!kvsIPC->connected()) {
    return listutils::typeError(
        "IPC-Connection Error: Not connected to Key-Value Store application.");
  }

  ListExpr dist_name;

  if (FText::checkType(nl->First(nl->First(args)))) {
    dist_name = nl->Second(nl->First(args));  // ((text 'distname')...)
  } else {
    dist_name = nl->TextAtom(nl->SymbolValue(
        nl->Second(nl->First(args))));  // ( (..) (distribution distname))
  }

  return nl->ThreeElemList(nl->SymbolAtom(Symbol::APPEND()),
                           nl->OneElemList(dist_name),
                           listutils::basicSymbol<CcBool>());  // return bool
}

int qtDistinctVM(Word* args, Word& result, int message, Word& local,
                 Supplier s) {
  result = qp->ResultStorage(s);
  CcBool* res = static_cast<CcBool*>(result.addr);

  // Distribution:
  string distributionName = static_cast<FText*>(args[2].addr)->GetValue();
  int distRef = kvsIPC->getDistributionRef(distributionName);
  if (distRef < 0) {
    ListExpr distributionType = qp->GetSupplierTypeExpr(qp->GetSon(s, 0));
    if (Distribution::checkType(distributionType)) {
      Distribution* dist = static_cast<Distribution*>(args[0].addr);

      distRef = kvsIPC->getDistributionRef(distributionName, dist->type,
                                           dist->toBin());
    } else {
      cout << "Error: No Distribution specified";
      res->Set(true, false);
      return CANCEL;
    }
  }

  if (distRef >= 0) {
    Rectangle<2>* rect = static_cast<Rectangle<2>*>(args[1].addr);

    double x = rect->MinD(0) * 1000;
    double y = rect->MinD(1) * 1000;

    res->Set(true, kvsIPC->qtDistinct(distRef, x, y));

    return 0;
  } else {
    res->Set(true, false);
    return CANCEL;
  }
}

}
