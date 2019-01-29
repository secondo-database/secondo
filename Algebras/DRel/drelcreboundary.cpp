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

        if( !nl->HasLength( args, 3 ) ) {
            return listutils::typeError( err +
                ": three arguments are expected" );
        }

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

        std::string attrName = nl->SymbolValue(
            nl->Second( nl->Second( args ) ) );
        ListExpr attrList = nl->Second( nl->Second( 
                            nl->First( nl->First( args ) ) ) );

        ListExpr attrType;
        if( listutils::findAttribute( attrList, attrName, attrType ) == 0 ) {
            return listutils::typeError(
                err + ": attr name " + attrName + " not found" );
        }

        ListExpr resType = nl->TwoElemList(
            nl->SymbolAtom( Vector::BasicType( ) ),
            attrType );

        ListExpr appendList = nl->OneElemList(
            nl->StringAtom( attrName ) );

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

        ListExpr arg1Type = qp->GetType( qp->GetSon( s, 0 ) );
        collection::Collection* resultColl;

        int size = ( ( CcInt* )args[ 2 ].addr )->GetValue( );

        ListExpr arg1PtrList = DRelHelpers::createPointerList( 
                arg1Type, args[ 0 ].addr );

        std::string attr, query;

        int count = ( ( Relation* )args[ 0 ].addr )->GetNoTuples( );
        attr = ( ( CcString* )args[ 3 ].addr )->GetValue( );
        std::string arg1 = "(feed " + nl->ToString( arg1PtrList ) + ")";
        
        int sampleS = DRelHelpers::computeSampleSize( count );
        int nthS = DRelHelpers::everyNthTupleForSample( sampleS, count );
        int nthB = DRelHelpers::everyNthTupleForArray( count / nthS, size );

        query = "(collect_vector (head (transformstream (nth (sort (nth"
            " (project " + arg1 + " (" + attr + ") )" + 
            std::to_string( nthS ) + " FALSE) )" + std::to_string( nthB ) +
            " TRUE)) " + std::to_string( size ) + "))";

        ListExpr queryList;
        if( !nl->ReadFromString( query, queryList ) ) {

            result = qp->ResultStorage( s );
            resultColl = static_cast<collection::Collection*>( result.addr );
            resultColl->SetDefined( false );
            return 0;
        }

        bool correct = false;
        bool evaluable = false;
        bool defined = false;
        bool isFunction = false;
        std::string typeString, errorString;
        if( !QueryProcessor::ExecuteQuery( queryList, result, 
                typeString, errorString,
                correct, evaluable, defined, isFunction ) ) {

            result = qp->ResultStorage( s );
            resultColl = static_cast<collection::Collection*>( result.addr );
            resultColl->SetDefined( false );
            return 0;
        }
        
        if( !correct || !evaluable || !defined ) {
            
            result = qp->ResultStorage( s );
            resultColl = static_cast<collection::Collection*>( result.addr );
            resultColl->SetDefined( false );
            return 0;
        }

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
