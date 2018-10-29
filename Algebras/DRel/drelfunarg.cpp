
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



1 Implementation of the type operator DRELFUNARGx

*/
#include "NestedList.h"
#include "ListUtils.h"
#include "QueryProcessor.h"
#include "StandardTypes.h"

#include "Algebras/Distributed2/frel.h"
#include "Algebras/Distributed2/fsrel.h"

#include "DRel.h"

extern NestedList* nl;
extern QueryProcessor* qp;

namespace distributed2 {
    template<int pos, bool makeFS>
    ListExpr ARRAYFUNARG( ListExpr args );
}

using namespace distributed2;

namespace drel {

/*
1.1 Type Mapping

*/
    template<bool rel, int pos>
    ListExpr DRELFUNARG( ListExpr args ) {

        if( !nl->HasMinLength( args, pos ) ) {
            return listutils::typeError( "too less arguments" );
        }
        for( int i = 1; i < pos; i++ ) {
            args = nl->Rest( args );
        }

        ListExpr arg = nl->First( args );
        if( DRel::checkType( arg )
         || DFRel::checkType( arg ) ) {
            cout << rel << endl;
            if( rel ) {
                return nl->Second( arg );
            }
            else {
                return nl->Second( nl->Second( arg ) );
            }
        }

        return listutils::typeError( "Invalid type found" );

    }

/*
1.2 Specifications of type map operators.

*/
    OperatorSpec DRELFUNARG1SPEC(
        "d[f]rel(rel(X)) x ... -> X",
        "DRELFUNARG1(_)",
        "Type mapping operator.",
        "query drel1 drelfilter[.PLZ=99998] drelsummarize consume"
    );

    OperatorSpec DRELFUNARG2SPEC(
        "d[f]rel(rel(X)) x ... -> X",
        "DRELFUNARG2(_)",
        "Type mapping operator.",
        "query drel1 drelprojectextend[PLZ; PLZ2 : .PLZ + 2] drelsummarize"
        "consume"
    );

    OperatorSpec DRELRELFUNARG1SPEC(
        "d[f]rel(rel(X)) x ... -> rel(X)",
        "DRELRELFUNARG1(_)",
        "Type mapping operator.",
        "query drel1 drellgroupby[PLZ; Anz: group feed count] drelsummarize "
        "consume"
    );

/*
1.3 Operator instance of the type map operators.

*/
    Operator DRELFUNARG1OP(
        "DRELFUNARG1",
        DRELFUNARG1SPEC.getStr( ),
        0,
        Operator::SimpleSelect,
        DRELFUNARG<false, 1>
    );

    Operator DRELFUNARG2OP(
        "DRELFUNARG2",
        DRELFUNARG2SPEC.getStr( ),
        0,
        Operator::SimpleSelect,
        DRELFUNARG<false, 2>
    );
    
    Operator DRELRELFUNARG1OP(
        "DRELRELFUNARG1",
        DRELRELFUNARG1SPEC.getStr( ),
        0,
        Operator::SimpleSelect,
        DRELFUNARG<true, 1>
    );

    template<int pos, bool makeFS>
    ListExpr DRELFUNARGTTT( ListExpr args ) {

        if( !nl->HasMinLength( args, pos ) ){
            return listutils::typeError( "too less arguments" );
        }
        for( int i = 1; i < pos; i++ ){
            args = nl->Rest( args );
        }
        ListExpr arg = nl->First( args );

        if( DArray::checkType( arg ) 
         || SDArray::checkType( arg )
         || DRel::checkType( arg ) ){
            return nl->Second( arg );
        }

        if( DFArray::checkType( arg )
         || DRel::checkType( arg )
         || DFMatrix::checkType( arg ) ) {

            ListExpr res;
            if( makeFS ) {
            res  = nl->TwoElemList(
                    listutils::basicSymbol<fsrel>( ),
                    nl->Second( nl->Second( arg ) ) );
            } 
            else {
            res  = nl->TwoElemList(
                    listutils::basicSymbol<frel>( ),
                    nl->Second( nl->Second( arg ) ) );
            }
            return res;
        }

        return listutils::typeError("Invalid type found");
    }

    OperatorSpec DRELFUNARGTTT1SPEC(
        "d[f]rel(rel(X)) x ... -> frel(X)",
        "DRELFUNARGTTT1(_)",
        "Type mapping operator.",
        "query drel1 drellgroupby[PLZ; Anz: group feed count] drelsummarize "
        "consume"
    );

    OperatorSpec DRELFUNARGTTT2SPEC(
        "any x d[f]rel(rel(X)) ... -> frel(X)",
        "DRELFUNARGTTT2(_)",
        "Type mapping operator.",
        "query drel1 drellgroupby[PLZ; Anz: group feed count] drelsummarize "
        "consume"
    );

    Operator DRELFUNARGTTT1OP(
        "DRELFUNARGTTT1",
        DRELFUNARGTTT1SPEC.getStr( ),
        0,
        Operator::SimpleSelect,
        DRELFUNARGTTT<1, false>
    );

    Operator DRELFUNARGTTT2OP(
        "DRELFUNARGTTT2",
        DRELFUNARGTTT2SPEC.getStr( ),
        0,
        Operator::SimpleSelect,
        DRELFUNARGTTT<2, false>
    );

} // end of namespace drel