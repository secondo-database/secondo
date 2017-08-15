/*
----
This file is part of SECONDO.

Copyright (C) 2004-2009, University in Hagen, Faculty of Mathematics
and Computer Science, Database Systems for New Applications.

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

#include "Operators.h"

#include "TemporalAlgebra.h"
#include "Types.h"
#include "LongIntsTC.h"
#include "Ints.h"

namespace ColumnMovingAlgebra
{

  const OperatorInfo PresentOperator::info = OperatorInfo(
      "present",
      "(mints | mpoints) x (instant | periods) -> (bools | longints)",
      "_ present _",
      "returns true | the indices for all moving objects, \n"
      "that are defined at the given instant or periods",
      "query mintegers present [const instant value \"2005-11-20-07:01:44\"]");

  ValueMapping PresentOperator::valueMappings[] = {
    ValueMapping00,
    ValueMapping01,
    ValueMapping10,
    ValueMapping11,
    nullptr
  };

  ListExpr PresentOperator::TypeMapping(ListExpr args) {
    return typeMapping(signatures(), args);
  }

  int PresentOperator::SelectValueMapping(ListExpr args) {
    return selectValueMapping(signatures(), args);
  }
  
  list<AttrArrayOperatorSignatur> PresentOperator::signatures() {
    return list<AttrArrayOperatorSignatur> {
      { temporalalgebra::MInt::BasicType(), Instant::BasicType(), 
        BoolsType::TI(false).GetTypeExpr() },
      { temporalalgebra::MInt::BasicType(), temporalalgebra::Periods
        ::BasicType(), BoolsType::TI(false).GetTypeExpr() },
      { temporalalgebra::MPoint::BasicType(), Instant::BasicType(), 
        CRelAlgebra::LongIntsTI(false).GetTypeExpr() },
      { temporalalgebra::MPoint::BasicType(), temporalalgebra::Periods::
        BasicType(), CRelAlgebra::LongIntsTI(false).GetTypeExpr() },
    };
  }
  
  int PresentOperator::ValueMapping00(ArgVector args, Word &result, 
    int message, Word &local, Supplier s) 
  {
    static_cast<MInts*>(args[0].addr)->present( 
        *static_cast<Instant*>(args[1].addr), 
        qp->ResultStorage<Bools>(result, s) );
    return 0;
  }

  int PresentOperator::ValueMapping01(ArgVector args, Word &result, 
    int message, Word &local, Supplier s) 
  {
    static_cast<MInts*>(args[0].addr)->present( 
        *static_cast<temporalalgebra::Periods*>(args[1].addr), 
        qp->ResultStorage<Bools>(result, s) );
    return 0;
  }
  
  int PresentOperator::ValueMapping10(ArgVector args, Word &result, 
    int message, Word &local, Supplier s) 
  {
    static_cast<MPoints*>(args[0].addr)->present( 
        *static_cast<Instant*>(args[1].addr), 
        qp->ResultStorage<CRelAlgebra::LongInts>(result, s) );
    return 0;
  }

  int PresentOperator::ValueMapping11(ArgVector args, Word &result, 
    int message, Word &local, Supplier s) 
  {
    static_cast<MPoints*>(args[0].addr)->present( 
        *static_cast<temporalalgebra::Periods*>(args[1].addr), 
        qp->ResultStorage<CRelAlgebra::LongInts>(result, s) );
    return 0;
  }



  const OperatorInfo AtInstantOperator::info = OperatorInfo(
      "atinstant",
      "(mints | mpoints) x instant -> (iint | ipoint)",
      "_ atinstant _",
      "returns the value of the moving objects for the given instant",
      "query mi atinstant [const instant value \"2005-11-20-07:01:44\"]");

  ValueMapping AtInstantOperator::valueMappings[] = {
    ValueMapping0,
    ValueMapping1,
    nullptr
  };

  ListExpr AtInstantOperator::TypeMapping(ListExpr args) {
    return typeMapping(signatures(), args);
  }

  int AtInstantOperator::SelectValueMapping(ListExpr args) {
    return selectValueMapping(signatures(), args);
  }
  
  list<AttrArrayOperatorSignatur> AtInstantOperator::signatures() {
    return list<AttrArrayOperatorSignatur> {
      { temporalalgebra::MInt::BasicType(), Instant::BasicType(), 
        IIntsType::TI(false).GetTypeExpr() },
      { temporalalgebra::MPoint::BasicType(), Instant::BasicType(), 
        IPointsType::TI(false).GetTypeExpr() },
    };
  }
  
  int AtInstantOperator::ValueMapping0(ArgVector args, Word &result, 
    int message, Word &local, Supplier s) 
  {
    static_cast<MInts*>(args[0].addr)->atInstant( 
        *static_cast<Instant*>(args[1].addr), 
        qp->ResultStorage<IInts>(result, s) );
    return 0;
  }
  
  int AtInstantOperator::ValueMapping1(ArgVector args, Word &result, 
    int message, Word &local, Supplier s) 
  {
    static_cast<MPoints*>(args[0].addr)->atInstant( 
        *static_cast<Instant*>(args[1].addr), 
        qp->ResultStorage<IPoints>(result, s) );
    return 0;
  }



  const OperatorInfo AtPeriodsOperator::info = OperatorInfo(
      "atperiods",
      "(mints | mpoints) x periods -> (mints | mpoints)",
      "_ atperiods _",
      "restricts the moving objects to the given periods",
      "query movingIntegers atperiods [const periods value \n"
      "(\"2005-11-20-07:01:44\" \"2005-11-20-07:01:44\" TRUE TRUE)]");

  ValueMapping AtPeriodsOperator::valueMappings[] = {
    ValueMapping0,
    ValueMapping1,
    nullptr
  };

  ListExpr AtPeriodsOperator::TypeMapping(ListExpr args) {
    return typeMapping(signatures(), args);
  }

  int AtPeriodsOperator::SelectValueMapping(ListExpr args) {
    return selectValueMapping(signatures(), args);
  }
  
  list<AttrArrayOperatorSignatur> AtPeriodsOperator::signatures() {
    return list<AttrArrayOperatorSignatur> {
      { temporalalgebra::MInt::BasicType(),   temporalalgebra::Periods::
        BasicType(), MIntsType::TI(false).GetTypeExpr() },
      { temporalalgebra::MPoint::BasicType(), temporalalgebra::Periods::
        BasicType(), MPointsType::TI(false).GetTypeExpr() },
    };
  }
  
  int AtPeriodsOperator::ValueMapping0(ArgVector args, Word &result, 
    int message, Word &local, Supplier s) 
  {
    static_cast<MInts*>(args[0].addr)->atPeriods( 
        *static_cast<temporalalgebra::Periods*>(args[1].addr), 
        qp->ResultStorage<MInts>(result, s) );
    return 0;
  }
  
  int AtPeriodsOperator::ValueMapping1(ArgVector args, Word &result, 
    int message, Word &local, Supplier s) 
  {
    static_cast<MPoints*>(args[0].addr)->atPeriods( 
        *static_cast<temporalalgebra::Periods*>(args[1].addr), 
        qp->ResultStorage<MPoints>(result, s) );
    return 0;
  }



  const OperatorInfo PassesOperator::info = OperatorInfo(
      "passes",
      "(mints | mpoints) x (int | (point | region)) -> (bools | longints)",
      "_ passes _",
      "returns true | the indices for all moving objects, \n"
      "that pass the given value at any time",
      "query movingIntegers passes 1");

  ValueMapping PassesOperator::valueMappings[] = {
    ValueMapping0,
    ValueMapping10,
    ValueMapping11,
    nullptr
  };

  ListExpr PassesOperator::TypeMapping(ListExpr args) {
    return typeMapping(signatures(), args);
  }

  int PassesOperator::SelectValueMapping(ListExpr args) {
    return selectValueMapping(signatures(), args);
  }
  
  list<AttrArrayOperatorSignatur> PassesOperator::signatures() {
    return list<AttrArrayOperatorSignatur> {
      { temporalalgebra::MInt::BasicType(),   CcInt::BasicType(), 
        BoolsType::TI(false).GetTypeExpr() },
      { temporalalgebra::MPoint::BasicType(), Point::BasicType(), 
        CRelAlgebra::LongIntsTI(false).GetTypeExpr() },
      { temporalalgebra::MPoint::BasicType(), Region::BasicType(), 
        CRelAlgebra::LongIntsTI(false).GetTypeExpr() },
    };
  }
  
  int PassesOperator::ValueMapping0(ArgVector args, Word &result, int message, 
    Word &local, Supplier s) 
  {
    static_cast<MInts*>(args[0].addr)->passes( 
        *static_cast<CcInt*>(args[1].addr), 
        qp->ResultStorage<Bools>(result, s) );
    return 0;
  }
  
  int PassesOperator::ValueMapping10(ArgVector args, Word &result, int message, 
    Word &local, Supplier s) 
  {
    static_cast<MPoints*>(args[0].addr)->passes( 
        *static_cast<Point*>(args[1].addr), 
        qp->ResultStorage<CRelAlgebra::LongInts>(result, s) );
    return 0;
  }
  
  int PassesOperator::ValueMapping11(ArgVector args, Word &result, int message, 
    Word &local, Supplier s) 
  {
    static_cast<MPoints*>(args[0].addr)->passes( 
        *static_cast<Region*>(args[1].addr), 
        qp->ResultStorage<CRelAlgebra::LongInts>(result, s) );
    return 0;
  }



  const OperatorInfo AtOperator::info = OperatorInfo(
      "at",
      "(mints | mpoints) \n"
      "x ((int | rint) | (point | region)) \n"
      "-> (mints | mpoints)",
      "_ at _",
      "restricts the given moving objects to the given value or ranges",
      "query movingIntegers at 1");

  ValueMapping AtOperator::valueMappings[] = {
    ValueMapping00,
    ValueMapping01,
    ValueMapping10,
    ValueMapping11,
    nullptr
  };

  ListExpr AtOperator::TypeMapping(ListExpr args) {
    return typeMapping(signatures(), args);
  }

  int AtOperator::SelectValueMapping(ListExpr args) {
    return selectValueMapping(signatures(), args);
  }
  
  list<AttrArrayOperatorSignatur> AtOperator::signatures() {
    return list<AttrArrayOperatorSignatur> {
      { temporalalgebra::MInt::BasicType(), CcInt::BasicType(), 
        MIntsType::TI(false).GetTypeExpr() },
      { temporalalgebra::MInt::BasicType(), temporalalgebra::RInt::
        BasicType(), MIntsType::TI(false).GetTypeExpr() },
      { temporalalgebra::MPoint::BasicType(), Point::BasicType(), 
        MPointsType::TI(false).GetTypeExpr() },
      { temporalalgebra::MPoint::BasicType(), Region::BasicType(), 
        MPointsType::TI(false).GetTypeExpr() },
    };
  }
  
  int AtOperator::ValueMapping00(ArgVector args, Word &result, 
    int message, Word &local, Supplier s) 
  {
    static_cast<MInts*>(args[0].addr)->at( 
        *static_cast<CcInt*>(args[1].addr), 
        qp->ResultStorage<MInts>(result, s) );
    return 0;
  }
  
  int AtOperator::ValueMapping01(ArgVector args, Word &result, 
    int message, Word &local, Supplier s) 
  {
    static_cast<MInts*>(args[0].addr)->at( 
        *static_cast<temporalalgebra::RInt*>(args[1].addr), 
        qp->ResultStorage<MInts>(result, s) );
    return 0;
  }
  
  int AtOperator::ValueMapping10(ArgVector args, Word &result, 
    int message, Word &local, Supplier s) 
  {
    static_cast<MPoints*>(args[0].addr)->at( 
        *static_cast<Point*>(args[1].addr), 
        qp->ResultStorage<MPoints>(result, s) );
    return 0;
  }
  
  int AtOperator::ValueMapping11(ArgVector args, Word &result, 
    int message, Word &local, Supplier s) 
  {
    static_cast<MPoints*>(args[0].addr)->at( 
        *static_cast<Region*>(args[1].addr), 
        qp->ResultStorage<MPoints>(result, s) );
    return 0;
  }



  const OperatorInfo AddRandomOperator::info = OperatorInfo(
      "addrandom",
      "(mpoints) x int -> (mpoints)",
      "_ addrandom _",
      "adds random data to the moving object for testing purposes",
      "query movingPoints addrandom 100");

  ValueMapping AddRandomOperator::valueMappings[] = {
    ValueMapping0,
    nullptr
  };

  ListExpr AddRandomOperator::TypeMapping(ListExpr args) {
    return typeMapping(signatures(), args);
  }

  int AddRandomOperator::SelectValueMapping(ListExpr args) {
    return selectValueMapping(signatures(), args);
  }
  
  list<AttrArrayOperatorSignatur> AddRandomOperator::signatures() {
    return list<AttrArrayOperatorSignatur> {
      { temporalalgebra::MPoint::BasicType(), CcInt::BasicType(), 
        MPointsType::TI(false).GetTypeExpr() },
    };
  }
  
  int AddRandomOperator::ValueMapping0(ArgVector args, Word &result, 
    int message, Word &local, Supplier s) 
  {
    static_cast<MPoints*>(args[0].addr)->addRandomRows( 
        *static_cast<CcInt*>(args[1].addr), 
        qp->ResultStorage<MPoints>(result, s) );
    return 0;
  }



  const OperatorInfo IndexOperator::info = OperatorInfo(
      "index",
      "mpoints x [int, int] -> mpoints",
      "_ index [_, _]",
      "returns an mpoints with the data of the mpoints given as first \n"
      "argument. the new mpoints is indexed with a grid index, which splits \n"
      "the time dimension and location dimension \n"
      "as often as determined by the second and third argument",
      "query movingPoints index [100, 100]");

  ValueMapping IndexOperator::valueMappings[] = {
    ValueMapping0,
    nullptr
  };

  ListExpr IndexOperator::TypeMapping(ListExpr args) {
    if(!nl->HasLength(args,3)) 
      return NList::typeError("Three arguments expected.");

    const ListExpr firstArg = nl->First(args);

    CRelAlgebra::AttrArrayTypeConstructor *typeConstructorA =
      CRelAlgebra::AttrArray::GetTypeConstructor(firstArg);

    if (typeConstructorA == nullptr) 
      return NList::typeError("First Argument isn't of kind ATTRARRAY.");

    const ListExpr attributeType = typeConstructorA->GetAttributeType(firstArg,
                                                                        false);

    if (!nl->IsEqual(attributeType, temporalalgebra::MPoint::BasicType())) 
      return NList::typeError("First Argument isn't of type ATTRARRAY(MPOINT)");

    const ListExpr secondArg = nl->Second(args);

    if (!nl->IsEqual(secondArg, CcInt::BasicType())) 
      return NList::typeError("Second Argument isn't of type INT");
        
    const ListExpr thirdArg = nl->Third(args);

    if (!nl->IsEqual(thirdArg, CcInt::BasicType())) 
      return NList::typeError("Third Argument isn't of type INT");

		return MPointsType::TI(false).GetTypeExpr();       
  }

  int IndexOperator::SelectValueMapping(ListExpr args) {
    return 0;
  }
  
  int IndexOperator::ValueMapping0(ArgVector args, Word &result, 
    int message, Word &local, Supplier s) 
  {
    static_cast<MPoints*>(args[0].addr)->index( 
        *static_cast<CcInt*>(args[1].addr), *static_cast<CcInt*>(args[2].addr), 
        qp->ResultStorage<MPoints>(result, s) );
    return 0;
  }

}
