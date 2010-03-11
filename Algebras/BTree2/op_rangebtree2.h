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

//paragraph [1] Title: [{\Large \bf \begin{center}] [\end{center}}]
//paragraph [10] Footnote: [{\footnote{] [}}]
//[TOC] [\tableofcontents]

[1] Header of the range operators for the BTree2-Algebra 

[TOC]

1 Defines and Includes

*/

#ifndef _BTREE2_OP_RANGEBTREE_H
#define _BTREE2_OP_RANGEBTREE_H

#include "Operator.h"
#include "NestedList.h"
#include "AlgebraTypes.h"

#include "BTree2.h"

namespace BTree2Algebra {
namespace Operators {

/*
2 Class with description operators

*/
class rangebtree2 {

public:

static Operator exactmatch2;
static Operator exactmatchS;
static Operator exactmatch;
static Operator range2;
static Operator rangeS;
static Operator range;
static Operator leftrange2;
static Operator leftrangeS;
static Operator leftrange;
static Operator rightrange2;
static Operator rightrangeS;
static Operator rightrange;

};

} // end namespace Operators
} // end namespace BTree2Algebra

#endif
