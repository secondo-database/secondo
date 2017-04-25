/*
----
This file is part of SECONDO.

Copyright (C) 2004, University in Hagen, Department of Computer Science,
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

*/

#include "Not.h"

#include <algorithm>
#include <cstddef>
#include <exception>
#include "LongInt.h"
#include "LongInts.h"
#include "LongIntsTI.h"
#include "ListUtils.h"
#include "LogMsg.h"
#include "QueryProcessor.h"
#include <stdint.h>
#include <string>

using namespace CRelAlgebra::Operators;

using listutils::typeError;
using std::back_inserter;
using std::exception;
using std::set_union;
using std::string;
using std::vector;

extern NestedList *nl;
extern QueryProcessor *qp;

Not::Not() :
  Operator(info, ValueMapping, TypeMapping)
{
}

const OperatorInfo Not::info = OperatorInfo(
  "not", "longint x longints -> longints",
  "not(r, s)",
  "Set inversion for a set s of sorted integers.\n"
  "The inversion is evaluated for those values in the set which satisfy "
  "0 <= value < r.",
  "query not(longint(10), [const longints value (1 2 3)])");

ListExpr Not::TypeMapping(ListExpr args)
{
  if (!nl->HasLength(args, 2))
  {
    return typeError("Expected two arguments!");
  }

  if (!LongInt::checkType(nl->First(args)))
  {
    return typeError("First parameter (range) isn't a longint!");
  }

  string typeError;
  if (!LongIntsTI::Check(nl->Second(args), typeError))
  {
    return listutils::typeError("Second parameter (indices) isn't of type "
                                "longints!");
  }

  return LongIntsTI(false).GetTypeExpr();
}

int Not::ValueMapping(ArgVector args, Word &result, int, Word&, Supplier s)
{
  try
  {
    int64_t range = ((LongInt*)args[0].addr)->GetValue();

    LongInts &set = *(LongInts*)args[1].addr,
      &setInversion = qp->ResultStorage<LongInts>(result, s);

    setInversion.Clear();

    int64_t i = 0;

    for (int64_t j : set)
    {
      while (i < j)
      {
        setInversion.Append(i++);
      }

      if ((i = j + 1) >= range)
      {
        break;
      }
    }

    while (i < range)
    {
      setInversion.Append(i++);
    }

    return 0;
  }
  catch (const exception &e)
  {
    ErrorReporter::ReportError(e.what());
  }

  return FAILURE;
}