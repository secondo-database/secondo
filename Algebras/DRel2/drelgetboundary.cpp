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
 see OperationSpec

 @note
 Checked - 2020 

 @history
 Version 1.0 - Created - T. Beckmann - 2018
 Version 1.1 - Small improvements - D. Selenyi - 25.07.2020

 @todo
 Nothing

*/

/*
1 Implementation of the secondo operator getboundary

*/
//#define DRELDEBUG

#include <iostream>

#include "include/NestedList.h"
#include "include/ListUtils.h"
#include "include/QueryProcessor.h"
#include "include/StandardTypes.h"

#include "Algebras/Spatial/SpatialAlgebra.h"

#include "Algebras/Collection/CollectionAlgebra.h"

#include "Algebras/DRel/DRel.h"

extern NestedList* nl;
extern QueryProcessor* qp;

namespace drel {

/*
1.1 Type Mapping

*/
    ListExpr getboundaryTM( ListExpr args ) {

        #ifdef DRELDEBUG
        cout << "getboundaryTM" << endl;
        cout << nl->ToString( args ) << endl;
        #endif

        std::string err = "d[f]rel expected ";

        if( !nl->HasLength( args, 1 ) ) {
            return listutils::typeError( err + 
                ": one argument is expected" );
        }

        distributionType distType;
        if( !DRelHelpers::drelCheck( nl->First( args ), distType ) ) {
            return listutils::typeError( err +
                ": first argument is not a d[f]rel" );
        }
        if( distType != range ) {
            return listutils::typeError( err +
                ": d[f]rel is not partitioned by range" );
        }
        
        return nl->Fourth( nl->Third( nl->First( args ) ) );
    }

/*
1.2 Value Mapping

Get the boundary object of a partitioned by range d[f]rel.

*/
    template<class R>
    int getboundaryVMT( Word* args, Word& result, int message,
        Word& local, Supplier s ) {

        #ifdef DRELDEBUG
        cout << "getboundaryVMT" << endl;
        cout << args << endl;
        #endif

        result = qp->ResultStorage( s );
        collection::Collection* boundary =
            static_cast<collection::Collection*>( result.addr );

        R* drel = ( R* )args[ 0 ].addr;

        if( !drel->IsDefined( ) ) {
            boundary->SetDefined( false );
            return 0;
        }

        boundary->CopyFrom( ( ( DistTypeRange* )drel->getDistType( ) )
            ->getBoundary( ) );
        boundary->SetDefined( true );
        boundary->Finish( );

        return 0;
    }

/*
1.3 ValueMapping Array for getboundary

*/
ValueMapping getboundaryVM[ ] = {
    getboundaryVMT<DRel>,
    getboundaryVMT<DFRel>
};

/*
1.4 Selection function for getboundary

*/
int getboundarySelect( ListExpr args ) {

    return DRel::checkType( nl->First( args ) ) ? 0 : 1;
}

/*
1.5 Specification of getboundary

*/
    OperatorSpec getboundary(
        " d[f]rel(rel(tuple(X))) -> vector(x) ",
        " getboundary(_)",
        "Get the boundary object of a partitioned by range d[f]rel. ",
        " query getboundary(drel1)"
    );

/*
1.6 Operator instance of getboundary operator

*/
    Operator getboundaryOp(
        "getboundary",
        getboundary.getStr( ),
        2,
        getboundaryVM,
        getboundarySelect,
        getboundaryTM
    );

} // end of namespace drel
