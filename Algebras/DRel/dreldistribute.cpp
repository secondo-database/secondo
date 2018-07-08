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



1 Implementation of the secondo operator dreldistribute and drelfdistribute

*/
//#define DRELDEBUG

#include <iostream>

#include "NestedList.h"
#include "ListUtils.h"
#include "QueryProcessor.h"
#include "StandardTypes.h"
#include "SecParser.h"
#include "Algebras/Stream/Stream.h"

#include "Algebras/FText/FTextAlgebra.h"

#include "Algebras/Temporal/TemporalAlgebra.h"
#include "Algebras/Spatial/SpatialAlgebra.h"
#include "Algebras/Distributed2/CommandLogger.h"
#include "Algebras/Distributed2/Distributed2Algebra.h"
#include "Algebras/Relation-C++/OperatorFeed.h"

#include "DRel.h"
#include "DRelHelpers.h"
#include "DistributeOpHelper.h"

extern NestedList* nl;
extern QueryProcessor* qp;

using namespace distributed2;

namespace drel {

    /*
    5 Distribute operator

    1.1 ~distributeTM~

    Type mapping for the distribute operators.

    */
    template<class R, class A>
    ListExpr distributeTM( ListExpr args ) {

        #ifdef DRELDEBUG
        cout << "distributeTM" << endl;
        cout << "args" << endl;
        cout << nl->ToString( args ) << endl;
        #endif

        std::string err = "stream x rel x string x distType [x attr] [x int] "
            "expected";
        ListExpr newRes = nl->TheEmptyList( );
        ListExpr appendList = nl->TheEmptyList( );

        distributionType requestedDistType;

        if( !( nl->HasMinLength( args, 4 ) && nl->ListLength( args ) <= 6 ) ) {
            return listutils::typeError( err + ": wrong number of args" );
        }

        ListExpr relType = nl->First( args );
        ListExpr workerRelType = nl->Second( args );
        ListExpr nameType = nl->Third( args );
        ListExpr reqDistType = nl->Fourth( args );
        #ifdef DRELDEBUG
        cout << "relType" << endl;
        cout << nl->ToString( relType ) << endl;
        cout << "workerRelType" << endl;
        cout << nl->ToString( workerRelType ) << endl;
        cout << "nameType" << endl;
        cout << nl->ToString( nameType ) << endl;
        cout << "reqDistType" << endl;
        cout << nl->ToString( reqDistType ) << endl;
        #endif

        if( !Relation::checkType( relType ) ) {
            return listutils::typeError( err + 
                ": first parameter is no tuple stream" );
        }
        ListExpr streamType = nl->TwoElemList( 
            listutils::basicSymbol<Stream<Tuple>>(), 
            nl->Second( relType ) );
        #ifdef DRELDEBUG
        cout << "streamType" << endl;
        cout << nl->ToString( streamType ) << endl;
        #endif

        string errmsg;
        ListExpr types, positions;
        if( !( isWorkerRelDesc( workerRelType, positions, types, errmsg ) ) ) {
            return listutils::typeError(
                err + ": relation is not a worker relation" );
        }

        if( !CcString::checkType( nameType ) ) {
            return listutils::typeError( err + 
                ": third parameter is not a string" );
        }
        
        if( !nl->IsAtom( reqDistType ) ) {
            return listutils::typeError( err + 
                ": fourth parameter is distribution type" );
        }

        if( !supportedType( 
                nl->SymbolValue( reqDistType ),
                requestedDistType ) ) {
            return  listutils::typeError( err + 
                ": requested distType is not suppored" );
        }

        // create result type and append list for replicated
        if( nl->HasLength( args, 4 ) && requestedDistType == replicated ) {

            streamType = nl->TwoElemList( nl->First( streamType ),
                nl->TwoElemList( nl->First( nl->Second( streamType ) ),
                    listutils::concat(
                        nl->Second( nl->Second( streamType ) ),
                        nl->OneElemList(
                            nl->TwoElemList( nl->SymbolAtom( "Original" ),
                                listutils::basicSymbol<CcBool>( ) ) ) ) ) );

            ListExpr result = distribute3TM<A>( nl->FiveElemList(
                streamType,
                nameType,
                nl->SymbolAtom( CcInt::BasicType( ) ),
                nl->SymbolAtom( CcBool::BasicType( ) ),
                workerRelType ) );

            #ifdef DRELDEBUG
            cout << "distribute3TM result" << endl;
            cout << nl->ToString( result ) << endl;
            #endif

            if( !nl->HasLength( result, 3 ) ) {
                return result;
            }
            ListExpr resType = nl->Third( result );
            if( !A::checkType( resType ) ) {
                return result;
            }

            appendList = nl->FourElemList(
                nl->First( nl->Second( result ) ),
                nl->Second( nl->Second( result ) ),
                nl->Third( nl->Second( result ) ),
                nl->TextAtom( nl->ToString( relType ) ) );

            newRes = nl->ThreeElemList( listutils::basicSymbol<R>( ),
                nl->Second( resType ),
                nl->OneElemList( nl->IntAtom( requestedDistType ) ) );
            
        }
        // create result type and append list for random distribution
        else if( nl->HasLength( args, 5 ) && requestedDistType == random ) {

            ListExpr numSlotsType = nl->Fifth( args );
            ListExpr result = distribute3TM<A>( nl->FiveElemList(
                streamType,
                nameType,
                numSlotsType,
                nl->SymbolAtom( CcBool::BasicType( ) ),
                workerRelType ) );

            #ifdef DRELDEBUG
            cout << "distribute3TM result" << endl;
            cout << nl->ToString( result ) << endl;
            #endif

            if( !nl->HasLength( result, 3 ) ) {
                return result;
            }
            ListExpr resType = nl->Third( result );
            if( !A::checkType( resType ) ) {
                return result;
            }

            appendList = nl->ThreeElemList(
                nl->First( nl->Second( result ) ),
                nl->Second( nl->Second( result ) ),
                nl->Third( nl->Second( result ) ) );

            newRes = nl->ThreeElemList( listutils::basicSymbol<R>( ),
                nl->Second( resType ),
                nl->OneElemList( nl->IntAtom( requestedDistType ) ) );
        }

        // reqested type has to be hash, range, spatial2d 
        // or spatial3d for 6 arguments
        else if( nl->HasLength( args, 6 ) ) {

            if( !( nl->AtomType( nl->Fifth( args ) ) 
                    == SymbolType ) ) {
                return listutils::typeError( err + ": fifth parameter is not "
                    "an attribute" );
            }

            ListExpr numSlotsType = nl->Sixth( args );

            std::string attrName = nl->SymbolValue( nl->Fifth( args ) );
            ListExpr attrList = nl->Second( nl->Second( streamType ) );

            ListExpr attrType;
            int pos = listutils::findAttribute( attrList, attrName, attrType );
            if( pos == 0 ) {
                return listutils::typeError( 
                    err + ": attr name " + attrName + " not found" );
            }

            // create result type and append list for hash distribution
            if( requestedDistType == hash ) {
                
                if( !( CcInt::checkType( attrType ) ) ) {
                    return listutils::typeError( 
                        err + ": attribute is not of type " + 
                            CcInt::BasicType( ) );
                }

                // get result type from distributed4 operator
                ListExpr result = distribute4TMT<A>(
                    nl->FiveElemList(
                        streamType,
                        nameType,
                        nl->ThreeElemList( 
                            nl->SymbolAtom( "map" ),
                            nl->Second( streamType ),
                            listutils::basicSymbol<CcInt>( ) ),
                        numSlotsType,
                        workerRelType ) );

                #ifdef DRELDEBUG
                cout << "distribute4TMT result" << endl;
                cout << nl->ToString( result ) << endl;
                #endif

                if( !nl->HasLength( result, 3 ) ) {
                    return result;
                }
                ListExpr resType = nl->Third( result );
                if( !A::checkType( resType ) ) {
                    return result;
                }

                newRes = nl->ThreeElemList( listutils::basicSymbol<R>( ),
                    nl->Second( resType ),
                    nl->TwoElemList( 
                        nl->IntAtom( requestedDistType ),
                        nl->IntAtom( pos - 1 ) ) );

                appendList = nl->FourElemList(
                    nl->First( nl->Second( result ) ),
                    nl->Second( nl->Second( result ) ),
                    nl->Third( nl->Second( result ) ),
                    nl->StringAtom( attrName ) );
            }
            // create result type and append list for range distribution
            else if( requestedDistType == range ) {

                if( !DistTypeRange::allowedAttrType( attrType ) ) {
                    return listutils::typeError( err + ": attribute type "
                        "is not supported for range distribution" );
                }

                // get result type from distributed4 operator
                ListExpr result = distribute4TMT<A>(
                    nl->FiveElemList(
                        streamType,
                        nameType,
                        nl->ThreeElemList(
                            nl->SymbolAtom( "map" ),
                            nl->Second( streamType ),
                            nl->SymbolAtom( CcInt::BasicType( ) ) ),
                        numSlotsType,
                        workerRelType ) );

                if( !nl->HasLength( result, 3 ) ) {
                    return result;
                }
                ListExpr resType = nl->Third( result );
                if( !A::checkType( resType ) ) {
                    return result;
                }

                newRes = nl->ThreeElemList( listutils::basicSymbol<R>( ),
                    nl->Second( resType ),
                    nl->FourElemList(
                        nl->IntAtom( requestedDistType ),
                        nl->IntAtom( pos - 1 ),
                        nl->IntAtom( rand( ) ),
                        nl->TwoElemList(
                            nl->SymbolAtom( Vector::BasicType( ) ),
                            attrType ) ) );

                appendList = nl->FourElemList(
                    nl->First( nl->Second( result ) ),
                    nl->Second( nl->Second( result ) ),
                    nl->Third( nl->Second( result ) ),
                    nl->StringAtom( attrName ) );
            }
            // create result type and append list for spatial2d distribution
            else if( requestedDistType == spatial2d ) {

                if( !DistTypeSpatial<temporalalgebra::CellGrid2D>
                    ::allowedAttrType2d( attrType ) ) {
                    return listutils::typeError( err + ": attribute type "
                        "is not supported for spatial2d distribution" );
                }

                streamType = nl->TwoElemList( nl->First( streamType ),
                    nl->TwoElemList( nl->First( nl->Second( streamType ) ),
                        listutils::concat( 
                            nl->Second( nl->Second( streamType ) ),
                            nl->TwoElemList( 
                                nl->TwoElemList( nl->SymbolAtom( "Cell" ),
                                 listutils::basicSymbol<CcInt>( ) ),
                                nl->TwoElemList( nl->SymbolAtom( "Original" ),
                                 listutils::basicSymbol<CcBool>( ) ) ) ) ) );

                // get result type from distributed4 operator
                ListExpr result = ddistribute2TMT<A>(
                    nl->FiveElemList(
                        streamType,
                        nameType,
                        nl->SymbolAtom( "Cell" ),
                        numSlotsType,
                        workerRelType ) );

                if( !nl->HasLength( result, 3 ) ) {
                    return result;
                }
                ListExpr resType = nl->Third( result );
                if( !A::checkType( resType ) ) {
                    return result;
                }

                newRes = nl->ThreeElemList( listutils::basicSymbol<R>( ),
                    nl->Second( resType ),
                    nl->FourElemList(
                        nl->IntAtom( requestedDistType ),
                        nl->IntAtom( pos - 1 ),
                        nl->IntAtom( rand( ) ),
                        nl->SymbolAtom( 
                            temporalalgebra::CellGrid2D::BasicType( ) ) ) );

                appendList = nl->SixElemList(
                    nl->First( nl->Second( result ) ),
                    nl->Second( nl->Second( result ) ),
                    nl->Third( nl->Second( result ) ),
                    nl->Fourth( nl->Second( result ) ),
                    nl->StringAtom( attrName ),
                    nl->TextAtom( nl->ToString( relType ) ) );
            }
            // create result type and append list for spatial3d distribution
            else if( requestedDistType == spatial3d ) {

                if( !DistTypeSpatial<temporalalgebra::CellGrid<3>>
                    ::allowedAttrType3d( attrType ) ) {
                    return listutils::typeError( err + ": attribute type "
                        "is not supported for spatial2d distribution" );
                }

                streamType = nl->TwoElemList( nl->First( streamType ),
                    nl->TwoElemList( nl->First( nl->Second( streamType ) ),
                        listutils::concat(
                            nl->Second( nl->Second( streamType ) ),
                            nl->TwoElemList(
                                nl->TwoElemList( nl->SymbolAtom( "Cell" ),
                                    listutils::basicSymbol<CcInt>( ) ),
                                nl->TwoElemList( nl->SymbolAtom( "Original" ),
                                    listutils::basicSymbol<CcBool>( ) ) ) ) ) );

                // get result type from distributed4 operator
                ListExpr result = ddistribute2TMT<A>(
                    nl->FiveElemList(
                        streamType,
                        nameType,
                        nl->SymbolAtom( "Cell" ),
                        numSlotsType,
                        workerRelType ) );

                if( !nl->HasLength( result, 3 ) ) {
                    return result;
                }
                ListExpr resType = nl->Third( result );
                if( !A::checkType( resType ) ) {
                    return result;
                }

                newRes = nl->ThreeElemList( listutils::basicSymbol<R>( ),
                    nl->Second( resType ),
                    nl->FourElemList(
                        nl->IntAtom( requestedDistType ),
                        nl->IntAtom( pos - 1 ),
                        nl->IntAtom( rand( ) ),
                        nl->SymbolAtom(
                            temporalalgebra::CellGrid<3>::BasicType( ) ) ) );

                appendList = nl->SixElemList(
                    nl->First( nl->Second( result ) ),
                    nl->Second( nl->Second( result ) ),
                    nl->Third( nl->Second( result ) ),
                    nl->Fourth( nl->Second( result ) ),
                    nl->StringAtom( attrName ),
                    nl->TextAtom( nl->ToString( relType ) ) );
            }
            else {
                return listutils::typeError( 
                    err + ": for this number of arguments only range, "
                    "spatial2d or spatial3d is supported" );
            }

        }
        else {
            return listutils::typeError(
                err + ": distribution type does not fit to the number of "
                "arguments" );
        }

        #ifdef DRELDEBUG
        cout << "appendList" << endl;
        cout << nl->ToString( appendList ) << endl;
        cout << "newRes" << endl;
        cout << nl->ToString( newRes ) << endl;
        cout << "dreldistributeTM result type" << endl;
        cout << nl->ToString( nl->ThreeElemList( 
            nl->SymbolAtom( Symbols::APPEND( ) ),
            appendList,
            newRes ) ) << endl;
        #endif


        return nl->ThreeElemList( nl->SymbolAtom( Symbols::APPEND( ) ),
            appendList,
            newRes );
    }
    
    /*
    1.2 ~distributeVMTreplicated~

    Value mapping of the distribute operator to replicate data.

    */
    template<class RType, class AType, class DType, class HType, class CType>
    int distributeVMTreplicated( Word* args, Word& result, int message,
        Word& local, Supplier s ) {

        QueryProcessor* qps = new QueryProcessor( nl, am );

        Relation* rel = ( Relation* )args[ 0 ].addr;
        Relation* workers = ( Relation* )args[ 1 ].addr;
        FText* sourceRelType = ( FText* )args[ 7 ].addr;

        ListExpr sourceType;
        if( !nl->ReadFromString( sourceRelType->GetValue( ), sourceType ) ) {
            result = qp->ResultStorage( s );
            RType* drel = ( RType* )result.addr;
            drel->makeUndefined( );
            return 0;
        }

        OpTree stream = DistributeOpHelper::createReplicationOpTree( 
            qps, rel, sourceType, workers->GetNoTuples( ) );

        ArgVector argVec = {
            stream,
            args[ 2 ].addr,
            new CcInt( workers->GetNoTuples( ), true ),
            new CcBool( true, true ),  // only round robin
            args[ 1 ].addr,
            args[ 4 ].addr,
            args[ 5 ].addr,
            args[ 6 ].addr };

        distribute3VMT<AType, DType, HType, CType>( argVec,
            result, message, local, s );

        RType* drel = ( RType* )result.addr;
        if( drel->IsDefined( ) ) {
            drel->setDistType( new DistTypeBasic( replicated ) );
        }

        delete qps;

        return 0;
    }
    
    /*
    1.3 ~distributeVMTrandom~

    Value mapping of the distribute operator to distribute by round robin.

    */
    template<class RType, class AType, class DType, class HType, class CType>
    int distributeVMTrandom( Word* args, Word& result, int message,
        Word& local, Supplier s ) {
        
        #ifdef DRELDEBUG
        cout << "distributeVMTrandom" << endl;
        cout << "result type" << endl;
        cout << nl->ToString( qp->GetType( s ) ) << endl;
        #endif

        QueryProcessor* qps = new QueryProcessor( nl, am );

        Relation* rel = ( Relation* )args[ 0 ].addr;
        OpTree stream = DistributeOpHelper::createStreamOpTree(
            qps, nl->Second( qp->GetType( s ) ), rel );

        // new argument vector for distributqe3VMT
        ArgVector argVec = { 
            stream,
            args[ 2 ].addr,
            args[ 4 ].addr,
            new CcBool( true, true ),  // only round robin
            args[ 1 ].addr,
            args[ 5 ].addr,
            args[ 6 ].addr,
            args[ 7 ].addr };

        distribute3VMT<AType, DType, HType, CType>( argVec,
            result, message, local, s );

        RType* drel = ( RType* )result.addr;
        if( drel->IsDefined( ) ) {
            drel->setDistType( new DistTypeBasic( random ) );
        }

        delete qps;

        return 0;
    }    
    
    /*
    1.4 ~distributeVMThash~

    Value mapping of the distribute operator to distribute by hash.

    */
    template<class RType, class AType, class DType, class HType, class CType>
    int distributeVMThash( Word* args, Word& result, int message,
        Word& local, Supplier s ) {

        #ifdef DRELDEBUG
        cout << "distributeVMThash" << endl;
        cout << "result type" << endl;
        cout << nl->ToString( qp->GetType( s ) ) << endl;
        #endif

        QueryProcessor* qps = new QueryProcessor( nl, am );

        Relation* rel = ( Relation* )args[ 0 ].addr;
        OpTree stream = DistributeOpHelper::createStreamOpTree(
            qps, nl->Second( qp->GetType( s ) ), rel );

        string attrName = ( ( CcString* )args[ 9 ].addr )->GetValue( );

        ListExpr attrType;
        int pos = listutils::findAttribute(
            nl->Second( nl->Second( nl->Second( qp->GetType( s ) ) ) ),
            attrName,
            attrType );

        // Create OpTree for the hash function
        ListExpr funarg1 = nl->TwoElemList(
            nl->SymbolAtom( "t" ),
            nl->Second( nl->Second( qp->GetType( s ) ) ) );

        ListExpr fundef = nl->ThreeElemList(
            nl->SymbolAtom( "hashvalue" ),
            nl->ThreeElemList(
                nl->SymbolAtom( "attr" ),
                nl->SymbolAtom( "t" ),
                nl->SymbolAtom( attrName ) ), // Attributname for hashfunction
            nl->IntAtom( 10000 ) );

        ListExpr funList =
            nl->ThreeElemList(
                nl->SymbolAtom( "fun" ),
                funarg1,
                fundef );

        OpTree tree = 0;
        bool correct = false;
        bool evaluable = false;
        bool defined = false;
        bool isFunction = false;
        ListExpr resultType;
        QueryProcessor* qp2 = new QueryProcessor( nl, am );
        qp2->Construct( funList,
            correct,
            evaluable,
            defined,
            isFunction,
            tree,
            resultType );

        if( !correct ) {
            cout << "can not create operator tree" << endl;
            result = qp->ResultStorage( s );
            RType* drel = ( RType* )result.addr;
            drel->makeUndefined( );
            delete qps;
            return 0;
        }

        // new argument vector for distribute4VMT with the OpTree passed to 
        // the valuemapping of the distribute3 operator.
        ArgVector argVec = { 
            stream,
            args[ 2 ].addr,
            tree,
            args[ 5 ].addr,
            args[ 1 ].addr,
            args[ 6 ].addr,
            args[ 7 ].addr,
            args[ 8 ].addr };

        distribute4VMT<AType, DType, HType, CType>( argVec,
            result, message, local, s );

        qp2->Destroy( tree, true );
        delete qp2;

        RType* drel = ( RType* )result.addr;
        if( drel->IsDefined( ) ) {
            drel->setDistType( new DistTypeHash( hash, pos - 1 ) );
        }

        delete qps;

        return 0;

    }

    /*
    1.5 ~distributeVMTrange~

    Value mapping of the distribute operator to distribute by range.

    */
    template<class RType, class AType, class DType, class HType, class CType>
    int distributeVMTrange( Word* args, Word& result, int message,
        Word& local, Supplier s ) {

        #ifdef DRELDEBUG
        cout << "distributeVMTrange" << endl;
        cout << "result type" << endl;
        cout << nl->ToString( qp->GetType( s ) ) << endl;
        #endif

        QueryProcessor* qps = new QueryProcessor( nl, am );

        Relation* rel = ( Relation* )args[ 0 ].addr;
        OpTree stream = DistributeOpHelper::createStreamOpTree(
            qps, nl->Second( qp->GetType( s ) ), rel );

        CcInt* size = ( CcInt* )args[ 5 ].addr;
        string attrName = ( ( CcString* )args[ 10 ].addr )->GetValue( );
        
        ListExpr attrType;
        int pos = listutils::findAttribute(
            nl->Second( nl->Second( nl->Second( qp->GetType( s ) ) ) ),
            attrName,
            attrType );

        #ifdef DRELDEBUG
        cout << "attribute number" << endl;
        cout << pos << endl;
        cout << "attribute type" << endl;
        cout << nl->ToString( attrType ) << endl;
        #endif

        // get samplesize
        vector<Attribute*> sample;
        int count = rel->GetNoTuples( );

        int sampleSize = DRelHelpers::computeSampleSize( count );
        int nth = DRelHelpers::everyNthTupleForSample( sampleSize, count );

        // create a sample
        #ifdef DRELDEBUG
        cout << "create sample" << endl;
        #endif
        GenericRelationIterator* it = rel->MakeScan( );
        Tuple* tuple;
        while( ( tuple = it->GetNthTuple( nth, false ) ) ) {
            sample.push_back( tuple->GetAttribute( pos - 1 )->Clone( ) );
        }

        // sort the sample
        sort( sample.begin( ), sample.end( ), DRelHelpers::compareAttributes );

        // create the boundary
        int algebraId, typeId;
        string typeName;
        SecondoCatalog* sc = SecondoSystem::GetCatalog( );
        sc->GetTypeId( nl->SymbolAtom( Vector::BasicType( ) ),
            algebraId, typeId, typeName );
        ListExpr vectorNumType = nl->TwoElemList(
            nl->IntAtom( algebraId ), nl->IntAtom( typeId ) );

        sc->GetTypeId( attrType, algebraId, typeId, typeName );
        ListExpr attrNumType = nl->TwoElemList(
            nl->IntAtom( algebraId ), nl->IntAtom( typeId ) );

        collection::Collection* boundary = new collection::Collection(
            collection::vector,
            nl->TwoElemList( vectorNumType, attrNumType ) );
        boundary->Clear( );
        boundary->SetDefined( true );

        nth = DRelHelpers::everyNthTupleForArray( 
            sample.size( ), size->GetValue( ) - 1 );
        int i = 1;
        for( vector<Attribute*>::iterator it = sample.begin( );
            it != sample.end( ); ++it ) {

            if( i == nth ) {
                i = 1;
                boundary->Insert( ( *it ), 1 );
            } else {
                i++;
            }
            ( *it )->DeleteIfAllowed( );
        }

        sample.clear( );

        boundary->Finish( );

        #ifdef DRELDEBUG
        cout << endl;
        boundary->Print( cout );
        cout << endl;
        #endif

        // create the function to get the index of each attribute
        ListExpr funarg1 = nl->TwoElemList(
            nl->SymbolAtom( "t" ),
            nl->Second( nl->Second( qp->GetType( s ) ) ) );

        ListExpr fundef = nl->ThreeElemList(
            nl->SymbolAtom( "getboundaryindex" ),
            nl->TwoElemList(
                nl->TwoElemList( 
                    nl->SymbolAtom( Vector::BasicType( ) ), 
                    attrType ),
                nl->TwoElemList( 
                    nl->SymbolAtom( "ptr" ),                // pointer op
                    listutils::getPtrList( boundary ) ) ),  // Use pointer
            nl->ThreeElemList(
                nl->SymbolAtom( "attr" ),
                nl->SymbolAtom( "t" ),
                nl->SymbolAtom( attrName ) ) );

        ListExpr funList =
            nl->ThreeElemList(
                nl->SymbolAtom( "fun" ),
                funarg1,
                fundef );

        OpTree tree = 0;
        bool correct = false;
        bool evaluable = false;
        bool defined = false;
        bool isFunction = false;
        ListExpr resultType;
        QueryProcessor* qp2 = new QueryProcessor( nl, am );
        qp2->Construct( funList,
            correct,
            evaluable,
            defined,
            isFunction,
            tree,
            resultType );

        if( !correct ) {
            result = qp->ResultStorage( s );
            RType* drel = ( RType* )result.addr;
            drel->makeUndefined( );
            return 0;
        }

        // new argument vector for distribute4VMT
        ArgVector argVec = { 
            stream,
            args[ 2 ].addr,
            tree,
            size,
            args[ 1 ].addr,
            args[ 6 ].addr,
            args[ 7 ].addr,
            args[ 8 ].addr };

        distribute4VMT<AType, DType, HType, CType>( argVec,
            result, message, local, s );

        delete qp2;

        RType* drel = ( RType* )result.addr;
        if( drel->IsDefined( ) ) {
            drel->setDistType( new DistTypeRange( range, pos - 1, boundary ) );
        }

        delete qps;

        return 0;

    }

    /*
    1.6 ~distributeVMTspatial~

    Value mapping of the distribute operator for spatial distribution.

    */
    template<class RType, class AType, class DType, class HType, class CType, 
        class GType, distributionType T >
    int distributeVMTspatial( Word* args, Word& result, int message,
        Word& local, Supplier s ) {

        #ifdef DRELDEBUG
        cout << "distributeVMTspatial2d" << endl;
        cout << "result type" << endl;
        cout << nl->ToString( qp->GetType( s ) ) << endl;
        #endif

        Relation* rel = ( Relation* )args[ 0 ].addr;
        CcInt* size = ( CcInt* )args[ 5 ].addr;
        string attrName = ( ( CcString* )args[ 10 ].addr )->GetValue( );
        FText* sourceRelType = ( FText* )args[ 11 ].addr;


        ListExpr attrType;
        int pos = listutils::findAttribute(
            nl->Second( nl->Second( nl->Second( qp->GetType( s ) ) ) ),
            attrName,
            attrType );

        #ifdef DRELDEBUG
        cout << "tuplelist" << endl;
        cout << 
            nl->ToString( nl->Second( nl->Second( qp->GetType( s ) ) ) ) 
            << endl;
        cout << "attrName" << endl;
        cout << attrName << endl;
        cout << "pos" << endl;
        cout << pos << endl;
        cout << "sourceRelType" << endl;
        cout << sourceRelType->GetValue( ) << endl;
        #endif

        ListExpr sourceType;
        if( !nl->ReadFromString( sourceRelType->GetValue( ), sourceType ) ) {
            result = qp->ResultStorage( s );
            RType* drel = ( RType* )result.addr;
            drel->makeUndefined( );
            return 0;
        }

        // Create the grid to distribute
        GType* grid = DistributeOpHelper::createCellGrid<GType>( 
            rel, sourceType, attrName, size->GetIntval( ) );

        #ifdef DRELDEBUG
        cout << "grid" << endl;
        grid->Print( cout );
        #endif

        QueryProcessor* qps = new QueryProcessor( nl, am );

        OpTree stream = DistributeOpHelper::createStreamCellGridOpTree<GType>(
            qps, rel, sourceType, attrName, grid );
        
        // new argument vector for distribute4VMT with the OpTree passed to 
        // the valuemapping of the distribute3 operator.
        ArgVector argVec = { 
            stream,
            args[ 2 ].addr,
            args[ 4 ].addr,
            size,
            args[ 1 ].addr,
            args[ 6 ].addr,
            args[ 7 ].addr,
            args[ 8 ].addr,
            args[ 9 ].addr };

        ddistribute2VMT<AType, DType, HType, CType>( argVec,
            result, message, local, s );

        RType* drel = ( RType* )result.addr;
        if( drel->IsDefined( ) ) {
            drel->setDistType( 
                new DistTypeSpatial<GType>(
                    T, pos - 1, grid ) );
        }

        delete qps;

        return 0;

    }

    /*
    1.7 Specification of drelfdistirbute

    */
    OperatorSpec drelfdistributeSpec(
        " stream(tuple(X)) x rel(tuple(X)) x string x "
        "distType [x attr] [x int] -> dfrel(X) ",
        " _ drelfdistribute[ _, _, _, _, _]",
        "Distributes a tuple stream to the workers of the worker relation. "
        "The first argument is the stream to distribute. The second "
        "argument is the worker relation. It must be a relation having "
        "attributes Host, Port, and Config. Host and Config must be of "
        "type string or text, the Port attribute must be of type int. "
        "The third Argument is the name for the resulting dfrel. If "
        "the name is an empty string, a name is choosen automatically. "
        "The fourth argument is the distribution type. Possible values "
        "are random, hash, range, spatial2d spatial3d and replicated. "
        "This argument specifies the type to distribute the relation "
        "to the workers. The fifth argument is an attribute to distribute "
        "the relation. This attribute is required for the distType hash,  "
        "range, spatial2d and spatial3d and controls in which slot of the "
        "resulting array is the corresponding tuple inserted. The sixth "
        "argument specifies the size of the resulting array. If replicated "
        "is choosen this argument is unnecessary. ",
        " query strassen feed drelfdistribute[Worker3, \"\", \"range\", No, 5]"
    );

    /*
    1.8 ValueMapping Array of drelfdistribute

    */
    ValueMapping drelfdistributeVM[ ] = {
        distributeVMTrandom<DFRel, DFArray, FRelCopy, CcString, CcString>,
        distributeVMTrandom<DFRel, DFArray, FRelCopy, CcString, FText>,
        distributeVMTrandom<DFRel, DFArray, FRelCopy, FText, CcString>,
        distributeVMTrandom<DFRel, DFArray, FRelCopy, FText, FText>,
        distributeVMThash<DFRel, DFArray, FRelCopy, CcString, CcString>,
        distributeVMThash<DFRel, DFArray, FRelCopy, CcString, FText>,
        distributeVMThash<DFRel, DFArray, FRelCopy, FText, CcString>,
        distributeVMThash<DFRel, DFArray, FRelCopy, FText, FText>,
        distributeVMTrange<DFRel, DFArray, FRelCopy, CcString, CcString>,
        distributeVMTrange<DFRel, DFArray, FRelCopy, CcString, FText>,
        distributeVMTrange<DFRel, DFArray, FRelCopy, FText, CcString>,
        distributeVMTrange<DFRel, DFArray, FRelCopy, FText, FText>,
        distributeVMTspatial<DFRel, DFArray, FRelCopy, CcString, CcString,
            temporalalgebra::CellGrid2D, spatial2d>,
        distributeVMTspatial<DFRel, DFArray, FRelCopy, CcString, FText,
            temporalalgebra::CellGrid2D, spatial2d>,
        distributeVMTspatial<DFRel, DFArray, FRelCopy, FText, CcString,
            temporalalgebra::CellGrid2D, spatial2d>,
        distributeVMTspatial<DFRel, DFArray, FRelCopy, FText, FText,
            temporalalgebra::CellGrid2D, spatial2d>,
        distributeVMTspatial<DFRel, DFArray, FRelCopy, CcString, CcString,
            temporalalgebra::CellGrid<3>, spatial3d>,
        distributeVMTspatial<DFRel, DFArray, FRelCopy, CcString, FText,
            temporalalgebra::CellGrid<3>, spatial3d>,
        distributeVMTspatial<DFRel, DFArray, FRelCopy, FText, CcString,
            temporalalgebra::CellGrid<3>, spatial3d>,
        distributeVMTspatial<DFRel, DFArray, FRelCopy, FText, FText,
            temporalalgebra::CellGrid<3>, spatial3d>,
        distributeVMTreplicated<DFRel, DFArray, FRelCopy, CcString, CcString>,
        distributeVMTreplicated<DFRel, DFArray, FRelCopy, CcString, FText>,
        distributeVMTreplicated<DFRel, DFArray, FRelCopy, FText, CcString>,
        distributeVMTreplicated<DFRel, DFArray, FRelCopy, FText, FText>
    };

    /*
    1.9 Selection function

    */
    int distributeSelect( ListExpr args ) {
        
        #ifdef DRELDEBUG
        cout << "distributeSelect" << endl;
        cout << "args" << endl;
        cout << nl->ToString( args ) << endl;
        #endif

        distributionType type;
        supportedType( nl->SymbolValue( nl->Fourth( args ) ), type );

        ListExpr attrList = nl->Second( nl->Second( nl->Second( args ) ) );
        ListExpr hostType, configType;
        listutils::findAttribute( attrList, "Host", hostType );
        listutils::findAttribute( attrList, "Config", configType );
        int n1 = CcString::checkType( hostType ) ? 0 : 2;
        int n2 = CcString::checkType( configType ) ? 0 : 1;

        return n1 + n2 + 4 * type;
    }

    /*
    1.10 Operator instance of drelfdistribute operator 

    */
    Operator drelfdistributeOp(
        "drelfdistribute",
        drelfdistributeSpec.getStr( ),
        24,
        drelfdistributeVM,
        distributeSelect,
        distributeTM<DFRel, DFArray>
    );

    /*
    1.11 Specification of dreldistirbute

    */
    OperatorSpec dreldistributeSpec(
        " stream(tuple(X)) x rel(tuple(X)) x string x "
        "distType [x attr] [x int] -> drel(X) ",
        " _ dreldistribute[ _, _, _, _, _]",
        "Distributes a tuple stream to the workers of the worker relation. "
        "The first argument is the stream to distribute. The second "
        "argument is the worker relation. It must be a relation having "
        "attributes Host, Port, and Config. Host and Config must be of "
        "type string or text, the Port attribute must be of type int. "
        "The third Argument is the name for the resulting drel. If "
        "the name is an empty string, a name is choosen automatically. "
        "The fourth argument is the distribution type. Possible values "
        "are random, hash, range, spatial2d spatial3d and replicated. "
        "This argument specifies the type to distribute the relation "
        "to the workers. The fifth argument is an attribute to distribute "
        "the relation. This attribute is required for the distType hash,  "
        "range, spatial2d and spatial3d and controls in which slot of the "
        "resulting array is the corresponding tuple inserted. The sixth "
        "argument specifies the size of the resulting array. If replicated "
        "is choosen this argument is unnecessary. ",
        " query strassen feed dreldistribute[Worker3, \"\", \"range\", No, 5]"
    );


} // end of namespace drel