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

[1] Header file of the TemporalExt Algebra

[TOC]

1 Overview

The type system of the TemporalExt Algebra can be seen below.


2 Defines, includes, and constants

*/
#ifndef _TEMPORALEXT_ALGEBRA_H_
#define _TEMPORALEXT_ALGEBRA_H_

#include <iostream>
#include "NestedList.h"
#include "QueryProcessor.h"
#include "Algebra.h"
#include "StandardAttribute.h"
#include "StandardTypes.h"
#include "SpatialAlgebra.h"
#include "NestedList.h"
#include "DBArray.h"
#include "RectangleAlgebra.h"
#include "DateTime.h"

extern NestedList* nl;
extern QueryProcessor* qp;

/*
3 C++ Classes (Definition)

*/
typedef ConstTemporalUnit<CcString> UString;
typedef Mapping< UString, CcString > MString;
typedef Range<CcBool> RBool;
typedef Range<CcString> RString;

class MPointExt : public MPoint
{
public:

    MPointExt() : MPoint() {}

    void MDirection( MReal* result) const;
};

#endif // _TEMPORALEXT_ALGEBRA_H_



