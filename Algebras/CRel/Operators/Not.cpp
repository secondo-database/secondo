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
#include "AttrArray.h"
#include <cstddef>
#include <exception>
#include "Ints.h"
#include "LongInt.h"
#include "LongIntsTC.h"
#include "LogMsg.h"
#include "OperatorUtils.h"
#include "QueryProcessor.h"
#include <stdint.h>
#include <string>
#include "Symbols.h"
#include "TBlock.h"
#include "TBlockTC.h"
#include "TypeUtils.h"

using namespace CRelAlgebra::Operators;

using std::back_inserter;
using std::exception;
using std::set_union;
using std::string;
using std::vector;

extern NestedList *nl;
extern QueryProcessor *qp;

Not::Not() :
  Operator(info, valueMappings, SelectValueMapping, TypeMapping)
{
}

ValueMapping Not::valueMappings[] =
{
  RangeValueMapping,
  TBlockValueMapping,
  AttrArrayValueMapping,
  nullptr
};

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
    return GetTypeError("Expected two arguments.");
  }

  const ListExpr firstArg = nl->First(args);

  if (!LongInt::checkType(firstArg) && !TBlockTI::Check(firstArg) &&
      !CheckKind(Kind::ATTRARRAY(), firstArg))
  {
    return GetTypeError(0, "Isn't a longint, tblock or of kind ATTRARRAY.");
  }

  if (!LongIntsTI::Check(nl->Second(args)))
  {
    return GetTypeError(1, "indices", "Isn't of type longints.");
  }

  return LongIntsTI(false).GetTypeExpr();
}

int Not::SelectValueMapping(ListExpr args)
{
  const ListExpr firstArg = nl->First(args);

  return LongInt::checkType(firstArg) ? 0 : TBlockTI::Check(firstArg) ? 1 : 2;
}

int Not::RangeValueMapping(ArgVector args, Word &result, int, Word&, Supplier s)
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

int Not::TBlockValueMapping(ArgVector args, Word &result, int, Word&,
                            Supplier s)
{
  try
  {
    const TBlock &block = *(const TBlock*)args[0].addr;
    const TBlockFilter &filter = block.GetFilter();

    LongInts &set = *(LongInts*)args[1].addr,
      &setInversion = qp->ResultStorage<LongInts>(result, s);

    setInversion.Clear();

    const size_t countA = filter.GetRowCount();

    if (countA > 0)
    {
      size_t indexA = 0;

      for (int64_t b : set)
      {
        int64_t a = filter[indexA++];

        while (a < b)
        {
          setInversion.Append(a);
          a = filter[indexA++];
        }
      }

      while (indexA < countA)
      {
        setInversion.Append(filter[indexA++]);
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

int Not::AttrArrayValueMapping(ArgVector args, Word &result, int, Word&,
                               Supplier s)
{
  try
  {
    const AttrArray &array = *(const AttrArray*)args[0].addr;
    const AttrArrayFilter &filter = array.GetFilter();

    LongInts &set = *(LongInts*)args[1].addr,
      &setInversion = qp->ResultStorage<LongInts>(result, s);

    setInversion.Clear();

    const size_t countA = filter.GetCount();

    if (countA > 0)
    {
      size_t indexA = 0;

      for (int64_t b : set)
      {
        int64_t a = filter[indexA++];

        while (a < b)
        {
          setInversion.Append(a);
          a = filter[indexA++];
        }

        setInversion.Append(a);
      }

      while (indexA < countA)
      {
        setInversion.Append(filter[indexA++]);
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