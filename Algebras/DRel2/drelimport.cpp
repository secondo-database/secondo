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
 Error: DbEnv: BDB2055 Lock table is out of available locker entries 
 Solution: secondoconfig.ini => \# Lock System Configuration => higher numbers in locks

 @history
 Version 1.0 - Created - T. Beckmann - 2018
 Version 1.1 - Small improvements - D. Selenyi - 25.07.2020

 @todo
 Nothing

*/

/*
1 Implementation of the secondo operators drelimport

*/
//#define DRELDEBUG

#include "include/NestedList.h"
#include "include/ListUtils.h"
#include "include/QueryProcessor.h"
#include "include/StandardTypes.h"
#include "include/SecParser.h"
#include "include/Stream.h"

#include "Algebras/Distributed2/CommandLogger.h"
#include "Algebras/Distributed2/Distributed2Algebra.h"

#include "Algebras/FText/FTextAlgebra.h"

#include "Algebras/DRel/DRelHelpers.h"
#include "Algebras/DRel/DRel.h"

extern NestedList* nl;
extern QueryProcessor* qp;

using namespace distributed2;

namespace distributed2 {

    class FRelCopy;
    class RelFileRestorer;

    template<class R>
    ListExpr distribute3TM( ListExpr args );

    template<class AType, class DType, class HType, class CType>
    int distribute3VMT(
        Word* args, Word& result, int message, Word& local, Supplier s );
};

namespace drel {

    ListExpr file2streamTM( ListExpr args );

    OpTree createfile2streamTree(
        QueryProcessor* qps, std::string filename ) {

        bool correct = false;
        bool evaluable = false;
        bool defined = false;
        bool isFunction = false;
        ListExpr resultType;

        OpTree tree = 0;
        qps->Construct(
            nl->TwoElemList(
                nl->SymbolAtom( "file2stream" ),
                nl->StringAtom( filename ) ),
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
1.1 Type Mappings ~drelimportTM~

Expecting string|text x string x integer x relation

*/
    ListExpr drelimportTM( ListExpr args ) {

        #ifdef DRELDEBUG
        cout << "drelimportTM" << endl;
        cout << nl->ToString( args ) << endl;
        #endif

        std::string err = "string|text x string x int x rel expected";

        if( !nl->HasLength( args, 4 ) ) {
            return listutils::typeError( err +
                ": four arguments are expected" );
        }

        ListExpr streamType = file2streamTM(
                nl->OneElemList( nl->First( args ) ) );

        if( !listutils::isTupleStream( streamType ) ) {
            return streamType;
        }

        if( !CcString::checkType( nl->Second( args ) ) ) {
            return listutils::typeError(err + 
                ": second argument is not a string" );
        }

        if( !CcInt::checkType( nl->Third( args ) ) ) {
            return listutils::typeError(err + 
                ": third argument is not a integer" );
        }

        if( !Relation::checkType( nl->Fourth( args ) ) ) {
            return listutils::typeError(err + 
                ": fourth argument is not a relation" );
        }

        std::string errmsg;
        ListExpr types, positions;
        if( !( isWorkerRelDesc( 
                nl->Fourth( args ), positions, types, errmsg ) ) ) {
                     return listutils::typeError(
                     err + ": relation is not a worker relation" );
        }

        ListExpr result = distribute3TM<DArray>( 
            nl->FiveElemList(
                streamType,
                nl->Second( args ),
                nl->Third( args ),
                nl->SymbolAtom( CcBool::BasicType( ) ),
                nl->Fourth( args ) ) );     

        if( !nl->HasLength( result, 3 ) ) {
            return result;
        }

        ListExpr resType = nl->Third( result );

        if( !DArray::checkType( resType ) ) {
            return result;
        }

        ListExpr resultType = nl->ThreeElemList(
            listutils::basicSymbol<DRel>( ),
            nl->Second( nl->Third( result ) ),
            nl->OneElemList( nl->IntAtom( random ) ) );

        ListExpr append = nl->Second( result );

        return nl->ThreeElemList( nl->SymbolAtom( Symbols::APPEND( ) ),
            append,
            resultType );
    }

/*
1.2 Value Mapping ~drelimportVMT~

Reads a file and create a drel.

*/
    template<class R, class DType, class HType, class CType>
    int drelimportVMT( Word* args, Word& result, int message,
        Word& local, Supplier s ) {

        #ifdef DRELDEBUG
        cout << "drelimportVMT" << endl;
        cout << args  << endl;
        #endif

        std::string filename = ( ( R* )args[ 0 ].addr )->GetValue( );

        QueryProcessor* qps = new QueryProcessor( nl, am );
        OpTree stream = createfile2streamTree( qps, filename );

        CcBool* roundRobin = new CcBool( true, true );

        #ifdef DRELDEBUG
        cout << "drelimportVMT" << endl;
        cout << "filename:" << filename << endl;
        cout << "name:" << ( ( CcString* )args[ 1 ].addr )->GetValue( ) << endl;
        cout << "slots:" << ( ( CcInt* )args[ 2 ].addr )->GetValue( ) << endl;
        cout << "Workers:"<< ((Relation*)args[ 3 ].addr)->GetNoTuples() << endl;
        cout << "host:" << ( ( CcInt* )args[ 4 ].addr )->GetValue( ) << endl;
        cout << "port:" << ( ( CcInt* )args[ 5 ].addr )->GetValue( ) << endl;
        cout << "config:" << ( ( CcInt* )args[ 6 ].addr )->GetValue( ) << endl;
        #endif

        ArgVector argVec = { stream, args[ 1 ].addr,
            args[ 2 ].addr,
            roundRobin,  // only round robin
            args[ 3 ].addr, 
            args[ 4 ].addr, 
            args[ 5 ].addr, 
            args[ 6 ].addr };

        distribute3VMT<DArray, DType, HType, CType>( argVec,
            result, message, local, s );

        delete roundRobin;

        DRel* drel = ( DRel* )result.addr;
        drel->setDistType( new DistTypeBasic( random ) );

        qps->Destroy(stream,true);
        delete qps;

        return 0;
    }

/*
1.3 ValueMapping Array for drelimport

*/
    ValueMapping drelimportVM[ ] = {
        drelimportVMT<CcString, RelFileRestorer, CcString, CcString>,  
        drelimportVMT<CcString, RelFileRestorer, CcString, FText>,
        drelimportVMT<CcString, RelFileRestorer, FText, CcString>,
        drelimportVMT<CcString, RelFileRestorer, FText, FText>,
        drelimportVMT<FText, RelFileRestorer, CcString, CcString>,
        drelimportVMT<FText, RelFileRestorer, CcString, FText>,
        drelimportVMT<FText, RelFileRestorer, FText, CcString>,   
        drelimportVMT<FText, RelFileRestorer, FText, FText>
    };

/*
1.4 Selection function for drelimport

*/
    int drelimportSelect( ListExpr args ) {

        ListExpr rel = nl->Fourth( args );
        ListExpr attrList = nl->Second( nl->Second( rel ) );
        ListExpr hostType, configType;
        listutils::findAttribute( attrList, "Host", hostType);
        listutils::findAttribute( attrList, "Config", configType);
        int n1 = CcString::checkType( hostType )? 0 : 2;
        int n2 = CcString::checkType( configType )? 0 : 1;

        int x = CcString::checkType( nl->First( args ) ) ? 0 : 4;

        return n1 + n2 + x;
    }

/*
1.5 Specification for the operator drelimport

*/
    OperatorSpec drelimportSpec(
        " string|text x string x int x rel "
        "-> drel(rel(tuple((Data text))))",
        " drelimport(_,_,_,_)",
        "Imports a file and create a random partitioned drel with relation "
        "schema rel(tuple((Data text)))",
        " query drelimport[\"import.txt\", \"DRel01\", 50, Workers3]"
    );

/*
1.6 Operator instance of the operator drelimport

*/
    Operator drelimportOp(
        "drelimport",
        drelimportSpec.getStr( ),
        8,
        drelimportVM,
        drelimportSelect,
        drelimportTM
    );

} // end of namespace drel
