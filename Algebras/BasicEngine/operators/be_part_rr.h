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

#pragma once

#include "StandardTypes.h"
#include "Algebras/FText/FTextAlgebra.h"
#include "Algebras/Distributed2/DArray.h"

namespace BasicEngine {

ListExpr be_partRRTM(ListExpr args);


/*
1.2.2 Value Mapping

*/
template <class T>
int be_partRRSFVM(Word *args, Word &result, int message, Word &local,
                  Supplier s) {

  result = qp->ResultStorage(s);

  T *tab = (T *)args[0].addr;
  CcInt *slot = (CcInt *)args[1].addr;
  distributed2::DArray *res = (distributed2::DArray *)result.addr;

  try {
    if (be_control == nullptr || !be_control->isMaster()) {
      cout << noWorker << endl;
      res->makeUndefined();
      return 0;
    }

    if (slot->GetIntval() <= 0) {
      cout << negSlots << endl;
      res->makeUndefined();
      return 0;
    }

    PartitionData partitionData = {};
    partitionData.table = tab->toText();
    partitionData.slotnum = slot->GetIntval();

    distributed2::DArray val =
        be_control->partitionTableFromMaster(partitionData, rr);

    res->copyFrom(val);

  } catch (SecondoException &e) {
    BOOST_LOG_TRIVIAL(error)
        << "Got error while partitioning table " << e.what();
    res->makeUndefined();
    return 0;
  }
  return 0;
}

/*
1.2.3 Specification

*/
OperatorSpec be_partRRSpec(
   "{string, text} x int --> DArray(SQLREL)",
   "be_part_rr(_,_)",
   "This operator distribute a relation by round-robin "
   "to the worker. The number of slots have to be positiv "
   "and should be a multiple of your number of workers.",
   "query be_part_rr('cars',60)"
);

/*
1.2.4 ValueMapping Array

*/
ValueMapping be_partRRVM[] = {
  be_partRRSFVM<CcString>,
  be_partRRSFVM<FText>
};

/*
1.2.5 Selection Function

*/
int be_partRRSelect(ListExpr args){
  return CcString::checkType(nl->First(args)) ? 0 : 1;
};

/*
1.2.6 Operator instance

*/
Operator be_partRROp(
  "be_part_rr",
  be_partRRSpec.getStr(),
  sizeof(be_partRRVM),
  be_partRRVM,
  be_partRRSelect,
  be_partRRTM
);

} // namespace BasicEngine
