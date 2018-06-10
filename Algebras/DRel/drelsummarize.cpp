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



1 Implementation of the secondo operator compareDistType

*/
#include "DRel.h"
#include "NestedList.h"
#include "ListUtils.h"
#include "QueryProcessor.h"
#include "StandardTypes.h"

#include "Boundary.h"
#include "DRelHelpers.h"

extern NestedList* nl;
extern QueryProcessor* qp;

namespace distributed2 {

    ListExpr dsummarizeTM( ListExpr args );

    template<class T, class A>
    int dsummarizeVMT( Word* args, Word& result, int message,
        Word& local, Supplier s );

    template<class A>
    class dsummarizeRelInfo;
}

using namespace distributed2;

namespace drel {

    /*
    1.1 Type Mapping

    Expect a DRel or a DFRel.

    */
    ListExpr drelsummarizeTM( ListExpr args ) {

        std::string err = "d[f]rel expected";

        if( ! nl->HasLength( args, 1 ) ) {
            return listutils::typeError( err + 
                ": one argument is expected" );
        }

        if( DFRel::checkType( nl->First( args ) ) ) {

            return dsummarizeTM( nl->OneElemList( nl->TwoElemList( 
                nl->SymbolAtom( DFArray::BasicType( ) ), 
                nl->Second( nl->First( args ) ) ) ) );
        }
        if( DRel::checkType( nl->First( args ) ) ) {

            return dsummarizeTM( nl->OneElemList( nl->TwoElemList( 
                nl->SymbolAtom( DArray::BasicType( ) ), 
                nl->Second( nl->First( args ) ) ) ) );
        }

        return listutils::typeError( err +
            ": first argument is not a drel" );
    }

    /*
    1.2 Value Mapping

    Selects all elements of the drel or dfrel from the workers.

    */
    template<class T, class R>
    int drelsummarizeVMT( Word* args, Word& result, int message,
        Word& local, Supplier s ) {

        return dsummarizeVMT<T, R>( args, result, message, local, s );
    }

    /*
    1.3 ValueMapping Array of drelsummarize

    */
    ValueMapping drelsummarizeVM[ ] = {
        drelsummarizeVMT<dsummarizeRelInfo<DArray>, DArray >,
        drelsummarizeVMT<dsummarizeRelInfo<DFArray>, DFArray >
    };

    /*
    1.4 Selection function of drelsummarize

    */
    int drelsummarizeSelect( ListExpr args ) {

        return DRel::checkType( nl->First( args ) ) ? 0 : 1;
    }

    /*
    1.5 Specification of drelsummarize

    */
    OperatorSpec drelsummarizeSpec(
        "d[f]rel(rel(X)) -> stream(X)",
        "_ drelsummarize",
        "Produces a stream of the drel elements.",
        "query drel1 dsummarize count"
    );

    /*
    1.6 Operator instance of drelsummarize operator

    */
    Operator drelsummarizeOp(
        "drelsummarize",
        drelsummarizeSpec.getStr( ),
        2,
        drelsummarizeVM,
        drelsummarizeSelect,
        drelsummarizeTM
    );

} // end of namespace drel