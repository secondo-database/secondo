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
    }

    /*
    6.2 Copyconstructor

    */
    DistTypeHash::DistTypeHash( const DistTypeHash& _distType ) :
        DistTypeBasic( _distType ), attr( _distType.attr ) {
    }

    /*
    6.3 Assignment operator

    */
    DistTypeHash& DistTypeHash::operator=( const DistTypeHash& _distType ) {
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
        return attr;
    }

    /*
    6.7 ~copy~

    Make a copy of the current object.

    */
    DistTypeBasic* DistTypeHash::copy( ) {
        return new DistTypeHash( *this );
    }

    /*
    6.8 ~getTypeExpr~

    Returns the Typelist of the disttype. For the basic type it is a 
    CcString and a CcInt.

    */
    ListExpr DistTypeHash::getTypeList( ) {
        return nl->TwoElemList( 
            listutils::basicSymbol<CcString>( ),
            listutils::basicSymbol<CcInt>( ) );
    }

    /*
    6.9 ~checkType~

    Checks whether the type in nested list format fits to this disttype.

    */
    bool DistTypeHash::checkType( ListExpr list ) {
        if( !nl->HasLength( list, 2 ) ) {
            return false;
        }
        
        if( !CcString::checkType( nl->First( list ) ) ) {
            return false;
        }

        return CcInt::checkType( nl->Second( list ) );
    }

    /*
    6.10 ~save~

    Writes a DistType to the storage.

    */
    bool DistTypeHash::save( SmiRecord& valueRecord, size_t& offset, 
        const ListExpr typeInfo ) {

        if( !DistTypeBasic::save( 
            valueRecord, offset, nl->First( typeInfo ) ) ) {
            return false;
        }
        return distributed2::writeVar( attr, valueRecord, offset );
    }

    /*
    6.11 ~readFrom~

    Reads a list an creates a DistType.

    */
    DistTypeHash* DistTypeHash::readFrom( const ListExpr _list ) {
        if( !nl->HasLength( _list, 2 ) ) {
            return 0;
        }
        distributionType tType;
        if( !readType( nl->First( _list ), tType ) ) {
            return 0;
        }
        int tAttr;
        if( !readAttr( nl->Second( _list ), tAttr ) ) {
            return 0;
        }
        return new DistTypeHash( tType, tAttr );
    }

    /*
    6.12 ~readAttr~

    Reads an attribute from a list.

    */
    bool DistTypeHash::readAttr( const ListExpr _list, int& _attr ) {
        if( !nl->IsAtom( _list ) ) {
            return false;
        }
        if( nl->AtomType( _list ) != IntType ) {
            return false;
        }
        _attr = nl->IntValue( _list );
        return true;
    }

    /*
    6.13 ~toListExpr~

    Returns the object as a list.

    */
    ListExpr DistTypeHash::toListExpr( ListExpr typeInfo ) {
        return nl->TwoElemList(
            nl->StringAtom( getName( getDistType( ) ) ), nl->IntAtom( attr ) );
    }

} // end of namespace drel