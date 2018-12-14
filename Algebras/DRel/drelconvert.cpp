
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



1 Implementation of the secondo operator convert2darray

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
    ListExpr drel2darrayTM( ListExpr args ) {

        std::string err = "d[f]rel or d[f]array expected";

        if( !nl->HasLength( args, 1 ) ) {
            return listutils::typeError( err + 
                ": one arguments is expected" );
        }

        ListExpr result;
        if( DFRel::checkType( nl->First( args ) ) ) {
            result = nl->TwoElemList(
                listutils::basicSymbol<distributed2::DFArray>( ),
                nl->Second( nl->First( args ) ) );
        }
        else if( DRel::checkType( nl->First( args ) ) ) {
             result = nl->TwoElemList(
                 listutils::basicSymbol<distributed2::DArray>( ),
                 nl->Second( nl->First( args ) ) );
        }
        else if( distributed2::DFArray::checkType( nl->First( args ) ) ) {

            if(!Relation::checkType( nl->Second( nl->First( args ) ) ) ) {
                return listutils::typeError( err +
                ": first argument is not a d[f]array with a relation" );
            }

            result = nl->ThreeElemList(
                listutils::basicSymbol<DFRel>( ),
                nl->Second( nl->First( args ) ),
                nl->OneElemList( nl->IntAtom( random ) ) );
        }
        else if( distributed2::DArray::checkType( nl->First( args ) ) ) {

            if(!Relation::checkType( nl->Second( nl->First( args ) ) ) ) {
                return listutils::typeError( err +
                ": first argument is not a d[f]array with a relation" );
            }

             result = nl->ThreeElemList(
                 listutils::basicSymbol<DRel>( ),
                 nl->Second( nl->First( args ) ),
                 nl->OneElemList( nl->IntAtom( random ) ) );
        }
        else {
            return listutils::typeError( err +
                ": first argument is not a d[f]rel or a d[f]array" );
        }

        return result;
    }

/*
1.2 Value Mapping

Creates a d[f]array.

*/
    template<class T, class R>
    int drel2darrayVMT( Word* args, Word& result, int message,
        Word& local, Supplier s ) {

        result = qp->ResultStorage( s );

        T* input = ( T* )args[ 0 ].addr;
        R* res = ( R* ) result.addr;
        res->copyFrom( *input );
        // res set keepRemoteObjects 

        return 0;
    }

/*
1.3 Value Mapping

Creates a d[f]rel.

*/
    template<class T, class R>
    int darray2drelVMT( Word* args, Word& result, int message,
        Word& local, Supplier s ) {

        result = qp->ResultStorage( s );

        T* input = ( T* )args[ 0 ].addr;
        R* res = ( R* ) result.addr;
        res->copyFrom( *input );
        // res set keepRemoteObjects 

        if( res->IsDefined( ) ) {
            DistTypeBasic* distType = new DistTypeBasic( random );
            res->setDistType( distType );
        }

        return 0;
    }

/*
1.4 ValueMapping Array of drel2darray

*/
    ValueMapping drel2darrayVM[ ] = {
        drel2darrayVMT<DRel, distributed2::DArray>,
        drel2darrayVMT<DFRel, distributed2::DFArray>,
        darray2drelVMT<distributed2::DArray, DRel>,
        darray2drelVMT<distributed2::DFArray, DFRel>
    };

/*
1.5 Selection function

*/
    int drel2darraySelect( ListExpr args ) {

        string type = nl->SymbolValue( nl->First( nl->First( args ) ) );

        if( type == DRel::BasicType( ) ) {
            return 0;
        }
        else if( type == DFRel::BasicType( ) ) {
            return 1;
        }
        else if( type == distributed2::DArray::BasicType( ) ) {
            return 2;
        }
        else {
            return 3;
        }
    }

/*
1.6 Specification of drel2darray

*/
    OperatorSpec drel2darraySpec(
        "d[f]rel(X) or d[f]array "
        "-> d[f]rel(X) or d[f]array ",
        " _ drel2darray",
        "Convert a d[f]rel to a d[f]array or a dfrel to a dfarray. ",
        " query drel1 drel2darray"
    );

/*
1.7 Operator instance of drel2darray operator

*/
    Operator drel2darrayOp(
        "drel2darray",
        drel2darraySpec.getStr( ),
        4,
        drel2darrayVM,
        drel2darraySelect,
        drel2darrayTM
    );

} // end of namespace drel