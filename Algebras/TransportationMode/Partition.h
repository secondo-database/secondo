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

ostream& operator<<(ostream& o, const ownertype& owner);


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
  void Print(ostream& out)const;

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

ostream& operator<<(ostream& o, const MyAVLSegment& s);


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

void MySetOp(const Region& reg1, const Region& reg2,Region& result,
           myavlseg::SetOperation op);
void MySetOp(const Line& line, const Region& region, Line& result,
           myavlseg::SetOperation op);
void MySetOp(const Line& line1, const Line& line2, Line& result,
             myavlseg::SetOperation op);

void MyMinus(const Region& reg1, const Region& reg2, Region& result);
void MyIntersection(const Region& reg1, const Region& reg2, Region& result);
void MyIntersection(const Line& line, const Region& reg, Line& result);
void MyIntersection(const Line& l1, const Line& l2, Line& result);

void MyUnion(const Region& reg1, const Region& reg2, Region& result);
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
ostream& operator<<(ostream& o, const MySegDist& seg);

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
  vector<int> junid1;
  vector<int> junid2;
  vector<Region> outer_regions_s;
  vector<Region> outer_regions1;
  vector<Region> outer_regions2;
  vector<Region> outer_regions_l;
  vector<Region> outer_regions4;
  vector<Region> outer_regions5;

  vector<Region> outer_fillgap1;
  vector<Region> outer_fillgap2;
  vector<Region> outer_fillgap;

  vector<Line> pave_line1;
  vector<Line> pave_line2;
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
  void TransferSegment(MyHalfSegment&, vector<MyHalfSegment>&, int, bool);


  //add the segment to line, but change its points coordinate to int value
  void AddHalfSegmentResult(MyHalfSegment hs, Line* res, int& edgeno);
  //for the given line stored in segs, get the line after transfer
  void Gettheboundary(vector<MyHalfSegment>& segs,
                      vector<MyHalfSegment>& boundary, int delta,
                      bool clock_wise);
  //get all the points forming the boundary for road region
  void ExtendSeg1(vector<MyHalfSegment>& segs,int delta,
                bool clock_wise,
                vector<Point>& outer, vector<Point>& outer_half);
 //get all the points forming the boundary for both road and pavement region
  void ExtendSeg2(vector<MyHalfSegment>& segs,int delta,
                     bool clock_wise, vector<Point>& outer);

 //translate a given line with a distance limitation 
  void ExtendSeg3(vector<MyHalfSegment>& segs,int delta,
                     bool clock_wise, vector<Point>& outer);

  // order the segments so that the end point of last one connects to the start
  // point of the next one, the result is stored as vector<MyHalfSegment>

  void ReorderLine(SimpleLine*, vector<MyHalfSegment>&);
  //create a region from the given set of ordered points
  void ComputeRegion(vector<Point>& outer_region, vector<Region>& regs);
  void CheckRegionPS(vector<Point>& outer_region);
  //extend each road to a region
  void ExtendRoad(int attr_pos, int w);
  //remove triangle area after cutting
  void FilterDirtyRegion(vector<Region>& regs, Region* reg);
  //cut the intersection region between pavement and road
  void ClipPaveRegion(Region& reg,
                       vector<Region>& paves,int rid, Region* inborder);
  //fill the gap between two pavements at some junction positions
  void FillPave(Network* n, vector<Region>& pavements1,
                vector<Region>& pavements2,
                vector<double>& routes_length,
                vector<Region>& paves1, vector<Region>& paves2);
  //get the pavement beside each road
  void Getpavement(Network* n, Relation* rel1, int attr_pos,
                  Relation* rel2, int attr_pos1, int attr_pos2, int w);
  //get the closest point in hs to p and return the point and distance
  double GetClosestPoint(HalfSegment& hs, Point& p, Point& cp);
  double GetClosestPoint_New(HalfSegment& hs, Point& p, Point& cp);

  //transfer the halfsegment by a deviation
  void TransferHalfSegment(HalfSegment& hs, int delta, bool flag);
  //for the given line, get its curve after transfer
  void GetSubCurve(SimpleLine* curve, Line* newcurve,int roadwidth, bool clock);

  //build zebra crossing at junction position, called by GetZebraCrossing()
  bool BuildZebraCrossing(vector<MyPoint>& endpoints1,
                          vector<MyPoint>& endpoints2,
                          Region* reg_pave1, Region* reg_pave2,
                          Line* pave1, Region* crossregion,
                          Point& junp, Region* last_zc);
//for the road line around the junction position, it creates the zebra crossing
  void GetZebraCrossing(SimpleLine* subcurve,
                        Region* reg_pave1, Region* reg_pave2,
                        int roadwidth, Line* pave1, double delta_l,
                        Point p1, Region* crossregion, Region* last_zc);


   // Extend the line in decreasing direction
  void Decrease(SimpleLine* curve, Region* reg_pave1,
                      Region* reg_pave2, double len, Line* pave,
                      int roadwidth, Region* crossregion, Region* last_zc);
  //Extend the line in increasing direction
  void Increase(SimpleLine* curve, Region* reg_pave1,
                      Region* reg_pave2, double len, Line* pave,
                      int roadwidth, Region* crossregion, Region* last_zc);
  //create the pavement at each junction position
  void CreatePavement(SimpleLine* curve, Region* reg_pave1,
                      Region* reg_pave2, double len, 
                      int roadwidth, Region* crossregion1,
                      Region* crossregion2, Region* last_zc);

  //cut the common pavements of two roads at the junction position
  void DecomposePave(Region* reg1, Region* reg2, vector<Region>& result);
  void GetCommPave1(vector<Region_Oid>& pave1,
                    vector<Region_Oid>& pave2, int,int);
  void GetCommPave2(Region* reg, int, vector<Region_Oid>& pave2);
  void DecomposePavement1(Network* n, Relation* rel,
                        int attr_pos1, int attr_pos2, int attr_pos3);
  void DecomposePavement2(int start_oid, Relation* rel,
                        int attr_pos1, int attr_pos2);
  void GetPavementEdge1(Network*, Relation*, BTree*, int, int, int);
  void GetPavementEdge2(Relation*, Relation*, BTree*, int, int, int);

  bool RidPosExist(int rid, float pos, vector<vector<float> >& rid_pos_list);
  ///////////cut the commone area between pavements and road regions///
  void Junpavement(Network* n, Relation* rel, int attr_pos1,
                  int attr_pos2, int width, Relation* rel_road,int attr_pos3);

  //Detect whether three points collineation
  bool Collineation(Point& p1, Point& p2, Point& p3);
  //Check that the pavement gap should not intersect the two roads
  bool SameSide1(Region* reg1, Region* reg2, Line* r1r2,Point* junp);
  //build a small region around the three halfsegments
  bool SameSide2(Region* reg1, Region* reg2, Line* r1r2,
                  Point* junp, MyHalfSegment thirdseg, Region& smallreg);

  //the common area of two regions
  inline bool PavementIntersection(Region* reg1, Region* reg2);
  //check the junction position rids.size() != 2 rids.size() != 6
  void NewFillPavementDebug(Relation* rel, Relation* routes,
                      int id1, int id2,
                      Point* junp, int attr_pos1, int attr_pos2,
                      vector<int> rids);

  // check for the junction where two road intersect
  // rids.size() == 2, used by operator fillgap

  void NewFillPavement1(Relation* rel, Relation* routes,
                      int id1, int id2,
                      Point* junp, int attr_pos1, int attr_pos2,
                      vector<int> rids);

  //check for the junction where three roads intersect
  //called by operator fillgap

  void NewFillPavement2(Relation* rel, Relation* routes,
                      int id1, int id2,
                      Point* junp, int attr_pos1, int attr_pos2,
                      vector<int> rids);

  //the same function as in NewFillPavement2, but with different input
  //parameters called by function FillPave()

  void NewFillPavement3(Relation* routes, int id1, int id2,
                      Point* junp, vector<Region>& paves1,
                      vector<Region>& paves2, vector<int> rids,
                      vector<Region>& newpaves1, vector<Region>& newpaves2);

  //the same function as NewFillPavement2, but with different input parameters
  //called by function FillPave()

  void NewFillPavement4(Relation* routes, int id1, int id2,
                      Point* junp, vector<Region>& paves1,
                      vector<Region>& paves2, vector<int> rids,
                      vector<Region>& newpaves1, vector<Region>& newpaves2);
  void NewFillPavement5(Relation* routes, int id1, int id2,
                      Point* junp, vector<Region>& paves1,
                      vector<Region>& paves2, vector<int> rids,
                      vector<Region>& newpaves1, vector<Region>& newpaves2);

  //for operator fillgap
  void FillHoleOfPave(Network* n, Relation* rel,  int attr_pos1,
                      int attr_pos2, int width);

};


/*
for each route, it returns all possible locations where interesting points
can locate 

*/
struct StrRS{
	Network* n;
	Relation* r1;
	Relation* r2;
	unsigned int count;
	TupleType* resulttype;
    vector<int> rids; 
	vector<Line> lines;
    vector<Point> interestps;
    vector<Point> ps; 
    vector<bool> ps_type; 
	StrRS();
	~StrRS();
	StrRS(Network* net, Relation* rel1, Relation* rel2);
	void GetSections(int attr_pos1, int attr_pos2, int attr_pos3);
    void GenPoints1(int attr_pos1, int attr_pos2, int attr_pos3, 
                   int attr_pos4, int no_ps);
    void GenPoints2(R_Tree<2,TupleId>*, int attr_pos1, int attr_pos2, 
                    unsigned int);
    void DFTraverse(R_Tree<2,TupleId>*, SmiRecordId, Point*, int);
    void GetInterestingPoints(HalfSegment hs, Point ip, 
                              vector<MyPoint>& intersect_ps, Region*, Region*);
};

#define TM_MYPI 3.1415927

/*
data clean process 

*/
struct DataClean{
  unsigned int count;
  TupleType* resulttype; 
  
  DataClean(){ count = 0; resulttype = NULL;} 
  ~DataClean(){if(resulttype != NULL) delete resulttype;}

  vector<SimpleLine> sl_list;


  void ModifyLine(SimpleLine* in, SimpleLine* out);
  void CheckRoads(Relation* r, R_Tree<2,TupleId>* rtree);
  void DFTraverse(Relation* rel,R_Tree<2,TupleId>* rtree, SmiRecordId adr, 
                          Line* sl, vector<int>& id_list, unsigned int id);
  void DFTraverse2(Relation* rel,R_Tree<2,TupleId>* rtree, SmiRecordId adr, 
                          Line* sl, vector<int>& id_list, unsigned int id);
};


#endif
