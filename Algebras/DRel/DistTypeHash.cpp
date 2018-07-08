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
#define DRELDEBUG

#include <iterator>
#include <assert.h>

#include "DistTypeHash.h"
#include "StandardTypes.h"

extern NestedList* nl;

using namespace distributed2;

namespace drel {
    /*
    6 Class ~DistTypeHash~

    Implementation.

    6.1 Constructors

    */
    DistTypeHash::DistTypeHash( distributionType _type, int _attr ) :
        DistTypeBasic( _type ), attr( _attr ) {
        #ifdef DRELDEBUG
        cout << "DistTypeHash constructor" << endl;
        #endif
    }

    /*
    6.2 Copyconstructor

    */
    DistTypeHash::DistTypeHash( const DistTypeHash& _distType ) :
        DistTypeBasic( _distType ), attr( _distType.attr ) {
        #ifdef DRELDEBUG
        cout << "DistTypeHash constructor" << endl;
        #endif
    }

    /*
    6.3 Assignment operator

    */
    DistTypeHash& DistTypeHash::operator=( const DistTypeHash& _distType ) {

        #ifdef DRELDEBUG
        cout << "DistTypeHash assignment operator" << endl;
        #endif

        if( this == &_distType ) {
            return *this;
        }
        DistTypeBasic::operator=( _distType );
        attr = _distType.attr;
        return *this;
    }

    /*
    6.4 Destructor

    */
    DistTypeHash::~DistTypeHash( ) {
    }

    /*
    6.5 ~isEqual~

    Compares the current DistType with another one.

    */
    bool DistTypeHash::isEqual( DistTypeBasic* _distType ) {

        #ifdef DRELDEBUG
        cout << "DistTypeHash::isEqual" << endl;
        #endif

        if( typeid( *_distType ) != typeid( *this ) ) {
            return false;
        }

        return getDistType( ) == _distType->getDistType( );
    }

    /*
    6.6 ~getAttr~

    Returns the number of the attribute used to distribute by hash.

    */
    int DistTypeHash::getAttr( ) {

        #ifdef DRELDEBUG
        cout << "DistTypeHash::getAttr" << endl;
        #endif

        return attr;
    }

    /*
    6.7 ~copy~

    Make a copy of the current object.

    */
    DistTypeBasic* DistTypeHash::copy( ) {

        #ifdef DRELDEBUG
        cout << "DistTypeHash::copy" << endl;
        #endif

        return new DistTypeHash( *this );
    }

    /*
    6.8 ~checkType~

    Checks whether the type in nested list format fits to this disttype.

    */
    bool DistTypeHash::checkType( ListExpr list ) {

        #ifdef DRELDEBUG
        cout << "DistTypeHash::checkType" << endl;
        #endif

        if( !nl->HasLength( list, 2 ) ) {
            return false;
        }
        
        if( !CcString::checkType( nl->First( list ) ) ) {
            return false;
        }

        return CcInt::checkType( nl->Second( list ) );
    }

    /*
    6.9 ~save~

    Writes a DistType to the storage.

    */
    bool DistTypeHash::save( SmiRecord& valueRecord, size_t& offset, 
        const ListExpr typeInfo ) {

        #ifdef DRELDEBUG
        cout << "DistTypeHash::save" << endl;
        cout << "typeInfo" << endl;
        cout << nl->ToString( typeInfo ) << endl;
        #endif

        if( !DistTypeBasic::save( 
            valueRecord, offset, nl->OneElemList( nl->First( typeInfo ) ) ) ) {
            return false;
        }

        return true;
    }

    /*
    6.10 ~toListExpr~

    Returns the object as a list.

    */
    ListExpr DistTypeHash::toListExpr( ListExpr typeInfo ) {

        #ifdef DRELDEBUG
        cout << "DistTypeHash::toListExpr" << endl;
        #endif

        return nl->TwoElemList(
            nl->StringAtom( getName( getDistType( ) ) ), nl->IntAtom( attr ) );
    }

    /*
    6.11 ~print~

    Prints the dist type informations. Used for debugging.

    */
    void DistTypeHash::print( ) {
        DistTypeBasic::print( );
        cout << "attr" << endl;
        cout << attr << endl;
    }

    /*
    6.12 ~computeNewAttrPos~

    Computes the new position of the attribute used to distribute. Used for 
    operations like a projection.

    */
    bool DistTypeHash::computeNewAttrPos( ListExpr attrPosList, int& attrPos ) {

        #ifdef DRELDEBUG
        cout << "DistTypeHash::computeNewAttrPos" << endl;
        cout << "attrPosList" << endl;
        cout << nl->ToString( attrPosList ) << endl;
        cout << "attrPos" << endl;
        cout << attrPos << endl;
        #endif

        assert( DRelHelpers::listOfIntAtoms( attrPosList ) );

        int pos = 0;

        while( !nl->IsEmpty( attrPosList ) ) {

            if( nl->IntValue( nl->First( attrPosList ) ) == attrPos ) {
                attrPos = pos;
                return true;
            }
            pos++;

            attrPosList = nl->Rest( attrPosList );
        }

        return false;
    }

} // end of namespace drel