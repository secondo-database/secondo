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



1 Implementation of the secondo operator getBoundaryIndex

*/
#include <iostream>

#include "NestedList.h"
#include "ListUtils.h"
#include "QueryProcessor.h"
#include "StandardTypes.h"
#include "Algebras/Stream/Stream.h"

#include "Algebras/FText/FTextAlgebra.h"

#include "Algebras/Distributed2/CommandLogger.h"
#include "Algebras/Distributed2/Distributed2Algebra.h"
#include "Algebras/Spatial/SpatialAlgebra.h"

#include "Boundary.h"
#include "DRelHelpers.h"

extern NestedList* nl;
extern QueryProcessor* qp;

namespace drel {

    /*
    1.1 Type Mapping

    Except a boundary and an attribute with the same type as the bounary.

    */
    ListExpr getBoundaryIndexTM( ListExpr args ) {

        std::string err = "boundary, attr expected";

        if( ! nl->HasLength( args, 2 ) ) {
            return listutils::typeError( err + 
                ": two arguments are expected" );
        }

        if( !Boundary::checkType( nl->First( args ), nl->Second( args ) ) ) {
            return listutils::typeError( err +
                ": first argument is not a boundary or the attribute " +
                "type does not fit to the boundary type" );
        }

        return listutils::basicSymbol<CcInt>( );
    }

    /*
    1.2 Value Mapping

    Get for an attribute the indexnumber within a boundary type.

    */
    int getBoundaryIndexVM( Word* args, Word& result, int message,
        Word& local, Supplier s ) {

        Boundary* boundary = ( Boundary* )args[ 0 ].addr;
        Attribute* attr = ( Attribute* )args[ 1 ].addr;

        int index = boundary->getBoundaryIndexNumber( attr );
        result = qp->ResultStorage( s );
        CcInt* res = ( CcInt* ) result.addr;
        if( index == -1 ) {
            res->SetDefined( false );
            return 0;
        }
        res->Set( true, index );

        return 0;
    }

    /*
    1.3 Specification of getboundaryindex

    */
    OperatorSpec getBoundaryIndex(
        " boundary x attr "
        "-> int ",
        " _ getBoundaryIndex[ _ ]",
        "Returns for an attribute the the indexnumber in within a "
        "boundary.",
        "query getBoundaryIndex[ PLZ ]"
    );

    /*
    1.4 Operator instance of getboundaryindex operator

    */
    Operator getBoundaryIndexOp(
        "getboundaryindex",
        getBoundaryIndex.getStr( ),
        getBoundaryIndexVM,
        Operator::SimpleSelect,
        getBoundaryIndexTM
    );

} // end of namespace drel