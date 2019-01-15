
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



1 Implementation of the secondo operators

Implementation of drelcreatebtree and drelbulkloadrtree.
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

Type mapping for drelcreatebtree. Expect a drel, a string and an attribute.

*/
    ListExpr drelcreatebtreeTM( ListExpr args ) {

        #ifdef DRELDEBUG
        cout << "drelcreatebtreeTM" << endl;
        cout << "args" << endl;
        cout << nl->ToString( args ) << endl;
        #endif

        std::string err = "dfrel(X) x string x attr expected";

        if( !nl->HasLength( args, 3 ) ) {
            return listutils::typeError( err +
                ": three arguments are expected" );
        }

        if( !DRelHelpers::isListOfTwoElemLists( args ) ) {
            return listutils::typeError( "internal Error" );
        }

        ListExpr arg1Type = nl->First( nl->First( args ) );
        ListExpr arg2Type = nl->First( nl->Second( args ) );
        ListExpr arg3Type = nl->First( nl->Third( args ) );
        ListExpr arg1Value = nl->Second( nl->First( args ) );
        ListExpr arg3Value = nl->Second( nl->Third( args ) );

        ListExpr darrayType;
        if( !DRelHelpers::drelCheck( arg1Type, darrayType ) ) {
            return listutils::typeError(
                err + ": first argument is not a d[f]rel" );
        }

        if( !CcString::checkType( arg2Type ) ) {
            return listutils::typeError(
                err + ": second argument is not a string" );
        }

        if( !nl->IsAtom( arg3Type ) ) {
            return listutils::typeError(
                err + ": thrid argument is not an attribute" );
        }

        ListExpr relType =  nl->Second( darrayType );

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
                    arg3Value ) ) );     // Attribute

        #ifdef DRELDEBUG
        cout << "funType" << endl;
        cout << nl->ToString( funType ) << endl;
        #endif

        // result type of dloop
        ListExpr result = dloopTM(
            nl->ThreeElemList(
                nl->TwoElemList( darrayType, arg1Value ),
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

        return nl->ThreeElemList(
            nl->SymbolAtom( Symbols::APPEND( ) ),
            nl->Second( result ),
            nl->Third( result ) );
    }

/*
1.1 Type Mapping

Type mapping for drelbulkloadrtree. Expect a drel, a string and an attribute.

*/
    ListExpr drelbulkloadrtreeTM( ListExpr args ) {

        #ifdef DRELDEBUG
        cout << "drelbulkloadrtreeTM" << endl;
        cout << "args" << endl;
        cout << nl->ToString( args ) << endl;
        #endif

        std::string err = "dfrel(X) x string x attr expected";

        if( !nl->HasLength( args, 3 ) ) {
            return listutils::typeError( err +
                ": three arguments are expected" );
        }

        if( !DRel::checkType( nl->First( nl->First( args ) ) ) ) {
            return listutils::typeError(
                err + ": first argument is not a d[f]rel" );
        }

        if( !CcString::checkType( nl->First( nl->Second( args ) ) ) ) {
            return listutils::typeError(
                err + ": second argument is not a string" );
        }

        ListExpr relType =  nl->Second( nl->First( nl->First( args ) ) );
        ListExpr darrayType = nl->TwoElemList(
            listutils::basicSymbol<DArray>( ),
            relType );

        // create function type to call dloopTM
        ListExpr funType = nl->TwoElemList(
            nl->ThreeElemList(
                nl->SymbolAtom( "map" ),
                relType,
                nl->FourElemList(
                    nl->SymbolAtom( "rtree" ),
                    nl->Second( relType ),
                    nl->Second( nl->Third( args )),
                    nl->BoolAtom( false ) ) ),
            nl->ThreeElemList(
                nl->SymbolAtom( "fun" ),
                nl->TwoElemList(
                    nl->SymbolAtom( "darrayelem_1" ),
                    nl->SymbolAtom( "ARRAYFUNARG1" ) ),
                nl->ThreeElemList(
                    nl->SymbolAtom( "bulkloadrtree" ),
                    nl->ThreeElemList(
                      nl->SymbolAtom("remove"),
                      nl->ThreeElemList(
                        nl->SymbolAtom( "sortby" ),
                        nl->ThreeElemList(
                            nl->SymbolAtom( "extend" ),
                            nl->TwoElemList(
                                nl->SymbolAtom( "feed" ),
                                nl->SymbolAtom( "darrayelem_1" ) ),
                            nl->TwoElemList(
                                nl->TwoElemList(
                                    nl->SymbolAtom( "TID" ),
                                    nl->ThreeElemList(
                                        nl->SymbolAtom( "fun" ),
                                        nl->TwoElemList(
                                            nl->SymbolAtom( "tuple_2" ),
                                            nl->SymbolAtom( "TUPLE" ) ),
                                        nl->TwoElemList(
                                            nl->SymbolAtom( "tupleid" ),
                                            nl->SymbolAtom( "tuple_2" ) ) ) ),
                                nl->TwoElemList(
                                    nl->SymbolAtom( "MBR" ),
                                    nl->ThreeElemList(
                                        nl->SymbolAtom( "fun" ),
                                        nl->TwoElemList(
                                            nl->SymbolAtom( "tuple_3" ),
                                            nl->SymbolAtom( "TUPLE" ) ),
                                        nl->ThreeElemList(
                                            nl->SymbolAtom( "attr" ),
                                            nl->SymbolAtom( "tuple_3" ),
                                            nl->Second( nl->Third( args ) 
                                                ) ) ) ) ) ), // Attribute
                          nl->OneElemList( 
                              nl->SymbolAtom( "MBR" ) ) ), // sortby
                          nl->OneElemList( nl->SymbolAtom("MBR"))
                        ),
                        nl->Second( nl->Third( args )) )));

        #ifdef DRELDEBUG
        cout << "funType" << endl;
        cout << nl->ToString( funType ) << endl;
        #endif

        // result type of dloop
        ListExpr result = dloopTM(
            nl->ThreeElemList(
                nl->TwoElemList( darrayType, nl->Second( nl->First( args ) ) ),
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

        return nl->ThreeElemList(
            nl->SymbolAtom( Symbols::APPEND( ) ),
            nl->Second( result ),
            nl->Third( result ) );
    }

/*
1.4 Value Mapping

Uses a d[f]rel and creates a new drel. The d[f]rel is created by calling 
the dloop value mapping of the Distributed2Algebra.

*/
    template<class T>
    int dreldloopVMT( Word* args, Word& result, int message,
        Word& local, Supplier s ) {

        #ifdef DRELDEBUG
        cout << "dreldloopVMT" << endl;
        #endif

        dloopVMT<T>( args, result, message, local, s );

        return 0;
    }

/*
1.5 ValueMapping Array dreldloop
    
Used by the operators with only a drel input.

*/
    ValueMapping dreldloopVM[ ] = {
        dreldloopVMT<DArray>,
        dreldloopVMT<DFArray>
    };

/*
1.6 Selection function for dloop operators


*/
    int dreldloopSelect( ListExpr args ) {

        return DRel::checkType( nl->First( args ) ) ? 0 : 1;
    }

/*
1.7 Specifications

1.7.1 Specification of drelcreatebtree

*/
    OperatorSpec drelcreatebtreeSpec(
        " d[f]rel(X) x string x attr "
        "-> darray(Y) ",
        " _ drelcreatebtree[_]",
        "Creates a btree for a d[f]rel as a darray ",
        " query drel1 drelcreatebtree[\"\",Name]"
    );

/*
1.7.2 Specification of drelbulkloadtree

*/
    OperatorSpec drelbulkloadrtreeSpec(
        " d[f]rel(X) x string x attr "
        "-> darray(Y) ",
        " _ drelbulkloadrtree[_,_]",
        "Creates a rtree for a d[f]rel as a darray ",
        " query drel1 drelbulkloadrtree[\"drel1_Name\", GeoData]"
    );

/*
1.8 Operator instances

1.8.1 Operator instance of drelcreatebtree operator

*/
    Operator drelcreatebtreeOp(
        "drelcreatebtree",
        drelcreatebtreeSpec.getStr( ),
        2,
        dreldloopVM,
        dreldloopSelect,
        drelcreatebtreeTM
    );

/*
1.8.2 Operator instance of drelbulkloadrtree operator

*/
    Operator drelbulkloadrtreeOp(
        "drelbulkloadrtree",
        drelbulkloadrtreeSpec.getStr( ),
        2,
        dreldloopVM,
        dreldloopSelect,
        drelbulkloadrtreeTM
    );

} // end of namespace drel
