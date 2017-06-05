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

#include "Or.h"

#include <algorithm>
#include <cstdint>
#include <exception>
#include "Ints.h"
#include "LongIntsTC.h"
#include <iterator>
#include "ListUtils.h"
#include "LogMsg.h"
#include "OperatorUtils.h"
#include "QueryProcessor.h"
#include <string>

#include "StringUtils.h"

using namespace CRelAlgebra;
using namespace CRelAlgebra::Operators;

using std::back_inserter;
using std::exception;
using std::max;
using std::set_union;
using std::string;
using std::vector;

extern NestedList *nl;
extern QueryProcessor *qp;

Or::Or() :
  Operator(info, ValueMapping, TypeMapping)
{
  SetRequestsArguments();
}

const OperatorInfo Or::info = OperatorInfo(
  "or", "longints x longints -> longints",
  "_ or _",
  "Set union of two sorted sets of integers.\n",
  "query [const longints value (1 2 3)] or [const longints value (2 3 4)]");

ListExpr Or::TypeMapping(ListExpr args)
{
  //Expect two arguments
  if (!nl->HasLength(args, 2))
  {
    return GetTypeError("Expected two arguments.");
  }

  //Check both arguments for 'indices' type
  if (!LongIntsTI::Check(nl->First(args)))
  {
    return GetTypeError(0, "Isn't of type longints.");
  }

  if (!LongIntsTI::Check(nl->Second(args)))
  {
    return GetTypeError(1, "Isn't of type longints.");
  }

  return LongIntsTI(false).GetTypeExpr();
}

void SetUnion(LongInts &setUnion, LongInts &setA, LongInts &setB)
{
  setUnion.Clear();

  LongIntsIterator iteratorA = setA.GetIterator(),
    iteratorB = setB.GetIterator();

  if (iteratorA.IsValid())
  {
    if (iteratorB.IsValid())
    {
      int64_t a = iteratorA.Get(),
        b = iteratorB.Get();

      do
      {
        while (a == b)
        {
          setUnion.Append(a);

          if (!iteratorA.MoveToNext())
          {
            if (!iteratorB.MoveToNext())
            {
              return;
            }

            do
            {
              setUnion.Append(iteratorB.Get());
            }
            while(iteratorB.MoveToNext());

            return;
          }

          if (!iteratorB.MoveToNext())
          {
            do
            {
              setUnion.Append(iteratorA.Get());
            }
            while (iteratorA.MoveToNext());

            return;
          }

          a = iteratorA.Get();

          b = iteratorB.Get();
        }

        while (a < b)
        {
          setUnion.Append(a);

          if (!iteratorA.MoveToNext())
          {
            do
            {
              setUnion.Append(iteratorB.Get());
            }
            while(iteratorB.MoveToNext());

            return;
          }

          a = iteratorA.Get();
        }

        while (b < a)
        {
          setUnion.Append(b);

          if (!iteratorB.MoveToNext())
          {
            do
            {
              setUnion.Append(iteratorA.Get());
            }
            while(iteratorA.MoveToNext());

            return;
          }

          b = iteratorB.Get();
        }
      }
      while (true);
    }
    else
    {
      do
      {
        setUnion.Append(iteratorA.Get());
      }
      while (iteratorA.MoveToNext());
    }
  }
  else if (iteratorB.IsValid())
  {
    do
    {
      setUnion.Append(iteratorB.Get());
    }
    while (iteratorB.MoveToNext());
  }
}

int Or::ValueMapping(ArgVector args, Word &result, int, Word&, Supplier s)
{
  try
  {
    LongInts &indicesA = *(LongInts*)qp->Request(args[0].addr).addr,
      &indices = qp->ResultStorage<LongInts>(result, s);

    LongInts &indicesB = *(LongInts*)qp->Request(args[1].addr).addr;

    SetUnion(indices, indicesA, indicesB);

    return 0;
  }
  catch (const exception &e)
  {
    ErrorReporter::ReportError(e.what());
  }

  return FAILURE;
}

COr::COr() :
  Operator(info, ValueMapping, TypeMapping)
{
  SetRequestsArguments();
}

const OperatorInfo COr::info = OperatorInfo(
  "cor", "longints x longints -> longints",
  "_ cor [fun, fun]",
  "Set union of two sorted sets of integers.\n",
  "Logical disjunktion of two predicate functions over tuple blocks.\n"
  "Predicate functions are evaluated from left to right.\n"
  "");

ListExpr COr::TypeMapping(ListExpr args)
{
  //Expect three arguments
  if (!nl->HasLength(args, 3))
  {
    return GetTypeError("Expected three arguments.");
  }

  const ListExpr blockType = nl->First(args);
  if (!TBlockTI::Check(blockType))
  {
    return GetTypeError(0, "block", "Isn't a tblock.");
  }

  //Check 'predicateA' argument

  const ListExpr mapTypeA = nl->Second(args);

  if (!nl->HasMinLength(mapTypeA, 2) ||
      !nl->IsEqual(nl->First(mapTypeA), Symbols::MAP()))
  {
    return GetTypeError(1, "predicateA", "Isn't a map.");
  }

  if(!nl->HasLength(mapTypeA, 3) ||
     !nl->Equal(nl->Second(mapTypeA), blockType))
  {
    return GetTypeError(1, "predicateA", "Doesn't accept the first argument "
                        "(block).");
  }

  if(!LongIntsTI::Check(nl->Third(mapTypeA)))
  {
    return GetTypeError(1, "predicateA", "Doesn't evaluate to " +
                        LongIntsTC::name);
  }

  const ListExpr mapTypeB = nl->Third(args);

  if (!nl->HasMinLength(mapTypeB, 2) ||
      !nl->IsEqual(nl->First(mapTypeB), Symbols::MAP()))
  {
    return GetTypeError(2, "predicateB", "Isn't a map.");
  }

  if(!nl->HasLength(mapTypeB, 3) ||
     !nl->Equal(nl->Second(mapTypeB), blockType))
  {
    return GetTypeError(2, "predicateB", "Doesn't accept the first argument "
                        "(block).");
  }

  if(!LongIntsTI::Check(nl->Third(mapTypeB)))
  {
    return GetTypeError(2, "predicateB", "Doesn't evaluate to " +
                        LongIntsTC::name);
  }

  return LongIntsTI(false).GetTypeExpr();
}

int COr::ValueMapping(ArgVector args, Word &result, int, Word&, Supplier s)
{
  try
  {
    LongInts &indices = qp->ResultStorage<LongInts>(result, s);

    TBlock *block = (TBlock*)qp->Request(args[0].addr).addr;

    Supplier predicateA = args[1].addr;

    (*qp->Argument(predicateA))[0].addr = block;

    Word resultA;
    qp->Request(predicateA, resultA);

    LongInts &indicesA = *(LongInts*)resultA.addr;

    const uint64_t rowCount = block->GetRowCount(),
      countA = indicesA.GetCount();

    if (countA < rowCount)
    {
      SharedArray<uint64_t> filter(rowCount - indicesA.GetCount());

      uint64_t i = 0;
      int64_t j = 0;

      for (const LongIntEntry &entry : indicesA)
      {
        while (j < entry.value)
        {
          filter[i++] = j++;
        }

        ++j;
      }

      while ((uint64_t)j < rowCount)
      {
        filter[i++] = j++;
      }

      TBlock *filteredBlock = new TBlock(*block, filter);

      Supplier predicateB = args[2].addr;

      (*qp->Argument(predicateB))[0].addr = filteredBlock;

      Word resultB;
      qp->Request(predicateB, resultB);

      filteredBlock->DecRef();

      SetUnion(indices, indicesA, *(LongInts*)resultB.addr);
    }
    else
    {
      indices.Clear();

      for (const LongIntEntry &entry : indicesA)
      {
        indices.Append(entry);
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