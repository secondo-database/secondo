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

1 Operators.cpp

*/

#include "Operators.h"

#include "TemporalAlgebra.h"
#include "Types.h"
#include "LongIntsTC.h"
#include "Ints.h"
#include <chrono>

namespace ColumnMovingAlgebra
{

/*
1.1 Implementation of the class PresentOperator

The following implementations of operators are very similar. They consist of
a ~OperatorInfo~ structure which contains information for the user interface,
a list of value mapping functions, a type mapping function and value mapping
function (which both call the generic implementation in the base class),
a function that returns all signatures of the operator and finally the
value mapping functions. The value mapping functions will only be responsible
for some type casts and will then call the appropriate functions of the
corresponding attribut arrays.

*/

  const OperatorInfo PresentOperator::info = OperatorInfo(
      "present",
      "mbools   x (instant | periods) -> longints \n"
      "mints    x (instant | periods) -> longints \n"
      "mstrings x (instant | periods) -> longints \n"
      "mreals   x (instant | periods) -> longints \n"
      "mpoints  x (instant | periods) -> longints \n"
      "mregions x (instant | periods) -> longints",
      "_ present _",
      "returns the indices for all moving objects, \n"
      "that are defined at the given instant or periods",
      "query mpts present [const instant value \"2005-11-20-07:01:44\"]");

  ValueMapping PresentOperator::valueMappings[] = {
    ValueMapping00,
    ValueMapping01,
    ValueMapping10,
    ValueMapping11,
    ValueMapping20,
    ValueMapping21,
    ValueMapping30,
    ValueMapping31,
    ValueMapping40,
    ValueMapping41,
    ValueMapping50,
    ValueMapping51,
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
      { temporalalgebra::MInt::BasicType(), 
        Instant::BasicType(), 
        CRelAlgebra::LongIntsTI(false).GetTypeExpr() },
      { temporalalgebra::MInt::BasicType(), 
        temporalalgebra::Periods::BasicType(), 
        CRelAlgebra::LongIntsTI(false).GetTypeExpr() },
      { temporalalgebra::MPoint::BasicType(), 
        Instant::BasicType(), 
        CRelAlgebra::LongIntsTI(false).GetTypeExpr() },
      { temporalalgebra::MPoint::BasicType(), 
        temporalalgebra::Periods::BasicType(), 
        CRelAlgebra::LongIntsTI(false).GetTypeExpr() },
      { temporalalgebra::MRegion::BasicType(), 
        Instant::BasicType(), 
        CRelAlgebra::LongIntsTI(false).GetTypeExpr() },
      { temporalalgebra::MRegion::BasicType(), 
        temporalalgebra::Periods::BasicType(), 
        CRelAlgebra::LongIntsTI(false).GetTypeExpr() },
      { temporalalgebra::MReal::BasicType(), 
        Instant::BasicType(), 
        CRelAlgebra::LongIntsTI(false).GetTypeExpr() },
      { temporalalgebra::MReal::BasicType(), 
        temporalalgebra::Periods::BasicType(), 
        CRelAlgebra::LongIntsTI(false).GetTypeExpr() },
      { temporalalgebra::MBool::BasicType(), 
        Instant::BasicType(), 
        CRelAlgebra::LongIntsTI(false).GetTypeExpr() },
      { temporalalgebra::MBool::BasicType(), 
        temporalalgebra::Periods::BasicType(), 
        CRelAlgebra::LongIntsTI(false).GetTypeExpr() },
      { temporalalgebra::MString::BasicType(), 
        Instant::BasicType(), 
        CRelAlgebra::LongIntsTI(false).GetTypeExpr() },
      { temporalalgebra::MString::BasicType(), 
        temporalalgebra::Periods::BasicType(), 
        CRelAlgebra::LongIntsTI(false).GetTypeExpr() },
    };
  }
  
  int PresentOperator::ValueMapping00(ArgVector args, Word &result, 
    int message, Word &local, Supplier s) 
  {
    static_cast<MInts*>(args[0].addr)->present( 
        *static_cast<Instant*>(args[1].addr), 
        qp->ResultStorage<CRelAlgebra::LongInts>(result, s) );
    return 0;
  }

  int PresentOperator::ValueMapping01(ArgVector args, Word &result, 
    int message, Word &local, Supplier s) 
  {
    static_cast<MInts*>(args[0].addr)->present( 
        *static_cast<temporalalgebra::Periods*>(args[1].addr), 
        qp->ResultStorage<CRelAlgebra::LongInts>(result, s) );
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
  
  int PresentOperator::ValueMapping20(ArgVector args, Word &result, 
    int message, Word &local, Supplier s) 
  {
    static_cast<MRegions*>(args[0].addr)->present( 
        *static_cast<Instant*>(args[1].addr), 
        qp->ResultStorage<CRelAlgebra::LongInts>(result, s) );
    return 0;
  }

  int PresentOperator::ValueMapping21(ArgVector args, Word &result, 
    int message, Word &local, Supplier s) 
  {
    static_cast<MRegions*>(args[0].addr)->present( 
        *static_cast<temporalalgebra::Periods*>(args[1].addr), 
        qp->ResultStorage<CRelAlgebra::LongInts>(result, s) );
    return 0;
  }
  
  int PresentOperator::ValueMapping30(ArgVector args, Word &result, 
    int message, Word &local, Supplier s) 
  {
    static_cast<MReals*>(args[0].addr)->present( 
        *static_cast<Instant*>(args[1].addr), 
        qp->ResultStorage<CRelAlgebra::LongInts>(result, s) );
    return 0;
  }

  int PresentOperator::ValueMapping31(ArgVector args, Word &result, 
    int message, Word &local, Supplier s) 
  {
    static_cast<MReals*>(args[0].addr)->present( 
        *static_cast<temporalalgebra::Periods*>(args[1].addr), 
        qp->ResultStorage<CRelAlgebra::LongInts>(result, s) );
    return 0;
  }
  
  int PresentOperator::ValueMapping40(ArgVector args, Word &result, 
    int message, Word &local, Supplier s) 
  {
    static_cast<MBools*>(args[0].addr)->present( 
        *static_cast<Instant*>(args[1].addr), 
        qp->ResultStorage<CRelAlgebra::LongInts>(result, s) );
    return 0;
  }

  int PresentOperator::ValueMapping41(ArgVector args, Word &result, 
    int message, Word &local, Supplier s) 
  {
    static_cast<MBools*>(args[0].addr)->present( 
        *static_cast<temporalalgebra::Periods*>(args[1].addr), 
        qp->ResultStorage<CRelAlgebra::LongInts>(result, s) );
    return 0;
  }
  
  int PresentOperator::ValueMapping50(ArgVector args, Word &result, 
    int message, Word &local, Supplier s) 
  {
    static_cast<MStrings*>(args[0].addr)->present( 
        *static_cast<Instant*>(args[1].addr), 
        qp->ResultStorage<CRelAlgebra::LongInts>(result, s) );
    return 0;
  }

  int PresentOperator::ValueMapping51(ArgVector args, Word &result, 
    int message, Word &local, Supplier s) 
  {
    static_cast<MStrings*>(args[0].addr)->present( 
        *static_cast<temporalalgebra::Periods*>(args[1].addr), 
        qp->ResultStorage<CRelAlgebra::LongInts>(result, s) );
    return 0;
  }

/*
1.2 Implementation of the class AtInstantOperator

*/

  const OperatorInfo AtInstantOperator::info = OperatorInfo(
      "atinstant",
      "mbools   x instant -> ibools  \n"
      "mints    x instant -> iints  \n"
      "mstrings x instant -> istrings  \n"
      "mreals   x instant -> ireals   \n"
      "mpoints  x instant -> ipoints  \n"
      "mregions x instant -> iregions ",
      "_ atinstant _",
      "returns the value of the moving objects for the given instant",
      "query mpts atinstant [const instant value \"2005-11-20-07:01:44\"]");

  ValueMapping AtInstantOperator::valueMappings[] = {
    ValueMapping0,
    ValueMapping1,
    ValueMapping2,
    ValueMapping3,
    ValueMapping4,
    ValueMapping5,
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
      { temporalalgebra::MInt::BasicType(), 
        Instant::BasicType(), 
        IIntsType::TI(false).GetTypeExpr() },
      { temporalalgebra::MPoint::BasicType(), 
        Instant::BasicType(), 
        IPointsType::TI(false).GetTypeExpr() },
      { temporalalgebra::MRegion::BasicType(), 
        Instant::BasicType(), 
        IRegionsType::TI(false).GetTypeExpr() },
      { temporalalgebra::MReal::BasicType(), 
        Instant::BasicType(), 
        IRealsType::TI(false).GetTypeExpr() },
      { temporalalgebra::MBool::BasicType(), 
        Instant::BasicType(), 
        IBoolsType::TI(false).GetTypeExpr() },
      { temporalalgebra::MString::BasicType(), 
        Instant::BasicType(), 
        IStringsType::TI(false).GetTypeExpr() },
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
  
  int AtInstantOperator::ValueMapping2(ArgVector args, Word &result, 
    int message, Word &local, Supplier s) 
  {
    static_cast<MRegions*>(args[0].addr)->atInstant( 
        *static_cast<Instant*>(args[1].addr), 
        qp->ResultStorage<IRegions>(result, s) );
    return 0;
  }
  
  int AtInstantOperator::ValueMapping3(ArgVector args, Word &result, 
    int message, Word &local, Supplier s) 
  {
    static_cast<MReals*>(args[0].addr)->atInstant( 
        *static_cast<Instant*>(args[1].addr), 
        qp->ResultStorage<IReals>(result, s) );
    return 0;
  }
  
  int AtInstantOperator::ValueMapping4(ArgVector args, Word &result, 
    int message, Word &local, Supplier s) 
  {
    static_cast<MBools*>(args[0].addr)->atInstant( 
        *static_cast<Instant*>(args[1].addr), 
        qp->ResultStorage<IBools>(result, s) );
    return 0;
  }
  
  int AtInstantOperator::ValueMapping5(ArgVector args, Word &result, 
    int message, Word &local, Supplier s) 
  {
    static_cast<MStrings*>(args[0].addr)->atInstant( 
        *static_cast<Instant*>(args[1].addr), 
        qp->ResultStorage<IStrings>(result, s) );
    return 0;
  }

/*
1.2 Implementation of the class AtPeriodsOperator

*/

  const OperatorInfo AtPeriodsOperator::info = OperatorInfo(
      "atperiods",
      "mbools   x periods -> mbools  \n"
      "mints    x periods -> mints  \n"
      "mstrings x periods -> mstrings  \n"
      "mreals   x periods -> mreals   \n"
      "mpoints  x periods -> mpoints  \n"
      "mregions x periods -> mregions ",
      "_ atperiods _",
      "restricts the moving objects to the given periods",
      "query movingIntegers atperiods [const periods value \n"
      "(\"2005-11-20-07:01:44\" \"2005-11-20-07:01:44\" TRUE TRUE)]");

  ValueMapping AtPeriodsOperator::valueMappings[] = {
    ValueMapping0,
    ValueMapping1,
    ValueMapping2,
    ValueMapping3,
    ValueMapping4,
    ValueMapping5,
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
      { temporalalgebra::MInt::BasicType(),   
        temporalalgebra::Periods::BasicType(), 
        MIntsType::TI(false).GetTypeExpr() },
      { temporalalgebra::MPoint::BasicType(), 
        temporalalgebra::Periods::BasicType(), 
        MPointsType::TI(false).GetTypeExpr() },
      { temporalalgebra::MRegion::BasicType(), 
        temporalalgebra::Periods::BasicType(), 
        MRegionsType::TI(false).GetTypeExpr() },
      { temporalalgebra::MReal::BasicType(), 
        temporalalgebra::Periods::BasicType(), 
        MRealsType::TI(false).GetTypeExpr() },
      { temporalalgebra::MBool::BasicType(), 
        temporalalgebra::Periods::BasicType(), 
        MBoolsType::TI(false).GetTypeExpr() },
      { temporalalgebra::MString::BasicType(), 
        temporalalgebra::Periods::BasicType(), 
        MStringsType::TI(false).GetTypeExpr() },
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
  
  int AtPeriodsOperator::ValueMapping2(ArgVector args, Word &result, 
    int message, Word &local, Supplier s) 
  {
    static_cast<MRegions*>(args[0].addr)->atPeriods( 
        *static_cast<temporalalgebra::Periods*>(args[1].addr), 
        qp->ResultStorage<MRegions>(result, s) );
    return 0;
  }
  
  int AtPeriodsOperator::ValueMapping3(ArgVector args, Word &result, 
    int message, Word &local, Supplier s) 
  {
    static_cast<MReals*>(args[0].addr)->atPeriods( 
        *static_cast<temporalalgebra::Periods*>(args[1].addr), 
        qp->ResultStorage<MReals>(result, s) );
    return 0;
  }
  
  int AtPeriodsOperator::ValueMapping4(ArgVector args, Word &result, 
    int message, Word &local, Supplier s) 
  {
    static_cast<MBools*>(args[0].addr)->atPeriods( 
        *static_cast<temporalalgebra::Periods*>(args[1].addr), 
        qp->ResultStorage<MBools>(result, s) );
    return 0;
  }
  
  int AtPeriodsOperator::ValueMapping5(ArgVector args, Word &result, 
    int message, Word &local, Supplier s) 
  {
    static_cast<MStrings*>(args[0].addr)->atPeriods( 
        *static_cast<temporalalgebra::Periods*>(args[1].addr), 
        qp->ResultStorage<MStrings>(result, s) );
    return 0;
  }

/*
1.2 Implementation of the class PassesOperator

*/

  const OperatorInfo PassesOperator::info = OperatorInfo(
      "passes",
      "mbools  x (bool   | rbool  ) -> longints \n"
      "mints   x (int    | rint   ) -> longints \n"
      "mstrins x (string | rstring) -> longints \n"
      "mreals  x (real   | rreal  ) -> longints \n"
      "mpoints x (point  | region ) -> longints ",
      "_ passes _",
      "returns the indices for all moving objects, \n"
      "that pass the given value at any time",
      "query movingIntegers passes 1");

  ValueMapping PassesOperator::valueMappings[] = {
    ValueMapping00,
    ValueMapping01,
    ValueMapping10,
    ValueMapping11,
    ValueMapping20,
    ValueMapping21,
    ValueMapping30,
    ValueMapping31,
    ValueMapping40,
    ValueMapping41,
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
      { temporalalgebra::MInt::BasicType(),   
        CcInt::BasicType(), 
        CRelAlgebra::LongIntsTI(false).GetTypeExpr() },
      { temporalalgebra::MInt::BasicType(),   
        temporalalgebra::RInt::BasicType(), 
        CRelAlgebra::LongIntsTI(false).GetTypeExpr() },
      { temporalalgebra::MPoint::BasicType(), 
        Point::BasicType(), 
        CRelAlgebra::LongIntsTI(false).GetTypeExpr() },
      { temporalalgebra::MPoint::BasicType(), 
        Region::BasicType(), 
        CRelAlgebra::LongIntsTI(false).GetTypeExpr() },
      { temporalalgebra::MReal::BasicType(), 
        CcReal::BasicType(), 
        CRelAlgebra::LongIntsTI(false).GetTypeExpr() },
      { temporalalgebra::MReal::BasicType(), 
        temporalalgebra::RReal::BasicType(), 
        CRelAlgebra::LongIntsTI(false).GetTypeExpr() },
      { temporalalgebra::MBool::BasicType(), 
        CcBool::BasicType(), 
        CRelAlgebra::LongIntsTI(false).GetTypeExpr() },
      { temporalalgebra::MBool::BasicType(), 
        temporalalgebra::RBool::BasicType(), 
        CRelAlgebra::LongIntsTI(false).GetTypeExpr() },
      { temporalalgebra::MString::BasicType(), 
        CcString::BasicType(), 
        CRelAlgebra::LongIntsTI(false).GetTypeExpr() },
      { temporalalgebra::MString::BasicType(), 
        temporalalgebra::RString::BasicType(), 
        CRelAlgebra::LongIntsTI(false).GetTypeExpr() },
    };
  }
  
  int PassesOperator::ValueMapping00(ArgVector args, Word &result, int message, 
    Word &local, Supplier s) 
  {
    static_cast<MInts*>(args[0].addr)->passes( 
        *static_cast<CcInt*>(args[1].addr), 
        qp->ResultStorage<CRelAlgebra::LongInts>(result, s) );
    return 0;
  }
  
  int PassesOperator::ValueMapping01(ArgVector args, Word &result, int message, 
    Word &local, Supplier s) 
  {
    static_cast<MInts*>(args[0].addr)->passes( 
        *static_cast<temporalalgebra::RInt*>(args[1].addr), 
        qp->ResultStorage<CRelAlgebra::LongInts>(result, s) );
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
  
  int PassesOperator::ValueMapping20(ArgVector args, Word &result, int message, 
    Word &local, Supplier s) 
  {
    static_cast<MReals*>(args[0].addr)->passes( 
        *static_cast<CcReal*>(args[1].addr), 
        qp->ResultStorage<CRelAlgebra::LongInts>(result, s) );
    return 0;
  }
  
  int PassesOperator::ValueMapping21(ArgVector args, Word &result, int message, 
    Word &local, Supplier s) 
  {
    static_cast<MReals*>(args[0].addr)->passes( 
        *static_cast<temporalalgebra::RReal*>(args[1].addr), 
        qp->ResultStorage<CRelAlgebra::LongInts>(result, s) );
    return 0;
  }
  
  int PassesOperator::ValueMapping30(ArgVector args, Word &result, int message, 
    Word &local, Supplier s) 
  {
    static_cast<MBools*>(args[0].addr)->passes( 
        *static_cast<CcBool*>(args[1].addr), 
        qp->ResultStorage<CRelAlgebra::LongInts>(result, s) );
    return 0;
  }
  
  int PassesOperator::ValueMapping31(ArgVector args, Word &result, int message, 
    Word &local, Supplier s) 
  {
    static_cast<MBools*>(args[0].addr)->passes( 
        *static_cast<temporalalgebra::RBool*>(args[1].addr), 
        qp->ResultStorage<CRelAlgebra::LongInts>(result, s) );
    return 0;
  }
  
  int PassesOperator::ValueMapping40(ArgVector args, Word &result, int message, 
    Word &local, Supplier s) 
  {
    static_cast<MStrings*>(args[0].addr)->passes( 
        *static_cast<CcString*>(args[1].addr), 
        qp->ResultStorage<CRelAlgebra::LongInts>(result, s) );
    return 0;
  }
  
  int PassesOperator::ValueMapping41(ArgVector args, Word &result, int message, 
    Word &local, Supplier s) 
  {
    static_cast<MStrings*>(args[0].addr)->passes( 
        *static_cast<temporalalgebra::RString*>(args[1].addr), 
        qp->ResultStorage<CRelAlgebra::LongInts>(result, s) );
    return 0;
  }

/*
1.2 Implementation of the class AtOperator

*/

  const OperatorInfo AtOperator::info = OperatorInfo(
      "at",
      "mbools   x (bool   | rbool   ) -> mbools  \n"
      "mints    x (int    | rint    ) -> mints  \n"
      "mstrings x (string | rstring ) -> mstrings  \n"
      "mreals   x (real   | real    ) -> mreals \n"
      "mpoints  x (point  | region  ) -> mpoints",
      "_ at _",
      "restricts the given moving objects to the given value or ranges",
      "query movingIntegers at 1");

  ValueMapping AtOperator::valueMappings[] = {
    ValueMapping00,
    ValueMapping01,
    ValueMapping10,
    ValueMapping11,
    ValueMapping20,
    ValueMapping21,
    ValueMapping30,
    ValueMapping31,
    ValueMapping40,
    ValueMapping41,
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
      { temporalalgebra::MInt::BasicType(), 
        CcInt::BasicType(), 
        MIntsType::TI(false).GetTypeExpr() },
      { temporalalgebra::MInt::BasicType(), 
        temporalalgebra::RInt::BasicType(), 
        MIntsType::TI(false).GetTypeExpr() },
      { temporalalgebra::MPoint::BasicType(), 
        Point::BasicType(), 
        MPointsType::TI(false).GetTypeExpr() },
      { temporalalgebra::MPoint::BasicType(), 
        Region::BasicType(), 
        MPointsType::TI(false).GetTypeExpr() },
      { temporalalgebra::MReal::BasicType(), 
        CcReal::BasicType(), 
        MRealsType::TI(false).GetTypeExpr() },
      { temporalalgebra::MReal::BasicType(), 
        temporalalgebra::RReal::BasicType(), 
        MRealsType::TI(false).GetTypeExpr() },
      { temporalalgebra::MBool::BasicType(), 
        CcBool::BasicType(), 
        MBoolsType::TI(false).GetTypeExpr() },
      { temporalalgebra::MBool::BasicType(), 
        temporalalgebra::RBool::BasicType(), 
        MBoolsType::TI(false).GetTypeExpr() },
      { temporalalgebra::MString::BasicType(), 
        CcString::BasicType(), 
        MStringsType::TI(false).GetTypeExpr() },
      { temporalalgebra::MString::BasicType(), 
        temporalalgebra::RString::BasicType(), 
        MStringsType::TI(false).GetTypeExpr() },
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

  int AtOperator::ValueMapping20(ArgVector args, Word &result, 
    int message, Word &local, Supplier s) 
  {
    static_cast<MReals*>(args[0].addr)->at( 
        *static_cast<CcReal*>(args[1].addr), 
        qp->ResultStorage<MReals>(result, s) );
    return 0;
  }
  
  int AtOperator::ValueMapping21(ArgVector args, Word &result, 
    int message, Word &local, Supplier s) 
  {
    static_cast<MReals*>(args[0].addr)->at( 
        *static_cast<temporalalgebra::RReal*>(args[1].addr), 
        qp->ResultStorage<MReals>(result, s) );
    return 0;
  }

  int AtOperator::ValueMapping30(ArgVector args, Word &result, 
    int message, Word &local, Supplier s) 
  {
    static_cast<MBools*>(args[0].addr)->at( 
        *static_cast<CcBool*>(args[1].addr), 
        qp->ResultStorage<MBools>(result, s) );
    return 0;
  }
  
  int AtOperator::ValueMapping31(ArgVector args, Word &result, 
    int message, Word &local, Supplier s) 
  {
    static_cast<MBools*>(args[0].addr)->at( 
        *static_cast<temporalalgebra::RBool*>(args[1].addr), 
        qp->ResultStorage<MBools>(result, s) );
    return 0;
  }

  int AtOperator::ValueMapping40(ArgVector args, Word &result, 
    int message, Word &local, Supplier s) 
  {
    static_cast<MStrings*>(args[0].addr)->at( 
        *static_cast<CcString*>(args[1].addr), 
        qp->ResultStorage<MStrings>(result, s) );
    return 0;
  }
  
  int AtOperator::ValueMapping41(ArgVector args, Word &result, 
    int message, Word &local, Supplier s) 
  {
    static_cast<MStrings*>(args[0].addr)->at( 
        *static_cast<temporalalgebra::RString*>(args[1].addr), 
        qp->ResultStorage<MStrings>(result, s) );
    return 0;
  }

/*
1.2 Implementation of the class InsideOperator

*/

  const OperatorInfo InsideOperator::info = OperatorInfo(
      "inside",
      "mpoint  x mregions -> mbools \n"
      "mpoints x region   -> mbools \n"
      "mpoints x mregion  -> mbools \n"
      "mpoints x mregions -> mbools ",
      "_ inside _",
      "returns a mbools which indicates, when the given moving points \n"
      "are inside the given region or mregion",
      "query movingPoints inside myRegion");

  ValueMapping InsideOperator::valueMappings[] = {
    ValueMapping0,
    ValueMapping1,
    ValueMapping2,
    ValueMapping3,
    nullptr
  };

  ListExpr InsideOperator::TypeMapping(ListExpr args) {
    int r = IntersectionOperator::mapping(args);
           
    if (r == -1)
      return NList::typeError("\nExpected one of the following operants:\n"
        "mpoints x region\n"
        "mpoints x mregion\n"
        "mpoint  x mregions\n"
        "mpoints x mregions");
    
    return MBoolsType::TI(false).GetTypeExpr();       
  }

  int InsideOperator::SelectValueMapping(ListExpr args) {
    int r = IntersectionOperator::mapping(args);
    assert(r != -1);
    return r;
  }
  
  int InsideOperator::ValueMapping0(ArgVector args, Word &result, 
    int message, Word &local, Supplier s) 
  {
    auto mps = static_cast<MPoints*>(args[0].addr);
    auto r   = static_cast<Region*>(args[1].addr);
    int64_t min, max;
    mps->getDefTimeLimits(min, max);
    MRegions mrs;
    mrs.addConstMRegion(*r, Interval { min, max, true, true } );
    MPoints intermediate;
    mrs.intersection(*mps, intermediate);
    mps->defTimeIntersection(intermediate, 
                             qp->ResultStorage<MBools>(result, s));
    return 0;
  }
  
  int InsideOperator::ValueMapping1(ArgVector args, Word &result, 
    int message, Word &local, Supplier s) 
  {
    auto mps = static_cast<MPoints*>(args[0].addr);
    auto mr  = static_cast<temporalalgebra::MRegion*>(args[1].addr);
    MRegions mrs;
    mrs.Append(*mr);
    MPoints intermediate;
    mrs.intersection(*mps, intermediate);
    mps->defTimeIntersection(intermediate, 
                             qp->ResultStorage<MBools>(result, s));
    return 0;
  }
  
  int InsideOperator::ValueMapping2(ArgVector args, Word &result, 
    int message, Word &local, Supplier s) 
  {
    auto mp  = static_cast<temporalalgebra::MPoint*>(args[0].addr);
    auto mrs = static_cast<MRegions*>(args[1].addr);
    MPoints mps;
    mps.Append(*mp);
    MPoints intermediate;
    mrs->intersection(mps, intermediate);
    mps.defTimeIntersection(intermediate, 
                            qp->ResultStorage<MBools>(result, s));
    return 0;
  }
  
  int InsideOperator::ValueMapping3(ArgVector args, Word &result, 
    int message, Word &local, Supplier s) 
  {
    auto mps = static_cast<MPoints*>(args[0].addr);
    auto mrs = static_cast<MRegions*>(args[1].addr);
    MPoints intermediate;
    mrs->intersection(*mps, intermediate);
    mps->defTimeIntersection(intermediate, 
                             qp->ResultStorage<MBools>(result, s));
    return 0;
  }

/*
1.2 Implementation of the class IntersectionOperator

*/

  const OperatorInfo IntersectionOperator::info = OperatorInfo(
      "intersection",
      "mpoint  x mregions -> mpoints \n"
      "mpoints x region   -> mpoints \n"
      "mpoints x mregion  -> mpoints \n"
      "mpoints x mregions -> mpoints ",
      "intersection (_, _)",
      "restricts the moving point to the given region or mregion",
      "query intersection (movingPoints, myRegion)");

  ValueMapping IntersectionOperator::valueMappings[] = {
    ValueMapping0,
    ValueMapping1,
    ValueMapping2,
    ValueMapping3,
    nullptr
  };

  int IntersectionOperator::mapping(ListExpr args) {
    if(!nl->HasLength(args,2)) 
      return -1;

    const ListExpr a = nl->First(args);
    const ListExpr b = nl->Second(args);

    if (nl->IsEqual(a, temporalalgebra::MPoint::BasicType())) {
      CRelAlgebra::AttrArrayTypeConstructor *bTC =
        CRelAlgebra::AttrArray::GetTypeConstructor(b);

      if (bTC == nullptr) 
        return -1;

      const ListExpr bAttrT = bTC->GetAttributeType(b, false);

      if (nl->IsEqual(bAttrT, temporalalgebra::MRegion::BasicType())) 
        return 2;
    } else {
      CRelAlgebra::AttrArrayTypeConstructor *aTC =
        CRelAlgebra::AttrArray::GetTypeConstructor(a);

      if (aTC == nullptr) 
        return -1;

      const ListExpr aAttrT = aTC->GetAttributeType(b, false);

      if (!nl->IsEqual(aAttrT, temporalalgebra::MPoint::BasicType())) 
        return -1;
        
      if (nl->IsEqual(b, Region::BasicType())) 
        return 0;
      
      if (nl->IsEqual(b, temporalalgebra::MRegion::BasicType())) 
        return 1;

      CRelAlgebra::AttrArrayTypeConstructor *bTC =
        CRelAlgebra::AttrArray::GetTypeConstructor(b);

      if (bTC == nullptr) 
        return -1;

      const ListExpr bAttrT = bTC->GetAttributeType(b, false);

      if (nl->IsEqual(bAttrT, temporalalgebra::MRegion::BasicType())) 
        return 3;
    }

    return -1;
  }

  ListExpr IntersectionOperator::TypeMapping(ListExpr args) {
    int r = mapping(args);
           
    if (r == -1)
      return NList::typeError("\nExpected one of the following operants:\n"
        "mpoints x region\n"
        "mpoints x mregion\n"
        "mpoint  x mregions\n"
        "mpoints x mregions");
    
    return MPointsType::TI(false).GetTypeExpr();       
  }

  int IntersectionOperator::SelectValueMapping(ListExpr args) {
    int r = mapping(args);
    assert(r != -1);
    return r;
  }
  
  int IntersectionOperator::ValueMapping0(ArgVector args, Word &result, 
    int message, Word &local, Supplier s) 
  {
    auto mps = static_cast<MPoints*>(args[0].addr);
    auto r   = static_cast<Region*>(args[1].addr);
    int64_t min, max;
    mps->getDefTimeLimits(min, max);
    MRegions mrs;
    mrs.addConstMRegion(*r, Interval { min, max, true, true } );
    mrs.intersection(*mps, qp->ResultStorage<MPoints>(result, s));
    return 0;
  }
  
  int IntersectionOperator::ValueMapping1(ArgVector args, Word &result, 
    int message, Word &local, Supplier s) 
  {
    auto mps = static_cast<MPoints*>(args[0].addr);
    auto mr  = static_cast<temporalalgebra::MRegion*>(args[1].addr);
    MRegions mrs;
    mrs.Append(*mr);
    mrs.intersection(*mps, qp->ResultStorage<MPoints>(result, s));
    return 0;
  }
  
  int IntersectionOperator::ValueMapping2(ArgVector args, Word &result, 
    int message, Word &local, Supplier s) 
  {
    auto mp  = static_cast<temporalalgebra::MPoint*>(args[0].addr);
    auto mrs = static_cast<MRegions*>(args[1].addr);
    MPoints mps;
    mps.Append(*mp);
    mrs->intersection(mps, qp->ResultStorage<MPoints>(result, s));
    return 0;
  }
  
  int IntersectionOperator::ValueMapping3(ArgVector args, Word &result, 
    int message, Word &local, Supplier s) 
  {
    auto mps = static_cast<MPoints*>(args[0].addr);
    auto mrs = static_cast<MRegions*>(args[1].addr);
    mrs->intersection(*mps, qp->ResultStorage<MPoints>(result, s));
    return 0;
  }

/*
1.2 Implementation of the class AddRandomOperator

*/

  const OperatorInfo AddRandomOperator::info = OperatorInfo(
      "addrandom",
      "bools   x int -> bools \n"
      "ints    x int -> ints \n"
      "strings x int -> strings \n"
      "reals   x int -> reals \n"
      "mpoints x int -> mpoints ",
      "_ addrandom _",
      "adds random units to every moving object in the first argument. "
      "the second argument determines the number of random units to add. ",
      "query movingPoints addrandom 100");

  ValueMapping AddRandomOperator::valueMappings[] = {
    ValueMapping0,
    ValueMapping1,
    ValueMapping2,
    ValueMapping3,
    ValueMapping4,
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
      { temporalalgebra::MBool::BasicType(), CcInt::BasicType(), 
        MBoolsType::TI(false).GetTypeExpr() },
      { temporalalgebra::MInt::BasicType(), CcInt::BasicType(), 
        MIntsType::TI(false).GetTypeExpr() },
      { temporalalgebra::MString::BasicType(), CcInt::BasicType(), 
        MStringsType::TI(false).GetTypeExpr() },
      { temporalalgebra::MReal::BasicType(), CcInt::BasicType(), 
        MRealsType::TI(false).GetTypeExpr() },
      { temporalalgebra::MPoint::BasicType(), CcInt::BasicType(), 
        MPointsType::TI(false).GetTypeExpr() },
    };
  }
  
  int AddRandomOperator::ValueMapping0(ArgVector args, Word &result, 
    int message, Word &local, Supplier s) 
  {
    static_cast<MBools*>(args[0].addr)->addRandomUnits( 
        *static_cast<CcInt*>(args[1].addr), 
        qp->ResultStorage<MBools>(result, s) );
    return 0;
  }
  
  int AddRandomOperator::ValueMapping1(ArgVector args, Word &result, 
    int message, Word &local, Supplier s) 
  {
    static_cast<MInts*>(args[0].addr)->addRandomUnits( 
        *static_cast<CcInt*>(args[1].addr), 
        qp->ResultStorage<MInts>(result, s) );
    return 0;
  }
  
  int AddRandomOperator::ValueMapping2(ArgVector args, Word &result, 
    int message, Word &local, Supplier s) 
  {
    static_cast<MStrings*>(args[0].addr)->addRandomUnits( 
        *static_cast<CcInt*>(args[1].addr), 
        qp->ResultStorage<MStrings>(result, s) );
    return 0;
  }
  
  int AddRandomOperator::ValueMapping3(ArgVector args, Word &result, 
    int message, Word &local, Supplier s) 
  {
    static_cast<MReals*>(args[0].addr)->addRandomUnits( 
        *static_cast<CcInt*>(args[1].addr), 
        qp->ResultStorage<MReals>(result, s) );
    return 0;
  }
  
  int AddRandomOperator::ValueMapping4(ArgVector args, Word &result, 
    int message, Word &local, Supplier s) 
  {
    static_cast<MPoints*>(args[0].addr)->addRandomUnits( 
        *static_cast<CcInt*>(args[1].addr), 
        qp->ResultStorage<MPoints>(result, s) );
    return 0;
  }

/*
1.2 Implementation of the class IndexOperator

The index operator is implemented differently, as it has a more complex
signature.

*/

  const OperatorInfo IndexOperator::info = OperatorInfo(
      "index",
      "mpoints x "
      "[real, real, int, real, real, int, instant, instant, int] -> mpoints",
      "_ index [_, _, _, _, _, _, _, _, _]",
      "returns an mpoints with the data of the mpoints given as first "
      "argument. the returned mpoints is indexed with a grid index. \n"
      "the lower boundary of the grid in the dimension x, y and t "
      "are given by the second, fith and eights argument. \n" 
      "the upper boundary of the grid in the dimension x, y and t "
      "are given by the third, sixth and ninth argument. \n" 
      "the number of splits in the dimension x, y and t "
      "are given by the fourth, seventh and tenth argument. " ,
      "let indexedMovingPoints = movingPoints index "
      "[0.0, 100.0, 10, 0.0, 100.0, 10, "
      "[const instant value \"2000-01-01\"], "
      "[const instant value \"2010-01-01\"], 10]");

  ValueMapping IndexOperator::valueMappings[] = {
    ValueMapping0,
    nullptr
  };

  ListExpr IndexOperator::TypeMapping(ListExpr args) {
    if(!nl->HasLength(args, 10)) 
      return NList::typeError("Ten arguments expected.");

    const ListExpr firstArg = nl->First(args);

    CRelAlgebra::AttrArrayTypeConstructor *typeConstructorA =
      CRelAlgebra::AttrArray::GetTypeConstructor(firstArg);

    if (typeConstructorA == nullptr) 
      return NList::typeError("First Argument isn't of kind ATTRARRAY.");

    const ListExpr attributeType = typeConstructorA->GetAttributeType(firstArg,
                                                                        false);

    if (!nl->IsEqual(attributeType, temporalalgebra::MPoint::BasicType())) 
      return NList::typeError(
        "First Argument isn't of type ATTRARRAY(MPOINT)");

    for (int i : (const int []) {2, 3, 5 ,6})
      if (!nl->IsEqual(nl->Nth(i, args), CcReal::BasicType())) {
        return NList::typeError("Argument " + std::to_string(i) + 
                                " is not of type REAL");
    }

    for (int i : (const int []) {4, 7, 10})
      if (!nl->IsEqual(nl->Nth(i, args), CcInt::BasicType())) {
        return NList::typeError("Argument " + std::to_string(i) + 
                                " is not of type INT");
    }

    for (int i : (const int []) {8, 9})
      if (!Instant::checkType(nl->Nth(i, args))) {
        return NList::typeError("Argument " + std::to_string(i) + 
                                " is not of type INSTANT");
    }

    return MPointsType::TI(false).GetTypeExpr();       
  }

  int IndexOperator::SelectValueMapping(ListExpr args) {
    return 0;
  }
  
  int IndexOperator::ValueMapping0(ArgVector args, Word &result, 
    int message, Word &local, Supplier s) 
  {
    static_cast<MPoints*>(args[0].addr)->index( 
        *static_cast<CcReal*> (args[1].addr), 
        *static_cast<CcReal*> (args[2].addr), 
        *static_cast<CcInt*>  (args[3].addr), 
        *static_cast<CcReal*> (args[4].addr), 
        *static_cast<CcReal*> (args[5].addr), 
        *static_cast<CcInt*>  (args[6].addr), 
        *static_cast<Instant*>(args[7].addr), 
        *static_cast<Instant*>(args[8].addr), 
        *static_cast<CcInt*>  (args[9].addr), 
        qp->ResultStorage<MPoints>(result, s) );
    return 0;
  }

}
