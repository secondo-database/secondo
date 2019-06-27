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
#ifndef _DistTypeRange_h_
#define _DistTypeRange_h_

#include "DistTypeHash.h"
#include "Algebras/Collection/CollectionAlgebra.h"

namespace drel {
/*
1 Class ~DistType~

This class represents the distirbution type informations for the class 
drel. The supported types are represented by the enum distributionType.
This class is used for partitioning by range. The class stores also the 
ranges.

*/
    class DistTypeRange : public DistTypeHash {
    public:
/*
1.1 Methods

*/
        DistTypeRange( distributionType _type, int _attr, 
            collection::Collection* _boundary );
        DistTypeRange( distributionType _type, int _attr, int _key, 
            collection::Collection* _boundary );

        DistTypeRange( const DistTypeRange& _distType );
        DistTypeRange &operator=( const DistTypeRange &_distType );
        virtual ~DistTypeRange( );

        virtual bool isEqual( DistTypeBasic* _distType );
        int getKey( );
        collection::Collection* getBoundary( );

        static bool allowedAttrType( ListExpr _list );

        virtual DistTypeBasic* copy( );

        static bool checkType( ListExpr list );

        bool save( SmiRecord& valueRecord, size_t& offset, 
            const ListExpr typeInfo );
        virtual ListExpr toListExpr( ListExpr typeInfo );
        virtual void print( );


    private:

/*
1.2 Members

1.2.1 ~key~
        
The key for this distribution type. It is a random key to compare 
two range distribution types. 

*/
        int key;
/*
1.2.2 ~boundary~

The pointer to the boundary object.

*/
        collection::Collection* boundary;
    };

} // end of namespace drel

#endif // _DistTypeRange_h_
