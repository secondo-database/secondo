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

#define PI 3.146264371

enum WindowEdge { WTOP, WBOTTOM, WLEFT, WRIGHT };

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
const double FACTOR = 0.000001;

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
    inline Point() {};
/*
This constructor should not be used.

There are two ways of constructing a point:

*/
    inline Point( const bool d, 
                  const Coord& x = Coord(), 
                  const Coord& y = Coord() ):
    defined( d ),
    x( x ),
    y( y )
    {}
/*
The first one receives a boolean value ~d~ indicating if the point is defined
and two coordinate ~x~ and ~y~ values.

*/
    inline Point( const Point& p ):
    defined( p.IsDefined() )
    {
      if( defined )
      {
        x = p.x;
        y = p.y;
      }
    }
/*
The second one receives a point ~p~ as argument and creates a point that is a
copy of ~p~.

*/
    inline ~Point() 
    {}
/*
The destructor.

3.2 Member functions

*/
    inline const Coord& GetX() const
    {
      return x;
    }
/*
Returns the ~x~ coordinate of the point.

*Precondition:* ~IsDefined()~

*/
    inline const Coord& GetY() const
    {
      return y;
    }
/*
Returns the ~y~ coordinate of the point.

*Precondition:* ~IsDefined()~

*/
    inline const Rectangle<2> BoundingBox() const
    {
      if( defined )
        return Rectangle<2>( true, this->x, this->x, this->y, this->y );
      return Rectangle<2>( false );
    }
    
/*
Returns the point bounding box which is also a point.

*/
    inline void Set( const Coord& x, const Coord& y )
    {
      defined = true;
      this->x = x;
      this->y = y;
    }
/*
Sets the value of the point object.

*/
    inline void Translate( const Coord& x, const Coord& y )
    {
      assert( defined );
      this->x += x;
      this->y += y;
    }
/*
Translates a point position by adding values ~x~ and ~y~.

*/
    inline Point& operator=(const Point& p)
    {
      defined = p.defined;
      if( defined )
      {
        x = p.x;
        y = p.y;
      }
      return *this;
    }
/*
Assignement operator redefinition.

*/
    inline void Scale(const Coord& factor)
    {
      assert( defined );
      this->x *= factor;
      this->y *= factor;
    }
/*
Scales a point position by multipliing with ~factor~

3.3 Operations

3.3.1 Operation $=$ (~equal~)

*Precondition:* ~u.IsDefined() $\&\&$ v.IsDefined()~

*Semantics:* $u = v$

*Complexity:* $O(1)$

*/
    inline bool operator==( const Point& p ) const
    {
      return x == p.x && y == p.y;
    }
/*
3.3.2 Operation $\neq$ (~not equal~)

*Precondition:* ~u.IsDefined() $\&\&$ v.IsDefined()~

*Semantics:* $u \neq v$

*Complexity:* $O(1)$

*/
    inline bool operator!=( const Point& p ) const
    {
      return x != p.x || y != p.y;
    }
/*
3.3.3 Operation $\leq$ (~less or equal~)

*Precondition:* ~u.IsDefined() $\&\&$ v.IsDefined()~

*Semantics:* $u \leq v$

*Complexity:* $O(1)$

*/
    inline bool operator<=( const Point& p ) const
    {
      if( x < p.x )
        return 1;
      else if( x == p.x && y <= p.y )
        return 1;
      return 0;
    }
/*
3.3.4 Operation $<$ (~less than~)

*Precondition:* ~u.IsDefined() $\&\&$ v.IsDefined()~

*Semantics:* $u < v$

*Complexity:* $O(1)$

*/
    inline bool operator<( const Point& p ) const
    {
      if( x < p.x )
        return 1;
      else if( x == p.x && y < p.y )
        return 1;
      return 0;
    }
/*
3.3.5 Operation $\geq$ (~greater or equal~)

*Precondition:* ~u.IsDefined() $\&\&$ v.IsDefined()~

*Semantics:* $u \geq v$

*Complexity:* $O(1)$

*/
    inline bool operator>=( const Point& p ) const
    {
      if( x > p.x )
        return 1;
      else if( x == p.x && y >= p.y )
        return 1;
      return 0;
    }
/*
3.3.6 Operation $>$ (~greater than~)

*Precondition:* ~u.IsDefined() $\&\&$ v.IsDefined()~

*Semantics:* $u > v$

*Complexity:* $O(1)$

*/
    inline bool operator>( const Point& p ) const
    {
      if( x > p.x )
        return 1;
      else if( x == p.x && y > p.y )
        return 1;
      return 0;
    }
/*
3.3.7 Operation ~inside~ (with ~points~)

*Precondition:* ~u.IsDefined() $\&\&$ V.IsOrdered()~

*Semantics:* $u \in V$

*Complexity:* $O(log(n))$, where ~n~ is the size of the point set ~V~

*/
    bool Inside( const Points& ps ) const;
/*
3.3.8 Operation ~inside~ (with ~rectangle~)

*Precondition:* ~u.IsDefined()~

*Semantics:* $u \in V$

*Complexity:* $O(1)$

*/
    inline bool Inside( const Rectangle<2>& r ) const
    {
      if( x < r.MinD(0) || x > r.MaxD(0) )
        return false;
      else if( y < r.MinD(1) || y > r.MaxD(1) )
        return false;
      return true;
    }
/*
3.3.8 Operation ~intersection~ (with ~point~)

*Precondition:* ~u.IsDefined() $\&\&$ v.IsDefined()~

*Semantics:* if $u = v$ then $u$ else $\perp$

*Complexity:* $O(1)$

*/
    inline void Intersection( const Point& p, Point& result ) const
    {
      if( *this == p )
        result = *this;
      else
        result.SetDefined( false );
    }
/*
3.3.9 Operation ~intersection~ (with ~points~)

*Precondition:* ~u.IsDefined() $\&\&$ V.IsOrdered()~

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
    inline void Minus( const Point& p, Point& result ) const
    {
      if( *this == p )
        result.SetDefined( false );
      else
        result = *this;
    }
/*
3.3.9 Operation ~minus~ (with ~points~)

*Precondition:* ~u.IsDefined() $\&\&$ V.IsOrdered()~

*Semantics:* if $u \in V$ then $\perp$ else $u$

*Complexity:* $O(log(n))$, where ~n~ is the size of the point set ~V~

*/
    void Minus( const Points& ps, Point& result ) const;
/*
3.3.10 Operation ~distance~

*Precondition:* ~u.IsDefined()~ and ~v.IsDefined()~

*Semantics:* compute the distance between u and v

*Complexity:* $O(1)$

*/
    inline double Distance( const Point& p ) const
    {

#ifdef RATIONAL_COORDINATES
      double dx = (p.x.IsInteger()? p.x.IntValue():p.x.Value()) -
                  (x.IsInteger()? x.IntValue():x.Value());
      double dy = (p.y.IsInteger()? p.y.IntValue():p.y.Value()) -
                  (y.IsInteger()? y.IntValue():y.Value());
#else
      double dx = p.x - x;
      double dy = p.y - y;
#endif

      return sqrt( pow( dx, 2 ) + pow( dy, 2 ) );
    }
/*
3.3.11 Functions needed to import the the ~Point~ data type to tuple

There are totally 10 functions which are defined as virtual functions. They need
to be defined here in order for the Point data type to be used in Tuple definition
as an attribute.

*/
    inline bool IsDefined() const
    {
      return defined;
    }

    inline void SetDefined(bool defined)
    {
      this->defined = defined;
    }

    inline size_t HashValue() const
    {
      if( !defined )
        return 0;

      size_t h;
#ifdef RATIONAL_COORDINATES
      h=(size_t)
          (5*(x.IsInteger()? x.IntValue():x.Value())
            + (y.IsInteger()? y.IntValue():y.Value()));
#else
      h=(size_t)(5*x + y);
#endif
      return h;
    }

    inline void CopyFrom(const StandardAttribute* right)
    {
      const Point* p = (const Point*)right;
      defined = p->defined;
      if( defined )
        Set( p->x, p->y );
    }
    
    inline int Compare(const Attribute * arg) const
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

    inline bool Adjacent(const Attribute * arg) const
    {
      return false;
    }

    inline Point* Clone() const
    {
      return new Point( *this );
    }

    ostream& Print( ostream &os ) const;

/*
3.4 Attributes

*/
  protected:

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
    inline Points() {}
/*
This constructor should not be used.

*/
    inline Points( const int initsize ):
    points( initsize ),
    bbox( false ),
    ordered( true )
    {}
/*
The first one constructs an empty point set but open space for ~initsize~ points.

*/
    inline Points( const Points& ps):
    points( ps.Size() ),
    bbox( ps.BoundingBox() ),
    ordered( true )
    {
      for( int i = 0; i < ps.Size(); i++ )
      {
        const Point *p;
        ps.Get( i, p );
        points.Put( i, *p );
      }
    }
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

4.2 Functions for Bulk Load of Points

As said before, the point set is implemented as an ordered persistent array of points.
The time complexity of an insertion operation in an ordered array is $O(n)$, where ~n~
is the size of the point set. In some cases, bulk load of points for example, it is good
to relax the ordered condition to improve the performance. We have relaxed this ordered
condition only for bulk load of points. All other operations assume that the point set is
ordered.

*/
    inline bool IsOrdered() const
    {
      return ordered;
    }
/*
Returns whether the point set is ordered. There is a flag ~ordered~ (see attributes) in order
to avoid a scan in the point set to answer this question.

*/
    void StartBulkLoad();
/*
Marks the begin of a bulk load of points relaxing the condition that the points must be
ordered.

*/
    void EndBulkLoad( const bool sort = true );
/*
Marks the end of a bulk load and sorts the point set.

4.3 Member functions

*/
    inline const Rectangle<2> BoundingBox() const
    {
      if( !IsEmpty() )
        return bbox;
      return Rectangle<2>( false );
    }
/*
Returns the bounding box that spatially contains all points.

*/
    inline bool IsEmpty() const
    {
      return Size() == 0;
    }
/*
Returns if the set is empty of not.

*/
    inline int Size() const
    {
      return points.Size();
    }
/*
Returns the size of the point set. Returns ~0~ if the set is empty.

*/
    inline void Get( const int i, Point const*& p ) const
    {
       points.Get( i, p );
    }
/*
Retrieves the point ~p~ at position ~i~ in the point set.

*Precondition:* $0 \leq i < Size()$

*/

    Points& operator=(const Points& ps);
/*
Assignement operator redefinition.

*Precondition:* ~ps.IsOrdered()~

*/
    bool Contains(const Point& p) const;
/*
Searches (binary search algorithm) for a point in the point set and
return ~true~ if found and ~false~ if not.

*Precondition:* ~this.IsOrdered() $\&\&$ p.IsDefined()~

*/
    bool Contains(const Points& ps) const;


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
    bool operator==(const Points&) const;
/*
4.4.2 Operation $\neq$ (~not equal~)

*Precondition:* ~this.IsOrdered() $\&\&$ ps.IsOrdered()~

*Semantics:* $this = ps$

*Complexity:* $O(n+m)$, where ~n~ is the size of this point set and m the size of ~ps~.

*/
    bool operator!=(const Points&) const;
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
    Points& operator+=(const Points& ps);
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
    bool Inside(const Points& ps) const;
/*
4.4.7 Operation ~intersects~

*Precondition:* ~this.IsOrdered() $\&\&$ ps.IsOrdered()~

*Semantics:* $this \cap ps$

*Complexity:* $O(n+m)$, where ~n~ is the size of this point set and m the size of ~ps~.

*/
    bool Intersects(const Points& ps) const;
/*
4.4.8 Object Traversal Functions

*Precondition:* ~this.IsOrdered()~

*Semantics:*  These functions are object traversal functions which is useful when we are
using ROSE algebra algorithms (DZM).

*Complexity:* $O(1)$.

*/
    inline void SelectFirst() const
    {
      if (IsEmpty()) pos=-1;
      else pos=0;
    }

    inline void SelectNext() const
    {
      if ((pos>=0) && (pos<Size()-1)) pos++;
      else pos=-1;
    }

    inline bool EndOfPt() const
    {
      return pos == -1;
    }

    inline bool GetPt( const Point*& p ) const
    {
      if( pos>=0 && pos <= Size()-1 )
      {
        points.Get( pos, p);
        return true;
      }
      return false;
    }

    void InsertPt( const Point& p );

    inline void Clear()
    {
      points.Clear();
      pos=-1;
      ordered=true;
      bbox.SetDefined(false);
    }
/*
4.6 Functions needed to import the the ~Points~ data type to tuple

There are totally 10 functions which are defined as virtual functions. They need
to be defined here in order for the Point data type to be used in Tuple definition
as an attribute.

*/
    inline int NumOfFLOBs() const
    {
      return 1;
    }

    inline FLOB *GetFLOB(const int i)
    {
      return &points;
    }

    inline bool IsDefined() const
    {
      return true;
    }

    inline void SetDefined(bool Defined)
    {
    }

    inline bool Adjacent(const Attribute * arg) const
    {
      return false;
    }

    inline Points* Clone() const
    {
      return new Points( *this );
    }

    size_t HashValue() const;
    void CopyFrom(const StandardAttribute* right);
    int Compare(const Attribute * arg) const;
    ostream& Print( ostream &os ) const;

  private:
/*
4.5 Private member functions

*/
    void Sort();
/*
Sorts the persistent array of points.

*/
    void RemoveDuplicates();
/*
Removes all Duplicates from this points value.
The points must be sorted.

*/


    int Position(const Point&) const;
/*
Searches (binary search algorithm) for a point in the point set and
returns its position. Returns -1 if the point is not found.

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
    mutable int pos;
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
ostream& operator<<( ostream& o, const Points& ps );
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
  inline AttrType() {}

  inline AttrType& operator=( const AttrType& at )
  {
    insideAbove = at.insideAbove;
    faceno = at.faceno;
    cycleno = at.cycleno;
    edgeno = at.edgeno;
    coverageno = at.coverageno;
    partnerno = at.partnerno;
    return *this;
  }

  inline bool Valid() const
  {
    return faceno >= 0 &&
           cycleno >= 0 &&
           edgeno >= 0 &&
           partnerno >= 0;
  }

  int faceno;
  int cycleno;
  int edgeno;
  int coverageno;  //this number is used for the fast spacial 
                   //scan of the inside_pr algorithm
  int partnerno; //store the position of the partner half 
                 //segment in half segment ordered array
  bool insideAbove; //indicates if the region's area 
                    //is above or left of its segment
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
    inline CHalfSegment() {}
/*
This constructor should not be used.

*/
    inline CHalfSegment( bool Defined, bool LDP = false, 
                         const Point& P1 = Point( false ), 
                         const Point& P2 = Point( false ) )
    {
       defined = Defined;
       if( defined )
       {
         ldp = LDP;
         if (P1<P2)
         {
           lp = P1;
           rp = P2;
         }
         else if (P1>P2)
         {
           lp = P2; 
           rp = P1;
         }
         else defined = false;
       }
       else
         ldp = false;
    }

    inline CHalfSegment( const CHalfSegment& chs )
    {
       defined = chs.IsDefined();
       if( defined )
       {
         ldp = chs.GetLDP();
         lp = chs.GetLP();
         rp = chs.GetRP();
         attr=chs.GetAttr();
       } 
    }

    inline ~CHalfSegment()
    {}
/*
5.2 Functions Reading Property Values from a CHalfSegment Object

*Semantics:* Reading Property values of a half segment.

*Complexity:* $O( 1)$

*/
    inline bool IsDefined() const
    {
      return defined;
    }
/*
This function returns a boolean value indicating whether the half segment is defined.

*/
    inline bool Valid() const
    {
      return attr.Valid() &&  
             (!defined || (defined && lp < rp));
    }
/*
Checks whether the segment is valid. Used for debugging purposes.

*/
    inline const Point& GetLP() const
    {
      assert( defined );
      return lp;
    }
/*
This function returns the left point of the half segment.

*/
    inline const Point& GetRP() const
    {
      assert( defined );
      return rp;
    }
/*
This function returns the right point of the half segment.

*/
    inline const Point& GetDPoint() const
    {
      assert( defined );
      return (ldp ? lp : rp);
    }
/*
This function returns the dominating point of the half segment.

*/
    inline const Point& GetSPoint() const
    {
      assert( defined );
      return (ldp ? rp : lp);
    }
/*
This function returns the secondary point of the half segment.

*/
    inline bool GetLDP() const
    {
      assert( defined );
      return ldp;
    }
/*
This function returns the boolean flag which indicates whether the dominating point is on the
left side.

*/
    inline const Rectangle<2> BoundingBox() const
    {
      assert( defined );
      return Rectangle<2>( true,
                           MIN( GetLP().GetX(), GetRP().GetX() ),
                           MAX( GetLP().GetX(), GetRP().GetX() ),
                           MIN( GetLP().GetY(), GetRP().GetY() ),
                           MAX( GetLP().GetY(), GetRP().GetY() ) );
    }
/*
Returns the bounding box of the half segment.

*/
    inline const AttrType& GetAttr() const
    {
      return attr;
    }
/*
This function returns the "attribute" value associated with a half segment. The "attribute" argument is
useful when we process region values.

5.3 Functions Setting Property Values of a CHalfSegment Object

*Semantics:* Writing Property values of a half segment.

*Complexity:* $O( 1)$

*/
    inline void Set(bool LDP, const Point& P1, const Point& P2)
    {
      defined = true;
      ldp = LDP;
      if (P1<P2)
      {
        lp = P1;
        rp = P2;
      }
      else if (P1>P2)
      {
        lp = P2;
        rp = P1;
      }
      else defined = false;
    }
/*
This function sets the value of a half segment. The parameter LP and RP can ignore the order, and the
function will compare the parameter points and put the smaller one to LP and larger one to RP.

*/
    inline void Translate( const Coord& x, const Coord& y )
    {
      assert( defined );
      lp.Translate( x, y );
      rp.Translate( x, y );
    }
/*
Translates the half segment.

*/
   
    inline void Scale(const Coord& factor)
    {
      assert(defined);
      lp.Scale(factor);
      rp.Scale(factor);
    }
/*
Scales the half segment given a ~factor~.

*/

    inline void SetDefined(bool Defined)
    {
      defined = Defined;
    }
/*
This function sets the value of the "defined" argument of a half segment.

*/
    inline void SetAttr(AttrType& ATTR)
    {
      attr = ATTR;
    }
/*
This function sets the value of the "attr" argument of a half segment.

*/
    inline void SetLDP(bool LDP)
    {
      ldp = LDP;
    }
/*
This function sets the value of the "Left Dominating Point" flag of a half segment.

5.4 Overloaded class operators

5.4.1 Operation $=$ (~assignment~)

*Semantics:* $this = chs$. It assigns the value of a half segment to another.

*Complexity:* $O( 1)$

*/
    inline CHalfSegment& operator=(const CHalfSegment& chs)
    {
      assert( chs.IsDefined() );
      defined = true;
      ldp = chs.GetLDP();
      lp = chs.GetLP();
      rp = chs.GetRP();
      attr = chs.GetAttr();
      return *this;
    }
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
    inline CHalfSegment*  Clone() 
    {
      return new CHalfSegment(*this);
    }

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
    bool overlapintersect( const CHalfSegment& chs, 
                           CHalfSegment& reschs ) const;
/*
This function compute whether two half segments intersect each other with a segment, if yes,
the intersection segment is returned.

*/
    bool innerIntersects( const CHalfSegment& chs) const;
/*
This function decides whether two half segments intersect in the following manner: a point of
the first segment and a innerpoint of the second segment is the same.

*/
    bool innerInter( const CHalfSegment& chs,  Point& resp,
                     CHalfSegment& rchs, bool& first, bool& second ) const;
/*

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
5.7 Clipping functions

5.7.1 Clipping window

The CohenSutherlandLineClipping function implements the Cohen and Sutherland algorithm
for clipping a line to a clipping window.

The WindowClippingIn computes the part of the segment that is inside the window, if
it exists. The inside parameter is set to true if there is a partion of the line
inside the window. The pointIntersection parameter is set to true if the intersection
part of the segment is a point instead of a segment.

*/
   void CohenSutherlandLineClipping(const Rectangle<2> &window,
                            double &x0, double &y0, double &x1, double &y1,
                            bool &accept) const;

   void WindowClippingIn(const Rectangle<2> &window,
                         CHalfSegment &chsInside, 
                         bool &inside,
                         bool &isIntersectionPoint,
                         Point &intersectionPoint) const;

/*
Returns a segment representing the part of the half segment that is inside the window.
*/

/*
5.8 Inside Function

*Semantics:* This operation computes whether one half segment is inside another. If segment A is part of
another segment B, then we say A is inside B. eg. -------======-------.

*Complexity:* $O( 1 )$

*/
    bool Inside(const CHalfSegment& chs) const ;
/*
5.9 Contain Function

*Semantics:* This operation computes whether one point is contained in a half segment. If a point P is inside
a segment S, then we say P is contained by S. eg. ---------o---------.

*Complexity:* $O( 1 )$

*/
    bool Contains( const Point& p ) const;
/*
5.10 rayAbove Function

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
5.12 attribute comparison Functions

*Semantics:* These two operations compare two half segments according to their attribute values. They are
used for the logicsort() function.

*Complexity:* $O( 1 )$

*/
    int logicless(const CHalfSegment& chs) const;
    int logicgreater(const CHalfSegment& chs) const;
    bool LogicEqual(const CHalfSegment& chs) const;

  private:
/*
5.13 Properties

*/
    Point lp;
    Point rp;
/*
These two properties give the left and right point of the half segment.

*/
    bool defined;
/*
This boolean property indicates whether the half segment is defined.

*/
    bool ldp;
/*
This boolean property indicates whether the half segment has its left point as its dominating point.

*/
  public:
    AttrType attr;
/*
This ~attribute~ property is useful if we process region values in the way indicated in the ROSE
paper.

*/
};
/*
5.14 overloaded output operator

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
    inline CLine() {}
/*
This constructor should not be used.

*/
    inline CLine(const int initsize) :
    line( initsize ),
    bbox( false ),
    ordered( true )
    {}

    inline CLine(const CLine& cl ):
    line( cl.Size() ),
    bbox( cl.bbox ),
    ordered( true )
    {
      assert( cl.IsOrdered());
      for( int i = 0; i < cl.Size(); i++ )
      {
        const CHalfSegment *chs;
        cl.Get( i, chs );
        line.Put( i, *chs );
      }
    }

    inline void Destroy()
    {
      line.Destroy();
    }

    inline ~CLine()
    {}

    inline const Rectangle<2> BoundingBox() const
    {
      if( !IsEmpty() )
        return bbox;
      return Rectangle<2>( false );
    }

/*
6.2 Functions Reading Property Values from an Object

*Semantics:* Reading Property values of a line value.

*Complexity:* $O( 1)$

*/
    inline bool IsOrdered() const
    {
      return ordered;
    }
/*
This function decides whether the half segments in the line value is sorted.

*/

    inline bool IsEmpty() const
    {
      return Size() == 0;
    }
/*
This function decides whether the line value is empty.

*/
    inline int Size() const
    {
      return line.Size();
    }
/*
This function returns the number of half segments in the line value.

*/
    inline void Get( const int i, CHalfSegment const*& chs ) const
    {
      line.Get( i, chs );
    }
/*
This function reads the ith half segment from the line value.

6.3 Bulkload Functions

*/
    void StartBulkLoad();
/*
This function marks the begin of a bulk load of line relaxing the condition that the half segments
must be ordered.

*/
    void EndBulkLoad( const bool sort = true );
/*
This function marks the end of a bulk load and sorts the half segments.

6.4 Overloaded Class Operators

6.4.1 Operation $=$ (~assignment~)

*Semantics:* $this = cl$. It assigns the value of a line to another.

*Complexity:* $O( n )$

*/
    CLine& operator=(const CLine& cl);
/*
6.4.2 Operation $==$ (~equal~)

*Semantics:* $this ==cl$. It decides whether two line values are identical.

*Complexity:* $O( n )$

*/
    bool operator==(const CLine& cl) const;

/*
6.4.2 Operation $!=$ (~equal~)

*Semantics:* $this !=cl$. It decides whether two line values are different.

*Complexity:* $O( n )$

*/
    bool operator!=(const CLine& cl) const;
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
    void Clip( const Rectangle<2>& r, CLine& result ) const;
/*

6.5 Clone Function

*/
    inline CLine* Clone() const
    {
      return new CLine( *this );
    }
/*
6.6 Object Traversal Functions

*Semantics:* The following functions are implemented as basic utilities. They will be useful
if we want to adapt our algorithms to object-traversal-based ones.

*Complexity:* All these functions have a complexity of $O( 1 )$ .

*/
    inline void SelectFirst() const
    {
      if (IsEmpty()) pos=-1;
      else pos=0;
    }
/*
put the pointer ~pos~ to the first half segment in the line value.

*/
    inline void SelectNext() const
    {
      if ((pos>=0) && (pos<Size()-1)) pos++;
      else pos=-1;
    }
/*
move the pointer ~pos~ to the next half segment in the line value.

*/
    inline bool EndOfHs() const
    {
      return pos == -1;
    }
/*
decide whether ~pos~ is -1, which indicates that no more half segments in the line value
 need to be processed.

*/
    inline bool GetHs( CHalfSegment const*& chs ) const
    {
      if( pos >= 0 && pos <= Size()-1 )
      {
        line.Get( pos, chs);
        return true;
      }
      return false;
    }
/*
get the current half segment from the line value according to the ~pos~ pointer.

*/
    void InsertHs( const CHalfSegment& chs );

/*
6.7 Window clipping functions

6.7.1 WindowClippingIn

This function returns the part of the line that is inside the window.
The inside parameter is set to true if there is at least one segment
part inside the window. If the intersection part is a point, then
it is not considered in the result.

*/
void WindowClippingIn(const Rectangle<2> &window,
                      CLine &clippedLine,
                      bool &inside) const;

/*
6.7.2 WindowClippingOut

This function returns the part of the line that is outside the window.
The outside parameter is set to true if there is at least one part of the segment
that is outside of the window. If the intersection part is a point, then
it is not considered in the result.

*/

void WindowClippingOut(const Rectangle<2> &window,
                       CLine &clippedLine,
                       bool &outside) const;

/*
insert a half segment into the line value, and put the ~pos~ pointer to this newly inserted
half segment.

6.8 Functions needed to import the the ~Line~ data type to tuple

There are totally 10 functions which are defined as virtual functions. They need
to be defined here in order for the Point data type to be used in Tuple definition
as an attribute.

*/
    inline int NumOfFLOBs() const
    {
      return 1;
    }

    inline FLOB *GetFLOB(const int i)
    {
      return &line;
    }

    inline bool IsDefined() const
    {
      return true;
    }

    inline void SetDefined(bool Defined)
    {}

    inline bool Adjacent(const Attribute * arg) const
    {
      return false;
    }

    size_t HashValue() const;
    void CopyFrom(const StandardAttribute* right);
    int Compare(const Attribute * arg) const;
    ostream& Print( ostream &os ) const;
    void Clear();

    void CohenSutherlandLineClipping(const Rectangle<2> &window,
                                     CHalfSegment &chsInside, bool &accept);
  private:
/*
6.9 Private member functions

*/
    void Sort();
/*
Sorts (quick-sort algorithm) the persistent array of half segments in the line value.

*/
    int Position(const CHalfSegment&) const;
/*
Searches (binary search algorithm) for a half segment in the line value and
returns its position. Returns -1 if the half segment is not found.

6.10 Atrtibutes

*/
    DBArray<CHalfSegment> line;
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
};

/*
6.11 overloaded output operator

*/
ostream& operator<<( ostream& o, const CLine& cl );

/*
7 Class Region

This class implements the memory representation of the ~region~ type constructor. A region is
composed of a set of faces. Each face consists of a ouer cycle and a groups of holes.

*/

class EdgePoint;

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
    inline CRegion() {}
/*
This constructor should not be used.

*/
    inline CRegion(const int initsize):
    region( initsize ),
    bbox( false ),
    ordered( true )
    {}

    inline CRegion(const CRegion& cr, bool onlyLeft = false):
    region( cr.Size() ),
    bbox(cr.BoundingBox()),
    ordered( true )
    {
      if( !onlyLeft )
      {
        assert( cr.IsOrdered() );
        for( int i = 0; i < cr.Size(); i++ )
        {
          const CHalfSegment *chs;
          cr.Get( i, chs );
          Put( i, *chs );
        }
      }
      else
      {
        int j=0;
        for( int i = 0; i < cr.Size(); i++ )
        {
          const CHalfSegment *chs;
          cr.Get( i, chs );
          if (chs->GetLDP())
          {
            Put( j, *chs );
            j++;
          }
        }
      }
    }

    inline void Destroy()
    {
      region.Destroy();
    }

    inline ~CRegion()
    {}

    inline bool Valid() const;

    inline const Rectangle<2> BoundingBox() const
    {
      if( !IsEmpty() )
        return bbox;
      return Rectangle<2>( false );
    }

/*
7.2 Functions Reading Property Values from an Object

*Semantics:* Reading Property values of a region value.

*Complexity:* $O( 1)$

*/
    inline bool IsOrdered() const
    {
      return ordered;
    }
/*
This function decides whether the half segments in the region value is sorted.

*/
    inline bool IsEmpty() const
    {
      return Size() == 0;
    }
/*
This function decides whether the region value is empty.

*/
    inline int Size() const
    {
      return region.Size();
    }
/*
This function returns the number of half segments in the region value.

*/
    inline void Get( const int i, CHalfSegment const*& chs ) const
    {
      return region.Get( i, chs );
    }
/*
This function reads the ith half segment from the region value.

*/
  
    inline void Put( const int i, const CHalfSegment& chs )
    {
      if( !chs.Valid() )
      {
        cerr << "Trying to write an invalid half segment: " << chs << endl;
        exit( 1 );
      }
      region.Put( i, chs );
    }
/*
Writes a halfsegment ~chs~ into position ~i~.

7.3 Bulkload Functions

*/
    void StartBulkLoad();
/*
Marks the begin of a bulk load of line relaxing the condition that the half segments must be
ordered.

*/
    void EndBulkLoad( const bool sort = true );
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
    CRegion& operator=(const CRegion& cr);
/*
7.5.2 Operation $==$ (~equal~)

*Semantics:* $this ==cr$. It decides whether two region values are identical.

*Complexity:* $O( n )$

*/
    bool operator==(const CRegion& cr) const;
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
7.5.2 Operation $!=$ (~not equal~)

*Semantics:* $this !=cr$. It decides whether two region values are not identical.

*Complexity:* $O( n )$

*/
    bool operator!=(const CRegion& cr) const;

/*
7.6 Clone Function

*/
    inline CRegion* Clone() const
    {
      return new CRegion( *this );
    }

/*
7.7 Object Traversal Functions

*Semantics:* The following functions are implemented as basic utilities. They will be useful
if we want to adapt our algorithms to object-traversal-based ones.

*Complexity:* All the following functions have a complexity of $O( 1)$ .

*/
    inline void SelectFirst() const
    {
      if (IsEmpty()) pos=-1;
      else pos=0;
    }
/*
Put the pointer ~pos~ to the first half segment in the region value.

*/
    inline void SelectNext() const
    {
      if ((pos>=0) && (pos<Size()-1)) pos++;
      else pos=-1;
    }
/*
Move the pointer "pos" to the next half segment in the region value.

*/
    inline bool EndOfHs() const
    {
      return pos == -1;
    }
/*
decide whether ~pos~ is -1, which indicates that no more half segments in the region value
 need to be processed.

*/
    inline bool GetHs( CHalfSegment const*& chs ) const
    {
      if( pos >= 0 && pos <= Size()-1 )
      {
        region.Get( pos, chs);
        return true;
      }
      return false;
    }
/*
get the current half segment from the region value according to the ~pos~ pointer.

*/
    void InsertHs( const CHalfSegment& chs );
/*
Insert a half segment into the region value, and put the ~pos~ pointer to this newly inserted
half segment.

*/
    inline const AttrType& GetAttr() const
    {
      assert(( pos>=0) && (pos<=Size()-1));
      const CHalfSegment *chs;
      region.Get( pos, chs);
      return chs->GetAttr();
    }
/*
read the ~attr~ value of the current half segment from the region value. The current
half segment is indicated by ~pos~

*/
    inline const AttrType& GetAttr(int position) const
    {
      assert(( position>=0) && (position<=Size()-1));
      const CHalfSegment *chs;
      region.Get( position, chs);
      return chs->GetAttr();
    }
/*
read the ~attr~ value of the half segment at the position ~position~ from the region value.

*/
    inline void UpdateAttr( AttrType& attr )
    {
      if (( pos>=0) && (pos<=Size()-1))
      {
        const CHalfSegment *chs;
        region.Get( pos, chs);
        CHalfSegment aux( *chs );
        aux.SetAttr(attr);
        Put( pos, aux );
      }
    }
/*
update the ~attr~ value of the current half segment from the region value.The current
half segment is indicated by ~pos~

*/
    inline void UpdateAttr( int position, AttrType& ATTR )
    {
      if (( position>=0) && (position<=Size()-1))
      {
        const CHalfSegment *chs;
        region.Get( position, chs);
        CHalfSegment aux (*chs);
        aux.SetAttr(ATTR);
        Put( position, aux );
      }
    }
/*
update the ~attr~ value of the half segment at position ~position~  from the region value.

7.8 contain function (point)

*Semantics:* This function decides whether a point is inside the region.

*Complexity:* $O( n )$  where ~n~ is the number of segments of the region.

*/
    bool contain_old( const Point& p ) const;
    bool contain( const Point& p ) const;
    bool containpr( const Point& p, int &pathlength, int & scanned ) const;
/*
7.9 innercontain function

*Semantics:* This function decides whether a point is inside the inner part of the region.

*Complexity:* $O( n )$  where ~n~ is the number of segments of the region.

*/
    bool innercontain( const Point& p ) const;
/*
7.10 contain function (segment)

*Semantics:* This function decides whether a half segment is inside the region.

*Complexity:* $O( n )$  where ~n~ is the number of segments of the region.

*/
    bool contain( const CHalfSegment& chs ) const;
/*
7.11 holeedge-contain function

*Semantics:* This function decides whether a half segment is inside a hole edge of the region.

*Complexity:* $O( n )$  where ~n~ is the number of segments of the region.

*/
    bool holeedgecontain( const CHalfSegment& chs ) const;
/*
The following two functions are used to sort the half segments according to their attributes;

*/
    void logicsort();
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

    inline FLOB *GetFLOB(const int i)
    {
      return &region;
    }

    inline bool IsDefined() const
    {
      return true;
    }

    inline void SetDefined(bool Defined)
    {}
      
    inline bool Adjacent(const Attribute * arg) const
    {
      return false;
    }

    size_t HashValue() const;
    void CopyFrom(const StandardAttribute* right);
    int Compare(const Attribute * arg) const;
    ostream& Print( ostream &os ) const;
    void Clear();

/*
~Translate~

Moves the region according x and y and stores the result in result.


*/

    void Translate(const Coord& x, const Coord& y, CRegion* result){
        result->Clear();
        if(!IsEmpty()){
            result->StartBulkLoad();
            int size = Size();
            const CHalfSegment* chs;
            for(int i=0;i<size;i++){
              Get(i,chs);
              CHalfSegment aux(*chs);
              aux.Translate(x,y);
              *result += aux;  
            }
            result->EndBulkLoad(false);
        } 
    }

/*
~Translate~
 
Moves this region.

*/
    void Translate(const Coord& x, const Coord& y){
       double t[2];
       t[0] = x;
       t[1] = y;
       bbox = bbox.Translate(t);
       int size = Size();
       const CHalfSegment* chs;
       for(int i=0;i<size;i++){
           Get(i,chs);
           CHalfSegment tchs(*chs); 
           tchs.Translate(x,y);
           Put(i,tchs);
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

  static bool GetCycleDirection(const Point &pA, 
                                const Point &pP, const Point &pB);

  bool GetCycleDirection() const;

/*
7.15 window clipping functions

*/

/*
7.15.1 Window clipping IN function

This function returns the clipped half segments that are within a window, which
result from the clipping of a region to a clip window.

*/

void WindowClippingIn(const Rectangle<2> &window,
                      CRegion &clippedRegion) const;

/*
7.15.2 Window clipping OUT function

This function returns the clipped half segments that are outside a window, which
result from the clipping of a region to a clip window.

*/

void WindowClippingOut(const Rectangle<2> &window,
                       CRegion &clippedRegion) const;
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
                    CRegion &clippedRegion,bool inside) const;
/*
7.15.4 Get clipped half segment IN function

This function returns the clipped half segments (that are within the window)
resulting from the clipping of a region to a clip window.

*/

  void GetClippedHSIn(const Rectangle<2> &window,
                      CRegion &clippedRegion,
                      vector<EdgePoint> pointsOnEdge[4],
                      int &partnerno) const;

/*
7.15.5 Get clipped half segment OUT function

This function returns the clipped half segments (that are outside the window)
resulting from the clipping of a region to a clip window.

*/

 void GetClippedHSOut(const Rectangle<2> &window,
                      CRegion &clippedRegion,
                      vector<EdgePoint> pointsOnEdge[4],
                      int &partnerno) const;

/*
7.15.6 Add clipped half segment function

This function is just used in order to add clipped half segments to the clipped
region.

*/
   void AddClippedHS(const Point &pl,const Point &pr, 
                     AttrType &attr,int &partnerno);

/*
7.15.7 Clipped half segment on edge function

This function checks if the clipped half segment lies on one of the window's edges,
and if it happens the end points of the half segment are add to the corresponding
list of the ~points on edge~. The ~points on edge~ list are used to create the
new half segments that lies on edge.

*/
   static bool ClippedHSOnEdge(const Rectangle<2> &window,
                               const CHalfSegment &chs,
                               bool clippingIn,
                               vector<EdgePoint> pointsOnEdge[4]);

/*
7.15.8 Create new segments function

This function creates the half segments resulting from the connection
of the points that lies on the window's edges.

*/
   static void CreateNewSegments(vector <EdgePoint>pointsOnEdge, CRegion &cr,
                                 const Point &bPoint,const Point &ePoint,
                                 WindowEdge edge,int &partnerno,bool inside);
/*
7.15.9 Create new segments function

This function creates new half segments corresponding to the edges of the window.
These half segments are only created if the window's edge is completly inside
the window.

*/
   void CreateNewSegmentsWindowVertices(const Rectangle<2> &window,
                                vector<EdgePoint> pointsOnEdge[4],CRegion &cr,
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
   void ComputeCycle(CHalfSegment &chs, int faceno,
                  int cycleno,int &edgeno, bool *cycle);

/*

7.15.11.2. Is CriticalPoint function

Returns if a point (adjacentPoint) is a critical point.

*/

bool IsCriticalPoint(const Point &adjacentPoint,const int &chsPosition) const;

/*

7.15.11.2. Get adjacent half segment function

This function returns the adjacent half segment of chs that hasn't set the
cycle and edge numbers yet. It also returns the point that belongs to both half segments, and
also if this point is a critical one.

*/

  bool GetAdjacentHS(const CHalfSegment &chs, const int &chsPosition,
                     int &position, const int &partnerno,
                     const int &partnernoP, CHalfSegment const*& adjacentCHS,
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
   int GetNewFaceNo(CHalfSegment &chsS,bool *cycle);



/*
7.16 Intersect funcion

*/
bool Intersects(const CRegion &r) const;

/*
7.17 Components function

This function returns the faces of this region as a set of regions.
The pointers inside the array ~components~ are here initialized
and must be deleted outside.

*/
    void Components( vector<CRegion*>& components );

  private:
/*
7.17 Private member functions

*/
    void Sort();
/*
sorts (quick-sort algorithm) the persistent array of half segments in the region value.

*/
    int Position(const CHalfSegment&) const;
    int Position(const Point&) const;
/*
searches (binary search algorithm) for a half segment in the region value and
returns its position. Returns -1 if the half segment is not found.
*/

/*
7.18 Atrtibutes

*/
    DBArray<CHalfSegment> region;

    Rectangle<2> bbox;

/*
The persisten array of half segments.

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
ostream& operator<<( ostream& o, const CRegion& cr );

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

    EdgePoint(const Point p,const bool dir,const bool reject): Point(p)
    {
       direction = dir;
       rejected = reject;
    }

    void Set(const Coord& X, const Coord& Y,const bool dir, const bool reject)
    {
      x = X;
      y = Y;
      direction = dir;
      defined = true;
      rejected = reject;
    }

    void Set(const Point p,const bool dir,const bool reject)
    {
      Set(p.GetX(),p.GetY(),dir,reject);
    }

    void  Set( const Coord& X, const Coord& Y)
    {
       defined=true;
       rejected = false;
       x=X;
       y=Y;
    }



    EdgePoint& operator=(const EdgePoint& p)
    {
      defined = p.IsDefined();
      if( defined )
      {
        x = p.GetX();
        y = p.GetY();
        direction = p.direction;
        rejected = p.rejected;
      }
      return *this;
    }

    static EdgePoint* GetEdgePoint(const Point &p,const Point &p2,
                                   bool insideAbove,
                                   const Point &v,const bool reject);

    bool operator==(const EdgePoint& p)
    {
      if( AlmostEqual( this->GetX(), p.GetX() ) &&
          AlmostEqual( this->GetY(), p.GetY() ) &&
          (this->direction == p.direction)  &&
          (this->rejected == p.rejected) )
          return true;
      return false;
    }

    bool operator!=(const EdgePoint &p)
    {
      return !(*this==p);
    }


    inline bool EdgePoint::operator<( const EdgePoint& p ) const
    {
      assert( defined && p.defined );
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
    Assignement operator redefinition.

    *Precondition:* ~p.IsDefined()~

    */
    /*
      This function sets the value of the "attr" argument of a half segment.
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
    //Indicates if the point is of a segment that was rejected
};

/*
10.2 SCycle

This class is used to store the information need for cycle computation which sets the face number,
cycle number and edge number of the half segments.

*/

class SCycle
{
  public:
    CHalfSegment chs1,chs2;
    int chs1PosRight,chs1PosLeft,
        chs2PosRight,chs2PosLeft;
    bool goToCHS1Right,goToCHS1Left,
         goToCHS2Right,goToCHS2Left;
    int chs1Partnerno,chs2Partnerno;
    Point *criticalPoint;
    Point nextPoint;

    SCycle(){}

    SCycle(const CHalfSegment &chs, const int partnerno,
           const CHalfSegment &chsP,const int partnernoP,
           Point *criticalP,const Point &nextPOINT)
    {
      chs1 = chs;
      chs2 = chsP;
      chs1PosRight = chs1PosLeft = partnernoP;
      chs2PosRight = chs2PosLeft = partnerno;
      chs1Partnerno = partnerno;
      chs2Partnerno = partnernoP;

      goToCHS1Right=goToCHS1Left=goToCHS2Right=goToCHS2Left=true;
      criticalPoint = criticalP;
      nextPoint = nextPOINT;

    }

    SCycle(const SCycle &sAux)
    {
      chs1 = sAux.chs1;
      chs2 = sAux.chs2;
      chs1PosRight  = sAux.chs1PosRight;
      chs1PosLeft   = sAux.chs1PosLeft;
      chs1Partnerno = sAux.chs1Partnerno ;
      goToCHS1Right = sAux.goToCHS1Right;
      goToCHS1Left  = sAux.goToCHS1Left;

      chs2PosRight  = sAux.chs2PosRight;
      chs2PosLeft   = sAux.chs2PosLeft;
      chs2Partnerno = sAux.chs2Partnerno ;
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

#endif // __SPATIAL_ALGEBRA_H__
