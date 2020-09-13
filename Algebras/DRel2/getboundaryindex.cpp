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
 Version 1.1 - Small improvements and check if its outside of the boundary - D. Selenyi - 25.07.2020

 @todo
 Nothing

*/

/*

1 Implementation of the secondo operator getBoundaryIndex

*/

//#define DRELDEBUG

#include <iostream>

#include "include/NestedList.h"
#include "include/ListUtils.h"
#include "include/QueryProcessor.h"
#include "include/StandardTypes.h"
#include "include/Stream.h"

#include "Algebras/FText/FTextAlgebra.h"

#include "Algebras/Distributed2/CommandLogger.h"
#include "Algebras/Distributed2/Distributed2Algebra.h"

#include "Algebras/Spatial/SpatialAlgebra.h"

#include "Algebras/Collection/CollectionAlgebra.h"

#include "Algebras/DRel/DRelHelpers.h"

extern NestedList* nl;
extern QueryProcessor* qp;

namespace drel {

using std::runtime_error;

/*
1.1 Type Mapping

Expect a boundary and an attribute with the same type as the boundary.

*/
    ListExpr getBoundaryIndexTM( ListExpr args ) {

        #ifdef DRELDEBUG
        cout << "getBoundaryIndexTM" << endl;
        cout << nl->ToString( args ) << endl;
        #endif

        std::string err = "vector(t) x attr expected";

        if( ! nl->HasLength( args, 2 ) ) {
            return listutils::typeError( err + 
                ": two arguments are expected" );
        }

        if( ! nl->HasLength(nl->First( args ), 2 ) ) {
            return listutils::typeError( err + 
                ": first argument has to have two arguments" );
        }

        ListExpr errorInfo;
        if( !collection::Collection::KindCheck( nl->First( args ), errorInfo )
         && !Vector::checkType( nl->First( nl->First( args ) ) ) ) {
            return listutils::typeError( err + 
                ": first argument is not a vector" );
        }

        if( !nl->IsAtom( nl->Second( args ) )
         || nl->AtomType( nl->Second( args ) ) != SymbolType ) {
            return listutils::typeError( err +
                ": second arguments is not an attribute" );
        }

        if( !nl->Equal( nl->Second( nl->First( args ) ), nl->Second( args ) ) ){
            return listutils::typeError( err +
                ": the attribute type does not fit to the vector type" );
        }

        return listutils::basicSymbol<CcInt>( );
    }

/*
1.2 Value Mapping

Get for an attribute the indexnumber within a boundary type.
If its outside of boundary, then there will be an error message.

*/
    int getBoundaryIndexVM( Word* args, Word& result, int message,
        Word& local, Supplier s ) {

        #ifdef DRELDEBUG
        cout << "getBoundaryIndexVM" << endl;
        cout << args << endl;
        #endif

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

        if (index == -1)
        {
            int boundary = vector->GetNoComponents( )-1;
            throw runtime_error{"Your search value is outside of the"
                  " existing boundarys. Max boundary is: "+ 
                  std::to_string(boundary) };        
            res->Set(true, 0);
        }
        return 0;
    }

/*
1.3 Specification of getboundaryindex

*/
    OperatorSpec getBoundaryIndex(
        " vector(X) x X , X in DATA -> int",
        " getboundaryindex(_,_)",
        "Returns the indexnumber within a "
        "boundary vector for a value. The boundary is a sorted vector.",
        "query getboundaryindex(boundary, 60000)"
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
