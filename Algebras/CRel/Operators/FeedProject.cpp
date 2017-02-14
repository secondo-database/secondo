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

#include "FeedProject.h"

#include "AttrArray.h"
#include "CRelTI.h"
#include <exception>
#include "ListUtils.h"
#include "LogMsg.h"
#include "QueryProcessor.h"
#include "StandardTypes.h"
#include "StreamValueMapping.h"
#include <string>
#include "Symbols.h"
#include "TBlockTI.h"

using namespace CRelAlgebra;
using namespace CRelAlgebra::Operators;

using std::exception;
using std::string;

extern NestedList *nl;
extern QueryProcessor *qp;

FeedProject::FeedProject() :
  Operator(info, StreamValueMapping<State>, TypeMapping)
{
}

const OperatorInfo FeedProject::info = OperatorInfo(
  "feedproject",
  "crel(c, m, (A)) x (a0, ..., ai) -> stream(tblock(m, (a0, ..., ai)))",
  "_ feedproject _",
  "Produces a stream of tuple-blocks from a column-oriented relation projected "
  "on the selected attributes.",
  "query cities feed cconsume");

ListExpr FeedProject::TypeMapping(ListExpr args)
{
  if (!nl->HasLength(args, 2))
  {
    return listutils::typeError("Expected two arguments!");
  }

  const ListExpr crelType = nl->First(args);
  string crelTypeError;

  if (!CRelTI::Check(crelType, crelTypeError))
  {
    return listutils::typeError(crelTypeError);
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

  const CRelTI crelInfo(crelType);

  TBlockTI tblockInfo(crelInfo);
  tblockInfo.attributeInfos.clear();

  const size_t columnCount = crelInfo.attributeInfos.size();

  ListExpr attributeIndices = nl->OneElemList(nl->Empty()),
    attributeIndicesBack = attributeIndices;

  size_t attributeIndexCount = 0;

  do
  {
    const ListExpr attributeNameExpr = Take(attributeNames);

    if (!nl->IsNodeType(SymbolType, attributeNameExpr))
    {
      return listutils::typeError("!");
    }

    const string attributeName = nl->SymbolValue(attributeNameExpr);

    size_t attributeIndex = 0;
    do
    {
      if (crelInfo.attributeInfos[attributeIndex].name == attributeName)
      {
        tblockInfo.attributeInfos.push_back(
          crelInfo.attributeInfos[attributeIndex]);
        break;
      }
    }
    while (++attributeIndex < columnCount);

    if (attributeIndex == columnCount)
    {
      return listutils::typeError("Attribute name '" + attributeName + "' not "
                                  "found!");
    }

    Append(attributeIndicesBack, nl->IntAtom(attributeIndex));

    ++attributeIndexCount;
  }
  while (!nl->IsEmpty(attributeNames));

  return nl->ThreeElemList(nl->SymbolAtom(Symbols::APPEND()),
                            nl->Cons(nl->IntAtom(attributeIndexCount),
                                    nl->Rest(attributeIndices)),
                            nl->TwoElemList(nl->SymbolAtom(Symbol::STREAM()),
                                            tblockInfo.GetTypeInfo()));
}

FeedProject::State::State(ArgVector args, Supplier s) :
  m_relation(*(CRel*)args[0].addr),
  m_blockIndex(0),
  m_fileId(SecondoSystem::GetCatalog()->GetFlobFile()->GetFileId()),
  m_file(new SmiRecordFile(false))
{
  qp->DeleteResultStorage(s);

  const size_t indexCount = ((CcInt*)args[2].addr)->GetValue();

  for (size_t i = 0; i < indexCount; ++i)
  {
    m_attributeIndices.push_back(((CcInt*)args[i + 3].addr)->GetValue());
  }
}

TBlock *FeedProject::State::Request()
{
  if (m_blockIndex < m_relation.GetBlockCount())
  {
    TBlock &block = m_relation.GetBlock(m_blockIndex++);

    return new TBlock(block, &m_attributeIndices.front(),
                      m_attributeIndices.size());
  }

  return NULL;
}