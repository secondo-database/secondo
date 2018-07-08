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

#include "Algebras/Temporal/TemporalAlgebra.h"
#include "NestedList.h"
//#include "Boundary.h"

namespace drel {
    /*
    1 Class ~DRelHelpers~

    A class with usefull helper functions for the DRelAlgebra.

    */
    class DRelHelpers {
        public:
            static bool isListOfTwoElemLists( ListExpr list );

            static bool isDRelDescr( ListExpr arg, ListExpr& drelType, 
                ListExpr& relType, ListExpr& distType, ListExpr& drelValue );
            static bool isDRelDescr( ListExpr arg, ListExpr& drelType, 
                ListExpr& relType, ListExpr& distType, ListExpr& drelValue,
                ListExpr& darrayType );

            static bool replaceDRELFUNARG( ListExpr arg, std::string type, 
                ListExpr& fun );
            static bool replaceDRELFUNARG( ListExpr arg, std::string type, 
                ListExpr& fun, ListExpr& map );

            static int computeSampleSize( const int totalSize );
            static int everyNthTupleForSample( const int totalSize );
            static int everyNthTupleForSample(
                const int sampleSize, const int totalSize );
            static int everyNthTupleForArray(
                const int sampleSize, const int arraySize );

            static bool compareAttributes(
                const Attribute* attr1, const Attribute* attr2 );

            static bool listOfIntAtoms( ListExpr list );
    };

} // end of namespace drel

#endif // _DFRelHelpers_h_