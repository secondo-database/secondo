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

#include "Algebras/Temporal/TemporalAlgebra.h"
#include "DistTypeSpatial.h"
#include "StandardTypes.h"

extern NestedList* nl;

using namespace distributed2;
using namespace distributed2;

namespace drel {
    /*
    6 Class ~DistType~

    Implementation.

    6.1 Constructors

    */
    template<class T>
    DistTypeSpatial<T>::DistTypeSpatial( 
        distributionType _type, int _attr, T* _grid ) :
        DistTypeHash( _type, _attr ), key( rand( ) ), grid( _grid ) {
    }
    
    template<class T>
    DistTypeSpatial<T>::DistTypeSpatial( 
        distributionType _type, int _attr, int _key, T* _grid ) :
        DistTypeHash( _type, _attr ), key( _key ), grid( _grid ) {
    }

    /*
    6.2 Copyconstructor

    */
    template<class T>
    DistTypeSpatial<T>::DistTypeSpatial( const DistTypeSpatial& _distType ) :
        DistTypeHash( _distType ), 
        key( _distType.key ), grid( _distType.grid ) {
    }

    /*
    6.3 Assignment operator

    */
    template<class T>
    DistTypeSpatial<T>& DistTypeSpatial<T>::operator=( 
        const DistTypeSpatial& _distType ) {

        if( this == &_distType ) {
            return *this;
        }
        DistTypeHash::operator=( _distType );
        key = _distType.key;
        grid = _distType.grid;
        return *this;
    }

    /*
    6.4 Destructor

    */
    template<class T>
    DistTypeSpatial<T>::~DistTypeSpatial( ) {
    }

    /*
    6.5 ~isEqual~

    Compares the current DistType with another one.

    */
    template<class T>
    bool DistTypeSpatial<T>::isEqual( DistTypeBasic* _distType ) {
        if( typeid( *_distType ) != typeid( *this ) ) {
            return false;
        }
        DistTypeSpatial<T>* other = 
            static_cast<DistTypeSpatial<T>*>( _distType );

        return getDistType( ) == other->getDistType( )
            && getKey( ) == other->getKey( )
            && ( getGrid( )->Compare( other->getGrid( ) ) == 0);
    }

    /*
    6.6 ~getKey~

    Returns the key.

    */
    template<class T>
    int DistTypeSpatial<T>::getKey( ) {
        return key;
    }

    /*
    6.7 ~getGrid~

    Returns the grid.

    */
    template<class T>
    T* DistTypeSpatial<T>::getGrid( ) {
        return grid;
    }

    /*
    6.8 ~copy~

    Make a copy of the current object.

    */
    template<class T>
    DistTypeBasic* DistTypeSpatial<T>::copy( ) {
        return new DistTypeSpatial<T>( *this );
    }

    /*
    6.9 ~getTypeList~

    Returns the Typelist of the disttype. For the spatial type it is a
    CcString, two CcInt and a grid object.

    */
    template<class T>
    ListExpr DistTypeSpatial<T>::getTypeList( ListExpr attrType ) {
        return nl->FourElemList(
            listutils::basicSymbol<CcString>( ),
            listutils::basicSymbol<CcInt>( ),
            listutils::basicSymbol<CcInt>( ),
            listutils::basicSymbol<T>( ) );
    }

    /*
    6.10 ~checkType~

    Checks whether the type in nested list format fits to this disttype.

    */
    template<class T>
    bool DistTypeSpatial<T>::checkType( ListExpr list ) {
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

        return T::checkType( nl->Fourth( list ) );
    }

    /*
    6.11 ~save~

    Writes a DistType to the storage.

    */
    template<class T>
    bool DistTypeSpatial<T>::save( 
        SmiRecord& valueRecord, size_t& offset, const ListExpr typeInfo ) {

        if( !DistTypeHash::save( 
            valueRecord, offset, nl->TwoElemList( nl->First( typeInfo ), 
                nl->Second( typeInfo ) ) ) ) {
            return false;
        }
        if( !distributed2::writeVar( key, valueRecord, offset ) ) {
            return false;
        }
        if( !grid ) {
            return false;
        }

        Word value( grid );
        SaveAttribute<T>( valueRecord, offset, nl->Fourth( typeInfo ), value );

        return true;
    }

    /*
    6.12 ~readFrom~

    Reads a list an creates a DistType.

    */
    template<class T>
    DistTypeSpatial<T>* DistTypeSpatial<T>::readFrom( const ListExpr _list ) {
        if( !nl->HasLength( _list, 4 ) ) {
            return 0;
        }

        distributionType tType;
        if( !readType( nl->First( _list ), tType ) ) {
            return 0;
        }
        if( tType != spatial2d && tType != spatial3d ) {
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

        T* grid = new T( 0 );

        if( !grid->ReadFrom( nl->Fourth( _list ), nl->TheEmptyList( ) ) ) {
            return 0;
        }
        
        return new DistTypeSpatial<T>( tType, tAttr, tKey, grid );
    }

    /*
    6.13 ~readKey~

    Reads the key from a list.

    */
    template<class T>
    bool DistTypeSpatial<T>::readKey( const ListExpr _list, int& _key ) {
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
    template<class T>
    ListExpr DistTypeSpatial<T>::toListExpr( ListExpr typeInfo ) {
        return nl->FourElemList(
            nl->StringAtom( getName( getDistType( ) ) ), 
            nl->IntAtom( getAttr( ) ),
            nl->IntAtom( key ),
            grid->ToListExpr( 
                nl->Fourth( typeInfo ) ) );
    }

    template class DistTypeSpatial<temporalalgebra::CellGrid2D>;
    template class DistTypeSpatial<temporalalgebra::CellGrid<3>>;

} // end of namespace drel