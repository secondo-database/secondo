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

#ifndef BE_UNION_H
#define BE_UNION_H

#include "StandardTypes.h"

namespace BasicEngine {

ListExpr be_unionTM(ListExpr args);

/*
1.10.2 Value Mapping

*/
template <class T>
int be_unionSFVM(Word *args, Word &result, int message, Word &local,
                 Supplier s) {

  result = qp->ResultStorage(s);
  T *tab = (T *)args[0].addr;
  bool val = false;

  try {
    if (be_control && be_control->isMaster()) {
      val = be_control->munion(tab->toText());
    } else {
      cout << noWorker << endl;
    }

    ((CcBool *)result.addr)->Set(true, val);
  } catch (SecondoException &e) {
    BOOST_LOG_TRIVIAL(error)
        << "Got error while executing union operator" << e.what();
    ((CcBool *)result.addr)->Set(true, false);
  }

  return 0;
}

/*
1.10.3 Specification

*/
OperatorSpec
    be_unionVMSpec("{string,text} -> bool", "be_union(_ )",
                   "This operator collecting one table from all workers "
                   "to the master.",
                   "query be_union('cars_short')");

/*
1.10.4 ValueMapping Array

*/
ValueMapping be_unionVM[] = {
    be_unionSFVM<CcString>,
    be_unionSFVM<FText>,
};

/*
1.10.5 Selection Function

*/
int be_unionSelect(ListExpr args) {
  return CcString::checkType(nl->First(args)) ? 0 : 1;
}

/*
1.10.6 Operator instance

*/
Operator be_unionOp("be_union", be_unionVMSpec.getStr(), sizeof(be_unionVM),
                    be_unionVM, be_unionSelect, be_unionTM);

} // namespace BasicEngine

#endif
