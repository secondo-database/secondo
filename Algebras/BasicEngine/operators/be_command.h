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

#ifndef BE_COMMAND_H
#define BE_COMMAND_H

#include "StandardTypes.h"

namespace BasicEngine {

ListExpr be_commandTM(ListExpr args);



/*
1.6.2 Value Mapping

*/
template <class T>
int be_commandSFVM(Word *args, Word &result, int message, Word &local,
                   Supplier s) {

  bool val = false;
  result = qp->ResultStorage(s);

  T *query = (T *)args[0].addr;

  try {
    if (be_control) {
      ConnectionGeneric *dbms_connection = be_control->getDBMSConnection();
      val = dbms_connection->sendCommand(query->GetValue(), true);
    } else {
      cout << noMaster << endl;
    }

    ((CcBool *)result.addr)->Set(true, val);
  } catch (SecondoException &e) {
    BOOST_LOG_TRIVIAL(error)
        << "Got error while executing command operator " << e.what();
    ((CcBool *)result.addr)->Set(true, false);
  }
  return 0;
}

/*
1.6.3 Specification

*/
OperatorSpec be_commandVMSpec(
   "{string, text} --> bool",
   "be_command(_ )",
   "Execute a sql-statement at the locale second DBMS. "
   "The statement must be in a correct syntax for this DBMS. ",
   "query be_command('COPY cars FROM /home/filetransfers/cars_3.bin BINARY')"
);

/*
1.6.4 ValueMapping Array

*/
ValueMapping be_commandVM[] = {
  be_commandSFVM<CcString>,
  be_commandSFVM<FText>
};

/*
1.6.5 Selection Function

*/
int be_commandSelect(ListExpr args){
  return CcString::checkType(nl->First(args))?0:1;
};

/*
1.6.6 Operator instance

*/
Operator be_commandOp(
  "be_command",
  be_commandVMSpec.getStr(),
  sizeof(be_commandVM),
  be_commandVM,
  be_commandSelect,
  be_commandTM
);

} // namespace BasicEngine

#endif