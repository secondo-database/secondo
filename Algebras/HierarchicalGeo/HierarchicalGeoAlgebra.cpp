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

This file contains the implementation of the type constructors
~uncertain~, ~cpoint~, ~cupoint~, ~cmpoint~, ~hcupoint~, ~hcmpoint~ and
~hmpoint~. The memory data structures used for these type constructors are
implemented in the HierarchicalGeoAlgebra.h file.

2 Defines, includes, and constants

*/

//#define TRACE_ON
#undef TRACE_ON
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
#include "SecParser.h"
#include "StandardTypes.h"
#include "SpatialAlgebra.h"
#include "PolySolver.h"
#include "RelationAlgebra.h"
//#include "StreamAlgebra.h"
#include "TemporalAlgebra.h"
#include "MovingRegionAlgebra.h"
#include "TypeMapUtils.h"
#include "Symbols.h"
#include "StringUtils.h"

#include <math.h>

extern NestedList* nl;
extern QueryProcessor* qp;

#include "DateTime.h"
#include "HierarchicalGeoAlgebra.h"



/*
2.1 Definition of some constants

*/
static const double minEpsilonLimiter = 0.00001;

/*
3 Implementation of C++ Classes

3.1 Template Class ~Uncertain~

*/


/*
3.2 Class ~CUPoint~

*/

void CUPoint::UTrajectory(const double e, Region& result ) const
{
  result.SetDefined( true );
  result.Clear();    // clear the result region

  double cmpepsilon;

  if( e < epsilon )
    cmpepsilon = epsilon;
  else
    cmpepsilon = e;

  if (AlmostEqual( p0, p1 ) )
  {
    // p0 almost equals p1, so the trajectory of this cupoint can be
    // represented as a circular region whose center is defined by p0 and
    // whose radius is defined by the uncertainty-value epsilon.
    Circle( p0, cmpepsilon, 16, result );
  }
  else
  {
    Coord x1, y1, x2, y2, x3, y3, x4, y4;
    Coord xleft, yleft, xright, yright;

    if ( (p0.GetX() < p1.GetX()) ||
         (p0.GetX() == p1.GetX() && p0.GetY() < p1.GetY() ) )
    {
      xleft = p0.GetX();
      yleft = p0.GetY();
      xright = p1.GetX();
      yright = p1.GetY();
    }
    else
    {
      xleft = p1.GetX();
      yleft = p1.GetY();
      xright = p0.GetX();
      yright = p0.GetY();
    }


    if (AlmostEqual(p0.GetX(), p1.GetX()) )
    {
      // The uncertain unit point moves along the y-axis.
      x1 = xleft - cmpepsilon;
      y1 = yleft;
      x2 = xleft + cmpepsilon;
      y2 = yleft;
      x3 = xright + cmpepsilon;
      y3 = yright;
      x4 = xright - cmpepsilon;
      y4 = yright;
    }
    else if (AlmostEqual(p0.GetY(), p1.GetY()) )
    {
      // The uncertain unit point moves along the x-axis.
      x1 = xleft;
      y1 = yleft + cmpepsilon;
      x2 = xleft;
      y2 = yleft - cmpepsilon;
      x3 = xright;
      y3 = yright - cmpepsilon;
      x4 = xright;
      y4 = yright + cmpepsilon;
    }
    else
    {
/*
Create 4 halfsegments as the edges of a rectangular box that defines the
uncertainty-area of this cupoint's trajectory.

To determine the edge-points of these halfsegments, the trigonometric
functions sin(alpha) and cos(alpha) are used:

*/
      double lengthX = fabs(xright - xleft);
      double lengthY = fabs(yright - yleft);
      double length = p0.Distance(p1);
      double sinalpha = lengthY / length;
      double cosalpha = lengthX / length;

      if (yleft < yright )
      {
        x1 = xleft - cmpepsilon * sinalpha;
        y1 = yleft + cmpepsilon * cosalpha;
        x2 = xleft + cmpepsilon * sinalpha;
        y2 = yleft - cmpepsilon * cosalpha;
        y3 = y2 + lengthY;
        y4 = y1 + lengthY;
      }
      else
      {
        x1 = xleft + cmpepsilon * sinalpha;
        y1 = yleft + cmpepsilon * cosalpha;
        x2 = xleft - cmpepsilon * sinalpha;
        y2 = yleft - cmpepsilon * cosalpha;
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

    HalfSegment hs(false, ep1, ep2);
    int partnerno = 0;
    int n = 16;
    double valueX, valueY;
    double angle;
    double radius;
    //double startangle;
    if( (p0.GetX() < p1.GetX()) ||
        (p0.GetX() == p1.GetX() && p0.GetY() < p1.GetY()) )
    {
      radius = p0.Distance( ep1 );
      angle = ( M_PI / 180 ) * p0.Direction( ep1 );
    }
    else {
      radius = p1.Distance( ep1 );
      angle = ( M_PI / 180 ) * p1.Direction( ep1 );
    }
    result.StartBulkLoad();
    Point v1(false, 0, 0), v2(false, 0, 0);

    for( int i = 0; i <= n; i++ )
    {

      if ( i == 0 )
      {
        // create the first halfsegment, parallel to the unit's direction
        hs.Set(true, ep4, ep1);
        hs.attr.faceno = 0;         // only one face
        hs.attr.cycleno = 0;        // only one cycle
        hs.attr.edgeno = partnerno;
        hs.attr.partnerno = partnerno++;
        hs.attr.insideAbove = false;
        result += hs;
        hs.SetLeftDomPoint( !hs.IsLeftDomPoint() );
        result += hs;

        // The startpoint of the first half-circle is ep1
        v1.Set(x1 ,y1);
        //angle = startangle + (i+1) * 2 * M_PI/n; // angle to starting vertex
        angle = angle + 2 * M_PI/n;
        valueX = xleft + radius * cos(angle);
        valueY = yleft + radius * sin(angle);
        v2.Set(valueX ,valueY);
      }
      else if (i == n/2-1)
      {
        // the end-point of the first half-circle is ep2
        //angle = startangle + i * 2 * M_PI/n ; // angle to starting vertex
        valueX = xleft + radius * cos(angle);
        valueY = yleft + radius * sin(angle);
        v1.Set(valueX ,valueY);
        v2.Set(x2, y2);
      }
      else if (i == n/2)
      {
        // create the second halfsegment, parallel to the unit's direction
        hs.Set(true, ep2, ep3);
        hs.attr.faceno = 0;         // only one face
        hs.attr.cycleno = 0;        // only one cycle
        hs.attr.edgeno = partnerno;
        hs.attr.partnerno = partnerno++;
        hs.attr.insideAbove = true;
        result += hs;
        hs.SetLeftDomPoint( !hs.IsLeftDomPoint() );
        result += hs;

        // the startpoint of the second half-circle is ep3
        // initialize the centerpoint for the second half-circle
        xleft = xright;
        yleft = yright;
        v1.Set(x3 ,y3);
        angle = angle + 2 * M_PI/n; // angle to starting vertex
        valueX = xleft + radius * cos(angle);
        valueY = yleft + radius * sin(angle);
        v2.Set(valueX ,valueY);
      }
      else if ( i == n )
      {
        // the endpoint of the second half-circle is ep4
        //angle = startangle + i * 2 * M_PI/n; // angle to starting vertex
        valueX = xleft + radius * cos(angle);
        valueY = yleft + radius * sin(angle);
        v1.Set(valueX ,valueY);
        v2.Set(x4, y4);
      }
      else
      {
        // The first point/vertex of the segment
        //angle = startangle + i * 2 * M_PI/n; // angle to starting vertex
        valueX = xleft + radius * cos(angle);
        valueY = yleft + radius * sin(angle);
        v1.Set(valueX ,valueY);

        // The second point/vertex of the segment
        //if ((i+1) >= n)            // angle to end vertex
        //  angle = startangle + 0 * 2 * M_PI/n;    // for inner vertex
        //else
        angle = angle + 2 * M_PI/n;
        valueX = xleft + radius * cos(angle);
        valueY = yleft + radius * sin(angle);
        v2.Set(valueX ,valueY);
      }

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
      Instant t0(timeInterval.start);
      Instant t1(timeInterval.end);

      double x = (p1.GetX() - p0.GetX()) * ((t - t0) / (t1 - t0)) + p0.GetX();
      double y = (p1.GetY() - p0.GetY()) * ((t - t0) / (t1 - t0)) + p0.GetY();

      result.Set( x, y );
      result.SetDefined(true);
    }
}


void CUPoint::AtInterval( const temporalalgebra::Interval<Instant>& i,
                           TemporalUnit<Point>& result ) const
{
  assert( IsDefined() );
  assert( i.IsValid() );


  TemporalUnit<Point>::AtInterval( i, result );
  temporalalgebra::UPoint *pResult = (temporalalgebra::UPoint*)&result;

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
  {
    // +++++ for debugging purposes only +++++
    //if( pResult->p1.IsDefined() )
    //  cout << "p1 ist definiert!\n";
    //else
    //  cout << "p1 ist NICHT definiert!\n";

    TemporalFunction( result.timeInterval.end, pResult->p1 );
  }

  pResult->SetDefined ( true );
}

bool CUPoint::D_Passes( const Point& p ) const
{
  assert( p.IsDefined() );
  assert( IsDefined() );

  if (epsilon > 0.0)
    return false;

  if( (timeInterval.lc && AlmostEqual( p, p0 )) ||
      (timeInterval.rc && AlmostEqual( p, p1 )) )
    return true;

  if( AlmostEqual( p0.GetX(), p1.GetX() ) &&
      AlmostEqual( p0.GetX(), p.GetX() ) )
    // If the segment is vertical
  {
    if( ( p0.GetY() <= p.GetY() && p1.GetY() >= p.GetY() ) ||
        ( p0.GetY() >= p.GetY() && p1.GetY() <= p.GetY() ) )
      return true;
  }
  else if( AlmostEqual( p0.GetY(), p1.GetY() ) &&
      AlmostEqual( p0.GetY(), p.GetY() ) )
    // If the segment is horizontal
  {
    if( ( p0.GetX() <= p.GetX() && p1.GetX() >= p.GetX() ) ||
        ( p0.GetX() >= p.GetX() && p1.GetX() <= p.GetX() ) )
      return true;
  }
  else
  {
    double k1 = ( p.GetX() - p0.GetX() ) / ( p.GetY() - p0.GetY() ),
           k2 = ( p1.GetX() - p0.GetX() ) / ( p1.GetY() - p0.GetY() );

    if( AlmostEqual( k1, k2 ) &&
        ( ( p0.GetX() < p.GetX() && p1.GetX() > p.GetX() ) ||
          ( p0.GetX() > p.GetX() && p1.GetX() < p.GetX() ) ) )
      return true;
  }
  return false;
}

bool CUPoint::D_Passes( const Region& r ) const
{
  assert( r.IsDefined() );
  assert( IsDefined() );

/*
1. If the cupoint's bbox and the region's bbox do not overlap, the result
is FALSE.

*/
  if( !r.BoundingBox().Intersects( this->BBox2D() ) )
    return false;

  bool containsP0 = false;
  bool containsP1 = false;
  bool distP0GreaterEpsilon = false;
  bool distP1GreaterEpsilon = false;
  bool cupIntersectsRgn = false;
  int i;
  double dist;
  HalfSegment segRgn;        // a halfsegment for iterating the region

/*
2. Determine, if one of the endpoints of the cupoint lies inside the region.

*/
  if (r.Contains( p0 ) )
  {
    containsP0 = true;
    distP0GreaterEpsilon = true;
  }

  if ( AlmostEqual(p0, p1) )
  {
    // there is just one point to prove.
    for(i = 0; i < r.Size(); i++)
    {
      r.Get( i, segRgn);
      dist = segRgn.Distance(p0);
      if (segRgn.IsLeftDomPoint() &&
          containsP0 && (dist < epsilon || AlmostEqual(dist, epsilon)) )
        // P0 is too close to this region's halfsegment
        distP0GreaterEpsilon = false;
    }
    if( distP0GreaterEpsilon )
      return true;
    return false;
  }

  if ( r.Contains( p1 ) )
  {
    containsP1 = true;
    distP1GreaterEpsilon = true;
  }

  Point defPP(false, 0, 0);

/*
Point defPP defines the point where the cupoint completely crosses the
regions border (The point on segCup which lies inside the region and its
distance to the regions border equals the epsilon-value.)

*/
  bool defPPtooClose = false;
  defPP.SetDefined(false);
  bool p0tooClose;
  bool p1tooClose;

/*
3. If one of the endpoints lies inside the region, determine if the distance of
this endpoint to the regions border is greater than epsilon.

*/
  HalfSegment segCup(true, p0, p1);
  if( p0 >= p1 )
    segCup.Set(false, p1, p0);
    // p0 is the dominating point of the halfsegment

  //r.StartBulkLoad();
  int lastDefPPhs;
/*
The Variable lastDefPPhs stores the index to the last halfsegment of the
region to which a definite passing Point was computed. This is to ensure that
the distance between a later defined defPP and this halfsegment can be proved
again.

*/
  bool lastDefPPhsIsDefined = false;
  int hSegsTooClose[r.Size()];
                  // stores the indices of halfsegments of the region whose
                  // distance to the cupoint is less than epsilon
  int noSegsTooClose = 0;

  for(i = 0; i < r.Size(); i++)
  {
    r.Get(i, segRgn );

    if( segRgn.IsLeftDomPoint() )
    {
      // +++++ for debugging purposes only +++++
      //Coord lpx = segRgn->GetLeftPoint().GetX();
      //Coord lpy = segRgn->GetLeftPoint().GetY();
      //Coord rpx = segRgn->GetRightPoint().GetX();
      //Coord rpy = segRgn->GetRightPoint().GetY();
      //cout << "segRgn is defined by the edgepoints " << lpx << " " << lpy
      //  << "     " << rpx << " " << rpy << endl;

      p0tooClose = false;
      p1tooClose = false;
      defPPtooClose = false;
      cupIntersectsRgn = segCup.Intersects(segRgn);

      dist = segRgn.Distance(p0);
      if (containsP0 && dist < epsilon && !AlmostEqual(dist, epsilon) )
      {
        // P0 is too close to this region's halfsegment

        // +++++ for debugging purposes only +++++
        //cout << "Distance between segRgn and P0: " << dist << endl;

        distP0GreaterEpsilon = false;  // this variable will stay false
        p0tooClose = true;   // this variable will be reset on every turn
      }

      dist = segRgn.Distance(p1);
      if (containsP1 && dist < epsilon && !AlmostEqual(dist, epsilon) )
      {
        // P0 is too close to this region's halfsegment

        // +++++ for debugging purposes only +++++
        //cout <<"Distance between segRgn and P1: " << segRgn->Distance(p1)
        //  << endl;

        distP1GreaterEpsilon = false;
        p1tooClose = true;
      }

      if( !cupIntersectsRgn && !p0tooClose && ! p1tooClose )
      {
        dist = segCup.Distance(segRgn);
        if( dist < epsilon && !AlmostEqual(dist, epsilon) )
        {
          hSegsTooClose[noSegsTooClose] = i;
          noSegsTooClose++;

          // +++++ for debugging purposes only +++++
          //cout << "A halfsegment has been added to 'hSegsTooClose[]'.\n";
          //cout << "hSegsTooClose contains " << noSegsTooClose << " elements."
          //    << endl;
        }
      }


      if( defPP.IsDefined() )
      {
        dist = segRgn.Distance(defPP);
        defPPtooClose = ( dist < epsilon && !AlmostEqual(dist, epsilon) );

        // +++++ for debugging purposes only +++++
        //if (defPPtooClose) {
        //  cout << "segRgn->Distance(defPP) is less than epsilon!\n";
        //  cout << "Distance to defPP : " << segRgn->Distance(defPP) << endl;
        //}
      }

      if( ((containsP0 && p0tooClose) || (containsP1 && p1tooClose) ||
          ((cupIntersectsRgn) && !defPP.IsDefined())) || defPPtooClose )
      {
        // +++++ for debugging purposes only +++++
        //if(containsP0 && p0tooClose)
        //    cout << "containsP0 && p0tooClose is TRUE!\n";
        //if(containsP1 && p1tooClose)
        //    cout << "containsP1 && p1tooClose is TRUE!\n";
        //if(cupIntersectsRgn)
        //    cout << "cupIntersectsRgn is TRUE!\n";
        //if(defPPtooClose)
        //    cout << "defPPtooClose is TRUE!\n";

/*
If one of the endpoints lies inside the region and the distance to the region's
border is less than epsilon, or if the cupoint intersects the region.

*/
        if( FindDefPassingPoint(segCup, segRgn, epsilon, defPP) )
        {
          // +++++ for debugging purposes only +++++
          //cout << "FindDefPassingPoint returns true.\n";

          defPPtooClose = false;
          if( lastDefPPhsIsDefined )
          {
            // A defPP was previously defined, so for this new defPP, the
            // distance to halfsegment i has to be compared to epsilon
            r.Get(lastDefPPhs, segRgn);
            dist = segRgn.Distance(defPP);
            if( dist < epsilon && !AlmostEqual(dist, epsilon) )
            {
              // +++++ for debugging purposes only +++++
              //cout << "defPP has distance: " << lastDefPPhs->Distance(defPP)
              //  << " to segment " << lastDefPPhs->GetLeftPoint().GetX()
              //  << " " << lastDefPPhs->GetLeftPoint().GetY() << "     "
              //  << lastDefPPhs->GetRightPoint().GetX() << " "
              //  << lastDefPPhs->GetRightPoint().GetY() << endl;

              // The determined defPP is too close to a previous mentioned hs!
              defPPtooClose = true;
              defPP.SetDefined(false);
              lastDefPPhsIsDefined = false;
            }
            else {
              lastDefPPhs = i; // hold a pointer to the region's hs
              lastDefPPhsIsDefined = true;
            }
          }
          else
          {
            lastDefPPhs = i; // save the index of the region's halfsegment
            lastDefPPhsIsDefined = true;
          }

          // +++++ for debugging purposes only +++++
          //if(defPP.IsDefined())
          //  cout << "defPP is defined\n";
        }
        else {
          // +++++ for debugging purposes only +++++
          //cout << "defPP is set to UNDEFINED!\n";

          defPP.SetDefined(false);
        }
      }
    }
  }
  //r.EndBulkLoad();
  if( distP0GreaterEpsilon || distP1GreaterEpsilon )
  {
/*
One of the endpoints lies inside the region, and its distance to the region's
border is greater than epsilon, so the predicate 'definitely-passes' is
fullfilled.

*/
    // +++++ for debugging purposes only +++
    //if (distP0GreaterEpsilon)
    //  cout << "D_Passes: P0 liegt mit Abstand Epsilon in Region!\n";
    //if (distP1GreaterEpsilon)
    //  cout << "D_Passes: P1 liegt mit Abstand Epsilon in Region!\n";

    return true;
  }
  if( defPP.IsDefined() )
  {

    // +++++ for debugging purposes only +++++
    //cout << "D_Passes: es existiert ein defPassingPoint in Region!\n";
    //cout << "defPP = " << defPP.GetX() << " " << defPP.GetY() << "\n";

    if( noSegsTooClose > 0 )
    {
      // determine if the distance of the defined point defPP is less than
      // epsilon. If so, return false
      for(int j = 0; j < noSegsTooClose; j++)
      {
        // +++++ for debugging purposes only +++++
        //cout << "Distance between hSegsTooClose[" << j+1 << "] = "
        //    << hSegsTooClose[j].Distance( defPP ) << endl;

        r.Get(hSegsTooClose[j], segRgn);
        dist = segRgn.Distance( defPP );
        if( dist < epsilon && !AlmostEqual(dist, epsilon) )
        {
          // +++++ for debugging purposes only +++++
          //cout << "The final defPP is too close to a hs in hSegsTooClose[]!"
          //    << endl;

          return false;
        }
      }
    }
    return true;
  }
  return false;
}

bool CUPoint::P_Passes( const Point& p ) const
{
  assert( p.IsDefined() );
  assert( IsDefined() );

/*
If the uncertainty-value equals 0, the possibly\_passes function returns
the same as the definitely\_passes function.

*/
  if( epsilon == 0.0 )
    return D_Passes( p );

  if( AlmostEqual( p0, p1) )
  {
    double dist = p0.Distance( p );
    if( dist < epsilon || AlmostEqual(dist, epsilon) )
      return true;
    else
      return false;
  }
  else
  {
    HalfSegment hs(false, p0, p1);
    if( (p0.GetX() < p1.GetX()) ||
        ( (p0.GetX() == p1.GetX()) && (p0.GetY() < p1.GetY()) ) )
      hs.Set(true, p0, p1);
    double dist = hs.Distance(p);
    if(dist < epsilon || AlmostEqual(dist, epsilon) )
      return true;
    else
      return false;
  }
}

bool CUPoint::P_Passes( const Region& r ) const
{
  assert( r.IsDefined() );
  assert( IsDefined() );

/*
1. If the cupoint's bbox and the region's bbox do not overlap, the result
is FALSE.

*/
  if( !r.BoundingBox().Intersects( this->BBox2D() ) )
    return false;

  if( r.Contains(p0) )
    return true;

  if( AlmostEqual(p0, p1) )
  {
    double dist = r.Distance( p0 );
    if( dist < epsilon || AlmostEqual(dist, epsilon) )
      return true;
    else
      return false;
  }
  else
  {
    if( r.Contains(p1) )
      return true;

/*
Else (if the Bounding-Boxes overlap) determine, if the distance of the
halfsegment segCup (defined by the unit's endpoints) to the region is less than
the uncertainty-value.

*/
    HalfSegment segCup(false, p0, p1);

    if( (p0.GetX() < p1.GetX()) ||
        ( (p0.GetX() == p1.GetX()) && (p0.GetY() < p1.GetY()) ) )
      segCup.Set(true, p0, p1);

    HalfSegment segRgn;
    int i = 0;

    while( i < r.Size() )
    {
      r.Get( i, segRgn);

      if( segRgn.Distance( segCup ) <= epsilon ||
          segCup.Distance( segRgn ) <= epsilon )
        return true;
      i++;
    }
  }
  return false;
}

void CUPoint::AtInstant( const Instant& t, 
                         temporalalgebra::Intime<Region>& result ) const
{
  assert( t.IsDefined() );
  if( IsDefined() && t.IsDefined() )
  {
    // compute result
    double instd = t.ToDouble();
    double mind = timeInterval.start.ToDouble();
    double maxd = timeInterval.end.ToDouble();
    if( (mind > instd && !AlmostEqual(mind,instd)) ||
         (maxd < instd && !AlmostEqual(maxd,instd))
      )
    {
      result.SetDefined(false);
    }
    else
    {
      result.SetDefined( true );
      Point respoint(false, 0, 0);
      TemporalFunction( t, respoint );

      double e = GetEpsilon();
      if( e == 0.0 )
      {
        e = fabs(p0.GetX());
        if( fabs(p1.GetX()) > e )
          e = fabs(p1.GetX());
        if( fabs(p0.GetY()) > e )
          e = fabs(p0.GetY());
        if( fabs(p1.GetY()) > e )
          e = fabs(p1.GetY());
        e = e * minEpsilonLimiter;
      }
      Circle(respoint, e, 16, result.value);
      result.instant.CopyFrom( &t );
    }
  }
  else
  {
    result.SetDefined(false);
  }
}
/*

*/
void CUPoint::D_At( const Point& p, CUPoint& result ) const
{
  assert( p.IsDefined() );
  assert( IsDefined() );

  if (epsilon == 0.0)
  {
    CUPoint *pResult = &result;

    if( AlmostEqual( p0, p1 ) )
    {
      if( AlmostEqual( p, p0 ) )
        *pResult = *this;

      else
        pResult->SetDefined(false);
    }
    else if( AlmostEqual( p, p0 ) )
    {
      if( timeInterval.lc )
      {
        temporalalgebra::Interval<Instant> interval( timeInterval.start,
         timeInterval.start, true, true );
        CUPoint unit(epsilon, interval, p, p );
        *pResult = unit;
        //delete unit;
      }
      else pResult->SetDefined( false );
    }
    else if( AlmostEqual( p, p1 ) )
    {
      if( timeInterval.rc )
      {
        temporalalgebra::Interval<Instant> interval( timeInterval.end,
         timeInterval.end, true, true );
        CUPoint unit(epsilon, interval, p, p );
        *pResult = unit;
        //delete unit;
      }
      else pResult->SetDefined( false );
    }
    else if( AlmostEqual( p0.GetX(), p1.GetX() ) &&
             AlmostEqual( p0.GetX(), p.GetX() ) )
      // If the segment is vertical
    {
      if( ( p0.GetY() <= p.GetY() && p1.GetY() >= p.GetY() ) ||
          ( p0.GetY() >= p.GetY() && p1.GetY() <= p.GetY() ) )
      {
        Instant t( timeInterval.start +
                   ( ( timeInterval.end - timeInterval.start ) *
                   ( ( p.GetY() - p0.GetY() ) / ( p1.GetY() - p0.GetY() ) ) ));
        temporalalgebra::Interval<Instant> interval( t, t, true, true );
        CUPoint unit(epsilon, interval, p, p );
        *pResult = unit;
        //delete unit;
      }
      else pResult->SetDefined( false );
    }
    else if( AlmostEqual( p0.GetY(), p1.GetY() ) &&
        AlmostEqual( p0.GetY(), p.GetY() ) )
      // If the segment is horizontal
    {
      if( ( p0.GetX() <= p.GetX() && p1.GetX() >= p.GetX() ) ||
          ( p0.GetX() >= p.GetX() && p1.GetX() <= p.GetX() ) )
      {
        Instant t( timeInterval.start +
                   ( ( timeInterval.end - timeInterval.start ) *
                   ( ( p.GetX() - p0.GetX() ) / ( p1.GetX() - p0.GetX() ) ) ));
        temporalalgebra::Interval<Instant> interval( t, t, true, true );
        CUPoint unit(epsilon, interval, p, p );
        *pResult = unit;
        //delete unit;
      }
      else pResult->SetDefined( false );
    }
    else
    {
      double k1 = ( p.GetX() - p0.GetX() ) / ( p.GetY() - p0.GetY() ),
             k2 = ( p1.GetX() - p0.GetX() ) / ( p1.GetY() - p0.GetY() );

      if( AlmostEqual( k1, k2 ) &&
          ( ( p0.GetX() <= p.GetX() && p1.GetX() >= p.GetX() ) ||
            ( p0.GetX() >= p.GetX() && p1.GetX() <= p.GetX() ) ) )
      {
        Instant t( timeInterval.start +
                   ( ( timeInterval.end - timeInterval.start ) *
                   ( ( p.GetX() - p0.GetX() ) / ( p1.GetX() - p0.GetX() ) ) ));
        temporalalgebra::Interval<Instant> interval( t, t, true, true );
        CUPoint unit(epsilon, interval, p, p );
        *pResult = unit;
        //delete unit;
      }
      else pResult->SetDefined( false );
    }
  }
  else result.SetDefined(false);
}


void CUPoint::D_At( const Region& r, CMPoint& result ) const
{
  assert( r.IsDefined() );
  assert( IsDefined() );

  result.Clear();

/*
1. If the cupoint's bbox and the region's bbox do not overlap, the result is
empty (or undefined).

*/
  if( !r.BoundingBox().Intersects( this->BBox2D() ) )
    return;

  //CUPoint *pResult;
  bool containsP0 = false;
  bool containsP1 = false;
  bool distP0GreaterEpsilon = false;
  bool distP1GreaterEpsilon = false;
  bool cupIntersectsRgn = false;
  int i;
  HalfSegment segRgn;        // a halfsegment for iterating the region
  double dist;
/*
2. Determine, if one of the endpoints of the cupoint lies inside the region.

*/
  if (r.Contains( p0 ) )
  {
    containsP0 = true;
    distP0GreaterEpsilon = true;
  }

  if ( AlmostEqual(p0, p1) )
  {
    // there is just one point to prove.
    for(i = 0; i < r.Size(); i++)
    {
      r.Get( i, segRgn);
      dist = segRgn.Distance(p0);
      if (segRgn.IsLeftDomPoint() &&
          containsP0 && (dist < epsilon) && !AlmostEqual(dist, epsilon) )
        // P0 is too close to this region's halfsegment
        distP0GreaterEpsilon = false;
    }
    if( distP0GreaterEpsilon )
    {
      // the constant unit lies inside the region
      result.Add(*this);
    }
  }
  else
  {
    if ( r.Contains( p1 ) )
    {
      containsP1 = true;
      distP1GreaterEpsilon = true;
    }
/*
To compute various Passing Points, a spatial representation of the unit is
needed (the epsilon-value is used separately and so is ignored here).

*/
    bool p0tooClose;
    bool p1tooClose;
    bool hstooClose;
    bool aDefPPtooClose = false;
    const int maxDefPPs = 16;
    int definedDefPPs = 0;
    int aktDefPPs = 0;
    int k;
    int l;
    Point defPP[maxDefPPs];
/*
The array defPP stores points, where the cupoint completely crosses the regions
border (the points on segCup that lie inside the region and their distance to
the regions border equals the epsilon-value). In most cases there will be only
1 or 2 Passing Points, but if there is a long CUPoint lying upon the border of
a very rugged region, there may be more than just 2 Passing Points.

*/
    bool defPPtooClose[maxDefPPs];
    int lastDefPPhs[maxDefPPs];
/*
The Variable lastDefPPhs[] is a pointer-array to the last halfsegments of the
region to which a definite passing Point was computed. This is to ensure that
the distance between a later defined defPP and this halfsegment can be proved
again.

*/
    for (k = 0; k < maxDefPPs; k++)
    {
      defPP[k].SetDefined(false);
      defPPtooClose[k] = false;
      lastDefPPhs[k] = -1;
    }

    HalfSegment segCup(false, p1, p0);
    if( p0 < p1 )
      segCup.Set(true, p0, p1);
      // the dominating point of the halfsegment marks p0

    //r.StartBulkLoad();

    int hSegsTooClose[32]; // stores the halfsegments of the region whose
                           // distance to the cupoint is less than epsilon
    int noSegsTooClose = 0;
/*
3. If one of the endpoints lies inside the region, determine if the distance of
this endpoint to the regions border is greater than epsilon. If so, this
endpoint is part of the result-unit(s). If one (or both) of the endpoints lie
outside the region, or the distance to the region's border is less than the
uncertainty-value, determine one or more Passing Points as described above.

*/
    for(i = 0; i < r.Size(); i++)
    {
      r.Get( i, segRgn );

      if( segRgn.IsLeftDomPoint() )
      {
        // As every Segment is represented by 2 HalfSegments, just one of them
        // has to be proved.

        // +++++ for debugging purposes only +++++
        //Coord lpx = segRgn->GetLeftPoint().GetX();
        //Coord lpy = segRgn->GetLeftPoint().GetY();
        //Coord rpx = segRgn->GetRightPoint().GetX();
        //Coord rpy = segRgn->GetRightPoint().GetY();
        //cout << "segRgn is defined by the edgepoints " << lpx << " " << lpy
        //  << "     " << rpx << " " << rpy << endl;

        p0tooClose = false;
        p1tooClose = false;
        for(k = 0; k < aktDefPPs; k++)
          defPPtooClose[k] = false;

        // Determine reasons to compute a definite Passing Point:

        cupIntersectsRgn = segCup.Intersects(segRgn);

        if (containsP0 && (segRgn.Distance(p0) <= epsilon) )
        {
          // P0 is too close to this region's halfsegment

          // +++++ for debugging purposes only +++++
          //cout << "Distance between segRgn and P0: " << segRgn->Distance(p0)
          //  << endl;

          distP0GreaterEpsilon = false;  // this variable will stay false
          p0tooClose = true;   // this variable will be reset on every turn
        }

        if (containsP1 && (segRgn.Distance(p1) <= epsilon) )
        {
          // P0 is too close to this region's halfsegment

          // +++++ for debugging purposes only +++++
          //cout <<"Distance between segRgn and P1: " << segRgn->Distance(p1)
          //  << endl;

          distP1GreaterEpsilon = false;
          p1tooClose = true;
        }

        for(k = 0; k < aktDefPPs; k++)
        {
/*
Determine if the distance of a previously defined defPP to the actual region's
halfsegment is less than epsilon. If so, set its defPPtooClose[k]-flag to
TRUE, mark it as 'undefined' and decrease the counter 'definedDefPPs'.

*/
          if( defPP[k].IsDefined() )
          {
            dist = segRgn.Distance(defPP[k]);
            defPPtooClose[k] = (dist < epsilon && !AlmostEqual(dist, epsilon));
            if( defPPtooClose[k] )
            {
              defPP[k].SetDefined(false);
              aDefPPtooClose = true;
              definedDefPPs--;

              // +++++ for debugging purposes only +++++
              //cout << "segRgn->Distance(defPP" << k << ") less than epsilon!"
              //     << endl;
              //cout << "Distance to defPP[" << k << "]: "
              //    << segRgn->Distance(defPP[k]) << endl;
              //cout << "There are actually " << definedDefPPs << " defPPs "
              //    << "defined!\n";
            }
          }
        }


/*
Determine if the distance of the actual halfsegment to the unit is less than
epsilon. If so, store its index in the array hSegsTooClose[].

*/
        dist = segCup.Distance( segRgn );
        hstooClose = ( dist < epsilon && !AlmostEqual(dist, epsilon) );
        if( hstooClose )
        {
          if(noSegsTooClose < 32)
          {
            hSegsTooClose[noSegsTooClose] = i;
            noSegsTooClose++;
          }
/*
For it is extremely unlikely that more than 32 of the region's halfsegments lie
too close to the unit, the array-size hSegsTooClose is set to this value. If
such a rare case should happen, the Operation exits with no result and returns
the following error message:

*/
          else {
            cout << "Error in CUPoint::D_At(Region&, CMPoint&): \n";
            cout << "There are more than 32 HalfSegments in the region whose"
                 << " distance to the unit is less than epsilon!"
                 << "The result is not computable by this algorithm!";
            cerr << "Error in CUPoint::D_At(Region&, CMPoint&): \n";
            cerr << "There are more than 32 HalfSegments in the region whose"
                 << " distance to the unit is less than epsilon!"
                 << "The result is not computable by this algorithm!";
            result.SetDefined(false);
            return;
          }
        }

        if( hstooClose || (containsP0 && p0tooClose) ||
            (containsP1 && p1tooClose) || cupIntersectsRgn || aDefPPtooClose )
        {
          // +++++ for debugging purposes only +++++
          //if(hstooClose)
          //    cout << "hstooClose is TRUE!\n";
          //if(containsP0 && p0tooClose)
          //    cout << "containsP0 && p0tooClose is TRUE!\n";
          //if(containsP1 && p1tooClose)
          //    cout << "containsP1 && p1tooClose is TRUE!\n";
          //if(cupIntersectsRgn)
          //    cout << "cupIntersectsRgn is TRUE!\n";
          //if(aDefPPtooClose)
          //    cout << "aDefPPtooClose is TRUE!\n";

          Point p(false, 0, 0);
          if( FindDefPassingPoint(segCup, segRgn, epsilon, p) )
          {
            // +++++ for debugging purposes only +++++
            //cout << "FindDefPassingPoint returns true.\n";

            for(k = 0; k < aktDefPPs; k++)
            {
              if( defPP[k].IsDefined() )
              {
                if( AlmostEqual(p.GetX(), defPP[k].GetX()) &&
                    AlmostEqual(p.GetY(), defPP[k].GetY()) )
                  p.SetDefined(false);
              }
              if( defPPtooClose[k] && lastDefPPhs[k] > -1 )
              {
/*
If a defPP was previously defined, the distance for this new defPP to the
halfsegment indexed by lastDefPPhs[k] has to be compared to epsilon.

*/
                r.Get( lastDefPPhs[k], segRgn );
                dist = segRgn.Distance(p);
                if( dist < epsilon && !AlmostEqual(dist, epsilon) )
                  p.SetDefined(false);
                lastDefPPhs[k] = -1;
              }
            }
            aDefPPtooClose = false;

            if( p.IsDefined() )
            {
              k = 0;
              while( defPP[k].IsDefined() )
                k++;
              if( k == aktDefPPs )
              {
                if( aktDefPPs < maxDefPPs )
                  aktDefPPs++;
/*
For it is extremely unlikely to get more than 16 defPPs, the array-size is
set to this value. If such a rare case should happen, the Operation exits with
no result and returns the following error message:

*/

                else {
                  cout << "Error in CUPoint::D_At(Region&, CMPoint&): \n";
                  cout << "There are more than 16 Passing Points defined for "
                    << "this unit this leads to an internal array-overflow!"
                    << "The result is not computable by this algorithm!";
                  cerr << "Error in CUPoint::D_At(Region&, CMPoint&): \n";
                  cerr << "There are more than 16 Passing Points defined for "
                    << "this unit this leads to an internal array-overflow!"
                    << "The result is not computable by this algorithm!";
                  result.SetDefined(false);
                  return;
                }
              }

              defPP[k] = p;
              defPPtooClose[k] = false;
              lastDefPPhs[k] = i;   // set lastDefPPhs[l] to the index of
                                    // the actual region
              definedDefPPs++;

              // +++++ for debugging purposes only +++++
              //cout << "The number of defined Passing Points was "
              //  << "INcreased to " << definedDefPPs << "! \n";
              //cout << "A new defPP is defined. Now " << definedDefPPs
              //  << endl;
            }
          }
        }
      }
    }
    //r.EndBulkLoad();

    // Build new CUPoint-Objects from the determined points:

/*
If the whole Unit lies inside the region, it can simply be added to the result.

*/
    if( distP0GreaterEpsilon && distP1GreaterEpsilon && aktDefPPs == 0 )
    {
      result.Add( *this );
    }
    else
    {
      // Eliminate all defPP's that lie too close to one of the HalfSegments
      // stored in the hSegsTooClose-Array.

      if( noSegsTooClose > 0 )
      {
        for( k = 0; k < aktDefPPs; k++ )
        {
          if( defPP[k].IsDefined() )
          {
            for(int j = 0; j < noSegsTooClose; j++)
            {
              r.Get( hSegsTooClose[j], segRgn );
              dist = segRgn.Distance( defPP[k] );
              if( dist < epsilon && !AlmostEqual( dist, epsilon ) )
              {
                defPP[k].SetDefined(false);
                definedDefPPs--;

                // +++++ for debugging purposes only +++++
                //cout << "The number of defined Passing Points was "
                //  << "DEcreased to " << definedDefPPs << "! \n";
                //cout << "Distance to defPP[" << k << "]: "
                //  << segRgn->Distance(defPP[k]) << endl;
              }
            }
          }
        }
      }

/*
Determine two points from the remaining defPPs for a new unit. Include the
endpoints in this evaluation if they lie inside the region and their distance
to the region's border is greater than epsilon.

*/
      // +++++ for debugging purposes only +++++
      //int countPoints = definedDefPPs + distP0GreaterEpsilon
      //  + distP1GreaterEpsilon;
      //cout << endl << "There are " << countPoints << " DefPPs left!\n\n";


      Point ep0(false, 0, 0);
      Point ep1(false, 0, 0);
      bool firstrun = distP0GreaterEpsilon;

      // +++++ for debugging purposes only +++++
      //cout << "Begin building new CUPoints...\n";

      while( definedDefPPs > 0 )
      {
        // +++++ for debugging purposes only +++++
        //cout << "There are actually  " << definedDefPPs << "defPPs! \n";

        if( p0 < p1 )
        {
          // +++++ for debugging purposes only +++++
          //cout << "p0 < p1\n";

          if( firstrun )
          {
            ep0 = p0;
            ep1.SetDefined(false);
            for( k = 0; k < aktDefPPs; k++ )
            {
              if( defPP[k].IsDefined() )
              {
                if( !ep1.IsDefined() )
                {
                  ep1 = defPP[k];
                  l = k;
                }
                else if( defPP[k] < ep1 )
                {
                  ep1 = defPP[k];
                  l = k;
                }
              }
            }
            defPP[l].SetDefined(false);
            definedDefPPs--;
            firstrun = false;
          }
          else if( definedDefPPs < 2 && distP1GreaterEpsilon )
          {
            ep0.SetDefined(false);
            ep1 = p1;
            for( k = 0; k < aktDefPPs; k++ )
            {
              if( defPP[k].IsDefined() )
              {
                if( !ep0.IsDefined() )
                {
                  ep0 = defPP[k];
                  l = k;
                }
                else if( defPP[k] < ep0 )
                {
                  ep0 = defPP[k];
                  l = k;
                }
              }
            }
            defPP[l].SetDefined(false);
            definedDefPPs--;
          }
          else {
            ep0.SetDefined(false);
            ep1.SetDefined(false);
            for( k = 0; k < aktDefPPs; k++ )
            {
              if( defPP[k].IsDefined() )
              {
                // +++++ for debugging purposes only +++++
                //cout << "defPP[" << k << "] is defined and represents point "
                //  << defPP[k].GetX() << " " << defPP[k].GetY() << endl;

                if( !ep0.IsDefined() )
                {
                  ep0 = defPP[k];
                  l = k;
                }
                else if( defPP[k] < ep0 )
                {
                  ep0 = defPP[k];
                  l = k;
                }
              }
            }
            // +++++ for debugging purposes only +++++
            //cout << "ep0 is the point at array-position " << l << "\n";

            defPP[l].SetDefined(false);

            for( k = 0; k < aktDefPPs; k++ )
            {
              if( defPP[k].IsDefined() )
              {
                if( !ep1.IsDefined() )
                {
                  ep1 = defPP[k];
                  l = k;
                }
                else if( defPP[k] < ep1 )
                {
                  ep1 = defPP[k];
                  l = k;
                }
              }
            }

            // +++++ for debugging purposes only +++++
            //cout << "ep1 is the point at array-position " << l << "\n";

            defPP[l].SetDefined(false);
            definedDefPPs = definedDefPPs - 2;
          }
        }
        else
        {
          // ( p0 > p1 )
          // +++++ for debugging purposes only +++++
          //cout << "p0 > p1\n";

          if( firstrun )
          {
            ep0 = p0;
            ep1.SetDefined(false);
            for( k = 0; k < aktDefPPs; k++ )
            {
              if( defPP[k].IsDefined() )
              {
                if( !ep1.IsDefined() )
                {
                  ep1 = defPP[k];
                  l = k;
                }
                else if( defPP[k] > ep1 )
                {
                  ep1 = defPP[k];
                  l = k;
                }
              }
            }
            defPP[l].SetDefined(false);
            definedDefPPs--;
            firstrun = false;
          }
          else if( definedDefPPs < 2 && distP1GreaterEpsilon )
          {
            ep0.SetDefined(false);
            ep1 = p1;
            for( k = 0; k < aktDefPPs; k++ )
            {
              if( defPP[k].IsDefined() )
              {
                if( !ep0.IsDefined() )
                {
                  ep0 = defPP[k];
                  l = k;
                }
                else if( defPP[k] > ep0 )
                {
                  ep0 = defPP[k];
                  l = k;
                }
              }
            }
            defPP[l].SetDefined(false);
            definedDefPPs--;
          }
          else
          {
            ep0.SetDefined(false);
            ep1.SetDefined(false);
            for( k = 0; k < aktDefPPs; k++ )
            {
              if( defPP[k].IsDefined() )
              {
                // +++++ for debugging purposes only +++++
                //cout << "defPP[" << k << "] is defined and represents point "
                //  << defPP[k].GetX() << " " << defPP[k].GetY() << endl;

                if( !ep0.IsDefined() )
                {
                  ep0 = defPP[k];
                  l = k;
                }
                else if( defPP[k] > ep0 )
                {
                  ep0 = defPP[k];
                  l = k;
                }
              }
            }
            //cout << "ep0 is the point at array-position " << l << "\n";
            defPP[l].SetDefined(false);
            for( k = 0; k < aktDefPPs; k++ )
            {
              if( defPP[k].IsDefined() )
              {
                // +++++ for debugging purposes only +++++
                //cout << "defPP[" << k << "] is defined and represents point "
                //  << defPP[k].GetX() << " " << defPP[k].GetY() << endl;

                if( !ep1.IsDefined() )
                {
                  ep1 = defPP[k];
                  l = k;
                }
                else if( defPP[k] > ep1 )
                {
                  ep1 = defPP[k];
                  l = k;
                }
              }
            }

            // +++++ for debugging purposes only +++++
            //cout << "ep1 is the point at array-position " << l << "\n";

            defPP[l].SetDefined(false);
            definedDefPPs = definedDefPPs - 2;
          }
        }

        // +++++ for debugging purposes only +++
        //cout << "D_Passes: P0 liegt mit Abstand Epsilon in Region!\n";

/*
Determine the timeInterval for the new unit:

*/
        Instant t0(instanttype);
        Instant t1(instanttype);
        bool tlc;
        bool trc;

        // +++++ for debugging purposes only +++++
        //cout << "Value of definedDefPPs: " << definedDefPPs << endl;
        //if( !ep0.IsDefined() ) {
        //  cout << "Point ep0 is NOT defined!\n";
        //  cout << "Values of ep0: " << ep0.GetX() << " " << ep0.GetY()
        //      << endl;
        //}
        //else {
        //  cout << "Point ep0 is defined!\n";
        //  cout << "Values of ep0: " << ep0.GetX() << " " << ep0.GetY()
        //      << endl;
        //}
        //if( !ep1.IsDefined() ) {
        //  cout << "Point ep1 is NOT defined!\n";
        //  cout << "Values of ep1: " << ep1.GetX() << " " << ep1.GetY()
        //      << endl;
        //}
        //else {
        //  cout << "Point ep1 is defined!\n";
        //  cout << "Values of ep1: " << ep1.GetX() << " " << ep1.GetY()
        //      << endl;
        //}


        if( AlmostEqual( p0.GetX(), p1.GetX() ) &&
            AlmostEqual( p0.GetX(), ep0.GetX() ) &&
            AlmostEqual( p0.GetX(), ep1.GetX() ) )
            // If the segment is vertical
        {
          if( (( p0.GetY() <= ep0.GetY() && p1.GetY() >= ep0.GetY() ) ||
              ( p0.GetY() >= ep0.GetY() && p1.GetY() <= ep0.GetY() )) &&
              (( p0.GetY() <= ep1.GetY() && p1.GetY() >= ep1.GetY() ) ||
              ( p0.GetY() >= ep1.GetY() && p1.GetY() <= ep1.GetY() )) )
          {
            if( ep0 == p0 )
            {
              t0 = timeInterval.start;
              tlc = timeInterval.lc;
            }
            else {
              t0 = timeInterval.start +
                    (timeInterval.end - timeInterval.start) *
                    ( (ep0.GetY() - p0.GetY()) / (p1.GetY() - p0.GetY()) );
              tlc = true;
            }
            if( ep1 == p1 )
            {
              t1 = timeInterval.end;
              trc = timeInterval.rc;
            }
            else {
              t1 = timeInterval.start +
                    (timeInterval.end - timeInterval.start) *
                    ( (ep1.GetY() - p0.GetY()) / (p1.GetY() - p0.GetY()) );
              trc = true;
            }
            temporalalgebra::Interval<Instant> interval( t0, t1, tlc, trc );
            CUPoint unit( epsilon, interval, ep0, ep1 );
            result.Add( unit );
          }
        }
        else if( AlmostEqual( p0.GetY(), p1.GetY() ) &&
                AlmostEqual( p0.GetY(), ep0.GetY() ) &&
                AlmostEqual( p0.GetY(), ep1.GetY() ) )
                // The segment is horizontal
        {
          if( (( p0.GetX() <= ep0.GetX() && p1.GetX() >= ep0.GetX() ) ||
              ( p0.GetX() >= ep0.GetX() && p1.GetX() <= ep0.GetX() )) &&
              (( p0.GetX() <= ep1.GetX() && p1.GetX() >= ep1.GetX() ) ||
              ( p0.GetX() >= ep1.GetX() && p1.GetX() <= ep1.GetX() )) )
          {
            if( ep0 == p0 )
            {
              t0 = timeInterval.start;
              tlc = timeInterval.lc;
            }
            else {
              t0 = timeInterval.start +
                    (timeInterval.end - timeInterval.start) *
                    ( (ep0.GetX() - p0.GetX()) / (p1.GetX() - p0.GetX()) );
              tlc = true;
            }
            if( ep1 == p1 )
            {
              t1 = timeInterval.end;
              trc = timeInterval.rc;
            }
            else {
              t1 = timeInterval.start +
                    (timeInterval.end - timeInterval.start) *
                    ( (ep1.GetX() - p0.GetX()) / (p1.GetX() - p0.GetX()) );
              trc = true;
            }
            temporalalgebra::Interval<Instant> interval( t0, t1, tlc, trc );
            CUPoint unit( epsilon, interval, ep0, ep1 );
            result.Add( unit );
          }
        }
        else
        {
          // +++++ for debugging purposes only +++++
          //cout << "Determine, if ep0 and ep1 are points of the unit... \n";

          double k0;
          if(ep0 != p0)
            k0 = (ep0.GetX() - p0.GetX()) / (ep0.GetY() - p0.GetY());
          double k1 = (ep1.GetX() - p0.GetX()) / (ep1.GetY() - p0.GetY());
          double k2 = (p1.GetX() - p0.GetX()) / (p1.GetY() - p0.GetY());

          // +++++ for debugging purposes only +++++
          //cout << "k0 = " << k0 << "   k1 = " << k1 << "   k2 = " << k2
          //  << endl;

          if( (ep0 == p0 || AlmostEqual( k0, k2 ) ) && AlmostEqual( k1, k2 ) &&
              ( (( p0.GetX() <= ep0.GetX() && p1.GetX() >= ep0.GetX() ) ||
              ( p0.GetX() >= ep0.GetX() && p1.GetX() <= ep0.GetX() )) &&
              (( p0.GetX() <= ep1.GetX() && p1.GetX() >= ep1.GetX() ) ||
              ( p0.GetX() >= ep1.GetX() && p1.GetX() <= ep1.GetX() )) ) )
          {
            if( ep0 == p0 )
            {
              // +++++ for debugging purposes only +++++
              //cout << "ep0 equals p0!\n";

              t0 = timeInterval.start;
              tlc = timeInterval.lc;
            }
            else {
              // +++++ for debugging purposes only +++++
              //cout << "ep0 is a Point within the Unit!\n";

              t0 = timeInterval.start +
                    (timeInterval.end - timeInterval.start) *
                    ( (ep0.GetX() - p0.GetX()) / (p1.GetX() - p0.GetX()) );
              tlc = true;
            }

            if( ep1 == p1 )
            {
              // +++++ for debugging purposes only +++++
              //cout << "ep0 equals p1!\n";

              t1 = timeInterval.end;
              trc = timeInterval.rc;
            }
            else {
              // +++++ for debugging purposes only +++++
              //cout << "ep1 is a Point within the Unit!\n";

              t1 = timeInterval.start +
                    (timeInterval.end - timeInterval.start) *
                    ( (ep1.GetX() - p0.GetX()) / (p1.GetX() - p0.GetX()) );
              trc = true;
            }
            temporalalgebra::Interval<Instant> interval( t0, t1, tlc, trc );

            // +++++ for debugging purposes only +++++
            //cout << "Create a new CUPoint...\n";

            CUPoint unit( epsilon, interval, ep0, ep1 );
            result.Add( unit );
          }
        }
      }
    }
  }
}

void CUPoint::P_At( const Point& p, CUPoint& result) const
{
  assert( p.IsDefined() );
  assert( IsDefined() );


  if( !p.Inside(this->BBox2D()) )
  {
    result.SetDefined(false);
    return;
  }

  //CUPoint *pResult = &result;
  double distp0 = p.Distance( p0 );

  if ( AlmostEqual(p0, p1) )
  {
    if( distp0 < epsilon || AlmostEqual( distp0, epsilon ) )
      result = *this;
  }
  else
  {
    double distp1 = p.Distance( p1 );
    if( ( distp0 < epsilon || AlmostEqual( distp0, epsilon ) ) &&
        ( distp1 < epsilon || AlmostEqual( distp1, epsilon ) ) )
      result = *this;

/*
To determine that part of a cupoint, that is possibly at the given point, draw
a circle with the radius of the uncertainty-value around this point and
determine the intersection-points of this circle with the halfsegment created
from the cupoint's endpoints.

*/

    //HalfSegment segCup( true, p0, p1 );

    Coord circleX = p.GetX(),
          circleY = p.GetY();

    double a,b,c;
    double bb4ac, mu1, mu2;
    Coord p0x = p0.GetX();
    Coord p0y = p0.GetY();
    Coord p1x = p1.GetX();
    Coord p1y = p1.GetY();
    Coord lengthX = p1x - p0x;
    Coord lengthY = p1y - p0y;

    a = lengthX * lengthX + lengthY * lengthY;
    b = 2 * (lengthX * (p0x - circleX) + lengthY * (p0y - circleY) );
    c = circleX * circleX + circleY * circleY;
    c += p0x * p0x + p0y * p0y;
    c -= 2 * (circleX * p0x + circleY * p0y);
    c -= epsilon * epsilon;
    bb4ac = b * b - 4 * a * c;
    // originally: if (fabs(a) <= EPS || bb4ac < 0) but 'EPS' was
    // not declared in the code-example, this algorithm is derived from.
    if( bb4ac < 0 )
    {
      mu1 = 0;
      mu2 = 0;
      result.SetDefined(false);
      return;
    }

    mu1 = (-b + sqrt(bb4ac)) / (2 * a);
    mu2 = (-b - sqrt(bb4ac)) / (2 * a);

    Point ep0(false, 0, 0);
    Point ep1(false, 0, 0);

    if( distp0 < epsilon || AlmostEqual( distp0, epsilon ) )
      ep0 = p0;
    else
      ep0.Set( p0x + mu2*(p1x - p0x), p0y + mu2*(p1y - p0y) );
    if( distp1 < epsilon || AlmostEqual( distp1, epsilon ) )
      ep1 = p1;
    else
      ep1.Set( p0x + mu1*(p1x - p0x), p0y + mu1*(p1y - p0y) );

    Instant t0(instanttype);
    Instant t1(instanttype);
    bool tlc;
    bool trc;

    if( AlmostEqual( p0.GetX(), p1.GetX() ) &&
        AlmostEqual( p0.GetX(), ep0.GetX() ) &&
        AlmostEqual( p0.GetX(), ep1.GetX() ) )
        // If the segment is vertical
    {
      if( (( p0.GetY() <= ep0.GetY() && p1.GetY() >= ep0.GetY() ) ||
          ( p0.GetY() >= ep0.GetY() && p1.GetY() <= ep0.GetY() )) &&
          (( p0.GetY() <= ep1.GetY() && p1.GetY() >= ep1.GetY() ) ||
          ( p0.GetY() >= ep1.GetY() && p1.GetY() <= ep1.GetY() )) )
      {
        if( ep0 == p0 )
        {
          t0 = timeInterval.start;
          tlc = timeInterval.lc;
        }
        else {
          t0 = timeInterval.start +
                (timeInterval.end - timeInterval.start) *
                ( (ep0.GetY() - p0.GetY()) / (p1.GetY() - p0.GetY()) );
          tlc = true;
        }
        if( ep1 == p1 )
        {
          t1 = timeInterval.end;
          trc = timeInterval.rc;
        }
        else {
          t1 = timeInterval.start +
                (timeInterval.end - timeInterval.start) *
                ( (ep1.GetY() - p0.GetY()) / (p1.GetY() - p0.GetY()) );
          trc = true;
        }
        temporalalgebra::Interval<Instant> interval( t0, t1, tlc, trc );
        CUPoint unit( epsilon, interval, ep0, ep1 );
        result = unit;
      }
    }
    else if( AlmostEqual( p0.GetY(), p1.GetY() ) &&
            AlmostEqual( p0.GetY(), ep0.GetY() ) &&
            AlmostEqual( p0.GetY(), ep1.GetY() ) )
            // The segment is horizontal
    {
      if( (( p0.GetX() <= ep0.GetX() && p1.GetX() >= ep0.GetX() ) ||
          ( p0.GetX() >= ep0.GetX() && p1.GetX() <= ep0.GetX() )) &&
          (( p0.GetX() <= ep1.GetX() && p1.GetX() >= ep1.GetX() ) ||
          ( p0.GetX() >= ep1.GetX() && p1.GetX() <= ep1.GetX() )) )
      {
        if( ep0 == p0 )
        {
          t0 = timeInterval.start;
          tlc = timeInterval.lc;
        }
        else {
          t0 = timeInterval.start +
                (timeInterval.end - timeInterval.start) *
                ( (ep0.GetX() - p0.GetX()) / (p1.GetX() - p0.GetX()) );
          tlc = true;
        }
        if( ep1 == p1 )
        {
          t1 = timeInterval.end;
          trc = timeInterval.rc;
        }
        else {
          t1 = timeInterval.start +
                (timeInterval.end - timeInterval.start) *
                ( (ep1.GetX() - p0.GetX()) / (p1.GetX() - p0.GetX()) );
          trc = true;
        }
        temporalalgebra::Interval<Instant> interval( t0, t1, tlc, trc );
        CUPoint unit( epsilon, interval, ep0, ep1 );
        result = unit;
      }
    }
    else
    {
      // +++++ for debugging purposes only +++++
      //cout << "Determine, if ep0 and ep1 are points of the unit... \n";

      double k0;
      if(ep0 != p0)
        k0 = (ep0.GetX() - p0.GetX()) / (ep0.GetY() - p0.GetY());
      double k1 = (ep1.GetX() - p0.GetX()) / (ep1.GetY() - p0.GetY());
      double k2 = (p1.GetX() - p0.GetX()) / (p1.GetY() - p0.GetY());

      //cout << "k0 = " << k0 << "   k1 = " << k1 << "   k2 = " << k2
      //  << endl;

      if( (ep0 == p0 || AlmostEqual( k0, k2 ) ) && AlmostEqual( k1, k2 ) &&
          ( (( p0.GetX() <= ep0.GetX() && p1.GetX() >= ep0.GetX() ) ||
          ( p0.GetX() >= ep0.GetX() && p1.GetX() <= ep0.GetX() )) &&
          (( p0.GetX() <= ep1.GetX() && p1.GetX() >= ep1.GetX() ) ||
          ( p0.GetX() >= ep1.GetX() && p1.GetX() <= ep1.GetX() )) ) )
      {
        if( ep0 == p0 )
        {
          // +++++ for debugging purposes only +++++
          //cout << "ep0 equals p0!\n";

          t0 = timeInterval.start;
          tlc = timeInterval.lc;
        }
        else {
          // +++++ for debugging purposes only +++++
          //cout << "ep0 is a Point within the Unit!\n";

          t0 = timeInterval.start +
                (timeInterval.end - timeInterval.start) *
                ( (ep0.GetX() - p0.GetX()) / (p1.GetX() - p0.GetX()) );
          tlc = true;
        }

        if( ep1 == p1 )
        {
          // +++++ for debugging purposes only +++++
          //cout << "ep0 equals p1!\n";

          t1 = timeInterval.end;
          trc = timeInterval.rc;
        }
        else {
          // +++++ for debugging purposes only +++++
          //cout << "ep1 is a Point within the Unit!\n";

          t1 = timeInterval.start +
                (timeInterval.end - timeInterval.start) *
                ( (ep1.GetX() - p0.GetX()) / (p1.GetX() - p0.GetX()) );
          trc = true;
        }
        temporalalgebra::Interval<Instant> interval( t0, t1, tlc, trc );

        // +++++ for debugging purposes only +++++
        //cout << "Create a new CUPoint...\n";

        CUPoint unit( epsilon, interval, ep0, ep1 );
        result = unit;
      }
    }
  }
}

void CUPoint::P_At( const Region& r, CMPoint& result) const
{
  assert( r.IsDefined() );
  assert( IsDefined() );

  result.Clear();

/*
1. If the cupoint's bbox and the region's bbox do not overlap, the result
is empty (or undefined).

*/
  if( !r.BoundingBox().Intersects( this->BBox2D() ) )
  {
    // +++++ for debugging purposes only +++++
    //cout << "The Region does NOT intersect the CUPoint's bounding box!\n";

    return;
  }

  //CUPoint *pResult;
  bool containsP0 = r.Contains( p0 );
  int i;
  HalfSegment segRgn;        // a halfsegment for iterating the region

/*
2. Determine if the distance of the unit (especially its endpoints) to the
region is less than the uncertainty-value.

If the uncertain unit point is constant (if p0 almost equals p1), the unit is
possibly at the region, if p0 lies inside the region, or its distance to the
region is less than epsilon.

*/

  if ( AlmostEqual(p0, p1) )
  {
    if( containsP0 )
    {
      result.Add(*this);
      return;
    }

    for(i = 0; i < r.Size(); i++ )
    {
      r.Get( i, segRgn);
      if (segRgn.IsLeftDomPoint() &&
          (segRgn.Distance(p0) <= epsilon) )
      {
        result.Add(*this);
        return;
      }
    }
  }
/*
If the uncertain unit point is not constant, one has to determine if the
maximum distance to the region is less than the uncertainty-value. If so, the
complete unit is 'possibly-at' the region. Else, there must be one or more
Passing Points where the uncertainty-area enters or leaves the region.

*/
  else
  {
    bool containsP1 = r.Contains( p1 );
    bool cupIntersectsRgn = false;
    double distP0;
    int hsMinDistP0 = -1;
    double distP1;
    int hsMinDistP1 = -1;
/*
To compute various Passing Points, a spatial representation of the unit is
needed (the epsilon-value is used separately and so is ignored here).

*/
    HalfSegment segCup(true, p0, p1);
    bool hsCloserEpsilon;
    bool aPosPPtooClose = false;
    const int maxPosPPs = 16;
    int definedPosPPs = 0;
    int aktPosPPs = 0;
    int k;
    int l;
    Point posPP[maxPosPPs];
/*
The array posPP stores points, where the uncertainty-area of the cupoint
touches the regions border (the points on segCup whose distance to the regions
border equals the epsilon-value). In most cases there will be only 1 or 2
Passing Points, but if there is a long CUPoint lying upon the border of a very
rugged region, there may be more than just 2 Passing Points.

*/
    bool posPPtooClose[maxPosPPs];
    int lastPosPPhs[maxPosPPs];
/*
The Variable lastPosPPhs is a pointer-array to the last halfsegments of the
region to which a Passing Point was computed. This is to ensure that the
distance between a later defined posPP and this halfsegment can be proved
again.

*/
    for (k = 0; k < maxPosPPs; k++)
    {
      // initialize the arrays
      posPP[k].SetDefined(false);
      posPPtooClose[k] = false;
      lastPosPPhs[k] = -1;
    }
    //r.StartBulkLoad();


    double dist;
    int hSegsCloserEpsilon[32];
/*
The int-array hSegsTooFar stores the halfsegments of the region whose distance
to the cupoint is greater than epsilon.

*/
    int noSegsCloserEpsilon = 0;


    for(i = 0; i < r.Size(); i++)
    {
      r.Get( i, segRgn );

      if( segRgn.IsLeftDomPoint() )
      {
        // As every Segment is represented by 2 halfsegments, just one of them
        // has to be proved.

        // +++++ for debugging purposes only +++++
        //Coord lpx = segRgn->GetLeftPoint().GetX();
        //Coord lpy = segRgn->GetLeftPoint().GetY();
        //Coord rpx = segRgn->GetRightPoint().GetX();
        //Coord rpy = segRgn->GetRightPoint().GetY();
        //cout << "segRgn is defined by the edgepoints " << lpx << " " << lpy
        //  << "     " << rpx << " " << rpy << endl;

        hsCloserEpsilon = false;
        for(k = 0; k < aktPosPPs; k++)
          posPPtooClose[k] = false;

/*
If the endpoints do not lie inside the region, and their distances to the
region is greater than epsilon, one may need to compute Passing Points. For
this purpose, the closest halfsegments to the endpoints, and their distance is
stored in variables.

*/
        cupIntersectsRgn = segCup.Intersects(segRgn);

        if( !containsP0 )
        {
          dist = segRgn.Distance(p0);
          if ( hsMinDistP0 < 0 || dist < distP0 )
          {
            distP0 = dist;
            hsMinDistP0 = i;
          }
        }

        if (!containsP1 )
        {
          dist = segRgn.Distance(p1);
          if( hsMinDistP1 < 0 || dist < distP1 )
          {
            distP1 = dist;
            hsMinDistP1 = i;
          }
        }
/*
Determine if the distance of a previously defined posPP to the actual region's
halfsegment is greater than epsilon. If so, set its posPPtooFar[k]-value to
TRUE, mark it as 'undefined' and decrease the counter 'definedPosPPs'.

*/
        for(k = 0; k < aktPosPPs; k++)
        {
          if( posPP[k].IsDefined() )
          {
            dist = segRgn.Distance( posPP[k] );
            posPPtooClose[k] = (dist < epsilon && !AlmostEqual(dist, epsilon));
            if( posPPtooClose[k] )
            {
              posPP[k].SetDefined(false);
              //if(definedPosPPs > 0)
              definedPosPPs--;
              aPosPPtooClose = true;

              // +++++ for debugging purposes only +++++
              //cout << "segRgn->Distance(posPP" << k
              //  << ") greater than epsilon" << endl;
              //cout << "Distance to posPP[" << k << "]: "
              //    << segRgn->Distance(posPP[k]) << endl;
              //cout << "There are actually " << definedPosPPs << " posPPs "
              //    << "defined!\n";
            }
          }
        }
/*
Determine if the unit lies on the outer side of this region's halfsegment, and
if the distance to this halfsegment is less than epsilon. If so, store the
index of this halfsegment in the array hSegsCloserEpsilon, to compare it later
to the possibly defined Passing Points.

*/
        dist = segCup.Distance( segRgn );
        if( dist < epsilon && !AlmostEqual(dist, epsilon) )
        {
          if(noSegsCloserEpsilon < 32)
          {
            hSegsCloserEpsilon[noSegsCloserEpsilon] = i;
            noSegsCloserEpsilon++;
          }
          else {
            cout << "Error in CUPoint::D_At(Region&, CMPoint&): \n";
            cout << "There are more than 32 HalfSegments in the region whose"
                 << " distance to the unit is less than epsilon!"
                 << "The result is not computable by this algorithm!";
            cerr << "Error in CUPoint::D_At(Region&, CMPoint&): \n";
            cerr << "There are more than 32 HalfSegments in the region whose"
                 << " distance to the unit is less than epsilon!"
                 << "The result is not computable by this algorithm!";
            result.SetDefined(false);
            return;
          }
          double dummy;
          hsCloserEpsilon = (( segRgn.attr.insideAbove &&
            !segRgn.RayAbove(p0, dummy) && !segRgn.RayAbove(p1, dummy)) ||
            (!segRgn.attr.insideAbove && segRgn.RayAbove(p0, dummy) &&
             segRgn.RayAbove(p1, dummy)) );
        }
/*


*/
        if( hsCloserEpsilon || cupIntersectsRgn || aPosPPtooClose )
        {
          // +++++ for debugging purposes only +++++
          //if(hsCloserEpsilon)
          //    cout << "hstooClose is TRUE!\n";
          //if(cupIntersectsRgn)
          //    cout << "cupIntersectsRgn is TRUE!\n";
          //if(aPosPPtooClose)
          //    cout << "aDefPPtooClose is TRUE!\n";

          Point p(false, 0, 0);
          if( FindPosPassingPoint(segCup, segRgn, epsilon, p) )
          {
            // +++++ for debugging purposes only +++++
            //cout << "FindPosPassingPoint returns true.\n";

            for(k = 0; k < aktPosPPs; k++)
            {
              if( posPP[k].IsDefined() )
              {
                if( AlmostEqual(p.GetX(), posPP[k].GetX()) &&
                    AlmostEqual(p.GetY(), posPP[k].GetY()) )
                {
                  // +++++ for debugging purposes only +++++
                  //cout << "The found posPP is already defined. Don't add "
                  //  "this Point to the mentioned posPP's!\n";

                  p.SetDefined(false);
                }
              }
              if( posPPtooClose[k] && lastPosPPhs[k] > -1 )
              {
                // A defPP was previously defined, so for this new defPP, the
                // distance to halfsegment lastDefPPhs[k] has to be compared
                // to epsilon
                r.Get( lastPosPPhs[k], segRgn );
                dist = segRgn.Distance(p);
                if( dist < epsilon && !AlmostEqual(dist, epsilon) )
                  p.SetDefined(false);
                lastPosPPhs[k] = -1;
              }
            }
            aPosPPtooClose = false;

            if( p.IsDefined() )
            {
              k = 0;
              while( posPP[k].IsDefined() )
                k++;
              if( k == aktPosPPs )
              {
                if( aktPosPPs < maxPosPPs )
                  aktPosPPs++;
                else {
                  cout << "Error in CUPoint::P_At(Region&, CMPoint&): \n";
                  cout << "There are more than 16 Passing Points defined for "
                    << "this unit. This leads to an internal array-overflow!"
                    << "The result is not computable by this algorithm!";
                  cerr << "Error in CUPoint::P_At(Region&, CMPoint&): \n";
                  cerr << "There are more than 16 Passing Points defined for "
                    << "this unit. This leads to an internal array-overflow!"
                    << "The result is not computable by this algorithm!";
                  result.SetDefined(false);
                  return;
                }
              }

              posPP[k] = p;
              posPPtooClose[k] = false;
              lastPosPPhs[k] = i;   // set lastDefPPhs[l] to the index of
                                    // the actual region
              definedPosPPs++;

              // +++++ for debugging purposes only +++++
              //cout << "The number of defined Passing Points was "
              //  << "INcreased to " << definedPosPPs << "! \n";
              //cout << "posPP = " << posPP[k].GetX() << " " << posPP[k].GetY()
              //  << endl;
            }
          }
        }
      }
    }
    //r.EndBulkLoad();

/*
Build new CUPoint-Objects from the determined points:

If the distance of the whole unit to the region is less than epsilon, it can
simply be added to the result.

*/
    if( ( containsP0 || ((hsMinDistP0 > -1) &&
          ( (distP0 < epsilon) || AlmostEqual(distP0, epsilon) ) ) ) &&
        (containsP1 || ((hsMinDistP1 > -1) &&
          ( (distP1 < epsilon) || AlmostEqual(distP1, epsilon) ) ) ) &&
        (definedPosPPs < 1) )
    {
      result.Add( *this );
      return;
    }
/*
Else, determine, if the distance of one of the endpoints to the region is
greater than epsilon. If so, compute an additional Passing Point and determine
if there is a halfsegment in the array hSegsCloserEpsilon with a distance less
than epsilon to this Point. If this occurs, the Passing Point has to be
redefined.

*/

    if( !containsP0 && hsMinDistP0 > -1 && distP0 > epsilon )
    {
      // +++++ for debugging purposes only +++++
      //cout << endl << "The distance of P0 to the region is greater than "
      //  "epsilon, so compute an additional posPP!\n";

      Point p(false, 0, 0);
      r.Get(hsMinDistP0, segRgn);
      if( FindPosPassingPoint(segCup, segRgn, epsilon, p) )
      {
        // +++++ for debugging purposes only +++++
        //cout << "FindPosPassingPoint returns true.\n";

        for(k = 0; k < aktPosPPs; k++)
        {
          if( posPP[k].IsDefined() )
          {
            if( AlmostEqual(p.GetX(), posPP[k].GetX()) &&
                AlmostEqual(p.GetY(), posPP[k].GetY()) )
              p.SetDefined(false);
          }
        }
        if( p.IsDefined() )
        {
          k = 0;
          while( posPP[k].IsDefined() )
            k++;
          if( k == aktPosPPs )
          {
            if( aktPosPPs < maxPosPPs )
              aktPosPPs++;
            else {
              cout << "Error in CUPoint::P_At(Region&, CMPoint&): \n";
              cout << "There are more than 16 Passing Points defined for "
                << "this unit. This leads to an internal array-overflow!"
                << "The result is not computable by this algorithm!";
              cerr << "Error in CUPoint::P_At(Region&, CMPoint&): \n";
              cerr << "There are more than 16 Passing Points defined for "
                << "this unit. This leads to an internal array-overflow!"
                << "The result is not computable by this algorithm!";
              result.SetDefined(false);
              return;
            }
          }

          posPP[k] = p;
          posPPtooClose[k] = false;
          lastPosPPhs[k] = i;   // set lastDefPPhs[l] to the index of
                                // the actual region
          definedPosPPs++;

          // +++++ for debugging purposes only +++++
          //cout << "The number of defined Passing Points was "
          //  << "INcreased to " << definedPosPPs << "! \n";
          //cout << "posPP = " << posPP[k].GetX() << " " << posPP[k].GetY()
          //    << endl;
        }
      }
    }

    if( !containsP1 && hsMinDistP1 > -1 && distP1 > epsilon )
    {
      // +++++ for debugging purposes only +++++
      //cout << endl << "The distance of P1 to the region is greater than "
      //  "epsilon, so compute an additional posPP!\n";

      Point p(false, 0, 0);
      r.Get( hsMinDistP1, segRgn);
      if( FindPosPassingPoint(segCup, segRgn, epsilon, p) )
      {
        // +++++ for debugging purposes only +++++
        //cout << "FindPosPassingPoint returns true.\n";

        for(k = 0; k < aktPosPPs; k++)
        {
          if( posPP[k].IsDefined() )
          {
            if( AlmostEqual(p.GetX(), posPP[k].GetX()) &&
                AlmostEqual(p.GetY(), posPP[k].GetY()) )
              p.SetDefined(false);
          }
        }
        if( p.IsDefined() )
        {
          k = 0;
          while( posPP[k].IsDefined() )
            k++;
          if( k == aktPosPPs )
          {
            if( aktPosPPs < maxPosPPs )
              aktPosPPs++;
            else {
              cout << "Error in CUPoint::P_At(Region&, CMPoint&): \n";
              cout << "There are more than 16 Passing Points defined for "
                << "this unit. This leads to an internal array-overflow!"
                << "The result is not computable by this algorithm!";
              cerr << "Error in CUPoint::P_At(Region&, CMPoint&): \n";
              cerr << "There are more than 16 Passing Points defined for "
                << "this unit. This leads to an internal array-overflow!"
                << "The result is not computable by this algorithm!";
              result.SetDefined(false);
              return;
            }
          }

          posPP[k] = p;
          posPPtooClose[k] = false;
          lastPosPPhs[k] = i;   // set lastPosPPhs[l] to the index of
                                // the actual region
          definedPosPPs++;

          // +++++ for debugging purposes only +++++
          //cout << "The number of defined Passing Points was "
          //  << "INcreased to " << definedPosPPs << "! \n";
          //cout << "posPP = " << posPP[k].GetX() << " " << posPP[k].GetY()
          //    << endl;
        }
      }
    }

    // Eliminate all posPP's that lie too close to one of the HalfSegments
    // stored in the hSegsTooClose-Array.
    if( noSegsCloserEpsilon > 0 )
    {
      for( k = 0; k < aktPosPPs; k++ )
      {
        if( posPP[k].IsDefined() )
        {
          int j = 0;
          while(j < noSegsCloserEpsilon && posPP[k].IsDefined() )
          {
            r.Get( hSegsCloserEpsilon[j], segRgn );
            dist = segRgn.Distance( posPP[k] );
            if( dist < epsilon && !AlmostEqual( dist, epsilon ) )
            {
              posPP[k].SetDefined(false);
              definedPosPPs--;

              // +++++ for debugging purposes only +++++
              //cout << "Segment No. " << j << ", defined by the points (";
              //segRgn->GetLeftPoint().Print(cout);
              //cout << " ";
              //segRgn->GetRightPoint().Print(cout);
              //cout << ") is too close to posPP[" << k << "]\n";
              //cout << "The number of defined Passing Points was "
              //  << "DEcreased to " << definedPosPPs << "! \n";
              //cout << "Distance to posPP[" << k << "] ("<< posPP[k].GetX()
              //  << " " << posPP[k].GetY()"): " << segRgn->Distance(posPP[k])
              //  << endl;
            }
            j++;
          }
        }
      }
    }

    // +++++ for debugging purposes only +++++
    //cout << "Distance of p0 to the region = " << distP0 << endl;
    //cout << "The closest halfsegment to p0 has the index " << hsMinDistP0
    //  << endl;
    //cout << "Distance of p1 to the region = " << distP1 << endl;
    //cout << "The closest halfsegment to p1 has the index " << hsMinDistP1
    //  << endl;
    //cout << "There are actually " << definedPosPPs << " defined posPPs.\n";


    if( ( containsP0 || ((hsMinDistP0 > -1) &&
          ( (distP0 < epsilon) || AlmostEqual(distP0, epsilon) ) ) ) &&
        (containsP1 || ((hsMinDistP1 > -1) &&
          ( (distP1 < epsilon) || AlmostEqual(distP1, epsilon) ) ) )&&
        (definedPosPPs < 1) )
    {
      result.Add( *this );
      return;
    }
    else
    {
      // determine two points from the remaining defPPs for a new unit. Include
      // the endpoints in this evaluation if they lie inside the region and
      // their distance to the region's border is greater than epsilon.

      Point ep0(false, 0, 0);
      Point ep1(false, 0, 0);
      bool firstrun = (containsP0 || ((hsMinDistP0 > -1) &&
                        ( (distP0 < epsilon) || AlmostEqual(distP0, epsilon))));

      // +++++ for debugging purposes only +++++
      //cout << "Begin building new CUPoints...\n";

      while( definedPosPPs > 0 )
      {
        // +++++ for debugging purposes only +++++
        //cout << "There are actually  " << definedPosPPs << "posPPs! \n";

        if( p0 < p1 )
        {
          // +++++ for debugging purposes only +++++
          //cout << "p0 < p1\n";

          if( firstrun )
          {
            ep0 = p0;
            ep1.SetDefined(false);
            for( k = 0; k < aktPosPPs; k++ )
            {
              if( posPP[k].IsDefined() )
              {
                if( !ep1.IsDefined() )
                {
                  ep1 = posPP[k];
                  l = k;
                }
                else if( posPP[k] < ep1 )
                {
                  ep1 = posPP[k];
                  l = k;
                }
              }
            }
            posPP[l].SetDefined(false);
            definedPosPPs--;
            firstrun = false;
          }
          else if( (definedPosPPs<2) && ( containsP1 || ((hsMinDistP1>-1) &&
                        ((distP1<epsilon) || AlmostEqual(distP1, epsilon)) ) ) )
          {
            ep0.SetDefined(false);
            ep1 = p1;
            for( k = 0; k < aktPosPPs; k++ )
            {
              if( posPP[k].IsDefined() )
              {
                if( !ep0.IsDefined() )
                {
                  ep0 = posPP[k];
                  l = k;
                }
                else if( posPP[k] < ep0 )
                {
                  ep0 = posPP[k];
                  l = k;
                }
              }
            }
            posPP[l].SetDefined(false);
            definedPosPPs--;
          }
          else {
            ep0.SetDefined(false);
            ep1.SetDefined(false);
            for( k = 0; k < aktPosPPs; k++ )
            {
              if( posPP[k].IsDefined() )
              {
                // +++++ for debugging purposes only +++++
                //cout << "posPP[" << k << "] is defined and represents point "
                //  << posPP[k].GetX() << " " << posPP[k].GetY() << endl;

                if( !ep0.IsDefined() )
                {
                  ep0 = posPP[k];
                  l = k;
                }
                else if( posPP[k] < ep0 )
                {
                  ep0 = posPP[k];
                  l = k;
                }
              }
            }
            // +++++ for debugging purposes only +++++
            //cout << "ep0 is the point at array-position " << l << "\n";

            posPP[l].SetDefined(false);

            for( k = 0; k < aktPosPPs; k++ )
            {
              if( posPP[k].IsDefined() )
              {
                if( !ep1.IsDefined() )
                {
                  ep1 = posPP[k];
                  l = k;
                }
                else if( posPP[k] < ep1 )
                {
                  ep1 = posPP[k];
                  l = k;
                }
              }
            }
            // +++++ for debugging purposes only +++++
            //cout << "ep1 is the point at array-position " << l << "\n";

            posPP[l].SetDefined(false);
            definedPosPPs = definedPosPPs - 2;
          }
        }
        else
        {
          // ( p0 > p1 )

          // +++++ for debugging purposes only +++++
          //cout << "p0 > p1\n";

          if( firstrun )
          {
            ep0 = p0;
            ep1.SetDefined(false);
            for( k = 0; k < aktPosPPs; k++ )
            {
              if( posPP[k].IsDefined() )
              {
                if( !ep1.IsDefined() )
                {
                  ep1 = posPP[k];
                  l = k;
                }
                else if( posPP[k] > ep1 )
                {
                  ep1 = posPP[k];
                  l = k;
                }
              }
            }
            posPP[l].SetDefined(false);
            definedPosPPs--;
            firstrun = false;
          }
          else if( (definedPosPPs < 2) && ( containsP1 || ((hsMinDistP1 > -1) &&
                      ((distP1 < epsilon) || AlmostEqual(distP1, epsilon))) ) )
          {
            ep0.SetDefined(false);
            ep1 = p1;
            for( k = 0; k < aktPosPPs; k++ )
            {
              if( posPP[k].IsDefined() )
              {
                if( !ep0.IsDefined() )
                {
                  ep0 = posPP[k];
                  l = k;
                }
                else if( posPP[k] > ep0 )
                {
                  ep0 = posPP[k];
                  l = k;
                }
              }
            }
            posPP[l].SetDefined(false);
            definedPosPPs--;
          }
          else
          {
            ep0.SetDefined(false);
            ep1.SetDefined(false);
            for( k = 0; k < aktPosPPs; k++ )
            {
              if( posPP[k].IsDefined() )
              {
                // +++++ for debugging purposes only +++++
                //cout << "posPP[" << k << "] is defined and represents point "
                //  << posPP[k].GetX() << " " << posPP[k].GetY() << endl;

                if( !ep0.IsDefined() )
                {
                  ep0 = posPP[k];
                  l = k;
                }
                else if( posPP[k] > ep0 )
                {
                  ep0 = posPP[k];
                  l = k;
                }
              }
            }

            // +++++ for debugging purposes only +++++
            //cout << "ep0 is the point at array-position " << l << "\n";

            posPP[l].SetDefined(false);
            for( k = 0; k < aktPosPPs; k++ )
            {
              if( posPP[k].IsDefined() )
              {
                // +++++ for debugging purposes only +++++
                //cout << "posPP[" << k << "] is defined and represents point "
                //  << posPP[k].GetX() << " " << posPP[k].GetY() << endl;

                if( !ep1.IsDefined() )
                {
                  ep1 = posPP[k];
                  l = k;
                }
                else if( posPP[k] > ep1 )
                {
                  ep1 = posPP[k];
                  l = k;
                }
              }
            }

            // +++++ for debugging purposes only +++++
            //cout << "ep1 is the point at array-position " << l << "\n";

            posPP[l].SetDefined(false);
            definedPosPPs = definedPosPPs - 2;
          }
        }
        // build a new CUPoint from these two points

        // +++++ for debugging purposes only +++
        //cout << "D_Passes: P0 liegt mit Abstand Epsilon in Region!\n";

        // determine the timeInterval for the new unit
        Instant t0(instanttype);
        Instant t1(instanttype);
        bool tlc;
        bool trc;

        // +++++ for debugging purposes only +++++
        //cout << "Value of definedDefPPs: " << definedPosPPs << endl;
        //if( !ep0.IsDefined() ) {
        //  cout << "Point ep0 is NOT defined!\n";
        //  cout << "Values of ep0: " << ep0.GetX() << " " << ep0.GetY()
        //      << endl;
        //}
        //else {
        //  cout << "Point ep0 is defined!\n";
        //  cout << "Values of ep0: " << ep0.GetX() << " " << ep0.GetY()
        //      << endl;
        //}
        //if( !ep1.IsDefined() ) {
        //  cout << "Point ep1 is NOT defined!\n";
        //  cout << "Values of ep1: " << ep1.GetX() << " " << ep1.GetY()
        //      << endl;
        //}
        //else {
        //  cout << "Point ep1 is defined!\n";
        //  cout << "Values of ep1: " << ep1.GetX() << " " << ep1.GetY()
        //      << endl;
        //}


        if( AlmostEqual( p0.GetX(), p1.GetX() ) &&
            AlmostEqual( p0.GetX(), ep0.GetX() ) &&
            AlmostEqual( p0.GetX(), ep1.GetX() ) )
            // If the segment is vertical
        {
          if( (( p0.GetY() <= ep0.GetY() && p1.GetY() >= ep0.GetY() ) ||
              ( p0.GetY() >= ep0.GetY() && p1.GetY() <= ep0.GetY() )) &&
              (( p0.GetY() <= ep1.GetY() && p1.GetY() >= ep1.GetY() ) ||
              ( p0.GetY() >= ep1.GetY() && p1.GetY() <= ep1.GetY() )) )
          {
            if( ep0 == p0 )
            {
              t0 = timeInterval.start;
              tlc = timeInterval.lc;
            }
            else {
              t0 = timeInterval.start +
                    (timeInterval.end - timeInterval.start) *
                    ( (ep0.GetY() - p0.GetY()) / (p1.GetY() - p0.GetY()) );
              tlc = true;
            }
            if( ep1 == p1 )
            {
              t1 = timeInterval.end;
              trc = timeInterval.rc;
            }
            else {
              t1 = timeInterval.start +
                    (timeInterval.end - timeInterval.start) *
                    ( (ep1.GetY() - p0.GetY()) / (p1.GetY() - p0.GetY()) );
              trc = true;
            }
            temporalalgebra::Interval<Instant> interval( t0, t1, tlc, trc );
            CUPoint unit( epsilon, interval, ep0, ep1 );
            result.Add( unit );
          }
        }
        else if( AlmostEqual( p0.GetY(), p1.GetY() ) &&
                AlmostEqual( p0.GetY(), ep0.GetY() ) &&
                AlmostEqual( p0.GetY(), ep1.GetY() ) )
                // The segment is horizontal
        {
          if( (( p0.GetX() <= ep0.GetX() && p1.GetX() >= ep0.GetX() ) ||
              ( p0.GetX() >= ep0.GetX() && p1.GetX() <= ep0.GetX() )) &&
              (( p0.GetX() <= ep1.GetX() && p1.GetX() >= ep1.GetX() ) ||
              ( p0.GetX() >= ep1.GetX() && p1.GetX() <= ep1.GetX() )) )
          {
            if( ep0 == p0 )
            {
              t0 = timeInterval.start;
              tlc = timeInterval.lc;
            }
            else {
              t0 = timeInterval.start +
                    (timeInterval.end - timeInterval.start) *
                    ( (ep0.GetX() - p0.GetX()) / (p1.GetX() - p0.GetX()) );
              tlc = true;
            }
            if( ep1 == p1 )
            {
              t1 = timeInterval.end;
              trc = timeInterval.rc;
            }
            else {
              t1 = timeInterval.start +
                    (timeInterval.end - timeInterval.start) *
                    ( (ep1.GetX() - p0.GetX()) / (p1.GetX() - p0.GetX()) );
              trc = true;
            }
            temporalalgebra::Interval<Instant> interval( t0, t1, tlc, trc );
            CUPoint unit( epsilon, interval, ep0, ep1 );
            result.Add( unit );
          }
        }
        else
        {
          // +++++ for debugging purposes only +++++
          //cout << "Determine, if ep0 and ep1 are points of the unit... \n";

          double k0;
          if(ep0 != p0)
            k0 = (ep0.GetX() - p0.GetX()) / (ep0.GetY() - p0.GetY());
          double k1 = (ep1.GetX() - p0.GetX()) / (ep1.GetY() - p0.GetY());
          double k2 = (p1.GetX() - p0.GetX()) / (p1.GetY() - p0.GetY());

          // +++++ for debugging purposes only +++++
          //cout << "k0 = " << k0 << "   k1 = " << k1 << "   k2 = " << k2
          //  << endl;

          if( (ep0 == p0 || AlmostEqual( k0, k2 ) ) && AlmostEqual( k1, k2 ) &&
              ( (( p0.GetX() <= ep0.GetX() && p1.GetX() >= ep0.GetX() ) ||
              ( p0.GetX() >= ep0.GetX() && p1.GetX() <= ep0.GetX() )) &&
              (( p0.GetX() <= ep1.GetX() && p1.GetX() >= ep1.GetX() ) ||
              ( p0.GetX() >= ep1.GetX() && p1.GetX() <= ep1.GetX() )) ) )
          {
            if( ep0 == p0 )
            {
              // +++++ for debugging purposes only +++++
              //cout << "ep0 equals p0!\n";

              t0 = timeInterval.start;
              tlc = timeInterval.lc;
            }
            else {
              // +++++ for debugging purposes only +++++
              //cout << "ep0 is a Point within the Unit!\n";

              t0 = timeInterval.start +
                    (timeInterval.end - timeInterval.start) *
                    ( (ep0.GetX() - p0.GetX()) / (p1.GetX() - p0.GetX()) );
              tlc = true;
            }

            if( ep1 == p1 )
            {
              // +++++ for debugging purposes only +++++
              //cout << "ep0 equals p1!\n";

              t1 = timeInterval.end;
              trc = timeInterval.rc;
            }
            else {
              // +++++ for debugging purposes only +++++
              //cout << "ep1 is a Point within the Unit!\n";

              t1 = timeInterval.start +
                    (timeInterval.end - timeInterval.start) *
                    ( (ep1.GetX() - p0.GetX()) / (p1.GetX() - p0.GetX()) );
              trc = true;
            }
            temporalalgebra::Interval<Instant> interval( t0, t1, tlc, trc );

            // +++++ for debugging purposes only +++++
            //cout << "Create a new CUPoint...\n";

            CUPoint unit( epsilon, interval, ep0, ep1 );
            result.Add( unit );
          }
        }
      }
    }
  }
}

/*
3.3 Class ~CMPoint~

3.3.1 Implementation of Member Functions

*/
void CMPoint::Clear()
{
  Mapping<CUPoint, Point>::Clear(); // call super
  bbox.SetDefined(false);          // invalidate bbox
}

void CMPoint::Add( const CUPoint& unit )
{
  assert( unit.IsValid() );
  assert( unit.IsDefined() );
  units.Append( unit );
  if(units.Size() == 1)
  {
    bbox = unit.BoundingBox();
  }
  else
  {
    bbox = bbox.Union(unit.BoundingBox());
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
  CUPoint last;
  Get(size-1,last);
  if(last.timeInterval.end!=unit.timeInterval.start ||
     !( (last.timeInterval.rc )  ^ (unit.timeInterval.lc))){
     // intervals are not connected
     Add(unit);
     bbox = bbox.Union(unit.BoundingBox());
     return;
  }
  if(!AlmostEqual(last.p1, unit.p0)){
    // jump in spatial dimension
    Add(unit);
    bbox = bbox.Union(unit.BoundingBox());
    return;
  }
  // define the epsilon-value of the two merged uncertain units:
  double e;
  if (unit.epsilon > last.epsilon)
    e = unit.epsilon;
  else
    e = last.epsilon;

  temporalalgebra::Interval<Instant> complete(last.timeInterval.start,
                             unit.timeInterval.end,
                             last.timeInterval.lc,
                             unit.timeInterval.rc);
  CUPoint cupoint(e, complete,last.p0, unit.p1);
  delete &e;
  Point p(false, 0, 0);
  cupoint.TemporalFunction(last.timeInterval.end, p, true);
  if(!AlmostEqual(p,last.p0)){
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
  DbArray<CUPoint> tmp(0);
  units.Restrict( intervals,tmp ); // call super
  units.copyFrom(tmp);
  bbox.SetDefined(false);      // invalidate bbox
  RestoreBoundingBox();        // recalculate it
}

bool CMPoint::EndBulkLoad(const bool sort, const bool dummy)
{
  bool res = Mapping<CUPoint, Point>::EndBulkLoad( sort ); // call super
  RestoreBoundingBox();                        // recalculate, if necessary
  return res;
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
    CUPoint unit;
    int size = GetNoComponents();
    bool isfirst = true;
    for( int i = 0; i < size; i++ )
    {
      Get( i, unit );
      if (isfirst)
      {
        epsilon = unit.GetEpsilon();
        UncertainSetDefined( true );
        isfirst = false;
      }
      else if( epsilon < unit.GetEpsilon() )
      {
        epsilon = unit.GetEpsilon();
      }
      // else: there's nothing to do.
    }
  }
}


/*
RestoreBoundingBox() checks, whether the temporalalgebra::MPoint's 
MBR ~bbox~ is ~undefined~
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
    CUPoint unit;
    int size = GetNoComponents();
    bool isfirst = true;
    for( int i = 0; i < size; i++ )
    {
      Get( i, unit );
      if (isfirst)
      {
        bbox = unit.BoundingBox();
        isfirst = false;
      }
      else
      {
        bbox = bbox.Union(unit.BoundingBox());
      }
    }
  } // else: bbox unchanged and still correct
}

Rectangle<3u> CMPoint::BoundingBox() const
{
  return bbox;
}

Rectangle<2> CMPoint::BBox2D() const
{
  return Rectangle<2>( true, bbox.MinD(0), bbox.MaxD(0),
                             bbox.MinD(1), bbox.MaxD(1) );
}


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

bool CMPoint::Present( const temporalalgebra::Periods& t ) const
{
  assert( IsDefined() );
  assert( IsOrdered() );
  assert( t.IsDefined() );
  assert( t.IsOrdered() );

  if(bbox.IsDefined())
  { // do MBR-check
    double MeMin = bbox.MinD(2);
    double MeMax = bbox.MaxD(2);
    Instant tmin(instanttype); t.Minimum(tmin);
    Instant tmax(instanttype); t.Maximum(tmax);
    double pmin = tmin.ToDouble();
    double pmax = tmax.ToDouble();
    if( (pmax < MeMin && !AlmostEqual(pmax,MeMin)) ||
         (pmin > MeMax && !AlmostEqual(pmin,MeMax))
      )
    {
      return false;
    }
  }
  temporalalgebra::Periods defTime( 0 );
  DefTime( defTime );
  return t.Intersects( defTime );
}

void CMPoint::AtInstant( const Instant& t, 
                         temporalalgebra::Intime<Region>& result ) const
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
          CUPoint posUnit;
          units.Get( pos, posUnit );
          result.SetDefined( true );
          Point respoint(false, 0, 0);
          posUnit.TemporalFunction( t, respoint );

          double e = posUnit.GetEpsilon();
          if( e == 0.0 )
          {
            e = fabs(posUnit.p0.GetX());
            if( fabs(posUnit.p1.GetX()) > e )
              e = fabs(posUnit.p1.GetX());
            if( fabs(posUnit.p0.GetY()) > e )
              e = fabs(posUnit.p0.GetY());
            if( fabs(posUnit.p1.GetY()) > e )
              e = fabs(posUnit.p1.GetY());
            e = e * minEpsilonLimiter;
          }
          Circle(respoint, e, 16, result.value);
          result.instant.CopyFrom( &t );
        }
      }
    }
  } else
  {
    result.SetDefined(false);
  }
}

void CMPoint::AtPeriods( const temporalalgebra::Periods& p, 
                         CMPoint& result ) const
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
      Instant perMinInst(instanttype); p.Minimum(perMinInst);
      Instant perMaxInst(instanttype); p.Maximum(perMaxInst);
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
        CUPoint unit;
        temporalalgebra::Interval<Instant> interval;
        int i = 0, j = 0;
        Get( i, unit );
        p.Get( j, interval );

        while( 1 )
        {
          if( unit.timeInterval.Before( interval ) )
          {
            if( ++i == GetNoComponents() )
              break;
            Get( i, unit );
          }
          else if( interval.Before( unit.timeInterval ) )
          {
            if( ++j == p.GetNoComponents() )
              break;
            p.Get( j, interval );
          }
          else
          { // we have overlapping intervals, now
            CUPoint r( false );
            unit.AtInterval( interval, r );
            r.epsilon = unit.epsilon;
            r.UncertainSetDefined(true);
            result.Add( r );

            if( interval.end == unit.timeInterval.end )
            { // same ending instant
              if( interval.rc == unit.timeInterval.rc )
              { // same ending instant and rightclosedness: Advance both
                if( ++i == GetNoComponents() )
                  break;
                Get( i, unit );
                if( ++j == p.GetNoComponents() )
                  break;
                p.Get( j, interval );
              }
              else if( interval.rc == true )
              { // Advanve in mapping
                if( ++i == GetNoComponents() )
                  break;
                Get( i, unit );
              }
              else
              { // Advance in periods
                assert( unit.timeInterval.rc == true );
                if( ++j == p.GetNoComponents() )
                  break;
                p.Get( j, interval );
              }
            }
            else if( interval.end > unit.timeInterval.end )
            { // Advance in mpoint
              if( ++i == GetNoComponents() )
                break;
              Get( i, unit );
            }
            else
            { // Advance in periods
              assert( interval.end < unit.timeInterval.end );
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

bool CMPoint::D_Passes( const Point& p ) const
{
  assert( p.IsDefined() );
  assert( IsDefined() );
  bool result = false;
  {
    if( !p.Inside(BBox2D()) )
      return false;

    CUPoint unit;
    int i = 0;
    while( result == false && i < GetNoComponents() )
    {
      Get( i, unit );
      result = unit.D_Passes( p );
      i++;
    }

    // If the epsilon-value equals 0, the cupoint-object is certain and
    // can be casted to a temporalalgebra::UPoint-object.
    return result;
  }
}


bool CMPoint::D_Passes( const Region& r ) const
{
  bool result = false;
  if( IsDefined() && r.IsDefined() )
  {
    if( !r.BoundingBox().Intersects(BBox2D()) )
      return false;

    CUPoint unit;
    int i = 0;
    while( result == false && i < GetNoComponents() )
    {
      Get( i, unit );
      result = unit.D_Passes( r );
      i++;
    }
  }

  return result;
}

bool CMPoint::P_Passes( const Point& p ) const
{
  assert( p.IsDefined() );
  assert( IsDefined() );
  bool result = false;
  {
    if( !p.Inside(BBox2D()) )
      return false;

    CUPoint unit;
    int i = 0;
    while( result == false && i < GetNoComponents() )
    {
      Get( i, unit );
      result = unit.P_Passes( p );
      i++;
    }

    // If the epsilon-value equals 0, the cupoint-object is certain and
    // can be casted to a temporalalgebra::UPoint-object.
    return result;
  }
}

bool CMPoint::P_Passes( const Region& r ) const
{
  bool result = false;
  if( IsDefined() && r.IsDefined() )
  {
    if( !r.BoundingBox().Intersects(BBox2D()) )
      return false;

    CUPoint unit;
    int i = 0;
    while( result == false && i < GetNoComponents() )
    {
      Get( i, unit );
      result = unit.P_Passes( r );
      i++;
    }
  }

  return result;
}

void CMPoint::D_At( const Point& p, CMPoint& result ) const
{
  assert( IsOrdered() );
  assert( p.IsDefined() );

  result.Clear();

  if( !p.Inside(BBox2D()) )
    result.SetDefined( false );

  CUPoint unit;
  CUPoint resultunit( false );
  int i = 0;
  while( i < GetNoComponents() )
  {
    result.StartBulkLoad();

    Get( i, unit );
    unit.D_At( p, resultunit );
    if( resultunit.IsDefined() )
      result.Add( resultunit );
    i++;

    result.EndBulkLoad();
  }
}

void CMPoint::D_At( const Region& r, CMPoint& result ) const
{
  assert( IsOrdered() );
  assert( r.IsDefined() );

  if( !r.BoundingBox().Intersects(BBox2D()) )
    result.SetDefined( false );

  result.Clear();

  CUPoint unit;
  CMPoint resultunits( 0 );
  CUPoint resunit;

  result.StartBulkLoad();

  // +++++ for debugging purposes only +++++
  //cout << "The CMPoint contains " << GetNoComponents() << " Units.\n";

  for(int i = 0; i < GetNoComponents(); i++)
  {
    Get( i, unit );
    unit.D_At( r, resultunits );
    if( resultunits.IsDefined() )
    {
      // +++++ for debugging purposes only +++++
      //cout << "CMPoint::D_At: resultunit " << i << " is defined"
      //  << "    and has " << resultunits.GetNoComponents() << " components."
      //  << endl;

      for(int j = 0; j < resultunits.GetNoComponents(); j++)
      {
        resultunits.Get( j, resunit );

        // +++++ for debugging purposes only +++++
        //cout << endl;
        //cout << "CMPoint::D_At(...): Add CUPoint to the result-CMPoint: \n";
        //cout << "CUPoint: epsilon = " << resunit->epsilon << "  coordinates "
        //  << "= "  << resunit->p0.GetX() << " " << resunit->p0.GetY()
        //  << "     " << resunit->p1.GetX() << " " << resunit->p1.GetY()
        //  << endl;

        result.Add( resunit );
      }
    }
  }
  result.EndBulkLoad();
}

void CMPoint::P_At( const Point& p, CMPoint& result ) const
{
  assert( IsOrdered() );
  assert( p.IsDefined() );

  result.Clear();

  if( !p.Inside(BBox2D()) )
    result.SetDefined( false );

  CUPoint unit;
  CUPoint resunit( false );

  result.StartBulkLoad();
  for( int i = 0; i < GetNoComponents(); i++ )
  {
    Get(i, unit);
    unit.P_At( p, resunit );
    if( resunit.IsDefined() )
      result.Add( resunit );
  }
  result.EndBulkLoad();
}

void CMPoint::P_At( const Region& r, CMPoint& result ) const
{
  assert( IsOrdered() );
  assert( r.IsDefined() );

  if( !r.BoundingBox().Intersects(BBox2D()) )
    result.SetDefined( false );

  result.Clear();

  CUPoint unit;
  CMPoint resultunits( 0 );
  CUPoint resunit;

  result.StartBulkLoad();

  // +++++ for debugging purposes only +++++
  //cout << "The CMPoint contains " << GetNoComponents() << " Units.\n";

  for(int i = 0; i < GetNoComponents(); i++)
  {
    Get( i, unit );

    // +++++ for debugging purposes only +++++
    //cout << "Unit " << i << " is defined by the endpoints "
    //  << unit->p0.GetX() << " " << unit->p0.GetY() << "    "
    //  << unit->p1.GetX() << " " << unit->p1.GetY() << endl;

    unit.P_At( r, resultunits );
    if( resultunits.IsDefined() )
    {
      // +++++ for debugging purposes only +++++
      //cout << "CMPoint::D_At: resultunit " << i << " is defined"
      //  << "    and has " << resultunits.GetNoComponents() << " components."
      //  << endl;

      for(int j = 0; j < resultunits.GetNoComponents(); j++)
      {
        resultunits.Get( j, resunit );

        // +++++ for debugging purposes only +++++
        //cout << endl;
        //cout << "CMPoint::D_At(...): Add CUPoint to the result-CMPoint: \n";
        //cout << "CUPoint: epsilon = " << resunit->epsilon << "  coordinates "
        //  << "= " << resunit->p0.GetX() << " " << resunit->p0.GetY()
        //  << "     " << resunit->p1.GetX() << " " << resunit->p1.GetY()
        //  << endl;

        result.Add( resunit );
      }
    }
  }
  result.EndBulkLoad();
}

/*
3.3.2 Implementations of Functions to be part of relations

*/
ostream& CMPoint::Print( ostream &os ) const
{
  if( !IsDefined() )
    return os << "(CMPoint: undefined)";

  os << "(CMPoint: defined, MBR = ";
  bbox.Print(os);
  os << ", maximum Epsilon = " << epsilon << endl;
  os << "contains " << GetNoComponents() << " units: ";
  for(int i = 0; i < GetNoComponents(); i++)
  {
     CUPoint unit;
    Get( i, unit );
    os << "\n\t";
    unit.Print(os);
  }
  os << "\n)" << endl;
  return os;
}

Attribute* CMPoint::Clone() const
{
  assert( IsOrdered() );
  CMPoint *result = new CMPoint( GetNoComponents() );
  if(GetNoComponents()>0){
    result->units.resize(GetNoComponents());
  }
  result->StartBulkLoad();
  CUPoint unit;
  for( int i = 0; i < GetNoComponents(); i++ )
  {
    Get( i, unit );
    result->Add( unit );
  }
  result->EndBulkLoad( false );
  return (Attribute*) result;
}

void CMPoint::CopyFrom( const Attribute* right )
{
  const CMPoint *r = (const CMPoint*)right;
  assert( r->IsOrdered() );
  Clear();
  int size = r->GetNoComponents();
  if(size > 0)
    this->units.resize(size);

  StartBulkLoad();
  CUPoint unit;
  for( int i = 0; i < r->GetNoComponents(); i++ )
  {
    r->Get( i, unit );
    Add( unit );
  }
  EndBulkLoad( false );
}

/*
3.4 HierarchicalEntity

*/



/*
3.5 HCUPoint

*/


/*
3.6 HCMPoint

3.6.1 Member Functions

*/
inline bool HCMPoint::IsEmpty() const
{
  if( GetNoComponents() == 0 )
    return true;
  return false;
}

inline int HCMPoint::GetNoComponents() const
{
  int noComponents = layer0.Size() + layer1.Size() + layer2.Size() +
                      layer3.Size() + layer4.Size();
  return noComponents;
}

double HCMPoint::GetLayerepsilon( const int layer ) const
{
  switch( layer )
  {
    case 0:
      return layer0epsilon;
    case 1:
      return layer1epsilon;
    case 2:
      return layer2epsilon;
    case 3:
      return layer3epsilon;
    case 4:
      return layer4epsilon;
    default:
      // this point should never been reached
      return -1;
  }
}

void HCMPoint::SetLayerepsilon( const int layer, const double epsilon )
{
  switch( layer )
  {
    case 0:
      layer0epsilon = epsilon;
    case 1:
      layer1epsilon = epsilon;
    case 2:
      layer2epsilon = epsilon;
    case 3:
      layer3epsilon = epsilon;
    case 4:
      layer4epsilon = epsilon;
    default:
      // this point should never been reached
      break;
  }
}

inline void HCMPoint::Get( const int layer, const int i, HCUPoint& ntt ) const
{
  assert( layer > -1 );
  assert( layer < 5 );

  switch( layer )
  {
    case 0:
      if( i >= layer0.Size() )
        cout << "There are less than " << i << " elements in layer0!\n";
      else
        layer0.Get(i, ntt);
      break;

    case 1:
      if( i >= layer1.Size() )
        cout << "There are less than " << i << " elements in layer1!\n";
      else
        layer1.Get(i, ntt);
      break;

    case 2:
      if( i >= layer2.Size() )
        cout << "There are less than " << i << " elements in layer2!\n";
      else
        layer2.Get(i, ntt);
      break;

    case 3:
      if( i >= layer3.Size() )
        cout << "There are less than " << i << " elements in layer3!\n";
      else
        layer3.Get(i, ntt);
      break;

    case 4:
      if( i >= layer4.Size() )
        cout << "There are less than " << i << " elements in layer4!\n";
      else
        layer4.Get(i, ntt);
      break;

    default:
      // this case is prevented by the assert-commands
      break;
  }
}

inline void HCMPoint::Get( const int layer, const int i, CUPoint& cup ) const
{
  HCUPoint ntt;
  Get( layer, i, ntt );
  cup = &(ntt.value);
}

void HCMPoint::Get( const int i, HCUPoint& ntt ) const
{
  assert( i < GetNoComponents() );
  int idx = i;

  if( idx < layer0.Size() )
  {
    layer0.Get( idx, ntt );
    return;
  }
  idx -= layer0.Size();

  if( idx < layer1.Size() )
  {
    layer1.Get( idx, ntt );
    return;
  }
  idx -= layer1.Size();

  if( idx < layer2.Size() )
  {
    layer2.Get( idx, ntt );
    return;
  }
  idx -= layer2.Size();

  if( idx < layer3.Size() )
  {
    layer3.Get( idx, ntt );
    return;
  }
  idx -= layer3.Size();

  layer4.Get( idx, ntt );

  if ( !ntt.IsDefined() )
  {
    cout << __FILE__ << "," << __LINE__ << ":" << __PRETTY_FUNCTION__
         << " Get(" << i << ", Entity): Entity is undefined:";
    ntt.Print(cout); cout << endl;
    assert( false );
  }
  if ( !ntt.IsValid() )
  {
    cout << __FILE__ << "," << __LINE__ << ":" << __PRETTY_FUNCTION__
      << " Get(" << i << ", Entity): Entity is invalid:";
    ntt.Print(cout); cout << endl;
    assert( false );
  }
}

inline void HCMPoint::Put( const int layer, const int i, HCUPoint& ntt )
{
  assert( layer > -1 );
  assert( layer < 5 );

  switch( layer )
  {
    case 0:
      if( i > layer0.Size() )
        layer0.resize( i );
      layer0.Put(i, ntt);
      break;

    case 1:
      if( i > layer1.Size() )
        layer1.resize( i );
      layer1.Put(i, ntt);
      break;

    case 2:
      if( i > layer2.Size() )
        layer2.resize( i );
      layer2.Put(i, ntt);
      break;

    case 3:
      if( i > layer3.Size() )
        layer3.resize( i );
      layer3.Put(i, ntt);
      break;

    case 4:
      if( i > layer4.Size() )
        layer4.resize( i );
      layer4.Put(i, ntt);
      break;

    default:
      // this case is prevented by the assert-commands
      break;
  }
}



void HCMPoint::GetCMPoint( const double epsilon, CMPoint& result )
{
  assert( !IsEmpty() );

  result.Clear();
  // Determine the first layer, whose epsilon-value is less than the wanted
  // epsilon-value and build a CMPoint from its entities:
  if( epsilon >= layer0epsilon )
  {
    result.Resize( layer0.Size() );
    HCUPoint ntt;
    result.StartBulkLoad();
    for(int i = 0; i < layer0.Size(); i++)
    {
      layer0.Get( i, ntt );
      CUPoint resunit( ntt.value );
      result.Add( resunit );
    }
    result.EndBulkLoad();
  }
  else if( epsilon >= layer1epsilon )
  {
    result.Resize( layer1.Size() );
    HCUPoint ntt;
    result.StartBulkLoad();
    for(int i = 0; i < layer1.Size(); i++)
    {
      layer1.Get( i, ntt );
      CUPoint resunit( ntt.value );
      result.Add( resunit );
    }
    result.EndBulkLoad();
  }
  else if( epsilon >= layer2epsilon )
  {
    result.Resize( layer2.Size() );
    HCUPoint ntt;
    result.StartBulkLoad();
    for(int i = 0; i < layer2.Size(); i++)
    {
      layer2.Get( i, ntt );
      CUPoint resunit( ntt.value );
      result.Add( resunit );
    }
    result.EndBulkLoad();
  }
  else if( epsilon >= layer3epsilon )
  {
    result.Resize( layer3.Size() );
    HCUPoint ntt;
    result.StartBulkLoad();
    for(int i = 0; i < layer3.Size(); i++)
    {
      layer3.Get( i, ntt );
      CUPoint resunit( ntt.value );
      result.Add( resunit );
    }
    result.EndBulkLoad();
  }
  else if( epsilon >= layer4epsilon )
  {
    result.Resize( layer4.Size() );
    HCUPoint ntt;
    result.StartBulkLoad();
    for(int i = 0; i < layer4.Size(); i++)
    {
      layer4.Get( i, ntt );
      CUPoint resunit( ntt.value );
      result.Add( resunit );
    }
    result.EndBulkLoad();
  }
  else
  {
    cout << "The wanted Uncertainty-value is less than the smallest epsilon-"
      "value! The most certain CMPoint has an Uncertainty-value of "
      << layer4epsilon << "!\n";
    result.SetDefined(false);
  }
}

void HCMPoint::Add( const HCUPoint& hcup )
{
  assert( hcup.IsDefined() );
  assert( hcup.IsValid() );

  const int layerno = hcup.GetLayer();

  switch( layerno )
  {
    case 0:
      if( layer0.Size() < hcup.GetIndex() )
        layer0.resize( hcup.GetIndex() + 1 );
      layer0.Put( hcup.GetIndex(), hcup );
      break;

    case 1:
      if( layer1.Size() < hcup.GetIndex() )
        layer1.resize( hcup.GetIndex() + 1 );
      layer1.Put( hcup.GetIndex(), hcup );
      break;

    case 2:
      if( layer2.Size() < hcup.GetIndex() )
        layer2.resize( hcup.GetIndex() + 1 );
      layer2.Put( hcup.GetIndex(), hcup );
      break;

    case 3:
      if( layer3.Size() < hcup.GetIndex() )
        layer3.resize( hcup.GetIndex() + 1 );
      layer3.Put( hcup.GetIndex(), hcup );
      break;

    case 4:
      if( layer4.Size() < hcup.GetIndex() )
        layer4.resize( hcup.GetIndex() + 1 );
      layer4.Put( hcup.GetIndex(), hcup );
      break;

    default:
      cout << __FILE__ << "," << __LINE__ << ":" << __PRETTY_FUNCTION__
        << " Add(hcupoint): invalid layer (" << hcup.GetLayer() << "):";
      hcup.Print(cout); cout << endl;
      assert( false );
  }
}

void HCMPoint::Clear()
{
  layer0epsilon = -1;
  layer0.clean();

  layer1epsilon = -1;
  layer1.clean();

  layer2epsilon = -1;
  layer2.clean();

  layer3epsilon = -1;
  layer3.clean();

  layer4epsilon = -1;
  layer4.clean();
}
/*

*/

inline int HCMPoint::LayerSize( const int layer ) const
{
  switch( layer )
  {
    case 0:
      return layer0.Size();

    case 1:
      return layer1.Size();

    case 2:
      return layer2.Size();

    case 3:
      return layer3.Size();

    case 4:
      return layer4.Size();

    default:
      // this point should never be reached!
      return 0;
  }
}

inline void HCMPoint::ResizeLayer( const int layer, const int n )
{
  switch( layer )
  {
    case 0:
      layer0.resize( n );
      break;

    case 1:
      layer1.resize( n );
      break;

    case 2:
      layer2.resize( n );
      break;

    case 3:
      layer3.resize( n );
      break;

    case 4:
      layer4.resize( n );
      break;

    default:
      // this point should never be reached!
      break;
  }
}

inline void HCMPoint::TrimLayerToSize( const int layer )
{
  switch( layer )
  {
    case 0:
      layer0.TrimToSize();
      break;

    case 1:
      layer1.TrimToSize();
      break;

    case 2:
      layer2.TrimToSize();
      break;

    case 3:
      layer3.TrimToSize();
      break;

    case 4:
      layer4.TrimToSize();
      break;

    default:
      // this point should never be reached!
      break;
  }
}

int HCMPoint::Position( int layer, const Instant& t )
{
  assert( t.IsDefined() );

  int size;

  switch( layer )
  {
    case 0:
      size = layer0.Size();
      break;

    case 1:
      size = layer1.Size();
      break;

    case 2:
      size = layer2.Size();
      break;

    case 3:
      size = layer3.Size();
      break;

    case 4:
      size = layer4.Size();
      break;

    default:
      // This point should never be reached.
      break;
  }

  int first = 0, last = size - 1;
  Instant t1( t );

  while (first <= last)
  {
    int mid = ( first + last ) / 2;

    if( (mid < 0) || (mid >= size) )
      return -1;

    CUPoint midUnit;
    Get( layer, mid, midUnit );

    if( midUnit.timeInterval.Contains(t1) )
      return mid;
    else  //not contained
      if( ( t1 > midUnit.timeInterval.end ) ||
          ( t1 == midUnit.timeInterval.end ) )
        first = mid + 1;
      else if( ( t1 < midUnit.timeInterval.start ) ||
               ( t1 == midUnit.timeInterval.start ) )
        last = mid - 1;
      else
        return -1; //should never be reached.
    }
    return -1;
}

int HCMPoint::Position( int layer, const Instant& t, const int start,
                        const int end )
{
  assert( t.IsDefined() );
  assert( start >= 0 && end < LayerSize(layer) );

  Instant t1( t );
  int first = start;
  int last = end;

  // +++++ for debugging purposes only +++++
  //cout << "HCMPoint::Position: Start while- with 'first' = " << first
  //  << " and 'last' = " << last << endl;

  while (first <= last)
  {
    int mid = ( first + last ) / 2;

    if( (mid < start) || (mid > end) )
      return -1;

    CUPoint midUnit;
    Get( layer, mid, midUnit );

    // +++++ for debugging purposes only +++++
    //cout << "Prove CUPoint No. " << mid << ":\n";
    //midUnit->Print(cout);

    if( midUnit.timeInterval.Contains(t1) )
    {
      // +++++ for debugging purposes only +++++
      //cout << "CUPoint No. " << mid << "contains the Time-Instant.\n";

      return mid;
    }
    else  //not contained
      if( ( t1 > midUnit.timeInterval.end ) ||
          ( t1 == midUnit.timeInterval.end ) )
      {
        first = mid + 1;

        // +++++ for debugging purposes only +++++
        //cout << "The Time-Instant is GREATER than the CUPoint's "
        //  << "timeInterval.end. 'first' is set to " << first << "!\n";
      }
      else if( ( t1 < midUnit.timeInterval.start ) ||
               ( t1 == midUnit.timeInterval.start ) )
      {
        last = mid - 1;
        // +++++ for debugging purposes only +++++
        //cout << "The Time-Instant is LESS than the CUPoint's "
        //  << "timeInterval.end. 'last' is set to " << last << "!\n";
      }
      else
        return -1; //should never be reached.
    }
    return -1;
}
/*

*/

int HCMPoint::Generalize(const int layer, const bool checkBreakPoints,
                          const DateTime dur)
{
  const int origlayerno = layer;
  const int genlayerno = layer - 1;
  double *layerepsilon;
  double aktepsilon;
  int size;

  switch( layer )
  {
    case 1:
      // +++++ for debugging purposes only +++++
      //cout << "Computing of CUPoints for layer 0!\n";

      layerepsilon = &layer0epsilon;
      size = layer1.Size();
      if( size < 2 )
        return false;
      aktepsilon = epsilon * factor * factor * factor * factor;
      break;

    case 2:
      // +++++ for debugging purposes only +++++
      //cout << "Computing of CUPoints for layer 1!\n";

      layerepsilon = &layer1epsilon;
      size = layer2.Size();
      if( size < 2 )
        return false;
      aktepsilon = epsilon * factor * factor * factor;
      break;

    case 3:
      // +++++ for debugging purposes only +++++
      //cout << "Computing of CUPoints for layer 2!\n";

      layerepsilon = &layer2epsilon;
      size = layer3.Size();
      if( size < 2 )
        return false;
      aktepsilon = epsilon * factor * factor;
      break;

    case 4:
      // +++++ for debugging purposes only +++++
      //cout << "Computing of CUPoints for layer 3!\n";

      layerepsilon = &layer3epsilon;
      size = layer4.Size();
      if( size < 2 )
        return false;
      aktepsilon = epsilon * factor;
      break;

    default:
      // should never been reached!
      break;
  }

  // +++++ for debugging purposes only +++++
  //cout << "Control Variable-Initialization for next Generalization:\n";
  //cout << "     origlayerno       = " << origlayerno << endl;
  //cout << "     genlayerno        = " << genlayerno << endl;
  //cout << "     size of origlayer = " << size << endl;
  //cout << "     aktepsilon        = " << aktepsilon << endl << endl;

  // Create two bitmaps and a double array to keep in mind which sample point
  // belongs to the result and which real epsilon-value is computed for this
  // results cupoint.
  bool useleft[size];
  bool useright[size];
  double realepsilon[size];

  // Initialize the whole bitmaps to state FALSE and the double array to
  // value -1;
  for(int i = 0; i < size; i++)
  {
    useleft[i] = false;
    useright[i] = false;
    realepsilon[i] = -1;
  }

  int first = 0;
  int last = 1;
  CUPoint u1;
  CUPoint u2;


  while(last < size )
  {
    // for this purpose, only the upoint-part of the cupoint is needed:
    Get(origlayerno, last-1, u1);
    Get(origlayerno, last, u2);

    // check whether one of the cupoints is a constant cupoint and whether
    // last and last-1 are connected
    if( checkBreakPoints && IsBreakPoint(&u1,dur))
    {
      if(last-1 > first){
         Simplify(first, last-2, origlayerno, useleft, useright, realepsilon,
                   aktepsilon);
      }
      Simplify(last-1, last-1, origlayerno, useleft, useright, realepsilon,
                aktepsilon);
      first = last;
      last++;
    }
    else if( checkBreakPoints && IsBreakPoint(&u2,dur))
    {
      Simplify(first, last-1, origlayerno, useleft, useright, realepsilon,
                aktepsilon);
      last++;
      Simplify(last-1, last-1, origlayerno, useleft, useright, realepsilon,
                aktepsilon);
      first = last;
      last++;
    }
    else if(connected(&u1,&u2)) // enlarge the sequence
      last++;
    else
    {
      // +++++ for debugging purposes only +++++
      //cout << "The Units u1 = ";
      //u1->Print(cout);
      //cout << " and u2 = ";
      //u2->Print(cout);
      //cout << " are not connected with each other!\n";
      //cout << "Function Simplify(...) is called with parameters: \n";
      //cout << "     first =       " << first << endl;
      //cout << "     last-1 =      " << last-1 << endl;
      //cout << "     origlayerno = " << origlayerno << endl;
      //cout << "     aktepsilon =     " << aktepsilon << endl << endl;

      Simplify(first, last-1, origlayerno, useleft, useright, realepsilon,
                aktepsilon);
      first=last;
      last++;
    }
  }
  // process the last recognized sequence
  Simplify(first,last-1,origlayerno,useleft,useright,realepsilon,aktepsilon);

  // +++++ for debugging purposes only +++++
  //cout << "\nlast recognized sequence of units: \n";
  //cout << "Function Simplify(...) is called with parameters: \n";
  //cout << "     first =       " << first << endl;
  //cout << "     last-1 =      " << last-1 << endl;
  //cout << "     origlayerno = " << origlayerno << endl;
  //cout << "     epsilon =     " << aktepsilon << endl << endl;


  // count the number of remaining units
  int count = 1; // count the most right sample point
  for(int i = 0; i < size; i++)
  {
    if( realepsilon[i] > *layerepsilon )
      *layerepsilon = realepsilon[i];
    if( useleft[i] )
      count++;
  }
  ResizeLayer(genlayerno, count);

  // +++++ for debugging purposes only +++++
  //cout << "The Simplfiy()-Calls detected " << count << " for the next"
  //  " layer! \n";
  //cout << "The epsilon value of layer " << genlayerno << " has been set to "
  //  << *layerepsilon << endl;

  Instant start(instanttype);
  Point p0(false, 0, 0);
  bool closeLeft;
  //bool leftDefined = false;
  int generalizedby = 0;
  int originstart = 0;

  for(int i = 0; i < size; i++)
  {
    HCUPoint hcup;
    CUPoint cup;

    Get(origlayerno, i, hcup);
    HCUPoint aux( hcup );
    cup = &(aux.value);
    aux.SetGeneralizedby( generalizedby );
    Put(origlayerno, i, aux);

    if( useleft[i] )
    {
      // +++++ for debugging purposes only +++++
      //if(leftDefined)
      //  cout << " error in mpoint simplification,"
      //       << " overwrite an existing leftPoint "  << endl;

      originstart = i;
      p0 = cup.p0;
      closeLeft = cup.timeInterval.lc;
      start = cup.timeInterval.start;
      //leftDefined = true;
    }
    if( useright[i] )
    {
      // +++++ for debugging purposes only +++++
      //if(!leftDefined)
      //  cout << " error in mpoint simplification,"
      //       << " rightdefined before leftdefined "  << endl;

      temporalalgebra::Interval<Instant> interval(
                      start, cup.timeInterval.end, closeLeft,
                                  cup.timeInterval.rc);
      CUPoint newCUPoint(realepsilon[i], interval, p0, cup.p1);
      // Create a new Entity for the hierarchical structure, containing this
      // new CUPoint.
      HCUPoint newHCUPoint(newCUPoint,genlayerno,generalizedby,originstart,i);
      Put(genlayerno, generalizedby, newHCUPoint);

      // +++++ for debugging purposes only +++++
      //cout << "A new HCUPoint with index " << generalizedby
      //  << " has been created in layer " << genlayerno << "\n";
      //newHCUPoint.Print(cout);
      //cout << endl;

      // The next cup belongs to the next origin-hcup belongs to the next
      // generalization.
      generalizedby++;
      //leftDefined = false;
    }
  }
  return count;
}

void HCMPoint::Simplify(const int min, const int max, const int layer,
                    bool* useleft, bool* useright, double* realepsilon,
                    const double epsilon)
{

  // the endpoints are used in each case
  useleft[min] = true;
  useright[max] = true;



  if(min==max) // no intermediate sampling points -> nothing to simplify
  {
    CUPoint cup;
    Get(layer, max, cup);
    realepsilon[max] = cup.GetEpsilon();
    return;
  }

  CUPoint u1;
  CUPoint u2;
  // build a HalfSegment from the endpoints
  Get(layer, min, u1);
  Get(layer, max, u2);
  HalfSegment hs(true, u1.p0, u2.p1);

  // search for the point with the highest distance to its simplified position
  double maxDist = 0;
  int maxIndex=0;
  Point p_orig(false, 0, 0);
  CUPoint u;
  double distance;
  for(int i=min+1;i<=max;i++){
     Get(layer, i,u);
     //upoint.TemporalFunction(u->timeInterval.start,p_simple, true);
     distance  = hs.Distance(u.p0);
     if(distance>maxDist){ // new maximum found
        maxDist = distance;
        maxIndex = i;
     }
  }

  if(maxIndex==0){  // no difference found
    realepsilon[max] = maxDist;
    return;
  }
  if(maxDist<=epsilon){  // differnce is in allowed range
      realepsilon[max] = maxDist;
      return;
  }

  // split at the left point of maxIndex
  Simplify(min,maxIndex-1,layer,useleft,useright,realepsilon,epsilon);
  Simplify(maxIndex,max,layer,useleft,useright,realepsilon,epsilon);
}

/*
RestoreBoundingBox() checks, whether the temporalalgebra::MPoint's
 MBR ~bbox~ is ~undefined~
and thus may needs to be recalculated and if, does so.

*/

void HCMPoint::RestoreBoundingBox(const bool force)
{
  if(!IsDefined() || GetNoComponents() == 0)
  { // invalidate bbox
    bbox.SetDefined(false);
  }
  else if(force || !bbox.IsDefined())
  { // construct bbox
    int layer, size;
    GetFirstLayer(layer, size);

    CUPoint unit;
    bool isfirst = true;
    for( int i = 0; i < size; i++ )
    {
      Get(layer, i, unit );
      if (isfirst)
      {
        bbox = unit.BoundingBox();
        isfirst = false;
      }
      else
      {
        bbox = bbox.Union(unit.BoundingBox());
      }
    }
  } // else: bbox unchanged and still correct
}

// Class functions
Rectangle<3u> HCMPoint::BoundingBox() const
{
  return bbox;
}

Rectangle<2> HCMPoint::BBox2D() const
{
  // +++++ for debugging purposes only +++++
  //cout << "The BBox is defined as: MinD(0) = "<< bbox.MinD(0) <<" MaxD(0) = "
  //  << bbox.MaxD(0) << "     and MinD(1) = " << bbox.MinD(1) << " MaxD(1) = "
  //  << bbox.MaxD(1) << endl << endl;


  return Rectangle<2>( true, bbox.MinD(0), bbox.MaxD(0),
                             bbox.MinD(1), bbox.MaxD(1) );
}

inline void HCMPoint::GetFirstLayer( int& layer, int& size ) const
{
  if(layer0.Size() > 0)
  {
    layer = 0;
    size = layer0.Size();
  }
  else if(layer1.Size() > 0)
  {
    layer = 1;
    size = layer1.Size();
  }
  else if(layer2.Size() > 0)
  {
    layer = 2;
    size = layer2.Size();
  }
  else if(layer3.Size() > 0)
  {
    layer = 3;
    size = layer3.Size();
  }
  else if(layer4.Size() > 0)
  {
    layer = 4;
    size = layer4.Size();
  }
}

void HCMPoint::DefTime( temporalalgebra::Periods& p )
{
  int layer, size;
  GetFirstLayer(layer, size);

  temporalalgebra::Periods result( size );

  CUPoint unit;
  result.StartBulkLoad();
  for( int i = 0; i < size; i++ )
  {
    Get( layer, i, unit );
    result.Add( unit.timeInterval );
  }
  result.EndBulkLoad( false );
  result.Merge( p );
}

bool HCMPoint::Present( const Instant& t )
{
  assert( IsDefined() );
  assert( t.IsDefined() );

  int layer, size;
  GetFirstLayer(layer, size);

  if(bbox.IsDefined())
  { // do MBR-check
    double instd = t.ToDouble();
    double mint = bbox.MinD(2);
    double maxt = bbox.MaxD(2);
    if( (instd < mint && !AlmostEqual(instd,mint)) ||
        (instd > maxt && !AlmostEqual(instd,mint))
      )
    {
      // +++++ for debugging purposes only +++++
      //cout << "BoundingBox-Check failed!\n";

      return false;
    }
  }
  int pos = Position(layer, t);
  if( pos == -1 )         //not contained in any unit
    return false;
  return true;
}

/*
bool Present( const temporalalgebra::Periods\& t )

Checks all Units in the most uncertain Layer of the HCMPoint, if the given
temporalalgebra::Periods-Value is completely inside the Definition-time 
of the HCMPoint-object.

*/
bool HCMPoint::Present( const temporalalgebra::Periods& t )
{
  assert( IsDefined() );
  assert( t.IsDefined() );
  assert( t.IsOrdered() );

  if(bbox.IsDefined())
  { // do MBR-check
    double MeMin = bbox.MinD(2);
    double MeMax = bbox.MaxD(2);
    Instant tmin(instanttype); t.Minimum(tmin);
    Instant tmax(instanttype); t.Maximum(tmax);
    double pmin = tmin.ToDouble();
    double pmax = tmax.ToDouble();

    // +++++ for debugging purposes only +++++
    cout << "HCMPoint::Present:\n";
    cout << "BBox-Min = " << MeMin << "    BBox-Max = " << MeMax << endl;
    cout << "temporalalgebra::Periods-Min = " << pmin 
         << "    temporalalgebra::Periods-Max = " << pmax << endl
      << endl;


    if( (pmax < MeMin && !AlmostEqual(pmax,MeMin)) ||
         (pmin > MeMax && !AlmostEqual(pmin,MeMax))
      )
    {
      // +++++ for debugging purposes only +++++
      cout << "HCMPoint::Present: BBox-Check returned: no Intersection!\n";

      return false;
    }
  }
  temporalalgebra::Periods defTime( 0 );
  DefTime( defTime );
  return t.Intersects( defTime );
}

/*
AtPeriods( const temporalalgebra::Periods\& per, HMPoint\& result )

Computes, by a recursive in-order run, for every layer of this hmpoint-object
the intersection of the hmpoint to the given periods and returns the
intersection-set as a new hmpoint.

*/
void HCMPoint::AtPeriods( const temporalalgebra::Periods& p, CMPoint& result )
{
  assert( p.IsDefined() );
  assert( p.IsOrdered() );

  result.Clear();

  if( !IsDefined() ) // the result cmpoint stays empty!
    return;

  if( !bbox.IsDefined() || IsEmpty() ) // the result cmpoint stays empty!
    return;

  Instant perMinInst(instanttype); p.Minimum(perMinInst);
  Instant perMaxInst(instanttype); p.Maximum(perMaxInst);
  double permind = perMinInst.ToDouble();
  double permaxd = perMaxInst.ToDouble();
  double mind = bbox.MinD(2);
  double maxd = bbox.MaxD(2);
  if( (mind > permaxd && !AlmostEqual(mind,permaxd)) ||
      (maxd < permind && !AlmostEqual(maxd,permind)) )
    return;

  int layer, size;
  GetFirstLayer(layer, size);
  result.StartBulkLoad();
  AtPeriods( layer, 0, size-1, p, result );
  result.EndBulkLoad();
}

/*
bool AtInstant( const int layer, const int start, const int end,
const Region\& r )

This recursive Function determines, by a Devide-and-Conquer-Search through the
hierarchical structure, a circular shaped temporalalgebra::Intime<Region>-
Object, related to
the given Instant.

*/
int HCMPoint::AtPeriods( const int layer, const int start, const int end,
                  const temporalalgebra::Periods& p, CMPoint& result )
{
  temporalalgebra::Interval<Instant> interval;
  HCUPoint ntt;
  int i = start, j = 0;
  Get(layer, i, ntt );
  CUPoint unit( ntt.value );
  p.Get( j, interval );
  temporalalgebra::Periods auxPeriods( 0 );
  bool bottomLayer = (LayerSize(layer+1) == 0);

  while( true )
  {
    if( unit.timeInterval.Before( interval ) )
    {
      if( auxPeriods.GetNoComponents() > 0 )
      {
        int ostart = ntt.GetOriginstart();
        int oend = ntt.GetOriginend();
        if(layer < 5 && ostart > -1 && oend > -1)
        {
          AtPeriods(layer+1, ostart, oend, auxPeriods, result);
          auxPeriods.Clear();
        }
      }
      if(++i > end )
        break;
      Get( layer, i, ntt );
      unit = ntt.value;
    }
    else if( interval.Before( unit.timeInterval ) )
    {
      if(++j == p.GetNoComponents() )
        break;
      p.Get( j, interval );
    }
    else
    { // the actual intervals overlap each other

      // +++++ for debugging purposes only +++++
      //cout << "On layer " << layer << " the interval ";
      //interval->Print(cout);
      //cout << endl << "  overlaps the timeInterval of Unit " << i << "!\n";
      //cout << "    ";
      //unit.timeInterval.Print(cout);
      //cout << endl << endl;


      if( bottomLayer )
      {
        CUPoint r( false );
        unit.AtInterval( interval, r );
        r.epsilon = unit.GetEpsilon();
        r.UncertainSetDefined(true);

        assert( r.IsDefined() );
        assert( r.IsValid() );

        // +++++ for debugging purposes only +++++
        //cout << "Add new Unit with timeInterval = ";
        //r.timeInterval.Print(cout);
        //cout << endl << endl;

        result.Add( r );
      }
      else
      {
        auxPeriods.Add( interval );

        // +++++ for debugging purposes only +++++
        //cout << "Add the periods-interval to 'auxPeriods'!\n";
        //cout << "auxPeriods for layer " << layer+1 << " has "
        //  << auxPeriods.GetNoComponents() << " components.\n" << endl;
      }

      if( interval.end == unit.timeInterval.end )
      { // same ending instant
        if( interval.rc == unit.timeInterval.rc )
        { // same ending instant and rightclosedness: Advance both
          int ostart = ntt.GetOriginstart();
          int oend = ntt.GetOriginend();
          if( !bottomLayer && ostart > -1 && oend > -1)
          {
            AtPeriods(layer+1, ostart, oend, auxPeriods, result);
            auxPeriods.Clear();
          }
          if( ++i > end )
            break;
          Get(layer, i, ntt);
          unit = ntt.value;
          if( ++j == p.GetNoComponents() )
            break;
          p.Get( j, interval );

        }
        else if( interval.rc == true )
        { // Advance in hmpoint
          int ostart = ntt.GetOriginstart();
          int oend = ntt.GetOriginend();
          if( !bottomLayer && ostart > -1 && oend > -1)
          {
            AtPeriods(layer+1, ostart, oend, auxPeriods, result);
            auxPeriods.Clear();
          }

          if( ++i > end )
            break;
          Get(layer, i, ntt);
          unit = ntt.value;
        }
        else
        { // Advance in periods
          assert( interval.end < unit.timeInterval.end );
          if( ++j == p.GetNoComponents() )
            break;
          p.Get( j, interval );
        }
      }
      else if( interval.end > unit.timeInterval.end )
      { // Advance in hmpoint
        int ostart = ntt.GetOriginstart();
        int oend = ntt.GetOriginend();
        if( !bottomLayer && ostart > -1 && oend > -1)
        {
          AtPeriods(layer+1, ostart, oend, auxPeriods, result);
          auxPeriods.Clear();
        }
        if( ++i > end )
          break;
        Get(layer, i, ntt);
        unit = ntt.value;
      }
      else
      { // Advance in periods
        assert( interval.end < unit.timeInterval.end );
        if( ++j == p.GetNoComponents() )
        {
          int ostart = ntt.GetOriginstart();
          int oend = ntt.GetOriginend();
          if( !bottomLayer && ostart > -1 && oend > -1)
          {
            AtPeriods(layer+1, ostart, oend, auxPeriods, result);
            auxPeriods.Clear();
          }
          break;
        }
        p.Get( j, interval );
      }
    }
  }
  return 0;
}


/*
bool D\_Passes( const Point\& p )

Checks, if the given Point-Value lies inside the BoundingBox of this HCMPoint.
If so, it calls a recursive Function to determine if the HCMPoint definitely-
passes the given Point-value.

*/
bool HCMPoint::D_Passes( const Point& p )
{
  assert( p.IsDefined() );
  assert( IsDefined() );

  if( !p.Inside(BBox2D()) )
    return false;

  bool result = false;
  int layer, size;
  GetFirstLayer(layer, size);

  result = D_Passes( layer, 0, size-1, p );
  return result;
}

/*
bool D\_Passes( const int layer, const int start, const int end,
const Point\& p )

This recursive function determines, by a pre-order run through the hierarchical
 structure, if the HCMPoint definitely-passes the given Point-value.

*/
bool HCMPoint::D_Passes( const int layer, const int start, const int end,
                  const Point& p )
{
  bool result = false;
  HCUPoint ntt;
  CUPoint cup;
  temporalalgebra::UPoint u;

  for(int i = start; i <= end; i++ )
  {
    Get(layer, i, ntt);
    cup = (ntt.value);
    u = (temporalalgebra::UPoint)(ntt.value);
    if( cup.GetEpsilon() == 0.0 && u.Passes( p ) )
      return true;
    if( cup.P_Passes( p ) && layer < 4 )
    {
      int ostart = ntt.GetOriginstart();
      int oend = ntt.GetOriginend();
      if( ostart > -1 && oend > -1)
        result = D_Passes( layer+1, ostart, oend, p );
    }
    if( result )
      return true;
  }
  return false;
}

/*
bool D\_Passes( const Region\& r )

Checks, if the given Region-Value intersects the BoundingBox of this HCMPoint.
If so, it calls a recursive Function to determine if the HCMPoint definitely-
passes the given Region-value.

*/
bool HCMPoint::D_Passes( const Region& r )
{
  assert( r.IsDefined() );
  assert( IsDefined() );

  if( !r.BoundingBox().Intersects(BBox2D()) )
    return false;

  bool result = false;
  int layer, size;
  GetFirstLayer(layer, size);

  result = D_Passes( layer, 0, size-1, r );
  return result;
}

/*
bool D\_Passes( const int layer, const int start, const int end,
const Region\& r )

This recursive Function determines, by an in-order run through the hierarchical
 structure, if the HCMPoint definitely-passes the given Region-value.

*/
bool HCMPoint::D_Passes( const int layer, const int start, const int end,
                  const Region& r )
{
  bool result = false;
  HCUPoint ntt;
  CUPoint cup;

  for(int i = start; i <= end; i++ )
  {
    Get(layer, i, cup);
    if( cup.D_Passes( r ) )
      return true;
  }
  for(int i = start; i <= end; i++ )
  {
    Get(layer, i, ntt);
    cup = &(ntt.value);
    if( cup.P_Passes( r ) && layer < 4 )
    {
      int ostart = ntt.GetOriginstart();
      int oend = ntt.GetOriginend();
      if( ostart > -1 && oend > -1)
        result = D_Passes( layer+1, ostart, oend, r );
    }
    if( result )
      return true;
  }
  return false;
}

/*
bool P\_Passes( const Point\& p )

Checks, if the given Point-Value lies inside the BoundingBox of this HCMPoint.
If so, it calls a recursive Function to determine if the HCMPoint possibly-
passes the given Point-value.

*/
bool HCMPoint::P_Passes( const Point& p )
{
  assert( p.IsDefined() );
  assert( IsDefined() );

  if( !p.Inside(BBox2D()) )
    return false;

  bool result = false;
  int layer, size;
  GetFirstLayer(layer, size);

  result = P_Passes( layer, 0, size-1, p );
  return result;
}

/*
bool P\_Passes( const int layer, const int start, const int end,
const Point\& p )

This recursive function determines, by a pre-order run through the hierarchical
 structure, if the HCMPoint possibly-passes the given Point-value.

*/
bool HCMPoint::P_Passes( const int layer, const int start, const int end,
                  const Point& p )
{
  bool result = false;
  HCUPoint ntt;
  CUPoint cup;

  for(int i = start; i <= end; i++ )
  {
    Get(layer, i, ntt);
    cup = &(ntt.value);
    int ostart = ntt.GetOriginstart();
    int oend = ntt.GetOriginend();
    if( ostart == -1 && oend == -1 )
    {
      if( cup.P_Passes( p ) )
        return true;
    }
    else if( cup.P_Passes( p ) && layer < 4 )
      result = P_Passes( layer+1, ostart, oend, p );
    if( result )
      return true;
  }
  return false;
}

/*
bool P\_Passes( const Region\& r )

Checks, if the given Region-Value intersects the BoundingBox of this HCMPoint.
If so, it calls a recursive Function to determine if the HCMPoint possibly-
passes the given Region-value.

*/
bool HCMPoint::P_Passes( const Region& r )
{
  assert( r.IsDefined() );
  assert( IsDefined() );

  if( !r.BoundingBox().Intersects(BBox2D()) )
    return false;

  bool result = false;
  int layer, size;
  GetFirstLayer(layer, size);

  result = P_Passes( layer, 0, size-1, r );
  return result;
}

/*
bool P\_Passes( const int layer, const int start, const int end,
const Region\& r )

This recursive Function determines, if the HCMPoint possibly-passes the
given Region-value.

*/
bool HCMPoint::P_Passes( const int layer, const int start, const int end,
                  const Region& r )
{
  bool result = false;
  HCUPoint ntt;
  CUPoint cup;


  for(int i = start; i <= end; i++ )
  {
    Get(layer, i, ntt);
    cup = (ntt.value);
    if( cup.P_Passes( r ) )
    {
      int start = ntt.GetOriginstart();
      int end = ntt.GetOriginend();
      if( start == -1 && end == -1 )
        return true;
      else if( layer < 4 )
        result = P_Passes( layer+1, start, end, r );
    }
    if( result )
      return true;
  }
  return false;
}

/*
Checks, if the given Point-Value lies inside the BoundingBox of this HCMPoint.
If so, it calls a recursive Function to determine the HCMPoint definitely-
at the given Point-value.

*/
void HCMPoint::D_At( const Point& p, CMPoint& result )
{
  assert( p.IsDefined() );
  assert( IsDefined() );

  result.Clear();
  if( !p.Inside(BBox2D()) )
    return;

  //bool resIsValid = false;
  int layer, size;
  GetFirstLayer(layer, size);

  result.StartBulkLoad();
  D_At( layer, 0, size-1, p, result);
  result.EndBulkLoad();

  // +++++ for debugging purposes only +++++
  //if(result.IsEmpty())
  //  cout << "The resulting CMPoint-Object is EMPTY!\n";

}

/*
This recursive function determines, by a in-order run through the hierarchical
 structure, the HCMPoint definitely-at the given Point-value.

*/
bool HCMPoint::D_At( const int layer, const int start, const int end,
                  const Point& p, CMPoint& result)
{
  HCUPoint ntt;
  CUPoint unit;
  bool resIsValid = false;

  for(int i = start; i <= end; i++ )
  {
    Get(layer, i, ntt);
    int ostart = ntt.GetOriginstart();
    int oend = ntt.GetOriginend();

    if( ((ostart < 0) && (oend < 0)) || (LayerSize(layer+1) == 0) )
    {
      unit = &(ntt.value);
      CUPoint resunit( false );
      unit.D_At( p, resunit );
      if( resunit.IsDefined() )
        result.Add( resunit );
      resIsValid = !result.IsEmpty();
    }
    else if( ntt.value.P_Passes(p) )
      resIsValid = D_At( layer+1, ostart, oend, p, result ); // recursive call
  }
  return resIsValid;
}

/*
HCMPoint D\_At( const Region\& r, CMPoint\& result )

Checks, if the given Region-Value intersects the BoundingBox of this HCMPoint.
If so, it calls a recursive Function to determine the Units lying definitely-
at the given Region-value.

*/
void HCMPoint::D_At( const Region& r, CMPoint& result )
{
  assert( r.IsDefined() );
  assert( IsDefined() );

  result.Clear();
  if( !r.BoundingBox().Intersects(BBox2D()) )
    return;

  //bool resIsValid = false;
  int layer, size;
  GetFirstLayer(layer, size);

  //CMPoint rescmpoint( 0 );
  result.StartBulkLoad();
  D_At( layer, 0, size-1, r, result );
  result.EndBulkLoad();

  // +++++ for debugging purposes only +++++
  //if(result.IsEmpty())
  //  cout << "The resulting CMPoint-Object is EMPTY!\n";

  //if( resIsValid && !rescmpoint.IsEmpty() )
  //  Generalize(rescmpoint, GetEpsilon(), GetFactor(), result);
}

/*
This recursive Function determines, by a pre-order run through the hierarchical
 structure, if the HCMPoint definitely-passes the given Region-value.

*/
bool HCMPoint::D_At( const int layer, const int start, const int end,
                  const Region& r, CMPoint& result )
{
  HCUPoint ntt;
  CUPoint unit;
  bool resIsValid = false;

  // +++++ for debugging purposes only +++++
  //cout << "Call for recursive Function HCMPoint::D_At with Arguments:\n";
  //cout << "     layer = " << layer << endl;
  //cout << "     start = " << start << endl;
  //cout << "     end = " << end << endl << endl;

  for(int i = start; i <= end; i++ )
  {
    Get(layer, i, ntt);
    unit = (ntt.value);
    int ostart = ntt.GetOriginstart();
    int oend = ntt.GetOriginend();

    if( ((ostart < 0) && (oend < 0)) || (LayerSize(layer+1) == 0) )
    {
      // +++++ for debugging purposes only +++++
      //  cout << "HCMPoint::D_At: Reached bottom layer!\n";

      CMPoint resultunits( 0 );
      CUPoint resunit;

      unit.D_At( r, resultunits );
      if( resultunits.IsDefined() )
      {
        // +++++ for debugging purposes only +++++
        //cout << "HCMPoint::D_At: resultunit for entity" << i
        //  << " has " << resultunits.GetNoComponents() <<" components."
        //  << endl;

        for(int j = 0; j < resultunits.GetNoComponents(); j++)
        {
          resultunits.Get( j, resunit );

          // +++++ for debugging purposes only +++++
          //cout << endl;
          //cout<< "HCMPoint::D_At(...): Add CUPoint to the result-CMPoint:\n";
          //resunit->Print(cout);

          result.Add( resunit );
        }
      }
      resIsValid = !result.IsEmpty();
    }
    else if( unit.P_Passes( r ) )
    {
      // +++++ for debugging purposes only +++++
      //cout << "HCMPoint::D_At: the unit of entity" << i << " on layer " <<
      //  layer << " passes the region.\n";

      resIsValid =  D_At( layer+1, ostart, oend, r, result );
    }
  }
  return resIsValid;
}

/*
Checks, if the given Point-Value lies inside the BoundingBox of this HCMPoint.
If so, it calls a recursive Function to determine the HCMPoint definitely-
at the given Point-value.

*/
void HCMPoint::P_At( const Point& p, CMPoint& result )
{
  assert( p.IsDefined() );
  assert( IsDefined() );

  result.Clear();
  if( !p.Inside(BBox2D()) )
    return;

  //bool resIsValid = false;
  int layer, size;
  GetFirstLayer(layer, size);

  result.StartBulkLoad();
  P_At( layer, 0, size-1, p, result);
  result.EndBulkLoad();

  // +++++ for debugging purposes only +++++
  //if(result.IsEmpty())
  //  cout << "The resulting CMPoint-Object is EMPTY!\n";

}

/*
This recursive function determines, by a pre-order run through the hierarchical
 structure, the HCMPoint definitely-at the given Point-value.

*/
bool HCMPoint::P_At( const int layer, const int start, const int end,
                  const Point& p, CMPoint& result)
{
  HCUPoint ntt;
  CUPoint unit;
  bool resIsValid = false;

  for(int i = start; i <= end; i++ )
  {
    Get(layer, i, ntt);
    int ostart = ntt.GetOriginstart();
    int oend = ntt.GetOriginend();

    if( ((ostart < 0) && (oend < 0)) || (LayerSize(layer+1) == 0) )
    {
      unit = ntt.value;
      CUPoint resunit( false );
      unit.P_At( p, resunit );
      if( resunit.IsDefined() )
        result.Add( resunit );
      resIsValid = !result.IsEmpty();
    }
    else
    {
      resIsValid = P_At( layer+1, ostart, oend, p, result );
    }
  }
  return resIsValid;
}

/*
HCMPoint D\_At( const Region\& r, CMPoint\& result )

Checks, if the given Region-Value intersects the BoundingBox of this HCMPoint.
If so, it calls a recursive Function to determine the Units lying definitely-
at the given Region-value.

*/
void HCMPoint::P_At( const Region& r, CMPoint& result )
{
  assert( r.IsDefined() );
  assert( IsDefined() );

  result.Clear();
  if( !r.BoundingBox().Intersects(BBox2D()) )
    return;

  //bool resIsValid = false;
  int layer, size;
  GetFirstLayer(layer, size);

  //CMPoint& rescmpoint;
  result.StartBulkLoad();
  P_At( layer, 0, size-1, r, result );
  result.EndBulkLoad();

  // +++++ for debugging purposes only +++++
  //if(result.IsEmpty())
  //  cout << "The resulting CMPoint-Object is EMPTY!\n";

  //if( resIsValid && !rescmpoint.IsEmpty() )
  //  Generalize(rescmpoint, GetEpsilon(), GetFactor(), result);
}

/*
This recursive Function determines, by a pre-order run through the hierarchical
 structure, the HCMPoint-object that possibly-passes the given Region-value.

*/
bool HCMPoint::P_At( const int layer, const int start, const int end,
                  const Region& r, CMPoint& result )
{
  HCUPoint ntt;
  CUPoint unit;
  bool resIsValid = false;

  // +++++ for debugging purposes only +++++
  //cout << "Call for recursive Function HCMPoint::D_At with Arguments:\n";
  //cout << "     layer = " << layer << endl;
  //cout << "     start = " << start << endl;
  //cout << "     end = " << end << endl << endl;

  for(int i = start; i <= end; i++ )
  {
    Get(layer, i, ntt);
    unit = (ntt.value);
    int ostart = ntt.GetOriginstart();
    int oend = ntt.GetOriginend();

    if( ((ostart < 0) && (oend < 0)) || (LayerSize(layer+1) == 0) )
    {
      // +++++ for debugging purposes only +++++
      //  cout << "HCMPoint::P_At: Reached bottom layer!\n";

      CMPoint resultunits( 0 );
      CUPoint resunit;

      unit.P_At( r, resultunits );
      if( resultunits.IsDefined() )
      {
        // +++++ for debugging purposes only +++++
        //cout << "HCMPoint::P_At: resultunit for entity" << i
        //  << " has " << resultunits.GetNoComponents() <<" components."
        //  << endl;

        for(int j = 0; j < resultunits.GetNoComponents(); j++)
        {
          resultunits.Get( j, resunit );

          // +++++ for debugging purposes only +++++
          //cout << endl;
          //cout<< "HCMPoint::P_At(...): Add CUPoint to the result-CMPoint:\n";
          //resunit->Print(cout);

          result.Add( resunit );
        }
      }
      resIsValid = !result.IsEmpty();
    }
    else if( unit.P_Passes( r ) )
    {
      // +++++ for debugging purposes only +++++
      //cout << "HCMPoint::P_At: the unit of entity" << i << " on layer " <<
      //  layer << " passes the region.\n";

      resIsValid =  P_At( layer+1, ostart, oend, r, result );
    }
  }
  return resIsValid;
}

/*
 AtInstant( const Instant\& t )

Checks, if the given Instant-Value lies inside the BoundingBox of this
HCMPoint. If so, it calls a recursive Function to determine a circular shaped
temporalalgebra::Intime<Region>-Object related to the given Instant.

*/
void HCMPoint::AtInstant( const Instant& t, 
                          temporalalgebra::Intime<Region>& result )
{
  assert( t.IsDefined() );

  if( IsDefined() && t.IsDefined() )
  {
    if( !bbox.IsDefined() )
    { // result is undefined

      // +++++ for debugging purposes only +++++
      //cout << "HCMPoint::AtInstant: The bbox is UNDEFINED!\n";

      result.SetDefined(false);
    } else if( IsEmpty() )
    { // result is undefined

      // +++++ for debugging purposes only +++++
      //cout << "HCMPoint::AtInstant: The HCMPoint is EMPTY!\n";

      result.SetDefined(false);
    }
    else
    { // proove if the Instant lies inside the bbox
      double instd = t.ToDouble();
      double mind = bbox.MinD(2);
      double maxd = bbox.MaxD(2);
      if( (mind > instd && !AlmostEqual(mind,instd)) ||
           (maxd < instd && !AlmostEqual(maxd,instd)) )
        result.SetDefined(false);
      else
      {
        int layer, size; 
        GetFirstLayer(layer, size);

        // +++++ for debugging purposes only +++++
        //cout << "HCMPoint::AtInstant: Initial recursive call...\n";

        AtInstant( layer, 0, size-1, t, result );
      }
    }
  }
  else
    result.SetDefined(false);
}

/*
bool AtInstant( const int layer, const int start, const int end,
const Region\& r )

This recursive Function determines, by a Devide-and-Conquer-Search through the
hierarchical structure, a circular shaped 
temporalalgebra::Intime<Region>-Object, related to
the given Instant.

*/
int HCMPoint::AtInstant( const int layer, const int start, const int end,
                  const Instant& t, temporalalgebra::Intime<Region>& result )
{
  int pos = Position( layer, t, start, end );

  if( pos == -1 )  // not contained in any unit
  {
    // +++++ for debugging purposes only +++++
    //cout << "No Unit of layer " << layer << " between the indices " << start
    //  << " and " << end << " contains the demanded Time-Instant!\n";

    result.SetDefined( false );
  }
  else if(layer < 5 && LayerSize(layer+1) > 0)
  {
    HCUPoint ntt;
    Get(layer, pos, ntt);
    int ostart = ntt.GetOriginstart();
    int oend = ntt.GetOriginend();

    // +++++ for debugging purposes only +++++
    //cout << "Further recursive call for \n";
    //cout << "     layer : " << layer+1 << endl;
    //cout << "     ostart: " << ostart << endl;
    //cout << "     oend  : " << oend << "!\n";

    AtInstant( layer+1, ostart, oend, t, result ); // recursive call
  }
  else
  {
    CUPoint posUnit;
    Get(layer, pos, posUnit );
    result.SetDefined( true );
    Point respoint(false, 0, 0);
    posUnit.TemporalFunction( t, respoint );

    // If epsilon equals 0, set a minimal epsilon value greater 0 to ensure,
    // that a region-object is created!
    double e = posUnit.GetEpsilon();
    if( e == 0.0 )
    {
      e = fabs(posUnit.p0.GetX());
      if( fabs(posUnit.p1.GetX()) > e )
        e = fabs(posUnit.p1.GetX());
      if( fabs(posUnit.p0.GetY()) > e )
        e = fabs(posUnit.p0.GetY());
      if( fabs(posUnit.p1.GetY()) > e )
        e = fabs(posUnit.p1.GetY());
      e = e * minEpsilonLimiter;
    }

    // +++++ for debugging purposes only +++++
    //cout << "Create result-Circle...\n";

    Circle(respoint, e, 16, result.value);
    result.instant.CopyFrom( &t );
  }
  return 0;
}

/*
3.6.2 Functions to be part of relations

*/

inline ostream& HCMPoint::Print( ostream &os ) const
{
  if( !IsDefined() )
    return os << "(HCMPoint: undefined)";

  os << "(HCMPoint: defined, MBR = ";
  bbox.Print(os);
  os << "Initial epsilon-value = " << epsilon << ", Factor = " << factor
    << endl;
  os << " Epsilon-values of the 5 Layers = ( ";
  for( int i = 0; i < 5; i++)
    os << GetLayerepsilon(i) << " ";
  os << ")\n";
  os << ", contains " << GetNoComponents() << " entities: \n";
  int size;
  HCUPoint ntt;
  for(int i = 0; i < 5; i++)
  {
    os << "\t(\n";
    size = LayerSize(i);
    for(int j = 0; j < size; j++)
    {
      Get(i, j, ntt);
      os << "\n\t\t";
      ntt.Print(os);
    }
    os << "\t)\n";
  }
  os << "\n)" << endl;
  return os;
}

inline int HCMPoint::Compare( const Attribute *arg ) const
{
  const HCMPoint* hcmp2 = dynamic_cast<const HCMPoint*>(arg);
  size_t size1;
  size_t size2;
  size_t index;
  HCUPoint ntt1;
  HCUPoint ntt2;
  int cmp = 0;

  size1 = GetNoComponents();
  size2 = hcmp2->GetNoComponents();
  if( size1 < size2 )
    return -1;
  if( size1 > size2 )
    return 1;

  // compare the entities of each layer

  for(int i = 0; i < 5; i++)
  {
    size1 = LayerSize(i);
    size2 = hcmp2->LayerSize(i);
    if( size1 < size2 )
      return -1;
    if( size1 > size2 )
      return 1;
    index = 0;
    while( (index < size1) && (index < size2) )
    {
      Get(i, index, ntt1);
      hcmp2->Get(i, index, ntt2);
      cmp = ntt1.Compare(&ntt2);
      if(cmp) // different entities
        return cmp;
      index++;
    }
  }

  return 0;
}

inline Attribute* HCMPoint::Clone() const
{
  HCMPoint *result = new HCMPoint( 0 );

  if(GetNoComponents() > 0)
  {
    HCUPoint hcup;
    int size;

    for(int i = 0; i < 5; i++)
    {
      size = this->LayerSize(i);
      if(size > 0)
      {
        result->ResizeLayer( i, size );
        for( int j = 0; j < size; j++)
        {
          this->Get(i, j, hcup );
          HCUPoint ntt( hcup );
          result->Put(i,  j, ntt );
        }
        result->SetLayerepsilon( i, this->GetLayerepsilon(i) );
      }
    }
  }
  result->SetEpsilon(this->GetEpsilon());
  result->SetFactor(this->GetFactor());
  result->bbox = this->BoundingBox();

  // +++++ for debugging purposes only +++++
  cout << "HCMPoint::Clone(): The HCMPoint's boundingbox is defined as: ";
  result->BoundingBox().Print(cout);

  return result;
}

inline void HCMPoint::CopyFrom( const Attribute* right )
{
  const HCMPoint *hcmp = dynamic_cast<const HCMPoint*>(right);

  Clear();
  if(hcmp->GetNoComponents() > 0)
  {
    HCUPoint ntt;
    int size;

    for(int i = 0; i < 5; i++)
    {
      size = hcmp->LayerSize(i);
      if(size > 0)
      {
        ResizeLayer( i, size );
        for( int j = 0; j < size; j++)
        {
          hcmp->Get( i, j, ntt );
          HCUPoint hcup( ntt );
          Put( i, j, hcup );
        }
        SetLayerepsilon( i, hcmp->GetLayerepsilon(i) );
      }
      else
        SetLayerepsilon( i, -1.0 );
    }
    SetEpsilon(hcmp->GetEpsilon());
    SetFactor(hcmp->GetFactor());
    bbox = hcmp->BoundingBox();

    // +++++ for debugging purposes only +++++
    //cout << "HCMPoint::CopyFrom: The HCMPoint's boundingbox is defined as: ";
    //BoundingBox().Print(cout);
  }
}

inline Flob* HCMPoint::GetFLOB( const int i)
{
  assert( i > -1 );
  assert( i < NumOfFLOBs() );

  switch( i )
  {
    case 0:
      return &layer0;
    case 1:
      return &layer1;
    case 2:
      return &layer2;
    case 3:
      return &layer3;
    case 4:
      return &layer4;
    default:
      // This case is prevented by the assertion that i has to be less than 5!
      // layer4 is just returned to prevent a compiler-warning.
      return &layer4;
  }
}

/*
3.7 HMPoint

3.7.1 Member Functions

*/

void HMPoint::Get( const int layer, const int i, HCUPoint& ntt ) const
{
  assert( layer > -1 );
  assert( layer < 6 );

  if(layer == 5)
  {
    if( i > certainlayer.Size() )
      cout << "There are less than " << i << " elements in certainlayer!\n";
    else
      certainlayer.Get(i, ntt);
  }
  else
    HCMPoint::Get( layer, i, ntt );
}

void HMPoint::Get( const int layer, const int i, CUPoint& cup ) const
{
  HCUPoint ntt;
  Get( layer, i, ntt );
  cup = (ntt.value);
}

//void HMPoint::Get( const int layer, const int i, 
// temporalalgebra::UPoint const*& up ) const
//{
//  const HCUPoint* ntt;
//  Get( layer, i, ntt );
//  up = (temporalalgebra::UPoint*)&(ntt->value);
//}

void HMPoint::Get( const int i, HCUPoint& ntt ) const
{
  assert( i < GetNoComponents() );
  int idx = i;

  if( idx < layer0.Size() )
  {
    layer0.Get( idx, ntt );
    return;
  }
  idx -= layer0.Size();

  if( idx < layer1.Size() )
  {
    layer1.Get( idx, ntt );
    return;
  }
  idx -= layer1.Size();

  if( idx < layer2.Size() )
  {
    layer2.Get( idx, ntt );
    return;
  }
  idx -= layer2.Size();

  if( idx < layer3.Size() )
  {
    layer3.Get( idx, ntt );
    return;
  }
  idx -= layer3.Size();

  if( idx < layer4.Size() )
  {
    layer4.Get( idx, ntt );
    return;
  }
  idx -= layer4.Size();

  certainlayer.Get( idx, ntt );

  if ( !ntt.IsDefined() )
  {
    cout << __FILE__ << "," << __LINE__ << ":" << __PRETTY_FUNCTION__
         << " Get(" << i << ", Entity): Entity is undefined:";
    ntt.Print(cout); cout << endl;
    assert( false );
  }
  if ( !ntt.IsValid() )
  {
    cout << __FILE__ << "," << __LINE__ << ":" << __PRETTY_FUNCTION__
      << " Get(" << i << ", Entity): Entity is invalid:";
    ntt.Print(cout); cout << endl;
    assert( false );
  }
}

void HMPoint::GetCMPoint( const double epsilon, CMPoint& result )
{
  assert( !IsEmpty() );

  result.Clear();
  // Determine the first layer, whose epsilon-value is less than the wanted
  // epsilon-value:
  if( layer0epsilon > 0 && epsilon >= layer0epsilon )
  {
    result.Resize( layer0.Size() );
    HCUPoint ntt;
    result.StartBulkLoad();
    for(int i = 0; i < layer0.Size(); i++)
    {
      layer0.Get( i, ntt );
      CUPoint resunit( ntt.value );
      result.Add( resunit );
    }
    result.EndBulkLoad();
  }
  else if( layer1epsilon > 0 && epsilon >= layer1epsilon )
  {
    result.Resize( layer1.Size() );
    HCUPoint ntt;
    result.StartBulkLoad();
    for(int i = 0; i < layer1.Size(); i++)
    {
      layer1.Get( i, ntt );
      CUPoint resunit( ntt.value );
      result.Add( resunit );
    }
    result.EndBulkLoad();
  }
  else if( layer2epsilon > 0 && epsilon >= layer2epsilon )
  {
    result.Resize( layer2.Size() );
    HCUPoint ntt;
    result.StartBulkLoad();
    for(int i = 0; i < layer2.Size(); i++)
    {
      layer2.Get( i, ntt );
      CUPoint resunit( ntt.value );
      result.Add( resunit );
    }
    result.EndBulkLoad();
  }
  else if( layer3epsilon > 0 && epsilon >= layer3epsilon )
  {
    result.Resize( layer3.Size() );
    HCUPoint ntt;
    result.StartBulkLoad();
    for(int i = 0; i < layer3.Size(); i++)
    {
      layer3.Get( i, ntt );
      CUPoint resunit( ntt.value );
      result.Add( resunit );
    }
    result.EndBulkLoad();
  }
  else if( layer4epsilon > 0 && epsilon >= layer4epsilon )
  {
    result.Resize( layer4.Size() );
    HCUPoint ntt;
    result.StartBulkLoad();
    for(int i = 0; i < layer4.Size(); i++)
    {
      layer4.Get( i, ntt );
      CUPoint resunit( ntt.value );
      result.Add( resunit );
    }
    result.EndBulkLoad();
  }
  else
  {
    result.Resize( certainlayer.Size() );
    HCUPoint ntt;
    result.StartBulkLoad();
    for(int i = 0; i < certainlayer.Size(); i++)
    {
      certainlayer.Get( i, ntt );
      CUPoint resunit( ntt.value );
      result.Add( resunit );
    }
    result.EndBulkLoad();
  }
}

void HMPoint::GetMPoint( temporalalgebra::MPoint& result )
{
  if( certainlayer.Size() == 0 )
  {
    result.SetDefined(false);
    return;
  }

  result.Clear();
  result.Resize( certainlayer.Size() );
  HCUPoint ntt;
  temporalalgebra::UPoint resunit(false);
  result.StartBulkLoad();
  for(int i = 0; i < certainlayer.Size(); i++)
  {
    certainlayer.Get( i, ntt );
    resunit = static_cast<temporalalgebra::UPoint>(ntt.value);
    result.Add( resunit );
  }
  result.EndBulkLoad();
}

void HMPoint::Put( const int layer, const int i, HCUPoint& ntt )
{
  assert( layer > -1 );
  assert( layer < 6 );

  if( layer == 5 )
  {
    if( i > certainlayer.Size() )
      certainlayer.resize( i );
    certainlayer.Put(i, ntt);
  }
  else
    HCMPoint::Put( layer, i, ntt );
}

void HMPoint::Add( const HCUPoint& hcup )
{
  if ( !hcup.IsDefined() )
  {
    cout << __FILE__ << "," << __LINE__ << ":" << __PRETTY_FUNCTION__
      << " Add(hcupoint): Entity is undefined:";
    hcup.Print(cout); cout << endl;
    assert( false );
  }
  if ( !hcup.IsValid() )
  {
    cout << __FILE__ << "," << __LINE__ << ":" << __PRETTY_FUNCTION__
      << " Add(hcupoint): Entity is invalid:";
    hcup.Print(cout); cout << endl;
    assert( false );
  }
  const int layerno = hcup.GetLayer();

  switch ( layerno )
  {
    case 0:
      if( layer0.Size() < hcup.GetIndex() )
        layer0.resize( hcup.GetIndex() + 1 );
      layer0.Put( hcup.GetIndex(), hcup );
      break;

    case 1:
      if( layer1.Size() < hcup.GetIndex() )
        layer1.resize( hcup.GetIndex() + 1 );
      layer1.Put( hcup.GetIndex(), hcup );
      break;

    case 2:
      if( layer2.Size() < hcup.GetIndex() )
        layer2.resize( hcup.GetIndex() + 1 );
      layer2.Put( hcup.GetIndex(), hcup );
      break;

    case 3:
      if( layer3.Size() < hcup.GetIndex() )
        layer3.resize( hcup.GetIndex() + 1 );
      layer3.Put( hcup.GetIndex(), hcup );
      break;

    case 4:
      if( layer4.Size() < hcup.GetIndex() )
        layer4.resize( hcup.GetIndex() + 1 );
      layer4.Put( hcup.GetIndex(), hcup );
      break;

    case 5:
      if( certainlayer.Size() < hcup.GetIndex() )
        certainlayer.resize( hcup.GetIndex() + 1 );
      certainlayer.Put( hcup.GetIndex(), hcup );
      break;

    default:
      cout << __FILE__ << "," << __LINE__ << ":" << __PRETTY_FUNCTION__
      << " Add(hcupoint): invalid layer (" << hcup.GetLayer() << "):";
      hcup.Print(cout); cout << endl;
      assert( false );
      break;
  }
}

inline int HMPoint::LayerSize( const int layer ) const
{
  if( layer == 5 )
    return certainlayer.Size();
  else
    return HCMPoint::LayerSize( layer );
}

void HMPoint::ResizeLayer( const int layer, const int n )
{
  if( layer == 5 )
    certainlayer.resize( n );
  else
    HCMPoint::ResizeLayer( layer, n ); // call super
}

int HMPoint::Position( int layer, const Instant& t, const int start,
                        const int end )
{
  assert( t.IsDefined() );
  assert( start >= 0 && end < LayerSize(layer) );

  Instant t1( t );
  int first = start;
  int last = end;

  // +++++ for debugging purposes only +++++
  //cout << "HCMPoint::Position: Start while- with 'first' = " << first
  //  << " and 'last' = " << last << endl;

  while (first <= last)
  {
    int mid = ( first + last ) / 2;

    if( (mid < start) || (mid > end) )
      return -1;

    CUPoint midUnit;
    Get( layer, mid, midUnit );

    // +++++ for debugging purposes only +++++
    //cout << "Prove CUPoint No. " << mid << ":\n";
    //midUnit->Print(cout);

    if( midUnit.timeInterval.Contains(t1) )
    {
      // +++++ for debugging purposes only +++++
      //cout << "CUPoint No. " << mid << "contains the Time-Instant.\n";

      return mid;
    }
    else  //not contained
      if( ( t1 > midUnit.timeInterval.end ) ||
          ( t1 == midUnit.timeInterval.end ) )
      {
        first = mid + 1;

        // +++++ for debugging purposes only +++++
        //cout << "The Time-Instant is GREATER than the CUPoint's "
        //  << "timeInterval.end. 'first' is set to " << first << "!\n";
      }
      else if( ( t1 < midUnit.timeInterval.start ) ||
               ( t1 == midUnit.timeInterval.start ) )
      {
        last = mid - 1;
        // +++++ for debugging purposes only +++++
        //cout << "The Time-Instant is LESS than the CUPoint's "
        //  << "timeInterval.end. 'last' is set to " << last << "!\n";
      }
      else
        return -1; //should never be reached.
    }
    return -1;
}


int HMPoint::Generalize(const int layer, const bool checkBreakPoints,
                          const DateTime dur)
{
  const int origlayerno = layer;
  const int genlayerno = layer - 1;
  double *layerepsilon;
  double aktepsilon;
  int size;

  switch( layer )
  {
    case 1:
      // +++++ for debugging purposes only +++++
      //cout << "Computing of CUPoints for layer 0!\n";

      layerepsilon = &layer0epsilon;
      size = layer1.Size();
      if( size < 2 )
        return false;
      aktepsilon = epsilon * factor * factor * factor * factor;
      break;

    case 2:
      // +++++ for debugging purposes only +++++
      //cout << "Computing of CUPoints for layer 1!\n";

      layerepsilon = &layer1epsilon;
      size = layer2.Size();
      if( size < 2 )
        return false;
      aktepsilon = epsilon * factor * factor * factor;
      break;

    case 3:
      // +++++ for debugging purposes only +++++
      //cout << "Computing of CUPoints for layer 2!\n";

      layerepsilon = &layer2epsilon;
      size = layer3.Size();
      if( size < 2 )
        return false;
      aktepsilon = epsilon * factor * factor;
      break;

    case 4:
      // +++++ for debugging purposes only +++++
      //cout << "Computing of CUPoints for layer 3!\n";

      layerepsilon = &layer3epsilon;
      size = layer4.Size();
      if( size < 2 )
        return false;
      aktepsilon = epsilon * factor;
      break;

    case 5:
      // +++++ for debugging purposes only +++++
      //cout << "Computing of CUPoints for layer 4!\n";

      layerepsilon = &layer4epsilon;
      size = certainlayer.Size();
      if( size < 2 )
      {
        cout << "The mpoint has only 1 element!";
        return false;
      }
      aktepsilon = epsilon;
      break;

    default:
      // should never been reached!
      break;
  }

  // +++++ for debugging purposes only +++++
  //cout << "Control Variable-Initialization for next Generalization:\n";
  //cout << "     origlayerno       = " << origlayerno << endl;
  //cout << "     genlayerno        = " << genlayerno << endl;
  //cout << "     size of origlayer = " << size << endl;
  //cout << "     aktepsilon        = " << aktepsilon << endl << endl;

  // Create two bitmaps and a double array to keep in mind which sample point
  // belongs to the result and which real epsilon-value is computed for this
  // results cupoint.
  bool useleft[size];
  bool useright[size];
  double realepsilon[size];

  // Initialize the whole bitmaps to state FALSE and the double array to
  // value -1;
  for(int i = 0; i < size; i++)
  {
    useleft[i] = false;
    useright[i] = false;
    realepsilon[i] = -1;
  }

  int first = 0;
  int last = 1;
  CUPoint u1;
  CUPoint u2;
  ;

  while(last < size )
  {
    // for this purpose, only the upoint-part of the cupoint is needed:
    Get(origlayerno, last-1, u1);
    Get(origlayerno, last, u2);

    // check whether one of the cupoints is a constant cupoint and whether
    // last and last-1 are connected
    if( checkBreakPoints && IsBreakPoint(&u1,dur))
    {
      if(last-1 > first){
         Simplify(first, last-2, origlayerno, useleft, useright, realepsilon,
                   aktepsilon);
      }
      Simplify(last-1, last-1, origlayerno, useleft, useright, realepsilon,
                aktepsilon);
      first = last;
      last++;
    }
    else if( checkBreakPoints && IsBreakPoint(&u2,dur))
    {
      Simplify(first, last-1, origlayerno, useleft, useright, realepsilon,
                aktepsilon);
      last++;
      Simplify(last-1, last-1, origlayerno, useleft, useright, realepsilon,
                aktepsilon);
      first = last;
      last++;
    }
    else if(connected(&u1,&u2)) // enlarge the sequence
      last++;
    else
    {
      // +++++ for debugging purposes only +++++
      //cout << "The Units u1 = ";
      //u1->Print(cout);
      //cout << " and u2 = ";
      //u2->Print(cout);
      //cout << " are not connected with each other!\n";
      //cout << "Function Simplify(...) is called with parameters: \n";
      //cout << "     first =       " << first << endl;
      //cout << "     last-1 =      " << last-1 << endl;
      //cout << "     origlayerno = " << origlayerno << endl;
      //cout << "     aktepsilon =     " << aktepsilon << endl << endl;

      Simplify(first, last-1, origlayerno, useleft, useright, realepsilon,
                aktepsilon);
      first=last;
      last++;
    }
  }
  // process the last recognized sequence
  Simplify(first,last-1,origlayerno,useleft,useright,realepsilon,aktepsilon);

  // +++++ for debugging purposes only +++++
  //cout << "\nlast recognized sequence of units: \n";
  //cout << "Function Simplify(...) is called with parameters: \n";
  //cout << "     first =       " << first << endl;
  //cout << "     last-1 =      " << last-1 << endl;
  //cout << "     origlayerno = " << origlayerno << endl;
  //cout << "     epsilon =     " << aktepsilon << endl << endl;


  // count the number of remaining units
  int count = 1; // count the most right sample point
  for(int i = 0; i < size; i++)
  {
    if( realepsilon[i] > *layerepsilon )
      *layerepsilon = realepsilon[i];
    if( useleft[i] )
      count++;
  }
  ResizeLayer(genlayerno, count);

  // +++++ for debugging purposes only +++++
  //cout << "The Simplfiy()-Calls detected " << count << " for the next"
  //  " layer! \n";
  //cout << "The epsilon value of layer " << genlayerno << " has been set to "
  //  << *layerepsilon << endl;

  Instant start(instanttype);
  Point p0(false, 0, 0);
  bool closeLeft;
  //bool leftDefined = false;
  int generalizedby = 0;
  int originstart = 0;

  for(int i = 0; i < size; i++)
  {
    HCUPoint hcup;
    CUPoint cup;

    Get(origlayerno, i, hcup);
    HCUPoint aux( hcup );
    cup = &(aux.value);
    aux.SetGeneralizedby( generalizedby );
    Put(origlayerno, i, aux);

    if( useleft[i] )
    {
      // +++++ for debugging purposes only +++++
      //if(leftDefined)
      //  cout << " error in mpoint simplification,"
      //       << " overwrite an existing leftPoint "  << endl;

      originstart = i;
      p0 = cup.p0;
      closeLeft = cup.timeInterval.lc;
      start = cup.timeInterval.start;
      //leftDefined = true;
    }
    if( useright[i] )
    {
      // +++++ for debugging purposes only +++++
      //if(!leftDefined)
      //  cout << " error in mpoint simplification,"
      //       << " rightdefined before leftdefined "  << endl;

      temporalalgebra::Interval<Instant> interval( 
                          start, cup.timeInterval.end, closeLeft,
                                  cup.timeInterval.rc);
      CUPoint newCUPoint(realepsilon[i], interval, p0, cup.p1);
      // Create a new Entity for the hierarchical structure, containing this
      // new CUPoint.
      HCUPoint newHCUPoint(newCUPoint,genlayerno,generalizedby,originstart,i);
      Put(genlayerno, generalizedby, newHCUPoint);

      // +++++ for debugging purposes only +++++
      //cout << "A new HCUPoint with index " << generalizedby
      //  << " has been created in layer " << genlayerno << "\n";
      //newHCUPoint.Print(cout);
      //cout << endl;

      // The next cup belongs to the next origin-hcup belongs to the next
      // generalization.
      generalizedby++;
      //leftDefined = false;
    }
  }
  return count;
}

void HMPoint::Simplify(const int min, const int max, const int layer,
                    bool* useleft, bool* useright, double* realepsilon,
                    const double epsilon)
{

  // the endpoints are used in each case
  useleft[min] = true;
  useright[max] = true;



  if(min==max) // no intermediate sampling points -> nothing to simplify
  {
    CUPoint cup;
    Get(layer, max, cup);
    realepsilon[max] = cup.GetEpsilon();
    return;
  }

  CUPoint u1;
  CUPoint u2;
  // build a HalfSegment from the endpoints
  Get(layer, min, u1);
  Get(layer, max, u2);
  HalfSegment hs(true, u1.p0, u2.p1);

  // search for the point with the highest distance to its simplified position
  double maxDist = 0;
  int maxIndex=0;
  Point p_orig(false, 0, 0);
  //Point p_simple;
  //const HCUPoint* hcup;
  CUPoint u;
  double distance;
  for(int i=min+1;i<=max;i++){
     Get(layer, i,u);
     //upoint.TemporalFunction(u->timeInterval.start,p_simple, true);
     distance  = hs.Distance(u.p0);
     if(distance>maxDist){ // new maximum found
        maxDist = distance;
        maxIndex = i;
     }
  }

  if(maxIndex==0){  // no difference found
    realepsilon[max] = maxDist;
    return;
  }
  if(maxDist<=epsilon){  // differnce is in allowed range
      realepsilon[max] = maxDist;
      return;
  }

  // split at the left point of maxIndex
  Simplify(min,maxIndex-1,layer,useleft,useright,realepsilon,epsilon);
  Simplify(maxIndex,max,layer,useleft,useright,realepsilon,epsilon);
}

/*
ReduceHierarchy

Creates a new HCMPoint-Object from this HMPoint. This new Object contains only
the layers down to that epsilon-value, that is necessary to allow queries with
the given epsilon-value. (So in most cases, the uncertainty-value of the most
certain layer is a bit smaller than the given epsilon-value.

*/
void HMPoint::ReduceHierarchy( const double e, HCMPoint& result )
{
  assert( !IsEmpty() );
  if(e < GetLayer4epsilon() )
  {
    cout << "The given Uncertainty-Value is too small! There is nothing to"
      " reduce!\n";
    cerr << "The given Uncertainty-Value is too small! There is nothing to"
      " reduce!\n";
    return;
  }

  result.Clear();
  HCUPoint ntt;
  int size;
  int i = 0;
  // copy layers with epsilon greater e
  while( i < 5 )
  {
    size = LayerSize(i);
    if( size > 0 )
    {
      if( GetLayerepsilon(i) <= e )
        break;
      result.ResizeLayer( i, size );
      for(int j = 0; j < size; j++)
      {
        Get(i, j, ntt);
        HCUPoint hcup(ntt);
        result.Put(i, j, hcup);
      }
    }
    result.SetLayerepsilon( i, GetLayerepsilon(i) );
    i++;
  }
  // Copy the necessary most certain layer and set the indices of originstart
  // and originend to -1.
  size = LayerSize(i);
  if( size > 0 )
  {
    result.ResizeLayer( i, size );
    for(int j = 0; j < size; j++)
    {
      Get(i, j, ntt);
      HCUPoint hcup(ntt);
      hcup.SetOriginstart(-1);
      hcup.SetOriginend(-1);
      result.Put(i, j, hcup);
    }
  }
  result.SetEpsilon( GetEpsilon() );
  result.SetFactor( GetFactor() );
  result.SetLayerepsilon( i, GetLayerepsilon(i) );
  i++;
  while( i < 5 )
  {
    result.SetLayerepsilon( i, -1 );
    i++;
  }
  result.RestoreBoundingBox();
}


/*
bool D\_Passes( const Point\& p )

Checks, if the given Point-Value lies inside the BoundingBox of this HCMPoint.
If so, it calls a recursive Function to determine if the HCMPoint definitely-
passes the given Point-value.

*/
bool HMPoint::D_Passes( const Point& p )
{
  assert( p.IsDefined() );
  assert( IsDefined() );

  if( !p.Inside(BBox2D()) )
    return false;

  bool result = false;
  int layer, size;
  GetFirstLayer(layer, size);

  result = D_Passes( layer, 0, size-1, p );
  return result;
}

/*
bool D\_Passes( const int layer, const int start, const int end,
const Point\& p )

This recursive function determines, by a pre-order run through the hierarchical
 structure, if the HMPoint definitely-passes the given Point-value.

*/
bool HMPoint::D_Passes( const int layer, const int start, const int end,
                  const Point& p )
{
  // +++++ for debugging purposes only +++++
  //cout << "======> Call for D_Passes ( " << layer << ", " << start << ", "
  //  << end << " ) \n";

  bool result = false;
  HCUPoint ntt;
  CUPoint cup;

  if( layer == 5 )
  {
    // +++++ for debugging purposes only +++++
    //cout << "Reached certain layer!\n";

    for(int i = start; i <= end; i++)
    {
      // +++++ for debugging purposes only +++++
      //cout << "Proove Unit " << i << "...\n";

      Get(layer, i, cup);
      // +++++ for debugging purposes only +++++
      //cout << "Proove Unit ";
      //cup->Print(cout);
      //cout << endl;

      if( cup.D_Passes( p ) )
      {
        // +++++ for debugging purposes only +++++
        //cout << "u->Passes( p ) returned TRUE for Unit " << i << "!\n";

        return true;
      }
    }
  }
  else
  {
    for(int i = start; i <= end; i++ )
    {
      // +++++ for debugging purposes only +++++
      //cout << "Proove Unit " << i << "...\n";

      Get(layer, i, ntt);
      cup = (ntt.value);
      if( (cup.GetEpsilon() == 0.0) && cup.D_Passes( p ) )
      {
        // +++++ for debugging purposes only +++++
        //cout << "u->Passes( p ) returned TRUE for Unit " << i << " on Layer "
        //  << layer << "!\n";

        return true;
      }
      if( cup.P_Passes( p ) && layer < 5 )
      {
        // +++++ for debugging purposes only +++++
        //cout << "Recursive call for D_Passes( " << layer+1 << ", "
        // << ntt->GetOriginstart() << ", " << ntt->GetOriginend() << " ) !\n";

        result = D_Passes( layer+1, ntt.GetOriginstart(), ntt.GetOriginend(),
                            p );
      }
      else
      {
        // +++++ for debugging purposes only +++++
        //cout << "P_Passes(...) for unit " << i << " returned FALSE!\n";
      }
      if( result )
        return true;
    }
  }
  return false;
}

/*
bool D\_Passes( const Region\& r )

Checks, if the given Region-Value intersects the BoundingBox of this HCMPoint.
If so, it calls a recursive Function to determine if the HCMPoint definitely-
passes the given Region-value.

*/
bool HMPoint::D_Passes( const Region& r )
{
  // +++++ for debugging purposes only +++++
  cout << "call for HMPoint::D_Passes( ... Region )\n";

  assert( r.IsDefined() );
  assert( IsDefined() );

  if( !r.BoundingBox().Intersects(BBox2D()) )
    return false;

  bool result = false;
  int layer, size;
  GetFirstLayer(layer, size);

  result = D_Passes( layer, 0, size-1, r );
  return result;
}

/*
bool D\_Passes( const int layer, const int start, const int end,
const Region\& r )

This recursive Function determines, by an in-order run through the hierarchical
 structure, if the HCMPoint definitely-passes the given Region-value.

*/
bool HMPoint::D_Passes( const int layer, const int start, const int end,
                  const Region& r )
{
  // +++++ for debugging purposes only +++++
  //cout << "call for recursive function HMPoint::D_Passes( ... Region )\n";


  bool result = false;
  HCUPoint ntt;
  CUPoint cup;

  for(int i = start; i <= end; i++ )
  {
    Get(layer, i, cup);
    if( cup.D_Passes( r ) )
    {
      // +++++ for debugging purposes only +++++
      //cout << "D_Passes( region ) returned TRUE for Unit " << i <<
      //  " on Layer " << layer << "!\n";

      return true;
    }
  }
  for(int i = start; i <= end; i++ )
  {
    Get(layer, i, ntt);
    cup = (ntt.value);
    if( cup.P_Passes( r ) && layer < 5 )
    {
      // +++++ for debugging purposes only +++++
      //cout << "recursive call for D_Passes( " << layer+1 << ", "
      //  <<ntt->GetOriginstart()<<", "<<ntt->GetOriginend()<<", "<< " )\n\n";

      result = D_Passes( layer+1, ntt.GetOriginstart(), ntt.GetOriginend(),
                          r );
    }
    if( result )
      return true;
  }
  return false;
}

/*
Checks, if the given Point-Value lies inside the BoundingBox of this HCMPoint.
If so, it calls a recursive Function to determine the HCMPoint definitely-
at the given Point-value.

*/
void HMPoint::D_At( const Point& p, temporalalgebra::MPoint& result )
{
  assert( p.IsDefined() );
  assert( IsDefined() );

  result.Clear();
  if( !p.Inside(BBox2D()) )
    return;

  //bool resIsValid = false;
  int layer, size;
  GetFirstLayer(layer, size);

  result.StartBulkLoad();
  D_At( layer, 0, size-1, p, result);
  result.EndBulkLoad();

  // +++++ for debugging purposes only +++++
  //if(result.IsEmpty())
  //  cout << "The resulting CMPoint-Object is EMPTY!\n";

}

/*
This recursive function determines, by a pre-order run through the hierarchical
 structure, the HCMPoint definitely-at the given Point-value.

*/
bool HMPoint::D_At( const int layer, const int start, const int end,
                  const Point& p, temporalalgebra::MPoint& result)
{
  HCUPoint ntt;
  CUPoint unit;
  bool resIsValid = false;

  for(int i = start; i <= end; i++ )
  {
    Get(layer, i, ntt);
    int ostart = ntt.GetOriginstart();
    int oend = ntt.GetOriginend();

    if( ((ostart < 0) && (oend < 0)) || LayerSize(layer+1) == 0 )
    {
      unit = (ntt.value);
      CUPoint resunit( false );

      unit.D_At( p, resunit );
      if( resunit.IsDefined() )
        result.Add( static_cast<temporalalgebra::UPoint>(resunit) );
      resIsValid = !result.IsEmpty();
    }
    else if( ntt.value.P_Passes(p) )
      resIsValid = D_At( layer+1, ostart, oend, p, result ); //recursive call
  }
  return resIsValid;
}

/*
HCMPoint D\_At( const Region\& r, CMPoint\& result )

Checks, if the given Region-Value intersects the BoundingBox of this HCMPoint.
If so, it calls a recursive Function to determine the Units lying definitely-
at the given Region-value.

*/
void HMPoint::D_At( const Region& r, temporalalgebra::MPoint& result )
{
  assert( r.IsDefined() );
  assert( IsDefined() );

  result.Clear();
  if( !r.BoundingBox().Intersects(BBox2D()) )
    return;

  //bool resIsValid = false;
  int layer, size;
  GetFirstLayer(layer, size);

  //CMPoint& rescmpoint;
  result.StartBulkLoad();
  D_At( layer, 0, size-1, r, result );
  result.EndBulkLoad();

  // +++++ for debugging purposes only +++++
  //if(result.IsEmpty())
  //  cout << "The resulting CMPoint-Object is EMPTY!\n";

  //if( resIsValid && !rescmpoint.IsEmpty() )
  //  Generalize(rescmpoint, GetEpsilon(), GetFactor(), result);
}

/*
This recursive Function determines, by a pre-order run through the hierarchical
 structure, if the HCMPoint definitely-passes the given Region-value.

*/
bool HMPoint::D_At( const int layer, const int start, const int end,
                  const Region& r, temporalalgebra::MPoint& result )
{
  HCUPoint ntt;
  CUPoint unit;
  bool resIsValid = false;

  // +++++ for debugging purposes only +++++
  //cout << "Call for recursive Function HCMPoint::D_At with Arguments:\n";
  //cout << "     layer = " << layer << endl;
  //cout << "     start = " << start << endl;
  //cout << "     end = " << end << endl << endl;

  for(int i = start; i <= end; i++ )
  {
    Get(layer, i, ntt);
    unit = (ntt.value);
    int ostart = ntt.GetOriginstart();
    int oend = ntt.GetOriginend();

    if( ((ostart < 0) && (oend < 0)) || (LayerSize(layer+1) == 0) )
    {
      // +++++ for debugging purposes only +++++
      //  cout << "HCMPoint::D_At: Reached bottom layer!\n";

      CMPoint resultunits( 0 );
      CUPoint resunit;

      unit.D_At( r, resultunits );
      if( resultunits.IsDefined() )
      {
        // +++++ for debugging purposes only +++++
        //cout << "HCMPoint::D_At: resultunit for entity" << i
        //  << " has " << resultunits.GetNoComponents() <<" components."
        //  << endl;

        for(int j = 0; j < resultunits.GetNoComponents(); j++)
        {
          resultunits.Get( j, resunit );

          // +++++ for debugging purposes only +++++
          //cout << endl;
          //cout<< "HCMPoint::D_At(...): Add CUPoint to the result-CMPoint:\n";
          //resunit->Print(cout);

          result.Add( static_cast<temporalalgebra::UPoint>(resunit) );
        }
      }
      resIsValid = !result.IsEmpty();
    }
    else if( unit.P_Passes( r ) )
    {
      // +++++ for debugging purposes only +++++
      //cout << "HCMPoint::D_At: the unit of entity" << i << " on layer " <<
      //  layer << " passes the region.\n";

      resIsValid =  D_At( layer+1, ostart, oend, r, result );
    }
  }
  return resIsValid;
}


/*
 AtInstant( const Instant\& t )

Checks, if the given Instant-Value lies inside the BoundingBox of this
HCMPoint. If so, it calls a recursive Function to determine a circular shaped
temporalalgebra::Intime<Region>-Object related to the given Instant.

*/
void HMPoint::AtInstant( const Instant& t, 
                         temporalalgebra::Intime<Point>& result )
{
  assert( t.IsDefined() );

  if( IsDefined() && t.IsDefined() )
  {
    if( !bbox.IsDefined() )
    { // result is undefined
      result.SetDefined(false);
    } else if( IsEmpty() )
    { // result is undefined
      result.SetDefined(false);
    }
    else
    { // proove if the Instant lies inside the bbox
      double instd = t.ToDouble();
      double mind = bbox.MinD(2);
      double maxd = bbox.MaxD(2);
      if( (mind > instd && !AlmostEqual(mind,instd)) ||
           (maxd < instd && !AlmostEqual(maxd,instd)) )
        result.SetDefined(false);
      else
      {
        int layer, size;
        GetFirstLayer(layer, size);
        AtInstant( layer, 0, size-1, t, result );
      }
    }
  }
  else
    result.SetDefined(false);
}

/*
bool AtInstant( const int layer, const int start, const int end,
const Region\& r )

This recursive Function determines, by a Devide-and-Conquer-Search through the
hierarchical structure, a circular shaped 
temporalalgebra::Intime<Region>-Object, related to
the given Instant.

*/
int HMPoint::AtInstant( const int layer, const int start, const int end,
                  const Instant& t, temporalalgebra::Intime<Point>& result )
{
  int pos = Position( layer, t, start, end );

  if( pos == -1 )  // not contained in any unit
    result.SetDefined( false );
  else if(layer == 5 )
  {
    // +++++ for debugging purposes only +++++
    //cout << "Reached layer 5, create ipoint... \n";

    CUPoint posUnit;
    Get(layer, pos, posUnit );
    result.SetDefined( true );
    posUnit.TemporalFunction( t, result.value );
    result.instant.CopyFrom( &t );
  }
  else
  {
    HCUPoint ntt;
    Get(layer, pos, ntt);
    int ostart = ntt.GetOriginstart();
    int oend = ntt.GetOriginend();

    // +++++ for debugging purposes only +++++
    //cout << "Further recursive call for \n";
    //cout << "     layer : " << layer+1 << endl;
    //cout << "     ostart: " << ostart << endl;
    //cout << "     oend  : " << oend << "!\n";

    AtInstant( layer+1, ostart, oend, t, result ); // recursive call
  }
  return 0;
}

/*
AtPeriods( const temporalalgebra::Periods\& per, HMPoint\& result )

Computes, by a recursive in-order run, for every layer of this hmpoint-object
the intersection of the hmpoint to the given periods and returns the
intersection-set as a new hmpoint.

*/
void HMPoint::AtPeriods( const temporalalgebra::Periods& p, 
                         temporalalgebra::MPoint& result )
{
  assert( p.IsDefined() );
  assert( p.IsOrdered() );

  result.Clear();

  if( !IsDefined() ) // the result hmpoint stays empty!
    return;

  if( !bbox.IsDefined() || IsEmpty() ) // the result hmpoint stays empty!
    return;

  Instant perMinInst(instanttype); p.Minimum(perMinInst);
  Instant perMaxInst(instanttype); p.Maximum(perMaxInst);
  double permind = perMinInst.ToDouble();
  double permaxd = perMaxInst.ToDouble();
  double mind = bbox.MinD(2);
  double maxd = bbox.MaxD(2);
  if( (mind > permaxd && !AlmostEqual(mind,permaxd)) ||
      (maxd < permind && !AlmostEqual(maxd,permind)) )
    return;

  int layer, size;
  GetFirstLayer(layer, size);
  result.StartBulkLoad();
  AtPeriods( layer, 0, size-1, p, result );
  result.EndBulkLoad();
}

/*
bool AtInstant( const int layer, const int start, const int end,
const Region\& r )

This recursive Function determines, by a Devide-and-Conquer-Search through the
hierarchical structure, a circular shaped 
temporalalgebra::Intime<Region>-Object, related to
the given Instant.

*/
int HMPoint::AtPeriods( const int layer, const int start, const int end,
                  const temporalalgebra::Periods& p, 
                  temporalalgebra::MPoint& result )
{
  temporalalgebra::Interval<Instant> interval;
  HCUPoint ntt;
  int i = start, j = 0;
  //int layerSize = LayerSize(layer);
  Get(layer, i, ntt );
  CUPoint unit( ntt.value );
  p.Get( j, interval );
  temporalalgebra::Periods auxPeriods( 0 );

  while( true )
  {
    if( unit.timeInterval.Before( interval ) )
    {
      if( auxPeriods.GetNoComponents() > 0 )
      {
        int ostart = ntt.GetOriginstart();
        int oend = ntt.GetOriginend();
        if(layer < 5 && ostart > -1 && oend > -1)
        {
          AtPeriods(layer+1, ostart, oend, auxPeriods, result);
          auxPeriods.Clear();
        }
      }
      if(++i > end )
        break;
      Get( layer, i, ntt );
      unit = ntt.value;
    }
    else if( interval.Before( unit.timeInterval ) )
    {
      if(++j == p.GetNoComponents() )
        break;
      p.Get( j, interval );
    }
    else
    { // the actual intervals overlap each other

      // +++++ for debugging purposes only +++++
      //cout << "On layer " << layer << " the interval ";
      //interval->Print(cout);
      //cout << endl << "  overlaps the timeInterval of Unit " << i << "!\n";
      //cout << "    ";
      //unit.timeInterval.Print(cout);
      //cout << endl << endl;


      if(layer == 5)
      {
        CUPoint r( false );
        unit.AtInterval( interval, r );
        r.epsilon = 0.0;
        r.UncertainSetDefined(true);

        assert( r.IsDefined() );
        assert( r.IsValid() );

        // +++++ for debugging purposes only +++++
        //cout << "Add new Unit with timeInterval = ";
        //r.timeInterval.Print(cout);
        //cout << endl << endl;

        result.Add( r );
      }
      else
      {
        auxPeriods.Add( interval );

        // +++++ for debugging purposes only +++++
        //cout << "Add the periods-interval to 'auxPeriods'!\n";
        //cout << "auxPeriods for layer " << layer+1 << " has "
        //  << auxPeriods.GetNoComponents() << " components.\n" << endl;
      }

      if( interval.end == unit.timeInterval.end )
      { // same ending instant
        if( interval.rc == unit.timeInterval.rc )
        { // same ending instant and rightclosedness: Advance both
          int ostart = ntt.GetOriginstart();
          int oend = ntt.GetOriginend();
          if(layer < 5 && ostart > -1 && oend > -1)
          {
            AtPeriods(layer+1, ostart, oend, auxPeriods, result);
            auxPeriods.Clear();
          }
          if( ++i > end )
            break;
          Get(layer, i, ntt);
          unit = ntt.value;
          if( ++j == p.GetNoComponents() )
            break;
          p.Get( j, interval );

        }
        else if( interval.rc == true )
        { // Advance in hmpoint
          int ostart = ntt.GetOriginstart();
          int oend = ntt.GetOriginend();
          if(layer < 5 && ostart > -1 && oend > -1)
          {
            AtPeriods(layer+1, ostart, oend, auxPeriods, result);
            auxPeriods.Clear();
          }

          if( ++i > end )
            break;
          Get(layer, i, ntt);
          unit = ntt.value;
        }
        else
        { // Advance in periods
          assert( interval.end < unit.timeInterval.end );
          if( ++j == p.GetNoComponents() )
            break;
          p.Get( j, interval );
        }
      }
      else if( interval.end > unit.timeInterval.end )
      { // Advance in hmpoint
        int ostart = ntt.GetOriginstart();
        int oend = ntt.GetOriginend();
        if(layer < 5 && ostart > -1 && oend > -1)
        {
          AtPeriods(layer+1, ostart, oend, auxPeriods, result);
          auxPeriods.Clear();
        }
        if( ++i > end )
          break;
        Get(layer, i, ntt);
        unit = ntt.value;
      }
      else
      { // Advance in periods
        assert( interval.end < unit.timeInterval.end );
        if( ++j == p.GetNoComponents() )
        {
          int ostart = ntt.GetOriginstart();
          int oend = ntt.GetOriginend();
          if(layer < 5 && ostart > -1 && oend > -1)
          {
            AtPeriods(layer+1, ostart, oend, auxPeriods, result);
            auxPeriods.Clear();
          }
          break;
        }
        p.Get( j, interval );
      }
    }
  }
  return 0;
}

/*
3.7.2 Functions to be part of relations

*/

inline ostream& HMPoint::Print( ostream &os ) const
{
  if( !IsDefined() )
    return os << "(HMPoint: undefined)";

  os << "(HMPoint: defined, MBR = ";
  bbox.Print(os);
  os << "Initial epsilon-value = " << epsilon << ", Factor = " << factor
    << endl;
  os << " Epsilon-values of the 5 uncertain Layers = ( ";
  for( int i = 0; i < 5; i++)
    os << GetLayerepsilon(i) << " ";
  os << ")\n";
  os << ", contains " << GetNoComponents() << " entities: \n";
  int size;
  HCUPoint ntt;
  for(int i = 0; i < 6; i++)
  {
    os << "\t(\n";
    size = LayerSize(i);
    for(int j = 0; j < size; j++)
    {
      Get(i, j, ntt);
      os << "\n\t\t";
      ntt.Print(os);
    }
    os << "\t)\n";
  }
  os << "\n)" << endl;
  return os;
}

inline int HMPoint::Compare( const Attribute *arg ) const
{
  const HMPoint* hmp2 = dynamic_cast<const HMPoint*>(arg);
  size_t size1;
  size_t size2;
  size_t index;
  HCUPoint ntt1;
  HCUPoint ntt2;
  int cmp;

  size1 = GetNoComponents();
  size2 = hmp2->GetNoComponents();
  if( size1 < size2 )
    return -1;
  if( size1 > size2 )
    return 1;

  cmp = HCMPoint::Compare( arg ); // call super

  size1 = certainlayer.Size();
  size2 = hmp2->certainlayer.Size();

  if( size1 < size2 )
    return -1;
  if( size1 > size2 )
    return 1;
  index = 0;
  while( (index < size1) && (index < size2) )
  {
    certainlayer.Get(index, ntt1);
    hmp2->certainlayer.Get(index, ntt2);
    cmp = ntt1.Compare(&ntt2);
    if(cmp) // different entities
      return cmp;
    index++;
  }
  return 0;
}

inline Attribute* HMPoint::Clone() const
{
  HMPoint *result = new HMPoint( 0 );

  if(GetNoComponents() > 0)
  {
    HCUPoint hcup;

    for( int i = 0; i < 6; i++)
    {
      int size = this->LayerSize(i);
      if(size > 0)
      {
        result->ResizeLayer( i, size );
        for( int j = 0; j < size; j++)
        {
          this->Get(i, j, hcup );
          HCUPoint ntt( hcup );
          result->Put(i,  j, ntt );
        }
        result->SetLayerepsilon( i, this->GetLayerepsilon(i) );
      }
    }
  }
  result->SetEpsilon(this->GetEpsilon());
  result->SetFactor(this->GetFactor());
  result->bbox = this->BoundingBox();

  // +++++ for debugging purposes only +++++
  //cout << "HMPoint::Clone: The HCMPoint's boundingbox is defined as: ";
  //result->BoundingBox().Print(cout);

  return result;
}

inline void HMPoint::CopyFrom( const Attribute* right )
{
  const HMPoint *hmp = dynamic_cast<const HMPoint*>(right);

  Clear();
  if(hmp->GetNoComponents() > 0)
  {
    HCUPoint ntt;

    for(int i = 0; i < 6; i++)
    {
      int size = hmp->LayerSize(i);
      if(size > 0)
      {
        ResizeLayer( i, size );
        for( int j = 0; j < size; j++)
        {
          hmp->Get( i, j, ntt );
          HCUPoint hcup( ntt );
          Put( i, j, hcup );
        }
        SetLayerepsilon( i, hmp->GetLayerepsilon(i) );
      }
    }
    SetEpsilon(hmp->GetEpsilon());
    SetFactor(hmp->GetFactor());
    bbox = hmp->BoundingBox();

    // +++++ for debugging purposes only +++++
    //cout << "HMPoint::CopyFrom: The HCMPoint's boundingbox is defined as: ";
    //BoundingBox().Print(cout);
  }
}

inline Flob* HMPoint::GetFLOB( const int i)
{
  assert( i > -1 );
  assert( i < NumOfFLOBs() );

  if( i == 5 )
    return &certainlayer;
  else
    return HCMPoint::GetFLOB( i );
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
      //  a standardised circle around p with circumference U = 2 * M_PI * r.

      for( int i = 0; i < n; i++ )
      {
        // The first point/vertex of the segment
        angle = i * 2 * M_PI/n; // angle to starting vertex
        valueX = x + radius * cos(angle);
        valueY = y + radius * sin(angle);
        Point v1(true, valueX ,valueY);

        // The second point/vertex of the segment
        if ((i+1) >= n)            // angle to end vertex
          angle = 0 * 2 * M_PI/n;    // for inner vertex
        else
          angle = (i+1) * 2 * M_PI/n;// for ending = starting vertex
        valueX = x + radius * cos(angle);
        valueY = y + radius * sin(angle);
        Point v2(true, valueX ,valueY);

        // Create a halfsegment for this segment
        HalfSegment hs(true, v1, v2);
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
                    const double epsilon, Point& defPP)
{
  Coord xl, yl, xr, yr;
  Point auxlp(true, 0, 0), auxrp(true, 1, 1);
  // the Points auxlp and auxrp must be defined to initialize the HalfSegment
  // aux!
  HalfSegment aux(false, auxlp, auxrp);

  // create a parallel HalfSegment on the inner side of rgnhs, which has a
  // distance of epsilon to rgnhs.

  if (AlmostEqual(rgnhs.GetLeftPoint().GetX(), rgnhs.GetRightPoint().GetX()))
  {
    // The region's halfsegment is vertical.
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
    // The region's halfsegment is horizontal.
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
  if( aux.Intersection( chs, defPP ) )
  {
    // +++++ for debugging purposes only +++
    //cout << "FindDefPassingPoint: aux.Intersection(chs, defPP) returns TRUE"
    // << endl;
    return true;
  }
  else
  {
    // If the halfsegment does not intersect the parallel halfsegment,
    // determine, if it intersects a circle around the dominating point
    // with a radius of epsilon.

      // +++++ for debugging purposes only +++
    //cout << "Determine defPP to the DomPoint of rgnhs.\n";

    Coord circleX, circleY;
    if ( chs.Distance( rgnhs.GetLeftPoint() ) <= epsilon )
    {
      circleX = rgnhs.GetLeftPoint().GetX();
      circleY = rgnhs.GetLeftPoint().GetY();
    }
    else if ( chs.Distance( rgnhs.GetRightPoint() ) <= epsilon )
    {
      circleX = rgnhs.GetRightPoint().GetX();
      circleY = rgnhs.GetRightPoint().GetY();
    }
    else {
      defPP.SetDefined(false);
      return false;
    }

    double a,b,c;
    double bb4ac, mu1, mu2;
    Coord p1x = chs.GetLeftPoint().GetX();
    Coord p1y = chs.GetLeftPoint().GetY();
    Coord p2x = chs.GetRightPoint().GetX();
    Coord p2y = chs.GetRightPoint().GetY();
    Coord lengthX = p2x - p1x;
    Coord lengthY = p2y - p1y;


    a = lengthX * lengthX + lengthY * lengthY;
    b = 2 * (lengthX * (p1x - circleX) + lengthY * (p1y - circleY) );
    c = circleX * circleX + circleY * circleY;
    c += p1x * p1x + p1y * p1y;
    c -= 2 * (circleX * p1x + circleY * p1y);
    c -= epsilon * epsilon;
    bb4ac = b * b - 4 * a * c;
    // originally: if (fabs(a) <= EPS || bb4ac < 0) but 'EPS' was
    // not declared in the code-example, this algorithm is derived from.
    if( bb4ac < 0 ) {
      mu1 = 0;
      mu2 = 0;
      return(false);
    }

    mu1 = (-b + sqrt(bb4ac)) / (2 * a);
    mu2 = (-b - sqrt(bb4ac)) / (2 * a);

    if( ( rgnhs.attr.insideAbove &&
        rgnhs.GetLeftPoint().GetY() >= rgnhs.GetRightPoint().GetY() &&
        chs.GetLeftPoint().GetY() < chs.GetRightPoint().GetY() ) ||
        ( !rgnhs.attr.insideAbove &&
        rgnhs.GetLeftPoint().GetY() < rgnhs.GetRightPoint().GetY() &&
        chs.GetLeftPoint().GetY() >= chs.GetRightPoint().GetY() ) )
      defPP.Set( p1x + mu1*(p2x - p1x), p1y + mu1*(p2y - p1y) );
    else
      defPP.Set( p1x + mu2*(p2x - p1x), p1y + mu2*(p2y - p1y) );

    // +++++ for debugging purposes only +++++
    //cout << "FindDefPassingPoint: \n";
    //cout << "     refering edgepoint of the region: " << circleX << " "
    //      << circleY << endl;
    //cout << "     determined definite-passing-point: " << defPP.GetX() << " "
    //      << defPP.GetY() << endl;
    //cout << "     epsilon = " << epsilon << endl;
    //cout << "     distance between the two points = "
    //      << rgnhs.GetDomPoint().Distance( defPP ) << endl;
    //cout << "    " << p1x << " <= " << defPP.GetX() << " <= " << p2x << "\n";
    //cout << "    " << p1y << " <= " << defPP.GetY() << " <= " << p2y << "\n";

    if( (p1x <= defPP.GetX() && defPP.GetX() <= p2x) &&
        ((p1y <= defPP.GetY() && defPP.GetY() <= p2y) ||
        (p2y <= defPP.GetY() && defPP.GetY() <= p1y)) )
    {
      // if defPP is a Point of chs
      return true;
    }
  }
  return false;
}

bool FindPosPassingPoint( const HalfSegment& chs, const HalfSegment& rgnhs,
                    const double epsilon, Point& posPP)
{
  Coord xl, yl, xr, yr;
  Point auxlp(true, 0, 0), auxrp(true, 1, 1);
  // The Points auxlp and auxrp must be defined to enable the initialization
  // of the HalfSegment aux!
  HalfSegment aux(false, auxlp, auxrp);

  // create a parallel HalfSegment on the inner side of rgnhs, which has a
  // distance of epsilon to rgnhs.

  if (AlmostEqual(rgnhs.GetLeftPoint().GetX(), rgnhs.GetRightPoint().GetX()))
  {
    // The region's halfsegment is vertical.
    if (rgnhs.attr.insideAbove)
    {
      // The region lies on the left side of the halfsegment.
      xl = rgnhs.GetLeftPoint().GetX() + epsilon;
      yl = rgnhs.GetLeftPoint().GetY();
      xr = rgnhs.GetRightPoint().GetX() + epsilon;
      yr = rgnhs.GetRightPoint().GetY();
    }
    else {
      // The region lies on the right side of the halfsegment.
      xl = rgnhs.GetLeftPoint().GetX() - epsilon;
      yl = rgnhs.GetLeftPoint().GetY();
      xr = rgnhs.GetRightPoint().GetX() - epsilon;
      yr = rgnhs.GetRightPoint().GetY();
    }
  }
  else if (AlmostEqual(rgnhs.GetLeftPoint().GetY(),
                      rgnhs.GetRightPoint().GetY()) )
  {
    // The region's halfsegment is horizontal.
    if (rgnhs.attr.insideAbove)
    {
      // The region lies above the halfsegment.
      xl = rgnhs.GetLeftPoint().GetX();
      yl = rgnhs.GetLeftPoint().GetY() - epsilon;
      xr = rgnhs.GetRightPoint().GetX();
      yr = rgnhs.GetRightPoint().GetY() - epsilon;
    }
    else {
      // The region lies below the halfsegment.
      xl = rgnhs.GetLeftPoint().GetX();
      yl = rgnhs.GetLeftPoint().GetY() + epsilon;
      xr = rgnhs.GetRightPoint().GetX();
      yr = rgnhs.GetRightPoint().GetY() + epsilon;
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
        xl = xl + epsilon * sinalpha;
        yl = yl - epsilon * cosalpha;
        xr = xr + epsilon * sinalpha;
        yr = yr - epsilon * cosalpha;
      }
      else {
        xl = xl - epsilon * sinalpha;
        yl = yl + epsilon * cosalpha;
        xr = xr - epsilon * sinalpha;
        yr = yr + epsilon * cosalpha;
      }
    }
    else {
      if(rgnhs.attr.insideAbove)
      {
        xl = xl - epsilon * sinalpha;
        yl = yl - epsilon * cosalpha;
        xr = xr - epsilon * sinalpha;
        yr = yr - epsilon * cosalpha;
      }
      else {
        xl = xl + epsilon * sinalpha;
        yl = yl + epsilon * cosalpha;
        xr = xr + epsilon * sinalpha;
        yr = yr + epsilon * cosalpha;
      }
    }
    auxlp.Set(xl, yl);
    auxrp.Set(xr, yr);
    aux.Set(rgnhs.IsLeftDomPoint(), auxlp, auxrp);
  }
  // Find the intersection-point between chs and aux if there is one.
  if( aux.Intersection( chs, posPP ) )
  {
    // +++++ for debugging purposes only +++
    //cout << "FindDefPassingPoint: aux.Intersection(chs, defPP) returns TRUE"
    // << endl;
    return true;
  }
  else
  {
    // If the halfsegment does not intersect the parallel halfsegment,
    // determine, if it intersects a circle around the dominating point
    // with a radius of epsilon.

      // +++++ for debugging purposes only +++
    //cout << "Determine defPP to the DomPoint of rgnhs.\n";

    Coord circleX, circleY;
    if ( chs.Distance( rgnhs.GetLeftPoint() ) <= epsilon )
    {
      circleX = rgnhs.GetLeftPoint().GetX();
      circleY = rgnhs.GetLeftPoint().GetY();
    }
    else if ( chs.Distance( rgnhs.GetRightPoint() ) <= epsilon )
    {
      circleX = rgnhs.GetRightPoint().GetX();
      circleY = rgnhs.GetRightPoint().GetY();
    }
    else {
      posPP.SetDefined(false);
      return false;
    }

    double a,b,c;
    double bb4ac, mu1, mu2;
    Coord p1x = chs.GetLeftPoint().GetX();
    Coord p1y = chs.GetLeftPoint().GetY();
    Coord p2x = chs.GetRightPoint().GetX();
    Coord p2y = chs.GetRightPoint().GetY();
    Coord lengthX = p2x - p1x;
    Coord lengthY = p2y - p1y;


    a = lengthX * lengthX + lengthY * lengthY;
    b = 2 * (lengthX * (p1x - circleX) + lengthY * (p1y - circleY) );
    c = circleX * circleX + circleY * circleY;
    c += p1x * p1x + p1y * p1y;
    c -= 2 * (circleX * p1x + circleY * p1y);
    c -= epsilon * epsilon;
    bb4ac = b * b - 4 * a * c;
    // originally: if (fabs(a) <= EPS || bb4ac < 0) but 'EPS' was
    // not declared in the code-example, this algorithm is derived from.
    if( bb4ac < 0 ) {
      mu1 = 0;
      mu2 = 0;
      return(false);
    }

    mu1 = (-b + sqrt(bb4ac)) / (2 * a);
    mu2 = (-b - sqrt(bb4ac)) / (2 * a);

    if( ( rgnhs.attr.insideAbove &&
        rgnhs.GetLeftPoint().GetY() >= rgnhs.GetRightPoint().GetY() &&
        chs.GetLeftPoint().GetY() < chs.GetRightPoint().GetY() ) ||
        ( !rgnhs.attr.insideAbove &&
        rgnhs.GetLeftPoint().GetY() < rgnhs.GetRightPoint().GetY() &&
        chs.GetLeftPoint().GetY() >= chs.GetRightPoint().GetY() ) )
      posPP.Set( p1x + mu2*(p2x - p1x), p1y + mu2*(p2y - p1y) );
    else
      posPP.Set( p1x + mu1*(p2x - p1x), p1y + mu1*(p2y - p1y) );

    // +++++ for debugging purposes only +++++
    //cout << "FindDefPassingPoint: \n";
    //cout << "     refering edgepoint of the region: " << circleX << " "
    //      << circleY << endl;
    //cout << "     determined definite-passing-point: " << defPP.GetX() << " "
    //      << defPP.GetY() << endl;
    //cout << "     epsilon = " << epsilon << endl;
    //cout << "     distance between the two points = "
    //      << rgnhs.GetDomPoint().Distance( defPP ) << endl;
    //cout << "    " << p1x << " <= " << defPP.GetX() << " <= " << p2x << "\n";
    //cout << "    " << p1y << " <= " << defPP.GetY() << " <= " << p2y << "\n";

    if( (p1x <= posPP.GetX() && posPP.GetX() <= p2x) &&
        ((p1y <= posPP.GetY() && posPP.GetY() <= p2y) ||
        (p2y <= posPP.GetY() && posPP.GetY() <= p1y)) )
    {
      // if defPP is a Point of chs
      return true;
    }
  }
  return false;
}

void Generalize( const double epsilon, const double factor,
                 const temporalalgebra::MPoint& source, 
                 const bool checkBreakPoints,
                 const DateTime dur, HMPoint& result)
{
  if( !source.IsDefined() )
  {
    cout << "The given temporalalgebra::MPoint-object is NOT defined!\n";
    result.SetDefined(false);
    return;
  }
  if( source.GetNoComponents() < 2 )
  {
    cout << "The given temporalalgebra::MPoint-object "
         << "contains less than 2 units!\n"
      << "There is nothing to generalize!\n";
    result.SetDefined(false);
    return;
  }

  assert( source.IsOrdered() );
  assert( epsilon > 0.0 );
  assert( factor > 1.0 );

  result.Clear();

  result.SetEpsilon( epsilon );
  result.SetFactor( factor );

  // insert the temporalalgebra::MPoint into the DBArray certainlayer
  int n = source.GetNoComponents();
  temporalalgebra::UPoint unit;
  result.certainlayer.resize( n );

  for(int i = 0; i < n; i++)
  {
    source.Get(i, unit);
    CUPoint cunit(unit);
    HCUPoint ntt(cunit, 5 , i, -1, -1);
    result.certainlayer.Put(i, ntt);
  }

  result.certainlayer.TrimToSize();
  // create a generalization for each layer

  //result.Generalize(5, checkBreakPoints, dur);

  int j = 5;
  int sizeOfLastLayer = n;
  while(j > 0 && sizeOfLastLayer > 1)
  {
    sizeOfLastLayer = result.Generalize(j, checkBreakPoints, dur);
    j--;
  }
  result.RestoreBoundingBox( true );
}

void Generalize( const double eps, const double factor, const int layer,
                  const CMPoint& source, const bool checkBreakPoints,
                  const DateTime dur, HCMPoint& result)
{
  result.Clear();
  double epsilon = eps;
  if( !source.IsDefined() )
  {
    cout << "The given temporalalgebra::MPoint-object is NOT defined!\n";
    return;
  }
  if( source.GetNoComponents() < 4 )
  {
    cout << "The given CMPoint-object contains less than 4 units!\n"
      << "In this case, a hierarchical structured dataset makes no sense!\n";
    return;
  }
  if( source.GetEpsilon() > epsilon )
  {
    cout << "The given uncertainty-value " << epsilon << " is less than the "
      "epsilon-value " << source.GetEpsilon() << " of the given CMPoint! "
      "The CMPoint's epsilon-value is used instead!\n";
    epsilon = source.GetEpsilon();
  }

  assert( source.IsOrdered() );
  assert( factor > 1.0 );

  int j = layer;
  if(j == -1)
    j = 4;
  result.SetEpsilon( epsilon );
  result.SetFactor( factor );

  // insert the temporalalgebra::MPoint into the DBArray certainlayer
  int n = source.GetNoComponents();
  CUPoint unit;
  result.ResizeLayer( j, n );

  for(int i = 0; i < n; i++)
  {
    source.Get(i, unit);
    HCUPoint ntt(unit, j, i, -1, -1);
    result.Put(j, i, ntt);
  }

  result.TrimLayerToSize( j );
  // create a generalization for each layer

  //result.Generalize(5, checkBreakPoints, dur);

  int sizeOfLastLayer = n;
  while(j > 0 && sizeOfLastLayer > 1)
  {
    sizeOfLastLayer = result.Generalize(j, checkBreakPoints, dur);
    j--;
  }
  result.RestoreBoundingBox( true );
}


static bool IsBreakPoint(const CUPoint* u,const DateTime& duration){
   if(u->p0 != u->p1){ // equal points required
     return false;
   }
   DateTime dur = u->timeInterval.end -  u->timeInterval.start;
   return (dur > duration);
}

static bool connected(const CUPoint* u1, const temporalalgebra::UPoint* u2){
  if( !AlmostEqual(u1->p1, u2->p0) ){ // spatial connected
    // +++++ for debugging purposes only +++++
    cout << "The Units are not SPATIAL connected!\n";
    return false;
  }
  // temporal connection
  if(! (u1->timeInterval.end == u2->timeInterval.start) ){
    // +++++ for debugging purposes only +++++
    cout << "The Units are not TEMPORAL connected!\n";
    return false;
  }
  return true;
}

/*
5 Type Constructors

5.1  Type Constructor ~CUPoint~

Type ~cupoint~ represents a pair (epsilon, (tinterval, (x0, y0, x1, y1)))
consisting of an uncertainty-value and a value of type upoint.

5.1.1 List Representation

The list representation of an ~upoint~ is

----   ( epsilon ( timeinterval (x0 yo x1 y1) ) )
----

For example:

----    ( 37.5 ( ( (instant 6.37)  (instant 9.9)   TRUE FALSE)
----                    (1.0 2.3 4.1 2.1) ) )

5.1.2 function Describing the Signature of the Type Constructor

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
5.1.3 Kind Checking Function

*/
bool CheckCUPoint( ListExpr type, ListExpr& errorInfo )
{
  return (nl->IsEqual( type, CUPoint::BasicType() ));
}

/*
5.1.4 ~Out~-function

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
5.1.5 ~In~-function

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

          temporalalgebra::Interval<Instant> tinterval( *start, *end,
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
      temporalalgebra::Interval<DateTime>(DateTime(instanttype),
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
5.1.6 ~Create~-function

*/
Word CreateCUPoint( const ListExpr typeInfo )
{
  return (SetWord( new CUPoint() ));
}

/*
5.1.7 ~Delete~-function

*/
void DeleteCUPoint( const ListExpr typeInfo, Word& w )
{
  delete (CUPoint *)w.addr;
  w.addr = 0;
}

/*
5.1.8 ~Close~-function

*/
void CloseCUPoint( const ListExpr typeInfo, Word& w )
{
  delete (CUPoint *)w.addr;
  w.addr = 0;
}

/*
5.1.9 ~Clone~-function

*/
Word CloneCUPoint( const ListExpr typeInfo, const Word& w )
{
  CUPoint *cupoint = (CUPoint *)w.addr;
  return SetWord( new CUPoint( *cupoint ) );
}

/*
5.1.10 ~Sizeof~-function

*/
int SizeOfCUPoint()
{
  return sizeof(CUPoint);
}

/*
5.1.11 ~Cast~-function

*/
void* CastCUPoint( void* addr )
{
  return (new (addr) CUPoint);
}

/*
Creation of the type constructor ~cupoint~

*/

TypeConstructor uncertainunitpoint(
        CUPoint::BasicType(),                //name
        CUPointProperty,     //property function describing signature
        OutCUPoint,
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
5.3 Type Constructor CMPoint

Type ~cmpoint~ represents a moving point.

5.3.1 List Representation

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

5.3.2 function Describing the Signature of the Type Constructor

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
5.3.3 Kind Checking Function

*/
bool
CheckCMPoint( ListExpr type, ListExpr& errorInfo )
{
  return (nl->IsEqual( type, CMPoint::BasicType() ));
}

TypeConstructor uncertainmovingpoint(
        CMPoint::BasicType(),                           //name
        CMPointProperty,        //property function describing signature
        temporalalgebra::OutMapping<CMPoint, CUPoint, OutCUPoint>,
        temporalalgebra::InMapping<CMPoint, CUPoint, InCUPoint>,
        0,
        0,                 //SaveToList and RestoreFromList functions
        temporalalgebra::CreateMapping<CMPoint>,
        temporalalgebra::DeleteMapping<CMPoint>, 
        0,
        0,      // object open and save
        temporalalgebra::CloseMapping<CMPoint>,
        temporalalgebra::CloneMapping<CMPoint>, //object close and clone
        temporalalgebra::CastMapping<CMPoint>,    //cast function
        temporalalgebra::SizeOfMapping<CMPoint>, //sizeof function
        CheckCMPoint );  //kind checking function


/*
5.4 Type Constructor HCUPoint

The type ~HCUPoint~ represents an Uncertain Unit Point within a hierarchical
structure like HMPoint or HCMPoint.

5.4.1 List Representation

The list representation of a ~hmpoint~ is

----    ( generalizedby layer index originstart originend ( cupoint ) )
----

For example:

----  ( (3 5 27 -1 -1)
          (0.7
            (((instant 6.37)  (instant 9.9)   TRUE FALSE) (1.0 2.3 4.1 2.1))))
----


5.4.2 function Describing the Signature of the Type Constructor

*/
ListExpr HCUPointProperty()
{
  return (nl->TwoElemList(
          nl->FourElemList(nl->StringAtom("Signature"),
                  nl->StringAtom("Example Type List"),
                  nl->StringAtom("List Rep"),
                  nl->StringAtom("Example List")),
          nl->FourElemList(nl->StringAtom("-> HIERARCHICAL UNIT"),
                  nl->StringAtom("(hcupoint) "),
                  nl->TextAtom("( ( generalizedby layer index originstart "
                          "originend ) ( epsilon, (timeInterval "
                          "(real_x0 real_y0 real_x1 real_y1) ) ) ) "),
                  nl->TextAtom("( (3 5 27 -1 -1) ( 0.7 ((i1 i2 TRUE FALSE)"
                          "(1.0 2.2 2.5 2.1))))"))));
}


/*
5.4.3 Kind Checking Function

*/
bool CheckHCUPoint( ListExpr type, ListExpr& errorInfo )
{
  return (nl->IsEqual( type, "hcupoint" ));
}

/*
5.4.4 ~Out~-function

*/
ListExpr OutHCUPoint( ListExpr typeInfo, Word value )
{
  HCUPoint* hcupoint = (HCUPoint*)(value.addr);

  if( !(((HCUPoint*)value.addr)->IsDefined()) )
    return (nl->SymbolAtom("undef"));
  else
  {
    ListExpr indexList = nl->FiveElemList(
        nl->IntAtom( hcupoint->GetGeneralizedby() ),
        nl->IntAtom( hcupoint->GetLayer() ),
        nl->IntAtom( hcupoint->GetIndex() ),
        nl->IntAtom( hcupoint->GetOriginstart() ),
        nl->IntAtom( hcupoint->GetOriginend() ));


    ListExpr timeintervalList = nl->FourElemList(
        OutDateTime( nl->TheEmptyList(),
        SetWord(&hcupoint->value.timeInterval.start) ),
        OutDateTime( nl->TheEmptyList(),
        SetWord(&hcupoint->value.timeInterval.end) ),
        nl->BoolAtom( hcupoint->value.timeInterval.lc ),
        nl->BoolAtom( hcupoint->value.timeInterval.rc));

    ListExpr pointsList = nl->FourElemList(
        nl->RealAtom( hcupoint->value.p0.GetX() ),
        nl->RealAtom( hcupoint->value.p0.GetY() ),
        nl->RealAtom( hcupoint->value.p1.GetX() ),
        nl->RealAtom( hcupoint->value.p1.GetY() ));

    ListExpr upointList = nl->TwoElemList( timeintervalList, pointsList );

    ListExpr cupointList = nl->TwoElemList(
                            nl->RealAtom( hcupoint->value.GetEpsilon() ),
                            upointList );

    return nl->TwoElemList( indexList, cupointList );
  }
}

/*
5.4.5 ~In~-function

*/
Word InHCUPoint( const ListExpr typeInfo, const ListExpr instance,
               const int errorPos, ListExpr& errorInfo, bool& correct )
{
  string errmsg;
  if ( nl->ListLength( instance ) == 2 )
  // 2 arguments are necessary: epsilon and a upoint
  {
    ListExpr indices = nl->First( instance );     // the epsilon value
    ListExpr cupoint = nl->Second( instance );    // the upoint representation

    if ( nl->ListLength( indices ) == 5 &&
         nl->IsAtom( nl->First(indices) ) &&
         nl->AtomType( nl->First(indices) ) == IntType &&
         nl->IsAtom( nl->Second(indices) ) &&
         nl->AtomType( nl->Second(indices) ) == IntType &&
         nl->IsAtom( nl->Third(indices) ) &&
         nl->AtomType( nl->Third(indices) ) == IntType &&
         nl->IsAtom( nl->Fourth(indices) ) &&
         nl->AtomType( nl->Fourth(indices) ) == IntType &&
         nl->IsAtom( nl->Fifth(indices) ) &&
         nl->AtomType( nl->Fifth(indices) ) == IntType )
    {
      correct = true;

      int genby = nl->IntValue( nl->First(indices) ),
          l = nl->IntValue( nl->Second(indices) ),
          idx = nl->IntValue( nl->Third(indices) ),
          ostart = nl->IntValue( nl->Fourth(indices) ),
          oend = nl->IntValue( nl->Fifth(indices) );

      if( l > 6 )
      {
        correct = false;
        errmsg = "InHUPoint(): Error! There are max 6 layers!";
        errorInfo = nl->Append(errorInfo, nl->StringAtom(errmsg));
        return SetWord( Address(0) );
      }

      if( nl->ListLength( cupoint ) != 2 )
      {
        correct = false;
        errmsg = "InHUPoint(): Error! CUPoint in invalid format!";
        errorInfo = nl->Append(errorInfo, nl->StringAtom(errmsg));
        return SetWord( Address(0) );
      }

      ListExpr upoint = nl->Second( cupoint );

      if( nl->IsAtom( nl->First( cupoint ) ) &&
          ( nl->AtomType( nl->First(cupoint) ) == RealType ||
            nl->AtomType( nl->First(cupoint) ) == IntType ) )
      {
        double e;
        if (nl->AtomType( nl->First(cupoint) ) == IntType)
          e = nl->IntValue( nl->First(cupoint) );
        else if(nl->AtomType( nl->First(cupoint) ) == RealType)
          e = nl->RealValue( nl->First(cupoint) );

        if ( nl->ListLength( upoint ) == 2 )
        // the upoint also consists of two components...
        {
          ListExpr tintvl = nl->First( upoint );        // the time-interval
          ListExpr endpoints = nl->Second( upoint );    // the two point values

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
              errmsg ="InHCUPoint(): Error in time-interval defining instant.";
              errorInfo = nl->Append(errorInfo, nl->StringAtom(errmsg));
              delete start;
              return SetWord( Address(0) );
            }

            Instant *end = (Instant *)InInstant( nl->TheEmptyList(),
                nl->Second( tintvl ), errorPos, errorInfo, correct ).addr;

            if( !correct )
            {
              errmsg ="InHCUPoint(): Error in time-interval defining instant.";
              errorInfo = nl->Append(errorInfo, nl->StringAtom(errmsg));
              delete start;
              delete end;
              return SetWord( Address(0) );
            }

            temporalalgebra::Interval<Instant> tinterval( *start, *end,
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
              {
                HCUPoint *hcupoint = new HCUPoint( *cupoint, genby, l, idx,
                                        ostart, oend);
                return SetWord( hcupoint );
              }
              errmsg = errmsg + "InCUPoint(): Error in start/end point.";
              errorInfo = nl->Append(errorInfo, nl->StringAtom(errmsg));
              delete cupoint;
            }
          }
        }
      }
    }
  }
  errmsg = "InHCUPoint(): Error in representation.";
  errorInfo = nl->Append(errorInfo, nl->StringAtom(errmsg));
  correct = false;
  return SetWord( Address(0) );
}

/*
5.4.6 ~Create~-function

*/
Word CreateHCUPoint( const ListExpr typeInfo )
{
  return (SetWord( new HCUPoint() ));
}

/*
5.4.7 ~Delete~-function

*/
void DeleteHCUPoint( const ListExpr typeInfo, Word& w )
{
  delete (HCUPoint *)w.addr;
  w.addr = 0;
}

/*
5.4.8 ~Close~-function

*/
void CloseHCUPoint( const ListExpr typeInfo, Word& w )
{
  delete (HCUPoint *)w.addr;
  w.addr = 0;
}

/*
5.4.9 ~Clone~-function

*/
Word CloneHCUPoint( const ListExpr typeInfo, const Word& w )
{
  HCUPoint *hcupoint = (HCUPoint *)w.addr;
  return SetWord( new HCUPoint( *hcupoint ) );
}

/*
5.4.10 ~Sizeof~-function

*/
int SizeOfHCUPoint()
{
  return sizeof(HCUPoint);
}

/*
5.4.11 ~Cast~-function

*/
void* CastHCUPoint( void* addr )
{
  return (new (addr) HCUPoint);
}

TypeConstructor hierarchicaluncertainunitpoint(
        "hcupoint",       //name
        HCUPointProperty, //property function describing signature
        OutHCUPoint,
        InHCUPoint,       //Out and In functions
        0,
        0,                //SaveToList and RestoreFromList functions
        CreateHCUPoint,
        DeleteHCUPoint,   //object creation and deletion
        0,
        0,                // object open and save
        CloseHCUPoint,
        CloneHCUPoint,    //object close and clone
        CastHCUPoint,     //cast function
        SizeOfHCUPoint,   //sizeof function
        CheckHCUPoint );  //kind checking function
/*
5.5 Type Constructor ~HCMPoint~

5.5.1 List Representation

The list representation of a ~hcmpoint~ is

----    ( e1 ... en )
----

, where e1, ..., en are entities of type ~hcupoint~.

for example:

----    (
          ( (3 5 27 -1 -1)
            (0.7
              (((instant 6.37) (instant 9.9) TRUE FALSE) (1.0 2.3 4.1 2.1))))
          ( (3 5 28 -1 -1)
            (0.9
              (((instant 9.9) (instant 10.2) TRUE FALSE) (4.1 2.1 5.3 2.7))))
        )
----

5.5.2 Function describing the Signature of the Type Constructor

*/
ListExpr HCMPointProperty()
{
  return (nl->TwoElemList(
          nl->FourElemList(nl->StringAtom("Signature"),
                  nl->StringAtom("Example Type List"),
                  nl->StringAtom("List Rep"),
                  nl->StringAtom("Example List")),
          nl->FourElemList(nl->StringAtom("-> HIERARCHICAL MAPPING"),
                  nl->StringAtom("(hcmpoint) "),
                  nl->StringAtom("( (epsilon factor) (e1 ... en) ) "),
                  nl->TextAtom("( ( 0.2 3.2 )"
                          "((3 5 27 -1 -1)( 0.7 ((i1 i2 TRUE FALSE)"
                          "(1.0 2.2 2.5 2.1)))) )"))));
}


/*
5.5.3 Kind Checking Function

*/
bool CheckHCMPoint( ListExpr type, ListExpr& errorInfo )
{
  return (nl->IsEqual( type, HCMPoint::BasicType() ));
}

/*
5.5.4 ~Out~-function

*/
ListExpr OutHCMPoint( ListExpr typeInfo, Word value )
{
  HCMPoint* hcmp = (HCMPoint*)(value.addr);
  if(! hcmp->IsDefined())
    return nl->SymbolAtom("undef");
  else
  if( hcmp->IsEmpty() )
    return (nl->TheEmptyList());
  else
  {
    ListExpr l = nl->TheEmptyList(),
             lastElem, entityList;

    // return the entities of each layer

    for( int i = 0; i < hcmp->layer0.Size(); i++ )
    {
      HCUPoint ntt;
      hcmp->layer0.Get( i, ntt );
      HCUPoint aux = (HCUPoint)ntt;
      entityList = OutHCUPoint( nl->TheEmptyList(), SetWord(&aux) );
      if( l == nl->TheEmptyList() )
      {
        l = nl->Cons( entityList, nl->TheEmptyList() );
        lastElem = l;
      }
      else
        lastElem = nl->Append( lastElem, entityList );
    }

    for( int i = 0; i < hcmp->layer1.Size(); i++ )
    {
      HCUPoint ntt;
      hcmp->layer1.Get( i, ntt );
      HCUPoint aux = (HCUPoint)ntt;
      entityList = OutHCUPoint( nl->TheEmptyList(), SetWord(&aux) );
      if( l == nl->TheEmptyList() )
      {
        l = nl->Cons( entityList, nl->TheEmptyList() );
        lastElem = l;
      }
      else
        lastElem = nl->Append( lastElem, entityList );
    }

    for( int i = 0; i < hcmp->layer2.Size(); i++ )
    {
      HCUPoint ntt;
      hcmp->layer2.Get( i, ntt );
      HCUPoint aux = (HCUPoint)ntt;
      entityList = OutHCUPoint( nl->TheEmptyList(), SetWord(&aux) );
      if( l == nl->TheEmptyList() )
      {
        l = nl->Cons( entityList, nl->TheEmptyList() );
        lastElem = l;
      }
      else
        lastElem = nl->Append( lastElem, entityList );
    }

    for( int i = 0; i < hcmp->layer3.Size(); i++ )
    {
      HCUPoint ntt;
      hcmp->layer3.Get( i, ntt );
      HCUPoint aux = (HCUPoint)ntt;
      entityList = OutHCUPoint( nl->TheEmptyList(), SetWord(&aux) );
      if( l == nl->TheEmptyList() )
      {
        l = nl->Cons( entityList, nl->TheEmptyList() );
        lastElem = l;
      }
      else
        lastElem = nl->Append( lastElem, entityList );
    }

    for( int i = 0; i < hcmp->layer4.Size(); i++ )
    {
      HCUPoint ntt;
      hcmp->layer4.Get( i, ntt );
      HCUPoint aux = (HCUPoint)ntt;
      entityList = OutHCUPoint( nl->TheEmptyList(), SetWord(&aux) );
      if( l == nl->TheEmptyList() )
      {
        l = nl->Cons( entityList, nl->TheEmptyList() );
        lastElem = l;
      }
      else
        lastElem = nl->Append( lastElem, entityList );
    }

    ListExpr epsilonfactor = nl->TwoElemList(nl->RealAtom( hcmp->GetEpsilon() ),
                                      nl->RealAtom( hcmp->GetFactor() ) );
    ListExpr layerepsilons = nl->FiveElemList(
                                      nl->RealAtom(hcmp->GetLayerepsilon(0)),
                                      nl->RealAtom(hcmp->GetLayerepsilon(1)),
                                      nl->RealAtom(hcmp->GetLayerepsilon(2)),
                                      nl->RealAtom(hcmp->GetLayerepsilon(3)),
                                      nl->RealAtom(hcmp->GetLayerepsilon(4)));
    ListExpr all = nl->ThreeElemList( epsilonfactor, layerepsilons, l );

    return all;
  }
}
/*
5.5.5 ~In~-function

*/
Word InHCMPoint( const ListExpr typeInfo, const ListExpr instance,
                const int errorPos, ListExpr& errorInfo, bool& correct )
{
  int numEntities = nl->ListLength(instance);
  HCMPoint* hcmp = new HCMPoint( numEntities );
  correct = true;
  int nttcounter = 0;
  string errmsg;

  if(nl->ListLength( instance ) != 3)
  {
    errmsg = "InHCMPoint(): The List has an unexpected length (!= 2).";
    errorInfo = nl->Append(errorInfo, nl->StringAtom(errmsg));
    correct = false;
    delete hcmp;
    return SetWord( Address(0) );
  }

  ListExpr epsilonfactor = nl->First( instance );
  ListExpr layerepsilons = nl->Second( instance );
  ListExpr elemlist = nl->Third( instance );

  if( nl->ListLength( epsilonfactor ) == 2 &&
      nl->IsAtom( nl->First(epsilonfactor) ) &&
      nl->AtomType( nl->First(epsilonfactor) ) == RealType &&
      nl->IsAtom( nl->Second(epsilonfactor) ) &&
      nl->AtomType( nl->Second(epsilonfactor) ) == RealType )
  {
    hcmp->SetEpsilon( nl->RealValue( nl->First(epsilonfactor)) );
    hcmp->SetFactor( nl->RealValue( nl->Second(epsilonfactor)) );
  }
  else
  {
    errmsg = "InHCMPoint(): One of the attributes 'epsilon' or 'factor' is "
            "invalid.";
    errorInfo = nl->Append(errorInfo, nl->StringAtom(errmsg));
    correct = false;
    delete hcmp;
    return SetWord( Address(0) );
  }

  if( nl->ListLength( layerepsilons ) == 5 &&
      nl->IsAtom( nl->First(layerepsilons) ) &&
      nl->AtomType( nl->First(layerepsilons) ) == RealType &&
      nl->IsAtom( nl->Second(layerepsilons) ) &&
      nl->AtomType( nl->Second(layerepsilons) ) == RealType &&
      nl->IsAtom( nl->Third(layerepsilons) ) &&
      nl->AtomType( nl->Third(layerepsilons) ) == RealType &&
      nl->IsAtom( nl->Fourth(layerepsilons) ) &&
      nl->AtomType( nl->Fourth(layerepsilons) ) == RealType &&
      nl->IsAtom( nl->Fifth(layerepsilons) ) &&
      nl->AtomType( nl->Fifth(layerepsilons) ) == RealType )
  {
    hcmp->SetLayer0epsilon( nl->RealValue( nl->First(layerepsilons)) );
    hcmp->SetLayer1epsilon( nl->RealValue( nl->Second(layerepsilons)) );
    hcmp->SetLayer2epsilon( nl->RealValue( nl->Third(layerepsilons)) );
    hcmp->SetLayer3epsilon( nl->RealValue( nl->Fourth(layerepsilons)) );
    hcmp->SetLayer4epsilon( nl->RealValue( nl->Fifth(layerepsilons)) );
  }
  else
  {
    errmsg = "InHCMPoint(): One of the attributes 'layer<n>epsilon' is "
            "invalid.";
    errorInfo = nl->Append(errorInfo, nl->StringAtom(errmsg));
    correct = false;
    delete hcmp;
    return SetWord( Address(0) );
  }

  ListExpr rest = elemlist;
  if (nl->AtomType( rest ) != NoAtom)
  {
    if(nl->IsEqual(rest,"undef"))
    {
       hcmp->SetDefined(false);
       return SetWord( Address( hcmp ) );
    }
    else
    {
      correct = false;
      delete hcmp;
      return SetWord( Address( 0 ) );
    }
  }
  else while( !nl->IsEmpty( rest ) )
  {
    ListExpr first = nl->First( rest );
    rest = nl->Rest( rest );

    HCUPoint *ntt = (HCUPoint*)InHCUPoint( nl->TheEmptyList(), first,
                                errorPos, errorInfo, correct ).addr;

    if( correct && ( !ntt->IsDefined() ) )
    {
      errmsg = "InHCMPoint(): Entity " + stringutils::int2str(nttcounter) 
               + " is undef.";
      errorInfo = nl->Append(errorInfo, nl->StringAtom(errmsg));
      correct = false;
      delete ntt;
      delete hcmp;
      return SetWord( Address(0) );
    }
    if ( !correct )
    {
      errmsg = "InHCMPoint(): Representation of Entity "
                + stringutils::int2str(nttcounter) + " is wrong.";
      errorInfo = nl->Append(errorInfo, nl->StringAtom(errmsg));
      hcmp->Destroy();
      delete hcmp;
      return SetWord( Address(0) );
    }

    const int layer = ntt->GetLayer();

    switch( layer )
    {
      case 0:
        hcmp->layer0.Put( ntt->GetIndex(), *ntt );
        break;

      case 1:
        hcmp->layer1.Put( ntt->GetIndex(), *ntt );
        break;

      case 2:
        hcmp->layer2.Put( ntt->GetIndex(), *ntt );
        break;

      case 3:
        hcmp->layer3.Put( ntt->GetIndex(), *ntt );
        break;

      case 4:
        hcmp->layer4.Put( ntt->GetIndex(), *ntt );
        break;

      default:
        // this point should never be reached!
        cout << "Error in InHCMPoint: The layer-number of an entity is greater"
          << " than 4!\n";
        correct = false;
        delete hcmp;
        return SetWord( Address(0) );
    }
    nttcounter++;
    delete ntt;
  }
  hcmp->RestoreBoundingBox( );

  return SetWord( hcmp );
}


/*
5.5.8 ~Create~-function

*/
Word CreateHCMPoint( const ListExpr typeInfo )
{
  return (SetWord( new HCMPoint( 0 ) ));
}

/*
5.5.9 ~Delete~-function

*/
void DeleteHCMPoint( const ListExpr typeInfo, Word& w )
{
  ((HCMPoint *)w.addr)->Destroy();
  delete (HCMPoint *)w.addr;
  w.addr = 0;
}

/*
5.5.10 ~Close~-function

*/
void CloseHCMPoint( const ListExpr typeInfo, Word& w )
{
  delete (HCMPoint *)w.addr;
  w.addr = 0;
}

/*
5.5.11 ~Clone~-function

*/
Word CloneHCMPoint( const ListExpr typeInfo, const Word& w )
{
  return SetWord( ((HCMPoint *)w.addr)->Clone() );
}

/*
5.5.12 ~Sizeof~-function

*/
int SizeOfHCMPoint()
{
  return sizeof(HCMPoint);
}

/*
5.5.13 ~Cast~-function

*/
void* CastHCMPoint(void* addr)
{
  return new (addr) HCMPoint;
}

/*
5.5.14 Creation of the type constructor ~mpoint~

*/
TypeConstructor hierarchicaluncertainmovingpoint(
        HCMPoint::BasicType(),          //name
        HCMPointProperty,    //property function describing signature
        OutHCMPoint,
        InHCMPoint,          //Out and In functions
        0,
        0,                  //SaveToList and RestoreFromList functions
        CreateHCMPoint,
        DeleteHCMPoint,      //object creation and deletion
        0,
        0,                  // object open and save
        CloseHCMPoint,
        CloneHCMPoint,       //object close and clone
        CastHCMPoint,        //cast function
        SizeOfHCMPoint,      //sizeof function
        CheckHCMPoint );     //kind checking function

/*
5.5 Type Constructor ~HMPoint~

5.5.1 List Representation

The list representation of a ~hmpoint~ is

----    ( e1 ... en )
----

, where e1, ..., en are entities of type ~hcupoint~.

for example:

----    (
          ( (3 5 27 -1 -1)
            (0.7
              (((instant 6.37) (instant 9.9) TRUE FALSE) (1.0 2.3 4.1 2.1))))
          ( (3 5 28 -1 -1)
            (0.9
              (((instant 9.9) (instant 10.2) TRUE FALSE) (4.1 2.1 5.3 2.7))))
        )
----

5.5.2 Function describing the Signature of the Type Constructor

*/
ListExpr HMPointProperty()
{
  return (nl->TwoElemList(
          nl->FourElemList(nl->StringAtom("Signature"),
                  nl->StringAtom("Example Type List"),
                  nl->StringAtom("List Rep"),
                  nl->StringAtom("Example List")),
          nl->FourElemList(nl->StringAtom("-> HIERARCHICAL MAPPING"),
                  nl->StringAtom("(hmpoint) "),
                  nl->StringAtom("( (epsilon factor) (e1 ... en) ) "),
                  nl->TextAtom("( ( 0.2 3.2 )"
                          "((3 5 27 -1 -1)( 0.7 ((i1 i2 TRUE FALSE)"
                          "(1.0 2.2 2.5 2.1)))) )"))));
}


/*
5.5.3 Kind Checking Function

*/
bool CheckHMPoint( ListExpr type, ListExpr& errorInfo )
{
  return (nl->IsEqual( type, HMPoint::BasicType() ));
}

/*
5.5.4 ~Out~-function

*/
ListExpr OutHMPoint( ListExpr typeInfo, Word value )
{
  HMPoint* hmp = (HMPoint*)(value.addr);
  if(! hmp->IsDefined())
    return nl->SymbolAtom("undef");
  else
  if( hmp->IsEmpty() )
    return (nl->TheEmptyList());
  else
  {
    ListExpr l = nl->TheEmptyList(),
             lastElem, entityList;

    // return the entities of each layer

    for( int i = 0; i < hmp->layer0.Size(); i++ )
    {
      HCUPoint ntt;
      hmp->layer0.Get( i, ntt );
      HCUPoint aux = (HCUPoint)ntt;
      entityList = OutHCUPoint( nl->TheEmptyList(), SetWord(&aux) );
      if( l == nl->TheEmptyList() )
      {
        l = nl->Cons( entityList, nl->TheEmptyList() );
        lastElem = l;
      }
      else
        lastElem = nl->Append( lastElem, entityList );
    }

    for( int i = 0; i < hmp->layer1.Size(); i++ )
    {
      HCUPoint ntt;
      hmp->layer1.Get( i, ntt );
      HCUPoint aux = (HCUPoint)ntt;
      entityList = OutHCUPoint( nl->TheEmptyList(), SetWord(&aux) );
      if( l == nl->TheEmptyList() )
      {
        l = nl->Cons( entityList, nl->TheEmptyList() );
        lastElem = l;
      }
      else
        lastElem = nl->Append( lastElem, entityList );
    }

    for( int i = 0; i < hmp->layer2.Size(); i++ )
    {
      HCUPoint ntt;
      hmp->layer2.Get( i, ntt );
      HCUPoint aux = (HCUPoint)ntt;
      entityList = OutHCUPoint( nl->TheEmptyList(), SetWord(&aux) );
      if( l == nl->TheEmptyList() )
      {
        l = nl->Cons( entityList, nl->TheEmptyList() );
        lastElem = l;
      }
      else
        lastElem = nl->Append( lastElem, entityList );
    }

    for( int i = 0; i < hmp->layer3.Size(); i++ )
    {
      HCUPoint ntt;
      hmp->layer3.Get( i, ntt );
      HCUPoint aux = (HCUPoint)ntt;
      entityList = OutHCUPoint( nl->TheEmptyList(), SetWord(&aux) );
      if( l == nl->TheEmptyList() )
      {
        l = nl->Cons( entityList, nl->TheEmptyList() );
        lastElem = l;
      }
      else
        lastElem = nl->Append( lastElem, entityList );
    }

    for( int i = 0; i < hmp->layer4.Size(); i++ )
    {
      HCUPoint ntt;
      hmp->layer4.Get( i, ntt );
      HCUPoint aux = (HCUPoint)ntt;
      entityList = OutHCUPoint( nl->TheEmptyList(), SetWord(&aux) );
      if( l == nl->TheEmptyList() )
      {
        l = nl->Cons( entityList, nl->TheEmptyList() );
        lastElem = l;
      }
      else
        lastElem = nl->Append( lastElem, entityList );
    }

    for( int i = 0; i < hmp->certainlayer.Size(); i++ )
    {
      HCUPoint ntt;
      hmp->certainlayer.Get( i, ntt );
      HCUPoint aux = (HCUPoint)ntt;
      entityList = OutHCUPoint( nl->TheEmptyList(), SetWord(&aux) );
      if( l == nl->TheEmptyList() )
      {
        l = nl->Cons( entityList, nl->TheEmptyList() );
        lastElem = l;
      }
      else
        lastElem = nl->Append( lastElem, entityList );
    }

    ListExpr hmpattr = nl->TwoElemList(nl->RealAtom( hmp->GetEpsilon() ),
                                       nl->RealAtom( hmp->GetFactor() ) );
    ListExpr hcmpattr = nl->FiveElemList(nl->RealAtom(hmp->GetLayerepsilon(0)),
                                        nl->RealAtom(hmp->GetLayerepsilon(1)),
                                        nl->RealAtom(hmp->GetLayerepsilon(2)),
                                        nl->RealAtom(hmp->GetLayerepsilon(3)),
                                        nl->RealAtom(hmp->GetLayerepsilon(4)));
    ListExpr all = nl->ThreeElemList( hmpattr, hcmpattr, l );

    return all;
  }
}
/*
5.5.5 ~In~-function

*/
Word InHMPoint( const ListExpr typeInfo, const ListExpr instance,
                const int errorPos, ListExpr& errorInfo, bool& correct )
{
  int numEntities = nl->ListLength(instance);
  HMPoint* hmp = new HMPoint( numEntities );
  correct = true;
  int nttcounter = 0;
  string errmsg;

  if(nl->ListLength( instance ) != 3)
  {
    errmsg = "InHMPoint(): The List has an unexpected length (!= 2).";
    errorInfo = nl->Append(errorInfo, nl->StringAtom(errmsg));
    correct = false;
    delete hmp;
    return SetWord( Address(0) );
  }

  ListExpr hmpointattr = nl->First( instance );
  ListExpr hcmpointattr = nl->Second( instance );
  ListExpr elemlist = nl->Third( instance );

  if( nl->ListLength( hmpointattr ) == 2 &&
      nl->IsAtom( nl->First(hmpointattr) ) &&
      nl->AtomType( nl->First(hmpointattr) ) == RealType &&
      nl->IsAtom( nl->Second(hmpointattr) ) &&
      nl->AtomType( nl->Second(hmpointattr) ) == RealType )
  {
    hmp->SetEpsilon( nl->RealValue( nl->First(hmpointattr)) );
    hmp->SetFactor( nl->RealValue( nl->Second(hmpointattr)) );
  }
  else
  {
    errmsg = "InHMPoint(): One of the attributes 'epsilon' or 'factor' is "
            "invalid.";
    errorInfo = nl->Append(errorInfo, nl->StringAtom(errmsg));
    correct = false;
    delete hmp;
    return SetWord( Address(0) );
  }

  if( nl->ListLength( hcmpointattr ) == 5 &&
      nl->IsAtom( nl->First(hcmpointattr) ) &&
      nl->AtomType( nl->First(hcmpointattr) ) == RealType &&
      nl->IsAtom( nl->Second(hcmpointattr) ) &&
      nl->AtomType( nl->Second(hcmpointattr) ) == RealType &&
      nl->IsAtom( nl->Third(hcmpointattr) ) &&
      nl->AtomType( nl->Third(hcmpointattr) ) == RealType &&
      nl->IsAtom( nl->Fourth(hcmpointattr) ) &&
      nl->AtomType( nl->Fourth(hcmpointattr) ) == RealType &&
      nl->IsAtom( nl->Fifth(hcmpointattr) ) &&
      nl->AtomType( nl->Fifth(hcmpointattr) ) == RealType )
  {
    hmp->SetLayer0epsilon( nl->RealValue( nl->First(hcmpointattr)) );
    hmp->SetLayer1epsilon( nl->RealValue( nl->Second(hcmpointattr)) );
    hmp->SetLayer2epsilon( nl->RealValue( nl->Third(hcmpointattr)) );
    hmp->SetLayer3epsilon( nl->RealValue( nl->Fourth(hcmpointattr)) );
    hmp->SetLayer4epsilon( nl->RealValue( nl->Fifth(hcmpointattr)) );
  }
  else
  {
    errmsg = "InHMPoint(): One of the attributes 'layer<n>epsilon' is "
            "invalid.";
    errorInfo = nl->Append(errorInfo, nl->StringAtom(errmsg));
    correct = false;
    delete hmp;
    return SetWord( Address(0) );
  }

  ListExpr rest = elemlist;
  if (nl->AtomType( rest ) != NoAtom)
  {
    if(nl->IsEqual(rest,"undef"))
    {
       hmp->SetDefined(false);
       return SetWord( Address( hmp ) );
    }
    else
    {
      correct = false;
      delete hmp;
      return SetWord( Address( 0 ) );
    }
  }
  else while( !nl->IsEmpty( rest ) )
  {
    ListExpr first = nl->First( rest );
    rest = nl->Rest( rest );

    HCUPoint *ntt = (HCUPoint*)InHCUPoint( nl->TheEmptyList(), first,
                                errorPos, errorInfo, correct ).addr;

    if( correct && ( !ntt->IsDefined() ) )
    {
      errmsg = "InHMPoint(): Entity " + stringutils::int2str(nttcounter) 
               + " is undef.";
      errorInfo = nl->Append(errorInfo, nl->StringAtom(errmsg));
      correct = false;
      delete ntt;
      delete hmp;
      return SetWord( Address(0) );
    }
    if ( !correct )
    {
      errmsg = "InHMPoint(): Representation of Entity "
                + stringutils::int2str(nttcounter) + " is wrong.";
      errorInfo = nl->Append(errorInfo, nl->StringAtom(errmsg));
      hmp->Destroy();
      delete hmp;
      return SetWord( Address(0) );
    }

    const int layer = ntt->GetLayer();

    switch( layer )
    {
      case 0:
        hmp->layer0.Put( ntt->GetIndex(), *ntt );
        break;

      case 1:
        hmp->layer1.Put( ntt->GetIndex(), *ntt );
        break;

      case 2:
        hmp->layer2.Put( ntt->GetIndex(), *ntt );
        break;

      case 3:
        hmp->layer3.Put( ntt->GetIndex(), *ntt );
        break;

      case 4:
        hmp->layer4.Put( ntt->GetIndex(), *ntt );
        break;

      case 5:
        hmp->certainlayer.Put( ntt->GetIndex(), *ntt );
        break;

      default:
        // this point should never be reached!
        cout << "Error in InHMPoint: The layer-number of an entity is greater"
          << " than 5!\n";
        correct = false;
        delete hmp;
        return SetWord( Address(0) );
    }
    nttcounter++;
    delete ntt;
  }
  hmp->RestoreBoundingBox( );

  return SetWord( hmp );
}


/*
5.5.8 ~Create~-function

*/
Word CreateHMPoint( const ListExpr typeInfo )
{
  return (SetWord( new HMPoint( 0 ) ));
}

/*
5.5.9 ~Delete~-function

*/
void DeleteHMPoint( const ListExpr typeInfo, Word& w )
{
  ((HMPoint *)w.addr)->Destroy();
  delete (HMPoint *)w.addr;
  w.addr = 0;
}

/*
5.5.10 ~Close~-function

*/
void CloseHMPoint( const ListExpr typeInfo, Word& w )
{
  delete (HMPoint *)w.addr;
  w.addr = 0;
}

/*
5.5.11 ~Clone~-function

*/
Word CloneHMPoint( const ListExpr typeInfo, const Word& w )
{
  return SetWord( ((HMPoint *)w.addr)->Clone() );
}

/*
5.5.12 ~Sizeof~-function

*/
int SizeOfHMPoint()
{
  return sizeof(HMPoint);
}

/*
5.5.13 ~Cast~-function

*/
void* CastHMPoint(void* addr)
{
  return new (addr) HMPoint;
}

/*
5.5.14 Creation of the type constructor ~mpoint~

*/
TypeConstructor hierarchicalmovingpoint(
        HMPoint::BasicType(),          //name
        HMPointProperty,    //property function describing signature
        OutHMPoint,
        InHMPoint,          //Out and In functions
        0,
        0,                  //SaveToList and RestoreFromList functions
        CreateHMPoint,
        DeleteHMPoint,      //object creation and deletion
        0,
        0,                  // object open and save
        CloseHMPoint,
        CloneHMPoint,       //object close and clone
        CastHMPoint,        //cast function
        SizeOfHMPoint,      //sizeof function
        CheckHMPoint );     //kind checking function

/*
Type Constructor +++++ hier weitere Typkonstruktoren anfuegen +++++

6 Operators

Definition of operators is similar to definition of type constructors. An
operator is defined by creating an instance of class ~Operator~. Again we
have to define some functions before we are able to create an ~Operator~
instance.

6.1 Type mapping functions

A type mapping function takes a nested list as argument. Its contents are
type descriptions of an operator's input parameters. A nested list describing
the output type of the operator is returned.


6.1.1 Type mapping function ~UncertainTypeMapReal~

This type mapping function is used for the Operation ~Epsilon()~.

*/
ListExpr UncertainTypeMapReal( ListExpr args )
{
  if ( nl->ListLength( args ) == 1 )
  {
    ListExpr arg1 = nl->First( args );

    if ( nl->IsEqual( arg1, CUPoint::BasicType() ) ||
          nl->IsEqual( arg1, CMPoint::BasicType() ) ||
          nl->IsEqual( arg1, HCMPoint::BasicType() ) )
      return nl->SymbolAtom(CcReal::BasicType());
    if (nl->AtomType( args ) == SymbolType)
    {
      ErrorReporter::ReportError("Type mapping function got a "
              "parameter of type " +nl->SymbolValue(args) + ".");
      return nl->SymbolAtom(Symbol::TYPEERROR());
    }
  }
  ErrorReporter::ReportError("Type mapping function got a "
        "parameter of length != 1.");
  return nl->SymbolAtom(Symbol::TYPEERROR());
}


/*
6.1.2 Type mapping function ~HierarchicalTypeMapInt~

This type mapping function is used for the Operation ~no\_components()~.

*/
ListExpr HierarchicalTypeMapInt( ListExpr args )
{
  if ( nl->ListLength( args ) == 1 )
  {
    ListExpr arg1 = nl->First( args );

    if ( nl->IsEqual( arg1, CMPoint::BasicType() ) ||
          nl->IsEqual( arg1, HCMPoint::BasicType() ) ||
          nl->IsEqual( arg1, HMPoint::BasicType()) )
      return nl->SymbolAtom(CcInt::BasicType());
    if (nl->AtomType( args ) == SymbolType)
    {
      ErrorReporter::ReportError("Type mapping function got a "
              "parameter of type " +nl->SymbolValue(args) + ".");
      return nl->SymbolAtom(Symbol::TYPEERROR());
    }
  }
  ErrorReporter::ReportError("Type mapping function got a "
        "parameter of length != 1.");
  return nl->SymbolAtom(Symbol::TYPEERROR());
}

/*
6.1.3 Type mapping function ~UncertainTypeMapBaseToUncertain~

This type mapping function is used for the Operator ~touncertain~. The keyword
'base' indicates a reduction of an uncertain type to its particular base type.
So in this case a 'base type' can also be a spatial or temporal type.

*/

ListExpr UncertainTypeMapBaseToUncertain( ListExpr args )
{
  if ( nl->ListLength( args ) == 2 )
  {
    ListExpr arg1 = nl->First( args );
    ListExpr arg2 = nl->Second( args );

    if( nl->IsEqual( arg1, temporalalgebra::UPoint::BasicType())  &&
        nl->IsEqual( arg2, CcReal::BasicType()) )
      return nl->SymbolAtom( CUPoint::BasicType() );

    if( nl->IsEqual( arg1, temporalalgebra::MPoint::BasicType()) &&
        nl->IsEqual( arg2, CcReal::BasicType()) )
      return nl->SymbolAtom( CMPoint::BasicType() );

    if (nl->AtomType( arg1 ) == SymbolType &&
        nl->AtomType( arg2 ) == SymbolType )
    {
      ErrorReporter::ReportError("Type mapping function got parameters of "
              "type " +nl->SymbolValue(arg1)+ " and " +nl->SymbolValue(arg2)+
              ".");
      return nl->SymbolAtom(Symbol::TYPEERROR());
    }
  }
  ErrorReporter::ReportError("Type mapping function got a "
        "parameter of length != 2.");
  return nl->SymbolAtom( Symbol::TYPEERROR() );
}


/*
6.1.8 Type mapping function ~UncertainMovingTypeMapSpatial~

This is for the operator ~trajectory~.

*/
ListExpr UncertainMovingTypeMapSpatial( ListExpr args )
{
  if ( nl->ListLength( args ) == 1 )
  {
    ListExpr arg1 = nl->First( args );

    if( nl->IsEqual( arg1, CMPoint::BasicType() ) ||
        nl->IsEqual( arg1, CUPoint::BasicType()) ||
        nl->IsEqual( arg1, HCMPoint::BasicType()) )
      return nl->SymbolAtom( Region::BasicType() );

    if( nl->IsEqual( arg1, HMPoint::BasicType() ) )
      return nl->SymbolAtom( Line::BasicType() );

    if (nl->AtomType( args ) == SymbolType)
    {
      ErrorReporter::ReportError("Type mapping function got a "
              "parameter of type " +nl->SymbolValue(args) + ".");
      return nl->SymbolAtom(Symbol::TYPEERROR());
    }
  }

  ErrorReporter::ReportError("Type mapping function got a "
        "parameter of length != 1.");
  return nl->SymbolAtom( Symbol::TYPEERROR() );
}


/*
6.1.9 Type mapping function ~UncertainMovingTypeMapTemporal~

This is defined for the operators ~deftime~.

*/
ListExpr UncertainMovingTypeMapPeriods( ListExpr args )
{
  if ( nl->ListLength( args ) == 1 )
  {
    ListExpr arg1 = nl->First( args );

    if( nl->IsEqual( arg1, CMPoint::BasicType() ) ||
        nl->IsEqual( arg1, HCMPoint::BasicType() ) ||
        nl->IsEqual( arg1, HMPoint::BasicType() ) )
      return nl->SymbolAtom( temporalalgebra::Periods::BasicType() );

    if (nl->AtomType( args ) == SymbolType)
    {
      ErrorReporter::ReportError("Type mapping function got a "
              "parameter of type " +nl->SymbolValue(args) + ".");
      return nl->SymbolAtom(Symbol::TYPEERROR());
    }
  }
  ErrorReporter::ReportError("Type mapping function got a "
        "parameter of length != 1.");
  return nl->SymbolAtom( Symbol::TYPEERROR() );
}

/*
6.1.10 Type mapping function ~UncertainMovingTypeMapBool~

It is for the operator ~present~.

*/
ListExpr UncertainMovingInstantPeriodsTypeMapBool( ListExpr args )
{
  if ( nl->ListLength( args ) == 2 )
  {
    ListExpr arg1 = nl->First( args ),
             arg2 = nl->Second( args );

    if( (nl->IsEqual(arg1, CMPoint::BasicType()) ||
        nl->IsEqual(arg1, HCMPoint::BasicType()) ||
        nl->IsEqual(arg1, HMPoint::BasicType()) ) &&
        (nl->IsEqual(arg2, Instant::BasicType()) ||
        nl->IsEqual(arg2, temporalalgebra::Periods::BasicType())) )
      return nl->SymbolAtom( CcBool::BasicType() );

    if (nl->AtomType( arg1 ) == SymbolType &&
        nl->AtomType( arg2 ) == SymbolType )
    {
      ErrorReporter::ReportError("Type mapping function got parameters of "
          "type " +nl->SymbolValue(arg1)+ " and " +nl->SymbolValue(arg2)+ ".");
      return nl->SymbolAtom(Symbol::TYPEERROR());
    }
  }
  ErrorReporter::ReportError("Type mapping function got a "
        "parameter of length != 2.");
  return nl->SymbolAtom( Symbol::TYPEERROR() );
}

/*
6.1.11 Type mapping function ~UncertainMovingTypeMapBool~

This is the type mapping function for the operators ~d\_passes~.

*/
ListExpr UncertainDPassesTypeMapBool( ListExpr args )
{
  if ( nl->ListLength( args ) == 2 )
  {
    ListExpr arg1 = nl->First( args ),
             arg2 = nl->Second( args );

    if( nl->IsEqual( arg2, Point::BasicType() ) ||
        nl->IsEqual( arg2, Region::BasicType() ) )
    {
      if( nl->IsEqual( arg1, CMPoint::BasicType()) ||
          nl->IsEqual( arg1, CUPoint::BasicType()) ||
          nl->IsEqual( arg1, HCMPoint::BasicType()) ||
          nl->IsEqual( arg1, HMPoint::BasicType() ) )
        return nl->SymbolAtom( CcBool::BasicType() );
    }
    if (nl->AtomType( arg1 ) == SymbolType &&
        nl->AtomType( arg2 ) == SymbolType )
    {
      ErrorReporter::ReportError("Type mapping function got parameters of "
          "type " +nl->SymbolValue(arg1)+ " and " +nl->SymbolValue(arg2)+ ".");
      return nl->SymbolAtom(Symbol::TYPEERROR());
    }
  }
  ErrorReporter::ReportError("Type mapping function got a "
        "parameter of length != 2.");
  return nl->SymbolAtom( Symbol::TYPEERROR() );
}

/*
6.1.11 Type mapping function ~UncertainMovingTypeMapBool~

This is the type mapping function for the operators ~p\_passes~.

*/
ListExpr UncertainPPassesTypeMapBool( ListExpr args )
{
  if ( nl->ListLength( args ) == 2 )
  {
    ListExpr arg1 = nl->First( args ),
             arg2 = nl->Second( args );

    if( nl->IsEqual( arg2, Point::BasicType() ) ||
        nl->IsEqual( arg2, Region::BasicType() ) )
    {
      if( nl->IsEqual( arg1, CMPoint::BasicType()) ||
          nl->IsEqual( arg1, CUPoint::BasicType()) ||
          nl->IsEqual( arg1, HCMPoint::BasicType()) )
        return nl->SymbolAtom( CcBool::BasicType() );
    }
    if (nl->AtomType( arg1 ) == SymbolType &&
        nl->AtomType( arg2 ) == SymbolType )
    {
      ErrorReporter::ReportError("Type mapping function got parameters of "
          "type " +nl->SymbolValue(arg1)+ " and " +nl->SymbolValue(arg2)+ ".");
      return nl->SymbolAtom(Symbol::TYPEERROR());
    }
  }
  ErrorReporter::ReportError("Type mapping function got a "
        "parameter of length != 2.");
  return nl->SymbolAtom( Symbol::TYPEERROR() );
}

/*
6.1.12 Type mapping function ~UncertainMovingTypeMapBool~

This is the type mapping function for the operators ~d\_passes~ and
~p\_passes~.

*/
ListExpr HierarchicalPassesTypeMapBool( ListExpr args )
{
  if ( nl->ListLength( args ) == 2 )
  {
    ListExpr arg1 = nl->First( args ),
             arg2 = nl->Second( args );

    if( nl->IsEqual( arg2, Point::BasicType() ) ||
        nl->IsEqual( arg2, Region::BasicType() ) )
    {
      if( nl->IsEqual( arg1, HMPoint::BasicType()) )
        return nl->SymbolAtom( CcBool::BasicType() );
    }
    if (nl->AtomType( arg1 ) == SymbolType &&
        nl->AtomType( arg2 ) == SymbolType )
    {
      ErrorReporter::ReportError("Type mapping function got parameters of "
          "type " +nl->SymbolValue(arg1)+ " and " +nl->SymbolValue(arg2)+ ".");
      return nl->SymbolAtom(Symbol::TYPEERROR());
    }
  }
  ErrorReporter::ReportError("Type mapping function got a "
        "parameter of length != 2.");
  return nl->SymbolAtom( Symbol::TYPEERROR() );
}

/*
6.1.12 Type mapping function ~UncertainMovingTypeMapCMPoint~

It is for the operator ~d\_at~

*/
ListExpr UncertainDAtTypeMapMoving( ListExpr args )
{
  if ( nl->ListLength( args ) == 2 )
  {
    ListExpr arg1 = nl->First( args ),
             arg2 = nl->Second( args );

    if( nl->IsEqual( arg2, Point::BasicType() ) )
    {
      if( nl->IsEqual(arg1, CUPoint::BasicType()) )
        return nl->SymbolAtom(CUPoint::BasicType());
      if( nl->IsEqual(arg1, CMPoint::BasicType()) ||
          nl->IsEqual(arg1, HCMPoint::BasicType()) )
        return nl->SymbolAtom(CMPoint::BasicType());
      if( nl->IsEqual(arg1, HMPoint::BasicType()) )
        return nl->SymbolAtom(temporalalgebra::MPoint::BasicType());
    }
    if( nl->IsEqual( arg2, Region::BasicType()) )
    {
      if( nl->IsEqual(arg1, CUPoint::BasicType()) ||
          nl->IsEqual(arg1, CMPoint::BasicType()) )
        return nl->SymbolAtom(CMPoint::BasicType());
      if( nl->IsEqual(arg1, HCMPoint::BasicType()) )
        return nl->SymbolAtom(CMPoint::BasicType());
      if( nl->IsEqual(arg1, HMPoint::BasicType()) )
        return nl->SymbolAtom(temporalalgebra::MPoint::BasicType());
    }

    if (nl->AtomType( arg1 ) == SymbolType &&
        nl->AtomType( arg2 ) == SymbolType )
    {
      ErrorReporter::ReportError("Type mapping function got parameters of "
          "type " +nl->SymbolValue(arg1)+ " and " +nl->SymbolValue(arg2)+ ".");
      return nl->SymbolAtom(Symbol::TYPEERROR());
    }
  }
  ErrorReporter::ReportError("Type mapping function got a "
        "parameter of length != 2.");
  return nl->SymbolAtom( Symbol::TYPEERROR() );
}

/*
6.1.12 Type mapping function ~UncertainMovingTypeMapCMPoint~

It is for the operator ~d\_at~

*/
ListExpr HierarchicalAtTypeMapMoving( ListExpr args )
{
  if ( nl->ListLength( args ) == 2 )
  {
    ListExpr arg1 = nl->First( args ),
             arg2 = nl->Second( args );

    if( nl->IsEqual( arg2, Point::BasicType() ) ||
        nl->IsEqual( arg2, Region::BasicType() ) )
    {
      if( nl->IsEqual(arg1, HMPoint::BasicType()) )
        return nl->SymbolAtom(temporalalgebra::MPoint::BasicType());
    }

    if (nl->AtomType( arg1 ) == SymbolType &&
        nl->AtomType( arg2 ) == SymbolType )
    {
      ErrorReporter::ReportError("Type mapping function got parameters of "
          "type " +nl->SymbolValue(arg1)+ " and " +nl->SymbolValue(arg2)+ ".");
      return nl->SymbolAtom(Symbol::TYPEERROR());
    }
  }
  ErrorReporter::ReportError("Type mapping function got a "
        "parameter of length != 2.");
  return nl->SymbolAtom( Symbol::TYPEERROR() );
}

/*
6.1.12 Type mapping function ~UncertainMovingTypeMapCMPoint~

It is for the operator ~d\_at~

*/
ListExpr UncertainPAtTypeMapMoving( ListExpr args )
{
  if ( nl->ListLength( args ) == 2 )
  {
    ListExpr arg1 = nl->First( args ),
             arg2 = nl->Second( args );

    if( nl->IsEqual( arg2, Point::BasicType() ) )
    {
      if( nl->IsEqual(arg1, CUPoint::BasicType()) )
        return nl->SymbolAtom(CUPoint::BasicType());
      if( nl->IsEqual(arg1, CMPoint::BasicType()) ||
          nl->IsEqual(arg1, HCMPoint::BasicType()) )
        return nl->SymbolAtom(CMPoint::BasicType());
    }
    if( nl->IsEqual( arg2, Region::BasicType()) )
    {
      if( nl->IsEqual(arg1, CUPoint::BasicType()) ||
          nl->IsEqual(arg1, CMPoint::BasicType()) )
        return nl->SymbolAtom(CMPoint::BasicType());
      if( nl->IsEqual(arg1, HCMPoint::BasicType()) )
        return nl->SymbolAtom(CMPoint::BasicType());
    }

    if (nl->AtomType( arg1 ) == SymbolType &&
        nl->AtomType( arg2 ) == SymbolType )
    {
      ErrorReporter::ReportError("Type mapping function got parameters of "
          "type " +nl->SymbolValue(arg1)+ " and " +nl->SymbolValue(arg2)+ ".");
      return nl->SymbolAtom(Symbol::TYPEERROR());
    }
  }
  ErrorReporter::ReportError("Type mapping function got a "
        "parameter of length != 2.");
  return nl->SymbolAtom( Symbol::TYPEERROR() );
}

/*
6.1.12 Type mapping function ~UncertainMovingTypeMapeIRegion~

It is for the operator ~atinstant~.

*/
ListExpr UncertainMovingInstantTypeMapIntime( ListExpr args )
{
  if ( nl->ListLength( args ) == 2 )
  {
    ListExpr arg1 = nl->First( args ),
             arg2 = nl->Second( args );

    if( nl->IsEqual( arg2, Instant::BasicType() ) )
    {
      if( nl->IsEqual( arg1, CUPoint::BasicType() ) ||
          nl->IsEqual( arg1, CMPoint::BasicType() ) ||
          nl->IsEqual( arg1, HCMPoint::BasicType()) )
        return nl->SymbolAtom( temporalalgebra::IRegion::BasicType() );
    }

    if( nl->IsEqual( arg2, Instant::BasicType() ) )
    {
      if( nl->IsEqual( arg1, HMPoint::BasicType()) )
        return nl->SymbolAtom( temporalalgebra::IPoint::BasicType() );
    }

    if (nl->AtomType( arg1 ) == SymbolType &&
        nl->AtomType( arg2 ) == SymbolType )
    {
      ErrorReporter::ReportError("Type mapping function got parameters of "
          "type " +nl->SymbolValue(arg1)+ " and " +nl->SymbolValue(arg2)+ ".");
      return nl->SymbolAtom(Symbol::TYPEERROR());
    }
  }
  ErrorReporter::ReportError("Type mapping function got a "
        "parameter of length != 2.");
  return nl->SymbolAtom( Symbol::TYPEERROR() );
}

/*
6.1.13 Type mapping function ~UncertainMovingPeriodsTypeMapMoving~

It is for the operator ~atperiods~.

*/
ListExpr UncertainMovingPeriodsTypeMapMoving( ListExpr args )
{
  if ( nl->ListLength( args ) == 2 )
  {
    ListExpr arg1 = nl->First( args ),
             arg2 = nl->Second( args );

    if( nl->IsEqual( arg2, temporalalgebra::Periods::BasicType() ) )
    {
      if( nl->IsEqual( arg1, CMPoint::BasicType() ) ||
          nl->IsEqual( arg1, HCMPoint::BasicType() ) )
        return nl->SymbolAtom( CMPoint::BasicType() );

      if( nl->IsEqual( arg1, HMPoint::BasicType() ) )
        return nl->SymbolAtom( temporalalgebra::MPoint::BasicType() );
    }

    if (nl->AtomType( arg1 ) == SymbolType &&
        nl->AtomType( arg2 ) == SymbolType )
    {
      ErrorReporter::ReportError("Type mapping function got parameters of "
          "type " +nl->SymbolValue(arg1)+ " and " +nl->SymbolValue(arg2)+ ".");
      return nl->SymbolAtom(Symbol::TYPEERROR());
    }
  }
  ErrorReporter::ReportError("Type mapping function got a "
        "parameter of length != 2.");
  return nl->SymbolAtom( Symbol::TYPEERROR() );
}

/*
6.1.14 Type Mapping Function ~UncertainMovingTypeMapUnits~

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

    if( nl->IsEqual( arg1, CMPoint::BasicType() ) )
      return nl->TwoElemList(nl->SymbolAtom(Symbol::STREAM()),
       nl->SymbolAtom(CUPoint::BasicType()));
    else
      ErrorReporter::ReportError("Type mapping function got wrong "
                            "types as parameters.");
  }
  ErrorReporter::ReportError("Type mapping function got a "
                      "parameter of length != 1.");
  return nl->SymbolAtom(Symbol::TYPEERROR());
}

/*
6.1.18 Type mapping function "UncertainTemporalBBoxTypeMap"

For operator ~bbox~

*/

ListExpr UncertainTemporalBBoxTypeMap( ListExpr args )
{
  ListExpr arg1;
  if ( nl->ListLength( args ) == 1 )
  {
    arg1 = nl->First( args );

    if( nl->IsEqual( arg1, CUPoint::BasicType() ) )
      return (nl->SymbolAtom( Rectangle<3>::BasicType() ));

    if( nl->IsEqual( arg1, CMPoint::BasicType() ) )
      return (nl->SymbolAtom( Rectangle<3>::BasicType() ));

    if (nl->AtomType( args ) == SymbolType)
    {
      ErrorReporter::ReportError("Type mapping function got a "
              "parameter of type " +nl->SymbolValue(args) + ".");
      return nl->SymbolAtom(Symbol::TYPEERROR());
    }
  }
  ErrorReporter::ReportError("Type mapping function got a "
        "parameter of length != 1.");
  return nl->SymbolAtom( Symbol::TYPEERROR() );
}


/*
6.1.19 Type Mapping Function ~MovingTypeMapUnits~

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

    if( nl->IsEqual( arg1, CMPoint::BasicType() ) )
      return nl->TwoElemList(nl->SymbolAtom(Symbol::STREAM()),
       nl->SymbolAtom(CUPoint::BasicType()));

    if (nl->AtomType( args ) == SymbolType)
    {
      ErrorReporter::ReportError("Type mapping function got a "
              "parameter of type " +nl->SymbolValue(args) + ".");
      return nl->SymbolAtom(Symbol::TYPEERROR());
    }
  }
  ErrorReporter::ReportError("Type mapping function got a "
        "parameter of length != 1.");
  return nl->SymbolAtom(Symbol::TYPEERROR());
}

/*
6.1.20 Type Mapping Function ~MovingTypeMapHierarchy~

It is used for the operator ~generalize~

Type mapping for ~generalize~ is

----    (mpoint epsilon factor) -> (hmpoint)
----

*/
ListExpr MovingTypeMapHierarchy( ListExpr args )
{
  if ( nl->ListLength(args) == 3 )
  {
    ListExpr arg1 = nl->First(args);
    ListExpr arg2 = nl->Second(args);
    ListExpr arg3 = nl->Third(args);

    if( nl->IsEqual( arg1, temporalalgebra::MPoint::BasicType() ) &&
        nl->IsEqual( arg2, CcReal::BasicType() ) &&
        nl->IsEqual( arg3, CcReal::BasicType() ) )
      return nl->SymbolAtom(HMPoint::BasicType());

    if( nl->IsEqual( arg1, CMPoint::BasicType() ) &&
        nl->IsEqual( arg2, CcReal::BasicType() ) &&
        nl->IsEqual( arg3, CcReal::BasicType() ) )
      return nl->SymbolAtom(HCMPoint::BasicType());

    if (nl->AtomType( arg1 ) == SymbolType &&
        nl->AtomType( arg2 ) == SymbolType &&
        nl->AtomType( arg3 ) == SymbolType )
    {
      ErrorReporter::ReportError("Type mapping function got parameters of "
          "type " +nl->SymbolValue(arg1)+ ", " +nl->SymbolValue(arg2)+ "and "
          +nl->SymbolValue(arg3)+ ".");
      return nl->SymbolAtom(Symbol::TYPEERROR());
    }
  }
  ErrorReporter::ReportError("Type mapping function got a "
        "parameter of length != 3.");
  return nl->SymbolAtom(Symbol::TYPEERROR());
}

/*
6.1.20 Type Mapping Function ~HierarchicalMovingTypeMapMoving~

It is used for the operator ~getmpoint~

Type mapping for ~getmpoint~ is

----    (hmpoint) -> (mpoint)
----

*/
ListExpr HierarchicalMovingTypeMapMoving( ListExpr args )
{
  if ( nl->ListLength(args) == 1 )
  {
    ListExpr arg1 = nl->First(args);

    if( nl->IsEqual( arg1, HMPoint::BasicType() ) )
      return nl->SymbolAtom(temporalalgebra::MPoint::BasicType());

    if (nl->AtomType( args ) == SymbolType)
    {
      ErrorReporter::ReportError("Type mapping function got a "
              "parameter of type " +nl->SymbolValue(args) + ".");
      return nl->SymbolAtom(Symbol::TYPEERROR());
    }
  }
  ErrorReporter::ReportError("Type mapping function got a "
        "parameter of length != 1.");
  return nl->SymbolAtom(Symbol::TYPEERROR());
}

/*
6.1.20 Type Mapping Function ~HierarchicalMovingTypeMapMoving~

It is used for the operator ~getmpoint~

Type mapping for ~getmpoint~ is

----    (hmpoint) -> (mpoint)
----

*/
ListExpr HierarchicalMovingTypeMapUncertainMoving( ListExpr args )
{
  if ( nl->ListLength(args) == 2 )
  {
    ListExpr arg1 = nl->First(args);
    ListExpr arg2 = nl->Second(args);

    if( ( nl->IsEqual(arg1, HMPoint::BasicType()) ||
        nl->IsEqual(arg1, HCMPoint::BasicType()) ) &&
        nl->IsEqual( arg2, CcReal::BasicType() ) )
      return nl->SymbolAtom(CMPoint::BasicType());

    if (nl->AtomType( arg1 ) == SymbolType &&
        nl->AtomType( arg2 ) == SymbolType )
    {
      ErrorReporter::ReportError("Type mapping function got parameters of "
          "type " +nl->SymbolValue(arg1)+ " and " +nl->SymbolValue(arg2)+ ".");
      return nl->SymbolAtom(Symbol::TYPEERROR());
    }
  }
  ErrorReporter::ReportError("Type mapping function got a "
        "parameter of length != 2.");
  return nl->SymbolAtom(Symbol::TYPEERROR());
}

/*
6.1.21 Type Mapping Function ~HierarchicalTypeMapHierarchical~

It is used for the operator ~reduce\_hierarchy~

Type mapping for ~reduce\_hierarchy~ is

----    (hmpoint real) -> (hcmpoint)
----

*/
ListExpr HierarchicalTypeMapHierarchical( ListExpr args )
{
  if ( nl->ListLength(args) == 2 )
  {
    ListExpr arg1 = nl->First(args);
    ListExpr arg2 = nl->Second(args);

    if( nl->IsEqual( arg1, HMPoint::BasicType() ) &&
        nl->IsEqual( arg2, CcReal::BasicType() ) )
      return nl->SymbolAtom(HCMPoint::BasicType());

    if (nl->AtomType( arg1 ) == SymbolType &&
        nl->AtomType( arg2 ) == SymbolType )
    {
      ErrorReporter::ReportError("Type mapping function got parameters of "
          "type " +nl->SymbolValue(arg1)+ " and " +nl->SymbolValue(arg2)+ ".");
      return nl->SymbolAtom(Symbol::TYPEERROR());
    }
  }
  ErrorReporter::ReportError("Type mapping function got a "
        "parameter of length != 2.");
  return nl->SymbolAtom(Symbol::TYPEERROR());
}

/*
6.2 Selection functions

A selection function is quite similar to a type mapping function. The only
difference is that it doesn't return a type but the index of a value
mapping function being able to deal with the respective combination of
input parameter types.

Note that a selection function does not need to check the correctness of
argument types; it has already been checked by the type mapping function that
it is applied to correct arguments.

6.2.1 Selection function ~UncertainSimpleSelect~

Is used for the ~epsilon~ operator.

*/
int UncertainSimpleSelect( ListExpr args )
{
  ListExpr arg1 = nl->First( args );

  if( nl->SymbolValue( arg1 ) == CUPoint::BasicType() )
    return 0;

  if( nl->SymbolValue( arg1 ) == CMPoint::BasicType() )
    return 1;

  if( nl->SymbolValue( arg1 ) == HCMPoint::BasicType() )
    return 2;

  return -1; // This point should never be reached
}

/*
6.2.2 Selection function ~HierarchicalSimpleSelect~

This Selection Function is used for operator ~deftime~ and ~atperiods~.

*/
int HierarchicalSimpleSelect( ListExpr args )
{
  ListExpr arg1 = nl->First( args );

  if( nl->SymbolValue( arg1 ) == CMPoint::BasicType() )
    return 0;

  if( nl->SymbolValue( arg1 ) == HCMPoint::BasicType() )
    return 1;

  if( nl->SymbolValue( arg1 ) == HMPoint::BasicType() )
    return 2;
  return -1;
}

/*
6.2.2 Selection function ~UncertainTemporalSelect~

Is used for the ~trajectory~ operator.

*/
int UncertainTemporalSelect( ListExpr args )
{
  ListExpr arg1 = nl->First( args );

  if( nl->SymbolValue( arg1 ) == CUPoint::BasicType() )
    return 0;

  if( nl->SymbolValue( arg1 ) == CMPoint::BasicType() )
    return 1;

  if( nl->SymbolValue( arg1 ) == HCMPoint::BasicType() )
    return 2;

  if( nl->SymbolValue( arg1 ) == HMPoint::BasicType() )
    return 3;

  return -1; // This point should never be reached
}

/*
6.2.3 Selection function ~UncertainTemporalAtTimeSelect~

Is used for the ~atinstant~ operator.

*/
int UncertainTemporalAtTimeSelect( ListExpr args )
{
  ListExpr arg1 = nl->First( args );

  if( nl->SymbolValue( arg1 ) == CUPoint::BasicType() )
    return 0;

  if( nl->SymbolValue( arg1 ) == CMPoint::BasicType() )
    return 1;

  if( nl->SymbolValue( arg1 ) == HCMPoint::BasicType() )
    return 2;

  if( nl->SymbolValue( arg1 ) == HMPoint::BasicType() )
    return 3;

  return -1; // This point should never be reached
}


/*
6.2.3 Selection function ~UncertainPassesSelect~

This is used for varius ~...passes~ and ~...at~ operators.

*/

int UncertainPassesSelect( ListExpr args )
{
  ListExpr arg1 = nl->First( args );
  ListExpr arg2 = nl->Second( args );

  if( nl->SymbolValue( arg1 ) == CUPoint::BasicType() &&
      nl->SymbolValue( arg2 ) == Point::BasicType() )
    return 0;

  if( nl->SymbolValue( arg1 ) == CUPoint::BasicType() &&
      nl->SymbolValue( arg2 ) == Region::BasicType() )
    return 1;

  if( nl->SymbolValue( arg1 ) == CMPoint::BasicType() &&
      nl->SymbolValue( arg2 ) == Point::BasicType() )
    return 2;

  if( nl->SymbolValue( arg1 ) == CMPoint::BasicType() &&
      nl->SymbolValue( arg2 ) == Region::BasicType() )
    return 3;

  if( nl->SymbolValue( arg1 ) == HCMPoint::BasicType() &&
      nl->SymbolValue( arg2 ) == Point::BasicType() )
    return 4;

  if( nl->SymbolValue( arg1 ) == HCMPoint::BasicType() &&
      nl->SymbolValue( arg2 ) == Region::BasicType() )
    return 5;

  if( nl->SymbolValue( arg1 ) == HMPoint::BasicType() &&
      nl->SymbolValue( arg2 ) == Point::BasicType() )
    return 6;

  if( nl->SymbolValue( arg1 ) == HMPoint::BasicType() &&
      nl->SymbolValue( arg2 ) == Region::BasicType() )
    return 7;

  return -1; // This point should never be reached
}

/*
6.2.3 Selection function ~HierarchicalPassesSelect~

This is used for operator ~passes~.

*/

int HierarchicalPassesSelect( ListExpr args )
{
  ListExpr arg1 = nl->First( args );
  ListExpr arg2 = nl->Second( args );

  if( nl->SymbolValue( arg1 ) == HMPoint::BasicType() &&
      nl->SymbolValue( arg2 ) == Point::BasicType() )
    return 0;

  if( nl->SymbolValue( arg1 ) == HMPoint::BasicType() &&
      nl->SymbolValue( arg2 ) == Region::BasicType() )
    return 1;

  return -1; // This point should never be reached
}

/*
6.2.4 Selection function ~UncertainMovingInstantPeriodsSelect~

This function is used for the operator ~present~ .

*/
int UncertainMovingInstantPeriodsSelect( ListExpr args )
{
  ListExpr arg1 = nl->First( args );
  ListExpr arg2 = nl->Second( args );

  if( nl->SymbolValue( arg1 ) == CMPoint::BasicType() &&
      nl->SymbolValue( arg2 ) == Instant::BasicType() )
    return 0;

  if( nl->SymbolValue( arg1 ) == CMPoint::BasicType() &&
      nl->SymbolValue( arg2 ) == temporalalgebra::Periods::BasicType() )
    return 1;

  if( nl->SymbolValue( arg1 ) == HCMPoint::BasicType() &&
      nl->SymbolValue( arg2 ) == Instant::BasicType() )
    return 2;

  if( nl->SymbolValue( arg1 ) == HCMPoint::BasicType() &&
      nl->SymbolValue( arg2 ) == temporalalgebra::Periods::BasicType() )
    return 3;

  if( nl->SymbolValue( arg1 ) == HMPoint::BasicType() &&
      nl->SymbolValue( arg2 ) == Instant::BasicType() )
    return 4;

  if( nl->SymbolValue( arg1 ) == HMPoint::BasicType() &&
      nl->SymbolValue( arg2 ) == temporalalgebra::Periods::BasicType() )
    return 5;


  return -1; // This point should never be reached
}

/*
6.2.5 Selection function ~TemporalToUncertainSelect~

*/
int TemporalToUncertainSelect( ListExpr args )
{
  ListExpr arg1 = nl->First( args );

  if( nl->SymbolValue( arg1 ) == temporalalgebra::UPoint::BasicType() )
    return 0;

  if( nl->SymbolValue( arg1 ) == temporalalgebra::MPoint::BasicType() )
    return 1;

  return -1; // This point should never be reached
}

/*
6.2.6 Selection function ~HierarchicalToUncertainSelect~

*/
int HierarchicalToUncertainSelect( ListExpr args )
{
  ListExpr arg1 = nl->First( args );

  if( nl->SymbolValue( arg1 ) == HMPoint::BasicType() )
    return 0;

  if( nl->SymbolValue( arg1 ) == HCMPoint::BasicType() )
    return 1;

  return -1; // This point should never be reached
}

/*
6.2.6 Selection function ~TemporalToHierarchy~

*/
int TemporalToHierarchySelect( ListExpr args )
{
  ListExpr arg1 = nl->First( args );

  if( nl->SymbolValue( arg1 ) == temporalalgebra::MPoint::BasicType() )
    return 0;

  if( nl->SymbolValue( arg1 ) == CMPoint::BasicType() )
    return 1;

  return -1; // This point should never be reached
}

/*
7 Value mapping functions

7.1 Value mapping functions for class ~CUPoint~

7.1.1 Value mapping function for operator ~epsilon~

*/
int CUPointEpsilon( Word* args, Word& result, int message, Word& local,
                                  Supplier s )
{
  result = qp->ResultStorage( s );
  CUPoint* u  = static_cast<CUPoint*>(args[0].addr);

  if( u->UncertainIsDefined() )
  {
    ((CcReal*)result.addr)->Set( u->GetEpsilon() );
  }
  else
    ((CcReal*)result.addr)->SetDefined( false );

  return 0;
}

/*
7.1.2 Value mapping functions for operator ~touncertain~

*/
int CUPointToUncertain( Word* args, Word& result, int message, Word& local,
                                  Supplier s )
{
  result = qp->ResultStorage( s );
  temporalalgebra::UPoint *u = 
                  static_cast<temporalalgebra::UPoint*>(args[0].addr);
  CcReal *e = static_cast<CcReal*>(args[1].addr);
  CUPoint *cup = ((CUPoint*)result.addr);

  if( u->IsDefined() && e->IsDefined() )
  {
    CUPoint aux( ((double)e->GetValue()) , *u);
    cup->CopyFrom( &aux );
    aux.DeleteIfAllowed();
  }
  else
    cup->SetDefined(false);

  return 0;
}


/*
6.2.2 Value mapping function for operator ~trajectory~

Unfortunately there will be no region created, if the epsilon value equals 0.
To avoid this, a minimal epsilon value is computed by deviding the coordinate
with the highest value by a constant named minEpsilonLimiter.

*/
int CUPointTrajectory( Word* args, Word& result, int message,
 Word& local, Supplier s )
{
  result = qp->ResultStorage( s );

  Region *region = ((Region*)result.addr);
  Region aux( 0 );
  CUPoint *cupoint = ((CUPoint*)args[0].addr);

  // If epsilon equals 0, set a minimal epsilon value greater 0 to ensure,
  // that a region-object is created!
  double max = fabs(cupoint->p0.GetX());
  if( fabs(cupoint->p1.GetX()) > max )
    max = fabs(cupoint->p1.GetX());
  if( fabs(cupoint->p0.GetY()) > max )
    max = fabs(cupoint->p0.GetY());
  if( fabs(cupoint->p1.GetY()) > max )
    max = fabs(cupoint->p1.GetY());
  max = max * minEpsilonLimiter;

  cupoint->UTrajectory(max, aux );
  region->CopyFrom( &aux );
  //aux.DeleteIfAllowed();  // causes errors!

  return 0;
}

/*
6.2.3 Value mapping functions for operator ~d\_passes~

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
6.2.4 Value mapping functions for operator ~p\_passes~

*/

// If the first argument is a CUPoint and the second one is a point:
int CUPointP_PassesPoint(Word* args, Word& result, int message,
                Word& local, Supplier s)
{
  result = qp->ResultStorage( s );

  CUPoint *u = ((CUPoint*)args[0].addr);
  Point* p = ((Point*)args[1].addr);

  if( !p->IsDefined() )
    ((CcBool *)result.addr)->Set( false, false );
  else if( u->P_Passes( *p ) )
    ((CcBool *)result.addr)->Set( true, true );
  else
    ((CcBool *)result.addr)->Set( true, false );

  return 0;
}


// If the first argument is a CUPoint and the second one is a region:
int CUPointP_PassesRegion(Word* args, Word& result, int message,
                Word& local, Supplier s)
{
  result = qp->ResultStorage( s );

  CUPoint *u = ((CUPoint*)args[0].addr);
  Region* r = ((Region*)args[1].addr);

  if( !r->IsDefined() )
    ((CcBool *)result.addr)->Set( false, false );
  else if( u->P_Passes( *r ) )
    ((CcBool *)result.addr)->Set( true, true );
  else
    ((CcBool *)result.addr)->Set( true, false );

  return 0;
}

/*
6.2.4 Value mapping function for operator ~atinstant~

*/
int CUPointAtInstant( Word* args, Word& result, int message,
                          Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  CUPoint* cup = ((CUPoint*)args[0].addr);
  Instant* inst = (Instant*) args[1].addr;
  temporalalgebra::IRegion aux(false);
  temporalalgebra::Intime<Region>* pResult =
                     (temporalalgebra::Intime<Region>*)result.addr;

  cup->AtInstant(*inst, aux);
  pResult->CopyFrom( &aux );
  aux.DeleteIfAllowed();
  return 0;
}

/*
6.2.5 Value mapping functions for operator ~d\_at~

*/

// If the first argument is a CUPoint and the second one is a point:
int CUPointD_AtPoint(Word* args, Word& result, int message,
                Word& local, Supplier s)
{
  result = qp->ResultStorage( s );

  CUPoint *u = ((CUPoint*)args[0].addr);
  Point* p = ((Point*)args[1].addr);
  CUPoint* pResult = (CUPoint*)result.addr;

  u->D_At(*p, *pResult);

  return 0;
}

// If the first argument is a CUPoint and the second one is a region:
int CUPointD_AtRegion(Word* args, Word& result, int message,
                Word& local, Supplier s)
{
  result = qp->ResultStorage( s );

  CUPoint *u = ((CUPoint*)args[0].addr);
  Region* r = ((Region*)args[1].addr);

  CMPoint* pResult = (CMPoint*)result.addr;

  u->D_At(*r, *pResult);

  return 0;
}

/*
6.2.6 Value mapping functions for operator ~p\_at~

*/

// If the first argument is a CUPoint and the second one is a point:
int CUPointP_AtPoint(Word* args, Word& result, int message,
                Word& local, Supplier s)
{
  result = qp->ResultStorage( s );

  CUPoint *u = ((CUPoint*)args[0].addr);
  Point* p = ((Point*)args[1].addr);

  CUPoint* pResult = (CUPoint*)result.addr;

  u->P_At(*p, *pResult);

  return 0;
}

// If the first argument is a CUPoint and the second one is a region:
int CUPointP_AtRegion(Word* args, Word& result, int message,
                Word& local, Supplier s)
{
  result = qp->ResultStorage( s );

  CUPoint *u = ((CUPoint*)args[0].addr);
  Region* r = ((Region*)args[1].addr);

  CMPoint* pResult = (CMPoint*)result.addr;

  u->P_At(*r, *pResult);

  return 0;
}

/*
6.3 Value mapping functions for class ~CMPoint~

6.3.1 Value mapping function for operator ~epsilon~

*/
int CMPointEpsilon( Word* args, Word& result, int message, Word& local,
                                  Supplier s )
{
  result = qp->ResultStorage( s );
  CMPoint* m  = static_cast<CMPoint*>(args[0].addr);

  if( m->UncertainIsDefined() )
  {
    ((CcReal*)result.addr)->Set( m->GetEpsilon() );
  }
  else
    ((CcReal*)result.addr)->SetDefined( false );

  return 0;
}

/*
6.3.2 Value mapping function for operator ~no\_components~

*/
int CMPointNoComponents( Word* args, Word& result, int message, Word& local,
                                  Supplier s )
{
  result = qp->ResultStorage( s );
  CMPoint* m  = static_cast<CMPoint*>(args[0].addr);

  if( m->IsDefined() )
  {
    ((CcInt*)result.addr)->Set( m->GetNoComponents() );
  }
  else
    ((CcInt*)result.addr)->SetDefined( false );

  return 0;
}

/*
7.2.2 Value mapping functions for operator ~touncertain~

*/
int CMPointToUncertain( Word* args, Word& result, int message, Word& local,
                                  Supplier s )
{
  result = qp->ResultStorage( s );
  temporalalgebra::MPoint *m = 
                 static_cast<temporalalgebra::MPoint*>(args[0].addr);
  CcReal *e = static_cast<CcReal*>(args[1].addr);
  CMPoint *cmp = ((CMPoint*)result.addr);

  temporalalgebra::UPoint unit;

  cmp->Clear();

  if( m->IsDefined() && e->IsDefined() )
  {
    for(int i = 0; i < m->GetNoComponents(); i++)
    {
      m->Get(i, unit);
      CUPoint aux( ((double)e->GetValue()), unit );
      cmp->Add(aux);
      //aux.DeleteIfAllowed(); // causes Errors!
    }
  }
  else
    cmp->SetDefined(false);

  return 0;
}

/*
6.3.2 Value mapping function for operator ~trajectory~

*/
int CMPointTrajectory( Word* args, Word& result, int message,
 Word& local, Supplier s )
{
  // +++++ for debugging purposes only +++++
  //cout << "Value Mapping Function CMPointTrajectory is called!\n";

  ostringstream strTrajectPtr;
  string querystring;
  result = qp->ResultStorage( s );

  if( args[0].addr > 0 )
  {
    // create the query list:
    strTrajectPtr << (long)args[0].addr;

    // +++++ for debugging purposes only +++
    //cout << "Das aufgerufene Objekt hat die Adresse: " << strTrajectPtr.str()
    //  << ".\n";

    querystring =
    "(aggregateB"
      "(projectextend"
        "(namedtransformstream"
          "(units (cmpoint (ptr " + strTrajectPtr.str() + ")) )"
          "Unit)"
        "()"
        "("
          "(regions"
            "(fun"
              "(tuple1 TUPLE)"
                "(trajectory"
                  "(attr tuple1 Unit))))))"
      "regions"
      "(fun"
        "(r1 region)"
        "(r2 region)"
        "(union_new r1 r2))"
      "(region"
        "()))";

    // +++++ for debugging purposes only +++++
    //cout << querystring << endl;
    if( !QueryProcessor::ExecuteQuery(querystring, result) )
      cout << "Error in executing operator query" << endl;
  }
  return 0;
}

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
  temporalalgebra::Periods* periods = ((temporalalgebra::Periods*)args[1].addr);

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
  temporalalgebra::Intime<Region>* pResult = 
                  (temporalalgebra::Intime<Region>*)result.addr;
  temporalalgebra::IRegion aux(false);

  cmp->AtInstant(*inst, aux);
  pResult->CopyFrom( &aux );
  //aux.DeleteIfAllowed();  //causes errors!
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
  CMPoint aux(false);
  CMPoint* pResult = (CMPoint*)result.addr;
  temporalalgebra::Periods* per = (temporalalgebra::Periods*)args[1].addr;

  cmp->AtPeriods(*per,aux);
  pResult->CopyFrom( &aux );
  return 0;
}

/*
6.3.6 Value mapping function for operator ~units~

*/

class MyUnitsLocalInfo{
public:
    Word mWord;
    int unitIndex;
};

int UncertainMappingUnits(Word* args, Word& result, int message,
                Word& local, Supplier s)
{
  CMPoint* m;
  CUPoint unit;
  MyUnitsLocalInfo* localinfo;

  switch( message )
  {
    case OPEN:

      localinfo = new MyUnitsLocalInfo();
      localinfo->mWord = args[0];
      localinfo->unitIndex = 0;
      local = SetWord(localinfo);
      return 0;

    case REQUEST:

      if( local.addr == 0 )
        return CANCEL;
      localinfo = (MyUnitsLocalInfo *) local.addr;
      m = (CMPoint*)localinfo->mWord.addr;
      if( (0 <= localinfo->unitIndex)
          && (localinfo->unitIndex < m->GetNoComponents()) )
      {
        m->Get( localinfo->unitIndex++, unit );
        CUPoint* aux = new CUPoint( unit );

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
        delete (MyUnitsLocalInfo *)local.addr;
      return 0;
  }
  // should not happen
  return -1;
}

/*
6.3.7 value mapping functions for operator ~d\_passes~

*/

// If the first argument is a CMPoint and the second one is a point:
int CMPointD_PassesPoint(Word* args, Word& result, int message,
                Word& local, Supplier s)
{
  result = qp->ResultStorage( s );

  CMPoint *m = ((CMPoint*)args[0].addr);
  Point* p = ((Point*)args[1].addr);

  if( !p->IsDefined() )
    ((CcBool *)result.addr)->Set( false, false );
  else if( m->D_Passes( *p ) )
    ((CcBool *)result.addr)->Set( true, true );
  else
    ((CcBool *)result.addr)->Set( true, false );

  return 0;
}

// If the first argument is a CMPoint and the second one is a region:
int CMPointD_PassesRegion(Word* args, Word& result, int message,
                Word& local, Supplier s)
{
  result = qp->ResultStorage( s );

  CMPoint *m = ((CMPoint*)args[0].addr);
  Region* r = ((Region*)args[1].addr);

  if( !r->IsDefined() )
    ((CcBool *)result.addr)->Set( false, false );
  else if( m->D_Passes( *r ) )
    ((CcBool *)result.addr)->Set( true, true );
  else
    ((CcBool *)result.addr)->Set( true, false );

  return 0;
}

/*
6.3.8 value mapping functions for operator ~p\_passes~

*/

// If the first argument is a CMPoint and the second one is a point:
int CMPointP_PassesPoint(Word* args, Word& result, int message,
                Word& local, Supplier s)
{
  result = qp->ResultStorage( s );

  CMPoint *m = ((CMPoint*)args[0].addr);
  Point* p = ((Point*)args[1].addr);

  if( !p->IsDefined() )
    ((CcBool *)result.addr)->Set( false, false );
  else if( m->P_Passes( *p ) )
    ((CcBool *)result.addr)->Set( true, true );
  else
    ((CcBool *)result.addr)->Set( true, false );

  return 0;
}

// If the first argument is a CMPoint and the second one is a region:
int CMPointP_PassesRegion(Word* args, Word& result, int message,
                Word& local, Supplier s)
{
  result = qp->ResultStorage( s );

  CMPoint *m = ((CMPoint*)args[0].addr);
  Region* r = ((Region*)args[1].addr);

  if( !r->IsDefined() )
    ((CcBool *)result.addr)->Set( false, false );
  else if( m->P_Passes( *r ) )
    ((CcBool *)result.addr)->Set( true, true );
  else
    ((CcBool *)result.addr)->Set( true, false );

  return 0;
}

/*
6.3.9 value mapping functions for operator ~d\_at~

*/

// If the first argument is a CMPoint and the second one is a point:
int CMPointD_AtPoint(Word* args, Word& result, int message,
                Word& local, Supplier s)
{
  result = qp->ResultStorage( s );

  CMPoint *m = ((CMPoint*)args[0].addr);
  Point* p = ((Point*)args[1].addr);
  CMPoint* pResult = (CMPoint*)result.addr;
  CMPoint aux( 0 );

  m->D_At(*p, aux);
  pResult->CopyFrom( &aux );

  return 0;
}

// If the first argument is a CMPoint and the second one is a region:
int CMPointD_AtRegion(Word* args, Word& result, int message,
                Word& local, Supplier s)
{
  result = qp->ResultStorage( s );

  CMPoint *m = ((CMPoint*)args[0].addr);
  Region* r = ((Region*)args[1].addr);
  CMPoint* pResult = (CMPoint*)result.addr;
  CMPoint aux( 0 );

  m->D_At(*r, aux);
  pResult->CopyFrom( &aux );

  return 0;
}

/*
6.3.10 value mapping functions for operator ~p\_at~

*/

// If the first argument is a CMPoint and the second one is a point:
int CMPointP_AtPoint(Word* args, Word& result, int message,
                Word& local, Supplier s)
{
  result = qp->ResultStorage( s );

  CMPoint *m = ((CMPoint*)args[0].addr);
  Point* p = ((Point*)args[1].addr);
  CMPoint* pResult = (CMPoint*)result.addr;
  CMPoint aux( 0 );

  m->P_At(*p, aux);
  pResult->CopyFrom( &aux );

  return 0;
}

// If the first argument is a CMPoint and the second one is a region:
int CMPointP_AtRegion(Word* args, Word& result, int message,
                Word& local, Supplier s)
{
  result = qp->ResultStorage( s );

  CMPoint *m = ((CMPoint*)args[0].addr);
  Region* r = ((Region*)args[1].addr);
  CMPoint* pResult = (CMPoint*)result.addr;
  CMPoint aux( 0 );

  m->P_At(*r, aux);
  pResult->CopyFrom( &aux );

  return 0;
}

/*
6.4 Value Mapping Functions for type ~HMPoint~

6.4.11 value mapping functions for operator ~generalize~

*/

int GeneralizeMPoint( Word* args, Word& result, int message, Word& local,
                                  Supplier s )
{
  result = qp->ResultStorage( s );
  temporalalgebra::MPoint *m = 
                 static_cast<temporalalgebra::MPoint*>(args[0].addr);
  CcReal *e = static_cast<CcReal*>(args[1].addr);
  CcReal *f = static_cast<CcReal*>(args[2].addr);
  HMPoint* pResult = (HMPoint*)result.addr;
  HMPoint aux( 0 );
  DateTime dt(0, 0, durationtype);

  if( m->IsDefined() && e->IsDefined() && f->IsDefined() )
  {
    Generalize( static_cast<double>(e->GetValue()),
                static_cast<double>(f->GetValue()), *m, false, dt, aux );
    pResult->CopyFrom( &aux );
  }
  else
    pResult->SetDefined(false);

  return 0;
}

/*
6.4.2 Value mapping function for operator ~no\_components~

*/
int HMPointNoComponents( Word* args, Word& result, int message, Word& local,
                                  Supplier s )
{
  result = qp->ResultStorage( s );
  HMPoint* hmp  = static_cast<HMPoint*>(args[0].addr);

  if( hmp->IsDefined() )
  {
    ((CcInt*)result.addr)->Set( hmp->GetNoComponents() );
  }
  else
    ((CcInt*)result.addr)->SetDefined( false );

  return 0;
}

/*
6.4.12 value mapping functions for operator ~trajectory~

*/

int HMPointTrajectory( Word* args, Word& result, int message, Word& local,
                                  Supplier s )
{
  result = qp->ResultStorage( s );
  HMPoint *hmp = static_cast<HMPoint*>(args[0].addr);
  Line* pResult = (Line*)result.addr;
  Line auxln( 0 );

  temporalalgebra::MPoint aux( 0 );

  if( hmp->IsDefined() )
  {
    hmp->GetMPoint(aux);
    aux.Trajectory(auxln);
    aux.DeleteIfAllowed();
    pResult->CopyFrom( &auxln );
    auxln.DeleteIfAllowed();
  }
  else
    pResult->SetDefined(false);

  return 0;
}

/*
6.4.12 value mapping functions for operator ~getmpoint~

*/

int HMPointGetMPoint( Word* args, Word& result, int message, Word& local,
                                  Supplier s )
{
  result = qp->ResultStorage( s );
  HMPoint *hmp = static_cast<HMPoint*>(args[0].addr);
  temporalalgebra::MPoint* pResult = (temporalalgebra::MPoint*)result.addr;
  temporalalgebra::MPoint aux( 0 );


  if( hmp->IsDefined() )
  {
    hmp->GetMPoint(aux);
    pResult->CopyFrom( &aux );
  }
  else
    pResult->SetDefined(false);
  return 0;
}

/*
6.4.12 value mapping functions for operator ~getcmpoint~

*/
int HMPointGetCMPoint( Word* args, Word& result, int message, Word& local,
                                  Supplier s )
{
  result = qp->ResultStorage( s );
  HMPoint *hmp = static_cast<HMPoint*>(args[0].addr);
  CcReal *epsilon = static_cast<CcReal*>(args[1].addr);
  CMPoint* pResult = (CMPoint*)result.addr;
  CMPoint aux( 0 );

  if( hmp->IsDefined() )
  {
    hmp->GetCMPoint( epsilon->GetValue(), aux );
    pResult->CopyFrom( &aux );
  }
  else
    pResult->SetDefined(false);

  return 0;
}

/*
6.4.13 Value Mapping functions for operator ~d\_passes~

*/

// If the second argument is a Point:
int HMPointD_PassesPoint(Word* args, Word& result, int message,
                Word& local, Supplier s)
{
  result = qp->ResultStorage( s );

  HMPoint *hmp = ((HMPoint*)args[0].addr);
  Point* p = ((Point*)args[1].addr);

  if( !p->IsDefined() )
    ((CcBool *)result.addr)->Set( false, false );
  else if( hmp->D_Passes( *p ) )
    ((CcBool *)result.addr)->Set( true, true );
  else
    ((CcBool *)result.addr)->Set( true, false );

  return 0;
}

// If the second argument is a Region:
int HMPointD_PassesRegion(Word* args, Word& result, int message,
                Word& local, Supplier s)
{
  result = qp->ResultStorage( s );

  HMPoint *hmp = ((HMPoint*)args[0].addr);
  Region* r = ((Region*)args[1].addr);

  if( !r->IsDefined() )
    ((CcBool *)result.addr)->Set( false, false );
  else if( hmp->D_Passes( *r ) )
    ((CcBool *)result.addr)->Set( true, true );
  else
    ((CcBool *)result.addr)->Set( true, false );

  return 0;
}

/*
6.4.14 value mapping functions for operator ~d\_at~

*/

// If the first argument is a HMPoint and the second one is a point:
int HMPointD_AtPoint(Word* args, Word& result, int message,
                Word& local, Supplier s)
{
  result = qp->ResultStorage( s );

  HMPoint *m = ((HMPoint*)args[0].addr);
  Point* p = ((Point*)args[1].addr);
  temporalalgebra::MPoint* pResult = (temporalalgebra::MPoint*)result.addr;
  temporalalgebra::MPoint aux( 0 );

  m->D_At(*p, aux);
  pResult->CopyFrom( &aux );

  return 0;
}

// If the first argument is a HMPoint and the second one is a region:
int HMPointD_AtRegion(Word* args, Word& result, int message,
                Word& local, Supplier s)
{
  result = qp->ResultStorage( s );

  HMPoint *m = ((HMPoint*)args[0].addr);
  Region* r = ((Region*)args[1].addr);
  temporalalgebra::MPoint* pResult = (temporalalgebra::MPoint*)result.addr;
  temporalalgebra::MPoint aux( 0 );

  m->D_At(*r, aux);
  pResult->CopyFrom( &aux );

  return 0;
}

/*
6.4.18 value mapping functions for operator ~getcmpoint~

*/
int HMPointReduceHierarchy( Word* args, Word& result, int message, Word& local,
                                  Supplier s )
{
  result = qp->ResultStorage( s );
  HMPoint *hmp = static_cast<HMPoint*>(args[0].addr);
  CcReal *epsilon = static_cast<CcReal*>(args[1].addr);
  HCMPoint* pResult = (HCMPoint*)result.addr;
  HCMPoint aux( 0 );

  if( hmp->IsDefined() )
  {
    hmp->ReduceHierarchy( epsilon->GetValue(), aux );
    pResult->CopyFrom( &aux );
  }
  else
    pResult->SetDefined(false);

  return 0;
}

/*
6.4.19 Value mapping function for operator ~atinstant~

*/
int HMPointAtInstant( Word* args, Word& result, int message,
                          Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  HMPoint* hmp = ((HMPoint*)args[0].addr);
  Instant* inst = (Instant*) args[1].addr;
  temporalalgebra::Intime<Point>* pResult = 
                     (temporalalgebra::Intime<Point>*)result.addr;

  hmp->AtInstant(*inst, *pResult);
  return 0;
}

/*
6.4.20 Value mapping function for operator ~atperiods~

*/
int HMPointAtPeriods( Word* args, Word& result, int message,
                          Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  HMPoint* hmp = ((HMPoint*)args[0].addr);
  temporalalgebra::Periods* per = (temporalalgebra::Periods*)args[1].addr;
  temporalalgebra::MPoint* pResult = (temporalalgebra::MPoint*)result.addr;

  hmp->AtPeriods(*per,*pResult);
  return 0;
}

/*
6.5 Value Mapping Functions for type ~HCMPoint~

6.5.1 value mapping functions for operator ~generalize~

*/

int GeneralizeCMPoint( Word* args, Word& result, int message, Word& local,
                                  Supplier s )
{
  result = qp->ResultStorage( s );
  CMPoint *m = static_cast<CMPoint*>(args[0].addr);
  CcReal *e = static_cast<CcReal*>(args[1].addr);
  CcReal *f = static_cast<CcReal*>(args[2].addr);
  HCMPoint* pResult = (HCMPoint*)result.addr;
  DateTime dt(0, 0, durationtype);

  if( m->IsDefined() && e->IsDefined() && f->IsDefined() )
    Generalize( static_cast<double>(e->GetValue()),
                static_cast<double>(f->GetValue()),-1,*m,false,dt,*pResult );
  else
    pResult->Clear();

  return 0;
}

/*
6.5.2 value mapping function for operator ~epsilon~

*/
int HCMPointEpsilon( Word* args, Word& result, int message, Word& local,
                                  Supplier s )
{
  result = qp->ResultStorage( s );
  HCMPoint* hcmp  = static_cast<HCMPoint*>(args[0].addr);

  if( hcmp->IsDefined() )
  {
    if(hcmp->LayerSize(4) > 0)
      ((CcReal*)result.addr)->Set( hcmp->GetLayer4epsilon() );
    else if( hcmp->LayerSize(3) > 0 )
      ((CcReal*)result.addr)->Set( hcmp->GetLayer3epsilon() );
    else if( hcmp->LayerSize(2) > 0 )
      ((CcReal*)result.addr)->Set( hcmp->GetLayer2epsilon() );
    else if( hcmp->LayerSize(1) > 0 )
      ((CcReal*)result.addr)->Set( hcmp->GetLayer1epsilon() );
    else if( hcmp->LayerSize(0) > 0 )
      ((CcReal*)result.addr)->Set( hcmp->GetLayer0epsilon() );
    else
      ((CcReal*)result.addr)->SetDefined( false );
  }
  else
    ((CcReal*)result.addr)->SetDefined( false );

  return 0;
}

/*
6.5.2 Value mapping function for operator ~no\_components~

*/
int HCMPointNoComponents( Word* args, Word& result, int message, Word& local,
                                  Supplier s )
{
  result = qp->ResultStorage( s );
  HCMPoint* hcmp  = static_cast<HCMPoint*>(args[0].addr);

  if( hcmp->IsDefined() )
  {
    ((CcInt*)result.addr)->Set( hcmp->GetNoComponents() );
  }
  else
    ((CcInt*)result.addr)->SetDefined( false );

  return 0;
}

/*
6.3.2 Value mapping function for operator ~trajectory~

*/
int HCMPointTrajectory( Word* args, Word& result, int message,
 Word& local, Supplier s )
{
  // +++++ for debugging purposes only +++++
  //cout << "Value Mapping Function CMPointTrajectory is called!\n";

  HCMPoint* hcmp = static_cast<HCMPoint*>(args[0].addr);

  int size = 0;
  int i = 5;
  while( size == 0 )
  {
    i--;
    size = hcmp->LayerSize( i );
  }
  double e = hcmp->GetLayerepsilon( i );
  CMPoint cmp( 0 );
  hcmp->GetCMPoint( e, cmp );


  ostringstream strTrajectPtr;
  string querystring;
  result = qp->ResultStorage( s );

  if( &cmp > 0 )
  {
    // create the query list:
    strTrajectPtr << (long)&cmp;

    // +++++ for debugging purposes only +++
    //cout << "Das aufgerufene Objekt hat die Adresse: " << strTrajectPtr.str()
    //  << ".\n";

    querystring =
    "(aggregateB"
      "(projectextend"
        "(namedtransformstream"
          "(units (cmpoint (ptr " + strTrajectPtr.str() + ")) )"
          "Unit)"
        "()"
        "("
          "(regions"
            "(fun"
              "(tuple1 TUPLE)"
                "(trajectory"
                  "(attr tuple1 Unit))))))"
      "regions"
      "(fun"
        "(r1 region)"
        "(r2 region)"
        "(union_new r1 r2))"
      "(region"
        "()))";

    // +++++ for debugging purposes only +++++
    //cout << querystring << endl;
    if( !QueryProcessor::ExecuteQuery(querystring, result) )
      cout << "Error in executing operator query" << endl;
  }

  cmp.DeleteIfAllowed();

  return 0;
}

/*
6.5.3 value mapping function for operator ~getcmpoint~

*/
int HCMPointGetCMPoint( Word* args, Word& result, int message, Word& local,
                                  Supplier s )
{
  result = qp->ResultStorage( s );
  HCMPoint *hmp = static_cast<HCMPoint*>(args[0].addr);
  CcReal *epsilon = static_cast<CcReal*>(args[1].addr);
  CMPoint* pResult = (CMPoint*)result.addr;
  CMPoint aux( 0 );

  if( hmp->IsDefined() )
  {
    hmp->GetCMPoint( epsilon->GetValue(), aux );
    pResult->CopyFrom( &aux );
  }
  else
    pResult->SetDefined(false);

  return 0;
}

/*
6.5.1 value mapping function for operator ~deftime~

*/
int HCMPointDeftime( Word* args, Word& result, int message, Word& local,
                                  Supplier s )
{
  result = qp->ResultStorage( s );
  HCMPoint* hcmp = static_cast<HCMPoint*>(args[0].addr);
  temporalalgebra::Periods* pResult = (temporalalgebra::Periods*)result.addr;
  temporalalgebra::Periods aux( 0 );

  if( hcmp->IsDefined() )
  {
    hcmp->DefTime( aux );
    pResult->CopyFrom( &aux );
  }
  else
    pResult->SetDefined(false);

  return 0;
}

/*
6.5.4 Value Mapping function for operator ~present~

*/

// If the second argument is an Instant:
int HCMPointPresent_i( Word* args, Word& result,
                     int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );

  HCMPoint *hcmp = ((HCMPoint*)args[0].addr);
  Instant* inst = ((Instant*)args[1].addr);

  if( !inst->IsDefined() )
    ((CcBool *)result.addr)->Set( false, false );
  else if( hcmp->Present( *inst ) )
    ((CcBool *)result.addr)->Set( true, true );
  else
    ((CcBool *)result.addr)->Set( true, false );

  return 0;
}

// If the second argument is a Period:
int HCMPointPresent_p( Word* args, Word& result,
                         int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );

  HCMPoint *hcmp = ((HCMPoint*)args[0].addr);
  temporalalgebra::Periods* periods = ((temporalalgebra::Periods*)args[1].addr);

  if( periods->IsEmpty() )
    ((CcBool *)result.addr)->Set( false, false );
  else if( hcmp->Present( *periods ) )
    ((CcBool *)result.addr)->Set( true, true );
  else
    ((CcBool *)result.addr)->Set( true, false );

  return 0;
}

/*
6.5.5 Value Mapping functions for operator ~d\_passes~

*/

// If the second argument is a Point:
int HCMPointD_PassesPoint(Word* args, Word& result, int message,
                Word& local, Supplier s)
{
  result = qp->ResultStorage( s );

  HCMPoint *hcmp = ((HCMPoint*)args[0].addr);
  Point* p = ((Point*)args[1].addr);

  if( !p->IsDefined() )
    ((CcBool *)result.addr)->Set( false, false );
  else if( hcmp->D_Passes( *p ) )
    ((CcBool *)result.addr)->Set( true, true );
  else
    ((CcBool *)result.addr)->Set( true, false );

  return 0;
}

// If the second argument is a Region:
int HCMPointD_PassesRegion(Word* args, Word& result, int message,
                Word& local, Supplier s)
{
  result = qp->ResultStorage( s );

  HCMPoint *hcmp = ((HCMPoint*)args[0].addr);
  Region* r = ((Region*)args[1].addr);

  if( !r->IsDefined() )
    ((CcBool *)result.addr)->Set( false, false );
  else if( hcmp->D_Passes( *r ) )
    ((CcBool *)result.addr)->Set( true, true );
  else
    ((CcBool *)result.addr)->Set( true, false );

  return 0;
}

/*
6.5.5 Value Mapping functions for operator ~p\_passes~

*/

// If the second argument is a Point:
int HCMPointP_PassesPoint(Word* args, Word& result, int message,
                Word& local, Supplier s)
{
  result = qp->ResultStorage( s );

  HCMPoint *hcmp = ((HCMPoint*)args[0].addr);
  Point* p = ((Point*)args[1].addr);

  if( !p->IsDefined() )
    ((CcBool *)result.addr)->Set( false, false );
  else if( hcmp->P_Passes( *p ) )
    ((CcBool *)result.addr)->Set( true, true );
  else
    ((CcBool *)result.addr)->Set( true, false );

  return 0;
}

// If the second argument is a Region:
int HCMPointP_PassesRegion(Word* args, Word& result, int message,
                Word& local, Supplier s)
{
  result = qp->ResultStorage( s );

  HCMPoint *hcmp = ((HCMPoint*)args[0].addr);
  Region* r = ((Region*)args[1].addr);

  if( !r->IsDefined() )
    ((CcBool *)result.addr)->Set( false, false );
  else if( hcmp->P_Passes( *r ) )
    ((CcBool *)result.addr)->Set( true, true );
  else
    ((CcBool *)result.addr)->Set( true, false );

  return 0;
}

/*
6.5.6 value mapping functions for operator ~d\_at~

*/

// If the first argument is a HCMPoint and the second one is a point:
int HCMPointD_AtPoint(Word* args, Word& result, int message,
                Word& local, Supplier s)
{
  result = qp->ResultStorage( s );

  HCMPoint *m = ((HCMPoint*)args[0].addr);
  Point* p = ((Point*)args[1].addr);
  CMPoint* pResult = (CMPoint*)result.addr;
  CMPoint aux( 0 );

  m->D_At(*p, aux);
  pResult->CopyFrom( &aux );

  return 0;
}

// If the first argument is a HCMPoint and the second one is a region:
int HCMPointD_AtRegion(Word* args, Word& result, int message,
                Word& local, Supplier s)
{
  result = qp->ResultStorage( s );

  HCMPoint *m = ((HCMPoint*)args[0].addr);
  Region* r = ((Region*)args[1].addr);
  CMPoint* pResult = (CMPoint*)result.addr;
  CMPoint aux( 0 );

  m->D_At(*r, aux);
  pResult->CopyFrom( &aux );

  return 0;
}


/*
6.5.7 value mapping functions for operator ~p\_at~

*/

// If the first argument is a HCMPoint and the second one is a point:
int HCMPointP_AtPoint(Word* args, Word& result, int message,
                Word& local, Supplier s)
{
  result = qp->ResultStorage( s );

  HCMPoint *m = ((HCMPoint*)args[0].addr);
  Point* p = ((Point*)args[1].addr);
  CMPoint* pResult = (CMPoint*)result.addr;
  CMPoint aux( 0 );

  m->P_At(*p, aux);
  pResult->CopyFrom( &aux );

  return 0;
}

// If the first argument is a HCMPoint and the second one is a region:
int HCMPointP_AtRegion(Word* args, Word& result, int message,
                Word& local, Supplier s)
{
  result = qp->ResultStorage( s );

  HCMPoint *m = ((HCMPoint*)args[0].addr);
  Region* r = ((Region*)args[1].addr);
  CMPoint* pResult = (CMPoint*)result.addr;
  CMPoint aux( 0 );

  m->P_At(*r, aux);
  pResult->CopyFrom( &aux );

  return 0;
}

/*
6.5.4 Value mapping function for operator ~atinstant~

*/
int HCMPointAtInstant( Word* args, Word& result, int message,
                          Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  HCMPoint* hcmp = ((HCMPoint*)args[0].addr);
  Instant* inst = (Instant*) args[1].addr;
  temporalalgebra::Intime<Region>* pResult = 
                     (temporalalgebra::Intime<Region>*)result.addr;
  temporalalgebra::IRegion aux(false);

  hcmp->AtInstant(*inst, aux);
  pResult->CopyFrom( &aux );
  return 0;
}

/*
6.5.20 Value mapping function for operator ~atperiods~

*/
int HCMPointAtPeriods( Word* args, Word& result, int message,
                          Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  HCMPoint* hcmp = ((HCMPoint*)args[0].addr);
  temporalalgebra::Periods* per = (temporalalgebra::Periods*)args[1].addr;
  CMPoint* pResult = (CMPoint*)result.addr;
  CMPoint aux( 0 );

  hcmp->AtPeriods(*per,aux);
  pResult->CopyFrom( &aux );

  return 0;
}

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
                                      CUPointEpsilon,
                                      CMPointEpsilon,
                                      HCMPointEpsilon };

ValueMapping hierarchicalnocomponentsmap[] = {
                                      CMPointNoComponents,
                                      HCMPointNoComponents,
                                      HMPointNoComponents };

ValueMapping uncertaintrajectorymap[] = {
                                      CUPointTrajectory,
                                      CMPointTrajectory,
                                      HCMPointTrajectory,
                                      HMPointTrajectory };

ValueMapping uncertaintemporalpresentmap[] = {
                                      CMPointPresent_i,
                                      CMPointPresent_p,
                                      HCMPointPresent_i,
                                      HCMPointPresent_p,
                                      HCMPointPresent_i,
                                      HCMPointPresent_p };

ValueMapping uncertaindpassesmap[] = {
                                      CUPointD_PassesPoint,
                                      CUPointD_PassesRegion,
                                      CMPointD_PassesPoint,
                                      CMPointD_PassesRegion,
                                      HCMPointD_PassesPoint,
                                      HCMPointD_PassesRegion,
                                      HMPointD_PassesPoint,
                                      HMPointD_PassesRegion };

ValueMapping hierarchicalpassesmap[] = {
                                      HMPointD_PassesPoint,
                                      HMPointD_PassesRegion };

ValueMapping uncertainppassesmap[] = {
                                      CUPointP_PassesPoint,
                                      CUPointP_PassesRegion,
                                      CMPointP_PassesPoint,
                                      CMPointP_PassesRegion,
                                      HCMPointP_PassesPoint,
                                      HCMPointP_PassesRegion };

ValueMapping uncertaintemporalatinstantmap[] = {
                                      CUPointAtInstant,
                                      CMPointAtInstant,
                                      HCMPointAtInstant,
                                      HMPointAtInstant };

ValueMapping hierarchicaltemporalatperiodsmap[] = {
                                      CMPointAtPeriods,
                                      HCMPointAtPeriods,
                                      HMPointAtPeriods };

ValueMapping uncertaindatmap[] = {
                                      CUPointD_AtPoint,
                                      CUPointD_AtRegion,
                                      CMPointD_AtPoint,
                                      CMPointD_AtRegion,
                                      HCMPointD_AtPoint,
                                      HCMPointD_AtRegion,
                                      HMPointD_AtPoint,
                                      HMPointD_AtRegion };

ValueMapping hierarchicalatmap[] = {
                                      HMPointD_AtPoint,
                                      HMPointD_AtRegion };

ValueMapping uncertainpatmap[] = {
                                      CUPointP_AtPoint,
                                      CUPointP_AtRegion,
                                      CMPointP_AtPoint,
                                      CMPointP_AtRegion,
                                      HCMPointP_AtPoint,
                                      HCMPointP_AtRegion };

ValueMapping temporaltouncertainmap[] = {
                                      CUPointToUncertain,
                                      CMPointToUncertain };

ValueMapping hierarchicalgetcmpointmap[] = {
                                      HMPointGetCMPoint,
                                      HCMPointGetCMPoint };

ValueMapping hierarchicaldeftimemap[] = {
                                   temporalalgebra::MappingDefTime<CMPoint>,
                                   HCMPointDeftime,
                                   HCMPointDeftime };

ValueMapping generalizemap[] = {      GeneralizeMPoint,
                                      GeneralizeCMPoint };

/*
Specification strings

*/

const string UncertainSpecEpsilon  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>cupoint||cmpoint||hcmpoint -> epsilon</text--->"
  "<text>epsilon ( _ )</text--->"
  "<text>Returns an uncertain values' epsilon value.</text--->"
  "<text>epsilon ( i1 )</text--->"
  ") )";

const string HierarchicalSpecNoComponents  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>uT -> epsilon</text--->"
  "<text>epsilon ( _ )</text--->"
  "<text>Returns an uncertain values' epsilon value.</text--->"
  "<text>epsilon ( i1 )</text--->"
  ") )";

const string CPointSpecToCPoint =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>point, real -> cpoint</text--->"
  "<text>toCPoint ( _, _ )</text--->"
  "<text>Builds a new CPoint from the given Real- and Point-values.</text--->"
  "<text>cpt = tocpoint ( 50.0, alexanderplatz )</text--->"
  ") )";

const string TemporalSpecDefTime  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>uncertain/hierarchical moving point -> periods</text--->"
  "<text>deftime( _ )</text--->"
  "<text>Gets the defined time of the corresponding uncertain or hierarchical"
  " moving point.</text--->"
  "<text>deftime( cmp1 )</text--->"
  ") )";

const string UncertainMovingSpecTrajectory =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>cupoint || cmpoint -> region</text--->"
  "<text>trajectory ( _ )</text--->"
  "<text>Returns a Region-Object, representing the possible trajectory-area "
  "of the given uncertain unit/moving point.</text--->"
  "<text>query trajectory( cuphavel ) || query trajectory( \"cmphavel\" "
  "</text--->) )";

const string UncertainTemporalSpecPresent  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>(cmT || hmT instant) -> bool,\n"
  "(cmT || hmT periods) -> bool</text--->"
  "<text>_ present _ </text--->"
  "<text>Checks whether the moving object is present at the given "
  "instant or period.</text--->"
  "<text>htrain7 present instant1</text--->"
  ") )";

const string UncertainTemporalSpecAtInstant =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>(cupoint||cmpoint||hcmpoint instant) -> iregion</text--->"
  "<text>_ atinstant _ </text--->"
  "<text>From an uncertain or hierarchical moving point, get the intime "
  "region representing the uncertain point of the instant.</text--->"
  "<text>cmpoint1 atinstant instant1</text--->"
  ") )";

const string UncertainTemporalSpecAtPeriods =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>(cmpoint||hcmpoint periods) -> cmpoint "
  "(hmpoint periods) -> mpoint</text--->"
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
  "( <text>(cmpoint||cupoint||hcmpoint||hmpoint x point||region ) -> "
  "bool</text--->"
  "<text>_ d_passes _ </text--->"
  "<text>Checks whether the uncertain moving object definitely passes the "
  "given spatial object.</text--->"
  "<text>htrain7 d_passes mehringdamm</text--->"
  ") )";

const string HierarchicalTemporalSpecPasses =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>(hmpoint x point||region ) -> bool</text--->"
  "<text>_ passes _ </text--->"
  "<text>Checks whether the hierarchical moving point passes the given "
  "spatial object.</text--->"
  "<text>htrain7 passes thecenter</text--->"
  ") )";

const string UncertainTemporalSpecPPasses =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>(cmpoint||cupoint||hcmpoint x point||region ) -> bool</text--->"
  "<text>_ d_passes _ </text--->"
  "<text>Checks whether the uncertain moving object possibly passes the "
  "given spatial object.</text--->"
  "<text>hctrain7 p_passes thecenter</text--->"
  ") )";

const string UncertainTemporalSpecDAt =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>(cupoint x point) -> cupoint \n"
  "(cmpoint||hcmpoint x point) -> cmpoint \n"
  "(cupoint||cmpoint||hcmpoint x region) -> cmpoint </text--->"
  "<text> _ d_at _ </text--->"
  "<text>Restricts the moving object to the times where its value "
  "equals the given value.</text--->"
  "<text>ctrain7 d_at thecenter</text--->"
  ") )";

const string HierarchicalTemporalSpecAt =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>(hmpoint x point||region) -> mpoint \n </text--->"
  "<text> _ at _ </text--->"
  "<text>Restricts the moving object to the times where its value "
  "equals the given value.</text--->"
  "<text>htrain7 at havel</text--->"
  ") )";

const string UncertainTemporalSpecPAt =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>(cupoint x point) -> cupoint \n"
  "(cmpoint||hcmpoint x point) -> cmpoint \n"
  "(cupoint||cmpoint||hcmpoint x region) -> cmpoint </text--->"
  "<text> _ p_at _ </text--->"
  "<text>Restricts the moving object to the times where its value "
  "equals the given value.</text--->"
  "<text>cmphavel p_at havel</text--->"
  ") )";

const string TemporalSpecToUncertain =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>(upoint x real) -> cupoint \n"
  "(mpoint x real) -> cmpoint </text--->"
  "<text> touncertain( _, _ ) </text--->"
  "<text>Creates an uncertain object from the given object, simply by "
  "interpreting the real value as the uncertainty-value.</text--->"
  "<text>query touncertain( train7, 23.4 )</text--->"
  ") )";

const string MovingSpecGeneralize =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>(mpoint x real x real) -> hmpoint"
  " (cmpoint x real x real) -> hcmpoint</text--->"
  "<text> generalize( _, _, _ ) </text--->"
  "<text>Creates up to 5 generalizations from the given mpoint or cmpoint, "
  "using the second argument as the initial epsilon and the third argument as "
  "a factor to increase the epsilon value.</text--->"
  "<text>let htrain7 = generalize( train7, 25.0, 1.5  )</text--->"
  ") )";

const string HierarchicalMovingSpecGetMPoint =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>(hmpoint) -> mpoint </text--->"
  "<text> getmpoint( _ ) </text--->"
  "<text>Extracts the original mpoint, from a hierarchical moving point"
  "(hmpoint).</text--->"
  "<text>let htrain7 = getmpoint( htrain7 )</text--->"
  ") )";

const string HierarchicalMovingSpecGetCMPoint =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>(hmpoint || hcmpoint) -> mcpoint </text--->"
  "<text> getcmpoint( _ ) </text--->"
  "<text>Extracts that cmpoint from a hierarchical moving point, that has the"
  "greatest uncertainty-value, that is less than the given real value."
  "</text--->"
  "<text>let htrain7 = getcmpoint( htrain7, 20.0 )</text--->"
  ") )";

const string HierarchicalMovingSpecReduceHierarchy =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>(hmpoint real) -> hcmoint </text--->"
  "<text> reduce_hierarchy( _, _ ) </text--->"
  "<text>Creates a HCMPoint-Object containing just the layers down to that "
  "layer which is necessary to represent a cmpoint with the given epsilon-"
  "value.</text--->"
  "<text>let hctrain7 = scale( htrain7, 20.0 )</text--->"
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

Operator hierarchicalnocomponents( "no_components",
                              HierarchicalSpecNoComponents,
                              3,
                              hierarchicalnocomponentsmap,
                              HierarchicalSimpleSelect,
                              HierarchicalTypeMapInt );

Operator uncertaintrajectory( "trajectory",
                              UncertainMovingSpecTrajectory,
                              4,
                              uncertaintrajectorymap,
                              UncertainTemporalSelect,
                              UncertainMovingTypeMapSpatial);

Operator uncertaintemporaldeftime( "deftime",
                              TemporalSpecDefTime,
                              3,
                              hierarchicaldeftimemap,
                              HierarchicalSimpleSelect,
                              UncertainMovingTypeMapPeriods );

Operator uncertaintemporalpresent( "present",
                              UncertainTemporalSpecPresent,
                              6,
                              uncertaintemporalpresentmap,
                              UncertainMovingInstantPeriodsSelect,
                              UncertainMovingInstantPeriodsTypeMapBool);

Operator uncertaintemporalatinstant( "atinstant",
                              UncertainTemporalSpecAtInstant,
                              4,
                              uncertaintemporalatinstantmap,
                              UncertainTemporalAtTimeSelect,
                              UncertainMovingInstantTypeMapIntime );

Operator uncertaintemporalatperiods( "atperiods",
                              UncertainTemporalSpecAtPeriods,
                              3,
                              hierarchicaltemporalatperiodsmap,
                              HierarchicalSimpleSelect,
                              UncertainMovingPeriodsTypeMapMoving );

Operator uncertaintemporalunits( "units",
                              UncertainTemporalSpecUnits,
                              UncertainMappingUnits,
                              Operator::SimpleSelect,
                              UncertainTemporalTypeMapUnits );

Operator uncertaintemporaldpasses( "d_passes",
                              UncertainTemporalSpecDPasses,
                              8,
                              uncertaindpassesmap,
                              UncertainPassesSelect,
                              UncertainDPassesTypeMapBool );

Operator hierarchicaltemporalpasses( "passes",
                              HierarchicalTemporalSpecPasses,
                              2,
                              hierarchicalpassesmap,
                              HierarchicalPassesSelect,
                              HierarchicalPassesTypeMapBool );

Operator uncertaintemporalppasses( "p_passes",
                              UncertainTemporalSpecPPasses,
                              6,
                              uncertainppassesmap,
                              UncertainPassesSelect,
                              UncertainPPassesTypeMapBool );

Operator uncertaintemporaldat( "d_at",
                              UncertainTemporalSpecDAt,
                              8,
                              uncertaindatmap,
                              UncertainPassesSelect,
                              UncertainDAtTypeMapMoving );

Operator hierarchicaltemporalat( "at",
                              HierarchicalTemporalSpecAt,
                              2,
                              hierarchicalatmap,
                              HierarchicalPassesSelect,
                              HierarchicalAtTypeMapMoving );

Operator uncertaintemporalpat( "p_at",
                              UncertainTemporalSpecPAt,
                              6,
                              uncertainpatmap,
                              UncertainPassesSelect,
                              UncertainPAtTypeMapMoving );

Operator temporaltouncertain( "touncertain",
                              TemporalSpecToUncertain,
                              2,
                              temporaltouncertainmap,
                              TemporalToUncertainSelect,
                              UncertainTypeMapBaseToUncertain );

Operator movingpointgeneralize( "generalize",
                              MovingSpecGeneralize,
                              2,
                              generalizemap,
                              TemporalToHierarchySelect,
                              MovingTypeMapHierarchy );

Operator hierarchicalmovingpointgetmpoint( "getmpoint",
                              HierarchicalMovingSpecGetMPoint,
                              HMPointGetMPoint,
                              Operator::SimpleSelect,
                              HierarchicalMovingTypeMapMoving );

Operator hierarchicalmovingpointgetcmpoint( "getcmpoint",
                              HierarchicalMovingSpecGetCMPoint,
                              2,
                              hierarchicalgetcmpointmap,
                              HierarchicalToUncertainSelect,
                              HierarchicalMovingTypeMapUncertainMoving );

Operator hierarchicalmovingpointreducehierarchy( "reduce_hierarchy",
                              HierarchicalMovingSpecReduceHierarchy,
                              HMPointReduceHierarchy,
                              Operator::SimpleSelect,
                              HierarchicalTypeMapHierarchical );

/*
Creating the Algebra

*/
class HierarchicalGeoAlgebra : public Algebra
{
  public:
  HierarchicalGeoAlgebra() : Algebra()
  {
    AddTypeConstructor( &uncertainunitpoint );
    uncertainunitpoint.AssociateKind( Kind::DATA() );
    uncertainunitpoint.AssociateKind( Kind::UNCERTAIN() );
    uncertainunitpoint.AssociateKind( Kind::TEMPORAL() );

    AddTypeConstructor( &uncertainmovingpoint );
    uncertainmovingpoint.AssociateKind( Kind::DATA() );
    uncertainmovingpoint.AssociateKind( Kind::UNCERTAIN() );
    uncertainmovingpoint.AssociateKind( Kind::TEMPORAL() );

    AddTypeConstructor( &hierarchicaluncertainunitpoint );
    hierarchicaluncertainunitpoint.AssociateKind( Kind::DATA() );
    hierarchicaluncertainunitpoint.AssociateKind( Kind::UNCERTAIN() );
    hierarchicaluncertainunitpoint.AssociateKind( Kind::HIERARCHICAL() );

    AddTypeConstructor( &hierarchicalmovingpoint );
    hierarchicalmovingpoint.AssociateKind( Kind::DATA() );
    hierarchicalmovingpoint.AssociateKind( Kind::UNCERTAIN() );
    hierarchicalmovingpoint.AssociateKind( Kind::HIERARCHICAL() );

    AddTypeConstructor( &hierarchicaluncertainmovingpoint );
    hierarchicaluncertainmovingpoint.AssociateKind( Kind::DATA() );
    hierarchicaluncertainmovingpoint.AssociateKind( Kind::UNCERTAIN() );
    hierarchicaluncertainmovingpoint.AssociateKind( Kind::HIERARCHICAL() );

    AddOperator( &uncertainepsilon );
    AddOperator( &uncertaintrajectory );
    AddOperator( &uncertaintemporaldeftime );
    AddOperator( &uncertaintemporalpresent );
    AddOperator( &uncertaintemporalatinstant );
    AddOperator( &uncertaintemporalatperiods );
    AddOperator( &uncertaintemporalunits );
    AddOperator( &uncertaintemporaldpasses );
    AddOperator( &uncertaintemporalppasses );
    AddOperator( &uncertaintemporaldat );
    AddOperator( &uncertaintemporalpat );
    AddOperator( &temporaltouncertain );
    AddOperator( &movingpointgeneralize );
    AddOperator( &hierarchicalmovingpointgetmpoint );
    AddOperator( &hierarchicalmovingpointgetcmpoint );
    AddOperator( &hierarchicaltemporalpasses );
    AddOperator( &hierarchicalmovingpointreducehierarchy );
    AddOperator( &hierarchicalnocomponents );
    AddOperator( &hierarchicaltemporalat );
  }
  ~HierarchicalGeoAlgebra() {};
};

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
  return (new HierarchicalGeoAlgebra());
}

