
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



1 Implementation of the secondo operator drelexactmatch

*/
#include "NestedList.h"
#include "ListUtils.h"
#include "QueryProcessor.h"
#include "StandardTypes.h"

#include "Algebras/FText/FTextAlgebra.h"

#include "DRel.h"

extern NestedList* nl;
extern QueryProcessor* qp;

namespace distributed2 {
    ListExpr dloop2TM( ListExpr args );

    int dloop2VM( Word* args, Word& result, int message,
        Word& local, Supplier s );
}

using namespace distributed2;

namespace drel {

    /*
    1.1 Type Mapping

    Expect two DRels (one with btree and one with a relation) and a search 
    value.

    */
    ListExpr drelexactmatchTM( ListExpr args ) {

        std::string err = "drel(btree(X)) x drel(rel(X)) x ANY expected";

        if( !nl->HasLength( args, 3 ) ) {
            return listutils::typeError( err +
                ": three arguments are expected" );
        }

        ListExpr temp = args;
        while( !nl->IsEmpty( temp ) ) {
            if( !nl->HasLength( nl->First( temp ), 2 ) ) {
                return listutils::typeError( "internal Error" );
            }
            temp = nl->Rest( temp );
        }

        ListExpr drelBTreeType = nl->First( nl->First( args ) );
        ListExpr drelBTreeName = nl->Second( nl->First( args ) );
        ListExpr bTreeType = nl->Second( drelBTreeType );
        ListExpr drelType = nl->First( nl->Second( args ) );
        ListExpr drelName = nl->Second( nl->Second( args ) );
        ListExpr relType = nl->Second( drelType );

        ListExpr searchValue = nl->Second( nl->Third( args ) );

        if( !DRel::checkType( drelType ) ) {
            return listutils::typeError( err +
                ": first argument is not a drel" );
        }

        ListExpr darrayBTreeType = nl->TwoElemList(
            listutils::basicSymbol<DArray>( ),
            bTreeType );
        ListExpr darrayType = nl->TwoElemList(
            listutils::basicSymbol<DArray>( ),
            relType );

        // create function type to call dloop2TM
        ListExpr funType = nl->TwoElemList(
            nl->FourElemList(
                nl->SymbolAtom( "map" ),
                bTreeType,
                relType,
                relType ),
            nl->FourElemList(
                nl->SymbolAtom( "fun" ),
                nl->TwoElemList(
                    nl->SymbolAtom( "elem11" ),
                    nl->SymbolAtom( "DARRAYELEM" ) ),
                nl->TwoElemList(
                    nl->SymbolAtom( "elem22" ),
                    nl->SymbolAtom( "DARRAYELEM2" ) ),
                nl->TwoElemList(
                    nl->SymbolAtom( "consume" ),
                    nl->FourElemList(
                        nl->SymbolAtom( "exactmatch" ),
                        nl->SymbolAtom( "elem11" ),
                        nl->SymbolAtom( "elem22" ),
                        searchValue ) ) ) );

        // result type of dloop
        ListExpr result = dloop2TM(
            nl->FourElemList(
                nl->TwoElemList( darrayBTreeType, drelBTreeName ),
                nl->TwoElemList( darrayType, drelName ),
                nl->TwoElemList( listutils::basicSymbol<CcString>( ), 
                    nl->StringAtom( "" ) ),
                funType ) );

        if( !nl->HasLength( result, 3 ) ) {
            return result;
        }
        if( !DArray::checkType( nl->Third( result ) ) ) {
            return result;
        }

        ListExpr append = nl->Second( result );
        ListExpr newRes = nl->ThreeElemList( 
            listutils::basicSymbol<DRel>( ),
            nl->Second( nl->Third( result ) ),
            nl->Third( drelType ) );  // disttype

        return nl->ThreeElemList(
            nl->SymbolAtom( Symbols::APPEND( ) ),
            append,
            newRes );
    }

    /*
    1.2 Value Mapping

    Uses a distributed btree and a drel to call the exactmatch operator.

    */
    int drelexactmatchVM( Word* args, Word& result, int message,
        Word& local, Supplier s ) {

        DRel* drelBTree = ( DRel* )args[ 0 ].addr;
        DRel* drel = ( DRel* )args[ 1 ].addr;

        // Compare dist types
        if( !drelBTree->equalDistType<DRel>( drel ) ) {
            result = qp->ResultStorage( s );
            DRel* res = ( DRel* )result.addr;
            res->makeUndefined( );
            return 0;
        }

        ArgVector argVec = { drelBTree,
            drel,
            new CcBool( false ), // dummy
            new CcString( true ),
            args[ 3 ].addr };

        dloop2VM( argVec, result, message, local, s );

        DRel* resultDRel = ( DRel* )result.addr;
        if( !resultDRel->IsDefined( ) ) {
            return 0;
        }

        resultDRel->setDistType( drel->getDistType( )->copy( ) );

        return 0;
    }

    /*
    1.3 Specification of drelexactmatch

    */
    OperatorSpec drelexactmatchSpec(
        " drel(X) x drel(X) x string "
        "-> drel(X) ",
        " _ _ drelexactmatch[_]",
        "Uses a distributed btree and a drel to call the exactmatch operator.",
        " query drel1_Name drel1 drelexactmatch[\"Berlin\"]"
    );

    /*
    1.4 Operator instance of drelexactmatch operator

    */
    Operator drelexactmatchOp(
        "drelexactmatch",
        drelexactmatchSpec.getStr( ),
        drelexactmatchVM,
        Operator::SimpleSelect,
        drelexactmatchTM
    );

} // end of namespace drel