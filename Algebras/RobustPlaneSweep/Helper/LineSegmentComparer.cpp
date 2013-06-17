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

#include <stdexcept>
#include "LineSegmentComparer.h"

using namespace std;

namespace RobustPlaneSweep
{
void LineSegmentComparer::FindPossiblePairs()
{
  for (int list = 0; list < 2; ++list) {
    vector<HalfSegment>::const_iterator begin =
        (list == 0 ? _segments0.begin() : _segments1.begin());

    vector<HalfSegment>::const_iterator end =
        (list == 0 ? _segments0.end() : _segments1.end());

    size_t index = 0;
    for (vector<HalfSegment>::const_iterator
    i = begin; i != end; ++i, ++index) {
      for (int se = 0; se < 2; ++se) {
        Point p = (se == 0 ? i->GetLeftPoint() : i->GetRightPoint());
        double key = p.GetX() + p.GetY();

        pair<vector<size_t>*, vector<size_t>*> foundPair;
        unordered_map<double, pair<vector<size_t>*, vector<size_t>*>>::
        const_iterator findIterator = _possiblePairs.find(key);

        if (findIterator != _possiblePairs.end()) {
          foundPair = findIterator->second;
        } else {
          foundPair =
              pair<vector<size_t>*, vector<size_t>*>(new vector<size_t>,
                                                     new vector<size_t>);
          _possiblePairs[key] = foundPair;
        }

        if (list == 0) {
          foundPair.first->push_back(index);
        } else {
          foundPair.second->push_back(index);
        }
      }
    }
  }
}

bool LineSegmentComparer::IsEqual()
{
  FindPossiblePairs();

  size_t totalCount0 = _segments0.size();
  size_t totalCount1 = _segments1.size();

  vector<bool> found0;
  vector<bool> found1;

  for (size_t i = 0; i < totalCount0; ++i) {
    found0.push_back(false);
  }

  for (size_t i = 0; i < totalCount1; ++i) {
    found1.push_back(false);
  }

  for (unordered_map<double, pair<vector<size_t>*, vector<size_t>*>>::
  const_iterator possiblePair = _possiblePairs.begin();
      possiblePair != _possiblePairs.end(); ++possiblePair) {
    for (vector<size_t>::const_iterator
    i0 = possiblePair->second.first->begin();
        i0 != possiblePair->second.first->end(); ++i0) {
      if (found0[*i0]) {
        continue;
      }

      for (vector<size_t>::const_iterator i1 =
          possiblePair->second.second->begin();
          i1 != possiblePair->second.second->end(); ++i1) {
        if (found0[*i0]) {
          break;
        }

        if (found1[*i1]) {
          continue;
        }

        if (_segments0[*i0].GetLeftPoint() == _segments1[*i1].GetLeftPoint()
            && _segments0[*i0].GetRightPoint()
               == _segments1[*i1].GetRightPoint()
            && _segments0[*i0].IsLeftDomPoint()
               == _segments1[*i1].IsLeftDomPoint())
               {
          if (_segments0[*i0].attr.insideAbove ==
              _segments1[*i1].attr.insideAbove) {
            if (found0[*i0] || found1[*i1]) {
              throw new logic_error("loop break/continue missing or wrong!");
            }
            found0[*i0] = true;
            found1[*i1] = true;
          }
        }
      }
    }
  }

  for (size_t i = 0; i < totalCount0; ++i) {
    if (!found0[i]) {
      return false;
    }
  }

  for (size_t i = 0; i < totalCount1; ++i) {
    if (!found1[i]) {
      return false;
    }
  }

  return true;
}
}
