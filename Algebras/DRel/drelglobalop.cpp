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


1 Implementation of the secondo operators sort, sortby, 
drelgroupby and rdup

*/
//#define DRELDEBUG

#include "NestedList.h"
#include "ListUtils.h"
#include "QueryProcessor.h"
#include "StandardTypes.h"
#include "SecParser.h"

#include "Algebras/FText/FTextAlgebra.h"

#include "DRelHelpers.h"
#include "DRel.h"
#include "Partitioner.hpp"
#include "BoundaryCalculator.hpp"

extern NestedList* nl;
extern QueryProcessor* qp;

using namespace distributed2;

namespace distributed2 {

    extern Distributed2Algebra* algInstance;

    template<class A>
    int dmapVMT( Word* args, Word& result, int message,
        Word& local, Supplier s );

    template<class R>
    int areduceVMT(Word* args, Word& result, int message,
        Word& local, Supplier s );
};

namespace drel {

    ListExpr lsortTM( ListExpr args );
    ListExpr drellsortbyTM( ListExpr args );
    ListExpr lrdupTM( ListExpr args );
    ListExpr drellgroupbyTM( ListExpr args, bool global );

    template<class R, class T>
    int dreldmapVMT( Word* args, Word& result, int message,
        Word& local, Supplier s );

    class dmapFunctionMapper {

        public:

            enum dmapLocalMapper { lsort, lsortby, lrdup, lgroupby };

            static ListExpr callDMapTM( ListExpr args, dmapLocalMapper i ) {

                switch ( i ) {
                    case lsort: return lsortTM( args );
                    case lsortby: return drellsortbyTM( args );
                    case lrdup: return lrdupTM( args );
                    case lgroupby: return drellgroupbyTM( args, true );
                }

                return nl->TheEmptyList( );
            }

            static std::string getFun( dmapLocalMapper i ) {

                switch ( i ) {
                    case lsort: return "( fun( elem_1 AREDUCEARG1 )(sort ( "
                        "feed elem_1 ) ) )";
                    case lrdup: return "( fun( elem_1 AREDUCEARG1 )(rdup ( "
                        "feed elem_1 ) ) )";
                    default: return "";
                }

                return "";
            }

            static std::string getFun( ListExpr list, dmapLocalMapper i ) {

                switch ( i ) {
                    case lsortby: return "( fun( elem_1 AREDUCEARG1 )"
                        "(sortby ( feed elem_1 ) " + nl->ToString( list ) + 
                        " ) )";
                    default: return "";
                }

                return "";
            }
            static std::string getFun( 
                ListExpr list1, ListExpr list2, dmapLocalMapper i ) {

                switch ( i ) {
                    case lgroupby: return "( fun( elem_1 AREDUCEARG1 )"
                        "(groupby ( feed elem_1 ) " + nl->ToString( list1 ) + 
                        nl->ToString( list2 ) + " ) )";
                    default: return "";
                }

                return "";
            }
    };


/*
1.1 Type Mappings for all global operators with one d[f]rel

1.1.1 Type Mapping ~drelglobalOp2TM~

Expect a d[f]rel and an attrtibute list to execute a mapped function on the 
the distributed relation. Type mapping for global simple operators with two 
arguments.

*/
    template<dmapFunctionMapper::dmapLocalMapper i>
    ListExpr drelglobalOp2TM( ListExpr args ) {

        std::string err = "d[f]rel(X) x attrlist expected";

        ListExpr dreldmapRes = dmapFunctionMapper::callDMapTM( 
                        args, i );

        if( !nl->HasLength( dreldmapRes, 3 ) ) {
            return dreldmapRes;
        }

        ListExpr arg1Type = nl->First( nl->First( args ) );
        ListExpr relType = nl->Second( arg1Type );
        ListExpr distType = nl->Third( arg1Type );
        ListExpr arg2Type = nl->First( nl->Second( args ) );

        std::string attrName = nl->SymbolValue( 
            nl->First( arg2Type ) );
        ListExpr attrList = nl->Second( nl->Second( relType ) );

        ListExpr attrType;
        int pos = listutils::findAttribute( attrList, attrName, attrType );
        if( pos == 0 ) {
            return listutils::typeError(
                err + ": attr name " + attrName + " not found" );
        }

        // Compare the first attribute with the current partion of the drel
        // If the attribute match with the distributtion attribute only lsortby
        // will be necessary.
        if( nl->HasMinLength( distType, 2 ) ) {

            distributionType type;
            if( !getTypeByNum( ( int )
                    ( nl->IntValue( nl->First( distType ) ) ), type ) ) {
                        
                return listutils::typeError(
                    err + ": internal error" );
            }

            if( type == range ) {
                int attrNum = ( int )
                    ( nl->IntValue( nl->Second( distType ) ) );

                if( attrNum == pos - 1 ) { 
                    return dreldmapRes;
                }
            }
        }

        ListExpr resultType = nl->ThreeElemList(
            listutils::basicSymbol<DFRel>( ),
            nl->Second( arg1Type ),
            nl->FourElemList(
                    nl->IntAtom( range ),
                    nl->IntAtom( pos - 1 ),
                    nl->IntAtom( rand( ) ),
                    nl->TwoElemList(
                        nl->SymbolAtom( Vector::BasicType( ) ),
                        attrType ) ) );

        std::string fun =  dmapFunctionMapper::getFun( arg2Type, i );

        if( fun == "" ) {
            return listutils::typeError(
                    err + ": internal error" );
        }

        ListExpr appendList = nl->ThreeElemList(
            nl->StringAtom( attrName ),
            nl->IntAtom( pos - 1 ),
            nl->TextAtom( fun ) );

        return nl->ThreeElemList( 
            nl->SymbolAtom( Symbols::APPEND( ) ),
            appendList,
            resultType );
    }

/*
1.1.2 Type Mapping ~drelglobalOpTM~

Expect only a d[f]rel as argument. Execute a mapped function on the d[f]rel. 
Used by global operators without any argumants expect the d[f]rel.

*/
    template<dmapFunctionMapper::dmapLocalMapper i>
    ListExpr drelglobalOpTM( ListExpr args ) {

        std::string err = "d[f]rel(X) expected";

        ListExpr dreldmapRes = dmapFunctionMapper::callDMapTM( 
                        args, i );

        if( !nl->HasLength( dreldmapRes, 3 ) ) {
            return dreldmapRes;
        }

        ListExpr arg1Type = nl->First( nl->First( args ) );
        ListExpr relType = nl->Second( arg1Type );
        ListExpr distType = nl->Third( arg1Type );

        ListExpr attrList = nl->Second( nl->Second( relType ) );
        ListExpr attrName = nl->First( nl->First( attrList ) );
        ListExpr attrType = nl->Second( nl->First( attrList ) );

        // Compare the first attribute with the current partion of the drel
        // If the attribute match with the distributtion attribute only lsortby
        // will be necessary.
        if( nl->HasMinLength( distType, 2 ) ) {

            distributionType type;
            if( !getTypeByNum( ( int )
                    ( nl->IntValue( nl->First( distType ) ) ), type ) ) {

                return listutils::typeError(
                    err + ": internal error" );
            }

            if( type == range ) {
                int attrNum = ( int )
                    ( nl->IntValue( nl->Second( distType ) ) );

                if( attrNum == 0 ) { 
                    return dreldmapRes;
                }
            }
        }

        ListExpr resultType = nl->ThreeElemList(
            listutils::basicSymbol<DFRel>( ),
            nl->Second( arg1Type ),
            nl->FourElemList(
                    nl->IntAtom( range ),
                    nl->IntAtom( 0 ),
                    nl->IntAtom( rand( ) ),
                    nl->TwoElemList(
                        nl->SymbolAtom( Vector::BasicType( ) ),
                        attrType ) ) );

        std::string fun =  dmapFunctionMapper::getFun( i );

        if( fun == "" ) {
            return listutils::typeError(
                    err + ": internal error" );
        }

        ListExpr appendList = nl->ThreeElemList(
            nl->StringAtom( nl->SymbolValue( attrName ) ),
            nl->IntAtom( 0 ),
            nl->TextAtom( fun ) );

        return nl->ThreeElemList( 
            nl->SymbolAtom( Symbols::APPEND( ) ),
            appendList,
            resultType );
    }

/*
1.1.3 Type Mapping ~drelglobalOp3TM~

Expect a d[f]rel and an attrtibute list to execute a mapped function on the 
the distributed relation. Type mapping for global simple operators with two 
arguments.

*/
    template<dmapFunctionMapper::dmapLocalMapper i>
    ListExpr drelglobalOp3TM( ListExpr args ) { 

        std::string err = "d[f]rel(X) x attrlist x fun expected";

        ListExpr dreldmapRes = dmapFunctionMapper::callDMapTM( 
                        args, i );

        if( !nl->HasLength( dreldmapRes, 3 ) ) {
            return dreldmapRes;
        }

        ListExpr arg1Type = nl->First( nl->First( args ) );
        ListExpr arg2Type = nl->First( nl->Second( args ) );
        ListExpr arg3Value = nl->Second( nl->Third( args ) );
        ListExpr relType = nl->Second( arg1Type );
        ListExpr distType = nl->Third( arg1Type );

        std::string attrName = nl->SymbolValue( 
            nl->First( arg2Type ) );
        ListExpr attrList = nl->Second( nl->Second( relType ) );

        ListExpr attrType;
        int pos = listutils::findAttribute( attrList, attrName, attrType );
        if( pos == 0 ) {
            return listutils::typeError(
                err + ": attr name " + attrName + " not found" );
        }

        // Compare the first attribute with the current partion of the drel
        // If the attribute match with the distributtion attribute only lsortby
        // will be necessary.
        if( nl->HasMinLength( distType, 2 ) ) {

            distributionType type;
            if( !getTypeByNum( ( int )
                    ( nl->IntValue( nl->First( distType ) ) ), type ) ) {

                return listutils::typeError(
                    err + ": internal error" );
            }

            if( type == range ) {
                int attrNum = ( int )
                    ( nl->IntValue( nl->Second( distType ) ) );

                if( attrNum == pos - 1 ) { 
                    return dreldmapRes;
                }
            }
        }

        ListExpr resultType = nl->ThreeElemList(
            listutils::basicSymbol<DFRel>( ),
            nl->Second( nl->Third( dreldmapRes ) ),
            nl->FourElemList(
                    nl->IntAtom( range ),
                    nl->IntAtom( pos - 1 ),
                    nl->IntAtom( rand( ) ),
                    nl->TwoElemList(
                        nl->SymbolAtom( Vector::BasicType( ) ),
                        attrType ) ) );

        std::string fun =  dmapFunctionMapper::getFun( arg2Type,
            arg3Value, i );

        if( fun == "" ) {
            return listutils::typeError(
                err + ": internal error" );
        }

        ListExpr appendList = nl->ThreeElemList(
            nl->StringAtom( attrName ),
            nl->IntAtom( pos - 1 ),
            nl->First( nl->Second( dreldmapRes ) ) );

        return nl->ThreeElemList( 
            nl->SymbolAtom( Symbols::APPEND( ) ),
            appendList,
            resultType );
    }

/*
1.2 Value Mapping ~dreldmapVMT~

Uses a d[f]rel and creates a new drel. The d[f]rel is created by 
repartitioning the d[f]rel and execute a function on the d[f]rel.

*/
    template<class R, class T>
    int drelgobalOpVMT( Word* args, Word& result, int message,
        Word& local, Supplier s ) {

        int parmNum = qp->GetNoSons( s );

        ListExpr boundaryType = nl->Fourth( nl->Third( qp->GetType( s ) ) );
        ListExpr drelType = qp->GetType( qp->GetSon( s, 0 ) );
        R* drel = ( R* )args[ 0 ].addr;
        int key = nl->IntValue( nl->Third( nl->Third( qp->GetType( s ) ) ) );
        int pos = ( ( CcInt* )args[ parmNum - 2 ].addr )->GetValue( );
        string attrName = 
            ( ( CcString* )args[ parmNum - 3 ].addr )->GetValue( );

        collection::Collection* boundary;

        BoundaryCalculator<R>* calc = new BoundaryCalculator<R>( 
                attrName, boundaryType, drel, drelType, 1238 );

        if( !calc->countDRel( ) ){
            cout << "error while determining the relation size" 
                    << endl;
            result = qp->ResultStorage( s );
            ( ( DFRel* )result.addr )->makeUndefined( );
            return 0;
        }

        if( !calc->computeBoundary( ) ){
            cout << "error while computing the boundaries" << endl;
            result = qp->ResultStorage( s );
            ( ( DFRel* )result.addr )->makeUndefined( );
            return 0;
        }

        boundary = calc->getBoundary( )->Clone( );

        Partitioner<R, T>* parti = new Partitioner<R, T>( attrName, 
            boundaryType, drel, drelType, boundary->Clone( ), 1238 );

        if( !parti->repartition2DFMatrix( ) ) {
            cout << "repartition failed!!" << endl;
            result = qp->ResultStorage( s );
            ( ( DFRel* )result.addr )->makeUndefined( );
            return 0;
        }

        DFMatrix* matrix = parti->getDFMatrix( );

        if( !matrix || !matrix->IsDefined( ) ) {
            cout << "repartition failed!!" << endl;
            result = qp->ResultStorage( s );
            ( ( DFRel* )result.addr )->makeUndefined( );
            return 0;
        }

        ListExpr areduceQuery = nl->FiveElemList(
            nl->SymbolAtom( "areduce" ),
            nl->TwoElemList(
                parti->getMatrixType( ),
                nl->TwoElemList(
                    nl->SymbolAtom( "ptr" ),
                    listutils::getPtrList( matrix ) ) ),
            nl->StringAtom( "" ),
            nl->ThreeElemList(
                nl->SymbolAtom( "fun" ),
                nl->TwoElemList(
                    nl->SymbolAtom( "elem_1" ),
                    nl->SymbolAtom( "AREDUCEARG1" ) ),
                nl->TwoElemList(
                    nl->SymbolAtom( "feed" ),
                    nl->SymbolAtom( "elem_1" ) ) ),
            nl->IntAtom( 1238 ) );

        string typeString, errorString;
        bool correct = false;
        bool evaluable = false;
        bool defined = false;
        bool isFunction = false;
        Word areduceResult;
        bool resBool = QueryProcessor::ExecuteQuery( areduceQuery, 
            areduceResult, typeString, errorString, correct, evaluable,
             defined, isFunction );

        result = qp->ResultStorage( s );
        DFRel* resultDFRel = ( DFRel* )result.addr;
        
        if( !resBool || !correct || !evaluable || !defined ) {
            resultDFRel->makeUndefined( );
            return 0;
        }

        resultDFRel->copyFrom( *( ( DArrayBase* )areduceResult.addr ) );

        if( !resultDFRel->IsDefined( ) ) {
            return 0;
        }

        DistTypeRange* distType = new DistTypeRange( 
            range, pos, key, boundary );
        resultDFRel->setDistType( distType );

        return 0;
    }

/*
1.3 ValueMapping Array for dmap
    
Used by the operators with only a drel input.

*/
    ValueMapping drelgobalOpVM[ ] = {
        dreldmapVMT<DRel, DArray>,
        dreldmapVMT<DFRel, DFArray>,
        drelgobalOpVMT<DRel, DArray>,
        drelgobalOpVMT<DFRel, DFArray>
    };

/*
1.4 Selection function for dreldmap

Used to select the right position of the parameters. It is necessary, 
because the dmap-Operator ignores the second parameter. So so parameters 
must be moved to the right position for the dmap value mapping.

*/
    int drelglobalOpSelect( ListExpr args ) {

        ListExpr distType = nl->Third( nl->First( args ) );

        distributionType type;
        getTypeByNum( ( int )( nl->IntValue( nl->First( distType ) ) ), type );

        if( nl->HasLength( args, 1 ) ) {
            if( type == range ) {

                int attrNum = ( int ) 
                    ( nl->IntValue( nl->Second( distType ) ) );

                if( attrNum == 0 ) {
                    return DRel::checkType( 
                        nl->First( args ) ) ? 0 : 1;
                }
            }
            return DRel::checkType( nl->First( args ) ) ? 2 : 3;
        }
        
        if( type == range ) {

            int attrNum = ( int ) ( nl->IntValue( nl->Second( distType ) ) );

            ListExpr relType = nl->Second( nl->First( args ) );
            ListExpr attrList = nl->Second( nl->Second( relType ) );
            std::string attrName = nl->SymbolValue( 
                nl->First( nl->Second( args ) ) );

            ListExpr temp;
            if( listutils::findAttribute( 
                attrList, attrName, temp ) - 1 == attrNum ) {

                return DRel::checkType( 
                    nl->First( args ) ) ? 0 : 1;
            }
        }

        return DRel::checkType( nl->First( args ) ) ? 2 : 3 ;
    }

/*
1.5 Specification for all operators using dmapVM

1.5.7 Specification of rdup

Operator specification of the rdup operator.

*/
    OperatorSpec rdupSpec(
        " d[f]rel(X) "
        "-> d[f]rel(X) ",
        " _ rdup",
        "Removes duplicates in a d[f]rel. "
        "NOTE: Duplicates are only removed from the global d[f]rel and a "
        "repartition may be done.",
        " query drel1 rdup"
    );

/*
1.5.8 Specification of sort

Operator specification of the sort operator.

*/
    OperatorSpec sortSpec(
        " d[f]rel(X) "
        "-> d[f]rel(X) ",
        " _ sort",
        "Sorts a d[f]rel. "
        "NOTE: The operator only sorts the global d[f]rel and a "
        "repartition may be done.",
        " query drel1 sort"
    );
    
/*
1.5.9 Specification of drelgroupby

Operator specification of the drelgroupby operator.

*/
    OperatorSpec drelgroupbySpec(
        " d[f]rel(X) x attrlist x funlist "
        "-> d[f]rel(X) ",
        " _ drelgroupby[attrlist, funlist]",
        "Groups a d[f]rel according to attributes "
        "ai1, ..., aik and feeds the groups to other "
        "functions. The results of those functions are "
        "appended to the grouping attributes. The empty "
        "list is allowed for the grouping attributes (this "
        "results in a single group with all input tuples)."
        "NOTE: The operator groups the global d[f]rel and a "
        "repartition may be done.",
        " query drel1 drelgroupby[PLZ; Anz: group feed count]"
    );

/*
1.5.10 Specification of sortby

Operator specification of the sortby operator.

*/
    OperatorSpec sortbySpec(
        " d[f]rel(X) "
        "-> d[f]rel(X) ",
        " _ sortby[attrlist]",
        "Sorts a d[f]rel by a specific attribute list. "
        "NOTE: The operator sorts the global d[f]rel and a "
        "repartition may be done.",
        " query drel1 sortby[PLZ]"
    );

/*
1.6 Operator instance of operators using dmapVM

1.6.7 Operator instance of rdup operator

*/
    Operator rdupOp(
        "rdup",
        rdupSpec.getStr( ),
        4,
        drelgobalOpVM,
        drelglobalOpSelect,
        drelglobalOpTM<dmapFunctionMapper::dmapLocalMapper::lrdup>
    );
    
/*
1.6.8 Operator instance of sort operator

*/
    Operator sortOp(
        "sort",
        sortSpec.getStr( ),
        4,
        drelgobalOpVM,
        drelglobalOpSelect,
        drelglobalOpTM<dmapFunctionMapper::dmapLocalMapper::lsort>
    );
    
/*
1.6.9 Operator instance of drelgroupby operator

*/
    Operator drelgroupbyOp(
        "drelgroupby",
        drelgroupbySpec.getStr( ),
        4,
        drelgobalOpVM,
        drelglobalOpSelect,
        drelglobalOp3TM<dmapFunctionMapper::dmapLocalMapper::lgroupby>
    );

/*
1.6.10 Operator instance of sortby operator

*/
    Operator sortbyOp(
        "sortby",
        sortbySpec.getStr( ),
        4,
        drelgobalOpVM,
        drelglobalOpSelect,
        drelglobalOp2TM<dmapFunctionMapper::dmapLocalMapper::lsortby>
    );

} // end of namespace drel
