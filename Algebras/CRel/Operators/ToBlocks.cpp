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

#include "ToBlocks.h"

#include "AttrArray.h"
#include <cstddef>
#include <exception>
#include "ListUtils.h"
#include "LogMsg.h"
#include "LongInt.h"
#include "QueryProcessor.h"
#include "StandardTypes.h"
#include "StreamValueMapping.h"
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

ToBlocks::ToBlocks() :
  Operator(info, StreamValueMapping<State>, TypeMapping)
{
  SetUsesArgsInTypeMapping();
}

const OperatorInfo ToBlocks::info = OperatorInfo(
  "toblocks", "stream(tuple) x (int | tblock) -> stream(tblock)",
  "_ toblocks[ _ ]",
  "Transforms a stream of tuples or into a stream of tuple blocks. "
  "The second argument can be either the desired block size (int) or a "
  "template (tblock) for the streams block type.\n\n",
  "query cities feed toblocks[1] blockcount");

ListExpr ToBlocks::TypeMapping(ListExpr args)
{
  const size_t argCount = nl->ListLength(args);

  if (argCount < 2 || argCount > 3)
  {
    return typeError("Expected two or three arguments!");
  }

  //First parameter a stream?
  ListExpr stream = nl->First(nl->First(args));
  if (!isStream(stream))
  {
    return typeError("The first argument (source) isn't a stream!");
  }

  const ListExpr streamType = GetStreamType(stream),
    secondArg = nl->Second(args),
    secondArgType = nl->First(secondArg);

  TBlockTI typeInfo(false);

  //First parameter a stream of 'tuple'?
  if (!Tuple::checkType(streamType))
  {
    return typeError("The first argument (source) isn't a stream of 'tuple'.");
  }

  if (TBlockTI::Check(secondArgType))
  {
    typeInfo = TBlockTI(secondArgType, false);

    if (!nl->Equal(nl->Second(streamType),
                   nl->Second(typeInfo.GetTupleTypeExpr())))
    {
      return typeError("The types or names of the attributes in the first "
                       "argument (source) don't match those in the second "
                       "argument (target template).");
    }
  }
  else
  {
    size_t desiredBlockSize;

    if (!GetSizeTValue(secondArgType, nl->Second(secondArg), desiredBlockSize))
    {
      return typeError("The second argument is neither a 'int' or 'longint' "
                       "(block size) nor a 'tblock' (target template).");
    }

    ListExpr attributeList = nl->Second(streamType),
      columns = nl->OneElemList(nl->Empty()),
      columnsEnd = columns;

    //Create column types of kind ATTRARRAY from attribute types of kind DATA
    while (!nl->IsEmpty(attributeList))
    {
      const ListExpr current = nl->First(attributeList),
        columnName = nl->First(current),
        columnType = AttrArrayTypeConstructor::GetDefaultAttrArrayType(
                      nl->Second(current), false);

      attributeList = nl->Rest(attributeList);


      columnsEnd = nl->Append(columnsEnd,
                              nl->TwoElemList(columnName, columnType));
    }

    typeInfo.AppendColumnInfos(nl->Rest(columns));
    typeInfo.SetDesiredBlockSize(desiredBlockSize);
  }

  if (argCount > 2 && !CcBool::checkType(nl->First(nl->Third(args))))
  {
    return typeError("The third argument (persistable) isn't a bool.");
  }

  //Return 'tblock' type
  return typeInfo.GetTypeExpr(true);
}

ToBlocks::State::State(ArgVector args, Supplier s) :
  m_blockInfo(((TBlock*)qp->ResultStorage(s).addr)->GetInfo()),
  m_stream(args[0]),
  m_blockSize(TBlockTI(qp->GetType(s), false).GetDesiredBlockSize() *
              TBlockTI::blockSizeFactor),
  m_fileId((qp->GetNoSons(s) >= 3 && ((CcBool*)args[2].addr)->GetValue()) ?
           SecondoSystem::GetCatalog()->GetFlobFile()->GetFileId() : 0),
  m_file(m_fileId != 0 ? new SmiRecordFile(false) : nullptr)
{
  m_stream.open();
}

ToBlocks::State::~State()
{
  m_stream.close();
}

TBlock *ToBlocks::State::Request()
{
  Tuple *tuple = m_stream.request();

  if (tuple != nullptr)
  {
    TBlock *block = new TBlock(m_blockInfo, m_fileId, m_fileId, m_file);

    block->Append(*tuple);

    tuple->DeleteIfAllowed();

    while (block->GetSize() < m_blockSize &&
           (tuple = m_stream.request()) != nullptr)
    {
      block->Append(*tuple);

      tuple->DeleteIfAllowed();
    }

    return block;
  }

  return nullptr;
}