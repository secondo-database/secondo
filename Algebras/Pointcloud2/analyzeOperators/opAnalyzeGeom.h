/*
----
This file is part of SECONDO.

Copyright (C) 2019,
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


//[<] [\ensuremath{<}]
//[>] [\ensuremath{>}]

\setcounter{tocdepth}{3}
\tableofcontents



1 Header for the AnalyzeGeom-Operator

*/
#pragma once
#include <memory>
#include <string>

#include "Operator.h"

#include "AbstractGeomPrimitive.h"

#include "../utility/BitArray.h"
#include "../tcPointcloud2.h"


namespace pointcloud2 {

const std::string OBJ_ID = "ObjID";
const std::string CAT_ID = "CatID";

class op_analyzeGeom {
    static bool REPORT_TO_CONSOLE;

    static ListExpr analyzeGeomTM(ListExpr args);

    static int analyzeGeomVM(Word* args, Word& result, int message,
            Word& local, Supplier s);
public:
    std::shared_ptr<Operator> getOperator();

    std::string getOperatorSpec();

private:
    static std::vector<PointBase<DIMENSIONS>> takeSample(
        const std::shared_ptr<std::vector<DbScanPoint<DIMENSIONS>>>& points,
        const size_t size_of_sample,
        const std::shared_ptr<BitArray> bit_mask);

    /* makes the ObjIDs and CatIDs which were calculated in the analysis
     * persistent by storing them in the tuples associated with the points of
     * the Pointcloud2. */
    static void saveResultsToPointcloud(Pointcloud2* _cloud,
        std::shared_ptr<MMRCloud> mmr_cloud,
        std::shared_ptr<ObjectIdManager> objManager);


    static std::vector<std::shared_ptr<AbstractGeomPrimitive>>
    getAllPrimitives();
};
} // end of namespace
