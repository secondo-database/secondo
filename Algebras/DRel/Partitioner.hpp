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

*/
#ifndef _Partitioner_h_
#define _Partitioner_h_

#include "Algebras/Collection/CollectionAlgebra.h"
#include "Stream.h"
#include "DRelHelpers.h"
#include "DRel.h"
#include "Algebras/Distributed2/CommandLogger.h"
#include "Algebras/Distributed2/Distributed2Algebra.h"

//#define DRELDEBUG

extern NestedList* nl;
extern QueryProcessor* qp;

namespace distributed2 {

    extern Distributed2Algebra* algInstance;

    ListExpr partitionTM(ListExpr args);
    ListExpr partitionFTM(ListExpr args);

    template<class A>
    int partitionVMT( Word* args, Word& result, int message,
                    Word& local, Supplier s );
}

namespace drel {
/*
1 ~Partitioner~

Class to repartition a DRel by range.

*/
    template<class R, class T>
    class Partitioner {

    public:
/*
1.1 Constructors

*/
        Partitioner( std::string _attr, ListExpr _boundaryType, R* _drel, 
            ListExpr _sourcedType, collection::Collection* _boundary, 
            int _port, std::string _boundaryName ) :
            attr( _attr ), boundaryType( _boundaryType ), drel( _drel ), 
            sourcedType( _sourcedType ), boundary( _boundary ), 
            qp( new QueryProcessor( nl, am ) ), matrix( 0 ), port( _port ), 
            boundaryName( _boundaryName ), tree( 0 ) {

            boundary->Copy( );
            dType = drel->getDistType( )->getDistType( );
        }

        Partitioner( std::string _attr, ListExpr _boundaryType, R* _drel, 
            ListExpr _sourcedType, collection::Collection* _boundary, 
            int _port ) :
            Partitioner( _attr, _boundaryType, _drel, _sourcedType, _boundary,
            _port, distributed2::algInstance->getTempName( ) ) {

            dType = drel->getDistType( )->getDistType( );
        }

/*
1.2 Copy-Constructor

*/
        Partitioner( const Partitioner& src ) :
            attr( src.attr ), boundaryType( src.boundaryType ), 
            drel( src.drel ), sourcedType( src.sourcedType ), 
            boundary( *( src.boundary ) ), qp( new QueryProcessor( nl, am ) ), 
            matrix( *( src.matrix ) ), port( src.port ), 
            boundaryName( src.boundaryName ), dType( src.dType ), tree( 0 ) {
        }

/*
1.2 Assignment operator

*/
        Partitioner& operator=( const Partitioner& src ) {
            if( this == &src ) {
                return *this;
            }
            attr = src.attr;
            boundaryType = src.boundaryType;
            drel = src.drel;
            sourcedType = src.sourcedType;
            boundary = src.boundary;
            matrix = src.matrix;
            port = src.port;
            boundaryName = src.boundaryName;
            qp = new QueryProcessor( nl, am );
            dType = src.dType;
            tree = 0;

            return *this;
        }

/*
1.3 Destructor

*/
        ~Partitioner( ) {
            if( qp && tree ) {
                qp->Destroy( tree, false );
                delete qp;
            }

            if( boundary ) {
                boundary->DeleteIfAllowed( );
            }

            if( matrix ) {
                delete matrix;
            }
        }

/*
1.6 ~shareBoundary~

Copies the boundary object to all workers.

*/
        bool shareBoundary( ) {

            cout << endl;
            cout << "Start: Bring boundary object to the workers ..." << endl;

            std::string query =
            "(share2 \"" + boundaryName + "\" (" + nl->ToString( boundaryType ) + 
            "( ptr " + nl->ToString( listutils::getPtrList( boundary ) ) + 
            ")) TRUE (drel2darray (" + nl->ToString( sourcedType ) +
            " (ptr " + nl->ToString( listutils::getPtrList( drel ) ) + 
            "))))";

            #ifdef DRELDEBUG
            cout << "query to share boundary" << endl;
            cout << query << endl;
            #endif

            Word result;
            if( !QueryProcessor::ExecuteQuery( query, result ) ) {
                cout << "ERROR: Bring boundary to the workers failed!" << endl;
                return false;
            }

            FText* res = ( FText* )result.addr;

            if( !res ) {
                cout << "ERROR: Bring boundary to the workers failed!" << endl;
                return false;
            }

            if( !res->IsDefined( ) ) {
                cout << "ERROR: Bring boundary to the workers failed!" << endl;
                delete res;
                return false;
            }

            cout << res->GetValue( ) << endl;

            delete res;

            cout << "Done. Boundary object is now on the workers!" << endl;

            return true;
        }

/*
1.7 ~repartition2DFMatrix~

Repartitions the drel to a DFMatrix.

*/
        bool repartition2DFMatrix( ) {

            cout << endl;
            cout << "Start: Create new partitioning on the workers "
                "as a DFMatrix ..." << endl;

            if( !shareBoundary( ) ) {
                return false;
            }

            ListExpr partitionTMDArray = nl->TwoElemList(
                getArrayType( ),
                nl->SymbolAtom( "dummy" ) );

            ListExpr partitionTMName = nl->TwoElemList(
                listutils::basicSymbol<CcString>( ),
                nl->StringAtom( "dummy" ) );

            ListExpr partitionTMInt = nl->TwoElemList(
                listutils::basicSymbol<CcInt>( ),
                nl->IntAtom( 0 ) );

            ListExpr resultType, query;
            int functions;

            // remove attribute original
            if( dType == replicated
             || dType == spatial2d
             || dType == spatial3d ) {

                functions = 2;

                ListExpr removeAttr = DRelHelpers::getRemovePartitonAttr( 
                    dType );

                ListExpr newTupleType = nl->TwoElemList(
                    listutils::basicSymbol<Tuple>( ),
                    DRelHelpers::removePartitionAttributes(
                        nl->Second( 
                            nl->Second( 
                                nl->Second( sourcedType ) ) ),
                        dType ) );
                
                ListExpr removeFilter = nl->TwoElemList(
                    nl->FourElemList(
                        nl->SymbolAtom( "map" ),
                        nl->Second( sourcedType ),
                        nl->Second( sourcedType ),
                        nl->TwoElemList(
                            listutils::basicSymbol<Stream<Tuple>>( ),
                            newTupleType ) ),
                    nl->FourElemList(
                        nl->SymbolAtom( "fun" ),
                        nl->TwoElemList( 
                            nl->SymbolAtom( "elem1_1" ),
                            nl->SymbolAtom( "FFR" ) ),
                        nl->TwoElemList( 
                            nl->SymbolAtom( "elem2_2" ),
                            nl->SymbolAtom( "FFR" ) ),
                        nl->ThreeElemList(
                            nl->SymbolAtom( "remove" ),
                            nl->ThreeElemList(
                                nl->SymbolAtom( "filter" ),
                                nl->TwoElemList( 
                                    nl->SymbolAtom( "feed" ),
                                    nl->SymbolAtom( "elem1_1" ) ),
                                nl->ThreeElemList(
                                    nl->SymbolAtom( "fun" ),
                                    nl->TwoElemList( 
                                        nl->SymbolAtom( "streamelem_3" ),
                                        nl->SymbolAtom( "STREAMELEM" ) ),
                                    nl->ThreeElemList(
                                        nl->SymbolAtom( "=" ),
                                        nl->ThreeElemList(
                                            nl->SymbolAtom( "attr" ),
                                            nl->SymbolAtom( "streamelem_3" ),
                                            nl->SymbolAtom( "Original" ) ),
                                        nl->BoolAtom( true ) ) ) ),
                            removeAttr ) ) );

                ListExpr partitionTMFun = nl->TwoElemList(
                    nl->FourElemList(
                        nl->SymbolAtom( "map" ),
                        newTupleType,
                        newTupleType,
                        listutils::basicSymbol<CcInt>( ) ),
                    nl->FourElemList(
                        nl->SymbolAtom( "fun" ),
                        nl->TwoElemList(
                            nl->SymbolAtom( "elem1_4" ),
                            nl->SymbolAtom( "FFR" ) ),
                         nl->TwoElemList(
                            nl->SymbolAtom( "elem2_5" ),
                            nl->SymbolAtom( "FFR" ) ),
                        nl->ThreeElemList(
                            nl->SymbolAtom( "getboundaryindex" ),
                            nl->SymbolAtom( boundaryName ),
                            nl->ThreeElemList(
                                nl->SymbolAtom( "attr" ),
                                nl->SymbolAtom( "elem2_5" ),
                                nl->SymbolAtom( attr ) ) ) ) );

                resultType = distributed2::partitionFTM( 
                    nl->FiveElemList(
                        partitionTMDArray,
                        partitionTMName,
                        removeFilter,
                        partitionTMFun,
                        partitionTMInt ) );

                query = nl->SixElemList(
                    nl->SymbolAtom( "partitionF" ),
                    createDRelPointerList( ), 
                    nl->StringAtom( "" ),
                    nl->Second( removeFilter ),
                    nl->FourElemList(
                        nl->SymbolAtom( "fun" ),
                        nl->TwoElemList(
                            nl->SymbolAtom( "elem1_4" ),
                            nl->SymbolAtom( "FFR" ) ),
                        nl->TwoElemList(
                            nl->SymbolAtom( "elem2_5" ),
                            nl->SymbolAtom( "FFR" ) ),
                        nl->ThreeElemList(
                            nl->SymbolAtom( "getboundaryindex" ),
                            createBoundaryPointerList( ),
                            nl->ThreeElemList(
                                nl->SymbolAtom( "attr" ),
                                nl->SymbolAtom( "elem2_5" ),
                                nl->SymbolAtom( attr ) ) ) ),
                    nl->IntAtom( 0 ) );
            }
            else {

                functions = 1;

                ListExpr partitionTMFun = nl->TwoElemList(
                    nl->ThreeElemList(
                        nl->SymbolAtom( "map" ),
                        nl->Second( nl->Second( sourcedType ) ),
                        listutils::basicSymbol<CcInt>( ) ),
                    nl->ThreeElemList(
                        nl->SymbolAtom( "fun" ),
                        nl->TwoElemList(
                            nl->SymbolAtom( "elem_1" ),
                            nl->SymbolAtom( "SUBSUBTYPE1" ) ),
                        nl->ThreeElemList(
                            nl->SymbolAtom( "getboundaryindex" ),
                            nl->SymbolAtom( boundaryName ),
                            nl->ThreeElemList(
                                nl->SymbolAtom( "attr" ),
                                nl->SymbolAtom( "elem_1" ),
                                nl->SymbolAtom( attr ) ) ) ) );

                resultType = distributed2::partitionTM( 
                    nl->FourElemList(
                        partitionTMDArray,
                        partitionTMName,
                        partitionTMFun,
                        partitionTMInt ) );

                query = nl->FiveElemList(
                    nl->SymbolAtom( "partition" ),
                    createDRelPointerList( ), 
                    nl->StringAtom( "" ),
                    nl->ThreeElemList(
                        nl->SymbolAtom( "fun" ),
                        nl->TwoElemList(
                            nl->SymbolAtom( "elem_1" ),
                            nl->SymbolAtom( "SUBSUBTYPE1" ) ),
                        nl->ThreeElemList(
                            nl->SymbolAtom( "getboundaryindex" ),
                            createBoundaryPointerList( ),
                            nl->ThreeElemList(
                                nl->SymbolAtom( "attr" ),
                                nl->SymbolAtom( "elem_1" ),
                                nl->SymbolAtom( attr ) ) ) ),
                    nl->IntAtom( 0 ) );
            }

            #ifdef DRELDEBUG
            cout << "partition query" << endl;
            cout << nl->ToString( query ) << endl;
            cout << "partitionTM result" << endl;
            cout << nl->ToString( resultType ) << endl;
            #endif

            if( !nl->HasLength( resultType, 3 ) ) {
                cout << "ERROR: Create new partitioning failed!" << endl;
                return false;
            }

            ListExpr matrixType = nl->Third( resultType );
            if( !distributed2::DFMatrix::checkType( matrixType ) ) {
                cout << "ERROR: Create new partitioning failed!" << endl;
                 return false;
            }

            if( !createPartitionOpTree( query ) ) {
                cout << "ERROR: Create new partitioning failed!" << endl;
                return false;
            }

            CcString* name = new CcString( true, "" );
            CcInt* newSize = new CcInt( true , 0 );
            Word result, local, dummy;

            if( functions == 1 ) {

                // first append value
                FText* fun = new FText( true, 
                    nl->TextValue( nl->First( nl->Second( resultType ) ) ) );

                ArgVector argVec = { drel, name, dummy, newSize, fun };

                // call partition value mapping
                distributed2::partitionVMT<T>(
                    argVec, result, 0, local, tree );

                delete fun;
            }
            else {

                // first append value
                FText* fun1 = new FText( true, 
                    nl->TextValue( nl->First( nl->Second( resultType ) ) ) );
                FText* fun2 = new FText( true, 
                    nl->TextValue( nl->Second( nl->Second( resultType ) ) ) );

                ArgVector argVec = { drel, name, dummy, dummy, newSize, 
                    fun1, fun2 };

                // call partition value mapping
                distributed2::partitionVMT<T>(
                    argVec, result, 0, local, tree );

                delete fun1;
                delete fun2;
            }

            delete name;
            delete newSize;

            matrix = ( distributed2::DFMatrix* )result.addr;

            if( !matrix ) {
                cout << "ERROR: Create new partitioning failed!" << endl;
                qp->Destroy( tree, false );
                delete qp;
                matrix = 0;
                return false;
            }

            if( !matrix->IsDefined( ) ) {
                cout << "ERROR: Create new partitioning failed!" << endl;
                qp->Destroy( tree, false );
                delete qp;
                matrix = 0;
                return false;
            }

            cout << "Done. New partitioning is created on the workers!" 
                 << endl;

            return true;
        }

/*
1.10.2 ~getBoundary~

*/
        collection::Collection* getBoundary( ) {

            return boundary;
        }

/*
1.10.3 ~getDFMatrix~

*/
        distributed2::DFMatrix* getDFMatrix( ) {

            return matrix;
        }

/*
1.10.4 ~getArrayType~

*/
        ListExpr getArrayType( ) {
            ListExpr arrayType = nl->TheEmptyList();

            if( nl->ToString( nl->First( sourcedType ) ) == 
                DRel::BasicType( ) ) {

                arrayType = nl->TwoElemList(
                    listutils::basicSymbol<distributed2::DArray>( ),
                    nl->Second( sourcedType ) );
            }
            else if( nl->ToString( nl->First( sourcedType ) ) == 
                DFRel::BasicType( ) ) {

                arrayType = nl->TwoElemList(
                    listutils::basicSymbol<distributed2::DFArray>( ),
                    nl->Second( sourcedType ) );
            }

            return arrayType;
        }

/*
1.10.5 ~getMatrixType~

*/
        ListExpr getMatrixType( ) {

            if( dType == spatial2d || dType == spatial3d ) {

                ListExpr relType = nl->Second( nl->Second( sourcedType ) );

                ListExpr attrList = nl->Second( relType );
                attrList = DRelHelpers::removeAttrFromAttrList( 
                    attrList, "Original" );
                attrList = DRelHelpers::removeAttrFromAttrList( 
                    attrList, "Cell" );

                relType = nl->TwoElemList(
                    listutils::basicSymbol<Relation>( ),
                    nl->TwoElemList(
                        listutils::basicSymbol<Tuple>( ),
                        attrList ) );

                return nl->TwoElemList(
                    listutils::basicSymbol<distributed2::DFMatrix>( ),
                    relType );
            }
            
            return nl->TwoElemList(
                listutils::basicSymbol<distributed2::DFMatrix>( ),
                nl->Second( sourcedType ) );
        }

/*
1.11 set functions

1.11.1 ~setBoundary~

*/

        void setBoundary( collection::Collection* _boundary ) {

            if( boundary == _boundary ) {
                return;
            }

            if( boundary ) {
                boundary->DeleteIfAllowed( );
            }

            boundary = _boundary;
        }

    private:


/*
1.14 ~createDRelPointerList~

*/
        ListExpr createDRelPointerList( ) {

            return nl->TwoElemList(
                getArrayType( ),
                nl->TwoElemList(
                    nl->SymbolAtom( "ptr" ),
                    listutils::getPtrList( drel ) ) );
        }

/*
1.15 ~createBoundaryPointerList~

*/
        ListExpr createBoundaryPointerList( ) {

            return nl->TwoElemList(
                boundaryType,
                nl->TwoElemList(
                    nl->SymbolAtom( "ptr" ),
                    listutils::getPtrList( boundary ) ) );
        }

/*
1.16 ~createPartitionOpTree~

*/
        bool createPartitionOpTree( ListExpr query ) {

            bool correct = false;
            bool evaluable = false;
            bool defined = false;
            bool isFunction = false;
            ListExpr resultType;

            qp->Construct(
                query,
                correct,
                evaluable,
                defined,
                isFunction,
                tree,
                resultType );

            return correct && evaluable && defined;
        }

/*
1.17 Members

*/
        std::string attr;
        ListExpr boundaryType;
        R* drel;
        ListExpr sourcedType;
        collection::Collection* boundary;
        QueryProcessor* qp;
        distributed2::DFMatrix* matrix;
        int port;
        std::string boundaryName;
        distributionType dType;
        OpTree tree;

    };
    
} // end of namespace drel

#endif // _Partitioner_h_
