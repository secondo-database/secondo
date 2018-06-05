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

#ifndef LineTImpl_H
#define LineTImpl_H


#include "RegionCreator.h"
#include "AVLSegment.h"
#include "LineT.h"
#include "../../Tools/Flob/MMDbArray.h"

#include <stack>

bool douglas_peucker(const std::vector<Point>& orig, // original line
                     const double epsilon, // maximum derivation
                     bool* use, // result
                     const int min, const int max,
                     const bool force,
                     const Geoid* geoid); // current range

void douglas_peucker(const std::vector<Point>& orig, // original line
                     const double epsilon, // maximum derivation
                     bool* use, // result
                     const bool force,
                     const Geoid* geoid); // current range


template<template<typename T1>class Array1,
         template<typename T2> class Array2>
void SelectFirst_ll( const LineT<Array1>& L1, 
                     const LineT<Array2>& L2,
                     object& obj, status& stat )
{
  L1.SelectFirst();
  L2.SelectFirst();

  HalfSegment hs1, hs2;
  bool gotHs1 = L1.GetHs( hs1 ),
       gotHs2 = L2.GetHs( hs2 );

  if( !gotHs1 && !gotHs2 )
  {
    obj = none;
    stat = endboth;
  }
  else if( !gotHs1 )
  {
    obj = second;
    stat = endfirst;
  }
  else if( !gotHs2 )
  {
    obj = first;
    stat = endsecond;
  }
  else //both defined
  {
    stat = endnone;
    if( hs1 < hs2 )
      obj = first;
    else if( hs1 > hs2 )
      obj = second;
    else obj = both;
  }
}


template<template<typename T1>class Array1,
         template<typename T2> class Array2>
void SelectNext_ll( const LineT<Array1>& L1, 
                    const LineT<Array2>& L2,
                    object& obj, status& stat )
{
  // 1. get the current elements
  HalfSegment hs1, hs2;
  bool gotHs1 = L1.GetHs( hs1 ),
       gotHs2 = L2.GetHs( hs2 );

  //2. move the pointers
  if( !gotHs1 && !gotHs2 )
  {
    //do nothing
  }
  else if( !gotHs1 )
  {
    L2.SelectNext();
    gotHs2 = L2.GetHs( hs2 );
  }
  else if( !gotHs2 )
  {
    L1.SelectNext();
    gotHs1 = L1.GetHs( hs1 );
  }
  else //both currently defined
  {
    if( hs1 < hs2 ) //then hs1 is the last output
    {
      L1.SelectNext();
      gotHs1 = L1.GetHs( hs1 );
    }
    else if( hs1 > hs2 )
    {
      L2.SelectNext();
      gotHs2 = L2.GetHs( hs2 );
    }
    else
    {
      L1.SelectNext();
      gotHs1 = L1.GetHs( hs1 );
      L2.SelectNext();
      gotHs2 = L2.GetHs( hs2 );
    }
  }

  //3. generate the outputs
  if( !gotHs1 && !gotHs2 )
  {
    obj = none;
    stat = endboth;
  }
  else if( !gotHs1 )
  {
    obj = second;
    stat = endfirst;
  }
  else if( !gotHs2 )
  {
    obj = first;
    stat = endsecond;
  }
  else //both defined
  {
    stat = endnone;
    if( hs1 < hs2 )
      obj = first;
    else if( hs1 > hs2 )
      obj = second;
    else
      obj = both;
  }
}



/*
11.4 Class ~Line~

*/
template<template<typename T>class Array>
inline LineT<Array>::LineT( const int n ) :
StandardSpatialAttribute<2>(true),
line( n ),
bbox( false ),
ordered( true ),
noComponents( 0 ),
length( 0.0 ),
currentHS( -1 )
{ }

template<template<typename T>class Array>
inline LineT<Array>::LineT( const LineT& cl ) :
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

template<template<typename T>class Array>
template<template<typename T2>class Array2>
inline LineT<Array>::LineT( const LineT<Array2>& cl ) :
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
  convertDbArray(cl.line,line);
}

template<template<typename T>class Array>
inline void LineT<Array>::Destroy()
{
  line.Destroy();
}


template<template<typename T>class Array>
inline double LineT<Array>::Length() const
{
  assert( IsDefined() );
  return length;
}


template<template<typename T>class Array>
inline const Rectangle<2> LineT<Array>::BoundingBox(
       const Geoid* geoid /*=0*/) const
{
  if(geoid){ // spherical geometry case:
    if(!geoid->IsDefined() || !IsDefined()){
      return Rectangle<2>(false);
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

template<template<typename T>class Array>
inline bool LineT<Array>::IsOrdered() const
{
  return ordered;
}

template<template<typename T>class Array>
inline bool LineT<Array>::IsEmpty() const
{
  return !IsDefined() || (line.Size() == 0);
}


template<template<typename T>class Array>
inline int LineT<Array>::Size() const
{
  return line.Size();
}

template<template<typename T>class Array>
inline void LineT<Array>::Get( const int i, HalfSegment& hs ) const
{
  assert( IsDefined() );
  assert(i>=0);
  assert(i<line.Size());
  line.Get( i, &hs );
}

template<template<typename T>class Array>
inline void LineT<Array>::Resize(const int newSize){
   if(newSize>Size()){
      line.resize(newSize);
   }
}

template<template<typename T>class Array>
inline void LineT<Array>::TrimToSize(){
  line.TrimToSize();
}

template<template<typename T>class Array>
inline void LineT<Array>::Put( const int i, const HalfSegment& hs )
{
  assert( IsDefined() );
  line.Put( i, hs );
}

template<template<typename T>class Array>
inline void LineT<Array>::SelectFirst() const
{
  if( IsEmpty() )
    pos = -1;
  else pos = 0;
}

template<template<typename T>class Array>
inline void LineT<Array>::SelectNext() const
{
  if( pos >= 0 && pos < Size() - 1 )
    pos++;
  else pos = -1;
}

template<template<typename T>class Array>
inline bool LineT<Array>::EndOfHs() const
{
  return pos == -1;
}

template<template<typename T>class Array>
inline bool LineT<Array>::GetHs(  HalfSegment& hs ) const
{
  assert( IsDefined() );
  if( pos >= 0 && pos <= Size()-1 )
  {
    line.Get( pos, &hs );
    return true;
  }
  return false;
}

template<template<typename T>class Array>
template<class LineType>
double LineT<Array>::Distance( const LineType& l, 
                               const Geoid* geoid /* = 0 */ ) const
{
  assert( !IsEmpty() );   // includes !undef
  assert( !l.IsEmpty() ); // includes !undef
  assert(!geoid || geoid->IsDefined() );
  if( IsEmpty() || l.IsEmpty()){
    return -1;
  }
  assert( IsOrdered() );
  assert( l.IsOrdered() );
  HalfSegment hs1, hs2;
  double result = std::numeric_limits<double>::max();
  double segDistance = -666.666;
  for( int i = 0; i < Size(); i++ ){
    Get( i, hs1 );
    if( hs1.IsLeftDomPoint() ) {
      for( int j = 0; j < l.Size(); j++ ) {
        l.Get( j, hs2 );
        if( hs1.Intersects( hs2, geoid ) ){
          return 0.0;
        }
        segDistance = hs1.Distance( hs2, geoid );
        if(geoid && AlmostEqual(segDistance,0.0)){
          return 0.0;
        }
        if(segDistance>=0.0){
          result = MIN( result, segDistance );
        }
      }
    }
  }
  return result;
}


int HalfSegmentCompare(const void *a, const void *b);


/*
7 Type Constructor ~line~

A ~line~ value is a set of halfsegments. In the external (nestlist)
representation, a line value is expressed as a set of segments.
However, in the internal (class) representation, it is expressed
as a set of sorted halfsegments, which are stored as a PArray.

7.1 Implementation of the class ~line~

*/
template<template<typename T>class Array>
void LineT<Array>::StartBulkLoad()
{
  ordered = false;
}

/*
~EndBulkLoad~

Finishs the bulkload for a line. If this function is called,
both HalfSegments assigned to a segment of the line must be part
of this line.

The parameter ~sort~ can be set to __false__ if the Halfsegments are
already ordered using the HalfSegment order.

The parameter ~realminize~ can be set to __false__ if the line is
already realminized, meaning each pair of different Segments has
at most a common endpoint. Furthermore, the two halgsegments belonging
to a segment must have the same edge number. The edge numbers mut be
in Range [0..Size()-1]. HalfSegments belonging to different segments
must have different edge numbers.

Only change one of the parameters if you exacly know what you do.
Changing such parameters without fulifilling the conditions stated
above may construct invalid line representations which again may
produce a system crash within some operators.

*/
template<template<typename T>class Array>
void LineT<Array>::EndBulkLoad( const bool sort /* = true */,
                        const bool realminize /* = true */,
                        const bool robust /* = false */ //don't try plane sweep
                      ){
  if( !IsDefined() ) {
    Clear();
    SetDefined( false );
  }

  if(sort){
    Sort();
  }
if(Size()>0){
   if(realminize){
     Array<HalfSegment>* line2 = ::Realminize<Array>(line,false, robust);
     line2->Sort(HalfSegmentCompare);
     line.copyFrom(*line2);
     line2->Destroy();
     delete line2;
   }
   SetPartnerNo();
}
computeComponents();
TrimToSize();
}

template<template<typename T>class Array>
template<template<typename T2>class Array2>
LineT<Array>& LineT<Array>::operator=( const LineT<Array2>& l )
{
assert( l.ordered );
convertDbArrays(l.line,line);
bbox = l.bbox;
length = l.length;
noComponents = l.noComponents;
ordered = true;
currentHS = l.currentHS;
this->SetDefined(l.IsDefined());

return *this;
}

template<template<typename T>class Array>
LineT<Array>& LineT<Array>::operator=( const LineT& l )
{
  assert( l.ordered );
  line.copyFrom(l.line);
  bbox = l.bbox;
  length = l.length;
  noComponents = l.noComponents;
  ordered = true;
  currentHS = l.currentHS;
  this->SetDefined(l.IsDefined());
  return *this;
}

template<template<typename T>class Array>
bool LineT<Array>::operator==( const LineT<Array>& l ) const
{
if(!IsDefined() && !l.IsDefined()){
  return true;
}
if(!IsDefined() || !l.IsDefined()){
  return false;
}
if( IsEmpty() && l.IsEmpty() ) {
  return true;
}
if( Size() != l.Size() ){
  return false;
}
if( bbox != l.bbox ){
  return false;
}
assert( ordered && l.ordered );
HalfSegment hs, lhs;
for(int i=0;i<Size();i++){
  line.Get(i, hs);
  l.line.Get(i, lhs);
  if( hs != lhs ){
    return false;
  }
}
return true;
}

template<template<typename T>class Array>
bool LineT<Array>::operator!=( const LineT<Array>& l ) const
{
return !( *this == l);
}

template<template<typename T>class Array>
LineT<Array>& LineT<Array>::operator+=( const HalfSegment& hs )
{
if(!IsDefined()){
  assert( IsDefined() );
  return *this;
}

if( IsEmpty() )
  bbox = hs.BoundingBox();
else
  bbox = bbox.Union( hs.BoundingBox() );

if( !IsOrdered() )
  line.Append( hs );
else
{
  int pos;
  if( !Find( hs, pos ) )
  {
    HalfSegment auxhs;
    for( int i = line.Size() - 1; i >= pos; i++ )
    {
      line.Get( i, auxhs );
      line.Put( i+1, auxhs );
    }
    line.Put( pos, hs );
  }
}
return *this;
}

template<template<typename T>class Array>
LineT<Array>& LineT<Array>::operator+=(const LineT<Array>& l){
 if(!IsDefined() || !l.IsDefined()){
   SetDefined( false );
   return *this;
 }

 if(l.line.Size()==0){
   return *this;
 }

 assert(!IsOrdered());

 if(IsEmpty()){
    bbox = l.bbox;
 } else {
    bbox = bbox.Union(l.bbox);
 }

 line.Append(l.line);
 return *this;
}


template<template<typename T>class Array>
LineT<Array>& LineT<Array>::operator-=( const HalfSegment& hs )
{
if(!IsDefined()){
  return *this;
}

assert( IsOrdered() );
int pos;
HalfSegment auxhs;
if( Find( hs, pos ) )
{
  for( int i = pos; i < Size(); i++ )
  {
    line.Get( i+1, auxhs );
    line.Put( i, auxhs );
  }
}

// Naive way to redo the bounding box.
if( IsEmpty() )
  bbox.SetDefined( false );
int i = 0;
line.Get( i++, auxhs );
bbox = auxhs.BoundingBox();
for( ; i < Size(); i++ )
{
  line.Get( i, auxhs );
  bbox = bbox.Union( auxhs.BoundingBox() );
}

return *this;
}

template<template<typename T>class Array>
bool LineT<Array>::Contains( const Point& p, const Geoid* geoid/*=0*/ ) const
{
  assert( IsDefined() );
  assert( p.IsDefined() );
  if( IsEmpty() || (geoid&& !geoid->IsDefined()))
    return false;

  int pos;
  if( Find( p, pos ) )
    return true;

  if( pos >= Size() )
    return false;

  HalfSegment hs;
  for( ; pos >= 0; pos-- ){
  Get( pos, hs );
  if( hs.IsLeftDomPoint() ){
    if( hs.Contains( p ) )
      return true;
    }
  }
  return false;
}

template<template<typename T>class Array>
template<template<typename T2>class Array2>
bool LineT<Array>::Intersects( const LineT<Array2>& l, 
                               const Geoid* geoid/*=0*/ ) const
{
  assert( IsDefined() );
  assert( l.IsDefined() );
  if( IsEmpty() || l.IsEmpty() )
    return false;

  assert( IsOrdered() );
  assert( l.IsOrdered() );
  if( !BoundingBox().Intersects( l.BoundingBox() ) )
    return false;

  HalfSegment hs1, hs2;
  for( int i = 0; i < Size(); i++ )
  {
    Get( i, hs1 );
    if( hs1.IsLeftDomPoint() )
    {
      for( int j = 0; j < l.Size(); j++ )
      {
        l.Get( j, hs2 );
        if (hs2.IsLeftDomPoint())
        {
          if( hs1.Intersects( hs2 ) )
            return true;
        }
      }
    }
  }
  return false;
}

template<template<typename T>class Array>
template<template<typename T2>class Array2>
bool LineT<Array>::Intersects( const RegionT<Array2>& r, 
                               const Geoid* geoid/*=0*/ ) const
{
  assert( IsDefined() );
  assert( r.IsDefined() );
  assert( !geoid || geoid->IsDefined() );
  if( IsEmpty() || r.IsEmpty() || (geoid && !geoid->IsDefined()) ){
    return false;
  }
  if( !BoundingBox().Intersects( r.BoundingBox(),geoid ) )
    return false;
  assert( IsOrdered() );
  assert( r.IsOrdered() );
  HalfSegment hsl, hsr;
  for( int i = 0; i < Size(); i++ ){
    Get( i, hsl );
    if( hsl.IsLeftDomPoint() ){
        for( int j = 0; j < r.Size(); j++ ){
        r.Get( j, hsr );
        if( hsr.IsLeftDomPoint() ){
          if( hsl.Intersects( hsr,geoid ) )
            return true;
          }
        }

      if( r.Contains( hsl.GetLeftPoint(),geoid ) ||
          r.Contains( hsl.GetRightPoint(),geoid ) ){
          return true;
      }
    }
  }
  return false;
}

template<template<typename T>class Array>
template<template<typename T2>class Array2>
bool LineT<Array>::Inside( const LineT<Array2>& l, 
                           const Geoid* geoid/*=0*/ ) const
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

template<template<typename T>class Array>
template<template<typename T2>class Array2>
bool LineT<Array>::Inside( const RegionT<Array2>& r, 
                           const Geoid* geoid/*=0*/ ) const
{
  assert( IsDefined() );
  assert( r.IsDefined() );
  assert( !geoid || geoid->IsDefined() );
  if(!IsDefined() || !r.IsDefined() || (geoid && !geoid->IsDefined())){
    return false;
  }
  if( IsEmpty() ){
    return true;
  }
  if( r.IsEmpty() ){
    return false;
  }
  if( !r.BoundingBox().Contains( bbox ) ){
    return false;
  }
  assert( IsOrdered() );
  assert( r.IsOrdered() );
  HalfSegment hsl;
  for( int i = 0; i < Size(); i++ ){
    Get( i, hsl );
    if( hsl.IsLeftDomPoint() ){
      if( !r.Contains( hsl ) ){
        return false;
      }
    }
  }
  return true;
}

template<template<typename T>class Array>
template<template<typename T2>class Array2>
bool LineT<Array>::Adjacent( const RegionT<Array2>& r, 
                             const Geoid* geoid/*=0*/ ) const
{
  assert( IsDefined() );
  assert( r.IsDefined() );
  assert( !geoid || geoid->IsDefined() );

  if( IsEmpty() || r.IsEmpty()  || (geoid && !geoid->IsDefined()) ){
    return false;
  }
  if( !BoundingBox().Intersects( r.BoundingBox() ) ){
    return false;
  }
  assert( IsOrdered() );
  assert( r.IsOrdered() );
  HalfSegment hsl, hsr;
  bool found = false;
  for( int i = 0; i < Size(); i++ ){
    Get( i, hsl );
    if( hsl.IsLeftDomPoint() ){
      if( r.InnerContains( hsl.GetLeftPoint(),geoid ) ||
          r.InnerContains( hsl.GetRightPoint(),geoid ) ){
        return false;
      }
      for( int j = 0; j < r.Size(); j++ ){
        r.Get( j, hsr );
        if( hsr.IsLeftDomPoint() ) {
          if( !hsr.Intersects( hsl,geoid ) ){
            continue;
          }
          found = true;
          if( hsr.Crosses( hsl,geoid ) ){
            return false;
          }
        }
      }
    }
  }
  return found;
}


template<template<typename T>class Array>
template<template<typename T2>class Array2>
void LineT<Array>::Intersection(const Point& p, 
                        PointsT<Array2>& result,
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

template<template<typename T>class Array>
template<template<typename T2>class Array2,
         template<typename T3>class Array3>
void LineT<Array>::Intersection(const PointsT<Array2>& ps, 
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


template<template<typename T>class Array>
template<template<typename T2>class Array2,
         template<typename T3>class Array3>
void LineT<Array>::Intersection( const LineT<Array2>& l, LineT<Array3>& result,
                         const Geoid* geoid/*=0*/ ) const
{
  SetOp(*this,l,result,avlseg::intersection_op, geoid);
}

template<template<typename T>class Array>
template<template<typename T2>class Array2,
         template<typename T3>class Array3>
void LineT<Array>::Intersection(const RegionT<Array2>& r, LineT<Array3>& result,
                        const Geoid* geoid/*=0*/) const{
 r.Intersection(*this,result,geoid);
}

template<template<typename T>class Array>
template<template<typename T2>class Array2,
         template<typename T3>class Array3>
void LineT<Array>::Intersection( const SimpleLineT<Array2>& l, 
                                 SimpleLineT<Array3>& result,
                         const Geoid* geoid/*=0*/ ) const
{
 SetOp(*this,l,result,avlseg::intersection_op, geoid);
}

template<template<typename T>class Array>
template<template<typename T2>class Array2>
void LineT<Array>::Minus(const Point& p, LineT<Array2>& result,
                 const Geoid* geoid/*=0*/) const {
  result.Clear();
  if(!IsDefined() || !p.IsDefined()){
    result.SetDefined(false);
    return;
  }
  result.CopyFrom(this);
}

template<template<typename T>class Array>
template<template<typename T2>class Array2,
         template<typename T3>class Array3>
void LineT<Array>::Minus(const PointsT<Array2>& ps, LineT<Array3>& result,
                 const Geoid* geoid/*=0*/) const {
  result.Clear();
  if(!IsDefined() || !ps.IsDefined() || (geoid&& !geoid->IsDefined()) ){
    result.SetDefined(false);
    return;
  }
  result.CopyFrom(this);
}

template<template<typename T>class Array>
template<template<typename T2>class Array2,
         template<typename T3>class Array3>
void LineT<Array>::Minus(const LineT<Array2>& line, LineT<Array3>& result,
                 const Geoid* geoid/*=0*/) const{
 SetOp(*this,line,result,avlseg::difference_op,geoid);
}

template<template<typename T>class Array>
template<template<typename T2>class Array2,
         template<typename T3>class Array3>
void LineT<Array>::Minus(const RegionT<Array2>& region, LineT<Array3>& result,
                 const Geoid* geoid/*=0*/) const{
 SetOp(*this,region, result,avlseg::difference_op,geoid);
}

template<template<typename T>class Array>
template<template<typename T2>class Array2,
         template<typename T3>class Array3>
void LineT<Array>::Minus(const SimpleLineT<Array2>& line, LineT<Array3>& result,
                 const Geoid* geoid/*=0*/) const{
  SetOp(*this,line,result,avlseg::difference_op,geoid);
}

template<template<typename T>class Array>
template<template<typename T2>class Array2>
void LineT<Array>::Union(const Point& p, LineT<Array2>& result, 
                         const Geoid* geoid/*=0*/) const{
  result.Clear();
  if(!IsDefined() || !p.IsDefined() || (geoid&& !geoid->IsDefined()) ){
    result.SetDefined(false);
    return;
  }
  result.CopyFrom(this);
}


template<template<typename T>class Array>
template<template<typename T2>class Array2,
         template<typename T3>class Array3>
void LineT<Array>::Union(const PointsT<Array2>& ps, LineT<Array3>& result,
                 const Geoid* geoid/*=0*/) const{
  result.Clear();
  if(!IsDefined() || !ps.IsDefined() || (geoid&& !geoid->IsDefined())){
    result.SetDefined(false);
    return;
  }
  result.CopyFrom(this);
}

template<template<typename T>class Array>
template<template<typename T2>class Array2,
         template<typename T3>class Array3>
void LineT<Array>::Union(const LineT<Array2>& line, LineT<Array3>& result,
                 const Geoid* geoid/*=0*/) const{
 try{
    SetOp(*this, line, result, avlseg::union_op,geoid,true);
 } catch(...){
    std::cerr << "union via plane sweep  failed, use slower implementation";
    result.Clear();
    result.Resize(this->Size()+line.Size());
    HalfSegment hs;
    result.StartBulkLoad();
    for(int i=0;i<line.Size();i++){
       line.Get(i,hs);
       result += hs;
    }
    for(int i=0;i<this->Size();i++){
       this->Get(i,hs);
       result += hs;
    }
    result.EndBulkLoad(true,true,true);
 }
}

template<template<typename T>class Array>
template<template<typename T2>class Array2,
         template<typename T3>class Array3>
void LineT<Array>::Union(const RegionT<Array2>& region, RegionT<Array3>& result,
                 const Geoid* geoid/*=0*/) const{
 region.Union(*this,result,geoid);
}

template<template<typename T>class Array>
template<template<typename T2>class Array2,
         template<typename T3>class Array3>
void LineT<Array>::Union(const SimpleLineT<Array2>& line, LineT<Array3>& result,
                 const Geoid* geoid/*=0*/) const{
  SetOp(*this, line, result, avlseg::union_op,geoid);
}

template<template<typename T>class Array>
template<template<typename T2>class Array2,
         template<typename T3>class Array3>
void LineT<Array>::Crossings( const LineT<Array2>& l, PointsT<Array3>& result,
                      const Geoid* geoid/*=0*/ ) const{
  result.Clear();
  if( !IsDefined() || !l.IsDefined() || (geoid&& !geoid->IsDefined())) {
    result.SetDefined( false );
    return;
  }
  result.SetDefined( true );
  if( IsEmpty() || l.IsEmpty() ){
    return;
  }
  assert( IsOrdered() );
  assert( l.IsOrdered() );
  HalfSegment hs1, hs2;
  Point p;
  result.StartBulkLoad();
  for( int i = 0; i < Size(); i++ ){
    Get( i, hs1 );
    if( hs1.IsLeftDomPoint() )  {
      for( int j = 0; j < l.Size(); j++ ){
        l.Get( j, hs2 );
        if( hs2.IsLeftDomPoint() ){
          if( hs1.Intersection( hs2, p, geoid ) ){
            result += p;
          }
        }
      }
    }
  }
  result.EndBulkLoad(true, true); // sort and remove duplicates
}


template<template<typename T>class Array>
template<template<typename T2>class Array2>
void LineT<Array>::Crossings(PointsT<Array2>& result, 
                             const Geoid* geoid/*=0*/) const{
  result.Clear();
  if(!IsDefined() || (geoid&& !geoid->IsDefined())){
    result.SetDefined(false);
    return;
  }
  result.SetDefined(true);
  int i = 0;
  int size = Size();
  Point lastPoint(false);
  HalfSegment hs;
  Point p;
  int count = 0;
  result.StartBulkLoad();
  while(i<size){
    Get(i,hs);
    p = hs.GetDomPoint();
    i++;
    if(!lastPoint.IsDefined()){ // first point
        lastPoint = p;
        count = 0;
    } else if(AlmostEqual(lastPoint,p)){
      count++;
    } else {
      if(count>2){ // crossing found
        result += p;
      }
      lastPoint = p;
      count = 1;
    }
  }
  if(lastPoint.IsDefined() && count>2){
    result += lastPoint;
  }
  result.EndBulkLoad();
}

template<template<typename T>class Array>
double LineT<Array>::Distance( const Point& p, 
                               const Geoid* geoid /* = 0 */ ) const {
  assert( !IsEmpty() );
  assert( p.IsDefined() );
  assert(!geoid || geoid->IsDefined() );
  if( IsEmpty() || !p.IsDefined()){
    return -1;
  }

  assert( IsOrdered() );
  HalfSegment hs;
  double result = std::numeric_limits<double>::max();
  double segDistance = -666.666;
  for( int i = 0; i < Size(); i++ ){
    Get( i, hs );
    if( hs.IsLeftDomPoint() ){
      if( !geoid && hs.Contains( p ) ){
        return 0.0;
      }
      segDistance = hs.Distance( p, geoid );
      if(geoid && AlmostEqual(segDistance,0.0)){
        return 0.0;
      }
      if(segDistance>=0.0){
        result = MIN( result, segDistance );
      }
    }
  }
  return result;
}

template<template<typename T>class Array>
double LineT<Array>::MaxDistance( const Point& p, 
                                  const Geoid* geoid /* = 0 */ ) const {
  assert( !IsEmpty() );
  assert( p.IsDefined() );
  assert(!geoid || geoid->IsDefined() );
  if( IsEmpty() || !p.IsDefined()){
    return -1;
  }
  assert( IsOrdered() );
  HalfSegment hs;
  double result = 0;
  double segDistance = -666.666;

  for( int i = 0; i < Size(); i++ ){
    Get( i, hs );
    if( hs.IsLeftDomPoint() )  {
      segDistance = hs.Distance( p, geoid );
      if(segDistance>=0.0){
        result = MAX( result, segDistance );
      }
    }
  }
  return result;
}

template<template<typename T>class Array>
template<template<typename T2>class Array2>
double LineT<Array>::Distance( const PointsT<Array2>& ps, 
                               const Geoid* geoid /* = 0 */ ) const {
  assert( !IsEmpty() ); // includes !undef
  assert( !ps.IsEmpty() ); // includes !undef
  assert(!geoid || geoid->IsDefined() );
  if( IsEmpty() || ps.IsEmpty()){
    return -1;
  }
  assert( IsOrdered() );
  assert( ps.IsOrdered() );
  HalfSegment hs;
  Point p;
  double result = std::numeric_limits<double>::max();
  double segDistance = -666.666;
  for( int i = 0; i < Size(); i++ ){
    Get( i, hs );
    if( hs.IsLeftDomPoint() ){
      for( int j = 0; j < ps.Size(); j++ ){
        ps.Get( j, p );
        if(!geoid && hs.Contains( p, geoid ) ){
          return 0.0;
        }
        segDistance = hs.Distance( p, geoid );
        if(geoid && AlmostEqual(segDistance,0.0)){
          return 0.0;
        }
        if(segDistance>=0.0){
          result = MIN( result, segDistance );
        }
      }
    }
  }
  return result;
}




template<template<typename T>class Array>
template<template<typename T2>class Array2>
void LineT<Array>::DistanceSmallerThan(const LineT<Array2>& l,
                              const double  maxDist,
                              const bool allowEqual,
                              CcBool& result,
                              const Geoid* geoid) const{

  assert( !IsEmpty() );   // includes !undef
  assert( !l.IsEmpty() ); // includes !undef
  assert(!geoid || geoid->IsDefined() );
  if( IsEmpty() || l.IsEmpty()){
    result.SetDefined(false);
    return;
  }
  assert( IsOrdered() );
  assert( l.IsOrdered() );

  if(maxDist < 0 || (AlmostEqual(maxDist,0) && !allowEqual)){
    result.SetDefined(false);
    return;
  }

  HalfSegment hs1, hs2;
  double segDistance = -666.666;
  for( int i = 0; i < Size(); i++ ){
    Get( i, hs1 );
    if( hs1.IsLeftDomPoint() ) {
      for( int j = 0; j < l.Size(); j++ ) {
        l.Get( j, hs2 );
        if( hs1.Intersects( hs2, geoid ) ){
          result.Set(true,true);
          return;
        }
        segDistance = hs1.Distance( hs2, geoid );
        if( (segDistance < maxDist) ||
            (allowEqual && AlmostEqual(segDistance,maxDist))){
           result.Set(true,true);
           return;
        }
      }
    }
  }
  result.Set(true,false);
}

template<template<typename T>class Array>
double LineT<Array>::Distance( const Rectangle<2>& r,
                       const Geoid* geoid /*=0*/ ) const {
  assert( !IsEmpty() ); // includes !undef
  assert( IsOrdered() );
  assert( r.IsDefined() );
  assert( !geoid || geoid->IsDefined() );
  if(geoid){
    cout << __PRETTY_FUNCTION__ << ": Spherical geometry not implemented."
         <<endl;
    assert(false); // TODO: Implement spherical geometry case.
  }
  if( IsEmpty() || !r.IsDefined() || (geoid && !geoid->IsDefined()) ){
    return -1;
  }
  assert( IsOrdered() );
  HalfSegment hs;
  double dist = std::numeric_limits<double>::max();
  for(int i=0; i < line.Size() && dist>0; i++){
    line.Get(i,hs);
    if(hs.IsLeftDomPoint()){
      double d = hs.Distance(r,geoid);
      if(d<dist){
          dist = d;
      }
    }
  }
  return dist;
}

template<template<typename T>class Array>
bool LineT<Array>::Intersects( const Rectangle<2>& r,
                       const Geoid* geoid /*=0*/ ) const {
  assert( !IsEmpty() ); // includes !undef
  assert( IsOrdered() );
  assert( r.IsDefined() );
  assert( !geoid || geoid->IsDefined() );
  if(!BoundingBox().Intersects(r,geoid)){
     return false;
  }

  if(geoid){
    cout << __PRETTY_FUNCTION__ << ": Spherical geometry not implemented."
         <<endl;
    assert(false); // TODO: Implement spherical geometry case.
  }
  HalfSegment hs;
  for(int i=0;i<line.Size();i++){
    line.Get(i,hs);
    if(hs.IsLeftDomPoint()){
       if(hs.Intersects(r,geoid)){
         return true;
       }
    }
  }
  return false;
}


template<template<typename T>class Array>
double LineT<Array>::MaxDistance( const Rectangle<2>& r,
                          const Geoid* geoid /*=0*/ ) const
{
  assert( !IsEmpty() ); // includes !undef
  assert( r.IsDefined() );
  assert( !geoid || geoid->IsDefined() );
  if(geoid){
    cout << __PRETTY_FUNCTION__ << ": Spherical geometry not implemented."
         <<endl;
    assert(false); // TODO: Implement spherical geometry case.
  }
  if( IsEmpty() || !r.IsDefined() || (geoid && !geoid-IsDefined()) ){
    return -1;
  }
  assert( IsOrdered() );
  HalfSegment hs;
  double dist = 0;
  for(int i=0; i < line.Size(); i++){
    line.Get(i,hs);
    if(hs.IsLeftDomPoint()){
      double d = hs.MaxDistance(r,geoid);
      if(d > dist){
          dist = d;
      }
    }
  }
  return dist;
}


template<template<typename T>class Array>
int LineT<Array>::NoComponents() const {
  return noComponents;
}

template<template<typename T>class Array>
template<template<typename T2>class Array2>
void LineT<Array>::Translate( const Coord& x, const Coord& y, 
                              LineT<Array2>& result ) const
{
result.Clear();
if(!IsDefined()){
  result.SetDefined(false);
  return;
}
result.SetDefined(true);

assert( IsOrdered() );
result.Resize(this->Size());
result.bbox = this->bbox;
HalfSegment hs;
for( int i = 0; i < Size(); i++ )
{
  Get( i, hs );
  hs.Translate( x, y );
  result.line.Put(i,hs);
}

double t[2] = {x, y};
result.bbox.Translate(t);
}

template<template<typename T>class Array>
template<template<typename T2>class Array2>
void LineT<Array>::Rotate( const Coord& x, const Coord& y,
                 const double alpha,
                 LineT<Array2>& result ) const
{
result.Clear();
if(!IsDefined()){
   result.SetDefined(false);
   return;
}
result.Resize(Size());
result.SetDefined(true);

double s = sin(alpha);
double c = cos(alpha);

double m00 = c;
double m01 = -s;
double m02 = x - x*c + y*s;
double m10 = s;
double m11 = c;
double m12 = y - x*s-y*c;


result.StartBulkLoad();
HalfSegment hso;
Point p1;
Point p2;

for( int i = 0; i < Size(); i++ )
{
  Get( i, hso );
  p1.Set( m00*hso.GetLeftPoint().GetX()
          + m01*hso.GetLeftPoint().GetY() + m02,
          m10*hso.GetLeftPoint().GetX()
         + m11*hso.GetLeftPoint().GetY() + m12);
  p2.Set( m00*hso.GetRightPoint().GetX()
           + m01*hso.GetRightPoint().GetY() + m02,
           m10*hso.GetRightPoint().GetX()
           + m11*hso.GetRightPoint().GetY() + m12);

  HalfSegment hsr(hso); // ensure to copy attr;
  hsr.Set(hso.IsLeftDomPoint(),p1,p2);
  result += hsr;
}
result.EndBulkLoad(); // reordering may be required

}

template<template<typename T>class Array>
template<template<typename T2>class Array2>
void LineT<Array>::Scale( const Coord& sx, const Coord& sy,
                  LineT<Array2>& result ) const {
  result.Clear();
  if(!IsDefined()){
     result.SetDefined(false);
     return;
  }
  result.Resize(Size());
  result.SetDefined(true);

  result.StartBulkLoad();
  HalfSegment hso;

  for( int i = 0; i < Size(); i++ ) {
     Get( i, hso );
     if(hso.Scale(sx,sy)){
        result += hso;
     }
  }
  result.EndBulkLoad(); // reordering may be required
}

template<template<typename T>class Array>
template<template<typename T2>class Array2>
void LineT<Array>::Transform( RegionT<Array2>& result ) const
{
  result.Clear();
  if( !IsDefined() ){
    result.SetDefined( false );
    return;
  }
  result.SetDefined( true );
  if(Size()==0){
     return;
  }

  char* usage = new char[Size()];
  char* critical = new char[this->Size()];
  markUsage(&(this->line), usage, critical);


  Array<HalfSegment>* halfsegments = new Array<HalfSegment>(this->Size());
  if( !IsEmpty() ) {
    assert( IsOrdered() );
    HalfSegment hs;
    int edgeno=0;
    for( int i = 0; i < Size(); i++ )
    {
      if(usage[i] != 1){
        assert(usage[i]!=3);
        Get( i, hs );
        if(hs.IsLeftDomPoint()){
           hs.attr.edgeno = edgeno;
           halfsegments->Append(hs);
           hs.SetLeftDomPoint(false);
           halfsegments->Append(hs);
           edgeno++;
        }
      }
    }
  }
  halfsegments->Sort(HalfSegmentCompare);
  RegionCreator<Array>::setPartnerNo(halfsegments);
  RegionCreator<Array>::createRegion(halfsegments,&result);
  halfsegments->destroyIfNonPersistent();
  delete halfsegments;
  delete[] usage;
  delete[] critical;
}



template<template<typename T>class Array>
void LineT<Array>::Realminize(){
 // special case: empty line
if(!IsDefined()){
  line.clean();
  return;
}

LineT<Array> tmp(0);
Realminize2(*this,tmp);
*this = tmp;
}


template<template<typename T>class Array>
template<template<typename T2>class Array2>
void LineT<Array>::Boundary(PointsT<Array2>* result) const{
// we assume that the interior of each halfsegment has no
// common point with any part of another halfsegment
result->Clear();
if(!IsDefined()){
   result->SetDefined(false);
   return;
}
result->SetDefined(true);
if(IsEmpty()){
  return;
}
HalfSegment hs;
HalfSegment hs_n; // neighbooring halfsegment
Point p;
int size = Size();
result->StartBulkLoad();
for(int i=0;i<size;i++){
   bool common = false;
   Get(i,hs);
   p=hs.GetDomPoint();
   if(i>0){
     Get(i-1,hs_n);
     if(p==hs_n.GetDomPoint()){
        common=true;
     }
   }
   if(i<size-1){
     Get(i+1,hs_n);
     if(p==hs_n.GetDomPoint()){
        common=true;
     }
   }
   if(!common){
      *result += p;
   }
}
result->EndBulkLoad(false,false);

}


template<template<typename T>class Array>
bool LineT<Array>::Find( const HalfSegment& hs, int& pos, 
                         const bool& exact ) const
{
assert( IsDefined() );
assert( IsOrdered() );
if( exact )
  return line.Find( &hs, HalfSegmentCompare, pos );
return line.Find( &hs, HalfSegmentCompare, pos );
}

template<template<typename T>class Array>
bool LineT<Array>::Find( const Point& p, int& pos, const bool& exact ) const
{
assert( IsDefined() );
assert( p.IsDefined() );
assert( IsOrdered() );
if( exact )
  return line.Find( &p, PointHalfSegmentCompare, pos );
return line.Find( &p, PointHalfSegmentCompareAlmost, pos );
}

template<template<typename T>class Array>
void LineT<Array>::SetPartnerNo() {
if(!IsDefined() || line.Size()==0){
 return;
}

// reserve a slot for each edgeno
int tmpsize = (line.Size()+1)/2;

DbArray<int> TMP(tmpsize);
  // intialize the array (it's only needed for
  // wrong sorted halfsegments)
for(int i=0;i<tmpsize+1;i++){
  TMP.Put(i,-1);
}


HalfSegment hs1;
HalfSegment hs2;
int lpp;
for(int i=0; i<line.Size(); i++){
  line.Get(i,hs1);
  TMP.Get(hs1.attr.edgeno,lpp);
  if(hs1.IsLeftDomPoint()){
    if(lpp>=0){ // error case, segment already registered
      std::cerr << "wrong order in halfsegment array" << std::endl;
      std::cerr << "the system may be instable" << std::endl;
      int leftpos = lpp;
      HalfSegment right = hs1;
      right.attr.partnerno = leftpos;
      right.attr.insideAbove = false;
      right.attr.coverageno = 0;
      right.attr.cycleno = 0;
      right.attr.faceno = 0;
      line.Get(leftpos,hs2);
      HalfSegment left = hs2;
      left.attr.partnerno = i;
      left.attr.insideAbove = false;
      left.attr.coverageno = 0;
      left.attr.cycleno = 0;
      left.attr.faceno = 0;
      line.Put(i,right);
      line.Put(leftpos,left);
    } else {
      // normal case, put number to tmp
      TMP.Put(hs1.attr.edgeno, i);
    }
  } else { // RightDomPoint
    if(lpp<0){ // error case left segment not found
      std::cerr << "Error in halfsegment array detected" << std::endl;
      std::cerr << "may be a wrong ordering or wrong set edgeno's" << std::endl;
      TMP.Put(hs1.attr.edgeno, i);
    } else {
      int leftpos = lpp;
      HalfSegment right = hs1;
      right.attr.partnerno = leftpos;
      right.attr.insideAbove = false;
      right.attr.coverageno = 0;
      right.attr.cycleno = 0;
      right.attr.faceno = 0;
      line.Get(leftpos,hs2);
      HalfSegment left = hs2;
      left.attr.partnerno = i;
      left.attr.insideAbove = false;
      left.attr.coverageno = 0;
      left.attr.cycleno = 0;
      left.attr.faceno = 0;
      line.Put(i,right);
      line.Put(leftpos,left);
    }
  }
 }
  TMP.Destroy();
}

template<template<typename T>class Array>
bool LineT<Array>::GetNextSegment( const int poshs, const HalfSegment& hs,
                           int& posnexths, HalfSegment& nexths )
{
  if( poshs > 0 )
  {
    Get( posnexths = poshs - 1, nexths );
    if( hs.GetDomPoint() == nexths.GetDomPoint() )
      return true;
  }

  if( poshs < Size() - 1 )
  {
    Get( posnexths = poshs + 1, nexths );
    if( hs.GetDomPoint() == nexths.GetDomPoint() )
      return true;
  }

  return false;
}

template<template<typename T>class Array>
bool LineT<Array>::GetNextSegments( const int poshs, const HalfSegment& hs,
                            std::vector<bool>& visited,
                            int& posnexths, HalfSegment& nexths,
                            std::stack< std::pair<int,  HalfSegment> >& nexthss)
{
  bool first = true;
  HalfSegment aux;

  int auxposhs = poshs;
  while( auxposhs > 0 )
  {
    auxposhs--;
    if( !visited[auxposhs] )
    {
      Get( auxposhs, aux );
      if( hs.GetDomPoint() == aux.GetDomPoint() )
      {
        if( first )
        {
          first = false;
          nexths = aux;
          posnexths = auxposhs;
        }
        else
          nexthss.push( std::make_pair( auxposhs, aux ) );
      }
      else
        break;
    }
  }

  auxposhs = poshs;
  while( auxposhs < Size() - 1 )
  {
    auxposhs++;
    if( !visited[auxposhs] )
    {
      Get( auxposhs, aux );
      if( hs.GetDomPoint() == aux.GetDomPoint() )
      {
        if( first )
        {
          first = false;
          nexths = aux;
          posnexths = auxposhs;
        }
        else
          nexthss.push( std::make_pair( auxposhs, aux ) );
      }
      else
        break;
    }
  }

  return !first;
}

/*
~computeComponents~

Computes FaceNo, edgeno of each halfsegment.
Sets length,noComponents, and bbox of the line.


*/

template<template<typename T>class Array>
int LineT<Array>::getUnusedExtension(int startPos,const Array<bool>& used)const{
  HalfSegment hs;
  line.Get(startPos,hs);
  Point p = hs.GetDomPoint();
  int pos = startPos-1;
  bool done = false;
  bool u;
  // search on the left side
  while(pos>=0 && !done){
     line.Get(pos,hs);
     Point p2 = hs.GetDomPoint();
     if(!AlmostEqual(p,p2)){
       done = true;
     }else {
       used.Get(pos,u);
       if(!u){
         return pos;
       } else {
         pos--;
       }
     }
  }
  // search on the right side
  done = false;
  pos = startPos+1;
  int size = line.Size();
  while(!done && pos<size){
     line.Get(pos,hs);
     Point p2 = hs.GetDomPoint();
     if(!AlmostEqual(p,p2)){
       done = true;
     } else {
       used.Get(pos,u);
       if(!u){
         return pos;
       } else {
        pos++;
       }
     }
  }
  return -1;
}

template<template<typename T>class Array>
void LineT<Array>::collectFace(int faceno, int startPos, Array<bool>& used){
  std::set<int> extensionPos;

  used.Put(startPos,true);
  HalfSegment hs1;
  HalfSegment hs2;

  int pos = startPos;
  line.Get(startPos,hs1);
  HalfSegment Hs1 = hs1;
  int edgeno = 0;
  Hs1.attr.insideAbove=false;
  Hs1.attr.coverageno = 0;
  Hs1.attr.cycleno=0;
  Hs1.attr.faceno=faceno;
  Hs1.attr.edgeno = edgeno;
  line.Put(pos,Hs1);
  used.Put(pos,true);

  // get and Set the Partner
  int partner = Hs1.attr.partnerno;
  line.Get(partner,hs2);
  HalfSegment Hs2 = hs2;
  Hs2.attr.insideAbove=false;
  Hs2.attr.coverageno = 0;
  Hs2.attr.cycleno=0;
  Hs2.attr.faceno=faceno;
  Hs2.attr.edgeno = edgeno;
  used.Put(partner,true);
  line.Put(partner,Hs2);

  if(!bbox.IsDefined()){
    bbox = hs1.BoundingBox();
  } else {
    bbox = bbox.Union(hs1.BoundingBox());
  }
  length += hs1.Length();

  if(getUnusedExtension(pos,used)>=0){
     extensionPos.insert(pos);
  }
  if(getUnusedExtension(partner,used)>=0){
     extensionPos.insert(partner);
  }

  edgeno++;
  while(!extensionPos.empty()){

    int spos =  *(extensionPos.begin());
    pos = getUnusedExtension(spos,used);
    if(pos < 0){
      extensionPos.erase(spos);
    } else { // extension found at position pos
      line.Get(pos,hs1);
      Hs1 = (hs1);
      Hs1.attr.insideAbove=false;
      Hs1.attr.coverageno = 0;
      Hs1.attr.cycleno=0;
      Hs1.attr.faceno=faceno;
      Hs1.attr.edgeno = edgeno;
      used.Put(pos,true);
      line.Put(pos,Hs1);

      partner = Hs1.attr.partnerno;
      line.Get(partner,hs2);
      Hs2 = (hs2);
      Hs2.attr.insideAbove=false;
      Hs2.attr.coverageno = 0;
      Hs2.attr.cycleno=0;
      Hs2.attr.faceno=faceno;
      Hs2.attr.edgeno = edgeno;
      used.Put(partner,true);
      line.Put(partner,Hs2);
      if(getUnusedExtension(partner,used)>=0){
        extensionPos.insert(partner);
      }
      length += hs1.Length();
      bbox = bbox.Union(hs1.BoundingBox());
      edgeno++;
    }
  }
}

/*
~ComputeComponents~

Computes the length of this lines as well as its bounding box and the number
of components of this line. Each Halfsegment is assigned to a face number
(the component) and an egde number within this face.



*/

template<template<typename T>class Array>
void LineT<Array>::computeComponents() {
  length = 0.0;
  noComponents = 0;
  bbox.SetDefined(false);

  if(!IsDefined() || Size()==0){
    return;
  }

  Array<bool> used(line.Size());

  for(int i=0;i<line.Size();i++){
    used.Append(false);
  }

  int faceno = 0;

  bool u;
  for(int i=0;i<line.Size();i++){
    used.Get(i,u);
    if(!(u)){ // an unused halfsegment
      collectFace(faceno,i,used);
      faceno++;
    }
  }
  noComponents = faceno;
  used.Destroy();
}

template<template<typename T>class Array>
void LineT<Array>::Sort()
{
  assert( !IsOrdered() );
  line.Sort( HalfSegmentCompare );
  ordered = true;
}

template<template<typename T>class Array>
void LineT<Array>::RemoveDuplicates()
{
  assert( IsOrdered() );
  int size = line.Size();
  if( size == 0 ){
   // nothing to do
     return;
  }

  int newEdgeNumbers[size]; // mapping oldnumber -> newnumber
  for(int i=0;i<size;i++){
     newEdgeNumbers[i] = -1;
  }
  int newedge = 0;  // next unused edge number

  HalfSegment tmp;
  HalfSegment last;
  HalfSegment ptmp;
  int pos = 0;
  line.Get( 0, ptmp );
  last = ptmp;

  newEdgeNumbers[last.attr.edgeno] = newedge;
  last.attr.edgeno = newedge;
  line.Put(0,last);
  newedge++;

  for( int i = 1; i < size; i++ )
  {
    line.Get( i, ptmp );
    tmp = ptmp;
    int edge = tmp.attr.edgeno;
    if( last != tmp )
      // new segment found
    {
      pos++;
      if(newEdgeNumbers[edge]<0){ // first
         newEdgeNumbers[edge] = newedge;
         tmp.attr.edgeno = newedge;
         newedge++;
      } else {
         tmp.attr.edgeno = newEdgeNumbers[edge];
      }
      line.Put( pos, tmp );
      last = tmp;
    } else { // duplicate found
      newEdgeNumbers[edge] = newedge-1; // use the last edge number
    }
  }
  if( pos + 1 != size ){
    // duplicates found
    line.resize( pos + 1 );
  }
}

template<template<typename T>class Array>
template<template<typename T2>class Array2>
void LineT<Array>::WindowClippingIn( const Rectangle<2> &window,
                             LineT<Array2> &clippedLine,
                             bool &inside ) const
{
  clippedLine.Clear();
  if( !IsDefined() || !window.IsDefined() ) {
    clippedLine.SetDefined( false );
    return;
  }
  clippedLine.SetDefined( true );
  inside = false;
  clippedLine.StartBulkLoad();
  for (int i=0; i < Size();i++)
  {
    HalfSegment hs;
    HalfSegment hsInside;
    bool insidehs = false,
         isIntersectionPoint = false;
    Get( i, hs );

    if( hs.IsLeftDomPoint() )
    {
      Point intersectionPoint;
      hs.WindowClippingIn( window, hsInside, insidehs,
                            isIntersectionPoint,
                            intersectionPoint );
      if( insidehs && !isIntersectionPoint )
      {
        clippedLine += hsInside;
        hsInside.SetLeftDomPoint( !hsInside.IsLeftDomPoint() );
        clippedLine += hsInside;
        inside = true;
      }
    }
  }
  clippedLine.EndBulkLoad();
}

template<template<typename T>class Array>
template<template<typename T2>class Array2>
void LineT<Array>::WindowClippingOut( const Rectangle<2> &window,
                              LineT<Array2> &clippedLine,
                              bool &outside ) const
{
  clippedLine.Clear();
  if( !IsDefined() || !window.IsDefined() ) {
    clippedLine.SetDefined( false );
    return;
  }
  clippedLine.SetDefined( true );
  outside = false;
  clippedLine.StartBulkLoad();
  for (int i=0; i < Size();i++)
  {
    HalfSegment hs;
    HalfSegment hsInside;
    bool outsidehs = false,
         isIntersectionPoint = false;
    Get( i, hs );

    if( hs.IsLeftDomPoint() )
    {
      Point intersectionPoint;
      hs.WindowClippingIn( window, hsInside, outsidehs,
                            isIntersectionPoint,
                            intersectionPoint );
      if( outsidehs && !isIntersectionPoint )
      {
        if( hs.GetLeftPoint() != hsInside.GetLeftPoint() )
        {//Add the part of the half segment composed by the left
         // point of hs and the left point of hsInside.
          HalfSegment hsLeft( true,
                              hs.GetLeftPoint(),
                              hsInside.GetLeftPoint() );
          AttrType attr = hs.GetAttr();
          hsLeft.SetAttr(attr);
          clippedLine += hsLeft;
          hsLeft.SetLeftDomPoint( !hsLeft.IsLeftDomPoint() );
          clippedLine += hsLeft;
          outside = true;
        }
        if( hs.GetRightPoint() != hsInside.GetRightPoint() )
        {//Add the part of the half segment composed by the left
         // point of hs and the left point of hsInside.
          HalfSegment hsRight( true,
                               hs.GetRightPoint(),
                               hsInside.GetRightPoint() );
          AttrType attr=hs.GetAttr();
          hsRight.SetAttr(attr);
          clippedLine += hsRight;
          hsRight.SetLeftDomPoint( !hsRight.IsLeftDomPoint() );
          clippedLine += hsRight;
          outside = true;
        }
      }
      else
      {
        HalfSegment auxhs = hs;
        clippedLine += auxhs;
        auxhs.SetLeftDomPoint( !auxhs.IsLeftDomPoint() );
        clippedLine += auxhs;
        outside = true;
      }
    }
  }
  clippedLine.EndBulkLoad();
}


template<template<typename T>class Array>
std::ostream& operator<<( std::ostream& os, const LineT<Array>& cl )
{
  os << "<";
  if( !cl.IsDefined() ) {
    os << " undefined ";
  } else {
    HalfSegment hs;
    for( int i = 0; i < cl.Size(); i++ )
    {
      cl.Get( i, hs );
      os << " " << hs << endl;
    }
  }
  os << ">";
  return os;
}


template<template<typename T>class Array>
size_t LineT<Array>::HashValue() const
{
  if( IsEmpty() ) // subsumes undef
    return 0;

  size_t h = 0;

  HalfSegment hs;
  Coord x1, y1, x2, y2;

  for( int i = 0; i < Size() && i < 5; i++ )
  {
    Get( i, hs );
    x1 = hs.GetLeftPoint().GetX();
    y1 = hs.GetLeftPoint().GetY();
    x2 = hs.GetRightPoint().GetX();
    y2 = hs.GetRightPoint().GetY();

    h += (size_t)( (5 * x1 + y1) + (5 * x2 + y2) );
  }
  return h;
}

template<template<typename T>class Array>
void LineT<Array>::Clear()
{
  line.clean();
  pos = -1;
  ordered = true;
  bbox.SetDefined( false );
  SetDefined(true);
}

template<template<typename T>class Array>
void LineT<Array>::CopyFrom( const Attribute* right )
{
  *this = *((const LineT<Array> *)right);
}

template<template<typename T>class Array>
template<template<typename T2>class Array2>
void LineT<Array>::CopyFrom( const LineT<Array2>* right )
{
  *this = *right;
}



template<template<typename T>class Array>
int LineT<Array>::Compare( const Attribute *arg ) const
{
   return Compare( *(LineT<Array>*) arg);

}


template<template<typename T>class Array>
template<template<typename T2>class Array2>
int LineT<Array>::Compare( const LineT<Array2>& l ) const
{
  if(!IsDefined() && !l.IsDefined()){
   return 0;
  }
  if(!IsDefined()){
   return -1;
  }
  if(!l.IsDefined()){
   return 1;
  }

  if( Size() > l.Size() )
    return 1;
  if( Size() < l.Size() )
    return -1;

  if(Size()==0){
    return 0;
  }

  object obj;
  status stat;
  SelectFirst_ll( *this, l, obj, stat );

  while( stat == endnone )
  {
    if( obj == first )
      return -1;
    if( obj == second )
      return 1;

    SelectNext_ll( *this, l, obj, stat );
  }
  return 0;
}

template<template<typename T>class Array>
LineT<Array>* LineT<Array>::Clone() const
{
  return new LineT<Array>( *this );
}


template<template<typename T>class Array>
double LineT<Array>::Length(const Geoid &geoid, bool& valid) const {
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
      length += hs.LengthOrthodrome(geoid, valid);;
    };
  }
  return length;
}

template<template<typename T>class Array>
std::ostream& LineT<Array>::Print( std::ostream &os ) const
{
  std::ios_base::fmtflags oldOptions = os.flags();
  os.setf(std::ios_base::fixed,std::ios_base::floatfield);
  os.precision(8);
  os << "<";
  if( !IsDefined() ) {
    os << " undefined ";
  } else {
    HalfSegment hs;
    for( int i = 0; i < Size(); i++ )
    {
      Get( i, hs );
      os << " " << hs << endl;
    }
  }
  os << ">";
  os.flags(oldOptions);
  return os;
}

template<template<typename T>class Array>
template<template<typename T2>class Array2,
         template<typename T3>class Array3>
void LineT<Array>::Simplify(LineT<Array2>& result, const double epsilon,
                    const PointsT<Array3>& importantPoints /*= Points(0)*/ ,
                    const Geoid* geoid /*= 0*/) const{
 result.Clear(); // remove old stuff

 if( !IsDefined() || (geoid && !geoid->IsDefined()) ){ // this/geoid undefined
    result.SetDefined(false);
    return;
 }
 result.SetDefined(true);
 if(epsilon<=0){ // maximum derivation reached in each case
     result.CopyFrom(this);
     return;
 }
 // at least one immediate point is required to simplify
 // the line, thus at leat 4 halfsegments are needed.
 if(Size()<4){
    result.CopyFrom(this);
    return;
 }
 // an array with the used halfsegments
 bool used[Size()];
 for(int i=0;i<Size();i++){
    used[i] = false;
 }

 std::vector<Point> forward;
 std::vector<Point> backward;
 std::vector<Point> complete;

 HalfSegment hs; // current halfsegment

 result.StartBulkLoad();
 int pos = 0;
 int size = Size();
 int egdeno=0;
 while(pos<size){
  // skip all halfsegments in prior runs
  while(pos<size && used[pos]){
    pos++;
  }

  if(pos<size){
     // unused halfsegment found
     forward.clear();
     backward.clear();
     int next = pos;
     // trace the polyline until it ends or critical point is reached
     bool done = false;
     while(!done){
        used[next]= true;
        Get(next,hs);
        forward.push_back(hs.GetDomPoint());
        int partner = hs.attr.partnerno;
        used[partner] = true;
        Get(partner,hs);
        Point p = hs.GetDomPoint();
        if(importantPoints.Contains(p)){
          done = true;
        }else {
           int s = std::max(0,partner-2);
           int e = std::min(partner+3,Size());
           int count =0;
           HalfSegment tmp;

           // search around partner for segments with an
           // equal dominating point.
          for(int k=s; (k<e) && (count < 2); k++){
             if(k!=partner){
                Get(k,tmp);
                Point p2 = tmp.GetDomPoint();
                if(AlmostEqual(p,p2)){
                   count++;
                   next = k;
                }
             }
           }
           done = (count != 1) || used[next];
        }
     }
     forward.push_back(hs.GetDomPoint());

     // check possible enlargement into the other direction
     next = pos; // start again at pos
     done = false;
     do{
        Get(next,hs);
        Point p = hs.GetDomPoint();
        // check whether next has an unique connected segment
        int s = std::max(0, next-2);
        int e = std::min(next+3,Size());
        int count = 0;
        HalfSegment tmp;
        int partner = 0;
        // search around next
        if(importantPoints.Contains(p)){
          done = true;
        } else {
           for(int k=s; (k<e) && (count <2); k++){
              if(k!=next){
                 Get(k,tmp);
                 Point p2 = tmp.GetDomPoint();
                 if(AlmostEqual(p,p2)){
                   count++;
                   partner = k;
                 }
              }
           }
           done = (count!=1) || used[partner];
           if(!done){ // extension found
               used[partner] = true;
               Get(partner,hs);
               next = hs.attr.partnerno;
               used[next] = true;
               Get(next,hs);
               backward.push_back(hs.GetDomPoint());
           }
       }
     } while(!done);
     // connect backward and forward into complete
     complete.clear();
     for(int i=backward.size()-1; i>=0; i--){
       complete.push_back(backward[i]);
     }
     for(unsigned int i=0;i<forward.size(); i++){
       complete.push_back(forward[i]);
     }


     // determine all segments to use and copy them into the result
     bool use[complete.size()];
     douglas_peucker(complete,epsilon,use,false,geoid);
     int size = complete.size();
     Point last = complete[0];
     for(int i=1;i<size;i++){
        if(use[i]){
           Point p = complete[i];
           HalfSegment nhs(true,last,p);
           nhs.attr.edgeno=egdeno;
           result += nhs;
           nhs.SetLeftDomPoint(false);
           result += nhs;
           egdeno++;
           last = p;
        }
     }

   }
 }
 result.EndBulkLoad();
}


template<template<typename T>class Array>
template<template<typename T2>class Array2>
void LineT<Array>::Vertices( PointsT<Array2>* result ) const
{
result->Clear();
if(!IsDefined()){
  result->SetDefined(false);
  return;
}
result->SetDefined(true);
if( IsEmpty() ){
  return;
}

assert( IsOrdered() );
HalfSegment hs;
result->StartBulkLoad();
for( int i = 0; i < Size(); i++ )
{
  Get( i, hs );
  *result += hs.GetDomPoint();
}
result->EndBulkLoad( false, true );
}



#endif

