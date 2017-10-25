/*
----
This file is part of SECONDO.

Copyright (C) 2004, University in Hagen, Department of Computer Science,
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

//paragraph [1] Title: [{\Large \bf \begin {center}] [\end {center}}]
//[TOC] [\tableofcontents]
//[_] [\_]

[1] File of the Spatial Algebra LR

September, 20017 Torsten Weidmann

[TOC]

1 Overview

This file build the Spatial Algebra LR by implementing all operators and
declaring the needed type constructors.

2 Defines and includes

*/

#include "QueryProcessor.h"
#include "Line.h"
#include "Region.h"
#include "RectangleBB.h"
#include "GenericTC.h"
#include "Symbols.h"
#include "StandardTypes.h"

#include <string>

namespace salr {

  ListExpr moveToTM(ListExpr args) {
    if (!nl->HasLength(args, 3)) {
      return listutils::typeError("Wrong number of arguments");
    }
    if (!Line::checkType(nl->First(args))
        || !CcReal::checkType(nl->Second(args))
        || !CcReal::checkType(nl->Third(args))) {
      return listutils::typeError("line2 x real x real expected");
    }
    return nl->SymbolAtom(Line::BasicType());
  }

  int moveToVM(Word *args, Word &result, int message, Word &local, Supplier s) {
    result = qp->ResultStorage(s);
    Line *line = (Line *) args[0].addr;
    const CcReal *tx = (CcReal *) args[1].addr;
    const CcReal *ty = (CcReal *) args[2].addr;
    Line* newLine = new Line(*line);
    newLine->moveTo(tx->GetValue(), ty->GetValue());
    result.addr = newLine;
    return 0;
  }

  OperatorSpec moveToSpec(
    "line2 x real x real -> line2",
    "_ lr_moveto [_, _]",
    "Adds a moveTo to a line2.",
    "query l1 lr_moveto [3.0, 3.0]"
  );

  Operator moveToOp(
    "lr_moveto",
    moveToSpec.getStr(),
    moveToVM,
    Operator::SimpleSelect,
    moveToTM
  );

  ListExpr lineToTM(ListExpr args) {
    if (!nl->HasLength(args, 3)) {
      return listutils::typeError("Wrong number of arguments");
    }
    if (!Line::checkType(nl->First(args))
        || !CcReal::checkType(nl->Second(args))
        || !CcReal::checkType(nl->Third(args))) {
      return listutils::typeError("line2 x real x real expected");
    }
    return nl->SymbolAtom(Line::BasicType());
  }

  int lineToVM(Word *args, Word &result, int message, Word &local, Supplier s) {
    result = qp->ResultStorage(s);
    Line *line = (Line *) args[0].addr;
    const CcReal *tx = (CcReal *) args[1].addr;
    const CcReal *ty = (CcReal *) args[2].addr;
    Line* newLine = new Line(*line);
    newLine->lineTo(tx->GetValue(), ty->GetValue());
    result.addr = newLine;
    return 0;
  }

  OperatorSpec lineToSpec(
    "line2 x real x real -> line2",
    "_ lineTo [_, _]",
    "Adds a lineTo to a line2.",
    "query l1 lineTo [3.0, 3.0]"
  );

  Operator lineToOp(
    "lineTo",
    lineToSpec.getStr(),
    lineToVM,
    Operator::SimpleSelect,
    lineToTM
  );

  ListExpr quadToTM(ListExpr args) {
    if (!nl->HasLength(args, 5)) {
      return listutils::typeError("Wrong number of arguments");
    }
    if (!Line::checkType(nl->First(args))
        || !CcReal::checkType(nl->Second(args))
        || !CcReal::checkType(nl->Third(args))
        || !CcReal::checkType(nl->Fourth(args))
        || !CcReal::checkType(nl->Fifth(args))) {
      return listutils::typeError("line2 x real x real x real x real expected");
    }
    return nl->SymbolAtom(Line::BasicType());
  }

  int quadToVM(Word *args, Word &result, int message, Word &local, Supplier s) {
    result = qp->ResultStorage(s);
    Line *line = (Line *) args[0].addr;
    const CcReal *tx1 = (CcReal *) args[1].addr;
    const CcReal *ty1 = (CcReal *) args[2].addr;
    const CcReal *tx2 = (CcReal *) args[3].addr;
    const CcReal *ty2 = (CcReal *) args[4].addr;
    Line* newLine = new Line(*line);
    newLine->quadTo(tx1->GetValue(), ty1->GetValue(), tx2->GetValue(),
                 ty2->GetValue());
    result.addr = newLine;
    return 0;
  }

  OperatorSpec quadToSpec(
    "line2 x real x real x real x real -> line2",
    "_ quadTo [_, _, _, _]",
    "Adds a quadTo to a line2.",
    "query l1 quadTo [3.0, 3.0, 3.0, 4.0]"
  );

  Operator quadToOp(
    "quadTo",
    quadToSpec.getStr(),
    quadToVM,
    Operator::SimpleSelect,
    quadToTM
  );

  ListExpr closeLineTM(ListExpr args) {
    if (!nl->HasLength(args, 1)) {
      return listutils::typeError("Wrong number of arguments");
    }
    if (!Line::checkType(nl->First(args))) {
      return listutils::typeError("line2 expected");
    }
    return nl->SymbolAtom(Line::BasicType());
  }

  int
  closeLineVM(Word *args, Word &result, int message, Word &local, Supplier s) {
    result = qp->ResultStorage(s);
    Line *line = (Line *) args[0].addr;
    Line* newLine = new Line(*line);
    newLine->closeLine();
    result.addr = newLine;
    return 0;
  }

  OperatorSpec closeLineSpec(
    "line2 -> line2",
    "_ closeLine",
    "Adds a closeLine to a line2.",
    "query l1 closeLine"
  );

  Operator closeLineOp(
    "closeLine",
    closeLineSpec.getStr(),
    closeLineVM,
    Operator::SimpleSelect,
    closeLineTM
  );

  ListExpr lineToRegionTM(ListExpr args) {
    if (!nl->HasLength(args, 1)) {
      return listutils::typeError("Wrong number of arguments");
    }
    if (!Line::checkType(nl->First(args))) {
      return listutils::typeError("line2 expected");
    }
    return nl->SymbolAtom(Region::BasicType());
  }

  int
  lineToRegionVM(Word *args, Word &result, int message, Word &local, Supplier s)
  {
    result = qp->ResultStorage(s);
    Region* res = static_cast<Region*>(result.addr);
    Line *line = (Line *) args[0].addr;
    *res = Region(*line);
    return 0;
  }

  OperatorSpec lineToRegionSpec(
    "line2 -> region2",
    "_ toRegion",
    "Creates a region2 to from a line2",
    "query l1 toRegion"
  );

  Operator lineToRegionOp(
    "toRegion",
    lineToRegionSpec.getStr(),
    lineToRegionVM,
    Operator::SimpleSelect,
    lineToRegionTM
  );

  int containsFun_L (Word* args, Word& result, int message,
                Word& local, Supplier s)
  {
    result = qp->ResultStorage(s);
    Line *line = (Line *) args[0].addr;
    const CcReal *tx = (CcReal *) args[1].addr;
    const CcReal *ty = (CcReal *) args[2].addr;
    CcBool* b = static_cast<CcBool*>(result.addr);
    b->Set(true, line->contains(tx->GetValue(), ty->GetValue()));
    return 0;
  }

  int containsFun_R (Word* args, Word& result, int message,
                Word& local, Supplier s)
  {
    result = qp->ResultStorage(s);
    Region *region = (Region *) args[0].addr;
    const CcReal *tx = (CcReal *) args[1].addr;
    const CcReal *ty = (CcReal *) args[2].addr;
    CcBool* b = static_cast<CcBool*>(result.addr);
    b->Set(true, region->contains(tx->GetValue(), ty->GetValue()));
    return 0;
  }

  ListExpr containsTypeMap(ListExpr args)
  {
    NList type(args);
    const std::string errMsg = "Expecting line or region with two double.";
    if (type == NList(Line::BasicType(), CcReal::BasicType(),
                       CcReal::BasicType())) {
      return NList(CcBool::BasicType()).listExpr();
    }
    if (type == NList(Region::BasicType(), CcReal::BasicType(),
                      CcReal::BasicType())) {
      return NList(CcBool::BasicType()).listExpr();
    }
    return NList::typeError(errMsg);
  }

  int containsSelect(ListExpr args) {
    NList type(args);
    if (type.first().isSymbol( Line::BasicType()))
      return 0;
    else if(type.first().isSymbol( Region::BasicType()))
      return 1;
    else
      return 3;
  }

  struct containsInfo : OperatorInfo {
    containsInfo()
    {
      name      = "lr_contains";
      signature = Line::BasicType() + " x " + CcReal::BasicType() + " x " +
          CcReal::BasicType() + " -> " +CcBool::BasicType();
      appendSignature(Region::BasicType() + " x "+ CcReal::BasicType() + " x " +
               CcReal::BasicType() + " -> " +CcBool::BasicType());
      syntax    = "_ lr_contains [_, _]";
      meaning   = "Contains predicate.";
    }
  };

  int intersectFun_L (Word* args, Word& result, int message,
                     Word& local, Supplier s)
  {
    result = qp->ResultStorage(s);
    Line *line = (Line *) args[0].addr;
    RectangleBB *rec = (RectangleBB *) args[1].addr;
    CcBool* b = static_cast<CcBool*>(result.addr);
    b->Set(true, line->intersects(rec));
    return 0;
  }

  int intersectFun_R (Word* args, Word& result, int message,
                     Word& local, Supplier s)
  {
    result = qp->ResultStorage(s);
    Region *region = (Region *) args[0].addr;
    RectangleBB *rec = (RectangleBB *) args[1].addr;
    CcBool* b = static_cast<CcBool*>(result.addr);
    b->Set(true, region->intersects(rec));
    return 0;
  }

  ListExpr intersectTypeMap(ListExpr args)
  {
    NList type(args);
    const std::string errMsg = "Expecting line or region with rectangleBB.";
    if (type == NList(Line::BasicType(), RectangleBB::BasicType())) {
      return NList(CcBool::BasicType()).listExpr();
    }
    if (type == NList(Region::BasicType(), RectangleBB::BasicType())) {
      return NList(CcBool::BasicType()).listExpr();
    }
    return NList::typeError(errMsg);
  }

  int intersectSelect(ListExpr args) {
    NList type(args);
    if (type.first().isSymbol( Line::BasicType()))
      return 0;
    else if(type.first().isSymbol( Region::BasicType()))
      return 1;
    else
      return 3;
  }

  struct intersectInfo : OperatorInfo {
    intersectInfo()
    {
      name      = "lr_intersects";
      signature = Line::BasicType() + " x " + RectangleBB::BasicType()
                  + " -> " +CcBool::BasicType();
      appendSignature(Region::BasicType() + " x "+ RectangleBB::BasicType()
                      + " -> " +CcBool::BasicType());
      syntax    = "_ lr_intersects [_, _]";
      meaning   = "Intersection predicate.";
    }
  };

  int boundsFun_L (Word* args, Word& result, int message,
                      Word& local, Supplier s)
  {
    result = qp->ResultStorage(s);
    Line *line = (Line *) args[0].addr;
    result.addr = line->getBounds();
    return 0;
  }

  int boundsFun_R (Word* args, Word& result, int message,
                      Word& local, Supplier s)
  {
    result = qp->ResultStorage(s);
    Region *region = (Region *) args[0].addr;
    result.addr = region->getBounds();
    return 0;
  }

  ListExpr boundsTypeMap(ListExpr args)
  {
    NList type(args);
    const std::string errMsg = "Expecting line or region";
    if (type.first().isSymbol(Line::BasicType())) {
      return NList(RectangleBB::BasicType()).listExpr();
    }
    if (type.first().isSymbol(Region::BasicType())) {
      return NList(RectangleBB::BasicType()).listExpr();
    }
    return NList::typeError(errMsg);
  }

  int boundsSelect(ListExpr args) {
    NList type(args);
    if (type.first().isSymbol( Line::BasicType()))
      return 0;
    else if(type.first().isSymbol( Region::BasicType()))
      return 1;
    else
      return 3;
  }

  struct boundsInfo : OperatorInfo {
    boundsInfo()
    {
      name      = "getbounds";
      signature = Line::BasicType() + " -> " + RectangleBB::BasicType();
      appendSignature(Region::BasicType() + " -> "+ RectangleBB::BasicType());
      syntax    = "_ getbounds";
      meaning   = "Get Boundary.";
    }
  };

  GenTC <Line> LineTC;
  GenTC <Region> RegionTC;
  GenTC <RectangleBB> RectangleBBTC;

  class SpatialLRAlgebra : public Algebra {
  public:
    SpatialLRAlgebra() : Algebra() {
      AddTypeConstructor(&LineTC);
      LineTC.AssociateKind(Kind::DATA());

      AddTypeConstructor(&RegionTC);
      RegionTC.AssociateKind(Kind::DATA());

      AddTypeConstructor(&RectangleBBTC);
      RectangleBBTC.AssociateKind(Kind::DATA());

      AddOperator(&moveToOp);
      AddOperator(&lineToOp);
      AddOperator(&quadToOp);
      AddOperator(&closeLineOp);
      AddOperator(&lineToRegionOp);

      ValueMapping containsFuns[] = {containsFun_L, containsFun_R, 0};
      AddOperator(containsInfo(), containsFuns, containsSelect,containsTypeMap);

      ValueMapping intersectFuns[] = {intersectFun_L, intersectFun_R, 0};
      AddOperator(intersectInfo(), intersectFuns,
                  intersectSelect, intersectTypeMap);

      ValueMapping boundsFuns[] = {boundsFun_L, boundsFun_R, 0};
      AddOperator(boundsInfo(), boundsFuns, boundsSelect, boundsTypeMap);
    }

    ~SpatialLRAlgebra() {};
  };
}

extern "C"
Algebra *
InitializeSpatialLRAlgebra(NestedList *nlRef, QueryProcessor *qpRef) {
  return (new salr::SpatialLRAlgebra());
}