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

April - November 2008, M. H[oe]ger for bachelor thesis.

[2] Implementation with exakt dataype

Oktober 2014 - Maerz 2015, S. Schroeer for master thesis.

[TOC]

1 Helper classes and methods

1.1 Some forward declaration of helper methods

1 class declarations

2 Defines and Includes

*/
#ifndef SETOPERATOR2_H_
#define SETOPERATOR2_H_

#include "MovingRegion2Algebra.h"
#include "SetOps2.h"


namespace mregionops2 {

/*

1 Class SetOperator2

This class provides the three set operations
with the signature mregion2 [x] mregion2 [->] mregion2.

and two Prädikats
with the signature mregion2 [x] mregion2 [->] bool.

*/

class SetOperator2 {

public:

/*

1.1 Constructor

The constructor takes three pointers to ~MRegion2~ instances,
according to the signature a [x] b [->] res.

*/

    //cstor for mregion2 result
    SetOperator2(MRegion2* const _a,
                 MRegion2* const _b,
                 MRegion2* const _res) :
                 a(_a), b(_b), res(_res),bRes(false){
    };
    //cstor for boolean result
    SetOperator2(MRegion2* const _a,
                 MRegion2* const _b,
                 bool* const _res) :
                 a(_a), b(_b), res(0),bRes(_res){
    };
    
    bool* bRes;
/*

1.1 Operators

1.1.1 Intersection

Performs the operation $a \cap b$ and writes the result to res.

*/

    void Intersection();

/*

1.1.1 Union

Performs the operation $a \cup b$ and writes the result to res.

*/

    void Union();

/*

1.1.1 Minus

Performs the operation $a \setminus b$ and writes the result to res.

*/

    void Minus();

/*

1.1.1 Intersect

Performs the operation $a \setminus b$ and writes the result to res.

*/

    void Intersect();

/*

1.1.1 Inside

Performs the operation $a \setminus b$ and writes the result to res.

*/

    void Inside();

private:

/*

1.1 Private methods

*/

    void Operate(const SetOp op);

/*

1.1 Attributes

*/

    MRegion2* const a;
    MRegion2* const b;
    MRegion2* const res;


};

}

#endif /*SETOPERATOR2_H_*/

