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

*/


#include <algorithm>

#include "IntersectionAlgorithm.h"

using namespace std;

namespace RobustPlaneSweep
{
  bool IntersectionAlgorithm::OverlappingSegmentsSortComparer(
    InternalResultLineSegment x, 
    InternalResultLineSegment y)
  {
    bool xReverseStartEnd = InternalPoint::Compare(x.GetStart(),x.GetEnd())<0;
    bool yReverseStartEnd = InternalPoint::Compare(y.GetStart(),y.GetEnd())<0;

    int result = InternalPoint::Compare(
      (!xReverseStartEnd ? x.GetStart() : x.GetEnd()),
      (!yReverseStartEnd ? y.GetStart() : y.GetEnd()));

    if (result == 0) {
      result = InternalPoint::Compare(
        (!xReverseStartEnd ? x.GetEnd() : x.GetStart()),
        (!yReverseStartEnd ? y.GetEnd() : y.GetStart()));
    }

    return result<0;
  }

  vector<InternalResultLineSegment>* 
    IntersectionAlgorithm::RemoveOverlappingSegments(
    vector<InternalResultLineSegment>& segments)
  {
    sort(
      segments.begin(),
      segments.end(),
      IntersectionAlgorithm::OverlappingSegmentsSortComparer);

    vector<InternalResultLineSegment>* result=
      new vector<InternalResultLineSegment>();

    bool mergedSegmentSet=false;
    InternalResultLineSegment mergedSegment(
      AttrType(0),
      InternalPoint(0,0),
      InternalPoint(0,0));

    for(vector<InternalResultLineSegment>::const_iterator 
      segment=segments.begin();
      segment!=segments.end();++segment) {
        if (!mergedSegmentSet) {
          mergedSegment = *segment;
          mergedSegmentSet=true;
        } else {
          if ((InternalPoint::IsEqual(
            segment->GetStart(), 
            mergedSegment.GetStart()) &&
            InternalPoint::IsEqual(
            segment->GetEnd(), 
            mergedSegment.GetEnd())) ||
            (InternalPoint::IsEqual(
            segment->GetStart(), 
            mergedSegment.GetEnd()) && 
            InternalPoint::IsEqual(
            segment->GetEnd(), 
            mergedSegment.GetStart()))) {

              AttrType mergedAttributes = 
                _data->MergeAttributes(
                mergedSegment.GetAttr(),
                segment->GetAttr());

              mergedSegment = InternalResultLineSegment(
                mergedAttributes, 
                mergedSegment.GetStart(), 
                mergedSegment.GetEnd());

          } else {
            result->push_back(mergedSegment);
            mergedSegment = *segment;
          }
        }
    }

    if (mergedSegmentSet) {
      result->push_back(mergedSegment);
    }

    return result;
  }

  void IntersectionAlgorithm::CreateTransformation()
  {
    const Rectangle<2> boundingBox=_data->GetBoundingBox();

    if (!boundingBox.IsDefined()) {
      _transformation = new InternalPointTransformation(
        0,
        0, 
        GetInitialScaleFactor(),
        0);
      return;
    }

    double minX = boundingBox.MinD(0);
    double maxX = boundingBox.MaxD(0);
    double minY = boundingBox.MinD(1);
    double maxY = boundingBox.MaxD(1);

    int maxValue = (int)ceil(std::max(
      std::max(fabs(maxX), fabs(minX)), 
      std::max(fabs(maxY), fabs(minY))));
    
    int availableDigits = 14;
    while (maxValue > 0) {
      availableDigits--;
      maxValue /= 10;
    }

    double maxExtent = std::max(maxX - minX, maxY - minY)*1.1;
    int scaleFactor = GetInitialScaleFactor();

    maxExtent *= scaleFactor;

    int roundingDecimalPlaces = 0;

    for (; ; ) {
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

    int roundToDecimals=_data->GetRoundToDecimals();
    if (roundToDecimals >= 0 && roundToDecimals<roundingDecimalPlaces) {
      roundingDecimalPlaces = roundToDecimals;
    }

    int mod = scaleFactor;
    for (int i = 0; i < -roundingDecimalPlaces; i++) {
      if (mod % 10 != 0) {
        throw new std::logic_error(
          "invalid scale factor! (rounding not possible)");
      }
      mod /= 10;
    }

    long long offsetX = (long long)floor(minX * scaleFactor);
    long long offsetY = (long long)floor(minY * scaleFactor);

    offsetX = offsetX - (offsetX % mod) - (offsetX < 0 ? mod : 0);
    offsetY = offsetY - (offsetY % mod) - (offsetY < 0 ? mod : 0);

    _transformation = new InternalPointTransformation(
      offsetX, 
      offsetY, 
      scaleFactor, 
      roundingDecimalPlaces);
  }
}
