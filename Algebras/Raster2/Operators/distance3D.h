/*

This file is part of SECONDO.

Copyright (C) 2011, University in Hagen, Department of Computer Science,
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

*/

#include "StandardTypes.h"
#include "SpatialAlgebra.h"
#include "CellIterator.h"
#include "../sint.h"
#include "../sreal.h"
#include "ListUtils.h"
#include "AlgebraTypes.h"


/*

1 Operator ~distance3D~

This operator computes the 3D distance between 2 points on a surface 


st can be an sint or an sreal.

The raster is the raster representing elevation.
p1 and p2 are the points between the distance is to compute.
geoid defines the geoid for computing the 2D distance between p1 and p2
If geoid is null, the euklidean distance is used.
If setwise is set to false, just the elevation at p1 and p2 is used, otherwise
also the elevation at intermedian cells. If one of cells is undefined in the 
raster the difference between the elevations is assumed to be zero.


1.1 the function

*/

namespace raster2{


template<class st>
CcReal distance3DFun(const st& raster, const Point& p1, const Point& p2,
                  const Geoid* geoid, const CcBool& stepwise){
  CcReal result(true,0);
  if( !p1.IsDefined() || !p2.IsDefined() || !stepwise.IsDefined()){
    result.SetDefined(false);
    return result;
  }
  if(geoid!=0 && !geoid->IsDefined()){
    result.SetDefined(false);
    return result;
  }
  if(!stepwise.GetValue()){ // simple case
     double dist2 = p1.Distance(p2,geoid);
     double h1 = raster.atlocation(p1.GetX(), p1.GetY());
     double h2 = raster.atlocation(p2.GetX(), p2.GetY());
     double hd = 0.0; 
     if(!raster.isUndefined(h1) && !raster.isUndefined(h2)){
       hd = h1-h2;
     }

     double dist3 = sqrt( dist2*dist2 + hd*hd);
     result.Set(true,dist3);
     return result;
  }
  // stepwise computation
  CellIterator it(raster.getGrid(),p1.GetX(),p1.GetY(),p2.GetX(),p2.GetY()); 
  if(!it.hasNext()){ // equal points
    result.Set(true,0);
    return result;
  }
  double dx = p2.GetX() - p1.GetX();
  double dy = p2.GetY() - p1.GetY();
  double x1 = p1.GetX();
  double y1 = p1.GetY();

  pair<double,double> last=it.next();

  if(!it.hasNext()){ // within a single cell
     result.Set(true, p1.Distance(p2,geoid));
     return result;
  }

  double dist3 = 0.0;
  
  while(it.hasNext()){
     pair<double,double> current = it.next();
     double d1 = last.first==0?0:(last.first+last.second)/2;
     double d2 = current.second==1?1:(current.first+current.second)/2;
     // first point
     double x = x1 + d1*dx;
     double y = y1 + d1*dy;
     double h1 = raster.atlocation(x,y);
     Point p(true,x,y);
     // second point
     x = x1 + d2*dx;
     y = y1 + d2*dy;
     double h2 = raster.atlocation(x,y);
     Point q(true,x,y);

     double dist2 = p.Distance(q,geoid);
     double hd = 0.0;
     if(!raster.isUndefined(h1) && !raster.isUndefined(h2)){
       hd = h1-h2;
     }
     double dist = sqrt( dist2*dist2 + hd*hd);
     dist3 +=dist;
     last = current;
  }
  result.Set(true,dist3);
  return result;
}

/*
2. Secondo wrapper for distance function

2.1 Type Mapping

Signature is {sint, sreal} x point x point x bool [x geoid] -> real

*/

ListExpr distance3DTM(ListExpr args){
  string err = "{sint,sreal} x point x point x bool [x geoid] expected";
  int len = nl->ListLength(args);
  if(len!=4 && len!=5){
     return listutils::typeError(err);
  } 
  ListExpr arg = nl->First(args);
  if(!sint::checkType(arg) && !sreal::checkType(arg)){
     return listutils::typeError(err);
  }
  if(!Point::checkType(nl->Second(args)) ||
     !Point::checkType(nl->Third(args)) ||
     !CcBool::checkType(nl->Fourth(args))){
     return listutils::typeError(err);
  } 
  if(len==5 && !Geoid::checkType(nl->Fifth(args))){
     return listutils::typeError(err);
  }
  return listutils::basicSymbol<CcReal>();
}

/*
2.2 Value Mapping Implementation

*/
template<class st>
int distance3DVM1 (Word* args, Word& result, int message, 
                  Word& local, Supplier s) {
   st* raster = (st*) args[0].addr;
   Point* p1 = (Point*) args[1].addr;
   Point* p2 = (Point*) args[2].addr;
   CcBool* stepwise = (CcBool*) args[3].addr;
   Geoid* geoid = qp->GetNoSons(s)==5
                      ? (Geoid*) args[4].addr
                      : 0;

   result = qp->ResultStorage(s);
   CcReal* res = (CcReal*) result.addr;
   (*res) = distance3DFun<st>(*raster, *p1, *p2, geoid, *stepwise);
   return 0;
}

/*
2.3 Value Mapping Array

*/
ValueMapping distance3DVM[] = {
      distance3DVM1<sint>,
      distance3DVM1<sreal>
   };

/*
2.4 Selection function

*/
int distance3DSelect(ListExpr args){
  return sint::checkType(nl->First(args))?0:1;
}

/*
2.5 Specification

*/
OperatorSpec distance3DSpec(
 "{sint,sreal} x point x point x bool [x geoid] -> real",
 "distance3D(_,_,_,_)",
 "Computes the 3D distance between 2 points using the elevation information"
 " from a raster. If the boolean parameter is set to be true, also the"
 " elevation information of intermediate cells is used. ",
 "query distance3D( raster1, p1, p2, TRUE, geoid1");


/*
2.6 Operator instance

*/
Operator distance3D(
   "distance3D",
   distance3DSpec.getStr(),
   2,
   distance3DVM,
   distance3DSelect,
   distance3DTM 
);


/*
3 Operator ~length3D~

This operator computes the lenegth of the way of a moving point using elevation information of a raster

3.1 Type mapping

*/
ListExpr length3DTM(ListExpr args){
  string err = "{sint,sreal} x mpoint x bool [x geoid] expected";
  int len = nl->ListLength(args);
  if(len!=3 && len!=4){
     return listutils::typeError(err +" wrong number of args");
  } 
  ListExpr arg = nl->First(args);
  if(!sint::checkType(arg) && !sreal::checkType(arg)){
     return listutils::typeError(err  + " first arg is not a raster");
  }
  if(!MPoint::checkType(nl->Second(args)) ){
     return listutils::typeError(err + " second arg is not an mpoint");
  }
   
  if(!CcBool::checkType(nl->Third(args)) ){
     return listutils::typeError(err + " third arg is not a bool");
  } 
  if(len==4 && !Geoid::checkType(nl->Fourth(args))){
     return listutils::typeError(err + "fourth arg is not a geoid");
  }
  return listutils::basicSymbol<CcReal>();
}

/*
3.2 Value Mapping

*/
template<class st>
int length3DVM1 (Word* args, Word& result, int message, 
                  Word& local, Supplier s) {
   st* raster = (st*) args[0].addr;
   MPoint* mp = (MPoint*) args[1].addr;
   CcBool* stepwise = (CcBool*) args[2].addr;
   Geoid* geoid = qp->GetNoSons(s)==4
                      ? (Geoid*) args[3].addr
                      : 0;

   result = qp->ResultStorage(s);
   CcReal* res = (CcReal*) result.addr;
   if(!mp->IsDefined()){
     res->SetDefined(false);
     return 0;
   }
   res->Set(true,0);
   for(int i=0;i<mp->GetNoComponents();i++){
      UPoint unit;
      mp->Get(i,unit);
      (*res) += distance3DFun<st>(*raster,unit.p0,unit.p1,geoid,*stepwise);
   }
   return 0;
}

/*
2.3 Value Mapping Array

*/
ValueMapping length3DVM[] = {
      length3DVM1<sint>,
      length3DVM1<sreal>
   };

/*
2.4 Selection function

*/
int length3DSelect(ListExpr args){
  return sint::checkType(nl->First(args))?0:1;
}

/*
2.5 Specification

*/
OperatorSpec length3DSpec(
 "{sint,sreal} x mpoint x bool [x geoid] -> real",
 "length3D(_,_,_,_)",
 "Computes the length of the trajectory of a moving point"
 " using elevation information of a raster"
 " If the boolean parameter is set to be true, for each unit also the"
 " elevation information of intermediate cells is used. ",
 "query length3D( raster1, mp, TRUE, geoid1");


/*
2.6 Operator instance

*/
Operator length3D(
   "length3D",
   length3DSpec.getStr(),
   2,
   length3DVM,
   length3DSelect,
   length3DTM 
);

} // end of namespace raster2
