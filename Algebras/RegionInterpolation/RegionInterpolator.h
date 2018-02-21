/*
---- 
This file is part of SECONDO.

Copyright (C) 2004, University in Hagen, Department of Computer Science, 
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


//paragraph [1] Title: [{\Large \bf \begin {center}] [\end {center}}]
//[TOC] [\tableofcontents]

 


[1] Header File and API-Documenation of the RegionInterpoaltionAlgebra

May 2008,  A. Ruloffs


[TOC]

1 Overview

This header file essentially contains the definition of the RegionInterpolationAlgebra class. 
It is usesed to include all other header files.  

2 Defines and includes

*/

#ifndef REGIONINTERPOLATOR_H_
#define REGIONINTERPOLATOR_H_

//#define USE_OVERLAP

#include <vector>
#include <iostream>
#include <iomanip>
#include <string>
#include <limits>
#include <sstream>
#include <stack>
#include <typeinfo>
#include <map>

#include "../../Tools/Flob/DbArray.h"
//#include "DBArray.h"        //needed in graph and path

#include "Algebras/Spatial/SpatialAlgebra.h"    //needed for Points
#include "Algebra.h"        //always needed in Algebras
#include "NestedList.h"        //always needed in Algebras
#include "QueryProcessor.h"    //always needed in Algebras
#include "StandardTypes.h"    //always needed in Algebras
#include "Algebras/Temporal/TemporalAlgebra.h"
#include "Algebras/MovingRegion/MovingRegionAlgebra.h"
#include "DateTime.h"
#include "Attribute.h"

#include "Lines.h"
#include "RegionTreeNode.h"
#include "ConvexHullTreeNode.h"
#include "Utils.h"
#include "Face.h"
#include "Region.h"
#include "SingleMatch.h"
#include "Match.h"
#include "CentroidMatch.h"
#include "SteinerPointMatch.h"
#include "OverlapingMatch.h"
#include "OptimalMatch.h" 
#include "mLineRep.h"



namespace RegionInterpol
{
class RegionInterpolationAlgebra : public Algebra
{
   public:
      RegionInterpolationAlgebra() ;
      ~RegionInterpolationAlgebra();
};
}

extern NestedList* nl;
extern QueryProcessor *qp;

#endif 
