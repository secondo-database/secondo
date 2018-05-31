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
#ifndef _DFRelHelpers_h_
#define _DFRelHelpers_h_

#include "NestedList.h"
#include "Boundary.h"

namespace drel {

    class DRelHelpers {
        public:
            static bool findAttribute( 
                ListExpr attrList, const std::string& name, ListExpr& type );

            static bool countRecords( 
                const std::string attrName, 
                const std::string relName, 
                int& records );
            
            static Boundary* createBoundaryQuery(
                const std::string relation,
                const std::string attrName,
                const int boundarySize );
            
            static std::string randomBoundaryName( );

            static int randomKey( );
    };

} // end of namespace drel

#endif // _DFRelHelpers_h_