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

#include "../BasicEngine.h"
#include "Algebras/Distributed2/DArray.h"
#include "Algebras/FText/FTextAlgebra.h"
#include "StandardTypes.h"

using namespace distributed2;
using namespace std;

namespace BasicEngine {

/*
1.12 Operator  ~init\_cluster~

Establishes a connection to a running daatbase System.
The result of this operator is a boolean indicating the success
of the operation.

1.12.1 Type Mapping

This operator gets a hostname,a port and a Worker relation.

*/
ListExpr be_init_cluster_tm(ListExpr args) {

  string err = "\n {string, text} x {string, text} x {string, text} "
               "x int x {string, text} x rel "
               "--> bool (db-type, db-user, db-pass, port, db-name,"
               " worker relation) expected";

  // Example
  // ((text 'pgsql') (text 'user')  (text 'pass') (int 50506) (text 'mydb')
  //  ((rel (tuple ((Host string) (Port int) (Config string)
  //  (PGPort int) (DBName string)))) WorkersPG))

  if (!(nl->HasLength(args, 6))) {
    return listutils::typeError("Six arguments expected. " + err);
  }

  ListExpr dbType = nl->First(nl->First(args));
  ListExpr dbUser = nl->First(nl->Second(args));
  ListExpr dbPass = nl->First(nl->Third(args));
  ListExpr dbPort = nl->First(nl->Fourth(args));
  ListExpr dbName = nl->First(nl->Fifth(args));
  ListExpr relation = nl->First(nl->Sixth(args));

  string relationName = nl->ToString(nl->Second(nl->Sixth(args)));

  if (!CcString::checkType(dbType) && !FText::checkType(dbType)) {
    return listutils::typeError("Value of first argument have "
                                "to be a string or a text. " +
                                err);
  }

  if (!CcString::checkType(dbUser) && !FText::checkType(dbUser)) {
    return listutils::typeError("Value of second argument have "
                                "to be a string or a text. " +
                                err);
  }

  if (!CcString::checkType(dbPass) && !FText::checkType(dbPass)) {
    return listutils::typeError("Value of third argument have "
                                "to be a string or a text. " +
                                err);
  }

  if (!CcInt::checkType(dbPort)) {
    return listutils::typeError("Value of fourth argument have "
                                "to be a int." +
                                err);
  }

  if (!CcString::checkType(dbName) && !FText::checkType(dbName)) {
    return listutils::typeError("Value of fifth argument have "
                                "to be a string or a text. " +
                                err);
  }

  if (!Relation::checkType(relation)) {
    return listutils::typeError("Value of sixth argument have "
                                "to be a relation." +
                                err);
  }

  // Append the used relation name to the result
  // The relation is distributed in the VM to the worker
  ListExpr res =
      nl->ThreeElemList(nl->SymbolAtom(Symbol::APPEND()),
                        nl->OneElemList(nl->StringAtom(relationName)),
                        nl->SymbolAtom(CcBool::BasicType()));

  return res;
}

} // namespace BasicEngine