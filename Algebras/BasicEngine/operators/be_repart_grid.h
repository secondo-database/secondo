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

ListExpr be_repartGridTM(ListExpr args);

/*
1.14.2 Value Mapping for the operator ~be\_repart\_grid~

*/
template <class T, class H, class I, class K>
int be_repartGridSFVM(Word *args, Word &result, int message, Word &local,
                      Supplier s) {

  result = qp->ResultStorage(s);

  T *tab = (T *)args[0].addr;
  H *key = (H *)args[1].addr;
  I *geo_col = (I *)args[2].addr;
  K *gridname = (K *)args[3].addr;
  CcInt *slot = (CcInt *)args[4].addr;
  distributed2::DArray *darray = (distributed2::DArray *)args[5].addr;
  FText *darrayName = (FText *)args[6].addr;
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
    partitionData.table = tab->toText();
    partitionData.key = key->toText();
    partitionData.attribute = geo_col->toText();
    partitionData.gridname = gridname->toText();
    partitionData.slotnum = slot->GetIntval();

    bool val = be_control -> repartitionTable(partitionData, grid, 
      darray, darrayNameValue);

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
1.14.3 Specification

*/
OperatorSpec be_repartGridSpec(
    "{string, text} x {string, text} x {string, text} "
    "x {string, text} x distributed2::DArray(SQLREL) --> bool",
    "be_repart_grid(_,_,_,_,_)",
    "This operator re-distribute a relation by specified grid "
    "to the worker. You can specified the name of the grid. "
    "The number of slots and size have to be "
    "positive. The column should be a spatial attribute",
    "query be_repart_grid('water', 'gid', 'geog', 'mygrid', darray)");

/*
1.14.4 ValueMapping Array

*/
ValueMapping be_repartGridVM[] = {
    be_repartGridSFVM<CcString, CcString, CcString, CcString>,
    be_repartGridSFVM<FText, CcString, CcString, CcString>,
    be_repartGridSFVM<CcString, FText, CcString, CcString>,
    be_repartGridSFVM<FText, FText, CcString, CcString>,
    be_repartGridSFVM<CcString, CcString, FText, CcString>,
    be_repartGridSFVM<FText, CcString, FText, CcString>,
    be_repartGridSFVM<CcString, FText, FText, CcString>,
    be_repartGridSFVM<FText, FText, FText, CcString>,
    be_repartGridSFVM<CcString, CcString, CcString, FText>,
    be_repartGridSFVM<FText, CcString, CcString, FText>,
    be_repartGridSFVM<CcString, FText, CcString, FText>,
    be_repartGridSFVM<FText, FText, CcString, FText>,
    be_repartGridSFVM<CcString, CcString, FText, FText>,
    be_repartGridSFVM<FText, CcString, FText, FText>,
    be_repartGridSFVM<CcString, FText, FText, FText>,
    be_repartGridSFVM<FText, FText, FText, FText>};

/*
1.14.5 Selection Function

*/
int be_repartGridSelect(ListExpr args) {
  if (CcString::checkType(nl->Fourth(args))) {
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
      }
    }
  } else {
    if (CcString::checkType(nl->First(args))) {
      if (CcString::checkType(nl->Second(args))) {
        return CcString::checkType(nl->Third(args)) ? 8 : 12;
      } else {
        return CcString::checkType(nl->Third(args)) ? 10 : 14;
      }
    } else {
      if (CcString::checkType(nl->Second(args))) {
        return CcString::checkType(nl->Third(args)) ? 9 : 13;
      } else {
        return CcString::checkType(nl->Third(args)) ? 12 : 15;
      }
    }
  }
}

/*
1.14.6 Operator instance

*/
Operator be_repartGridOp("be_repart_grid", be_repartGridSpec.getStr(),
                         sizeof(be_repartGridVM), be_repartGridVM,
                         be_repartGridSelect, be_repartGridTM);

} // namespace BasicEngine

