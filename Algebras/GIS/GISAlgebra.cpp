/*
This file is part of SECONDO.

Copyright (C) 2013, University in Hagen, Department of Computer Science,
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

*/

/*
//[_] [\_]
//[TOC] [\tableofcontents]
//[Title] [ \title{GIS Algebra} \author{Jana Stehmann} \maketitle]
//[times] [\ensuremath{\times}]
//[->] [\ensuremath{\rightarrow}]

*/

/*
GIS Algebra implements operators for raster terrain analysis

The following operators are available:

atinstant
atlocation

*/

/*
SECONDO includes

*/

#include "Algebra.h"
#include "NestedList.h"
#include "QueryProcessor.h"
#include "GISAlgebra.h"

/*
GISAlgebra operators includes

*/

#include "slope.h"
#include "aspect.h"
#include "hillshade.h"
#include "ruggedness.h"
#include "contour.h"

/*
extern declarations

*/

extern NestedList* nl;
extern QueryProcessor* qp;

/*
Method InitializeGISAlgebra initializes global variables nl of type NestedList
an qp of type QueryProcessor.

author: Jana Stehmann
parameters: pNestedList - a pointer to a NestedList object
            pQueryProcessor - a pointer to global QueryProcessor object
return value: a pointer to GIS Algebra
exceptions: -

*/

extern "C" Algebra* InitializeGISAlgebra(NestedList* pNestedList,
                                          QueryProcessor* pQueryProcessor)
{
  Algebra* pAlgebra = 0;

  nl = pNestedList;
  assert(nl != 0);

  qp = pQueryProcessor;
  assert(qp != 0);

  pAlgebra = new GISAlgebra::GISAlgebra();
  assert(pAlgebra != 0);

  return pAlgebra;
}

/*
declaration of namespace GISAlgebra

*/

namespace GISAlgebra
{

/*
Constructor GISAlgebra initializes GIS Algebra by adding type constructors
of GIS Algebra datatypes and by adding operators to GIS Algebra.

author: Jana Stehmann
parameters: -
return value: -
exceptions: -

*/

GISAlgebra::GISAlgebra()
            :Algebra()
{
  /*
  Type Constructors

  */

  /*
  Operators

  */

  AddOperator(slopeInfo(), slopeFuns, slopeSelectFun, slopeTypeMap);
  AddOperator(aspectInfo(), aspectFuns, aspectSelectFun, aspectTypeMap);
  AddOperator(hillshadeInfo(), hillshadeFuns, hillshadeSelectFun, 
              hillshadeTypeMap);
  AddOperator(ruggednessInfo(), ruggednessFuns, ruggednessSelectFun, 
              ruggednessTypeMap);
  Operator* cont = AddOperator(contourInfo(), contourFuns, 
                     contourSelectFun, contourTypeMap);
#ifndef contourlines_fixed_cache
  cont->SetUsesMemory();
#endif
}

/*
Destructor deinitializes GIS Algebra.

author: Jana Stehmann
parameters: -
return value: -
exceptions: -

*/

GISAlgebra::~GISAlgebra()
{
  
}

}
