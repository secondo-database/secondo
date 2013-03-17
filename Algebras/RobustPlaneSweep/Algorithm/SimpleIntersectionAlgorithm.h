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

#pragma once

#include "IntersectionAlgorithm.h"

namespace RobustPlaneSweep
{
  class SimpleIntersectionAlgorithm : public IntersectionAlgorithm 
  {
  private:
    std::vector<InternalLineSegment*>* _internalSegments;

    void CreateInternalLineSegments()
    {
      _internalSegments=new std::vector<InternalLineSegment*>();

      GetData()->InitializeFetch();

      HalfSegment segment;
      while(GetData()->FetchInputHalfSegment(segment))
      {
        if(segment.IsLeftDomPoint()) {
          InternalLineSegment* internalSegment=
            new InternalLineSegment(*GetTransformation(), segment);
          if (!InternalPoint::IsEqual(
            internalSegment->GetLeft(), 
            internalSegment->GetRight())) {
              _internalSegments->push_back(internalSegment);
          } else {
            delete internalSegment;
          }
        }
      }
    }

    void CreateResult();

  protected:

    const std::vector<InternalLineSegment*>::const_iterator 
      GetInputBegin() const
    {
      return _internalSegments->begin();
    }

    const std::vector<InternalLineSegment*>::const_iterator 
      GetInputEnd() const
    {
      return _internalSegments->end();
    }

    size_t GetInputSize() const
    {
      return _internalSegments->size();
    }

    virtual void DetermineIntersectionsInternal() = 0;

  public:
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

    SimpleIntersectionAlgorithm(IntersectionAlgorithmData* data) : 
      IntersectionAlgorithm (data)
    {
      _internalSegments=NULL;
    }

    ~SimpleIntersectionAlgorithm(){
      if(_internalSegments!=NULL) {
        for(std::vector<InternalLineSegment*>::const_iterator 
          i=_internalSegments->begin();
          i!=_internalSegments->end();++i){
            delete (*i);
        }

        delete _internalSegments;
        _internalSegments=NULL;
      }
    }
  };
}