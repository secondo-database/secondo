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

using namespace std;

#include "../../Tools/Flob/Flob.h"
#include "../../Tools/Flob/DbArray.h"
#include "Algebra.h"
#include "NestedList.h"
#include "ListUtils.h"
#include "GenericTC.h"
#include "QueryProcessor.h"
#include "StandardTypes.h"
#include "SpatialAlgebra.h"
#include "SecondoConfig.h"
#include "AvlTree.h"
#include "AVLSegment.h"
#include "AlmostEqual.h"
#include "../Relation-C++/RelationAlgebra.h"
#include "RegionTools.h"
#include "Symbols.h"
#include "Stream.h"
#include "RegionCreator.h"

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

#include "DRM.h"


#ifndef M_PI
const double M_PI = acos( -1.0 );
#endif

extern NestedList* nl;
extern QueryProcessor* qp;

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

enum object {none, first, second, both};
enum status {endnone, endfirst, endsecond, endboth};

void SelectFirst_pp( const Points& P1, const Points& P2,
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

void SelectNext_pp( const Points& P1, const Points& P2,
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

void SelectFirst_pl( const Points& P, const Line& L,
                     object& obj, status& stat )
{
  P.SelectFirst();
  L.SelectFirst();

  Point p1;
  Point p2;
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
  Point p1;
  Point p2;
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

  Point p1;
  Point p2;
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
  Point p1;
  Point p2;
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

void SelectFirst_ll( const Line& L1, const Line& L2,
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

void SelectNext_ll( const Line& L1, const Line& L2,
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

void SelectFirst_rr( const Region& R1, const Region& R2,
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

void SelectNext_rr( const Region& R1, const Region& R2,
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

bool Point::Inside( const Region& r,
                    const Geoid* geoid /*=0*/ ) const
{
  return r.Contains(*this,geoid);
}

bool Point::Inside( const Line& l,
                    const Geoid* geoid /*=0*/ ) const
{
  return l.Contains(*this,geoid);
}

bool Point::Inside(const SimpleLine& l, const Geoid* geoid /*=0*/) const
{
  return l.Contains(*this,geoid);
}

bool Point::Inside( const Points& ps,
                    const Geoid* geoid /*=0*/ ) const
{
  return ps.Contains(*this,geoid);
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


void Point::Intersection(const Point& p, Points& result,
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

void Point::Intersection(const Points& ps, Points& result,
                         const Geoid* geoid /*=0*/ ) const{
  ps.Intersection(*this, result, geoid);
}

void Point::Intersection(const Line& l, Points& result,
                         const Geoid* geoid /*=0*/) const{
  l.Intersection(*this, result, geoid);
}

void Point::Intersection(const Region& r, Points& result,
                         const Geoid* geoid /*=0*/) const{
  r.Intersection(*this, result, geoid);
}

void Point::Intersection(const SimpleLine& l, Points& result,
                         const Geoid* geoid /*=0*/) const{
  l.Intersection(*this, result, geoid);
}

void Point::Minus(const Point& p, Points& result,
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
void Point::Minus(const Points& ps, Points& result,
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

void Point::Minus(const Line& l, Points& result,
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

void Point::Minus(const Region& r, Points& result,
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

void Point::Minus(const SimpleLine& l, Points& result,
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

void Point::Union(const Point& p, Points& result,
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

void Point::Union(const Points& ps, Points& result,
                  const Geoid* geoid /*=0*/) const{
  ps.Union(*this, result, geoid);
}

void Point::Union(const Line& l, Line& result,
                  const Geoid* geoid /*=0*/) const{
  l.Union(*this, result, geoid);
}

void Point::Union(const Region& r, Region& result,
                  const Geoid* geoid /*=0*/) const{
  r.Union(*this, result, geoid);
}

void Point::Union(const SimpleLine& l, SimpleLine& result,
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
    return -1.0;
  }
  if(geoid && (!checkGeographicCoord() || !p.checkGeographicCoord()) ){
    cerr << __PRETTY_FUNCTION__ << ": Invalid geographic coordinate." << endl;
    return -1.0;
  }
  if(AlmostEqual(*this, p)){
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
                          double& lonMinDEG, double lonMaxDEG) const{
  if(    !checkGeographicCoord() || !other.checkGeographicCoord()
      || AlmostEqual(*this,other)
      || (latitudeDEG<-90) || (latitudeDEG>90) ){
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
    return Rectangle<2>(false, 0.0, 0.0, 0.0, 0.0);
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
                                         const bool epsilon /* = 1e-12 */) const
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
    cout << __PRETTY_FUNCTION__ << ": Invalid parameter." << endl;
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
4.11 ~Cast~-function

*/
void* CastPoint(void* addr)
{
  return (new (addr) Point());
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
  CastPoint,                  //cast function
  SizeOfPoint,                //sizeof function
  CheckPoint);               //kind checking function

/*
5 Type Constructor ~points~

A ~points~ value is a finite set of points.

5.1 Implementation of the class ~Points~

*/
bool Points::Find( const Point& p, int& pos, const bool& exact ) const
{
  assert( IsOrdered() );
  assert( IsDefined());
  if (exact){
    return points.Find( &p, PointCompare, pos );
  } else {
    return points.Find( &p, PointCompareAlmost, pos );
  }
}

Points& Points::operator=( const Points& ps )
{
  assert( ps.IsOrdered() );
  points.copyFrom(ps.points);
  bbox = ps.BoundingBox();
  ordered = true;
  SetDefined(ps.IsDefined());
  return *this;
}

void Points::StartBulkLoad()
{
  ordered = false;
}

void Points::EndBulkLoad( bool sort, bool remDup, bool trim )
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

bool Points::operator==( const Points& ps ) const
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

bool Points::operator!=( const Points& ps ) const
{
  return !( *this == ps );
}

Points& Points::operator+=( const Point& p )
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

Points& Points::operator+=( const Points& ps )
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
    Points newPs( Size() + ps.Size() );
    Union( ps, newPs );
    *this = newPs;
  }
  return *this;
}

Points& Points::operator-=( const Point& p )
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

ostream& operator<<( ostream& o, const Points& ps )
{
  o << "<";
  if( !ps.IsDefined() ) {
    o << " undef ";
  } else {
    for( int i = 0; i < ps.Size(); i++ )
    {
      Point p;
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

void Points::Sort(const bool exact /*= true*/)
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
bool AlmostContains( const DbArray<Point>& points, const Point& p,
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



void Points::RemoveDuplicates()
{
 assert(IsOrdered());
 //Point allPoints[points.Size()];
 DbArray<Point> allPoints(points.Size());
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

bool Points::Contains( const Point& p, const Geoid* geoid /*=0*/ ) const
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

bool Points::Contains( const Points& ps, const Geoid* geoid /*=0*/ ) const
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

bool Points::Inside( const Points& ps, const Geoid* geoid /*=0*/ ) const
{
  assert( IsDefined() );
  assert( IsOrdered() );
  assert( ps.IsDefined() );
  assert( ps.IsOrdered() );
  assert( !geoid || geoid->IsDefined() );
  return ps.Contains( *this, geoid );
}

bool Points::Inside( const Line& l, const Geoid* geoid /*=0*/ ) const
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

bool Points::Inside( const Region& r, const Geoid* geoid /*=0*/ ) const
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

bool Points::Intersects( const Points& ps, const Geoid* geoid /*=0*/ ) const
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

bool Points::Intersects( const Line& l, const Geoid* geoid /*=0*/ ) const
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

bool Points::Intersects( const Region& r, const Geoid* geoid /*=0*/ ) const
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

bool Points::Adjacent( const Region& r, const Geoid* geoid /*=0*/ ) const
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

void Points::Intersection(const Point& p, Points& result,
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

void Points::Intersection( const Points& ps, Points& result,
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

void Points::Intersection( const Line& l, Points& result,
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

void Points::Intersection( const Region& r, Points& result,
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

void Points::Intersection( const SimpleLine& l, Points& result,
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

void Points::Minus( const Point& p, Points& ps,
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

void Points::Minus( const Points& ps, Points& result,
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

void Points::Minus( const Line& l, Points& result,
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

void Points::Minus( const Region& r, Points& result,
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

void Points::Minus( const SimpleLine& l, Points& result,
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

void Points::Union( const Point& p, Points& result,
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

void Points::Union( const Points& ps, Points& result,
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


void Points::Union( const Line& line, Line& result,
                    const Geoid* geoid /*=0*/ ) const{
   line.Union(*this,result,geoid);
}

void Points::Union( const Region& region, Region& result,
                    const Geoid* geoid /*=0*/) const{
   region.Union(*this,result,geoid);
}

void Points::Union(const SimpleLine& line, SimpleLine& result,
                   const Geoid* geoid /*=0*/) const{
  line.Union(*this,result, geoid);
}

double Points::Distance( const Point& p, const Geoid* geoid /* = 0 */ ) const
{
  assert( !IsEmpty() );
  assert( p.IsDefined() );
  assert( !geoid || geoid->IsDefined() );

  double result = numeric_limits<double>::max();
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

double Points::Distance( const Points& ps, const Geoid* geoid /* = 0 */ ) const
{
  assert( !IsEmpty() );
  assert( !ps.IsEmpty() );
  assert( !geoid || geoid->IsDefined() );

  double result = numeric_limits<double>::max();
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

double Points::Distance( const Rectangle<2>& r, const Geoid* geoid/*=0*/ ) const
{
  assert( IsDefined() );
  assert( !IsEmpty() );
  assert( r.IsDefined() );
  assert( !geoid || geoid->IsDefined() );
  double result = numeric_limits<double>::max();
  Point pi;
  for( int i = 0; i < Size(); i++ )
  {
    Get( i, pi );
    result = MIN( result, pi.Distance( r, geoid ) );
  }
  return result;
}

bool Points::Intersects( const Rectangle<2>& r, const Geoid* geoid/*=0*/ ) const
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

void Points::Translate( const Coord& x, const Coord& y, Points& result ) const
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


void Points::Rotate( const Coord& x, const Coord& y,
                     const double alpha,
                     Points& result ) const
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

Point Points::theCenter() const{
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



size_t Points::HashValue() const
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

bool Points::IsValid() const
{
  if( IsEmpty() ) // IsEmpty() includes undef
    return true;

  Point p1, p2;
  Get( 0, p1 );
  if( !p1.IsDefined() ){
    cerr << __PRETTY_FUNCTION__ << ": Undefined Point!" << endl;
    cerr << "\tp1 = "; p1.Print(cerr); cerr << endl;
    return false;
  }
  for( int i = 1; i < Size(); i++ )
  {
    Get( i, p2 );
    if( !p2.IsDefined() ){
      cerr << __PRETTY_FUNCTION__ << ": Undefined Point!" << endl;
      cerr << "\tp2 = "; p2.Print(cerr); cerr << endl;
      return false;
    }
    if( AlmostEqual( p1, p2 ) ){
      cerr << __PRETTY_FUNCTION__ << ": Almost equal Points!" << endl;
      cerr << "\tp1 = "; p1.Print(cerr);
      cerr << "\n\tp2 = "; p2.Print(cerr); cerr << endl;
      return false;
    }
    p1 = p2;
  }
  return true;
}

void Points::Clear()
{
  points.clean();
  pos = -1;
  ordered = true;
  bbox.SetDefined( false );
}

void Points::CopyFrom( const Attribute* right )
{
  const Points *ps = (const Points*)right;
  assert( ps->IsOrdered() );
  *this = *ps;
}

int Points::Compare( const Attribute* arg ) const
{
  const Points* ps = (const Points*)arg;

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

int Points::CompareAlmost( const Attribute* arg ) const
{
  const Points* ps = (const Points*)arg;

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


bool Points::Adjacent( const Attribute* arg ) const
{
  return 0;
  // for points which takes double values, we can not decide whether they are
  //adjacent or not.
}

Points* Points::Clone() const
{
  return new Points( *this );
}

ostream& Points::Print( ostream &os ) const
{
  return os << *this;
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

  Point p;
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
      Point p1, p2;
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
      Point p1, p2;
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
7 Type Constructor ~line~

A ~line~ value is a set of halfsegments. In the external (nestlist)
representation, a line value is expressed as a set of segments.
However, in the internal (class) representation, it is expressed
as a set of sorted halfsegments, which are stored as a PArray.

7.1 Implementation of the class ~line~

*/
void Line::StartBulkLoad()
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

void Line::EndBulkLoad( const bool sort /* = true */,
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
     DbArray<HalfSegment>* line2 = ::Realminize(line,false, robust);
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

Line& Line::operator=( const Line& l )
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

bool Line::operator==( const Line& l ) const
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

bool Line::operator!=( const Line& l ) const
{
return !( *this == l);
}

Line& Line::operator+=( const HalfSegment& hs )
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

Line& Line::operator+=(const Line& l){
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


Line& Line::operator-=( const HalfSegment& hs )
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

bool Line::Contains( const Point& p, const Geoid* geoid/*=0*/ ) const
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

bool Line::Intersects( const Line& l, const Geoid* geoid/*=0*/ ) const
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

bool Line::Intersects( const Region& r, const Geoid* geoid/*=0*/ ) const
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

bool Line::Inside( const Line& l, const Geoid* geoid/*=0*/ ) const
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

bool Line::Inside( const Region& r, const Geoid* geoid/*=0*/ ) const
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

bool Line::Adjacent( const Region& r, const Geoid* geoid/*=0*/ ) const
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


void Line::Intersection(const Point& p, Points& result,
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

void Line::Intersection(const Points& ps, Points& result,
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


void Line::Intersection( const Line& l, Line& result,
                         const Geoid* geoid/*=0*/ ) const
{
  SetOp(*this,l,result,avlseg::intersection_op, geoid);
}

void Line::Intersection(const Region& r, Line& result,
                        const Geoid* geoid/*=0*/) const{
 r.Intersection(*this,result,geoid);
}

void Line::Intersection( const SimpleLine& l, SimpleLine& result,
                         const Geoid* geoid/*=0*/ ) const
{
 SetOp(*this,l,result,avlseg::intersection_op, geoid);
}

void Line::Minus(const Point& p, Line& result,
                 const Geoid* geoid/*=0*/) const {
  result.Clear();
  if(!IsDefined() || !p.IsDefined()){
    result.SetDefined(false);
    return;
  }
  result.CopyFrom(this);
}

void Line::Minus(const Points& ps, Line& result,
                 const Geoid* geoid/*=0*/) const {
  result.Clear();
  if(!IsDefined() || !ps.IsDefined() || (geoid&& !geoid->IsDefined()) ){
    result.SetDefined(false);
    return;
  }
  result.CopyFrom(this);
}

void Line::Minus(const Line& line, Line& result,
                 const Geoid* geoid/*=0*/) const{
 SetOp(*this,line,result,avlseg::difference_op,geoid);
}

void Line::Minus(const Region& region, Line& result,
                 const Geoid* geoid/*=0*/) const{
 SetOp(*this,region, result,avlseg::difference_op,geoid);
}

void Line::Minus(const SimpleLine& line, Line& result,
                 const Geoid* geoid/*=0*/) const{
  SetOp(*this,line,result,avlseg::difference_op,geoid);
}

void Line::Union(const Point& p, Line& result, const Geoid* geoid/*=0*/) const{
  result.Clear();
  if(!IsDefined() || !p.IsDefined() || (geoid&& !geoid->IsDefined()) ){
    result.SetDefined(false);
    return;
  }
  result.CopyFrom(this);
}

void Line::Union(const Points& ps, Line& result,
                 const Geoid* geoid/*=0*/) const{
  result.Clear();
  if(!IsDefined() || !ps.IsDefined() || (geoid&& !geoid->IsDefined())){
    result.SetDefined(false);
    return;
  }
  result.CopyFrom(this);
}

void Line::Union(const Line& line, Line& result,
                 const Geoid* geoid/*=0*/) const{
 try{
    SetOp(*this, line, result, avlseg::union_op,geoid,true);
 } catch(...){
    cerr << "union via plane sweep  failed, use slower implementation";
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

void Line::Union(const Region& region, Region& result,
                 const Geoid* geoid/*=0*/) const{
 region.Union(*this,result,geoid);
}

void Line::Union(const SimpleLine& line, Line& result,
                 const Geoid* geoid/*=0*/) const{
  SetOp(*this, line, result, avlseg::union_op,geoid);
}

void Line::Crossings( const Line& l, Points& result,
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

void Line::Crossings(Points& result, const Geoid* geoid/*=0*/) const{
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



double Line::Distance( const Point& p, const Geoid* geoid /* = 0 */ ) const {
  assert( !IsEmpty() );
  assert( p.IsDefined() );
  assert(!geoid || geoid->IsDefined() );
  if( IsEmpty() || !p.IsDefined()){
    return -1;
  }

  assert( IsOrdered() );
  HalfSegment hs;
  double result = numeric_limits<double>::max();
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

double Line::MaxDistance( const Point& p, const Geoid* geoid /* = 0 */ ) const {
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

double Line::Distance( const Points& ps, const Geoid* geoid /* = 0 */ ) const {
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
  double result = numeric_limits<double>::max();
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

double Line::Distance( const Line& l, const Geoid* geoid /* = 0 */ ) const
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
  double result = numeric_limits<double>::max();
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

void Line::DistanceSmallerThan(const Line& l,
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


double Line::Distance( const Rectangle<2>& r,
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
  double dist = numeric_limits<double>::max();
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

bool Line::Intersects( const Rectangle<2>& r,
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
double Line::MaxDistance( const Rectangle<2>& r,
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


int Line::NoComponents() const {
  return noComponents;
}

void Line::Translate( const Coord& x, const Coord& y, Line& result ) const
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

void Line::Rotate( const Coord& x, const Coord& y,
                 const double alpha,
                 Line& result ) const
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


/*
~getNext~

auxiliary function for Line::Transform

*/
int getNext(const DbArray<HalfSegment>* hss, int pos, const char* usage){
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
void markUsage( Line* line, char* usage, char* critical){
    markUsage( (DbArray<HalfSegment>*) line->GetFLOB(0),usage, critical);
}


void markUsage(const DbArray<HalfSegment>* hss, char* usage, char* critical ){
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
  bool removed = false; // true if halfsegments was removed
  // 1.2 mark segments
  for(int i=0;i<hss->Size();i++) {
     if((usage[i]==0) ){
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

             removed = true;
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



void Line::Transform( Region& result ) const
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


  DbArray<HalfSegment>* halfsegments =
                      new DbArray<HalfSegment>(this->Size());
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
  RegionCreator::setPartnerNo(halfsegments);
  RegionCreator::createRegion(halfsegments,&result);
  delete halfsegments;
  delete[] usage;
  delete[] critical;
}



/*
Simple function marking some elements between min and max as used to avoid
segments degenerated to single points.

*/
static double maxDist(vector<Point>& orig, // points
                      int min, int max, // range
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

static bool douglas_peucker(vector<Point>& orig, // original line
                     const double epsilon, // maximum derivation
                     bool* use, // result
                     int min, int max,
                     bool force = false,
                     const Geoid* geoid = 0){ // current range
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



static void  douglas_peucker(vector<Point>& orig,  // original chain of points
                             const double epsilon, // maximum derivation
                             bool* use,            // result
                             const Geoid* geoid = 0){
  for(unsigned int i=0;i<orig.size();i++){
    use[i] = false;
  }
  // call the recursive implementation
  douglas_peucker(orig,epsilon, use, 0, orig.size()-1,geoid);
}



void Line::Simplify(Line& result, const double epsilon,
                    const Points& importantPoints /*= Points(0)*/ ,
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

 vector<Point> forward;
 vector<Point> backward;
 vector<Point> complete;

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
           int s = max(0,partner-2);
           int e = min(partner+3,Size());
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
        int s = max(0, next-2);
        int e = min(next+3,Size());
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
     douglas_peucker(complete,epsilon,use,geoid);
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


void Line::Realminize(){
 // special case: empty line
if(!IsDefined()){
  line.clean();
  return;
}

Line tmp(0);
Realminize2(*this,tmp);
*this = tmp;
}

void Line::Vertices( Points* result ) const
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

void Line::Boundary(Points* result) const{
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


bool Line::Find( const HalfSegment& hs, int& pos, const bool& exact ) const
{
assert( IsDefined() );
assert( IsOrdered() );
if( exact )
  return line.Find( &hs, HalfSegmentCompare, pos );
return line.Find( &hs, HalfSegmentCompare, pos );
}

bool Line::Find( const Point& p, int& pos, const bool& exact ) const
{
assert( IsDefined() );
assert( p.IsDefined() );
assert( IsOrdered() );
if( exact )
  return line.Find( &p, PointHalfSegmentCompare, pos );
return line.Find( &p, PointHalfSegmentCompareAlmost, pos );
}

void Line::SetPartnerNo() {
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
      cerr << "wrong order in halfsegment array" << endl;
      cerr << "the system may be instable" << endl;
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
      cerr << "Error in halfsegment array detected" << endl;
      cerr << "may be a wrong ordering or wrong set edgeno's" << endl;
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

bool Line::GetNextSegment( const int poshs, const HalfSegment& hs,
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

bool Line::GetNextSegments( const int poshs, const HalfSegment& hs,
                            vector<bool>& visited,
                            int& posnexths, HalfSegment& nexths,
                            stack< pair<int,  HalfSegment> >& nexthss )
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
          nexthss.push( make_pair( auxposhs, aux ) );
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
          nexthss.push( make_pair( auxposhs, aux ) );
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

int Line::getUnusedExtension(int startPos,const DbArray<bool>& used)const{
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

void Line::collectFace(int faceno, int startPos, DbArray<bool>& used){
  set<int> extensionPos;

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

void Line::computeComponents() {
  length = 0.0;
  noComponents = 0;
  bbox.SetDefined(false);

  if(!IsDefined() || Size()==0){
    return;
  }

  DbArray<bool> used(line.Size());

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

void Line::Sort()
{
  assert( !IsOrdered() );
  line.Sort( HalfSegmentCompare );
  ordered = true;
}

void Line::RemoveDuplicates()
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

void Line::WindowClippingIn( const Rectangle<2> &window,
                             Line &clippedLine,
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

void Line::WindowClippingOut( const Rectangle<2> &window,
                              Line &clippedLine,
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


ostream& operator<<( ostream& os, const Line& cl )
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

size_t Line::HashValue() const
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

void Line::Clear()
{
  line.clean();
  pos = -1;
  ordered = true;
  bbox.SetDefined( false );
  SetDefined(true);
}

void Line::CopyFrom( const Attribute* right )
{
  *this = *((const Line *)right);
}

int Line::Compare( const Attribute *arg ) const
{

  const Line &l = *((const Line*)arg);

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

Line* Line::Clone() const
{
  return new Line( *this );
}


double Line::Length(const Geoid &geoid, bool& valid) const {
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

ostream& Line::Print( ostream &os ) const
{
  ios_base::fmtflags oldOptions = os.flags();
  os.setf(ios_base::fixed,ios_base::floatfield);
  os.precision(8);
  os << *this;
  os.flags(oldOptions);
//   for (int i = 0; i < Size(); i++)
//   {
//     HalfSegment hs;
//     Get(i, hs);
//     if (hs.IsLeftDomPoint()) hs.Print(os);
//   }
  return os;
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


/*
7 The type SimpleLine

7.1 Constructor

This constructor coinstructs a simple line from ~src~

If ~src~ is not simple, the simple line will be invalidated, i.e.
the defined flag is set to false;

*/
SimpleLine::SimpleLine(const Line& src):
     StandardSpatialAttribute<2>(src.IsDefined()), segments(0),lrsArray(0),
     startSmaller(true), isCycle(false),isOrdered(true),length(0.0),
     bbox(false),currentHS(-1)
{
  fromLine(src);
}

/*
7.2 Bulk Loading Functions

*/
void SimpleLine::StartBulkLoad(){
  isOrdered = false;
  SetDefined( true );
}

/*
~Operator +=~

Appends an HalfSegment during BulkLoading.

*/
SimpleLine& SimpleLine::operator+=(const HalfSegment& hs){
  assert(!isOrdered && IsDefined());
  segments.Append(hs);
  return *this;
}

bool SimpleLine::EndBulkLoad(){
  if( !IsDefined() ) {
    Clear();
    SetDefined( false );
  }
  // Sort the segments
  Sort();
  // Realminize the segments
  DbArray<HalfSegment>* tmp;
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

Point SimpleLine::StartPoint() const {
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

Point SimpleLine::StartPoint( bool startsSmaller ) const {
  if( IsEmpty() || !IsDefined() ) return Point( false );
  if (startsSmaller == startSmaller) return StartPoint();
  else return EndPoint();
}

/*
~EndPoint~

Returns the endpoint of this simple Line.

*/

Point SimpleLine::EndPoint() const {
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

Point SimpleLine::EndPoint( bool startsSmaller ) const {
  if( IsEmpty() || !IsDefined() ) return Point( false );
  if (startsSmaller == startSmaller) return EndPoint();
  else return StartPoint();
}

bool SimpleLine::Contains( const Point& p,
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

bool SimpleLine::Inside(const SimpleLine& l, const Geoid* geoid) const
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

void SimpleLine::Intersection(const Point& p, Points& result,
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

void SimpleLine::Intersection(const Points& ps, Points& result,
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

void SimpleLine::Intersection( const Line& l, SimpleLine& result,
                               const Geoid* geoid/*=0*/ ) const
{
  SetOp(*this,l,result,avlseg::intersection_op, geoid);
}

void SimpleLine::Intersection( const SimpleLine& l, SimpleLine& result,
                               const Geoid* geoid/*=0*/ ) const
{
  SetOp(*this,l,result,avlseg::intersection_op, geoid);
}

void SimpleLine::Intersection(const Region& r, SimpleLine& result,
                              const Geoid* geoid/*=0*/) const{
  r.Intersection(*this,result,geoid);
}

void SimpleLine::Minus(const Point& p, SimpleLine& result,
                       const Geoid* geoid/*=0*/) const {
  result.Clear();
  if(!IsDefined() || !p.IsDefined()){
    result.SetDefined(false);
    return;
  }
  result.CopyFrom(this);
}

void SimpleLine::Minus(const Points& ps, SimpleLine& result,
                       const Geoid* geoid/*=0*/) const {
 result.Clear();
 if(!IsDefined() || !ps.IsDefined() || (geoid&& !geoid->IsDefined()) ){
   result.SetDefined(false);
   return;
 }
 result.CopyFrom(this);
}

void SimpleLine::Minus(const Line& line, SimpleLine& result,
                       const Geoid* geoid/*=0*/) const{
  SetOp(*this,line,result,avlseg::difference_op,geoid);
}

void SimpleLine::Minus(const Region& region, SimpleLine& result,
                       const Geoid* geoid/*=0*/) const{
  SetOp(*this,region, result,avlseg::difference_op,geoid);
}

void SimpleLine::Minus(const SimpleLine& line, SimpleLine& result,
                       const Geoid* geoid/*=0*/) const{
  SetOp(*this,line,result,avlseg::difference_op,geoid);
}

void SimpleLine::Union(const Point& p, SimpleLine& result,
                       const Geoid* geoid/*=0*/) const{
  result.Clear();
  if(!IsDefined() || !p.IsDefined() || (geoid&& !geoid->IsDefined()) ){
    result.SetDefined(false);
    return;
  }
  result.CopyFrom(this);
}

void SimpleLine::Union(const Points& ps, SimpleLine& result,
                       const Geoid* geoid/*=0*/) const{
  result.Clear();
  if(!IsDefined() || !ps.IsDefined() || (geoid&& !geoid->IsDefined())){
    result.SetDefined(false);
    return;
  }
  result.CopyFrom(this);
}

void SimpleLine::Union(const Line& line, Line& result,
                       const Geoid* geoid/*=0*/) const{
  SetOp(*this, line, result, avlseg::union_op,geoid);
}

void SimpleLine::Union(const Region& region, Region& result,
                       const Geoid* geoid/*=0*/) const{
  region.Union(*this,result,geoid);
}

void SimpleLine::Union(const SimpleLine& line, Line& result,
                       const Geoid* geoid/*=0*/) const{
  SetOp(*this, line, result, avlseg::union_op,geoid);
}

double SimpleLine::Distance(const Point& p, const Geoid* geoid /* = 0 */)const {
  assert( !IsEmpty() );
  assert( p.IsDefined() );
  assert( ! geoid || geoid->IsDefined() );
  HalfSegment hs;
  double result = numeric_limits<double>::max();
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


double SimpleLine::Distance(const Points& ps, const Geoid* geoid /* =0 */)const{
  assert( !IsEmpty() );
  assert( !ps.IsEmpty() );
  assert( ! geoid || geoid->IsDefined() );
  HalfSegment hs;
  Point p;
  double result = numeric_limits<double>::max();
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

double SimpleLine::Distance(const SimpleLine& sl,
                            const Geoid* geoid /* =0 */ )const{
  assert( !IsEmpty() );
  assert( !sl.IsEmpty() );
  assert( ! geoid || geoid->IsDefined() );
  HalfSegment hs1, hs2;
  double result = numeric_limits<double>::max();
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

double SimpleLine::Distance(const Rectangle<2>& r,
                            const Geoid* geoid /*=0*/)const{
  assert( !IsEmpty() );
  assert( r.IsDefined() );
  assert( !geoid || geoid->IsDefined() );
  if(geoid){
    cout << __PRETTY_FUNCTION__ << ": Spherical geometry not implemented."
         <<endl;
    assert(false); // TODO: Implement spherical geometry case.
  }
  Line sll(0);
  toLine(sll);
  return sll.Distance( r, geoid );
}

bool SimpleLine::Intersects(const Rectangle<2>& r,
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


bool SimpleLine::AtPosition( double pos,
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

bool SimpleLine::AtPosition( double pos,
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

bool SimpleLine::AtPoint(const Point& p, const bool startsSmaller,
                         double& result,
                         const Geoid* geoid /*= 0*/) const{
  return AtPoint(p, startsSmaller, 0.0, result, geoid);
}

bool SimpleLine::AtPoint( const Point& p,
                          double& result,
                          double tolerance /*=0.0*/,
                          const Geoid* geoid /*=0*/) const {
  return AtPoint(p,this->startSmaller, tolerance, result, geoid);
}

bool SimpleLine::AtPoint( const Point& p,
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


void SimpleLine::SubLine( double pos1, double pos2, SimpleLine& l ) const {
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


void SimpleLine::SubLine( double pos1, double pos2,
                         bool startsSmaller, SimpleLine& l ) const {
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


void SimpleLine::Crossings( const SimpleLine& l, Points& result,
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


bool SimpleLine::Intersects(const SimpleLine& l,
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



bool SimpleLine::SelectInitialSegment( const Point &startPoint,
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

bool SimpleLine::SelectSubsequentSegment() {
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


bool SimpleLine::getWaypoint( Point &destination ) const{
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

void SimpleLine::fromLine(const Line& src)
{
  fromLine(src,true);
}

void SimpleLine::fromLine(const Line& src, const bool smaller)
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


void SimpleLine::toLine(Line& result)const{
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


void SimpleLine::SetPartnerNo(){
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

 bool SimpleLine::computePolyline(){
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
  vector<bool> used(size,false);
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


int SimpleLine::Compare(const Attribute* arg)const{
  const SimpleLine* line = static_cast<const SimpleLine*>(arg);
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

ostream& SimpleLine::Print(ostream& o)const{
  o << "SimpleLine def =" << IsDefined()
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

double SimpleLine::Length(const Geoid &geoid, bool& valid) const {
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

vector<SimpleLine>* SimpleLine::SplitAtPPoints(Points* pts)
{
  Point curPoint;
  double pos;
  SimpleLine res(0);
  vector<SimpleLine>* result = new vector<SimpleLine>();
  result->clear();
  vector<double>* splitPositions= new vector<double>();
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

ostream& operator<<(ostream& o, const SimpleLine& cl){
   cl.Print(o);
   return o;
}

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
 if(nl->HasLength(instance,2)){
   if(nl->AtomType(nl->Second(instance))==BoolType){
     startSmaller = nl->BoolValue(nl->Second(instance));
     instance = nl->First(instance);
   }
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
   line->DeleteIfAllowed();
   return SetWord(Address(0));
 }else{
   line->SetStartSmaller(startSmaller);
   return SetWord(line);
 }
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


/*
8 Type Constructor ~region~

A ~region~ value is a set of halfsegments. In the external (nestlist)
representation, a region value is expressed as a set of faces, and each
face is composed of a set of cycles.  However, in the internal
(class) representation, it is expressed as a set of sorted halfsegments,
which are stored as a PArray.

8.1 Implementation of the class ~region~

*/
Region::Region( const Region& cr, bool onlyLeft ) :
StandardSpatialAttribute<2>(cr.IsDefined()),
region( cr.Size() ),
bbox( cr.bbox ),
noComponents( cr.noComponents ),
ordered( true )
{
  if( IsDefined() && cr.Size() >0 ) {
    assert( cr.IsOrdered() );
    if( !onlyLeft ){
      region.copyFrom(cr.region);
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

Region::Region( const Rectangle<2>& r )
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

Region::Region( const Point& p1, const Point& p2, const Point& p3 ):
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

void Region::StartBulkLoad()
{
  ordered = false;
}

void Region::EndBulkLoad( bool sort, bool setCoverageNo,
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


  if( computeRegion )
    ComputeRegion();

  region.TrimToSize();
  ordered = true;
}

bool Region::Contains( const Point& p, const Geoid* geoid/*=0*/ ) const
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
  map<int, int> faceISN;
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

  for( map<int, int>::iterator iter = faceISN.begin();
       iter != faceISN.end();
       iter++ ){
    if( iter->second % 2 != 0 ){
      return true;
    }
  }
  return false;
}

bool Region::InnerContains( const Point& p, const Geoid* geoid/*=0*/ ) const
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
  map<int, int> faceISN;
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

  for( map<int, int>::iterator iter = faceISN.begin();
       iter != faceISN.end();
       iter++ ){
    if( iter->second % 2 != 0 ){
      return true;
    }
  }
  return false;
}

bool Region::Intersects( const HalfSegment& inHs,
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


bool Region::Contains( const HalfSegment& hs, const Geoid* geoid/*=0*/ ) const
{
  assert( IsDefined() );
  assert( !geoid || geoid->IsDefined() );
  if(geoid){
    cerr << __PRETTY_FUNCTION__ << ": Spherical geometry not implemented!."
         << endl;
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
  vector<Point> intersection_points;
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

bool Region::InnerContains( const HalfSegment& hs,
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

bool Region::HoleEdgeContain( const HalfSegment& hs,
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
bool Region::Intersects( const Region &r, const Geoid* geoid/*=0*/ ) const
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


void Region::Intersection(const Point& p, Points& result,
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

void Region::Intersection(const Points& ps, Points& result,
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

void Region::Intersection(const Line& l, Line& result,
                          const Geoid* geoid/*=0*/) const{

  try{
     SetOp(l,*this,result,avlseg::intersection_op, geoid, true);
  } catch (exception& e){
    // compute intersection using robust implementation
    cerr << "Problem during plane sweep detected, "
         << "switch to slow implementation" << endl;
    robust::intersection(*this,l,result);
  }


}


void Region::Intersection(const Region& r, Region& result,
                          const Geoid* geoid/*=0*/) const{
  SetOp(*this,r,result,avlseg::intersection_op, geoid);
}

void Region::Intersection(const SimpleLine& l, SimpleLine& result,
                          const Geoid* geoid/*=0*/) const{
  SetOp(l,*this,result,avlseg::intersection_op, geoid);
}

void Region::Union(const Point& p, Region& result,
                   const Geoid* geoid/*=0*/) const{
  if(!IsDefined() || !p.IsDefined() || (geoid && !geoid->IsDefined()) ){
    result.Clear();
    result.SetDefined(false);
    return;
  }
  result.SetDefined(true);
  result.CopyFrom(this);
}

void Region::Union(const Points& ps, Region& result,
                   const Geoid* geoid/*=0*/) const{
  if(!IsDefined() || !ps.IsDefined() || (geoid && !geoid->IsDefined()) ){
    result.Clear();
    result.SetDefined(false);
    return;
  }
  result.SetDefined(true);
  result.CopyFrom(this);
}

void Region::Union(const Line& line, Region& result,
                   const Geoid* geoid/*=0*/) const{
  if(!IsDefined() || !line.IsDefined() || (geoid && !geoid->IsDefined()) ){
    result.Clear();
    result.SetDefined(false);
    return;
  }
  result.SetDefined(true);
  result.CopyFrom(this);
}

void Region::Union(const Region& region, Region& result,
                   const Geoid* geoid/*=0*/) const{
   SetOp(*this,region,result,avlseg::union_op, geoid);
}

void Region::Union(const SimpleLine& line, Region& result,
                   const Geoid* geoid/*=0*/) const{
  if(!IsDefined() || !line.IsDefined() || (geoid && !geoid->IsDefined()) ){
    result.Clear();
    result.SetDefined(false);
    return;
  }
  result.SetDefined(true);
  result.CopyFrom(this);
}

void Region::Minus(const Point& p, Region& result,
                   const Geoid* geoid/*=0*/) const{
  if(!IsDefined() || !p.IsDefined() || (geoid && !geoid->IsDefined()) ){
    result.Clear();
    result.SetDefined(false);
    return;
  }
  result.SetDefined(true);
  result.CopyFrom(this);
}

void Region::Minus(const Points& ps, Region& result,
                   const Geoid* geoid/*=0*/) const{
  if(!IsDefined() || !ps.IsDefined() || (geoid && !geoid->IsDefined()) ){
    result.Clear();
    result.SetDefined(false);
    return;
  }
  result.SetDefined(true);
  result.CopyFrom(this);
}

void Region::Minus(const Line& line, Region& result,
                   const Geoid* geoid/*=0*/) const{
  if(!IsDefined() || !line.IsDefined() || (geoid && !geoid->IsDefined()) ){
    result.Clear();
    result.SetDefined(false);
    return;
  }
  result.SetDefined(true);
  result.CopyFrom(this);
}

void Region::Minus(const Region& region, Region& result,
                   const Geoid* geoid/*=0*/) const{
   SetOp(*this,region,result,avlseg::difference_op, geoid);
}

void Region::Minus(const SimpleLine& line, Region& result,
                   const Geoid* geoid/*=0*/) const{
  if(!IsDefined() || !line.IsDefined() || (geoid && !geoid->IsDefined()) ){
    result.Clear();
    result.SetDefined(false);
    return;
  }
  result.SetDefined(true);
  result.CopyFrom(this);
}

bool Region::Inside( const Region& r, const Geoid* geoid/*=0*/ ) const
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

bool Region::Adjacent( const Region& r,
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

bool Region::Overlaps( const Region& r, const Geoid* geoid/*=0*/ ) const
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

bool Region::OnBorder( const Point& p, const Geoid* geoid/*=0*/ ) const
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

bool Region::InInterior( const Point& p, const Geoid* geoid/*=0*/ ) const
{
  return InnerContains( p, geoid );
}

double Region::Distance( const Point& p, const Geoid* geoid /*=0*/ ) const
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
  double result = numeric_limits<double>::max();

  for( int i = 0; i < Size(); i++ )
  {
    Get( i, hs );

    if( hs.IsLeftDomPoint() )
      result = MIN( result, hs.Distance( p, geoid ) );
  }
  return result;
}

double Region::Distance( const Rectangle<2>& r,
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
  double mindist = numeric_limits<double>::max();
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


bool Region::Intersects( const Rectangle<2>& r,
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




double Region::Area(const Geoid* geoid/*=0*/) const
{
  assert( IsDefined() );
  assert( !geoid || geoid->IsDefined() );
  if(geoid){
    cerr << __PRETTY_FUNCTION__ << ": Spherical geometry not implemented."
         << endl; //TODO: Implement spherical geometry case.
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

double Region::Distance( const Points& ps, const Geoid* geoid /*=0*/ ) const
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

  double result = numeric_limits<double>::max();
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

double Region::Distance( const Region &r, const Geoid* geoid /*=0*/ ) const
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
  double result = numeric_limits<double>::max();
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


double Region::Distance( const Line &l, const Geoid* geoid /*=0*/ ) const
{
  assert( !IsEmpty() ); // subsumes IsDefined()
  assert( !l.IsEmpty() ); // subsumes IsDefined()
  assert( !geoid || geoid->IsDefined() );
  if(geoid){
    cout << __PRETTY_FUNCTION__ << ": Spherical geometry not implemented."
         << endl;
    assert(false); // TODO: Implement spherical geometry case.
  }
  if( IsEmpty() || l.IsEmpty() || (geoid && !geoid->IsDefined()) ) {
     return -1;
  }
  double result = numeric_limits<double>::max();
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



void Region::Components( vector<Region*>& components )
{
  components.clear();
  if( IsEmpty() ) { // subsumes IsDefined()
    return;
  }
  for(int i=0;i<noComponents;i++){
   components.push_back(new Region(1));
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

void Region::getHoles(Region& result) const{
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




void Region::Translate( const Coord& x, const Coord& y, Region& result ) const
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

void Region::Rotate( const Coord& x, const Coord& y,
                   const double alpha,
                   Region& result ) const
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

void Region::TouchPoints( const Line& l, Points& result,
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

void Region::TouchPoints( const Region& r, Points& result,
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
  Point p;
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
  result.EndBulkLoad( true, false );
}

void Region::CommonBorder( const Region& r, Line& result,
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

int Region::NoComponents() const
{
  assert( IsDefined() );
  return noComponents;
}

void Region::Vertices( Points* result, const Geoid* geoid/*=0*/ ) const
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


void Region::Boundary( Line* result, const Geoid* geoid/*=0*/ ) const
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



Region& Region::operator=( const Region& r )
{
  assert( r.IsOrdered() );
  region.copyFrom(r.region);
  bbox = r.bbox;
  noComponents = r.noComponents;
  del.isDefined = r.del.isDefined;
  return *this;
}

bool Region::operator==( const Region& r ) const
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

bool Region::operator!=( const Region &cr) const
{
  return !(*this==cr);
}

Region& Region::operator+=( const HalfSegment& hs )
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

Region& Region::operator-=( const HalfSegment& hs )
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

bool Region::Find( const HalfSegment& hs, int& pos ) const
{
  assert( IsOrdered() );
  assert(IsDefined());
  return region.Find( &hs, HalfSegmentCompare, pos );
}

bool Region::Find( const Point& p, int& pos ) const
{
  assert( IsOrdered() );
  assert( IsDefined());
  return region.Find( &p, PointHalfSegmentCompare, pos );
}

void Region::Sort()
{
  if(!IsDefined()){
    return;
  }
  assert( !IsOrdered() );

  region.Sort( HalfSegmentCompare );

  ordered = true;
}

void Region::LogicSort()
{

  if(!IsDefined()){
   return;
  }
  region.Sort( HalfSegmentLogicCompare );

  ordered = true;
}

ostream& operator<<( ostream& os, const Region& cr )
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

void Region::SetPartnerNo()
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
  Point v;
  AttrType attr;
  attr = hs.GetAttr();
  Point p2;
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

void Region::CreateNewSegments(vector <EdgePoint>pointsOnEdge, Region &cr,
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

void Region::CreateNewSegmentsWindowVertices(const Rectangle<2> &window,
                                vector<EdgePoint> pointsOnEdge[4],Region &cr,
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

bool Region::ClippedHSOnEdge(const Rectangle<2> &window,const HalfSegment &hs,
                             bool clippingIn,vector<EdgePoint> pointsOnEdge[4],
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

bool Region::GetCycleDirection( const Point &pA,
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

bool Region::GetCycleDirection() const
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
  Point pA, pP, pB;
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




void Region::GetClippedHSIn(const Rectangle<2> &window,
                            Region &clippedRegion,
                            vector<EdgePoint> pointsOnEdge[4],
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

void Region::AddClippedHS( const Point &pl,
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

void Region::GetClippedHSOut(const Rectangle<2> &window,
                             Region &clippedRegion,
                             vector<EdgePoint> pointsOnEdge[4],
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

void Region::GetClippedHS(const Rectangle<2> &window,
                          Region &clippedRegion,
                          bool inside,
                          const Geoid* geoid/*=0*/) const
{
  vector<EdgePoint> pointsOnEdge[4];
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

bool Region::IsCriticalPoint( const Point &adjacentPoint,
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

bool Region::GetAdjacentHS( const HalfSegment &hs,
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
    if ( position<0 || position>=this->Size())
      break;

    Get(position,adjacentCHS);
    if (partnernoP == position)
      continue;

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

void Region::ComputeCycle( HalfSegment &hs,
                           int faceno,
                           int cycleno,
                           int &edgeno,
                           bool *cycle )
{

  Point nextPoint = hs.GetLeftPoint(),
            lastPoint = hs.GetRightPoint(),
            previousPoint, *currentCriticalPoint=NULL;
  AttrType attr, attrP;
  HalfSegment hsP;
  vector<SCycle> sCycleVector;
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

       if (this->IsCriticalPoint(nextPoint,attrP.partnerno))
         currentCriticalPoint = new Point(nextPoint);

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
//       cout<<"flag 1 "<<adjacentPointFound<<" p1 "<<previousPoint<<endl;
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
//       cout<<"flag 2 "<<adjacentPointFound<<" p2 "<<previousPoint<<endl;
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
//       cout<<"flag 3 "<<adjacentPointFound<<" p3 "<<previousPoint<<endl;
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

//      cout<<"flag 4 "<<adjacentPointFound<<" p4 "<<previousPoint<<endl;

     }

     if(!adjacentPointFound){
         cerr<<"previousPoint "<<previousPoint<<endl;
         cerr << "Problem in rebuilding cycle in a region " << endl;
         cerr << "no adjacent point found" << endl;
         cerr << "Halfsegments : ---------------     " << endl;
         HalfSegment hs;
         for(int i=0;i<Size();i++){
            Get(i,hs);
            cerr << i << " : " << (hs) << endl;
         }
         assert(adjacentPointFound); // assert(false)
     }
     sCycleVector.push_back(*s);

     if ( (currentCriticalPoint!=NULL) && (*currentCriticalPoint==nextPoint) )
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
          if (firstSCycle.criticalPoint==NULL)
            break;
          if (*firstSCycle.criticalPoint!=*currentCriticalPoint)
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
int Region::GetNewFaceNo(HalfSegment &hsS, bool *cycle)
{
  int coverno=0;
  int startpos=0;
  double y0;
  AttrType attr;
  vector<HalfSegment> v;

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

int Region::GetNewFaceNo(const HalfSegment& hsIn, const int startpos) const {

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

        cerr << "Problem in rebuilding cycle in a region " << endl;
        cerr << "No outer cycle found" << endl;
        cerr << "hsIn: " << hsIn << endl;
        cerr << "Halfsegments : ---------------     " << endl;
        HalfSegment hs;

        for(int i=0;i<Size();i++) {

            Get(i,hs);
            cerr << i << " : " << (hs) << endl;
        }

        assert(false);
    }

    //the new cycle is a holecycle of the face ~maxHS.attr.faceno~
    return maxHS.GetAttr().faceno;
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



void Region::ComputeRegion()
{
  if( !IsDefined() )
    return;
  //array that stores in position i the last cycle number of the face i
  vector<int> face;
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

void Region::WindowClippingIn(const Rectangle<2> &window,
                              Region &clippedRegion,
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

void Region::WindowClippingOut(const Rectangle<2> &window,
                               Region &clippedRegion,
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

size_t   Region::HashValue() const
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

void Region::Clear()
{
  region.clean();
  pos = -1;
  ordered = true;
  bbox.SetDefined(false);
}

void Region::SetEmpty()
{
  region.clean();
  pos = -1;
  ordered = true;
  bbox.SetDefined(false);
  SetDefined(true);
}


void Region::CopyFrom( const Attribute* right )
{
  *this = *(const Region *)right;
}

int Region::Compare( const Attribute* arg ) const
{
  Region* cr = (Region* )(arg);
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

ostream& Region::Print( ostream &os ) const
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

Region *Region::Clone() const
{
  return new Region( *this );
}

bool Region::InsertOk( const HalfSegment& hs ) const
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

static vector<Point> getCycle(const bool isHole,
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


static vector< vector <Point> > getCycles(const Region& reg){
      // first step , map halsfsegment according to faceno and cycleno

      map< pair<int, int> , vector<HalfSegment> > m;

      HalfSegment hs;
      for(int i=0;i<reg.Size(); i++){
         reg.Get(i,hs);
         if(hs.IsLeftDomPoint()){
            int faceno = hs.attr.faceno;
            int cycleno = hs.attr.cycleno;
            m[make_pair(faceno, cycleno)].push_back(hs);
         }
      }

      vector< vector <Point> > result;
      map< pair<int, int> , vector<HalfSegment> >::iterator it;
      for(it=m.begin(); it!=m.end(); it++){
        pair< pair<int, int> , vector<HalfSegment> > cycleDesc = *it;
        bool isHole = cycleDesc.first.second > 0;
        result.push_back(getCycle(isHole, cycleDesc.second));
      }
      return result;

   }


void Region::saveShape(ostream& o, uint32_t RecNo) const
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

        vector<vector < Point> > cycles = getCycles(*this);

        uint32_t numParts = cycles.size();


        uint32_t  numPoints = 0;

        vector<vector < Point> >::iterator it;
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
           vector<Point> cycle = cycles[i];
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
  Point p1,p2;
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
  double f1,f2,ft;
  do
  {
    f1=newF(f,x,y,p);
    f2=f;
    f=f1;
    ft=180*f1/Pi;
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
    Point outputP, leftoverP;

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

          Point *currvertex,p1,p2,firstP;

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
ListExpr
SpatialDistanceMap( ListExpr args )
{
  int noargs = nl->ListLength(args);
  string errmsg = "Expected (T1 x T2 [x geoid]) where T1,T2 in "
                  "{point,points,line,sline,region,rectangle}.";

  if( (noargs < 2) || (noargs > 3) ){
    return listutils::typeError(errmsg);
  }

  ListExpr arg1, arg2;
  arg1 = nl->First( args );
  arg2 = nl->Second( args );

  SpatialType t1 = SpatialTypeOfSymbol(arg1);
  SpatialType t2 = SpatialTypeOfSymbol(arg2);
  ListExpr erg = nl->SymbolAtom(CcReal::BasicType());

  if ( (t1 == stpoint) && (t2 == stpoint) ){
    return erg;
  }else if ( (t1  == stpoint) && (t2 == stpoints) ){
    return erg;
  }else if ( (t1 == stpoints) && (t2 == stpoint )) {
    return erg;
  } else if ( (t1 == stpoint) && (t2 == stline ) ){
    return erg;
  } else if( (t1 == stline) && ( t2 ==  stpoint )){
    return erg;
  } else if( (t1  == stpoint) && (t2 == stregion )) {
    return erg;
  } else if ( (t1  == stregion) && ( t2 == stpoint )){
    return erg;
  } else if ( ( t1 == stpoints) && ( t2 == stpoints )){
    return erg;
  } else if ( (t1 == stpoints) && (t2 == stline )){
    return erg;
  } else  if ( ( t1 == stline )&&( t2  == stpoints )){
    return erg;
  } else if ( (t1 == stpoints) && ( t2  == stregion )){
    return erg;
  } else if ( (t1  == stregion) && (t2 == stpoints )){
    return erg;
  } else if ( (t1 == stline) && (t2 == stline )){
    return erg;
  } else if ( ( t1 == stregion) && ( t2 == stregion )){
    return erg;
  } else if ( (t1 == stsline) && (t2 == stpoint)){
    return erg;
  } else if( ( t1 == stpoint) && (t2 == stsline)){
    return erg;
  } else if(( t1 == stsline) && ( t2 == stpoints)){
    return erg;
  } else if((t1==stpoints) && (t2==stsline)){
    return erg;
  } else if((t1==stsline) && (t2==stsline)){
    return erg;
  } else if((t1==stbox) && (t2==stpoint)){
    return erg;
  } else if((t1==stpoint) && (t2==stbox)){
    return erg;
  } else if((t1==stbox) && (t2==stpoints)){
    return erg;
  } else if((t1==stpoints) && (t2==stbox)){
    return erg;
  } else if((t1==stbox) && (t2==stline)){
    return erg;
  } else if((t1==stline) && (t2==stbox)){
    return erg;
  } else if((t1==stbox) && (t2==stregion)){
    return erg;
  } else if((t1==stregion) && (t2==stbox)){
    return erg;
  } else if((t1==stbox) && (t2==stsline)){
    return erg;
  } else if((t1==stsline) && (t2==stbox)){
    return erg;
  }
  return listutils::typeError(errmsg);
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
                              nl->SymbolAtom(Point::BasicType()) );

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
       SpatialTypeOfSymbol( arg1 ) != stsline ){
    return listutils::typeError(errmsg);
  }
  if( (noargs == 2) &&
      !listutils::isSymbol(nl->Second(args),Geoid::BasicType()) ) {
    return listutils::typeError(errmsg);
  }
  return (nl->SymbolAtom( Rectangle<2>::BasicType() ));
}

/*
10.1.17 Type mapping function for operator ~translate~

This type mapping function is used for the ~translate~ operator. This operator
moves a region parallelly to another place and gets another region.

*/
ListExpr
SpatialTranslateMap( ListExpr args )
{
  ListExpr arg1, arg2;
  if ( nl->ListLength( args ) == 2 )
  {
    arg1 = nl->First( args );
    arg2 = nl->Second( args );
    if(!nl->HasLength(arg2,2)){
       return listutils::typeError("Invalid number of parameter");
    }

    if( SpatialTypeOfSymbol( arg1 ) == stregion &&
        nl->IsEqual(nl->First( arg2 ), CcReal::BasicType()) &&
        nl->IsEqual(nl->Second( arg2 ), CcReal::BasicType()))
      return (nl->SymbolAtom( Region::BasicType() ));

    if( SpatialTypeOfSymbol( arg1 ) == stline &&
        nl->IsEqual(nl->First( arg2 ), CcReal::BasicType()) &&
        nl->IsEqual(nl->Second( arg2 ), CcReal::BasicType()))
      return (nl->SymbolAtom( Line::BasicType() ));

    if( SpatialTypeOfSymbol( arg1 ) == stpoints &&
        nl->IsEqual(nl->First( arg2 ), CcReal::BasicType()) &&
        nl->IsEqual(nl->Second( arg2 ), CcReal::BasicType()))
      return (nl->SymbolAtom( Points::BasicType() ));

    if( SpatialTypeOfSymbol( arg1 ) == stpoint &&
        nl->IsEqual(nl->First( arg2 ), CcReal::BasicType()) &&
        nl->IsEqual(nl->Second( arg2 ), CcReal::BasicType()))
      return (nl->SymbolAtom( Point::BasicType() ));
  }
  return listutils::typeError("");
}

/*
10.1.17 Type mapping function for operator ~rotate~

This type mapping function is used for the ~rotate~ operator.
The mamp is spatialtype x real x real x real -> spatialtype

*/
ListExpr
SpatialRotateMap( ListExpr args )
{
  if ( nl->ListLength( args ) != 4 )
  { ErrorReporter::ReportError("wrong number of arguments (4 expected)");
    return nl->TypeError();
  }
  ListExpr arg1 = nl->First(args);
  ListExpr arg2 = nl->Second(args);
  ListExpr arg3 = nl->Third(args);
  ListExpr arg4 = nl->Fourth(args);

  if( !nl->IsEqual(arg2,CcReal::BasicType()) ||
      !nl->IsEqual(arg3,CcReal::BasicType()) ||
      !nl->IsEqual(arg4,CcReal::BasicType())){
    ErrorReporter::ReportError("spatial x real x real x real expected");
    return nl->TypeError();
  }

  if(!nl->AtomType(arg1)==SymbolType){
    ErrorReporter::ReportError("spatial x real x real x real expected");
    return nl->TypeError();
  }
  string st = nl->SymbolValue(arg1);
  if( st!=Point::BasicType() && st!=Points::BasicType() &&
      st!=Line::BasicType() && st!=Region::BasicType()){
    ErrorReporter::ReportError("spatial x real x real x real expected");
    return nl->TypeError();
  }
  return nl->SymbolAtom(st);

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
  ListExpr arg1, arg2;
  if ( nl->ListLength( args ) == 2 )
  {
    arg1 = nl->First( args );
    arg2 = nl->Second( args );

    if ( SpatialTypeOfSymbol( arg1 ) == stline)
        return (nl->SymbolAtom( Line::BasicType() ));

    if ( SpatialTypeOfSymbol( arg1 ) == stregion )
        return (nl->SymbolAtom( Region::BasicType() ));
  }
  return listutils::typeError("");
}

/*
10.1.18 Type mapping function for operator ~scale~

This type mapping function is used for the ~scale~ operator. This operator
scales a spatial object by a given factor.

*/
ListExpr SpatialScaleMap(ListExpr args)
{
   if(nl->ListLength(args)!=2){
      return listutils::typeError("operator scale requires two arguments");
   }
   ListExpr arg1 = nl->First(args);
   ListExpr arg2 = nl->Second(args);
   if(!(nl->IsEqual(arg2 , CcReal::BasicType()))){
      return listutils::typeError("expectes real as 2nd");
   }
   if(nl->IsEqual(arg1,Region::BasicType()))
     return nl->SymbolAtom(Region::BasicType());
   if(nl->IsEqual(arg1,Line::BasicType()))
     return nl->SymbolAtom(Line::BasicType());
   if(nl->IsEqual(arg1,Point::BasicType()))
     return nl->SymbolAtom(Point::BasicType());
   if(nl->IsEqual(arg1,Points::BasicType()))
     return nl->SymbolAtom(Points::BasicType());
   return listutils::typeError("Expected 1st to be of {region, line, "
                               "points, points}");}

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
   if((len!=2) && (len!=3)){
     return listutils::typeError("invalid number of"
                                 " arguments (has to be 2 or 3 )");
   }
   if(!nl->IsEqual(nl->First(args),Line::BasicType()) ||
      !nl->IsEqual(nl->Second(args),CcReal::BasicType())){
     return listutils::typeError("line x real [x points] expected");
   }
   if( (len==3) &&
       !(nl->IsEqual(nl->Third(args),Points::BasicType()))){
     return listutils::typeError("line x real [ x points] expected");
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
  if(!nl->IsEqual(nl->First(args),Line::BasicType())){
    return listutils::typeError("line expected");
  }
  return nl->TwoElemList(
               nl->SymbolAtom(Symbol::STREAM()),
               nl->SymbolAtom(Line::BasicType())
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
    return listutils::typeError("two arguments expected, but got " + len);
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
    return listutils::typeError("two arguments expected, but got " + len);
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
int
SpatialSelectDistance( ListExpr args )
{
  ListExpr arg1 = nl->First( args );
  ListExpr arg2 = nl->Second( args );

  SpatialType st1 = SpatialTypeOfSymbol( arg1 );
  SpatialType st2 = SpatialTypeOfSymbol( arg2 );

  if ( st1 == stpoint && st2 == stpoint ) return 0;

  if ( st1  == stpoint && st2 == stpoints ) return 1;

  if ( st1 == stpoint && st2 == stline ) return 2;

  if ( st1 == stpoint && st2 == stregion ) return 3;

  if ( st1 == stpoints && st2 == stpoint ) return 4;

  if ( st1 == stpoints && st2 == stpoints ) return 5;

  if ( st1 == stpoints && st2 == stline ) return 6;

  if ( st1 == stpoints && st2 == stregion ) return 7;

  if ( st1 == stline && st2 == stpoint ) return 8;

  if ( st1 == stline && st2 == stpoints ) return 9;

  if ( st1 == stline && st2 == stline ) return 10;

  if ( st1 == stregion && st2 == stpoint ) return 11;

  if ( st1 == stregion && st2 == stpoints ) return 12;

  if ( st1 == stregion && st2 == stregion ) return 13;

  if( st1 == stsline && st2 == stpoint ) return 14;

  if( st1 == stpoint && st2 == stsline) return 15;

  if( st1 == stsline && st2 == stpoints) return 16;

  if( st1 == stpoints && st2 == stsline ) return 17;

  if( st1 == stsline && st2 == stsline) return  18;

  if( st1 == stbox && st2 == stpoint ) return  19;

  if( st1 == stpoint && st2 == stbox ) return  20;

  if( st1 == stbox && st2 == stpoints ) return  21;

  if( st1 == stpoints && st2 == stbox ) return  22;

  if( st1 == stbox && st2 == stline ) return  23;

  if( st1 == stline && st2 == stbox ) return  24;

  if( st1 == stbox && st2 == stregion ) return  25;

  if( st1 == stregion && st2 == stbox ) return  26;

  if( st1 == stbox && st2 == stsline ) return  27;

  if( st1 == stsline && st2 == stbox ) return  28;

  // (rect2 x rect2) has already been implemented in the RectangleAlgebra!

  return -1; // This point should never be reached
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
10.3.19 Selection function ~SpatialSelectTranslate~

This select function is used for the ~translate~, rotate,
and ~scale~ operators.

*/
int
SpatialSelectTranslate( ListExpr args )
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
   } else {
      return 1; // line x real x points
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
  Point p;
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
  double d = p1->Direction(*p2, isHeading, geoid); // all saveguards included!
  res->Set(d>=0.0,d);
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
  const Region *r1 = ((const Region*)args[0].addr),
               *r2 = ((const Region*)args[1].addr);
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
10.4.26 Value mapping functions of operator ~translate~

*/
int SpatialTranslate_p( Word* args, Word& result, int message,
                        Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  Point *res = static_cast<Point*>(result.addr);
  const Point *p= (const Point*)args[0].addr;

  Supplier son = qp->GetSupplier( args[1].addr, 0 );
  Word t;
  qp->Request( son, t );
  const CcReal *tx = ((CcReal *)t.addr);

  son = qp->GetSupplier( args[1].addr, 1 );
  qp->Request( son, t );
  const CcReal *ty = ((CcReal *)t.addr);

  if( p->IsDefined() && tx->IsDefined() && ty->IsDefined()){
     *res = *p;
     res->Translate( tx->GetRealval(),  ty->GetRealval() );
  } else {
     res->SetDefined( false );
  }
  return 0;
}

int SpatialTranslate_ps( Word* args, Word& result, int message,
                         Word& local, Supplier s )
{
  result = qp->ResultStorage( s );

  const Points *ps = (const Points*)args[0].addr;

  Supplier son = qp->GetSupplier( args[1].addr, 0 );
  Word t;
  qp->Request( son, t );
  const CcReal *tx = ((CcReal *)t.addr);

  son = qp->GetSupplier( args[1].addr, 1 );
  qp->Request( son, t );
  const CcReal *ty = ((CcReal *)t.addr);

  if( ps->IsDefined() && tx->IsDefined() && ty->IsDefined() )
      ps->Translate( tx->GetRealval(),
                     ty->GetRealval(),
                     *((Points*)result.addr) );
  else
    ((Points*)result.addr)->SetDefined( false );

  return 0;
}

int SpatialTranslate_l( Word* args, Word& result, int message,
                        Word& local, Supplier s )
{
  result = qp->ResultStorage( s );

  Line *cl = (Line *)args[0].addr,
       *pResult = (Line *)result.addr;

  Supplier son = qp->GetSupplier( args[1].addr, 0 );
  Word t;
  qp->Request( son, t );
  const CcReal *tx = ((CcReal *)t.addr);

  son = qp->GetSupplier( args[1].addr, 1 );
  qp->Request( son, t );
  const CcReal *ty = ((CcReal *)t.addr);

  if(  cl->IsDefined()&& tx->IsDefined() && ty->IsDefined() ) {
      const Coord txval = (Coord)(tx->GetRealval()),
                  tyval = (Coord)(ty->GetRealval());
      cl->Translate( txval, tyval, *pResult );
  }
  else
    ((Line*)result.addr)->SetDefined( false );

  return 0;
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
10.4.27 Value Mapping functions of the Operator Scale

*/
int SpatialScale_p( Word* args, Word& result, int message,
                    Word& local, Supplier s ){
  result = qp->ResultStorage(s);
  Point* p = (Point*) args[0].addr;
  CcReal*  factor = (CcReal*) args[1].addr;
  Point* res = (Point*) result.addr;
  if ( !p->IsDefined() || !factor->IsDefined() )
  {
    res->SetDefined(false);
  }
  else
  {
    res->SetDefined(true);
    res->Set(p->GetX(),p->GetY());
    double f = factor->GetRealval();
    res->Scale(f);
  }
  return 0;
}

int SpatialScale_ps( Word* args, Word& result, int message,
                     Word& local, Supplier s ){
  result = qp->ResultStorage(s);
  Points* p = (Points*) args[0].addr;
  CcReal*  factor = (CcReal*) args[1].addr;
  Points* res = (Points*) result.addr;
  if( !p->IsDefined() || !factor->IsDefined() )
  {
    res->SetDefined(false);
  }
  else
  {
    res->SetDefined(true);
    double f = factor->GetRealval();
    // make res empty if it is not already
    if(!res->IsEmpty()){
       Points P(0);
       (*res) = P;
    }
    if(!p->IsEmpty()){
       res->StartBulkLoad();
       int size = p->Size();
       Point PTemp;
       for(int i=0;i<size;i++){
           p->Get(i,PTemp);
           Point aux( PTemp );
           aux.Scale(f);
           (*res) += aux;
        }
        res->EndBulkLoad();
    }
  }
  return 0;
}

int SpatialScale_l( Word* args, Word& result, int message,
                    Word& local, Supplier s ){
  result = qp->ResultStorage(s);
  Line* L = (Line*) args[0].addr;
  CcReal* factor = (CcReal*) args[1].addr;
  Line* res = (Line*) result.addr;
  if( !L->IsDefined() || !factor->IsDefined() )
  {
    res->SetDefined(false);
  }
  else
  {
    res->SetDefined(true);
    double f = factor->GetRealval();
    // delete result if not empty
    if(!res->IsEmpty()){
       Line Lempty(0);
       (*res) = Lempty;
    }
    if(!L->IsEmpty()){
       res->StartBulkLoad();
       int size = L->Size();
       HalfSegment hs;
       for(int i=0;i<size;i++){
         L->Get(i,hs);
         HalfSegment aux( hs );
         aux.Scale(f);
         (*res) += aux;
       }
       res->EndBulkLoad();
    }
  }
  return 0;
}

int SpatialScale_r( Word* args, Word& result, int message,
                    Word& local, Supplier s ){
  result    = qp->ResultStorage(s);
  Region *R      = (Region*) args[0].addr;
  CcReal *factor = (CcReal*) args[1].addr;
  Region *res    = (Region*) result.addr;
  if( !R->IsDefined() || !factor->IsDefined() )
  {
    res->SetDefined(false);
  }
  else
  {
    res->Clear();
    res->SetDefined(true);
    double f = factor->GetRealval();
    if(!R->IsEmpty()){
       res->StartBulkLoad();
       int size = R->Size();
       HalfSegment hs;
       for(int i=0;i<size;i++){
         R->Get(i,hs);
         hs.Scale(f);
         (*res) += hs;
       }
      res->EndBulkLoad();
    }
  }
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

      Point p;
      localInfo->GetPt( p );
      result.addr = new Point( p );
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
10.4.29 Value mapping function for the ~polylines~ operator

*/

class LineSplitter{
public:
/*
~Constructor~

Creates a LineSplitter from the given line.

*/
   LineSplitter(Line* line, bool ignoreCriticalPoints, bool allowCycles,
                Points* points = 0){
        this->theLine = line;
        size = line->Size();
        lastPos =0;
        this->points = points;
        used = new bool[size];
        memset(used,false,size);
        this->ignoreCriticalPoints = ignoreCriticalPoints;
        this->allowCycles = allowCycles;
   }

/*
~Destroys~ the lineSplitter

*/

   ~LineSplitter(){
      delete [] used;
      points = 0;
    }

/*
~NextLine~

This function extracts the next simple part of the line.
If the line is processed completely, the result will be
0. This function creates a new line instance via the new
operator. The caller of this function has to ensure the
deletion of this object.

*/
    Line* NextLine(){
      // go to the first unused halfsegment
      while(lastPos<size && used[lastPos]){
        lastPos++;
      }
      if(lastPos>=size){
         return 0;
      }
      // unused segment found,  construct a new Line
      int maxSize = max(1,size - lastPos);
      Line* result = new Line(maxSize);
      set<Point> pointset;
      int pos = lastPos;
      bool done = false;
      result->Clear();
      result->StartBulkLoad();
      HalfSegment hs1;
      HalfSegment hs2; // partner of hs1
      int edgeno = 0;
      bool seconddir = false;
      theLine->Get(pos,hs1);
      Point firstPoint = hs1.GetDomPoint();
      bool isCycle = false;


      while(!done){ // extension possible
        theLine->Get(pos,hs1);
        int partnerpos = hs1.GetAttr().partnerno;
        theLine->Get(partnerpos, hs2);
        Point p1 = hs1.GetDomPoint();
        pointset.insert(p1);
        Point p = hs2.GetDomPoint();
        pointset.insert(p);
        // add the halfsegments to the result
        HalfSegment Hs1 = hs1;
        HalfSegment Hs2 = hs2;
        AttrType attr1 = Hs1.GetAttr();
        attr1.edgeno = edgeno;
        Hs1.SetAttr(attr1);
        AttrType attr2 = Hs2.GetAttr();
        attr2.edgeno=edgeno;
        Hs2.SetAttr(attr2);
        edgeno++;
        (*result) += (Hs1);
        (*result) += (Hs2);
        // mark as used
        used[pos] = true;
        used[partnerpos] = true;

        if(isCycle){
           done = true;
        } else {
           bool found = false;
           int sp = partnerpos-1;

           if(points==0 || !points->Contains(p)){//no forced split

             // search for extension of the polyline
             // search left of partnerpos for an extension
             HalfSegment hs3;
             while(sp>0 && !found){
               if(!used[sp]){
                 theLine->Get(sp,hs3);
                 if(AlmostEqual(p,hs3.GetDomPoint())){
                   Point p3 = hs3.GetSecPoint(); // cycles?
                   if(pointset.find(p3)==pointset.end() ||
                     (allowCycles && AlmostEqual(p3,firstPoint))){
                     if(AlmostEqual(p3,firstPoint)){
                       isCycle = true;
                     }
                     found = true;
                   } else {
                     sp--;
                   }
                 } else {
                   sp = -1; // stop searching
                 }
               } else {
                 sp --; // search next
               }
             }
             // search on the right side
             if(!found){
                sp = partnerpos + 1;
                while(sp<size && !found){
                  if(!used[sp]){
                    HalfSegment hs3;
                    theLine->Get(sp,hs3);
                    if(AlmostEqual(p,hs3.GetDomPoint())){
                      Point p3 = hs3.GetSecPoint(); // avoid cycles
                      if(pointset.find(p3)==pointset.end() ||
                         (allowCycles && AlmostEqual(p3,firstPoint))){
                        if(AlmostEqual(p3,firstPoint)){
                          isCycle = true;
                        }
                        found = true;
                      } else {
                        sp++;
                      }
                    } else {
                        sp = size; // stop searching
                    }
                  } else {
                    sp ++; // search next
                  }
                }
             }
        }

        if(found){ // sp is a potential extension of the line
          if(ignoreCriticalPoints || !isCriticalPoint(partnerpos)){
            pos = sp;
          } else {
            done = true;
          }
        }  else { // no extension found
          done = true;
        }

        if(done && !seconddir && (lastPos < (size-1)) &&
           (points==0 || !points->Contains(firstPoint)) &&
           (!isCycle)){
           // done means at this point, the line can't be extended
           // in the direction start from the first selected halfsegment.
           // but is is possible the extend the line by going into the
           // reverse direction
           seconddir = true;
           HalfSegment hs;
           theLine->Get(lastPos,hs);
           Point p = hs.GetDomPoint();
           while(lastPos<size && used[lastPos]){
             lastPos ++;
           }
           if(lastPos <size){
             theLine->Get(lastPos,hs);
             Point p2 = hs.GetDomPoint();
             if(AlmostEqual(p,p2)){
               if(pointset.find(hs.GetSecPoint())==pointset.end()){
                 if(ignoreCriticalPoints || !isCriticalPoint(lastPos)){
                   pos = lastPos;
                   done = false;
                 }
               }
             }
           }
          }
        } // isCycle
      } // while
      result->EndBulkLoad();
      return result;
    }
private:
/*
~isCriticalPoint~

Checks whether the dominating point of the halfsegment at
position index is a critical one meaning a junction within the
line.

*/
   bool isCriticalPoint(int index){
      // check for critical point
      HalfSegment hs;
      theLine->Get(index,hs);
      Point  cpoint = hs.GetDomPoint();
      int count = 0;
      for(int i=max(0,index-2); i<= min(theLine->Size(),index+2) ; i++){
           if(i>=0 && i<size){
              theLine->Get(i,hs);
              if(AlmostEqual(cpoint, hs.GetDomPoint())){
                  count++;
              }
           }
       }
       return count>2;
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
   bool* used;
   Line* theLine;
   int lastPos;
   int size;
   bool ignoreCriticalPoints;
   Points* points;
   bool allowCycles;
};


template<bool allowCycles>
int SpatialPolylines(Word* args, Word& result, int message,
                    Word& local, Supplier s){

   LineSplitter *localinfo;
   Line   *l, *res;
   CcBool *b;

   result = qp->ResultStorage(s);
   switch (message){
      case OPEN:
          l = (Line*)args[0].addr;
          b = (CcBool*)args[1].addr;
          if(qp->GetNoSons(s)==2){
             if( !l->IsEmpty() && b->IsDefined() ) {
              local.setAddr(new LineSplitter(l,
                                  b->GetBoolval(),
                                  allowCycles));
             } else {
               local.setAddr( 0 );
             }
          } else if(    !l->IsEmpty()
                     && b->IsDefined()
                     && ((Points*)args[2].addr)->IsDefined() ){
            local.setAddr(new LineSplitter(l,
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
           localinfo = (LineSplitter*) local.addr;
           res = localinfo->NextLine();
           if(res==0){
              return CANCEL;
           } else {
              result.setAddr(res);
              return YIELD;
           }
      case CLOSE:
           if(local.addr!=0){
             localinfo = (LineSplitter*) local.addr;
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
class SegmentsInfo{
  public:
    SegmentsInfo(Line* line){
       this->theLine =(Line*) line->Copy(); // increase the ref counter of line
       this->position = 0;
       this->size = line->Size();
    }
    ~SegmentsInfo(){
      if(theLine!=0){
         theLine->DeleteIfAllowed(); // mark as free'd
      }
    }
    Line* NextSegment(){
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
  private:
     int position;
     int size;
     Line* theLine;
};


int SpatialSegments(Word* args, Word& result, int message,
                    Word& local, Supplier s){

 SegmentsInfo* si=0;
 Line* res =0;
 switch(message){
    case OPEN:
      if( !((Line*)args[0].addr)->IsEmpty() ) // subsumes undef
        local.addr = new SegmentsInfo((Line*)args[0].addr);
      else
        local.setAddr( 0 );
      return 0;

   case REQUEST:
      if( !local.addr )
        return CANCEL;
      si = (SegmentsInfo*) local.addr;
      res = si->NextSegment();
      if(res){
         result.setAddr(res);
         return YIELD;
      } else {
         return CANCEL;
      }

    case CLOSE:
      if(local.addr)
      {
        si = (SegmentsInfo*) local.addr;
        delete si;
        local.setAddr(0);
      }
      return 0;
 }
 return 0;
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
   if( line->IsDefined() && epsilon->IsDefined() ){
     line->Simplify( *res, epsilon->GetRealval() );
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
   if( line->IsDefined() && epsilon->IsDefined() && ps->IsDefined() )
     line->Simplify( *res, epsilon->GetRealval(), *ps);
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

   Point p;
   ps->Get(i,p);
   ((Point*)result.addr)->CopyFrom(&p);
   return 0;
}


/*
10.4.34 Value Mapping for the ~makeline~ and ~makesline~ operators

*/
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
     return 0;
  }
  res->StartBulkLoad();
  HalfSegment h(true, *p1, *p2);
  h.attr.edgeno = 0;
  (*res) += h;
  h.SetLeftDomPoint(false);
  (*res) += h;
  res->EndBulkLoad();
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
   Point p1;
   for(int i=0;i<p->Size();i++){
      p->Get(i,p1);
      Point p2;
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
   Point p1;
   for(int i=0;i<p->Size();i++){
      p->Get(i,p1);
      Point p2;
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
    Point p1;
    arg->Get(i,p1);
    Point p2;
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
  Point FirstPoint = *P0;
  Point SecondPoint = FirstPoint;
  Point LastPoint = FirstPoint;
  bool first = true;
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
    if (first)
    {
      SecondPoint = *P1;
      first = false;
    }
    LastPoint = *P1;
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
   if (L->IsCycle())
   {
     if (FirstPoint > SecondPoint) L->SetStartSmaller(false);
     else L->SetStartSmaller(true);
   }
   else
   {
    if(FirstPoint > LastPoint) L->SetStartSmaller(false);
    else L->SetStartSmaller(true);
   }

   qp->Close(args[0].addr);
   if(P0){ P0->DeleteIfAllowed(); P0 = 0; }
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
  SpatialDistance<Point,Point,false>,
  SpatialDistance<Points,Point,true>,
  SpatialDistance<Line,Point,true>,
  SpatialDistance<Region,Point,true>,
  SpatialDistance<Points,Point,false>,
  SpatialDistance<Points,Points,false>,
  SpatialDistance<Line,Points,true>,
  SpatialDistance<Region,Points,true>,
  SpatialDistance<Line,Point,false>,
  SpatialDistance<Line,Points,false>,
  SpatialDistance<Line,Line,false>,
  SpatialDistance<Region,Point,false>,
  SpatialDistance<Region,Points,false>,
  SpatialDistance<Region,Region,false>,
  SpatialDistance<SimpleLine,Point,false>,
  SpatialDistance<SimpleLine,Point, true>,
  SpatialDistance<SimpleLine, Points, false>,
  SpatialDistance<SimpleLine, Points, true>,
  SpatialDistance<SimpleLine, SimpleLine, false>,
  SpatialDistance<Point, Rectangle<2>, true>,
  SpatialDistance<Point, Rectangle<2>, false>,
  SpatialDistance<Points, Rectangle<2>, true>,
  SpatialDistance<Points, Rectangle<2>, false>,
  SpatialDistance<Line, Rectangle<2>, true>,
  SpatialDistance<Line, Rectangle<2>, false>,
  SpatialDistance<Region, Rectangle<2>, true>,
  SpatialDistance<Region, Rectangle<2>, false>,
  SpatialDistance<SimpleLine, Rectangle<2>, true>,
  SpatialDistance<SimpleLine, Rectangle<2>, false>
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
  SpatialBBox<SimpleLine> };

ValueMapping spatialtouchpointsmap[] = {
  SpatialTouchPoints_lr,
  SpatialTouchPoints_rl,
  SpatialTouchPoints_rr };

ValueMapping spatialtranslatemap[] = {
  SpatialTranslate_p,
  SpatialTranslate_ps,
  SpatialTranslate_l,
  SpatialTranslate_r };

ValueMapping spatialrotatemap[] = {
  SpatialRotate<Point>,
  SpatialRotate<Points>,
  SpatialRotate<Line>,
  SpatialRotate<Region>};

ValueMapping spatialwindowclippinginmap[] = {
  SpatialWindowClippingIn_l,
  SpatialWindowClippingIn_r };

ValueMapping spatialwindowclippingoutmap[] = {
  SpatialWindowClippingOut_l,
  SpatialWindowClippingOut_r };

ValueMapping spatialscalemap[] = {
  SpatialScale_p,
  SpatialScale_ps,
  SpatialScale_l,
  SpatialScale_r };

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
  "( <text>(point||points||line||sline||rect x "
  "point||points||line||sline||rect) -> real</text--->"
  "<text>distance( _, _ )</text--->"
  "<text>compute distance between two spatial objects.</text--->"
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

const string SpatialSpecTranslate  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>(point||points||line||region x real x real) -> "
  "point||points||line||region</text--->"
  "<text> _ translate[ dx, dy ]</text--->"
  "<text> move the object parallely for some distance.</text--->"
  "<text> query region1 translate[3.5, 15.1]</text--->"
  ") )";

const string SpatialSpecRotate  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>(point||points||line||region x real x real x real) -> "
  "point||points||line||region</text--->"
  "<text> _ translate[ x, y, theta ]</text--->"
  "<text> rotates the spatial object by 'theta' degrees around (x,y) </text--->"
  "<text> query region1 rotate[3.5, 15.1, 10.0]</text--->"
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

const string SpatialSpecScale  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>for T in {point, points, line, region}: "
  "T x real -> T</text--->"
  "<text> _ scale [ _ ] </text--->"
  "<text> scales an object by the given factor.</text--->"
  "<text> query region1 scale[1000.0]</text--->"
  ") )";

const string SpatialSpecComponents  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
  "( <text>points -> stream(point), region -> stream(region), "
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
    "( <text>line  x real [x points] -> line </text--->"
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
  "intersects",
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
  "intersection",
  SpatialIntersectionSpec,
  25,
  spatialintersectionVM,
  SpatialSetOpSelect,
  SpatialIntersectionTypeMap );

Operator spatialminus (
  "minus",
  SpatialMinusSpec,
  25,
  spatialminusVM,
  SpatialSetOpSelect,
  SpatialMinusTypeMap );

Operator spatialunion (
  "union",
  SpatialUnionSpec,
  25,
  spatialunionVM,
  SpatialSetOpSelect,
  SpatialUnionTypeMap );

Operator spatialcrossings (
  "crossings",
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
  29,
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
  5,
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

Operator spatialtranslate (
  "translate",
  SpatialSpecTranslate,
  4,
  spatialtranslatemap,
  SpatialSelectTranslate,
  SpatialTranslateMap );

Operator spatialrotate (
  "rotate",
  SpatialSpecRotate,
  4,
  spatialrotatemap,
  SpatialSelectTranslate,
  SpatialRotateMap );


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

Operator spatialscale (
  "scale",
  SpatialSpecScale,
  4,
  spatialscalemap,
  SpatialSelectTranslate,
  SpatialScaleMap );

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
  SpatialSegments,
  Operator::SimpleSelect,
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
9.23 Operator bufferLine

9.23.1 Type Mapping

Signature is :

   line x real -> region

*/

ListExpr bufferLineTM(ListExpr args){
  string err = "line x real [x bool] expected";
  int len = nl->ListLength(args);
  if((len!=2) && (len!=3 )){
    return listutils::typeError(err + " (wrong number of arguments)");
  }
  if(!Line::checkType(nl->First(args))){
    return listutils::typeError(err);
  }
  if(!CcReal::checkType(nl->Second(args))){
    return listutils::typeError(err);
  }
  if(len==2){
    return nl->ThreeElemList( nl->SymbolAtom( Symbol::APPEND()),
                          nl->OneElemList(nl->BoolAtom(false)),
                          nl->SymbolAtom(Region::BasicType()));

  }
  if(!CcBool::checkType(nl->Third(args))){
    return listutils::typeError(err);
  }
  return nl->SymbolAtom(Region::BasicType());
}


/*
9.23.2 Value Mapping


Auxiliary Function seg2reg

This funtion converts a halfsegment into a region by adding a buffer
zone of a given size.

*/

Region* seg2reg(HalfSegment& hs, double& buffer, const bool round = false){

  Point p1 = hs.GetLeftPoint();
  Point p2 = hs.GetRightPoint();

  double len = hs.Length();

  double dx = p2.GetX() - p1.GetX();
  double dy = p2.GetY() - p1.GetY();

  dx = (dx*buffer)/len;
  dy = (dy*buffer)/len;

  if(!round){ // use rectangular extension
     p1.Translate(-dx/2, -dy/2);
     p2.Translate(dx/2, dy/2);
  }



  Point r1(true, p1.GetX() + dy/2, p1.GetY() - dx/2);
  Point r2(true, p1.GetX() - dy/2, p1.GetY() + dx/2);

  Point r3(true, p2.GetX() + dy/2, p2.GetY() - dx/2);
  Point r4(true, p2.GetX() - dy/2, p2.GetY() + dx/2);

  AttrType attr(0);
  int edgeno = 0;

  Region* res = new Region(8);

  res->StartBulkLoad();


  HalfSegment hs1(true,r1,r2);
  hs1.attr.edgeno=edgeno;
  hs1.attr.insideAbove = dy>0;
  (*res) += hs1;
  hs1.SetLeftDomPoint(false);
  (*res) += hs1;
  edgeno++;


  HalfSegment hs2(true,r1,r3);
  hs2.attr.edgeno=edgeno;
  hs2.attr.insideAbove = true;
  (*res) += hs2;
  hs2.SetLeftDomPoint(false);
  (*res) += hs2;
  edgeno++;


  HalfSegment hs3(true,r3,r4);
  hs3.attr.edgeno=edgeno;
  hs3.attr.insideAbove = dy<=0;
  (*res) += hs3;
  hs3.SetLeftDomPoint(false);
  (*res) += hs3;
  edgeno++;


  HalfSegment hs4(true,r2,r4);
  hs4.attr.edgeno=edgeno;
  hs4.attr.insideAbove = false;
  (*res) += hs4;
  hs4.SetLeftDomPoint(false);
  (*res) += hs4;
  edgeno++;
  res->EndBulkLoad();

  if(round){
     Region* c1 = new Region(100);
     Region* c2 = new Region(100);
     double r = buffer/2;
     int n = buffer/2;
     if(n<8){
       n=8;
     }
     if(n>100){
       n = 100;
     }
     generateCircle(&p1,r,n,c1);
     generateCircle(&p2,r,n,c2);
     Region* tmp1 = new Region(200);
     c1->Union(*c2,*tmp1);
     delete c1;
     c2->Clear();
     res->Union(*tmp1,*c2);
     delete res;
     res = c2;
  }
  return res;
}




int bufferLineVM(Word* args, Word& result, int message,
                               Word& local, Supplier s){

  Line* line = (Line*) args[0].addr;
  CcReal* buffer = (CcReal*) args[1].addr;
  CcBool* round1 = (CcBool*) args[2].addr;

  bool round = round1->IsDefined() && round1->GetValue();

  result = qp->ResultStorage(s);
  Region* res = (Region*) result.addr;

  if(!line->IsDefined() || !buffer->IsDefined() || buffer->GetValue() <= 0){
     res->SetDefined(false);
     return 0;
  }

  double b = buffer->GetValue();

  res->Clear();
  Region* currentReg = 0;
  Region* wholeReg = 0;

  // cout << "Process " << line->Size() << "HalfSegments " << endl;

  for(int i=0;i<line->Size();i++){
     HalfSegment hs;
     line->Get(i,hs);
     if(hs.IsLeftDomPoint()){

         // cout << "Process HalfSegment no " << i << endl;

         currentReg = seg2reg(hs,b, round);


         if(wholeReg==0){
         //   cout << "This was the fisrt region" << endl;
            wholeReg = currentReg;
            currentReg = 0;
         } else {
          //  cout << "Not the first region, try to union" << endl;
            Region* tmp = new Region(0);
            wholeReg->Union(*currentReg,*tmp);
            wholeReg->DeleteIfAllowed();
            currentReg->DeleteIfAllowed();
            wholeReg = tmp;
            tmp=0;
          //  cout << "union finished" << endl;
         }
      }
  }
  res->CopyFrom(wholeReg);
  wholeReg->DeleteIfAllowed();
  return 0;
}


const string bufferLineSpec =
"( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
"( <text> line x real [ x round] -> region</text--->"
"<text> bufferline(_,_ [, _]) </text--->"
"<text>Converts a line into a region by adding a buffer.</text--->"
"<text>query bufferLine(BGrenzenLinie,200)</text---> ) )";


Operator bufferLine (
  "bufferLine",
  bufferLineSpec,
  bufferLineVM,
  Operator::SimpleSelect,
  bufferLineTM
);



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
       RegionCreator::findCritical(hss,isCritical);
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
         double dist = RegionCreator::getLeftDist(hs,x,y,true);
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
         double dist1 = RegionCreator::getLeftDist(hs,x,y, true);

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
      Point p;
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
      Point smallestDomPoint;
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
  RegionCreator::createRegion((DbArray<HalfSegment>*) line->GetFLOB(0),res);
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
  if(!nl->HasLength(args,2)){
    return listutils::typeError(err +  "  (wrong number of args)");
  }
  if(!Stream<Attribute>::checkType(nl->First(args))){
    return listutils::typeError(err +
                             "  ( first arg is not an attribute stream)");
  }
  if(!CcBool::checkType(nl->Second(args))){
    return listutils::typeError(err + " (second arg is not a bool)");
  }
  ListExpr attr = nl->Second(nl->First(args));
  if(!listutils::isKind(attr,Kind::SPATIAL2D())){
    return listutils::typeError(err + " (attribute not in kind SPATIAL2D)");
  }
  return listutils::basicSymbol<Rectangle<2> >();
}

template<int dim>
int collect_boxVM(Word* args, Word& result, int message, Word& local,
                       Supplier s )
{
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

OperatorSpec collect_boxSpec (
    "stream<SPATIAL> x bool -> rectangle",
    " _ collect_box[_]",
    "Computes the bounding box from a stream of spatial attributes"
    "If the second parameter is ste to be true, undefined elements "
    " within the stream are ignored. Otherwise an undefined element"
    " will lead to an undefined result.",
    "query strassen feed projecttransformstream[GeoData] collect_box[TRUE] "
  );

Operator collect_box(
   "collect_box",
   collect_boxSpec.getStr(),
   collect_boxVM<2>,
   Operator::SimpleSelect,
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

  return listutils::basicSymbol<Line>();
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
       distComp = &RegionCreator::getUpDist;
       dir = 0;
    } else {
       distComp = &RegionCreator::getDownDist;
       dir = 1;
    }
  }  else {
    double y2 = hs.GetSecPoint().GetY();
    double y1 = hs.GetDomPoint().GetY();
    if(hs.attr.insideAbove  == (y2>y1)){
      distComp = &RegionCreator::getLeftDist;
      dir = 2;
    }  else{
      distComp = &RegionCreator::getRightDist;
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

  Attribute* arg = (Attribute*) args[0].addr;
  result = qp->ResultStorage(s);
  CcBool* res = (CcBool*) result.addr;
  if(!arg->IsDefined()){
    res->SetDefined(false);
  } else {
    DbArray<HalfSegment>* hss = (DbArray<HalfSegment>*) arg->GetFLOB(0);
    robust::RealmChecker rc(hss);
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
  ListExpr tt = robust::RealmChecker::getTupleType();
  return nl->TwoElemList( listutils::basicSymbol<Stream<Tuple> >(),
                          tt);
}

/*
1.43.2 Value Mapping

*/

int badRealmVM(Word* args, Word& result, int message, Word& local,
               Supplier s ){

  robust::RealmChecker* li = (robust::RealmChecker*) local.addr;
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
                        local.addr = new robust::RealmChecker(hss);
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
             res->clear();
             res->SetDefined(false);
             if(lastP){
               lastP->DeleteIfAllowed();
             }
             p->DeleteIfAllowed();
             stream.close();
             return 0;
          } else {
            p->DeleteIfAllowed();
          }
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

    point.AssociateKind(Kind::DATA());
    points.AssociateKind(Kind::DATA());
    line.AssociateKind(Kind::DATA());
    region.AssociateKind(Kind::DATA());
    sline.AssociateKind(Kind::DATA());
    dline.AssociateKind(Kind::DATA());
    drm.AssociateKind(Kind::DATA());
    oim.AssociateKind(Kind::DATA());



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

    AddOperator(&bufferLine);
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
    AddOperator(&badRealmOp);
    AddOperator(&crossings_rob);

    AddOperator(&splitline);
    AddOperator(&computeDRM);
    AddOperator(&computeOIM);

    AddOperator(&collectDline);


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
