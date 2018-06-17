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


#include "RegionT.h"
#include "SimpleLineT.h"
#include "RegionCreator.h"
#include <map>
#include <vector>

#include "RobustSetOps.h"

#include "WinUnix.h"

/*
11.4 Implementation of Class ~Region~

11.4.1 Constructors

*/
template<template<typename T>class Array>
inline RegionT<Array>::RegionT( const int initsize ) :
StandardSpatialAttribute<2>(true),
region( initsize ),
bbox( false ),
noComponents( 0 ),
ordered( true )
{
 }


bool IsInsideAbove(const HalfSegment& hs,
                   const Point& thirdPoint);


template<template<typename T>class Array>
RegionT<Array>::RegionT( const Point& p1, const Point& p2, const Point& p3 ):
   StandardSpatialAttribute<2>(true),region(6)
{
  Clear();
  if( !p1.IsDefined() || !p2.IsDefined() || !p3.IsDefined() ){
    SetDefined(false); // UNDEFINED region
  } else if(AlmostEqual(p1,p2) || AlmostEqual(p2,p3) || AlmostEqual(p3,p1) ){
    SetDefined(true);  // EMPTY region
  } else {             // triangular region
    SetDefined(true);
    StartBulkLoad();

    HalfSegment hs;
    int edgecnt = 0;

    hs.Set(true,p1,p2);
    hs.attr.faceno = 0;         // only one face
    hs.attr.cycleno = 0;        // only one cycle
    hs.attr.edgeno = edgecnt;
    hs.attr.partnerno = edgecnt++;
    hs.attr.insideAbove = IsInsideAbove(hs,p3);
    *this += hs;
    hs.SetLeftDomPoint( !hs.IsLeftDomPoint() );
    *this += hs;

    hs.Set(true,p2,p3);
    hs.attr.faceno = 0;         // only one face
    hs.attr.cycleno = 0;        // only one cycle
    hs.attr.edgeno = edgecnt;
    hs.attr.partnerno = edgecnt++;
    hs.attr.insideAbove = IsInsideAbove(hs,p1);
    *this += hs;
    hs.SetLeftDomPoint( !hs.IsLeftDomPoint() );
    *this += hs;

    hs.Set(true,p3,p1);
    hs.attr.faceno = 0;         // only one face
    hs.attr.cycleno = 0;        // only one cycle
    hs.attr.edgeno = edgecnt;
    hs.attr.partnerno = edgecnt++;
    hs.attr.insideAbove = IsInsideAbove(hs,p2);
    *this += hs;
    hs.SetLeftDomPoint( !hs.IsLeftDomPoint() );
    *this += hs;

    EndBulkLoad();
  }
}

template<template<typename T>class Array>
template<template<typename T2> class Array2>
RegionT<Array>::RegionT( const RegionT<Array2>& cr, bool onlyLeft ) :
StandardSpatialAttribute<2>(cr.IsDefined()),
region( cr.Size() ),
bbox( cr.bbox ),
noComponents( cr.NoComponents() ),
ordered( true )
{

  if( IsDefined() && cr.Size() >0 ) {
    assert( cr.IsOrdered() );
    if( !onlyLeft ){
      convertDbArrays(cr.region,region);
    } else {
      StartBulkLoad();
      HalfSegment hs;
      int j=0;
      for( int i = 0; i < cr.Size(); i++ ) {
        cr.Get( i, hs );
        if (hs.IsLeftDomPoint()) {
          region.Put( j, hs );
          j++;
        }
      }
      EndBulkLoad( false, false, false, false );
    }
  }
}


template<template<typename T>class Array>
RegionT<Array>::RegionT( const RegionT& cr) :
StandardSpatialAttribute<2>(cr.IsDefined()),
region( cr.Size() ),
bbox( cr.bbox ),
ordered( true )
{
  if(IsDefined()) {
    // NoComponents() can only be called on defined regions
    noComponents = cr.NoComponents();

    if(cr.Size() > 0) {
       assert( cr.IsOrdered() );
       region.copyFrom(cr.region);
    }
  }
}

template<template<typename T>class Array>
template<template<typename T2> class Array2>
RegionT<Array>::RegionT( const RegionT<Array2>& cr) :
StandardSpatialAttribute<2>(cr.IsDefined()),
region( cr.Size() ),
bbox( cr.bbox ),
noComponents( cr.NoComponents() ),
ordered( true )
{
  if( IsDefined() && cr.Size() >0 ) {
    assert( cr.IsOrdered() );
    convertDbArrays(cr.region,region);
  }
}

template<template<typename T>class Array>
RegionT<Array>::RegionT( const Rectangle<2>& r )
   :StandardSpatialAttribute<2>(r.IsDefined()),
    region(8)
{
    Clear();
    if(  r.IsDefined() )
    {
      SetDefined( true);
      HalfSegment hs;
      int partnerno = 0;
      double min0 = r.MinD(0), max0 = r.MaxD(0),
      min1 = r.MinD(1), max1 = r.MaxD(1);

      Point v1(true, max0, min1),
      v2(true, max0, max1),
      v3(true, min0, max1),
      v4(true, min0, min1);

      if( AlmostEqual(v1, v2) ||
          AlmostEqual(v2, v3) ||
          AlmostEqual(v3, v4) ||
          AlmostEqual(v4, v1) )
      { // one interval is (almost) empty, so will be the region
        SetDefined( true );
        return;
      }

      SetDefined( true );
      StartBulkLoad();

      hs.Set(true, v1, v2);
      hs.attr.faceno = 0;         // only one face
      hs.attr.cycleno = 0;        // only one cycle
      hs.attr.edgeno = partnerno;
      hs.attr.partnerno = partnerno++;
      hs.attr.insideAbove = (hs.GetLeftPoint() == v1);
      *this += hs;
      hs.SetLeftDomPoint( !hs.IsLeftDomPoint() );
      *this += hs;

      hs.Set(true, v2, v3);
      hs.attr.faceno = 0;         // only one face
      hs.attr.cycleno = 0;        // only one cycle
      hs.attr.edgeno = partnerno;
      hs.attr.partnerno = partnerno++;
      hs.attr.insideAbove = (hs.GetLeftPoint() == v2);
      *this += hs;
      hs.SetLeftDomPoint( !hs.IsLeftDomPoint() );
      *this += hs;

      hs.Set(true, v3, v4);
      hs.attr.faceno = 0;         // only one face
      hs.attr.cycleno = 0;        // only one cycle
      hs.attr.edgeno = partnerno;
      hs.attr.partnerno = partnerno++;
      hs.attr.insideAbove = (hs.GetLeftPoint() == v3);
      *this += hs;
      hs.SetLeftDomPoint( !hs.IsLeftDomPoint() );
      *this += hs;

      hs.Set(true, v4, v1);
      hs.attr.faceno = 0;         // only one face
      hs.attr.cycleno = 0;        // only one cycle
      hs.attr.edgeno = partnerno;
      hs.attr.partnerno = partnerno++;
      hs.attr.insideAbove = (hs.GetLeftPoint() == v4);
      *this += hs;
      hs.SetLeftDomPoint( !hs.IsLeftDomPoint() );
      *this += hs;

      EndBulkLoad();
    }
    else {
      SetDefined( false );
    }
}


/*
Destroy

This will destroy the contained flob.

*/


template<template<typename T>class Array>
inline void RegionT<Array>::Destroy()
{
  region.destroy();
}

/*
~Bounding Box~

If the geoid is null, this function just returns the
internally stored box. Otherwise, a box is computed
according the given geoid.

*/

template<template<typename T>class Array>
inline const Rectangle<2> RegionT<Array>::BoundingBox(
                                  const Geoid* geoid /*=0*/) const
{
  if(geoid){ // spherical geometry case:
    if(!geoid->IsDefined() || !IsDefined()){
      return Rectangle<2>(false);
    }
    Rectangle<2> geobbox = Rectangle<2>(false);
    for (int i=0; i<Size() ;i++){
      HalfSegment hs;
      Get( i, hs );
      if( hs.IsLeftDomPoint() ){
        if(!geobbox.IsDefined()){
          geobbox = hs.BoundingBox(geoid);
        } else {
          geobbox = geobbox.Union(hs.BoundingBox(geoid));
        }
      } // else: ignore inverse HalfSegments
    } // endfor
  } // else: euclidean case
  return bbox;
}

/*
~isOrdered~

CHecks whether the halfsegment array is sorted.

*/

template<template<typename T>class Array>
inline bool RegionT<Array>::IsOrdered() const
{
  return ordered;
}


/*
~GetAttr~

Returns the attributes of the halfsegment stored at the 
specified position.

*/
template<template<typename T>class Array>
inline const AttrType& RegionT<Array>::GetAttr( int position ) const
{
  assert(( position>=0) && (position<=Size()-1));
  HalfSegment hs;
  region.Get( position, &hs);
  return hs.GetAttr();
}


/*
~UpdateAttr~

Changes the attributes of the halfsegment at the current position of
the 'iterator'.

*/
template<template<typename T>class Array>
inline void RegionT<Array>::UpdateAttr( AttrType& attr )
{
  if (( pos>=0) && (pos<=Size()-1))
  {
    HalfSegment hs;
    region.Get( pos, &hs);
    hs.SetAttr( attr );
    region.Put( pos, hs );
  }
}

/*
~UpdateAttr~

Changes the attributes of the halfssegment at the speicfied position,

*/

template<template<typename T>class Array>
inline void RegionT<Array>::UpdateAttr( int position, AttrType& attr )
{
  if (( position>=0) && (position<=Size()-1))
  {
    HalfSegment hs;
    region.Get( position, &hs );
    hs.SetAttr( attr );
    region.Put( position, hs );
  }
}

/*
Resets the internally 'iterator'.

*/

template<template<typename T>class Array>
inline void RegionT<Array>::SelectFirst() const
{
  if( IsEmpty() )
    pos = -1;
  else pos = 0;
}

/*
moves the internally iterator forward.

*/

template<template<typename T>class Array>
inline void RegionT<Array>::SelectNext() const
{
  if( pos >= 0 && pos < Size() - 1 )
    pos++;
  else pos = -1;
}

/*
Checks whether the internally iterator is at the end.

*/

template<template<typename T>class Array>
inline bool RegionT<Array>::EndOfHs() const
{
  return (pos==-1);
}


/*
Returns the halfsegment at the current iterator position.

*/
template<template<typename T>class Array>
inline bool RegionT<Array>::GetHs(HalfSegment& hs ) const
{
  assert( IsDefined() );
  if( pos >= 0 && pos <= Size()-1 )
  {
    region.Get( pos, &hs );
    return true;
  }
  return false;
}

/*
resizes the halfsegment array.

*/

template<template<typename T>class Array>
inline void RegionT<Array>::Resize(const int newSize){
  if(newSize>Size()){
    region.resize(newSize);
  }
}

/*
Resizes the halfsegment array to fit exactly the contained halfsegments.

*/
template<template<typename T>class Array>
inline void RegionT<Array>::TrimToSize(){
  region.TrimToSize();
}

/*
switches to bulkload mode

*/

template<template<typename T>class Array>
void RegionT<Array>::StartBulkLoad()
{
  ordered = false;
}


/*
Finishes the bulkload mode.
Be carefully with the flags. 

*/
template<template<typename T>class Array>
void RegionT<Array>::EndBulkLoad( bool sort, bool setCoverageNo,
                          bool setPartnerNo, bool computeRegion )
{
  if( !IsDefined() ) {
    Clear();
    return;
  }

  if( sort ) {
    Sort();
  }

  if( setCoverageNo )
  {
    HalfSegment hs;
    int currCoverageNo = 0;

    for( int i = 0; i < this->Size(); i++ )
    {
      Get( i, hs );

      if( hs.IsLeftDomPoint() )
        currCoverageNo++;
      else
        currCoverageNo--;

      hs.attr.coverageno = currCoverageNo;

      region.Put( i, hs );
    }
  }

  if( setPartnerNo )
    SetPartnerNo();


  if( computeRegion ) {
   try{
      ComputeRegion();
   } catch(...){
      RegionCreator<Array>::createRegion(&region,this);   
   }
  }

  region.TrimToSize();
  ordered = true;
}


/*
Checks whether the region contains p. Returns also true if the p 
is located of the region's boundary.

*/
template<template<typename T>class Array>
bool RegionT<Array>::Contains( const Point& p, const Geoid* geoid/*=0*/ ) const
{

  assert( IsDefined() );
  assert( p.IsDefined() );
  assert( !geoid || geoid->IsDefined() );

  if( IsEmpty() || !p.IsDefined() || (geoid && !geoid->IsDefined())){
    return false;
  }
  if( !p.Inside(bbox) ){
    return false;
  }
  assert( IsOrdered() );
  std::map<int, int> faceISN;
  HalfSegment hs;

  int coverno=0;
  int startpos=0;
  double y0;

  //1. find the right place by binary search
  if( Find( p, startpos ) ){
    return true;
  }
  if ( startpos == 0 ){   //p is smallest
    return false;
  } else if ( startpos == Size() ){  //p is largest
    return false;
  }

  int i = startpos - 1;

  //2. deal with equal-x hs's
  region.Get( i, hs );
  while( i > 0 &&
         hs.GetDomPoint().GetX() == p.GetX() ){
    if( hs.Contains(p,geoid) ){
      return true;
    }
    if( hs.IsLeftDomPoint() && hs.RayAbove( p, y0 ) ){
      if( faceISN.find( hs.attr.faceno ) != faceISN.end() ){
        faceISN[ hs.attr.faceno ]++;
      } else {
        faceISN[ hs.attr.faceno ] = 1;
      }
    }
    region.Get( --i, hs );
  }

  // at this point, i is pointing to the last hs whose dp.x != p.x

  //3. get the coverage value
  coverno = hs.attr.coverageno;

  //4. search the region value for coverageno steps
  int touchedNo = 0;
  while( i >= 0 && touchedNo < coverno ){
    this->Get(i, hs);
    if( hs.Contains(p) ){
      return true;
    }
    if( hs.IsLeftDomPoint() &&
        hs.GetLeftPoint().GetX() <= p.GetX() &&
        p.GetX() <= hs.GetRightPoint().GetX() ){
      touchedNo++;
    }
    if( hs.IsLeftDomPoint() && hs.RayAbove( p, y0 ) ){
      if( faceISN.find( hs.attr.faceno ) != faceISN.end() ){
        faceISN[ hs.attr.faceno ]++;
      } else {
        faceISN[ hs.attr.faceno ] = 1;
      }
    }
    i--;  //the iterator
  }

  for( std::map<int, int>::iterator iter = faceISN.begin();
       iter != faceISN.end();
       iter++ ){
    if( iter->second % 2 != 0 ){
      return true;
    }
  }
  return false;
}

/*
Checks whether p is located in the interior of this region.

*/

template<template<typename T>class Array>
bool RegionT<Array>::InnerContains( const Point& p, 
                                    const Geoid* geoid/*=0*/ ) const
{
  assert( IsDefined() );
  assert( p.IsDefined() );
  assert( !geoid || geoid->IsDefined() );

  if( IsEmpty() || !p.IsDefined() || (geoid && !geoid->IsDefined())){
    return false;
  }
  if( !p.Inside(bbox) ){
    return false;
  }
  assert( IsOrdered() );
  std::map<int, int> faceISN;
  HalfSegment hs;

  int coverno = 0,
      startpos = 0;
  double y0;

  //1. find the right place by binary search
  if( Find( p, startpos ) ){
    return false;
  }
  if ( startpos == 0 ){   //p is smallest
    return false;
  } else if ( startpos == Size() ){  //p is largest
    return false;
  }
  int i = startpos - 1;

  //2. deal with equal-x hs's
  region.Get( i, hs );
  while( i > 0 &&
         hs.GetDomPoint().GetX() == p.GetX() ){
    if( hs.Contains(p,geoid) ){
      return false;
    }
    if( hs.IsLeftDomPoint() && hs.RayAbove( p, y0 ) ){
      if( faceISN.find( hs.attr.faceno ) != faceISN.end() ){
        faceISN[ hs.attr.faceno ]++;
      } else {
        faceISN[ hs.attr.faceno ] = 1;
      }
    }
    region.Get( --i, hs );
  }

  // at this point, i is pointing to the last hs whose dp.x != p.x

  //3. get the coverage value
  coverno = hs.attr.coverageno;

  //4. search the region value for coverageno steps
  int touchedNo = 0;
  while( i >= 0 && touchedNo < coverno ){
    this->Get(i, hs);
    if( hs.Contains(p) ){
      return false;
    }
    if( hs.IsLeftDomPoint() &&
        hs.GetLeftPoint().GetX() <= p.GetX() &&
        p.GetX() <= hs.GetRightPoint().GetX() ){
      touchedNo++;
    }
    if( hs.IsLeftDomPoint() && hs.RayAbove( p, y0 ) ){
      if( faceISN.find( hs.attr.faceno ) != faceISN.end() ){
        faceISN[ hs.attr.faceno ]++;
      } else {
        faceISN[ hs.attr.faceno ] = 1;
      }
    }
    i--;  //the iterator
  }

  for( std::map<int, int>::iterator iter = faceISN.begin();
       iter != faceISN.end();
       iter++ ){
    if( iter->second % 2 != 0 ){
      return true;
    }
  }
  return false;
}


/*
Checks whether this regions intersects inHs.

*/

template<template<typename T>class Array>
bool RegionT<Array>::Intersects( const HalfSegment& inHs,
                         const Geoid* geoid/*=0*/) const
{
  assert( IsDefined() );
  assert( !geoid || geoid->IsDefined() );
  if( !IsEmpty() && bbox.Intersects( inHs.BoundingBox() ) )
  {
    if( Contains( inHs.GetLeftPoint(), geoid ) ||
        Contains( inHs.GetRightPoint(), geoid ) ){
      return true;
    }
    HalfSegment hs;
    for( int i = 0; i < Size(); i++ ){
      Get( i, hs );
      if( hs.Intersects( inHs, geoid ) ){
        return true;
      }
    }
  }
  return false;
}


/*
CHecks whether this region contains hs.

*/

template<template<typename T>class Array>
bool RegionT<Array>::Contains( const HalfSegment& hs, 
                               const Geoid* geoid/*=0*/ ) const
{
  assert( IsDefined() );
  assert( !geoid || geoid->IsDefined() );
  if(geoid){
    std::cerr << __PRETTY_FUNCTION__ << ": Spherical geometry not implemented!."
         << std::endl;
    assert(false); // TODO: implement sperical geometry case
  }
  if( IsEmpty() ){
    return false;
  }
  if( !hs.GetLeftPoint().Inside(bbox,geoid) ||
      !hs.GetRightPoint().Inside(bbox,geoid) ){
    return false;
  }
  if( !Contains(hs.GetLeftPoint(),geoid) ||
      !Contains(hs.GetRightPoint(),geoid) ){
    return false;
  }
  HalfSegment auxhs;
  bool checkMidPoint = false;
  std::vector<Point> intersection_points;
  //now we know that both endpoints of hs is inside region
  for( int i = 0; i < Size(); i++ ){
    Get(i, auxhs);
    if( auxhs.IsLeftDomPoint() ){
      if( hs.Crosses(auxhs,geoid) ){
        return false;
      } else if( hs.Inside(auxhs,geoid) ){//hs is part of the border
        return true;
      } else if( hs.Intersects(auxhs,geoid)){
              if(auxhs.Contains(hs.GetLeftPoint(),geoid) ||
                  auxhs.Contains(hs.GetRightPoint(),geoid) ||
                  hs.Contains(auxhs.GetLeftPoint(),geoid) ||
                  hs.Contains(auxhs.GetRightPoint(),geoid)){
                    checkMidPoint = true;
                   //the intersection point that is not the endpoint
                    Point temp_p;

                  if(hs.Intersection(auxhs,temp_p, geoid))
                    intersection_points.push_back(temp_p);
                  HalfSegment temp_hs;
                  if(hs.Intersection(auxhs, temp_hs, geoid)){
                      intersection_points.push_back(temp_hs.GetLeftPoint());
                      intersection_points.push_back(temp_hs.GetRightPoint());
                  }
              }
      }
    }
  }

  if( checkMidPoint ){
    Point midp( true,
                ( hs.GetLeftPoint().GetX() + hs.GetRightPoint().GetX() ) / 2,
                ( hs.GetLeftPoint().GetY() + hs.GetRightPoint().GetY() ) / 2 );
    if( !Contains( midp ) ){ return false; }
  }
  for(unsigned int i = 0;i < intersection_points.size();i++){
      //cout<<intersection_points.size()<<endl;
      Point p = intersection_points[i];
      double x1 = (hs.GetLeftPoint().GetX() + p.GetX())/2;
      double y1 = (hs.GetLeftPoint().GetY() + p.GetY())/2;
      double x2 = (hs.GetRightPoint().GetX() + p.GetX())/2;
      double y2 = (hs.GetRightPoint().GetY() + p.GetY())/2;
      Point midp1(true, x1, y1);
      Point midp2(true, x2, y2);
      if(!Contains(midp1,geoid)){ return false; }
      if(!Contains(midp2,geoid)){ return false; }
  }
  return true;
}


/*
Checks wther hs is completely in the interior of this region.

*/
template<template<typename T>class Array>
bool RegionT<Array>::InnerContains( const HalfSegment& hs,
                            const Geoid* geoid/*=0*/ ) const
{
  assert( IsDefined() );
  assert( !geoid || geoid->IsDefined() );
  if( IsEmpty() ){
    return false;
  }
  if( !hs.GetLeftPoint().Inside(bbox,geoid) ||
      !hs.GetRightPoint().Inside(bbox,geoid) ){
    return false;
  }
  if( !InnerContains(hs.GetLeftPoint(),geoid) ||
      !InnerContains(hs.GetRightPoint(),geoid) ){
    return false;
  }
  HalfSegment auxhs;

  //now we know that both endpoints of hs are completely inside the region
  for( int i = 0; i < Size(); i++ ){
    Get(i, auxhs);
    if( auxhs.IsLeftDomPoint() ){
      if( hs.Intersects( auxhs, geoid ) ){
        return false;
      }
    }
  }
  return true;
}

template<template<typename T>class Array>
bool RegionT<Array>::HoleEdgeContain( const HalfSegment& hs,
                              const Geoid* geoid/*=0*/ ) const
{
  assert( IsDefined() );
  assert( !geoid || geoid->IsDefined() );
  if( !IsEmpty() ) {
    return false;
  }
  HalfSegment auxhs;
  for( int i = 0; i < Size(); i++ ){
    Get( i, auxhs );
    if( auxhs.IsLeftDomPoint() &&
        auxhs.attr.cycleno > 0 &&
        hs.Inside( auxhs, geoid ) ){
      return true;
    }
  }
  return false;
}

/*
4.4.7 Operation ~intersects~

*/
template<template<typename T>class Array>
template<template<typename T2> class ArrayT2> 
bool RegionT<Array>::Intersects( const RegionT<ArrayT2> &r, 
                                 const Geoid* geoid/*=0*/ ) const
{
  assert( IsDefined() );
  assert( r.IsDefined() );
  assert( !geoid || geoid->IsDefined() );
  if( IsEmpty() || r.IsEmpty() || (geoid && !geoid->IsDefined()) ){
    return false;
  }
  if( !BoundingBox().Intersects( r.BoundingBox(), geoid ) ){
    return false;
  }
  assert( IsOrdered() );
  assert( r.IsOrdered() );
  if( Inside( r, geoid ) || r.Inside( *this, geoid ) ){
    return true;
  }
  HalfSegment hs1, hs2;
  for( int i = 0; i < Size(); i++ ){
    Get( i, hs1 );
    if( hs1.IsLeftDomPoint() ){
      for( int j = 0; j < r.Size(); j++ ){
        r.Get( j, hs2 );
        if( hs2.IsLeftDomPoint() &&
            hs1.Intersects( hs2, geoid ) ){
          return true;
        }
      }
    }
  }
  return false;
}


/*
Intersection with different data types

*/

template<template<typename T>class Array>
template<template<typename T2>class Array2>
void RegionT<Array>::Intersection(const Point& p, PointsT<Array2>& result,
                          const Geoid* geoid/*=0*/) const{
  result.Clear();
  if(!IsDefined() || !p.IsDefined() || (geoid && !geoid->IsDefined()) ){
    result.SetDefined(false);
    return;
  }
  result.SetDefined(true);
  if(this->Contains(p,geoid)){
    result+= p;
  }
}

template<template<typename T>class Array>
template<template<typename T2>class Array2,
         template<typename T3>class Array3>
void RegionT<Array>::Intersection(const PointsT<Array2>& ps, 
                          PointsT<Array3>& result,
                          const Geoid* geoid/*=0*/) const{
  result.Clear();
  if(!IsDefined() || !ps.IsDefined() || (geoid && !geoid->IsDefined()) ){
    result.SetDefined(false);
    return;
  }
  Point p;
  result.StartBulkLoad();
  for(int i=0;i<ps.Size();i++){
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
void RegionT<Array>::Intersection(const LineT<Array2>& l, LineT<Array3>& result,
                          const Geoid* geoid/*=0*/) const{

  try{
     SetOp(l,*this,result,avlseg::intersection_op, geoid, true);
  } catch (...){
    // compute intersection using robust implementation
    std::cerr << "Problem during plane sweep detected, "
         << "switch to slow implementation" << std::endl;
    robust::intersection(*this,l,result);
  }
}


template<template<typename T>class Array>
template<template<typename T2>class Array2,
         template<typename T3>class Array3>
void RegionT<Array>::Intersection(const RegionT<Array2>& r, 
                                  RegionT<Array3>& result,
                                  const Geoid* geoid/*=0*/) const{
  SetOp(*this,r,result,avlseg::intersection_op, geoid);
}

template<template<typename T>class Array>
template<template<typename T2>class Array2,
         template<typename T3>class Array3>
void RegionT<Array>::Intersection(const SimpleLineT<Array2>& l, 
                                  SimpleLineT<Array3>& result,
                                  const Geoid* geoid/*=0*/) const{
  SetOp(l,*this,result,avlseg::intersection_op, geoid);
}

template<template<typename T>class Array>
template<template<typename T2>class Array2>
void RegionT<Array>::Union(const Point& p, RegionT<Array2>& result,
                   const Geoid* geoid/*=0*/) const{
  if(!IsDefined() || !p.IsDefined() || (geoid && !geoid->IsDefined()) ){
    result.Clear();
    result.SetDefined(false);
    return;
  }
  result.SetDefined(true);
  result.CopyFrom(this);
}


/*
Union with different spatial types.

*/

template<template<typename T>class Array>
template<template<typename T2>class Array2,
         template<typename T3>class Array3>
void RegionT<Array>::Union(const PointsT<Array2>& ps, RegionT<Array3>& result,
                   const Geoid* geoid/*=0*/) const{
  if(!IsDefined() || !ps.IsDefined() || (geoid && !geoid->IsDefined()) ){
    result.Clear();
    result.SetDefined(false);
    return;
  }
  result.SetDefined(true);
  result.CopyFrom(this);
}

template<template<typename T>class Array>
template<template<typename T2>class Array2,
         template<typename T3>class Array3>
void RegionT<Array>::Union(const LineT<Array2>& line, RegionT<Array3>& result,
                   const Geoid* geoid/*=0*/) const{
  if(!IsDefined() || !line.IsDefined() || (geoid && !geoid->IsDefined()) ){
    result.Clear();
    result.SetDefined(false);
    return;
  }
  result.SetDefined(true);
  result.CopyFrom(this);
}

template<template<typename T>class Array>
template<template<typename T2> class ArrayT2,
         template<typename T3> class ArrayT3>
void RegionT<Array>::Union(const RegionT<ArrayT2>& region, 
                           RegionT<ArrayT3>& result,
                           const Geoid* geoid/*=0*/) const{
   SetOp(*this,region,result,avlseg::union_op, geoid);
}

template<template<typename T>class Array>
template<template<typename T2>class Array2,
         template<typename T3>class Array3>
void RegionT<Array>::Union(const SimpleLineT<Array2>& line, 
                           RegionT<Array3>& result,
                           const Geoid* geoid/*=0*/) const{
  if(!IsDefined() || !line.IsDefined() || (geoid && !geoid->IsDefined()) ){
    result.Clear();
    result.SetDefined(false);
    return;
  }
  result.SetDefined(true);
  result.CopyFrom(this);
}

template<template<typename T>class Array>
template<template<typename T2>class Array2>
void RegionT<Array>::Minus(const Point& p, RegionT<Array2>& result,
                   const Geoid* geoid/*=0*/) const{
  if(!IsDefined() || !p.IsDefined() || (geoid && !geoid->IsDefined()) ){
    result.Clear();
    result.SetDefined(false);
    return;
  }
  result.SetDefined(true);
  result.CopyFrom(this);
}


/*
Implementation of Minus.

*/

template<template<typename T>class Array>
template<template<typename T2>class Array2,
         template<typename T3>class Array3>
void RegionT<Array>::Minus(const PointsT<Array2>& ps, RegionT<Array3>& result,
                   const Geoid* geoid/*=0*/) const{
  if(!IsDefined() || !ps.IsDefined() || (geoid && !geoid->IsDefined()) ){
    result.Clear();
    result.SetDefined(false);
    return;
  }
  result.SetDefined(true);
  result.CopyFrom(this);
}

template<template<typename T>class Array>
template<template<typename T2>class Array2,
         template<typename T3>class Array3>
void RegionT<Array>::Minus(const LineT<Array2>& line, RegionT<Array3>& result,
                   const Geoid* geoid/*=0*/) const{
  if(!IsDefined() || !line.IsDefined() || (geoid && !geoid->IsDefined()) ){
    result.Clear();
    result.SetDefined(false);
    return;
  }
  result.SetDefined(true);
  result.CopyFrom(this);
}

template<template<typename T>class Array>
template<template<typename T2>class Array2,
         template<typename T3>class Array3>
void RegionT<Array>::Minus(const RegionT<Array2>& region, 
                           RegionT<Array3>& result,
                           const Geoid* geoid/*=0*/) const{
   SetOp(*this,region,result,avlseg::difference_op, geoid);
}

template<template<typename T>class Array>
template<template<typename T2>class Array2,
         template<typename T3>class Array3>
void RegionT<Array>::Minus(const SimpleLineT<Array2> & line,
                           RegionT<Array3>& result,
                           const Geoid* geoid/*=0*/) const{
  if(!IsDefined() || !line.IsDefined() || (geoid && !geoid->IsDefined()) ){
    result.Clear();
    result.SetDefined(false);
    return;
  }
  result.SetDefined(true);
  result.CopyFrom(this);
}

/*
Inside check.

*/

template<template<typename T>class Array>
template<template<typename T2> class Array2>
bool RegionT<Array>::Inside( const RegionT<Array2>& r, 
                             const Geoid* geoid/*=0*/ ) const
{

  assert( IsDefined() );
  assert( r.IsDefined() );
  assert( !geoid || geoid->IsDefined() );

  if(!IsDefined() || !r.IsDefined() || (geoid && !geoid->IsDefined()) ){
    return false;
  }
  if( IsEmpty() ){
    return true;
  }
  if(r.IsEmpty()){
    return false;
  }
  assert( IsOrdered() );
  assert( r.IsOrdered() );
  if( !r.BoundingBox().Contains( bbox, geoid ) ){
    return false;
  }
  HalfSegment hs1, hs2;
  for( int i = 0; i < Size(); i++ ){
    Get( i, hs1 );
    if( hs1.IsLeftDomPoint() ){
      if( !r.Contains( hs1,geoid ) ){
        return false;
      }
    }
  }
  bool existhole = false,
       allholeedgeinside = true;
  for( int j = 0; j < r.Size(); j++ ){
    r.Get( j, hs2 );
    if( hs2.IsLeftDomPoint() &&
        hs2.attr.cycleno > 0 ){
    //&& (hs2 is not masked by another face of region2)
      if( !HoleEdgeContain( hs2,geoid ) ){
        existhole=true;
        if( !Contains( hs2,geoid ) ){
          allholeedgeinside=false;
        }
      }
    }
  }
  if( existhole && allholeedgeinside ){
    return false;
  }
  return true;
}

/*
Check for adjacency with another region.

*/

template<template<typename T>class Array>
template<template<typename T2>class Array2>
bool RegionT<Array>::Adjacent( const RegionT<Array2>& r,
                       const Geoid* geoid/*=0*/ ) const
{
  assert( IsDefined() );
  assert( r.IsDefined() );
  assert( !geoid || geoid->IsDefined() );
  if( IsEmpty() || r.IsEmpty() || (geoid && !geoid->IsDefined()) ){
    return false;
  }
  if( !BoundingBox().Intersects( r.BoundingBox(),geoid ) ){
    return false;
  }
  assert( IsOrdered() );
  assert( r.IsOrdered() );
  if( Inside( r,geoid ) || r.Inside( *this,geoid ) )
    return false;

  HalfSegment hs1, hs2;
  bool found = false;
  for( int i = 0; i < Size(); i++ ){
    Get( i, hs1 );
    if( hs1.IsLeftDomPoint() ){
      for( int j = 0; j < r.Size(); j++ ){
        r.Get( j, hs2 );
        if( hs2.IsLeftDomPoint() && hs1.Intersects( hs2,geoid ) ){
          if( hs1.Crosses( hs2, geoid ) ){
            return false;
          }
          found = true;
        }
      }
    }
  }
  return found;
}

/*
Check for overlap.

*/

template<template<typename T>class Array>
template<template<typename T2>class Array2>
bool RegionT<Array>::Overlaps( const RegionT<Array2>& r, 
                               const Geoid* geoid/*=0*/ ) const
{
  assert( IsDefined() );
  assert( r.IsDefined() );
  assert( !geoid || geoid->IsDefined() );
  if( IsEmpty() || r.IsEmpty() || (geoid && !geoid->IsDefined()) ){
    return false;
  }
  if( !BoundingBox().Intersects( r.BoundingBox(),geoid ) ){
    return false;
  }
  assert( IsOrdered() );
  assert( r.IsOrdered() );
  if( Inside( r ) || r.Inside( *this ) )
    return true;

  HalfSegment hs1, hs2;
  for( int i = 0; i < Size(); i++ ){
    Get( i, hs1 );
    if( hs1.IsLeftDomPoint() ){
      for( int j = 0; j < r.Size(); j++ ){
        r.Get( j, hs2 );
        if( hs2.IsLeftDomPoint() ){
          if( hs2.Crosses( hs1,geoid ) ){
            return true;
          }
        }
      }
    }
  }
  return false;
}

/*
Checks whether p is on border of this region.

*/

template<template<typename T>class Array>
bool RegionT<Array>::OnBorder( const Point& p, const Geoid* geoid/*=0*/ ) const
{
  assert( IsDefined() );
  assert( p.IsDefined() );
  assert( !geoid || geoid->IsDefined() );
  if(IsEmpty() || !p.IsDefined() || (geoid && !geoid->IsDefined()) ){
    return false;
  }
  int pos;
  if( Find( p, pos ) ){ // the point is found on one half segment
    return true;
  }
  if( pos == 0 ||
      pos == Size() ){ // the point is smaller or bigger than all segments
    return false;
  }
  HalfSegment hs;
  pos--;
  while( pos >= 0 ){
    Get( pos--, hs );
    if( hs.IsLeftDomPoint() && hs.Contains( p,geoid ) ){
      return true;
    }
  }
  return false;
}

template<template<typename T>class Array>
bool RegionT<Array>::InInterior( const Point& p, 
                                 const Geoid* geoid/*=0*/ ) const
{
  return InnerContains( p, geoid );
}


/*
Minimum distance to p.

*/
template<template<typename T>class Array>
double RegionT<Array>::Distance( const Point& p, 
                                 const Geoid* geoid /*=0*/ ) const
{
  assert( !IsEmpty() ); // subsumes defined
  assert( p.IsDefined() );
  assert( !geoid || geoid->IsDefined() );

  if(geoid){
    cout << __PRETTY_FUNCTION__ << ": Spherical geometry not implemented."
         <<endl;
    assert(false); // TODO: Implement spherical geometry case.
  }

  assert( IsOrdered() );
  if( Contains( p ) )
    return 0.0;

  HalfSegment hs;
  double result = std::numeric_limits<double>::max();

  for( int i = 0; i < Size(); i++ )
  {
    Get( i, hs );

    if( hs.IsLeftDomPoint() )
      result = MIN( result, hs.Distance( p, geoid ) );
  }
  return result;
}

/*
Minimum distance to r.

*/

template<template<typename T>class Array>
double RegionT<Array>::Distance( const Rectangle<2>& r,
                         const Geoid* geoid /*=0*/ ) const
{
  assert( !IsEmpty() ); // subsumes defined
  assert( r.IsDefined() );
  assert( !geoid || geoid->IsDefined() );
    if(geoid){
    cout << __PRETTY_FUNCTION__ << ": Spherical geometry not implemented."
         <<endl;
    assert(false); // TODO: Implement spherical geometry case.
  }

  assert( IsOrdered() );
  Point p1(true,r.MinD(0),r.MinD(1));
  Point p2(true,r.MaxD(0),r.MinD(1));
  Point p3(true,r.MaxD(0),r.MaxD(1));
  Point p4(true,r.MinD(0),r.MaxD(1));

  if(Contains(p1,geoid)) return 0.0;
  if(Contains(p2,geoid)) return 0.0;
  if(Contains(p3,geoid)) return 0.0;
  if(Contains(p4,geoid)) return 0.0;

  HalfSegment hs;
  double mindist = std::numeric_limits<double>::max();
  for(int i=0;i<region.Size(); i++){
     Get(i,hs);
     if(hs.IsLeftDomPoint()){
       double d = hs.Distance(r,geoid);
       if(d<mindist){
          mindist = d;
          if(AlmostEqual(mindist,0)){
             return 0.0;
          }
       }
     }
  }
  return mindist;
}


/*
Check for intersection. 

*/

template<template<typename T>class Array>
bool RegionT<Array>::Intersects( const Rectangle<2>& r,
                         const Geoid* geoid /*=0*/ ) const{

   if(!IsDefined() || !r.IsDefined()){
     return false;
   }
   if(IsEmpty() || r.IsEmpty()){
     return false;
   }
   // either, the rectangle is contained in the region
   // or one of the halfsegments of the region
   // intersects the rectangle
   
   Point p1(true,r.MinD(0),r.MinD(1));
   Point p2(true,r.MaxD(0),r.MinD(1));
   Point p3(true,r.MaxD(0),r.MaxD(1));
   Point p4(true,r.MinD(0),r.MaxD(1));

   if(Contains(p1,geoid)) return true;
   if(Contains(p2,geoid)) return true;
   if(Contains(p3,geoid)) return true;
   if(Contains(p4,geoid)) return true;

   HalfSegment hs; 
   for(int i=0;i<region.Size(); i++){
     Get(i,hs);
     if(hs.IsLeftDomPoint()){
       if(hs.Intersects(r,geoid)){
          return true;
       }
     }
   }
   return false;
}


/*
Size of the covered area.

*/

template<template<typename T>class Array>
double RegionT<Array>::Area(const Geoid* geoid/*=0*/) const
{
  assert( IsDefined() );
  assert( !geoid || geoid->IsDefined() );
  if(geoid){
    std::cerr << __PRETTY_FUNCTION__ << ": Spherical geometry not implemented."
         << std::endl; //TODO: Implement spherical geometry case.
    assert(false);
  }
  int n = Size();
  double area = 0.0,
         x0 = 0.0, y0 = 0.0,
         x1 = 0.0, y1 = 0.0;

  // get minimum with respect to Y-dimension
  double minY = MIN(BoundingBox().MinD(1), +0.0);

  HalfSegment hs;
  for(int i=0; i<n; i++){
    Get( i, hs );
    if( hs.IsLeftDomPoint() ){ // use only one halfsegment
      x0 = hs.GetLeftPoint().GetX();
      x1 = hs.GetRightPoint().GetX();
      // y0, y1 must be >= 0, so we correct them
      y0 = hs.GetLeftPoint().GetY() - minY;
      y1 = hs.GetRightPoint().GetY() - minY;
      double a = (x1-x0) * ((y1+y0) * 0.5);
      if ( hs.attr.insideAbove ){
        a = -a;
      }
      area += a;
    }
  }

  return area;
}

/*
Distance to other spatial types.


*/

template<template<typename T>class Array>
template<template<typename T2>class Array2>
double RegionT<Array>::Distance( const PointsT<Array2>& ps, 
                                 const Geoid* geoid /*=0*/ ) const
{
  assert( !IsEmpty() ); // subsumes IsDefined()
  assert( !ps.IsEmpty() ); // subsumes IsDefined()
  assert( IsOrdered() );
  assert( ps.IsOrdered() );
  assert( !geoid || geoid->IsDefined() );

  if(geoid){
    cout << __PRETTY_FUNCTION__ << ": Spherical geometry not implemented."
         <<endl;
    assert(false); // TODO: Implement spherical geometry case.
  }

  double result = std::numeric_limits<double>::max();
  Point p;

  for( int i = 0; i < ps.Size(); i++ ){
    ps.Get( i, p );
    if( Contains( p ) ){
      return 0.0;
    }
    HalfSegment hs;
    for( int j = 0; j < Size(); j++ ){
      Get( j, hs );
      if( hs.IsLeftDomPoint() )
        result = MIN( result, hs.Distance( p, geoid ) );
    }
  }
  return result;
}

template<template<typename T>class Array>
template<template<typename T2>class Array2>
double RegionT<Array>::Distance( const RegionT<Array2> &r, 
                                 const Geoid* geoid /*=0*/ ) const
{
  assert( !IsEmpty() ); // subsumes IsDefined()
  assert( !r.IsEmpty() ); // subsumes IsDefined()
  assert( IsOrdered() );
  assert( r.IsOrdered() );
  assert( !geoid || geoid->IsDefined() );

  if(geoid){
    cout << __PRETTY_FUNCTION__ << ": Spherical geometry not implemented."
         <<endl;
    assert(false); // TODO: Implement spherical geometry case.
  }

  if( Inside( r, geoid ) || r.Inside( *this, geoid ) ){
    return 0.0;
  }
  double result = std::numeric_limits<double>::max();
  HalfSegment hs1, hs2;
  for( int i = 0; i < Size(); i++ ){
    Get( i, hs1 );
    if( hs1.IsLeftDomPoint() ){
      for( int j = 0; j < r.Size(); j++ ){
        r.Get( j, hs2 );
        if( hs2.IsLeftDomPoint() ){
          if( hs1.Intersects( hs2, geoid ) ){
            return 0.0;
          }
          result = MIN( result, hs1.Distance( hs2, geoid ) );
        }
      }
    }
  }
  return result;
}



/*
Divides this region into their faces.

*/

template<template<typename T>class Array>
template<template<typename T2>class Array2>
void RegionT<Array>::Components( std::vector<RegionT<Array2>*>& components )
{
  components.clear();
  if( IsEmpty() ) { // subsumes IsDefined()
    return;
  }
  for(int i=0;i<noComponents;i++){
   components.push_back(new RegionT<Array2>(1));
   components[i]->StartBulkLoad();
  }
  HalfSegment hs;
  for(int i=0;i<Size();i++){
    Get(i,hs);
    int face = hs.attr.faceno;
    hs.attr.faceno = 0;
    (*components[face]) += hs;
  }
  for(int i=0;i<noComponents;i++){
    components[i]->EndBulkLoad(false,false,false,false);
  }
}

/*
Returns the holes of this region.

*/
template<template<typename T>class Array>
template<template<typename T2>class Array2>
void RegionT<Array>::getHoles(RegionT<Array2>& result) const{
   if(!IsDefined()){
      result.SetDefined(false);
   }
   result.Clear();
   result.SetDefined(true);
   result.StartBulkLoad();
   int edgeno = 0;
   for(int i=0; i< Size(); i++){
       HalfSegment hs;
       Get(i,hs);
       if(hs.IsLeftDomPoint()){
         if(hs.attr.cycleno!=0){ // not the outer cycle
            hs.attr.edgeno = edgeno;
            hs.attr.insideAbove = ! hs.attr.insideAbove;
            result += hs;
            hs.SetLeftDomPoint(!hs.IsLeftDomPoint());
            result += hs;
            edgeno++;
         }
      }
   }
   result.EndBulkLoad(true,true,true,true);
}


/*
Affine transformations.

*/

template<template<typename T>class Array>
template<template<typename T2>class Array2>
void RegionT<Array>::Translate( const Coord& x, const Coord& y, 
                                RegionT<Array2>& result ) const
{
  result.Clear();
  if( !IsDefined() ) {
    result.SetDefined( false );
    return;
  }
  result.SetDefined( true );
  assert( IsOrdered() );
  HalfSegment hs;
  result.StartBulkLoad();
  for( int i = 0; i < Size(); i++ )
  {
    Get( i, hs );
    hs.Translate( x, y );
    result += hs;
  }
  result.SetNoComponents( NoComponents() );
  result.EndBulkLoad( false, false, false, false );
}

template<template<typename T>class Array>
template<template<typename T2>class Array2>
void RegionT<Array>::Rotate( const Coord& x, const Coord& y,
                   const double alpha,
                   RegionT<Array2>& result ) const
{
  result.Clear();
  if( !IsDefined() ) {
    result.SetDefined( false );
    return;
  }
  result.Resize(Size());
  result.SetDefined( true );

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
  Point p1o;
  Point p2o;

  for( int i = 0; i < Size(); i++ )
  {
    Get( i, hso );
    p1o = hso.GetLeftPoint();
    p2o = hso.GetRightPoint();
    p1.Set( m00*p1o.GetX()
            + m01*p1o.GetY() + m02,
            m10*p1o.GetX()
           + m11*p1o.GetY() + m12);
    p2.Set( m00*p2o.GetX()
             + m01*p2o.GetY() + m02,
             m10*p2o.GetX()
             + m11*p2o.GetY() + m12);

    HalfSegment hsr(hso); // ensure to copy attr;
    hsr.Set(hso.IsLeftDomPoint(),p1,p2);
    bool above = hso.attr.insideAbove;

    if(p1>p2)  {
       above = !above;
    }

    hsr.attr.insideAbove = above;
    result += hsr;
  }
  result.EndBulkLoad(true,true,true,false); // reordering may be required

}


template<template<typename T>class Array>
template<template<typename T2>class Array2>
void RegionT<Array>::Scale( const Coord& sx, const Coord& sy,
                   RegionT<Array2>& result ) const
{
  result.Clear();
  if( !IsDefined() ) {
    result.SetDefined( false );
    return;
  }
  result.Resize(Size());
  result.SetDefined( true );
  result.StartBulkLoad();
  HalfSegment hso;
  for( int i = 0; i < Size(); i++ ) {
    Get( i, hso );
    if(!hso.Scale(sx,sy)){
       result.Clear();
       result.SetDefined(false);
       return;
    }
    result += hso;
  }
  result.EndBulkLoad(); // reordering may be required
}


/*
Computres touchpooints with l.

*/

template<template<typename T>class Array>
template<template<typename T2>class Array2,
         template<typename Ti3>class Array3>
void RegionT<Array>::TouchPoints( const LineT<Array2>& l, 
                                  PointsT<Array3>& result,
                                  const Geoid* geoid/*=0*/ ) const
{
  result.Clear();
  if( !IsDefined() || !l.IsDefined() || (geoid && !geoid->IsDefined()) ) {
    result.SetDefined( false );
    return;
  }
  result.SetDefined( true );

  if( IsEmpty() || l.IsEmpty() ) {
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
          if( hs1.Intersection( hs2, p, geoid ) ) {
            result += p;
          }
        }
      }
    }
  }
  result.EndBulkLoad();
}

template<template<typename T>class Array>
template<template<typename T2>class Array2,
         template<typename Ti3>class Array3>
void RegionT<Array>::TouchPoints( 
                          const RegionT<Array2>& r, 
                          PointsT<Array3>& result,
                          const Geoid* geoid/*=0*/ ) const
{

  result.Clear();
  if( !IsDefined() || !r.IsDefined() || (geoid && !geoid->IsDefined()) ) {
    result.SetDefined( false );
    return;
  }
  result.SetDefined( true );
  if( IsEmpty() || r.IsEmpty() ){
    return;
  }
  assert( IsOrdered() && r.IsOrdered() );
  HalfSegment hs1, hs2;
  Point p(false);
  result.StartBulkLoad();
  for( int i = 0; i < Size(); i++ ){
    Get( i, hs1 );
    if( hs1.IsLeftDomPoint() ){
      for( int j = 0; j < r.Size(); j++ ){
        r.Get( j, hs2 );
        if( hs2.IsLeftDomPoint() ){
          if( hs1.Intersection( hs2, p, geoid ) ){
            result += p;
          }
        }
      }
    }
  }
  result.EndBulkLoad( true);
}

/*
Computes the common part of the boundary.

*/

template<template<typename T>class Array>
template<template<typename T2>class Array2,
         template<typename Ti3>class Array3>
void RegionT<Array>::CommonBorder( 
                           const RegionT<Array2>& r, 
                           LineT<Array3>& result,
                           const Geoid* geoid/*=0*/ ) const
{
  result.Clear();
  if( !IsDefined() || !r.IsDefined() || (geoid && !geoid->IsDefined()) ) {
    result.SetDefined( false );
    return;
  }
  result.SetDefined( true );
  if( IsEmpty() || r.IsEmpty() ){
    return;
  }
  assert( IsOrdered() && r.IsOrdered() );
  HalfSegment hs1, hs2;
  HalfSegment reshs;
  int edgeno = 0;
  Point p;
  result.StartBulkLoad();
  for( int i = 0; i < Size(); i++ ){
    Get( i, hs1 );
    if( hs1.IsLeftDomPoint() ){
      for( int j = 0; j < r.Size(); j++ ){
        r.Get( j, hs2 );
        if( hs2.IsLeftDomPoint() ){
          if( hs1.Intersection( hs2, reshs, geoid ) ){
            reshs.attr.edgeno = edgeno++;
            result += reshs;
            reshs.SetLeftDomPoint( !reshs.IsLeftDomPoint() );
            result += reshs;
          }
        }
      }
    }
  }
  result.EndBulkLoad();
}

/*
returns the number of components.

*/

template<template<typename T>class Array>
int RegionT<Array>::NoComponents() const
{
  assert( IsDefined() );
  return noComponents;
}

/*
Returns all points of the containes halfsegments.

*/
template<template<typename T>class Array>
template<template<typename T2>class Array2>
void RegionT<Array>::Vertices( PointsT<Array2>* result, 
                              const Geoid* geoid/*=0*/ ) const
{
  result->Clear();
  if(!IsDefined() || (geoid && !geoid->IsDefined()) ){
    result->SetDefined(false);
    return;
  }
  result->SetDefined(true);
  if( IsEmpty() ){
    return;
  }
  assert( IsOrdered() );
  HalfSegment hs;
  int size = Size();
  for( int i = 0; i < size; i++ ){
    Get( i, hs );
    Point p = hs.GetDomPoint();
    *result += p;
  }
  result->EndBulkLoad( false, true );
}

/*
Computes the boundary of this region.

*/
template<template<typename T>class Array>
template<template<typename T2>class Array2>
void RegionT<Array>::Boundary( LineT<Array2>* result, 
                               const Geoid* geoid/*=0*/ ) const
{
  result->Clear();
  if(!IsDefined() || (geoid && !geoid->IsDefined()) ){
      result->SetDefined(false);
      return;
  }
  assert( IsOrdered() );
  result->SetDefined(true);
  if( IsEmpty() ){
    return;
  }
  HalfSegment hs;
  result->StartBulkLoad();
  int size = Size();
  for( int i = 0; i < size; i++ ){
    Get( i, hs );
    if(hs.IsLeftDomPoint()){
       hs.attr.edgeno = i;
       *result += hs;
       hs.SetLeftDomPoint(false);
       *result += hs;
    }
  }
  result->EndBulkLoad();
}


/*
Assignment operator.

*/
template<template<typename T>class Array>
template<template<typename T2>class Array2>
RegionT<Array>& RegionT<Array>::operator=( const RegionT<Array2>& r )
{
  assert( r.IsOrdered() );
  convertDbArrays(r.region, region); 
  bbox = r.bbox;
  noComponents = r.noComponents;
  del.isDefined = r.del.isDefined;
  return *this;
}

template<template<typename T>class Array>
RegionT<Array>& RegionT<Array>::operator=( const RegionT& r )
{
  assert( r.IsOrdered() );
  region.copyFrom(r.region);
  bbox = r.bbox;
  noComponents = r.noComponents;
  del.isDefined = r.del.isDefined;
  return *this;
}



/*
auxiliary functions for parallel scan through 2 regions.

*/
template<template<typename T2>class Array2,
         template<typename T3>class Array3>
void SelectFirst_rr( const RegionT<Array2>& R1, const RegionT<Array3>& R2,
                     object& obj, status& stat )
{
  R1.SelectFirst();
  R2.SelectFirst();

  HalfSegment hs1, hs2;
  bool gotHs1 = R1.GetHs( hs1 ),
       gotHs2 = R2.GetHs( hs2 );

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
      obj=both;
  }
}

template<template<typename T2>class Array2,
         template<typename T3>class Array3>
void SelectNext_rr( const RegionT<Array2>& R1, const RegionT<Array3>& R2,
                    object& obj, status& stat )
{
  // 1. get the current elements
  HalfSegment hs1, hs2;
  bool gotHs1 = R1.GetHs( hs1 ),
       gotHs2 = R2.GetHs( hs2 );

  //2. move the pointers
  if( !gotHs1 && !gotHs2 )
  {
    //do nothing
  }
  else if( !gotHs1 )
  {
    R2.SelectNext();
    gotHs2 = R2.GetHs( hs2 );
  }
  else if( !gotHs2 )
  {
    R1.SelectNext();
    gotHs1 = R1.GetHs( hs1 );
  }
  else //both currently defined
  {
    if( hs1 < hs2 ) //then hs1 is the last output
    {
      R1.SelectNext();
      gotHs1 = R1.GetHs( hs1 );
    }
    else if( hs1 > hs2 )
    {
      R2.SelectNext();
      gotHs2 = R2.GetHs( hs2 );
    }
    else
    {
      R1.SelectNext();
      gotHs1 = R1.GetHs( hs1 );
      R2.SelectNext();
      gotHs2 = R2.GetHs( hs2 );
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
Check for equality.

*/
template<template<typename T>class Array>
bool RegionT<Array>::operator==( const RegionT<Array>& r ) const
{
  if(!IsDefined() && !r.IsDefined()){
    return true;
  }

  if(!IsDefined() || !r.IsDefined()){
    return false;
  }

  if( Size() != r.Size() )
    return false;

  if( IsEmpty() && r.IsEmpty() )
    return true;

  if( bbox != r.bbox )
    return false;

  assert( ordered && r.ordered );
  object obj;
  status stat;
  SelectFirst_rr( *this, r, obj, stat );

  while( obj == both || obj == none )
  {
    if( stat == endboth )
      return true;

    SelectNext_rr( *this, r, obj, stat );
  }
  return false;
}

template<template<typename T>class Array>
bool RegionT<Array>::operator!=( const RegionT<Array> &cr) const
{
  return !(*this==cr);
}


/*
Adding and subtracting halfsegments.

*/
template<template<typename T>class Array>
RegionT<Array>& RegionT<Array>::operator+=( const HalfSegment& hs )
{
  assert(IsDefined());

  if( IsEmpty() )
    bbox = hs.BoundingBox();
  else
    bbox = bbox.Union( hs.BoundingBox() );

  if( !IsOrdered() )
  {
    region.Append(hs);
  }
  else
  {
    int pos;
    if( !Find( hs, pos ) )
    {
      HalfSegment auxhs;
      for( int i = region.Size() - 1; i >= pos; i++ )
      {
        region.Get( i, auxhs );
        region.Put( i+1, auxhs );
      }
      region.Put( pos, hs );
    }
  }
  return *this;
}

template<template<typename T>class Array>
RegionT<Array>& RegionT<Array>::operator-=( const HalfSegment& hs )
{
  assert(IsDefined());
  assert( IsOrdered() );

  int pos;
  if( Find( hs, pos ) )
  {
    HalfSegment auxhs;
    for( int i = pos; i < Size(); i++ )
    {
      region.Get( i+1, auxhs );
      region.Put( i, auxhs );
    }
  }

  // Naive way to redo the bounding box.
  if( IsEmpty() )
    bbox.SetDefined( false );
  int i = 0;
  HalfSegment auxhs;
  region.Get( i++, auxhs );
  bbox = auxhs.BoundingBox();
  for( ; i < Size(); i++ )
  {
    region.Get( i, auxhs );
    bbox = bbox.Union( auxhs.BoundingBox() );
  }

  return *this;
}


/*
Searches the index of a speified halfsegmnst.

*/
template<template<typename T>class Array>
bool RegionT<Array>::Find( const HalfSegment& hs, int& pos ) const
{
  assert( IsOrdered() );
  assert(IsDefined());
  return region.Find( &hs, HalfSegmentCompare, pos );
}

template<template<typename T>class Array>
bool RegionT<Array>::Find( const Point& p, int& pos ) const
{
  assert( IsOrdered() );
  assert( IsDefined());
  return region.Find( &p, PointHalfSegmentCompare, pos );
}

template<template<typename T>class Array>
void RegionT<Array>::Sort()
{
  if(!IsDefined()){
    return;
  }
  assert( !IsOrdered() );

  region.Sort( HalfSegmentCompare );

  ordered = true;
}


int HalfSegmentLogicCompare(const void *a, const void *b);

template<template<typename T>class Array>
void RegionT<Array>::LogicSort()
{

  if(!IsDefined()){
   return;
  }
  region.Sort( HalfSegmentLogicCompare );

  ordered = true;
}

template<template<typename T>class Array>
std::ostream& operator<<( std::ostream& os, const RegionT<Array>& cr )
{
  os << "<"<<endl;
  if( !cr.IsDefined() ) {
    os << " undefined ";
  } else {
    HalfSegment hs;
    for( int i = 0; i < cr.Size(); i++ )
    {
      cr.Get( i, hs );
//      os << " " << hs << endl;
      Point lp = hs.GetLeftPoint();
      Point rp = hs.GetRightPoint();
      printf("(%.10f, %.10f) (%.10f, %.10f)\n",
            lp.GetX(),lp.GetY(),rp.GetX(),rp.GetY());
    }
  }
  os << ">";
  return os;
}



/*
Sets the partner numbers. Is is assumed, that corresponding edges have
the same edgeno,

*/
template<template<typename T>class Array>
void RegionT<Array>::SetPartnerNo()
{
  if( !IsDefined() )
    return;
  int size = Size();
  int* tmp = new int[size/2];
  memset(tmp,0,size*sizeof(int) / 2);
  HalfSegment hs;
  for( int i = 0; i < size; i++ )
  {
    Get( i, hs );
    if( hs.IsLeftDomPoint() )
    {
      tmp[hs.attr.edgeno] = i;
    }
    else
    {
      int p = tmp[hs.attr.edgeno];
      HalfSegment hs1( hs );
      hs1.attr.partnerno = p;
      Put( i, hs1 );
      Get( p, hs );
      hs1 = hs;
      hs1.attr.partnerno = i;
      Put( p, hs1 );
    }
  }
  delete[] tmp;
}

bool GetAcceptedPoint( std::vector <EdgePoint>pointsOnEdge,
                       int &i, const int end,
                       EdgePoint &ep );


template<template<typename T>class Array>
template<template<typename T2>class Array2>
void RegionT<Array>::CreateNewSegments(std::vector<EdgePoint>pointsOnEdge,
                                       RegionT<Array2> &cr,
                                       const Point &bPoint,const Point &ePoint,
                                       WindowEdge edge,int &partnerno,
                                       bool inside, const Geoid* geoid/*=0*/)
//The inside attribute indicates if the points on edge will originate
//segments that are inside the window (its values is true), or outside
//the window (its value is false)
{
  int begin, end, i;
  HalfSegment *hs;
  AttrType attr(0);
  EdgePoint dp,dpAux;

  if (pointsOnEdge.size()==0) return;
/*
  for (int j=0;j<pointsOnEdge.size();j++)
    cout<<endl<<j<<": "<<pointsOnEdge[j];

*/
  sort(pointsOnEdge.begin(),pointsOnEdge.end());

/*
  for (int j=0;j<pointsOnEdge.size();j++)
    cout<<endl<<j<<": "<<pointsOnEdge[j];

*/
  begin = 0;
  end = pointsOnEdge.size()-1;

  dp = pointsOnEdge[begin];
  if ( dp.direction)//dp points to left or down
  {

    if (!dp.rejected)
    {
      //If dp is a rejected point then it must not be considered
      //as point to be connected to the window edge
      hs = new HalfSegment(true, bPoint, dp);

      attr.partnerno = partnerno;
      partnerno++;
      if ( (edge == WTOP) || (edge == WLEFT) )
        attr.insideAbove = !inside;
        //If inside == true, then insideAbove attribute of the top and left
        //half segments must be set to false, otherwise its value must be true.
        //In other words, the insideAbove atribute value is the opposite of the
        //parameter inside's value.
      else
        if ( (edge == WRIGHT) || (edge == WBOTTOM))
          attr.insideAbove = inside;
        //If inside == true, then insideAbove attribute of the right and bottom
        //half segments must be set to true, otherwise its value must be false.
        //In other words, the insideAbove atribute value is the same of the
        //parameter inside's value.
      hs->SetAttr(attr);
      cr+=(*hs);
      hs->SetLeftDomPoint( !hs->IsLeftDomPoint() );
      cr+=(*hs);
      delete hs;
    }
    begin++;
    //The variable ~begin~ must be incremented until exists
    // points with the same coordinates
    //and directions as dp
    while (begin<=end)
    {
      dpAux = pointsOnEdge[begin];
      if (!( (dpAux.GetX() == dp.GetX()) && (dpAux.GetY() == dp.GetY())
            && (dpAux.direction==dp.direction) ) )
        break;
      begin++;
    }
  }


  dp = pointsOnEdge[end];
  if ( !dp.direction) //dp points to right or up
  {
    bool rejectEndPoint=dp.rejected;
    end--;

    while ( end >= begin )
    {
      dpAux = pointsOnEdge[end];
      if ( !( (dpAux.GetX() == dp.GetX() ) && ( dpAux.GetY() == dp.GetY() ) &&
              (dpAux.direction==dp.direction) ) )
         break;

      //when a rejected point is found the rejectEndPoint
      //does not change anymore.

      end--;
    }

    if (!rejectEndPoint)
    {
      hs = new HalfSegment(true, dp, ePoint);
      attr.partnerno = partnerno;
      if ( (edge == WTOP) || (edge == WLEFT) )
        attr.insideAbove = !inside;
      else
        if ( (edge == WRIGHT) || (edge == WBOTTOM))
          attr.insideAbove = inside;
      partnerno++;
      hs->SetAttr(attr);
      cr+=(*hs);
      hs->SetLeftDomPoint( !hs->IsLeftDomPoint() );
      cr+=(*hs);

      delete hs;
    }
  }

  i = begin;
  while (i < end)
  {
    EdgePoint ep1,ep2;
    if ( GetAcceptedPoint(pointsOnEdge,i,end,ep1) )
    {
      i++;
      if (GetAcceptedPoint(pointsOnEdge,i,end, ep2) )
        i++;
      else
        break;
    }
    else
      break;
    if ( ! ( (ep1.GetX() == ep2.GetX()) && (ep1.GetY() == ep2.GetY()) ) )
    {  //discard degenerated edges
      hs = new HalfSegment(true, ep1, ep2);
      attr.partnerno = partnerno;
      partnerno++;
      if ( (edge == WTOP) || (edge == WLEFT) )
        attr.insideAbove = !inside;
      else
        if ( (edge == WRIGHT) || (edge == WBOTTOM))
          attr.insideAbove = inside;
      hs->SetAttr(attr);
      cr+=(*hs);
      hs->SetLeftDomPoint( !hs->IsLeftDomPoint() );
      cr+=(*hs);
      delete hs;
    }
  }
}

template<template<typename T>class Array>
template<template<typename T2>class Array2>
void RegionT<Array>::CreateNewSegmentsWindowVertices(const Rectangle<2> &window,
                                std::vector<EdgePoint> pointsOnEdge[4],
                                RegionT<Array2> &cr,
                                int &partnerno,bool inside,
                                const Geoid* geoid/*=0*/) const
//The inside attribute indicates if the points on edge will originate
//segments that are inside the window (its values is true), or outside
//the window (its value is false)
{
  Point tlPoint(true,window.MinD(0),window.MaxD(1)),
        trPoint(true,window.MaxD(0),window.MaxD(1)),
        blPoint(true,window.MinD(0),window.MinD(1)),
        brPoint(true,window.MaxD(0),window.MinD(1));
  bool tl=false, tr=false, bl=false, br=false;

  AttrType attr(0);
  if ( ( (pointsOnEdge[WTOP].size()==0) ||
         (pointsOnEdge[WLEFT].size()==0) )
     && ( this->Contains(tlPoint,geoid) ) ){
      tl = true;
  }
  if ( ( (pointsOnEdge[WTOP].size()==0) ||
         (pointsOnEdge[WRIGHT].size()==0)  )
       && ( this->Contains(trPoint,geoid) ) ){
      tr = true;
  }
  if ( ( (pointsOnEdge[WBOTTOM].size()==0) ||
         (pointsOnEdge[WLEFT].size()==0)  )
       && ( this->Contains(blPoint,geoid) ) ){
      bl = true;
  }
  if ( ( (pointsOnEdge[WBOTTOM].size()==0) ||
         (pointsOnEdge[WRIGHT].size()==0)  )
         && ( this->Contains(brPoint,geoid) ) ){
      br = true;
  }
  //Create top edge
  if (tl && tr && (pointsOnEdge[WTOP].size()==0)){
    HalfSegment *hs;
    hs = new HalfSegment(true, tlPoint, trPoint);
    //If inside == true, then insideAbove attribute of the top and left
    //half segments must be set to false, otherwise its value must be true.
    //In other words, the insideAbove atribute value is the opposite of the
    //inside function's parameter value.
    attr.insideAbove = !inside;
    attr.partnerno = partnerno;
    partnerno++;

    hs->SetAttr(attr);
    cr+=(*hs);
    hs->SetLeftDomPoint( !hs->IsLeftDomPoint() );
    cr+=(*hs);
    delete hs;
  }
  //Create left edge
  if (tl && bl && (pointsOnEdge[WLEFT].size()==0)){
    HalfSegment *hs;
    hs = new HalfSegment(true, tlPoint, blPoint);
    //If inside == true, then insideAbove attribute of the top and left
    //half segments must be set to false, otherwise its value must be true.
    //In other words, the insideAbove atribute value is the opposite of the
    //parameter inside's value.
    attr.insideAbove = !inside;
    attr.partnerno = partnerno;
    partnerno++;
    hs->SetAttr(attr);
    cr+=(*hs);
    hs->SetLeftDomPoint( !hs->IsLeftDomPoint() );
    cr+=(*hs);
    delete hs;
  }
  //Create right edge
  if (tr && br && (pointsOnEdge[WRIGHT].size()==0)){
    HalfSegment *hs;
    hs = new HalfSegment(true, trPoint, brPoint);
    //If inside == true, then insideAbove attribute of the right and bottom
    //half segments must be set to true, otherwise its value must be false.
    //In other words, the insideAbove atribute value is the same of the
    //parameter inside's value.
    attr.insideAbove = inside;
    attr.partnerno = partnerno;
    partnerno++;
    hs->SetAttr(attr);
    cr+=(*hs);
    hs->SetLeftDomPoint( !hs->IsLeftDomPoint() );
    cr+=(*hs);
    delete hs;
  }
  //Create bottom edge
  if (bl && br && (pointsOnEdge[WBOTTOM].size()==0)){
    HalfSegment *hs;
    hs = new HalfSegment(true, blPoint, brPoint);
    //If inside == true, then insideAbove attribute of the right and bottom
    //half segments must be set to true, otherwise its value must be false.
    //In other words, the insideAbove atribute value is the same of the
    //parameter inside's value.
    attr.insideAbove = inside;
    attr.partnerno = partnerno;
    partnerno++;
    hs->SetAttr(attr);
    cr+=(*hs);
    hs->SetLeftDomPoint( !hs->IsLeftDomPoint() );
    cr+=(*hs);
    delete hs;
  }
}

template<template<typename T>class Array>
bool RegionT<Array>::ClippedHSOnEdge(const Rectangle<2> &window,
                        const HalfSegment &hs,
                        bool clippingIn,std::vector<EdgePoint> pointsOnEdge[4],
                        const Geoid* geoid/*=0*/)
{
//This function returns true if the segment lies on one of the window's edge.
// The clipped half segments that lie on the edges must be rejected according to
// the kind of clipping (returning the portion of the region that is inside the
// region or the portion that is outside).

  EdgePoint ep1,ep2;
  AttrType attr=hs.GetAttr();
  bool reject = false,
       result = false;
  //Returns true if the clipped hs was treated as a segment on edge
  if ( hs.GetLeftPoint().GetY() == hs.GetRightPoint().GetY() )
  { //horizontal edge
    if (( hs.GetLeftPoint().GetY() == window.MaxD(1) ) )
    { //top edge
  // If the half segment lies on the upper edge and
  // the insideAbove attribute's value
  // is true then the region's area is outside the window,
  // and the half segment mustn't
  // be included in the clipped region (Reject).
  // However, its end points maybe will have to be
  // connected to the vertices of the window.
  // It happens only when the vertice of the
  // window is inside the region and the end point
  // is the first point on the window's
  // edge (for the upper-left vertice) or the last
  // point on the window's vertice (for
  // the upper right edge).
      if ( clippingIn && attr.insideAbove )
        reject = true;
      else
        if ( !clippingIn && !attr.insideAbove )
          reject = true;
      ep1.Set(hs.GetLeftPoint(),false,reject); //--> right
      ep2.Set(hs.GetRightPoint(),true,reject);  //<-- left
      pointsOnEdge[WTOP].push_back(ep1);
      pointsOnEdge[WTOP].push_back(ep2);
      result = true;
    }
    else //bottom edge
      if (( hs.GetLeftPoint().GetY() == window.MinD(1) ) )
      {
        if ( clippingIn && !attr.insideAbove )
           reject = true;
        else
          if ( !clippingIn && attr.insideAbove )
            reject = true;
        ep1.Set(hs.GetLeftPoint(),false,reject); //--> right
        ep2.Set(hs.GetRightPoint(),true,reject);  //<-- left
        pointsOnEdge[WBOTTOM].push_back(ep1);
        pointsOnEdge[WBOTTOM].push_back(ep2);
        result = true;
      }
  }
  else //Vertical edges
    if ( hs.GetLeftPoint().GetX() == hs.GetRightPoint().GetX() )
    {
      if ( hs.GetLeftPoint().GetX() == window.MinD(0) ) //Left edge
      {
        if ( clippingIn && attr.insideAbove )
          reject = true;
        else
          if (!clippingIn && !attr.insideAbove )
            reject = true;
        ep1.Set(hs.GetLeftPoint(),false,reject); //^ up
        ep2.Set(hs.GetRightPoint(),true,reject);  //v dowb
        pointsOnEdge[WLEFT].push_back(ep1);
        pointsOnEdge[WLEFT].push_back(ep2);
        result = true;
      }
      else
        if ( hs.GetLeftPoint().GetX() == window.MaxD(0) ) //Right edge
        {
          if ( clippingIn && !attr.insideAbove )
            reject = true;
          else
            if ( !clippingIn && attr.insideAbove )
              reject = true;
          ep1.Set(hs.GetLeftPoint(),false,reject); //^ up
          ep2.Set(hs.GetRightPoint(),true,reject);  //v dowb
          pointsOnEdge[WRIGHT].push_back(ep1);
          pointsOnEdge[WRIGHT].push_back(ep2);
          result = true;
        }
    }
  return result;
}

template<template<typename T>class Array>
bool RegionT<Array>::GetCycleDirection( const Point &pA,
                                const Point &pP,
                                const Point &pB )
{
  double m_p_a,m_p_b;
  if (pA.GetX() == pP.GetX()){//A --> P is a vertical segment
    if (pA.GetY() > pP.GetY() ) {//A --> P directed downwards (case 1)
      return false; //Counterclockwise
    } else {//upwards (case 2)
      return true; // Clockwise
    }
  }
  if (pB.GetX() == pP.GetX()) {//P --> B is a vertical segment
    if ( pP.GetY() > pB.GetY()){ //downwords (case 3)
      return false; //Conterclockwise
    } else {//upwards
      return true; //Clockwise
    }
  }

  //compute the slopes of P-->A and P-->B
  m_p_a = ( pA.GetY() - pP.GetY() ) / ( pA.GetX() - pP.GetX() );
  m_p_b = ( pB.GetY() - pP.GetY() ) / ( pB.GetX() - pP.GetX() );
  if (m_p_a > m_p_b) //case 5
    return false;//counterclockwise
  else  //case 6
    return true; //clockwise
}

template<template<typename T>class Array>
bool RegionT<Array>::GetCycleDirection() const
{
/*
Preconditions:
* The region must represent just one cycle!!!!
* It is need that the edgeno stores the order that the half segments were
 typed, and the half segments must be sorted in the half segment order. In
 other words if hs1.attr.edgeno is less than hs2.attr.edgeno then hs1 was
 typed first than hs2.

This function has the purpose of choosing the A, P, and B points in order
to call the function that really computes the cycle direction.
As the point P is leftmost point then it is the left point of hs1 or
 the left point of hs2 because in the half segment order these two points
are equal.  Now the problem is to decide which of the right points are A
and B. At the first sight we could say that the point A is the right point
of the half segment with lowest partner number. However it is not true ever
because the APB connected points may be go over the
bound of the pointlist. This will be the case if the cycle is in the form
P,B,..,A and B,...,A,P. Nevertheless the segments are ordered in the half
segment order, and when the last half segment is been considered for choosing
 the APB connected points, the point A will be always the right point of the
last segment.

*/
  Point pA(true,0,0), pP(true,0,0), pB(true,0,0);
  HalfSegment hs1, hs2;
  this->Get(0,hs1);
  this->Get(1,hs2);

  assert( hs1.GetLeftPoint()==hs2.GetLeftPoint() );
  pP = hs1.GetLeftPoint();
  // If we have the last half segment connected to the first half
  // segment, the difference //between their partner numbers is
  // more than one.
  if (abs(hs1.attr.edgeno - hs2.attr.edgeno)>1)
  {
    if (hs1.attr.edgeno > hs2.attr.edgeno)
    {
      pA = hs1.GetRightPoint();
      pB = hs2.GetRightPoint();
    }
    else
    {
      pA = hs2.GetRightPoint();
      pB = hs1.GetRightPoint();
    }
  }
  else
    if (hs1.attr.edgeno < hs2.attr.edgeno)
    {
      pA = hs1.GetRightPoint();
      pB = hs2.GetRightPoint();
    }
    else
    {
      pA = hs2.GetRightPoint();
      pB = hs1.GetRightPoint();
    }
  return GetCycleDirection(pA,pP,pB);
}

   //cycleDirection: true (cycle is clockwise) / false
  // (cycle is counterclockwise)
  //It is need that the attribute insideAbove of the half segments represents
  //the order that  their points were typed: true (left point, right point) /
  //false (right point, left point).

void AddPointToEdgeArray( const Point &p,
                          const HalfSegment &hs,
                          const Rectangle<2> &window,
                          std::vector<EdgePoint> pointsOnEdge[4] );



template<template<typename T>class Array>
template<template<typename T2>class Array2>
void RegionT<Array>::GetClippedHSIn(const Rectangle<2> &window,
                            RegionT<Array2> &clippedRegion,
                            std::vector<EdgePoint> pointsOnEdge[4],
                            int &partnerno,
                            const Geoid* geoid/*=0*/) const
{
  HalfSegment hs;
  HalfSegment hsInside;
  bool inside, isIntersectionPoint;

  SelectFirst();
  for(int i=0; i < Size(); i++){
    GetHs( hs );
    if (hs.IsLeftDomPoint()){
      Point intersectionPoint;
      hs.WindowClippingIn(window, hsInside, inside,
                           isIntersectionPoint,intersectionPoint,geoid);
      if (isIntersectionPoint){
         AddPointToEdgeArray(intersectionPoint,hs,window, pointsOnEdge);
      } else if ( inside ) {
          bool hsOnEdge = ClippedHSOnEdge(window,hsInside,true,
                                          pointsOnEdge,geoid);
          if (!hsOnEdge){
            //Add the clipped segment to the new region if it was not rejected
            hsInside.attr.partnerno=partnerno;
            partnerno++;
            hsInside.SetAttr(hsInside.attr);
            clippedRegion += hsInside;
            hsInside.SetLeftDomPoint( !hsInside.IsLeftDomPoint() );
            clippedRegion += hsInside;

            //Add the points to the array of the points that lie on
            //some of the window's edges
            const Point& lp = hsInside.GetLeftPoint(),
                         rp = hsInside.GetRightPoint();

            //If the point lies on one edge it must be added to the
            //corresponding vector.
            AddPointToEdgeArray(lp,hs,window, pointsOnEdge);
            AddPointToEdgeArray(rp, hs,window, pointsOnEdge);
          }
        }
    }
    SelectNext();
  }
}

template<template<typename T>class Array>
void RegionT<Array>::AddClippedHS( const Point &pl,
                           const Point &pr,
                           AttrType &attr,
                           int &partnerno )
{
  HalfSegment hs(true,pl,pr);
  attr.partnerno = partnerno;
  partnerno++;
  hs.SetAttr(attr);
  (*this)+=hs;
  hs.SetLeftDomPoint( !hs.IsLeftDomPoint() );
  (*this)+=hs;
}

template<template<typename T>class Array>
template<template<typename T2>class Array2>
void RegionT<Array>::GetClippedHSOut(const Rectangle<2> &window,
                             RegionT<Array2> &clippedRegion,
                             std::vector<EdgePoint> pointsOnEdge[4],
                             int &partnerno,
                             const Geoid* geoid/*=0*/) const
{
  for (int i=0; i < Size();i++){
    HalfSegment hs;
    HalfSegment hsInside;
    bool inside=false,isIntersectionPoint=false;
    Get(i,hs);
    if (hs.IsLeftDomPoint()){
      Point intersectionPoint;
      hs.WindowClippingIn(window,hsInside, inside,
                           isIntersectionPoint,intersectionPoint,geoid);
      if (inside){
        bool hsOnEdge=false;
        if (isIntersectionPoint){
          HalfSegment aux( hs );
          if (hs.GetLeftPoint()!=intersectionPoint)
            clippedRegion.AddClippedHS(aux.GetLeftPoint(),
                                       intersectionPoint,
                                       aux.attr,partnerno) ;
          if (hs.GetRightPoint()!=intersectionPoint)
            clippedRegion.AddClippedHS(intersectionPoint,
                                       aux.GetRightPoint(),
                                       aux.attr,partnerno);
          AddPointToEdgeArray(intersectionPoint,aux,
                              window, pointsOnEdge);
        } else {
          hsOnEdge = ClippedHSOnEdge(window, hsInside,
                                     false, pointsOnEdge, geoid);
          if (!hsOnEdge){
            HalfSegment aux( hs );
            if (hs.GetLeftPoint()!=hsInside.GetLeftPoint()){
             //Add the part of the half segment composed by the left
             //point of hs and the left point of hsInside.
              clippedRegion.AddClippedHS(aux.GetLeftPoint(),
                                         hsInside.GetLeftPoint(),
                                         aux.attr,partnerno);
            }
            AddPointToEdgeArray(hsInside.GetLeftPoint(),aux,
                                window, pointsOnEdge);
            if (hs.GetRightPoint()!=hsInside.GetRightPoint()){
             //Add the part of the half segment composed by the right
             //point of hs and the right point of hsInside.
              clippedRegion.AddClippedHS(hsInside.GetRightPoint(),
                                         aux.GetRightPoint(),
                                         aux.attr,partnerno);
            }
            AddPointToEdgeArray(hsInside.GetRightPoint(),aux,
                                window, pointsOnEdge);
          }
        }
      }
      else
      {
        HalfSegment aux( hs );
        clippedRegion.AddClippedHS(aux.GetLeftPoint(),
                                   aux.GetRightPoint(),
                                   aux.attr,partnerno);
      }
    }
    SelectNext();
  }
}

template<template<typename T>class Array>
template<template<typename T2>class Array2>
void RegionT<Array>::GetClippedHS(const Rectangle<2> &window,
                          RegionT<Array2> &clippedRegion,
                          bool inside,
                          const Geoid* geoid/*=0*/) const
{
  std::vector<EdgePoint> pointsOnEdge[4];
    //upper edge, right edge, bottom, left
  int partnerno=0;

  clippedRegion.StartBulkLoad();

  if (inside){
    GetClippedHSIn(window,clippedRegion,pointsOnEdge,partnerno);
  } else {
    GetClippedHSOut(window,clippedRegion,pointsOnEdge,partnerno);
  }

  Point bPoint,ePoint;
  bPoint.Set(window.MinD(0),window.MaxD(1)); //left-top
  ePoint.Set(window.MaxD(0),window.MaxD(1)); //right-top
  CreateNewSegments(pointsOnEdge[WTOP],clippedRegion,bPoint,ePoint,
                    WTOP,partnerno,inside);
  bPoint.Set(window.MinD(0),window.MinD(1)); //left-bottom
  ePoint.Set(window.MaxD(0),window.MinD(1)); //right-bottom
  CreateNewSegments(pointsOnEdge[WBOTTOM],clippedRegion,bPoint,ePoint,
                    WBOTTOM,partnerno,inside);
  bPoint.Set(window.MinD(0),window.MinD(1)); //left-bottom
  ePoint.Set(window.MinD(0),window.MaxD(1)); //left-top
  CreateNewSegments(pointsOnEdge[WLEFT],clippedRegion,bPoint,ePoint,
                    WLEFT,partnerno,inside);
  bPoint.Set(window.MaxD(0),window.MinD(1)); //right-bottom
  ePoint.Set(window.MaxD(0),window.MaxD(1)); //right-top
  CreateNewSegments(pointsOnEdge[WRIGHT],clippedRegion,bPoint,ePoint,
                     WRIGHT,partnerno,inside);
  CreateNewSegmentsWindowVertices(window, pointsOnEdge,clippedRegion,
                                  partnerno,inside);
  clippedRegion.EndBulkLoad();
}

template<template<typename T>class Array>
bool RegionT<Array>::IsCriticalPoint( const Point &adjacentPoint,
                              const int hsPosition ) const
{
  int adjPosition = hsPosition,
      adjacencyNo = 0,
      step = 1;
  do
  {
    HalfSegment adjCHS;
    adjPosition+=step;
    if ( adjPosition<0 || adjPosition>=this->Size())
      break;
    Get(adjPosition,adjCHS);
    if (!adjCHS.IsLeftDomPoint())
      continue;
    AttrType attr = adjCHS.GetAttr();
    //When looking for critical points, the partner of
    //the adjacent half segment found
    //cannot be consired.
    if (attr.partnerno == hsPosition)
      continue;
    if ( ( adjacentPoint==adjCHS.GetLeftPoint() ) ||
         ( adjacentPoint==adjCHS.GetRightPoint() ) )
      adjacencyNo++;
    else
    {
      if (step==-1)
        return false;
      step=-1;
      adjPosition=hsPosition;
    }
  }
  while (adjacencyNo<2);

  return (adjacencyNo>1);
}

template<template<typename T>class Array>
bool RegionT<Array>::GetAdjacentHS( const HalfSegment &hs,
                            const int hsPosition,
                            int &position,
                            const int partnerno,
                            const int partnernoP,
                            HalfSegment& adjacentCHS,
                            const Point &adjacentPoint,
                            Point &newAdjacentPoint,
                            bool *cycle,
                            int step) const
{
  bool adjacencyFound=false;
  do
  {
    position+=step;
    if ( position<0 || position>=this->Size()){
      // no halfsegments found in this direction
      break;
    }

    Get(position,adjacentCHS);
    if (partnernoP == position){
      // back direction of the original
      continue;
    }

    if ( adjacentPoint==adjacentCHS.GetLeftPoint() ){
        if (!cycle[position]){
          newAdjacentPoint = adjacentCHS.GetRightPoint();
          adjacencyFound = true;
        }
    }
    else if  ( adjacentPoint==adjacentCHS.GetRightPoint() ){
            if (!cycle[position]){
              newAdjacentPoint = adjacentCHS.GetLeftPoint();
              adjacencyFound = true;
            }
    }
    else
        break;
  }
  while (!adjacencyFound);

//  cout<<"adjacencyFound "<<adjacencyFound<<endl;

  return adjacencyFound;
}
/*
The parameter ~hasCriticalPoint~ indicates that the cycle that
is been computed has a critical point.

*/

template<template<typename T>class Array>
void RegionT<Array>::ComputeCycle( HalfSegment &hs,
                           int faceno,
                           int cycleno,
                           int &edgeno,
                           bool *cycle )
{

  Point nextPoint = hs.GetLeftPoint(),
        lastPoint = hs.GetRightPoint(),
        previousPoint, currentCriticalPoint(false);
  AttrType attr, attrP;
  HalfSegment hsP;
  std::vector<SCycle> sCycleVector;
  SCycle *s=NULL;

  do
  {
     if (s==NULL)
     {
       //Update attributes
       attr = hs.GetAttr();

       Get(attr.partnerno,hsP);
       attrP = hsP.GetAttr();

       attr.faceno=faceno;
       attr.cycleno=cycleno;
       attr.edgeno=edgeno;

       UpdateAttr(attrP.partnerno,attr);

       attrP.faceno=faceno;
       attrP.cycleno=cycleno;
       attrP.edgeno=edgeno;
       UpdateAttr(attr.partnerno,attrP);

       edgeno++;

       cycle[attr.partnerno]=true;
       cycle[attrP.partnerno]=true;

       if (this->IsCriticalPoint(nextPoint,attrP.partnerno)) {
         currentCriticalPoint = nextPoint;
       }

       s = new SCycle(hs,attr.partnerno,hsP,attrP.partnerno,
                      currentCriticalPoint,nextPoint);
     }
     HalfSegment adjacentCHS;
     Point adjacentPoint;
     bool adjacentPointFound=false;
     previousPoint = nextPoint;
     if (s->goToCHS1Right)
     {
       s->goToCHS1Right=GetAdjacentHS(s->hs1,
                                      s->hs2Partnerno,
                                      s->hs1PosRight,
                                      s->hs1Partnerno,
                                      s->hs2Partnerno,adjacentCHS,
                                      previousPoint,
                                      nextPoint,
                                      cycle,
                                      1);
       adjacentPointFound=s->goToCHS1Right;
     //  cout<<"flag 1 "<<adjacentPointFound<<" p1 "<<previousPoint<<endl;
     }
     if ( !adjacentPointFound && s->goToCHS1Left )
     {
       s->goToCHS1Left=GetAdjacentHS(s->hs1,
                                     s->hs2Partnerno,
                                     s->hs1PosLeft,
                                     s->hs1Partnerno,
                                     s->hs2Partnerno,
                                     adjacentCHS,
                                     previousPoint,
                                     nextPoint,
                                     cycle,
                                     -1);
       adjacentPointFound=s->goToCHS1Left;
       //cout<<"flag 2 "<<adjacentPointFound<<" p2 "<<previousPoint<<endl;
     }
     if (!adjacentPointFound && s->goToCHS2Right)
     {
       s->goToCHS2Right=GetAdjacentHS(s->hs2,
                                      s->hs1Partnerno,
                                      s->hs2PosRight,
                                      s->hs2Partnerno,
                                      s->hs1Partnerno,
                                      adjacentCHS,
                                      previousPoint,
                                      nextPoint,
                                      cycle,
                                      1);
       adjacentPointFound=s->goToCHS2Right;
       //cout<<"flag 3 "<<adjacentPointFound<<" p3 "<<previousPoint<<endl;
     }
     if (!adjacentPointFound && s->goToCHS2Left)
     {
       s->goToCHS2Left=GetAdjacentHS(s->hs2,
                                     s->hs1Partnerno,
                                     s->hs2PosLeft,
                                     s->hs2Partnerno,
                                     s->hs1Partnerno,
                                     adjacentCHS,
                                     previousPoint,
                                     nextPoint,
                                     cycle,
                                     -1);
       adjacentPointFound = s->goToCHS2Left;

      // cout<<"flag 4 "<<adjacentPointFound<<" p4 "<<previousPoint<<endl;

     }
    
     if(!adjacentPointFound){
         std::cerr<<"previousPoint "<<previousPoint<<std::endl;
         std::cerr << "Problem in rebuilding cycle in a region " << std::endl;
         std::cerr << "no adjacent point found" << std::endl;
         std::cerr << "Halfsegments : ---------------     " << std::endl;
         HalfSegment hs;
         for(int i=0;i<Size();i++){
            Get(i,hs);
            std::cerr << i << " : " << (hs) << std::endl;
         }
         throw(0);
     }
     sCycleVector.push_back(*s);

     if ((currentCriticalPoint.IsDefined()) && currentCriticalPoint==nextPoint)
     {
       //The critical point defines a cycle, so it is need to
       //remove the segments
       //from the vector, and set the segment as not visited in thei
       // cycle array.
       //FirsAux is the first half segment with the critical point equals to
       //criticalPoint.
       SCycle sAux,firstSCycle;

       do
       {
          sAux=sCycleVector.back();
          sCycleVector.pop_back();
          firstSCycle=sCycleVector.back();
          if (!firstSCycle.criticalPoint.IsDefined())
            break;
          if (firstSCycle.criticalPoint!=currentCriticalPoint)
            break;
          cycle[sAux.hs1Partnerno]=false;
          cycle[sAux.hs2Partnerno]=false;
          edgeno--;
       }while(sCycleVector.size()>1);
       delete s; //when s is deleted, the critical point is also deleted.
       s = 0;
       if (sCycleVector.size()==1)
       {
         sCycleVector.pop_back();
         if(s){
           delete s;
         }
         s = new SCycle(firstSCycle);
       }
       else{
         if(s){
           delete s;
         }
         s= new SCycle(sAux);
       }
       hs = s->hs1;
       currentCriticalPoint=s->criticalPoint;
       nextPoint=s->nextPoint;
       continue;
     }

     if ( nextPoint==lastPoint )
     {
       //Update attributes
       attr = adjacentCHS.GetAttr();

       Get(attr.partnerno,hsP);
       attrP = hsP.GetAttr();

       attr.faceno=faceno;
       attr.cycleno=cycleno;
       attr.edgeno=edgeno;

       UpdateAttr(attrP.partnerno,attr);

       attrP.faceno=faceno;
       attrP.cycleno=cycleno;
       attrP.edgeno=edgeno;
       UpdateAttr(attr.partnerno,attrP);

       edgeno++;

       cycle[attr.partnerno]=true;
       cycle[attrP.partnerno]=true;

       break;
     }
     hs = adjacentCHS;
     delete s;
     s=NULL;
  }
  while(1);
  if(s){
    delete s;
    s = 0;
  }

}

//This function returns the value of the attribute inside above of
//the first half segment under the half segment hsS.
template<template<typename T>class Array>
int RegionT<Array>::GetNewFaceNo(HalfSegment &hsS, bool *cycle)
{
  int coverno=0;
  int startpos=0;
  double y0;
  AttrType attr;
  std::vector<HalfSegment> v;

  //1. find the right place by binary search
  Find( hsS, startpos );

  int hsVisiteds=0;

  //2. deal with equal-x hs's
  //To verify if it is need to deal with this

  attr = hsS.GetAttr();
  coverno = attr.coverageno;

  //search the region value for coverageno steps
  int touchedNo=0;
  HalfSegment hs;
  const Point& p = hsS.GetLeftPoint();

  int i=startpos;
  while (( i>=0)&&(touchedNo<coverno))
  {
    this->Get(i, hs);
    hsVisiteds++;

    if ( (cycle[i]) && (hs.IsLeftDomPoint()) &&
         ( (hs.GetLeftPoint().GetX() <= p.GetX()) &&
         (p.GetX() <= hs.GetRightPoint().GetX()) ))
    {
      touchedNo++;
      if (!hs.RayAbove(p, y0))
        v.push_back(hs);
    }
    i--;  //the iterator
  }
  if (v.size()==0)
    return -1; //the new face number will be the last face number +1
  else
  {
    sort(v.begin(),v.end());
    //The first half segment is the next half segment above hsS
    HalfSegment hs = v[v.size()-1];
    attr = hs.GetAttr();
    if (attr.insideAbove)
      return attr.faceno; //the new cycle is a cycle of the face ~attr.faceno~
    else
      return -1; //new face
  }
}

template<template<typename T>class Array>
int RegionT<Array>::GetNewFaceNo(const HalfSegment& hsIn, 
                                 const int startpos) const {

    // Precondition:
    // hsIn is the smallest (in halfsegment-order) segment of a cycle.
    // startpos is the index of hsIn in the DBArray.

    if (hsIn.GetAttr().insideAbove) {

        // hsIn belongs to a new face:
        return -1;
    }

    // Now we know hsIn belongs to a new hole and we
    // have to encounter the enclosing face.
    // This is done by searching the next halfsegment maxHS 'under' hsIn.
    // Since we go downwards, the facenumber of maxHS must be already known
    // and is equal to the facenumber of hsIn.

    double y0;
    double maxY0;
    HalfSegment hs;
    HalfSegment maxHS;
    bool hasMax = false;
    const Point& p = hsIn.GetLeftPoint();
    const int coverno = hsIn.GetAttr().coverageno;
    int touchedNo = 0;
    int i = startpos - 1;
    bool first = true;

    while (i >=0 && touchedNo < coverno) {

        Get(i, hs);

        if (!hs.IsLeftDomPoint()) {

            i--;
            continue;
        }

        if (hs.GetLeftPoint().GetX() <= p.GetX() &&
            p.GetX() <= hs.GetRightPoint().GetX()) {

            touchedNo++;
        }

        if (!AlmostEqual(hs.GetRightPoint().GetX(), p.GetX()) &&
            hs.RayDown(p, y0)) {

            if (first ||
                y0 > maxY0 ||
                (AlmostEqual(y0, maxY0) && hs > maxHS)) {

                // To find the first halfsegment 'under' hsIn
                // we compare them as follows:
                // (1) y-value of the intersection point between a ray down
                //     from the left point of hsIn and hs.
                // (2) halfsegment order.

                maxY0 = y0;
                maxHS = hs;
                first = false;
                hasMax = true;
            }
        }

        i--;
    }

    if (!hasMax) {

        std::cerr << "Problem in rebuilding cycle in a region " << std::endl;
        std::cerr << "No outer cycle found" << std::endl;
        std::cerr << "hsIn: " << hsIn << std::endl;
        std::cerr << "Halfsegments : ---------------     " << std::endl;
        HalfSegment hs;

        for(int i=0;i<Size();i++) {

            Get(i,hs);
            std::cerr << i << " : " << (hs) << std::endl;
        }

        throw 0;
    }

    //the new cycle is a holecycle of the face ~maxHS.attr.faceno~
    return maxHS.GetAttr().faceno;
}

template<template<typename T>class Array>
void RegionT<Array>::ComputeRegion()
{
  if( !IsDefined() )
    return;
  //array that stores in position i the last cycle number of the face i
  std::vector<int> face;
  //array that stores in the position ~i~ if the half
  //segment hi had already the face
  //number, the cycle number and the edge number
  //attributes set properly, in other words,
  //it means that hi is already part of a cycle
  bool *cycle;
  int lastfaceno=0,
      faceno=0,
      cycleno = 0,
      edgeno = 0;
  bool isFirstCHS=true;
  HalfSegment hs;

  if (Size()==0)
    return;
   //Insert in the vector the first cycle of the first face
  face.push_back(0);
  cycle = new bool[Size()];
#ifdef SECONDO_MAC_OSX
  // something goes wrong at mac osx and the memset function
  int size = Size();
  for(int i=0;i<size;i++){
    cycle[i] = false;
  }
#else
  memset( cycle, 0, Size()*sizeof(bool) );
#endif
  for ( int i=0; i<Size(); i++)
  {
    Get(i,hs);
    HalfSegment aux(hs);
    if ( aux.IsLeftDomPoint() && !cycle[i])
    {
      if(!isFirstCHS)
      {
        int facenoAux = GetNewFaceNo(aux,i);
        if (facenoAux==-1)
        {/*The lhs half segment will start a new face*/
          lastfaceno++;
          faceno = lastfaceno;
           /*to store the first cycle number of the face lastFace*/
          face.push_back(0);
          cycleno = 0;
          edgeno = 0;
        }
        else
        { /*The half segment ~hs~ belongs to an existing face*/
          faceno = facenoAux;
          face[faceno]++;
          cycleno = face[faceno];
          edgeno = 0;
        }
      }
      else
      {
        isFirstCHS = false;
      }
      ComputeCycle(aux, faceno,cycleno, edgeno, cycle);
    }
  }
  delete [] cycle;
  noComponents = lastfaceno + 1;
}

template<template<typename T>class Array>
template<template<typename T2>class Array2>
void RegionT<Array>::WindowClippingIn(const Rectangle<2> &window,
                              RegionT<Array2> &clippedRegion,
                              const Geoid* geoid/*=0*/) const
{
  clippedRegion.Clear();
  if( !IsDefined() || !window.IsDefined() || (geoid && !geoid->IsDefined()) ) {
    clippedRegion.SetDefined( false );
    return;
  }
  clippedRegion.SetDefined( true );
  //cout<<endl<<"Original: "<<*this<<endl;
  if (!this->bbox.Intersects(window,geoid)){
    return;
  }
  //If the bounding box of the region is inside the window,
  //then the clippedRegion
  //is equal to the region been clipped.

  if (window.Contains(this->bbox,geoid)){
    clippedRegion = *this;
  } else {
    //cout<<endl<<this->bbox<<endl;
    this->GetClippedHS(window,clippedRegion,true, geoid);
    //cout<<endl<<"Clipped HS:"<<endl<<clippedRegion;
    clippedRegion.ComputeRegion();
  }
  //cout<<endl<<"Clipped;"<<clippedRegion;
}

template<template<typename T>class Array>
template<template<typename T2>class Array2>
void RegionT<Array>::WindowClippingOut(const Rectangle<2> &window,
                               RegionT<Array2> &clippedRegion,
                               const Geoid* geoid/*=0*/) const
{
  clippedRegion.Clear();
  if( !IsDefined() || !window.IsDefined() || (geoid && !geoid->IsDefined()) ) {
    clippedRegion.SetDefined( false );
    return;
  }
  clippedRegion.SetDefined( true );
  //If the bounding box of the region is inside the window,
  //then the clipped region is empty
  //cout<<"region: "<<*this<<endl;
  if (window.Contains(this->bbox,geoid)){
    return;
  }
  if (!window.Intersects(this->bbox,geoid)){
    clippedRegion = *this;
  } else {
    this->GetClippedHS(window,clippedRegion,false,geoid);
 //   cout<<endl<<"Clipped HS:"<<endl<<clippedRegion;
    clippedRegion.ComputeRegion();
  }
 // cout<<endl<<"clippedRegion: "<<clippedRegion;
}

template<template<typename T>class Array>
size_t   RegionT<Array>::HashValue() const
{
  //cout<<"cregion hashvalue1*******"<<endl;
  if(IsEmpty()) // subsumes !IsDefined()
    return 0;

  unsigned long h=0;
  HalfSegment hs;
  Coord x1, y1;
  Coord x2, y2;

  for( int i = 0; ((i < Size())&&(i<5)); i++ )
  {
    Get( i, hs );
    x1=hs.GetLeftPoint().GetX();
    y1=hs.GetLeftPoint().GetY();

    x2=hs.GetRightPoint().GetX();
    y2=hs.GetRightPoint().GetY();
    h=h+(unsigned long)((5*x1 + y1)+ (5*x2 + y2));
  }
  return size_t(h);
}

template<template<typename T>class Array>
void RegionT<Array>::Clear()
{
  region.clean();
  pos = -1;
  ordered = true;
  bbox.SetDefined(false);
}

template<template<typename T>class Array>
void RegionT<Array>::SetDefined(bool defined) {
    if(!defined){
        Clear();
    }
    Attribute::SetDefined( defined );
}
template<template<typename T>class Array>
void RegionT<Array>::SetEmpty()
{
  region.clean();
  pos = -1;
  ordered = true;
  bbox.SetDefined(false);
  SetDefined(true);
}


template<template<typename T>class Array>
void RegionT<Array>::CopyFrom( const Attribute* right )
{
  *this = *(const RegionT<Array> *)right;
}

template<template<typename T>class Array>
template<template<typename T2>class Array2>
void RegionT<Array>::CopyFrom( const RegionT<Array2>* right )
{
  *this = *right;
}

template<template<typename T>class Array>
template<template<typename T2>class Array2>
int RegionT<Array>::Compare( const RegionT<Array2>* cr ) const{
  if ( !cr )
    return -2;

  if (!IsDefined() && (!cr->IsDefined())){
    return 0;
  }

  if(!IsDefined()){
    return -1;
  }
  if(!cr->IsDefined()){
    return 1;
  }
  if(Size()<cr->Size()){
    return -1;
  }
  if(Size()>cr->Size()){
    return 1;
  }
  if(Size()==0){ // two empty regions
    return 0;
  }

  int bboxCmp = bbox.Compare( &cr->bbox );
  if(bboxCmp!=0){
   return bboxCmp;
  }

  HalfSegment hs1, hs2;
  for( int i = 0; i < Size(); i++ ) {
     Get( i, hs1);
     cr->Get( i, hs2 );
     int hsCmp = hs1.Compare(hs2);
     if(hsCmp!=0){
       return hsCmp;
     }
  }
  return 0;
}
template<template<typename T>class Array>
int RegionT<Array>::Compare( const Attribute* arg ) const
{
  RegionT<Array>* cr = (RegionT<Array>* )(arg);
  return Compare(cr);
}



template<template<typename T>class Array>
std::ostream& RegionT<Array>::Print( std::ostream &os ) const
{
  os << "<";
  if( !IsDefined() ) {
    os << " undefined ";
  } else {
    HalfSegment hs;
    for( int i = 0; i < Size(); i++ )
    {
      Get( i, hs );
      os << " " << hs;
    }
  }
  os << ">";
  return os;
}


template<template<typename T>class Array>
RegionT<Array> *RegionT<Array>::Clone() const
{
  return new RegionT( *this);
}

template<template<typename T>class Array>
bool RegionT<Array>::InsertOk( const HalfSegment& hs ) const
{
  HalfSegment auxhs;
  double dummyy0;

  // Uncommenting out the following line will increase performance when
  // bulk importing correct data:
  // CD: Problem: Database BerlinTest is currently faulty and cannot be
  //     restored when checking is enabled
  return true;

  int prevcycleMeet[50];

  int prevcyclenum=0;
  for( int i = 0; i < 50; i++ )
    prevcycleMeet[i]=0;

  for( int i = 0; i<= region.Size()-1; i++ )
  {
    region.Get( i, auxhs );

    if (auxhs.IsLeftDomPoint())
    {
      if (hs.Intersects(auxhs))
      {
        if ((hs.attr.faceno!=auxhs.attr.faceno)||
            (hs.attr.cycleno!=auxhs.attr.cycleno))
        {
          cout<<"two cycles intersect with the ";
          cout<<"following edges:";
          cout<<auxhs<<" :: "<<hs<<endl;
          return false;
        }
        else
        {
          if ((auxhs.GetLeftPoint()!=hs.GetLeftPoint()) &&
              (auxhs.GetLeftPoint()!=hs.GetRightPoint()) &&
              (auxhs.GetRightPoint()!=hs.GetLeftPoint()) &&
              (auxhs.GetRightPoint()!=hs.GetRightPoint()))
          {
            cout<<"two edges: " <<auxhs<<" :: "<< hs
                <<" of the same cycle intersect in middle!"
                <<endl;
            return false;
          }
        }
      }
      else
      {
        if ((hs.attr.cycleno>0) &&
            (auxhs.attr.faceno==hs.attr.faceno) &&
            (auxhs.attr.cycleno!=hs.attr.cycleno))
        {
          if (auxhs.RayAbove(hs.GetLeftPoint(), dummyy0))
          {
            prevcycleMeet[auxhs.attr.cycleno]++;
            if (prevcyclenum < auxhs.attr.cycleno)
              prevcyclenum=auxhs.attr.cycleno;
          }
        }
      }
    }
  }

  if ((hs.attr.cycleno>0))
  {
    if  (prevcycleMeet[0] % 2 ==0)
    {
      cout<<"hole(s) is not inside the outer cycle! "<<endl;
      return false;
    }
    for (int i=1; i<=prevcyclenum; i++)
    {
      if (prevcycleMeet[i] % 2 !=0)
      {
        cout<<"one hole is inside another! "<<endl;
        return false;
      }
    }
  }
/*
Now we know that the new half segment is not inside any other previous holes
of the same face. However, whether this new hole contains any previous hole
of the same face is not clear. In the following we do this kind of check.

*/

  if (((hs.attr.faceno>0) || (hs.attr.cycleno>2)))
  {
    HalfSegment hsHoleNEnd, hsHoleNStart;

    if (region.Size() ==0) return true;

    int holeNEnd=region.Size()-1;
    region.Get(holeNEnd, hsHoleNEnd );

    if  ((hsHoleNEnd.attr.cycleno>1) &&
         ((hs.attr.faceno!=hsHoleNEnd.attr.faceno)||
         (hs.attr.cycleno!=hsHoleNEnd.attr.cycleno)))
    {
      if (hsHoleNEnd.attr.cycleno>1)
      {
        int holeNStart=holeNEnd - 1;
        region.Get(holeNStart, hsHoleNStart );

        while ((hsHoleNStart.attr.faceno==hsHoleNEnd.attr.faceno) &&
               (hsHoleNStart.attr.cycleno==hsHoleNEnd.attr.cycleno)&&
               (holeNStart>0))
        {
          holeNStart--;
          region.Get(holeNStart, hsHoleNStart );
        }
        holeNStart++;

        int prevHolePnt=holeNStart-1;
        HalfSegment hsPrevHole, hsLastHole;

        bool stillPrevHole = true;
        while ((stillPrevHole) && (prevHolePnt>=0))
        {
          region.Get(prevHolePnt, hsPrevHole );
          prevHolePnt--;

          if ((hsPrevHole.attr.faceno!= hsHoleNEnd.attr.faceno)||
              (hsPrevHole.attr.cycleno<=0))
          {
            stillPrevHole=false;
          }

          if (hsPrevHole.IsLeftDomPoint())
          {
            int holeNMeent=0;
            for (int i=holeNStart; i<=holeNEnd; i++)
            {
              region.Get(i, hsLastHole );
              if ((hsLastHole.IsLeftDomPoint())&&
                  (hsLastHole.RayAbove
                  (hsPrevHole.GetLeftPoint(), dummyy0)))
                holeNMeent++;
            }
            if  (holeNMeent % 2 !=0)
            {
              cout<<"one hole is inside another!!! "<<endl;
              return false;
            }
          }
        }
      }
    }
  }
  return true;
}

std::vector<Point> getCycle(const bool isHole,
                              const std::vector<HalfSegment>& vhs);


template<template<typename T>class Array>
std::vector< std::vector <Point> > getCycles(const RegionT<Array>& reg){
      // first step , map halsfsegment according to faceno and cycleno

      std::map< std::pair<int, int> , std::vector<HalfSegment> > m;

      HalfSegment hs;
      for(int i=0;i<reg.Size(); i++){
         reg.Get(i,hs);
         if(hs.IsLeftDomPoint()){
            int faceno = hs.attr.faceno;
            int cycleno = hs.attr.cycleno;
            m[std::make_pair(faceno, cycleno)].push_back(hs);
         }
      }

      std::vector< std::vector <Point> > result;
      std::map< std::pair<int, int> , std::vector<HalfSegment> >::iterator it;
      for(it=m.begin(); it!=m.end(); it++){
        std::pair< std::pair<int, int> ,
                   std::vector<HalfSegment> > cycleDesc = *it;
        bool isHole = cycleDesc.first.second > 0;
        result.push_back(getCycle(isHole, cycleDesc.second));
      }
      return result;
}

template<template<typename T>class Array>
void RegionT<Array>::saveShape(std::ostream& o, uint32_t RecNo) const
{
     // first, write the record header
     WinUnix::writeBigEndian(o,RecNo);
     // an empty region
     if(!IsDefined() || IsEmpty()){
        uint32_t length = 2;
        WinUnix::writeBigEndian(o,length);
        uint32_t type = 0;
        WinUnix::writeLittleEndian(o,type);
     } else {

        std::vector<std::vector < Point> > cycles = getCycles(*this);

        uint32_t numParts = cycles.size();


        uint32_t  numPoints = 0;

        std::vector<std::vector < Point> >::iterator it;
        for(it = cycles.begin(); it!=cycles.end(); it++){
           numPoints += it->size();
        }

        uint32_t  numBytes = 44 + 4 * numParts + 16*numPoints;

        uint32_t length = numBytes / 2;


        WinUnix::writeBigEndian(o,length);
        WinUnix::writeLittleEndian(o,getshpType()); // 4
        double minX = getMinX();
        double maxX = getMaxX();
        double minY = getMinY();
        double maxY = getMaxY();
        // write the boundig box
        WinUnix::writeLittle64(o,minX);        // 8 * 4
        WinUnix::writeLittle64(o,minY);
        WinUnix::writeLittle64(o,maxX);
        WinUnix::writeLittle64(o,maxY);

        WinUnix::writeLittleEndian(o,numParts);
        WinUnix::writeLittleEndian(o,numPoints);

        // write the parts
        uint32_t pos = 0;
        for(unsigned int i=0; i<numParts; i++){
           WinUnix::writeLittleEndian(o,pos);
           pos += cycles[i].size();
        }
        // write the points
        for(unsigned int i=0;i<numParts; i++){
           std::vector<Point> cycle = cycles[i];
           for(unsigned int j=0;j<cycle.size(); j++){
              Point p = cycle[j];
              double x = p.GetX();
              double y = p.GetY();
              WinUnix::writeLittle64(o,x);
              WinUnix::writeLittle64(o,y);
           }
        }



     }
}


template<template<typename T>class Array>
void RegionT<Array>::writeShape(std::ostream& o, uint32_t RecNo) const
{
     saveShape(o,RecNo);
}

template<template<typename T>class Array>
template<class LineType>
double RegionT<Array>::Distance( const LineType &l, 
                                 const Geoid* geoid /*=0*/ ) const
{
  assert( !IsEmpty() ); // subsumes IsDefined()
  assert( !l.IsEmpty() ); // subsumes IsDefined()
  assert( !geoid || geoid->IsDefined() );
  if(geoid){
    cout << __PRETTY_FUNCTION__ << ": Spherical geometry not implemented."
         << std::endl;
    assert(false); // TODO: Implement spherical geometry case.
  }
  if( IsEmpty() || l.IsEmpty() || (geoid && !geoid->IsDefined()) ) {
     return -1;
  }
  double result = std::numeric_limits<double>::max();
  HalfSegment hs1, hs2;
  for(int i=0; i<l.Size();i++){
     l.Get(i,hs2);
     if(hs2.IsLeftDomPoint()){
       if(Contains(hs2.GetDomPoint(),geoid)){
         return 0.0;
       }
       if(Contains(hs2.GetSecPoint(),geoid)){
         return 0.0;
       }
     }
  }

  for( int i = 0; i < Size(); i++ ){
    Get( i, hs1 );
    if( hs1.IsLeftDomPoint() ){
      for( int j = 0; j < l.Size(); j++ ){
        l.Get( j, hs2 );
        if( hs2.IsLeftDomPoint() ){
          if( hs1.Intersects( hs2 ) ){
            return 0.0;
          }
          result = MIN( result, hs1.Distance( hs2, geoid ) );
        }
      }
    }
  }
  return result;
}


