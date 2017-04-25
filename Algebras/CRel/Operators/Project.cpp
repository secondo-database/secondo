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
#include "ListUtils.h"
#include "LogMsg.h"
#include "QueryProcessor.h"
#include <set>
#include "StandardTypes.h"
#include <string>
#include "StringUtils.h"
#include "StreamValueMapping.h"
#include "Symbols.h"
#include "TypeUtils.h"

using namespace CRelAlgebra;
using namespace CRelAlgebra::Operators;

using listutils::isStream;
using std::set;
using std::string;
using std::vector;
using stringutils::any2str;

extern NestedList *nl;
extern QueryProcessor *qp;

Project::Info::Info(const TBlockTI &blockTypeInfo, const ListExpr columnNames) :
  m_blockTypeInfo(blockTypeInfo)
{
  m_blockTypeInfo.columnInfos.clear();

  ListExpr namesExpr = columnNames;

  if (nl->IsAtom(namesExpr))
  {
    m_error = "List of column names isn't a list!";
    return;
  }

  if (nl->IsEmpty(namesExpr))
  {
    m_error = "List of column names is empty!";
    return;
  }

  const size_t columnCount = blockTypeInfo.columnInfos.size();

  ListExpr indices = nl->Empty(),
    indicesBack = indices;

  size_t i = 0;

  set<string> names;

  do
  {
    const ListExpr nameExpr = nl->First(namesExpr);

    if (!nl->IsNodeType(SymbolType, nameExpr))
    {
      m_error = "Column name no. " + any2str(i) + " isn't a symbol.";
      return;
    }

    const string name = nl->SymbolValue(nameExpr);

    if (!names.insert(name).second)
    {
      m_error = "List of column names contains duplicate value '" + name + "'.";
      return;
    }

    size_t index = 0;

    for (const TBlockTI::ColumnInfo &columnInfo : blockTypeInfo.columnInfos)
    {
      if (columnInfo.name == name)
      {
        m_blockTypeInfo.columnInfos.push_back(columnInfo);
        break;
      }

      ++index;
    }

    if (index == columnCount)
    {
      m_error = "A column with the name '" + name + "' doesn't exist.";
      return;
    }

    m_indices.push_back(index);

    if (nl->IsEmpty(indices))
    {
      indices = nl->OneElemList(nl->IntAtom(index));
      indicesBack = indices;
    }
    else
    {
      indicesBack = nl->Append(indicesBack, nl->IntAtom(index));
    }
  }
  while (!nl->IsEmpty(namesExpr = nl->Rest(namesExpr)));
}

bool Project::Info::HasError() const
{
  return !m_error.empty();
}

const string &Project::Info::GetError() const
{
  return m_error;
}

const TBlockTI &Project::Info::GetBlockTypeInfo() const
{
  return m_blockTypeInfo;
}

const vector<size_t> &Project::Info::GetIndices() const
{
  return m_indices;
}

ListExpr Project::Info::GetIndicesExpr() const
{
  const ListExpr indices = nl->OneElemList(nl->IntAtom(m_indices.size()));

  ListExpr indicesEnd = indices;

  for (const size_t &index : m_indices)
  {
    indicesEnd = nl->Append(indicesEnd, nl->IntAtom(index));
  }

  return indices;
}

ListExpr Project::Info::GetIndicesExpr(ListExpr listEnd) const
{
  ListExpr indicesEnd = nl->Append(listEnd, nl->IntAtom(m_indices.size()));

  for (const size_t &index : m_indices)
  {
    indicesEnd = nl->Append(indicesEnd, nl->IntAtom(index));
  }

  return indicesEnd;
}

Project::Project() :
  Operator(info, StreamValueMapping<State>, TypeMapping)
{
}

const OperatorInfo Project::info = OperatorInfo(
  "project", "stream(tblock) x symbol x symbol* -> stream(tblock)",
  "_ project[ list ]",
  "Produces a stream of tuple blocks from a relation projected on the columns "
  "determined by the provided names.",
  "query people feedproject[Name, Age] consume");

ListExpr Project::TypeMapping(ListExpr args)
{
  if (!nl->HasLength(args, 2))
  {
    return listutils::typeError("Expected two arguments!");
  }

  const ListExpr stream = nl->First(args);
  if (!isStream(stream))
  {
    return listutils::typeError("First argument isn't' a stream!");
  }

  const ListExpr blockType = GetStreamType(stream);

  string typeError;
  if (!TBlockTI::Check(blockType, typeError))
  {
    return listutils::typeError("First argument isn't a stream of tblock: " +
                                typeError);
  }

  const Info info = Info(TBlockTI(blockType, false), nl->Second(args));

  if (info.HasError())
  {
    return listutils::typeError("Error in second argument (column names): " +
                                info.GetError());
  }

  const ListExpr projectedBlockType = info.GetBlockTypeInfo().GetTypeExpr();

  return nl->ThreeElemList(nl->SymbolAtom(Symbols::APPEND()),
                           info.GetIndicesExpr(),
                           nl->TwoElemList(nl->SymbolAtom(Symbol::STREAM()),
                                           projectedBlockType));
}

Project::State::State(ArgVector args, Supplier s) :
  m_stream(args[0])
{
  qp->DeleteResultStorage(s);

  const size_t indexCount = ((CcInt*)args[2].addr)->GetValue();

  for (size_t i = 0; i < indexCount; ++i)
  {
    m_indices.push_back(((CcInt*)args[i + 3].addr)->GetValue());
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

  if (block != nullptr)
  {
    TBlock *result = new TBlock(*block, &m_indices.front(), m_indices.size());

    block->DecRef();

    return result;
  }

  return nullptr;
}