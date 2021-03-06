/*
----
This file is part of SECONDO.

Copyright (C) 2013,
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


/*
1 PreciseAlgebra

This algebra provides precise types based on rationals of arbitrary size.


*/

#include "PrecSecTypes.h"
#include "Algebra.h"
#include "NestedList.h"
#include "QueryProcessor.h"
#include "StandardTypes.h"
#include "ListUtils.h"
#include "AlmostEqual.h"
#include "Symbols.h"

#include "Algebras/Standard-C++/RationalAttr.h"
#include "Algebras/Standard-C++/LongInt.h"
#include "Algebras/Spatial/Point.h"
#include "Algebras/Spatial/SpatialAlgebra.h"
#include "Algebras/FText/FTextAlgebra.h"
#include "Stream.h"
#include "Algebras/Relation-C++/RelationAlgebra.h"


extern NestedList* nl;
extern QueryProcessor *qp; 
extern AlgebraManager *am;

using namespace std;


namespace precise{

/*
1 Type constructors

*/
GenTC<PrecCoord> precise;
GenTC<PrecPoint> precisePoint;
GenTC<PrecPoints> precisePoints;
GenTC<PrecRegion> preciseRegion;
GenTC<PrecLine> preciseLine;

GenTC<PrecTime<datetime::instanttype> > preciseInstant;
GenTC<PrecTime<datetime::durationtype> > preciseDuration;


/*
2 Operators

2.1 Arithmetic Operations


2.1.1 Type Mapping

   precise x {precise, int, real, longint, rational} -> precise
   {precise, int, real, longint, rational} x precise -> precise

*/
bool isNumeric(ListExpr arg){
 return PrecCoord::checkType(arg) ||
        CcInt::checkType(arg) ||
        CcReal::checkType(arg) ||
        LongInt::checkType(arg) ||
        Rational::checkType(arg);
}


ListExpr arOpsTM(ListExpr args){
   string err = "precise x {precise, int, real, longint, rational} or \n" 
            "{precise, int, real, longint, rational} x precise -> precise"
            " expected";

    if(!nl->HasLength(args,2)){
        return listutils::typeError(err);
    }
    if(PrecCoord::checkType(nl->First(args))){
        if(!isNumeric(nl->Second(args))){
           return listutils::typeError(err);
        } else {
           return listutils::basicSymbol<PrecCoord>();
        }
    }
    if(!PrecCoord::checkType(nl->Second(args))){
       return listutils::typeError(err);
    }
    if(!isNumeric(nl->First(args))){
       return listutils::typeError(err);
    }
    return listutils::basicSymbol<PrecCoord>();
}

/*
2.1.2 Value Mapping

*/
enum arOps {PLUS,MINUS,MUL,DIVIDE};

template<class T1, class T2, int OP>
int arOpsVM (Word* args, Word& result, int message, Word& local,
            Supplier s ){
     T1* arg1 = (T1*) args[0].addr;
     T2* arg2 = (T2*) args[1].addr;

     result = qp->ResultStorage(s);
     PrecCoord* res = (PrecCoord*) result.addr;
     if(!arg1->IsDefined() || !arg2->IsDefined()){
       res->SetDefined(false);
       return 0;
     }

     MPrecCoordinate a1(arg1->GetValue());
     MPrecCoordinate a2(arg2->GetValue());

     switch(OP){
        case PLUS: 
                 res->setValue(a1+a2);
                 break;
        case MINUS:  
                 res->setValue( a1 - a2); 
                 break;
        case MUL:  
                 res->setValue( a1 * a2); 
                 break;
        case DIVIDE:  
                 if(a2 == 0){
                    res->SetDefined(false);
                 } else {
                     res->setValue(a1 / a2); 
                 }
                 break;
        default: assert(false);
     } 
     return 0;
}

/*
2.1.3 ValueMapping Arrays

*/
ValueMapping plusVM[] = {
     arOpsVM<PrecCoord,PrecCoord,PLUS>,
     arOpsVM<PrecCoord,CcInt,PLUS>,
     arOpsVM<PrecCoord,CcReal,PLUS>,
     arOpsVM<PrecCoord,LongInt,PLUS>,
     arOpsVM<PrecCoord,Rational,PLUS>,
     arOpsVM<CcInt,PrecCoord,PLUS>,
     arOpsVM<CcReal,PrecCoord,PLUS>,
     arOpsVM<LongInt,PrecCoord,PLUS>,
     arOpsVM<Rational,PrecCoord,PLUS>
  };

ValueMapping minusVM[] = {
     arOpsVM<PrecCoord,PrecCoord,MINUS>,
     arOpsVM<PrecCoord,CcInt,MINUS>,
     arOpsVM<PrecCoord,CcReal,MINUS>,
     arOpsVM<PrecCoord,LongInt,MINUS>,
     arOpsVM<PrecCoord,Rational,MINUS>,
     arOpsVM<CcInt,PrecCoord,MINUS>,
     arOpsVM<CcReal,PrecCoord,MINUS>,
     arOpsVM<LongInt,PrecCoord,MINUS>,
     arOpsVM<Rational,PrecCoord,MINUS>
  };
ValueMapping mulVM[] = {
     arOpsVM<PrecCoord,PrecCoord,MUL>,
     arOpsVM<PrecCoord,CcInt,MUL>,
     arOpsVM<PrecCoord,CcReal,MUL>,
     arOpsVM<PrecCoord,LongInt,MUL>,
     arOpsVM<PrecCoord,Rational,MUL>,
     arOpsVM<CcInt,PrecCoord,MUL>,
     arOpsVM<CcReal,PrecCoord,MUL>,
     arOpsVM<LongInt,PrecCoord,MUL>,
     arOpsVM<Rational,PrecCoord,MUL>
  };

ValueMapping divideVM[] = {
     arOpsVM<PrecCoord,PrecCoord,DIVIDE>,
     arOpsVM<PrecCoord,CcInt,DIVIDE>,
     arOpsVM<PrecCoord,CcReal,DIVIDE>,
     arOpsVM<PrecCoord,LongInt,DIVIDE>,
     arOpsVM<PrecCoord,Rational,DIVIDE>,
     arOpsVM<CcInt,PrecCoord,DIVIDE>,
     arOpsVM<CcReal,PrecCoord,DIVIDE>,
     arOpsVM<LongInt,PrecCoord,DIVIDE>,
     arOpsVM<Rational,PrecCoord,DIVIDE>
  };

/*
2.1.4 Selection function

*/
int arOpsSelect(ListExpr args){
   ListExpr arg1 = nl->First(args);
   ListExpr arg2 = nl->Second(args);
   if(PrecCoord::checkType(arg1)){
      if(PrecCoord::checkType(arg2)){
          return 0;
      }
      if(CcInt::checkType(arg2)){
          return 1;
      }
      if(CcReal::checkType(arg2)){
          return 2;
      }
      if(LongInt::checkType(arg2)){
          return 3;
      }
      if(Rational::checkType(arg2)){
          return 4;
      }
      return -1;
   }
   if(CcInt::checkType(arg1)){
       return 5;
   }
   if(CcReal::checkType(arg1)){
       return 6;
   }
   if(LongInt::checkType(arg1)){
       return 7;
   }
   if(Rational::checkType(arg1)){
       return 8;
   }
   return -1;
}

/*
2.1.5 Specification

*/
OperatorSpec getOpSpec(const string opName, const string opResult){
   OperatorSpec res(
        " numeric x numeric -> precise",
        " _ " + opName + " _",
        "Computes the " + opResult + " of the two arguments",
        "query a1 " + opName + " a2"
   );
   return res;
}

/*
2.1.6 Operator instances

*/
Operator plusOP(
  "+",
  getOpSpec("+","sum").getStr(),
  9,
  plusVM,
  arOpsSelect,
  arOpsTM
);

Operator minusOP(
  "-",
  getOpSpec("-","difference").getStr(),
  9,
  minusVM,
  arOpsSelect,
  arOpsTM
);

Operator mulOP(
  "*",
  getOpSpec("*","product").getStr(),
  9,
  mulVM,
  arOpsSelect,
  arOpsTM
);

Operator divideOP(
  "/",
  getOpSpec("/","quotient").getStr(),
  9,
  divideVM,
  arOpsSelect,
  arOpsTM
);

/*
2.2 Arithmetic operations for time types

2.2.1 plus

*/
ListExpr timePlusTM(ListExpr args){
  string err = "{precInstant, duration} x duration expected" ;
  if(!nl->HasLength(args,2)){
    return listutils::typeError(err);
  }
  if(     !precDuration::checkType(nl->Second(args))
     || (   !precDuration::checkType(nl->First(args)) 
         && !precInstant::checkType(nl->First(args)))){
    return listutils::typeError(err);
  }  
  return nl->First(args);
}

template<class T>
int timePlusVM1 (Word* args, Word& result, int message, Word& local,
            Supplier s ){

  T* arg1 = (T*) args[0].addr;
  precDuration* arg2 = (precDuration*) args[1].addr;
  result = qp->ResultStorage(s);
  T* res = (T*) result.addr;
  if(!arg1->IsDefined() || !arg2->IsDefined()){
     res->SetDefined(false);
  } else {
     *res = *arg1 + *arg2;
  }
  return 0;
}

ValueMapping timePlusVM[] = {
   timePlusVM1<precInstant>,
   timePlusVM1<precDuration>
};

int timePlusSelect(ListExpr args){
   ListExpr arg1 = nl->First(args);
   if(precInstant::checkType(arg1)) {
      return 0;
   }
   if(precDuration::checkType(arg1)) {
      return 1;
   }
   return -1;
}

OperatorSpec timePlusSpec(
        " a1 x precDuration -> a1 , where a1 in {precInstant, precDuration}",
        " _  +  _",
        "adds the arguments",
        "query a1 + a2"
 );


Operator timePlusOP(
  "+",
  timePlusSpec.getStr(),
  2,
  timePlusVM,
  timePlusSelect,
  timePlusTM
);


/*
~minus~

*/

ListExpr timeMinusTM(ListExpr args){
  string err = "instant x instant  | instant x duration | "
               "duration x duration expacted";
  if(!nl->HasLength(args,2)){
    return listutils::typeError(err);
  }
  ListExpr arg1 = nl->First(args);
  ListExpr arg2 = nl->Second(args);
  if(precDuration::checkType(arg1)){
     if(precDuration::checkType(arg2)){
       return listutils::basicSymbol<precDuration>();
     } else {
       return listutils::typeError(err);
     }
  }
  if(!precInstant::checkType(arg1)){
       return listutils::typeError(err);
  }
  if(precInstant::checkType(arg2)){
     return listutils::basicSymbol<precDuration>();
  } else if(precDuration::checkType(arg2)){
     return listutils::basicSymbol<precInstant>();
  } else {
       return listutils::typeError(err);
  }
}


template<class A1, class A2, class R>
int timeMinusVM1 (Word* args, Word& result, int message, Word& local,
            Supplier s ){

  A1* arg1 = (A1*) args[0].addr;
  A2* arg2 = (A2*) args[1].addr;
  result = qp->ResultStorage(s);
  R* res = (R*) result.addr;
  if(!arg1->IsDefined() || !arg2->IsDefined()){
     res->SetDefined(false);
  } else {
     res->setValue( arg1->GetValue() - arg2->GetValue());
  }
  return 0;
}

ValueMapping timeMinusVM[] = {
  timeMinusVM1<precInstant, precInstant, precDuration>,
  timeMinusVM1<precInstant, precDuration, precInstant>,
  timeMinusVM1<precDuration, precDuration, precDuration>
};

int timeMinusSelect(ListExpr args){
   if(precInstant::checkType(nl->First(args)) &&
      precInstant::checkType(nl->Second(args))){
      return 0;
   }
   if(precInstant::checkType(nl->First(args)) &&
      precDuration::checkType(nl->Second(args))){
      return 1;
   }
   if(precDuration::checkType(nl->First(args)) &&
      precDuration::checkType(nl->Second(args))){
      return 2;
   }
   return -1;
}


OperatorSpec timeMinusSpec(
   " i x i -> d, d x d -> d, i x d -> i, "
   "where i=precInstant, d = precDuration",
   " _  -  _",
   "subtracts the second argument from the first one",
   "query a1 - a2"
);


Operator timeMinusOP(
  "-",
  timeMinusSpec.getStr(),
  3,
  timeMinusVM,
  timeMinusSelect,
  timeMinusTM
);


/*

mul, divide

*/

ListExpr timeMulDivTM(ListExpr args){
   string err = "duration x numeric expected";
   if(!nl->HasLength(args,2)){
      return listutils::typeError(err);      
   }
   if(!precDuration::checkType(nl->First(args)) ||
      !isNumeric(nl->Second(args))){
     return listutils::typeError(err);
   }
   return listutils::basicSymbol<precDuration>();
}

template<class A2, bool isDivide>
int timeMulDivVM1 (Word* args, Word& result, int message, Word& local,
            Supplier s ){
   precDuration* a1 = (precDuration*) args[0].addr;
   A2* a2 = (A2*) args[1].addr;
   result = qp->ResultStorage(s);
   precDuration* res = (precDuration*) result.addr;
   if(!a1->IsDefined() || !a2->IsDefined()){
      res->SetDefined(false);
      return 0;
   } 
   if(isDivide){
     MPrecCoordinate mc(a2->GetValue()); 
     if(mc.isNull()){
        res->SetDefined(false);
     } else {
        res->setValue( a1->GetValue() / mc);
     }
  } else {
     res->setValue( a1->GetValue() * MPrecCoordinate(a2->GetValue()));
  }
  return 0;
}

ValueMapping timeDivVM[] = {
    timeMulDivVM1<PrecCoord,true>,
    timeMulDivVM1<CcInt,true>,
    timeMulDivVM1<CcReal,true>,
    timeMulDivVM1<LongInt,true>,
    timeMulDivVM1<Rational,true>
};

ValueMapping timeMulVM[] = {
    timeMulDivVM1<PrecCoord,false>,
    timeMulDivVM1<CcInt,false>,
    timeMulDivVM1<CcReal,false>,
    timeMulDivVM1<LongInt,false>,
    timeMulDivVM1<Rational,false>
};

int timeMulDivSelect(ListExpr args){
    ListExpr a = nl->Second(args);
    if(PrecCoord::checkType(a)) return 0;
    if(CcInt::checkType(a)) return 1;
    if(CcReal::checkType(a)) return 2;
    if(LongInt::checkType(a)) return 3;
    if(Rational::checkType(a)) return 4;
    return -1;
}

OperatorSpec timeDivSpec(
   " precDuration  x numeric -> precDuration ",
   " _  /  _",
   "Divides the first argument by the second one",
   "query a1 / a2"
);

OperatorSpec timeMulSpec(
   " precDuration  x numeric -> precDuration ",
   " _  * _",
   "Multiplies the first argument with the second one",
   "query a1 * a2"
);


Operator timeDivOP(
  "/",
  timeDivSpec.getStr(),
  5,
  timeDivVM,
  timeMulDivSelect,
  timeMulDivTM
);

Operator timeMulOP(
  "*",
  timeMulSpec.getStr(),
  5,
  timeMulVM,
  timeMulDivSelect,
  timeMulDivTM
);




/*
2.2 Operator toPrecise

This operator converts a numeric value into a precise value.

2.2.1 Type Mapping

*/

ListExpr toPreciseTM(ListExpr args){
  string err = "{int,real,longint, rational, precise, point, points} "
               " [x int [x bool [x int]]]} expected";
  int len = nl->ListLength(args);
  if(len<1 || len > 4){
     return listutils::typeError(err);
  }
  
  if(len>=2){
      ListExpr arg2 = nl->Second(args);
      if(!CcInt::checkType(arg2) ){
          return listutils::typeError(err);
      }
  }
  if(len>=3){
     ListExpr arg3 = nl->Third(args);
     if(!CcBool::checkType(arg3)){
          return listutils::typeError(err);
     }
  }
  if(len>=4){
     ListExpr arg4 = nl->Fourth(args);
     if(!CcInt::checkType(arg4)){
          return listutils::typeError(err);
     }
  }
  ListExpr resType;
  ListExpr arg1 = nl->First(args);
  if(isNumeric(arg1)){
     resType =  listutils::basicSymbol<PrecCoord>();
  } else if(Point::checkType(arg1)){
     resType = listutils::basicSymbol<PrecPoint>();
  } else if(Points::checkType(arg1)){
     resType = listutils::basicSymbol<PrecPoints>();
  } else if(Region::checkType(arg1)){
     resType = listutils::basicSymbol<PrecRegion>();
  } else if(Line::checkType(arg1)){
     resType = listutils::basicSymbol<PrecLine>();
  } else if(datetime::DateTime::checkType(arg1)){
     resType = listutils::basicSymbol<PrecTime<datetime::instanttype> >();
     if(len!=1){
       return listutils::typeError(err);
     }
  } else if(Duration::checkType(arg1)){
     resType = listutils::basicSymbol<PrecTime<datetime::durationtype> >();
     if(len!=1){
       return listutils::typeError(err);
     }
  } else  {
    return listutils::typeError(err);
  }

  switch(len){
     case 1: return nl->ThreeElemList( nl->SymbolAtom(Symbols::APPEND()),
                                       nl->ThreeElemList(nl->IntAtom(1), 
                                                       nl->BoolAtom(false),
                                                       nl->IntAtom(16)),
                                       resType);
     case 2 : return nl->ThreeElemList( nl->SymbolAtom(Symbols::APPEND()),
                                        nl->TwoElemList(nl->BoolAtom(false),
                                                        nl->IntAtom(16)),
                                        resType);
     case 3 : return nl->ThreeElemList( nl->SymbolAtom(Symbols::APPEND()),
                                        nl->OneElemList(nl->IntAtom(16)),
                                        resType);
     case 4 : return resType;
     default:  return listutils::typeError(err);
  }
}

/*
2.2.2 Value Mapping

*/


template<class S, class T>
int toPreciseVM1 (Word* args, Word& result, int message, Word& local,
            Supplier s ){

  result = qp->ResultStorage(s);
  T* res = (T*) result.addr;
  res->clear();
  S* arg = (S*) args[0].addr;
  CcInt* scale = (CcInt*) args[1].addr;
  CcBool* useStr = (CcBool*) args[2].addr;
  CcInt* digits = (CcInt*) args[3].addr;
  if(    !arg->IsDefined() || !scale->IsDefined() 
      || !useStr->IsDefined()|| !digits->IsDefined()){
     res->SetDefined(false);
     return 0;
  }
  if( (digits->GetValue() < 0) || (scale->GetValue() <= 0)){
     res->SetDefined(false);
     return 0;
  }
  res->readFrom(*arg, scale->GetValue(), useStr->GetValue(), 
                digits->GetValue());
  return 0;
}


/*
2.2.3 Value Mapping Array and Selection function

*/

ValueMapping toPreciseVM[] = {
     toPreciseVM1<PrecCoord,PrecCoord>,
     toPreciseVM1<CcInt,PrecCoord>,
     toPreciseVM1<CcReal,PrecCoord>,
     toPreciseVM1<LongInt,PrecCoord>,
     toPreciseVM1<Rational,PrecCoord>,
     toPreciseVM1<Point,PrecPoint>,
     toPreciseVM1<Points,PrecPoints>,
     toPreciseVM1<Region,PrecRegion>,
     toPreciseVM1<Line,PrecLine>,
     toPreciseVM1<datetime::DateTime,PrecTime<datetime::instanttype> >,
     toPreciseVM1<datetime::DateTime,PrecTime<datetime::durationtype> >
  };

int toPreciseSelect(ListExpr args){

  int arg = nl->First(args);
  if(PrecCoord::checkType(arg)) return 0;
  if(CcInt::checkType(arg)) return 1;
  if(CcReal::checkType(arg)) return 2;
  if(LongInt::checkType(arg)) return 3;
  if(Rational::checkType(arg)) return 4;
  if(Point::checkType(arg)) return 5;
  if(Points::checkType(arg)) return 6;
  if(Region::checkType(arg)) return 7;
  if(Line::checkType(arg)) return 8;
  if(datetime::DateTime::checkType(arg)) return 9;
  if(Duration::checkType(arg)) return 10;
  return -1;
}

/*
2.2.4 Specification

*/

OperatorSpec toPreciseSpec(
        " {numeric, point, points, line, region} -> preciseXX",
        " toPrecise(_)",
        " Converts a numeric value into a precise value",
        "query toPrecise(17.5)"
);

/*
2.2.5 Operator Instance

*/

Operator toPreciseOP(
  "toPrecise",
  toPreciseSpec.getStr(),
  11,
  toPreciseVM,
  toPreciseSelect,
  toPreciseTM
);


/*
2.3 Operator ~translate~

2.3.1 Type Mapping

*/
ListExpr translateTM(ListExpr args){
  string err = "precPoint x (numeric x numeric) expected";
  if(!nl->HasLength(args,2)){
     return listutils::typeError(err);
  }
  ListExpr arg2 = nl->Second(args);
  if(!nl->HasLength(arg2,2)){
     return listutils::typeError(err);
  }

  if(!isNumeric(nl->First(arg2)) || !isNumeric(nl->Second(arg2))){
     return listutils::typeError(err);
  }
  ListExpr arg = nl->First(args);
  if(PrecPoint::checkType(arg)){
     return listutils::basicSymbol<PrecPoint>();
  } 
  return listutils::typeError(err);
}


/*
2.3.2 Value Mapping

*/
template<class S, class N1, class N2>
int translateVM1 (Word* args, Word& result, int message, Word& local,
            Supplier s ){

    S* arg = (S*) args[0].addr;
    Supplier son = qp->GetSupplier( args[1].addr, 0 );
    Word t;
    qp->Request( son, t );
    const N1* t1 = ((N1*)t.addr);
    son = qp->GetSupplier( args[1].addr, 1 );
    qp->Request( son, t );
    const N2* t2 = ((N2*)t.addr);
    result = qp->ResultStorage(s);
    S* res = (S*) result.addr;
    if(!t1->IsDefined() || !t2->IsDefined() || !arg->IsDefined()){
       res->SetDefined(false);  
    }
    arg->compTranslate(t1->GetValue(), t2->GetValue(), *res);
    return  0;
}

/*
2.3.2 ValueMapping Array and Selection function

*/

ValueMapping translateVM[] = {
    translateVM1<PrecPoint,CcInt,CcInt>,
    translateVM1<PrecPoint,CcInt,CcReal>,
    translateVM1<PrecPoint,CcInt,LongInt>,
    translateVM1<PrecPoint,CcInt,Rational>,
    translateVM1<PrecPoint,CcInt,PrecCoord>,

    translateVM1<PrecPoint,CcReal,CcInt>,
    translateVM1<PrecPoint,CcReal,CcReal>,
    translateVM1<PrecPoint,CcReal,LongInt>,
    translateVM1<PrecPoint,CcReal,Rational>,
    translateVM1<PrecPoint,CcReal,PrecCoord>,

    translateVM1<PrecPoint,LongInt,CcInt>,
    translateVM1<PrecPoint,LongInt,CcReal>,
    translateVM1<PrecPoint,LongInt,LongInt>,
    translateVM1<PrecPoint,LongInt,Rational>,
    translateVM1<PrecPoint,LongInt,PrecCoord>,

    translateVM1<PrecPoint,Rational,CcInt>,
    translateVM1<PrecPoint,Rational,CcReal>,
    translateVM1<PrecPoint,Rational,LongInt>,
    translateVM1<PrecPoint,Rational,Rational>,
    translateVM1<PrecPoint,Rational,PrecCoord>,

    translateVM1<PrecPoint,PrecCoord,CcInt>,
    translateVM1<PrecPoint,PrecCoord,CcReal>,
    translateVM1<PrecPoint,PrecCoord,LongInt>,
    translateVM1<PrecPoint,PrecCoord,Rational>,
    translateVM1<PrecPoint,PrecCoord,PrecCoord>,
    
    translateVM1<PrecPoints,CcInt,CcInt>,
    translateVM1<PrecPoints,CcInt,CcReal>,
    translateVM1<PrecPoints,CcInt,LongInt>,
    translateVM1<PrecPoints,CcInt,Rational>,
    translateVM1<PrecPoints,CcInt,PrecCoord>,

    translateVM1<PrecPoints,CcReal,CcInt>,
    translateVM1<PrecPoints,CcReal,CcReal>,
    translateVM1<PrecPoints,CcReal,LongInt>,
    translateVM1<PrecPoints,CcReal,Rational>,
    translateVM1<PrecPoints,CcReal,PrecCoord>,

    translateVM1<PrecPoints,LongInt,CcInt>,
    translateVM1<PrecPoints,LongInt,CcReal>,
    translateVM1<PrecPoints,LongInt,LongInt>,
    translateVM1<PrecPoints,LongInt,Rational>,
    translateVM1<PrecPoints,LongInt,PrecCoord>,

    translateVM1<PrecPoints,Rational,CcInt>,
    translateVM1<PrecPoints,Rational,CcReal>,
    translateVM1<PrecPoints,Rational,LongInt>,
    translateVM1<PrecPoints,Rational,Rational>,
    translateVM1<PrecPoints,Rational,PrecCoord>,

    translateVM1<PrecPoints,PrecCoord,CcInt>,
    translateVM1<PrecPoints,PrecCoord,CcReal>,
    translateVM1<PrecPoints,PrecCoord,LongInt>,
    translateVM1<PrecPoints,PrecCoord,Rational>,
    translateVM1<PrecPoints,PrecCoord,PrecCoord>,
 };

int getNumIndex(ListExpr arg){
  if(CcInt::checkType(arg)) return 0;
  if(CcReal::checkType(arg)) return 1;
  if(LongInt::checkType(arg)) return 2;
  if(Rational::checkType(arg)) return 3;
  if(PrecCoord::checkType(arg)) return 4;
  return -1;
}

int translateSelect(ListExpr args){

  ListExpr arg1 = nl->First(args);
  ListExpr arg2 = nl->First(nl->Second(args));
  ListExpr arg3 = nl->Second(nl->Second(args));
  int o1 = 0;
  if(PrecPoint::checkType(arg1)){
     o1 = 0;
  }
  if(PrecPoints::checkType(arg1)){
     o1 = 1;
  }

  return 25*o1 + getNumIndex(arg2)*5 + getNumIndex(arg3);
}

/*
2.3.4 Specification 

*/

OperatorSpec translateSpec(
        " precPoint[s] x numeric x numeric -> precPoint",
        " _ translate [_ , _]",
        " Translates the spatial argument",
        " query makePrecPoint(16,17) translate[1, 2]"
);

/*
2.3.5 Operator instance

*/

Operator translateOP(
  "translate",
  translateSpec.getStr(),
  50,
  translateVM,
  translateSelect,
  translateTM
);


/*
2.4 Operator scale

2.4.1 Type Mapping

*/
ListExpr scaleTM(ListExpr args){
  string err = "precPoint(s) x numeric [x numeric] called";
  if(!nl->HasLength(args,2) && !nl->HasLength(args,3)){
     return listutils::typeError(err);
  }
  if(    !PrecPoint::checkType(nl->First(args)) 
      && !PrecPoints::checkType(nl->First(args))){
     return listutils::typeError(err);
  } 
  if(!isNumeric(nl->Second(args))){
     return listutils::typeError(err);
  }
  if(nl->HasLength(args,3) && !isNumeric(nl->Third(args))){
     return listutils::typeError(err);
  }
  return nl->First(args);
}


/*
2.4.2 Value Mapping

*/

template<class T, class N1, class N2>
int scaleVM1 (Word* args, Word& result, int message, Word& local,
            Supplier s ){

    result = qp->ResultStorage(s);
    T* res = (T*) result.addr;
    T* arg = (T*) args[0].addr;

    if(!arg->IsDefined()){
       res->SetDefined(false);
       return 0;
    }

    N1* s1 = (N1*) args[1].addr;
    if(!s1->IsDefined()){
       res->SetDefined(false);
       return 0;
    }
    if(qp->GetNoSons(s)==2){
       arg->compScale(s1->GetValue(), s1->GetValue(), *res);
       return 0;
    }
    N2* s2 = (N2*) args[2].addr;
    if(!s2->IsDefined()){
       res->SetDefined(0);
       return 0;
    }
    arg->compScale(s1->GetValue(), s2->GetValue(), *res);
    return 0;
}



/*
2.4.3 Value Mapping Array and Selection Function

*/
ValueMapping scaleVM[] = {
    scaleVM1<PrecPoint,CcInt,CcInt>,
    scaleVM1<PrecPoint,CcInt,CcReal>,
    scaleVM1<PrecPoint,CcInt,LongInt>,
    scaleVM1<PrecPoint,CcInt,Rational>,
    scaleVM1<PrecPoint,CcInt,PrecCoord>,

    scaleVM1<PrecPoint,CcReal,CcInt>,
    scaleVM1<PrecPoint,CcReal,CcReal>,
    scaleVM1<PrecPoint,CcReal,LongInt>,
    scaleVM1<PrecPoint,CcReal,Rational>,
    scaleVM1<PrecPoint,CcReal,PrecCoord>,

    scaleVM1<PrecPoint,LongInt,CcInt>,
    scaleVM1<PrecPoint,LongInt,CcReal>,
    scaleVM1<PrecPoint,LongInt,LongInt>,
    scaleVM1<PrecPoint,LongInt,Rational>,
    scaleVM1<PrecPoint,LongInt,PrecCoord>,

    scaleVM1<PrecPoint,Rational,CcInt>,
    scaleVM1<PrecPoint,Rational,CcReal>,
    scaleVM1<PrecPoint,Rational,LongInt>,
    scaleVM1<PrecPoint,Rational,Rational>,
    scaleVM1<PrecPoint,Rational,PrecCoord>,

    scaleVM1<PrecPoint,PrecCoord,CcInt>,
    scaleVM1<PrecPoint,PrecCoord,CcReal>,
    scaleVM1<PrecPoint,PrecCoord,LongInt>,
    scaleVM1<PrecPoint,PrecCoord,Rational>,
    scaleVM1<PrecPoint,PrecCoord,PrecCoord>,
    
    scaleVM1<PrecPoints,CcInt,CcInt>,
    scaleVM1<PrecPoints,CcInt,CcReal>,
    scaleVM1<PrecPoints,CcInt,LongInt>,
    scaleVM1<PrecPoints,CcInt,Rational>,
    scaleVM1<PrecPoints,CcInt,PrecCoord>,

    scaleVM1<PrecPoints,CcReal,CcInt>,
    scaleVM1<PrecPoints,CcReal,CcReal>,
    scaleVM1<PrecPoints,CcReal,LongInt>,
    scaleVM1<PrecPoints,CcReal,Rational>,
    scaleVM1<PrecPoints,CcReal,PrecCoord>,

    scaleVM1<PrecPoints,LongInt,CcInt>,
    scaleVM1<PrecPoints,LongInt,CcReal>,
    scaleVM1<PrecPoints,LongInt,LongInt>,
    scaleVM1<PrecPoints,LongInt,Rational>,
    scaleVM1<PrecPoints,LongInt,PrecCoord>,

    scaleVM1<PrecPoints,Rational,CcInt>,
    scaleVM1<PrecPoints,Rational,CcReal>,
    scaleVM1<PrecPoints,Rational,LongInt>,
    scaleVM1<PrecPoints,Rational,Rational>,
    scaleVM1<PrecPoints,Rational,PrecCoord>,

    scaleVM1<PrecPoints,PrecCoord,CcInt>,
    scaleVM1<PrecPoints,PrecCoord,CcReal>,
    scaleVM1<PrecPoints,PrecCoord,LongInt>,
    scaleVM1<PrecPoints,PrecCoord,Rational>,
    scaleVM1<PrecPoints,PrecCoord,PrecCoord>,
 };

int scaleSelect(ListExpr args){
  int o1 = 0; // offset for value to scale
  if(PrecPoint::checkType(nl->First(args))){
      o1 = 0;
  }
  if(PrecPoints::checkType(nl->First(args))){
      o1 = 1;
  }

  int o2 = getNumIndex(nl->Second(args));
  int o3 = nl->HasLength(args,2)?0:getNumIndex(nl->Third(args));
  return 25*o1 + 5*o2 + o3;

}


/*
2.3.4 Specification 

*/

OperatorSpec scaleSpec(
        " precPoint[s] x numeric x numeric -> precPoint",
        " _ scale [_ , _]",
        " Translates the spatial argument",
        " query makePrecPoint(16,17) scale[1, 2]"
);

/*
2.3.5 Operator instance

*/

Operator scaleOP(
  "scale",
  scaleSpec.getStr(),
  50,
  scaleVM,
  scaleSelect,
  scaleTM
);

/*
2.5 Operator makePrecPoint

2.5.1 Type Mapping

*/
ListExpr makePrecPointTM(ListExpr args){

  string err = "numeric x numeric expected";
  if(!nl->HasLength(args,2)){
      return listutils::typeError(err);
  }
  if(!isNumeric(nl->First(args)) || !isNumeric(nl->Second(args))){
     return listutils::typeError(err);
  }
  return listutils::basicSymbol<PrecPoint>();
} 

/*
2.5.2 Value Mapping Template

*/
template<class N1, class N2>
int makePrecPointVM1 (Word* args, Word& result, int message, Word& local,
            Supplier s ){
   result = qp->ResultStorage(s);
   PrecPoint* res = (PrecPoint*) result.addr;
   N1* x = (N1*) args[0].addr;
   N2* y = (N2*) args[1].addr;
   if(!x->IsDefined() || !y->IsDefined()){
      res->SetDefined(false);
      return 0;
   }
   res->set(x->GetValue(), y->GetValue(),1);
   return 0;
}

/*
2.5.3 Value Mapping Array and Selection Function

*/

ValueMapping makePrecPointVM[] = {
    makePrecPointVM1<CcInt,CcInt>,
    makePrecPointVM1<CcInt,CcReal>,
    makePrecPointVM1<CcInt,LongInt>,
    makePrecPointVM1<CcInt,Rational>,
    makePrecPointVM1<CcInt,PrecCoord>,

    makePrecPointVM1<CcReal,CcInt>,
    makePrecPointVM1<CcReal,CcReal>,
    makePrecPointVM1<CcReal,LongInt>,
    makePrecPointVM1<CcReal,Rational>,
    makePrecPointVM1<CcReal,PrecCoord>,

    makePrecPointVM1<LongInt,CcInt>,
    makePrecPointVM1<LongInt,CcReal>,
    makePrecPointVM1<LongInt,LongInt>,
    makePrecPointVM1<LongInt,Rational>,
    makePrecPointVM1<LongInt,PrecCoord>,

    makePrecPointVM1<Rational,CcInt>,
    makePrecPointVM1<Rational,CcReal>,
    makePrecPointVM1<Rational,LongInt>,
    makePrecPointVM1<Rational,Rational>,
    makePrecPointVM1<Rational,PrecCoord>,

    makePrecPointVM1<PrecCoord,CcInt>,
    makePrecPointVM1<PrecCoord,CcReal>,
    makePrecPointVM1<PrecCoord,LongInt>,
    makePrecPointVM1<PrecCoord,Rational>,
    makePrecPointVM1<PrecCoord,PrecCoord>,
 };

int makePrecPointSelect(ListExpr args){
  int o1 = getNumIndex(nl->First(args));
  int o2 = getNumIndex(nl->Second(args));
  return  5*o1 + o2;
}


/*
2.5.4 Specification 

*/

OperatorSpec makePrecPointSpec(
        "  numeric x numeric -> precPoint",
        " makePrecPoint(_,_)",
        " create a new precise point value",
        " query makePrecPoint(16,[ const precise value (1 '1/3')) "
);

/*
2.3.5 Operator instance

*/

Operator makePrecPointOP(
  "makePrecPoint",
  makePrecPointSpec.getStr(),
  25,
  makePrecPointVM,
  makePrecPointSelect,
  makePrecPointTM
);

/*
2.6 Operator contains

2.6.1 Type Mapping

*/
ListExpr containsTM(ListExpr args){
  string err = "precPoints x {precPoint, precPoints} expected";
  if(!nl->HasLength(args,2)){
      return listutils::typeError(err);
  }
  if(!PrecPoints::checkType(nl->First(args))){
      return listutils::typeError(err);
  }
  ListExpr arg2 = nl->Second(args);
  if(!PrecPoints::checkType(arg2) && !PrecPoint::checkType(arg2)){
      return listutils::typeError(err);
  }
  return listutils::basicSymbol<CcBool>();
}


/*
2.6.2 Value Mapping template

*/

template<class T1, class T2>
int containsVM1 (Word* args, Word& result, int message, Word& local,
            Supplier s ){

  T1* arg1 = (T1*) args[0].addr;
  T2* arg2 = (T2*) args[1].addr;
  result = qp->ResultStorage(s);
  CcBool* res = (CcBool*) result.addr;
  arg1->contains(*arg2,*res);
  return 0;
}

/*
2.6.3 Value Mapping array and selection function

*/
ValueMapping containsVM[] = {
    containsVM1<PrecPoints, PrecPoint>,
    containsVM1<PrecPoints, PrecPoints>
};

int containsSelect(ListExpr args){

  ListExpr arg2 = nl->Second(args);
  return PrecPoint::checkType(arg2)?0:1;
}

/*
2.6.4 Specification

*/

OperatorSpec containsSpec(
        "  precPoints x {precPoints, precPoint} -> bool",
        " _ contains _",
        " Checks whether the second argument is part of the first one",
        " query ps1 contains ps2"
);

/*
2.6.5 Operator instance

*/

Operator containsOP(
  "contains",
  containsSpec.getStr(),
  2,
  containsVM,
  containsSelect,
  containsTM
);


/*
2.7 Operator intersects

2.7.1 Type Mapping

*/
ListExpr intersectsTM(ListExpr args){
  string err = "precPoints x precPoints expected";
  if(!nl->HasLength(args,2)){
     return listutils::typeError(err);
  }
  if(PrecPoints::checkType(nl->First(args)) &&
     PrecPoints::checkType(nl->Second(args))){
     return listutils::basicSymbol<CcBool>();
  }
  if(PrecLine::checkType(nl->First(args)) &&
     PrecLine::checkType(nl->Second(args))){
     return listutils::basicSymbol<CcBool>();
  }
  return listutils::typeError(err);
}

/*
2.7.2 Value Mapping template

*/

template<class T1, class T2>
int intersectsVM1 (Word* args, Word& result, int message, Word& local,
            Supplier s ){
   T1* arg1 = (T1*) args[0].addr;
   T2* arg2 = (T2*) args[1].addr;
   result = qp->ResultStorage(s);
   CcBool* res = (CcBool*) result.addr;
   arg1->intersects(*arg2,*res);
   return 0;
}

/*
2.7.3 Value Mapping array and selection function

*/
ValueMapping intersectsVM[] = {
    intersectsVM1<PrecPoints, PrecPoints>, // to be continued
    intersectsVM1<PrecLine, PrecLine> // to be continued
};

int intersectsSelect(ListExpr args){
  if(PrecPoints::checkType(nl->First(args)) &&
     PrecPoints::checkType(nl->Second(args))){
     return 0;
  }
  if(PrecLine::checkType(nl->First(args)) &&
     PrecLine::checkType(nl->Second(args))){
     return 1;
  }
  return -1;
}

/*
2.7.4 Specification

*/

OperatorSpec intersectsSpec(
        "  precPoints x precPoints -> bool",
        " _ intersects _",
        " Checks whether the arguments have common points",
        " query ps1 intersects ps2"
);

/*
2.7.5 Operator instance

*/

Operator intersectsOP(
  "intersects",
  intersectsSpec.getStr(),
  2,
  intersectsVM,
  intersectsSelect,
  intersectsTM
);

/*

2.8 Operator union

2.8.1 Type Mapping

*/
ListExpr unionTM(ListExpr args){

  string err = "precPoints x precPoints expected";
  if(!nl->HasLength(args,2)){
     return listutils::typeError(err);
  }
  if(   PrecPoints::checkType(nl->First(args))
     && PrecPoints::checkType(nl->Second(args))){
     return listutils::basicSymbol<PrecPoints>();
  }
  if(   PrecLine::checkType(nl->First(args))
     && PrecLine::checkType(nl->Second(args))){
     return listutils::basicSymbol<PrecLine>();
  }
  if( PrecRegion::checkType(nl->First(args))
     && PrecRegion::checkType(nl->Second(args))){
     return listutils::basicSymbol<PrecRegion>();
  }
  return listutils::typeError(err);

}

/*
2.8.2 Value Mapping template

*/

template<class T1, class T2, class R>
int unionVM1 (Word* args, Word& result, int message, Word& local,
            Supplier s ){
  T1* arg1 = (T1*) args[0].addr;
  T2* arg2 = (T1*) args[1].addr;
  result = qp->ResultStorage(s);
  R* res = (R*) result.addr;
  arg1->compUnion(*arg2,*res);
  return 0;
}

/*
2.8.3 Value Mapping array and Selection function

*/

ValueMapping unionVM[] = {
   unionVM1<PrecPoints,PrecPoints, PrecPoints>,
   unionVM1<PrecLine,PrecLine, PrecLine>,
   unionVM1<PrecRegion,PrecRegion, PrecRegion> // to be continued
};

int unionSelect(ListExpr args){
  if(   PrecPoints::checkType(nl->First(args))
     && PrecPoints::checkType(nl->Second(args))){
     return 0;
  }
  if(   PrecLine::checkType(nl->First(args))
     && PrecLine::checkType(nl->Second(args))){
     return 1;
  }
  if( PrecRegion::checkType(nl->First(args))
     && PrecRegion::checkType(nl->Second(args))){
     return 2;
  }
  return -1;
}

/*
2.8.4 Specification

*/

OperatorSpec unionSpec(
        "  precX x precX -> precX",
        " _ union _",
        " computes the union of the arguments",
        " query ps1 union ps2"
);


/*
2.8.5 Operator instance

*/
Operator unionOP(
  "union",
  unionSpec.getStr(),
  3,
  unionVM,
  unionSelect,
  unionTM
);


/*

2.9 Operator intersection

2.9.1 Type Mapping

*/
ListExpr intersectionTM(ListExpr args){

  string err = "precPoints x precPoints, or precLine x precLine expected";
  if(!nl->HasLength(args,2)){
     return listutils::typeError(err);
  }
  if(   PrecPoints::checkType(nl->First(args))
     || PrecPoints::checkType(nl->Second(args))){
        return listutils::basicSymbol<PrecPoints>();
  }
  if(   PrecLine::checkType(nl->First(args))
     || PrecLine::checkType(nl->Second(args))){
        return listutils::basicSymbol<PrecLine>();
  }
  return listutils::typeError(err);
}

/*
2.9.2 Value Mapping template

*/

template<class T1, class T2, class R>
int intersectionVM1 (Word* args, Word& result, int message, Word& local,
            Supplier s ){

  T1* arg1 = (T1*) args[0].addr;
  T2* arg2 = (T1*) args[1].addr;
  result = qp->ResultStorage(s);
  R* res = (R*) result.addr;
  arg1->intersection(*arg2,*res);
  return 0;
}

/*
2.9.3 Value Mapping array and Selection function

*/

ValueMapping intersectionVM[] = {
   intersectionVM1<PrecPoints,PrecPoints, PrecPoints>,
   intersectionVM1<PrecLine,PrecLine, PrecLine> // to be continued
};

int intersectionSelect(ListExpr args){
  ListExpr a1 = nl->First(args);
  if(PrecPoints::checkType(a1)) return 0;
  if(PrecLine::checkType(a1)) return 1;
  return -1;
}

/*
2.9.4 Specification

*/

OperatorSpec intersectionSpec(
        "  precT x precT -> precT, T in {Points, Line}",
        "  intersection(_, _)",
        " computes the intersection of the arguments",
        " query intersection(ps1,ps2)"
);


/*
2.9.5 Operator instance

*/
Operator intersectionOP(
  "intersection",
  intersectionSpec.getStr(),
  2,
  intersectionVM,
  intersectionSelect,
  intersectionTM
);


/*

2.9 Operator difference

2.9.1 Type Mapping

*/
ListExpr differenceTM(ListExpr args){

  string err = "precT  x precT expected, T in {points, line}";
  if(!nl->HasLength(args,2)){
     return listutils::typeError(err);
  }
  if(   PrecPoints::checkType(nl->First(args))
     && PrecPoints::checkType(nl->Second(args))){
     return listutils::basicSymbol<PrecPoints>();
  }
  if(   PrecLine::checkType(nl->First(args))
     && PrecLine::checkType(nl->Second(args))){
     return listutils::basicSymbol<PrecLine>();
  }
  return listutils::typeError(err);

}

/*
2.10.2 Value Mapping template

*/

template<class T1, class T2, class R>
int differenceVM1 (Word* args, Word& result, int message, Word& local,
            Supplier s ){

  T1* arg1 = (T1*) args[0].addr;
  T2* arg2 = (T1*) args[1].addr;
  result = qp->ResultStorage(s);
  R* res = (R*) result.addr;
  arg1->difference(*arg2,*res);
  return 0;
}

/*
2.10.3 Value Mapping array and Selection function

*/

ValueMapping differenceVM[] = {
   differenceVM1<PrecPoints,PrecPoints, PrecPoints>,
   differenceVM1<PrecLine,PrecLine, PrecLine>
};

int differenceSelect(ListExpr args){
  ListExpr a1 = nl->First(args);
  if(PrecPoints::checkType(a1)) return 0;
  if(PrecLine::checkType(a1)) return 1;
  return -1;
}

/*
2.10.4 Specification

*/

OperatorSpec differenceSpec(
        "  precT x precT -> precT , T in {points, line}",
        " difference(_, _)",
        " computes the difference of the arguments",
        " query difference(ps1, ps2)"
);


/*
2.10.5 Operator instance

*/
Operator differenceOP(
  "difference",
  differenceSpec.getStr(),
  2,
  differenceVM,
  differenceSelect,
  differenceTM
);


/*
2.11 Operator str2precise

2.11.1 Type Mapping

*/
ListExpr str2preciseTM(ListExpr args){
  string err = " {string ,text} [ x int]  expected";
  if(!nl->HasLength(args,1) && !nl->HasLength(args,2)){
     return listutils::typeError(err);
  }
  ListExpr arg = nl->First(args);
  if(!CcString::checkType(arg) && !FText::checkType(arg)){
     return listutils::typeError(err);
  }
  if(nl->HasLength(args,2)){
     if(!CcInt::checkType(nl->Second(args))){
        return listutils::typeError(err);
     }
     return listutils::basicSymbol<PrecCoord>();
  }
  return nl->ThreeElemList(
           nl->SymbolAtom(Symbols::APPEND()),
           nl->OneElemList(nl->IntAtom(1)),
           listutils::basicSymbol<PrecCoord>()
         ); 
}

/*
2.11.2 ValueMapping

*/

template<class T>
int str2preciseVM1 (Word* args, Word& result, int message, Word& local,
            Supplier s ){

  T* arg1 = (T*) args[0].addr;
  CcInt* arg2 = (CcInt*)  args[1].addr;
  result = qp->ResultStorage(s);
  PrecCoord* res = (PrecCoord*) result.addr;
  if(!arg1->IsDefined() || !arg2->IsDefined()){
     res->SetDefined(false);
     return 0;
  }

  MPrecCoordinate c(0);
  if(!c.readFromString(arg1->GetValue(), arg2->GetValue())){
    res->SetDefined(false);
  } else {
    *res = c;
  }
  return 0;
}

/*
2.11.3 ValueMapping array and Selection function

*/
ValueMapping str2preciseVM[] = {
    str2preciseVM1<CcString>,
    str2preciseVM1<FText>
};

int str2preciseSelect(ListExpr args){
  return CcString::checkType(nl->First(args))?0:1;
}

/*
2.11.4 Specification

*/

OperatorSpec str2preciseSpec(
        "  {string, text} -> precise",
        " str2precise(_)",
        " converts a string into a precise value",
        " query str2precise('8.9876')"
);


/*
2.11.5 Operator instance

*/
Operator str2preciseOP(
  "str2precise",
  str2preciseSpec.getStr(),
  2,
  str2preciseVM,
  str2preciseSelect,
  str2preciseTM
);


/*
2.12 Operator noElements

This operator returns the number of contained elements, e.g.
the number of points within a point value and the number
of segments within a line or region.

*/
ListExpr noElementsTM(ListExpr args){
   string err = "precPoints, precLine, or precRegion expected";
   if(!nl->HasLength(args,1)){
     return listutils::typeError(err);
   }
   ListExpr arg = nl->First(args);
   if(   PrecPoints::checkType(arg) 
      || PrecLine::checkType(arg) 
      || PrecRegion::checkType(arg)){
     return listutils::basicSymbol<CcInt>();
  }
  return listutils::typeError(err);
}

template<class T>
int noElementsVM1 (Word* args, Word& result, int message, Word& local,
                  Supplier s ){

    T* arg = (T*) args[0].addr;
    result = qp->ResultStorage(s);
    CcInt* res = (CcInt*) result.addr;
    if(!arg->IsDefined()){
        res->SetDefined(false);
    } else {
      res->Set(true,arg->getNoElements());
    }
    return 0;
}

ValueMapping noElementsVM[] = {
    noElementsVM1<PrecPoints>,
    noElementsVM1<PrecLine>,
    noElementsVM1<PrecRegion>
};

int noElementsSelect(ListExpr args){
    ListExpr a = nl->First(args);
    if(PrecPoints::checkType(a)) return 0;
    if(PrecLine::checkType(a)) return 1;
    if(PrecRegion::checkType(a)) return 2;
    return -1;
}

OperatorSpec noElementsSpec(
        "  {precPoints, precLine, precRegion} -> precise",
        " noElements(_)",
        " returns the number of elements (points, segments)",
        " query noElements(toPrecise(BGrenzenLine))"
);

Operator noElementsOP(
  "noElements",
  noElementsSpec.getStr(),
  3,
  noElementsVM,
  noElementsSelect,
  noElementsTM
);

/*
2.12 operator halfSegments

This operator returns the halfsegments of a region or line 
as a stream of tuples.

*/

template<class T>
ListExpr getAttr(const string& name){
    return nl->TwoElemList(
            nl->SymbolAtom(name),
            listutils::basicSymbol<T>());
}

ListExpr halfSegmentsTM(ListExpr args){
   string err = "precLine or precRegion expected";
   if(!nl->HasLength(args,1)){
      return listutils::typeError(err);
   }
   ListExpr a = nl->First(args);
   if(PrecLine::checkType(a) ||
      PrecRegion::checkType(a)){

     ListExpr attrList = nl->OneElemList(getAttr<CcInt>("FaceNo"));
                  
     ListExpr last = attrList;
     last = nl->Append(last, getAttr<CcInt>("CycleNo"));
     last = nl->Append(last, getAttr<CcInt>("EdgeNo"));
     last = nl->Append(last, getAttr<CcInt>("CoverageNo"));
     last = nl->Append(last, getAttr<CcBool>("InsideAbove"));
     last = nl->Append(last, getAttr<CcInt>("PartnerNo"));
     last = nl->Append(last, getAttr<CcBool>("Ldp"));
     last = nl->Append(last, getAttr<PrecLine>("Segment"));

     return nl->TwoElemList(
           listutils::basicSymbol<Stream<Tuple> >(),
           nl->TwoElemList(
               listutils::basicSymbol<Tuple>(),
               attrList)) ;
   }
   return listutils::typeError(err);

}

template<class T>
class halfSegmentsInfo{

  public:
    halfSegmentsInfo(const T* v, ListExpr ttl): 
      pos(0), size(v->IsDefined()?v->Size():0), value(v){
      tt = new TupleType(ttl); 
   }

   ~halfSegmentsInfo(){
       tt->DeleteIfAllowed();
    }

   Tuple* getNext(){
      if(pos>=size){
         return 0;
      }
      Tuple* res = getTuple(value->getHalfSegment(pos));
      pos++;
      return res;
   }

  private:
     size_t pos;
     size_t size;
     const T* value;
     TupleType* tt;

   Tuple* getTuple(MPrecHalfSegment hs){
      Tuple* res = new Tuple(tt);
      res->PutAttribute(0, new CcInt(true,hs.attributes.faceno));
      res->PutAttribute(1, new CcInt(true,hs.attributes.cycleno));
      res->PutAttribute(2, new CcInt(true,hs.attributes.edgeno));
      res->PutAttribute(3, new CcInt(true,hs.attributes.coverageno));
      res->PutAttribute(4, new CcBool(true,hs.attributes.insideAbove));
      res->PutAttribute(5, new CcInt(true,hs.attributes.partnerno));
      res->PutAttribute(6, new CcBool(true,hs.isLeftDomPoint()));
      PrecLine* l = new PrecLine(true);
      l->startBulkLoad();
      l->append(hs);
      l->endBulkLoad();
      res->PutAttribute(7, l);
      return res;
   }
};


template<class T>
int halfSegmentsVM1 (Word* args, Word& result, int message, Word& local,
                  Supplier s ){

  halfSegmentsInfo<T>* li = (halfSegmentsInfo<T>*) local.addr;
  switch(message){
     case OPEN: {
          if(li){ delete li; }
          local.addr = new halfSegmentsInfo<T>((T*)args[0].addr,
                                       nl->Second(GetTupleResultType(s)));
          return 0;
     }
     case REQUEST: {
         result.addr = li?li->getNext():0;
         return result.addr?YIELD:CANCEL;
     }
     case CLOSE: {
         if(li){
            delete li;
            local.addr = 0;
         }
         return 0;
     }
  }
  return -1;
}

ValueMapping halfSegmentsVM[] = {
    halfSegmentsVM1<PrecLine>,
    halfSegmentsVM1<PrecRegion>
};

int halfSegmentsSelect(ListExpr args){
    ListExpr a = nl->First(args);
    if(PrecLine::checkType(a)) return 0;
    if(PrecRegion::checkType(a)) return 1;
    return -1;
}

OperatorSpec halfSegmentsSpec(
        "  { precLine, precRegion} -> stream(tuple([FaceNo : int, ...]))",
        " halfSegments(_)",
        " returns the halfsegemnts building the argument",
        " query halfSegments(toPrecise(BGrenzenLine)) count"
);

Operator halfSegmentsOP(
  "halfSegments",
  halfSegmentsSpec.getStr(),
  2,
  halfSegmentsVM,
  halfSegmentsSelect,
  halfSegmentsTM
);

/*
2.13 Operator vertices

*/

ListExpr verticesTM(ListExpr args){
   string err="precLine or precRegion expected";
   if(!nl->HasLength(args,1)){
     return listutils::typeError(err);
   }
   ListExpr a = nl->First(args);
   if(PrecLine::checkType(a) ||
      PrecRegion::checkType(a)){
       return listutils::basicSymbol<PrecPoints>();
   }
   return listutils::typeError(err);
}

template<class T>
int verticesVM1 (Word* args, Word& result, int message, Word& local,
                 Supplier s ){

  result = qp->ResultStorage(s);
  PrecPoints* res = (PrecPoints*) result.addr;
  T* arg = (T*) args[0].addr;
  if(!arg->IsDefined()){
     res->SetDefined(false);
  } else {
     res->startBulkLoad( arg->getScale());
     for(size_t i=0;i<arg->Size();i++){
         MPrecHalfSegment hs = arg->getHalfSegment(i);
         res->append(hs.getLeftPoint());
         res->append(hs.getRightPoint());
     }
     res->endBulkLoad();
  }
  return 0;
}


ValueMapping verticesVM[] = {
    verticesVM1<PrecLine>,
    verticesVM1<PrecRegion>
};

int verticesSelect(ListExpr args){
    ListExpr a = nl->First(args);
    if(PrecLine::checkType(a)) return 0;
    if(PrecRegion::checkType(a)) return 1;
    return -1;
}

OperatorSpec verticesSpec(
        "  { precLine, precRegion} -> precPoints",
        " vertices(_)",
        " returns the vertices of the argument",
        " query vertices(toPrecise(BGrenzenLine)) "
);

Operator verticesOP(
  "vertices",
  verticesSpec.getStr(),
  2,
  verticesVM,
  verticesSelect,
  verticesTM
);

/*
2.13 operator boundary

2.13.1 Type Mapping

*/
ListExpr boundaryTM(ListExpr args){
   string err = "precRegion expected";
   if(!nl->HasLength(args,1)){
      return listutils::typeError(err);
   }
   ListExpr a = nl->First(args);
   if(PrecRegion::checkType(a)){
      return listutils::basicSymbol<PrecLine>();
   }
   return listutils::typeError(err);
}

/*
2.13.2 Value Mapping 

*/
template<class AT, class RT>
int boundaryVM1 (Word* args, Word& result, int message, Word& local,
                 Supplier s ){
    AT* arg = (AT*) args[0].addr;
    result = qp->ResultStorage(s);
    RT* res = (RT*) result.addr;
    arg->boundary(*res);
    return 0; 
}

/*
2.13.3 ValueMapping Array and Selection function

*/
ValueMapping boundaryVM[] = {
    boundaryVM1<PrecRegion, PrecLine>
 };

int boundarySelect(ListExpr args){
   ListExpr arg = nl->First(args);
   if(PrecRegion::checkType(arg)){
      return 0;
   }
   return -1;
}

/*
2.13.4 Specification

*/
OperatorSpec boundarySpec(
        "  precRegion -> precLine, precLine -> precPoint",
        " boundary(_)",
        " returns the boundary of the argument",
        " query boundary(toPrecise(BGrenzenLine)) "
);

/*
2.13.5 Operator instance

*/
Operator boundaryOP(
  "boundary",
  boundarySpec.getStr(),
  1,
  boundaryVM,
  boundarySelect,
  boundaryTM
);

/*
2.14 Operator bbox

2.14.1 bboxTM

*/
ListExpr bboxTM (ListExpr args){
  string err = "precPoint, precPoints, precLine, or precRegion expected";
  if(!nl->HasLength(args,1)){
    return listutils::typeError(err);
  }
  ListExpr arg = nl->First(args);
  if(   PrecPoint::checkType(arg)
     || PrecPoints::checkType(arg)
     || PrecLine::checkType(arg)
     || PrecRegion::checkType(arg)){
    return listutils::basicSymbol<Rectangle<2> >();
  }
  return listutils::typeError(err);
}


template<class T>
int bboxVM1 (Word* args, Word& result, int message, Word& local,
                 Supplier s ){

   T* arg = (T*) args[0].addr;
   result = qp->ResultStorage(s);
   Rectangle<2>* res = (Rectangle<2>*) result.addr;
   *res = arg->BoundingBox();
   return 0;
}


ValueMapping bboxVM[] = {
    bboxVM1<PrecPoint>,
    bboxVM1<PrecPoints>,
    bboxVM1<PrecLine>,
    bboxVM1<PrecRegion>
 };

int bboxSelect(ListExpr args){
   ListExpr arg = nl->First(args);
   if(PrecPoint::checkType(arg)){
      return 0;
   }
   if(PrecPoints::checkType(arg)){
      return 1;
   }
   if(PrecLine::checkType(arg)){
      return 2;
   }
   if(PrecRegion::checkType(arg)){
      return 3;
   }
   return -1;
}

OperatorSpec bboxSpec(
        " precPoint | precPoints | precLine | precRegion -> rect",
        " bbox(_)",
        " returns the bounding box of the argument",
        " query bbox(toPrecise(BGrenzenLine)) "
);

/*
2.13.5 Operator instance

*/
Operator bboxOP(
  "bbox",
  bboxSpec.getStr(),
  4,
  bboxVM,
  bboxSelect,
  bboxTM
);





/*
2.14 collect

*/
ListExpr collectTM(ListExpr args){

 string err = " {stream(precLine), stream(precPoint), "
              "stream(precPoints)} x boolean expected";

 if(!nl->HasLength(args,2)){
    return listutils::typeError(err);
 }
 if(!CcBool::checkType(nl->Second(args))){
    return listutils::typeError(err);
 }
 ListExpr s = nl->First(args);
 if( Stream<PrecPoint>::checkType(s)){
   return nl->Second(s);
 }
 if( Stream<PrecLine>::checkType(s)){
   return nl->Second(s);
 }
 if( Stream<PrecLine>::checkType(s)){
   return nl->Second(s);
 }
 return listutils::typeError(err);
}


/*
Value Mapping

*/

template<class Arg, class Res>
int collectVM1 (Word* args, Word& result, int message, Word& local,
                 Supplier s ){


  Stream<Arg> stream(args[0]);
  CcBool* iu = (CcBool*) args[1].addr;
  bool ignoreUD = iu->IsDefined() && iu->GetValue();
  result = qp->ResultStorage(s);
  Res* res = (Res*)  result.addr;
  res->clear();
  res->SetDefined(true);
  res->startBulkLoad();
  stream.open();
  Arg* elem = stream.request();
  while(elem){
    if(!elem->IsDefined()){
      if(!ignoreUD){
         res->cancelBulkLoad(); 
         res->SetDefined(false);
         stream.close();
         return 0;
      }
    } else {
        Arg* e1 = new Arg(*elem);
        res->append(*e1);
        delete e1;
    }
    elem->DeleteIfAllowed();
    elem = stream.request();
  }
  stream.close();
  res->endBulkLoad();
  return 0;
}

/*
Value Mapping array and Selection function

*/

ValueMapping collectVM[] = {
   collectVM1<PrecPoint, PrecPoints>,
   collectVM1<PrecPoints, PrecPoints>,
   collectVM1<PrecLine, PrecLine>,
};


/*
Selection Function

*/
int collectSelect(ListExpr args){
   ListExpr a = nl->Second(nl->First(args));
   if(PrecPoint::checkType(a)) return 0;
   if(PrecPoints::checkType(a)) return 1;
   if(PrecLine::checkType(a)) return 2;
   return -1;
}

OperatorSpec collectSpec(
    " stream(X) x bool -> X , with X in {precPoint, precPoints, precLine}",
    " _ collect [_]",
    " Builds the union of all elements in the stream. If the boolean"
    " parameter is TRUE, undefined value in the stream are ignored. Otherwise,"
    " an undefined element in the stream leads to an undefined result.",
    " query strassen feed replaceAttr[GeoData : toPrecise(.GeoData)] "
                     "projecttransformstream[GeoData] collect[TRUE]"
);


/*
Operator instance

*/
Operator collectOP(
  "collectprecise",
  collectSpec.getStr(),
  3,
  collectVM,
  collectSelect,
  collectTM
);






} // end of namespace precise

class PreciseAlgebra : public Algebra
{
 public:
  PreciseAlgebra() : Algebra()
  {
    AddTypeConstructor( &precise::precise );
    precise::precise.AssociateKind("DATA");
    AddTypeConstructor( &precise::precisePoint );
    precise::precisePoint.AssociateKind("DATA");
    AddTypeConstructor( &precise::precisePoints );
    precise::precisePoints.AssociateKind("DATA");
    AddTypeConstructor( &precise::preciseRegion );
    precise::preciseRegion.AssociateKind("DATA");
    AddTypeConstructor( &precise::preciseLine );
    precise::preciseLine.AssociateKind("DATA");

    AddTypeConstructor( &precise::preciseInstant );
    precise::preciseInstant.AssociateKind("DATA");
    AddTypeConstructor( &precise::preciseDuration );
    precise::preciseDuration.AssociateKind("DATA");


    AddOperator(&precise::plusOP);
    AddOperator(&precise::minusOP);
    AddOperator(&precise::mulOP);
    AddOperator(&precise::divideOP);
    AddOperator(&precise::timePlusOP);
    AddOperator(&precise::timeMinusOP);
    AddOperator(&precise::timeMulOP);
    AddOperator(&precise::timeDivOP);
    AddOperator(&precise::toPreciseOP);
    AddOperator(&precise::translateOP);
    AddOperator(&precise::scaleOP);
    AddOperator(&precise::makePrecPointOP);
    AddOperator(&precise::containsOP);
    AddOperator(&precise::intersectsOP);
    AddOperator(&precise::unionOP);
    AddOperator(&precise::intersectionOP);
    AddOperator(&precise::differenceOP);
    AddOperator(&precise::str2preciseOP);
    AddOperator(&precise::noElementsOP);
    AddOperator(&precise::halfSegmentsOP);
    AddOperator(&precise::verticesOP);
    AddOperator(&precise::boundaryOP);
    AddOperator(&precise::bboxOP);
    AddOperator(&precise::collectOP);
  }
};




extern "C"
Algebra*
InitializePreciseAlgebra( NestedList* nlRef, QueryProcessor* qpRef )
{
  nl = nlRef;
  qp = qpRef;
  return (new PreciseAlgebra());
}

