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

[1] Header File for the class ~SimpleSweepIntersectionAlgorithm~

[TOC]

1 Overview

This header file contains all structs and classes required for the
class ~SimpleSweepIntersectionAlgorithm~.

This file is not required for SECONDO. It is only used inside the test project.

This class uses a simple sweep algorithm. There are start- and end-sweep-events.
At a start event, the segment is inserted into a hash table and 
at an end event, the segment is removed. Before a segment is inserted, 
every segment inside the hash table is tested against the inserting segment.

1 Includes

*/

#pragma once

#include "SimpleIntersectionAlgorithm.h"

namespace RobustPlaneSweep
{
/*

1 Class ~SimpleSweepIntersectionAlgorithm~

*/
class SimpleSweepIntersectionAlgorithm : public SimpleIntersectionAlgorithm
{
protected:
/*

1.1 ~GetInitialScaleFactor~

*/
  int GetInitialScaleFactor() const
  {
    return 1;
  }

/*

1.1 ~DetermineIntersectionsInternal~

*/
  void DetermineIntersectionsInternal();

public:
/*

1.1 Constructor

*/
  explicit SimpleSweepIntersectionAlgorithm(IntersectionAlgorithmData* data) :
      SimpleIntersectionAlgorithm(data)
  {
  }
};
}
