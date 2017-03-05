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

#include "OperatorSelftest.h"   

#include "Algebra.h"
#include "NestedList.h"
#include "NList.h"
#include "LogMsg.h"
#include "QueryProcessor.h"
#include "ConstructorTemplates.h"
#include "StandardTypes.h"
#include <string>
#include "TypeMapUtils.h"
#include "Symbols.h"

extern NestedList*     nl;
extern QueryProcessor* qp;
extern AlgebraManager* am;

namespace temporalalgebra {
  namespace mregionops3 {
/*

6 Implementation of the Algebra Class

*/
    class MRegionOps3Algebra : public Algebra {
    public:
      MRegionOps3Algebra(): Algebra() {
        // Selftetest for the Algebra 
        AddOperator(getSelftestPtr(), true); 
      }// Konstruktor
          
      ~MRegionOps3Algebra() {
      }// Destruktor
        
    }; // class MRegionOps3Algebra
    
  } // end of namespace mregionops3
} // end of namespace temporalalgebra

extern "C" Algebra* InitializeMRegionOps3Algebra(NestedList*     nlRef,
                                                 QueryProcessor* qpRef,
                                                 AlgebraManager* amRef) {
  nl = nlRef;
  qp = qpRef;
  am = amRef;
  return new temporalalgebra::mregionops3::MRegionOps3Algebra();
}