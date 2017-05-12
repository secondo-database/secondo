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

#include "AttrArray.h"
#include "CRel.h"
#include "CRelTI.h"
#include <cstddef>
#include <exception>
#include "ListUtils.h"
#include "LogMsg.h"
#include "LongInt.h"
#include "OperatorUtils.h"
#include "QueryProcessor.h"
#include "RelationAlgebra.h"
#include "StandardTypes.h"
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

CConsume::CConsume() :
  Operator(info, valueMappings, SelectValueMapping, TypeMapping)
{
  SetUsesArgsInTypeMapping();
}

ValueMapping CConsume::valueMappings[] =
{
  TBlockValueMapping,
  TupleValueMapping,
  nullptr
};

const OperatorInfo CConsume::info = OperatorInfo(
  "cconsume", "stream(tuple | tblock) x (int | crel) x [int] -> crel",
  "_ cconsume[ _, _ ]",
  "Creates a column-oriented relation from a stream of tuples or tuple blocks. "
  "The second argument can be either the desired block size (int) or a "
  "template (crel) for the new relation. If the second argument is a block "
  "size, a optional third argument representing the relation's cache size "
  "(int) is accepted.\n\n"
  "stream(tuple) x int: The column types in the relation are derived from "
  "the tuple attribute types.\n\n"
  "stream(tblock) x int: The column types in the relation equal those in the "
  "tuple block type.\n"
  "stream(tuple | tblock) x crel): The relation type equals the template's "
  "type.\n",
  "query cities feed cconsume[1]");

ListExpr CConsume::TypeMapping(ListExpr args)
{
  //Two args?
  const size_t argCount = nl->ListLength(args);

  if (argCount < 2 || argCount > 3)
  {
    return GetTypeError("Expected two or three arguments.");
  }

  //First parameter a stream?
  ListExpr stream = nl->First(nl->First(args));
  if (!isStream(stream))
  {
    return GetTypeError(0, "source", "Isn't a stream.");
  }

  const ListExpr streamType = GetStreamType(stream),
    secondArg = nl->Second(args),
    secondArgType = nl->First(secondArg);

  CRelTI typeInfo(false);

  const bool sourceIsBlockStream = TBlockTI::Check(streamType);

  //First parameter a stream of 'tuple'?
  if (!sourceIsBlockStream && !Tuple::checkType(streamType))
  {
    return GetTypeError(0, "source", "Isn't a stream of type tuple or tblock.");
  }

  if (CRelTI::Check(secondArgType))
  {
    typeInfo = CRelTI(secondArgType, false);

    const ListExpr sourceAttributeList = sourceIsBlockStream ?
      nl->Second(TBlockTI(streamType, false).GetTupleTypeExpr()) :
      nl->Second(streamType);

    if (!nl->Equal(sourceAttributeList,
                   nl->Second(typeInfo.GetTupleTypeExpr())))
    {
      if (sourceIsBlockStream)
      {
        return GetTypeError("Attribute types or names of the columns in the "
                            "first argument (source) don't match those in the "
                            "second argument (target template).");
      }

      return GetTypeError("The types or names of the attributes in the first "
                          "argument (source) don't match those in the second "
                          "argument (target template).");
    }

    if (argCount > 2)
    {
      return GetTypeError("Expected two arguments because the second argument "
                          "is of type crel (target template).");
    }
  }
  else
  {
    size_t desiredBlockSize;

    if (!GetSizeTValue(secondArgType, nl->Second(secondArg), desiredBlockSize))
    {
      return GetTypeError(1, "Neither of type int, longint (block size) nor "
                          "crel (target template).");
    }

    size_t cacheSize;

    if (argCount < 3)
    {
      cacheSize = 1;
    }
    else
    {
      const ListExpr cacheSizeExpr = nl->Third(args);

      if (!GetSizeTValue(nl->First(cacheSizeExpr), nl->Second(cacheSizeExpr),
                         cacheSize) ||
          cacheSize == 0)
      {
        return GetTypeError(2, "cache size", "Isn't a int or longint > 0.");
      }
    }

    if (sourceIsBlockStream)
    {
      typeInfo = CRelTI(TBlockTI(streamType, false), cacheSize);
    }
    else
    {
      ListExpr attributeList = nl->Second(streamType),
        columns = nl->Empty(),
        columnsEnd = columns;

      //Create column types of kind ATTRARRAY from attribute types of kind DATA
      while (!nl->IsEmpty(attributeList))
      {
        const ListExpr current = nl->First(attributeList),
          columnName = nl->First(current),
          columnType = AttrArrayTypeConstructor::GetDefaultAttrArrayType(
                        nl->Second(current), false);

        attributeList = nl->Rest(attributeList);

        if (nl->IsEmpty(columns))
        {
          columns = nl->OneElemList(nl->TwoElemList(columnName, columnType));
          columnsEnd = columns;
        }
        else
        {
          columnsEnd = nl->Append(columnsEnd,
                                  nl->TwoElemList(columnName, columnType));
        }
      }

      typeInfo.AppendColumnInfos(columns);
      typeInfo.SetCacheSize(cacheSize);
    }

    typeInfo.SetDesiredBlockSize(desiredBlockSize);
  }

  //Return 'crel' type
  return typeInfo.GetTypeExpr();
}

int CConsume::SelectValueMapping(ListExpr args)
{
  if (TBlockTI::Check(nl->First(args)))
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

int CConsume::TupleValueMapping(ArgVector args, Word &result, int, Word&,
                                Supplier s)
{
  try
  {
    Stream<Tuple> stream = Stream<Tuple>(args[0]);
    CRel &relation = qp->ResultStorage<CRel>(result, s);
    Tuple *tuple;

    stream.open();

    while ((tuple = stream.request()) != nullptr)
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