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

#include "Algebra.h"
#include "NestedList.h"
#include "QueryProcessor.h"
#include "TileAlgebra.h"

/*
includes for TileAlgebra types

*/

#include "t/tintArray.h"
#include "t/tintFlob.h"
#include "grid/tgrid.h"
#include "grid/mtgrid.h"
#include "t/tint.h"
#include "t/treal.h"
#include "t/tbool.h"
#include "t/tstring.h"
#include "mt/mtint.h"
#include "mt/mtreal.h"
#include "mt/mtbool.h"
#include "mt/mtstring.h"
#include "it/itint.h"
#include "it/itreal.h"
#include "it/itbool.h"
#include "it/itstring.h"

/*
includes for TileAlgebra operators

*/

#include "operators/load.h"
#include "operators/atlocation.h"
#include "operators/atinstant.h"
#include "operators/inst.h"
#include "operators/val.h"
#include "operators/atperiods.h"
#include "operators/atrange.h"
#include "operators/deftime.h"
#include "operators/bbox.h"
#include "operators/minimum.h"
#include "operators/maximum.h"
#include "operators/map.h"
#include "operators/map2.h"
#include "operators/fromline.h"
#include "operators/fromregion.h"
#include "operators/toregion.h"
#include "operators/t2mt.h"
#include "operators/compose.h"
#include "operators/getgrid.h"
#include "operators/CELL1.h"
#include "operators/CELL2.h"
// #include "operators/CELLS.h"

/*
extern declarations

*/

extern NestedList* nl;
extern QueryProcessor* qp;

extern "C" Algebra* InitializeTileAlgebra(NestedList* pNestedList,
                                          QueryProcessor* pQueryProcessor)
{
  Algebra* pAlgebra = 0;

  nl = pNestedList;
  assert(nl != 0);

  qp = pQueryProcessor;
  assert(qp != 0);

  pAlgebra = new TileAlgebra::TileAlgebra();
  assert(pAlgebra != 0);

  return pAlgebra;
}

namespace TileAlgebra
{
  
TileAlgebra::TileAlgebra()
            :Algebra()
{
  /*
  Type Constructors

  */

  AddTypeConstructor(new TypeConstructor(tintArray::GetTypeConstructor()));
  AddTypeConstructor(new TypeConstructor(tintFlob::GetTypeConstructor()));
  AddTypeConstructor(new TypeConstructor(tgrid::GetTypeConstructor()));
  AddTypeConstructor(new TypeConstructor(mtgrid::GetTypeConstructor()));
  AddTypeConstructor(new TypeConstructor(tint::GetTypeConstructor()));
  AddTypeConstructor(new TypeConstructor(treal::GetTypeConstructor()));
  AddTypeConstructor(new TypeConstructor(tbool::GetTypeConstructor()));
  AddTypeConstructor(new TypeConstructor(tstring::GetTypeConstructor()));
  AddTypeConstructor(new TypeConstructor(mtint::GetTypeConstructor()));
  AddTypeConstructor(new TypeConstructor(mtreal::GetTypeConstructor()));
  AddTypeConstructor(new TypeConstructor(mtbool::GetTypeConstructor()));
  AddTypeConstructor(new TypeConstructor(mtstring::GetTypeConstructor()));
  AddTypeConstructor(new TypeConstructor(itint::GetTypeConstructor()));
  AddTypeConstructor(new TypeConstructor(itreal::GetTypeConstructor()));
  AddTypeConstructor(new TypeConstructor(itbool::GetTypeConstructor()));
  AddTypeConstructor(new TypeConstructor(itstring::GetTypeConstructor()));

  /*
  Operators

  */

  AddOperator(loadInfo(), loadFunctions, loadSelectFunction,
              loadTypeMappingFunction);
  AddOperator(atlocationInfo(), atlocationFunctions, atlocationSelectFunction,
              atlocationTypeMappingFunction);
  AddOperator(atinstantInfo(), atinstantFunctions, atinstantSelectFunction,
              atinstantTypeMappingFunction);
  AddOperator(instInfo(), instFunctions, instSelectFunction,
              instTypeMappingFunction);
  AddOperator(valInfo(), valFunctions, valSelectFunction,
              valTypeMappingFunction);
  AddOperator(atperiodsInfo(), atperiodsFunctions, atperiodsSelectFunction,
              atperiodsTypeMappingFunction);
  AddOperator(atrangeInfo(), atrangeFunctions, atrangeSelectFunction,
              atrangeTypeMappingFunction);
  AddOperator(deftimeInfo(), deftimeFunctions, deftimeSelectFunction,
              deftimeTypeMappingFunction);
  AddOperator(bboxInfo(), bboxFunctions, bboxSelectFunction,
              bboxTypeMappingFunction);
  AddOperator(minimumInfo(), minimumFunctions, minimumSelectFunction,
              minimumTypeMappingFunction);
  AddOperator(maximumInfo(), maximumFunctions, maximumSelectFunction,
              maximumTypeMappingFunction);
  AddOperator(mapInfo(), mapFunctions, mapSelectFunction,
              mapTypeMappingFunction);
  AddOperator(map2Info(), map2Functions, map2SelectFunction,
              map2TypeMappingFunction);
  AddOperator(fromlineInfo(), fromlineFunction,
              fromlineTypeMappingFunction);
  AddOperator(fromregionInfo(), fromregionFunction,
              fromregionTypeMappingFunction);
  AddOperator(toregionInfo(), toregionFunction,
              toregionTypeMappingFunction);
  AddOperator(t2mtInfo(), t2mtFunctions, t2mtSelectFunction,
              t2mtTypeMappingFunction);
  AddOperator(composeInfo(), composeFunctions, composeSelectFunction,
              composeTypeMappingFunction);
  AddOperator(getgridInfo(), getgridFunctions, getgridSelectFunction,
              getgridTypeMappingFunction);
  AddOperator(CELL1Info(), 0, CELL1TypeMappingFunction);
  AddOperator(CELL2Info(), 0, CELL2TypeMappingFunction);
  // AddOperator(CELLSInfo(), 0, CELLSTypeMappingFunction);
}

TileAlgebra::~TileAlgebra()
{
  
}

}
