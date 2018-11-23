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

*/
#ifndef _BoundaryCalculator_h_
#define _BoundaryCalculator_h_

#include "Algebras/Collection/CollectionAlgebra.h"
#include "DRelHelpers.h"
#include "DRel.h"

//#define DRELDEBUG

extern NestedList* nl;
extern QueryProcessor* qp;

namespace distributed2 {

    ListExpr partitionTM(ListExpr args);

    template<class A>
    int partitionVMT( Word* args, Word& result, int message,
                    Word& local, Supplier s );
}

namespace drel {
/*
1 ~Partitioner~

Class to calculate boundaries for partitioning by range.

*/
    template<class R>
    class BoundaryCalculator {

    public:
/*
1.1 Constructors

*/
        BoundaryCalculator( std::string _attr, ListExpr _boundaryType, R* _drel, 
            ListExpr _sourcedType, int _port ) :
            attr( _attr ), boundaryType( _boundaryType ), drel( _drel ), 
            sourcedType( _sourcedType ), 
            count( -1 ), boundary( 0 ),
            port( _port ) {
        }

        BoundaryCalculator( std::string _attr, ListExpr _boundaryType, R* _drel, 
            ListExpr _sourcedType, int _port, int _count ) :
            attr( _attr ), boundaryType( _boundaryType ), drel( _drel ), 
            sourcedType( _sourcedType ), count( _count ), boundary( 0 ),
            port( _port ) {

            if( count < -1 ) {
                count = -1;
            }
        }

/*
1.2 Copy-Constructor

*/
        BoundaryCalculator( const BoundaryCalculator& src ) :
            attr( src.attr ), boundaryType( src.boundaryType ), 
            drel( src.drel ), sourcedType( src.sourcedType ), 
            count( src.count ), boundary( *( src.boundary ) ), 
            port( src.port ) {
            boundary->Copy( );
        }

/*
1.2 Assignment operator

*/
        BoundaryCalculator& operator=( const BoundaryCalculator& src ) {
            if( this == &src ) {
                return *this;
            }
            attr = src.attr;
            boundaryType = src.boundaryType;
            drel = src.drel;
            sourcedType = src.sourcedType;
            boundary = src.boundary;
            port = src.port;

            return *this;
        }

/*
1.3 Destructor

*/
        ~BoundaryCalculator( ) {
            if( boundary ) {
                boundary->DeleteIfAllowed( );
            }
        }

/*
1.4 ~computeBoundary~

Computes a boundary object.

*/
        bool computeBoundary( ) {

            cout << endl;
            cout << "Start: Create boundary object ..." << endl;

            if( count < 0 ) {
                if( !countDRel( ) ) {
                    return false;
                }
            }

            int sampleSize = DRelHelpers::computeSampleSize( count );
            int nthSample = DRelHelpers::everyNthTupleForSample( 
                sampleSize, count );

            std::string query =
            "(createboundary (sort (dsummarize (dmap (drelconvert "
            "(" + nl->ToString( sourcedType ) + " (ptr " + 
            nl->ToString( listutils::getPtrList( drel ) ) + "))) \"\" "
            "(fun (dmapelem1 ARRAYFUNARG1) (project (nth (feed "
            "dmapelem1) " + std::to_string( nthSample ) + " FALSE) (" + attr + 
            ")))))) " + attr + " " + std::to_string( drel->getSize( ) ) + " " +
            std::to_string( sampleSize ) + ")";

            Word result;
            if( !QueryProcessor::ExecuteQuery( query, result ) ) {
                cout << "ERROR: Create boundary object failed!" << endl;
                return false;
            }

            boundary = static_cast<collection::Collection*>( result.addr );

            if( !boundary ) {
                cout << "ERROR: Create boundary object failed!" << endl;
                return false;
            }
            else if( !boundary->IsDefined( ) ) {
                cout << "ERROR: Create boundary object failed!" << endl;
                delete boundary;
                boundary = 0;
                return false;
            }

            cout << "Done. Boundary object created!" << endl;

            return true;
        }

/*
1.5 ~countDRel~

Computes the number of tuple in the given drel.

*/
        bool countDRel( ) {

            cout << endl;
            cout << "Start: Compute the size of the drel ..." << endl;

            std::string query =
            "(tie (getValue (dmap (drelconvert (" + 
            nl->ToString( sourcedType ) +
            " (ptr " + nl->ToString( listutils::getPtrList( drel ) ) + ")))"
            " \"\" (fun (dmapelem1 ARRAYFUNARG1) (count dmapelem1)))) "
            "(fun (first2 ELEMENT) (second3 ELEMENT) (+first2 second3)))";

            Word result;
            if( !QueryProcessor::ExecuteQuery( query, result ) ) {
                cout << "ERROR: Computation of the drel size failed!" << endl;
                return false;
            }

            CcInt* res = ( CcInt* )result.addr;

            if( !res ) {
                cout << "ERROR: Computation of the drel size failed!" << endl;
                return false;
            }

            if( !res->IsDefined( ) ) {
                cout << "ERROR: Computation of the drel size failed!" << endl;
                delete res;
                count = 0;
                return false;
            }

            count = res->GetValue( );
            delete res;

            cout << "Done. DRel size: " + std::to_string( count ) << endl;

            return true;
        }

/*
1.10 get functions

This functions will compute the objects if they are not already created.

1.11.1 ~getCount~

*/
        int getCount( ) {

            return count;
        }


/*
1.10.2 ~getBoundary~

*/
        collection::Collection* getBoundary( ) {
            
            if( boundary ) {
                boundary->Copy( );
            }

            return boundary;
        }

/*
1.10.3 ~getArrayType~

*/
        ListExpr getArrayType( ) {
            ListExpr arrayType;

            if( nl->ToString( nl->First( sourcedType ) ) == 
                DRel::BasicType( ) ) {

                arrayType = nl->TwoElemList(
                    listutils::basicSymbol<distributed2::DArray>( ),
                    nl->Second( sourcedType ) );
            }
            else if( nl->ToString( nl->First( sourcedType ) ) == 
                DFRel::BasicType( ) ) {

                arrayType = nl->TwoElemList(
                    listutils::basicSymbol<distributed2::DFArray>( ),
                    nl->Second( sourcedType ) );
            }

            return arrayType;
        }

/*
1.11 set functions

1.11.1 ~setCount~

*/

        void setCount( int _count ) {

            count = _count;
        }

    private:

/*
1.12 ~createDRelPointerList~

*/
        ListExpr createDRelPointerList( ) {

            return nl->TwoElemList(
                getArrayType( ),
                nl->TwoElemList(
                    nl->SymbolAtom( "ptr" ),
                    listutils::getPtrList( drel ) ) );
        }

/*
1.13 Members

*/
        std::string attr;
        ListExpr boundaryType;
        R* drel;
        ListExpr sourcedType;
        int count;
        collection::Collection* boundary;
        int port;

    };
    
} // end of namespace drel

#endif // _BoundaryCalculator_h_