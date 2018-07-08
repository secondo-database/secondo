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

1 Helper class Implementation

*/
//#define DRELDEBUG

#include "Algebras/Temporal/TemporalAlgebra.h"
#include "DistributeOpHelper.h"

extern NestedList* nl;

using namespace distributed2;

namespace drel {

    /*
    1 ~createStreamOpTree~

    Creates a new operator tree to open a stream for a relation. For the
    relation the type of the relation is nessecarry as a ListExpr. The
    user is responsible for the right type of the relation.

    */
    OpTree DistributeOpHelper::createStreamOpTree(
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
    OpTree DistributeOpHelper::createReplicationOpTree(
        QueryProcessor* qps, Relation* rel, ListExpr relType,
        int size ) {

        #ifdef DRELDEBUG
        cout << "createStreamCellGridOpTree" << endl;
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
    OpTree DistributeOpHelper::createStreamCellGridOpTree(
        QueryProcessor* qps, Relation* rel, ListExpr relType,
        string attrName, T* grid ) {

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

    template OpTree DistributeOpHelper::
        createStreamCellGridOpTree<temporalalgebra::CellGrid2D>(
            QueryProcessor* qps, Relation* rel, ListExpr relType,
            string attrName, temporalalgebra::CellGrid2D* grid );
    
    template OpTree DistributeOpHelper::
        createStreamCellGridOpTree<temporalalgebra::CellGrid<3>>(
            QueryProcessor* qps, Relation* rel, ListExpr relType,
            string attrName, temporalalgebra::CellGrid<3>* grid );

    /*
    4 ~createCellGrid~

    Generates a optimal grid for a given relation with a given type. The grid
    will be computed for a given attribute name and a given size of the
    target array.

    */
    template<class T>
    T* DistributeOpHelper::createCellGrid(
        Relation* rel, ListExpr relType, string attrName, int size ) {

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

    template temporalalgebra::CellGrid2D*
        DistributeOpHelper::createCellGrid<temporalalgebra::CellGrid2D>(
        Relation* rel, ListExpr relType, string attrName, int size );

    template temporalalgebra::CellGrid<3>*
        DistributeOpHelper::createCellGrid<temporalalgebra::CellGrid<3>>(
        Relation* rel, ListExpr relType, string attrName, int size );

} // end of namespace drel