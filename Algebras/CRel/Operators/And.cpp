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

#include "And.h"

#include <algorithm>
#include <cstddef>
#include <exception>
#include "IndicesTI.h"
#include <iterator>
#include "ListUtils.h"
#include "LogMsg.h"
#include "QueryProcessor.h"
#include <string>
#include <vector>

using namespace CRelAlgebra::Operators;

using std::back_inserter;
using std::exception;
using std::max;
using std::set_intersection;
using std::string;
using std::vector;

extern NestedList *nl;
extern QueryProcessor *qp;

And::And() :
  Operator(info, ValueMapping, TypeMapping)
{
}

const OperatorInfo And::info = OperatorInfo(
  "and", "nö",
  "_ feed [fun]",
  "schnö",
  "niemals");

ListExpr And::TypeMapping(ListExpr args)
{
  //Expect two arguments
  if (!nl->HasLength(args, 2))
  {
    return listutils::typeError("Expected two arguments!");
  }

  //Check both arguments for 'indices' type
  string typeError;
  if (!IndicesTI::Check(nl->First(args), typeError))
  {
    return listutils::typeError("First argument isn't a set of indices: " +
                                typeError);
  }

  if (!IndicesTI::Check(nl->Second(args), typeError))
  {
    return listutils::typeError("Second argument isn't a set of indices: " +
                                typeError);
  }

  IndicesTI typeInfoA = IndicesTI(nl->First(args)),
    typeInfoB = IndicesTI(nl->Second(args)),
    typeInfo;

  //The resulting set has the greater of the two arguments's capacities
  typeInfo.SetCapacity(max(typeInfoA.GetCapacity(), typeInfoB.GetCapacity()));

  return typeInfo.GetTypeInfo();
}

int And::ValueMapping(ArgVector args, Word &result, int, Word&, Supplier s)
{
  try
  {
    vector<size_t> &setA = *(vector<size_t>*)args[0].addr,
      &setB = *(vector<size_t>*)args[1].addr,
      &setIntersection = qp->ResultStorage<vector<size_t>>(result, s);

    setIntersection.clear();

    set_intersection(setA.begin(), setA.end(), setB.begin(), setB.end(),
                     back_inserter(setIntersection));

    return 0;
  }
  catch (const exception &e)
  {
    ErrorReporter::ReportError(e.what());
  }

  return FAILURE;
}