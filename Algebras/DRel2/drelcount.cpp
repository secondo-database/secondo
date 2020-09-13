
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

 @author
 T. Beckmann

 @description
 see OperatorSpec

 @note
 Checked - 2020 
 Template lcount for checking orginal/cell attributes and working with them.

 @history
 Version 1.0 - Created - T. Beckmann - 2018
 Version 1.1 - Small improvements - D. Selenyi - 25.07.2020
 Version 1.2 - lcount/count is checking the attributes Original/Cell if those exists, gives a Flag to Typemapping and delete those attributes if they exists. - D. Selenyi - 25.07.2020

 @todo
 Nothing
*/

/*
1 Implementation of the secondo operator count, lcount

*/
//#define DRELDEBUG


#include "include/NestedList.h"
#include "include/ListUtils.h"
#include "include/QueryProcessor.h"
#include "include/StandardTypes.h"

#include "Algebras/DRel/DRelHelpers.h"
#include "Algebras/DRel/DRel.h"

extern NestedList* nl;
extern QueryProcessor* qp;

using namespace distributed2;

namespace drel {

/*
1.1 Type Mapping

Except a d[f]rel as argument.

*/
    ListExpr countTM( ListExpr args ) {

        #ifdef DRELDEBUG
        cout << "countTM" << endl;
        cout << nl->ToString( args ) << endl;
        #endif

        std::string err = "d[f]rel expected";

        if( !nl->HasLength( args, 1 ) ) {
            return listutils::typeError( err + 
                ": one argument is expected" );
        }

        //Checking if Original/Cell exists
        ListExpr darrayType;
        if( !DRelHelpers::drelCheck(nl->First( args), darrayType) ) {
            return listutils::typeError(err + 
                ": first argument is not a d[f]rel" );
        }

        //Prepare remove Original and Cell
        ListExpr attrList, attrType;
        attrList = nl->Second(nl->Second( nl->Second( darrayType ) ));
        int flag = 0; //Flag to show if Original or 
                      //Original/Cell as Attributes exists

        if(listutils::findAttribute(attrList, "Original", attrType)){
          flag = 1;
        }
        
        if(listutils::findAttribute(attrList, "Cell", attrType)){
          flag = 2;
        }

        ListExpr appendList = nl->OneElemList( 
            nl->IntAtom( flag ));

        return nl->ThreeElemList(
            nl->SymbolAtom( Symbols::APPEND( ) ),
            appendList,
            listutils::basicSymbol<CcInt>( ) );

    }

/*
1.2 Type Mapping

Expect a d[f]rel and a bool as argument.

*/
    ListExpr lcountTM( ListExpr args ) {

        #ifdef DRELDEBUG
        cout << "lcountTM" << endl;
        cout << nl->ToString( args ) << endl;
        #endif

        std::string err = "d[f]rel x bool expected";

        if( !nl->HasLength( args, 2 ) ) {
            return listutils::typeError( err + 
                ": two arguments expected" );
        }

        //Checking if Original/Cell exists
        ListExpr darrayType;
        if( !DRelHelpers::drelCheck(nl->First( args), darrayType) ) {
            return listutils::typeError(err + 
                ": first argument is not a d[f]rel" );
        }

        if( !CcBool::checkType( nl->Second( args ) ) ) {
            return listutils::typeError( err + 
                ": second argument is not a bool" );
        }
        
        //Prepare remove Original and Cell
        ListExpr attrList, attrType;

        attrList = nl->Second(nl->Second( nl->Second( darrayType ) ));
        int flag = 0; //Flag to show if Original or 
                      //Original/Cell as Attributes exists

        if(listutils::findAttribute(attrList, "Original", attrType)){
          flag = 1;
        }
        
        if(listutils::findAttribute(attrList, "Cell", attrType)){
          flag = 2;
        }

        ListExpr resultType = nl->TwoElemList(
            listutils::basicSymbol<DArray>( ),
            listutils::basicSymbol<CcInt>( ) );

        ListExpr appendList = nl->OneElemList( 
            nl->IntAtom( flag ));

        return nl->ThreeElemList(
            nl->SymbolAtom( Symbols::APPEND( ) ),
            appendList,
            resultType );
    }

/*
1.3 Value Mapping Operator lcount

*/
    template<class T>
    int lcountVMT( Word* args, Word& result, int message,
        Word& local, Supplier s ) {

        #ifdef DRELDEBUG
        cout << "lcountVMT" << endl;
        cout << args << endl;
        #endif

        DRel* drel = ( DRel* )args[ 0 ].addr;
        bool replTuple = ( ( CcBool* )args[ 1 ].addr )->GetValue( );
        int flag = ((CcInt* )args[2].addr)->GetValue();

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
           || dType == spatial3d )  && 
             (flag == 1 || flag == 2)){  
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

        ListExpr queryList;
        if( !nl->ReadFromString( queryS, queryList ) ) {
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
        result = qp->ResultStorage( s );
        darray = ( DArray* )result.addr;
        Word tmpResult;

        if( !QueryProcessor::ExecuteQuery( queryList, tmpResult, 
                typeString, errorString,
                correct, evaluable, defined, isFunction ) ) {
            darray->makeUndefined( );
            return 0;
        }

        #ifdef DRELDEBUG
        cout << "queryList: " << nl->ToString(queryList) << endl;
        cout << "typeString: " << typeString << endl;
        cout << "errorString: " << errorString << endl;
        #endif

        if( !correct || !evaluable || !defined ) {
            darray = ( DArray* )result.addr;
            darray->makeUndefined( );
            return 0;
        }
        
        (*darray) = (*( (DArray*) tmpResult.addr));
        delete (DArray*) tmpResult.addr; 
        return 0;
    }

/*
1.4 Value Mapping Operator count

*/
    template<class T>
    int countVMT( Word* args, Word& result, int message,
        Word& local, Supplier s ) {

        #ifdef DRELDEBUG
        cout << "countVMT" << endl;
        cout << args << endl;
        #endif

        DRel* drel = ( DRel* )args[ 0 ].addr;
        int flag = ((CcInt* )args[1].addr)->GetValue();
        
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

        if(( dType == replicated
         || dType == spatial2d
         || dType == spatial3d ) && 
             (flag == 1 || flag == 2)){
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


        ListExpr queryList;
        if( !nl->ReadFromString( queryS, queryList ) ) {
            res->SetDefined( false );
            return 0;
        }

        bool correct = false;
        bool evaluable = false;
        bool defined = false;
        bool isFunction = false;
        std::string typeString, errorString;
        Word tmpResult;

        if( !QueryProcessor::ExecuteQuery( queryList, tmpResult, 
                typeString, errorString,
                correct, evaluable, defined, isFunction ) ) {
            res->SetDefined( false );
            return 0;
        }

        #ifdef DRELDEBUG
        cout << "queryList: " << nl->ToString(queryList) << endl;
        cout << "typeString: " << typeString << endl;
        cout << "errorString: " << errorString << endl;
        #endif
        
        if( !correct || !evaluable || !defined ) {
            res->SetDefined( false );
            return 0;
        }

        // overtake result from queryList
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
