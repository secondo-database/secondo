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


1 Implementation of the secondo operators drelimport

*/
//#define DRELDEBUG

#include "NestedList.h"
#include "ListUtils.h"
#include "QueryProcessor.h"
#include "StandardTypes.h"
#include "SecParser.h"

#include "Algebras/Distributed2/CommandLogger.h"
#include "Algebras/Distributed2/Distributed2Algebra.h"
#include "Algebras/Stream/Stream.h"
#include "Algebras/FText/FTextAlgebra.h"

#include "DRelHelpers.h"
#include "DRel.h"

extern NestedList* nl;
extern QueryProcessor* qp;

namespace drel {

/*
1.1 Type Mappings ~file2streamTM~

Type mapping of the operator file2stream

*/
    ListExpr file2streamTM( ListExpr args ) {

        string err = "string|text expected";

        ListExpr arg1Type = nl->First( args );

        if( !FText::checkType( arg1Type )
         && !CcString::checkType( arg1Type ) ) {
            return listutils::typeError(
                err + ": first argument is not a string or text" );
        }

        ListExpr resultType = nl->TwoElemList(
                listutils::basicSymbol<Stream<Tuple>>( ),
                nl->TwoElemList(
                    listutils::basicSymbol<Tuple>( ),
                    nl->OneElemList(
                        nl->TwoElemList(
                            nl->SymbolAtom( "Data" ),
                            listutils::basicSymbol<FText>( ) ) ) ) );

        return resultType;
    }

    template<class R>
    class FileReader {
        public:
            FileReader( string _file ) : file( _file ) {

                f.open( file, ios::in );

                SecondoCatalog* sc = SecondoSystem::GetCatalog();
                tupleType = sc->NumericType( 
                    nl->TwoElemList(
                        listutils::basicSymbol<Tuple>( ),
                        nl->OneElemList(
                            nl->TwoElemList(
                                nl->SymbolAtom( "Data" ),
                                listutils::basicSymbol<FText>( ) ) ) ) );
            }

            ~FileReader() {

            }

            Tuple* getTuple( ) {

                if( !f.is_open( ) ) {
                    return 0;
                }

                if( f.eof( ) ) {
                    f.close( );
                    return 0;
                }

                string line;
                getline( f, line );

                #ifdef DRELDEBUG
                cout << "line: " << line << endl;
                #endif

                // skip empty lines
                while( line == "" ) {
                    getline( f, line );
                    if( f.eof( ) ) {
                        f.close( );
                        return 0;
                    }
                }

                Tuple* t = new Tuple( tupleType );
                t->PutAttribute( 0, new FText( true, line ) );

                return t;
            }

        private:
            ListExpr tupleType;
            string file;
            ifstream f;

    };

/*
1.2 Value Mapping ~file2streamVMT~

Reads a file and create a drel.

*/
    template<class R>
    int file2streamVMT( Word* args, Word& result, int message,
        Word& local, Supplier s ) {

        #ifdef DRELDEBUG
        cout << "file2streamVMT" << endl;
        #endif

        FileReader<R>* reader = ( FileReader<R>* )local.addr;

        switch(message) {

            case OPEN: {

                string filename = ( ( R* )args[ 0 ].addr )->GetValue( );

                #ifdef DRELDEBUG
                cout << "OPEN" << endl;
                cout << "filename: " << filename << endl;
                #endif

                if ( reader ) {
                    delete reader;
                }

                local.addr = new FileReader<R>( filename );
                
                return 0;
            }

            case REQUEST: {

                #ifdef DRELDEBUG
                cout << "REQUEST" << endl;
                #endif
               
                result.addr = reader ? reader->getTuple( ) : 0;
                return result.addr ? YIELD : CANCEL;
            }

            case CLOSE: {

                #ifdef DRELDEBUG
                cout << "CLOSE" << endl;
                #endif

                if ( reader ) {
                    delete reader;
                    local.addr = 0;
                }

                return 0;
            }

        }
        return 0;
    }

/*
1.3 ValueMapping Array for file2stream

*/
    ValueMapping file2streamVM[ ] = {
        file2streamVMT<CcString>,
        file2streamVMT<FText>
    };

/*
1.4 Selection function for int file2stream

*/
    int file2streamSelect( ListExpr args ) {

        return CcString::checkType( nl->First( args ) ) ? 0 : 1;
    }

/*
1.5 Specification for the operator int file2stream

*/
    OperatorSpec file2streamSpec(
        " string|text -> stream(tuple((Data text)))",
        " file2stream(_)",
        "Imports a file and create a stream with the schema "
        "stream(tuple((Data text)))",
        " query int file2stream(\"import.txt\")"
    );

/*
1.6 Operator instance of the operator int file2stream

*/
    Operator file2streamOp(
        "file2stream",
        file2streamSpec.getStr( ),
        2,
        file2streamVM,
        file2streamSelect,
        file2streamTM
    );

} // end of namespace drel