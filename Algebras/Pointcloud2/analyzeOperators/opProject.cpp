/*
----
This file is part of SECONDO.

Copyright (C) 2019,
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


//[<] [\ensuremath{<}]
//[>] [\ensuremath{>}]

\setcounter{tocdepth}{3}
\tableofcontents



1 Implementation of the restrict Operators

These Operators are:

  * projectUTM

*/
#include "opProject.h"
#include <string>

#include "QueryProcessor.h"

#include "../tcPointcloud2.h"
#include "../utility/UTM.h"


using namespace pointcloud2;
using namespace std;

extern NestedList *nl;
extern QueryProcessor *qp;

/*
1.1 The Pointcloud2 projectUTM Operator

*/
ListExpr op_projectUTM::projectUTMTM(ListExpr args) {
    const string err("pointcloud2 (WGS84) expected");

    if (!nl->HasLength(args, 1)) {
        return listutils::typeError(err + " (one arguments expected)");
    }

    ListExpr pc2Type = nl->First(args);
    if (!Pointcloud2::TypeCheck(pc2Type)) {
        return listutils::typeError(err + " (error in first argument)");
    }

    ListExpr pc2Ref = nl->Second(pc2Type);

    if ((nl->IsAtom(pc2Ref) && !nl->IsEqual(pc2Ref,"WGS84",false))
        || (!nl->IsAtom(pc2Ref)
            && !nl->IsEqual(nl->First(pc2Ref),"WGS84",false))) {
        return listutils::typeError(err + " (Ref has to be WGS84)");
    }

    size_t tupLength;

    if (nl->IsAtom(pc2Ref)){
      pc2Type = Pointcloud2::cloudTypeWithParams(
                          nl->SymbolAtom("EUCLID"));
      tupLength = 0;
    }else{
      ListExpr pc2Format = nl->Second(pc2Ref);
      pc2Type = Pointcloud2::cloudTypeWithParams(
                          nl->SymbolAtom("EUCLID"),
                          pc2Format);
      tupLength = nl->ListLength(nl->Second(pc2Format));
    }

    return nl->ThreeElemList(
        nl->SymbolAtom(Symbols::APPEND()),
        nl->OneElemList(nl->IntAtom(tupLength)),
        pc2Type);
}


int op_projectUTM::projectUTMVM( Word* args, Word& result, int message,
    Word& local, Supplier s ){

  result = qp->ResultStorage(s);
  Pointcloud2* res = static_cast<pointcloud2::Pointcloud2*>(result.addr);

  Pointcloud2* pc2Source =
      static_cast<pointcloud2::Pointcloud2*>(args[0].addr);

  PcPoint pcPoint;
  SmiRecordFileIterator *it = pc2Source->getFileIterator();
  SmiRecord record;

  CcInt *tupLengthAppend = static_cast<CcInt *>(args[1].addr);
  const size_t tupL = tupLengthAppend->GetIntval();

  const ListExpr resultType = GetTupleResultType(s);
  const ListExpr resultTuple = tupL > 0 ?
      nl->Second(nl->Second(resultType)) : nl->TheEmptyList();

  //calculate UTM zone
  Rect3 bbox = pc2Source->getBoundingBox();
  const int UTM_zone = FLOOR((bbox.MinD(0) + 180.0) / 6) + 1;
  std::cout<<"UTM Zone: "<<UTM_zone<<((bbox.MinD(1)<0) ? "S" : "N")<<endl;

  TupleType* tt = (tupL > 0) ? new TupleType(resultTuple) : nullptr;

  res->startInsert();
  while( it->Next(record) ) {
    double x;
    double y;
    pc2Source->getPoint(record.GetId(), &pcPoint);
    LatLonToUTMXY(pcPoint._y, pcPoint._x, UTM_zone, x, y);
    pcPoint._x = x; pcPoint._y = y;

    if (tupL>0){
      Tuple* elem = new Tuple(tt);
      Tuple* elemsrc = pc2Source->getTuple(pcPoint._tupleId);
      for (size_t i=0;i<tupL;i++){
          elem->CopyAttribute(i, elemsrc, i);
      }
      res->insert(pcPoint, elem);
      elemsrc->DeleteIfAllowed();
      // do NOT call elem->DeleteIfAllowed(); here!
    }else{
      res->insert(pcPoint);
    }
  }
  delete it;
  if (tt) {
      tt->DeleteIfAllowed();
  }
  res->finalizeInsert();
  return 0;
}

std::string op_projectUTM::getOperatorSpec(){
  return OperatorSpec(
      " pointcloud2(WGS84) -> pointcloud2(EUCLID)",
      " pc2 projectUTM ",
      " Returns UTM projected pc2 ",
      " query pc2 projectUTM"
  ).getStr();
}

std::shared_ptr<Operator> op_projectUTM::getOperator(){
  return std::make_shared<Operator>("projectUTM",
      getOperatorSpec(),
      &op_projectUTM::projectUTMVM,
      Operator::SimpleSelect,
      &op_projectUTM::projectUTMTM);
}

/*
1.2 The Pointcloud2 projectWGS84 Operator

*/
ListExpr op_projectWGS84::projectWGS84TM(ListExpr args) {
    const string err("pointcloud2 (EUCLID) expected");

    if (!nl->HasLength(args, 3)) {
        return listutils::typeError(err + " (three arguments expected)");
    }

    ListExpr pc2Type = nl->First(args);
    if (!Pointcloud2::TypeCheck(pc2Type)) {
        return listutils::typeError(err + " (error in first argument)");
    }

    if (!CcInt::checkType(nl->Second(args))) {
      return listutils::typeError(err + " (second argument no int)");
    }

    if (!CcBool::checkType(nl->Third(args))) {
      return listutils::typeError(err + " (third argument no bool)");
    }

    ListExpr pc2Ref = nl->Second(pc2Type);

    if ((nl->IsAtom(pc2Ref) && !nl->IsEqual(pc2Ref,"EUCLID",false))
        || (!nl->IsAtom(pc2Ref)
            && !nl->IsEqual(nl->First(pc2Ref),"EUCLID",false))) {
        return listutils::typeError(err + " (Ref has to be EUCLID)");
    }

    size_t tupLength;

    if (nl->IsAtom(pc2Ref)){
      pc2Type = Pointcloud2::cloudTypeWithParams(
                          nl->SymbolAtom("WGS84"));
      tupLength = 0;
    }else{
      ListExpr pc2Format = nl->Second(pc2Ref);
      pc2Type = Pointcloud2::cloudTypeWithParams(
                          nl->SymbolAtom("WGS84"),
                          pc2Format);
      tupLength = nl->ListLength(nl->Second(pc2Format));
    }

    return nl->ThreeElemList(
        nl->SymbolAtom(Symbols::APPEND()),
        nl->OneElemList(nl->IntAtom(tupLength)),
        pc2Type);
}


int op_projectWGS84::projectWGS84VM( Word* args, Word& result, int message,
    Word& local, Supplier s ){

  result = qp->ResultStorage(s);
  Pointcloud2* res = static_cast<pointcloud2::Pointcloud2*>(result.addr);

  Pointcloud2* pc2Source =
      static_cast<pointcloud2::Pointcloud2*>(args[0].addr);

  PcPoint pcPoint;
  SmiRecordFileIterator *it = pc2Source->getFileIterator();
  SmiRecord record;

  CcInt *tupLengthAppend = static_cast<CcInt *>(args[3].addr);
  const size_t tupL = tupLengthAppend->GetIntval();

  const ListExpr resultType = GetTupleResultType(s);
  const ListExpr resultTuple = tupL > 0 ?
      nl->Second(nl->Second(resultType)) : nl->TheEmptyList();

  //UTM zone
  const int UTM_zone = static_cast<CcInt*>(args[1].addr)->GetIntval();

  //South?
  const bool southhemi = static_cast<CcBool*>(args[2].addr)->GetBoolval();

  TupleType* tt = (tupL > 0) ? new TupleType(resultTuple) : nullptr;

  res->startInsert();
  while( it->Next(record) ) {
    double x;
    double y;
    pc2Source->getPoint(record.GetId(), &pcPoint);
    UTMXYToLatLon(pcPoint._x, pcPoint._y, UTM_zone, southhemi, y, x);
    pcPoint._x = x; pcPoint._y = y;

    if (tupL > 0){
      Tuple* elem = new Tuple(tt);
      Tuple* elemsrc = pc2Source->getTuple(pcPoint._tupleId);
      for (size_t i=0;i<tupL;i++){
          elem->CopyAttribute(i, elemsrc, i);
      }
      res->insert(pcPoint, elem);
      elemsrc->DeleteIfAllowed();
      // do NOT call elem->DeleteIfAllowed(); here!
    }else{
      res->insert(pcPoint);
    }
  }
  delete it;
  res->finalizeInsert();
  if (tt) {
      tt->DeleteIfAllowed();
  }

  return 0;
}

std::string op_projectWGS84::getOperatorSpec(){
  return OperatorSpec(
      " pointcloud2(EUCLID,UTM-Zone) -> pointcloud2(WGS84)",
      " pc2 projectWGS84 [5] ",
      " Returns WGS84 pc2 ",
      " query pc2 projectUTM[5]"
  ).getStr();
}

std::shared_ptr<Operator> op_projectWGS84::getOperator(){
  return std::make_shared<Operator>("projectWGS84",
      getOperatorSpec(),
      &op_projectWGS84::projectWGS84VM,
      Operator::SimpleSelect,
      &op_projectWGS84::projectWGS84TM);
}

/*
1.1 The Pointcloud2 UTMZone Operator

*/
ListExpr op_UTMZone::UTMZoneTM(ListExpr args) {
    const string err("pointcloud2 (WGS84) expected");

    if (!nl->HasLength(args, 1)) {
        return listutils::typeError(err + " (one arguments expected)");
    }

    ListExpr pc2Type = nl->First(args);
    if (!Pointcloud2::TypeCheck(pc2Type)) {
        return listutils::typeError(err + " (error in first argument)");
    }

    ListExpr pc2Ref = nl->Second(pc2Type);

        if ((nl->IsAtom(pc2Ref) && !nl->IsEqual(pc2Ref,"WGS84",false))
            || (!nl->IsAtom(pc2Ref)
                && !nl->IsEqual(nl->First(pc2Ref),"WGS84",false))) {
            return listutils::typeError(err + " (Ref has to be WGS84)");
        }

    return listutils::basicSymbol<CcInt>();
}


int op_UTMZone::UTMZoneVM( Word* args, Word& result, int message,
    Word& local, Supplier s ){

  result = qp->ResultStorage(s);
  CcInt* res = static_cast<CcInt*>(result.addr);

  Pointcloud2* pc2 =
      static_cast<pointcloud2::Pointcloud2*>(args[0].addr);

  if (pc2 != nullptr) {
      Rect bbox= pc2->getBoundingBox().Project2D(0,1);
      int UTM_zone = FLOOR((bbox.MinD(0) + 180.0) / 6) + 1;
      res->Set(true, UTM_zone);
  } else {
    res->Set(false);
  }

  return 0;
}

std::string op_UTMZone::getOperatorSpec(){
  return OperatorSpec(
      " pointcloud2(WGS84) -> int",
      " pc2 UTMZone ",
      " Returns UTM Zone of pc2 ",
      " query pc2 UTMZone"
  ).getStr();
}

std::shared_ptr<Operator> op_UTMZone::getOperator(){
  return std::make_shared<Operator>("utmZone",
      getOperatorSpec(),
      &op_UTMZone::UTMZoneVM,
      Operator::SimpleSelect,
      &op_UTMZone::UTMZoneTM);
}

/*
1.1 The Pointcloud2 UTMSouth Operator

*/
ListExpr op_UTMSouth::UTMSouthTM(ListExpr args) {
  const string err("pointcloud2 (WGS84) expected");

  if (!nl->HasLength(args, 1)) {
      return listutils::typeError(err + " (one arguments expected)");
  }

  ListExpr pc2Type = nl->First(args);
  if (!Pointcloud2::TypeCheck(pc2Type)) {
      return listutils::typeError(err + " (error in first argument)");
  }

  ListExpr pc2Ref = nl->Second(pc2Type);

      if ((nl->IsAtom(pc2Ref) && !nl->IsEqual(pc2Ref,"WGS84",false))
          || (!nl->IsAtom(pc2Ref)
              && !nl->IsEqual(nl->First(pc2Ref),"WGS84",false))) {
          return listutils::typeError(err + " (Ref has to be WGS84)");
      }

  return listutils::basicSymbol<CcBool>();
}


int op_UTMSouth::UTMSouthVM( Word* args, Word& result, int message,
    Word& local, Supplier s ){

  result = qp->ResultStorage(s);
  CcInt* res = static_cast<CcInt*>(result.addr);

  Pointcloud2* pc2 =
      static_cast<pointcloud2::Pointcloud2*>(args[0].addr);

  if (pc2 != nullptr) {
    Rect bbox= pc2->getBoundingBox().Project2D(0,1);
    res->Set(true, bbox.MinD(1) < 0);
  } else {
    res->Set(false);
  }

  return 0;
}

std::string op_UTMSouth::getOperatorSpec(){
  return OperatorSpec(
      " pointcloud2(WGS84) -> bool",
      " pc2UTMSouth ",
      " Returns true if southern hemisphere",
      " query pc2 UTMSouth"
  ).getStr();
}

std::shared_ptr<Operator> op_UTMSouth::getOperator(){
  return std::make_shared<Operator>("utmSouth",
      getOperatorSpec(),
      &op_UTMSouth::UTMSouthVM,
      Operator::SimpleSelect,
      &op_UTMSouth::UTMSouthTM);
}

