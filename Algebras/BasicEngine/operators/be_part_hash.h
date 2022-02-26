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

#include "Algebras/Distributed2/DArray.h"
#include "Algebras/FText/FTextAlgebra.h"
#include "StandardTypes.h"

namespace BasicEngine {

ListExpr be_partHashTM(ListExpr args);

/*
1.3.2 Value Mapping

*/
template <class T, class H>
int be_partHashSFVM(Word *args, Word &result, int message, Word &local,
                    Supplier s) {

  result = qp->ResultStorage(s);

  T *tab = (T *)args[0].addr;
  H *key = (H *)args[1].addr;
  CcInt *slot = (CcInt *)args[2].addr;
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
    partitionData.key = key->toText();
    partitionData.slotnum = slot->GetIntval();

    distributed2::DArray val 
      = be_control->partitionTable(partitionData, hash, false);
      
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
1.3.3 Specification

*/
OperatorSpec be_partHashSpec(
    "{string, text} x {string, text} x int --> DArray(SQLREL)",
    "be_part_hash(_,_,_)",
    "This operator distribute a relation by hash-value "
    "to the worker. You can specified a multi key by separating "
    "the fields with a comma. The number of slots have to be positiv "
    "and should be a multiple of your number of workers.",
    "query be_part_hash('cars','moid',60)");

/*
1.3.4 ValueMapping Array

*/
ValueMapping be_partHashVM[] = {
    be_partHashSFVM<CcString, CcString>, be_partHashSFVM<FText, CcString>,
    be_partHashSFVM<CcString, FText>, be_partHashSFVM<FText, FText>};

/*
1.3.5 Selection Function

*/
int be_partHashSelect(ListExpr args) {
  if (CcString::checkType(nl->First(args))) {
    return CcString::checkType(nl->Second(args)) ? 0 : 2;
  } else {
    return CcString::checkType(nl->Second(args)) ? 1 : 3;
  }
}

/*
1.3.6 Operator instance

*/
Operator be_partHashOp("be_part_hash", be_partHashSpec.getStr(),
                       sizeof(be_partHashVM), be_partHashVM, be_partHashSelect,
                       be_partHashTM);

} // namespace BasicEngine
