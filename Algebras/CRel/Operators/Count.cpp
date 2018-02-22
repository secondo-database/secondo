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

#include "AttrArray.h"
#include "../CRel.h"
#include "../TypeConstructors/CRelTC.h"
#include <cstdint>
#include <exception>
#include "ListUtils.h"
#include "LogMsg.h"
#include "Algebras/Standard-C++/LongInt.h"
#include "OperatorUtils.h"
#include "QueryProcessor.h"
#include "Algebras/Stream/Stream.h"
#include <string>
#include "Symbols.h"
#include "../TBlock.h"
#include "../TypeConstructors/TBlockTC.h"
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
  ArrayStreamValueMapping,
  ArrayValueMapping,
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
  //or (stream of) ATTRARRAY
  if (!CRelTI::Check(firstArgExpr) &&
      !TBlockTI::Check(GetStreamType(firstArgExpr, true)))
  {
    const ListExpr streamType = GetStreamType(firstArgExpr, true);

    if (!TBlockTI::Check(streamType) &&
        !CheckKind(Kind::ATTRARRAY(), streamType))
    {
      return GetTypeError(0, "Isn't a crel or (stream of) tblock or (stream of)"
                             " ATTRARRAY.");
    }
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

  int index = TBlockTI::Check(arg) ? 1 : 3;

  return isStream(arg) ? index : index + 1;
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

    uint64_t count = 0;

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

int Count::ArrayStreamValueMapping(ArgVector args, Word &result, int, Word&,
                                   Supplier s)
{
  try
  {
    Stream<AttrArray> stream = Stream<AttrArray>(args[0]);
    AttrArray *array;

    stream.open();

    uint64_t count = 0;

    while ((array = stream.request()) != nullptr)
    {
      count += array->GetFilter().GetCount();

      array->DecRef();
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

int Count::ArrayValueMapping(ArgVector args, Word &result, int, Word&,
                             Supplier s)
{
  try
  {
    AttrArray *array = (AttrArray*)args[0].addr;

    qp->ResultStorage<LongInt>(result, s).SetValue(
      array->GetFilter().GetCount());

    return 0;
  }
  catch (const exception &e)
  {
    ErrorReporter::ReportError(e.what());
  }

  return FAILURE;
}
