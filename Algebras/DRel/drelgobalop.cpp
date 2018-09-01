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


1 Implementation of the secondo operators drelfilter, drelproject, 
drelprojectextend, drellsortby, drellgroupby, drellsort, drellrdup, 
drelrename, drelhead and drelextend

This operators have the same value mapping witch calls the dmap operator of 
the Distributed2Algebra.

*/
//#define DRELDEBUG

#include "NestedList.h"
#include "ListUtils.h"
#include "QueryProcessor.h"
#include "StandardTypes.h"
#include "SecParser.h"

#include "Algebras/FText/FTextAlgebra.h"
#include "Algebras/Distributed2/CommandLogger.h"
#include "Algebras/Distributed2/Distributed2Algebra.h"

#include "DRelHelpers.h"
#include "DRel.h"
#include "Partitionier.hpp"

extern NestedList* nl;
extern QueryProcessor* qp;

using namespace distributed2;

namespace distributed2 {

    extern Distributed2Algebra* algInstance;

    template<class A>
    int dmapVMT( Word* args, Word& result, int message,
        Word& local, Supplier s );
};

namespace drel {

    ListExpr drellsortbyTM( ListExpr args );
    ListExpr drellrdupTM( ListExpr args );
    ListExpr drellgroupbyTM( ListExpr args );

    template<class R, class T, int parm>
    int dreldmapVMT( Word* args, Word& result, int message,
        Word& local, Supplier s );

    class dmapFunctionMapper {

        public:

            enum dmapLocalMapper { lsort, lsortby, lrdup, lgroupby };

            static ListExpr callDMapTM( ListExpr args, dmapLocalMapper i ) {

                switch ( i ) {
                    case lsort: return drellsortbyTM( args );
                    case lsortby: return drellsortbyTM( args );
                    case lrdup: return drellrdupTM( args );
                    case lgroupby: return drellgroupbyTM( args );
                }

                return nl->TheEmptyList( );
            }
    };


/*
1.1 Type Mappings for all operators using dmapVM

1.1 Type Mapping ~drelsortTM~

Expect a d[f]rel and attrtibute name to sort the distributed relation. 
Type mapping for the drelsort operator.

*/
    template<dmapFunctionMapper::dmapLocalMapper i>
    ListExpr drelglobalOpTM( ListExpr args ) {

        std::string err = "d[f]rel(X) x attrlist expected";

        if( !nl->HasLength( args, 2 ) ) {
            return listutils::typeError( err +
                ": two arguments are expected" );
        }

        if( !DRelHelpers::isListOfTwoElemLists( args ) ) {
            return listutils::typeError( "internal Error" );
        }

        ListExpr arg1Type = nl->First( nl->First( args ) );
        ListExpr arg2Type = nl->First( nl->Second( args ) );

        if( !DRel::checkType( arg1Type )
         && !DFRel::checkType( arg1Type ) ) {
            return listutils::typeError( err +
                ": first argument is not a d[f]rel" );
        }
        ListExpr relType = nl->Second( arg1Type );
        if( !Relation::checkType( relType ) ) {
            return listutils::typeError( err +
                ": first argument is not a d[f]rel" );
        }

        ListExpr distType = nl->Third( arg1Type );

        if( !nl->HasMinLength( arg2Type, 1 )
         && !( nl->AtomType( nl->First( arg2Type ) ) 
                == SymbolType ) ) {
            cout << "fehler" << endl;
            return listutils::typeError( err + ": second parameter is not "
                "an attribute list" );
        }

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
            if( !getTypeByNum( ( int )( nl->IntValue( nl->First( distType ) ) ),
                    type ) ) {
                return listutils::typeError(
                    err + ": internal error" );
            }

            if( type == range ) {
                int attrNum = ( int )
                    ( nl->IntValue( nl->Second( distType ) ) );

                if( attrNum == pos - 1 ) { 

                    // repartion not required
                    ListExpr dreldampRes = dmapFunctionMapper::callDMapTM( 
                        args, i );

                    if( !nl->HasLength( dreldampRes, 3 ) ) {
                        return listutils::typeError(
                            err + ": internal error" );
                    }

                    return dreldampRes;
                }
            }
        }

        // search the attributes
        ListExpr rest = nl->Rest( arg2Type );
        while( !nl->IsEmpty( rest ) ) {

            if( !nl->IsAtom( nl->First( rest ) )
             && !( nl->AtomType( nl->First( arg2Type ) ) 
                == SymbolType ) ) {
                return listutils::typeError( err + ": second parameter is not "
                "an attribute list" );
            } 

            ListExpr tempType;
            std::string tempName = nl->SymbolValue( nl->First( arg2Type ) );
            if( listutils::findAttribute( 
                attrList, tempName, tempType ) == 0 ) {

                return listutils::typeError(
                    err + ": attr name " + attrName + " not found" );
            }

            rest = nl->Rest( rest );
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

        std::string fun = "( fun( elem_1 AREDUCEARG1 )(sortby ( "
            "feed elem_1 ) " + nl->ToString( arg2Type ) + " ) )";

        ListExpr appendList = nl->FourElemList(
            nl->StringAtom( nl->SymbolValue( attrType ) ),
            nl->StringAtom( attrName ),
            nl->IntAtom( pos - 1 ),
            nl->TextAtom( fun ) );

        return nl->ThreeElemList( 
            nl->SymbolAtom( Symbols::APPEND( ) ),
            appendList,
            resultType );
    }

/*
1.2 Value Mapping ~dreldmapVMT~

Uses a d[f]rel and creates a new drel. The d[f]rel is created by calling 
the dmap value mapping of the Distributed2Algebra.

*/
    template<class R, class T>
    int drelgobalOpVMT( Word* args, Word& result, int message,
        Word& local, Supplier s ) {

        R* drel = ( R* )args[ 0 ].addr;
        string attrType = ( ( CcString* )args[ 2 ].addr )->GetValue( );
        string attrName = ( ( CcString* )args[ 3 ].addr )->GetValue( );
        int pos = ( ( CcInt* )args[ 4 ].addr )->GetValue( );
        string funString = ( ( FText* )args[ 5 ].addr )->GetValue( );

        ListExpr fun;
        if( !nl->ReadFromString( funString, fun ) ) {
            result = qp->ResultStorage( s );
            ( ( DFRel* )result.addr )->makeUndefined( );
            return 0;
        }

        string boundaryName = distributed2::algInstance->getTempName();

        Partitionier<R, T>* parti = new Partitionier<R, T>( attrName, attrType,
            drel, qp->GetType( qp->GetSon( s, 0 ) ), 1238, boundaryName );

        if( !parti->repartition2DFArray( fun, result ) ) {
            result = qp->ResultStorage( s );
            ( ( DFRel* )result.addr )->makeUndefined( );
        }

        DFRel* resultDrel = ( DFRel* )result.addr;
        if( !resultDrel->IsDefined( ) ) {
            return 0;
        }

        collection::Collection* boundary = parti->getBoundary( );

        DistTypeRange* distType = new DistTypeRange( range, pos, boundary );
        resultDrel->setDistType( distType );

        return 0;
    }

/*
1.3 ValueMapping Array for dmap
    
Used by the operators with only a drel input.

*/
    ValueMapping drelgobalOpVM[ ] = {
        dreldmapVMT<DRel, DArray, 0>,
        dreldmapVMT<DRel, DArray, 1>,
        dreldmapVMT<DRel, DArray, 2>,
        dreldmapVMT<DFRel, DFArray, 0>,
        dreldmapVMT<DFRel, DFArray, 1>,
        dreldmapVMT<DFRel, DFArray, 2>,
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

        cout << "drelglobalOpSelect" << endl;
        cout << nl->ToString( args ) << endl;

        distributionType type;
        getTypeByNum( ( int )( nl->IntValue( 
            nl->First( nl->Third( nl->First( args ) ) ) ) ),
            type );
        
        if( type == range ) {

            int attrNum = ( int )
                ( nl->IntValue( 
                    nl->Second( nl->Third( nl->First( args ) ) ) ) );

            ListExpr relType = nl->Second( nl->First( args ) );
            ListExpr attrList = nl->Second( nl->Second( relType ) );
            std::string attrName = nl->SymbolValue( 
                nl->First( nl->Second( args ) ) );

            ListExpr temp;
            if( listutils::findAttribute( 
                attrList, attrName, temp ) == attrNum ) {

                int parm = nl->ListLength( args ) - 1;
                return DRel::checkType( 
                    nl->First( args ) ) ? 0 + parm : 3 + parm;
            }
        }

        return DRel::checkType( nl->First( args ) ) ? 6 : 7;
    }

/*
1.5 Specification for all operators using dmapVM

1.5.7 Specification of drelrdup

Operator specification of the drelrdup operator.

*/
    OperatorSpec drelrdupSpec(
        " d[f]rel(X) "
        "-> d[f]rel(X) ",
        " _ drelrdup",
        "Removes duplicates in a d[f]rel. "
        "NOTE: Duplicates are only removed from the global d[f]rel and a "
        "repartition may be done.",
        " query drel1 drelrdup"
    );

/*
1.5.8 Specification of drelsort

Operator specification of the drelsort operator.

*/
    OperatorSpec drelsortSpec(
        " d[f]rel(X) "
        "-> d[f]rel(X) ",
        " _ drellsort",
        "Sorts a d[f]rel. "
        "NOTE: The operator only sorts the global d[f]rel and a "
        "repartition may be done.",
        " query drel1 drellsort"
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
        " query drel1 drellgroupby[PLZ; Anz: group feed count]"
    );

/*
1.5.10 Specification of drelsortby

Operator specification of the drelsortby operator.

*/
    OperatorSpec drelsortbySpec(
        " d[f]rel(X) "
        "-> d[f]rel(X) ",
        " _ drelsortby[attrlist]",
        "Sorts a d[f]rel by a specific attribute list. "
        "NOTE: The operator sorts the global d[f]rel and a "
        "repartition may be done.",
        " query drel1 drellsortby[PLZ]"
    );

/*
1.6 Operator instance of operators using dmapVM

1.6.7 Operator instance of drelrdup operator

*/
    Operator drelrdupOp(
        "drelrdup",
        drelrdupSpec.getStr( ),
        8,
        drelgobalOpVM,
        drelglobalOpSelect,
        drelglobalOpTM<dmapFunctionMapper::dmapLocalMapper::lrdup>
    );
    
/*
1.6.8 Operator instance of drelsort operator

*/
    Operator drelsortOp(
        "drelsort",
        drelsortSpec.getStr( ),
        8,
        drelgobalOpVM,
        drelglobalOpSelect,
        drelglobalOpTM<dmapFunctionMapper::dmapLocalMapper::lsort>
    );
    
/*
1.6.9 Operator instance of drelgroupby operator

*/
    Operator drelgroupbyOp(
        "drellgroupby",
        drelgroupbySpec.getStr( ),
        8,
        drelgobalOpVM,
        drelglobalOpSelect,
        drelglobalOpTM<dmapFunctionMapper::dmapLocalMapper::lgroupby>
    );

/*
1.6.10 Operator instance of drelsortby operator

*/
    Operator drelsortbyOp(
        "drelsortby",
        drelsortbySpec.getStr( ),
        8,
        drelgobalOpVM,
        drelglobalOpSelect,
        drelglobalOpTM<dmapFunctionMapper::dmapLocalMapper::lsortby>
    );

} // end of namespace drel