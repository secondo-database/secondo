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

[1] Implementation file for the class ~SimpleSweepIntersectionAlgorithm~

[TOC]

1 Overview

This file contains all structs and classes required for the 
class ~SimpleSweepIntersectionAlgorithm~.

This file is not required for SECONDO. It is only used inside the test project.

This class uses a simple sweep algorithm. There are start- and end-sweep-events.
At a start event, the segment is inserted into a hash table and 
at an end event, the segment is removed. Before a segment is inserted, 
every segment inside the hash table is tested against the inserting segment.

1 Includes

*/
#include <vector>
#include <algorithm>

#include "../Helper/LineIntersection.h"
#include "SimpleSweepIntersectionAlgorithm.h"

using namespace std;

namespace RobustPlaneSweep
{
/*

1 Class ~SimpleSweepEvent~

*/
class SimpleSweepEvent
{
private:
  InternalLineSegment* _segment;
  bool _isStart;

public:
/*

1.1 Constructor

*/
  SimpleSweepEvent(InternalLineSegment* segment, bool isStart) :
      _segment(segment),
      _isStart(isStart)
  {
  }

/*

1.1 ~GetIsStart~

*/
  bool GetIsStart() const
  {
    return _isStart;
  }

/*

1.1 ~GetSegment~

*/
  InternalLineSegment* GetSegment() const
  {
    return _segment;
  }

/*

1.1 ~operator\textless~

*/
  bool operator<(const SimpleSweepEvent &y) const
  {
    int xx, yy;

    if (_isStart) {
      xx = _segment->GetLeft().GetX();
    } else {
      xx = _segment->GetRight().GetX();
    }

    if (y._isStart) {
      yy = y._segment->GetLeft().GetX();
    } else {
      yy = y._segment->GetRight().GetX();
    }

    if (xx < yy) {
      return true;
    } else if (xx > yy) {
      return false;
    }

    return (_isStart ? 0 : 1) < (y._isStart ? 0 : 1);
  }
};

/*

1 Class ~SimpleSweepIntersectionAlgorithm~

1.1 ~DetermineIntersectionsInternal~

*/
void SimpleSweepIntersectionAlgorithm::DetermineIntersectionsInternal()
{
  std::vector<SimpleSweepEvent> sweepEvents;

  sweepEvents.reserve(GetInputSize() * 2);

  std::vector<InternalLineSegment*>::const_iterator begin = GetInputBegin();
  std::vector<InternalLineSegment*>::const_iterator end = GetInputEnd();

  for (std::vector<InternalLineSegment*>::const_iterator i = begin;
      i != end;
      ++i) {
    InternalLineSegment* segment = *i;
    sweepEvents.push_back(SimpleSweepEvent(segment, true));
    sweepEvents.push_back(SimpleSweepEvent(segment, false));
  }

  std::sort(sweepEvents.begin(), sweepEvents.end());

  std::unordered_set<InternalLineSegment*> currentSegments;

  for (vector<SimpleSweepEvent>::const_iterator sweepEvent =
      sweepEvents.begin(); sweepEvent != sweepEvents.end(); ++sweepEvent) {
    InternalLineSegment* si = sweepEvent->GetSegment();

    if (sweepEvent->GetIsStart()) {
      for (unordered_set<InternalLineSegment*>::const_iterator sj_iter =
          currentSegments.begin(); sj_iter != currentSegments.end();
          ++sj_iter) {

        InternalLineSegment* sj = *sj_iter;
        InternalIntersectionPoint i0, i1;

        int c = LineIntersection::GetIntersections(si->GetLeft(),
                                                   si->GetRight(),
                                                   sj->GetLeft(),
                                                   sj->GetRight(),
                                                   false,
                                                   false,
                                                   i0,
                                                   i1);

        if (c == 0) {
        } else if (c == 1) {
          si->AddIntersection(i0);
          sj->AddIntersection(i0);
        } else if (c == 2) {
          si->AddIntersection(i0);
          sj->AddIntersection(i0);
          si->AddIntersection(i1);
          sj->AddIntersection(i1);
        }
      }

      currentSegments.insert(si);
    } else {
      currentSegments.erase(si);
    }
  }

  if (!currentSegments.empty()) {
    throw new std::logic_error("not correctly processed!");
  }
}
}
