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
#include "Algebras/Relation-C++/OperatorFilter.h"
#include "Algebras/Relation-C++/OperatorProject.h"
#include "Algebras/Distributed2/CommandLogger.h"

#include "DRelHelpers.h"
#include "DRel.h"
#include "Partitioner.hpp"
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
1.1 Type Mapping ~drelpartitionTM~

Expect a d[f]rel and an attribute name to repartition the given d[f]rel.

*/
    ListExpr drelpartitionTM( ListExpr args ) {

        #ifdef DRELDEBUG
        cout << "drelpartitionTM" << endl;
        cout << "args" << endl;
        cout << nl->ToString( args ) << endl;
        #endif

        std::string err = "d[f]rel(X) x attr x [int|vector|d[f]rel] expected";

        if( !nl->HasLength( args, 2 )
         && !nl->HasLength( args, 3 ) ) {
            return listutils::typeError( err +
                ": two or three arguments are expected1" );
        }

        distributionType type;
        if( !DRelHelpers::drelCheck( nl->First( args ), type ) ) {
            return listutils::typeError(
                err + ": first argument is not a d[f]rel" );
        }

        ListExpr relType = nl->Second( nl->First( args ) );

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

        int key = rand( );
        if( nl->HasLength( args, 3 ) ) {
            if( Vector::checkType( nl->Third( args ) ) ) {
                if( !nl->Equal( nl->Second( nl->Third( args ) ), attrType ) ) {
                    return listutils::typeError(
                        err + ": attr type does not fit to the vector type" );
                }
            }
            else if( DRel::checkType( nl->Third( args ) )
                  || DFRel::checkType( nl->Third( args ) ) ) {
                if( !nl->HasLength( nl->Third( nl->Third( args ) ), 4 ) ) {
                    return listutils::typeError(
                        err + ": third argument is a d[f]rel, but it is "
                        "not partitioned by range" );
                }
                if( !nl->Equal( 
                    nl->Second( nl->Fourth( nl->Third( nl->Third( args ) ) ) ),
                    attrType ) ) {
                    return listutils::typeError(
                        err + ": d[f]rel argument is partitioned by range, but"
                        "the partioning attribute has a wrong type" );
                }
                key = nl->IntValue( 
                    nl->Third( nl->Third( nl->Third( args ) ) ) );
            }
            else if( !CcInt::checkType( nl->Third( args ) ) ) {
                return listutils::typeError( err +
                    ": third argument is not an integer or an vector3" );
            }
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
                        DRelHelpers::removePartitionAttributes( 
                            attrList, type ) ) ),
                nl->FourElemList(
                    nl->IntAtom( range ),
                    nl->IntAtom( pos - 1 ),
                    nl->IntAtom( key ),
                    nl->TwoElemList(
                        nl->SymbolAtom( Vector::BasicType( ) ),
                        attrType ) ) );

        return nl->ThreeElemList( 
            nl->SymbolAtom( Symbols::APPEND( ) ),
            appendList,
            resultType );
    }

/*
1.2 Value Mapping ~drelpartitionVMT~

Uses a d[f]rel and an attribute to repartition the d[f]rel by the given 
attribute.

*/
    template<class R, class T, bool boundaryParm>
    int drelpartitionVMT( Word* args, Word& result, int message,
        Word& local, Supplier s ) {

        #ifdef DRELDEBUG
        cout << "drelpartitionVMT" << endl;
        #endif
      
        ListExpr boundaryType = nl->Fourth( nl->Third( qp->GetType( s ) ) );
        ListExpr drelType = qp->GetType( qp->GetSon( s, 0 ) );
        R* drel = ( R* )args[ 0 ].addr;
        int key = nl->IntValue( nl->Third( nl->Third( qp->GetType( s ) ) ) );

        int x = ( qp->GetNoSons(s) == 5 ) ? 1 : 0;
        std::string attrName = ( ( CcString* )args[ 2 + x ].addr )->GetValue( );
        int pos = ( ( CcInt* )args[ 3 + x ].addr )->GetValue( );

        collection::Collection* boundary;

        if( boundaryParm ) {

            boundary = ( collection::Collection* )args[ 2 ].addr;
        }
        else {

            BoundaryCalculator<R>* calc = new BoundaryCalculator<R>( 
                attrName, boundaryType, drel, drelType, getDRelPort() );

            if( x == 1 ) {
                int count = ( ( CcInt* )args[ 2 ].addr )->GetIntval( );
                calc->setCount( count );
            }
            else {
                if( !calc->countDRel( ) ){
                    cout << "error while determining the relation size" 
                         << endl;
                    result = qp->ResultStorage( s );
                    ( ( DFRel* )result.addr )->makeUndefined( );
                    return 0;
                }
            }

            if( !calc->computeBoundary( ) ){
                cout << "error while computing the boundaries" << endl;
                result = qp->ResultStorage( s );
                ( ( DFRel* )result.addr )->makeUndefined( );
                return 0;
            }

            boundary = calc->getBoundary( );

            delete calc;
        }

        Partitioner<R, T>* parti = new Partitioner<R, T>( attrName, 
            boundaryType, drel, drelType, boundary, getDRelPort() );

        if( !parti->repartition2DFMatrix( ) ) {
            cout << "repartition failed!!" << endl;
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
        DFRel* resultDrel = ( DFRel* )result.addr;

        delete name;
        delete port;
        delete parti;

        DistTypeRange* distType = new DistTypeRange( 
            range, pos, key, boundary );
        resultDrel->setDistType( distType );

        return 0;
    }

/*
1.3 ValueMapping Array for drelpartition

*/
    ValueMapping drelpartitionVM[ ] = {
        drelpartitionVMT<DRel, DArray, false>,
        drelpartitionVMT<DFRel, DFArray, false>,
        drelpartitionVMT<DRel, DArray, true>,
        drelpartitionVMT<DFRel, DFArray, true>
    };

/*
1.4 Selection function for drelpartition

*/
    int drelpartitionSelect( ListExpr args ) {

        int x = nl->HasLength( args, 3 ) 
             && ( Vector::checkType( nl->Third( args ) ) 
                || DRel::checkType( nl->Third( args ) ) 
                || DRel::checkType( nl->Third( args ) ) ) ?
            2 : 0;

        return DRel::checkType( nl->First( args ) ) ? x + 0 : x + 1;
    }

/*
1.5 Specification of drelpartition

*/
    OperatorSpec drelpartitionSpec(
        " d[f]rel(X) x attr x [int|vector|d[f]rel] "
        "-> dfrel(X) ",
        " _ drelpartition[_,_]",
        "Repartition of a d[f]rel with partition by range. The attribute is "
        "the key attribute to repartition the d[f]rel. Optional the size of "
        "the d[f]rel can be used as an argument if known or a vector for the "
        "boundaries can be used. A existing by range d[f]rel can also be used "
        "as third argument",
        " query drel1 drelpartition[PLZ]"
    );

/*
1.6 Operator instance of drelpartition operator

*/
    Operator drelpartitionOp(
        "drelpartition",
        drelpartitionSpec.getStr( ),
        4,
        drelpartitionVM,
        drelpartitionSelect,
        drelpartitionTM
    );

} // end of namespace drel
