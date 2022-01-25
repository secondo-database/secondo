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

#ifndef BE_MCOMMAND_H
#define BE_MCOMMAND_H

#include "StandardTypes.h"

namespace BasicEngine {

ListExpr be_mcommandTM(ListExpr args);


/*
1.9.2 Value Mapping

*/
template <class T>
int be_mcommandSFVM(Word *args, Word &result, int message, Word &local,
                    Supplier s) {

  bool val = false;
  result = qp->ResultStorage(s);
  T *query = (T *)args[0].addr;

  try {
    if (be_control && be_control->isMaster()) {
      val = be_control->mcommand(query->toText());
    } else {
      cout << noWorker << endl;
    }

    ((CcBool *)result.addr)->Set(true, val);

  } catch (SecondoException &e) {
    BOOST_LOG_TRIVIAL(error) << "Got error while execute mcommand " << e.what();
    ((CcBool *)result.addr)->Set(true, false);
  }
  return 0;
}

/*
1.9.3 Specification

*/
OperatorSpec be_mcommandVMSpec(
   "{string,text} -> bool",
   "_ be_mcommand(_)",
   "Distribute a sql-command to the worker. The statement "
   "must be in a correct syntax for this DBMS.",
   "query be_mcommand('Drop Table cars;')"
);


/*
1.9.4 ValueMapping Array

*/
ValueMapping be_mcommandVM[] = {
  be_mcommandSFVM<CcString>,
  be_mcommandSFVM<FText>
};

/*
1.9.5 Selection Function

*/
int be_mcommandSelect(ListExpr args){
  return CcString::checkType(nl->First(args))?0:1;
};

/*
1.9.6 Operator instance

*/
Operator be_mcommandOp(
  "be_mcommand",
  be_mcommandVMSpec.getStr(),
  sizeof(be_mcommandVM),
  be_mcommandVM,
  be_mcommandSelect,
  be_mcommandTM
);


} // namespace BasicEngine

#endif