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
}

using namespace distributed2;

namespace drel {

    template<bool instream>
    int createboundaryVMT( 
        Word* args, Word& result, int message, Word& local, Supplier s );

/*
1 ~createStreamOpTree~

Creates a new operator tree to open a stream for a relation. For the
relation the type of the relation is nessecarry as a ListExpr. The
user is responsible for the right type of the relation.

*/
    OpTree createStreamOpTree(
        QueryProcessor* qps, ListExpr relType, Relation* rel ) {

        if( !Relation::checkType( relType ) ) {
            return 0;
        }

        bool correct = false;
        bool evaluable = false;
        bool defined = false;
        bool isFunction = false;
        ListExpr resultType;

        OpTree tree = 0;
        qps->Construct(
            nl->TwoElemList(
                nl->SymbolAtom( "feed" ),
                nl->TwoElemList(
                    relType,
                    nl->TwoElemList(
                        nl->SymbolAtom( "ptr" ),
                        listutils::getPtrList( rel ) ) ) ),
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

/*
2 ~createReplicationOpTree~

Creates a new operator tree to open a stream for a relation to replicate
the relation. For the relation the type of the relation is nessecarry as
a ListExpr. The user is responsible for the right type of the relation.
Of each tuple n tuple will be in the stream. n is the number of array
fields wich will be used to distribute the stream. For the distribution
the stream get two new attributes. The first attribute is Cell to get
the number of the field to distribute each tuple. The second argument
is Original. This attribute is used to reduce dublicates while collecting
the distributed tuple.

*/
    OpTree createReplicationOpTree(
        QueryProcessor* qps, Relation* rel, ListExpr relType,
        int size ) {

        #ifdef DRELDEBUG
        cout << "createReplicationOpTree" << endl;
        #endif

        ListExpr query = nl->ThreeElemList(
            nl->SymbolAtom( "remove" ),
            nl->ThreeElemList(
                nl->SymbolAtom( "extend" ),
                nl->ThreeElemList(
                    nl->SymbolAtom( "extendstream" ),
                    nl->TwoElemList(
                        nl->SymbolAtom( "feed" ),
                        nl->TwoElemList(
                            relType,
                            nl->TwoElemList(
                                nl->SymbolAtom( "ptr" ),
                                listutils::getPtrList( rel ) ) ) ),
                    nl->OneElemList(
                        nl->TwoElemList(
                            nl->SymbolAtom( "Cell" ),
                            nl->ThreeElemList(
                                nl->SymbolAtom( "fun" ),
                                nl->TwoElemList(
                                    nl->SymbolAtom( "tuple1" ),
                                    nl->SymbolAtom( "TUPLE" ) ),
                                nl->ThreeElemList(
                                    nl->SymbolAtom( "intstream" ),
                                    nl->IntAtom( 1 ),
                                    nl->IntAtom( size ) ) ) ) ) ),
                nl->OneElemList(
                    nl->TwoElemList(
                        nl->SymbolAtom( "Original" ),
                        nl->ThreeElemList(
                            nl->SymbolAtom( "fun" ),
                            nl->TwoElemList(
                                nl->SymbolAtom( "tuple2" ),
                                nl->SymbolAtom( "TUPLE" ) ),
                            nl->ThreeElemList(
                                nl->SymbolAtom( "=" ),
                                nl->ThreeElemList(
                                    nl->SymbolAtom( "attr" ),
                                    nl->SymbolAtom( "tuple2" ),
                                    nl->SymbolAtom( "Cell" ) ),
                                nl->IntAtom( 1 ) ) ) ) ) ),
            nl->OneElemList(
                nl->SymbolAtom( "Cell" ) ) );

        #ifdef DRELDEBUG
        cout << "query" << endl;
        cout << nl->ToString( query ) << endl;
        #endif

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

/*
3 ~createStreamCellGridOpTree~

Creates a new operator tree to open a stream for a relation to distribute
the relation by spatial. For the relation the type of the relation is
nessecarry as a ListExpr. The user is responsible for the right type of
the relation. For the distribution the stream get two new attributes. The
first attribute is Cell to get the number of the field to distribute each
tuple. The second argument is Original. This attribute is used to reduce
dublicates while collecting the distributed tuple. The grid argument is
used to distirbute the tuple.

*/
    template<class T>
    OpTree createStreamCellGridOpTree(
        QueryProcessor* qps, Relation* rel, ListExpr relType,
        std::string attrName, T* grid ) {

        #ifdef DRELDEBUG
        cout << "createStreamCellGridOpTree" << endl;
        #endif

        ListExpr query = nl->ThreeElemList(
            nl->SymbolAtom( "extend" ),
            nl->ThreeElemList(
                nl->SymbolAtom( "extendstream" ),
                nl->TwoElemList(
                    nl->SymbolAtom( "feed" ),
                    nl->TwoElemList(
                        relType,
                        nl->TwoElemList(
                            nl->SymbolAtom( "ptr" ),
                            listutils::getPtrList( rel ) ) ) ),
                nl->OneElemList(
                    nl->TwoElemList(
                        nl->SymbolAtom( "Cell" ),
                        nl->ThreeElemList(
                            nl->SymbolAtom( "fun" ),
                            nl->TwoElemList(
                                nl->SymbolAtom( "tuple1" ),
                                nl->SymbolAtom( "TUPLE" ) ),
                            nl->ThreeElemList(
                                nl->SymbolAtom( "cellnumber" ),
                                nl->TwoElemList(
                                    nl->SymbolAtom( "bbox" ),
                                    nl->ThreeElemList(
                                        nl->SymbolAtom( "attr" ),
                                        nl->SymbolAtom( "tuple1" ),
                                        nl->SymbolAtom( attrName ) ) ),
                                nl->TwoElemList(
                                    listutils::basicSymbol<T>( ),
                                    nl->TwoElemList(
                                        nl->SymbolAtom( "ptr" ),
                                        listutils::getPtrList( grid )
                                    ) ) ) ) ) ) ),
            nl->OneElemList(
                nl->TwoElemList(
                    nl->SymbolAtom( "Original" ),
                    nl->ThreeElemList(
                        nl->SymbolAtom( "fun" ),
                        nl->TwoElemList(
                            nl->SymbolAtom( "tuple2" ),
                            nl->SymbolAtom( "TUPLE" ) ),
                        nl->ThreeElemList(
                            nl->SymbolAtom( "=" ),
                            nl->ThreeElemList(
                                nl->SymbolAtom( "attr" ),
                                nl->SymbolAtom( "tuple2" ),
                                nl->SymbolAtom( "Cell" ) ),
                            nl->ThreeElemList(
                                nl->SymbolAtom( "extract" ),
                                nl->TwoElemList(
                                    nl->SymbolAtom( "transformstream" ),
                                    nl->ThreeElemList(
                                        nl->SymbolAtom( "cellnumber" ),
                                        nl->TwoElemList(
                                            nl->SymbolAtom( "bbox" ),
                                            nl->ThreeElemList(
                                                nl->SymbolAtom( "attr" ),
                                                nl->SymbolAtom( "tuple2" ),
                                                nl->SymbolAtom( attrName )
                                            ) ),
                                        nl->TwoElemList(
                                            listutils::basicSymbol<T>( ),
                                            nl->TwoElemList(
                                                nl->SymbolAtom( "ptr" ),
                                                listutils::
                                                getPtrList( grid )
                                            ) ) ) ),
                                nl->SymbolAtom( "Elem" ) ) ) ) ) ) );

        #ifdef DRELDEBUG
        cout << "query" << endl;
        cout << nl->ToString( query ) << endl;
        #endif

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

/*
4 ~createCellGrid~

Generates a optimal grid for a given relation with a given type. The grid
will be computed for a given attribute name and a given size of the
target array.

*/
    template<class T>
    T* createCellGrid(
        Relation* rel, ListExpr relType, std::string attrName, int size ) {

        #ifdef DRELDEBUG
        cout << "createCellGrid" << endl;
        #endif

        ListExpr query = nl->ThreeElemList(
            nl->SymbolAtom( "rect2cellgrid" ),
            nl->ThreeElemList(
                nl->SymbolAtom( "drelcollect_box" ),
                nl->TwoElemList(
                    nl->SymbolAtom( "transformstream" ),
                    nl->FourElemList(
                        nl->SymbolAtom( "projectextend" ),
                        nl->TwoElemList(
                            nl->SymbolAtom( "feed" ),
                            nl->TwoElemList(
                                relType,
                                nl->TwoElemList(
                                    nl->SymbolAtom( "ptr" ),
                                    listutils::getPtrList( rel ) ) ) ),
                        nl->TheEmptyList( ),
                        nl->OneElemList(
                            nl->TwoElemList(
                                nl->SymbolAtom( "Box" ),
                                nl->ThreeElemList(
                                    nl->SymbolAtom( "fun" ),
                                    nl->TwoElemList(
                                        nl->SymbolAtom( "tuple1" ),
                                        nl->SymbolAtom( "TUPLE" ) ),
                                    nl->TwoElemList(
                                        nl->SymbolAtom( "bbox" ),
                                        nl->ThreeElemList(
                                            nl->SymbolAtom( "attr" ),
                                            nl->SymbolAtom( "tuple1" ),
                                            nl->SymbolAtom( attrName )
                                        ) ) ) ) ) ) ),
                nl->BoolAtom( true ) ),
            nl->IntAtom( size ) );

        #ifdef DRELDEBUG
        cout << "query" << endl;
        cout << nl->ToString( query ) << endl;
        #endif

        Word resultGrid;
        QueryProcessor::ExecuteQuery( nl->ToString( query ), resultGrid );
        T* grid = ( T* )resultGrid.addr;

        #ifdef DRELDEBUG
        cout << "resultGrid" << endl;
        grid->Print( cout );
        #endif

        return grid;
    }

/*
1 Distribute operator

1.1 ~distributeTM~

Type mapping for the distribute operators.

*/
    template<class R, class A>
    ListExpr distributeTM( ListExpr args ) {

        /*std::string err = "rel x rel x string x distType [x attr] [x int] "
            "expected";*/
        std::string err = "rel x string x distType [x ident] [x int] x rel"
            "expected";
        ListExpr newRes = nl->TheEmptyList( );
        ListExpr appendList = nl->TheEmptyList( );

        distributionType requestedDistType;

        if( !( nl->HasMinLength( args, 4 ) && nl->ListLength( args ) <= 6 ) ) {
            return listutils::typeError( err + ": wrong number of args" );
        }

        ListExpr relType = nl->First( args );
        ListExpr nameType = nl->Second( args );
        ListExpr reqDistType = nl->Third( args );

        ListExpr workerRelType = nl->TheEmptyList( );
        switch( nl->ListLength( args ) ) {
        case 4: workerRelType = nl->Fourth( args ); break;
        case 5: workerRelType = nl->Fifth( args ); break;
        case 6: workerRelType = nl->Sixth( args ); break;
        }

        if( !Relation::checkType( relType ) ) {
            return listutils::typeError( err + 
                ": first parameter is no relation" );
        }
        ListExpr streamType = nl->TwoElemList( 
            listutils::basicSymbol<Stream<Tuple>>(), 
            nl->Second( relType ) );

        std::string errmsg;
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

            ListExpr numSlotsType = nl->Fourth( args );
            ListExpr result = distribute3TM<A>( nl->FiveElemList(
                streamType,
                nameType,
                numSlotsType,
                nl->SymbolAtom( CcBool::BasicType( ) ),
                workerRelType ) );

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

            if( !( nl->AtomType( nl->Fourth( args ) )
                    == SymbolType ) ) {
                return listutils::typeError( err + ": fifth parameter is not "
                    "an attribute" );
            }

            ListExpr numSlotsType = nl->Fifth( args );

            std::string attrName = nl->SymbolValue( nl->Fourth( args ) );
            ListExpr attrList = nl->Second( nl->Second( streamType ) );

            ListExpr attrType;
            int pos = listutils::findAttribute( attrList, attrName, attrType );
            if( pos == 0 ) {
                return listutils::typeError( 
                    err + ": attr name " + attrName + " not found" );
            }

            // create result type and append list for hash distribution
            if( requestedDistType == hash ) {

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
        Relation* workers = ( Relation* )args[ 3 ].addr;
        FText* sourceRelType = ( FText* )args[ 7 ].addr;

        ListExpr sourceType;
        if( !nl->ReadFromString( sourceRelType->GetValue( ), sourceType ) ) {
            result = qp->ResultStorage( s );
            RType* drel = ( RType* )result.addr;
            drel->makeUndefined( );
            return 0;
        }

        OpTree stream = createReplicationOpTree( 
            qps, rel, sourceType, workers->GetNoTuples( ) );

        CcInt* slots = new CcInt( workers->GetNoTuples( ) );
        CcBool* roundrobin = new CcBool( true, true );

        ArgVector argVec = { stream, args[ 1 ].addr,
            slots,
            roundrobin,
            args[ 3 ].addr, 
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
        delete slots;
        delete roundrobin;

        return 0;
    }
    
/*
1.3 ~distributeVMTrandom~

Value mapping of the distribute operator to distribute by round robin.

*/
    template<class RType, class AType, class DType, class HType, class CType>
    int distributeVMTrandom( Word* args, Word& result, int message,
        Word& local, Supplier s ) {

        QueryProcessor* qps = new QueryProcessor( nl, am );

        Relation* rel = ( Relation* )args[ 0 ].addr;
        OpTree stream = createStreamOpTree(
            qps, nl->Second( qp->GetType( s ) ), rel );

        CcBool* roundrobin = new CcBool( true, true );

        // new argument vector for distributqe3VMT
        ArgVector argVec = { 
            stream, 
            args[ 1 ].addr, 
            args[ 3 ].addr,
            roundrobin,
            args[ 4 ].addr, 
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
        delete roundrobin;

        return 0;
    }    
    
/*
1.4 ~distributeVMThash~

Value mapping of the distribute operator to distribute by hash.

*/
    template<class RType, class AType, class DType, class HType, class CType>
    int distributeVMThash( Word* args, Word& result, int message,
        Word& local, Supplier s ) {

        QueryProcessor* qps = new QueryProcessor( nl, am );

        Relation* rel = ( Relation* )args[ 0 ].addr;
        OpTree stream = createStreamOpTree(
            qps, nl->Second( qp->GetType( s ) ), rel );

        std::string attrName = ( ( CcString* )args[ 9 ].addr )->GetValue( );

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
            nl->IntAtom( 99999 ) );

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
            args[ 1 ].addr, 
            tree, 
            args[ 4 ].addr,
            args[ 5 ].addr, 
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

        QueryProcessor* qps = new QueryProcessor( nl, am );

        Relation* rel = ( Relation* )args[ 0 ].addr;
        OpTree stream = createStreamOpTree(
            qps, nl->Second( qp->GetType( s ) ), rel );

        std::string attrName = ( ( CcString* )args[ 9 ].addr )->GetValue( );
        ListExpr attrType;
        int pos = listutils::findAttribute(
            nl->Second( nl->Second( nl->Second( qp->GetType( s ) ) ) ),
            attrName,
            attrType );
        
        // create the boundary
        Word boundaryW;
        ArgVector argVecB = {
            rel,
            args[ 2 ].addr, // dummy not used
            args[ 4 ].addr,
            args[ 9 ].addr
        };

        createboundaryVMT<false>( argVecB, boundaryW, message, local, s );
        collection::Collection* boundary = 
            ( collection::Collection* )boundaryW.addr;

        if( !boundary || !boundary->IsDefined( ) ) {
            result = qp->ResultStorage( s );
            RType* drel = ( RType* )result.addr;
            drel->makeUndefined( );
        }

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
            args[ 1 ].addr, 
            tree, 
            args[ 4 ].addr,
            args[ 5 ].addr, 
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

        Relation* rel = ( Relation* )args[ 0 ].addr;
        int size = ( ( CcInt* )args[ 4 ].addr )->GetIntval( );
        size = 37; // static grid size. don't think about it
        std::string attrName = ( ( CcString* )args[ 10 ].addr )->GetValue( );
        FText* sourceRelType = ( FText* )args[ 11 ].addr;


        ListExpr attrType;
        int pos = listutils::findAttribute(
            nl->Second( nl->Second( nl->Second( qp->GetType( s ) ) ) ),
            attrName,
            attrType );

        ListExpr sourceType;
        if( !nl->ReadFromString( sourceRelType->GetValue( ), sourceType ) ) {
            result = qp->ResultStorage( s );
            RType* drel = ( RType* )result.addr;
            drel->makeUndefined( );
            return 0;
        }

        // Create the grid to distribute
        GType* grid = createCellGrid<GType>( 
            rel, sourceType, attrName, size );

        QueryProcessor* qps = new QueryProcessor( nl, am );

        OpTree stream = createStreamCellGridOpTree<GType>(
            qps, rel, sourceType, attrName, grid );
        
        // new argument vector for distribute2VMT with the OpTree passed to 
        // the valuemapping of the distribute2 operator.
        ArgVector argVec = { 
            stream, 
            args[ 1 ].addr, 
            args[ 3 ].addr, 
            args[ 4 ].addr, 
            args[ 5 ].addr, 
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

    OperatorSpec drelfdistributeSpec(
        " rel(tuple(X)) x string x distType [x ident] [x int] x"
        " rel(tuple(Y)) -> drel(tuple(X)) ",
        " _ drelfdistribute[ _, _, _, _, _]",
        "Distributes a relation to the workers of the worker relation. "
        "The first argument is the relation to distribute. The second "
        "Argument is the name for the resulting drel. If the name is an "
        "empty string, a name is choosen automatically. The Third "
        "argument is the distribution type. Possible values "
        "are RANDOM, HASH, RANGE, SPATIAL2D SPATIAL3D and REPLICATED. "
        "This argument specifies the type to distribute the relation "
        "to the workers. The fourth argument is an attribute to distribute "
        "the relation. This attribute is required for the distType HASH,  "
        "RANGE, SPATIAL2D and SPATIAL3D and controls in which slot of the "
        "resulting array is the corresponding tuple inserted. The fifth "
        "argument specifies the size of the resulting array. If REPLICATED "
        "is choosen this argument is unnecessary. The last argument is the "
        "worker relation. It must be a relation having attributes Host, Port, "
        "and Config. Host and Config must be of type string or text, the Port "
        "attribute must be of type int.",
        " query strassen drelfdistribute[Worker3, \"\", \"range\", No, 5]"
    );

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

    int distributeSelect( ListExpr args ) {

        distributionType type;
        supportedType( nl->SymbolValue( nl->Third( args ) ), type );

        ListExpr attrList = nl->TheEmptyList( );
        switch( nl->ListLength( args ) ) {
        case 4: attrList = nl->Second( nl->Second( nl->Fourth( args ) ) ); 
            break;
        case 5: attrList = nl->Second( nl->Second( nl->Fifth( args ) ) );
            break;
        case 6: attrList = nl->Second( nl->Second( nl->Sixth( args ) ) );;
            break;
        }

        ListExpr hostType, configType;
        listutils::findAttribute( attrList, "Host", hostType );
        listutils::findAttribute( attrList, "Config", configType );
        int n1 = CcString::checkType( hostType ) ? 0 : 2;
        int n2 = CcString::checkType( configType ) ? 0 : 1;

        return n1 + n2 + 4 * type;
    }

    Operator drelfdistributeOp(
        "drelfdistribute",
        drelfdistributeSpec.getStr( ),
        24,
        drelfdistributeVM,
        distributeSelect,
        distributeTM<DFRel, DFArray>
    );

    OperatorSpec dreldistributeSpec(
        " rel(tuple(X)) x string x distType [x ident] [x int] x"
        " rel(tuple(Y)) -> drel(tuple(X)) ",
        " _ dreldistribute[ _, _, _, _, _]",
        "Distributes a relation to the workers of the worker relation. "
        "The first argument is the relation to distribute. The second "
        "Argument is the name for the resulting drel. If the name is an "
        "empty string, a name is choosen automatically. The Third "
        "argument is the distribution type. Possible values "
        "are RANDOM, HASH, RANGE, SPATIAL2D SPATIAL3D and REPLICATED. "
        "This argument specifies the type to distribute the relation "
        "to the workers. The fourth argument is an attribute to distribute "
        "the relation. This attribute is required for the distType HASH,  "
        "RANGE, SPATIAL2D and SPATIAL3D and controls in which slot of the "
        "resulting array is the corresponding tuple inserted. The fifth "
        "argument specifies the size of the resulting array. If REPLICATED "
        "is choosen this argument is unnecessary. The last argument is the "
        "worker relation. It must be a relation having attributes Host, Port, "
        "and Config. Host and Config must be of type string or text, the Port "
        "attribute must be of type int.",
        " query strassen dreldistribute[\"\", \"RANGE\", No, 5, Worker3]"
    );

    ValueMapping dreldistributeVM[ ] = {
        distributeVMTrandom<DRel, DArray, RelFileRestorer, CcString, CcString>,
        distributeVMTrandom<DRel, DArray, RelFileRestorer, CcString, FText>,
        distributeVMTrandom<DRel, DArray, RelFileRestorer, FText, CcString>,
        distributeVMTrandom<DRel, DArray, RelFileRestorer, FText, FText>,
        distributeVMThash<DRel, DArray, RelFileRestorer, CcString, CcString>,
        distributeVMThash<DRel, DArray, RelFileRestorer, CcString, FText>,
        distributeVMThash<DRel, DArray, RelFileRestorer, FText, CcString>,
        distributeVMThash<DRel, DArray, RelFileRestorer, FText, FText>,
        distributeVMTrange<DRel, DArray, RelFileRestorer, CcString, CcString>,
        distributeVMTrange<DRel, DArray, RelFileRestorer, CcString, FText>,
        distributeVMTrange<DRel, DArray, RelFileRestorer, FText, CcString>,
        distributeVMTrange<DRel, DArray, RelFileRestorer, FText, FText>,
        distributeVMTspatial<DRel, DArray, RelFileRestorer, CcString, CcString,
            temporalalgebra::CellGrid2D, spatial2d>,
        distributeVMTspatial<DRel, DArray, RelFileRestorer, CcString, FText,
            temporalalgebra::CellGrid2D, spatial2d>,
        distributeVMTspatial<DRel, DArray, RelFileRestorer, FText, CcString,
            temporalalgebra::CellGrid2D, spatial2d>,
        distributeVMTspatial<DRel, DArray, RelFileRestorer, FText, FText,
            temporalalgebra::CellGrid2D, spatial2d>,
        distributeVMTspatial<DRel, DArray, RelFileRestorer, CcString, CcString,
            temporalalgebra::CellGrid<3>, spatial3d>,
        distributeVMTspatial<DRel, DArray, RelFileRestorer, CcString, FText,
            temporalalgebra::CellGrid<3>, spatial3d>,
        distributeVMTspatial<DRel, DArray, RelFileRestorer, FText, CcString,
            temporalalgebra::CellGrid<3>, spatial3d>,
        distributeVMTspatial<DRel, DArray, RelFileRestorer, FText, FText,
            temporalalgebra::CellGrid<3>, spatial3d>,
        distributeVMTreplicated<DRel, DArray, RelFileRestorer, CcString, 
                                CcString>,
        distributeVMTreplicated<DRel, DArray, RelFileRestorer, CcString, 
                                FText>,
        distributeVMTreplicated<DRel, DArray, RelFileRestorer, FText, 
                                CcString>,
        distributeVMTreplicated<DRel, DArray, RelFileRestorer, FText, FText>
    };

    Operator dreldistributeOp(
        "dreldistribute",
        dreldistributeSpec.getStr( ),
        24,
        dreldistributeVM,
        distributeSelect,
        distributeTM<DRel, DArray>
    );

} // end of namespace drel
