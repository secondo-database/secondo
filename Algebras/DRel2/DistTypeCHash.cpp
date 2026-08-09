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

 @author
  D. Selenyi

 @description
 see OperatorSpec

 @note
 Checked - 2020 

 @history
 Version 1.0 - Created - D. Selenyi - 2020

 @todo
 Nothing

*/

//#define DRELDEBUG

#include <iterator>
#include <assert.h>

#include "DistTypeCHash.h"
#include "StandardTypes.h"

extern NestedList* nl;

using namespace distributed2;

namespace drel {
/*
6 Class ~DistTypeCHash~

Implementation.

6.1 Constructors

*/

    DistTypeCHash::DistTypeCHash( distributionType _type, int _attr) :
        DistTypeBasic( _type ), attr( _attr )  {
        #ifdef DRELDEBUG
        std::cout << "DistTypeCHash constructor" << std::endl;
        #endif
    }

/*
6.2 Copyconstructor

*/
    DistTypeCHash::DistTypeCHash( const DistTypeCHash& _distType ) :
        DistTypeBasic( _distType ), attr( _distType.attr ) {
        #ifdef DRELDEBUG
        std::cout << "DistTypeCHash constructor" << std::endl;
        #endif
    }

/*
6.3 Assignment operator

*/
    DistTypeCHash& DistTypeCHash::operator=( const DistTypeCHash& _distType ) {

        #ifdef DRELDEBUG
        std::cout << "DistTypeCHash assignment operator" << std::endl;
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
    DistTypeCHash::~DistTypeCHash( ) {
    }

/*
6.5 ~isEqual~

Compares the current DistType with another one.

*/
    bool DistTypeCHash::isEqual( DistTypeBasic* _distType ) {

        #ifdef DRELDEBUG
        std::cout << "DistTypeCHash::isEqual" << std::endl;
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
    int DistTypeCHash::getAttr( ) {

        #ifdef DRELDEBUG
        std::cout << "DistTypeCHash::getAttr" << std::endl;
        #endif

        return attr;
    }

/*
6.7 ~copy~

Make a copy of the current object.

*/
    DistTypeBasic* DistTypeCHash::copy( ) {

        #ifdef DRELDEBUG
        std::cout << "DistTypeCHash::copy" << std::endl;
        #endif

        return new DistTypeCHash( *this );
    }

/*
6.8 ~checkType~

Checks whether the type in nested list format fits to this disttype.

*/
    bool DistTypeCHash::checkType( ListExpr list ) {

        #ifdef DRELDEBUG
        std::cout << "DistTypeCHash::checkType" << std::endl;
        #endif

        if( !nl->HasLength( list, 3 ) ) {
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
    bool DistTypeCHash::save( SmiRecord& valueRecord, size_t& offset, 
        const ListExpr typeInfo ) {

        #ifdef DRELDEBUG
        std::cout << "DistTypeCHash::save" << std::endl;
        std::cout << "typeInfo" << std::endl;
        std::cout << nl->ToString( typeInfo ) << std::endl;
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
    ListExpr DistTypeCHash::toListExpr( ListExpr typeInfo ) {

        #ifdef DRELDEBUG
        std::cout << std::endl << "DistTypeCHash::toListExpr" << std::endl;
        #endif

        return nl->TwoElemList(
            nl->StringAtom( getName( getDistType( ) ) ), nl->IntAtom( attr) );
    }

/*
6.11 ~print~

Prints the dist type informations. Used for debugging.

*/
    void DistTypeCHash::print( ) {
        DistTypeBasic::print( );
        std::cout << "attr: " << attr << std::endl;
    }

/*
6.12 ~computeNewAttrPos~

Computes the new position of the attribute used to distribute. Used for 
operations like a projection.

*/
    bool DistTypeCHash::computeNewAttrPos( ListExpr attrPosList, int& attrPos ){

        #ifdef DRELDEBUG
        std::cout << "DistTypeCHash::computeNewAttrPos" << std::endl;
        std::cout << "attrPosList" << std::endl;
        std::cout << nl->ToString( attrPosList ) << std::endl;
        std::cout << "attrPos" << std::endl;
        std::cout << attrPos << std::endl;
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
