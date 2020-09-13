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

 @author
 T. Beckmann

 @description
 see OperatorSpec

 @note
 Checked - 2020 

 @history
 Version 1.0 - Created - T. Beckmann - 2018

 @todo
 Nothing

*/

//#define DRELDEBUG

#include "Algebras/Temporal/TemporalAlgebra.h"
#include "DRel.h"
#include "NestedList.h"
#include "QueryProcessor.h"
#include "StandardTypes.h"
#include "ConstructorTemplates.h"

extern NestedList* nl;
extern QueryProcessor* qp;

using namespace distributed2;

namespace drel {

/*
1.24 ~DFRelTC~

TypeConstructor of the type DFRel

*/
    TypeConstructor DFRelTC(
        DFRel::BasicType( ),
        DFRel::Property,
        DFRel::Out,
        DFRel::In,
        0, 0,
        DFRel::Create,
        DFRel::Delete,
        DFRel::Open,
        DFRel::Save,
        DFRel::Close,
        DFRel::Clone,
        DFRel::Cast,
        DFRel::SizeOf,
        DFRel::TypeCheck
    );

/*
1.25 ~DRelTC~

TypeConstructor of the type DRel

*/
    TypeConstructor DRelTC(
        DRel::BasicType( ),
        DRel::Property,
        DRel::Out,
        DRel::In,
        0, 0,
        DRel::Create,
        DRel::Delete,
        DRel::Open,
        DRel::Save,
        DRel::Close,
        DRel::Clone,
        DRel::Cast,
        DRel::SizeOf,
        DRel::TypeCheck
    );

} // end of namespace drel
