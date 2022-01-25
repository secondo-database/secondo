/*
----
This file is part of SECONDO.

Copyright (C) 2022,
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

//[$][\$]

*/

#ifndef BE_GRID_CREATE_H
#define BE_GRID_CREATE_H

#include "StandardTypes.h"

namespace BasicEngine {


ListExpr be_gridCreateTM(ListExpr args);


/*
1.3.3 Value Mapping

*/
template <class T>
int be_GridCreateSFVM(Word *args, Word &result, int message, Word &local,
                      Supplier s) {

  result = qp->ResultStorage(s);
  bool operationResult = false;

  T *gridName = (T *)args[0].addr;
  CcReal *startX = (CcReal *)args[1].addr;
  CcReal *startY = (CcReal *)args[2].addr;
  CcReal *cellSize = (CcReal *)args[3].addr;
  CcInt *cellsX = (CcInt *)args[4].addr;
  CcInt *cellsY = (CcInt *)args[5].addr;

  CcBool *res = (CcBool *)result.addr;

  try {
    if (!gridName->IsDefined()) {
      std::cerr << "GridName parameter has to be defined" << std::endl;
      operationResult = false;
    } else if (be_control == nullptr) {
      std::cerr << "Please init basic engine first" << std::endl;
      operationResult = false;
    } else if (!be_control->isMaster()) {
      std::cout << noWorker << std::endl;
      operationResult = false;
    } else {
      std::string gridNameString = gridName->toText();
      double startXDouble = startX->GetValue();
      double startYDouble = startY->GetValue();
      double cellSizeDouble = cellSize->GetValue();
      size_t cellsXInt = cellsX->GetValue();
      size_t cellsYInt = cellsY->GetValue();

      GridManager::createGrid(be_control, gridNameString, startXDouble, 
        startYDouble, cellSizeDouble, cellsXInt, cellsYInt);
      operationResult = true;
    }

    res->Set(true, operationResult);
  } catch (SecondoException &e) {
    BOOST_LOG_TRIVIAL(error)
        << "Got error during the creation of the grid" << e.what();
    res->Set(true, false);
  }

  return 0;
}

/*
1.3.3 Specification

*/
OperatorSpec be_gridCreateSpec(
   "{string, text} x real x real x real x int x int--> bool",
   "be_repart_grid(_)",
   "This operator creates a new grid with the given name and specification."
   "(1) Name of the grid, (2) start x, (3) start y, (4) cell size x/y, "
   "(5) cells x, (6) cells y",
   "query be_repart_grid('mygrid', 'gid', 'geod', 'mygrid', darray)"
);

/*
1.3.4 ValueMapping Array

*/
ValueMapping be_gridCreateVM[] = {
  be_GridCreateSFVM<CcString>,
  be_GridCreateSFVM<FText>
};

/*
1.3.5 Selection Function

*/
int be_gridCreateSelect(ListExpr args){
  return CcString::checkType(nl->First(args)) ? 0 : 1;
}

/*
1.3.6 Operator instance

*/
Operator be_gridCreateOp(
  "be_grid_create",
  be_gridCreateSpec.getStr(),
  sizeof(be_gridCreateVM),
  be_gridCreateVM,
  be_gridCreateSelect,
  be_gridCreateTM
);

} // namespace BasicEngine

#endif