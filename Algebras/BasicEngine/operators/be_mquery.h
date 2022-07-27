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

#ifndef BE_MQUERY_H
#define BE_MQUERY_H

#include "StandardTypes.h"

namespace BasicEngine {

ListExpr be_mqueryTM(ListExpr args);

/*
1.8.2 Value Mapping

*/
template <class T, class H>
int be_mquerySFVM(Word *args, Word &result, int message, Word &local,
                  Supplier s) {

  bool val = false;
  result = qp->ResultStorage(s);
  T *query = (T *)args[0].addr;
  H *tab = (H *)args[1].addr;

  try {
    if (be_control && be_control->isMaster()) {
      val = be_control->mquery(query->toText(), tab->toText());
    } else {
      cout << noWorker << endl;
    }

    ((CcBool *)result.addr)->Set(true, val);

  } catch (SecondoException &e) {
    BOOST_LOG_TRIVIAL(error)
        << "Got error while executing mquery operator " << e.what();
    ((CcBool *)result.addr)->Set(true, false);
  }
  return 0;
}

/*
1.8.3 Specification

*/
OperatorSpec
    be_mqueryVMSpec("{string,text} x {string,text}-> bool", "be_mquery(_,_)",
                    "Distribute a query to the worker and writes the result in "
                    "the specified table. The statement must be in a correct "
                    "syntax for this DBMS.",
                    "query be_mquery('select * from cars','cars_short')");

/*
1.8.4 ValueMapping Array

*/
ValueMapping be_mqueryVM[] = {
    be_mquerySFVM<CcString, CcString>, be_mquerySFVM<FText, CcString>,
    be_mquerySFVM<CcString, FText>, be_mquerySFVM<FText, FText>};

/*
1.8.5 Selection Function

*/
int be_mquerySelect(ListExpr args) {
  if (CcString::checkType(nl->First(args))) {
    return CcString::checkType(nl->Second(args)) ? 0 : 2;
  } else {
    return CcString::checkType(nl->Second(args)) ? 1 : 3;
  }
};

/*
1.8.6 Operator instance

*/
Operator be_mqueryOp("be_mquery", be_mqueryVMSpec.getStr(), sizeof(be_mqueryVM),
                     be_mqueryVM, be_mquerySelect, be_mqueryTM);

} // namespace BasicEngine

#endif