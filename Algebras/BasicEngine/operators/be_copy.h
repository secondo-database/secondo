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

#ifndef BE_COPY_H
#define BE_COPY_H

#include "StandardTypes.h"

namespace BasicEngine {
ListExpr be_copyTM(ListExpr args);

/*
1.7.2 Value Mapping

*/
template <class T, class H>
int be_copySFVM(Word *args, Word &result, int message, Word &local,
                Supplier s) {

  result = qp->ResultStorage(s);

  T *source = (T *)args[0].addr;
  H *destination = (H *)args[1].addr;

  if (!source->IsDefined()) {
    std::cerr << "Error: Source parameter is undefined" << std::endl;
    ((CcBool *)result.addr)->Set(true, false);
    return 0;
  }

  if (!destination->IsDefined()) {
    std::cerr << "Error: Destination  parameter is undefined" << std::endl;
    ((CcBool *)result.addr)->Set(true, false);
    return 0;
  }

  if (be_control == nullptr) {
    std::cout << noMaster << endl;
    ((CcBool *)result.addr)->Set(true, false);
    return 0;
  }
  try {
    std::string sourceParameter = source->GetValue();
    std::string destinationParameter = destination->GetValue();

    if (boost::algorithm::starts_with(sourceParameter, "/") &&
        !boost::algorithm::starts_with(destinationParameter, "/")) {

      // Import (First parameter is a file, second a table)
      bool beResult =
          be_control->importTable(destinationParameter, sourceParameter);

      ((CcBool *)result.addr)->Set(true, beResult);
      return 0;

    } else if (!boost::algorithm::starts_with(sourceParameter, "/") &&
               boost::algorithm::starts_with(destinationParameter, "/")) {

      // Export (First parameter is a table, second a file)
      bool beResult =
          be_control->exportTable(sourceParameter, destinationParameter);

      ((CcBool *)result.addr)->Set(true, beResult);
      return 0;
    }

    std::cerr << "Error: Exactly one parameter has to be a "
         << "absolute path, starting with '/'" << std::endl;

    ((CcBool *)result.addr)->Set(true, false);
  } catch (SecondoException &e) {
    BOOST_LOG_TRIVIAL(error)
        << "Got error while executing copy operator " << e.what();
    ((CcBool *)result.addr)->Set(true, false);
  }
  return 0;
}

/*
1.7.3 Specification

*/
OperatorSpec be_copyVMSpec(
    "{string, text} x {string, text} --> bool", "be_copy(_,_)",
    "You can use this operator to import or export a relation "
    "to a file. Be sure to have the permissions to read an write in "
    "this folder. be_copy(From,To), From/To can be a table or a path.",
    "query be_copy('cars','/home/filetransfers/cars_3.bin')");

/*
1.7.4 ValueMapping Array

*/
ValueMapping be_copyVM[] = {
    be_copySFVM<CcString, CcString>, be_copySFVM<FText, CcString>,
    be_copySFVM<CcString, FText>, be_copySFVM<FText, FText>};

/*
1.7.5 Selection Function

*/
int be_copySelect(ListExpr args) {
  if (CcString::checkType(nl->First(args))) {
    return CcString::checkType(nl->Second(args)) ? 0 : 2;
  } else {
    return CcString::checkType(nl->Second(args)) ? 1 : 3;
  }
};

/*
1.7.6 Operator instance

*/
Operator be_copyOp("be_copy", be_copyVMSpec.getStr(), sizeof(be_copyVM),
                   be_copyVM, be_copySelect, be_copyTM);

} // namespace BasicEngine

#endif