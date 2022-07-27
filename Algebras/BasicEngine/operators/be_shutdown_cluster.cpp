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

#include <boost/log/core.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/trivial.hpp>

#include "Algebras/Distributed2/DArray.h"
#include "Algebras/FText/FTextAlgebra.h"
#include "StandardTypes.h"
#include "../BasicEngine.h"

using namespace distributed2;
using namespace std;

namespace BasicEngine {

/*
1.1.1 Type Mapping

This operator has no paramter

*/
ListExpr be_shutdown_cluster_tm(ListExpr args) {
  string err = "No parameter (--> bool) expected";

  if (!(nl->HasLength(args, 0))) {
    return listutils::typeError("No arguments expected. " + err);
  }
  return nl->SymbolAtom(CcBool::BasicType());
}

/*
1.1.2 Value Mapping

*/
int be_shutdown_cluster_vm(Word *args, Word &result, int message, Word &local,
                           Supplier s) {

  result = qp->ResultStorage(s);

  try {
    if (be_control != nullptr) {

      if (!be_control->isMaster()) {
        cout << "Error: Can not shutdown worker nodes, we are not"
             << " in master mode" << endl;
        cout << "Please use be_shutdown to shutdown the local engine." << endl
             << endl;
        ((CcBool *)result.addr)->Set(false, true);
        return 0;
      }

      cout << "Shutting down basic engine worker" << endl;
      bool shutdownResult = be_control->shutdownWorker();

      if (!shutdownResult) {
        cout << "Error: Shutdown of the workers failed" << endl << endl;

        ((CcBool *)result.addr)->Set(true, false);
        return 0;
      }

      cout << "Shutting down basic engine master" << endl;
      delete be_control;
      be_control = nullptr;

      ((CcBool *)result.addr)->Set(true, true);
      return 0;
    } else {
      cout << "Basic engine is not active" << endl;
      ((CcBool *)result.addr)->Set(true, false);
      return 0;
    }
  } catch (SecondoException &e) {
    BOOST_LOG_TRIVIAL(error)
        << "Got error while shutdown basic engine " << e.what();
    ((CcBool *)result.addr)->Set(true, false);
  }

  return 0;
}

} // namespace BasicEngine