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

#include "DistTypeRange.h"
#include "StandardTypes.h"

extern NestedList* nl;

using namespace distributed2;

namespace drel {
    /*
    6 Class ~DistType~

    Implementation.

    6.1 Constructors

    */
    DistTypeRange::DistTypeRange( distributionType _type, int _attr, 
        Boundary* _boundary ) :
        DistTypeHash( _type, _attr ), key( rand( ) ), boundary( _boundary ) {
    }
    
    DistTypeRange::DistTypeRange( distributionType _type, int _attr, int _key, 
        Boundary* _boundary ) :
        DistTypeHash( _type, _attr ), key( _key ), boundary( _boundary ) {
    }

    /*
    6.2 Copyconstructor

    */
    DistTypeRange::DistTypeRange( const DistTypeRange& _distType ) :
        DistTypeHash( _distType ), key( _distType.key ), 
        boundary( _distType.boundary ) {
    }

    /*
    6.3 Assignment operator

    */
    DistTypeRange& DistTypeRange::operator=( const DistTypeRange& _distType ) {
        if( this == &_distType ) {
            return *this;
        }
        DistTypeHash::operator=( _distType );
        key = _distType.key;
        boundary = _distType.boundary;
        return *this;
    }

    /*
    6.4 Destructor

    */
    DistTypeRange::~DistTypeRange( ) {
    }

    /*
    6.5 ~isEqual~

    Compares the current DistType with another one.

    */
    bool DistTypeRange::isEqual( DistTypeBasic* _distType ) {
        if( typeid( *_distType ) != typeid( *this ) ) {
            return false;
        }
        DistTypeRange* other = static_cast<DistTypeRange*>( _distType );
        return getDistType( ) == other->getDistType( )
            && getKey( ) == other->getKey( )
            && boundary->isEqual( other->getBoundary( ) );
    }

    /*
    6.6 ~getKey~

    Returns the key.

    */
    int DistTypeRange::getKey( ) {
        return key;
    }

    /*
    6.7 ~getBoundary~

    Returns the boundary.

    */
    Boundary* DistTypeRange::getBoundary( ) {
        return boundary;
    }

    /*
    6.8 ~copy~

    Make a copy of the current object.

    */
    DistTypeBasic* DistTypeRange::copy( ) {
        return new DistTypeRange( *this );
    }

    /*
    6.9 ~getTypeList~

    Returns the Typelist of the disttype. For the range type it is a
    CcString, two CcInt and one Boundary.

    */
    ListExpr DistTypeRange::getTypeList( ListExpr attrType ) {
        return nl->FourElemList(
            listutils::basicSymbol<CcString>( ),
            listutils::basicSymbol<CcInt>( ),
            listutils::basicSymbol<CcInt>( ),
            nl->TwoElemList(
                listutils::basicSymbol<Boundary>( ),
                nl->OneElemList( attrType ) ) );
    }

    /*
    6.10 ~checkType~

    Checks whether the type in nested list format fits to this disttype.

    */
    bool DistTypeRange::checkType( ListExpr list ) {
        if( !nl->HasLength( list, 4 ) ) {
            return false;
        }

        if( !CcString::checkType( nl->First( list ) ) ) {
            return false;
        }

        if( !CcInt::checkType( nl->Second( list ) ) ) {
            return false;
        }
        
        if( !CcInt::checkType( nl->Third( list ) ) ) {
            return false;
        }

        return Boundary::checkType( nl->Fourth( list ) );
    }

    /*
    6.11 ~save~

    Writes a DistType to the storage.

    */
    bool DistTypeRange::save( SmiRecord& valueRecord, 
        size_t& offset, const ListExpr typeInfo ) {

        if( !DistTypeHash::save( valueRecord, offset, 
            nl->TwoElemList( nl->First( typeInfo ), 
                nl->Second( typeInfo ) ) ) ) {
            return false;
        }
        if( !distributed2::writeVar( key, valueRecord, offset ) ) {
            return false;
        }
        if( !boundary ) {
            return false;
        }

        return boundary->save( valueRecord, offset );
    }

    /*
    6.12 ~readFrom~

    Reads a list an creates a DistType.

    */
    DistTypeRange* DistTypeRange::readFrom( const ListExpr _list ) {
        if( !nl->HasLength( _list, 4 ) ) {
            return 0;
        }

        distributionType tType;
        if( !readType( nl->First( _list ), tType ) ) {
            return 0;
        }
        if( tType != range ) {
            return 0;
        }

        int tAttr;
        if( !readAttr( nl->Second( _list ), tAttr ) ) {
            return 0;
        }

        int tKey;
        if( !readKey( nl->Third( _list ), tKey ) ) {
            return 0;
        }

        Boundary* boundary = 0;
        // Fehlt noch
        
        return new DistTypeRange( tType, tAttr, tKey, boundary );
    }

    /*
    6.13 ~readKey~

    Reads the key from a list.

    */
    bool DistTypeRange::readKey( const ListExpr _list, int& _key ) {
        if( !nl->IsAtom( _list ) ) {
            return false;
        }
        if( nl->AtomType( _list ) != IntType ) {
            return false;
        }
        _key = nl->IntValue( _list );
        return true;
    }

    /*
    6.14 ~toListExpr~

    Returns the object as a list.

    */
    ListExpr DistTypeRange::toListExpr( ListExpr typeInfo ) {
        return nl->FourElemList(
            nl->StringAtom( getName( type ) ), 
            nl->IntAtom( attr ),
            nl->IntAtom( key ),
            boundary->toListExpr( ) );
    }

} // end of namespace drel