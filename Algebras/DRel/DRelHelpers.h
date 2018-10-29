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
#include "Algebras/Collection/CollectionAlgebra.h"
#include "NestedList.h"

#include "DistTypeEnum.h"

//#define DRELDEBUG

namespace distributed2 {
    class DArrayBase;
}

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

            static int getIndex( collection::Collection* vector, 
                Attribute* attr );

            static int countDRel( std::string drel );

            static bool drelCheck( ListExpr list );
            static bool drelCheck( ListExpr list, ListExpr& darray );
            static bool drelCheck( ListExpr list, distributionType& type );
            static bool drelCheck( ListExpr list, 
                ListExpr& darray, distributionType& type, 
                int& attr, int& key );

            static ListExpr repartition4JoinRequired( 
                ListExpr drel1Value, ListExpr drel2Value,
                ListExpr attr1Name, ListExpr attr2Name,
                distributionType dType1, distributionType dType2, 
                ListExpr attr1List, ListExpr attr2List, 
                const int dAttr1, const int dAttr2, 
                const int dKey1, const int dKey2,
                bool& drel1reparti, bool& drel2reparti );

            static ListExpr removeAttrFromAttrList( 
                ListExpr attrList, ListExpr attr );
            static ListExpr removeAttrFromAttrList( 
                ListExpr attrList, std::string attr );
            static ListExpr removeAttrFromAttrList( 
                ListExpr attrList, std::set<std::string>& names );
            static ListExpr removePartitionAttributes( 
                ListExpr attrList, distributionType type );
            static ListExpr getRemovePartitonAttr( distributionType type );

            template<class R, class Q, class T>
            static bool createRepartitionQuery( 
                ListExpr drelType,
                R* drel1,
                T* drel2,
                ListExpr resultDistType,
                string attrName,
                int port,
                ListExpr& query,
                int elem1 = 1,
                int elem2 = 2,
                int streamelem = 3 );
    };

} // end of namespace drel

#endif // _DFRelHelpers_h_