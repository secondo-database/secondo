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

 @history
 Version 1.0 - Created - T. Beckmann - 2018
 Version 1.1 - Small improvements and programmed comparison, new - D. Selenyi - 25.07.2020

 @todo
 Nothing

*/

/*
1 Implementation of the secondo operator compareDistType

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

namespace drel {

/*
1.1 Type Mapping

Expect a DRel or DFRel and another DRel or DFRel.

*/
    ListExpr compareDistTypeTM( ListExpr args ) {

        #ifdef DRELDEBUG
        cout << "compareDistTypeTM" << endl;
        cout << nl->ToString( args ) << endl;
        #endif

        std::string err = "d[f]rel x d[f]rel expected";

        if( ! nl->HasLength( args, 2 ) ) {
            return listutils::typeError( err + 
                ": two arguments are expected" );
        }

        if( !DFRel::checkType( nl->First( args ) )
         && !DRel::checkType( nl->First( args ) ) ) {
            return listutils::typeError( err +
                ": first argument is not a d[f]rel" );
        }
        
        if( !DFRel::checkType( nl->Second( args ) )
         && !DRel::checkType( nl->Second( args ) ) ) {
            return listutils::typeError( err +
                ": second argument is not a d[f]rel" );
        }

        return listutils::basicSymbol<CcBool>( );
    }

/*
1.2 Value Mapping

Compares the disttypes of two drels. Return true, if the drels have the 
same disttype.

*/
    template<class T, class R>
    int compareDistTypeVMT( Word* args, Word& result, int message,
        Word& local, Supplier s ) {

        distributionType dType1, dType2;
        bool equal = false;

        //Purpose: Find out the Type of the d[f]rel (example: Replicated,...)
        DRelHelpers::drelCheck(qp->GetType( qp->GetSon( s, 0 ) ), dType1);
        DRelHelpers::drelCheck(qp->GetType( qp->GetSon( s, 1 ) ), dType2);

        if (dType1 == dType2)
          { equal = true;}

        result = qp->ResultStorage( s );
        CcBool* res = ( CcBool* ) result.addr;
        res->Set( true, equal );

        return 0;
    }

/*
1.3 ValueMapping Array of drfdistribute

*/
    ValueMapping compareDistTypeVM[ ] = {
        compareDistTypeVMT<DRel, DRel>,
        compareDistTypeVMT<DRel, DFRel>,
        compareDistTypeVMT<DFRel, DRel>,
        compareDistTypeVMT<DFRel, DFRel>
    };

/*
1.4 Selection function

*/
    int compareDistTypeSelect( ListExpr args ) {

        int n1 = nl->SymbolValue( nl->First( nl->First( args ) ) ) == 
            DRel::BasicType( ) ? 0 : 2;
        int n2 = nl->SymbolValue( nl->First( nl->Second( args ) ) ) == 
            DRel::BasicType( ) ? 0 : 1;
        
        return n1 + n2;
    }

/*
1.5 Specification of comparedisttype

*/
    OperatorSpec compareDistTypeSpec(
        " drel(X) x drel(X) "
        "-> bool ",
        " _ _ comparedisttype",
        "Compares to drels and return true, if the disttype are "
        "equal. ",
        " query drel1 drel2 comparedisttype"
    );

/*
1.6 Operator instance of comparedisttype operator

*/
    Operator compareDistTypeOp(
        "comparedisttype",
        compareDistTypeSpec.getStr( ),
        4,
        compareDistTypeVM,
        compareDistTypeSelect,
        compareDistTypeTM
    );

} // end of namespace drel
