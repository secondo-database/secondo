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

[1] Implementation file for the class ~IntersectionAlgorithm~

[TOC]

1 Overview

This file contains all structs and classes required for the 
class ~IntersectionAlgorithm~.

This class is the base class for all intersection algorithm classes.

1 Includes

*/
#include <algorithm>

#include "IntersectionAlgorithm.h"

using namespace std;

namespace RobustPlaneSweep
{
/*

1 Class ~IntersectionAlgorithm~

1.1 ~OverlappingSegmentsSortComparer~

*/
int IntersectionAlgorithm::
OverlappingSegmentsSortComparer(const InternalResultLineSegment& s0,
                                const InternalResultLineSegment& s1)
{
  bool xReverseStartEnd, yReverseStartEnd;

  xReverseStartEnd = InternalPoint::Compare(s0.GetStart(), s0.GetEnd()) < 0;
  yReverseStartEnd = InternalPoint::Compare(s1.GetStart(), s1.GetEnd()) < 0;

  int result = InternalPoint::Compare((!xReverseStartEnd ? s0.GetStart() :
                                                           s0.GetEnd()),
                                      (!yReverseStartEnd ? s1.GetStart() :
                                                           s1.GetEnd()));

  if (result == 0) {
    result = InternalPoint::Compare((!xReverseStartEnd ? s0.GetEnd() :
                                                         s0.GetStart()),
                                    (!yReverseStartEnd ? s1.GetEnd() :
                                                         s1.GetStart()));
  }

  if (result == 0) {
    if (s0.GetStart().GetX() != s0.GetEnd().GetX()) {
      int x = s0.GetStart().GetX()
              + ((s0.GetEnd().GetX() - s0.GetStart().GetX()) / 2);

      Rational s0a =
          Rational(s0.GetOriginalStart().GetY() - s0.GetOriginalEnd().GetY(),
                   s0.GetOriginalStart().GetX() - s0.GetOriginalEnd().GetX());

      Rational s1a =
          Rational(s1.GetOriginalStart().GetY() - s1.GetOriginalEnd().GetY(),
                   s1.GetOriginalStart().GetX() - s1.GetOriginalEnd().GetX());

      Rational p0 = s0a * (x - s0.GetOriginalStart().GetX())
                    + s0.GetOriginalStart().GetY();

      Rational p1 = s1a * (x - s1.GetOriginalStart().GetX()) +
                    s1.GetOriginalStart().GetY();

      result = -Rational::Compare(p0, p1);

      if (result == 0) {
        result = Rational::Compare(s0a, s1a);
      }
    } else {
      int y = s0.GetStart().GetY()
              + ((s0.GetEnd().GetY() - s0.GetStart().GetY()) / 2);

      Rational s0a =
          Rational(s0.GetOriginalStart().GetX() - s0.GetOriginalEnd().GetX(),
                   s0.GetOriginalStart().GetY() - s0.GetOriginalEnd().GetY());

      Rational s1a =
          Rational(s1.GetOriginalStart().GetX() - s1.GetOriginalEnd().GetX(),
                   s1.GetOriginalStart().GetY() - s1.GetOriginalEnd().GetY());

      Rational p0 = s0a * (y - s0.GetOriginalStart().GetY())
                    + s0.GetOriginalStart().GetX();

      Rational p1 = s1a * (y - s1.GetOriginalStart().GetY())
                    + s1.GetOriginalStart().GetX();

      result = Rational::Compare(p0, p1);

      if (result == 0) {
        result = -Rational::Compare(s0a, s1a);
      }
    }
  }

  return result;
}

/*

1.1 ~RemoveOverlappingSegments~

*/
vector<InternalResultLineSegment>* IntersectionAlgorithm::
RemoveOverlappingSegments(vector<InternalResultLineSegment>& segments)
{
  sort(segments.begin(),
       segments.end(),
       IntersectionAlgorithm::OverlappingSegmentsSortLess);

  vector<InternalResultLineSegment> temp;
  vector<InternalResultLineSegment>* result = NULL;

  for (int loop = 0; loop < 2; loop++) {
    const vector<InternalResultLineSegment>* source;

    if (loop == 0) {
      source = &segments;
      result = &temp;
    } else {
      source = &temp;
      result = new vector<InternalResultLineSegment>();
    }

    bool mergedSegmentSet = false;
    InternalResultLineSegment ms(InternalPoint(0, 0),
                                 InternalPoint(0, 0),
                                 InternalPoint(0, 0),
                                 InternalPoint(0, 0),
                                 InternalAttribute(),
                                 false);

    for (vector<InternalResultLineSegment>::const_iterator s =
        source->begin(); s != source->end(); ++s) {

      if (!mergedSegmentSet) {
        ms = *s;
        mergedSegmentSet = true;
      } else {
        if ((InternalPoint::IsEqual(s->GetStart(), ms.GetStart())
             && InternalPoint::IsEqual(s->GetEnd(), ms.GetEnd()))
            || (InternalPoint::IsEqual(s->GetStart(), ms.GetEnd())
                && InternalPoint::IsEqual(s->GetEnd(), ms.GetStart()))) {
          if (loop == 1 || OverlappingSegmentsSortComparer(ms, *s) == 0) {
            ms = InternalResultLineSegment::MergeSegments(ms, *s);
          } else {
            result->push_back(ms);
            ms = *s;
          }
        } else {
          result->push_back(ms);
          ms = *s;
        }
      }
    }

    if (mergedSegmentSet) {
      result->push_back(ms);
    }
  }

  return result;
}

/*

1.1 ~CreateTransformation~

*/
void IntersectionAlgorithm::CreateTransformation()
{
  const Rectangle<2> boundingBox = _data->GetBoundingBox();

  if (!boundingBox.IsDefined()) {
    _transformation =
        new InternalPointTransformation(0, 0, GetInitialScaleFactor(), 0, 1);

    return;
  }

  double minX = boundingBox.MinD(0);
  double maxX = boundingBox.MaxD(0);
  double minY = boundingBox.MinD(1);
  double maxY = boundingBox.MaxD(1);

  int maxValue = (int)ceil(std::max(std::max(fabs(maxX), fabs(minX)),
                                    std::max(fabs(maxY), fabs(minY))));

  int availableDigits = 14;
  while (maxValue > 0) {
    availableDigits--;
    maxValue /= 10;
  }

  double maxExtent = std::max(maxX - minX, maxY - minY) * 1.1;
  int scaleFactor = GetInitialScaleFactor();

  maxExtent *= scaleFactor;

  int roundingDecimalPlaces = 0;

  for (;;) {
    maxExtent *= 10;
    if (maxExtent >= 1000000000 || scaleFactor >= 200000000) {
      break;
    }
    scaleFactor *= 10;
    availableDigits--;
    roundingDecimalPlaces++;
  }

  if (availableDigits < 0) {
    roundingDecimalPlaces -= (-availableDigits);
  }

  int roundToDecimals, roundingDecimalSteps;
  _data->GetRoundToDecimals(roundToDecimals, roundingDecimalSteps);

  if ((10 % roundingDecimalSteps) != 0) {
    throw new std::logic_error("invalid step size!"
                               " (only 1, 2 or 5 are allowed)");
  }

  if (roundToDecimals >= 0 && roundToDecimals <= roundingDecimalPlaces) {
    roundingDecimalPlaces = roundToDecimals;
  } else {
    roundingDecimalSteps = 1;
  }

  int mod = scaleFactor;
  for (int i = 0; i < -roundingDecimalPlaces; i++) {
    if (mod % 10 != 0) {
      throw new std::logic_error("invalid scale factor!"
                                 " (rounding not possible)");
    }
    mod /= 10;
  }

  long long offsetX = (long long)floor(minX * scaleFactor);
  long long offsetY = (long long)floor(minY * scaleFactor);

  offsetX = offsetX - (offsetX % mod) - (offsetX < 0 ? mod : 0);
  offsetY = offsetY - (offsetY % mod) - (offsetY < 0 ? mod : 0);

  _transformation = new InternalPointTransformation(offsetX,
                                                    offsetY,
                                                    scaleFactor,
                                                    roundingDecimalPlaces,
                                                    roundingDecimalSteps);
}
}
