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



#ifndef LINESPLITTERIMPL_H
#define LINESPLITTERIMPL_H


/*
10.4.29 Value mapping function for the ~polylines~ operator

*/

/*
~Constructor~

Creates a LineSplitter from the given line.

*/
template<template<typename T>class Array>
LineSplitter<Array>::LineSplitter(LineT<Array>* line, bool ignoreCriticalPoints,
                         bool allowCycles, PointsT<Array>* points /* = 0 */){
    this->theLine = line;
    size = line->Size();
    lastPos =0;
    this->points = points;
    used = new bool[size];
    memset(used,false,size);
    this->ignoreCriticalPoints = ignoreCriticalPoints;
    this->allowCycles = allowCycles;
}

/*
~Destroys~ the lineSplitter

*/

template<template<typename T>class Array>
LineSplitter<Array>::~LineSplitter(){
  delete [] used;
  points = 0;
}

/*
~NextLine~

This function extracts the next simple part of the line.
If the line is processed completely, the result will be
0. This function creates a new line instance via the new
operator. The caller of this function has to ensure the
deletion of this object.

*/
template<template<typename T>class Array>
LineT<Array>* LineSplitter<Array>::NextLine(
                   std::list<Point> *pointlist /* = 0 */) {
  // go to the first unused halfsegment
  while(lastPos<size && used[lastPos]){
    lastPos++;
  }
  if(lastPos>=size){
     return 0;
  }
  // unused segment found,  construct a new Line
  int maxSize = std::max(1,size - lastPos);
  LineT<Array> *result = 0;
  if (pointlist) {
    pointlist->clear();
  }
  else {
    result = new LineT<Array>(maxSize);
  }
  std::set<Point> pointset;
  int pos = lastPos;
  bool done = false;
  if (!pointlist) {
    result->Clear();
    result->StartBulkLoad();
  }
  HalfSegment hs1;
  HalfSegment hs2; // partner of hs1
  int edgeno = 0;
  bool seconddir = false;
  theLine->Get(pos,hs1);
  Point firstPoint = hs1.GetDomPoint();
  if (pointlist) {
    pointlist->push_back(firstPoint);
  }
  bool isCycle = false;


  while(!done){ // extension possible
    theLine->Get(pos,hs1);
    int partnerpos = hs1.GetAttr().partnerno;
    theLine->Get(partnerpos, hs2);
    Point p1 = hs1.GetDomPoint();
    pointset.insert(p1);
    Point p = hs2.GetDomPoint();
    pointset.insert(p);
    // add the halfsegments to the result
    HalfSegment Hs1 = hs1;
    HalfSegment Hs2 = hs2;
    AttrType attr1 = Hs1.GetAttr();
    attr1.edgeno = edgeno;
    Hs1.SetAttr(attr1);
    AttrType attr2 = Hs2.GetAttr();
    attr2.edgeno=edgeno;
    Hs2.SetAttr(attr2);
    edgeno++;
    if (pointlist) {
      if (seconddir) {
        pointlist->push_front(p);
      }
      else {
        pointlist->push_back(p);
      }
    }
    else {
      (*result) += (Hs1);
      (*result) += (Hs2);
    }
    // mark as used
    used[pos] = true;
    used[partnerpos] = true;

    if(isCycle){
       done = true;
    } else {
       bool found = false;
       int sp = partnerpos-1;

       if(points==0 || !points->Contains(p)){//no forced split

         // search for extension of the polyline
         // search left of partnerpos for an extension
         HalfSegment hs3;
         while(sp>0 && !found){
           if(!used[sp]){
             theLine->Get(sp,hs3);
             if(AlmostEqual(p,hs3.GetDomPoint())){
               Point p3 = hs3.GetSecPoint(); // cycles?
               if(pointset.find(p3)==pointset.end() ||
                 (allowCycles && AlmostEqual(p3,firstPoint))){
                 if(AlmostEqual(p3,firstPoint)){
                   isCycle = true;
                 }
                 found = true;
               } else {
                 sp--;
               }
             } else {
               sp = -1; // stop searching
             }
           } else {
             sp --; // search next
           }
         }
         // search on the right side
         if(!found){
            sp = partnerpos + 1;
            while(sp<size && !found){
              if(!used[sp]){
                HalfSegment hs3;
                theLine->Get(sp,hs3);
                if(AlmostEqual(p,hs3.GetDomPoint())){
                  Point p3 = hs3.GetSecPoint(); // avoid cycles
                  if(pointset.find(p3)==pointset.end() ||
                     (allowCycles && AlmostEqual(p3,firstPoint))){
                    if(AlmostEqual(p3,firstPoint)){
                      isCycle = true;
                    }
                    found = true;
                  } else {
                    sp++;
                  }
                } else {
                    sp = size; // stop searching
                }
              } else {
                sp ++; // search next
              }
            }
         }
    }

    if(found){ // sp is a potential extension of the line
      if(ignoreCriticalPoints || !isCriticalPoint(partnerpos)){
        pos = sp;
      } else {
        done = true;
      }
    }  else { // no extension found
      done = true;
    }

    if(done && !seconddir && (lastPos < (size-1)) &&
       (points==0 || !points->Contains(firstPoint)) &&
       (!isCycle)){
       // done means at this point, the line can't be extended
       // in the direction start from the first selected halfsegment.
       // but is is possible the extend the line by going into the
       // reverse direction
       seconddir = true;
       HalfSegment hs;
       theLine->Get(lastPos,hs);
       Point p = hs.GetDomPoint();
       while(lastPos<size && used[lastPos]){
         lastPos ++;
       }
       if(lastPos <size){
         theLine->Get(lastPos,hs);
         Point p2 = hs.GetDomPoint();
         if(AlmostEqual(p,p2)){
           if(pointset.find(hs.GetSecPoint())==pointset.end()){
             if(ignoreCriticalPoints || !isCriticalPoint(lastPos)){
               pos = lastPos;
               done = false;
             }
           }
         }
       }
      }
    } // isCycle
  } // while
  if (!pointlist) {
    result->EndBulkLoad();
  }
  return result;
}
/*
~isCriticalPoint~

Checks whether the dominating point of the halfsegment at
position index is a critical one meaning a junction within the
line.

*/
template<template<typename T>class Array>
bool LineSplitter<Array>::isCriticalPoint(int index){
  // check for critical point
  HalfSegment hs;
  theLine->Get(index,hs);
  Point  cpoint = hs.GetDomPoint();
  int count = 0;
  for(int i=std::max(0,index-2); i<= std::min(theLine->Size(),index+2) ; i++){
       if(i>=0 && i<size){
          theLine->Get(i,hs);
          if(AlmostEqual(cpoint, hs.GetDomPoint())){
              count++;
          }
       }
   }
   return count>2;
}



#endif


