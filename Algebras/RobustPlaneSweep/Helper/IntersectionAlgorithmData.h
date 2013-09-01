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

[1] Header File for the class ~IntersectionAlgorithmData~

[TOC]

1 Overview

This header file contains the class ~IntersectionAlgorithmData~.

The class ~IntersectionAlgorithmData~ is used to control the 
~BentleyOttmann~ and the ~Hobby~ class.

1 Includes

*/

#pragma once

#ifdef RPS_TEST
#include "../Helper/SpatialAlgebraStubs.h"
#else
#include "SpatialAlgebra.h"
#endif

namespace RobustPlaneSweep
{
typedef int HalfSegmentIntersectionId;

/*

1 Forward declaration of class ~InternalAttribute~

*/
class InternalAttribute;

/*

1 Class ~IntersectionAlgorithmData~

*/
class IntersectionAlgorithmData
{
public:
/*

1.1 ~FirstGeometryIsRegion~

Must return true iff the first geometry is a region. This parameter
controls wether a segment attribute is Boundary or InteriorAbove/Below.

*/
  virtual bool FirstGeometryIsRegion() const = 0;

/*

1.1 ~SecondGeometryIsRegion~

Must return true iff the second geometry is a region. This parameter
controls wether a segment attribute is Boundary or InteriorAbove/Below.

*/
  virtual bool SecondGeometryIsRegion() const = 0;

/*

1.1 ~InitializeFetch~

Is called before the first ~FetchInput~ call. Can be used to initialize 
counters, indexes etc. 

*/
  virtual void InitializeFetch() = 0;

/*

1.1 ~FetchInput~

This method is called to fetch the next half segment or point. 
If the return value is false, then there is no more data. 
If the point is defined (~IsDefined()~), then the next data item is a point 
otherwise the next data item is a half segment.
The ~belongsToSecondGeometry~ parameter denotes, wether the data item 
belongs to the first or the second geometry.

*/
  virtual bool FetchInput(HalfSegment &segment,
                          Point& point,
                          bool &belongsToSecondGeometry) = 0;

/*

1.1 ~GetHalfSegmentId~

Intented to recognize the internal segment, when the half segment with the
right dominating point is read. 

Not used at the moment. 

*/
  virtual HalfSegmentIntersectionId
  GetHalfSegmentId(const HalfSegment& /*segment*/) const
  {
    return 0;
  }

/*

1.1 ~OutputData~

Iff ~OutputData~ returns true, then 
~OutputHalfSegment~ and/or ~OutputPoint~ are called.

*/
  virtual bool OutputData() const
  {
    return false;
  }

/*

1.1 ~OutputHalfSegment~

Called to output a half segment. 
The attribute can be used to filter the result 
(e.g. for boolean set operations).

*/
  virtual void OutputHalfSegment(const HalfSegment& /*segment*/,
                                 const InternalAttribute& /*attribute*/)
  {
    throw new std::logic_error("there shouldn't be "
                               "any halfsegments to output!");
  }

/*

1.1 ~OutputPoint~

Called to output a point. This method is only called, if points were read. 
This method is not called for intersection points. 
The attribute can be used to filter the 
result (e.g. for boolean set operations).

*/
  virtual void OutputPoint(const Point& /*point*/,
                           const InternalAttribute& /*attribute*/)
  {
    throw new std::logic_error("there shouldn't be "
                               "any points to output!");
  }

/*

1.1 ~GetBoundingBox~

This method is called to determine the transformation parameters for the 
internal point transformation. The return value must not be smaller then 
the actual input data. 

*/
  virtual const Rectangle<2> GetBoundingBox() = 0;

/*

1.1 ~IsInputOrderedByX~

If ~IsInputOrderedByX~ returns true, then the input is assumed to be ordered by
the x-coordinate. Otherwise, the complete input data is read and sorted inside 
the ~BentleyOttmann~ class.

*/
  virtual bool IsInputOrderedByX() const
  {
    return true;
  }

/*

1.1 ~GetRoundToDecimals~

This method is called to determine the decimal places of the result. 

By default, the result coordinates are rounded to  to eight decimal places. 
Due to numeric imprecision the rounded coordinates can have a distance greater
zero, but less than 10\^(-8) (the ~AlmostEqual~ distance). With the ~stepSize~
parameter value 2 the last decimal place of the coordinates are rounded 
to 0, 2, 4, 6 or 8. 

*/
  virtual void GetRoundToDecimals(int& decimals, int& stepSize) const
  {
    // because of AlmostEqual
    decimals = 8;
    stepSize = 2;
  }

/*

1.1 ~OnGeometryIntersectionFound~

This method is called, once the first intersection is found. 
If the method returns true, then the sweep is aborted. 

Useful for predicates like ~intersects~.

*/
  virtual bool OnGeometryIntersectionFound()
  {
    return false;
  }

/*

1.1 ~OutputFinished~

This method is called, after the sweep is finished.

*/
  virtual void OutputFinished()
  {
  }

/*

1.1 ~ReportIntersections~

Iff this method returns true, then the ~ReportIntersection~ method is called 
for each found intersection point.

*/
  virtual bool ReportIntersections() const
  {
    return false;
  }

/*

1.1 ~ReportIntersection~

This method is called if an intersection point is found. 

*/
  virtual void ReportIntersection(const Point& /*intersectionPoint*/,
                                  const bool /*overlappingIntersection*/)
  {
    throw new std::logic_error("there shouldn't be "
                               "any intersections to report!");
  }

/*

1.1 Destructor

*/
  virtual ~IntersectionAlgorithmData()
  {
  }
};
}
