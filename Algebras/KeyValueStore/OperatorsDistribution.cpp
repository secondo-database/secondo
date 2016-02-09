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

  if (!kvsIPC->connected()) {
    return listutils::typeError(
        "IPC-Connection Error: Not connected to Key-Value Store application.");
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
          coords[0] = mbb->MinD(0) * 1000;
          coords[1] = mbb->MinD(1) * 1000;
          coords[2] = mbb->MaxD(0) * 1000;
          coords[3] = mbb->MaxD(1) * 1000;

          if (kvsIPC->distAddRect(distRef, 4, coords, outputSet, requestOnly)) {
            if (outputSet->size() > 0) {
              res->iter = res->outputServerIds.begin();
              local.addr = res;
              return 0;
            } else {
              cout << "Error: 0 results" << endl;
            }
          } else {
            cout << "Error: distAddRect failed" << endl;
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
        } else {
          cout << "Error: Handling input type failed.\n";
        }

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

  if (!kvsIPC->connected()) {
    // Distribution
    Distribution* dist = static_cast<Distribution*>(args[0].addr);

    int noSons = qp->GetNoSons(s);
    string distributionName =
        static_cast<FText*>(args[noSons - 1].addr)->GetValue();

    int distRef = kvsIPC->getDistributionRef(distributionName);
    if (distRef < 0) {
      distRef = kvsIPC->getDistributionRef(distributionName, dist->type,
                                           dist->toBin());
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

    cout << "Error: Couldn't load distribution." << endl;
  } else {
    cout << "Error: No IPC-Connection" << endl;
  }

  res->Set(true, false);

  return 0;
}

//_ kvsFilter( {distribution, distributionName} ,region ,globalId
//[,updateDistribution] )

ListExpr kvsFilterTM(ListExpr args) {
  if (!nl->HasLength(args, 4) && !nl->HasLength(args, 5)) {
    return listutils::typeError(
        "4 OR 5 arguments expected. [stream (tuple (...)) x  distribution x "
        "region x int x bool = _ ({distribution, distributionName} ,region "
        ",globalId [,updateDistribution])].");
  }

  ListExpr firstType = nl->First(nl->First(args));
  if (!Stream<Tuple>::checkType(firstType)) {
    return listutils::typeError(
        "1st argument should be stream (tuple (...)). [stream (tuple (...)) x  "
        "distribution x region x int x bool = _ ({distribution, "
        "distributionName} ,region ,globalId [,updateDistribution])].");
  }

  ListExpr secondType = nl->First(nl->Second(args));
  if (!FText::checkType(secondType) && !Distribution::checkType(secondType)) {
    return listutils::typeError(
        "2nd argument should be distribution OR text. [stream (tuple (...)) x  "
        "distribution x region x int x bool = _ ({distribution, "
        "distributionName} ,region ,globalId [,updateDistribution])].");
  }

  ListExpr thirdType = nl->First(nl->Third(args));
  if (nl->AtomType(thirdType) != SymbolType) {
    return listutils::typeError(
        "3rd argument should be symbol. [stream (tuple (...)) x  distribution "
        "x region x int x bool = _ ({distribution, distributionName} ,region "
        ",globalId [,updateDistribution])].");
  }

  ListExpr fouthType = nl->First(nl->Fourth(args));
  if (nl->AtomType(fouthType) != SymbolType) {
    return listutils::typeError(
        "4th argument should be symbol. [stream (tuple (...)) x  distribution "
        "x region x int x bool = _ ({distribution, distributionName} ,region "
        ",globalId [,updateDistribution])].");
  }

  if (nl->HasLength(args, 5)) {
    ListExpr fifthType = nl->First(nl->Fifth(args));
    if (!CcBool::checkType(fifthType)) {
      return listutils::typeError(
          "5th argument should be bool. [stream (tuple (...)) x  distribution "
          "x region x int x bool = _ ({distribution, distributionName} ,region "
          ",globalId [,updateDistribution])].");
    }
  }

  if (!kvsIPC->connected()) {
    return listutils::typeError(
        "IPC-Connection Error: Not connected to Key-Value Store application.");
  }

  ListExpr dist_name;
  if (FText::checkType(secondType)) {
    dist_name = nl->Second(nl->Second(args));  // ((text 'distname')...)
  } else {
    dist_name = nl->TextAtom(nl->SymbolValue(
        nl->Second(nl->Second(args))));  // ( (..) (distribution distname))
  }

  // Debug:(((stream (tuple ((Osm_id string) (Name string) (Type string)
  // (GeoData region) (ServerId int)))) (extendstream (head (feed Buildings) 1)
  // ((ServerId (fun (tuple1 TUPLE) (kvsServerId testdist4 (bbox (attr tuple1
  // GeoData)) TRUE)))))) (ServerId ServerId))

  ListExpr tuple_desc = nl->Second(nl->First(nl->First(args)));
  if (nl->IsEqual(nl->First(tuple_desc), Tuple::BasicType()) &&
      nl->ListLength(tuple_desc) == 2) {
    ListExpr attrL = nl->Second(tuple_desc);

    if (IsTupleDescription(attrL)) {
      string geo_attr_name = nl->SymbolValue(thirdType);
      string id_attr_name = nl->SymbolValue(fouthType);

      ListExpr attrType;
      int geo_attr_index = FindAttribute(attrL, geo_attr_name, attrType);
      int id_attr_index = FindAttribute(attrL, id_attr_name, attrType);

      if (geo_attr_index > 0 && id_attr_index > 0) {
        return nl->ThreeElemList(
            nl->SymbolAtom(Symbol::APPEND()),
            nl->ThreeElemList(nl->IntAtom(geo_attr_index),
                              nl->IntAtom(id_attr_index), dist_name),
            nl->First(nl->First(args)));  // return input stream
      } else {
        return listutils::typeError(
            "input not as expected: Attributes not found in tuple...");
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

int kvsFilterVM(Word* args, Word& result, int message, Word& local,
                Supplier s) {
  class FilterLocal {
   public:
    FilterLocal(Address streamAddr, int distRef)
        : distRef(distRef),
          inTupleStream(streamAddr),
          filtered(0),
          notfiltered(0) {}
    int distRef;
    Stream<Tuple> inTupleStream;

    int filtered, notfiltered;
  };

  FilterLocal* filter = static_cast<FilterLocal*>(local.addr);

  switch (message) {
    case OPEN: {
      delete filter;

      int noSons = qp->GetNoSons(s);

      // Distribution:
      string distributionName =
          static_cast<FText*>(args[noSons - 1].addr)->GetValue();
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

      if (distRef >= 0) {
        filter = new FilterLocal(args[0].addr, distRef);
        filter->inTupleStream.open();

        local.addr = filter;
        return 0;
      } else {
        local.addr = 0;
        return CANCEL;
      }
    }
    case REQUEST: {
      if (filter != 0) {
        int noSons = qp->GetNoSons(s);

        int geoAttrIndex = ((CcInt*)(args[noSons - 3].addr))->GetIntval() - 1;
        int idAttrIndex = ((CcInt*)(args[noSons - 2].addr))->GetIntval() - 1;
        bool updateDistribution = false;
        if (noSons == 5 + 3) {
          updateDistribution = ((CcBool*)(args[4].addr))->GetValue();
        }

        Tuple* tuple;

        cout << "kvsFilter: Starting to process stream..." << endl;
        while ((tuple = filter->inTupleStream.request()) != 0) {
          const Rectangle<2> mbb =
              static_cast<Region*>(tuple->GetAttribute(geoAttrIndex))
                  ->BoundingBox();
          unsigned int globalId =
              static_cast<CcInt*>(tuple->GetAttribute(idAttrIndex))->GetValue();

          double coords[4];

          coords[0] = mbb.MinD(0) * 1000;
          coords[1] = mbb.MinD(1) * 1000;
          coords[2] = mbb.MaxD(0) * 1000;
          coords[3] = mbb.MaxD(1) * 1000;

          set<int> results;
          int nrCoords = 4;
          if (kvsIPC->distFilter(filter->distRef, nrCoords, coords, globalId,
                                 updateDistribution)) {
            filter->notfiltered++;
            result.addr = tuple;
            return YIELD;
          } else {
            filter->filtered++;
          }
        }

        filter->inTupleStream.close();

        cout << "Finished kvsFilter - filtered:" << filter->filtered
             << " not filtered:" << filter->notfiltered << endl;

        delete filter;
        local.addr = 0;
      }
      result.addr = 0;
      return CANCEL;
    }
    case CLOSE: {
      delete filter;
      local.addr = 0;
    }
      return 0;
  }
  return 0;
}

ListExpr kvsDistStreamTM(ListExpr args) {
  if (!nl->HasLength(args, 1)) {
    return listutils::typeError(
        "Expecting 1 argument "
        "[{distribution, text} = ({distribution, distributionName})].");
  }

  if (!Distribution::checkType(nl->First(nl->First(args))) &&
      !FText::checkType(nl->First(nl->First(args)))) {
    return listutils::typeError(
        "1st argument should be distribution OR text(distribution name). "
        "[{distribution, text} = ({distribution, distributionName})].");
  }

  // append distribution name
  ListExpr dist_name;
  if (FText::checkType(nl->First(nl->First(args)))) {
    dist_name = nl->Second(nl->First(args));  // ((text 'distname')...)
  } else {
    string dist_name_str = nl->SymbolValue(nl->Second(nl->First(args)));
    dist_name = nl->TextAtom(dist_name_str);  // ( (..) (distribution distname))
  }

  ListExpr resType = nl->TwoElemList(
      listutils::basicSymbol<Stream<Tuple> >(),
      nl->TwoElemList(
          listutils::basicSymbol<Tuple>(),
          nl->TwoElemList(
              nl->TwoElemList(nl->SymbolAtom("Area"),
                              nl->SymbolAtom(Rectangle<2>::BasicType())),
              nl->TwoElemList(nl->SymbolAtom("Object-Count"),
                              nl->SymbolAtom(CcInt::BasicType())))));

  return nl->ThreeElemList(nl->SymbolAtom(Symbol::APPEND()),
                           nl->OneElemList(dist_name), resType);
}

int kvsDistStreamVM(Word* args, Word& result, int message, Word& local,
                    Supplier s) {
  result = qp->ResultStorage(s);

  std::list<Tuple*>* resultStream = static_cast<list<Tuple*>*>(local.addr);

  switch (message) {
    case OPEN: {
      delete resultStream;

      Distribution* dist = 0;

      int noSons = qp->GetNoSons(s);

      // Distribution:
      string distributionName =
          static_cast<FText*>(args[noSons - 1].addr)->GetValue();
      int distRef = kvsIPC->getDistributionRef(distributionName);

      if (distRef < 0) {
        ListExpr distributionType = qp->GetSupplierTypeExpr(qp->GetSon(s, 0));
        if (Distribution::checkType(distributionType)) {
          dist = static_cast<Distribution*>(args[0].addr);

          distRef = kvsIPC->getDistributionRef(distributionName, dist->type,
                                               dist->toBin());
        } else {
          cout << "Error: No Distribution specified";
          return CANCEL;
        }
      }

      if (distRef >= 0) {
        resultStream = new std::list<Tuple*>;

        // create tupleType
        ListExpr resultTupleType = nl->TwoElemList(
            nl->SymbolAtom(Tuple::BasicType()),
            nl->TwoElemList(
                nl->TwoElemList(nl->SymbolAtom("Area"),
                                nl->SymbolAtom(Rectangle<2>::BasicType())),
                nl->TwoElemList(nl->SymbolAtom("Object-Count"),
                                nl->SymbolAtom(CcInt::BasicType()))));
        ListExpr numResultTupleType =
            SecondoSystem::GetCatalog()->NumericType(resultTupleType);
        TupleType* tt = new TupleType(numResultTupleType);

        // retrieve distribution
        if (dist == 0) {
          // get distribution snapshot from application

          string distData;

          if (kvsIPC->getDistributionData(distRef, &distData)) {
            dist = Distribution::getInstance(distData);
          }
        }

        // retrieve list
        std::list<std::pair<double*, int> > areaList;

        dist->createAreaObjectCountList(&areaList);

        // create result stream

        while (!areaList.empty()) {
          double* mbb = areaList.front().first;
          int count = areaList.front().second;

          Tuple* tempTuple = new Tuple(tt);

          CcInt* tempCount = new CcInt(count);
          Rectangle<2>* tempRect = new Rectangle<2>(true, &mbb[0], &mbb[2]);

          tempTuple->PutAttribute(0, tempRect);
          tempTuple->PutAttribute(1, tempCount);

          resultStream->push_back(tempTuple);

          delete[] mbb;
          areaList.pop_front();
        }

        tt->DeleteIfAllowed();

        local.addr = resultStream;
        result.addr = 0;
        return 0;
      } else {
        return CANCEL;
      }
    }
    case REQUEST: {
      if (resultStream != 0 && !resultStream->empty()) {
        Tuple* tempTuple = resultStream->front();
        resultStream->pop_front();

        result = SetWord(tempTuple);
        return YIELD;
      } else {
        result.addr = 0;
        return CANCEL;
      }
    }
    case CLOSE: {
      delete resultStream;
      local.addr = 0;
    }
      return CANCEL;
  }
  return CANCEL;
}
}
