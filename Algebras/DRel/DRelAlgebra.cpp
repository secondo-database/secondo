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

extern Operator DRELFUNARG1OP;
extern Operator DRELFUNARG2OP;

extern Operator DRELFUNARGTTT1OP;
extern Operator DRELFUNARGTTT2OP;

extern Operator DRELRELFUNARG1OP;

extern Operator createboundaryOp;
extern Operator getboundaryOp;
extern Operator getBoundaryIndexOp;
extern Operator rect2cellgridOp;

extern Operator drelfdistributeOp;
extern Operator dreldistributeOp;

extern Operator drelpartitionOp;

extern Operator compareDistTypeOp;
extern Operator drelcollect_boxOp;
extern Operator convertOp;

extern Operator dsummarizeOp;

extern Operator drelfilterOp;
extern Operator projectOp;
extern Operator drelextendOp;
extern Operator drelprojectextendOp;
extern Operator headOp;
extern Operator renameOp;

extern Operator lrdupOp;
extern Operator lsortOp;
extern Operator lsortbyOp;
extern Operator drellgroupbyOp;

extern Operator drelcreatebtreeOp;
extern Operator drelexactmatchOp;
extern Operator drelrangeOp;

extern Operator drelbulkloadrtreeOp;
extern Operator drelwindowintersectsOp;

extern Operator rdupOp;
extern Operator sortOp;
extern Operator sortbyOp;
extern Operator drelgroupbyOp;

extern Operator drelsortmergejoinOp;
extern Operator drelitHashJoinOp;
/*
1 Implementation of the Algebra DRel

*/
DRelAlgebra::DRelAlgebra() {
    AddTypeConstructor( &DRelTC );
    DRelTC.AssociateKind( Kind::SIMPLE() );
    AddTypeConstructor( &DFRelTC );
    DFRelTC.AssociateKind( Kind::SIMPLE( ) );
    
    AddOperator( &DRELFUNARG1OP );
    AddOperator( &DRELFUNARG2OP );

    AddOperator( &DRELRELFUNARG1OP );

    AddOperator( &createboundaryOp );
    createboundaryOp.SetUsesArgsInTypeMapping( );
    AddOperator( &getboundaryOp );
    AddOperator( &getBoundaryIndexOp );
    AddOperator( &rect2cellgridOp );

    AddOperator( &drelfdistributeOp );
    AddOperator( &dreldistributeOp );

    AddOperator( &drelpartitionOp );

    AddOperator( &drelfilterOp );
    drelfilterOp.SetUsesArgsInTypeMapping( );
    AddOperator( &projectOp );
    projectOp.SetUsesArgsInTypeMapping( );
    AddOperator( &drelextendOp );
    drelextendOp.SetUsesArgsInTypeMapping( );
    AddOperator( &drelprojectextendOp );
    drelprojectextendOp.SetUsesArgsInTypeMapping( );
    AddOperator( &headOp );
    headOp.SetUsesArgsInTypeMapping( );
    AddOperator( &renameOp );
    renameOp.SetUsesArgsInTypeMapping( );

    AddOperator( &lrdupOp );
    lrdupOp.SetUsesArgsInTypeMapping( );
    AddOperator( &lsortOp );
    lsortOp.SetUsesArgsInTypeMapping( );
    AddOperator( &lsortbyOp );
    lsortbyOp.SetUsesArgsInTypeMapping( );
    AddOperator( &drellgroupbyOp );
    drellgroupbyOp.SetUsesArgsInTypeMapping( );

    AddOperator( &compareDistTypeOp );
    AddOperator( &drelcollect_boxOp );
    AddOperator( &convertOp );

    AddOperator( &dsummarizeOp );

    AddOperator( &drelcreatebtreeOp );
    drelcreatebtreeOp.SetUsesArgsInTypeMapping( );
    AddOperator( &drelexactmatchOp );
    drelexactmatchOp.SetUsesArgsInTypeMapping( );
    AddOperator( &drelrangeOp );
    drelrangeOp.SetUsesArgsInTypeMapping( );

    AddOperator( &drelbulkloadrtreeOp );
    drelbulkloadrtreeOp.SetUsesArgsInTypeMapping( );

    AddOperator( &drelwindowintersectsOp );
    drelwindowintersectsOp.SetUsesArgsInTypeMapping( );

    AddOperator( &rdupOp );
    rdupOp.SetUsesArgsInTypeMapping( );
    AddOperator( &sortOp );
    sortOp.SetUsesArgsInTypeMapping( );
    AddOperator( &sortbyOp );
    sortbyOp.SetUsesArgsInTypeMapping( );
    AddOperator( &drelgroupbyOp );
    drelgroupbyOp.SetUsesArgsInTypeMapping( );

    AddOperator( &drelsortmergejoinOp );
    drelsortmergejoinOp.SetUsesArgsInTypeMapping( );
    AddOperator( &drelitHashJoinOp );
    drelitHashJoinOp.SetUsesArgsInTypeMapping( );
}

extern "C"
Algebra*
   InitializeDRelAlgebra( NestedList* nlRef,
                          QueryProcessor* qpRef ) {
   return new DRelAlgebra();
}

} // end of namespace drel