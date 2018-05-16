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


1 ~DRelHelpers~

Class with usefull helper functions for the DRelAlgebra

*/
#include "DRelHelpers.h"

#include "ListUtils.h"
#include "StandardTypes.h"
#include "QueryProcessor.h"

#include "Algebras/MainMemory2/MainMemoryExt.h"

using namespace std;

namespace mm2algebra {
    MemoryAVLObject* createAVLTree( MemoryRelObject* mmrel, int attrPos,
        ListExpr type );
}

namespace drel {

    /*
    1.1 ~findAttribute~

    Return true if the attribute name is found in the attribute list. False 
    otherwise.

    */
    bool DRelHelpers::findAttribute( 
        ListExpr attrList, const string& name, ListExpr& type ) {

        return !( listutils::findAttribute( attrList, name, type ) == 0 );
    }

    /*
    1.2 ~countRecords~

    Counts the records in a given relation. If an error occour false is 
    returned. True otherwise. The number of records is records.

    */
    bool DRelHelpers::countRecords( 
        const string attrName, const string relName, int& records ) {

        string query = "( count ( filter( feed " + relName + " ) "
            "( fun( t TUPLE ) ( isdefined (attr t " + attrName + " ) ) ) ) )";
        Word result;
        if( !QueryProcessor::ExecuteQuery( query, result ) ) {
            cout << "ExecuteError of " + query << endl;
            records = -1;
            return false;
        }

        CcInt* number = ( CcInt* )result.addr;
        if( !number->IsDefined( ) ) {
            number->DeleteIfAllowed( );
            records = -1;
            return false;
        }

        records = number->GetIntval( );
        number->DeleteIfAllowed( );
        result.setAddr( 0 );

        return true;
    }

    /*
    1.3 ~createMemSample~

    Creates for a given relation name and a given attribute name a sample and 
    returns a memory pointer to a inmemory boundary relation. The boundary 
    relation is build as a sample of the projection to the attribute and 
    a number of the block. For example the sample boundary relation of the 
    relation plz with the attribute PLZ looks like 
    rel(tuple((PLZ int) (D int))). D is here the attribute name of the 
    numbering attribute.

    */
    mm2algebra::MPointer* DRelHelpers::createMemSample(
        const string relation, const string attr ) {
        string query = 
            "(mconsume "
                "(addcounter"
                    "(nth"
                        "(sortby"
                            "(project"
                                "(nth"
                                    "(filter"
                                        "(feed " + relation + ")"
                                        "(fun(t TUPLE)"
                                        "(isdefined(attr t " + attr + ") ) )"
                                    ")"
                                "9 FALSE)"
                            "(" + attr + "))"
                        "(" + attr + "))"
                    "100 TRUE)"
                "D 1)"
            ")";

        Word result;
        if( !QueryProcessor::ExecuteQuery( query, result ) ) {
            cout << "ExecuteError of " + query << endl;
            return 0;
        }

        mm2algebra::MPointer* sample = ( mm2algebra::MPointer* )result.addr;
        result.setAddr( 0 );

        return sample;
    }

    /*
    1.4 ~createAVLtree~

    Creates an inmemory AVLTree for an inmemory relation pointer.
    The parameters are the inmemory relation, the type of the relation as 
    NestedList and the position of the attribute in the relation to create 
    the avl tree. Default for the attribute position is 2. The reltype has 
    to look like tuple((PLZ int) (D int))

    */
    mm2algebra::MPointer* DRelHelpers::createAVLtree( 
        mm2algebra::MPointer* mmrelp, int attrpos ) {

        ListExpr attrList;
        if(! nl->ReadFromString(
            mmrelp->GetValue( )->getObjectTypeExpr( ), attrList ) ) {
            return 0;
        }

        attrList = nl->Second( nl->Second( nl->Second( attrList ) ) );
        cout << "attrList" << endl;
        cout << nl->ToString( attrList ) << endl;

        if( attrpos < 0 || !( nl->ListLength( attrList ) <= attrpos + 1 ) ) {
            cout << "attrpos passt nicht" << endl;
            return 0;
        }

        if( !listutils::isAttrList( attrList ) ) {
            cout << "no attr list" << endl;
            return 0;
        }

        for( int i = 1 ; i < attrpos ; i++ ) {
            attrList = nl->Rest( attrList );
        }
        ListExpr type = nl->Second( nl->First( attrList ) );

        mm2algebra::MemoryRelObject* mmrel =
            ( mm2algebra::MemoryRelObject* )mmrelp->GetValue( );
        mm2algebra::MemoryAVLObject* obj = 
            mm2algebra::createAVLTree( mmrel, attrpos, type );
        mm2algebra::MPointer* avltreep = 
            new mm2algebra::MPointer( ( mm2algebra::MemoryObject* )0, true );
        avltreep->setPointer( obj );

        return avltreep;
    }
    
    /*
    1.5 ~createSampleMemList~

    Creates a ListExpr for the OpTree to create a inmemory sample relation 
    of a given relation for an specifc attribute.
    For Example:
    (matchbelow2 (

    mcreateAVLtree (
    mconsume (
        addcounter (
            nth (
                sortby (
                    project (
                        nth (
                            filter (feed plz) 
                            (fun (t TUPLE) (isdefined (attr t PLZ)))) 
                            9 FALSE)
                        (PLZ)) 
                (PLZ)) 
             100 TRUE) 
         D 1))
    PLZ)
    (mconsume (
        addcounter (
            nth (
                sortby (
                    project (
                        nth (
                            filter (feed plz) 
                            (fun (t TUPLE) (isdefined (attr t PLZ)))) 
                            9 FALSE)
                        (PLZ)) 
                (PLZ)) 
             100 TRUE) 
         D 1))
    )

    */
    ListExpr DRelHelpers::createSampleMemList(
        const string relName, const string attrName ) {
        
        // ListExpr to create the mconsume tree to create 
        // an in memory boundary relation
        ListExpr funList =
            nl->TwoElemList(
                nl->SymbolAtom( "mconsume" ),
                nl->FourElemList(
                    nl->SymbolAtom( "addcounter" ),
                    nl->FourElemList(
                        nl->SymbolAtom( "nth" ),
                        nl->ThreeElemList(
                            nl->SymbolAtom( "sortby" ),
                            nl->ThreeElemList(
                                nl->SymbolAtom( "project" ),
                                nl->FourElemList(
                                    nl->SymbolAtom( "nth" ),
                                    nl->ThreeElemList(
                                        nl->SymbolAtom( "filter" ),
                                        nl->TwoElemList(
                                            nl->SymbolAtom( "feed" ),
                                            nl->SymbolAtom( relName )
                                        ),
                                        nl->ThreeElemList(
                                            nl->SymbolAtom( "fun" ),
                                            nl->TwoElemList(
                                                nl->SymbolAtom( "t" ),
                                                nl->SymbolAtom( "TUPLE" )
                                            ),
                                            nl->TwoElemList(
                                                nl->SymbolAtom( "isdefined" ),
                                                nl->ThreeElemList(
                                                    nl->SymbolAtom( "attr" ),
                                                    nl->SymbolAtom( "t" ),
                                                    nl->SymbolAtom( attrName )
                                                )
                                            )
                                        )
                                    ),
                                    nl->IntAtom( 9 ),
                                    nl->BoolAtom( false )
                                ),
                                nl->OneElemList( nl->SymbolAtom( attrName ) )
                            ),
                            nl->OneElemList( nl->SymbolAtom( attrName ) )
                        ),
                        nl->IntAtom( 100 ),
                        nl->BoolAtom( true )
                    ),
                    nl->SymbolAtom( "D" ),
                    nl->IntAtom( 1 )
                )
            );

        // create the tree for matchbelow2 to match the tuples to the boundary
        funList = 
            nl->ThreeElemList(
                nl->SymbolAtom( "fun" ),
                nl->TwoElemList(
                    nl->SymbolAtom( "z" ),
                    nl->SymbolAtom( "TUPLE" )
                ),
                nl->SixElemList(
                    nl->SymbolAtom( "matchbelow2" ),
                    nl->ThreeElemList(
                        nl->SymbolAtom( "mcreateAVLtree" ),
                        funList,
                        nl->SymbolAtom( attrName ) ),
                    funList,
                    nl->ThreeElemList(
                        nl->SymbolAtom( "attr" ),
                        nl->SymbolAtom( "z" ),
                        nl->SymbolAtom( attrName )
                    ),
                    nl->SymbolAtom( "D" ),
                    nl->IntAtom( 0 )
                )
        );

        return funList;
    }

    /*
    1.6 ~computeSampleSize~

    Algorithm to compute the sample size.

    */
    int DRelHelpers::computeSampleSize( 
        const int relSize, const int arraySize ) {

        if( relSize <= arraySize ) {
            return relSize;
        }

        int sampleSize = relSize / arraySize;
        if( sampleSize > 8000 ) {
            return 8000;
        }

        return sampleSize;
    }

    /*
    1.7 ~computeFractionSample~

    Algorithm to compute the fraction of a relation to get a sample like 
    every nth record.

    */
    int DRelHelpers::computeFractionSample( 
        const int relSize, const int arraySize ) {

        int sampleSize = computeSampleSize( relSize, arraySize );
        if( arraySize > sampleSize ) {
            return 1;
        }

        return relSize/sampleSize;
    }

    /*bool DRelHelpers::createBoundaries( 
        string attrName, string relName, int records ) {

        string query = "( count ( filter( feed " + relName + " ) 
        ( fun( t TUPLE ) ( isdefined (attr t " + attrName + " ) ) ) ) )";
        Word result;
        if( !QueryProcessor::ExecuteQuery( query, result ) ) {
            cout << "ExecuteError of " + query << endl;
            records = -1;
            return false;
        }

        CcInt* number = ( CcInt* )result.addr;
        if( !number->IsDefined( ) ) {
            number->DeleteIfAllowed( );
            records = -1;
            return false;
        }

        records = number->GetIntval( );
        number->DeleteIfAllowed( );
        result.setAddr( 0 );

        return true;
    }*/

} // end of namespace drel