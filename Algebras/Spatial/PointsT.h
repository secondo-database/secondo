/*
----
This file is part of SECONDO.

Copyright (C) 2017, 
University in Hagen, 
Faculty of Mathematics and Computer Science,
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

*/



#ifndef POINTST_H
#define POINTST_H

#include "../Rectangle/RectangleAlgebra.h"
#include "Coord.h"
#include "Point.h"

template<template<typename T> class ArrayT> class LineT;
template<template<typename T> class ArrayT> class RegionT;
template<template<typename T> class ArrayT> class SimpleLineT;

/* 
auxiliary functions 

*/

int PointCompare( const void *a, const void *b );
int PointCompareAlmost( const void *a, const void *b );
enum object {none, first, second, both};
enum status {endnone, endfirst, endsecond, endboth};

template<template<typename T1>class Array1,
         template<typename T2>class Array2>
void SelectFirst_pp( const PointsT<Array1>& P1, 
                     const PointsT<Array2>& P2,
                     object& obj, status& stat );

template<template<typename T1>class Array1,
         template<typename T2>class Array2>
void SelectNext_pp( const PointsT<Array1>& P1, 
                    const PointsT<Array2>& P2,
                    object& obj, status& stat );

/*
1 The class PointsT

*/

template<template<typename T> class ArrayT>
class PointsT: public StandardSpatialAttribute<2>
{
  public:
/*
5.1 Constructors and Destructor

There are three ways of constructing a point set:

*/
    inline PointsT() {}
/*
This constructor should not be used.

*/
    explicit PointsT( const int initsize );
/*
The first one constructs an empty point set but open space for ~initsize~ 
points.

*/
    PointsT( const PointsT& ps);

    template< template<typename T2> class Array2>
    PointsT(const PointsT<Array2>&);

/*
The second one receives another point set ~ps~ as argument and constructs 
a point set which is a copy of ~ps~.

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
    inline ~PointsT()
    {}
/*
The destructor.

5.2 Functions for Bulk Load of Points

As said before, the point set is implemented as an ordered persistent array 
of points.  The time complexity of an insertion operation in an ordered array
is $O(n)$, where ~n~ is the size of the point set. In some cases, bulk load of 
points for example, it is good to relax the ordered condition to improve the 
performance. We have relaxed this ordered condition only for bulk load of 
points. All other operations assume that the point set is ordered.

*/
    bool IsOrdered() const;
/*
Returns whether the point set is ordered. There is a flag ~ordered~
(see attributes) in order to avoid a scan in the point set to answer 
this question.

*/
    void StartBulkLoad();
/*
Marks the begin of a bulk load of points relaxing the condition that the
points must be ordered.

*/
    void EndBulkLoad( bool sort = true, bool remDup = true, bool trim = true );
/*
Marks the end of a bulk load and sorts the point set if the argument ~sort~
is set to true.

5.3 Member functions

*/
    const Rectangle<2> BoundingBox(const Geoid* geoid = 0) const;
/*
Returns the bounding box that spatially contains all points.

*/
    bool IsEmpty() const;
/*
Returns true iff the set is undefined or empty.

*/
    bool IsValid() const;
/*
Checks if the point set is valid, i.e., if it contains only defined points and
no duplicates.

*/
    int Size() const;
/*
Returns the size of the point set in terms of number of points.
Returns ~0~ if the set is empty.

*/
    void Clear();
/*
Clears the point set.

*/
    void Resize(const int newSize);
/*
Sets the new capacity of the points array to the
maximum of its original size and the argument.

*/
    inline void TrimToSize();
/*
Sets the new capacity of the points array to the amount really required.

*/

    bool Get( const int i, Point& p ) const;
/*
Retrieves the point ~p~ at position ~i~ in the point set.

*Precondition:* $0 \leq i < Size()$

*/
    PointsT<ArrayT>& operator=( const PointsT& ps );
    
    template<template<typename T2> class Array2>
    PointsT<ArrayT>& operator=( const PointsT<Array2>& ps );
/*
Assignement operator redefinition.

*/
    bool Contains( const Point& p, const Geoid* geoid=0 ) const;
/*
Searches (binary search algorithm) for a point in the point set and
return ~true~ if found and ~false~ if not.

*Precondition:* ~this.IsOrdered() $\&\&$ p.IsDefined()~

*/
    template<template<typename T2> class Array2>
    bool Contains( const PointsT<Array2>& ps, const Geoid* geoid=0 ) const;
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
    bool operator==( const PointsT<ArrayT>& ) const;

    bool operator==( const Point&) const;

/*
5.4.2 Operation $\neq$ (~not equal~)

*Precondition:* ~U.IsOrdered() $\&\&$ V.IsOrdered()~

*Semantics:* $U = V$

*Complexity:* $O(n+m)$, where ~n~ is the size of ~U~ and m the size of ~V~.

*/
    bool operator!=( const PointsT<ArrayT>& ) const;
/*
5.4.3 Operation ~union~ (with ~point~)

*Precondition:* ~v.IsDefined()~

*Semantics:* $U \cup \{v\}$

*Complexity:* $O(1)$, if the set is not ordered, and $O(log(n)+n)$, otherwise,
where ~n~ is the size
of ~U~.

*/
    PointsT<ArrayT>& operator+=( const Point& p );
/*
5.4.4 Operation ~union~ (with ~points~)

*Semantics:* $U \cup V$

*Complexity:* $O(m)$, if the sets are not ordered, and $O(m+(m+n)log(m+n))$,
otherwise, where ~n~ is the size of ~U~ and ~m~ is the size of ~V~.

*/
    PointsT<ArrayT>& operator+=( const PointsT<ArrayT>& ps );
/*
5.4.5 Operation ~minus~ (with ~point~)

*Precondition:* ~U.IsOrdered() $\&\&$ v.IsDefined()~

*Semantics:* $U \backslash \{v\}$

*Complexity:* $O(log(n)+n)$

*/
    PointsT<ArrayT>& operator-=( const Point& p );
/*
5.4.6 Operation ~inside~

*Precondition:* ~U.IsOrdered() $\&\&$ V.IsOrdered()~

*Semantics:* $U \subseteq V$

*Complexity:* $O(n+m)$, where ~n~ is the size of ~U~ and m the size of ~V~.

*/
  template<template<typename T2> class Array2>
  bool Inside( const PointsT<Array2>& ps, const Geoid* geoid=0 ) const;
/*
6.4.4 Operation ~inside~ (with ~line~)

*Precondition:* ~U.IsOrdered() and V.IsOrdered()~

*Semantics:* $U \subseteq V$

*Complexity:* $O(m.n)$, where ~m~ is the size of ~U~ and ~n~ the size of ~V~.

*/
  template<template<typename T2> class Array2>
  bool Inside( const LineT<Array2>& l, const Geoid* geoid=0 ) const;
/*
6.4.4 Operation ~inside~ (with ~region~)

*Precondition:* ~U.IsOrdered() and V.IsOrdered()~

*Semantics:* $U \subseteq V$

*Complexity:* $O(m \log n)$, where ~m~ is the size of ~U~ and ~n~ the
size of ~V~.

*/
    template<template<typename T2> class Array2>
    bool Inside( const RegionT<Array2>& r, const Geoid* geoid=0 ) const;
/*
5.4.7 Operation ~intersects~ (with ~points~)

*Precondition:* ~U.IsOrdered() $\&\&$ V.IsOrdered()~

*Semantics:* $U \cap V \neq \emptyset$

*Complexity:* $O(m+n)$, where ~m~ is the size of ~U~ and ~n~ the size of ~V~.

*/
    template<template<typename T2> class Array2>
    bool Intersects( const PointsT<Array2>& ps, const Geoid* geoid=0 ) const;
/*
5.4.7 Operation ~intersects~ (with ~line~)

*Precondition:* ~U.IsOrdered() $\&\&$ V.IsOrdered()~

*Semantics:* $U \cap V \neq \emptyset$

*Complexity:* $O(m \log n)$, where ~m~ is the size of ~U~ and ~n~ 
the size of ~V~.

*/
    template<template<typename T2> class Array2>
    bool Intersects( const LineT<Array2>& l, const Geoid* geoid=0 ) const;
/*
5.4.7 Operation ~intersects~ (with ~region~)

*Precondition:* ~U.IsOrdered() $\&\&$ V.IsOrdered()~

*Semantics:* $U \cap V \neq \emptyset$

*Complexity:* $O(m \log n)$, where ~m~ is the size of ~U~ and ~n~ the size 
of ~V~.

*/
    template<template<typename T2> class Array2>
    bool Intersects( const RegionT<Array2>& r, const Geoid* geoid=0 ) const;
/*
5.4.7 Operation ~adjacent~ (with ~region~)

*Precondition:* ~U.IsOrdered() $\&\&$ V.IsOrdered()~

*Semantics:* $\partial U \cap \partial V \neq \emptyset 
\land U^0 \cap V^0 = \emptyset$

*Complexity:* $O(n.m)$, where ~n~ is the size of ~U~ and m the size of ~V~.

*/
  template<template<typename T2> class Array2>
  bool Adjacent( const RegionT<Array2>& r, const Geoid* geoid=0 ) const;
/*
5.4.8 Operation ~intersection~

*/
  template<template<typename T2> class Array2>
  void Intersection(const Point& p, PointsT<Array2>& result,
                    const Geoid* geoid=0) const;


  template<template<typename T2> class Array2,
           template<typename T3> class Array3>
  void Intersection( const PointsT<Array2>& ps, PointsT<Array3>& result,
                     const Geoid* geoid=0 ) const;
  template<template<typename T2> class Array2,
           template<typename T3> class Array3>
  void Intersection( const LineT<Array2>& l, PointsT<Array3>& result,
                     const Geoid* geoid=0 ) const;
  template<template<typename T2> class Array2,
           template<typename T3> class Array3>
  void Intersection( const RegionT<Array2>& r, PointsT<Array3>& result ,
                     const Geoid* geoid=0) const;
  template<template<typename T2> class Array2,
           template<typename T3> class Array3>
  void Intersection(const SimpleLineT<Array2>&l, PointsT<Array3>& result,
                    const Geoid* geoid=0) const;

/*
5.4.8 Operation ~minus~

*/
  
  template<template<typename T2> class Array2>
  void Minus( const Point& p, PointsT<Array2>& result, 
              const Geoid* geoid=0 ) const;

  template<template<typename T2> class Array2,
           template<typename T3> class Array3>
  void Minus( const PointsT<Array2>& ps, PointsT<Array3>& result, 
              const Geoid* geoid=0 ) const;

  template<template<typename T2> class Array2,
           template<typename T3> class Array3>
  void Minus( const LineT<Array2>& l, PointsT<Array3>& result, 
              const Geoid* geoid=0 ) const;

  template<template<typename T2> class Array2,
           template<typename T3> class Array3>
  void Minus( const RegionT<Array2>& r, PointsT<Array3>& result, 
              const Geoid* geoid=0 ) const;

  template<template<typename T2> class Array2,
           template<typename T3> class Array3>
  void Minus( const SimpleLineT<Array2>& l, PointsT<Array3>& result, 
              const Geoid* geoid=0 ) const;


/*
5.4.9 Operation ~union~

*/
  template<template<typename T2> class Array2>
  void Union(const Point& p, PointsT<Array2>& result, 
             const Geoid* geoid=0 ) const;

  template<template<typename T2> class Array2,
           template<typename T3> class Array3>
  void Union(const PointsT<Array2>& ps, PointsT<Array3>& result, 
             const Geoid* geoid=0 ) const;

  template<template<typename T2> class Array2,
           template<typename T3> class Array3>
  void Union(const LineT<Array2>& line, LineT<Array3>& result, 
             const Geoid* geoid=0 ) const;

  template<template<typename T2> class Array2,
           template<typename T3> class Array3>
  void Union(const RegionT<Array2>& region, RegionT<Array3>& result, 
             const Geoid* geoid=0 ) const;

  template<template<typename T2> class Array2,
           template<typename T3> class Array3>
  void Union(const SimpleLineT<Array2>& line, SimpleLineT<Array3>& result,
             const Geoid* geoid=0 ) const;



  double Distance( const Point& p, const Geoid* geoid=0 ) const;
/*
5.4.9 Operation ~distance~ (with ~points~)

*Precondition:* ~U.IsOrdered() and V.IsOrdered()~

*Semantics:* $\min\{ dist(u, v) | u \in U, v \in V \}$

*Complexity:* $O(m.n)$, where ~m~ is the size of ~U~ and ~n~ the size of ~V~

*/
  template<template<typename T2> class Array2>
  double Distance( const PointsT<Array2>& ps, const Geoid* geoid=0 ) const;
/*
5.4.9 Operation ~distance~ (with ~rect2~)

*Precondition:* ~U.IsOrdered() and V.IsOrdered()~


*/
  double Distance( const Rectangle<2>& r, const Geoid* geoid=0 ) const;


  bool Intersects( const Rectangle<2>& r, const Geoid* geoid=0 ) const;
  


/*
4.3.14 Operation ~translate~

*Precondition:* ~U.IsOrdered()~

*Semantics:* ~U + (x, y)~

*Complexity:* $O(n)$, where ~n~ is the size of ~U~

*/
  template<template<typename T2> class Array2>
  void Translate( const Coord& x, const Coord& y, PointsT<Array2>& ps ) const;


/*
4.3.15 Operation ~rotate~

Rotates all contained points around the point defined by (x,y) with
angle ~alpha~.

*/

  template<template<typename T2> class Array2>
  void Rotate( const Coord& x, const Coord& y, double alpha,
               PointsT<Array2>& res ) const;

/*
4.3.15 Operation ~scale~

Performes a scale transformation.

*/

  template<template<typename T2> class Array2>
  void Scale( const Coord& x, const Coord& y, PointsT<Array2>& res ) const;

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
    void SelectFirst() const;
/*
Puts the pointer ~pos~ to the first point in the ~points~ value.

*/
    void SelectNext() const;
/*
Moves the pointer ~pos~ to the next point in the ~points~ value.

*/
    bool EndOfPt() const;
/*
Decides whether ~pos~ is -1, which indicates that no more points in 
the ~points~ value need to be processed.

*/
    bool GetPt( Point& p ) const;
/*
Gets the current point from the ~points~ value according to the ~pos~ pointer.

5.6 Functions needed to import the the ~points~ data type to tuple

There are totally 10 functions which are defined as virtual functions. They need
to be defined here in order for the Point data type to be used in Tuple
definition as an attribute.

*/
    int NumOfFLOBs() const;
    Flob* GetFLOB( const int i );
    size_t Sizeof() const;
    size_t HashValue() const;
    void CopyFrom( const Attribute* right );
    int Compare( const Attribute *arg ) const;

    template<template<typename T2> class Array2>
    int Compare( const PointsT<Array2>* ) const;

    int CompareAlmost( const Attribute *arg ) const;
    template<template<typename T2> class Array2>
    int CompareAlmost( const PointsT<Array2>* ) const;

    bool Adjacent( const Attribute *arg ) const;

    virtual PointsT<ArrayT>* Clone() const;
    std::ostream& Print( std::ostream &os ) const;


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

    virtual void writeShape(std::ostream& o, uint32_t RecNo) const{

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


   virtual std::string getSQLType(){ return "MDSYS.SDO_GEOMETRY"; }
   virtual std::string getSQLRepresentation(){
       if(!IsDefined() || IsEmpty()){
         return "NULL";
       }
       return "MDSYS.SDO_GEOMETRY('" + getWKT() + "')";
   }

   std::string getWKT() const{
     std::stringstream ss;
     ss << "MULTIPOINT(";
     for(int i=0;i<Size();i++){
        if(i>0){
          ss << ", ";
        }
        Point p;
        Get(i,p);
        ss << p.GetX() << " " << p.GetY();
     }
     ss << ")";
     return ss.str();
   }

  static const std::string BasicType(){
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
    ArrayT<Point> points;
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
According to ROSE algebra, the carrier set of points should contain a 
pos pointer

*/
};



/*
5 Type Constructor ~points~

A ~points~ value is a finite set of points.

5.1 Implementation of the class ~Points~

*/
template<template<typename T>class ArrayT>
bool PointsT<ArrayT>::Find( const Point& p, int& pos, const bool& exact ) const
{
  assert( IsOrdered() );
  assert( IsDefined());
  if (exact){
    return points.Find( &p, PointCompare, pos );
  } else {
    return points.Find( &p, PointCompareAlmost, pos );
  }
}

template<template<typename T>class ArrayT>
PointsT<ArrayT>& PointsT<ArrayT>::operator=( const PointsT& ps )
{
  assert( ps.IsOrdered() );
  points.copyFrom(ps.points);
  bbox = ps.BoundingBox();
  ordered = true;
  SetDefined(ps.IsDefined());
  return *this;
}


template<template<typename T>class ArrayT>
template<template<typename T2>class ArrayT2>
PointsT<ArrayT>& PointsT<ArrayT>::operator=( const PointsT<ArrayT2>& ps )
{
  assert( ps.IsOrdered() );
  convertDbArrays(ps.points,points);
  bbox = ps.BoundingBox();
  ordered = true;
  SetDefined(ps.IsDefined());
  return *this;
}

template<template<typename T>class ArrayT>
void PointsT<ArrayT>::StartBulkLoad()
{
  ordered = false;
}

template<template<typename T>class ArrayT>
void PointsT<ArrayT>::EndBulkLoad( bool sort, bool remDup, bool trim )
{
  if( !IsDefined() ) {
    Clear();
    SetDefined( false );
  }

  if( sort ){
    Sort();
  }
  else{
    ordered = true;
  }
  if( remDup ){
    RemoveDuplicates();
  }
  if(trim){
    points.TrimToSize();
  }
}

template<template<typename T>class ArrayT>
bool PointsT<ArrayT>::operator==( const PointsT<ArrayT>& ps ) const
{

  if(!IsDefined() && !ps.IsDefined()){
    return true;
  }
  if(!IsDefined() || !ps.IsDefined()){
    return false;
  }

  if( Size() != ps.Size() )
    return false;

  if( IsEmpty() && ps.IsEmpty() )
    return true;

  if( bbox != ps.bbox )
    return false;

  assert( IsOrdered() && ps.IsOrdered() );
  object obj;
  status stat;
  SelectFirst_pp( *this, ps, obj, stat );

  while( obj == both || obj == none )
  {
    if( stat == endboth )
      return true;

    SelectNext_pp( *this, ps, obj, stat );
  }
  return false;
}

template<template<typename T>class ArrayT>
bool PointsT<ArrayT>::operator!=( const PointsT<ArrayT>& ps ) const
{
  return !( *this == ps );
}

template<template<typename T>class ArrayT>
PointsT<ArrayT>& PointsT<ArrayT>::operator+=( const Point& p )
{
  if(!IsDefined()){
    return *this;
  }
  if( !IsOrdered() )
  { // DBArray is unsorted
    if( IsEmpty() )
      bbox = p.BoundingBox();
    else
      bbox = bbox.Union( p.BoundingBox() );
    points.Append(p);
  }
  else
  { // DBArray is sorted
    int pos;
    if( !Find( p, pos, false ) )
    { // no AlmostEqual point contained
      if( IsEmpty() )
        bbox = p.BoundingBox();
      else
        bbox = bbox.Union( p.BoundingBox() );
      Find( p, pos, true ); // find exact insertion position
      Point auxp;
      for( int i = points.Size() - 1; i >= pos; --i )
      {
        points.Get( i, &auxp );
        points.Put( i+1, auxp );
      }
      points.Put( pos, p );
    } // else: do not insert
  }
  return *this;
}

template<template<typename T>class ArrayT>
PointsT<ArrayT>& PointsT<ArrayT>::operator+=( const PointsT<ArrayT>& ps )
{
  if(!IsDefined()){
    return *this;
  }
  if(!ps.IsDefined()){
    SetDefined(false);
    Clear();
    return *this;
  }

  if( IsEmpty() )
    bbox = ps.BoundingBox();
  else
    bbox = bbox.Union( ps.BoundingBox() );

  if( !IsOrdered() )
  {
    if((int)points.GetCapacity() < (points.Size() + ps.Size())){
       points.resize( Size() + ps.Size() );
    }
    Point p;
    for( int i = 0; i < ps.Size(); i++ )
    {
      ps.Get( i, p );
      points.Append( p );
    }
  }
  else
  {
    PointsT<ArrayT> newPs( Size() + ps.Size() );
    Union( ps, newPs );
    *this = newPs;
  }
  return *this;
}

template<template<typename T>class ArrayT>
PointsT<ArrayT>& PointsT<ArrayT>::operator-=( const Point& p )
{
  if( !IsDefined() ) {
    assert( IsDefined() );
    return *this;
  }
  if( !p.IsDefined() ) {
    assert( p.IsDefined() );
    Clear();
    SetDefined( false );
    return *this;
  }
  assert( IsOrdered() );
  int pos, posLow, posHigh;
  if( Find( p, pos, false ) )
  { // found an AlmostEqual point
    Point auxp;
    posLow = pos;
    while(posLow > 0)
    { // find first pos with AlmostEqual point
      points.Get( posLow-1, &auxp );
      if( AlmostEqual(p, auxp) )
      { posLow--; }
      else
      { break; }
    }
    posHigh = pos;
    while(posHigh < Size())
    { // find last pos with AlmostEqual point
      points.Get( posHigh+1, &auxp );
      if( AlmostEqual(p, auxp) )
      { posHigh++; }
      else
      { break; }
    }
    for( int i = 0; i < Size()-posHigh; i++ )
    { // keep smaller and move down bigger points
      points.Get( posHigh+i, &auxp );
      points.Put( posLow+i, auxp );
    }
    points.resize( Size()-(1+posHigh-posLow) );

    // Naive way to redo the bounding box.
    if( IsEmpty() )
      bbox.SetDefined( false );
    int i = 0;
    points.Get( i++, &auxp );
    bbox = auxp.BoundingBox();
    for( ; i < Size(); i++ )
    {
      points.Get( i, &auxp );
      bbox = bbox.Union( auxp.BoundingBox() );
    }
  }
  return *this;
}

#endif
