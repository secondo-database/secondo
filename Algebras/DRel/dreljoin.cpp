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


1 Implementation of the secondo operators sortmergejoin, itHashJoin, 
and itSpatialJoin

*/
//#define DRELDEBUG

#include "NestedList.h"
#include "ListUtils.h"
#include "QueryProcessor.h"
#include "StandardTypes.h"
#include "SecParser.h"

#include "Stream.h"
#include "Algebras/FText/FTextAlgebra.h"

#include "DRelHelpers.h"
#include "DRel.h"
#include "Partitioner.hpp"
#include "drelport.h"

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

ListExpr realJoinMMRTreeTM(ListExpr args);

namespace drel {

/*
1.1 Type Mappings ~drelsimpleJoinTM~

Type mapping for all simple join operators with only two attributes as join 
parameters. Used for sortmergejoin and itHashJoin.

*/
    template<int joinType>
    ListExpr drelsimpleJoinTM( ListExpr args ) {

        std::string join = joinType == 0 ? "sortmergejoin" : "itHashJoin";

        if( joinType < 0 || joinType > 1 ) {
            return listutils::typeError( "join not supported" );
        }

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
        ListExpr resultType;

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

        bool rel1Flag = false;
        bool rel2Flag = false;
        bool compatFlag = false;

        if( dType1 == hash || dType1 == range ) {
            ListExpr ftype;
            int pos =  listutils::findAttribute( 
                resultAttr1List, nl->SymbolValue( attr1Name ), ftype );
            if( dAttr1 == pos - 1 ) {
                rel1Flag = true;
            }
        }

        if( dType2 == hash || dType2 == range ) {
            ListExpr ftype;
            int pos =  listutils::findAttribute( 
                resultAttr2List, nl->SymbolValue( attr2Name ), ftype );
            if( dAttr2 == pos - 1 ) {
                rel2Flag = true;
            }
        }

        if( rel1Flag && rel2Flag && dType1 == range && dType2 == range ) {
            compatFlag = dKey1 == dKey2;
        }

        ListExpr resultRel = nl->TwoElemList(
            listutils::basicSymbol<Relation>( ),
            nl->TwoElemList(
                listutils::basicSymbol<Tuple>( ),
                ConcatLists( 
                    resultAttr1List, resultAttr2List ) ) );

        if( rel1Flag && rel2Flag && compatFlag ) {
            resultType = nl->ThreeElemList(
                    listutils::basicSymbol<DFRel>( ),
                    resultRel,
                    nl->Third( nl->First( nl->First( args ) ) ) );
        }
        else if( rel1Flag && rel2Flag && !compatFlag ) {
            
            if( !nl->IsAtom( drel1Value )
             || !nl->IsAtom( drel2Value )
             || nl->AtomType( drel1Value ) != SymbolType
             || nl->AtomType( drel2Value ) != SymbolType ) {
                return listutils::typeError( 
                    "cannot estimate drel size" );
            }

            int size1 = DRelHelpers::countDRel( nl->SymbolValue( drel1Value ) );
            int size2 = DRelHelpers::countDRel( nl->SymbolValue( drel2Value ) );

            if( size1 <= size2 ) {
                resultType = nl->ThreeElemList(
                    listutils::basicSymbol<DFRel>( ),
                    resultRel,
                    nl->Third( nl->First( nl->First( args ) ) ) );
                    rel2Flag = false;
                    cout << "repartition of drel 1 is necessary" << endl;
            }
            else {
                resultType = nl->ThreeElemList(
                    listutils::basicSymbol<DFRel>( ),
                    resultRel,
                    nl->Third( nl->First( nl->Second( args ) ) ) );
                    rel1Flag = false;
                    cout << "repartition of drel 2 is necessary" << endl;
            }
        }
        else if( rel1Flag && !rel2Flag ) {
            resultType = nl->ThreeElemList(
                listutils::basicSymbol<DFRel>( ),
                resultRel,
                nl->Third( nl->First( nl->First( args ) ) ) );
        }
        else if( !rel1Flag && rel2Flag ) {
            resultType = nl->ThreeElemList(
                listutils::basicSymbol<DFRel>( ),
                resultRel,
                nl->Third( nl->First( nl->Second( args ) ) ) );
        }
        else {
            ListExpr ftype;
            int pos =  listutils::findAttribute( 
                resultAttr1List, nl->SymbolValue( attr1Name ), ftype ) - 1;

            resultType = nl->ThreeElemList(
                listutils::basicSymbol<DFRel>( ),
                resultRel,
                nl->TwoElemList( 
                    nl->IntAtom( hash ),
                    nl->IntAtom( pos ) ) );
        }

        std::string query1 = "(dmap2 ";
  
        std::string query2 = "\"\" (fun (elem1_1 ARRAYFUNARG1) "
            "(elem2_2 ARRAYFUNARG2) (" + join + " "
            "(feed elem1_1) (feed elem2_2) " +
            nl->SymbolValue( attr1Name ) + " " + 
            nl->SymbolValue( attr2Name ) + ")) "+ getDRelPortString() + ")";

        ListExpr appendList = ConcatLists(
            nl->FiveElemList( 
                nl->BoolAtom( rel1Flag ),
                nl->BoolAtom( rel2Flag ),
                nl->BoolAtom( compatFlag ),
                nl->TextAtom( query1 ),
                nl->TextAtom( query2 ) ),
            nl->TwoElemList(
                nl->StringAtom( nl->SymbolValue( attr1Name ) ),
                nl->StringAtom( nl->SymbolValue( attr2Name ) ) ) );

        return nl->ThreeElemList( 
            nl->SymbolAtom( Symbols::APPEND( ) ),
            appendList,
            resultType );
    }

/*
1.2 ~createdrel2darray~

Creates a d[f]array query for a pointer on a d[f]rel.
The drelType argument has to be the nested list type of the drel object.

*/
    template<class R>
    ListExpr createdrel2darray( ListExpr drelType, R* drel ) {
        return nl->TwoElemList(
            nl->SymbolAtom( "drel2darray" ),
            nl->TwoElemList(
                drelType,
                nl->TwoElemList(
                    nl->SymbolAtom( "ptr" ),
                    listutils::getPtrList( new R( *drel ) ) ) ) );
    }

    ListExpr hashPartition( ListExpr drelType, void* ptr, std::string attr, 
        int elem ) {

        std::string queryS;
        distributionType type;
        DRelHelpers::drelCheck( drelType, type );

        

        if( type == spatial2d || type == spatial3d ) {

            std::string elem1str, elem2str, streamstr;

            if( elem == 1 ) {
                elem1str = std::to_string( elem + 4 );
                elem2str = std::to_string( elem + 5 );
                streamstr = std::to_string( elem + 6 );
            }
            else {
                elem1str = std::to_string( elem + 5 );
                elem2str = std::to_string( elem + 6 );
                streamstr = std::to_string( elem + 7 );
            }

            queryS = "(collect2 (partitionF " + nl->ToString( 
                DRelHelpers::createdrel2darray( drelType, ptr ) ) + " \"\" "
                "(fun (elem" + elem1str + "_1 FFR) (elem" + elem2str + "_2 "
                "FFR) (remove (filter (feed elem" + elem1str + "_1) (fun "
                "(streamelem_" + streamstr + " STREAMELEM) (= (attr "
                "streamelem_" + streamstr + " Original) TRUE))) (Original "
                "Cell))) (fun (elem" + elem1str + "_4 FFR) (elem" + 
                elem2str + "_5 FFR) (hashvalue (attr elem" + elem2str + "_5 "
                "Name) 99999)) 0) \"\" "+getDRelPortString()+")";

        }
        else {
            std::string elemstr = std::to_string( elem );

            queryS = "(collect2 (partition " + nl->ToString(
                    DRelHelpers::createdrel2darray( drelType, ptr ) ) +
                    "\"\" (fun (elem_" + elemstr + " SUBSUBTYPE1) (hashvalue "
                    "(attr elem_" + elemstr + " " + attr + ") 99999)) 0) \"\" "
                    + getDRelPortString()+")";
        }

        ListExpr query;
        nl->ReadFromString( queryS, query );

        return query;
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

        std::string join = joinType == 0 ? "sortmergejoin" : "itHashJoin";
        
        R* drel1 = ( R* ) args[ 0 ].addr;
        T* drel2 = ( T* ) args[ 1 ].addr;

        distributionType dType;
        int attr, key;
        DFRel::checkDistType( 
            nl->Third( qp->GetType( s ) ), dType, attr, key );

        ListExpr drelType[ ] = {
            qp->GetType( qp->GetSon( s, 0 ) ),
            qp->GetType( qp->GetSon( s, 1 ) ) };

        bool relFlag[ ] = { ( ( CcBool* ) args[ 4 ].addr )->GetBoolval( ),
                            ( ( CcBool* ) args[ 5 ].addr )->GetBoolval( ) };

        std::string query[ ] = { ( ( FText* ) args[ 7 ].addr )->GetValue( ),
                            ( ( FText* ) args[ 8 ].addr )->GetValue( ) };
        std::string attrName[ ] = { 
                             ( ( CcString* ) args[ 9 ].addr )->GetValue( ),
                             ( ( CcString* ) args[ 10 ].addr )->GetValue( ) };

        result = qp->ResultStorage( s );
        DFRel* resultDFRel = ( DFRel* )result.addr;

        ListExpr partitionDRel1, partitionDRel2;
        collection::Collection* boundary=0;
        if( dType == hash ) {
            if( !relFlag[ 0 ] ) {
                partitionDRel1 = hashPartition( 
                    drelType[ 0 ], drel1, attrName[ 0 ], 1 );
            }
            else {
                partitionDRel1 = DRelHelpers::createdrel2darray( 
                    drelType[ 0 ], drel1 );
            }

            if( !relFlag[ 1 ] ) {
                partitionDRel2 = hashPartition( 
                    drelType[ 1 ], drel2, attrName[ 1 ], 2 );
            }
            else {
                partitionDRel2 = DRelHelpers::createdrel2darray( 
                    drelType[ 1 ], drel2 );
            }
        }
        else {  // must be range
            ListExpr boundaryType = nl->Fourth( 
                nl->Third( qp->GetType( s ) ) );

            if( !relFlag[ 0 ] ) {
                boundary = ( ( DistTypeRange* )drel2->getDistType( ) )
                    ->getBoundary( );

                Partitioner<R, P>* parti = new Partitioner<R, P>( 
                    attrName[ 0 ], boundaryType, drel1, drelType[ 0 ], 
                    boundary, 1240 );

                if( !parti->repartition2DFMatrix( ) ) {
                    cout << "repartition failed!!" << endl;
                    resultDFRel->makeUndefined( );
                    return 0;
                }

                partitionDRel1 = DRelHelpers::createPointerList( 
                    parti->getMatrixType( ), parti->getDFMatrix( ) );

                partitionDRel1 = nl->FourElemList(
                    nl->SymbolAtom( "collect2" ),
                    partitionDRel1,
                    nl->StringAtom( "" ),
                    nl->IntAtom( 1240 ) );
            }
            else {
                partitionDRel1 = DRelHelpers::createdrel2darray( 
                    drelType[ 0 ], drel1 );
            }
            if( !relFlag[ 1 ] ) {
                boundary = ( ( DistTypeRange* )drel1->getDistType( ) )
                    ->getBoundary( );

                Partitioner<T, Q>* parti = new Partitioner<T, Q>( 
                    attrName[ 1 ], boundaryType, drel2, drelType[ 1 ], 
                    boundary, 1241 );

                if( !parti->repartition2DFMatrix( ) ) {
                    cout << "repartition failed!!" << endl;
                    resultDFRel->makeUndefined( );
                    return 0;
                }

                partitionDRel2 = DRelHelpers::createPointerList( 
                    parti->getMatrixType( ), parti->getDFMatrix( ) );

                partitionDRel2 = nl->FourElemList(
                    nl->SymbolAtom( "collect2" ),
                    partitionDRel2,
                    nl->StringAtom( "" ),
                    nl->IntAtom( 1241 ) );
            }
            else {
                partitionDRel2 = DRelHelpers::createdrel2darray( 
                    drelType[ 1 ], drel2 );
            }
        }
        
        std::string queryS = query[ 0 ] + nl->ToString( partitionDRel1 ) +
            nl->ToString( partitionDRel2 ) + query[ 1 ];

        ListExpr queryR;
        nl->ReadFromString( queryS, queryR );

        bool correct = false;
        bool evaluable = false;
        bool defined = false;
        bool isFunction = false;
        std::string typeString, errorString;
        Word dmapResult;
        if( !QueryProcessor::ExecuteQuery( queryR, dmapResult, 
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

        if( dType == hash ) {
            resultDFRel->setDistType( 
                new DistTypeHash( dType, attr) );
        }
        else {
            resultDFRel->setDistType( 
                new DistTypeRange( dType, attr, key, boundary ) );
        }

        return 0;
    }

/*
1.3 ValueMapping Array for dmap
    
Used by the operators with only a drel input.

*/
    ValueMapping sortmergejoinVM[ ] = {
        drelsimpleJoinVMT<DRel, DArray, DRel, DArray, 0>,
        drelsimpleJoinVMT<DFRel, DFArray, DRel, DArray, 0>,
        drelsimpleJoinVMT<DRel, DArray, DFRel, DFArray, 0>,
        drelsimpleJoinVMT<DFRel, DFArray, DFRel, DFArray, 0>
    };

    ValueMapping itHashJoinVM[ ] = {
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

1.5.7 Specification of sortmergejoin

Operator specification of the sortmergejoin operator.

*/
    OperatorSpec sortmergejoinSpec(
        " d[f]rel(rel(X)) x d[f]rel(rel(X)) x attr x attr "
        "-> dfrel(rel(Y)) ",
        " _ _ sortmergejoin[_,_]",
        "Computes the equijoin of two d[f]rels using the new sort "
        "operator implementation.",
        " query drel1 {p} drel2 {o} sortmergejoin[PLZ_p, PLZ_o]"
    );

    OperatorSpec itHashJoinSpec(
        " d[f]rel(rel(X)) x d[f]rel(rel(X)) x attr x attr "
        "-> dfrel(rel(Y)) ",
        " _ _ itHashJoin[_,_]",
        "Computes a hash join of two d[f]rels. ",
        " query drel1 {p} drel2 {o} itHashJoin[PLZ_p, PLZ_o]"
    );

/*
1.6 Operator instance of the join operators

1.6.7 Operator instance of sortmergejoin operator

*/
    Operator sortmergejoinOp(
        "sortmergejoin",
        sortmergejoinSpec.getStr( ),
        4,
        sortmergejoinVM,
        drelsimpleJoinSelect,
        drelsimpleJoinTM<0>
    );

    Operator itHashJoinOp(
        "itHashJoin",
        itHashJoinSpec.getStr( ),
        4,
        itHashJoinVM,
        drelsimpleJoinSelect,
        drelsimpleJoinTM<1>
    );

} // end of namespace drel
