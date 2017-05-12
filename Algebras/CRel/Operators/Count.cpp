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

#include "Count.h"

#include "CRel.h"
#include "CRelTI.h"
#include <cstddef>
#include <exception>
#include "ListUtils.h"
#include "LogMsg.h"
#include "LongInt.h"
#include "OperatorUtils.h"
#include "QueryProcessor.h"
#include "Stream.h"
#include <string>
#include "TBlock.h"
#include "TBlockTI.h"
#include "TypeUtils.h"

using namespace CRelAlgebra;
using namespace CRelAlgebra::Operators;

using listutils::isStream;
using std::exception;
using std::string;

extern NestedList *nl;
extern QueryProcessor *qp;

Count::Count() :
  Operator(info, valueMappings, SelectValueMapping, TypeMapping)
{
}

const OperatorInfo Count::info = OperatorInfo(
  "count", "crel | stream(tblock) | tblock -> longint",
  "_ count",
  "Returns the (accumulated) number of entries for a given column_oriented "
  "relation, tuple block or stream of tuple block(s)",
  "query people feed filter[.Age > 50] count");

ValueMapping Count::valueMappings[] =
{
  CRelValueMapping,
  BlockStreamValueMapping,
  BlockValueMapping,
  nullptr
};

ListExpr Count::TypeMapping(ListExpr args)
{
  //Expect one parameter
  if (!nl->HasLength(args, 1))
  {
    return GetTypeError("Expected one argument!");
  }

  const ListExpr firstArgExpr = nl->First(args);

  //Check first parameter for crel or (stream of) tblock
  if (!CRelTI::Check(firstArgExpr) &&
      !TBlockTI::Check(GetStreamType(firstArgExpr, true)))
  {
    return GetTypeError(0, "Isn't a crel or (stream of) tblock.");
  }

  //Result is a 'int'
  return nl->SymbolAtom(LongInt::BasicType());
}

int Count::SelectValueMapping(ListExpr args)
{
  const ListExpr arg = nl->First(args);

  if (CRelTI::Check(arg))
  {
    return 0;
  }

  return isStream(arg) ? 1 : 2;
}

int Count::CRelValueMapping(ArgVector args, Word &result, int, Word&,
                            Supplier s)
{
  try
  {
    CRel *rel = (CRel*)args[0].addr;

    qp->ResultStorage<LongInt>(result, s).SetValue(rel->GetRowCount());

    return 0;
  }
  catch (const exception &e)
  {
    ErrorReporter::ReportError(e.what());
  }

  return FAILURE;
}

int Count::BlockStreamValueMapping(ArgVector args, Word &result, int, Word&,
                                   Supplier s)
{
  try
  {
    Stream<TBlock> stream = Stream<TBlock>(args[0]);
    TBlock *block;

    stream.open();

    size_t count = 0;

    while ((block = stream.request()) != nullptr)
    {
      count += block->GetFilter().GetRowCount();

      block->DecRef();
    }

    stream.close();

    qp->ResultStorage<LongInt>(result, s).SetValue(count);

    return 0;
  }
  catch (const exception &e)
  {
    ErrorReporter::ReportError(e.what());
  }

  return FAILURE;
}

int Count::BlockValueMapping(ArgVector args, Word &result, int, Word&,
                             Supplier s)
{
  try
  {
    TBlock *block = (TBlock*)args[0].addr;

    qp->ResultStorage<LongInt>(result, s).SetValue(
      block->GetFilter().GetRowCount());

    return 0;
  }
  catch (const exception &e)
  {
    ErrorReporter::ReportError(e.what());
  }

  return FAILURE;
}