
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

using namespace distributed2;

namespace drel {

    /*
    1.1 Type Mapping

    Expect a drel with a function to filter the tuples.

    */
    ListExpr drelfilterTM( ListExpr args ) {

        std::string err = "d[f]rel(X) x fun expected";

        if( !nl->HasLength( args, 2 ) ) {
            return listutils::typeError( err +
                ": four arguments are expected" );
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
        ListExpr fun = nl->Second( args );

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


        // create function type to call dmapTM
        ListExpr funList = nl->TwoElemList(
            nl->ThreeElemList(
                nl->SymbolAtom( "map" ),
                relType,
                nl->TwoElemList(
                    listutils::basicSymbol<Stream<Tuple> >( ),
                    nl->Second( relType ) ) ),
            nl->ThreeElemList(
                nl->SymbolAtom( "fun" ),
                nl->TwoElemList(
                    nl->SymbolAtom( "dmapelem1" ),
                    nl->SymbolAtom( "ARRAYFUNARG1" ) ),
                //nl->TwoElemList(
                //   nl->SymbolAtom( "consume" ),
                    nl->ThreeElemList(
                        nl->SymbolAtom( "filter" ),
                        nl->TwoElemList(
                            nl->SymbolAtom( "feed" ),
                            nl->SymbolAtom( "dmapelem1" ) ),
                        nl->ThreeElemList(
                            nl->SymbolAtom( "fun" ),
                            nl->TwoElemList(
                                nl->SymbolAtom( "elem11" ),
                                nl->SymbolAtom( "STREAMELEM" ) ),
                            nl->Third( nl->Second( fun ) ) ) ) ) );
                            //nl->Third( nl->Second( fun ) ) ) ) ) ) );

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
        } else if( DFArray::checkType( nl->Third( result ) ) ) {
            newRes = nl->ThreeElemList(
                listutils::basicSymbol<DFRel>( ),
                nl->Second( nl->Third( result ) ),
                nl->Third( drelType ) );  // disttype
        } else {
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

    Uses a drel and filter all the tuple matching to the filter function.
    Result is a new drel.

    */
    template<class R, class T>
    int drelfilterVMT( Word* args, Word& result, int message,
        Word& local, Supplier s ) {
        
        R* drel = ( R* )args[ 0 ].addr;

        ArgVector argVec = { drel,
            new CcString( "" ),
            args[ 1 ].addr,
            args[ 2 ].addr,
            args[ 3 ].addr,
            args[ 4 ].addr,
            args[ 5 ].addr };

        dmapVMT<T>( argVec, result, message, local, s );

        R* resultDRel = ( R* )result.addr;
        if( !resultDRel->IsDefined( ) ) {
            return 0;
        }

        resultDRel->setDistType( drel->getDistType( )->copy( ) );

        return 0;
    }

    /*
    1.3 Specification of drelfilter

    */
    OperatorSpec drelfilterSpec(
        " drel(X) x fun "
        "-> drel(X) ",
        " _ drelfilter[_]",
        "Only tuples, fulfilling a certain condition are passed to the new "
        "d[f]rel",
        " query drel1 drelfilter[.PLZ=99998]"
    );

    /*
    1.4 ValueMapping Array of drelfilter

    */
    ValueMapping drelfilterVM[ ] = {
        drelfilterVMT<DRel, DArray>,
        drelfilterVMT<DFRel, DFArray>
    };

    /*
    1.5 Selection function of drelfilter

    */
    int drelfilterSelect( ListExpr args ) {

        return DRel::checkType( nl->First( args ) ) ? 0 : 1;
    }

    /*
    1.6 Operator instance of drelfilter operator

    */
    Operator drelfilterOp(
        "drelfilter",
        drelfilterSpec.getStr( ),
        2,
        drelfilterVM,
        drelfilterSelect,
        drelfilterTM
    );

} // end of namespace drel