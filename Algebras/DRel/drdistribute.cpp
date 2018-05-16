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



1 Implementation of the secondo operator drdistribute

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

#include "DRel.h"
#include "DRelHelpers.h"

extern NestedList* nl;
extern QueryProcessor* qp;

namespace distributed2 {
    // used classes and functions of the Distributed2Algebra

    class FRelCopy;
    class RelFileRestorer;

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

    int distribute3Select( ListExpr args );
}

using namespace distributed2;

namespace drel {
    /*
    1.1 Helper functions

    1.1.1 ~distributedefaultparmsTM~

    Checks the first four parameters for the distribute operator. They are 
    independent of the distribution type.

    */
    bool distributedefaultparmsTM( 
        ListExpr args, 
        const string err, 
        ListExpr& error, 
        distributionType& requestedDistType ) {

        if( !( nl->ListLength( args ) >= 4 && nl->ListLength( args ) <= 6 ) ) {
            error = listutils::typeError( err + ": wrong number of args" );
            return false;
        }

        // Check the first fourth parameters
        if( !( nl->HasLength( nl->First( args ), 2 )
            && nl->HasLength( nl->Second( args ), 2 )
            && nl->HasLength( nl->Third( args ), 2 )
            && nl->HasLength( nl->Fourth( args ), 2 ) ) ) {
            error = listutils::typeError( err + ": internal error" );
            return false;
        }

        if( !Relation::checkType( nl->First( nl->First( args ) ) ) ) {
            error = listutils::typeError( 
                err + ": first argument is not a relation" );
            return false;
        }

        if( !CcString::checkType( nl->First( nl->Fourth( args ) ) ) ) {
            error = listutils::typeError( 
                err + ": fourth parameter is not a string" );
            return false;
        }

        if( !nl->IsAtom( nl->Second( nl->Fourth( args ) ) ) ) {
            error = listutils::typeError( 
                err + ": error with requested distType" );
            return false;
        }

        if( !supportedType( nl->StringValue( 
                                nl->Second( nl->Fourth( args ) ) ), 
                                requestedDistType ) ) {
            error = listutils::typeError( 
                err + ": requested distType is not suppored" );
            return false;
        }
        return true;
    }
    
    /*
    1.1.1 ~createStreamOpTree~

    Creates an operator tree to create the stream from a relation.

    */
    bool createStreamOpTree( 
        QueryProcessor* qps, string relName, OpTree &tree ) {

        bool correct = false;
        bool evaluable = false;
        bool defined = false;
        bool isFunction = false;
        ListExpr resultType;

        qps->Construct(
            nl->TwoElemList(
                nl->SymbolAtom( "consume" ),
                nl->TwoElemList(
                    nl->SymbolAtom( "feed" ),
                    nl->SymbolAtom( relName ) ) ),
            correct,
            evaluable,
            defined,
            isFunction,
            tree,
            resultType );

        return correct;
    }

    /*
    1.3 Type Mapping

    Get a relation, a worker relation, a string as name at the workers, a 
    distribution type, a distribution type if necessary and an integer 
    for the array size (not necessary for distribution type replicated).

    */
    template<class R, class A>
    ListExpr distributeTM( ListExpr args ) {

        std::string err = "rel x rel x string x distType [x attr] [x int] "
            "expected";
        ListExpr result;
        ListExpr resType;
        ListExpr appendList;
        ListExpr error;

        distributionType requestedDistType;
        if( !distributedefaultparmsTM(
            args, err, error, requestedDistType ) ) {
            return error;
        }

        // Input types
        ListExpr relType = nl->First( nl->First( args ) );
        string relName = nl->SymbolValue( nl->Second( nl->First( args ) ) );
        ListExpr workerRelType = nl->First( nl->Second( args ) );
        ListExpr nameType = nl->First( nl->Third( args ) );

        // streamType for the TypeMapping of the Distributed2Algebra operators
        ListExpr streamType = nl->TwoElemList( 
            listutils::basicSymbol<Stream<Tuple> >( ),
            nl->Second( relType ) );

        if( nl->HasLength( args, 4 ) ) {

            // reqested typeString has to be replicated
            if( !( requestedDistType == replicated ) ) {
                return listutils::typeError( 
                    err + ": for this number of arguments "
                    "only replicated is supported" );
            }

            // not implemented jet
            return listutils::typeError( err + ": internal error" );
        }

        else if( nl->HasLength( args, 5 ) ) {

            // reqested typeString has to be random for 5 arguments
            if( !( requestedDistType == random ) ) {
                return listutils::typeError( 
                    err + ": for this number of arguments only random is "
                    "supported" );
            }
            if( !( nl->HasLength( nl->Fifth( args ), 2 ) ) ) {
                return listutils::typeError( err + ": internal error" );
            }

            ListExpr numSlotsType = nl->First( nl->Fifth( args ) );
            result = distribute3TM<A>( nl->FiveElemList(
                streamType,
                nameType,
                numSlotsType,
                nl->SymbolAtom( CcBool::BasicType( ) ),
                workerRelType ) );

            if( !nl->HasLength( result, 3 ) ) {
                return result;
            }
            resType = nl->Third( result );
            if( !A::checkType( resType ) ) {
                return result;
            }

            appendList = nl->FourElemList(
                nl->First( nl->Second( result ) ),
                nl->Second( nl->Second( result ) ),
                nl->Third( nl->Second( result ) ),
                nl->StringAtom( relName ) );
        }

        // reqested typeString has to be hash, range, spatial2d 
        // or spatial3d for 5 arguments
        else if( nl->HasLength( args, 6 ) ) {

            if( !( nl->HasLength( nl->Fifth( args ), 2 ) ) ) {
                return listutils::typeError( err + ": internal error" );
            }
            if( !( nl->AtomType( nl->First( nl->Fifth( args ) ) ) 
                    == SymbolType ) ) {
                return listutils::typeError( err + ": fifth parameter is not "
                    "an attribute" );
            }

            if( !( nl->HasLength( nl->Sixth( args ), 2 ) ) ) {
                return listutils::typeError( err + ": internal error" );
            }
            ListExpr numSlotsType = nl->First( nl->Sixth( args ) );

            std::string attrName = nl->SymbolValue( 
                                        nl->Second( nl->Fifth( args ) ) );
            ListExpr attrList = nl->Second( nl->Second( relType ) );
            ListExpr attrType;

            if( !DRelHelpers::findAttribute( attrList, attrName, attrType ) ) {
                return listutils::typeError( 
                    err + ": attr name " + attrName + " not found" );
            }

            if( requestedDistType == hash ) {
                
                if( !( CcInt::checkType( attrType ) ) ) {
                    return listutils::typeError( 
                        err + ": attribute is not of type " + 
                            CcInt::BasicType( ) );
                }

                
                // get result type from distributed4 operator
                result = distribute4TMT<A>(
                    nl->FiveElemList(
                        streamType,
                        nameType,
                        nl->ThreeElemList( 
                            nl->SymbolAtom( "map" ),
                            nl->Second( relType ),
                            nl->SymbolAtom( CcInt::BasicType( ) ) ),
                        numSlotsType,
                        workerRelType ) );
            }
            else if( requestedDistType == range ) {

                if( !( CcInt::checkType( attrType ) )
                    && !( CcString::checkType( attrType ) ) ) {
                    return listutils::typeError( 
                        err + ": attribute is not of type " + 
                        CcInt::BasicType( ) +
                        " or " + CcString::BasicType( ) );
                }

                // get result type from distributed4 operator
                result = distribute4TMT<A>(
                    nl->FiveElemList(
                        streamType,
                        nameType,
                        nl->ThreeElemList(
                            nl->SymbolAtom( "map" ),
                            nl->Second( relType ),
                            nl->SymbolAtom( CcInt::BasicType( ) ) ),
                        numSlotsType,
                        workerRelType ) );
            }
            else if( requestedDistType == spatial2d 
                || requestedDistType == spatial3d ) {

                if( !( Point::checkType( attrType ) )
                    && !( Line::checkType( attrType ) )
                    && !( Region::checkType( attrType ) ) ) {
                    return listutils::typeError( 
                        err + ": attribute is not of type " +
                        Point::BasicType( ) + ", " + Line::BasicType( ) + 
                        " or " + Region::BasicType( ) );
                }

                // get result type from distributed4 operator
                result = distribute4TMT<A>(
                    nl->FiveElemList(
                        streamType,
                        nameType,
                        nl->ThreeElemList(
                            nl->SymbolAtom( "map" ),
                            nl->Second( relType ),
                            nl->SymbolAtom( CcInt::BasicType( ) ) ),
                        numSlotsType,
                        nameType ) );
            }
            else {
                return listutils::typeError( 
                    err + ": for this number of arguments only range, "
                    "spatial2d or spatial3d is supported" );
            }

            if( !nl->HasLength( result, 3 ) ) {
                return result;
            }
            resType = nl->Third( result );
            if( !A::checkType( resType ) ) {
                return result;
            }

            // typemap was OK, add attribute name to the append list
            appendList = nl->FiveElemList(
                nl->First( nl->Second( result ) ),
                nl->Second( nl->Second( result ) ),
                nl->Third( nl->Second( result ) ),
                nl->StringAtom( attrName ),
                nl->StringAtom( relName ) );

        }
        else {
            return listutils::typeError( err + ": wrong number of args" );
        }

        //create result type
        ListExpr newRes = nl->FiveElemList( listutils::basicSymbol<R>( ),
            nl->Second( resType ),
            nl->SymbolAtom( CcString::BasicType( ) ),
            nl->SymbolAtom( CcInt::BasicType( ) ),
            nl->SymbolAtom( CcInt::BasicType( ) ) );

        return nl->ThreeElemList( nl->SymbolAtom( Symbols::APPEND( ) ),
            appendList,
            newRes );
    }

    /*
    1.4 Value Mapping

    Distribute the relation to the workers by calling the distribute2, 
    distribute3 and distribute4 value mappings of the Distributed2Algebra 

    */
    template<class RType, class AType, class DType, class HType, class CType>
    int distributeVMT( Word* args, Word& result, int message,
        Word& local, Supplier s ) {

        Relation* workers = ( Relation* )args[ 1 ].addr;
        CcString* name = ( CcString* )args[ 2 ].addr;
        CcString* typeString = ( CcString* )args[ 3 ].addr;
        distributionType type = getType( typeString->GetValue( ) );

        string relName;
        if( type == replicated ) {
            relName = ( ( CcString* )args[ 7 ].addr )->GetValue( );
        }
        else if ( type == random ) {
            relName = ( ( CcString* )args[ 8 ].addr )->GetValue( );
        }
        else {
            relName = ( ( CcString* )args[ 10 ].addr )->GetValue( );
        }

        // Create Stream
        QueryProcessor* qps = new QueryProcessor( nl, am );
        OpTree treeS = 0;

        if( !createStreamOpTree( qps, relName, treeS ) ) {
            cout << "Error while create operator tree for the stream '(feed "
                + relName + ")'" << endl;
            result = qp->ResultStorage( s );
            RType* drel = ( RType* )result.addr;
            drel->makeUndefined( );
            qps->Destroy( treeS, true );
            delete qps;
            return 0;
        }

        if( type == random ) {
            CcInt* size = ( CcInt* )args[ 4 ].addr;
            CcBool* method = new CcBool( true, true );

            ArgVector argVec = { args[ 0 ].addr, 
                                 name, 
                                 size, 
                                 method, 
                                 workers, 
                                 args[5].addr, 
                                 args[ 6 ].addr, 
                                 args[ 7 ].addr };

            distribute3VMT<AType, DType, HType, CType>( argVec, 
                result, message, local, s );

            RType* drel = ( RType* )result.addr;
            if( drel->IsDefined( ) ) {
                drel->setDistType( DistType::createDistType( type ) );
            }
        }
        else if( type == hash ) {
            CcInt* size = ( CcInt* )args[ 5 ].addr;
            string attrname = ( string )( char* )
                ( ( CcString* )args[ 9 ].addr )->GetStringval( );

            ListExpr funarg1 = nl->TwoElemList(
                nl->SymbolAtom( "t" ),
                nl->Second( nl->Second( qp->GetType( s ) ) ) );

            ListExpr fundef = nl->ThreeElemList(
                nl->SymbolAtom( "hashvalue" ),
                nl->ThreeElemList(
                    nl->SymbolAtom( "attr" ),
                    nl->SymbolAtom( "t" ),
                    nl->SymbolAtom( attrname ) ),    // Attributname
                nl->IntAtom( 10000 ) );

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
                qps->Destroy( treeS, true );
                delete qps;
                return 0;
            }

            ArgVector argVec = { treeS, 
                                 name, 
                                 tree, 
                                 size, 
                                 workers, 
                                 args[ 6 ].addr, 
                                 args[ 7 ].addr, 
                                 args[ 8 ].addr };

            distribute4VMT<AType, DType, HType, CType>( argVec,
                result, message, local, s );

            qp2->Destroy( tree, true );
            delete qp2;

            cout << 
                nl->ToString( nl->Second( nl->Second( qp->GetType( s ) ) ) ) 
                << endl;

            RType* drel = ( RType* )result.addr;
            if( drel->IsDefined( ) ) {
                ListExpr attrType;
                int pos = listutils::findAttribute( 
                    nl->Second( nl->Second( nl->Second( qp->GetType( s ) ) ) ),
                    attrname,
                    attrType );
                drel->setDistType( DistType::createDistType( type, pos ) );
            }
        }
        else if( type == range ) {
            //CcInt* size = ( CcInt* )args[ 5 ].addr;
            string attrName = ( string )( char* )
                ( ( CcString* )args[ 9 ].addr )->GetStringval( );

            int records;
            if( !DRelHelpers::countRecords( attrName, relName, records ) ) {
                cout << "can not request the number of records of relation " 
                    + relName << endl;
                result = qp->ResultStorage( s );
                RType* drel = ( RType* )result.addr;
                drel->makeUndefined( );
                qps->Destroy( treeS, true );
                delete qps;
                return 0;
            }


            cout << "Total number of records: " + to_string( records ) << endl;
            cout << "Create sample of 5000" << endl;
            int nth = ceil( ( ( double )records ) / 5000 );
            cout << "Extract each " + to_string( nth ) + 
                "th record for the sample" << endl;
            mm2algebra::MPointer* mmsamplep = 
                DRelHelpers::createMemSample( relName, attrName );
            mm2algebra::MPointer* mmsampleavlp = 
                DRelHelpers::createAVLtree( mmsamplep );

            cout << attrName << endl;

            ListExpr funList = DRelHelpers::createSampleMemList(
                relName, attrName );
            cout << nl->ToString( funList ) << endl;

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
                qps->Destroy( treeS, true );
                delete qps;
                return 0;
            }

            cout << "sample" << endl;
            cout << mmsamplep << endl;
            cout << mmsampleavlp << endl;

            result = qp->ResultStorage( s );
            RType* drel = ( RType* )result.addr;
            drel->makeUndefined( );

        }

        qps->Destroy( treeS, true );
        delete qps;

        return 0;
    }

    /*
    1.5 Specification of drfdistirbute

    */
    OperatorSpec fdistributeSpec(
        " rel(tuple(X)) x rel(tuple(X)) x string x distType [x attr] [x int] "
        "-> dfrel(X) ",
        " _ drfdistribute[ _, _, _, _, _]",
        "Distributes a relation into a dfrel. "
        "The first argument is the relation to distribute. The second "
        "argument is the worker relation. It must be a relation having "
        "attributes Host, Port, and Config. Host and Config must be of "
        "type string or text, the Port attribute must be of type int. "
        "The third Argument is the name for the resulting dfrel. If "
        "the name is an empty string, a name is choosen automatically. "
        "The fourth argument is the distribution type. Possible values "
        "are random, hash, range, spatial2d spatial3d and replicated. "
        "This argument specifies the type to distribute the relation "
        "to the workers. The fifth argument is an attribute to distribute "
        "the relation. This attribute is required for the distType hash,  "
        "range, spatial2d and spatial3d and controls in which slot of the "
        "resulting array is the corresponding tuple inserted. The sixth "
        "argument specifies the size of the resulting array. If replicated "
        "is choosen this argument is unnecessary. ",
        " query strassen drfdistribute[Worker3, \"\", \"range\", No, 5]"
    );

    /*
    1.6 ValueMapping Array of drfdistribute

    */
    ValueMapping fdistributeVM[ ] = {
        distributeVMT<DFRel, DFArray, FRelCopy, CcString, CcString>,
        distributeVMT<DFRel, DFArray, FRelCopy, CcString, FText>,
        distributeVMT<DFRel, DFArray, FRelCopy, FText, CcString>,
        distributeVMT<DFRel, DFArray, FRelCopy, FText, FText>
    };

    /*
    1.7 Selection function

    */
    int distributeSelect( ListExpr args ) {
        ListExpr rel = nl->Second( args );
        ListExpr attrList = nl->Second( nl->Second( rel ) );
        ListExpr hostType, configType;
        listutils::findAttribute( attrList, "Host", hostType );
        listutils::findAttribute( attrList, "Config", configType );
        int n1 = CcString::checkType( hostType ) ? 0 : 2;
        int n2 = CcString::checkType( configType ) ? 0 : 1;
        return n1 + n2;
    }

    /*
    1.8 Operator instance of drfdistribute operator 

    */
    Operator fdistributeOp(
        "drfdistribute",
        fdistributeSpec.getStr( ),
        4,
        fdistributeVM,
        distributeSelect,
        distributeTM<DFRel, DFArray>
    );

    /*
    1.9 Specification of drddistirbute

    */
    OperatorSpec ddistributeSpec(
        " rel(tuple(X)) x rel(tuple(X)) x string x distType [x attr] [x int] "
        "-> drel(X) ",
        " _ drddistribute[ _, _, _, _, _]",
        "Distributes a relation into a drel. "
        "The first argument is the relation to distribute. The second "
        "argument is the worker relation. It must be a relation having "
        "attributes Host, Port, and Config. Host and Config must be of "
        "type string or text, the Port attribute must be of type int. "
        "The third Argument is the name for the resulting drel. If "
        "the name is an empty string, a name is choosen automatically. "
        "The fourth argument is the distribution type. Possible values "
        "are random, hash, range, spatial2d spatial3d and replicated. "
        "This argument specifies the type to distribute the relation "
        "to the workers. The fifth argument is an attribute to distribute "
        "the relation. This attribute is required for the distType hash,  "
        "range, spatial2d and spatial3d and controls in which slot of the "
        "resulting array is the corresponding tuple inserted. The sixth "
        "argument specifies the size of the resulting array. If replicated "
        "is choosen this argument is unnecessary. ",
        " query strassen drddistribute[Worker3, \"\", \"range\", No, 5]"
    );

    /*
    1.10 ValueMapping Array of drddistribute

    */
    ValueMapping ddistributeVM[ ] = {
        distributeVMT<DRel, DArray, RelFileRestorer, CcString, CcString>,
        distributeVMT<DRel, DArray, RelFileRestorer, CcString, FText>,
        distributeVMT<DRel, DArray, RelFileRestorer, FText, CcString>,
        distributeVMT<DRel, DArray, RelFileRestorer, FText, FText>
    };

    /*
    1.11 Operator instance of drddistribute operator

    */
    Operator ddistributeOp(
        "drddistribute",
        ddistributeSpec.getStr( ),
        4,
        ddistributeVM,
        distributeSelect,
        distributeTM<DRel, DArray>
    );

} // end of namespace drel