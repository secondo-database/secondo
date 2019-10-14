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


1 Implementation of the secondo operators drelfilter, project, 
drelprojectextend, drellsortby, drellgroupby, lsort, lrdup, 
lrename, head and drelextend

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
#include "Algebras/Relation-C++/OperatorProject.h"
#include "Algebras/Relation-C++/OperatorFilter.h"
#include "Algebras/Rectangle/RectangleAlgebra.h"
#include "Algebras/Distributed2/CommandLogger.h"
#include "Algebras/Distributed2/Distributed2Algebra.h"

#include "Algebras/FText/FTextAlgebra.h"

#include "DRelHelpers.h"
#include "DRel.h"
#include "drelport.h"

extern NestedList* nl;
extern QueryProcessor* qp;

ListExpr RenameTypeMap( ListExpr args );
template<int operatorId>
    ListExpr IndexQueryTypeMap( ListExpr args );

namespace distributed2 {

    extern Distributed2Algebra* algInstance;

    ListExpr dmapTM( ListExpr args );

    template<int x>
    ListExpr dmapXTMT( ListExpr args );

    template<class A>
    int dmapVMT( Word* args, Word& result, int message,
        Word& local, Supplier s );

    int dmapXVM(Word* args, Word& result, int message,
            Word& local, Supplier s );

    template<class A, bool fromCat>
    int shareVMT(Word* args, Word& result, int message,
            Word& local, Supplier s );
}

namespace extrelationalg {
    ListExpr ExtendTypeMap( ListExpr args );
    ListExpr ExtProjectExtendTypeMap( ListExpr args );
    ListExpr SortByTypeMap( ListExpr args );
    ListExpr GroupByTypeMap( ListExpr args );
}

using namespace distributed2;

namespace drel {

/*
1.1 Type Mappings for all operators using dmapVM

1.1.1 Type Mapping ~drelfilterTM~

Expect a d[f]rel with a function to filter the tuples. Type mapping for the 
drelfilter operator.

*/
    ListExpr drelfilterTM( ListExpr args ) {

        #ifdef DRELDEBUG
        cout << "drelfilterTM" << endl;
        cout << "args" << endl;
        cout << nl->ToString( args ) << endl;
        #endif

        std::string err = "d[f]rel(X) x fun expected";

        if( !nl->HasLength( args, 2 ) ) {
            return listutils::typeError( err +
                ": two arguments are expected" );
        }

        if( !DRelHelpers::isListOfTwoElemLists( args ) ) {
            return listutils::typeError( "internal Error" );
        }

        ListExpr drelType, relType, distType, drelValue, darrayType;
        if( !DRelHelpers::isDRelDescr( nl->First( args ), drelType, relType, 
            distType, drelValue, darrayType ) ) {
            return listutils::typeError( err +
                ": first argument is not a d[f]rel" );
        }

        ListExpr fun, map;
        if( !DRelHelpers::replaceDRELFUNARG( nl->Second( args ),
            "STREAMELEM", fun, map ) ) {
            return listutils::typeError( err +
                ": error in the function format" );
        }

        ListExpr result = OperatorFilter::FilterTypeMap(
            nl->TwoElemList(
                nl->TwoElemList(
                    nl->TwoElemList(
                        listutils::basicSymbol<Stream<Tuple> >( ),
                        nl->Second( relType ) ),
                    nl->TwoElemList(
                        nl->SymbolAtom( "feed" ),
                        drelValue ) ), // only dummy for the filter TM
                nl->TwoElemList( map, fun ) ) );

        #ifdef DRELDEBUG
        cout << "filter tm" << endl;
        cout << nl->ToString( result ) << endl;
        #endif

        // filter TM ok?
        if( !listutils::isTupleStream( result ) ) {
            return result;
        }

        // create string to call dmap the call is devided in two parts
        // the argument for the drel is missing and will be filled in the
        // value mapping
        std::string funText1 = "(dmap ";
        std::string funText2 = "\"\" (fun (dmapelem_1 ARRAYFUNARG1) "
            "(filter (feed dmapelem_1)" + nl->ToString( fun ) + 
            ") ) )";

        #ifdef DRELDEBUG
        cout << "funText1" << endl;
        cout << funText1 << endl;
        cout << "funText2" << endl;
        cout << funText2 << endl;
        #endif

        ListExpr resultType = nl->ThreeElemList(
            listutils::basicSymbol<DFRel>( ),
            relType,
            nl->Third( drelType ) );  // disttype

        ListExpr append = nl->TwoElemList( 
            nl->TextAtom( funText1 ),
            nl->TextAtom( funText2 ) );

        return nl->ThreeElemList(
            nl->SymbolAtom( Symbols::APPEND( ) ),
            append,
            resultType );
    }

/*
1.1.2 Type Mapping ~projectTM~

Expect a d[f]rel an attribute list. Type mapping for the project 
operator.

*/
    ListExpr projectTM( ListExpr args ) {

        #ifdef DRELDEBUG
        cout << "projectTM" << endl;
        cout << "args" << endl;
        cout << nl->ToString( args ) << endl;
        #endif

        std::string err = "d[f]rel(X) x attrlist expected";

        if( !nl->HasLength( args, 2 ) ) {
            return listutils::typeError( err +
                ": two arguments are expected" );
        }

        if( !DRelHelpers::isListOfTwoElemLists( args ) ) {
            return listutils::typeError( "internal Error" );
        }

        ListExpr drelType, relType, distType, drelValue, darrayType;
        if( !DRelHelpers::isDRelDescr( nl->First( args ), drelType, relType, 
            distType, drelValue, darrayType ) ) {
            return listutils::typeError( err +
                ": first argument is not a d[f]rel" );
        }

        ListExpr attrlist = nl->First( nl->Second( args ) );

        ListExpr result = OperatorProject::ProjectTypeMap(
            nl->TwoElemList(
                nl->TwoElemList(
                    listutils::basicSymbol<Stream<Tuple> >( ),
                    nl->Second( relType ) ),
                attrlist ) );

        #ifdef DRELDEBUG
        cout << "project tm" << endl;
        cout << nl->ToString( result ) << endl;
        #endif

        // project TM ok?
        if( !nl->HasLength( result, 3 ) ) {
            return result;
        }
        if( !listutils::isTupleStream( nl->Third( result ) ) ) {
            return result;
        }

        relType = nl->TwoElemList(
            listutils::basicSymbol<Relation>( ),
            nl->Second( nl->Third( result ) ) );

        #ifdef DRELDEBUG
        cout << "distType with attr pos to check" << endl;
        cout << nl->ToString( distType ) << endl;
        #endif

        // distribution by an attribute?
        if( nl->HasMinLength( distType, 2 ) ) {

            int newPos = nl->IntValue( nl->Second( distType ) ) + 1;
            #ifdef DRELDEBUG
            cout << "distribution by attribute check for new position" << endl;
            cout << "newPos" << endl;
            cout << newPos << endl;
            #endif

            if( DistTypeHash::computeNewAttrPos( 
                nl->Second( nl->Second( result ) ), newPos ) ) {

                switch( nl->ListLength( distType ) ) {
                case 3:
                    distType = nl->ThreeElemList(
                        nl->First( distType ),
                        nl->IntAtom( newPos ),
                        nl->Third( distType ) );
                    break;
                case 4:
                    distType = nl->FourElemList(
                        nl->First( distType ),
                        nl->IntAtom( newPos ),
                        nl->Third( distType ),
                        nl->Fourth( distType ) );
                    break;
                default:
                    distType = nl->TwoElemList(
                        nl->First( distType ),
                        nl->IntAtom( newPos ) );
                }
                
                #ifdef DRELDEBUG
                cout << "new distType" << endl;
                cout << nl->ToString( distType ) << endl;
                #endif
            }
            else {
                return listutils::typeError( err +
                    ": it is not allowed to project without the "
                    "distribution attribute" );
            }

        }

        // create string to call dmap the call is devided in two parts
        // the argument for the drel is missing and will be filled in the
        // value mapping
        std::string funText1 = "(dmap ";
        std::string funText2 = "\"\" (fun (dmapelem_1 ARRAYFUNARG1) "
            "(project (feed dmapelem_1)" + nl->ToString( attrlist ) + 
            ") ) )";

        #ifdef DRELDEBUG
        cout << "funText1" << endl;
        cout << funText1 << endl;
        cout << "funText2" << endl;
        cout << funText2 << endl;
        #endif

        ListExpr resultType = nl->ThreeElemList(
            listutils::basicSymbol<DFRel>( ),
            relType,
            distType );

        ListExpr append = nl->TwoElemList( 
            nl->TextAtom( funText1 ),
            nl->TextAtom( funText2 ) );

        return nl->ThreeElemList(
            nl->SymbolAtom( Symbols::APPEND( ) ),
            append,
            resultType );
    }

/*
1.1.3 Type Mapping ~drelprojectextendTM~

Expect a d[f]rel with an attribute list and a function list to extend the 
tuples. This is a combination of the operators project and extend.

*/
    ListExpr drelprojectextendTM( ListExpr args ) {

        #ifdef DRELDEBUG
        cout << "drelprojectextendTM" << endl;
        cout << "args" << endl;
        cout << nl->ToString( args ) << endl;
        #endif

        std::string err = "d[f]rel(X) x attrlist x funlist expected";

        if( !nl->HasLength( args, 3 ) ) {
            return listutils::typeError( err +
                ": three arguments are expected" );
        }

        if( !DRelHelpers::isListOfTwoElemLists( args ) ) {
            return listutils::typeError( "internal Error" );
        }

        ListExpr drelType, relType, distType, drelValue, darrayType;
        if( !DRelHelpers::isDRelDescr( nl->First( args ), drelType, relType, 
            distType, drelValue, darrayType ) ) {
            return listutils::typeError( err +
                ": first argument is not a d[f]rel" );
        }

        ListExpr attrlist = nl->First( nl->Second( args ) );

        if( !nl->HasMinLength( nl->First( nl->Third( args ) ), 1 )
         || !nl->HasMinLength( nl->Second( nl->Third( args ) ), 1 ) ) {
            return listutils::typeError( err +
                ": error in the function format" );
        }

        ListExpr map = nl->First( nl->Third( args ) );

        ListExpr tempfun;
        ListExpr fun = nl->TheEmptyList( );
        ListExpr temp = nl->Second( nl->Third( args ) );
        while( !nl->IsEmpty( temp ) ) {

            if( !nl->HasLength( nl->First( temp ), 2 ) ) {
                return listutils::typeError( "internal Error" );
            }

            if( !DRelHelpers::replaceDRELFUNARG( 
                nl->Second( nl->First( temp ) ), 
                "TUPLE", tempfun ) ) {
                return listutils::typeError( err +
                    ": error in the function format" );
            }

            if( nl->IsEmpty( fun ) ) {
                fun = nl->OneElemList( nl->TwoElemList( 
                    nl->First( nl->First( temp ) ), tempfun ) );
            } else {
                fun = listutils::concat( fun, 
                    nl->OneElemList( nl->TwoElemList(
                    nl->First( nl->First( temp ) ), tempfun ) ) );
            }

            temp = nl->Rest( temp );
        }

        ListExpr result = extrelationalg::ExtProjectExtendTypeMap(
            nl->ThreeElemList(
                nl->TwoElemList(
                    listutils::basicSymbol<Stream<Tuple> >( ),
                    nl->Second( relType ) ), 
                attrlist,
                map ) );

        #ifdef DRELDEBUG
        cout << "projectextend tm" << endl;
        cout << nl->ToString( result ) << endl;
        #endif

        // filter TM ok?
        if( !nl->HasLength( result, 3 ) ) {
            return result;
        }
        if( !listutils::isTupleStream( nl->Third( result ) ) ) {
            return result;
        }

        relType = nl->TwoElemList(
            listutils::basicSymbol<Relation>( ),
            nl->Second( nl->Third( result ) ) );

        // distribution by an attribute?
        if( nl->HasMinLength( distType, 2 ) ) {

            int newPos = nl->IntValue( nl->Second( distType ) ) + 1;
            #ifdef DRELDEBUG
            cout << "distribution by attribute check for new position" << endl;
            cout << "newPos" << endl;
            cout << newPos << endl;
            #endif

            if( DistTypeHash::computeNewAttrPos(
                nl->Second( nl->Second( result ) ), newPos ) ) {

                switch( nl->ListLength( distType ) ) {
                case 3:
                    distType = nl->ThreeElemList(
                        nl->First( distType ),
                        nl->IntAtom( newPos ),
                        nl->Third( distType ) );
                    break;
                case 4:
                    distType = nl->FourElemList(
                        nl->First( distType ),
                        nl->IntAtom( newPos ),
                        nl->Third( distType ),
                        nl->Fourth( distType ) );
                    break;
                default:
                    distType = nl->TwoElemList(
                        nl->First( distType ),
                        nl->IntAtom( newPos ) );
                }

                drelType = nl->ThreeElemList(
                    nl->First( drelType ),
                    distType,
                    nl->Third( drelType ) );

                #ifdef DRELDEBUG
                cout << "new drelType" << endl;
                cout << nl->ToString( drelType ) << endl;
                #endif
            } else {
                return listutils::typeError( err +
                    ": it is not allowed to project without the "
                    "distribution attribute" );
            }

        }

        // create string to call dmap the call is devided in two parts
        // the argument for the drel is missing and will be filled in the
        // value mapping
        std::string funText1 = "(dmap ";
        std::string funText2 = "\"\" (fun (dmapelem_1 ARRAYFUNARG1) "
            "(projectextend (feed dmapelem_1)" + 
            nl->ToString( attrlist ) + nl->ToString( fun ) + ") ) )";

        #ifdef DRELDEBUG
        cout << "funText1" << endl;
        cout << funText1 << endl;
        cout << "funText2" << endl;
        cout << funText2 << endl;
        #endif

        ListExpr resultType = nl->ThreeElemList(
            listutils::basicSymbol<DFRel>( ),
            relType,
            distType );

        ListExpr append = nl->TwoElemList( 
            nl->TextAtom( funText1 ),
            nl->TextAtom( funText2 ) );

        return nl->ThreeElemList(
            nl->SymbolAtom( Symbols::APPEND( ) ),
            append,
            resultType );
    }

/*
1.1.4 Type Mapping ~drelextendTM~

Expect a d[f]rel with a function list to extend the tuples. This is a 
combination of the operator extend.

*/
    ListExpr drelextendTM( ListExpr args ) {

        #ifdef DRELDEBUG
        cout << "drelextendTM" << endl;
        cout << "args" << endl;
        cout << nl->ToString( args ) << endl;
        #endif

        std::string err = "d[f]rel(X) x funlist expected";

        if( !nl->HasLength( args, 2 ) ) {
            return listutils::typeError( err +
                ": three arguments are expected" );
        }

        if( !DRelHelpers::isListOfTwoElemLists( args ) ) {
            return listutils::typeError( "internal Error" );
        }

        ListExpr drelType, relType, distType, drelValue, darrayType;
        if( !DRelHelpers::isDRelDescr( nl->First( args ), drelType, relType, 
            distType, drelValue, darrayType ) ) {
            return listutils::typeError( err +
                ": first argument is not a d[f]rel" );
        }

        if( !nl->HasMinLength( nl->First( nl->Second( args ) ), 1 )
            || !nl->HasMinLength( nl->Second( nl->Second( args ) ), 1 ) ) {
            return listutils::typeError( err +
                ": error in the function format" );
        }

        ListExpr map = nl->First( nl->Second( args ) );

        ListExpr tempfun;
        ListExpr fun = nl->TheEmptyList( );
        ListExpr temp = nl->Second( nl->Second( args ) );
        while( !nl->IsEmpty( temp ) ) {

            if( !nl->HasLength( nl->First( temp ), 2 ) ) {
                return listutils::typeError( "internal Error" );
            }

            if( !DRelHelpers::replaceDRELFUNARG( 
                nl->Second( nl->First( temp ) ), 
                "TUPLE", tempfun ) ) {
                return listutils::typeError( err +
                    ": error in the function format" );
            }

            if( nl->IsEmpty( fun ) ) {
                fun = nl->OneElemList( nl->TwoElemList(
                    nl->First( nl->First( temp ) ), tempfun ) );
            } else {
                fun = listutils::concat( fun, 
                    nl->OneElemList( nl->TwoElemList(
                    nl->First( nl->First( temp ) ), tempfun ) ) );
            }

            temp = nl->Rest( temp );
        }

        ListExpr result = extrelationalg::ExtendTypeMap(
            nl->TwoElemList(
                nl->TwoElemList(
                    listutils::basicSymbol<Stream<Tuple> >( ),
                    nl->Second( relType ) ),
                map ) );

        #ifdef DRELDEBUG
        cout << "extend tm" << endl;
        cout << nl->ToString( result ) << endl;
        #endif

        // filter TM ok?
        if( !nl->HasLength( result, 2 ) ) {
            return result;
        }
        if( !listutils::isTupleStream( result ) ) {
            return result;
        }

        relType = nl->TwoElemList(
            listutils::basicSymbol<Relation>( ),
            nl->Second( result ) );

        // create string to call dmap the call is devided in two parts
        // the argument for the drel is missing and will be filled in the
        // value mapping
        std::string funText1 = "(dmap ";
        std::string funText2 = "\"\" (fun (dmapelem_1 ARRAYFUNARG1) "
            "(extend (feed dmapelem_1)" + nl->ToString( fun ) + 
            ") ) )";

        #ifdef DRELDEBUG
        cout << "funText1" << endl;
        cout << funText1 << endl;
        cout << "funText2" << endl;
        cout << funText2 << endl;
        #endif

        ListExpr resultType = nl->ThreeElemList(
            listutils::basicSymbol<DFRel>( ),
            relType,
            distType );

        ListExpr append = nl->TwoElemList( 
            nl->TextAtom( funText1 ),
            nl->TextAtom( funText2 ) );

        return nl->ThreeElemList(
            nl->SymbolAtom( Symbols::APPEND( ) ),
            append,
            resultType );
    }

/*
1.1.5 Type Mapping ~headTM~

Expect a d[f]rel and an int value. Type mapping for the head
operator.

*/
    ListExpr headTM( ListExpr args ) {

        #ifdef DRELDEBUG
        cout << "headTM" << endl;
        cout << "args" << endl;
        cout << nl->ToString( args ) << endl;
        #endif

        std::string err = "d[f]rel(X) x int expected";

        if( !nl->HasLength( args, 2 ) ) {
            return listutils::typeError( err +
                ": two arguments are expected" );
        }

        if( !DRelHelpers::isListOfTwoElemLists( args ) ) {
            return listutils::typeError( "internal Error" );
        }

        ListExpr drelType, relType, distType, drelValue, darrayType;
        if( !DRelHelpers::isDRelDescr( nl->First( args ), drelType, relType, 
            distType, drelValue, darrayType ) ) {
            return listutils::typeError( err +
                ": first argument is not a d[f]rel" );
        }

        ListExpr secondType = nl->First( nl->Second( args ) );
        ListExpr secondValue = nl->Second( nl->Second( args ) );

        if( !CcInt::checkType( secondType ) ) {
            return listutils::typeError( err +
                ": second argument is not an integer" );
        }

        // create string to call dmap the call is devided in two parts
        // the argument for the drel is missing and will be filled in the
        // value mapping
        std::string funText1 = "(dmap ";
        std::string funText2 = "\"\" (fun (dmapelem_1 ARRAYFUNARG1) "
            "(head (feed dmapelem_1)" + nl->ToString( secondValue ) + 
            ") ) )";

        #ifdef DRELDEBUG
        cout << "funText1" << endl;
        cout << funText1 << endl;
        cout << "funText2" << endl;
        cout << funText2 << endl;
        #endif

        ListExpr resultType = nl->ThreeElemList(
            listutils::basicSymbol<DFRel>( ),
            relType,
            distType );

        ListExpr append = nl->TwoElemList( 
            nl->TextAtom( funText1 ),
            nl->TextAtom( funText2 ) );

        return nl->ThreeElemList(
            nl->SymbolAtom( Symbols::APPEND( ) ),
            append,
            resultType );
    }

/*
1.1.6 Type Mapping ~renameTM~

Expect a d[f]rel and a symbol. Type mapping for the rename
operator.

*/
    ListExpr renameTM( ListExpr args ) {

        #ifdef DRELDEBUG
        cout << "renameTM" << endl;
        cout << "args" << endl;
        cout << nl->ToString( args ) << endl;
        #endif

        std::string err = "d[f]rel(X) x ar expected";

        if( !nl->HasLength( args, 2 ) ) {
            return listutils::typeError( err +
                ": two arguments are expected" );
        }

        if( !DRelHelpers::isListOfTwoElemLists( args ) ) {
            return listutils::typeError( "internal Error" );
        }

        ListExpr drelType, relType, distType, drelValue, darrayType;
        if( !DRelHelpers::isDRelDescr( nl->First( args ), drelType, relType,
            distType, drelValue, darrayType ) ) {
            return listutils::typeError( err +
                ": first argument is not a d[f]rel" );
        }

        distributionType dType;
        DRelHelpers::drelCheck( nl->First( nl->First( args ) ), dType );
        if( dType == replicated || dType == spatial2d || dType == spatial3d ) {
            return listutils::typeError( err +
                ": replicated or spatial distributed d[f]rel is not allowed" );
        }

        ListExpr secondType = nl->First( nl->Second( args ) );
        ListExpr secondValue = nl->Second( nl->Second( args ) );

        ListExpr result = RenameTypeMap(
            nl->TwoElemList(
                nl->TwoElemList(
                    listutils::basicSymbol<Stream<Tuple> >( ),
                    nl->Second( relType ) ),
                secondType ) );

        // project TM ok?
        if( !nl->HasLength( result, 2 ) ) {
            return result;
        }
        if( !listutils::isTupleStream( result ) ) {
            return result;
        }

        relType = nl->TwoElemList(
            listutils::basicSymbol<Relation>( ),
            nl->Second( result ) );

        // create string to call dmap the call is devided in two parts
        // the argument for the drel is missing and will be filled in the
        // value mapping
        std::string funText1 = "(dmap ";
        std::string funText2 = "\"\" (fun (dmapelem_1 ARRAYFUNARG1) "
            "(rename (feed dmapelem_1)" + 
            nl->ToString( secondValue ) + ") ) )";

        #ifdef DRELDEBUG
        cout << "funText1" << endl;
        cout << funText1 << endl;
        cout << "funText2" << endl;
        cout << funText2 << endl;
        #endif

        ListExpr resultType = nl->ThreeElemList(
            listutils::basicSymbol<DFRel>( ),
            relType,
            distType );

        ListExpr append = nl->TwoElemList( 
            nl->TextAtom( funText1 ),
            nl->TextAtom( funText2 ) );

        return nl->ThreeElemList(
            nl->SymbolAtom( Symbols::APPEND( ) ),
            append,
            resultType );
    }

/*
1.1.7 Type Mapping ~lrdupTM~

Expect a d[f]rel and a symbol. Type mapping for the lrdup
operator.

*/
    ListExpr lrdupTM( ListExpr args ) {

        #ifdef DRELDEBUG
        cout << "lrdupTM" << endl;
        cout << "args" << endl;
        cout << nl->ToString( args ) << endl;
        #endif

        std::string err = "d[f]rel(X) expected";

        if( !nl->HasLength( args, 1 ) ) {
            return listutils::typeError( err +
                ": two arguments are expected" );
        }

        if( !DRelHelpers::isListOfTwoElemLists( args ) ) {
            return listutils::typeError( "internal Error" );
        }

        ListExpr drelType, relType, distType, drelValue, darrayType;
        if( !DRelHelpers::isDRelDescr( nl->First( args ), drelType, relType,
            distType, drelValue, darrayType ) ) {
            return listutils::typeError( err +
                ": first argument is not a d[f]rel" );
        }

        // create string to call dmap the call is devided in two parts
        // the argument for the drel is missing and will be filled in the
        // value mapping
        std::string funText1 = "(dmap ";
        std::string funText2 = "\"\" (fun (dmapelem_1 ARRAYFUNARG1) "
            "(rdup (sort (feed dmapelem_1) ) ) ) )";

        #ifdef DRELDEBUG
        cout << "funText1" << endl;
        cout << funText1 << endl;
        cout << "funText2" << endl;
        cout << funText2 << endl;
        #endif

        ListExpr resultType = nl->ThreeElemList(
            listutils::basicSymbol<DFRel>( ),
            relType,
            distType );

        ListExpr append = nl->TwoElemList( 
            nl->TextAtom( funText1 ),
            nl->TextAtom( funText2 ) );

        return nl->ThreeElemList(
            nl->SymbolAtom( Symbols::APPEND( ) ),
            append,
            resultType );
    }

/*
1.1.8 Type Mapping ~lsortTM~

Expect a d[f]rel and a symbol. Type mapping for the lsort
operator.

*/
    ListExpr lsortTM( ListExpr args ) {

        #ifdef DRELDEBUG
        cout << "lsortTM" << endl;
        cout << "args" << endl;
        cout << nl->ToString( args ) << endl;
        #endif

        std::string err = "d[f]rel(X) expected";

        if( !nl->HasLength( args, 1 ) ) {
            return listutils::typeError( err +
                ": two arguments are expected" );
        }

        if( !DRelHelpers::isListOfTwoElemLists( args ) ) {
            return listutils::typeError( "internal Error" );
        }

        ListExpr drelType, relType, distType, drelValue, darrayType;
        if( !DRelHelpers::isDRelDescr( nl->First( args ), drelType, relType,
            distType, drelValue, darrayType ) ) {
            return listutils::typeError( err +
                ": first argument is not a d[f]rel" );
        }

        // create string to call dmap the call is devided in two parts
        // the argument for the drel is missing and will be filled in the
        // value mapping
        std::string funText1 = "(dmap ";
        std::string funText2 = "\"\" (fun (dmapelem_1 ARRAYFUNARG1) "
            "(sort (feed dmapelem_1) ) ) )";

        #ifdef DRELDEBUG
        cout << "funText1" << endl;
        cout << funText1 << endl;
        cout << "funText2" << endl;
        cout << funText2 << endl;
        #endif

        ListExpr resultType = nl->ThreeElemList(
            listutils::basicSymbol<DFRel>( ),
            relType,
            distType );

        ListExpr append = nl->TwoElemList( 
            nl->TextAtom( funText1 ),
            nl->TextAtom( funText2 ) );

        return nl->ThreeElemList(
            nl->SymbolAtom( Symbols::APPEND( ) ),
            append,
            resultType );
    }

/*
1.1.9 Type Mapping ~drellgroupbyTM~

Expect a d[f]rel, an attribute list to group the tuple and a function list.
Type mapping for the drellgroup operator.

*/
    template<bool global>
    ListExpr drellgroupbyTM( ListExpr args ) {

        #ifdef DRELDEBUG
        cout << "drellgroupbyTM" << endl;
        cout << "args" << endl;
        cout << nl->ToString( args ) << endl;
        #endif

        std::string err = "d[f]rel(X) x attrlist x funlist expected";

        if( !nl->HasLength( args, 3 ) ) {
            return listutils::typeError( err +
                ": three arguments are expected" );
        }

        if( !DRelHelpers::isListOfTwoElemLists( args ) ) {
            return listutils::typeError( "internal Error" );
        }

        ListExpr drelType, relType, distType, drelValue, darrayType;
        if( !DRelHelpers::isDRelDescr( nl->First( args ), drelType, relType,
            distType, drelValue, darrayType ) ) {
            return listutils::typeError( err +
                ": first argument is not a d[f]rel" );
        }

        distributionType dType;
        DRelHelpers::drelCheck( nl->First( nl->First( args ) ), dType );
        if( !global 
         && ( dType == replicated 
           || dType == spatial2d 
           || dType == spatial3d ) ) {
            
            return listutils::typeError( err +
                ": replicated or spatial distributed d[f]rel is not allowed" );
        }

        ListExpr attrlist = nl->First( nl->Second( args ) );

        if( !nl->HasMinLength( nl->First( nl->Third( args ) ), 1 )
            || !nl->HasMinLength( nl->Second( nl->Third( args ) ), 1 ) ) {
            return listutils::typeError( err +
                ": error in the function format" );
        }

        ListExpr map = nl->First( nl->Third( args ) );

        ListExpr tempfun;
        ListExpr fun = nl->TheEmptyList( );
        ListExpr temp = nl->Second( nl->Third( args ) );
        while( !nl->IsEmpty( temp ) ) {

            if( !nl->HasLength( nl->First( temp ), 2 ) ) {
                return listutils::typeError( "internal Error" );
            }

            if( !DRelHelpers::replaceDRELFUNARG(
                nl->Second( nl->First( temp ) ),
                "GROUP", tempfun ) ) {
                return listutils::typeError( err +
                    ": error in the function format" );
            }

            if( nl->IsEmpty( fun ) ) {
                fun = nl->OneElemList( nl->TwoElemList(
                    nl->First( nl->First( temp ) ), tempfun ) );
            } else {
                fun = listutils::concat( fun,
                    nl->OneElemList( nl->TwoElemList(
                        nl->First( nl->First( temp ) ), tempfun ) ) );
            }

            temp = nl->Rest( temp );
        }

        ListExpr result = extrelationalg::GroupByTypeMap(
            nl->ThreeElemList(
                nl->TwoElemList(
                    listutils::basicSymbol<Stream<Tuple> >( ),
                    nl->Second( relType ) ),
                attrlist,
                map ) );

        #ifdef DRELDEBUG
        cout << "groupby tm" << endl;
        cout << nl->ToString( result ) << endl;
        #endif

        // groupby TM ok?
        if( !nl->HasLength( result, 3 ) ) {
            return result;
        }
        if( !listutils::isTupleStream( nl->Third( result ) ) ) {
            return result;
        }

        relType = nl->TwoElemList(
            listutils::basicSymbol<Relation>( ),
            nl->Second( nl->Third( result ) ) );

        // distribution by an attribute?
        if( nl->HasMinLength( distType, 2 ) ) {

            int newPos = nl->IntValue( nl->Second( distType ) ) + 1;
            #ifdef DRELDEBUG
            cout << "distribution by attribute check for new position" << endl;
            cout << "newPos" << endl;
            cout << newPos << endl;
            #endif

            if( nl->HasMinLength( nl->Second( result ), 2 )
             && DistTypeHash::computeNewAttrPos(
                 nl->Rest( nl->Second( result ) ), newPos ) ) {

                switch( nl->ListLength( distType ) ) {
                case 3:
                    distType = nl->ThreeElemList(
                        nl->First( distType ),
                        nl->IntAtom( newPos ),
                        nl->Third( distType ) );
                    break;
                case 4:
                    distType = nl->FourElemList(
                        nl->First( distType ),
                        nl->IntAtom( newPos ),
                        nl->Third( distType ),
                        nl->Fourth( distType ) );
                    break;
                default:
                    distType = nl->TwoElemList(
                        nl->First( distType ),
                        nl->IntAtom( newPos ) );
                }

                #ifdef DRELDEBUG
                cout << "new drelType" << endl;
                cout << nl->ToString( drelType ) << endl;
                #endif
            } else {
                if( !global ) {
                    return listutils::typeError( err +
                        ": it is not allowed to create a group without the "
                        "distribution attribute" );
                }
            }

        }

        // create string to call dmap the call is devided in two parts
        // the argument for the drel is missing and will be filled in the
        // value mapping
        std::string funText1 = "(dmap ";
        std::string funText2 = "\"\" (fun (dmapelem_1 ARRAYFUNARG1) "
            "(groupby (feed dmapelem_1) " + nl->ToString( attrlist ) +
            nl->ToString( fun ) + " ) ) )";

        #ifdef DRELDEBUG
        cout << "funText1" << endl;
        cout << funText1 << endl;
        cout << "funText2" << endl;
        cout << funText2 << endl;
        #endif

        ListExpr resultType = nl->ThreeElemList(
            listutils::basicSymbol<DFRel>( ),
            relType,
            distType );

        ListExpr append = nl->TwoElemList( 
            nl->TextAtom( funText1 ),
            nl->TextAtom( funText2 ) );

        return nl->ThreeElemList(
            nl->SymbolAtom( Symbols::APPEND( ) ),
            append,
            resultType );
    }

    template ListExpr drellgroupbyTM<true>( ListExpr args );
    template ListExpr drellgroupbyTM<false>( ListExpr args );

/*
1.1.10 Type Mapping ~lsortbyTM~

Expect a d[f]rel an attribute list. Type mapping for the lsortby
operator.

*/
    ListExpr lsortbyTM( ListExpr args ) {

        #ifdef DRELDEBUG
        cout << "lsortbyTM" << endl;
        cout << "args" << endl;
        cout << nl->ToString( args ) << endl;
        #endif

        std::string err = "d[f]rel(X) x attrlist expected";

        if( !nl->HasLength( args, 2 ) ) {
            return listutils::typeError( err +
                ": two arguments are expected" );
        }

        if( !DRelHelpers::isListOfTwoElemLists( args ) ) {
            return listutils::typeError( "internal Error" );
        }

        ListExpr drelType, relType, distType, drelValue, darrayType;
        if( !DRelHelpers::isDRelDescr( nl->First( args ), drelType, relType,
            distType, drelValue, darrayType ) ) {
            return listutils::typeError( err +
                ": first argument is not a d[f]rel" );
        }

        ListExpr attrlist = nl->First( nl->Second( args ) );

        // TM sortby
        ListExpr result = extrelationalg::SortByTypeMap(
            nl->TwoElemList(
                nl->TwoElemList(
                    listutils::basicSymbol<Stream<Tuple> >( ),
                    nl->Second( relType ) ),
                attrlist ) );

        // sortby tm okay?
        if( !nl->HasLength( result, 3 ) ) {
            return result;
        }


        // create string to call dmap the call is devided in two parts
        // the argument for the drel is missing and will be filled in the
        // value mapping
        std::string funText1 = "(dmap ";
        std::string funText2 = "\"\" (fun (dmapelem_1 ARRAYFUNARG1) "
            "(sortby (feed dmapelem_1) " + nl->ToString( attrlist ) + 
            " ) ) )";

        #ifdef DRELDEBUG
        cout << "funText1" << endl;
        cout << funText1 << endl;
        cout << "funText2" << endl;
        cout << funText2 << endl;
        #endif

        ListExpr resultType = nl->ThreeElemList(
            listutils::basicSymbol<DFRel>( ),
            relType,
            distType );

        ListExpr append = nl->TwoElemList( 
            nl->TextAtom( funText1 ),
            nl->TextAtom( funText2 ) );

        return nl->ThreeElemList(
            nl->SymbolAtom( Symbols::APPEND( ) ),
            append,
            resultType );
    }

/*
1.1.13 Type Mapping rangeTM

Expect a d[f]array with a btree and a d[f]rel and two values 
to define the range.

*/
    ListExpr rangeTM( ListExpr args ) {

        std::string err = "drel(btree(X)) x drel(rel(X)) x ANY x ANY expected";

        cout << "args" << endl;
        cout << nl->ToString( args ) << endl;

        if( !nl->HasLength( args, 4 ) ) {
            return listutils::typeError( err +
                ": four arguments are expected" );
        }

        if( !DRelHelpers::isListOfTwoElemLists( args ) ) {
            return listutils::typeError( "internal Error" );
        }

        ListExpr drelType, relType, distType, drelValue, darrayType;
        if( !DRelHelpers::isDRelDescr( nl->Second( args ), drelType, relType,
            distType, drelValue, darrayType ) ) {
            return listutils::typeError( err +
                ": second argument is not a d[f]rel" );
        }

        ListExpr darrayBTreeType = nl->First( nl->First( args ) );

        ListExpr range1Type = nl->First( nl->Third( args ) );
        ListExpr range2Type = nl->First( nl->Fourth( args ) );

        if( !DArray::checkType( darrayBTreeType ) ) {
            return listutils::typeError( err +
                ": first argument is not a darray" );
        }

        ListExpr bTreeType = nl->Second( darrayBTreeType );

        ListExpr result = IndexQueryTypeMap<2>(
            nl->FourElemList(
                bTreeType,
                relType,
                range1Type,
                range2Type ) );

        if( !listutils::isTupleStream( result ) ) {
            return result;
        }

        ListExpr range1 = nl->Second( nl->Third( args ) );
        ListExpr range2 = nl->Second( nl->Fourth( args ) );

        // create string to call dmap the call is devided in two parts
        // the argument for the drel is missing and will be filled in the
        // value mapping
        std::string funText1 = "(dmap2 ";
        std::string funText2 = "\"\" (fun (elem1_1 ARRAYFUNARG1) "
            "(elem2_2 ARRAYFUNARG2) "
            "(range elem1_1 elem2_2 " + nl->ToString( range1 ) + " " +
            nl->ToString( range2 ) + " ) ) " + getDRelPortString() +" )";

        #ifdef DRELDEBUG
        cout << "funText1" << endl;
        cout << funText1 << endl;
        cout << "funText2" << endl;
        cout << funText2 << endl;
        #endif

        ListExpr resultType = nl->ThreeElemList(
            listutils::basicSymbol<DFRel>( ),
            relType,
            distType );

        ListExpr append = nl->TwoElemList( 
            nl->TextAtom( funText1 ),
            nl->TextAtom( funText2 ) );

        return nl->ThreeElemList(
            nl->SymbolAtom( Symbols::APPEND( ) ),
            append,
            resultType );
    }

/*
1.1.12 Type Mapping exactmatchTM

Expect a d[f]array with a btree and a d[f]rel and a search
value.

*/
    ListExpr exactmatchTM( ListExpr args ) {

        std::string err = "drel(btree(X)) x drel(rel(X)) x ANY expected";

        if( !nl->HasLength( args, 3 ) ) {
            return listutils::typeError( err +
                ": three arguments are expected" );
        }

        if( !DRelHelpers::isListOfTwoElemLists( args ) ) {
            return listutils::typeError( "internal Error" );
        }

        ListExpr darrayBTreeType = nl->First( nl->First( args ) );
        if( !DArray::checkType( darrayBTreeType ) ) {
            return listutils::typeError( err +
                ": first argument is not a darray" );
        }

        ListExpr drelType, relType, distType, drelValue, darrayType;
        if( !DRelHelpers::isDRelDescr( nl->Second( args ), drelType, relType,
            distType, drelValue, darrayType ) ) {
            return listutils::typeError( err +
                ": second argument is not a d[f]rel" );
        }

        ListExpr searchType = nl->First( nl->Third( args ) );
        ListExpr bTreeType = nl->Second( darrayBTreeType );

        ListExpr result = IndexQueryTypeMap<3>(
            nl->ThreeElemList(
                bTreeType,
                relType,
                searchType ) );

        if( !listutils::isTupleStream( result ) ) {
            return result;
        }

        ListExpr searchValue = nl->Second( nl->Third( args ) );

        // create string to call dmap the call is devided in two parts
        // the argument for the drel is missing and will be filled in the
        // value mapping
        std::string funText1 = "(dmap2 ";
        std::string funText2 = "\"\" (fun (elem1_1 ARRAYFUNARG1) "
            "(elem2_2 ARRAYFUNARG2) "
            "(exactmatch elem1_1 elem2_2 " + 
            nl->ToString( searchValue ) + " ) ) "+ getDRelPortString() + " )";

        #ifdef DRELDEBUG
        cout << "funText1" << endl;
        cout << funText1 << endl;
        cout << "funText2" << endl;
        cout << funText2 << endl;
        #endif

        ListExpr resultType = nl->ThreeElemList(
            listutils::basicSymbol<DFRel>( ),
            relType,
            distType );

        ListExpr append = nl->TwoElemList( 
            nl->TextAtom( funText1 ),
            nl->TextAtom( funText2 ) );

        return nl->ThreeElemList(
            nl->SymbolAtom( Symbols::APPEND( ) ),
            append,
            resultType );
    }

/*
1.1.13 Type Mapping ~windowintersectsTM~

Expect a d[f]rel and an attrtibute list to execute a mapped function on the 
the distributed relation. Type mapping for global simple operators with two 
arguments.

*/
    ListExpr windowintersectsTM( ListExpr args ) { 

        std::string err = "darray[rtree] x d[f]rel(X) x rect expected";

        if( !nl->HasLength( args, 3 ) ) {
            return listutils::typeError( err +
                ": three arguments are expected" );
        }

        if( !DRelHelpers::isListOfTwoElemLists( args ) ) {
            return listutils::typeError( "internal Error" );
        }

        ListExpr darrayBTreeType = nl->First( nl->First( args ) );
        if( !DArray::checkType( darrayBTreeType ) ) {
            return listutils::typeError( err +
                ": first argument is not a darray" );
        }

        ListExpr drelType, relType, distType, drelValue, darrayType;
        if( !DRelHelpers::isDRelDescr( nl->Second( args ), drelType, relType,
            distType, drelValue, darrayType ) ) {
            return listutils::typeError( err +
                ": second argument is not a d[f]rel" );
        }

        ListExpr arg3Type = nl->First( nl->Third( args ) );
        ListExpr arg3Value = nl->Second( nl->Third( args ) );

        if( !Rectangle<2>::checkType( arg3Type ) ) {
            return listutils::typeError( 
                err + ": third argument is rectangle" );
        }

        if( !nl->IsAtom( arg3Type ) ) {
            return listutils::typeError( 
                err + ": third argument is not a saved rectangle" );
        }

        //string tempName = distributed2::algInstance->getTempName( );
        std::string tempName = nl->SymbolValue( arg3Value );

        // Bring rect to the workers
        cout << "bring intersect argument to the workers" << endl;

        ListExpr shareQuery = nl->FourElemList(
            nl->SymbolAtom( "share" ),
            nl->StringAtom( nl->SymbolValue( arg3Value ) ),
            nl->BoolAtom( true ),
            nl->TwoElemList(
                nl->SymbolAtom( "drel2darray" ),
                drelValue ) );

        Word result;
        bool correct, evaluable, defined, isFunction;
        std::string typeString, errorString;
        QueryProcessor::ExecuteQuery( 
            shareQuery, result, typeString, errorString, correct, evaluable, 
            defined, isFunction );

        FText* shareResult = ( FText* )result.addr;
        if( !correct || !evaluable || !defined
         || !shareResult->IsDefined( ) ) {
            return listutils::typeError( 
                "error while bring the argument to the workers" );
        }
        cout << shareResult->GetValue( ) << endl;
        delete shareResult;

        // create string to call dmap the call is devided in two parts
        // the argument for the drel is missing and will be filled in the
        // value mapping
        std::string funText1 = "(dmap2 ";
        std::string funText2 = "\"\" (fun (elem1_1 ARRAYFUNARG1) "
            "(elem2_2 ARRAYFUNARG2) "
            "(windowintersects elem1_1 elem2_2 " + tempName + 
            " ) ) " + getDRelPortString() + " )";

        #ifdef DRELDEBUG
        cout << "funText1" << endl;
        cout << funText1 << endl;
        cout << "funText2" << endl;
        cout << funText2 << endl;
        #endif

        ListExpr resultType = nl->ThreeElemList(
            listutils::basicSymbol<DFRel>( ),
            relType,
            distType );

        ListExpr append = nl->TwoElemList( 
            nl->TextAtom( funText1 ),
            nl->TextAtom( funText2 ) );

        return nl->ThreeElemList(
            nl->SymbolAtom( Symbols::APPEND( ) ),
            append,
            resultType );
    }

/*
1.1.13 Type Mapping inloopjoinTM

Expect two DRels (one with btree and one with a relation) and a search
value.

*/
    ListExpr inloopjoinTM( ListExpr args ) {

        std::string err = "drel(rel(X)) x darray(btree(Y)) x drel(rel(Z)) "
                          "x attr expected";

        if( !nl->HasLength( args, 4 ) ) {
            return listutils::typeError( err +
                ": two arguments are expected" );
        }

        if( !DRelHelpers::isListOfTwoElemLists( args ) ) {
            return listutils::typeError( "internal Error" );
        }

        ListExpr drel1Type, rel1Type, dist1Type, drel1Value, darray1Type;
        if( !DRelHelpers::isDRelDescr( nl->First( args ), drel1Type, rel1Type,
            dist1Type, drel1Value, darray1Type ) ) {
            return listutils::typeError( err +
                ": first argument is not a d[f]rel" );
        }

        ListExpr darrayBTreeType = nl->First( nl->Second( args ) );
        if( !DArray::checkType( darrayBTreeType ) ) {
            return listutils::typeError( err +
                ": second argument is not a darray" );
        }
        //ListExpr btreeType = nl->Second( nl->Second( args ) );

        ListExpr drel2Type, rel2Type, dist2Type, drel2Value, darray2Type;
        if( !DRelHelpers::isDRelDescr( nl->Third( args ), drel2Type, rel2Type,
            dist2Type, drel2Value, darray2Type ) ) {
            return listutils::typeError( err +
                ": third argument is not a d[f]rel" );
        }

        ListExpr arg4Type = nl->First( nl->Fourth( args ) );
        if( !nl->IsAtom( arg4Type )
          || nl->AtomType( arg4Type ) != SymbolType ) {
            return listutils::typeError( err +
                ": fourth argument is not an attribute" );
        }

        // create string to call dmap the call is devided in two parts
        // the argument for the drel is missing and will be filled in the
        // value mapping
        ListExpr arg4Value = nl->Second( nl->Fourth( args ) );
        std::string funText1 = "(dmap3 ";
        std::string funText2 = "\"\" (fun (elem1_1 ARRAYFUNARG1) "
            "(elem2_2 ARRAYFUNARG2) (elem3_3 ARRAYFUNARG3) "
            "(loopjoin (feed elem1_1) (fun (tuple_4 TUPLE) "
            "(exactmatch elem2_2 elem3_3 (attr tuple_4 " +
            nl->ToString( arg4Value ) + ") ) ) ) ) " 
            + getDRelPortString() + " )";

        #ifdef DRELDEBUG
        cout << "funText1" << endl;
        cout << funText1 << endl;
        cout << "funText2" << endl;
        cout << funText2 << endl;
        #endif

        ListExpr attr1List = nl->Second( nl->Second( rel1Type ) );
        ListExpr attr2List = nl->Second( nl->Second( rel2Type ) );
        ListExpr relType = nl->TwoElemList(
            listutils::basicSymbol<Relation>( ),
            nl->TwoElemList(
                listutils::basicSymbol<Tuple>( ),
                ConcatLists( attr1List, attr2List ) ) );

        ListExpr resultType = nl->ThreeElemList(
            listutils::basicSymbol<DFRel>( ),
            relType,
            dist2Type );

        ListExpr append = nl->TwoElemList( 
            nl->TextAtom( funText1 ),
            nl->TextAtom( funText2 ) );

        return nl->ThreeElemList(
            nl->SymbolAtom( Symbols::APPEND( ) ),
            append,
            resultType );
    }

/*
1.2 Value Mapping ~dreldmapVMT~

Uses a d[f]rel and creates a new drel. The d[f]rel is created by calling 
the dmap operators of the Distributed2Algebra. The function is in the 
text arguments of the typemapping.

*/
    template<class R, bool setDType>
    int dreldmapNewVMT( Word* args, Word& result, int message,
        Word& local, Supplier s ) {

        #ifdef DRELDEBUG
        cout << "dreldmapNewVMT" << endl;
        #endif

        int x = qp->GetNoSons( s );

        R* drel = ( R* )args[ 0 ].addr;
        FText* fun1 = ( FText* )args[ x - 2 ].addr;
        FText* fun2 = ( FText* )args[ x - 1 ].addr;

        result = qp->ResultStorage(s);
        DFRel* resultDFRel = ( DFRel* )result.addr;

        if( !drel->IsDefined( ) ) {
            resultDFRel->makeUndefined( );
            return 0;
        }

        // create dmap call with drel pointer
        std::string drelptr = nl->ToString( DRelHelpers::createdrel2darray( 
            qp->GetType( qp->GetSon( s, 0 ) ), drel ) );

        std::string funText = fun1->GetValue( ) + drelptr + fun2->GetValue( );

        #ifdef DRELDEBUG
        cout << "funText" << endl;
        cout << funText << endl;
        #endif

        ListExpr funList;

        if( !nl->ReadFromString( funText, funList ) ) {
            resultDFRel->makeUndefined( );
        }

        bool correct = false;
        bool evaluable = false;
        bool defined = false;
        bool isFunction = false;
        std::string typeString, errorString;
        Word dmapResult;

        if( !QueryProcessor::ExecuteQuery( funList, dmapResult, 
                typeString, errorString,
                correct, evaluable, defined, isFunction ) ) {
            resultDFRel->makeUndefined( );
            return 0;
        }
        
        if( !correct || !evaluable || !defined ) {
            resultDFRel->makeUndefined( );
            return 0;
        }

        DFArray* dfarray = ( DFArray* )dmapResult.addr;
        if( !dfarray->IsDefined( ) ) {
            resultDFRel->makeUndefined( );
            delete dfarray;
            return 0;
        }

        resultDFRel->copyFrom( *dfarray );
        delete dfarray;

        if( setDType ) {
            resultDFRel->setDistType( drel->getDistType( )->copy( ) );
        }

        return 0;
    }

    template int dreldmapNewVMT<DRel, false>( Word* args, Word& result, 
        int message, Word& local, Supplier s );
    template int dreldmapNewVMT<DFRel, false>( Word* args, Word& result, 
        int message, Word& local, Supplier s );

/*
1.3 Value Mapping ~dreldmap2VMT~

Uses a d[f]rel and a d[f]array and and creates a new drel. The d[f]rel 
is created by calling the dmap operators of the Distributed2Algebra. 
The function is in the text arguments of the typemapping.

*/
    template<class R>
    int dreldmapNew2VMT( Word* args, Word& result, int message,
        Word& local, Supplier s ) {

        #ifdef DRELDEBUG
        cout << "dreldmapVMT" << endl;
        #endif

        int x = qp->GetNoSons( s );

        DArray* darray = ( DArray* )args[ 0 ].addr;
        R* drel = ( R* )args[ 1 ].addr;
        FText* fun1 = ( FText* )args[ x - 2 ].addr;
        FText* fun2 = ( FText* )args[ x - 1 ].addr;

        result = qp->ResultStorage(s);
        DFRel* resultDFRel = ( DFRel* )result.addr;

        if( !drel->IsDefined( )
         || !darray->IsDefined( ) ) {
            resultDFRel->makeUndefined( );
            return 0;
        }

        // create dmap call with drel pointer
        std::string darrayptr = nl->ToString( DRelHelpers::createPointerList( 
                qp->GetType( qp->GetSon( s, 0 ) ), darray ) );
        std::string drelptr = nl->ToString( DRelHelpers::createdrel2darray( 
            qp->GetType( qp->GetSon( s, 1 ) ), drel ) );

        std::string funText = fun1->GetValue( ) + darrayptr + drelptr + 
            fun2->GetValue( );

        #ifdef DRELDEBUG
        cout << "funText" << endl;
        cout << funText << endl;
        #endif

        ListExpr funList;
        if( !nl->ReadFromString( funText, funList ) ) {
            resultDFRel->makeUndefined( );
        }

        bool correct = false;
        bool evaluable = false;
        bool defined = false;
        bool isFunction = false;
        std::string typeString, errorString;
        Word dmapResult;
        if( !QueryProcessor::ExecuteQuery( funList, dmapResult, 
                typeString, errorString,
                correct, evaluable, defined, isFunction ) ) {
            resultDFRel->makeUndefined( );
            return 0;
        }
        
        if( !correct || !evaluable || !defined ) {
            resultDFRel->makeUndefined( );
            return 0;
        }

        DFArray* dfarray = ( DFArray* )dmapResult.addr;
        if( !dfarray->IsDefined( ) ) {
            resultDFRel->makeUndefined( );
            delete dfarray;
            return 0;
        }

        resultDFRel->copyFrom( *dfarray );
        delete dfarray;

        resultDFRel->setDistType( drel->getDistType( )->copy( ) );

        return 0;
    }

/*
1.4 Value Mapping ~dreldmap3VMT~

Uses a d[f]rel and a d[f]array and and creates a new drel. The d[f]rel 
is created by calling the dmap operators of the Distributed2Algebra. 
The function is in the text arguments of the typemapping.

*/
    template<class R, class T>
    int dreldmap3VMT( Word* args, Word& result, int message,
        Word& local, Supplier s ) {

        #ifdef DRELDEBUG
        cout << "dreldmap3VMT" << endl;
        #endif

        int x = qp->GetNoSons( s );

        R* drel1 = ( R* )args[ 0 ].addr;
        DArray* darray = ( DArray* )args[ 1 ].addr;
        T* drel2 = ( T* )args[ 2 ].addr;
        FText* fun1 = ( FText* )args[ x - 2 ].addr;
        FText* fun2 = ( FText* )args[ x - 1 ].addr;

        result = qp->ResultStorage(s);
        DFRel* resultDFRel = ( DFRel* )result.addr;

        if( !drel1->IsDefined( )
         || !darray->IsDefined( )
         || !drel2->IsDefined( ) ) {
            resultDFRel->makeUndefined( );
            return 0;
        }

        // create dmap call with drel pointer
        std::string drel1ptr = nl->ToString( DRelHelpers::createdrel2darray( 
            qp->GetType( qp->GetSon( s, 0 ) ), drel1 ) );
        std::string darrayptr = nl->ToString( DRelHelpers::createPointerList( 
            qp->GetType( qp->GetSon( s, 1 ) ), darray ) );
        std::string drel2ptr = nl->ToString( DRelHelpers::createdrel2darray( 
            qp->GetType( qp->GetSon( s, 2 ) ), drel2 ) );

        std::string funText = fun1->GetValue( ) + drel1ptr + darrayptr 
                            + drel2ptr + " " + fun2->GetValue( );

        #ifdef DRELDEBUG
        cout << "funText" << endl;
        cout << funText << endl;
        #endif

        ListExpr funList;
        if( !nl->ReadFromString( funText, funList ) ) {
            resultDFRel->makeUndefined( );
        }

        bool correct = false;
        bool evaluable = false;
        bool defined = false;
        bool isFunction = false;
        std::string typeString, errorString;
        Word dmapResult;
        if( !QueryProcessor::ExecuteQuery( funList, dmapResult, 
                typeString, errorString,
                correct, evaluable, defined, isFunction ) ) {
            resultDFRel->makeUndefined( );
            return 0;
        }
        
        if( !correct || !evaluable || !defined ) {
            resultDFRel->makeUndefined( );
            return 0;
        }

        DFArray* dfarray = ( DFArray* )dmapResult.addr;
        if( !dfarray->IsDefined( ) ) {
            resultDFRel->makeUndefined( );
            delete dfarray;
            return 0;
        }

        resultDFRel->copyFrom( *dfarray );
        delete dfarray;

        resultDFRel->setDistType( drel2->getDistType( )->copy( ) );

        return 0;
    }

/*
1.5 ValueMapping Array for dmap
    
Used by the operators with only a d[f]rel input.

*/
    ValueMapping dreldmapNewVM[ ] = {
        dreldmapNewVMT<DRel, true>,
        dreldmapNewVMT<DFRel, true>
    };

/*
1.6 ValueMapping Array for dmap
    
Used by the operators with a darray and a d[f]rel as input.

*/
    ValueMapping dreldmapNew2VM[ ] = {
        dreldmapNew2VMT<DRel>,
        dreldmapNew2VMT<DFRel>
    };

/*
1.8 ValueMapping Array for dmap
    
Used by the operators with a darray and a d[f]rel as input.

*/
    ValueMapping dreldmapNew3VM[ ] = {
        dreldmap3VMT<DRel, DRel>,
        dreldmap3VMT<DRel, DFRel>,
        dreldmap3VMT<DFRel, DRel>,
        dreldmap3VMT<DFRel, DFRel>
    };

/*
1.8 Selection functions for operators without repartitioning

*/
    int dreldmapSelect( ListExpr args ) {

        return DRel::checkType( nl->First( args ) ) ? 0 : 1;
    }

    int dreldmap2Select( ListExpr args ) {

        return DRel::checkType( nl->Second( args ) ) ? 0 : 1;
    }

    int dreldmap3Select( ListExpr args ) {
        // first and third drels
        int x1 = DRel::checkType( nl->First( args ) ) ? 0 : 2;
        int x2 = DRel::checkType( nl->Third( args ) ) ? 0 : 2;

        return x1 + x2;
    }

/*
1.9 Specification for all operators using dmapVM

1.9.1 Specification of project

Operator specification of the porject operator.

*/
    OperatorSpec projectSpec(
        " drel(X) x attrlist "
        "-> drel(Y) ",
        " _ project[attrlist]",
        "Passed only the listed attributes to a stream. It is not allowed to "
        "create a new d[f]rel without the partion attribute.",
        " query drel1 project[PLZ, Ort]"
    );

/*
1.9.2 Specification of drelfilter

Operator specification of the drelfilter operator.

*/
    OperatorSpec drelfilterSpec(
        " drel(X) x fun "
        "-> drel(X) ",
        " _ drelfilter[fun]",
        "Only tuples, fulfilling a certain condition are passed to the new "
        "d[f]rel",
        " query drel1 drelfilter[.PLZ=99998]"
    );

/*
1.9.3 Specification of drelprojectextend

Operator specification of the drelprojectextend operator.

*/
    OperatorSpec drelprojectextendSpec(
        " drel(X) x attrlist x funlist "
        "-> drel(X) ",
        " _ drelprojectextend[list; funlist]",
        "First projects the drel and then extends the drel with additional "
        "attributes specified by the funlist",
        " query drel1 drelprojectextend[No; Mult5: .No * 5]"
    );

/*
1.9.4 Specification of drelextend

Operator specification of the drelextend operator.

*/
    OperatorSpec drelextendSpec(
        " drel(X) x funlist "
        "-> drel(X) ",
        " _ drelextend[list]",
        "Extends the drel with additional attributes specified by the funlist",
        " query drel1 drelextend[Mult5: .No * 5]"
    );

/*
1.9.5 Specification of head

Operator specification of the head operator.

*/
    OperatorSpec headSpec(
        " drel(X) x int -> drel(X) ",
        " _ head[int]",
        " Reduces each slot to it's first n tuples. ",
        " query drel1 head[5]"
    );

/*
1.9.6 Specification of rename

Operator specification of the rename operator.

*/
    OperatorSpec renameSpec(
        " drel(X) x ar "
        "-> drel(X) ",
        " _ rename[symbol]",
        "Renames all attribute names by adding"
        " them with the postfix passed as parameter. "
        "NOTE: parameter must be of symbol type.",
        " query drel1 rename[A]"
    );

/*
1.9.7 Specification of lrdup

Operator specification of the lrdup operator.

*/
    OperatorSpec lrdupSpec(
        " drel(X) "
        "-> drel(X) ",
        " _ lrdup",
        "Removes duplicates in a d[f]rel. "
        "NOTE: Duplicates are only removed in a local array field "
        "and not in the global d[f]rel.",
        " query drel1 lrdup"
    );

/*
1.9.8 Specification of lsort

Operator specification of the lsort operator.

*/
    OperatorSpec lsortSpec(
        " drel(X) "
        "-> drel(X) ",
        " _ lsort",
        "Sorts a d[f]rel. "
        "NOTE: The operator only sorts the local array fields "
        "and not in global d[f]rel.",
        " query drel1 lsort"
    );
    
/*
1.9.9 Specification of drellgroupby

Operator specification of the drellgroupby operator.

*/
    OperatorSpec drellgroupbySpec(
        " drel(X) x attrlist x funlist "
        "-> drel(X) ",
        " _ drellgroupby[attrlist, funlist]",
        "Groups a d[f]rel according to attributes "
        "ai1, ..., aik and feeds the groups to other "
        "functions. The results of those functions are "
        "appended to the grouping attributes. The empty "
        "list is allowed for the grouping attributes (this "
        "results in a single group with all input tuples)."
        "NOTE: The operator only groups the local array fields "
        "and not in global d[f]rel.",
        " query drel1 drellgroupby[PLZ; Anz: group feed count]"
    );

/*
1.9.10 Specification of lsortby

Operator specification of the lsortby operator.

*/
    OperatorSpec lsortbySpec(
        " drel(X) "
        "-> drel(X) ",
        " _ lsortby[attrlist]",
        "Sorts a d[f]rel by a specific attribute list. "
        "NOTE: The operator only sorts the local array fields "
        "and not in global d[f]rel.",
        " query drel1 lsortby[PLZ]"
    );

/*
1.9.11 Specification of windowintersects

Operator specification of the windowintersects operator.

*/
    OperatorSpec windowintersectsSpec(
        " darray(rtree(X)) x d[f]rel[X]"
        "-> dfrel(X) ",
        " _ _ windowintersects[_]",
        "Computes a windowsintersects of a darray with an rtree, an "
        "d[f]rel and a rectangle.",
        " query darray1 drel1 windowintersects[rectangle]"
    );

/*
1.9.12 Specification of range

*/
    OperatorSpec rangeSpec(
        " darray(btree(X)) x drel(X) x string "
        "-> darray(X) ",
        " _ _ range[_,_]",
        "Uses a distributed btree and a drel to call the range operator.",
        " query drel1_Name drel1 range[\"Berlin\",\"Mannheim\"]"
    );

/*
1.9.13 Specification of exactmatch

*/
    OperatorSpec exactmatchSpec(
        " darray(btree(X)) x drel(X) x string "
        "-> darray(X) ",
        " _ _ exactmatch[_]",
        "Uses a distributed btree and a drel to call the exactmatch operator.",
        " query drel1_Name drel1 exactmatch[\"Berlin\"]"
    );

/*
1.9.14 Specification of inloopjoin

*/
    OperatorSpec inloopjoinSpec(
        " d[f]rel(X) x fun -> dfrel(X) ",
        " _ _ _ inloopjoin[_]",
        "Uses a d[f]rel and computes a loop join with an other d[f]rel and a "
        "btree. The function is static with the exactmatch operator ",
        " query drel1 drel2_A drel2 inloopjoin[B]"
    );

/*
1.10 Operator instance of operators using dmapVM

1.10.1 Operator instance of drelfilter operator

*/
    Operator drelfilterOp(
        "drelfilter",
        drelfilterSpec.getStr( ),
        2,
        dreldmapNewVM,
        dreldmapSelect,
        drelfilterTM
    );

/*
1.10.2 Operator instance of drelproject operator

*/
    Operator projectOp(
        "project",
        projectSpec.getStr( ),
        2,
        dreldmapNewVM,
        dreldmapSelect,
        projectTM
    );

/*
1.10.3 Operator instance of drelprojectextend operator

*/
    Operator drelprojectextendOp(
        "drelprojectextend",
        drelprojectextendSpec.getStr( ),
        2,
        dreldmapNewVM,
        dreldmapSelect,
        drelprojectextendTM
    );

/*
1.10.4 Operator instance of drelextend operator

*/
    Operator drelextendOp(
        "drelextend",
        drelextendSpec.getStr( ),
        2,
        dreldmapNewVM,
        dreldmapSelect,
        drelextendTM
    );

/*
1.10.5 Operator instance of head operator

*/
    Operator headOp(
        "head",
        headSpec.getStr( ),
        2,
        dreldmapNewVM,
        dreldmapSelect,
        headTM
    );
    
/*
1.10.6 Operator instance of drelrename operator

*/
    Operator renameOp(
        "rename",
        renameSpec.getStr( ),
        2,
        dreldmapNewVM,
        dreldmapSelect,
        renameTM
    );
    
/*
1.10.7 Operator instance of lrdup operator

*/
    Operator lrdupOp(
        "lrdup",
        lrdupSpec.getStr( ),
        2,
        dreldmapNewVM,
        dreldmapSelect,
        lrdupTM
    );
    
/*
1.10.8 Operator instance of lsort operator

*/
    Operator lsortOp(
        "lsort",
        lsortSpec.getStr( ),
        2,
        dreldmapNewVM,
        dreldmapSelect,
        lsortTM
    );
    
/*
1.10.9 Operator instance of drellgroupby operator

*/
    Operator drellgroupbyOp(
        "drellgroupby",
        drellgroupbySpec.getStr( ),
        2,
        dreldmapNewVM,
        dreldmapSelect,
        drellgroupbyTM<false>
    );

/*
1.10.10 Operator instance of lsortby operator

*/
    Operator lsortbyOp(
        "lsortby",
        lsortbySpec.getStr( ),
        2,
        dreldmapNewVM,
        dreldmapSelect,
        lsortbyTM
    );

/*
1.10.11 Operator instance of windowintersects operator

*/
    Operator windowintersectsOp(
        "windowintersects",
        windowintersectsSpec.getStr( ),
        2,
        dreldmapNew2VM,
        dreldmap2Select,
        windowintersectsTM
    );

/*
1.10.12 Operator instance of range operator

*/
    Operator rangeOp(
        "range",
        rangeSpec.getStr( ),
        2,
        dreldmapNew2VM,
        dreldmap2Select,
        rangeTM
    );

/*
1.10.13 Operator instance of exactmatch operator

*/
    Operator exactmatchOp(
        "exactmatch",
        exactmatchSpec.getStr( ),
        2,
        dreldmapNew2VM,
        dreldmap2Select,
        exactmatchTM
    );

/*
1.10.14 Operator instance of inloopjoin operator

*/
    Operator inloopjoinOp(
        "inloopjoin",
        inloopjoinSpec.getStr( ),
        4,
        dreldmapNew3VM,
        dreldmap3Select,
        inloopjoinTM
    );

} // end of namespace drel
