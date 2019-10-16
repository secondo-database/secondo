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
//[_] [\_]

[1] Header File of the Spatial Algebra

February, 2003 Victor Teixeira de Almeida

March-July, 2003 Zhiming DING

January, 2005 Leonardo Guerreiro Azevedo

December 2005, Victor Almeida deleted the deprecated algebra levels
(~executable~, ~descriptive~, and ~hibrid~). Only the executable
level remains. Models are also removed from type constructors.

[TOC]

1 Overview

This header file essentially contains the definition of the classes ~Point~,
~Points~, ~Line~, and ~Region~ used in the Spatial Algebra. These classes
respectively correspond to the memory representation for the type constructors
~point~, ~points~, ~line~, and ~region~.  Figure \cite{fig:spatialdatatypes.eps}
shows examples of these spatial data types.

2 Defines and includes

*/
#ifndef __SPATIAL_ALGEBRA_H__
#define __SPATIAL_ALGEBRA_H__

#include <math.h>
#include <cmath>
#include <fstream>
#include <stack>
#include <vector>
#include <queue>
#include <limits>
#include "Attribute.h"
#include "SimplePoint.h"
#include "../../Tools/Flob/DbArray.h"
#include "../../Tools/Flob/MMDbArray.h"
#include "../Rectangle/RectangleAlgebra.h"
#include "AvlTree.h"
#include "AlmostEqual.h"
#include "PointsT.h"
#include "LineT.h"
#include "RegionT.h"
#include "SimpleLineT.h"

#include "HalfSegment.h"
#include "Coord.h"
#include "Algebras/Geoid/Geoid.h"
#include "NestedList.h"
#include "ListUtils.h"
#include "Berlin2WGS.h"

#include "AVLSegment.h"
#include "LineSplitter.h"


/*
10 Auxiliary classes used by window clipping functions

10.1 Edge Point

This class stores the information need about the points that lie on the window edge:
- The point's coordinates
- The direction of the point which represents where is the area of the region related to the point.
- If the point must be rejected during the creation of the new segments that lie on the window's edges.

*/
class EdgePoint : public Point
{

  public:
    EdgePoint(): Point(true,0,0)
    {
    }

    EdgePoint( const Point p,
               const bool dir,
               const bool reject):
    Point(p)
    {
      direction = dir;
      rejected = reject;
    }

    void Set( const Coord& X, const Coord& Y,
              const bool dir, const bool reject )
    {
      x = X;
      y = Y;
      direction = dir;
      rejected = reject;
    }

    void Set( const Point& p,
              const bool dir,
              const bool reject)
    {
      Set( p.GetX(), p.GetY(), dir, reject );
    }

    void  Set( const Coord& X, const Coord& Y )
    {
      rejected = false;
      x = X;
      y = Y;
    }

    EdgePoint& operator=( const EdgePoint& p )
    {
      x = p.GetX();
      y = p.GetY();
      direction = p.direction;
      rejected = p.rejected;
      return *this;
    }

    static EdgePoint* GetEdgePoint( const Point &p,
                                    const Point &p2,
                                    bool insideAbove,
                                    const Point &v,
                                    const bool reject );

    bool operator==( const EdgePoint& p )
    {
      if( AlmostEqual( this->GetX(), p.GetX() ) &&
          AlmostEqual( this->GetY(), p.GetY() ) &&
          (this->direction == p.direction)  &&
          (this->rejected == p.rejected) )
          return true;
      return false;
    }

    bool operator!=( const EdgePoint &p )
    {
      return !(*this==p);
    }

    inline bool operator<( const EdgePoint& p ) const
    {
      if( this->x < p.x ){
        return true;
      } else if( this->x == p.x && this->y < p.y ){
        return true;
      } else if( this->x == p.x && this->y == p.y ) {
            //If both points has the same coordinates, if they have diferent
            // directions
            // the less one will be that has the attribute direction true i
            // (left and down),
            // otherwise, both have direction true, and the less one will
            // be that was rejected.
            //The rejected points come before accepted points
            if (this->direction == p.direction){
              if (this->rejected){
                return true;
              }
            } else if (this->direction){
              return true;
            }
          }
      return false;
    }

/*
Attributes

*/
    bool direction;
/*
The direction attributes represents where is the area of the region related to the point
and the edge of the window in which it lies:
--> For horizontal edges (top and bottom edges), its value is true if the area of the region
    is on the left (<==), and false if it lies on the right (==>).
--> For vertical edges (left and right edges), its value is true if the area of the region
    is down the point (DOWN), and false otherwise (UP).

*/
    bool rejected;
/*
Indicates whether the point is of a segment that was rejected

*/
};


/*
10.2 SCycle

This class is used to store the information need for cycle computation which sets the face number,
cycle number and edge number of the half segments.

*/

class SCycle
{
  public:
    HalfSegment hs1,hs2;
    int hs1PosRight,hs1PosLeft,
        hs2PosRight,hs2PosLeft;
    bool goToCHS1Right,goToCHS1Left,
         goToCHS2Right,goToCHS2Left;
    int hs1Partnerno,hs2Partnerno;
    Point criticalPoint, nextPoint;

    SCycle() {}

    SCycle( const HalfSegment &hs, const int partnerno,
            const HalfSegment &hsP,const int partnernoP,
            const Point &criticalP, const Point &nextPOINT):
    hs1(hs), hs2(hsP), 
    hs1PosRight(partnernoP), hs1PosLeft(partnernoP),
    hs2PosRight(partnerno),hs2PosLeft(partnerno),
    goToCHS1Right(true),goToCHS1Left(true),
    goToCHS2Right(true),goToCHS2Left(true),
    hs1Partnerno(partnerno),hs2Partnerno(partnernoP),
    criticalPoint(criticalP),nextPoint(nextPOINT)
    { }

    SCycle(const SCycle &sAux) = default;

    ~SCycle()
    {}
};

#include "PointsTImpl.h"
#include "LineTImpl.h"
#include "RegionTImpl.h"
#include "SimpleLineTImpl.h"
#include "AVLSegmentImpl.h"


typedef PointsT<DbArray> Points;
typedef PointsT<MMDbArray> MMPoints;
typedef LineT<DbArray>Line;
typedef LineT<MMDbArray>MMLine;
typedef RegionT<DbArray>Region;
typedef RegionT<MMDbArray>MMRegion;
typedef SimpleLineT<DbArray> SimpleLine;
typedef SimpleLineT<MMDbArray> MMSimpleLine;



/*
Coordinates are represented by real numbers.

*/

/*
The $\pi$ value.

*/

/*
The four edges of a window.

*/
class Point;
class HalfSegment;
class GrahamScan;
class SimplePoint;


/*
Forward declarations.

3 Auxiliary Functions

*/
// const double FACTOR = 0.00000001; // moved to Attribute.h

double ApplyFactor( const double d );


inline int CompareDouble(const double a, const double b){
   if(AlmostEqual(a,b))
   {
       return 0;
   }
   if(a<b)
   {
      return -1;
   }
   return 1;
}

bool getDir(const std::vector<Point>& vp);




// for finding insert position and sorting the DBArray:
int PointCompare( const void *a, const void *b );

// for checking whether DBArray contains an element and
// removing duplicates:
int PointCompareAlmost( const void *a, const void *b );

/*
5 Class Points

This class implements the memory representation of the ~points~ type constructor.
A points value is a finite set of points. An example of a points value can be seen
in the Figure \cite{fig:spatialdatatypes.eps}.

The implementation of the points type constructor is a persistent array of points
ordered by lexicographic order.

*/





/*
7 Class Region

This class implements the memory representation of the ~region~ type constructor. A region is
composed of a set of faces. Each face consists of a set of cycles which correspond to an outer
cycle and a groups of holes (inner cycles).

A ~region~ value is a set of half segments. In the external (nested list) representation, a region
value is expressed as a set of faces, and each face is composed of a set of cycles.  However, in the
internal (class) representation, it is expressed as a set of sorted half segments, which are stored
in a persistend DBArray.

*/
/*
Forward declaration of class ~EdgePoint~

*/


/*
8 Function headers

*/

Word InPoint( const ListExpr typeInfo, const ListExpr instance,
              const int errorPos, ListExpr& errorInfo, bool& correct );
ListExpr OutPoint( ListExpr typeInfo, Word value );

Word InLine( const ListExpr typeInfo, const ListExpr instance,
              const int errorPos, ListExpr& errorInfo, bool& correct ) ;
ListExpr OutLine( ListExpr typeInfo, Word value );

Word InRegion( const ListExpr typeInfo, const ListExpr instance,
               const int errorPos, ListExpr& errorInfo, bool& correct );
ListExpr OutRegion( ListExpr typeInfo, Word value );

Word InSimpleLine( const ListExpr typeInfo, const ListExpr instance,
        const int errorPos, ListExpr& errorInfo, bool& correct ) ;
ListExpr OutSimpleLine( ListExpr typeInfo, Word value );



double Angle(const Point &v, const Point &p1, const Point &p2);
double VectorSize(const Point &p1, const Point &p2, const Geoid* geoid = 0);


/*
11.5 Class ~GrahamScan~

*/

/*
Classes suppoorting the computation of the convex hull of
an pointset.

*/


class GrahamScan{
public:
  template<template<typename T>class Array>
  static void convexHull(const PointsT<Array>* ps, RegionT<Array>* result){
     result->Clear();
     if(!ps->IsDefined() ){
         result->SetDefined(false);
         return;
     }
     if(ps->Size()<3){
        result->SetDefined(false);
        return;
     }
     GrahamScan scan(ps);
     int size = scan.computeHull();
     if(size<3){ // points was on a single line
        result->SetDefined(false);
        return;
     }


     result->SetDefined(true);
     result->StartBulkLoad();
     for(int i=0;i<size-1; i++){
        SimplePoint p1(scan.p[i]);
        SimplePoint p2(scan.p[i+1]);
        // build the halfsegment
        HalfSegment hs1(true,p1.getPoint(),p2.getPoint());
        HalfSegment hs2(false,p1.getPoint(),p2.getPoint());
        hs1.attr.edgeno = i;
        hs2.attr.edgeno = i;
        bool ia = isInsideAbove(p1,p2);
        hs1.attr.insideAbove = ia;
        hs2.attr.insideAbove = ia;
        (*result) += hs1;
        (*result) += hs2;
     }
     // close the polygon
     SimplePoint p1(scan.p[size-1]);
     SimplePoint p2(scan.p[0]);
     // build the halfsegment
     HalfSegment hs1(true,p1.getPoint(),p2.getPoint());
     HalfSegment hs2(false,p1.getPoint(),p2.getPoint());
     hs1.attr.edgeno = size-1;
     hs2.attr.edgeno = size-1;
     bool ia = isInsideAbove(p1,p2);
     hs1.attr.insideAbove = ia;
     hs2.attr.insideAbove = ia;
     (*result) += hs1;
     (*result) += hs2;
     result->EndBulkLoad();
  }


private:
   std::vector<SimplePoint> p;
   int n;
   int h;


  template<template<typename T> class Array>
  GrahamScan(const PointsT<Array>* ps){
     n = ps->Size();
     Point pt;
     for(int i=0;i<n;i++){
        ps->Get(i,pt);
        p.push_back(SimplePoint(pt));
     }
  }

   int computeHull(){
    if(n<3){
      return n;
    }
    h = 0;
    grahamScan();
    return h;
  }
   void grahamScan(){
     int min = indexOfLowestPoint();

     exchange(0,min);



     SimplePoint pl(p[0]);
     makeRelTo(pl);
     sort();
     makeRelTo(pl.reversed());

     int i=3;
     int k=3;
     while(k<n){
        exchange(i,k);
        while(!isConvex(i-1)){
           exchange(i-1,i);
           i--;
        }
        k++;
        i++;
     }
     // remove a possibly last 180 degree angle
     if(i>=3){
        if(!p[i-1].isConvex(p[i-2],p[0])){
            i--;
        }
     }

     h = i;
   }

   static bool isInsideAbove(const SimplePoint& p1, const SimplePoint& p2){
     double diffx = p2.getX()-p1.getX();
     double diffy = p2.getY()-p1.getY();

     if(AlmostEqual(diffx,0.0)){
        return diffy < 0;
     }
     if(AlmostEqual(diffy,0.0)){
        return diffx > 0;
     }

     bool sx = diffx>0;
    // bool sy = diffy>0;
    // return sx == sy;
    return sx;
   }


   void exchange(const int i, const int j){
      SimplePoint t(p[i]);
      p[i] = p[j];
      p[j] = t;
   }

   void makeRelTo(const SimplePoint& p0){
       SimplePoint p1(p0);
       for(int i=0;i<n;i++){
          p[i].makeRelTo(p1);
       }
   }

   int indexOfLowestPoint()const{
     unsigned min = 0;
     for(unsigned int i=1; i<p.size(); i++){
        if(p[i].isLower(p[min])){
           min = i;
        }
     }
     return min;
   }

   bool isConvex(const int i){
     return p[i].isConvex(p[i-1],p[i+1]);
   }

   void sort(){
     std::vector<SimplePoint>::iterator it= p.begin();
     it++;
     std::sort(it,p.end()); // without the first point
   }


}; // end of class GrahamScan


/*
4 Some classes realizing different projections


*/
class UTM{
public:
  UTM(){
     init();
  }

  ~UTM(){}

  bool operator()(const Point& src, Point& res){
     if(!src.IsDefined() ){
       res.SetDefined(false);
       return false;
     }
     double bw(src.GetX());
     double lw(src.GetY());
     if(bw<-180 || bw>180 || lw<-90 || lw>90){
        res.SetDefined(false);
        return false;
     }
     // zone number??
     // long lzn =(int)((lw+180)/6) + 1;
     long lzn = 39;

     //long bd = (int)(1+(bw+80)/8);
     double br = bw*M_PI/180;
     double tan1 = tan(br);
     double tan2 = tan1*tan1;
     double tan4 = tan2*tan2;
     double cos1 = cos(br);
     double cos2 = cos1*cos1;
     double cos4 = cos2*cos2;
     double cos3 = cos2*cos1;
     double cos5 = cos4*cos1;
     double etasq = ex2*cos2;
     // Querkruemmungshalbmesser nd
     double nd = c/sqrt(1+etasq);
     double g = e0*bw + e2*sin(2*br) + e4*sin(4*br) + e6*sin(6*br);
     long lh = (lzn - 30)*6 - 3;
     double dl = (lw - lh)*M_PI/180;
     double dl2 = dl*dl;
     double dl4 = dl2*dl2;
     double dl3 = dl2*dl;
     double dl5 = dl4*dl;
     double x;
     if(bw<0){
        x = 10e6 + 0.9996*(g + nd*cos2*tan1*dl2/2 +
                           nd*cos4*tan1*(5-tan2+9*etasq)*dl4/24);
     }else{
        x = 0.9996*(g + nd*cos2*tan1*dl2/2 +
                    nd*cos4*tan1*(5-tan2+9*etasq)*dl4/24) ;
     }
     double y = 0.9996*(nd*cos1*dl +
                        nd*cos3*(1-tan2+etasq)*dl3/6 +
                        nd*cos5*(5-18*tan2+tan4)*dl5/120) + 500000;
     res.Set(x,y);
     return true;
  }
private:
   double a;
   double f;
   double c;
   double ex2;
   double ex4;
   double ex6;
   double ex8;
   double e0;
   double e2;
   double e4;
   double e6;

  void init(){
    a = 6378137.000;
    f = 3.35281068e-3;
    c = a/(1-f); // Polkrümmungshalbmesser in german
    ex2 = (2*f-f*f)/((1-f)*(1-f));
    ex4 = ex2*ex2;
    ex6 = ex4*ex2;
    ex8 = ex4*ex4;
    e0 = c*(M_PI/180)*(1 - 3*ex2/4 +
               45*ex4/64  - 175*ex6/256  + 11025*ex8/16384);
    e2 = c*(  - 3*ex2/8 + 15*ex4/32  - 525*ex6/1024 +  2205*ex8/4096);
    e4 = c*(15*ex4/256 - 105*ex6/1024 +  2205*ex8/16384);
    e6 = c*( -  35*ex6/3072 +   315*ex8/12288);
  }

};








/*
~insertEvents~

This function is used to realize plane sweep algorithms.
It inserts the Halfsegmsnts for ~seg~ into ~q~1 and
~q~2 depending on the owner of ~seg~. The flags ~createLeft~
and ~createRight~ control which Halfsegments (LeftDomPoint or not)
should be inserted.

*/
void insertEvents(const avlseg::AVLSegment& seg,
                  const bool createLeft,
                  const bool createRight,
                  std::priority_queue<avlseg::ExtendedHalfSegment,
                              std::vector<avlseg::ExtendedHalfSegment>,
                              std::greater<avlseg::ExtendedHalfSegment> >& q1,
                  std::priority_queue<avlseg::ExtendedHalfSegment,
                              std::vector<avlseg::ExtendedHalfSegment>,
                              std::greater<avlseg::ExtendedHalfSegment> >& q2);

/*
~splitByNeighbour~

This function splits current (and neigbour) if required. neigbour
is replaces in ~sss~ by its left part (the part before the crossing)
and current is shortened to its left part. The remainding parts (the right parts)
are inserted into the corresponding queues depending on the woner of ~current~ and
~neighbour~.

If forceThrow is set to true, in each case of trouble, an excapeion is thrown.
Otherwise, the algorithms tries to correct the error.

*/

bool splitByNeighbour(avltree::AVLTree<avlseg::AVLSegment>& sss,
                      avlseg::AVLSegment& current,
                      avlseg::AVLSegment*& neighbour,
                      std::priority_queue<avlseg::ExtendedHalfSegment,
                          std::vector<avlseg::ExtendedHalfSegment>,
                          std::greater<avlseg::ExtendedHalfSegment> >& q1,
                      std::priority_queue<avlseg::ExtendedHalfSegment,
                          std::vector<avlseg::ExtendedHalfSegment>,
                          std::greater<avlseg::ExtendedHalfSegment> >& q2,
                     const bool forceThrow);


/*
~isSpatialType~

This function checks whether ~type~ represents a spatial type, i.e. point, points,
line, region.

*/
bool IsSpatialType(ListExpr type);

struct P3D;

class WGSGK{

public:
  WGSGK(){ init(); }
  bool project(const Point& src, Point& result) const;
  bool project(const HalfSegment& src, HalfSegment& result) const;
  bool getOrig(const Point& src, Point& result) const;
  void enableWGS(const bool enabled);
  void setMeridian(const int m);

private:
  void HelmertTransformation(const double x,
                             const double y,
                             const double z,
                             P3D& p) const;
  void BesselBLToGaussKrueger(const double b,
                              const double ll,
                              Point& result) const;
  void BLRauenberg (const double x, const double y,
                    const double z, P3D& result) const;
  double newF(const double f, const double x,
              const double y, const double p) const;

  bool  gk2geo(const double GKRight, const double GKHeight,
               Point&  result) const;
  bool  bessel2WGS(const double geoDezRight, const double geoDezHeight,
                   Point& result) const;

  void init();

  double Pi;
  double awgs;         // WGS84 Semi-Major Axis = Equatorial Radius in meters
  double bwgs;      // WGS84 Semi-Minor Axis = Polar Radius in meters
  double abes;       // Bessel Semi-Major Axis = Equatorial Radius in meters
  double bbes;       // Bessel Semi-Minor Axis = Polar Radius in meters
  double cbes;       // Bessel latitude to Gauss-Krueger meters
  double dx;                // Translation Parameter 1
  double dy;                  // Translation Parameter 2
  double dz;                // Translation Parameter 3
  double rotx;   // Rotation Parameter 1
  double roty;   // Rotation Parameter 2
  double rotz;  // Rotation Parameter 3
  double sc;           // Scaling Factor
  double h1;
  double eqwgs;
  double eqbes;
  double MDC;  // standard in Hagen
  bool useWGS; // usw coordinates in wgs ellipsoid
  double rho;
};

/*
Auxiliary Function

This function creates a regular n-corder around p with radius radius.

The point must be defined, the radius must be greater than zero. n must be
between 3 and 100. Otherwise, the result will be undefined.


*/
void generateCircle(Point* p, double radius, int n , Region* res);


template<template<typename T>class Array>
bool Point::Inside( const RegionT<Array>& r,
                    const Geoid* geoid /*=0*/ ) const
{
  return r.Contains(*this,geoid);
}

template<template<typename T>class Array>
bool Point::Inside( const LineT<Array>& l,
                    const Geoid* geoid /*=0*/ ) const
{
  return l.Contains(*this,geoid);
}

template<template<typename T>class Array>
bool Point::Inside(const SimpleLineT<Array>& l, const Geoid* geoid /*=0*/) const
{
  return l.Contains(*this,geoid);
}

template<template<typename T>class Array>
bool Point::Inside( const PointsT<Array>& ps,
                    const Geoid* geoid /*=0*/ ) const
{
  return ps.Contains(*this,geoid);
}


#endif // __SPATIAL_ALGEBRA_H__
