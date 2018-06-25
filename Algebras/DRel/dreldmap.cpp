
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



1 Implementation of the secondo operators drelfilter, drelproject and 
drelprojectextend.

This operators have the same value mapping witch calls the dmap operator of 
the Distributed2Algebra.

*/
#include "NestedList.h"
#include "ListUtils.h"
#include "QueryProcessor.h"
#include "StandardTypes.h"

#include "Algebras/Stream/Stream.h"
#include "Algebras/Relation-C++/OperatorFilter.h"
#include "Algebras/Relation-C++/OperatorProject.h"

#include "DRelHelpers.h"
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
    ListExpr HeadTypeMap( ListExpr args );
}

using namespace distributed2;

namespace drel {

    /*
    1.1 Type Mapping drelfilterTM

    Expect a drel with a function to filter the tuples. Type mapping for the 
    drelfilter operator.

    */
    ListExpr drelfilterTM( ListExpr args ) {

        std::string err = "d[f]rel(X) x fun expected";

        if( !nl->HasLength( args, 2 ) ) {
            return listutils::typeError( err +
                ": two arguments are expected" );
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
                        drelName ) ), // only dummy for the filter TM
                nl->TwoElemList( map, fun ) ) );

        // filter TM ok?
        if( !listutils::isTupleStream( result ) ) {
            return result;
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

        ListExpr dmapResult = dmapTM(
            nl->ThreeElemList(
                nl->TwoElemList( darrayType, drelName ),
                nl->TwoElemList( 
                    listutils::basicSymbol<CcString>( ), 
                    nl->StringAtom( "" ) ),
                funList ) );

        if( !nl->HasLength( dmapResult, 3 ) ) {
            return dmapResult;
        }

        ListExpr newDRelType;
        if( DArray::checkType( nl->Third( dmapResult ) ) ) {
            newDRelType = listutils::basicSymbol<DRel>( );
        } else if( DFArray::checkType( nl->Third( dmapResult ) ) ) {
            newDRelType = listutils::basicSymbol<DFRel>( );
        } else {
            return dmapResult;
        }

        ListExpr newRes = nl->ThreeElemList(
            newDRelType,
            nl->Second( nl->Third( dmapResult ) ),
            nl->Third( drelType ) );  // disttype

        ListExpr append = nl->Second( dmapResult );

        return nl->ThreeElemList(
            nl->SymbolAtom( Symbols::APPEND( ) ),
            append,
            newRes );
    }

    /*
    1.2 Type Mapping drelprojectTM

    Expect a drel an attribute list. Type mapping for the drelproject 
    operator.

    */
    ListExpr drelprojectTM( ListExpr args ) {

        std::string err = "d[f]rel(X) x attrlist expected";

        if( !nl->HasLength( args, 2 ) ) {
            return listutils::typeError( err +
                ": two arguments are expected" );
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

        ListExpr attrlist = nl->First( nl->Second( args ) );

        ListExpr result = OperatorProject::ProjectTypeMap(
            nl->TwoElemList(
                nl->TwoElemList(
                    listutils::basicSymbol<Stream<Tuple> >( ),
                    nl->Second( relType ) ),
                attrlist ) );

        // project TM ok?
        if( !nl->HasLength( result, 3 ) ) {
            return result;
        }
        if( !listutils::isTupleStream( nl->Third( result ) ) ) {
            return result;
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

        ListExpr dmapResult = dmapTM(
            nl->ThreeElemList(
                nl->TwoElemList( darrayType, drelName ),
                nl->TwoElemList(
                    listutils::basicSymbol<CcString>( ),
                    nl->StringAtom( "" ) ),
                funList ) );

        if( !nl->HasLength( dmapResult, 3 ) ) {
            return dmapResult;
        }

        ListExpr newDRelType;
        if( DArray::checkType( nl->Third( dmapResult ) ) ) {
            newDRelType = listutils::basicSymbol<DRel>( );
        } else if( DFArray::checkType( nl->Third( dmapResult ) ) ) {
            newDRelType = listutils::basicSymbol<DFRel>( );
        } else {
            return dmapResult;
        }

        ListExpr newRes = nl->ThreeElemList(
            newDRelType,
            nl->Second( nl->Third( dmapResult ) ),
            nl->Third( drelType ) );  // disttype

        ListExpr append = nl->Second( dmapResult );

        return nl->ThreeElemList(
            nl->SymbolAtom( Symbols::APPEND( ) ),
            append,
            newRes );
    }

    /*
    1.3 Type Mapping drelprojectextendTM

    Expect a drel with an attribute list and a function list to extend the 
    tuples. This is a combination of the operators project and extend.

    */
    ListExpr drelprojectextendTM( ListExpr args ) {

        std::string err = "d[f]rel(X) x attrlist x funlist expected";

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

        // filter TM ok?
        if( !nl->HasLength( result, 3 ) ) {
            return result;
        }
        if( !listutils::isTupleStream( nl->Third( result ) ) ) {
            return result;
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

        ListExpr dmapResult = dmapTM(
            nl->ThreeElemList(
                nl->TwoElemList( darrayType, drelName ),
                nl->TwoElemList(
                    listutils::basicSymbol<CcString>( ),
                    nl->StringAtom( "" ) ),
                funList ) );

        if( !nl->HasLength( dmapResult, 3 ) ) {
            return dmapResult;
        }

        ListExpr newDRelType;
        if( DArray::checkType( nl->Third( dmapResult ) ) ) {
            newDRelType = listutils::basicSymbol<DRel>( );
        } else if( DFArray::checkType( nl->Third( dmapResult ) ) ) {
            newDRelType = listutils::basicSymbol<DFRel>( );
        } else {
            return dmapResult;
        }

        ListExpr newRes = nl->ThreeElemList(
            newDRelType,
            nl->Second( nl->Third( dmapResult ) ),
            nl->Third( drelType ) );  // disttype

        ListExpr append = nl->Second( dmapResult );

        return nl->ThreeElemList(
            nl->SymbolAtom( Symbols::APPEND( ) ),
            append,
            newRes );
    }

    /*
    1.3 Type Mapping drelextendTM

    Expect a drel with a function list to extend the tuples. This is a 
    combination of the operator extend.

    */
    ListExpr drelextendTM( ListExpr args ) {

        std::string err = "d[f]rel(X) x funlist expected";

        if( !nl->HasLength( args, 2 ) ) {
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

        // filter TM ok?
        if( !nl->HasLength( result, 2 ) ) {
            return result;
        }
        if( !listutils::isTupleStream( result ) ) {
            return result;
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

        ListExpr dmapResult = dmapTM(
            nl->ThreeElemList(
                nl->TwoElemList( darrayType, drelName ),
                nl->TwoElemList(
                    listutils::basicSymbol<CcString>( ),
                    nl->StringAtom( "" ) ),
                funList ) );

        if( !nl->HasLength( dmapResult, 3 ) ) {
            return dmapResult;
        }

        ListExpr newDRelType;
        if( DArray::checkType( nl->Third( dmapResult ) ) ) {
            newDRelType = listutils::basicSymbol<DRel>( );
        } else if( DFArray::checkType( nl->Third( dmapResult ) ) ) {
            newDRelType = listutils::basicSymbol<DFRel>( );
        } else {
            return dmapResult;
        }

        ListExpr newRes = nl->ThreeElemList(
            newDRelType,
            nl->Second( nl->Third( dmapResult ) ),
            nl->Third( drelType ) );  // disttype

        ListExpr append = nl->Second( dmapResult );

        return nl->ThreeElemList(
            nl->SymbolAtom( Symbols::APPEND( ) ),
            append,
            newRes );
    }

    /*
    1.2 Type Mapping drelheadTM

    Expect a drel an int value. Type mapping for the drelhead
    operator.

    */
    ListExpr drelheadTM( ListExpr args ) {

        std::string err = "d[f]rel(X) x attrlist expected";

        if( !nl->HasLength( args, 2 ) ) {
            return listutils::typeError( err +
                ": two arguments are expected" );
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

        ListExpr secondType = nl->First( nl->Second( args ) );
        ListExpr secondValue = nl->Second( nl->Second( args ) );

        ListExpr result = extrelationalg::HeadTypeMap(
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
                    nl->SymbolAtom( "head" ),
                    nl->TwoElemList(
                        nl->SymbolAtom( "feed" ),
                        nl->SymbolAtom( "dmapelem1" ) ),
                    secondValue ) ) );

        ListExpr dmapResult = dmapTM(
            nl->ThreeElemList(
                nl->TwoElemList( darrayType, drelName ),
                nl->TwoElemList(
                    listutils::basicSymbol<CcString>( ),
                    nl->StringAtom( "" ) ),
                funList ) );

        if( !nl->HasLength( dmapResult, 3 ) ) {
            return dmapResult;
        }

        ListExpr newDRelType;
        if( DArray::checkType( nl->Third( dmapResult ) ) ) {
            newDRelType = listutils::basicSymbol<DRel>( );
        } else if( DFArray::checkType( nl->Third( dmapResult ) ) ) {
            newDRelType = listutils::basicSymbol<DFRel>( );
        } else {
            return dmapResult;
        }

        ListExpr newRes = nl->ThreeElemList(
            newDRelType,
            nl->Second( nl->Third( dmapResult ) ),
            nl->Third( drelType ) );  // disttype

        ListExpr append = nl->Second( dmapResult );

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
    template<class R, class T, int parm>
    int dreldmapVMT( Word* args, Word& result, int message,
        Word& local, Supplier s ) {
        
        R* drel = ( R* )args[ 0 ].addr;

        ArgVector argVec = { drel,
            new CcString( "" ),
            args[ 1 + parm ].addr,
            args[ 2 + parm ].addr,
            args[ 3 + parm ].addr,
            args[ 4 + parm ].addr,
            args[ 5 + parm ].addr };

        dmapVMT<T>( argVec, result, message, local, s );

        R* resultDRel = ( R* )result.addr;
        if( !resultDRel->IsDefined( ) ) {
            return 0;
        }

        resultDRel->setDistType( drel->getDistType( )->copy( ) );

        return 0;
    }

    /*
    1.5 ValueMapping Array for dmap
    
    Used by the operators with only a drel input.

    */
    ValueMapping dreldmapVM[ ] = {
        dreldmapVMT<DRel, DArray, 0>,
        dreldmapVMT<DFRel, DFArray, 0>,
        dreldmapVMT<DRel, DArray, 1>,
        dreldmapVMT<DFRel, DFArray, 1>
    };

    /*
    1.6 Selection function for dreldmap


    */
    int dreldmapSelect( ListExpr args ) {

        int parm = nl->ListLength( args ) - 1;

        return DRel::checkType( nl->First( args ) ) ? 0 + parm : 1 + parm;
    }

    /*
    1.7 Specification of drelproject

    */
    OperatorSpec drelprojectSpec(
        " drel(X) x attrlist "
        "-> drel(Y) ",
        " _ drelproject[attrlist]",
        "Passed only the listed attributes to a stream. It is not allowed to "
        "create a new d[f]rel without the partion attribute.",
        " query drel1 drelproject[PLZ, Ort] consume"
    );

    /*
    1.8 Specification of drelfilter

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
    1.9 Specification of drelprojectextend

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
    1.10 Specification of drelextend

    */
    OperatorSpec drelextendSpec(
        " drel(X) x funlist "
        "-> drel(X) ",
        " _ drelextend[list]",
        "Extends the drel with additional attributes specified by the funlist",
        " query drel1 drelextend[Mult5: .No * 5]"
    );

    /*
    1.3 Specification of drelhead

    */
    OperatorSpec drelheadSpec(
        " drel(X) x list "
        "-> drel(X) ",
        " _ drelhead[int]",
        "Passed only the first n tuple of each array field ",
        " query drel1 drelhead[5]"
    );

    /*
    1.11 Operator instance of drelfilter operator

    */
    Operator drelfilterOp(
        "drelfilter",
        drelfilterSpec.getStr( ),
        4,
        dreldmapVM,
        dreldmapSelect,
        drelfilterTM
    );

    /*
    1.12 Operator instance of drelproject operator

    */
    Operator drelprojectOp(
        "drelproject",
        drelprojectSpec.getStr( ),
        4,
        dreldmapVM,
        dreldmapSelect,
        drelprojectTM
    );

    /*
    1.13 Operator instance of drelprojectextend operator

    */
    Operator drelprojectextendOp(
        "drelprojectextend",
        drelprojectextendSpec.getStr( ),
        4,
        dreldmapVM,
        dreldmapSelect,
        drelprojectextendTM
    );

    /*
    1.14 Operator instance of drelextend operator

    */
    Operator drelextendOp(
        "drelextend",
        drelextendSpec.getStr( ),
        4,
        dreldmapVM,
        dreldmapSelect,
        drelextendTM
    );

    /*
    1.6 Operator instance of drelhead operator

    */
    Operator drelheadOp(
        "drelhead",
        drelheadSpec.getStr( ),
        4,
        dreldmapVM,
        dreldmapSelect,
        drelheadTM
    );

} // end of namespace drel