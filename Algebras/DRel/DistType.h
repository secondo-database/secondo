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
#ifndef _DistType_h_
#define _DistType_h_

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

    class DistType {
    public:
        /*
        1.1 Methods

        */
        DistType( );
        DistType( distributionType _type );
        DistType( distributionType _type, int _attr );
        DistType( distributionType _type, int _attr, int _key );

        DistType( const DistType& _distType );
        DistType &operator=( const DistType &_distType );
        ~DistType( );

        bool isEqual( DistType* _distType );
        distributionType getDistType( );
        int getAttr( );
        int getKey( );

        static std::string getTypeExpr( );
        static std::string getAttrExpr( );
        static std::string getKeyExpr( );

        static DistType* open( 
            SmiRecord& valueRecord, size_t& offset, const ListExpr typeInfo );
        bool save( SmiRecord& valueRecord, size_t& offset );

        static DistType* createDistType( std::string _type );
        static DistType* createDistType( std::string _type, int _attr );
        static DistType* createDistType( 
            std::string _type, int _attr, int _key );
        static DistType* createDistType( distributionType _type );
        static DistType* createDistType( distributionType _type, int _attr );
        static DistType* createDistType( 
            distributionType _type, int _attr, int _key );
        
        static bool readType( ListExpr _list, distributionType& _type );
        static bool readInt( ListExpr _list, int& _attr );

    private:
        /*
        1.2 Members

        1.2.1 ~type~
        
        The distribution type.

        */
        distributionType type;

        /*
        1.2.2 ~attr~

        The number of the attribute used for the distribution. -1 if the 
        attribute is not needed (for example random).

        */
        int attr;

        /*
        1.2.3 ~key~

        The genereted key the check for equal distribution. 

        */
        int key;
    };

} // end of namespace drel

#endif // _DistType_h_