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
//[Title] [ \title{Tile Algebra} \author{Dirk Zacher} \maketitle]
//[times] [\ensuremath{\times}]
//[->] [\ensuremath{\rightarrow}]

*/

/*
Tile Algebra implements datatypes and operators for spatial and
moving spatial data. The datatypes implemented in Tile Algebra are
attribute datatypes, so they can be used in relations.

Tile Algebra provides the following datatypes:

tgrid
mtgrid
tint
treal
tbool
tstring
mtint
mtreal
mtbool
mtstring
itint
itreal
itbool
itstring

On these datatypes the following operators are available:

atinstant
atlocation
atperiods
atrange
bbox
CELL1
CELL2
CELLS
compose
deftime
fromline
fromregion
getgrid
inst
map
map2
matchgrid
maximum
minimum
t2mt
tiles
toraster2
toregion
val

*/

/*
SECONDO includes

*/

#include "Algebra.h"
#include "NestedList.h"
#include "QueryProcessor.h"
#include "TileAlgebra.h"

/*
TileAlgebra datatype includes

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
TileAlgebra operators includes

*/

#include "operators/atinstant.h"
#include "operators/atlocation.h"
#include "operators/atperiods.h"
#include "operators/atrange.h"
#include "operators/bbox.h"
#include "operators/CELL1.h"
#include "operators/CELL2.h"
#include "operators/CELLS.h"
#include "operators/compose.h"
#include "operators/deftime.h"
#include "operators/fromline.h"
#include "operators/fromregion.h"
#include "operators/getgrid.h"
#include "operators/inst.h"
#include "operators/load.h"
#include "operators/map.h"
#include "operators/map2.h"
#include "operators/matchgrid.h"
#include "operators/maximum.h"
#include "operators/minimum.h"
#include "operators/t2mt.h"
#include "operators/tiles.h"
#include "operators/toraster2.h"
#include "operators/toregion.h"
#include "operators/val.h"

/*
extern declarations

*/

extern NestedList* nl;
extern QueryProcessor* qp;

/*
Method InitializeTileAlgebra initializes global variables nl of type NestedList
an qp of type QueryProcessor.

author: Dirk Zacher
parameters: pNestedList - a pointer to a NestedList object
            pQueryProcessor - a pointer to global QueryProcessor object
return value: a pointer to Tile Algebra
exceptions: -

*/

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

/*
declaration of namespace TileAlgebra

*/

namespace TileAlgebra
{

/*
Constructor TileAlgebra initializes Tile Algebra by adding type constructors
of Tile Algebra datatypes and by adding operators to Tile Algebra.

author: Dirk Zacher
parameters: -
return value: -
exceptions: -

*/

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

  AddOperator(atinstantInfo(), atinstantFunctions, atinstantSelectFunction,
              atinstantTypeMappingFunction);
  AddOperator(atlocationInfo(), atlocationFunctions, atlocationSelectFunction,
              atlocationTypeMappingFunction);
  AddOperator(atperiodsInfo(), atperiodsFunctions, atperiodsSelectFunction,
              atperiodsTypeMappingFunction);
  AddOperator(atrangeInfo(), atrangeFunctions, atrangeSelectFunction,
              atrangeTypeMappingFunction);
  AddOperator(bboxInfo(), bboxFunctions, bboxSelectFunction,
              bboxTypeMappingFunction);
  AddOperator(CELL1Info(), 0, CELL1TypeMappingFunction);
  AddOperator(CELL2Info(), 0, CELL2TypeMappingFunction);
  AddOperator(CELLSInfo(), 0, CELLSTypeMappingFunction);
  AddOperator(composeInfo(), composeFunctions, composeSelectFunction,
              composeTypeMappingFunction);
  AddOperator(deftimeInfo(), deftimeFunctions, deftimeSelectFunction,
              deftimeTypeMappingFunction);
  AddOperator(fromlineInfo(), fromlineFunction,
              fromlineTypeMappingFunction);
  AddOperator(fromregionInfo(), fromregionFunction,
              fromregionTypeMappingFunction);
  AddOperator(getgridInfo(), getgridFunctions, getgridSelectFunction,
              getgridTypeMappingFunction);
  AddOperator(instInfo(), instFunctions, instSelectFunction,
              instTypeMappingFunction);
  AddOperator(loadInfo(), loadFunctions, loadSelectFunction,
              loadTypeMappingFunction);
  AddOperator(mapInfo(), mapFunctions, mapSelectFunction,
              mapTypeMappingFunction);
  AddOperator(map2Info(), map2Functions, map2SelectFunction,
              map2TypeMappingFunction);
  AddOperator(matchgridInfo(), matchgridFunctions, matchgridSelectFunction,
              matchgridTypeMappingFunction)->SetUsesMemory();
  AddOperator(maximumInfo(), maximumFunctions, maximumSelectFunction,
              maximumTypeMappingFunction);
  AddOperator(minimumInfo(), minimumFunctions, minimumSelectFunction,
              minimumTypeMappingFunction);
  AddOperator(t2mtInfo(), t2mtFunctions, t2mtSelectFunction,
              t2mtTypeMappingFunction);
  AddOperator(tilesInfo(), tilesFunctions, tilesSelectFunction,
              tilesTypeMappingFunction);
  AddOperator(toraster2Info(), toraster2Functions, toraster2SelectFunction,
              toraster2TypeMappingFunction);
  AddOperator(toregionInfo(), toregionFunction,
              toregionTypeMappingFunction);
  AddOperator(valInfo(), valFunctions, valSelectFunction,
              valTypeMappingFunction);
}

/*
Destructor deinitializes Tile Algebra.

author: Dirk Zacher
parameters: -
return value: -
exceptions: -

*/

TileAlgebra::~TileAlgebra()
{
  
}

}
