/*
----
This file is part of SECONDO.

Copyright (C) 2008, University in Hagen, Department of Computer Science,
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
//[x] [$\times $]
//[->] [$\rightarrow $]

[1] Implementation 

April - November 2008, M. H[oe]ger for bachelor thesis.

[2] Implementation with exakt dataype, 

Oktober 2014 - Maerz 2015, S. Schroeer for master thesis.

[TOC]

1 Introduction

2 Defines and Includes

*/

#include "PointExtSet.h"

namespace mregionops2 {

/*
1 Class Point3DExt

*/

/***********************************

6.1 PointExtSet::GetIntersectionSegment

***********************************/

bool PointExtSet::GetIntersectionSegment(Segment3D& result) const {

    if (s.size() != 4)
        return false;

    set<Point3DExt>::iterator it = s.begin();

    Point3DExt p1 = *it;
    it++;
    Point3DExt p2 = *it;

    if (p1.sourceFlag == p2.sourceFlag)
        return false;

    it++;
    Point3DExt p3 = *it;

    if (p2 == p3) {
        // The length of the intersection segment is zero.
        return false;
    }

    result = Segment3D(p2, p3);
    return true;
}

void PointExtSet::Print() const {
    
    set<Point3DExt>::iterator iter;
    
    for (iter = s.begin(); iter != s.end(); ++iter) {
        Point3DExt p = *iter;
        cout << p << endl;
    }
}


/***********************************

 end of namespace mregionops2

***********************************/
};
