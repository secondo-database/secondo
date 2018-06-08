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
        { "random", random }, { "hash", hash }, { "range", range },
        { "spatial2d", spatial2d }, { "spatial3d", spatial3d },
        { "replicated", replicated }
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
    3 ~getName~

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
    4 ~supportedType~

    Returns true if the given string is the string of a supported 
    distributionType.

    */
    bool supportedType( const std::string _type ) {
        std::map<std::string, distributionType>::iterator it;
        it = distributionTypeMap.find( _type );
        return it != distributionTypeMap.end( );
    }

    /*
    5 ~supportedType~

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
    6 Class ~DistType~

    Implementation.
    
    6.1 Constructors

    */
    DistTypeBasic::DistTypeBasic( ) {
    }

    DistTypeBasic::DistTypeBasic( distributionType _type ) :
        type( _type ) {
    }

    /*
    6.2 Copyconstructor

    */
    DistTypeBasic::DistTypeBasic( const DistTypeBasic& _distType ) :
        type( _distType.type ) {
    }

    /*
    6.3 Assignment operator

    */
    DistTypeBasic& DistTypeBasic::operator=( const DistTypeBasic& _distType ) {
        if( this == &_distType ) {
            return *this;
        }
        type = _distType.type;
        return *this;
    }

    /*
    6.4 Destructor

    */
    DistTypeBasic::~DistTypeBasic( ) {
    }

    /*
    6.5 ~isEqual~

    Compares the current DistType with another one.

    */
    bool DistTypeBasic::isEqual( DistTypeBasic* _distType ) {
        if( typeid( *_distType ) != typeid( *this ) ) {
            return false;
        }
        return type == replicated;
    }

    /*
    6.6 ~getDistType~

    Returns the distribution type.

    */
    distributionType DistTypeBasic::getDistType( ) {
        return type;
    }

    /*
    6.7 ~getTypeList~

    Returns the Typelist of the disttype. For the basic type it is only a 
    CcString.

    */
    ListExpr DistTypeBasic::getTypeList( ) {
        return nl->OneElemList( listutils::basicSymbol<CcString>( ) );
    }

    /*
    6.8 ~checkType~

    Checks whether the type in nested list format fits to this disttype.

    */
    bool DistTypeBasic::checkType( ListExpr list ) {
        if( !nl->HasLength( list, 1) ) {
            return false;
        }
        return CcString::checkType( nl->First( list ) );
    }

    /*
    6.9 ~save~

    Writes a DistType to the storage.

    */
    bool DistTypeBasic::save( SmiRecord& valueRecord, size_t& offset, 
        const ListExpr typeInfo ) {
        return distributed2::writeVar( getName( type ), valueRecord, offset );
    }

    /*
    6.10 ~readFrom~

    Reads the disttype from a list.

    */
    DistTypeBasic* DistTypeBasic::readFrom( const ListExpr _list ) {
        if( ! nl->HasLength( _list, 1 ) ) {
            return 0;
        }
        distributionType tType;
        if( ! readType( nl->First( _list ), tType ) ) {
            return 0;
        }
        return new DistTypeBasic( tType );
    }

    /*
    6.11 ~readFrom~

    Returns the disttype as nestedlist.

    */
    ListExpr DistTypeBasic::toListExpr( ListExpr typeInfo ) {
        return nl->OneElemList( nl->StringAtom( getName( type ) ) );
    }

    /*
    6.12 ~readType~

    Reads a distributionType from a nested list. Returns true for a correct 
    nested list. False otherwise.

    */
    bool DistTypeBasic::readType( const ListExpr _list, 
        distributionType& _type ) {

        if( ! nl->IsAtom( _list ) ) {
            return false;
        }
        if( nl->AtomType( _list ) != StringType ) {
            return false;
        }
        return supportedType( nl->StringValue( _list ), _type );
    }

} // end of namespace drel