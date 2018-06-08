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



1 Implementation of the secondo operator createboundary

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

    Get a rectangle of demension 2 or 3 and an integer.

    */
    ListExpr rect2cellgridTM( ListExpr args ) {

        std::string err = "[rect / rect3] x int expected";

        if( !nl->HasLength( args, 2 ) ) {
            return listutils::typeError( err +
                ": three arguments are expected" );
        }

        if( !CcInt::checkType( nl->Second( args ) ) ) {
            return listutils::typeError( err +
                ": second argument has to be an integer" );
        }

        if( Rectangle<2>::checkType( nl->First( args ) ) ) {
            return listutils::basicSymbol<temporalalgebra::CellGrid2D>( );
        }
        else if( Rectangle<3>::checkType( nl->First( args ) ) ) {
            return listutils::basicSymbol<temporalalgebra::CellGrid<3>>( );
        }
        
        return listutils::typeError( err +
            ": first argument is not a rect or rect3" );
    }

    /*
    1.2 Value Mapping

    Creates a cellgrid2d or a cellgrid3d.

    */
    int rect2cellgrid2dVM( Word* args, Word& result, int message,
        Word& local, Supplier s ) {

        Rectangle<2>* rect = ( Rectangle<2>* )args[ 0 ].addr;
        CcInt* size = ( CcInt* )args[ 1 ].addr;

        result = qp->ResultStorage( s );
        temporalalgebra::CellGrid2D* res = 
            ( temporalalgebra::CellGrid2D* )result.addr;

        DRelHelpers::setGrid( res, rect, size->GetIntval( ) );

        return 0;
    }
    
    int rect2cellgrid3dVM( Word* args, Word& result, int message,
        Word& local, Supplier s ) {

        cout << "rect2cellgrid3dVM" << endl;

        // not implemented jet

        //Rectangle<3>* rect = ( Rectangle<3>* )args[ 0 ].addr;
        //CcInt* size = ( CcInt* )args[ 1 ].addr;

        //result = qp->ResultStorage( s );
        //temporalalgebra::CellGrid<3>* res = 
        //    ( temporalalgebra::CellGrid<3>* )result.addr;

        //DRelHelpers::setGrid( res, rect, size->GetIntval( ) );

        return 0;
    }

    /*
    1.3 Specification of rect2cellgrid

    */
    OperatorSpec rect2cellgrid(
        " [ rect / rect2 ] x int "
        "-> cellgrid2d, cellgrid3d ",
        " _ rect2cellgrid[ _ ]",
        "Creates a 2d grid or a 3d grid for a given rectangle.",
        " query [const rect value(5 9 50 52)] rect2cellgrid[20]"
    );

    /*
    1.4 ValueMapping Array of drfdistribute

    */
    ValueMapping rect2cellgridVM[ ] = {
        rect2cellgrid2dVM,
        rect2cellgrid3dVM
    };

    /*
    1.7 Selection function

    */
    int rect2cellgridSelect( ListExpr args ) {
        return Rectangle<2>::checkType( nl->First( args ) ) ? 0 : 1;
    }

    /*
    1.4 Operator instance of rect2cellgrid operator

    */
    Operator rect2cellgridOp(
        "rect2cellgrid",
        rect2cellgrid.getStr( ),
        2,
        rect2cellgridVM,
        rect2cellgridSelect,
        rect2cellgridTM
    );

} // end of namespace drel