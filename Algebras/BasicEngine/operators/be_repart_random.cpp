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
1.2 Operator  ~be\_repart\_random~

Repartition a relation by random, sends the data
to the worker and import the data

1.2.2 Type Mapping

*/
ListExpr be_repartRandomTM(ListExpr args) {
string err = "{string, text} x DArray(SQLREL) --> bool"
       "(tab-name, key, number of slots) expected";

  if(!nl->HasLength(args,2)){
    return listutils::typeError("Three arguments expected.\n " + err);
  }

  ListExpr table = nl->First(nl->First(args));
  ListExpr darray = nl->First(nl->Second(args));
  string darrayName = nl->ToString(nl->Second(nl->Second(args)));

  if(!CcString::checkType(table) && !FText::checkType(table)){
    return listutils::typeError("Value of first argument have "
                  "to be a string or a text.\n" + err);
  }

  if(!DArray::checkType(darray)){
    return listutils::typeError("Value of second argument have "
                    "to be a darry.\n" + err);
  }

  // Append the used darray name to the result
  // The darray is distributed in the VM to the worker
  ListExpr res =
      nl->ThreeElemList(nl->SymbolAtom(Symbol::APPEND()),
                        nl->OneElemList(nl->StringAtom(darrayName)),
                        nl->SymbolAtom(CcBool::BasicType()));

  return res;
}

} // namespace BasicEngine