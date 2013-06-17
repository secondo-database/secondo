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

#include "../Helper/LineIntersection.h"
#include "NaiveIntersectionAlgorithm.h"

using namespace std;

namespace RobustPlaneSweep
{
void NaiveIntersectionAlgorithm::DetermineIntersectionsInternal()
{
  vector<InternalLineSegment*>::const_iterator begin = GetInputBegin();
  vector<InternalLineSegment*>::const_iterator end = GetInputEnd();

  for (vector<InternalLineSegment*>::const_iterator i = begin; i != end; ++i) {
    InternalLineSegment* si = *i;
    for (vector<InternalLineSegment*>::const_iterator j =
        i + 1; j != end; ++j) {
      InternalLineSegment* sj = *j;

      InternalIntersectionPoint i0, i1;
      int c = LineIntersection::GetIntersections(si->GetLeft(),
                                                 si->GetRight(),
                                                 sj->GetLeft(),
                                                 sj->GetRight(),
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
  }
}
}
