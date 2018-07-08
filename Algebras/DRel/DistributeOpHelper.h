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
#ifndef _DistributeOpHelper_h_
#define _DistributeOpHelper_h_

#include "NestedList.h"
#include "DRelHelpers.h"

namespace distributed2 {
    // used classes and functions of the Distributed2Algebra

    class FRelCopy;
    class RelFileRestorer;

    template<class R>
    ListExpr ddistribute2TMT( ListExpr args );

    template<class AType, class DType, class HType, class CType>
    int ddistribute2VMT(
        Word* args, Word& result, int message, Word& local, Supplier s );

    template<class R>
    ListExpr distribute3TM( ListExpr args );

    template<class AType, class DType, class HType, class CType>
    int distribute3VMT(
        Word* args, Word& result, int message, Word& local, Supplier s );

    template<class R>
    ListExpr distribute4TMT( ListExpr args );

    template<class AType, class DType, class HType, class CType>
    int distribute4VMT(
        Word* args, Word& result, int message, Word& local, Supplier s );
}

namespace drel {

    /*
    1 Class ~DistType~

    This class represents the distirbution type informations for the class
    drel. The supported types are represented by the enum distributionType.
    This type is used for basic distribution like random and replicated.

    */
    class DistributeOpHelper {
    public:
        /*
        1.1 Methods

        */
        static OpTree createStreamOpTree(
            QueryProcessor* qps, ListExpr relType, Relation* rel );

        static OpTree createReplicationOpTree(
            QueryProcessor* qps, Relation* rel, ListExpr relType,
            int size );

        template<class T>
        static OpTree createStreamCellGridOpTree(
            QueryProcessor* qps, Relation* rel, ListExpr relType,
            string attrName, T* grid );

        template<class T>
        static T* createCellGrid(
            Relation* rel, ListExpr relType, string attrName, int size );
    };

} // end of namespace drel

#endif // _DistributeOpHelper_h_