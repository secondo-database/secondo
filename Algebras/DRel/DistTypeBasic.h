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
#ifndef _DistTypeBasic_h_
#define _DistTypeBasic_h_

#include <string>
#include <map>

#include "NestedList.h"
#include "SecondoSMI.h"

#include "Algebras/Distributed2/Dist2Helper.h"

namespace drel {

    enum distributionType { 
        random, hash, range, spatial2d, spatial3d, replicated };

    distributionType getType( const std::string _type );
    std::string getName( const distributionType _type );

    bool supportedType( const std::string _type );
    bool supportedType( 
        const std::string _typeString , distributionType& type);

    /*
    1 Class ~DistType~

    This class represents the distirbution type informations for the class 
    drel. The supported types are represented by the enum distributionType.

    */

    class DistTypeBasic {
    public:
        /*
        1.1 Methods

        */
        DistTypeBasic( );
        DistTypeBasic( distributionType _type );

        DistTypeBasic( const DistTypeBasic& _distType );
        DistTypeBasic &operator=( const DistTypeBasic &_distType );
        virtual ~DistTypeBasic( );

        virtual bool isEqual( DistTypeBasic* _distType );
        distributionType getDistType( );

        virtual DistTypeBasic* copy( );

        static ListExpr getTypeList( );
        static bool checkType( ListExpr list );

        virtual bool save( SmiRecord& valueRecord, size_t& offset, 
            const ListExpr typeInfo );
        static DistTypeBasic* readFrom( const ListExpr _list );
        virtual ListExpr toListExpr( ListExpr typeInfo );

        static bool readType( const ListExpr _list, distributionType& _type );

    protected:
        /*
        1.2 Members

        1.2.1 ~key~

        The key for this distribution type. It is a random key to compare
        two range distribution types.

        */
        distributionType type;
    };

} // end of namespace drel

#endif // _DistTypeBasic_h_