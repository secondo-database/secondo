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

#ifndef BE_VALIDATE_QUERY_H
#define BE_VALIDATE_QUERY_H

#include "StandardTypes.h"

namespace BasicEngine {

ListExpr be_validateQueryTM(ListExpr args);

/*
1.3.3 Value Mapping

*/
template <class T>
int be_validateQuerySFVM(Word *args, Word &result, int message, Word &local,
                         Supplier s) {

  result = qp->ResultStorage(s);
  bool validationResult = false;

  T *query = (T *)args[0].addr;
  CcBool *res = (CcBool *)result.addr;

  try {
    if (!query->IsDefined()) {
      std::cerr << "Query parameter has to be defined" << std::endl;
      validationResult = false;
    } else if (be_control == nullptr) {
      std::cerr << "Please init basic engine first" << std::endl;
      validationResult = false;
    } else {
      std::string queryString = query->toText();
      validationResult = be_control->validateQuery(queryString);
    }

    res->Set(true, validationResult);
  } catch (SecondoException &e) {
    BOOST_LOG_TRIVIAL(error)
        << "Got error during the query validation " << e.what();
    res->Set(true, false);
  }

  return 0;
}

/*
1.3.3 Specification

*/
OperatorSpec be_validateQuerySpec(
   "{string, text} --> bool",
   "be_validate_query(_)",
   "This operator validates the given SQL query.",
   "query be_validate_query('SELECT * FROM users')"
);

/*
1.3.4 ValueMapping Array

*/
ValueMapping be_validateQueryVM[] = {
  be_validateQuerySFVM<CcString>,
  be_validateQuerySFVM<FText>
};

/*
1.3.5 Selection Function

*/
int be_validateQuerySelect(ListExpr args){
  return CcString::checkType(nl->First(args)) ? 0 : 1;
}

/*
1.3.6 Operator instance

*/
Operator be_validateQueryOp(
  "be_validate_query",
  be_validateQuerySpec.getStr(),
  sizeof(be_validateQueryVM),
  be_validateQueryVM,
  be_validateQuerySelect,
  be_validateQueryTM
);

} // namespace BasicEngine

#endif