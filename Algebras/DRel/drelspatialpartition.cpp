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

#include "Algebras/Stream/Stream.h"
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

extern NestedList* nl;
extern QueryProcessor* qp;

namespace distributed2 {

    extern Distributed2Algebra* algInstance;

    int collect2VM(Word* args, Word& result, int message,
             Word& local, Supplier s );
};

using namespace distributed2;

namespace drel {

    template<class T>
    bool computeCellGrid( ListExpr drelType, T* drel, bool filterOriginal, 
        string attr, Word& result ) {

        ListExpr filter;
        if( filterOriginal ) {

            filter = nl->ThreeElemList(
                nl->SymbolAtom( "filter" ),
                nl->TwoElemList(
                    nl->SymbolAtom( "feed" ),
                    nl->SymbolAtom( "dmapelem_1" ) ),
                nl->ThreeElemList(
                    nl->SymbolAtom( "fun" ),
                    nl->TwoElemList(
                        nl->SymbolAtom( "streamelem_2" ),
                        nl->SymbolAtom( "STREAMELEM" ) ),
                    nl->ThreeElemList(
                        nl->SymbolAtom( "attr" ),
                        nl->SymbolAtom( "streamelem_2" ),
                        nl->SymbolAtom( "Original" ) ) ) );
        }
        else {
            filter = nl->TwoElemList(
                nl->SymbolAtom( "feed" ),
                nl->SymbolAtom( "dmapelem_1" ) );
        }

        ListExpr query = nl->TwoElemList(
            nl->SymbolAtom( "getValue" ),
            nl->FourElemList(
                nl->SymbolAtom( "dmap" ),
                nl->TwoElemList(
                    nl->SymbolAtom( "drelconvert" ),
                    nl->TwoElemList(
                        drelType,
                        nl->TwoElemList(
                            nl->SymbolAtom( "ptr" ),
                            listutils::getPtrList( drel ) ) ) ),
                nl->StringAtom( "" ),
                nl->ThreeElemList(
                nl->SymbolAtom( "fun" ),
                nl->TwoElemList(
                    nl->SymbolAtom( "dmapelem_1" ),
                    nl->SymbolAtom( "ARRAYFUNARG1" ) ),
                nl->ThreeElemList(
                    nl->SymbolAtom( "rect2cellgrid" ),
                    nl->ThreeElemList(
                        nl->SymbolAtom( "drelcollect_box" ),
                        nl->TwoElemList(
                            nl->SymbolAtom( "transformstream" ),
                            nl->FourElemList(
                                nl->SymbolAtom( "projectextend" ),
                                nl->ThreeElemList( 
                                    nl->SymbolAtom( "remove" ),
                                    filter,
                                    DRelHelpers::getRemovePartitonAttr( 
                                        spatial2d ) ),
                                nl->TheEmptyList( ),
                                nl->OneElemList(
                                    nl->TwoElemList(
                                        nl->SymbolAtom( "Box" ),
                                        nl->ThreeElemList(
                                            nl->SymbolAtom( "fun" ),
                                            nl->TwoElemList(
                                                nl->SymbolAtom( "tuple_2" ),
                                                nl->SymbolAtom( "TUPLE" ) ),
                                        nl->TwoElemList(
                                            nl->SymbolAtom( "bbox" ),
                                            nl->ThreeElemList(
                                                nl->SymbolAtom( "attr" ),
                                                nl->SymbolAtom( "tuple_2" ),
                                                nl->SymbolAtom( attr ) ) ) ) )
                                                     ) ) ),
                        nl->BoolAtom( true ) ),
                        nl->IntAtom( 37 ) ) ) ) );

        cout << "query" << endl;
        cout << nl->ToString( query ) << endl;
        
        string typeString, errorString;
        bool correct = false;
        bool evaluable = false;
        bool defined = false;
        bool isFunction = false;
        bool resBool = QueryProcessor::ExecuteQuery( query, 
            result, typeString, errorString, correct, evaluable,
            defined, isFunction );

        return correct && evaluable && defined && resBool;
    }

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
    template<class R, class T, class G, bool gridParm>
    int drelspatialpartitionVMT( Word* args, Word& result, int message,
        Word& local, Supplier s ) {

        #ifdef DRELDEBUG
        cout << "drelspatialpartitionVMT" << endl;
        #endif
        DFRel* resultDFRel = ( DFRel* )result.addr;

        int x = qp->GetNoSons( s );

        ListExpr drelType = qp->GetType( qp->GetSon( s, 0 ) );
        R* drel = ( R* )args[ 0 ].addr;
        string attrName = ( ( CcString* )args[ x - 2 ].addr )->GetValue( );

        distributionType dType;
        int attr, key;
        DFRel::checkDistType( 
            nl->Third( qp->GetType( s ) ), dType, attr, key );

        // start partitioner
        PartitionerS<R, T, G>* parti =
            new PartitionerS<R, T, G>(
                attrName, nl->Fourth( nl->Third( qp->GetType( s ) ) ),
                drel, drelType, 1238 );

        // compute the grid
        if( !parti->computeGrid( ) ) {

            resultDFRel->makeUndefined( );
            return 0;
        }

        // start repartitioning
        if( !parti->repartition2DFMatrix( ) ) {

            resultDFRel->makeUndefined( );
            return 0;
        }

        G* grid = parti->getGrid( );
        grid->Copy( );

        DFMatrix* matrix = parti->getDFMatrix( );
        ListExpr matrixType = parti->getMatrixType( );

        string query = "(collect2 " + nl->ToString(
            DRelHelpers::createPointerList( matrixType, matrix ) ) +
            " \"\" 1238)";

        Word resultDFArray;
        if( !QueryProcessor::ExecuteQuery( query, resultDFArray) ) {
            resultDFRel->makeUndefined( );
            return 0;
        }

        DFArray* dfarray = ( DFArray* )resultDFArray.addr;
        resultDFRel->copyFrom( *dfarray );

        delete parti;
        delete dfarray;

        DistTypeSpatial<G>* distType = new DistTypeSpatial<G>( 
            range, attr, key, grid );
        resultDFRel->setDistType( distType );

        return 0;
    }

    template<class R, class T, bool gridParm>
    int drelspatialpartitionVMT( Word* args, Word& result, int message,
        Word& local, Supplier s ) {

        #ifdef DRELDEBUG
        cout << "drelspatialpartitionVMT" << endl;
        #endif
        result = qp->ResultStorage( s );

        distributionType dType;
        int attr, key;
        DFRel::checkDistType( 
            nl->Third( qp->GetType( s ) ), dType, attr, key );

        if( dType == spatial2d ) {
            drelspatialpartitionVMT
                <R, T, temporalalgebra::CellGrid2D, gridParm>(
                    args, result, message, local, s);     
        }
        else {
            drelspatialpartitionVMT
                <R, T, temporalalgebra::CellGrid<3>, gridParm>(
                    args, result, message, local, s);     
        }

        return 0;
    }

/*
1.3 ValueMapping Array for drelspatialpartition

*/
    ValueMapping drelspatialpartitionVM[ ] = {
        drelspatialpartitionVMT<DRel, DArray, false>,
        drelspatialpartitionVMT<DFRel, DFArray, false>,
        drelspatialpartitionVMT<DRel, DArray, true>,
        drelspatialpartitionVMT<DFRel, DFArray, true>
    };

/*
1.4 Selection function for drelspatialpartition

*/
    int drelspatialpartitionSelect( ListExpr args ) {

        int x = nl->HasLength( args, 3 ) 
             && ( Vector::checkType( nl->Third( args ) ) 
                || DRel::checkType( nl->Third( args ) ) 
                || DRel::checkType( nl->Third( args ) ) ) ?
            2 : 0;

        return DRel::checkType( nl->First( args ) ) ? x + 0 : x + 1;
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
        4,
        drelspatialpartitionVM,
        drelspatialpartitionSelect,
        drelspatialpartitionTM
    );

} // end of namespace drel
