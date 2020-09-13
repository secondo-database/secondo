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
        cout << "DistTypeCHash constructor" << endl;
        #endif
    }

/*
6.2 Copyconstructor

*/
    DistTypeCHash::DistTypeCHash( const DistTypeCHash& _distType ) :
        DistTypeBasic( _distType ), attr( _distType.attr ) {
        #ifdef DRELDEBUG
        cout << "DistTypeCHash constructor" << endl;
        #endif
    }

/*
6.3 Assignment operator

*/
    DistTypeCHash& DistTypeCHash::operator=( const DistTypeCHash& _distType ) {

        #ifdef DRELDEBUG
        cout << "DistTypeCHash assignment operator" << endl;
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
        cout << "DistTypeCHash::isEqual" << endl;
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
        cout << "DistTypeCHash::getAttr" << endl;
        #endif

        return attr;
    }

/*
6.7 ~copy~

Make a copy of the current object.

*/
    DistTypeBasic* DistTypeCHash::copy( ) {

        #ifdef DRELDEBUG
        cout << "DistTypeCHash::copy" << endl;
        #endif

        return new DistTypeCHash( *this );
    }

/*
6.8 ~checkType~

Checks whether the type in nested list format fits to this disttype.

*/
    bool DistTypeCHash::checkType( ListExpr list ) {

        #ifdef DRELDEBUG
        cout << "DistTypeCHash::checkType" << endl;
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
        cout << "DistTypeCHash::save" << endl;
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
    ListExpr DistTypeCHash::toListExpr( ListExpr typeInfo ) {

        #ifdef DRELDEBUG
        cout << endl << "DistTypeCHash::toListExpr" << endl;
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
        cout << "attr: " << attr << endl;
    }

/*
6.12 ~computeNewAttrPos~

Computes the new position of the attribute used to distribute. Used for 
operations like a projection.

*/
    bool DistTypeCHash::computeNewAttrPos( ListExpr attrPosList, int& attrPos ){

        #ifdef DRELDEBUG
        cout << "DistTypeCHash::computeNewAttrPos" << endl;
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
