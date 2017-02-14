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

#include "Project.h"

#include "AttrArray.h"
#include "CRelTI.h"
#include <exception>
#include "ListUtils.h"
#include "LogMsg.h"
#include "QueryProcessor.h"
#include "StandardTypes.h"
#include <string>
#include "StreamValueMapping.h"
#include "Symbols.h"
#include "TBlockTI.h"

using namespace CRelAlgebra;
using namespace CRelAlgebra::Operators;

using std::exception;
using std::string;

extern NestedList *nl;
extern QueryProcessor *qp;

Project::Project() :
  Operator(info, StreamValueMapping<State>, TypeMapping)
{
}

const OperatorInfo Project::info = OperatorInfo(
  "project",
  "crel(c, m, (A)) x (a0, ..., ai) -> stream(tblock(m, (a0, ..., ai)))",
  "_ feedproject _",
  "Produces a stream of tuple-blocks from a column-oriented relation projected "
  "on the selected attributes.",
  "query cities feed cconsume");

ListExpr Project::TypeMapping(ListExpr args)
{
  if (!nl->HasLength(args, 2))
  {
    return listutils::typeError("Expected two arguments!");
  }

  const ListExpr stream = nl->First(args);
  if (!nl->HasLength(stream, 2) ||
      !nl->IsEqual(nl->First(stream), Symbol::STREAM()))
  {
    return listutils::typeError("First argument isn't' a stream!");
  }

  const ListExpr blockType = nl->Second(stream);

  string typeError;
  if (!TBlockTI::Check(blockType, typeError))
  {
    return listutils::typeError("First argument isn't a stream of tblock: " +
                                typeError);
  }

  ListExpr attributeNames = nl->Second(args);
  if (nl->IsAtom(attributeNames))
  {
    return listutils::typeError("Second argument isn't a list!");
  }

  if (nl->IsEmpty(attributeNames))
  {
    return listutils::typeError("Second argument is a empty list!");
  }

  TBlockTI blockTypeInfo = TBlockTI(blockType),
    projectedBlockTypeInfo = blockTypeInfo;

  projectedBlockTypeInfo.attributeInfos.clear();

  const size_t columnCount = blockTypeInfo.attributeInfos.size();

  ListExpr attributeIndices = nl->OneElemList(nl->Empty()),
    attributeIndicesBack = attributeIndices;

  while (!nl->IsEmpty(attributeNames))
  {
    const ListExpr attributeNameExpr = nl->First(attributeNames);
    attributeNames = nl->Rest(attributeNames);

    if (!nl->IsNodeType(SymbolType, attributeNameExpr))
    {
      return listutils::typeError("!");
    }

    const string attributeName = nl->SymbolValue(attributeNameExpr);

    size_t attributeIndex = 0;
    for (const TBlockTI::AttributeInfo &attributeInfo :
         blockTypeInfo.attributeInfos)
    {
      if (attributeInfo.name == attributeName)
      {
        projectedBlockTypeInfo.attributeInfos.push_back(attributeInfo);
        break;
      }

      ++attributeIndex;
    }

    if (attributeIndex == columnCount)
    {
      return listutils::typeError("Attribute name '" + attributeName + "' not "
                                  "found!");
    }

    attributeIndicesBack = nl->Append(attributeIndicesBack,
                                      nl->IntAtom(attributeIndex));
  }

  const size_t attributeIndexCount =
    projectedBlockTypeInfo.attributeInfos.size();
  const ListExpr projectedBlockType = projectedBlockTypeInfo.GetTypeInfo();

  return nl->ThreeElemList(nl->SymbolAtom(Symbols::APPEND()),
                           nl->Cons(nl->IntAtom(attributeIndexCount),
                                    nl->Rest(attributeIndices)),
                           nl->TwoElemList(nl->SymbolAtom(Symbol::STREAM()),
                                           projectedBlockType));
}

Project::State::State(ArgVector args, Supplier s) :
  m_stream(args[0]),
  m_fileId(SecondoSystem::GetCatalog()->GetFlobFile()->GetFileId()),
  m_file(new SmiRecordFile(false))
{
  qp->DeleteResultStorage(s);

  const size_t indexCount = ((CcInt*)args[2].addr)->GetValue();

  for (size_t i = 0; i < indexCount; ++i)
  {
    m_attributeIndices.push_back(((CcInt*)args[i + 3].addr)->GetValue());
  }

  m_stream.open();
}

Project::State::~State()
{
  m_stream.close();
}

TBlock *Project::State::Request()
{
  TBlock *block = m_stream.request();

  if (block != NULL)
  {
    TBlock *result = new TBlock(*block, &m_attributeIndices.front(),
                                m_attributeIndices.size());

    block->DecRef();

    return result;
  }

  return NULL;
}