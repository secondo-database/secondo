/*
----
This file is part of SECONDO.

Copyright (C) 2015,
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


//[$][\$]

*/

#ifndef ARRAYORTASKSFUNARG_H
#define ARRAYORTASKSFUNARG_H
#include "Attribute.h"
#include "Algebra.h"
#include "NestedList.h"
#include "QueryProcessor.h"
#include "AlgebraManager.h"
#include "Operator.h"
#include "StandardTypes.h"
#include "Symbols.h"
#include "ListUtils.h"
#include "Algebras/Distributed2/CommandLogger.h"
#include "Algebras/Array/ArrayAlgebra.h"
#include "SocketIO.h"
#include "Task.h"
#include "Algebras/Distributed2/FileRelations.h"
#include "Algebras/Distributed2/fsrel.h"
#include "Stream.h"
#include "Algebras/Distributed2/DArray.h"
#include "Algebras/Distributed2/Distributed2Algebra.h"
#include "Algebras/FText/FTextAlgebra.h"

namespace distributed5
{

extern Operator ARRAYORTASKSFUNARG1Op;
extern Operator ARRAYORTASKSFUNARG2Op;
extern Operator ARRAYORTASKSFUNARG3Op;
extern Operator ARRAYORTASKSFUNARG4Op;
extern Operator ARRAYORTASKSFUNARG5Op;

extern Operator ARRAYORTASKSFUNFSARG2Op;

extern Operator ARRAYORTASKSFUNARGPARTITIONFOp;

} // namespace distributed5

#endif
