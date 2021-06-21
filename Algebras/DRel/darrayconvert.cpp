
/*
----
This file is part of SECONDO.

Copyright (C) 2021,
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



1 Implementation of the secondo operator darray2drel

*/
#include "DRel.h"
#include "NestedList.h"
#include "ListUtils.h"
#include "QueryProcessor.h"
#include "StandardTypes.h"

#include "DRelHelpers.h"

extern NestedList* nl;
extern QueryProcessor* qp;

namespace drel {

/*
1.1 Type Mapping

Get a d[f]rel or a d[f]arry as argument.

*/
    ListExpr darray2drelTM( ListExpr args ) {

        std::string err = "d[f]rel or d[f]array x partType expected";

        if( nl->ListLength( args ) < 2 ) {
            return listutils::typeError( err + 
                ": At least two elements expected" );
        }

        std::string distributionTypeString 
            = nl->SymbolValue(nl->First(nl->Second(args)));
       
        if( ! supportedType(distributionTypeString)) {
            return listutils::typeError("Unsupported distribution "
                 " type: " + distributionTypeString);
        }

        distributionType distributionType = getType( distributionTypeString );

        // (((darray (rel (tuple ((PLZ int) (Ort string))))) PLZByP) 
        // (HASH HASH) (string "PLZ"))
        //std::cout << nl->ToString(args) << endl;
        ListExpr resultType = nl->First( nl -> First( args) );

        if(distributionType == random) {
            //===================
            // Random
            //===================
            if( !nl->HasLength( args, 2 ) ) {
                return listutils::typeError( err + 
                    ": two arguments are expected" );
            }

            ListExpr symbol;

            if( distributed2::DFArray::checkType( resultType ) ) {
                symbol = listutils::basicSymbol<DFRel>( );
            }
            else if( distributed2::DArray::checkType( resultType ) ) {
                symbol = listutils::basicSymbol<DFRel>( );
            } else {
                return listutils::typeError( err +
                    ": first argument is not a d[f]rel or a d[f]array" );
            }

            if(!Relation::checkType( nl->Second( resultType ) ) ) {
                    return listutils::typeError( err +
                    ": first argument is not a d[f]array with a relation" );
            }

            return nl->ThreeElemList(
                    symbol,
                    nl->Second( resultType ),
                    nl->OneElemList( nl->IntAtom( random ) ) );

        } else if(distributionType == hash) {
            //===================
            // Hash
            //===================
            std::string pErr = "d[f]rel or d[f]array x partType "
                "x string expected";

            if( !nl->HasLength( args, 3 ) ) {
                return listutils::typeError( pErr + 
                    ": three arguments are expected" );
            }

            // ((darray (rel (tuple ((PLZ int) (Ort string)))) 
            // -> ((PLZ int) (Ort string))
            ListExpr attrList = nl->Second(nl->Second(nl->Second(resultType)));

            if(!CcString::checkType( nl->First(nl->Third(args ) ) ) ) {
                return listutils::typeError( err +
                ": third argument is not a string" );
            }

            std::string name = nl->StringValue(nl->Second(nl->Third(args)));

            int pos = -1;
            int pass = 0;
            while(!nl->IsEmpty(attrList)){

                ListExpr attrName = nl->First(nl->First(attrList));

                if(nl->IsEqual(attrName, name)){
                    pos = pass;
                    break;
                }

                attrList = nl->Rest(attrList);
                pass++;
            }

            if(pos == -1) {
                return listutils::typeError( "Unable to find attribute: " 
                    + name );
            }

            ListExpr symbol;

            if( distributed2::DFArray::checkType( resultType ) ) {
                symbol = listutils::basicSymbol<DFRel>( );
            }
            else if( distributed2::DArray::checkType( resultType ) ) {
                symbol = listutils::basicSymbol<DFRel>( );
            } else {
                return listutils::typeError( err +
                    ": first argument is not a d[f]rel or a d[f]array" );
            }

            if(!Relation::checkType( nl->Second( resultType ) ) ) {
                    return listutils::typeError( err +
                    ": first argument is not a d[f]array with a relation" );
            }

            ListExpr partResult = nl->ThreeElemList(
                    symbol,
                    nl->Second( resultType ),
                    nl->OneElemList( nl->IntAtom( hash ) ) );

            return nl->ThreeElemList(
                nl->SymbolAtom(Symbols::APPEND()),
                nl->OneElemList(nl->IntAtom(pos)),
                partResult);
        } 


    return listutils::typeError("Currently Unsupported distribution "
         " type: " + distributionTypeString);
    }


/*
1.3 Value Mapping - random version

Creates a d[f]rel.

*/
    template<class T, class R>
    int darray2drelRandomVMT( Word* args, Word& result, int message,
        Word& local, Supplier s ) {

        result = qp->ResultStorage( s );

        T* input = ( T* )args[ 0 ].addr;
        R* res = ( R* ) result.addr;
        res->copyFrom( *input ); 
        res->setKeepRemoteObjects( true );

        if( res->IsDefined( ) ) {
            DistTypeBasic* distType = new DistTypeBasic( random );
            res->setDistType( distType );
        }

        return 0;
    }

/*
1.3 Value Mapping - hash version

Creates a d[f]rel.

*/
    template<class T, class R>
    int darray2drelHashVMT( Word* args, Word& result, int message,
        Word& local, Supplier s ) {

        result = qp->ResultStorage( s );

        T* input = ( T* )args[ 0 ].addr;
        R* res = ( R* ) result.addr;

        CcInt* posValue = (CcInt*) args[3].addr;

        if(! posValue->IsDefined()) {
            std::cerr << "Partitioning attribute is undefined" << endl;
            return 0;
        }

        res->copyFrom( *input ); 
        res->setKeepRemoteObjects( true );

        if( res->IsDefined( ) ) {
            DistTypeHash* distType = new DistTypeHash( hash, 
                posValue -> GetValue() );
            res->setDistType( distType );
        }

        return 0;
    }

/*
1.4 ValueMapping Array of drel2darray

*/
    ValueMapping darray2drelVM[ ] = {
        darray2drelRandomVMT<distributed2::DArray, DRel>,
        darray2drelRandomVMT<distributed2::DFArray, DFRel>,
        darray2drelHashVMT<distributed2::DArray, DRel>,
        darray2drelHashVMT<distributed2::DFArray, DFRel>
    };

/*
1.5 Selection function

*/
    int darray2drelSelect( ListExpr args ) {

        // args: ((darray (rel (tuple ((PLZ int) (Ort string))))) HASH string)

        std::string type = nl->SymbolValue( nl->First( nl->First( args ) ) );

        std::string distributionTypeString = nl->SymbolValue(nl->Second(args));
        distributionType distributionType = getType( distributionTypeString );

        if(distributionType == random) {
            if( type == distributed2::DArray::BasicType( ) ) {
                return 0;
            } else if( type == distributed2::DFArray::BasicType( ) ) {
                return 1;
            } 
        } else if(distributionType == hash) {
            if( type == distributed2::DArray::BasicType( ) ) {
                return 2;
            } else if( type == distributed2::DFArray::BasicType( ) ) {
                return 3;
            } 
        }

        std::cerr << "Error: Unsupported type " << type << endl;
        return -1;
    }

/*
1.6 Specification of drel2darray

*/
    OperatorSpec darray2drelSpec(
        "d[f]array(X) x distType [ x string ] ",
        "-> d[f]rel(X)"
        " _ darray2drel[_, _]",
        "Convert a d[f]array into a d[f]rel.",
        "query darray1 darray2drel[HASH, \"Nr\"]"
    );

/*
1.7 Operator instance of drel2darray operator

*/
    Operator darray2drelOp(
        "darray2drel",
        darray2drelSpec.getStr( ),
        4,
        darray2drelVM,
        darray2drelSelect,
        darray2drelTM
    );

} // end of namespace drel
