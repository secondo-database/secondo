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

1 Defines and includes

*/


#include <string.h>
#include <stdio.h>
#include <fstream>

#include "Algebra.h"
#include "NestedList.h"
#include "QueryProcessor.h"
#include "StandardTypes.h"
#include "Algebras/FText/FTextAlgebra.h"
#include "Attribute.h"
#include "FileSystem.h"
#include "Tools/Flob/Flob.h"
#include "Tools/Flob/DbArray.h"
#include "Base64.h"
#include "Symbols.h"
#include "ListUtils.h"


#include "GenericTC.h"
#include "Stream.h"
#include "NList.h"
#include "FVector.h"

extern NestedList* nl;
extern QueryProcessor *qp;

using namespace std;

namespace fvector{

/*

1 Type Constructor ~fvector~


create a TypeConstructor instance

*/

GenTC<FVector> FVectorTC;


/*
6 Creating the Algebra

*/

class FVectorAlgebra : public Algebra {
public:
    FVectorAlgebra() : Algebra() {
//----------
        AddTypeConstructor( &FVectorTC);
        FVectorTC.AssociateKind( Kind::DATA() );
//----------

  }
  ~FVectorAlgebra() {};
};

} // end namespace fvector 

/*
7 Initialization

Each algebra module needs an initialization function. The algebra manager
has a reference to this function if this algebra is included in the list
of required algebras, thus forcing the linker to include this module.

The algebra manager invokes this function to get a reference to the instance
of the algebra class and to provide references to the global nested list
container (used to store constructor, type, operator and object information)
and to the query processor.

The function has a C interface to make it possible to load the algebra
dynamically at runtime.

*/

extern "C"
Algebra*
InitializeFVectorAlgebra( NestedList* nlRef, QueryProcessor* qpRef ) {
    nl = nlRef;
    qp = qpRef;
    return (new fvector::FVectorAlgebra);
}


