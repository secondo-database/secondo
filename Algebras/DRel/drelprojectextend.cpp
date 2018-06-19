
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



1 Implementation of the secondo operator drelrange

*/
#include "NestedList.h"
#include "ListUtils.h"
#include "QueryProcessor.h"
#include "StandardTypes.h"

#include "Algebras/Stream/Stream.h"

#include "DRel.h"

extern NestedList* nl;
extern QueryProcessor* qp;

namespace distributed2 {
    ListExpr dmapTM( ListExpr args );

    template<class A>
    int dmapVMT( Word* args, Word& result, int message,
        Word& local, Supplier s );
}

namespace extrelationalg {
    ListExpr ExtendTypeMap( ListExpr args );
    ListExpr ExtProjectExtendTypeMap( ListExpr args );
}

using namespace distributed2;

namespace drel {

    /*
    1.1 Type Mapping

    Expect a drel with an attribute list and a function list to extend the 
    tuples. This is a combination of the operators project and extend.

    */
    ListExpr drelprojectextendTM( ListExpr args ) {

        std::string err = "d[f]rel(X) x attrlist x funlist expected";

        if( !nl->HasLength( args, 3 ) ) {
            return listutils::typeError( err +
                ": min. 2 arguments required" );
        }

        if( !nl->HasLength( nl->First( args ), 2 ) ) {
            return listutils::typeError( "internal Error" );
        }

        ListExpr drelType = nl->First( nl->First( args ) );
        ListExpr drelName = nl->Second( nl->First( args ) );
        ListExpr relType = nl->Second( drelType );

        ListExpr attributes = nl->TheEmptyList( );
        ListExpr temp = nl->OneElemList( nl->First( nl->Rest( args ) ) );
        while( !nl->IsEmpty( temp ) ) {

            if( !nl->HasLength( nl->First( temp ), 2 ) ) {
                return listutils::typeError( "internal Error" );
            }

            if( !nl->AtomType( nl->First( nl->First( temp ) ) )
                || !nl->AtomType( nl->Second( nl->First( temp ) ) ) ) {

                return listutils::typeError( err +
                    ": argument list not correct. Has to be a list of " +
                    "attributes." );
            }

            if( nl->IsEmpty( attributes ) ) {
                attributes = nl->First( nl->First( temp ) );
            } else {
                attributes = nl->Cons( attributes, 
                    nl->First( nl->First( temp ) ) );
            }

            temp = nl->Rest( temp );
        }

        ListExpr parmFunListMap = nl->TheEmptyList( );
        ListExpr parmFunList = nl->TheEmptyList( );
        temp = nl->Rest( nl->Rest( args ) );
        while( !nl->IsEmpty( temp ) ) {

            if( !nl->HasLength( nl->First( temp ), 2 ) ) {
                return listutils::typeError( "internal Error" );
            }

            if( nl->IsEmpty( parmFunListMap ) ) {
                parmFunListMap = nl->First( nl->First( temp ) );
                parmFunList = nl->Second( nl->First( temp ) );
            } else {
                parmFunListMap = nl->Cons( parmFunListMap, 
                    nl->First( nl->First( temp ) ) );
                parmFunList = nl->Cons( parmFunList, 
                    nl->Second( nl->First( temp ) ) );
            }

            temp = nl->Rest( temp );
        }

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
                ": first argument is not a drel" );
        }
        
        ListExpr resStream = extrelationalg::ExtProjectExtendTypeMap(
            nl->ThreeElemList(
                nl->TwoElemList(
                    listutils::basicSymbol<Stream<Tuple> >( ),
                    nl->Second( relType ) ),
                attributes,
                parmFunListMap ) );

        if( !nl->HasLength( resStream, 3 ) ) {
            return listutils::typeError( err +
                ": error with the projectextend function" );
        }

        // Replace DRELFUNARG1 with TUPLE. This is nessesarry to execute the 
        // query at the workers.
        ListExpr parmNewFunList = nl->TheEmptyList( );
        temp = parmFunList;
        while( !nl->IsEmpty( temp ) ) {
            if( !nl->HasLength( nl->First( temp ), 2 ) ) {
                return listutils::typeError( "internal Error" );
            }
            if( !nl->HasLength( nl->Second( nl->First( temp ) ), 3 ) ) {
                return listutils::typeError( "internal Error" );
            }
            if( !nl->HasLength(
                nl->Second( nl->Second( nl->First( temp ) ) ), 2 ) ) {
                return listutils::typeError( "internal Error" );
            }

            ListExpr parmNewFunListTuple = nl->OneElemList(
                nl->TwoElemList(
                    nl->First( nl->First( temp ) ),
                    nl->ThreeElemList(
                        nl->First( nl->Second( nl->First( temp ) ) ),
                        nl->TwoElemList(
                            nl->First( nl->Second(
                                nl->Second( nl->First( temp ) ) ) ),
                            nl->SymbolAtom( "TUPLE" ) ),
                        nl->Third( nl->Second( nl->First( temp ) ) ) ) ) );
            if( nl->IsEmpty( parmNewFunList ) ) {
                parmNewFunList = parmNewFunListTuple;
            } else {
                parmNewFunList = listutils::concat( parmNewFunList,
                    parmNewFunListTuple );
            }

            temp = nl->Rest( temp );
        }

       

        // create function type to call dmapTM
        ListExpr funList = nl->TwoElemList(
            nl->ThreeElemList(
                nl->SymbolAtom( "map" ),
                relType,
                nl->Third( resStream ) ),   // only result type without append
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
                    attributes,
                    parmNewFunList ) ) );

        // result type of dmap
        ListExpr result = dmapTM(
            nl->ThreeElemList(
                nl->TwoElemList( darrayType, drelName ),
                nl->TwoElemList( 
                    listutils::basicSymbol<CcString>( ), 
                    nl->StringAtom( "" ) ),
                funList ) );

        if( !nl->HasLength( result, 3 ) ) {
            return result;
        }
        ListExpr newRes;
        if( DArray::checkType( nl->Third( result ) ) ) {
            newRes = nl->ThreeElemList(
                listutils::basicSymbol<DRel>( ),
                nl->Second( nl->Third( result ) ),
                nl->Third( drelType ) );  // disttype
        }
        else if( DFArray::checkType( nl->Third( result ) ) ) {
            newRes = nl->ThreeElemList(
                listutils::basicSymbol<DFRel>( ),
                nl->Second( nl->Third( result ) ),
                nl->Third( drelType ) );  // disttype
        }
        else {
            return result;
        }

        ListExpr append = nl->Second( result );

        return nl->ThreeElemList(
            nl->SymbolAtom( Symbols::APPEND( ) ),
            append,
            newRes );
    }

    /*
    1.2 Value Mapping

    Uses a drel and first project the drel and then extends the drel with the 
    choosen attributes defined by a function.
    Result is a new projected and extended drel.

    */
    template<class R, class T>
    int drelprojectextendVMT( Word* args, Word& result, int message,
        Word& local, Supplier s ) {
        
        R* drel = ( R* )args[ 0 ].addr;

        ArgVector argVec = { drel,
            new CcString( "" ),
            args[ 2 ].addr,
            args[ 3 ].addr,
            args[ 4 ].addr,
            args[ 5 ].addr,
            args[ 6 ].addr };
        
        /*ArgVector argVec = { drel,
            new CcString( "" ),
            args[ 1 ].addr,
            args[ 2 ].addr,
            args[ 3 ].addr,
            args[ 4 ].addr,
            args[ 5 ].addr };*/

        dmapVMT<T>( argVec, result, message, local, s );

        R* resultDRel = ( R* )result.addr;
        if( !resultDRel->IsDefined( ) ) {
            return 0;
        }

        resultDRel->setDistType( drel->getDistType( )->copy( ) );

        return 0;
    }

    /*
    1.3 Specification of drelprojectextend

    */
    OperatorSpec drelprojectextendSpec(
        " drel(X) x funlist "
        "-> drel(X) ",
        " _ drelprojectextend[list; funlist]",
        "First projects the drel and then extends the drel with additional "
        "attributes specified by the funlist",
        " query drel1 drelprojectextend[No; Mult5: .No * 5]"
    );

    /*
    1.4 ValueMapping Array of drelprojectextend

    */
    ValueMapping drelprojectextendVM[ ] = {
        drelprojectextendVMT<DRel, DArray>,
        drelprojectextendVMT<DFRel, DFArray>
    };

    /*
    1.5 Selection function of drelprojectextend

    */
    int drelprojectextendSelect( ListExpr args ) {

        return DRel::checkType( nl->First( args ) ) ? 0 : 1;
    }

    /*
    1.6 Operator instance of drelprojectextend operator

    */
    Operator drelprojectextendOp(
        "drelprojectextend",
        drelprojectextendSpec.getStr( ),
        2,
        drelprojectextendVM,
        drelprojectextendSelect,
        drelprojectextendTM
    );

} // end of namespace drel