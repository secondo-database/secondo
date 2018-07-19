
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

Get a drel or dfrel as argument.

*/
    ListExpr convert2darrayTM( ListExpr args ) {

        std::string err = "drel or dfrel expected";

        if( ! nl->HasLength( args, 1 ) ) {
            return listutils::typeError( err + 
                ": one arguments is expected" );
        }

        if( DFRel::checkType( nl->First( args ) ) ) {
            return nl->TwoElemList(
                listutils::basicSymbol<distributed2::DFArray>( ),
                nl->Second( nl->First( args ) ) );
        }
         if( DRel::checkType( nl->First( args ) ) ) {
             return nl->TwoElemList(
                 listutils::basicSymbol<distributed2::DArray>( ),
                 nl->Second( nl->First( args ) ) );
        }

        return listutils::typeError( err +
            ": first argument is not a drel or dfrel" );
    }

/*
1.2 Value Mapping

Creates a darray or a dfarray.

*/
    template<class T, class R>
    int convert2darrayVMT( Word* args, Word& result, int message,
        Word& local, Supplier s ) {

        result = qp->ResultStorage( s );

        T* drel = ( T* )args[ 0 ].addr;
        R* res = ( R* ) result.addr;
        res->copyFrom( *drel );

        return 0;
    }

/*
1.3 ValueMapping Array of convert2darray

*/
    ValueMapping convert2darrayVM[ ] = {
        convert2darrayVMT<DRel, distributed2::DArray>,
        convert2darrayVMT<DFRel, distributed2::DFArray>
    };

/*
1.4 Selection function

*/
    int convert2darraySelect( ListExpr args ) {
        return nl->SymbolValue( 
            nl->First( nl->First( args ) ) ) == DRel::BasicType( ) ? 0 : 1;
    }

/*
1.5 Specification of convert2darray

*/
    OperatorSpec convert2darraySpec(
        " drel(X) "
        "-> darray ",
        " _ convert2darray",
        "Convert a drel to a darray or a dfrel to a dfarray. ",
        " query drel1 convert2darray"
    );

/*
1.6 Operator instance of convert2darray operator

*/
    Operator convert2darrayOp(
        "convert2darray",
        convert2darraySpec.getStr( ),
        2,
        convert2darrayVM,
        convert2darraySelect,
        convert2darrayTM
    );

} // end of namespace drel