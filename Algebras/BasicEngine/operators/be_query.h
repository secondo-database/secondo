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

#ifndef BE_QUERY_H
#define BE_QUERY_H

#include "StandardTypes.h"

namespace BasicEngine {

ListExpr be_queryTM(ListExpr args);



/*
1.5.2 Value Mapping

*/
template <class T, class H>
int be_querySFVM(Word *args, Word &result, int message, Word &local,
                 Supplier s) {

  std::string query_exec;
  bool val = false;
  result = qp->ResultStorage(s);

  T *query = (T *)args[0].addr;
  H *resultTab = (H *)args[1].addr;

  try {
    if (be_control) {
      // Delete target Table, ignore failure
      std::string tablename = resultTab->toText();
      try {
        be_control->getDBMSConnection()->dropTable(tablename);
      } catch (SecondoException &e) {
        BOOST_LOG_TRIVIAL(debug)
            << "Unable to delete old table " << tablename << ". Ignoring.";
      }

      // execute the query
      val = be_control->createTable(resultTab->toText(), query->toText());
    } else {
      cout << noMaster << endl;
    }

    ((CcBool *)result.addr)->Set(true, val);

  } catch (SecondoException &e) {
    BOOST_LOG_TRIVIAL(error)
        << "Got error while executing query operator " << e.what();
    ((CcBool *)result.addr)->Set(true, false);
  }

  return 0;
}

/*
1.5.3 Specification

*/
OperatorSpec be_queryVMSpec(
   "{string, text} x {string, text} --> bool",
   "be_query(_ , _ )",
   "Execute a sql-statement at the locale second DBMS and stores the "
   "result in the specified table. The statement must be in a correct "
   "syntax for this DBMS. ",
   "query be_query('select * from cars where Speed = 30', 'cars_neu')"
);

/*
1.5.4 ValueMapping Array

*/
ValueMapping be_queryVM[] = {
  be_querySFVM<CcString,CcString>,
  be_querySFVM<FText,CcString>,
  be_querySFVM<CcString,FText>,
  be_querySFVM<FText,FText>
};

/*
1.5.5 Selection Function

*/
int be_querySelect(ListExpr args){
  if(CcString::checkType(nl->First(args))){
    return CcString::checkType(nl->Second(args))?0:2;
  }else{
    return CcString::checkType(nl->Second(args))?1:3;
  }
}

/*
1.5.6 Operator instance

*/
Operator be_queryOp(
  "be_query",
  be_queryVMSpec.getStr(),
  sizeof(be_queryVM),
  be_queryVM,
  be_querySelect,
  be_queryTM
);

} // namespace BasicEngine

#endif