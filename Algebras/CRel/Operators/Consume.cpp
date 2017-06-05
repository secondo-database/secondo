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

#include "Consume.h"

#include "CRel.h"
#include "CRelTC.h"
#include <cstdint>
#include <exception>
#include "LogMsg.h"
#include "OperatorUtils.h"
#include "QueryProcessor.h"
#include "Stream.h"
#include <string>
#include "TBlock.h"
#include "TBlockTC.h"
#include "TypeUtils.h"

using namespace CRelAlgebra;
using namespace CRelAlgebra::Operators;

using std::exception;
using std::string;

extern NestedList *nl;
extern QueryProcessor *qp;

Consume::Consume() :
  Operator(info, ValueMapping, TypeMapping)
{
}

const OperatorInfo Consume::info = OperatorInfo(
  "consume", "stream(tblock) -> crel",
  "_ consume",
  "Creates a column-oriented relation from a stream of tuple blocks.",
  "query cities feed consume");

ListExpr Consume::TypeMapping(ListExpr args)
{
  //One arg?
  if (!nl->HasLength(args, 1))
  {
    return GetTypeError("Expected one argument.");
  }

  ListExpr blockType;

  //Is first parameter a stream of tblock?
  if (!IsBlockStream(nl->First(args), blockType))
  {
    return GetTypeError(0, "source", "Isn't' a stream of tblock.");
  }

  //Return 'crel' type
  return CRelTI(TBlockTI(blockType, false), 1).GetTypeExpr();
}

int Consume::ValueMapping(Word* args, Word &result, int, Word&, Supplier s)
{
  try
  {
    Stream<TBlock> stream = Stream<TBlock>(args[0]);
    CRel &relation = qp->ResultStorage<CRel>(result, s);
    TBlock *block;

    if (relation.GetRowCount() > 0)
    {
      relation.Clear();
    }

    stream.open();

    while ((block = stream.request()) != nullptr)
    {
      for (const TBlockEntry &tuple : block->GetFilter())
      {
        relation.Append(tuple);
      }

      block->DecRef();
    }

    stream.close();

    return 0;
  }
  catch (const exception &e)
  {
    ErrorReporter::ReportError(e.what());
  }

  return FAILURE;
}