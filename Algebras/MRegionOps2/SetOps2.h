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

[1] Headerfile 

Oktober 2014 - Maerz 2015, S. Schroer for master thesis.


[TOC]

1 Helper classes and methods

1.1 Some forward declaration of helper methods

1 class declarations

2 Defines and Includes

*/

#ifndef SETOPS2_H_
#define SETOPS2_H_

namespace mregionops2 {


enum SetOp {

    INTERSECTION,
    UNION,
    MINUS,
    INSIDE,
    INTERSECT
};

enum Direction {

    POSITIVE,
    NEGATIVE
};

enum AreaDirection {

    LEFT,
    RIGHT,
    BOTH,
    NONE
};

enum CycleStatus {

    PARTOFRESULT,
    NOTPARTOFRESULT,
    HASINTSEGS,
    NOTYETKNOWN
};
}

#endif
