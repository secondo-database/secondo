/*
//paragraph [1] Title: [{\Large \bf \begin {center}] [\end {center}}]

[1] Header File of the Spatial Algebra

February, 2002 Victor Teixeira de Almeida

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

#include "Rational.h"
#include "PArray.h"

#define RATIONAL_COORDINATES
#ifdef RATIONAL_COORDINATES
#ifdef WITH_ARBITRARY_PRECISION
typedef Rational Coord;
#else // WITH_ARBITRARY_PRECISION
typedef Rational Coord;
#endif
#else // RATIONAL_COORDINATES
typedef long Coord;
#endif
/*
There are two main defines that will control how the coordinate system is 
implemented, namely ~RATIONAL\_COORDINATES~ and ~WITH\_ARBITRARY\_PRECISION~.
The first one defines if the system will use rational coordinates instead 
of simple integer (~long~) ones. The second one is used only if rational 
coordinates are being used and defines if the system will use rational 
numbers with arbitrary precision (~not implemented yet~) instead of fixed 
precision (~Rational~).

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

class Point
{
  public:
/*
3.1. Constructors and Destructor

There are two ways of constructing a point:

*/
    Point( const bool d = false, const Coord& x = 0L, const Coord& y = 0L );
/*
The first one receives a boolean value ~d~ indicating if the point is defined
and two coordinate ~x~ and ~y~ values. Note that this constructor can be
called without arguments because all of them have default values.

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
    const bool IsDefined() const;
/*
Returns if the point is defined.

*/
    const Coord& GetX() const;
/*
Returns the ~x~ coordinate of the point.

*Precondition:* ~IsDefined()~

*/
    const Coord& GetY() const;
/*
Returns the ~y~ coordinate of the point.

*Precondition:* ~IsDefined()~

*/
    Point& operator=(const Point& p);
/*
Assignement operator redefinition.

*Precondition:* ~p.IsDefined()~

*/
    void SetDefined( const bool d = true );
/*
Sets the point to defined or undefined depending on the value of ~d~.

3.3 Operations

3.3.1 Operation $=$ (~equal~) 

*Precondition:* ~u.IsDefined() $\&\&$ v.IsDefined()~

*Semantics:* $u = v$

*Complexity:* $O(1)$

*/
    int operator==(const Point& p) const;
/*
3.3.2 Operation $\neq$ (~not equal~)

*Precondition:* ~u.IsDefined() $\&\&$ v.IsDefined()~

*Semantics:* $u \neq v$

*Complexity:* $O(1)$

*/
    int operator!=(const Point& p) const;
/*
3.3.3 Operation $\leq$ (~less or equal~)

*Precondition:* ~u.IsDefined() $\&\&$ v.IsDefined()~

*Semantics:* $u \leq v$

*Complexity:* $O(1)$

*/
    int operator<=(const Point& p) const;
/*
3.3.4 Operation $<$ (~less than~)

*Precondition:* ~u.IsDefined() $\&\&$ v.IsDefined()~

*Semantics:* $u < v$

*Complexity:* $O(1)$

*/
    int operator<(const Point& p) const;
/*
3.3.5 Operation $\geq$ (~greater or equal~)

*Precondition:* ~u.IsDefined() $\&\&$ v.IsDefined()~

*Semantics:* $u \geq v$

*Complexity:* $O(1)$

*/
    int operator>=(const Point& p) const;
/*
3.3.6 Operation $>$ (~greater than~)

*Precondition:* ~u.IsDefined() $\&\&$ v.IsDefined()~

*Semantics:* $u > v$

*Complexity:* $O(1)$

*/
    int operator>(const Point& p) const;
/*
3.3.7 Operation ~inside~ 

*Precondition:* ~u.IsDefined()~

*Semantics:* $u \in V$

*Complexity:* $O(log(n))$, where ~n~ is the size of the point set ~V~

*/
    const bool Inside( const Points& ps ) const;
/*
3.3.8 Operation ~intersection~ (with ~point~)

*Precondition:* ~u.IsDefined() $\&\&$ v.IsDefined()~

*Semantics:* if $u = v$ then $u$ else $\perp$

*Complexity:* $O(1)$

*/
    void Intersection( const Point& p, Point& result ) const;
/*
3.3.9 Operation ~intersection~ (with ~points~)

*Precondition:* ~u.IsDefined()~

*Semantics:* if $u \in V$ then $u$ else $\perp$

*Complexity:* $O(log(n))$, where ~n~ is the size of the point set ~V~

*/
    void Intersection( const Points& ps, Point& result ) const;
/*
3.3.10 Operation ~minus~ (with ~point~)

*Precondition:* ~u.IsDefined() $\&\&$ v.IsDefined()~

*Semantics:* if $u = v$ then $\perp$ else $u$

*Complexity:* $O(1)$

*/
    void Minus( const Point& p, Point& result ) const;
/*
3.3.9 Operation ~minus~ (with ~points~)

*Precondition:* ~u.IsDefined()~

*Semantics:* if $u \in V$ then $\perp$ else $u$

*Complexity:* $O(log(n))$, where ~n~ is the size of the point set ~V~

*/
    void Minus( const Points& ps, Point& result ) const;

  private:
/*
3.4 Attributes

*/
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

/*
4 Class Points

This class implements the memory representation of the ~points~ type constructor.
A points value is a finite set of points. An example of a points value can be seen
in the Figure \cite{fig:spatialdatatypes.eps}.

The implementation of the points type constructor is a persistent array of points
ordered by lexicographic order.

*/
class Points
{
  public:
/*
4.1 Constructors and Destructor

There are three ways of constructing a point set:

*/
    Points();
/*
The first one receives no arguments and constructs an empty point set.

*/
    Points( const Points& ps );
/*
The second one receives another point set ~ps~ as argument and constructs a point
set which is a copy of ~ps~.

*/
    Points( const SmiRecordId recordId, bool update = true );
/* 
The third and the last one receives a ~recordId~ and a flag ~update~ as arguments.
This constructor is applied not to create a new point set, but to read it from
the disk. The ~recordId~ gives the position of the point set in the persistent
array (~PArray~) structure and the flag ~update~ is used to open the array for 
update or read-only.

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
    const bool IsOrdered() const;
/*
Returns if the point se is ordered. There is a flag ~ordered~ (see attributes) in order
to avoid a scan in the point set to answer this question.

*/
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
    const bool IsEmpty() const;
/*
Returns if the set is empty of not.

*/
    const int Size() const;
/*
Returns the size of the point set. Returns ~0~ if the set is empty.

*/
    void Get( const int i, Point& p ) const;
/*
Retrieves the point ~p~ at position ~i~ in the point set.

*Precondition:* $0 \leq i < Size()$

*/
    const SmiRecordId GetPointsRecordId() const;
/*
Returns the record identification of the points persistent array.

*/
    Points& operator=(const Points& ps);
/*
Assignement operator redefinition.

*Precondition:* ~ps.IsOrdered()~

*/
    const bool Contains(const Point& p) const;
/*
Searches (binary search algorithm) for a point in the point set and
return ~true~ if found and ~false~ if not.

*Precondition:* ~this.IsOrdered() $\&\&$ p.IsDefined()~

*/
    const bool Contains(const Points& ps) const;
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
    int operator==(const Points&) const;
/*
4.4.2 Operation $\neq$ (~not equal~)

*Precondition:* ~this.IsOrdered() $\&\&$ ps.IsOrdered()~

*Semantics:* $this = ps$

*Complexity:* $O(n+m)$, where ~n~ is the size of this point set and m the size of ~ps~.

*/
    int operator!=(const Points&) const;
/*
4.4.2 Operation ~union~ (with ~point~)

*Precondition:* ~p.IsDefined()~

*Semantics:* $this \cup \{p\}$

*Complexity:* $O(1)$, if the set is not ordered, and $O(log(n)+n)$, otherwise. ~n~ is the size 
of this point set.

*/
    Points& operator+=(const Point& p);
/*
4.4.3 Operation ~union~ (with ~points~)

*Precondition:* 

*Semantics:* $this \cup ps$

*Complexity:* $O(m)$, if the set is not ordered, and $O(m+(m+n)log(m+n))$, otherwise. ~n~ is the size 
of this point set and ~m~ is the size of ~ps~.

*/
    Points& operator+=(const Points& ps);
/*
4.4.4 Operation ~minus~ (with ~point~)

*Precondition:* ~p.IsDefined() $\&\&$ this.IsOrdered()~

*Semantics:* $this \backslash \{p\}$

*Complexity:* $O(log(n)+n)$

*/
    Points& operator-=(const Point& p);
/*
4.4.5 Operation ~inside~

*Precondition:* ~this.IsOrdered() $\&\&$ ps.IsOrdered()~

*Semantics:* $this \subseteq ps$

*Complexity:* $O(n+m)$, where ~n~ is the size of this point set and m the size of ~ps~.

*/
    const bool Inside(const Points& ps) const;
/*
4.4.6 Operation ~intersects~

*Precondition:* ~this.IsOrdered() $\&\&$ ps.IsOrdered()~

*Semantics:* $this \cap ps$

*Complexity:* $O(n+m)$, where ~n~ is the size of this point set and m the size of ~ps~.

*/
    const bool Intersects(const Points& ps) const;

  private:
/*
4.5 Private member functions

*/
    void Sort();
    void QuickSortRecursive( const int low, const int high );
/*
Sorts (quick-sort algorithm) the persistent array of points.

*/
    const int Position(const Point&) const;
/*
Searches (binary search algorithm) for a point in the point set and
returns its position. Returns -1 if the point is not found.

*/

/*
4.6 Atrtibutes

*/
    PArray<Point>* points;
/*
The persisten array of points.

*/
    bool ordered;
/*
The flag that indicates whether the persistent array is in ordered state.

*/
};

/*
4.7 Auxiliary functions

*/
ostream& operator<<( ostream& o, const Point& p );
/*
Print the point ~p~ in the out stream ~o~.

*/
ostream& operator<<( ostream& o, const Points& ps );
/*
Print the point set ~ps~ int the out stream ~o~.

*/


#endif // __SPATIAL_ALGEBRA_H__
