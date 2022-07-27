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
1.2 Operator  ~be\_partRR~

Distribute a relation by Round-Robin, sends the data
to the worker and import the data

1.2.1 Type Mapping

This operator gets a tablename

*/
ListExpr be_partRRTM(ListExpr args) {
  string err = "{string, text} x int --> DArray(SQLREL)"
               "(tab-name, key, number of slots) expected";

  if (!nl->HasLength(args, 2)) {
    return listutils::typeError("Three arguments expected.\n " + err);
  }
  if (!CcString::checkType(nl->First(args)) &&
      !FText::checkType(nl->First(args))) {
    return listutils::typeError("Value of first argument have "
                                "to be a string or a text.\n" +
                                err);
  }

  if (!CcInt::checkType(nl->Second(args))) {
    return listutils::typeError("Value of second argument have "
                                "to be an integer.\n" +
                                err);
  }

  return nl->SymbolAtom(DArray::BasicType());
}

} // namespace BasicEngine