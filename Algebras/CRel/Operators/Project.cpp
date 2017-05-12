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
#include <exception>
#include "ListUtils.h"
#include "LogMsg.h"
#include "OperatorUtils.h"
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
using std::exception;
using std::set;
using std::string;
using std::vector;
using stringutils::any2str;

extern NestedList *nl;
extern QueryProcessor *qp;

//Project::Info-----------------------------------------------------------------

Project::Info::Info(const TBlockTI &blockTypeInfo, const ListExpr columnNames,
                    bool invert) :
  m_blockTypeInfo(blockTypeInfo)
{
  m_blockTypeInfo.columnInfos.clear();

  ListExpr namesExpr = columnNames;

  if (nl->IsAtom(namesExpr))
  {
    m_error = "List of column names isn't a list!";
    return;
  }

  const size_t columnCount = blockTypeInfo.columnInfos.size();

  size_t i = 0;

  set<string> names;

  while (!nl->IsEmpty(namesExpr))
  {
    const ListExpr nameExpr = nl->First(namesExpr);

    namesExpr = nl->Rest(namesExpr);

    if (!nl->IsNodeType(SymbolType, nameExpr))
    {
      m_error = "Column name no. " + any2str(i) + " isn't a symbol.";
      break;
    }

    const string name = nl->SymbolValue(nameExpr);

    if (!names.insert(name).second)
    {
      m_error = "List of column names contains duplicate value '" + name + "'.";
      break;
    }

    size_t index = 0;

    for (const TBlockTI::ColumnInfo &columnInfo : blockTypeInfo.columnInfos)
    {
      if (columnInfo.name == name)
      {
        break;
      }

      ++index;
    }

    if (index == columnCount)
    {
      m_error = "A column with the name '" + name + "' doesn't exist.";
      break;
    }

    m_indices.push_back(index);
  }

  if (invert)
  {
    vector<size_t> indices;

    i = 0;

    for (const size_t excludedIndex : m_indices)
    {
      while (i < excludedIndex)
      {
        indices.push_back(i++);
      }

      ++i;
    }

    while (i < columnCount)
    {
      indices.push_back(i++);
    }

    m_indices = indices;
  }

  if (m_indices.empty())
  {
    m_error = "Resulting column list is empty!";
  }

  for (const size_t index : m_indices)
  {
    m_blockTypeInfo.columnInfos.push_back(blockTypeInfo.columnInfos[index]);
  }
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

//Project-----------------------------------------------------------------------

Project::Project() :
  Operator(info, valueMappings, SelectValueMapping, TypeMapping)
{
}

ValueMapping Project::valueMappings[] =
{
  StreamValueMapping<State>,
  TBlockValueMapping,
  nullptr
};

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
    return GetTypeError("Expected two arguments.");
  }

  const ListExpr blockType = nl->First(args);

  if (!TBlockTI::Check(blockType))
  {
    return GetTypeError(0, "Isn't a (stream of) tblock.");
  }

  const Info info = Info(TBlockTI(blockType, false), nl->Second(args));

  if (info.HasError())
  {
    return GetTypeError(1, "column names", info.GetError());
  }

  const ListExpr result =
    info.GetBlockTypeInfo().GetTypeExpr(isStream(blockType));

  return nl->ThreeElemList(nl->SymbolAtom(Symbols::APPEND()),
                           info.GetIndicesExpr(), result);
}

int Project::SelectValueMapping(ListExpr args)
{
  return isStream(nl->First(args)) ? 0 : 1;
}

int Project::TBlockValueMapping(ArgVector args, Word &result, int, Word&,
                                Supplier s)
{
  try
  {
    //Don't want to return a new instance so delete it
    qp->DeleteResultStorage(s);

    const TBlock &block = *(TBlock*)args[0].addr;

    const size_t indexCount = ((CcInt*)args[2].addr)->GetValue();

    size_t indices[indexCount];

    for (size_t i = 0; i < indexCount; ++i)
    {
      indices[i] = ((CcInt*)args[i + 3].addr)->GetValue();
    }

    TBlock *projection = new TBlock(block, indices, indexCount);

    //Return the pointer to the column
    //Let the delete function only decrease the ref-count
    qp->ChangeResultStorage(s, result = Word(projection));
    qp->SetDeleteFunction(s, [](const ListExpr, Word &value)
    {
      ((TBlock*)value.addr)->DecRef();
    });

    return 0;
  }
  catch (const exception &e)
  {
    ErrorReporter::ReportError(e.what());
  }

  return FAILURE;
}

//Project::State----------------------------------------------------------------

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