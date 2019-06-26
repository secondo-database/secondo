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
//#define DRELDEBUG

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
        collection::Collection* _boundary ) :
        DistTypeHash( _type, _attr ), key( rand( ) ), boundary( _boundary ) {

        #ifdef DRELDEBUG
        cout << "DistTypeRange::DistTypeRange" << endl;
        cout << "type" << endl;
        cout << _type << endl;
        cout << "attr" << endl;
        cout << _attr << endl;
        cout << "key" << endl;
        cout << key << endl;
        #endif
    }
    
    DistTypeRange::DistTypeRange( distributionType _type, int _attr, int _key, 
        collection::Collection* _boundary ) :
        DistTypeHash( _type, _attr ), key( _key ), boundary( _boundary ) {

        #ifdef DRELDEBUG
        cout << "DistTypeRange::DistTypeRange" << endl;
        cout << "type" << endl;
        cout << _type << endl;
        cout << "attr" << endl;
        cout << _attr << endl;
        cout << "key" << endl;
        cout << _key << endl;
        #endif
    }

/*
6.2 Copyconstructor

*/
    DistTypeRange::DistTypeRange( const DistTypeRange& _distType ) :
        DistTypeHash( _distType ), key( _distType.key ), 
        boundary( _distType.boundary ) {

        #ifdef DRELDEBUG
        cout << "DistTypeRange copy constructor" << endl;
        #endif
    }

/*
6.3 Assignment operator

*/
    DistTypeRange& DistTypeRange::operator=( const DistTypeRange& _distType ) {

        #ifdef DRELDEBUG
        cout << "DistTypeRange assignment operator" << endl;
        #endif

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
        #ifdef DRELDEBUG
        cout << "DistTypeRange destructor" << endl;
        #endif
        if(boundary){
          delete boundary;
        }
    }

/*
6.5 ~isEqual~

Compares the current DistType with another one.

*/
    bool DistTypeRange::isEqual( DistTypeBasic* _distType ) {

        #ifdef DRELDEBUG
        cout << "DistTypeRange::isEqual" << endl;
        #endif

        if( typeid( *_distType ) != typeid( *this ) ) {
            return false;
        }
        DistTypeRange* other = static_cast<DistTypeRange*>( _distType );
        return getDistType( ) == other->getDistType( )
            && getKey( ) == other->getKey( )
            && boundary->Compare( other->getBoundary( ) );
    }

/*
6.6 ~getKey~

Returns the key.

*/
    int DistTypeRange::getKey( ) {

        #ifdef DRELDEBUG
        cout << "DistTypeRange::getKey" << endl;
        #endif

        return key;
    }

/*
6.7 ~getBoundary~

Returns the boundary.

*/
    collection::Collection* DistTypeRange::getBoundary( ) {

        #ifdef DRELDEBUG
        cout << "DistTypeRange::getBoundary" << endl;
        #endif

        return boundary;
    }

/*
6.8 ~allowedAttrType~

Returns ture if the given ListExpr is the nested list representation 
of a suported type to distribute by this type.

*/
    bool DistTypeRange::allowedAttrType( ListExpr _list ) {

        #ifdef DRELDEBUG
        cout << "DistTypeRange::allowedAttrType" << endl;
        #endif

        /*return CcInt::checkType( _list )
            || CcString::checkType( _list )
            || CcReal::checkType( _list );*/

        return true;
    }

/*
6.9 ~copy~

Make a copy of the current object.

*/
    DistTypeBasic* DistTypeRange::copy( ) {

        #ifdef DRELDEBUG
        cout << "DistTypeRange::copy" << endl;
        #endif

        return new DistTypeRange( *this );
    }

/*
6.10 ~checkType~

Checks whether the type in nested list format fits to this disttype.

*/
    bool DistTypeRange::checkType( ListExpr list ) {

        #ifdef DRELDEBUG
        cout << "DistTypeRange::checkType" << endl;
        #endif

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

        return Vector::checkType( nl->Fourth( list ) );
    }

/*
6.11 ~save~

Writes a DistType to the storage.

*/
    bool DistTypeRange::save( SmiRecord& valueRecord, 
        size_t& offset, const ListExpr typeInfo ) {

        #ifdef DRELDEBUG
        cout << "DistTypeRange::save" << endl;
        cout << "typeInfo" << endl;
        cout << nl->ToString( typeInfo ) << endl;
        #endif

        if( !DistTypeHash::save( valueRecord, offset, 
            nl->TwoElemList( nl->First( typeInfo ), 
                nl->Second( typeInfo ) ) ) ) {
            return false;
        }

        if( !boundary ) {
            return false;
        }

        Word value( boundary );
        return collection::Collection::Save( 
            valueRecord, offset, nl->Fourth( typeInfo ), value );
    }

/*
6.12 ~toListExpr~

Returns the object as a list.

*/
    ListExpr DistTypeRange::toListExpr( ListExpr typeInfo ) {

        #ifdef DRELDEBUG
        cout << "DistTypeRange::toListExpr" << endl;
        cout << "typeInfo" << endl;
        cout << nl->ToString( typeInfo ) << endl;
        #endif

        Word value( boundary );
        return nl->FourElemList(
            nl->StringAtom( getName( getDistType( ) ) ),
            nl->IntAtom( attr ),
            nl->IntAtom( key ),
            collection::Collection::Out( nl->Fourth( typeInfo ), value ) );
    }

/*
6.13 ~print~

Prints the dist type informations. Used for debugging.

*/
    void DistTypeRange::print( ) {
        DistTypeHash::print( );
        cout << "key" << endl;
        cout << key << endl;
    }

} // end of namespace drel
