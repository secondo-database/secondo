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

[1] Implementation

Oktober 2014 - Maerz 2015, S. Schroeer for master thesis.

[TOC]

1 Helper classes and methods

1.1 Some forward declaration of helper methods

*/

#ifndef INTSEGCONTAINER_H_
#define INTSEGCONTAINER_H_

#include "IntersectionSegment.h"

#include <set>

namespace mregionops2 {

class IntersectionSegment;

/*
1 Struct IntSegCompare

This struct implements the ~IntersectionSegment~ order,
used in ~IntSegContainer~.

*/
struct IntSegCompare {

    bool operator()(const IntersectionSegment* const& s1,
                    const IntersectionSegment* const& s2) const;
};


/*
1 Class IntSegContainer

This class is used by the class ~PFace~ and provides essentially
  * an ordered set of ~IntersectionSegments~
  * an ordered multiset of critical timelevels
  * the ability to perform a plane-sweep in t-direction from bottom to top.

*/

class IntSegContainer {

public:

/*
1.1 Constructor

*/

    inline IntSegContainer()

    {

    }

/*
1.1 Destructor

*/

    ~IntSegContainer();

/*
1.1 Operators

1.1.1 AddIntSeg

Adds seg to the set of ~IntersectionSegments~.

*/

    void AddIntSeg(IntersectionSegment* seg);
    void FinalizeIntSegs();
    void FillIntSegsTable(unsigned int count);
    void InvertAreaDirections();
    void Print() const;
    void PrintActive() const;

private:

/*
1.1 Private methods

1.1 Attributes

1.1.1 intSegs, intSegIter

A ~std::set~ to store the ~IntersectionSegments~ using the order
provided by ~IntSegCompare~ and a suitable iterator.

*/
    set<IntersectionSegment*, IntSegCompare> intSegs;


};

} // end of namespace mregionops2

#endif /*INTSEGCONTAINER_H_*/



