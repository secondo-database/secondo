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

#include <fstream>
#include <stack>
#include <vector>
#include <queue>
#include "StandardAttribute.h"
#include "DBArray.h"
#include "RectangleAlgebra.h"
#include "WinUnix.h"
#include "AvlTree.h"
#include "Symbols.h"


typedef double Coord;
/*
Coordinates are represented by real numbers.

*/

#define PI 3.146264371
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

/*
Forward declarations.

3 Auxiliary Functions

*/
// const double FACTOR = 0.00000001; // moved to Attribute.h

inline double ApplyFactor( const double d );
inline bool AlmostEqual( const Point& p1,
                         const Point& p2 );
inline bool AlmostEqual( const HalfSegment& hs1,
                         const HalfSegment& hs2 );


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


/*
4 Struct Point

This class implements the memory representation of the ~point~ type
constructor. A point represents a point in the Euclidean plane or is
undefined. The figure \cite{fig:spatialdatatypes.eps} shows an example
of a point data type.

*/
class Point: public StandardSpatialAttribute<2>
{
  public:
/*
4.1. Constructors and Destructor

*/
    inline Point() {};
/*
This constructor should not be used.

There are two ways of constructing a point:

*/
    inline Point( const bool d,
                  const Coord& x = Coord(),
                  const Coord& y = Coord() );
/*
The first one receives a boolean value ~d~ indicating if the point is defined
and two coordinate ~x~ and ~y~ values.

*/
    inline Point( const Point& p );
/*
The second one receives a point ~p~ as argument and creates a point that is a
copy of ~p~.

*/
    inline ~Point()
    {}
/*
The destructor.

4.2 Member functions

*/
    inline const Coord& GetX() const
    {
      return x;
    }
/*
Returns the ~x~-coordinate.

*/
    inline const Coord& GetY() const
    {
      return y;
    }
/*
Returns the ~y~-coordinate.

*/
    inline const Rectangle<2> BoundingBox() const;
/*
Returns the point bounding box which is also a point.

*/
    inline void Set( const Coord& x, const Coord& y );
/*
Sets the value of the point object.

*/
    inline Point& operator=( const Point& p );
    inline bool operator<=( const Point& p ) const;
    inline bool operator<( const Point& p ) const;
    inline bool operator>=( const Point& p ) const;
    inline bool operator>( const Point& p ) const;
    inline Point operator+( const Point& p ) const;
    inline Point operator-( const Point& p ) const;
    inline Point operator*( const double d ) const;
/*
Operators redefinition.

4.3 Operations

4.3.1 Operation ~scale~

*Precondition:* ~u.IsDefined()~

*Semantics:* $factor * u$

*Complexity:* $O(1)$

*/
    inline void Scale( const Coord& factor )
    {
      assert( IsDefined() );
      this->x *= factor;
      this->y *= factor;
    }
/*
4.3.1 Operation $=$ (~equal~)

*Precondition:* ~u.IsDefined() $\&\&$ v.IsDefined()~

*Semantics:* $u = v$

*Complexity:* $O(1)$

*/
    inline bool operator==( const Point& p ) const;
/*
4.3.2 Operation $\neq$ (~not equal~)

*Precondition:* ~u.IsDefined() $\&\&$ v.IsDefined()~

*Semantics:* $u \neq v$

*Complexity:* $O(1)$Algebras/RTree/RTree.examples

*/
    inline bool operator!=( const Point& p ) const;
/*
4.3.7 Operation ~inside~ (with ~points~)

*Precondition:* ~u.IsDefined() $\&\&$ V.IsOrdered()~

*Semantics:* $u \in V$

*Complexity:* $O(log(n))$, where ~n~ is the size of the point set ~V~

*/
    bool Inside( const Points& ps ) const;
/*
4.3.8 Operation ~inside~ (with ~rectangle~)

*Precondition:* ~u.IsDefined()~

*Semantics:* $u \in V$

*Complexity:* $O(1)$

*/
    bool Inside( const Rectangle<2>& r ) const;
/*
6.4.4 Operation ~inside~ (with ~line~)

*Precondition:* ~u.IsDefined() and V.IsOrdered()~

*Semantics:* $u \in V$

*Complexity:* $O(n)$, where ~n~ is the size of ~V~.

*/
    bool Inside( const Line& l ) const;
/*
6.4.4 Operation ~inside~ (with ~region~)

*Precondition:* ~u.IsDefined() and V.IsOrdered()~

*Semantics:* $u \in V$

*Complexity:* $O(log(n))$, where ~n~ is the size of ~V~.

*/
    bool Inside( const Region& r ) const;
/*
4.3.13 Operation ~distance~ (with ~point~)

*Precondition:* ~u.IsDefined()~ and ~v.IsDefined()~

*Semantics:* $dist(u,v) = \sqrt{(u.x - v.x)^2 + (u.y - v.y)^2}$

*Complexity:* $O(1)$

*/
    double Distance( const Point& p ) const;
/*
4.3.13 Operation ~distance~ (with ~points~)

*Precondition:* ~u.IsDefined()~ and ~V.IsOrdered()~

*Semantics:* $\min\{ dist(u, v) | v \in V \}$

*Complexity:* $O(n)$, where ~n~ is the size of ~V~

*/
    double Distance( const Points& ps ) const;
/*
4.3.13 Operation ~distance~ (with ~rect2~)

*Precondition:* ~u.IsDefined()~ and ~V.IsOrdered()~

*/
    double Distance( const Rectangle<2>& r ) const;
/*
4.3.13 Operation ~direction~

*Precondition:* ~u.IsDefined()~ and ~v.IsDefined()~

*Semantics:* returns the angle of the line from ~u~ to ~v~, measured in degrees.

*Complexity:* $O(1)$

*/
    double Direction( const Point& p ) const;
/*
4.3.14 Operation ~translate~

*Precondition:* ~u.IsDefined()~

*Semantics:*  ~(u.x + x, u.y + y)~

*Complexity:* $O(1)$

*/
    inline Point Translate( const Coord& x, const Coord& y ) const;
    inline void Translate( const Coord& x, const Coord& y );


/*
4.3.15 Operation ~rotate~

This function rotates this point around the point defined by (x,y)
with a degree of alpha. The result is stored in res.

*/

   inline void Rotate(const Coord& x, const Coord& y, const double alpha,
                      Point& res) const;


/*
4.3.15 Operation ~add~

*Precondition:* ~p1.IsDefined(), p2.IsDefined()~

*Semantics:*  ~(p1.x + p2.x, p1.y + p2.y)~

*Complexity:* $O(1)$

*/
    inline Point Add( const Point& p ) const;

    inline bool IsEmpty()const{
      return !IsDefined();
    }

/*
4.4 Functions needed to import the the ~point~ data type to tuple

There are totally 8 functions which are defined as virtual functions. They need
to be defined here in order for the Point data type to be used in Tuple definition
as an attribute.

*/
    inline size_t Sizeof() const;

    inline size_t HashValue() const
    {
      if( !IsDefined() )
        return 0;
      return (size_t)(5*x + y);
    }

    inline void CopyFrom( const StandardAttribute* right )
    {
      const Point* p = (const Point*)right;
      SetDefined( p->IsDefined() );
      if( IsDefined() )
        Set( p->x, p->y );
    }

    inline int Compare( const Attribute *arg ) const
    { // CD: Implementation following guidelines from Attribute.h:
      const Point* p = (const Point*)arg;
      if( !IsDefined() && !p->IsDefined() )
        return 0;
      if( !IsDefined() && p->IsDefined() )
        return -1;
      if( !p->IsDefined() )
        return 1;
      if( *this > *p )
        return 1;
      if( *this < *p )
        return -1;
      return 0;
    }

    inline int CompareAlmost( const Attribute *arg ) const
    {
      const Point* p = (const Point*)arg;
      if( !IsDefined() && !p->IsDefined() )
        return 0;
      if( !IsDefined() && p->IsDefined() )
        return -1;
      if( !p->IsDefined() )
        return 1;
      if( AlmostEqual( *this, *p ) )
        return 0;
      if( *this > *p )
        return 1;
      if( *this < *p )
        return -1;
      return 0;
    }

    inline bool Adjacent( const Attribute *arg ) const
    {
      return false;
    }

    virtual inline Point* Clone() const
    {
      return new Point( *this );
    }

    ostream& Print( ostream &os ) const;

    virtual uint32_t getshpType() const{
       return 1; // Point Type
    }

    virtual bool hasBox(){
       return IsDefined();
    }

    virtual double getMinX() const{
      return x;
    }
    virtual double getMaxX() const{
      return x;
    }
    virtual double getMinY() const{
      return y;
    }
    virtual double getMaxY() const{
      return y;
    }

    virtual void writeShape(ostream& o, uint32_t RecNo) const{

       // first, write the record header
       WinUnix::writeBigEndian(o,RecNo);

       if(!IsDefined()){
         uint32_t length = 2;
         WinUnix::writeBigEndian(o,length);
         uint32_t type = 0;
         WinUnix::writeLittleEndian(o,type);
       } else {
         uint32_t length = 10;
         WinUnix::writeBigEndian(o,length);
         uint32_t type = 1;
         WinUnix::writeLittleEndian(o,type);
         WinUnix::writeLittle64(o,x);
         WinUnix::writeLittle64(o,y);
       }
    }

    static const string BasicType(){
       return symbols::POINT;
    }


  protected:
/*
4.5 Attributes

*/
    Coord x;
/*
The ~x~ coordinate.

*/
    Coord y;
/*
The ~y~ coordinate.

*/
};

/*
4.6 Auxiliary functions

*/
ostream& operator<<( ostream& o, const Point& p );
ostream& operator<<( ostream& o, const Point& p );

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
    inline Points( const int initsize );
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
      points.Destroy();
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
    void EndBulkLoad( bool sort = true, bool remDup = true );
/*
Marks the end of a bulk load and sorts the point set if the argument ~sort~ is set to true.

5.3 Member functions

*/
    inline const Rectangle<2> BoundingBox() const;
/*
Returns the bounding box that spatially contains all points.

*/
    inline bool IsEmpty() const;
/*
Returns whether the set is empty of not.

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

    inline void Get( const int i, Point const*& p ) const;
/*
Retrieves the point ~p~ at position ~i~ in the point set.

*Precondition:* $0 \leq i < Size()$

*/
    Points& operator=( const Points& ps );
/*
Assignement operator redefinition.

*/
    bool Contains( const Point& p ) const;
/*
Searches (binary search algorithm) for a point in the point set and
return ~true~ if found and ~false~ if not.

*Precondition:* ~this.IsOrdered() $\&\&$ p.IsDefined()~

*/
    bool Contains( const Points& ps ) const;
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
    bool Inside( const Points& ps ) const;
/*
6.4.4 Operation ~inside~ (with ~line~)

*Precondition:* ~U.IsOrdered() and V.IsOrdered()~

*Semantics:* $U \subseteq V$

*Complexity:* $O(m.n)$, where ~m~ is the size of ~U~ and ~n~ the size of ~V~.

*/
    bool Inside( const Line& l ) const;
/*
6.4.4 Operation ~inside~ (with ~region~)

*Precondition:* ~U.IsOrdered() and V.IsOrdered()~

*Semantics:* $U \subseteq V$

*Complexity:* $O(m \log n)$, where ~m~ is the size of ~U~ and ~n~ the size of ~V~.

*/
    bool Inside( const Region& r ) const;
/*
5.4.7 Operation ~intersects~ (with ~points~)

*Precondition:* ~U.IsOrdered() $\&\&$ V.IsOrdered()~

*Semantics:* $U \cap V \neq \emptyset$

*Complexity:* $O(m+n)$, where ~m~ is the size of ~U~ and ~n~ the size of ~V~.

*/
    bool Intersects( const Points& ps ) const;
/*
5.4.7 Operation ~intersects~ (with ~line~)

*Precondition:* ~U.IsOrdered() $\&\&$ V.IsOrdered()~

*Semantics:* $U \cap V \neq \emptyset$

*Complexity:* $O(m \log n)$, where ~m~ is the size of ~U~ and ~n~ the size of ~V~.

*/
    bool Intersects( const Line& l ) const;
/*
5.4.7 Operation ~intersects~ (with ~region~)

*Precondition:* ~U.IsOrdered() $\&\&$ V.IsOrdered()~

*Semantics:* $U \cap V \neq \emptyset$

*Complexity:* $O(m \log n)$, where ~m~ is the size of ~U~ and ~n~ the size of ~V~.

*/
    bool Intersects( const Region& r ) const;
/*
5.4.7 Operation ~adjacent~ (with ~region~)

*Precondition:* ~U.IsOrdered() $\&\&$ V.IsOrdered()~

*Semantics:* $\partial U \cap \partial V \neq \emptyset \land U^0 \cap V^0 = \emptyset$

*Complexity:* $O(n.m)$, where ~n~ is the size of ~U~ and m the size of ~V~.

*/
    bool Adjacent( const Region& r ) const;
/*
5.4.8 Operation ~intersection~ (with ~points~)

*Precondition:* ~U.IsOrdered() $\&\&$ V.IsOrdered()~

*Semantics:* $U \cap V$

*Complexity:* $O(m + n)$ where ~m~ is the size of ~U~ and ~n~ the size of ~V~.

*/
    void Intersection( const Points& ps, Points& result ) const;
/*
5.4.8 Operation ~intersection~ (with ~line~)

*Precondition:* ~U.IsOrdered() $\&\&$ V.IsOrdered()~

*Semantics:* $U \cap V$

*Complexity:* $O(m.n)$ where ~m~ is the size of ~U~ and ~n~ the size of ~V~.

*/
    void Intersection( const Line& l, Points& result ) const;
/*
5.4.8 Operation ~intersection~ (with ~region~)

*Precondition:* ~U.IsOrdered() $\&\&$ V.IsOrdered()~

*Semantics:* $U \cap V$

*Complexity:* $O(m.n)$ where ~m~ is the size of ~U~ and ~n~ the size of ~V~.

*/
    void Intersection( const Region& r, Points& result ) const;
/*
5.4.8 Operation ~minus~ (with ~point~)

*Precondition:* ~U.IsOrdered() $\&\&$ v.IsDefined()~

*Semantics:* $U \ \{v\}$

*Complexity:* $O(n)$ where ~n~ is the size of ~U~.

*/
    void Minus( const Point& p, Points& result ) const;
/*
5.4.8 Operation ~minus~ (with ~points~)

*Precondition:* ~U.IsOrdered() $\&\&$ V.IsOrdered()~

*Semantics:* $U \ V$

*Complexity:* $O(m + n)$ where ~m~ is the size of ~U~ and ~n~ the size of ~V~.

*/
    void Minus( const Points& ps, Points& result ) const;
/*
5.4.8 Operation ~minus~ (with ~line~)

*Precondition:* ~U.IsOrdered() $\&\&$ V.IsOrdered()~

*Semantics:* $U \ V$

*Complexity:* $O(m.n)$ where ~m~ is the size of ~U~ and ~n~ the size of ~V~.

*/
    void Minus( const Line& l, Points& result ) const;
/*
5.4.8 Operation ~minus~ (with ~region~)

*Precondition:* ~U.IsOrdered() $\&\&$ V.IsOrdered()~

*Semantics:* $U \ V$

*Complexity:* $O(m.n)$ where ~m~ is the size of ~U~ and ~n~ the size of ~V~.

*/
    void Minus( const Region& r, Points& result ) const;
/*
5.4.8 Operation ~union~ (with ~point~)

*Precondition:* ~U.IsOrdered() $\&\&$ v.IsDefined()~

*Semantics:* $U \cup \{v\}$

*Complexity:* $O(n)$ where ~n~ is the size of ~U~.

*/
    void Union( const Point& p, Points& result ) const;
/*
5.4.8 Operation ~union~ (with ~points~)

*Precondition:* ~U.IsOrdered() $\&\&$ V.IsOrdered()~

*Semantics:* $U \cup V$

*Complexity:* $O(m + n)$ where ~m~ is the size of ~U~ and ~n~ the size of ~V~.

*/
    void Union( const Points& ps, Points& result ) const;
/*
5.4.9 Operation ~distance~ (with ~point~)

*Precondition:* ~U.IsOrdered() and v.IsDefined()~

*Semantics:* $\min\{ dist(u, v) | u \in U \}$

*Complexity:* $O(n)$, where ~n~ is the size of ~U~

*/
    double Distance( const Point& p ) const;
/*
5.4.9 Operation ~distance~ (with ~points~)

*Precondition:* ~U.IsOrdered() and V.IsOrdered()~

*Semantics:* $\min\{ dist(u, v) | u \in U, v \in V \}$

*Complexity:* $O(m.n)$, where ~m~ is the size of ~U~ and ~n~ the size of ~V~

*/
    double Distance( const Points& ps ) const;
/*
5.4.9 Operation ~distance~ (with ~rect2~)

*Precondition:* ~U.IsOrdered() and V.IsOrdered()~


*/
    double Distance( const Rectangle<2>& r ) const;

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
    inline bool GetPt( const Point*& p ) const;
/*
Gets the current point from the ~points~ value according to the ~pos~ pointer.

5.6 Functions needed to import the the ~points~ data type to tuple

There are totally 10 functions which are defined as virtual functions. They need
to be defined here in order for the Point data type to be used in Tuple definition
as an attribute.

*/
    inline int NumOfFLOBs() const;
    inline FLOB *GetFLOB( const int i );
    inline size_t Sizeof() const;
    size_t HashValue() const;
    void CopyFrom( const StandardAttribute* right );
    int Compare( const Attribute *arg ) const;
    int CompareAlmost( const Attribute *arg ) const;
    bool Adjacent( const Attribute *arg ) const;
    virtual Points* Clone() const;
    ostream& Print( ostream &os ) const;


    virtual uint32_t getshpType() const{
       return 8; // Point Type
    }

    virtual bool hasBox(){
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
         const Point* p;
         for(uint32_t i=0;i<size;i++){
            points.Get(i,p);
            double x = p->GetX();
            double y = p->GetY();
            WinUnix::writeLittle64(o,x);
            WinUnix::writeLittle64(o,y);
         }
       }
    }


  static const string BasicType(){
    return symbols::POINTS;
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
    DBArray<Point> points;
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
6 Struct ~AttrType~

The following type definition indicates the structure of the ~attr~ value associated with
half segments. This attribute is utilized only when we are handling regions. In line values
this attibute is ignored.

*/
struct AttrType
{
  inline AttrType() {}
/*
The simple constructor.

*/
  inline AttrType( const AttrType& at ):
  faceno( at.faceno ),
  cycleno( at.cycleno ),
  edgeno( at.edgeno ),
  coverageno( at.coverageno ),
  insideAbove( at.insideAbove ),
  partnerno( at.partnerno )
  {}
/*
The copy constructor.

*/
  inline AttrType& operator=( const AttrType& at )
  {
    faceno = at.faceno; cycleno = at.cycleno;
    edgeno = at.edgeno; coverageno = at.coverageno;
    insideAbove = at.insideAbove; partnerno = at.partnerno;
    return *this;
  }
/*
Redefinition of the assignement operator.

6.1 Attributes

*/
  int faceno;
/*
The face identifier

*/
  int cycleno;
/*
The cycle identifier

*/
  int edgeno;
/*
The edge (segment) identifier

*/
  int coverageno;
/*
Used for fast spatial scan of the inside[_]pr algorithm

*/
  bool insideAbove;
/*
Indicates whether the region's area is above or left of its segment

*/
  int partnerno;
/*
Stores the position of the partner half segment in half segment ordered array

*/
};

/*
7 Class ~HalfSegment~

This class implements the memory representation of  ~halfsegment~. Although ~halfsegment~
is not an independent type constructor, it is the basic construction unit of the ~line~ and the ~region~
type constructors.

A ~halfsegment~ value is composed of a pair of points and a flag indicating the dominating
point. The left point is always smaller than the right one.

*/
class HalfSegment
{
  public:

/*
5.1 Constructors and Destructor

A half segment is composed by two points which are called ~left point~ (~lp~) and ~right point~ (~rp~),
$lp < rp$, and a flag ~ldp~ (~left dominating point~) which tells whether the left point is the dominating
point.

*/
    inline HalfSegment() {}
/*
This constructor should not be used.

*/
    inline HalfSegment( bool ldp, const Point& lp, const Point& rp );
/*
Creates a half segment receiving all attributes as arguments. The order between the left
and right points is not important. If ~lp~ is bigger than ~rp~ their values are changed.

*/
    inline HalfSegment( const HalfSegment& hs );
/*
Creates a half segment copying the values from ~hs~.

*/
    inline ~HalfSegment() {}
/*
The destructor.

5.2 Member Functions

*/
    inline const Point& GetLeftPoint() const;
/*
Returns the left point of the half segment.

*/
    inline const Point& GetRightPoint() const;
/*
Returns the right point of the half segment.

*/
    inline const Point& GetDomPoint() const;
/*
Returns the dominating point of the half segment.

*/
    inline const Point& GetSecPoint() const;
/*
Returns the secondary point of the half segment.

*/
    inline bool IsLeftDomPoint() const;
/*
Returns the boolean flag which indicates whether the dominating point is on the left side.

*/
    inline const Rectangle<2> BoundingBox() const;
/*
Returns the bounding box of the half segment.

*/
    inline const AttrType& GetAttr() const;
/*
Returns the "attr" value associated with a half segment. The "attr" value is useful when we
process region values.

*/
    inline double Length() const;
/*
Returns the length of the half segmtent, i.e., the distance between the left point to the
right point.

*/
    inline Point AtPosition( double pos ) const;
/*
Returns the point at relative position ~pos~.

*/
    inline double AtPoint( const Point& p ) const;
/*
Returns the relative position of the point ~p~.

*/
    inline bool SubHalfSegment( double pos1, double pos2,
                                HalfSegment& result ) const;
/*
Returns the sub half segment trimmed by ~pos1~ and ~pos2~.

*/
    void Set( bool ldp, const Point& lp, const Point& rp );
/*
Sets the value of a half segment. The parameters ~lp~ and ~rp~ can ignore the order, and the
function will compare the parameter points and put the smaller one to ~lp~ and larger one to ~rp~.

*/
    void Translate( const Coord& x, const Coord& y );

/*
Translates the half segment by adding the coordinates ~x~ and ~y~ (which can be negative) to both
~lp~ and ~rp~ points.

*/

    inline void Scale( const Coord& f )
    {
      lp.Scale( f );
      rp.Scale( f );
    }
/*
Scales the half segment given a factor ~f~.

*/
    inline void SetAttr( AttrType& attr );
/*
Sets the value of the "attr" attribute of a half segment.

*/
    inline void SetLeftDomPoint( bool ldp );
/*
Sets the value of the dominating point flag of a half segment.

*/

    inline bool IsVertical()const;


    HalfSegment& operator=( const HalfSegment& hs );
    bool operator==( const HalfSegment& hs ) const;
    bool operator!=( const HalfSegment& hs ) const;
    bool operator<(const HalfSegment& hs) const;
    bool operator<=(const HalfSegment& hs) const;
    bool operator>(const HalfSegment& hs) const;
    bool operator>=(const HalfSegment& hs) const;
/*
Operator redefinitions.

*/
    int Compare( const HalfSegment& hs ) const;
/*
This function make comparison between two halfsegments. The rule of the comparison is specified
in the ROSE Algebra paper. That is:  the half sgenments will be ordered according to the following values:
dominating points -\verb+>+  LDP flages  -\verb+>+ directions (rotations).

*/
    bool Intersects( const HalfSegment& hs ) const;
/*
Decides whether two half segments intersect with each other with any kind of intersection.

*/
    bool InnerIntersects( const HalfSegment& hs ) const;
/*
Decides whether two half segments intersect in the following manner: a point of the first segment and an
innerpoint of the second segment are the same.

*/
    bool Crosses( const HalfSegment& hs ) const;
/*
Computes whether two half segments intersect in their mid-points. Endpoints are not considered in
computing the results.

*/
    bool Intersection( const HalfSegment& hs, Point& p ) const;
/*
This function computes whether two half segments cross each other and returns the crossing point ~p~.

*/
    bool Intersection( const HalfSegment& hs1, HalfSegment& hs2 ) const;
/*
This function computes whether two half segments intersect each other and returns the resulting
halfsegment ~hs~.

*/
   void CohenSutherlandLineClipping( const Rectangle<2> &window,
                                     double &x0, double &y0,
                                     double &x1, double &y1,
                                     bool &accept) const;
/*
Implements the Cohen and Sutherland algorithm for clipping a segment to a clipping window.

*/
   void WindowClippingIn( const Rectangle<2> &window,
                          HalfSegment &hs,
                          bool &inside,
                          bool &isIntersectionPoint,
                          Point &intersectionPoint) const;
/*
Computes the part of the segment that is inside the rectangle ~window~, if
it exists. The ~inside~ parameter is set to true if there is a partion of the segment
inside the window. The ~isIntersectionPoint~ parameter is set to true if the intersection
part of the segment is a point instead of a segment, and if so, ~intersectionPoint~
receives the intersection point.

*/
    bool Inside( const HalfSegment& hs ) const ;
/*
Computes whether the half segment is inside the one in ~hs~.

*/
    bool Contains( const Point& p ) const;
/*
Computes whether the point ~p~ is contained in the half segment.
Uses the ~AlmostEqual~ function.

*/
    bool RayAbove( const Point& p, double &abovey0 ) const;
/*

Decides whether a half segment is above a point. This is useful when we want to decide
whether a point is inside a region.

*/
    bool RayDown(const Point& p, double &yIntersection) const;


    double Distance( const Point& p ) const;
/*
Computes the distance from the half segment to the point ~p~.

*/
    double Distance( const HalfSegment& hs ) const;
/*
Computes the minimum distance from two half segments.

*/

    double Distance(const Rectangle<2>& rect) const;
    double MaxDistance(const Rectangle<2>& rect) const;
    int LogicCompare( const HalfSegment& hs ) const;
/*
Compares two half segments according to their attribute values (~attr~).

*/
    bool innerInter( const HalfSegment& chs,  Point& resp,
                     HalfSegment& rchs, bool& first, bool& second ) const;
/*
Used in the Plane Sweep Algebra

*/

  private:

/*
5.13 Attributes

*/
    bool ldp;
/*
Indicates which is the dominating point: the left or the right point.

*/
    Point lp;
    Point rp;
/*
These two attributes give the left and right point of the half segment.

*/
  public:

    AttrType attr;
/*
This ~attribute~ property is useful if we process region values in the way indicated in the ROSE
paper.

*/
};
/*
5.14 Overloaded output operator

*/
ostream& operator<<( ostream& o, const HalfSegment& hs );

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
    inline Line( const int n );
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
    inline double Length() const;

    inline double SpatialSize() const{
      return Length();
    }
/*
Returns the length of the line, i.e. the sum of the lengths of all segments.

*/
//    inline void SetLength( double length );
/*
Sets the length of the line.

*/
    inline const Rectangle<2> BoundingBox() const;
/*
Returns the bounding box of the line.

*/
  //  inline void SetBoundingBox( const Rectangle<2>& bbox );
/*
Sets the bounding box of the line.

*/
    inline bool IsEmpty() const;
/*
Returns whether the line value is empty.

*/
    inline int Size() const;
/*
Returns the number of half segments in the line value.

*/
    bool Contains( const Point& p ) const;
/*
Checks whether the point ~p~ is contained in the line

*/
    inline void Get( const int i, const HalfSegment*& hs ) const;
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
    bool Intersects( const Line& l ) const;
/*
6.4.4 Operation ~intersects~ (with ~region~)

*Precondition:* ~U.IsOrdered() and V.IsOrdered()~

*Semantics:* $U \cap V \neq \emptyset$

*Complexity:* $O(m(n + \log n))$, where ~m~ is the size of ~U~ and ~n~ the size of ~V~.

*/
    bool Intersects( const Region& r ) const;
/*
6.4.4 Operation ~inside~ (with ~line~)

*Precondition:* ~U.IsOrdered() and V.IsOrdered()~

*Semantics:* $U \in V$

*Complexity:* $O(m.n)$, where ~m~ is the size of ~U~ and ~n~ the size of ~V~.

*/
    bool Inside( const Line& l ) const;
/*
6.4.4 Operation ~inside~ (with ~region~)

*Precondition:* ~U.IsOrdered() and V.IsOrdered()~

*Semantics:* $U \in V$

*Complexity:* $O(m.log(n))$, where ~m~ is the size of ~U~ and ~n~ the size of ~V~.

*/
    bool Inside( const Region& r ) const;
/*
6.4.4 Operation ~adjacent~ (with ~region~)

*Precondition:* ~U.IsOrdered() $\&\&$ V.IsOrdered()~

*Semantics:* $\partial U \cap \partial V \neq \emptyset \land U^0 \cap V^0 = \emptyset$

*Complexity:* $O(n.m)$, where ~n~ is the size of ~U~ and m the size of ~V~.

*/
    bool Adjacent( const Region& r ) const;
/*
6.4.4 Operation ~intersection~ (with ~line~)

*Precondition:* ~U.IsOrdered() and V.IsOrdered()~

*Semantics:* $U \cap V$

*Complexity:* $O(m.n)$, where ~m~ is the size of ~U~ and ~n~ the size of ~V~.

*/
    void Intersection( const Line& l, Line& result ) const;
/*
6.4.4 Operation ~minus~ (with ~line~)

*Precondition:* ~U.IsOrdered() and V.IsOrdered()~

*Semantics:* $U \backslash V$

*Complexity:* $O(m.n)$, where ~m~ is the size of ~U~ and ~n~ the size of ~V~.

*/
    void Minus( const Line& l, Line& result ) const;
/*
6.4.5 Operation ~crossings~

*Precondition:* ~U.IsOrdered() and V.IsOrdered()~

*Semantics:* $\{ p \in U \cap V | p \textrm{ is isolated in } U \cap V \}$

*Complexity:* $O(m.n + r\log r)$, where ~m~ is the size of ~U~, ~n~ the size of ~V~, and ~r~
is the ~points~ result size.

*/
    void Crossings( const Line& l, Points& result ) const;
/*
6.4.5 Operation ~distance~ (with ~point~)

*Precondition:* ~!U.IsEmpty() and v.IsDefined()~

*Semantics:* $\min \{ dist(u, v) | u \in U \}$

*Complexity:* $O(n)$, where ~n~ is the size of ~U~.

*/
    double Distance( const Point& p ) const;
    double MaxDistance(const Point& p) const;
/*
6.4.5 Operation ~distance~ (with ~points~)

*Precondition:* ~!U.IsEmpty() and !V.IsEmpty()~

*Semantics:* $\min \{ dist(u, v) | u \in U, v \in V \}$

*Complexity:* $O(m.n)$, where ~m~ is the size of ~U~ and ~n~ is the size of ~V~.

*/
    double Distance( const Points& ps ) const;
/*
6.4.5 Operation ~distance~ (with ~line~)

*Precondition:* ~!U.IsEmpty() and !V.IsEmpty()~

*Semantics:* $\min \{ dist(u, v) | u \in U, v \in V \}$

*Complexity:* $O(m.n)$, where ~m~ is the size of ~U~ and ~n~ is the size of ~V~.

*/
    double Distance( const Line& l ) const;

/*
6.4.5 Operation ~distance~ (with ~rect2~)

*Precondition:* ~!U.IsEmpty() and !V.IsEmpty()~

*/
    double Distance( const Rectangle<2>& r ) const;


/*
5.4.9 Operation ~distance~ (with ~points~)

*Precondition:* ~U.IsOrdered() and V.IsOrdered()~

*Semantics:* $\min\{ dist(u, v) | u \in U, v \in V \}$

*Complexity:* $O(m.n)$, where ~m~ is the size of ~U~ and ~n~ the size of ~V~

*/
  double MaxDistance(const Rectangle<2>& r) const;
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
                  const Points& importantPoint = Points(0)) const;



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
    inline bool GetHs( const HalfSegment*& hs ) const;
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

    inline FLOB *GetFLOB( const int i )
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
    void CopyFrom( const StandardAttribute* right );
    int Compare( const Attribute *arg ) const;
//     int CompareAlmost( const Attribute *arg ) const;
    virtual Line *Clone() const;
    ostream& Print( ostream &os ) const;
    void Clear();



    virtual uint32_t getshpType() const{
       return 3; // PolyLine Type
    }

    virtual bool hasBox(){
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
         const HalfSegment* hs;
         for(int i=0;i<line.Size();i++){
           line.Get(i,hs);
           if(hs->IsLeftDomPoint()){
              Point p = hs->GetLeftPoint();
              WinUnix::writeLittle64(o,p.GetX());
              WinUnix::writeLittle64(o,p.GetY());
              p = hs->GetRightPoint();
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
      return symbols::LINE;
   }

 protected:
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
                    int& posnexths, const HalfSegment*& nexths );
    bool
    GetNextSegments( const int poshs,
                     const HalfSegment& hs,
                     vector<bool>& visited,
                     int& posnexths,
                     const HalfSegment*& nexths,
                     stack< pair<int, const HalfSegment*> >& nexthss );
    void computeComponents();

    void collectFace(int faceno, int startPos, DBArray<bool>& used);
    int getUnusedExtension(int startPos, const DBArray<bool>& used) const;

/*
Calculates and sets the number of components for the line. For every half segment, the following
information is stored: faceno contains the number of the component, cycleno contains the
ramification that the segment belongs, and edgeno contains the segment number.

The method ~VisitHalfSegments~ is a recursive function that does the job for
~SetNoComponents~.

6.11 Attributes

*/
    DBArray<HalfSegment> line;
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
  SimpleLine(int size):
            segments(size),lrsArray(size/2),
            startSmaller(true),
            isCycle(false),isOrdered(true),length(0.0),
            bbox(false),currentHS(-1){
     del.refs=1;
     del.isDelete=true;
     SetDefined(false);
    }

/*
~Constructor~


Constructs a ~SimpleLine~ from a complex one.

*/
  SimpleLine(const Line& src);

/*
~CopyConstructor~

*/
  SimpleLine(const SimpleLine& src):
    segments(src.Size()),lrsArray(src.Size()){
    Equalize(src);
    del.refs=1;
    del.isDelete=true;
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

Returns the length of this SimpleLine;

*/
  inline double Length() const{
    return length;
  }

  inline double SpatialSize() const{
     return Length();
  }


/*
~BoundingBox~

Returns the MBR of this SimpleLine object.

*/
  inline const Rectangle<2> BoundingBox() const{
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

/*
~EndPoint~

Returns the end point of this simple line.

*/
  Point EndPoint(const bool startSmaller) const;

/*
~Contains~

Checks whether ~p~ is located on this line.

*/
  bool Contains(const Point& p) const;

/*
~TrimToSize~

Changes the capacities of the contained arrays to the required size.

*/
  inline void TrimToSize(){
     segments.TrimToSize();
     lrsArray.TrimToSize();
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
     const HalfSegment* hs1;
     const HalfSegment* hs2;
     for(int i=0;i<segments.Size();i++){
       segments.Get(i,hs1);
       sl.segments.Get(i,hs2);
       if(!AlmostEqual(*hs1,*hs2)){
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
  double Distance(const Point& p )const;

  double Distance(const Points& ps ) const;

  double Distance(const SimpleLine& sl) const;

  double Distance(const Rectangle<2>& r) const;




/*
~AtPosition~


*/
  bool AtPosition(double pos, const bool startsSmaller,
                  Point& p) const;

/*
~AtPoint~

*/

  bool AtPoint(const Point& p, const bool startsSmaller,
               double& result) const;


/*
~SubLine~

*/
  void SubLine(const double pos1, const double pos2,
               bool startsSmaller, SimpleLine& l) const;

/*
~Crossings~

*/
 void Crossings(const SimpleLine& l, Points& result) const;

/*
~Union~

*/
bool Intersects(const SimpleLine& l) const;


/*
~Attribute Functions~

The following functions are needed to act as an attribute type.

*/
  inline int NumOfFLOBs() const{
    return 2;
  }

  inline FLOB* GetFLOB(const int i){
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

  void CopyFrom(const StandardAttribute* right){
     Equalize(*(static_cast<const SimpleLine*>(right)));
  }

  int Compare(const Attribute* arg) const;

  virtual SimpleLine* Clone() const{
     SimpleLine* res =  new SimpleLine(*this);
     return  res;
  }

  ostream& Print(ostream& os) const;

  void Clear(){
     segments.Clear();
     lrsArray.Clear();
     SetDefined( true );
     bbox.SetDefined(false);
     length=0.0;
     isOrdered=true;
     currentHS = -1;
  }

  bool SelectInitialSegment( const Point& startPoint,
                             const double tolerance = 1.0);

  bool SelectSubsequentSegment();

  bool getWaypoint(Point& destination) const;

  inline bool GetStartSmaller() const{
    return startSmaller;
  }

  bool IsCycle()const {
    return isCycle;
  }

  void toLine(Line& result) const;

  void fromLine(const Line& src);

  static void* Cast(void* addr){
    return new (addr) SimpleLine();
  }

  inline void Get( const int i, const HalfSegment*& hs ) const{
    segments.Get(i,hs);
  }

  bool Get(LRS &lrs, int &i){
    return Find(lrs, i);
  }

  inline void Put(const int i, const HalfSegment& hs){
    segments.Put(i,hs);
  }


  inline void Get( const int i, const LRS*& lrs ) const{
    lrsArray.Get(i,lrs);
  }

  inline void Put(const int i, const LRS& lrs){
    lrsArray.Put(i,lrs);
  }

  static const string BasicType(){
    return "sline";
  }

  private:
    DBArray<HalfSegment> segments;
    DBArray<LRS> lrsArray;
    bool startSmaller;
    bool isCycle;
    bool isOrdered;
    double length;
    Rectangle<2> bbox;
    int currentHS;

    void Equalize(const SimpleLine& src){
        const HalfSegment* seg;
        segments.Clear();
        lrsArray.Clear();

        int size = src.segments.Size();
        if(size>0){
          segments.Resize(size);
        }
        for(int i=0;i<size;i++){
           src.segments.Get(i,seg);
           segments.Append(*seg);
        }

        size = src.lrsArray.Size();
        if(size>0){
          lrsArray.Resize(size);
        }
        const LRS* lrs;
        for(int i=0;i<size;i++){
           src.lrsArray.Get(i,lrs);
           lrsArray.Append(*lrs);
        }
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

   if( IsEmpty()  || ! IsDefined()){
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
    inline Region( const int n );
/*
Constructs an empty region allocating space for ~n~ half segments.

*/
    Region( const Region& cr, bool onlyLeft = false );
/*
The copy constructor. If the flag ~onlyLeft~ is set, then only the half segments with left
dominating point are copied.

*/

    Region( const Rectangle<2>& r );
/*
Creates a rectangular region from a rect2 objects.

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
    const Rectangle<2> BoundingBox() const;
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
    inline void Get( const int i, const HalfSegment*& chs ) const
    {
      return region.Get( i, chs );
    }
/*
Reads the ith half segment from the ~region~ value.

*/
    inline void Put( const int i, const HalfSegment& hs )
    {
      region.Put( i, hs );
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
    bool Intersects( const Region& r ) const;
/*
6.4.4 Operation ~inside~

*Precondition:* ~U.IsOrdered() and V.IsOrdered()~

*Semantics:* $U \subseteq V$

*Complexity:* $O(m.n)$, where ~m~ is the size of ~U~ and ~n~ the size of ~V~.

*/
    bool Inside( const Region& r ) const;
/*
6.4.4 Operation ~adjacent~

*Precondition:* ~U.IsOrdered() $\&\&$ V.IsOrdered()~

*Semantics:* $\partial U \cap \partial V \neq \emptyset \land U^0 \cap V^0 = \emptyset$

*Complexity:* $O(m.n)$, where ~m~ is the size of ~U~ and ~n~ the size of ~V~.

*/
    bool Adjacent( const Region& r ) const;
/*
6.4.4 Operation ~overlaps~

*Precondition:* ~U.IsOrdered() $\&\&$ V.IsOrdered()~

*Semantics:* $U^0 \cap V^0 \neq \emptyset$

*Complexity:* $O(m.n)$, where ~m~ is the size of ~U~ and ~n~ the size of ~V~.

*/
    bool Overlaps( const Region& r ) const;
/*
6.4.4 Operation ~onborder~

*Precondition:* ~U.IsOrdered() $\&\&$ v.IsDefined()~

*Semantics:* $v \in \partial U$

*Complexity:* $O(m)$, where ~m~ is the size of ~U~.

*/
    bool OnBorder( const Point& p ) const;
/*
6.4.4 Operation ~ininterior~

*Precondition:* ~U.IsOrdered() $\&\&$ v.IsDefined()~

*Semantics:* $v \in U^0$

*Complexity:* $O(m)$, where ~m~ is the size of ~U~.

*/
    bool InInterior( const Point& p ) const;
/*
6.4.4 Operation ~distance~ (with ~point~)

*Precondition:* ~U.IsOrdered() $\&\&$ v.IsDefined()~

*Semantics:* $\min\{ dist(u, v) | u \in U \}$

*Complexity:* $O(m)$, where ~m~ is the size of ~U~.

*/
    double Distance( const Point& p ) const;
/*
6.4.4 Operation ~distance~ (with ~points~)

*Precondition:* ~U.IsOrdered() $\&\&$ V.IsOrdered()~

*Semantics:* $\min\{ dist(u, v) | u \in U, v \in V \}$

*Complexity:* $O(m.n)$, where ~m~ is the size of ~U~ and ~n~ the size of ~V~.

*/
    double Distance( const Points& p ) const;
/*
6.4.4 Operation ~distance~ (with ~points~)

*Precondition:* ~U.IsOrdered() $\&\&$ V.IsOrdered()~

*Semantics:* $\min\{ dist(u, v) | u \in U, v \in V \}$

*Complexity:* $O(m.n)$, where ~m~ is the size of ~U~ and ~n~ the size of ~V~.

*/
  double Distance( const Region& r ) const;
/*
  6.4.4 Operation ~distance~ (with ~region~)

  *Precondition:* ~U.IsOrdered() $\&\&$ V.IsOrdered()~

  *Semantics:* $\min\{ dist(u, v) | u \in U, v \in V \}$

  *Complexity:* $O(m.n)$, where ~m~ is the size of ~U~ and ~n~ the size of ~V~.

*/

  double Distance(const Line& l) const;

  double Distance( const Rectangle<2>& r ) const;
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
    void TouchPoints( const Line& l, Points& result ) const;
/*
6.4.5 Operation ~touch[_]points~ (with ~region~)

*Precondition:* ~U.IsOrdered() and V.IsOrdered()~

*Semantics:* $\{ p \in \partial U \cap \partial V | p \textrm{ is isolated in } \partial U \cap \partial V \}$

*Complexity:* $O(m.n + r\log r)$, where ~m~ is the size of ~U~, ~n~ the size of ~V~, and ~r~
is the ~points~ result size.

*/
    void TouchPoints( const Region& r, Points& result ) const;
/*
6.4.5 Operation ~common[_]border~

*Precondition:* ~U.IsOrdered() and V.IsOrdered()~

*Semantics:* $\partial U \cap \partial V $

*Complexity:* $O(m.n + r\log r)$, where ~m~ is the size of ~U~, ~n~ the size of ~V~, and ~r~
is the ~line~ result size.

*/
    void CommonBorder( const Region& r, Line& result ) const;
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
    void Vertices( Points* result ) const;


/*
6.4.5 Operation ~boundary~

*Precondition:* ~U.IsOrdered()~

*Semantics:* The boundary of ~U~

*Complexity:* $O(m)$, where ~m~ is the size of ~U~.

*/
    void Boundary(Line* result) const;

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
    inline bool GetHs( const HalfSegment*& hs ) const;
/*
Gets the current half segment from the ~region~ value according to the ~pos~ pointer.

7.8 contain function (point)

*Semantics:* This function decides whether a point is inside the region.

*Complexity:* $O( n )$  where ~n~ is the number of segments of the region.

*/
    bool Contains( const Point& p ) const;
/*
7.9 innercontain function

*Semantics:* This function decides whether a point is inside the inner part of the region.

*Complexity:* $O( n )$  where ~n~ is the number of segments of the region.

*/
    bool InnerContains( const Point& p ) const;
/*
7.10 contain function (segment)

*Semantics:* This function decides whether a half segment is inside the region.

*Complexity:* $O( n )$  where ~n~ is the number of segments of the region.

*/
    bool Contains( const HalfSegment& hs ) const;
/*
7.10 inner contain function (segment)

*Semantics:* This function decides whether a half segment is completely inside the region.

*Complexity:* $O( n )$  where ~n~ is the number of segments of the region.

*/
    bool InnerContains( const HalfSegment& hs ) const;
/*
7.10 intersects function (segment)

*Semantics:* This function decides whether a half segment intersects the region.

*Complexity:* $O( n )$  where ~n~ is the number of segments of the region.

*/
    bool Intersects( const HalfSegment& hs ) const;
/*
7.11 holeedge-contain function

*Semantics:* This function decides whether a half segment is inside a hole edge of the region.

*Complexity:* $O( n )$  where ~n~ is the number of segments of the region.

*/
    bool HoleEdgeContain( const HalfSegment& hs ) const;
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

    inline FLOB *GetFLOB( const int i )
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
    void CopyFrom( const StandardAttribute* right );
    int Compare( const Attribute *arg ) const;
//     int CompareAlmost( const Attribute *arg ) const;
    ostream& Print( ostream &os ) const;
    virtual Region *Clone() const;
    void Clear();
    void SetEmpty();

/*

VTA - Move this to the operators section

~Translate~

Moves the region according x and y and stores the result in result.


*/

    void Translate(const Coord& x, const Coord& y, Region& result) const;

    void Rotate(const Coord& x, const Coord& y, const double alpha,
                Region& result) const;

/*

VTA - Move this to the operators section

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
       const HalfSegment* hs;
       for(int i=0;i<size;i++)
       {
           Get(i,hs);
           HalfSegment ths(*hs);
           ths.Translate(x,y);
           region.Put(i,ths);
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
                        Region &clippedRegion) const;

/*
7.15.2 Window clipping OUT function

This function returns the clipped half segments that are outside a window, which
result from the clipping of a region to a clip window.

*/

  void WindowClippingOut(const Rectangle<2> &window,
                         Region &clippedRegion) const;
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
                    Region &clippedRegion,bool inside) const;
/*
7.15.4 Get clipped half segment IN function

This function returns the clipped half segments (that are within the window)
resulting from the clipping of a region to a clip window.

*/

  void GetClippedHSIn(const Rectangle<2> &window,
                      Region &clippedRegion,
                      vector<EdgePoint> pointsOnEdge[4],
                      int &partnerno) const;
/*
7.15.5 Get clipped half segment OUT function

This function returns the clipped half segments (that are outside the window)
resulting from the clipping of a region to a clip window.

*/
 void GetClippedHSOut(const Rectangle<2> &window,
                      Region &clippedRegion,
                      vector<EdgePoint> pointsOnEdge[4],
                      int &partnerno) const;
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
                               vector<EdgePoint> pointsOnEdge[4]);
/*
7.15.8 Create new segments function

This function creates the half segments resulting from the connection
of the points that lies on the window's edges.

*/
   static void CreateNewSegments(vector <EdgePoint>pointsOnEdge, Region &cr,
                                 const Point &bPoint,const Point &ePoint,
                                 WindowEdge edge,int &partnerno,bool inside);
/*
7.15.9 Create new segments function

This function creates new half segments corresponding to the edges of the window.
These half segments are only created if the window's edge is completly inside
the window.

*/
   void CreateNewSegmentsWindowVertices(const Rectangle<2> &window,
                                vector<EdgePoint> pointsOnEdge[4],Region &cr,
                                int &partnerno,bool inside) const;
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
                      int partnernoP, HalfSegment const*& adjacentCHS,
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

   double Area() const;

   double SpatialSize() const{
     return Area();
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

  const DBArray<HalfSegment>& GetHalfSegments(){
      return region;
  }

  static const string BasicType(){
    return symbols::REGION;
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
    DBArray<HalfSegment> region;
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
ListExpr
OutPoint( ListExpr typeInfo, Word value );

Word
InLine( const ListExpr typeInfo, const ListExpr instance,
        const int errorPos, ListExpr& errorInfo, bool& correct ) ;
ListExpr
OutLine( ListExpr typeInfo, Word value );

Word
InRegion( const ListExpr typeInfo, const ListExpr instance,
          const int errorPos, ListExpr& errorInfo, bool& correct );
ListExpr
OutRegion( ListExpr typeInfo, Word value );

Word
InSimpleLine( const ListExpr typeInfo, const ListExpr instance,
        const int errorPos, ListExpr& errorInfo, bool& correct ) ;
ListExpr
OutSimpleLine( ListExpr typeInfo, Word value );

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
      if( this->x < p.x )
        return 1;
      else
        if( this->x == p.x && this->y < p.y )
          return 1;
        else
          if( this->x == p.x && this->y == p.y )
          { //If both points has the same coordinates, if they have diferent
            // directions
            // the less one will be that has the attribute direction true i
            // (left and down),
            // otherwise, both have direction true, and the less one will
            // be that was rejected.
            //The rejected points come before accepted points
            if (this->direction == p.direction)
            {
              if (this->rejected)
                return 1;
            }
            else if (this->direction)
              return 1;
          }
      return 0;
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
      if (criticalPoint==NULL)
      {
        delete criticalPoint;
        criticalPoint=NULL;
      }
    }
};

double Angle(const Point &v, const Point &p1, const Point &p2);
double VectorSize(const Point &p1, const Point &p2);

/*
11 The inline functions

11.1 Class ~Point~

*/
inline Point::Point( const bool d, const Coord& x, const Coord& y ) :
  x( x ),
  y( y )
{
  SetDefined(d);
  del.refs=1;
  del.isDelete=true;
}

inline Point::Point( const Point& p )
{
  SetDefined( p.IsDefined() );
  if( IsDefined() )
  {
    x = p.x;
    y = p.y;
  }
  del.refs=1;
  del.isDelete=true;
}

inline const Rectangle<2> Point::BoundingBox() const
{
  assert( IsDefined() );
  return Rectangle<2>( true,
                       x - ApplyFactor(x),
                       x + ApplyFactor(x),
                       y - ApplyFactor(y),
                       y + ApplyFactor(y) );
}

inline void Point::Set( const Coord& x, const Coord& y )
{
  SetDefined( true );
  this->x = x;
  this->y = y;
}

inline Point Point::Translate( const Coord& x, const Coord& y ) const
{
  assert( IsDefined() );
  return Point( true, this->x + x, this->y + y );
}


inline void Point::Translate( const Coord& x, const Coord& y )
{
  if( IsDefined() )
  {
    this->x += x;
    this->y += y;
  }
}

inline Point Point::Add( const Point& p ) const
{
  assert( IsDefined() && p.IsDefined() );
  return Point( true, this->x + p.x, this->y + p.y );
}

inline Point& Point::operator=( const Point& p )
{
  SetDefined( p.IsDefined() );
  if( IsDefined() )
  {
    x = p.x;
    y = p.y;
  }
  del.refs=1;
  del.isDelete=true;
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
  //  return x == p.x && y == p.y;
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
  bool eqx = AlmostEqual(x,p.x);
  return (!eqx && (x > p.x)) ||
         (eqx && !AlmostEqual(y,p.y) && (y>p.y));
  //  return x > p.x || ( x == p.x && y > p.y ); // changed by TB
}




inline Point Point::operator+( const Point& p ) const
{
  assert( IsDefined() && p.IsDefined() );
  return Point( true, x + p.x, y + p.y );
}

inline Point Point::operator-( const Point& p ) const
{
  assert( IsDefined() && p.IsDefined() );
  return Point( true, x - p.x, y - p.y );
}

inline Point Point::operator*( const double d ) const
{
  assert( IsDefined() );
  return Point( true, x * d, y * d );
}

inline size_t Point::Sizeof() const
{
  return sizeof( *this );
}

/*
11.2 Class ~Points~

*/
inline Points::Points( const int initsize ) :
points( initsize ),
bbox( false ),
ordered( true )
{ del.refs=1;
  del.isDelete=true;
  del.isDefined=true;
}

inline Points::Points( const Points& ps ) :
points( ps.Size() ),
bbox( ps.BoundingBox() ),
ordered( true )
{
  assert( ps.IsOrdered() );

  for( int i = 0; i < ps.Size(); i++ )
  {
    const Point *p;
    ps.Get( i, p );
    points.Put( i, *p );
  }
  del.refs=1;
  del.isDelete=true;
  del.isDefined = ps.del.isDefined;
}

inline const Rectangle<2> Points::BoundingBox() const
{
  return bbox;
}

inline void Points::Get( const int i, const Point*& p ) const
{
  points.Get( i, p );
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

inline FLOB *Points::GetFLOB(const int i)
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
  if( IsEmpty() )
    pos = -1;
  else pos = 0;
}

inline void Points::SelectNext() const
{
  if( pos >= 0 && pos < Size() - 1 )
    pos++;
  else pos = -1;
}

inline bool Points::EndOfPt() const
{
  return pos == -1;
}

inline bool Points::GetPt( const Point*& p ) const
{
  if( pos >= 0 && pos <= Size()-1 )
  {
    points.Get( pos, p );
    return true;
  }
  return false;
}

inline void Points::Resize(const int newSize){
  if(newSize>Size()){
    points.Resize(newSize);
  }
}

inline void Points::TrimToSize(){
  points.TrimToSize();
}

/*
11.3 Class ~HalfSegment~

*/
inline
HalfSegment::HalfSegment( bool ldp,
                          const Point& lp,
                          const Point& rp ):
ldp( ldp ),
lp( lp ),
rp( rp )
{
  assert(lp.IsDefined());
  assert(rp.IsDefined());
  assert( !AlmostEqual( lp, rp ) );

  if( lp > rp )
  {
    this->lp = rp;
    this->rp = lp;
  }
}

inline
HalfSegment::HalfSegment( const HalfSegment& hs ):
ldp( hs.ldp ),
lp( hs.lp ),
rp( hs.rp ),
attr( hs.attr )
{
  assert(lp.IsDefined());
  assert(rp.IsDefined());
}

inline const Point&
HalfSegment::GetLeftPoint() const
{
  return lp;
}

inline const Point&
HalfSegment::GetRightPoint() const
{
  return rp;
}

inline const Point&
HalfSegment::GetDomPoint() const
{
  if( ldp )
    return lp;
  return rp;
}

inline const Point&
HalfSegment::GetSecPoint() const
{
  if( ldp )
    return rp;
  return lp;
}

inline bool
HalfSegment::IsLeftDomPoint() const
{
  return ldp;
}

inline void
HalfSegment::SetLeftDomPoint( bool ldp )
{
  this->ldp = ldp;
}

inline bool HalfSegment::IsVertical()const{
  return AlmostEqual(lp.GetX(),rp.GetX());
}


inline const Rectangle<2>
HalfSegment::BoundingBox() const
{
  double minx = MIN( GetLeftPoint().GetX(), GetRightPoint().GetX() ),
         maxx = MAX( GetLeftPoint().GetX(), GetRightPoint().GetX() ),
         miny = MIN( GetLeftPoint().GetY(), GetRightPoint().GetY() ),
         maxy = MAX( GetLeftPoint().GetY(), GetRightPoint().GetY() );

  return Rectangle<2>( true,
                       minx - ApplyFactor(minx),
                       maxx + ApplyFactor(maxx),
                       miny - ApplyFactor(miny),
                       maxy + ApplyFactor(maxy) );
}

inline const AttrType&
HalfSegment::GetAttr() const
{
  return attr;
}

inline void
HalfSegment::SetAttr( AttrType& attr )
{
  this->attr = attr;
}

inline double
HalfSegment::Length() const
{
  return rp.Distance( lp );
}

inline Point
HalfSegment::AtPosition( double pos ) const
{
  if( pos < 0 ||
      AlmostEqual( pos, 0 ) )
    return GetDomPoint();

  if( pos > Length() ||
      AlmostEqual( pos, Length() ) )
    return GetSecPoint();

  return Point( true,
                GetDomPoint().GetX() + pos *
                  (GetSecPoint().GetX() - GetDomPoint().GetX()) / Length(),
                GetDomPoint().GetY() + pos *
                  (GetSecPoint().GetY() - GetDomPoint().GetY()) / Length() );
}

inline double
HalfSegment::AtPoint( const Point& p ) const
{
  assert( Contains( p ) );
  if( AlmostEqual( rp.GetX(), lp.GetX() ) &&
      AlmostEqual( p.GetX(), rp.GetX() ) )
    // the segment is vertical
    return Length() * (p.GetY() - GetDomPoint().GetY()) /
                      (GetSecPoint().GetY() - GetDomPoint().GetY());
  return Length() * (p.GetX() - GetDomPoint().GetX()) /
                    (GetSecPoint().GetX() - GetDomPoint().GetX());
}

inline bool
HalfSegment::SubHalfSegment( double pos1, double pos2,
                                         HalfSegment& result ) const
{
  if( AlmostEqual( AtPosition( pos1 ), AtPosition( pos2 ) ) )
    return false;

  result.Set( true, AtPosition( pos1 ), AtPosition( pos2 ) );
  return true;
}

/*
11.4 Class ~Line~

*/
inline Line::Line( const int n ) :
line( n ),
bbox( false ),
ordered( true ),
noComponents( 0 ),
length( 0.0 ),
currentHS( -1 )
{ del.refs=1;
  del.isDelete=true;
del.isDefined=true;
}

inline Line::Line( const Line& cl ) :
line( cl.Size() ),
bbox( cl.bbox ),
ordered( true ),
noComponents( cl.noComponents ),
length( cl.length ),
currentHS ( cl.currentHS)
{
  assert( cl.IsOrdered());

  const HalfSegment *hs;
  for( int i = 0; i < cl.Size(); i++ )
  {
    cl.Get( i, hs );
    line.Put( i, *hs );
  }
  del.refs=1;
  del.isDelete=true;
  del.isDefined = cl.IsDefined();
}

inline void Line::Destroy()
{
  line.Destroy();
}

  /*
   // unsave code

inline void Line::SetNoComponents( int noComponents )
{
  this->noComponents = noComponents;
}

inline void Line::SetLength( double length )
{
  this->length = length;
}


 */


inline double Line::Length() const
{
  return length;
}


inline const Rectangle<2> Line::BoundingBox() const
{
  return bbox;
}

  /*
  // unsave code

  inline void Line::SetBoundingBox( const Rectangle<2>& bbox )
  {
    this->bbox = bbox;
  }
  */

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

inline void Line::Get( const int i, const HalfSegment*& hs ) const
{
  assert(i>=0);
  assert(i<line.Size());
  line.Get( i, hs );
}

inline void Line::Resize(const int newSize){
   if(newSize>Size()){
      line.Resize(newSize);
   }
}

inline void Line::TrimToSize(){
  line.TrimToSize();
}

inline void Line::Put( const int i, const HalfSegment& hs )
{
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

inline bool Line::GetHs( const HalfSegment*& hs ) const
{
  if( pos >= 0 && pos <= Size()-1 )
  {
    line.Get( pos, hs );
    return true;
  }
  return false;
}


/*
11.4 Class ~Region~

*/
inline Region::Region( const int initsize ) :
region( initsize ),
bbox( false ),
noComponents( 0 ),
ordered( true )
{ del.refs=1;
  del.isDelete=true;
  del.isDefined = true;
}

inline void Region::Destroy()
{
  region.Destroy();
}

inline const Rectangle<2> Region::BoundingBox() const
{
  return bbox;
}

inline bool Region::IsOrdered() const
{
  return ordered;
}

inline const AttrType& Region::GetAttr( int position ) const
{
  assert(( position>=0) && (position<=Size()-1));
  const HalfSegment *hs;
  region.Get( position, hs);
  return hs->GetAttr();
}

inline void Region::UpdateAttr( AttrType& attr )
{
  if (( pos>=0) && (pos<=Size()-1))
  {
    const HalfSegment *hs;
    region.Get( pos, hs);
    HalfSegment auxhs( *hs );
    auxhs.SetAttr( attr );
    region.Put( pos, auxhs );
  }
}

inline void Region::UpdateAttr( int position, AttrType& attr )
{
  if (( position>=0) && (position<=Size()-1))
  {
    const HalfSegment *hs;
    region.Get( position, hs );
    HalfSegment auxhs( *hs );
    auxhs.SetAttr( attr );
    region.Put( position, auxhs );
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

inline bool Region::GetHs( const HalfSegment*& hs ) const
{
  if( pos >= 0 && pos <= Size()-1 )
  {
    region.Get( pos, hs );
    return true;
  }
  return false;
}

inline void Region::Resize(const int newSize){
  if(newSize>Size()){
    region.Resize(newSize);
  }
}

inline void Region::TrimToSize(){
  region.TrimToSize();
}

/*
11.4 Auxiliary functions

*/
inline bool AlmostEqual( const Point& p1, const Point& p2 )
{
  return AlmostEqual( p1.GetX(), p2.GetX() ) &&
         AlmostEqual( p1.GetY(), p2.GetY() );
}

inline bool AlmostEqual( const HalfSegment& hs1, const HalfSegment& hs2 )
{
  return AlmostEqual( hs1.GetDomPoint(), hs2.GetDomPoint() ) &&
         AlmostEqual( hs1.GetSecPoint(), hs2.GetSecPoint() );
}

inline double ApplyFactor( const double d )
{
  if( fabs(d) <= 10.0 )
    return FACTOR * fabs(d);
  return FACTOR;
}

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
     double br = bw*PI/180;
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
     double dl = (lw - lh)*PI/180;
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
    e0 = c*(PI/180)*(1 - 3*ex2/4 +
               45*ex4/64  - 175*ex6/256  + 11025*ex8/16384);
    e2 = c*(  - 3*ex2/8 + 15*ex4/32  - 525*ex6/1024 +  2205*ex8/4096);
    e4 = c*(15*ex4/256 - 105*ex6/1024 +  2205*ex8/16384);
    e6 = c*( -  35*ex6/3072 +   315*ex8/12288);
  }

};







/*
5 Auxiliary structures for plane sweep algorithms

5.1 Definition of ~ownertype~

This enumeration is used to indicate the source of an ~AVLSegment~.

*/
namespace avlseg{

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

class AVLSegment{

public:

/*
3.1.1 Constructors

~Standard Constructor~

*/
  AVLSegment();


/*
~Constructor~

This constructor creates a new segment from the given HalfSegment.
As owner only __first__ and __second__ are the allowed values.

*/

  AVLSegment(const HalfSegment* hs, ownertype owner);


/*
~Constructor~

Create a Segment only consisting of a single point.

*/

  AVLSegment(const Point* p, ownertype owner);


/*
~Copy Constructor~

*/
   AVLSegment(const AVLSegment& src);


/*
3.2.1 Destructor

*/
   ~AVLSegment() {}


/*
3.3.1 Operators

*/

  AVLSegment& operator=(const AVLSegment& src);

  bool operator==(const AVLSegment& s) const;

  bool operator<(const AVLSegment& s) const;

  bool operator>(const AVLSegment& s) const;

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

  void Equalize( const AVLSegment& src);


/*
3.5.1 Geometric Functions

~crosses~

Checks whether this segment and __s__ have an intersection point of their
interiors.

*/
 bool crosses(const AVLSegment& s) const;

/*
~crosses~

This function checks whether the interiors of the related
segments are crossing. If this function returns true,
the parameters ~x~ and ~y~ are set to the intersection point.

*/
 bool crosses(const AVLSegment& s,double& x, double& y) const;

/*
~extends~

This function returns true, iff this segment is an extension of
the argument, i.e. if the right point of ~s~ is the left point of ~this~
and the slopes are equal.

*/
  bool extends(const AVLSegment& s) const;


/*
~exactEqualsTo~

This function checks if s has the same geometry like this segment, i.e.
if both endpoints are equal.

*/
bool exactEqualsTo(const AVLSegment& s)const;

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

  bool innerDisjoint(const AVLSegment& s)const;

/*
~Intersects~

This function checks whether this segment and ~s~ have at least a
common point.

*/

  bool intersects(const AVLSegment& s)const;


/*
~overlaps~

Checks whether this segment and ~s~ have a common segment.

*/
   bool overlaps(const AVLSegment& s) const;

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

 int compareTo(const AVLSegment& s) const;

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

  int split(const AVLSegment& s, AVLSegment& left, AVLSegment& common,
            AVLSegment& right, const bool checkOwner = true) const;


/*
~splitAt~

This function divides a segment into two parts at the point
provided by (x, y). The point must be on the interior of this segment.

*/

  void splitAt(const double x, const double y,
               AVLSegment& left,
               AVLSegment& right)const;


/*
~splitCross~

Splits two crossing segments into the 4 corresponding parts.
Both segments have to cross each other.

*/
void splitCross(const AVLSegment& s, AVLSegment& left1, AVLSegment& right1,
                AVLSegment& left2, AVLSegment& right2) const;


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
   int compareSlopes(const AVLSegment& s) const;


/*
~XOverlaps~

Checks whether the x interval of this segment overlaps the
x interval of ~s~.

*/

  bool xOverlaps(const AVLSegment& s) const;


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

ostream& operator<<(ostream& o, const AVLSegment& s);

}


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
                  priority_queue<HalfSegment,
                                 vector<HalfSegment>,
                                 greater<HalfSegment> >& q1,
                  priority_queue<HalfSegment,
                                 vector<HalfSegment>,
                                 greater<HalfSegment> >& q2);

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
                      priority_queue<HalfSegment,
                                     vector<HalfSegment>,
                                     greater<HalfSegment> >& q1,
                      priority_queue<HalfSegment,
                                     vector<HalfSegment>,
                                     greater<HalfSegment> >& q2);

/*
~splitByNeighbours~

Splits the segments ~leftN~ and ~rightN~ at their crossing point and
replaces them by their left parts in ~sss~. The right parts are
inserted into the queues.

*/
void splitNeighbours(avltree::AVLTree<avlseg::AVLSegment>& sss,
                     avlseg::AVLSegment const*& leftN,
                     avlseg::AVLSegment const*& rightN,
                     priority_queue<HalfSegment,
                                    vector<HalfSegment>,
                                    greater<HalfSegment> >& q1,
                     priority_queue<HalfSegment,
                                    vector<HalfSegment>,
                                    greater<HalfSegment> >& q2);

/*
~SetOp~

This function realizes some set operations on two line objects.
~op~ may be instantiated with avlseg::union[_]op, avlseg::difference[_]op,
or avlseg::intersection[_]op.

*/
Line* SetOp(const Line& line1, const Line& line2, avlseg::SetOperation op);

/*
~SetOp~

This function realizes some set operations on two region objects.
~op~ may be instantiated with avlseg::union[_]op, avlseg::difference[_]op,
or avlseg::intersection[_]op.

*/
Region* SetOp(const Region& reg1, const Region& reg2, avlseg::SetOperation op);

/*
~Realminize~

Creates a reamlinized representation of the HalfSegments in ~segments~. This means,
if halfsegments overlap, only one of the segments left. If halfsegments are crossing
or touching at their interior, their are split.

*/
DBArray<HalfSegment>* Realminize(const DBArray<HalfSegment>& segments);

/*
~Split~

This function is similar to Realminize. In contrast to that function, segments covering the
same space left instead to replace by a single segment.

*/
DBArray<HalfSegment>* Split(const DBArray<HalfSegment>& segments);

/*
~hasOverlaps~

This function checks whether in ~segments~ at least two segments overlap.

*/
bool hasOverlaps(const DBArray<HalfSegment>& segments,
                 const bool ignoreEqual=false);


/*
~isSpatialType~

This function checks whether ~type~ represents a spatial type, i.e. point, points,
line, region.

*/
bool IsSpatialType(ListExpr type);
avlseg::ownertype selectNext(Region const* const reg1,
                     int& pos1,
                     Region const* const reg2,
                     int& pos2,
                     priority_queue<HalfSegment,
                                    vector<HalfSegment>,
                                    greater<HalfSegment> >& q1,
                     priority_queue<HalfSegment,
                                    vector<HalfSegment>,
                                    greater<HalfSegment> >& q2,
                     HalfSegment& result,
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
avlseg::ownertype selectNext(Line const* const line1,
                     int& pos1,
                     Line const* const line2,
                     int& pos2,
                     priority_queue<HalfSegment,
                                    vector<HalfSegment>,
                                    greater<HalfSegment> >& q1,
                     priority_queue<HalfSegment,
                                    vector<HalfSegment>,
                                    greater<HalfSegment> >& q2,
                     HalfSegment& result,
                     int& src
                    );

avlseg::ownertype selectNext(Line const* const line,
                     int& pos1,
                     Region const* const region,
                     int& pos2,
                     priority_queue<HalfSegment,
                                    vector<HalfSegment>,
                                    greater<HalfSegment> >& q1,
                     priority_queue<HalfSegment,
                                    vector<HalfSegment>,
                                    greater<HalfSegment> >& q2,
                     HalfSegment& result,
                     int& src
                    );

avlseg::ownertype selectNext( Line const* const line,
                      priority_queue<HalfSegment,
                                     vector<HalfSegment>,
                                     greater<HalfSegment> >& q,
                      int& posLine,
                      Points const* const point,
                      int& posPoint,
                      HalfSegment& resHs,
                      Point& resPoint);

avlseg::ownertype selectNext( Line const* const line,
                      priority_queue<HalfSegment,
                                     vector<HalfSegment>,
                                     greater<HalfSegment> >& q,
                      int& posLine,
                      Point const* const point,
                      int& posPoint, // >0: point already used
                      HalfSegment& resHs,
                      Point& resPoint);

void Realminize2(const Line& src, Line& result);


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


#endif // __SPATIAL_ALGEBRA_H__
