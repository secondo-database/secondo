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

#include "BlockCount.h"

#include <cstdint>
#include <exception>
#include "LogMsg.h"
#include "OperatorUtils.h"
#include "QueryProcessor.h"
#include "Algebras/Stream/Stream.h"
#include <string>
#include "../TBlock.h"
#include "../TypeConstructors/TBlockTC.h"
#include "TypeUtils.h"

using namespace CRelAlgebra;
using namespace CRelAlgebra::Operators;

using std::exception;
using std::string;

extern NestedList *nl;
extern QueryProcessor *qp;

BlockCount::BlockCount() :
  Operator(info, ValueMapping, TypeMapping)
{
}

const OperatorInfo BlockCount::info = OperatorInfo(
  "blockcount", "stream tblock -> int",
  "_ blockcount",
  "Returns the number of blocks in the provided stream.",
  "query people feed filter[attr(., Age) > 50] blockcount");

ListExpr BlockCount::TypeMapping(ListExpr args)
{
  //Expect one parameter
  if (!nl->HasLength(args, 1))
  {
    return GetTypeError("Expected one argument.");
  }

  //Check first parameter for stream
  if (!IsBlockStream(nl->First(args)))
  {
    return GetTypeError(0, "Isn't a stream of tblock.");
  }

  //Result is a 'int'
  return nl->SymbolAtom(CcInt::BasicType());
}

int BlockCount::ValueMapping(ArgVector args, Word &result, int, Word&,
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
      ++count;

      block->DecRef();
    }

    stream.close();

    qp->ResultStorage<CcInt>(result, s).Set(count);

    return 0;
  }
  catch (const exception &e)
  {
    ErrorReporter::ReportError(e.what());
  }

  return FAILURE;
}
