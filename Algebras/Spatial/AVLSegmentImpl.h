
/*
----
This file is part of SECONDO.

Copyright (C) 2017, 
University in Hagen, 
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

//[_] [\_]

*/




#ifndef AVLSEGMENTIMPL_H
#define AVLSEGMENTIMPL_H

#include "RobustSetOps.h"
#include "AvlTree.h"
#include "Tools/Flob/DbArray.h"


/*
1 Implementation of class AVLSegment.

*/


class XRemover : public unary_functor<avlseg::AVLSegment, bool>{
  public:
    XRemover(): value(0.0) {}
    XRemover(const XRemover& src): value(src.value) {}
    XRemover& operator=(const XRemover& src){
      value = src.value;
      return *this;
    }
    void setX(double x){
       value = x;
    }
    bool operator()(const avlseg::AVLSegment& s) const{
       double x2 = s.getX2();
       return !AlmostEqual(value,x2) && x2 < value;
    }

  private:
    double value;
};


void insertEvents(const avlseg::AVLSegment& seg,
                  const bool createLeft,
                  const bool createRight,
                  std::priority_queue<avlseg::ExtendedHalfSegment,
                                std::vector<avlseg::ExtendedHalfSegment>,
                                std::greater<avlseg::ExtendedHalfSegment> >& q1,
                  std::priority_queue<avlseg::ExtendedHalfSegment,
                                 std::vector<avlseg::ExtendedHalfSegment>,
                               std::greater<avlseg::ExtendedHalfSegment> >& q2);


bool splitByNeighbour(avltree::AVLTree<avlseg::AVLSegment>& sss,
                      avlseg::AVLSegment& current,
                      avlseg::AVLSegment *& neighbour,
                      std::priority_queue<avlseg::ExtendedHalfSegment,
                                std::vector<avlseg::ExtendedHalfSegment>,
                                std::greater<avlseg::ExtendedHalfSegment> >& q1,
                      std::priority_queue<avlseg::ExtendedHalfSegment,
                                 std::vector<avlseg::ExtendedHalfSegment>,
                               std::greater<avlseg::ExtendedHalfSegment> >& q2,
                     const bool forceThrow );


void splitNeighbours(avltree::AVLTree<avlseg::AVLSegment>& sss,
                     avlseg::AVLSegment *& leftN,
                     avlseg::AVLSegment *& rightN,
                     std::priority_queue<avlseg::ExtendedHalfSegment,
                                std::vector<avlseg::ExtendedHalfSegment>,
                                std::greater<avlseg::ExtendedHalfSegment> >& q1,
                     std::priority_queue<avlseg::ExtendedHalfSegment,
                                    std::vector<avlseg::ExtendedHalfSegment>,
                                std::greater<avlseg::ExtendedHalfSegment> >& q2,
                     const bool forceThrow);

/*
 checks whether the dominating points within the halfsegments are sorted
 correctly.

*/
template<class Array>
bool checkOrder(const Array& segs, bool exact){
  if(segs.Size() < 2){
    return true;
  }
  Point p1(false);
  Point p2(false);
  HalfSegment hs1;
  HalfSegment hs2;

  segs.Get(0,hs1);
  p1 = hs1.GetDomPoint();

  for(int i=1;i<segs.Size();i++){
     segs.Get(i,hs2);
     p2 = hs2.GetDomPoint();

     double x1 = p1.GetX();
     double y1 = p1.GetY();
     double x2 = p2.GetX();
     double y2 = p2.GetY();
     if(exact){
        if(x2<x1){
          return false;
        }
        if( (x1==x2) && (y2<y1)){
          return false;
        }

     } else {
       if(AlmostEqual(x1,x2)){
         if(!AlmostEqual(y1,y2) && (y2<y1)){
            return false;
         }
       } else {
         if(x2<x1){
            return false;
         }
       }
     }
     p1 = p2;

  }
  return true;
}


/*

~selectNext~

Selects the minimum halfsegment from ~v~1, ~v~2, ~q~1, and ~q~2.
If no values are available, the return value will be __none__.
In this case, __result__ remains unchanged. Otherwise, __result__
is set to the minimum value found. In this case, the return value
will be ~first~ or ~second~.
If some halfsegments are equal, the one
from  ~v~1 is selected.
Note: ~pos~1 and ~pos~2 are increased automatically. In the same way,
      the topmost element of the selected queue is deleted.

The template parameter can be instantiated with ~Region~ or ~Line~

*/
template<class T1, class T2>
avlseg::ownertype selectNext(const T1& v1,
                     int& pos1,
                     const T2& v2,
                     int& pos2,
                     std::priority_queue<avlseg::ExtendedHalfSegment,
                                std::vector<avlseg::ExtendedHalfSegment>,
                                std::greater<avlseg::ExtendedHalfSegment> >& q1,
                     std::priority_queue<avlseg::ExtendedHalfSegment,
                                std::vector<avlseg::ExtendedHalfSegment>,
                                std::greater<avlseg::ExtendedHalfSegment> >& q2,
                     avlseg::ExtendedHalfSegment& result,
                     int src = 0
                    ){

  const avlseg::ExtendedHalfSegment* values[4];
  HalfSegment hs0hs, hs2hs;
  avlseg::ExtendedHalfSegment hs0, hs1, hs2, hs3;
  int number = 0; // number of available values
  // read the available elements
  if(pos1<v1.Size()){
     v1.Get(pos1,hs0hs);
     hs0.CopyFrom(hs0hs);
     values[0] = &hs0;
     number++;
  }  else {
     values[0]=0;
  }
  if(q1.empty()){
    values[1] = 0;
  } else {
    values[1] = &q1.top();
    number++;
  }
  if(pos2<v2.Size()){
     v2.Get(pos2,hs2hs);
     hs2.CopyFrom(hs2hs);
     values[2] = &hs2;
     number++;
  }  else {
     values[2] = 0;
  }
  if(q2.empty()){
    values[3]=0;
  } else {
    values[3] = &q2.top();
    number++;
  }
  // no halfsegments found

  if(number == 0){
     return avlseg::none;
  }
  // search for the minimum.
  int index = -1;
  for(int i=0;i<4;i++){
      if(values[i] && !values[i]->isInitialized()){
          std::cerr << "selectNext returns an undefined extened HalfSegment"
                    << std::endl;
          std::cerr << "the index is " << i << std::endl;
          values[i]->Print(std::cerr) << std::endl;
          assert(values[i]->isInitialized());
       }
    if(values[i]){
       if(index<0 || (*values[index] > *values[i])){
          index = i;
       }
    }
  }
  result = *values[index];


  src = index +  1;
  switch(index){
    case 0: pos1++; return avlseg::first;
    case 1: q1.pop();  return avlseg::first;
    case 2: pos2++;  return avlseg::second;
    case 3: q2.pop();  return avlseg::second;
    default: assert(false);
  }
  return avlseg::none;
}



template<template<typename T> class Array>
avlseg::ownertype selectNext(const RegionT<Array>& reg1,
                     int& pos1,
                     const RegionT<Array>& reg2,
                     int& pos2,
                     std::priority_queue<avlseg::ExtendedHalfSegment,
                                std::vector<avlseg::ExtendedHalfSegment>,
                                std::greater<avlseg::ExtendedHalfSegment> >& q1,
                     std::priority_queue<avlseg::ExtendedHalfSegment,
                                std::vector<avlseg::ExtendedHalfSegment>,
                                std::greater<avlseg::ExtendedHalfSegment> >& q2,
                     avlseg::ExtendedHalfSegment& result,
                     int& src // for debugging only
                    ){
   return selectNext<RegionT<Array>,RegionT<Array> >(reg1,pos1,reg2,pos2,q1,
                                                     q2,result,src);
}


template
avlseg::ownertype selectNext<DbArray>(const RegionT<DbArray>& reg1,
                     int& pos1,
                     const RegionT<DbArray>& reg2,
                     int& pos2,
                     std::priority_queue<avlseg::ExtendedHalfSegment,
                                std::vector<avlseg::ExtendedHalfSegment>,
                                std::greater<avlseg::ExtendedHalfSegment> >& q1,
                     std::priority_queue<avlseg::ExtendedHalfSegment,
                                std::vector<avlseg::ExtendedHalfSegment>,
                                std::greater<avlseg::ExtendedHalfSegment> >& q2,
                     avlseg::ExtendedHalfSegment& result,
                     int& src // for debugging only
                    );


/*
7.8 ~line~ [x] ~line~


Instantiation of the ~selectNext~ function.

*/
template<template<typename T> class Array>
avlseg::ownertype selectNext(const LineT<Array>& line1,
                     int& pos1,
                     const LineT<Array>& line2,
                     int& pos2,
                     std::priority_queue<avlseg::ExtendedHalfSegment,
                                std::vector<avlseg::ExtendedHalfSegment>,
                                std::greater<avlseg::ExtendedHalfSegment> >& q1,
                     std::priority_queue<avlseg::ExtendedHalfSegment,
                                std::vector<avlseg::ExtendedHalfSegment>,
                                std::greater<avlseg::ExtendedHalfSegment> >& q2,
                     avlseg::ExtendedHalfSegment& result,
                     int& src
                    ){

   return selectNext<LineT<Array>,LineT<Array> >(line1,pos1,line2,
                                                 pos2,q1,q2,result, src);
}

/*
7.9 ~line~ [x] ~region~


~selectNext~

Instantiation of the ~selectNext~ function for ~line~ [x] ~region~.

*/
template<template<typename T> class Array>
avlseg::ownertype selectNext(const LineT<Array>& line,
                     int& pos1,
                     const RegionT<Array>& region,
                     int& pos2,
                     std::priority_queue<avlseg::ExtendedHalfSegment,
                                std::vector<avlseg::ExtendedHalfSegment>,
                                std::greater<avlseg::ExtendedHalfSegment> >& q1,
                     std::priority_queue<avlseg::ExtendedHalfSegment,
                                std::vector<avlseg::ExtendedHalfSegment>,
                                std::greater<avlseg::ExtendedHalfSegment> >& q2,
                     avlseg::ExtendedHalfSegment& result,
                     int& src
                    ){

   return selectNext<LineT<Array>,RegionT<Array> >(line,pos1,region,pos2,
                                                   q1,q2,result,src);
}



/*
~SelectNext~ line [x] point

This function looks which event from the line or from the point
is smaller. The line is divided into the original line part
at position ~posLine~ and possible splitted segments stored
in ~q~. The return value of the function will be ~first~ if the
next event comes from the line value and ~second~ if the next
event comes from the point value. Depending of the return value,
one of the arguments ~resHs~ or ~resPoint~ is set the the value of
this event.
The positions are increased automatically by this function.

*/
template<template<typename T> class Array>
avlseg::ownertype selectNext( const LineT<Array>& line,
                      std::priority_queue<avlseg::ExtendedHalfSegment,
                                 std::vector<avlseg::ExtendedHalfSegment>,
                                 std::greater<avlseg::ExtendedHalfSegment> >& q,
                      int& posLine,
                      const Point&  point,
                      int& posPoint, // >0: point already used
                      avlseg::ExtendedHalfSegment& resHs,
                      Point& resPoint){


   int size = line.Size();
   avlseg::ExtendedHalfSegment hsl;
   bool hs1exists = false;
   avlseg::ExtendedHalfSegment hsq;
   bool hsqexists = false;
   avlseg::ExtendedHalfSegment hsmin;
   bool hsminexists = false;
   avlseg::ExtendedHalfSegment hstmp;

   int src = 0;
   if(posLine < size){
      HalfSegment hsl_transfer;
      line.Get(posLine,hsl_transfer);
      hsl = hsl_transfer;
      hs1exists = true;
   }
   if(!q.empty()){
       hstmp = q.top();
       hsq = hstmp;
       hsqexists = true;
   }
   if(hs1exists){
      src = 1;
      hsmin = hsl;
      hsminexists = true;
   }
   if(hsqexists){
     if(!hs1exists || (hsq < hsl)){
       src = 2;
       hsmin = hsq;
       hsminexists = true;
     }
   }

   if(posPoint==0){  // point not already used
     if(!hsminexists){
       src = 3;
     } else {
       Point p = hsmin.GetDomPoint();
       if(point < p){
            src = 3;
        }
     }
   }

   switch(src){
    case 0: return avlseg::none;
    case 1: posLine++;
            resHs = hsmin;
            return avlseg::first;
    case 2: q.pop();
            resHs = hsmin;
            return avlseg::first;
    case 3: resPoint = point;
            posPoint++;
            return avlseg::second;
    default: assert(false);
             return avlseg::none;
   }
}


/*
~selectNext~ line [x] points

This function works like the function above but instead for a point, a
points value is used.

*/

template<template<typename T> class Array>
avlseg::ownertype selectNext(const LineT<Array>& line,
                      std::priority_queue<avlseg::ExtendedHalfSegment,
                                 std::vector<avlseg::ExtendedHalfSegment>,
                                 std::greater<avlseg::ExtendedHalfSegment> >& q,
                      int& posLine,
                      const PointsT<Array>& point,
                      int& posPoint,
                      avlseg::ExtendedHalfSegment& resHs,
                      Point& resPoint){

   int sizeP = point.Size();
   int sizeL = line.Size();


   avlseg::ExtendedHalfSegment hsl;
   bool hslexists = false;
   avlseg::ExtendedHalfSegment hsq;
   bool hsqexists = false;
   avlseg::ExtendedHalfSegment hsmin;
   bool hsminexists = false;
   avlseg::ExtendedHalfSegment hstmp;
   int src = 0;
   if(posLine < sizeL){
      HalfSegment hsl_transfer;
      line.Get(posLine,hsl_transfer);
      hsl = hsl_transfer;
      hslexists = true;
   }
   if(!q.empty()){
       hstmp = q.top();
       hsq = hstmp;
       hsqexists = true;
   }
   if(hslexists){
      src = 1;
      hsmin = hsl;
      hsminexists = true;
   }
   if(hsqexists){
     if(!hslexists || (hsq < hsl)){
       src = 2;
       hsmin = hsq;
       hsminexists = true;
     }
   }

   Point  cp;
   if(posPoint<sizeP){  // point not already used
     point.Get(posPoint,cp);
     if(!hsminexists){
       src = 3;
     } else {
       Point p = hsmin.GetDomPoint();
       if(cp < p){
            src = 3;
        }
     }
   }

   switch(src){
    case 0: return avlseg::none;
    case 1: posLine++;
            resHs = hsmin;
            return avlseg::first;
    case 2: q.pop();
            resHs = hsmin;
            return avlseg::first;
    case 3: resPoint = cp;
            posPoint++;
            return avlseg::second;
    default: assert(false);
             return avlseg::none;
   }

}


/*
8 ~Realminize~

This function converts  a line given as ~src~ into a realminized version
stored in ~result~.

*/
template<template<typename T> class Array>
avlseg::ownertype selectNext(const LineT<Array>& src, int& pos,
                            std::priority_queue<avlseg::ExtendedHalfSegment,
                                 std::vector<avlseg::ExtendedHalfSegment>,
                                 std::greater<avlseg::ExtendedHalfSegment> >& q,
                            avlseg::ExtendedHalfSegment& result){

 int size = src.Size();
 if(size<=pos){
    if(q.empty()){
      return avlseg::none;
    } else {
      result = q.top();
      q.pop();
      return avlseg::first;
    }
 } else {
   HalfSegment hs1;
   src.Get(pos,hs1);
   avlseg::ExtendedHalfSegment hs(hs1);
   if(q.empty()){
      result = hs;
      pos++;
      return avlseg::first;
   } else{
      avlseg::ExtendedHalfSegment hsq = q.top();
      if(hsq<hs){
         result = hsq;
         q.pop();
         return avlseg::first;
      } else {
         pos++;
         result = hs;
         return avlseg::first;
      }
   }
 }
}

template<template<typename T> class Array>
void Realminize2(const LineT<Array>& src, LineT<Array>& result, 
                 const bool forceThrow=false){

  result.Clear();
  if(!src.IsDefined()){
     result.SetDefined(false);
     return;
  }
  result.SetDefined(true);
  if(src.Size()==0){ // empty line, nothing to realminize
    return;
  }

  std::priority_queue<avlseg::ExtendedHalfSegment,
                 std::vector<avlseg::ExtendedHalfSegment>,
                 std::greater<avlseg::ExtendedHalfSegment> > q;
  avltree::AVLTree<avlseg::AVLSegment> sss;

  int pos = 0;

  avlseg::ExtendedHalfSegment nextHS;
  avlseg::AVLSegment* member=0;
  avlseg::AVLSegment* leftN  = 0;
  avlseg::AVLSegment* rightN = 0;

  avlseg::AVLSegment left1, right1,left2,right2;

  result.StartBulkLoad();
  int edgeno = 0;
  avlseg::AVLSegment tmpL,tmpR;


  while(selectNext(src,pos,q,nextHS)!=avlseg::none) {
      avlseg::AVLSegment current(nextHS,avlseg::first);
      member = sss.getMember(current,leftN,rightN);
      if(leftN){
         tmpL = *leftN;
         leftN = &tmpL;
      }
      if(rightN){
         tmpR = *rightN;
         rightN = &tmpR;
      }
      if(nextHS.IsLeftDomPoint()){
         if(member){ // overlapping segment found in sss
            double xm = member->getX2();
            double xc = current.getX2();
            if(!AlmostEqual(xm,xc) && (xm<xc)){ // current extends member
               current.splitAt(xm,member->getY2(),left1,right1);
               insertEvents(right1,true,true,q,q);
            }
         } else { // no overlapping segment found
            splitByNeighbour(sss,current,leftN,q,q, forceThrow);
            splitByNeighbour(sss,current,rightN,q,q, forceThrow);
            if(!current.isPoint()){
              sss.insert(current);
              insertEvents(current,false,true,q,q);
            }
         }
      } else {  // nextHS rightDomPoint
          if(member && member->exactEqualsTo(current)){
             // insert the halfsegments
             avlseg::ExtendedHalfSegment hs1 =
                           current.convertToExtendedHs(true);
             avlseg::ExtendedHalfSegment hs2 =
                           current.convertToExtendedHs(false);
             hs1.attr.edgeno = edgeno;
             hs2.attr.edgeno = edgeno;
             result += hs1;
             result += hs2;
             splitNeighbours(sss,leftN,rightN,q,q, forceThrow);
             edgeno++;
             sss.remove(*member);
          }
      }
  }
  result.EndBulkLoad();
} // Realminize2

template<template<typename T> class Array>
avlseg::ownertype selectNext(const Array<HalfSegment>& src, int& pos,
                     std::priority_queue<avlseg::ExtendedHalfSegment,
                     std::vector<avlseg::ExtendedHalfSegment>,
                     std::greater<avlseg::ExtendedHalfSegment> >& q,
                     avlseg::ExtendedHalfSegment& result){

 int size = src.Size();
 if(size<=pos){
    if(q.empty()){
      return avlseg::none;
    } else {
      result = q.top();
      q.pop();
      return avlseg::first;
    }
 } else {
   HalfSegment hs1;
   src.Get(pos,hs1);
   avlseg::ExtendedHalfSegment hs(hs1);
   if(q.empty()){
      result = hs;
      pos++;
      return avlseg::first;
   } else{
      avlseg::ExtendedHalfSegment hsq = q.top();
      if(hsq<hs){
         result = hsq;
         q.pop();
         return avlseg::first;
      } else {
         pos++;
         result = hs;
         return avlseg::first;
      }
   }
 }
}


template<template<typename T> class Array>
Array<HalfSegment>* Realminize(const Array<HalfSegment>& segments, 
                  const bool forceThrow = false,
                  const bool robust = false){


  Array<HalfSegment>* res = new Array<HalfSegment>(segments.Size());
  if(robust){
    res->clean();
    robust::realminize(segments,*res);
  } else {
     try {
   
        if(segments.Size()==0){ // no halfsegments, nothing to realminize
          res->TrimToSize();
          return res;
        }
      
        std::priority_queue<avlseg::ExtendedHalfSegment,
                       std::vector<avlseg::ExtendedHalfSegment>,
                       std::greater<avlseg::ExtendedHalfSegment> > q1;
        // dummy queue
        std::priority_queue<avlseg::ExtendedHalfSegment,
                       std::vector<avlseg::ExtendedHalfSegment>,
                       std::greater<avlseg::ExtendedHalfSegment> > q2;
        avltree::AVLTree<avlseg::AVLSegment> sss;
      
        int pos = 0;
      
        avlseg::ExtendedHalfSegment nextHS;
        avlseg::AVLSegment* member=0;
        avlseg::AVLSegment* leftN  = 0;
        avlseg::AVLSegment* rightN = 0;
      
        avlseg::AVLSegment left1, right1,left2,right2;
      
        int edgeno = 0;
        avlseg::AVLSegment tmpL,tmpR,tmpM;
      
        XRemover xRemover;
      
        while(selectNext(segments,pos,q1,nextHS)!=avlseg::none) {
      
            avlseg::AVLSegment current(nextHS,avlseg::first);
            member = sss.getMember(current,leftN,rightN);
            if(leftN){
               tmpL = *leftN;
               leftN = &tmpL;
            }
            if(rightN){
               tmpR = *rightN;
               rightN = &tmpR;
            }
            if(member){
               tmpM = *member;
               member = &tmpM;
            }
            if(nextHS.IsLeftDomPoint()){
               if(member && member->overlaps(current)){
                  // overlapping segment found in sss
                  double xm = member->getX2();
                  double xc = current.getX2();
                  if(!AlmostEqual(xm,xc) && (xm<xc)){ // current extends member
                     // possibly rounding errors
                     if( xm < current.getX1()){
                        // create halfsegments and remove member
                        HalfSegment hs1 = current.convertToExtendedHs(true);
                        HalfSegment hs2 = current.convertToExtendedHs(false);
                        hs1.attr.edgeno = edgeno;
                        hs2.attr.edgeno = edgeno;
                        res->Append(hs1);
                        res->Append(hs2);
                        splitNeighbours(sss,leftN,rightN,q1,q2, forceThrow);
                        edgeno++;
                        sss.remove(*member);
                        insertEvents(current,true,true,q1,q2);
                     } else {
                       current.splitAtRight(xm,member->getY2(),right1);
                       insertEvents(right1,true,true,q1,q2);
                     }
                  } else if(AlmostEqual(xm,xc) && current.isVertical()){
                     double ym = member->getY2();
                     double yc = current.getY2();
                     if(!AlmostEqual(ym,yc) && (ym < yc)){
                        current.splitAtRight(xc,yc,right1);
                        insertEvents(right1,true,true,q1,q2);
                     }
                  }
               } else { // no overlapping segment found
                  if(splitByNeighbour(sss,current,leftN,q1,q2, forceThrow)){
                     insertEvents(current,true,true,q1,q2);
                  } else if(splitByNeighbour(sss,current,rightN,q1,q2, 
                                             forceThrow)) {
                     insertEvents(current,true,true,q1,q2);
                  } else {
                     sss.insert(current);
                     insertEvents(current,false,true,q1,q2);
                  }
               }
            } else {  // nextHS rightDomPoint
                if(member && member->exactEqualsTo(current)){
                   // insert the halfsegments
                   HalfSegment hs1 = current.convertToExtendedHs(true);
                   HalfSegment hs2 = current.convertToExtendedHs(false);
                   hs1.attr.edgeno = edgeno;
                   hs2.attr.edgeno = edgeno;
                   res->Append(hs1);
                   res->Append(hs2);
                   splitNeighbours(sss,leftN,rightN,q1,q2, forceThrow);
                   edgeno++;
                   sss.remove(*member);
                }
          }
      
          if(avlseg::AVLSegment::isError()){
             std::cerr << "error during comparision detected" << std::endl;
             avltree::AVLTree<avlseg::AVLSegment>::iterator it = sss.begin();
             double val = avlseg::AVLSegment::getErrorValue();
             std::vector<avlseg::AVLSegment*> evilsegments;
             cout << "start iterating" << std::endl;
             unsigned int size = sss.Size();
             cout << "The tree has " << size << " entries" << std::endl;
             unsigned int b=0;
             while(!it.onEnd()){
                b++;
                avlseg::AVLSegment* seg = it.Get();
                double x2 = seg->getX2();
                if(!AlmostEqual(x2,val) && x2 < val){
                    evilsegments.push_back(seg);
                }
                it++;
                assert(b<=size);
             }
             if(evilsegments.size() > 0){
                cout << "recognized " << evilsegments.size()
                     << " evil segments" << std::endl;
                cout << "error_value = " << val << std::endl;
                unsigned int count = 0;
                for(unsigned int i=0; i< evilsegments.size();i++){
                  avlseg::AVLSegment* seg = evilsegments[i];
                  HalfSegment hs1 = seg->convertToExtendedHs(true);
                  HalfSegment hs2 = seg->convertToExtendedHs(false);
                  hs1.attr.edgeno = edgeno;
                  hs2.attr.edgeno = edgeno;
                  res->Append(hs1);
                  res->Append(hs2);
                  bool ok = sss.remove(*seg);
                  edgeno++;
                  evilsegments[i] = 0;
                  if(!ok){
                    count++;
                  }
                }
                if(count != 0){
                  cout << "some (" << count
                       << " of the evil segments was not found, scan the tree"
                       << " to remove them" << std::endl;
                  xRemover.setX(val);
                  cout << "Start removeAll" << std::endl;
                  unsigned int count2 = sss.removeAll(xRemover);
                  if(count != count2){
                    cout << count
                         << " elements should be remove, but only " << count2
                         << " was found " << std::endl;
                  }
                }
                cout << "segments removed" << std::endl;
             } else {
                cout << "evil segment already processed" << std::endl;
             }
             avlseg::AVLSegment::clearError();
      
          }
        }
        if(sss.Size()!=0){
          cout << " After planesweep, the status structure is not empty ! " 
               << std::endl;
        }
     } catch (...){
       std::cerr << "Realminize via plane sweep failed, switch to "
               "slower (robust) implementation" 
            << std::endl;
       res->clean();
       robust::realminize(segments,*res);
     }
  } 
  res->Sort(HalfSegmentCompare);
  res->TrimToSize();
  return res;
}


template<template<typename T> class Array>
bool hasOverlaps(const Array<HalfSegment>& segments,
                 const bool ignoreEqual,
                 const bool forceThrow = false){
  if(segments.Size()<2){ // no overlaps possible
    return false;
  }

  std::priority_queue<avlseg::ExtendedHalfSegment,
                 std::vector<avlseg::ExtendedHalfSegment>,
                 std::greater<avlseg::ExtendedHalfSegment> > q;
  avltree::AVLTree<avlseg::AVLSegment> sss;

  int pos = 0;

  avlseg::ExtendedHalfSegment nextHS;
  avlseg::AVLSegment* member=0;
  avlseg::AVLSegment* leftN  = 0;
  avlseg::AVLSegment* rightN = 0;

  avlseg::AVLSegment left1, right1,left2,right2;

  avlseg::AVLSegment tmpL,tmpR;

  while(selectNext(segments,pos,q,nextHS)!=avlseg::none) {
      avlseg::AVLSegment current(nextHS,avlseg::first);
      member = sss.getMember(current,leftN,rightN);
      if(leftN){
         tmpL = *leftN;
         leftN = &tmpL;
      }
      if(rightN){
         tmpR = *rightN;
         rightN = &tmpR;
      }
      if(nextHS.IsLeftDomPoint()){
         if(member){ // overlapping segment found in sss
            if(!ignoreEqual || !member->exactEqualsTo(current)){
              return true;
            }
            double xm = member->getX2();
            double xc = current.getX2();
            if(!AlmostEqual(xm,xc) && (xm<xc)){ // current extends member
               current.splitAtRight(xm,member->getY2(),right1);
               insertEvents(right1,true,true,q,q);
            }
         } else { // no overlapping segment found
            splitByNeighbour(sss,current,leftN,q,q, forceThrow);
            splitByNeighbour(sss,current,rightN,q,q, forceThrow);
            if(!current.isPoint()){
              sss.insert(current);
              insertEvents(current,false,true,q,q);
            }
         }
      } else {  // nextHS rightDomPoint
          if(member && member->exactEqualsTo(current)){
             // insert the halfsegments
             splitNeighbours(sss,leftN,rightN,q,q, forceThrow);
             sss.remove(*member);
          }
      }
  }
  return false;
}

/*
~Split~

This function works similar to the realminize function. The difference is,
that overlapping parts of segments are kept, instead of beeig removed.
But at all crossing points and so on, the segments will be split.

*/
template<template<typename T> class Array>
Array<HalfSegment>* Split(const Array<HalfSegment>& segments,
             const bool forceThrow =false){

  Array<HalfSegment>* res = new Array<HalfSegment>(0);

  if(segments.Size()<2){ // no intersecting halfsegments
    res->TrimToSize();
    return res;
  }

  std::priority_queue<avlseg::ExtendedHalfSegment,
                 std::vector<avlseg::ExtendedHalfSegment>,
                 std::greater<avlseg::ExtendedHalfSegment> > q;
  avltree::AVLTree<avlseg::AVLSegment> sss;

  int pos = 0;

  avlseg::ExtendedHalfSegment nextHS;
  avlseg::AVLSegment* member=0;
  avlseg::AVLSegment* leftN  = 0;
  avlseg::AVLSegment* rightN = 0;

  avlseg::AVLSegment left1, right1,left2,right2;

  int edgeno = 0;
  avlseg::AVLSegment tmpL,tmpR;

  while(selectNext(segments,pos,q,nextHS)!=avlseg::none) {
      avlseg::AVLSegment current(nextHS,avlseg::first);
      member = sss.getMember(current,leftN,rightN);
      if(leftN){
         tmpL = *leftN;
         leftN = &tmpL;
      }
      if(rightN){
         tmpR = *rightN;
         rightN = &tmpR;
      }
      if(nextHS.IsLeftDomPoint()){
         if(member){ // overlapping segment found in sss
            // insert the common part into res
            avlseg::AVLSegment tmp_left, tmp_common, tmp_right;
            uint32_t sr = member->split(current,tmp_left,
                                        tmp_common,tmp_right,false);

            tmp_left.setOwner(avlseg::first);
            tmp_common.setOwner(avlseg::first);
            tmp_right.setOwner(avlseg::first);
            Point pl(true,tmp_common.getX1(),tmp_common.getY1());
            Point pr(true,tmp_common.getX2(),tmp_common.getY2());

            HalfSegment hs1 = tmp_common.convertToExtendedHs(true);
            HalfSegment hs2 = tmp_common.convertToExtendedHs(false);
            hs1.attr.edgeno = edgeno;
            hs2.attr.edgeno = edgeno;
            res->Append(hs1);
            res->Append(hs2);
            edgeno++;


            sss.remove(*member);
            if(sr & avlseg::LEFT){
              if(!tmp_left.isPoint()){
                sss.insert(tmp_left);
                insertEvents(tmp_left,false,true,q,q);
              }
            }
            insertEvents(tmp_common,true,true,q,q);
            if(sr & avlseg::RIGHT){
              insertEvents(tmp_right,true,true,q,q);
            }
         } else { // no overlapping segment found
            splitByNeighbour(sss,current,leftN,q,q, forceThrow);
            splitByNeighbour(sss,current,rightN,q,q, forceThrow);
            if(!current.isPoint()){
              sss.insert(current);
              insertEvents(current,false,true,q,q);
            }
         }
      } else {  // nextHS rightDomPoint
          if(member && member->exactEqualsTo(current)){
             // insert the halfsegments
             HalfSegment hs1 = current.convertToExtendedHs(true);
             HalfSegment hs2 = current.convertToExtendedHs(false);
             hs1.attr.edgeno = edgeno;
             hs2.attr.edgeno = edgeno;
             res->Append(hs1);
             res->Append(hs2);
             splitNeighbours(sss,leftN,rightN,q,q, forceThrow);
             edgeno++;
             sss.remove(*member);
          }
      }
  }
  res->Sort(HalfSegmentCompare);
  res->TrimToSize();
  // work around because problems with overlapping segments
  if(hasOverlaps(*res,true)){
    Array<HalfSegment>* tmp = Split(*res);
    delete res;
    res = tmp;
  }

  return res;
}




/*
9.2 ~line~ [x] ~line~ [->] ~line~

This combination can be used for all possible set operations.


*/
template<template<typename T> class Array>
void SetOp(const LineT<Array>& line1,
           const LineT<Array>& line2,
           LineT<Array>& result,
           avlseg::SetOperation op,
           const Geoid* geoid=0,
           const bool forceThrow =false){

   result.Clear();

   if(geoid){
     std::cerr << __PRETTY_FUNCTION__ << ": Spherical geometry not implemented."
     << std::endl;
     assert( false ); // TODO: implement spherical geometry case
   }

   if(!line1.IsDefined() || !line2.IsDefined() ||
      (geoid && !geoid->IsDefined()) ){
       result.SetDefined(false);
       return;
   }
   result.SetDefined(true);
   if(line1.Size()==0){
       switch(op){
         case avlseg::union_op : result = line2;
                         return;
         case avlseg::intersection_op : return; // empty line
         case avlseg::difference_op : return; // empty line
         default : assert(false);
       }
   }
   if(line2.Size()==0){
      switch(op){
         case avlseg::union_op: result = line1;
                        return;
         case avlseg::intersection_op: return;
         case avlseg::difference_op: result = line1;
                             return;
         default : assert(false);
      }
   }

  std::priority_queue<avlseg::ExtendedHalfSegment,
                 std::vector<avlseg::ExtendedHalfSegment>,
                 std::greater<avlseg::ExtendedHalfSegment> > q1;
  std::priority_queue<avlseg::ExtendedHalfSegment,
                 std::vector<avlseg::ExtendedHalfSegment>,
                 std::greater<avlseg::ExtendedHalfSegment> > q2;
  avltree::AVLTree<avlseg::AVLSegment> sss;
  avlseg::ownertype owner;
  int pos1 = 0;
  int pos2 = 0;
  avlseg::ExtendedHalfSegment nextHs;
  int src = 0;

  avlseg::AVLSegment* member=0;
  avlseg::AVLSegment* leftN = 0;
  avlseg::AVLSegment* rightN = 0;

  avlseg::AVLSegment left1,right1,common1,
             left2,right2;

  int edgeno =0;
  avlseg::AVLSegment tmpL,tmpR;

  result.StartBulkLoad();
  while( (owner=selectNext(line1,pos1,
                           line2,pos2,
                           q1,q2,nextHs,src))!=avlseg::none){
       avlseg::AVLSegment current(nextHs,owner);
       member = sss.getMember(current,leftN,rightN);
       if(leftN){
         tmpL = *leftN;
         leftN = &tmpL;
       }
       if(rightN){
         tmpR = *rightN;
         rightN = &tmpR;
       }
       if(nextHs.IsLeftDomPoint()){
          if(member){ // found an overlapping segment
             if(member->getOwner()==current.getOwner() ||
                member->getOwner()==avlseg::both){ // same source
                 double xm = member->getX2();
                 double xc = current.getX2();
                 if(!AlmostEqual(xm,xc) && (xm<xc)){ // current extends member
                    current.splitAtRight(xm,member->getY2(),right1);
                    insertEvents(right1,true,true,q1,q2);
                 }
             }  else { // member and current come from different sources
                 uint32_t parts = member->split(current,left1,common1,right1);
                 sss.remove(*member);
                 member = &common1;
                 if(parts & avlseg::LEFT){
                     if(!left1.isPoint()){
                       sss.insert(left1);
                       insertEvents(left1,false,true,q1,q2);
                     }
                 }
                 assert(parts & avlseg::COMMON);
                 if(!common1.isPoint()){
                   sss.insert(common1);
                   insertEvents(common1,false,true,q1,q2);
                 }
                 if(parts & avlseg::RIGHT){
                    insertEvents(right1,true,true,q1,q2);
                 }
             }
          } else { // no overlapping segment found
            splitByNeighbour(sss,current,leftN,q1,q2, forceThrow);
            splitByNeighbour(sss,current,rightN,q1,q2, forceThrow);
            if(!current.isPoint()){
              sss.insert(current);
              insertEvents(current,false,true,q1,q2);
            }
          }
       } else { // nextHS rightDomPoint
         if(member && member->exactEqualsTo(current)){
             // insert the segments into the result
             switch(op){
                case avlseg::union_op : {
                     HalfSegment hs1 =
                           member->convertToExtendedHs(true,avlseg::first);
                     hs1.attr.edgeno = edgeno;
                     result += hs1;
                     hs1.SetLeftDomPoint(false);
                     result += hs1;
                     edgeno++;
                     break;
                } case avlseg::intersection_op : {
                     if(member->getOwner()==avlseg::both){
                        HalfSegment hs1 =
                           member->convertToExtendedHs(true,avlseg::first);
                        hs1.attr.edgeno = edgeno;
                        result += hs1;
                        hs1.SetLeftDomPoint(false);
                        result += hs1;
                        edgeno++;
                      }
                      break;
                } case avlseg::difference_op :{
                      if(member->getOwner()==avlseg::first){
                        HalfSegment hs1 =
                            member->convertToExtendedHs(true,avlseg::first);
                        hs1.attr.edgeno = edgeno;
                        result += hs1;
                        hs1.SetLeftDomPoint(false);
                        result += hs1;
                        edgeno++;
                      }
                      break;
                } default : {
                      assert(false);
                }
             }
             sss.remove(*member);
             splitNeighbours(sss,leftN,rightN,q1,q2, forceThrow);
         }
       }
  }
  result.EndBulkLoad(true,false);
} // setop line x line -> line

template<template<typename T> class Array>
LineT<Array>* SetOp(const LineT<Array>& line1, const LineT<Array>& line2,
                    avlseg::SetOperation op,
                    const Geoid* geoid=0){
  LineT<Array>* result = new LineT<Array>(1);
  SetOp(line1,line2,*result,op,geoid);
  return result;
}


/*

9.3 ~region~ [x] ~region~ [->] ~region~

*/
template<template<typename T> class Array>
void SetOp(const RegionT<Array>& reg1,
           const RegionT<Array>& reg2,
           RegionT<Array>& result,
           avlseg::SetOperation op,
           const Geoid* geoid=0,
           const bool forceThrow=false){

   if(geoid){
      std::cerr << __PRETTY_FUNCTION__ 
                << ": Spherical geometry not implemented."
                << std::endl;
      assert( false ); // TODO: implement spherical case
   }
   result.Clear();
   if(!reg1.IsDefined() || !reg2.IsDefined() || (geoid && !geoid->IsDefined())){
       result.SetDefined(false);
       return;
   }
   result.SetDefined(true);
   if(reg1.Size()==0){
       switch(op){
         case avlseg::union_op : result = reg2;
                         return;
         case avlseg::intersection_op : return; // empty region
         case avlseg::difference_op : return; // empty region
         default : assert(false);
       }
   }
   if(reg2.Size()==0){
      switch(op){
         case avlseg::union_op: result = reg1;
                        return;
         case avlseg::intersection_op: return;
         case avlseg::difference_op: result = reg1;
                             return;
         default : assert(false);
      }
   }

   if(!reg1.BoundingBox().Intersects(reg2.BoundingBox(),geoid)){
      switch(op){
        case avlseg::union_op: {
          result.StartBulkLoad();
          int edgeno=0;
          int s = reg1.Size();
          HalfSegment hs;
          for(int i=0;i<s;i++){
              reg1.Get(i,hs);
              if(hs.IsLeftDomPoint()){
                 HalfSegment HS(hs);
                 HS.attr.edgeno = edgeno;
                 result += HS;
                 HS.SetLeftDomPoint(false);
                 result += HS;
                 edgeno++;
              }
          }
          s = reg2.Size();
          for(int i=0;i<s;i++){
              reg2.Get(i,hs);
              if(hs.IsLeftDomPoint()){
                 HalfSegment HS(hs);
                 HS.attr.edgeno = edgeno;
                 result += HS;
                 HS.SetLeftDomPoint(false);
                 result += HS;
                 edgeno++;
              }
          }
          result.EndBulkLoad();
          return;
        } case avlseg::difference_op: {
           result = reg1;
           return;
        } case avlseg::intersection_op:{
           return;
        } default: assert(false);
      }
   }



  std::priority_queue<avlseg::ExtendedHalfSegment,
                 std::vector<avlseg::ExtendedHalfSegment>,
                 std::greater<avlseg::ExtendedHalfSegment> > q1;
  std::priority_queue<avlseg::ExtendedHalfSegment,
                 std::vector<avlseg::ExtendedHalfSegment>,
                 std::greater<avlseg::ExtendedHalfSegment> > q2;
  avltree::AVLTree<avlseg::AVLSegment> sss;
  avlseg::ownertype owner;
  int pos1 = 0;
  int pos2 = 0;
  avlseg::ExtendedHalfSegment nextHs;
  int src = 0;

  avlseg::AVLSegment* member = 0;
  avlseg::AVLSegment* leftN  = 0;
  avlseg::AVLSegment* rightN = 0;

  avlseg::AVLSegment left1,right1,common1,
             left2,right2;

  int edgeno =0;
  avlseg::AVLSegment tmpL,tmpR;

  result.StartBulkLoad();


  while( (owner=selectNext(reg1,pos1,
                           reg2,pos2,
                           q1,q2,nextHs,src))!=avlseg::none){


       avlseg::AVLSegment current(nextHs,owner);
       avlseg::AVLSegment currentCopy(current);


       member = sss.getMember(current,leftN,rightN);



       if(leftN){
          tmpL = *leftN;
          leftN = &tmpL;
       }
       if(rightN){
          tmpR = *rightN;
          rightN = &tmpR;
       }
       if(nextHs.IsLeftDomPoint()){
          if(member){ // overlapping segment found
            if((member->getOwner()==avlseg::both) ||
               (member->getOwner()==owner)){
               std::cerr << "overlapping segments detected within a "
                         << "single region"
                         << std::endl;
               std::cerr << "the argument is "
                    << (owner==avlseg::first?"first":"second")
                    << std::endl;
               std::cerr.precision(16);
               std::cerr << "stored is " << *member << std::endl;
               std::cerr << "current = " << current << std::endl;
               avlseg::AVLSegment tmp_left, tmp_common, tmp_right;
               member->split(current,tmp_left, tmp_common, tmp_right,
                             false);
               std::cerr << "The common part is " << tmp_common << std::endl;
               std::cerr << "The lenth = " << tmp_common.length() << std::endl;
               assert(false);
            }
            uint32_t parts = member->split(current,left1,common1,right1,true);
            sss.remove(*member);
            if(parts & avlseg::LEFT){
              if(!left1.isPoint()){
                sss.insert(left1);
                insertEvents(left1,false,true,q1,q2);
              }
            }
            assert(parts & avlseg::COMMON);
            // update coverage numbers
            if(current.getInsideAbove()){
               common1.con_above++;
            }  else {
               common1.con_above--;
            }
            if(!common1.isPoint()){
              sss.insert(common1);
              if((parts & avlseg::LEFT) || (parts & avlseg::RIGHT)){
                 insertEvents(common1,false,true,q1,q2);
              } // otherwise the original is not changed
            }
            if(parts & avlseg::RIGHT){
               insertEvents(right1,true,true,q1,q2);
            }
          } else { // there is no overlapping segment

            // try to split segments if required
            splitByNeighbour(sss,current,leftN,q1,q2, forceThrow);
            splitByNeighbour(sss,current,rightN,q1,q2, forceThrow);


            // update coverage numbers
            bool iac = current.getOwner()==avlseg::first
                            ?current.getInsideAbove_first()
                            :current.getInsideAbove_second();

            iac = current.getOwner()==avlseg::first
                                           ?current.getInsideAbove_first()
                                           :current.getInsideAbove_second();

            if(leftN && current.extends(*leftN)){
              current.con_below = leftN->con_below;
              current.con_above = leftN->con_above;
            }else{
              if(leftN && leftN->isVertical()){
                 current.con_below = leftN->con_below;
              } else if(leftN){
                 current.con_below = leftN->con_above;
              } else {
                 current.con_below = 0;
              }
              if(iac){
                 current.con_above = current.con_below+1;
              } else {
                 current.con_above = current.con_below-1;
              }
            }
            // insert element if it was changed

            if(!current.isPoint()){
              sss.insert(current);
              if(!current.exactEqualsTo(currentCopy)){
                insertEvents(current,false,true,q1,q2);
              }
            }
          }
       } else {  // nextHs.IsRightDomPoint
          if(member && member->exactEqualsTo(current)){
              switch(op){
                case avlseg::union_op :{

                   if( (member->con_above==0) || (member->con_below==0)) {
                      HalfSegment hs1 = member->getOwner()==avlseg::both
                               ?member->convertToExtendedHs(true,avlseg::first)
                               :member->convertToExtendedHs(true);
                      hs1.attr.edgeno = edgeno;
                      result += hs1;
                      hs1.SetLeftDomPoint(false);
                      result += hs1;
                      edgeno++;
                   }
                   break;
                }
                case avlseg::intersection_op: {

                  if(member->con_above==2 || member->con_below==2){
                      HalfSegment hs1 = member->getOwner()==avlseg::both
                              ?member->convertToExtendedHs(true,avlseg::first)
                              :member->convertToExtendedHs(true);
                      hs1.attr.edgeno = edgeno;
                      hs1.attr.insideAbove = (member->con_above==2);
                      result += hs1;
                      hs1.SetLeftDomPoint(false);
                      result += hs1;
                      edgeno++;
                  }
                  break;
                }
                case avlseg::difference_op : {
                  switch(member->getOwner()){
                    case avlseg::first:{
                      if(member->con_above + member->con_below == 1){
                         HalfSegment hs1 = member->getOwner()==avlseg::both
                               ?member->convertToExtendedHs(true,avlseg::first)
                               :member->convertToExtendedHs(true);
                         hs1.attr.edgeno = edgeno;
                         result += hs1;
                         hs1.SetLeftDomPoint(false);
                         result += hs1;
                         edgeno++;
                      }
                      break;
                    }
                    case avlseg::second:{
                      if(member->con_above + member->con_below == 3){
                         HalfSegment hs1 = member->getOwner()==avlseg::both
                               ?member->convertToExtendedHs(true,avlseg::second)
                               :member->convertToExtendedHs(true);
                         hs1.attr.insideAbove = ! hs1.attr.insideAbove;
                         hs1.attr.edgeno = edgeno;
                         result += hs1;
                         hs1.SetLeftDomPoint(false);
                         result += hs1;
                         edgeno++;
                      }
                      break;
                    }
                    case avlseg::both: {
                      if((member->con_above==1) && (member->con_below== 1)){
                         HalfSegment hs1 = member->getOwner()==avlseg::both
                               ?member->convertToExtendedHs(true,avlseg::first)
                               :member->convertToExtendedHs(true);
                         hs1.attr.insideAbove = member->getInsideAbove_first();
                         hs1.attr.edgeno = edgeno;
                         result += hs1;
                         hs1.SetLeftDomPoint(false);
                         result += hs1;
                         edgeno++;
                      }
                      break;
                    }
                    default : assert(false);
                  } // switch member->getOwner
                  break;
                } // case difference
                default : assert(false);
              } // end of switch
              sss.remove(*member);
              splitNeighbours(sss,leftN,rightN,q1,q2, forceThrow);
          } // current found in sss
       } // right endpoint
  }



  result.EndBulkLoad();


} // setOP region x region -> region



template<template<typename T> class Array>
RegionT<Array>* SetOp(const RegionT<Array>& reg1, const RegionT<Array>& reg2,
                      avlseg::SetOperation op,
                      const Geoid* geoid=0,
                      const bool forceThrow=false){
  RegionT<Array>* result = new RegionT<Array>(1);
  SetOp(reg1,reg2,*result,op,geoid);
  return result;
}


/*
9  ~CommonBorder~

Signature: ~region~ [x] ~region~ [->] ~line~

*/
template<template<typename T> class Array>
void CommonBorder(
           const RegionT<Array>& reg1,
           const RegionT<Array>& reg2,
           LineT<Array>& result,
           const Geoid* geoid=0,
           const bool forceThrow=false){

   result.Clear();

   if(geoid){
     std::cerr << __PRETTY_FUNCTION__ << ": Spherical geometry not implemented."
     << std::endl;
     assert( false ); // TODO: implement spherical geometry case
   }

   if(!reg1.IsDefined() || !reg2.IsDefined() ||
      (geoid && !geoid->IsDefined()) ){
       result.SetDefined(false);
       return;
   }
   result.SetDefined(true);
   if(reg1.Size()==0 || reg2.Size()==0){
       // a region is empty -> the common border is also empty
       return;
   }
   if(!reg1.BoundingBox().Intersects(reg2.BoundingBox())){
      // no common border possible
      return;
   }

  std::priority_queue<avlseg::ExtendedHalfSegment,
                 std::vector<avlseg::ExtendedHalfSegment>,
                 std::greater<avlseg::ExtendedHalfSegment> > q1;
  std::priority_queue<avlseg::ExtendedHalfSegment,
                 std::vector<avlseg::ExtendedHalfSegment>,
                 std::greater<avlseg::ExtendedHalfSegment> > q2;
  avltree::AVLTree<avlseg::AVLSegment> sss;
  avlseg::ownertype owner;
  int pos1 = 0;
  int pos2 = 0;
  avlseg::ExtendedHalfSegment nextHs;
  int src = 0;

  avlseg::AVLSegment* member=0;
  avlseg::AVLSegment* leftN = 0;
  avlseg::AVLSegment* rightN = 0;

  avlseg::AVLSegment left1,right1,common1,
             left2,right2;

  int edgeno =0;
  avlseg::AVLSegment tmpL,tmpR;

  result.StartBulkLoad();
  bool done = false;
  int size1 = reg1.Size();
  int size2 = reg2.Size();

  while( ((owner=selectNext(reg1,pos1,
                            reg2,pos2,
                            q1,q2,nextHs,src))!=avlseg::none)
         && !done  ){
       avlseg::AVLSegment current(nextHs,owner);
       member = sss.getMember(current,leftN,rightN);
       if(leftN){
          tmpL = *leftN;
          leftN = &tmpL;
       }
       if(rightN){
          tmpR = *rightN;
          rightN = &tmpR;
       }
       if(nextHs.IsLeftDomPoint()){
          if(member){ // overlapping segment found
            assert(member->getOwner()!=avlseg::both);
            assert(member->getOwner()!=owner);
            uint32_t parts = member->split(current,left1,common1,right1);
            sss.remove(*member);
            if(parts & avlseg::LEFT){
              if(!left1.isPoint()){
                sss.insert(left1);
                insertEvents(left1,false,true,q1,q2);
              }
            }
            assert(parts & avlseg::COMMON);
            if(!common1.isPoint()){
              sss.insert(common1);
              insertEvents(common1,false,true,q1,q2);
            }
            if(parts & avlseg::RIGHT){
               insertEvents(right1,true,true,q1,q2);
            }
          } else { // there is no overlapping segment
            // try to split segments if required
            splitByNeighbour(sss,current,leftN,q1,q2, forceThrow);
            splitByNeighbour(sss,current,rightN,q1,q2, forceThrow);
            if(!current.isPoint()){
              sss.insert(current);
              insertEvents(current,false,true,q1,q2);
            }
          }
       } else {  // nextHs.IsRightDomPoint
          if(member && member->exactEqualsTo(current)){
              if(member->getOwner()==avlseg::both){
                 HalfSegment hs =
                         member->convertToExtendedHs(true,avlseg::first);
                 hs.attr.edgeno = edgeno;
                 result += hs;
                 hs.SetLeftDomPoint(false);
                 result += hs;
                 edgeno++;
              }
              sss.remove(*member);
              splitNeighbours(sss,leftN,rightN,q1,q2,forceThrow);
          } // current found in sss
          if(((pos1 >= size1) && q1.empty())  ||
             ((pos2 >= size2) && q2.empty())){
             done = true;
          }
       } // right endpoint
  }
  result.EndBulkLoad();
} // commonborder

/*
9.4 ~region~ [x] ~line~ [->] ~region~

This combination can only be used for the operations
~union~ and ~difference~. In both cases, the result will be
the original region value.

*/
template<template<typename T> class Array>
void SetOp(const RegionT<Array>& region,
           const LineT<Array>& line,
           RegionT<Array>& result,
           avlseg::SetOperation op,
           const Geoid* geoid=0){

   assert(op == avlseg::union_op || op == avlseg::difference_op);
   result.Clear();
   if(!line.IsDefined() || !region.IsDefined() ||
      (geoid && !geoid->IsDefined()) ){
      result.SetDefined(false);
      return;
   }
   result.SetDefined(true);
   result.CopyFrom(&region);
}

/*
9.5  ~line~ [x] ~region~ [->] ~line~

Here, only the ~difference~ and ~intersection~ operation are applicable.


*/
template<template<typename T> class Array>
void SetOp(const LineT<Array>& line,
           const RegionT<Array>& region,
           LineT<Array>& result,
           avlseg::SetOperation op,
           const Geoid* geoid=0,
           const bool forceThrow =false){

  assert(op==avlseg::intersection_op || op == avlseg::difference_op);

  if(geoid){
    std::cerr << __PRETTY_FUNCTION__ << ": Spherical geometry not implemented."
         << std::endl;
    assert( false ); // TODO: implement spherical geometry case
  }
  result.Clear();
  if(!line.IsDefined() || !region.IsDefined() ||
     (geoid && !geoid->IsDefined()) ){
       result.SetDefined(false);
       return;
   }
   result.SetDefined(true);
   if(line.Size()==0){ // empty line -> empty result
       switch(op){
         case avlseg::intersection_op : return; // empty region
         case avlseg::difference_op : return; // empty region
         default : assert(false);
       }
   }
   if(region.Size()==0){
      switch(op){
         case avlseg::intersection_op: return;
         case avlseg::difference_op: result = line;
                             return;
         default : assert(false);
      }
   }

  // set result to a highly estimated value to avoid
  // frequent enlargements of the underlying DbArray
  result.Resize(line.Size());  

  std::priority_queue<avlseg::ExtendedHalfSegment,
                 std::vector<avlseg::ExtendedHalfSegment>,
                 std::greater<avlseg::ExtendedHalfSegment> > q1;
  std::priority_queue<avlseg::ExtendedHalfSegment,
                 std::vector<avlseg::ExtendedHalfSegment>,
                 std::greater<avlseg::ExtendedHalfSegment> > q2;
  avltree::AVLTree<avlseg::AVLSegment> sss;
  avlseg::ownertype owner;
  int pos1 = 0;
  int pos2 = 0;
  int size1= line.Size();
  avlseg::ExtendedHalfSegment nextHs;
  int src = 0;

  avlseg::AVLSegment* member=0;
  avlseg::AVLSegment* leftN = 0;
  avlseg::AVLSegment* rightN = 0;

  avlseg::AVLSegment left1,right1,common1,
             left2,right2;

  int edgeno =0;
  avlseg::AVLSegment tmpL,tmpR;
  bool done = false;

  result.StartBulkLoad();
  // perform a planesweeo
  while( ((owner=selectNext(line,pos1,
                            region,pos2,
                            q1,q2,nextHs,src))!=avlseg::none)
         && ! done){
     avlseg::AVLSegment current(nextHs,owner);
     member = sss.getMember(current,leftN,rightN);
     if(leftN){
        tmpL = *leftN;
        leftN = &tmpL;
     }
     if(rightN){
        tmpR = *rightN;
        rightN = &tmpR;
     }
     if(nextHs.IsLeftDomPoint()){
        if(member){ // there is an overlapping segment in sss
           if(member->getOwner()==owner ||
              member->getOwner()==avlseg::both     ){
              if(current.ininterior(member->getX2(),member->getY2())){
                 current.splitAtRight(member->getX2(),member->getY2(),right1);
                 insertEvents(right1,true,true,q1,q2);
              }
           } else { // member and source come from difference sources
             uint32_t parts = member->split(current,left1,common1,right1);
             sss.remove(*member);
             member = &common1;
             if(parts & avlseg::LEFT){
                if(!left1.isPoint()){
                  sss.insert(left1);
                  insertEvents(left1,false,true,q1,q2);
                }
             }
             assert(parts & avlseg::COMMON);
             if(owner==avlseg::second) {  // the region
               if(current.getInsideAbove()){
                  common1.con_above++;
               } else {
                  common1.con_above--;
               }
             } // for a line is nothing to do
             if(!common1.isPoint()){
               sss.insert(common1);
               insertEvents(common1,false,true,q1,q2);
             }
             if(parts & avlseg::RIGHT){
                 insertEvents(right1,true,true,q1,q2);
             }
           }
        } else { // no overlapping segment in sss found
          splitByNeighbour(sss,current,leftN,q1,q2, forceThrow);
          splitByNeighbour(sss,current,rightN,q1,q2, forceThrow);
          // update coverage numbers
          if(owner==avlseg::second){ // the region
            bool iac = current.getInsideAbove();
            if(leftN && current.extends(*leftN)){
              current.con_below = leftN->con_below;
              current.con_above = leftN->con_above;
            }else{
              if(leftN && leftN->isVertical()){
                 current.con_below = leftN->con_below;
              } else if(leftN){
                 current.con_below = leftN->con_above;
              } else {
                 current.con_below = 0;
              }
              if(iac){
                 current.con_above = current.con_below+1;
              } else {
                 current.con_above = current.con_below-1;
              }
            }
          } else { // the line
            if(leftN){
               if(leftN->isVertical()){
                  current.con_below = leftN->con_below;
               } else {
                  current.con_below = leftN->con_above;
               }
            }
            current.con_above = current.con_below;
          }
          // insert element
          if(!current.isPoint()){
            sss.insert(current);
            insertEvents(current,false,true,q1,q2);
          }
        }
     } else { // nextHs.IsRightDomPoint()
       if(member && member->exactEqualsTo(current)){

          switch(op){
              case avlseg::intersection_op: {
                if( (member->getOwner()==avlseg::both) ||
                    (member->getOwner()==avlseg::first && member->con_above>0)){
                    HalfSegment hs1 =
                           member->convertToExtendedHs(true,avlseg::first);
                    hs1.attr.edgeno = edgeno;
                    result += hs1;
                    hs1.SetLeftDomPoint(false);
                    result += hs1;
                    edgeno++;
                }
                break;
              }
              case avlseg::difference_op: {
                if( (member->getOwner()==avlseg::first) &&
                    (member->con_above==0)){
                    HalfSegment hs1 =
                          member->convertToExtendedHs(true,avlseg::first);
                    hs1.attr.edgeno = edgeno;
                    result += hs1;
                    hs1.SetLeftDomPoint(false);
                    result += hs1;
                    edgeno++;
                }
                break;
              }
              default : assert(false);
          }
          sss.remove(*member);
          splitNeighbours(sss,leftN,rightN,q1,q2, forceThrow);
       }
       if(pos1>=size1 && q1.empty()){ // line is processed
          done = true;
       }
     }
  }
  result.EndBulkLoad();
} // setOP(line x region -> line)

/*
line x sline -> sline

Here is only ~intersection~ applicable.

*/
template<template<typename T> class Array>
void SetOp(const LineT<Array>& line1, 
           const SimpleLineT<Array>& line2, 
           SimpleLineT<Array>& result,
           avlseg::SetOperation op, const Geoid* geoid =0,
           const bool forceThrow=false){
  result.Clear();

  if(geoid){
    std::cerr << __PRETTY_FUNCTION__ << ": Spherical geometry not implemented."
    << std::endl;
    assert( false ); // TODO: implement spherical geometry case
  }
  if(!line1.IsDefined() || !line2.IsDefined() ||
       (geoid && !geoid->IsDefined()) ){
       result.SetDefined(false);
     return;
  }
  result.SetDefined(true);
  if(line1.Size() == 0 || line2.Size() == 0)
    return; // empty line

  std::priority_queue<avlseg::ExtendedHalfSegment,
  std::vector<avlseg::ExtendedHalfSegment>,
  std::greater<avlseg::ExtendedHalfSegment> > q1;
  std::priority_queue<avlseg::ExtendedHalfSegment,
  std::vector<avlseg::ExtendedHalfSegment>,
  std::greater<avlseg::ExtendedHalfSegment> > q2;
  avltree::AVLTree<avlseg::AVLSegment> sss;
  avlseg::ownertype owner;
  int pos1 = 0;
  int pos2 = 0;
  avlseg::ExtendedHalfSegment nextHs;
  int src = 0;

  avlseg::AVLSegment* member=0;
  avlseg::AVLSegment* leftN = 0;
  avlseg::AVLSegment* rightN = 0;

  avlseg::AVLSegment left1,right1,common1, left2,right2;

  int edgeno =0;
  avlseg::AVLSegment tmpL,tmpR;

  result.StartBulkLoad();
  while((owner=selectNext(line1,pos1,line2,pos2, q1,q2,nextHs,src))
        != avlseg::none){
    avlseg::AVLSegment current(nextHs,owner);
    member = sss.getMember(current,leftN,rightN);
    if(leftN){
      tmpL = *leftN;
      leftN = &tmpL;
    }
    if(rightN){
      tmpR = *rightN;
      rightN = &tmpR;
    }
    if(nextHs.IsLeftDomPoint()){
      if(member){ // found an overlapping segment
        if(member->getOwner()==current.getOwner() ||
           member->getOwner()==avlseg::both){ // same source
          double xm = member->getX2();
          double xc = current.getX2();
          if(!AlmostEqual(xm,xc) && (xm<xc)){ // current extends member
            current.splitAtRight(xm,member->getY2(),right1);
            insertEvents(right1,true,true,q1,q2);
          }
        }
        else
        { // member and current come from different sources
          uint32_t parts = member->split(current,left1,common1,right1);
          sss.remove(*member);
          member = &common1;
          if(parts & avlseg::LEFT){
            if(!left1.isPoint()){
              sss.insert(left1);
              insertEvents(left1,false,true,q1,q2);
            }
          }
          assert(parts & avlseg::COMMON);
          if(!common1.isPoint()){
            sss.insert(common1);
            insertEvents(common1,false,true,q1,q2);
          }
          if(parts & avlseg::RIGHT){
            insertEvents(right1,true,true,q1,q2);
          }
        }
      }
      else
      { // no overlapping segment found
        splitByNeighbour(sss,current,leftN,q1,q2, forceThrow);
        splitByNeighbour(sss,current,rightN,q1,q2, forceThrow);
        if(!current.isPoint()){
          sss.insert(current);
          insertEvents(current,false,true,q1,q2);
        }
      }
    }
    else
    { // nextHS rightDomPoint
      if(member && member->exactEqualsTo(current)){
        // insert the segments into the result
        if(member->getOwner()==avlseg::both){
          HalfSegment hs1 = member->convertToExtendedHs(true,avlseg::first);
          hs1.attr.edgeno = edgeno;
          result += hs1;
          hs1.SetLeftDomPoint(false);
          result += hs1;
          edgeno++;
        }
        sss.remove(*member);
        splitNeighbours(sss,leftN,rightN,q1,q2, forceThrow);
      }
    }
  }
  result.EndBulkLoad();
}

/*
line x sline -> line

applicable for difference  and union

*/
template<template<typename T> class Array>
void SetOp(const LineT<Array>& line1, 
           const SimpleLineT<Array>& line2, 
           LineT<Array>& result,
           avlseg::SetOperation op, const Geoid* geoid =0,
           const bool forceThrow=false){
  result.Clear();
  if(geoid){
    std::cerr << __PRETTY_FUNCTION__ << ": Spherical geometry not implemented."
    << std::endl;
    assert( false ); // TODO: implement spherical geometry case
  }
  if(!line1.IsDefined() || !line2.IsDefined() ||
    (geoid && !geoid->IsDefined()) ){
    result.SetDefined(false);
    return;
  }
  result.SetDefined(true);
  if(line1.Size()==0){
    switch(op){
            case avlseg::union_op : line2.toLine(result);; return;
            case avlseg::difference_op : return; // empty line
            default : assert(false);
    }
  }
  if(line2.Size()==0){
    switch(op){
      case avlseg::union_op: result = line1; return;
      case avlseg::difference_op: result = line1; return;
      default : assert(false);
    }
  }
  std::priority_queue<avlseg::ExtendedHalfSegment,
  std::vector<avlseg::ExtendedHalfSegment>,
  std::greater<avlseg::ExtendedHalfSegment> > q1;
  std::priority_queue<avlseg::ExtendedHalfSegment,
  std::vector<avlseg::ExtendedHalfSegment>,
  std::greater<avlseg::ExtendedHalfSegment> > q2;
  avltree::AVLTree<avlseg::AVLSegment> sss;
  avlseg::ownertype owner;
  int pos1 = 0;
  int pos2 = 0;
  avlseg::ExtendedHalfSegment nextHs;
  int src = 0;

  avlseg::AVLSegment* member=0;
  avlseg::AVLSegment* leftN = 0;
  avlseg::AVLSegment* rightN = 0;

  avlseg::AVLSegment left1,right1,common1,left2,right2;
  int edgeno =0;
  avlseg::AVLSegment tmpL,tmpR;

  result.StartBulkLoad();
  while( (owner=selectNext(line1,pos1,line2,pos2, q1,q2,nextHs,src))
           != avlseg::none){
    avlseg::AVLSegment current(nextHs,owner);
    member = sss.getMember(current,leftN,rightN);
    if(leftN){
      tmpL = *leftN;
      leftN = &tmpL;
    }
    if(rightN){
      tmpR = *rightN;
      rightN = &tmpR;
    }
    if(nextHs.IsLeftDomPoint()){
      if(member){ // found an overlapping segment
        if(member->getOwner()==current.getOwner() ||
           member->getOwner()==avlseg::both){ // same source
          double xm = member->getX2();
          double xc = current.getX2();
          if(!AlmostEqual(xm,xc) && (xm<xc)){ // current extends member
            current.splitAtRight(xm,member->getY2(),right1);
            insertEvents(right1,true,true,q1,q2);
          }
        }
        else
        { // member and current come from different sources
          uint32_t parts = member->split(current,left1,common1,right1);
          sss.remove(*member);
          member = &common1;
          if(parts & avlseg::LEFT){
            if(!left1.isPoint()){
              sss.insert(left1);
              insertEvents(left1,false,true,q1,q2);
            }
          }
          assert(parts & avlseg::COMMON);
          if(!common1.isPoint()){
            sss.insert(common1);
            insertEvents(common1,false,true,q1,q2);
          }
          if(parts & avlseg::RIGHT){
            insertEvents(right1,true,true,q1,q2);
          }
        }
      }
      else
      { // no overlapping segment found
        splitByNeighbour(sss,current,leftN,q1,q2, forceThrow);
        splitByNeighbour(sss,current,rightN,q1,q2, forceThrow);
        if(!current.isPoint()){
          sss.insert(current);
          insertEvents(current,false,true,q1,q2);
        }
      }
    }
    else
    { // nextHS rightDomPoint
      if(member && member->exactEqualsTo(current)){
        // insert the segments into the result
        switch(op){
          case avlseg::union_op : {
            HalfSegment hs1 = member->convertToExtendedHs(true,avlseg::first);
            hs1.attr.edgeno = edgeno;
            result += hs1;
            hs1.SetLeftDomPoint(false);
            result += hs1;
            edgeno++;
            break;
          }

          case avlseg::difference_op :{
            if(member->getOwner()==avlseg::first){
              HalfSegment hs1 =
                member->convertToExtendedHs(true,avlseg::first);
                hs1.attr.edgeno = edgeno;
                result += hs1;
                hs1.SetLeftDomPoint(false);
                result += hs1;
                edgeno++;
            }
            break;
          }

          default : {
            assert(false);
          }
        }
        sss.remove(*member);
        splitNeighbours(sss,leftN,rightN,q1,q2, forceThrow);
      }
    }
  }
  result.EndBulkLoad(true,false);
}

/*
sline x line -> sline

applicable for intersection and difference

*/
template<template<typename T> class Array>
void SetOp(const SimpleLineT<Array>& line1, const LineT<Array>& line2,
           SimpleLineT<Array>& result,
           avlseg::SetOperation op, const Geoid* geoid =0,
            const bool forceThrow=false){
  result.Clear();

  if(geoid){
    std::cerr << __PRETTY_FUNCTION__ << ": Spherical geometry not implemented."
    << std::endl;
    assert( false ); // TODO: implement spherical geometry case
  }

  if(!line1.IsDefined() || !line2.IsDefined() ||
     (geoid && !geoid->IsDefined()) ){
    result.SetDefined(false);
    return;
  }
  result.SetDefined(true);
  if(line1.Size()==0){
    switch(op){
      case avlseg::intersection_op : return; // empty line
      case avlseg::difference_op : return; // empty line
      default : assert(false);
    }
  }
  if(line2.Size()==0){
    switch(op){
      case avlseg::intersection_op: return;
      case avlseg::difference_op: result = line1; return;
      default : assert(false);
    }
  }

  std::priority_queue<avlseg::ExtendedHalfSegment,
  std::vector<avlseg::ExtendedHalfSegment>,
  std::greater<avlseg::ExtendedHalfSegment> > q1;
  std::priority_queue<avlseg::ExtendedHalfSegment,
  std::vector<avlseg::ExtendedHalfSegment>,
  std::greater<avlseg::ExtendedHalfSegment> > q2;
  avltree::AVLTree<avlseg::AVLSegment> sss;
  avlseg::ownertype owner;
  int pos1 = 0;
  int pos2 = 0;
  avlseg::ExtendedHalfSegment nextHs;
  int src = 0;

  avlseg::AVLSegment* member=0;
  avlseg::AVLSegment* leftN = 0;
  avlseg::AVLSegment* rightN = 0;

  avlseg::AVLSegment left1,right1,common1, left2,right2;

  int edgeno =0;
  avlseg::AVLSegment tmpL,tmpR;

  result.StartBulkLoad();
  while( (owner=selectNext(line1,pos1,line2,pos2,q1,q2,nextHs,src))
          != avlseg::none){
    avlseg::AVLSegment current(nextHs,owner);
    member = sss.getMember(current,leftN,rightN);
    if(leftN){
      tmpL = *leftN;
      leftN = &tmpL;
    }
    if(rightN){
      tmpR = *rightN;
      rightN = &tmpR;
    }
    if(nextHs.IsLeftDomPoint()){
      if(member){ // found an overlapping segment
        if(member->getOwner()==current.getOwner() ||
           member->getOwner()==avlseg::both){ // same source
          double xm = member->getX2();
          double xc = current.getX2();
          if(!AlmostEqual(xm,xc) && (xm<xc)){ // current extends member
            current.splitAtRight(xm,member->getY2(),right1);
            insertEvents(right1,true,true,q1,q2);
          }
        }
        else
        { // member and current come from different sources
          uint32_t parts = member->split(current,left1,common1,right1);
          sss.remove(*member);
          member = &common1;
          if(parts & avlseg::LEFT){
            if(!left1.isPoint()){
              sss.insert(left1);
              insertEvents(left1,false,true,q1,q2);
            }
          }
          assert(parts & avlseg::COMMON);
          if(!common1.isPoint()){
            sss.insert(common1);
            insertEvents(common1,false,true,q1,q2);
          }
          if(parts & avlseg::RIGHT){
            insertEvents(right1,true,true,q1,q2);
          }
        }
      }
      else
      { // no overlapping segment found
        splitByNeighbour(sss,current,leftN,q1,q2, forceThrow);
        splitByNeighbour(sss,current,rightN,q1,q2, forceThrow);
        if(!current.isPoint()){
          sss.insert(current);
          insertEvents(current,false,true,q1,q2);
        }
      }
    }
    else
    { // nextHS rightDomPoint
      if(member && member->exactEqualsTo(current)){
        // insert the segments into the result
        switch(op){
          case avlseg::intersection_op : {
            if(member->getOwner()==avlseg::both){
              HalfSegment hs1 = member->convertToExtendedHs(true,avlseg::first);
              hs1.attr.edgeno = edgeno;
              result += hs1;
              hs1.SetLeftDomPoint(false);
              result += hs1;
              edgeno++;
            }
            break;
          }

          case avlseg::difference_op :{
            if(member->getOwner()==avlseg::first){
              HalfSegment hs1 = member->convertToExtendedHs(true,avlseg::first);
              hs1.attr.edgeno = edgeno;
              result += hs1;
              hs1.SetLeftDomPoint(false);
              result += hs1;
              edgeno++;
            }
            break;
          }

          default : {
            assert(false);
          }
        }
        sss.remove(*member);
        splitNeighbours(sss,leftN,rightN,q1,q2, forceThrow);
      }
    }
  }
  result.EndBulkLoad();
}

/*
sline x line -> line

for union only

*/


template<template<typename T> class Array>
void SetOp(const SimpleLineT<Array>& line1, 
           const LineT<Array>& line2, LineT<Array>& result,
           avlseg::SetOperation op, const Geoid* geoid=0,
           const bool forceThrow=false)
{
  result.Clear();
  if(geoid){
    std::cerr << __PRETTY_FUNCTION__ << ": Spherical geometry not implemented."
    << std::endl;
    assert( false ); // TODO: implement spherical geometry case
  }
  if(!line1.IsDefined() || !line2.IsDefined() ||
     (geoid && !geoid->IsDefined()) ){
    result.SetDefined(false);
    return;
  }
  result.SetDefined(true);
  if(line1.Size()==0){
    result = line2;
    return;
  }
  if(line2.Size()==0){
    line1.toLine(result);
    return;
  }

  std::priority_queue<avlseg::ExtendedHalfSegment,
  std::vector<avlseg::ExtendedHalfSegment>,
  std::greater<avlseg::ExtendedHalfSegment> > q1;
  std::priority_queue<avlseg::ExtendedHalfSegment,
  std::vector<avlseg::ExtendedHalfSegment>,
  std::greater<avlseg::ExtendedHalfSegment> > q2;
  avltree::AVLTree<avlseg::AVLSegment> sss;
  avlseg::ownertype owner;
  int pos1 = 0;
  int pos2 = 0;
  avlseg::ExtendedHalfSegment nextHs;
  int src = 0;

  avlseg::AVLSegment* member=0;
  avlseg::AVLSegment* leftN = 0;
  avlseg::AVLSegment* rightN = 0;
  avlseg::AVLSegment left1,right1,common1,left2,right2;

  int edgeno =0;
  avlseg::AVLSegment tmpL,tmpR;

  result.StartBulkLoad();
  while( (owner=selectNext(line1,pos1,line2,pos2,q1,q2,nextHs,src))
          != avlseg::none){
    avlseg::AVLSegment current(nextHs,owner);
    member = sss.getMember(current,leftN,rightN);
    if(leftN){
      tmpL = *leftN;
      leftN = &tmpL;
    }
    if(rightN){
      tmpR = *rightN;
      rightN = &tmpR;
    }
    if(nextHs.IsLeftDomPoint()){
      if(member){ // found an overlapping segment
        if(member->getOwner()==current.getOwner() ||
           member->getOwner()==avlseg::both){ // same source
          double xm = member->getX2();
          double xc = current.getX2();
          if(!AlmostEqual(xm,xc) && (xm<xc)){ // current extends member
            current.splitAtRight(xm,member->getY2(),right1);
            insertEvents(right1,true,true,q1,q2);
          }
        }
        else
        { // member and current come from different sources
          uint32_t parts = member->split(current,left1,common1,right1);
          sss.remove(*member);
          member = &common1;
          if(parts & avlseg::LEFT){
            if(!left1.isPoint()){
              sss.insert(left1);
              insertEvents(left1,false,true,q1,q2);
            }
          }
          assert(parts & avlseg::COMMON);
          if(!common1.isPoint()){
            sss.insert(common1);
            insertEvents(common1,false,true,q1,q2);
          }
          if(parts & avlseg::RIGHT){
            insertEvents(right1,true,true,q1,q2);
          }
        }
      }
      else
      { // no overlapping segment found
        splitByNeighbour(sss,current,leftN,q1,q2, forceThrow);
        splitByNeighbour(sss,current,rightN,q1,q2, forceThrow);
        if(!current.isPoint()){
          sss.insert(current);
          insertEvents(current,false,true,q1,q2);
        }
      }
    }
    else
    { // nextHS rightDomPoint
      if(member && member->exactEqualsTo(current)){
         // insert the segments into the result
        HalfSegment hs1 = member->convertToExtendedHs(true,avlseg::first);
        hs1.attr.edgeno = edgeno;
        result += hs1;
        hs1.SetLeftDomPoint(false);
        result += hs1;
        edgeno++;
      }
      sss.remove(*member);
      splitNeighbours(sss,leftN,rightN,q1,q2, forceThrow);
    }
  }
  result.EndBulkLoad(true,false);
}

/*
sline x sline -> sline

for ~intersection~ and ~minus~

*/
template<template<typename T> class Array>
void SetOp(const SimpleLineT<Array>& line1, 
           const SimpleLineT<Array>& line2, 
           SimpleLineT<Array>& result,
           avlseg::SetOperation op, const Geoid* geoid =0,
           const bool forceThrow=false)
{
  result.Clear();

  if(geoid){
    std::cerr << __PRETTY_FUNCTION__ << ": Spherical geometry not implemented."
    << std::endl;
    assert( false ); // TODO: implement spherical geometry case
  }
  if(!line1.IsDefined() || !line2.IsDefined() ||
     (geoid && !geoid->IsDefined()) ){
    result.SetDefined(false);
    return;
  }
  result.SetDefined(true);
  if(line1.Size()==0){
    switch(op){
      case avlseg::intersection_op : return; // empty line
      case avlseg::difference_op : return; // empty line
      default : assert(false);
    }
  }
  if(line2.Size()==0){
    switch(op){
      case avlseg::intersection_op: return;
      case avlseg::difference_op: result = line1; return;
      default : assert(false);
    }
  }

  std::priority_queue<avlseg::ExtendedHalfSegment,
  std::vector<avlseg::ExtendedHalfSegment>,
  std::greater<avlseg::ExtendedHalfSegment> > q1;
  std::priority_queue<avlseg::ExtendedHalfSegment,
  std::vector<avlseg::ExtendedHalfSegment>,
  std::greater<avlseg::ExtendedHalfSegment> > q2;
  avltree::AVLTree<avlseg::AVLSegment> sss;
  avlseg::ownertype owner;
  int pos1 = 0;
  int pos2 = 0;
  avlseg::ExtendedHalfSegment nextHs;
  int src = 0;

  avlseg::AVLSegment* member=0;
  avlseg::AVLSegment* leftN = 0;
  avlseg::AVLSegment* rightN = 0;

  avlseg::AVLSegment left1,right1,common1, left2,right2;

  int edgeno =0;
  avlseg::AVLSegment tmpL,tmpR;

  result.StartBulkLoad();
  while( (owner=selectNext(line1,pos1,line2,pos2,q1,q2,nextHs,src))
         != avlseg::none){
    avlseg::AVLSegment current(nextHs,owner);
    member = sss.getMember(current,leftN,rightN);
    if(leftN){
      tmpL = *leftN;
      leftN = &tmpL;
    }
    if(rightN){
      tmpR = *rightN;
      rightN = &tmpR;
    }
    if(nextHs.IsLeftDomPoint()){
      if(member){ // found an overlapping segment
        if(member->getOwner()==current.getOwner() ||
           member->getOwner()==avlseg::both){ // same source
          double xm = member->getX2();
          double xc = current.getX2();
          if(!AlmostEqual(xm,xc) && (xm<xc)){ // current extends member
            current.splitAtRight(xm,member->getY2(),right1);
            insertEvents(right1,true,true,q1,q2);
          }
        }
        else
        { // member and current come from different sources
          uint32_t parts = member->split(current,left1,common1,right1);
          sss.remove(*member);
          member = &common1;
          if(parts & avlseg::LEFT){
            if(!left1.isPoint()){
              sss.insert(left1);
              insertEvents(left1,false,true,q1,q2);
            }
          }
          assert(parts & avlseg::COMMON);
          if(!common1.isPoint()){
            sss.insert(common1);
            insertEvents(common1,false,true,q1,q2);
          }
          if(parts & avlseg::RIGHT){
            insertEvents(right1,true,true,q1,q2);
          }
        }
      }
      else
      { // no overlapping segment found
        splitByNeighbour(sss,current,leftN,q1,q2, forceThrow);
        splitByNeighbour(sss,current,rightN,q1,q2, forceThrow);
        if(!current.isPoint()){
          sss.insert(current);
          insertEvents(current,false,true,q1,q2);
        }
      }
    }
    else
    { // nextHS rightDomPoint
      if(member && member->exactEqualsTo(current)){
        // insert the segments into the result
        switch(op){
          case avlseg::intersection_op : {
            if(member->getOwner()==avlseg::both){
              HalfSegment hs1 = member->convertToExtendedHs(true,avlseg::first);
              hs1.attr.edgeno = edgeno;
              result += hs1;
              hs1.SetLeftDomPoint(false);
              result += hs1;
              edgeno++;
            }
            break;
          }

          case avlseg::difference_op :{
            if(member->getOwner()==avlseg::first){
              HalfSegment hs1 = member->convertToExtendedHs(true,avlseg::first);
              hs1.attr.edgeno = edgeno;
              result += hs1;
              hs1.SetLeftDomPoint(false);
              result += hs1;
              edgeno++;
            }
            break;
          }

          default : {
            assert(false);
          }
        }
        sss.remove(*member);
        splitNeighbours(sss,leftN,rightN,q1,q2,forceThrow);
      }
    }
  }
  result.EndBulkLoad();
}

/*
sline x sline -> line

for union only

*/
template<template<typename T> class Array>
void SetOp(const SimpleLineT<Array>& line1, 
           const SimpleLineT<Array>& line2, 
           LineT<Array>& result,
           avlseg::SetOperation op, const Geoid* geoid =0,
           const bool forceThrow=false){
  result.Clear();

  if(geoid){
    std::cerr << __PRETTY_FUNCTION__ << ": Spherical geometry not implemented."
    << std::endl;
    assert( false ); // TODO: implement spherical geometry case
  }

  if(!line1.IsDefined() || !line2.IsDefined() ||
     (geoid && !geoid->IsDefined()) ){
    result.SetDefined(false);
    return;
  }
  result.SetDefined(true);
  if(line1.Size()==0){
    line2.toLine(result);
    return;
  }
  if(line2.Size()==0){
    line1.toLine(result);
    return;
  }

  std::priority_queue<avlseg::ExtendedHalfSegment,
  std::vector<avlseg::ExtendedHalfSegment>,
  std::greater<avlseg::ExtendedHalfSegment> > q1;
  std::priority_queue<avlseg::ExtendedHalfSegment,
  std::vector<avlseg::ExtendedHalfSegment>,
  std::greater<avlseg::ExtendedHalfSegment> > q2;
  avltree::AVLTree<avlseg::AVLSegment> sss;
  avlseg::ownertype owner;
  int pos1 = 0;
  int pos2 = 0;
  avlseg::ExtendedHalfSegment nextHs;
  int src = 0;

  avlseg::AVLSegment* member=0;
  avlseg::AVLSegment* leftN = 0;
  avlseg::AVLSegment* rightN = 0;

  avlseg::AVLSegment left1,right1,common1, left2,right2;

  int edgeno =0;
  avlseg::AVLSegment tmpL,tmpR;

  result.StartBulkLoad();
  while( (owner=selectNext(line1,pos1, line2,pos2,q1,q2,nextHs,src))
          != avlseg::none){
    avlseg::AVLSegment current(nextHs,owner);
    member = sss.getMember(current,leftN,rightN);
    if(leftN){
      tmpL = *leftN;
      leftN = &tmpL;
    }
    if(rightN){
      tmpR = *rightN;
      rightN = &tmpR;
    }
    if(nextHs.IsLeftDomPoint()){
      if(member){ // found an overlapping segment
        if(member->getOwner()==current.getOwner() ||
           member->getOwner()==avlseg::both){ // same source
          double xm = member->getX2();
          double xc = current.getX2();
          if(!AlmostEqual(xm,xc) && (xm<xc)){ // current extends member
            current.splitAtRight(xm,member->getY2(),right1);
            insertEvents(right1,true,true,q1,q2);
          }
        }
        else
        { // member and current come from different sources
          uint32_t parts = member->split(current,left1,common1,right1);
          sss.remove(*member);
          member = &common1;
          if(parts & avlseg::LEFT){
            if(!left1.isPoint()){
              sss.insert(left1);
              insertEvents(left1,false,true,q1,q2);
            }
          }
          assert(parts & avlseg::COMMON);
          if(!common1.isPoint()){
            sss.insert(common1);
            insertEvents(common1,false,true,q1,q2);
          }
          if(parts & avlseg::RIGHT){
            insertEvents(right1,true,true,q1,q2);
          }
        }
      }
      else
      { // no overlapping segment found
        splitByNeighbour(sss,current,leftN,q1,q2, forceThrow);
        splitByNeighbour(sss,current,rightN,q1,q2, forceThrow);
        if(!current.isPoint()){
          sss.insert(current);
          insertEvents(current,false,true,q1,q2);
        }
      }
    }
    else
    { // nextHS rightDomPoint
      if(member && member->exactEqualsTo(current)){
        // insert the segments into the result
        HalfSegment hs1 = member->convertToExtendedHs(true,avlseg::first);
        hs1.attr.edgeno = edgeno;
        result += hs1;
        hs1.SetLeftDomPoint(false);
        result += hs1;
        edgeno++;
      }
      sss.remove(*member);
      splitNeighbours(sss,leftN,rightN,q1,q2,forceThrow);
    }
  }
  result.EndBulkLoad(true,false);
}

/*
region x sline -> region

for union and difference the result is always the given region.

*/
template<template<typename T> class Array>
void SetOp(const RegionT<Array>& region, 
           const SimpleLineT<Array>& line, 
           RegionT<Array>& result,
           avlseg::SetOperation op, const Geoid* geoid =0){
  assert(op == avlseg::union_op || op == avlseg::difference_op);
  result.Clear();
  if(!line.IsDefined() || !region.IsDefined() ||
    (geoid && !geoid->IsDefined()) ){
    result.SetDefined(false);
    return;
  }
  result.SetDefined(true);
  result.CopyFrom(&region);
}

/*
sline x region -> sline

for intersection and difference only.

*/
template<template<typename T> class Array>
void SetOp(const SimpleLineT<Array>& line, 
           const RegionT<Array>& region, SimpleLineT<Array>& result,
           avlseg::SetOperation op, const Geoid* geoid =0,
           const bool forceThrow = false){
  assert(op==avlseg::intersection_op || op == avlseg::difference_op);
  if(geoid){
    std::cerr << __PRETTY_FUNCTION__ << ": Spherical geometry not implemented."
        << std::endl;
    assert( false ); // TODO: implement spherical geometry case
  }
  result.Clear();
  if(!line.IsDefined() || !region.IsDefined() ||
      (geoid && !geoid->IsDefined()) ){
    result.SetDefined(false);
    return;
  }
  result.SetDefined(true);
  if(line.Size()==0){ // empty line -> empty result
    switch(op){
      case avlseg::intersection_op : return; // empty region
      case avlseg::difference_op : return; // empty region
      default : assert(false);
    }
  }
  if(region.Size()==0){
    switch(op){
      case avlseg::intersection_op: return;
      case avlseg::difference_op: result = line; return;
      default : assert(false);
    }
  }
  std::priority_queue<avlseg::ExtendedHalfSegment,
  std::vector<avlseg::ExtendedHalfSegment>,
  std::greater<avlseg::ExtendedHalfSegment> > q1;
  std::priority_queue<avlseg::ExtendedHalfSegment,
  std::vector<avlseg::ExtendedHalfSegment>,
  std::greater<avlseg::ExtendedHalfSegment> > q2;
  avltree::AVLTree<avlseg::AVLSegment> sss;
  avlseg::ownertype owner;
  int pos1 = 0;
  int pos2 = 0;
  int size1= line.Size();
  avlseg::ExtendedHalfSegment nextHs;
  int src = 0;

  avlseg::AVLSegment* member=0;
  avlseg::AVLSegment* leftN = 0;
  avlseg::AVLSegment* rightN = 0;

  avlseg::AVLSegment left1,right1,common1,left2,right2;

  int edgeno =0;
  avlseg::AVLSegment tmpL,tmpR;
  bool done = false;

  result.StartBulkLoad();
  // perform a planesweeo
  while(((owner=selectNext(line,pos1,region,pos2,q1,q2,nextHs,src))
          != avlseg::none) && ! done){
    avlseg::AVLSegment current(nextHs,owner);
    member = sss.getMember(current,leftN,rightN);
    if(leftN){
      tmpL = *leftN;
      leftN = &tmpL;
    }
    if(rightN){
      tmpR = *rightN;
      rightN = &tmpR;
    }
    if(nextHs.IsLeftDomPoint()){
      if(member){ // there is an overlapping segment in sss
        if(member->getOwner()==owner ||
           member->getOwner()==avlseg::both     ){
          if(current.ininterior(member->getX2(),member->getY2())){
             current.splitAtRight(member->getX2(),member->getY2(),right1);
            insertEvents(right1,true,true,q1,q2);
          }
        }
        else
        { // member and source come from difference sources
          uint32_t parts = member->split(current,left1,common1,right1);
          sss.remove(*member);
          member = &common1;
          if(parts & avlseg::LEFT){
            if(!left1.isPoint()){
              sss.insert(left1);
              insertEvents(left1,false,true,q1,q2);
            }
          }
          assert(parts & avlseg::COMMON);
          if(owner==avlseg::second) {  // the region
            if(current.getInsideAbove()){
              common1.con_above++;
            }
            else
            {
              common1.con_above--;
            }
          } // for a line is nothing to do
          if(!common1.isPoint()){
            sss.insert(common1);
            insertEvents(common1,false,true,q1,q2);
          }
          if(parts & avlseg::RIGHT){
            insertEvents(right1,true,true,q1,q2);
          }
        }
      }
      else
      { // no overlapping segment in sss found
        splitByNeighbour(sss,current,leftN,q1,q2, forceThrow);
        splitByNeighbour(sss,current,rightN,q1,q2, forceThrow);
        // update coverage numbers
        if(owner==avlseg::second){ // the region
          bool iac = current.getInsideAbove();
          if(leftN && current.extends(*leftN)){
            current.con_below = leftN->con_below;
            current.con_above = leftN->con_above;
          }
          else
          {
            if(leftN && leftN->isVertical()){
              current.con_below = leftN->con_below;
            } else
              if(leftN){
                current.con_below = leftN->con_above;
              }
              else
              {
                current.con_below = 0;
              }
            if(iac){
              current.con_above = current.con_below+1;
            }
            else
            {
              current.con_above = current.con_below-1;
            }
          }
        }
        else
        { // the line
          if(leftN){
            if(leftN->isVertical()){
              current.con_below = leftN->con_below;
            }
            else
            {
              current.con_below = leftN->con_above;
            }
          }
          current.con_above = current.con_below;
        }
        // insert element
        if(!current.isPoint()){
          sss.insert(current);
          insertEvents(current,false,true,q1,q2);
        }
      }
    }
    else
    { // nextHs.IsRightDomPoint()
      if(member && member->exactEqualsTo(current)){
        switch(op){
          case avlseg::intersection_op: {
            if( (member->getOwner()==avlseg::both) ||
                (member->getOwner()==avlseg::first && member->con_above>0)){
              HalfSegment hs1 = member->convertToExtendedHs(true,avlseg::first);
              hs1.attr.edgeno = edgeno;
              result += hs1;
              hs1.SetLeftDomPoint(false);
              result += hs1;
              edgeno++;
            }
            break;
          }

          case avlseg::difference_op: {
            if( (member->getOwner()==avlseg::first) &&
                (member->con_above==0)){
              HalfSegment hs1 = member->convertToExtendedHs(true,avlseg::first);
              hs1.attr.edgeno = edgeno;
              result += hs1;
              hs1.SetLeftDomPoint(false);
              result += hs1;
              edgeno++;
            }
            break;
          }

          default : assert(false);
        }
        sss.remove(*member);
        splitNeighbours(sss,leftN,rightN,q1,q2,forceThrow);
      }
      if(pos1>=size1 && q1.empty()){ // line is processed
        done = true;
      }
    }
  }
  result.EndBulkLoad();
}

#endif 

