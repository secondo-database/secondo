/*
----
This file is part of SECONDO.

Copyright (C) 2013, University in Hagen, 
Faculty of Mathematics and Computer Science,
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
//[ue] [\"u]
//[ae] [\"a]
//[oe] [\"o]


[TOC]

1 Helper classes and methods

1.1 Some forward declaration of helper methods

*/

#include "IntersectionSegment.h"

namespace mregionops2 {



IntersectionSegment::IntersectionSegment(const Segment3D& s)

{

    // The startpoint's t-coord is always lower or equal to the
    // endpoint's t-coord.
    // Note: We don't care for x and y!

    if (s.GetStart().GetT() <= s.GetEnd().GetT()) {

        startXYT = s.GetStart();
        endXYT = s.GetEnd();

    } else {

        startXYT = s.GetEnd();
        endXYT = s.GetStart();
    }
}

bool IntersectionSegment::IsLeftOf(const IntersectionSegment* intSeg) const {
    
    // Precondition: 
    // this->GetStartT() is inside the interval 
    // [intSeg->GetStartT(), intSeg->GetEndT()]
    // and this and intSeg don't intersect in their interior.
       
const mpq_class sideOfStart = GetStartWT()->WhichSide(*intSeg->GetStartWT(), 
                                                       *intSeg->GetEndWT());
    // Null anstatt eps vs -eps
    if (sideOfStart > 0)
        return true;
    
    if (sideOfStart < 0)
        return false;
    
const mpq_class sideOfEnd = GetEndWT()->WhichSide(*intSeg->GetStartWT(), 
                                                   *intSeg->GetEndWT());
    
    return sideOfEnd > 0;
}


} // end namespace
