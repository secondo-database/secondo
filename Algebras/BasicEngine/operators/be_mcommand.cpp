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
1.9 Operator  ~be\_mcommand~

Execute a command on all worker

1.9.1 Type Mapping

This operator gets a query and a config relation

*/
ListExpr be_mcommandTM(ListExpr args) {
  string err = "{string,text} [x darray(SQLREL) [x darray(SQLREL)]] -> bool"
               "(sql-command, [darray, darray] expected";

  if (nl->ListLength(args) >= 4) {
    return listutils::typeError("Up to three arguments expected.\n " + err);
  }

  if (!CcString::checkType(nl->First(args)) &&
      !FText::checkType(nl->First(args))) {

    return listutils::typeError("Value of first argument have "
                                "to be a string or a text.\n" +
                                err);
  }

  // Append default values
  if (nl->ListLength(args) < 3) {

    ListExpr defaults;

    if (!nl->HasLength(args, 1)) {
      defaults =
          nl->TwoElemList(listutils::getUndefined(), listutils::getUndefined());
    }

    if (!nl->HasLength(args, 2)) {
      defaults = nl->OneElemList(listutils::getUndefined());
    }

    return nl->ThreeElemList(nl->SymbolAtom(Symbols::APPEND()), defaults,
                             nl->SymbolAtom(CcBool::BasicType()));
  }

  // All parameter are present
  return nl->SymbolAtom(CcBool::BasicType());
}

} // namespace BasicEngine