/*
----
This file is part of SECONDO.

Copyright (C) 2017, University in Hagen, Department of Computer Science,
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


#include "PointsT.h"
#include "RegionT.h"
#include "SimpleLineT.h"
#include "SpatialAlgebra.h"
#include "Tools/Flob/MMDbArray.h"


template<template<typename T>class Array>
void PointsT<Array>::Sort(const bool exact /*= true*/)
{
  assert( !IsOrdered() );
  if(exact){
      points.Sort( PointCompare );
  } else{
      points.Sort(PointCompareAlmost);
  }
  ordered = true;
}


/*
Function supporting the RemoveDuplicates function.
This function checks whether in an array of points
a point exists which is AlmostEqual to the given one.
The search is restricted to the range in array given
by the indices __min__ and __max__.

*/
template<template<typename T>class Array>
bool AlmostContains( const Array<Point>& points, const Point& p,
                     int min, int max, int size){

  if(min>max){
     return false;
  }
  Point pa;
  if(min==max){ // search around the position found
     // search left of min
     int pos = min;
     double x = p.GetX();
     points.Get(pos,&pa);
     while(pos>=0 && AlmostEqual(pa.GetX(),x)){
        if(AlmostEqual(pa,p)){
           return true;
        }
        pos--;
        if(pos>=0){
          points.Get(pos,&pa);
        }
     }
     // search right of min
     pos=min+1;
     if(pos<size){
        points.Get(pos,&pa);
     }
     while(pos<size &&AlmostEqual(pa.GetX(),x)){
        if(AlmostEqual(pa,p)){
          return  true;
        }
        pos++;
        if(pos<size){
           points.Get(pos,&pa);
        }
    }
    return false; // no matching point found
  } else {
      int mid = (min+max)/2;
      points.Get(mid,&pa);
      double x = pa.GetX();
      double cx = p.GetX();
      if(AlmostEqual(x,cx)){
         return AlmostContains(points,p,mid,mid,size);
      } else if(cx<x){
         return AlmostContains(points,p,min,mid-1,size);
      }else {
         return AlmostContains(points,p,mid+1,max,size);
      }
  }
}



template<template<typename T>class Array>
void PointsT<Array>::RemoveDuplicates()
{
 assert(IsOrdered());
 //Point allPoints[points.Size()];
 Array<Point> allPoints(points.Size());
 Point p;
 for(int i=0;i<points.Size();i++){
    points.Get(i,p);
    bool found = AlmostContains(allPoints,p,0,
                                allPoints.Size()-1,
                                allPoints.Size());
    if(!found){
      allPoints.Append(p);
    }
 }

 if(allPoints.Size()!=Size()){
     points.clean();
     for(int i=0;i < allPoints.Size(); i++){
        allPoints.Get(i,p);
        points.Append(p);
     }
 }
 allPoints.destroy();
}

template<template<typename T>class Array>
bool PointsT<Array>::Contains( const Point& p, const Geoid* geoid /*=0*/ ) const
{
  assert( IsDefined() );
  assert( p.IsDefined() );
  assert( IsOrdered() );
  assert( !geoid || geoid->IsDefined() );

  if( IsEmpty() )
    return false;

  if( !p.Inside( bbox, geoid ) )
    return false;

  int pos;
  return Find( p, pos, false ); // find using AlmostEqual
}

template<template<typename T>class Array>
bool PointsT<Array>::Contains( const PointsT<Array>& ps, 
                               const Geoid* geoid /*=0*/ ) const
{
  assert( IsDefined() );
  assert( IsOrdered() );
  assert( ps.IsDefined() );
  assert( ps.IsOrdered() );
  assert( !geoid || geoid->IsDefined() );

  if(!IsDefined() || !ps.IsDefined() || (geoid && !geoid->IsDefined()) ){
    return false;
  }

  if( IsEmpty() && ps.IsEmpty() )
    return true;

  if( IsEmpty() || ps.IsEmpty() )
    return false;

  if( !bbox.Contains( ps.BoundingBox(), geoid ) )
    return false;

  object obj;
  status stat;
  SelectFirst_pp( *this, ps, obj, stat );

  while( stat != endsecond && stat != endboth )
  {
    if( obj == second || stat == endfirst )
      return false;

    SelectNext_pp( *this, ps, obj, stat );
  }
  return true;
}

template<template<typename T>class Array>
bool PointsT<Array>::Inside( const PointsT<Array>& ps, 
                             const Geoid* geoid /*=0*/ ) const
{
  assert( IsDefined() );
  assert( IsOrdered() );
  assert( ps.IsDefined() );
  assert( ps.IsOrdered() );
  assert( !geoid || geoid->IsDefined() );
  return ps.Contains( *this, geoid );
}

template<template<typename T>class Array>
bool PointsT<Array>::Inside( const LineT<Array>& l, 
                             const Geoid* geoid /*=0*/ ) const
{
  assert( IsDefined() );
  assert( IsOrdered() );
  assert( l.IsDefined() );
  assert( l.IsOrdered() );
  assert( !geoid || geoid->IsDefined() );

  if( IsEmpty() )
    return true;

  if( l.IsEmpty() )
    return false;

  if( !l.BoundingBox().Contains( bbox, geoid ) )
    return false;

  Point p;
  for( int i = 0; i < Size(); i++ )
  {
    Get( i, p );
    if( !l.Contains( p, geoid ) )
      return false;
  }
  return true;
}

template<template<typename T>class Array>
bool PointsT<Array>::Inside( const RegionT<Array>& r, 
                             const Geoid* geoid /*=0*/ ) const
{
  assert( IsDefined() );
  assert( IsOrdered() );
  assert( r.IsDefined() );
  assert( r.IsOrdered() );
  assert( !geoid || geoid->IsDefined() );

  if(geoid){
    cout << __PRETTY_FUNCTION__ << ": Spherical geometry not implemented."
         <<endl;
    assert(false); // TODO: Implement spherical geometry case.
  }

  if( IsEmpty() )
    return true;

  if( r.IsEmpty() )
    return false;

  if( !r.BoundingBox().Contains( bbox, geoid ) )
    return false;

  Point p;
  for( int i = 0; i < Size(); i++ )
  {
    Get( i, p );
    if( !r.Contains( p, geoid ) )
      return false;
  }
  return true;
}

template<template<typename T>class Array>
bool PointsT<Array>::Intersects( const PointsT<Array>& ps, 
                                 const Geoid* geoid /*=0*/ ) const
{
  assert( IsDefined() );
  assert( IsOrdered() );
  assert( ps.IsDefined() );
  assert( ps.IsOrdered() );
  assert( !geoid || geoid->IsDefined() );

  if( IsEmpty() || ps.IsEmpty() || (geoid && !geoid->IsDefined()) )
    return false;

  if( !bbox.Intersects( ps.BoundingBox(), geoid ) )
    return false;

  object obj;
  status stat;
  SelectFirst_pp( *this, ps, obj, stat );

  while( stat != endboth )
  {
    if( obj == both )
      return true;
    SelectNext_pp( *this, ps, obj, stat );
  }
  return false;
}

template<template<typename T>class Array>
bool PointsT<Array>::Intersects( const LineT<Array>& l, 
                                 const Geoid* geoid /*=0*/ ) const
{
  assert( IsDefined() );
  assert( IsOrdered() );
  assert( l.IsDefined() );
  assert( l.IsOrdered() );
  assert( !geoid || geoid->IsDefined() );

  if(geoid){
    cout << __PRETTY_FUNCTION__ << ": Spherical geometry not implemented."
         <<endl;
    assert(false); // TODO: Implement spherical geometry case.
  }

  if( IsEmpty() || l.IsEmpty() || (geoid && !geoid->IsDefined()))
    return false;

  if( !BoundingBox().Intersects( l.BoundingBox(), geoid ) )
    return false;

  Point p;
  for( int i = 0; i < Size(); i++ )
  {
    Get( i, p );
    if( l.Contains( p, geoid ) )
      return true;
  }

  return false;
}

template<template<typename T>class Array>
bool PointsT<Array>::Intersects( const RegionT<Array>& r, 
                                 const Geoid* geoid /*=0*/ ) const
{
  assert( IsDefined() );
  assert( IsOrdered() );
  assert( r.IsDefined() );
  assert( r.IsOrdered() );
  assert( !geoid || geoid->IsDefined() );

  if(geoid){
    cout << __PRETTY_FUNCTION__ << ": Spherical geometry not implemented."
         <<endl;
    assert(false); // TODO: Implement spherical geometry case.
  }

  if( IsEmpty() || r.IsEmpty() || (geoid && !geoid->IsDefined()) )
    return false;

  if( !BoundingBox().Intersects( r.BoundingBox(), geoid ) )
    return false;

  Point p;
  for( int i = 0; i < Size(); i++ )
  {
    Get( i, p );
    if( r.Contains( p, geoid ) )
      return true;
  }
  return false;
}

template<template<typename T>class Array>
bool PointsT<Array>::Adjacent( const RegionT<Array>& r, 
                               const Geoid* geoid /*=0*/ ) const
{
  assert( IsDefined() );
  assert( IsOrdered() );
  assert( r.IsDefined() );
  assert( r.IsOrdered() );
  assert( !geoid || geoid->IsDefined() );

  if(geoid){
    cout << __PRETTY_FUNCTION__ << ": Spherical geometry not implemented."
         <<endl;
    assert(false); // TODO: Implement spherical geometry case.
  }

  if( IsEmpty() || r.IsEmpty() || (geoid && !geoid->IsDefined()))
    return false;

  if( !BoundingBox().Intersects( r.BoundingBox(), geoid ) )
    return false;

  Point p;
  bool found = false;
  for( int i = 0; i < Size(); i++ )
  {
    Get( i, p );

    if( !r.Contains( p, geoid ) )
      continue;

    // At least one point is contained in the region
    // If it is not inside the region, then the
    // function will return true.
    found = true;
    if( r.InnerContains( p, geoid ) )
      return false;
  }
  return found;
}

template<template<typename T>class Array>
void PointsT<Array>::Intersection(const Point& p, PointsT<Array>& result,
                          const Geoid* geoid /*=0*/) const{
   result.Clear();
   if(!IsDefined() || ! p.IsDefined() || (geoid && !geoid->IsDefined()) ){
     result.SetDefined(false);
     return;
   }
   if(this->Contains(p, geoid)){
      result += p;
   }
}

template<template<typename T>class Array>
void PointsT<Array>::Intersection( const PointsT<Array>& ps, 
                                   PointsT<Array>& result,
                                   const Geoid* geoid /*=0*/ ) const
{
  result.Clear();
  if( !IsDefined() || !ps.IsDefined() || (geoid && !geoid->IsDefined()) ) {
    result.SetDefined( false );
    return;
  }
  result.SetDefined( true );
  assert( ordered );
  assert( ps.ordered );

  if( IsEmpty() || ps.IsEmpty() ){
    return;
  }

  object obj;
  status stat;
  Point p;
  SelectFirst_pp( *this, ps, obj, stat );

  result.StartBulkLoad();
  while( stat != endboth )
  {
    if( obj == both )
    {
      int GotPt = ps.GetPt( p );
      assert( GotPt );
      result += p;
    }
    SelectNext_pp( *this, ps, obj, stat );
  }
  result.EndBulkLoad( false, false );
}

template<template<typename T>class Array>
void PointsT<Array>::Intersection( const LineT<Array>& l, 
                                   PointsT<Array>& result,
                           const Geoid* geoid /*=0*/ ) const
{
  result.Clear();
  if( !IsDefined() || !l.IsDefined() || (geoid && !geoid->IsDefined()) ) {
    result.SetDefined( false );
    return;
  }
  result.SetDefined( true );
  assert( IsOrdered() );
  assert( l.IsOrdered() );

  if( IsEmpty() || l.IsEmpty() )
    return;

  Point p;
  result.StartBulkLoad();
  for( int i = 0; i < Size(); i++ )
  {
    Get( i, p );
    if( l.Contains( p, geoid ) )
      result += p;
  }
  result.EndBulkLoad( false, false );
}

template<template<typename T>class Array>
void PointsT<Array>::Intersection( const RegionT<Array>& r, 
                                   PointsT<Array>& result,
                                   const Geoid* geoid /*=0*/ ) const
{
  result.Clear();
  if( !IsDefined() || !r.IsDefined() || (geoid && !geoid->IsDefined()) ) {
    result.SetDefined( false );
    return;
  }
  result.SetDefined( true );
  assert( IsOrdered() );
  assert( r.IsOrdered() );

  if( IsEmpty() || r.IsEmpty() )
    return;

  Point p;
  result.StartBulkLoad();
  for( int i = 0; i < Size(); i++ )
  {
    Get( i, p );
    if( r.Contains( p, geoid ) )
      result += p;
  }
  result.EndBulkLoad( false, false );
}

template<template<typename T>class Array>
void PointsT<Array>::Intersection( 
                           const SimpleLineT<Array>& l, 
                           PointsT<Array>& result,
                           const Geoid* geoid /*=0*/ ) const
{
  result.Clear();
  if( !IsDefined() || !l.IsDefined() || (geoid && !geoid->IsDefined()) ) {
    result.SetDefined( false );
    return;
  }
  result.SetDefined( true );
  assert( IsOrdered() );
  assert( l.IsOrdered() );

  if( IsEmpty() || l.IsEmpty() )
    return;

  Point p;
  result.StartBulkLoad();
  for( int i = 0; i < Size(); i++ )
  {
    Get( i, p );
    if( l.Contains( p, geoid ) )
      result += p;
  }
  result.EndBulkLoad( false, false );
}

template<template<typename T>class Array>
void PointsT<Array>::Minus( const Point& p, PointsT<Array>& ps,
                    const Geoid* geoid /*=0*/ ) const
{
  ps.Clear();
  if( !IsDefined() || !p.IsDefined() || (geoid && !geoid->IsDefined()) ) {
    ps.SetDefined( false );
    return;
  }
  ps.SetDefined( true );

  assert( ordered );
  ps.StartBulkLoad();
  Point pi;
  for( int i = 0; i < Size(); i++ ) {
    Get( i, pi );
    if( !AlmostEqual(pi, p) )
      ps += pi;
  }
  ps.EndBulkLoad( false, false );
}

template<template<typename T>class Array>
void PointsT<Array>::Minus( const PointsT<Array>& ps, PointsT<Array>& result,
                    const Geoid* geoid /*=0*/ ) const
{
  result.Clear();
  if( !IsDefined() || !ps.IsDefined() || (geoid && !geoid->IsDefined()) ) {
    result.SetDefined( false );
    return;
  }
  result.SetDefined( true );
  assert( ordered );
  assert( ps.ordered );

  result.StartBulkLoad();
  int size1 = this->Size();
  int size2 = ps.Size();
  int pos1 = 0, pos2 = 0;
  Point p1, p2;

  while(pos1<size1 && pos2<size2) {
    this->Get(pos1, p1);
    ps.Get(pos2, p2);
    if( AlmostEqual(p1, p2) ) {
      pos1++;
    }
    else if (p1 < p2) {
      result += p1;
      pos1++;
    } else { // *p1 > *p2
      pos2++;
    }
  }
  while(pos1<size1) {
    this->Get(pos1, p1);
    result += p1;
    pos1++;
  }
  result.EndBulkLoad( false, false );
}

template<template<typename T>class Array>
void PointsT<Array>::Minus( const LineT<Array>& l, PointsT<Array>& result,
                    const Geoid* geoid /*=0*/ ) const
{
  result.Clear();
  if( !IsDefined() || !l.IsDefined() || (geoid && !geoid->IsDefined()) ) {
    result.SetDefined( false );
    return;
  }
  result.SetDefined( true );
  assert( IsOrdered() );
  assert( l.IsOrdered() );

  Point p;
  result.StartBulkLoad();
  for( int i = 0; i < Size(); i++ )
  {
    Get( i, p );
    if( !l.Contains( p, geoid ) )
      result += p;
  }
  result.EndBulkLoad( false, false );
}

template<template<typename T>class Array>
void PointsT<Array>::Minus( const RegionT<Array>& r, PointsT<Array>& result,
                    const Geoid* geoid /*=0*/ ) const
{
  result.Clear();
  if( !IsDefined() || !r.IsDefined() || (geoid && !geoid->IsDefined())) {
    result.SetDefined( false );
    return;
  }
  result.SetDefined( true );
  assert( IsOrdered() );
  assert( r.IsOrdered() );

  Point p;
  result.StartBulkLoad();
  for( int i = 0; i < Size(); i++ )
  {
    Get( i, p );
    if( !r.Contains( p, geoid ) )
      result += p;
  }
  result.EndBulkLoad( false, false );
}

template<template<typename T>class Array>
void PointsT<Array>::Minus( const SimpleLineT<Array>& l, PointsT<Array>& result,
                    const Geoid* geoid /*=0*/ ) const
{
  result.Clear();
  if( !IsDefined() || !l.IsDefined() || (geoid && !geoid->IsDefined()) ) {
    result.SetDefined( false );
    return;
  }
  result.SetDefined( true );
  assert( IsOrdered() );
  assert( l.IsOrdered() );

  Point p;
  result.StartBulkLoad();
  for( int i = 0; i < Size(); i++ )
  {
    Get( i, p );
    if( !l.Contains( p, geoid ) )
      result += p;
  }
  result.EndBulkLoad( false, false );
}

template<template<typename T>class Array>
void PointsT<Array>::Union( const Point& p, PointsT<Array>& result,
                    const Geoid* geoid /*=0*/ ) const
{
  result.Clear();
  if( !IsDefined() || !p.IsDefined() || (geoid && !geoid->IsDefined()) ) {
    result.SetDefined( false );
    return;
  }
  result.SetDefined( true );
  assert( ordered );

  result.StartBulkLoad();
  Point pi;
  bool inserted = false;
  for( int i = 0; i < Size(); i++)
  {
    Get( i, pi );

    if( !inserted && pi == p )
      inserted = true;

    if( !inserted && pi > p )
    {
      result += p;
      inserted = true;
    }
    result += pi;
  }
  if( !inserted )
    result += p;

  result.EndBulkLoad( false, true );
}

template<template<typename T>class Array>
void PointsT<Array>::Union( const PointsT<Array>& ps, 
                                  PointsT<Array>& result,
                    const Geoid* geoid /*=0*/ ) const
{
  result.Clear();
  if( !IsDefined() || !ps.IsDefined() || (geoid && !geoid->IsDefined()) ) {
    result.SetDefined( false );
    return;
  }
  result.SetDefined( true );
  assert( ordered );
  assert( ps.ordered );

  object obj;
  status stat;
  SelectFirst_pp( *this, ps, obj, stat );
  Point p;

  result.StartBulkLoad();
  while( stat != endboth )
  {
    if( obj == first || obj == both )
    {
      int GotPt = GetPt( p );
      assert( GotPt );
    }
    else if( obj == second )
    {
      int GotPt = ps.GetPt( p );
      assert( GotPt );
    }
    result += p;
    SelectNext_pp( *this, ps, obj, stat );
  }
  result.EndBulkLoad( false, true );
}


template<template<typename T>class Array>
void PointsT<Array>::Union( const LineT<Array>& line, LineT<Array>& result,
                    const Geoid* geoid /*=0*/ ) const{
   line.Union(*this,result,geoid);
}

template<template<typename T>class Array>
void PointsT<Array>::Union( const RegionT<Array>& region, 
                            RegionT<Array>& result,
                            const Geoid* geoid /*=0*/) const{
   region.Union(*this,result,geoid);
}

template<template<typename T>class Array>
void PointsT<Array>::Union(const SimpleLineT<Array>& line, 
                           SimpleLineT<Array>& result,
                           const Geoid* geoid /*=0*/) const{
  line.Union(*this,result, geoid);
}

template<template<typename T>class Array>
double PointsT<Array>::Distance( const Point& p, 
                                 const Geoid* geoid /* = 0 */ ) const
{
  assert( !IsEmpty() );
  assert( p.IsDefined() );
  assert( !geoid || geoid->IsDefined() );

  double result = std::numeric_limits<double>::max();
  for( int i = 0; i < Size(); i++ ){
    Point pi;
    Get( i, pi );
    if( AlmostEqual( p, pi ) ){
      return 0.0;
    }
    result = MIN( result, pi.Distance( p, geoid ) );
  }
  return result;
}

template<template<typename T>class Array>
double PointsT<Array>::Distance( const PointsT<Array>& ps, 
                                 const Geoid* geoid /* = 0 */ ) const
{
  assert( !IsEmpty() );
  assert( !ps.IsEmpty() );
  assert( !geoid || geoid->IsDefined() );

  double result = std::numeric_limits<double>::max();
  Point pi, pj;
  for( int i = 0; i < Size(); i++ ){
    Get( i, pi );
    for( int j = 0; j < ps.Size(); j++ ){
      ps.Get( j, pj );
      if( AlmostEqual( pi, pj ) ){
        return 0.0;
      }
      result = MIN( result, pi.Distance( pj, geoid ) );
    }
  }
  return result;
}

template<template<typename T>class Array>
double PointsT<Array>::Distance( const Rectangle<2>& r, 
                                 const Geoid* geoid/*=0*/ ) const
{
  assert( IsDefined() );
  assert( !IsEmpty() );
  assert( r.IsDefined() );
  assert( !geoid || geoid->IsDefined() );
  double result = std::numeric_limits<double>::max();
  Point pi;
  for( int i = 0; i < Size(); i++ )
  {
    Get( i, pi );
    result = MIN( result, pi.Distance( r, geoid ) );
  }
  return result;
}

template<template<typename T>class Array>
bool PointsT<Array>::Intersects( const Rectangle<2>& r, 
                                 const Geoid* geoid/*=0*/ ) const
{
  assert( IsDefined() );
  assert( !IsEmpty() );
  assert( r.IsDefined() );
  assert( !geoid || geoid->IsDefined() );
  if(!BoundingBox().Intersects(r,geoid)){
     return false;
  }
  Point pi;
  for( int i = 0; i < Size(); i++ )
  {
    Get( i, pi );
    if(pi.Intersects(r,geoid)){
      return true;
    } 
  }
  return false;
}

template<template<typename T>class Array>
void PointsT<Array>::Translate( const Coord& x, const Coord& y, 
                                PointsT<Array>& result ) const
{
  result.Clear();
  if( !IsDefined() ) {
    result.SetDefined( false );
    return;
  }
  result.SetDefined( true );

  assert( ordered );
  result.StartBulkLoad();
  Point p;
  for( int i = 0; i < Size(); i++ )
  {
    Get( i, p );
    p.Translate( x, y );
    result += p;
  }
  result.EndBulkLoad( false, false );
}


template<template<typename T>class Array>
void PointsT<Array>::Scale( const Coord& x, const Coord& y, 
                            PointsT<Array>& result ) const
{
  result.Clear();
  if( !IsDefined() ) {
    result.SetDefined( false );
    return;
  }
  result.SetDefined( true );
  result.Resize(Size());
  result.StartBulkLoad();
  Point p;
  for( int i = 0; i < Size(); i++ )
  {
    Get( i, p );
    p.Scale( x, y );
    result += p;
  }
  result.EndBulkLoad( true, true );
}

template<template<typename T>class Array>
void PointsT<Array>::Rotate( const Coord& x, const Coord& y,
                     const double alpha,
                     PointsT<Array>& result ) const
{
  result.Clear();
  if(!IsDefined()){
     result.SetDefined(false);
     return;
  }
  result.SetDefined(true);
  result.Resize(Size());

  double s = sin(alpha);
  double c = cos(alpha);

  double m00 = c;
  double m01 = -s;
  double m02 = x - x*c + y*s;
  double m10 = s;
  double m11 = c;
  double m12 = y - x*s-y*c;


  result.StartBulkLoad();
  Point p;
  Point rot(true,0,0);

  for( int i = 0; i < Size(); i++ )
  {
    Get( i, p );
    rot.Set( m00*p.GetX() + m01*p.GetY() + m02,
             m10*p.GetX() + m11*p.GetY() + m12);
    result += rot;
  }
  result.EndBulkLoad( true, false );

}

template<template<typename T>class Array>
Point PointsT<Array>::theCenter() const{
   Point res(true,0,0);
   if(!IsDefined() || (Size()==0)){
     res.SetDefined(false);
   } else {
     int size = Size();
     Point p;
     double x = 0.0;
     double y = 0.0;
     for(int i=0;i<size;i++){
         Get(i,p);
         x += p.GetX();
         y += p.GetY();
     }
     res.Set(x/size,y/size);
   }
   return res;
}



template<template<typename T>class Array>
size_t PointsT<Array>::HashValue() const
{
  if( IsEmpty() ) // IsEmpty() includes undef
  return 0;

  size_t h = 0;

  Point p;
  for( int i = 0; i < Size() && i < 5; i++ )
  {
    Get( i, p );
    h = h + (size_t)(5 * p.GetX() + p.GetY());
  }
  return h;
}

template<template<typename T>class Array>
bool PointsT<Array>::IsValid() const
{
  if( IsEmpty() ) // IsEmpty() includes undef
    return true;

  Point p1, p2;
  Get( 0, p1 );
  if( !p1.IsDefined() ){
    std::cerr << __PRETTY_FUNCTION__ << ": Undefined Point!" << std::endl;
    std::cerr << "\tp1 = "; p1.Print(std::cerr); std::cerr << std::endl;
    return false;
  }
  for( int i = 1; i < Size(); i++ )
  {
    Get( i, p2 );
    if( !p2.IsDefined() ){
      std::cerr << __PRETTY_FUNCTION__ << ": Undefined Point!" << std::endl;
      std::cerr << "\tp2 = "; p2.Print(std::cerr); std::cerr << std::endl;
      return false;
    }
    if( AlmostEqual( p1, p2 ) ){
      std::cerr << __PRETTY_FUNCTION__ << ": Almost equal Points!" << endl;
      std::cerr << "\tp1 = "; p1.Print(std::cerr);
      std::cerr << "\n\tp2 = "; p2.Print(std::cerr); std::cerr << std::endl;
      return false;
    }
    p1 = p2;
  }
  return true;
}

template<template<typename T>class Array>
void PointsT<Array>::Clear()
{
  points.clean();
  pos = -1;
  ordered = true;
  bbox.SetDefined( false );
}

template<template<typename T>class Array>
void PointsT<Array>::CopyFrom( const Attribute* right )
{
  const PointsT<Array> *ps = (const PointsT<Array>*)right;
  assert( ps->IsOrdered() );
  *this = *ps;
}

template<template<typename T>class Array>
int PointsT<Array>::Compare( const Attribute* arg ) const
{
  const PointsT<Array>* ps = (const PointsT<Array>*)arg;

  if( !ps )
    return (-2);

  if(!IsDefined() && !ps->IsDefined()){
    return 0;
  }
  if(!IsDefined()){
    return -1;
  }
  if(!ps->IsDefined()){
    return 1;
  }

  if( IsEmpty() && ps->IsEmpty() )
    return 0;

  if( IsEmpty() )
    return -1;

  if( ps->IsEmpty() )
    return 1;

  if( Size() > ps->Size() )
    return 1;

  if( Size() < ps->Size() )
    return -1;

  Point p1, p2;
  for( int i = 0; i < Size(); i++ )
  {
    Get( i, p1);
    ps->Get( i, p2 );

    if( p1 > p2 )
      return 1;

    if( p1 < p2 )
      return -1;
  }
  return 0;
}

template<template<typename T>class Array>
int PointsT<Array>::CompareAlmost( const Attribute* arg ) const
{
  const PointsT<Array>* ps = (const PointsT<Array>*)arg;

  if( !ps )
    return (-2);


  if(!IsDefined() && !ps->IsDefined()){
    return 0;
  }
  if(!IsDefined()){
    return -1;
  }
  if(!ps->IsDefined()){
    return 1;
  }

  if( IsEmpty() && ps->IsEmpty() )
    return 0;

  if( IsEmpty() )
    return -1;

  if( ps->IsEmpty() )
    return 1;

  if( Size() > ps->Size() )
    return 1;

  if( Size() < ps->Size() )
    return -1;

  Point p1, p2;
  for( int i = 0; i < Size(); i++ )
  {
    Get( i, p1);
    ps->Get( i, p2 );

    if( !AlmostEqual(p1, p2) )
    {
      if( p1 > p2 )
        return 1;
      if( p1 < p2 )
        return -1;
    }
  }
  return 0;
}


template<template<typename T>class Array>
bool PointsT<Array>::Adjacent( const Attribute* arg ) const
{
  return 0;
  // for points which takes double values, we can not decide whether they are
  //adjacent or not.
}

template<template<typename T>class Array>
PointsT<Array>* PointsT<Array>::Clone() const
{
  return new PointsT<Array>( *this );
}

template<template<typename T>class Array>
std::ostream& PointsT<Array>::Print( std::ostream &os ) const
{
  return os << *this;
}
/*
11.2 Class ~Points~

*/
template<template<typename T>class Array>
inline PointsT<Array>::PointsT( const int initsize ) :
StandardSpatialAttribute<2>(true),
points( initsize ),
bbox( false ),
ordered( true )
{ }


template<template<typename T>class Array>
inline PointsT<Array>::PointsT( const PointsT<Array>& ps ) :
StandardSpatialAttribute<2>(ps.IsDefined()),
points( ps.Size() ),
bbox( ps.BoundingBox() ),
ordered( true )
{
  if( IsDefined() ) {
    assert( ps.IsOrdered() );
    convertDbArrays<Point>(ps.points,points);
  }
}

template<template<typename T>class Array>
inline const Rectangle<2> 
PointsT<Array>::BoundingBox(const Geoid* geoid /*=0*/) const
{
  // no special implementation for spherical geometry required!
  return bbox;
}

template<template<typename T>class Array>
inline bool PointsT<Array>::Get( const int i, Point& p ) const
{
  assert( IsDefined() );
  return points.Get( i, &p );
}

template<template<typename T>class Array>
inline int PointsT<Array>::Size() const
{
  return points.Size();
}

template<template<typename T>class Array>
inline bool PointsT<Array>::IsEmpty() const
{
  return !IsDefined() || (points.Size() == 0);
}

template<template<typename T>class Array>
inline bool PointsT<Array>::IsOrdered() const
{
  return ordered;
}

template<template<typename T>class Array>
inline int PointsT<Array>::NumOfFLOBs() const
{
  return 1;
}

template<template<typename T>class Array>
inline bool PointsT<Array>::operator==(const Point& p) const{
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


template<template<typename T>class Array>
inline Flob *PointsT<Array>::GetFLOB(const int i)
{
  assert( i >= 0 && i < NumOfFLOBs() );
  return &points;
}


template<template<typename T>class Array>
inline size_t PointsT<Array>::Sizeof() const
{
  return sizeof( *this );
}

template<template<typename T>class Array>
inline void PointsT<Array>::SelectFirst() const
{
  if( IsEmpty() ){
    pos = -1;
  } else {
    pos = 0;
  }
}

template<template<typename T>class Array>
inline void PointsT<Array>::SelectNext() const
{
  if( pos >= 0 && pos < Size() - 1 ) {
    pos++;
  } else {
    pos = -1;
  }
}

template<template<typename T>class Array>
inline bool PointsT<Array>::EndOfPt() const
{
  return pos == -1;
}

template<template<typename T>class Array>
inline bool PointsT<Array>::GetPt(Point& p ) const
{
  if( pos >= 0 && pos <= Size()-1 ){
    points.Get( pos, &p );
    return true;
  }
  return false;
}

template<template<typename T>class Array>
inline void PointsT<Array>::Resize(const int newSize){
  if(newSize>Size()){
    points.resize(newSize);
  }
}

template<template<typename T>class Array>
inline void PointsT<Array>::TrimToSize(){
  points.TrimToSize();
}


template<template<typename T>class Array>
void SelectFirst_pp( const PointsT<Array>& P1, 
                     const PointsT<Array>& P2,
                     object& obj, status& stat )
{
  P1.SelectFirst();
  P2.SelectFirst();

  Point p1, p2;
  bool gotP1 = P1.GetPt( p1 ),
       gotP2 = P2.GetPt( p2 );

  if( !gotP1 && !gotP2 )
  {
    obj = none;
    stat = endboth;
  }
  else if( !gotP1 )
  {
    obj = second;
    stat = endfirst;
  }
  else if( !gotP2 )
  {
    obj = first;
    stat = endsecond;
  }
  else //both defined
  {
    stat = endnone;
    if( p1 < p2 )
      obj = first;
    else if( p1 > p2 )
      obj = second;
    else
      obj = both;
  }
}

template<template<typename T>class Array>
void SelectNext_pp( const PointsT<Array>& P1, 
                    const PointsT<Array>& P2,
                    object& obj, status& stat )
{
  // 1. get the current elements
  Point p1, p2;
  bool gotP1 = P1.GetPt( p1 ),
       gotP2 = P2.GetPt( p2 );

  //2. move the pointers
  if( !gotP1 && !gotP2 )
  {
    //do nothing
  }
  else if( !gotP1 )
  {
    P2.SelectNext();
    gotP2 = P2.GetPt( p2 );
  }
  else if( !gotP2 )
  {
    P1.SelectNext();
    gotP1 = P1.GetPt( p1 );
  }
  else //both currently defined
  {
    if( p1 < p2 ) //then hs1 is the last output
    {
      P1.SelectNext();
      gotP1 = P1.GetPt( p1 );
    }
    else if( p1 > p2 )
    {
      P2.SelectNext();
      gotP2 = P2.GetPt( p2 );
    }
    else
    {
      P1.SelectNext();
      gotP1 = P1.GetPt( p1 );
      P2.SelectNext();
      gotP2 = P2.GetPt( p2 );
    }
  }

  //3. generate the outputs
  if( !gotP1 && !gotP2 )
  {
    obj = none;
    stat = endboth;
  }
  else if( !gotP1 )
  {
    obj = second;
    stat = endfirst;
  }
  else if( !gotP2 )
  {
    obj = first;
    stat = endsecond;
  }
  else //both defined
  {
    stat = endnone;
    if( p1 < p2 )
      obj = first;
    else if( p1 > p2 )
      obj = second;
    else
      obj = both;
  }
}




// explicit instantiations

  /*

template class PointsT<DbArray > ;
template class PointsT<MMDbArray >;


template void SelectFirst_pp<DbArray>( 
                     const PointsT<DbArray>& P1, 
                     const PointsT<DbArray>& P2,
                     object& obj, status& stat );

template void SelectFirst_pp<MMDbArray>( 
                     const PointsT<MMDbArray>& P1, 
                     const PointsT<MMDbArray>& P2,
                     object& obj, status& stat );


template void SelectNext_pp<DbArray>( 
                    const PointsT<DbArray>& P1, 
                    const PointsT<DbArray>& P2,
                    object& obj, status& stat );

template void SelectNext_pp<MMDbArray>( 
                    const PointsT<MMDbArray>& P1, 
                    const PointsT<MMDbArray>& P2,
                    object& obj, status& stat );

 */
