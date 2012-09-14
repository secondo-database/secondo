
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



/*
~contains~

This operator checks whether a Point is part of a region.
The function returns 0 if the point is outside the region,
1 if the point is in interior and 2 if the point is onborder.

*/
int contains(const Region& reg, const Point& p){
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
         if( RegionCreator::intersects(p.GetX(),p.GetY(),hs)){
             count++;
         }
      }
   }
   size_t mask = 1;
   return (count & mask) > 0;
}






/*
1. Intersection between line and region.

1.1 Auxiliary function processParallel

Inserts a range into result vector res defining the part of hs
overlaped by hsreg.

*/
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
      delta1 = (ry1-ldy) / ldy;
      delta2 = (ry2-ldy) / ldy;

   } else { // use x for computing delta
      double ry1 = hsreg.GetDomPoint().GetY();
      double ry2 = hsreg.GetSecPoint().GetY();
      delta1 = (ry1-ldy) / ldy;
      delta2 = (ry2-ldy) / ldy;
   }
   if(delta1>delta2){
     double tmp = delta1;
     delta1 = delta2;
     delta2 = tmp;
   }
   if( (delta1 >= 1.0) || AlmostEqual(delta1,1.0)){
      return;
   }
   if((delta2<=0) || AlmostEqual(delta1,0.0)){
      return;
   }
   if(delta1<0){
     delta1=0;
   }
   if(delta2>1.0){
      delta2=1.0;
   }
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

void addSplitPoint( const HalfSegment hsline, 
                    const HalfSegment& hsreg , 
                    vector<pair<double, bool> >& res){




   double lx1 = hsline.GetDomPoint().GetX();
   double ly1 = hsline.GetDomPoint().GetY();
   double lx2 = hsline.GetSecPoint().GetX();
   double ly2 = hsline.GetSecPoint().GetY();

   double rx1 = hsreg.GetDomPoint().GetX();
   double ry1 = hsreg.GetDomPoint().GetY();
   double rx2 = hsreg.GetSecPoint().GetX();
   double ry2 = hsreg.GetSecPoint().GetY();

   double u = lx2-lx1;
   double v = rx1-rx2;
   double w = lx1-rx1;
   double x = ly2-ly1;
   double y = ry1-ry2; 
   double z = ly1-ry1;

   double k = y*u-v*x;

   if(AlmostEqual(k,0)){ // parallel segments
      //cout << "parallel segments found " << endl;
      processParallel(hsline,hsreg,res);
      return; 
   }

   double delta2 = (w*x-z*u) / k;
   double delta1;
   if(abs(u) > abs(x)){
      delta1 = -1*((w+delta2*v)/u);
   } else {
      delta1 = -1*((z+delta2*y)/x);
   }   
   if(delta1<0 || delta2 < 0){  // intersection point outside segments
        return;
   }   
   if(delta1>1 || delta2 > 1){  // intersection point outside splitpart
      return;
   }   

   if(AlmostEqual(delta1,0.0)){ // intersection point is the dominating point
     // no split -> ignore
     return;  
   }
   if(AlmostEqual(delta1,1.0)){
      return;
   }
   bool leftBehindDelta = !RegionCreator::isRight(hsreg.GetDomPoint(), 
                                                  hsreg.GetSecPoint(), 
                                                  hsline.GetSecPoint()); 
   bool insideLeft = hsreg.insideLeft();
   res.push_back(pair<double,bool>(delta1,leftBehindDelta == insideLeft));
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


/*
Evaluates the splitpoints and creates the halfsegment
parts inside the region.

*/
void insertLineParts(HalfSegment& hs, 
                     vector<pair<double,bool> > splitpoints, 
                     const Region& region,
                     Line& result,
                     int& edgeno){



    //cout << "insertLineParts called " << endl;
    //cout << "no SplitPoints = " << splitpoints.size() << endl;


    // cout << "called insertLineParts" << endl;
    


    if(splitpoints.empty()){ // segment completely inside or 
      // cout << "No intersection, no problems" << endl;
       // completely outside the region
       // because, we ignore 'splitpoint' at the endpoints, we 
       // have to use an inner point of the halfsegment for 
       // checking the containedness
      // cout << "no SPlitPoints " << endl;
      // cout << "check " << hs.middlePoint() << " for containedness" << endl;
       if(contains(region,hs.middlePoint())){
         // cout << "Region contains point" << endl;
          hs.attr.edgeno = edgeno;
          result += hs;
          hs.SetLeftDomPoint(!hs.IsLeftDomPoint());
          result += hs;
          edgeno++; 
       } else {
          // cout << "point is not contained" << endl;
       }
       //cout << "finsihed" << endl;
       return;
    }



    // halfsegment was split by splitpoints
    // step 1: sort splitpoints by delta value
    sort(splitpoints.begin(), splitpoints.end(),splitpointless);
    // if not present, 
    if(!AlmostEqual(splitpoints[0].first,0.0)){
       splitpoints.insert(splitpoints.begin(),
                          pair<double,bool>(0.0,!splitpoints[0].second));
    } 
    if(!AlmostEqual(splitpoints.back().first,1.0)){
       splitpoints.push_back(pair<double,bool>(1.0,!splitpoints.back().second));
    }



    //cout << " go to complex case" << endl;
    // now, process splitpoints having equal delta values
    // within such a sequence, all splits can have the same direction
    // in this case, we keep only one of the group
    // if there are different split directions, we remove all splits at
    // this position
    vector<pair<double,bool> > splitpoints2;
    size_t pos = 0;
    while(pos <  splitpoints.size()){
      pair<double,bool> first = splitpoints[pos];
      bool differ = false;
      size_t pos2 = pos + 1;
      bool done = pos2 >= splitpoints.size();
      while(!done){
         if(!AlmostEqual(first.first,splitpoints[pos2].first)){
            done = true;
         } else {
            differ = first.second != splitpoints[pos2].second;
            pos2++;
         }
      }
      if(!differ){
        splitpoints2.push_back(first);
      } 
      pos = pos2;
    }



    //cout << "part 1 finished" << endl;
    
    if(splitpoints2.empty()){ // segment completely inside or 
       // TODO: ensure, that the used point is not a former
       // splitpoint   
       if(contains(region,hs.middlePoint())){
          hs.attr.edgeno = edgeno;
          result += hs;
          hs.SetLeftDomPoint(!hs.IsLeftDomPoint());
          result += hs;
          edgeno++; 
       } 
       return;
    }

    //cout << "splits are present " << endl;


    pos = 0;
    while(pos < splitpoints2.size()){
        //cout << "start while at pos " << pos << endl;
        while(pos < (splitpoints2.size())-1 && !splitpoints2[pos].second){
            pos++;
        }
        //cout << "start found at position " << pos << endl;

        if(pos < splitpoints2.size()-1){ // found starting fragment
 
           Point p1 = atDelta(hs,splitpoints2[pos].first);

           while(pos < splitpoints2.size() && splitpoints2[pos].second){
              pos++;
           }
           //cout << "found end part at " << pos << endl;
           double s = 1.0;
           if(pos<splitpoints2.size()){
              s = splitpoints2[pos].first;
              pos++;
           } 
           Point p2 = atDelta(hs,s);
           if(!AlmostEqual(p1,p2)){
               // create and insert halfsegment
               HalfSegment hs(true,p1,p2);
               hs.attr.edgeno = edgeno;
               result += hs;
               hs.SetLeftDomPoint(!hs.IsLeftDomPoint());
               result += hs;
               edgeno++;
           }
        } else {
           pos++;
        }
    }
    //cout << "all fisnihed" << endl;

 }

/*
 Intersection between region and line.


*/
void intersection(const Region& region, const Line& line, Line& result){



   result.Clear();
   if(!region.IsDefined() || ! line.IsDefined()){
      result.SetDefined(false);
      return;
   }
   result.SetDefined(true);

   if(region.IsEmpty() || line.IsEmpty()){ 
      // interscetion with empty set is empty
      return;
   }

   if(!region.BoundingBox().Intersects(line.BoundingBox())){
       // non-intersecting bounding boxes
       return; 
   }

   result.Resize(line.Size());
   
   // 1st. Build an RTree and insert the halfsegments of the region

   mmrtree::RtreeT<2,int> tree(4,8);
   HalfSegment hs;
   for(int i=0;i<region.Size();i++){
     region.Get(i,hs);
     if(hs.IsLeftDomPoint()){
       tree.insert(hs.BoundingBox(),i);
     }  
   }
   //cout << "R-tree built" << endl;
 
   result.StartBulkLoad();
   int edgeno = 0;
   for(int i=0;i<line.Size();i++){
      line.Get(i,hs);
      if(hs.IsLeftDomPoint()){
          vector<pair<double, bool> > splitPoints;
          mmrtree::RtreeT<2,int>::iterator* it = tree.find(hs.BoundingBox());
          HalfSegment hs2;
          int const* pos;
          while(  (pos = it->next()) != 0){
             region.Get(*pos,hs2);
             addSplitPoint(hs,hs2,splitPoints);
          }
          delete it;
          insertLineParts(hs,splitPoints, region, result, edgeno);
      } 
   }
   result.EndBulkLoad(true,false); 
}
