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

[1] Header File of the Partition

March, 2010 Jianqiu xu

[TOC]

1 Overview

2 Defines and includes

*/

#ifndef Partition_H
#define Partition_H


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

#include <fstream>
#include <stack>
#include <vector>
#include <queue>
#include "Attribute.h"
#include "../../Tools/Flob/DbArray.h"
#include "../../Tools/Flob/Flob.h"
#include "WinUnix.h"
#include "AvlTree.h"
#include "Symbols.h"
#include "GeneralType.h"


/*
5 Auxiliary structures for plane sweep algorithms

5.1 Definition of ~ownertype~

This enumeration is used to indicate the source of an ~AVLSegment~.

*/
namespace myavlseg{

enum ownertype{none, first, second, both};

enum SetOperation{union_op, intersection_op, difference_op};

std::ostream& operator<<(std::ostream& o, const ownertype& owner);


const int LEFT      = 1;
const int RIGHT     = 2;
const int COMMON = 4;

/*
3.2 The Class ~AVLSegment~

This class is used for inserting into an avl tree during a plane sweep.


*/

class MyAVLSegment{

public:

/*
3.1.1 Constructors

~Standard Constructor~

*/
  MyAVLSegment();


/*
~Constructor~

This constructor creates a new segment from the given HalfSegment.
As owner only __first__ and __second__ are the allowed values.

*/

  MyAVLSegment(const HalfSegment& hs, ownertype owner);


/*
~Constructor~

Create a Segment only consisting of a single point.

*/

  MyAVLSegment(const Point& p, ownertype owner);


/*
~Copy Constructor~

*/
   MyAVLSegment(const MyAVLSegment& src);


/*
3.2.1 Destructor

*/
   ~MyAVLSegment() {}


/*
3.3.1 Operators

*/

  MyAVLSegment& operator=(const MyAVLSegment& src);

  bool operator==(const MyAVLSegment& s) const;

  bool operator<(const MyAVLSegment& s) const;

  bool operator>(const MyAVLSegment& s) const;

/*
3.3.1 Further Needful Functions

~Print~

This function writes this segment to __out__.

*/
  void Print(std::ostream& out)const;

/*

~Equalize~

The value of this segment is taken from the argument.

*/

  void Equalize( const MyAVLSegment& src);


/*
3.5.1 Geometric Functions

~crosses~

Checks whether this segment and __s__ have an intersection point of their
interiors.

*/
 bool crosses(const MyAVLSegment& s) const;

/*
~crosses~

This function checks whether the interiors of the related
segments are crossing. If this function returns true,
the parameters ~x~ and ~y~ are set to the intersection point.

*/
 bool crosses(const MyAVLSegment& s,double& x, double& y) const;

/*
~extends~

This function returns true, iff this segment is an extension of
the argument, i.e. if the right point of ~s~ is the left point of ~this~
and the slopes are equal.

*/
  bool extends(const MyAVLSegment& s) const;


/*
~exactEqualsTo~

This function checks if s has the same geometry like this segment, i.e.
if both endpoints are equal.

*/
bool exactEqualsTo(const MyAVLSegment& s)const;

/*
~isVertical~

Checks whether this segment is vertical.

*/

 bool isVertical() const;


/*
~isPoint~

Checks if this segment consists only of a single point.

*/
  bool isPoint() const;

/*
~length~

Returns the length of this segment.

*/
  double length();

/*
~InnerDisjoint~

This function checks whether this segment and s have at most a
common endpoint.

*/

  bool innerDisjoint(const MyAVLSegment& s)const;

/*
~Intersects~

This function checks whether this segment and ~s~ have at least a
common point.

*/

  bool intersects(const MyAVLSegment& s)const;


/*
~overlaps~

Checks whether this segment and ~s~ have a common segment.

*/
   bool overlaps(const MyAVLSegment& s) const;

/*
~ininterior~

This function checks whether the point defined by (x,y) is
part of the interior of this segment.

*/
   bool ininterior(const double x,const  double y)const;


/*
~contains~

Checks whether the point defined by (x,y) is located anywhere on this
segment.

*/
   bool contains(const double x,const  double y)const;


/*
3.6.1 Comparison

Compares this with s. The x intervals must overlap.

*/

 int compareTo(const MyAVLSegment& s) const;

/*
~SetOwner~

This function changes the owner of this segment.

*/
  void setOwner(ownertype o);


/*
3.7.1 Some ~Get~ Functions

~getInsideAbove~

Returns the insideAbove value for such segments for which this value is unique,
e.g. for segments having owner __first__ or __second__.

*/
  bool getInsideAbove() const;


  inline double getX1() const { return x1; }

  inline double getX2() const { return x2; }

  inline double getY1() const { return y1; }

  inline double getY2() const { return y2; }

  inline ownertype getOwner() const { return owner; }

  inline bool getInsideAbove_first() const { return insideAbove_first; }

  inline bool getInsideAbove_second() const { return insideAbove_second; }


/*
3.8.1 Split Functions

~split~

This function splits two overlapping segments.
Preconditions:

1) this segment and ~s~ have to overlap.

2) the owner of this and ~s~ must be different

~left~, ~common~ and ~right~ will contain the
explicitely left part, a common part, and
an explecitely right part. The left and/or right part
my be empty. The existence can be checked using the return
value of this function. Let ret the return value. It holds:

  __ret | LEFT__: the left part exists

  __ret | COMMON__: the common part exist (always true)

  __ret | RIGHT__: the right part exists


The constants LEFT, COMMON, and RIGHT have been defined
earlier.

*/

  int split(const MyAVLSegment& s, MyAVLSegment& left, MyAVLSegment& common,
            MyAVLSegment& right, const bool checkOwner = true) const;


/*
~splitAt~

This function divides a segment into two parts at the point
provided by (x, y). The point must be on the interior of this segment.

*/

  void splitAt(const double x, const double y,
               MyAVLSegment& left,
               MyAVLSegment& right)const;


/*
~splitCross~

Splits two crossing segments into the 4 corresponding parts.
Both segments have to cross each other.

*/
void splitCross(const MyAVLSegment& s, MyAVLSegment& left1,
               MyAVLSegment& right1, MyAVLSegment& left2,
               MyAVLSegment& right2) const;


/*
3.9.1 Converting Functions

~ConvertToHs~

This functions creates a ~HalfSegment~ from this segment.
The owner must be __first__ or __second__.

*/
HalfSegment convertToHs(bool lpd, ownertype owner = both )const;


/*
3.10.1 Public Data Members

These members are not used in this class. So the user of
this class can change them without any problems within this
class itself.

*/
 int con_below;  // should be used as a coverage number
 int con_above;  // should be used as a coverage number


/*
3.11.1 Private Part

Here the data members as well as some auxiliary functions are
collected.

*/


private:
  /* data members  */
  double x1,y1,x2,y2; // the geometry of this segment
  bool insideAbove_first;
  bool insideAbove_second;
  ownertype owner;    // who is the owner of this segment


/*
~pointequal~

This function checks if the points defined by (x1, y1) and
(x2,y2) are equals using the ~AlmostEqual~ function.

*/
  static bool pointEqual(const double x1, const double y1,
                         const double x2, const double y2);


/*
~pointSmaller~

This function checks if the point defined by (x1, y1) is
smaller than the point defined by (x2, y2).

*/

 static bool pointSmaller(const double x1, const double y1,
                          const double x2, const double y2);

/*
~comparePoints~

*/
  static int comparePoints(const double x1,const  double y1,
                            const double x2,const double y2);

/*
~compareSlopes~

compares the slopes of __this__ and __s__. The slope of a vertical
segment is greater than all other slopes.

*/
   int compareSlopes(const MyAVLSegment& s) const;


/*
~XOverlaps~

Checks whether the x interval of this segment overlaps the
x interval of ~s~.

*/

  bool xOverlaps(const MyAVLSegment& s) const;


/*
~XContains~

Checks if the x coordinate provided by the parameter __x__ is contained
in the x interval of this segment;

*/
  bool xContains(const double x) const;


/*
~GetY~

Computes the y value for the specified  __x__.
__x__ must be contained in the x-interval of this segment.
If the segment is vertical, the minimum y value of this
segment is returned.

*/
  double getY(const double x) const;
};

std::ostream& operator<<(std::ostream& o, const MyAVLSegment& s);


};

/*
a smaller factor compared with original version of AlmostEqual

*/
inline bool MyAlmostEqual( const double d1, const double d2 )
{
//  const double factor = 0.00001;
  const double factor = 0.000001;

  double diff = fabs(d1-d2);
  return diff< factor;
}

/*
Moidfy the coordinate value of the point, change it into int value or
reduce the precision to be more practical

*/
#define GetCloser(a) fabs(floor(a)-a) > fabs(ceil(a)-a)? ceil(a):floor(a)

inline void ModifyPoint(Point& p)
{
    double a, b;
    int x, y;
    a = p.GetX();
    b = p.GetY();

    x = static_cast<int>(GetCloser(a));
    y = static_cast<int>(GetCloser(b));
    p.Set(x,y);


/*    double x, y;
    x = p.GetX();
    y = p.GetY();
    x = ((int)(x*10000.0 + 0.5))/10000.0;
    y = ((int)(y*10000.0 + 0.5))/10000.0;
    p.Set(x,y);*/


}
inline void Modify_Point(Point& p)
{
    double x,y;
    x = p.GetX();
    y = p.GetY();
//    printf("%.10f %.10f\n",x, y);
    x = ((int)(x*100.0 + 0.5))/100.0;
    y = ((int)(y*100.0 + 0.5))/100.0;

//    printf("%.10f %.10f\n",x, y);

    p.Set(x,y);
}
/*
keep the number of digits after dot for float or double representation

*/
inline void Modify_Point2(Point& p)
{
    double x,y;

    x = p.GetX();
    y = p.GetY();
//    printf("%.10f %.10f\n",x, y);

//    x = ((int)(x*1000.0 + 0.5))/1000.0;
//    y = ((int)(y*1000.0 + 0.5))/1000.0;
     double xx = (double)(int(x*1000.0 + 0.5))/1000;
     double yy = (double)(int(y*1000.0 + 0.5))/1000;

//    printf("%.10f %.10f\n",xx, yy);

      p.Set(xx,yy);

}

inline void Modify_Point3(Point& p, double f)
{
    double x, y;

    x = p.GetX();
    y = p.GetY();
//    printf("%.10f %.10f\n",x, y);

//    x = ((int)(x*1000.0 + 0.5))/1000.0;
//    y = ((int)(y*1000.0 + 0.5))/1000.0;
     double xx = (double)(int(x*f));
     double yy = (double)(int(y*f));

//    printf("%.10f %.10f\n",xx, yy);

      p.Set(xx, yy);

}

template< template<typename T1> class Array1,
          template<typename T2> class Array2,
          template<typename T2> class Array3 >
void MySetOp(const RegionT<Array1>& reg1, 
             const RegionT<Array2>& reg2,
             RegionT<Array3>& result,
           myavlseg::SetOperation op);

void MySetOp(const Line& line, const Region& region, Line& result,
           myavlseg::SetOperation op);
void MySetOp(const Line& line1, const Line& line2, Line& result,
             myavlseg::SetOperation op);

template< template<typename T1> class Array1,
          template<typename T2> class Array2,
          template<typename T3> class Array3>
void MyMinus(const RegionT<Array1>& reg1, 
             const RegionT<Array2>& reg2, 
             RegionT<Array3>& result);


template< template<typename T1> class Array1,
          template<typename T2> class Array2,
          template<typename T3> class Array3>
void MyIntersection(const RegionT<Array1>& reg1, 
                    const RegionT<Array2>& reg2, 
                    RegionT<Array3>& result);

void MyIntersection(const Line& line, const Region& reg, Line& result);
void MyIntersection(const Line& l1, const Line& l2, Line& result);

template< template<typename T1> class Array1,
          template<typename T2> class Array2,
          template<typename T3> class Array3>
void MyUnion(const RegionT<Array1>& reg1, 
             const RegionT<Array2>& reg2, 
             RegionT<Array3>& result);

bool MyRegIntersects(const Region* reg1, const Region* reg2);
bool MyHSIntersects(const HalfSegment* hs1, const HalfSegment* hs2);
bool MyInside(Line*,Region*);
bool RegContainHS(Region* r, HalfSegment hs);
double PointOnSline(SimpleLine* sl, Point* loc, bool b);

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
 //   cout<<"from "<<from<<" to "<<to<<endl;
    printf("(%.8f %.8f) (%.8f %.8f)\n", from.GetX(), from.GetY(),
                                        to.GetX(), to.GetY());
  }
  void Exchange()
  {
      Point temp = from;
      from = to;
      to = temp;
  }
  bool def;
  Point from,to;
};

struct MySegDist:public MyHalfSegment{
  double dist;
  MySegDist(){dist=0.0;}
  MySegDist(bool def, const Point& p1, const Point& p2, double d):
            MyHalfSegment(def,p1,p2),dist(d){}
  MySegDist(const MySegDist& msd):MyHalfSegment(msd),dist(msd.dist){}
  MySegDist& operator=(const MySegDist& msd)
  {
      MyHalfSegment::operator=(msd);
      dist = msd.dist;
      return *this;
  }
  bool operator<(const MySegDist& msd) const
  {
//    cout<<"from1 "<<from<<" to1 "<<to<<endl;
//    cout<<"from2 "<<msd.from<<" to2 "<<msd.to<<endl;
    bool result = dist < msd.dist;
//    cout<<"< "<<result<<endl;
    return result;
  }

  bool operator==(const MySegDist& msd) const
  {
//    cout<<" == "<<endl;
    if(AlmostEqual(from, msd.from) && AlmostEqual(to, msd.to) &&
       AlmostEqual(dist, msd.dist)) return true;
    if(AlmostEqual(to, msd.from) && AlmostEqual(from, msd.to) &&
       AlmostEqual(dist, msd.dist)) return true;
    return false;
  }
};
std::ostream& operator<<(std::ostream& o, const MySegDist& seg);

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

struct MyPoint_Ext:public MyPoint{
    Point loc2; 
    double dist2;
    MyPoint_Ext(){}
    MyPoint_Ext(const Point& p1, const Point& p2, double d1, double d2):
    MyPoint(p1,d1),loc2(p2),dist2(d2){}
    MyPoint_Ext(const MyPoint_Ext& mpe):
    MyPoint(mpe),loc2(mpe.loc2), dist2(mpe.dist2){}
    MyPoint_Ext& operator=(const MyPoint_Ext& mpe)
    {
        MyPoint::operator=(mpe);
        loc2 = mpe.loc2; 
        dist2 = mpe.dist2;
        return *this; 
    }
     void Print()
      {
        cout<<" loc1 " <<loc<<"loc2 "<<loc2
            <<" dist1 "<<dist<<" dist2 "<<dist2<<endl; 
      }
};


/* structure for rotational plane sweep */
struct RPoint{
  Point p;
  double angle;
  Point n1, n2;
  double dist;
  int regid;
  RPoint(){}
  RPoint(Point& q, double a, double d, int id):p(q),angle(a),dist(d),regid(id){}
  RPoint(const RPoint& rp):p(rp.p),angle(rp.angle),
                           n1(rp.n1), n2(rp.n2), dist(rp.dist),regid(rp.regid){}
  RPoint& operator=(const RPoint& rp)
  {
    p = rp.p;
    angle = rp.angle;
    n1 = rp.n1;
    n2 = rp.n2;
    dist = rp.dist;
    regid = rp.regid;
    return *this;
  }
  void SetNeighbor(Point& p1, Point& p2)
  {
    n1 = p1;
    n2 = p2;
  }
  bool operator<(const RPoint& rp) const
  {
    if(AlmostEqual(angle,rp.angle)){
        return dist > rp.dist;
    }else
      return angle > rp.angle;
  }
  void Print()
  {
//    cout<<" n1 "<<n1<<" n2 "<<n2<<endl;
//    cout<<"p "<<p<<" angle "<<angle<<"dist "<<dist<<endl;
    cout<<"p "<<p<<"angle "<<angle<<endl;
  }

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

struct Region_Oid{
  int oid;
  Region reg;
  Region_Oid(){}
  Region_Oid(int id, Region& r):oid(id),reg(r){}
  Region_Oid(const Region_Oid& ro):oid(ro.oid), reg(ro.reg){}
  Region_Oid& operator=(const Region_Oid& ro)
  {
     oid = ro.oid;
     reg = ro.reg;
     return *this;
  }
  void Print()
  {
    cout<<"id "<<oid<<" reg "<<reg<<endl;
  }
};
/*
struct for partition space

*/

struct SpacePartition{
  Relation* l;
  TupleType* resulttype;
  unsigned int count;
  std::vector<int> junid1;
  std::vector<int> junid2;
  std::vector<Region> outer_regions_s;
  std::vector<Region> outer_regions1;
  std::vector<Region> outer_regions2;
  std::vector<Region> outer_regions_l;
  std::vector<Region> outer_regions4;
  std::vector<Region> outer_regions5;

  std::vector<Region> outer_fillgap1;
  std::vector<Region> outer_fillgap2;
  std::vector<Region> outer_fillgap;

  std::vector<Line> pave_line1;
  std::vector<Line> pave_line2;
  /////////////function implementation//////////////////////////////
  SpacePartition();
  SpacePartition(Relation* in_line);

  //get the intersection value between a circle and a line function
  void GetDeviation(Point, double, double, double&, double&, int);
  //check the rotation from (p1-p0) to (p2-p0) is clock_wise or not
  bool GetClockwise(Point& p0,Point& p1, Point& p2);
  //get the angle of the rotation from (p1-p0) to (p2-p0)
  double GetAngle(Point& p0,Point& p1, Point& p2);
  //move the segment by a deviation to the left or right
  void TransferSegment(MyHalfSegment&, std::vector<MyHalfSegment>&, int, bool);


  //add the segment to line, but change its points coordinate to int value
  void AddHalfSegmentResult(MyHalfSegment hs, Line* res, int& edgeno);
  //for the given line stored in segs, get the line after transfer
  void Gettheboundary(std::vector<MyHalfSegment>& segs,
                      std::vector<MyHalfSegment>& boundary, int delta,
                      bool clock_wise);
  //get all the points forming the boundary for road region
  void ExtendSeg1(std::vector<MyHalfSegment>& segs,int delta,
                bool clock_wise,
                std::vector<Point>& outer, std::vector<Point>& outer_half);
 //get all the points forming the boundary for both road and pavement region
  void ExtendSeg2(std::vector<MyHalfSegment>& segs,int delta,
                     bool clock_wise, std::vector<Point>& outer);

 //translate a given line with a distance limitation 
  void ExtendSeg3(std::vector<MyHalfSegment>& segs,int delta,
                     bool clock_wise, std::vector<Point>& outer);

  // order the segments so that the end point of last one connects to the start
  // point of the next one, the result is stored as std::vector<MyHalfSegment>

  void ReorderLine(SimpleLine*, std::vector<MyHalfSegment>&);
  //create a region from the given set of ordered points
  template<template<typename T> class Array>
  void ComputeRegion(std::vector<Point>& outer_region, 
                     std::vector<RegionT<Array> >& regs);


  bool CheckRegionPS(std::vector<Point>& outer_region);
  //extend each road to a region
  void ExtendRoad(int attr_pos, int w);
  //remove triangle area after cutting
  void FilterDirtyRegion(std::vector<Region>& regs, Region* reg);
  //cut the intersection region between pavement and road
  void ClipPaveRegion(Region& reg,
                       std::vector<Region>& paves,int rid, Region* inborder);
  //fill the gap between two pavements at some junction positions
  void FillPave(network::Network* n, std::vector<Region>& pavements1,
                std::vector<Region>& pavements2,
                std::vector<double>& routes_length,
                std::vector<Region>& paves1, std::vector<Region>& paves2);
  //get the pavement beside each road
  void Getpavement(network::Network* n, Relation* rel1, int attr_pos,
                  Relation* rel2, int attr_pos1, int attr_pos2, int w);
  //get the closest point in hs to p and return the point and distance
  double GetClosestPoint(HalfSegment& hs, Point& p, Point& cp);
  double GetClosestPoint_New(HalfSegment& hs, Point& p, Point& cp);

  //transfer the halfsegment by a deviation
  void TransferHalfSegment(HalfSegment& hs, int delta, bool flag);
  //for the given line, get its curve after transfer
  template< template<typename T1> class Array1,
          template<typename T2> class Array2>
  void GetSubCurve(SimpleLineT<Array1>* curve, 
                   LineT<Array2>* newcurve,
                   int roadwidth, 
                   bool clock);

  //build zebra crossing at junction position, called by GetZebraCrossing()
  template< template<typename T1> class Array1,
            template<typename T2> class Array2, 
            template<typename T3> class Array3, 
            template<typename T4> class Array4,
            template<typename T5> class Array5> 
  bool BuildZebraCrossing(std::vector<MyPoint>& endpoints1,
                          std::vector<MyPoint>& endpoints2,
                          RegionT<Array1>* reg_pave1, 
                          RegionT<Array2>* reg_pave2,
                          LineT<Array3>* pave1, 
                          RegionT<Array4>* crossregion,
                          Point& junp, 
                          RegionT<Array5>* last_zc);
//for the road line around the junction position, it creates the zebra crossing
template< template<typename T1> class Array1,
          template<typename T2> class Array2,
          template<typename T3> class Array3,
          template<typename T4> class Array4,
          template<typename T5> class Array5,
          template<typename T6> class Array6>
  void GetZebraCrossing(SimpleLineT<Array1>* subcurve,
                        RegionT<Array2>* reg_pave1, RegionT<Array3>* reg_pave2,
                        int roadwidth, LineT<Array4>* pave1, double delta_l,
                        Point p1, RegionT<Array5>* crossregion, 
                        RegionT<Array6>* last_zc);


   // Extend the line in decreasing direction
template< template<typename T1> class Array1,
          template<typename T2> class Array2,
          template<typename T3> class Array3,
          template<typename T4> class Array4,
          template<typename T5> class Array5,
          template<typename T6> class Array6>
  void Decrease(SimpleLineT<Array1>* curve, RegionT<Array2>* reg_pave1,
                      RegionT<Array3>* reg_pave2, double len, 
                      LineT<Array4>* pave,
                      int roadwidth, RegionT<Array5>* crossregion, 
                      RegionT<Array6>* last_zc);
  //Extend the line in increasing direction
template< template<typename T1> class Array1,
          template<typename T2> class Array2,
          template<typename T3> class Array3,
          template<typename T4> class Array4,
          template<typename T5> class Array5,
          template<typename T6> class Array6>
  void Increase(SimpleLineT<Array1>* curve, RegionT<Array2>* reg_pave1,
                      RegionT<Array3>* reg_pave2, double len, 
                      LineT<Array4>* pave,
                      int roadwidth, RegionT<Array5>* crossregion, 
                      RegionT<Array6>* last_zc);
  //create the pavement at each junction position
template< template<typename T1> class Array1,
          template<typename T2> class Array2,
          template<typename T3> class Array3,
          template<typename T4> class Array4,
          template<typename T5> class Array5,
          template<typename T6> class Array6>
  void CreatePavement(SimpleLineT<Array1>* curve, RegionT<Array2>* reg_pave1,
                      RegionT<Array3>* reg_pave2, double len, 
                      int roadwidth, RegionT<Array4>* crossregion1,
                      RegionT<Array5>* crossregion2, RegionT<Array6>* last_zc);

  //cut the common pavements of two roads at the junction position
  void DecomposePave(Region* reg1, Region* reg2, std::vector<Region>& result);
  void GetCommPave1(std::vector<Region_Oid>& pave1,
                    std::vector<Region_Oid>& pave2, int,int);
  void GetCommPave2(Region* reg, int, std::vector<Region_Oid>& pave2);
  void DecomposePavement1(network::Network* n, Relation* rel,
                        int attr_pos1, int attr_pos2, int attr_pos3);
  void DecomposePavement2(int start_oid, Relation* rel,
                        int attr_pos1, int attr_pos2);
  void GetPavementEdge1(network::Network*, Relation*, BTree*, int, int, int);
  void GetPavementEdge2(Relation*, Relation*, BTree*, int, int, int);

  bool RidPosExist(int rid, float pos, 
                   std::vector<std::vector<float> >& rid_pos_list);
  ///////////cut the commone area between pavements and road regions///
  void Junpavement(network::Network* n, Relation* rel, int attr_pos1,
                  int attr_pos2, int width, Relation* rel_road,int attr_pos3);

  //Detect whether three points collineation
  bool Collineation(Point& p1, Point& p2, Point& p3);
  //Check that the pavement gap should not intersect the two roads
  
  template< template<typename T1> class Array1,
          template<typename T2> class Array2,
          template<typename T3> class Array3>
  bool SameSide1(RegionT<Array1>* reg1, RegionT<Array2>* reg2, 
                 LineT<Array3>* r1r2,Point* junp);
  //build a small region around the three halfsegments
  template< template<typename T1> class Array1,
            template<typename T2> class Array2,
            template<typename T3> class Array3,
            template<typename T4> class Array4>
  bool SameSide2(RegionT<Array1>* reg1, RegionT<Array2>* reg2, 
                 LineT<Array3>* r1r2,
                 Point* junp, MyHalfSegment thirdseg, 
                 RegionT<Array4>& smallreg);

  //the common area of two regions
  template< template<typename T1> class Array1,
            template<typename T2> class Array2>
  inline bool PavementIntersection(RegionT<Array1>* reg1, 
                                   RegionT<Array2>* reg2);
  //check the junction position rids.size() != 2 rids.size() != 6
  void NewFillPavementDebug(Relation* rel, Relation* routes,
                      int id1, int id2,
                      Point* junp, int attr_pos1, int attr_pos2,
                      std::vector<int> rids);

  // check for the junction where two road intersect
  // rids.size() == 2, used by operator fillgap

  void NewFillPavement1(Relation* rel, Relation* routes,
                      int id1, int id2,
                      Point* junp, int attr_pos1, int attr_pos2,
                      std::vector<int> rids);

  //check for the junction where three roads intersect
  //called by operator fillgap

  void NewFillPavement2(Relation* rel, Relation* routes,
                      int id1, int id2,
                      Point* junp, int attr_pos1, int attr_pos2,
                      std::vector<int> rids);

  //the same function as in NewFillPavement2, but with different input
  //parameters called by function FillPave()

  void NewFillPavement3(Relation* routes, int id1, int id2,
                      Point* junp, std::vector<Region>& paves1,
                      std::vector<Region>& paves2, std::vector<int> rids,
                      std::vector<Region>& newpaves1, 
                      std::vector<Region>& newpaves2);

  //the same function as NewFillPavement2, but with different input parameters
  //called by function FillPave()

  void NewFillPavement4(Relation* routes, int id1, int id2,
                      Point* junp, std::vector<Region>& paves1,
                      std::vector<Region>& paves2, std::vector<int> rids,
                      std::vector<Region>& newpaves1, 
                      std::vector<Region>& newpaves2);
  void NewFillPavement5(Relation* routes, int id1, int id2,
                      Point* junp, std::vector<Region>& paves1,
                      std::vector<Region>& paves2, std::vector<int> rids,
                      std::vector<Region>& newpaves1, 
                      std::vector<Region>& newpaves2);

  //for operator fillgap
  void FillHoleOfPave(network::Network* n, Relation* rel,  int attr_pos1,
                      int attr_pos2, int width);

};


/*
for each route, it returns all possible locations where interesting points
can locate 

*/
struct StrRS{
    network::Network* n;
    Relation* r1;
    Relation* r2;
    unsigned int count;
    TupleType* resulttype;
    std::vector<int> rids; 
    std::vector<Line> lines;
    std::vector<Point> interestps;
    std::vector<Point> ps; 
    std::vector<bool> ps_type; 
    StrRS();
    ~StrRS();
    StrRS(network::Network* net, Relation* rel1, Relation* rel2);
    void GetSections(int attr_pos1, int attr_pos2, int attr_pos3);
    void GenPoints1(int attr_pos1, int attr_pos2, int attr_pos3, 
                   int attr_pos4, int no_ps);
    void GenPoints2(R_Tree<2,TupleId>*, int attr_pos1, int attr_pos2, 
                    unsigned int);
    void DFTraverse(R_Tree<2,TupleId>*, SmiRecordId, Point*, int);
    void GetInterestingPoints(HalfSegment hs, Point ip, 
                              std::vector<MyPoint>& intersect_ps, Region*,
                               Region*);
};

#define TM_MYPI 3.1415927

/*
data clean process 

*/
struct DataClean{
  unsigned int count;
  TupleType* resulttype; 
  
  DataClean(){ count = 0; resulttype = NULL;} 
  ~DataClean(){if(resulttype != NULL) resulttype->DeleteIfAllowed();}

  std::vector<SimpleLine> sl_list;


  void ModifyLine(SimpleLine* in, SimpleLine* out);
  void CheckRoads(Relation* r, R_Tree<2,TupleId>* rtree);
  void DFTraverse(Relation* rel,R_Tree<2,TupleId>* rtree, SmiRecordId adr, 
                          Line* sl, std::vector<int>& id_list, unsigned int id);
  void DFTraverse2(Relation* rel,R_Tree<2,TupleId>* rtree, SmiRecordId adr, 
                          Line* sl, std::vector<int>& id_list, unsigned int id);
};

/*
Implementation of template functions.

*/

void myinsertEvents(const myavlseg::MyAVLSegment& seg,
                  const bool createLeft,
                  const bool createRight,
                  priority_queue<HalfSegment,
                                 vector<HalfSegment>,
                                 greater<HalfSegment> >& q1,
                  priority_queue<HalfSegment,
                                 vector<HalfSegment>,
                                 greater<HalfSegment> >& q2);

bool MysplitByNeighbour(avltree::AVLTree<myavlseg::MyAVLSegment>& sss,
                      myavlseg::MyAVLSegment& current,
                      myavlseg::MyAVLSegment*& neighbour,
                      priority_queue<HalfSegment,
                                     vector<HalfSegment>,
                                     greater<HalfSegment> >& q1,
                      priority_queue<HalfSegment,
                                     vector<HalfSegment>,
                                     greater<HalfSegment> >& q2);


void MysplitNeighbours(avltree::AVLTree<myavlseg::MyAVLSegment>& sss,
                     myavlseg::MyAVLSegment *& leftN,
                     myavlseg::MyAVLSegment *& rightN,
                     priority_queue<HalfSegment,
                                    vector<HalfSegment>,
                                    greater<HalfSegment> >& q1,
                     priority_queue<HalfSegment,
                                    vector<HalfSegment>,
                                    greater<HalfSegment> >& q2);


/*
The first region minuses the seccond region and store the result as the third

*/
template< template<typename T1> class Array1,
          template<typename T2> class Array2,
          template<typename T3> class Array3>
void MyMinus(const RegionT<Array1>& reg1, 
             const RegionT<Array2>& reg2, 
             RegionT<Array3>& result)
{
  MySetOp(reg1,reg2,result,myavlseg::difference_op);
}

/*
The first region unions the seccond region and store the result as the third

*/
template< template<typename T1> class Array1,
          template<typename T2> class Array2,
          template<typename T3> class Array3>
void MyUnion(const RegionT<Array1>& reg1, 
             const RegionT<Array2>& reg2, 
             RegionT<Array3>& result)
{
  MySetOp(reg1,reg2,result,myavlseg::union_op);
}

template<template<typename T> class Array>
void SpacePartition:: ComputeRegion(vector<Point>& outer_region,
                                    vector<RegionT<Array> >& regs)
{
    /////////note that points are counter_clock_wise ordered///////////////
    //for(unsigned i = 0;i < outer_region.size();i++)
    //  cout<<outer_region[i] << endl;
    
      ///////////////////////////////////////////////////////////////
      //////////// check for overlapping segments///////////////////
      //////////////////////////////////////////////////////////////
//      CheckRegionPS(outer_region);
      bool check = CheckRegionPS(outer_region);
      if(check == false) return;

      ////////////////////////////////////////////////////////////
      MMRegion* cr = new MMRegion( 0 );
      cr->StartBulkLoad();

      int fcno=-1;
      int ccno=-1;
      int edno= 0;
      int partnerno = 0;

      fcno++;
      ccno++;
      bool isCycle = true;

      Point firstPoint = outer_region[0];
      Point prevPoint = firstPoint;

      //Starting to compute a new cycle

      MMPoints *cyclepoints= new MMPoints( 8 ); 

      MMRegion *rDir = new MMRegion(32);
      rDir->StartBulkLoad();
      Point currvertex = firstPoint;

      cyclepoints->StartBulkLoad();
      *cyclepoints += currvertex;
      Point p1 = currvertex;
      Point firstP = p1;
      cyclepoints->EndBulkLoad();

      for(unsigned int i = 1;i < outer_region.size();i++){
        currvertex = outer_region[i];

//        if(cyclepoints->Contains(currvertex))assert(false);
        if(cyclepoints->Contains(currvertex))continue;

        ////////////////step -- 1/////////////////////////////
        Point p2 = currvertex;
        cyclepoints->StartBulkLoad();
        *cyclepoints += currvertex;
        cyclepoints->EndBulkLoad(true,false,false);
        /////////////step --- 2 create halfsegment/////////////////////////

        HalfSegment* hs = new HalfSegment(true, prevPoint, currvertex);
        hs->attr.faceno=fcno;
        hs->attr.cycleno=ccno;
        hs->attr.edgeno=edno;
        hs->attr.partnerno=partnerno;
        partnerno++;
        hs->attr.insideAbove = (hs->GetLeftPoint() == p1);
        ////////////////////////////////////////////////////////
        p1 = p2;
        edno++;
        prevPoint= currvertex;

        if(cr->InsertOk(*hs)){
           *cr += *hs;
//           cout<<"cr+1 "<<*hs<<endl;
           if( hs->IsLeftDomPoint()){
              *rDir += *hs;
//              cout<<"rDr+1 "<<*hs<<endl;
              hs->SetLeftDomPoint( false );
           }else{
                hs->SetLeftDomPoint( true );
//                cout<<"rDr+2 "<<*hs<<endl;
                (*rDir) += (*hs);
                }
            (*cr) += (*hs);
//            cout<<"cr+2 "<<*hs<<endl;
            delete hs;
        }else assert(false);
      }//end for

      cyclepoints->DeleteIfAllowed();
//     printf("(%.6f %.6f) (%.6f %.6f)\n", firstPoint.GetX(), firstPoint.GetY(),
//             currvertex.GetX(), currvertex.GetY());

      ////////////////////last segment//////////////////////////
      //edno++; // edgeno already increased
      HalfSegment* hs = new HalfSegment(true, firstPoint, currvertex);
      hs->attr.faceno=fcno;
      hs->attr.cycleno=ccno;
      hs->attr.edgeno=edno;
      hs->attr.partnerno=partnerno;
      hs->attr.insideAbove = (hs->GetRightPoint() == firstP);
      partnerno++;

      //////////////////////////////////////////////////////////
      if (cr->InsertOk(*hs)){
//          cout<<"insert last segment"<<endl;
          *cr += *hs;
//          cout<<"cr+3 "<<*hs<<endl;
          if(hs->IsLeftDomPoint()){
             *rDir += *hs;
//            cout<<"rDr+3 "<<*hs<<endl;
            hs->SetLeftDomPoint( false );
          }else{
              hs->SetLeftDomPoint( true );
//              cout<<"rDr+4 "<<*hs<<endl;
              *rDir += *hs;
            }
          *cr += *hs;
//          cout<<"cr+4 "<<*hs<<endl;
          delete hs;
          rDir->EndBulkLoad(true, false, false, false);


          //To calculate the inside above attribute
//          bool direction = rDir->GetCycleDirection();
          ////explicitly define it for all regions, false -- area > 0////
          bool direction = false;//counter_wise
//          cout<<"direction "<<direction<<endl;
          int h = cr->Size() - ( rDir->Size() * 2 );
          while(h < cr->Size()){
            //after each left half segment of the region is its
            //correspondig right half segment
            HalfSegment hsIA;
            bool insideAbove;
            cr->Get(h,hsIA);

            if (direction == hsIA.attr.insideAbove)
               insideAbove = false;
            else
               insideAbove = true;
            if (!isCycle)
                insideAbove = !insideAbove;
            HalfSegment auxhsIA( hsIA );
            auxhsIA.attr.insideAbove = insideAbove;
            cr->UpdateAttr(h,auxhsIA.attr);
            //Get right half segment
            cr->Get(h+1,hsIA);
            auxhsIA = hsIA;
            auxhsIA.attr.insideAbove = insideAbove;
            cr->UpdateAttr(h+1,auxhsIA.attr);
            h+=2;
          }
          rDir->DeleteIfAllowed();
        }else assert(false);


      cr->SetNoComponents( fcno+1 );
      cr->EndBulkLoad( true, true, true, false );
      //regs.push_back(RegionT<Array>(*cr,false));
      regs.push_back(*cr);

      cr->DeleteIfAllowed();

}

/*
Check that the pavement gap should not intersect the two roads

*/
template< template<typename T1> class Array1,
          template<typename T2> class Array2,
          template<typename T3> class Array3>
bool SpacePartition::SameSide1(RegionT<Array1>* reg1, 
                               RegionT<Array2>* reg2, 
                               LineT<Array3>* r1r2,
                               Point* junp)
{
      vector<MyPoint> mps1;
      vector<MyPoint> mps2;
      for(int i = 0;i < reg1->Size();i++){
          HalfSegment hs;
          reg1->Get(i,hs);
          if(hs.IsLeftDomPoint()){
            Point lp = hs.GetLeftPoint();
            Point rp = hs.GetRightPoint();
            MyPoint mp1(lp, lp.Distance(*junp));
            MyPoint mp2(rp, rp.Distance(*junp));
            mps1.push_back(mp1);
            mps1.push_back(mp2);
          }
      }
      for(int i = 0;i < reg2->Size();i++){
          HalfSegment hs;
          reg2->Get(i,hs);
          if(hs.IsLeftDomPoint()){
            Point lp = hs.GetLeftPoint();
            Point rp = hs.GetRightPoint();
            MyPoint mp1(lp, lp.Distance(*junp));
            MyPoint mp2(rp, rp.Distance(*junp));
            mps2.push_back(mp1);
            mps2.push_back(mp2);
          }
      }
      sort(mps1.begin(), mps1.end());
      sort(mps2.begin(), mps2.end());


//      cout<<mps1[0].loc<<mps2[0].loc<<mps1[2].loc<<mps2[2].loc<<endl;

      if(mps1.size()<3) return false; // not a region
      if(mps2.size()<3) return false; // not a region
      


      vector<Point> border1;
      border1.push_back(mps1[0].loc);
      border1.push_back(mps1[2].loc);
      border1.push_back(mps2[2].loc);
      border1.push_back(mps2[0].loc);
      vector<Point> border;
      const double delta_dist = 0.1;
      for(unsigned int i = 0;i < border1.size();i++){
        unsigned int j = 0;
        for(;j < border.size();j++){
          if(border[j].Distance(border1[i]) < delta_dist) break;
        }
        if(j == border.size())
          border.push_back(border1[i]);
      }

//      for(unsigned int i = 0;i < border.size();i++)
//        cout<<border[i]<<endl;


      if(border.size() < 3) return false; //not a region
      if(border.size() == 3){
        unsigned int count = 0;
        unsigned int index = 0;

        while(count < 3){
          HalfSegment hs;
          hs.Set(true, border[index], border[(index + 1)%border.size()]);
          Line* temp =  new Line(0);
          int edgeno = 0;
          hs.attr.edgeno = edgeno++;
          *temp += hs;
          hs.SetLeftDomPoint(!hs.IsLeftDomPoint());
          *temp += hs;
          if(temp->Intersects(*r1r2)){
            temp->DeleteIfAllowed();
            return false;
          }
          count++;
          index++;
          temp->DeleteIfAllowed();
        }
        //three points collineation
        if(Collineation(border[0], border[1], border[2])) return false;

            vector<Point> ps;
            if(GetClockwise(border[0], border[1], border[2])){
                ps.push_back(border[1]);
                ps.push_back(border[0]);
                ps.push_back(border[2]);
            }else{
                ps.push_back(border[2]);
                ps.push_back(border[0]);
                ps.push_back(border[1]);
            }

        //should be counter clock wise
            vector<Region> gap;
            ComputeRegion(ps, gap);
            assert(gap.size() > 0);
            outer_fillgap.push_back(gap[0]);
            return true;
            ////////////////////////////////////////////


      }else{ // four points construct a region
          HalfSegment hs;
          hs.Set(true, border[0], border[3]);
          Line* temp =  new Line(0);
          int edgeno = 0;
          hs.attr.edgeno = edgeno++;
          *temp += hs;
          hs.SetLeftDomPoint(!hs.IsLeftDomPoint());
          *temp += hs;
          if(temp->Intersects(*r1r2)){
            temp->DeleteIfAllowed();
            return false;
          }else{
            temp->DeleteIfAllowed();
            vector<Point> ps;

//            cout<<border[3]<< border[0]<< border[2]<<endl;
            if(GetClockwise(border[3], border[0], border[2])){
                ps.push_back(border[0]);
                ps.push_back(border[3]);
                ps.push_back(border[2]);
                ps.push_back(border[1]);
            }else{
                ps.push_back(border[2]);
                ps.push_back(border[3]);
                ps.push_back(border[0]);
                ps.push_back(border[1]);
            }


/*            for(unsigned int i = 0;i < ps.size();i++)
                cout<<ps[i]<<endl; */
            unsigned int i = 0;
            for(;i < ps.size();i++){
                unsigned int index1 = i;
                unsigned int index2 = (i + 1)%ps.size();
                unsigned int index3 = (i + 2)%ps.size();

/*                cout<<i<<" "<<ps[index1]<<" "<<ps[index2]
                    <<" "<<ps[index3]<<endl;*/
                ////////////the special case that three points collineartion///
                if(Collineation(ps[index1], ps[index2], ps[index3]))continue;

                if(GetClockwise(ps[index2], ps[index1], ps[index3]))break;

                vector<Point> temp_ps;
                for(int j = ps.size() - 1;j >= 0;j--)
                  temp_ps.push_back(ps[j]);
                ps.clear();
                for(unsigned int j = 0;j < temp_ps.size();j++)
                  ps.push_back(temp_ps[j]);
                break;
            }

            ////////old checking////////////////
//             assert( i < ps.size());
//            //should be counter clock wise
//             vector<Region> gap;
//             ComputeRegion(ps, gap);
//             outer_fillgap.push_back(gap[0]);
//             return true;

            /////four points can also be collineartion/////////
            if(i < ps.size()){
              vector<Region> gap;
              ComputeRegion(ps, gap);
              assert(gap.size() > 0);
              outer_fillgap.push_back(gap[0]);
              return true;
            }else{
              return false;
            }

            /////////////////////////////////////////////////
/*            cout<<"four points"<<endl;
            for(unsigned int j = 0;j < ps.size();j++){
              vector<Point> ps1;
              int index = (j + 1) % ps.size();
              for(int i = 0 ; i < ps.size() ;i++){
                  Point p = ps[index];
                  ps1.push_back(p);
                  index = (index + 1) % ps.size();
              }
              vector<Region> result1;
              ComputeRegion(ps1, result1);


              if(result1[0].GetCycleDirection()){
                  outer_fillgap.push_back(result1[0]);
                  return true;
              }
            }
            assert(false);*/
            //////////////////////////////////////////////////
          }
      }
}


/*
build a small region around the three halfsegments
the pavement gap should not intersect the small region

*/

template< template<typename T1> class Array1,
          template<typename T2> class Array2,
          template<typename T3> class Array3,
          template<typename T4> class Array4>
bool SpacePartition::SameSide2(RegionT<Array1>* reg1, 
                               RegionT<Array2>* reg2, 
                               LineT<Array3>* r1r2,
                               Point* junp, MyHalfSegment thirdseg, 
                               RegionT<Array4>& smallreg)
{
      if(reg1->Size() < 3 || reg2->Size() < 3){
        return false;
      }

      vector<MyPoint> mps1;
      vector<MyPoint> mps2;
      for(int i = 0;i < reg1->Size();i++){
          HalfSegment hs;
          reg1->Get(i,hs);
          if(hs.IsLeftDomPoint()){
            Point lp = hs.GetLeftPoint();
            Point rp = hs.GetRightPoint();
            MyPoint mp1(lp, lp.Distance(*junp));
            MyPoint mp2(rp, rp.Distance(*junp));
            mps1.push_back(mp1);
            mps1.push_back(mp2);
          }
      }

      for(int i = 0;i < reg2->Size();i++){
          HalfSegment hs;
          reg2->Get(i,hs);
          if(hs.IsLeftDomPoint()){
            Point lp = hs.GetLeftPoint();
            Point rp = hs.GetRightPoint();
            MyPoint mp1(lp, lp.Distance(*junp));
            MyPoint mp2(rp, rp.Distance(*junp));
            mps2.push_back(mp1);
            mps2.push_back(mp2);
          }
      }
      sort(mps1.begin(), mps1.end());
      sort(mps2.begin(), mps2.end());

//      cout<<mps1[0].loc<<mps2[0].loc<<mps1[2].loc<<mps2[2].loc<<endl;

      vector<Point> border1;
      border1.push_back(mps1[0].loc);
      border1.push_back(mps1[2].loc);
      border1.push_back(mps2[2].loc);
      border1.push_back(mps2[0].loc);
      vector<Point> border;
      const double delta_dist = 0.1;
      for(unsigned int i = 0;i < border1.size();i++){
        unsigned int j = 0;
        for(;j < border.size();j++){
          if(border[j].Distance(border1[i]) < delta_dist) break;
        }
        if(j == border.size())
          border.push_back(border1[i]);
      }

//      for(unsigned int i = 0;i < border.size();i++)
//        cout<<border[i]<<endl;


      if(border.size() < 3) return false; //not a region
      if(border.size() == 3){
//          cout<<"3"<<endl;
          unsigned int count = 0;
          unsigned int index = 0;

          while(count < 3){
            HalfSegment hs;
            hs.Set(true, border[index], border[(index + 1)%border.size()]);
            Line* temp =  new Line(0);
            int edgeno = 0;
            hs.attr.edgeno = edgeno++;
            *temp += hs;
            hs.SetLeftDomPoint(!hs.IsLeftDomPoint());
            *temp += hs;

            bool flag = true;
            if(border[index].Inside(smallreg) == false)
              flag = true;
            else
              flag = false;

            if(temp->Intersects(*r1r2) || flag == false){
              temp->DeleteIfAllowed();
              return false;
            }
            count++;
            index++;
            temp->DeleteIfAllowed();
          }
          if(Collineation(border[0], border[1], border[2])) return false;

          vector<Point> ps;
          if(GetClockwise(border[0], border[1], border[2])){
                ps.push_back(border[1]);
                ps.push_back(border[0]);
                ps.push_back(border[2]);
          }else{
                ps.push_back(border[2]);
                ps.push_back(border[0]);
                ps.push_back(border[1]);
          }

        //should be counter clock wise
            vector<Region> gap;
            ComputeRegion(ps, gap);
            assert(gap.size() > 0);
            outer_fillgap.push_back(gap[0]);
            return true;

      }else{ // four points construct a region
//          cout<<"4"<<endl;
          HalfSegment hs;
          hs.Set(true, border[0], border[3]);
          Line* temp =  new Line(0);
          int edgeno = 0;
          hs.attr.edgeno = edgeno++;
          *temp += hs;
          hs.SetLeftDomPoint(!hs.IsLeftDomPoint());
          *temp += hs;

          bool flag = true;
          if(border[0].Inside(smallreg) == false &&
             border[3].Inside(smallreg) == false)
            flag = true;
          else
            flag = false;

          if(temp->Intersects(*r1r2) || flag == false){
            temp->DeleteIfAllowed();
            return false;
          }else{
            temp->DeleteIfAllowed();
            vector<Point> ps;

            if(GetClockwise(border[3], border[0], border[2])){
                ps.push_back(border[0]);
                ps.push_back(border[3]);
                ps.push_back(border[2]);
                ps.push_back(border[1]);
            }else{
                ps.push_back(border[2]);
                ps.push_back(border[3]);
                ps.push_back(border[0]);
                ps.push_back(border[1]);
            }

            unsigned int i = 0;
            for(;i < ps.size();i++){
                unsigned int index1 = i;
                unsigned int index2 = (i + 1)%ps.size();
                unsigned int index3 = (i + 2)%ps.size();
                ////////////the special case that three points collineartion///
                if(Collineation(ps[index1], ps[index2], ps[index3]))continue;
                if(GetClockwise(ps[index2], ps[index1], ps[index3]))break;

                vector<Point> temp_ps;
                for(int j = ps.size() - 1;j >= 0;j--)
                  temp_ps.push_back(ps[j]);
                ps.clear();
                for(unsigned int j = 0;j < temp_ps.size();j++)
                  ps.push_back(temp_ps[j]);
                break;
            }
            assert( i < ps.size());

        //should be counter clock wise

            vector<Region> gap;
            ComputeRegion(ps, gap);
            assert(gap.size() > 0);
            outer_fillgap.push_back(gap[0]);
            return true;
          }

      }
}


/*
the common area of two regions

*/
template< template<typename T1> class Array1,
          template<typename T2> class Array2>
inline bool SpacePartition::PavementIntersection(RegionT<Array1>* reg1, 
                                                 RegionT<Array2>* reg2)
{
    if(reg1->Intersects(*reg2) == false)
      return false;

    Region* reg = new Region(0);
    MyIntersection(*reg1, *reg2, *reg);
    double regarea = reg->Area();

//    cout<<"intersection area "<<regarea<<endl;
    reg->DeleteIfAllowed();
    if(MyAlmostEqual(regarea,0.0)) return false;
    return true;
}

/*
Intersection, union and minus between two regions

*/
template< template<typename T1> class Array1,
          template<typename T2> class Array2,
          template<typename T3> class Array3>
void MySetOp(const RegionT<Array1>& reg1,
           const RegionT<Array2>& reg2,
           RegionT<Array3>& result,
           myavlseg::SetOperation op){

   result.Clear();
   if(!reg1.IsDefined() || !reg2.IsDefined()){
       result.SetDefined(false);
       return;
   }
   result.SetDefined(true);
   if(reg1.Size()==0){
       switch(op){
         case myavlseg::union_op : result = reg2;
                         return;
         case myavlseg::intersection_op : return; // empty region
         case myavlseg::difference_op : return; // empty region
         default : assert(false);
       }
   }
   if(reg2.Size()==0){
      switch(op){
         case myavlseg::union_op: result = reg1;
                        return;
         case myavlseg::intersection_op: return;
         case myavlseg::difference_op: result = reg1;
                             return;
         default : assert(false);
      }
   }

   if(!reg1.BoundingBox().Intersects(reg2.BoundingBox())){
      switch(op){
        case myavlseg::union_op: {
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
        } case myavlseg::difference_op: {
           result = reg1;
           return;
        } case myavlseg::intersection_op:{
           return;
        } default: assert(false);
      }
   }

  priority_queue<HalfSegment,  vector<HalfSegment>, greater<HalfSegment> > q1;
  priority_queue<HalfSegment,  vector<HalfSegment>, greater<HalfSegment> > q2;
  avltree::AVLTree<myavlseg::MyAVLSegment> sss;
  myavlseg::ownertype owner;
  int pos1 = 0;
  int pos2 = 0;
  HalfSegment nextHs;
  int src = 0;

  myavlseg::MyAVLSegment* member = 0;
  myavlseg::MyAVLSegment* leftN  = 0;
  myavlseg::MyAVLSegment* rightN = 0;

  myavlseg::MyAVLSegment left1,right1,common1,
             left2,right2;

  int edgeno =0;
  myavlseg::MyAVLSegment tmpL,tmpR;

  result.StartBulkLoad();

  while( (owner=myselectNext(reg1,pos1,
                           reg2,pos2,
                           q1,q2,nextHs,src))!=myavlseg::none){

       myavlseg::MyAVLSegment current(nextHs,owner);
       member = sss.getMember(current,leftN,rightN);

//       cout<<"current "<<current <<"owner "<<owner<<endl;

//       sss.Print(cout);

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
            if((member->getOwner()==myavlseg::both) ||
               (member->getOwner()==owner)){
               cerr << "overlapping segments detected within a single region"
                    << endl;
               cerr << "the argument is "
                    << (owner==myavlseg::first?"first":"second")
                    << endl;
               cerr.precision(16);
               cerr << "stored is " << *member << endl;
               cerr << "current = " << current << endl;
               myavlseg::MyAVLSegment tmp_left, tmp_common, tmp_right;
               member->split(current,tmp_left, tmp_common, tmp_right, false);
               cerr << "The common part is " << tmp_common << endl;
               cerr << "The lenth = " << tmp_common.length() << endl;
               assert(false);
            }
            int parts = member->split(current,left1,common1,right1);
            sss.remove(*member);
            if(parts & myavlseg::LEFT){
              if(!left1.isPoint()){
//                cout<<"left1 "<<left1<<endl;
                sss.insert(left1);
                myinsertEvents(left1,false,true,q1,q2);
              }
            }
            assert(parts & myavlseg::COMMON);
            // update coverage numbers
            if(current.getInsideAbove()){
               common1.con_above++;
            }  else {
               common1.con_above--;
            }
            if(!common1.isPoint()){
//              cout<<"comm1 "<<common1<<endl;
              sss.insert(common1);
              myinsertEvents(common1,false,true,q1,q2);
            }
            if(parts & myavlseg::RIGHT){
               myinsertEvents(right1,true,true,q1,q2);
            }
          } else { // there is no overlapping segment
            // try to split segments if required

              MysplitByNeighbour(sss, current, leftN, q1, q2);
              MysplitByNeighbour(sss, current, rightN,q1, q2);


//              cout<<"current "<<current<<endl;
            // update coverage numbers
            bool iac = current.getOwner()== myavlseg::first
                            ?current.getInsideAbove_first()
                            :current.getInsideAbove_second();



/*            iac = current.getOwner()== myavlseg::first
                                           ?current.getInsideAbove_first()
                                           :current.getInsideAbove_second();*/



            if(leftN){
//              cout<<"leftN "<<leftN->con_below<<" "<<leftN->con_above<<endl;
//              cout<<*leftN<<endl;
            }
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
            // insert element
            if(!current.isPoint()){
//              cout<<"current2 "<<current<<endl;
              sss.insert(current);
              myinsertEvents(current,false,true,q1,q2);
            }
          }
        } else{  // nextHs.IsRightDomPoint
            if(member && member->exactEqualsTo(current)){
              switch(op){
                case myavlseg::union_op :{

                   if( (member->con_above==0) || (member->con_below==0)) {
                      HalfSegment hs1 = member->getOwner()==myavlseg::both
                                      ?member->convertToHs(true,myavlseg::first)
                                      :member->convertToHs(true);
                      hs1.attr.edgeno = edgeno;
                      result += hs1;
                      hs1.SetLeftDomPoint(false);
                      result += hs1;
                      edgeno++;
                   }
                   break;
                }
                case myavlseg::intersection_op: {

                  if(member->con_above==2 || member->con_below==2){
                      HalfSegment hs1 = member->getOwner()==myavlseg::both
                                      ?member->convertToHs(true,myavlseg::first)
                                      :member->convertToHs(true);
                      hs1.attr.edgeno = edgeno;
                      hs1.attr.insideAbove = (member->con_above==2);
                      result += hs1;
                      hs1.SetLeftDomPoint(false);
                      result += hs1;
                      edgeno++;

                  }
                  break;
                }
                case myavlseg::difference_op : {
                  switch(member->getOwner()){
                    case myavlseg::first:{
                      if(member->con_above + member->con_below == 1){
                         HalfSegment hs1 = member->getOwner()==myavlseg::both
                                      ?member->convertToHs(true,myavlseg::first)
                                      :member->convertToHs(true);
                         hs1.attr.edgeno = edgeno;
                         result += hs1;
                         hs1.SetLeftDomPoint(false);
                         result += hs1;
                         edgeno++;
                      }
                      break;
                    }
                    case myavlseg::second:{
                      if(member->con_above + member->con_below == 3){
                         HalfSegment hs1 = member->getOwner()==myavlseg::both
                                     ?member->convertToHs(true,myavlseg::second)
                                      :member->convertToHs(true);
                         hs1.attr.insideAbove = ! hs1.attr.insideAbove;
                         hs1.attr.edgeno = edgeno;
                         result += hs1;
                         hs1.SetLeftDomPoint(false);
                         result += hs1;
                         edgeno++;
                      }
                      break;
                    }
                    case myavlseg::both: {
                      if((member->con_above==1) && (member->con_below== 1)){
                         HalfSegment hs1 = member->getOwner()==myavlseg::both
                                     ?member->convertToHs(true,myavlseg::first)
                                      :member->convertToHs(true);
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
              MysplitNeighbours(sss,leftN,rightN,q1,q2);
          } // current found in sss
        } // right endpoint
  }

    if(result.Size() > 0 && result.Size() < 6){////its not a region
        result.Clear();
        result.EndBulkLoad(false, false, false, false);
    }
    else{
        result.EndBulkLoad();

    ////////////////////////////////
/*        Region* reg = new Region(0);
        reg->StartBulkLoad();
        int edgeno = 0;
        for(int i = 0;i < result.Size();i++){
          HalfSegment hs1;
          result.Get(i, hs1);
          if(!hs1.IsLeftDomPoint()) continue;
          HalfSegment hs2(hs1);
          Point lp = hs1.GetLeftPoint();
          Point rp = hs1.GetRightPoint();
          ModifyPoint(lp);
          ModifyPoint(rp);
          hs2.Set(true, lp, rp);
          hs2.attr.edgeno = edgeno++;
          *reg += hs2;
          hs2.SetLeftDomPoint(!hs2.IsLeftDomPoint());
          *reg += hs2;
        }
        reg->SetNoComponents(result.NoComponents());
        reg->EndBulkLoad();
        result.Clear();
        result = *reg;
        delete reg; */
    ///////////////////////////////
    }

} // setOP region x region -> region


/*
The first region intersects the seccond region and store the result as the third

*/
template< template<typename T1> class Array1,
          template<typename T2> class Array2,
          template<typename T3> class Array3>
void MyIntersection(const RegionT<Array1>& reg1, 
                    const RegionT<Array2>& reg2, 
                    RegionT<Array3>& result)
{
  MySetOp(reg1,reg2,result,myavlseg::intersection_op);
}



#endif
