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
1.3 Operator  ~be\_grid\_create~

Create a new grid with the given name and
specification

1.3.2 Type Mapping

*/

ListExpr be_gridCreateTM(ListExpr args) {
  string err = "\n {string, text} x real x real x real x int x int --> bool"
               "(grid name, x-value, y-value, slot size, number of slots)"
               " expected";

  if (!nl->HasLength(args, 6)) {
    return listutils::typeError("Six arguments expected. " + err);
  }

  if (!CcString::checkType(nl->First(args)) &&
      !FText::checkType(nl->First(args))) {
    return listutils::typeError("Value of first argument have "
                                "to be a string or a text." +
                                err);
  }

  if (!CcReal::checkType(nl->Second(args))) {
    return listutils::typeError("Value of second argument have "
                                "to be an real." +
                                err);
  }

  if (!CcReal::checkType(nl->Third(args))) {
    return listutils::typeError("Value of third argument have "
                                "to be an real." +
                                err);
  }

  if (!CcReal::checkType(nl->Fourth(args))) {
    return listutils::typeError("Value of fourth argument have "
                                "to be an real." +
                                err);
  }

  if (!CcInt::checkType(nl->Fifth(args))) {
    return listutils::typeError("Value of fifth argument have "
                                "to be an integer." +
                                err);
  }

  if (!CcInt::checkType(nl->Sixth(args))) {
    return listutils::typeError("Value of sixth argument have "
                                "to be an integer." +
                                err);
  }

  return nl->SymbolAtom(CcBool::BasicType());
}

} // namespace BasicEngine