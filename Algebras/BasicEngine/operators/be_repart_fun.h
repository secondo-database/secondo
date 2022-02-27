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

#ifndef BE_REPART_FUN_H
#define BE_REPART_FUN_H

#include "Algebras/Distributed2/DArray.h"
#include "Algebras/FText/FTextAlgebra.h"
#include "StandardTypes.h"

namespace BasicEngine {

ListExpr be_repartFunTM(ListExpr args);

/*
1.4.2 Value Mapping for the operator ~be\_repart\_fun~

*/
template <class T, class H, class N>
int be_repartFunSFVM(Word *args, Word &result, int message, Word &local,
                     Supplier s) {

  result = qp->ResultStorage(s);

  T *tab = (T *)args[0].addr;
  H *key = (H *)args[1].addr;
  H *partitionfun = (H *)args[2].addr;
  CcInt *slot = (CcInt *)args[3].addr;
  distributed2::DArray *darray = (distributed2::DArray *)args[4].addr;
  FText *darrayName = (FText *)args[5].addr;
  CcBool *res = (CcBool *) result.addr;

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
    partitionData.table =  tab->toText();
    partitionData.key = key->toText();
    partitionData.partitionfun = partitionfun->toText();
    partitionData.slotnum = slot->GetIntval();

    bool val = be_control -> repartitionTable(partitionData, fun, 
      darray, darrayNameValue);
    
    res->Set(true, val);

  } catch (SecondoException &e) {
    BOOST_LOG_TRIVIAL(error)
        << "Got error during the repartitioning of the table " << e.what();
    res->SetDefined(false);
    return 0;
  }

  return 0;
}

/*
1.4.3 Specification

*/
OperatorSpec be_repartFunSpec(
    "{string, text} x {string, text} x {string, text} x "
    "distributed2::DArray(SQLREL) --> bool",
    "be_repart_fun(_,_,_,_)",
    "This operator redistribute a relation by a special function "
    "to the worker. Special functions are RR, Hash and random. "
    "You can specified a multi key by separating "
    "the fields with a comma.",
    "query be_repart_fun('cars', 'moid', 'random', darray)");

/*
1.4.4 ValueMapping Array

*/
ValueMapping be_repartFunVM[] = {be_repartFunSFVM<CcString, CcString, CcString>,
                                 be_repartFunSFVM<FText, CcString, CcString>,
                                 be_repartFunSFVM<CcString, FText, CcString>,
                                 be_repartFunSFVM<FText, FText, CcString>,
                                 be_repartFunSFVM<CcString, CcString, FText>,
                                 be_repartFunSFVM<FText, CcString, FText>,
                                 be_repartFunSFVM<CcString, FText, FText>,
                                 be_repartFunSFVM<FText, FText, FText>};

/*
1.4.5 Selection Function

*/
int be_repartFunSelect(ListExpr args) {
  if (CcString::checkType(nl->First(args))) {
    if (CcString::checkType(nl->Second(args))) {
      return CcString::checkType(nl->Third(args)) ? 0 : 4;
    } else {
      return CcString::checkType(nl->Third(args)) ? 2 : 6;
    }
  } else {
    if (CcString::checkType(nl->Second(args))) {
      return CcString::checkType(nl->Third(args)) ? 1 : 5;
    } else {
      return CcString::checkType(nl->Third(args)) ? 3 : 7;
    };
  }
};

/*
1.4.6 Operator instance

*/
Operator be_repartFunOp("be_repart_fun", be_repartFunSpec.getStr(),
                        sizeof(be_repartFunVM), be_repartFunVM,
                        be_repartFunSelect, be_repartFunTM);

} // namespace BasicEngine

#endif
