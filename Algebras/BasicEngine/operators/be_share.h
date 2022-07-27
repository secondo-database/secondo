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

#ifndef BE_SHARE_H
#define BE_SHARE_H

#include "StandardTypes.h"

namespace BasicEngine {

ListExpr be_shareTM(ListExpr args);

/*
1.3.3 Value Mapping

*/
template <class T>
int be_shareSFVM(Word *args, Word &result, int message, Word &local,
                 Supplier s) {

  result = qp->ResultStorage(s);
  bool shareResult = false;

  T *table = (T *)args[0].addr;
  CcBool *res = (CcBool *)result.addr;

  try {
    if (!table->IsDefined()) {
      std::cerr << "Table parameter has to be defined" << std::endl;
      shareResult = false;
    } else if (be_control == nullptr) {
      std::cerr << "Please init basic engine first" << std::endl;
      shareResult = false;
    } else {
      std::string tableName = table->toText();
      shareResult = be_control->shareTable(tableName);
    }

    res->Set(true, shareResult);
  } catch (SecondoException &e) {
    BOOST_LOG_TRIVIAL(error)
        << "Got error during the sharing of the table " << e.what();
    res->Set(true, false);
  }

  return 0;
}

/*
1.3.3 Specification

*/
OperatorSpec
    be_shareSpec("{string, text} --> bool", "be_share(_)",
                 "This operator shares the given relation with all workers.",
                 "query be_share('cars')");

/*
1.3.4 ValueMapping Array

*/
ValueMapping be_shareVM[] = {be_shareSFVM<CcString>, be_shareSFVM<FText>};

/*
1.3.5 Selection Function

*/
int be_shareSelect(ListExpr args) {
  return CcString::checkType(nl->First(args)) ? 0 : 1;
}

/*
1.3.6 Operator instance

*/
Operator be_shareOp("be_share", be_shareSpec.getStr(), sizeof(be_shareVM),
                    be_shareVM, be_shareSelect, be_shareTM);

} // namespace BasicEngine

#endif