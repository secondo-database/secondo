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

        #ifdef DRELDEBUG
        cout << "DistTypeSpatial::DistTypeSpatial" << endl;
        cout << "type" << endl;
        cout << _type << endl;
        cout << "attr" << endl;
        cout << _attr << endl;
        cout << "key" << endl;
        cout << key << endl;
        #endif
    }
    
    template<class T>
    DistTypeSpatial<T>::DistTypeSpatial( 
        distributionType _type, int _attr, int _key, T* _grid ) :
        DistTypeHash( _type, _attr ), key( _key ), grid( _grid ) {

        #ifdef DRELDEBUG
        cout << "DistTypeSpatial::DistTypeSpatial" << endl;
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
    template<class T>
    DistTypeSpatial<T>::DistTypeSpatial( const DistTypeSpatial& _distType ) :
        DistTypeHash( _distType ), 
        key( _distType.key ), grid( _distType.grid ) {

        #ifdef DRELDEBUG
        cout << "DistTypeSpatial copy constructor" << endl;
        #endif
    }

/*
6.3 Assignment operator

*/
    template<class T>
    DistTypeSpatial<T>& DistTypeSpatial<T>::operator=( 
        const DistTypeSpatial& _distType ) {

        #ifdef DRELDEBUG
        cout << "DistTypeSpatial assignment operator" << endl;
        #endif


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

        #ifdef DRELDEBUG
        cout << "DistTypeSpatial destructor" << endl;
        #endif
    }

/*
6.5 ~isEqual~

Compares the current DistType with another one.

*/
    template<class T>
    bool DistTypeSpatial<T>::isEqual( DistTypeBasic* _distType ) {

        #ifdef DRELDEBUG
        cout << "DistTypeSpatial::isEqual" << endl;
        #endif

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

        #ifdef DRELDEBUG
        cout << "DistTypeSpatial::getKey" << endl;
        #endif

        return key;
    }

/*
6.7 ~getGrid~

Returns the grid.

*/
    template<class T>
    T* DistTypeSpatial<T>::getGrid( ) {

        #ifdef DRELDEBUG
        cout << "DistTypeSpatial::getGrid" << endl;
        #endif

        return grid;
    }

/*
6.8 ~allowedAttrType~

Returns ture if the given ListExpr is the nested list representation
of a suported type to distribute by spatial2d.

*/
    template<class T>
    bool DistTypeSpatial<T>::allowedAttrType2d( ListExpr _list ) {

        #ifdef DRELDEBUG
        cout << "DistTypeSpatial::allowedAttrType2d" << endl;
        #endif

        return Point::checkType( _list )
            || Line::checkType( _list )
            || Region::checkType( _list );
    }

/*
6.9 ~allowedAttrType~

Returns ture if the given ListExpr is the nested list representation
of a suported type to distribute by spatial3d.

*/
    template<class T>
    bool DistTypeSpatial<T>::allowedAttrType3d( ListExpr _list ) {

        #ifdef DRELDEBUG
        cout << "DistTypeSpatial::allowedAttrType3d" << endl;
        #endif

        return temporalalgebra::UPoint::checkType( _list );
    }

/*
6.10 ~copy~

Make a copy of the current object.

*/
    template<class T>
    DistTypeBasic* DistTypeSpatial<T>::copy( ) {

        #ifdef DRELDEBUG
        cout << "DistTypeSpatial::copy" << endl;
        #endif

        return new DistTypeSpatial<T>( *this );
    }

/*
6.11 ~checkType~

Checks whether the type in nested list format fits to this disttype.

*/
    template<class T>
    bool DistTypeSpatial<T>::checkType( ListExpr list ) {

        #ifdef DRELDEBUG
        cout << "DistTypeSpatial::checkType" << endl;
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

        return T::checkType( nl->Fourth( list ) );
    }

/*
6.12 ~save~

Writes a DistType to the storage.

*/
    template<class T>
    bool DistTypeSpatial<T>::save( 
        SmiRecord& valueRecord, size_t& offset, const ListExpr typeInfo ) {

        #ifdef DRELDEBUG
        cout << "DistTypeSpatial::save" << endl;
        #endif

        if( !DistTypeHash::save( 
            valueRecord, offset, nl->TwoElemList( nl->First( typeInfo ), 
                nl->Second( typeInfo ) ) ) ) {
            return false;
        }

        if( !grid ) {
            return false;
        }

        #ifdef DRELDEBUG
        cout << "grid value" << endl;
        grid->Print( cout );
        cout << endl;
        #endif

        Word value( grid );
        bool saveGrid = SaveAttribute<T>( 
            valueRecord, offset, nl->Fourth( typeInfo ), value );
        #ifdef DRELDEBUG
        cout << "dist type info" << endl;
        cout << nl->ToString( nl->Fourth( typeInfo ) ) << endl;
        cout << "save grid ok?" << endl;
        cout << saveGrid << endl;
        #endif

        return saveGrid;
    }

/*
6.13 ~toListExpr~

Returns the object as a list.

*/
    template<class T>
    ListExpr DistTypeSpatial<T>::toListExpr( ListExpr typeInfo ) {

        #ifdef DRELDEBUG
        cout << "DistTypeSpatial::toListExpr" << endl;
        #endif

        return nl->FourElemList(
            nl->StringAtom( getName( getDistType( ) ) ), 
            nl->IntAtom( getAttr( ) ),
            nl->IntAtom( key ),
            grid->ToListExpr( 
                nl->Fourth( typeInfo ) ) );
    }

/*
6.14 ~print~

Prints the dist type informations. Used for debugging.

*/
    template<class T>
    void DistTypeSpatial<T>::print( ) {
        DistTypeHash::print( );
        cout << "key" << endl;
        cout << key << endl;
    }

/*
6.15 ~print~

Reads the grid object from a nested list.

*/
    template<class T>
    T * DistTypeSpatial<T>::ReadFrom( 
        const ListExpr value, const ListExpr typeInfo ) {

        T* grid = new T( 0 );
        grid->ReadFrom( value, typeInfo );
        return grid;
    }

    template class DistTypeSpatial<temporalalgebra::CellGrid2D>;
    template class DistTypeSpatial<temporalalgebra::CellGrid<3>>;

} // end of namespace drel