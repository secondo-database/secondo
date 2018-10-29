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


1 Implementation of the secondo operators drelsortmergejoin, drelitHashJoin, 
and drelitSpatialJoin

*/
//#define DRELDEBUG

#include "NestedList.h"
#include "ListUtils.h"
#include "QueryProcessor.h"
#include "StandardTypes.h"
#include "SecParser.h"

#include "Algebras/Distributed2/CommandLogger.h"
#include "Algebras/Distributed2/Distributed2Algebra.h"
#include "Algebras/Stream/Stream.h"
#include "Algebras/FText/FTextAlgebra.h"

#include "DRelHelpers.h"
#include "DRel.h"

extern NestedList* nl;
extern QueryProcessor* qp;

using namespace distributed2;

ListExpr RemoveTypeMap(ListExpr args);

namespace distributed2 {

    extern Distributed2Algebra* algInstance;

    int dmapXVM(Word* args, Word& result, int message,
            Word& local, Supplier s );

    template<int x>
    ListExpr dmapXTMT(ListExpr args);

    template<int pos, bool makeFS>
    ListExpr ARRAYFUNARG( ListExpr args );
};

namespace extrelationalg {

    template<bool OptionalIntAllowed, int defaultValue>
    ListExpr JoinTypeMap (ListExpr args);
};

namespace extrel2 {

    ListExpr itHashJoinTM(ListExpr args);
};

namespace drel {

/*
1.1 Type Mappings ~drelsimpleJoinTM~

Type mapping for all simple join operators with only two attributes as join 
parameters. Used for sortmergejoin and itHashJoin.

*/
    template<int joinType>
    ListExpr drelsimpleJoinTM( ListExpr args ) {

        string join = joinType == 0 ? "sortmergejoin" : "itHashJoin";

        std::string err = "d[f]rel(X) x d[f]rel(X) x attr x attr expected";

        if( !nl->HasLength( args, 4 ) ) {
            return listutils::typeError( err +
                ": four arguments are expected" );
        }

        if( !DRelHelpers::isListOfTwoElemLists( args ) ) {
            return listutils::typeError( "internal Error" );
        }

        ListExpr drel1Type = nl->First( nl->First( args ) );
        ListExpr drel1Value = nl->Second( nl->First( args ) );
        ListExpr drel2Type = nl->First( nl->Second( args ) );
        ListExpr drel2Value = nl->Second( nl->Second( args ) );

        ListExpr attr1Name = nl->First( nl->Third( args ) );
        ListExpr attr2Name = nl->First( nl->Fourth( args ) );

        ListExpr darray1Type, darray2Type;
        distributionType dType1, dType2;
        int dAttr1, dKey1, dAttr2, dKey2;

        if( !DRelHelpers::drelCheck( 
            drel1Type, darray1Type, dType1, dAttr1, dKey1 ) ) {

            return listutils::typeError(
                err + ": first argument is not a d[f]rel" );
        }

        if( !DRelHelpers::drelCheck( 
            drel2Type, darray2Type, dType2, dAttr2, dKey2 ) ) {
            
            return listutils::typeError(
                err + ": first argument is not a d[f]rel" );
        }

        if( dType1 == replicated || dType2 == replicated ) {
            return listutils::typeError( 
                "join of replicated d[f]rels not supported yet" );
        }

        ListExpr attr1List = 
            nl->Second( nl->Second( nl->Second( drel1Type ) ) );
        ListExpr attr2List = 
            nl->Second( nl->Second( nl->Second( drel2Type ) ) );

        // remove addition attributes used for partitioning ( e.g. Original )
        ListExpr resultAttr1List = 
            DRelHelpers::removePartitionAttributes( attr1List, dType1 );
        ListExpr resultAttr2List = 
            DRelHelpers::removePartitionAttributes( attr2List, dType2 );

        // type map check for join
        ListExpr joinTMArg = nl->FourElemList(
                nl->TwoElemList(
                    listutils::basicSymbol<Stream<Tuple>>( ),
                    nl->TwoElemList(
                        listutils::basicSymbol<Tuple>( ),
                        resultAttr1List ) ),
                nl->TwoElemList(
                    listutils::basicSymbol<Stream<Tuple>>( ),
                    nl->TwoElemList(
                        listutils::basicSymbol<Tuple>( ),
                        resultAttr2List ) ),
                attr1Name,
                attr2Name );

        ListExpr joinTMresult = joinType == 0 ? 
                extrelationalg::JoinTypeMap<false, 0>( joinTMArg ) :
                extrel2::itHashJoinTM( joinTMArg );
        
        if( !nl->HasLength( joinTMresult, 3 ) ) {
            return joinTMresult;
        }

        // type map check for dmap
        ListExpr map = nl->FourElemList(
                nl->SymbolAtom( "map" ),
                ARRAYFUNARG<1, false>( nl->OneElemList( darray1Type ) ), 
                ARRAYFUNARG<1, false>( nl->OneElemList( darray2Type ) ), 
                nl->TwoElemList(
                    listutils::basicSymbol<Stream<Tuple>>( ),
                    nl->TwoElemList(
                        listutils::basicSymbol<Tuple>( ),
                    ConcatLists( resultAttr1List, resultAttr2List ) ) ) );

        ListExpr feed1, feed2;

        if( dType1 == replicated 
         || dType1 == spatial2d
         || dType1 == spatial3d ) {

            ListExpr removeAttr = DRelHelpers::getRemovePartitonAttr( dType1 );

            feed1 = nl->ThreeElemList(
                nl->SymbolAtom( "remove" ),
                nl->ThreeElemList(
                    nl->SymbolAtom( "filter" ),
                    nl->TwoElemList(
                        nl->SymbolAtom( "feed" ),
                        nl->SymbolAtom( "elem1_1" ) ),
                    nl->ThreeElemList(
                        nl->SymbolAtom( "fun" ),
                        nl->TwoElemList(
                            nl->SymbolAtom( "streamelem_1" ),
                            nl->SymbolAtom( "STREAMELEM" ) ),
                        nl->ThreeElemList(
                            nl->SymbolAtom( "=" ),
                            nl->ThreeElemList(
                                nl->SymbolAtom( "attr" ),
                                nl->SymbolAtom( "streamelem_1" ),
                                nl->SymbolAtom( "Original" ) ),
                            nl->BoolAtom( true ) ) ) ),
                removeAttr );
        }
        else {
            feed1 = nl->TwoElemList(
                nl->SymbolAtom( "feed" ),
                nl->SymbolAtom( "elem1_1" ) );
        }

        if( dType2 == replicated 
         || dType2 == spatial2d
         || dType2 == spatial3d ) {

            ListExpr removeAttr = DRelHelpers::getRemovePartitonAttr( dType2 );

            feed2 = nl->ThreeElemList(
                nl->SymbolAtom( "remove" ),
                nl->ThreeElemList(
                    nl->SymbolAtom( "filter" ),
                    nl->TwoElemList(
                        nl->SymbolAtom( "feed" ),
                        nl->SymbolAtom( "elem2_2" ) ),
                    nl->ThreeElemList(
                        nl->SymbolAtom( "fun" ),
                        nl->TwoElemList(
                            nl->SymbolAtom( "streamelem_2" ),
                            nl->SymbolAtom( "STREAMELEM" ) ),
                        nl->ThreeElemList(
                            nl->SymbolAtom( "=" ),
                            nl->ThreeElemList(
                                nl->SymbolAtom( "attr" ),
                                nl->SymbolAtom( "streamelem_2" ),
                                nl->SymbolAtom( "Original" ) ),
                            nl->BoolAtom( true ) ) ) ),
                removeAttr );
        }
        else {
            feed2 = nl->TwoElemList(
                nl->SymbolAtom( "feed" ),
                nl->SymbolAtom( "elem2_2" ) );
        }

        ListExpr fun = nl->FourElemList(
            nl->SymbolAtom( "fun" ),
            nl->TwoElemList(
                nl->SymbolAtom( "elem1_1" ),
                nl->SymbolAtom( "ARRAYFUNARG1" ) ),
            nl->TwoElemList(
                nl->SymbolAtom( "elem2_2" ),
                nl->SymbolAtom( "ARRAYFUNARG2" ) ),
            nl->FiveElemList(
                nl->SymbolAtom( join ),
                feed1,
                feed2,
                attr1Name,
                attr2Name ) );

        ListExpr dmapResult = dmapXTMT<2>(
            nl->FiveElemList(
                nl->TwoElemList( darray1Type, drel1Value ),
                nl->TwoElemList( darray2Type, drel2Value ),
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

        // Now TM for dmap2 Operator was successful
        // Check for repartitioning
        bool drel1reparti, drel2reparti;

        ListExpr resultDistType = DRelHelpers::repartition4JoinRequired( 
            drel1Value, drel2Value,
            attr1Name, attr2Name,
            dType1, dType2,
            resultAttr1List, resultAttr2List,
            dAttr1, dAttr2, dKey1, dKey2,
            drel1reparti, drel2reparti );

        if( drel1reparti ) {
            cout << "repartitioning of first drel required..." << endl;
        }

        if( drel2reparti ) {
            cout << "repartitioning of second drel required..." << endl;
        }

        ListExpr resultType = nl->ThreeElemList(
            listutils::basicSymbol<DFRel>( ),
            nl->Second( nl->Third( dmapResult ) ),
            resultDistType );

        ListExpr appendList = ConcatLists( 
            nl->Second( dmapResult ), 
            nl->FourElemList( 
                nl->BoolAtom( drel1reparti ),
                nl->BoolAtom( drel2reparti ),
                nl->StringAtom( nl->SymbolValue( attr1Name ) ),
                nl->StringAtom( nl->SymbolValue( attr2Name ) ) ) );

        return nl->ThreeElemList( 
            nl->SymbolAtom( Symbols::APPEND( ) ),
            appendList,
            resultType );
    }

/*
1.2 ~createDRelConvert~

Creates a d[f]array query for a pointer on a d[f]rel.
The drelType argument has to be the nested list type of the drel object.

*/
    template<class R>
    ListExpr createDRelConvert( ListExpr drelType, R* drel ) {
        return nl->TwoElemList(
            nl->SymbolAtom( "drelconvert" ),
            nl->TwoElemList(
                drelType,
                nl->TwoElemList(
                    nl->SymbolAtom( "ptr" ),
                    listutils::getPtrList( new R( *drel ) ) ) ) );
    }

/*
1.4 Value Mapping ~drelsimpleJoinVMT~

Uses two d[f]rels and compute a distributed itHashJoin and creates a new 
dfrel. The repartitioning of the two d[f]rels will be done automaticly if 
necessary. Used for sortmergejoin and itHashJoin.

*/
    template<class R, class P, class T, class Q, int joinType>
    int drelsimpleJoinVMT( Word* args, Word& result, int message,
        Word& local, Supplier s ) {

        string join = joinType == 0 ? "sortmergejoin" : "itHashJoin";
        
        R* drel1 = ( R* ) args[ 0 ].addr;
        T* drel2 = ( T* ) args[ 1 ].addr;

        ListExpr resultDistType = nl->Third( qp->GetType( s ) );
        distributionType targetDistType;
        getTypeByNum( nl->IntValue( nl->First( resultDistType ) ),
            targetDistType );

        ListExpr drelType[ ] = {
            qp->GetType( qp->GetSon( s, 0 ) ),
            qp->GetType( qp->GetSon( s, 1 ) ) };

        bool drelRepartiReq[ ] = {
            ( ( CcBool* ) args[ 6 ].addr )->GetBoolval( ),
            ( ( CcBool* ) args[ 7 ].addr )->GetBoolval( ) };

        string attrName[ ] = {
            ( ( CcString* ) args[ 8 ].addr )->GetValue( ),
            ( ( CcString* ) args[ 9 ].addr )->GetValue( ) };

        ListExpr repartitionQuery[ 2 ];
        
        // create repartion query for the first drel if neccessary
        if( drelRepartiReq[ 0 ] ) {
            if( !DRelHelpers::createRepartitionQuery<R, P, T>( 
                    drelType[ 0 ],
                    drel1,
                    drel2,
                    resultDistType,
                    attrName[ 0 ],
                    1238,
                    repartitionQuery[ 0 ],
                    1, 2, 3 ) ) {

                result = qp->ResultStorage( s );
                ( ( DFRel* )result.addr )->makeUndefined( );
            }
        }
        else {
            repartitionQuery[ 0 ] = createDRelConvert( drelType[ 0 ], drel1 );
        }

        // create repartion query for the second drel if neccessary
        if( drelRepartiReq[ 1 ] ) {
            if( !DRelHelpers::createRepartitionQuery<T, Q, R>( 
                    drelType[ 1 ],
                    drel2,
                    drel1,
                    resultDistType,
                    attrName[ 1 ],
                    1239,
                    repartitionQuery[ 1 ],
                    3, 4, 6 ) ) {

                result = qp->ResultStorage( s );
                ( ( DFRel* )result.addr )->makeUndefined( );
            }
        }
        else {
            repartitionQuery[ 1 ] = createDRelConvert( drelType[ 1 ], drel2 );
        }

        ListExpr query = nl->SixElemList(
            nl->SymbolAtom( "dmap2" ),
            repartitionQuery[ 0 ],
            repartitionQuery[ 1 ],
            nl->StringAtom( "" ),
            nl->FourElemList(
                nl->SymbolAtom( "fun" ),
                nl->TwoElemList(
                    nl->SymbolAtom( "elem1_3" ),
                    nl->SymbolAtom( "ARRAYFUNARG1" ) ),
                nl->TwoElemList(
                    nl->SymbolAtom( "elem2_4" ),
                    nl->SymbolAtom( "ARRAYFUNARG2" ) ),
                nl->FiveElemList(
                    nl->SymbolAtom( join ),
                    nl->TwoElemList(
                        nl->SymbolAtom( "feed" ),
                        nl->SymbolAtom( "elem1_3" ) ),
                    nl->TwoElemList(
                        nl->SymbolAtom( "feed" ),
                        nl->SymbolAtom( "elem2_4" ) ),
                    nl->SymbolAtom( attrName[ 0 ] ),
                    nl->SymbolAtom( attrName[ 1 ] ) ) ),
            nl->IntAtom( 1240 ) );

        string typeString, errorString;
        bool correct = false;
        bool evaluable = false;
        bool defined = false;
        bool isFunction = false;
        Word queryResult;
        bool resBool = QueryProcessor::ExecuteQuery( query, queryResult, 
            typeString, errorString, correct, evaluable, defined, isFunction );

        result = qp->ResultStorage( s );
        DFRel* resultDFRel = ( DFRel* )result.addr;

        if( !resBool || !correct || !evaluable || !defined ) {
            resultDFRel->makeUndefined( );
            return 0;
        }

        resultDFRel->copyFrom( *( ( DFArray* )queryResult.addr ) );

        if( !resultDFRel->IsDefined( ) ) {
            return 0;
        }

        if( targetDistType == hash ) {
            resultDFRel->setDistType( new DistTypeHash( hash, 
                nl->IntValue( nl->Second( resultDistType ) ) ) );
        }
        else {

            collection::Collection* resBoundary;
            if( drelRepartiReq[ 0 ] ) {
                resBoundary = ( ( DistTypeRange* )drel2->getDistType( ) )
                        ->getBoundary( )->Clone( );
            }
            else {
                resBoundary = ( ( DistTypeRange* )drel1->getDistType( ) )
                        ->getBoundary( )->Clone( );
            }

            resultDFRel->setDistType( new DistTypeRange( range, 
                nl->IntValue( nl->Second( resultDistType ) ),
                nl->IntValue( nl->Third( resultDistType ) ),
                resBoundary ) );
        }

        return 0;
    }

/*
1.3 ValueMapping Array for dmap
    
Used by the operators with only a drel input.

*/
    ValueMapping drelsortmergejoinVM[ ] = {
        drelsimpleJoinVMT<DRel, DArray, DRel, DArray, 0>,
        drelsimpleJoinVMT<DFRel, DFArray, DRel, DArray, 0>,
        drelsimpleJoinVMT<DRel, DArray, DFRel, DFArray, 0>,
        drelsimpleJoinVMT<DFRel, DFArray, DFRel, DFArray, 0>
    };

    ValueMapping drelitHashJoinVM[ ] = {
        drelsimpleJoinVMT<DRel, DArray, DRel, DArray, 1>,
        drelsimpleJoinVMT<DFRel, DFArray, DRel, DArray, 1>,
        drelsimpleJoinVMT<DRel, DArray, DFRel, DFArray, 1>,
        drelsimpleJoinVMT<DFRel, DFArray, DFRel, DFArray, 1>
    };

/*
1.4 Selection function for dreldmap

Used to select the right position of the parameters. It is necessary, 
because the dmap-Operator ignores the second parameter. So so parameters 
must be moved to the right position for the dmap value mapping.

*/
    int drelsimpleJoinSelect( ListExpr args ) {

        int t1 = DRel::checkType( nl->First( args ) ) ? 0 : 1;
        int t2 = DRel::checkType( nl->Second( args ) ) ? 0 : 2;

        return t1 + t2;
    }

/*
1.5 Specification for all operators using dmapVM

1.5.7 Specification of drelsortmergejoin

Operator specification of the drelsortmergejoin operator.

*/
    OperatorSpec drelsortmergejoinSpec(
        " d[f]rel(rel(X)) x d[f]rel(rel(X)) x attr x attr "
        "-> dfrel(rel(Y)) ",
        " _ _ drelsortmergejoin[_,_]",
        "Computes the equijoin of two d[f]rels using the new sort "
        "operator implementation.",
        " query drel1 {p} drel2 {o} drelsortmergejoin[PLZ_p, PLZ_o]"
    );

    OperatorSpec drelitHashJoinSpec(
        " d[f]rel(rel(X)) x d[f]rel(rel(X)) x attr x attr "
        "-> dfrel(rel(Y)) ",
        " _ _ drelitHashJoin[_,_]",
        "Computes a hash join of two d[f]rels. ",
        " query drel1 {p} drel2 {o} drelitHashJoin[PLZ_p, PLZ_o]"
    );

/*
1.6 Operator instance of the join operators

1.6.7 Operator instance of drelsortmergejoin operator

*/
    Operator drelsortmergejoinOp(
        "drelsortmergejoin",
        drelsortmergejoinSpec.getStr( ),
        4,
        drelsortmergejoinVM,
        drelsimpleJoinSelect,
        drelsimpleJoinTM<0>
    );

    Operator drelitHashJoinOp(
        "drelitHashJoin",
        drelitHashJoinSpec.getStr( ),
        4,
        drelitHashJoinVM,
        drelsimpleJoinSelect,
        drelsimpleJoinTM<1>
    );

} // end of namespace drel