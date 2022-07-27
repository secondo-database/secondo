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

ListExpr be_repartRandomTM(ListExpr args);

/*
1.2.2 Value Mapping

*/
template <class T>
int be_repartRandomSFVM(Word *args, Word &result, int message, Word &local,
                        Supplier s) {

  result = qp->ResultStorage(s);

  T *tab = (T *)args[0].addr;
  CcInt *slot = (CcInt *)args[1].addr;
  distributed2::DArray *darray = (distributed2::DArray *)args[2].addr;
  FText *darrayName = (FText *)args[3].addr;
  CcBool *res = (CcBool *)result.addr;

  if (!darrayName->IsDefined()) {
    std::cerr << "Error: DArray name is undefined" << std::endl;
    res->Set(true, false);
    return 0;
  }

  std::string darrayNameValue = darrayName->toText();

  try {

    if (be_control == nullptr) {
      std::cerr << "Please init basic engine first" << std::endl;
      res->SetDefined(false);
      return 0;
    }

    if (slot->GetIntval() <= 0) {
      cout << negSlots << std::endl;
      res->SetDefined(false);
      return 0;
    }

    PartitionData partitionData = {};
    partitionData.table = tab->toText();
    partitionData.slotnum = slot->GetIntval();

    bool val = be_control->repartitionTable(partitionData, random, darray,
                                            darrayNameValue);

    res->Set(true, val);
  } catch (SecondoException &e) {
    BOOST_LOG_TRIVIAL(error)
        << "Got error while repartitioning the table " << e.what();
    res->SetDefined(false);
    return 0;
  }

  return 0;
}

/*
1.2.3 Specification

*/
OperatorSpec be_repartRandomSpec(
    "{string, text} x distributed2::DArray(SQLREL) --> bool",
    "be_repart_random(_,_)",
    "This operator repartition a relation by random "
    "to the worker of the darray.",
    "query be_repart_random('cars', darray)");

/*
1.2.4 ValueMapping Array

*/
ValueMapping be_repartRandomVM[] = {be_repartRandomSFVM<CcString>,
                                    be_repartRandomSFVM<FText>};

/*
1.2.5 Selection Function

*/
int be_repartRandomSelect(ListExpr args) {
  return CcString::checkType(nl->First(args)) ? 0 : 1;
};

/*
1.2.6 Operator instance

*/
Operator be_repartRandomOp("be_repart_random", be_repartRandomSpec.getStr(),
                           sizeof(be_repartRandomVM), be_repartRandomVM,
                           be_repartRandomSelect, be_repartRandomTM);

} // namespace BasicEngine
