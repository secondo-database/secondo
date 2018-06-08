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
#ifndef _DistTypeSpatial_h_
#define _DistTypeSpatial_h_

#include "DistTypeHash.h"
#include "Boundary.h"

namespace drel {
    /*
    1 Class ~DistType~

    This class represents the distirbution type informations for the class 
    drel. The supported types are represented by the enum distributionType.

    */
    template<class T>
    class DistTypeSpatial : public DistTypeHash {
    public:
        /*
        1.1 Methods

        */
        DistTypeSpatial( distributionType _type, int _attr, T* _grid );
        DistTypeSpatial( distributionType _type, int _attr, int _key, 
            T* _grid );

        DistTypeSpatial( const DistTypeSpatial& _distType );
        DistTypeSpatial &operator=( const DistTypeSpatial &_distType );
        virtual ~DistTypeSpatial( );

        virtual bool isEqual( DistTypeBasic* _distType );
        int getKey( );
        T* getGrid( );

        static ListExpr getTypeList( ListExpr attrType );
        static bool checkType( ListExpr list );

        bool save( SmiRecord& valueRecord, size_t& offset, 
            const ListExpr typeInfo );
        static DistTypeSpatial* readFrom( const ListExpr _list );
        virtual ListExpr toListExpr( ListExpr typeInfo );

    protected:
        static bool readKey( const ListExpr _list, int& _key );

    private:

        /*
        1.2 Members

        1.2.1 ~attr~
        
        The number of used attribute to distribute the relation. The numbering 
        starts with 0.

        */
        int key;
        /*
        1.2.2 ~grid~

        The pointer to the grid object, used to distribute.

        */
        T* grid;
    };

} // end of namespace drel

#endif // _DistTypeSpatial_h_