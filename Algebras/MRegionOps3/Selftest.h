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

[1] Implementation of the MRegionOps3Algebra

[TOC]

1 Introduction

2 Defines and Includes

*/

#include <string.h>
#include "Algebra.h"
#include "NestedList.h"
#include "NList.h"
#include "StandardTypes.h"

#include "PointVectorSegment.h"
#include "NumericUtil.h"
#include "SetOps.h"

#ifndef SELFTEST_H
#define SELFTEST_H

namespace temporalalgebra {
  namespace mregionops3 {
    
    class Selftest {
    public:  
      Selftest();      
      ~Selftest();
      bool run();
    private:  
      
      void Point3DTest1();
      void Point2DTest1();
        
      void RationalPoint3DTest1();
      void RationalPoint3DTest2();
        
      void RationalVector3DTest1();
      void RationalVector3DTest2();
      void RationalVector3DTest3();
        
      void Segment3DTest();
      void RationalSegment3DTest1();
      void RationalSegment3DTest2();
        
      void RationalPoint2DTest1();
      void RationalPoint2DTest2();
        
      void RationalVector2DTest1();
        
      void Segment2DTest1();
      void Segment2DTest2();
        
      void ContainerPoint3DTest();
        
      void RationalPoint3DExtTest();
      void RationalPoint3DExtSetTest1();
      void RationalPoint3DExtSetTest2();
      void RationalPoint3DExtSetTest3();
      void RationalPoint3DExtSetTest4();
        
      void RationalPlane3DTest1();      
      void RationalPlane3DTest2();       
      void RationalPlane3DTest3();

      void IntersectionPointTest();
        
      void IntersectionSegmentTest1();
      void IntersectionSegmentTest2();
      void IntersectionSegmentTest3();
        
      void IntSegContainerTest1();
      void IntSegContainerTest2();
     
      void PFaceTest1();        
      void PFaceTest2();
      void PFaceTest3();
      void PFaceTest4();
      void PFaceTest5();      
      void PFaceTest6();
      void PFaceTest7();
      void PFaceTest8();
        
      void GlobalTimeValuesTest();
        
     
      void SourceUnitTest1();
      void SourceUnitTest2();
      void SourceUnitTest3();
        
      void IntSegContainerTest3();
      void IntSegContainerTest4();
      void IntSegContainerTest5();
        
      void SegmentTest();
    
      void ResultUnitFactoryTest1();
      void ResultUnitFactoryTest2();
      void ResultUnitFactoryTest3();
      void ResultUnitFactoryTest4();
      void ResultUnitFactoryTest5();
        
      void ContainerSegmentTest1();
       
      void SourceUnitTest4();
      void ResultUnitTest1();
      void UnitTest1();
        
      void MSegmentTest1();
      void MSegmentTest2();

      void SourceUnitPairTest1();
      
      void assert_(std::string test,  std::string message, bool success);
      
      int numberOfTestsRun;
      int numberOfTestsFailed; 
    }; // class Selftest        
  } // end of namespace mregionops3
} // end of namespace temporalalgebra
#endif 
// SELFTEST_H  
