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

[1] Implementation of the MRegionOps2Algebra

April - November 2008, M. H[oe]ger for bachelor thesis.

[2] Implementation with exakt dataype, 

April - November 2014, S. Schroer for master thesis.

[TOC]

1 Introduction

2 Defines and Includes

*/

#include "Point3DExt.h"

namespace mregionops2 {

/*
1 Class Point3DExt

*/

bool Point3DExt::operator <(const Point3DExt& p) const {

    if (GetX() < p.GetX())
        return true;
    if (GetX() > p.GetX())
        return false;
    if (GetY() < p.GetY())
        return true;
    if (GetY() > p.GetY())
        return false;
    if (GetT() < p.GetT())
        return true;
    if (GetT() > p.GetT())
        return false;

    //cout << "sourceFlag < p.sourceFlag" << endl;
    return sourceFlag < p.sourceFlag;
}


/***********************************

 end of namespace mregionops2

***********************************/
};
