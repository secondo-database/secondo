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

#include <stack>
#include "StandardAttribute.h"
#include "DBArray.h"
#include "RectangleAlgebra.h"
#include "TopRel.h"

using namespace toprel;

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
/*
Forward declarations.

3 Auxiliary Functions

*/
const double FACTOR = 0.00000001;

inline double ApplyFactor( const double d );
inline bool AlmostEqual( const double d1, 
                         const double d2 );
inline bool AlmostEqual( const Point& p1, 
                         const Point& p2 );
inline bool AlmostEqual( const HalfSegment& hs1, 
                         const HalfSegment& hs2 );

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
      assert( defined );
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

*Complexity:* $O(1)$

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
4.3.15 Operation ~add~

*Precondition:* ~p1.IsDefined(), p2.IsDefined()~

*Semantics:*  ~(p1.x + p2.x, p1.y + p2.y)~

*Complexity:* $O(1)$

*/
    inline Point Add( const Point& p ) const;

/*
4.4 Functions needed to import the the ~point~ data type to tuple

There are totally 8 functions which are defined as virtual functions. They need
to be defined here in order for the Point data type to be used in Tuple definition
as an attribute.

*/
    inline bool IsDefined() const;
    inline void SetDefined( bool defined );
    inline size_t Sizeof() const;

    inline size_t HashValue() const
    {
      if( !defined )
        return 0;
      return (size_t)(5*x + y);
    }

    inline void CopyFrom( const StandardAttribute* right )
    {
      const Point* p = (const Point*)right;
      defined = p->defined;
      if( defined )
        Set( p->x, p->y );
    }

    inline int Compare( const Attribute *arg ) const
    {
      if( !defined )
        return -1;
      const Point* p = (const Point*)arg;
      if( !p )
        return -2;
      if( !defined && !p->defined )
        return 0;
      if( !p->defined )
        return 1;
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

  protected:
/*
4.5 Attributes

*/
    bool defined;
/*
A flag that tells if the point is defined or not.

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
int PointCompare( const void *a, const void *b );

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
    Int9M GetTopRel( const Points& ps ) const;
/*
??? - See the TopRelAlgebra.

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
4.3.14 Operation ~translate~

*Precondition:* ~U.IsOrdered()~

*Semantics:* ~U + (x, y)~

*Complexity:* $O(n)$, where ~n~ is the size of ~U~

*/
    void Translate( const Coord& x, const Coord& y, Points& ps ) const;
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
    inline bool IsDefined() const;
    inline void SetDefined( bool Defined );
    inline size_t Sizeof() const;
    size_t HashValue() const;
    void CopyFrom( const StandardAttribute* right );
    int Compare( const Attribute *arg ) const;
    bool Adjacent( const Attribute *arg ) const;
    virtual Points* Clone() const;
    ostream& Print( ostream &os ) const;

  private:
/*
5.7 Private member functions

*/
    void Sort();
/*
Sorts the persistent array of points.

*/
    void RemoveDuplicates();
/*
Remove duplicates in the (ordered) array of points. 

*/
    bool Find( const Point& p, int& pos ) const;
/*
Searches (binary search algorithm) for a point in the point set and
returns its position. Returns -1 if the point is not found.

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
    bool Intersection( const HalfSegment& hs, HalfSegment& hs ) const;
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
    double Distance( const Point& p ) const;
/*
Computes the distance from the half segment to the point ~p~.

*/
    double Distance( const HalfSegment& hs ) const;
/*
Computes the minimum distance from two half segments.

*/
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
    inline Line() {}
/*
This constructor should not be used.

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
     void EndBulkLoad( bool sort = true, bool remDup = true, 
                       bool setPartnerNo = true, bool setNoComponents = true );
/*
Marks the end of a bulk load and sorts the half segments set if the argument ~sort~ is set to true.

6.2 Member functions

*/
    inline void SetNoComponents( int noComponents );
/*
Sets the number of components with the given argument value ~noComponents~.

*/
    inline double Length() const;
/*
Returns the length of the line, i.e. the sum of the lengths of all segments.

*/
    inline void SetLength( double length );
/*
Sets the length of the line.

*/
    inline void SetLineType( bool simple, bool cycle, bool startsSmaller );
/*
Sets the flags that indicate the line type.

*/
    inline const Rectangle<2> BoundingBox() const;
/*
Returns the bounding box of the line.

*/
    inline void SetBoundingBox( const Rectangle<2>& bbox );
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
    inline Point StartPoint( bool startsSmaller ) const;
/*
Returns the starting point of the line.

*/
    inline Point EndPoint( bool startsSmaller ) const;
/*
Returns the end point of the line.

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

    inline void Put( const int i, const HalfSegment& hs );
/*
Writes the the half segment ~hs~ to the ith position.

*/
    inline void Get( const int i, const LRS*& lrs ) const;
/*
Reads the ith ~lrs~ from the line value.

*/
    inline void Put( const int i, const LRS& lrs );
/*
Writes the the ~lrs~ to the ith position.

*/
    Line& operator=( const Line& cl );
/*
Assignement operator redefinition.

6.4 Opearations

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
/*
4.3.14 Operation ~transform~

*Precondition:* ~U.IsOrdered()~

*Semantics:* ~U -> R~

*Complexity:* $O(n)$, where ~n~ is the size of ~U~

*/
    void Transform( Region& r ) const;
/*

6.4.6 Operation ~atposition~

*Precondition:* ~U.IsOrdered()~

*Semantics:* translates the line 

*Complexity:* $O(n)$, where ~n~ is the size of ~U~

*/
    bool 
    AtPosition( double pos, bool startsSmaller, Point& p ) const;
/*
6.4.6 Operation ~atpoint~

*Precondition:* ~U.IsOrdered() and v.IsDefined()~

*Semantics:* 

*Complexity:* $O(n)$, where ~n~ is the size of ~U~

*/
    bool 
    AtPoint( const Point& p, bool startsSmaller, double& result ) const;
/*
6.4.6 Operation ~subline~

*Precondition:* ~U.IsOrdered() and v.IsDefined() and w.IsDefined~

*Semantics:* 

*Complexity:* $O(n)$, where ~n~ is the size of ~U~

*/
    void SubLine( double pos1, double pos2, 
                  bool startsSmaller, Line& l ) const;
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
      return 2;
    }

    inline FLOB *GetFLOB( const int i )
    {
      if( i == 0 )
        return &line;
      return &lrsArray;
    }

    inline bool IsDefined() const
    {
      return true;
    }

    inline void SetDefined( bool Defined )
    {}

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
    virtual Line *Clone() const;
    ostream& Print( ostream &os ) const;
    void Clear();

  private:
/*
6.9 Private member functions

*/
    void Sort();
/*
Sorts (quick-sort algorithm) the persistent array of half segments in the line value.

*/
    void RemoveDuplicates();
/*
Remove duplicates in the (ordered) array of half-segments.

*/
    bool Find( const HalfSegment& hs, int& pos ) const;
/*
Searches (binary search algorithm) for a half segment in the line value and
returns its position. Returns false if the half segment is not found.

*/
    bool Find( const Point& p, int& pos ) const;
/*
Searches (binary search algorithm) for a point in the line value and
returns its position, i.e. the first half segment with dominating point 
less than or equal to ~p~. Returns true if the half segment dominating
point is equal to ~p~ and false otherwise.

*/
    bool Find( const LRS& lrs, int& pos ) const;
/*
Searches (binary search algorithm) for a ~lrs~ in the ~lrsArray~ value and
returns its position. Returns false if the ~lrs~ is not found.

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
    void 
    VisitHalfSegments( int& poshs, const HalfSegment*& hs, double& lrspos,
                       int& edgeno, int cycleno, int faceno,
                       stack< pair<int, const HalfSegment*> >& nexthss,
                       vector<bool>& visited );
    void SetNoComponents();
/*
Calculates and sets the number of components for the line. For every half segment, the following 
information is stored: faceno contains the number of the component, cycleno contains the 
ramification that the segment belongs, and edgeno contains the segment number.

The method ~VisitHalfSegments~ is a recursive function that does the job for 
~SetNoComponents~.

6.10 Atrtibutes

*/
    DBArray<HalfSegment> line;
/*
The persisten array of half segments.

*/
    DBArray<LRS> lrsArray;
/*
The persistent array containing the Linear Referencing System ordering.

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
    bool simple;
/*
Tells whether this is a simple line, which means it has only one components and no branches.
This is sometimes called a ~line string~. We only have the linear referencing system
for simple lines, it makes no sense for the other cases.

*/
    bool cycle;
/*
Tells whether this is a simple line and a cycle.

*/
    bool startsSmaller;
/*
For simple lines without cycles, tells whether the starting point of the linear
referencing system is smaller (lexicographic order) than the end point.

*/
};

/*
6.11 overloaded output operator

*/
ostream& operator<<( ostream& o, const Line& cl );

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
    inline Region() {}
/*
This constructor should not be used.

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
      return Size() == 0;
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

    inline bool IsDefined() const
    {
      return true;
    }

    inline void SetDefined( bool defined )
    {}

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
    ostream& Print( ostream &os ) const;
    virtual Region *Clone() const;
    void Clear();

/*

VTA - Move this to the operators section

~Translate~

Moves the region according x and y and stores the result in result.


*/

    void Translate(const Coord& x, const Coord& y, Region& result) const;

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
  defined( d ),
  x( x ),
  y( y )
{}

inline Point::Point( const Point& p ) :
  defined( p.IsDefined() )
{
  if( defined )
  {
    x = p.x;
    y = p.y;
  }
}

inline const Rectangle<2> Point::BoundingBox() const
{
  assert( defined );
  return Rectangle<2>( true,
                       x - ApplyFactor(x),
                       x + ApplyFactor(x),
                       y - ApplyFactor(y),
                       y + ApplyFactor(y) );
}

inline void Point::Set( const Coord& x, const Coord& y )
{
  defined = true;
  this->x = x;
  this->y = y;
}

inline Point Point::Translate( const Coord& x, const Coord& y ) const
{
  assert( defined );
  return Point( true, this->x + x, this->y + y );
}

inline void Point::Translate( const Coord& x, const Coord& y )
{
  if( defined )
  {
    this->x += x;
    this->y += y;
  }
}

inline Point Point::Add( const Point& p ) const
{
  assert( defined && p.defined );
  return Point( true, this->x + p.x, this->y + p.y );
}

inline Point& Point::operator=( const Point& p )
{
  defined = p.defined;
  if( defined )
  {
    x = p.x;
    y = p.y;
  }
  return *this;
}

inline bool Point::operator==( const Point& p ) const
{
  assert( defined && p.defined );
  return x == p.x && y == p.y;
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
  assert( defined && p.defined );
  return x < p.x ||
         x == p.x && y < p.y;
}

inline bool Point::operator>=( const Point& p ) const
{
  return !( *this < p );
}

inline bool Point::operator>( const Point& p ) const
{
  assert( defined && p.defined );
  return x > p.x ||
         x == p.x && y > p.y;
}

inline Point Point::operator+( const Point& p ) const
{
  assert( defined && p.defined );
  return Point( true, x + p.x, y + p.y );
}

inline Point Point::operator-( const Point& p ) const
{
  assert( defined && p.defined );
  return Point( true, x - p.x, y - p.y );
}

inline Point Point::operator*( const double d ) const
{
  assert( defined );
  return Point( true, x * d, y * d );
}

inline bool Point::IsDefined() const
{
  return defined;
}

inline void Point::SetDefined( bool defined )
{
  this->defined = defined;
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
{}

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
  return points.Size() == 0;
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

inline bool Points::IsDefined() const
{
  return true;
}

inline void Points::SetDefined( bool defined )
// since every points is defined, this function does nothing
{
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
lrsArray( n / 2 ),
bbox( false ),
ordered( true ),
noComponents( 0 ),
length( 0.0 ),
simple( true ),
cycle( false ),
startsSmaller( false )
{}

inline Line::Line( const Line& cl ) :
line( cl.Size() ),
lrsArray( cl.lrsArray.Size() ),
bbox( cl.bbox ),
ordered( true ),
noComponents( cl.noComponents ),
length( cl.length ),
simple( cl.simple ),
cycle( cl.cycle ),
startsSmaller( cl.startsSmaller )
{
  assert( cl.IsOrdered());

  const HalfSegment *hs;
  for( int i = 0; i < cl.Size(); i++ )
  {
    cl.Get( i, hs );
    line.Put( i, *hs );
  }

  const LRS *lrs;
  for( int i = 0; i < cl.lrsArray.Size(); i++ )
  {
    cl.lrsArray.Get( i, lrs );
    lrsArray.Put( i, *lrs ); 
  }
}

inline void Line::Destroy()
{
  line.Destroy();
  lrsArray.Destroy();
}

inline void Line::SetNoComponents( int noComponents )
{
  this->noComponents = noComponents;
}

inline void Line::SetLength( double length )
{
  this->length = length;
}

inline double Line::Length() const
{
  return length;
}

inline void 
Line::SetLineType( bool simple, bool cycle, bool startsSmaller )
{
  this->simple = simple;
  this->cycle = cycle;
  this->startsSmaller = startsSmaller;
}

inline const Rectangle<2> Line::BoundingBox() const
{
  return bbox;
}

inline void Line::SetBoundingBox( const Rectangle<2>& bbox )
{
  this->bbox = bbox;
}

inline bool Line::IsOrdered() const
{
  return ordered;
}

inline bool Line::IsEmpty() const
{
  return (line.Size() == 0);
}

inline int Line::Size() const
{
  return line.Size();
}

inline Point Line::StartPoint( bool startsSmaller ) const
{
  if( IsEmpty() )
    return Point( false );

  if( startsSmaller && this->startsSmaller )
    pos = 0;
  else  
    pos = Size() - 1;

  const LRS *lrs;
  Get( pos, lrs );

  const HalfSegment* hs;
  Get( lrs->hsPos, hs );

  return pos == 0 ?
         hs->GetDomPoint() :
         hs->GetSecPoint();
}

inline Point Line::EndPoint( bool startsSmaller ) const
{
  if( IsEmpty() )
    return Point( false );

  if( startsSmaller && this->startsSmaller )
    pos = Size()-1;
  else  
    pos = 0;

  const LRS *lrs;
  Get( pos, lrs );

  const HalfSegment* hs;
  Get( lrs->hsPos, hs );

  return pos == 0 ?
         hs->GetDomPoint() :
         hs->GetSecPoint();
}
inline void Line::Get( const int i, const HalfSegment*& hs ) const
{
  line.Get( i, hs );
}

inline void Line::Resize(const int newSize){
   if(newSize>Size()){
      line.Resize(newSize);
   }
}

inline void Line::Put( const int i, const HalfSegment& hs )
{
  line.Put( i, hs );
}

inline void Line::Get( const int i, const LRS*& lrs ) const
{
  lrsArray.Get( i, lrs );
}

inline void Line::Put( const int i, const LRS& lrs )
{
  lrsArray.Put( i, lrs );
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
{}

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

/*
11.4 Auxiliary functions

*/
inline bool AlmostEqual( const double d1, const double d2 )
{
  double i1, i2;
  double dd1 = modf( d1, &i1 ),
         dd2 = modf( d2, &i2 );
  long ii1 = (long)i1,
       ii2 = (long)i2;

  if( abs(ii1 - ii2) > 1 ) 
    return false;

  int d = abs(ii1) - abs(ii2);
  return fabs(dd1 - dd2 - d) < FACTOR;
}

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

#endif // __SPATIAL_ALGEBRA_H__
