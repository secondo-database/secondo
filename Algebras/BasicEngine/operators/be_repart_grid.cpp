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
1.14.1 Type Mapping

This operator gets a tablename, key-column, a geo\_column, (x,y)
leftbottom coordinates number of slots per row and column
 and the slot size of each square

*/
ListExpr be_repartGridTM(ListExpr args) {
  string err = "\n {string, text} x {string, text} x {string, text} "
               "x {string, text} x DArray(SQLREL) --> bool"
               "(tab-name,geo_col,primary key, grid name, darray)"
               " expected";

  if (!nl->HasLength(args, 5)) {
    return listutils::typeError("Five arguments expected. " + err);
  }

  if (!CcString::checkType(nl->First(args)) &&
      !FText::checkType(nl->First(args))) {
    return listutils::typeError("Value of first argument have "
                                "to be a string or a text." +
                                err);
  }

  if (!CcString::checkType(nl->Second(args)) &&
      !FText::checkType(nl->Second(args))) {
    return listutils::typeError("Value of second argument have "
                                "to be a string or a text." +
                                err);
  }

  if (!CcString::checkType(nl->Third(args)) &&
      !FText::checkType(nl->Third(args))) {
    return listutils::typeError("Value of third argument have "
                                "to be a string or a text." +
                                err);
  }

  if (!CcString::checkType(nl->Fourth(args)) &&
      !FText::checkType(nl->Fourth(args))) {
    return listutils::typeError("Value of fourth argument have "
                                "to be a string or a text." +
                                err);
  }

  if (!DArray::checkType(nl->Fifth(args))) {
    return listutils::typeError("Value of fifth argument have "
                                "to be a darray." +
                                err);
  }

  return nl->SymbolAtom(CcBool::BasicType());
}

} // namespace BasicEngine