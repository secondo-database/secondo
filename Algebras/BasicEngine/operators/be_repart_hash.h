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

ListExpr be_repartHashTM(ListExpr args);

/*
1.3.2 Value Mapping

*/
template <class T, class H>
int be_repartHashSFVM(Word *args, Word &result, int message, Word &local,
                      Supplier s) {

  result = qp->ResultStorage(s);

  T *tab = (T *)args[0].addr;
  H *key = (H *)args[1].addr;
  CcInt *slot = (CcInt *)args[2].addr;
  distributed2::DArray *darray = (distributed2::DArray *)args[3].addr;

  CcBool *res = (CcBool *) result.addr;

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
    partitionData.key = key->toText();
    partitionData.slotnum = slot->GetIntval();

    bool val = be_control -> repartitionTable(partitionData, hash, darray);
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
1.3.3 Specification

*/
OperatorSpec be_repartHashSpec(
    "{string, text} x {string, text} x distributed2::DArray(SQLREL) --> bool",
    "be_repart_hash(_,_,_)",
    "This operator repartition a relation by hash-value "
    "to the worker of the daaray. You can specify a multi "
    "key by separating the fields with a comma.",
    "query be_repart_hash('cars', 'moid', darray)");

/*
1.3.4 ValueMapping Array

*/
ValueMapping be_repartHashVM[] = {
    be_repartHashSFVM<CcString, CcString>, be_repartHashSFVM<FText, CcString>,
    be_repartHashSFVM<CcString, FText>, be_repartHashSFVM<FText, FText>};

/*
1.3.5 Selection Function

*/
int be_repartHashSelect(ListExpr args) {
  if (CcString::checkType(nl->First(args))) {
    return CcString::checkType(nl->Second(args)) ? 0 : 2;
  } else {
    return CcString::checkType(nl->Second(args)) ? 1 : 3;
  }
}

/*
1.3.6 Operator instance

*/
Operator be_repartHashOp("be_repart_hash", be_repartHashSpec.getStr(),
                         sizeof(be_repartHashVM), be_repartHashVM,
                         be_repartHashSelect, be_repartHashTM);

} // namespace BasicEngine
