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
#include "Attribute.h"
#include "../../Tools/Flob/DbArray.h"
#include "../Rectangle/RectangleAlgebra.h"
#include "WinUnix.h"
#include "AvlTree.h"
#include "AlmostEqual.h"
#include "AVLSegment.h"

#include "HalfSegment.h"
#include "Coord.h"
#include "Geoid.h"
#include "NestedList.h"
#include "ListUtils.h"

/*
Coordinates are represented by real numbers.

*/

/*
The $\pi$ value.

*/

enum WindowEdge { WTOP, WBOTTOM, WLEFT, WRIGHT };
/*
The four edges of a window.

*/
class Point;
class Points;
class HalfSegment;
class Line;
class Region;
class SimpleLine;
class GrahamScan;
class SimplePoint;


/*
Forward declarations.

3 Auxiliary Functions

*/
// const double FACTOR = 0.00000001; // moved to Attribute.h

inline double ApplyFactor( const double d );


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

bool getDir(const vector<Point>& vp);


int HalfSegmentCompare(const void *a, const void *b);
int PointHalfSegmentCompare( const void *a, const void *b );
int PointHalfSegmentCompareAlmost( const void *a, const void *b );
int LRSCompare( const void *a, const void *b );


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
class Points: public StandardSpatialAttribute<2>
{
  public:
/*
5.1 Constructors and Destructor

There are three ways of constructing a point set:

*/
    inline Points() {}
/*
This constructor should not be used.

*/
    explicit inline Points( const int initsize );
/*
The first one constructs an empty point set but open space for ~initsize~ points.

*/
    inline Points( const Points& ps);
/*
The second one receives another point set ~ps~ as argument and constructs a point
set which is a copy of ~ps~.

*/
    inline void Destroy()
    {
      points.destroy();
    }
/*
This function should be called before the destructor if one wants to destroy the
persistent array of points. It marks the persistent array for destroying. The
destructor will perform the real destroying.

*/
    inline ~Points()
    {}
/*
The destructor.

5.2 Functions for Bulk Load of Points

As said before, the point set is implemented as an ordered persistent array of points.
The time complexity of an insertion operation in an ordered array is $O(n)$, where ~n~
is the size of the point set. In some cases, bulk load of points for example, it is good
to relax the ordered condition to improve the performance. We have relaxed this ordered
condition only for bulk load of points. All other operations assume that the point set is
ordered.

*/
    inline bool IsOrdered() const;
/*
Returns whether the point set is ordered. There is a flag ~ordered~ (see attributes) in order
to avoid a scan in the point set to answer this question.

*/
    void StartBulkLoad();
/*
Marks the begin of a bulk load of points relaxing the condition that the points must be
ordered.

*/
    void EndBulkLoad( bool sort = true, bool remDup = true, bool trim = true );
/*
Marks the end of a bulk load and sorts the point set if the argument ~sort~ is set to true.

5.3 Member functions

*/
    inline const Rectangle<2> BoundingBox(const Geoid* geoid = 0) const;
/*
Returns the bounding box that spatially contains all points.

*/
    inline bool IsEmpty() const;
/*
Returns true iff the set is undefined or empty.

*/
    bool IsValid() const;
/*
Checks if the point set is valid, i.e., if it contains only defined points and
no duplicates.

*/
    inline int Size() const;
/*
Returns the size of the point set in terms of number of points.
Returns ~0~ if the set is empty.

*/
    void Clear();
/*
Clears the point set.

*/
    inline void Resize(const int newSize);
/*
Sets the new capacity of the points array to the
maximum of its original size and the argument.

*/
    inline void TrimToSize();
/*
Sets the new capacity of the points array to the amount really required.

*/

    inline bool Get( const int i, Point& p ) const;
/*
Retrieves the point ~p~ at position ~i~ in the point set.

*Precondition:* $0 \leq i < Size()$

*/
    Points& operator=( const Points& ps );
/*
Assignement operator redefinition.

*/
    bool Contains( const Point& p, const Geoid* geoid=0 ) const;
/*
Searches (binary search algorithm) for a point in the point set and
return ~true~ if found and ~false~ if not.

*Precondition:* ~this.IsOrdered() $\&\&$ p.IsDefined()~

*/
    bool Contains( const Points& ps, const Geoid* geoid=0 ) const;
/*
Returns ~true~ if this point set contains the ~ps~ point set and
~false~ otherwise.

*Precondition:* ~this.IsOrdered() $\&\&$ ps.IsOrdered()~

*/


/*

5.4 Operations

5.4.1 Operation $=$ (~equal~)

*Precondition:* ~U.IsOrdered() $\&\&$ V.IsOrdered()~

*Semantics:* $U = V$

*Complexity:* $O(n+m)$, where ~n~ is the size of ~U~ and m the size of ~V~.

*/
    bool operator==( const Points& ) const;

    bool operator==( const Point&) const;

/*
5.4.2 Operation $\neq$ (~not equal~)

*Precondition:* ~U.IsOrdered() $\&\&$ V.IsOrdered()~

*Semantics:* $U = V$

*Complexity:* $O(n+m)$, where ~n~ is the size of ~U~ and m the size of ~V~.

*/
    bool operator!=( const Points& ) const;
/*
5.4.3 Operation ~union~ (with ~point~)

*Precondition:* ~v.IsDefined()~

*Semantics:* $U \cup \{v\}$

*Complexity:* $O(1)$, if the set is not ordered, and $O(log(n)+n)$, otherwise, where ~n~ is the size
of ~U~.

*/
    Points& operator+=( const Point& p );
/*
5.4.4 Operation ~union~ (with ~points~)

*Semantics:* $U \cup V$

*Complexity:* $O(m)$, if the sets are not ordered, and $O(m+(m+n)log(m+n))$, otherwise, where ~n~ is
the size of ~U~ and ~m~ is the size of ~V~.

*/
    Points& operator+=( const Points& ps );
/*
5.4.5 Operation ~minus~ (with ~point~)

*Precondition:* ~U.IsOrdered() $\&\&$ v.IsDefined()~

*Semantics:* $U \backslash \{v\}$

*Complexity:* $O(log(n)+n)$

*/
    Points& operator-=( const Point& p );
/*
5.4.6 Operation ~inside~

*Precondition:* ~U.IsOrdered() $\&\&$ V.IsOrdered()~

*Semantics:* $U \subseteq V$

*Complexity:* $O(n+m)$, where ~n~ is the size of ~U~ and m the size of ~V~.

*/
  bool Inside( const Points& ps, const Geoid* geoid=0 ) const;
/*
6.4.4 Operation ~inside~ (with ~line~)

*Precondition:* ~U.IsOrdered() and V.IsOrdered()~

*Semantics:* $U \subseteq V$

*Complexity:* $O(m.n)$, where ~m~ is the size of ~U~ and ~n~ the size of ~V~.

*/
  bool Inside( const Line& l, const Geoid* geoid=0 ) const;
/*
6.4.4 Operation ~inside~ (with ~region~)

*Precondition:* ~U.IsOrdered() and V.IsOrdered()~

*Semantics:* $U \subseteq V$

*Complexity:* $O(m \log n)$, where ~m~ is the size of ~U~ and ~n~ the size of ~V~.

*/
    bool Inside( const Region& r, const Geoid* geoid=0 ) const;
/*
5.4.7 Operation ~intersects~ (with ~points~)

*Precondition:* ~U.IsOrdered() $\&\&$ V.IsOrdered()~

*Semantics:* $U \cap V \neq \emptyset$

*Complexity:* $O(m+n)$, where ~m~ is the size of ~U~ and ~n~ the size of ~V~.

*/
    bool Intersects( const Points& ps, const Geoid* geoid=0 ) const;
/*
5.4.7 Operation ~intersects~ (with ~line~)

*Precondition:* ~U.IsOrdered() $\&\&$ V.IsOrdered()~

*Semantics:* $U \cap V \neq \emptyset$

*Complexity:* $O(m \log n)$, where ~m~ is the size of ~U~ and ~n~ the size of ~V~.

*/
    bool Intersects( const Line& l, const Geoid* geoid=0 ) const;
/*
5.4.7 Operation ~intersects~ (with ~region~)

*Precondition:* ~U.IsOrdered() $\&\&$ V.IsOrdered()~

*Semantics:* $U \cap V \neq \emptyset$

*Complexity:* $O(m \log n)$, where ~m~ is the size of ~U~ and ~n~ the size of ~V~.

*/
    bool Intersects( const Region& r, const Geoid* geoid=0 ) const;
/*
5.4.7 Operation ~adjacent~ (with ~region~)

*Precondition:* ~U.IsOrdered() $\&\&$ V.IsOrdered()~

*Semantics:* $\partial U \cap \partial V \neq \emptyset \land U^0 \cap V^0 = \emptyset$

*Complexity:* $O(n.m)$, where ~n~ is the size of ~U~ and m the size of ~V~.

*/
  bool Adjacent( const Region& r, const Geoid* geoid=0 ) const;
/*
5.4.8 Operation ~intersection~

*/
  void Intersection(const Point& p, Points& result,
                    const Geoid* geoid=0) const;
  void Intersection( const Points& ps, Points& result,
                     const Geoid* geoid=0 ) const;
  void Intersection( const Line& l, Points& result,
                     const Geoid* geoid=0 ) const;
  void Intersection( const Region& r, Points& result ,
                     const Geoid* geoid=0) const;
/*
5.4.8 Operation ~minus~

*/
  void Minus( const Point& p, Points& result, const Geoid* geoid=0 ) const;
  void Minus( const Points& ps, Points& result, const Geoid* geoid=0 ) const;
  void Minus( const Line& l, Points& result, const Geoid* geoid=0 ) const;
  void Minus( const Region& r, Points& result, const Geoid* geoid=0 ) const;


/*
5.4.9 Operation ~union~

*/
  void Union(const Point& p, Points& result, const Geoid* geoid=0 ) const;
  void Union(const Points& ps, Points& result, const Geoid* geoid=0 ) const;
  void Union(const Line& line, Line& result, const Geoid* geoid=0 ) const;
  void Union(const Region& region, Region& result, const Geoid* geoid=0 ) const;



    double Distance( const Point& p, const Geoid* geoid=0 ) const;
/*
5.4.9 Operation ~distance~ (with ~points~)

*Precondition:* ~U.IsOrdered() and V.IsOrdered()~

*Semantics:* $\min\{ dist(u, v) | u \in U, v \in V \}$

*Complexity:* $O(m.n)$, where ~m~ is the size of ~U~ and ~n~ the size of ~V~

*/
    double Distance( const Points& ps, const Geoid* geoid=0 ) const;
/*
5.4.9 Operation ~distance~ (with ~rect2~)

*Precondition:* ~U.IsOrdered() and V.IsOrdered()~


*/
  double Distance( const Rectangle<2>& r, const Geoid* geoid=0 ) const;

/*
4.3.14 Operation ~translate~

*Precondition:* ~U.IsOrdered()~

*Semantics:* ~U + (x, y)~

*Complexity:* $O(n)$, where ~n~ is the size of ~U~

*/
  void Translate( const Coord& x, const Coord& y, Points& ps ) const;


/*
4.3.15 Operation ~rotate~

Rotates all contained points around the point defined by (x,y) with
angle ~alpha~.

*/

  void Rotate( const Coord& x, const Coord& y, double alpha,
               Points& res ) const;


/*
4.3.16 Operation ~center~

Computes the center of this points object.

*/
   Point theCenter() const;



/*
4.4 Object Traversal Functions

These functions are object traversal functions which are useful when we are
using ROSE algebra algorithms.

*Pre-condition:* ~IsOrdered()~

*Complexity:* All these functions have a complexity of $O( 1 )$ .

*/
    inline void SelectFirst() const;
/*
Puts the pointer ~pos~ to the first point in the ~points~ value.

*/
    inline void SelectNext() const;
/*
Moves the pointer ~pos~ to the next point in the ~points~ value.

*/
    inline bool EndOfPt() const;
/*
Decides whether ~pos~ is -1, which indicates that no more points in the ~points~ value
need to be processed.

*/
    inline bool GetPt( Point& p ) const;
/*
Gets the current point from the ~points~ value according to the ~pos~ pointer.

5.6 Functions needed to import the the ~points~ data type to tuple

There are totally 10 functions which are defined as virtual functions. They need
to be defined here in order for the Point data type to be used in Tuple definition
as an attribute.

*/
    inline int NumOfFLOBs() const;
    inline Flob* GetFLOB( const int i );
    inline size_t Sizeof() const;
    size_t HashValue() const;
    void CopyFrom( const Attribute* right );
    int Compare( const Attribute *arg ) const;
    int CompareAlmost( const Attribute *arg ) const;
    bool Adjacent( const Attribute *arg ) const;
    virtual Points* Clone() const;
    ostream& Print( ostream &os ) const;


    virtual uint32_t getshpType() const{
       return 8; // Point Type
    }

    virtual bool hasBox() const{
       return IsDefined();
    }

    virtual double getMinX() const{
      return bbox.MinD(0);
    }
    virtual double getMaxX() const{
      return bbox.MaxD(0);
    }
    virtual double getMinY() const{
      return bbox.MinD(1);
    }
    virtual double getMaxY() const{
      return bbox.MaxD(1);
    }

    virtual void writeShape(ostream& o, uint32_t RecNo) const{

       // first, write the record header
       WinUnix::writeBigEndian(o,RecNo);
       uint32_t size = points.Size();
       if(!IsDefined() || size==0){
         uint32_t length = 2;
         WinUnix::writeBigEndian(o,length);
         uint32_t type = 0;
         WinUnix::writeLittleEndian(o,type);
       } else {
         // length = 20 w for header
         // + 8* w for eacxh two doubles
         // w = 16 bit word
         uint32_t length = 20 + 8*size;
         WinUnix::writeBigEndian(o,length);
         WinUnix::writeLittleEndian(o,getshpType());
         double minX = getMinX();
         double maxX = getMaxX();
         double minY = getMinY();
         double maxY = getMaxY();
         WinUnix::writeLittle64(o,minX);
         WinUnix::writeLittle64(o,minY);
         WinUnix::writeLittle64(o,maxX);
         WinUnix::writeLittle64(o,maxY);
         // number of points
         WinUnix::writeLittleEndian(o,size);
         Point p(0,0);
         for(uint32_t i=0;i<size;i++){
            points.Get(i,&p);
            double x = p.GetX();
            double y = p.GetY();
            WinUnix::writeLittle64(o,x);
            WinUnix::writeLittle64(o,y);
         }
       }
    }


  static const string BasicType(){
    return "points";
  }
  static const bool checkType(const ListExpr type){
    return listutils::isSymbol(type, BasicType());
  }


  private:
/*
5.7 Private member functions

*/
    void Sort(const bool exact = true);
/*
Sorts the persistent array of points.

*/
    void RemoveDuplicates();
/*
Remove duplicates in the (ordered) array of points.

*/
    bool Find( const Point& p, int& pos, const bool& exact = true ) const;
/*
Searches (binary search algorithm) for a point in the point set and
returns its position. Returns -1 if the point is not found.

If exact is true, an exact search is done. If it is false, AlmostEqual
will be used instead of Equality. Use the first to find insertion
positions in the DBArray, the latter to just lookup keys to check if they
are contained.

5.8 Atrtibutes

*/
    DbArray<Point> points;
/*
The persistent array of points.

*/
    Rectangle<2> bbox;
/*
The bounding box that spatially contains all points.

*/
    bool ordered;
/*
The flag that indicates whether the persistent array is in ordered state.

*/
    mutable int pos;
/*
According to ROSE algebra, the carrier set of points should contain a pos pointer

*/
};

/*
5.9 Overloaded output operator

*/
ostream& operator<<( ostream& o, const Points& ps );



/*
6 Struct LRS

This struct implements the Linear Referencing System (LRS) ordering for lines. It basicaly contains
a position to the half segment in the line and its offset in the LRS. A line value will contain an
array ordered by these positions.

*/
struct LRS
{
  LRS() {}

  LRS( double lrsPos, int hsPos ):
  lrsPos( lrsPos ), hsPos( hsPos )
  {}

  LRS( const LRS& lrs ):
  lrsPos( lrs.lrsPos ), hsPos( lrs.hsPos )
  {}

  LRS& operator=( const LRS& lrs )
  { lrsPos = lrs.lrsPos; hsPos = lrs.hsPos; return *this; }

  double lrsPos;
  int hsPos;
};

/*
6 Class Line

This class implements the memory representation of the ~line~ type constructor. A line value is
actually composed of a set of arbitrarily arranged line segments. In the ROSE algebra paper, it
is called ~lines~.

A ~line~ value is a set of half segments. In the external (nested list) representation, a line value is
expressed as a set of segments. However, in the internal (class) representation, it is expressed
as a set of sorted half segments, which are stored in a DBArray.

*/
class Line: public StandardSpatialAttribute<2>
{
  public:
/*
6.1 Constructors and Destructor

*/
    explicit inline Line( const int n );
/*
Constructs an empty line allocating space for ~n~ half segments.

*/
    inline Line( const Line& cl );
/*
The copy constructor.

*/
    inline void Destroy();
/*
This function should be called before the destructor if one wants to destroy the
persistent array of half segments. It marks the persistent array for destroying. The
destructor will perform the real destroying.

*/
    inline ~Line() {}
/*
The destructor.

6.2 Functions for Bulk Load

As said before, the line is implemented as an ordered persistent array of half segments.
The time complexity of an insertion operation in an ordered array is $O(n)$, where ~n~
is the number of half segments. In some cases, bulk load of segments for example, it is good
to relax the ordered condition to improve the performance. We have relaxed this ordered
condition only for bulk load of half segments. All other operations assume that the set of
half segments is ordered.

*/
    inline bool IsOrdered() const;
/*
Returns whether the set of half segments is ordered. There is a flag ~ordered~ (see attributes)
in order to avoid a scan in the half segments set to answer this question.

*/
    void StartBulkLoad();
/*
Marks the begin of a bulk load of half segments relaxing the condition that the points must be
ordered.

*/
     void EndBulkLoad (bool sort = true,
                       bool realminize = true);
/*

Marks the end of a bulk load for this line.
If all parameters are set to __true__, the only condition to the content
of the Halfsegment array is that for each segment both corresponding Halfsegments are
included.

If ~sort~ is set to __false__, the halfsegments must be sorted using the
halfsegment order.

If ~realminize~ is set to __false__, the halfsegments has to be realminized. This means
each pair of different halfsegments has at most a common endpoint.
Furthermore, the edge numbers of the halfsegments must be the same for the
two halfsegments of a segment. The allowed range for the edge numbers is [0..Size()/2-1].

*/

/*
6.2 Member functions

*/

/*
length computed for metric (X,Y)-coordinates

*/
    inline double Length() const;

/*
length computed for geographic (LON,LAT)-coordinates and a Geoid
If any coordinate is invalid, ~valid~ is set to false (true otherwise).

*/
    double Length(const Geoid& g, bool& valid) const;

/*
Returns the length of the line, i.e. the sum of the lengths of all segments.

*/
    inline double SpatialSize() const{
      return Length();
    }

/*
Returns the length computed for geographic (LON,LAT)-coordinates and a Geoid
If any coordinate is invalid, ~valid~ is set to false (true otherwise).

*/
    inline double SpatialSize(const Geoid& g, bool& valid) const{
      return Length(g, valid);
    }


//    inline void SetLength( double length );
/*
Sets the length of the line.

*/
    inline const Rectangle<2> BoundingBox(const Geoid* geoid = 0) const;
/*
Returns the bounding box of the line.

*/
  //  inline void SetBoundingBox( const Rectangle<2>& bbox );
/*
Sets the bounding box of the line.

*/
    inline bool IsEmpty() const;
/*
Returns true iff the line is undefined or empty.

*/
    inline int Size() const;
/*
Returns the number of half segments in the line value.

*/
    bool Contains( const Point& p, const Geoid* geoid=0 ) const;
/*
Checks whether the point ~p~ is contained in the line

*/
    inline void Get( const int i, HalfSegment& hs ) const;
/*
Reads the ith half segment from the line value.

*/

    inline void Resize(const int newSize);
/*
Sets the new capacity of the halfsegment array to the
maximum of its original size and the argument.

*/
    inline void TrimToSize();
/*
Sets the new capacity of the halfsegment array to the
amount really required.

*/

    inline void Put( const int i, const HalfSegment& hs );
/*
Writes the the half segment ~hs~ to the ith position.

*/
    Line& operator=( const Line& cl );
/*
Assignement operator redefinition.

6.4 Operations

6.4.1 Operation $=$ (~equal~)

*Precondition:* ~U.IsOrdered() and V.IsOrdered()~

*Semantics:* $U == V$

*Complexity:* $O(m + n)$, where ~m~ is the size of ~U~ and ~n~ the size of ~V~.

*/
    bool operator==( const Line& cl ) const;
/*
6.4.2 Operation $\neq$ (~not equal~)

*Precondition:* ~U.IsOrdered() and V.IsOrdered()~

*Semantics:* $U != V$

*Complexity:* $O(m + n)$, where ~m~ is the size of ~U~ and ~n~ the size of ~V~.

*/
    bool operator!=( const Line& cl ) const;
/*
6.4.3 Operation ~union~

*Semantics:* $U \cup \{v\}$

*Complexity:* $O( 1 )$, if the set is not ordered; and $O(\log n + n)$, otherwise; where
~n~ is the size of ~U~.

*/
    Line& operator+=( const HalfSegment& hs );


/*
6.4.4 Oeration ~plus~

Appends all halfsegments from l to that line.
This instance must must be in bulkload mode.

*/
   Line& operator+=(const Line& l);

/*
6.4.4 Operation ~minus~

*Precondition:* ~U.IsOrdered()~

*Semantics:* $U \ \{v\}$

*Complexity:* $O(log(n)+n)$, where ~n~ is the size of ~U~.

*/
    Line& operator-=( const HalfSegment& hs );
/*
6.4.4 Operation ~intersects~ (with ~line~)

*Precondition:* ~U.IsOrdered() and V.IsOrdered()~

*Semantics:* $U \cap V \neq \emptyset$

*Complexity:* $O(m.n)$, where ~m~ is the size of ~U~ and ~n~ the size of ~V~.

*/
  bool Intersects( const Line& l, const Geoid* geoid=0 ) const;
/*
6.4.4 Operation ~intersects~ (with ~region~)

*Precondition:* ~U.IsOrdered() and V.IsOrdered()~

*Semantics:* $U \cap V \neq \emptyset$

*Complexity:* $O(m(n + \log n))$, where ~m~ is the size of ~U~ and ~n~ the size of ~V~.

*/
bool Intersects( const Region& r, const Geoid* geoid=0 ) const;



/*
6.4.4 Operation ~inside~ (with ~line~)

*Precondition:* ~U.IsOrdered() and V.IsOrdered()~

*Semantics:* $U \in V$

*Complexity:* $O(m.n)$, where ~m~ is the size of ~U~ and ~n~ the size of ~V~.

*/
    bool Inside( const Line& l, const Geoid* geoid=0 ) const;
/*
6.4.4 Operation ~inside~ (with ~region~)

*Precondition:* ~U.IsOrdered() and V.IsOrdered()~

*Semantics:* $U \in V$

*Complexity:* $O(m.log(n))$, where ~m~ is the size of ~U~ and ~n~ the size of ~V~.

*/
    bool Inside( const Region& r, const Geoid* geoid=0 ) const;
/*
6.4.4 Operation ~adjacent~ (with ~region~)

*Precondition:* ~U.IsOrdered() $\&\&$ V.IsOrdered()~

*Semantics:* $\partial U \cap \partial V \neq \emptyset \land U^0 \cap V^0 = \emptyset$

*Complexity:* $O(n.m)$, where ~n~ is the size of ~U~ and m the size of ~V~.

*/
    bool Adjacent( const Region& r, const Geoid* geoid=0 ) const;

/*
6.4.4 Operation ~intersection~

*/
  void Intersection(const Point& p, Points& result, const Geoid* geoid=0) const;
  void Intersection(const Points& ps, Points& result,
                    const Geoid* geoid=0) const;
  void Intersection( const Line& l, Line& result, const Geoid* geoid=0 ) const;
  void Intersection( const Region& l, Line& result,
                     const Geoid* geoid=0 ) const;
/*
6.4.4 Operation ~minus~

*/
  void Minus( const Point& l, Line& result, const Geoid* geoid=0 ) const;
  void Minus( const Points& l, Line& result, const Geoid* geoid=0 ) const;
  void Minus( const Line& l, Line& result, const Geoid* geoid=0 ) const;
  void Minus( const Region& l, Line& result, const Geoid* geoid=0 ) const;

/*
6.4.4 Operation ~union~

*/
  void Union( const Point& l, Line& result, const Geoid* geoid=0 ) const;
  void Union( const Points& l, Line& result, const Geoid* geoid=0 ) const;
  void Union( const Line& l, Line& result, const Geoid* geoid=0 ) const;
  void Union( const Region& l, Region& result, const Geoid* geoid=0 ) const;

/*
6.4.5 Operation ~crossings~

*Precondition:* ~U.IsOrdered() and V.IsOrdered()~

*Semantics:* $\{ p \in U \cap V | p \textrm{ is isolated in } U \cap V \}$

*Complexity:* $O(m.n + r\log r)$, where ~m~ is the size of ~U~, ~n~ the size of ~V~, and ~r~
is the ~points~ result size.

*/
  void Crossings( const Line& l, Points& result, const Geoid* geoid=0 ) const;


/*
6.4.5 Operation ~Crossings~

This operation returns all internal crossing nodes, i.e. all
points where more than 2 segments have a common endpoint.

*/

  void Crossings(Points& result, const Geoid* geoid=0) const;

/*
6.4.5 Operation ~distance~ (with ~point~)

*Precondition:* ~!U.IsEmpty() and v.IsDefined()~

*Semantics:* $\min \{ dist(u, v) | u \in U \}$

*Complexity:* $O(n)$, where ~n~ is the size of ~U~.

*/
    double Distance( const Point& p, const Geoid* geoid=0 ) const;
    double MaxDistance(const Point& p, const Geoid* geoid=0 ) const;
/*
6.4.5 Operation ~distance~ (with ~points~)

*Precondition:* ~!U.IsEmpty() and !V.IsEmpty()~

*Semantics:* $\min \{ dist(u, v) | u \in U, v \in V \}$

*Complexity:* $O(m.n)$, where ~m~ is the size of ~U~ and ~n~ is the size of ~V~.

*/
    double Distance( const Points& ps, const Geoid* geoid=0 ) const;
/*
6.4.5 Operation ~distance~ (with ~line~)

*Precondition:* ~!U.IsEmpty() and !V.IsEmpty()~

*Semantics:* $\min \{ dist(u, v) | u \in U, v \in V \}$

*Complexity:* $O(m.n)$, where ~m~ is the size of ~U~ and ~n~ is the size of ~V~.

*/
    double Distance( const Line& l, const Geoid* Geoid=0 ) const;

    void DistanceSmallerThan(const Line& l,
                            const double  maxDist,
                            const bool allowEqual,
                            CcBool& result,
                            const Geoid* geoid=0) const;


/*
6.4.5 Operation ~distance~ (with ~rect2~)

*Precondition:* ~!U.IsEmpty() and !V.IsEmpty()~

*/
  double Distance( const Rectangle<2>& r, const Geoid* geoid=0 ) const;


/*
5.4.9 Operation ~distance~ (with ~points~)

*Precondition:* ~U.IsOrdered() and V.IsOrdered()~

*Semantics:* $\min\{ dist(u, v) | u \in U, v \in V \}$

*Complexity:* $O(m.n)$, where ~m~ is the size of ~U~ and ~n~ the size of ~V~

*/
  double MaxDistance(const Rectangle<2>& r, const Geoid* geoid=0 ) const;
/*
6.4.5 Operation ~no\_components~

*Precondition:* ~U.IsOrdered()~

*Semantics:* the number of components of ~U~.

*Complexity:* $O(1)$

*/
    int NoComponents() const;
/*
4.3.14 Operation ~translate~

*Precondition:* ~U.IsOrdered()~

*Semantics:* ~U + (x, y)~

*Complexity:* $O(n)$, where ~n~ is the size of ~U~

*/
    void Translate( const Coord& x, const Coord& y, Line& l ) const;

    void Rotate( const Coord& x, const Coord& y, double alpha,
                    Line& l ) const;

/*
4.3.14 Operation ~transform~

*Precondition:* ~U.IsOrdered()~

*Semantics:* ~U -> R~

*Complexity:* $O(n)$, where ~n~ is the size of ~U~

*/
    void Transform( Region& r ) const;

/*
6.4.6 ~Simplify~

This function stores a simplified version into the argument __result__.
The simplification is performed within three steps.
In an initial step, simple lines are extracted from the original one.
The reason is, that junctions within the line should be kept.
In the second step. each simple line is simplified by removing
sampling points using the well known Douglas Peucker algorithm. By using this
algorithm, it's guarantet that the maximum derivation from the original line
is smaller or equal to epsilon (or zero if epsilon is smaller than zero).
Unfortunately by simplifying the line, new selfintersections can be
created. We remove them in a final step.


*/
    void Simplify(Line& result, const double epsilon,
                  const Points& importantPoint = Points(0),
                  const Geoid* geoid=0 ) const;



/*
~Realminize~

Removes overlapping segments and splits the line at all crossings
of the segments. May be that simple segments are represented by
many parts in the result.

*/
   void Realminize();


/*
6.4.6 Operation ~vertices~

*Precondition:* ~U.IsOrdered()~

*Semantics:* the vertices of ~U~

*Complexity:* $O(m)$, where ~m~ is the size of ~U~.

*/
    void Vertices( Points* result ) const;

/*
6.4.6 Operation ~boundary~

*Precondition:* ~U.IsOrdered()~

*Semantics:* the boundary of ~U~

*Complexity:* $O(m)$, where ~m~ is the size of ~U~.

*/
    void Boundary( Points* result ) const;

/*
4.4 Object Traversal Functions

These functions are object traversal functions which are useful when we are
using ROSE algebra algorithms.

*Pre-condition:* ~IsOrdered()~

*Complexity:* All these functions have a complexity of $O( 1 )$ .

*/
    inline void SelectFirst() const;
/*
Puts the pointer ~pos~ to the first half segment in the ~line~ value.

*/
    inline void SelectNext() const;
/*
Moves the pointer ~pos~ to the next half segment in the ~line~ value.

*/
    inline bool EndOfHs() const;
/*
Decides whether ~pos~ is -1, which indicates that no more half segments in the ~line~
value need to be processed.

*/
    inline bool GetHs( HalfSegment& hs ) const;
/*
Gets the current half segment from the ~line~ value according to the ~pos~ pointer.

6.7 Window clipping functions

6.7.1 WindowClippingIn

This function returns the part of the line that is inside the window.
The inside parameter is set to true if there is at least one segment
part inside the window. If the intersection part is a point, then
it is not considered in the result.

*/
    void WindowClippingIn( const Rectangle<2> &window,
                           Line &clippedLine,
                           bool &inside ) const;
/*
6.7.2 WindowClippingOut

This function returns the part of the line that is outside the window.
The outside parameter is set to true if there is at least one part of the segment
that is outside of the window. If the intersection part is a point, then
it is not considered in the result.

*/
    void WindowClippingOut( const Rectangle<2> &window,
                            Line &clippedLine,
                            bool &outside ) const;
/*
6.8 Functions needed to import the the ~line~ data type to tuple

There are totally 10 functions which are defined as virtual functions. They need
to be defined here in order for the Point data type to be used in Tuple definition
as an attribute.

*/
    inline int NumOfFLOBs() const
    {
      return 1;
    }

    inline Flob *GetFLOB( const int i )
    {
        return &line;
    }

    inline size_t Sizeof() const
    {
      return sizeof( *this );
    }

    inline bool Adjacent( const Attribute* arg ) const
    {
      return false;
    }

    size_t HashValue() const;
    void CopyFrom( const Attribute* right );
    int Compare( const Attribute *arg ) const;
//     int CompareAlmost( const Attribute *arg ) const;
    virtual Line *Clone() const;
    ostream& Print( ostream &os ) const;
    void Clear();



    virtual uint32_t getshpType() const{
       return 3; // PolyLine Type
    }

    virtual bool hasBox() const{
       return IsDefined();
    }

    virtual double getMinX() const{
      return bbox.MinD(0);
    }
    virtual double getMaxX() const{
      return bbox.MaxD(0);
    }
    virtual double getMinY() const{
      return bbox.MinD(1);
    }
    virtual double getMaxY() const{
      return bbox.MaxD(1);
    }

    virtual void writeShape(ostream& o, uint32_t RecNo) const{

       // first, write the record header
       WinUnix::writeBigEndian(o,RecNo);
       int size = line.Size();
       if(!IsDefined() || size==0){
         uint32_t length = 2;
         WinUnix::writeBigEndian(o,length);
         uint32_t type = 0;
         WinUnix::writeLittleEndian(o,type);
       } else {

         // first version: store each halfsegment as a single
         // polyline
         uint32_t segs = line.Size()/2;

         uint32_t length =  (44 + segs*4 + segs*4 * 8 )/ 2;

         WinUnix::writeBigEndian(o,length);
         // header end

         // type
         WinUnix::writeLittleEndian(o,getshpType());
         // box
         WinUnix::writeLittle64(o,getMinX());
         WinUnix::writeLittle64(o,getMinY());
         WinUnix::writeLittle64(o,getMaxX());
         WinUnix::writeLittle64(o,getMaxY());
         // numparts
         WinUnix::writeLittleEndian(o,segs);
         // numpoints
         WinUnix::writeLittleEndian(o,2*segs);
         // parts
         for(uint32_t i=0;i<segs;i++){
            WinUnix::writeLittleEndian(o,i*2);
         }
         // points
         HalfSegment hs;
         for(int i=0;i<line.Size();i++){
           line.Get(i,&hs);
           if(hs.IsLeftDomPoint()){
              Point p = hs.GetLeftPoint();
              WinUnix::writeLittle64(o,p.GetX());
              WinUnix::writeLittle64(o,p.GetY());
              p = hs.GetRightPoint();
              WinUnix::writeLittle64(o,p.GetX());
              WinUnix::writeLittle64(o,p.GetY());
           }
         }


       }
    }

   static void* Cast(void* addr){
      return new (addr) Line();
   }

   static const string BasicType(){
      return "line";
   }
   static const bool checkType(const ListExpr type){
     return listutils::isSymbol(type, BasicType());
   }

   const DbArray<HalfSegment>& GetArray() const{
     return line;
   }

   inline Line() {} // This constructor should only be used
                    // within the Cast function.
  private:

/*
6.10 Private member functions

*/
    void Sort();
/*
Sorts (quick-sort algorithm) the persistent array of half segments in the line value.

*/
    void RemoveDuplicates();
/*
Remove duplicates in the (ordered) array of half-segments.

*/
    bool Find( const HalfSegment& hs, int& pos,
               const bool& exact = false ) const;
/*
Searches (binary search algorithm) for a half segment in the line value and
returns its position. Returns false if the half segment is not found.

*/
    bool Find( const Point& p, int& pos, const bool& exact = false ) const;
/*
Searches (binary search algorithm) for a point in the line value and
returns its position, i.e. the first half segment with dominating point
less than or equal to ~p~. Returns true if the half segment dominating
point is equal to ~p~ and false otherwise.

*/
    void SetPartnerNo();
/*
Sets the partnerno attribute for all half segments of the line. The left half segment partnerno
points to the position to right one and the right half segment partnerno points to the position
to the left one.

*/
    bool
    GetNextSegment( const int poshs, const HalfSegment& hs,
                    int& posnexths, HalfSegment& nexths );
    bool
    GetNextSegments( const int poshs,
                     const HalfSegment& hs,
                     vector<bool>& visited,
                     int& posnexths,
                     HalfSegment& nexths,
                     stack< pair<int, HalfSegment> >& nexthss );
    void computeComponents();

    void collectFace(int faceno, int startPos, DbArray<bool>& used);
    int getUnusedExtension(int startPos, const DbArray<bool>& used) const;

/*
Calculates and sets the number of components for the line. For every half segment, the following
information is stored: faceno contains the number of the component, cycleno contains the
ramification that the segment belongs, and edgeno contains the segment number.

The method ~VisitHalfSegments~ is a recursive function that does the job for
~SetNoComponents~.

6.11 Attributes

*/
    DbArray<HalfSegment> line;
/*
The persisten array of half segments.

*/
    Rectangle<2> bbox;
/*
The bounding box that fully encloses all half segments of the line.

*/
    mutable int pos;
/*
The pointer to the current half segments. The pointer is important in object traversal algorithms.

*/
    bool ordered;
/*
Whether the half segments in the line value are sorted.

*/
    int noComponents;
/*
The number of components for the line.

*/
    double length;
/*
The length of the line.

*/

    int currentHS;
/*
Contains the number of the current HalfSegment for linear iteration of
a ~simple~ line.

*/
};

/*
6.12 overloaded output operator

*/
ostream& operator<<( ostream& o, const Line& cl );


/*
7 The class ~SimpleLine~

This class represents a line building a simple polyline, i.e. a line
without branches and zero or one components.

*/
class SimpleLine: public StandardSpatialAttribute<2>
{
  public:
/*
7.1 Constructors

~Constructor~

This constructor creates an undefined SimpleLine object and initializes the
contained arrays to have ~size~ number od slots.

*/
  explicit SimpleLine(int size):
            StandardSpatialAttribute<2>(false),
            segments(size),lrsArray(size/2),
            startSmaller(true),
            isCycle(false),isOrdered(true),length(0.0),
            bbox(false),currentHS(-1){ }

/*
~Constructor~


Constructs a ~SimpleLine~ from a complex one.

*/
  explicit SimpleLine(const Line& src);

/*
~CopyConstructor~

*/
  SimpleLine(const SimpleLine& src):
    StandardSpatialAttribute<2>(src.IsDefined()),
    segments(src.Size()),lrsArray(src.Size()){
    Equalize(src);
  }

/*
~Destructor~

*/
  ~SimpleLine() {}

/*
~Assignment Operator~

*/
  SimpleLine& operator=(const SimpleLine& src){
     Equalize(src);
     return *this;
  }

/*
~Destroy~

*/
  void Destroy(){
     segments.Destroy();
     lrsArray.Destroy();
  }


/*
~BulkLoading~

*/
  void StartBulkLoad();

  bool EndBulkLoad();

  SimpleLine& operator+=( const HalfSegment& hs);

  inline bool IsOrdered() const{
    return isOrdered;
  }

/*
~Length~

Returns the length of this SimpleLine (computed usind metric (X,Y)-coordinates);

*/
  inline double Length() const{
    return length;
  }


/*
Returns the simple line's length computed for geographic (LON,LAT)-coordinates
and a Geoid. If any coordinate is invalid, ~valid~ is set to false (true otherwise).

*/
  double Length(const Geoid& g, bool& valid) const;

  inline double SpatialSize() const{
    return Length();
  }

  inline double SpatialSize(const Geoid& g, bool& valid) const{
     return Length(g, valid);
  }


/*
~BoundingBox~

Returns the MBR of this SimpleLine object.

*/
  inline const Rectangle<2> BoundingBox(const Geoid* geoid = 0) const{
    if(geoid){ // spherical geometry case:
      if(!geoid->IsDefined() || !IsDefined()){
        return Rectangle<2>(false, 0.0, 0.0, 0.0, 0.0);
      }
      Rectangle<2> geobbox = Rectangle<2>(false);
      for (int i=0; i<Size() ;i++){
        HalfSegment hs;
        Get( i, hs );
        if( hs.IsLeftDomPoint() ){
          if(!geobbox.IsDefined()){
            geobbox = hs.BoundingBox(geoid);
          } else {
            geobbox = geobbox.Union(hs.BoundingBox(geoid));
          }
        } // else: ignore inverse HalfSegments
      } // end for
      return geobbox;
    } // else: euclidean MBR
    return bbox;
  }

/*
~IsEmpty~

Checks wether this line has no geometry.

*/
  inline bool IsEmpty() const{
    return !IsDefined() || (segments.Size() == 0);
  }

/*
~Size~

This function returns the number of halfsegments within this SimpleLine value.

*/
  inline int Size() const{
    return segments.Size();
  }

/*
~StartPoint~

Returns the start point of this simple line.

*/
  Point StartPoint(const bool startSmaller) const;
  Point StartPoint() const;

/*
~EndPoint~

Returns the end point of this simple line.

*/
  Point EndPoint(const bool startSmaller) const;
  Point EndPoint() const;

/*
~Contains~

Checks whether ~p~ is located on this line.

*/
  bool Contains(const Point& p, const Geoid* geoid=0) const;

/*
~TrimToSize~

Changes the capacities of the contained arrays to the required size.

*/
  inline void TrimToSize(){
     segments.TrimToSize();
     lrsArray.TrimToSize();
  }

/*
~StartsSmaller~

Returns true if SimpleLine start at smaller end point.

*/
  inline bool StartsSmaller(){
    return startSmaller;
  }

/*
~Comparison~

The operator __==__ checks whether the structures of two simple lines are equal.
This operator may return a __false__ result even if the lines have the same
geometry.

*/
  bool operator==(const SimpleLine& sl) const{
     if(!IsDefined() && !sl.IsDefined()){
       return true;
     }
     if(!IsDefined() || !sl.IsDefined()){
       return false;
     }
     if(bbox != sl.bbox){
       return false;
     }
     if(segments.Size() != sl.segments.Size()){
        return false;
     }
     if(startSmaller!=sl.startSmaller){
        return false;
     }
     if(!AlmostEqual(length,sl.length)){
       return false;
     }
     HalfSegment hs1;
     HalfSegment hs2;
     for(int i=0;i<segments.Size();i++){
       segments.Get(i,&hs1);
       sl.segments.Get(i,&hs2);
       if(!AlmostEqual(hs1, hs2)){
         return false;
       }
     }
     // the lrsArray is equals if the segments are equals
     return  true;
  }

  bool operator!=(const SimpleLine& sl) const{
     return !(*this == sl);
  }

/*
~Distance Operators~

*/

  double Distance(const Point& p, const Geoid* geoid=0 )const;

  double Distance(const Points& ps, const Geoid* geoid=0 ) const;

  double Distance(const SimpleLine& sl, const Geoid* geoid=0) const;

  double Distance(const Rectangle<2>& r, const Geoid* geoid=0) const;


/*
~SetStartSmaller~

*/
  void SetStartSmaller(const bool smaller) {
    startSmaller = smaller;
  }

/*
~GetStartSmaller~

*/
  bool GetStartSmaller() const {
    return IsDefined() && startSmaller;
  }

/*
~AtPosition~


*/
  bool AtPosition(double pos, const bool startsSmaller,
                  Point& p, const Geoid* geoid=0) const;
  bool AtPosition(double pos, Point& p, const Geoid* geoid = 0) const;

/*
~AtPoint~

*/

  bool AtPoint(const Point& p, const bool startsSmaller, double& result,
               const Geoid* geoid= 0) const;

  bool AtPoint(const Point& p, const bool startsSmaller, const double tolerance,
               double& result, const Geoid* geoid=0) const;
  bool AtPoint(const Point& p, double& result, const double tolerance = 0.0,
               const Geoid* geoid=0) const;

/*
~SubLine~

*/

void SubLine(const double pos1, const double pos2,
             bool startsSmaller, SimpleLine& l) const;
void SubLine(const double pos1, const double pos2, SimpleLine& l) const;

/*
~Crossings~

*/

void Crossings(const SimpleLine& l, Points& result,
               const Geoid* geoid=0) const;

/*
~Union~

*/
  bool Intersects(const SimpleLine& l, const Geoid* geoid=0) const;
  bool Contains (const Point& p, const Geoid* geoid = 0) const;

/*
~Attribute Functions~

The following functions are needed to act as an attribute type.

*/
  inline int NumOfFLOBs() const{
    return 2;
  }

  inline Flob* GetFLOB(const int i){
    if(i==0)
       return &segments;
    return &lrsArray;
  }


  inline size_t Sizeof() const{
     return sizeof(*this);
  }

  inline bool Adjacent(const Attribute* arg) const{
    return false;
  }
  size_t HashValue() const{
    return bbox.HashValue() + segments.Size();
  }

  void CopyFrom(const Attribute* right){
     Equalize(*(static_cast<const SimpleLine*>(right)));
  }

  int Compare(const Attribute* arg) const;

  bool operator<(SimpleLine *sl) const{
    return (Compare((Attribute*) sl) < 0);
  }

  virtual SimpleLine* Clone() const{
     SimpleLine* res =  new SimpleLine(*this);
     return  res;
  }

  ostream& Print(ostream& os) const;

  void Clear(){
     segments.clean();
     lrsArray.clean();
     SetDefined( true );
     bbox.SetDefined(false);
     length=0.0;
     isOrdered=true;
     currentHS = -1;
  }

  bool SelectInitialSegment( const Point& startPoint,
                             const double tolerance = 1.0,
                             const Geoid* geoid=0);

  bool SelectSubsequentSegment();

  bool getWaypoint(Point& destination) const;

  bool IsCycle()const {
    return isCycle;
  }

  void toLine(Line& result) const;

  void fromLine(const Line& src);
  void fromLine(const Line& src, const bool smaller);

  static void* Cast(void* addr){
    return new (addr) SimpleLine();
  }

  inline void Get( const int i, HalfSegment& hs ) const{
    segments.Get(i,&hs);
  }

  bool Get(LRS &lrs, int &i){
    return Find(lrs, i);
  }

  inline void Put(const int i, const HalfSegment& hs){
    segments.Put(i,hs);
  }


  inline void Get( const int i,  LRS& lrs ) const{
    lrsArray.Get(i,&lrs);
  }

  inline void Put(const int i, const LRS& lrs){
    lrsArray.Put(i,lrs);
  }

  inline void Resize(const int newSize){
    if(newSize>segments.Size()){
        segments.resize(newSize);
    }
  }

  static const string BasicType(){
    return "sline";
  }
  static const bool checkType(const ListExpr type){
    return listutils::isSymbol(type, BasicType());
  }

  private:
    DbArray<HalfSegment> segments;
    DbArray<LRS> lrsArray;
    bool startSmaller;
    bool isCycle;
    bool isOrdered;
    double length;
    Rectangle<2> bbox;
    int currentHS;

    void Equalize(const SimpleLine& src){
        HalfSegment seg;
        segments.copyFrom(src.segments);
        lrsArray.copyFrom(src.lrsArray);
        this->SetDefined( src.IsDefined() );
        this->startSmaller = src.startSmaller;
        this->isCycle = src.isCycle;
        this->isOrdered = src.isOrdered;
        this->length = src.length;
        this->bbox = src.bbox;
        this->currentHS = src.currentHS;
    }
/*
~StandardConstructor~

Only for use within the Cast function.

*/
   SimpleLine() { }

/*
~Find~

*/
bool Find( const Point& p, int& pos, const bool& exact = false ) const {
  assert( IsOrdered() );
  if( exact ){
    return segments.Find( &p, PointHalfSegmentCompare, pos );
  } else {
    return segments.Find( &p, PointHalfSegmentCompareAlmost, pos );
  }
}

 bool Find( const LRS& lrs, int& pos ) const {
   assert( IsOrdered() );

   if( IsEmpty() ){ // subsumes !IsDefined()
     return false;
   }

   if( lrs.lrsPos < 0 && !AlmostEqual( lrs.lrsPos, 0 ) &&
       lrs.lrsPos > Length() && !AlmostEqual( lrs.lrsPos, Length() ) ){
     return false;
   }

   lrsArray.Find( &lrs, LRSCompare, pos );
   if( pos > 0 ){
     pos--;
   }

   return true;
 }

/*
~Sort~

Sorts the array of HalfSegments.

*/
 void Sort(){
   segments.Sort(HalfSegmentCompare);
   isOrdered = true;
 }

/*
~SetPartnerNo~

Changes the partnerno of each HalfSegment to the index of the reverse
segment within the halfsegmenst array.

*/
 void SetPartnerNo();

/*
~computePolyline~

This function searches for a polyline within the halfsegment array and
creates the lrsarray. Additionally, the edge number of each segment is set
to the corresponding entry within the lrs array. If the segments does not
represent a simple polyLine, i.e. several components or branches, the result
of this function will be __false__.

*/
bool computePolyline();


};


ostream& operator<<(ostream& o, const SimpleLine& cl);

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
class EdgePoint;
/*
Forward declaration of class ~EdgePoint~

*/

class Region : public StandardSpatialAttribute<2>
{
  public:
/*
7.1 Constructors and Destructor

*/
    explicit inline Region( const int n );
/*
Constructs an empty region allocating space for ~n~ half segments.

*/
    Region( const Region& cr, bool onlyLeft = false );
/*
The copy constructor. If the flag ~onlyLeft~ is set, then only the half segments with left
dominating point are copied.

*/

    explicit Region( const Rectangle<2>& r );
/*
Creates a rectangular region from a rect2 objects.

*/

    Region( const Point& p1, const Point& p2, const Point& p3 );
/*
Creates a triangular region from three Point objects.

*/

    inline void Destroy();
/*
This function should be called before the destructor if one wants to destroy the
persistent array of half segments. It marks the persistent array for destroying. The
destructor will perform the real destroying.

*/
    inline ~Region() {}
/*
The destructor.

6.2 Functions for Bulk Load of Points

As said before, the region is implemented as an ordered persistent array of half segments.
The time complexity of an insertion operation in an ordered array is $O(n)$, where ~n~
is the number of half segments. In some cases, bulk load of segments for example, it is good
to relax the ordered condition to improve the performance. We have relaxed this ordered
condition only for bulk load of half segments. All other operations assume that the set of
half segments is ordered.

*/
    inline bool IsOrdered() const;
/*
Returns whether the set of half segments is ordered. There is a flag ~ordered~ (see attributes)
in order to avoid a scan in the half segments set to answer this question.

*/
    void StartBulkLoad();
/*
Marks the begin of a bulk load of half segments relaxing the condition that the half segments
must be ordered.

*/
     void EndBulkLoad( bool sort = true,
                       bool setCoverageNo = true,
                       bool setPartnerNo = true,
                       bool computeRegion = true );
/*
Marks the end of a bulk load and sorts the half segments set if the argument ~sort~ is set to true.

*/

     inline void Resize(const int newSize);
/*
Sets the new capacity of the halfsegment array to the
maximum of its original size and the argument.

*/
     inline void TrimToSize();
/*
Sets the new capacity of the halfsegment array to the
amount really required.

*/
   static void* Cast(void* addr) {
     return (new (addr) Region());
   }


/*
6.2 Member functions

*/
    inline void SetNoComponents( int noComponents )
    {
      this->noComponents = noComponents;
    }
/*
Sets the number of components with the given argument value ~noComponents~.

*/
    const Rectangle<2> BoundingBox(const Geoid* geoid = 0) const;
/*
Returns the bounding box of the region.

*/
    inline bool IsEmpty() const
    {
      return !IsDefined() || (Size() == 0);
    }
/*
Returns whether the ~region~ value is empty.

*/
    inline int Size() const
    {
      return region.Size();
    }
/*
Returns the number of half segments in the ~region~ value.

*/
    inline bool Get( const int i, HalfSegment& chs ) const
    {
      return region.Get( i, &chs );
    }
/*
Reads the ith half segment from the ~region~ value.

*/
    inline bool Put( const int i, const HalfSegment& hs )
    {
      return region.Put( i, hs );
    }
/*
Writes a halfsegment ~chs~ into position ~i~.

*/
    inline const AttrType& GetAttr( int position ) const;
/*
Reads the ~attr~ value of the half segment at the position ~position~ from the ~region~ value.

*/
    inline void UpdateAttr( int position, AttrType& attr );
/*
Updates the ~attr~ value of the half segment at position ~position~ from the ~region~ value.

*/
    inline void UpdateAttr( AttrType& attr );
/*
Updates the ~attr~ value of the current half segment from the ~region~ value.The current
half segment is indicated by ~pos~.

*/
    bool InsertOk( const HalfSegment& hs ) const;
/*
This function check whether a region value is valid after the insertion of a new half segment.
Whenever a half segment is about to be inserted, the state of the region is checked.
A valid region must satisfy the following conditions:

1) Any two cycles of the same region must be disconnected, which means that no edges
of different cycles can intersect each other;

2) Edges of the same cycle can only intersect in their endpoints, but not in their middle points;

3) For a certain face, the holes must be inside the outer cycle;

4) For a certain face, any two holes can not contain each other;

5) Faces must have the outer cycle, but they can have no holes;

6) For a certain cycle, any two vertex can not be the same;

7) Any cycle must be made up of at least 3 edges;

8) It is allowed that one face is inside another provided that their edges do not intersect.

*/
    Region& operator=( const Region& r );
/*
Assignement operator redefinition.

7.5 Operations

7.5.2 Operation $=$ (~equal~)

*Semantics:* $U == V$

*Complexity:* $O(m + n)$, where ~m~ is the size of ~U~ and ~n~ the size of ~V~.

*/
    bool operator==( const Region& c ) const;
/*
6.4.2 Operation $\neq$ (~not equal~)

*Precondition:* ~U.IsOrdered() and V.IsOrdered()~

*Semantics:* $U != V$

*Complexity:* $O(m + n)$, where ~m~ is the size of ~U~ and ~n~ the size of ~V~.

*/
    bool operator!=( const Region& r ) const;
/*
6.4.3 Operation ~union~

*Semantics:* $U \cup \{v\}$

*Complexity:* $O( 1 )$, if the set is not ordered; and $O(\log n + n)$, otherwise; where
~n~ is the size of ~U~.

*/
    Region& operator+=( const HalfSegment& hs );
/*
6.4.4 Operation ~minus~

*Precondition:* ~U.IsOrdered()~

*Semantics:* $U \ \{v\}$

*Complexity:* $O(log(n)+n)$, where ~n~ is the size of ~U~.

*/
    Region& operator-=( const HalfSegment& hs );
/*
6.4.4 Operation ~intersects~

*Precondition:* ~U.IsOrdered() and V.IsOrdered()~

*Semantics:* $U \cap V \neq \emptyset$

*Complexity:* $O()$, where ~m~ is the size of ~U~ and ~m~ the size of ~V~.

*/
  bool Intersects( const Region& r, const Geoid* geoid = 0 ) const;
/*
6.4.4 Operation ~intersection~

*/
  void Intersection(const Point& p, Points& result, const Geoid* geoid=0) const;
  void Intersection(const Points& ps, Points& result,
                    const Geoid* geoid=0) const;
  void Intersection(const Line& l, Line& result, const Geoid* geoid=0) const;
  void Intersection(const Region& r, Region& result,
                    const Geoid* geoid=0) const;

/*
6.4.4 Operation ~Union~

*/
  void Union(const Point& p, Region& result, const Geoid* geoid=0) const;
  void Union(const Points& ps, Region& result, const Geoid* geoid=0) const;
  void Union(const Line& l, Region& result, const Geoid* geoid=0) const;
  void Union(const Region& r, Region& result, const Geoid* geoid=0) const;


/*
6.4.4 Operation ~Minus~

*/
  void Minus(const Point& p, Region& result, const Geoid* geoid=0) const;
  void Minus(const Points& ps, Region& result, const Geoid* geoid=0) const;
  void Minus(const Line& l, Region& result, const Geoid* geoid=0) const;
  void Minus(const Region& r, Region& result, const Geoid* geoid=0) const;


/*
6.4.4 Operation ~inside~

*Precondition:* ~U.IsOrdered() and V.IsOrdered()~

*Semantics:* $U \subseteq V$

*Complexity:* $O(m.n)$, where ~m~ is the size of ~U~ and ~n~ the size of ~V~.

*/
  bool Inside( const Region& r, const Geoid* geoid=0 ) const;
/*
6.4.4 Operation ~adjacent~

*Precondition:* ~U.IsOrdered() $\&\&$ V.IsOrdered()~

*Semantics:* $\partial U \cap \partial V \neq \emptyset \land U^0 \cap V^0 = \emptyset$

*Complexity:* $O(m.n)$, where ~m~ is the size of ~U~ and ~n~ the size of ~V~.

*/
    bool Adjacent( const Region& r, const Geoid* geoid =0 ) const;
/*
6.4.4 Operation ~overlaps~

*Precondition:* ~U.IsOrdered() $\&\&$ V.IsOrdered()~

*Semantics:* $U^0 \cap V^0 \neq \emptyset$

*Complexity:* $O(m.n)$, where ~m~ is the size of ~U~ and ~n~ the size of ~V~.

*/
  bool Overlaps( const Region& r, const Geoid* geoid=0 ) const;
/*
6.4.4 Operation ~onborder~

*Precondition:* ~U.IsOrdered() $\&\&$ v.IsDefined()~

*Semantics:* $v \in \partial U$

*Complexity:* $O(m)$, where ~m~ is the size of ~U~.

*/
  bool OnBorder( const Point& p, const Geoid* geoid=0 ) const;
/*
6.4.4 Operation ~ininterior~

*Precondition:* ~U.IsOrdered() $\&\&$ v.IsDefined()~

*Semantics:* $v \in U^0$

*Complexity:* $O(m)$, where ~m~ is the size of ~U~.

*/
  bool InInterior( const Point& p, const Geoid* geoid=0 ) const;
/*
6.4.4 Operation ~distance~ (with ~point~)

*Precondition:* ~U.IsOrdered() $\&\&$ v.IsDefined()~

*Semantics:* $\min\{ dist(u, v) | u \in U \}$

*Complexity:* $O(m)$, where ~m~ is the size of ~U~.

*/
  double Distance( const Point& p, const Geoid* geoid=0 ) const;
/*
6.4.4 Operation ~distance~ (with ~points~)

*Precondition:* ~U.IsOrdered() $\&\&$ V.IsOrdered()~

*Semantics:* $\min\{ dist(u, v) | u \in U, v \in V \}$

*Complexity:* $O(m.n)$, where ~m~ is the size of ~U~ and ~n~ the size of ~V~.

*/
  double Distance( const Points& p, const Geoid* geoid=0 ) const;
/*
6.4.4 Operation ~distance~ (with ~points~)

*Precondition:* ~U.IsOrdered() $\&\&$ V.IsOrdered()~

*Semantics:* $\min\{ dist(u, v) | u \in U, v \in V \}$

*Complexity:* $O(m.n)$, where ~m~ is the size of ~U~ and ~n~ the size of ~V~.

*/
  double Distance( const Region& r, const Geoid* geoid=0 ) const;
/*
  6.4.4 Operation ~distance~ (with ~region~)

  *Precondition:* ~U.IsOrdered() $\&\&$ V.IsOrdered()~

  *Semantics:* $\min\{ dist(u, v) | u \in U, v \in V \}$

  *Complexity:* $O(m.n)$, where ~m~ is the size of ~U~ and ~n~ the size of ~V~.

*/

  double Distance(const Line& l, const Geoid* geoid=0) const;

  double Distance( const Rectangle<2>& r, const Geoid* geoid=0 ) const;
/*
  6.4.4 Operation ~distance~ (with ~rect2~)

  *Precondition:* ~U.IsOrdered() $\&\&$ V.IsOrdered()~

  *Semantics:* $\min\{ dist(u, v) | u \in U, v \in V \}$

  *Complexity:* $O(m.n)$, where ~m~ is the size of ~U~ and ~n~ the size of ~V~.

*/

/*
6.4.4 Operation ~components~

*Precondition:* ~U.IsOrdered()~

*Semantics:* returns the faces as a set (~vector~) of regions

*Complexity:* $O(n)$, where ~n~ is the size of ~U~.

The pointers inside the array ~components~ are here initialized
and must be deleted outside.

*/
  void Components( vector<Region*>& components );
/*
6.4.5 Operation ~touch[_]points~ (with ~line~)

*Precondition:* ~U.IsOrdered() and V.IsOrdered()~

*Semantics:* $\{ p \in U \cap \partial V | p \textrm{ is isolated in } U \cap \partial V \}$

*Complexity:* $O(m.n + r\log r)$, where ~m~ is the size of ~U~, ~n~ the size of ~V~, and ~r~
is the ~points~ result size.

*/
  void TouchPoints( const Line& l, Points& result, const Geoid* geoid=0 ) const;
/*
6.4.5 Operation ~touch[_]points~ (with ~region~)

*Precondition:* ~U.IsOrdered() and V.IsOrdered()~

*Semantics:* $\{ p \in \partial U \cap \partial V | p \textrm{ is isolated in } \partial U \cap \partial V \}$

*Complexity:* $O(m.n + r\log r)$, where ~m~ is the size of ~U~, ~n~ the size of ~V~, and ~r~
is the ~points~ result size.

*/
  void TouchPoints( const Region& r, Points& result,
                    const Geoid* geoid=0 ) const;
/*
6.4.5 Operation ~common[_]border~

*Precondition:* ~U.IsOrdered() and V.IsOrdered()~

*Semantics:* $\partial U \cap \partial V $

*Complexity:* $O(m.n + r\log r)$, where ~m~ is the size of ~U~, ~n~ the size of ~V~, and ~r~
is the ~line~ result size.

*/
  void CommonBorder( const Region& r, Line& result,
                     const Geoid* geoid=0 ) const;
/*
6.4.5 Operation ~no\_components~

*Precondition:* ~U.IsOrdered()~

*Semantics:* the number of components (faces) of ~U~

*Complexity:* $O(1)$

*/
    int NoComponents() const;
/*
6.4.5 Operation ~vertices~

*Precondition:* ~U.IsOrdered()~

*Semantics:* the vertices of ~U~

*Complexity:* $O(m)$, where ~m~ is the size of ~U~.

*/
    void Vertices( Points* result, const Geoid* geoid=0 ) const;


/*
6.4.5 Operation ~boundary~

*Precondition:* ~U.IsOrdered()~

*Semantics:* The boundary of ~U~

*Complexity:* $O(m)$, where ~m~ is the size of ~U~.

*/
    void Boundary(Line* result, const Geoid* geoid=0) const;

/*
4.4 Object Traversal Functions

These functions are object traversal functions which are useful when we are
using ROSE algebra algorithms.

*Pre-condition:* ~IsOrdered()~

*Complexity:* All these functions have a complexity of $O( 1 )$ .

*/
    inline void SelectFirst() const;
/*
Puts the pointer ~pos~ to the first half segment in the ~region~ value.

*/
    inline void SelectNext() const;
/*
Moves the pointer ~pos~ to the next half segment in the ~region~ value.

*/
    inline bool EndOfHs() const;
/*
Decides whether ~pos~ is -1, which indicates that no more half segments in the ~region~
value need to be processed.

*/
    inline bool GetHs(HalfSegment& hs ) const;
/*
Gets the current half segment from the ~region~ value according to the ~pos~ pointer.

7.8 contain function (point)

*Semantics:* This function decides whether a point is inside the region.

*Complexity:* $O( n )$  where ~n~ is the number of segments of the region.

*/
    bool Contains( const Point& p, const Geoid* geoid=0 ) const;
/*
7.9 innercontain function

*Semantics:* This function decides whether a point is inside the inner part of the region.

*Complexity:* $O( n )$  where ~n~ is the number of segments of the region.

*/
    bool InnerContains( const Point& p, const Geoid* geoid=0 ) const;
/*
7.10 contain function (segment)

*Semantics:* This function decides whether a half segment is inside the region.

*Complexity:* $O( n )$  where ~n~ is the number of segments of the region.

*/
    bool Contains( const HalfSegment& hs, const Geoid* geoid=0 ) const;
/*
7.10 inner contain function (segment)

*Semantics:* This function decides whether a half segment is completely inside the region.

*Complexity:* $O( n )$  where ~n~ is the number of segments of the region.

*/
    bool InnerContains( const HalfSegment& hs, const Geoid* geoid=0 ) const;
/*
7.10 intersects function (segment)

*Semantics:* This function decides whether a half segment intersects the region.

*Complexity:* $O( n )$  where ~n~ is the number of segments of the region.

*/
    bool Intersects( const HalfSegment& hs, const Geoid* geoid=0 ) const;
/*
7.11 holeedge-contain function

*Semantics:* This function decides whether a half segment is inside a hole edge of the region.

*Complexity:* $O( n )$  where ~n~ is the number of segments of the region.

*/
    bool HoleEdgeContain( const HalfSegment& hs, const Geoid* geoid=0 ) const;
/*
The following two functions are used to sort the half segments according to their attributes;

*/
    void LogicSort();
/*
7.12 Functions needed to import the the ~Region~ data type to tuple

There are totally 10 functions which are defined as virtual functions. They need
to be defined here in order for the Point data type to be used in Tuple definition
as an attribute.

*/
    inline int NumOfFLOBs() const
    {
      return 1;
    }

    inline Flob *GetFLOB( const int i )
    {
      return &region;
    }




    inline void SetDefined( bool defined )
    {
        if(!defined){
          Clear();
        }
        Attribute::SetDefined( defined );
    }

    inline size_t Sizeof() const
    {
      return sizeof( *this );
    }

    inline bool Adjacent( const Attribute* arg ) const
    {
      return false;
    }

    size_t HashValue() const;
    void CopyFrom( const Attribute* right );
    int Compare( const Attribute *arg ) const;
//     int CompareAlmost( const Attribute *arg ) const;
    ostream& Print( ostream &os ) const;
    virtual Region *Clone() const;
    void Clear();
    void SetEmpty();

/*
~Translate~

Moves the region according x and y and stores the result in result.


*/

    void Translate(const Coord& x, const Coord& y, Region& result) const;

    void Rotate(const Coord& x, const Coord& y, const double alpha,
                Region& result) const;

/*
~Translate~

Moves this region.

*/
    void Translate(const Coord& x, const Coord& y)
    {
       double t[2];
       t[0] = x;
       t[1] = y;
       bbox = bbox.Translate(t);
       int size = Size();
       HalfSegment hs;
       for(int i=0;i<size;i++)
       {
           Get(i,hs);
           hs.Translate(x,y);
           region.Put(i,hs);
       }
    }



/*

*Semantics:* This function sets the attribute InsideAbove of each region's half segment.
             This attribute indicates if the area of the region lies above or left of the half segment.

*Complexity:* $O( n log n ) $  where ~n~ is the number of segments of the region.

*/
/*
7.13 set partner number function

This function requires the edgenumbers set correctly.

*/
   void SetPartnerNo();


/*
7.14 Get cycle direction functions

These functions determine the direction of a region's cycle that was typed by
the user. It is returned true if the cycle is clockwise (the enclosed part is on
the right) or false if the cycle is counterclockwise (the cycle has the enclosed
part on the left). The points were typed in the order A, P and B, and P is the
leftmost point of the cycle.

*/
  static bool GetCycleDirection( const Point &pA,
                                 const Point &pP,
                                 const Point &pB);
  bool GetCycleDirection() const;

/*
7.15 window clipping functions

7.15.1 Window clipping IN function

This function returns the clipped half segments that are within a window, which
result from the clipping of a region to a clip window.

*/
  void WindowClippingIn(const Rectangle<2> &window,
                        Region &clippedRegion, const Geoid* geoid=0) const;

/*
7.15.2 Window clipping OUT function

This function returns the clipped half segments that are outside a window, which
result from the clipping of a region to a clip window.

*/

  void WindowClippingOut(const Rectangle<2> &window,
                         Region &clippedRegion, const Geoid* geoid=0) const;
/*
7.15.3 Get clipped half segment function

This function returns the clipped half segments resulting from the clipping of a
region to a clip window. It calls the ~GetClippedHSIn~ in order to get the clipped
half segments that are within the window, or ~GetClippedHSOut~ in order to get
the clipped half segments thar are outside the window. Afterwards it calls the
function ~CreateNewSegments~ to create the new half segments resulting from the connection
of the points that lies on the window's edges. Finally, it calls the function
~CreateNewSegmentsWindowVertices~ in order to create half segments corresponding
to the edges of the window. These half segments are only created if the window's
edge is completly inside the window.

*/
  void GetClippedHS(const Rectangle<2> &window,
                    Region &clippedRegion, bool inside,
                    const Geoid* geoid=0) const;
/*
7.15.4 Get clipped half segment IN function

This function returns the clipped half segments (that are within the window)
resulting from the clipping of a region to a clip window.

*/

  void GetClippedHSIn(const Rectangle<2> &window,
                      Region &clippedRegion,
                      vector<EdgePoint> pointsOnEdge[4],
                      int &partnerno, const Geoid* geoid=0) const;
/*
7.15.5 Get clipped half segment OUT function

This function returns the clipped half segments (that are outside the window)
resulting from the clipping of a region to a clip window.

*/
 void GetClippedHSOut(const Rectangle<2> &window,
                      Region &clippedRegion,
                      vector<EdgePoint> pointsOnEdge[4],
                      int &partnerno, const Geoid* geoid=0) const;
/*
7.15.6 Add clipped half segment function

This function is just used in order to add clipped half segments to the clipped
region.

*/
   void AddClippedHS( const Point &pl,
                      const Point &pr,
                      AttrType &attr,
                      int &partnerno );

/*
7.15.7 Clipped half segment on edge function

This function checks if the clipped half segment lies on one of the window's edges,
and if it happens the end points of the half segment are add to the corresponding
list of the ~points on edge~. The ~points on edge~ list are used to create the
new half segments that lies on edge.

*/
   static bool ClippedHSOnEdge(const Rectangle<2> &window,
                               const HalfSegment &chs,
                               bool clippingIn,
                               vector<EdgePoint> pointsOnEdge[4],
                               const Geoid* geoid=0);
/*
7.15.8 Create new segments function

This function creates the half segments resulting from the connection
of the points that lies on the window's edges.

*/
   static void CreateNewSegments(vector <EdgePoint>pointsOnEdge, Region &cr,
                                 const Point &bPoint,const Point &ePoint,
                                 WindowEdge edge,int &partnerno,bool inside,
                                 const Geoid* geoid=0);
/*
7.15.9 Create new segments function

This function creates new half segments corresponding to the edges of the window.
These half segments are only created if the window's edge is completly inside
the window.

*/
   void CreateNewSegmentsWindowVertices(const Rectangle<2> &window,
                                vector<EdgePoint> pointsOnEdge[4],Region &cr,
                                int &partnerno,bool inside,
                                const Geoid* geoid=0) const;
/*
7.15.10 Compute region function

This function computes a region from a list of half segments, in other orders
it sets the face number, cycle number and edge number of the half segments.
It calls the function ~GetNewFaceNo~ in order to get a new face number, and
it calls the function compute cycle to compute the cycle and edge numbers.
There are two pre-requisite: the partner number of the half segments must be already set,
and they need to be ordered in the half segment order.

*/
   void ComputeRegion();

/*
7.15.11 Compute cycle functions

The following functions are used to compute the cycle and edge numbers of cycles.

7.15.11.1. Compute cycle function

This function sets the cycle and edge number of a face's cycle. It calls the functions
~GetAdjacentHS~ and ~SearchForCriticalPoint~.

*/
   void ComputeCycle( HalfSegment &hs,
                      int faceno,
                      int cycleno,
                      int &edgeno,
                      bool *cycle );

/*

7.15.11.2. Is CriticalPoint function

Returns if a point (adjacentPoint) is a critical point.

*/
   bool IsCriticalPoint( const Point &adjacentPoint,
                         int chsPosition ) const;
/*

7.15.11.2. Get adjacent half segment function

This function returns the adjacent half segment of hs that hasn't set the
cycle and edge numbers yet. It also returns the point that belongs to both half segments, and
also if this point is a critical one.

*/
  bool GetAdjacentHS( const HalfSegment &hs, int hsPosition,
                      int &position, int partnerno,
                      int partnernoP, HalfSegment& adjacentCHS,
                      const Point &adjacentPoint, Point &newAdjacentPoint,
                      bool *cycle, int step) const;
/*

7.15.11.3. Search for critical point

This function returns if a half segment has critical point.

*/
   bool SearchForCriticalPoint(Point &p, int chsPosition) const;
/*
7.15.12 Get new face number function

This function finds the face number for a cycle. It finds it the cycle
is a hole of an existing face, or if it is a cycle of a new face.

*/
  int GetNewFaceNo(HalfSegment &chsS,bool *cycle);

  double Area(const Geoid* geoid = 0) const;

  double SpatialSize() const{
    return Area();
  }

  double SpatialSize(const Geoid& g, bool& valid) const{
    cerr << __PRETTY_FUNCTION__ << ": Function not implemented." << endl;
    assert(false);
    valid = false;
    return -666.0;
  }

  int GetNewFaceNo(const HalfSegment& hsIn, const int startpos) const;


/*
Returns the region's area.
The region must be defined!

*/

  virtual bool hasBox() const {
     return IsDefined() && !IsEmpty();
  }

  virtual void writeShape(ostream& o, uint32_t RecNo) const{
     saveShape(o,RecNo);
   }

   virtual double getMinX() const{
     return bbox.MinD(0);
   }
   virtual double getMaxX() const{
     return bbox.MaxD(0);
   }
   virtual double getMinY() const{
     return bbox.MinD(1);
   }
   virtual double getMaxY() const{
     return bbox.MaxD(1);
   }

   virtual uint32_t getshpType() const{
     return 5;
   }

  const DbArray<HalfSegment>& GetHalfSegments(){
      return region;
  }

  static const string BasicType(){
    return "region";
  }
  static const bool checkType(const ListExpr type){
    return listutils::isSymbol(type, BasicType());
  }

   inline Region() {}
/*
This constructor should not be used.

*/

  private:



/*
7.17 Private member functions

*/
    void Sort();
/*
sorts (quick-sort algorithm) the persistent array of half segments in the region value.

*/
    bool Find( const HalfSegment&, int& pos ) const;
    bool Find( const Point&, int& pos ) const;
/*
searches (binary search algorithm) for a half segment in the region value and
returns its position. Returns -1 if the half segment is not found.

*/

  void saveShape(ostream& o, uint32_t RecNo) const;

/*
Saves the region in shape format to o.

*/


/*
7.18 Atrtibutes

*/
    DbArray<HalfSegment> region;
/*
The database array of segments.

*/
    Rectangle<2> bbox;
/*
The bounding box that encloses the region.

*/
    int noComponents;
/*
The number of components (faces) of the region.

*/
    mutable int pos;
/*
The pointer to the current half segments. The pointer is important in object traversal algorithms.

*/
    bool ordered;
/*
Whether the half segments in the region value are sorted.

*/

};

/*
8 Function headers

*/
ostream& operator<<( ostream& o, const Region& cr );

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
    EdgePoint(): Point()
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
    Point *criticalPoint;
    Point nextPoint;

    SCycle(){}

    SCycle( const HalfSegment &hs, const int partnerno,
            const HalfSegment &hsP,const int partnernoP,
            Point *criticalP, const Point &nextPOINT)
    {
      hs1 = hs;
      hs2 = hsP;
      hs1PosRight = hs1PosLeft = partnernoP;
      hs2PosRight = hs2PosLeft = partnerno;
      hs1Partnerno = partnerno;
      hs2Partnerno = partnernoP;

      goToCHS1Right=goToCHS1Left=goToCHS2Right=goToCHS2Left=true;
      criticalPoint = criticalP;
      nextPoint = nextPOINT;
    }

    SCycle(const SCycle &sAux)
    {
      hs1 = sAux.hs1;
      hs2 = sAux.hs2;
      hs1PosRight  = sAux.hs1PosRight;
      hs1PosLeft   = sAux.hs1PosLeft;
      hs1Partnerno = sAux.hs1Partnerno ;
      goToCHS1Right = sAux.goToCHS1Right;
      goToCHS1Left  = sAux.goToCHS1Left;

      hs2PosRight  = sAux.hs2PosRight;
      hs2PosLeft   = sAux.hs2PosLeft;
      hs2Partnerno = sAux.hs2Partnerno ;
      goToCHS2Right = sAux.goToCHS2Right;
      goToCHS2Left  = sAux.goToCHS2Left;

      criticalPoint = sAux.criticalPoint;
      nextPoint     = sAux.nextPoint;

    }

    ~SCycle()
    {
      if (criticalPoint==NULL){
        delete criticalPoint;
        criticalPoint=NULL;
      }
    }
};

double Angle(const Point &v, const Point &p1, const Point &p2);
double VectorSize(const Point &p1, const Point &p2, const Geoid* geoid = 0);

/*
11 The inline functions

11.1 Class ~Point~

*/
inline Point::Point( const bool d, const Coord& x, const Coord& y ) :
  StandardSpatialAttribute<2>(d),
  x( x ),
  y( y )
{ }

inline Point::Point( const Point& p ) :
    StandardSpatialAttribute<2>(p.IsDefined()),
    x( p.x ), y( p.y )
{ }

inline const Rectangle<2> Point::BoundingBox(const Geoid* geoid /*=0*/) const
{
  assert( IsDefined() );
  if( IsDefined() ) {
    if( geoid && geoid->IsDefined() ){ // spherical case
      return Rectangle<2>( true,
                           MAX(-180.0,x - ApplyFactor(x)),
                           MIN(+180.0,x + ApplyFactor(x)),
                           MAX(- 90.0,y - ApplyFactor(y)),
                           MIN(+ 90.0,y + ApplyFactor(y)) );
    } else if(!geoid){
      return Rectangle<2>( true,
                          x - ApplyFactor(x),
                          x + ApplyFactor(x),
                          y - ApplyFactor(y),
                          y + ApplyFactor(y) );
    }
  } // else:
  return Rectangle<2>( false, 0.0, 0.0, 0.0, 0.0 );
}

inline void Point::Set( const Coord& x, const Coord& y )
{
  SetDefined( true );
  this->x = x;
  this->y = y;
}

inline void Point::Set(const Point& p)
{
  SetDefined(p.IsDefined());
  if (p.IsDefined()) Set(p.GetX(), p.GetY());
}

inline Point Point::Add( const Point& p, const Geoid* geoid /*=0*/ ) const
{
  assert( IsDefined() && p.IsDefined() );
  if( !IsDefined() || !p.IsDefined() || (geoid && !geoid->IsDefined()) ) {
    return Point( false, 0.0, 0.0 );
  }
  return Point( true, this->x + p.x, this->y + p.y );
}

inline Point& Point::operator=( const Point& p )
{
  SetDefined( p.IsDefined() );
  if( IsDefined() ){
    x = p.x;
    y = p.y;
  }
  return *this;
}

inline bool Point::operator==( const Point& p ) const
{
  if(!IsDefined() && !p.IsDefined()){
    return true;
  }
  if(!IsDefined() || !p.IsDefined()){
    return false;
  }

  return AlmostEqual(x, p.x) && AlmostEqual(y, p.y); // changed by TB

}

inline bool Point::operator==(const Points& ps) const{
   if(!IsDefined() && !ps.IsDefined()){
     return true;
   }
   if(!IsDefined() || !ps.IsDefined()){
     return false;
   }
   if(ps.Size()!=1){
     return false;
   }
   Point p1;
   ps.Get(0,p1);
   return AlmostEqual(*this,p1);
}


inline bool Point::operator!=( const Point& p ) const
{
  return !( *this == p );
}

inline bool Point::operator<=( const Point& p ) const
{
  return !( *this > p );
}

inline bool Point::operator<( const Point& p ) const
{
  assert( IsDefined() && p.IsDefined() );
  if( !IsDefined() ){
    return p.IsDefined();
  }
  if ( !p.IsDefined() ){
    return false;
  }
  bool eqx = AlmostEqual(x,p.x);
  return (!eqx && (x < p.x) ) ||
         (eqx && !AlmostEqual(y,p.y) && (y < p.y));
 //  return x < p.x || (x == p.x && y < p.y); // changed by TB
}

inline bool Point::operator>=( const Point& p ) const
{
  return !( *this < p );
}

inline bool Point::operator>( const Point& p ) const
{
  assert( IsDefined() && p.IsDefined() );
  if( !p.IsDefined() ){
    return IsDefined();
  }
  if ( !IsDefined() ){
    return false;
  }
  bool eqx = AlmostEqual(x,p.x);
  return (!eqx && (x > p.x)) ||
         (eqx && !AlmostEqual(y,p.y) && (y>p.y));
  //  return x > p.x || ( x == p.x && y > p.y ); // changed by TB
}




inline Point Point::operator+( const Point& p ) const
{
  assert( IsDefined() && p.IsDefined() );
  return Point( (IsDefined() && p.IsDefined()), x + p.x, y + p.y );
}

inline Point Point::operator-( const Point& p ) const
{
  assert( IsDefined() && p.IsDefined() );
  return Point( (IsDefined() && p.IsDefined()), x - p.x, y - p.y );
}

inline Point Point::operator*( const double d ) const
{
  assert( IsDefined() );
  return Point( IsDefined(), x * d, y * d );
}

inline size_t Point::Sizeof() const
{
  return sizeof( *this );
}

inline void Point::Translate(const Coord& x, const Coord& y){
  if( IsDefined() ){
    this->x += x;
    this->y += y;
  }
}

/*
11.2 Class ~Points~

*/
inline Points::Points( const int initsize ) :
StandardSpatialAttribute<2>(true),
points( initsize ),
bbox( false ),
ordered( true )
{ }

inline Points::Points( const Points& ps ) :
StandardSpatialAttribute<2>(ps.IsDefined()),
points( ps.Size() ),
bbox( ps.BoundingBox() ),
ordered( true )
{
  if( IsDefined() ) {
    assert( ps.IsOrdered() );
    points.copyFrom(ps.points);
  }
}

inline const Rectangle<2> Points::BoundingBox(const Geoid* geoid /*=0*/) const
{
  // no special implementation for spherical geometry required!
  return bbox;
}

inline bool Points::Get( const int i, Point& p ) const
{
  assert( IsDefined() );
  return points.Get( i, &p );
}

inline int Points::Size() const
{
  return points.Size();
}

inline bool Points::IsEmpty() const
{
  return !IsDefined() || (points.Size() == 0);
}

inline bool Points::IsOrdered() const
{
  return ordered;
}

inline int Points::NumOfFLOBs() const
{
  return 1;
}

inline bool Points::operator==(const Point& p) const{
   if(!IsDefined() && !p.IsDefined()){
     return true;
   }
   if(!IsDefined() || !p.IsDefined()){
     return false;
   }
   if(Size()!=1){
     return false;
   }
   Point p1;
   Get(0,p1);
   return AlmostEqual(p,p1);
}


inline Flob *Points::GetFLOB(const int i)
{
  assert( i >= 0 && i < NumOfFLOBs() );
  return &points;
}


inline size_t Points::Sizeof() const
{
  return sizeof( *this );
}

inline void Points::SelectFirst() const
{
  if( IsEmpty() ){
    pos = -1;
  } else {
    pos = 0;
  }
}

inline void Points::SelectNext() const
{
  if( pos >= 0 && pos < Size() - 1 ) {
    pos++;
  } else {
    pos = -1;
  }
}

inline bool Points::EndOfPt() const
{
  return pos == -1;
}

inline bool Points::GetPt(Point& p ) const
{
  if( pos >= 0 && pos <= Size()-1 ){
    points.Get( pos, &p );
    return true;
  }
  return false;
}

inline void Points::Resize(const int newSize){
  if(newSize>Size()){
    points.resize(newSize);
  }
}

inline void Points::TrimToSize(){
  points.TrimToSize();
}


/*
11.4 Class ~Line~

*/
inline Line::Line( const int n ) :
StandardSpatialAttribute<2>(true),
line( n ),
bbox( false ),
ordered( true ),
noComponents( 0 ),
length( 0.0 ),
currentHS( -1 )
{ }

inline Line::Line( const Line& cl ) :
StandardSpatialAttribute<2>(cl.IsDefined()),
line( cl.Size() ),
bbox( cl.bbox ),
ordered( true ),
noComponents( cl.noComponents ),
length( cl.length ),
currentHS ( cl.currentHS)
{
  if(!IsDefined())
    return;
  assert( cl.IsOrdered() );
  line.copyFrom(cl.line);
}

inline void Line::Destroy()
{
  line.Destroy();
}

inline double Line::Length() const
{
  assert( IsDefined() );
  return length;
}


inline const Rectangle<2> Line::BoundingBox(const Geoid* geoid /*=0*/) const
{
  if(geoid){ // spherical geometry case:
    if(!geoid->IsDefined() || !IsDefined()){
      return Rectangle<2>(false, 0.0, 0.0, 0.0, 0.0);
    }
    Rectangle<2> geobbox = Rectangle<2>(false);
    for(int i=0; i<Size() ;i++){
      HalfSegment hs;
      Get( i, hs );
      if( hs.IsLeftDomPoint() ){
        if(!geobbox.IsDefined()){
          geobbox = hs.BoundingBox(geoid);
        } else {
          geobbox = geobbox.Union(hs.BoundingBox(geoid));
        }
      } // else: ignore inverse HalfSegments
    } // end for
    return geobbox;
  } // else: euclidean MBR
  return bbox;
}

inline bool Line::IsOrdered() const
{
  return ordered;
}

inline bool Line::IsEmpty() const
{
  return !IsDefined() || (line.Size() == 0);
}

inline int Line::Size() const
{
  return line.Size();
}

inline void Line::Get( const int i, HalfSegment& hs ) const
{
  assert( IsDefined() );
  assert(i>=0);
  assert(i<line.Size());
  line.Get( i, &hs );
}

inline void Line::Resize(const int newSize){
   if(newSize>Size()){
      line.resize(newSize);
   }
}

inline void Line::TrimToSize(){
  line.TrimToSize();
}

inline void Line::Put( const int i, const HalfSegment& hs )
{
  assert( IsDefined() );
  line.Put( i, hs );
}

inline void Line::SelectFirst() const
{
  if( IsEmpty() )
    pos = -1;
  else pos = 0;
}

inline void Line::SelectNext() const
{
  if( pos >= 0 && pos < Size() - 1 )
    pos++;
  else pos = -1;
}

inline bool Line::EndOfHs() const
{
  return pos == -1;
}

inline bool Line::GetHs(  HalfSegment& hs ) const
{
  assert( IsDefined() );
  if( pos >= 0 && pos <= Size()-1 )
  {
    line.Get( pos, &hs );
    return true;
  }
  return false;
}


/*
11.4 Class ~Region~

*/
inline Region::Region( const int initsize ) :
StandardSpatialAttribute<2>(true),
region( initsize ),
bbox( false ),
noComponents( 0 ),
ordered( true )
{ }

inline void Region::Destroy()
{
  region.destroy();
}

inline const Rectangle<2> Region::BoundingBox(const Geoid* geoid /*=0*/) const
{
  if(geoid){ // spherical geometry case:
    if(!geoid->IsDefined() || !IsDefined()){
      return Rectangle<2>(false, 0.0, 0.0, 0.0, 0.0);
    }
    Rectangle<2> geobbox = Rectangle<2>(false);
    for (int i=0; i<Size() ;i++){
      HalfSegment hs;
      Get( i, hs );
      if( hs.IsLeftDomPoint() ){
        if(!geobbox.IsDefined()){
          geobbox = hs.BoundingBox(geoid);
        } else {
          geobbox = geobbox.Union(hs.BoundingBox(geoid));
        }
      } // else: ignore inverse HalfSegments
    } // endfor
  } // else: euclidean case
  return bbox;
}

inline bool Region::IsOrdered() const
{
  return ordered;
}

inline const AttrType& Region::GetAttr( int position ) const
{
  assert(( position>=0) && (position<=Size()-1));
  HalfSegment hs;
  region.Get( position, &hs);
  return hs.GetAttr();
}

inline void Region::UpdateAttr( AttrType& attr )
{
  if (( pos>=0) && (pos<=Size()-1))
  {
    HalfSegment hs;
    region.Get( pos, &hs);
    hs.SetAttr( attr );
    region.Put( pos, hs );
  }
}

inline void Region::UpdateAttr( int position, AttrType& attr )
{
  if (( position>=0) && (position<=Size()-1))
  {
    HalfSegment hs;
    region.Get( position, &hs );
    hs.SetAttr( attr );
    region.Put( position, hs );
  }
}

inline void Region::SelectFirst() const
{
  if( IsEmpty() )
    pos = -1;
  else pos = 0;
}

inline void Region::SelectNext() const
{
  if( pos >= 0 && pos < Size() - 1 )
    pos++;
  else pos = -1;
}

inline bool Region::EndOfHs() const
{
  return (pos==-1);
}

inline bool Region::GetHs(HalfSegment& hs ) const
{
  assert( IsDefined() );
  if( pos >= 0 && pos <= Size()-1 )
  {
    region.Get( pos, &hs );
    return true;
  }
  return false;
}

inline void Region::Resize(const int newSize){
  if(newSize>Size()){
    region.resize(newSize);
  }
}

inline void Region::TrimToSize(){
  region.TrimToSize();
}

/*
11.5 Class ~GrahamScan~

*/

/*
Classes suppoorting the computation of the convex hull of
an pointset.

*/
ostream& operator<<(ostream& o,const SimplePoint& p);

class SimplePoint{
  public:
     explicit SimplePoint(const Point* p){
       this->x = p->GetX();
       this->y = p->GetY();
     }

     explicit SimplePoint(Point& p){
       this->x = p.GetX();
       this->y = p.GetY();
     }

     SimplePoint(){
        x = 0;
        y = 0;
     }

     SimplePoint(double x, double y){
       this->x = x;
       this->y = y;
     }

     SimplePoint(const SimplePoint& p){
        this->x = p.x;
        this->y = p.y;
     }

     SimplePoint& operator=(const SimplePoint& p){
       this->x = p.x;
       this->y = p.y;
       return *this;
     }

     ~SimplePoint(){}

     SimplePoint relTo(const SimplePoint& p) const{
        return SimplePoint(this->x - p.x, this->y-p.y);
     }

     void makeRelTo(const SimplePoint& p){
        this->x -= p.x;
        this->y -= p.y;
     }

     SimplePoint moved(const double x0, const double y0)const{
        return SimplePoint(x+x0, y+y0);
     }

     SimplePoint reversed()const{
        return SimplePoint(-x,-y);
     }

     bool isLower(const SimplePoint& p)const{
        if(!AlmostEqual(y,p.y)){
           return y < p.y;
        }
        if(AlmostEqual(x,p.x)){ // equal points
           return false;
        }
        return x < p.x;
     }

     double mdist()const{ // manhatten distance to (0,0)
       return abs(x) + abs(y);
     }

     double mdist(const SimplePoint p)const{
        return abs(x-p.x) + abs(y-p.y);
     }

     bool isFurther(const SimplePoint& p)const{
        return mdist() > p.mdist();
     }

     bool isBetween(const SimplePoint& p0, const SimplePoint p1) const{
        return p0.mdist(p1) >= mdist(p0)+mdist(p1);
     }

     double cross(const SimplePoint& p)const{
        return x*p.y - p.x*y;
     }

     bool isLess(const SimplePoint& p) const{
        double f = cross(p);
        bool res;
        if(AlmostEqual(f,0.0)){
          res = isFurther(p);
        } else {
          res = f>0;
        }
        return  res;
     }

     bool operator<(const SimplePoint& p) const{
         return isLess(p);
     }

     bool operator==(const SimplePoint& p) const{
         return AlmostEqual(x,p.x)&& AlmostEqual(y,p.y);
     }

    bool operator>(const SimplePoint& p) const{
         return !(AlmostEqual(x,p.x) && AlmostEqual(y,p.y)) && !isLess(p);
     }

     double area2(const SimplePoint& p0, const SimplePoint& p1) const{
        return p0.relTo(*this).cross(p1.relTo(*this));
     }

     bool isConvex(const SimplePoint& p0, const SimplePoint& p1) const {
        double f = area2(p0,p1);
        if(AlmostEqual(f,0.0)){
           bool between = isBetween(p0,p1);
           return !between;
        }
        return f<0;
     }

     Point getPoint()const {
        return Point(true,x,y);
     }

     double getX()const{ return x;}
     double getY()const{ return y;}


  private:
     double x;
     double y;

}; // end of class SimplePoint


class GrahamScan{
public:
  static void convexHull(const Points* ps, Region* result){
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
   vector<SimplePoint> p;
   int n;
   int h;


  GrahamScan(const Points* ps){
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
     vector<SimplePoint>::iterator it= p.begin();
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
                  priority_queue<avlseg::ExtendedHalfSegment,
                                 vector<avlseg::ExtendedHalfSegment>,
                                 greater<avlseg::ExtendedHalfSegment> >& q1,
                  priority_queue<avlseg::ExtendedHalfSegment,
                                 vector<avlseg::ExtendedHalfSegment>,
                                 greater<avlseg::ExtendedHalfSegment> >& q2);

/*
~splitByNeighbour~

This function splits current (and neigbour) if required. neigbour
is replaces in ~sss~ by its left part (the part before the crossing)
and current is shortened to its left part. The remainding parts (the right parts)
are inserted into the corresponding queues depending on the woner of ~current~ and
~neighbour~.

*/

bool splitByNeighbour(avltree::AVLTree<avlseg::AVLSegment>& sss,
                      avlseg::AVLSegment& current,
                      avlseg::AVLSegment const*& neighbour,
                      priority_queue<avlseg::ExtendedHalfSegment,
                                vector<avlseg::ExtendedHalfSegment>,
                                greater<avlseg::ExtendedHalfSegment> >& q1,
                      priority_queue<avlseg::ExtendedHalfSegment,
                                vector<avlseg::ExtendedHalfSegment>,
                                greater<avlseg::ExtendedHalfSegment> >& q2);

/*
~splitByNeighbours~

Splits the segments ~leftN~ and ~rightN~ at their crossing point and
replaces them by their left parts in ~sss~. The right parts are
inserted into the queues.

*/
void splitNeighbours(avltree::AVLTree<avlseg::AVLSegment>& sss,
                     avlseg::AVLSegment const*& leftN,
                     avlseg::AVLSegment const*& rightN,
                     priority_queue<avlseg::ExtendedHalfSegment,
                                    vector<avlseg::ExtendedHalfSegment>,
                                    greater<avlseg::ExtendedHalfSegment> >& q1,
                     priority_queue<avlseg::ExtendedHalfSegment,
                                    vector<avlseg::ExtendedHalfSegment>,
                                    greater<avlseg::ExtendedHalfSegment> >& q2);

/*
~SetOp~

This function realizes some set operations on two line objects.
~op~ may be instantiated with avlseg::union[_]op, avlseg::difference[_]op,
or avlseg::intersection[_]op.

*/
Line* SetOp(const Line& line1, const Line& line2, avlseg::SetOperation op,
            const Geoid* geoid=0 );

/*
~SetOp~

This function realizes some set operations on two region objects.
~op~ may be instantiated with avlseg::union[_]op, avlseg::difference[_]op,
or avlseg::intersection[_]op.

*/
Region* SetOp(const Region& reg1, const Region& reg2, avlseg::SetOperation op,
              const Geoid* geoid=0 );



void SetOp(const Line& line, const Region& region,
           Line& result, avlseg::SetOperation op, const Geoid* geoid=0);

void SetOp(const Line& line, const Region& region,
           Region& result, avlseg::SetOperation op, const Geoid* geoid=0);


void SetOp(const Line& line1, const Line& line2,
           Line& result, avlseg::SetOperation op, const Geoid* geoid=0);

void SetOp(const Region& reg1, const Region& reg2,
           Region& result, avlseg::SetOperation op, const Geoid* geoid=0);
/*
~Realminize~

Creates a reamlinized representation of the HalfSegments in ~segments~. This means,
if halfsegments overlap, only one of the segments left. If halfsegments are crossing
or touching at their interior, their are split.

*/
DbArray<HalfSegment>* Realminize(const DbArray<HalfSegment>& segments);

/*
~Split~

This function is similar to Realminize. In contrast to that function, segments covering the
same space left instead to replace by a single segment.

*/
DbArray<HalfSegment>* Split(const DbArray<HalfSegment>& segments);

/*
~hasOverlaps~

This function checks whether in ~segments~ at least two segments overlap.

*/
bool hasOverlaps(const DbArray<HalfSegment>& segments,
                 const bool ignoreEqual=false);


/*
~isSpatialType~

This function checks whether ~type~ represents a spatial type, i.e. point, points,
line, region.

*/
bool IsSpatialType(ListExpr type);




avlseg::ownertype selectNext(const Region& reg1,
                     int& pos1,
                     const Region& reg2,
                     int& pos2,
                     priority_queue<avlseg::ExtendedHalfSegment,
                                    vector<avlseg::ExtendedHalfSegment>,
                                    greater<avlseg::ExtendedHalfSegment> >& q1,
                     priority_queue<avlseg::ExtendedHalfSegment,
                                    vector<avlseg::ExtendedHalfSegment>,
                                    greater<avlseg::ExtendedHalfSegment> >& q2,
                     avlseg::ExtendedHalfSegment& result,
                     int& src // for debugging only
                    );

/*
~selectNext~

The following set of functions determines the smallest Halfsegment from the
arguments  HalfSegment at position ~pos1~ of ~line1~, HalfSegment at position
~pos2~ of ~line2~ and the heads of the queues. The smallest halfsegment is
returned in result. The return value is __avlseg::first__ iff the Halfsegment
comes from ~line1~ or ~q1~, __avlseg::second__ if the HalfSegment comes from
~line2~ or ~q2~ and __avlseg::none__ if all arguments are exhausted.
The argument ~src~ is used for debugging purposed and returns the exact source
of the smallest halfSegment.


*/
avlseg::ownertype selectNext(const Line& line1,
                     int& pos1,
                     const Line& line2,
                     int& pos2,
                     priority_queue<avlseg::ExtendedHalfSegment,
                                    vector<avlseg::ExtendedHalfSegment>,
                                    greater<avlseg::ExtendedHalfSegment> >& q1,
                     priority_queue<avlseg::ExtendedHalfSegment,
                                    vector<avlseg::ExtendedHalfSegment>,
                                    greater<avlseg::ExtendedHalfSegment> >& q2,
                     avlseg::ExtendedHalfSegment& result,
                     int& src
                    );

avlseg::ownertype selectNext(const Line& line,
                     int& pos1,
                     const Region& region,
                     int& pos2,
                     priority_queue<avlseg::ExtendedHalfSegment,
                                    vector<avlseg::ExtendedHalfSegment>,
                                    greater<avlseg::ExtendedHalfSegment> >& q1,
                     priority_queue<avlseg::ExtendedHalfSegment,
                                    vector<avlseg::ExtendedHalfSegment>,
                                    greater<avlseg::ExtendedHalfSegment> >& q2,
                     avlseg::ExtendedHalfSegment& result,
                     int& src
                    );

avlseg::ownertype selectNext( const Line&  line,
                      priority_queue<avlseg::ExtendedHalfSegment,
                                     vector<avlseg::ExtendedHalfSegment>,
                                     greater<avlseg::ExtendedHalfSegment> >& q,
                      int& posLine,
                      const Points&  point,
                      int& posPoint,
                      avlseg::ExtendedHalfSegment& resHs,
                      Point& resPoint);

avlseg::ownertype selectNext( const Line& line,
                      priority_queue<avlseg::ExtendedHalfSegment,
                                     vector<avlseg::ExtendedHalfSegment>,
                                     greater<avlseg::ExtendedHalfSegment> >& q,
                      int& posLine,
                      const Point& point,
                      int& posPoint, // >0: point already used
                      avlseg::ExtendedHalfSegment& resHs,
                      Point& resPoint);

void Realminize2(const Line& src, Line& result);

void CommonBorder( const Region& reg1, const Region& reg2, Line& result,
                   const Geoid* geoid = 0);

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



#endif // __SPATIAL_ALGEBRA_H__
