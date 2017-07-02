/*
----
This file is part of SECONDO.

Copyright (C) 2008, University in Hagen, Department of Computer Science,
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
//[ue] [\"u]
//[ae] [\"a]
//[oe] [\"o]
//[x] [$\times $]
//[->] [$\rightarrow $]

[1] Implementation of the MRegionOpsAlgebra

April - November 2008, M. H[oe]ger for bachelor thesis.

[TOC]

1 Introduction

2 Defines and Includes

*/

#include "PFace.h"
#include "PointVectorSegment.h"
#include "NumericUtil.h"


#ifndef SOURCEUNIT_H
#define SOURCEUNIT_H

namespace temporalalgebra {
  namespace mregionops3 {
    
    enum SetOp {
      INTERSECTION,
      UNION,
      MINUS
    };
    
    class UnitPair {
    public:
      UnitPair();
      
      void addPFace(SourceFlag flag, const ContainerPoint3D& points, 
                    Segment& left, Segment&right);
      bool operate(SetOp setOp);
      size_t countResultUnits();
      void getResultUnit(size_t slide, Unit& unit);

    private:
      Unit unitA;
      Unit unitB;
      GlobalTimeValues timeValues;  
      ContainerPoint3D points; 
    };
    

  } // end of namespace mregionops3
} // end of namespace temporalalgebra
#endif 
// SOURCEUNIT_H