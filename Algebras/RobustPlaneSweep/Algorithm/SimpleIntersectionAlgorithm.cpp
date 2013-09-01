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

[1] Implementation file for the class ~SimpleIntersectionAlgorithm~

[TOC]

1 Overview

This file contains the class ~SimpleIntersectionAlgorithm~.

This file is not required for SECONDO. It is only used inside the test project.

The ~SimpleIntersectionAlgorithm~ is the base class for all test intersection
algorithms. The input and the output are handled inside this class.

1 Includes

*/
#include "SimpleIntersectionAlgorithm.h"

using namespace std;

namespace RobustPlaneSweep
{
/*

1 Class ~SimpleIntersectionAlgorithm~

1.1 ~CreateResult~

*/
void SimpleIntersectionAlgorithm::CreateResult()
{
  vector<InternalLineSegment*>::const_iterator begin = GetInputBegin();
  vector<InternalLineSegment*>::const_iterator end = GetInputEnd();

  vector<InternalResultLineSegment> tempResult;
  for (vector<InternalLineSegment*>::const_iterator i = begin; i != end; ++i) {
    InternalLineSegment *segment = *i;
    segment->BreakupLines(*GetTransformation(), tempResult);
  }

  vector<InternalResultLineSegment>* nonOverlappingLineSegments =
      RemoveOverlappingSegments(tempResult);

  for (vector<InternalResultLineSegment>::const_iterator i =
      nonOverlappingLineSegments->begin();
      i != nonOverlappingLineSegments->end(); ++i) {
    GetData()->OutputHalfSegment(i->GetHalfSegment(GetTransformation()),
                                 i->GetInternalAttribute());
  }

  delete nonOverlappingLineSegments;
}
}
