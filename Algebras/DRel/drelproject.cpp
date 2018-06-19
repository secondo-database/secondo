
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



1 Implementation of the secondo operator drelproject

*/
#include "NestedList.h"
#include "ListUtils.h"
#include "QueryProcessor.h"
#include "StandardTypes.h"

#include "Algebras/Stream/Stream.h"
#include "Algebras/Relation-C++/OperatorProject.h"
#include "Algebras/FText/FTextAlgebra.h"

#include "DRel.h"
#include "DRel2StreamInfo.h"

extern NestedList* nl;
extern QueryProcessor* qp;

namespace distributed2 {
    ListExpr dmapTM( ListExpr args );

    template<class A>
    int dmapVMT( Word* args, Word& result, int message,
        Word& local, Supplier s );

    template<class T, class A>
    int dsummarizeVMT( Word* args, Word& result, int message,
        Word& local, Supplier s );
}

using namespace distributed2;

namespace drel {

    /*
    1.1 Type Mapping

    Expect a drel with a function to filter the tuples.

    */
    ListExpr drelprojectTM( ListExpr args ) {

        std::string err = "d[f]rel(X) x attrlist expected";

        if( !nl->HasLength( args, 2 ) ) {
            return listutils::typeError( err + ": two arguments required" );
        }

        if( !nl->HasLength( nl->First( args ), 2 ) ) {
            return listutils::typeError( "internal Error" );
        }

        ListExpr drelType = nl->First( nl->First( args ) );
        std::string drelName = nl->ToString( nl->Second( nl->First( args ) ) );

        if( !DRel::checkType( drelType )
            && !DFRel::checkType( drelType ) ) {
            return listutils::typeError( err +
                ": first argument is not a d[f]rel" );
        }

        ListExpr relType = nl->Second( drelType );
        ListExpr attributes = nl->TheEmptyList( );
        ListExpr temp = nl->Rest( args );
        while( !nl->IsEmpty( temp ) ) {

            if( !nl->HasLength( nl->First( temp ), 2 ) ) {
                return listutils::typeError( "internal Error" );
            }

            if( !nl->AtomType( nl->First( nl->First( temp ) ) )
                || !nl->AtomType( nl->Second( nl->First( temp ) ) ) ) {

                return listutils::typeError( err +
                    ": argument list not correct. Has to be a list of " +
                    "attributes." );
            }

            if( nl->IsEmpty( attributes ) ) {
                attributes = nl->First( nl->First( temp ) );
            } else {
                attributes = nl->Cons( attributes,
                    nl->First( nl->First( temp ) ) );
            }

            temp = nl->Rest( temp );
        }

        ListExpr result = OperatorProject::ProjectTypeMap(
            nl->TwoElemList(
                nl->TwoElemList(
                    listutils::basicSymbol<Stream<Tuple> >( ),
                    nl->Second( relType ) ),
                attributes ) );

        if( !nl->HasLength( result, 3 ) ) {
            return result;
        }

        std::string fun = "( fun( dmapelem1 ARRAYFUNARG1 )( project"
            "( feed dmapelem1 )" + nl->ToString( attributes ) + " ) )";


        return nl->ThreeElemList(
            nl->SymbolAtom( Symbol::APPEND( ) ),
            nl->TwoElemList(
                nl->StringAtom( drelName ),
                nl->TextAtom( fun ) ),
            nl->Third( result ) ); // result stream
    }

    /*
    1.2 Value Mapping

    Uses a drel and project the choosen attributes.
    Result is a new drel.

    */
    template<class R, class T>
    int drelprojectVMT( Word* args, Word& result, int message,
        Word& local, Supplier s ) {

        DRel2StreamInfo<T>* info = ( DRel2StreamInfo<T>* )local.addr;

        switch( message ) {

        case OPEN:
        {
            if( info ) {
                delete info;
            }

            CcString* drelName = ( CcString* )args[ 2 ].addr;
            FText* fun = ( FText* )args[ 3 ].addr;

            // create a query to use the dmapVMT onlay OpTree is nessesary
            ListExpr dmapFun;
            nl->ReadFromString(
                "( dmap ( convert2darray " + drelName->GetValue( ) + " )"
                "\"\"" + fun->GetValue( ) + " )", dmapFun );

            QueryProcessor* qps = new QueryProcessor( nl, am );
            bool correct = false;
            bool evaluable = false;
            bool defined = false;
            bool isFunction = false;
            ListExpr resultType;
            OpTree tree = 0;

            qps->Construct(
                dmapFun,
                correct,
                evaluable,
                defined,
                isFunction,
                tree,
                resultType );

            cout << "correct" << endl;
            cout << correct << endl;

            ArgVector dmapArgVec = {
                args[ 0 ].addr,
                new CcString( "" ),
                args[ 1 ].addr,
                fun,
                new CcBool( true, false ),
                new CcBool( true, true )
            };

            Word dmapResult;
            Word dmapLocal;
            dmapVMT<T>( dmapArgVec, dmapResult, message, dmapLocal, tree );

            ArgVector argVec = { dmapResult.addr };
            Word dsummarizeLocal;
            int res = dsummarizeVMT<dsummarizeRelInfo<T>, T>( argVec, result,
                message, dsummarizeLocal, s );

            info = new DRel2StreamInfo<T>(
                &argVec, dsummarizeLocal.addr, qps, &tree );
            local.addr = info;

            return res;

            break;
        }
        case REQUEST:
        {
            return dsummarizeVMT<dsummarizeRelInfo<T>, T>( *( info->args ),
                result, message, info->local, s );

            break;
        }
        case CLOSE:
        {
            dsummarizeVMT<dsummarizeRelInfo<T>, T>( *( info->args ), result,
                message, info->local, s );

            if( info ) {
                delete info;
                local.addr = 0;
            }

            break;
        }
        }

        return -1;
    }

    /*
    1.3 Specification of drelfilter

    */
    OperatorSpec drelprojectSpec(
        " drel(X) x list "
        "-> drel(X) ",
        " _ drelproject[list]",
        "Passed only the listed attributes to the new drel",
        " query drel1 drelproject[PLZ, Ort]"
    );

    /*
    1.4 ValueMapping Array of drelfilter

    */
    ValueMapping drelprojectVM[ ] = {
        drelprojectVMT<DRel, DArray>,
        drelprojectVMT<DFRel, DFArray>
    };

    /*
    1.5 Selection function of drelfilter

    */
    int drelprojectSelect( ListExpr args ) {

        return DRel::checkType( nl->First( args ) ) ? 0 : 1;
    }

    /*
    1.6 Operator instance of drelfilter operator

    */
    Operator drelprojectOp(
        "drelproject",
        drelprojectSpec.getStr( ),
        2,
        drelprojectVM,
        drelprojectSelect,
        drelprojectTM
    );

} // end of namespace drel