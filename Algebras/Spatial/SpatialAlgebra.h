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

[1] Header File of the Spatial Algebra

February, 2003 Victor Teixeira de Almeida

March-July, 2003 Zhiming DING

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

#include "StandardAttribute.h"
#include "DBArray.h"
#include "RectangleAlgebra.h"

//#define RATIONAL_COORDINATES
#define DOUBLE_COORDINATES

#ifdef DOUBLE_COORDINATES
  typedef double Coord;
#else
  #ifdef RATIONAL_COORDINATES
    #ifdef WITH_ARBITRARY_PRECISION
      #include "Rational.h"
      typedef Rational Coord;
    #else
      #include "Rational.h"
      typedef Rational Coord;
    #endif
  #else
    typedef long Coord;
  #endif
#endif

/*
There are two main defines that will control how the coordinate system is
implemented, namely ~RATIONAL\_COORDINATES~ and ~WITH\_ARBITRARY\_PRECISION~.
The first one defines if the system will use rational coordinates instead
of simple integer (~long~) ones. The second one is used only if rational
coordinates are being used and defines if the system will use rational
numbers with arbitrary precision (~not implemented yet~) instead of fixed
precision (~Rational~).

3 Auxiliary Functions

*/
const double FACTOR = 0.0001;

class Point;

bool AlmostEqual( const double d1, const double d2 );
bool AlmostEqual( const Point& p1, const Point& p2 );

/*
3 Class StandardSpatialAttribute

Now implemented in RectangleAlgebra.

*/



/*
3 Class Point

This class implements the memory representation of the ~point~ type
constructor. A point represents a point in the Euclidean plane or is
undefined. The figure \cite{fig:spatialdatatypes.eps} shows an example
of a point data type.

*/
class Points;
/*
A forward declaration of the class ~Points~.

*/

class Point: public StandardSpatialAttribute<2>
{
  public:
/*
3.1. Constructors and Destructor

*/
    Point() {};
/*
This constructor should not be used.

There are two ways of constructing a point:

*/
    Point( const bool d, const Coord& x = Coord(), const Coord& y = Coord() );
/*
The first one receives a boolean value ~d~ indicating if the point is defined
and two coordinate ~x~ and ~y~ values.

*/
    Point( const Point& p );
/*
The second one receives a point ~p~ as argument and creates a point that is a
copy of ~p~.

*/
    ~Point();
/*
The destructor.

3.2 Member functions

*/
    inline const Coord& GetX() const;
/*
Returns the ~x~ coordinate of the point.

*Precondition:* ~IsDefined()~

*/
    inline const Coord& GetY() const;
/*
Returns the ~y~ coordinate of the point.

*Precondition:* ~IsDefined()~

*/
    inline const Rectangle<2> BoundingBox() const;
/*
Returns the point bounding box which is also a point.

*/

    inline void Set( const Coord& x, const Coord& y );
/*
Sets the value of the point object.

*/
    inline void Translate( const Coord& x, const Coord& y );
/*
Translates a point position by adding values ~x~ and ~y~.

*/
    inline Point& operator=(const Point& p);
/*
Assignement operator redefinition.

3.3 Operations

3.3.1 Operation $=$ (~equal~)

*Precondition:* ~u.IsDefined() $\&\&$ v.IsDefined()~

*Semantics:* $u = v$

*Complexity:* $O(1)$

*/
    inline bool operator==( const Point& p ) const;
/*
3.3.2 Operation $\neq$ (~not equal~)

*Precondition:* ~u.IsDefined() $\&\&$ v.IsDefined()~

*Semantics:* $u \neq v$

*Complexity:* $O(1)$

*/
    inline bool operator!=( const Point& p ) const;
/*
3.3.3 Operation $\leq$ (~less or equal~)

*Precondition:* ~u.IsDefined() $\&\&$ v.IsDefined()~

*Semantics:* $u \leq v$

*Complexity:* $O(1)$

*/
    inline bool operator<=( const Point& p ) const;
/*
3.3.4 Operation $<$ (~less than~)

*Precondition:* ~u.IsDefined() $\&\&$ v.IsDefined()~

*Semantics:* $u < v$

*Complexity:* $O(1)$

*/
    inline bool operator<( const Point& p ) const;
/*
3.3.5 Operation $\geq$ (~greater or equal~)

*Precondition:* ~u.IsDefined() $\&\&$ v.IsDefined()~

*Semantics:* $u \geq v$

*Complexity:* $O(1)$

*/
    inline bool operator>=( const Point& p ) const;
/*
3.3.6 Operation $>$ (~greater than~)

*Precondition:* ~u.IsDefined() $\&\&$ v.IsDefined()~

*Semantics:* $u > v$

*Complexity:* $O(1)$

*/
    inline bool operator>( const Point& p ) const;
/*
3.3.7 Operation ~inside~ (with ~points~)

*Precondition:* ~u.IsDefined() $\&\&$ V.IsOrdered()~

*Semantics:* $u \in V$

*Complexity:* $O(log(n))$, where ~n~ is the size of the point set ~V~

*/
    bool Inside( Points& ps ) const;
/*
3.3.8 Operation ~inside~ (with ~rectangle~)

*Precondition:* ~u.IsDefined()~

*Semantics:* $u \in V$

*Complexity:* $O(1)$

*/
    bool Inside( const Rectangle<2>& r ) const;
/*
3.3.8 Operation ~intersection~ (with ~point~)

*Precondition:* ~u.IsDefined() $\&\&$ v.IsDefined()~

*Semantics:* if $u = v$ then $u$ else $\perp$

*Complexity:* $O(1)$

*/
    void Intersection( const Point& p, Point& result ) const;
/*
3.3.9 Operation ~intersection~ (with ~points~)

*Precondition:* ~u.IsDefined() $\&\&$ V.IsOrdered()~

*Semantics:* if $u \in V$ then $u$ else $\perp$

*Complexity:* $O(log(n))$, where ~n~ is the size of the point set ~V~

*/
    void Intersection( Points& ps, Point& result ) const;
/*
3.3.10 Operation ~minus~ (with ~point~)

*Precondition:* ~u.IsDefined() $\&\&$ v.IsDefined()~

*Semantics:* if $u = v$ then $\perp$ else $u$

*Complexity:* $O(1)$

*/
    void Minus( const Point& p, Point& result ) const;
/*
3.3.9 Operation ~minus~ (with ~points~)

*Precondition:* ~u.IsDefined() $\&\&$ V.IsOrdered()~

*Semantics:* if $u \in V$ then $\perp$ else $u$

*Complexity:* $O(log(n))$, where ~n~ is the size of the point set ~V~

*/
    void Minus( Points& ps, Point& result ) const;
/*
3.3.10 Operation ~distance~

*Precondition:* ~u.IsDefined()~ and ~v.IsDefined()~

*Semantics:* compute the distance between u and v

*Complexity:* $O(1)$

*/
    double Distance( const Point& p ) const;
/*
3.3.11 Functions needed to import the the ~Point~ data type to tuple

There are totally 10 functions which are defined as virtual functions. They need
to be defined here in order for the Point data type to be used in Tuple definition
as an attribute.

*/
    inline bool IsDefined() const;
    inline void SetDefined(bool Defined);
    size_t HashValue();
    void CopyFrom(StandardAttribute* right);
    int Compare(Attribute * arg);
    bool Adjacent(Attribute * arg);
    int Sizeof() const;
    Point* Clone();
    ostream& Print( ostream &os );

/*
3.4 Attributes

*/
  private:

    Coord x;
/*
The ~x~ coordinate.

*/
    Coord y;
/*
The ~y~ coordinate.

*/
    bool defined;
/*
A flag that tells if the point is defined or not.

*/
};

typedef Point CPoint;


/*
4 Class Points

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
4.1 Constructors and Destructor

There are three ways of constructing a point set:

*/
    Points() {}
/*
This constructor should not be used.

*/
    Points( const int initsize );
/*
The first one constructs an empty point set but open space for ~initsize~ points.

*/
    Points( Points& ps);
/*
The second one receives another point set ~ps~ as argument and constructs a point
set which is a copy of ~ps~.

*/
    void Destroy();
/*
This function should be called before the destructor if one wants to destroy the
persistent array of points. It marks the persistent array for destroying. The
destructor will perform the real destroying.

*/
    ~Points();
/*
The destructor.

4.2 Functions for Bulk Load of Points

As said before, the point set is implemented as an ordered persistent array of points.
The time complexity of an insertion operation in an ordered array is $O(n)$, where ~n~
is the size of the point set. In some cases, bulk load of points for example, it is good
to relax the ordered condition to improve the performance. We have relaxed this ordered
condition only for bulk load of points. All other operations assume that the point set is
ordered.

*/
    bool IsOrdered() const;
/*
Returns if the point se is ordered. There is a flag ~ordered~ (see attributes) in order
to avoid a scan in the point set to answer this question.

*/
    void     setOrdered(bool isordered);

    void StartBulkLoad();
/*
Marks the begin of a bulk load of points relaxing the condition that the points must be
ordered.

*/
    void EndBulkLoad();
/*
Marks the end of a bulk load and sorts the point set.

4.3 Member functions

*/
    const Rectangle<2> BoundingBox() const;
/*
Returns the bounding box that spatially contains all points.

*/
    bool IsEmpty() const;
/*
Returns if the set is empty of not.

*/
    int Size() const;
/*
Returns the size of the point set. Returns ~0~ if the set is empty.

*/
    void Get( const int i, Point& p );
/*
Retrieves the point ~p~ at position ~i~ in the point set.

*Precondition:* $0 \leq i < Size()$

*/
    Points& operator=(Points& ps);
/*
Assignement operator redefinition.

*Precondition:* ~ps.IsOrdered()~

*/
    bool Contains(const Point& p);
/*
Searches (binary search algorithm) for a point in the point set and
return ~true~ if found and ~false~ if not.

*Precondition:* ~this.IsOrdered() $\&\&$ p.IsDefined()~

*/
    bool Contains(Points& ps);
/*
Returns ~true~ if this point set contains the ~ps~ point set and
~false~ otherwise.

*Precondition:* ~this.IsOrdered() $\&\&$ ps.IsOrdered()~

4.4 Operations

4.4.1 Operation $=$ (~equal~)

*Precondition:* ~this.IsOrdered() $\&\&$ ps.IsOrdered()~

*Semantics:* $this = ps$

*Complexity:* $O(n+m)$, where ~n~ is the size of this point set and m the size of ~ps~.

*/
    bool operator==(Points&);
/*
4.4.2 Operation $\neq$ (~not equal~)

*Precondition:* ~this.IsOrdered() $\&\&$ ps.IsOrdered()~

*Semantics:* $this = ps$

*Complexity:* $O(n+m)$, where ~n~ is the size of this point set and m the size of ~ps~.

*/
    bool operator!=(Points&);
/*
4.4.3 Operation ~union~ (with ~point~)

*Precondition:* ~p.IsDefined()~

*Semantics:* $this \cup \{p\}$

*Complexity:* $O(1)$, if the set is not ordered, and $O(log(n)+n)$, otherwise. ~n~ is the size
of this point set.

*/
    Points& operator+=(const Point& p);
/*
4.4.4 Operation ~union~ (with ~points~)

*Precondition:*

*Semantics:* $this \cup ps$

*Complexity:* $O(m)$, if the set is not ordered, and $O(m+(m+n)log(m+n))$, otherwise. ~n~ is the size
of this point set and ~m~ is the size of ~ps~.

*/
    Points& operator+=(Points& ps);
/*
4.4.5 Operation ~minus~ (with ~point~)

*Precondition:* ~p.IsDefined() $\&\&$ this.IsOrdered()~

*Semantics:* $this \backslash \{p\}$

*Complexity:* $O(log(n)+n)$

*/
    Points& operator-=(const Point& p);
/*
4.4.6 Operation ~inside~

*Precondition:* ~this.IsOrdered() $\&\&$ ps.IsOrdered()~

*Semantics:* $this \subseteq ps$

*Complexity:* $O(n+m)$, where ~n~ is the size of this point set and m the size of ~ps~.

*/
    bool Inside(Points& ps);
/*
4.4.7 Operation ~intersects~

*Precondition:* ~this.IsOrdered() $\&\&$ ps.IsOrdered()~

*Semantics:* $this \cap ps$

*Complexity:* $O(n+m)$, where ~n~ is the size of this point set and m the size of ~ps~.

*/
    bool Intersects(Points& ps);
/*
4.4.8 Object Traversal Functions

*Precondition:* ~this.IsOrdered()~

*Semantics:*  These functions are object traversal functions which is useful when we are
using ROSE algebra algorithms (DZM).

*Complexity:* $O(1)$.

*/
    void SelectFirst();
    void SelectNext();
    bool EndOfPt();
    void GetPt( Point& p );
    void InsertPt( Point& p );

    void     Clear();

    private:
/*
4.5 Private member functions

*/
    void Sort();
/*
Sorts the persistent array of points.

*/
    int Position(const Point&);
/*
Searches (binary search algorithm) for a point in the point set and
returns its position. Returns -1 if the point is not found.

4.6 Functions needed to import the the ~Points~ data type to tuple

There are totally 10 functions which are defined as virtual functions. They need
to be defined here in order for the Point data type to be used in Tuple definition
as an attribute.

*/
    int NumOfFLOBs();
    FLOB *GetFLOB(const int i);

    bool     IsDefined() const;
    void     SetDefined(bool Defined);
    size_t   HashValue();
    void     CopyFrom(StandardAttribute* right);
    int      Compare(Attribute * arg);
    bool     Adjacent(Attribute * arg);
    int      Sizeof() const;
    Points*    Clone() ;
    ostream& Print( ostream &os );

/*
4.6 Atrtibutes

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
    int pos;
/*
 Added by DZM. According to ROSE algebra, the carrier set of points should contain a pos pointer

*/
};

typedef Points CPoints;

/*
4.7 Auxiliary functions

*/
ostream& operator<<( ostream& o, const Point& p );
/*
Print the point ~p~ in the out stream ~o~.

*/
ostream& operator<<( ostream& o, Points& ps );
/*
Print the point set ~ps~ int the out stream ~o~.

*/

/*
The following type definition indicates the structure of the ~attr~ value associated with
half segments. This attribute is utilized only when we are handling regions. In line values
this attibute is ignored.

*/
struct AttrType
{
  AttrType() {}

  int faceno;
  int cycleno;
  int edgeno;
  int coverageno;  //this number is used for the fast spacial scan of the inside_pr algorithm
  //set<int> attr;
  int attr;
};

/*
5 Class Half Segment

This class implements the memory representation of  ~halfsegment~. Although ~halfsegment~
is not an independent type constructor, it is the basic construction unit of the ~line~ and the ~region~
type constructors.

A ~halfsegment~ value is composed of a pair of points, and a boolean flag indicating the dominating
point. The left point is always smaller than the right one.

The names for the classes CHalfSegment, CLine, and CRegion are started with a ~C~ prefix, which
indicats that the name is for a class.  Since very similar names (class names, parry names, and type names)
appear in the source code, it is a good idea to put this prefix to reduce the possibility of confusion.

*/

class CHalfSegment
{
  public:

/*
5.1 Constructors and Destructor

A Half Segment is composed of two points which are called ~left point~ LP and ~right point~ RP
(LR \verb+<+ RP), and a ~flag~ LDP (~Left Dominating Point~) which tells which point is the
dominating point. The Boolean Flag ~Defined~ allows us to use an ~undifined~ value.

*/
    CHalfSegment() {}
/*
This constructor should not be used.

*/
    CHalfSegment( bool Defined, bool LDP = false, const Point& LP = Point( false ), const Point& RP = Point( false ) );
    CHalfSegment( const CHalfSegment& chs );
    ~CHalfSegment();

/*
5.2 Functions Reading Property Values from a CHalfSegment Object

*Semantics:* Reading Property values of a half segment.

*Complexity:* $O( 1)$

*/
    bool IsDefined() const;
/*
This function returns a boolean value indicating whether the half segment is defined.

*/
    const Point&  GetLP() const;
/*
This function returns the left point of the half segment.

*/
    const Point&  GetRP() const;
/*
This function returns the right point of the half segment.

*/
    const Point&  GetDPoint() const;
/*
This function returns the dominating point of the half segment.

*/
    const Point&  GetSPoint() const;
/*
This function returns the secondary point of the half segment.

*/
    bool GetLDP() const;
/*
This function returns the boolean flag which indicates whether the dominating point is on the
left side.

*/
    const Rectangle<2> BoundingBox() const;
/*
Returns the bounding box of the half segment.

*/
    const AttrType&  GetAttr() const;
/*
This function returns the "attribute" value associated with a half segment. The "attribute" argument is
useful when we process region values.

5.3 Functions Setting Property Values of a CHalfSegment Object

*Semantics:* Writing Property values of a half segment.

*Complexity:* $O( 1)$

*/
    void     Set(bool LDP, const Point& LP, const Point& RP);
/*
This function sets the value of a half segment. The parameter LP and RP can ignore the order, and the
function will compare the parameter points and put the smaller one to LP and larger one to RP.

*/
    void Translate( const Coord& x, const Coord& y );

    void     SetDefined(bool Defined);
/*
This function sets the value of the "defined" argument of a half segment.

*/
    void     SetAttr(AttrType& ATTR);
/*
This function sets the value of the "attr" argument of a half segment.

*/
    void     SetLDP(bool LDP);
/*
This function sets the value of the "Left Dominating Point" flag of a half segment.

5.4 Overloaded class operators

5.4.1 Operation $=$ (~assignment~)

*Semantics:* $this = chs$. It assigns the value of a half segment to another.

*Complexity:* $O( 1)$

*/
    CHalfSegment& operator=(const CHalfSegment& chs);
/*
5.4.2 Auxiliary Function (~compare~)

*Semantics:* This function make comparison between two halfsegments. The rule of the comparison is specified
in the ROSE Algebra paper. That is:  the half sgenments will be ordered according to the following values:
dominating points -\verb+>+  LDP flages  -\verb+>+ directions (rotations).

*Complexity:* $O( 1 )$

*/
    int chscmp(const CHalfSegment& chs) const;
/*
5.4.3 Operation $==$ (~equal~)

*Semantics:* $this ==chs$. It decides whether two half segments are identical.

*Complexity:* $O( 1 )$

*/
    bool operator==(const CHalfSegment& chs) const;
    bool operator!=(const CHalfSegment& chs) const;
/*
5.4.4 Operation $<$ (~less than~)

*Semantics:* $u < v$. It decides whether whether one half segment is smaller than the other.

*Complexity:* $O( 1 )$

*/
    bool operator<(const CHalfSegment& chs) const;
/*
5.4.5 Operation $>$ (~greater than~)

*Semantics:* $u > v$. It decides whether whether one half segment is greater than the other.

*Complexity:* $O( 1 )$

*/
    bool operator>(const CHalfSegment& chs) const;
/*
5.5 Clone Function

*/
    CHalfSegment*  Clone() {return (new CHalfSegment(*this));}

/*
5.6 Intersects Function

*Semantics:*  This group of functions decide whether two half segments intersect with each other.
Since there is no Realm here as a precondition, two half segments may intersect each other with
their middle points. So we defined a lot of different kinds of intersect functions for different purposes.

*Complexity:* $O( 1 )$

*/
    bool Intersects( const CHalfSegment& chs) const;
/*
This function is the most common one, and it computes whether two half segments intersect each other,
no matter where the intersection is. They can be endpoints or middle points

*/
    bool Intersects( const CHalfSegment& chs, CHalfSegment& reschs) const;
/*
This function computes whether two half segments intersect each other, and at the same,
time, it will return the intersected part as a result. The intersected part should be a segment.
If it is a point, the it will be ignored.

*/
    bool spintersect( const CHalfSegment& chs, Point& resp) const;
/*
This function (single point intersects) compute whether two half segments intersect each other
 with a single point, if yes, the intersection point is returned.

*/
    bool overlapintersect( const CHalfSegment& chs, CHalfSegment& reschs ) const;
/*
This function compute whether two half segments intersect each other with a segment, if yes,
the intersection segment is returned.

*/
    bool innerIntersects( const CHalfSegment& chs) const;
/*
This function decides whether two half segments intersect in the following manner: a point of
the first segment and a innerpoint of the second segment is the same.

*/
    bool cross( const CHalfSegment& chs ) const;
/*
This function computes whether two half segments intersect with their mid-points. Be aware
that endpoints are not considered in computing the results.

*/
    bool crossings( const CHalfSegment& chs, Point& p ) const;
/*
This function is ued for the ~crossings~ operator. It  computes whether two half segments
is crossing each other.

*/
    bool overlap( const CHalfSegment& chs) const;
/*
This function computes whether  two half segments overlap each other. If their inner part
intersect, then the result is true.

*/

/*
5.7 Inside Function

*Semantics:* This operation computes whether one half segment is inside another. If segment A is part of
another segment B, then we say A is inside B. eg. -------======-------.

*Complexity:* $O( 1 )$

*/
    bool Inside(const CHalfSegment& chs) const ;
/*
5.8 Contain Function

*Semantics:* This operation computes whether one point is contained in a half segment. If a point P is inside
a segment S, then we say P is contained by S. eg. ---------o---------.

*Complexity:* $O( 1 )$

*/
    bool Contains( const Point& p ) const;
/*
5.9 rayAbove Function

*Semantics:* This function decides whether a half segment is above a point. This is
useful when we want to decide whether a point is inside a region.

*Complexity:* $O( 1 )$

*/
    bool rayAbove( const Point& p, double &abovey0 ) const;

/*
5.10 Operation ~Distance~ (with ~point~)

*Precondition:* ~u.IsDefined()~

*Semantics:*  To compute the Distance between a line segment and a given point

*Complexity:* $O(1)$

*/
    double Distance( const Point& p ) const;

/*
5.11 attribute comparison Functions

*Semantics:* These two operations compare two half segments according to their attribute values. They are
used for the logicsort() function.

*Complexity:* $O( 1 )$

*/
    int logicless(const CHalfSegment& chs) const;
    int logicgreater(const CHalfSegment& chs) const;

  private:
/*
5.11 Properties

*/
    bool defined;
/*
This boolean property indicates whether the half segment is defined.

*/
    bool ldp;
/*
This boolean property indicates whether the half segment has its left point as its dominating point.

*/
    Point lp;
    Point rp;
/*
These two properties give the left and right point of the half segment.

*/
    public:
    AttrType attr;
/*
This ~attribute~ property is useful if we process region values in the way indicated in the ROSE
paper.

*/
};
/*
5.12 overloaded output operator

*/
ostream& operator<<( ostream& o, const CHalfSegment& chs );

/*
6 Class Line

This class implements the memory representation of the ~line~ type constructor. A line value is
actually composed of a set of arbitrarily arranged line segments. In the ROSE algebra paper, it
is called ~lines~.

*/

class CLine: public StandardSpatialAttribute<2>
{
  public:
/*
6.1 Constructors and Destructor

A ~line~ value is a set of halfsegments. In the external (nestlist) representation, a line value is
expressed as a set of segments. However, in the internal (class) representation, it is expressed
as a set of sorted halfsegments, which are stored as a PArray.

*/
    CLine() {}
/*
This constructor should not be used.

*/
    CLine(const int initsize);
    CLine(CLine& cl );
    void Destroy();
    ~CLine();

    const Rectangle<2> BoundingBox() const;

/*
6.2 Functions Reading Property Values from an Object

*Semantics:* Reading Property values of a line value.

*Complexity:* $O( 1)$

*/
    bool IsOrdered() const;
/*
This function decides whether the half segments in the line value is sorted.

*/
    void     setOrdered(bool isordered);

    bool IsEmpty() const;
/*
This function decides whether the line value is empty.

*/
    int Size() const;
/*
This function returns the number of half segments in the line value.

*/
    void Get( const int i, CHalfSegment& chs );
/*
This function reads the ith half segment from the line value.

6.3 Bulkload Functions

*/
    void StartBulkLoad();
/*
This function marks the begin of a bulk load of line relaxing the condition that the half segments
must be ordered.

*/

    void EndBulkLoad();
/*
This function marks the end of a bulk load and sorts the half segments.

6.4 Overloaded Class Operators

6.4.1 Operation $=$ (~assignment~)

*Semantics:* $this = cl$. It assigns the value of a line to another.

*Complexity:* $O( n )$

*/
    CLine& operator=(CLine& cl);
/*
6.4.2 Operation $==$ (~equal~)

*Semantics:* $this ==cl$. It decides whether two line values are identical.

*Complexity:* $O( n )$

*/
    bool operator==(CLine& cl);
/*
6.4.3 Operation $+=$ (~Union~)

*Semantics:* $this += chs$. It adds a half segment into a line value.

*Complexity:* $O( 1 )$ , if the set is not ordered; and $O( log(n)+n)$ , if the set is ordered.

*/
    CLine& operator+=(const CHalfSegment& chs);
/*
6.4.4 Operation $-=$ (~Minus~)

*Semantics:* $this -=chs$. It deletes a half segment from a line value.

*Complexity:* $O( log(n)+n)$.

*/
    CLine& operator-=(const CHalfSegment& chs);
/*
6.4.5 Operation ~Clip~

*Semantics:* This operation receives a rectangle and clips the line to the parts that are
inside the rectangle.

*Complexity:* $O(n)$.

*/
    void Clip( const Rectangle<2>& r, CLine& result );
/*

6.5 Clone Function

*/
    CLine*    Clone();

/*
6.6 Object Traversal Functions

*Semantics:* The following functions are implemented as basic utilities. They will be useful
if we want to adapt our algorithms to object-traversal-based ones.

*Complexity:* All these functions have a complexity of $O( 1 )$ .

*/
    void SelectFirst();
/*
put the pointer ~pos~ to the first half segment in the line value.

*/
    void SelectNext();
/*
move the pointer ~pos~ to the next half segment in the line value.

*/
    bool EndOfHs();
/*
decide whether ~pos~ is -1, which indicates that no more half segments in the line value
 need to be processed.

*/
    void GetHs( CHalfSegment& chs );
/*
get the current half segment from the line value according to the ~pos~ pointer.

*/
    void InsertHs( CHalfSegment& chs );
/*
insert a half segment into the line value, and put the ~pos~ pointer to this newly inserted
half segment.

6.7 Functions needed to import the the ~Line~ data type to tuple

There are totally 10 functions which are defined as virtual functions. They need
to be defined here in order for the Point data type to be used in Tuple definition
as an attribute.

*/
    int NumOfFLOBs();
    FLOB *GetFLOB(const int i);

    bool     IsDefined() const;
    void     SetDefined(bool Defined);
    size_t   HashValue();
    void     CopyFrom(StandardAttribute* right);
    int      Compare(Attribute * arg);
    bool     Adjacent(Attribute * arg);
    int      Sizeof() const;
    //CLine*    Clone() ;
    ostream& Print( ostream &os );
    void     Clear();
  private:
/*
6.7 Private member functions

*/
    void Sort();
/*
Sorts (quick-sort algorithm) the persistent array of half segments in the line value.

*/
    int Position(const CHalfSegment&);
/*
Searches (binary search algorithm) for a half segment in the line value and
returns its position. Returns -1 if the half segment is not found.

6.8 Atrtibutes

*/
    DBArray<CHalfSegment> line;
/*
The persisten array of half segments.

*/
    Rectangle<2> bbox;
/*
The bounding box that fully encloses all half segments of the line.

*/
    int pos;
/*
The pointer to the current half segments. The pointer is important in object traversal algorithms.

*/
    bool ordered;
/*
Whether the half segments in the line value are sorted.

*/
};

/*
6.9 overloaded output operator

*/
ostream& operator<<( ostream& o, CLine& cl );

/*
7 Class Region

This class implements the memory representation of the ~region~ type constructor. A region is
composed of a set of faces. Each face consists of a ouer cycle and a groups of holes.

*/

class CRegion : public StandardSpatialAttribute<2>
{
  public:
/*
7.1 Constructors and Destructor

A ~region~ value is a set of halfsegments. In the external (nestlist) representation, a region value is
expressed as a set of faces, and each face is composed of a set of cycles.  However, in the internal
(class) representation, it is expressed as a set of sorted halfsegments, which are stored as a PArray.

The system will do the basic check on the validity of the region data (see the explaination of the
insertOK() function).

*/
    CRegion() {}
/*
This constructor should not be used.

*/
    CRegion(const int initsize);
    CRegion(CRegion& cr, bool onlyLeft = false);
    void Destroy();
    ~CRegion();

    const Rectangle<2> BoundingBox() const;

/*
7.2 Functions Reading Property Values from an Object

*Semantics:* Reading Property values of a region value.

*Complexity:* $O( 1)$

*/
    bool IsOrdered() const;
/*
This function decides whether the half segments in the region value is sorted.

*/
    bool IsEmpty() const;
/*
This function decides whether the region value is empty.

*/
    int Size() const;
/*
This function returns the number of half segments in the region value.

*/
    void Get( const int i, CHalfSegment& chs );
/*
This function reads the ith half segment from the region value.

7.3 Bulkload Functions

*/
    void StartBulkLoad();
/*
Marks the begin of a bulk load of line relaxing the condition that the half segments must be
ordered.

*/
    void EndBulkLoad();
/*
Marks the end of a bulk load and sorts the half segments.

7.4 Validity Checking Function

*/
    bool insertOK(const CHalfSegment& chs);
/*
This function check whether a region value is valid after the insertion of a new half segment.
Whenever a half segment is about to be inserted, the state of the region is checked.
A valid region must satisfy the following conditions:

1)  any two cycles of the same region must be disconnect, which means that no edges
of different cycles can intersect each other;

2) edges of the same cycle can only intersect with their endpoints, but no their middle points;

3)  For a certain face, the holes must be inside the outer cycle;

4)  For a certain face, any two holes can not contain each other;

5)  Faces must have the outer cycle, but they can have no holes;

6)  for a certain cycle, any two vertex can not be the same;

7)  any cycle must be made up of at least 3 edges;

8)  It is allowed that one face is inside another provided that their edges do not intersect.


7.5 Overloaded Class Operators

7.5.1 Operation $=$ (~assignment~)

*Semantics:* $this = cr$. It assigns the value of a region to another.

*Complexity:* $O( n )$

*/
    CRegion& operator=(CRegion& cr);
/*
7.5.2 Operation $==$ (~equal~)

*Semantics:* $this ==cr$. It decides whether two region values are identical.

*Complexity:* $O( n )$

*/
    bool operator==(CRegion& cr);
/*
7.5.3 Operation $+=$ (~Union~)

*Semantics:* $this +=chs$. It adds a half segment into a region value.

*Complexity:* $O( 1 )$ , if the set is not ordered; and $O( log(n)+n)$ , if the set is ordered.

*/
    CRegion& operator+=(const CHalfSegment& chs);
/*
7.5.4 Operation $-=$ (~Minus~)

*Semantics:* $this -=chs$. It delete a half segment from a region value.

*Complexity:* $O( log(n)+n)$ .

*/
    CRegion& operator-=(const CHalfSegment& chs);
/*

7.6 Clone Function

*/
    CRegion*    Clone();

/*
7.7 Object Traversal Functions

*Semantics:* The following functions are implemented as basic utilities. They will be useful
if we want to adapt our algorithms to object-traversal-based ones.

*Complexity:* All the following functions have a complexity of $O( 1)$ .

*/
    void SelectFirst();
/*
Put the pointer ~pos~ to the first half segment in the region value.

*/
    void SelectNext();
/*
Move the pointer "pos" to the next half segment in the region value.

*/
    bool EndOfHs();
/*
decide whether ~pos~ is -1, which indicates that no more half segments in the region value
 need to be processed.

*/
    void GetHs( CHalfSegment& chs );
/*
get the current half segment from the region value according to the ~pos~ pointer.

*/
    void InsertHs( CHalfSegment& chs );
/*
Insert a half segment into the region value, and put the ~pos~ pointer to this newly inserted
half segment.

*/
    const AttrType& GetAttr();
/*
read the ~attr~ value of the current half segment from the region value. The current
half segment is indicated by ~pos~

*/
    void UpdateAttr( AttrType& attr );
/*
update the ~attr~ value of the current half segment from the region value.The current
half segment is indicated by ~pos~

7.8 contain function (point)

*Semantics:* This function decides whether a point is inside the region.

*Complexity:* $O( n )$  where ~n~ is the number of segments of the region.

*/
    bool contain_old( const Point& p );
    bool contain( const Point& p );
    bool containpr( const Point& p, int &pathlength, int & scanned );
/*
7.9 innercontain function

*Semantics:* This function decides whether a point is inside the inner part of the region.

*Complexity:* $O( n )$  where ~n~ is the number of segments of the region.

*/
    bool innercontain( const Point& p );
/*
7.10 contain function (segment)

*Semantics:* This function decides whether a half segment is inside the region.

*Complexity:* $O( n )$  where ~n~ is the number of segments of the region.

*/
    bool contain( const CHalfSegment& chs );
/*
7.11 holeedge-contain function

*Semantics:* This function decides whether a half segment is inside a hole edge of the region.

*Complexity:* $O( n )$  where ~n~ is the number of segments of the region.

*/
    bool holeedgecontain( const CHalfSegment& chs );
/*
The following two functions are used to sort the half segments according to their attributes;

*/
    void logicsort();

    void setOrdered(bool isordered);
/*
7.12 Functions needed to import the the ~Region~ data type to tuple

There are totally 10 functions which are defined as virtual functions. They need
to be defined here in order for the Point data type to be used in Tuple definition
as an attribute.

*/
    int NumOfFLOBs();
    FLOB *GetFLOB(const int i);

    bool     IsDefined() const;
    void     SetDefined(bool Defined);
    size_t   HashValue();
    void     CopyFrom(StandardAttribute* right);
    int      Compare(Attribute * arg);
    bool     Adjacent(Attribute * arg);
    int      Sizeof() const;
    //CRegion*    Clone() ;
    ostream& Print( ostream &os );
    void     Clear();

  private:
/*
7.13 Private member functions

*/
    void Sort();
/*
sorts (quick-sort algorithm) the persistent array of half segments in the region value.

*/
    int Position(const CHalfSegment&);
    int Position(const Point&);
/*
searches (binary search algorithm) for a half segment in the region value and
returns its position. Returns -1 if the half segment is not found.

7.14 Atrtibutes

*/
    DBArray<CHalfSegment> region;

    Rectangle<2> bbox;

/*
The persisten array of half segments.

*/
    int pos;
/*
The pointer to the current half segments. The pointer is important in object traversal algorithms.

*/
    bool ordered;
/*
Whether the half segments in the region value are sorted.

*/

};

/*
7.15 overloaded output operator

*/
ostream& operator<<( ostream& o, CRegion& cr );

Word InPoint( const ListExpr typeInfo, const ListExpr instance, const int errorPos, ListExpr& errorInfo, bool& correct );
ListExpr OutPoint( ListExpr typeInfo, Word value );

#endif // __SPATIAL_ALGEBRA_H__


/*
8 Operations of the Spatial Algebra

See ~Operations to be Implemented in the Spatial Algebra~ for the discription of operations of the spatial algebra.

*/
