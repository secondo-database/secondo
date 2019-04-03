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



#include "SimpleLineT.h"
#include "LineT.h"
#include "AVLSegment.h"
#include "../../Tools/Flob/MMDbArray.h"
#include "SpatialAlgebra.h"

/*
7 The type SimpleLineT

7.1 Constructor

This constructor coinstructs a simple line from ~src~

If ~src~ is not simple, the simple line will be invalidated, i.e.
the defined flag is set to false;

*/
template<template<typename T> class Array>
template<template<typename T2> class Array2>
SimpleLineT<Array>::SimpleLineT(const LineT<Array2>& src):
     StandardSpatialAttribute<2>(src.IsDefined()), segments(0),lrsArray(0),
     startSmaller(true), isCycle(false),isOrdered(true),length(0.0),
     bbox(false),currentHS(-1)
{
  fromLine(src);
}

/*
7.2 Bulk Loading Functions

*/
template<template<typename T> class Array>
void SimpleLineT<Array>::StartBulkLoad(){
  isOrdered = false;
  SetDefined( true );
}

/*
~Operator +=~

Appends an HalfSegment during BulkLoading.

*/
template<template<typename T> class Array>
SimpleLineT<Array>& SimpleLineT<Array>::operator+=(const HalfSegment& hs){
  assert(!isOrdered && IsDefined());
  segments.Append(hs);
  return *this;
}

template<template<typename T> class Array>
void SimpleLineT<Array>::Add(const HalfSegment& hs) {
  assert(!isOrdered && IsDefined());
  segments.Append(hs);
}

template<template<typename T> class Array>
bool SimpleLineT<Array>::EndBulkLoad(){
  if( !IsDefined() ) {
    Clear();
    SetDefined( false );
  }
  // Sort the segments
  Sort();
  // Realminize the segments
  Array<HalfSegment>* tmp;
  tmp = Realminize(segments);
  segments.clean();
  segments.copyFrom(*tmp);
  tmp->Destroy();
  delete tmp;
  SetPartnerNo();

  // recompute Bounding box;
  if(segments.Size()>0){
    HalfSegment hs;
    segments.Get(0,hs);
    bbox = hs.BoundingBox();
    for(int i=1; i< segments.Size();i++){
      segments.Get(i,hs);
      bbox = bbox.Union(hs.BoundingBox());
    }
  }else{
    bbox.SetDefined(false);
  }

  if(!computePolyline()){
     Clear();
     SetDefined(false);
     return false;
  } else {
     TrimToSize();
     return true;
  }
}

/*
~StartPoint~

Determines the startPoint of this simple line.

*/

template<template<typename T> class Array>
Point SimpleLineT<Array>::StartPoint() const {
  if( !IsDefined() || IsEmpty() ) return Point( false );
  LRS lrs;
  HalfSegment hs;
  int pos = 0;
  if (startSmaller){
    lrsArray.Get( pos, lrs );
    // Get half-segment
    segments.Get( lrs.hsPos, hs );
    // Return one end of the half-segment depending
    // on the start.
    return hs.GetDomPoint();
  } else {
    pos = lrsArray.Size()-1;
    lrsArray.Get( pos, lrs );
    // Get half-segment
    segments.Get( lrs.hsPos, hs );
    // Return one end of the half-segment depending
    // on the start.
    return hs.GetSecPoint();
  }

}

template<template<typename T> class Array>
Point SimpleLineT<Array>::StartPoint( bool startsSmaller ) const {
  if( IsEmpty() || !IsDefined() ) return Point( false );
  if (startsSmaller == startSmaller) return StartPoint();
  else return EndPoint();
}

/*
~EndPoint~

Returns the endpoint of this simple Line.

*/

template<template<typename T> class Array>
Point SimpleLineT<Array>::EndPoint() const {
  if( !IsDefined() || IsEmpty()) return Point( false );
  LRS lrs;
  HalfSegment hs;
  int pos = lrsArray.Size()-1;
  if (startSmaller){
    lrsArray.Get( pos, lrs );
    // Get half-segment
    segments.Get( lrs.hsPos, hs );
    // Return one end of the half-segment depending
    // on the start.
    return hs.GetSecPoint();
  } else {
    pos = 0;
    lrsArray.Get( pos, lrs );
    // Get half-segment
    segments.Get( lrs.hsPos, hs );
    // Return one end of the half-segment depending
    // on the start.
    return hs.GetDomPoint();
  }
}

template<template<typename T> class Array>
Point SimpleLineT<Array>::EndPoint( bool startsSmaller ) const {
  if( IsEmpty() || !IsDefined() ) return Point( false );
  if (startsSmaller == startSmaller) return EndPoint();
  else return StartPoint();
}

template<template<typename T> class Array>
bool SimpleLineT<Array>::Contains( const Point& p,
                           const Geoid* geoid /*=0*/ ) const {
 assert( IsDefined() );
 assert( p.IsDefined() );
 if( IsEmpty()  || !p.IsDefined() || (geoid && !geoid->IsDefined()) ){
   return false;
 }
 int pos;
 if( segments.Find( &p, PointHalfSegmentCompareAlmost, pos )){
   // p is a dominating point of a line
   return true;
 }
 if( pos >= Size() ){
    return false;
 }
 HalfSegment hs;
 for( ; pos >= 0; pos-- ){
   segments.Get( pos, &hs );
   if( hs.IsLeftDomPoint() ) {
     if( hs.Contains( p, geoid ) ){
       return true;
     }
   }
 }
 return false;
}

template<template<typename T> class Array>
template<template<typename T2> class Array2>
bool SimpleLineT<Array>::Inside(const SimpleLineT<Array2> & l, 
                                const Geoid* geoid) const
{
  assert( IsDefined() );
  assert( l.IsDefined() );
  assert( !geoid || geoid->IsDefined() );
  if(!IsDefined() || !l.IsDefined() || (geoid && !geoid->IsDefined()) ){
    return false;
  }
  if( IsEmpty() ){
    return true;
  }
  if( l.IsEmpty() ){
    return false;
  }
  if( !l.BoundingBox().Contains( bbox ) ){
    return false;
  }
  assert( IsOrdered() );
  assert( l.IsOrdered() );
  HalfSegment hs1, hs2;
  for( int i = 0; i < Size(); i++ ){
    Get( i, hs1 );
    if( hs1.IsLeftDomPoint() ){
      bool found = false;
      for( int j = 0; j < l.Size() && !found; j++ ){
        l.Get( j, hs2 );
        if( hs2.IsLeftDomPoint() && hs1.Inside( hs2,geoid ) ){
          found = true;
        }
      }
      if( !found ){
        return false;
      }
    }
  }
  return true;
}

template<template<typename T> class Array>
template<template<typename T2> class Array2>
void SimpleLineT<Array>::Intersection(const Point& p, PointsT<Array2>& result,
                              const Geoid* geoid/*=0*/)const {
  result.Clear();
  if(!IsDefined() || !p.IsDefined() || (geoid&& !geoid->IsDefined()) ){
    result.SetDefined(false);
    return;
  }
  result.SetDefined(true);
  if(this->Contains(p, geoid)){
    result += p;
  }
}

template<template<typename T> class Array>
template<template<typename T2> class Array2,
         template<typename T2> class Array3>
void SimpleLineT<Array>::Intersection(const PointsT<Array2>& ps, 
                                      PointsT<Array3>& result,
                                      const Geoid* geoid/*=0*/) const{
  // naive implementation, should be changed to be faster
  result.Clear();
  if(!IsDefined() || !ps.IsDefined() || (geoid&& !geoid->IsDefined())){
    result.SetDefined(false);
    return;
  }
  Point p;
  result.StartBulkLoad();
  for(int i=0;i<ps.Size(); i++){
    ps.Get(i,p);
    if(this->Contains(p,geoid)){
      result += p;
    }
  }
  result.EndBulkLoad(false,false,false);
}

template<template<typename T> class Array>
template<template<typename T2> class Array2,
         template<typename T3> class Array3>
void SimpleLineT<Array>::Intersection( const LineT<Array2>& l, 
                               SimpleLineT<Array3>& result,
                               const Geoid* geoid/*=0*/ ) const
{
  SetOp(*this,l,result,avlseg::intersection_op, geoid);
}

template<template<typename T> class Array>
template<template<typename T2> class Array2,
         template<typename T3> class Array3>
void SimpleLineT<Array>::Intersection( const SimpleLineT<Array2>& l, 
                                       SimpleLineT<Array3>& result,
                               const Geoid* geoid/*=0*/ ) const
{
  SetOp(*this,l,result,avlseg::intersection_op, geoid);
}

template<template<typename T> class Array>
template<template<typename T2> class Array2,
         template<typename T3> class Array3>
void SimpleLineT<Array>::Intersection(const RegionT<Array2>& r, 
                                      SimpleLineT<Array3>& result,
                                      const Geoid* geoid/*=0*/) const{
  r.Intersection(*this,result,geoid);
}

template<template<typename T> class Array>
template<template<typename T2> class Array2>
void SimpleLineT<Array>::Minus(const Point& p, SimpleLineT<Array2>& result,
                       const Geoid* geoid/*=0*/) const {
  result.Clear();
  if(!IsDefined() || !p.IsDefined()){
    result.SetDefined(false);
    return;
  }
  result.CopyFrom(this);
}

template<template<typename T> class Array>
template<template<typename T2> class Array2,
         template<typename T3> class Array3>
void SimpleLineT<Array>::Minus(const PointsT<Array2>& ps, 
                               SimpleLineT<Array3>& result,
                       const Geoid* geoid/*=0*/) const {
 result.Clear();
 if(!IsDefined() || !ps.IsDefined() || (geoid&& !geoid->IsDefined()) ){
   result.SetDefined(false);
   return;
 }
 result.CopyFrom(this);
}

template<template<typename T> class Array>
template<template<typename T2> class Array2,
         template<typename T3> class Array3>
void SimpleLineT<Array>::Minus(const LineT<Array2>& line, 
                               SimpleLineT<Array3>& result,
                       const Geoid* geoid/*=0*/) const{
  SetOp(*this,line,result,avlseg::difference_op,geoid);
}

template<template<typename T> class Array>
template<template<typename T2> class Array2,
         template<typename T3> class Array3>
void SimpleLineT<Array>::Minus(const RegionT<Array2>& region, 
                               SimpleLineT<Array3>& result,
                       const Geoid* geoid/*=0*/) const{
  SetOp(*this,region, result,avlseg::difference_op,geoid);
}

template<template<typename T> class Array>
template<template<typename T2> class Array2,
         template<typename T3> class Array3>
void SimpleLineT<Array>::Minus(const SimpleLineT<Array2>& line, 
                               SimpleLineT<Array3>& result,
                       const Geoid* geoid/*=0*/) const{
  SetOp(*this,line,result,avlseg::difference_op,geoid);
}


template<template<typename T> class Array>
template<template<typename T2> class Array2>
void SimpleLineT<Array>::Union(const Point& p, SimpleLineT<Array2>& result,
                       const Geoid* geoid/*=0*/) const{
  result.Clear();
  if(!IsDefined() || !p.IsDefined() || (geoid&& !geoid->IsDefined()) ){
    result.SetDefined(false);
    return;
  }
  result.CopyFrom(this);
}


template<template<typename T> class Array>
template<template<typename T2> class Array2,
         template<typename T3> class Array3>
void SimpleLineT<Array>::Union(const PointsT<Array2>& ps, 
                               SimpleLineT<Array3>& result,
                       const Geoid* geoid/*=0*/) const{
  result.Clear();
  if(!IsDefined() || !ps.IsDefined() || (geoid&& !geoid->IsDefined())){
    result.SetDefined(false);
    return;
  }
  result.CopyFrom(this);
}

template<template<typename T> class Array>
template<template<typename T2> class Array2,
         template<typename T3> class Array3>
void SimpleLineT<Array>::Union(const LineT<Array2>& line, LineT<Array3>& result,
                       const Geoid* geoid/*=0*/) const{
  SetOp(*this, line, result, avlseg::union_op,geoid);
}

template<template<typename T> class Array>
template<template<typename T2> class Array2,
         template<typename T3> class Array3>
void SimpleLineT<Array>::Union(const RegionT<Array2>& region,
                               RegionT<Array3>& result,
                       const Geoid* geoid/*=0*/) const{
  region.Union(*this,result,geoid);
}

template<template<typename T> class Array>
template<template<typename T2> class Array2,
         template<typename T3> class Array3>
void SimpleLineT<Array>::Union(const SimpleLineT<Array2>& line,
                               LineT<Array3>& result,
                       const Geoid* geoid/*=0*/) const{
  SetOp(*this, line, result, avlseg::union_op,geoid);
}

template<template<typename T> class Array>
double SimpleLineT<Array>::Distance(const Point& p, 
                                    const Geoid* geoid /* = 0 */)const {
  assert( !IsEmpty() );
  assert( p.IsDefined() );
  assert( ! geoid || geoid->IsDefined() );
  HalfSegment hs;
  double result = std::numeric_limits<double>::max();
  for( int i = 0; i < Size(); i++ ) {
    Get( i, hs );
    if( hs.IsLeftDomPoint() ) {
      if( hs.Contains( p ), geoid ){
        return 0.0;
      }
      result = MIN( result, hs.Distance( p, geoid ) );
    }
  }
  return result;
}


template<template<typename T> class Array>
template<template<typename T2> class Array2>
double SimpleLineT<Array>::Distance(const PointsT<Array2>& ps, 
                                    const Geoid* geoid /* =0 */)const{
  assert( !IsEmpty() );
  assert( !ps.IsEmpty() );
  assert( ! geoid || geoid->IsDefined() );
  HalfSegment hs;
  Point p;
  double result = std::numeric_limits<double>::max();
  for( int i = 0; i < Size(); i++ ) {
    Get( i, hs );
    if( hs.IsLeftDomPoint() ) {
      for( int j = 0; j < ps.Size(); j++ ) {
        ps.Get( j, p );
        if( hs.Contains( p, geoid ) ){
          return 0;
        }
        result = MIN( result, hs.Distance( p, geoid ) );
      }
    }
  }
  return result;
}

template<template<typename T> class Array>
template<template<typename T2> class Array2>
double SimpleLineT<Array>::Distance(const SimpleLineT<Array2>& sl,
                            const Geoid* geoid /* =0 */ )const{
  assert( !IsEmpty() );
  assert( !sl.IsEmpty() );
  assert( ! geoid || geoid->IsDefined() );
  HalfSegment hs1, hs2;
  double result = std::numeric_limits<double>::max();
  for( int i = 0; i < Size(); i++ ) {
    Get( i, hs1 );
    if( hs1.IsLeftDomPoint() ) {
      for( int j = 0; j < sl.Size(); j++ ) {
        sl.Get( j, hs2 );

        if( hs1.Intersects( hs2, geoid ) ){
          return 0.0;
        }
        result = MIN( result, hs1.Distance( hs2, geoid ) );
      }
    }
  }
  return result;
}

template<template<typename T> class Array>
double SimpleLineT<Array>::Distance(const Rectangle<2>& r,
                            const Geoid* geoid /*=0*/)const{
  assert( !IsEmpty() );
  assert( r.IsDefined() );
  assert( !geoid || geoid->IsDefined() );
  if(geoid){
    cout << __PRETTY_FUNCTION__ << ": Spherical geometry not implemented."
         <<endl;
    assert(false); // TODO: Implement spherical geometry case.
  }
  LineT<Array> sll(0);
  toLine(sll);
  return sll.Distance( r, geoid );
}

template<template<typename T> class Array>
bool SimpleLineT<Array>::Intersects(const Rectangle<2>& r,
                            const Geoid* geoid ) const{

  if(!IsDefined() || !r.IsDefined()){
    return false;
  }
  if(!BoundingBox().Intersects(r,geoid)){
     return false;
  }
  HalfSegment hs;
  for( int i = 0; i < Size(); i++ ) {
    Get( i, hs);
    if(hs.Intersects(r,geoid)){
      return true;
    } 
  }
  return false;
}


template<template<typename T> class Array>
bool SimpleLineT<Array>::AtPosition( double pos,
                             Point& p,
                             const Geoid* geoid /* = 0 */) const {
  assert( ! geoid || geoid->IsDefined() );
  if(IsEmpty() || (geoid && !geoid->IsDefined()) ){ // subsumes !IsDefined()
    p.SetDefined( false );
    return false;
  }
  if( !startSmaller ) pos = length - pos;
  LRS lrs( pos, 0 );
  int lrsPos;
  if( !Find( lrs,lrsPos ) ){
    p.SetDefined( false );
    return false;
  }
  LRS lrs2;

  lrsArray.Get( lrsPos, lrs2 );
  HalfSegment hs;
  segments.Get( lrs2.hsPos, &hs );
  p = hs.AtPosition( pos - lrs2.lrsPos, geoid );
  p.SetDefined( true );
  return true;
}

template<template<typename T> class Array>
bool SimpleLineT<Array>::AtPosition( double pos,
                             bool startsSmaller,
                             Point& p,
                             const Geoid* geoid /* = 0 */) const {

  if (IsDefined() && 0.0 <= pos && pos <= length){
    if(startSmaller == startsSmaller)
      return AtPosition(pos, p, geoid);
    else
      return AtPosition(length - pos,p,geoid);
  }
  else{
    p.SetDefined(false);
    return false;
  }

}

/*
~AtPoint~

*/

template<template<typename T> class Array>
bool SimpleLineT<Array>::AtPoint(const Point& p, const bool startsSmaller,
                         double& result,
                         const Geoid* geoid /*= 0*/) const{
  return AtPoint(p, startsSmaller, 0.0, result, geoid);
}

template<template<typename T> class Array>
bool SimpleLineT<Array>::AtPoint( const Point& p,
                          double& result,
                          double tolerance /*=0.0*/,
                          const Geoid* geoid /*=0*/) const {
  return AtPoint(p,this->startSmaller, tolerance, result, geoid);
}

template<template<typename T> class Array>
bool SimpleLineT<Array>::AtPoint( const Point& p,
                          bool startsSmaller,
                          double tolerance,
                          double& result,
                          const Geoid* geoid /*=0*/) const {
 assert( !IsEmpty() );
 assert( p.IsDefined() );
 if( IsEmpty() || !p.IsDefined() || (geoid && !geoid->IsDefined()) ){
   return false;
 }
 if(geoid){
   cout << __PRETTY_FUNCTION__ << ": Spherical geometry not implemented."
        <<endl;
   assert(false); // TODO: Implement spherical geometry case.
 }
 bool found = false;
 HalfSegment hs;
 int pos;
 if( Find( p, pos ) ) {
   found = true;
   segments.Get( pos, &hs );
  } else if( pos < Size() ) {
     for( ; pos >= 0; pos-- ) {
       segments.Get( pos, hs );
       if( hs.IsLeftDomPoint() && hs.Contains( p, geoid ) ) {
         found = true;
         break;
       }
     }
  }

  if( found ){
    LRS lrs;
    lrsArray.Get( hs.attr.edgeno, lrs );
    segments.Get( lrs.hsPos, &hs );
    result = lrs.lrsPos + p.Distance( hs.GetDomPoint() );

    if(!startsSmaller) result = length - result;

    if (tolerance != 0.0)
    {
      if( AlmostEqualAbsolute( result, 0.0, tolerance ) ) result = 0.0;
      else
        if(AlmostEqualAbsolute( result, length, tolerance)) result = length;
    }
    else
    {
      if (AlmostEqual(result,0.0) || result < 0.0) result = 0.0;
      else
        if (AlmostEqual(result, length) || result > length) result = length;
    }

    assert( result >= 0.0 && result <= length );

    return true;
  }
  return false;
}


template<template<typename T> class Array>
template<template<typename T2> class Array2>
void SimpleLineT<Array>::SubLine( double pos1, double pos2, 
                                  SimpleLineT<Array2>& l ) const {
  l.Clear();
  if( !IsDefined() ){
    l.SetDefined( false );
    return;
  }
  l.SetDefined( true );

  if( pos1 < 0 ){
    pos1 = 0;
  } else if( pos1 > length ){
    pos1 = length;
  }

  if( pos2 < 0 ){
    pos2 = 0;
  } else if( pos2 > length ){
    pos2 = length;
  }

  if( AlmostEqual( pos1, pos2 ) || pos1 > pos2 ){
    return;
  }

  double start = pos1;
  double end = pos2;
  if(!this->startSmaller ) {
    start = length - pos2;
    end = length - pos1;
  }

  // First search for the first half segment
  LRS lrs( start, 0 );
  int lrsPos = 0;
  Find( lrs, lrsPos );

  LRS lrs2;
  lrsArray.Get( lrsPos, lrs2 );

  HalfSegment hs;
  segments.Get( lrs2.hsPos, hs );

  l.Clear();
  l.StartBulkLoad();
  int edgeno = 0;

  HalfSegment auxHs;
  if( hs.SubHalfSegment( start - lrs2.lrsPos, end - lrs2.lrsPos, auxHs ) ) {
    auxHs.attr.edgeno = ++edgeno;
    l += auxHs;
    auxHs.SetLeftDomPoint( !auxHs.IsLeftDomPoint() );
    l += auxHs;
  }

  while( lrsPos < lrsArray.Size() - 1 &&
       (lrs2.lrsPos + hs.Length() < end ||
        AlmostEqual( lrs2.lrsPos + hs.Length(), end ) ) ) {
    // Get the next half segment in the sequence
    lrsArray.Get( ++lrsPos, lrs2 );
    segments.Get( lrs2.hsPos, hs );

    if( hs.SubHalfSegment( start - lrs2.lrsPos, end - lrs2.lrsPos, auxHs)){
      auxHs.attr.edgeno = ++edgeno;
      l += auxHs;
      auxHs.SetLeftDomPoint( !auxHs.IsLeftDomPoint() );
      l += auxHs;
    }
  }
  l.EndBulkLoad();
  Point pStartPoint ( true );
  AtPosition ( pos1, startSmaller, pStartPoint );
  Point pEndPoint ( true );
  AtPosition ( pos2, startSmaller, pEndPoint );
  if ( pStartPoint.GetX() < pEndPoint.GetX() ||
     ( pStartPoint.GetX() == pEndPoint.GetX() &&
       pStartPoint.GetY() < pEndPoint.GetY()))
  {
    l.SetStartSmaller(true);
  }
  else
  {
    l.SetStartSmaller(false);
  }
}


template<template<typename T> class Array>
template<template<typename T2> class Array2>
void SimpleLineT<Array>::SubLine( double pos1, double pos2,
                         bool startsSmaller, SimpleLineT<Array2> & l ) const {
  l.Clear();
  if( !IsDefined() ){
    l.SetDefined( false );
    return;
  }
  l.SetDefined( true );

  if( pos1 < 0 ){
    pos1 = 0;
  } else if( pos1 > length ){
    pos1 = length;
  }

  if( pos2 < 0 ){
    pos2 = 0;
  } else if( pos2 > length ){
    pos2 = length;
  }

  if( AlmostEqual( pos1, pos2 ) || pos1 > pos2 ){
    return;
  }

  double start = pos1;
  double end = pos2;
  if( !startsSmaller) {
    start = length - pos2;
    end = length - pos1;
  }

  // First search for the first half segment
  LRS lrs( start, 0 );
  int lrsPos = 0;
  Find( lrs, lrsPos );

  LRS lrs2;
  lrsArray.Get( lrsPos, lrs2 );

  HalfSegment hs;
  segments.Get( lrs2.hsPos, hs );

  l.Clear();
  l.StartBulkLoad();
  int edgeno = 0;

  HalfSegment auxHs;
  if( hs.SubHalfSegment( start - lrs2.lrsPos, end - lrs2.lrsPos, auxHs ) ) {
     auxHs.attr.edgeno = ++edgeno;
     l += auxHs;
     auxHs.SetLeftDomPoint( !auxHs.IsLeftDomPoint() );
     l += auxHs;
   }

   while( lrsPos < lrsArray.Size() - 1 &&
          ( lrs2.lrsPos + hs.Length() < end||
            AlmostEqual( lrs2.lrsPos + hs.Length(), end ) ) ) {
     // Get the next half segment in the sequence
     lrsArray.Get( ++lrsPos, lrs2 );
     segments.Get( lrs2.hsPos, hs );

     if( hs.SubHalfSegment( start - lrs2.lrsPos, end - lrs2.lrsPos, auxHs)){
       auxHs.attr.edgeno = ++edgeno;
       l += auxHs;
       auxHs.SetLeftDomPoint( !auxHs.IsLeftDomPoint() );
       l += auxHs;
     }
   }

   l.EndBulkLoad();
   Point pStartPoint ( true );
   AtPosition ( pos1, startsSmaller, pStartPoint );
   Point pEndPoint ( true );
   AtPosition ( pos2, startsSmaller, pEndPoint );
   if ( pStartPoint.GetX() < pEndPoint.GetX() ||
      ( pStartPoint.GetX() == pEndPoint.GetX() &&
            pStartPoint.GetY() < pEndPoint.GetY()))
   {
     l.SetStartSmaller(true);
   }
   else
   {
     l.SetStartSmaller(false);
   }
}


template<template<typename T> class Array>
template<template<typename T2> class Array2,
         template<typename T3> class Array3>
void SimpleLineT<Array>::Crossings( const SimpleLineT<Array2>& l, 
                            PointsT<Array3>& result,
                            const Geoid* geoid /*=0*/ ) const
{
  result.Clear();
  if( !IsDefined() || !l.IsDefined() ) {
    result.SetDefined( false );
    return;
  }
  result.SetDefined( true );
  if( IsEmpty() || l.IsEmpty()  || (geoid && !geoid->IsDefined())){
    return;
  }
  assert( IsOrdered() && l.IsOrdered() );
  HalfSegment hs1, hs2;
  Point p;
  result.StartBulkLoad();
  for( int i = 0; i < Size(); i++ ){
    Get( i, hs1 );
    if( hs1.IsLeftDomPoint() ){
      for( int j = 0; j < l.Size(); j++ ){
        l.Get( j, hs2 );
        if( hs2.IsLeftDomPoint() ){
          if( hs1.Intersection( hs2, p, geoid ) )
            result += p;
        }
      }
    }
  }
  result.EndBulkLoad(true, true); // sort and remove duplicates
}


template<template<typename T> class Array>
template<template<typename T2> class Array2>
bool SimpleLineT<Array>::Intersects(const SimpleLineT<Array2> & l,
                            const Geoid* geoid /*=0*/ ) const{
  assert( IsDefined() );
  assert( l.IsDefined() );
  assert( !geoid || geoid->IsDefined() );
   if(!IsDefined() || ! l.IsDefined() || (geoid && !geoid->IsDefined())){
      return false;
   }
   if(IsEmpty() || l.IsEmpty()){
      return false;
   }
   if(!geoid && !BoundingBox().Intersects(l.BoundingBox())){
      return false;
   }
  HalfSegment hs1, hs2;
  for( int i = 0; i < Size(); i++ ){
    Get( i, hs1 );
    if( hs1.IsLeftDomPoint() ){
      for( int j = 0; j < l.Size(); j++ ){
        l.Get( j, hs2 );
        if (hs2.IsLeftDomPoint()){
          if( hs1.Intersects( hs2, geoid ) )
            return true;
        }
      }
    }
  }
  return false;
}



template<template<typename T> class Array>
bool SimpleLineT<Array>::SelectInitialSegment( const Point &startPoint,
                                       const double tolerance,
                                       const Geoid* geoid /* = 0 */){
  assert( IsDefined() );
  assert( startPoint.IsDefined() );
  assert( ! geoid || geoid->IsDefined() );
  if(isCycle ){
     return false;
  }
  bool success = Find(startPoint, currentHS, false);
  if ( !success || currentHS < 0 || currentHS >= Size() ){
     currentHS = -1;
     if (tolerance > 0.0) {
       // try to find the point with minimum distance to startPoint,
       // where the distance is smaller than tolerance
       double minDist = tolerance; // currentHS is -1
       double distance = 0.0;
       for(int pos=0; pos<Size(); pos++) {
         // scan all dominating point, save the index of the HalfSegment with
         // the currently smallest distance to startPoint to currentHS and the
         // current minimum distance to minDist
         HalfSegment hs;
         segments.Get( pos, hs );
         distance = hs.GetDomPoint().Distance(startPoint, geoid);
         if (distance <= minDist) {
           minDist   = distance;
           currentHS = pos;
         }
       }
       if (currentHS != -1) {
         return true;
       }
     }
     return false;
   }
   return true;
}

template<template<typename T> class Array>
bool SimpleLineT<Array>::SelectSubsequentSegment() {
  assert( IsDefined() );

  HalfSegment hs;

  if( isCycle || currentHS < 0 ){
     return false;
  }
  segments.Get(currentHS, hs);
  int partner = hs.attr.partnerno;
  HalfSegment nexths;

  // look at position before currentHS's partner
  if( partner>0 ) {
    currentHS = partner - 1;
    segments.Get(currentHS, nexths);
    if ( AlmostEqual(nexths.GetDomPoint(), hs.GetSecPoint()) ) {
       return true;
    }
  }
  // look at position after currentHS's partner
  if( partner < Size()-1 ) {
     currentHS = partner + 1;
     segments.Get(currentHS, nexths);
     if ( AlmostEqual(nexths.GetDomPoint(), hs.GetSecPoint()) ) {
       return true;
     }
   }
   // No subsequent HalfSegment found:
   currentHS = -1;
   return false;
}


template<template<typename T> class Array>
bool SimpleLineT<Array>::getWaypoint( Point &destination ) const{
   assert( IsDefined() );
   if( isCycle || currentHS < 0 || currentHS >= Size() ) {
     destination.SetDefined( false );
     return false;
   }
   HalfSegment hs;
   segments.Get(currentHS, hs);
   destination = hs.GetSecPoint();
   destination.SetDefined( true );
   return true;
}

template<template<typename T> class Array>
template<template<typename T2> class Array2>
void SimpleLineT<Array>::fromLine(const LineT<Array2>& src)
{
  fromLine(src,true);
}

template<template<typename T> class Array>
template<template<typename T2> class Array2>
void SimpleLineT<Array>::fromLine(const LineT<Array2>& src, const bool smaller)
{
  Clear(); // remove all old segments
  if(!src.IsDefined()){
     SetDefined(false);
     return;
  }
  SetDefined(true);
  if(src.IsEmpty()){
    return;
  }
  StartBulkLoad();
  int edgeno = 0;
  HalfSegment hs;
  for(int i=0;i<src.Size();i++){
    src.Get(i,hs);
    if(hs.IsLeftDomPoint()){
       hs.attr.edgeno = edgeno;
       edgeno++;
       (*this) += hs;
       hs.SetLeftDomPoint(false);
       (*this) += hs;
    }
  }
  EndBulkLoad();
  SetStartSmaller(smaller);
}


template<template<typename T> class Array>
template<template<typename T2> class Array2>
void SimpleLineT<Array>::toLine(LineT<Array2>& result)const{
  result.Clear();
  if(!IsDefined()){
    result.SetDefined(false);
    return;
  }
  result.SetDefined(true);
  HalfSegment hs;
  result.StartBulkLoad();
  for(int i=0;i<segments.Size();i++){
    segments.Get(i,hs);
    result +=  hs;
  }
  result.EndBulkLoad();
}


template<template<typename T> class Array>
void SimpleLineT<Array>::SetPartnerNo(){
  if( !IsDefined() || segments.Size()==0){
    return;
  }
  DbArray<int> TMP((segments.Size()+1)/2);

  HalfSegment hs1;
  HalfSegment hs2;
  for(int i=0; i<segments.Size(); i++){
     segments.Get(i,hs1);
     if(hs1.IsLeftDomPoint()){
        TMP.Put(hs1.attr.edgeno, i);
      } else {
        int lpp;
        TMP.Get(hs1.attr.edgeno,lpp);
        int leftpos = lpp;
        HalfSegment right = hs1;
        right.attr.partnerno = leftpos;
        right.attr.insideAbove = false;
        right.attr.coverageno = 0;
        right.attr.cycleno = 0;
        right.attr.faceno = 0;
        segments.Get(leftpos,hs2);
        HalfSegment left = hs2;
        left.attr.partnerno = i;
        left.attr.insideAbove = false;
        left.attr.coverageno = 0;
        left.attr.cycleno = 0;
        left.attr.faceno = 0;
        segments.Put(i,right);
        segments.Put(leftpos,left);
      }
   }
   TMP.Destroy();
 }

template<template<typename T> class Array>
 bool SimpleLineT<Array>::computePolyline(){
  if( !IsDefined() ) {
    return false;
  }
  lrsArray.clean();
  isCycle = false;
  length = 0;
  if( segments.Size()==0){ // an empty line
     return true;
  }

  // the halfsegment array has to be sorted, realminized and
  // the partnernumber must be set correctly

  // step 1: try to find the start of the polyline and check for branches
  int size = segments.Size();
  int start = -1;
  int end = -1;
  int count = 0;
  int pos = 0;
  HalfSegment hs;
  Point p1;
  Point p2;
  while(pos<size){
    count = 1;
    segments.Get(pos,hs);
    p1 = hs.GetDomPoint();
    pos++;
    if(pos<size){
      segments.Get(pos,hs);
      p2 = hs.GetDomPoint();
    }
    while(pos<size && AlmostEqual(p1,p2)){
      count++;
      if(count>2){  // branch detected
        return false;
      } else {
        pos++;
        p1 = p2;
        if(pos<size){
           segments.Get(pos,hs);
           p2 = hs.GetDomPoint();
        }
      }
    }
    if(count==1){
       if(start<0){
         start = pos - 1;
       } else if(end < 0){
         end = pos - 1;
       } else { // third end detected
         return false;
       }
    }
  }

  if(start<0 && end>=0){ // loop detected
      return false;
  }

  pos = 0;
  if(start<0){ // line is a cycle
    isCycle=true;
  } else {
    isCycle = false;
    pos = start;
  }

  // the line has two or zero endpoints, may be several components
  std::vector<bool> used(size,false);
  int noUnused = size;
  HalfSegment hs1;
  HalfSegment hs2;
  lrsArray.resize(segments.Size()/2 + 1);
  double lrsPos = 0.0;
  int hsPos = pos;
  int edge = 0;
  while(noUnused > 0){
    segments.Get(hsPos,hs1);
    used[hsPos]=true; // segment is used
    noUnused--;
    int partnerpos = hs1.attr.partnerno;
    segments.Get(partnerpos,hs2);
    used[partnerpos] = true; // partner is used
    noUnused--;
    // store edgenumber
    HalfSegment HS1 = hs1;
    HalfSegment HS2 = hs2;
    HS1.attr.edgeno = edge;
    HS2.attr.edgeno = edge;
    edge++;
    segments.Put(hsPos,HS1);
    segments.Put(partnerpos,HS2);

    lrsArray.Append(LRS(lrsPos,hsPos));
    lrsPos += hs1.Length();
    Point p1 = hs2.GetDomPoint();
    if(noUnused > 0){
       bool found = false;
       if(partnerpos > 0 && !used[partnerpos-1]){ // check left side
         segments.Get(partnerpos-1,hs2);
         Point p2 = hs2.GetDomPoint();
         if(AlmostEqual(p1,p2)){ // extension found
           found = true;
           hsPos = partnerpos-1;
         }
       }
       if(!found  && (partnerpos < (size-1) && !used[partnerpos+1])){
           segments.Get(partnerpos+1,hs2);
           Point p2 = hs2.GetDomPoint();
           if(AlmostEqual(p1,p2)){
              found = true;
              hsPos = partnerpos+1;
           }
       }
       if(!found){  // no extension found
         return false;
       }
    }
  }
  lrsArray.Append(LRS(lrsPos,hsPos));
  length = lrsPos;
  return true;
}


template<template<typename T> class Array>
int SimpleLineT<Array>::Compare(const Attribute* arg)const{
  const SimpleLineT<Array> * line =  
                 static_cast<const SimpleLineT<Array> *>(arg);
  return Compare(line);
}


template<template<typename T> class Array>
template<template<typename T2> class Array2>
int SimpleLineT<Array>::Compare(const SimpleLineT<Array2>* line)const{
  if(!IsDefined() && !line->IsDefined()){
     return true;
  }
  if(!IsDefined()){
    return -1;
  }
  if(!line->IsDefined()){
     return 1;
  }
  if(segments.Size() < line->segments.Size()){
    return -1;
  }
  if(segments.Size() > line->segments.Size()){
    return 1;
  }
  if(startSmaller && !line->startSmaller) {
    return 1;
  }
  if(!startSmaller && line->startSmaller) {
    return -1;
  }
  int cmp;
  HalfSegment hs1;
  HalfSegment hs2;
  for(int i=0;i<segments.Size();i++){
     segments.Get(i,hs1);
     line->segments.Get(i,hs2);
     if( (cmp = hs1.Compare(hs2)) !=0){
       return cmp;
     }
  }
  return 0;
}

template<template<typename T> class Array>
std::ostream& SimpleLineT<Array>::Print(std::ostream& o)const{
  o << "SimpleLineT def =" << IsDefined()
    << " size = " << Size()
    << " startSmaller: " << startSmaller << endl;
  for (int i = 0; i < Size(); i++)
  {
    HalfSegment hs;
    Get(i, hs);
    if (hs.IsLeftDomPoint()) hs.Print(o);
  }
  return o;
}

template<template<typename T> class Array>
double SimpleLineT<Array>::Length(const Geoid &geoid, bool& valid) const {
  valid = true;
  if(!IsDefined()){
    valid = false;
    return 0.0;
  }
  double length = 0.0;
  for (int i=0; (valid && (i<Size())) ;i++){
    HalfSegment hs;
    Get( i, hs );
    if( hs.IsLeftDomPoint() ){
      length += hs.LengthOrthodrome(geoid, valid);
    };
  }
  return length;
}

template<template<typename T> class Array>
std::vector<SimpleLineT<Array> >* SimpleLineT<Array>::SplitAtPPoints(
 PointsT<Array>* pts)
{
  Point curPoint(true);
  double pos;
  SimpleLineT<Array> res(0);
  std::vector<SimpleLineT<Array> >* result = 
                               new std::vector<SimpleLineT<Array> >();
  result->clear();
  std::vector<double>* splitPositions= new std::vector<double>();
  splitPositions->clear();
  splitPositions->push_back(0.0);
  splitPositions->push_back(Length());
  for (int i = 0; i < pts->Size(); i++)
  {
    pts->Get(i,curPoint);
    AtPoint(curPoint, pos);
    splitPositions->push_back(pos);
  }
  stable_sort(splitPositions->begin(), splitPositions->end());
  double from = 0.0;
  double to = from;
  for (size_t j = 0; j < splitPositions->size(); j++)
  {
    to = splitPositions->at(j);
    if (to != from)
    {
      SubLine(from, to, res);
      result->push_back(res);
      from = to;
    }
  }
  if (from != Length())
  {
    SubLine(from, Length(), res);
    result->push_back(res);
  }
  splitPositions->clear();
  delete splitPositions;
  return result;
}

template<template<typename T> class Array>
std::ostream& operator<<(std::ostream& o, const SimpleLineT<Array>& cl){
   cl.Print(o);
   return o;
}


