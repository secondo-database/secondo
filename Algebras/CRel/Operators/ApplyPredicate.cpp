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
#include <cstdint>
#include <exception>
#include "Ints.h"
#include "LongIntsTC.h"
#include "LogMsg.h"
#include "OperatorUtils.h"
#include "QueryProcessor.h"
#include "RelationAlgebra.h"
#include "SecondoSystem.h"
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
  Operator(info, valueMappings, SelectValueMapping, TypeMapping)
{
}

ValueMapping ApplyPredicate::valueMappings[] =
{
  AttrArrayValueMapping,
  TBlockValueMapping,
  nullptr
};

const OperatorInfo ApplyPredicate::info = OperatorInfo(
  "apply", "(ATTRARRAY | tblock) x (map BLOCKENTRY bool) -> longints",
  "_ apply[ fun ]",
  "Determines the indices of those entries in the array / tblock whose "
  "attribute / tuple representations fullfill the predicate function.\n"
  "The returned indices are in ascending order.",
  "query [const longints value (1 2 3)] apply[. >= 2] consume");

ListExpr ApplyPredicate::TypeMapping(ListExpr args)
{
  //Expect two arguments
  if(!nl->HasLength(args,2))
  {
    return GetTypeError("Two arguments expected.");
  }

  //Check the first argument for attrarray or tblock
  const ListExpr firstArg = nl->First(args);

  ListExpr expectedMapArg;

  if (TBlockTI::Check(firstArg))
  {
    expectedMapArg = TBlockTI(firstArg, false).GetTupleTypeExpr();
  }
  else
  {
    AttrArrayTypeConstructor *constructor =
      AttrArray::GetTypeConstructor(firstArg);

    if (constructor == nullptr)
    {
      return GetTypeError(0, "Isn't tblock or ATTRARRAY.");
    }

    expectedMapArg = constructor->GetAttributeType(firstArg, false);
  }

  //Check the second argument for function type
  //Compare the arrays Attribute-Type with the functions parameter type
  //Check the functions result type
  const ListExpr mapType = nl->Second(args);
  if(!nl->HasLength(mapType, 3) ||
     !nl->IsEqual(nl->First(mapType), Symbols::MAP()) ||
     !nl->Equal(nl->Second(mapType), expectedMapArg) ||
     !CcBool::checkType(nl->Third(mapType)))
  {
    const ListExpr expectedMapType =
      nl->ThreeElemList(nl->SymbolAtom(Symbols::MAP()), expectedMapArg,
                        nl->SymbolAtom(CcBool::BasicType()));

    return GetTypeError(1, "map", "Expected " + nl->ToString(expectedMapType) +
                        ".");
  }

  //Result type is 'indices'
  return LongIntsTI(false).GetTypeExpr();
}

int ApplyPredicate::SelectValueMapping(ListExpr args)
{
  return TBlockTI::Check(nl->First(args)) ? 1 : 0;
}

int ApplyPredicate::AttrArrayValueMapping(ArgVector args, Word &result, int,
                                          Word&, Supplier s)
{
  try
  {
    //The function
    const Supplier predicate = args[1].addr;
    //The function's parameter
    Address &predicateArg = (*qp->Argument(predicate))[0].addr;
    //The indices which satisfy the predicate
    LongInts &indices = qp->ResultStorage<LongInts>(result, s);

    indices.Clear();

    for (const AttrArrayEntry &entry : ((AttrArray*)args[0].addr)->GetFilter())
    {
      Attribute *attribute = entry.GetAttribute();

      predicateArg = attribute;

      CcBool &predicateResult = *(CcBool*)qp->Request(predicate).addr;

      attribute->DeleteIfAllowed();

      if (predicateResult.IsDefined() && predicateResult.GetValue())
      {
        indices.Append(entry.GetRow());
      }
    }

    return 0;
  }
  catch (const exception &e)
  {
    ErrorReporter::ReportError(e.what());
  }

  return FAILURE;
}

int ApplyPredicate::TBlockValueMapping(ArgVector args, Word &result, int,
                                       Word&, Supplier s)
{
  try
  {
    //The function
    const Supplier predicate = args[1].addr;

    //The indices which satisfy the predicate
    LongInts &indices = qp->ResultStorage<LongInts>(result, s);

    indices.Clear();

    const TBlock &block = *(TBlock*)args[0].addr;

    const uint64_t columnCount = block.GetColumnCount();

    const ListExpr tupleType =
      TBlockTI(qp->GetType(qp->GetSon(s, 0)), false).GetTupleTypeExpr();

    Tuple tuple = Tuple(SecondoSystem::GetCatalog()->NumericType(tupleType));

    //The function's parameter
    (*qp->Argument(predicate))[0].addr = &tuple;

    for (const TBlockEntry &entry : block.GetFilter())
    {
      for (uint64_t i = 0; i < columnCount; ++i)
      {
        tuple.PutAttribute(i, entry[i].GetAttribute());
      }

      CcBool &predicateResult = *(CcBool*)qp->Request(predicate).addr;

      if (predicateResult.IsDefined() && predicateResult.GetValue())
      {
        indices.Append(entry.GetRow());
      }
    }

    return 0;
  }
  catch (const exception &e)
  {
    ErrorReporter::ReportError(e.what());
  }

  return FAILURE;
}