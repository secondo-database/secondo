/*
----
This file is part of SECONDO.

Copyright (C) 2012, University in Hagen, Department of Computer Science,
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


#ifndef REGION_CREATOR_IMPL_H
#define REGION_CREATOR_IMPL_H

#include "RegionCreator.h"


template<template<typename T> class Array>
void RegionCreator<Array>::createRegion(const Array<HalfSegment>* hss, 
                                 RegionT<Array>* result){
   RegionCreator rc(hss, result);
}



template<template<typename T> class Array>
RegionCreator<Array>::RegionCreator(const Array<HalfSegment>* hss, 
                                    RegionT<Array>* result){
   
   const Array<HalfSegment>* realm=0;

   if(false){
     // the following steps can be omitted, if the 
     // halfsegments are 
     // - complete (partners are there and partnerno is set correctly)
     // - realminized (no overlapping or intersecting halfsegments)
     // - sorted (according halfsegment order

     // ensure to have for each halfsegment the corresponding partner
     Array<HalfSegment>* pairs = forcePairsFromLeftDom(hss); 
     // split intersecting halfsegments and remove overlappings
     pairs->Sort(HalfSegmentCompare);
     Array<HalfSegment>* realm = Realminize(*pairs);
     pairs->Destroy();
     delete pairs;
     // sort realminized halfsegments
     realm->Sort(HalfSegmentCompare); 
     // set corresponsing partners
     setPartnerNo(realm);
   } else {
      realm = hss;
   }

   // findcycles within halfSegments
   findCycles(realm);

   detectHoles();

   findCorrespondingOuters();

   setInsideAbove();
   
   buildRegion(result);
   
}

template<template<typename T> class Array>
void RegionCreator<Array>::printCycles() const{
    cout << "found " << cycles.size() << " cycles " << endl;

    for(size_t i=0;i<cycles.size();i++){
      cout << "Cycle No " << i << endl; 
      cout << (holes[i]?"hole cycle":"outer cycle") << endl;
      printCycle(i);
      cout << endl << endl << endl;
    }
}

template<template<typename T> class Array>
void RegionCreator<Array>::printCycle(size_t c) const{
   cout << "cout cycle contains " << cycles[c].size() 
        << " halfSegments" << endl;
   cout << "cycle belongs to outer cycle " << correspondingOuters[c] << endl;
   for(size_t i=0;i<cycles[c].size(); i++){
      if(i>0){
        cout << " -> ";
      }
      cout << cycles[c][i].GetSecPoint();
   }
   cout << " -> " << cycles[c].back().GetDomPoint(); 
}


template<template<typename T> class Array>
void RegionCreator<Array>::detectHoles(){
    holes.clear();
    for(size_t i=0;i<cycles.size();i++){
        holes.push_back(isHole(i));
    }
}

template<template<typename T> class Array>
bool RegionCreator<Array>::isHole(size_t i) const{
    HalfSegment hs = cycles[i][0];
    Point dp = hs.GetDomPoint();
    Point sp = hs.GetSecPoint();
    return isInside((dp.GetX()+sp.GetX())/2,
                    (dp.GetY()+sp.GetY())/2,
                    i);
}

template<template<typename T> class Array>
bool RegionCreator<Array>::isInside(const double x, const double y, 
                             size_t ommit) const{
    size_t count = 0;
    for(size_t c=0;c<cycles.size();c++){
        if(c!=ommit){
           for(size_t h=0; h<cycles[c].size();h++){
              count += intersects(x,y,cycles[c][h]);
           }
        }
    }
    size_t mask = 1;
    return (count & mask) > 0;
}

template<template<typename T> class Array>
size_t RegionCreator<Array>::intersects(const double x, const double y, 
                                 const HalfSegment& hs) {
  // returns 1 if a ray starting at (x,y) horizontaly to left 
  // intersects hs, 0 otherwise
  Point p1 = hs.GetDomPoint();
  Point p2 = hs.GetSecPoint();
  double x1 = p1.GetX();
  double y1 = p1.GetY();
  double x2 = p2.GetX();
  double y2 = p2.GetY();
  double x_min = std::min(x1,x2);
  double x_max = std::max(x1,x2);
  double y_min = std::min(y1,y2);
  double y_max = std::max(y1,y2);

  if(!AlmostEqual(x_min,x) && (x_min > x)){ // segment right of (x,y)
      return 0; 
  } 
  if(!AlmostEqual(y_min,y) && (y_min > y)){ // segment avove (x,y)
      return 0;
  } 
  if(!AlmostEqual(y_max,y) && (y_max < y)) { // segment below (x,y)
      return 0;
  }

 if(x_max >  x){ // intersecting point possible right of x
   if(AlmostEqual(y1,y2)){
      return 0;
   }
   double delta = (y-y1)/(y2-y1);
   double xsec = x1+delta*(x2-x1);
   if(!AlmostEqual(xsec,x) && xsec > x) {
      // intersection point right of (x,y)
      return 0;
   }
 }

 if(AlmostEqual(y,y_min)){
    return 0;
 } 
 return 1; 
}

/*
~findCorrespondingOuters~

After the cycles are detected and the kind of each cycle (hole, outercycle) is
stored, this function can be used to assign each cycle to an outer cycle.

*/
template<template<typename T> class Array>
  void RegionCreator<Array>::findCorrespondingOuters(){
    correspondingOuters.clear();
    for(size_t i=0;i<cycles.size();i++){
       // find left most point within cycle
       // this is required to avoid finding of isles
       Point p = cycles[i][0].GetDomPoint();
       double x = p.GetX();
       int index = 0;
       for(size_t h=1;h<cycles[i].size();h++){
          Point p1 = cycles[i][h].GetDomPoint();
          double x1 = p1.GetX();
          if(x1 < x){
              p = p1;
              x = x1;
              index = h;
          }
       }
       leftMostPoints.push_back(index);
       if(!holes[i]){ // outer cycle corresponds to itself
          correspondingOuters.push_back(i);
       } else {
           correspondingOuters.push_back(findLeftNearestOuter(p.GetX(),
                                                              p.GetY()));
       }
    }
  }

template<template<typename T> class Array>
  int RegionCreator<Array>::findLeftNearestOuter(const double x, 
                                                 const double y) const{
     int index = -1;
     double dist = 0;
     for(size_t i=0; i< cycles.size();i++){
        if(!holes[i]){
            double dist1 = getLeftDist(i,x,y);
            if(dist1>=0){ // there is an intersection
              if( (index < 0) || (dist1 < dist)) {
                 index = i;
                 dist = dist1;
              }
            }
        }
     }
     if(index < 0){
       cout << "internal error, found index < 0 in " 
            << __FILE__ << "@" << __LINE__ << endl
            << "in function" << __PRETTY_FUNCTION__ << endl;
       throw std::string("invalid index");

     }
     return index;
  }

  /*
   ~getLeftDist~

   Treats all segments of the cycle with given cycle number.
   It constructs a ray starting at (x,y) going horizontally to left.
   From all intersection points of the ray with the segments, it return the
   minimum distance. If no intersection point exists, -1 is returned. 

  */
template<template<typename T> class Array>
  double RegionCreator<Array>::getLeftDist(const int cycle, 
                                    const double x, const double y) const{
     double dist = -1;
     for(size_t i=0;i<cycles[cycle].size();i++){
        double dist1 = getLeftDist(cycles[cycle][i],x,y);
        if(dist1>=0){
           if((dist < 0) || dist1<dist){
             dist = dist1;
           }
        }  
     }
     return dist;
  }


  /*
   Checks whether hs intersects a ray starting at (x,y) going horizontally 
   to left.
   If no such intersection exists, -1 is returned, other wise the distance 
   between the intersection point and (x,y).

  */
template<template<typename T> class Array>
  double RegionCreator<Array>::getLeftDist(const HalfSegment& hs, 
                            const double x, const double y, const bool move){


     double x1 = hs.GetDomPoint().GetX();
     double y1 = hs.GetDomPoint().GetY();
     double x2 = hs.GetSecPoint().GetX();
     double y2 = hs.GetSecPoint().GetY();
     double xmin = std::min(x1,x2);
     if(!AlmostEqual(xmin,x) && (xmin > x)){ //segment completely right of (x,y)
       return -1;
     }
     double xmax = std::max(x1,x2);
     double ymin = std::min(y1,y2);
     if(!AlmostEqual(ymin,y) && (ymin > y)){ // segment above (x,y)
       return -1;
     }
     double ymax = std::max(y1,y2);
     if(!AlmostEqual(ymax,y) && (ymax < y)) { // segment below (x,y)
       return -1;
     }
     if(move && AlmostEqual(ymin,y)){
       return -1;
     }
     if(AlmostEqual(y1,y2)){  // horizontal segment
        return std::abs(x - xmax); // abs is for rounding errors
     } 
    
     double delta = (y-y1)/(y2-y1);
     double xsec = x1 + delta*(x2-x1);
     if(AlmostEqual(x,xsec)){
        // cout << "hit" << endl;
        return 0;
     }
     if(xsec > x){
       // cout << "cutpoint is right" << endl;
       return -1;
     }      
     // cout << " interection, dist = " << x-xsec << endl;
     return x - xsec;
  }


template<template<typename T> class Array>
  double RegionCreator<Array>::getRightDist(const HalfSegment& hs, 
                            const double x, const double y){

     //cout << "check " << hs.SimpleString() << endl;
     //cout << "with Point ("  << x << ", " << y << ")" << endl;

     double x1 = hs.GetDomPoint().GetX();
     double y1 = hs.GetDomPoint().GetY();
     double x2 = hs.GetSecPoint().GetX();
     double y2 = hs.GetSecPoint().GetY();
     double xmax = std::max(x1,x2);
     if(!AlmostEqual(xmax,x) && (xmax <  x)){ //segment completely left of (x,y)
       return -1;
     }
     double xmin = std::min(x1,x2);
     double ymin = std::min(y1,y2);
     if(!AlmostEqual(ymin,y) && (ymin > y)){ // segment above (x,y)
       return -1;
     }
     double ymax = std::max(y1,y2);
     if(!AlmostEqual(ymax,y) && (ymax < y)) { // segment below (x,y)
       return -1;
     }
     if(AlmostEqual(y1,y2)){  // horizontal segment
        // cout << "horizontal" << endl;
        return std::abs(xmin - x); // abs is for rounding errors
     } 

     double delta = (y-y1)/(y2-y1);
     double xsec = x1 + delta*(x2-x1);
     if(AlmostEqual(x,xsec) && xsec < x){
        return 0;
     }
     if(xsec < x){
       // cout << "cutpoint is left" << endl;
       return -1;
     }      
     // cout << " interection, dist = " << x-xsec << endl;
     return  xsec - x;
  }
/*
Returns the distance from (x,y) to hs following a vertical 
ray in upper direction. If the ray does not intersects the halfsegment,
-1 is returned.

*/
template<template<typename T> class Array>
  double RegionCreator<Array>::getUpDist(const HalfSegment& hs, 
                            const double x, const double y){

     double x1 = hs.GetDomPoint().GetX();
     double y1 = hs.GetDomPoint().GetY();
     double x2 = hs.GetSecPoint().GetX();
     double y2 = hs.GetSecPoint().GetY();
     double ymax = std::max(y1,y2);
     
     if(!AlmostEqual(ymax,y) && (ymax < y)){ //segment completely under (x,y)
       //cout << "under" << endl;
       return -1;
     }
     
     double xmin=std::min(x1,x2);
     if(!AlmostEqual(xmin,x) && xmin>x){
       // right of the ray
       return -1;
     }
     double xmax = std::max(x1,x2);
     if(!AlmostEqual(xmax,x) && xmax < x){
        // left of the ray
        return -1; 
     }

     double ymin = std::min(y1,y2);
     if(AlmostEqual(x1,x2)){ // vertical segment
        if(ymin<y){
           return 0;
        }
        return (ymin-y);
     }

     double delta = (x -x1) / (x2-x1);
     double ycut = y1 + delta*(y2-y1);

     if(!AlmostEqual(ycut,y) && ycut < y){
        return -1;
     }
     if(AlmostEqual(ycut,y)){
         return 0;
     }
     return std::abs(ycut-y) ;
  }

template<template<typename T> class Array>
  double RegionCreator<Array>::getDownDist(const HalfSegment& hs, 
                            const double x, const double y){

     double x1 = hs.GetDomPoint().GetX();
     double y1 = hs.GetDomPoint().GetY();
     double x2 = hs.GetSecPoint().GetX();
     double y2 = hs.GetSecPoint().GetY();
     double ymin = std::min(y1,y2);
     
     if(!AlmostEqual(ymin,y) && (ymin  > y)){ //segment completely above (x,y)
       //cout << "above" << endl;
       return -1;
     }
     
     double xmin=std::min(x1,x2);
     if(!AlmostEqual(xmin,x) && xmin>x){
       // right of the ray
       return -1;
     }
     double xmax = std::max(x1,x2);
     if(!AlmostEqual(xmax,x) && xmax < x){
        // left of the ray
        return -1; 
     }

     double ymax = std::max(y1,y2);
     if(AlmostEqual(x1,x2)){ // vertical segment
        if(ymax>y){
           return 0;
        }
        return (y - ymax);
     }

     double delta = (x -x1) / (x2-x1);
     double ycut = y1 + delta*(y2-y1);

     if(!AlmostEqual(ycut,y) && ycut > y){ // above
        return -1;
     }
     if(AlmostEqual(ycut,y)){ // nearly hit
         return 0;
     }
     return std::abs(y-ycut) ;
  }




/*
~forcePairs~

Creates for each HalfSegment having a left dominating point two entries 
within the result array

*/
template<template<typename T> class Array>
Array<HalfSegment>* 
   RegionCreator<Array>::forcePairsFromLeftDom(const Array<HalfSegment>* src){
   Array<HalfSegment>* res = new Array<HalfSegment>(src->Size());
   HalfSegment hs;
   int edgeno = 0;
   for(int i=0;i<src->Size();i++){
      src->Get(i,hs);
      if(hs.IsLeftDomPoint()){
         hs.attr.edgeno = edgeno;
         res->Append(hs);
         hs.SetLeftDomPoint(false);
         res->Append(hs);
      }
   }
   return res;
}


template<template<typename T> class Array>
void RegionCreator<Array>::setPartnerNo(Array<HalfSegment>* hss){
  int size = hss->Size();
  int* tmp = new int[size/2];

  for(int i=0; i<size/2; i++){
    tmp[i] = -1;
  }
  HalfSegment hs;
  for( int i = 0; i < size; i++ ) 
  {
    hss->Get( i, hs ); 
    if(tmp[hs.attr.edgeno] == -1 )
    {
      tmp[hs.attr.edgeno] = i;
    }
    else  
    {
      int p = tmp[hs.attr.edgeno];
      HalfSegment hs1( hs ); 
      hs1.attr.partnerno = p;
      hss->Put( i, hs1 );
      hss->Get( p, hs ); 
      hs1 = hs; 
      hs1.attr.partnerno = i;
      hss->Put( p, hs1 );
    }
  }
  delete[] tmp;
}


/*
~findCycles~

find minimal cycles within a sorted dbarray

*/
template<template<typename T> class Array>
   void RegionCreator<Array>::findCycles(const Array<HalfSegment>* hss) {
      cycles.clear();
      int size = hss->Size();
      if(size < 3){
         return; 
      }
      char* usage = new char[size];
      memset(usage,0,size*sizeof(char));
      char* critical = new char[size];
      findCritical(hss,critical);
      int pos = 0;
      while(pos < size){
         if(usage[pos]==0){ // unused halfsegment
            HalfSegment hs;
            hss->Get(pos,hs);
            if(hs.IsLeftDomPoint()){
               findCycle(hss, pos, usage, critical);
            } else {
               pos++;
            }
         } else {
            pos++;
         }
      }
      delete[] usage;
      delete[] critical;
   }

template<template<typename T> class Array>
   void RegionCreator<Array>::findCycle(const Array<HalfSegment>* hss, 
                                 int pos, char* usage, const char* critical){
      pos = getStartPos(hss,pos,usage);  
      if( pos <  0 ) {
        return;
      }
      // extend path until (sub) path is closed or dead end is found
      std::vector<int> path;
      path.push_back(pos);
      HalfSegment hs;
      usage[pos] = 3; // part of current path
      hss->Get(pos,hs);
      usage[hs.attr.partnerno] = 4; 
      int next = getNext(hss,pos,usage);

      while(!path.empty()) {
          if(next < 0){
             // no extension found => mark segments in path as non-cycle
             // go back until the next critical point is reachedA
             bool done = path.empty();
             while(!done){
               if(path.empty()){
                 done = true;
               } else {
                 pos = path.back();
                 path.pop_back();
                 usage[pos] = 1;
                 hss->Get(pos,hs);
                 usage[hs.attr.partnerno] = 1;

                 if(critical[pos]){
                   done = true;
                 } else {
                 }
               }
             }
             if(!path.empty()){
               pos = path.back();
               next = getNext(hss,pos,usage);
             } else {
             }
          } else {
            if(usage[next] == 3){
                // (sub) path found
                int p = path.back();
                path.pop_back();
                cycles.push_back(std::vector<HalfSegment>());
                while(p!=next && !path.empty()){
                  usage[p] = 2;
                  hss->Get(p,hs);
                  cycles.back().push_back(hs);
                  usage[hs.attr.partnerno] = 2;
                  p = path.back();
                  path.pop_back();
                }
                usage[p] = 2;
                hss->Get(p,hs);
                usage[hs.attr.partnerno] = 2;
                cycles.back().push_back(hs);
                if(!path.empty()){
                  pos = path.back(); // try to extend first part of path
                  next = getNext(hss,pos,usage);
                }
            }  else { // normal extension
               pos = next;
               path.push_back(pos);
               usage[pos] = 3;
               hss->Get(pos,hs);
               usage[hs.attr.partnerno] = 4;
               next = getNext(hss,pos,usage);
            }
          }
        } // while path not empty 
   }


/*
~getStartPos~

Determines the position of an halfsegment within hss having the 
following properties:

- it's usage is 0 (unused)
- it has the same dominating point as the halfsegment stored as position posA
- The dominating point is the left one
- from all these halfsegments, it has the maximum slope 

*/
template<template<typename T> class Array>
   int RegionCreator<Array>::getStartPos(const Array<HalfSegment>* hss, 
                                  int pos, char* usage){
      std::vector<std::pair<HalfSegment,int> > candidates;
      // stores all unused halfsegments with same dompoint 
      HalfSegment hs;
      hss->Get(pos, hs);
      if((usage[pos]==0) && hs.IsLeftDomPoint()){
         candidates.push_back(std::pair<HalfSegment,int>(hs,pos));
      }

      Point dp = hs.GetDomPoint();
      int pos1 = pos-1;
      bool done = false;
      while(pos1>0 && !done){
          hss->Get(pos1,hs);
          Point dp1 = hs.GetDomPoint();
          if(!AlmostEqual(dp1,dp)){
            done = true;
          } else {
            if( (usage[pos1]==0) && (hs.IsLeftDomPoint())){
               candidates.push_back(std::pair<HalfSegment,int>(hs,pos1));
            }
          }
          pos1--;
      }
      pos1 = pos+1;
      done = false;
      while(pos1<hss->Size() && !done){
          hss->Get(pos1,hs);
          Point dp1 = hs.GetDomPoint();
          if(!AlmostEqual(dp1,dp)){
            done = true;
          } else {
            if( (usage[pos1]==0) && (hs.IsLeftDomPoint())){
               candidates.push_back(std::pair<HalfSegment,int>(hs,pos1));
            }
          }
          pos1++;
      }
      if(candidates.empty()){ // no candidate found
         return -1; 
      }
      int index = -1;
      double slope = 0;
      for(size_t i = 0; i<candidates.size();i++){
         HalfSegment hs = candidates[i].first;
         Point dp = hs.GetDomPoint();
         Point sp = hs.GetSecPoint();
         if(AlmostEqual(dp.GetX(),sp.GetX())){ // vertical segment
            assert(dp.GetY() < sp.GetY()); // otherwise IsLeftDomPoint 
                                           // should have returned false
            // we will not find an halfsegment with higher slope
            return candidates[i].second;
         } else {
           double slope1 = (sp.GetY()-dp.GetY()) / (sp.GetX() - dp.GetX());
           if((index < 0) || slope<slope1){
              index = i;
              slope = slope1;
           }
         }
      }
      assert(index >= 0);
      return candidates[index].second;
   }



/*
~findCritical~

Marks the halfsegments of hss as

- 0 : dompoint is a connection between two segments
- 1 : dompoint is a dead end
- 2 : dompoint connect more than 2 segments (branch)

*/

template<template<typename T> class Array>
   void RegionCreator<Array>::findCritical(const Array<HalfSegment>* hss, 
                                    char* critical){

     memset(critical,0,hss->Size()); // initialize with 0
     int count = 0;
     HalfSegment hs;
     Point lastPoint(true);

     for(int i=0;i<hss->Size();i++){
        hss->Get(i,hs);
        Point currentPoint = hs.GetDomPoint();
        if(i==0){ // start new sequence
          lastPoint = currentPoint;
          count = 1;
        } else {
           if(AlmostEqual(lastPoint,currentPoint)){ // extend sequence
              count++;
           } else {
             if(count != 2){
                int mark = count==1?1:2;
                for(int r=1;r<=count;r++){
                   critical[i-r] = mark;
                }
             }
             lastPoint = currentPoint;
             count = 1;
           }
        }
     }
     if(count != 2){
        int mark = count ==1?1:2;
        for(int r=1;r<=count;r++){
           critical[hss->Size()-r] = mark;
        }
     }
   }

template<template<typename T> class Array>
int RegionCreator<Array>::getNext(const Array<HalfSegment>* hss, int pos, 
                           const char* usage) {
      HalfSegment hs;
      hss->Get(pos,hs);
      
      int ppos = hs.attr.partnerno;
      std::vector<std::pair<HalfSegment,int> > candidates;
      Point pdp = hs.GetSecPoint();

      //assert(usage[ppos] == 4);
      int ppos1 = ppos-1;
      bool done = false;
      HalfSegment hs1;
      while(ppos1>=0 && !done){
         hss->Get(ppos1,hs1);
         Point dp1 = hs1.GetDomPoint();
         if(!AlmostEqual(dp1,pdp)){
           done = true;
         } else {
           if(usage[ppos1] == 3){
              return ppos1;
           } else if(usage[ppos1]==0){
              candidates.push_back(std::pair<HalfSegment,int>(hs1,ppos1));
           }
         }
         ppos1--;
      }
      ppos1 = ppos+1;
      done = false;
      while(ppos1<hss->Size() && !done){
         hss->Get(ppos1,hs1);
         Point dp1 = hs1.GetDomPoint();
         if(!AlmostEqual(dp1,pdp)){
           done = true;
         } else {
           if(usage[ppos1] == 3){
              return ppos1;
           } else if(usage[ppos1]==0){
              candidates.push_back(std::pair<HalfSegment,int>(hs1,ppos1));
           }
         }
         ppos1++;
      }

      if(candidates.empty()){ // no extension possible
        return -1;
      }


      if(candidates.size()==1){ // simple extension, no branch
        return candidates[0].second;
      }

      //partition candidates to those ones left of hs and those ones right of hs
      std::vector<std::pair<HalfSegment,int> > candidates_left;
      std::vector<std::pair<HalfSegment,int> > candidates_right;
      Point dp = hs.GetDomPoint();
      Point sp = hs.GetSecPoint();
      for(unsigned int i=0;i<candidates.size();i++){
         Point p = candidates[i].first.GetSecPoint();
         if(isRight(dp,sp,p)){
            candidates_right.push_back(candidates[i]);
         } else {
            candidates_left.push_back(candidates[i]);
         }
      }


      candidates = candidates_right.size()>0?candidates_right:candidates_left;
      // search for the right most extension
      int index = 0;
      for(unsigned int i=1;i<candidates.size() ; i++){
         if(moreRight(candidates[index].first, candidates[i].first)){
            index = i;
         }
      }

      return candidates[index].second;
   }


template<template<typename T> class Array>
  bool RegionCreator<Array>::isRight(const Point& p, const Point& q, 
                                     const Point& r) {


    double rx=r.GetX();
    double ry=r.GetY();
    double px=p.GetX();
    double py=p.GetY();
    double qx=q.GetX();
    double qy=q.GetY();
    
    double A2 = px*qy + py*rx + qx*ry - (rx*qy + ry*px +qx*py);



    bool res =  A2 < 0;  
    return res;

  }


template<template<typename T> class Array>
  bool RegionCreator<Array>::moreRight(const HalfSegment& hs1, 
                                const HalfSegment& hs2) {
     Point dp1 = hs1.GetDomPoint();
     Point dp2 = hs2.GetDomPoint();
     assert(AlmostEqual(dp1,dp2));
     Point sp1 = hs1.GetSecPoint();
     Point sp2 = hs2.GetSecPoint();
     return isRight(dp1,sp1,sp2);
  }

template<template<typename T> class Array>
  void RegionCreator<Array>::setInsideAbove(){
     for(size_t i=0;i<cycles.size(); i++){
        setInsideAbove(i);
     }
  }

template<template<typename T> class Array>
  void RegionCreator<Array>::setInsideAbove(const int i){
     int leftMost = leftMostPoints[i];
     int size = cycles[i].size();
     int pos1 = (leftMost + size - 1) % size;
     int pos2 = (leftMost + 1) % size;
     Point p1 = cycles[i][pos1].GetDomPoint();
     Point p2 = cycles[i][leftMost].GetDomPoint();
     Point p3 = cycles[i][pos2].GetDomPoint();
     bool clockwise = RegionT<Array>::GetCycleDirection(p1,p2,p3);
     bool isRight = holes[i]?!clockwise:clockwise;
     for(int h=0;h<size;h++){
        if(isRight){
           cycles[i][h].attr.insideAbove = ( cycles[i][h].IsLeftDomPoint()); 
        } else {
          cycles[i][h].attr.insideAbove = ( !cycles[i][h].IsLeftDomPoint()); 
        }
     }
  }

template<template<typename T> class Array>
  void RegionCreator<Array>::buildRegion(RegionT<Array>* result) const{
     result->Clear();
     result->SetDefined(true);
     result->StartBulkLoad();
     int faceno = 0;
     int edgeno = 0;
     for(size_t i=0; i< cycles.size(); i++){
         if(!holes[i]) {
             saveFace(i, faceno, edgeno, result);  
             faceno++;
         }
     }  
     result->SetNoComponents(faceno);
     result->EndBulkLoad(true, true, true, false); //sort,coverageNo,Partnerno
  }

template<template<typename T> class Array>
  void RegionCreator<Array>::saveFace(const int cycle, const int faceno,  
                               int& edgeno, RegionT<Array>* result) const{
     int cycleno = 0;
     if(!saveCycle(cycle, faceno, cycleno, edgeno, result)){
         return;
     }
     cycleno++;
     for(size_t i=0;i<cycles.size();i++){
         if((correspondingOuters[i]==cycle) && holes[i]){
             if(saveCycle(i,faceno, cycleno,edgeno, result)){
               cycleno++;
             }
         }
     }
  }

template<template<typename T> class Array>
  bool RegionCreator<Array>::saveCycle(const int cycle, const int faceno, 
                                const int cycleno, int& edgeno, 
                                RegionT<Array>* result)const{

      if(cycles[cycle].size() < 3){
          std::cerr << "found cycle with less than 3 halfsegments -> ignore"
                    << endl;
          return false;
      }

      for(size_t i=0;i<cycles[cycle].size();i++){
          HalfSegment hs = cycles[cycle][i];
          hs.attr.faceno = faceno;
          hs.attr.cycleno = cycleno;
          hs.attr.edgeno = edgeno;
          (*result) += hs;
          hs.SetLeftDomPoint(!hs.IsLeftDomPoint());
          (*result) += hs;
          edgeno++;
      }
      return true;
  }

// Instantiations
// template class RegionCreator<DbArray>;
// template class RegionCreator<MMDbArray>;

#endif
