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

[1] Headerfile of the Segment classes

April - November 2008, M. H[oe]ger for bachelor thesis.

[2] Implementation with exakt dataype

Oktober 2014 - Maerz 2015, S. Schroer for master thesis.

[TOC]

1 Introduction

2 Defines and Includes

*/
#ifndef SEGMENT3D_H_
#define SEGMENT3D_H_

#include "Point3D.h"

namespace mregionops2 {

/*
3 Class Segment3D

This class provides an oriented segment in the euclidian space.
It's start- and endpoint is represented by a Point3D each.

*/
class Segment3D {

public:
    
/*

3.1 Constructors

*/   

    inline Segment3D() {
        
    }

    inline Segment3D(const Point3D& _start, const Point3D& _end) :
        start(_start), end(_end) {

    }
    
/*

3.2 Getter methods.

*/

    inline const Point3D& GetStart() const {
        return start;
    }
    inline const Point3D& GetEnd() const {
        return end;
    }
    
/*

3.3 Operators and Predicates
        
3.3.1 IsOrthogonalToTAxis

Returns ~true~, if this is parallel to the xy-plane.

*/   

    inline const bool IsOrthogonalToTAxis() const {
          return (start.GetT() == end.GetT());
    }
    

private:

    Point3D start, end;

};

} // end of namespace mregionops2

#endif /*SEGMENT3DH_*/
