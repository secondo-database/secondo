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

#include <iterator>
#include <assert.h>

#include "DistTypeBasic.h"
#include "StandardTypes.h"

extern NestedList* nl;

using namespace distributed2;

namespace drel {

    /*
    1 ~distributionTypeMap~

    Map of a string and the matching distributionType.

    */
    std::map<std::string, distributionType> distributionTypeMap = {
        { "RANDOM", random }, { "HASH", hash }, { "RANGE", range },
    { "SPATIAL2D", spatial2d }, { "SPATIAL3D", spatial3d },
    { "REPLICATED", replicated }
    };

    /*
    2 ~getType~

    Map of a string and the matching distributionType.

    */
    distributionType getType( const std::string _type ) {
        std::map<std::string, distributionType>::iterator it;
        for( it = distributionTypeMap.begin( );
            it != distributionTypeMap.end( ); it++ )
            if( it->first == _type )
                return it->second;

        assert( false );
    }

    /*
    3 ~getTypeByNum~

    Get the distributionType by a number. Returns false, if the number is not
    a allowed distributionType.

    */
    bool getTypeByNum( const int _num, distributionType& type ) {

        if( _num < 0 || _num >= 6 ) {
            return false;
        }

        switch( _num ) {
        case 0: type = random; break;
        case 1: type = hash; break;
        case 2: type = range; break;
        case 3: type = spatial2d; break;
        case 4: type = spatial3d; break;
        case 5: type = replicated; break;
        }

        return true;
    }

    /*
    4 ~getName~

    Returns the string value of a distributionType.

    */
    std::string getName( const distributionType _type ) {
        std::map<std::string, distributionType>::iterator it;
        for( it = distributionTypeMap.begin( );
            it != distributionTypeMap.end( ); it++ )
            if( it->second == _type )
                return it->first;

        assert( false );
        return "unknown";
    }

    /*
    5 ~supportedType~

    Returns true if the given string is the string of a supported
    distributionType.

    */
    bool supportedType( const std::string _type ) {
        std::map<std::string, distributionType>::iterator it;
        it = distributionTypeMap.find( _type );
        return it != distributionTypeMap.end( );
    }

    /*
    6 ~supportedType~

    Returns true if the given string is the string of a supported
    distributionType.

    */
    bool supportedType(
        const std::string _typeString, distributionType& type ) {

        std::map<std::string, distributionType>::iterator it;
        it = distributionTypeMap.find( _typeString );
        type = it->second;
        return it != distributionTypeMap.end( );
    }

    /*
    7 Class ~DistType~

    Implementation.
    
    7.1 Constructors

    */
    DistTypeBasic::DistTypeBasic( ) {

        #ifdef DRELDEBUG
        cout << "DistTypeBasic::DistTypeBasic" << endl;
        #endif
    }

    DistTypeBasic::DistTypeBasic( distributionType _type ) :
        type( _type ) {

        #ifdef DRELDEBUG
        cout << "DistTypeBasic::DistTypeBasic" << endl;
        cout << "distType" << endl;
        cout << _type << endl;
        #endif
    }

    /*
    7.2 Copyconstructor

    */
    DistTypeBasic::DistTypeBasic( const DistTypeBasic& _distType ) :
        type( _distType.type ) {

        #ifdef DRELDEBUG
        cout << "DistTypeBasic copy constructor" << endl;
        #endif
    }

    /*
    7.3 Assignment operator

    */
    DistTypeBasic& DistTypeBasic::operator=( const DistTypeBasic& _distType ) {

        #ifdef DRELDEBUG
        cout << "DistTypeBasic assignment operator" << endl;
        #endif

        if( this == &_distType ) {
            return *this;
        }
        type = _distType.type;
        return *this;
    }

    /*
    7.4 Destructor

    */
    DistTypeBasic::~DistTypeBasic( ) {

        #ifdef DRELDEBUG
        cout << "DistTypeBasic destructor" << endl;
        #endif

    }

    /*
    7.5 ~isEqual~

    Compares the current DistType with another one.

    */
    bool DistTypeBasic::isEqual( DistTypeBasic* _distType ) {

        #ifdef DRELDEBUG
        cout << "DistTypeBasic::isEqual" << endl;
        #endif

        if( typeid( *_distType ) != typeid( *this ) ) {
            return false;
        }
        return type == replicated;
    }

    /*
    7.6 ~getDistType~

    Returns the distribution type.

    */
    distributionType DistTypeBasic::getDistType( ) {

        #ifdef DRELDEBUG
        cout << "DistTypeBasic::getDistType" << endl;
        #endif

        return type;
    }

    /*
    8.7 ~copy~

    Make a copy of the current object.

    */
    DistTypeBasic* DistTypeBasic::copy( ) {

        #ifdef DRELDEBUG
        cout << "DistTypeBasic::copy" << endl;
        #endif

        return new DistTypeBasic( *this );
    }

    /*
    8.8 ~checkType~

    Checks whether the type in nested list format fits to this disttype.

    */
    bool DistTypeBasic::checkType( ListExpr list ) {

        #ifdef DRELDEBUG
        cout << "DistTypeBasic::checkType" << endl;
        #endif

        if( !nl->HasLength( list, 1) ) {
            return false;
        }
        return CcInt::checkType( nl->First( list ) );
    }

    /*
    8.9 ~save~

    Writes a DistType to the storage.

    */
    bool DistTypeBasic::save( SmiRecord& valueRecord, size_t& offset, 
        const ListExpr typeInfo ) {

        #ifdef DRELDEBUG
        cout << "DistTypeBasic::save" << endl;
        #endif

        return true;
    }

    /*
    8.10 ~toListExpr~

    Returns the disttype as nestedlist.

    */
    ListExpr DistTypeBasic::toListExpr( ListExpr _typeInfo ) {

        #ifdef DRELDEBUG
        cout << "DistTypeBasic::save" << endl;
        cout << "typeInfo" << endl;
        cout << nl->ToString( typeInfo ) << endl;
        cout << "result list" << endl;
        cout << nl->ToString( nl->OneElemList( nl->IntAtom( type ) ) ) << endl;
        #endif

        return nl->OneElemList(
            nl->StringAtom( getName( getDistType( ) ) ) );
    }

    /*
    8.11 ~print~

    Prints the dist type informations. Used for debugging.

    */
    void DistTypeBasic::print( ) {
        cout << "type" << endl;
        cout << getName( type ) << endl;
    }

} // end of namespace drel