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

[1] Files of SpatialLRAlgebra

November, 2017 Torsten Weidmann

[TOC]

1 SpatialLRAlgebra

1.1 Overview

This file contains the declaration of the SpatialLRAlgebra as well as all
 associated types and operations. All methods necessary for these operations
 are implemented here as well.

Defines and includes.

*/

#include "QueryProcessor.h"
#include "Line.h"
#include "Region.h"
#include "RectangleBB.h"
#include "GenericTC.h"
#include "Symbols.h"
#include "StandardTypes.h"
#include "SpatialAlgebra.h"

#include <string>

namespace salr {

/*
1.2 Begin of method declarations.

Methods for the *moveTo* operation.

*/
  ListExpr moveToTM(ListExpr args) {
    if (!nl->HasLength(args, 3)) {
      return listutils::typeError("Wrong number of arguments");
    }
    if (!Line2::checkType(nl->First(args))
        || !CcReal::checkType(nl->Second(args))
        || !CcReal::checkType(nl->Third(args))) {
      return listutils::typeError("line2 x real x real expected");
    }
    return nl->SymbolAtom(Line2::BasicType());
  }

  int moveToVM(Word *args, Word &result, int message, Word &local, Supplier s) {
    result = qp->ResultStorage(s);
    Line2* res = static_cast<Line2*>(result.addr);
    Line2 *line = (Line2 *) args[0].addr;
    const CcReal *tx = (CcReal *) args[1].addr;
    const CcReal *ty = (CcReal *) args[2].addr;
    *res = Line2(*line);
    res->moveTo(tx->GetValue(), ty->GetValue());
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

/*
Methods for the *lineTo* operation.

*/
  ListExpr lineToTM(ListExpr args) {
    if (!nl->HasLength(args, 3)) {
      return listutils::typeError("Wrong number of arguments");
    }
    if (!Line2::checkType(nl->First(args))
        || !CcReal::checkType(nl->Second(args))
        || !CcReal::checkType(nl->Third(args))) {
      return listutils::typeError("line2 x real x real expected");
    }
    return nl->SymbolAtom(Line2::BasicType());
  }

  int lineToVM(Word *args, Word &result, int message, Word &local, Supplier s) {
    result = qp->ResultStorage(s);
    Line2* res = static_cast<Line2*>(result.addr);
    Line2 *line = (Line2 *) args[0].addr;
    const CcReal *tx = (CcReal *) args[1].addr;
    const CcReal *ty = (CcReal *) args[2].addr;
    *res = Line2(*line);
    res->lineTo(tx->GetValue(), ty->GetValue());
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

/*
Methods for the *quadTo* operation.

*/
  ListExpr quadToTM(ListExpr args) {
    if (!nl->HasLength(args, 5)) {
      return listutils::typeError("Wrong number of arguments");
    }
    if (!Line2::checkType(nl->First(args))
        || !CcReal::checkType(nl->Second(args))
        || !CcReal::checkType(nl->Third(args))
        || !CcReal::checkType(nl->Fourth(args))
        || !CcReal::checkType(nl->Fifth(args))) {
      return listutils::typeError("line2 x real x real x real x real expected");
    }
    return nl->SymbolAtom(Line2::BasicType());
  }

  int quadToVM(Word *args, Word &result, int message, Word &local, Supplier s) {
    result = qp->ResultStorage(s);
    Line2* res = static_cast<Line2*>(result.addr);
    Line2 *line = (Line2 *) args[0].addr;
    const CcReal *tx1 = (CcReal *) args[1].addr;
    const CcReal *ty1 = (CcReal *) args[2].addr;
    const CcReal *tx2 = (CcReal *) args[3].addr;
    const CcReal *ty2 = (CcReal *) args[4].addr;
    *res = Line2(*line);
    res->quadTo(tx1->GetValue(), ty1->GetValue(), tx2->GetValue(),
                 ty2->GetValue());
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

/*
Methods for the *closeLine* operation.

*/
  ListExpr closeLineTM(ListExpr args) {
    if (!nl->HasLength(args, 1)) {
      return listutils::typeError("Wrong number of arguments");
    }
    if (!Line2::checkType(nl->First(args))) {
      return listutils::typeError("line2 expected");
    }
    return nl->SymbolAtom(Line2::BasicType());
  }

  int
  closeLineVM(Word *args, Word &result, int message, Word &local, Supplier s) {
    result = qp->ResultStorage(s);
    Line2* res = static_cast<Line2*>(result.addr);
    Line2 *line = (Line2 *) args[0].addr;
    *res = Line2(*line);
    res->closeLine();
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

/*
Methods for the *toregion2* operation, with line2 as argument.

*/
  ListExpr lineToRegionTM(ListExpr args) {
    if (!nl->HasLength(args, 1)) {
      return listutils::typeError("Wrong number of arguments");
    }
    if (!Line2::checkType(nl->First(args))) {
      return listutils::typeError("line2 expected");
    }
    return nl->SymbolAtom(Region2::BasicType());
  }

  int
  lineToRegionVM(Word *args, Word &result, int message, Word &local, Supplier s)
  {
    result = qp->ResultStorage(s);
    Region2* res = static_cast<Region2*>(result.addr);
    Line2 *line = (Line2 *) args[0].addr;
    *res = Region2(*line);
    return 0;
  }

  OperatorSpec lineToRegionSpec(
    "line2 -> region2",
    "toregion2 (_)",
    "Creates a region2 to from a line2",
    "query toregion2 (l1)"
  );

  Operator lineToRegionOp(
    "toregion2",
    lineToRegionSpec.getStr(),
    lineToRegionVM,
    Operator::SimpleSelect,
    lineToRegionTM
  );

/*
Methods for the *toline2* operation.

*/
  ListExpr lineToLine2TM(ListExpr args) {
    if (!nl->HasLength(args, 1)) {
      return listutils::typeError("Wrong number of arguments");
    }
    if (!Line::checkType(nl->First(args))) {
      return listutils::typeError("line expected");
    }
    return nl->SymbolAtom(Line2::BasicType());
  }

  int
  lineToLine2VM(Word *args, Word &result, int message, Word &local, Supplier s)
  {
    result = qp->ResultStorage(s);
    Line2* res = static_cast<Line2*>(result.addr);
    Line *line = (Line *) args[0].addr;
    *res = Line2(*line);
    return 0;
  }

  OperatorSpec lineToLine2Spec(
    "line -> line2",
    "toline2 (_)",
    "Creates a line2 from a line",
    "query toline2 (l1)"
  );

  Operator lineToLine2Op(
    "toline2",
    lineToLine2Spec.getStr(),
    lineToLine2VM,
    Operator::SimpleSelect,
    lineToLine2TM
  );

/*
Methods for the *toline* operation.

*/
  ListExpr line2ToLineTM(ListExpr args) {
    if (!nl->HasLength(args, 1)) {
      return listutils::typeError("Wrong number of arguments");
    }
    if (!Line2::checkType(nl->First(args))) {
      return listutils::typeError("line2 expected");
    }
    return nl->SymbolAtom(Line::BasicType());
  }

  int
  line2ToLineVM(Word *args, Word &result, int message, Word &local, Supplier s)
  {
    result = qp->ResultStorage(s);
    Line* res = static_cast<Line*>(result.addr);
    Line2 *line = (Line2 *) args[0].addr;
    *res = line->toLine();
    return 0;
  }

  OperatorSpec line2ToLineSpec(
    "line2 -> line",
    "toline (_)",
    "Creates a line from a line2",
    "query toline (l1)"
  );

  Operator line2ToLineOp(
    "toline",
    line2ToLineSpec.getStr(),
    line2ToLineVM,
    Operator::SimpleSelect,
    line2ToLineTM
  );

/*
Methods for the *toregion2* operation, with region as argument.

*/
  ListExpr regionToRegion2TM(ListExpr args) {
    if (!nl->HasLength(args, 1)) {
      return listutils::typeError("Wrong number of arguments");
    }
    if (!Region::checkType(nl->First(args))) {
      return listutils::typeError("region expected");
    }
    return nl->SymbolAtom(Region2::BasicType());
  }

  int
  regionToRegion2VM(Word *args, Word &result, int message, Word &local,
                    Supplier s)
  {
    result = qp->ResultStorage(s);
    Region2* res = static_cast<Region2*>(result.addr);
    Region *region = (Region *) args[0].addr;
    *res = Region2(*region);
    return 0;
  }

  OperatorSpec regionToRegion2Spec(
    "region -> region2",
    "toregion2 (_)",
    "Creates a region2 from a region",
    "query toregion2 (r1)"
  );

  Operator regionToRegion2Op(
    "toregion2",
    regionToRegion2Spec.getStr(),
    regionToRegion2VM,
    Operator::SimpleSelect,
    regionToRegion2TM
  );

/*
Methods for the *toregion* operation.

*/
  ListExpr region2ToRegionTM(ListExpr args) {
    if (!nl->HasLength(args, 1)) {
      return listutils::typeError("Wrong number of arguments");
    }
    if (!Region2::checkType(nl->First(args))) {
      return listutils::typeError("region2 expected");
    }
    return nl->SymbolAtom(Region::BasicType());
  }

  int
  region2ToRegionVM(Word *args, Word &result, int message, Word &local,
                    Supplier s)
  {
    result = qp->ResultStorage(s);
    Region2 *region = (Region2 *)args[0].addr;
    Line2 line = Line2(*region);
    Line cl = line.toLine();
    Region *pResult = (Region *)result.addr;
    pResult->Clear();
    cl.Transform( *pResult );
    return 0;
  }

  OperatorSpec region2ToRegionSpec(
    "region2 -> region",
    "_ toregion",
    "Creates a region from a region2",
    "query toregion (r2)"
  );

  Operator region2ToRegionOp(
    "toregion",
    region2ToRegionSpec.getStr(),
    region2ToRegionVM,
    Operator::SimpleSelect,
    region2ToRegionTM
  );

/*
Methods for the *lr\_intersects* operation.

*/
  int intersectFun_L (Word* args, Word& result, int message,
                     Word& local, Supplier s)
  {
    result = qp->ResultStorage(s);
    Line2 *line = (Line2 *) args[0].addr;
    RectangleBB *rec = (RectangleBB *) args[1].addr;
    CcBool* b = static_cast<CcBool*>(result.addr);
    b->Set(true, line->intersects(rec));
    return 0;
  }

  int intersectFun_R (Word* args, Word& result, int message,
                     Word& local, Supplier s)
  {
    result = qp->ResultStorage(s);
    Region2 *region = (Region2 *) args[0].addr;
    RectangleBB *rec = (RectangleBB *) args[1].addr;
    CcBool* b = static_cast<CcBool*>(result.addr);
    b->Set(true, region->intersects(rec));
    return 0;
  }

  ListExpr intersectTypeMap(ListExpr args)
  {
    NList type(args);
    const std::string errMsg = "Expecting line or region with rectangleBB.";
    if (type == NList(Line2::BasicType(), RectangleBB::BasicType())) {
      return NList(CcBool::BasicType()).listExpr();
    }
    if (type == NList(Region2::BasicType(), RectangleBB::BasicType())) {
      return NList(CcBool::BasicType()).listExpr();
    }
    return NList::typeError(errMsg);
  }

  int intersectSelect(ListExpr args) {
    NList type(args);
    if (type.first().isSymbol( Line2::BasicType()))
      return 0;
    else if(type.first().isSymbol( Region2::BasicType()))
      return 1;
    else
      return 3;
  }

  struct intersectInfo : OperatorInfo {
    intersectInfo()
    {
      name      = "lr_intersects";
      signature = Line2::BasicType() + " x " + RectangleBB::BasicType()
                  + " -> " +CcBool::BasicType();
      appendSignature(Region2::BasicType() + " x "+ RectangleBB::BasicType()
                      + " -> " +CcBool::BasicType());
      syntax    = "_ lr_intersects _";
      meaning   = "Intersection predicate.";
    }
  };

/*
Methods for the *getbounds* operation.

*/
  int boundsFun_L (Word* args, Word& result, int message,
                      Word& local, Supplier s)
  {
    result = qp->ResultStorage(s);
    RectangleBB* res = static_cast<RectangleBB*>(result.addr);
    Line2 *line = (Line2 *) args[0].addr;
    *res = line->getBounds();
    return 0;
  }

  int boundsFun_R (Word* args, Word& result, int message,
                      Word& local, Supplier s)
  {
    result = qp->ResultStorage(s);
    RectangleBB* res = static_cast<RectangleBB*>(result.addr);
    Region2 *region = (Region2 *) args[0].addr;
    *res = region->getBounds();
    return 0;
  }

  ListExpr boundsTypeMap(ListExpr args)
  {
    NList type(args);
    const std::string errMsg = "Expecting line or region";
    if (type.first().isSymbol(Line2::BasicType())) {
      return NList(RectangleBB::BasicType()).listExpr();
    }
    if (type.first().isSymbol(Region2::BasicType())) {
      return NList(RectangleBB::BasicType()).listExpr();
    }
    return NList::typeError(errMsg);
  }

  int boundsSelect(ListExpr args) {
    NList type(args);
    if (type.first().isSymbol( Line2::BasicType()))
      return 0;
    else if(type.first().isSymbol( Region2::BasicType()))
      return 1;
    else
      return 3;
  }

  struct boundsInfo : OperatorInfo {
    boundsInfo()
    {
      name      = "getbounds";
      signature = Line2::BasicType() + " -> " + RectangleBB::BasicType();
      appendSignature(Region2::BasicType() + " -> "+ RectangleBB::BasicType());
      syntax    = "_ getbounds";
      meaning   = "Get Boundary.";
    }
  };

/*
Methods for the *union1* operation.

*/
  ListExpr unionTM(ListExpr args) {
    if (!nl->HasLength(args, 2)) {
      return listutils::typeError("Wrong number of arguments");
    }
    if (!Region2::checkType(nl->First(args))
        || !Region2::checkType(nl->Second(args))) {
      return listutils::typeError("region2 x region2 expected");
    }
    return nl->SymbolAtom(Region2::BasicType());
  }

  int
  unionVM(Word *args, Word &result, int message, Word &local, Supplier s)
  {
    result = qp->ResultStorage(s);
    Region2* res = static_cast<Region2*>(result.addr);
    Region2 *r1 = (Region2 *) args[0].addr;
    Region2 *r2 = (Region2 *) args[1].addr;
    r2->updateCurves();
    *res = Region2(*r1);
    res->union1(r2);
    return 0;
  }

  OperatorSpec unionSpec(
    "region2 x region2 -> region2",
    "_ union1 _",
    "Creates the union of two region2",
    "query r1 union1 r2"
  );

  Operator unionOp(
    "union1",
    unionSpec.getStr(),
    unionVM,
    Operator::SimpleSelect,
    unionTM
  );

/*
Methods for the *minus1* operation.

*/
  ListExpr minusTM(ListExpr args) {
    if (!nl->HasLength(args, 2)) {
      return listutils::typeError("Wrong number of arguments");
    }
    if (!Region2::checkType(nl->First(args))
        || !Region2::checkType(nl->Second(args))) {
      return listutils::typeError("region2 x region2 expected");
    }
    return nl->SymbolAtom(Region2::BasicType());
  }

  int
  minusVM(Word *args, Word &result, int message, Word &local, Supplier s)
  {
    result = qp->ResultStorage(s);
    Region2* res = static_cast<Region2*>(result.addr);
    Region2 *r1 = (Region2 *) args[0].addr;
    Region2 *r2 = (Region2 *) args[1].addr;
    r2->updateCurves();
    *res = Region2(*r1);
    res->minus1(r2);
    return 0;
  }

  OperatorSpec minusSpec(
    "region2 x region2 -> region2",
    "_ minus1 _",
    "Returns the part of r1 which does not intersect with r2.",
    "query r1 minus1 r2"
  );

  Operator minusOp(
    "minus1",
    minusSpec.getStr(),
    minusVM,
    Operator::SimpleSelect,
    minusTM
  );

/*
Methods for the *intersection1* operation.

*/
  ListExpr intersectionTM(ListExpr args) {
    if (!nl->HasLength(args, 2)) {
      return listutils::typeError("Wrong number of arguments");
    }
    if (!Region2::checkType(nl->First(args))
        || !Region2::checkType(nl->Second(args))) {
      return listutils::typeError("region2 x region2 expected");
    }
    return nl->SymbolAtom(Region2::BasicType());
  }

  int
  intersectionVM(Word *args, Word &result, int message, Word &local, Supplier s)
  {
    result = qp->ResultStorage(s);
    Region2* res = static_cast<Region2*>(result.addr);
    Region2 *r1 = (Region2 *) args[0].addr;
    Region2 *r2 = (Region2 *) args[1].addr;
    r2->updateCurves();
    *res = Region2(*r1);
    res->intersection1(r2);
    return 0;
  }

  OperatorSpec intersectionSpec(
    "region2 x region2 -> region2",
    "intersection1 (_, _)",
    "Returns the region in which r1 and r2 intersect.",
    "query intersection1(r1, r2)"
  );

  Operator intersectionOp(
    "intersection1",
    intersectionSpec.getStr(),
    intersectionVM,
    Operator::SimpleSelect,
    intersectionTM
  );

/*
Methods for the *xor1* operation.

*/
  ListExpr xorTM(ListExpr args) {
    if (!nl->HasLength(args, 2)) {
      return listutils::typeError("Wrong number of arguments");
    }
    if (!Region2::checkType(nl->First(args))
        || !Region2::checkType(nl->Second(args))) {
      return listutils::typeError("region2 x region2 expected");
    }
    return nl->SymbolAtom(Region2::BasicType());
  }

  int
  xorVM(Word *args, Word &result, int message, Word &local, Supplier s)
  {
    result = qp->ResultStorage(s);
    Region2* res = static_cast<Region2*>(result.addr);
    Region2 *r1 = (Region2 *) args[0].addr;
    Region2 *r2 = (Region2 *) args[1].addr;
    r2->updateCurves();
    *res = Region2(*r1);
    res->xor1(r2);
    return 0;
  }

  OperatorSpec xorSpec(
    "region2 x region2 -> region2",
    "_ xor1 _",
    "Returns the region in which r1 and r2 don't intersect.",
    "query r1 xor1 r2"
  );

  Operator xorOp(
    "xor1",
    xorSpec.getStr(),
    xorVM,
    Operator::SimpleSelect,
    xorTM
  );

/*
1.3 Declarations of constructors for new data types.

*/
  GenTC <Line2> LineTC;
  GenTC <Region2> RegionTC;
  GenTC <RectangleBB> RectangleBBTC;

/*
1.4 Class ~SpatialLRAlgebra~

*/
  class SpatialLRAlgebra : public Algebra {
  public:
    SpatialLRAlgebra() : Algebra() {
/*
Adding type constructors to algebra.

*/
      AddTypeConstructor(&LineTC);
      LineTC.AssociateKind(Kind::DATA());
      AddTypeConstructor(&RegionTC);
      RegionTC.AssociateKind(Kind::DATA());
      AddTypeConstructor(&RectangleBBTC);
      RectangleBBTC.AssociateKind(Kind::DATA());

/*
Adding operators to algebra.

*/
      AddOperator(&moveToOp);
      AddOperator(&lineToOp);
      AddOperator(&quadToOp);
      AddOperator(&closeLineOp);
      AddOperator(&lineToRegionOp);
      AddOperator(&lineToLine2Op);
      AddOperator(&line2ToLineOp);
      AddOperator(&regionToRegion2Op);
      AddOperator(&region2ToRegionOp);
      AddOperator(&unionOp);
      AddOperator(&minusOp);
      AddOperator(&intersectionOp);
      AddOperator(&xorOp);

/*
Adding value mappings for overloaded operators to algebra.

*/
      ValueMapping intersectFuns[] = {intersectFun_L, intersectFun_R, 0};
      AddOperator(intersectInfo(), intersectFuns,
                  intersectSelect, intersectTypeMap);

      ValueMapping boundsFuns[] = {boundsFun_L, boundsFun_R, 0};
      AddOperator(boundsInfo(), boundsFuns, boundsSelect, boundsTypeMap);
    }

/*
Destructor.

*/
    ~SpatialLRAlgebra() {};
  };
}

/*
Initialize the algebra.

*/
extern "C"
Algebra *
InitializeSpatialLRAlgebra(NestedList *nlRef, QueryProcessor *qpRef) {
  return (new salr::SpatialLRAlgebra());
}