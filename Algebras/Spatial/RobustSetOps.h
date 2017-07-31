
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


1 Implementation of robust versions of set operations for spatial objects

*/

#ifndef ROBUSTSETOPS_H
#define ROBUSTSETOPS_H

#include "RegionT.h"
#include "MMRTree.h"
#include "../Relation-C++/RelationAlgebra.h"



namespace robust {
/*
1.1 Intersection between region and line

Results in a line.

*/

template<template<typename T1>class Array1,
         template<typename T2>class Array2,
         template<typename T3>class Array3>
void intersection(const RegionT<Array1>& r, const LineT<Array2>& line, 
                  LineT<Array3>& result);

template<template<typename T1>class Array1,
         template<typename T2>class Array2,
         template<typename T3>class Array3>
void intersection(const LineT<Array1>& l, const RegionT<Array2>& r, 
                  LineT<Array3>& result);


/*
1.2 checks whether the region contains the point. If the point is outside or
at least one of the arguments is undefined, the result will be false. The result
is 1 if the point is inside the region and 2 if the point is onborder of the 
region.

*/
template<template<typename T>class Array>
int contains(const RegionT<Array>& reg, const Point& p);



/*
1.3 realminize

Realminize function

*/
template<template<typename T1>class Array1,
         template<typename T2>class Array2>
void realminize(const Array1<HalfSegment>& src, 
                      Array2<HalfSegment>& result);


/*
1.4 ~checkRealm~

Checks whether the given set of halfsegments is realminized.

*/

template<template<typename T>class Array>
class RealmChecker{

  public:
     RealmChecker(const Array<HalfSegment>* _hss);
     RealmChecker(const Array<HalfSegment>* _hss,
                  TupleType* _tt);

     ~RealmChecker();

     bool checkRealm();
  
     Tuple* nextTuple( const bool print = false);

     static ListExpr getTupleType(); 

     static bool isRealm(const HalfSegment& hs1,
                         const HalfSegment& hs2,
                         const bool print = false);


      static LineT<Array>* getLine(HalfSegment hs); 

  private:
     const Array<HalfSegment>* hss;
     mmrtree::RtreeT<2,int> tree;
     int pos;
     mmrtree::RtreeT<2,int>::iterator* it;
     TupleType* tt;
     HalfSegment currentHs;

     void reset(); 
    
     Tuple* createTuple(const int pos1, const int pos2,
                        const HalfSegment& hs1, 
                        const HalfSegment& hs2) const;


};


template<template<typename T1>class Array1,
         template<typename T2>class Array2,
         template<typename T3>class Array3>
void intersection(const LineT<Array1>& l1, const LineT<Array2>& l2,  
                  LineT<Array3>& result);


template<template<typename T1>class Array1,
         template<typename T2>class Array2,
         template<typename T3>class Array3>
void crossings(const LineT<Array1>& l1, const LineT<Array2>& l2, 
               PointsT<Array3>& result);


} // end of namespace robust


#include "RobustSetOpsImpl.h"


#endif


