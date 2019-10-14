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

To simplify inclusion of header files most of them are organized in this file.
The order of them is (at least partly) important for some headers depend on
others.

*/

#ifndef ALGEBRAS_NESTEDRELATION2_INCLUDE_H_
#define ALGEBRAS_NESTEDRELATION2_INCLUDE_H_

/*
The NestedRelation2 algebra depends on SECONDOs standard data types, the
Relation algebra and SECONDOs implementation of streams. It also features its
own type of exception.

*/
#include "StandardTypes.h"

#include "ARel.h"
#include "NRel.h"

#include "Algebras/Relation-C++/RelationAlgebra.h"
#include "Stream.h"

#include "Nr2aException.h"
#include "NestedRelation2Algebra.h"

/*
One more class provides helper functions used in the algebra internally.

*/
#include "Nr2aLocalInfo.h"
#include "Nr2aHelper.h"
#include "LinearProgressEstimator.h"
#include "Nr2aLocalInfo.h"

/*
Each operator in the algebra is implemented in its own header and source file,
both named after the corresponding operator.

*/
#include "Count.h"
#include "Feed.h"
#include "Aconsume.h"
#include "BlockingProgressEstimator.h"
#include "Consume.h"
#include "Unnest.h"
#include "Nest.h"
#include "Extract.h"
#include "Rename.h"
#include "GetTuples.h"
#include "DblpImport.h"
#include "GenRel.h"
#include "TypeOf.h"


#endif /* ALGEBRAS_NESTEDRELATION2_INCLUDE_H_*/
