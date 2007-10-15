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

//paragraph [1] Title: [{\Large \bf \begin{center}] [\end{center}}]
//paragraph [10] Footnote: [{\footnote{] [}}]
//[TOC] [\tableofcontents]

[1] Implementation of Module

September - November 2007 Sascha Vaut

[TOC]

1 Overview

Up to now, this file contains the implementation of the type constructors 
~uncertain, cpoint, cupoint~ and ~cmpoint~. The memory data structures used for
these type constructors are implemented in the HierarchicalGeoAlgebra.h file.

2 Defines, includes, and constants

*/

#define TRACE_ON
#include "LogMsg.h"
#undef TRACE_ON

#include <cmath>
#include <limits>
#include <iostream>
#include <sstream>
#include <string>
#include <stack>
#include <vector>
#include "NestedList.h"
#include "QueryProcessor.h"
#include "Algebra.h"
#include "StandardTypes.h"
#include "SpatialAlgebra.h"
#include "PolySolver.h"
#include "RelationAlgebra.h"
//#include "StreamAlgebra.h"
#include "TemporalAlgebra.h"
#include "MovingRegionAlgebra.h"
#include "TypeMapUtils.h"
#include <math.h>

extern NestedList* nl;
extern QueryProcessor* qp;

#include "DateTime.h"
#include "HierarchicalGeoAlgebra.h"



/*
2.1 Definition of some constants


3 Implementation of C++ Classes

3.1 Template Class ~Uncertain~

*/


/*
3.2 Class ~CUPoint~

*/
//virtual void CUPoint::TemporalFunction( const Instant& t,
//                                 Point& result,
//                                 bool ignoreLimits = false ) const 
//{
//  
//}

void CUPoint::UTrajectory( Region& result ) const
{
  result.SetDefined( true );
  result.Clear();    // clear the result region
  
  if (AlmostEqual( p0, p1 ) )
  {
    // p0 almost equals p1, so the trajectory of this cupoint can be
    // represented as a circular region whose center is defined by p0 and
    // whose radius is defined by the uncertainty-value epsilon.
    Circle( p0, epsilon, 16, result );
  }
  else
  {
    Coord x1, y1, x2, y2, x3, y3, x4, y4;
    
    if (AlmostEqual(p0.GetX(), p1.GetX()) )
    {
      // The uncertain unit point moves along the y-axis.
      x1 = p0.GetX() + epsilon;
      y1 = p0.GetY();
      x2 = p0.GetX() - epsilon;
      y2 = p0.GetY();
      x3 = p1.GetX() - epsilon;
      y3 = p1.GetY();
      x4 = p1.GetX() + epsilon;
      y4 = p1.GetY();
    }
    else if (AlmostEqual(p0.GetY(), p1.GetY()) )
    {
      // The uncertain unit point moves along the x-axis.
      x1 = p0.GetX();
      y1 = p0.GetY() - epsilon;
      x2 = p0.GetX();
      y2 = p0.GetY() + epsilon;
      x3 = p1.GetX();
      y3 = p1.GetY() + epsilon;
      x4 = p1.GetX();
      y4 = p1.GetY() - epsilon;
    }
    else
    {
      // Create 4 halfsegments as the edges of a rectangular box
      // that defines the uncertainty-area of this cupoint's trajectory.  
      
      // To determine the edge-points of these halfsegments, the trigonometric
      // functions sin(alpha) and cos(alpha) are used:
      double lengthX = fabs(p1.GetX() - p0.GetX());
      double lengthY = fabs(p1.GetY() - p0.GetY());
      double length = p0.Distance(p1);
      double sinalpha = lengthY / length;
      double cosalpha = lengthX / length;
      Point pleft;
      Point pright;
      
      if ( p0.GetX() < p1.GetX() )
      {
        pleft.Set( p0.GetX(), p0.GetY() );
        pright.Set( p1.GetX(), p1.GetY() ); 
      }
      else
      {
        pleft.Set( p1.GetX(), p1.GetY() );
        pright.Set( p0.GetX(), p0.GetY() );
      }
      
      if (pleft.GetY() < pright.GetY() )
      {
        x1 = pleft.GetX() + epsilon * sinalpha;
        y1 = pleft.GetY() - epsilon * cosalpha;
        x2 = pleft.GetX() - epsilon * sinalpha;
        y2 = pleft.GetY() + epsilon * cosalpha;
        y3 = y2 + lengthY;
        y4 = y1 + lengthY;
      }
      else
      {
        x1 = pleft.GetX() + epsilon * sinalpha;
        y1 = pleft.GetY() + epsilon * cosalpha;
        x2 = pleft.GetX() - epsilon * sinalpha;
        y2 = pleft.GetY() - epsilon * cosalpha;
        y3 = y2 - lengthY;
        y4 = y1 - lengthY;
      }
      x3 = x2 + lengthX;
      x4 = x1 + lengthX;
      
    }
    
    // Create points of the coordinates:
    Point ep1(true, x1, y1);
    Point ep2(true, x2, y2);
    Point ep3(true, x3, y3);
    Point ep4(true, x4, y4);
    
    if( AlmostEqual(ep1, ep2) ||
        AlmostEqual(ep2, ep3) ||
        AlmostEqual(ep3, ep4) ||
        AlmostEqual(ep4, ep1) )
    { // one interval is (almost) empty, so will be the region
      result.SetDefined( true );
      return;
    }
    
    HalfSegment hs;
    int partnerno = 0;
    
    result.StartBulkLoad();
    // Create halfsegments of the points and insert them to region 'result':
    hs.Set(true, ep1, ep2);
    hs.attr.faceno = 0;         // only one face
    hs.attr.cycleno = 0;        // only one cycle
    hs.attr.edgeno = partnerno;
    hs.attr.partnerno = partnerno++;
    hs.attr.insideAbove = (hs.GetLeftPoint() == ep2);
    result += hs;
    hs.SetLeftDomPoint( !hs.IsLeftDomPoint() );
    result += hs;
    
    hs.Set(true, ep2, ep3);
    hs.attr.faceno = 0;         // only one face
    hs.attr.cycleno = 0;        // only one cycle
    hs.attr.edgeno = partnerno;
    hs.attr.partnerno = partnerno++;
    hs.attr.insideAbove = (hs.GetLeftPoint() == ep3);
    result += hs;
    hs.SetLeftDomPoint( !hs.IsLeftDomPoint() );
    result += hs;
    
    hs.Set(true, ep3, ep4);
    hs.attr.faceno = 0;         // only one face
    hs.attr.cycleno = 0;        // only one cycle
    hs.attr.edgeno = partnerno;
    hs.attr.partnerno = partnerno++;
    hs.attr.insideAbove = (hs.GetLeftPoint() == ep4);
    result += hs;
    hs.SetLeftDomPoint( !hs.IsLeftDomPoint() );
    result += hs;
    
    hs.Set(true, ep4, ep1);
    hs.attr.faceno = 0;         // only one face
    hs.attr.cycleno = 0;        // only one cycle
    hs.attr.edgeno = partnerno;
    hs.attr.partnerno = partnerno++;
    hs.attr.insideAbove = (hs.GetLeftPoint() == ep1);
    result += hs;
    hs.SetLeftDomPoint( !hs.IsLeftDomPoint() );
    result += hs;
    
    result.EndBulkLoad();
    result.SetDefined( true );
  }
  return;
}

void CUPoint::TemporalFunction( const Instant& t,
                               Point& result,
                               bool ignoreLimits ) const
{
  if( !IsDefined() ||
      !t.IsDefined() ||
      (!timeInterval.Contains( t ) && !ignoreLimits) )
    {
      result.SetDefined(false);
    }
  else if( t == timeInterval.start )
    {
      result = p0;
      result.SetDefined(true);
    }
  else if( t == timeInterval.end )
    {
      result = p1;
      result.SetDefined(true);
    }
  else
    {
      Instant t0 = timeInterval.start;
      Instant t1 = timeInterval.end;

      double x = (p1.GetX() - p0.GetX()) * ((t - t0) / (t1 - t0)) + p0.GetX();
      double y = (p1.GetY() - p0.GetY()) * ((t - t0) / (t1 - t0)) + p0.GetY();

      result.Set( x, y );
      result.SetDefined(true);
    }
}


void CUPoint::AtInterval( const Interval<Instant>& i,
                           TemporalUnit<Point>& result ) const
{
  assert( IsDefined() );
  assert( i.IsValid() );

  
  TemporalUnit<Point>::AtInterval( i, result );
  UPoint *pResult = (UPoint*)&result;

  if( timeInterval.start == result.timeInterval.start )
    {
      pResult->p0 = p0;
      pResult->timeInterval.start = timeInterval.start;
      pResult->timeInterval.lc = (pResult->timeInterval.lc && timeInterval.lc);
    }
  else
    TemporalFunction( result.timeInterval.start, pResult->p0 );

  if( timeInterval.end == result.timeInterval.end )
    {
      pResult->p1 = p1;
      pResult->timeInterval.end = timeInterval.end;
      pResult->timeInterval.rc = (pResult->timeInterval.rc && timeInterval.rc);
      
    }
  else
    TemporalFunction( result.timeInterval.end, pResult->p1 );

  pResult->SetDefined ( true );
}

bool CUPoint::D_Passes( const Point& val ) const
{
  assert( val.IsDefined() );
  assert( IsDefined() );
  
  if (epsilon > 0.0)
    return false;
  else
  {
    // If the epsilon-value equals 0, the cupoint-object is certain and
    // can be casted to a UPoint-object.
    return ((UPoint*)this)->Passes( val );
  }
}

bool CUPoint::D_Passes( const Region& r ) const
{
  assert( r.IsDefined() );
  assert( IsDefined() );
  
  bool result = false;
  
  //1. If the cupoint's bbox and the region's bbox do not overlap, the result
  // is FALSE
  if( !r.BoundingBox().Intersects( this->BBox2D() ) )
    return false;
  
  bool containsP0 = false;
  bool containsP1 = false;
  bool distP0GreaterEpsilon = false;
  bool distP1GreaterEpsilon = false;
  bool cupIntersectsRgn = false;
  int i;
  HalfSegment segCup;
  const HalfSegment *segRgn;        // HalfSegments for iterating the region
  Point defPP;    // defines the point where the cupoint completely crosses
                        // the regions border (The point on segCup which lies
                        // inside the region and its distance to the regions
                        // border equals the epsilon-value.)
  bool defPPtooClose = true;
  defPP.SetDefined(false);
  
  
  //2. Determine, if one of the endpoints of the cupoint lies inside the region
  if (r.Contains( p0 ) )
  {
    containsP0 = true;
    distP0GreaterEpsilon = true;
  }
  if ( !AlmostEqual(p0, p1) )
    if ( r.Contains( p1 ) )
    {
      containsP1 = true;
      distP1GreaterEpsilon = true;
    }
  
  //3. If one of the endpoints lies inside the region, determine if the
  //distance of this endpoint to the regions border is greater than epsilon.
  
  if( p0 < p1 )
    segCup.Set(true, p0, p1);
  else
    segCup.Set(false, p1, p0);
    // p0 is the dominating point of the halfsegment
      
  //r.StartBulkLoad();
  for(i = 0; i < r.Size(); i++)
  {
    r.Get( i, segRgn);
    
    if (segCup.Intersects(*segRgn) )
      cupIntersectsRgn = true;
    
    if (containsP0 && (segRgn->Distance(p0) <= epsilon) )
      // P0 is too close to this region's halfsegment
      distP0GreaterEpsilon = false;
    if (containsP1 && (segRgn->Distance(p1) <= epsilon) )
      // P0 is too close to this region's halfsegment
      distP1GreaterEpsilon = false;

    if( containsP0 && !distP0GreaterEpsilon || 
          containsP1 && !distP1GreaterEpsilon || 
          !containsP0 && !containsP1 && cupIntersectsRgn )
    {
      // If one of the endpoints lies inside the region and the distance
      // to the region's border is less than epsilon, or if the cupoint
      // intersects the region
      if( defPP.IsDefined() )
        defPPtooClose = (segRgn->Distance(defPP) <= epsilon);
      if( defPPtooClose || !defPP.IsDefined() )
        if( FindDefPassingPoint(segCup, *segRgn, epsilon, &defPP) )
          defPPtooClose = false;
        else {
          defPPtooClose = true;
          defPP.SetDefined(false);
        }
    }
  }
  //r.EndBulkLoad();
  if (distP0GreaterEpsilon || distP1GreaterEpsilon || !defPPtooClose)
    // One of the endpoints lies inside the region, and its distance to the
    // region's border is greater than epsilon, so the predicate 'definitely-
    // passes' is fullfilled.
    return true;
  
  
  return result;
}

/*
3.3 Class ~CMPoint~

*/
void CMPoint::Clear()
{
  Mapping<CUPoint, Point>::Clear(); // call super
  bbox.SetDefined(false);          // invalidate bbox
}

void CMPoint::Add( const CUPoint& unit )
{
//   cout << "CALLED: MPoint::Add" << endl;
  assert( unit.IsValid() );
  assert( unit.IsDefined() );
  units.Append( unit );
  if(units.Size() == 1)
  {
//     cout << "        MPoint::Add FIRST ADD" << endl;
//     cout << "\t Old bbox = "; bbox.Print(cout);
    bbox = unit.BoundingBox();
//     cout << "\n\t New bbox = "; bbox.Print(cout);
  } else {
//     cout << "\t Old bbox = "; bbox.Print(cout);
    bbox = bbox.Union(unit.BoundingBox());
//     cout << "\n\t New bbox = "; bbox.Print(cout);
  }
  RestoreEpsilon();
  RestoreBoundingBox(false);
}

void CMPoint::MergeAdd(const CUPoint& unit){
  assert( unit.IsValid() );
  assert( unit.IsDefined() );

  int size = GetNoComponents();
  if(size==0){ // the first unit
     Add(unit);
     bbox = unit.BoundingBox();
     return;
  }
  const CUPoint* last;
  Get(size-1,last);
  if(last->timeInterval.end!=unit.timeInterval.start ||
     !( (last->timeInterval.rc )  ^ (unit.timeInterval.lc))){
     // intervals are not connected
     Add(unit);
     bbox = bbox.Union(unit.BoundingBox());
     return;
  }
  if(!AlmostEqual(last->p1, unit.p0)){
    // jump in spatial dimension
    Add(unit);
    bbox = bbox.Union(unit.BoundingBox());
    return;
  }
  // define the epsilon-value of the two merged uncertain units:
  double e;
  if (unit.epsilon > last->epsilon)
    e = unit.epsilon;
  else
    e = last->epsilon;
  
  Interval<Instant> complete(last->timeInterval.start,
                             unit.timeInterval.end,
                             last->timeInterval.lc,
                             unit.timeInterval.rc);
  CUPoint cupoint(e, complete,last->p0, unit.p1);
  delete &e;
  Point p;
  cupoint.TemporalFunction(last->timeInterval.end, p, true);
  if(!AlmostEqual(p,last->p0)){
     Add(unit);
     bbox = bbox.Union(unit.BoundingBox());
     return;
  }
  assert( cupoint.IsValid() );
  assert( cupoint.IsDefined() );
  units.Put(size-1,cupoint); // overwrite the last unit by a connected one
}


void CMPoint::Restrict( const vector< pair<int, int> >& intervals )
{
  units.Restrict( intervals ); // call super
  bbox.SetDefined(false);      // invalidate bbox
  RestoreBoundingBox();        // recalculate it
}

ostream& CMPoint::Print( ostream &os ) const
{
  if( !IsDefined() )
  {
    return os << "(CMPoint: undefined)";
  }
  os << "(CMPoint: defined, MBR = ";
  bbox.Print(os);
  os << ", contains " << GetNoComponents() << " units: ";
  for(int i=0; i<GetNoComponents(); i++)
  {
    const CUPoint *unit;
    Get( i , unit );
    os << "\n\t";
    unit->Print(os);
  }
  os << "\n)" << endl;
  return os;
}

void CMPoint::EndBulkLoad(const bool sort)
{
  Mapping<CUPoint, Point>::EndBulkLoad( sort ); // call super
  RestoreBoundingBox();                        // recalculate, if necessary
}

/*
RestoreEpsilon() checks, wether the CMPoints ~epsilon~ value equals the maximum
epsilon value of all contained uncertain units, and resets it if needed.

*/
void CMPoint::RestoreEpsilon()
{
  if(!IsDefined() || GetNoComponents() == 0)
  {  // If the cmpoint isn't defined or has no components, the epsilon-value
      // is set to undefined.
    UncertainSetDefined(false);
  }
  else
  { // Determine the maximum value of the epsilon-values of all units.
    const CUPoint *unit;
    int size = GetNoComponents();
    bool isfirst = true;
    for( int i = 0; i < size; i++ )
    {
      Get( i, unit );
      if (isfirst)
      {
        epsilon = unit->GetEpsilon();
        UncertainSetDefined( true );
        isfirst = false;
      }
      else if( epsilon < unit->GetEpsilon() )
      {
        epsilon = unit->GetEpsilon();
      }
      // else: there's nothing to do.
    }
  } 
}


/*
RestoreBoundingBox() checks, whether the MPoint's MBR ~bbox~ is ~undefined~
and thus may needs to be recalculated and if, does so.

*/

void CMPoint::RestoreBoundingBox(const bool force)
{
  if(!IsDefined() || GetNoComponents() == 0)
  { // invalidate bbox
    bbox.SetDefined(false);
  }
  else if(force || !bbox.IsDefined())
  { // construct bbox
    const CUPoint *unit;
    int size = GetNoComponents();
    bool isfirst = true;
    for( int i = 0; i < size; i++ )
    {
      Get( i, unit );
      if (isfirst)
      {
        bbox = unit->BoundingBox();
        isfirst = false;
      }
      else
      {
        bbox = bbox.Union(unit->BoundingBox());
      }
    }
  } // else: bbox unchanged and still correct
}

// Class functions
Rectangle<3u> CMPoint::BoundingBox() const
{
  return bbox;
}


//TODO "Aufruf von aggregateS über QueryProcessor::ExecuteQuery(...) einbauen."
// Siehe StandardAlgebra.cpp - RelcountFun2()
/*void CMPoint::Trajectory( Region& region ) const
{
  Region unitregion;

  region.Clear();
  region.StartBulkLoad();

  // create a subquery to determine the trajectory-region of the
  const CUPoint *unit;
  int edgeno = 0;
  int size = GetNoComponents();
  if (size > 0 )
    region.Resize(size);
  
  for( int i = 0; i < size; i++ )
  {
    Get( i, unit );
    
    if( !AlmostEqual( unit->p0, unit->p1 ) )
    {
      region
    }
  
}*/

bool CMPoint::Present( const Instant& t ) const
{
  assert( IsDefined() );
  assert( t.IsDefined() );
  assert( IsOrdered() );

  if(bbox.IsDefined())
  { // do MBR-check
    double instd = t.ToDouble();
    double mint = bbox.MinD(2);
    double maxt = bbox.MaxD(2);
    if( (instd < mint && !AlmostEqual(instd,mint)) ||
        (instd > maxt && !AlmostEqual(instd,mint))
      )
    {
      return false;
    }
  }
  int pos = Position(t);
  if( pos == -1 )         //not contained in any unit
    return false;
  return true;
}

bool CMPoint::Present( const Periods& t ) const
{
  assert( IsDefined() );
  assert( IsOrdered() );
  assert( t.IsDefined() );
  assert( t.IsOrdered() );

  Periods defTime( 0 );
  DefTime( defTime );
  if(bbox.IsDefined())
  { // do MBR-check
    double MeMin = bbox.MinD(2);
    double MeMax = bbox.MaxD(2);
    Instant tmin; t.Minimum(tmin);
    Instant tmax; t.Maximum(tmax);
    double pmin = tmin.ToDouble();
    double pmax = tmax.ToDouble();
    if( (pmin < MeMin && !AlmostEqual(pmin,MeMin)) ||
         (pmax > MeMax && !AlmostEqual(pmax,MeMax))
      )
    {
      return false;
    }
  }
  return t.Intersects( defTime );
}

void CMPoint::AtInstant( const Instant& t, Intime<Region>& result ) const
{
  assert( IsOrdered() );
  assert( t.IsDefined() );
  if( IsDefined() && t.IsDefined() )
  {
    if( !bbox.IsDefined() )
    { // result is undefined
      result.SetDefined(false);
    } else if( IsEmpty() )
    { // result is undefined
      result.SetDefined(false);
    } else
    { // compute result
      double instd = t.ToDouble();
      double mind = bbox.MinD(2);
      double maxd = bbox.MaxD(2);
      if( (mind > instd && !AlmostEqual(mind,instd)) ||
           (maxd < instd && !AlmostEqual(maxd,instd))
        )
      {
        result.SetDefined(false);
      } else
      {
        int pos = Position( t );
        if( pos == -1 )  // not contained in any unit
          result.SetDefined( false );
        else
        {
          const CUPoint *posUnit;
          units.Get( pos, posUnit );
          result.SetDefined( true );
          Point respoint;
          posUnit->TemporalFunction( t, respoint );
          Circle(respoint, posUnit->epsilon, 16, result.value);
          result.instant.CopyFrom( &t );
        }
      }
    }
  } else
  {
    result.SetDefined(false);
  }
}

void CMPoint::AtPeriods( const Periods& p, CMPoint& result ) const
{
  assert( IsOrdered() );
  assert( p.IsOrdered() );

  result.Clear();
  result.SetDefined(true);
  if( IsDefined() && p.IsDefined() )
  {
    if( !bbox.IsDefined())
    { // result is undefined
      result.SetDefined(false);
    } else if( IsEmpty() || p.IsEmpty())
    { // result is defined but empty
      result.SetDefined(true);
    } else
    { // compute result
      Instant perMinInst; p.Minimum(perMinInst);
      Instant perMaxInst; p.Maximum(perMaxInst);
      double permind = perMinInst.ToDouble();
      double permaxd = perMaxInst.ToDouble();
      double mind = bbox.MinD(2);
      double maxd = bbox.MaxD(2);
      if( (mind > permaxd && !AlmostEqual(mind,permaxd)) ||
          (maxd < permind && !AlmostEqual(maxd,permind)))
      {
        result.SetDefined(true);
      } else
      {
        result.StartBulkLoad();
        const CUPoint *unit;
        const Interval<Instant> *interval;
        int i = 0, j = 0;
        Get( i, unit );
        p.Get( j, interval );

        while( 1 )
        {
          if( unit->timeInterval.Before( *interval ) )
          {
            if( ++i == GetNoComponents() )
              break;
            Get( i, unit );
          }
          else if( interval->Before( unit->timeInterval ) )
          {
            if( ++j == p.GetNoComponents() )
              break;
            p.Get( j, interval );
          }
          else
          { // we have overlapping intervals, now
            CUPoint r;
            unit->AtInterval( *interval, r );
            r.epsilon = unit->epsilon;
            r.UncertainSetDefined(true);
            result.Add( r );
//          cout << "\n\tunit = "; unit->Print(cout); cout << endl;
//          cout << "\tinterval =       "; interval->Print(cout); cout << endl;
//          cout << "\tr    = "; r.Print(cout); cout << endl;

            if( interval->end == unit->timeInterval.end )
            { // same ending instant
              if( interval->rc == unit->timeInterval.rc )
              { // same ending instant and rightclosedness: Advance both
                if( ++i == GetNoComponents() )
                  break;
                Get( i, unit );
                if( ++j == p.GetNoComponents() )
                  break;
                p.Get( j, interval );
              }
              else if( interval->rc == true )
              { // Advanve in mapping
                if( ++i == GetNoComponents() )
                  break;
                Get( i, unit );
              }
              else
              { // Advance in periods
                assert( unit->timeInterval.rc == true );
                if( ++j == p.GetNoComponents() )
                  break;
                p.Get( j, interval );
              }
            }
            else if( interval->end > unit->timeInterval.end )
            { // Advance in mpoint
              if( ++i == GetNoComponents() )
                break;
              Get( i, unit );
            }
            else
            { // Advance in periods
              assert( interval->end < unit->timeInterval.end );
              if( ++j == p.GetNoComponents() )
                break;
              p.Get( j, interval );
            }
          }
        }
        result.EndBulkLoad();
      }
    }
  } else
  {
    result.SetDefined(false);
  }
}

/*
4 Implementation of some auxiliary functions

4.1 Circle

This function computes a circular shaped region around a given Point p with
the given radius, built from n HalfSegments.

*/

void Circle( const Point p, const double radius, const int n, Region& result)
{
  double x = p.GetX();
  double y = p.GetY();
  double valueX, valueY;
  double angle;
  int partnerno = 0;
  HalfSegment hs;
  
  result.Clear();            // clear the result region
  if (!p.IsDefined() || radius<=0.0 || n<3 )
  { // Nothing to do
    result.SetDefined( false );
  }
  else
  {
    result.StartBulkLoad();
    if (n<101)
    {
      //  Calculate a polygon with (n) vertices and (n) edges.
      //  To get the vertices, divide 360 degree in n parts using
      //  a standardised circle around p with circumference U = 2 * PI * r.

      for( int i = 0; i < n; i++ )
      {
        // The first point/vertex of the segment
        angle = i * 2 * PI/n; // angle to starting vertex
        valueX = x + radius * cos(angle);
        valueY = y + radius * sin(angle);
        Point v1(true, valueX ,valueY);

        // The second point/vertex of the segment
        if ((i+1) >= n)            // angle to end vertex
          angle = 0 * 2 * PI/n;    // for inner vertex
        else
          angle = (i+1) * 2 * PI/n;// for ending = starting vertex
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
        result += hs;
        hs.SetLeftDomPoint( !hs.IsLeftDomPoint() );
        result += hs;
      }
    }
    result.EndBulkLoad();
    result.SetDefined( true );
  }
  return;
}

/*
4.2 FindDefPassingPoint

If the distance between the two given HalfSegments is less than the given 
epsilon value, this function finds that point in the first HalfSegment, which 
has exactly a distance of epsilon to the second HalfSegment. The second
HalfSegment must be part of a region-object (the attr.insideAbove-value must
be evaluable)!

*/
bool FindDefPassingPoint( const HalfSegment& chs, const HalfSegment& rgnhs,
                    const double epsilon, Point* defPP)
{
  Coord xl, yl, xr, yr;
  Point auxlp, auxrp;
  HalfSegment aux;
  
  // create a parallel HalfSegment on the inner side of rgnhs, which has a
  // distance of epsilon to rgnhs.
    
  if (AlmostEqual(rgnhs.GetLeftPoint().GetX(), rgnhs.GetRightPoint().GetX()))
  {
    // The halfsegment is vertical.
    if (rgnhs.attr.insideAbove)
    {
      // The region lies on the left side of the halfsegment.
      xl = rgnhs.GetLeftPoint().GetX() - epsilon;
      yl = rgnhs.GetLeftPoint().GetY();
      xr = rgnhs.GetRightPoint().GetX() - epsilon;
      yr = rgnhs.GetRightPoint().GetY();
    }
    else {
      // The region lies on the right side of the halfsegment.
      xl = rgnhs.GetLeftPoint().GetX() + epsilon;
      yl = rgnhs.GetLeftPoint().GetY();
      xr = rgnhs.GetRightPoint().GetX() + epsilon;
      yr = rgnhs.GetRightPoint().GetY();
    }
  }
  else if (AlmostEqual(rgnhs.GetLeftPoint().GetY(), 
                      rgnhs.GetRightPoint().GetY()) )
  {
    // The halfsegment is horizontal.
    if (rgnhs.attr.insideAbove)
    {
      // The region lies above the halfsegment.
      xl = rgnhs.GetLeftPoint().GetX();
      yl = rgnhs.GetLeftPoint().GetY() + epsilon;
      xr = rgnhs.GetRightPoint().GetX();
      yr = rgnhs.GetRightPoint().GetY() + epsilon;
    }
    else {
      // The region lies below the halfsegment.
      xl = rgnhs.GetLeftPoint().GetX();
      yl = rgnhs.GetLeftPoint().GetY() - epsilon;
      xr = rgnhs.GetRightPoint().GetX();
      yr = rgnhs.GetRightPoint().GetY() - epsilon;
    }
  }
  else {
    // To determine the edge-points of the halfsegment, the trigonometric
    // functions sin(alpha) and cos(alpha) are used:
    xl = rgnhs.GetLeftPoint().GetX();
    yl = rgnhs.GetLeftPoint().GetY();
    xr = rgnhs.GetRightPoint().GetX();
    yr = rgnhs.GetRightPoint().GetY();


    double lengthX = fabs(xr - xl);
    double lengthY = fabs(yr - yl);
    double length = rgnhs.GetLeftPoint().Distance(rgnhs.GetRightPoint());
    double sinalpha = lengthY / length;
    double cosalpha = lengthX / length;
    
    if (rgnhs.GetLeftPoint().GetY() < rgnhs.GetRightPoint().GetY() )
    {
      if(rgnhs.attr.insideAbove)
      {
        xl = xl - epsilon * sinalpha;
        yl = yl + epsilon * cosalpha;
        xr = xr - epsilon * sinalpha;
        yr = yr + epsilon * cosalpha;
      }
      else {
        xl = xl + epsilon * sinalpha;
        yl = yl - epsilon * cosalpha;
        xr = xr + epsilon * sinalpha;
        yr = yr - epsilon * cosalpha;
      }
    }
    else {
      if(rgnhs.attr.insideAbove)
      {
        xl = xl + epsilon * sinalpha;
        yl = yl + epsilon * cosalpha;
        xr = xr + epsilon * sinalpha;
        yr = yr + epsilon * cosalpha;
      }
      else {
        xl = xl - epsilon * sinalpha;
        yl = yl - epsilon * cosalpha;
        xr = xr - epsilon * sinalpha;
        yr = yr - epsilon * cosalpha;
      }
    }
    auxlp.Set(xl, yl);
    auxrp.Set(xr, yr);
    aux.Set(rgnhs.IsLeftDomPoint(), auxlp, auxrp);
  }
  // Find the intersection-point between chs and aux if there is one.
  if( aux.Intersection( chs, *defPP ) )
    return true;
  return false;
}

/*
+++++ eventuell nicht noetig! +++

4.3 FictiveIntersection

Computes the fictive intersection-point between the two given HalfSegments
('fictive' means, the intersection point of the infinite extended HalfSegments)


bool FictiveIntersection(const HalfSegment& hsa, const HalfSegment& hsb, 
                                      Point& resp)
{
  double k, a, K, A;

  Coord xl = hsa.GetLeftPoint().GetX(),
        yl = hsa.GetLeftPoint().GetY(),
        xr = hsa.GetRightPoint().GetX(),
        yr = hsa.GetRightPoint().GetY(),
        Xl = hsb.GetLeftPoint().GetX(),
        Yl = hsb.GetLeftPoint().GetY(),
        Xr = hsb.GetRightPoint().GetX(),
        Yr = hsb.GetRightPoint().GetY();

  if( AlmostEqual( xl, xr ) &&
      AlmostEqual( Xl, Xr ) )
    // both segments are vertical
  {
    // +++++ Muss hier nicht auch Xl mit xr verglichen werden? +++
    if( AlmostEqual( yr, Yl ) )
    {
      resp.Set( xl, yr );
      return true;
    }
    if( AlmostEqual( yl, Yr ) )
    {
      resp.Set( xl, yl );
      return true;
    }
    return false;
  }

  if( !AlmostEqual( xl, xr ) )
    // hsa is not vertical
  {
    k = (yr - yl) / (xr - xl);    // k = the slope of hsa
    a = yl - k * xl;                 // a = the y-value of hsa when x is 0
  }

  if( !AlmostEqual( Xl, Xr ) )
    // hsb is not vertical
  {
    K = (Yr - Yl) / (Xr - Xl);  // K = the slope of hsb
    A = Yl - K * Xl;               // A = the y-value of hsb when x is 0
  }

  if( AlmostEqual( Xl, Xr ) )
    //only hsb is vertical
  {
    Coord y0 = k * Xl + a;

    // (Xl, y0) is the fictive intersection point. 
    resp.Set( Xl, y0 );
    return true;
  }

  if( AlmostEqual( xl, xr ) )
    // only hsa is vertical
  {
    Coord Y0 = K * xl + A;
    // (xl, Y0) is the fictive intersection point
    resp.Set( xl, Y0 );
    return true;
  }

  // both segments are non-vertical

  if( AlmostEqual( k, K ) )
    // both segments have the same inclination
  {
    if( AlmostEqual( hsa.rp, hsb.lp ) )
    {
      resp = hsa.rp;
      return true;
    }
    if( AlmostEqual( hsa.lp, hsb.rp ) )
    {
      resp = hsa.lp;
      return true;
    }
    return false;
  }

  Coord x0 = (A - a) / (k - K),
        y0 = x0 * k + a;
  // the segments fictive-intersect at (x0, y0)
  {
    resp.Set( x0, y0 );
    return true;
  }

  return false;
}*/

/*
5 Type Constructors

5.1 Type Constructor ~CPoint~

Type ~cpoint~ represents an (epsilon, (x, y))-pair.

List Representation

The list representation of a ~cpoint~ is

----    ( epsilon ( x y ) )
----

For example:

----    ( 20.5 ( 329.456 22.289 ) )
----

Function describing the signature of the Type Constructor

*/
ListExpr CPointProperty()
{
  return (nl->TwoElemList(
            nl->FiveElemList(
                  nl->StringAtom("Signature"),
                  nl->StringAtom("Example Type List"),
                  nl->StringAtom("List Rep"),
                  nl->StringAtom("Example List"),
                  nl->StringAtom("Remarks")),
            nl->FiveElemList(
                  nl->StringAtom("-> UNCERTAIN"),
                  nl->StringAtom("cpoint"),
                  nl->StringAtom("(<epsilon>(<x> <y>))"),
                  nl->StringAtom("( 20.5 ( 329.456 22.289) )"),
                  nl->StringAtom(" epsilon must be a pos. real-value." ))));
}

/*
Kind Checking Function

*/

bool CheckCPoint( ListExpr type, ListExpr& errorInfo )
{
  return (nl->IsEqual( type, "cpoint"));
}

/*
~Out~-function

*/
ListExpr OutCPoint( ListExpr typeInfo, Word value )
{
  CPoint* cpoint = (CPoint*)(value.addr);
  
  if ( !cpoint->IsDefined() )
    return (nl->SymbolAtom("undef"));
  else
    {
      ListExpr coordinates =  nl->TwoElemList(
      				nl->RealAtom( cpoint->p.GetX() ),
      				nl->RealAtom( cpoint->p.GetY() )); 
                  
      return nl->TwoElemList(
            nl->RealAtom( cpoint->GetEpsilon() ),
            coordinates );
    }
}

/*
~In~-function

*/

Word InCPoint( const ListExpr typeInfo, const ListExpr instance,
               const int errorPos, ListExpr& errorInfo, bool& correct )
{
  string errmsg;
  if ( nl->ListLength( instance ) == 2 )    
  // 2 arguments are necessary: epsilon and a point
  {
    ListExpr first = nl->First( instance );               // the epsilon value
    ListExpr second = nl->Second( instance );    // the point representation
    
    if ( nl->IsAtom( first ) && (nl->AtomType( first ) == RealType ||
            nl->AtomType( first ) == IntType) )
    {
      // The following commands are switched 'off' because they caused 
      // to crash the SecondoTTYBDB while updating a cpoint-object.
      // The only error-message was: 
      //*** glibc detected ***  free(): invalid pointer: 0xbfab9dc8 ***
      
      /*double e;
      if (nl->AtomType( first ) == IntType)
        e = nl->IntValue( first );
      else if(nl->AtomType( first ) == RealType)
        e = nl->RealValue( first );
      else
        correct = false;
        
      if ( !correct )
      {
        errmsg = "InCPoint(): First instant must be a real or int.";
        errorInfo = nl->Append(errorInfo, nl->StringAtom(errmsg));
        delete &e;
        return SetWord( Address(0) );
      }*/
      
      if ( nl->ListLength( second ) == 2 &&
            nl->IsAtom( nl->First( second )) &&
            (nl->AtomType( nl->First( second )) == RealType ||
            nl->AtomType( nl->First( second )) == IntType) &&
            nl->IsAtom( nl->Second( second )) &&
            (nl->AtomType( nl->Second( second )) == RealType ||
            nl->AtomType( nl->Second( second )) == IntType))
      // if the second list element contains two real- or int-values, 
      // representing point-coordinates
      { 
        correct = true;
        Point *p = (Point *)InPoint( nl->TheEmptyList(), second, 
                                        errorPos, errorInfo, correct ).addr;
        if ( !correct )
        {
          errmsg = "InCPoint(): Second instant must be a representation" 
                         "of a point value.";
          errorInfo = nl->Append(errorInfo, nl->StringAtom(errmsg));
          delete &p;
          return SetWord( Address(0) );
        }
        
        CPoint* cpoint = new CPoint( nl->RealValue( first ), 
                (StandardAttribute*) p );
        delete p;
        //delete &e;
        correct = cpoint->IsValid();
        
        if ( !correct )
        {
          errmsg = "InCPoint(): The cpoint-value is invalid!";
          errorInfo = nl->Append(errorInfo, nl->StringAtom(errmsg));
          delete p;
          return SetWord( Address(0) );
        }
        return SetWord( cpoint );
      }
      else
      {
        correct = false;
        errmsg = "InCPoint(): Second instant must be a representation" 
                         "of a point value.";
        errorInfo = nl->Append(errorInfo, nl->StringAtom(errmsg));
      }
    }
    else
    {
      correct = false;
      errmsg = "InCPoint(): Error in first instant. First instant must be an "
            "atomic value of type Real.";
      errorInfo = nl->Append(errorInfo, nl->StringAtom(errmsg));
    }
  }
  errmsg = "InCPoint(): List must contain 2 elements. ";
  errorInfo = nl->Append(errorInfo, nl->StringAtom(errmsg));
  return SetWord( Address(0) );
}


/*
~Create~-function

*/
Word CreateCPoint( const ListExpr typeInfo )
{
  return (SetWord( new CPoint() ));
}


/*
~Delete~-function

*/
void DeleteCPoint( const ListExpr typeInfo, Word& w )
{
  delete (CPoint *)w.addr;
  w.addr = 0;
}


/*
~Close~-function

*/
void CloseCPoint( const ListExpr typeInfo, Word& w )
{
  delete (CPoint *)w.addr;
  w.addr = 0;
}


/*
~Clone~-function

*/
Word CloneCPoint( const ListExpr typeInfo, const Word& w )
{
  CPoint *cpoint = (CPoint *)w.addr;
  return SetWord( new CPoint( *cpoint ) );
}


/*
~Sizeof~-function

*/
int SizeOfCPoint()
{
  return sizeof(CPoint);
}


/*
~Cast~-function

*/

void * CastCPoint(void* addr)
{
  return new (addr) CPoint;
}

/*
Creation of the type constructor ~cpoint~

*/

TypeConstructor uncertainpoint(
        "cpoint",                //name
        CPointProperty,     //property function describing signature
        OutCPoint,
        InCPoint,               //Out and In functions
        0,
        0,                         //SaveToList and RestoreFromList functions
        CreateUncertain<Point>,
        DeleteUncertain<Point>,        //object creation and deletion
        0,
        0,                         // object open and save
        CloseCPoint,   
        CloneCPoint,         //object close and clone
        CastCPoint,           //cast function
        SizeOfCPoint,       //sizeof function
        CheckCPoint );      //kind checking function


/*
4.2 The Type Constructor ~cupoint~

Type ~cupoint~ represents a pair (epsilon, (tinterval, (x0, y0, x1, y1)))
consisting of an uncertainty-value and a value of type upoint.

4.2.1 List Representation

The list representation of an ~upoint~ is

----   ( epsilon ( timeinterval (x0 yo x1 y1) ) )
----

For example:

----    ( 37.5 ( ( (instant 6.37)  (instant 9.9)   TRUE FALSE)   
----                    (1.0 2.3 4.1 2.1) ) )

4.2.2 function Describing the Signature of the Type Constructor

*/
ListExpr CUPointProperty()
{
  return (nl->TwoElemList(
          nl->FourElemList(nl->StringAtom("Signature"),
                  nl->StringAtom("Example Type List"),
                  nl->StringAtom("List Rep"),
                  nl->StringAtom("Example List")),
          nl->FourElemList(nl->StringAtom("-> UNCERTAIN UNIT"),
                  nl->StringAtom("(cupoint) "),
                  nl->TextAtom("( epsilon ( timeInterval "
                          "(real_x0 real_y0 real_x1 real_y1) ) ) "),
                  nl->StringAtom("(0.7 ((i1 i2 TRUE FALSE)" 
                          "(1.0 2.2 2.5 2.1)))"))));
}


/*
4.2.3 Kind Checking Function

*/
bool CheckCUPoint( ListExpr type, ListExpr& errorInfo )
{
  return (nl->IsEqual( type, "cupoint" ));
}

/*
4.2.4 ~Out~-function

*/
ListExpr OutCUPoint( ListExpr typeInfo, Word value )
{
  CUPoint* cupoint = (CUPoint*)(value.addr);
  
  if( !(((CUPoint*)value.addr)->IsValid()) )
    return (nl->SymbolAtom("undef"));
  else
    {
      ListExpr timeintervalList = nl->FourElemList(
          OutDateTime( nl->TheEmptyList(),
          SetWord(&cupoint->timeInterval.start) ),
          OutDateTime( nl->TheEmptyList(), 
                  SetWord(&cupoint->timeInterval.end) ),
          nl->BoolAtom( cupoint->timeInterval.lc ),
          nl->BoolAtom( cupoint->timeInterval.rc));
      
      ListExpr pointsList = nl->FourElemList(
          nl->RealAtom( cupoint->p0.GetX() ),
          nl->RealAtom( cupoint->p0.GetY() ),
          nl->RealAtom( cupoint->p1.GetX() ),
          nl->RealAtom( cupoint->p1.GetY() ));
      
      ListExpr unitpointList = nl->TwoElemList(
          timeintervalList, pointsList );
          
      return nl->TwoElemList( nl->RealAtom( cupoint->GetEpsilon() ), 
          unitpointList );
    }
}

/*
4.2.5 ~In~-function

The Nested list form is like this:  
  ( 37.4 ( ( 6.37  9.9  TRUE FALSE)   (1.0 2.3 4.1 2.1) ) )

*/
Word InCUPoint( const ListExpr typeInfo, const ListExpr instance,
               const int errorPos, ListExpr& errorInfo, bool& correct )
{
  string errmsg;
  if ( nl->ListLength( instance ) == 2 )    
  // 2 arguments are necessary: epsilon and a upoint
  {
    ListExpr first = nl->First( instance );               // the epsilon value
    ListExpr second = nl->Second( instance );    // the upoint representation
    
    if ( nl->IsAtom( first ) && (nl->AtomType( first ) == RealType ||
            nl->AtomType( first ) == IntType ) )
    {
      double e;
      if (nl->AtomType( first ) == IntType)
        e = nl->IntValue( first );
      else if(nl->AtomType( first ) == RealType)
        e = nl->RealValue( first );
      
      if ( nl->ListLength( second ) == 2 )
      // the upoint also consists of two components...
      {
        ListExpr tintvl = nl->First( second );        // the time-interval
        ListExpr endpoints = nl->Second( second );     // the two point values
      
        if( nl->ListLength( tintvl ) == 4 &&
            nl->IsAtom( nl->Third( tintvl ) ) &&
            nl->AtomType( nl->Third( tintvl ) ) == BoolType &&
            nl->IsAtom( nl->Fourth( tintvl ) ) &&
            nl->AtomType( nl->Fourth( tintvl ) ) == BoolType )
        {
          correct = true;
          Instant *start = (Instant *)InInstant( nl->TheEmptyList(),
             nl->First( tintvl ), errorPos, errorInfo, correct ).addr;
    
          if( !correct )
          {
            errmsg = "InCUPoint(): Error in time-interval defining instant.";
            errorInfo = nl->Append(errorInfo, nl->StringAtom(errmsg));
            delete start;
            return SetWord( Address(0) );
          }
      
          Instant *end = (Instant *)InInstant( nl->TheEmptyList(),
              nl->Second( tintvl ), errorPos, errorInfo, correct ).addr;
      
          if( !correct )
          {
            errmsg = "InCUPoint(): Error in time-interval defining instant.";
            errorInfo = nl->Append(errorInfo, nl->StringAtom(errmsg));
            delete start;
            delete end;
            return SetWord( Address(0) );
          }
    
          Interval<Instant> tinterval( *start, *end,
                                       nl->BoolValue( nl->Third( tintvl ) ),
                                       nl->BoolValue( nl->Fourth( tintvl ) ) );
          delete start;
          delete end;
        
          correct = tinterval.IsValid();
          if (!correct)
          {
            errmsg = "InCUPoint(): Non valid time interval.";
            errorInfo = nl->Append(errorInfo, nl->StringAtom(errmsg));
            return SetWord( Address(0) );
          }

          if( nl->ListLength( endpoints ) == 4 )
          {
            Coord x0, y0, x1, y1;
            
            if( nl->IsAtom( nl->First( endpoints ) ) &&
                nl->AtomType( nl->First( endpoints )) == IntType)
              x0 = nl->IntValue(nl->First( endpoints ));
            else if ( nl->IsAtom( nl->First( endpoints ) ) &&
                nl->AtomType( nl->First( endpoints )) == RealType)
              x0 = nl->RealValue(nl->First( endpoints ));
            else
              correct = false;
            
            if( nl->IsAtom( nl->Second( endpoints ) ) &&
                nl->AtomType( nl->Second( endpoints )) == IntType)
              y0 = nl->IntValue(nl->Second( endpoints ));
            else if ( nl->IsAtom( nl->Second( endpoints ) ) &&
                nl->AtomType( nl->Second( endpoints )) == RealType)
              y0 = nl->RealValue(nl->Second( endpoints ));
            else
              correct = false;
            
            if( nl->IsAtom( nl->Third( endpoints ) ) &&
                nl->AtomType( nl->Third( endpoints )) == IntType)
              x1 = nl->IntValue(nl->Third( endpoints ));
            else if ( nl->IsAtom( nl->Third( endpoints ) ) &&
                nl->AtomType( nl->Third( endpoints )) == RealType)
              x1 = nl->RealValue(nl->Third( endpoints ));
            else
              correct = false;
            
            if( nl->IsAtom( nl->Fourth( endpoints ) ) &&
                nl->AtomType( nl->Fourth( endpoints )) == IntType)
              y1 = nl->IntValue(nl->Fourth( endpoints ));
            else if ( nl->IsAtom( nl->Fourth( endpoints ) ) &&
                nl->AtomType( nl->Fourth( endpoints )) == RealType)
              y1 = nl->RealValue(nl->Fourth( endpoints ));
            else
              correct = false;
            
            if( !correct )
            {
              errmsg = "InCUPoint(): Non valid point-coordinates.";
              errorInfo = nl->Append(errorInfo, nl->StringAtom(errmsg));
              return SetWord( Address(0) );
            }
              
            CUPoint *cupoint = new CUPoint( e, tinterval, x0, y0, x1, y1  );
    
            correct = cupoint->UnitIsValid();
            if( correct )
              return SetWord( cupoint );
        
            errmsg = errmsg + "InCUPoint(): Error in start/end point.";
            errorInfo = nl->Append(errorInfo, nl->StringAtom(errmsg));
            delete cupoint;
          }
        }
      }
    }
  }
  else if ( nl->IsAtom( instance ) && nl->AtomType( instance ) == SymbolType
            && nl->SymbolValue( instance ) == "undef" )
  {
    CUPoint *cupoint = new CUPoint(false);
    cupoint->timeInterval=
      Interval<DateTime>(DateTime(instanttype),
                         DateTime(instanttype),true,true);
    correct = cupoint->timeInterval.IsValid();
    if ( correct )
      return (SetWord( cupoint ));
  }
  errmsg = "InCUPoint(): Error in representation.";
  errorInfo = nl->Append(errorInfo, nl->StringAtom(errmsg));
  correct = false;
  return SetWord( Address(0) );
}


/*
4.2.6 ~Create~-function

*/
Word CreateCUPoint( const ListExpr typeInfo )
{
  return (SetWord( new CUPoint() ));
}

/*
4.2.7 ~Delete~-function

*/
void DeleteCUPoint( const ListExpr typeInfo, Word& w )
{
  delete (CUPoint *)w.addr;
  w.addr = 0;
}

/*
4.2.8 ~Close~-function

*/
void CloseCUPoint( const ListExpr typeInfo, Word& w )
{
  delete (CUPoint *)w.addr;
  w.addr = 0;
}

/*
4.2.9 ~Clone~-function

*/
Word CloneCUPoint( const ListExpr typeInfo, const Word& w )
{
  CUPoint *cupoint = (CUPoint *)w.addr;
  return SetWord( new CUPoint( *cupoint ) );
}

/*
4.2.10 ~Sizeof~-function

*/
int SizeOfCUPoint()
{
  return sizeof(CUPoint);
}

/*
4.2.11 ~Cast~-function

*/
void* CastCUPoint( void* addr ) 
{
  return (new (addr) CUPoint);
}

/*
Creation of the type constructor ~cupoint~

*/

TypeConstructor uncertainunitpoint(
        "cupoint",                //name
        CUPointProperty,     //property function describing signature
        OutCUPoint,
        // For consequent implementation, the Out-function in the previous line
        // should be 'OutUncertain<Point, OutPoint>' instead of 'OutCPoint',
        // but the use of 'OutUncertain...' leads to a compiler error according
        // to this template-function in 'HierarchicalGeoAlgebra.h'! See there 
        // for further information! (Sascha Vaut)
        InCUPoint,               //Out and In functions
        0,
        0,                         //SaveToList and RestoreFromList functions
        CreateCUPoint,
        DeleteCUPoint,        //object creation and deletion
        0,
        0,                         // object open and save
        CloseCUPoint,   
        CloneCUPoint,         //object close and clone
        CastCUPoint,           //cast function
        SizeOfCUPoint,       //sizeof function
        CheckCUPoint );      //kind checking function


/*
4.3 Type Constructor CMPoint

Type ~cmpoint~ represents a moving point.

4.3.1 List Representation

The list representation of a ~cmpoint~ is

----    ( u1 ... un )
----

,where u1, ..., un are units of type ~cupoint~.

For example:

----  (
          ( 0.4 
              ((instant 6.37)  (instant 9.9)   TRUE FALSE) (1.0 2.3 4.1 2.1)))
          ( 0.5
              ((instant 11.4)  (instant 13.9)  FALSE FALSE) (4.1 2.1 8.9 4.3)))
        )
----

4.3.2 function Describing the Signature of the Type Constructor

*/
ListExpr CMPointProperty()
{
  ListExpr examplelist = nl->TextAtom();
  nl->AppendText(examplelist,"((0.42((i1 i2 TRUE FALSE) (1.0 2.2 2.5 2.1)))" 
          " ...)");
  
  return (nl->TwoElemList(
      nl->FourElemList(nl->StringAtom("Signature"),
                         nl->StringAtom("Example Type List"),
                         nl->StringAtom("List Rep"),
                         nl->StringAtom("Example List")),
      nl->FourElemList(nl->StringAtom("-> MAPPING"),
                         nl->StringAtom("(cmpoint) "),
                         nl->StringAtom("( u1 ... un ) "),
                         examplelist)));
}

/*
4.3.3 Kind Checking Function

*/
bool
CheckCMPoint( ListExpr type, ListExpr& errorInfo )
{
  return (nl->IsEqual( type, "cmpoint" ));
}

TypeConstructor uncertainmovingpoint(
        "cmpoint",                           //name
        CMPointProperty,        //property function describing signature
        OutMapping<CMPoint, CUPoint, OutCUPoint>,
        InMapping<CMPoint, CUPoint, InCUPoint>,//Out and In functions
        0,
        0,                 //SaveToList and RestoreFromList functions
        CreateMapping<CMPoint>,
        DeleteMapping<CMPoint>,     //object creation and deletion
        0,
        0,      // object open and save
        CloseMapping<CMPoint>,
        CloneMapping<CMPoint>, //object close and clone
        CastMapping<CMPoint>,    //cast function
        SizeOfMapping<CMPoint>, //sizeof function
        CheckCMPoint );  //kind checking function


/*
Type Constructor +++++ hier weitere Typkonstruktoren anfuegen +++++

5 Operators

Definition of operators is similar to definition of type constructors. An
operator is defined by creating an instance of class ~Operator~. Again we
have to define some functions before we are able to create an ~Operator~
instance.

5.1 Type mapping functions

A type mapping function takes a nested list as argument. Its contents are
type descriptions of an operator's input parameters. A nested list describing
the output type of the operator is returned.


5.1.1 Type mapping function ~UncertainTypeMapReal~

This type mapping function is used for the Operation ~Epsilon()~.

*/
ListExpr UncertainTypeMapReal( ListExpr args )
{
  if ( nl->ListLength( args ) == 1 )
  {
    ListExpr arg1 = nl->First( args );
    
    if ( nl->IsEqual( arg1, "cpoint" ) || 
          nl->IsEqual( arg1, "cupoint" ) || 
          nl->IsEqual( arg1, "cmpoint" ) )
      return nl->SymbolAtom("real");
    if (nl->AtomType( args ) == SymbolType)
    {
      ErrorReporter::ReportError("Type mapping function got a "
              "parameter of type " +nl->SymbolValue(args) + ".");
      return nl->SymbolAtom("typeerror");
    }
  }
  ErrorReporter::ReportError("Type mapping function got a "
        "parameter of length != 1.");
  return nl->SymbolAtom("typeerror");
}

/*
5.1.2 Type mapping function ~UncertainTypeMapBase~

This type mapping function is used for the Operation ~Val()~. The keyword
'base' indicates a reduction of an uncertain type to its particular base type.
So in this case a 'base type' can also be a spatial or temporal type.

*/

ListExpr UncertainTypeMapBase( ListExpr args )
{
  if ( nl->ListLength( args ) == 1 )
  {
    ListExpr arg1 = nl->First( args );
    
    if( nl->IsEqual( arg1, "cpoint") )
      return nl->SymbolAtom( "point" );
      
    if( nl->IsEqual( arg1, "cupoint") )
      return nl->SymbolAtom( "upoint" );
    if (nl->AtomType( args ) == SymbolType)
    {
      ErrorReporter::ReportError("Type mapping function got a "
              "parameter of type " +nl->SymbolValue(args) + 
              "which is no uncertain type.");
      return nl->SymbolAtom("typeerror");
    }
  }
  ErrorReporter::ReportError("Type mapping function got a "
        "parameter of length != 1.");
  return nl->SymbolAtom( "typeerror" );
}


/*
5.1.3 Type mapping function ~CertainToUncertain~

This type mapping function is used for the ~<certaintype>to<uncertaintype>~ 
Operations.

*/

ListExpr CertainToUncertain( ListExpr args )
{
  if ( nl->ListLength( args ) == 2 )
  {
    ListExpr first = nl->First(args);
    ListExpr second = nl->Second(args);
    if ( nl->IsEqual( first, "real" ) )
    {
      // find out if the second argument is of an implemented uncertain type
      if ( nl->IsEqual( second, "point" ) )
        return nl->SymbolAtom("cpoint");
      if ( nl->IsEqual( second, "upoint" ) )
        return nl->SymbolAtom("cupoint");
    }
    if ( (nl->AtomType(first) == SymbolType) && (nl->AtomType(second) == 
            SymbolType))
      ErrorReporter::ReportError("Type mapping function got parameters of "
        "type "
          + nl->SymbolValue(first) + " and "
          + nl->SymbolValue(second));
    else
      ErrorReporter::ReportError("Type mapping function got wrong types "
        "as parameters.");
  }
  ErrorReporter::ReportError("Type mapping function got a parameter of length "
    "!= 2.");
  return nl->SymbolAtom("typeerror");
}

/*
5.1.7 Type mapping function ~UncertainTempSetValueTypeMapInt~

It is for the ~no\_components~ operator.

*/
ListExpr UncertainTempSetValueTypeMapInt( ListExpr args )
{
  if ( nl->ListLength( args ) == 1 )
  {
    ListExpr arg1 = nl->First( args );

    if( nl->IsEqual( arg1, "cmpoint" ) )
      return nl->SymbolAtom( "int" );
  }
  return nl->SymbolAtom( "typeerror" );
}

/*
5.1.8 Type mapping function ~UncertainMovingTypeMapSpatial~

This is for the operator ~trajectory~.

*/
ListExpr UncertainMovingTypeMapSpatial( ListExpr args )
{
  if ( nl->ListLength( args ) == 1 )
  {
    ListExpr arg1 = nl->First( args );

    if( nl->IsEqual( arg1, "cmpoint" ) ||
        nl->IsEqual( arg1, "cupoint") )
      return nl->SymbolAtom( "region" );
  }
  return nl->SymbolAtom( "typeerror" );
}


/*
5.1.9 Type mapping function ~UncertainMovingTypeMapTemporal~

This is defined for the operators ~deftime~.

*/
ListExpr UncertainMovingTypeMapPeriods( ListExpr args )
{
  if ( nl->ListLength( args ) == 1 )
  {
    ListExpr arg1 = nl->First( args );

    if( nl->IsEqual( arg1, "cmpoint" ) )
      return nl->SymbolAtom( "periods" );
  }
  return nl->SymbolAtom( "typeerror" );
}

/*
5.1.10 Type mapping function ~UncertainMovingTypeMapBool~

It is for the operator ~present~.

*/
ListExpr UncertainMovingInstantPeriodsTypeMapBool( ListExpr args )
{
   if ( nl->ListLength( args ) == 2 )
  {
    ListExpr arg1 = nl->First( args ),
             arg2 = nl->Second( args );

    if( nl->IsEqual( arg2, "instant" ) ||
        nl->IsEqual( arg2, "periods" ) )
    {
      if( nl->IsEqual( arg1, "cmpoint") )

        return nl->SymbolAtom( "bool" );
    }
  }
  return nl->SymbolAtom( "typeerror" );
}

/*
5.1.11 Type mapping function ~UncertainMovingTypeMapBool~

This is the type mapping function for the operators ~d_passes~ and ~p_passes~.

*/
ListExpr UncertainMovingTypeMapBool( ListExpr args )
{
   if ( nl->ListLength( args ) == 2 )
  {
    ListExpr arg1 = nl->First( args ),
             arg2 = nl->Second( args );

    if( nl->IsEqual( arg2, "point" ) ||
        nl->IsEqual( arg2, "region" ) )
    {
      if( nl->IsEqual( arg1, "cmpoint") ||
          nl->IsEqual( arg1, "cupoint") )
        return nl->SymbolAtom( "bool" );
    }
  }
  return nl->SymbolAtom( "typeerror" );
}

/*
5.1.12 Type mapping function ~UncertainMovingTypeMapCMPoint~

It is for the operators ~atperiods~, ~definitely_at~, ~possibly_at~

*/


/*
5.1.12 Type mapping function ~UncertainMovingTypeMapeIRegion~

It is for the operator ~atinstant~.

*/
ListExpr UncertainMovingInstantTypeMapIntime( ListExpr args )
{
  if ( nl->ListLength( args ) == 2 )
  {
    ListExpr arg1 = nl->First( args ),
             arg2 = nl->Second( args );

    if( nl->IsEqual( arg2, "instant" ) )
    {
      if( nl->IsEqual( arg1, "cmpoint" ) )
        return nl->SymbolAtom( "intimeregion" );
    }
  }
  return nl->SymbolAtom( "typeerror" );
}

/*
5.1.13 Type mapping function ~UncertainMovingPeriodsTypeMapMoving~

It is for the operator ~atperiods~.

*/
ListExpr UncertainMovingPeriodsTypeMapMoving( ListExpr args )
{
  if ( nl->ListLength( args ) == 2 )
  {
    ListExpr arg1 = nl->First( args ),
             arg2 = nl->Second( args );

    if( nl->IsEqual( arg2, "periods" ) )
    {
      if( nl->IsEqual( arg1, "cmpoint" ) )
        return nl->SymbolAtom( "cmpoint" );
    }
  }
  return nl->SymbolAtom( "typeerror" );
}

/*
5.1.14 Type Mapping Function ~UncertainMovingTypeMapUnits~

It is used for the operator ~units~

Type mapping for ~units~ is

----    (mpoint) -> (stream upoint)
----

*/
ListExpr UncertainMovingTypeMapUnits( ListExpr args )
{
  if ( nl->ListLength(args) == 1 )
  {
    ListExpr arg1 = nl->First(args);

    if( nl->IsEqual( arg1, "cmpoint" ) )
      return nl->TwoElemList(nl->SymbolAtom("stream"),
       nl->SymbolAtom("cupoint"));
    else
      ErrorReporter::ReportError("Type mapping function got wrong "
                            "types as parameters.");
  }
  ErrorReporter::ReportError("Type mapping function got a "
                      "parameter of length != 1.");
  return nl->SymbolAtom("typeerror");
}

/*
5.1.18 Type mapping function "UncertainTemporalBBoxTypeMap"

For operator ~bbox~

*/

ListExpr UncertainTemporalBBoxTypeMap( ListExpr args )
{
  ListExpr arg1;
  if ( nl->ListLength( args ) == 1 )
  {
    arg1 = nl->First( args );

    if( nl->IsEqual( arg1, "cupoint" ) )
      return (nl->SymbolAtom( "rect3" ));

    if( nl->IsEqual( arg1, "cmpoint" ) )
      return (nl->SymbolAtom( "rect3" ));

    //if( nl->IsEqual( arg1, "cipoint" ) )
    //  return (nl->SymbolAtom( "rect3" ));

  }
  return nl->SymbolAtom( "typeerror" );
}


/*
5.1.19 Type Mapping Function ~MovingTypeMapUnits~

It is used for the operator ~units~

Type mapping for ~units~ is

----    (cmpoint) -> (stream cupoint)
----

*/
ListExpr UncertainTemporalTypeMapUnits( ListExpr args )
{
  if ( nl->ListLength(args) == 1 )
  {
    ListExpr arg1 = nl->First(args);

    if( nl->IsEqual( arg1, "cmpoint" ) )
      return nl->TwoElemList(nl->SymbolAtom("stream"),
       nl->SymbolAtom("cupoint"));
  }
  return nl->SymbolAtom("typeerror");
}

/*
5.2 Selection function

A selection function is quite similar to a type mapping function. The only
difference is that it doesn't return a type but the index of a value
mapping function being able to deal with the respective combination of
input parameter types.

Note that a selection function does not need to check the correctness of
argument types; it has already been checked by the type mapping function that
it is applied to correct arguments.

5.2.1 Selection function ~UncertainSimpleSelect~

Is used for the ~epsilon~ and ~val~ operators.

*/
int UncertainSimpleSelect( ListExpr args )
{
  ListExpr arg1 = nl->First( args );
  
  if( nl->SymbolValue( arg1 ) == "cpoint" )
    return 0;
    
  if( nl->SymbolValue( arg1 ) == "cupoint" )
    return 1;
    
  if( nl->SymbolValue( arg1 ) == "cmpoint" )
    return 2;
  // ...space for further possible argument types
  
  return -1; // This point should never be reached
}

int UncertainPassesSelect( ListExpr args )
{
  ListExpr arg1 = nl->First( args );
  ListExpr arg2 = nl->Second( args );
  
  if( nl->SymbolValue( arg1 ) == "cupoint" &&
      nl->SymbolValue( arg2 ) == "point" )
    return 0;
  
  if( nl->SymbolValue( arg1 ) == "cupoint" &&
      nl->SymbolValue( arg2 ) == "region" )
    return 1;
  
  if( nl->SymbolValue( arg1 ) == "cmpoint" &&
      nl->SymbolValue( arg2 ) == "point" )
    return 2;
    
  if( nl->SymbolValue( arg1 ) == "cmpoint" &&
      nl->SymbolValue( arg2 ) == "region" )
    return 3;    
  
  return -1; // This point should never be reached
}

/*
5.2.2 Selection function ~UncertainMovingInstantPeriodsSelect~

*/
int UncertainMovingInstantPeriodsSelect( ListExpr args )
{
  ListExpr arg1 = nl->Second( args );
  
  if( nl->SymbolValue( arg1 ) == "instant" )
    return 0;
    
  if( nl->SymbolValue( arg1 ) == "periods" )
    return 1;
    
  return -1; // This point should never be reached
}

/*
6 Value mapping functions

6.1 Value mapping functions for class cpoint


6.1.1 Value mapping function for operator ~tocpoint~

*/

int ToCPoint( Word* args, Word& result, int message, Word& local,
                                        Supplier s )
{
  result = qp->ResultStorage( s );
  CcReal* e = (CcReal*)args[0].addr;
  Point* p = (Point*)args[1].addr;
  
  if ( e >= 0 )
    if ( p->IsDefined() )
    {
      CPoint cp( e->GetValue(), (StandardAttribute*) p );
      ((CPoint*)result.addr)->Set(cp);
    }
    else
    {
      ((CPoint*)result.addr)->UncertainSetDefined( false );
      cerr << "Result object is set to state: defined = false." << endl;
    }
  return 0;
}

/*
6.2 Value mapping functions for class cupoint

6.2.1 Value mapping function for operator ~trajectory~

*/
int CUPointTrajectory( Word* args, Word& result, int message,
 Word& local, Supplier s )
{
  result = qp->ResultStorage( s );

  Region *region = ((Region*)result.addr);
  CUPoint *cupoint = ((CUPoint*)args[0].addr);
  cupoint->UTrajectory( *region );

  return 0;
}



/*
6.3 Value mapping functions for class cmpoint

*/

/*
6.3.2 Value mapping function for operator ~trajectory~

*/
/*int MPointTrajectory( Word* args, Word& result, int message,
 Word& local, Supplier s )
{
  result = qp->ResultStorage( s );

  Region *region = ((Region*)result.addr);
  CMPoint *mpoint = ((CMPoint*)args[0].addr);
  cmpoint->Trajectory( *region );

  return 0;
}*/

/*
6.3.3 Value mapping function for operator ~present~

*/

// If the second argument is an Instant:
int CMPointPresent_i( Word* args, Word& result,
                     int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );

  CMPoint *m = ((CMPoint*)args[0].addr);
  Instant* inst = ((Instant*)args[1].addr);

  if( !inst->IsDefined() )
    ((CcBool *)result.addr)->Set( false, false );
  else if( m->Present( *inst ) )
    ((CcBool *)result.addr)->Set( true, true );
  else
    ((CcBool *)result.addr)->Set( true, false );

  return 0;
}

// If the second argument is a Period:
int CMPointPresent_p( Word* args, Word& result,
                         int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );

  CMPoint *m = ((CMPoint*)args[0].addr);
  Periods* periods = ((Periods*)args[1].addr);

  if( periods->IsEmpty() )
    ((CcBool *)result.addr)->Set( false, false );
  else if( m->Present( *periods ) )
    ((CcBool *)result.addr)->Set( true, true );
  else
    ((CcBool *)result.addr)->Set( true, false );

  return 0;
}

/*
6.3.4 Value mapping function for operator ~atinstant~

*/
int CMPointAtInstant( Word* args, Word& result, int message,
                          Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  CMPoint* cmp = ((CMPoint*)args[0].addr);
  Instant* inst = (Instant*) args[1].addr;
  Intime<Region>* pResult = (Intime<Region>*)result.addr;

  cmp->AtInstant(*inst, *pResult);
  return 0;
}

/*
6.3.5 Value mapping function for operator ~atperiods~

*/
int CMPointAtPeriods( Word* args, Word& result, int message,
                          Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  CMPoint* cmp = ((CMPoint*)args[0].addr);
  CMPoint* pResult = (CMPoint*)result.addr;
  Periods* per = (Periods*)args[1].addr;

  cmp->AtPeriods(*per,*pResult);
  return 0;
}

/*
6.3.6 Value mapping function for operator ~units~

*/

int UncertainMappingUnits(Word* args, Word& result, int message, 
                Word& local, Supplier s)
{
  CMPoint* m;
  const CUPoint* unit;
  UnitsLocalInfo *localinfo;

  switch( message )
  {
    case OPEN:

      localinfo = new UnitsLocalInfo;
      localinfo->mWord = args[0];
      localinfo->unitIndex = 0;
      local = SetWord(localinfo);
      return 0;

    case REQUEST:
      
      if( local.addr == 0 )
        return CANCEL;
      localinfo = (UnitsLocalInfo *) local.addr;
      m = (CMPoint*)localinfo->mWord.addr;
      if( (0 <= localinfo->unitIndex)
          && (localinfo->unitIndex < m->GetNoComponents()) )
      {
        m->Get( localinfo->unitIndex++, unit );
        CUPoint* aux = new CUPoint( *unit );

        //Attribute* attr = static_cast<Attribute*>(aux);
        //SHOW( attr->NoRefs() )
        //SHOW( (void*)attr )
        //SHOW( (void*)aux )

        result = SetWord( aux );
        return YIELD;
      }
      return CANCEL;

    case CLOSE:
        
      if( local.addr != 0 )
        delete (UnitsLocalInfo *)local.addr;
      return 0;
  }
  /* should not happen */
  return -1;
}

/*
6.3.7 value mapping functions for operator ~d_passes~

*/

// If the first argument is a CUPoint and the second one is a point:
int CUPointD_PassesPoint(Word* args, Word& result, int message, 
                Word& local, Supplier s)
{
  result = qp->ResultStorage( s );

  CUPoint *u = ((CUPoint*)args[0].addr);
  Point* p = ((Point*)args[1].addr);

  if( !p->IsDefined() )
    ((CcBool *)result.addr)->Set( false, false );
  else if( u->D_Passes( *p ) )
    ((CcBool *)result.addr)->Set( true, true );
  else
    ((CcBool *)result.addr)->Set( true, false );

  return 0;
}


// If the first argument is a CUPoint and the second one is a region:
int CUPointD_PassesRegion(Word* args, Word& result, int message, 
                Word& local, Supplier s)
{
  result = qp->ResultStorage( s );

  CUPoint *u = ((CUPoint*)args[0].addr);
  Region* r = ((Region*)args[1].addr);

  if( !r->IsDefined() )
    ((CcBool *)result.addr)->Set( false, false );
  else if( u->D_Passes( *r ) )
    ((CcBool *)result.addr)->Set( true, true );
  else
    ((CcBool *)result.addr)->Set( true, false );

  return 0;
}
/*
// If the first argument is a CMPoint and the second one is a point:
int CMPointD_PassesPoint(Word* args, Word& result, int message, 
                Word& local, Supplier s)
{
  
}

// If the first argument is a CMPoint and the second one is a region:
int CMPointD_PassesPoint(Word* args, Word& result, int message, 
                Word& local, Supplier s)
{
  
}
*/

/*
Definition of operators

Definition of operators is done in a way similar to definition of 
type constructors: an instance of class ~Operator~ is defined.

Because almost all operators are overloaded, we have first to define an 
array of value mapping functions for each operator. For nonoverloaded
operators there is also such an array defined, so it is easier to make them
overloaded.

ValueMapping arrays

*/

ValueMapping uncertainepsilonmap[] = { 
                                      UncertainEpsilon<Point>,
                                      UncertainEpsilon<Point>,
                                      UncertainEpsilon<Point> };


/*ValueMapping uncertainvalmap[] = {
                                      UncertainVal<Point>,
                                      UncertainVal<UPoint> };*/
                                      
ValueMapping uncertaintemporalpresentmap[] = {
                                      CMPointPresent_i,
                                      CMPointPresent_p };

ValueMapping uncertaindpassesmap[] = {
                                      CUPointD_PassesPoint,
                                      CUPointD_PassesRegion//,
                                      //CMPointD_PassesPoint,
                                      /*CMPointD_PassesRegion*/ };
/*
Specification strings

*/

const string UncertainSpecEpsilon  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>uT -> epsilon</text--->"
  "<text>epsilon ( _ )</text--->"
  "<text>Returns an uncertain values' epsilon value.</text--->"
  "<text>epsilon ( i1 )</text--->"
  ") )";


/*const string UncertainSpecVal =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>uT -> x</text--->"
  "<text>val ( _ )</text--->"
  "<text>Returns an uncertain value's value.</text--->"
  "<text>val ( i1 )</text--->"
  ") )";*/


const string CPointSpecToCPoint =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>point, real -> cpoint</text--->"
  "<text>toCPoint ( _, _ )</text--->"
  "<text>Builds a new CPoint from the given Real- and Point-values.</text--->"
  "<text>cpt = tocpoint ( 50.0, alexanderplatz )</text--->"
  ") )";

const string TemporalSpecDefTime  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>uncertain moving point -> periods</text--->"
  "<text>deftime( _ )</text--->"
  "<text>Gets the defined time of the corresponding uncertain moving point."
  "</text--->"
  "<text>deftime( cmp1 )</text--->"
  ") )";

const string UncertainMovingSpecTrajectory = 
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>cmpoint -> region</text--->"
  "<text>trajectory ( _ )</text--->"
  "<text>Returns a Region-Object, representing the possible trajectory-area"
  "of the given uncertain moving point.</text--->"
  "<text>query trajectory ( mobilphone )</text--->"
  ") )";

const string UncertainTemporalSpecPresent  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>(cmT instant) -> bool,\n"
  "(cmT periods) -> bool</text--->"
  "<text>_ present _ </text--->"
  "<text>Checks whether the moving object is present at the given "
  "instant or period.</text--->"
  "<text>cmpoint1 present instant1</text--->"
  ") )";

const string UncertainTemporalSpecAtInstant =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>(cmpoint instant) -> iregion</text--->"
  "<text>_ atinstant _ </text--->"
  "<text>From an uncertain moving point, get the intime region "
  "representing the uncertain point of the instant.</text--->"
  "<text>cmpoint1 atinstant instant1</text--->"
  ") )";

const string UncertainTemporalSpecAtPeriods =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>(cmpoint periods) -> cmpoint</text--->"
  "<text>_ atperiods _ </text--->"
  "<text>Restrict the uncertain moving point to the given periods.</text--->"
  "<text>cmpoint1 atperiods periods1</text--->"
  ") )";

const string UncertainTemporalSpecUnits  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>For cmpoint -> (stream cupoint)</text--->"
  "<text> units( _ )</text--->"
  "<text>Get the stream of units of the uncertain moving point.</text--->"
  "<text>units( cmpoint1 )</text--->"
  ") )";

const string UncertainTemporalSpecDPasses =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>(cmpoint||cupoint x point||region ) -> bool</text--->"
  "<text>_ d_passes _ </text--->"
  "<text>Checks whether the uncertain moving object definitely passes the "
  "given spatial object.</text--->"
  "<text>cmpoint1 d_passes point1</text--->"
  ") )";

/*
Operators

*/

Operator uncertainepsilon( "epsilon",
                              UncertainSpecEpsilon,
                              3,
                              uncertainepsilonmap,
                              UncertainSimpleSelect,
                              UncertainTypeMapReal );

/*Operator uncertainval( "val",
                              UncertainSpecVal,
                              2,
                              uncertainvalmap,
                              UncertainSimpleSelect,
                              UncertainTypeMapBase );*/
                             
Operator tocpoint( "tocpoint",
                              CPointSpecToCPoint,
                              ToCPoint,
                              Operator::SimpleSelect,
                              CertainToUncertain );

Operator uncertaintrajectory( "trajectory",
                              UncertainMovingSpecTrajectory,
                              CUPointTrajectory,
                              Operator::SimpleSelect,
                              UncertainMovingTypeMapSpatial);    
                              
Operator uncertaintemporaldeftime( "deftime",
                              TemporalSpecDefTime,
                              MappingDefTime<CMPoint>,
                              Operator::SimpleSelect,
                              UncertainMovingTypeMapPeriods );    
                              
Operator uncertaintemporalpresent( "present",
                          UncertainTemporalSpecPresent,
                          2,
                          uncertaintemporalpresentmap,
                          UncertainMovingInstantPeriodsSelect,
                          UncertainMovingInstantPeriodsTypeMapBool);

Operator uncertaintemporalatinstant( "atinstant",
                            UncertainTemporalSpecAtInstant,
                            CMPointAtInstant,
                            Operator::SimpleSelect,
                            UncertainMovingInstantTypeMapIntime );

Operator uncertaintemporalatperiods( "atperiods",
                            UncertainTemporalSpecAtPeriods,
                            CMPointAtPeriods,
                            Operator::SimpleSelect,
                            UncertainMovingPeriodsTypeMapMoving );

Operator uncertaintemporalunits( "units",
                            UncertainTemporalSpecUnits,
                            UncertainMappingUnits,
                            Operator::SimpleSelect,
                            UncertainTemporalTypeMapUnits );

Operator uncertaintemporaldpasses( "d_passes",
                            UncertainTemporalSpecDPasses,
                            2,
                            uncertaindpassesmap,
                            UncertainPassesSelect,
                            UncertainMovingTypeMapBool );
                            

/*
Creating the Algebra
 
*/
class HierarchicalGeoAlgebra : public Algebra
{
  public:
  HierarchicalGeoAlgebra() : Algebra()
  {
    AddTypeConstructor( &uncertainpoint );
    uncertainpoint.AssociateKind( "DATA" );
    uncertainpoint.AssociateKind( "UNCERTAIN" );
    uncertainpoint.AssociateKind( "SPATIAL" );
    
    AddTypeConstructor( &uncertainunitpoint );
    uncertainunitpoint.AssociateKind( "DATA" );
    uncertainunitpoint.AssociateKind( "UNCERTAIN" );
    uncertainunitpoint.AssociateKind( "TEMPORAL" );
    
    AddTypeConstructor( &uncertainmovingpoint );
    uncertainmovingpoint.AssociateKind( "DATA" );
    uncertainmovingpoint.AssociateKind( "UNCERTAIN" );
    uncertainmovingpoint.AssociateKind( "TEMPORAL" );
    
    AddOperator( &uncertainepsilon );
    //AddOperator( &uncertainval );
    //AddOperator( &tocpoint );
    AddOperator( &uncertaintrajectory );
    AddOperator( &uncertaintemporaldeftime );
    AddOperator( &uncertaintemporalpresent );
    AddOperator( &uncertaintemporalatinstant );
    AddOperator( &uncertaintemporalatperiods );
    AddOperator( &uncertaintemporalunits );
    AddOperator( &uncertaintemporaldpasses );
  }
  ~HierarchicalGeoAlgebra() {};
};
HierarchicalGeoAlgebra hierarchicalGeoAlgebra;

/*
Initialization

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
InitializeHierarchicalGeoAlgebra( NestedList* nlRef, 
                                    QueryProcessor* qpRef )
{
  nl = nlRef;
  qp = qpRef;
  return (&hierarchicalGeoAlgebra);
}

