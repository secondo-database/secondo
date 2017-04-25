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
#include "CRelTI.h"
#include <cstddef>
#include <exception>
#include "ListUtils.h"
#include "LogMsg.h"
#include "QueryProcessor.h"
#include "Stream.h"
#include <string>
#include "TBlock.h"
#include "TBlockTI.h"
#include "TypeUtils.h"

using namespace CRelAlgebra;
using namespace CRelAlgebra::Operators;

using listutils::isStream;
using listutils::typeError;
using std::exception;
using std::string;

extern NestedList *nl;
extern QueryProcessor *qp;

Consume::Consume() :
  Operator(info, ValueMapping, TypeMapping)
{
  SetUsesArgsInTypeMapping();
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
    return typeError("Expected one argument!");
  }

  //Is first parameter a stream?
  ListExpr stream = nl->First(nl->First(args));
  if (!isStream(stream))
  {
    return typeError("First argument (source) isn't' a stream!");
  }

  const ListExpr streamType = GetStreamType(stream);
  string streamTypeError;

  //Is first parameter a stream of 'tblock'?
  if (!TBlockTI::Check(streamType, streamTypeError))
  {
    return typeError("First argument (source) isn't' a stream of 'tblock'!");
  }

  //Return 'crel' type
  return CRelTI(TBlockTI(streamType, false), 1).GetTypeExpr();
}

int Consume::ValueMapping(Word* args, Word &result, int, Word&, Supplier s)
{
  try
  {
    Stream<TBlock> stream = Stream<TBlock>(args[0]);
    CRel &relation = qp->ResultStorage<CRel>(result, s);
    TBlock *block;

    stream.open();

    while ((block = stream.request()) != nullptr)
    {
      for (const TBlockEntry &tuple : *block)
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