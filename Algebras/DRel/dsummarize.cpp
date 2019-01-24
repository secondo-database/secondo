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



1 Implementation of the secondo operator drelsummarize

drelsummarize uses a d[f]rel and creates a stream of tuple.

*/
//#define DRELDEBUG

#include "DRel.h"
#include "NestedList.h"
#include "ListUtils.h"
#include "QueryProcessor.h"
#include "StandardTypes.h"
#include "Algebras/Stream/Stream.h"

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

Expect a d[f]el.

*/
    ListExpr dsummarizeTM( ListExpr args ) {

        #ifdef DRELDEBUG
        cout << "dsummarizeTM" << endl;
        cout << "args" << endl;
        cout << nl->ToString( args ) << endl;
        #endif

        std::string err = "d[f]rel expected";

        if( !nl->HasLength( args, 1 ) ) {
            return listutils::typeError( err + 
                ": one argument is expected" );
        }

        distributionType type;
        if( !DRelHelpers::drelCheck( nl->First( args ), type ) ) {
            return listutils::typeError( err + 
                ": first argument is not a d[f]rel" );
        }

        ListExpr rel = nl->Second( nl->First( args ) );
        ListExpr attrList = nl->Second( nl->Second( rel ) );
        if( type == spatial2d || type == spatial3d || type == replicated ) {
            attrList = DRelHelpers::removeAttrFromAttrList( 
                attrList, "Original" );
            if( type != replicated ) {
                attrList = DRelHelpers::removeAttrFromAttrList( 
                    attrList, "Cell" );
            }
        }

        return nl->TwoElemList(
            listutils::basicSymbol<Stream<Tuple>>( ),
            nl->TwoElemList(
                listutils::basicSymbol<Tuple>( ),
                attrList ) );
    }

/*
1.2 Value Mapping

Selects all elements of the d[f]rel from the workers.

*/
    template<class T>
    int dsummarizeVMT( Word* args, Word& result, int message,
        Word& local, Supplier s ) {

        #ifdef DRELDEBUG
        cout << "dsummarizeVMT" << endl;
        #endif

        return distributed2::dsummarizeVMT<dsummarizeRelInfo<T>, T>( 
            args, result, message, local, s );
    }

    class DRelLocalSummarize {
        public:
        DRelLocalSummarize( DFRel* _drel ) :
            drel( _drel ), local( ( Address ) 0 ), args{ drel } {
        }

        ~DRelLocalSummarize( ) {
            delete drel;
        }
        
        DFRel* drel;
        Word local;
        ArgVector args;
    };

    template<class T, class R>
    int dsummarizeVMT( Word* args, Word& result, int message,
        Word& local, Supplier s ) {

        #ifdef DRELDEBUG
        cout << "dsummarizeVMT 2" << endl;
        #endif

        DRelLocalSummarize* li = ( DRelLocalSummarize* )local.addr;

        switch( message ) {

            case OPEN: 
            {
                if( li ) delete li;

                R* drel = ( R* )args[ 0 ].addr;
                std::string queryS;
                ListExpr drelType = qp->GetType( qp->GetSon( s, 0 ) );
                ListExpr drelPtr = DRelHelpers::createdrel2darray(
                    drelType, drel );

                if( drel->getDistType( )->getDistType( ) == replicated ) {
                    queryS = "(dmap " + nl->ToString( drelPtr ) + " \"\" (fun "
                        "(dmapelem_1 ARRAYFUNARG1) (remove (filter (feed "
                        "dmapelem_1) (fun (streamelem_2 STREAMELEM) "
                        "(= (attr streamelem_2 Original) TRUE))) "
                        "(Original))))";
                }
                else {
                    queryS = "(dmap " + nl->ToString( drelPtr ) + " \"\" (fun "
                        "(dmapelem_1 ARRAYFUNARG1) (remove (filter (feed "
                        "dmapelem_1) (fun (streamelem_2 STREAMELEM) "
                        "(= (attr streamelem_2 Original) TRUE))) "
                        "(Cell Original))))";
                }

                Word dfrelResult;
                if( !QueryProcessor::ExecuteQuery( queryS, dfrelResult ) ) {
                    cout << "error while remove the dist attributes" << endl;
                    return CANCEL;
                }

                local.addr = new DRelLocalSummarize( 
                    ( DFRel* )dfrelResult.addr );

                DRelLocalSummarize* li = ( DRelLocalSummarize* )local.addr;

                return distributed2::dsummarizeVMT<
                    dsummarizeRelInfo<DFArray>, DFArray>( 
                        li->args, result, message, li->local, s );

                break;
            }
            case REQUEST:
            {
                return distributed2::dsummarizeVMT<
                    dsummarizeRelInfo<DFArray>, DFArray>( 
                        li->args, result, message, li->local, s );

                break;
            }
            case CLOSE:
            {
                distributed2::dsummarizeVMT<
                    dsummarizeRelInfo<DFArray>, DFArray>( 
                        li->args, result, message, li->local, s );
                
                if( li ) {
                    delete li;
                    local.addr = 0;
                }

                break;
            }
        }

        return -1;
    }

/*
1.3 ValueMapping Array of drelsummarize

*/
    ValueMapping dsummarizeVM[ ] = {
        dsummarizeVMT<DArray>,
        dsummarizeVMT<DFArray>,
        dsummarizeVMT<DArray, DRel>,
        dsummarizeVMT<DFArray, DFRel>
    };

/*
1.4 Selection function of dsummarize

*/
    int dsummarizeSelect( ListExpr args ) {

        distributionType type;
        DRelHelpers::drelCheck( nl->First( args ), type );
        int x = ( type == spatial2d || type == spatial3d ||
                  type == replicated ) ? 2 : 0;

        return DRel::checkType( nl->First( args ) ) ? 0 + x : 1 + x;
    }

/*
1.5 Specification of dsummarize

*/
    OperatorSpec dsummarizeSpec(
        "d[f]rel(rel(X)) -> stream(X)",
        "_ dsummarize",
        "Produces a stream of the drel elements.",
        "query drel1 dsummarize count"
    );

/*
1.6 Operator instance of dsummarize operator

*/
    Operator dsummarizeOp(
        "dsummarize",
        dsummarizeSpec.getStr( ),
        4,
        dsummarizeVM,
        dsummarizeSelect,
        dsummarizeTM
    );

} // end of namespace drel
