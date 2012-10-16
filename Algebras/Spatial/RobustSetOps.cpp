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




namespace robust{

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


   //cout << "called addSplitPoint" << endl;
   //cout << "hsline = " << hsline.getLineString() << endl;
   //cout << "hsreg = " << hsreg.getLineString() << endl;


   //cout << "k = " << k << endl; 

   //if(AlmostEqual(k,0)){ // parallel segments
   if(k==0){
      //cout << "parallel segments found " << endl;
      processParallel(hsline,hsreg,res);
      return; 
   }

   double delta2 = (w*x-z*u) / k;

   //cout << "delta2 = " << delta2 << endl;
   double delta1;
   if(abs(u) > abs(x)){
      delta1 = -1*((w+delta2*v)/u);
   } else {
      delta1 = -1*((z+delta2*y)/x);
   }   

   //cout << "delta1 = " << delta1 << endl;

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

   //cout << "insert delta " << delta1 << endl;
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


    if(splitpoints.empty()){ // segment completely inside or 
      // cout << "No intersection, no problems" << endl;
       // completely outside the region
       // because, we ignore 'splitpoint' at the endpoints, we 
       // have to use an inner point of the halfsegment for 
       // checking the containedness
      // cout << "no SPlitPoints " << endl;
      // cout << "check " << hs.middlePoint() << " for containedness" << endl;

      // if(contains(region,hs.middlePoint())){
       if(region.InInterior(hs.middlePoint())){ 
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

   //cout << "build r-tree" << endl;

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





enum splitKind  {normalSplit, startCommon, endCommon};

class splitPointComp{
  public:
  bool operator()(const pair<double,splitKind>& p1, 
                  const pair<double,splitKind>& p2){
    if(p1.first<p2.first){
      return true;
    }
    if(p1.first>p2.first){
       return false;
    }
    return p1.second < p2.second;  
  }

};





string splitkind(splitKind i){
   switch(i){
     case normalSplit : return "normalSplit split point";
     case startCommon : return "start of already processed common part";
     case endCommon : return "end of already processed common part" ;
     default: return  "unknown" ;
     
   }
}

bool insertSegment(const HalfSegment& hs, 
                   const double pos1, const double pos2, 
                   DbArray<HalfSegment>& result, int& edgeno){

    Point p1 = atDelta(hs,pos1);
    Point p2 = atDelta(hs,pos2);
    if(AlmostEqual(p1,p2)){
       // too short for building a halfsegment
       return false;
    }
    HalfSegment res(true,p1,p2);
    res.attr.edgeno = edgeno;
    result.Append(res);
    res.SetLeftDomPoint(!res.IsLeftDomPoint());
    result.Append(res);
    edgeno++;
    return true;
}


void insertSegmentParts(HalfSegment& hs, 
                        vector< pair<double, splitKind> >& splitpoints, 
                        DbArray<HalfSegment>& result, 
                        int& edgeno){


   //cout << "insertsegmentparts called for " << hs.SimpleString() << endl;

   if(splitpoints.size()==0){ // keep original Segment
       //cout << "no split, insert hs" << endl;
       hs.attr.edgeno = edgeno;
       result.Append(hs);
       hs.SetLeftDomPoint(!hs.IsLeftDomPoint());
       result.Append(hs);
       edgeno++;
       return;
   }
   splitPointComp cmp;
   sort(splitpoints.begin(), splitpoints.end(),cmp);
   if((splitpoints[0].first!=0.0) || (splitpoints[0].second!=normalSplit) ){
      splitpoints.insert(splitpoints.begin(), 
                         pair<double,splitKind>(0.0,normalSplit));
   }
   if((splitpoints.back().first!=1.0) || 
      (splitpoints.back().second!=normalSplit)){
      splitpoints.push_back(pair<double,splitKind>(1.0,normalSplit));
   }
   double lastPos = splitpoints[0].first;
   splitKind lastKind = splitpoints[0].second;
   int numCommonParts = 0;
   for(size_t i=1;i<splitpoints.size();i++){
      double currentPos = splitpoints[i].first;
      splitKind currentKind = splitpoints[i].second;
      switch(currentKind){
        case normalSplit: {
             if(lastKind==normalSplit){
                // otherwise, this plit is within an already processed part
                bool ok = insertSegment(hs,lastPos,currentPos,result,edgeno);
                if(ok){
                   lastPos = currentPos;
                }  
             } 
             break;
        }
        case startCommon : {
           if(lastKind==normalSplit){
             assert(numCommonParts==0);
             insertSegment(hs,lastPos,currentPos, result, edgeno);
             lastPos = currentPos;
             lastKind = startCommon;
           }
           numCommonParts++;
           break;
        }
        case endCommon : {
           assert(numCommonParts>0);
           numCommonParts--;
           if(numCommonParts==0){
              lastPos = currentPos;
              lastKind = normalSplit;
           }
           break;
        }
        default : assert(false);
      }
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


void realminize(const DbArray<HalfSegment>& src, 
                      DbArray<HalfSegment>& result){
    if(src.Size()==0){
      result.clean();
      return;
    }
    // the result size is at least the size of src
    result.resize(src.Size());

    // step1: insert halfSegments into an mmrtree
    mmrtree::RtreeT<2,int> tree(4,8);
    HalfSegment hs;
    for(int i=0;i<src.Size();i++){
      src.Get(i,hs);
      if(hs.IsLeftDomPoint()){
         tree.insert(hs.BoundingBox(),i);
      }  
    }
    // step 2 process halfsegments
    int edgeno = 0;
    for(int i=0;i<src.Size();i++){
       src.Get(i,hs);
       if(hs.IsLeftDomPoint()){
          //cout << "process Segment" << hs.SimpleString() << endl;

          mmrtree::RtreeT<2,int>::iterator* it = tree.find(hs.BoundingBox());
          HalfSegment hs2;
          int const* pos;
          vector<pair<double,splitKind> > splitPoints;
          while( (pos = it->next()) != 0){
              if((*pos)!=i){
                 src.Get(*pos,hs2);
                 //cout << "Test with" << hs2.SimpleString() << endl;
                 realminize(hs,hs2, (*pos) < i , splitPoints);   
              }
          }
          delete it;
          insertSegmentParts(hs,splitPoints,result, edgeno); 
       }
    }
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

void crossings(const Line& l1, 
               const Line& l2,
               Points& result) {
    if(!l1.IsDefined() || !l2.IsDefined()){
       result.SetDefined(false);
       return;
    }
    result.Clear();
    result.SetDefined(true);

    //build an r-tree from the first line

    // step1: insert halfSegments into an mmrtree
    mmrtree::RtreeT<2,int> tree(4,8);
    HalfSegment hs;
    for(int i=0;i<l1.Size();i++){
      l1.Get(i,hs);
      if(hs.IsLeftDomPoint()){
         tree.insert(hs.BoundingBox(),i);
      }  
    }

    // step 2 process halfsegments of l2
    set<Point,ApproxPointLess> candidates;
    set<Point,ApproxPointLess> falseHitCandidates;

    for(int i=0;i<l2.Size();i++){
       l2.Get(i,hs);
       if(hs.IsLeftDomPoint()){
          mmrtree::RtreeT<2,int>::iterator* it = tree.find(hs.BoundingBox());
          HalfSegment hs2;
          int const* pos;
          while( (pos = it->next()) != 0){
             l1.Get(*pos,hs2);
             crossings(hs,hs2,candidates, falseHitCandidates);   
          }
          delete it;
       }
    }

    result.StartBulkLoad();
    set<Point,ApproxPointLess>::iterator it;
  
    for(it = candidates.begin();it!=candidates.end();it++){
       if(falseHitCandidates.find(*it) == falseHitCandidates.end()){
          result += *it;
       }
    } 
   
    result.EndBulkLoad();

}


 bool RealmChecker::isRealm(const HalfSegment& hs1, 
                            const HalfSegment& hs2, 
                            const bool print){
    if(onSameLine(hs1,hs2)){
       bool ok =  !overlaps(hs1,hs2);
       if(!ok && print){
          cout << "found overlapping segments " << endl;
          cout << "S1 : " << hs1.getLineString() << endl;
          cout << "S2 : " << hs2.getLineString() << endl;
       }
       return ok;
    } else {

       Point dp1 = hs1.GetDomPoint();
       Point sp1 = hs1.GetSecPoint();
       Point dp2 = hs2.GetDomPoint();
       Point sp2 = hs2.GetSecPoint();
       if(AlmostEqual(dp1,dp2) || AlmostEqual(dp1,sp2) ||
           AlmostEqual(sp1,dp2) || AlmostEqual(sp1,sp2) ){
         return  true;
       }


       double delta1, delta2;
       bool intersection = computeSplitPoint(hs1,hs2,delta1,delta2);
       if(!intersection){ // parallel segments
          return true;
       }
       bool ok = true;
       if( (0 < delta1) && (delta1 < 1)){
          if( (delta2>=0) && (delta2<=1)){
              
              ok =  false;
          }
       }
       if( (0 < delta2) && (delta2 < 1)){
          if( (delta1>=0) && (delta1<=1)){
             ok = false;
          }
       }
       if(!ok && print){
         cout << "found crossing segments" << endl;
         cout << "S1 : " << hs1.getLineString() << endl;
         cout << "S2 : " << hs2.getLineString() << endl;
         cout << "delta1 = " << delta1 << endl;
         cout << "delta2 = " << delta2 << endl;
       }
       return ok;
    }
    

 }



RealmChecker::RealmChecker(const DbArray<HalfSegment>* _hss):
    hss(_hss), tree(4,8), pos(0), it(0) {

    ListExpr numTupleType = 
            SecondoSystem::GetCatalog()->NumericType(getTupleType());
    tt = new TupleType(numTupleType);
    HalfSegment hs1;
    for(int i=0;i<hss->Size();i++){
      hss->Get(i,hs1);
      if(hs1.IsLeftDomPoint()){
         tree.insert(hs1.BoundingBox(),i);
      }  
    }
}

void RealmChecker::reset(){
   if(it){
     delete it;
     it = 0;
   }
   pos = 0;
}


RealmChecker::~RealmChecker(){
    hss = 0;
    if(it){
      delete it; 
    }
    tt->DeleteIfAllowed();
}


bool RealmChecker::checkRealm(){
    reset();
    Tuple* t = nextTuple(true);
    if(t){
       t->DeleteIfAllowed();
       return false;
    } else {
       return true;
    }
}



Tuple* RealmChecker::nextTuple(const bool print /* = false */ ){
  while(pos < hss->Size()){
      while(!it){
        hss->Get(pos,currentHs);
        if(currentHs.IsLeftDomPoint()){ 
            it = tree.find(currentHs.BoundingBox());
        } else {
          pos++;
          if(pos>=hss->Size()){
             return 0;
          }
        }
      }
      int const* itpos;
      bool ok;
      HalfSegment hs2;
      while( (itpos = it->next()) != 0) {
         if(*itpos != this->pos){ 
             hss->Get(*itpos,hs2);
             ok = isRealm(currentHs,hs2, print);
             if(!ok){
                return createTuple(this->pos, *itpos, currentHs, hs2); 
             }
         }
      } 
      delete it;
      it = 0;
      pos++; 
   }
   return 0;
}

Tuple* RealmChecker::createTuple(const int pos1, const int pos2, 
                   const HalfSegment& hs1, const HalfSegment& hs2) const{

   Tuple* res = new Tuple(tt);
   res->PutAttribute(0, new CcInt(true,pos1));
   res->PutAttribute(1, new CcInt(true, hs1.attr.partnerno));
   res->PutAttribute(2, getLine(hs1));
   res->PutAttribute(3, new CcInt(true,pos2));
   res->PutAttribute(4, new CcInt(true, hs2.attr.partnerno));
   res->PutAttribute(5, getLine(hs2));
   return res;
}

Line* RealmChecker::getLine( HalfSegment hs){
   Line* result = new Line(2);
   result->StartBulkLoad();
   hs.attr.edgeno = 0;
   hs.SetLeftDomPoint(true);
   (*result) += hs;
   hs.SetLeftDomPoint(false);
   (*result) += hs;
   result->EndBulkLoad();
   return result;
}


ListExpr RealmChecker::getTupleType(){

  ListExpr attr = nl->TwoElemList(
                       nl->SymbolAtom("No1"),
                       listutils::basicSymbol<CcInt>());

  ListExpr res = nl->OneElemList( attr);
  ListExpr last = res;

  attr = nl->TwoElemList(
                       nl->SymbolAtom("Partner1"),
                       listutils::basicSymbol<CcInt>());
  last = nl->Append(last, attr);
  attr = nl->TwoElemList(
                       nl->SymbolAtom("Segment1"),
                       listutils::basicSymbol<Line>());
  last = nl->Append(last, attr);

  attr = nl->TwoElemList(
                       nl->SymbolAtom("No2"),
                       listutils::basicSymbol<CcInt>());
  last = nl->Append(last, attr);
  attr = nl->TwoElemList(
                       nl->SymbolAtom("Partner2"),
                       listutils::basicSymbol<CcInt>());
  last = nl->Append(last, attr);
  attr = nl->TwoElemList(
                       nl->SymbolAtom("Segment2"),
                       listutils::basicSymbol<Line>());
  last = nl->Append(last, attr);
  return  nl->TwoElemList(listutils::basicSymbol<Tuple>(),res); 
}


void intersection(const Line& l1, const Line& l2,  Line& result){

   if(!l1.IsDefined() || !l1.IsDefined()){
      result.SetDefined(false);
      return;
   }
   if(l1.Size() == 0){
      result.CopyFrom(&l2);
      return;
   }
   if(l2.Size() == 0){
      result.CopyFrom(&l1);
      return;
   }
   if(!l1.BoundingBox().Intersects(l2.BoundingBox())){
      result.Clear();
      return;
   }
   // overlapping bounding boxes, we have to compute 
   // insert l1 into an r-tree
   mmrtree::RtreeT<2,int> tree(4,8);
   HalfSegment hs;
   for(int i=0;i<l1.Size();i++){
      l1.Get(i,hs);
      if(hs.IsLeftDomPoint()){
        tree.insert(hs.BoundingBox(),i);
      }
   }
   // iterate over l2, look for common parts in tree
   result.Clear();
   result.SetDefined(true);
   result.StartBulkLoad();
   vector<pair<double,bool> > v;
   int edgeno = 0;
   for(int i=0;i<l2.Size();i++){
      l2.Get(i,hs);
      if(hs.IsLeftDomPoint()){
        mmrtree::RtreeT<2,int>::iterator* it=0;
        it = tree.find(hs.BoundingBox());
        int  const* pos;
        HalfSegment hs2;
        while( (pos = it->next()) != 0){
           l1.Get(*pos,hs2);
           if(onSameLine(hs,hs2)){
              processParallel(hs,hs2,v);
              if(!v.empty()){
                double d1 = v[0].first;
                double d2 = v[1].first;
                v.clear();
                HalfSegment hsr(true,atDelta(hs,d1), atDelta(hs,d2));
                hsr.attr.edgeno = edgeno;
                result += hsr;
                hsr.SetLeftDomPoint(!hsr.IsLeftDomPoint());
                result += hsr;
                edgeno++;       
              }
           } 
        } 
        delete it;
     }
  }
  result.EndBulkLoad( true,false); // sort, no realminize
}

void intersection(const Line& l, const Region& r, Line& result){
   intersection(r,l,result);
}

} // end of namespace robust
