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
#include <cstddef>
#include <exception>
#include "LongInts.h"
#include "LongIntsTI.h"
#include <iterator>
#include "ListUtils.h"
#include "LogMsg.h"
#include "QueryProcessor.h"
#include <string>

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
    return listutils::typeError("Expected two arguments!");
  }

  //Check both arguments for 'indices' type
  string typeError;
  if (!LongIntsTI::Check(nl->First(args), typeError))
  {
    return listutils::typeError("First argument isn't a set of indices: " +
                                typeError);
  }

  if (!LongIntsTI::Check(nl->Second(args), typeError))
  {
    return listutils::typeError("Second argument isn't a set of indices: " +
                                typeError);
  }

  return LongIntsTI(false).GetTypeExpr();
}

int Or::ValueMapping(ArgVector args, Word &result, int, Word&, Supplier s)
{
  try
  {
    LongInts &setA = *(LongInts*)qp->Request(args[0].addr).addr,
      &setB = *(LongInts*)qp->Request(args[1].addr).addr,
      &setIntersection = qp->ResultStorage<LongInts>(result, s);

    setIntersection.Clear();

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
            setIntersection.Append(a);

            if (!iteratorA.MoveToNext())
            {
              if (!iteratorB.MoveToNext())
              {
                return 0;
              }

              do
              {
                setIntersection.Append(iteratorB.Get());
              }
              while(iteratorB.MoveToNext());

              return 0;
            }

            if (!iteratorB.MoveToNext())
            {
              do
              {
                setIntersection.Append(iteratorA.Get());
              }
              while (iteratorA.MoveToNext());

              return 0;
            }

            a = iteratorA.Get();

            b = iteratorB.Get();
          }

          while (a < b)
          {
            setIntersection.Append(a);

            if (!iteratorA.MoveToNext())
            {
              do
              {
                setIntersection.Append(iteratorB.Get());
              }
              while(iteratorB.MoveToNext());

              return 0;
            }

            a = iteratorA.Get();
          }

          while (b < a)
          {
            setIntersection.Append(b);

            if (!iteratorB.MoveToNext())
            {
              do
              {
                setIntersection.Append(iteratorA.Get());
              }
              while(iteratorA.MoveToNext());

              return 0;
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
          setIntersection.Append(iteratorA.Get());
        }
        while (iteratorA.MoveToNext());
      }
    }
    else if (iteratorB.IsValid())
    {
      do
      {
        setIntersection.Append(iteratorB.Get());
      }
      while (iteratorB.MoveToNext());
    }

    return 0;
  }
  catch (const exception &e)
  {
    ErrorReporter::ReportError(e.what());
  }

  return FAILURE;
}