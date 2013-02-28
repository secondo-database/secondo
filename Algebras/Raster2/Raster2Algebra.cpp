/*
This file is part of SECONDO.

Copyright (C) 2011, University in Hagen, Department of Computer Science,
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

#include "Algebra.h"
#include "Raster2Algebra.h"
#include "sint.h"
#include "sreal.h"
#include "sbool.h"
#include "sstring.h"
#include "msint.h"
#include "msreal.h"
#include "msbool.h"
#include "msstring.h"
#include "isint.h"
#include "isreal.h"
#include "isbool.h"
#include "isstring.h"
#include "grid2.h"
#include "grid3.h"
#include "UniqueStringArray.h"
#include "Operators/atlocation.h"
#include "Operators/atinstant.h"
#include "Operators/inst.h"
#include "Operators/val.h"
#include "Operators/atperiods.h"
#include "Operators/atrange.h"
#include "Operators/deftime.h"
#include "Operators/bbox.h"
#include "Operators/minimum.h"
#include "Operators/maximum.h"
#include "Operators/map.h"
#include "Operators/map2.h"
#include "Operators/fromRegion.h"
#include "Operators/toRegion.h"
#include "Operators/s2ms.h"
#include "Operators/compose.h"
#include "Operators/matchgrid.h"
#include "Operators/getgrid.h"
#include "Operators/importHgt.h"
#include "Operators/importEsriGrid.h"
#include "Operators/importEsriRaster.h"
#include "Operators/CELL1.h"
#include "Operators/CELL2.h"
#include "Operators/CELLS.h"
#include "Operators/fromLine.h"
#include "Operators/isdefined.h"

extern NestedList* nl;
extern QueryProcessor* qp;

extern "C"
Algebra* InitializeRaster2Algebra(NestedList* nlRef,
                                  QueryProcessor* qpRef)
{
  nl = nlRef;
  qp = qpRef;
  
  return (new raster2::Raster2Algebra());
}

namespace raster2
{
  
Raster2Algebra::Raster2Algebra()
               :Algebra()
{
  AddTypeConstructor(new TypeConstructor(sint::getTypeConstructor()),true);
  AddTypeConstructor(new TypeConstructor(sreal::getTypeConstructor()),true);
  AddTypeConstructor(new TypeConstructor(sbool::getTypeConstructor()),true);
  AddTypeConstructor(new TypeConstructor(sstring::getTypeConstructor()),true);
  AddTypeConstructor(new TypeConstructor(msint::getTypeConstructor()),true);
  AddTypeConstructor(new TypeConstructor(msreal::getTypeConstructor()),true);
  AddTypeConstructor(new TypeConstructor(msbool::getTypeConstructor()),true);
  AddTypeConstructor(new TypeConstructor(msstring::getTypeConstructor()),true);
  AddTypeConstructor(new TypeConstructor(isint::getTypeConstructor()),true);
  AddTypeConstructor(new TypeConstructor(isreal::getTypeConstructor()),true);
  AddTypeConstructor(new TypeConstructor(isbool::getTypeConstructor()),true);
  AddTypeConstructor(new TypeConstructor(isstring::getTypeConstructor()),true);
  AddTypeConstructor(new TypeConstructor(grid2::getTypeConstructor()),true);
  AddTypeConstructor(new TypeConstructor(grid3::getTypeConstructor()),true);
  AddTypeConstructor(new TypeConstructor(
                         UniqueStringArray::getTypeConstructor()),true);


 
  AddOperator(atlocationInfo(), atlocationFuns, atlocationSelectFun,
              atlocationTypeMap);
  AddOperator(atinstantInfo(), atinstantFuns, atinstantSelectFun,
              atinstantTypeMap)->SetUsesMemory();
  AddOperator(instInfo(), instFuns, instSelectFun, instTypeMap);
  AddOperator(valInfo(), valFuns, valSelectFun, valTypeMap);
  AddOperator(atperiodsInfo(), atperiodsFuns, atperiodsSelectFun,
              atperiodsTypeMap)->SetUsesMemory();
  AddOperator(atrangeInfo(), atrangeFuns, atrangeSelectFun, atrangeTypeMap);
  AddOperator(deftimeInfo(), deftimeFuns, deftimeSelectFun, deftimeTypeMap);
  AddOperator(bboxInfo(), bboxFuns, bboxSelectFun, bboxTypeMap);
  AddOperator(minimumInfo(), minimumFuns, minimumSelectFun, minimumTypeMap);
  AddOperator(maximumInfo(), maximumFuns, maximumSelectFun, maximumTypeMap);
  AddOperator(mapInfo(), mapFuns, mapSelectFun, mapTypeMap);
  AddOperator(map2Info(), map2Funs, map2SelectFun, map2TypeMap);
  AddOperator(fromRegionInfo(), fromRegionFun, fromRegionTypeMap);
  AddOperator(toRegionInfo(), toRegionFun, toRegionTypeMap);
  AddOperator(s2msInfo(), s2msFuns, s2msSelectFun, s2msTypeMap);
  AddOperator(composeInfo(), composeFuns, composeSelectFun, composeTypeMap);

  Operator* mgop = AddOperator(matchgridInfo(), matchgridFuns, 
                               matchgridSelectFun, matchgridTypeMap);
  mgop->SetUsesMemory();
  AddOperator(getgridInfo(), getgridFuns, getgridSelectFun, getgridTypeMap);
  AddOperator(importHgtInfo(), importHgtFun, importHgtTypeMap);
  AddOperator(importEsriGridInfo(), importEsriGridFuns,
		  importEsriGridSelectFun,
		  importEsriGridTypeMap)->SetUsesArgsInTypeMapping();
  AddOperator(importEsriRasterInfo(),
              importEsriRasterFun, importEsriRasterTypeMap);

  AddOperator(cell1Info(), 0, cell1TypeMap);
  AddOperator(cell2Info(), 0, cell2TypeMap);
  AddOperator(cellsInfo(), 0, cellsTypeMap);
  AddOperator(fromLineInfo(), fromLineFun, fromLineTypeMap);
  AddOperator(isdefinedInfo(), isdefinedFuns, isdefinedSelectFun, isdefinedTM);
}

Raster2Algebra::~Raster2Algebra()
{
  
}

}
