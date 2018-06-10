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
#include <iostream>

#include "NestedList.h"
#include "ListUtils.h"
#include "QueryProcessor.h"
#include "StandardTypes.h"
#include "Algebras/Stream/Stream.h"

#include "Algebras/FText/FTextAlgebra.h"

#include "Algebras/Temporal/TemporalAlgebra.h"
#include "Algebras/Spatial/SpatialAlgebra.h"
#include "Algebras/Distributed2/CommandLogger.h"
#include "Algebras/Distributed2/Distributed2Algebra.h"
#include "Algebras/Relation-C++/OperatorFeed.h"

#include "DRel.h"
#include "ShareInfo.h"
#include "DRelHelpers.h"

extern NestedList* nl;
extern QueryProcessor* qp;

namespace distributed2 {
    // used classes and functions of the Distributed2Algebra

    class FRelCopy;
    class RelFileRestorer;

    template<class R>
    ListExpr ddistribute2TMT( ListExpr args );

    template<class AType, class DType, class HType, class CType>
    int ddistribute2VMT( 
        Word* args, Word& result, int message, Word& local, Supplier s );
    
    template<class R>
    ListExpr distribute3TM( ListExpr args );

    template<class AType, class DType, class HType, class CType>
    int distribute3VMT( 
        Word* args, Word& result, int message, Word& local, Supplier s );

    template<class R>
    ListExpr distribute4TMT( ListExpr args );

    template<class AType, class DType, class HType, class CType>
    int distribute4VMT( 
        Word* args, Word& result, int message, Word& local, Supplier s );

    int shareTM( ListExpr args );

    template<class A>
    int shareVMT( Word* args, Word& result, int message,
        Word& local, Supplier s );
}

using namespace distributed2;

namespace drel {

    bool distributedefaultparmsTM(
        ListExpr args,
        const string err,
        ListExpr& error,
        distributionType& requestedDistType ) {

        if( !( nl->ListLength( args ) >= 4 && nl->ListLength( args ) <= 6 ) ) {
            error = listutils::typeError( err + ": wrong number of args" );
            return false;
        }

        // Check the first fourth parameters
        if( !( nl->HasLength( nl->First( args ), 2 )
            && nl->HasLength( nl->Second( args ), 2 )
            && nl->HasLength( nl->Third( args ), 2 )
            && nl->HasLength( nl->Fourth( args ), 2 ) ) ) {
            error = listutils::typeError( err + ": internal error" );
            return false;
        }

        if( !Relation::checkType( nl->First( nl->First( args ) ) ) ) {
            error = listutils::typeError(
                err + ": first argument is not a relation" );
            return false;
        }

        if( !CcString::checkType( nl->First( nl->Fourth( args ) ) ) ) {
            error = listutils::typeError(
                err + ": fourth parameter is not a string" );
            return false;
        }

        if( !nl->IsAtom( nl->Second( nl->Fourth( args ) ) ) ) {
            error = listutils::typeError(
                err + ": error with requested distType" );
            return false;
        }

        if( !supportedType( nl->StringValue(
            nl->Second( nl->Fourth( args ) ) ),
            requestedDistType ) ) {
            error = listutils::typeError(
                err + ": requested distType is not suppored" );
            return false;
        }
        return true;
    }

    OpTree createStreamOpTree(
        QueryProcessor* qps, string relName  ) {

        bool correct = false;
        bool evaluable = false;
        bool defined = false;
        bool isFunction = false;
        ListExpr resultType;

        OpTree tree = 0;
        qps->Construct(
            nl->TwoElemList(
                nl->SymbolAtom( "feed" ),
                nl->SymbolAtom( relName ) ),
            correct,
            evaluable,
            defined,
            isFunction,
            tree,
            resultType,
            true );
        qps->SetEvaluable( tree, true );

        return tree;
    }
    
    OpTree createStreamCellGridOpTree(
        QueryProcessor* qps, string relName, string attrName, 
        string gridName, string type ) {

        ListExpr query;
        if( !nl->ReadFromString( "( extendstream( "
            "feed " + relName + " )( ( Cell( fun( t TUPLE )"
            "( cellnumber( bbox( attr t " + attrName + " )"
            ")" + gridName + " ) ) ) ) )", query ) ) {
            return 0;
        }

        bool correct = false;
        bool evaluable = false;
        bool defined = false;
        bool isFunction = false;
        ListExpr resultType;

        OpTree tree = 0;
        qps->Construct(
            query,
            correct,
            evaluable,
            defined,
            isFunction,
            tree,
            resultType,
            true );
        qps->SetEvaluable( tree, true );

        return tree;
    }

    template<class R, class A>
    ListExpr distributeTM( ListExpr args ) {

        std::string err = "rel x rel x string x distType [x attr] [x int] "
            "expected";
        ListExpr result;
        ListExpr newRes;
        ListExpr resType;
        ListExpr appendList;
        ListExpr error;
        ListExpr attrType;

        distributionType requestedDistType;
        if( !distributedefaultparmsTM(
            args, err, error, requestedDistType ) ) {
            return error;
        }

        // Input types
        ListExpr relType = nl->First( nl->First( args ) );
        string relName = nl->SymbolValue( nl->Second( nl->First( args ) ) );
        ListExpr workerRelType = nl->First( nl->Second( args ) );
        ListExpr nameType = nl->First( nl->Third( args ) );

        // streamType for the TypeMapping of the Distributed2Algebra operators
        ListExpr streamType = nl->TwoElemList( 
            listutils::basicSymbol<Stream<Tuple> >( ),
            nl->Second( relType ) );

        if( nl->HasLength( args, 4 ) ) {

            // reqested typeString has to be replicated
            if( !( requestedDistType == replicated ) ) {
                return listutils::typeError( 
                    err + ": for this number of arguments "
                    "only replicated is supported" );
            }

            ListExpr positions;
            string errmsg;
            ListExpr types;;
            if( !( isWorkerRelDesc( workerRelType, positions, types, 
                errmsg ) ) ) {
                return listutils::typeError(
                    err + ": relation is not a worker relation" );
            }

            appendList = nl->SixElemList(
                nl->First( positions ),
                nl->Second( positions ),
                nl->Third( positions ),
                nl->BoolAtom(
                    CcString::checkType(
                        nl->First( types ) ) ),
                nl->BoolAtom(
                    CcString::checkType(
                        nl->Third( types ) ) ),
                nl->StringAtom( relName ) );

            newRes = nl->ThreeElemList( listutils::basicSymbol<R>( ),
                relType,
                DistTypeBasic::getTypeList( ) );
        }

        else if( nl->HasLength( args, 5 ) ) {

            // reqested typeString has to be random for 5 arguments
            if( !( requestedDistType == random ) ) {
                return listutils::typeError( 
                    err + ": for this number of arguments only random is "
                    "supported" );
            }
            if( !( nl->HasLength( nl->Fifth( args ), 2 ) ) ) {
                return listutils::typeError( err + ": internal error" );
            }

            ListExpr numSlotsType = nl->First( nl->Fifth( args ) );
            result = distribute3TM<A>( nl->FiveElemList(
                streamType,
                nameType,
                numSlotsType,
                nl->SymbolAtom( CcBool::BasicType( ) ),
                workerRelType ) );

            if( !nl->HasLength( result, 3 ) ) {
                return result;
            }
            resType = nl->Third( result );
            if( !A::checkType( resType ) ) {
                return result;
            }

            appendList = nl->FourElemList(
                nl->First( nl->Second( result ) ),
                nl->Second( nl->Second( result ) ),
                nl->Third( nl->Second( result ) ),
                nl->StringAtom( relName ) );

            newRes = nl->ThreeElemList( listutils::basicSymbol<R>( ),
                nl->Second( resType ),
                DistTypeBasic::getTypeList( ) );
        }

        // reqested typeString has to be hash, range, spatial2d 
        // or spatial3d for 5 arguments
        else if( nl->HasLength( args, 6 ) ) {

            if( !( nl->HasLength( nl->Fifth( args ), 2 ) ) ) {
                return listutils::typeError( err + ": internal error" );
            }
            if( !( nl->AtomType( nl->First( nl->Fifth( args ) ) ) 
                    == SymbolType ) ) {
                return listutils::typeError( err + ": fifth parameter is not "
                    "an attribute" );
            }

            if( !( nl->HasLength( nl->Sixth( args ), 2 ) ) ) {
                return listutils::typeError( err + ": internal error" );
            }
            ListExpr numSlotsType = nl->First( nl->Sixth( args ) );

            std::string attrName = nl->SymbolValue( 
                                        nl->Second( nl->Fifth( args ) ) );
            ListExpr attrList = nl->Second( nl->Second( relType ) );

            if( !DRelHelpers::findAttribute( attrList, attrName, attrType ) ) {
                return listutils::typeError( 
                    err + ": attr name " + attrName + " not found" );
            }

            if( requestedDistType == hash ) {
                
                if( !( CcInt::checkType( attrType ) ) ) {
                    return listutils::typeError( 
                        err + ": attribute is not of type " + 
                            CcInt::BasicType( ) );
                }

                // get result type from distributed4 operator
                result = distribute4TMT<A>(
                    nl->FiveElemList(
                        streamType,
                        nameType,
                        nl->ThreeElemList( 
                            nl->SymbolAtom( "map" ),
                            nl->Second( relType ),
                            listutils::basicSymbol<CcInt>( ) ),
                        numSlotsType,
                        workerRelType ) );

                if( !nl->HasLength( result, 3 ) ) {
                    return result;
                }
                resType = nl->Third( result );
                if( !A::checkType( resType ) ) {
                    return result;
                }

                resType = nl->Third( result );
                newRes = nl->ThreeElemList( listutils::basicSymbol<R>( ),
                    nl->Second( resType ),
                    DistTypeHash::getTypeList( ) );

                appendList = nl->FiveElemList(
                    nl->First( nl->Second( result ) ),
                    nl->Second( nl->Second( result ) ),
                    nl->Third( nl->Second( result ) ),
                    nl->StringAtom( attrName ),
                    nl->StringAtom( relName ) );
            }
            else if( requestedDistType == range ) {

                if( !( CcInt::checkType( attrType ) )
                    && !( CcString::checkType( attrType ) ) ) {
                    return listutils::typeError( 
                        err + ": attribute is not of type " + 
                        CcInt::BasicType( ) +
                        " or " + CcString::BasicType( ) );
                }

                // get result type from distributed4 operator
                result = distribute4TMT<A>(
                    nl->FiveElemList(
                        streamType,
                        nameType,
                        nl->ThreeElemList(
                            nl->SymbolAtom( "map" ),
                            nl->Second( relType ),
                            nl->SymbolAtom( CcInt::BasicType( ) ) ),
                        numSlotsType,
                        workerRelType ) );

                if( !nl->HasLength( result, 3 ) ) {
                    return result;
                }
                resType = nl->Third( result );
                if( !A::checkType( resType ) ) {
                    return result;
                }

                resType = nl->Third( result );
                newRes = nl->ThreeElemList( listutils::basicSymbol<R>( ),
                    nl->Second( resType ),
                    DistTypeRange::getTypeList( attrType ) );

                appendList = nl->FiveElemList(
                    nl->First( nl->Second( result ) ),
                    nl->Second( nl->Second( result ) ),
                    nl->Third( nl->Second( result ) ),
                    nl->StringAtom( attrName ),
                    nl->StringAtom( relName ) );
            }
            else if( requestedDistType == spatial2d ) {

                if( !( Point::checkType( attrType ) )
                    && !( Line::checkType( attrType ) )
                    && !( Region::checkType( attrType ) ) ) {
                    return listutils::typeError( 
                        err + ": attribute is not of type " +
                        Point::BasicType( ) + ", " + Line::BasicType( ) + 
                        " or " + Region::BasicType( ) );
                }

                streamType = nl->TwoElemList( nl->First( streamType ),
                    nl->TwoElemList( nl->First( nl->Second( streamType ) ),
                        listutils::concat( 
                            nl->Second( nl->Second( streamType ) ),
                            nl->OneElemList( 
                                nl->TwoElemList( nl->SymbolAtom( "Cell" ),
                                listutils::basicSymbol<CcInt>( ) ) ) ) ) );

                // get result type from distributed4 operator
                result = ddistribute2TMT<A>(
                    nl->FiveElemList(
                        streamType,
                        nameType,
                        nl->SymbolAtom( "Cell" ),
                        numSlotsType,
                        workerRelType ) );

                if( !nl->HasLength( result, 3 ) ) {
                    return result;
                }
                resType = nl->Third( result );
                if( !A::checkType( resType ) ) {
                    return result;
                }

                resType = nl->Third( result );
                newRes = nl->ThreeElemList( listutils::basicSymbol<R>( ),
                    nl->Second( resType ),
                    DistTypeSpatial<temporalalgebra::CellGrid2D>::getTypeList( 
                        attrType ) );

                appendList = nl->SixElemList(
                    nl->First( nl->Second( result ) ),
                    nl->Second( nl->Second( result ) ),
                    nl->Third( nl->Second( result ) ),
                    nl->Fourth( nl->Second( result ) ),
                    nl->StringAtom( attrName ),
                    nl->StringAtom( relName ) );
            }
            else if( requestedDistType == spatial3d ) {

                if( !( Point::checkType( attrType ) )
                    && !( Line::checkType( attrType ) )
                    && !( Region::checkType( attrType ) ) ) {
                    return listutils::typeError( 
                        err + ": attribute is not of type " +
                        Point::BasicType( ) + ", " + Line::BasicType( ) + 
                        " or " + Region::BasicType( ) );
                }

                // get result type from distributed4 operator
                result = distribute4TMT<A>(
                    nl->FiveElemList(
                        streamType,
                        nameType,
                        nl->ThreeElemList(
                            nl->SymbolAtom( "map" ),
                            nl->Second( relType ),
                            nl->SymbolAtom( CcInt::BasicType( ) ) ),
                        numSlotsType,
                        nameType ) );

                if( !nl->HasLength( result, 3 ) ) {
                    return result;
                }
                resType = nl->Third( result );
                if( !A::checkType( resType ) ) {
                    return result;
                }

                resType = nl->Third( result );
                newRes = nl->ThreeElemList( listutils::basicSymbol<R>( ),
                    nl->Second( resType ),
                    DistTypeSpatial<temporalalgebra::CellGrid<3>>::getTypeList(
                        attrType ) );

                appendList = nl->SixElemList(
                    nl->First( nl->Second( result ) ),
                    nl->Second( nl->Second( result ) ),
                    nl->Third( nl->Second( result ) ),
                    nl->Fourth( nl->Second( result ) ),
                    nl->StringAtom( attrName ),
                    nl->StringAtom( relName ) );
            }
            else {
                return listutils::typeError( 
                    err + ": for this number of arguments only range, "
                    "spatial2d or spatial3d is supported" );
            }

        }
        else {
            return listutils::typeError( err + ": wrong number of args" );
        }

        return nl->ThreeElemList( nl->SymbolAtom( Symbols::APPEND( ) ),
            appendList,
            newRes );
    }
    
    template<class RType, class AType, class DType, class HType, class CType>
    int distributeVMTreplicated( Word* args, Word& result, int message,
        Word& local, Supplier s ) {

        Relation* workers = ( Relation* )args[ 1 ].addr;
        int hostPos = ( ( CcInt* )args[ 4 ].addr )->GetValue( );
        int portPos = ( ( CcInt* )args[ 5 ].addr )->GetValue( );
        int confPos = ( ( CcInt* )args[ 6 ].addr )->GetValue( );
        bool hostStr = ( ( CcBool* )args[ 7 ].addr )->GetValue( );
        bool confStr = ( ( CcBool* )args[ 8 ].addr )->GetValue( );
        CcString* objName = ( CcString* )args[ 9 ].addr;

        result = qp->ResultStorage( s );
        RType* drel = ( RType* )result.addr;

        if( hostStr && confStr ) {
            drel->copyFrom( 
                DArrayBase::createFromRel<CcString, CcString, AType>(
                workers, 400, "tmp",
                hostPos, portPos, confPos ) );
        } 
        else if( hostStr && !confStr ) {
            drel->copyFrom( DArrayBase::createFromRel<CcString, FText, AType>(
                workers, 400, "tmp",
                hostPos, portPos, confPos ) );
        } 
        else if( !hostStr && confStr ) {
            drel->copyFrom( DArrayBase::createFromRel<FText, CcString, AType>(
                workers, 400, "tmp",
                hostPos, portPos, confPos ) );
        } 
        else if( !hostStr && !confStr ) {
            drel->copyFrom( DArrayBase::createFromRel<FText, FText, AType>(
                workers, 400, "tmp",
                hostPos, portPos, confPos ) );
        }
        else {
            drel->makeUndefined( );
            return 0;
        }

        if( !objName->IsDefined( ) ) {
            drel->makeUndefined( );
            return 0;
        }

        if( !drel->IsDefined( ) ) {
            return 0;
        }


        FText* ftext = new FText( true, "" );
        shareInfo<AType> info( objName->GetValue( ), true, drel, ftext );
        info.share( );

        drel->setDistType( new DistTypeBasic( replicated ) );

        return 0;
    }
    
    template<class RType, class AType, class DType, class HType, class CType>
    int distributeVMTrandom( Word* args, Word& result, int message,
        Word& local, Supplier s ) {

        Relation* workers = ( Relation* )args[ 1 ].addr;
        CcString* name = ( CcString* )args[ 2 ].addr;
        CcInt* size = ( CcInt* )args[ 4 ].addr;
        string relName = ( ( CcString* )args[ 8 ].addr )->GetValue( );

        QueryProcessor* qps = new QueryProcessor( nl, am );
        OpTree treeS = createStreamOpTree( qps, relName );

        // new argument vector for distributqe3VMT
        ArgVector argVec = { treeS,
            name,
            size,
            new CcBool( true, true ),  // only round robin
            workers,
            args[ 5 ].addr,
            args[ 6 ].addr,
            args[ 7 ].addr };

        distribute3VMT<AType, DType, HType, CType>( argVec,
            result, message, local, s );

        RType* drel = ( RType* )result.addr;
        if( drel->IsDefined( ) ) {
            drel->setDistType( new DistTypeBasic( random ) );
        }

        qps->Destroy( treeS, true );
        delete qps;

        return 0;
    }    
    
    template<class RType, class AType, class DType, class HType, class CType>
    int distributeVMThash( Word* args, Word& result, int message,
        Word& local, Supplier s ) {

        Relation* workers = ( Relation* )args[ 1 ].addr;
        CcString* name = ( CcString* )args[ 2 ].addr;
        CcInt* size = ( CcInt* )args[ 5 ].addr;
        string attrName = ( string )( char* )
            ( ( CcString* )args[ 9 ].addr )->GetStringval( );
        string relName = ( ( CcString* )args[ 10 ].addr )->GetValue( );

        QueryProcessor* qps = new QueryProcessor( nl, am );
        OpTree treeS = createStreamOpTree( qps, relName );

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
            qps->Destroy( treeS, true );
            delete qps;
            return 0;
        }

        // new argument vector for distribute4VMT with the OpTree passed to 
        // the valuemapping of the distribute3 operator.
        ArgVector argVec = { treeS,
            name,
            tree,
            size,
            workers,
            args[ 6 ].addr,
            args[ 7 ].addr,
            args[ 8 ].addr };

        distribute4VMT<AType, DType, HType, CType>( argVec,
            result, message, local, s );

        qp2->Destroy( tree, true );
        delete qp2;

        RType* drel = ( RType* )result.addr;
        if( drel->IsDefined( ) ) {

            ListExpr attrType;
            int pos = listutils::findAttribute(
                nl->Second( nl->Second( nl->Second( qp->GetType( s ) ) ) ),
                attrName,
                attrType );

            drel->setDistType( new DistTypeHash( hash, pos - 1 ) );

        }

        qps->Destroy( treeS, true );
        delete qps;

        return 0;

    }

    template<class RType, class AType, class DType, class HType, class CType>
    int distributeVMTrange( Word* args, Word& result, int message,
        Word& local, Supplier s ) {

        Relation* rel = ( Relation* )args[ 0 ].addr;
        Relation* workers = ( Relation* )args[ 1 ].addr;
        CcString* name = ( CcString* )args[ 2 ].addr;
        CcInt* size = ( CcInt* )args[ 5 ].addr;
        string attrName = ( string )( char* )
            ( ( CcString* )args[ 9 ].addr )->GetStringval( );
        string relName = ( ( CcString* )args[ 10 ].addr )->GetValue( );

        ListExpr attrType;
        int pos = listutils::findAttribute(
            nl->Second( nl->Second( nl->Second( qp->GetType( s ) ) ) ),
            attrName,
            attrType );

        // Create boundary and add to catalog
        Boundary* boundary = Boundary::createBoundary(
            rel, pos - 1, nl->SymbolValue( attrType ),
            size->GetIntval( ) );
        SecondoCatalog* sc = SecondoSystem::GetCatalog( );
        string boundaryName = DRelHelpers::randomBoundaryName( );
        ListExpr typeInfo = nl->TwoElemList(
            nl->SymbolAtom( Boundary::BasicType( ) ),
            nl->OneElemList( attrType ) );
        sc->InsertObject(
            boundaryName, Boundary::BasicType( ), typeInfo, boundary, true );

        QueryProcessor* qps = new QueryProcessor( nl, am );
        OpTree treeS = createStreamOpTree( qps, relName );

        // create the function to get the index of each attribute
        ListExpr funarg1 = nl->TwoElemList(
            nl->SymbolAtom( "t" ),
            nl->Second( nl->Second( qp->GetType( s ) ) ) );

        ListExpr fundef = nl->ThreeElemList(
            nl->SymbolAtom( "getboundaryindex" ),
            nl->SymbolAtom( boundaryName ),
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
            qps->Destroy( treeS, true );
            delete qps;
            return 0;
        }

        // new argument vector for distribute4VMT
        ArgVector argVec = { treeS,
            name,
            tree,
            size,
            workers,
            args[ 6 ].addr,
            args[ 7 ].addr,
            args[ 8 ].addr };

        distribute4VMT<AType, DType, HType, CType>( argVec,
            result, message, local, s );

        // qp2->Destroy( tree, true );
        // Word boundaryWord( boundary );
        // sc->DeleteObj( typeInfo, boundaryWord );
        delete qp2;

        RType* drel = ( RType* )result.addr;
        if( drel->IsDefined( ) ) {

            boundary->IncReference( );
            drel->setDistType( new DistTypeRange( range, pos - 1, boundary ) );
        }

        qps->Destroy( treeS, true );
        delete qps;

        return 0;

    }

    template<class RType, class AType, class DType, class HType, class CType>
    int distributeVMTspatial( Word* args, Word& result, int message,
        Word& local, Supplier s ) {

        Relation* rel = ( Relation* )args[ 0 ].addr;
        Relation* workers = ( Relation* )args[ 1 ].addr;
        CcString* name = ( CcString* )args[ 2 ].addr;
        CcInt* size = ( CcInt* )args[ 5 ].addr;
        string attrName = ( string )( char* )
            ( ( CcString* )args[ 10 ].addr )->GetStringval( );
        string relName = ( ( CcString* )args[ 11 ].addr )->GetValue( );

        ListExpr attrType;
        int pos = listutils::findAttribute(
            nl->Second( nl->Second( nl->Second( qp->GetType( s ) ) ) ),
            attrName,
            attrType );

        // Create the grid to distribute
        temporalalgebra::CellGrid2D* grid = DRelHelpers::createGrid( 
            rel, pos - 1, size->GetIntval( ) );
        SecondoCatalog* sc = SecondoSystem::GetCatalog( );
        string gridName = DRelHelpers::randomGridName( );
        if( !sc->InsertObject(
                gridName, temporalalgebra::CellGrid2D::BasicType( ), 
                nl->SymbolAtom( temporalalgebra::CellGrid2D::BasicType( ) ), 
                grid, true ) ) {
        }
        
        QueryProcessor* qps = new QueryProcessor( nl, am );
        OpTree treeS = createStreamCellGridOpTree( 
            qps, relName, attrName, gridName, nl->ToString( nl->Second( 
                nl->Second( qp->GetType( s ) ) ) ) );

        // new argument vector for distribute4VMT with the OpTree passed to 
        // the valuemapping of the distribute3 operator.
        ArgVector argVec = { treeS,
            name,
            args[ 4 ].addr,
            size,
            workers,
            args[ 6 ].addr,
            args[ 7 ].addr,
            args[ 8 ].addr,
            args[ 9 ].addr };

        ddistribute2VMT<AType, DType, HType, CType>( argVec,
            result, message, local, s );

        RType* drel = ( RType* )result.addr;
        if( drel->IsDefined( ) ) {
            drel->setDistType( 
                new DistTypeSpatial<temporalalgebra::CellGrid2D>( 
                    spatial2d, pos - 1, 
                    new temporalalgebra::CellGrid2D( *grid ) ) );
        }

        // Word boundaryWord( boundary );
        // sc->DeleteObj( typeInfo, grid );
        //qps->Destroy( treeS, true );
        delete qps;

        return 0;

    }

    /*
    1.4.6 Value Mapping for all with an attribute

    Distribute the relation to the workers by calling the distribute2,
    distribute3 and distribute4 value mappings of the Distributed2Algebra. 
    This ist the value mapping for the distribution types hash, range, 
    spatial2d and spatial3d. Calls the other value mappings described above. 

    */
    template<class RType, class AType, class DType, class HType, class CType>
    int distributeVMT( Word* args, Word& result, int message,
        Word& local, Supplier s ) {

        CcString* typeString = ( CcString* )args[ 3 ].addr;
        distributionType type = getType( typeString->GetValue( ) );

        switch( type ) {
        case hash:
            distributeVMThash<RType, AType, DType, HType, CType>(
                args, result, message, local, s );
            break;
        case range:
            distributeVMTrange<RType, AType, DType, HType, CType>(
                args, result, message, local, s );
            break;
        case spatial2d:
            distributeVMTspatial<RType, AType, DType, HType, CType>(
                args, result, message, local, s );
            break;
        case spatial3d:
            return 0;
            break;
        default:
            return 0;
            break;
        }

        return 0;

    }

    /*
    1.5 Specification of drelfdistirbute

    */
    OperatorSpec drelfdistributeSpec(
        " rel(tuple(X)) x rel(tuple(X)) x string x distType [x attr] [x int] "
        "-> dfrel(X) ",
        " _ drelfdistribute[ _, _, _, _, _]",
        "Distributes a relation into a dfrel. "
        "The first argument is the relation to distribute. The second "
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
        " query strassen drelfdistribute[Worker3, \"\", \"range\", No, 5]"
    );

    /*
    1.6 ValueMapping Array of drelfdistribute

    */
    ValueMapping drelfdistributeVM[ ] = {
        distributeVMT<DFRel, DFArray, FRelCopy, CcString, CcString>,
        distributeVMT<DFRel, DFArray, FRelCopy, CcString, FText>,
        distributeVMT<DFRel, DFArray, FRelCopy, FText, CcString>,
        distributeVMT<DFRel, DFArray, FRelCopy, FText, FText>,
        distributeVMTreplicated<DFRel, DFArray, FRelCopy, CcString, CcString>,
        distributeVMTreplicated<DFRel, DFArray, FRelCopy, CcString, FText>,
        distributeVMTreplicated<DFRel, DFArray, FRelCopy, FText, CcString>,
        distributeVMTreplicated<DFRel, DFArray, FRelCopy, FText, FText>,
        distributeVMTrandom<DFRel, DFArray, FRelCopy, CcString, CcString>,
        distributeVMTrandom<DFRel, DFArray, FRelCopy, CcString, FText>,
        distributeVMTrandom<DFRel, DFArray, FRelCopy, FText, CcString>,
        distributeVMTrandom<DFRel, DFArray, FRelCopy, FText, FText>
    };

    /*
    1.7 Selection function

    */
    int distributeSelect( ListExpr args ) {

        ListExpr rel = nl->Second( args );
        ListExpr attrList = nl->Second( nl->Second( rel ) );
        ListExpr hostType, configType;
        listutils::findAttribute( attrList, "Host", hostType );
        listutils::findAttribute( attrList, "Config", configType );
        int n1 = CcString::checkType( hostType ) ? 0 : 2;
        int n2 = CcString::checkType( configType ) ? 0 : 1;

        if( nl->HasLength( args, 4 ) ) {
            return n1 + n2 + 4; // for dist type replicated
        }
        if( nl->HasLength( args, 5 ) ) {
            return n1 + n2 + 8; // for dist type random
        }

        return n1 + n2;
    }

    /*
    1.8 Operator instance of drelfdistribute operator 

    */
    Operator drelfdistributeOp(
        "drelfdistribute",
        drelfdistributeSpec.getStr( ),
        4,
        drelfdistributeVM,
        distributeSelect,
        distributeTM<DFRel, DFArray>
    );

    /*
    1.9 Specification of dreldistirbute

    */
    OperatorSpec dreldistributeSpec(
        " rel(tuple(X)) x rel(tuple(X)) x string x distType [x attr] [x int] "
        "-> drel(X) ",
        " _ dreldistribute[ _, _, _, _, _]",
        "Distributes a relation into a drel. "
        "The first argument is the relation to distribute. The second "
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
        " query strassen dreldistribute[Worker3, \"\", \"range\", No, 5]"
    );

    /*
    1.10 ValueMapping Array of dreldistribute

    */
    ValueMapping dreldistributeVM[ ] = {
        distributeVMT<DRel, DArray, RelFileRestorer, CcString, CcString>,
        distributeVMT<DRel, DArray, RelFileRestorer, CcString, FText>,
        distributeVMT<DRel, DArray, RelFileRestorer, FText, CcString>,
        distributeVMT<DRel, DArray, RelFileRestorer, FText, FText>,
        distributeVMTreplicated<DRel, DArray, RelFileRestorer, CcString, 
                                CcString>,
        distributeVMTreplicated<DRel, DArray, RelFileRestorer, CcString, FText>,
        distributeVMTreplicated<DRel, DArray, RelFileRestorer, FText, CcString>,
        distributeVMTreplicated<DRel, DArray, RelFileRestorer, FText, FText>,
        distributeVMTrandom<DRel, DArray, RelFileRestorer, CcString, CcString>,
        distributeVMTrandom<DRel, DArray, RelFileRestorer, CcString, FText>,
        distributeVMTrandom<DRel, DArray, RelFileRestorer, FText, CcString>,
        distributeVMTrandom<DRel, DArray, RelFileRestorer, FText, FText>
    };

    /*
    1.11 Operator instance of dreldistribute operator

    */
    Operator dreldistributeOp(
        "dreldistribute",
        dreldistributeSpec.getStr( ),
        12,
        dreldistributeVM,
        distributeSelect,
        distributeTM<DRel, DArray>
    );

} // end of namespace drel