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

#include "CConsume.h"

#include "CRel.h"
#include "CRelTI.h"
#include <cstddef>
#include <exception>
#include "ListUtils.h"
#include "LogMsg.h"
#include "QueryProcessor.h"
#include "RelationAlgebra.h"
#include "Stream.h"
#include <string>
#include "TBlock.h"
#include "TBlockTI.h"

using namespace CRelAlgebra;
using namespace CRelAlgebra::Operators;

using std::exception;
using std::string;

extern NestedList *nl;
extern QueryProcessor *qp;

CConsume::CConsume() :
  Operator(info, valueMappings, SelectValueMapping, TypeMapping)
{
  SetUsesArgsInTypeMapping();
}

const OperatorInfo CConsume::info = OperatorInfo(
  "cconsume", "stream(tblock(m, (A))) -> crel(c, m, (A))",
  "_ cconsume",
  "Produces a column-oriented relation from a stream of tuple-blocks.",
  "query cities feed cconsume");

ValueMapping CConsume::valueMappings[] =
{
  TBlockValueMapping,
  TupleValueMapping,
  NULL
};

ListExpr CConsume::TypeMapping(ListExpr args)
{
  //Expect two args
  if (!nl->HasLength(args, 2))
  {
    return listutils::typeError("Expected two arguments!");
  }

  //Check first parameter for stream
  ListExpr stream = nl->First(nl->First(args));
  if (!nl->HasLength(stream, 2) ||
      !nl->IsEqual(nl->First(stream), Symbol::STREAM()))
  {
    return listutils::typeError("First argument isn't' a stream!");
  }

  //Check second parameter for 'int' type and boundary
  ListExpr blockSizeArg = nl->Second(nl->Second(args));
  long blockSize;

  if (!nl->IsNodeType(IntType, blockSizeArg) ||
      (blockSize = nl->IntValue(blockSizeArg)) < 0)
  {
    return listutils::typeError("Second argument (block size) isn't an int >= "
                                "0.");
  }

  CRelTI crelInfo;

  ListExpr argType = nl->Second(stream);
  string argTypeError;

  //Check first parameter's stream type for 'tblock'
  if (TBlockTI::Check(argType, argTypeError))
  {
    crelInfo = CRelTI(TBlockTI(argType), 1, blockSize);
  }
  //Check first parameter's stream type for 'tuple'
  else if (Tuple::checkType(argType))
  {
    crelInfo.AppendAttributeInfos(nl->Second(argType));
    crelInfo.SetCacheSize(1);
    crelInfo.SetDesiredBlockSize(blockSize);
  }
  else
  {
    return listutils::typeError("First argument is neither a stream of tblock "
                                "nor a stream of tuple.");
  }

  //Return 'crel' type
  return crelInfo.GetTypeInfo();
}

int CConsume::SelectValueMapping(ListExpr args)
{
  string typeError;
  if (TBlockTI::Check(GetStreamType(nl->First(args)), typeError))
  {
    return 0;
  }

  return 1;
}

int CConsume::TBlockValueMapping(Word* args, Word &result, int, Word&,
                                 Supplier s)
{
  try
  {
    Stream<TBlock> stream = Stream<TBlock>(args[0]);
    CRel &relation = qp->ResultStorage<CRel>(result, s);
    TBlock *block;

    stream.open();

    while ((block = stream.request()) != NULL)
    {
      for (const BlockTuple &tuple : *block)
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

int CConsume::TupleValueMapping(ArgVector args, Word &result, int, Word&,
                                Supplier s)
{
  try
  {
    Stream<Tuple> stream = Stream<Tuple>(args[0]);
    CRel &relation = qp->ResultStorage<CRel>(result, s);
    Tuple *tuple;

    stream.open();

    while ((tuple = stream.request()) != NULL)
    {
      relation.Append(*tuple);

      tuple->DeleteIfAllowed();
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