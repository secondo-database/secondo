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



1 Implementation of the secondo operator createboundary

*/
#include <iostream>

#include "NestedList.h"
#include "ListUtils.h"
#include "QueryProcessor.h"
#include "StandardTypes.h"
#include "Algebras/Stream/Stream.h"

#include "Algebras/FText/FTextAlgebra.h"

#include "Algebras/Distributed2/CommandLogger.h"
#include "Algebras/Distributed2/Distributed2Algebra.h"
#include "Algebras/Spatial/SpatialAlgebra.h"
#include "Algebras/Collection/CollectionAlgebra.h"

#include "DRelHelpers.h"

extern NestedList* nl;
extern QueryProcessor* qp;

namespace drel {

/*
1.1 Type Mapping

Get a relation, an attribute and a size object.

*/
    ListExpr createboundaryTM( ListExpr args ) {

        std::string err = "rel(tuple(X)) x attr x int expected";

        if( nl->HasLength( args, 3 ) ) {

            if( !DRelHelpers::isListOfTwoElemLists( args ) ) {
                return listutils::typeError( err + ": internal error" );
            }

            if( !Relation::checkType( nl->First( nl->First( args ) ) ) ) {
                return listutils::typeError( err +
                    ": first argument is not a relation" );
            }
        
            if( ! listutils::isSymbol( nl->Second( nl->Second( args ) ) ) ) {
                return listutils::typeError( err + 
                    ": second argument has to be an attribute name" );
            }
        
            if( ! CcInt::checkType( nl->First( nl->Third( args ) ) ) ) {
                return listutils::typeError( err + 
                    ": third argument has to be an integer" );
            }

            if( ! nl->IsAtom( nl->Second( nl->Third( args ) ) ) ) {
                return listutils::typeError( err +
                    ": internal error" );
            }

            if( nl->AtomType( nl->Second( nl->Third( args ) ) ) != IntType ) {
                return listutils::typeError( err +
                    ": internal error" );
            }

            if( ! ( nl->IntValue( nl->Second( nl->Third( args ) ) ) > 0 ) ) {
                return listutils::typeError( err +
                    ": third argument has to be an integer greater than 0" );
            }
        }
        else if( nl->HasLength( args, 4 ) ) {

            if( !DRelHelpers::isListOfTwoElemLists( args ) ) {
                return listutils::typeError( err + ": internal error" );
            }

            if( !listutils::isTupleStream( nl->First( nl->First( args ) ) ) ) {
                return listutils::typeError( err +
                    ": first argument is not a tuple stream" );
            }

            if( !listutils::isSymbol( nl->Second( nl->Second( args ) ) ) ) {
                return listutils::typeError( err +
                    ": second argument has to be an attribute name" );
            }

            if( !CcInt::checkType( nl->First( nl->Third( args ) ) ) ) {
                return listutils::typeError( err +
                    ": third argument has to be an integer" );
            }

            if( !nl->IsAtom( nl->Second( nl->Third( args ) ) ) ) {
                return listutils::typeError( err +
                    ": internal error" );
            }

            if( nl->AtomType( nl->Second( nl->Third( args ) ) ) != IntType ) {
                return listutils::typeError( err +
                    ": internal error" );
            }

            if( !( nl->IntValue( nl->Second( nl->Third( args ) ) ) > 0 ) ) {
                return listutils::typeError( err +
                    ": third argument has to be an integer greater than 0" );
            }

            if( !CcInt::checkType( nl->First( nl->Fourth( args ) ) ) ) {
                return listutils::typeError( err +
                    ": fourth argument has to be an integer" );
            }

            if( !nl->IsAtom( nl->Second( nl->Fourth( args ) ) ) ) {
                return listutils::typeError( err +
                    ": internal error" );
            }

            if( nl->AtomType( nl->Second( nl->Fourth( args ) ) ) != IntType ) {
                return listutils::typeError( err +
                    ": internal error" );
            }

            if( !( nl->IntValue( nl->Second( nl->Fourth( args ) ) ) > 0 ) ) {
                return listutils::typeError( err +
                    ": fourth argument has to be an integer greater than 0" );
            }
        }
        else {
            return listutils::typeError( err +
                ": three or four arguments are expected" );
        }

        std::string attrName = nl->SymbolValue(
            nl->Second( nl->Second( args ) ) );
        ListExpr attrList = nl->Second( nl->Second( 
                            nl->First( nl->First( args ) ) ) );
        ListExpr attrType;

        int pos = listutils::findAttribute( attrList, attrName, attrType );

        if( pos == 0 ) {
            return listutils::typeError(
                err + ": attr name " + attrName + " not found" );
        }

        ListExpr resType = nl->TwoElemList(
            nl->SymbolAtom( Vector::BasicType( ) ),
            attrType );

        ListExpr appendList = nl->OneElemList(
            nl->IntAtom( pos - 1 ) );

        return nl->ThreeElemList( nl->SymbolAtom( Symbols::APPEND( ) ),
            appendList,
            resType );
    }

/*
1.2 Value Mapping

Creates a boundary object by determinating the boundaries with a sample 
of the realation.

*/
    template<bool instream>
    int createboundaryVMT( Word* args, Word& result, int message,
        Word& local, Supplier s ) {

        result = qp->ResultStorage( s );
        collection::Collection* resultColl =
            static_cast<collection::Collection*>( result.addr );
        resultColl->Clear( );
        resultColl->SetDefined( true );

        CcInt* size = ( CcInt* )args[ 2 ].addr;

        vector<Attribute*> sample;
        int count, attrPos;

        if( instream ) {
            count = ( ( CcInt* )args[ 3 ].addr )->GetValue( );
            attrPos = ( ( CcInt* )args[ 4 ].addr )->GetValue( );
        }
        else {
            count = ( ( Relation* )args[ 0 ].addr )->GetNoTuples( );
            attrPos = ( ( CcInt* )args[ 3 ].addr )->GetValue( );
        }

        int sampleSize = DRelHelpers::computeSampleSize( count );
        int nth = DRelHelpers::everyNthTupleForSample(
            sampleSize, count );

        if( instream ) {
            Stream<Tuple> stream( args[ 0 ] );

            // create a sample
            Tuple* tuple;
            stream.open( );
            int i = 1;
            while( ( tuple = stream.request( ) ) ) {
                if( i == nth ) {
                    tuple->IncReference( );
                    sample.push_back(
                        tuple->GetAttribute( attrPos ) );
                    i = 1;
                } else {
                    i++;
                }
                tuple->DeleteIfAllowed( );
            }
            stream.close( );
        }
        else {
            Relation* rel = ( Relation* )args[ 0 ].addr;

            // create a sample
            GenericRelationIterator* it = rel->MakeScan( );
            Tuple* tuple;
            while( ( tuple = it->GetNthTuple( nth, false ) ) ) {
                /*tuple->IncReference( );
                sample.push_back( 
                    tuple->GetAttribute( attrPos->GetValue( ) ) );*/
                sample.push_back( 
                    tuple->GetAttribute( attrPos )->Clone( ) );
                tuple->DeleteIfAllowed( );
            }
            delete it;
        }


        // sort the sample
        sort( sample.begin( ), sample.end( ), DRelHelpers::compareAttributes );

        // create the boundary
        nth = DRelHelpers::everyNthTupleForArray( 
            sample.size( ), size->GetValue( ) );
        int i = 1;
        for( vector<Attribute*>::iterator it = sample.begin( );
            it != sample.end( ); ++it ) {

            if( i == nth ) {
                i = 1;
                resultColl->Insert( ( *it )->Clone( ), 1 );
            } else {
                i++;
            }
            ( *it )->DeleteIfAllowed( );
        }

        sample.clear( );

        resultColl->Finish( );

        return 0;
    }

/*
1.3 ValueMapping Array for createboundary

*/
ValueMapping createboundaryVM[ ] = {
    createboundaryVMT<false>,
    createboundaryVMT<true>
};

/*
1.4 Selection function for createboundary

*/
int createboundarySelect( ListExpr args ) {

    return nl->HasLength( args, 3 ) ? 0 : 1;
}

/*
1.5 Specification of createboundary

*/
    OperatorSpec createboundary(
        " rel(tuple(X)) x attr x int or stream(tuple(X)) x attr x int x int "
        "-> boundary(x) ",
        " _ createboundary[ _, _, _ ]",
        "Creates a boundary object for a relation. The boundaries "
        "are determinated by a sample of the realtion.",
        " query plz createboundary[PLZ, 50]"
    );

/*
1.6 Operator instance of createboundary operator

*/
    Operator createboundaryOp(
        "createboundary",
        createboundary.getStr( ),
        2,
        createboundaryVM,
        createboundarySelect,
        createboundaryTM
    );

} // end of namespace drel