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

#include "RationalAttr.h"
#include "LongInt.h"
#include "Point.h"
#include "SpatialAlgebra.h"
#include "FTextAlgebra.h"


extern NestedList* nl;
extern QueryProcessor *qp; 
extern AlgebraManager *am;


namespace precise{

/*
1 Type constructors

*/
GenTC<PrecCoord> precise;
GenTC<PrecPoint> precisePoint;
GenTC<precPoints> precisePoints;
GenTC<PrecRegion> preciseRegion;


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
2.2 Operator toPrecise

This operator converts a numeric value into a precise value.

2.2.1 Type Mapping

*/

ListExpr toPreciseTM(ListExpr args){
  string err = "{int,real,longint, rational, precise, point, points} "
               " [x int [x bool]]} expected";
  int len = nl->ListLength(args);
  if(len<1 || len > 3){
     return listutils::typeError(err);
  }
  
  if(len>=2){
      ListExpr arg2 = nl->Second(args);
      if(!CcInt::checkType(arg2) ){
          return listutils::typeError(err);
      }
  }
  if(len==3){
     ListExpr arg3 = nl->Third(args);
     if(!CcBool::checkType(arg3)){
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
     resType = listutils::basicSymbol<precPoints>();
  } else if(Region::checkType(arg1)){
     resType = listutils::basicSymbol<PrecRegion>();
  } else {
    return listutils::typeError(err);
  }
  switch(len){
     case 1: return nl->ThreeElemList( nl->SymbolAtom(Symbols::APPEND()),
                                       nl->TwoElemList(nl->IntAtom(1), 
                                                       nl->BoolAtom(false)),
                                       resType);
     case 2 : return nl->ThreeElemList( nl->SymbolAtom(Symbols::APPEND()),
                                        nl->OneElemList(nl->BoolAtom(false)),
                                        resType);
     case 3 : return resType;
     default:  return listutils::typeError(err);
  }
}

/*
2.2.2 Value Mapping

*/


template<class T>
int toPreciseVM1 (Word* args, Word& result, int message, Word& local,
            Supplier s ){

  T* arg = (T*) args[0].addr;
  result = qp->ResultStorage(s);
  PrecCoord* res = (PrecCoord*) result.addr;
  uint32_t scale = 1;
  if(qp->GetNoSons(s)==2){
     CcInt* arg2 = (CcInt*) args[1].addr;
     if(!arg2->IsDefined()){
          res->SetDefined(false);
          return 0;
     } 
     CcInt::inttype v = arg2->GetValue();
     if(v<=0){
        res->SetDefined(false);
        return 0;
     }
     scale = v;
  }
 
  if(!arg->IsDefined()){
     res->SetDefined(false);
     return 0;
  }
  MPrecCoordinate m (arg->GetValue(), scale);
  if(scale!=1){
       m *= scale;
  }
  *res = m;
  return 0;
}


int toPreciseVM2 (Word* args, Word& result, int message, Word& local,
            Supplier s ){
  Point* arg = (Point*) args[0].addr;
  result = qp->ResultStorage(s);
  PrecPoint* res = (PrecPoint*) result.addr;
  uint32_t scale = 1;
  if(qp->GetNoSons(s)==2){
     CcInt* arg2 = (CcInt*) args[1].addr;
     if(!arg2->IsDefined()){
          res->SetDefined(false);
          return 0;
     } 
     CcInt::inttype v = arg2->GetValue();
     if(v<=0){
        res->SetDefined(false);
        return 0;
     }
     scale = v;
  }
  res->set(arg->GetX(), arg->GetY(), scale);
  if(scale!=1){
     res->compScale(scale, scale,*res);
  }
  return 0;
}

int toPreciseVM3 (Word* args, Word& result, int message, Word& local,
            Supplier s ){

  Points* arg = (Points*) args[0].addr;
  result = qp->ResultStorage(s);
  precPoints* res = (precPoints*) result.addr;
  res->clear();
  if(!arg->IsDefined()){
     res->SetDefined(false);
     return 0;   
  }
  uint32_t scale = 1;
  if(qp->GetNoSons(s)==2){
     CcInt* arg2 = (CcInt*) args[1].addr;
     if(!arg2->IsDefined()){
          res->SetDefined(false);
          return 0;
     } 
     CcInt::inttype v = arg2->GetValue();
     if(v<=0){
        res->SetDefined(false);
        return 0;
     }
     scale = v;
  }

  Point p;
  MPrecPoint pp(0,0);
  res->StartBulkLoad(scale);
  for(int i=0;i<arg->Size(); i++){
      arg->Get(i,p);
      pp.set(p.GetX(), p.GetY(), scale);
      if(scale!=1){
         pp.compScale(scale);
      }
      res->append(pp);
  }
  res->EndBulkLoad();
  return 0;
}


template<class S, class T>
int toPreciseVM4 (Word* args, Word& result, int message, Word& local,
            Supplier s ){

  result = qp->ResultStorage(s);
  T* res = (T*) result.addr;
  res->clear();
  S* arg = (S*) args[0].addr;
  CcInt* scale = (CcInt*) args[1].addr;
  CcBool* useStr = (CcBool*) args[2].addr;
  if(!arg->IsDefined() || !scale->IsDefined() ||!useStr->IsDefined()){
     res->SetDefined(false);
     return 0;
  }
  res->readFrom(*arg, scale->GetValue(), useStr->GetValue());
  return 0;
}


/*
2.2.3 Value Mapping Array and Selection function

*/

ValueMapping toPreciseVM[] = {
     toPreciseVM4<PrecCoord,PrecCoord>,
     toPreciseVM4<CcInt,PrecCoord>,
     toPreciseVM4<CcReal,PrecCoord>,
     toPreciseVM4<LongInt,PrecCoord>,
     toPreciseVM4<Rational,PrecCoord>,
     toPreciseVM4<Point,PrecPoint>,
     toPreciseVM4<Points,precPoints>,
     toPreciseVM4<Region,PrecRegion>
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
  return -1;
}

/*
2.2.4 Specification

*/

OperatorSpec toPreciseSpec(
        " {numeric, point, points, region} -> precise",
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
  8,
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
    
    translateVM1<precPoints,CcInt,CcInt>,
    translateVM1<precPoints,CcInt,CcReal>,
    translateVM1<precPoints,CcInt,LongInt>,
    translateVM1<precPoints,CcInt,Rational>,
    translateVM1<precPoints,CcInt,PrecCoord>,

    translateVM1<precPoints,CcReal,CcInt>,
    translateVM1<precPoints,CcReal,CcReal>,
    translateVM1<precPoints,CcReal,LongInt>,
    translateVM1<precPoints,CcReal,Rational>,
    translateVM1<precPoints,CcReal,PrecCoord>,

    translateVM1<precPoints,LongInt,CcInt>,
    translateVM1<precPoints,LongInt,CcReal>,
    translateVM1<precPoints,LongInt,LongInt>,
    translateVM1<precPoints,LongInt,Rational>,
    translateVM1<precPoints,LongInt,PrecCoord>,

    translateVM1<precPoints,Rational,CcInt>,
    translateVM1<precPoints,Rational,CcReal>,
    translateVM1<precPoints,Rational,LongInt>,
    translateVM1<precPoints,Rational,Rational>,
    translateVM1<precPoints,Rational,PrecCoord>,

    translateVM1<precPoints,PrecCoord,CcInt>,
    translateVM1<precPoints,PrecCoord,CcReal>,
    translateVM1<precPoints,PrecCoord,LongInt>,
    translateVM1<precPoints,PrecCoord,Rational>,
    translateVM1<precPoints,PrecCoord,PrecCoord>,
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
  if(precPoints::checkType(arg1)){
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
      && !precPoints::checkType(nl->First(args))){
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
    
    scaleVM1<precPoints,CcInt,CcInt>,
    scaleVM1<precPoints,CcInt,CcReal>,
    scaleVM1<precPoints,CcInt,LongInt>,
    scaleVM1<precPoints,CcInt,Rational>,
    scaleVM1<precPoints,CcInt,PrecCoord>,

    scaleVM1<precPoints,CcReal,CcInt>,
    scaleVM1<precPoints,CcReal,CcReal>,
    scaleVM1<precPoints,CcReal,LongInt>,
    scaleVM1<precPoints,CcReal,Rational>,
    scaleVM1<precPoints,CcReal,PrecCoord>,

    scaleVM1<precPoints,LongInt,CcInt>,
    scaleVM1<precPoints,LongInt,CcReal>,
    scaleVM1<precPoints,LongInt,LongInt>,
    scaleVM1<precPoints,LongInt,Rational>,
    scaleVM1<precPoints,LongInt,PrecCoord>,

    scaleVM1<precPoints,Rational,CcInt>,
    scaleVM1<precPoints,Rational,CcReal>,
    scaleVM1<precPoints,Rational,LongInt>,
    scaleVM1<precPoints,Rational,Rational>,
    scaleVM1<precPoints,Rational,PrecCoord>,

    scaleVM1<precPoints,PrecCoord,CcInt>,
    scaleVM1<precPoints,PrecCoord,CcReal>,
    scaleVM1<precPoints,PrecCoord,LongInt>,
    scaleVM1<precPoints,PrecCoord,Rational>,
    scaleVM1<precPoints,PrecCoord,PrecCoord>,
 };

int scaleSelect(ListExpr args){
  int o1 = 0; // offset for value to scale
  if(PrecPoint::checkType(nl->First(args))){
      o1 = 0;
  }
  if(precPoints::checkType(nl->First(args))){
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
  if(!precPoints::checkType(nl->First(args))){
      return listutils::typeError(err);
  }
  ListExpr arg2 = nl->Second(args);
  if(!precPoints::checkType(arg2) && !PrecPoint::checkType(arg2)){
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
    containsVM1<precPoints, PrecPoint>,
    containsVM1<precPoints, precPoints>
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
  if(!precPoints::checkType(nl->First(args)) ||
     !precPoints::checkType(nl->Second(args))){
     return listutils::typeError(err);
  }
  return listutils::basicSymbol<CcBool>();
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
    intersectsVM1<precPoints, precPoints> // to be continued
};

int intersectsSelect(ListExpr args){
    return 0;
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
  1,
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
  if(   !precPoints::checkType(nl->First(args))
     || !precPoints::checkType(nl->Second(args))){
     return listutils::typeError(err);
  }
  return listutils::basicSymbol<precPoints>();

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
   unionVM1<precPoints,precPoints, precPoints> // to be continued
};

int unionSelect(ListExpr args){
  return 0;
}

/*
2.8.4 Specification

*/

OperatorSpec unionSpec(
        "  precPoints x precPoints -> precPoints",
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
  1,
  unionVM,
  unionSelect,
  unionTM
);


/*

2.9 Operator intersection

2.9.1 Type Mapping

*/
ListExpr intersectionTM(ListExpr args){

  string err = "precPoints x precPoints expected";
  if(!nl->HasLength(args,2)){
     return listutils::typeError(err);
  }
  if(   !precPoints::checkType(nl->First(args))
     || !precPoints::checkType(nl->Second(args))){
     return listutils::typeError(err);
  }
  return listutils::basicSymbol<precPoints>();

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
   intersectionVM1<precPoints,precPoints, precPoints> // to be continued
};

int intersectionSelect(ListExpr args){
  return 0;
}

/*
2.9.4 Specification

*/

OperatorSpec intersectionSpec(
        "  precPoints x precPoints -> precPoints",
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
  1,
  intersectionVM,
  intersectionSelect,
  intersectionTM
);


/*

2.9 Operator difference

2.9.1 Type Mapping

*/
ListExpr differenceTM(ListExpr args){

  string err = "precPoints x precPoints expected";
  if(!nl->HasLength(args,2)){
     return listutils::typeError(err);
  }
  if(   !precPoints::checkType(nl->First(args))
     || !precPoints::checkType(nl->Second(args))){
     return listutils::typeError(err);
  }
  return listutils::basicSymbol<precPoints>();

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
   differenceVM1<precPoints,precPoints, precPoints> // to be continued
};

int differenceSelect(ListExpr args){
  return 0;
}

/*
2.10.4 Specification

*/

OperatorSpec differenceSpec(
        "  precPoints x precPoints -> precPoints",
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
  1,
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

  MPrecCoordinate c(0,0);
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

    AddOperator(&precise::plusOP);
    AddOperator(&precise::minusOP);
    AddOperator(&precise::mulOP);
    AddOperator(&precise::divideOP);
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

