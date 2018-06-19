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



1 Implementation of the secondo operator drelcreatebtree

*/
#include "NestedList.h"
#include "ListUtils.h"
#include "QueryProcessor.h"
#include "StandardTypes.h"

#include "DRel.h"

extern NestedList* nl;
extern QueryProcessor* qp;

namespace distributed2 {
    ListExpr dloopTM( ListExpr args );

    template<class A>
    int dloopVMT( Word* args, Word& result, int message,
        Word& local, Supplier s );
}

using namespace distributed2;

namespace drel {

    /*
    1.1 Type Mapping

    Expect a DRel or DFRel and another DRel or DFRel.

    */
    ListExpr drelcreatebtreeTM( ListExpr args ) {

        std::string err = "d[f]rel(X) x string x attr expected";

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

        ListExpr drelType = nl->First( nl->First( args ) );
        ListExpr drelName = nl->Second( nl->First( args ) );
        ListExpr relType = nl->Second( drelType );

        ListExpr darrayType;
        if( DRel::checkType( drelType ) ) {
            darrayType = nl->TwoElemList(
                listutils::basicSymbol<DArray>( ),
                relType );
        }
        else if( DFRel::checkType( drelType ) ) {
            darrayType = nl->TwoElemList(
                listutils::basicSymbol<DFArray>( ),
                relType );
        }
        else {
            return listutils::typeError( err +
                ": first argument is not a d[f]rel" );
        }

        // create function type to call dloopTM
        ListExpr funType = nl->TwoElemList(
            nl->ThreeElemList(
                nl->SymbolAtom( "map" ),
                relType,
                nl->ThreeElemList(
                    nl->SymbolAtom( "btree" ),
                    nl->Second( relType ),
                    listutils::basicSymbol<CcInt>( ) ) ),
            nl->ThreeElemList(
                nl->SymbolAtom( "fun" ),
                nl->TwoElemList(
                    nl->SymbolAtom( "darrayelem1" ),
                    nl->SymbolAtom( "ARRAYFUNARG1" ) ),
                nl->ThreeElemList(
                    nl->SymbolAtom( "createbtree" ),
                    nl->SymbolAtom( "darrayelem1" ),
                    nl->Second( nl->Third( args ) ) ) ) );     // Attribute

        // result type of dloop
        ListExpr result = dloopTM(
            nl->ThreeElemList(
                nl->TwoElemList( darrayType, drelName ),
                nl->Second( args ),
                funType ) );

        if( !nl->HasLength( result, 3 ) ) {
            return result;
        }
        if( !DArray::checkType( nl->Third( result ) ) ) {
            return result;
        }

        ListExpr btreeType = nl->Second( nl->Third( result ) );
        cout << "btreeType" << endl;
        cout << nl->ToString( btreeType ) << endl;
        ListExpr append = nl->Second( result );
        ListExpr newRes = nl->ThreeElemList(
            nl->First( drelType ),  // drel or dfrel
            btreeType,
            /*nl->TwoElemList(
                nl->First( btreeType ),
                nl->Second( btreeType ) ),*/
            nl->Third( drelType ) );  // disttype

        return nl->ThreeElemList(
            nl->SymbolAtom( Symbols::APPEND( ) ),
            append,
            newRes );
    }

    /*
    1.2 Value Mapping

    Creates a distributed btree for a d[f]rel.

    */
    template<class T, class R>
    int drelcreatebtreeVMT( Word* args, Word& result, int message,
        Word& local, Supplier s ) {

        dloopVMT<R>( args, result, message, local, s );

        T* drel = ( T* )args[ 0 ].addr;
        T* resultDRel = ( T* )result.addr;

        if( !resultDRel->IsDefined( ) ) {
            return 0;
        }

        resultDRel->setDistType( drel->getDistType( )->copy( ) );

        return 0;
    }

    /*
    1.3 ValueMapping Array of drecreatebtree

    */
    ValueMapping drelcreatebtreeVM[ ] = {
        drelcreatebtreeVMT<DRel, DArray>,
        drelcreatebtreeVMT<DFRel, DFArray>
    };

    /*
    1.4 Selection function

    */
    int drelcreatebtreeSelect( ListExpr args ) {

        return nl->SymbolValue( nl->First( nl->First( args ) ) ) ==
            DRel::BasicType( ) ? 0 : 1;
    }

    /*
    1.5 Specification of drecreatebtree

    */
    OperatorSpec drelcreatebtreeSpec(
        " d[f]rel(X) x string x attr "
        "-> d[f]rel(Y) ",
        " _ drelcreatebtree[_,_]",
        "Compares to drels and return true, if the disttype are "
        "equal. ",
        " query drel1 drelcreatebtree[\"drel1_Name\", Name]"
    );

    /*
    1.6 Operator instance of drecreatebtree operator

    */
    Operator drelcreatebtreeOp(
        "drelcreatebtree",
        drelcreatebtreeSpec.getStr( ),
        2,
        drelcreatebtreeVM,
        drelcreatebtreeSelect,
        drelcreatebtreeTM
    );

} // end of namespace drel