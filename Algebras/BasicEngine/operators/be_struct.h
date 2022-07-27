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

#ifndef BE_STRUCT_H
#define BE_STRUCT_H

#include "StandardTypes.h"

namespace BasicEngine {

ListExpr be_structTM(ListExpr args);

/*
1.11.2 Value Mapping

*/
template <class T>
int be_structSFVM(Word *args, Word &result, int message, Word &local,
                  Supplier s) {

  result = qp->ResultStorage(s);
  T *tab = (T *)args[0].addr;

  try {

    if (be_control) {
      std::string table = tab->GetValue();
      std::string localCreateName =
          be_control->getBasePath() + "/" + be_control->getSchemaFile(table);
      be_control->exportTableCreateStatementSQL(table, localCreateName);
      ((CcBool *)result.addr)->Set(true, true);
    } else {
      cout << noMaster << endl;
      ((CcBool *)result.addr)->Set(true, false);
    }

  } catch (SecondoException &e) {
    BOOST_LOG_TRIVIAL(error)
        << "Got error while create table structure " << e.what();
    ((CcBool *)result.addr)->Set(true, false);
  }
  return 0;
}

/*
1.11.3 Specification

*/
OperatorSpec be_structVMSpec(
    "{string,text} -> bool", "be_struct(_)",
    "This operator creates a table-create-Statement for a "
    "specified table and stores it in a file. This file is "
    "located in your local home-directory in the filetransfer folder."
    "Be sure to have this directory with the correct permissions.",
    "query be_struct('cars_short')");

/*
1.11.4 ValueMapping Array

*/
ValueMapping be_structVM[] = {
    be_structSFVM<CcString>,
    be_structSFVM<FText>,
};

/*
1.11.5 Selection Function

*/
int be_structSelect(ListExpr args) {
  return CcString::checkType(nl->First(args)) ? 0 : 1;
}

/*
1.11.6 Operator instance

*/
Operator be_structOp("be_struct", be_structVMSpec.getStr(), sizeof(be_structVM),
                     be_structVM, be_structSelect, be_structTM);

} // namespace BasicEngine

#endif