
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



1 Implementation of the secondo operators drelcreatebtree.

This operators have the same value mapping witch calls the dloop operator of 
the Distributed2Algebra.

*/
//#define DRELDEBUG

#include "NestedList.h"
#include "ListUtils.h"
#include "QueryProcessor.h"
#include "StandardTypes.h"

#include "Algebras/Stream/Stream.h"
#include "Algebras/Relation-C++/OperatorFilter.h"
#include "Algebras/Relation-C++/OperatorProject.h"
#include "Algebras/BTree2/op_createbtree2.h"

#include "DRelHelpers.h"
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

    Expects two D[F]Rels.

    */
    ListExpr drelcreatebtreeTM( ListExpr args ) {

        #ifdef DRELDEBUG
        cout << "drelcreatebtreeTM" << endl;
        cout << "args" << endl;
        cout << nl->ToString( args ) << endl;
        #endif

        std::string err = "d[f]rel(X) x string x attr expected";

        if( !nl->HasLength( args, 3 ) ) {
            return listutils::typeError( err +
                ": three arguments are expected" );
        }

        if( !DRelHelpers::isListOfTwoElemLists( args ) ) {
            return listutils::typeError( "internal Error" );
        }

        ListExpr drelType, relType, distType, drelName, darrayType;
        if( !DRelHelpers::isDRelDescr( nl->First( args ), drelType, relType,
            distType, drelName, darrayType ) ) {
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

        #ifdef DRELDEBUG
        cout << "funType" << endl;
        cout << nl->ToString( funType ) << endl;
        #endif

        // result type of dloop
        ListExpr result = dloopTM(
            nl->ThreeElemList(
                nl->TwoElemList( darrayType, drelName ),
                nl->Second( args ),
                funType ) );

        #ifdef DRELDEBUG
        cout << "dloopTM" << endl;
        cout << nl->ToString( result ) << endl;
        #endif

        if( !nl->HasLength( result, 3 ) ) {
            return result;
        }
        if( !DArray::checkType( nl->Third( result ) ) ) {
            return result;
        }

        ListExpr btreeType = nl->Second( nl->Third( result ) );
        ListExpr append = nl->Second( result );
        ListExpr newRes = nl->ThreeElemList(
            nl->First( drelType ),  // drel or dfrel
            btreeType,
            nl->Third( drelType ) );  // disttype

        return nl->ThreeElemList(
            nl->SymbolAtom( Symbols::APPEND( ) ),
            append,
            newRes );
    }

    /*
    1.4 Value Mapping

    Uses a d[f]rel and creates a new drel. The d[f]rel is created by calling 
    the dmap value mapping of the Distributed2Algebra.

    */
    template<class T, class R>
    int dreldloopVMT( Word* args, Word& result, int message,
        Word& local, Supplier s ) {

        #ifdef DRELDEBUG
        cout << "dreldloopVMT" << endl;
        #endif

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
    1.5 ValueMapping Array for dreldloop
    
    Used by the operators with only a drel input.

    */
    ValueMapping dreldloopVM[ ] = {
        dreldloopVMT<DRel, DArray>,
        dreldloopVMT<DFRel, DFArray>
    };

    /*
    1.6 Selection function for dreldloop


    */
    int dreldloopSelect( ListExpr args ) {

        return DRel::checkType( nl->First( args ) ) ? 0 : 1;
    }

    /*
    1.7 Specification of drelcreatebtree

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
    1.11 Operator instance of drelcreatebtree operator

    */
    Operator drelcreatebtreeOp(
        "drelcreatebtree",
        drelcreatebtreeSpec.getStr( ),
        2,
        dreldloopVM,
        dreldloopSelect,
        drelcreatebtreeTM
    );

} // end of namespace drel