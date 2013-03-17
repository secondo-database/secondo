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

#include <vector>
#include <unordered_map>
#include <utility>

#ifdef RPS_TEST
#include "SpatialAlgebraStubs.h"
#else
#include "SpatialAlgebra.h"
#endif

namespace RobustPlaneSweep
{
  class LineSegmentComparer
  {
  private:
    std::vector<HalfSegment> _segments0;
    std::vector<HalfSegment> _segments1;

    std::unordered_map<
      double,
      std::pair<std::vector<size_t>*,std::vector<size_t>*>>
      _possiblePairs;

    void FindPossiblePairs();

  public:
    template <class ForwardIterator0, class ForwardIterator1> 
    LineSegmentComparer(
      ForwardIterator0 start0,
      ForwardIterator0 end0,
      ForwardIterator1 start1,
      ForwardIterator1 end1)
    {
      for(ForwardIterator0 i=start0;i!=end0;++i) {
        _segments0.push_back (*i);
      }
      for(ForwardIterator1 i=start1;i!=end1;++i) {
        _segments1.push_back (*i);
      }

    }

    ~LineSegmentComparer() 
    {
      for(std::unordered_map<
        double,
        std::pair<std::vector<size_t>*,std::vector<size_t>*>>::const_iterator 
        i=_possiblePairs.begin();
        i!=_possiblePairs.end();++i) {
        delete i->second.first;
        delete i->second.second;
      }

    }

    bool IsEqual();
  };
}
