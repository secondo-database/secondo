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



1 Implementation of the secondo operator compareDistType

*/
#include "NestedList.h"
#include "ListUtils.h"
#include "QueryProcessor.h"
#include "StandardTypes.h"

#include "Algebras/Stream/Stream.h"
#include "Algebras/Rectangle/RectangleAlgebra.h"

extern NestedList* nl;
extern QueryProcessor* qp;

namespace drel {

/*
1.1 Type Mapping

Expect a DRel or DFRel and another DRel or DFRel.

*/
    ListExpr drelcollect_boxTM( ListExpr args ) {

        std::string err = "stream(spatial) x bool expected ";

        if( !nl->HasLength( args, 2 ) ) {
            return listutils::typeError( err + "  (wrong number of args)" );
        }
        if( !Stream<Attribute>::checkType( nl->First( args ) ) ) {
            return listutils::typeError( err +
                "  ( first arg is not an attribute stream)" );
        }
        if( !CcBool::checkType( nl->Second( args ) ) ) {
            return listutils::typeError( err + " (second arg is not a bool)" );
        }
        ListExpr attr = nl->Second( nl->First( args ) );
        if( listutils::isKind( attr, Kind::SPATIAL2D( ) ) ) {
            return listutils::basicSymbol<Rectangle<2> >( );
        }
        if( listutils::isKind( attr, Kind::SPATIAL3D( ) ) ) {
            return listutils::basicSymbol<Rectangle<3> >( );
        }
        
        return listutils::typeError( err + 
            " (attribute not in kind SPATIAL2D or SPATIAL3D)" );
    }

/*
1.2 Value Mapping

Compares the disttypes of two drels. Return true, if the drels have the 
same disttype.

*/
    template<int dim>
    int drelcollect_boxVMT( Word* args, Word& result, int message,
        Word& local, Supplier s ) {

        Stream<StandardSpatialAttribute<dim> > stream( args[ 0 ] );
        CcBool* ignoreUndefined = ( CcBool* )args[ 1 ].addr;
        result = qp->ResultStorage( s );
        Rectangle<dim>* res = ( Rectangle<dim>* ) result.addr;
        res->SetDefined( false );
        if( !ignoreUndefined->IsDefined( ) ) {
            return 0;
        }
        stream.open( );
        bool first = true;
        bool useundef = !ignoreUndefined->GetValue( );
        StandardSpatialAttribute<dim>* a = stream.request( );
        while( a ) {
            if( a->IsDefined( ) ) {
                Rectangle<dim> box = a->BoundingBox( );
                if( box.IsDefined( ) ) {
                    if( first ) {
                        res->SetDefined( true );
                        ( *res ) = box;
                        first = false;
                    } else {
                        res->Extend( box );
                    }
                }
            } else {
                if( !useundef ) {
                    res->SetDefined( false );
                    a->DeleteIfAllowed( );
                    a = 0;
                    stream.close( );
                    return 0;
                }
            }
            a->DeleteIfAllowed( );
            a = stream.request( );
        }
        stream.close( );
        return 0;
    }

/*
1.3 ValueMapping Array of drfdistribute

*/
    ValueMapping drelcollect_boxVM[ ] = {
        drelcollect_boxVMT<2>,
        drelcollect_boxVMT<3>
    };

/*
1.4 Selection function

*/
    int drelcollect_boxSelect( ListExpr args ) {

        ListExpr attr = nl->Second( nl->First( args ) );
        return listutils::isKind( attr, Kind::SPATIAL2D( ) ) ? 0 : 1;
    }

/*
1.5 Specification of comparedisttype

*/
    OperatorSpec drelcollect_boxSpec(
        "stream<SPATIAL> x bool -> rectangle",
        " _ drelcollect_box[_]",
        "Computes the bounding box from a stream of spatial attributes"
        "If the second parameter is set to be true, undefined elements "
        " within the stream are ignored. Otherwise an undefined element"
        " will lead to an undefined result.",
        "query strassen feed projecttransformstream[GeoData] collect_box[TRUE] "
    );

/*
1.6 Operator instance of comparedisttype operator

*/
    Operator drelcollect_boxOp(
        "drelcollect_box",
        drelcollect_boxSpec.getStr( ),
        2,
        drelcollect_boxVM,
        drelcollect_boxSelect,
        drelcollect_boxTM
    );

} // end of namespace drel