

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

//[_] [\_]


1. Declaration of a Region Type




*/


#ifndef RegionT_H
#define RegionT_H

#include "WindowEdge.h"
#include "../Rectangle/RectangleAlgebra.h"
#include "Coord.h"
#include "HalfSegment.h"

class Point;
template<template<typename T> class ArrayT> class PointsT;
template<template<typename T> class ArrayT> class LineT;
template<template<typename T> class ArrayT> class RegionT;
template<template<typename T> class ArrayT> class SimpleLineT;

class EdgePoint;
struct AttrType;


template<template<typename T>class ArrayT>
class RegionT : public StandardSpatialAttribute<2>
{
  public:

  template<template<typename T3> class Array3> friend class RegionT;

/*
7.1 Constructors and Destructor

*/
    explicit RegionT( const int n );
/*
Constructs an empty region allocating space for ~n~ half segments.

*/
    template<template<typename T2> class ArrayT2>
    RegionT( const RegionT<ArrayT2>& cr, bool onlyLeft);
    
    template<template<typename T2> class ArrayT2>
    RegionT( const RegionT<ArrayT2>& cr);

    RegionT( const RegionT& cr);

/*
The copy constructor. If the flag ~onlyLeft~ is set, then only the half 
segments with left dominating point are copied.

*/

    explicit RegionT( const Rectangle<2>& r );
/*
Creates a rectangular region from a rect2 objects.

*/

    RegionT( const Point& p1, const Point& p2, const Point& p3 );
/*
Creates a triangular region from three Point objects.

*/

    void Destroy();
/*
This function should be called before the destructor if one wants to destroy the
persistent array of half segments. It marks the persistent array for destroying.
The destructor will perform the real destroying.

*/
    inline ~RegionT() {
    }
/*
The destructor.

6.2 Functions for Bulk Load of Points

As said before, the region is implemented as an ordered persistent array 
of half segments.
The time complexity of an insertion operation in an ordered array is $O(n)$,
 where ~n~ is the number of half segments. In some cases, bulk load of 
segments for example, it is good to relax the ordered condition to improve 
the performance. We have relaxed this ordered condition only for bulk load 
of half segments. All other operations assume that the set of
half segments is ordered.

*/
    bool IsOrdered() const;
/*
Returns whether the set of half segments is ordered. There is a flag ~ordered~
(see attributes) in order to avoid a scan in the half segments set to answer 
this question.

*/
    void StartBulkLoad();
/*
Marks the begin of a bulk load of half segments relaxing the condition that 
the half segments must be ordered.

*/
     void EndBulkLoad( bool sort = true,
                       bool setCoverageNo = true,
                       bool setPartnerNo = true,
                       bool computeRegion = true );
/*
Marks the end of a bulk load and sorts the half segments set if the argument 
~sort~ is set to true.

*/

     void Resize(const int newSize);
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
     return (new (addr) RegionT<ArrayT>());
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
Reads the ~attr~ value of the half segment at the position ~position~ 
from the ~region~ value.

*/
    void UpdateAttr( int position, AttrType& attr );
/*
Updates the ~attr~ value of the half segment at position ~position~ 
from the ~region~ value.

*/
    inline void UpdateAttr( AttrType& attr );
/*
Updates the ~attr~ value of the current half segment from the ~region~
 value.The current half segment is indicated by ~pos~.

*/
    bool InsertOk( const HalfSegment& hs ) const;
/*
This function check whether a region value is valid after the 
insertion of a new half segment.
Whenever a half segment is about to be inserted, the state of the 
region is checked.
A valid region must satisfy the following conditions:

1) Any two cycles of the same region must be disconnected, which means 
that no edges
of different cycles can intersect each other;

2) Edges of the same cycle can only intersect in their endpoints, but 
not in their middle points;

3) For a certain face, the holes must be inside the outer cycle;

4) For a certain face, any two holes can not contain each other;

5) Faces must have the outer cycle, but they can have no holes;

6) For a certain cycle, any two vertex can not be the same;

7) Any cycle must be made up of at least 3 edges;

8) It is allowed that one face is inside another provided that their 
edges do not intersect.

*/
    template<template<typename T2> class Array2>
    RegionT<ArrayT>& operator=( const RegionT<Array2>& r );

    RegionT<ArrayT>& operator=( const RegionT& r );

/*
Assignement operator redefinition.

7.5 Operations

7.5.2 Operation $=$ (~equal~)

*Semantics:* $U == V$

*Complexity:* $O(m + n)$, where ~m~ is the size of ~U~ and ~n~ the size of ~V~.

*/
    bool operator==( const RegionT<ArrayT>& c ) const;
/*
6.4.2 Operation $\neq$ (~not equal~)

*Precondition:* ~U.IsOrdered() and V.IsOrdered()~

*Semantics:* $U != V$

*Complexity:* $O(m + n)$, where ~m~ is the size of ~U~ and ~n~ the size of ~V~.

*/
    bool operator!=( const RegionT<ArrayT>& r ) const;
/*
6.4.3 Operation ~union~

*Semantics:* $U \cup \{v\}$

*Complexity:* $O( 1 )$, if the set is not ordered; and $O(\log n + n)$, 
otherwise; where ~n~ is the size of ~U~.

*/
    RegionT<ArrayT>& operator+=( const HalfSegment& hs );
/*
6.4.4 Operation ~minus~

*Precondition:* ~U.IsOrdered()~

*Semantics:* $U \ \{v\}$

*Complexity:* $O(log(n)+n)$, where ~n~ is the size of ~U~.

*/
    RegionT<ArrayT>& operator-=( const HalfSegment& hs );
/*
6.4.4 Operation ~intersects~

*Precondition:* ~U.IsOrdered() and V.IsOrdered()~

*Semantics:* $U \cap V \neq \emptyset$

*Complexity:* $O()$, where ~m~ is the size of ~U~ and ~m~ the size of ~V~.

*/
  template<template<typename T2> class ArrayT2> 
  bool Intersects( const RegionT<ArrayT2>& r, const Geoid* geoid = 0 ) const;
/*
6.4.4 Operation ~intersection~

*/
  template<template<typename T2> class Array2>
  void Intersection(const Point& p, PointsT<Array2>& result, 
                    const Geoid* geoid=0) const;

  template<template<typename T2> class Array2,
           template<typename T3> class Array3>
  void Intersection(const PointsT<Array2>& ps, PointsT<Array3>& result,
                    const Geoid* geoid=0) const;

  template<template<typename T2> class Array2,
           template<typename T3> class Array3>
  void Intersection(const LineT<Array2>& l, LineT<Array3>& result, 
                    const Geoid* geoid=0) const;

  template<template<typename T2> class Array2,
           template<typename T3> class Array3>
  void Intersection(const RegionT<Array2>& r, RegionT<Array3>& result,
                    const Geoid* geoid=0) const;

  template<template<typename T2> class Array2,
           template<typename T3> class Array3>
  void Intersection(const SimpleLineT<Array2>& l, SimpleLineT<Array3>& result,
                    const Geoid* geoid=0) const;

/*
6.4.4 Operation ~Union~

*/
  template<template<typename T2> class Array2>
  void Union(const Point& p, RegionT<Array2>& result, 
             const Geoid* geoid=0) const;

  template<template<typename T2> class Array2,
           template<typename T3> class Array3>
  void Union(const PointsT<Array2>& ps, RegionT<Array3>& result, 
             const Geoid* geoid=0) const;

  template<template<typename T2> class Array2,
           template<typename T3> class Array3>
  void Union(const LineT<Array2>& l, RegionT<Array3>& result, 
             const Geoid* geoid=0) const;

  template<template<typename T2> class Array2,
           template<typename T3> class Array3>
  void Union(const RegionT<Array2>& r, RegionT<Array3>& result, 
             const Geoid* geoid=0) const;

  template<template<typename T2> class Array2,
           template<typename T3> class Array3>
  void Union(const SimpleLineT<Array2>& l, RegionT<Array3>& result, 
             const Geoid* geoid=0) const;


/*
6.4.4 Operation ~Minus~

*/
  template<template<typename T2> class Array2>
  void Minus(const Point& p, RegionT<Array2>& result, 
             const Geoid* geoid=0) const;

  template<template<typename T2> class Array2,
           template<typename T3> class Array3>
  void Minus(const PointsT<Array2>& ps, RegionT<Array3>& result, 
             const Geoid* geoid=0) const;

  template<template<typename T2> class Array2,
           template<typename T3> class Array3>
  void Minus(const LineT<Array2>& l, RegionT<Array3>& result, 
             const Geoid* geoid=0) const;

  template<template<typename T2> class Array2,
           template<typename T3> class Array3>
  void Minus(const RegionT<Array2>& r, RegionT<Array3>& result, 
             const Geoid* geoid=0) const;

  template<template<typename T2> class Array2,
           template<typename T3> class Array3>
  void Minus(const SimpleLineT<Array2>& l, RegionT<Array3>& result, 
             const Geoid* geoid=0) const;


/*
6.4.4 Operation ~inside~

*Precondition:* ~U.IsOrdered() and V.IsOrdered()~

*Semantics:* $U \subseteq V$

*Complexity:* $O(m.n)$, where ~m~ is the size of ~U~ and ~n~ the size of ~V~.

*/
  template<template<typename T2> class Array2>
  bool Inside( const RegionT<Array2>& r, const Geoid* geoid=0 ) const;
/*
6.4.4 Operation ~adjacent~

*Precondition:* ~U.IsOrdered() $\&\&$ V.IsOrdered()~

*Semantics:* $\partial U \cap \partial V \neq \emptyset 
              \land U^0 \cap V^0 = \emptyset$

*Complexity:* $O(m.n)$, where ~m~ is the size of ~U~ and ~n~ the size of ~V~.

*/
  template<template<typename T2> class Array2>
  bool Adjacent( const RegionT<Array2>& r, const Geoid* geoid =0 ) const;
/*
6.4.4 Operation ~overlaps~

*Precondition:* ~U.IsOrdered() $\&\&$ V.IsOrdered()~

*Semantics:* $U^0 \cap V^0 \neq \emptyset$

*Complexity:* $O(m.n)$, where ~m~ is the size of ~U~ and ~n~ the size of ~V~.

*/ 
  template<template<typename T2> class Array2>
  bool Overlaps( const RegionT<Array2>& r, const Geoid* geoid=0 ) const;
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
  template<template<typename T2> class Array2>
  double Distance( const PointsT<Array2>& p, const Geoid* geoid=0 ) const;
/*
6.4.4 Operation ~distance~ (with ~points~)

*Precondition:* ~U.IsOrdered() $\&\&$ V.IsOrdered()~

*Semantics:* $\min\{ dist(u, v) | u \in U, v \in V \}$

*Complexity:* $O(m.n)$, where ~m~ is the size of ~U~ and ~n~ the size of ~V~.

*/
  template<template<typename T2> class Array2>
  double Distance( const RegionT<Array2>& r, const Geoid* geoid=0 ) const;
/*
  6.4.4 Operation ~distance~ (with ~region~)

  *Precondition:* ~U.IsOrdered() $\&\&$ V.IsOrdered()~

  *Semantics:* $\min\{ dist(u, v) | u \in U, v \in V \}$

  *Complexity:* $O(m.n)$, where ~m~ is the size of ~U~ and ~n~ the size of ~V~.

*/
  template<class LineType>
  double Distance(const LineType& l, const Geoid* geoid=0) const;

  double Distance( const Rectangle<2>& r, const Geoid* geoid=0 ) const;
/*
  6.4.4 Operation ~distance~ (with ~rect2~)

  *Precondition:* ~U.IsOrdered() $\&\&$ V.IsOrdered()~

  *Semantics:* $\min\{ dist(u, v) | u \in U, v \in V \}$

  *Complexity:* $O(m.n)$, where ~m~ is the size of ~U~ and ~n~ the size of ~V~.

*/

  bool Intersects(const Rectangle<2>&r, const Geoid* geoid=0) const;


/*
6.4.4 Operation ~components~

*Precondition:* ~U.IsOrdered()~

*Semantics:* returns the faces as a set (~vector~) of regions

*Complexity:* $O(n)$, where ~n~ is the size of ~U~.

The pointers inside the array ~components~ are here initialized
and must be deleted outside.

*/
  template<template<typename T2> class Array2>
  void Components( std::vector<RegionT<Array2>*>& components );

/*
6.4.5 Operation ~getHoles~

This operation returns all holes of the region as another region.

*/
  template<template<typename T2> class Array2>
  void getHoles(RegionT<Array2>& result) const;


/*
6.4.5 Operation ~touch[_]points~ (with ~line~)

*Precondition:* ~U.IsOrdered() and V.IsOrdered()~

*Semantics:* $\{ p \in U \cap \partial V | 
              p \textrm{ is isolated in } U \cap \partial V \}$

*Complexity:* $O(m.n + r\log r)$, where ~m~ is the size of ~U~,
              ~n~ the size of ~V~, and ~r~
is the ~points~ result size.

*/ 
  template<template<typename T2> class Array2,
           template<typename T3> class Array3>
  void TouchPoints( const LineT<Array2>& l, PointsT<Array3>& result, 
                    const Geoid* geoid=0 ) const;
/*
6.4.5 Operation ~touch[_]points~ (with ~region~)

*Precondition:* ~U.IsOrdered() and V.IsOrdered()~

*Semantics:* $\{ p \in \partial U \cap \partial V | 
                 p \textrm{ is isolated in } \partial U \cap \partial V \}$

*Complexity:* $O(m.n + r\log r)$, where ~m~ is the size of ~U~, 
                ~n~ the size of ~V~, and ~r~
is the ~points~ result size.

*/
  template<template<typename T2>class Array2,
           template<typename Ti3>class Array3>
  void TouchPoints( const RegionT<Array2>& r, PointsT<Array3>& result,
                    const Geoid* geoid=0 ) const;
/*
6.4.5 Operation ~common[_]border~

*Precondition:* ~U.IsOrdered() and V.IsOrdered()~

*Semantics:* $\partial U \cap \partial V $

*Complexity:* $O(m.n + r\log r)$, where ~m~ is the size of ~U~, 
             ~n~ the size of ~V~, and ~r~
is the ~line~ result size.

*/
  template<template<typename T2>class Array2,
           template<typename Ti3>class Array3>
  void CommonBorder( const RegionT<Array2>& r, LineT<Array3>& result,
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
    template<template<typename T2>class Array2>
    void Vertices( PointsT<Array2>* result, const Geoid* geoid=0 ) const;


/*
6.4.5 Operation ~boundary~

*Precondition:* ~U.IsOrdered()~

*Semantics:* The boundary of ~U~

*Complexity:* $O(m)$, where ~m~ is the size of ~U~.

*/
    template<template<typename T2>class Array2>
    void Boundary(LineT<Array2>* result, const Geoid* geoid=0) const;

/*
4.4 Object Traversal Functions

These functions are object traversal functions which are useful when we are
using ROSE algebra algorithms.

*Pre-condition:* ~IsOrdered()~

*Complexity:* All these functions have a complexity of $O( 1 )$ .

*/
    void SelectFirst() const;
/*
Puts the pointer ~pos~ to the first half segment in the ~region~ value.

*/
    void SelectNext() const;
/*
Moves the pointer ~pos~ to the next half segment in the ~region~ value.

*/
    inline bool EndOfHs() const;
/*
Decides whether ~pos~ is -1, which indicates that no more half 
segments in the ~region~ value need to be processed.

*/
    bool GetHs(HalfSegment& hs ) const;
/*
Gets the current half segment from the ~region~ value according
 to the ~pos~ pointer.

7.8 contain function (point)

*Semantics:* This function decides whether a point is inside the region.

*Complexity:* $O( n )$  where ~n~ is the number of segments of the region.

*/
    bool Contains( const Point& p, const Geoid* geoid=0 ) const;
/*
7.9 innercontain function

*Semantics:* This function decides whether a point is inside the inner 
part of the region.

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

*Semantics:* This function decides whether a half segment is completely
 inside the region.

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

*Semantics:* This function decides whether a half segment is inside a hole
 edge of the region.

*Complexity:* $O( n )$  where ~n~ is the number of segments of the region.

*/
    bool HoleEdgeContain( const HalfSegment& hs, const Geoid* geoid=0 ) const;
/*
The following two functions are used to sort the half segments according
 to their attributes;

*/
    void LogicSort();
/*
7.12 Functions needed to import the the ~Region~ data type to tuple

There are totally 10 functions which are defined as virtual functions. They need
to be defined here in order for the Point data type to be used in Tuple 
definition as an attribute.

*/
    inline int NumOfFLOBs() const
    {
      return 1;
    }

    inline Flob *GetFLOB( const int i )
    {
      return &region;
    }




    void SetDefined( bool defined );

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
    template<template<typename T2>class Array2>
    void CopyFrom( const RegionT<Array2>* right );

    template<template<typename T2>class Array2>
    int Compare( const RegionT<Array2>* cr ) const;
    int Compare( const Attribute *arg ) const;
//     int CompareAlmost( const Attribute *arg ) const;
    std::ostream& Print( std::ostream &os ) const;
    virtual RegionT<ArrayT> *Clone() const;
    void Clear();
    void SetEmpty();

/*
~Translate~

Moves the region according x and y and stores the result in result.


*/

    template<template<typename T2>class Array2>
    void Translate(const Coord& x, const Coord& y, 
                   RegionT<Array2>& result) const;
    template<template<typename T2>class Array2>
    void Scale(const Coord& x, const Coord& y, RegionT<Array2>& result) const;

    template<template<typename T2>class Array2>
    void Rotate(const Coord& x, const Coord& y, const double alpha,
                RegionT<Array2>& result) const;

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

*Semantics:* This function sets the attribute InsideAbove of each 
region's half segment.
This attribute indicates if the area of the region lies above or left 
of the half segment.

*Complexity:* $O( n log n ) $  where ~n~ is the number of segments 
of the region.

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
  template<template<typename T2>class Array2>
  void WindowClippingIn(const Rectangle<2> &window,
                        RegionT<Array2> &clippedRegion, 
                        const Geoid* geoid=0) const;

/*
7.15.2 Window clipping OUT function

This function returns the clipped half segments that are outside a window, which
result from the clipping of a region to a clip window.

*/

  template<template<typename T2>class Array2>
  void WindowClippingOut(const Rectangle<2> &window,
                         RegionT<Array2> &clippedRegion, 
                         const Geoid* geoid=0) const;
/*
7.15.3 Get clipped half segment function

This function returns the clipped half segments resulting from the clipping of a
region to a clip window. It calls the ~GetClippedHSIn~ in order to get the 
clipped half segments that are within the window, or ~GetClippedHSOut~ in order
to get the clipped half segments thar are outside the window. Afterwards it 
calls the function ~CreateNewSegments~ to create the new half segments resulting
 from the connection of the points that lies on the window's edges. Finally, it
calls the function ~CreateNewSegmentsWindowVertices~ in order to create half 
segments corresponding to the edges of the window. These half segments are only
created if the window's edge is completly inside the window.

*/
  template<template<typename T2>class Array2>
  void GetClippedHS(const Rectangle<2> &window,
                    RegionT<Array2> &clippedRegion, bool inside,
                    const Geoid* geoid=0) const;
/*
7.15.4 Get clipped half segment IN function

This function returns the clipped half segments (that are within the window)
resulting from the clipping of a region to a clip window.

*/

  template<template<typename T2>class Array2>
  void GetClippedHSIn(const Rectangle<2> &window,
                      RegionT<Array2> &clippedRegion,
                      std::vector<EdgePoint> pointsOnEdge[4],
                      int &partnerno, const Geoid* geoid=0) const;
/*
7.15.5 Get clipped half segment OUT function

This function returns the clipped half segments (that are outside the window)
resulting from the clipping of a region to a clip window.

*/
 template<template<typename T2>class Array2>
 void GetClippedHSOut(const Rectangle<2> &window,
                      RegionT<Array2> &clippedRegion,
                      std::vector<EdgePoint> pointsOnEdge[4],
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

This function checks if the clipped half segment lies on one of the
window's edges, and if it happens the end points of the half segment are add 
to the corresponding list of the ~points on edge~. The ~points on edge~ list
are used to create the new half segments that lies on edge.

*/
   static bool ClippedHSOnEdge(const Rectangle<2> &window,
                               const HalfSegment &chs,
                               bool clippingIn,
                               std::vector<EdgePoint> pointsOnEdge[4],
                               const Geoid* geoid=0);
/*
7.15.8 Create new segments function

This function creates the half segments resulting from the connection
of the points that lies on the window's edges.

*/
   template<template<typename T2>class Array2>
   static void CreateNewSegments(std::vector <EdgePoint>pointsOnEdge,
                                 RegionT<Array2> &cr,
                                 const Point &bPoint,const Point &ePoint,
                                 WindowEdge edge,int &partnerno,bool inside,
                                 const Geoid* geoid=0);
/*
7.15.9 Create new segments function

This function creates new half segments corresponding to the edges of 
the window.
These half segments are only created if the window's edge is completly inside
the window.

*/
   template<template<typename T2>class Array2>
   void CreateNewSegmentsWindowVertices(const Rectangle<2> &window,
                                std::vector<EdgePoint> pointsOnEdge[4],
                                RegionT<Array2> &cr,
                                int &partnerno,bool inside,
                                const Geoid* geoid=0) const;
/*
7.15.10 Compute region function

This function computes a region from a list of half segments, in other orders
it sets the face number, cycle number and edge number of the half segments.
It calls the function ~GetNewFaceNo~ in order to get a new face number, and
it calls the function compute cycle to compute the cycle and edge numbers.
There are two pre-requisite: the partner number of the half segments must be
 already set, and they need to be ordered in the half segment order.

*/
   void ComputeRegion();

/*
7.15.11 Compute cycle functions

The following functions are used to compute the cycle and edge numbers of
 cycles.

7.15.11.1. Compute cycle function

This function sets the cycle and edge number of a face's cycle. It calls the
functions ~GetAdjacentHS~ and ~SearchForCriticalPoint~.

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
cycle and edge numbers yet. It also returns the point that belongs to both
half segments, and also if this point is a critical one.

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
    std::cerr << __PRETTY_FUNCTION__ << ": Function not implemented." 
              << std::endl;
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

  virtual void writeShape(std::ostream& o, uint32_t RecNo) const;

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

  const ArrayT<HalfSegment>& GetHalfSegments(){
      return region;
  }

  static const std::string BasicType(){
    return "region";
  }
  static const bool checkType(const ListExpr type){
    return listutils::isSymbol(type, BasicType());
  }

   inline RegionT<ArrayT>() {}
/*
This constructor should not be used.

*/

  private:



/*
7.17 Private member functions

*/
    void Sort();
/*
sorts (quick-sort algorithm) the persistent array of half segments in 
the region value.

*/
    bool Find( const HalfSegment&, int& pos ) const;
    bool Find( const Point&, int& pos ) const;
/*
searches (binary search algorithm) for a half segment in the region value and
returns its position. Returns -1 if the half segment is not found.

*/

  void saveShape(std::ostream& o, uint32_t RecNo) const;

/*
Saves the region in shape format to o.

*/


/*
7.18 Atrtibutes

*/
    ArrayT<HalfSegment> region;
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
The pointer to the current half segments. The pointer is important in 
object traversal algorithms.

*/
    bool ordered;
/*
Whether the half segments in the region value are sorted.

*/

};

#endif

