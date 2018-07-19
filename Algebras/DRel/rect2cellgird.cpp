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



1 Implementation of the secondo operator rect2cellgrid

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

        Rectangle<2>* rec = ( Rectangle<2>* )args[ 0 ].addr;
        int size = ( ( CcInt* )args[ 1 ].addr )->GetValue( );

        double cellSize1 = ( rec->MaxD( 0 ) - rec->MinD( 0 ) ) / size;
        double cellSize2 = ( rec->MaxD( 1 ) - rec->MinD( 1 ) ) / size;

        result = qp->ResultStorage( s );
        temporalalgebra::CellGrid2D* grid = 
            ( temporalalgebra::CellGrid2D* )result.addr;

        grid->set( rec->MinD( 0 ), rec->MinD( 1 ), cellSize1, cellSize2, size );

        #ifdef DRELDEBUG
        cout << "rect2cellgridVMT" << endl;
        cout << "grid output" << endl;
        cout << "grid defined?" << endl;
        cout << grid->IsDefined( ) << endl;
        grid->Print( cout );
        cout << endl;
        #endif

        return 0;
    }

    template<class Rect, class Grid>
    int rect2cellgridVMT( Word* args, Word& result, int message,
        Word& local, Supplier s ) {

        Rect* rec = ( Rect* )args[ 0 ].addr;
        int size = ( ( CcInt* )args[ 1 ].addr )->GetValue( );

        unsigned dim = rec->GetDim( );
        double cellSize[ dim ];
        double originPoint[ dim ];
        int cellNumbers[ dim - 1 ];

        for( size_t i = 0 ; i < dim - 1 ; i++ ) {
            cellNumbers[ i ] = size;
            cout << "cellNumbers[ i ]" << endl;
            cout << cellNumbers[ i ] << endl;
        }
        
        for( size_t i = 0 ; i < dim ; i++ ) {
            originPoint[ i ] = rec->MinD( i );
            cout << "originPoint[ i ]" << endl;
            cout << originPoint[ i ] << endl;
        }
        
        for( size_t i = 0 ; i < dim ; i++ ) {
            cellSize[ i ] = ( rec->MaxD( i ) - rec->MinD( i ) ) / size;
            cout << "cellSize[ i ]" << endl;
            cout << cellSize[ i ] << endl;
        }

        result = qp->ResultStorage( s );
        Grid* grid = ( Grid* )result.addr;

        grid->set( originPoint, cellSize, cellNumbers );
        grid->SetDefined( true );

        #ifdef DRELDEBUG
        cout << "rect2cellgridVMT" << endl;
        cout << "grid output" << endl;
        cout << "grid defined?" << endl;
        cout << grid->IsDefined( ) << endl;
        grid->Print( cout );
        cout << endl;
        #endif

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
        rect2cellgridVMT<Rectangle<3>, temporalalgebra::CellGrid<3>>
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