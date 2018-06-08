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
#include "DRelAlgebra.h"
#include "NestedList.h"
#include "QueryProcessor.h"
#include "StandardTypes.h"

namespace drel {

extern TypeConstructor DRelTC;
extern TypeConstructor DFRelTC;
extern TypeConstructor BoundaryTC;

extern Operator createboundaryOp;
extern Operator getBoundaryIndexOp;
extern Operator rect2cellgridOp;

extern Operator drelfdistributeOp;
extern Operator dreldistributeOp;

extern Operator compareDistTypeOp;
extern Operator convert2darrayOp;

/*
1 Implementation of the Algebra DRel

*/
DRelAlgebra::DRelAlgebra() {
    AddTypeConstructor( &DRelTC );
    DRelTC.AssociateKind( Kind::SIMPLE() );
    AddTypeConstructor( &DFRelTC );
    DFRelTC.AssociateKind( Kind::SIMPLE( ) );
    AddTypeConstructor( &BoundaryTC );
    BoundaryTC.AssociateKind( Kind::SIMPLE( ) );
    
    AddOperator( &createboundaryOp );
    createboundaryOp.SetUsesArgsInTypeMapping( );
    AddOperator( &getBoundaryIndexOp );
    AddOperator( &rect2cellgridOp );

    AddOperator( &drelfdistributeOp );
    drelfdistributeOp.SetUsesArgsInTypeMapping( );
    AddOperator( &dreldistributeOp );
    dreldistributeOp.SetUsesArgsInTypeMapping( );

    AddOperator( &compareDistTypeOp );
    AddOperator( &convert2darrayOp );
}

extern "C"
Algebra*
   InitializeDRelAlgebra( NestedList* nlRef,
                          QueryProcessor* qpRef ) {
   return new DRelAlgebra();
}

} // end of namespace drel