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
#ifndef _PartitionerS_h_
#define _PartitionerS_h_

#include "Algebras/Collection/CollectionAlgebra.h"
#include "Algebras/Stream/Stream.h"
#include "DRelHelpers.h"
#include "DRel.h"
#include "Algebras/Distributed2/CommandLogger.h"
#include "Algebras/Distributed2/Distributed2Algebra.h"

#include "Algebras/Distributed2/fsrel.h"

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
    template<class R, class S, class T>
    class PartitionerS {

    public:
/*
1.1 Constructors

*/
        PartitionerS( std::string _attr, 
                      ListExpr _gridType, 
                      R* _drel, 
                      ListExpr _sourcedType, 
                      T* _grid, 
                      int _port, 
                      std::string _gridName ) :
            attr( _attr ), gridType( _gridType ), drel( _drel ), 
            sourcedType( _sourcedType ), grid( _grid ), matrix( 0 ), 
            port( _port ), gridName( _gridName ),
            qp( new QueryProcessor( nl, am ) ), tree( 0 ) {

            grid->Copy( );
            dType = drel->getDistType( )->getDistType( );
        }

        PartitionerS( std::string _attr, ListExpr _gridType, R* _drel, 
            ListExpr _sourcedType, T* _grid, 
            int _port ) :
            PartitionerS( _attr, _gridType, _drel, _sourcedType, _grid,
            _port, distributed2::algInstance->getTempName( ) ) {

            grid->Copy( );
            dType = drel->getDistType( )->getDistType( );
        }


        PartitionerS( std::string _attr, ListExpr _gridType, R* _drel, 
            ListExpr _sourcedType, int _port ) :  
            attr( _attr ), gridType( _gridType ), drel( _drel ), 
            sourcedType( _sourcedType ), grid( 0 ), matrix( 0 ), 
            port( _port ), 
            gridName( distributed2::algInstance->getTempName( ) ),
            qp( new QueryProcessor( nl, am ) ), tree( 0 ) {

            dType = drel->getDistType( )->getDistType( );
        }
/*
1.2 Copy-Constructor

*/

        PartitionerS( const PartitionerS& src )  = delete;

/*
1.2 Assignment operator

*/
        PartitionerS& operator=( const PartitionerS& src )  = delete;


/*
1.3 Destructor

*/
        ~PartitionerS( ) {

            if( qp && tree ) {
                qp->Destroy( tree, false );
            }
            if(qp){
              delete qp;
            }

            if( grid ) {
                grid->DeleteIfAllowed( );
            }

            if( matrix ) {
                delete matrix;
            }
        }

/*
1.6 ~sharegrid~

Copies the grid object to all workers.

*/
        bool sharegrid( ) {

            cout << endl;
            cout << "Start: Bring grid object to the workers ..." << endl;

            if( !grid ) {
                if( !computeGrid( ) ){
                    return false;
                }
            }

            std::string query =
            "(share2 \"" + gridName + "\" (" + nl->ToString( gridType ) + 
            "( ptr " + nl->ToString( listutils::getPtrList( grid ) ) + 
            ")) TRUE (drel2darray (" + nl->ToString( sourcedType ) +
            " (ptr " + nl->ToString( listutils::getPtrList( drel ) ) + 
            "))))";

            #ifdef DRELDEBUG
            cout << "query to share grid" << endl;
            cout << query << endl;
            #endif

            Word result;
            if( !QueryProcessor::ExecuteQuery( query, result ) ) {
                cout << "ERROR: Bring grid to the workers failed!" << endl;
                return false;
            }

            FText* res = ( FText* )result.addr;

            if( !res ) {
                cout << "ERROR: Bring grid to the workers failed!" << endl;
                return false;
            }

            if( !res->IsDefined( ) ) {
                cout << "ERROR: Bring grid to the workers failed!" << endl;
                delete res;
                return false;
            }

            cout << res->GetValue( ) << endl;

            delete res;

            cout << "Done. grid object is now on the workers!" << endl;

            return true;
        }

        bool computeGrid( ) {

            std::string gridCollect = 
                "(rect2cellgrid (collect_box (transformstream "
                "(dsummarize (dmap " + nl->ToString(
                DRelHelpers::createdrel2darray( sourcedType, drel ) ) +
                " \"\" (fun (dmapelem_1 ARRAYFUNARG1) "
                "(projectextend (feed dmapelem_1) "
                "() ((Box (fun (tuple_2 TUPLE) "
                "(bbox (attr tuple_2 " + attr +"))))))))))"
                " TRUE) 37)";

            ListExpr query;
            nl->ReadFromString( gridCollect, query );

            std::string typeString, errorString;
            bool correct = false;
            bool evaluable = false;
            bool defined = false;
            bool isFunction = false;
            Word result;
            bool resBool = QueryProcessor::ExecuteQuery( query, 
                result, typeString, errorString, correct, evaluable,
                defined, isFunction );

            if( !(correct && evaluable && defined && resBool ) ) {
                return false;
            }

            grid = static_cast<T*>( result.addr );

            return true;
        }

/*
1.7 ~repartition2DFMatrix~

Repartitions the drel to a DFMatrix.

*/
        bool repartition2DFMatrix( ) {

            ListExpr partitionTMDArray = nl->TwoElemList(
                getArrayType( ),
                nl->SymbolAtom( "dummy" ) );

            ListExpr partitionTMName = nl->TwoElemList(
                listutils::basicSymbol<CcString>( ),
                nl->StringAtom( "dummy" ) );

            ListExpr partitionTMInt = nl->TwoElemList(
                listutils::basicSymbol<CcInt>( ),
                nl->IntAtom( 0 ) );

            ListExpr newTupleType = nl->TwoElemList(
                listutils::basicSymbol<Tuple>( ),
                DRelHelpers::addPartitionAttributes(
                    nl->Second( 
                        nl->Second( 
                            nl->Second( sourcedType ) ) ) ) );
            
            ListExpr relType =   DRel::checkType( sourcedType )
                        ? nl->Second( sourcedType )
                        : nl->TwoElemList(
                               listutils::basicSymbol<distributed2::fsrel>(),
                               nl->Second( nl->Second( sourcedType ) ) );
            
            ListExpr extendStreamMap = nl->FourElemList(
                    nl->SymbolAtom( "map" ), 
                    relType, 
                    relType, 
                    nl->TwoElemList(
                            listutils::basicSymbol<Stream<Tuple>>( ),
                            newTupleType ) );
            
            /*  // original version
            ListExpr extendStreamMap = nl->FourElemList(
                    nl->SymbolAtom( "map" ), 
                    nl->Second( sourcedType ),
                    nl->Second( sourcedType ),
                    nl->TwoElemList(
                            listutils::basicSymbol<Stream<Tuple>>( ),
                            newTupleType ) );
            */

            std::string extendStreamS;
            ListExpr partitionTMFun;

            if( dType == spatial2d || dType == spatial3d) {
                extendStreamS = "(fun (elem1_1 FFR) (elem2_2 FFR) "
                    "(extend (extendstream (remove (filter (feed elem1_1) "
                    "(fun (streamelem_3 STREAMELEM) (= (attr streamelem_3 "
                    "Original) TRUE))) (Original Cell)) ((Cell (fun "
                    "(tuple_4 TUPLE) (cellnumber (bbox (attr tuple_4 "
                    + attr + "))"
                    " " + gridName + "))))) ((Original (fun (tuple_5 TUPLE) "
                    "(= (attr tuple_5 Cell) (extract (transformstream "
                    "(cellnumber (bbox (attr tuple_4 "
                    + attr + ")) " + gridName + ")) Elem)))))))";

                partitionTMFun = nl->TwoElemList(
                    nl->FourElemList(
                        nl->SymbolAtom( "map" ),
                        newTupleType,
                        newTupleType,
                        listutils::basicSymbol<CcInt>( ) ),
                    nl->FourElemList(
                        nl->SymbolAtom( "fun" ),
                        nl->TwoElemList(
                            nl->SymbolAtom( "elem1_6" ),
                            nl->SymbolAtom( "FFR" ) ),
                            nl->TwoElemList(
                            nl->SymbolAtom( "elem2_7" ),
                            nl->SymbolAtom( "FFR" ) ),
                        nl->ThreeElemList(
                        nl->SymbolAtom( "attr" ),
                        nl->SymbolAtom( "elem2_7" ),
                        nl->SymbolAtom( "Cell" ) ) ) );
            }
            else {
                extendStreamS = "(fun (elem1_1 FFR) (elem2_2 FFR) "
                    "(extend (extendstream (feed elem1_1)"
                    " ((Cell (fun (tuple_3 TUPLE) "
                    "(cellnumber (bbox (attr tuple_3 "
                    + attr + ")) " + gridName + "))))) "
                    "((Original (fun (tuple_4 TUPLE) (= (attr tuple_4 Cell) "
                    "(extract (transformstream (cellnumber (bbox "
                    "(attr tuple_4 " + attr + ")) " + gridName + ")) "
                    "Elem)))))))";

                partitionTMFun = nl->TwoElemList(
                    nl->FourElemList(
                        nl->SymbolAtom( "map" ),
                        newTupleType,
                        newTupleType,
                        listutils::basicSymbol<CcInt>( ) ),
                    nl->FourElemList(
                        nl->SymbolAtom( "fun" ),
                        nl->TwoElemList(
                            nl->SymbolAtom( "elem1_5" ),
                            nl->SymbolAtom( "FFR" ) ),
                            nl->TwoElemList(
                            nl->SymbolAtom( "elem2_6" ),
                            nl->SymbolAtom( "FFR" ) ),
                        nl->ThreeElemList(
                        nl->SymbolAtom( "attr" ),
                        nl->SymbolAtom( "elem2_6" ),
                        nl->SymbolAtom( "Cell" ) ) ) );
            }
                
            ListExpr extendStreamFun;
            nl->ReadFromString( extendStreamS, extendStreamFun );

            extendStreamFun = nl->TwoElemList(
                extendStreamMap, extendStreamFun );

            ListExpr resultType = distributed2::partitionFTM( 
                nl->FiveElemList(
                    partitionTMDArray,
                    partitionTMName,
                    extendStreamFun,
                    partitionTMFun,
                    partitionTMInt ) );

            std::string queryS;
            if( dType == spatial2d || dType == spatial3d) {

                queryS = "(partitionF " + nl->ToString(
                    DRelHelpers::createdrel2darray( sourcedType, drel ) ) +
                    " \"\" (fun (elem1_1 FFR) (elem2_2 FFR) (extend "
                    "(extendstream (remove (filter (feed elem1_1) "
                    "(fun (streamelem_3 STREAMELEM) (= (attr streamelem_3 "
                    "Original) TRUE))) (Original Cell)) ((Cell (fun (tuple_4 "
                    "TUPLE) (cellnumber (bbox (attr tuple_4 " + attr + ")) "
                     + nl->ToString( createGridPointerList( ) ) + 
                    "))))) ((Original (fun (tuple_5 TUPLE) (= (attr "
                    "tuple_5 Cell) (extract (transformstream (cellnumber "
                    "(bbox (attr tuple_5 " + attr + ")) " + nl->ToString( 
                        createGridPointerList( ) ) + 
                    ")) Elem))))))) "
                    "(fun (elem1_6 FFR) (elem2_7 FFR) (attr elem2_7 Cell)) 0)";
            }
            else { // anpassen
                queryS = "(partitionF " + nl->ToString(
                    DRelHelpers::createdrel2darray( sourcedType, drel ) ) + 
                    " \"\" (fun (elem1_1 FFR) (elem2_2 FFR) (extend "
                    "(extendstream (feed elem1_1) ((Cell (fun (tuple_3 TUPLE)"
                    " (cellnumber (bbox (attr tuple_3 " + attr + ")) " 
                    + nl->ToString( createGridPointerList( ) ) + ")))))"
                    " ((Original (fun (tuple_4 TUPLE) (= (attr tuple_4 Cell)"
                    " (extract (transformstream (cellnumber (bbox "
                    "(attr tuple_4 " + attr + ")) " + 
                    nl->ToString( createGridPointerList( ) ) + ")) Elem"
                    "))))))) (fun (elem1_5 FFR) (elem2_6 FFR) "
                    "(attr elem2_6 Cell)) 0)";
            }

            ListExpr query;
            nl->ReadFromString( queryS, query );

            if( !nl->HasLength( resultType, 3 ) ) {
                cout << "ERROR: Create new partitioning failed!1" << endl;
                return false;
            }

            ListExpr matrixType = nl->Third( resultType );
            if( !distributed2::DFMatrix::checkType( matrixType ) ) {
                cout << "ERROR: Create new partitioning failed!2" << endl;
                 return false;
            }

            if( !createPartitionOpTree( query ) ) {
                cout << "ERROR: Create new partitioning failed!3" << endl;
                return false;
            }

            CcString* name = new CcString( true, "" );
            CcInt* newSize = new CcInt( true , 0 );
            Word result, local, dummy;

            FText* fun1 = new FText( true, 
                nl->TextValue( nl->First( nl->Second( resultType ) ) ) );
            FText* fun2 = new FText( true, 
                nl->TextValue( nl->Second( nl->Second( resultType ) ) ) );

            ArgVector argVec = { drel, name, dummy, dummy, newSize, 
                fun1, fun2 };

            // call partition value mapping
            distributed2::partitionVMT<S>(
                argVec, result, 0, local, tree );

            delete fun1;
            delete fun2;

            delete name;
            delete newSize;

            matrix = ( distributed2::DFMatrix* )result.addr;

            if( !matrix ) {
                cout << "ERROR: Create new partitioning failed!4" << endl;
                qp->Destroy( tree, false );
                delete qp;
                matrix = 0;
                return false;
            }

            if( !matrix->IsDefined( ) ) {
                cout << "ERROR: Create new partitioning failed!5" << endl;
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
1.10.2 ~getgrid~

*/
        T* getGrid( ) {
            return grid;
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
            ListExpr arrayType=nl->TheEmptyList();

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

            return nl->TwoElemList(
                listutils::basicSymbol<distributed2::DFMatrix>( ),
                nl->TwoElemList(
                    listutils::basicSymbol<Relation>( ),
                    nl->TwoElemList(
                        listutils::basicSymbol<Tuple>( ),
                        DRelHelpers::addPartitionAttributes(
                            nl->Second( 
                                nl->Second( 
                                    nl->Second( sourcedType ) ) ) ) ) ) );
        }

/*
1.11 set functions

1.11.1 ~setgrid~

*/

        void setgrid( T* _grid ) {

            if( grid == _grid ) {
                return;
            }

            if( grid ) {
                grid->DeleteIfAllowed( );
            }

            grid = _grid;

            grid->Copy( );
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
1.15 ~createGridPointerList~

*/
        ListExpr createGridPointerList( ) {

            return nl->TwoElemList(
                gridType,
                nl->TwoElemList(
                    nl->SymbolAtom( "ptr" ),
                    listutils::getPtrList( grid ) ) );
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
        ListExpr gridType;
        R* drel;
        ListExpr sourcedType;
        T* grid;
        distributed2::DFMatrix* matrix;
        int port;
        std::string gridName;
        distributionType dType;
        QueryProcessor* qp;
        OpTree tree;

    };
    
} // end of namespace drel

#endif // _PartitionerS_h_
