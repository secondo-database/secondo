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

#include "Algebras/Stream/Stream.h"
#include "Algebras/Relation-C++/OperatorFilter.h"
#include "Algebras/Relation-C++/OperatorProject.h"
#include "Algebras/Rectangle/RectangleAlgebra.h"
#include "Algebras/Distributed2/CommandLogger.h"
#include "Algebras/Distributed2/Distributed2Algebra.h"

#include "Algebras/FText/FTextAlgebra.h"

#include "DRelHelpers.h"
#include "DRel.h"

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

        if( DFRel::checkType( drelType ) ) {
            relType = nl->TwoElemList(
                listutils::basicSymbol<frel>( ),
                nl->Second( relType ) );
        }

        // create function type to call dmapTM
        ListExpr funList = nl->TwoElemList(
            nl->ThreeElemList(
                nl->SymbolAtom( "map" ),
                relType,
                result ),
            nl->ThreeElemList(
                nl->SymbolAtom( "fun" ),
                nl->TwoElemList(
                    nl->SymbolAtom( "dmapelem1" ),
                    nl->SymbolAtom( "ARRAYFUNARG1" ) ),
                    nl->ThreeElemList(
                        nl->SymbolAtom( "filter" ),
                        nl->TwoElemList(
                            nl->SymbolAtom( "feed" ),
                            nl->SymbolAtom( "dmapelem1" ) ),
                        fun ) ) );

        #ifdef DRELDEBUG
        cout << "funList" << endl;
        cout << nl->ToString( funList ) << endl;
        #endif

        ListExpr dmapResult = dmapTM(
            nl->ThreeElemList(
                nl->TwoElemList( darrayType, drelValue ),
                nl->TwoElemList( 
                    listutils::basicSymbol<CcString>( ), 
                    nl->StringAtom( "" ) ),
                funList ) );

        #ifdef DRELDEBUG
        cout << "dmapResult" << endl;
        cout << nl->ToString( dmapResult ) << endl;
        #endif

        if( !nl->HasLength( dmapResult, 3 ) ) {
            return dmapResult;
        }

        ListExpr newDRel;
        if( !DArray::checkType( nl->Third( dmapResult ) ) ) {
            newDRel = listutils::basicSymbol<DRel>( );
        }
        else if( !DFArray::checkType( nl->Third( dmapResult ) ) ) {
            newDRel = listutils::basicSymbol<DFRel>( );
        }
        else {
            return dmapResult;
        }

        ListExpr newRes = nl->ThreeElemList(
            newDRel,
            nl->Second( nl->Third( dmapResult ) ),
            nl->Third( drelType ) );  // disttype

        ListExpr append = nl->Second( dmapResult );

        return nl->ThreeElemList(
            nl->SymbolAtom( Symbols::APPEND( ) ),
            append,
            newRes );
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

                drelType = nl->ThreeElemList(
                    nl->First( drelType ),
                    distType,
                    nl->Third( drelType ) );
                
                #ifdef DRELDEBUG
                cout << "new drelType" << endl;
                cout << nl->ToString( drelType ) << endl;
                #endif
            }
            else {
                return listutils::typeError( err +
                    ": it is not allowed to project without the "
                    "distribution attribute" );
            }

        }

        if( DFRel::checkType( drelType ) ) {
            relType = nl->TwoElemList(
                listutils::basicSymbol<frel>( ),
                nl->Second( relType ) );
        }

        // create function type to call dmapTM
        ListExpr funList = nl->TwoElemList(
            nl->ThreeElemList(
                nl->SymbolAtom( "map" ),
                relType,
                nl->Third( result ) ),
            nl->ThreeElemList(
                nl->SymbolAtom( "fun" ),
                nl->TwoElemList(
                    nl->SymbolAtom( "dmapelem1" ),
                    nl->SymbolAtom( "ARRAYFUNARG1" ) ),
                nl->ThreeElemList(
                    nl->SymbolAtom( "project" ),
                    nl->TwoElemList(
                        nl->SymbolAtom( "feed" ),
                        nl->SymbolAtom( "dmapelem1" ) ),
                    attrlist ) ) );

        #ifdef DRELDEBUG
        cout << "funList" << endl;
        cout << nl->ToString( funList ) << endl;
        #endif

        ListExpr dmapResult = dmapTM(
            nl->ThreeElemList(
                nl->TwoElemList( darrayType, drelValue ),
                nl->TwoElemList(
                    listutils::basicSymbol<CcString>( ),
                    nl->StringAtom( "" ) ),
                funList ) );

        #ifdef DRELDEBUG
        cout << "dmapResult" << endl;
        cout << nl->ToString( dmapResult ) << endl;
        #endif

        if( !nl->HasLength( dmapResult, 3 ) ) {
            return dmapResult;
        }

        ListExpr newDRel;
        if( !DArray::checkType( nl->Third( dmapResult ) ) ) {
            newDRel = listutils::basicSymbol<DRel>( );
        }
        else if( !DFArray::checkType( nl->Third( dmapResult ) ) ) {
            newDRel = listutils::basicSymbol<DFRel>( );
        }
        else {
            return dmapResult;
        }

        ListExpr newRes = nl->ThreeElemList(
            newDRel,
            nl->Second( nl->Third( dmapResult ) ),
            distType );

        ListExpr append = nl->Second( dmapResult );

        return nl->ThreeElemList(
            nl->SymbolAtom( Symbols::APPEND( ) ),
            append,
            newRes );
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

        if( DFRel::checkType( drelType ) ) {
            relType = nl->TwoElemList(
                listutils::basicSymbol<frel>( ),
                nl->Second( relType ) );
        }

        // create function type to call dmapTM
        ListExpr funList = nl->TwoElemList(
            nl->ThreeElemList(
                nl->SymbolAtom( "map" ),
                relType,
                nl->Third( result ) ),
            nl->ThreeElemList(
                nl->SymbolAtom( "fun" ),
                nl->TwoElemList(
                    nl->SymbolAtom( "dmapelem1" ),
                    nl->SymbolAtom( "ARRAYFUNARG1" ) ),
                nl->FourElemList(
                    nl->SymbolAtom( "projectextend" ),
                    nl->TwoElemList(
                        nl->SymbolAtom( "feed" ),
                        nl->SymbolAtom( "dmapelem1" ) ),
                    attrlist,
                    fun ) ) );

        #ifdef DRELDEBUG
        cout << "funList" << endl;
        cout << nl->ToString( funList ) << endl;
        #endif

        ListExpr dmapResult = dmapTM(
            nl->ThreeElemList(
                nl->TwoElemList( darrayType, drelValue ),
                nl->TwoElemList(
                    listutils::basicSymbol<CcString>( ),
                    nl->StringAtom( "" ) ),
                funList ) );

        #ifdef DRELDEBUG
        cout << "dmapResult" << endl;
        cout << nl->ToString( dmapResult ) << endl;
        #endif

        if( !nl->HasLength( dmapResult, 3 ) ) {
            return dmapResult;
        }

        ListExpr newDRel;
        if( !DArray::checkType( nl->Third( dmapResult ) ) ) {
            newDRel = listutils::basicSymbol<DRel>( );
        }
        else if( !DFArray::checkType( nl->Third( dmapResult ) ) ) {
            newDRel = listutils::basicSymbol<DFRel>( );
        }
        else {
            return dmapResult;
        }

        ListExpr newRes = nl->ThreeElemList(
            newDRel,
            nl->Second( nl->Third( dmapResult ) ),
            distType );

        ListExpr append = nl->Second( dmapResult );

        return nl->ThreeElemList(
            nl->SymbolAtom( Symbols::APPEND( ) ),
            append,
            newRes );
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

        if( DFRel::checkType( drelType ) ) {
            relType = nl->TwoElemList(
                listutils::basicSymbol<frel>( ),
                nl->Second( relType ) );
        }

        // create function type to call dmapTM
        ListExpr funList = nl->TwoElemList(
            nl->ThreeElemList(
                nl->SymbolAtom( "map" ),
                relType,
                result ),
            nl->ThreeElemList(
                nl->SymbolAtom( "fun" ),
                nl->TwoElemList(
                    nl->SymbolAtom( "dmapelem1" ),
                    nl->SymbolAtom( "ARRAYFUNARG1" ) ),
                nl->ThreeElemList(
                    nl->SymbolAtom( "extend" ),
                    nl->TwoElemList(
                        nl->SymbolAtom( "feed" ),
                        nl->SymbolAtom( "dmapelem1" ) ),
                    fun ) ) );

        #ifdef DRELDEBUG
        cout << "funList" << endl;
        cout << nl->ToString( funList ) << endl;
        #endif

        ListExpr dmapResult = dmapTM(
            nl->ThreeElemList(
                nl->TwoElemList( darrayType, drelValue ),
                nl->TwoElemList(
                    listutils::basicSymbol<CcString>( ),
                    nl->StringAtom( "" ) ),
                funList ) );

        #ifdef DRELDEBUG
        cout << "dmapResult" << endl;
        cout << nl->ToString( dmapResult ) << endl;
        #endif

        if( !nl->HasLength( dmapResult, 3 ) ) {
            return dmapResult;
        }

        ListExpr newDRel;
        if( !DArray::checkType( nl->Third( dmapResult ) ) ) {
            newDRel = listutils::basicSymbol<DRel>( );
        }
        else if( !DFArray::checkType( nl->Third( dmapResult ) ) ) {
            newDRel = listutils::basicSymbol<DFRel>( );
        }
        else {
            return dmapResult;
        }

        ListExpr newRes = nl->ThreeElemList(
            newDRel,
            nl->Second( nl->Third( dmapResult ) ),
            distType );

        ListExpr append = nl->Second( dmapResult );

        return nl->ThreeElemList(
            nl->SymbolAtom( Symbols::APPEND( ) ),
            append,
            newRes );
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

        if( DFRel::checkType( drelType ) ) {
            relType = nl->TwoElemList(
                listutils::basicSymbol<frel>( ),
                nl->Second( relType ) );
        }

        // create function type to call dmapTM
        ListExpr funList = nl->TwoElemList(
            nl->ThreeElemList(
                nl->SymbolAtom( "map" ),
                relType,
                nl->TwoElemList(
                    listutils::basicSymbol<Stream<Tuple>>( ),
                    nl->Second( relType ) ) ),
            nl->ThreeElemList(
                nl->SymbolAtom( "fun" ),
                nl->TwoElemList(
                    nl->SymbolAtom( "dmapelem1" ),
                    nl->SymbolAtom( "ARRAYFUNARG1" ) ),
                nl->ThreeElemList(
                    nl->SymbolAtom( "head" ),
                    nl->TwoElemList(
                        nl->SymbolAtom( "feed" ),
                        nl->SymbolAtom( "dmapelem1" ) ),
                    secondValue ) ) );

        #ifdef DRELDEBUG
        cout << "funList" << endl;
        cout << nl->ToString( funList ) << endl;
        #endif

        ListExpr dmapResult = dmapTM(
            nl->ThreeElemList(
                nl->TwoElemList( darrayType, drelValue ),
                nl->TwoElemList(
                    listutils::basicSymbol<CcString>( ),
                    nl->StringAtom( "" ) ),
                funList ) );

        #ifdef DRELDEBUG
        cout << "dmapTM head" << endl;
        cout << nl->ToString( dmapResult ) << endl;
        #endif

        if( !nl->HasLength( dmapResult, 3 ) ) {
            return dmapResult;
        }

        ListExpr newDRel;
        if( !DArray::checkType( nl->Third( dmapResult ) ) ) {
            newDRel = listutils::basicSymbol<DRel>( );
        }
        else if( !DFArray::checkType( nl->Third( dmapResult ) ) ) {
            newDRel = listutils::basicSymbol<DFRel>( );
        }
        else {
            return dmapResult;
        }

        ListExpr newRes = nl->ThreeElemList(
            newDRel,
            nl->Second( nl->Third( dmapResult ) ),
            distType );

        ListExpr append = nl->Second( dmapResult );

        return nl->ThreeElemList(
            nl->SymbolAtom( Symbols::APPEND( ) ),
            append,
            newRes );
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

        #ifdef DRELDEBUG
        cout << "rename tm" << endl;
        cout << nl->ToString( result ) << endl;
        #endif

        if( DFRel::checkType( drelType ) ) {
            relType = nl->TwoElemList(
                listutils::basicSymbol<frel>( ),
                nl->Second( relType ) );
        }

        // create function type to call dmapTM
        ListExpr funList = nl->TwoElemList(
            nl->ThreeElemList(
                nl->SymbolAtom( "map" ),
                relType,
                result ),
            nl->ThreeElemList(
                nl->SymbolAtom( "fun" ),
                nl->TwoElemList(
                    nl->SymbolAtom( "dmapelem1" ),
                    nl->SymbolAtom( "ARRAYFUNARG1" ) ),
                nl->ThreeElemList(
                    nl->SymbolAtom( "rename" ),
                    nl->TwoElemList(
                        nl->SymbolAtom( "feed" ),
                        nl->SymbolAtom( "dmapelem1" ) ),
                    secondValue ) ) );

        #ifdef DRELDEBUG
        cout << "funList" << endl;
        cout << nl->ToString( funList ) << endl;
        #endif

        ListExpr dmapResult = dmapTM(
            nl->ThreeElemList(
                nl->TwoElemList( darrayType, drelValue ),
                //nl->TwoElemList( darrayType, nl->SymbolAtom( "dummy" ) ),
                nl->TwoElemList(
                    listutils::basicSymbol<CcString>( ),
                    nl->StringAtom( "" ) ),
                funList ) );

        #ifdef DRELDEBUG
        cout << "dmapTM rename" << endl;
        cout << nl->ToString( dmapResult ) << endl;
        #endif

        if( !nl->HasLength( dmapResult, 3 ) ) {
            return dmapResult;
        }

        ListExpr newDRel;
        if( !DArray::checkType( nl->Third( dmapResult ) ) ) {
            newDRel = listutils::basicSymbol<DRel>( );
        }
        else if( !DFArray::checkType( nl->Third( dmapResult ) ) ) {
            newDRel = listutils::basicSymbol<DFRel>( );
        }
        else {
            return dmapResult;
        }

        ListExpr newRes = nl->ThreeElemList(
            newDRel,
            nl->Second( nl->Third( dmapResult ) ),
            distType );

        ListExpr append = nl->Second( dmapResult );

        return nl->ThreeElemList(
            nl->SymbolAtom( Symbols::APPEND( ) ),
            append,
            newRes );
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

        if( DFRel::checkType( drelType ) ) {
            relType = nl->TwoElemList(
                listutils::basicSymbol<frel>( ),
                nl->Second( relType ) );
        }

        // create function type to call dmapTM
        ListExpr funList = nl->TwoElemList(
            nl->ThreeElemList(
                nl->SymbolAtom( "map" ),
                relType,
                nl->TwoElemList(
                    listutils::basicSymbol<Stream<Tuple>>( ),
                    nl->Second( relType ) ) ),
            nl->ThreeElemList(
                nl->SymbolAtom( "fun" ),
                nl->TwoElemList(
                    nl->SymbolAtom( "dmapelem1" ),
                    nl->SymbolAtom( "ARRAYFUNARG1" ) ),
                nl->TwoElemList(
                    nl->SymbolAtom( "rdup" ),
                    nl->TwoElemList(
                        nl->SymbolAtom( "feed" ),
                        nl->SymbolAtom( "dmapelem1" ) ) ) ) );

        #ifdef DRELDEBUG
        cout << "funList" << endl;
        cout << nl->ToString( funList ) << endl;
        #endif

        ListExpr dmapResult = dmapTM(
            nl->ThreeElemList(
                nl->TwoElemList( darrayType, drelValue ),
                nl->TwoElemList(
                    listutils::basicSymbol<CcString>( ),
                    nl->StringAtom( "" ) ),
                funList ) );

        #ifdef DRELDEBUG
        cout << "dmapTM lrdup" << endl;
        cout << nl->ToString( dmapResult ) << endl;
        #endif

        if( !nl->HasLength( dmapResult, 3 ) ) {
            return dmapResult;
        }

        ListExpr newDRel;
        if( !DArray::checkType( nl->Third( dmapResult ) ) ) {
            newDRel = listutils::basicSymbol<DRel>( );
        }
        else if( !DFArray::checkType( nl->Third( dmapResult ) ) ) {
            newDRel = listutils::basicSymbol<DFRel>( );
        }
        else {
            return dmapResult;
        }

        ListExpr newRes = nl->ThreeElemList(
            newDRel,
            nl->Second( nl->Third( dmapResult ) ),
            distType );

        ListExpr append = nl->Second( dmapResult );

        return nl->ThreeElemList(
            nl->SymbolAtom( Symbols::APPEND( ) ),
            append,
            newRes );
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

        if( DFRel::checkType( drelType ) ) {
            relType = nl->TwoElemList(
                listutils::basicSymbol<frel>( ),
                nl->Second( relType ) );
        }

        // create function type to call dmapTM
        ListExpr funList = nl->TwoElemList(
            nl->ThreeElemList(
                nl->SymbolAtom( "map" ),
                relType,
                nl->TwoElemList(
                    listutils::basicSymbol<Stream<Tuple>>( ),
                    nl->Second( relType ) ) ),
            nl->ThreeElemList(
                nl->SymbolAtom( "fun" ),
                nl->TwoElemList(
                    nl->SymbolAtom( "dmapelem1" ),
                    nl->SymbolAtom( "ARRAYFUNARG1" ) ),
                nl->TwoElemList(
                    nl->SymbolAtom( "sort" ),
                    nl->TwoElemList(
                        nl->SymbolAtom( "feed" ),
                        nl->SymbolAtom( "dmapelem1" ) ) ) ) );

        #ifdef DRELDEBUG
        cout << "funList" << endl;
        cout << nl->ToString( funList ) << endl;
        #endif

        ListExpr dmapResult = dmapTM(
            nl->ThreeElemList(
                nl->TwoElemList( darrayType, drelValue ),
                nl->TwoElemList(
                    listutils::basicSymbol<CcString>( ),
                    nl->StringAtom( "" ) ),
                funList ) );

        #ifdef DRELDEBUG
        cout << "dmapTM lsort" << endl;
        cout << nl->ToString( dmapResult ) << endl;
        #endif

        if( !nl->HasLength( dmapResult, 3 ) ) {
            return dmapResult;
        }

        ListExpr newDRel;
        if( !DArray::checkType( nl->Third( dmapResult ) ) ) {
            newDRel = listutils::basicSymbol<DRel>( );
        }
        else if( !DFArray::checkType( nl->Third( dmapResult ) ) ) {
            newDRel = listutils::basicSymbol<DFRel>( );
        }
        else {
            return dmapResult;
        }

        ListExpr newRes = nl->ThreeElemList(
            newDRel,
            nl->Second( nl->Third( dmapResult ) ),
            distType );

        ListExpr append = nl->Second( dmapResult );

        return nl->ThreeElemList(
            nl->SymbolAtom( Symbols::APPEND( ) ),
            append,
            newRes );
    }

/*
1.1.9 Type Mapping ~drellgroupbyTM~

Expect a d[f]rel, an attribute list to group the tuple and a function list.
Type mapping for the drellgroup operator.

*/
    ListExpr drellgroupbyTM( ListExpr args, bool global ) {

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

                drelType = nl->ThreeElemList(
                    nl->First( drelType ),
                    distType,
                    nl->Third( drelType ) );

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

        if( DFRel::checkType( drelType ) ) {
            relType = nl->TwoElemList(
                listutils::basicSymbol<frel>( ),
                nl->Second( relType ) );
        }

        // create function type to call dmapTM
        ListExpr funList = nl->TwoElemList(
            nl->ThreeElemList(
                nl->SymbolAtom( "map" ),
                relType,
                nl->Third( result ) ),
            nl->ThreeElemList(
                nl->SymbolAtom( "fun" ),
                nl->TwoElemList(
                    nl->SymbolAtom( "dmapelem1" ),
                    nl->SymbolAtom( "ARRAYFUNARG1" ) ),
                nl->FourElemList(
                    nl->SymbolAtom( "groupby" ),
                    nl->TwoElemList(
                        nl->SymbolAtom( "feed" ),
                        nl->SymbolAtom( "dmapelem1" ) ),
                    attrlist,
                    fun ) ) );

        #ifdef DRELDEBUG
        cout << "funList" << endl;
        cout << nl->ToString( funList ) << endl;
        #endif

        ListExpr dmapResult = dmapTM(
            nl->ThreeElemList(
                nl->TwoElemList( darrayType, drelValue ),
                nl->TwoElemList(
                    listutils::basicSymbol<CcString>( ),
                    nl->StringAtom( "" ) ),
                funList ) );

        #ifdef DRELDEBUG
        cout << "dmapResult" << endl;
        cout << nl->ToString( dmapResult ) << endl;
        #endif

        if( !nl->HasLength( dmapResult, 3 ) ) {
            return dmapResult;
        }
        
        ListExpr newDRel;
        if( !DArray::checkType( nl->Third( dmapResult ) ) ) {
            newDRel = listutils::basicSymbol<DRel>( );
        }
        else if( !DFArray::checkType( nl->Third( dmapResult ) ) ) {
            newDRel = listutils::basicSymbol<DFRel>( );
        }
        else {
            return dmapResult;
        }

        ListExpr newRes = nl->ThreeElemList(
            newDRel,
            nl->Second( nl->Third( dmapResult ) ),
            distType );

        ListExpr append = nl->Second( dmapResult );

        return nl->ThreeElemList(
            nl->SymbolAtom( Symbols::APPEND( ) ),
            append,
            newRes );
    }

    ListExpr drellgroupbyTM( ListExpr args ) {
        return drellgroupbyTM( args, false );
    }

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

        ListExpr result = extrelationalg::SortByTypeMap(
            nl->TwoElemList(
                nl->TwoElemList(
                    listutils::basicSymbol<Stream<Tuple> >( ),
                    nl->Second( relType ) ),
                attrlist ) );

        #ifdef DRELDEBUG
        cout << "sortby tm" << endl;
        cout << nl->ToString( result ) << endl;
        #endif

        // sortby TM ok?
        if( !nl->HasLength( result, 3 ) ) {
            return result;
        }
        if( !listutils::isTupleStream( nl->Third( result ) ) ) {
            return result;
        }

        if( DFRel::checkType( drelType ) ) {
            relType = nl->TwoElemList(
                listutils::basicSymbol<frel>( ),
                nl->Second( relType ) );
        }

        // create function type to call dmapTM
        ListExpr funList = nl->TwoElemList(
            nl->ThreeElemList(
                nl->SymbolAtom( "map" ),
                relType,
                nl->Third( result ) ),
            nl->ThreeElemList(
                nl->SymbolAtom( "fun" ),
                nl->TwoElemList(
                    nl->SymbolAtom( "dmapelem1" ),
                    nl->SymbolAtom( "ARRAYFUNARG1" ) ),
                nl->ThreeElemList(
                    nl->SymbolAtom( "sortby" ),
                    nl->TwoElemList(
                        nl->SymbolAtom( "feed" ),
                        nl->SymbolAtom( "dmapelem1" ) ),
                    attrlist ) ) );

        #ifdef DRELDEBUG
        cout << "funList" << endl;
        cout << nl->ToString( funList ) << endl;
        #endif

        ListExpr dmapResult = dmapTM(
            nl->ThreeElemList(
                nl->TwoElemList( darrayType, drelValue ),
                nl->TwoElemList(
                    listutils::basicSymbol<CcString>( ),
                    nl->StringAtom( "" ) ),
                funList ) );

        #ifdef DRELDEBUG
        cout << "dmapResult" << endl;
        cout << nl->ToString( dmapResult ) << endl;
        #endif

        if( !nl->HasLength( dmapResult, 3 ) ) {
            return dmapResult;
        }

        ListExpr newDRel;
        if( !DArray::checkType( nl->Third( dmapResult ) ) ) {
            newDRel = listutils::basicSymbol<DRel>( );
        }
        else if( !DFArray::checkType( nl->Third( dmapResult ) ) ) {
            newDRel = listutils::basicSymbol<DFRel>( );
        }
        else {
            return dmapResult;
        }

        ListExpr newRes = nl->ThreeElemList(
            newDRel,
            nl->Second( nl->Third( dmapResult ) ),
            distType );

        ListExpr append = nl->Second( dmapResult );

        return nl->ThreeElemList(
            nl->SymbolAtom( Symbols::APPEND( ) ),
            append,
            newRes );
    }

/*
1.1.13 Type Mapping rangeTM

Expect two DRels (one with btree and one with a relation) and two values 
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

        ListExpr darrayBTreeType = nl->First( nl->First( args ) );
        ListExpr darrayBTreeValue = nl->Second( nl->First( args ) );
        ListExpr drelType = nl->First( nl->Second( args ) );
        ListExpr drelValue = nl->Second( nl->Second( args ) );

        ListExpr range1Type = nl->First( nl->Third( args ) );
        ListExpr range2Type = nl->First( nl->Fourth( args ) );

        ListExpr range1 = nl->Second( nl->Third( args ) );
        ListExpr range2 = nl->Second( nl->Fourth( args ) );

        ListExpr darrayType;
        if( !DRelHelpers::drelCheck( 
            drelType, darrayType ) ) {

            return listutils::typeError(
                err + ": second argument is not a d[f]rel" );
        }

        if( !DArray::checkType( darrayBTreeType ) ) {
            return listutils::typeError( err +
                ": first argument is not a darray" );
        }

        ListExpr bTreeType = nl->Second( darrayBTreeType );
        ListExpr relType = nl->Second( drelType );

        ListExpr result = IndexQueryTypeMap<2>(
            nl->FourElemList(
                bTreeType,
                relType,
                range1Type,
                range2Type ) );

        if( !listutils::isTupleStream( result ) ) {
            return result;
        }

        if( DFRel::checkType( drelType ) ) {
            relType = nl->TwoElemList(
                listutils::basicSymbol<frel>( ),
                nl->Second( relType ) );
        }

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
                    nl->FiveElemList(
                        nl->SymbolAtom( "range" ),
                        nl->SymbolAtom( "elem11" ),
                        nl->SymbolAtom( "elem22" ),
                        range1,
                        range2 ) ) ) );

        if( DFRel::checkType( drelType ) ) {
            relType = nl->TwoElemList(
                listutils::basicSymbol<frel>( ),
                nl->Second( relType ) );
        }

        // result type of dmap2
        ListExpr dmapResult = dmapXTMT<2>(
            nl->FiveElemList(
                nl->TwoElemList( darrayBTreeType, darrayBTreeValue ),
                nl->TwoElemList( darrayType, drelValue ),
                nl->TwoElemList( 
                    listutils::basicSymbol<CcString>( ), 
                    nl->StringAtom( "" ) ),
                funType,
                nl->TwoElemList( 
                    listutils::basicSymbol<CcInt>( ),
                    nl->IntAtom( 1238 ) ) ) );

        if( !nl->HasLength( dmapResult, 3 ) ) {
            return dmapResult;
        }
        
        ListExpr newDRel;
        if( !DArray::checkType( nl->Third( dmapResult ) ) ) {
            newDRel = listutils::basicSymbol<DRel>( );
        }
        else if( !DFArray::checkType( nl->Third( dmapResult ) ) ) {
            newDRel = listutils::basicSymbol<DFRel>( );
        }
        else {
            return dmapResult;
        }

        ListExpr resultType = nl->ThreeElemList(
            newDRel,
            nl->Second( nl->Third( dmapResult ) ),
            nl->Third( drelType ) );

        ListExpr append = nl->ThreeElemList(
                nl->IntAtom( 1238 ),
                nl->First( nl->Second( dmapResult ) ),
                nl->Second( nl->Second( dmapResult ) ) );

        return nl->ThreeElemList( 
            nl->SymbolAtom( Symbols::APPEND( ) ),
            append,
            resultType );
    }

/*
1.1.12 Type Mapping exactmatchTM

Expect two DRels (one with btree and one with a relation) and a search
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
        ListExpr darrayBTreeValue = nl->Second( nl->First( args ) );
        ListExpr drelType = nl->First( nl->Second( args ) );
        ListExpr drelName = nl->Second( nl->Second( args ) );

        ListExpr searchType = nl->First( nl->Third( args ) );
        ListExpr searchValue = nl->Second( nl->Third( args ) );

        ListExpr darrayType;
        if( !DRelHelpers::drelCheck( 
            drelType, darrayType ) ) {

            return listutils::typeError(
                err + ": second argument is not a d[f]rel" );
        }

        ListExpr bTreeType = nl->Second( darrayBTreeType );
        ListExpr relType = nl->Second( drelType );

        ListExpr result = IndexQueryTypeMap<3>(
            nl->ThreeElemList(
                bTreeType,
                relType,
                searchType ) );

        if( !listutils::isTupleStream( result ) ) {
            return result;
        }

        if( DFRel::checkType( drelType ) ) {
            relType = nl->TwoElemList(
                listutils::basicSymbol<frel>( ),
                nl->Second( relType ) );
        }

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
        ListExpr dmapResult = dmapXTMT<2>(
            nl->FiveElemList(
                nl->TwoElemList( darrayBTreeType, darrayBTreeValue ),
                nl->TwoElemList( darrayType, drelName ),
                nl->TwoElemList( listutils::basicSymbol<CcString>( ),
                    nl->StringAtom( "" ) ),
                funType,
                nl->TwoElemList( 
                    listutils::basicSymbol<CcInt>( ),
                    nl->IntAtom( 1238 ) ) ) );

        if( !nl->HasLength( dmapResult, 3 ) ) {
            return dmapResult;
        }

        ListExpr newDRel;
        if( !DArray::checkType( nl->Third( dmapResult ) ) ) {
            newDRel = listutils::basicSymbol<DRel>( );
        }
        else if( !DFArray::checkType( nl->Third( dmapResult ) ) ) {
            newDRel = listutils::basicSymbol<DFRel>( );
        }
        else {
            return dmapResult;
        }

        ListExpr newRes = nl->ThreeElemList(
            newDRel,
            nl->Second( nl->Third( dmapResult ) ),
            nl->Third( drelType ) );

        ListExpr append = nl->FourElemList(
            nl->StringAtom( "" ),
            nl->IntAtom( 1238 ),
            nl->First( nl->Second( dmapResult ) ),
            nl->Second( nl->Second( dmapResult ) ) );

        return nl->ThreeElemList(
            nl->SymbolAtom( Symbols::APPEND( ) ),
            append,
            newRes );
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
                ": four arguments are expected" );
        }

        if( !DRelHelpers::isListOfTwoElemLists( args ) ) {
            return listutils::typeError( "internal Error" );
        }

        ListExpr arg1Type = nl->First( nl->First( args ) );
        ListExpr arg2Type = nl->First( nl->Second( args ) );
        ListExpr arg3Type = nl->First( nl->Third( args ) );
        //ListExpr arg1Value = nl->Second( nl->First( args ) );
        ListExpr arg2Value = nl->Second( nl->Second( args ) );
        ListExpr arg3Value = nl->Second( nl->Third( args ) );

        if( !DArray::checkType( arg1Type ) ) {
            return listutils::typeError( 
                err + ": first argument is not a darray" );
        }

        if( !listutils::isRTreeDescription( nl->Second( arg1Type ) ) ) {
            return listutils::typeError( 
                err + ": darray is not a rtree" );
        }

        ListExpr darrayType;
        if( !DRelHelpers::drelCheck( arg2Type, darrayType ) ) {
            return listutils::typeError( 
                err + ": second argument is not a d[f]rel" );
        }

        if( !Rectangle<2>::checkType( arg3Type ) ) {
            return listutils::typeError( 
                err + ": third argument is rectangle" );
        }

        if( !nl->IsAtom( arg3Value ) 
         || nl->AtomType( arg3Value ) != SymbolType ) {
            return listutils::typeError( 
                err + ": rectangle is not in catalog" );
        }

        //string tempName = distributed2::algInstance->getTempName( );
        string tempName = nl->SymbolValue( arg3Value );

        ListExpr map = nl->FourElemList(
            nl->SymbolAtom( "map" ),
            nl->Second( arg1Type ),
            nl->Second( darrayType ),
            nl->TwoElemList(
                    listutils::basicSymbol<Stream<Tuple>>( ),
                    nl->Second( nl->Second( darrayType ) ) ) );
        ListExpr fun = nl->FourElemList(
            nl->SymbolAtom( "fun" ),
            nl->TwoElemList(
                nl->SymbolAtom( "elem1_1" ),
                nl->SymbolAtom( "ARRAYFUNARG1" ) ),
            nl->TwoElemList(
                nl->SymbolAtom( "elem2_2" ),
                nl->SymbolAtom( "ARRAYFUNARG2" ) ),
            nl->FourElemList(
                nl->SymbolAtom( "windowintersects" ),
                nl->SymbolAtom( "elem1_1" ),
                nl->SymbolAtom( "elem2_2" ),
                arg3Value ) );

        // call dmapTM
        ListExpr dmapResult = dmapXTMT<2>(
            nl->FiveElemList(
                nl->First( args ),
                nl->TwoElemList( darrayType, arg2Value ),
                nl->TwoElemList( 
                    listutils::basicSymbol<CcString>( ),
                    nl->StringAtom( "" ) ),
                nl->TwoElemList( map, fun ),
                nl->TwoElemList( 
                    listutils::basicSymbol<CcInt>( ),
                    nl->IntAtom( 1238 ) ) ) );

        if( !nl->HasLength( dmapResult, 3 ) ) {
            return dmapResult;
        }

        // dmapTM ok
        ListExpr resultType = nl->ThreeElemList(
            listutils::basicSymbol<DFRel>( ),
            nl->Second( nl->Third( dmapResult ) ),
            nl->Third( arg2Type ) );

        // Bring rect to the workers
        cout << "bring intersect argument to the workers" << endl;

        ListExpr shareQuery = nl->FourElemList(
            nl->SymbolAtom( "share" ),
            nl->StringAtom( nl->SymbolValue( arg3Value ) ),
            nl->BoolAtom( true ),
            nl->TwoElemList(
                nl->SymbolAtom( "drelconvert" ),
                arg2Value ) );

        Word result;
        bool correct, evaluable, defined, isFunction;
        string typeString, errorString;
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

        ListExpr append = nl->FourElemList(
            nl->StringAtom( "" ),
            nl->IntAtom( 1238 ),
            nl->First( nl->Second( dmapResult ) ),
            nl->Second( nl->Second( dmapResult ) ) );

        return nl->ThreeElemList( 
            nl->SymbolAtom( Symbols::APPEND( ) ),
            append,
            resultType );
    }

/*
1.2 Value Mapping ~dreldmapVMT~

Uses a d[f]rel and creates a new drel. The d[f]rel is created by calling 
the dmap value mapping of the Distributed2Algebra.

*/
    template<class R, class T>
    int dreldmapVMT( Word* args, Word& result, int message,
        Word& local, Supplier s ) {

        #ifdef DRELDEBUG
        cout << "dreldmapVMT" << endl;
        #endif

        int parmNum = qp->GetNoSons( s );
        
        R* drel = ( R* )args[ 0 ].addr;
        CcString* name = new CcString( "" );

        ArgVector argVec = {
            drel,
            name,
            args[ 0 ].addr,     // ignored by dmapVMT
            args[ parmNum -3 ].addr,
            args[ parmNum -2 ].addr,
            args[ parmNum -1 ].addr };

        dmapVMT<T>( argVec, result, message, local, s );

        delete name;

        DFRel* resultDRel = ( DFRel* )result.addr;
        if( !resultDRel->IsDefined( ) ) {
            return 0;
        }

        resultDRel->setDistType( drel->getDistType( )->copy( ) );

        return 0;
    }

/*
1.3 Value Mapping ~dreldmap2VMT~

Uses a d[f]rel and a d[f]array and creates a new drel. The d[f]rel is created 
by calling the dmap2 value mapping of the Distributed2Algebra.

*/
    template<class R, class T>
    int dreldmap2VMT( Word* args, Word& result, int message,
        Word& local, Supplier s ) {

        #ifdef DRELDEBUG
        cout << "dreldmap2VMT" << endl;
        #endif

        int parmNum = qp->GetNoSons( s );

        DFRel* resultDRel;
        
        R* drel = ( R* )args[ 1 ].addr;

        ArgVector argVec = {
            args[ 0 ].addr,
            drel,
            args[ parmNum - 4 ].addr,   // name
            args[ 0 ].addr,             // ingnored
            args[ parmNum - 3 ].addr,   // port
            args[ parmNum - 2 ].addr,   // stream?
            args[ parmNum - 1 ].addr }; // function
        
        dmapXVM( argVec, result, message, local, s );

        resultDRel = ( DFRel* )result.addr;
        if( !resultDRel->IsDefined( ) ) {
            return 0;
        }

        resultDRel->setDistType( drel->getDistType( )->copy( ) );

        return 0;
    }

/*
1.4 Value Mapping ~rangeVMT~

Uses a d[f]rel and a d[f]array and creates a new drel. The d[f]rel is created 
by calling the dmap2 value mapping of the Distributed2Algebra.

*/
    template<class R, class T>
    int rangeVMT( Word* args, Word& result, int message,
        Word& local, Supplier s ) {

        #ifdef DRELDEBUG
        cout << "rangeVMT" << endl;
        #endif

        int parmNum = qp->GetNoSons( s );

        DFRel* resultDRel;
        
        CcString* name = new CcString( "" );
        
        R* drel = ( R* )args[ 1 ].addr;

        ArgVector argVec = {
            args[ 0 ].addr,
            drel,
            name,
            args[ 0 ].addr,             // ingnored
            args[ parmNum - 3 ].addr,   // port
            args[ parmNum - 2 ].addr,   // stream?
            args[ parmNum - 1 ].addr }; // function
        
        dmapXVM( argVec, result, message, local, s );

        delete name;

        resultDRel = ( DFRel* )result.addr;
        if( !resultDRel->IsDefined( ) ) {
            return 0;
        }

        resultDRel->setDistType( drel->getDistType( )->copy( ) );

        return 0;
    }

/*
1.5 ValueMapping Array for dmap
    
Used by the operators with only a d[f]rel input.

*/
    ValueMapping dreldmapVM[ ] = {
        dreldmapVMT<DRel, DArray>,
        dreldmapVMT<DFRel, DFArray>
    };

/*
1.6 ValueMapping Array for dmap
    
Used by the operators with a darray and a d[f]rel as input.

*/
    ValueMapping dreldmap2VM[ ] = {
        dreldmap2VMT<DRel, DArray>,
        dreldmap2VMT<DFRel, DFArray>
    };

/*
1.7 ValueMapping Array for dmap
    
Used by the operators with a darray and a d[f]rel as input.

*/
    ValueMapping rangeVM[ ] = {
        rangeVMT<DRel, DArray>,
        rangeVMT<DFRel, DFArray>
    };

/*
1.8 Selection function for dreldmap

Used to select the right position of the parameters. It is necessary, 
because the dmap-Operator ignores the second parameter. So so parameters 
must be moved to the right position for the dmap value mapping.

*/
    int dreldmapSelect( ListExpr args ) {

        return DRel::checkType( nl->First( args ) ) ? 0 : 1;
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
        " query drel1 project[PLZ, Ort] consume"
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
        " drel(X) x list "
        "-> drel(X) ",
        " _ head[int]",
        "Passed only the first n tuple of each array field ",
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
        " query drel1_Name drel1 drelexactmatch[\"Berlin\"]"
    );

/*
1.10 Operator instance of operators using dmapVM

1.10.1 Operator instance of drelfilter operator

*/
    Operator drelfilterOp(
        "drelfilter",
        drelfilterSpec.getStr( ),
        2,
        dreldmapVM,
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
        dreldmapVM,
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
        dreldmapVM,
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
        dreldmapVM,
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
        dreldmapVM,
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
        dreldmapVM,
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
        dreldmapVM,
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
        dreldmapVM,
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
        dreldmapVM,
        dreldmapSelect,
        drellgroupbyTM
    );

/*
1.10.10 Operator instance of lsortby operator

*/
    Operator lsortbyOp(
        "lsortby",
        lsortbySpec.getStr( ),
        2,
        dreldmapVM,
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
        dreldmap2VM,
        dreldmapSelect,
        windowintersectsTM
    );

/*
1.10.12 Operator instance of range operator

*/
    Operator rangeOp(
        "range",
        rangeSpec.getStr( ),
        2,
        rangeVM,
        dreldmapSelect,
        rangeTM
    );

/*
1.10.13 Operator instance of exactmatch operator

*/
    Operator exactmatchOp(
        "exactmatch",
        exactmatchSpec.getStr( ),
        2,
        dreldmap2VM,
        dreldmapSelect,
        exactmatchTM
    );

} // end of namespace drel