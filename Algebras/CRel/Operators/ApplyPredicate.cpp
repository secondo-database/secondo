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

#include "ApplyPredicate.h"

#include "Attribute.h"
#include "AttrArray.h"
#include <cstddef>
#include <exception>
#include "LongInts.h"
#include "LongIntsTI.h"
#include "ListUtils.h"
#include "LogMsg.h"
#include "QueryProcessor.h"
#include "StandardTypes.h"
#include <string>
#include "Symbols.h"
#include "TypeUtils.h"
#include <vector>

using namespace CRelAlgebra;
using namespace CRelAlgebra::Operators;

using std::exception;
using std::string;
using std::vector;

extern NestedList *nl;
extern QueryProcessor *qp;

//ApplyPredicate----------------------------------------------------------------

ApplyPredicate::ApplyPredicate() :
  Operator(info, ValueMapping, TypeMapping)
{
}

const OperatorInfo ApplyPredicate::info = OperatorInfo(
  "apply", "ATTRARRAY x (map DATA bool) -> longints",
  "_ apply[ fun ]",
  "Determines the indices of those entries in the array whose attribute "
  "representations fullfill the predicate function.\n"
  "The returned indices are in ascending order.",
  "query [const longints value (1 2 3)] apply[. >= 2] consume");

ListExpr ApplyPredicate::TypeMapping(ListExpr args)
{
  //Expect two arguments
  if(!nl->HasLength(args,2))
  {
    return listutils::typeError("Two arguments expected.");
  }

  //Check the first argument for 'attrarray' type
  const ListExpr arrayType = nl->First(args);

  if (!CheckKind(Kind::ATTRARRAY(), arrayType))
  {
    return listutils::typeError("First argument isn't of kind ATTRARRAY.");
  }

  //Check the second argument for function type
  const ListExpr mapType = nl->Second(args);
  if(!nl->HasLength(mapType, 3) ||
     !nl->IsEqual(nl->First(mapType), Symbols::MAP()))
  {
    return listutils::typeError("Second argument (map) doesn't take one "
                                "argument.");
  }

  AttrArrayTypeConstructor &constructor =
    (AttrArrayTypeConstructor&)*GetTypeConstructor(arrayType);

  //Compare the arrays Attribute-Type with the functions parameter type
  if (!nl->Equal(nl->Second(mapType),
      constructor.GetAttributeType(arrayType, false)))
  {
    return listutils::typeError("Second argument's (map) argument type doesn't "
                                "match the first argument's (attrarray) "
                                "attribute type.");
  }

  //Check the functions result type
  if(!CcBool::checkType(nl->Third(mapType)))
  {
    return listutils::typeError("Second argument (map) doesn't return a bool.");
  }

  //Result type is 'indices'
  return LongIntsTI(false).GetTypeExpr();
}

int ApplyPredicate::ValueMapping(ArgVector args, Word &result, int, Word&,
                                 Supplier s)
{
  try
  {
    //The function
    const Supplier predicate = args[1].addr;
    //The function's parameter
    Address &predicateArg = (*qp->Argument(predicate))[0].addr;
    //The indices which satisfy the predicate
    LongInts &indices = qp->ResultStorage<LongInts>(result, s);
    indices.IncRef();

    indices.Clear();

    size_t index = 0;

    for (const AttrArrayEntry &entry : *(AttrArray*)args[0].addr)
    {
      Attribute *attribute = entry.GetAttribute();

      predicateArg = attribute;

      CcBool &predicateResult = *(CcBool*)qp->Request(predicate).addr;

      attribute->DeleteIfAllowed();

      if (predicateResult.IsDefined() && predicateResult.GetValue())
      {
        indices.Append(index);
      }

      ++index;
    }

    return 0;
  }
  catch (const exception &e)
  {
    ErrorReporter::ReportError(e.what());
  }

  return FAILURE;
}