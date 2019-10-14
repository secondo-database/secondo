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


1 Implementation of the secondo operators drelfilter, drelproject, 
drelprojectextend, drellsortby, drellgroupby, drellsort, drellrdup, 
drelrename, drelhead and drelextend

This operators have the same value mapping witch calls the dmap operator of 
the Distributed2Algebra.

*/
//#define DRELDEBUG

#include "NestedList.h"
#include "ListUtils.h"
#include "QueryProcessor.h"
#include "StandardTypes.h"
#include "SecParser.h"

#include "Stream.h"
#include "Algebras/FText/FTextAlgebra.h"
#include "Algebras/Temporal/TemporalAlgebra.h"
#include "Algebras/Spatial/SpatialAlgebra.h"
#include "Algebras/Relation-C++/OperatorFilter.h"
#include "Algebras/Relation-C++/OperatorProject.h"
#include "Algebras/Distributed2/CommandLogger.h"

#include "DRelHelpers.h"
#include "DRel.h"
#include "PartitionerS.hpp"
#include "BoundaryCalculator.hpp"
#include "drelport.h"

extern NestedList* nl;
extern QueryProcessor* qp;

namespace distributed2 {

    extern Distributed2Algebra* algInstance;

    int collect2VM(Word* args, Word& result, int message,
             Word& local, Supplier s );
};

using namespace distributed2;

namespace drel {

/*
1.1 Type Mapping ~drelspatialpartitionTM~

Expect a d[f]rel and an attribute name to repartition the given d[f]rel.

*/
    ListExpr drelspatialpartitionTM( ListExpr args ) {

        #ifdef DRELDEBUG
        cout << "drelspatialpartitionTM" << endl;
        cout << "args" << endl;
        cout << nl->ToString( args ) << endl;
        #endif

        std::string err = "d[f]rel(X) x attr x [cellgrid2d|cellgrid3d|d[f]rel]"
                          " expected";

        if( !nl->HasLength( args, 2 )
         && !nl->HasLength( args, 3 ) ) {
            return listutils::typeError( err +
                ": two or three arguments are expected1" );
        }

        distributionType type;
        ListExpr darrayType;
        int attr, key;
        if( !DRelHelpers::drelCheck( nl->First( args ), 
                darrayType, type, attr, key ) ) {
            return listutils::typeError(
                err + ": first argument is not a d[f]rel" );
        }

        ListExpr relType = nl->Second( darrayType );

        if( !( nl->AtomType( nl->Second( args ) ) == SymbolType ) ) {
            return listutils::typeError( err + ": second parameter is not "
                "an attribute" );
        }

        std::string attrName = nl->SymbolValue( nl->Second( args ) );
        ListExpr attrList = nl->Second( nl->Second( relType ) );

        ListExpr attrType;
        int pos = listutils::findAttribute( attrList, attrName, attrType );
        if( pos == 0 ) {
            return listutils::typeError(
                err + ": attr name " + attrName + " not found" );
        }

        distributionType targetType;
        ListExpr cellgridtype;
        if( DistTypeSpatial<temporalalgebra::CellGrid2D>::allowedAttrType2d( 
                attrType ) ) {

            cellgridtype = listutils::
                basicSymbol<temporalalgebra::CellGrid2D>( );
             targetType = spatial2d;
        }
        else if( DistTypeSpatial<temporalalgebra::CellGrid<3>>::
            allowedAttrType2d( attrType ) ) {

            cellgridtype = listutils::
                basicSymbol<temporalalgebra::CellGrid<3>>( );
             targetType = spatial3d;
        }
        else {
            return listutils::typeError(
                err + ": attr type is not of kind spatial2d or spatial3d" );
        }

        int resultKey = rand( );
        bool secondDRel = false;
        bool griddef = false;
        distributionType drel2Type;
        ListExpr drel2DarrayType;
        int drel2Attr, drel2Key;


        // third argument a drel
        if( nl->HasLength( args, 3 )
            && DRelHelpers::drelCheck( nl->Third( args ), 
            drel2DarrayType, drel2Type, drel2Attr, drel2Key ) ) {

            if( targetType != drel2Type ) {

                return listutils::typeError(
                    err + ": third argument is a d[f]rel, but the "
                    "d[f]rel has not the right partitioning" );
            }
            secondDRel = true;
            resultKey = drel2Key;
        }

        // third argument a cellgrid2d but choosen attribute is 2d
        else {
            griddef = nl->HasLength( args, 3 ) && ( 
                ( targetType == spatial2d
                && temporalalgebra::CellGrid2D::checkType( 
                                    nl->Third( args ) ) )
                || ( targetType == spatial3d
                && temporalalgebra::CellGrid<3>::checkType( 
                                    nl->Third( args ) ) )
                );
        }

        if( nl->HasLength( args, 3 ) && !secondDRel && !griddef ) {
            return listutils::typeError(
                err + ": third argument is wrong" );
        }

        ListExpr appendList = nl->TwoElemList(
            nl->StringAtom( attrName ),
            nl->IntAtom( pos - 1 ) );

        ListExpr resultType = nl->ThreeElemList(
                listutils::basicSymbol<DFRel>( ),
                nl->TwoElemList(
                    listutils::basicSymbol<Relation>( ),
                    nl->TwoElemList(
                        listutils::basicSymbol<Tuple>( ),
                        DRelHelpers::addPartitionAttributes( attrList ) ) ),
                nl->FourElemList(
                    nl->IntAtom( targetType ),
                    nl->IntAtom( pos - 1 ),
                    nl->IntAtom( resultKey ),
                    cellgridtype ) );

        return nl->ThreeElemList( 
            nl->SymbolAtom( Symbols::APPEND( ) ),
            appendList,
            resultType );
    }

/*
1.2 Value Mapping ~drelspatialpartitionVMT~

Uses a d[f]rel and an attribute to repartition the d[f]rel by the given 
attribute.

*/
    template<class R, class T, int x, class G>
    int drelspatialpartitionVMT( Word* args, Word& result, int message,
        Word& local, Supplier s ) {

        #ifdef DRELDEBUG
        cout << "drelspatialpartitionVMT" << endl;
        #endif
        
        ListExpr drelType = qp->GetType( qp->GetSon( s, 0 ) );
        R* drel = ( R* )args[ 0 ].addr;
        std::string attrName;
        if( x == 0 ) {
            attrName = ( ( CcString* )args[ 2 ].addr )->GetValue( );
        }
        else {
            attrName = ( ( CcString* )args[ 3 ].addr )->GetValue( );
        }

        // start partitioner
        PartitionerS<R, T, G>* parti =
            new PartitionerS<R, T, G>(
                attrName, nl->Fourth( nl->Third( qp->GetType( s ) ) ),
                drel, drelType, getDRelPort() );

        if( x == 0 ) {
            // compute the grid
            if( !parti->computeGrid( ) ) {
                result = qp->ResultStorage( s );
                ( ( DFRel* )result.addr )->makeUndefined( );
                return 0;
            }
            parti->getGrid( )->Copy( );
        }
        else if( x == 1 ) {
            G* grid = ( G* )args[ 2 ].addr;
            parti->setgrid( grid );
        }
        else if( x == 2 ) {
            DistTypeBasic* inDType;
            if( DRel::checkType( qp->GetType( qp->GetSon( s, 2 ) ) ) ) {
                DRel* drelIn = ( DRel* )args[ 2 ].addr;
                inDType = drelIn->getDistType( );
            }
            else {
                DFRel* drelIn = ( DFRel* )args[ 2 ].addr;
                inDType = drelIn->getDistType( );
            }

            G* grid = ( ( DistTypeSpatial<G>* )inDType )->getGrid( );
            parti->setgrid( grid );
        }

        if( !parti->sharegrid( ) ) {
            result = qp->ResultStorage( s );
            ( ( DFRel* )result.addr )->makeUndefined( );
            return 0;
        }

        if( !parti->repartition2DFMatrix( ) ) {
            result = qp->ResultStorage( s );
            ( ( DFRel* )result.addr )->makeUndefined( );
            return 0;
        }

        DFMatrix* matrix = parti->getDFMatrix( );

        if( !matrix || !matrix->IsDefined( ) ) {
            cout << "repartition failed!!" << endl;
            result = qp->ResultStorage( s );
            ( ( DFRel* )result.addr )->makeUndefined( );
            return 0;
        }

        CcString* name = new CcString( "" );
        CcInt* port = new CcInt( true, getDRelPort() );
        ArgVector collect2Args = {
            matrix,
            name,
            port };

        collect2VM( collect2Args, result, message, local, s );
        DFRel* resultDFRel = ( DFRel* )result.addr;


        distributionType dType;
        int attr, key;
        DFRel::checkDistType( 
            nl->Third( qp->GetType( s ) ), dType, attr, key );

        DistTypeSpatial<G>* resultDType = new DistTypeSpatial<G>( 
            dType, attr, key, parti->getGrid( ) );

        resultDFRel->setDistType( resultDType );
        
        delete name;
        delete port;
        delete parti;

        return 0;
    }

    template<class R, class T, int x>
    int drelspatialpartitionVMT( Word* args, Word& result, int message,
        Word& local, Supplier s ) {

        #ifdef DRELDEBUG
        cout << "drelspatialpartitionVMT" << endl;
        #endif

        if( x < 0 || x > 3 ) {
            result = qp->ResultStorage( s );
            DFRel* resultDFRel = ( DFRel* )result.addr;
            resultDFRel->makeUndefined( );
            return 0;
        }

        distributionType dType;
        DRelHelpers::drelCheck( qp->GetType( s ), dType );

        // start partitioner
        if( dType == spatial2d ) {
            drelspatialpartitionVMT<R, T, x, temporalalgebra::CellGrid2D>
                ( args, result, message, local, s );
        }
        else {
            drelspatialpartitionVMT<R, T, x, temporalalgebra::CellGrid<3>>
                ( args, result, message, local, s );
        }

        return 0;
    }

/*
1.3 ValueMapping Array for drelspatialpartition

*/
    ValueMapping drelspatialpartitionVM[ ] = {
        drelspatialpartitionVMT<DRel, DArray, 0>,
        drelspatialpartitionVMT<DFRel, DFArray, 0>,
        drelspatialpartitionVMT<DRel, DArray, 1>,
        drelspatialpartitionVMT<DFRel, DFArray, 1>,
        drelspatialpartitionVMT<DRel, DArray, 2>,
        drelspatialpartitionVMT<DFRel, DFArray, 2>,
        drelspatialpartitionVMT<DRel, DArray, 3>,
        drelspatialpartitionVMT<DFRel, DFArray, 3>
    };

/*
1.4 Selection function for drelspatialpartition

*/
    int drelspatialpartitionSelect( ListExpr args ) {

        bool thirdArg = nl->HasLength( args, 3 );
        int x = 0;

        if( thirdArg ) {
            x = Vector::checkType( nl->Third( args ) ) ? 1 :
                DRel::checkType( nl->Third( args ) ) ? 2 : 3;
        }

        return DRel::checkType( nl->First( args ) ) ? 2 * x : 2 * x + 1;
    }

/*
1.5 Specification of drelspatialpartition

*/
    OperatorSpec drelspatialpartitionSpec(
        " d[f]rel(X) x attr x [cellgrid2d|cellgrid3d|d[f]rel] "
        "-> dfrel(X) ",
        " _ drelspatialpartition[_,_]",
        "Repartition of a d[f]rel with partition by spatial2d or spatial3d. "
        "The attribute is the key attribute to repartition the d[f]rel. "
        "Optional a cellgrid2d or cellgrid3d can be used. A existing by "
        "spatial2d or spatial3d d[f]rel can also be used as third argument",
        " query drel1 drelspatialpartition[GeoData]"
    );

/*
1.6 Operator instance of drelspatialpartition operator

*/
    Operator drelspatialpartitionOp(
        "drelspatialpartition",
        drelspatialpartitionSpec.getStr( ),
        8,
        drelspatialpartitionVM,
        drelspatialpartitionSelect,
        drelspatialpartitionTM
    );

} // end of namespace drel
