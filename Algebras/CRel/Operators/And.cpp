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

#include <cstddef>
#include <exception>
#include "Ints.h"
#include "LongIntsTC.h"
#include <iterator>
#include "LogMsg.h"
#include "OperatorUtils.h"
#include "QueryProcessor.h"
#include <string>

using namespace CRelAlgebra;
using namespace CRelAlgebra::Operators;

using std::exception;
using std::string;

extern NestedList *nl;
extern QueryProcessor *qp;

And::And() :
  Operator(info, ValueMapping, TypeMapping)
{
  SetRequestsArguments();
}

const OperatorInfo And::info = OperatorInfo(
  "and", "longints x longints -> longints",
  "_ and _",
  "Set intersection of two sorted sets of integers.\n"
  "Arguments are evaluated from left to right.\n"
  "If the left set is empty, the evaluation of the right set is skipped.",
  "");

ListExpr And::TypeMapping(ListExpr args)
{
  //Expect two arguments
  if (!nl->HasLength(args, 2))
  {
    return GetTypeError("Expected two arguments!");
  }

  //Check both arguments for 'indices' type
  if (!LongIntsTI::Check(nl->First(args)))
  {
    return GetTypeError(0, "indices a", "Not of type longints.");
  }

  if (!LongIntsTI::Check(nl->Second(args)))
  {
    return GetTypeError(1, "indices b", "Not of type longints.");
  }

  return LongIntsTI(false).GetTypeExpr();
}

void SetIntersection(LongInts &setIntersection, const LongInts &setA,
                     const LongInts &setB)
{
  setIntersection.Clear();

  LongIntsIterator iteratorA = setA.GetIterator();

  if (iteratorA.IsValid())
  {
    LongIntsIterator iteratorB = setB.GetIterator();

    if (iteratorB.IsValid())
    {
      int64_t a = iteratorA.Get(),
        b = iteratorB.Get();

      do
      {
        while (a == b)
        {
          setIntersection.Append(a);

          if(!iteratorA.MoveToNext() || !iteratorB.MoveToNext())
          {
            return;
          }

          a = iteratorA.Get();

          b = iteratorB.Get();
        }

        while (a < b)
        {
          if (!iteratorA.MoveToNext())
          {
            return;
          }

          a = iteratorA.Get();
        }

        while (b < a)
        {
          if (!iteratorB.MoveToNext())
          {
            return;
          }

          b = iteratorB.Get();
        }
      }
      while (true);
    }
  }
}

int And::ValueMapping(ArgVector args, Word &result, int, Word&, Supplier s)
{
  try
  {
    LongInts &setA = *(LongInts*)qp->Request(args[0].addr).addr,
      &setIntersection = qp->ResultStorage<LongInts>(result, s);

    setIntersection.Clear();

    if (setA.GetCount() > 0)
    {
      SetIntersection(setIntersection, setA,
                      *(LongInts*)qp->Request(args[1].addr).addr);
    }

    return 0;
  }
  catch (const exception &e)
  {
    ErrorReporter::ReportError(e.what());
  }

  return FAILURE;
}

CAnd::CAnd() :
  Operator(info, ValueMapping, TypeMapping)
{
  SetRequestsArguments();
}

const OperatorInfo CAnd::info = OperatorInfo(
  "cand", "tblock x (map tblock longints) x (map tblock longints) -> longints",
  "_ cand[fun, fun]",
  "Logical conjunction of two predicate functions over tuple blocks.\n"
  "Predicate functions are evaluated from left to right.\n",
  "");

ListExpr CAnd::TypeMapping(ListExpr args)
{
  //Expect three arguments
  if (!nl->HasLength(args, 3))
  {
    return GetTypeError("Expected three arguments!");
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

int CAnd::ValueMapping(ArgVector args, Word &result, int, Word&, Supplier s)
{
  try
  {
    LongInts &indices = qp->ResultStorage<LongInts>(result, s);

    indices.Clear();

    TBlock *block = (TBlock*)qp->Request(args[0].addr).addr;

    Supplier predicateA = args[1].addr;

    (*qp->Argument(predicateA))[0].addr = block;

    Word resultA;
    qp->Request(predicateA, resultA);

    LongInts &indicesA = *(LongInts*)resultA.addr;

    const size_t countA = indicesA.GetCount();

    if (countA > 0)
    {
      SharedArray<size_t> filter(countA);

      size_t i = 0;
      for (const LongIntEntry &entry : indicesA)
      {
        filter[i++] = entry.value;
      }

      TBlock *filteredBlock = new TBlock(*block, filter);

      Supplier predicateB = args[2].addr;

      (*qp->Argument(predicateB))[0].addr = filteredBlock;

      Word resultB;
      qp->Request(predicateB, resultB);

      filteredBlock->DecRef();

      for (const LongIntEntry &entry : *(LongInts*)resultB.addr)
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