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

#ifndef BE_GRID_DELETE_H
#define BE_GRUD_DELETE_H

#include "StandardTypes.h"

namespace BasicEngine {

ListExpr be_gridDeleteTM(ListExpr args);



/*
1.3.3 Value Mapping

*/
template <class T>
int be_GridDeleteSFVM(Word *args, Word &result, int message, Word &local,
                      Supplier s) {

  result = qp->ResultStorage(s);
  bool operationResult = false;

  T *gridName = (T *)args[0].addr;
  CcBool *res = (CcBool *)result.addr;

  try {
    if (!gridName->IsDefined()) {
      std::cerr << "Grid name has to be defined" << std::endl;
      operationResult = false;
    } else if (be_control == nullptr) {
      std::cerr << "Please init basic engine first" << std::endl;
      operationResult = false;
    } else if (!be_control->isMaster()) {
      std::cout << noWorker << std::endl;
      operationResult = false;
    } else {
      std::string gridNameString = gridName->toText();
      GridManager::deleteGrid(be_control, gridNameString);
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
OperatorSpec be_gridDeleteSpec(
   "{string, text} --> bool",
   "be_grid_delete(_)",
   "This operator deletes the grid with the given name.",
   "query be_delete_grid('mygrid')"
);

/*
1.3.4 ValueMapping Array

*/
ValueMapping be_gridDeleteVM[] = {
  be_GridDeleteSFVM<CcString>,
  be_GridDeleteSFVM<FText>
};

/*
1.3.5 Selection Function

*/
int be_gridDeleteSelect(ListExpr args){
  return CcString::checkType(nl->First(args)) ? 0 : 1;
}

/*
1.3.6 Operator instance

*/
Operator be_gridDeleteOp(
  "be_grid_delete",
  be_gridDeleteSpec.getStr(),
  sizeof(be_gridDeleteVM),
  be_gridDeleteVM,
  be_gridDeleteSelect,
  be_gridDeleteTM
);


} // namespace BasicEngine

#endif