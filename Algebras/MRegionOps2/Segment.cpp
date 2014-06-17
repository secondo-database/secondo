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

[1] Implementation of the Segment classes

April - November 2008, M. H[oe]ger for bachelor thesis.

[TOC]

1 Introduction

This file contains some implementations of the classes Segment2D and Segment3D.

2 Defines and Includes

*/

#include "Segment.h"

namespace mregionops2 {

/*
3 Overloaded output operators
    
*/ 

ostream& operator <<(ostream& o, const Segment2D& s) {

    o << s.GetStart() << " -> " << s.GetEnd();

    return o;
}

ostream& operator <<(ostream& o, const Segment3D& s) {

    o << s.GetStart() << " -> " << s.GetEnd();

    return o;
}

} // end of namespace mregionops2
