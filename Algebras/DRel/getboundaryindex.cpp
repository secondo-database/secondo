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
//#define DRELDEBUG

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
#include "Algebras/Collection/CollectionAlgebra.h"

#include "DRelHelpers.h"

extern NestedList* nl;
extern QueryProcessor* qp;

namespace drel {

/*
1.1 Type Mapping

Except a boundary and an attribute with the same type as the bounary.

*/
    ListExpr getBoundaryIndexTM( ListExpr args ) {

        #ifdef DRELDEBUG
        cout << "getBoundaryIndexTM" << endl;
        cout << nl->ToString( args ) << endl;
        #endif

        std::string err = "vector(t), attr expected";

        if( ! nl->HasLength( args, 2 ) ) {
            return listutils::typeError( err + 
                ": two arguments are expected" );
        }

        ListExpr arg1 = nl->First( args );
        ListExpr arg2 = nl->Second( args );

        if( ! nl->HasLength( arg1, 2 ) ) {
            return listutils::typeError( err + 
                ": first arguments is not a vector" );
        }

        ListExpr errorInfo;
        if( !collection::Collection::KindCheck( arg1, errorInfo )
         && !Vector::checkType( nl->First( arg1 ) ) ) {
            return listutils::typeError( err + 
                ": first arguments is not a vector" );
        }

        if( !nl->IsAtom( arg2 )
         || nl->AtomType( arg2 ) != SymbolType ) {
            return listutils::typeError( err +
                ": second arguments is not an attribute" );
        }

        if( !nl->Equal( nl->Second( arg1 ), arg2 ) ) {
            return listutils::typeError( err +
                ": the attribute type does not fit to the vector type" );
        }

        return listutils::basicSymbol<CcInt>( );
    }

/*
1.2 Value Mapping

Get for an attribute the indexnumber within a boundary type.

*/
    int getBoundaryIndexVM( Word* args, Word& result, int message,
        Word& local, Supplier s ) {

        collection::Collection* vector = 
            static_cast<collection::Collection*>( args[ 0 ].addr );
        Attribute* attr = static_cast<Attribute*>( args[ 1 ].addr );

        result = qp->ResultStorage( s );
        CcInt* res = ( CcInt* )result.addr;

        if( !vector->IsDefined( ) ) {
            res->SetDefined( false );
            return 0;
        }

        int index = DRelHelpers::getIndex( vector, attr );

        res->Set( true, index );

        return 0;
    }

/*
1.3 Specification of getboundaryindex

*/
    OperatorSpec getBoundaryIndex(
        " vector x attr "
        "-> int ",
        " getboundaryindex(_,_)",
        "Returns for an attribute the indexnumber within a "
        "boundary. The boundary is a sorted vector.",
        "query getboundaryindex(boundary, PLZ)"
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