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

#ifndef BE_RUN_SQL_H
#define BE_RUN_SQL_H

#include "StandardTypes.h"

namespace BasicEngine {

ListExpr be_runsqlTM(ListExpr args);

/*
1.13.2 Value Mapping

*/
template <class T>
int be_runsqlSFVM(Word *args, Word &result, int message, Word &local,
                  Supplier s) {

  T *path = (T *)args[0].addr;
  bool val = false;

  try {
    result = qp->ResultStorage(s);

    if (be_control) {
      val = be_control->runsql(path->toText());
    } else {
      cout << noMaster << endl;
    }

    ((CcBool *)result.addr)->Set(true, val);
  } catch (SecondoException &e) {
    BOOST_LOG_TRIVIAL(error) << "Got error while executing SQL " << e.what();
    ((CcBool *)result.addr)->Set(true, false);
  }

  return 0;
}

/*
1.13.3 Specification

*/
OperatorSpec be_runsqlSpec(
    "{string, text}  --> bool", "be_runsql(_)",
    "Opens a specified file and reading the SQL-Statement. After the the "
    "system execute this statement on the second DBMS. The statement "
    "must be in a correct syntax for this DBMS.",
    "query be_runsql('/home/cbe/filetransfer/createroads.sql')");

/*
1.13.4 ValueMapping Array

*/
ValueMapping be_runsqlVM[] = {
    be_runsqlSFVM<CcString>,
    be_runsqlSFVM<FText>,
};

/*
1.13.5 Selection Function

*/
int be_runsqlSelect(ListExpr args) {
  return CcString::checkType(nl->First(args)) ? 0 : 1;
};

/*
1.13.6 Operator instance

*/
Operator be_runsqlOp("be_runsql", be_runsqlSpec.getStr(), sizeof(be_runsqlVM),
                     be_runsqlVM, be_runsqlSelect, be_runsqlTM);

} // namespace BasicEngine

#endif
