
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



1 Implementation of the secondo operator count, lcount

*/
#include "DRel.h"
#include "NestedList.h"
#include "ListUtils.h"
#include "QueryProcessor.h"
#include "StandardTypes.h"

#include "DRelHelpers.h"

extern NestedList* nl;
extern QueryProcessor* qp;

using namespace distributed2;

namespace drel {

/*
1.1 Type Mapping

Get a d[f]rel as argument.

*/
    ListExpr countTM( ListExpr args ) {

        std::string err = "d[f]rel expected";

        if( !nl->HasLength( args, 1 ) ) {
            return listutils::typeError( err + 
                ": one argument is expected" );
        }

        if( !DRelHelpers::drelCheck( nl->First( args ) ) ) {
            return listutils::typeError( err + 
                ": first argument is not a d[f]rel" );
        }

        return listutils::basicSymbol<CcInt>( );
    }

/*
1.2 Type Mapping

Get a d[f]rel and a bool as argument.

*/
    ListExpr lcountTM( ListExpr args ) {

        std::string err = "d[f]rel x bool expected";

        if( !nl->HasLength( args, 2 ) ) {
            return listutils::typeError( err + 
                ": two arguments are expected" );
        }

        if( !DRelHelpers::drelCheck( nl->First( args ) ) ) {
            return listutils::typeError( err + 
                ": first argument is not a d[f]rel" );
        }

        if( !CcBool::checkType( nl->Second( args ) ) ) {
            return listutils::typeError( err + 
                ": second argument is not a bool" );
        }

        return nl->TwoElemList(
            listutils::basicSymbol<DArray>( ),
            listutils::basicSymbol<CcInt>( ) );
    }

/*
1.3 Value Mapping Operator lcount

*/
    template<class T>
    int lcountVMT( Word* args, Word& result, int message,
        Word& local, Supplier s ) {

        DRel* drel = ( DRel* )args[ 0 ].addr;
        bool replTuple = ( ( CcBool* )args[ 1 ].addr )->GetValue( );

        DArray* darray;

        if( !drel->IsDefined( ) ) {
            result = qp->ResultStorage( s );
            darray = ( DArray* )result.addr;
            darray->makeUndefined( );
            return 0;
        }

        distributionType dType = drel->getDistType( )->getDistType( );

        std::string drelptr = nl->ToString( DRelHelpers::createdrel2darray( 
            qp->GetType( qp->GetSon( s, 0 ) ), args[ 0 ].addr ) );

        std::string queryS;

        if( replTuple &&
           (  dType == replicated
           || dType == spatial2d
           || dType == spatial3d ) ) {
            queryS = "(dmap " + drelptr + "\"\""
                "(fun (dmapelem_1 ARRAYFUNARG1) (count (filter "
                "(feed dmapelem_1) (fun (streamelem_2 STREAMELEM) "
                "(= (attr streamelem_2 Original) TRUE))))))";
            
        }
        else {
            queryS = "(dmap " + drelptr + 
                " \"\" (fun (dmapelem_1 ARRAYFUNARG1) (count (feed dmapelem_1)"
                ")))";
        }

        ListExpr query;
        if( !nl->ReadFromString( queryS, query ) ) {
            result = qp->ResultStorage( s );
            darray = ( DArray* )result.addr;
            darray->makeUndefined( );
            return 0;
        }

        bool correct = false;
        bool evaluable = false;
        bool defined = false;
        bool isFunction = false;
        std::string typeString, errorString;
        if( !QueryProcessor::ExecuteQuery( query, result, 
                typeString, errorString,
                correct, evaluable, defined, isFunction ) ) {
            result = qp->ResultStorage( s );
            darray = ( DArray* )result.addr;
            darray->makeUndefined( );
            return 0;
        }
        
        if( !correct || !evaluable || !defined ) {
            result = qp->ResultStorage( s );
            darray = ( DArray* )result.addr;
            darray->makeUndefined( );
            return 0;
        }

        return 0;
    }

/*
1.4 Value Mapping Operator count

*/
    template<class T>
    int countVMT( Word* args, Word& result, int message,
        Word& local, Supplier s ) {

        DRel* drel = ( DRel* )args[ 0 ].addr;

        
        result = qp->ResultStorage( s );
        CcInt* res = ( CcInt* )result.addr;

        if( !drel->IsDefined( ) ) {
            res->SetDefined( false );
            return 0;
        }

        distributionType dType = drel->getDistType( )->getDistType( );

        std::string drelptr = nl->ToString( DRelHelpers::createdrel2darray( 
            qp->GetType( qp->GetSon( s, 0 ) ), args[ 0 ].addr ) );

        std::string queryS;

        if( dType == replicated
         || dType == spatial2d
         || dType == spatial3d ) {
            queryS = "(dmap " + drelptr + "\"\""
                "(fun (dmapelem_1 ARRAYFUNARG1) (count (filter "
                "(feed dmapelem_1) (fun (streamelem_2 STREAMELEM) "
                "(= (attr streamelem_2 Original) TRUE))))))";
            
        }
        else {
            queryS = "(dmap " + drelptr + 
                " \"\" (fun (dmapelem_1 ARRAYFUNARG1) (count (feed dmapelem_1)"
                ")))";
        }

        queryS = "(tie (getValue " + queryS +
            ") (fun (first_2 ELEMENT) (second_3 ELEMENT) "
            "(+ first_2 second_3)))";


        ListExpr query;
        if( !nl->ReadFromString( queryS, query ) ) {
            res->SetDefined( false );
            return 0;
        }

        bool correct = false;
        bool evaluable = false;
        bool defined = false;
        bool isFunction = false;
        std::string typeString, errorString;
        Word tmpResult;

        if( !QueryProcessor::ExecuteQuery( query, tmpResult, 
                typeString, errorString,
                correct, evaluable, defined, isFunction ) ) {
            res->SetDefined( false );
            return 0;
        }
        
        if( !correct || !evaluable || !defined ) {
            res->SetDefined( false );
            return 0;
        }
        // overtake result from query
        CcInt* tmpRes = (CcInt*) tmpResult.addr;
        res->CopyFrom(tmpRes);
        delete tmpRes;
        return 0;
    }

/*
1.5 ValueMapping Array for lcount

*/
    ValueMapping lcountVM[ ] = {
        lcountVMT<DRel>,
        lcountVMT<DFRel>
    };

/*
1.6 ValueMapping Array for count

*/
    ValueMapping countVM[ ] = {
        countVMT<DRel>,
        countVMT<DFRel>
    };

/*
1.7 Selection functions for count and lcount

*/
    int countSelect( ListExpr args ) {

        return DRel::checkType( nl->First( args ) ) ? 0 : 1;
    }

/*
1.8 Specification of lcount

*/
    OperatorSpec lcountSpec(
        "d[f]rel(X) x bool -> darray ",
        " _ lcount",
        "Count the tuples in the partition of a d[f]rel. The second argument "
        "is for filtering replicated tuples while using spatial partitioning."
        " TRUE for eliminating the replicas while counting.",
        " query drel1 lcount"
    );

/*
1.9 Operator instance of lcount operator

*/
    Operator lcountOp(
        "lcount",
        lcountSpec.getStr( ),
        2,
        lcountVM,
        countSelect,
        lcountTM
    );

/*
1.10 Specification of count

*/
    OperatorSpec countSpec(
        "d[f]rel(X) -> int ",
        " _ count",
        "Count the tuples of a d[f]rel. Replicated tuples of a spatially "
        "distributed drel are counted only once.",
        " query drel1 count"
    );

/*
1.11 Operator instance of count operator

*/
    Operator countOp(
        "count",
        countSpec.getStr( ),
        2,
        countVM,
        countSelect,
        countTM
    );

} // end of namespace drel
