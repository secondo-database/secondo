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

extern Operator DRELRELFUNARG1OP;

extern Operator createboundaryOp;
extern Operator getboundaryOp;
extern Operator getBoundaryIndexOp;
extern Operator rect2cellgridOp;
extern Operator file2streamOp;

extern Operator drelfdistributeOp;
extern Operator dreldistributeOp;
extern Operator drelimportOp;

extern Operator drelpartitionOp;
extern Operator drelspatialpartitionOp;

extern Operator compareDistTypeOp;
extern Operator drelcollect_boxOp;
extern Operator drel2darrayOp;

extern Operator countOp;
extern Operator lcountOp;

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
extern Operator exactmatchOp;
extern Operator rangeOp;

extern Operator drelbulkloadrtreeOp;
extern Operator windowintersectsOp;

extern Operator rdupOp;
extern Operator sortOp;
extern Operator sortbyOp;
extern Operator drelgroupbyOp;

extern Operator sortmergejoinOp;
extern Operator itHashJoinOp;

extern Operator inloopjoinOp;
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
    AddOperator( &file2streamOp );

    AddOperator( &drelfdistributeOp );
    AddOperator( &dreldistributeOp );
    AddOperator( &drelimportOp );

    AddOperator( &drelpartitionOp );
    AddOperator( &drelspatialpartitionOp );

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
    AddOperator( &drel2darrayOp );

    AddOperator( &countOp );
    AddOperator( &lcountOp );

    AddOperator( &dsummarizeOp );

    AddOperator( &drelcreatebtreeOp );
    drelcreatebtreeOp.SetUsesArgsInTypeMapping( );
    AddOperator( &exactmatchOp );
    exactmatchOp.SetUsesArgsInTypeMapping( );
    AddOperator( &rangeOp );
    rangeOp.SetUsesArgsInTypeMapping( );

    AddOperator( &drelbulkloadrtreeOp );
    drelbulkloadrtreeOp.SetUsesArgsInTypeMapping( );

    AddOperator( &windowintersectsOp );
    windowintersectsOp.SetUsesArgsInTypeMapping( );

    AddOperator( &rdupOp );
    rdupOp.SetUsesArgsInTypeMapping( );
    AddOperator( &sortOp );
    sortOp.SetUsesArgsInTypeMapping( );
    AddOperator( &sortbyOp );
    sortbyOp.SetUsesArgsInTypeMapping( );
    AddOperator( &drelgroupbyOp );
    drelgroupbyOp.SetUsesArgsInTypeMapping( );

    AddOperator( &sortmergejoinOp );
    sortmergejoinOp.SetUsesArgsInTypeMapping( );
    AddOperator( &itHashJoinOp );
    itHashJoinOp.SetUsesArgsInTypeMapping( );

    AddOperator( &inloopjoinOp );
    inloopjoinOp.SetUsesArgsInTypeMapping( );
}

extern "C"
Algebra*
   InitializeDRelAlgebra( NestedList* nlRef,
                          QueryProcessor* qpRef ) {
   return new DRelAlgebra();
}

} // end of namespace drel