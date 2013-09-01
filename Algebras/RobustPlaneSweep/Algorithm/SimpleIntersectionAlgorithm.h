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

[1] Header File for the class ~SimpleIntersectionAlgorithm~

[TOC]

1 Overview

This header file contains the class ~SimpleIntersectionAlgorithm~.

This file is not required for SECONDO. It is only used inside the test project.

The ~SimpleIntersectionAlgorithm~ is the base class for all test intersection
algorithms. The input and the output are handled inside this class.

1 Includes

*/

#pragma once

#include "IntersectionAlgorithm.h"

namespace RobustPlaneSweep
{
/*

1 Class ~SimpleIntersectionAlgorithm~

*/
class SimpleIntersectionAlgorithm : public IntersectionAlgorithm
{
private:
  std::vector<InternalLineSegment*>* _internalSegments;

/*

1.1 ~CreateInternalLineSegments~

*/
  void CreateInternalLineSegments()
  {
    _internalSegments = new std::vector<InternalLineSegment*>();

    GetData()->InitializeFetch();

    HalfSegment segment;
    Point point;
    bool belongsToSecondGeometry;
    while (GetData()->FetchInput(segment, point, belongsToSecondGeometry)) {
      if (segment.IsLeftDomPoint()) {
        bool isRegion;
        if (belongsToSecondGeometry) {
          isRegion = SecondGeometryIsRegion();
        } else {
          isRegion = FirstGeometryIsRegion();
        }

        InternalLineSegment* internalSegment =
            new InternalLineSegment(*GetTransformation(),
                                    segment,
                                    belongsToSecondGeometry,
                                    isRegion);

        if (!InternalPoint::IsEqual(internalSegment->GetLeft(),
                                    internalSegment->GetRight())) {
          _internalSegments->push_back(internalSegment);
        } else {
          delete internalSegment;
        }
      }
    }
  }

/*

1.1 ~CreateResult~

*/
  void CreateResult();

protected:
/*

1.1 ~GetInputBegin~

*/
  const std::vector<InternalLineSegment*>::const_iterator GetInputBegin() const
  {
    return _internalSegments->begin();
  }

/*

1.1 ~GetInputEnd~

*/
  const std::vector<InternalLineSegment*>::const_iterator GetInputEnd() const
  {
    return _internalSegments->end();
  }

/*

1.1 ~GetInputSize~

*/
  size_t GetInputSize() const
  {
    return _internalSegments->size();
  }

/*

1.1 ~DetermineIntersectionsInternal~

*/
  virtual void DetermineIntersectionsInternal() = 0;

public:
/*

1.1 ~DetermineIntersections~

*/
  void DetermineIntersections()
  {
    if (GetTransformation() == NULL) {
      CreateTransformation();
    }

    CreateInternalLineSegments();
    DetermineIntersectionsInternal();
    CreateResult();
    GetData()->OutputFinished();
  }

/*

1.1 Constructors

*/
  explicit SimpleIntersectionAlgorithm(IntersectionAlgorithmData* data) :
      IntersectionAlgorithm(data)
  {
    _internalSegments = NULL;
  }

  ~SimpleIntersectionAlgorithm()
  {
    if (_internalSegments != NULL) {
      for (std::vector<InternalLineSegment*>::const_iterator i =
          _internalSegments->begin(); i != _internalSegments->end(); ++i) {
        delete (*i);
      }

      delete _internalSegments;
      _internalSegments = NULL;
    }
  }
};
}
