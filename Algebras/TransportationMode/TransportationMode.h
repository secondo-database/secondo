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

//paragraph [1] Title: [{\Large \bf \begin {center}] [\end {center}}]
//[TOC] [\tableofcontents]

[1] Header File of the Transportation Mode Algebra

August, 2009 Jianqiu Xu
March, 2010 Jianqiu xu

[TOC]

1 Overview

2 Defines and includes

*/

#ifndef TransportationMode_H
#define TransportationMode_H


#include "Algebra.h"

#include "NestedList.h"

#include "QueryProcessor.h"
#include "RTreeAlgebra.h"
#include "BTreeAlgebra.h"
#include "TemporalAlgebra.h"
#include "StandardTypes.h"
#include "LogMsg.h"
#include "NList.h"
#include "RelationAlgebra.h"
#include "ListUtils.h"
#include "NetworkAlgebra.h"

#define GetCloser(a) fabs(floor(a)-a) > fabs(ceil(a)-a)? ceil(a):floor(a)

/*
for storing line or sline in such a way:
mhs1.from---mhs1.to---mhs2.from---mhs2.to

*/
struct MyHalfSegment{
  MyHalfSegment(){}
  MyHalfSegment(bool d, const Point& p1, const Point& p2):def(d),
                from(p1),to(p2){}
  MyHalfSegment(const MyHalfSegment& mhs):def(mhs.def),
                from(mhs.from),to(mhs.to){}
  MyHalfSegment& operator=(const MyHalfSegment& mhs)
  {
    def = mhs.def;
    from = mhs.from;
    to = mhs.to;
    return *this;
  }
  Point& GetLeftPoint(){return from;}
  Point& GetRightPoint(){return to;}
  void Print()
  {
    cout<<"from "<<from<<" to "<<to<<endl;
  }
  bool def;
  Point from,to;
};

/*
a point and a distance value to another point

*/
struct MyPoint{
  MyPoint(){}
  MyPoint(const Point& p, double d):loc(p), dist(d){}
  MyPoint(const MyPoint& mp):loc(mp.loc),dist(mp.dist){}
  MyPoint& operator=(const MyPoint& mp)
  {
    loc = mp.loc;
    dist = mp.dist;
    return *this;
  }
  bool operator<(const MyPoint& mp) const
  {
    return dist < mp.dist;
  }
  void Print()
  {
    cout<<"loc "<<loc<<" dist "<<dist<<endl;
  }

  Point loc;
  double dist;
};

/*
a compressed version of junction
loc--position, rid1, rid2, and relative position in each route, len1, len2

*/
struct MyJun{
  Point loc;
  int rid1;
  int rid2;
  double len1;
  double len2;
  MyJun(Point p, int r1, int r2, double l1, double l2):
       loc(p), rid1(r1), rid2(r2), len1(l1), len2(l2){}
  MyJun(const MyJun& mj):
       loc(mj.loc),rid1(mj.rid1),rid2(mj.rid2), len1(mj.len1), len2(mj.len2){}
  MyJun(){}
  MyJun& operator=(const MyJun& mj)
  {
     loc = mj.loc;
     rid1 = mj.rid1;
     rid2 = mj.rid2;
     len1 = mj.len1;
     len2 = mj.len2;
     return *this;
  }
  bool operator<(const MyJun& mj) const
  {
      return loc < mj.loc;
  }
  void Print()
  {
    cout<<"loc "<<loc<<" r1 "<<rid1<<" r2 "<<rid2<<endl;
  }
};

#endif
