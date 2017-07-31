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

1 Implementation file for robust set operations for spatial objects.

*/

#include <vector>
#include <algorithm>

#include "RobustSetOps.h"
#include "SpatialAlgebra.h"
#include "MMRTree.h"
#include "RegionCreator.h"
#include "SecondoSystem.h"


using namespace std;

namespace robust{

/*
~contains~

This operator checks whether a Point is part of a region.
The function returns 0 if the point is outside the region,
1 if the point is in interior and 2 if the point is onborder.

*/
template<template<typename T> class Array>
int contains(const RegionT<Array>& reg, const Point& p){
   if(!reg.IsDefined() || !p.IsDefined()){
     return false;
   }
   if(reg.IsEmpty()){
      return false;
   }
   size_t count = 0;
   HalfSegment hs; 
   for(int i=0;i<reg.Size();i++){
      reg.Get(i,hs);
      if(hs.IsLeftDomPoint()){
         if(hs.Contains(p)){
            return 2;
         } 
         if( RegionCreator<Array>::intersects(p.GetX(),p.GetY(),hs)){
             count++;
         }
      }
   }
   size_t mask = 1;
   return (count & mask) > 0;
}

template int contains<DbArray >(const RegionT<DbArray>& reg, 
                                const Point& p);
template int contains<MMDbArray >(const RegionT<MMDbArray>& reg, 
                                  const Point& p);


bool lineContains(const HalfSegment& hs, const Point& p, double& delta){


   // checks whether the line defined by hs contains p
   double x1 = hs.GetDomPoint().GetX(); 
   double y1 = hs.GetDomPoint().GetY(); 
   double x2 = hs.GetSecPoint().GetX(); 
   double y2 = hs.GetSecPoint().GetY(); 

   double dx = x2-x1;
   double dy = y2-y1;
  
   double x,y;

   if(abs(dx)>abs(dy)){
     x = p.GetX();
     delta = (x-x1)/ dx;
     y = y1 + delta * dy;
   }   else {
     y = p.GetY();
     delta = (y-y1)/dy;
     x = x1 + delta*dx;
   }
   return AlmostEqual(p, Point(true,x,y));
}

bool lineContains(const HalfSegment& hs, const Point& p){
   double delta;
   return lineContains(hs,p,delta);
}




bool onSameLine(const HalfSegment& hs1, const HalfSegment& hs2){
   if(hs1.Length() > hs2.Length()){
      bool p1c = lineContains(hs1, hs2.GetDomPoint());
      bool p2c = lineContains(hs1,hs2.GetSecPoint());
      return p1c && p2c;
   } else {
      bool p1c = lineContains(hs2, hs1.GetDomPoint());
      bool p2c = lineContains(hs2, hs1.GetSecPoint());
      return p1c && p2c;
   }
}






/*
1. Intersection between line and region.

1.1 Auxiliary function processParallel

Inserts a range into result vector res defining the part of hs
overlaped by hsreg.

*/

bool overlaps(const HalfSegment& hs1, const HalfSegment& hs2){
  if(!onSameLine(hs1,hs2)){
     return false;
  }
  double delta;
  Point dp1 = hs1.GetDomPoint();
  Point sp1 = hs1.GetSecPoint();
  
  Point dp2 = hs2.GetDomPoint();
  Point sp2 = hs2.GetSecPoint();

  if(AlmostEqual(dp1,dp2) && // equal segments
     AlmostEqual(sp1,sp2)){
    return true;
  }
  if(AlmostEqual(dp1,sp2) && // equal segments
     AlmostEqual(sp1,dp2)){
    return true;
  }

  lineContains(hs1,dp2, delta);
  if((delta>0) && (delta < 1)){
     if(!AlmostEqual(dp1,dp2) && !AlmostEqual(sp1,dp2)){ 
         return true;
     }
  }
  lineContains(hs1,sp2, delta);
  if((delta>0) && (delta < 1)){
     if(!AlmostEqual(dp1,sp2) && !AlmostEqual(sp1,sp2)){ 
        return true;
     }
  }
  lineContains(hs2,dp1, delta);
  if((delta>0) && (delta < 1)){
     if(!AlmostEqual(dp2,dp1) && !AlmostEqual(sp2,dp1)){ 
       return true;
     }
  }
  lineContains(hs2,sp1, delta);
  if((delta>0) && (delta < 1)){
     if(!AlmostEqual(dp2,sp1) && !AlmostEqual(sp2,sp1)){ 
        return true;
     }
  }
  return false;
}


void processParallel(const HalfSegment& hsline,
                     const HalfSegment& hsreg,
                     vector<pair<double,bool> >& res){

   double lx1 = hsline.GetDomPoint().GetX();
   double ldx = hsline.GetSecPoint().GetX() - lx1;
   double ly1 = hsline.GetDomPoint().GetY();
   double ldy = hsline.GetSecPoint().GetY() - ly1;
   
   double delta1;
   double delta2;
   if(ldy>ldx){ // use y for computing delta
      double ry1 = hsreg.GetDomPoint().GetY();
      double ry2 = hsreg.GetSecPoint().GetY();
      delta1 = (ry1-ly1) / ldy;
      delta2 = (ry2-ly1) / ldy;

   } else { // use x for computing delta
      double rx1 = hsreg.GetDomPoint().GetX();
      double rx2 = hsreg.GetSecPoint().GetX();
      delta1 = (rx1-lx1) / ldx;
      delta2 = (rx2-lx1) / ldx;
   }
   if(delta1>delta2){
     double tmp = delta1;
     delta1 = delta2;
     delta2 = tmp;
   }

   //cout << "delta1 = " << delta1 << endl;
   // cout << "delta2 = " << delta2 << endl;


   if( (delta1 >= 1.0) || AlmostEqual(delta1,1.0)){
      //cout << " split right of the segment" << endl;
      return;
   }
   if((delta2<=0) || AlmostEqual(delta2,0.0)){
      //cout << " split left of the segment" << endl;
      return;
   }
   if(delta1<0){
     delta1=0;
   }
   if(delta2>1.0){
      delta2=1.0;
   }
   //cout << "push_back " << endl;
   res.push_back(pair<double,bool>(delta1,true));
   res.push_back(pair<double,bool>(delta2,false));
}

/*

1.2 auxiliary function ~addSplitPoints~

If hsline and hsreg are intersecting, a pair (double, bool) is inserted into
res. The double (delta) value determines the relative position of the splitpoint
(a value in [0,1] starting at the dominating point. If the boolean parameter
is set to true, the region covers the part after delta, otherwise the parte 
before delta.


*/

bool computeSplitPoint(const HalfSegment& hs1, 
                       const HalfSegment& hs2,
                       double& delta1,
                       double& delta2){


   double lx1 = hs1.GetDomPoint().GetX();
   double ly1 = hs1.GetDomPoint().GetY();
   double lx2 = hs1.GetSecPoint().GetX();
   double ly2 = hs1.GetSecPoint().GetY();

   double rx1 = hs2.GetDomPoint().GetX();
   double ry1 = hs2.GetDomPoint().GetY();
   double rx2 = hs2.GetSecPoint().GetX();
   double ry2 = hs2.GetSecPoint().GetY();

   double u = lx2-lx1;
   double v = rx1-rx2;
   double w = lx1-rx1;
   double x = ly2-ly1;
   double y = ry1-ry2; 
   double z = ly1-ry1;

   double k = y*u-v*x;

   if(k==0){ // segments are parallel
      return false; 
   }

   delta2 = (w*x-z*u) / k;

   if(abs(u) > abs(x)){
      delta1 = -1*((w+delta2*v)/u);
   } else {
      delta1 = -1*((z+delta2*y)/x);
   }   

   return true;
}




/*
Compare function.

*/
bool splitpointless(const pair<double,bool>& a, const pair<double,bool>& b){
   return a.first < b.first;
}

/*
Returns the point relative on hs. delta must be in [0,1] to return a 
Point on the halfsegment.

*/

Point atDelta(const HalfSegment& hs, const double& delta){

  Point p1 = hs.GetDomPoint();
  Point p2 = hs.GetSecPoint();

  double x = p1.GetX() + delta*(p2.GetX()-p1.GetX());
  double y = p1.GetY() + delta*(p2.GetY()-p1.GetY());
  return Point(true,x,y);

}




string splitkind(splitKind i){
   switch(i){
     case normalSplit : return "normalSplit split point";
     case startCommon : return "start of already processed common part";
     case endCommon : return "end of already processed common part" ;
     default: return  "unknown" ;
     
   }
}





/*
Checks two haslfsegments which are on the same line for a common
subsegment. If there is a such segment, true is returned.
The delta values are set as the relative position
(range (0,1) of hs1.

*/

bool commonPart(const HalfSegment& hs1, const HalfSegment& hs2,
                double& delta1, double& delta2){
   lineContains(hs1, hs2.GetDomPoint(), delta1);
   lineContains(hs1, hs2.GetSecPoint(), delta2); 
   if(delta1>delta2){
     double tmp = delta1;
     delta1 = delta2;
     delta2 = tmp;
   }
   if((delta1 >= 1)  || (delta2 <=0)){ // no overlappings
      return false;
   }
   if(delta1<0){
     delta1 = 0;
   }
   if(delta2>1){
     delta2 = 1;
   }
   return true;
}

void realminizeParallel(const HalfSegment& hs1, const HalfSegment& hs2,
                        vector<pair<double,splitKind> >& splitPoints){


   double delta1;
   double delta2;
   if(commonPart(hs1,hs2,delta1,delta2)){
     splitPoints.push_back(pair<double,splitKind>(delta1,startCommon));
     splitPoints.push_back(pair<double,splitKind>(delta2,endCommon)); 
   }
}


void realminize(HalfSegment& hs1, HalfSegment& hs2, const bool secondFirst, 
                vector<pair<double,splitKind> >& splitpoints){


   if(onSameLine(hs1, hs2)){
       if(secondFirst){
          realminizeParallel(hs1, hs2, splitpoints);
       } 
       return;
   }
   double delta1, delta2;
   bool intersecting;
   if(secondFirst){ 
      intersecting = computeSplitPoint (hs1,hs2,delta1,delta2);
   } else {
      intersecting = computeSplitPoint (hs2,hs1,delta2,delta1);
   }
   if(!intersecting){ // no intersection found
      return;
   } 

   if(AlmostEqual(delta1,0)){
      Point p = atDelta(hs1,delta1);
      if(AlmostEqual(p,hs1.GetDomPoint())){
         delta1 = 0;
      }    
   }
   if(AlmostEqual(delta1,1)){
      Point p = atDelta(hs1,delta1);
      if(AlmostEqual(p,hs1.GetSecPoint())){
          delta1 = 1;
      }
   }
   if((delta1<0) || (delta1 > 1)){
      return;
   }
   splitpoints.push_back(pair<double,splitKind>(delta1,normalSplit));
   return;
}




void crossings(const HalfSegment& hs1, 
               const HalfSegment& hs2,
               set<Point,ApproxPointLess>& candidates,
               set<Point,ApproxPointLess>& falseHitsCandidates){


   if(!onSameLine(hs1,hs2)){
     double delta1, delta2;
     bool ok = computeSplitPoint(hs1,hs2,delta1,delta2);
     if(ok && (delta1>=0) && (delta1<=1) && (delta2>=0) && (delta2<=1)){
        candidates.insert(atDelta(hs1,delta1));
     }
     return;
   } else {
      double delta1, delta2;
      if(commonPart(hs1,hs2,delta1,delta2)){
         falseHitsCandidates.insert(atDelta(hs1,delta1));
         falseHitsCandidates.insert(atDelta(hs1,delta2));
      }
   }
   
}




























} // end of namespace robust
