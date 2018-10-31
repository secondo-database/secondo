/*
----
This file is part of SECONDO.

Copyright (C) 2017, University in Hagen, Department of Computer Science,
Database Systems for New Applications.

SECONDO is free software{} you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation{} either version 2 of the License, or
(at your option) any later version.

SECONDO is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY{} without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with SECONDO{} if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
----

*/

#include "Algebra.h"
#include "NestedList.h"
#include "QueryProcessor.h"
#include "AlgebraManager.h"
#include "StandardTypes.h"  
#include "Symbols.h"
#include "Algebras/Stream/Stream.h"
#include "ListUtils.h"
#include <AlgebraTypes.h>
#include <Operator.h>
#include <fstream>
#include <iostream>
#include "PointCloud.h"
#include "ImportPointCloud.h"

extern NestedList* nl;
extern QueryProcessor *qp;

using namespace std;

namespace pointcloud {

    /*
    ~importpointcloud~ operator

    Imports LSA point data into stream(pointcloud)
    string -> stream(pointcloud)

    Added and maintained by Gundula Swidersky, Dec 2017 

    This file is currently not needed, but kept until the operator is fully
    implemented.
    Maybe some functions will be added in next weeks.

    */




   
} // End of namespace routeplanningalgebra

