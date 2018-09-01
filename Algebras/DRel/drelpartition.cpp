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
#define DRELDEBUG

#include "NestedList.h"
#include "ListUtils.h"
#include "QueryProcessor.h"
#include "StandardTypes.h"
#include "SecParser.h"

#include "Algebras/Stream/Stream.h"
#include "Algebras/FText/FTextAlgebra.h"
#include "Algebras/Relation-C++/OperatorFilter.h"
#include "Algebras/Relation-C++/OperatorProject.h"
#include "Algebras/Distributed2/CommandLogger.h"
#include "Algebras/Distributed2/Distributed2Algebra.h"

#include "DRelHelpers.h"
#include "DRel.h"
#include "Partitionier.hpp"

extern NestedList* nl;
extern QueryProcessor* qp;

namespace distributed2 {

    extern Distributed2Algebra* algInstance;

    template<class A>
    int dmapVMT( Word* args, Word& result, int message,
        Word& local, Supplier s );
};

using namespace distributed2;

namespace drel {

/*
1.1 Type Mapping ~drelpartitionTM~

Expect a d[f]rel and an attribute name to repartition the given d[f]rel.

*/
    ListExpr drelpartitionTM( ListExpr args ) {

        std::string err = "d[f]rel(X) x attr expected";

        if( !nl->HasLength( args, 2 ) ) {
            return listutils::typeError( err +
                ": two arguments are expected" );
        }

        if( !DRel::checkType( nl->First( args ) )
         && !DFRel::checkType( nl->First( args ) ) ) {
            return listutils::typeError( err +
                ": first argument is not a d[f]rel" );
        }
        ListExpr relType = nl->Second( nl->First( args ) );
        if( !Relation::checkType( relType ) ) {
            return listutils::typeError( err +
                ": first argument is not a d[f]rel" );
        }

        ListExpr distType = nl->Third( nl->First( args ) );

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

        // Compare the choosen attribute with the current partion of the drel
        if( nl->HasMinLength( distType, 2 ) ) {

            distributionType type;
            if( !getTypeByNum( ( int )( nl->IntValue( nl->First( distType ) ) ),
                    type ) ) {
                return listutils::typeError(
                    err + ": internal error" );
            }

            if( type == range ) {
                int attrNum = ( int )
                    ( nl->IntValue( nl->Second( distType ) ) );

                if( attrNum == pos - 1 ) { 
                    // repartion not required
                    return listutils::typeError(
                        "d[f]rel is already partitioned by range with "
                        "attribute" + nl->ToString( nl->Second( args ) ) );
                }
            }

        }

        ListExpr appendList = nl->ThreeElemList(
            nl->StringAtom( nl->SymbolValue( attrType ) ),
            nl->StringAtom( attrName ),
            nl->IntAtom( pos - 1 ) );

        return nl->ThreeElemList( 
            nl->SymbolAtom( Symbols::APPEND( ) ),
            appendList,
            nl->ThreeElemList(
                listutils::basicSymbol<DFRel>( ),
                nl->Second( nl->First( args ) ),
                nl->FourElemList(
                        nl->IntAtom( range ),
                        nl->IntAtom( pos - 1 ),
                        nl->IntAtom( rand( ) ),
                        nl->TwoElemList(
                            nl->SymbolAtom( Vector::BasicType( ) ),
                            attrType ) ) ) );
    }

/*
1.2 Value Mapping ~drelpartitionVMT~

Uses a d[f]rel and an attribute to repartition the d[f]rel by the given 
attribute.

*/
    template<class R, class T>
    int drelpartitionVMT( Word* args, Word& result, int message,
        Word& local, Supplier s ) {
        
        R* drel = ( R* )args[ 0 ].addr;
        string attrType = ( ( CcString* )args[ 2 ].addr )->GetValue( );
        string attrName = ( ( CcString* )args[ 3 ].addr )->GetValue( );
        int pos = ( ( CcInt* )args[ 4 ].addr )->GetValue( );

        string boundaryName = distributed2::algInstance->getTempName();

        Partitionier<R, T>* parti = new Partitionier<R, T>( attrName, attrType, 
            drel, qp->GetType( qp->GetSon( s, 0 ) ), 1238, boundaryName );

        if( !parti->repartition2DFArray( result ) ) {
            result = qp->ResultStorage( s );
            ( ( DFRel* )result.addr )->makeUndefined( );
        }

        collection::Collection* boundary = parti->getBoundary( );

        DFRel* resultDrel = ( DFRel* )result.addr;
        DistTypeRange* distType = new DistTypeRange( range, pos, boundary );
        resultDrel->setDistType( distType );

        return 0;
    }

/*
1.3 ValueMapping Array for drelpartition

*/
    ValueMapping drelpartitionVM[ ] = {
        drelpartitionVMT<DRel, DArray>,
        drelpartitionVMT<DFRel, DFArray>
    };

/*
1.4 Selection function for drelpartition

*/
    int drelpartitionSelect( ListExpr args ) {

        return DRel::checkType( nl->First( args ) ) ? 0 : 1;
    }

/*
1.5 Specification of drelpartition

*/
    OperatorSpec drelpartitionSpec(
        " d[f]rel(X) x attr "
        "-> dfrel(X) ",
        " _ drelpartition[_]",
        "Repartition of a d[f]rel by partition by range. The attribute is "
        "the key attribute to repartition the d[f]rel",
        " query drel1 drelpartition[PLZ]"
    );

/*
1.6 Operator instance of drelpartition operator

*/
    Operator drelpartitionOp(
        "drelpartition",
        drelpartitionSpec.getStr( ),
        2,
        drelpartitionVM,
        drelpartitionSelect,
        drelpartitionTM
    );

} // end of namespace drel