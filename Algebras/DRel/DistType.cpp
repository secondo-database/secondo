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

#include "DistType.h"
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
    DistType::DistType( ) {
    }

    DistType::DistType( distributionType _type ) :
        type( _type ), attr( -1 ), key( -1 ) {
    }

    DistType::DistType( distributionType _type, int _attr ) :
        type( _type ), attr( _attr ), key( -1 ) {
    }

    DistType::DistType( distributionType _type, int _attr, int _key ) :
        type( _type ), attr( _attr ), key( _key ) {
    }

    DistType::DistType( 
        distributionType _type, int _attr, int _key, Boundary * _boundary ) :
        type( _type ), attr( _attr ), key( _key ), boundary( _boundary ) {
    }

    /*
    6.2 Copyconstructor

    */
    DistType::DistType( const DistType &_distType ) :
        type( _distType.type ), attr( _distType.attr ), key( _distType.key ) {
    }

    /*
    6.3 Assignment operator

    */
    DistType& DistType::operator=( const DistType &_distType ) {
        if( this == &_distType ) {
            return *this;
        }
        type = _distType.type;
        attr = _distType.attr;
        key  = _distType.key;
        return *this;
    }

    /*
    6.4 Destructor

    */
    DistType::~DistType( ) {
    }

    /*
    6.5 ~isEqual~

    Compares the current DistType with another one.

    */
    bool DistType::isEqual( DistType* _distType ) {
        return type == _distType->getDistType( )
            && attr == _distType->getAttr( )
            && key  == _distType->getKey( );
    }

    /*
    6.6 ~getDistType~

    Returns the distribution type.

    */
    distributionType DistType::getDistType( ) {
        return type;
    }

    /*
    6.7 ~getAttr~

    Returns the number of the distribution attribute. If not used
    attr should be -1.

    */
    int DistType::getAttr( ) {
        return attr;
    }

    /*
    6.8 ~getKey~

    Returns a key number generated while create the distribution type.

    */
    int DistType::getKey( ) {
        return key;
    }

    Boundary* DistType::getBoundary( ) {
        return boundary;
    }

    /*
    6.9 ~getTypeExpr~

    Returns the "attribute" name used in the drel type to store the type 
    information.

    */
    std::string DistType::getTypeExpr( ) {
        return "Typ";
    }

    /*
    6.10 ~getAttrExpr~

    Returns the "attribute" name used in the drel type to store the attribute 
    number of the distibution attribute.

    */
    std::string DistType::getAttrExpr( ) {
        return "Attr";
    }

    /*
    6.11 ~getKeyExpr~

    Returns the "attribute" name used in the drel type to store the key.

    */
    std::string DistType::getKeyExpr( ) {
        return "Key";
    }

    /*
    6.12 ~open~

    Opens a stored DistType.

    */
    DistType* DistType::open( 
        SmiRecord& valueRecord, size_t& offset, const ListExpr typeInfo ) {

        std::string typeString;
        int attr;
        int key;
        if( !readVar<std::string>( typeString, valueRecord, offset ) ) {
            return 0;
        }
        if( !readVar<int>( attr, valueRecord, offset ) ) {
            return 0;
        }
        if( !readVar<int>( key, valueRecord, offset ) ) {
            return 0;
        }

        distributionType type;
        if( !supportedType( typeString, type ) ) {
            return 0;
        }

        if( type != range && type != spatial2d && type != spatial3d ) {
            return new DistType( type, attr, key);
        }
        
        ListExpr numAttrType = 
            nl->Second( nl->Second( nl->Second( typeInfo ) ) );
        for( int i = 0 ; i < attr ; i++ ) {
            numAttrType = nl->Rest( numAttrType );
        }
        numAttrType = nl->Second( nl->First( numAttrType ) );
        ListExpr boundaryType = nl->TwoElemList( 
            nl->SymbolAtom( 
                Boundary::BasicType( ) ), nl->OneElemList( numAttrType ) );
        Word value;
        if( ! Boundary::Open( valueRecord, offset, boundaryType, value ) ) {
            return 0;
        }

        return new DistType( type, attr, key, ( Boundary* )value.addr );
    }

    /*
    6.13 ~createDistType~

    Creates a new DistType. The method checks the parameter. For example 
    to create a hash-partitioned DistType the attribute number is needed.

    */
    DistType* DistType::createDistType( std::string _type ) {
        distributionType type;
        if( !supportedType( _type, type ) ) {
            return 0;
        }
        return createDistType( type );
    }

    DistType* DistType::createDistType( std::string _type, int _attr ) {
        distributionType type;
        if( !supportedType( _type, type ) ) {
            return 0;
        }
        return createDistType( type, _attr );
    }

    DistType* DistType::createDistType( 
        std::string _type, int _attr, int _key ) {

        distributionType type;
        if( !supportedType( _type, type ) ) {
            return 0;
        }
        return createDistType( type, _attr, _key );
    }

    DistType* DistType::createDistType( distributionType _type ) {
        if( _type != replicated && _type != random ) {
            return 0;
        }
        return new DistType( _type );
    }

    DistType* DistType::createDistType( distributionType _type, int _attr ) {
        if( _type != hash ) {
            return 0;
        }
        if( _type < 0 ) {
            return 0;
        }
        return new DistType( _type, _attr );
    }

    DistType* DistType::createDistType( 
        distributionType _type, int _attr, int _key ) {

        if( _type != range && _type != spatial2d && _type != spatial3d ) {
            return 0;
        }
        if( _type < 0 || _key < 0 ) {
            return 0;
        }
        return new DistType( _type, _attr, _key );
    }

    DistType* DistType::createDistType( 
        distributionType _type, int _attr, int _key, Boundary* _boundary ) {

        if( _type != range && _type != spatial2d && _type != spatial3d ) {
            return 0;
        }
        if( _type < 0 || _key < 0  || _boundary == 0) {
            return 0;
        }
        return new DistType( _type, _attr, _key, _boundary);
    }

    /*
    6.14 ~save~

    Writes a DistType to the storage.

    */
    bool DistType::save( SmiRecord & valueRecord, size_t & offset ) {
        if( ! ( distributed2::writeVar( getName( type ), valueRecord, offset )
            && distributed2::writeVar( attr, valueRecord, offset )
            && distributed2::writeVar( key, valueRecord, offset ) ) ) {
            return false;
        }
        cout << "disttype boundary save" << endl;
        if( type == range || type == spatial2d || type == spatial3d ) {
            if( boundary == 0 ) {
                cout << "boundary == 0" << endl;
                cout << "false" << endl;
                return false;
            }
            if( boundary->save( valueRecord, offset ) ) {
                cout << "true" << endl;
                return true;
            }
            else {
                cout << "save" << endl;
                cout << "false" << endl;
                return false;
            }
        }
        return true;
    }

    /*
    6.15 ~readType~

    Reads a distributionType from a nested list. Returns true for a correct 
    nested list. False otherwise.

    */
    bool DistType::readType( ListExpr _list, distributionType& _type ) {
        if( !listutils::isSymbol( _list ) ) {
            return false;
        }
        return supportedType( nl->SymbolValue( _list ), _type );
    }

    /*
    6.16 ~readInt~

    Reads an integer from a nested list. Returns true for a correct 
    nested list. False otherwise.

    */
    bool DistType::readInt( ListExpr _list, int& _attr ) {
        if( nl->AtomType( _list ) != IntType) {
            return false;
        }
        return nl->IntValue( _list );
    }

    Boundary* DistType::readBoundary( 
        ListExpr _typeInfo, ListExpr _list, int _attr ) {

        ListExpr numAttrType = 
            nl->Second( nl->Second( nl->Second( _typeInfo ) ) );
        for( int i = 0; i < _attr; i++ ) {
            numAttrType = nl->Rest( numAttrType );
        }
        numAttrType = nl->Second( nl->First( numAttrType ) );
        ListExpr boundaryType = nl->TwoElemList( 
            nl->SymbolAtom( 
                Boundary::BasicType( ) ), nl->OneElemList( numAttrType ) );
        int errorPos = 0;
        ListExpr errorInfo;
        bool correct = false;
        Word value = Boundary::In( 
            boundaryType, _list, errorPos, errorInfo, correct );
        if( ! correct ) {
            return 0;
        }
        return ( Boundary* )value.addr;
    }

} // end of namespace drel