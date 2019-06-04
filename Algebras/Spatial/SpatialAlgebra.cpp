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

[1] Implementation of the Spatial Algebra

February, 2003. Victor Teixeira de Almeida

March-July, 2003. Zhiming Ding

January, 2005 Leonardo Guerreiro Azevedo

[TOC]

1 Overview

This implementation file essentially contains the implementation of the classes
~Point~, ~Points~, ~Line~, and ~Region~ used in the Spatial Algebra. These
classes respectively correspond to the memory representation for the type
constructors ~point~, ~points~, ~line~, and ~region~.

For more detailed information see SpatialAlgebra.h.

2 Defines and Includes

*/

#undef __TRACE__
//#define __TRACE__ cout <<  __FILE__ << "::" << __LINE__;
#define __TRACE__



#include "Label.h"
#include "../../Tools/Flob/Flob.h"
#include "../../Tools/Flob/DbArray.h"
#include "Algebra.h"
#include "NestedList.h"
#include "ListUtils.h"
#include "GenericTC.h"
#include "QueryProcessor.h"
#include "StandardTypes.h"
#include "SpatialAlgebra.h"
#include "Algebras/FText/FTextAlgebra.h"
#include "SecondoConfig.h"
#include "AvlTree.h"
#include "AVLSegment.h"
#include "AlmostEqual.h"
#include "Algebras/Relation-C++/RelationAlgebra.h"
#include "RegionTools.h"
#include "Symbols.h"
#include "Algebras/Stream/Stream.h"
#include "RegionCreator.h"
#include "StringUtils.h"

#include <vector>
#include <queue>
#include <stdexcept>
#include <iostream>
#include <string.h>
#include <string>
#include <cmath>
#include <algorithm>
#include <ctime>
#include <queue>
#include <iterator>
#include <sstream>
#include <limits>
#include <errno.h>
#include <cerrno>
#include "../TopRel/Tree.h"

#include "RobustSetOps.h"

#include "DLine.h"
#include "Segment.h"

#include "DRM.h"
#include "Disc.h"
#include "Stack.h"

#include "GeoDist.h"


using namespace std;

extern NestedList* nl;
extern QueryProcessor* qp;


typedef RegionCreator<DbArray> PRegionCreator;
typedef robust::RealmChecker<DbArray> PRealmChecker;


/*
3 Type investigation auxiliaries

Within this algebra module, we have to handle with values of four different
types: ~point~ and ~points~, ~line~ and ~region~.

Later on we will
examine nested list type descriptions. In particular, we
are going to check whether they describe one of the four types just introduced.
In order to simplify dealing with list expressions describing these types, we
declare an enumeration, ~SpatialType~, containing the four types, and a
function, ~SpatialTypeOfSymbol~, taking a nested list as argument and returning
the corresponding ~SpatialType~ type name.

*/
enum SpatialType { stpoint, stpoints, stline, stregion,
                   stbox, sterror, stsline };

SpatialType
SpatialTypeOfSymbol( ListExpr symbol )
{
  if ( nl->AtomType( symbol ) == SymbolType )
  {
    string s = nl->SymbolValue( symbol );
    if ( s == Point::BasicType()  ) return (stpoint);
    if ( s == Points::BasicType() ) return (stpoints);
    if ( s == Line::BasicType()   ) return (stline);
    if ( s == Region::BasicType() ) return (stregion);
    if ( s == Rectangle<2>::BasicType()   ) return (stbox);
    if ( s == SimpleLine::BasicType()  ) return (stsline);
  }
  return (sterror);
}

/*
9 Object Traversal functions

These functions are utilities useful for traversing objects.  They are basic
functions to be called by the operations defined below.

There are 6 combinations, pp, pl, pr, ll, lr, rr

*/


void SelectFirst_pl( const Points& P, const Line& L,
                     object& obj, status& stat )
{
  P.SelectFirst();
  L.SelectFirst();

  Point p1(true);
  Point p2(true);
  HalfSegment hs;

  bool gotPt = P.GetPt( p1 ),
       gotHs = L.GetHs( hs );

  if( gotHs )
    p2 = hs.GetDomPoint();

  if( !gotPt && !gotHs )
  {
    obj = none;
    stat = endboth;
  }
  else if( !gotPt )
  {
    obj = second;
    stat = endfirst;
  }
  else if( !gotHs )
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
      obj=both;
  }
}

void SelectNext_pl( const Points& P, const Line& L,
                    object& obj, status& stat )
{
  // 1. get the current elements
  Point p1(true);
  Point p2(true);
  HalfSegment hs;

  bool gotPt = P.GetPt( p1 ),
       gotHs = L.GetHs( hs );

  if( gotHs )
    p2 = hs.GetDomPoint();

  //2. move the pointers
  if( !gotPt && !gotHs )
  {
    //do nothing
  }
  else if( !gotPt )
  {
    L.SelectNext();
    gotHs = L.GetHs( hs );
    if( gotHs )
      p2 = hs.GetDomPoint();
  }
  else if( !gotHs )
  {
    P.SelectNext();
    gotPt = P.GetPt( p1 );
  }
  else //both currently defined
  {
    if( p1 < p2 ) //then hs1 is the last output
    {
      P.SelectNext();
      gotPt = P.GetPt( p1 );
    }
    else if( p1 > p2 )
    {
      L.SelectNext();
      gotHs = L.GetHs( hs );
      if( gotHs )
        p2 = hs.GetDomPoint();
    }
    else
    {
      P.SelectNext();
      gotPt = P.GetPt( p1 );
      L.SelectNext();
      gotHs = L.GetHs( hs );
      if( gotHs )
        p2 = hs.GetDomPoint();
    }
  }

  //3. generate the outputs
  if( !gotPt && !gotHs )
  {
    obj = none;
    stat = endboth;
  }
  else if( !gotPt )
  {
    obj = second;
    stat = endfirst;
  }
  else if( !gotHs )
  {
    obj = first;
    stat = endsecond;
  }
  else //both defined
  {
    stat=endnone;
    if( p1 < p2 )
      obj = first;
    else if( p1 > p2 )
      obj = second;
    else
      obj = both;
  }
}

void SelectFirst_pr( const Points& P, const Region& R,
                     object& obj, status& stat )
{
  P.SelectFirst();
  R.SelectFirst();

  Point p1(true);
  Point p2(true);
  HalfSegment hs;

  bool gotPt = P.GetPt( p1 ),
       gotHs = R.GetHs( hs );
  if( gotHs )
    p2 = hs.GetDomPoint();

  if( !gotPt && !gotHs )
  {
    obj = none;
    stat = endboth;
  }
  else if( !gotPt )
  {
    obj = second;
    stat = endfirst;
  }
  else if( !gotHs )
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
      obj=both;
  }
}

void SelectNext_pr( const Points& P, const Region& R,
                    object& obj, status& stat )
{
  // 1. get the current elements
  Point p1(true);
  Point p2(true);
  HalfSegment hs;

  bool gotPt = P.GetPt( p1 ),
       gotHs = R.GetHs( hs );
  if( gotHs )
    p2 = hs.GetDomPoint();

  //2. move the pointers
  if( !gotPt && !gotHs )
  {
    //do nothing
  }
  else if( !gotPt )
  {
    R.SelectNext();
    gotHs = R.GetHs( hs );
    if( gotHs )
      p2 = hs.GetDomPoint();
  }
  else if( !gotHs )
  {
    P.SelectNext();
    gotPt = P.GetPt( p1 );
  }
  else //both currently defined
  {
    if( p1 < p2 ) //then hs1 is the last output
    {
      P.SelectNext();
      gotPt = P.GetPt( p1 );
    }
    else if( p1 > p2 )
    {
      R.SelectNext();
      gotHs = R.GetHs( hs );
      if( gotHs )
        p2 = hs.GetDomPoint();
    }
    else
    {
      P.SelectNext();
      gotPt = P.GetPt( p1 );
      R.SelectNext();
      gotHs = R.GetHs( hs );
      if( gotHs )
        p2 = hs.GetDomPoint();
    }
  }

  //3. generate the outputs
  if( !gotPt && !gotHs )
  {
    obj = none;
    stat = endboth;
  }
  else if( !gotPt )
  {
    obj = second;
    stat = endfirst;
  }
  else if( !gotHs )
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


void SelectFirst_lr( const Line& L, const Region& R,
                     object& obj, status& stat )
{
  L.SelectFirst();
  R.SelectFirst();

  HalfSegment hs1, hs2;
  bool gotHs1 = L.GetHs( hs1 ),
       gotHs2 = R.GetHs( hs2 );

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

void SelectNext_lr( const Line& L, const Region& R,
                    object& obj, status& stat )
{
  // 1. get the current elements
  HalfSegment hs1, hs2;
  bool gotHs1 = L.GetHs( hs1 ),
       gotHs2 = R.GetHs( hs2 );

  //2. move the pointers
  if( !gotHs1 && !gotHs2 )
  {
    //do nothing
  }
  else if( !gotHs1 )
  {
    R.SelectNext();
    gotHs2 = R.GetHs( hs2 );
  }
  else if( !gotHs2 )
  {
    L.SelectNext();
    gotHs1 = L.GetHs( hs1 );
  }
  else //both currently defined
  {
    if( hs1 < hs2 ) //then hs1 is the last output
    {
      L.SelectNext();
      gotHs1 = L.GetHs( hs1 );
    }
    else if( hs1 > hs2 )
    {
      R.SelectNext();
      gotHs2 = R.GetHs( hs2 );
    }
    else
    {
      L.SelectNext();
      gotHs1 = L.GetHs( hs1 );
      R.SelectNext();
      gotHs2 = R.GetHs( hs2 );
    }
  }

  //3. generate the outputs
  if( !gotHs1 && gotHs2 )
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



/*
3 Auxiliary classes

*/

// Generic print-function object for printing with STL::for_each
template<class T> struct print : public unary_function<T, void>
{
  print(ostream& out) : os(out) {}
  void operator() (T x) { os << "\t" << x << "\n"; }
  ostream& os;
};


/*
4 Type Constructor ~point~

A value of type ~point~ represents a point in the Euclidean plane or is
undefined.

4.1 Implementation of the class ~Point~

*/
ostream& operator<<( ostream& o, const Point& p )
{
  ios_base::fmtflags oldOptions = o.flags();
  o.setf(ios_base::fixed,ios_base::floatfield);
  o.precision(8);
  if( p.IsDefined() )
    o << "(" << p.GetX() << ", " << p.GetY() << ")";
  else
    o << Symbol::UNDEFINED();
  o.flags(oldOptions);
  return o;
}

const Rectangle<2> Point::BoundingBox(const Geoid* geoid /*=0*/) const
{
  assert( IsDefined() );
  if( IsDefined() ) {
    if( geoid && geoid->IsDefined() ){ // spherical case
      double minx = MAX(-180, x - ApplyFactor(x));
      double maxx = MIN(180, x+ ApplyFactor(x));
      double miny = MAX(-90, y-ApplyFactor(y));
      double maxy = MIN(90,y+ApplyFactor(y));
      double minMax[] = {minx,maxx,miny,maxy};
      return Rectangle<2>( true, minMax);
    } else if(!geoid){
      double minMax[] = { x - ApplyFactor(x),
                          x + ApplyFactor(x),
                          y - ApplyFactor(y),
                          y + ApplyFactor(y) };
      return Rectangle<2>( true, minMax);
    }
  } // else:
  return Rectangle<2>( false );
}
ostream& Point::Print( ostream &os ) const
{
  return os << *this;
}

string Point::toString(const Geoid* geoid /*=0*/) const {
  if(!IsDefined()){
    return Symbol::UNDEFINED();
  }
  stringstream s;
  s.setf(ios_base::fixed,ios_base::floatfield);
  s.precision(8);
  if(geoid){ // print as geographic coords
    double lat = GetY();
    double lon = GetX();
    string ns = (lat>=0)?"N":"S";
    string ew = (lon>=0)?"E":"W";

    lat = fabs(lat);
    int degLat = (int)lat;
    double resLat = (lat - degLat)*60.0;
    int minLat = ((int) resLat);
    double secLat = (resLat - minLat)*60.0;

    lon = fabs(lon);
    int degLon = (int)lon;
    double resLon = (lon - degLon)*60.0;
    int minLon = ((int) resLon);
    double secLon = (resLon - minLon)*60.0;
    s << "(" << degLat << "°" << minLat << "'" << secLat << "\"" << ns << ", "
             << degLon << "°" << minLon << "'" << secLon << "\"" << ew << ")";
  } else { // print as euclidean coords
    s << *this;
  }
  return s.str();
}


bool Point::Inside( const Rectangle<2>& r, const Geoid* geoid /*=0*/ ) const
{
  assert( r.IsDefined() );
  if( !IsDefined() || !r.IsDefined() || (geoid && !geoid->IsDefined()) ){
    return false;
  }
  if( x < r.MinD(0) || x > r.MaxD(0) )
    return false;
  else if( y < r.MinD(1) || y > r.MaxD(1) )
    return false;
  return true;
}

void Point::ReadFromString(string value) {

  ListExpr list;
  if(!nl->ReadFromString(value,list)){
     if(!nl->ReadFromString("("+value+")",list)){
        SetDefined(false); 
        return;
     }
  }
  if(listutils::isSymbolUndefined(list)){
     SetDefined(false);
     return;
  }
  if(    nl->HasLength(list,2) 
      && listutils::isNumeric(nl->First(list))
      && listutils::isNumeric(nl->Second(list))){
     Set(listutils::getNumValue(nl->First(list)),
         listutils::getNumValue(nl->Second(list)));
     return;
  }
  if(    nl->HasLength(list,3) 
      && listutils::isNumeric(nl->First(list))
      && listutils::isNumeric(nl->Third(list))){
     Set(listutils::getNumValue(nl->First(list)),
         listutils::getNumValue(nl->Third(list)));
     return;
  }
  SetDefined(false);
}

template<template<typename T>class Array>
void Point::Intersection(const Point& p, PointsT<Array>& result,
                         const Geoid* geoid /*=0*/ ) const{
  result.Clear();
  if(!IsDefined() || !p.IsDefined() || (geoid && !geoid->IsDefined()) ){
    result.SetDefined(false);
    return;
  }
  result.SetDefined(true);
  if(AlmostEqual(*this, p)){
    result += *this;
  }
}

template<template<typename T>class Array>
void Point::Intersection(const PointsT<Array>& ps, PointsT<Array>& result,
                         const Geoid* geoid /*=0*/ ) const{
  ps.Intersection(*this, result, geoid);
}

template<template<typename T>class Array>
void Point::Intersection(const LineT<Array>& l, PointsT<Array>& result,
                         const Geoid* geoid /*=0*/) const{
  l.Intersection(*this, result, geoid);
}

template<template<typename T>class Array>
void Point::Intersection(const RegionT<Array>& r, PointsT<Array>& result,
                         const Geoid* geoid /*=0*/) const{
  r.Intersection(*this, result, geoid);
}

template<template<typename T>class Array>
void Point::Intersection(const SimpleLineT<Array>& l, PointsT<Array>& result,
                         const Geoid* geoid /*=0*/) const{
  l.Intersection(*this, result, geoid);
}

template<template<typename T>class Array>
void Point::Minus(const Point& p, PointsT<Array>& result,
                  const Geoid* geoid /*=0*/) const{
  result.Clear();
  if(!IsDefined() || !p.IsDefined() || (geoid && !geoid->IsDefined())){
    result.SetDefined(false);
    return;
  }
  result.SetDefined(true);
  if(!AlmostEqual(*this, p)){
     result += *this;
  }

}
template<template<typename T>class Array>
void Point::Minus(const PointsT<Array>& ps, PointsT<Array>& result,
                  const Geoid* geoid /*=0*/) const{
  result.Clear();
  if(!IsDefined() || !ps.IsDefined() || (geoid && !geoid->IsDefined())){
    result.SetDefined(false);
    return;
  }
  result.SetDefined(true);

  if(!ps.Contains(*this, geoid)){
    result += *this;
  }
}

template<template<typename T>class Array>
void Point::Minus(const LineT<Array>& l, PointsT<Array>& result,
                  const Geoid* geoid /*=0*/) const{
  result.Clear();
  if(!IsDefined() || !l.IsDefined() || (geoid && !geoid->IsDefined())){
    result.SetDefined(false);
    return;
  }
  result.SetDefined(true);
  if(!l.Contains(*this, geoid)){
     result += *this;
  }
}

template<template<typename T>class Array>
void Point::Minus(const RegionT<Array>& r, PointsT<Array>& result,
                  const Geoid* geoid /*=0*/) const{
  result.Clear();
  if(!IsDefined() || !r.IsDefined() || (geoid && !geoid->IsDefined())){
    result.SetDefined(false);
    return;
  }
  result.SetDefined(true);
  if(!r.Contains(*this, geoid)){
    result += *this;
  }
}

template<template<typename T>class Array>
void Point::Minus(const SimpleLineT<Array>& l, PointsT<Array>& result,
                  const Geoid* geoid /*=0*/) const{
  result.Clear();
  if(!IsDefined() || !l.IsDefined() || (geoid && !geoid->IsDefined())){
    result.SetDefined(false);
    return;
  }
  result.SetDefined(true);
  if(!l.Contains(*this, geoid)){
    result += *this;
  }
}

template<template<typename T>class Array>
void Point::Union(const Point& p, PointsT<Array>& result,
                  const Geoid* geoid /*=0*/) const{
  result.Clear();
  if(!IsDefined() || !p.IsDefined() || (geoid && !geoid->IsDefined())){
    result.SetDefined(false);
    return;
  }
  result.SetDefined(true);
  result.StartBulkLoad();
  result += *this;
  result += p;
  result.EndBulkLoad();
}

template<template<typename T>class Array>
void Point::Union(const PointsT<Array>& ps, PointsT<Array>& result,
                  const Geoid* geoid /*=0*/) const{
  ps.Union(*this, result, geoid);
}

template<template<typename T>class Array>
void Point::Union(const LineT<Array>& l, LineT<Array>& result,
                  const Geoid* geoid /*=0*/) const{
  l.Union(*this, result, geoid);
}

template<template<typename T>class Array>
void Point::Union(const RegionT<Array>& r, RegionT<Array>& result,
                  const Geoid* geoid /*=0*/) const{
  r.Union(*this, result, geoid);
}

template<template<typename T>class Array>
void Point::Union(const SimpleLineT<Array>& l, SimpleLineT<Array>& result,
                  const Geoid* geoid /*=0*/) const{
  l.Union(*this, result, geoid);
}

double Point::Distance( const Point& p, const Geoid* geoid /* = 0 */ ) const
{
  assert( IsDefined() );
  assert( p.IsDefined() );
  assert( !geoid || geoid->IsDefined() );
  if(geoid){
    bool ok = false;
    double bearInitial = 0, bearFinal = 0;
    double d = DistanceOrthodromePrecise(p,*geoid,ok,bearInitial,bearFinal);
    if(ok){
      return d;
    } else{
      assert(false);
    }
  } else {
    double dx = p.x - x,
           dy = p.y - y;
    return sqrt( pow( dx, 2 ) + pow( dy, 2 ) );
  }
}

double Point::Distance( const Rectangle<2>& r, const Geoid* geoid/*=0*/ ) const
{
  assert( IsDefined() );
  assert( r.IsDefined() );
  assert( !geoid || geoid->IsDefined() );
  if(geoid){
    cout << __PRETTY_FUNCTION__ << ": Spherical geometry not implemented!"
         << endl; // TODO: implement spherical gemetry case
    assert(false);
  }
  double rxmin = r.MinD(0), rxmax = r.MaxD(0),
         rymin = r.MinD(1), rymax = r.MaxD(1);
  double dx =
        (   (x > rxmin || AlmostEqual(x,rxmin))
         && (x < rxmax || AlmostEqual(x,rxmax))) ? (0.0) :
        (min(abs(x-rxmin),abs(x-rxmax)));
  double dy =
        (   (y > rymin || AlmostEqual(y,rymin))
         && (y < rymax || AlmostEqual(y,rymax))) ? (0.0) :
        (min(abs(y-rymin),abs(y-rymax)));

  return sqrt( pow( dx, 2 ) + pow( dy, 2 ) );
}

bool Point::Intersects(const Rectangle<2>& r, const Geoid* geoid/*=0*/) const{
  assert(IsDefined());
  assert(r.IsDefined());
  assert(!geoid); // not implemented yet
  return     (x>=r.MinD(0) ) && (x<=r.MaxD(0)) 
          && (y>=r.MinD(1) ) && (y<=r.MaxD(1)); 

}



  // calculate the enclosed angle between (a,b) and (b,c) in degrees
double Point::calcEnclosedAngle( const Point &a,
        const Point &b,
        const Point &c,
        const Geoid* geoid /* = 0 */){
  double beta = 0.0;
  errno = 0;
  if(geoid){ // use sperical trigonometry
    // very simplistic, works for short distances, but
    // should be improved!
    bool ok = true;
    double la = b.DistanceOrthodrome(c,*geoid, ok); // la = |(b,c)|
    assert(ok);
    double lb = a.DistanceOrthodrome(c,*geoid, ok); // lb = |(a,c)|
    assert(ok);
    double lc = a.DistanceOrthodrome(b,*geoid, ok); // lc = |(a,b)|
    assert(ok);
    assert(la != 0.0);
    assert(lc != 0.0);
    double cosb = (la*la + lc*lc - lb*lb) / (2*la*lc);
    cosb = max(cosb, -1.0);
    cosb = min(cosb, 1.0);
    beta = radToDeg( acos(cosb) );
    assert(errno == 0);
  } else { // use euclidean geometry
    double la = b.Distance(c);
    double lb = a.Distance(c);
    double lc = a.Distance(b);
    double cosb = (la*la + lc*lc - lb*lb) / (2*la*lc);
    cosb = max(cosb, -1.0);
    cosb = min(cosb, 1.0);
    beta = radToDeg( acos(cosb) );
    assert(errno == 0);
  }
  return beta;
}


// return the direction at starting point ~this~
double Point::Direction(const Point& p ,
                        const bool returnHeading,
                        const Geoid* geoid,
                        const bool atEndpoint /*=false*/ ) const{

  if(!IsDefined() || !p.IsDefined() || (geoid && !geoid->IsDefined()) ){
    throw( new SecondoException(string("invalid Arguments") + __FILE__ + 
                                  "  " + stringutils::int2str(__LINE__)));
    return -1.0;
  }
  if(geoid && (!checkGeographicCoord() || !p.checkGeographicCoord()) ){
    cerr << __PRETTY_FUNCTION__ << ": Invalid geographic coordinate." << endl;
    throw( new SecondoException(string("invalid coordinate") + __FILE__ + "  "
                                       + stringutils::int2str(__LINE__)));
    return -1.0;
  }
  if(AlmostEqual(*this, p)){
    throw( new SecondoException(string("equal points") + __FILE__ + "  "
                                 + stringutils::int2str(__LINE__)));
    return -1.0;
  }
  errno = 0;
  double direction = 0;
  if(!geoid){ //euclidean geometry
    Coord x1 = x,
          y1 = y,
          x2 = p.x,
          y2 = p.y;

    if( AlmostEqual( x1, x2 ) ){
      if( y2 > y1 ){
        direction = 90.0;
      } else {
        direction = 270.0;
      }
    } else if( AlmostEqual( y1, y2 ) ){
      if( x2 > x1 ){
        direction = 0.0;
      } else {
        direction = 180.0;
      }
    } else {
      direction = radToDeg( atan2( (y2 - y1) , (x2 - x1) ) );
      direction = (direction <0.0)?direction+360.0:direction;
    }
    if(errno != 0) {
      cerr << __PRETTY_FUNCTION__ << ": Numerical error in euclidean." << endl;
    throw( new SecondoException(string("numerical error") + __FILE__ + "  " 
                                        + stringutils::int2str(__LINE__)));
      return -1.0;
    }
    // normalize return value:
    if(returnHeading){ // return result as standard heading
      direction = directionToHeading(direction); // already normalized
    } else { // return result as standard direction: Normalize to 0<=DIR<360
      direction = AlmostEqual(direction,360.0)?0.0:direction;
    }
    return direction;
  } else {// spherical geometry
    double tc1=0.0, tc2=0.0; // true course at *this, p
    bool valid = true;
    double d = DistanceOrthodromePrecise(p, *geoid, valid, tc1, tc2);
    if(!valid || (d<0)){
      throw( new SecondoException(string("error in distance computation") 
                        + __FILE__ + "  " + stringutils::int2str(__LINE__)));
      return -1.0;
    }
    if(returnHeading){
      return (atEndpoint)?tc2:tc1;
    } else {
      return headingToDirection((atEndpoint)?tc2:tc1);
    }
  }
}

// Returns the vertex, i.e. the southernmost/northernmost point
// of the orthodrome *this -> p
Point Point::orthodromeVertex(const double& rcourseDEG) const {
  if( !checkGeographicCoord() || (rcourseDEG<0) || (rcourseDEG>360) ){
    return Point(false, 0.0, 0.0);
  }
  double Latv = 0;
  double Lov = 0;
  // Latitude Latv of Vertex
  double rcourse = directionToHeading(rcourseDEG); // orthodr. course in degree
  double cosLatv = cos(degToRad(this->GetY()))*sin(degToRad(rcourse));
  Latv = radToDeg(acos(cosLatv));
  if (AlmostEqual(rcourse,0) || AlmostEqual(rcourse,180)){
    Latv = 90;
  }
  if (Latv > 90){
    Latv = 180 - Latv;
  }
  if (this->GetY()<0){
    Latv = - Latv;
  }
  // Longitude Lov of Vertex
  if (!AlmostEqual(Latv,0)){
    double sinDlov = cos(degToRad(rcourse))/sin(degToRad(Latv));
    double Dlov = radToDeg(asin(sinDlov));
    if (rcourse > 180){
      Dlov = -Dlov;
    }
    Lov = Dlov + this->GetX();
  } else {
    Lov = 0;
  }
  return Point(true, Lov, Latv);
}

int Point::orthodromeAtLatitude( const Point &other, const double& latitudeDEG,
                          double& lonMinDEG, double& lonMaxDEG) const{
  if(    !checkGeographicCoord() || !other.checkGeographicCoord()
      || AlmostEqual(*this,other)
      || (latitudeDEG<-90) || (latitudeDEG>90) ){
    throw( new SecondoException(string("invalid parameter") + __FILE__ + "  " 
                                        + stringutils::int2str(__LINE__)));
    return 0;
  }

  double lon1 = degToRad(GetX());
  double lat1 = degToRad(GetY());
  double lon2 = degToRad(other.GetX());
  double lat2 = degToRad(other.GetY());
  double lat3 = degToRad(latitudeDEG);

  double l12 = lon1-lon2;
  double A = sin(lat1)*cos(lat2)*cos(lat3)*sin(l12);
  double B =   sin(lat1)*cos(lat2)*cos(lat3)*cos(l12)
             - cos(lat1)*sin(lat2)*cos(lat3);
  double C = cos(lat1)*cos(lat2)*sin(lat3)*sin(l12);
  double lon = atan2(B,A);
  if(fabs(C) <= sqrt(A*A + B*B)){
    double dlon = acos(C/sqrt(A*A+B*B));
    double lon3_1=fmod2(lon1+dlon+lon+M_PI, 2*M_PI) - M_PI;
    double lon3_2=fmod2(lon1-dlon+lon+M_PI, 2*M_PI) - M_PI;
    lonMinDEG = radToDeg(MIN(lon3_1,lon3_2));
    lonMaxDEG = radToDeg(MAX(lon3_1,lon3_2));
    if(AlmostEqual(lon3_1,lon3_2)){
      return 1;
    }
    return 2;
  }
  throw( new SecondoException(string("numerical error") + __FILE__ 
                                     + "  " + stringutils::int2str(__LINE__)));
  return 0;
}

bool isBetween(const double value, const double bound1, const double bound2){
  return ( (MIN(bound1,bound2)<=value) && (value<=MAX(bound1,bound2)) );
}

bool Point::orthodromeExtremeLatitudes(const Point &other, const Geoid &geoid,
                                double &minLat, double &maxLat) const
{
  if(!checkGeographicCoord() || !other.checkGeographicCoord()
                             || !geoid.IsDefined() ){
    minLat =  1000.0;
    maxLat = -1000.0;
//     cerr << __PRETTY_FUNCTION__ << ": Invalid parameter." << endl;
    throw( new SecondoException(string("invalid parameter") + __FILE__ 
                                + "  " + stringutils::int2str(__LINE__)));
    return false;
  }
  if(AlmostEqual(*this,other)){
//     cerr << __PRETTY_FUNCTION__ << ": *this == other." << endl;
    minLat=MIN(this->GetY(),other.GetY());
    maxLat=MAX(this->GetY(),other.GetY());
//     cerr << __PRETTY_FUNCTION__ << ": minLat=" << minLat << endl;
//     cerr << __PRETTY_FUNCTION__ << ": maxLat=" << maxLat << endl;
    return true;
  }
  double trueCourse = Direction(other,false,&geoid);
  if(AlmostEqual(trueCourse,0) || AlmostEqual(trueCourse,180)){
//     cerr << __PRETTY_FUNCTION__ << ": Polar course." << endl;
    minLat=MIN(this->GetY(),other.GetY());
    maxLat=MAX(this->GetY(),other.GetY());
//     cerr << __PRETTY_FUNCTION__ << ": minLat=" << minLat << endl;
//     cerr << __PRETTY_FUNCTION__ << ": maxLat=" << maxLat << endl;
    return true;
  }
  Point vertex = orthodromeVertex(trueCourse);
  if(!vertex.IsDefined()){
//     cerr << __PRETTY_FUNCTION__ << ": Cannot compute vertex." << endl;
    minLat=MIN(this->GetY(),other.GetY());
    maxLat=MAX(this->GetY(),other.GetY());
//     cerr << __PRETTY_FUNCTION__ << ": minLat=" << minLat << endl;
//     cerr << __PRETTY_FUNCTION__ << ": maxLat=" << maxLat << endl;
    throw( new SecondoException(string("invalid parameter") + __FILE__ 
                                  + "  " + stringutils::int2str(__LINE__)));
    return false;
  }
//   cerr << __PRETTY_FUNCTION__ << ": vertex=" << vertex << endl;

  double vertexReachedLonMin =  6666.6666;
  double vertexReachedLonMax = -6666.6666;
  int notransgressions = orthodromeAtLatitude(other,vertex.GetY(),
                                              vertexReachedLonMin,
                                              vertexReachedLonMax);
  if(notransgressions == 0){ // vertex not passed, should not happen
    cerr << __PRETTY_FUNCTION__ << ": Vertex not passed, should not happen."
         << endl;
    assert(false);
    return false;
  } else if(notransgressions == 1) { // vertex passed once.
    vertexReachedLonMax = vertexReachedLonMin; //
  } // else: vertex passed twice
  if(    (    isBetween(directionToHeading(trueCourse),0.0,180.0)
           && (    (vertexReachedLonMin<MIN(GetX(),other.GetX()))
                || (vertexReachedLonMax>MAX(GetX(),other.GetX())) ) )
      || (    isBetween(directionToHeading(trueCourse),180.0,360.0)
           && !(   (vertexReachedLonMin<MIN(GetX(),other.GetX()))
                 ||(vertexReachedLonMax>MAX(GetX(),other.GetX())) ) ) ){
    // vertex is not reached by this orthodrome
//     cerr << __PRETTY_FUNCTION__ << ": Vertex not passed." << endl;
    minLat = MIN(GetY(),other.GetY());
    maxLat = MAX(GetY(),other.GetY());
  } else if( (GetY()>=0) && (other.GetY()>=0) ){// both in northern hemissphere
    // vertex may be north of this and p
//     cerr << __PRETTY_FUNCTION__ << ": Both northern hemissphere." << endl;
    minLat = MIN(GetY(),other.GetY());
    maxLat = MAX(vertex.GetY(),MAX(GetY(),other.GetY()));
  } else if( (GetY()<0) && (other.GetY()<0) ){// both in southern hemissphere
    // vertex may be south of this and p
//     cerr << __PRETTY_FUNCTION__ << ": Both southern hemissphere." << endl;
    minLat = MIN(vertex.GetY(),MIN(GetY(),other.GetY()));
    maxLat = MAX(GetY(),other.GetY());
  } else { // both in different hemisspheres
    // vertex is either this or p
//     cerr << __PRETTY_FUNCTION__ << ": Both in different hemisspheres."<<endl;
    minLat = MIN(vertex.GetY(),MIN(GetY(),other.GetY()));
    maxLat = MAX(vertex.GetY(),MAX(GetY(),other.GetY()));
  }
//   cerr << __PRETTY_FUNCTION__ << ": minLat=" << minLat << endl;
//   cerr << __PRETTY_FUNCTION__ << ": maxLat=" << maxLat << endl;
  return true;
}

Rectangle<2> Point::GeographicBBox(const Point &other, const Geoid &geoid) const
{
  if(!checkGeographicCoord() || !other.checkGeographicCoord()
    || !geoid.IsDefined())
  {
    return Rectangle<2>(false);
  }
  double northmostLAT;
  double southmostLAT;
  orthodromeExtremeLatitudes(other, geoid, southmostLAT, northmostLAT);
//   cerr << __PRETTY_FUNCTION__ << ": northmostLAT = " << northmostLAT << endl;
//   cerr << __PRETTY_FUNCTION__ << ": southmostLAT = " << southmostLAT << endl;
//   cerr << __PRETTY_FUNCTION__ << ": GetX()=" << GetX() << endl;
//   cerr << __PRETTY_FUNCTION__ << ": other.GetX()=" << other.GetX() << endl;
//   cerr << __PRETTY_FUNCTION__ << ": MIN(GetX(),other.GetX()) = "
//        << MIN(GetX(),other.GetX()) << endl;
//   cerr << __PRETTY_FUNCTION__ << ": MAX(GetX(),other.GetX()) = "
//        << MAX(GetX(),other.GetX()) << endl;

  Point p1( true, MIN(GetX(),other.GetX()), southmostLAT );
  Point p2( true, MAX(GetX(),other.GetX()), northmostLAT );
//   cerr << __PRETTY_FUNCTION__ << "p1=" << p1 << ", bbox="
///       << p1.BoundingBox() << endl;
//   cerr << __PRETTY_FUNCTION__ << "p2=" << p2 << ", bbox="
//        << p2.BoundingBox() << endl;
//   cerr << __PRETTY_FUNCTION__ << "res="
//        << p1.BoundingBox().Union(p2.BoundingBox()) << endl;
  return p1.BoundingBox().Union(p2.BoundingBox());
}

void Point::Rotate(const Coord& x, const Coord& y,
                          const double alpha, Point& res) const{

  if(!IsDefined()){
     res.SetDefined(false);
     return;
  }

  double s = sin(alpha);
  double c = cos(alpha);

  double m00 = c;
  double m01 = -s;
  double m02 = x - x*c + y*s;
  double m10 = s;
  double m11 = c;
  double m12 = y - x*s-y*c;

  res.Set(  m00*this->x + m01*this->y + m02,
            m10*this->x + m11*this->y + m12);

}

/*
4.1 Spherical geometry operations

While the preceding operations use euclidic geometry, the following operations
use Spherical geometry.

*/

bool Point::checkGeographicCoord() const {
  return    IsDefined()
         && (GetX() >= -180) && (GetX() <= 180)  // x <-> Longitude (Laenge)
         && (GetY() >= -90) && (GetY() <= 90);   // y <-> Latitude (Breite)
}

/*
Distance between two points given in geodetic (Lat, Lon)-coordinates.
The distance is measured along a geoid passed as an argument.
The result uses the same unit as the geoid'd radius.

If an undefined Point or a Point with an invalid geographic coordinate is used,
~valid~ is set to false, otherwise the result is calculated and ~valid~ is set
to true.

*/
double Point::DistanceOrthodrome( const Point& p,
                                  const Geoid& g,
                                  bool& valid) const
{
  valid =  this->checkGeographicCoord() && p.checkGeographicCoord();
  if(!valid){
    return 0.0;
  }
  double a = g.getR(); // sphere's equatorical radius (from geoid)
  double f = g.getF(); // sphere's flattening (from geoid)

  // convert coordinates from degrees to radiant
  double b1 = GetY(),   l1 = GetX();  // X=longitude, Y=latitude
  double b2 = p.GetY(), l2 = p.GetX();// X=longitude, Y=latitude

  if(AlmostEqual(*this,p)){
    return 0.0;
  }
  double F = (b1 + b2) * M_PI / 360.0;
  double G = (b1 - b2) * M_PI / 360.0;
  double l = (l1 - l2) * M_PI / 360.0;

  // compute approximate distance
  double coslsq = cos(l) * cos(l);
  double sinlsq = sin(l) * sin(l);
  double sinG = sin(G);
  double cosG = cos(G);
  double cosF = cos(F);
  double sinF = sin(F);
  double sinGsq = sinG*sinG;
  double cosGsq = cosG*cosG;
  double sinFsq = sinF*sinF;
  double cosFsq = cosF*cosF;

  double S = sinGsq*coslsq + cosFsq*sinlsq;
  double C = cosGsq*coslsq + sinFsq*sinlsq;

  errno = 0;
  double SoverC = S/C;
  assert( errno == 0 );
  assert( SoverC >= 0 );
  double w = atan(sqrt(SoverC));
  assert( errno == 0 );
  double D = 2*w*a;

  // correct the distance
  double R = sqrt(S*C)/w;
  assert( errno == 0 );
  double H1 = (3*R-1)/(2*C);
  assert( errno == 0 );
  double H2 = (3*R+1)/(2*S);
  assert( errno == 0 );
  double s = D*( 1 + f*H1*sinFsq*cosGsq - f*H2*cosFsq*sinGsq );
  assert( errno == 0 );
  assert( s >= 0 );
  return s;
}


double Point::DistanceOrthodromePrecise( const Point& p,
                                         const Geoid& g,
                                         bool& valid,
                                         double& initialBearingDEG,
                                         double& finalBearingDEG,
                                         const double epsilon /* = 1 */) const
{
 /*
  * Vincenty Inverse Solution of Geodesics on the Ellipsoid
  *
  * from: Vincenty inverse formula - T Vincenty, "Direct and Inverse Solutions
  *       of Geodesics on the Ellipsoid with application of nested equations",
  *       Survey Review, vol XXII no 176, 1975
  *       http://www.ngs.noaa.gov/PUBS_LIB/inverse.pdf
  * Calculates geodetic distance between two points specified by
  * latitude/longitude using Vincenty inverse formula for ellipsoids
  *
  */

  valid =    this->checkGeographicCoord() && p.checkGeographicCoord()
          && g.IsDefined();
  if(!valid){
    cout << __PRETTY_FUNCTION__ << ": Invalid coordinates." << endl;
    return-666.666;
  }
  initialBearingDEG = -666.666;
  finalBearingDEG   = -666.666;
  if(AlmostEqual(*this,p)){
    return 0;
  }
  errno = 0;
  double lat1 = GetY();
  double lon1 = GetX();
  double lat2 = p.GetY();
  double lon2 = p.GetX();
  double a = g.getR();// = 6378137
  double f = g.getF();// = 1/298.257223563
  double b = (1-f)*a; // = 6356752.314245
  double L = degToRad(lon2-lon1);
  double U1 = atan( (1-f)*tan(degToRad(lat1)) );
  double U2 = atan( (1-f)*tan(degToRad(lat2)) );
  double sinU1 = sin(U1);
  double cosU1 = cos(U1);
  double sinU2 = sin(U2);
  double cosU2 = cos(U2);
  if(errno!=0){
    cout << __PRETTY_FUNCTION__ << ": Error at (1)." << endl;
    valid = false;
    return -666.666;
  }

  double lambda = L;
  double lambdaP;
  int iterLimit = 100;
  int iter = 1;
  double sinLambda, cosLambda,
         sinSigma, cosSigma, sigma,
         sinAlpha, cosSqAlpha,
         cos2SigmaM, C;
  do{
    sinLambda = sin(lambda);
    cosLambda = cos(lambda);
    sinSigma = sqrt(
              (cosU2*sinLambda)
            * (cosU2*sinLambda)
        +     (cosU1*sinU2-sinU1*cosU2*cosLambda)
            * (cosU1*sinU2-sinU1*cosU2*cosLambda));
    if (sinSigma==0){ // co-incident points
      valid = false;
      cout << __PRETTY_FUNCTION__ << ": Error: Co-incident points: *this="
           << *this << ", p=" << p << endl;
      valid = false;
      return -666.666;
    }
    if(errno!=0){
      cout << __PRETTY_FUNCTION__ << ": Error (1) in iteration " <<iter<< endl;
      valid = false;
      return -666.666;
    }
    cosSigma = sinU1*sinU2 + cosU1*cosU2*cosLambda;
    sigma = atan2(sinSigma, cosSigma);
    sinAlpha = cosU1 * cosU2 * sinLambda / sinSigma;
    cosSqAlpha = 1 - sinAlpha*sinAlpha;
    cos2SigmaM = cosSigma - 2*sinU1*sinU2/cosSqAlpha;
    if(cos2SigmaM!=cos2SigmaM){ // i.e.(cos2SigmaM == nan)||(cos2SigmaM == -nan)
      // equatorial line: cosSqAlpha=0 (§6)
      cos2SigmaM = 0;
      errno = 0;
    }
    C = f/16*cosSqAlpha*(4+f*(4-3*cosSqAlpha));
    lambdaP = lambda;
    lambda = L + (1-C) * f * sinAlpha *
      (   sigma
        + C*sinSigma*(cos2SigmaM+C*cosSigma*(-1+2*cos2SigmaM*cos2SigmaM)) );
    if(errno!=0){
      cout << __PRETTY_FUNCTION__ << ": Error (2) in iteration " <<iter<< endl;
      valid = false;
      return -666.666;
    }
  } while( (fabs(lambda-lambdaP) > epsilon) && (iter++<iterLimit) );
  if(iter>iterLimit) { // formula failed to converge
    cout << __PRETTY_FUNCTION__ << ": Formula failed to converge after "
         << iter << " iterations. Delta=" << fabs(lambda-lambdaP)
         << " > epsilon=" << epsilon << "." << endl;
    valid = false;
    return -666.666;
  }
  double uSq = cosSqAlpha * (a*a - b*b) / (b*b);
  double A = 1 + uSq/16384*(4096+uSq*(-768+uSq*(320-175*uSq)));
  double B = uSq/1024 * (256+uSq*(-128+uSq*(74-47*uSq)));
  double deltaSigma = B*sinSigma*(cos2SigmaM
      +(B/4)*(cosSigma*(-1+2*cos2SigmaM*cos2SigmaM)-
       (B/6)*cos2SigmaM*(-3+4*sinSigma*sinSigma)*(-3+4*cos2SigmaM*cos2SigmaM)));
  double s = b*A*(sigma-deltaSigma);
  if(errno!=0){
    cout << __PRETTY_FUNCTION__ << ": Error (3) " << iter << endl;
    valid = false;
    return -666.666;
  }

  // calculate initial/final bearings in addition to distance
  double fwdAz = atan2(cosU2*sinLambda,  cosU1*sinU2-sinU1*cosU2*cosLambda);
  double revAz = atan2(cosU1*sinLambda, -sinU1*cosU2+cosU1*sinU2*cosLambda);
  initialBearingDEG = radToDeg(fwdAz);
  finalBearingDEG   = radToDeg(revAz);
  // normalize heading results to 0<x<=360
  while( initialBearingDEG<0.0 ) {
    initialBearingDEG += 360.0;
  }
  if(initialBearingDEG > 360.0){
    initialBearingDEG = fmod(initialBearingDEG,360.0);
  }
  if( AlmostEqual(initialBearingDEG,0.0) ){
    initialBearingDEG = 360.0; // Map NORTH to heading 360, not to 0
  }

  while( finalBearingDEG<0.0 ){
    finalBearingDEG += 360.0;
  }
  if(finalBearingDEG > 360.0){
    finalBearingDEG = fmod(finalBearingDEG,360.0);
  }
  if( AlmostEqual(finalBearingDEG,0.0) ){
    finalBearingDEG = 360.0; // Map NORTH to heading 360, not to 0
  }

  if(errno!=0){
    cout << __PRETTY_FUNCTION__ << ": Error (4) " << iter << endl;
    valid = false;
    return -666.666;
  }
//   cout << __PRETTY_FUNCTION__ << ": After " << iter
//        << " Iterations: Distance[m]=" << s
//        << ", initialBearingDEG=" << initialBearingDEG
//        << ", finalBearingDEG=" << finalBearingDEG << endl;
  valid = true;
  return s;
}



Point Point::MidpointTo(const Point& p, const Geoid* geoid /* = 0 */ ) const
{
  if ( !this->IsDefined() || !p.IsDefined() ) {
    return Point( false );
  }

  if( AlmostEqual(*this,p) ){ // no calculation required.
    return *this;
  }
  if(geoid){ // spherical geometry
    // TODO: Use ellipsoid ~geoid~ instead of ideal sphere!
    bool ok = this->checkGeographicCoord() && p.checkGeographicCoord();
    double lon1 = degToRad( GetX() ); // X <-> geogr. Laenge (Longitude)
    double lat1 = degToRad( GetY() ); // Y <-> geogr. Breite (Latitude)
    double lat2 = degToRad( p.GetY() );
    double dLon = degToRad( p.GetX() - GetX() );
    errno = 0;
    double Bx = cos(lat2) * cos(dLon);
    double By = cos(lat2) * sin(dLon);
    double lat3 = atan2( sin(lat1)+sin(lat2),
                         sqrt( (cos(lat1)+Bx)*(cos(lat1)+Bx) + By*By) );
    double lon3 = lon1 + atan2(By, cos(lat1) + Bx);
    Point p( true, radToDeg(lon3), radToDeg(lat3) );
    p.SetDefined( ok && (errno==0) && p.checkGeographicCoord() );
    return p;
  } else { // euclidean geometry
    return Point( true, ( GetX() + p.GetX() )/2.0, ( GetY() + p.GetY() )/2.0 );
  }
}

Point Point::MidpointTo(const Point& p, const Coord& f,
                        const Geoid* geoid /*=0*/) const{
  if( !IsDefined() || !p.IsDefined() ||
      (geoid && !geoid->IsDefined()) || (f<0.0) || (f>1.0) ) {
  return Point(false);
  }
  if(AlmostEqual(f,0.0)) {
    return *this;
  }
  if(AlmostEqual(f,1.0)) {
    return p;
  }
  if(geoid){ // spherical TODO: use ellipsoid instead of ideal sphere
    double lon1 = degToRad(GetX()); // X <-> geogr. Laenge (Longitude)
    double lat1 = degToRad(GetY()); // Y <-> geogr. Breite (Latitude)
    double lon2 = degToRad(p.GetX());
    double lat2 = degToRad(p.GetY());
    if( AlmostEqual(lat1,lat2) && AlmostEqual(fabs(lon1-lon2),M_PI) ){
      return Point(false);
    }
    errno = 0;
    double A = sin(1 - f) / sin(1);
    double B = sin(f) / sin(1);
    double C = A * cos(lat1) * cos(lon1) + B * cos(lat2)*cos(lon2);
    double D = A * cos(lat1) * sin(lon1) + B * cos(lat2) * sin(lon2);
    double lat = atan2(A * sin(lat1) + B * sin(lat2),sqrt(C*C + D*D));
    double lon = atan2(D, C);
    Point p( true, radToDeg(lon), radToDeg(lat) );
    p.SetDefined( (errno==0) && p.checkGeographicCoord() );
    return p;
  } else { // euclidean
    double x1 = GetX();
    double y1 = GetY();
    double x2 = p.GetX();
    double y2 = p.GetY();
    double dx = x2-x1;
    double dy = y2-y1;
    double x = x1 + (f*dx);
    double y = y1 + (f*dy);
    return Point( true, x, y );
  }
}

/*
4.2 List Representation

The list representation of a point is

----  (x y)
----

4.3 ~Out~-function

*/
ListExpr
OutPoint( ListExpr typeInfo, Word value )
{
  Point* point = (Point*)(value.addr);
  if( point->IsDefined() )
    return nl->TwoElemList(
               nl->RealAtom( point->GetX() ),
               nl->RealAtom( point->GetY() ) );
  else
    return nl->SymbolAtom( Symbol::UNDEFINED() );
}

/*
4.4 ~In~-function

*/
Word
InPoint( const ListExpr typeInfo, const ListExpr instance,
       const int errorPos, ListExpr& errorInfo, bool& correct )
{
  correct = true;
  if( nl->ListLength( instance ) == 2 ) {
    ListExpr first = nl->First(instance);
    ListExpr second = nl->Second(instance);

    correct = listutils::isNumeric(first) && listutils::isNumeric(second);
    if(!correct){
       return SetWord( Address(0) );
    } else {
      return SetWord(new Point(true, listutils::getNumValue(first),
                                     listutils::getNumValue(second)));
    }
  } else if( listutils::isSymbolUndefined( instance ) ){
     return SetWord(new Point(false));
  }
  correct = false;
  return SetWord( Address(0) );
}

/*
4.5 ~Create~-function

*/
Word
CreatePoint( const ListExpr typeInfo )
{
  return SetWord( new Point( false ) );
}

/*
4.6 ~Delete~-function

*/
void
DeletePoint( const ListExpr typeInfo,
             Word& w )
{
  ((Point *)w.addr)->DeleteIfAllowed();
  w.addr = 0;
}

/*
4.7 ~Close~-function

*/
void
ClosePoint( const ListExpr typeInfo,
            Word& w )
{
  ((Point *)w.addr)->DeleteIfAllowed();
  w.addr = 0;
}

/*
4.8 ~Clone~-function

*/
Word
ClonePoint( const ListExpr typeInfo,
            const Word& w )
{
  return SetWord( new Point( *((Point *)w.addr) ) );
}

/*
4.8 ~SizeOf~-function

*/
int
SizeOfPoint()
{
  return sizeof(Point);
}

/*
4.9 Function describing the signature of the type constructor

*/
ListExpr
PointProperty()
{
  return nl->TwoElemList(
           nl->FourElemList(
             nl->StringAtom("Signature"),
             nl->StringAtom("Example Type List"),
             nl->StringAtom("List Rep"),
             nl->StringAtom("Example List")),
           nl->FourElemList(
             nl->StringAtom("-> DATA"),
             nl->StringAtom(Point::BasicType()),
             nl->StringAtom("(x y)"),
             nl->StringAtom("(10 5)")));
}

/*
4.10 Kind Checking Function

This function checks whether the type constructor is applied correctly. Since
type constructor ~point~ does not have arguments, this is trivial.

*/
bool
CheckPoint( ListExpr type, ListExpr& errorInfo )
{
  return (listutils::isSymbol( type, Point::BasicType() ));
}


/*
4.12 Creation of the type constructor instance

*/
TypeConstructor point(
  Point::BasicType(),                    //name
  PointProperty,              //property function describing signature
  OutPoint,      InPoint,     //Out and In functions
  0,             0,           //SaveToList and RestoreFromList functions
  CreatePoint,   DeletePoint, //object creation and deletion
  OpenAttribute<Point>,
  SaveAttribute<Point>,  // object open and save
  ClosePoint,    ClonePoint,  //object close, and clone
  Point::Cast,                  //cast function
  SizeOfPoint,                //sizeof function
  CheckPoint);               //kind checking function

ostream& operator<<( ostream& o, const Points& ps )
{
  o << "<";
  if( !ps.IsDefined() ) {
    o << " undef ";
  } else {
    for( int i = 0; i < ps.Size(); i++ )
    {
      Point p(true);
      ps.Get( i, p );
      o << " " << p;
    }
  }
  o << ">";
  return o;
}

// use this when adding and sorting the DBArray
int PointCompare( const void *a, const void *b )
{
  const Point *pa = (const Point*)a,
              *pb = (const Point*)b;
  assert(pa->IsDefined());
  assert(pb->IsDefined());
  if( *pa == *pb )
    return 0;

  if( *pa < *pb )
    return -1;

  return 1;
}

// use this when testing for containment or removing duplicates
// in the DBArray
int PointCompareAlmost( const void *a, const void *b )
{
  const Point *pa = (const Point*)a,
              *pb = (const Point*)b;

  assert(pa->IsDefined());
  assert(pb->IsDefined());
  if( AlmostEqual( *pa, *pb ) )
    return 0;

  if( *pa < *pb )
    return -1;

  return 1;
}

// Old implementation, should be replaced by the following one
// to avoid problems when sorting and removing duplicates
int PointHalfSegmentCompare( const void *a, const void *b )
{
  const Point *pa = (const Point *)a;
  const HalfSegment *hsb = (const HalfSegment *)b;
  assert(pa->IsDefined());

  if( AlmostEqual( *pa, hsb->GetDomPoint() ) )
    return 0;

  if( *pa < hsb->GetDomPoint() )
    return -1;

  return 1;
}

// // use this when adding and sorting the DBArray
// int PointHalfSegmentCompare( const void *a, const void *b )
// {
//   const Point *pa = (const Point *)a;
//   const HalfSegment *hsb = (const HalfSegment *)b;
//
//   if( *pa == hsb->GetDomPoint() )
//     return 0;
//
//   if( *pa < hsb->GetDomPoint() )
//     return -1;
//
//   return 1;
// }

// use this when testing for containment or removing duplicates
// in the DBArray
int PointHalfSegmentCompareAlmost( const void *a, const void *b )
{
  const Point *pa = (const Point *)a;
  const HalfSegment *hsb = (const HalfSegment *)b;
  assert(pa->IsDefined());

  if( AlmostEqual( *pa, hsb->GetDomPoint() ) )
    return 0;

  if( *pa < hsb->GetDomPoint() )
    return -1;

  return 1;
}


/*
5.2 List Representation

The list representation of a point is

----  (x y)
----

5.3 ~Out~-function

*/
ListExpr
OutPoints( ListExpr typeInfo, Word value )
{
  Points* points = (Points*)(value.addr);
  if(!points->IsDefined()){
    return nl->SymbolAtom(Symbol::UNDEFINED());
  }

  if( points->IsEmpty() )
    return nl->TheEmptyList();

  Point p(true);
  assert( points->Get( 0, p ) );
  ListExpr result =
    nl->OneElemList( OutPoint( nl->TheEmptyList(), SetWord( (void*)&p ) ) );
  ListExpr last = result;

  for( int i = 1; i < points->Size(); i++ )
  {
    assert( points->Get( i, p ) );
    last = nl->Append( last,
                       OutPoint( nl->TheEmptyList(), SetWord( (void*)&p ) ) );
  }
  return result;
}

/*
5.4 ~In~-function

*/
Word
InPoints( const ListExpr typeInfo, const ListExpr instance,
       const int errorPos, ListExpr& errorInfo, bool& correct )
{
  if(listutils::isSymbolUndefined(instance)) {
      Points* points = new Points(0);
      points->Clear();
      points->SetDefined(false);
      correct=true;
      return SetWord( Address(points) );
  }
  Points* points = new Points( max(0,nl->ListLength( instance) ) );
  points->SetDefined( true );
  if(nl->AtomType(instance)!=NoAtom) {
    points->DeleteIfAllowed();
    correct = false;
    cout << __PRETTY_FUNCTION__ << ": Unexpected Atom!" << endl;
    return SetWord( Address(points) );
  }

  ListExpr rest = instance;
  points->StartBulkLoad();
  while( !nl->IsEmpty( rest ) ) {
    ListExpr first = nl->First( rest );
    rest = nl->Rest( rest );

    Point *p = (Point*)InPoint( nl->TheEmptyList(),
                                first, 0, errorInfo, correct ).addr;
    if( correct && p->IsDefined() ) {
      (*points) += (*p);
      delete p;
    } else {
      if(p) {
        delete p;
      }
      cout << __PRETTY_FUNCTION__ << ": Incorrect or undefined point!" << endl;
      points->DeleteIfAllowed();
      correct = false;
      return SetWord( Address(0) );
    }

  }
  points->EndBulkLoad();

  if( points->IsValid() ) {
    correct = true;
    return SetWord( points );
  }
  points->DeleteIfAllowed();
  correct = false;
  cout << __PRETTY_FUNCTION__ << ": Invalid points value!" << endl;
  return SetWord( Address(0) );
}

/*
5.5 ~Create~-function

*/
Word
CreatePoints( const ListExpr typeInfo )
{
  return SetWord( new Points( 0 ) );
}

/*
5.6 ~Delete~-function

*/
void
DeletePoints( const ListExpr typeInfo, Word& w )
{
  Points *ps = (Points *)w.addr;
  ps->Destroy();
  ps->DeleteIfAllowed(false);
  w.addr = 0;
}

/*
5.7 ~Close~-function

*/
void
ClosePoints( const ListExpr typeInfo, Word& w )
{
  ((Points *)w.addr)->DeleteIfAllowed();
  w.addr = 0;
}

/*
5.8 ~Clone~-function

*/
Word
ClonePoints( const ListExpr typeInfo, const Word& w )
{
  return SetWord( new Points( *((Points *)w.addr) ) );
}

/*
7.8 ~Open~-function

*/
bool
OpenPoints( SmiRecord& valueRecord,
            size_t& offset,
            const ListExpr typeInfo,
            Word& value )
{
  Points *ps = (Points*)Attribute::Open( valueRecord, offset, typeInfo );
  value = SetWord( ps );
  return true;
}

/*
7.8 ~Save~-function

*/
bool
SavePoints( SmiRecord& valueRecord,
            size_t& offset,
            const ListExpr typeInfo,
            Word& value )
{
  Points *ps = (Points*)value.addr;
  Attribute::Save( valueRecord, offset, typeInfo, ps );
  return true;
}

/*
5.8 ~SizeOf~-function

*/
int
SizeOfPoints()
{
  return sizeof(Points);
}

/*
5.11 Function describing the signature of the type constructor

*/
ListExpr
PointsProperty()
{
  return (nl->TwoElemList(
            nl->FourElemList(nl->StringAtom("Signature"),
                       nl->StringAtom("Example Type List"),
           nl->StringAtom("List Rep"),
           nl->StringAtom("Example List")),
            nl->FourElemList(nl->StringAtom("-> DATA"),
                       nl->StringAtom(Points::BasicType()),
           nl->StringAtom("(<point>*) where point is (<x><y>)"),
           nl->StringAtom("( (10 1)(4 5) )"))));
}

/*
5.12 Kind checking function

This function checks whether the type constructor is applied correctly. Since
type constructor ~point~ does not have arguments, this is trivial.

*/
bool
CheckPoints( ListExpr type, ListExpr& errorInfo )
{
  return (nl->IsEqual( type, Points::BasicType() ));
}

/*
5.13 ~Cast~-function

*/
void* CastPoints(void* addr)
{
  return (new (addr) Points());
}

/*
5.14 Creation of the type constructor instance

*/
TypeConstructor points(
        Points::BasicType(),                     //name
        PointsProperty,               //property function describing signature
        OutPoints,      InPoints,     //Out and In functions
        0,              0,            //SaveTo and RestoreFrom List functions
        CreatePoints,   DeletePoints, //object creation and deletion
        OpenPoints,     SavePoints,   // object open and save
        ClosePoints,    ClonePoints,  //object close and clone
        CastPoints,                   //cast function
        SizeOfPoints,                 //sizeof function
        CheckPoints );                //kind checking function

/*
6 Type Constructor ~halfsegment~

A ~halfsegment~ value is a pair of points, with a boolean flag indicating the
dominating point .

6.1 Implementation of the class ~halfsegment~

*/
void HalfSegment::Translate( const Coord& x, const Coord& y )
{
   lp.Translate( x, y );
   rp.Translate( x, y );
}


void HalfSegment::Set( bool ldp, const Point& lp, const Point& rp )
{
  assert( lp.IsDefined() );
  assert( rp.IsDefined() );
  assert( !AlmostEqual( lp, rp ) );

  this->ldp = ldp;
  if( lp < rp )
  {
    this->lp = lp;
    this->rp = rp;
  }
  else // rp > lp
  {
    this->lp = rp;
    this->rp = lp;
  }
}

int HalfSegment::Compare( const HalfSegment& hs ) const
{
  const Point& dp = GetDomPoint(),
                   sp = GetSecPoint(),
                   DP = hs.GetDomPoint(),
                   SP = hs.GetSecPoint();

  if( dp < DP )
    return -1;
  else if( dp > DP )
    return 1;

  if( ldp != hs.ldp )
  {
    if( ldp == false )
      return -1;
    return 1;
  }
  else
  {
    bool v1 = IsVertical();
    bool v2 = hs.IsVertical();
    if( v1 && v2 ) // both are vertical
    {
      if(   (     (CompareDouble(sp.GetY(),dp.GetY())>0)
               && ( CompareDouble(SP.GetY(),DP.GetY())>0)
            )
          ||
            (     (CompareDouble(sp.GetY(),dp.GetY())<0)
               && (CompareDouble(SP.GetY(),DP.GetY())<0) ) )
      {
        if( sp < SP )
          return -1;
        if( sp > SP )
          return 1;
        return 0;
      }
      else if( CompareDouble(sp.GetY(),dp.GetY())>0)
      {
        if( ldp == true )
          return 1;
        return -1;
      }
      else
      {
        if( ldp == true )
          return -1;
        return 1;
      }
    }
    else if( AlmostEqual(dp.GetX(),sp.GetX()) )
    {
      if( CompareDouble(sp.GetY(), dp.GetY())>0 )
      {
        if( ldp == true )
          return 1;
        return -1;
      }
      else if( CompareDouble(sp.GetY(),dp.GetY())<0 )
      {
        if( ldp == true )
          return -1;
        return 1;
      }
    }
    else if( AlmostEqual(DP.GetX(), SP.GetX()) )
    {
      if( CompareDouble(SP.GetY() , DP.GetY())>0 )
      {
        if( ldp == true )
          return -1;
        return 1;
      }
      else if( CompareDouble(SP.GetY() , DP.GetY())<0 )
      {
        if( ldp == true )
          return 1;
        return -1;
      }
    }
    else
    {
      Coord xd = dp.GetX(), yd = dp.GetY(),
            xs = sp.GetX(), ys = sp.GetY(),
            Xd = DP.GetX(), Yd = DP.GetY(),
            Xs = SP.GetX(), Ys = SP.GetY();
      double k = (yd - ys) / (xd - xs),
             K= (Yd -Ys) / (Xd - Xs);

      if( CompareDouble(k , K) <0 )
        return -1;
      if( CompareDouble( k,  K) > 0)
        return 1;

      if( sp < SP )
        return -1;
      if( sp > SP )
        return 1;
      return 0;
    }
  }
  assert( true ); // This code should never be reached
  return 0;
}

HalfSegment& HalfSegment::operator=( const HalfSegment& hs )
{
  ldp = hs.ldp;
  lp = hs.lp;
  rp = hs.rp;
  attr = hs.attr;
  return *this;
}

bool HalfSegment::operator==( const HalfSegment& hs ) const
{
  return Compare(hs) == 0;
}

bool HalfSegment::operator!=( const HalfSegment& hs ) const
{
  return !( *this == hs );
}

bool HalfSegment::operator<( const HalfSegment& hs ) const
{
  return Compare(hs) == -1;
}

bool HalfSegment::operator>( const HalfSegment& hs ) const
{
  return Compare(hs) == 1;
}

int HalfSegment::LogicCompare( const HalfSegment& hs ) const
{
  if( attr.faceno < hs.attr.faceno )
    return -1;

  if( attr.faceno > hs.attr.faceno )
    return 1;

  if( attr.cycleno < hs.attr.cycleno )
    return -1;

  if( attr.cycleno > hs.attr.cycleno )
    return 1;

  if( attr.edgeno < hs.attr.edgeno )
    return -1;

  if( attr.edgeno > hs.attr.edgeno )
    return 1;

  return 0;
}


int HalfSegmentCompare(const void *a, const void *b)
{
  const HalfSegment *hsa = (const HalfSegment *)a,
                    *hsb = (const HalfSegment *)b;
  return hsa->Compare( *hsb );
}

int HalfSegmentLogicCompare(const void *a, const void *b)
{
  const HalfSegment *hsa = (const HalfSegment *)a,
                    *hsb = (const HalfSegment *)b;

  return hsa->LogicCompare( *hsb );
}

int LRSCompare( const void *a, const void *b )
{
  const LRS *lrsa = (const LRS *)a,
            *lrsb = (const LRS *)b;

  if( lrsa->lrsPos < lrsb->lrsPos )
    return -1;
  if( lrsa->lrsPos > lrsb->lrsPos )
    return 1;

  return 0;
}


bool HalfSegment::insideLeft() const{

    if(ldp){
      return attr.insideAbove;
    } else {
      return !attr.insideAbove;
    }
}



ostream& operator<<(ostream &os, const HalfSegment& hs)
{
  return os << "("
             <<"F("<< hs.attr.faceno
             <<") C("<<  hs.attr.cycleno
             <<") E(" << hs.attr.edgeno<<") DP("
             <<  (hs.IsLeftDomPoint()? "L":"R")
             <<") IA("<< (hs.attr.insideAbove? "A":"U")
             <<") Co("<<hs.attr.coverageno
             <<") PNo("<<hs.attr.partnerno
             <<") ("<< hs.GetLeftPoint() << " "<< hs.GetRightPoint() <<") ";
}

bool HalfSegment::Intersects( const HalfSegment& hs,
                              const Geoid* geoid /* = 0 */) const
{
  assert(!geoid); // spherical geometry not yet implemented!

  double k, a, K, A;
  if( !BoundingBox().Intersects( hs.BoundingBox() ) )
    return false;

  Coord xl = lp.GetX(),
        yl = lp.GetY(),
        xr = rp.GetX(),
        yr = rp.GetY(),
        Xl = hs.GetLeftPoint().GetX(),
        Yl = hs.GetLeftPoint().GetY(),
        Xr = hs.GetRightPoint().GetX(),
        Yr = hs.GetRightPoint().GetY();

  if( AlmostEqual( xl, xr ) &&
      AlmostEqual( Xl, Xr ) )
    // both segments are vertical
  {
    if( AlmostEqual( xl, Xl ) &&
        ( AlmostEqual( yl, Yl ) || AlmostEqual( yl, Yr ) ||
          AlmostEqual( yr, Yl ) || AlmostEqual( yr, Yr ) ||
          ( yl > Yl && yl < Yr ) || ( yr > Yl && yr < Yr ) ||
          ( Yl > yl && Yl < yr ) || ( Yr > yl && Yr < yr ) ) )
      return true;
    return false;
  }

  if( !AlmostEqual( xl, xr ) )
    // this segment is not vertical
  {
    k = (yr - yl) / (xr - xl);
    a = yl - k * xl;
  }


  if( !AlmostEqual( Xl, Xr ) )
    // hs is not vertical
  {
    K = (Yr - Yl) / (Xr - Xl);
    A = Yl - K * Xl;
  }

  if( AlmostEqual( Xl, Xr ) )
    //only hs is vertical
  {
    Coord y0 = k * Xl + a;

    if( ( Xl > xl || AlmostEqual( Xl, xl ) ) &&
        ( Xl < xr || AlmostEqual( Xl, xr ) ) )
    {
      if( ( ( y0 > Yl || AlmostEqual( y0, Yl ) ) &&
            ( y0 < Yr || AlmostEqual( y0, Yr ) ) ) ||
          ( ( y0 > Yr || AlmostEqual( y0, Yr ) ) &&
            ( y0 < Yl || AlmostEqual( y0, Yl ) ) ) )
        // (Xl, y0) is the intersection point
        return true;
    }
    return false;
  }

  if( AlmostEqual( xl, xr ) )
    // only this segment is vertical
  {
    Coord Y0 = K * xl + A;

    if( ( xl > Xl || AlmostEqual( xl, Xl ) ) &&
        ( xl < Xr || AlmostEqual( xl, Xr ) ) )
    {
      if( ( ( Y0 > yl || AlmostEqual( Y0, yl ) ) &&
            ( Y0 < yr || AlmostEqual( Y0, yr ) ) ) ||
          ( ( Y0 > yr || AlmostEqual( Y0, yr ) ) &&
            ( Y0 < yl || AlmostEqual( Y0, yl ) ) ) )
        // (xl, Y0) is the intersection point
        return true;
    }
    return false;
  }

  // both segments are non-vertical

  if( AlmostEqual( k, K ) )
    // both segments have the same inclination
  {
/*    if( ( AlmostEqual( A, a ) &&
        (( ( xl > Xl || AlmostEqual( xl, Xl ) ) &&
          ( xl < Xr || AlmostEqual( xl, Xr ) ) ))) ||
        ( ( Xl > xl || AlmostEqual( xl, Xl ) ) &&
          ( Xl < xr || AlmostEqual( xr, Xl ) ) ) )
      // the segments are in the same straight line
      return true;*/
    if( AlmostEqual( A, a ) &&
        (( ( xl > Xl || AlmostEqual( xl, Xl ) ) &&
          ( xl < Xr || AlmostEqual( xl, Xr ) ) ) ||
        ( ( Xl > xl || AlmostEqual( xl, Xl ) ) &&
          ( Xl < xr || AlmostEqual( xr, Xl ) ) ) ))
      // the segments are in the same straight line
      return true;

  }
  else
  {
    Coord x0 = (A - a) / (k - K);
    // y0 = x0 * k + a;

    if( ( x0 > xl || AlmostEqual( x0, xl ) ) &&
        ( x0 < xr || AlmostEqual( x0, xr ) ) &&
        ( x0 > Xl || AlmostEqual( x0, Xl ) ) &&
        ( x0 < Xr || AlmostEqual( x0, Xr ) ) )
      // the segments intersect at (x0, y0)
      return true;
  }
  return false;
}

bool HalfSegment::InnerIntersects( const HalfSegment& hs,
                                   const Geoid* geoid/*=0*/ ) const
{
  if(geoid){
    cerr << __PRETTY_FUNCTION__ << ": Spherical geometry not implemented."
         << endl;
    assert( false ); // TODO: implement spherical geometry case
  }
  double k = 0.0;
  double a = 0.0;
  double K = 0.0;
  double A = 0.0;
  Coord x0; //, y0;  (x0, y0) is the intersection

  Coord xl = lp.GetX(), yl = lp.GetY(),
        xr = rp.GetX(), yr = rp.GetY();

  if( xl != xr )
  {
    k = (yr - yl) / (xr - xl);
    a = yl - k * xl;
  }

  Coord Xl = hs.GetLeftPoint().GetX(),
        Yl = hs.GetLeftPoint().GetY(),
        Xr = hs.GetRightPoint().GetX(),
        Yr = hs.GetRightPoint().GetY();

  if( Xl != Xr )
  {
    K = (Yr - Yl) / (Xr - Xl);
    A = Yl - K * Xl;
  }

  if( xl == xr && Xl == Xr ) //both l and L are vertical lines
  {
    if( xl != Xl )
      return false;

    Coord ylow, yup, Ylow, Yup;
    if( yl < yr )
    {
      ylow = yl;
      yup = yr;
    }
    else
    {
      ylow = yr;
      yup = yl;
    }

    if( Yl < Yr)
    {
      Ylow = Yl;
      Yup = Yr;
    }
    else
    {
      Ylow = Yr;
      Yup = Yl;
    }

    if( ylow >= Yup || yup <= Ylow )
      return false;
    return true;
  }

  if( Xl == Xr )    //only L is vertical
  {
    double y0 = k * Xl + a;
    Coord yy = y0;

    //(Xl, y0) is the intersection of l and L
    if( Xl >= xl && Xl <= xr )
    {
      if( ( yy > Yl && yy < Yr ) ||
          ( yy > Yr && yy < Yl ) )
        return true;
      return false;
    }
    else return false;
  }

  if( xl == xr )    //only l is vertical
  {
    double Y0 = K * xl + A;
    Coord YY = Y0;

    //(xl, Y0) is the intersection of l and L
    if( xl > Xl && xl < Xr )
    {
      if( ( YY >= yl && YY <= yr ) ||
          ( YY >= yr && YY <= yl ) )
        return true;
      return false;
    }
    else return false;
  }

  //otherwise: both segments are non-vertical
  if( k == K )
  {
    if( A != a ) //Parallel lines
      return false;

    //they are in the same straight line
    if( xr <= Xl || xl >= Xr )
      return false;
    return true;
  }
  else
  {
    x0 = (A - a) / (k - K);  // y0=x0*k+a;
    Coord xx = x0;
    if( xx >= xl && xx <= xr && xx > Xl && xx < Xr )
      return true;
    return false;
  }
}


// corrected modulo function ( '%' has problems with negative values)
double fmod2(const double &y, const double &x) {
  return y - x*floor(y/x);
}

bool HalfSegment::Intersection( const HalfSegment& hs, Point& resp,
                                const Geoid* geoid /* = 0 */ ) const
{
  if(geoid){ // spherical geometry not yet implemented!
    assert(false);
    if(!geoid->IsDefined()){
      resp.SetDefined(false);
    }
    // see http://williams.best.vwh.net/avform.htm#Intersection
    Point res(false);//Destination point (UNDEFINED if no unique intersection)
    Point p1    = this->GetLeftPoint();  //1st segment's starting point
    Point p1end = this->GetRightPoint(); //1st segment's ending point
    Point p2    = hs.GetLeftPoint();     //2nd segment' s starting point
    Point p2end = hs.GetRightPoint();    //2nd segment's ending point
    if(!p1.checkGeographicCoord()   || !p2.checkGeographicCoord() ||
      !p1end.checkGeographicCoord() || !p2end.checkGeographicCoord() ) {
      return false;
    }
    double lon1 = degToRad(p1.GetX());
    double lat1 = degToRad(p1.GetY());
    double lon2 = degToRad(p2.GetX());
    double lat2 = degToRad(p2.GetY());

    double brng1 = p1.Direction(p1end,false,geoid);  //Initial bearing from p1
    double brng2 = p2.Direction(p2end,false,geoid);  //Initial bearing from p2
    if( (brng1<0) || (brng2<0) ){
      return false;
    }
    double brng13 = degToRad(brng1);
    double brng23 = degToRad(brng2);

    double dLat = lat2-lat1;
    double dLon = lon2-lon1;
    errno = 0;
    double dist12 = 2*asin( sqrt( sin(dLat/2)*sin(dLat/2) +
        cos(lat1)*cos(lat2)*sin(dLon/2)*sin(dLon/2) ) );
    if( (errno !=0) || (dist12 <= 0) ){
      return false;
    }
    // initial/final bearings between points
    errno = 0;
    double brngA = acos( ( sin(lat2) - sin(lat1)*cos(dist12) ) /
                   ( sin(dist12)*cos(lat1) ) );
    if(errno != 0){
      brngA = 0;  // protect against rounding
      errno = 0;
    }
    double brngB = acos( ( sin(lat1) - sin(lat2)*cos(dist12) ) /
                   ( sin(dist12)*cos(lat2) ) );
    double brng12 = 0;
    double brng21 = 0;
    if (sin(lon2-lon1) > 0) {
        brng12 = brngA;
        brng21 = 2*M_PI - brngB;
    } else {
        brng12 = 2*M_PI - brngA;
        brng21 = brngB;
    }

    double alpha1 = fmod2((brng13 - brng12 +M_PI),(2*M_PI)) - M_PI;//angle 2-1-3
    double alpha2 = fmod2((brng21 - brng23 +M_PI),(2*M_PI)) - M_PI;//angle 1-2-3

    if( (sin(alpha1)==0) && (sin(alpha2)==0) ){// infinite intersections
      return false;
    }
    if( (errno != 0) || (sin(alpha1)*sin(alpha2) < 0) ){//ambiguous intersection
      return false;
    }
    //Ed Williams takes abs of alpha1/alpha2, but seems to break calculation?
    //alpha1 = fabs(alpha1);
    //alpha2 = fabs(alpha2);

    double alpha3 = acos( -cos(alpha1)*cos(alpha2) +
      sin(alpha1)*sin(alpha2)*cos(dist12) );
    double dist13 = atan2( sin(dist12)*sin(alpha1)*sin(alpha2),
                           cos(alpha2)+cos(alpha1)*cos(alpha3) );
    double lat3 = asin( sin(lat1)*cos(dist13) +
                           cos(lat1)*sin(dist13)*cos(brng13) );
    double dLon13 = atan2( sin(brng13)*sin(dist13)*cos(lat1),
                           cos(dist13)-sin(lat1)*sin(lat3) );
    double lon3 = lon1+dLon13;
    lon3 = fmod2((lon3+M_PI),(2*M_PI)) - M_PI;  // normalise to -180..180º
    res.SetDefined( (errno == 0) );
    res.Set( radToDeg(lon3), radToDeg(lat3) );
  } else { // euclidean geometry
    double k = 0.0;
    double a = 0.0;
    double K = 0.0;
    double A = 0.0;

    if(!BoundingBox().Intersects(hs.BoundingBox())){
      resp.SetDefined(false);
      return false;
    }

    resp.SetDefined( true );

    Coord xl = lp.GetX(),
    yl = lp.GetY(),
    xr = rp.GetX(),
    yr = rp.GetY(),
    Xl = hs.GetLeftPoint().GetX(),
    Yl = hs.GetLeftPoint().GetY(),
    Xr = hs.GetRightPoint().GetX(),
    Yr = hs.GetRightPoint().GetY();

    // Check for same endpoints
    if (AlmostEqual(lp,hs.GetLeftPoint())){
      if (!hs.Contains(rp) && !this->Contains(hs.GetRightPoint())){
        resp = lp;
        return true;
      } else {
        return false; //overlapping halfsegments
      }
    } else if (AlmostEqual(rp,hs.GetLeftPoint())){
      if (!hs.Contains(lp) && !this->Contains(hs.GetRightPoint())){
        resp = rp;
        return true;
      } else {
        return false; //overlapping halfsegments
      }
    } else if (AlmostEqual(lp,hs.GetRightPoint())){
      if (!hs.Contains(rp) && !this->Contains(hs.GetLeftPoint())){
        resp = lp;
        return true;
      } else {
        return false; //overlapping halfsegments
      }
    } else if (AlmostEqual(rp,hs.GetRightPoint())){
        if (!hs.Contains(lp) && !this->Contains(hs.GetLeftPoint())){
          resp = rp;
          return true;
        } else {
          return false; //overlapping halfsegments
        }
    }

    if( AlmostEqual( xl, xr ) &&
      AlmostEqual( Xl, Xr ) ){
      // both segments are vertical
      if( AlmostEqual( yr, Yl ) ){
        resp.Set( xl, yr );
        return true;
      }
      if( AlmostEqual( yl, Yr ) ) {
        resp.Set( xl, yl );
        return true;
      }
      return false;
    }

    if( !AlmostEqual( xl, xr ) ) { // this segment is not vertical
      k = (yr - yl) / (xr - xl);
      a = yl - k * xl;
    }

    if( !AlmostEqual( Xl, Xr ) ){ // hs is not vertical
      K = (Yr - Yl) / (Xr - Xl);
      A = Yl - K * Xl;
    }

    if( AlmostEqual( Xl, Xr ) ) {//only hs is vertical
      Coord y0 = k * Xl + a;

      if( ( Xl > xl || AlmostEqual( Xl, xl ) ) &&
        ( Xl < xr || AlmostEqual( Xl, xr ) ) ){
        if( ( ( y0 > Yl || AlmostEqual( y0, Yl ) ) &&
          ( y0 < Yr || AlmostEqual( y0, Yr ) ) ) ||
          ( ( y0 > Yr || AlmostEqual( y0, Yr ) ) &&
          ( y0 < Yl || AlmostEqual( y0, Yl ) ) ) ){
          // (Xl, y0) is the intersection point
          resp.Set( Xl, y0 );
        return true;
        }
      }
      return false;
    }

    if( AlmostEqual( xl, xr ) ){
      // only this segment is vertical
      Coord Y0 = K * xl + A;

      if( ( xl > Xl || AlmostEqual( xl, Xl ) ) &&
        ( xl < Xr || AlmostEqual( xl, Xr ) ) ){
        if( ( ( Y0 > yl || AlmostEqual( Y0, yl ) ) &&
          ( Y0 < yr || AlmostEqual( Y0, yr ) ) ) ||
          ( ( Y0 > yr || AlmostEqual( Y0, yr ) ) &&
          ( Y0 < yl || AlmostEqual( Y0, yl ) ) ) ){
          // (xl, Y0) is the intersection point
          resp.Set( xl, Y0 );
        return true;
        }
      }
      return false;
    }

    // both segments are non-vertical

    if( AlmostEqual( k, K ) ){
      // both segments have the same inclination
      if( AlmostEqual( rp, hs.lp ) ){
        resp = rp;
        return true;
      }
      if( AlmostEqual( lp, hs.rp ) ){
        resp = lp;
        return true;
      }
      return false;
    }

    Coord x0 = (A - a) / (k - K);
    Coord y0 = x0 * k + a;

    if( ( x0 > xl || AlmostEqual( x0, xl ) ) &&
      ( x0 < xr || AlmostEqual( x0, xr ) ) &&
      ( x0 > Xl || AlmostEqual( x0, Xl ) ) &&
      ( x0 < Xr || AlmostEqual( x0, Xr ) ) ){
      // the segments intersect at (x0, y0)
      resp.Set( x0, y0 );
    return true;
    }

    return false;
  }
}


bool HalfSegment::Intersection( const HalfSegment& hs,
                                HalfSegment& reshs,
                                const Geoid* geoid /* = 0 */) const
{
  if(geoid) {
    Point p1s = GetLeftPoint();
    Point p1e = GetRightPoint();
    Point p2s = hs.GetLeftPoint();
    Point p2e = hs.GetRightPoint();
    bool ok = p1s.checkGeographicCoord() && p1e.checkGeographicCoord() &&
              p2s.checkGeographicCoord() && p2e.checkGeographicCoord() &&
              AlmostEqual(Distance( hs, geoid ),0.0);
    if(!ok){ // error OR no intersection (distance >=0)
      return false;
    }
    Point intersectpoint(false);
    if( Intersection( hs, intersectpoint, geoid) && intersectpoint.IsDefined()){
      return false; // intersection is a point;
    };
    if( (p1s.GetX() > 0) && (p1e.GetX() <0) ) { //crosses the +/-180° meridean!
      Point tmp(p1s);
      p1s=p1e;
      p1e=tmp; // swap start/endpoint for *this
    }
    if( (p2s.GetX() >0) && (p2e.GetX() <0) ) { // crosses the +/-180° meridean!
      Point tmp(p2s);
      p2s=p2e;
      p2e=tmp; // swap start/endpoint for hs
    }
    // no special case: crossing the equator (happens once at most!)

    Point leftborder(true);
    if(AlmostEqual(hs.Distance(p1s, geoid), 0.0))     {// LeftPoint on hs?
      leftborder = p1s;
    } else if(AlmostEqual(Distance(p2s, geoid), 0.0)) {// hs.LeftPoint on *this?
      leftborder = p2s;
    } else {
      return false;
    }
    Point rightborder(true);
    if(AlmostEqual(hs.Distance(p1e, geoid), 0.0))    {// RightPoint on hs?
      leftborder = p1e;
    } else if(AlmostEqual(Distance(p2e, geoid), 0.0)){// hs.RightPoint on *this?
      leftborder = p2e;
    } else {
      return false;
    }
    reshs.Set(true, leftborder, rightborder);
    return true;
  } // else: euclidean geometry
  double k, a, K, A;

  if( !BoundingBox().Intersects( hs.BoundingBox() ) )
    return false;

  if( AlmostEqual( *this, hs ) )
  {
    reshs = hs;
    return true;
  }

  Coord xl = lp.GetX(),
        yl = lp.GetY(),
        xr = rp.GetX(),
        yr = rp.GetY(),
        Xl = hs.GetLeftPoint().GetX(),
        Yl = hs.GetLeftPoint().GetY(),
        Xr = hs.GetRightPoint().GetX(),
        Yr = hs.GetRightPoint().GetY();

  if( AlmostEqual( xl, xr ) &&
      AlmostEqual( Xl, Xr ) )
    //both l and L are vertical lines
  {
    Coord ylow, yup, Ylow, Yup;
    if( yl < yr )
    {
      ylow = yl;
      yup = yr;
    }
    else
    {
      ylow = yr;
      yup = yl;
    }

    if( Yl < Yr )
    {
      Ylow = Yl;
      Yup = Yr;
    }
    else
    {
      Ylow = Yr;
      Yup = Yl;
    }

    if( yup > Ylow && ylow < Yup &&
        !AlmostEqual( yup, Ylow ) &&
        !AlmostEqual( ylow, Yup ) )
    {
      Point p1(true), p2(true);
      if( ylow > Ylow )
        p1.Set( xl, ylow );
      else
        p1.Set( xl, Ylow );

      if( yup < Yup )
        p2.Set( xl, yup );
      else
        p2.Set( xl, Yup );

      reshs.Set( true, p1, p2 );
      return true;
    }
    else return false;
  }

  if( AlmostEqual( Xl, Xr ) ||
      AlmostEqual( xl, xr ) )
    //only L or l is vertical
    return false;

  //otherwise: both *this and *arg are non-vertical lines

  k = (yr - yl) / (xr - xl);
  a = yl - k * xl;

  K = (Yr - Yl) / (Xr - Xl);
  A = Yl - K * Xl;

  if( AlmostEqual( k, K ) &&
      AlmostEqual( A, a ) )
  {
    if( xr > Xl && xl < Xr &&
        !AlmostEqual( xr, Xl ) &&
        !AlmostEqual( xl, Xr ) )
    {
      Point p1(true), p2(true);
      if( xl > Xl )
        p1.Set( xl, yl );
      else
        p1.Set( Xl, Yl );

      if( xr < Xr )
        p2.Set( xr, yr );
      else
        p2.Set( Xr, Yr );

      reshs.Set( true, p1, p2 );
      return true;
    }
  }

  return false;
}

bool HalfSegment::Crosses( const HalfSegment& hs,
                           const Geoid* geoid/*=0*/ ) const
{
  if(geoid){
    cerr << __PRETTY_FUNCTION__ << ": Spherial geometry not implemented."
         << endl;
    assert( false ); // TODO: implement spherical case.
  }
  double k = 0.0;
  double a = 0.0;
  double K = 0.0;
  double A = 0.0;

  if( !BoundingBox().Intersects( hs.BoundingBox(),geoid ) ){
    return false;
  }
  Coord xl = lp.GetX(),
        yl = lp.GetY(),
        xr = rp.GetX(),
        yr = rp.GetY(),
        Xl = hs.GetLeftPoint().GetX(),
        Yl = hs.GetLeftPoint().GetY(),
        Xr = hs.GetRightPoint().GetX(),
        Yr = hs.GetRightPoint().GetY();

  if( AlmostEqual( xl, xr ) &&
      AlmostEqual( Xl, Xr ) )
    // both segments are vertical
    return false;

  if( !AlmostEqual( xl, xr ) )
    // this segment is not vertical
  {
    k = (yr - yl) / (xr - xl);
    a = yl - k * xl;
  }

  if( !AlmostEqual( Xl, Xr ) )
    // hs is not vertical
  {
    K = (Yr - Yl) / (Xr - Xl);
    A = Yl - K * Xl;
  }

  if( AlmostEqual( Xl, Xr ) )
    //only hs is vertical
  {
    double y0 = k * Xl + a;

    if( Xl > xl && !AlmostEqual( Xl, xl ) &&
        Xl < xr && !AlmostEqual( Xl, xr ) )
    {
      if( ( y0 > Yl && !AlmostEqual( y0, Yl ) &&
            y0 < Yr && !AlmostEqual( y0, Yr ) ) ||
          ( y0 > Yr && !AlmostEqual( y0, Yr ) &&
            y0 < Yl && !AlmostEqual( y0, Yl ) ) )
        // (Xl, y0) is the intersection point
        return true;
    }
    return false;
  }

  if( AlmostEqual( xl, xr ) )
    // only this segment is vertical
  {
    double Y0 = K * xl + A;

    if( ( xl > Xl && !AlmostEqual( xl, Xl ) ) &&
        ( xl < Xr && !AlmostEqual( xl, Xr ) ) )
    {
      if( ( Y0 > yl && !AlmostEqual( Y0, yl ) &&
            Y0 < yr && !AlmostEqual( Y0, yr ) ) ||
          ( Y0 > yr && !AlmostEqual( Y0, yr ) &&
            Y0 < yl && !AlmostEqual( Y0, yl ) ) )
        // (xl, Y0) is the intersection point
        return true;
    }
    return false;
  }

  // both segments are non-vertical

  if( AlmostEqual( k, K ) )
    // both segments have the same inclination
    return false;

  double x0 = (A - a) / (k - K);
  // y0 = x0 * k + a;

  if( x0 > xl && !AlmostEqual( x0, xl ) &&
      x0 < xr && !AlmostEqual( x0, xr ) &&
      x0 > Xl && !AlmostEqual( x0, Xl ) &&
      x0 < Xr && !AlmostEqual( x0, Xr ) )
    // the segments intersect at (x0, y0)
    return true;

  return false;
}

bool HalfSegment::Inside( const HalfSegment& hs,
                          const Geoid* geoid/*=0*/ ) const
{
  return hs.Contains( GetLeftPoint(),geoid ) &&
         hs.Contains( GetRightPoint(),geoid );
}

bool HalfSegment::Contains( const Point& p, const Geoid* geoid/*=0*/ ) const{
  if( !p.IsDefined() || (geoid && !geoid->IsDefined()) ) {
    assert( p.IsDefined() );
    return false;
  }
  if(geoid){
    cerr << __PRETTY_FUNCTION__ << ": Spherical geometry not implemented."
    << endl;
    assert(!geoid);
  }
  if( AlmostEqual( p, lp ) ||
      AlmostEqual( p, rp ) ){
    return true;
  }
  Coord xl = lp.GetX(), yl = lp.GetY(),
        xr = rp.GetX(), yr = rp.GetY(),
        x = p.GetX(), y = p.GetY();

  if( xl != xr && xl != x )
    // the segment is not vertical
  {
    double k1 = (y - yl) / (x - xl),
           k2 = (yr - yl) / (xr - xl);

    if( AlmostEqual( k1, k2 ) )
    {
      if( ( x > xl || AlmostEqual( x, xl ) ) &&
          ( x < xr || AlmostEqual( x, xr ) ) )
        // we check only this possibility because lp < rp and
        // therefore, in this case, xl < xr
        return true;
    }
  }
  else if( AlmostEqual( xl, xr ) &&
           AlmostEqual( xl, x ) )
    // the segment is vertical and the point is also in the
    // same x-position. In this case we just have to check
    // whether the point is inside the y-interval
  {
    if( (( y > yl || AlmostEqual( y, yl ) ) &&
        ( y < yr || AlmostEqual( y, yr ) )) ||
        (( y < yl || AlmostEqual( y, yl ) ) &&
        ( y > yr || AlmostEqual( y, yr ) ) ))
      // Here we check both possibilities because we do not
      // know wheter yl < yr, given that we used the
      // AlmostEqual function in the previous condition
      return true;
  }

  return false;
}

double HalfSegment::Distance( const Point& p,
                              const Geoid* geoid /* = 0 */) const
{
  assert( p.IsDefined() );
  assert( !geoid || geoid->IsDefined() );
  if(!geoid){ // euclidean geometry
    Coord xl = GetLeftPoint().GetX(),
    yl = GetLeftPoint().GetY(),
    xr = GetRightPoint().GetX(),
    yr = GetRightPoint().GetY(),
    X = p.GetX(),
    Y = p.GetY();

    double result, auxresult;

    if( xl == xr || yl == yr )
    {
      if( xl == xr) //hs is vertical
      {
        if( (yl <= Y && Y <= yr) || (yr <= Y && Y <= yl) )
          result = fabs( X - xl );
        else
        {
          result = p.Distance( GetLeftPoint() );
          auxresult = p.Distance( GetRightPoint() );
          if( result > auxresult )
            result = auxresult;
        }
      }
      else         //hs is horizontal line: (yl==yr)
      {
        if( xl <= X && X <= xr )
          result = fabs( Y - yl );
        else
        {
          result = p.Distance( GetLeftPoint() );
          auxresult = p.Distance( GetRightPoint() );
          if( result > auxresult )
            result = auxresult;
        }
      }
    }
    else
    {
      double k = (yr - yl) / (xr - xl),
      a = yl - k * xl,
      xx = (k * (Y - a) + X) / (k * k + 1),
      yy = k * xx + a;
      Coord XX = xx,
      YY = yy;
      Point PP( true, XX, YY );
      if( xl <= XX && XX <= xr )
        result = p.Distance( PP );
      else
      {
        result = p.Distance( GetLeftPoint() );
        auxresult = p.Distance( GetRightPoint() );
        if( result > auxresult )
          result = auxresult;
      }
    }
    return result;
  }

  return geodist::getDist(*this, p, geoid);

  /*
  // else: spherical geometry
  bool ok = true;
  double d13 = p.DistanceOrthodrome(GetLeftPoint(),*geoid,ok);
  // initial bearing from LeftPoint to p
  double theta13 = GetLeftPoint().Direction(p,false,geoid); // in deg
  // initial bearing from LeftPoint to RightPoint
  double theta12 = GetLeftPoint().Direction(GetRightPoint(),false,geoid);//[deg]
  double R = geoid->getR();
  errno = 0;
  double res = asin(sin(d13/R)*sin((theta13-theta12)*M_PI/180.0))*R;
  ok = ok && (errno==0) && (theta13>=0.0) && (theta12>=0.0);
  if(ok){
    return res;
  } // else: error
  return -666.666;
  */
}

double HalfSegment::Distance( const HalfSegment& hs,
                              const Geoid* geoid /* = 0 */) const
{
  assert( !geoid || geoid->IsDefined() );
  if(!geoid){ // euclidean geometry
    if( Intersects( hs ) ){
      return 0.0;
    }
    double d1 = MIN( Distance( hs.GetLeftPoint(), geoid ),
                     Distance( hs.GetRightPoint(), geoid ) );

    double d2 = MIN( hs.Distance(this->GetLeftPoint(), geoid),
                     hs.Distance(this->GetRightPoint(), geoid));
    return MIN(d1,d2);
  } // else: spherical geometry
  cout << __PRETTY_FUNCTION__ << ": Spherical geometry not implemented." <<endl;
  assert(false); // TODO: Implement spherical geometry case.
}


double HalfSegment::Distance(const Rectangle<2>& rect,
                             const Geoid* geoid/*=0*/) const{

  assert( rect.IsDefined() );
  assert( !geoid || geoid->IsDefined() );

  if(geoid){
    cout << __PRETTY_FUNCTION__ << ": Spherical geometry not implemented."
        <<endl;
    assert(false); // TODO: Implement spherical geometry case.
  }
  if(rect.Contains(lp.BoundingBox()) ||
     rect.Contains(rp.BoundingBox()) ){
    return 0.0;
  }
  // both endpoints are outside the rectangle
  double x0(rect.MinD(0));
  double y0(rect.MinD(1));
  double x1(rect.MaxD(0));
  double y1(rect.MaxD(1));
  Point p0(true,x0,y0);
  Point p1(true,x1,y0);
  Point p2(true,x1,y1);
  Point p3(true,x0,y1);

  double dist;
  HalfSegment hs;
  if(AlmostEqual(p0,p1)){
    dist = this->Distance(p0,geoid);
  } else {
    hs.Set(true,p0,p1);
    dist = this->Distance(hs,geoid);
  }
  if(AlmostEqual(dist,0)){
    return 0.0;
  }
  if(AlmostEqual(p1,p2)){
    dist = MIN( dist, this->Distance(p1,geoid));
  } else {
    hs.Set(true,p1,p2);
    dist = MIN( dist, this->Distance(hs,geoid));
  }
  if(AlmostEqual(dist,0)){
    return 0.0;
  }

  if(AlmostEqual(p2,p3)){
    dist = MIN(dist, this->Distance(p2,geoid));
  } else {
    hs.Set(true,p2,p3);
    dist = MIN( dist, this->Distance(hs,geoid));
  }
  if(AlmostEqual(dist,0)){
    return 0.0;
  }
  if(AlmostEqual(p3,p0)){
    dist = MIN(dist, this->Distance(p3,geoid));
  } else {
    hs.Set(true,p3,p0);
    dist = MIN( dist, this->Distance(hs,geoid));
  }
  if(AlmostEqual(dist,0)){
    return 0.0;
  }
  return dist;
}



bool HalfSegment::Intersects(const Rectangle<2>& rect,
                             const Geoid* geoid/*=0*/) const{
  if(!rect.IsDefined()){
     return false;
  }
  if(rect.IsEmpty()){
     return false;
  }
  if(lp.Intersects(rect,geoid)) return true;
  if(rp.Intersects(rect,geoid)) return true;
  // check for intersection of the 4   
  // segments of the rectangle
  Point p1(true, rect.MinD(0), rect.MinD(1));
  Point p2(true, rect.MaxD(0), rect.MinD(1));
  Point p3(true, rect.MaxD(0), rect.MaxD(1));
  Point p4(true, rect.MinD(0), rect.MaxD(1));

  HalfSegment hs1(true,p1,p2);
  if(Intersects(hs1)) return true;
  HalfSegment hs2(true,p2,p3);
  if(Intersects(hs2)) return true;
  HalfSegment hs3(true,p3,p4);
  if(Intersects(hs3)) return true;
  HalfSegment hs4(true,p4,p1);
  if(Intersects(hs4)) return true;
  return false;

}


double HalfSegment::MaxDistance(const Rectangle<2>& rect,
                                const Geoid* geoid /*=0*/) const{

  assert( rect.IsDefined() );
  assert( !geoid || geoid->IsDefined() );
  if(geoid){
    cout << __PRETTY_FUNCTION__ << ": Spherical geometry not implemented."
         <<endl;
    assert(false); // TODO: Implement spherical geometry case.
  }
  // both endpoints are outside the rectangle
  double x0(rect.MinD(0));
  double y0(rect.MinD(1));
  double x1(rect.MaxD(0));
  double y1(rect.MaxD(1));
  Point p0(true,x0,y0);
  Point p1(true,x1,y0);
  Point p2(true,x1,y1);
  Point p3(true,x0,y1);
  double d1 = lp.Distance(p0,geoid);
  double d2 = lp.Distance(p1,geoid);
  double d3 = lp.Distance(p2,geoid);
  double d4 = lp.Distance(p3,geoid);
  double dist1 = MAX(MAX(d1,d2),MAX(d3,d4));
  d1 = rp.Distance(p0,geoid);
  d2 = rp.Distance(p1,geoid);
  d3 = rp.Distance(p2,geoid);
  d4 = rp.Distance(p3,geoid);
  double dist2 = MAX(MAX(d1,d2),MAX(d3,d4));
  double dist = MAX(dist1,dist2);
  return dist;
}


bool HalfSegment::RayAbove( const Point& p, double &abovey0 ) const
{
  assert( p.IsDefined() );

  const Coord& x = p.GetX(), y = p.GetY(),
               xl = GetLeftPoint().GetX(),
               yl = GetLeftPoint().GetY(),
               xr = GetRightPoint().GetX(),
               yr = GetRightPoint().GetY();

  if (xl!=xr)
  {
    if( x == xl && yl > y )
    {
      abovey0 = yl;
      return true;
    }
    else if( xl < x && x <= xr )
    {
      double k = (yr - yl) / (xr - xl),
             a = (yl - k * xl),
             y0 = k * x + a;
      Coord yy = y0;
      if( yy > y )
      {
        abovey0 = y0;
        return true;
      }
    }
  }
  return false;
}

typedef unsigned int outcode;
enum { TOP = 0x1, BOTTOM = 0x2, RIGHT = 0x4, LEFT = 0x8 };

outcode CompOutCode( double x, double y, double xmin,
                     double xmax, double ymin, double ymax)
{
  outcode code = 0;
  if (y > ymax)
    code |=TOP;
  else
    if (y < ymin)
      code |= BOTTOM;
  if ( x > xmax)
    code |= RIGHT;
  else
    if ( x < xmin)
      code |= LEFT;
  return code;
}

void HalfSegment::CohenSutherlandLineClipping( const Rectangle<2> &window,
                                               double &x0, double &y0,
                                               double &x1, double &y1,
                                               bool &accept,
                                               const Geoid* geoid/*=0*/ ) const
{
  assert( window.IsDefined() );
  assert( !geoid || geoid->IsDefined() );

  if(geoid){
    cerr << __PRETTY_FUNCTION__ << ": Spherical geometry not implemented."
         << endl;
    assert( false ); // TODO: implement spherical geometry case
  }

  // Outcodes for P0, P1, and whatever point lies outside the clip rectangle*/
  outcode outcode0, outcode1, outcodeOut;
  double xmin = window.MinD(0)  , xmax = window.MaxD(0),
         ymin = window.MinD(1), ymax = window.MaxD(1);
  bool done = false;
  accept = false;

  outcode0 = CompOutCode( x0, y0, xmin, xmax, ymin, ymax);
  outcode1 = CompOutCode( x1, y1, xmin, xmax, ymin, ymax);

  do{
    if ( !(outcode0 | outcode1) ) {
      //"Trivial accept and exit"<<endl;
      accept = true;
      done = true;
    } else if (outcode0 & outcode1){
      done = true;
        //"Logical and is true, so trivial reject and exit"<<endl;
    } else {
      //Failed both tests, so calculate the line segment to clip:
      //from an outside point to an instersection with clip edge.
      double x,y;
      // At least one endpoint is outside the clip rectangle; pick it.
      outcodeOut = outcode0 ? outcode0 : outcode1;
      //Now find the intersection point;
      //use formulas y = y0 + slope * (x - x0), x = x0 + (1 /slope) * (y-y0).

      if (outcodeOut & TOP){ //Divide the line at top of clip rectangle
        x = x0 + (x1 - x0) * (ymax - y0) / (y1 - y0);
        y = ymax;
      }else if (outcodeOut & BOTTOM){
        //Divide line at bottom edge of clip rectangle
        x = x0 + (x1 - x0) * (ymin - y0) / (y1 - y0);
        y = ymin;
      }else if (outcodeOut & RIGHT){
        //Divide line at right edge of clip rectangle
        y = y0 + (y1 - y0) * (xmax - x0) / (x1 - x0);
        x = xmax;
      } else {// divide lene at left edge of clip rectangle
        y = y0 + (y1 - y0) * (xmin - x0) / (x1 - x0);
        x = xmin;
      }

      //Now we move outside point to intersection point to clip
      //and get ready for next pass
      if (outcodeOut == outcode0){
        x0 = x;
        y0 = y;
        outcode0 = CompOutCode(x0, y0, xmin, xmax, ymin, ymax);
      } else {
        x1 = x;
        y1 = y;
        outcode1 = CompOutCode(x1, y1, xmin, xmax, ymin, ymax);
      }
    }
  }
  while( done == false );
}

void HalfSegment::WindowClippingIn( const Rectangle<2> &window,
                                    HalfSegment &hsInside, bool &inside,
                                    bool &isIntersectionPoint,
                                    Point &intersectionPoint,
                                    const Geoid* geoid/*=0*/) const
{
  if( !window.IsDefined() ) {
    intersectionPoint.SetDefined( false );
    assert( window.IsDefined() );
  }
  assert( !geoid || geoid->IsDefined() );
  if(geoid){
    cerr << __PRETTY_FUNCTION__ << ": Spherical geometry not implemented."
         << endl;
    assert( false ); // TODO: implement spherical geometry case
  }

  double x0 = GetLeftPoint().GetX(),
         y0 = GetLeftPoint().GetY(),
         x1 = GetRightPoint().GetX(),
         y1 = GetRightPoint().GetY();

  CohenSutherlandLineClipping(window, x0, y0, x1, y1, inside);
  isIntersectionPoint=false;
  intersectionPoint.SetDefined( false );

  if (inside)
  {
    Point lp( true, x0, y0 ), rp(true, x1, y1 );

    if (lp==rp)
    {
      intersectionPoint.SetDefined( true );
      isIntersectionPoint = true;
      intersectionPoint=lp;
    }
    else
    {
      AttrType attr=this->GetAttr();
      hsInside.Set(true, rp, lp);
      hsInside.SetAttr(attr);
    }
  }
}

/*
6.2 List Representation

The list representation of a HalfSegment is

----  ( bool (lp rp))
----

where the bool value indicate whether the dominating point is the left point.

6.3 ~In~ and ~Out~ Functions

*/
ListExpr
OutHalfSegment( ListExpr typeInfo, Word value )
{
  HalfSegment* hs;
  hs = (HalfSegment*)(value.addr);

  Point lp = hs->GetLeftPoint(),
            rp = hs->GetRightPoint();

  return nl->TwoElemList(
           nl-> BoolAtom(hs->IsLeftDomPoint() ),
           nl->TwoElemList(
             OutPoint( nl->TheEmptyList(), SetWord( &lp ) ),
             OutPoint( nl->TheEmptyList(), SetWord( &rp) ) ) );
}

Word
InHalfSegment( const ListExpr typeInfo, const ListExpr instance,
               const int errorPos, ListExpr& errorInfo, bool& correct )
{
  if ( nl->ListLength( instance ) == 2 )
  {
    ListExpr first = nl->First(instance),
             second = nl->Second(instance),
             firstP = nl->TheEmptyList(),
             secondP = nl->TheEmptyList();
    bool ldp;

    if( nl->IsAtom(first) && nl->AtomType(first) == BoolType )
    {
      ldp = nl->BoolValue(first);

      if( nl->ListLength(second) == 2 )
      {
        firstP = nl->First(second);
        secondP = nl->Second(second);
      }

      correct=true;
      Point *lp = (Point*)InPoint(nl->TheEmptyList(),
                                  firstP, 0, errorInfo, correct ).addr;
      if( correct && lp->IsDefined() )
      {
        Point *rp = (Point*)InPoint(nl->TheEmptyList(),
                                    secondP, 0, errorInfo, correct ).addr;
        if( correct && rp->IsDefined() && *lp != *rp )
        {
          HalfSegment *hs = new HalfSegment( ldp, *lp, *rp );
          delete lp;
          delete rp;
          return SetWord( hs );
        }
        delete rp;
      }
      delete lp;
    }
  }

  correct = false;
  return SetWord( Address(0) );
}


/*
~getNext~

auxiliary function for Line::Transform

*/
template<template<typename T>class Array>
int getNext(const Array<HalfSegment>* hss, int pos, const char* usage){
   HalfSegment hs;
   hss->Get(pos,hs);
   pos = hs.attr.partnerno;
   hss->Get(pos,hs);
   Point dp = hs.GetDomPoint();
   int res = -1;
   // go backwards from pos, store a candidate as res
   int pos1 = pos-1;
   bool done = false;
   // search left of pos
   while( (pos1>=0) && ! done){
       hss->Get(pos1,hs);
       Point dp1 = hs.GetDomPoint();
       if(!AlmostEqual(dp,dp1)){
         done = true;
       } else {
         if(usage[pos1] == 3){
           return pos1;
         } else if(usage[pos1]==0){
            res = pos1;
         }
         pos1--;
       }
   }
   // search right of pos
   pos1 = pos+1;
   done = false;
   while((pos1<hss->Size()) && ! done){
       hss->Get(pos1,hs);
        Point dp1 = hs.GetDomPoint();
        if(!AlmostEqual(dp,dp1)){
          done = true;
        } else {
           if(usage[pos1] == 3){
             return pos1;
           } else if(usage[pos1]==0){
              res = pos1;
           }
           pos1++;
        }
   }
   return res;
}


/*
~markUsage~

This function marks all HalfSgements of a line with its usage. This means:
0 : Part of an unique cycle
1 : not Part of a cycle
2 : Part of an ambiguous cycle


*/
template<template<typename T>class Array>
void markUsage( LineT<Array>* line, char* usage, char* critical){
    markUsage( (Array<HalfSegment>*) line->GetFLOB(0),usage, critical);
}


template<template<typename T>class Array>
void markUsage(const Array<HalfSegment>* hss, char* usage, char* critical ){
  // step 1: mark halfsegments not belonging to a cycle
  memset(usage,0,hss->Size());
  // meaning of elements of usage
  // 0 : not used => part of a cycle
  // 1 : not part of a cycle
  // 2 : part of a cycle
  // 3 : part of the current path (will be set to 1 or 2 later)
  // 4 : partner of a segment in the current path
  memset(critical,0,hss->Size());
  // 1.1 mark critical points
  int count = 0;
  HalfSegment hs;
  Point lastPoint;
  for(int i=0;i<hss->Size();i++){
     hss->Get(i,hs);
     Point currentPoint = hs.GetDomPoint();
     if(i==0){
       lastPoint = currentPoint;
       count = 1;
     } else {
        if(AlmostEqual(lastPoint,currentPoint)){
           count++;
        } else {
          if(count != 2){
             for(int r=1;r<=count-1;r++){
                critical[i-r] = 1;
             }
          }
          lastPoint = currentPoint;
          count = 1;
        }
     }
  }
  if(count != 2){
     for(int r=1;r<=count;r++){
        critical[hss->Size()-r] = 1;
     }
  }
  // 1.2 mark segments
  for(int i=0;i<hss->Size();i++) {
     if(usage[i]==0 ){
        // not used, but critical
        hss->Get(i,hs);
        vector<int> path;
        usage[i] = 3; // part of current path
        usage[hs.attr.partnerno] = 4;
        // extend path
        int pos = i;
        path.push_back(pos);
        int next = getNext(hss,pos,usage);
        while(!path.empty()) {
          if(next < 0){
             // no extension found => mark segments in path as non-cycle
             // go back until the next critical point is reachedA
             bool done = path.empty();
             while(!done){
               if(path.empty()){
                 done = true;
               } else {
                 pos = path.back();
                 path.pop_back();
                 usage[pos] = 1;
                 hss->Get(pos,hs);
                 usage[hs.attr.partnerno] = 1;

                 if(critical[pos]){
                   done = true;
                 } else {
                 }
               }
             }
             if(!path.empty()){
               pos = path.back();
               next = getNext(hss,pos,usage);
             }
          } else {
            if(usage[next] == 3){
                // (sub) path found
                int p = path.back();
                path.pop_back();
                while(p!=next && !path.empty()){
                  usage[p] = 2;
                  hss->Get(p,hs);
                  usage[hs.attr.partnerno] = 2;
                  p = path.back();
                  path.pop_back();
                }
                usage[p] = 2;
                hss->Get(p,hs);
                usage[hs.attr.partnerno] = 2;
                if(!path.empty()){
                  pos = path.back(); // try to extend first part of path
                  next = getNext(hss,pos,usage);
                }
            }  else { // normal extension
               pos = next;
               path.push_back(pos);
               usage[pos] = 3;
               hss->Get(pos,hs);
               usage[hs.attr.partnerno] = 4;
               next = getNext(hss,pos,usage);
            }
          }
        } // while path not empty
     }
  }

}






/*
Simple function marking some elements between min and max as used to avoid
segments degenerated to single points.

*/
static double maxDist(const vector<Point>& orig, // points
                      const int min, const int max, // range
                      int& index,       // resultindex
                      const Geoid* geoid=0){

  // search the point with the largest distance to the segment between
  // orig[min] and orig[max]
  double maxdist = 0.0;
  int maxindex = -1;

  try{
    if(!AlmostEqual(orig.at(min),orig.at(max))){
      HalfSegment hs(true,orig.at(min),orig.at(max));
      for(int i=min+1; i<max; i++){
        double dist = hs.Distance(orig.at(i),geoid);
        if(dist>maxdist){
          maxdist = dist;
          maxindex = i;
        }
      }
  } else { // special case of a cycle
      Point p = orig[min];
      for(int i=min+1; i<max; i++){
        double dist = p.Distance(orig.at(i),geoid);
        if(dist>maxdist){
          maxdist = dist;
          maxindex = i;
        }
      }
    }
    index = maxindex;
    return maxdist;
  } catch (out_of_range){
      cerr << "min=" << min << " max=" << max << " size="
          << orig.size() << endl;
      assert(false);
  }
}


/*
Implementation of the Douglas Peucker algorithm.

The return indoicates whether between min and max further points was
used.

*/

bool douglas_peucker(const vector<Point>& orig, // original line
                     const double epsilon, // maximum derivation
                     bool* use, // result
                     const int min, const int max,
                     const bool force ,
                     const Geoid* geoid){ // current range
  // always use the endpoints
  use[min] = true;
  use[max] = true;
  if(min+1>=max){ // no inner points, nothing to do
    return false;
  }
  int index;
  double maxdist = maxDist(orig,min,max,index,geoid);
  bool cycle = AlmostEqual(orig[min], orig[max]);
  if( (maxdist<=epsilon) &&  // line closed enough
      !cycle &&  // no degenerated segment
      !force){
    return false; // all ok, stop recursion
  } else {
    bool ins = douglas_peucker(orig,epsilon,use,min,index,cycle,geoid);
    if(index>=0){
      douglas_peucker(orig,epsilon,use,index,max,cycle && !ins,geoid);
    }
    return true;
  }
}



void  douglas_peucker(const vector<Point>& orig,  // original chain of points
                      const double epsilon, // maximum derivation
                      bool* use,            // result
                      const bool forceThrow,
                      const Geoid* geoid ){
  for(unsigned int i=0;i<orig.size();i++){
    use[i] = false;
  }
  // call the recursive implementation
  douglas_peucker(orig,epsilon, use, 0, orig.size()-1,forceThrow,geoid);
}



/*
7.2 List Representation

The list representation of a line is

----  ((x1 y1 x2 y2) (x1 y1 x2 y2) ....)
----

or

---- undef
----

7.3 ~Out~-function

*/
ListExpr
OutLine( ListExpr typeInfo, Word value )
{
  ListExpr result, last;
  HalfSegment hs;
  ListExpr halfseg, halfpoints, flatseg;
  Line* l = (Line*)(value.addr);

  if(!l->IsDefined()){
    return nl->SymbolAtom(Symbol::UNDEFINED());
  }


  if( l->IsEmpty() )
    return nl->TheEmptyList();

  result = nl->TheEmptyList();
  last = result;
  bool first = true;

  for( int i = 0; i < l->Size(); i++ )
  {
    l->Get( i, hs );
    if( hs.IsLeftDomPoint() == true )
    {
      halfseg = OutHalfSegment( nl->TheEmptyList(), SetWord( (void*)&hs ) );
      halfpoints = nl->Second( halfseg );
      flatseg = nl->FourElemList(
                  nl->First( nl->First( halfpoints ) ),
                  nl->Second( nl->First( halfpoints ) ),
                  nl->First( nl->Second( halfpoints ) ),
                  nl->Second( nl->Second( halfpoints ) ) );
      if( first == true )
      {
        result = nl->OneElemList( flatseg );
        last = result;
        first = false;
      }
      else
        last = nl->Append( last, flatseg );
    }
  }
  return result;
}

/*
7.4 ~In~-function

*/
Word
InLine( const ListExpr typeInfo, const ListExpr instance,
        const int errorPos, ListExpr& errorInfo, bool& correct )
{
  Line* l = new Line( 0 );

  if(listutils::isSymbolUndefined(instance)){
    l->SetDefined(false);
    correct=true;
    return SetWord(Address(l));
  }

  HalfSegment * hs;
  l->StartBulkLoad();
  ListExpr first, halfseg, halfpoint;
  ListExpr rest = instance;
  int edgeno = 0;

  if( !nl->IsAtom( instance ) )
  {
    while( !nl->IsEmpty( rest ) )
    {
      first = nl->First( rest );
      rest = nl->Rest( rest );

      if( nl->ListLength( first ) == 4 )
      {
        halfpoint = nl->TwoElemList(
                      nl->TwoElemList(
                        nl->First(first),
                        nl->Second(first)),
                      nl->TwoElemList(
                        nl->Third(first),
                        nl->Fourth(first)));
      } else { // wrong list representation
         l->DeleteIfAllowed();
         correct = false;
         return SetWord( Address(0) );
      }
      halfseg = nl->TwoElemList(nl->BoolAtom(true), halfpoint);
      hs = (HalfSegment*)InHalfSegment( nl->TheEmptyList(), halfseg,
                                        0, errorInfo, correct ).addr;
      if( correct )
      {
        hs->attr.edgeno = edgeno++;
        *l += *hs;
        hs->SetLeftDomPoint( !hs->IsLeftDomPoint() );
        *l += *hs;
      }
      delete hs;
    }
    l->EndBulkLoad();

    correct = true;
    return SetWord( l );
  }
  l->DeleteIfAllowed();
  correct = false;
  return SetWord( Address(0) );
}

/*
7.5 ~Create~-function

*/
Word
CreateLine( const ListExpr typeInfo )
{
  return SetWord( new Line( 0 ) );
}

/*
7.6 ~Delete~-function

*/
void
DeleteLine( const ListExpr typeInfo, Word& w )
{
  Line *l = (Line *)w.addr;
  l->Destroy();
  l->DeleteIfAllowed(false);
  w.addr = 0;
}

/*
7.7 ~Close~-function

*/
void
CloseLine( const ListExpr typeInfo, Word& w )
{
  ((Line *)w.addr)->DeleteIfAllowed();
  w.addr = 0;
}

/*
7.8 ~Clone~-function

*/
Word
CloneLine( const ListExpr typeInfo, const Word& w )
{
  return SetWord( new Line( *((Line *)w.addr) ) );
}

/*
7.8 ~Open~-function

*/
bool
OpenLine( SmiRecord& valueRecord,
          size_t& offset,
          const ListExpr typeInfo,
          Word& value )
{
  Line *l = (Line*)Attribute::Open( valueRecord, offset, typeInfo );
  value = SetWord( l );
  return true;
}

/*
7.8 ~Save~-function

*/
bool
SaveLine( SmiRecord& valueRecord,
          size_t& offset,
          const ListExpr typeInfo,
          Word& value )
{
  Line *l = (Line *)value.addr;
  Attribute::Save( valueRecord, offset, typeInfo, l );
  return true;
}

/*
7.9 ~SizeOf~-function

*/
int SizeOfLine()
{
  return sizeof(Line);
}

/*
7.11 Function describing the signature of the type constructor

*/
ListExpr
LineProperty()
{
  return nl->TwoElemList(
           nl->FourElemList(
             nl->StringAtom("Signature"),
             nl->StringAtom("Example Type List"),
             nl->StringAtom("List Rep"),
             nl->StringAtom("Example List")),
           nl->FourElemList(
             nl->StringAtom("-> DATA"),
             nl->StringAtom(Line::BasicType()),
             nl->StringAtom("(<segment>*) where segment is "
               "(<x1><y1><x2><y2>)"),
             nl->StringAtom("( (1 1 2 2)(3 3 4 4) )")));
}

/*
7.12 Kind checking function

This function checks whether the type constructor is applied correctly. Since
type constructor ~line~ does not have arguments, this is trivial.

*/
bool
CheckLine( ListExpr type, ListExpr& errorInfo )
{
  return nl->IsEqual( type, Line::BasicType() );
}

/*
7.13 ~Cast~-function

*/
void* CastLine(void* addr)
{
  return Line::Cast(addr);
}

/*
7.14 Creation of the type constructor instance

*/
TypeConstructor line(
        Line::BasicType(),                         //name
        LineProperty,                   //describing signature
        OutLine,        InLine,         //Out and In functions
        0,              0,              //SaveTo and RestoreFrom List functions
        CreateLine,     DeleteLine,     //object creation and deletion
        OpenLine,       SaveLine,       // object open and save
        CloseLine,      CloneLine,      //object close and clone
        CastLine,                       //cast function
        SizeOfLine,                     //sizeof function
        CheckLine );                    //kind checking function



Word
 InSimpleLine( const ListExpr typeInfo, const ListExpr instance1,
               const int errorPos, ListExpr& errorInfo, bool& correct ){

 ListExpr instance = instance1;
 correct = true;
 if(listutils::isSymbolUndefined(instance)){
    SimpleLine* line = new SimpleLine( 0 );
    line->SetDefined(false);
    return SetWord(Address(line));
 }

 if(nl->AtomType(instance)!=NoAtom){
    correct=false;
    return SetWord(Address(0));
 }

 bool startSmaller = true;
 if(   nl->HasLength(instance,2) 
    && (nl->AtomType(nl->Second(instance))==BoolType)){
     startSmaller = nl->BoolValue(nl->Second(instance));
     instance = nl->First(instance);
 }  
 HalfSegment* hs;
 SimpleLine* line= new SimpleLine(10);
 int edgeno = 0;
 ListExpr rest = instance;
 line->StartBulkLoad();
 while(!nl->IsEmpty(rest)){
   ListExpr segment = nl->First(rest);
   if(!nl->HasLength(segment,4)){
      correct=false;
      line->DeleteIfAllowed();
      return SetWord(Address(0));
   }
   ListExpr halfSegment = nl->TwoElemList(
                                 nl->BoolAtom(true),
                                 nl->TwoElemList(
                                    nl->TwoElemList(nl->First(segment),
                                                    nl->Second(segment)),
                                    nl->TwoElemList(nl->Third(segment),
                                                    nl->Fourth(segment))));
   hs = static_cast<HalfSegment*>(
           InHalfSegment( nl->TheEmptyList(), halfSegment,
                          0, errorInfo, correct ).addr);
   if(!correct){
      delete hs;
      return SetWord(Address(0));
   }
   hs->attr.edgeno = edgeno++;
   *line += *hs;
   hs->SetLeftDomPoint( !hs->IsLeftDomPoint() );
   *line += *hs;
   delete hs;
   rest = nl->Rest(rest);
 }
 if(!line->EndBulkLoad()){
   correct = false;
   line->DeleteIfAllowed();
   return SetWord(Address(0));
 }else{
   correct = true;
   line->SetStartSmaller(startSmaller);
   return SetWord(line);
 }
}


 ostream& operator<<(ostream& out, const LRS& lrs){
   out << "lrsPos "<< lrs.lrsPos << ", hsPos " << lrs.hsPos;
   return out;
 }



 ListExpr OutSimpleLine( ListExpr typeInfo, Word value ) {
   ListExpr result, last;
   HalfSegment hs;
   ListExpr halfseg, halfpoints, flatseg;
   SimpleLine* l = static_cast<SimpleLine*>(value.addr);

   if(!l->IsDefined()){
     return nl->SymbolAtom(Symbol::UNDEFINED());
   }

   if( l->IsEmpty() ){
     return nl->TwoElemList( nl->TheEmptyList(),
                             nl->BoolAtom( l->GetStartSmaller() ) );
   }

   result = nl->TheEmptyList();
   last = result;
   bool first = true;

   
   /*
   // version using halfsegment order

   for( int i = 0; i < l->Size(); i++ ) {
      l->Get( i, hs );
      if( hs.IsLeftDomPoint() ){
        halfseg = OutHalfSegment( nl->TheEmptyList(), SetWord( (void*)&hs ) );
        halfpoints = nl->Second( halfseg );
        flatseg = nl->FourElemList(
                    nl->First( nl->First( halfpoints ) ),
                    nl->Second( nl->First( halfpoints ) ),
                    nl->First( nl->Second( halfpoints ) ),
                    nl->Second( nl->Second( halfpoints ) ) );
        if( first == true ) {
          result = nl->OneElemList( flatseg );
          last = result;
          first = false;
        } else {
          last = nl->Append( last, flatseg );
        }
      }
   }
   */

   // version using lrs order

   LRS lrs;
   // for some curious reason, some halfsegments occur twice
   // in the lrs array
   set<int> used;


   for(int i=0;i<l->lrsSize(); i++){
      l->Get(i,lrs);
      if(used.find(lrs.hsPos)==used.end()){
        used.insert(lrs.hsPos);
        //cout << "LRS " << lrs << endl;
        l->Get(lrs.hsPos,hs);
        halfseg = OutHalfSegment( nl->TheEmptyList(), SetWord( (void*)&hs ) );
        halfpoints = nl->Second( halfseg );
        flatseg = nl->FourElemList(
                   nl->First( nl->First( halfpoints ) ),
                   nl->Second( nl->First( halfpoints ) ),
                   nl->First( nl->Second( halfpoints ) ),
                   nl->Second( nl->Second( halfpoints ) ) );
        if( first == true ) {
          result = nl->OneElemList( flatseg );
          last = result;
          first = false;
        } else {
          last = nl->Append( last, flatseg );
        }
      }
   }
      
   return nl->TwoElemList( result,
                           nl->BoolAtom( l->GetStartSmaller() ) );
}

 Word CreateSimpleLine( const ListExpr typeInfo ) {
   return SetWord( new SimpleLine( 0 ) );
 }

void DeleteSimpleLine( const ListExpr typeInfo, Word& w ) {
  SimpleLine *l = static_cast<SimpleLine*>(w.addr);
  l->Destroy();
  l->DeleteIfAllowed(false);
  w.addr = 0;
}


void CloseSimpleLine( const ListExpr typeInfo, Word& w ) {
 (static_cast<SimpleLine*>(w.addr))->DeleteIfAllowed();
 w.addr = 0;
}

Word CloneSimpleLine( const ListExpr typeInfo, const Word& w ) {
  return SetWord( new SimpleLine( *((SimpleLine *)w.addr) ) );
}

int SizeOfSimpleLine() {
   return sizeof(SimpleLine);
}

 ListExpr SimpleLineProperty() {
   return nl->TwoElemList(
            nl->FourElemList(
              nl->StringAtom("Signature"),
              nl->StringAtom("Example Type List"),
              nl->StringAtom("List Rep"),
              nl->StringAtom("Example List")),
            nl->FourElemList(
              nl->StringAtom("-> DATA"),
              nl->StringAtom(SimpleLine::BasicType()),
              nl->TextAtom("( (<segment>*) <bool> ) where segment is "
                             "(<x1><y1><x2><y2>) and bool is TRUE or FALSE"),
              nl->StringAtom("( ( (1 1 2 2) (2 2 1 4) ) FALSE )")));
}

bool CheckSimpleLine( ListExpr type, ListExpr& errorInfo ){
   return nl->IsEqual( type, SimpleLine::BasicType() );
}

void* CastSimpleLine(void* addr) {
  return SimpleLine::Cast(addr);;
}

TypeConstructor sline(
     SimpleLine::BasicType(),                         //name
     SimpleLineProperty,                   //describing signature
     OutSimpleLine,  InSimpleLine,         //Out and In functions
     0,              0,              //SaveTo and RestoreFrom List functions
     CreateSimpleLine, DeleteSimpleLine,     //object creation and deletion
     OpenAttribute<SimpleLine>,
     SaveAttribute<SimpleLine>,       // object open and save
     CloseSimpleLine, CloneSimpleLine,      //object close and clone
     CastSimpleLine,                       //cast function
     SizeOfSimpleLine,                     //sizeof function
     CheckSimpleLine);



bool IsInsideAbove(const HalfSegment& hs,
                   const Point& thirdPoint){
  if(AlmostEqual(hs.GetLeftPoint().GetX(),hs.GetRightPoint().GetX())){
    // vertical segment
    return
    (MIN(hs.GetLeftPoint().GetX(),hs.GetRightPoint().GetX())<thirdPoint.GetX());
  }
  return
    (MIN(hs.GetLeftPoint().GetY(),hs.GetRightPoint().GetY())<thirdPoint.GetY());
}


double VectorSize(const Point &p1, const Point &p2, const Geoid* geoid/*=0*/)
{
  assert( p1.IsDefined() );
  assert( p2.IsDefined() );
  assert( !geoid || geoid->IsDefined() );
  if(geoid){
    double size = p1.Distance(p1, geoid);
    assert( size > 0 );
    return size;
  }else {
    double size = pow( (p1.GetX() - p2.GetX()),2)
                  + pow( (p1.GetY() - p2.GetY()),2);
    size = sqrt(size);
    return size;
  }
}

//The angle function returns the angle of VP1P2
// P1 is the point on the window's edge
double Angle(const Point &v, const Point &p1,const Point &p2)
{
  assert( v.IsDefined() );
  assert( p1.IsDefined() );
  assert( p2.IsDefined() );
  double coss;

  //If P1P2 is vertical and the window's edge
  // been tested is horizontal , then
  //the angle VP1P2 is equal to 90 degrees. On the
  //other hand, if P1P2 is vertical
  //and the window's edge been tested is vertical, then
  // the angle is 90 degrees.
  //Similar tests are applied when P1P2 is horizontal.

  if (p1.GetX() == p2.GetX()){ //the segment is vertical
    if (v.GetY()==p1.GetY()){
        return M_PI/2; //horizontal edge
    } else {
        return 0;
    }
  }
  if (p1.GetY() == p2.GetY()){ //the segment is horizontal
    if (v.GetY()==p1.GetY()){
      return 0; //horizontal edge
    } else {
      return M_PI/2;
    }
  }
  coss = double( ( (v.GetX() - p1.GetX()) * (p2.GetX() - p1.GetX()) ) +
                 ( (v.GetY() - p1.GetY()) * (p2.GetY() - p1.GetY()) ) ) /
                 (VectorSize(v,p1) * VectorSize(p2,p1));
  //cout<<endl<<"Coss"<<coss;
  //coss = abs(coss);
  //cout<<endl<<"Coss"<<coss;
  return acos(coss);
}


ostream& operator<<( ostream& o, const EdgePoint& p )
{
  o << "(" << p.GetX() << ", " << p.GetY() << ")"
    <<" D("<<(p.direction ? "LEFT/DOWN" : "RIGHT/UP")<<")"
    <<" R("<<(p.rejected ? "Rejected" : "Accepted")<<")";
  return o;
}

EdgePoint* EdgePoint::GetEdgePoint( const Point &p,
                                    const Point &p2,
                                    bool insideAbove,
                                    const Point &v,
                                    const bool reject )
{
  //The point p2 must be outside the window
  bool direction;

  //window's vertical edge
  if (v.GetX()==p.GetX())
  {
    if (insideAbove)
      direction =  false; //UP
    else
      direction =  true; //DOWN
  }
  else  //Horizontal edge
  {
    if (insideAbove)
    {
      if ( (p.GetX()-p2.GetX())>0 )
        //p2.GetX() is located to the left of p.GetX()
        direction =  false; //RIGHT
      else
        direction =  true; //LEFT
    }
    else
    {
      if ( (p.GetX()-p2.GetX())>0 )
        //p2.GetX() is located to the right of p.GetX()
        direction =  true; //LEFT
      else
        direction =  false; //RIGHT
    }
  }
  return new EdgePoint(p,direction,reject);

}

void AddPointToEdgeArray( const Point &p,
                          const HalfSegment &hs,
                          const Rectangle<2> &window,
                          vector<EdgePoint> pointsOnEdge[4] )
{
  EdgePoint *dp;
  Point v(true);
  AttrType attr;
  attr = hs.GetAttr();
  Point p2(true);
  //If the left and right edges are been tested then
  //it is not need to check the angle
  //between the half segment and the edge. If the attribute
  //inside above is true, then
  //the direction is up (false), otherwise it is down (true).
  if (p.GetX() == window.MinD(0))
  {
    dp = new EdgePoint(p,!attr.insideAbove,false);
    pointsOnEdge[WLEFT].push_back(*dp);
  }
  else
    if (p.GetX() == window.MaxD(0))
    {
      dp = new EdgePoint(p,!attr.insideAbove,false);
      pointsOnEdge[WRIGHT].push_back(*dp);
    }
  if (p.GetY() == window.MinD(1))
  {
    v.Set(window.MinD(0), window.MinD(1));
    //In this case we don't know which point is outside the window,
    //so it is need to test both half segment's poinst. Moreover,
    //in order to use the same comparison that is used for
    //Top edge, it is need to choose the half segment point that
    //is over the bottom edge.
    if (hs.GetLeftPoint().GetY()>window.MinD(1))
      dp = EdgePoint::GetEdgePoint(p,hs.GetLeftPoint(),
                                   attr.insideAbove,v,false);
    else
      dp = EdgePoint::GetEdgePoint(p,hs.GetRightPoint(),
                                   attr.insideAbove,v,false);
    pointsOnEdge[WBOTTOM].push_back(*dp);
  }
  else
    if (p.GetY() == window.MaxD(1))
    {
      v.Set(window.MinD(0), window.MaxD(1));
      //In this case we don't know which point is outside the window,
      //so it is need to test
      if (hs.GetLeftPoint().GetY()>window.MaxD(1))
        dp = EdgePoint::GetEdgePoint(p,hs.GetLeftPoint(),
                                     attr.insideAbove,v,false);
      else
        dp = EdgePoint::GetEdgePoint(p,hs.GetRightPoint(),
                                     attr.insideAbove,v,false);
      pointsOnEdge[WTOP].push_back(*dp);
    }
}

bool GetAcceptedPoint( vector <EdgePoint>pointsOnEdge,
                       int &i, const int end,
                       EdgePoint &ep )
{
  //id is the indice of the current point in the scan
  //ep is the correct edge point that will be returned.
  ep = pointsOnEdge[i];
  //discard all rejected points
  while (ep.rejected && i<=end)
  {
    i++;
    if (i>end)
      return false;
    EdgePoint epAux = pointsOnEdge[i];
    //Discard all the points that was accepted but has a
    // corresponding rejection point.
    //In other words, point that has the same
    // coordinates and direction on the edge.
    if (!epAux.rejected && (epAux.direction==ep.direction) &&
         (epAux.GetX() == ep.GetX()) && (epAux.GetY() == ep.GetY()) )
    {
      while ( (i<=end) && (epAux.direction==ep.direction) &&
         (epAux.GetX() == ep.GetX()) && (epAux.GetY() == ep.GetY()) )
      {
        i++;
        if (i>end)
          return false;
        epAux = pointsOnEdge[i];
      }
    }
    ep = epAux;
  }
  return true;
}

bool HalfSegment::RayDown( const Point& p, double &yIntersection ) const
{
    if (this->IsVertical())
          return false;

    const Coord& x = p.GetX(), y = p.GetY(),
                 xl = GetLeftPoint().GetX(),
                 yl = GetLeftPoint().GetY(),
                 xr = GetRightPoint().GetX(),
                 yr = GetRightPoint().GetY();

    // between is true, iff xl <= x <= xr.
    const bool between = CompareDouble(x, xl) != -1 &&
                         CompareDouble(x, xr) != 1;

    if (!between)
        return false;

    const double k = (yr - yl) / (xr - xl);
    const double a = (yl - k * xl);
    const double y0 = k * x + a;

    if (CompareDouble(y0, y) == 1) // y0 > y: this is above p.
        return false;

    // y0 <= p: p is above or on this.

    yIntersection = y0;

    return true;
}





/*
~IsSpatialType~

This function checks whether the type given as a ListExpr is one of
~point~, ~points~, ~line~, or ~region~.

*/

bool IsSpatialType(ListExpr type){
   if(!nl->IsAtom(type)){
      return false;
   }
   if(nl->AtomType(type)!=SymbolType){
      return false;
   }
   string t = nl->SymbolValue(type);
   if(t==Point::BasicType()) return true;
   if(t==Points::BasicType()) return true;
   if(t==Line::BasicType()) return true;
   if(t==Region::BasicType()) return true;
   return false;
}




/*
This function check whether a region value is valid after the insertion of a
 new half segment.  Whenever a half segment is about to be inserted, the
state of the region is checked.
A valid region must satisfy the following conditions:

1)  any two cycles of the same region must be disconnect, which means that no
  edges of different cycles can intersect each other;

2) edges of the same cycle can only intersect with their endpoints, but no
  their middle points;

3)  For a certain face, the holes must be inside the outer cycle;

4)  For a certain face, any two holes can not contain each other;

5)  Faces must have the outer cycle, but they can have no holes;

6)  for a certain cycle, any two vertex can not be the same;

7)  any cycle must be made up of at least 3 edges;

8)  It is allowed that one face is inside another provided that their edges do
   not intersect.

*/

vector<Point> getCycle(const bool isHole,
                              const vector<HalfSegment>& vhs){

  // first extract the cycle
  bool used[vhs.size()];
  for(unsigned int i=0;i<vhs.size();i++){
    used[i] = false;
  }
  if(vhs.size() < 3 ){
    assert(false);
  }
  // trivial n^2 implementation
  HalfSegment hs = vhs[0];
  Point sp = hs.GetDomPoint();
  Point cp = hs.GetSecPoint();
  vector<Point> vp;

  vp.push_back(sp);
  vp.push_back(cp);

  used[0] = true;
  for(unsigned int i=1; i< vhs.size();i++){
    for(unsigned int j=1; j<vhs.size(); j++){
       if(!used[j]){
          Point p0 = vhs[j].GetDomPoint();
          Point p1 = vhs[j].GetSecPoint();
          if(AlmostEqual(p0,cp)){
             used[j] = true;
             cp = p1;
             vp.push_back(cp);
          } else if(AlmostEqual(p1,cp)){
             used[j] = true;
             cp = p0;
             vp.push_back(cp);
          }
       }
    }
  }
  vp.push_back(cp);
  // debugging only
  for(unsigned int i=0;i<vhs.size();i++){
     if(!used[i]){
        cerr << "Unused halfsegment found" << endl;
     }
  }


  bool cw = getDir(vp);

  if(!(( isHole && cw ) || (!isHole && !cw))){
    vector<Point> vp2;
    for(int i= vp.size()-1; i>=0; i--){
       vp2.push_back(vp[i]);
    }
    return vp2;
  } else {
    return vp;
  }
}





/*
Implementation of the Gauss Krueger Projection

*/

struct P3D{
  double x;
  double y;
  double z;
};

bool WGSGK::project(const Point& src, Point& result) const{
  if(!src.IsDefined()){
    cerr << __PRETTY_FUNCTION__ << ": Point Argument is undefined!" << endl;
    result.SetDefined(false);
    return false;
  }
  double x = src.GetX();
  double y = src.GetY();
  if(x<-180 || x>180 || y<-90 || y>90){
    cerr << __PRETTY_FUNCTION__ << ": Point " << src << " is not a valid "
         << "geographic coordinate!" << endl;
    result.SetDefined(false);
    return false;
  }

  double a = x*Pi/180;
  double b = y*Pi/180;
  if(!useWGS){
    BesselBLToGaussKrueger(b, a, result);
    return result.IsDefined();
  }
  double l1 = a;
  double b1 = b;
  a=awgs;
  b=bwgs;
  double eq=eqwgs;
  double N=a/sqrt(1-eq*sin(b1)*sin(b1));
  double Xq=(N+h1)*cos(b1)*cos(l1);
  double Yq=(N+h1)*cos(b1)*sin(l1);
  double Zq=((1-eq)*N+h1)*sin(b1);

  P3D p;
  HelmertTransformation(Xq, Yq, Zq, p);
  double X = p.x;
  double Y = p.y;
  double Z = p.z;

  a=abes;
  b=bbes;
  eq = eqbes;

  BLRauenberg(X, Y, Z, p);
  double b2 = p.x;
  double l2 = p.y;
  BesselBLToGaussKrueger(b2, l2, result);
  return result.IsDefined();
}


bool WGSGK::project(const HalfSegment& src, HalfSegment& result) const{
  result = src;
  Point p1(true),p2(true);
  if(!project(src.GetLeftPoint(),p1)) return false;
  if(!project(src.GetRightPoint(),p2)) return false;
  if(p2<p1){
    result.attr.insideAbove = ! src.attr.insideAbove;
    result.Set(src.IsLeftDomPoint(), p2, p1);
  } else {
    result.Set(src.IsLeftDomPoint(), p1, p2);
  }
  return true;
}


bool WGSGK::getOrig(const Point& src, Point& result) const{
  if(!src.IsDefined()){
    result.SetDefined(false);
    return false;
  }
  return  gk2geo(src.GetX(), src.GetY(), result);
}

void WGSGK::enableWGS(const bool enabled){
  useWGS = enabled;
}

void WGSGK::setMeridian(const int m){
  MDC = m;
}

void WGSGK::init(){
  Pi = 3.1415926535897932384626433832795028841971693993751058209749445923078164;
  rho = 180/Pi;
  awgs = 6378137.0;
  bwgs = 6356752.314;
  abes = 6377397.155;  // Bessel Semi-Major Axis = Equatorial Radius in meters
  bbes = 6356078.962;    // Bessel Semi-Minor Axis = Polar Radius in meters
  cbes = 111120.6196;    // Bessel latitude to Gauss-Krueger meters
  dx   = -585.7;         // Translation Parameter 1
  dy   = -87.0;          // Translation Parameter 2
  dz   = -409.2;         // Translation Parameter 3
  rotx = 2.540423689E-6; // Rotation Parameter 1
  roty = 7.514612057E-7; // Rotation Parameter 2
  rotz = -1.368144208E-5; // Rotation Parameter 3
  sc = 0.99999122;       // Scaling Factor
  h1 = 0;
  eqwgs = (awgs*awgs-bwgs*bwgs)/(awgs*awgs);
  eqbes = (abes*abes-bbes*bbes)/(abes*abes);
  MDC = 2.0;  // standard in Hagena
  useWGS = true; // usw coordinates in wgs ellipsoid
}

void WGSGK::HelmertTransformation(const double x, const double y,
                                  const double z, P3D& p) const{
  p.x = dx + (sc*(1*x+rotz*y-roty*z));
  p.y = dy + (sc*(-rotz*x+1*y+rotx*z));
  p.z = dz + (sc*(roty*x-rotx*y+1*z));
}


void WGSGK::BesselBLToGaussKrueger(const double b,
                                   const double ll,
                                   Point& result) const{
  //double bg=180*b/Pi;
  //double lng=180*ll/Pi;
  double l0 = 3*MDC;
  l0=Pi*l0/180;
  double l=ll-l0;
  double k=cos(b);
  double t=sin(b)/k;
  double eq=eqbes;
  double Vq=1+eq*k*k;
  double v=sqrt(Vq);
  double Ng=abes*abes/(bbes*v);
  double nk=(abes-bbes)/(abes+bbes);
  double X=((Ng*t*k*k*l*l)/2)+((Ng*t*(9*Vq-t*t-4)*k*k*k*k*l*l*l*l)/24);
  double gg=b+(((-3*nk/2)+(9*nk*nk*nk/16)) *
               sin(2*b)+15*nk*nk*sin(4*b)/16-35*nk*nk*nk*sin(6*b)/48);
  double SS=gg*180*cbes/Pi;
  double Ho=(SS+X);
  double Y=Ng*k*l+Ng*(Vq-t*t)*k*k*k*l*l*l/6+Ng*
            (5-18*t*t+t*t*t*t)*k*k*k*k*k*l*l*l*l*l/120;
  double kk=500000;
  //double Pii=Pi;
  double RVV = MDC;
  double Re=RVV*1000000+kk+Y;
  result.SetDefined(true);
  result.Set(Re, Ho);
}


void WGSGK::BLRauenberg (const double x, const double y,
                         const double z, P3D& result) const{

  double f=Pi*50/180;
  double p=z/sqrt(x*x+y*y);
  double f1,f2;
  do
  {
    f1=newF(f,x,y,p);
    f2=f;
    f=f1;
  }
  while(!(abs(f2-f1)<10E-10));

  result.x=f;
  result.y=atan(y/x);
  result.z=sqrt(x*x+y*y)/cos(f1)-abes/sqrt(1-eqbes*sin(f1)*sin(f1));
}


double WGSGK::newF(const double f, const double x,
                   const double y, const double p) const{
  double zw;
  double nnq;
  zw=abes/sqrt(1-eqbes*sin(f)*sin(f));
  nnq=1-eqbes*zw/(sqrt(x*x+y*y)/cos(f));
  return(atan(p/nnq));
}

bool  WGSGK::gk2geo(const double GKRight,
                    const double GKHeight,
                    Point&  result) const{
   if(GKRight<1000000 || GKHeight<1000000){
      result.SetDefined(false);
      return false;
   }
   double e2 = 0.0067192188;
   double c = 6398786.849;

   double bI = GKHeight/10000855.7646;
   double bII = bI*bI;
   double bf = 325632.08677 * bI * ((((((0.00000562025 * bII + 0.00022976983)
                                  * bII - 0.00113566119)
                                  * bII + 0.00424914906)
                                  * bII - 0.00831729565)
                                  * bII + 1));
   bf /= 3600*rho;
   double co = cos(bf);
   double g2 = e2 *(co*co);
   double g1 = c/sqrt(1+g2);
   double t = tan(bf);
   double fa = (GKRight - floor(GKRight/1000000)*1000000-500000)/g1;
   double GeoDezRight = ((bf - fa * fa * t * (1 + g2) / 2 +
                          fa * fa * fa * fa * t *
                         (5 + 3 * t * t + 6 * g2 - 6 * g2 * t * t) / 24) *
                         rho);
   double dl = fa - fa * fa * fa * (1 + 2 * t * t + g2) / 6 +
               fa * fa * fa * fa * fa *
               (1 + 28 * t * t + 24 * t * t * t * t) / 120;

   double Mer = floor(GKRight/1000000);
   double GeoDezHeight = dl*rho/co+Mer*3;
   if(useWGS){
      return bessel2WGS(GeoDezRight,GeoDezHeight,result);
   }else{
      result.SetDefined(true);
      result.Set(GeoDezHeight,GeoDezRight);
      return result.IsDefined();
   }
}

bool  WGSGK::bessel2WGS(const double geoDezRight1,
                        const double geoDezHeight1, Point& result) const{
    double aBessel = abes;
    double eeBessel = 0.0066743722296294277832;
    double ScaleFactor = 0.00000982;
    double RotXRad = -7.16069806998785E-06;
    double RotYRad = 3.56822869296619E-07;
    double RotZRad = 7.06858347057704E-06;
    double ShiftXMeters = 591.28;
    double ShiftYMeters = 81.35;
    double ShiftZMeters = 396.39;
    double aWGS84 = awgs;
    double eeWGS84 = 0.0066943799;
    double geoDezRight = (geoDezRight1/180)*Pi;
    double geoDezHeight = (geoDezHeight1/180)*Pi;
    double sinRight = sin(geoDezRight);
    double sinRight2 = sinRight*sinRight;
    double n = eeBessel*sinRight2;
    n = 1-n;
    n = sqrt(n);
    n = aBessel/n;
    double cosRight=cos(geoDezRight);
    double cosHeight=cos(geoDezHeight);
    double sinHeight = sin(geoDezHeight);
    double CartesianXMeters = n*cosRight*cosHeight;
    double CartesianYMeters = n*cosRight*sinHeight;
    double CartesianZMeters = n*(1-eeBessel)*sinRight;

    double CartOutputXMeters = (1 + ScaleFactor) *
                               CartesianXMeters + RotZRad *
                               CartesianYMeters -
                               RotYRad * CartesianZMeters + ShiftXMeters;
    double CartOutputYMeters = -RotZRad * CartesianXMeters +
                               (1 + ScaleFactor) * CartesianYMeters +
                               RotXRad * CartesianZMeters + ShiftYMeters;
    double CartOutputZMeters = RotYRad * CartesianXMeters -
                               RotXRad * CartesianYMeters +
                               (1 + ScaleFactor) * CartesianZMeters +
                               ShiftZMeters;

     geoDezHeight = atan(CartOutputYMeters/CartOutputXMeters);
     double Latitude = (CartOutputXMeters*CartOutputXMeters)+
                       (CartOutputYMeters*CartOutputYMeters);
     Latitude = sqrt(Latitude);
     double InitLat = Latitude;
     Latitude = CartOutputZMeters/Latitude;
     Latitude = atan(Latitude);
     double LatitudeIt = 99999999;
     do{
        LatitudeIt = Latitude;
        double sinLat = sin(Latitude);
        n = 1-eeWGS84*sinLat*sinLat;
        n = sqrt(n);
        n = aWGS84/n;
        Latitude = InitLat; // sqrt(CartoutputXMeters^2+CartOutputYMeters^2)
        Latitude = (CartOutputZMeters+eeWGS84*n*sin(LatitudeIt))/Latitude;
        Latitude = atan(Latitude);
     } while(abs(Latitude-LatitudeIt)>=0.000000000000001);

     result.SetDefined(true);
     result.Set((geoDezHeight/Pi)*180,  (Latitude/Pi)*180);
     return result.IsDefined();
}



/*

8.2 List Representation

The list representation of a region is

----  (face1  face2  face3 ... )
                 where facei=(outercycle, holecycle1, holecycle2....)

  cyclei= (vertex1, vertex2,  .....)
                where each vertex is a point.

  or

  undef
----






8.3 ~Out~-function

*/
ListExpr
OutRegion( ListExpr typeInfo, Word value )
{
  Region* cr = (Region*)(value.addr);
  if(!cr->IsDefined()){
    return nl->SymbolAtom(Symbol::UNDEFINED());
  }

  if( cr->IsEmpty() )
  {
    return (nl->TheEmptyList());
  }
  else
  {
    Region *RCopy=new Region(*cr, true); // in memory

    RCopy->LogicSort();

    HalfSegment hs, hsnext;

    ListExpr regionNL = nl->TheEmptyList();
    ListExpr regionNLLast = regionNL;

    ListExpr faceNL = nl->TheEmptyList();
    ListExpr faceNLLast = faceNL;

    ListExpr cycleNL = nl->TheEmptyList();
    ListExpr cycleNLLast = cycleNL;

    ListExpr pointNL;

    int currFace = -999999, currCycle= -999999; // avoid uninitialized use
    Point outputP(true), leftoverP(true);

    for( int i = 0; i < RCopy->Size(); i++ )
    {
      RCopy->Get( i, hs );
      if (i==0)
      {
        currFace = hs.attr.faceno;
        currCycle = hs.attr.cycleno;
        RCopy->Get( i+1, hsnext );

        if ((hs.GetLeftPoint() == hsnext.GetLeftPoint()) ||
            ((hs.GetLeftPoint() == hsnext.GetRightPoint())))
        {
          outputP = hs.GetRightPoint();
          leftoverP = hs.GetLeftPoint();
        }
        else if ((hs.GetRightPoint() == hsnext.GetLeftPoint()) ||
                 ((hs.GetRightPoint() == hsnext.GetRightPoint())))
        {
          outputP = hs.GetLeftPoint();
          leftoverP = hs.GetRightPoint();
        }
        else
        {
          cerr << "\n" << __PRETTY_FUNCTION__ << ": Wrong data format --- "
               << "discontiguous segments!" << endl
               << "\ths     = " << hs     << endl
               << "\thsnext = " << hsnext << endl;
          return nl->SymbolAtom(Symbol::UNDEFINED());
        }

        pointNL = OutPoint( nl->TheEmptyList(), SetWord(&outputP) );
        if (cycleNL == nl->TheEmptyList())
        {
          cycleNL = nl->OneElemList(pointNL);
          cycleNLLast = cycleNL;
        }
        else
        {
          cycleNLLast = nl->Append( cycleNLLast, pointNL );
        }
      }
      else
      {
        if (hs.attr.faceno == currFace)
        {
          if (hs.attr.cycleno == currCycle)
          {
            outputP=leftoverP;

            if (hs.GetLeftPoint() == leftoverP)
              leftoverP = hs.GetRightPoint();
            else if (hs.GetRightPoint() == leftoverP)
            {
              leftoverP = hs.GetLeftPoint();
            }
            else
            {
              cerr << "\n" << __PRETTY_FUNCTION__ << ": Wrong data format --- "
                  << "discontiguous segment in cycle!" << endl
                  << "\thh        = " << hs << endl
                  << "\tleftoverP = " << leftoverP << endl;
              return nl->SymbolAtom(Symbol::UNDEFINED());
            }

            pointNL=OutPoint( nl->TheEmptyList(),
                              SetWord( &outputP) );
            if (cycleNL == nl->TheEmptyList())
            {
              cycleNL=nl->OneElemList(pointNL);
              cycleNLLast = cycleNL;
            }
            else
            {
              cycleNLLast = nl->Append(cycleNLLast, pointNL);
            }
          }
          else
          {
            if (faceNL == nl->TheEmptyList())
            {
              faceNL = nl->OneElemList(cycleNL);
              faceNLLast = faceNL;
            }
            else
            {
              faceNLLast = nl->Append(faceNLLast, cycleNL);
            }
            cycleNL = nl->TheEmptyList();
            currCycle = hs.attr.cycleno;


            RCopy->Get( i+1, hsnext );
            if ((hs.GetLeftPoint() == hsnext.GetLeftPoint()) ||
                ((hs.GetLeftPoint() == hsnext.GetRightPoint())))
            {
              outputP = hs.GetRightPoint();
              leftoverP = hs.GetLeftPoint();
            }
            else if ((hs.GetRightPoint() == hsnext.GetLeftPoint()) ||
                     ((hs.GetRightPoint() == hsnext.GetRightPoint())))
            {
              outputP = hs.GetLeftPoint();
              leftoverP = hs.GetRightPoint();
            }
            else
            {
              cerr << "\n" << __PRETTY_FUNCTION__ << ": Wrong data format --- "
                  << "discontiguous segments in cycle!" << endl
                  << "\ths     = " << hs     << endl
                  << "\thsnext = " << hsnext << endl;
              return nl->SymbolAtom(Symbol::UNDEFINED());
            }

            pointNL = OutPoint( nl->TheEmptyList(),
                                SetWord(&outputP) );
            if (cycleNL == nl->TheEmptyList())
            {
              cycleNL = nl->OneElemList(pointNL);
              cycleNLLast = cycleNL;
            }
            else
            {
              cycleNLLast = nl->Append(cycleNLLast, pointNL);
            }
          }
        }
        else
        {
          if (faceNL == nl->TheEmptyList())
          {
            faceNL = nl->OneElemList(cycleNL);
            faceNLLast = faceNL;
          }
          else
          {
            faceNLLast = nl->Append(faceNLLast, cycleNL);
          }
          cycleNL = nl->TheEmptyList();


          if (regionNL == nl->TheEmptyList())
          {
            regionNL = nl->OneElemList(faceNL);
            regionNLLast = regionNL;
          }
          else
          {
            regionNLLast = nl->Append(regionNLLast, faceNL);
          }
          faceNL = nl->TheEmptyList();

          currFace = hs.attr.faceno;
          currCycle = hs.attr.cycleno;


          RCopy->Get( i+1, hsnext );
          if ((hs.GetLeftPoint() == hsnext.GetLeftPoint()) ||
             ((hs.GetLeftPoint() == hsnext.GetRightPoint())))
          {
            outputP = hs.GetRightPoint();
            leftoverP = hs.GetLeftPoint();
          }
          else if ((hs.GetRightPoint() == hsnext.GetLeftPoint()) ||
                  ((hs.GetRightPoint() == hsnext.GetRightPoint())))
          {
            outputP = hs.GetLeftPoint();
            leftoverP = hs.GetRightPoint();
          }
          else
          {
            cerr << "\n" << __PRETTY_FUNCTION__ << ": Wrong data format --- "
                << "discontiguous segments in cycle!" << endl
                << "\ths     = " << hs     << endl
                << "\thsnext = " << hsnext << endl;
            return nl->SymbolAtom(Symbol::UNDEFINED());
          }

          pointNL = OutPoint(nl->TheEmptyList(), SetWord(&outputP));
          if (cycleNL == nl->TheEmptyList())
          {
            cycleNL = nl->OneElemList(pointNL);
            cycleNLLast = cycleNL;
          }
          else
          {
            cycleNLLast = nl->Append(cycleNLLast, pointNL);
          }
        }
      }
    }

    if (faceNL == nl->TheEmptyList())
    {
      faceNL = nl->OneElemList(cycleNL);
      faceNLLast = faceNL;
    }
    else
    {
      faceNLLast = nl->Append(faceNLLast, cycleNL);
    }
    cycleNL = nl->TheEmptyList();


    if (regionNL == nl->TheEmptyList())
    {
      regionNL = nl->OneElemList(faceNL);
      regionNLLast = regionNL;
    }
    else
    {
      regionNLLast = nl->Append(regionNLLast, faceNL);
    }
    faceNL = nl->TheEmptyList();

    RCopy->DeleteIfAllowed();
    return regionNL;
  }
}

/*
8.4 ~In~-function

*/


Word
InRegion(const ListExpr typeInfo, const ListExpr instance,
           const int errorPos, ListExpr& errorInfo, bool& correct ){


  if(listutils::isSymbol(instance,Symbol::UNDEFINED())){
    Region* r = new Region(0);
    r->SetDefined(false);
    correct = true;
    return SetWord(Address(r));
  }


  if(nl->AtomType(instance) != NoAtom){
    correct = false;
    return SetWord(Address(0));
  }

  ListExpr regNL = instance;
  vector<vector<Point> > cycles;

  while(!nl->IsEmpty(regNL)){
    ListExpr faceNL = nl->First(regNL);
    regNL = nl->Rest(regNL);
    if(nl->AtomType(faceNL)!=NoAtom){
      correct = false;
      return SetWord(Address(0));
    }
    bool firstCycle = true;
    Rectangle<2> faceRect;
    while(!nl->IsEmpty(faceNL)){
      vector<Point> cycle;
      ListExpr cycleNL = nl->First(faceNL);
      faceNL = nl->Rest(faceNL);
      if(nl->AtomType(cycleNL)!=NoAtom){
         correct=false;
         return SetWord(Address(0));
      }

      Point firstPoint(false,0,0);
      bool fp = true;
      Rectangle<2> currentBB;
      while(!nl->IsEmpty(cycleNL)){
         ListExpr pointNL = nl->First(cycleNL);
         cycleNL = nl->Rest(cycleNL);
         if(nl->ListLength(pointNL)!=2){
           correct = false;
           return SetWord(Address(0));
         }
         if(!listutils::isNumeric(nl->First(pointNL)) ||
            !listutils::isNumeric(nl->Second(pointNL))){
            correct = false;
            return SetWord(Address(0));
         }
         Point p(true, listutils::getNumValue(nl->First(pointNL)),
                       listutils::getNumValue(nl->Second(pointNL)));
         cycle.push_back(p);
         if(fp){
            fp = false;
            firstPoint = p;
            currentBB = p.BoundingBox();
         } else {
            currentBB = currentBB.Union(p.BoundingBox());
         }
      }
      if(!AlmostEqual(firstPoint, cycle[cycle.size()-1])){
        cycle.push_back(firstPoint);
      }
      if(firstCycle || !faceRect.Contains(currentBB)){
        if(!getDir(cycle)){
           reverseCycle(cycle);
        }
        firstCycle=false;
        faceRect = currentBB;
      } else {
        if(getDir(cycle) ){
           reverseCycle(cycle);
        }

      }

      cycles.push_back(cycle);
    }
  }

  Region* res = buildRegion(cycles);
  correct = res!=0;
  return SetWord(res);
}


Word
InRegion_old( const ListExpr typeInfo, const ListExpr instance,
          const int errorPos, ListExpr& errorInfo, bool& correct )
{

  Region* cr = new Region( 0 );

  if(listutils::isSymbolUndefined(instance)){
    cr->SetDefined(0);
    correct=true;
    return SetWord(Address(cr));
  }

  cr->StartBulkLoad();


  ListExpr RegionNL = instance;
  ListExpr FaceNL, CycleNL;
  int fcno=-1;
  int ccno=-1;
  int edno=-1;
  int partnerno = 0;

  if (!nl->IsAtom(instance))
  {
    while( !nl->IsEmpty( RegionNL ) )
    {
      FaceNL = nl->First( RegionNL );
      RegionNL = nl->Rest( RegionNL);
      bool isCycle = true;

      //A face is composed by 1 cycle, and can have holes.
      //All the holes must be inside the face. (TO BE IMPLEMENTED0)
      //Region *faceCycle;

      fcno++;
      ccno=-1;
      edno=-1;

      if (nl->IsAtom( FaceNL ))
      {
        correct=false;
        return SetWord( Address(0) );
      }

      while (!nl->IsEmpty( FaceNL) )
      {
        CycleNL = nl->First( FaceNL );
        FaceNL = nl->Rest( FaceNL );

        ccno++;
        edno=-1;

        if (nl->IsAtom( CycleNL ))
        {
          correct=false;
          return SetWord( Address(0) );
        }

        if (nl->ListLength( CycleNL) <3)
        {
          cerr << __PRETTY_FUNCTION__ << ": A cycle must have at least 3 edges!"
               << endl;
          correct=false;
          return SetWord( Address(0) );
        }
        else
        {
          ListExpr firstPoint = nl->First( CycleNL );
          ListExpr prevPoint = nl->First( CycleNL );
          ListExpr flagedSeg = nl->TheEmptyList();
          ListExpr currPoint = nl->TheEmptyList();
          CycleNL = nl->Rest( CycleNL );

          //Starting to compute a new cycle

          Points *cyclepoints= new Points( 8 ); // in memory

          Point *currvertex,p1(true),p2(true),firstP(true);

          //This function has the goal to store the half segments of
          //the cycle that is been treated. When the cycle's computation
          //is terminated the region rDir will be used to compute the
          //insideAbove
          //attribute of the half segments of this cycle.
          Region *rDir = new Region(32);
          rDir->StartBulkLoad();


          currvertex = (Point*) InPoint ( nl->TheEmptyList(),
              firstPoint, 0, errorInfo, correct ).addr;
          if (!correct) {
             // todo: delete temp objects
             return SetWord( Address(0) );
          }

          cyclepoints->StartBulkLoad();
          (*cyclepoints) += (*currvertex);
          p1 = *currvertex;
          firstP = p1;
          cyclepoints->EndBulkLoad();
          delete currvertex;

          while ( !nl->IsEmpty( CycleNL) )
          {
//            cout<<"cycle "<<endl;
            currPoint = nl->First( CycleNL );
            CycleNL = nl->Rest( CycleNL );

            currvertex = (Point*) InPoint( nl->TheEmptyList(),
                  currPoint, 0, errorInfo, correct ).addr;
//            cout<<"curvertex "<<*currvertex<<endl;
            if (!correct) return SetWord( Address(0) );

            if (cyclepoints->Contains(*currvertex))
            {
              cerr<< __PRETTY_FUNCTION__ << ": The same vertex: "
                  <<(*currvertex)
                  <<" appears repeatedly within the current cycle!"<<endl;
              correct=false;
              return SetWord( Address(0) );
            }
            else
            {
              p2 = *currvertex;
              cyclepoints->StartBulkLoad();
              (*cyclepoints) += (*currvertex);
              cyclepoints->EndBulkLoad(true,false,false);
            }
            delete currvertex;

            flagedSeg = nl->TwoElemList
            (nl-> BoolAtom(true),
             nl->TwoElemList(prevPoint, currPoint));
            prevPoint=currPoint;
            edno++;
            //Create left dominating half segment
            HalfSegment * hs = (HalfSegment*)InHalfSegment
                      ( nl->TheEmptyList(), flagedSeg,
                       0, errorInfo, correct ).addr;
            if(!correct){
              if(hs){
                cerr << __PRETTY_FUNCTION__ << ": Creation of left dominating "
                     << "half segment (1) failed!" << endl;
                delete hs;
              }
              cr->DeleteIfAllowed();
              return SetWord( Address(0) );
            }
            hs->attr.faceno=fcno;
            hs->attr.cycleno=ccno;
            hs->attr.edgeno=edno;
            hs->attr.partnerno=partnerno;
            partnerno++;
            hs->attr.insideAbove = (hs->GetLeftPoint() == p1);
              //true (L-->R ),false (R--L)
            p1 = p2;

            if (( correct )&&( cr->InsertOk(*hs) ))
            {
              (*cr) += (*hs);
//              cout<<"cr+1 "<<*hs<<endl;
              if( hs->IsLeftDomPoint() )
              {
                (*rDir) += (*hs);
//                cout<<"rDr+1 "<<*hs<<endl;
                hs->SetLeftDomPoint( false );
              }
              else
              {
                hs->SetLeftDomPoint( true );
//                cout<<"rDr+2 "<<*hs<<endl;
                (*rDir) += (*hs);
              }
              (*cr) += (*hs);
//              cout<<"cr+2 "<<*hs<<endl;
              delete hs;
            }
            else
            {
              cerr<< __PRETTY_FUNCTION__ << ": Problematic HalfSegment: "
                  << endl;
              if(correct)
                cerr << "\nhs = " << (*hs) << " cannot be inserted." << endl;
              else
                cerr << "\nInvalid half segment description." << endl;
              correct=false;
              return SetWord( Address(0) );
            }

          }
          delete cyclepoints;

          edno++;
          flagedSeg= nl->TwoElemList
            (nl-> BoolAtom(true),
             nl->TwoElemList(firstPoint, currPoint));
          HalfSegment * hs = (HalfSegment*)InHalfSegment
                  ( nl->TheEmptyList(), flagedSeg,
                    0, errorInfo, correct ).addr;
          if(!correct){
            if(hs){
                cerr << __PRETTY_FUNCTION__ << ": Creation of "
                     << "half segment (2) failed!" << endl;
                delete hs;
            }
            cr->DeleteIfAllowed();
            return SetWord( Address(0) );
          }
          hs->attr.faceno=fcno;
          hs->attr.cycleno=ccno;
          hs->attr.edgeno=edno;
          hs->attr.partnerno=partnerno;
          hs->attr.insideAbove = (hs->GetRightPoint() == firstP);
          //true (L-->R ),false (R--L),
          //the order of typing is last point than first point.
          partnerno++;

          //The last half segment of the region
          if (( correct )&&( cr->InsertOk(*hs) ))
          {
            (*cr) += (*hs);
//             cout<<"cr+3 "<<*hs<<endl;
            if( hs->IsLeftDomPoint() )
            {
              (*rDir) += (*hs);
//              cout<<"rDr+3 "<<*hs<<endl;
              hs->SetLeftDomPoint( false );
            }
            else
            {
              hs->SetLeftDomPoint( true );
//              cout<<"rDr+4 "<<*hs<<endl;
              (*rDir) += (*hs);
            }
            (*cr) += (*hs);
//            cout<<"cr+4 "<<*hs<<endl;
            delete hs;
            rDir->EndBulkLoad(true, false, false, false);


            //To calculate the inside above attribute
            bool direction = rDir->GetCycleDirection();

            int h = cr->Size() - ( rDir->Size() * 2 );
            while ( h < cr->Size())
            {
              //after each left half segment of the region is its
              //correspondig right half segment
              HalfSegment hsIA;
              bool insideAbove;
              cr->Get(h,hsIA);
              /*
                The test for adjusting the inside above can be described
                as above, but was implemented in a different way that
                produces the same result.
                if ( (direction  && hsIA->attr.insideAbove) ||
                     (!direction && !hsIA->attr.insideAbove) )
                {
                  //clockwise and l-->r or
                  //counterclockwise and r-->l
                  hsIA->attr.insideAbove=false;
                }
                else
                  //clockwise and r-->r or
                  //counterclockwise and l-->r
                  true;

              */
              if (direction == hsIA.attr.insideAbove)
                insideAbove = false;
              else
                insideAbove = true;
              if (!isCycle)
                insideAbove = !insideAbove;
              HalfSegment auxhsIA( hsIA );
              auxhsIA.attr.insideAbove = insideAbove;
              cr->UpdateAttr(h,auxhsIA.attr);
              //Get right half segment
              cr->Get(h+1,hsIA);
              auxhsIA = hsIA;
              auxhsIA.attr.insideAbove = insideAbove;
              cr->UpdateAttr(h+1,auxhsIA.attr);
              h+=2;
            }

            //After the first face's cycle read the faceCycle variable is set.
            //Afterwards
            //it is tested if all the new cycles are inside the faceCycle.
            /*
            if (isCycle)
              faceCycle = new Region(rDir,false);
            else
              //To implement the test
            */
            rDir->DeleteIfAllowed();
            //After the end of the first cycle of the face,
            //all the following cycles are
            //holes, then isCycle is set to false.
            isCycle = false;

          }
          else
          {
            correct=false;
            return SetWord( Address(0) );
          }
        }
      }
    }

    cr->SetNoComponents( fcno+1 );
    cr->EndBulkLoad( true, true, true, false );

    correct = true;
    return SetWord( cr );
  }
  else
  {
    correct=false;
    return SetWord( Address(0) );
  }
}

/*
8.5 ~Create~-function

*/
Word
CreateRegion( const ListExpr typeInfo )
{
  //cout << "CreateRegion" << endl;

  return (SetWord( new Region( 0 ) ));
}

/*
8.6 ~Delete~-function

*/
void
DeleteRegion( const ListExpr typeInfo, Word& w )
{
  //cout << "DeleteRegion" << endl;

  Region *cr = (Region *)w.addr;
  cr->Destroy();
  cr->DeleteIfAllowed(false);
  w.addr = 0;
}

/*
8.7 ~Close~-function

*/
void
CloseRegion( const ListExpr typeInfo, Word& w )
{
  //cout << "CloseRegion" << endl;

  ((Region *)w.addr)->DeleteIfAllowed();
  w.addr = 0;
}

/*
8.8 ~Clone~-function

*/
Word
CloneRegion( const ListExpr typeInfo, const Word& w )
{
  //cout << "CloneRegion" << endl;

  Region *cr = new Region( *((Region *)w.addr) );
  return SetWord( cr );
}

/*
7.8 ~Open~-function

*/
bool
OpenRegion( SmiRecord& valueRecord,
            size_t& offset,
            const ListExpr typeInfo,
            Word& value )
{
  Region *r = (Region*)Attribute::Open( valueRecord, offset, typeInfo );
  value = SetWord( r );
  return true;
}

/*
7.8 ~Save~-function

*/
bool
SaveRegion( SmiRecord& valueRecord,
            size_t& offset,
            const ListExpr typeInfo,
            Word& value )
{
  Region *r = (Region *)value.addr;
  Attribute::Save( valueRecord, offset, typeInfo, r );
  return true;
}

/*
8.9 ~SizeOf~-function

*/
int SizeOfRegion()
{
  return sizeof(Region);
}

/*
8.11 Function describing the signature of the type constructor

*/
ListExpr
RegionProperty()
{
  ListExpr listreplist = nl->TextAtom();
  nl->AppendText(listreplist,"(<face>*) where face is"
                             " (<outercycle><holecycle>*); "
  "<outercycle> and <holecycle> are <points>*");
  ListExpr examplelist = nl->TextAtom();
  nl->AppendText(examplelist,"(((3 0)(10 1)(3 1))((3.1 0.1)"
           "(3.1 0.9)(6 0.8)))");
  ListExpr remarkslist = nl->TextAtom();
  nl->AppendText(remarkslist,"all <holecycle> must be completely within "
  "<outercycle>.");

  return (nl->TwoElemList(
            nl->FiveElemList(nl->StringAtom("Signature"),
                       nl->StringAtom("Example Type List"),
           nl->StringAtom("List Rep"),
           nl->StringAtom("Example List"),
           nl->StringAtom("Remarks")),
            nl->FiveElemList(nl->StringAtom("-> DATA"),
                       nl->StringAtom(Region::BasicType()),
           listreplist,
           examplelist,
           remarkslist)));
}

/*
8.12 Kind checking function

This function checks whether the type constructor is applied correctly. Since
type constructor ~point~ does not have arguments, this is trivial.

*/
bool
CheckRegion( ListExpr type, ListExpr& errorInfo )
{
  return (nl->IsEqual( type, Region::BasicType() ));
}


/*
8.14 Creation of the type constructor instance

*/
TypeConstructor region(
        Region::BasicType(),                       //name
        RegionProperty,                 //describing signature
        OutRegion,      InRegion,       //Out and In functions
        0,              0,              //SaveTo and RestoreFrom List functions
        CreateRegion,   DeleteRegion,   //object creation and deletion
        OpenRegion,     SaveRegion,     // object open and save
        CloseRegion,    CloneRegion,    //object close and clone
        Region::Cast,                     //cast function
        SizeOfRegion,                   //sizeof function
        CheckRegion );                  //kind checking function


/*
8.15 type constructor for label data type

*/
GenTC<Label> label;

/*
8.16 type contructor for disc

*/
GenTC<Disc> disc;

/*
8.17 Type constructor for Segment

*/
GenTC<Segment> segment;



/*
10 Operators

Definition of operators is similar to definition of type constructors. An
operator is defined by creating an instance of class ~Operator~. Again we
have to define some functions before we are able to create an ~Operator~
instance.

10.1 Type mapping functions

A type mapping function takes a nested list as argument. Its contents are
type descriptions of an operator's input parameters. A nested list describing
the output type of the operator is returned.

10.1.1 Type mapping function SpatialTypeMapBool

It is for the compare operators which have ~bool~ as resulttype, like =, !=, <,
<=, >, >=.

*/
ListExpr
SpatialTypeMapBool( ListExpr args )
{
  ListExpr arg1, arg2;
  if ( nl->ListLength( args ) == 2 )
  {
    arg1 = nl->First( args );
    arg2 = nl->Second( args );
    if ( SpatialTypeOfSymbol( arg1 ) == stpoint &&
         SpatialTypeOfSymbol( arg2 ) == stpoint)
      return (nl->SymbolAtom( CcBool::BasicType() ));
    if ( SpatialTypeOfSymbol( arg1 ) == stpoints &&
         SpatialTypeOfSymbol( arg2 ) == stpoints)
      return (nl->SymbolAtom( CcBool::BasicType() ));
    if ( SpatialTypeOfSymbol( arg1 ) == stline &&
         SpatialTypeOfSymbol( arg2 ) == stline)
      return (nl->SymbolAtom( CcBool::BasicType() ));
    if ( SpatialTypeOfSymbol( arg1 ) == stregion &&
         SpatialTypeOfSymbol( arg2 ) == stregion)
      return (nl->SymbolAtom( CcBool::BasicType() ));
    if ( SpatialTypeOfSymbol( arg1 ) == stpoint &&
         SpatialTypeOfSymbol( arg2 ) == stpoints)
      return (nl->SymbolAtom( CcBool::BasicType() ));
    if ( SpatialTypeOfSymbol( arg1 ) == stpoints &&
         SpatialTypeOfSymbol( arg2 ) == stpoint)
      return (nl->SymbolAtom( CcBool::BasicType() ));
    if ( SpatialTypeOfSymbol( arg1 ) == stpoint &&
         SpatialTypeOfSymbol( arg2 ) == stline)
      return (nl->SymbolAtom( CcBool::BasicType() ));
    if ( SpatialTypeOfSymbol( arg1 ) == stpoint &&
         SpatialTypeOfSymbol( arg2 ) == stregion)
      return (nl->SymbolAtom( CcBool::BasicType() ));
  }
  return listutils::typeError("");
}


ListExpr SpatialTypeMapCompare(ListExpr args){
   string err = " st x st expected, with"
                " st in {point, points, line, region, sline)";
   if(!nl->HasLength(args,2)){
     return listutils::typeError(err);
   }
   ListExpr arg1 = nl->First(args);
   ListExpr arg2 = nl->Second(args);
   if(!nl->Equal(arg1,arg2)){
     return listutils::typeError(err);
   }
   SpatialType st = SpatialTypeOfSymbol(arg1);
   if( st ==  stpoint ||
       st == stpoints ||
       st == stline   ||
       st == stregion ||
       st == stsline){
     return nl->SymbolAtom(CcBool::BasicType());
   }
   return listutils::typeError(err);
}

ListExpr SpatialTypeMapEqual(ListExpr args){
  string err = "spatial x spatial expected";
  if(nl->ListLength(args)!=2){
    return listutils::typeError(err + " wrong number of arguments");
  }
  ListExpr arg1 = nl->First(args);
  ListExpr arg2 = nl->Second(args);
  if(!listutils::isSymbol(arg1) || !listutils::isSymbol(arg2)){
    return listutils::typeError(err + " composite type detected");
  }
  string s1 = nl->SymbolValue(arg1);
  string s2 = nl->SymbolValue(arg2);
  if(s1==s2){
    if( (s1==Point::BasicType()) ||
        (s1==Points::BasicType()) ||
        (s1==Line::BasicType()) ||
        (s1==Region::BasicType()) ||
        (s1==SimpleLine::BasicType())){
      return nl->SymbolAtom(CcBool::BasicType());
    }
    return listutils::typeError(err + " (only spatial types allowed");
  }
  if( (s1==Point::BasicType()) && (s2==Points::BasicType())){
    return nl->SymbolAtom(CcBool::BasicType());
  }
  if( (s1==Points::BasicType()) && (s2==Point::BasicType())){
    return nl->SymbolAtom(CcBool::BasicType());
  }
  return listutils::typeError(err + " (only spatial types allowed");
}

ListExpr SpatialTypeMapIsLess(ListExpr args){
  string err = "point x point expected";
  if(nl->ListLength(args)!=2){
    return listutils::typeError(err + " wrong number of arguments");
  }
  ListExpr arg1 = nl->First(args);
  ListExpr arg2 = nl->Second(args);
  if(!listutils::isSymbol(arg1) || !listutils::isSymbol(arg2)){
    return listutils::typeError(err + " composite type detected");
  }
  string s1 = nl->SymbolValue(arg1);
  string s2 = nl->SymbolValue(arg2);
  if(s1==s2 &&  s1==Point::BasicType())
    return nl->SymbolAtom(CcBool::BasicType());
  else
    return listutils::typeError(err);
}

/*
10.1.2 Type mapping function GeoGeoMapBool

It is for the binary operators which have ~bool~ as result type, such as
interscets, inside, onborder, ininterior, etc.

*/

ListExpr
GeoGeoMapBool( ListExpr args )
{
  ListExpr arg1, arg2;
  if ( nl->ListLength( args ) == 2 ){
    arg1 = nl->First( args );
    arg2 = nl->Second( args );
    if (((SpatialTypeOfSymbol( arg1 ) == stpoint)  ||
         (SpatialTypeOfSymbol( arg1 ) == stpoints) ||
         (SpatialTypeOfSymbol( arg1 ) == stline)     ||
         (SpatialTypeOfSymbol( arg1 ) == stregion)) &&
        ((SpatialTypeOfSymbol( arg2 ) == stpoint)  ||
         (SpatialTypeOfSymbol( arg2 ) == stpoints) ||
         (SpatialTypeOfSymbol( arg2 ) == stline)     ||
         (SpatialTypeOfSymbol( arg2 ) == stregion))){
      return (nl->SymbolAtom( CcBool::BasicType() ));
    }
  }
  return listutils::typeError("");
}

/*
10.1.2 Type Mapping setsetmapbool

It is for all combinations of spatial types which are represented as
sets, .i.e all types except point.

*/

ListExpr
IntersectsTM( ListExpr args )
{
  ListExpr arg1, arg2;
  if ( nl->ListLength( args ) == 2 )
  {
    arg1 = nl->First( args );
    arg2 = nl->Second( args );
    SpatialType st1 = SpatialTypeOfSymbol( arg1 );
    SpatialType st2 = SpatialTypeOfSymbol( arg2 );
    if ( ((st1 == stpoints) || (st1 == stline) || (st1 == stregion)) &&
         ((st2 == stpoints) || (st2 == stline) || (st2 == stregion))) {
      return (nl->SymbolAtom( CcBool::BasicType() ));
    } else if(st1==stsline && st2==stsline) {
      return (nl->SymbolAtom( CcBool::BasicType() ));
    }
  }
  return listutils::typeError(" t_1 x t_2 expected,"
                              " with t_1, t_2 in {points,line,region}");
}

/*
10.1.2 PointsRegionMapBool

*/
ListExpr
PointsRegionMapBool( ListExpr args )
{
  ListExpr arg1, arg2;
  if ( nl->ListLength( args ) == 2 ){
    arg1 = nl->First( args );
    arg2 = nl->Second( args );
    if (((SpatialTypeOfSymbol( arg1 ) == stpoints) ) &&
        ((SpatialTypeOfSymbol( arg2 ) == stregion))){
      return (nl->SymbolAtom( CcBool::BasicType() ));
    }
  }
  return listutils::typeError("Expected points x region.");
}

/*
10.1.2 RegionRegionMapBool

*/
ListExpr
RegionRegionMapBool( ListExpr args )
{
  ListExpr arg1, arg2;
  if ( nl->ListLength( args ) == 2 ) {
    arg1 = nl->First( args );
    arg2 = nl->Second( args );
    if (((SpatialTypeOfSymbol( arg1 ) == stregion) ) &&
        ((SpatialTypeOfSymbol( arg2 ) == stregion))){
      return (nl->SymbolAtom( CcBool::BasicType() ));
    }
  }
  return listutils::typeError("region x region expected");
}

/*
10.1.2 PointRegionMapBool

*/
ListExpr
PointRegionMapBool( ListExpr args )
{
  ListExpr arg1, arg2;
  if ( nl->ListLength( args ) == 2 )
  {
    arg1 = nl->First( args );
    arg2 = nl->Second( args );
    if (((SpatialTypeOfSymbol( arg1 ) == stpoint) ) &&
        ((SpatialTypeOfSymbol( arg2 ) == stregion))){
      return (nl->SymbolAtom( CcBool::BasicType() ));
    }
  }
  return listutils::typeError("points x region expected");
}


/*
10.1.2 AdjacentTypeMap

*/
ListExpr
AdjacentTypeMap( ListExpr args )
{
  ListExpr arg1, arg2;
  if ( nl->ListLength( args ) == 2 ){
    arg1 = nl->First( args );
    arg2 = nl->Second( args );
    if(
      ((SpatialTypeOfSymbol(arg1)==stpoints) &&
       (SpatialTypeOfSymbol(arg2)==stregion)) ||
      ((SpatialTypeOfSymbol(arg1)==stline) &&
       (SpatialTypeOfSymbol(arg2)==stregion)) ||
      ((SpatialTypeOfSymbol(arg1)==stregion) &&
       (SpatialTypeOfSymbol(arg2)==stpoints)) ||
      ((SpatialTypeOfSymbol(arg1)==stregion) &&
       (SpatialTypeOfSymbol(arg2)==stline)) ||
      ((SpatialTypeOfSymbol(arg1)==stregion) &&
       (SpatialTypeOfSymbol(arg2)==stregion))){
      return nl->SymbolAtom(CcBool::BasicType());
    }
  }
  return listutils::typeError("points x region expected");
}


/*
10.1.2 InsideTypeMap

*/
ListExpr
InsideTypeMap( ListExpr args )
{
  ListExpr arg1, arg2;
  if ( nl->ListLength( args ) == 2 )
  {
    arg1 = nl->First( args );
    arg2 = nl->Second( args );
    if(
      ((SpatialTypeOfSymbol(arg1)==stpoint) &&
       (SpatialTypeOfSymbol(arg2)==stpoints)) ||

      ((SpatialTypeOfSymbol(arg1)==stpoint) &&
       (SpatialTypeOfSymbol(arg2)==stline)) ||

      ((SpatialTypeOfSymbol(arg1)==stpoint) &&
       (SpatialTypeOfSymbol(arg2)==stregion)) ||

      ((SpatialTypeOfSymbol(arg1)==stpoints) &&
       (SpatialTypeOfSymbol(arg2)==stpoints)) ||

      ((SpatialTypeOfSymbol(arg1)==stpoints) &&
       (SpatialTypeOfSymbol(arg2)==stline))||

      ((SpatialTypeOfSymbol(arg1)==stpoints) &&
       (SpatialTypeOfSymbol(arg2)==stregion)) ||

      ((SpatialTypeOfSymbol(arg1)==stline) &&
       (SpatialTypeOfSymbol(arg2)==stline)) ||

      ((SpatialTypeOfSymbol(arg1)==stline) &&
       (SpatialTypeOfSymbol(arg2)==stregion)) ||

      ((SpatialTypeOfSymbol(arg1)==stregion) &&
       (SpatialTypeOfSymbol(arg2)==stregion)) ||

      ((SpatialTypeOfSymbol(arg1)== stpoint) &&
       (SpatialTypeOfSymbol(arg2)==stsline))  ||

      ((SpatialTypeOfSymbol(arg1)== stsline) &&
       (SpatialTypeOfSymbol(arg2)==stsline)) ){
      return nl->SymbolAtom(CcBool::BasicType());
    }
  }
  return listutils::typeError("points x region expected");
}


/*
10.1.3 Type mapping function SpatialTypeMapBool1

It is for the operator ~isempty~ which have ~point~, ~points~, ~line~,
and ~region~ as input and ~bool~ resulttype.

*/

ListExpr
SpatialTypeMapBool1( ListExpr args )
{
  ListExpr arg1;
  if ( nl->ListLength( args ) == 1 )
  {
    arg1 = nl->First( args );
    if ( SpatialTypeOfSymbol( arg1 ) == stpoint )
      return (nl->SymbolAtom( CcBool::BasicType() ));
    if ( SpatialTypeOfSymbol( arg1 ) == stpoints )
      return (nl->SymbolAtom( CcBool::BasicType() ));
    if ( SpatialTypeOfSymbol( arg1 ) == stline )
      return (nl->SymbolAtom( CcBool::BasicType() ));
    if ( SpatialTypeOfSymbol( arg1 ) == stregion )
      return (nl->SymbolAtom( CcBool::BasicType() ));
    if ( SpatialTypeOfSymbol( arg1 ) == stsline )
      return (nl->SymbolAtom( CcBool::BasicType() ));
  }
  return listutils::typeError("");
}

ListExpr SpatialIntersectionTypeMap(ListExpr args){
  string err = "t1 x t2 expected, t_i in {points, points, line, sline, region";
  if(nl->ListLength(args)!=2){
    return listutils::typeError(err + ": wrong number of arguments");
  }
  ListExpr arg1 = nl->First(args);
  ListExpr arg2 = nl->Second(args);
  if(!listutils::isSymbol(arg1)){
    return listutils::typeError(err+ ": first arg not a spatial type");
  }
  if(!listutils::isSymbol(arg2)){
    return listutils::typeError(err+ ": second arg not a spatial type");
  }
  string a1 = nl->SymbolValue(arg1);
  string a2 = nl->SymbolValue(arg2);

  if(a1==Point::BasicType()){
    if(a2==Point::BasicType() || a2==Points::BasicType() ||
       a2==Line::BasicType() || a2==Region::BasicType() ||
       a2==SimpleLine::BasicType())
      return nl->SymbolAtom(Points::BasicType());
    return listutils::typeError(err+ ": second arg not a spatial type");
  }
  if(a1==Points::BasicType()){
    if(a2==Point::BasicType() || a2==Points::BasicType() ||
       a2==Line::BasicType() || a2==Region::BasicType() ||
       a2==SimpleLine::BasicType())
      return nl->SymbolAtom(Points::BasicType());
    return listutils::typeError(err+ ": second arg not a spatial type");
  }

  if(a1==Line::BasicType()){
    if(a2==Point::BasicType() ||a2==Points::BasicType())
      return nl->SymbolAtom(Points::BasicType());
    if(a2==Line::BasicType() || a2==Region::BasicType())
      return nl->SymbolAtom(Line::BasicType());
    if(a2==SimpleLine::BasicType())
      return nl->SymbolAtom(SimpleLine::BasicType());
    return listutils::typeError(err+ ": second arg not a spatial type");
  }

  if(a1==Region::BasicType()){
    if(a2==Point::BasicType() || a2==Points::BasicType())
      return nl->SymbolAtom(Points::BasicType());
    if(a2==Line::BasicType())   return nl->SymbolAtom(Line::BasicType());
    if(a2==Region::BasicType())  return nl->SymbolAtom(Region::BasicType());
    if(a2==SimpleLine::BasicType())
      return nl->SymbolAtom(SimpleLine::BasicType());
    return listutils::typeError(err+ ": second arg not a spatial type");
  }

  if(a1==SimpleLine::BasicType()){
    if(a2==Point::BasicType() ||a2==Points::BasicType())
      return nl->SymbolAtom(Points::BasicType());
    if(a2==Line::BasicType() || a2==Region::BasicType())
      return nl->SymbolAtom(Line::BasicType());
    if(a2==SimpleLine::BasicType())
      return nl->SymbolAtom(SimpleLine::BasicType());
    return listutils::typeError(err+ ": second arg not a spatial type");
  }

  return listutils::typeError(err+ ": first arg not a spatial type");
}

ListExpr SpatialMinusTypeMap(ListExpr args){
  string err = "t1 x t2 expected, t_i in {points, points, line, sline, region";
  if(nl->ListLength(args)!=2){
    return listutils::typeError(err + ": wrong number of arguments");
  }
  ListExpr arg1 = nl->First(args);
  ListExpr arg2 = nl->Second(args);
  if(!listutils::isSymbol(arg1)){
    return listutils::typeError(err+ ": first arg not a spatial type");
  }
  if(!listutils::isSymbol(arg2)){
    return listutils::typeError(err+ ": second arg not a spatial type");
  }
  string a1 = nl->SymbolValue(arg1);
  string a2 = nl->SymbolValue(arg2);

  if(a1==Point::BasicType()){
    if(a2==Point::BasicType())  return nl->SymbolAtom(Points::BasicType());
    if(a2==Points::BasicType()) return nl->SymbolAtom(Points::BasicType());
    if(a2==Line::BasicType())   return nl->SymbolAtom(Points::BasicType());
    if(a2==Region::BasicType()) return nl->SymbolAtom(Points::BasicType());
    if(a2==SimpleLine::BasicType()) return nl->SymbolAtom(Points::BasicType());
    return listutils::typeError(err+ ": second arg not a spatial type");
  }

  if(a1==Points::BasicType()){
    if(a2==Point::BasicType())  return nl->SymbolAtom(Points::BasicType());
    if(a2==Points::BasicType()) return nl->SymbolAtom(Points::BasicType());
    if(a2==Line::BasicType())   return nl->SymbolAtom(Points::BasicType());
    if(a2==Region::BasicType()) return nl->SymbolAtom(Points::BasicType());
    if(a2==SimpleLine::BasicType()) return nl->SymbolAtom(Points::BasicType());
    return listutils::typeError(err+ ": second arg not a spatial type");
  }

  if(a1==Line::BasicType()){
    if(a2==Point::BasicType())  return nl->SymbolAtom(Line::BasicType());
    if(a2==Points::BasicType()) return nl->SymbolAtom(Line::BasicType());
    if(a2==Line::BasicType())   return nl->SymbolAtom(Line::BasicType());
    if(a2==Region::BasicType()) return nl->SymbolAtom(Line::BasicType());
    if(a2==SimpleLine::BasicType()) return nl->SymbolAtom(Line::BasicType());
    return listutils::typeError(err+ ": second arg not a spatial type");
  }

  if(a1==Region::BasicType()){
    if(a2==Point::BasicType())  return nl->SymbolAtom(Region::BasicType());
    if(a2==Points::BasicType()) return nl->SymbolAtom(Region::BasicType());
    if(a2==Line::BasicType())   return nl->SymbolAtom(Region::BasicType());
    if(a2==Region::BasicType()) return nl->SymbolAtom(Region::BasicType());
    if(a2==SimpleLine::BasicType()) return nl->SymbolAtom(Region::BasicType());
    return listutils::typeError(err+ ": second arg not a spatial type");
  }

  if(a1==SimpleLine::BasicType()){
    if(a2==Point::BasicType())  return nl->SymbolAtom(SimpleLine::BasicType());
    if(a2==Points::BasicType()) return nl->SymbolAtom(SimpleLine::BasicType());
    if(a2==Line::BasicType())   return nl->SymbolAtom(SimpleLine::BasicType());
    if(a2==Region::BasicType()) return nl->SymbolAtom(SimpleLine::BasicType());
    if(a2==SimpleLine::BasicType())
      return nl->SymbolAtom(SimpleLine::BasicType());
    return listutils::typeError(err+ ": second arg not a spatial type");
  }

  return listutils::typeError(err+ ": first arg not a spatial type");

}

ListExpr SpatialUnionTypeMap(ListExpr args){
  string err = "t1 x t2 expected, t_i in {points, points, line, sline, region";
  if(nl->ListLength(args)!=2){
    return listutils::typeError(err + ": wrong number of arguments");
  }
  ListExpr arg1 = nl->First(args);
  ListExpr arg2 = nl->Second(args);
  if(!listutils::isSymbol(arg1)){
    return listutils::typeError(err+ ": first arg not a spatial type");
  }
  if(!listutils::isSymbol(arg2)){
    return listutils::typeError(err+ ": second arg not a spatial type");
  }
  string a1 = nl->SymbolValue(arg1);
  string a2 = nl->SymbolValue(arg2);

  if(a1==Point::BasicType()){
    if(a2==Point::BasicType())  return nl->SymbolAtom(Points::BasicType());
    if(a2==Points::BasicType()) return nl->SymbolAtom(Points::BasicType());
    if(a2==Line::BasicType())   return nl->SymbolAtom(Line::BasicType());
    if(a2==Region::BasicType())  return nl->SymbolAtom(Region::BasicType());
    if(a2==SimpleLine::BasicType())
      return nl->SymbolAtom(SimpleLine::BasicType());
    return listutils::typeError(err+ ": second arg not a spatial type");
  }
  if(a1==Points::BasicType()){
    if(a2==Point::BasicType())  return nl->SymbolAtom(Points::BasicType());
    if(a2==Points::BasicType()) return nl->SymbolAtom(Points::BasicType());
    if(a2==Line::BasicType())   return nl->SymbolAtom(Line::BasicType());
    if(a2==Region::BasicType())  return nl->SymbolAtom(Region::BasicType());
    if(a2==SimpleLine::BasicType())
      return nl->SymbolAtom(SimpleLine::BasicType());
    return listutils::typeError(err+ ": second arg not a spatial type");
  }
  if(a1==Line::BasicType()){
    if(a2==Point::BasicType())  return nl->SymbolAtom(Line::BasicType());
    if(a2==Points::BasicType()) return nl->SymbolAtom(Line::BasicType());
    if(a2==Line::BasicType())   return nl->SymbolAtom(Line::BasicType());
    if(a2==Region::BasicType())  return nl->SymbolAtom(Region::BasicType());
    if(a2==SimpleLine::BasicType()) return nl->SymbolAtom(Line::BasicType());
    return listutils::typeError(err+ ": second arg not a spatial type");
  }
  if(a1==Region::BasicType()){
    if(a2==Point::BasicType())  return nl->SymbolAtom(Region::BasicType());
    if(a2==Points::BasicType()) return nl->SymbolAtom(Region::BasicType());
    if(a2==Line::BasicType())   return nl->SymbolAtom(Region::BasicType());
    if(a2==Region::BasicType())  return nl->SymbolAtom(Region::BasicType());
    if(a2==SimpleLine::BasicType()) return nl->SymbolAtom(Region::BasicType());
    return listutils::typeError(err+ ": second arg not a spatial type");
  }
  if(a1==SimpleLine::BasicType()){
    if(a2==Point::BasicType())  return nl->SymbolAtom(SimpleLine::BasicType());
    if(a2==Points::BasicType()) return nl->SymbolAtom(SimpleLine::BasicType());
    if(a2==Line::BasicType())   return nl->SymbolAtom(Line::BasicType());
    if(a2==Region::BasicType())  return nl->SymbolAtom(Region::BasicType());
    if(a2==SimpleLine::BasicType()) return nl->SymbolAtom(Line::BasicType());
    return listutils::typeError(err+ ": second arg not a spatial type");
  }
  return listutils::typeError(err+ ": first arg not a spatial type");
}


/*
10.1.7 Type mapping function for operator ~crossings~

This type mapping function is the one for ~crossings~ operator. This operator
compute the crossing point of two lines so that the result type is a set
of points.

*/
ListExpr
SpatialCrossingsTM( ListExpr args )
{
  ListExpr arg1, arg2;
  if ( nl->ListLength( args ) == 2 )
  {
    arg1 = nl->First( args );
    arg2 = nl->Second( args );

    if ( SpatialTypeOfSymbol( arg1 ) == stline &&
         SpatialTypeOfSymbol( arg2 ) == stline )
      return (nl->SymbolAtom( Points::BasicType() ));
    if ( SpatialTypeOfSymbol( arg1 ) == stsline &&
         SpatialTypeOfSymbol( arg2 ) == stsline )
      return (nl->SymbolAtom( Points::BasicType() ));
  }
  if(nl->ListLength(args==1)){ // internal crossings of a single line
    arg1 = nl->First( args );
    if ( SpatialTypeOfSymbol( arg1 ) == stline){
        return (nl->SymbolAtom( Points::BasicType() ));
    }
  }
  return listutils::typeError("");
}

/*
10.1.8 Type mapping function for operator ~single~

This type mapping function is used for the ~single~ operator. This
operator transform a single-element points value to a point.

*/
ListExpr
SpatialSingleMap( ListExpr args )
{
  ListExpr arg1;
  if ( nl->ListLength( args ) == 1 )
  {
    arg1 = nl->First( args );

    if (SpatialTypeOfSymbol( arg1 ) == stpoints)
      return (nl->SymbolAtom( Point::BasicType() ));
  }
  return listutils::typeError("");
}

/*
10.1.9 Type mapping function for operator ~distance~

This type mapping function is used for the ~distance~ operator. This
operator computes the distance between two spatial objects.

*/

bool isDistanceParam(ListExpr arg){

  return Point::checkType(arg) ||
         Points::checkType(arg) ||
         Line::checkType(arg) ||
         SimpleLine::checkType(arg) ||
         Rectangle<2>::checkType(arg) ||
         Region::checkType(arg);
}

bool sphericalDistImplemented(ListExpr arg1, ListExpr arg2){
   if(Point::checkType(arg1)){
      return    Point::checkType(arg2) || Points::checkType(arg2)
             || Line::checkType(arg2) || SimpleLine::checkType(arg2);
   }
   if(Points::checkType(arg1)){
      return    Point::checkType(arg2) || Points::checkType(arg2)
             || Line::checkType(arg2);
   }
   if(Line::checkType(arg1)){
      return    Point::checkType(arg2) || Points::checkType(arg2);
   }
   if(SimpleLine::checkType(arg1)){
      return    Point::checkType(arg2) ;
   }
   if(Rectangle<2>::checkType(arg1)){
      return Rectangle<2>::checkType(arg2);
   }
   if(Region::checkType(arg1)){
      return false;
   }
   return false;
}

ListExpr
SpatialDistanceMap( ListExpr args )
{
  int noargs = nl->ListLength(args);
  string errmsg = "Expected (T1 x T2 [x geoid]) where T1,T2 in "
                  "{point,points,line,sline,region,rectangle} [ x geoid].";

  if( (noargs < 2) || (noargs > 3) ){
    return listutils::typeError(errmsg);
  }

  ListExpr arg1, arg2;
  arg1 = nl->First( args );
  arg2 = nl->Second( args );

  if(noargs==3){
    if(!Geoid::checkType(nl->Third(args))){
        return listutils::typeError(errmsg);
    }
  }
  if(!isDistanceParam(arg1) || !isDistanceParam(arg2)){
    return listutils::typeError(errmsg);
  }
  string err2 = "spherical geometry not implemented "
                "for this type combination";

  // spherical geometry is not implemented for all data types
  if(noargs==3 && !sphericalDistImplemented(arg1,arg2)){
     return listutils::typeError(err2);
  }

  return listutils::basicSymbol<CcReal>();
}



/*
10.1.10 Type Mapping for distanceSmallerThan

Signature is line x line x real x bool -> bool

*/
ListExpr distanceSmallerThanTM(ListExpr args){
  string err = "line x line x real x bool expected";
  if(!nl->HasLength(args,4)){
    return listutils::typeError(err);
  }

  if(!Line::checkType(nl->First(args)) ||
     !Line::checkType(nl->Second(args)) ||
     !CcReal::checkType(nl->Third(args)) ||
     !CcBool::checkType(nl->Fourth(args))){
    return listutils::typeError(err);
  }
  return nl->SymbolAtom(CcBool::BasicType());
}







/*
10.1.10 Type mapping function for operator ~direction~ and ~heading~

This type mapping function is used for the ~direction~ and for the
~heading~ perator. This operator computes the direction from the first point
to the second point.

---- point x point [x geoid] -> real
----

*/
ListExpr
SpatialDirectionHeadingMap( ListExpr args )
{
  string err = "Expected point x point [x geoid]. ";
  int len = nl->ListLength(args);
  if( (len!=2) && (len!=3)){
    return listutils::typeError(err);
  }
  if(!listutils::isSymbol(nl->First(args),Point::BasicType()) ||
     !listutils::isSymbol(nl->Second(args),Point::BasicType())){
    return listutils::typeError(err);
  }
  if( (len==3) && !listutils::isSymbol(nl->Third(args), Geoid::BasicType())){
    return listutils::typeError(err);
  }
  return (nl->SymbolAtom(CcReal::BasicType() ));
}

/*
10.1.11 Type mapping function for operator ~no\_components~

This type mapping function is used for the ~no\_components~ operator. This
operator computes the number of components of a spatial object. For poins,
this function returns the number of points contained in the point set.
For regions, this function returns the faces of the region.

*/
ListExpr
SpatialNoComponentsMap( ListExpr args )
{
  ListExpr arg1;
  if ( nl->ListLength( args ) == 1 )
  {
    arg1 = nl->First( args );

    if ((SpatialTypeOfSymbol( arg1 ) == stpoints)||
        (SpatialTypeOfSymbol( arg1 ) == stline)||
        (SpatialTypeOfSymbol( arg1 ) == stregion))
      return (nl->SymbolAtom( CcInt::BasicType() ));
  }
  return listutils::typeError("");
}

/*
10.1.11 Type mapping function for operator ~no\_segments~

This type mapping function is used for the ~no\_segments~ operator. This
operator computes the number of segments of a spatial object (lines and
regions only).

*/
ListExpr
SpatialNoSegmentsMap( ListExpr args )
{
  ListExpr arg1;
  if ( nl->ListLength( args ) == 1 )
  {
    arg1 = nl->First( args );
    if (Line::checkType( arg1 ) ||
        Region::checkType(arg1) ||
        SimpleLine::checkType(arg1) ||
        DLine::checkType(arg1))
        return listutils::basicSymbol<CcInt>();
  }
  return listutils::typeError("");
}

/*
10.1.12 Type mapping function for operator ~size~

This type mapping function is used for the ~size~ operator. This operator
computes the size of the spatial object. For line, the size is the totle length
of the line segments.

----
  {line|sline|region} --> real
  {line|sline} x string --> real
----

*/
ListExpr
SpatialSizeMap( ListExpr args )
{
  string errmsg = "Expected (region) or ({line|sline} [ x geoid ]).";
  int noargs = nl->ListLength( args );
  if ( (noargs<1) || (noargs >2) ){
    return listutils::typeError(errmsg);
  }
  ListExpr arg1 = nl->First( args );
  SpatialType st = SpatialTypeOfSymbol(arg1);
  if( (st!=stregion) && (st!=stline) && (st!=stsline) ){
    return listutils::typeError(errmsg);
  }
  if( (noargs==2) &&
      (    (!listutils::isSymbol(nl->Second(args), Geoid::BasicType()))
        || (listutils::isSymbol(nl->First(args),  Region::BasicType())) ) ){
    return listutils::typeError(errmsg);
  }
  return nl->SymbolAtom(CcReal::BasicType());
}

/*
10.1.13 Type mapping function for operator ~touchpoints~

This type mapping function is used for the ~touchpoints~ operator. This operator
computes the touchpoints of a region and another region or a line.

*/
ListExpr
SpatialTouchPointsMap( ListExpr args )
{
  ListExpr arg1, arg2;
  if ( nl->ListLength( args ) == 2 )
  {
    arg1 = nl->First( args );
    arg2 = nl->Second( args );

    if ( SpatialTypeOfSymbol( arg1 ) == stregion &&
         SpatialTypeOfSymbol( arg2 ) == stregion )
      return (nl->SymbolAtom( Points::BasicType() ));

    if ( SpatialTypeOfSymbol( arg1 ) == stregion &&
         SpatialTypeOfSymbol( arg2 ) == stline )
      return (nl->SymbolAtom( Points::BasicType() ));

    if ( SpatialTypeOfSymbol( arg1 ) == stline &&
         SpatialTypeOfSymbol( arg2 ) == stregion )
      return (nl->SymbolAtom( Points::BasicType() ));
  }
  return listutils::typeError("");
}

/*
10.1.17 Type mapping function for operator ~components~

This type mapping function is used for the ~components~ operator.

*/
ListExpr SpatialComponentsMap( ListExpr args )
{
  if( nl->ListLength( args ) == 1 )
  {
    if( SpatialTypeOfSymbol( nl->First( args ) ) == stpoints )
      return nl->TwoElemList( nl->SymbolAtom(Symbol::STREAM()),
                              nl->SymbolAtom(Points::BasicType()) );

    if( SpatialTypeOfSymbol( nl->First( args ) ) == stregion )
      return nl->TwoElemList( nl->SymbolAtom(Symbol::STREAM()),
                              nl->SymbolAtom(Region::BasicType()) );

    if( SpatialTypeOfSymbol( nl->First( args ) ) == stline )
      return nl->TwoElemList( nl->SymbolAtom(Symbol::STREAM()),
                              nl->SymbolAtom(Line::BasicType()) );

  }
  return listutils::typeError("point, line or region expected");
}


/*
10.1.18 Type Mapping function for operator ~vertices~

*/
ListExpr SpatialVerticesMap(ListExpr args)
{
  if(nl->ListLength(args)==1)
  {
    if( (listutils::isSymbol(nl->First(args),Region::BasicType())) ||
        (listutils::isSymbol(nl->First(args),Line::BasicType())) ){
      return nl->SymbolAtom(Points::BasicType());
    }
  }
  return listutils::typeError("region or line required");
}

/*
10.1.18 Type Mapping function for operator ~boundary~

*/
ListExpr SpatialBoundaryMap(ListExpr args)
{
  if(nl->ListLength(args)==1)
  {
    if( SpatialTypeOfSymbol( nl->First( args ) ) == stregion ){
      return nl->SymbolAtom(Line::BasicType());
    }

    if( SpatialTypeOfSymbol( nl->First( args ) ) == stline ){
        return nl->SymbolAtom(Points::BasicType());
    }
  }
  return listutils::typeError("region or line required");
}

/*
10.1.14 Type mapping function for operator ~commonborder~

This type mapping function is used for the ~commonborder~ operator. This
operator computes the commonborder of two regions.

*/
ListExpr
SpatialCommonBorderMap( ListExpr args )
{
  ListExpr arg1, arg2;
  if ( nl->ListLength( args ) == 2 )
  {
    arg1 = nl->First( args );
    arg2 = nl->Second( args );

    if ( SpatialTypeOfSymbol( arg1 ) == stregion &&
         SpatialTypeOfSymbol( arg2 ) == stregion )
      return (nl->SymbolAtom( Line::BasicType() ));
  }
  return listutils::typeError("");
}

/*
10.1.15 Type mapping function for operator ~bbox~

This type mapping function is used for the ~bbox~ operator. This operator
computes the bbox of a region, which is a ~rect~ (see RectangleAlgebra).

*/
ListExpr
SpatialBBoxMap( ListExpr args )
{
  int noargs = nl->ListLength( args );
  string errmsg = "Expected T [x geoid], T in {region, point, line, sline, "
                  "points}.";
  ListExpr arg1;
  if ( (noargs<1) || (noargs >2) ){
    return listutils::typeError(errmsg);
  }
  arg1 = nl->First( args );
  if ( SpatialTypeOfSymbol( arg1 ) != stregion &&
       SpatialTypeOfSymbol( arg1 ) != stpoint &&
       SpatialTypeOfSymbol( arg1 ) != stline &&
       SpatialTypeOfSymbol( arg1 ) != stpoints &&
       SpatialTypeOfSymbol( arg1 ) != stsline  &&
       !Rectangle<2>::checkType(arg1) ){
    return listutils::typeError(errmsg);
  }
  if( (noargs == 2) &&
      !listutils::isSymbol(nl->Second(args),Geoid::BasicType()) ) {
    return listutils::typeError(errmsg);
  }
  return (nl->SymbolAtom( Rectangle<2>::BasicType() ));
}



/*
10.1.16 Type Mapping function for center.

The signature is points -> point.

*/

ListExpr SpatialCenterMap(ListExpr args){

  if( (nl->ListLength(args)==1) &&
      (nl->IsEqual(nl->First(args),Points::BasicType())) ){
      return nl->SymbolAtom(Point::BasicType());
  }
  return listutils::typeError("points expected");
}


/*
10.1.17 Type Mapping function for convexhull.

The signature is points -> region.

*/

ListExpr SpatialConvexhullMap(ListExpr args){

  if( (nl->ListLength(args)==1) &&
      (nl->IsEqual(nl->First(args),Points::BasicType())) ){
      return nl->SymbolAtom(Region::BasicType());
  }
  return listutils::typeError("points expected");
}


/*
10.1.17 Type mapping function for operator ~windowclipping~

This type mapping function is used for the ~windowclipping~ operators. There are
two kind of operators, one that computes the part of the object that is inside
the window (windowclippingin), and another one that computes the part that is
outside of it (windowclippingout).

*/
ListExpr
SpatialWindowClippingMap( ListExpr args )
{
  ListExpr arg1;
  if ( nl->ListLength( args ) == 2 )
  {
    arg1 = nl->First( args );
  //  arg2 = nl->Second( args );

    if ( SpatialTypeOfSymbol( arg1 ) == stline)
        return (nl->SymbolAtom( Line::BasicType() ));

    if ( SpatialTypeOfSymbol( arg1 ) == stregion )
        return (nl->SymbolAtom( Region::BasicType() ));
  }
  return listutils::typeError("");
}


/*
10.1.6 Type mapping function for operator ~atpoint~

This type mapping function is the one for ~atpoint~ operator. This operator
receives a line and a point and returns the relative position of the point
in the line as a real.

*/
ListExpr
SpatialAtPointTypeMap( ListExpr args )
{
  ListExpr arg1, arg2, arg3;
  if ( nl->ListLength( args ) == 3 )
  {
    arg1 = nl->First( args );
    arg2 = nl->Second( args );
    arg3 = nl->Third( args );

    if ( SpatialTypeOfSymbol( arg1 ) == stsline &&
         SpatialTypeOfSymbol( arg2 ) == stpoint &&
         nl->IsEqual( arg3, CcBool::BasicType() ) )
      return (nl->SymbolAtom( CcReal::BasicType() ));
  }
  if (nl->ListLength(args) == 2)
  {
    arg1 = nl->First( args );
    arg2 = nl->Second( args );
    if ( SpatialTypeOfSymbol( arg1 ) == stsline &&
         SpatialTypeOfSymbol( arg2 ) == stpoint)
      return (nl->SymbolAtom( CcReal::BasicType() ));
  }
    return listutils::typeError("Expects sline, point and optional bool.");
}

/*
10.1.6 Type mapping function for operator ~atposition~

This type mapping function is the one for ~atposition~ operator. This operator
receives a line and a relative position and returns the corresponding point.

*/
ListExpr
SpatialAtPositionTypeMap( ListExpr args )
{
  ListExpr arg1, arg2, arg3;
  if ( nl->ListLength( args ) == 3 )
  {
    arg1 = nl->First( args );
    arg2 = nl->Second( args );
    arg3 = nl->Third( args );

    if ( SpatialTypeOfSymbol( arg1 ) == stsline &&
         nl->IsEqual( arg2, CcReal::BasicType() ) &&
         nl->IsEqual( arg3, CcBool::BasicType() ) )
      return (nl->SymbolAtom( Point::BasicType() ));
  }
  if ( nl->ListLength( args ) == 2 )
  {
    arg1 = nl->First( args );
    arg2 = nl->Second( args );
    if ( SpatialTypeOfSymbol( arg1 ) == stsline &&
         nl->IsEqual( arg2, CcReal::BasicType() ) )
      return (nl->SymbolAtom( Point::BasicType() ));
  }
  return listutils::typeError("Expects sline, real and optional bool.");
}

/*
10.1.6 Type mapping function for operator ~subline~

This type mapping function is the one for ~subline~ operator. This operator
receives a line and two relative positions and returns the corresponding
sub-line.

*/
ListExpr
SpatialSubLineMap( ListExpr args )
{
  ListExpr arg1, arg2, arg3, arg4;
  if ( nl->ListLength( args ) == 4 )
  {
    arg1 = nl->First( args );
    arg2 = nl->Second( args );
    arg3 = nl->Third( args );
    arg4 = nl->Fourth( args );

    if ( SpatialTypeOfSymbol( arg1 ) == stsline &&
         nl->IsEqual( arg2, CcReal::BasicType() ) &&
         nl->IsEqual( arg3, CcReal::BasicType() ) &&
         nl->IsEqual( arg4, CcBool::BasicType() ) )
      return (nl->SymbolAtom( SimpleLine::BasicType() ));
  }
  return listutils::typeError("");
}

/*
10.1.6 Type mapping function for operator ~add~

This type mapping function is the one for the ~add~ operator.
The result type is a point.

*/
ListExpr
SpatialAddTypeMap( ListExpr args )
{
  ListExpr arg1, arg2;
  if ( nl->ListLength( args ) == 2 )
  {
    arg1 = nl->First( args );
    arg2 = nl->Second( args );

    if ( SpatialTypeOfSymbol( arg1 ) == stpoint &&
         SpatialTypeOfSymbol( arg2 ) == stpoint )
      return (nl->SymbolAtom( Point::BasicType() ));
  }
  return listutils::typeError("");
}

/*
10.1.7 Type mapping function for the operators ~getx~ and ~gety~

This type mapping function is the one for the ~getx and ~gety operator.
The result type is a real.

*/
ListExpr
SpatialGetXYMap( ListExpr args )
{
  ListExpr arg1;
  if ( nl->ListLength( args ) == 1 )
  {
    arg1 = nl->First( args );

    if ( SpatialTypeOfSymbol( arg1 ) == stpoint )
      return (nl->SymbolAtom( CcReal::BasicType() ));
  }
  return listutils::typeError("");
}

/*
10.1.7 Type mapping function for the operators ~line2region~

This type mapping function is the one for the ~line2region~ operator.
The result type is a region.

*/
ListExpr
SpatialLine2RegionMap( ListExpr args )
{
  string err = "line expected";
  if(!nl->HasLength(args,1)){
    return listutils::typeError(err);
  }
  if(!Line::checkType(nl->First(args))){
    return listutils::typeError(err);
  }
  return listutils::basicSymbol<Region>();
}

/*
10.1.7 Type mapping function for the operator ~rect2region~

This type mapping function is the one for the ~rect2region~ operator.
The result type is a region.

*/
ListExpr
    SpatialRect2RegionMap( ListExpr args )
{
  ListExpr arg1;
  if ( nl->ListLength( args ) == 1 )
  {
    arg1 = nl->First( args );

    if ( SpatialTypeOfSymbol( arg1 ) == stbox )
      return (nl->SymbolAtom( Region::BasicType() ));
  }
  return listutils::typeError("");
}

/*
10.1.7 Type mapping function for the operator ~create_triangle~

---- point x point x point --> region
----

*/
ListExpr SpatialCreateTriangleTM( ListExpr args ){
  string errmsg = "Expected (point x point x point).";
  if( nl->ListLength( args ) != 3 ){
    return listutils::typeError(errmsg);
  }
  if( listutils::isSymbol(nl->First(args),Point::BasicType()) &&
      listutils::isSymbol(nl->Second(args),Point::BasicType()) &&
      listutils::isSymbol(nl->Third(args),Point::BasicType()) ){
    return (nl->SymbolAtom( Region::BasicType() ));
  }
  return listutils::typeError(errmsg);
}

/*
10.1.8 Type mapping function for the operator ~area~

This type mapping function is the one for the ~area~ operator.
The result type is a real.

*/
ListExpr
    SpatialAreaMap( ListExpr args )
{
  ListExpr arg1;
  if ( nl->ListLength( args ) == 1 )
  {
    arg1 = nl->First( args );

    if ( SpatialTypeOfSymbol( arg1 ) == stregion )
      return (nl->SymbolAtom( CcReal::BasicType() ));
  }
  return listutils::typeError("");
}


/*
10.1.9 Type mapping for the ~polylines~ operator

The ~polylines~ operator takes a complex line and creates
a set of simple polylines from it. Thus, the signature of
this operator is line -> stream(line)

*/
ListExpr PolylinesMap(ListExpr args){
  int len = nl->ListLength(args);
  if( (len!=2) &&len!=3){
    return listutils::typeError("line x bool [x points] expected");
  }
  if(!nl->IsEqual(nl->First(args),Line::BasicType()) ||
     !nl->IsEqual(nl->Second(args),CcBool::BasicType())){
    return listutils::typeError("line  x bool expected");
  }
  if(len==3){
     if(!nl->IsEqual(nl->Third(args),Points::BasicType())){
       return listutils::typeError("line x bool [x points] expected");
     }
  }
  return nl->TwoElemList(nl->SymbolAtom(Symbol::STREAM()),
                         nl->SymbolAtom(Line::BasicType()));
}


/*
10.1.10 Type mapping for the ~simplify~ operator

*/
ListExpr SimplifyTypeMap(ListExpr args){
   int len = nl->ListLength(args);
   if((len!=2) && (len!=3) && (len!=4)){
     return listutils::typeError("invalid number of"
                                 " arguments (has to be 2,3, or 4 )");
   }
   if(!nl->IsEqual(nl->First(args),Line::BasicType()) ||
      !nl->IsEqual(nl->Second(args),CcReal::BasicType())){
     return listutils::typeError("line x real [x points] [x geoid] expected");
   }
   if(len==2){
     return nl->SymbolAtom(Line::BasicType());
   }
   ListExpr third = nl->Third(args);
   if(len==3){
      if(!Points::checkType(third) && !Geoid::checkType(third)){
        return listutils::typeError("line x real [x points][x geoid] expected");
      } else {
        return nl->SymbolAtom(Line::BasicType());
      }
   }
   // len == 4
   if(!Points::checkType(third) || !Geoid::checkType(nl->Fourth(args))){
        return listutils::typeError("line x real[ x points][x geoid] expected");
   }
   return nl->SymbolAtom(Line::BasicType());
}



/*
10.1.11 Type Mapping for the ~segments~ operator

*/
ListExpr SegmentsTypeMap(ListExpr args){
  if(nl->ListLength(args)!=1){
    return listutils::typeError("Invalid number of arguments");
  }
  if(   !Line::checkType(nl->First(args)) 
     && !DLine::checkType(nl->First(args))){
    return listutils::typeError("line expected");
  }
  return nl->TwoElemList(
               nl->SymbolAtom(Symbol::STREAM()),
               nl->First(args)
         );
}

/*
10.1.12 Type Mapping for the ~get~ operator

Signatur is points x int -> point

*/
ListExpr GetTypeMap(ListExpr args){
   if( (nl->ListLength(args)==2) &&
       (nl->IsEqual(nl->First(args),Points::BasicType())) &&
       (nl->IsEqual(nl->Second(args),CcInt::BasicType()))){
      return nl->SymbolAtom(Point::BasicType());
   }
   return listutils::typeError("points x int expected");
}

/*
10.1.13 Type Mapping for the ~realminize~ operator

Signatur is line -> line

*/
ListExpr RealminizeTypeMap(ListExpr args){
  if( (nl->ListLength(args)==1) &&
       (nl->IsEqual(nl->First(args),Line::BasicType()))){
    return nl->SymbolAtom(Line::BasicType());
       }
       return listutils::typeError("line expected");
}

/*
10.1.14 Type Mapping for the ~makeline~ operator

Signature is point x point -> line

*/
ListExpr MakeLineTypeMap(ListExpr args){
  int len;
  if((len = nl->ListLength(args))!=2){
    return listutils::typeError("two arguments expected, but got " + 
              stringutils::int2str(len));
  }
  if(nl->IsEqual(nl->First(args),Point::BasicType()) &&
     nl->IsEqual(nl->Second(args),Point::BasicType())){
     return nl->SymbolAtom(Line::BasicType());
  } else {
    return listutils::typeError("point x point expected");
  }
}

/*
10.1.14 Type Mapping for the ~makesline~ operator

Signature is point x point -> sline

*/
ListExpr MakeSLineTypeMap(ListExpr args){
  int len;
  if((len = nl->ListLength(args))!=2){
    return listutils::typeError("two arguments expected, but got " 
              + stringutils::int2str(len));
  }
  if(nl->IsEqual(nl->First(args),Point::BasicType()) &&
     nl->IsEqual(nl->Second(args),Point::BasicType())){
     return nl->SymbolAtom(SimpleLine::BasicType());
  } else {
    return listutils::typeError("point x point expected");
  }
}


/*
~CommonBorder2TypeMap~

Signature: ~region~ [x] ~region~ [->] ~line~

*/

ListExpr CommonBorder2TypeMap(ListExpr args){

  if(nl->ListLength(args)!=2){
    return listutils::typeError("Wrong number of arguments,"
                                " region x region expected");
  }
  if(nl->IsEqual(nl->First(args),Region::BasicType()) &&
     nl->IsEqual(nl->Second(args),Region::BasicType())){
     return nl->SymbolAtom(Line::BasicType());
  }
  return listutils::typeError(" region x region expected");
}

/*
~toLineTypeMap~

Signature is :  sline [->] line

*/

ListExpr toLineTypeMap(ListExpr args){
  const string err = "sline expected";
  if(nl->ListLength(args)!=1){
    ErrorReporter::ReportError(err);
    return nl->TypeError();
  }
  if(nl->IsEqual(nl->First(args),SimpleLine::BasicType())){
    return nl->SymbolAtom(Line::BasicType());
  }
  ErrorReporter::ReportError(err);
  return listutils::typeError("");
}


/*
~fromLineTypeMap~

Signature is :  line [->] sline

*/

ListExpr fromLineTypeMap(ListExpr args){
  if ((nl->ListLength(args) == 2 &&
       nl->IsEqual(nl->First(args),Line::BasicType()) &&
       nl->IsEqual(nl->Second(args),CcBool::BasicType())) ||
      (nl->ListLength(args) == 1 &&
       nl->IsEqual(nl->First(args),Line::BasicType())))
    return nl->SymbolAtom(SimpleLine::BasicType());
  else
    return listutils::typeError("line or line and bool expected");
}

/*
~isCycleTypeMap~

Signature is :  sline [->] bool

*/

ListExpr isCycleTypeMap(ListExpr args){
  const string err = "sline expected";
  if(nl->ListLength(args)!=1){
    ErrorReporter::ReportError(err);
    return nl->TypeError();
  }
  if(nl->IsEqual(nl->First(args),SimpleLine::BasicType())){
    return nl->SymbolAtom(CcBool::BasicType());
  }
  ErrorReporter::ReportError(err);
  return listutils::typeError("");
}


/*
Type Mapping for ~utm~

*/
ListExpr utmTypeMap(ListExpr args){
  if(nl->ListLength(args)!=1){
    return listutils::typeError("one argument expected");
  }
  ListExpr arg = nl->First(args);
  string err = "spatial type expected";
  if(nl->AtomType(arg)!=SymbolType){
    return listutils::typeError(err);
  }
  string t = nl->SymbolValue(arg);
  if(t==Point::BasicType() || t==Points::BasicType() ){
    // line and region not implemented yet
    return nl->SymbolAtom(t);
  }
  return listutils::typeError(err);
}

/*
Type Mapping for ~gk~

*/
ListExpr gkTypeMap(ListExpr args){
  int len = nl->ListLength(args);
  if( (len < 1) || (len > 2) ){
    return listutils::typeError(" 1 or 2 arguments expected");
  }
  string err = "point, points, (s)line, or region  [ x int] expected";
  ListExpr arg = nl->First(args);
  if(!listutils::isSymbol(arg)){
    return listutils::typeError(err);
  }
  string t = nl->SymbolValue(arg);
  if(!( t==Point::BasicType() || t==Points::BasicType() ||
        t==Line::BasicType() || t==Region::BasicType() ||
        t==SimpleLine::BasicType() ||
        t==Rectangle<2>::BasicType())){
    return listutils::typeError(err);
  }
  if( (len==2) && listutils::isSymbol(nl->Second(args),CcInt::BasicType()) ){
    if( t==Point::BasicType() || t==Points::BasicType() ||
        t==Line::BasicType() || t==Region::BasicType() ||
        t==SimpleLine::BasicType()){
      return nl->SymbolAtom(t); // Zone provided by user
    }
  } else if (len==1){
    return nl->ThreeElemList(nl->SymbolAtom(Symbol::APPEND()),
           nl->OneElemList(nl->IntAtom(2)),          // standard zone for Hagen
           nl->SymbolAtom(t));
  }
  return listutils::typeError(err);
}

/*
Type Mapping for ~reverseGk~

*/
ListExpr reverseGkTypeMap(ListExpr args){
  string err = "point, points, line, or region expected";
  if(nl->ListLength(args)!=1){
     return listutils::typeError(err);
  }
  ListExpr arg = nl->First(args);
  if(!listutils::isSymbol(arg)){
     return listutils::typeError(err);
  }
  string v = nl->SymbolValue(arg);
  if( (v!=Point::BasicType()) && (v!=Points::BasicType())  &&
      (v!=Line::BasicType()) && (v!=Region::BasicType())){
     return listutils::typeError(err);
  }
  return nl->SymbolAtom(v);
}


/*
Type Mapping: geoid [->] real
For operators: getRadius, getFlattening

*/
ListExpr geoid2real_TM(ListExpr args){
  int noargs = nl->ListLength(args);
  string errmsg = "Expected ("+Geoid::BasicType()+").";
  if(noargs!=1){
    return listutils::typeError(errmsg);
  }
  if(listutils::isSymbol(nl->First(args), Geoid::BasicType())){
    return nl->SymbolAtom(CcReal::BasicType());
  }
  return listutils::typeError(errmsg);
}

/*
Type Mapping: string [ x {real|int} x {real|int} ] [->] geoid
For operator: create\_geoid

*/
ListExpr geoid_create_geoid_TM(ListExpr args){
  int noargs = nl->ListLength(args);
  string errmsg = "Expected ("+CcString::BasicType()+" [ x {"
                +CcInt::BasicType()+"|"+CcReal::BasicType()
                +"} x {"+CcInt::BasicType()+"|"+CcReal::BasicType()+"} ]).";
  // allow only 1 or 3 arguments:
  if( (noargs<1)|| (noargs==2)  || (noargs>3) ){
    return listutils::typeError(errmsg);
  }
  // check first argument
  if(!listutils::isSymbol(nl->First(args), CcString::BasicType())){
    return listutils::typeError(errmsg);
  }
  // check 2. + 3. argument
  if(noargs==3){
    if(    !listutils::isNumericType(nl->Second(args))
        || !listutils::isNumericType(nl->Third(args)) ){
      return listutils::typeError(errmsg);
    }
  }
  return nl->SymbolAtom(Geoid::BasicType());
}

/*
Type Mapping for ~collect\_line~ and ~collect\_sline~

----
  ((stream point)) -> line
  ((stream sline)) -> line
  ((stream line))  -> line


  ((stream point)) -> sline
  ((stream sline)) -> sline
  ((stream line))  -> sline
----

*/
ListExpr SpatialCollectLineTypeMap(ListExpr args){
  if( nl->IsEmpty(args) || nl->IsAtom(args) || nl->ListLength(args) != 2){
    return listutils::typeError("Expects exactly 2 arguments.");
  }
  ListExpr stream = nl->First(args);
  if(!Stream<Attribute>::checkType(stream)){
    return listutils::typeError("Expects a DATA stream.");
  }
  ListExpr T = nl->Second(stream);
  set<string> r;
  r.insert(Point::BasicType());
  r.insert(SimpleLine::BasicType());
  r.insert(Line::BasicType());

  if(!listutils::isASymbolIn(T,r)){
    return listutils::typeError("Expects stream element type to be one of "
                                "{point, sline, line}.");
  }
  ListExpr arg2 = nl->Second(args);
  if(!CcBool::checkType(arg2)){
    return listutils::typeError("Second argument must be bool.");
  }
  return nl->SymbolAtom(Line::BasicType());
}

ListExpr SpatialCollectSLineTypeMap(ListExpr args){
  if( nl->IsEmpty(args) || nl->IsAtom(args) || nl->ListLength(args) != 2){
    return listutils::typeError("Expects exactly 2 arguments.");
  }
  ListExpr stream = nl->First(args);
  if(!listutils::isDATAStream(stream)){
    return listutils::typeError("Expects a DATA stream.");
  }
  ListExpr T = nl->Second(stream);
  set<string> r;
  r.insert(Point::BasicType());
  r.insert(SimpleLine::BasicType());
  r.insert(Line::BasicType());

  if(!listutils::isASymbolIn(T,r)){
    return listutils::typeError("Expects stream element type to be one of "
                                "{point, sline, line}.");
  }
  ListExpr arg2 = nl->Second(args);
  if(!listutils::isSymbol(arg2,CcBool::BasicType())){
    return listutils::typeError("Second argument must be bool.");
  }
  return nl->SymbolAtom(SimpleLine::BasicType());
}

ListExpr SpatialCollectPointsTM(ListExpr args){
  string err = " {stream(point), stream(points)} x bool expected";
  if(nl->ListLength(args) != 2){
    return listutils::typeError(err);
  }

  ListExpr arg1 = nl->First(args);
  if(nl->ListLength(arg1) != 2){
    return listutils::typeError(err);
  }
  ListExpr s = nl->First(arg1);
  ListExpr p = nl->Second(arg1);
  if(!listutils::isSymbol(s,Symbol::STREAM())){
    return listutils::typeError(err);
  }
  if(!listutils::isSymbol(p,Point::BasicType()) &&
     !listutils::isSymbol(p,Points::BasicType())){
    return listutils::typeError(err);
  }
  ListExpr arg2 = nl->Second(args);
  if(!listutils::isSymbol(arg2,CcBool::BasicType())){
    return listutils::typeError(err);
  }
  return nl->SymbolAtom(Points::BasicType());
}

ListExpr SpatialTMSetStartSmaller(ListExpr args){
  if(!nl->HasLength(args,2)){
    return listutils::typeError("Expected (sline x bool).");
  }
  if(listutils::isSymbol(nl->First(args),SimpleLine::BasicType()) &&
     listutils::isSymbol(nl->Second(args),CcBool::BasicType())){
     return nl->SymbolAtom(SimpleLine::BasicType());
  }
  return listutils::typeError("Expected (sline x bool).");
}

ListExpr SpatialTMGetStartSmaller(ListExpr args){
  if(!nl->HasLength(args,1)){
    return listutils::typeError("Expected: sline.");
  }
  if(listutils::isSymbol(nl->First(args),SimpleLine::BasicType())){
      return nl->SymbolAtom(CcBool::BasicType());
    }
    return listutils::typeError("Expected: sline.");
}

ListExpr SpatialTMCreateSline(ListExpr args){
  if(!nl->HasLength(args,2)){
    return listutils::typeError("Expected (point x point).");
  }
  if(listutils::isSymbol(nl->First(args),Point::BasicType()) &&
     listutils::isSymbol(nl->Second(args),Point::BasicType())){
      return nl->SymbolAtom(SimpleLine::BasicType());
    }
    return listutils::typeError("Expected (point x point).");
}

/*
10.3 Selection functions

A selection function is quite similar to a type mapping function. The only
difference is that it doesn't return a type but the index of a value
mapping function being able to deal with the respective combination of
input parameter types.

Note that a selection function does not need to check the correctness of
argument types; it has already been checked by the type mapping function that it
is applied to correct arguments.

10.3.2 Selection function ~SpatialSelectIsEmpty~

It is used for the ~isempty~ operator

*/
int
SpatialSelectIsEmpty( ListExpr args )
{
  ListExpr arg1 = nl->First( args );

  if ( SpatialTypeOfSymbol( arg1 ) == stpoint )
    return 0;

  if ( SpatialTypeOfSymbol( arg1 ) == stpoints )
    return 1;

  if ( SpatialTypeOfSymbol( arg1 ) == stline )
    return 2;

  if ( SpatialTypeOfSymbol( arg1 ) == stregion )
    return 3;

  if(nl->IsEqual(arg1,SimpleLine::BasicType())){
    return 4;
  }

  return -1; // This point should never be reached
}

/*
10.3.3 Selection function ~SpatialSelectCompare~

It is used for compare operators ($=$, $\neq$, $<$, $>$, $\geq$, $\leq$)

*/
int
SpatialSelectCompare( ListExpr args )
{
  ListExpr arg1 = nl->First( args );
  ListExpr arg2 = nl->Second( args );

  if ( SpatialTypeOfSymbol( arg1 ) == stpoint &&
       SpatialTypeOfSymbol( arg2 ) == stpoint )
    return 0;

  if ( SpatialTypeOfSymbol( arg1 ) == stpoints &&
       SpatialTypeOfSymbol( arg2 ) == stpoints )
    return 1;

  if ( SpatialTypeOfSymbol( arg1 ) == stline &&
       SpatialTypeOfSymbol( arg2 ) == stline )
    return 2;

  if ( SpatialTypeOfSymbol( arg1 ) == stregion &&
       SpatialTypeOfSymbol( arg2 ) == stregion )
    return 3;

  if ( SpatialTypeOfSymbol( arg1 ) == stsline &&
       SpatialTypeOfSymbol( arg2 ) == stsline )
    return 4;

  return -1; // This point should never be reached
}

int SpatialSelectEqual(ListExpr args){
   ListExpr a1 = nl->First(args);
   ListExpr a2 = nl->Second(args);
   string s1 = nl->SymbolValue(a1);
   if(nl->Equal(a1,a2)){
     if(s1 == Point::BasicType()) return 0;
     if(s1 == Points::BasicType()) return 1;
     if(s1 == Line::BasicType()) return 2;
     if(s1 == Region::BasicType()) return 3;
     if(s1 == SimpleLine::BasicType()) return 4;
     return -1;
   } else {
     string s2 = nl->SymbolValue(a2);
     if( (s1==Point::BasicType()) && (s2==Points::BasicType())) return 5;
     if((s1==Points::BasicType()) && (s2==Point::BasicType())) return 6;
   }
   return -1;
}

int SpatialSelectIsLess(ListExpr args){
  ListExpr a1 = nl->First(args);
  ListExpr a2 = nl->Second(args);
  string s1 = nl->SymbolValue(a1);
  if(nl->Equal(a1,a2)){
    if(s1 == Point::BasicType()) return 0;
  }
  return -1;
}

/*
10.3.4 Selection function ~SpatialSelectIntersects~

It is used for the operator ~intersects~

*/
int
SpatialSelectIntersects( ListExpr args )
{
  ListExpr arg1 = nl->First( args );
  ListExpr arg2 = nl->Second( args );

  if ( SpatialTypeOfSymbol( arg1 ) == stpoints &&
       SpatialTypeOfSymbol( arg2 ) == stpoints )
    return 0;

  if ( SpatialTypeOfSymbol( arg1 ) == stpoints &&
       SpatialTypeOfSymbol( arg2 ) == stline )
    return 1;

  if ( SpatialTypeOfSymbol( arg1 ) == stpoints &&
       SpatialTypeOfSymbol( arg2 ) == stregion )
    return 2;

  if ( SpatialTypeOfSymbol( arg1 ) == stline &&
       SpatialTypeOfSymbol( arg2 ) == stpoints )
    return 3;

  if ( SpatialTypeOfSymbol( arg1 ) == stline &&
       SpatialTypeOfSymbol( arg2 ) == stline )
    return 4;

  if ( SpatialTypeOfSymbol( arg1 ) == stline &&
       SpatialTypeOfSymbol( arg2 ) == stregion )
    return 5;

  if ( SpatialTypeOfSymbol( arg1 ) == stregion &&
       SpatialTypeOfSymbol( arg2 ) == stpoints )
    return 6;

  if ( SpatialTypeOfSymbol( arg1 ) == stregion &&
       SpatialTypeOfSymbol( arg2 ) == stline )
    return 7;

  if ( SpatialTypeOfSymbol( arg1 ) == stregion &&
       SpatialTypeOfSymbol( arg2 ) == stregion )
    return 8;

  if ( SpatialTypeOfSymbol( arg1 ) == stsline &&
       SpatialTypeOfSymbol( arg2 ) == stsline )
    return 9;

  return -1; // This point should never be reached
}

/*
10.3.5 Selection function ~SpatialSelectInside~

This select function is used for the ~inside~ operator.

*/
int
SpatialSelectInside( ListExpr args )
{
  ListExpr arg1 = nl->First( args );
  ListExpr arg2 = nl->Second( args );

  if ( SpatialTypeOfSymbol( arg1 ) == stpoint &&
       SpatialTypeOfSymbol( arg2 ) == stpoints )
    return 0;

  if ( SpatialTypeOfSymbol( arg1 ) == stpoint &&
       SpatialTypeOfSymbol( arg2 ) == stline )
    return 1;

  if ( SpatialTypeOfSymbol( arg1 ) == stpoint &&
       SpatialTypeOfSymbol( arg2 ) == stregion )
    return 2;

  if ( SpatialTypeOfSymbol( arg1 ) == stpoints &&
       SpatialTypeOfSymbol( arg2 ) == stpoints )
    return 3;

  if ( SpatialTypeOfSymbol( arg1 ) == stpoints &&
       SpatialTypeOfSymbol( arg2 ) == stline )
    return 4;

  if ( SpatialTypeOfSymbol( arg1 ) == stpoints &&
       SpatialTypeOfSymbol( arg2 ) == stregion )
    return 5;

  if ( SpatialTypeOfSymbol( arg1 ) == stline &&
       SpatialTypeOfSymbol( arg2 ) == stline )
    return 6;

  if ( SpatialTypeOfSymbol( arg1 ) == stline &&
       SpatialTypeOfSymbol( arg2 ) == stregion )
    return 7;

  if ( SpatialTypeOfSymbol( arg1 ) == stregion &&
       SpatialTypeOfSymbol( arg2 ) == stregion )
    return 8;

  if (SpatialTypeOfSymbol(arg1) == stpoint &&
      SpatialTypeOfSymbol(arg2) == stsline)
    return 9;

  if (SpatialTypeOfSymbol(arg1) == stsline &&
      SpatialTypeOfSymbol(arg2) == stsline)
    return 10;

  return -1; // This point should never be reached
}

/*
10.3.6 Selection function ~SpatialSelectTopology~

This select function is used for the ~attached~ , and ~overlaps~  operator.

*/
int
SpatialSelectTopology( ListExpr args )
{
  ListExpr arg1 = nl->First( args );
  ListExpr arg2 = nl->Second( args );

  if ( SpatialTypeOfSymbol( arg1 ) == stpoints &&
       SpatialTypeOfSymbol( arg2 ) == stpoints )
    return 0;

  if ( SpatialTypeOfSymbol( arg1 ) == stpoints &&
       SpatialTypeOfSymbol( arg2 ) == stline )
    return 1;

  if ( SpatialTypeOfSymbol( arg1 ) == stpoints &&
       SpatialTypeOfSymbol( arg2 ) == stregion )
    return 2;

  if ( SpatialTypeOfSymbol( arg1 ) == stline &&
       SpatialTypeOfSymbol( arg2 ) == stpoints )
    return 3;

  if ( SpatialTypeOfSymbol( arg1 ) == stline &&
       SpatialTypeOfSymbol( arg2 ) == stline )
    return 4;

  if ( SpatialTypeOfSymbol( arg1 ) == stline &&
       SpatialTypeOfSymbol( arg2 ) == stregion )
    return 5;

  if ( SpatialTypeOfSymbol( arg1 ) == stregion &&
       SpatialTypeOfSymbol( arg2 ) == stpoints )
    return 6;

  if ( SpatialTypeOfSymbol( arg1 ) == stregion &&
       SpatialTypeOfSymbol( arg2 ) == stline )
    return 7;

  if ( SpatialTypeOfSymbol( arg1 ) == stregion &&
       SpatialTypeOfSymbol( arg2 ) == stregion )
    return 8;

  return -1; // This point should never be reached
}

/*
10.3.6 Selection function ~SpatialSelectAdjacent~

This select function is used for the ~adjacent~ operator.

*/
int
SpatialSelectAdjacent( ListExpr args )
{
  ListExpr arg1 = nl->First( args );
  ListExpr arg2 = nl->Second( args );

  if ( SpatialTypeOfSymbol( arg1 ) == stpoints &&
       SpatialTypeOfSymbol( arg2 ) == stregion )
    return 0;

  if ( SpatialTypeOfSymbol( arg1 ) == stline &&
       SpatialTypeOfSymbol( arg2 ) == stregion )
    return 1;

  if ( SpatialTypeOfSymbol( arg1 ) == stregion &&
       SpatialTypeOfSymbol( arg2 ) == stpoints )
    return 2;

  if ( SpatialTypeOfSymbol( arg1 ) == stregion &&
       SpatialTypeOfSymbol( arg2 ) == stline )
    return 3;

  if ( SpatialTypeOfSymbol( arg1 ) == stregion &&
       SpatialTypeOfSymbol( arg2 ) == stregion )
    return 4;

  return -1; // This point should never be reached
}


int SpatialSetOpSelect(ListExpr args){
  string a1 = nl->SymbolValue(nl->First(args));
  string a2 = nl->SymbolValue(nl->Second(args));

  if(a1==Point::BasicType()){
    if(a2==Point::BasicType())      return 0;
    if(a2==Points::BasicType())     return 1;
    if(a2==Line::BasicType())       return 2;
    if(a2==Region::BasicType())     return 3;
    if(a2==SimpleLine::BasicType()) return 4;
    return -1;
  }
  if(a1==Points::BasicType()){
    if(a2==Point::BasicType())      return 5;
    if(a2==Points::BasicType())     return 6;
    if(a2==Line::BasicType())       return 7;
    if(a2==Region::BasicType())     return 8;
    if(a2==SimpleLine::BasicType()) return 9;
    return -1;
  }
  if(a1==Line::BasicType()){
    if(a2==Point::BasicType())      return 10;
    if(a2==Points::BasicType())     return 11;
    if(a2==Line::BasicType())       return 12;
    if(a2==Region::BasicType())     return 13;
    if(a2==SimpleLine::BasicType()) return 14;
    return -1;
  }

  if(a1==Region::BasicType()){
    if(a2==Point::BasicType())      return 15;
    if(a2==Points::BasicType())     return 16;
    if(a2==Line::BasicType())       return 17;
    if(a2==Region::BasicType())     return 18;
    if(a2==SimpleLine::BasicType()) return 19;
    return -1;
  }

  if (a1 == SimpleLine::BasicType()){
    if(a2==Point::BasicType())      return 20;
    if(a2==Points::BasicType())     return 21;
    if(a2==Line::BasicType())       return 22;
    if(a2==Region::BasicType())     return 23;
    if(a2==SimpleLine::BasicType()) return 24;
    return -1;
  }
  return -1;
}


/*
10.3.13 Selection function ~SpatialSelectDistance~

This select function is used for the ~distance~ operator.

*/
int getDistancePos(ListExpr arg){
  if(Point::checkType(arg)) return 0;
  if(Points::checkType(arg)) return 1;
  if(Line::checkType(arg)) return 2;
  if(SimpleLine::checkType(arg)) return 3;
  if(Rectangle<2>::checkType(arg)) return 4;
  if(Region::checkType(arg)) return 5;
  return -1;
}


int
SpatialSelectDistance( ListExpr args )
{
  ListExpr arg1 = nl->First( args );
  ListExpr arg2 = nl->Second( args );
  int p1 = getDistancePos(arg1);
  int p2 = getDistancePos(arg2);
  if(p1<0 || p2 < 0 ) return -1;
  return p1*6 + p2;
}

/*
10.3.15 Selection function ~SpatialSelectNoComponents~

This select function is used for the ~no\_components~ operator.

*/
int
SpatialSelectNoComponents( ListExpr args )
{
  ListExpr arg1 = nl->First( args );

  if (SpatialTypeOfSymbol( arg1 ) == stpoints)
    return 0;

  if (SpatialTypeOfSymbol( arg1 ) == stline)
    return 1;

  if (SpatialTypeOfSymbol( arg1 ) == stregion)
    return 2;

  return -1; // This point should never be reached
}

/*
10.3.16 Selection function ~SpatialSelectNoSegments~

This select function is used for the ~no\_segments~ operator.

*/
int
SpatialSelectNoSegments( ListExpr args )
{
  ListExpr arg1 = nl->First( args );
  if(Line::checkType(arg1) ) return 0;
  if(Region::checkType(arg1)) return 1;
  if(SimpleLine::checkType(arg1)) return 2;
  if(DLine::checkType(arg1)) return 3;
  return -1;
}

/*
10.3.16 Selection function ~SpatialSelectBBox~

This select function is used for the ~bbox~ operator.

*/
int
SpatialSelectBBox( ListExpr args )
{
  ListExpr arg1 = nl->First( args );

  if (SpatialTypeOfSymbol( arg1 ) == stpoint)
    return 0;

  if (SpatialTypeOfSymbol( arg1 ) == stpoints)
    return 1;

  if (SpatialTypeOfSymbol( arg1 ) == stline)
    return 2;

  if (SpatialTypeOfSymbol( arg1 ) == stregion)
    return 3;

  if (SpatialTypeOfSymbol( arg1 ) == stsline)
    return 4;
  if(Rectangle<2>::checkType(arg1)){
     return 5;
  }

  return -1; // This point should never be reached
}

/*
10.3.17 Selection function ~SpatialSelectTouchPoints~

This select function is used for the ~touchpoints~ operator.

*/
int
SpatialSelectTouchPoints( ListExpr args )
{
  ListExpr arg1 = nl->First( args );
  ListExpr arg2 = nl->Second( args );

  if ( SpatialTypeOfSymbol( arg1 ) == stline &&
       SpatialTypeOfSymbol( arg2 ) == stregion )
    return 0;

  if ( SpatialTypeOfSymbol( arg1 ) == stregion &&
       SpatialTypeOfSymbol( arg2 ) == stline )
    return 1;

  if ( SpatialTypeOfSymbol( arg1 ) == stregion &&
       SpatialTypeOfSymbol( arg2 ) == stregion )
    return 2;

  return -1; // This point should never be reached
}

/*
10.3.17 Selection function ~SpatialComponentsSelect~

This select function is used for the ~components~ operator.

*/
int
SpatialComponentsSelect( ListExpr args )
{
  ListExpr arg1 = nl->First( args );

  if ( SpatialTypeOfSymbol( arg1 ) == stpoints )
    return 0;

  if ( SpatialTypeOfSymbol( arg1 ) == stregion )
    return 1;

  if ( SpatialTypeOfSymbol( arg1 ) == stline )
    return 2;
  return -1; // This point should never be reached
}


/*
10.3.19 Selection function ~SpatialSelectWindowClipping~

This select function is used for the ~windowclipping(in)(out)~ operators.

*/
int
SpatialSelectWindowClipping( ListExpr args )
{
  ListExpr arg1 = nl->First( args );

  if (SpatialTypeOfSymbol( arg1 ) == stline)
    return 0;

  if (SpatialTypeOfSymbol( arg1 ) == stregion)
    return 1;

  return -1; // This point should never be reached
}

/*
10.3.19 Selection function ~SpatialVerticesSelect~

This select function is used for the ~vertices~ operator.

*/
int SpatialVerticesSelect( ListExpr args )
{
  if( nl->IsEqual(nl->First(args), Line::BasicType()) )
    return 0;

  if( nl->IsEqual(nl->First(args), Region::BasicType()) )
    return 1;

  return -1; // This point should never be reached
}

/*
10.3.19 Selection function ~SpatialBoundarySelect~

This select function is used for the ~vertices~ operator.

*/
int SpatialBoundarySelect( ListExpr args )
{
  if( nl->IsEqual(nl->First(args), Region::BasicType()) )
    return 0;

  if( nl->IsEqual(nl->First(args), Line::BasicType()) )
    return 1;

  return -1; // This point should never be reached
}

/*
10.3.20 Selection function for the simplify operator

*/
int SpatialSimplifySelect(ListExpr args){
   if(nl->ListLength(args)==2){ // line x real
      return 0;
   } else if (Points::checkType(nl->Third(args))){
      return 1; // line x real x points [x geoid]
   } else {
      return 0; //line x real x geoid
   }

}

int SpatialAtPointSelect(ListExpr args){
  if (nl->ListLength(args) == 3)
    return 0;
  if (nl->ListLength(args) == 2)
    return 1;
  return -1;
}

int SpatialAtPositionSelect(ListExpr args){
  if (nl->ListLength(args) == 3)
    return 0;
  if (nl->ListLength(args) == 2)
    return 1;
  return -1;
}

int SpatialSelectSize(ListExpr args){
   SpatialType st = SpatialTypeOfSymbol(nl->First(args));
   if(st==stline){
     return 0;
   }
   if(st==stregion){
     return 1;
   }
   if(st==stsline){
     return 2;
   }
   return -1;
}

int SpatialSelectCrossings(ListExpr args){
   SpatialType st = SpatialTypeOfSymbol(nl->First(args));
   if(nl->ListLength(args)==2){
      if(st==stline){
        return 0;
      }
      if(st==stsline){
        return 1;
      }
   } else { // one singe argument = line
      return 2;
   }
   return -1;
}


int utmSelect(ListExpr args){
  string t = nl->SymbolValue(nl->First(args));
  if(t==Point::BasicType()) return 0;
  if(t==Points::BasicType()) return 1;
  return -1;
}

int gkSelect(ListExpr args){
  string t = nl->SymbolValue(nl->First(args));
  if(t==Point::BasicType()) return 0;
  if(t==Points::BasicType()) return 1;
  if(t==Line::BasicType()) return 2;
  if(t==Region::BasicType()) return 3;
  if(t==SimpleLine::BasicType()) return 4;
  if(t==Rectangle<2>::BasicType()) return 5;
  return -1;
}

int reverseGkSelect(ListExpr args){
  string t = nl->SymbolValue(nl->First(args));
  if(t==Point::BasicType()) return 0;
  if(t==Points::BasicType()) return 1;
  if(t==Line::BasicType()) return 2;
  if(t==Region::BasicType()) return 3;
  return -1;
}


int SpatialCollectLineSelect(ListExpr args){
  ListExpr T = nl->Second(nl->First(args));
  if(listutils::isSymbol(T, Point::BasicType())) return 0;
  if(listutils::isSymbol(T, SimpleLine::BasicType())) return 1;
  if(listutils::isSymbol(T, Line::BasicType())) return 2;
  return -1;
}

int SpatialCollectPointsSelect(ListExpr args){
   ListExpr T = nl->Second(nl->First(args));
   if(listutils::isSymbol(T,Point::BasicType())) return 0;
   if(listutils::isSymbol(T,Points::BasicType())) return 1;
   return -1;
}


/*
10.4 Value mapping functions

A value mapping function implements an operator's main functionality: it takes
input arguments and computes the result. Each operator consists of at least
one value mapping function. In the case of overloaded operators there are
several value mapping functions, one for each possible combination of input
parameter types.

10.4.1 Value mapping functions of operator ~isempty~

*/
template<class T>
int SpatialIsEmpty(Word* args, Word& result, int message,
                   Word& local, Supplier s ){
  result = qp->ResultStorage( s );
  (static_cast<CcBool*>(result.addr))->Set(true,
              static_cast<T*>(args[0].addr)->IsEmpty());
  return 0;
}


/*
10.4.2 Value mapping functions of operator ~$=$~

*/
template<class T1, class T2>
int
SpatialEqual( Word* args, Word& result, int message,
              Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  CcBool* res = static_cast<CcBool*>(result.addr);
  T1* a1 = static_cast<T1*>(args[0].addr);
  T2* a2 = static_cast<T2*>(args[1].addr);
  if(!a1->IsDefined() || !a2->IsDefined()){
     res->Set(false,false);
     return 0;
  }
  bool e = (*a1) == (*a2);
  res->Set(true,e);
  return 0;
}

/*
10.4.2 Value mapping functions of operator ~$<$~

*/

template<class T1, class T2>
int SpatialIsLess(Word* args, Word& result, int message, Word& local,
                  Supplier s)
{
  result = qp->ResultStorage( s );
  CcBool* res = static_cast<CcBool*>(result.addr);
  T1* a1 = static_cast<T1*>(args[0].addr);
  T2* a2 = static_cast<T2*>(args[1].addr);
  if(!a1->IsDefined() || !a2->IsDefined()){
    res->Set(false,false);
    return 0;
  }
    bool e = (*a1) < (*a2);
    res->Set(true,e);
    return 0;
}

/*
10.4.3 Value mapping functions of operator ~$\neq$~

*/
template<class T>
int
SpatialNotEqual( Word* args, Word& result, int message,
                      Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  T* a1 = static_cast<T*>(args[0].addr);
  T* a2 = static_cast<T*>(args[1].addr);
  CcBool* res = static_cast<CcBool*>(result.addr);
  if(!a1->IsDefined() || !a2->IsDefined()){
     res->Set(false,false);
     return 0;
  }
  res->Set(true, (*a1) != (*a2));
  return 0;
}

/*
10.4.8 Value mapping functions of operator ~intersects~

*/
template<class A, class B,bool symm>
int SpatialIntersectsVM( Word* args, Word& result, int message,
                       Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  CcBool* res = static_cast<CcBool*>(result.addr);
  A* a;
  B* b;
  if(symm){
    a = static_cast<A*>(args[1].addr);
    b = static_cast<B*>(args[0].addr);
  } else {
    a = static_cast<A*>(args[0].addr);
    b = static_cast<B*>(args[1].addr);
  }

  if(!a->IsDefined() || !b->IsDefined()){
     res->Set(false,false);
  } else {
     res->Set(true,a->Intersects(*b));
  }
  return 0;
}


/*
10.4.9 Value mapping functions of operator ~inside~

*/
template<class First, class Second>
int SpatialInsideGeneric( Word* args, Word& result, int message,
                          Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  First*  arrgh1 = static_cast<First*>(args[0].addr);
  Second* arrgh2 = static_cast<Second*>(args[1].addr);
  if( arrgh1->IsDefined() && arrgh2->IsDefined() )
    ((CcBool *)result.addr)->Set( true, arrgh1->Inside( *arrgh2 ) );
  else
    ((CcBool *)result.addr)->SetDefined( false );
  return 0;
}

/*
10.4.10 Value mapping functions of operator ~adjacent~

*/
int
SpatialAdjacent_psr( Word* args, Word& result, int message,
                     Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  Points* ps = static_cast<Points*>(args[0].addr);
  Region* r  = static_cast<Region*>(args[1].addr);
  if( ps->IsDefined() && r->IsDefined() )
    ((CcBool *)result.addr)->Set( true, ps->Adjacent( *r ) );
  else
    ((CcBool *)result.addr)->SetDefined( false );
  return 0;
}

int
SpatialAdjacent_rps( Word* args, Word& result, int message,
                     Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  Region* r  = static_cast<Region*>(args[0].addr);
  Points* ps = static_cast<Points*>(args[1].addr);
  if( ps->IsDefined() && r->IsDefined() )
    ((CcBool *)result.addr)->Set( true, ps->Adjacent( *r ) );
  else
    ((CcBool *)result.addr)->SetDefined( false );
  return 0;
}

int
SpatialAdjacent_lr( Word* args, Word& result, int message,
                    Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  Line*   l = static_cast<Line*>(args[0].addr);
  Region* r = static_cast<Region*>(args[1].addr);
  if( l->IsDefined() && r->IsDefined() )
    ((CcBool *)result.addr)->Set( true, l->Adjacent( *r ) );
  else
    ((CcBool *)result.addr)->SetDefined( false );
  return 0;
}

int
SpatialAdjacent_rl( Word* args, Word& result, int message,
                    Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  Region* r = static_cast<Region*>(args[0].addr);
  Line*   l = static_cast<Line*>(args[1].addr);
  if( l->IsDefined() && r->IsDefined() )
    ((CcBool *)result.addr)->Set( true, l->Adjacent( *r ) );
  else
    ((CcBool *)result.addr)->SetDefined( false );
  return 0;
}

int
SpatialAdjacent_rr( Word* args, Word& result, int message,
                    Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  Region* r1 = static_cast<Region*>(args[0].addr);
  Region* r2 = static_cast<Region*>(args[1].addr);
  if( r1->IsDefined() && r2->IsDefined() )
    ((CcBool *)result.addr)->Set( true, r1->Adjacent( *r2 ) );
  else
    ((CcBool *)result.addr)->SetDefined( false );
  return 0;
}

/*
10.4.12 Value mapping functions of operator ~overlaps~

*/
int
SpatialOverlaps_rr( Word* args, Word& result, int message,
                    Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  Region* r1 = static_cast<Region*>(args[0].addr);
  Region* r2 = static_cast<Region*>(args[1].addr);
  if( r1->IsDefined() && r2->IsDefined() )
    ((CcBool *)result.addr)->Set( true,r1->Overlaps( *r2 ) );
  else
    ((CcBool *)result.addr)->SetDefined( false );
  return 0;
}

/*
10.4.13 Value mapping functions of operator ~onborder~

*/
int
SpatialOnBorder_pr( Word* args, Word& result, int message,
                    Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  Point*  p = static_cast<Point*>(args[0].addr);
  Region* r = static_cast<Region*>(args[1].addr);
  if( p->IsDefined() && r->IsDefined() )
    ((CcBool *)result.addr)->Set( true, r->OnBorder( *p ) );
  else
    ((CcBool *)result.addr)->Set( false, false );
  return 0;
}

/*
10.4.14 Value mapping functions of operator ~ininterior~

*/
int
SpatialInInterior_pr( Word* args, Word& result, int message,
                      Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  Point*  p = static_cast<Point*>(args[0].addr);
  Region* r = static_cast<Region*>(args[1].addr);
  if( p->IsDefined() && r->IsDefined() )
    ((CcBool *)result.addr)->
      Set( true, r->InInterior( *p ) );
  else
    ((CcBool *)result.addr)->Set( false, false );
  return 0;
}

/*
10.4.15 Value mapping functions of operator ~intersection~

*/
template<class A1, class A2, class R>
int SpatialIntersectionGeneric(Word* args, Word& result, int message,
                               Word& local, Supplier s){

  result = qp->ResultStorage( s );
  A1* arg1 = static_cast<A1*>(args[0].addr);
  A2* arg2 = static_cast<A2*>(args[1].addr);
  R* res = static_cast<R*>(result.addr);
  arg1->Intersection(*arg2, *res);
  return 0;
}







/*
10.4.16 Value mapping functions of operator ~minus~

*/

template<class A1, class A2, class R>
int SpatialMinusGeneric(Word* args, Word& result, int message,
                    Word& local, Supplier s){

  result = qp->ResultStorage( s );
  A1* arg1 = static_cast<A1*>(args[0].addr);
  A2* arg2 = static_cast<A2*>(args[1].addr);
  R* res = static_cast<R*>(result.addr);
  assert(arg1);
  assert(arg2);
  assert(res);
  arg1->Minus(*arg2, *res);
  return 0;
}


/*
10.4.17 Value mapping functions of operator ~union~

*/

template<class A1, class A2, class R>
int SpatialUnionGeneric(Word* args, Word& result, int message,
                    Word& local, Supplier s){

  result = qp->ResultStorage( s );
  A1* arg1 = static_cast<A1*>(args[0].addr);
  A2* arg2 = static_cast<A2*>(args[1].addr);
  R* res = static_cast<R*>(result.addr);
  arg1->Union(*arg2, *res);
  return 0;
}

/*
10.4.18 Value mapping functions of operator ~crossings~

*/
template<class Ltype>
int
SpatialCrossings( Word* args, Word& result, int message,
                     Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  Ltype *cl1=((Ltype*)args[0].addr),
        *cl2=((Ltype*)args[1].addr);
  cl1->Crossings( *cl2, *(Points*)result.addr );
  return 0;
}


int
SpatialCrossings_single( Word* args, Word& result, int message,
                         Word& local, Supplier s ){
  result = qp->ResultStorage( s );
  Line*  cl=((Line*)args[0].addr);
  cl->Crossings( *(Points*)result.addr );
  return 0;
}


/*
10.4.19 Value mapping functions of operator ~single~

*/
int
SpatialSingle_ps( Word* args, Word& result, int message,
                  Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  Points *ps = ((Points*)args[0].addr);
  Point p(true);
  if( ps->IsDefined() && (ps->Size() == 1) )
  {
    ps->Get( 0, p );
    *(Point*)result.addr = p;
  }
  else
    ((Point *)result.addr)->SetDefined( false );
  return 0;
}

/*
10.4.20 Value mapping functions of operator ~distance~

*/
template<class A, class B, bool symm>
int SpatialDistance( Word* args, Word& result, int message,
                     Word& local, Supplier s ){

   result = qp->ResultStorage( s );
   CcReal* res = static_cast<CcReal*>(result.addr);
   const Geoid* geoid =
                    (qp->GetNoSons(s) ==3)?static_cast<Geoid*>(args[2].addr):0;
   A* arg1=0;
   B* arg2=0;
   if(symm){
     arg1 = static_cast<A*>(args[1].addr);
     arg2 = static_cast<B*>(args[0].addr);
   } else {
     arg1 = static_cast<A*>(args[0].addr);
     arg2 = static_cast<B*>(args[1].addr);
   }
   if(!arg1->IsDefined() || !arg2->IsDefined() ||
      arg1->IsEmpty() || arg2->IsEmpty() || (geoid && !geoid->IsDefined()) ){
      res->SetDefined(false);
   } else {
     double dist = arg1->Distance(*arg2,geoid);
     res->Set(true,dist);
   }
   return 0;
}

/*
10.4.21 Value Mapping of operator ~distanceSmallerThan~

Up to now, it's only emplemented for the Line type.

*/
template<class A , class B>
int distanceSmallerThanVM(Word* args,
                          Word& result,
                          int message,
                          Word& local,
                          Supplier s){

   result = qp->ResultStorage(s);
   CcBool* res = (CcBool*) result.addr;

   A* a1 = (A*) args[0].addr;
   B* a2 = (B*) args[1].addr;
   CcReal* d = (CcReal*) args[2].addr;
   CcBool* b = (CcBool*) args[3].addr;

   if(!a1->IsDefined() || !a2->IsDefined() ||
      !d->IsDefined() || !b->IsDefined()){
      res->SetDefined(false);
      return 0;
   }
   a1->DistanceSmallerThan(*a2,d->GetValue(), b->GetValue(), *res);
   return 0;
}




/*
10.4.21 Value mapping function of operator ~direction~ and ~heading~

For ~heading~ the template parameter is ~true~, for ~direction~, is is ~false~.

*/
template<bool isHeading>
int SpatialDirectionHeading_pp( Word* args, Word& result, int message,
                                Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  CcReal* res = static_cast<CcReal*>(result.addr);
  const Point *p1 = ((Point*)args[0].addr);
  const Point *p2 = ((Point*)args[1].addr);
  const Geoid* geoid =
            (qp->GetNoSons(s)==3)?static_cast<const Geoid*>(args[2].addr):0;
  try{
    double d = p1->Direction(*p2, isHeading, geoid); // all saveguards included!
    res->Set(d>=0.0,d);
  } catch (SecondoException* e){
    res->Set(false,0.0);
    delete e;
  }
  return 0;
}

/*
10.4.22 Value mapping functions of operator ~nocomponents~

*/

int
SpatialNoComponents_ps( Word* args, Word& result, int message,
                        Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  const Points* ps = static_cast<const Points*>(args[0].addr);
  if( ps->IsDefined() )
    ((CcInt *)result.addr)->Set( true, ps->Size() );
  else
    ((CcInt *)result.addr)->Set( false, 0 );
  return 0;
}

int
SpatialNoComponents_l( Word* args, Word& result, int message,
                       Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  const Line* l = static_cast<const Line*>(args[0].addr);
  if( l->IsDefined() )
    ((CcInt *)result.addr)->Set( true, l->NoComponents() );
  else
    ((CcInt *)result.addr)->Set( false, 0 );
  return 0;
}

int
SpatialNoComponents_r( Word* args, Word& result, int message,
                       Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  const Region* r = static_cast<const Region*>(args[0].addr);
  if( r->IsDefined() )
    ((CcInt *)result.addr)->Set( true, r->NoComponents() );
  else
    ((CcInt *)result.addr)->Set( false, 0 );
  return 0;
}

/*
10.4.22 Value mapping functions of operator ~no\_segments~

*/
template<class T>
int
SpatialNoSegments( Word* args, Word& result, int message,
                     Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  const T *cl=((const T*)args[0].addr);
  if( cl->IsDefined() ) {
    assert( cl->Size() % 2 == 0 );
    ((CcInt *)result.addr)->Set( true, cl->Size() / 2 );
  } else {
    ((CcInt *)result.addr)->Set( false, 0 );
  }
  return 0;
}

int
SpatialNoSegmentsDline( Word* args, Word& result, int message,
                     Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  const DLine *dl=((const DLine*)args[0].addr);
  CcInt* res = (CcInt*)   result.addr;
  if(dl->IsDefined()){
    res->Set(true,dl->getSize());
  } else {
     res->SetDefined(false);
  }
  return 0;
}

/*
10.4.22 Value mapping functions of operator ~bbox~

*/
template<class T>
int SpatialBBox(Word* args, Word& result, int message,
            Word& local, Supplier s ){

  result = qp->ResultStorage( s );
  Rectangle<2>* box = static_cast<Rectangle<2>* >(result.addr);
  const T* arg = static_cast<const T*>(args[0].addr);
  const Geoid* geoid =
                (qp->GetNoSons(s)==2)?static_cast<const Geoid*>(args[1].addr):0;
  if(!arg->IsDefined() || (geoid && !geoid->IsDefined()) ){
    box->SetDefined(false);
  } else {
    (*box) = arg->BoundingBox(geoid);
  }
  return 0;
}


/*
10.4.23 Value mapping functions of operator ~size~

*/
template<class T>
int SpatialSize( Word* args, Word& result, int message,
                 Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  CcReal* res = static_cast<CcReal*>(result.addr);
  const T*  a = static_cast<const T*>(args[0].addr);
  const Geoid* g =
                (qp->GetNoSons(s)==2)?static_cast<const Geoid*>(args[1].addr):0;
  if(!a->IsDefined()){
    res->SetDefined(false);
    return 0;
  }
  if(g){ // variant using (LON,LAT)-coordinates
    if(!g->IsDefined()){
      res->Set(false, 0.0);
      return 0;
    }
    bool valid = false;
    res->Set(true,a->SpatialSize(*g, valid));
    res->SetDefined(valid);
    return 0;
  } // else: variant using (X,Y)-coordinates
  res->Set(true,a->SpatialSize());
  return 0;
}

/*
10.4.24 Value mapping functions of operator ~touchpoints~

*/
int SpatialTouchPoints_lr( Word* args, Word& result, int message,
                           Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  const Line *l = ((const Line*)args[0].addr);
  const Region *r = ((const Region*)args[1].addr);
  r->TouchPoints( *l, *((Points *)result.addr) );
  return 0;
}

int SpatialTouchPoints_rl( Word* args, Word& result, int message,
                           Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  const Region *r = ((const Region*)args[0].addr);
  const Line   *l = ((const Line*)args[1].addr);
  r->TouchPoints( *l, *((Points *)result.addr) );
  return 0;
}

int SpatialTouchPoints_rr( Word* args, Word& result, int message,
                           Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  const Region *r1 = ((const Region*)args[0].addr);
  const Region *r2 = ((const Region*)args[1].addr);
  r1->TouchPoints( *r2, *((Points *)result.addr) );
  return 0;
}




ostream& operator<<(ostream& o,const SimplePoint& p) {
        o << "(" << p.getX() << ", " << p.getY() << ")";
        return o;
}


/*
10.4.25 Value mapping functions of operator ~commomborder~

*/

int SpatialCommonBorder_rr( Word* args, Word& result, int message,
                            Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  Line *l = (Line*)result.addr;
  const Region *cr1 = ((const Region*)args[0].addr),
               *cr2 = ((const Region*)args[1].addr);
  cr1->CommonBorder( *cr2, *l );

  return 0;
}


/*

10.3 Operator ~translate~

10.3.1  Type mapping function for operator ~translate~

*/
ListExpr
SpatialTranslateMap( ListExpr args )
{
  string err = "SPATIAL x (real x real) expected";
  if(!nl->HasLength(args,2)){
    return listutils::typeError(err);
  }

  ListExpr a1 = nl->First(args);
  if(   !Point::checkType(a1)
     && !Points::checkType(a1)
     && !Line::checkType(a1)
     && !Region::checkType(a1)
     && !DLine::checkType(a1)){
     return listutils::typeError(err + " (first arg not supported)");
   }
   ListExpr a2 = nl->Second(args);
   if(!nl->HasLength(a2,2)){
     return listutils::typeError("invalid number od arguments");
   }
   if(   !CcReal::checkType(nl->First(a2))
      || !CcReal::checkType(nl->Second(a2))){
     return listutils::typeError("translation arguments must be of type real");
   }
   return a1;
}

/*
10.3.2 Value Mapping template

*/
template<class T>
int SpatialTranslateVMT( Word* args, Word& result, int message,
                        Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  T* res = static_cast<T*>(result.addr);
  const T* a = (const T*)args[0].addr;

  Supplier son = qp->GetSupplier( args[1].addr, 0 );
  Word t;
  qp->Request( son, t );
  const CcReal *tx = ((CcReal *)t.addr);

  son = qp->GetSupplier( args[1].addr, 1 );
  qp->Request( son, t );
  const CcReal *ty = ((CcReal *)t.addr);

  if(!a->IsDefined() || !tx->IsDefined() || !ty->IsDefined()){
    res->SetDefined(false);
  } else {
     a->Translate(tx->GetValue(), ty->GetValue(), *res);
  }
  return 0;
}

ValueMapping spatialtranslatemap[] = {
  SpatialTranslateVMT<Point>,
  SpatialTranslateVMT<Points>,
  SpatialTranslateVMT<Line>,
  SpatialTranslateVMT<Region>,
  SpatialTranslateVMT<DLine> };

/*
10.3.19 Selection function ~SpatialSelectTranslate~

This select function is used for the ~translate~, rotate,
and ~scale~ operators.

*/
int SpatialSelectTranslate( ListExpr args ) {
  ListExpr arg1 = nl->First( args );
  if(Point::checkType(arg1)) return 0;
  if(Points::checkType(arg1)) return 1;
  if(Line::checkType(arg1)) return 2;
  if(Region::checkType(arg1)) return 3;
  if(DLine::checkType(arg1)) return 4;
  return -1;
}

const string SpatialSpecTranslate  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>(point||points||line||region x real x real) -> "
  "point||points||line||region</text--->"
  "<text> _ translate[ dx, dy ]</text--->"
  "<text> move the object parallely for some distance.</text--->"
  "<text> query region1 translate[3.5, 15.1]</text--->"
  ") )";


Operator spatialtranslate (
  "translate",
  SpatialSpecTranslate,
  5,
  spatialtranslatemap,
  SpatialSelectTranslate,
  SpatialTranslateMap );



/*
10.4 Operator ~rotate~

10.4.1  Type mapping 

*/
ListExpr
SpatialRotateMap( ListExpr args )
{
  if ( nl->ListLength( args ) != 4 ) {
    return listutils::typeError("wrong number of args");
  }
  // first arg must be of type point, points, line, region, dline

  ListExpr a1 = nl->First(args);
  if(   !Point::checkType(a1)
     && !Points::checkType(a1)
     && !Line::checkType(a1)
     && !Region::checkType(a1)
     && !DLine::checkType(a1)){
    return listutils::typeError("first arg is not a supported spatial type");
  }
  // the remaining arguments have to be of type real
  if(   !CcReal::checkType(nl->Second(args))
     || !CcReal::checkType(nl->Third(args))
     || !CcReal::checkType(nl->Fourth(args))){
     return listutils::typeError("rotation arguments have to be of type real");
  }
  return a1;
}

template<class T>
int SpatialRotate( Word* args, Word& result, int message,
                    Word& local, Supplier s ){
  result = qp->ResultStorage(s);
  T* res = static_cast<T*>(result.addr);
  T* st = static_cast<T*>(args[0].addr);
  CcReal* x = static_cast<CcReal*>(args[1].addr);
  CcReal* y = static_cast<CcReal*>(args[2].addr);
  CcReal* a = static_cast<CcReal*>(args[3].addr);
  if(!st->IsDefined() || !x->IsDefined() || !y->IsDefined()
     || !a->IsDefined()){
      res->SetDefined(false);
      return 0;
  }
  double angle = a->GetRealval() * M_PI / 180;
  st->Rotate(x->GetRealval(),y->GetRealval(),angle,*res);
  return 0;
}


ValueMapping spatialrotatemap[] = {
  SpatialRotate<Point>,
  SpatialRotate<Points>,
  SpatialRotate<Line>,
  SpatialRotate<Region>,
  SpatialRotate<DLine>};


int SpatialSelectRotate( ListExpr args ) {
  ListExpr arg1 = nl->First( args );
  if(Point::checkType(arg1)) return 0;
  if(Points::checkType(arg1)) return 1;
  if(Line::checkType(arg1)) return 2;
  if(Region::checkType(arg1)) return 3;
  if(DLine::checkType(arg1)) return 4;
  return -1;
}

const string SpatialSpecRotate  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>(point|points|line|region|dline x real x real x real) -> "
  "point||points||line||region|dline</text--->"
  "<text> _ translate[ x, y, theta ]</text--->"
  "<text> rotates the spatial object by 'theta' degrees around (x,y) </text--->"
  "<text> query region1 rotate[3.5, 15.1, 10.0]</text--->"
  ") )";


Operator spatialrotate (
  "rotate",
  SpatialSpecRotate,
  5,
  spatialrotatemap,
  SpatialSelectRotate,
  SpatialRotateMap );



/*
10.5 Operator ~scale~

10.5.1 Type Mapping

*/
ListExpr SpatialScaleMap(ListExpr args) {
  string err = "SPATIAL x (real x real) or SPATIAL x real expected";
  if (!nl->HasLength(args, 3) && !nl->HasLength(args, 2)) {
    return listutils::typeError(err);
  }
  ListExpr a1 = nl->First(args);
  if(   !Point::checkType(a1)
     && !Points::checkType(a1)
     && !Line::checkType(a1)
     && !Region::checkType(a1)
     && !DLine::checkType(a1)){
     return listutils::typeError(err + " (first arg not supported)");
   }
   if (!CcReal::checkType(nl->Second(args))) {
     return listutils::typeError("translation argument(s) must have type real");
   }
   if (nl->HasLength(args, 3)) {
     if (!CcReal::checkType(nl->Third(args))) {
       return listutils::
                       typeError("translation argument(s) must have type real");
     }
   }
   return a1;
}

template<class T>
int SpatialScaleVMT3( Word* args, Word& result, int message,
                        Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  T* res = static_cast<T*>(result.addr);
  const T* a = (const T*)args[0].addr;
  const CcReal *tx = (CcReal *) args[1].addr;
  const CcReal *ty = (CcReal *) args[2].addr;
  if(!a->IsDefined() || !tx->IsDefined() || !ty->IsDefined()){
    res->SetDefined(false);
  } else {
     a->Scale(tx->GetValue(), ty->GetValue(), *res);
  }
  return 0;
}

template<class T>
int SpatialScaleVMT2(Word* args, Word& result, int message, Word& local,
                     Supplier s) {
  result = qp->ResultStorage( s );
  T* res = static_cast<T*>(result.addr);
  const T* a = (const T*)args[0].addr;
  const CcReal *t = (CcReal*) args[1].addr;
  if(!a->IsDefined() || !t->IsDefined()){
    res->SetDefined(false);
  } 
  else {
     a->Scale(t->GetValue(), t->GetValue(), *res);
  }
  return 0;
}

ValueMapping spatialscalemap[] = {
  SpatialScaleVMT3<Point>,
  SpatialScaleVMT3<Points>,
  SpatialScaleVMT3<Line>,
  SpatialScaleVMT3<Region>,
  SpatialScaleVMT3<DLine>,
  SpatialScaleVMT2<Point>,
  SpatialScaleVMT2<Points>,
  SpatialScaleVMT2<Line>,
  SpatialScaleVMT2<Region>,
  SpatialScaleVMT2<DLine>};


int SpatialSelectScale( ListExpr args ) {
  ListExpr arg1 = nl->First(args);
  if (nl->HasLength(args, 3)) {
    if(Point::checkType(arg1)) return 0;
    if(Points::checkType(arg1)) return 1;
    if(Line::checkType(arg1)) return 2;
    if(Region::checkType(arg1)) return 3;
    if(DLine::checkType(arg1)) return 4;
  }
  else {
    if(Point::checkType(arg1)) return 5;
    if(Points::checkType(arg1)) return 6;
    if(Line::checkType(arg1)) return 7;
    if(Region::checkType(arg1)) return 8;
    if(DLine::checkType(arg1)) return 9;
  }
  return -1;
}

const string SpatialSpecScale  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>for T in {point, points, line, region}: "
  "T x real (x real) -> T</text--->"
  "<text> _ scale [ _ , _ ] </text--->"
  "<text> scales an object by the given factor. If the user specifies two real "
  "values, each of them represents the scale factor for one dimension</text--->"
  "<text> query region1 scale[1000.0]</text--->"
  ") )";


Operator spatialscale (
  "scale",
  SpatialSpecScale,
  10,
  spatialscalemap,
  SpatialSelectScale,
  SpatialScaleMap
 );





int SpatialCenter( Word* args, Word& result, int message,
                   Word& local, Supplier s ){
   result = qp->ResultStorage(s);
   Points* ps = static_cast<Points*>(args[0].addr);
   Point* res = static_cast<Point*>(result.addr);
   *res = ps->theCenter();
   return 0;
}

int SpatialConvexhull( Word* args, Word& result, int message,
                   Word& local, Supplier s ){
   result = qp->ResultStorage(s);
   Points* ps = static_cast<Points*>(args[0].addr);
   Region* res = static_cast<Region*>(result.addr);
   GrahamScan::convexHull(ps,res);
   return 0;
}

int
SpatialLine2Region( Word* args, Word& result, int message,
                    Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  Line *cl = (Line *)args[0].addr;
  Region *pResult = (Region *)result.addr;
  pResult->Clear();
  cl->Transform( *pResult );
  return 0;
}

/*
10.4.29 Value mapping function of operator ~rect2region~

*/

int SpatialRect2Region( Word* args, Word& result, int message,
                        Word& local, Supplier s )
{
  result = qp->ResultStorage( s );

  Rectangle<2> *rect = (Rectangle<2> *)args[0].addr;
  Region *res = (Region*)result.addr;
  *res = Region( *rect );
  return 0;
}

/*
10.4.29 Value mapping function of operator ~create_triangle~

*/

int SpatialCreateTriangleVM( Word* args, Word& result, int message,
                           Word& local, Supplier s ){
  result = qp->ResultStorage( s );
  Region* res = static_cast<Region*>(result.addr);
  const Point* p1 = static_cast<const Point*>(args[0].addr);
  const Point* p2 = static_cast<const Point*>(args[1].addr);
  const Point* p3 = static_cast<const Point*>(args[2].addr);
  *res = Region(*p1, *p2, *p3);
  return 0;
}

int SpatialArea( Word* args, Word& result, int message,
                 Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  Region *reg = (Region *)args[0].addr;
  CcReal *res = (CcReal *)result.addr;
  if(  reg->IsDefined() )
    res->Set( true, reg->Area() );
  else
    res->Set( false, 0.0 );
  return 0;
}

int
SpatialTranslate_r( Word* args, Word& result, int message,
                    Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  Region *cr = (Region *)args[0].addr,
          *pResult = (Region *)result.addr;
  pResult->Clear();
  Supplier son = qp->GetSupplier( args[1].addr, 0 );

  Word t;
  qp->Request( son, t );
  const CcReal *tx = ((CcReal *)t.addr);

  son = qp->GetSupplier( args[1].addr, 1 );
  qp->Request( son, t );
  const CcReal *ty = ((CcReal *)t.addr);

  if(  cr->IsDefined() && tx->IsDefined() && ty->IsDefined() ) {
      const Coord txval = (Coord)(tx->GetRealval()),
                  tyval = (Coord)(ty->GetRealval());
      cr->Translate( txval, tyval, *pResult );
  }
  else
    ((Region*)result.addr)->SetDefined( false );

  return 0;
}

/*
10.4.26 Value mapping functions of operator ~windowclippingin~

*/
int
SpatialWindowClippingIn_l( Word* args, Word& result, int message,
                           Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  Line *clippedLine = (Line*)result.addr;
  Line *l = ((Line *)args[0].addr);
  Rectangle<2> *window = ((Rectangle<2>*)args[1].addr);
  bool inside;
  l->WindowClippingIn(*window,*clippedLine,inside);
  return 0;
}

int
SpatialWindowClippingIn_r( Word* args, Word& result, int message,
                           Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  Region *clippedRegion=(Region*)result.addr;
  Region *r = ((Region *)args[0].addr);
  Rectangle<2> *window = ((Rectangle<2>*)args[1].addr);
  r->WindowClippingIn(*window,*clippedRegion);
  return 0;

}

/*
10.4.26 Value mapping functions of operator ~windowclippingout~

*/
int
SpatialWindowClippingOut_l( Word* args, Word& result, int message,
                            Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  Line *clippedLine = (Line*)result.addr;
  Line *l = ((Line *)args[0].addr);
  Rectangle<2> *window = ((Rectangle<2>*)args[1].addr);
  bool outside;
  l->WindowClippingOut(*window,*clippedLine,outside);
  return 0;
}

int
SpatialWindowClippingOut_r( Word* args, Word& result, int message,
                            Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  Region *clippedRegion=(Region*)result.addr;
  Region *r = ((Region *)args[0].addr);
  Rectangle<2> *window = ((Rectangle<2>*)args[1].addr);
  r->WindowClippingOut(*window,*clippedRegion);
  return 0;
}




/*
10.4.27 Value mapping functions of operator ~components~

*/
struct ComponentsLocalInfo
{
  vector<Region*> components;
  vector<Region*>::iterator iter;
};

int
SpatialComponents_r( Word* args, Word& result, int message,
                     Word& local, Supplier s )
{
  ComponentsLocalInfo *localInfo;

  switch( message )
  {
    case OPEN:
      if( !((Region*)args[0].addr)->IsEmpty() ){ // IsEmpty() subsumes undef
        localInfo = new ComponentsLocalInfo();
        ((Region*)args[0].addr)->Components( localInfo->components );
        localInfo->iter = localInfo->components.begin();
        local.setAddr( localInfo );
      } else {
        local.setAddr( 0 );
      }
      return 0;

    case REQUEST:
      if( !local.addr ) {
        return CANCEL;
      }
      localInfo = (ComponentsLocalInfo*)local.addr;
      if( localInfo->iter == localInfo->components.end() )
        return CANCEL;
      result.setAddr( *localInfo->iter++ );
      return YIELD;

    case CLOSE:

      if(local.addr)
      {
        localInfo = (ComponentsLocalInfo*)local.addr;
        while( localInfo->iter != localInfo->components.end() )
        {
          delete *localInfo->iter++;
        }
        delete localInfo;
        local.setAddr(0);
      }
      return 0;
  }
  return 0;
}

int
SpatialComponents_ps( Word* args, Word& result, int message,
                      Word& local, Supplier s )
{
  Points *localInfo;

  switch( message )
  {
    case OPEN:{
      Points* arg = static_cast<Points*>(args[0].addr);
      if( !arg->IsEmpty() ){ // subsumes undef
         localInfo = new Points(*arg);
         localInfo->SelectFirst();
         local.setAddr(localInfo);
      } else {
         local.setAddr(0);
      }
      return 0;
    }
    case REQUEST: {
      if(!local.addr){
        return CANCEL;
      }
      localInfo = (Points*)local.addr;
      if(localInfo->EndOfPt() ){
        return CANCEL;
      }
      Point p(false);
      localInfo->GetPt( p );
      Points* res = new Points(1);
      res->StartBulkLoad();
      (*res) += p;
      res->EndBulkLoad();
      result.addr = res;
      localInfo->SelectNext();
      return YIELD;
    }
    case CLOSE: {
      if(local.addr)
      {
        localInfo = (Points*)local.addr;
        delete localInfo;
        local.setAddr(0);
      }
      return 0;
    }
  }
  return 0;
}


class LineComponentsLi{
public:
  LineComponentsLi(Line* theLine){
    this->theLine = (Line*)theLine->Copy();
    size = theLine->Size();
    used = new bool[size];
    for(int i=0;i<size;i++){
       used[i] = false;
    }
    pos = 0;
  }

  ~LineComponentsLi(){
     theLine->DeleteIfAllowed();
     delete[] used;
   }

  Line* NextLine(){
     // search next unused part
     while(pos<size && used[pos]){
         pos++;
     }
     // all segments are used already
     if(pos>=size){
        return 0;
     }
     Line* result = new Line(size-pos);
     result->StartBulkLoad();
     int edgeno = 0;
     // pos points to an unused segments
     stack<int> criticalPoints;
     bool done=false;
     HalfSegment hs;
     int hspos = pos;
     HalfSegment hsp;
     int hsppos = -1;
     criticalPoints.push(pos); // mark to search an extension here
     while(!done){
        theLine->Get(hspos,hs);
        hsppos = hs.attr.partnerno;
        theLine->Get(hsppos,hsp);
        used[hspos]=true;
        used[hsppos]=true;
        HalfSegment hs1 = hs;
        hs1.attr.edgeno = edgeno;
        hs1.SetLeftDomPoint(false);
        (*result) += hs1;
        hs1.SetLeftDomPoint(true);
        (*result) += hs1;
        edgeno++;
        // search an extension of result
        Point p = hsp.GetDomPoint();
        criticalPoints.push(hsppos);

        // search within the stack
        bool found = false;
        while(!criticalPoints.empty() && !found){
          int k = criticalPoints.top();
          HalfSegment tmp;
          theLine->Get(k,tmp);
          Point p = tmp.GetDomPoint();
          // search left of k
          int m = k-1;
          while(m>0 && isDomPoint(p,m) && !found){
             found = !used[m];
             if(!found) m--;
          }
          if(found){
              hspos=m;
          } else { // search right of k
             m = k+1;
             while(m<size && isDomPoint(p,m) &&!found){
                found = !used[m];
                if(!found) m++;
             }
             if(found){
               hspos = m;
             }
          }

          if(!found){
             criticalPoints.pop();
          }
        }
        done = !found; // no extension found
     }
     result->EndBulkLoad();
     return result;
  }


private:
   bool isDomPoint(const Point& p,int pos){
     HalfSegment hs;
     theLine->Get(pos,hs);
     return AlmostEqual(p,hs.GetDomPoint());
   }


   Line* theLine;
   int pos;
   int size;
   bool* used;
};


int
SpatialComponents_l( Word* args, Word& result, int message,
                      Word& local, Supplier s ){

   switch(message){
     case OPEN:{
       if( !((Line*)(args[0].addr))->IsEmpty() ) {
        local.addr = new LineComponentsLi((Line*) args[0].addr);
       } else {
         local.setAddr( 0 );
       }
       return 0;
     }

     case REQUEST:{
         if( !local.addr )
           return CANCEL;
         LineComponentsLi* li = (LineComponentsLi*) local.addr;
         Line* res = li->NextLine();
         if(res==0){
            return CANCEL;
         } else {
            result.addr = res;
            return YIELD;
         }
     }
     case CLOSE:{
       if(local.addr){
          LineComponentsLi* li = (LineComponentsLi*) local.addr;
          delete li;
          local.setAddr(0);
       }
       return 0;
     }
   }
   return 0;
}


/*
10.4.28 Value mapping functions of operator ~vertices~

*/
int SpatialVertices_r(Word* args, Word& result, int message,
                      Word& local, Supplier s )
{
  result = qp->ResultStorage(s);
  Region* reg = (Region*)args[0].addr;
  Points* res = (Points*)result.addr;
  reg->Vertices(res);
  return 0;
}

int SpatialVertices_l(Word* args, Word& result, int message,
                      Word& local, Supplier s )
{
  result = qp->ResultStorage(s);
  ((Line*)args[0].addr)->Vertices( (Points*) result.addr );
  return 0;
}


/*
10.4.28 Value mapping functions of operator ~boundary~

*/

int SpatialBoundary_r(Word* args, Word& result, int message,
                      Word& local, Supplier s )
{
  result = qp->ResultStorage(s);
  Region* reg = (Region*)args[0].addr;
  Line* res = (Line*) result.addr;
  reg->Boundary(res);
  return 0;
}
int SpatialBoundary_l(Word* args, Word& result, int message,
                      Word& local, Supplier s )
{
  result = qp->ResultStorage(s);
  Line* line = (Line*)args[0].addr;
  Points* res = (Points*) result.addr;
  line->Boundary(res);
  return 0;
}

/*
10.4.23 Value mapping functions of operator ~atpoint~

*/
int
SpatialAtPointBool( Word* args, Word& result, int message,
                Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  SimpleLine *l = (SimpleLine*)args[0].addr;
  Point *p = (Point*)args[1].addr;
  CcBool *startsSmaller = (CcBool*)args[2].addr;
  double res;

  if( !l->IsEmpty() && // subsumes IsDefined()
      p->IsDefined() &&
      startsSmaller->IsDefined() &&
      l->AtPoint( *p, startsSmaller->GetBoolval(), 0.0, res ) )
    ((CcReal*)result.addr)->Set( true, res );
  else
    ((CcReal*)result.addr)->SetDefined( false );
  return 0;
}

int
SpatialAtPoint( Word* args, Word& result, int message,
                Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  SimpleLine *l = (SimpleLine*)args[0].addr;
  Point *p = (Point*)args[1].addr;
  double res;
  if( l->IsDefined() && !l->IsEmpty() &&
      p->IsDefined() && l->AtPoint( *p, res ) )
    ((CcReal*)result.addr)->Set( true, res );
  else
    ((CcReal*)result.addr)->SetDefined( false );
  return 0;
}

/*
10.4.23 Value mapping functions of operator ~atposition~

*/
int
SpatialAtPositionBool( Word* args, Word& result, int message,
                       Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  SimpleLine *l = (SimpleLine*)args[0].addr;
  CcReal *pos = (CcReal*)args[1].addr;
  CcBool *startsSmaller = (CcBool*)args[2].addr;
  Point *p = (Point*)result.addr;

  if( l->IsEmpty() || !pos->IsDefined() ||
      !startsSmaller->IsDefined() ||
      !l->AtPosition( pos->GetRealval(), startsSmaller->GetBoolval(), *p ) )
    p->SetDefined( false );

  return 0;
}

int
SpatialAtPosition( Word* args, Word& result, int message,
                       Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  SimpleLine *l = (SimpleLine*)args[0].addr;
  CcReal *pos = (CcReal*)args[1].addr;
  Point *p = (Point*)result.addr;

  if( l->IsEmpty() || !pos->IsDefined() ||
      !l->AtPosition( pos->GetRealval(), *p ) )
    p->SetDefined( false );

  return 0;
}

/*
10.4.23 Value mapping functions of operator ~subline~

*/
int
SpatialSubLine( Word* args, Word& result, int message,
                Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  SimpleLine *l = (SimpleLine*)args[0].addr;
  CcReal *pos1 = (CcReal*)args[1].addr,
         *pos2 = (CcReal*)args[2].addr;
  CcBool *startsSmaller = (CcBool*)args[3].addr;
  SimpleLine *rLine = (SimpleLine*)result.addr;

  if( pos1->IsDefined() &&
      pos2->IsDefined() &&
      startsSmaller->IsDefined() )
    l->SubLine( pos1->GetRealval(),
                pos2->GetRealval(),
                startsSmaller->GetBoolval(),
                *rLine );
  else {
    rLine->Clear();
    rLine->SetDefined( false );
  }
  return 0;
}

/*
10.4.26 Value mapping function of operator ~add~

*/
int
SpatialAdd_p( Word* args, Word& result, int message,
                    Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  const Point *p1= (Point*)args[0].addr;
  const Point *p2= (Point*)args[1].addr;

  if( p1->IsDefined() && p2->IsDefined() )
    *((Point*)result.addr) = *p1 + *p2 ;
  else
    ((Point*)result.addr)->SetDefined( false );

  return 0;
}

/*
10.4.27 Value mapping function of operator ~getx~

*/
int
SpatialGetX_p( Word* args, Word& result, int message,
                    Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  const Point *p = (Point*)args[0].addr;

  if( p->IsDefined() )
    ((CcReal*)result.addr)->Set( true, p->GetX() ) ;
  else
    ((CcReal*)result.addr)->Set( false, 0.0 );

  return 0;
}

/*
10.4.28 Value mapping function of operator ~gety~

*/
int
SpatialGetY_p( Word* args, Word& result, int message,
                    Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  const Point *p = (Point*)args[0].addr;

  if( p->IsDefined() )
    ((CcReal*)result.addr)->Set( true, p->GetY() ) ;
  else
    ((CcReal*)result.addr)->Set( false, 0.0 );

  return 0;
}


/*
bool AlmostEqual2(const Point& p1, const Point& p2 ){
  double z1 = abs(p1.GetX());
  double z2 = abs(p1.GetY());
  double z3 = abs(p2.GetX());
  double z4 = abs(p2.GetY());
  double Min = min(min(z1,z2) , min(z3,z4));
  double eps = max(FACTOR, FACTOR*Min);
  if(abs(z1-z3)>eps) return false;
  if(abs(z2-z4)>eps) return false;
  return true;
}
*/


template<bool allowCycles>
int SpatialPolylines(Word* args, Word& result, int message,
                    Word& local, Supplier s){

   LineSplitter<DbArray > *localinfo;
   Line   *l, *res;
   CcBool *b;

   result = qp->ResultStorage(s);
   switch (message){
      case OPEN:
          l = (Line*)args[0].addr;
          b = (CcBool*)args[1].addr;
          if(qp->GetNoSons(s)==2){
             if( !l->IsEmpty() && b->IsDefined() ) {
              local.setAddr(new LineSplitter<DbArray > (l,
                                  b->GetBoolval(),
                                  allowCycles));
             } else {
               local.setAddr( 0 );
             }
          } else if(    !l->IsEmpty()
                     && b->IsDefined()
                     && ((Points*)args[2].addr)->IsDefined() ){
            local.setAddr(new LineSplitter<DbArray >(l,
                            b->GetBoolval(),
                            allowCycles,
                            ((Points*)args[2].addr)));
          } else {
            local.setAddr( 0 );
          }
          return 0;
      case REQUEST:
           if( !local.addr ){
             return CANCEL;
           }
           localinfo = (LineSplitter<DbArray >*) local.addr;
           res = localinfo->NextLine();
           if(res==0){
              return CANCEL;
           } else {
              result.setAddr(res);
              return YIELD;
           }
      case CLOSE:
           if(local.addr!=0){
             localinfo = (LineSplitter<DbArray >*) local.addr;
             delete localinfo;
             local.setAddr(0);
           }
           return 0;
   }
   return 0; // ignore unknown message
}




/*
10.4.30 Value Mapping for the ~segments~ operator

*/
template<class LT>
class SegmentsInfo{
  public:
    SegmentsInfo(LT* line){
       this->theLine = line; // increase the ref counter of line
       this->position = 0;
       this->size = line->Size();
    }
    ~SegmentsInfo(){ }


    LT* NextSegment(){
       return next(theLine);
    }

  private:
     int position;
     int size;
     LT* theLine;


    Line* next(Line* line) {
       HalfSegment hs;
       // search for a segment with left dominating point
       bool found = false;
       while((position<size) && !found){
             theLine->Get(position,hs);
             if(hs.IsLeftDomPoint()){
                found=true;
             } else {
                position++;
             }
       }
       position++; // for the next run
       if(!found){ // no more segments available
          return 0;
       } else {
          Line* res = new Line(2);
          HalfSegment hs1 = hs;
          res->StartBulkLoad();
          hs1.attr.edgeno = 0;
          (*res) += hs1;
          hs1.SetLeftDomPoint(false);
          (*res) += hs1;
          res->EndBulkLoad();
          return res;
       }
    }

    DLine* next(DLine* line){
       if(position>=size){
           return 0;
       }
       SimpleSegment s;
       line->get(position,s);
       position++;
       DLine* res = new DLine(1);
       res->append(s);
       return res;
    }

};

template<class LT>
int SpatialSegmentsVMT(Word* args, Word& result, int message,
                    Word& local, Supplier s){

 SegmentsInfo<LT>* si= (SegmentsInfo<LT>*) local.addr;;
 switch(message){
    case OPEN:
      if(si){
         delete si;
         local.addr = 0;
      }
      if( !((LT*)args[0].addr)->IsEmpty() ){ // subsumes undef
        local.addr = new SegmentsInfo<LT>((LT*)args[0].addr);
      }
      return 0;

   case REQUEST:
      result.addr = si?si->NextSegment():0;
      return result.addr?YIELD:CANCEL;

    case CLOSE:
      if(si)
      {
        delete si;
        local.setAddr(0);
      }
      return 0;
 }
 return 0;
}

ValueMapping SpatialSegmentsVM[] = {
   SpatialSegmentsVMT<Line>,
   SpatialSegmentsVMT<DLine>
};

int SpatialSegmentsSelect(ListExpr args){
  return Line::checkType(nl->First(args))?0:1;
}




/*
10.4.31 Value Mappings for the simplify operator

*/

int SpatialSimplify_LReal(Word* args, Word& result, int message,
                    Word& local, Supplier s){
   result = qp->ResultStorage(s);
   Line*   res     = static_cast<Line*>(result.addr);
   Line*   line    = static_cast<Line*>(args[0].addr);
   CcReal* epsilon = static_cast<CcReal*>(args[1].addr);
   Geoid* geoid = 0;
   if(qp->GetNoSons(s)==3){
      geoid = static_cast<Geoid*>(args[2].addr);
      if(!geoid->IsDefined()){
         res->SetDefined(false);
         return 0;
      }
   }
   if( line->IsDefined() && epsilon->IsDefined() ){
     Points* ps= new Points(0);
     line->Simplify( *res, epsilon->GetRealval(),*ps,geoid );
     ps->DeleteIfAllowed();
   } else {
      res->SetDefined( false );
   }
   return 0;
}

int SpatialSimplify_LRealPs(Word* args, Word& result, int message,
                    Word& local, Supplier s){
   result = qp->ResultStorage(s);
   Line*   res     = static_cast<Line*>(result.addr);
   Line*   line    = static_cast<Line*>(args[0].addr);
   CcReal* epsilon = static_cast<CcReal*>(args[1].addr);
   Points* ps      = static_cast<Points*>(args[2].addr);
   Geoid* geoid = 0;
   if(qp->GetNoSons(s)==4){
      geoid = static_cast<Geoid*>(args[3].addr);
       if(!geoid->IsDefined()){
          res->SetDefined(0);
          return 0;
       }
   }
   if( line->IsDefined() && epsilon->IsDefined() && ps->IsDefined() )
     line->Simplify( *res, epsilon->GetRealval(), *ps,geoid);
   else
     res->SetDefined( false );
   return 0;
}


/*
10.4.32 Value Mapping for the ~get~ operator

*/
int SpatialGet(Word* args, Word& result, int message,
               Word& local, Supplier s){

   result = qp->ResultStorage(s);
   Points* ps = (Points*) args[0].addr;
   CcInt*  index = (CcInt*) args[1].addr;
   if(!ps->IsDefined() || !index->IsDefined()){
      ((Point*)result.addr)->SetDefined(false);
      return 0;
   }
   int i = index->GetIntval();
   if(i<0 || i >= ps->Size()){
      ((Point*)result.addr)->SetDefined(false);
      return 0;
   }

   Point p(true);
   ps->Get(i,p);
   ((Point*)result.addr)->CopyFrom(&p);
   return 0;
}


/*
10.4.34 Value Mapping for the ~makeline~ and ~makesline~ operators

*/
void SetStartSmaller(Line* l, bool s){}
void SetStartSmaller(SimpleLine* l, bool s){
  l->SetStartSmaller(s);
}



template<class LineType>
int SpatialMakeLine(Word* args, Word& result, int message,
                    Word& local, Supplier s){

  result = qp->ResultStorage(s);
  Point* p1 = static_cast<Point*>(args[0].addr);
  Point* p2 = static_cast<Point*>(args[1].addr);
  LineType* res = static_cast<LineType*>(result.addr);
  res->Clear();
  if(!p1->IsDefined() || !p2->IsDefined()){
       res->SetDefined(false);
       return 0;
  }
  if(AlmostEqual(*p1,*p2)){
     res->SetDefined(false);
     return 0;
  }
  res->StartBulkLoad();
  HalfSegment h(true, *p1, *p2);
  h.attr.edgeno = 0;
  (*res) += h;
  h.SetLeftDomPoint(false);
  (*res) += h;
  res->EndBulkLoad();
  if(p1->Compare(p2) > 0){
    SetStartSmaller(res,false);
  } else {
    SetStartSmaller(res,true);
  }
  return 0;
}
/*
~Realminize~

*/
int RealminizeVM(Word* args, Word& result, int message,
                Word& local, Supplier s){

   Line* arg = static_cast<Line*>(args[0].addr);
   result = qp->ResultStorage(s);
   Line* res = static_cast<Line*>(result.addr);
   Realminize2(*arg,*res);
   return 0;
}



/*
Value Mapping for ~CommonBorder2~

*/
int CommonBorder2VM(Word* args, Word& result, int message,
            Word& local, Supplier s){
   result = qp->ResultStorage(s);
   Region* arg1 = static_cast<Region*>(args[0].addr);
   Region* arg2 = static_cast<Region*>(args[1].addr);
   Geoid* geoid = 0; // TODO: Add spherical geometry case
   Line* res = static_cast<Line*>(result.addr);
   CommonBorder(*arg2,*arg1,*res,geoid);
   return 0;
}

/*
Value Mapping for ~toLine~

*/
int toLineVM(Word* args, Word& result, int message,
            Word& local, Supplier s){
   result = qp->ResultStorage(s);
   SimpleLine* sline = static_cast<SimpleLine*>(args[0].addr);
   Line* res = static_cast<Line*>(result.addr);
   sline->toLine(*res);
   return 0;
}


/*
Value Mapping for ~fromLine~

*/


int fromLineVM1(Word* args, Word& result, int message,
            Word& local, Supplier s){
   result = qp->ResultStorage(s);
   Line* line = static_cast<Line*>(args[0].addr);
   SimpleLine* res = static_cast<SimpleLine*>(result.addr);
   res->fromLine(*line);
   return 0;
}

int fromLineVM2(Word* args, Word& result, int message,
                Word& local, Supplier s){
  result = qp->ResultStorage(s);
  Line* line = static_cast<Line*>(args[0].addr);
  CcBool* smaller = static_cast<CcBool*> (args[1].addr);
  SimpleLine* res = static_cast<SimpleLine*>(result.addr);
  res->fromLine(*line,smaller->GetBoolval());
  return 0;
}

ValueMapping fromLineVM[] = {
  fromLineVM1,
  fromLineVM2
};

int fromLineSelect(ListExpr args){
  if(nl->ListLength(args)==1) return 0;
  if(nl->ListLength(args)==2) return 1;
  return -1;
}

/*
Value Mapping for ~isCycle~

*/
int isCycleVM(Word* args, Word& result, int message,
            Word& local, Supplier s) {
   result = qp->ResultStorage(s);
   SimpleLine* sline = static_cast<SimpleLine*>(args[0].addr);
   CcBool* res = static_cast<CcBool*>(result.addr);
   if( sline->IsDefined() ) {
    res->Set(true,sline->IsCycle());
   } else {
     res->SetDefined( false );
   }
   return 0;
}


int utmVM_p(Word* args, Word& result, int message,
          Word& local, Supplier s) {
   result = qp->ResultStorage(s);
   Point* p = static_cast<Point*>(args[0].addr);
   Point* res = static_cast<Point*>(result.addr);
   UTM utm;
   utm(*p,*res);
   return 0;
}

int utmVM_ps(Word* args, Word& result, int message,
          Word& local, Supplier s){
   result = qp->ResultStorage(s);
   Points* p = static_cast<Points*>(args[0].addr);
   Points* res = static_cast<Points*>(result.addr);
   UTM utm;
   res->Clear();
   res->Resize(p->Size());
   res->StartBulkLoad();
   Point p1(true);
   for(int i=0;i<p->Size();i++){
      p->Get(i,p1);
      Point p2(true);
      if(! utm(p1,p2)){
        res->EndBulkLoad();
        res->Clear();
        res->SetDefined(false);
        return 0;
      }
      (*res) += (p2);
   }
   res->EndBulkLoad();
   return 0;
}

int gkVM_p(Word* args, Word& result, int message,
          Word& local, Supplier s){
   result = qp->ResultStorage(s);
   Point* p = static_cast<Point*>(args[0].addr);
   CcInt* zone = static_cast<CcInt*>(args[1].addr);
   Point* res = static_cast<Point*>(result.addr);
   if(!zone->IsDefined() || zone->GetValue() < 0 || zone->GetValue() > 119){
    res->SetDefined(false);
   }
   WGSGK gk;
   gk.setMeridian(zone->GetValue());
   gk.project(*p,*res);
   return 0;
}


int gkVM_ps(Word* args, Word& result, int message,
          Word& local, Supplier s){
   result = qp->ResultStorage(s);
   Points* p = static_cast<Points*>(args[0].addr);
   CcInt* zone = static_cast<CcInt*>(args[1].addr);
   Points* res = static_cast<Points*>(result.addr);
   res->Clear();
   if(    !p->IsDefined() || !zone->IsDefined()
       || zone->GetValue()<0 || zone->GetValue() > 119){
     res->SetDefined( false );
     res->Clear();
   }
   WGSGK gk;
   gk.setMeridian(zone->GetValue());
   res->SetDefined( true );
   res->Resize(p->Size());
   res->StartBulkLoad();
   Point p1(true);
   for(int i=0;i<p->Size();i++){
      p->Get(i,p1);
      Point p2(true);
      if(! gk.project(p1,p2)){
        res->EndBulkLoad();
        res->Clear();
        res->SetDefined(false);
        return 0;
      }
      (*res) += (p2);
   }
   res->EndBulkLoad();
   return 0;
}

template<class T>
int gkVM_x(Word* args, Word& result, int message,
          Word& local, Supplier s){
   result = qp->ResultStorage(s);
   T* a = static_cast<T*>(args[0].addr);
   CcInt* zone = static_cast<CcInt*>(args[1].addr);
   T* res = static_cast<T*>(result.addr);
   res->Clear();
   if(    !a->IsDefined() || !zone->IsDefined()
       || zone->GetValue() < 0 || zone->GetValue() > 119){
     res->SetDefined( false );
     return 0;
   }
   WGSGK gk;
   gk.setMeridian(zone->GetValue());
   res->SetDefined( true );
   res->Resize(a->Size());
   res->StartBulkLoad();
   HalfSegment hs;
   HalfSegment hs2;
   for(int i=0;i<a->Size();i++){
      a->Get(i,hs);
      if(! gk.project(hs,hs2)){
        res->Clear();
        res->SetDefined(false);
        return 0;
      }
      (*res) += (hs2);
   }
   res->EndBulkLoad();
   return 0;
}

int gkVM_rect(Word* args, Word& result, int message,
          Word& local, Supplier s){

   result = qp->ResultStorage(s);
   Rectangle<2>* r = static_cast<Rectangle<2>*>(args[0].addr);
   CcInt* zone = static_cast<CcInt*>(args[1].addr);
   Rectangle<2>* res = static_cast<Rectangle<2>*>(result.addr);
   if(    !r->IsDefined() || !zone->IsDefined()
       || zone->GetValue() < 0 || zone->GetValue() > 119){
     res->SetDefined( false );
     return 0;
   }
   WGSGK gk;
   gk.setMeridian(zone->GetValue());
   res->SetDefined( true );
   double x1 = r->MinD(0);
   double y1 = r->MinD(1);
   double x2 = r->MaxD(0);
   double y2 = r->MaxD(1);

   Point p1(true,x1,y1);
   Point p2(true,x2,y2);
   Point p1p(true,0,0);
   Point p2p(true,0,0);
   if(!gk.project(p1,p1p) ||
      !gk.project(p2,p2p)){
      res->SetDefined(false);
      return  0;
   } 
   double mind[2];
   double maxd[2];
   mind[0] = min(p1p.GetX(),p2p.GetX());
   mind[1] = min(p1p.GetY(),p2p.GetY());
   maxd[0] = max(p1p.GetX(),p2p.GetX());
   maxd[1] = max(p1p.GetY(),p2p.GetY());
   res->Set(true,mind,maxd);
   return 0;
}

/*
Reverse Gausss Krueger Projection.


*/


void reverseGK(const Point* arg, Point* result){
  if(!arg->IsDefined()){
    result->SetDefined(false);
    return;
  }
  WGSGK gk;
  bool ok = gk.getOrig(*arg, *result);
  if(!ok){
    result->SetDefined(false);
  }
}


void reverseGK(const Points* arg, Points* result){
  WGSGK gk;
  result->Clear();
  result->Resize(arg->Size());
  result->StartBulkLoad();
  for(int i=0; i< arg->Size(); i++){
    Point p1(true);
    arg->Get(i,p1);
    Point p2(true);
    bool ok = gk.getOrig(p1,p2);
    if(!ok || !p2.IsDefined()){
       result->Clear();
       result->EndBulkLoad();
       result->SetDefined(false);
       return;
    }
    (*result) += p2;
  }
  result->EndBulkLoad();
}



bool reverseGK(HalfSegment& h, const WGSGK& gk){
  Point p1 = h.GetLeftPoint();
  Point p3(false,0,0);
  bool ok = gk.getOrig(p1,p3);
  if(!ok || !p3.IsDefined()){
    return false;
  }
  Point p2 = h.GetRightPoint();
  Point p4(false,0,0);
  ok = gk.getOrig(p2,p4);
  if(!ok || !p4.IsDefined()){
    return false;
  }
  h.Set(h.IsLeftDomPoint(), p3, p4);
  return true;
}




template <class T>
void reverseGK(const T* arg, T* result){
  if(!arg->IsDefined()){
    result->SetDefined(false);
    return;
  }
  WGSGK gk;
  result->Clear();
  result->Resize(arg->Size());
  result->StartBulkLoad();
  for(int i=0;i<arg->Size();i++){
    HalfSegment h;
    arg->Get(i,h);
    if(h.IsLeftDomPoint()){
      if(!reverseGK(h,gk)){
        result->EndBulkLoad();
        result->Clear();
        result->SetDefined(false);
        return;
      }
      (*result) += h;
      h.SetLeftDomPoint(false);
      (*result) += h;
    }
  }
  result->EndBulkLoad();
}

inline void reverseGK(const Line* arg, Line* result){
  return reverseGK<Line>(arg,result);
}

inline void reverseGK(const Region* arg, Region* result){
  return reverseGK<Region>(arg,result);
}



template<class T>
int reversegkVM(Word* args, Word& result, int message,
          Word& local, Supplier s){
   result = qp->ResultStorage(s);
   T* arg = static_cast<T*>(args[0].addr);
   T* res = static_cast<T*>(result.addr);
   reverseGK(arg,res);
   return 0;
}


/*
Operations on geoids

*/

int geoid_getRadius_VM(Word* args, Word& result, int message,
                    Word& local, Supplier s)
{
  result = qp->ResultStorage(s);
  CcReal* res = static_cast<CcReal*>(result.addr);
  Geoid* arg = static_cast<Geoid*>(args[0].addr);
  if(!arg->IsDefined()){
    res->Set(false, 0.0);
  } else {
    res->Set(true, arg->getR());
  }
  return 0;
}

int geoid_getFlattening_VM(Word* args, Word& result, int message,
                        Word& local, Supplier s)
{
  result = qp->ResultStorage(s);
  CcReal* res = static_cast<CcReal*>(result.addr);
  Geoid* arg = static_cast<Geoid*>(args[0].addr);
  if(!arg->IsDefined()){
    res->Set(false, 0.0);
  } else {
    res->Set(true, arg->getF());
  }
  return 0;
}

template<class T1, class T2>
int geoid_create_geoid_VM(Word* args, Word& result, int message,
                    Word& local, Supplier s)
{
  result = qp->ResultStorage(s);
  Geoid* res = static_cast<Geoid*>(result.addr);
  CcString* nameCcStr = static_cast<CcString*>(args[0].addr);
  T1* radiusT1 = 0;
  T2* flatteningT2 = 0;
  int noargs = qp->GetNoSons(s);
  if(!nameCcStr->IsDefined()){
    res->SetDefined(false);
    return 0;
  }
  string name = nameCcStr->GetValue();
  double radius = 1.0;
  double flattening = 0.0;
  if(noargs==3){
    radiusT1 = static_cast<T1*>(args[1].addr);
    flatteningT2 = static_cast<T2*>(args[2].addr);
    if(!radiusT1->IsDefined() || !flatteningT2->IsDefined()){
      res->SetDefined(false);
      return 0;
    }
    radius = radiusT1->GetValue();
    flattening = flatteningT2->GetValue();
    if( (radius <= 0.0) || (flattening < 0.0) ){
      res->SetDefined(false);
      return 0;
    }
    Geoid g(name, radius, flattening);
    res->CopyFrom(&g);
    return 0;
  }
  bool valid = false;
  Geoid::GeoidName gc = Geoid::getGeoIdNameFromString(name, valid);
  if(!valid){
    res->SetDefined(false);
    return 0;
  }
  res->setPredefinedGeoid(gc);
  return 0;
}

ValueMapping geoid_create_geoid_vm[] = {
  geoid_create_geoid_VM<CcReal,CcReal>, // (string)
  geoid_create_geoid_VM<CcReal,CcReal>, // (string x real x real)
  geoid_create_geoid_VM<CcReal,CcInt>,  // (string x real x int)
  geoid_create_geoid_VM<CcInt,CcReal>,  // (string x int x real)
  geoid_create_geoid_VM<CcInt,CcInt>    // (string x int x int)
};

int geoid_create_geoid_SELECT(ListExpr args){
  if(nl->ListLength(args)==1){ return 0; }
  ListExpr first = nl->Second(args);
  ListExpr second = nl->Third(args);
  if(    listutils::isSymbol(first,CcReal::BasicType())
      && listutils::isSymbol(second,CcReal::BasicType())){ return 1; }
  if(    listutils::isSymbol(first,CcReal::BasicType())
      && listutils::isSymbol(second,CcInt::BasicType())){ return 2; }
  if(    listutils::isSymbol(first,CcInt::BasicType())
      && listutils::isSymbol(second,CcReal::BasicType())){ return 3; }
  if(    listutils::isSymbol(first,CcInt::BasicType())
      && listutils::isSymbol(second,CcInt::BasicType())){ return 4; }
  return -1;
}

/*
Collecting line

*/
template<class ResLineType>
int SpatialCollect_lineVMPointstream(Word* args, Word& result, int message,
                                     Word& local, Supplier s){
  result = qp->ResultStorage(s);
  ResLineType* L = static_cast<ResLineType*>(result.addr);
  Point* P0 = 0;
  Point* P1 = 0;
  Word elem;
  L->Clear();
  L->SetDefined( true );

  qp->Open(args[0].addr);

  qp->Request(args[0].addr, elem);
  if(!qp->Received(args[0].addr)){
    qp->Close(args[0].addr);
    return 0;
  }
  P0 = static_cast<Point*>(elem.addr);
  assert( P0 != 0 );
  if(!P0->IsDefined()){ // found undefined Elem
    qp->Close(args[0].addr);
    L->SetDefined(false);
    P0->DeleteIfAllowed();
    qp->Close(args[0].addr);
    return 0;
  }

  L->StartBulkLoad();
  qp->Request(args[0].addr, elem);
  while ( qp->Received(args[0].addr) ){
      P1 = static_cast<Point*>(elem.addr);
      assert( P1 != 0 );
      if(!P1->IsDefined()){
        qp->Close(args[0].addr);
        L->Clear();
        L->SetDefined(false);
        if(P0){ P0->DeleteIfAllowed(); P0 = 0; }
        if(P1){ P1->DeleteIfAllowed(); P1 = 0; }
        qp->Close(args[0].addr);
        return 0;
      }
      if(AlmostEqual(*P0,*P1)){
        qp->Request(args[0].addr, elem);
        P1->DeleteIfAllowed();
        P1 = 0;
      } else {
        HalfSegment hs(true, *P0, *P1); // create halfsegment
        (*L) += (hs);
        hs.SetLeftDomPoint( !hs.IsLeftDomPoint() ); //createcounter-halfsegment
        (*L) += (hs);
        P0->DeleteIfAllowed();
        P0 = P1; P1 = 0;
        qp->Request(args[0].addr, elem); // get next Point
     }
  }
  L->EndBulkLoad(); // sort and realminize

  qp->Close(args[0].addr);
  if(P0){ P0->DeleteIfAllowed(); P0 = 0; }
  return 0;
}

// helper function for SpatialCollect_line
void append(Line& l1, const Line& l2){
  // l1 += l2; // runs not correctly
  int size = l2.Size();
  HalfSegment hs;
  for(int i = 0; i < size; i++){
    l2.Get( i, hs );
    l1 += hs;
  }
}

// helper function for SpatialCollect_line
void append(SimpleLine& l1, const SimpleLine& l2){
  int size = l2.Size();
  HalfSegment hs;
  for(int i = 0; i < size; i++){
    l2.Get( i, hs );
    l1 += hs;
  }
}

// helper function for SpatialCollect_line
void append(Line& l1, const SimpleLine& l2){
  int size = l2.Size();
  HalfSegment hs;
  for(int i = 0; i < size; i++){
    l2.Get( i, hs );
    l1 += hs;
  }
}

// helper function for SpatialCollect_line
void append(SimpleLine& l1, const Line& l2){
  int size = l2.Size();
  HalfSegment hs;
  for(int i = 0; i < size; i++){
    l2.Get( i, hs );
    l1 += hs;
  }
}


template <class StreamLineType, class ResLineType>
int SpatialCollect_lineVMLinestream(Word* args, Word& result, int message,
                                    Word& local, Supplier s){
  result = qp->ResultStorage(s);
  ResLineType* L = static_cast<ResLineType*>(result.addr);
  StreamLineType* line = 0;
  L->Clear();
  CcBool* ignoreUndefined = static_cast<CcBool*>(args[1].addr);
  if(!ignoreUndefined->IsDefined()){
    L->SetDefined(false);
    return 0;
  }
  bool ignore = ignoreUndefined->GetValue();

  Word elem;
  qp->Open(args[0].addr);

  L->StartBulkLoad();
  qp->Request(args[0].addr, elem);
  while ( qp->Received(args[0].addr) ){
    line = static_cast<StreamLineType*>(elem.addr);
    assert( line != 0 );
    if(!ignore && !line->IsDefined()){
      qp->Close(args[0].addr);
      L->Clear();
      L->SetDefined(false);
      if(line){ line->DeleteIfAllowed(); }
      return 0;
    }
    append(*L, *line);
    line->DeleteIfAllowed();
    line = 0;
    qp->Request(args[0].addr, elem); // get next line
  }
  L->EndBulkLoad(); // sort and realminize

  qp->Close(args[0].addr);
  return 0;
}

template<class StreamPointType>
int SpatialCollect_PointsVM(Word* args, Word& result, int message,
                                Word& local, Supplier s){

   CcBool* ignoreUndefined = static_cast<CcBool*>(args[1].addr);
   result = qp->ResultStorage(s);
   Points* res = static_cast<Points*>(result.addr);
   res->Clear();
   if(!ignoreUndefined->IsDefined()){
     res->SetDefined(false);
     return 0;
   }
   res->SetDefined(true);
   bool ignore = ignoreUndefined->GetValue();
   Word elem;
   res->StartBulkLoad();

   qp->Open(args[0].addr);
   qp->Request(args[0].addr,elem);

   while(qp->Received(args[0].addr)){
     StreamPointType* p = static_cast<StreamPointType*>(elem.addr);
     if(p->IsDefined()){
        (*res) += *p;
     } else if(!ignore){
        res->EndBulkLoad(false,false,false);
        res->Clear();
        res->SetDefined(false);
        qp->Close(args[0].addr);
        return 0;
     }
     p->DeleteIfAllowed();
     qp->Request(args[0].addr,elem);
   }
   qp->Close(args[0].addr);
   res->EndBulkLoad();
   return 0;

}

/*
collect sline

*/
template<class ResLineType>
int SpatialCollect_slineVMPointstream(Word* args, Word& result, int message,
                                     Word& local, Supplier s){
  result = qp->ResultStorage(s);
  ResLineType* L = static_cast<ResLineType*>(result.addr);
  L->Clear();
  L->SetDefined( true );
  CcBool* IgnoreUndef = (CcBool*) args[1].addr;
  if(!IgnoreUndef->IsDefined()){
    L->SetDefined(false);
    return 0;
  }
  bool ignoreUndef = IgnoreUndef->GetValue();

  Stream<Point> stream(args[0]);
  Point* firstPoint = 0; // the very first point
  Point* lp = 0; 
  stream.open();

  // search for the first defined point in stream
  while( (firstPoint == 0) && ((lp = stream.request())!=0) ){
    if(!lp->IsDefined()){
       lp->DeleteIfAllowed();
       if(!ignoreUndef){
          L->SetDefined(false);
          stream.close();
          return 0;
       }
    } else {
       firstPoint = (Point*) lp->Copy();
    }
  }
 
  if(!firstPoint){ // no defined point found in stream
    stream.close();
    return 0;  // an empty line
  }


  Point* cp;
  Point* secondPoint = 0;
  // iterate over remaining points
  L->StartBulkLoad();
  while( (cp = stream.request()) != 0){
    if(!cp->IsDefined()){ // spacial case undefined point in stream
      cp->DeleteIfAllowed();
      if(!ignoreUndef){
         firstPoint->DeleteIfAllowed();
         lp->DeleteIfAllowed();
         stream.close();
         L->Clear();
         L->SetDefined(false);
         return 0;
      }
    } else {
      if(AlmostEqual(*lp,*cp)){
         cp->DeleteIfAllowed(); //ignore point that is too close to the last one
      } else { // normal case: add halfsegments
         if(!secondPoint){
            secondPoint = (Point*) cp->Copy();
         }
         HalfSegment hs(true, *lp, *cp); // create halfsegment
         (*L) += (hs);
         hs.SetLeftDomPoint( !hs.IsLeftDomPoint() ); 
         (*L) += (hs);
         lp->DeleteIfAllowed();
         lp = cp;
      }
    }
  }
  stream.close();
  L->EndBulkLoad(); // sort and realminize
  if(lp){
    if(!L->IsCycle()){
        L->SetStartSmaller(*firstPoint < *lp);
    } else {
        L->SetStartSmaller(*firstPoint < *secondPoint);
    }
    lp->DeleteIfAllowed();
    if(secondPoint){
        secondPoint->DeleteIfAllowed();
    }
  }
  firstPoint->DeleteIfAllowed();
  return 0;
}

template <class StreamLineType, class ResLineType>
int SpatialCollect_slineVMLinestream(Word* args, Word& result, int message,
                                     Word& local, Supplier s){
   result = qp->ResultStorage(s);
   ResLineType* L = static_cast<ResLineType*>(result.addr);
   StreamLineType* line = 0;
   L->Clear();
   CcBool* ignoreUndefined = static_cast<CcBool*>(args[1].addr);
   if(!ignoreUndefined->IsDefined()){
     L->SetDefined(false);
     return 0;
   }
   bool ignore = ignoreUndefined->GetValue();
   Word elem;
   Point* firstPoint = 0;
   Point* lastPoint = 0;
   bool first = true;
   L->StartBulkLoad();
   qp->Open(args[0].addr);
   qp->Request(args[0].addr, elem);
   while ( qp->Received(args[0].addr) ){
    line = static_cast<StreamLineType*>(elem.addr);
    assert( line != 0 );
    if(!line->IsDefined())
    {
      if (!ignore)
      {
         qp->Close(args[0].addr);
         L->Clear();
         L->SetDefined(false);
         line->DeleteIfAllowed();
         line = 0;
         return 0;
      }
    }
    else
    {
      if (first)
      {
         firstPoint = new Point(line->StartPoint());
         lastPoint = new Point(line->EndPoint());
         first = false;
         append(*L, *line);
      }
      else
      {
        lastPoint->DeleteIfAllowed();
        lastPoint = new Point(line->EndPoint());
        append(*L, *line);
      }
    }
    line->DeleteIfAllowed();
    line = 0;
    qp->Request(args[0].addr, elem); // get next line
  }
  qp->Close(args[0].addr);
  L->EndBulkLoad(); // sort and realminize
  if (L->IsDefined()){
    if (firstPoint > lastPoint) L->SetStartSmaller(false);
    else L->SetStartSmaller(true);
  }
  lastPoint->DeleteIfAllowed();
  firstPoint->DeleteIfAllowed();
  return 0;
}

int SpatialVMSetStartSmaller(Word* args, Word& result, int message,
                             Word& local, Supplier s){
  SimpleLine* l = static_cast<SimpleLine*>(args[0].addr);
  CcBool* b = static_cast<CcBool*>(args[1].addr);
  result = qp->ResultStorage(s);
  SimpleLine* res = static_cast<SimpleLine*>(result.addr);
  if(!l->IsDefined() || !b->IsDefined()){
    res->SetDefined(false);
  } else {
    res->CopyFrom(l);
    res->SetStartSmaller(b->GetBoolval());
  }
  return 0;
}

int SpatialVMGetStartSmaller(Word* args, Word& result, int message,
                             Word& local, Supplier s){
  SimpleLine* l = static_cast<SimpleLine*>(args[0].addr);
  result = qp->ResultStorage(s);
  CcBool* res = static_cast<CcBool*>(result.addr);
  if(!l->IsDefined()){
    res->SetDefined(false);
  } else {
    res->Set(true,l->GetStartSmaller());
  }
  return 0;
}

int SpatialVMCreateSline(Word* args, Word& result, int message,
                             Word& local, Supplier s){
  Point* p1 = static_cast<Point*>(args[0].addr);
  Point* p2 = static_cast<Point*>(args[1].addr);
  result = qp->ResultStorage(s);
  SimpleLine* res = static_cast<SimpleLine*>(result.addr);
  if(!p1->IsDefined() || !p1->IsDefined()){
      res->SetDefined(false);
  } else if(AlmostEqual(*p1,*p2)){
      res->Clear();
      res->SetDefined(true);
  } else {
    res->Clear();
    res->SetDefined(true);
    res->StartBulkLoad();
    HalfSegment hs(true, *p1, *p2);
    *res += hs;
    hs.SetLeftDomPoint(false);
    *res += hs;
    res->EndBulkLoad();
    res->SetStartSmaller(*p1 < *p2);
  }
  return 0;
}

/*
10.5 Definition of operators

Definition of operators is done in a way similar to definition of
type constructors: an instance of class ~Operator~ is defined.

Because almost all operators are overloaded, we have first do define an
array of value mapping functions for each operator. For nonoverloaded
operators there is also such and array defined, so it easier to make them
overloaded.

10.5.1 Definition of value mapping vectors

*/
ValueMapping spatialisemptymap[] = {
  SpatialIsEmpty<Point>,
  SpatialIsEmpty<Points>,
  SpatialIsEmpty<Line>,
  SpatialIsEmpty<Region>,
  SpatialIsEmpty<SimpleLine> };

ValueMapping spatialequalmap[] = {
  SpatialEqual<Point,Point>,
  SpatialEqual<Points, Points>,
  SpatialEqual<Line,Line>,
  SpatialEqual<Region, Region>,
  SpatialEqual<SimpleLine, SimpleLine>,
  SpatialEqual<Point,Points>,
  SpatialEqual<Points, Point>};

ValueMapping spatialislessmap[] = { SpatialIsLess<Point, Point>};

ValueMapping spatialnotequalmap[] = {
  SpatialNotEqual<Point>,
  SpatialNotEqual<Points>,
  SpatialNotEqual<Line>,
  SpatialNotEqual<Region>,
  SpatialNotEqual<SimpleLine> };

ValueMapping spatialintersectsmap[] = {
  SpatialIntersectsVM<Points,Points,false>,
  SpatialIntersectsVM<Points,Line,false>,
  SpatialIntersectsVM<Points,Region,false>,
  SpatialIntersectsVM<Points,Line,true>,
  SpatialIntersectsVM<Line,Line,false>,
  SpatialIntersectsVM<Line,Region,false>,
  SpatialIntersectsVM<Points,Region,true>,
  SpatialIntersectsVM<Line,Region,true>,
  SpatialIntersectsVM<Region,Region,false>,
  SpatialIntersectsVM<SimpleLine,SimpleLine,false>
};

ValueMapping spatialinsidemap[] = {
  SpatialInsideGeneric<Point,Points>,
  SpatialInsideGeneric<Point,Line>,
  SpatialInsideGeneric<Point,Region>,
  SpatialInsideGeneric<Points,Points>,
  SpatialInsideGeneric<Points,Line>,
  SpatialInsideGeneric<Points,Region>,
  SpatialInsideGeneric<Line,Line>,
  SpatialInsideGeneric<Line,Region>,
  SpatialInsideGeneric<Region,Region>,
  SpatialInsideGeneric<Point,SimpleLine>,
  SpatialInsideGeneric<SimpleLine,SimpleLine>
};

ValueMapping spatialadjacentmap[] = {
  SpatialAdjacent_psr,
  SpatialAdjacent_lr,
  SpatialAdjacent_rps,
  SpatialAdjacent_rl,
  SpatialAdjacent_rr };


ValueMapping spatialintersectionVM[] = {
  SpatialIntersectionGeneric<Point, Point, Points>,
  SpatialIntersectionGeneric<Point, Points, Points>,
  SpatialIntersectionGeneric<Point, Line, Points>,
  SpatialIntersectionGeneric<Point, Region, Points>,
  SpatialIntersectionGeneric<Point, SimpleLine, Points>,

  SpatialIntersectionGeneric<Points, Point, Points>,
  SpatialIntersectionGeneric<Points, Points, Points>,
  SpatialIntersectionGeneric<Points, Line, Points>,
  SpatialIntersectionGeneric<Points, Region, Points>,
  SpatialIntersectionGeneric<Points, SimpleLine, Points>,

  SpatialIntersectionGeneric<Line, Point, Points>,
  SpatialIntersectionGeneric<Line, Points, Points>,
  SpatialIntersectionGeneric<Line, Line, Line>,
  SpatialIntersectionGeneric<Line, Region, Line>,
  SpatialIntersectionGeneric<Line, SimpleLine, SimpleLine>,

  SpatialIntersectionGeneric<Region, Point, Points>,
  SpatialIntersectionGeneric<Region, Points, Points>,
  SpatialIntersectionGeneric<Region, Line, Line>,
  SpatialIntersectionGeneric<Region, Region, Region>,
  SpatialIntersectionGeneric<Region, SimpleLine, SimpleLine>,

  SpatialIntersectionGeneric<SimpleLine, Point, Points>,
  SpatialIntersectionGeneric<SimpleLine, Points, Points>,
  SpatialIntersectionGeneric<SimpleLine, Line, SimpleLine>,
  SpatialIntersectionGeneric<SimpleLine, Region, SimpleLine>,
  SpatialIntersectionGeneric<SimpleLine, SimpleLine, SimpleLine>

};

ValueMapping spatialminusVM[] = {
  SpatialMinusGeneric<Point, Point, Points>,
  SpatialMinusGeneric<Point, Points, Points>,
  SpatialMinusGeneric<Point, Line, Points>,
  SpatialMinusGeneric<Point, Region, Points>,
  SpatialMinusGeneric<Point, SimpleLine, Points>,

  SpatialMinusGeneric<Points, Point, Points>,
  SpatialMinusGeneric<Points, Points, Points>,
  SpatialMinusGeneric<Points, Line, Points>,
  SpatialMinusGeneric<Points, Region, Points>,
  SpatialMinusGeneric<Points, SimpleLine, Points>,

  SpatialMinusGeneric<Line, Point, Line>,
  SpatialMinusGeneric<Line, Points, Line>,
  SpatialMinusGeneric<Line, Line, Line>,
  SpatialMinusGeneric<Line, Region, Line>,
  SpatialMinusGeneric<Line, SimpleLine, Line>,

  SpatialMinusGeneric<Region, Point, Region>,
  SpatialMinusGeneric<Region, Points, Region>,
  SpatialMinusGeneric<Region, Line, Region>,
  SpatialMinusGeneric<Region, Region, Region>,
  SpatialMinusGeneric<Region, SimpleLine, Region>,

  SpatialMinusGeneric<SimpleLine, Point, SimpleLine>,
  SpatialMinusGeneric<SimpleLine, Points, SimpleLine>,
  SpatialMinusGeneric<SimpleLine, Line, SimpleLine>,
  SpatialMinusGeneric<SimpleLine, Region, SimpleLine>,
  SpatialMinusGeneric<SimpleLine, SimpleLine, SimpleLine>

};


ValueMapping spatialunionVM[] = {
  SpatialUnionGeneric<Point, Point, Points>,
  SpatialUnionGeneric<Point, Points, Points>,
  SpatialUnionGeneric<Point, Line, Line>,
  SpatialUnionGeneric<Point, Region, Region>,
  SpatialUnionGeneric<Point, SimpleLine, SimpleLine>,

  SpatialUnionGeneric<Points, Point, Points>,
  SpatialUnionGeneric<Points, Points, Points>,
  SpatialUnionGeneric<Points, Line, Line>,
  SpatialUnionGeneric<Points, Region, Region>,
  SpatialUnionGeneric<Points, SimpleLine, SimpleLine>,

  SpatialUnionGeneric<Line, Point, Line>,
  SpatialUnionGeneric<Line, Points, Line>,
  SpatialUnionGeneric<Line, Line, Line>,
  SpatialUnionGeneric<Line, Region, Region>,
  SpatialUnionGeneric<Line, SimpleLine, Line>,

  SpatialUnionGeneric<Region, Point, Region>,
  SpatialUnionGeneric<Region, Points, Region>,
  SpatialUnionGeneric<Region, Line, Region>,
  SpatialUnionGeneric<Region, Region, Region>,
  SpatialUnionGeneric<Region, SimpleLine, Region>,

  SpatialUnionGeneric<SimpleLine, Point, SimpleLine>,
  SpatialUnionGeneric<SimpleLine, Points, SimpleLine>,
  SpatialUnionGeneric<SimpleLine, Line, Line>,
  SpatialUnionGeneric<SimpleLine, Region, Region>,
  SpatialUnionGeneric<SimpleLine, SimpleLine, Line>,

};

ValueMapping spatialdistancemap[] = {

  // arg1 = point
  SpatialDistance<Point,Point,false>,
  SpatialDistance<Points,Point,true>,
  SpatialDistance<Line,Point,true>,
  SpatialDistance<SimpleLine,Point,true>,
  SpatialDistance<Point,Rectangle<2>,false>,
  SpatialDistance<Region,Point, true>,
  
  // arg1 = points
  SpatialDistance<Points,Point,false>,
  SpatialDistance<Points,Points,false>,
  SpatialDistance<Line, Points, true>,
  SpatialDistance<SimpleLine, Points,true>,
  SpatialDistance<Points,Rectangle<2>,false>,
  SpatialDistance<Region, Points, true>,

  // arg1 = line
  SpatialDistance<Line,Point,false>,
  SpatialDistance<Line,Points,false>,
  SpatialDistance<Line,Line,false>,
  SpatialDistance<Line,SimpleLine, false>,
  SpatialDistance<Line,Rectangle<2>,false>,
  SpatialDistance<Region,Line, true>,

   // arg1 = sline
  SpatialDistance<SimpleLine,Point,false>,
  SpatialDistance<SimpleLine,Points,false>,
  SpatialDistance<Line,SimpleLine,true>,
  SpatialDistance<SimpleLine,SimpleLine,false>,
  SpatialDistance<SimpleLine,Rectangle<2>,false>,
  SpatialDistance<Region, SimpleLine, true>,

  // arg1 = rect  
  SpatialDistance<Point  ,Rectangle<2>, true>,
  SpatialDistance<Points ,Rectangle<2>, true>,
  SpatialDistance<Line   ,Rectangle<2>, true>,
  SpatialDistance<SimpleLine,Rectangle<2>, true>,
  SpatialDistance<Rectangle<2>,Rectangle<2>,false>,
  SpatialDistance<Region,Rectangle<2>,true>,

    // arg1 = region
  SpatialDistance<Region,Point,false>,
  SpatialDistance<Region,Points,false>,
  SpatialDistance<Region,Line,false>,
  SpatialDistance<Region,SimpleLine,false>,
  SpatialDistance<Region,Rectangle<2>,false>,
  SpatialDistance<Region,Region,false>,
};

ValueMapping spatialnocomponentsmap[] = {
  SpatialNoComponents_ps,
  SpatialNoComponents_l,
  SpatialNoComponents_r };

ValueMapping spatialnosegmentsmap[] = {
  SpatialNoSegments<Line>,
  SpatialNoSegments<Region>,
  SpatialNoSegments<SimpleLine>,
  SpatialNoSegmentsDline };

ValueMapping spatialbboxmap[] = {
  SpatialBBox<Point>,
  SpatialBBox<Points>,
  SpatialBBox<Line>,
  SpatialBBox<Region>,
  SpatialBBox<SimpleLine>,
  SpatialBBox<Rectangle<2> > };

ValueMapping spatialtouchpointsmap[] = {
  SpatialTouchPoints_lr,
  SpatialTouchPoints_rl,
  SpatialTouchPoints_rr };



ValueMapping spatialwindowclippinginmap[] = {
  SpatialWindowClippingIn_l,
  SpatialWindowClippingIn_r };

ValueMapping spatialwindowclippingoutmap[] = {
  SpatialWindowClippingOut_l,
  SpatialWindowClippingOut_r };


ValueMapping spatialcomponentsmap[] = {
  SpatialComponents_ps,
  SpatialComponents_r,
  SpatialComponents_l };

ValueMapping spatialverticesmap[] = {
  SpatialVertices_l,
  SpatialVertices_r };

ValueMapping spatialboundarymap[] = {
  SpatialBoundary_r,
  SpatialBoundary_l};

ValueMapping spatialaddmap[] = { SpatialAdd_p };

ValueMapping spatialgetxmap[] = { SpatialGetX_p };

ValueMapping spatialgetymap[] = { SpatialGetY_p };

ValueMapping spatialsimplifymap[] = { SpatialSimplify_LReal,
                                      SpatialSimplify_LRealPs };

ValueMapping spatialsizemap[] = {
      SpatialSize<Line>,
      SpatialSize<Region>,
      SpatialSize<SimpleLine>
};

ValueMapping SpatialAtPointMap[]  = { SpatialAtPointBool,
                                      SpatialAtPoint};

ValueMapping SpatialAtPositionMap[]  = {SpatialAtPositionBool,
                                        SpatialAtPosition };

ValueMapping SpatialCrossingsMap[] = {
          SpatialCrossings<Line>,
          SpatialCrossings<SimpleLine>,
          SpatialCrossings_single
      };


ValueMapping utmVM[] = {
          utmVM_p,
          utmVM_ps
      };

ValueMapping gkVM[] = {
          gkVM_p,
          gkVM_ps,
          gkVM_x<Line>,
          gkVM_x<Region>,
          gkVM_x<SimpleLine>,
          gkVM_rect
      };


ValueMapping reverseGKVM[] = {
          reversegkVM<Point>,
          reversegkVM<Points>,
          reversegkVM<Line>,
          reversegkVM<Region>
      };

ValueMapping spatialCollectLineMap[] = {
  SpatialCollect_lineVMPointstream<Line>,
  SpatialCollect_lineVMLinestream<SimpleLine,Line>,
  SpatialCollect_lineVMLinestream<Line,Line>
};

ValueMapping spatialCollectSLineMap[] = {
  SpatialCollect_slineVMPointstream<SimpleLine>,
  SpatialCollect_slineVMLinestream<SimpleLine,SimpleLine>,
  SpatialCollect_lineVMLinestream<Line,SimpleLine>
};

ValueMapping spatialCollectPointsMap[] = {
  SpatialCollect_PointsVM<Point>,
  SpatialCollect_PointsVM<Points>
};


/*
10.5.2 Definition of specification strings

*/
const string SpatialSpecIsEmpty  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
     "( <text>point -> bool, points -> bool, line -> bool,"
       "region -> bool, sline -> bool</text---> "
       "<text>isempty ( _ )</text--->"
       "<text>Returns TRUE if the value is undefined or empty. The result "
       "is always defined!</text--->"
       "<text>query isempty ( line1 )</text--->"
       ") )";

const string SpatialSpecEqual  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
  "( <text>(point point) -> bool, (points points) -> bool, "
  "(line line) -> bool, (region region) -> bool,"
  " (sline sline) -> bool </text--->"
  "<text>_ = _</text--->"
  "<text>TRUE, iff both arguments are equal.</text--->"
  "<text>query point1 = point2</text--->"
  ") )";

const string SpatialSpecIsLess =
"( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
"( <text>(point point) -> bool </text--->"
"<text>_ < _</text--->"
"<text>TRUE, iff first argument is smaller than second argument.</text--->"
"<text>query point1 < point2</text--->"
") )";

const string SpatialSpecNotEqual  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
  "( <text>(point point) -> bool, (points points) -> bool, "
  "(line line) -> bool, (region region) -> bool,"
  "(sline sline) -> bool</text--->"
  "<text>_ # _</text--->"
  "<text>TRUE, iff both arguments are not equal.</text--->"
  "<text>query point1 # point2</text--->"
  ") )";

const string SpatialSpecIntersects  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
  "( <text>(points||line||region x points||line||region) -> bool ||"
  " sline x sline -> bool  </text--->"
  "<text>_ intersects _</text--->"
  "<text>TRUE, iff both arguments intersect.</text--->"
  "<text>query region1 intersects region2</text--->"
  ") )";

const string SpatialSpecInside  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
  "( <text>(points||line||sline||region x points||line||sline||region) "
  "-> bool</text--->"
  "<text>_ inside _</text--->"
  "<text>TRUE iff the first argument is inside the second.</text--->"
  "<text>query point1 inside line1</text--->"
  ") )";

const string SpatialSpecAdjacent  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
  "( <text>(points||line||region x points||line||region) -> bool</text--->"
  "<text>_ adjacent _</text--->"
  "<text>TRUE, iff both regions are adjacent.</text--->"
  "<text>query r1 adjacent r2</text--->"
  ") )";

const string SpatialSpecOverlaps  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
  "( <text>(region x region) -> bool</text--->"
  "<text>_ overlaps _</text--->"
  "<text>TRUE, iff both objects overlap each other.</text--->"
  "<text>query line overlap region</text--->"
  ") )";

const string SpatialSpecOnBorder  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>(point x region) -> bool</text--->"
  "<text>_ onborder _</text--->"
  "<text>TRUE, iff the point is on the border of the region."
  "</text--->"
  "<text>query mehringdamm onborder thecenter</text--->"
  ") )";


const string SpatialSpecIninterior  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>(point x region) -> bool</text--->"
  "<text>_ ininterior _</text--->"
  "<text>TRUE, iff the point is within the region's interior."
  "</text--->"
  "<text>query mehringdamm ininterior thecenter</text--->"
  ") )";

const string SpatialSpecInInterior  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>(point x region) -> bool</text--->"
  "<text>_ ininterior _</text--->"
  "<text>TRUE, iff the first argument is in the interior of the second."
  "</text--->"
  "<text>query point ininterior region</text--->"
  ") )";

const string SpatialIntersectionSpec  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>{point x points, line, sline, region } x"
  "   {point, points, line, sline, region} -> T, "
  " where T = points if any point or point type is one of the "
  " arguments or the argument having the smaller dimension </text--->"
  "<text>intersection(arg1, arg2)</text--->"
  "<text>intersection of two spatial objects</text--->"
  "<text>query intersection(tiergarten, thecenter) </text--->"
  ") )";


const string SpatialMinusSpec  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>{point, points, line, sline, region } x"
  "   {point, points, line, sline region} -> T "
  " </text--->"
  "<text>arg1 minus arg2</text--->"
  "<text>difference of two spatial objects</text--->"
  "<text>query tiergarten minus thecenter </text--->"
  ") )";

const string SpatialUnionSpec  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>{point , points, line, sline, region } x"
  "   {point, points, line, sline, region} -> T "
  " </text--->"
  "<text>arg1 union arg2</text--->"
  "<text>union of two spatial objects</text--->"
  "<text>query tiergarten union thecenter </text--->"
  ") )";

const string SpatialSpecCrossings  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>(line x line) -> points || "
  "sline x sline -> points || line -> points</text--->"
  "<text>crossings( _, _ )</text--->"
  "<text>crossing points of two (or one) line(s).</text--->"
  "<text>query crossings(line1, line2)</text--->"
  ") )";

const string SpatialSpecSingle  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>(points) -> point</text--->"
  "<text> single( _ )</text--->"
  "<text>transform a single-element points value to point value.</text--->"
  "<text>query single(points)</text--->"
  ") )";

const string SpatialSpecDistance  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>(point||points||line||sline||rect||region x "
  "point||points||line||sline||rect||region) [ x geoid] -> real</text--->"
  "<text>distance( _, _,[,_] )</text--->"
  "<text>compute distance between two spatial objects. Note: spherical"
  " distance (using geoid) is not available for all types.</text--->"
  "<text>query distance(point, line)</text--->"
  ") )";

const string distanceSmallerThanSpec  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>line x line x real x bool -> bool "
  "</text--->"
  "<text>distanceSmallerThan( _, _ ,_,_)</text--->"
  "<text>Checks if the distance is smaller than a given value"
  " If the boolean parameter is set to be true, also an"
  " distance equals to the given max Dist is allowed. .</text--->"
  "<text>query distanceSmallerThan(l1,l2,50.0,TRUE)</text--->"
  ") )";
const string SpatialSpecDirection  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>point x point -> real</text--->"
  "<text>direction( P1, P2 [, Geoid] )</text--->"
  "<text>Compute the direction DIR from point P1 to point P1 in degrees "
  "(0<=DIR<360) using standard mathematical measure (counter-clockwise, "
  "0.0 degrees means parallel to the X-axis). The direction of a point to "
  "itself is UNDEFINED. If a Geoid is passed, P1, P2 are expected to be valid "
  "geographic positions, otherwise euclidean coordinates are assumed.</text--->"
  "<text>query direction(p1, p2)</text--->"
  ") )";

const string SpatialSpecHeading  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>point x point [x geoid] -> real</text--->"
  "<text>heading( P1, P2 [, Geoid] )</text--->"
  "<text>Compute the heading HEAD from point P1 to point P1 in degrees "
  "(0<HEAD<=360) using standard aviation heading (clockwise, 360 degrees "
  "means NORTH --- 0 is not allowed). The heading of a point to "
  "itself is UNDEFINED. If a Geoid is passed, P1, P2 are expected to be valid "
  "geographic positions, otherwise euclidean coordinates are assumed.</text--->"
  "<text>query heading(p1, p2)</text--->"
  ") )";

const string SpatialSpecNocomponents  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>(points||line||region) -> int</text--->"
  "<text> no_components( _ )</text--->"
  "<text>return the number of components (points: points, line: segments, "
  "region: faces) of a spatial object.</text--->"
  "<text>query no_components(region)</text--->"
  ") )";

const string SpatialSpecNoSegments  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>{line, region, sline, dline} -> int</text--->"
  "<text> no_segments( _ )</text--->"
  "<text>return the number of half segments of a region.</text--->"
  "<text>query no_segments(region)</text--->"
  ") )";

const string SpatialSpecBbox  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>(point||points||line||region||sline) [x geoid]-> rect</text--->"
  "<text> bbox( Obj [, Geoid ] )</text--->"
  "<text>Returns the bounding box of a spatial object Obj. If Geoid is passed, "
  "the geographical bounding box is computed.</text--->"
  "<text>query bbox(tiergarten)</text--->"
  ") )";

const string SpatialSpecSize  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text> {line|sline|region} -> real \n"
  " {line|sline} x geoid -> real</text--->"
  "<text> size( O [, Geoid ] )</text--->"
  "<text>Returns the size (line, sline: length, region: area) of a spatial "
  "object O. For sline and line objects, the optional parameter Geoid allows "
  "to compute line lengthes based on (LON,LAT)-coordinates instead of metric "
  "(X,Y)-coordinates.\n"
  "CAVEAT: For a 'line' the result is only valid if the original segments do "
  "not cross each other. For 'sline' the result should always be correct."
  "</text--->"
  "<text> query size(line)</text--->"
  ") )";

const string SpatialSpecTouchpoints  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>(line||region x region) -> points</text--->"
  "<text> touchpoints( _, _ ) </text--->"
  "<text> return the touch points of a region and another "
  "region or line.</text--->"
  "<text> query touchpoints(line, region)</text--->"
  ") )";

const string SpatialSpecCommonborder  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>(region x region) -> line</text--->"
  "<text> commonborder( _, _ )</text--->"
  "<text> return the common border of two regions.</text--->"
  "<text> query commonborder(region1, region2)</text--->"
  ") )";


const string SpatialSpecCenter  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text> points -> point </text--->"
  "<text> center( _ ) </text--->"
  "<text> computes the center of the points value</text--->"
  "<text> query center(vertices(tiergarten))</text--->"
  ") )";

const string SpatialSpecConvexhull  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text> points -> region </text--->"
  "<text> convexhull( _ ) </text--->"
  "<text> computes the convex hull of the points value</text--->"
  "<text> query convexhull(vertices(tiergarten))</text--->"
  ") )";

const string SpatialSpecAdd  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>point x point -> point</text--->"
  "<text> _ + _</text--->"
  "<text> Returns the vector sum of two points.</text--->"
  "<text> query [const point value (0.0 -1.2)] + "
  "[const point value (-5.0 1.2)] </text--->"
  ") )";

const string SpatialSpecWindowClippingIn  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>(line x rect) -> line, (region x rect) --> region</text--->"
  "<text> windowclippingin( _, _ ) </text--->"
  "<text> computes the part of the object that is inside the window.</text--->"
  "<text> query windowclippingin(line1, window)</text--->"
  ") )";

const string SpatialSpecWindowClippingOut  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>(line x rect) -> line, (region x rect) --> region</text--->"
  "<text> windowclippingout( _, _ ) </text--->"
  "<text> computes the part of the object that is outside the window.</text--->"
  "<text> query windowclippingout(line1, rect)</text--->"
  ") )";


const string SpatialSpecComponents  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
  "( <text>points -> stream(points), region -> stream(region), "
  "line -> stream(line)</text--->"
  "<text>components( _ )</text--->"
  "<text>Returns the components of a points (the contained point values) or "
  "region (the contained faces) object as a stream."
  "Both, empty and undefined objects result in empty stream.</text--->"
  "<text>query components(r1) count;</text--->"
  ") )";

const string SpatialSpecVertices  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
  "( <text>(region -> points) or (line -> points)</text--->"
  "<text>vertices(_)</text--->"
  "<text>Returns the vertices of a region or line as a stream."
  "Both, empty and undefined objects result in empty stream.</text--->"
  "<text>query vertices(r1)</text--->"
  ") )";

const string SpatialSpecBoundary  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
  "( <text>(region -> line) or (line -> points)</text--->"
  "<text>boundary(_)</text--->"
  "<text>Returns the boundary of a region or a line.</text--->"
  "<text>query boundary(thecenter)</text--->"
  ") )";

const string SpatialSpecAtPoint  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
  "( <text>sline x point [x bool] -> real</text--->"
  "<text>atpoint(_, _, _)</text--->"
  "<text>Returns the relative position of the point on the line."
  "The optional boolean flag indicates where the positions start, i.e. "
  "from the smaller point (TRUE) or from the bigger one (FALSE)."
  "If the boolean flag is not given the orientation is taken from the sline"
  "</text---><text>query atpoint(l, p, TRUE)</text--->"
  ") )";

const string SpatialSpecAtPosition  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
  "( <text>sline x real [x bool] -> point</text--->"
  "<text>atposition(_, _, _)</text--->"
  "<text>Returns the point at a relative position in the line."
  "The optional boolean flag indicates where the positions start, i.e. "
  "from the smaller point (TRUE) or from the bigger one (FALSE)."
  "If the boolean flag is  not given the orientation is taken from the sline"
  "</text---><text>query atposition(l, 0.0, TRUE)</text--->"
  ") )";

const string SpatialSpecSubLine  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
  "( <text>sline x real x real x bool -> line</text--->"
  "<text>subline(_, _, _, _)</text--->"
  "<text>Returns the sub-line inside the two relative positions."
  "The boolean flag indicates where the positions start, i.e. "
  "from the smaller point (TRUE) or from the bigger one (FALSE)."
  "</text---><text>query subline(l, 0.0, size(.l), TRUE)</text--->"
  ") )";

const string SpatialSpecGetX  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>point -> real</text--->"
  "<text>getx( _ )</text--->"
  "<text>Extracts the x-component of a point.</text--->"
  "<text> query getx([const point value (0.0 -1.2)])</text--->"
  ") )";

const string SpatialSpecGetY  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>point -> real</text--->"
  "<text>gety( _ )</text--->"
  "<text>Extracts the y-component of a point.</text--->"
  "<text> query gety([const point value (0.0 -1.2)])</text--->"
  ") )";

const string SpatialSpecLine2Region  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>line -> region</text--->"
  "<text>_ line2region</text--->"
  "<text>Converts a line object to a region object.</text--->"
  "<text> query gety([const point value (0.0 -1.2)])</text--->"
  ") )";

const string SpatialSpecRect2Region  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "( <text>rect -> region</text--->"
    "<text>_ rect2region</text--->"
    "<text>Converts a rect object to a region object.</text--->"
    "<text> query </text--->"
    ") )";

const string SpatialSpecCreateTriangle  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "( <text>point x poin x point -> region</text--->"
    "<text>create_triangle( P1, P2, P3 )</text--->"
    "<text>Creates a triangular region with vertexes P1, P2, P3. If Any two "
    "points are equal, the result is empty. If any point is undefined, the "
    "result is undefined. Use operators such as 'convexhull', 'union', and "
    "'minus' to compose more complex regions.</text--->"
    "<text> query create_triangle(makepoint(0,0),makepoint(0,10),"
    "makepoint(0,-10))</text--->"
    ") )";

const string SpatialSpecArea  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "( <text>region -> real</text--->"
    "<text>area( _ )</text--->"
    "<text>Returns the area of a region object as a real value.</text--->"
    "<text> query area( tiergarten )</text--->"
    ") )";

const string SpatialSpecPolylines  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "( <text>line  x bool [ x points] -> stream( line ) </text--->"
    "<text> _  polylines [ _ , _ ] </text--->"
    "<text>Returns a stream of simple line objects "
    "whose union is the original line. The boolean parameter"
    "indicates to ignore critical points as splitpoints.</text--->"
    "<text> query trajectory(train1) polylines [TRUE]  count</text--->"
    ") )";

const string SpatialSpecPolylinesC  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "( <text>line  x bool [ x points] -> stream( line ) </text--->"
    "<text> _  polylinesC [ _ , _ ] </text--->"
    "<text>Returns a stream of simple line objects "
    " whose union is the original line. The boolean parameter"
    "indicates to ignore critical points (branches) as splitpoints."
    " Some of the  resulting polylines may build a cycle.</text--->"
    "<text> query trajectoryC(train1) polylines [TRUE]  count</text--->"
    ") )";

const string SpatialSpecSimplify  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "( <text>line  x real [x points x [geoid] ] -> line </text--->"
    "<text> simplify( line, epsilon, [, ips ] ) </text--->"
    "<text>Simplifies a line value, using a maximum error of 'epsilon'. The "
    "points value 'ips' marks important points, that must be kept. </text--->"
    "<text> query simplify(trajectory(train1),10.0) count</text--->"
    ") )";

const string SpatialSpecSegments  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "( <text>line  -> stream( line ) </text--->"
    "<text> segments( _ ) </text--->"
    "<text>Returns a stream of segments of the line.</text--->"
    "<text>query  (segments(PotsdamLine) count) = "
                 "(no_segments(PotsdamLine)) </text--->"
    ") )";

const string SpatialSpecGet  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "( <text>points x int -> point </text--->"
    "<text> _ get _  </text--->"
    "<text>Returns a point from a points value.</text--->"
    "<text>query  vertices(BGrenzenLine) get [1]    </text--->"
    ") )";


const string SpatialSpecMakeLine  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "( <text>point x point  -> line </text--->"
    "<text> makeline( _, _ )  </text--->"
    "<text>Create a one-segment line from the arguments.</text--->"
    "<text>query makeline([const point value (0 0)],"
           " [ const point value (100 40)])</text--->"
    ") )";

const string SpatialSpecMakeSLine  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "( <text>point x point  -> line </text--->"
    "<text> makesline( _, _ )  </text--->"
    "<text>Create a one-segment sline from the arguments. The starting point "
    "is determined automatically based on the ordering of point values: the "
    "'smaller' argument becomes the sline's starting point. If you want to "
    "determine the starting point by yourself, use operator 'create_sline' "
    "instead!</text--->"
    "<text>query makesline([const point value (0 0)],"
           " [ const point value (100 40)])</text--->"
    ") )";

const string CommonBorder2Spec =
 "((\"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
 " ( <text> region x region -> line </text--->"
 " \"  _ commonborder2  _ \" "
 "  <text> computes the common part of the"
 " boundaries of the arguments </text---> "
  "  \" query r1 commonborder2 r2 \" ))";


const string SpatialSpecRealminize  =
     "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
     "( <text>line -> line </text--->"
     "<text> realminize( _ )  </text--->"
     "<text>Returns the realminized argument: segments are split at each inner "
     "crosspoint.</text--->"
     "<text>query realminize(train7sections)</text--->"
     ") )";

const string SpatialSpecToLine  =
     "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
     "( <text>sline -> line </text--->"
     "<text> toline( _ )  </text--->"
     "<text>Converts an sline into a line</text--->"
     "<text>query toline(fromline(trajectory(train7))</text--->"
     ") )";

const string SpatialSpecFromLine  =
     "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
     "( <text>line [x bool]-> sline </text--->"
     "<text> fromline( _ ) fromline (_ ,_)  </text--->"
     "<text>Converts a line into an sline, if no bool value is defined the"
     "sline starts from the smaller endpoint. Otherwise the bool value "
     "tells if the sline starts from the smaller or bigger endpoint.</text--->"
     "<text>query toline(fromline(trajectory(train7))</text--->"
     ") )";

const string SpatialSpecIsCycle  =
     "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
     "( <text>sline -> bool </text--->"
     "<text> iscycle( _ )  </text--->"
     "<text>check for cycle</text--->"
     "<text>query iscycle(fromline(trajectory(train7))</text--->"
     ") )";

const string utmSpec  =
     "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
     "( <text>t -> t, t in {point, points} </text--->"
     "<text> utm( _ )  </text--->"
     "<text>projects the arguments using the utm projection</text--->"
     "<text>query utm([const point value ( 0 0)])</text--->"
     ") )";

const string gkSpec  =
   "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
   "( <text>t [ x int] -> t, t in {point, points, line, region} </text--->"
   "<text> gk( geoobj [, zone] )  </text--->"
   "<text>Projects the argument 'geoobj' using the Gauss Krueger projection "
   "with center meridian 'zone'. Zone width is 3°. If 'zone' is not provided,"
   "2 (center meridian = 6°E, suits the location of Hagen) will be used as a "
   "default. 'geoobj' is expected to have geografic coordinates (LON,LAT) in °"
   ", the result's coordinates (NORTHING,EASTING) are in metres.</text--->"
   "<text>query gk([const point value (0 0)])</text--->"
   ") )";

const string reverseGkSpec  =
   "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
   "( <text>t -> t, t in {point, points, line, region} </text--->"
   "<text> reverseGk( geoobj  )  </text--->"
   "<text> Converts a spatial objects from Gauss Krueger reference systems"
   " into geographic coordinates using the WGS1984 ellipsoid. "
   " The computation is iterative, thus the result is only an approximation."
   " Whenever problems occur,i.e. if a coordinates is not in the Gauss"
   " Krueger system,the result will be undefined."
   "</text--->"
   "<text>query reverseGk(gk([const point value (7 51)]))</text--->"
   ") )";
/*
Operations on geoids

*/
const string geoid_getRadius_SPEC  =
   "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
   "( <text>geoid -> real</text--->"
   "<text> getRadius( G )  </text--->"
   "<text>Returns the radius parameter of geoid G.</text--->"
   "<text>query getRadius([const geoid value WGS1984])</text--->"
   ") )";

const string geoid_getFlattening_SPEC  =
   "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
   "( <text>geoid -> real</text--->"
   "<text> getFlattening( G )  </text--->"
   "<text>Returns the flattening parameter of geoid G.</text--->"
   "<text>query getFlattening([const geoid value UnitSphere]) = 0.0</text--->"
   ") )";

const string geoid_create_geoid_SPEC  =
   "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
   "( <text> string [ x {real|int} x {real|int} -> geoid</text--->"
   "<text> create_geoid( Name [, Radius, Flattening] )  </text--->"
   "<text>Returns a geoid. If only the string parameter Name is used, it must "
   "be the name of a predefined geoid (available: "+Geoid::getGeoIdNames()+"). "
   "Otherwise, Radius must be the positive radius, and 0.0 <= Flattening <= 1.0"
   "is the flattening parameter (0.0 results a perfect sphere).</text--->"
   "<text>query create_geoid(\"MyGeoid\", 1.0, 0.5)</text--->"
   ") )";

const string SpatialSpecCollectLine  =
   "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
   "( <text>stream(T) -> line, T in {point, line, sline} </text--->"
   "<text> _ collect_line [ IgnoreUndef ]</text--->"
   "<text>Collects a stream of 'line' or 'sline' values into a single 'line' "
   "value. Creates a 'line' value by consecutively connecting subsequent "
   "'point' values from the stream by segments. If the stream provides 0 or 1"
   "Element, the result is defined, but empty.\n"
   "If any of the stream elements is undefined and 'ignoreUndef' is set to "
   "FALSE, so is the resulting 'line' value.</text--->"
   "<text>query [const line value ()] feed collect_line [TRUE] )</text--->"
   ") )";

const string SpatialSpecCollectSLine  =
   "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
   "( <text>stream(T) -> sline, T in {point, line, sline} </text--->"
   "<text> _ collect_sline [ IgnoreUndef ]</text--->"
   "<text>Collects a stream of 'line' or 'sline' values into a single 'sline' "
   "value. Creates a 'sline' value by consecutively connecting subsequent "
   "'point' values from the stream by segments. If the stream provides 0 or 1"
   "Element, the result is defined, but empty.\n"
   "If any of the stream elemnts is undefined and parameter 'IgnoreUndef' is "
   "set to FALSE, the resulting 'sline' value is undef. If any segements cross "
   "each other or the segments do not form a single curve, the result is "
   "undefined.</text--->"
   "<text>query [const line value ()] feed collect_sline[TRUE])</text--->"
   ") )";

const string SpatialSpecCollectPoints  =
   "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
   "( <text>stream(T) x bool  -> points, T in {point, points} </text--->"
   "<text> _ collect_points[ IgnoreUndef ] </text--->"
   "<text>Collects a stream of point or points values into a single points "
   "value. If the bool parameter 'IgnoreUndef' is set to TRUE, undefined "
   "points within the  stream are ignored. Otherwise the result is set to be "
   "undefined if an undefined value is inside the stream. </text--->"
   "<text>query [const points value ()] feed collect_points[true])</text--->"
   ") )";

const string SpatialSpecSetStartSmaller  =
"( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
"( <text>sline x bool  -> sline </text--->"
"<text> set_startsmaller( l, b ) </text--->"
"<text>Sets the starting flag for a simple line 'l' to 'b'. </text--->"
"<text>query get_startsmaller(set_startsmaller(fromline(BGrenzenLine),FALSE)) "
"= FALSE</text--->"
") )";

const string SpatialSpecGetStartSmaller  =
"( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
"( <text>sline -> bool </text--->"
"<text> get_startsmaller( l ) </text--->"
"<text>Retrieves the starting flag for a simple line 'l'. </text--->"
"<text>query get_startsmaller(set_startsmaller(fromline(BGrenzenLine),FALSE)) "
"= FALSE</text--->"
") )";

const string SpatialSpecCreateSline  =
"( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
"( <text>point x point -> sline </text--->"
"<text> create_sline( p1, p2 ) </text--->"
"<text>Creates a simple line, directed from p1 to p2. If p1 = p2, the result is"
" empty.</text--->"
"<text>query atposition(create_sline(makepoint(10,10),makepoint(0,0)),0.0,TRUE)"
"= makepoint(10,10)</text--->"
") )";

/*
10.5.3 Definition of the operators

*/
Operator spatialisempty (
  "isempty",
  SpatialSpecIsEmpty,
  5,
  spatialisemptymap,
  SpatialSelectIsEmpty,
  SpatialTypeMapBool1 );

Operator spatialequal (
  "=",
  SpatialSpecEqual,
  7,
  spatialequalmap,
  SpatialSelectEqual,
  SpatialTypeMapEqual );

Operator spatialisless (
  "<",
  SpatialSpecIsLess,
  1,
  spatialislessmap,
  SpatialSelectIsLess,
  SpatialTypeMapIsLess );

Operator spatialnotequal (
  "#",
  SpatialSpecNotEqual,
  5,
  spatialnotequalmap,
  SpatialSelectCompare,
  SpatialTypeMapCompare );

Operator spatialintersects (
  "intersects1",
  SpatialSpecIntersects,
  10,
  spatialintersectsmap,
  SpatialSelectIntersects,
  IntersectsTM );

Operator spatialinside (
  "inside",
  SpatialSpecInside,
  11,
  spatialinsidemap,
  SpatialSelectInside,
  InsideTypeMap );

Operator spatialadjacent (
  "adjacent",
  SpatialSpecAdjacent,
  5,
  spatialadjacentmap,
  SpatialSelectAdjacent,
  AdjacentTypeMap );

Operator spatialoverlaps (
  "overlaps",
  SpatialSpecOverlaps,
  SpatialOverlaps_rr,
  Operator::SimpleSelect,
  RegionRegionMapBool );

Operator spatialonborder (
  "onborder",
  SpatialSpecOnBorder,
  SpatialOnBorder_pr,
  Operator::SimpleSelect,
  PointRegionMapBool );

Operator spatialininterior (
  "ininterior",
  SpatialSpecIninterior,
  SpatialInInterior_pr,
  Operator::SimpleSelect,
  PointRegionMapBool );


Operator spatialintersection (
  "intersection1",
  SpatialIntersectionSpec,
  25,
  spatialintersectionVM,
  SpatialSetOpSelect,
  SpatialIntersectionTypeMap );

Operator spatialminus (
  "minus1",
  SpatialMinusSpec,
  25,
  spatialminusVM,
  SpatialSetOpSelect,
  SpatialMinusTypeMap );

Operator spatialunion (
  "union1",
  SpatialUnionSpec,
  25,
  spatialunionVM,
  SpatialSetOpSelect,
  SpatialUnionTypeMap );

Operator spatialcrossings (
  "crossings1",
  SpatialSpecCrossings,
  3,
  SpatialCrossingsMap,
  SpatialSelectCrossings,
  SpatialCrossingsTM );

Operator spatialsingle (
  "single",
  SpatialSpecSingle,
  SpatialSingle_ps,
  Operator::SimpleSelect,
  SpatialSingleMap );

Operator spatialdistance (
  "distance",
  SpatialSpecDistance,
  36,
  spatialdistancemap,
  SpatialSelectDistance,
  SpatialDistanceMap );


Operator distanceSmallerThan (
  "distanceSmallerThan",
  distanceSmallerThanSpec,
  distanceSmallerThanVM<Line,Line>,
  Operator::SimpleSelect,
  distanceSmallerThanTM );

Operator spatialdirection (
  "direction",
  SpatialSpecDirection,
  SpatialDirectionHeading_pp<false>,
  Operator::SimpleSelect,
  SpatialDirectionHeadingMap );

Operator spatialheading (
  "heading",
  SpatialSpecHeading,
  SpatialDirectionHeading_pp<true>,
  Operator::SimpleSelect,
  SpatialDirectionHeadingMap );

Operator spatialnocomponents (
  "no_components",
  SpatialSpecNocomponents,
  3,
  spatialnocomponentsmap,
  SpatialSelectNoComponents,
  SpatialNoComponentsMap );

Operator spatialnosegments (
  "no_segments",
  SpatialSpecNoSegments,
  4,
  spatialnosegmentsmap,
  SpatialSelectNoSegments,
  SpatialNoSegmentsMap );

Operator spatialbbox (
  "bbox",
  SpatialSpecBbox,
  6,
  spatialbboxmap,
  SpatialSelectBBox,
  SpatialBBoxMap );

Operator spatialsize (
  "size",
  SpatialSpecSize,
  3,
  spatialsizemap,
  SpatialSelectSize,
  SpatialSizeMap );

Operator spatialtouchpoints (
  "touchpoints",
  SpatialSpecTouchpoints,
  3,
  spatialtouchpointsmap,
  SpatialSelectTouchPoints,
  SpatialTouchPointsMap );

Operator spatialcommonborder (
  "commonborder",
  SpatialSpecCommonborder,
  SpatialCommonBorder_rr,
  Operator::SimpleSelect,
  SpatialCommonBorderMap );




Operator spatialcenter (
  "center",
  SpatialSpecCenter,
  SpatialCenter,
  Operator::SimpleSelect,
  SpatialCenterMap );

Operator spatialconvexhull (
  "convexhull",
  SpatialSpecConvexhull,
  SpatialConvexhull,
  Operator::SimpleSelect,
  SpatialConvexhullMap );

Operator spatialadd (
  "+",
  SpatialSpecAdd,
  1,
  spatialaddmap,
  Operator::SimpleSelect,
  SpatialAddTypeMap );

Operator spatialwindowclippingin (
  "windowclippingin",
  SpatialSpecWindowClippingIn,
  4,
  spatialwindowclippinginmap,
  SpatialSelectWindowClipping,
  SpatialWindowClippingMap );

Operator spatialwindowclippingout (
  "windowclippingout",
  SpatialSpecWindowClippingOut,
  4,
  spatialwindowclippingoutmap,
  SpatialSelectWindowClipping,
  SpatialWindowClippingMap );


Operator spatialcomponents (
  "components",
  SpatialSpecComponents,
  3,
  spatialcomponentsmap,
  SpatialComponentsSelect,
  SpatialComponentsMap );

Operator spatialvertices (
  "vertices",
  SpatialSpecVertices,
  2,
  spatialverticesmap,
  SpatialVerticesSelect,
  SpatialVerticesMap);

Operator spatialboundary (
  "boundary",
  SpatialSpecBoundary,
  2,
  spatialboundarymap,
  SpatialBoundarySelect,
  SpatialBoundaryMap);

Operator spatialsimplify (
  "simplify",
  SpatialSpecSimplify,
  2,
  spatialsimplifymap,
  SpatialSimplifySelect,
  SimplifyTypeMap );

Operator spatialatpoint (
  "atpoint",
  SpatialSpecAtPoint,
  2,
  SpatialAtPointMap,
  SpatialAtPointSelect,
  SpatialAtPointTypeMap );

Operator spatialatposition (
  "atposition",
  SpatialSpecAtPosition,
  2,
  SpatialAtPositionMap,
  SpatialAtPositionSelect,
  SpatialAtPositionTypeMap );

Operator spatialsubline (
  "subline",
  SpatialSpecSubLine,
  SpatialSubLine,
  Operator::SimpleSelect,
  SpatialSubLineMap );

Operator spatialgetx (
  "getx",
  SpatialSpecGetX,
  1,
  spatialgetxmap,
  Operator::SimpleSelect,
  SpatialGetXYMap );

Operator spatialgety (
  "gety",
  SpatialSpecGetY,
  1,
  spatialgetymap,
  Operator::SimpleSelect,
  SpatialGetXYMap );

Operator spatialline2region (
  "line2region",
  SpatialSpecLine2Region,
  SpatialLine2Region,
  Operator::SimpleSelect,
  SpatialLine2RegionMap );

Operator spatialrect2region (
  "rect2region",
  SpatialSpecRect2Region,
  SpatialRect2Region,
  Operator::SimpleSelect,
  SpatialRect2RegionMap );

Operator spatialCreateTriangle (
  "create_triangle",
  SpatialSpecCreateTriangle,
  SpatialCreateTriangleVM,
  Operator::SimpleSelect,
  SpatialCreateTriangleTM );

Operator spatialarea (
    "area",
  SpatialSpecArea,
  SpatialArea,
  Operator::SimpleSelect,
  SpatialAreaMap );


Operator spatialpolylines (
  "polylines",
  SpatialSpecPolylines,
  SpatialPolylines<false>,
  Operator::SimpleSelect,
  PolylinesMap );

Operator spatialpolylinesC (
  "polylinesC",
  SpatialSpecPolylinesC,
  SpatialPolylines<true>,
  Operator::SimpleSelect,
  PolylinesMap );

Operator spatialsegments (
  "segments",
  SpatialSpecSegments,
  2,
  SpatialSegmentsVM,
  SpatialSegmentsSelect,
  SegmentsTypeMap );

Operator spatialget (
  "get",
  SpatialSpecGet,
  SpatialGet,
  Operator::SimpleSelect,
  GetTypeMap );

Operator makeline (
  "makeline",
  SpatialSpecMakeLine,
  SpatialMakeLine<Line>,
  Operator::SimpleSelect,
  MakeLineTypeMap );

Operator makesline (
    "makesline",
  SpatialSpecMakeSLine,
  SpatialMakeLine<SimpleLine>,
  Operator::SimpleSelect,
  MakeSLineTypeMap );

Operator realminize(
     "realminize",           //name
     SpatialSpecRealminize,   //specification
     RealminizeVM, //value mapping
     Operator::SimpleSelect,         //trivial selection function
     RealminizeTypeMap //type mapping
);



Operator commonborder2(
         "commonborder2",           //name
          CommonBorder2Spec,   //specification
          CommonBorder2VM, //value mapping
          Operator::SimpleSelect,         //trivial selection function
          CommonBorder2TypeMap //type mapping
);


Operator spatialtoline (
  "toline",
  SpatialSpecToLine,
  toLineVM,
  Operator::SimpleSelect,
  toLineTypeMap );

Operator spatialfromline (
  "fromline",
  SpatialSpecFromLine,
  2,
  fromLineVM,
  fromLineSelect,
  fromLineTypeMap );

Operator spatialiscycle (
  "iscycle",
  SpatialSpecIsCycle,
  isCycleVM,
  Operator::SimpleSelect,
  isCycleTypeMap );



Operator utmOp (
  "utm",
   utmSpec,
   2,
   utmVM,
   utmSelect,
   utmTypeMap );

Operator gkOp (
  "gk",
   gkSpec,
   6,
   gkVM,
   gkSelect,
   gkTypeMap );

Operator reverseGkOp (
  "reverseGk",
   reverseGkSpec,
   4,
   reverseGKVM,
   reverseGkSelect,
   reverseGkTypeMap );
/*
Operators for creating and querying geoids

*/
Operator geoid_getRadius
(
  "getRadius",
  geoid_getRadius_SPEC,
  geoid_getRadius_VM,
  Operator::SimpleSelect,
  geoid2real_TM
);

Operator geoid_getFlattening
(
  "getFlattening",
  geoid_getFlattening_SPEC,
  geoid_getFlattening_VM,
  Operator::SimpleSelect,
  geoid2real_TM
);

Operator geoid_create_geoid
(
  "create_geoid",
  geoid_create_geoid_SPEC,
  5,
  geoid_create_geoid_vm,
  geoid_create_geoid_SELECT,
  geoid_create_geoid_TM
);

/*
Creating lines and slines

*/
Operator spatialcollect_line (
  "collect_line",
  SpatialSpecCollectLine,
  3,
  spatialCollectLineMap,
  SpatialCollectLineSelect,
  SpatialCollectLineTypeMap);

Operator spatialcollect_sline (
  "collect_sline",
  SpatialSpecCollectSLine,
  3,
  spatialCollectSLineMap,
  SpatialCollectLineSelect,
  SpatialCollectSLineTypeMap);

Operator spatialcollect_points (
  "collect_points",
  SpatialSpecCollectPoints,
  2,
  spatialCollectPointsMap,
  SpatialCollectPointsSelect,
  SpatialCollectPointsTM);

Operator spatialsetstartsmaller (
  "set_startsmaller",
  SpatialSpecSetStartSmaller,
  SpatialVMSetStartSmaller,
  Operator::SimpleSelect,
  SpatialTMSetStartSmaller);

Operator spatialgetstartsmaller (
  "get_startsmaller",
  SpatialSpecGetStartSmaller,
  SpatialVMGetStartSmaller,
  Operator::SimpleSelect,
  SpatialTMGetStartSmaller);

Operator spatial_create_sline (
  "create_sline",
  SpatialSpecCreateSline,
  SpatialVMCreateSline,
  Operator::SimpleSelect,
  SpatialTMCreateSline
);

/*
5.15 Operator ~makepoint~

5.15.1 Type Mapping for ~makepoint~

---- {int|real} x {int|real} --> point
----

*/
ListExpr
TypeMapMakepoint( ListExpr args )
{
  if( ( nl->ListLength( args ) == 2)
      && listutils::isNumericType(nl->First(args))
      && listutils::isNumericType(nl->Second(args))){
    return nl->SymbolAtom(Point::BasicType());
  }
  return listutils::typeError("Expected ({int|real} x {int|real}).");
}

/*
5.15.2 Value Mapping for ~makepoint~

*/
template<class T1, class T2>
int MakePoint( Word* args, Word& result, int message, Word& local, Supplier s )
{
  const T1* value1 = static_cast<const T1*>(args[0].addr);
  const T2* value2 = static_cast<const T2*>(args[1].addr);
  result = qp->ResultStorage( s );
  Point* res = static_cast<Point*>(result.addr);

  if(!value1->IsDefined() || !value2->IsDefined()){
    res->SetDefined(false);
    return 0;
  }
  res->Set(value1->GetValue(), value2->GetValue()); // also sets to DEFINED
  return 0;
}

ValueMapping VM_MakePoint[] = {
  MakePoint<CcInt,CcInt>,
  MakePoint<CcInt,CcReal>,
  MakePoint<CcReal,CcInt>,
  MakePoint<CcReal,CcReal>
};

int SelectMakepoint( ListExpr args ){
  int res = 0;
  if(listutils::isSymbol(nl->First(args) ,CcReal::BasicType())) res+=2;
  if(listutils::isSymbol(nl->Second(args),CcReal::BasicType())) res+=1;
  return res;
}

/*
5.15.3 Specification for operator ~makepoint~

*/
const string
SpatialSpecMakePoint =
"( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
"( <text>{real|int} x {real|int} -> point</text--->"
"<text>makepoint ( _, _ ) </text--->"
"<text>create a point from two "
"given real or integer coordinates.</text--->"
"<text>makepoint (5.0,5.0)</text---> ) )";

/*
5.15.4 Selection Function of operator ~makepoint~

Not necessary.

*/

/*
5.15.5  Definition of operator ~makepoint~

*/
Operator spatialmakepoint( "makepoint",
                            SpatialSpecMakePoint,
                            4,
                            VM_MakePoint,
                            SelectMakepoint,
                            TypeMapMakepoint);


/*
6.16.1 Operator halfSegments

*/
ListExpr halfSegmentsTM(ListExpr args){
  if(nl->ListLength(args)!=1){
     return listutils::typeError("one argument expected");
  }
  ListExpr first = nl->First(args);
  if((listutils::isSymbol(first,Line::BasicType()) ||
      listutils::isSymbol(first,Region::BasicType())) ){

   ListExpr attrList = nl->OneElemList( nl->TwoElemList(
                                          nl->SymbolAtom("FaceNo"),
                                          nl->SymbolAtom(CcInt::BasicType())));
   ListExpr last = attrList;
   last = nl->Append(last, nl->TwoElemList(nl->SymbolAtom("CycleNo"),
                                           nl->SymbolAtom(CcInt::BasicType())));


   last = nl->Append(last, nl->TwoElemList(nl->SymbolAtom("EdgeNo"),
                                           nl->SymbolAtom(CcInt::BasicType())));

   last = nl->Append(last, nl->TwoElemList(nl->SymbolAtom("CoverageNo"),
                                           nl->SymbolAtom(CcInt::BasicType())));

   last = nl->Append(last, nl->TwoElemList(nl->SymbolAtom("InsideAbove"),
                                          nl->SymbolAtom(CcBool::BasicType())));

   last = nl->Append(last, nl->TwoElemList(nl->SymbolAtom("PartnerNo"),
                                           nl->SymbolAtom(CcInt::BasicType())));

   last = nl->Append(last, nl->TwoElemList(nl->SymbolAtom("Ldp"),
                                          nl->SymbolAtom(CcBool::BasicType())));

   last = nl->Append(last, nl->TwoElemList(nl->SymbolAtom("Segment"),
                                           nl->SymbolAtom(Line::BasicType())));

    return nl->TwoElemList(
         nl->SymbolAtom(Symbol::STREAM()),
         nl->TwoElemList(
           nl->SymbolAtom(Tuple::BasicType()),
           attrList));

  } else {
   return listutils::typeError("line or region expected");
 }
}


template<class T>
class halfSegmentsLocalInfo{
  public:
    halfSegmentsLocalInfo(T* s, ListExpr tt){
      if(!s->IsDefined()){
        source = 0;
        size = 0;
        pos = 0;
        tupleType = 0;
      } else {
         source = s;
         size = s->Size();
         pos = 0;
         tupleType = new TupleType(tt);
      }
    }

    ~halfSegmentsLocalInfo(){
      if(tupleType){
         tupleType->DeleteIfAllowed();
         tupleType=0;
      }
    }

    Tuple* next(){
      if(pos>=size){
        return 0;
      }
      HalfSegment hs;
      source->Get(pos,hs);
      pos++;
      Tuple* res = new Tuple(tupleType);
      res->PutAttribute(0, new CcInt(true,hs.attr.faceno));
      res->PutAttribute(1, new CcInt(true,hs.attr.cycleno));
      res->PutAttribute(2, new CcInt(true,hs.attr.edgeno));
      res->PutAttribute(3, new CcInt(true,hs.attr.coverageno));
      res->PutAttribute(4, new CcBool(true,hs.attr.insideAbove));
      res->PutAttribute(5, new CcInt(true,hs.attr.partnerno));
      res->PutAttribute(6, new CcBool(true,hs.IsLeftDomPoint()));
      Line* line = new Line(2);
      line->StartBulkLoad();
      (*line) += hs;
      hs.SetLeftDomPoint(!hs.IsLeftDomPoint());
      (*line) += hs;
      line->EndBulkLoad();
      res->PutAttribute(7, line);
      return res;
    }

  private:
    T* source;
    int size;
    int pos;
    TupleType* tupleType;
};


template<class T>
int halfSegmentsVM( Word* args, Word& result, int message,
                   Word& local, Supplier s )
{
   halfSegmentsLocalInfo<T>* li =
         static_cast<halfSegmentsLocalInfo<T>*>(local.addr);
   switch(message){
     case OPEN: {
              if(li){
                delete li;
              }
              ListExpr resultType = GetTupleResultType( s );
              li = new halfSegmentsLocalInfo<T>( static_cast<T*>(args[0].addr),
                         nl->Second(resultType)
                     );
              local.addr=li;
              return 0;
           }
     case REQUEST:{
         result.addr  = li->next();
         return result.addr?YIELD:CANCEL;
     }
     case CLOSE:{
              if(li){
                delete li;
                local.addr=0;
              }
              return 0;
     }

   }
   return -1;
}


ValueMapping halfSegmentsvm[] = { halfSegmentsVM<Line>,
                                 halfSegmentsVM<Region> };


int halfSegmentsSelect(ListExpr args){
  return listutils::isSymbol(nl->First(args),Line::BasicType())?0:1;
}


const string
halfSegmentsSpec =
"( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
"( <text> {line,region} -> stream(tuple(faceno, cycleno,...))</text--->"
"<text>halfSegments(_) </text--->"
"<text>Gives the values of the halfsegment into a tuple stream  "
"</text--->"
"<text>query halfSegments(zoogarten) count</text---> ) )";


Operator halfSegmentsOp (
  "halfSegments",
   halfSegmentsSpec,
   2,
   halfSegmentsvm,
   halfSegmentsSelect,
   halfSegmentsTM );


ListExpr distanceOrthodromeTM(ListExpr args){
  int noargs = nl->ListLength(args);
  string errmsg = "Expected (point x point) or (point x point x geoid).";
  if(noargs < 2){
    return listutils::typeError(errmsg);
  }
  if(   !listutils::isSymbol(nl->First(args),Point::BasicType())
     || !listutils::isSymbol(nl->Second(args),Point::BasicType()) ){
    return listutils::typeError(errmsg);
  }
  if(noargs==2){
    return nl->SymbolAtom(CcReal::BasicType());
  }
  if(noargs != 3){
    return listutils::typeError(errmsg);
  }
  if(listutils::isSymbol(nl->Third(args),Geoid::BasicType())){
    return nl->SymbolAtom(CcReal::BasicType());
  }
  return listutils::typeError(errmsg);
};

int distanceOrthodromeVM( Word* args, Word& result, int message,
                          Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  CcReal* res = static_cast<CcReal*>(result.addr);
  Point* p1 = static_cast<Point*>(args[0].addr);
  Point* p2 = static_cast<Point*>(args[1].addr);
  if(!p1->IsDefined() || !p2->IsDefined()){
    res->Set(false, 0.0);
    return 0;
  }
  Geoid* g = 0;
  if(qp->GetNoSons(s)==3){
    g = static_cast<Geoid*>(args[2].addr);
    if(!g->IsDefined()){
      res->Set(false, 0.0);
      return 0;
    }
  } else { // use the WGS1984 as the default
    g = new Geoid(Geoid::WGS1984);
  }
  bool valid = true;
  res->Set(true,p1->DistanceOrthodrome(*p2,*g,valid));
  res->SetDefined(valid);
  if( (qp->GetNoSons(s)!=3) && g) {
    delete g; g = 0;
  }
  return 0;
}

const string distanceOrthodromeSpec =
   "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
   "( <text> point x point [x geoid] -> real</text--->"
   "<text>distanceOrthodrome( p1, p2 [, g]) </text--->"
   "<text>Returns the length of the orthodrome between point p1 and p2 "
   "according to the specified geoid g. Both points must represent geographic "
   "coordinates (LON,LAT), where -180<=LON<=180 and -90<=LAT<=90."
   "If any argument is invalid or undefined, the result is UNDEF. If no geoid "
   "is specified, WGS1984 is used as the default geoid.</text--->"
   "<text>query distanceOrthodrome(makepoint(7.494968217,51.376125146),"
   "makepoint(-73.984633618, 40.728925452 ),create_geoid(\"WGS1984\"))"
   "</text---> ) )";

Operator spatial_distanceOrthodrome (
   "distanceOrthodrome",
    distanceOrthodromeSpec,
    distanceOrthodromeVM,
    Operator::SimpleSelect,
    distanceOrthodromeTM );

/*
6.16.2 Operator ~point2string~

---- point [ x geoid ] --> string
----

*/

ListExpr point2stringTM(ListExpr args){
  int noargs = nl->ListLength(args);
  string errmsg = "Expected point [x geoid].";
  if( (noargs<1) || (noargs>2) ){
    return listutils::typeError(errmsg);
  }
  if(!listutils::isSymbol(nl->First(args),Point::BasicType())){
    return listutils::typeError(errmsg);
  }
  if( (noargs == 2) &&
      !listutils::isSymbol(nl->Second(args),Geoid::BasicType())){
    return listutils::typeError(errmsg);
  }
  return nl->SymbolAtom(CcString::BasicType());
}

int point2stringVM(Word* args, Word& result, int message,
                               Word& local, Supplier s){
  const Point* p = static_cast<const Point*>(args[0].addr);
  const Geoid* geoid =
            (qp->GetNoSons(s)==2)?static_cast<const Geoid*>(args[1].addr):0;
  result = qp->ResultStorage(s);
  CcString* res = static_cast<CcString*>(result.addr);

  if(!p->IsDefined() || (geoid && !geoid->IsDefined()) ){
    res->SetDefined(false);
  } else {
    res->Set(true,p->toString(geoid));
  }
  return 0;
}

const string point2stringSpec =
   "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
   "( <text> point [x geoid] -> string</text--->"
   "<text>point2string( P, [, Geoid] ) </text--->"
   "<text>Returns a textual representation of the point P. If Geoid is not "
   "provided, the result is a pair of euclidean coordinates, otherwise, it is "
   "a textual representation of geographic coordinates.</text--->"
   "<text>query point2string(makepoint(7.494968217,51.376125146), "
   "create_geoid(\"WGS1984\"))"
   "</text---> ) )";

Operator point2string (
    "point2string",
    point2stringSpec,
    point2stringVM,
    Operator::SimpleSelect,
    point2stringTM );

/*
6.16.2 Operator ~midpointBetween~

---- point x point [x geoid] [x real] --> point
----

If the optional real parameter is not present, a real default parameter with
value 0.5 is appended.

*/
ListExpr PointPointOptGeoidOptReal2PointTM(ListExpr args){
  int noargs = nl->ListLength(args);
  string errmsg = "Expected point x point [x geoid] [x real].";
  if( (noargs<2) || (noargs>4) ){
    return listutils::typeError(errmsg);
  }
  if(!listutils::isSymbol(nl->First(args),Point::BasicType())){
    return listutils::typeError(errmsg);
  }
  if(!listutils::isSymbol(nl->Second(args),Point::BasicType())){
    return listutils::typeError(errmsg);
  }
  if( noargs == 2 ) {
    return nl->ThreeElemList(nl->SymbolAtom(Symbol::APPEND()),
                                            nl->OneElemList(nl->RealAtom(0.5)),
                                            nl->SymbolAtom(Point::BasicType()));
  }
  if( noargs == 3 ){
    if( listutils::isSymbol(nl->Third(args),Geoid::BasicType())) {
      // special case: append default real parameter of 0.5
      return nl->ThreeElemList(nl->SymbolAtom(Symbol::APPEND()),
                               nl->OneElemList(nl->RealAtom(0.5)),
                               nl->SymbolAtom(Point::BasicType()));
    }
    if( listutils::isSymbol(nl->Third(args),CcReal::BasicType()) ){
      return nl->SymbolAtom(Point::BasicType());
    }
  }
  if( noargs == 4 ){
    if( listutils::isSymbol(nl->Third(args),Geoid::BasicType()) &&
        listutils::isSymbol(nl->Fourth(args),CcReal::BasicType())) {
      return nl->SymbolAtom(Point::BasicType());
    }
  }
  return listutils::typeError(errmsg);
}


int midpointBetweenVM(Word* args, Word& result, int message,
                   Word& local, Supplier s){
  int noargs = qp->GetNoSons(s);
  const Point* p1 = static_cast<const Point*>(args[0].addr);
  const Point* p2 = static_cast<const Point*>(args[1].addr);
  const Geoid* geoid = (noargs==4)?static_cast<const Geoid*>(args[2].addr):0;
  const CcReal* f = static_cast<const CcReal*>(args[noargs-1].addr);
  result = qp->ResultStorage(s);
  Point* res = static_cast<Point*>(result.addr);

  if(!p1->IsDefined() || !p2->IsDefined() || !f->IsDefined() ||
     (geoid && !geoid->IsDefined()) ){
    res->SetDefined(false);
    return 0;
  }
  if(geoid){
    if( !p1->checkGeographicCoord() || !p2->checkGeographicCoord() ||
        (AlmostEqual(p1->GetY()+p2->GetY(), 0.0) &&
         AlmostEqual(fabs(p1->GetX() - p2->GetX()), 180.0) ) ) {
      // points may not be antipodal! points must represent valid geo-coords!
      res->SetDefined(false);
      return 0;
    }
  }
  res->SetDefined(true);
  *res = p1->MidpointTo(*p2,f->GetValue(),geoid);
  return 0;
}

const string midpointBetweenSpec =
"( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
"( <text> point x point [x geoid] [x real] -> point</text--->"
"<text>midpointbetween( P1, P2 [, Geoid] [, RelDost] ) </text--->"
"<text>Returns the point with the rlative distance 0.0<=RelDist<=1.0 on the "
"shortest path from point P1 to point P2. If RelDist is not given, it defaults "
"to 0.5.If Geoid is passed, the midpoint is with respect to the orthodrome "
"P1->P2 (the points being geographic coordinates), otherwise the midpoint "
"in euclidean space. If a Geoid is passed, and P1, P2 are antipodal, the "
"result is UNDEF.</text--->"
"<text>query midpointBetween(makepoint(7.494968217,51.376125146), "
"makepoint(7.0,51.0), create_geoid(\"WGS1984\"), 0.7)</text---> ) )";

Operator spatialmidpointbetween (
  "midpointBetween",
  midpointBetweenSpec,
  midpointBetweenVM,
  Operator::SimpleSelect,
  PointPointOptGeoidOptReal2PointTM );

/*
6.16.3 Operators ~direction2heading~ and ~heading2direction~

---- real --> real
----

*/
ListExpr Real2RealTM(ListExpr args){
  int noargs = nl->ListLength(args);
  if (noargs!=1 || !listutils::isSymbol(nl->First(args),CcReal::BasicType()) ) {
    return listutils::typeError("Expected (real).");
  }
  return nl->SymbolAtom(CcReal::BasicType());
}

template<bool toDirection>
int ConvertDirHeadVM(Word* args, Word& result, int message,
                     Word& local, Supplier s){
  const CcReal* c = static_cast<const CcReal*>(args[0].addr);
  result = qp->ResultStorage(s);
  CcReal* res = static_cast<CcReal*>(result.addr);

  if(!c->IsDefined()){
    res->SetDefined(false);
  } else {
    double out = (toDirection)?headingToDirection(c->GetValue())
                              :directionToHeading(c->GetValue());
    res->Set(true, out );
  }
  return 0;
}

const string DirectionToHeadingSpec =
"( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
"( <text> real -> real</text--->"
"<text>direction2heading( D ) </text--->"
"<text>Converts a direction D (mathematical angle notation, counter-clockwise, "
"0<=d<360, 0 = directed as the positive X-half-axis) to the navigational "
"heading value (0<H<=360, clockwise, 360 = NORTH).</text--->"
"<text>query direction2heading(135.5)</text---> ) )";

const string HeadingToDirectionSpec =
"( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
"( <text> real -> real</text--->"
"<text>heading2direction( D ) </text--->"
"<text>Converts a navigational heading value (0<H<=360, clockwise, 360 = NORTH)"
"to a direction (mathematical angle notation, counter-clockwise, 0<=d<360, "
"0 = directed as the positive X-half-axis).</text--->"
"<text>query heading2direction(135.5)</text---> ) )";

Operator spatialDirectionToHeading (
"direction2heading",
 DirectionToHeadingSpec,
 ConvertDirHeadVM<false>,
 Operator::SimpleSelect,
 Real2RealTM
);

Operator spatialHeadingToDirection (
"heading2direction",
 HeadingToDirectionSpec,
 ConvertDirHeadVM<true>,
 Operator::SimpleSelect,
 Real2RealTM
);

/*
6.17 Operator ~spatialgetstartpoint~

---- sline --> point

Returns the start point of the simple line.

*/

ListExpr SLine2PointTM(ListExpr args){
  if(!nl->HasLength(args,1)){
    return listutils::typeError("Expected: sline.");
  }
  if(listutils::isSymbol(nl->First(args),SimpleLine::BasicType())){
    return nl->SymbolAtom(Point::BasicType());
  }
  return listutils::typeError("Expected: sline.");
}

template<bool start>
int SpatialGetPointVM(Word* args, Word& result, int message,
                           Word& local, Supplier s){
  SimpleLine* l = static_cast<SimpleLine*>(args[0].addr);
  result = qp->ResultStorage(s);
  Point* res = static_cast<Point*>(result.addr);
  if(!l->IsDefined()){
    res->SetDefined(false);
  } else {
    if (start)
      res->Set(l->StartPoint());
    else
      res->Set(l->EndPoint());
  }
  return 0;
}

const string SpatialGetStartPointSpec =
"( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
"( <text> sline -> point</text--->"
"<text>getstartpoint( _ ) </text--->"
"<text>Returns the starting point of a simple line.</text--->"
"<text>query getstartpoint(fromline(BGrenzenLinie))</text---> ) )";

const string SpatialGetEndPointSpec =
"( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
"( <text> sline -> point</text--->"
"<text>getendpoint( _ ) </text--->"
"<text>Returns the end point of a simple line.</text--->"
"<text>query getendpoint(fromline(BGrenzenLinie))</text---> ) )";

Operator spatialgetstartpoint (
  "getstartpoint",
  SpatialGetStartPointSpec,
  SpatialGetPointVM<true>,
  Operator::SimpleSelect,
  SLine2PointTM
);

Operator spatialgetendpoint (
  "getendpoint",
  SpatialGetEndPointSpec,
  SpatialGetPointVM<false>,
  Operator::SimpleSelect,
  SLine2PointTM
);
/*
10 Type Constructor ~geoid~

*/
 GenTC<Geoid> geoid_t;




/*
5.14 Operator ~circle~

5.14.1 Type Mapping for ~circle~

*/
ListExpr
circleTM( ListExpr args )
{
 string err = "point x real x int expected";
 if(!nl->HasLength(args,3)){
   return listutils::typeError(err);
 }
 if(!Point::checkType(nl->First(args)) ||
    !CcReal::checkType(nl->Second(args)) ||
    !CcInt::checkType(nl->Third(args))){
   return listutils::typeError(err);
 }
 return nl->SymbolAtom(Region::BasicType());
}

void generateCircle(Point* p, double radius, int n , Region* res){
  res->Clear();                // clear the result region
  if (!p->IsDefined() ) { // Nothing to do
      res->SetDefined( false );
      return;
  }
  double x, y;
  x = p->GetX();
  y = p->GetY();
  if(n<3 ||  n>100 || radius <=0){
    res->SetDefined(false);
    return;
  }

  double valueX, valueY;
  double angle;
  int partnerno = 0;
  HalfSegment hs;

  res->SetDefined( true );
  res->StartBulkLoad();

  //  Calculate a polygon with (n) vertices and (n) edges.
  //  To get the vertices, divide 360 degree in n parts using
  //  a standardised circle around p with circumference U = 2 * PI * r.

  for( int i = 0; i < n; i++ ) {
      // The first point/vertex of the segment
      angle = i * 2 * M_PI/n; // angle to starting vertex
      valueX = x + radius * cos(angle);
      valueY = y + radius * sin(angle);
      Point v1(true, valueX ,valueY);

      // The second point/vertex of the segment
      if ((i+1) >= n){            // angle to end vertex
        angle = 0 * 2 * M_PI/n;    // for inner vertex
      } else {
        angle = (i+1) * 2 * M_PI/n;// for ending = starting vertex
      }
      valueX = x + radius * cos(angle);
      valueY = y + radius * sin(angle);
      Point v2(true, valueX ,valueY);

      // Create a halfsegment for this segment
      hs.Set(true, v1, v2);
      hs.attr.faceno = 0;         // only one face
      hs.attr.cycleno = 0;        // only one cycle
      hs.attr.edgeno = partnerno;
      hs.attr.partnerno = partnerno++;
      hs.attr.insideAbove = (hs.GetLeftPoint() == v1);

      // Add halfsegments 2 times with opposite LeftDomPoints
      *res += hs;
      hs.SetLeftDomPoint( !hs.IsLeftDomPoint() );
      *res += hs;
  }
  res->EndBulkLoad();
}


/*
5.14.2 Value Mapping for ~circle~

*/
int circleVM( Word* args, Word& result, int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  Point* p = (Point*)args[0].addr; // Centre of the circle
  CcReal* r = (CcReal*)args[1].addr; // Radius of the circle.
  CcInt* narg = (CcInt*)args[2].addr; // number of edges
  Region *res = (Region*)result.addr;

  if(!p->IsDefined() || !r->IsDefined() || !narg->IsDefined()){
    res->SetDefined(false);
  }  else {
    generateCircle(p, r->GetValue(), narg->GetValue(), res);
  }

  return 0;
}

/*
5.14.3 Specification for operator ~circle~

*/
const string
circleSpec =
"( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
"( <text>(point real int) -> region</text--->"
"<text>circle ( p, r, n ) </text--->"
"<text>Creates a region with a shape approximating a circle "
"with a given a given center point p and radius r>0.0 by a "
"regular polygon with 2<n<101 edges.\n"
"Parameters out of the given perimeters result in an "
"empty region, undef values in an undef one .</text--->"
"<text>circle (p,10.0,10)</text---> ) )";

/*
5.14.4 Selection Function of operator ~circle~

Not necessary.

*/

/*
5.14.5  Definition of operator ~circle~

*/
Operator spatialcircle( "circle",
                  circleSpec,
                  circleVM,
                  Operator::SimpleSelect,
                  circleTM);

/*
1.1 Operator ~longlines~

The operator gets a stream of ~sline~ values and returns a stream of ~sline~
values, where the input values are connected to as long as possible line parts.

1.1.1 TypeMapping

*/

ListExpr SpatialLongLinesTM(ListExpr args){
  string err = "line expected";
  if(!nl->HasLength(args,1)){
     return listutils::typeError(err);
  }

  if(!nl->IsEqual(nl->First(args),Line::BasicType())) {
    return listutils::typeError("Argument of type " + Line::BasicType() +
                                " expected.");
  }
  return nl->TwoElemList(nl->SymbolAtom(Symbol::STREAM()),
                         nl->SymbolAtom(SimpleLine::BasicType()));
}

/*
1.1.1 Value Mapping

1.1.1.1 class LongLinesLocalInfo

Stores informations of current situation between request calls. And delivers
result of request using ~GetNextResult~ function.

*/


class LongLinesLocalInfo{

public:

/*
Constructor initializes values

*/
LongLinesLocalInfo(Line* li) : firstPoint(false, 0.0,0.0){
    assert(li->IsDefined() && !li->IsEmpty());
    this->theLine = li;
    size = li->Size();
    used = new vector<bool>(size, false);
    pointset = new set<Point>();
    pointset->clear();
    intermediate = new vector<SimpleLine> ();
    intermediate->clear();
  };

/*
Deconstructor deletes internal array

*/
  ~LongLinesLocalInfo(){
    pointset->clear();
    delete pointset;
    used->clear();
    delete used;
    intermediate->clear();
    delete intermediate;
  };

/*
Splits the ~Line~ into ~SimpleLine~s which are each long as possible.

*/
  SimpleLine* GetNextResult(){
    // Find valid start Segment of a line (part)
    if (!intermediate->empty()){
      SimpleLine* res = new SimpleLine(intermediate->back());
      intermediate->pop_back();
      return res;
    }
    int curPos = 0;
    while (used->at(curPos) && curPos < size-1){
      curPos++;
    }
    if (used->at(curPos)) {
      return 0;
    }
    HalfSegment curHS, test;
    theLine->Get(curPos, curHS);
    Point curStartPoint = curHS.GetDomPoint();
    if (curPos + 1 < size && !used->at(curPos+1)) {
      theLine->Get(curPos+1,test);
      if (test.GetDomPoint() != curStartPoint) {
        return GetLongestFrom(curPos);
      }
    }
    Point curEndPoint = curHS.GetSecPoint();
    int partnerpos = curHS.GetAttr().partnerno;
    int testPos = partnerpos;
    bool done = false;
    vector<bool>* visited = new vector<bool>(*used);
    visited->at(curPos) = true;
    visited->at(partnerpos) = true;
    while (!done){
      if (partnerpos > 0) testPos = partnerpos -1;
      theLine->Get(testPos,test);
      if (!visited->at(testPos) &&
          AlmostEqual(test.GetDomPoint(),curEndPoint)){
        curPos = testPos;
        curEndPoint = test.GetSecPoint();
        partnerpos = test.GetAttr().partnerno;
        visited->at(curPos) = true;
        visited->at(partnerpos) = true;
      } else {
        if (partnerpos < size - 1) testPos = partnerpos + 1;
        theLine->Get(testPos,test);
        if (!visited->at(testPos) &&
            AlmostEqual(test.GetDomPoint(),curEndPoint)){
          curPos = testPos;
          curEndPoint = test.GetSecPoint();
          partnerpos = test.GetAttr().partnerno;
          visited->at(curPos) = true;
          visited->at(partnerpos) = true;
        } else {
          done = true;
        }
      }
    }
    visited->clear();
    delete visited;
    return GetLongestFrom(partnerpos);
  };

private:

  SimpleLine* SearchPartner(vector<double>* lengths,
                            vector<SimpleLine>* foLines){
    SimpleLine* res = 0;
    int pos1 = -1;
    int pos2 = -1;
    double maxLength = 0.0;
    do{
      maxLength = 0.0;
      for (unsigned int i = 0 ; i < lengths->size()-1; i++){
        for (unsigned int j = i+1; j < lengths->size(); j++){
          double curLength = lengths->at(i)+lengths->at(j);
          if (curLength > maxLength){
            maxLength = curLength;
            pos1 = i;
            pos2 = j;
          }
        }
      }
      if (maxLength > 0.0){
        if (pos1 == 0) {
          if (res == 0 && lengths->at(pos2) > 0.0){
            res = new SimpleLine(foLines->at(pos2));
            lengths->at(pos1) = 0.0;
            lengths->at(pos2) = 0.0;
          } else {
            if (res != 0 && lengths->at(pos2) > 0.0){
              intermediate->push_back(foLines->at(pos2));
              lengths->at(pos1) = 0.0;
              lengths->at(pos2) = 0.0;
            } else {
              lengths->at(pos1) = 0.0;
            }
          }
        } else {
          SimpleLine l1 = foLines->at(pos1);
          SimpleLine l2 = foLines->at(pos2);
          int edgeno = 0;
          SimpleLine* inres = new SimpleLine(true);
          inres->Clear();
          inres->StartBulkLoad();
          HalfSegment hs;
          if (lengths->at(pos1) > 0.0){
            for (int i = 0 ; i < l1.Size(); i++) {
              l1.Get(i,hs);
              AttrType attr = hs.GetAttr();
              attr.edgeno = edgeno;
              hs.SetAttr(attr);
              inres->operator+=(hs);
              if (hs.IsLeftDomPoint()) hs.SetLeftDomPoint(false);
              else hs.SetLeftDomPoint(true);
              inres->operator+=(hs);
              edgeno++;
            }
          }
          if (lengths->at(pos2) > 0.0){
            for (int i = 0 ; i < l2.Size(); i++) {
              l2.Get(i,hs);
              AttrType attr = hs.GetAttr();
              attr.edgeno = edgeno;
              hs.SetAttr(attr);
              inres->operator+=(hs);
              if (hs.IsLeftDomPoint()) hs.SetLeftDomPoint(false);
              else hs.SetLeftDomPoint(true);
              inres->operator+=(hs);
              edgeno++;
            }
          }
          inres->EndBulkLoad();
          intermediate->push_back(*inres);
          inres->Clear();
          delete inres;
          lengths->at(pos1) = 0.0;
          lengths->at(pos2) = 0.0;
        }
        pos1 = -1;
        pos2 = -1;
      }
    } while (maxLength > 0.0);
    return res;
  };

  void AddHalfSegmentToResult(SimpleLine* result, int index,
                              Point& curEndPoint, Point& firstPoint,
                              double& length, int& partnerpos,
                              bool& done, int& edgeno, bool first){
    HalfSegment curHs;
    theLine->Get(index, curHs);
    AttrType attr = curHs.GetAttr();
    curEndPoint = curHs.GetSecPoint();
    partnerpos = attr.partnerno;
    attr.edgeno = edgeno;
    curHs.SetAttr(attr);
    if (pointset->find(curEndPoint) != pointset->end() && !first) {
      done = true;
      if (AlmostEqual(curEndPoint,firstPoint)) {
        used->at(index) = true;
        used->at(partnerpos) = true;
        length += curHs.Length();
        result->operator+=(curHs);
        pointset->insert(curEndPoint);
        if (curHs.IsLeftDomPoint()) curHs.SetLeftDomPoint(false);
        else curHs.SetLeftDomPoint(true);
        result->operator+=(curHs);
        edgeno++;
      }
    } else {
      used->at(index) = true;
      used->at(partnerpos) = true;
      result->operator+=(curHs);
      pointset->insert(curEndPoint);
      if (curHs.IsLeftDomPoint()) curHs.SetLeftDomPoint(false);
      else curHs.SetLeftDomPoint(true);
      result->operator+=(curHs);
      length += curHs.Length();
      edgeno++;
    }
  };

  SimpleLine* GetLongestFrom(int indStart){
    // Initalize result computation
    SimpleLine* result = new SimpleLine(true);
    result->Clear();
    result->StartBulkLoad();
    double length = 0.0;
    HalfSegment curHs;
    theLine->Get(indStart, curHs);
    Point curStartPoint = curHs.GetDomPoint();
    firstPoint = curStartPoint;
    Point curEndPoint = curHs.GetSecPoint();
    pointset->insert(curStartPoint);
    int partnerpos = curHs.GetAttr().partnerno;
    bool done = false;
    int edgeno = 0;
    AddHalfSegmentToResult(result, indStart, curEndPoint, firstPoint,
                           length, partnerpos, done, edgeno, true);
    // The dominating point of the next segment in the line is equal to the
    // endpoint of the currentSegment. The next halfsegment therefore is stored
    // near partnerpos in ~line~.
    // Start search for HalfSegments starting in curEndPoint at left of
    // partnerpos
    while (!done){
      int firstPos = partnerpos;
      int lastPos = partnerpos;
      int countLeft = 0;
      int countRight = 0;
      int curPos = partnerpos -1;
      HalfSegment test;
      if (curPos >= 0 && curPos < size)
      {
        theLine->Get(curPos,test);
        while (AlmostEqual(test.GetDomPoint(),curEndPoint) && curPos >= 0){
          if (!used->at(curPos)) countLeft++;
          curPos--;
          if (curPos >= 0)theLine->Get(curPos, test);
        }
      }
      if (countLeft > 0) firstPos = curPos + 1;
      if (firstPos < 0) firstPos = 0;
      // Continue search at right of partnerpos
      curPos = partnerpos + 1;
      if (curPos >= 0 && curPos < size) {
        theLine->Get(curPos,test);
        while (AlmostEqual(test.GetDomPoint(), curEndPoint) && curPos < size){
          if (!used->at(curPos)) countRight++;
          curPos++;
          if (curPos < size)theLine->Get(curPos,test);
        }
      }

      if (countRight > 0) lastPos = curPos - 1;
      if (lastPos >= size) lastPos = size-1;
      int count = countLeft + countRight;
      if (count == 1) {
        int i = firstPos;
        bool ok = false;
        while (!ok && i <= lastPos){
          if(i != partnerpos && !used->at(i)){
            AddHalfSegmentToResult(result, i, curEndPoint,firstPoint,
                                   length, partnerpos, done, edgeno, false);
            ok = true;
          }
          i++;
        }
      } else {
        if (count > 1) {
          vector<double>* followLength = new vector<double>();
          vector<int>* followPos = new vector<int>();
          vector<SimpleLine>* followLine = new vector<SimpleLine>();
          followLength->push_back(length);
          followPos->push_back(-1);
          followLine->push_back(SimpleLine(false));
          int helpCount = 1;
          for (int i = firstPos; i <= lastPos; i++){
            if (i != partnerpos && !used->at(i)){
              followPos->push_back(i);
              helpCount++;
              used->at(i) = true;
              theLine->Get(i,test);
              int partnerpos = test.GetAttr().partnerno;
              if (partnerpos >= 0 && partnerpos < size)
                used->at(partnerpos) = true;
            }
          }
          for (int i = 1; i < helpCount; i++){
            SimpleLine* helpLine = GetLongestFrom(followPos->at(i));
            followLine->push_back(*helpLine);
            followLength->push_back(helpLine->Length());
            delete helpLine;
          }
          SimpleLine* ind = SearchPartner(followLength,followLine);
          if (ind == 0) {
            done = true;
          } else {
            for (int i = 0; i < ind->Size(); i++){
              ind->Get(i,test);
              AttrType attr = test.GetAttr();
              attr.edgeno = edgeno;
              test.SetAttr(attr);
              result->operator+=(test);
              if (test.IsLeftDomPoint()) test.SetLeftDomPoint(false);
              else test.SetLeftDomPoint(true);
              result->operator+=(test);
              edgeno++;
            }
            delete ind;
            ind = 0;
            done = true;
          }
          followLength->clear();
          delete followLength;
          followPos->clear();
          delete followPos;
          followLine->clear();
          delete followLine;
        } else { // count < 1 ,
          done = true;
        }
      }
    }
    //cleanup and return result
    result->EndBulkLoad();
    return result;
  };

/*
First point of current computed line for check of valid cycle

*/
  Point firstPoint;

/*
Pointer to a boolean array telling if a HalfSegement has been used or not.

*/
  vector<bool>* used;

/*
Source line

*/
  Line* theLine;

/*
Number of HalfSegments of source line.

*/
  int size;

/*
Already touched line points.

*/

  set<Point>* pointset;

/*
Intermediate results

*/

 vector<SimpleLine>* intermediate;
};

int SpatialLongLinesVM(Word* args, Word& result, int message, Word& local,
                       Supplier s ){

  switch(message){

    case OPEN:{

      Line* li = static_cast<Line*> (args[0].addr);
      if (!li->IsDefined() || li->IsEmpty()) {
        local.setAddr(0);
      } else {
        LongLinesLocalInfo* localinfo = new LongLinesLocalInfo(li);
        local.setAddr(localinfo);
      }
      return 0;
    }

    case REQUEST: {
      result = qp->ResultStorage(s);
      if(!local.addr) return CANCEL;
      LongLinesLocalInfo* localinfo = (LongLinesLocalInfo*) local.addr;
      SimpleLine* res = localinfo->GetNextResult();

      if(res == 0 || !res->IsDefined()){
        result.setAddr(0);
        return CANCEL;
      } else {
        result.setAddr(res);
        return YIELD;
      }
    }

    case CLOSE: {

      if(local.addr != 0){
        LongLinesLocalInfo* localinfo = (LongLinesLocalInfo*) local.addr;
        delete localinfo;
        local.setAddr(0);
      }
      return 0;
    }

  }

  return 0;
}


/*
1.1.1 Specification

*/

const string SpatialLongLinesSpec =
"( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
"( <text>line -> (stream sline)</text--->"
"<text> _ longlines </text--->"
"<text>Converts a line into an stream of longest possible sline "
"values.</text--->"
"<text> _ longlines </text---> ) )";

/*
1.1.1 Operatordefiniton

*/

Operator spatiallonglines(
  "longlines",
  SpatialLongLinesSpec,
  SpatialLongLinesVM,
  Operator::SimpleSelect,
  SpatialLongLinesTM
);

/*
1.1 Operator ~splitslineatpoints~

The operator gets an ~sline~ value and an ~points~ value defining a set of
split points. The operator splits the input value at the given points and
returns the result as an stream of ~sline~ values.

1.1.1 TypeMapping

*/

ListExpr SplitSLineAtPointsTM(ListExpr args){

  if(!nl->HasLength(args,2)){
    return listutils::typeError("two arguments expected");
  }

  if(!nl->IsEqual(nl->First(args),SimpleLine::BasicType()))
    return listutils::typeError("First argument must be an " +
                                  SimpleLine::BasicType());

  if (!nl->IsEqual(nl->Second(args), Points::BasicType()))
    return listutils::typeError("Second Argument must be an " +
                                 Points::BasicType());

  return nl->TwoElemList(nl->SymbolAtom(Symbol::STREAM()),
                         nl->SymbolAtom(SimpleLine::BasicType()));
}

/*
1.1.1 Value Mapping

1.1.1.1 class SplitSLineAtPointsLocalInfo

Stores informations of current situation between request calls. And delivers
result of request using ~GetNextResult~ function.

*/


struct SplitSLineAtPointsLocalInfo{

  SplitSLineAtPointsLocalInfo(SimpleLine* li, Points* pts)
  {
    assert(li->IsDefined() && !li->IsEmpty() &&
           pts->IsDefined());

    if (pts->IsEmpty())
    {
      intermediate = new vector<SimpleLine> ();
      intermediate->clear();
      intermediate->push_back(*li);
    }
    else
    {
      intermediate = li->SplitAtPPoints(pts);
    }
  };

  ~SplitSLineAtPointsLocalInfo(){
    intermediate->clear();
    delete intermediate;
  };

  SimpleLine* GetNextResult(){
    if (!intermediate->empty())
    {
      SimpleLine* res = new SimpleLine(intermediate->back());
      intermediate->pop_back();
      return res;
    } else
      return 0;
  };

vector<SimpleLine>* intermediate;


};

int SplitSLineAtPointsVM(Word* args, Word& result, int message, Word& local,
                         Supplier s )
{
  switch(message){

    case OPEN:{

      SimpleLine* li = static_cast<SimpleLine*> (args[0].addr);
      Points* pts = static_cast<Points*> (args[1].addr);
      if (!li->IsDefined() || li->IsEmpty() || !pts->IsDefined()) {
        local.setAddr(0);
      } else {
        SplitSLineAtPointsLocalInfo* localinfo =
          new SplitSLineAtPointsLocalInfo(li,pts);
        local.setAddr(localinfo);
      }
      return 0;
    }

    case REQUEST: {
      result = qp->ResultStorage(s);
      if(!local.addr) return CANCEL;
      SplitSLineAtPointsLocalInfo* localinfo =
        (SplitSLineAtPointsLocalInfo*) local.addr;
      SimpleLine* res = localinfo->GetNextResult();
      if(res == 0 || !res->IsDefined()){
        result.setAddr(0);
        return CANCEL;
      } else {
        result.setAddr(res);
        return YIELD;
      }
    }

    case CLOSE: {
      if(local.addr != 0)
      {
        SplitSLineAtPointsLocalInfo* localinfo =
          (SplitSLineAtPointsLocalInfo*) local.addr;
        delete localinfo;
        local.setAddr(0);
      }
      return 0;
    }

  }

  return 0;
}

/*
1.1.1 Specification

*/
 const string SplitSLineAtPointsSpec =
 "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
 "( <text>sline X points -> (stream sline)</text--->"
 "<text> splitslineatpoints (_, _) </text--->"
 "<text>Splits the sline value into sublines defined by the points.</text--->"
 "<text> splitslineatpoints(sline, points) </text---> ) )";

/*
1.1.1 Operatordefiniton

*/

 Operator spatialsplitslineatpoints(
   "splitslineatpoints",
   SplitSLineAtPointsSpec,
   SplitSLineAtPointsVM,
   Operator::SimpleSelect,
   SplitSLineAtPointsTM);



/*
10.38 ~findCycles~

This operator finds cycles in a line value. The returned cycles build a
partition of the space divided by the line. Each halfsegment is used
once, if it belongs to the outer cycle of the line, twice, if it separates
the inner region or is not used, if it leads to a dead end or connects a "hole"
with another cycle.

10.38.1 Type Mapping

The Signature is: line [->] stream(line)

*/
ListExpr findCyclesTM(ListExpr args){
 string err = "line [x bool]  expected";
 int len = nl->ListLength(args);
 if((len != 2) && (len!=1)){
    return listutils::typeError(err);
 }
 if(!Line::checkType(nl->First(args))){
    return listutils::typeError(err);
 }
 if(len==2){
   if(!CcBool::checkType(nl->Second(args))){
       return listutils::typeError(err);
   }
 }
 return nl->TwoElemList( listutils::basicSymbol<Stream<Line> >(),
                         listutils::basicSymbol<Line>());

}

/*
10.38.2 LocalInfo

*/
class FindCyclesInfo{
  public:

/*
~Constructor~

*/
     FindCyclesInfo(Line* _line, const bool extractH):line(_line), globalPos(0){
       extractHoles = extractH;
       assert(line->IsDefined());
       max = line->Size();
       usage = new char[max];
       memset(usage,0,max); // mark all halfssegments to be unused
       isCritical = new char[max];
       memset(isCritical,0,max);
       DbArray<HalfSegment>* hss = (DbArray<HalfSegment>*) line->GetFLOB(0);
       RegionCreator<DbArray >::findCritical(hss,isCritical);
     }

/*
~Destructor~

*/
     ~FindCyclesInfo(){
         delete[] usage;
         delete[] isCritical;
      }


/*
~nextLine~

Returns the next cycles within this line or 0
if no further cycle can be found.

*/

     Line* nextLine(){
         if(extractHoles){
            return nextLineExtractHoles();
         } else {
            return nextLineSimpleCycles();
         }
     }

     Line* nextLineSimpleCycles(){
        Line* res = 0;
        if(!cycles.empty()){
           res = constructLine(cycles.back());
           cycles.pop_back();
           return res;
        }

        while(globalPos < max){
           computeCycles(globalPos);
           if(!cycles.empty()){
              res = constructLine(cycles.back());
              cycles.pop_back();
              return res;
           }  else {
              usage[globalPos] = 1;
              globalPos++;
           }
        }
        return 0;
     }



     Line* nextLineExtractHoles(){ // extended version including holes detection
        Line* res = 0;
        if(!cycles.empty()){ // cycles are computed
            if(globalPos < (int)cycles.size()){
               res = constructLine(cycles[globalPos]);
               globalPos++;
               return res;
            }
            return 0;
        }

        // compute cycles
        // step 1: compute all simple cycles
        int pos = 0;
        while(pos < max){
           computeCycles(pos);
           pos++;
        }

        if(cycles.empty()){
          return 0;
        }
        if(cycles.size() < 2){ // no hole possible
           res = constructLine(cycles[0]);
           globalPos = 1;
           return res;
        }


        // step 3 store Bounding boxes for each cycle ( acceleration of step 3)
        vector<Rectangle<2> > boxes;
        for(size_t i=0; i < cycles.size();i++){
            boxes.push_back(computeBox(cycles[i]));
        }

        vector<int> outers;

        for(size_t c1 = 0; c1<cycles.size();c1++){
            int outer = -1;
            double dist = 0;
            for(size_t c2 = 0; c2<cycles.size(); c2++){
                if(c1!=c2){
                  if(boxes[c2].Contains(boxes[c1])){
                     pair<bool,double> r = isHole(cycles[c1], cycles[c2]);
                     if(r.first){
                        if((outer < 0) || (dist > r.second)){
                            outer = c2;
                            dist = r.second;
                        }
                     }
                  } else {
                  }
                }
            }
            outers.push_back(outer);
        }


        // step 4 append hole cycles to the outer cycles
        // it may be, that halfsegments belongs to more than one hole
        // such halfsegments should not be inserted into a result lineA

        memset(usage,0,max);
        for(size_t i=0;i<cycles.size();i++){
            if(outers[i]>=0){
               append(cycles[outers[i]],cycles[i],usage,1);
            }
        }
        res = constructLine(cycles[0]);
        globalPos = 1;
        return res;
     }

     static bool odd(const int i){
        int mask = 1;
        return (i & mask);
     }


  private:
    Line* line;
    int globalPos;
    int max;
    char* usage;
    char* isCritical;
    vector<int> currentPath;
    vector<vector<int> > cycles;
    bool extractHoles;


   void printCycle(const vector<int>& cycle){
      vector<int>::const_iterator it;
      for(it=cycle.begin();it!=cycle.end();it++){
         HalfSegment hs;
         line->Get(*it,hs);
         cout << *it << " : " << hs.SimpleString() << endl;
      }
   }

   void printCycleAsLine(const vector<int>& cycle, ostream& o = cout){
      vector<int>::const_iterator it;
      o << setprecision(16);
      o << "(line (";
      for(it=cycle.begin();it!=cycle.end();it++){
         HalfSegment hs;
         line->Get(*it,hs);
         o << "(" << hs.GetDomPoint().GetX() << " " << hs.GetDomPoint().GetY()
           << " " << hs.GetSecPoint().GetX() << " " << hs.GetSecPoint().GetY()
           << ")";
      }
      o << "))";
   }

   void printAsRel(const vector<int>& cycle, ostream& o = cout){
       o << setprecision(16);
       o << "( (rel(tuple((No int)(Partner int) (Seg line)))) (" << endl;
      vector<int>::const_iterator it;
      HalfSegment hs;
      for(it=cycle.begin();it!=cycle.end();it++){
        line->Get(*it,hs);
        o << "(" << *it << " " << hs.attr.partnerno << " "
          << "((" << hs.GetDomPoint().GetX() << " " << hs.GetDomPoint().GetY()
          << " " << hs.GetSecPoint().GetX() << " " << hs.GetSecPoint().GetY()
          << ")) )" << endl;
      }
      o << "))";

   }



   // axuxiliary function (for debugging
   // returns 0 if (x,y) is outside the cycle
   // returns 2 if (x,y) is on the cycle
   // returns 1 if (x,y) is inside the cycle
   int cycleContains(const vector<int>& cycle, const double& x,
                     const double& y) const{
      int hits = 0;
      HalfSegment hs;
      for(size_t i=0;i<cycle.size();i++){
         line->Get(cycle[i],hs);
         if(hs.Contains(Point(true,x,y))){
            return 2; //
         }
         double dist = RegionCreator<DbArray >::getLeftDist(hs,x,y,true);
         if(dist>=0){
            hits++;
         }
      }
      int mask = 1;
      return (hits&mask) > 0?1:0;
   }


   pair<bool, double> isHole(const vector<int>& hole, const vector<int>& outer){


      HalfSegment hs;
      int hi = hole[0];
      line->Get(hi,hs);
      Point dp = hs.GetDomPoint();
      Point sp = hs.GetSecPoint();
      double x = (dp.GetX() + sp.GetX()) /2;
      double y = (dp.GetY() + sp.GetY()) /2;
      int count = 0;
      double dist = 0;

      for(size_t i=0;i<outer.size();i++){
         int ho = outer[i];
         if(ho == hi){
            return pair<bool,double>(false,0);
         }
         line->Get(ho, hs);
         if(hs.attr.partnerno==hi) {
            return pair<bool,double>(false,0);
         }
         double dist1 = RegionCreator<DbArray >::getLeftDist(hs,x,y, true);

         if(dist1>=0){
            count++;
            if((count==1) || (dist1<dist)){
               dist = dist1;
            }
         }
      }
      if(odd(count)){
         return pair<bool,double>(true,dist);
      } else {
         return pair<bool,double>(false,0);
      }

   }


   int find(const vector<int>& v, int value){
     for(size_t i=0;i<v.size();i++){
        if(v[i]==value){
          return i;
        }
     }
     return -1;
   }

   void erase(vector<int>& v, int value){
     int pos = find(v,value);
     if(pos>=0){
       v[pos] = v.back();
       v.pop_back();
     }
   }

   void append(vector<int>& v1,  vector<int>& v2, char* usage, char Umark){
      // v1 is the outer
      // v2 is some hole
      // usage is 0 => unused
      // usage == Umark => halfsegment is already used

      HalfSegment hs;
      for(size_t i=0;i<v2.size();i++){
         int n = v2[i];
         line->Get(n,hs);
         int p = hs.attr.partnerno;
         if( (usage[n]==Umark) || (usage[p]==Umark)){
             erase(v1,n);
             erase(v1,p);
         } else {
             usage[n] = Umark;
             usage[p] = Umark;
             v1.push_back(n);
         }
      }
   }

   Rectangle<2> computeBox(const vector<int>& cycle){
      Rectangle<2> res(false);
      HalfSegment hs;
      Point p(true);
      for(size_t i=0;i<cycle.size();i++){
         line->Get(cycle[i],hs);
         p = hs.GetDomPoint();
         if(i==0){
            res = p.BoundingBox();
         } else {
            res.Extend(p.BoundingBox());
         }
      }
      return res;
   }


/*
~findCycle~

Tries to find a cycle starting at the dominating point of the Halfsegment
at position pos. Note, the cycle can also start with an halfsegment at
another location (having the same dominating point).  It's assumed the
dominating point is the left most one within this cycle.

Only non-used halfsegments are used for finding the cycle.
For dead ends, all halfssegments are set to be used. This avoids the
inspection of dead ends more than once.

*/
  Line* findCycle(int pos){
     int pos1 = findStartPos(pos);
     if(pos1 < 0){ // no start found
        usage[pos] = 1;
        return 0;
     }
     pos = pos1;
     usage[pos] = 3;
     HalfSegment hs;
     line->Get(pos,hs);
     if(usage[hs.attr.partnerno]==0){
        usage[hs.attr.partnerno]=4;
     }
     currentPath.push_back(pos);
     Line* line =  findCycle(currentPath);
     return line;
 }

   void computeCycles( int pos ){
      int pos1 = findStartPos(pos);
      if(pos1 < 0){
         return;  // keep cycles empty
      }
      pos = pos1;
      usage[pos] = 3;
      HalfSegment hs;
      line->Get(pos,hs);
      if(usage[hs.attr.partnerno]==0){
         usage[hs.attr.partnerno] = 4;
      }
      vector<int> path;
      path.push_back(pos);
      computeCycles(path);
   }

   void computeCycles(vector<int>& path){
      HalfSegment hs;
      while(!path.empty()){
         int pos = path.back();
         int next = nextPos(pos);
         if(next<0){ // no extension found
             reducePath(path);
         } else {
           if(usage[next]==3) { // found cycle
             vector<int> cycle;
             int s = path.back();
             while(s!=next){
                path.pop_back();
                cycle.push_back(s);
                usage[s] = 1;
                line->Get(s,hs);
                if(usage[hs.attr.partnerno]==4){
                   usage[hs.attr.partnerno] = 0;
                }
                s = path.back();
             }
             // process last element in path
             path.pop_back();
             cycle.push_back(s);
             usage[s] = 1;
             line->Get(s,hs);
             if(usage[hs.attr.partnerno]==4){
                usage[hs.attr.partnerno] = 0;
             }
             if(isClockwise(cycle)){
                cycles.push_back(cycle);
             }
           } else { // normal extension
              usage[next] = 3;
              line->Get(next,hs);
              if(!usage[hs.attr.partnerno]){
                 usage[hs.attr.partnerno] = 4;
              }
              path.push_back(next);
           }
         }
      }
   }

   bool isClockwise(vector<int>& path){
      assert(!path.empty());
      // find the smallest dom point in path
      int index = 0;
      HalfSegment hs;
      line->Get(path[0],hs);
      Point dp = hs.GetDomPoint();
      for(size_t i=1;i<path.size();i++){
          line->Get(path[i],hs);
          Point dp1 = hs.GetDomPoint();
          if(dp1<dp){
             index = i;
             dp = dp1;
          }
      }

      line->Get(path[index],hs);
      Point p2 = hs.GetDomPoint();
      Point p3 = hs.GetSecPoint();
      line->Get(path[ ( index +1 )%path.size()],hs);
      Point p1 = hs.GetDomPoint();
      return Region::GetCycleDirection(p1,p2,p3);
   }





 void print(const vector<int>& v)const{
   cout << "<";
   for(size_t i =0;i<v.size();i++){
     if(i>0) cout << ", ";
     cout << v[i];
   }
   cout << ">";
 }

 Line* findCycle(vector<int>& currentPath){
    int pos = currentPath.back();
    //cout << "start a new path with " << pos << endl;
    int next =  nextPos(pos);
    vector<int> resPath; // path to construct
    vector<int> cycleStarts; // stores the positions of subcycles
    while(!currentPath.empty()){
      while((next>=0) && !currentPath.empty()){
        if(usage[next] == 3){ // found sub path
          cycleStarts.push_back(resPath.size());
          while(currentPath.back() != next){
              int last = currentPath.back();
              resPath.push_back(last);
              currentPath.pop_back();
              usage[last] = 1;
              //cout << "put " << last << " from path into result" << endl;
          }
         // process first elem of path
         int last = currentPath.back();
         resPath.push_back(last);
         currentPath.pop_back();
         usage[last] = 1;
         //cout << "put " << last << " from path into result" << endl;
         if(!currentPath.empty()){
            pos = currentPath.back();
            next = nextPos(pos);
         }
      } else { // normal extension
         usage[next] = 3;
         HalfSegment hs;
         line->Get(next,hs);
         if(usage[hs.attr.partnerno]==0){
            usage[hs.attr.partnerno]= 4;
         }
         currentPath.push_back(next);
         //cout << "extend path " << next << endl;
         pos = next;
         next = nextPos(pos);
      }
     }
     if(!currentPath.empty()){
       reducePath(currentPath);
       if(!currentPath.empty()){
           pos = currentPath.back();
           next = nextPos(pos);
       }
     }
    } // currentPath not emptyA


    if(resPath.empty()){
       return 0;
    } else {
      HalfSegment hs;
      // mark the locked halfsegments as usuable
      // find out the index of the leftmost point within the path
      vector<int>::iterator it;
      size_t outerPathStarts = cycleStarts.back();
      int smallestIndex = cycleStarts.back();
      Point smallestDomPoint(true);
      for(size_t i=0;i<resPath.size();i++){
         line->Get(resPath[i],hs);
          if(usage[hs.attr.partnerno]==4){
            usage[hs.attr.partnerno] = 0;
          }
          if(i==outerPathStarts){
            smallestDomPoint = hs.GetDomPoint();
          } else if(i>outerPathStarts){
            Point p = hs.GetDomPoint();
            if(smallestDomPoint.GetX() > p.GetX()){
              smallestIndex = i;
              smallestDomPoint = p;
            }
          }
      }

      line->Get( resPath[smallestIndex],hs);
      Point p2 = hs.GetDomPoint();
      Point p3 = hs.GetSecPoint();
      size_t i3 = smallestIndex +1;
      if(i3==resPath.size()){
         i3 = outerPathStarts;
      }
      line->Get( resPath[i3],hs);
      Point p1 = hs.GetDomPoint();

      if(!Region::GetCycleDirection(p1,p2,p3)){
         //cout << "ignore Path of length " << resPath.size() << endl;
         return 0;
      } else {
         //cout << "path directed clockwise" << endl;
         //cout << "Path has " << resPath.size() << " edges" << endl;
         return constructLine(resPath);
      }
    }

  }


  void reducePath(vector<int>& path){
     //size_t oldsize = path.size();
     if(path.empty()){
        return;
     }
     int last = path.back();
     path.pop_back();
     usage[last] = 1;
     HalfSegment hs;
     line->Get(last,hs);
     if(usage[hs.attr.partnerno]==4){
        usage[hs.attr.partnerno] = 0;
     }
     //cout << "remove " << last << " from path " << endl;
     while(!path.empty()){
        if(isCritical[last]){ // possible a new extension
            return;
        } else {
           //cout << last << " is not critical  remove next "<< endl;
        }

        last = path.back();
        path.pop_back();
        usage[last] = 1;
        line->Get(last,hs);
        if(usage[hs.attr.partnerno]==4){
           usage[hs.attr.partnerno] = 0;
        }
        //cout << "remove " << last << " from path " << endl;
     }
  }




/*
Returns the position of the halfsegment having the same domination point as the
halfsegment at pos with the highest slope.
If only one unused halfsegment is available, -1 is returned. This can be the
case if 1) a dead end  2) only the halfsegment with the smallest slope is
available (this halfsegment cannot contribute to a cycle in clockwise order).

*/
  int findStartPos(int pos)  const{
     HalfSegment hs;
     line->Get(pos,hs);
     // collect all unused halfsegments with the same dominating point as hs
     // into a vector
     vector<pair<HalfSegment,int> > candidates;
     if(!usage[pos]){
         candidates.push_back(pair<HalfSegment,int>(hs,pos));
     }
     Point dp = hs.GetDomPoint();
     int pos1 = pos-1;
     bool done = false;
     while( (pos1>=0) && ! done) {
        line->Get(pos1,hs);
        if(!AlmostEqual(hs.GetDomPoint(),dp)){
          done = true;
        } else if(!usage[pos1]){
          candidates.push_back( pair<HalfSegment,int>(hs,pos1));
        }
        pos1--;
     }
     pos1=pos + 1;
     done = false;
     while( (pos1<line->Size()) && ! done) {
        line->Get(pos1,hs);
        if(!AlmostEqual(hs.GetDomPoint(),dp)){
          done = true;
        } else if(!usage[pos1]){
          candidates.push_back(pair<HalfSegment,int>(hs,pos1));
        }
        pos1++;
     }


     if(candidates.size() < 1){
       return -1;
     }
     // search hs with minimum slope

     // search for the unused halfsegment with he maximum slope
     int index = -1;
     double slope = 0;
     for(size_t i=0; i< candidates.size(); i++){
       if(!usage[candidates[i].second]){
          pair<HalfSegment,int> cand = candidates[i];
          double slope1=0;
          bool up;
          if(getSlope(cand.first,slope1,up)){
             if(index < 0 ){
                index = i;
                slope = slope1;
             } else if(slope1>slope){
                index = i;
                slope = slope1;
             }
          } else { // vertical segment
             if(up){ // maximum slope found immediately
               return candidates[i].second;
             }
          }
       }
     }
     if(index < 0){ // no unused element found
        return -1;
     }
     return candidates[index].second;

  }

/*
Computes the slope of an HalfSegment from dompoint to secpoint.
If the segment is vertical, false is returned and up is set to be
true if the segment goes up.

*/
  static bool getSlope(const HalfSegment& hs, double& slope, bool& up){
     Point p1 = hs.GetDomPoint();
     Point p2 = hs.GetSecPoint();
     double x1 = p1.GetX();
     double x2 = p2.GetX();
     double y1 = p1.GetY();
     double y2 = p2.GetY();
     up = y1 < y2;
     if(AlmostEqual(x1,x2)){
        return false;
     }
     slope = (y2-y1)/(x2-x1);
     return true;
  }

  Line* constructLine(vector<int>& path){
     if(path.empty()){
        return 0;
     }
     Line* res = new Line(path.size()*2);
     res->StartBulkLoad();
     vector<int>::iterator it;
     int c = -1;
     for(it=path.begin(); it!=path.end();it++){
        c++;
        HalfSegment hs;
        line->Get(*it,hs);
        hs.attr.edgeno = c;
        (*res) += hs;
        hs.SetLeftDomPoint(!hs.IsLeftDomPoint());
        (*res) +=hs;
     }
     res->EndBulkLoad(true,false);
     return res;
  }


/*
~nextPos~

extends a pth ending at the halfsegment at pos by the halfsegment having the
most right direction. If this halfsegment is not present (dead end) or
already used, (-1, false) is returned.

*/

   int nextPos(int pos) const{
      HalfSegment hs;
      line->Get(pos,hs);
      Point dp = hs.GetSecPoint(); // dompoint of the partner
      int ppos = hs.attr.partnerno;
      vector<pair<HalfSegment,int> > candidates;
      int ppos1 = ppos-1;
      bool done = false;
      HalfSegment hs1;
      while((ppos1>=0)&&!done){
         line->Get(ppos1,hs1);
         if(!AlmostEqual(dp,hs1.GetDomPoint())){
           done = true;
         } else {
           if(usage[ppos1]==3){
              return ppos1;
           }
           if(!usage[ppos1]){
              candidates.push_back(pair<HalfSegment,int>(hs1,ppos1));
           }
         }
         ppos1--;
      }
      ppos1 = ppos+1;
      done = false;
      while((ppos1<line->Size())&&!done){
         line->Get(ppos1,hs1);
         if(!AlmostEqual(dp,hs1.GetDomPoint())){
           done = true;
         } else {
           if(usage[ppos1]==3){
              return ppos1;
           }
           if(!usage[ppos1]){
             candidates.push_back(pair<HalfSegment,int>(hs1,ppos1));
           }
         }
         ppos1++;
      }

      if(candidates.empty()){ // no extension possible
        return -1;
      }
      if(candidates.size()==1){ // simple extension, no branch
        return candidates[0].second;
      }

      //partition candidates to those ones left of hs and those ones right of hs
      vector<pair<HalfSegment,int> > candidates_left;
      vector<pair<HalfSegment,int> > candidates_right;
      dp = hs.GetDomPoint();
      Point sp = hs.GetSecPoint();
      for(unsigned int i=0;i<candidates.size();i++){
         Point p = candidates[i].first.GetSecPoint();
         if(isRight(dp,sp,p)){
            candidates_right.push_back(candidates[i]);
         } else {
            candidates_left.push_back(candidates[i]);
         }
      }


      candidates = candidates_right.size()>0?candidates_right:candidates_left;


      // search for the right most extension
      int index = 0;
      for(unsigned int i=1;i<candidates.size() ; i++){
         if(moreRight(candidates[index].first, candidates[i].first)){
            index = i;
         }
      }

      return candidates[index].second;
   }



/*
~notLeft~

Returns true if __hs__ is vertical or directed to right.

*/

   bool notLeft(HalfSegment& hs) const{
     double dx = hs.GetDomPoint().GetX();
     double sx = hs.GetSecPoint().GetX();
     return (AlmostEqual(dx,sx) || (sx>dx));
   }

/*
~moreRight~

Returns true if hs2 closes a smaller clockwise cycle than hs1.
 Both halfsegments must
have the same dominating point.

*/
  bool moreRight(HalfSegment& hs1, HalfSegment& hs2) const{
     Point dp1 = hs1.GetDomPoint();
     Point dp2 = hs2.GetDomPoint();
     assert(AlmostEqual(dp1,dp2));
     Point sp1 = hs1.GetSecPoint();
     Point sp2 = hs2.GetSecPoint();
     return isRight(dp1,sp1,sp2);
  }


/*
~isRight~

Returns true if r is on the right side of the line defined by p and q

*/
  bool isRight(const Point& p, const Point& q, const Point& r) const{
    double rx=r.GetX();
    double ry=r.GetY();
    double px=p.GetX();
    double py=p.GetY();
    double qx=q.GetX();
    double qy=q.GetY();

    double A2 = px*qy + py*rx + qx*ry - (rx*qy + ry*px +qx*py);
    return A2 < 0;
  }


};


/*
10.38.3 Value Mapping

*/

int findCyclesVM(Word* args, Word& result, int message, Word& local,
                Supplier s )
{
  FindCyclesInfo* li = (FindCyclesInfo*) local.addr;
  switch(message){
    case OPEN: {
                  if(li){
                     delete li;
                     local.addr=0;
                  }
                  Line* arg = (Line*) args[0].addr;
                  bool extractHoles = false;
                  if(qp->GetNoSons(s)==2){
                    CcBool* b = (CcBool*) args[1].addr;
                    if(!b->IsDefined()){
                       return 0;
                    }
                    extractHoles = b->GetValue();
                  }
                  if(arg->IsDefined()){
                    local.addr = new FindCyclesInfo(arg, extractHoles);
                  }
                  return 0;
               }
     case REQUEST: {
                  result.addr = li?li->nextLine():0;
                  return result.addr?YIELD:CANCEL;
               }
     case CLOSE: {
                  if(li){
                    delete li;
                    local.addr = 0;
                  }
                  return 0;
               }
     default: return -1;
  }

}

/*
10.38.4 Specification

*/

OperatorSpec findCyclesSpec (
    "line [x bool] -> stream(line)",
    "findCycles(_,_)",
    "Finds minimum cycles within a line value. If the boolean argument is"
    " present and TRUE, holes are also part of the resulting lines",
    "query findCycles(BGrenzenLine) count"
  );

/*
10.38.5 Operator Instance

*/
Operator findCycles(
   "findCycles",
   findCyclesSpec.getStr(),
   findCyclesVM,
   Operator::SimpleSelect,
   findCyclesTM
);


/*
10.39 Debug-Operator markUsage

10.39.1 Type Mapping: line -> Stream(tuple(L:line)(U : int))

*/
ListExpr markUsageTM(ListExpr args){
   string err = "line expected";
   if(!nl->HasLength(args,1)){
      return listutils::typeError(err);
   }
   if(!Line::checkType(nl->First(args))){
      return listutils::typeError(err);
   }
   ListExpr attrList = nl->TwoElemList(
              nl->TwoElemList(
                  nl->SymbolAtom("L"),
                  listutils::basicSymbol<Line>()),
              nl->TwoElemList(
                  nl->SymbolAtom("Usage"),
                  listutils::basicSymbol<CcInt>()));
   return nl->TwoElemList(
               listutils::basicSymbol<Stream<Tuple> >(),
               nl->TwoElemList(
                    listutils::basicSymbol<Tuple>(),
                     attrList));
}

/*
10.30.2 Value Mapping

*/

class MarkUsageInfo{
  public:
   MarkUsageInfo(Line* line, ListExpr tupleType){
     this->line = line;
     if(line->IsDefined() && (line->Size()>0)){
        usage = new char[line->Size()];
        critical = new char[line->Size()];
        pos = 0;
        markUsage(line,usage, critical);
        tt = new TupleType(tupleType);
     } else {
        usage = 0;
        pos = -1;
        tt = 0;
     }
   }

   ~MarkUsageInfo(){
     if(usage){
       delete[] usage;
     }
     if( critical){
        delete[] critical;
     }
     if(tt){
        tt->DeleteIfAllowed();
     }
   }

   Tuple* next(){
      if(!usage){
        return 0;
      }
      if(pos>=line->Size()){
        return 0;
      }
      Tuple* res = new Tuple(tt);
      HalfSegment hs;
      line->Get(pos,hs);
      Line* l = new Line(2);
      hs.attr.edgeno = 0;
      l->StartBulkLoad();
      (*l) += hs;
      hs.SetLeftDomPoint(!hs.IsLeftDomPoint());
      (*l) += hs;
      l->EndBulkLoad();
      res->PutAttribute(0,l);
      res->PutAttribute(1,new CcInt(true,usage[pos]));
      pos++;
      return res;
   }

   private:
       Line* line;
       char* usage;
       char* critical;
       int pos;
       TupleType* tt;
};

int markUsageVM(Word* args, Word& result, int message, Word& local,
                         Supplier s )
{
   MarkUsageInfo* li = (MarkUsageInfo*) local.addr;
   switch(message){
      case OPEN: {
         if(li){
           delete li;
         }
         local.addr = new MarkUsageInfo((Line*) args[0].addr,
                                        nl->Second(GetTupleResultType(s)));
         return 0;
      }
      case REQUEST: {
         result.addr = li?li->next():0;
         return result.addr?YIELD:CANCEL;
      }
      case CLOSE: {
         if(li){
            delete li;
            local.addr = 0;
         }
      }
   }
   return -1;
}

OperatorSpec markUsageSpec (
    "line -> stream(tuple(L line)(U int))",
    "markUsage(_)",
    "Rteurns the usage of halfsegments. (for debugging only)",
    "query markUsage(GrenzenLine) "
  );

/*
10.38.5 Operator Instance

*/
Operator markUsageOp(
   "markUsage",
   markUsageSpec.getStr(),
   markUsageVM,
   Operator::SimpleSelect,
   markUsageTM
);

/*
10.40 Operator getCriticalPoints


10.40.1 Signature


*/

ListExpr criticalPointsTM(ListExpr args){
  string err = "line expected";
  if(!nl->HasLength(args,1)){
    return listutils::typeError(err);
  }
  if(!Line::checkType(nl->First(args))){
    return listutils::typeError(err);
  }
  return listutils::basicSymbol<Points>();
}



int criticalPointsVM(Word* args, Word& result, int message, Word& local,
                         Supplier s )
{
  Line* arg = (Line*) args[0].addr;
  result = qp->ResultStorage(s);
  Points* res = (Points*) result.addr;
  if(!arg->IsDefined()){
      res->SetDefined(false);
      return 0;
  } else {
     res->SetDefined(true);
     res->Clear();
     if(arg->Size()==0){
       return 0;
     }
     char* usage = new char[arg->Size()];
     char* crit = new char[arg->Size()];
     markUsage(arg,usage,crit);
     res->StartBulkLoad();
     for(int i=0;i<arg->Size();i++){
        if(crit[i]){
          HalfSegment hs;
          arg->Get(i,hs);
          if(hs.IsLeftDomPoint()){
             (*res) += hs.GetDomPoint();
          }
        }
     }
     res->EndBulkLoad();
     delete[] usage;
     delete[] crit;
     return 0;
  }

}

OperatorSpec criticalPointsSpec (
    "line -> points",
    "criticalPoints(_)",
    "Returns the critical points of a line value",
    "query criticalPoints(GrenzenLine) "
  );

Operator criticalPoints(
   "criticalPoints",
   criticalPointsSpec.getStr(),
   criticalPointsVM,
   Operator::SimpleSelect,
   criticalPointsTM
);



/*
Operator testRegionCreator

For debugging only, should be removed after tests

Signature : line -> region

*/
ListExpr testRegionCreatorTM(ListExpr args){
  string err = "line expected";
  if(!nl->HasLength(args,1)){
     return listutils::typeError(err);
  }
  if(!Line::checkType(nl->First(args))){
     return listutils::typeError(err);
  }
  return listutils::basicSymbol<Region>();
}


int testRegionCreatorVM(Word* args, Word& result, int message, Word& local,
                       Supplier s )
{
  Line* line = (Line*) args[0].addr;
  result=qp->ResultStorage(s);
  Region* res = (Region*) result.addr;
  RegionCreator<DbArray>::createRegion((DbArray<HalfSegment>*)
                           line->GetFLOB(0),res);
  return 0;
}

OperatorSpec testRegionCreatorSpec (
    "line -> region",
    "testRegionCreator(_)",
    "for debugging only",
    "query testRegionCreator(GrenzenLine) "
  );

Operator testRegionCreator(
   "testRegionCreator",
   testRegionCreatorSpec.getStr(),
   testRegionCreatorVM,
   Operator::SimpleSelect,
   testRegionCreatorTM
);


/*
1.40 Collect Box

1.40.1 TypeMapping

Signature is Stream<SPATIAL> x bool -> rectangle

*/
ListExpr collect_boxTM(ListExpr args){
  string err = "stream(spatial) x bool expected ";
  if (!nl->HasLength(args, 2)) {
    return listutils::typeError(err +  "  (wrong number of args)");
  }
  if (!Stream<Attribute>::checkType(nl->First(args))) {
    return listutils::typeError(err +" (first arg is not an attribute stream)");
  }
  if (!CcBool::checkType(nl->Second(args))) {
    return listutils::typeError(err + " (second arg is not a bool)");
  }
  ListExpr attr = nl->Second(nl->First(args));
  if (listutils::isKind(attr, Kind::SPATIAL1D())) {
    return listutils::basicSymbol<Rectangle<1> >();
  }
  if (listutils::isKind(attr, Kind::SPATIAL2D())) {
    return listutils::basicSymbol<Rectangle<2> >();
  }
  if (listutils::isKind(attr, Kind::SPATIAL3D())) {
    return listutils::basicSymbol<Rectangle<3> >();
  }
  if (listutils::isKind(attr, Kind::SPATIAL4D())) {
    return listutils::basicSymbol<Rectangle<4> >();
  }
  if (listutils::isKind(attr, Kind::SPATIAL8D())) {
    return listutils::basicSymbol<Rectangle<8> >();
  }
  return listutils::typeError(err + " (attribute not in kind SPATIALxD)");
}

int collect_boxSelect(ListExpr args) {
  ListExpr attr = nl->Second(nl->First(args));
  if (listutils::isKind(attr, Kind::SPATIAL1D())) return 0;
  if (listutils::isKind(attr, Kind::SPATIAL2D())) return 1;
  if (listutils::isKind(attr, Kind::SPATIAL3D())) return 2;
  if (listutils::isKind(attr, Kind::SPATIAL4D())) return 3;
  if (listutils::isKind(attr, Kind::SPATIAL8D())) return 4;
  return -1;
}

template<int dim>
int collect_boxVM(Word* args, Word& result, int message, Word& local,
                  Supplier s) {
  Stream<StandardSpatialAttribute<dim> > stream(args[0]);
  CcBool* ignoreUndefined = (CcBool*) args[1].addr;
  result = qp->ResultStorage(s);
  Rectangle<dim>* res = (Rectangle<dim>*) result.addr;
  res->SetDefined(false);
  if(!ignoreUndefined->IsDefined()){
     return 0;
  }
  stream.open();
  bool first = true;
  bool useundef = !ignoreUndefined->GetValue();
  StandardSpatialAttribute<dim>* a = stream.request();
  while(a){
     if(a->IsDefined()){
        Rectangle<dim> box  = a->BoundingBox();
        if(box.IsDefined()){
          if(first){
            res->SetDefined(true);
            (*res) = box;
            first = false;
          } else {
            res->Extend(box);
          }
        }
     } else {
        if(!useundef){
           res->SetDefined(false);
           a->DeleteIfAllowed();
           a=0;
           stream.close();
           return 0;
        }
     }
     a->DeleteIfAllowed();
     a = stream.request();
  }
  stream.close();
  return 0;
}

ValueMapping collect_boxVMs[] = {collect_boxVM<1>, collect_boxVM<2>, 
               collect_boxVM<3>, collect_boxVM<4>, collect_boxVM<8>};

OperatorSpec collect_boxSpec (
    "stream<SPATIAL> x bool -> rectangle",
    " _ collect_box[_]",
    "Computes the bounding box from a stream of spatial attributes"
    "If the second parameter is set to be true, undefined elements"
    " within the stream are ignored. Otherwise an undefined element"
    " will lead to an undefined result.",
    "query strassen feed projecttransformstream[GeoData] collect_box[TRUE] "
  );

Operator collect_box(
   "collect_box",
   collect_boxSpec.getStr(),
   5,
   collect_boxVMs,
   collect_boxSelect,
   collect_boxTM
);

/*
1.38 Operator intersection_rob

To test the robust implementation

1.38.1 Type Mapping

*/

ListExpr intersection_robTM(ListExpr args){
  string err = "region x line expected";
  if(!nl->HasLength(args,2)){
     return listutils::typeError(err);
  }
  ListExpr a1 = nl->First(args);
  ListExpr a2 = nl->Second(args);
  if(Region::checkType(a1) && // region x line
     Line::checkType(a2)){
     return listutils::basicSymbol<Line>();
  }
  if(Line::checkType(a1) && // line x region
     Region::checkType(a2)){
     return listutils::basicSymbol<Line>();
  }

  if(Line::checkType(a1) && // line x line
     Line::checkType(a2)){
     return listutils::basicSymbol<Line>();
  }

  return listutils::typeError();
}

/*
1.38.2 Value Mapping

Implemented as template for later support of other types.

*/
template<class A, class B, class R>
int intersection_robVM1(Word* args, Word& result, int message, Word& local,
                       Supplier s ){

   A* arg1 = (A*) args[0].addr;
   B* arg2 = (B*) args[1].addr;
   result = qp->ResultStorage(s);
   R* res = (R*) result.addr;
   if(!arg1->IsDefined() || !arg2->IsDefined()){
     res->SetDefined(false);
   } else {
     robust::intersection(*arg1,*arg2,*res);
   }
   return 0;
}

/*
1.38.3 ValueMappinmg Array and SelectionFunction

*/
ValueMapping intersection_robVM[] = {
      intersection_robVM1<Region,Line,Line>,
      intersection_robVM1<Line,Region,Line>,
      intersection_robVM1<Line,Line,Line>
    };

int intersection_robSelect(ListExpr args){
    ListExpr a1 = nl->First(args);
    ListExpr a2 = nl->Second(args);
    if(Region::checkType(a1) && Line::checkType(a2)){
      return 0;
    }
    if(Line::checkType(a1) && Region::checkType(a2)){
      return 0;
    }
    if(Line::checkType(a1) && Line::checkType(a2)){
      return 0;
    }
    return -1;
}

/*
1.38.4 Specification

*/
OperatorSpec intersection_robSpec (
    "region x line -> line | line x region -> line | line x line -> line",
    " intersection_rob(_,_)",
    "Computes the intersection of the arguments"
    "using a (hopefully) robust implementation.",
    "query intersection_rob(thecenter, boundary(thecenter) "
  );

/*
1.38.5 Operator instance

*/
Operator spatialintersection_rob( "intersection_rob",
                            intersection_robSpec.getStr(),
                            3,
                            intersection_robVM,
                            intersection_robSelect,
                            intersection_robTM);



/*
1.39 Operator contains_rob

1.39.1 Type Mapping

Signature is region x point [x bool] -> bool

*/
ListExpr contains_robTM(ListExpr args){

  string err = " region x point [ x bool] expected";
  int len = nl->ListLength(args);
  if( (len!=2) && (len!=3) ){
      return listutils::typeError(err);
  }
  if(!Region::checkType(nl->First(args)) ||
     !Point::checkType(nl->Second(args)) ){
     return listutils::typeError(err);
  }
  if(len==3){
    if(!CcBool::checkType(nl->Third(args))){
         return listutils::typeError(err);
    }
    return listutils::basicSymbol<CcBool>();
  }
  return nl->ThreeElemList( nl->SymbolAtom(Symbol::APPEND()),
                            nl->OneElemList(nl->BoolAtom(false)),
                            listutils::basicSymbol<CcBool>());

}
/*
1.39.2 Value Mapping

*/
int contains_robVM(Word* args, Word& result, int message, Word& local,
                    Supplier s ){
   Region* reg = (Region*) args[0].addr;
   Point* p = (Point*) args[1].addr;
   CcBool* b = (CcBool*) args[2].addr;
   result = qp->ResultStorage(s);
   CcBool* res = (CcBool*) result.addr;
   if(!reg->IsDefined() || !p->IsDefined() || !b->IsDefined()){
       res->SetDefined(false);
   }
   int r = robust::contains(*reg,*p);
   if(r==0){
     res->Set(true,false);
   } else {
     if(r==1 || b->GetValue()){
       res->Set(true,true);
     } else {
       res->Set(true,false);
    }
   }
   return 0;
}

/*
1.39.3 Specification

*/
OperatorSpec contains_robSpec (
    "region x point [x bool] -> line",
    " contains_rob(_,_)",
    "Checks whether the point is inside the region"
    "if the booolean argument is present and has value true"
    " the function returns also true, if the point is "
    " on the border of the region",
    "query contains_rob(thecenter, mehringdamm "
  );

/*
1.39.4 Operator instance

*/
Operator contains_rob(
   "contains_rob",
   contains_robSpec.getStr(),
   contains_robVM,
   Operator::SimpleSelect,
   contains_robTM
);


/*
1.40 Operator ~holes~

1.40.1 Type Mapping : region -> region

*/

ListExpr getHolesTM(ListExpr args){
  string err = "region expected";
  if(!nl->HasLength(args,1)){
    return listutils::typeError(err + "( wrong number of args");
  }
  if(!Region::checkType(nl->First(args))){
    return listutils::typeError(err);
  }
  return listutils::basicSymbol<Region>();
}

/*
1.40.2 Value Mapping

*/
int getHolesVM(Word* args, Word& result, int message, Word& local,
                    Supplier s ){

   Region* arg = (Region*) args[0].addr;
   result = qp->ResultStorage(s);
   Region* res = (Region*) result.addr;
   arg->getHoles(*res);
   return 0;
}

/*
1.40.3 Specification

*/

OperatorSpec getHolesSpec (
    "region -> region ",
    " getHoles(_) ",
    "Returns the holes of a region.",
    "query isempty(getHoles(thecenter)) "
  );

/*
1.40.4 Operator instance

*/
Operator getHoles(
   "getHoles",
   getHolesSpec.getStr(),
   getHolesVM,
   Operator::SimpleSelect,
   getHolesTM
);



/*
1.41 Opegartor collect_line2

This operatotr is for checking the robust realminize implementation.

1.41.1 Type Mapping

*/
ListExpr collect_line2TM(ListExpr args){
  string err =" stream(line) expected";
  if(!nl->HasLength(args,1)){
     return listutils::typeError(err + " (wrong number of args)");
  }
  if(!Stream<Line>::checkType(nl->First(args))){
     return listutils::typeError(err);
  }
  return listutils::basicSymbol<Line>();
}


int collect_line2VM(Word* args, Word& result, int message, Word& local,
                    Supplier s ){

  result = qp->ResultStorage(s);
  Line* res = (Line*) result.addr;
  Stream<Line> stream(args[0]);
  stream.open();
  Line* line;
  res->Clear();
  res->SetDefined(true);
  DbArray<HalfSegment>*  dba= new DbArray<HalfSegment>(2000);
  HalfSegment hs;
  while( (line = stream.request()) != 0){
     if(line->IsDefined()){
       for(int i=0;i<line->Size();i++){
          line->Get(i,hs);
           if(hs.IsLeftDomPoint()){
              dba->Append(hs);
           }
       }
     }
     line->DeleteIfAllowed();
  }
  stream.close();

  // get the DbArray from result;
  res->StartBulkLoad();
  DbArray<HalfSegment>* dbl = (DbArray<HalfSegment>*) res->GetFLOB(0);
  robust::realminize(*dba, *dbl);
  dba->destroy();
  delete dba;

  res->EndBulkLoad(true,false); // sort, no realminize
  return 0;
}


/*
1.41.3 Specification

*/

OperatorSpec collect_line2Spec (
    " stream(line) -> line ",
    " _ collect_line2 ",
    " Builds the union of all lines within the "
    "stream iggnoring undefined lines.",
    "query strassen feed projecttransformstream[GeoData] collect_line2 "
  );

/*
1.41.4 Operator instance

*/
Operator collect_line2(
   "collect_line2",
   collect_line2Spec.getStr(),
   collect_line2VM,
   Operator::SimpleSelect,
   collect_line2TM
);



/*
1.41 Operator ~getInnerPoint~

Returns some point from the inetrior of a region.

1.41.1 Type Mapping

*/
ListExpr getInnerPointTM(ListExpr args){
  string err = "region expected";
  if(!nl->HasLength(args,1)){
    return listutils::typeError(err + " (wrong number of arguments");
  }
  if(!Region::checkType(nl->First(args))){
    return listutils::typeError(err);
  }
  return listutils::basicSymbol<Point>();
}

/*
1.41.2 Value Mapping

*/
int getInnerPointVM(Word* args, Word& result, int message, Word& local,
                    Supplier s ){

  Region* arg = (Region*) args[0].addr;
  result = qp->ResultStorage(s);
  Point* res = (Point*) result.addr;
  if(!arg->IsDefined()){
      res->SetDefined(false);
      return 0;
  }
  if(arg->IsEmpty()){
      res->SetDefined(false);
      return 0;
  }
  HalfSegment hs;
  arg->Get(0,hs);
  double dx = abs(hs.GetDomPoint().GetX() - hs.GetSecPoint().GetX());
  double dy = abs(hs.GetDomPoint().GetY() - hs.GetSecPoint().GetY());
  double dist = -1;
  Point mp = hs.middlePoint();
  double x = mp.GetX();
  double y = mp.GetY();

  double (*distComp)(const HalfSegment& hs, const double x, const double y);

  int dir;

  if(dx>dy){
    if(hs.attr.insideAbove){
       distComp = &PRegionCreator::getUpDist;
       dir = 0;
    } else {
       distComp = &PRegionCreator::getDownDist;
       dir = 1;
    }
  }  else {
    double y2 = hs.GetSecPoint().GetY();
    double y1 = hs.GetDomPoint().GetY();
    if(hs.attr.insideAbove  == (y2>y1)){
      distComp = &PRegionCreator::getLeftDist;
      dir = 2;
    }  else{
      distComp = &PRegionCreator::getRightDist;
      dir = 3;
    }
  }
  double d;


  for(int i=1;i<arg->Size();i++){
    arg->Get(i,hs);
    if(hs.IsLeftDomPoint()){
       d = distComp(hs,x,y);
       if(d>0){
           if((dist<0) || d<dist){
              dist = d;
           }
       }
    }
  }

  assert(dist>0);
  switch(dir){
     case 0: y += dist/2.0; break;
     case 1: y -= dist/2.0; break;
     case 2: x -= dist/2.0; break;
     case 3: x += dist/2.0; break;
     default: assert(false);
  }
  res->Set(x,y);
  return 0;
}


/*
1.42.3 Specification

*/

OperatorSpec getInnerPointSpec (
    " region -> point ",
    " getInnerPoint(_) ",
    " Returns some point within the interior of a region.",
    " query WFaechen feed extend[IP : getInnerPoint(.GeoData)] "
    "filter[ inInterior(IP,.GeoData)] count = WFlaechen count "
  );


/*
1.42.4 Operator instance

*/
Operator getInnerPoint(
   "getInnerPoint",
   getInnerPointSpec.getStr(),
   getInnerPointVM,
   Operator::SimpleSelect,
   getInnerPointTM
);


/*
1.43 Operator ~checkRealm~

Debugging operator. Checks whether the halfsegments of a line or a regaion
are realminized.

1.43.1 Type Map

*/
ListExpr checkRealmTM(ListExpr args){
   string err = "line or region expected";
   if(!nl->HasLength(args,1)){
     return listutils::typeError(err);
   }
   ListExpr arg = nl->First(args);
   if(!Line::checkType(arg) && !Region::checkType(arg)){
     return listutils::typeError(err);
   }
   return listutils::basicSymbol<CcBool>();
}

/*
1.43.2 Value Mapping

*/
int checkRealmVM(Word* args, Word& result, int message, Word& local,
                    Supplier s ){

  if(message==INIT){
    ListExpr ttl = robust::RealmChecker<DbArray>::getTupleType();
    ListExpr numTupleType = SecondoSystem::GetCatalog()->NumericType(ttl);
    TupleType* tt = new TupleType(numTupleType);
    qp->GetLocal2(s).addr=tt;
    return 0;
  }
  if(message==FINISH){
    ( (TupleType*)qp->GetLocal2(s).addr)->DeleteIfAllowed();
    qp->GetLocal2(s).addr=0;
    return 0;
  }

  TupleType* tt = (TupleType*)qp->GetLocal2(s).addr;
  Attribute* arg = (Attribute*) args[0].addr;
  result = qp->ResultStorage(s);
  CcBool* res = (CcBool*) result.addr;
  if(!arg->IsDefined()){
    res->SetDefined(false);
  } else {
    DbArray<HalfSegment>* hss = (DbArray<HalfSegment>*) arg->GetFLOB(0);
    robust::RealmChecker<DbArray> rc(hss,tt);
    res->Set(true,rc.checkRealm());
  }
  return 0;
}

/*
1.42.3 Specification

*/

OperatorSpec checkRealmSpec (
    " {region,line} -> bool ",
    " checkRealm(_) ",
    " Checks the argumet for correct realminization (debug operator).",
    " query WFaechen feed  filter[ checkRealm(.GeoData)(] count "
  );


/*
1.42.4 Operator instance

*/
Operator checkRealmOp(
   "checkRealm",
   checkRealmSpec.getStr(),
   checkRealmVM,
   Operator::SimpleSelect,
   checkRealmTM
);


/*
1.43 Operator badRealm

This operator returns the pairs of halfsegments within a
line or a region violating the realm properties.

*/

ListExpr badRealmTM(ListExpr args){
  string err = "line or region expected";
  if(!nl->HasLength(args,1)){
    return listutils::typeError(err + " ( wrong number of args");
  }
  ListExpr arg = nl->First(args);
  if(!Line::checkType(arg) && !Region::checkType(arg)){
    return listutils::typeError(err);
  }
  ListExpr tt = robust::RealmChecker<DbArray>::getTupleType();
  return nl->TwoElemList( listutils::basicSymbol<Stream<Tuple> >(),
                          tt);
}

/*
1.43.2 Value Mapping

*/

int badRealmVM(Word* args, Word& result, int message, Word& local,
               Supplier s ){

  PRealmChecker* li = (PRealmChecker*) local.addr;
  switch(message){
      case OPEN : {
                      if(li){
                        delete li;
                        li = 0;
                      }
                      Attribute* a = (Attribute*) args[0].addr;
                      if(a->IsDefined()){
                        DbArray<HalfSegment>* hss =
                             (DbArray<HalfSegment>*) a->GetFLOB(0);
                        local.addr = new PRealmChecker(hss);
                      }
                      return 0;
                  }
      case REQUEST: {
                      result.addr = li?li->nextTuple():0;
                      return result.addr?YIELD:CANCEL;
                  }
      case CLOSE: {
              if(li){
                 delete li;
                 local.addr = 0;
              }
              return 0;
      }
  }
  return -1;

}


/*
1.43.3 Specification

*/

OperatorSpec badRealmSpec (
    " {region,line} -> stream(tuple( (No1 int)(Partner1 int)"
    "(Segment1 line)(No2 int)(Partner2 int)(Segment2 line))) ",
    " badRealm(_) ",
    " Returns pairs of  halfsegments of a line or a region "
    "violating the realm properties. (For debugging purposes).",
    " query badRealm(BGrenzenLine) count "
  );


/*
1.43.4 Operator instance

*/
Operator badRealmOp(
   "badRealm",
   badRealmSpec.getStr(),
   badRealmVM,
   Operator::SimpleSelect,
   badRealmTM
);


/*
1.44 Operator crossings_rob


1.44.1 Type Mapping

*/
ListExpr crossings_robTM(ListExpr args){

   string err = "line x line expected";
   if(!nl->HasLength(args,2)){
     return listutils::typeError(err);
   }
   if(!Line::checkType(nl->First(args)) ||
      !Line::checkType(nl->Second(args))){
     return listutils::typeError(err);
   }
   return listutils::basicSymbol<Points>();
}

/*
1.44.2 Value Mapping

*/

int crossings_robVM(Word* args, Word& result, int message, Word& local,
               Supplier s ){

  result = qp->ResultStorage(s);
  Line* l1 = (Line*) args[0].addr;
  Line* l2 = (Line*) args[1].addr;
  Points* res = (Points*) result.addr;
  robust::crossings(*l1,*l2,*res);
  return 0;
}



/*
1.44.3 Specification

*/

OperatorSpec crossings_robSpec (
    " line x line -> points",
    " crossings_rob(l1,l2) ",
    " Returns common points of two lines which are not part "
    " of a common segment",
    " query crossings_rob(BGrenzenLine,BGrenzenLine) "
  );


/*
1.44.4 Operator instance

*/
Operator crossings_rob(
   "crossings_rob",
   crossings_robSpec.getStr(),
   crossings_robVM,
   Operator::SimpleSelect,
   crossings_robTM
);




class LineSplitIterator{

public:

  LineSplitIterator(Line* line){
    theLine=line;
    if(!line->IsDefined() || (line->Size() == 0)){
       theLine = 0;
       used = 0;
       return;
    }
    size = line->Size();
    theLine = line;
    used = new bool[size];
    memset(used,0,size*sizeof(bool));
    position = 0;
    start = 0;
    searchStart();
  }

  ~LineSplitIterator(){
      if(used){
         delete[] used;
      }
   }


  bool next(HalfSegment& nextHs, bool& newStart){
     if(position>=size){
        return false;
     }
     theLine->Get(position,nextHs); // get HalfSegment
     newStart = (position==start);
     gotoNext();
     return true;
  }

private:
   Line* theLine;
   bool* used;
   uint32_t position;
   uint32_t start;
   uint32_t size;


   void gotoNext(){
     HalfSegment hs;
     theLine->Get(position,hs);
     used[position] = true;
     // goto Partnerhs
     uint32_t partnerNo =   hs.attr.partnerno;
     used[partnerNo] = true;
     int x = getExtension(partnerNo);
     if(x!=0){
        position = partnerNo+x;
        return;
     }
     searchStart();
   }

   // returns 0 if there is no extension
   // returns x != 0 if (pos+x) is an unused extension
   int getExtension(int pos){
     HalfSegment hs;
     theLine->Get(pos,hs);
     Point dp = hs.GetDomPoint();
     HalfSegment cand;
     // search left
     int pos2 = pos - 1;
     while(pos2>0){
        if(used[pos2]){
          pos2--;
        } else {
          theLine->Get(pos2,cand);
          if(AlmostEqual(dp,cand.GetDomPoint())){
             return pos2-pos;
          } else {
            pos2 = -1;
          }
        }
     }
     // search right
     pos2 = pos+1;
     while(pos2<(int)size){
       if(used[pos2]){
          pos2++;
       } else {
          theLine->Get(pos2,cand);
          if(AlmostEqual(dp,cand.GetDomPoint())){
             return pos2-pos;
          } else {
              pos2 = size;
          }
       }
     }
     return 0;
   }


   void searchStart(){

     start = 0;
     while( (start<size) && used[start]){
        start++;
     }
     if(start>=size){
        position = start;
        return;
     }
     int start2 = start;
     int x;
     x = getExtension(start2);
     HalfSegment n;
     bool* used2 = new bool[size];
     memset(used2,0,size*sizeof(bool));
     used2[start] = true;
     while(x){
        theLine->Get(start2 + x,n);

         start2 = n.attr.partnerno;
         x = getExtension(start2);
         if(used2[start2]){ // cycle
            position = start2;
            start = start2;
            delete[] used2;
            return;
         }
         used2[start2] = true;
     }
     delete[] used2;
     start = start2;
     position = start;
   }
};


/*
10.9.81 Operator ~linesplit~

This operator splits a line into pieces of equal length

*/

ListExpr splitlineTM(ListExpr args){

   int len = nl->ListLength(args);
   if((len<2) || (len>4) ){
     return listutils::typeError("2, 3, or 4  args required");
   }
   string err = "line x real [x geoid [xreal]] expected";
   if(!Line::checkType(nl->First(args)) ||
      !CcReal::checkType(nl->Second(args))){
     return listutils::typeError(err);
   }
   if((len>2) &&
      !Geoid::checkType(nl->Third(args))){
     return listutils::typeError(err);
   }
   if( (len>3 ) &&
       !CcReal::checkType(nl->Fourth(args))){
     return listutils::typeError(err);
   }

   ListExpr res = nl->TwoElemList(
                         listutils::basicSymbol<Stream<Line> >(),
                         listutils::basicSymbol<Line>() );
   if(len==4){ // all arguments come form user
      return res;
   }
   ListExpr gl = nl->TheEmptyList();
   if(len == 3) { // scaleFactor not present
     gl = nl->OneElemList(nl->RealAtom(1.0));
   } else {
     assert(len==2);
     gl = nl->TwoElemList(
                    nl->TwoElemList(
                      listutils::basicSymbol<Geoid>(),
                      nl->SymbolAtom(Symbol::UNDEFINED())),
                    nl->RealAtom(1.0));
   }
   return nl->ThreeElemList(nl->SymbolAtom(Symbol::APPEND()),
                           gl,
                           res);

}



class SplitLineInfo{


  public:
     SplitLineInfo(Line* _line, CcReal* _dist, Geoid* _geoid,
                   CcReal* _scale){
         if(!_line->IsDefined() || !_dist->IsDefined() ){
           it=0;
           return;
         }
         dist = _dist->GetValue();
         if((_line->Size()==0) || dist<0 || AlmostEqual(dist,0)){
            it = 0;
            return;
         }
         if(!_scale->IsDefined()){
            scale = 1.0;
         } else {
            scale = _scale->GetValue();
         }
         if(AlmostEqual(scale,0) || scale < 0){
            it = 0;
            return;
         }
         it = new LineSplitIterator(_line);
         hasLastHs = false;
         geoid = _geoid->IsDefined()?_geoid:0;
     }

     ~SplitLineInfo(){
        if(it){
          delete it;
        }
     }


    // debug code
    Line* next1(){
       static uint32_t count = 0;

       count++;
       HalfSegment hs;
       bool newStart;
       bool hn = it->next(hs, newStart);
       if(!hn){
          return 0;
       }
       Line* res = new Line(2);
       res->StartBulkLoad() ;
       (*res) += hs;
       hs.SetLeftDomPoint(!hs.IsLeftDomPoint());
       (*res) += hs;
       res->EndBulkLoad(true,true,true);
       return res;

    }
    // debug code

     Line* next(){
        Line* res = new Line(10);
        res->StartBulkLoad();
        double length = 0;
        bool done = false;
        if(hasLastHs){
          done = appendHs(res,lastHs, length);
        }

        HalfSegment hs;
        bool newStart;
        while(!done){
          bool nextHs = it->next(hs , newStart);
          if(!nextHs){
            done = true;
          } else {
             if(newStart){
               if(res->Size()==0){
                 done = appendHs(res,hs,length);
               } else {
                 done = true;
                 lastHs = hs;
                 hasLastHs = true;
              }
             } else {
                done = appendHs(res,hs,length);
             }
          }
        }



        res->EndBulkLoad(true,true,true);
        if(res->Size()==0){
          delete res;
          res = 0;
        }
        return res;

     }

  private:
    LineSplitIterator* it;
    HalfSegment lastHs;
    bool hasLastHs;
    double dist;
    Geoid* geoid;
    double scale;


   bool appendHs(Line*& res, HalfSegment& hs, double& length){

       Point p1 = hs.GetDomPoint();
       Point p2 = hs.GetSecPoint();
       p1.Scale(1.0/scale);
       p2.Scale(1.0/scale);

       //cout << "compute distance between " << p1 << " and " << p2 << endl;

       double hsl = p1.Distance(p2, geoid);

       if(((length + hsl)  <= dist) || AlmostEqual(length+hsl,dist)){
         // append complete hs
          (*res) += hs;
          hs.SetLeftDomPoint(!hs.IsLeftDomPoint());
          (*res) += hs;
          hasLastHs = false;
          length += hsl;
          return AlmostEqual(dist,length+hsl);
       }
       // append only a part of hs
       double rest = dist - length;
       assert(rest>0);
       double delta = rest/hsl;

       assert(delta > 0);
       assert(delta < 1);
       Point dp = hs.GetDomPoint();
       Point sp = hs.GetSecPoint();
       double x1 = dp.GetX();
       double x2 = sp.GetX();
       double xs = x1 + delta*(x2-x1);
       double y1 = dp.GetY();
       double y2 = sp.GetY();
       double ys = y1 + delta*(y2-y1);
       Point ps(true,xs,ys);

       if(AlmostEqual(dp,ps)){
         // nothing to append
         lastHs.Set(true,dp,sp);
         hasLastHs = true;
         return true;
       }
       if(AlmostEqual(ps,sp)){
          // append nearly all
          (*res) += hs;
          hs.SetLeftDomPoint(!hs.IsLeftDomPoint());
          (*res) += hs;
          hasLastHs = false;
          length += hsl;
          return true;
       }
       // real split
       HalfSegment hs1(true, dp, ps);
       HalfSegment hs2(true, ps, sp);

       lastHs = hs2;
       hasLastHs = true;
       (*res) += hs1;
       hs1.SetLeftDomPoint(false);
       (*res) += hs1;
       length += rest;
       return true;
   }


};



int splitlineVM(Word* args, Word& result, int message, Word& local,
               Supplier s ){


  SplitLineInfo* li = (SplitLineInfo*) local.addr;
  switch(message){
    case OPEN: {
        if(li){
          delete li;
        }
        local.addr = new SplitLineInfo((Line*) args[0].addr,
                                       (CcReal*) args[1].addr,
                                       (Geoid*) args[2].addr,
                                       (CcReal*) args[3].addr);
        return 0;

   }
   case REQUEST: {
       result.addr = li?li->next():0;
       return result.addr?YIELD:CANCEL;
   }
   case CLOSE : {
        if(li){
           delete li;
           local.addr = 0;
        }
   }

  }
  return -1;


}



OperatorSpec splitlineSpec (
    " line x real [x geoid [x real]] -> points",
    " linesplit(line, length, geoid, scalefactor)",
    " Splits a line into pieces of a given length. "
    " if geoid is given, the distance computations are"
    " done using this geoid. For scaled up geodata, the "
    "optional argument scalefactor can be used. All coordinates"
    " are divided by this factor befor the distance is computed",
    " query splitline(BGrenzenLine) count "
  );


/*
1.44.4 Operator instance

*/
Operator splitline(
   "splitline",
   splitlineSpec.getStr(),
   splitlineVM,
   Operator::SimpleSelect,
   splitlineTM
);


/*
2 Type Constructor dline

*/

GenTC<DLine> dline;

/*
Type Constructor DRM

*/
GenTC<DRM> drm;

GenTC<OIM> oim;


/*
Operator ~computeDRM~

Computes the direction relation matrix for 2 elements in Spatial2D.

*/

ListExpr computeDRMTM(ListExpr args){
  string err="SPATIAL2D x SPATIAL2D expected"; 
  if(!nl->HasLength(args,2)){
      return listutils::typeError(err);
  }
  if(!listutils::isKind(nl->First(args),Kind::SPATIAL2D()) ||
     !listutils::isKind(nl->Second(args),Kind::SPATIAL2D() )){
      return listutils::typeError(err);
  }
  return listutils::basicSymbol<DRM>();    
}


/*
Value Mapping

*/

int computeDRMVM(Word* args, Word& result, int message, Word& local,
               Supplier s ){

    StandardSpatialAttribute<2>* a =
                       (StandardSpatialAttribute<2>*) args[0].addr;
    StandardSpatialAttribute<2>* b =
                       (StandardSpatialAttribute<2>*) args[1].addr;
    result = qp->ResultStorage(s);
    DRM* res = (DRM*) result.addr;
    res->computeFrom(*a,*b);
    return 0;
}

/*

Specification

*/
OperatorSpec computeDRMSpec (
    " SPATIAL2D x SPATIAL2D -> drm",
    " computeDRM(_,_)",
    " Computes the direction relation matrix for two "
    " spatial objects.",
    " query computeDRM(BGrenzenline, mehringdamm)  "
  );

/*
Operator instance

*/
Operator computeDRM(
   "computeDRM",
   computeDRMSpec.getStr(),
   computeDRMVM,
   Operator::SimpleSelect,
   computeDRMTM
);


/*
Operator ~computeOIM~

Computes the objects interaction matrix for 2 elements in Spatial2D.

*/

ListExpr computeOIMTM(ListExpr args){
  string err="SPATIAL2D x SPATIAL2D expected"; 
  if(!nl->HasLength(args,2)){
      return listutils::typeError(err);
  }
  if(!listutils::isKind(nl->First(args),Kind::SPATIAL2D()) ||
     !listutils::isKind(nl->Second(args),Kind::SPATIAL2D() )){
      return listutils::typeError(err);
  }
  return listutils::basicSymbol<OIM>();    
}


/*
Value Mapping

*/

int computeOIMVM(Word* args, Word& result, int message, Word& local,
               Supplier s ){

    StandardSpatialAttribute<2>* a =
                       (StandardSpatialAttribute<2>*) args[0].addr;
    StandardSpatialAttribute<2>* b =
                       (StandardSpatialAttribute<2>*) args[1].addr;
    result = qp->ResultStorage(s);
    OIM* res = (OIM*) result.addr;
    res->computeFrom(*a,*b);
    return 0;
}

/*

Specification

*/
OperatorSpec computeOIMSpec (
    " SPATIAL2D x SPATIAL2D -> oim",
    " computeOIM(_,_)",
    " Computes the objects interaction matrix for two "
    " spatial objects.",
    " query computeOIM(BGrenzenline, mehringdamm)  "
  );

/*
Operator instance

*/
Operator computeOIM(
   "computeOIM",
   computeOIMSpec.getStr(),
   computeOIMVM,
   Operator::SimpleSelect,
   computeOIMTM
);

/*
Operator collectDline


Type Mapping

Signature: stream(point) x bool -> dline
           stream(line)  x bool -> dline
           stream(dline) x bool -> dline

*/
ListExpr collectDlineTM(ListExpr args){
   string err = " stream(X) [x bool] expected, x in {point,line,dline) ";
   if(!nl->HasLength(args,1) && !nl->HasLength(args,2)){
      return listutils::typeError(err);
   }
   if(nl->HasLength(args,2) && !CcBool::checkType(nl->Second(args))){
      return listutils::typeError(err);
   }
   ListExpr st = nl->First(args);
   if( !Stream<Point>::checkType(st)
       && !Stream<Line>::checkType(st) 
       && !Stream<DLine>::checkType(st)){
      return listutils::typeError(err);
   }
   if(nl->HasLength(args,2)){
       return listutils::basicSymbol<DLine>();
   } else {
      return nl->ThreeElemList(
                 nl->SymbolAtom(Symbols::APPEND()),
                 nl->OneElemList(nl->BoolAtom(true)),
                 listutils::basicSymbol<DLine>());
   }
}



int collectDlineVMpoint(Word* args, Word& result, int message, Word& local,
               Supplier s ){

    Stream<Point> stream(args[0]);
    bool stopUndef = false;
    CcBool* ignoreUndef= (CcBool*) args[1].addr;
    stopUndef = ignoreUndef->IsDefined() && !ignoreUndef->GetBoolval();
    result = qp->ResultStorage(s);
    DLine* res = (DLine*) result.addr;
    res->clear();
    res->SetDefined(true);
  
     stream.open();

     Point* p = 0;
     Point* lastP = 0;

     while( (p = stream.request()) ){
       if(!p->IsDefined()){
          if(stopUndef){
            if(lastP){
              lastP->DeleteIfAllowed();
              lastP=0;
            }
          }
          p->DeleteIfAllowed();
          p=0;
          
        } else {
            if(lastP){
                SimpleSegment s(lastP->GetX(), lastP->GetY(), 
                                p->GetX(), p->GetY());
                res->append(s);
                lastP->DeleteIfAllowed();
            }
            lastP = p;
        }
     }
     if(lastP){
        lastP->DeleteIfAllowed();
     }
     stream.close();  
     return 0;
}


int collectDlineVMline(Word* args, Word& result, int message, Word& local,
               Supplier s ){

    Stream<Line> stream(args[0]);
    bool stopUndef = false;
    CcBool* ignoreUndef= (CcBool*) args[1].addr;
    stopUndef = ignoreUndef->IsDefined() && !ignoreUndef->GetBoolval();
    result = qp->ResultStorage(s);
    DLine* res = (DLine*) result.addr;
    res->clear();
    res->SetDefined(true);
  
    stream.open();
    Line* line;
    while((line=stream.request())){
       if(!line->IsDefined()){
         if(stopUndef){
           res->clear();
           res->SetDefined(false);
           line->DeleteIfAllowed();
           stream.close();
           return 0; 
         } else {
           line->DeleteIfAllowed();
         }
       } else {
         HalfSegment hs;
         for(int i=0;i<line->Size();i++){
            line->Get(i,hs);
            if(hs.IsLeftDomPoint()){
              SimpleSegment s(hs);
              res->append(s);
            }
         }
         line->DeleteIfAllowed();
       }
    }
    stream.close();
    return 0;
}

int collectDlineVMdline(Word* args, Word& result, int message, Word& local,
               Supplier s ){

    Stream<DLine> stream(args[0]);
    bool stopUndef = false;
    CcBool* ignoreUndef= (CcBool*) args[1].addr;
    stopUndef = ignoreUndef->IsDefined() && !ignoreUndef->GetBoolval();
    result = qp->ResultStorage(s);
    DLine* res = (DLine*) result.addr;
    res->clear();
    res->SetDefined(true);
  
    stream.open();
    DLine* line;
    while((line=stream.request())){
       if(!line->IsDefined()){
         if(stopUndef){
           res->clear();
           res->SetDefined(false);
           line->DeleteIfAllowed();
           stream.close();
           return 0; 
         } else {
           line->DeleteIfAllowed();
         }
       } else {
         SimpleSegment s;
         for(int i=0;i<line->getSize();i++){
            line->get(i,s);
            res->append(s);
         }
         line->DeleteIfAllowed();
       }
    }
    stream.close();
    return 0;
}

/*
ValueMapping Array and Select function

*/

ValueMapping collectDlineVM[] ={
    collectDlineVMpoint,
    collectDlineVMline,
    collectDlineVMdline,
 };

int collectDlineSelect(ListExpr args){
  ListExpr sa = nl->Second(nl->First(args));
  if(Point::checkType(sa)) return 0;
  if(Line::checkType(sa)) return 1;
  if(DLine::checkType(sa)) return 2;
  return -1;
}


/*

Specification

*/
OperatorSpec collectDlineSpec (
    " stream(X) x bool -> dline, X in {point,line,dline)",
    " _ collectDline[_]",
    " Collects the stream elements into a single dline value"
    " If the boolean argument is true, undefined value in"
    " the stream are ignored, otherwise the result is undefined if"
    " there is any undefined stream element",
    " query strassen feed projecttransformstream[Geodata] collectDline[TRUE] "
  );

/*
Operator instance

*/

Operator collectDline(
   "collectDline",
   collectDlineSpec.getStr(),
   3,
   collectDlineVM,
   collectDlineSelect,
   collectDlineTM);


/*
40 Operator ~computeLabel~

40.1 Type Mapping

*/
ListExpr computeLabelTM(ListExpr args){
  string err = "line x string expected";
  if(!nl->HasLength(args,2)){
    return listutils::typeError(err);
  }
  if(   !Line::checkType(nl->First(args)) 
     || !CcString::checkType(nl->Second(args))){
     return listutils::typeError(err);
  }
  return listutils::basicSymbol<Label>();
}


/*
40.2 Value Mapping

*/

int computeLabelVM(Word* args, Word& result, int message, Word& local,
                   Supplier s ){
  Line* line = (Line*) args[0].addr;
  CcString* lab = (CcString*) args[1].addr;
  result = qp->ResultStorage(s);
  Label* res = (Label*) result.addr;
  if(!line->IsDefined() || !lab->IsDefined()){
    res->SetDefined(false);
    return 0;
  }
  if(line->Size()==0){
     res->set(lab->GetValue(),0,0,0);
  } 
  Rectangle<2> r = line->BoundingBox();
  double x = (r.MinD(0) + r.MaxD(0))  / 2;
  double y = (r.MinD(1) + r.MaxD(1)) / 2;
  Point p(true,x,y);

  HalfSegment hs;
  HalfSegment minHs;
  double distance;
  line->Get(0,hs);
  minHs = hs;
  distance = hs.Distance(p);
  // determine the halfsegment located near of the bbox center
  for(int i=1;i<line->Size();i++){
      line->Get(i,hs);
      if(hs.IsLeftDomPoint()){
          double dist = hs.Distance(p);
          if(dist < distance){
            distance = dist;
            minHs = hs;
          }
      }
  }
  p = minHs.GetLeftPoint();
  Point p2 = minHs.GetRightPoint();
  double dx = p2.GetX() - p.GetX();
  double dy = p2.GetY() - p.GetY();
  double ang;
  if(AlmostEqual(dy,0)){
     ang = 0;
  } else if(AlmostEqual(dx,0)){
     ang = 90;
  } else {
    double len = sqrt(dx*dx+dy*dy);
    ang = acos(dy/len) * 180 / M_PI;
    cout << "Name " << lab->GetValue() << endl;
    cout << "Angle 1 = " << ang << endl;
    ang = ang -90;
    if(ang<-90){
      ang = ang + 180;
    }
  }
  res->set(lab->GetValue(), p.GetX() + dx/3, p.GetY()+dy/3, ang);
  return 0;
}


/*

40.3 Specification

*/
OperatorSpec computeLabelSpec (
    " line  x string -> spatiallabel",
    " computeLabel(line, name)",
    " Computes a label for a line",
    " query computeLabel(BGrenzenLine,\"boundary\") "
  );

/*
40.4 Operator instance

*/

Operator computeLabelOP(
   "computeLabel",
   computeLabelSpec.getStr(),
   computeLabelVM,
   Operator::SimpleSelect,
   computeLabelTM
);

/*
41 Operator centroidDisc

*/
ListExpr centroidDiscTM(ListExpr args){
   string err = "points [x geoid]expected";
   if(!nl->HasLength(args,1) && !nl->HasLength(args,2)){
      return listutils::typeError(err);
   }
   if(!Points::checkType(nl->First(args))){
     return listutils::typeError(err);
   }
   if(nl->HasLength(args,2) && !Geoid::checkType(nl->Second(args))){
     return listutils::typeError(err);
   }
   return listutils::basicSymbol<Disc>();
}


int centroidDiscVM(Word* args, Word& result, int message, Word& local,
                   Supplier s ){

  result = qp->ResultStorage(s);
  Points* arg = (Points*) args[0].addr;
  Disc* res = (Disc*) result.addr;
  if(!arg->IsDefined()){
      res->SetDefined(false);
      return 0;
  }
  Geoid* geoid = 0;
  if(qp->GetNoSons(s) ==2){
     geoid = (Geoid*) args[1].addr;
     if(!geoid->IsDefined()){
       res->SetDefined(false);
       return 0;
     }
  }
  int size = arg->Size();
  if(size==0){
      res->SetDefined(false);
      return 0;
  }
  Point center = arg->theCenter();
  double rad =0;
  Point p(true);
  for(int i=0;i<size;i++){
     arg->Get(i,p);
     double d = center.Distance(p,geoid);
     if(d>rad){
        rad = d;
     }
  }
  res->set(center.GetX(),center.GetY(),rad);
  return 0;
}

/*

41.3 Specification

*/
OperatorSpec centroidDiscSpec (
    " points [x geoid] -> disc",
    " centroidDisc(_,_)",
    " Computes a disc enclosing all points from the argument "
    "with the centroid of the points as center",
    " query centroidDisc(train7stations) "
  );

/*
40.4 Operator instance

*/

Operator centroidDiscOP(
   "centroidDisc",
   centroidDiscSpec.getStr(),
   centroidDiscVM,
   Operator::SimpleSelect,
   centroidDiscTM
);


/*
Operator calcDisc

*/
ListExpr calcDiscTM(ListExpr args){
   string err = "points expected";
   if(!nl->HasLength(args,1)){
     return listutils::typeError(err);
   }
   if(!Points::checkType(nl->First(args))){
     return listutils::typeError(err);
   }
   return listutils::basicSymbol<Disc>();
}


/*
Implementation from:

Smallest enclosing disks (balls and ellipsoids), Emo Welzl, 1991

*/
Disc getDisc(const vector<Point>& R){
      switch(R.size()){
         case 0 : {Disc d(false); return d;}
         case 1 : {Disc d(R[0]); return d;}
         case 2 : {Disc d(R[0],R[1]); 
                    return d;
                   }
                    
         case 3 : { double d1 = R[0].Distance(R[1]);
                    double d2 = R[0].Distance(R[2]);
                    double d3 = R[1].Distance(R[2]);
                    Disc d (false);
                    if(d1 > d2 && d1 > d3){
                      d = Disc(R[0],R[1]);
                    } else if(d2>d3){
                       d = Disc(R[0],R[2]);
                    } else {
                       d = Disc(R[1],R[2]);
                    }
                    if(!d.contains(R[0]) ||
                       !d.contains(R[1]) ||
                       !d.contains(R[2])){
                       d = Disc(R[0],R[1],R[2]);
                    }
                    return d;
                  }
         default: assert(false);
      }
}


Disc getDisc(const Point* R, const int Rsize){
      switch(Rsize){
         case 0 : {Disc d(false); return d;}
         case 1 : {Disc d(R[0]); return d;}
         case 2 : {Disc d(R[0],R[1]); 
                    return d;
                   }
                    
         case 3 : { double d1 = R[0].Distance(R[1]);
                    double d2 = R[0].Distance(R[2]);
                    double d3 = R[1].Distance(R[2]);
                    Disc d (false);
                    if(d1 > d2 && d1 > d3){
                      d = Disc(R[0],R[1]);
                    } else if(d2>d3){
                       d = Disc(R[0],R[2]);
                    } else {
                       d = Disc(R[1],R[2]);
                    }
                    if(!d.contains(R[0]) ||
                       !d.contains(R[1]) ||
                       !d.contains(R[2])){
                       d = Disc(R[0],R[1],R[2]);
                    }
                    return d;
                  }
         default: assert(false);
      }
}

 /** originally, recursive method to compute the smallest enclosing disc **/
 Disc sed(Points& P, int pos, vector<Point> R){
    if(pos==P.Size() || R.size()==3){
       return getDisc(R);
    }
    Point p(true);
    P.Get(pos,p);
    Disc d = sed(P,pos+1,R);
    if(!d.contains(p)){
      R.push_back(p);
      d = sed(P,pos+1,R);
    }
    return d;
  }

struct sedEntry{

   sedEntry(const int _pos, 
            const Point _R[3], 
            int _Rsize,
            const bool _cond):
     pos(_pos),cond(_cond),Rsize(_Rsize),
     R{_R[0],_R[1],_R[2]}{
     for(int i=0;i<Rsize;i++){
       R[i] = _R[i];
     }
   }

   void append(const Point& p){
     R[Rsize] = p;
     Rsize++;
   }
   
   int pos;
   bool cond;
   int Rsize;
   Point R[3];
};

ostream& operator<<( ostream& o,const sedEntry& e){
  o << e.pos << ", " << e.Rsize;
  return o;
}


 // stack based variant of sed
Disc sedSt(Points& P){
   int size = P.Size();
   if(size==0){
      return Disc(false);
   }

   Disc d(false);

   //stack<sedEntry> st;
   Stack<sedEntry> st;
   Point initial[] = {Point(false),Point(false),Point(false)};
   sedEntry f1(0,initial,3,true);
   f1.Rsize=0;
   st.push(f1);
   sedEntry f2(0,initial,3,false); 
   f2.Rsize = 0;
   st.push(f2);

   int* shuffle = new int[size];
   for(int i=0;i<size;i++){
      shuffle[i] = i; 
   }
   for(int i=0;i<size;i++){
      int tmp = shuffle[i];
      int p = rand() % size;
      shuffle[i] = shuffle[p];
      shuffle[p] = tmp;
   }
   
   while(!st.isEmpty()){
     const sedEntry e = st.pop();
     if(e.pos==size || e.Rsize==3){
        d = getDisc(e.R,e.Rsize);
     } else {
       if(!e.cond){
          sedEntry e1(e.pos+1,e.R,e.Rsize,true);
          sedEntry e2(e.pos+1,e.R,e.Rsize,false);
          st.push(e1);
          st.push(e2);
       } else {
          int pos = e.pos;
          Point p(true);
          P.Get(shuffle[pos],p);
          if(!d.contains(p)){
              sedEntry e1(e.pos+1,e.R,e.Rsize,true);
              sedEntry e2(e.pos+1,e.R,e.Rsize,false);
              e1.append(p);
              e2.append(p);
              st.push(e1);
              st.push(e2);
          }
       }
     }
   }
   delete[] shuffle;
   return d;
}



int calcDiscVM(Word* args, Word& result, int message, Word& local,
                   Supplier s ){

  Points* ps = (Points*) args[0].addr;
  result = qp->ResultStorage(s);
  Disc* res = (Disc*) result.addr;
  if(!ps->IsDefined() || ps->IsEmpty()){
     res->SetDefined(false);
     return 0;
  }
  Disc d = sedSt(*ps);
  *res = d; 
  return 0;
}

OperatorSpec calcDiscSpec (
    " points  -> disc",
    " calcDisc(_)",
    " Computes a the minimum disc enclosing all points from the argument ",
    " query caclcDisc(train7stations) "
  );

/*
40.4 Operator instance

*/

Operator calcDiscOP(
   "calcDisc",
   calcDiscSpec.getStr(),
   calcDiscVM,
   Operator::SimpleSelect,
   calcDiscTM
);




/*
42 Operator createDisc

*/
ListExpr createDiscTM(ListExpr args){
  string err = "point [x point [x point]] expected";
  if(    !nl->HasLength(args,1) && !nl->HasLength(args,2) 
      && !nl->HasLength(args,3)){
    return listutils::typeError(err);
  }
  while(!nl->IsEmpty(args)){
    if(!Point::checkType(nl->First(args))){
       return listutils::typeError(err);
    }
    args = nl->Rest(args);
  }
  return listutils::basicSymbol<Disc>();
}


int createDiscVM(Word* args, Word& result, int message, Word& local,
                   Supplier s ){

   result = qp->ResultStorage(s);
   Disc* res = (Disc*) result.addr;
   switch(qp->GetNoSons(s)){
      case 1: { Disc d(*((Point*)args[0].addr));
                *res = d;
                return 0;
              }
      case 2: { Disc d( *((Point*)args[0].addr),
                        *((Point*)args[1].addr));
                *res = d;
                return 0;
              }
      case 3: { Disc d(*((Point*)args[0].addr),
                       *((Point*)args[1].addr),
                       *((Point*)args[2].addr));
                *res = d;
                return 0;
              }
      default: assert(false);
   }
   return -1;
}

OperatorSpec createDiscSpec (
    " point [x point [ x point]]  -> disc",
    " createDisc(_,_,_)",
    " Computes a the minimum disc enclosing the arguments ",
    " query createDisc( [const point value (8 7)]) "
  );

/*
40.4 Operator instance

*/

Operator createDiscOP(
   "createDisc",
   createDiscSpec.getStr(),
   createDiscVM,
   Operator::SimpleSelect,
   createDiscTM
);

/*
\section{Operator berlin2wgs}

Converts a pair of coordinates from bbbike / BerlinMOD format into WGS84

\subsection{Type Mapping}

*/
ListExpr berlin2wgsTM(ListExpr args) {
  if (!nl->HasLength(args, 1)) {
    return listutils::typeError("Exactly one argument expected.");
  }
  if (Point::checkType(nl->First(args)) || Points::checkType(nl->First(args)) ||
      Line::checkType(nl->First(args)) || Region::checkType(nl->First(args))) {
    return nl->First(args);
  }
  return listutils::typeError("Type point, points, line, or region expected.");
}

int berlin2wgsSelect(ListExpr args) {
  if (Point::checkType(nl->First(args)))  return 0;
  if (Points::checkType(nl->First(args))) return 1;
  if (Line::checkType(nl->First(args)))   return 2;
  if (Region::checkType(nl->First(args))) return 3;
  return -1;
}

template<class T>
int berlin2wgsVM(Word* args, Word& result, int message, Word& local,Supplier s){
  T* src = (T*)args[0].addr;
  result = qp->ResultStorage(s);
  T* res = (T*)result.addr;
  if (src->IsDefined()) {
    Berlin2WGS converter;
    converter.convert(src, res);
  }
  else {
    res->SetDefined(false);
  }
  return 0;
}

OperatorSpec berlin2wgsSpec(
  " T -> T, where T in {point, points, line, region}",
  " berlin2wgs( _ )",
  " Converts coordinates from bbbike/BerlinMOD format into WGS84 coordinates.",
  " query berlin2wgs([const point value (13132, 10876)])"
);

ValueMapping berlin2wgsVMs[] = {
  berlin2wgsVM<Point>,
  berlin2wgsVM<Points>,
  berlin2wgsVM<Line>,
  berlin2wgsVM<Region>
};

/*
\subsection{Operator instance}

*/
Operator berlin2wgs(
  "berlin2wgs",
  berlin2wgsSpec.getStr(),
  4,
  berlin2wgsVMs,
  berlin2wgsSelect,
  berlin2wgsTM
);



/*
10.132 Operator ~elements~

*/
ListExpr elementsTM(ListExpr args){
  string err="points expected";
  if(  !nl->HasLength(args,1) ||
       !Points::checkType(nl->First(args))){
    return listutils::typeError(err);
  }
  return nl->TwoElemList( listutils::basicSymbol<Stream<Point> >(),
                          listutils::basicSymbol<Point>());
}

class elementsInfo{
  public:
     elementsInfo(Points* _ps): ps(_ps), pos(0),max(0),tmp(0,0){
        max = ps->Size();
     }

     Point* next(){
        if(pos>=max){
           return 0;
        }
        ps->Get(pos,tmp);
        pos++;
        return new Point(tmp);
     }

   private:
      Points* ps;
      size_t pos;
      size_t max;
      Point tmp; 
}; 

int elementsVM(Word* args, Word& result, int message, Word& local,Supplier s){
   elementsInfo* li = (elementsInfo*) local.addr;
   switch(message){
      case OPEN:  {
            if(li){
               delete li;
               local.addr = 0;
             }
             Points* ps = (Points*) args[0].addr;
             if(ps->IsDefined()){
                local.addr = new elementsInfo(ps);
             }
             return 0;
      }
      case REQUEST:
             result.addr=li?li->next():0;
             return result.addr?YIELD:CANCEL;
      case CLOSE :
             if(li){
                delete li;
                local.addr = 0;
             }     
   }
   return -1;
}


OperatorSpec elementsSpec(
  " points -> stream(point)",
  " elements( _ )",
  " Puts the elements of a points object into a stream.",
  " query elements(train7stations) count"
);


Operator elementsOP(
  "elements",
  elementsSpec.getStr(),
  elementsVM,
  Operator::SimpleSelect,
  elementsTM
);


/*
Operator  twist

Creates a dline.

*/
ListExpr twistTM(ListExpr args){
  string err = "{real,int} x int x int expected";
  if(!nl->HasLength(args,3)){
     return listutils::typeError(err);
  }
  if(   (   !CcReal::checkType(nl->First(args))
          &&!CcInt::checkType(nl->First(args)))
     || !CcInt::checkType(nl->Second(args))
     || !CcInt::checkType(nl->Third(args))){
     return listutils::typeError(err);
  }
  return listutils::basicSymbol<DLine>();
}

template<class S>
int twistVMT(Word* args, Word& result, int message, Word& local,Supplier s){
   result = qp->ResultStorage(s);
   DLine* res = (DLine*) result.addr;
   S* size = (S*) args[0].addr;
   CcInt* corners = (CcInt*) args[1].addr;
   CcInt* offset = (CcInt*) args[2].addr;

   if(!size->IsDefined() || !corners->IsDefined() || !offset->IsDefined()){
      res->SetDefined(false);
      return 0;
   }
   double si = size->GetValue();
   int c = corners->GetValue();
   int o = offset->GetValue();
   if(si<=0 || c<1 || o % c == 0){
      res->SetDefined(false);
   } 
   res->clear();
   int i = 0;
   double a = 360.0 / c;
   a = (a * 3.14159265359) / 180;
   double x1 = si * sin((((i*o)%c)*a));
   double y1 = si * cos((((i*o)%c)*a));
   i++;
   while(i <= c){
     double x2 = si * sin((((i*o)%c)*a));
     double y2 = si * cos((((i*o)%c*a)));
     res->append(SimpleSegment(x1,y1,x2,y2));
     x1 = x2;
     y1 = y2;
     i++; 
   }
   return 0;
}

ValueMapping twistVM[] = {
    twistVMT<CcInt>,
    twistVMT<CcReal>
};

int twistSelect(ListExpr args){
  return CcInt::checkType(nl->First(args))?0:1;
}


OperatorSpec twistSpec(
  " {real,int} x int x int -> dline",
  " twist(size, corners, offset)",
  " Creates a nice  dline. ",
  " query twist(40.0,9,4)"
);

Operator twistOp(
  "twist",
  twistSpec.getStr(),
  2,
  twistVM,
  twistSelect,
  twistTM
);


/*
Operator creating the contour of a dline value.


*/

enum join_style{join_none,join_bevel, join_miter};

class Contour2{

 public:
    Contour2(DLine* line, double w, join_style _join, bool _closed ){
       plpos = 0;
       clpos = 0;
       closed = _closed;
       width=w;
       join = _join;
       maxMiter = 3*w;
       fillPolylines(line);
    }
    

    Contour2(Line* line, double w, join_style _join, bool _closed ){
       plpos = 0;
       clpos = 0;
       closed = _closed;
       width=w;
       join = _join;
       maxMiter = 3*w;
       fillPolylines(line);
    }

   // by contruction, the interior of the region 
   // is always left to the directed segments
    vector<SimplePoint>* next(){
       while(plpos<polylines.size()){
          vector<SimplePoint>& pl = polylines[plpos];
          size_t end = pl.size()-1;
          if(clpos==end){ // start new polyline
             plpos++;
             clpos=0;
          } else {
             bool left_end = !closed && (clpos==0);
             bool right_end = !closed && (clpos==end-1);
             int o = closed?(clpos==end-1?1:0):0;
             vector<SimplePoint>* res = getContour(pl[clpos], 
                                                   pl[(clpos+1) % pl.size()],
                                                   pl[(clpos+2+o) % pl.size()],
                                                   left_end, right_end);
             clpos++;
             if(res){
               return res;
             }
          }
       }
       return 0;
    }



 private:
    vector<vector<SimplePoint> > polylines;
    size_t plpos; // position in polylines
    size_t clpos; // position in current polyline
    bool closed;
    double width;
    join_style join;
    double maxMiter;
   
    enum direction{left, right, forward};


    void fillPolylines(DLine* line){
        SimpleSegment s;
        for(int i=0;i<line->Size();i++){
           line->get(i,s);
           addSegment(s);  
         }
         if(    (polylines.size()>0) && closed 
             && (polylines.back()[0] != polylines.back().back())){
           polylines.back().push_back(polylines.back()[0]);
         }
     }


     void fillPolylines(const Line* line){
         int size = line->Size();
         bool* used = new bool[size];
         memset(used,0, size*sizeof(bool));
         for(int i=0;i< size;i++){
             if(!used[i]){
                  HalfSegment hs;
                  line->Get(i,hs);
                  if(hs.IsLeftDomPoint()){
                     addPolyLine(line, i, used);
                  }
             }
         }
         delete[] used;
     }

     void addPolyLine(const Line* line, int pos, bool*& used){
        assert(!used[pos]);
        HalfSegment hs;
        line->Get(pos,hs);
        assert(hs.IsLeftDomPoint());
        used[pos] = true;
        vector<SimplePoint> pl;
        SimplePoint p1(hs.GetDomPoint().GetX(), hs.GetDomPoint().GetY());
        SimplePoint p2(hs.GetSecPoint().GetX(), hs.GetSecPoint().GetY());
        pl.push_back(p1);
        pl.push_back(p2);
        // extend polyline so far as possible 
        int pn = hs.attr.partnerno;
        used[pn] = true;
        line->Get(pn,hs);
        int next;
        while( (next = getExtension(line, pn, used, hs)) >=0){
           SimplePoint p(hs.GetSecPoint().GetX(), hs.GetSecPoint().GetY());
           pl.push_back(p);
           used[pn] = true;
           pn = hs.attr.partnerno;
           used[pn] = true;
           line->Get(pn,hs);
        }

        if(closed && pl[0] != pl.back() && pl.size()>1){
           pl.push_back(pl[0]);
        }
        if(pl.size()>1){
          polylines.push_back(pl);
        }

     }

     int getExtension(const Line* line, int &partner, 
                      bool* used, HalfSegment& hs){
          // extract the halfsegment
          line->Get(partner, hs);
          Point p(hs.GetDomPoint());
          int po = partner;
          // search at the left side for extension
          partner--;
          while( partner >= 0){
             line->Get(partner,hs);
             Point p2(hs.GetDomPoint());
             if(!AlmostEqual(p,p2)){
                partner = -1; // break
             } else {
                if(!used[partner]){ // extension foundA
                   return partner;
                }
             }
             partner--;
          } 
          // nothing found left, search right
          partner = po;
          partner++;
          while(partner < line->Size()){
             line->Get(partner,hs);
             Point p2(hs.GetDomPoint());
             if(!AlmostEqual(p,p2)){
                partner = line->Size(); // break
             } else {
                if(!used[partner]){ // extension found
                   return partner;
                }
             }
             partner ++;
          }
          return -1; // no extension found 
     }

     void addSegment(SimpleSegment& s){
        if(polylines.empty()){
           addFreshVector(s);
        } else {
           if(SimplePoint(s.x1,s.y1) != getLastPoint()){
             if(closed&&(polylines.back()[0] != polylines.back().back())){
               polylines.back().push_back(polylines.back()[0]);
             }
             addFreshVector(s);
           } else {
              polylines.back().push_back(SimplePoint(s.x2,s.y2)); 
           }
        }
     }

     void addFreshVector(SimpleSegment& s){
         vector<SimplePoint> v;
         v.push_back(SimplePoint(s.x1,s.y1));
         v.push_back(SimplePoint(s.x2,s.y2));
         polylines.push_back(v);
     }

     SimplePoint& getLastPoint(){
        return polylines.back().back();
     }

     vector<SimplePoint>* getContour(SimplePoint& p1, SimplePoint& p2, 
                              SimplePoint& p3, bool left_end, bool right_end){

          if(p1==p2){
            return 0;
          }

          vector<SimplePoint>* r = getRectangle(p1,p2); 
          if((join==join_none) || right_end || (p2==p3) ){
              return r;
          }
          direction dir = getDirection(p1,p2,p3);
          if(dir==forward){
            return r;
          }
          vector<SimplePoint>* r2 = getRectangle(p2,p3);
          
          if(join==join_bevel){
            if(dir==right){
               vector<SimplePoint>* res = new vector<SimplePoint>();
               res->push_back(r->at(0));
               res->push_back(r->at(1));
               res->push_back(p2);
               res->push_back(r2->at(3));
               res->push_back(r->at(2));
               res->push_back(r->at(3));
               delete r;
               delete r2;
               return res;
            } else {
               vector<SimplePoint>* res = new vector<SimplePoint>();
               res->push_back(r->at(0));
               res->push_back(r->at(1));
               res->push_back(r2->at(0));
               res->push_back(p2);
               res->push_back(r->at(2));
               res->push_back(r->at(3));
               delete r;
               delete r2;
               return res;
            }
          }
          if(join == join_miter){
            if(dir==right){
              SimplePoint ip = intersectionPoint(r->at(2), r->at(3), 
                                                r2->at(2), r2->at(3));
              if(dist(p2,ip) > maxMiter){
                  setDist(p2,ip,maxMiter);
              }
              vector<SimplePoint>* res = new vector<SimplePoint>();
              res->push_back(r->at(0)); 
              res->push_back(r->at(1));
              res->push_back(p2);
              res->push_back(r2->at(3));
              res->push_back(ip);
              res->push_back(r->at(2));
              res->push_back(r->at(3)); 
              delete r;
              delete r2;
              return res;
            } else {
               SimplePoint ip = intersectionPoint(r->at(0), r->at(1),
                                                r2->at(0), r2->at(1));
              if(dist(p2,ip) > maxMiter){
                  setDist(p2,ip,maxMiter);
              }
              vector<SimplePoint>* res = new vector<SimplePoint>();
              res->push_back(r->at(0));
              res->push_back(r->at(1));
              res->push_back(ip);
              res->push_back(r2->at(0));
              res->push_back(p2);
              res->push_back(r->at(2));
              res->push_back(r->at(3));
              delete r;
              delete r2;
              return res;
                 
            }
          }
          return 0;

      }

     vector<SimplePoint>* getRectangle(SimplePoint& p1,SimplePoint& p2){
           double x1 = p1.getX();
           double y1 = p1.getY();
           double x2 = p2.getX();
           double y2 = p2.getY();
           double dx = x2 - x1;
           double dy = y2 - y1; 
           double length = sqrt(dx*dx + dy*dy);
           double f = width / (2*length);
           dx = dx*f;
           dy = dy*f;
           vector<SimplePoint>* res = new vector<SimplePoint>();
           res->push_back(SimplePoint(x1 + dy, y1 - dx));
           res->push_back(SimplePoint(x2 + dy, y2 - dx));
           res->push_back(SimplePoint(x2 - dy, y2 + dx));
           res->push_back(SimplePoint(x1 - dy, y1 + dx));
           return res;
     }

    static direction getDirection(SimplePoint& point1, 
                                  SimplePoint& point2, 
                                  SimplePoint& point3){
       double p1 = point1.getX();
       double p2 = point1.getY();
       double q1 = point2.getX();
       double q2 = point2.getY();
       double r1 = point3.getX();
       double r2 = point3.getY();
       // directed triangle area
       double A = (r1-p1)*(r2+p2) + (q1-r1)*(q2+r2) + (p1-q1)*(p2+q2);
       if(AlmostEqual(A,0)){
         return forward;
       }
       return A>0?left: right;
    }

    static SimplePoint intersectionPoint(SimplePoint& p1, SimplePoint& p2,
                                         SimplePoint& p3, SimplePoint& p4){
     double u = p2.getX()- p1.getX();
     double v = p3.getX() - p4.getX();
     double w = p1.getX() - p3.getX();
     double x = p2.getY() - p1.getY();
     double y = p3.getY() - p4.getY();
     double z = p1.getY() - p3.getY();
     double k = y*u-v*x;
     if(k==0){  // segments are parallel
        assert(false); 
     }   
     double delta2 = (w*x-z*u) / k;
     double delta1;
     if(abs(u) > abs(x)){
        delta1 = -1*((w+delta2*v)/u);
     } else {
        delta1 = -1*((z+delta2*y)/x);
     }   
     double xp = p1.getX() + delta1*(p2.getX()-p1.getX());
     double yp = p1.getY() + delta1*(p2.getY() - p1.getY());
     // just for check
     return SimplePoint(xp,yp);
   }

   void setDist(const SimplePoint& p1, SimplePoint& p2, double dist){
       double dx = p2.getX() - p1.getX();
       double dy = p2.getY() - p1.getY();
       double l = sqrt(dx*dx + dy*dy);
       double delta = dist/l;
       p2.setX(p1.getX() + delta*dx);
       p2.setY(p1.getY() + delta*dy);
   }
   double dist(const SimplePoint& p1, const SimplePoint& p2){
       double dx = p2.getX() - p1.getX();
       double dy = p2.getY() - p2.getY();
       return sqrt(dx*dx + dy*dy);
   }



};

ListExpr contour2TM(ListExpr le){
   // to be extended by further arguments
   // currently only dline and width
   string err = "dline x real x int x bool expected";
   if(!nl->HasLength(le,4)){
     return listutils::typeError(err);
   }
   if(   (!DLine::checkType(nl->First(le)) && !Line::checkType(nl->First(le)))
       ||(    !CcReal::checkType(nl->Second(le)) 
           && !CcInt::checkType(nl->Second(le)))
       ||!CcInt::checkType(nl->Third(le))
       ||!CcBool::checkType(nl->Fourth(le))){
      return listutils::typeError(err);
   }
   return listutils::basicSymbol<Region>();
}

Region* buildRegion(vector<SimplePoint>& pl){
   Region* res = new Region(pl.size()*2);
   res->SetDefined(true);
   if(pl.size()<3){
     assert(false);
   } 
   res->StartBulkLoad();
   for(size_t i= 0; i<pl.size();i++){
      Point p1 = pl[i].getPoint();
      Point p2 = pl[(i+1) % pl.size()].getPoint();
      HalfSegment hs(true, p1,p2);
      hs.attr.faceno =0;
      hs.attr.cycleno=0;
      hs.attr.edgeno = i;
      hs.attr.coverageno = 0;
      hs.attr.partnerno = i;
      hs.attr.insideAbove = p1 < p2;
      (*res) += hs;
      hs.SetLeftDomPoint(false);
      (*res) += hs;
   }
   res->EndBulkLoad(true,true,true,false);
   return res;
}


template<class Li, class Wi>
int contour2VMT(Word* args, Word& result, int message, Word& local,Supplier s){
   result = qp->ResultStorage(s);
   Region* res = (Region*) result.addr;
   res->Clear();
   Li*  L = (Li*) args[0].addr;
   Wi* W = (Wi*) args[1].addr;
   CcInt* J = (CcInt*) args[2].addr;
   CcBool* C = (CcBool*) args[3].addr;

   if(!L->IsDefined() || !W->IsDefined() || !J->IsDefined() || !C->IsDefined()){
      res->SetDefined(false);
      return 0;    
   } 
   double w = W->GetValue();
   if(w<=0){
        res->Clear();
        return 0;
   }
   vector<SimplePoint>* poly;
   join_style join = join_none;
   switch(J->GetValue()){
      case 0 : join = join_none; break;
      case 1 : join = join_bevel; break;
      case 2 : join = join_miter; break;
   }
   
   Contour2 Co(L,w, join, C->GetValue());
   
   res->SetDefined(true);
   
   stack<pair<Region*,int> > st; 
   while( (poly= Co.next())){
      Region* reg = buildRegion(*poly);
      delete poly;
      pair<Region*, int> p(reg,1);
      while(!st.empty() &&st.top().second == p.second){
         pair<Region*,int> p2 = st.top();
         st.pop();
         Region* res = new Region(0);
         p.first->Union(*p2.first, *res);
         delete p.first;
         delete p2.first;
         p.first=res;
         p.second++;
      }
      st.push(p);
   }
   // now, we have to build the union of the stack elements 
   // in a linear way
   Region* reg1=0;
   Region* reg2=0;
   while(!st.empty()){
       pair<Region*, int> t = st.top();
       st.pop();
       if(reg1==0){
          reg1 = t.first;
       } else {
          if(!reg2){
            reg2 = new Region(0);
          }
          reg1->Union(*t.first, *reg2);
          delete t.first;
          swap(reg1,reg2);
       }
   } 
   if(reg1){
     res->CopyFrom(reg1);
     delete reg1;
   }
   if(reg2){
      delete reg2;
   }
   return 0;

}


OperatorSpec contour2Spec(
  "{dline,line} x {real,int} x int x bool -> region",
  "_ contour2 [_,_,_] ",
  "Adds a buffer around the segments of a dline."
  "The second argument specifies the half width of the buffer."
  "The third argument specifies the join style, currently implemented "
  "styles are: 1 : bevel join, 1 : miter join. All other number will "
  "lead to no join. The boolean argument specifies the closeness of the "
  "result." ,
  " query sl contour2[3.0]"
);


ValueMapping contour2VM[] = {
   contour2VMT<DLine, CcInt>,
   contour2VMT<DLine, CcReal>,
   contour2VMT<Line, CcInt>,
   contour2VMT<Line, CcReal>
};

int contour2Select(ListExpr args){
  int n1 = DLine::checkType(nl->First(args))?0:2;
  int n2 = CcInt::checkType(nl->Second(args))?0:1;
  return n1+n2;
}


Operator contour2Op(
     "contour2",
     contour2Spec.getStr(),
     4,
     contour2VM,
     contour2Select,
     contour2TM
);


/*

Operator twist2

*/
ListExpr twist2TM(ListExpr args){
  // 1st arg: dline providing the segments
  // 2nd arg: int, how often each segment is to divide
  // 3th arg: bool, close, connect the last with the first segment
  string err="dline x int x bool [x bool] expected";
  if(!nl->HasLength(args,3) && !nl->HasLength(args,4)){
    return listutils::typeError(err);
  }
  if(   !DLine::checkType(nl->First(args))
     || !CcInt::checkType(nl->Second(args))
     || !CcBool::checkType(nl->Third(args))){
    return listutils::typeError(err);
  }
  if(nl->HasLength(args,4)){
     if(!CcBool::checkType(nl->Fourth(args))){
        return listutils::typeError(err);
     }
     return listutils::basicSymbol<DLine>();
  }

  return nl->ThreeElemList(
               nl->SymbolAtom(Symbols::APPEND()),
               nl->OneElemList(nl->BoolAtom(false)),
               listutils::basicSymbol<DLine>());
}


void insertTwist(const SimpleSegment& s1, const SimpleSegment& s2, 
                 int num, DLine* res){

    double s1_x = s1.x1;
    double s1_y = s1.y1;

    double dx1 = s1.x2 - s1_x;
    double dy1 = s1.y2 - s1_y;
  
    double s2_x = s2.x1;
    double s2_y = s2.y1;

    double dx2 = s2.x2 - s2_x;
    double dy2 = s2.y2 - s2_y;

    for(int i=0; i<=num;i++){
       SimplePoint p1( s1_x + (i*dx1) / num, s1_y + (i*dy1)/num);
       SimplePoint p2( s2_x + (i*dx2) / num, s2_y + (i*dy2)/num);
       if(p1!=p2){
          res->append(SimpleSegment(p1.getX(), p1.getY(),p2.getX(), p2.getY()));
       }
    }


}

vector<SimplePoint> computeSLTwist(vector<SimpleSegment>& segments, int num){

  vector<SimplePoint> res;
  for(int i=0; i<=num; i++){
     for(size_t sn=0; sn<segments.size();sn++){
        SimpleSegment s = segments[sn];
        double x = s.x1 + i*(s.x2-s.x1)/num;
        double y = s.y1 + i*(s.y2-s.y1)/num;
        res.push_back(SimplePoint(x,y));
     }
  }
  return res;
}

int twist2VM(Word* args, Word& result, int message, Word& local,Supplier s){
   result = qp->ResultStorage(s);
   DLine* res = (DLine*) result.addr;
   DLine* line = (DLine*) args[0].addr;
   CcInt* num = (CcInt*) args[1].addr;
   CcBool* closed = (CcBool*) args[2].addr;
   CcBool* singleLine = (CcBool*) args[3].addr;
   res->clear();
   // undefined argument
   if(   !line->IsDefined() || !num->IsDefined() || !closed->IsDefined()
      || !singleLine->IsDefined()){
      res->SetDefined(false);
      return 0;
   }
   // invalid sized
   if(line->getSize()<2 || num->GetValue()<2){
      res->clear();
      return 0;
   }

   if(!singleLine->GetValue()){
     // simple lines
     int e = closed->GetValue()?line->getSize():line->getSize()-1;
     res->resize(num->GetValue()*e);
     SimpleSegment s1;
     SimpleSegment s2;
     for(int i=0;i<e;i++){
        line->get(i,s1);
        line->get((i+1)%line->Size(), s2);
        insertTwist(s1,s2,num->GetValue(),res);
     }
     return 0;
   } else {
     vector<SimpleSegment> v;
     for(int i=0;i<line->getSize(); i++){
        SimpleSegment s;
        line->get(i,s);
        v.push_back(s);
     }
     vector<SimplePoint> pl = computeSLTwist(v,num->GetValue());
     res->clear();
     if(pl.size()<2){
         return 0;
     }
     for(size_t i=0;i<pl.size()-1; i++){
         res->append(SimpleSegment(pl[i].getX(), pl[i].getY(),
                                   pl[i+1].getX(), pl[i+1].getY()));
     }
     return 0;
   }
}

OperatorSpec twist2Spec(
  "dline x int x bool [x bool]",
  "twist2(line,num,closed[,singleLine)) ",
  "Connects consecutive segments in the dline by a set of segments."
  "Each segment of the dline is divided into num parts."
  "If the closed argument is set to true, also the last segment of "
  "the dline is connected with the first one. If the singleLine argument "
  "is true, the resulting dline will consist of a single polyline. This "
  "implies also the closed shape independly of the given value. ",
  "query twist2(dl, 10, TRUE)"
);


Operator twist2Op(
     "twist2",
     twist2Spec.getStr(),
     twist2VM,
     Operator::SimpleSelect,
     twist2TM
);


/*
twist 3

*/
ListExpr twist3TM(ListExpr args){
  string err= "dline x double expected";
  if(!nl->HasLength(args,2)){
    return listutils::typeError(err);
  }
  if(!DLine::checkType(nl->First(args))){
    return listutils::typeError(err);
  }  
  if(   !CcReal::checkType(nl->Second(args))
     && !CcInt::checkType(nl->Second(args))){
    return listutils::typeError(err);
  }
  return listutils::basicSymbol<DLine>();

}

template<class D>
int twist3VMT(Word* args, Word& result, int message, Word& local,Supplier s){
  DLine* line = (DLine*) args[0].addr;
  D* dist = (D*) args[1].addr;
  result = qp->ResultStorage(s);
  DLine* res = (DLine*) result.addr;

  if(!line->IsDefined() || !dist->IsDefined()){
    res->SetDefined(false);
    return  0;
  }
  res->clear();
  queue<SimpleSegment> q;
  for(int i=0;i<line->Size(); i++){
     SimpleSegment s;
     line->get(i,s);
     res->append(s);
     q.push(s);
  }
  double d = dist->GetValue();
  if(q.size()<2 || d<=0){
    return 0;
  }
  SimpleSegment first = q.front();
  SimplePoint cp(first.x1,first.y1);
  q.pop();
  first = q.front();

  double length;
  while( (length=first.length()) > d){
      double delta = d / length;
      SimplePoint np ( first.x1 + delta * (first.x2-first.x1),
                       first.y1 + delta * (first.y2-first.y1));
      SimpleSegment s( cp.getX(), cp.getY(), np.getX(), np.getY());
      res->append(s);
      q.push(s);
      cp = np;
      q.pop();
      first = q.front();
  } 
  while(!q.empty()){
    res->append(q.front());
    q.pop();
  }
  return 0;
}

ValueMapping twist3VM[] = {
   twist3VMT<CcInt>,
   twist3VMT<CcReal>
};

int twist3Select(ListExpr args){
   return CcInt::checkType(nl->Second(args))?0:1;
}

OperatorSpec twist3Spec(
  "dline x double -> dline",
  "twist3(line,offset) ",
  "produces interesting patterns",
  "query twist2(dl, 10.0)"
);

Operator twist3Op(
   "twist3",
   twist3Spec.getStr(),
   2,
   twist3VM,
   twist3Select,
   twist3TM
);



/*
Operator toSVG

*/
ListExpr toSVGTM(ListExpr args){
   string err="dline expected";
   if(!nl->HasLength(args,1)){
     return listutils::typeError(err);
   }
   if(!DLine::checkType(nl->First(args))){
     return listutils::typeError(err);
   }
   return listutils::basicSymbol<FText>();
}

vector<vector<SimplePoint> > findPolylines(vector<SimpleSegment>& segments){
  // todo find non-simple extensions
  vector<vector<SimplePoint> > res;
  for(size_t i=0;i<segments.size(); i++){
     if(i==0 || res.back().back()!=SimplePoint(segments[i].x1, segments[i].y1)){
         vector<SimplePoint> v;
         v.push_back(SimplePoint(segments[i].x1, segments[i].y1));
         v.push_back(SimplePoint(segments[i].x2, segments[i].y2));
         res.push_back(v);
     } else {
         res.back().push_back(SimplePoint(segments[i].x2, segments[i].y2));
     }
  }
  return res;
}


string toSVG(vector<SimpleSegment>& segments, double w, double h){
   vector<vector<SimplePoint> > pls = findPolylines(segments);
   stringstream ss;
   ss << "<?xml version=\"1.0\"?>" << endl;
   ss << "<!DOCTYPE svg PUBLIC \"-//W3C//DTD SVG 1.1//EN\" "
      << "\"http://www.w3.org/Graphics/SVG/1.1/DTD/svg11.dtd\">" << endl;
   ss << "<svg version=\"1.1\" xmlns=\"http://www.w3.org/2000/svg\" "; 
   ss << "width=\""<< w <<"\" height=\""<<h<<"\"";
   ss << ">" << endl;
   for(size_t i=0;i<pls.size();i++){
     ss << "<path style=\"fill:none;stroke:black;stroke-linejoin=bevel\" d=\"";
     for(size_t p=0;p<pls[i].size();p++){
        ss << (p==0?"M":"L");
        ss << pls[i][p].getX() << " " << pls[i][p].getY() << " ";
     }
     ss << "\" />" << endl;
   }
   ss << "</svg>" << endl;
   return ss.str();
}

vector<SimpleSegment> getSVGSegments(DLine* line,double&w, double& h ){

   vector<SimpleSegment> res;
   double x1=0;
   double x2=0;
   double y1=0;
   double y2=0;
   for(int i=0;i<line->Size();i++){
      SimpleSegment s;
      line->get(i,s);
      if(i==0){
         x1 = min(s.x1,s.x2);
         x2 = max(s.x1,s.x2);
         y1 = min(s.y1,s.y2);
         y2 = max(s.y1,s.y2);
      }else {
         x1 = min(x1,min(s.x1,s.x2));
         x2 = max(x2,max(s.x1,s.x2));
         y1 = min(y1,min(s.y1,s.y2));
         y2 = max(y2,max(s.y1,s.y2));
      }
      res.push_back(s);
   }
   // move box to (0,0) and mirror the y-direction
   w = x2-x1;
   h = y2-y1;
   if(x1!=0 || y1!=0){
       for(size_t i=0;i<res.size();i++){
          SimpleSegment& s = res[i];
          s.x1 = s.x1 - x1;
          s.x2 = s.x2 - x1;
          s.y1 = h - (s.y1 - y1);
          s.y2 = h - (s.y2 - y1); 
       }
   }
   if(w==0) w=1;
   if(h==0) h=1;
   return res;

}


int toSVGVM(Word* args, Word& result, int message, Word& local,Supplier s){
  DLine* line = (DLine*) args[0].addr;
  result = qp->ResultStorage(s);
  FText* res = (FText*) result.addr;
  if(!line->IsDefined()){
    res->SetDefined(false);
    return 0;
  } 
  double w;
  double h;
  vector<SimpleSegment> segs = getSVGSegments(line,w , h);
  res->Set(true, toSVG(segs,w,h));
  return 0;
}


OperatorSpec toSVGSpec(
  "dline -> text",
  "toSVG(_)",
  "converts a dline into an svg representation",
  "query toSVG([const dline value ((0 0 10 10))] )"
);


Operator toSVGOp(
     "toSVG",
     toSVGSpec.getStr(),
     toSVGVM,
     Operator::SimpleSelect,
     toSVGTM
);


/*

Operator ~simpleProject~

This operator projects spatial objects using a very simple
projection method. Its just to test whether it works.

*/
ListExpr simpleProjectTM(ListExpr args){
  if(!nl->HasLength(args,1)){
    return listutils::typeError("wrong number of arguments (expected one)");
  }
  ListExpr a = nl->First(args);   

  if(Point::checkType(a)) {  return a;}
  if(Points::checkType(a)) {  return a;}
  if(DLine::checkType(a)) {  return a;}

  return listutils::typeError("unsupprted Type");


}
static double earthradius = 6378137.0;  // eath radius in meters
static double earthperimeter = 2.0*M_PI*earthradius;
static double earthfactor = earthperimeter/360;


bool isGeo(Rectangle<2>& r){
  if(r.MinD(0) < -180) return false;
  if(r.MaxD(0) > 180) return false;
  if(r.MinD(1) < -90) return false;
  if(r.MaxD(1) > 90) return false;
  return true; 
}


void simpleProjectFun( double& x, double& y){
  x = x * cos((M_PI*y)/180) * earthfactor;
  y = y * earthfactor;
}

void simpleProjectFun(Point* a, Point* r){
  double x = a->GetX();
  double y = a->GetY();
  simpleProjectFun(x,y);
  r->Set(true,x,y);
}


void simpleProjectFun(Points* a, Points* r){
   Point pa(true,0,0);
   Point pr(true,0,0);
   r->StartBulkLoad();
   for(int i=0;i<a->Size();i++){
      a->Get(i,pa);
      simpleProjectFun(&pa,&pr);
      (*r) += pr;
   }
   r->EndBulkLoad(true,false,true);
}


void simpleProjectFun(SimpleSegment* sa, SimpleSegment* sr){

    *sr = *sa;
     simpleProjectFun(sr->x1,sr->y1);
     simpleProjectFun(sr->x2,sr->y2);
}

void simpleProjectFun(DLine* a, DLine* r){
   SimpleSegment sa; 
   SimpleSegment sr;
   for(int i=0;i<a->Size();i++){
      a->get(i,sa);
      simpleProjectFun(&sa,&sr);
      r->append(sr);
   }
}

template<class A>
int simpleProjectVMT(Word* args, Word& result, int message, 
                    Word& local,Supplier s){
  A* arg = (A*) args[0].addr;
  result = qp->ResultStorage(s);
  A* res = (A*) result.addr;
  if(!arg->IsDefined()){
     res->SetDefined(false);
     return 0;
  } 
  res->Clear();
  Rectangle<2> bbox = arg->BoundingBox();
  if(!bbox.IsDefined()){
    // original object was empty
    return 0;
  } 
  if(!isGeo(bbox)){
    res->SetDefined(false);
    return 0;
  }
  simpleProjectFun(arg,res);
  return 0;
}


ValueMapping simpleProjectVM[] = {
   simpleProjectVMT<Point>,
   simpleProjectVMT<Points>,
   simpleProjectVMT<DLine>
};

int simpleProjectSelect(ListExpr args){
   ListExpr a = nl->First(args);
   if(Point::checkType(a)) return 0;
   if(Points::checkType(a)) return 1;
   if(DLine::checkType(a)) return 2;
   return -1;
}

OperatorSpec simpleProjectSpec(
   "SPATIAL -> SPATIAL",
   "simpleProject(_)",
   "Performs a simple projection to spatial objects",
   " query simpleProjection([const point value (7.475 ,51.359444)])"
);

Operator simpleProject(
   "simpleProject",
   simpleProjectSpec.getStr(),
   3,
   simpleProjectVM,
   simpleProjectSelect,
   simpleProjectTM
);


/*
Operator  ~todline~

Converts a line or a region value into a dline.

*/

ListExpr todlineTM(ListExpr args){
   if(!nl->HasLength(args,1)){
      return listutils::typeError("one argument expected");
   }
   if(  !Region::checkType(nl->First(args))
      &&!Line::checkType(nl->First(args))){
     return listutils::typeError("line or region expected");
   }
   return listutils::basicSymbol<DLine>();
}

template<class A>
int todlineVMT(Word* args, Word& result, int message, 
                    Word& local,Supplier s){
   A* a = (A*) args[0].addr;
   result = qp->ResultStorage(s);
   DLine* res = (DLine*) result.addr;
   res->clear();
   if(!a->IsDefined()){
     res->SetDefined(false);
     return 0;
   }
   res->SetDefined(true);
   HalfSegment hs;
   for(int i=0;i<a->Size();i++){
        a->Get(i,hs);
        if(hs.IsLeftDomPoint()){
            SimpleSegment ss(hs);
            res->append(ss);
        }
   }
   return 0;
}

ValueMapping todlineVM[] = {
    todlineVMT<Line>,
    todlineVMT<Region>
};

int todlineSelect(ListExpr args){
  return Line::checkType(nl->First(args))?0:1;
}

OperatorSpec todlineSpec(
  "{line,region} -> dline",
  " _ todline",
  " converts a line or the boundary of a region into"
  " a dline value",
  " query thecenter todline"
);

Operator todlineOp(
   "todline",
   todlineSpec.getStr(),
   2,
   todlineVM,
   todlineSelect,
   todlineTM
);


/*
52 Operator ~distanceWithin~

Checks whether the distance between two objects is smaller than a
given value.

*/
ListExpr distanceWithinTM(ListExpr args){
   string err = "SPATIAL x SPATIAL x {int, double} [x geoid] expected";
   if(!nl->HasLength(args,3) && !nl->HasLength(args,4)){
     return listutils::typeError(err+ " (wrong number of args)");
   }
   if(nl->HasLength(args,4) && !Geoid::checkType(nl->Fourth(args))){
     return listutils::typeError(err + " (4th arg not of type geoid)");
   }
   if(  !CcInt::checkType(nl->Third(args)) 
      &&!CcReal::checkType(nl->Third(args))){
     return listutils::typeError(err + " (third arg not of type int or real)");
   }
   ListExpr a = nl->First(args);
   if(   !Rectangle<2>::checkType(a)
      && !Point::checkType(a)
      && !Points::checkType(a)
      && !Line::checkType(a)
      && !Region::checkType(a)
      && !DLine::checkType(a)){
     return listutils::typeError(err + "(first arg not supported)");
   }
   a = nl->Second(args);
   if(   !Rectangle<2>::checkType(a)
      && !Point::checkType(a)
      && !Points::checkType(a)
      && !Line::checkType(a)
      && !Region::checkType(a)
      && !DLine::checkType(a)){
     return listutils::typeError(err + "(first arg not supported)");
   }
   return listutils::basicSymbol<CcBool>(); 
}


// generic solution
template<class A1, class A2>
bool distWithin(A1* a1, A2* a2, double dist, Geoid* geoid){
   double d = a1->Distance(*a2,geoid);
   return d <= dist;
}


template<class A1, class A2, class D, bool swap>
int distanceWithinVMT(Word* args, Word& result, int message, 
                    Word& local,Supplier s){

   result = qp->ResultStorage(s);
   CcBool* res = (CcBool*) result.addr;
   A1* a1 = 0;
   A2* a2 = 0;
   if(swap){
     a1 = (A1*) args[1].addr;
     a2 = (A2*) args[0].addr;
   } else {
     a1 = (A1*) args[0].addr;
     a2 = (A2*) args[1].addr;
   }
   D*  d = (D*) args[2].addr;
   Geoid* geoid = qp->GetNoSons(s)==4?(Geoid*)args[3].addr:0;
   if(!a1->IsDefined() || !a2->IsDefined() || !d->IsDefined()
      || (geoid && !geoid->IsDefined())){
     res->SetDefined(0);
   }
   double dist = d->GetValue();
   if(dist<0){
     res->Set(true,false);
     return 0;
   }
   Rectangle<2> b1 = a1->BoundingBox();
   Rectangle<2> b2 = a2->BoundingBox();
   if(geoid){
      if(!isGeo(b1) || !isGeo(b2)){
          res->SetDefined(false);
      }
   }   
   double bd = b1.Distance(b2, geoid);
   if(bd > dist){
      res->Set(true,false);
      return 0;
   }
   res->Set(true, distWithin(a1,a2,dist,geoid));
   return 0;
}

// overloaded for point
template<class D>
int distanceWithinVMT(Word* args, Word& result, int message, 
                    Word& local,Supplier s){

   result = qp->ResultStorage(s);
   CcBool* res = (CcBool*) result.addr;
   Point* a1 = (Point*) args[0].addr;
   Point* a2 = (Point*) args[1].addr;;
   D*  d = (D*) args[2].addr;
   Geoid* geoid = qp->GetNoSons(s)==4?(Geoid*)args[3].addr:0;
   if(!a1->IsDefined() || !a2->IsDefined() || !d->IsDefined()
      || (geoid && !geoid->IsDefined())){
     res->SetDefined(0);
   }
   double dist = d->GetValue();
   if(dist<0){
     res->Set(true,false);
     return 0;
   }
   double bd = a1->Distance(*a2, geoid);
   res->Set(true, bd<= dist);
   return 0;
}




ValueMapping distanceWithinVM[] = {
   distanceWithinVMT<CcInt>,
   distanceWithinVMT<CcReal>,
   distanceWithinVMT<Line,Point, CcInt, false>,
   distanceWithinVMT<Line,Point, CcReal, false>,
   distanceWithinVMT<Line,Point, CcInt, true>,
   distanceWithinVMT<Line,Point, CcReal, true>,
};


int distanceWithinSelect(ListExpr args){
  ListExpr a1 = nl->First(args);
  ListExpr a2 = nl->Second(args);
  ListExpr a3 = nl->Third(args);
  if(Point::checkType(a1) && Point::checkType(a2) && CcInt::checkType(a3)){
     return 0;
  }
  if(Point::checkType(a1) && Point::checkType(a2) && CcReal::checkType(a3)){
     return 1;
  }
  if(Line::checkType(a1) && Point::checkType(a2) && CcInt::checkType(a3)){
     return 2;
  }
  if(Line::checkType(a1) && Point::checkType(a2) && CcReal::checkType(a3)){
     return 3;
  }
  if(Point::checkType(a1) && Line::checkType(a2) && CcInt::checkType(a3)){
     return 4;
  }
  if(Point::checkType(a1) && Line::checkType(a2) && CcReal::checkType(a3)){
     return 5;
  }
  return -1;
}

OperatorSpec distanceWithinSpec(
  "SPATIAL x SPATIAL x {int, real} [x geoid] -> bool",
  "distanceWithin(_,_,_[,_])",
  "Checks whether the distance between two Objects is smaller "
  "than a given value", 
  "query distanceWithin(alexanderplatz, mehringdamm, 2000)"
);

Operator distanceWithinOp(
  "distanceWithin",
  distanceWithinSpec.getStr(),
  6,
  distanceWithinVM,
  distanceWithinSelect,
  distanceWithinTM
);


/*
53 Operator ~orderLine~

Orders the segments of a simple line so that the segments are connected.

\subsection{Type Mapping}
*/
ListExpr orderLineTM(ListExpr args) {
  string err = "one argument of the type sline expected";
  if (!nl->HasLength(args, 1)) {
    return listutils::typeError(err);
  }
  if (!SimpleLine::checkType(nl->First(args))) {
    return listutils::typeError(err);
  }
  ListExpr attrList = nl->TwoElemList(nl->TwoElemList(
                                        nl->SymbolAtom("Start"),
                                        nl->SymbolAtom(Point::BasicType())),
                                      nl->TwoElemList(
                                        nl->SymbolAtom("End"),
                                        nl->SymbolAtom(Point::BasicType())));
  return nl->TwoElemList(nl->SymbolAtom(Symbol::STREAM()),
                         nl->TwoElemList(nl->SymbolAtom(Tuple::BasicType()),
                                         attrList));
}

/*
\subsection{Class ~OrderLineLI~}

*/
class OrderSLineLI {
  
 public:
  OrderSLineLI(const SimpleLine& src) {
    tt = 0;
    pos = 0;
    if (!src.IsDefined()) {
      return;
    }
    tt = getTupleType();
    multimap<Point, int> point2seg;
    vector<HalfSegment> halfSegments;
    Point pt1(true, 0.0, 0.0);
    Point pt2(true, 1.0, 1.0);
    HalfSegment hs(true, pt1, pt2);
    for (int i = 0; i < src.Size(); i++) {
      halfSegments.push_back(hs);
    }
    vector<bool> isActive;
    isActive.resize(src.Size());
    int noActiveSegs = 0;
    for (int i = 0; i < src.Size(); i++) {
      src.Get(i, hs);
      if (hs.IsLeftDomPoint()) {
        point2seg.insert(make_pair(hs.GetLeftPoint(), i));
        point2seg.insert(make_pair(hs.GetRightPoint(), i));
        HalfSegment tempHs(hs);
        halfSegments[i] = tempHs;
        isActive[i] = true;
        noActiveSegs++;
      }
    }
//     cout << point2seg.size() << " pts and " << noActiveSegs 
//          << " hs inserted" << endl;
    int segNo = 0;
    hs = halfSegments[segNo];
    pt1 = hs.GetLeftPoint();
    seq.push_back(pt1);
    pt2 = pt1;
    bool successorFound = true;
    pair<multimap<Point, int>::iterator, multimap<Point, int>::iterator> it;
    while (successorFound && noActiveSegs > 0) {
      isActive[segNo] = false;
      noActiveSegs--;
//       cout << "segment " << segNo << " deactivated; " << noActiveSegs 
//            << " active" << endl << "search successor of " << pt1 << endl;
      it = point2seg.equal_range(pt1);
      multimap<Point, int>::iterator it1 = it.first;
      while (it1 != it.second && !isActive[it1->second]) {
        it1++;
      }
      if (it1 == it.second) { // no successor
        successorFound = false;
      }
      else {
        segNo = it1->second;
//         cout << "segment " << segNo << " is active" << endl;
        hs = halfSegments[segNo];
        if (hs.GetLeftPoint() == pt1) { // last added
          pt1 = hs.GetRightPoint();
        }
        else {
          pt1 = hs.GetLeftPoint();
        }
        seq.push_back(pt1);
      }
    }
  }
  
  ~OrderSLineLI() {
    if (tt) {
      tt->DeleteIfAllowed();
    }
  }
  
  TupleType *getTupleType() {
    SecondoCatalog* sc = SecondoSystem::GetCatalog();
    ListExpr resultTupleType = nl->TwoElemList(
      nl->SymbolAtom(Tuple::BasicType()),
      nl->TwoElemList(nl->TwoElemList(nl->SymbolAtom("Start"),
                                      nl->SymbolAtom(Point::BasicType())),
                      nl->TwoElemList(nl->SymbolAtom("End"),
                                      nl->SymbolAtom(Point::BasicType()))));
    ListExpr numResultTupleType = sc->NumericType(resultTupleType);
    return new TupleType(numResultTupleType);
  }
  
  Tuple *nextTuple() {
    if (pos == seq.size()) {
      return 0;
    }
    if (!tt) {
      tt = getTupleType();
    }
    Point *p2 = 0;
    if (pos == seq.size() - 1) {
      p2 = new Point(seq[0]);
    }
    else {
      p2 = new Point(seq[pos + 1]);
    }
    Point *p1 = new Point(seq[pos]);
    pos++;
    Tuple *result = new Tuple(tt);
    result->PutAttribute(0, p1);
    result->PutAttribute(1, p2);
    return result;
  }
  
 private:
  TupleType *tt;
  vector<Point> seq;
  unsigned int pos;
};

/*

\subsection{Value Mapping}
*/
int orderLineVM(Word* args, Word& result, int message, Word& local,Supplier s) {
  SimpleLine *src = static_cast<SimpleLine*>(args[0].addr);
  OrderSLineLI *li = static_cast<OrderSLineLI*>(local.addr);
  switch (message) {
    case OPEN: {
      if (li) {
        delete li;
        local.addr = 0;
      }
      li = new OrderSLineLI(*src);
      local.addr = li;
      return 0;
    }
    case REQUEST: {
      result.addr = li ? li->nextTuple() : 0;
      return result.addr ? YIELD : CANCEL;
    }
    case CLOSE: {
      if (local.addr) {
        li = (OrderSLineLI*)local.addr;
        delete li;
        local.addr = 0;
      }
      return 0;
    }
  }
  return 0;
}

OperatorSpec orderLineSpec(
  "sline -> stream(Start: point, End: point)",
  "orderLine( _ )",
  "Orders the segments of a simple line so that they are connected.",
  "query orderLine(fromline(BGrenzenLine))"
);

Operator orderLine(
  "orderLine",
  orderLineSpec.getStr(),
  orderLineVM,
  Operator::SimpleSelect,
  orderLineTM
);



/*
11 Creating the Algebra

*/

class SpatialAlgebra : public Algebra
{
 public:
  SpatialAlgebra() : Algebra()
  {
    AddTypeConstructor( &point );
    AddTypeConstructor( &points );
    AddTypeConstructor( &line );
    AddTypeConstructor( &region );

    AddTypeConstructor( &sline);
    AddTypeConstructor( &dline);

    AddTypeConstructor( &drm);
    AddTypeConstructor( &oim);

    AddTypeConstructor( & label);


    point.AssociateKind(Kind::DATA());
    point.AssociateKind(Kind::CSVEXPORTABLE());
    point.AssociateKind(Kind::CSVIMPORTABLE());
    points.AssociateKind(Kind::DATA());
    line.AssociateKind(Kind::DATA());
    region.AssociateKind(Kind::DATA());
    sline.AssociateKind(Kind::DATA());
    dline.AssociateKind(Kind::DATA());
    drm.AssociateKind(Kind::DATA());
    oim.AssociateKind(Kind::DATA());
    label.AssociateKind(Kind::DATA());



    point.AssociateKind(Kind::SPATIAL2D());
    points.AssociateKind(Kind::SPATIAL2D());
    line.AssociateKind(Kind::SPATIAL2D());
    region.AssociateKind(Kind::SPATIAL2D());
    sline.AssociateKind(Kind::SPATIAL2D());
    dline.AssociateKind(Kind::SPATIAL2D());


    point.AssociateKind(Kind::SHPEXPORTABLE());
    points.AssociateKind(Kind::SHPEXPORTABLE());
    line.AssociateKind(Kind::SHPEXPORTABLE());
    region.AssociateKind(Kind::SHPEXPORTABLE());


    point.AssociateKind(Kind::SQLEXPORTABLE());
    points.AssociateKind(Kind::SQLEXPORTABLE());

    AddTypeConstructor(&geoid_t);
    geoid_t.AssociateKind(Kind::DATA());

    AddTypeConstructor(&disc);
    disc.AssociateKind(Kind::DATA());

    AddTypeConstructor(&segment);
    segment.AssociateKind(Kind::DATA());


    AddOperator( &spatialisempty );
    AddOperator( &spatialequal );
    AddOperator( &spatialisless);
    AddOperator( &spatialnotequal );
    AddOperator( &spatialintersects );
    AddOperator( &spatialinside );
    AddOperator( &spatialadjacent );
    AddOperator( &spatialoverlaps );
    AddOperator( &spatialonborder );
    AddOperator( &spatialininterior );
    AddOperator( &spatialintersection);
    AddOperator( &spatialintersection_rob);
    AddOperator( &spatialminus );
    AddOperator( &spatialunion );
    AddOperator( &spatialcrossings );
    AddOperator( &spatialtouchpoints);
    AddOperator( &spatialcommonborder);
    AddOperator( &spatialsingle );
    AddOperator( &spatialdistance );
    AddOperator( &distanceSmallerThan );
    AddOperator( &spatialdirection );
    AddOperator( &spatialheading );
    AddOperator( &spatialnocomponents );
    AddOperator( &spatialnosegments );
    AddOperator( &spatialsize );
    AddOperator( &spatialbbox);
    AddOperator( &spatialtranslate );
    AddOperator( &spatialrotate );
    AddOperator( &spatialcenter );
    AddOperator( &spatialconvexhull );
    AddOperator( &spatialwindowclippingin );
    AddOperator( &spatialwindowclippingout );
    AddOperator( &spatialcomponents );
    AddOperator( &spatialvertices );
    AddOperator( &spatialboundary );
    AddOperator( &spatialscale );
    AddOperator( &spatialatpoint );
    AddOperator( &spatialatposition );
    AddOperator( &spatialsubline );
    AddOperator( &spatialadd );
    AddOperator( &spatialgetx );
    AddOperator( &spatialgety );
    AddOperator( &spatialline2region );
    AddOperator( &spatialrect2region );
    AddOperator( &spatialarea );
    AddOperator( &spatialpolylines);
    AddOperator( &spatialpolylinesC);
    AddOperator( &spatialsegments );
    AddOperator( &spatialget );
    AddOperator( &spatialsimplify);
    AddOperator( &realminize);
    AddOperator( &makeline);
    AddOperator( &makesline);
    AddOperator( &commonborder2);
    AddOperator( &spatialtoline);
    AddOperator( &spatialfromline);
    AddOperator( &spatialiscycle);
    AddOperator( &utmOp);
    AddOperator( &gkOp);
    AddOperator( &reverseGkOp);
    AddOperator( &spatialcollect_line);
    AddOperator( &spatialcollect_sline);
    AddOperator( &spatialcollect_points);
    AddOperator( &spatialmakepoint );
    AddOperator( &halfSegmentsOp );
    AddOperator( &spatialgetstartpoint);
    AddOperator( &spatialgetendpoint);
    AddOperator( &spatialsetstartsmaller ) ;
    AddOperator( &spatialgetstartsmaller ) ;
    AddOperator( &spatial_create_sline ) ;
    AddOperator( &spatial_distanceOrthodrome );
    AddOperator( &geoid_getRadius );
    AddOperator( &geoid_getFlattening );
    AddOperator( &geoid_create_geoid );
    AddOperator( &point2string );
    AddOperator( &spatialmidpointbetween );
    AddOperator( &spatialDirectionToHeading );
    AddOperator( &spatialHeadingToDirection );
    AddOperator( &spatialCreateTriangle );

    AddOperator(&spatialcircle);
    AddOperator(&spatiallonglines);
    AddOperator(&spatialsplitslineatpoints);

    AddOperator(&findCycles);

    AddOperator(&markUsageOp);
    AddOperator(&criticalPoints);
    AddOperator(&testRegionCreator);
    AddOperator(&collect_box);
    AddOperator(&contains_rob);
    AddOperator(&getHoles);

    AddOperator(&collect_line2);
    AddOperator(&getInnerPoint);
    AddOperator(&checkRealmOp);
    checkRealmOp.enableInitFinishSupport();
    AddOperator(&badRealmOp);
    AddOperator(&crossings_rob);

    AddOperator(&splitline);
    AddOperator(&computeDRM);
    AddOperator(&computeOIM);
    AddOperator(&collectDline);
    AddOperator(&computeLabelOP);
    AddOperator(&centroidDiscOP);
    AddOperator(&calcDiscOP);
    AddOperator(&createDiscOP);
    AddOperator(&berlin2wgs);
    AddOperator(&elementsOP);
    AddOperator(&twistOp);
    AddOperator(&contour2Op);
    AddOperator(&twist2Op);
    AddOperator(&twist3Op);
    AddOperator(&toSVGOp);
    AddOperator(&simpleProject);
    AddOperator(&todlineOp);
    AddOperator(&distanceWithinOp);
    AddOperator(&orderLine);
  }
  ~SpatialAlgebra() {};
};

/*
12 Initialization

Each algebra module needs an initialization function. The algebra manager
has a reference to this function if this algebra is included in the list
of required algebras, thus forcing the linker to include this module.

The algebra manager invokes this function to get a reference to the instance
of the algebra class and to provide references to the global nested list
container (used to store constructor, type, operator and object information)
and to the query processor.

The function has a C interface to make it possible to load the algebra
dynamically at runtime.

*/

extern "C"
Algebra*
InitializeSpatialAlgebra( NestedList* nlRef, QueryProcessor* qpRef )
{
  nl = nlRef;
  qp = qpRef;
  return (new SpatialAlgebra());
}
