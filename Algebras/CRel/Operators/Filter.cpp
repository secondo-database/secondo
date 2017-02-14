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

#include "Filter.h"

#include "AttrArray.h"
#include <exception>
#include "IndicesTI.h"
#include "ListUtils.h"
#include "LogMsg.h"
#include <memory>
#include "QueryProcessor.h"
#include "StreamValueMapping.h"
#include <string>
#include "Symbols.h"
#include "TBlockTI.h"

using namespace CRelAlgebra;
using namespace CRelAlgebra::Operators;

using std::exception;
using std::string;
using std::unique_ptr;
using std::vector;

extern NestedList *nl;
extern QueryProcessor *qp;

Filter::Filter() :
  Operator(info, StreamValueMapping<State>, TypeMapping)
{
}

const OperatorInfo Filter::info = OperatorInfo(
  "filter", "nö",
  "_ feed [fun]",
  "schnö",
  "niemals");

ListExpr Filter::TypeMapping(ListExpr args)
{
  const size_t argumentCount = nl->ListLength(args);

  if (argumentCount != 1 && argumentCount != 2)
  {
    return listutils::typeError("Expected one or two arguments!");
  }

  ListExpr stream = nl->First(args);
  if (!nl->HasLength(stream, 2) ||
      !nl->IsEqual(nl->First(stream), Symbol::STREAM()))
  {
    return listutils::typeError("First argument isn't' a stream!");
  }

  const ListExpr tblockType = nl->Second(stream);

  string typeError;
  if (!TBlockTI::Check(tblockType, typeError))
  {
    return listutils::typeError("First argument isn't a stream of tblock: " +
                                typeError);
  }

  const ListExpr mapType = nl->Second(args);
  if(!nl->HasLength(mapType, 3) ||
     !nl->IsEqual(nl->First(mapType), Symbols::MAP()))
  {
    return listutils::typeError("Second argument (map) doesn't take one "
                                "argument.");
  }

  if (!nl->Equal(nl->Second(mapType), tblockType))
  {
    return listutils::typeError("Second argument's (map) argument type doesn't "
                                "match the first argument's (stream) tblock "
                                "type.");
  }

  if(!IndicesTI::Check(nl->Third(mapType), typeError))
  {
    return listutils::typeError("Second argument (map) doesn't return a "
                                "intset: " + typeError);
  }

  return stream;
}

Filter::State::State(Word* args, Supplier s) :
  m_sourceBlock(NULL),
  m_filteredIndices(NULL),
  m_stream(args[0]),
  m_filter(args[1].addr),
  m_filterArg((*qp->Argument(m_filter))[0].addr)
{
  qp->DeleteResultStorage(s);

  m_stream.open();
}

Filter::State::~State()
{
  m_stream.close();

  if (m_sourceBlock != NULL)
  {
    m_sourceBlock->DecRef();
  }
}

TBlock *Filter::State::Request()
{
  while (m_sourceBlock == NULL)
  {
    if ((m_sourceBlock = m_stream.request()) != NULL)
    {
      m_filterArg = m_sourceBlock;

      Word filteredIndices;
      qp->Request(m_filter, filteredIndices);

      if (!(m_filteredIndices = (vector<size_t>*)filteredIndices.addr)->empty())
      {
        m_filteredIndex = 0;
      }
      else
      {
        m_sourceBlock->DecRef();
        m_sourceBlock = NULL;
      }
    }
    else
    {
      return NULL;
    }
  }

  const size_t size = 1000000,
    columnCount = m_sourceBlock->GetColumnCount();

  TBlock *target = new TBlock(m_sourceBlock->GetInfo(), 0, 0);

  unique_ptr<ArrayAttribute[]> tuple(new ArrayAttribute[columnCount]);

  do
  {
    AttrArray *columns[columnCount];

    for (size_t i = 0; i < columnCount; ++i)
    {
      columns[i] = &m_sourceBlock->GetAt(i);
    }

    const size_t filteredCount = m_filteredIndices->size();

    for (size_t i = m_filteredIndex; i < filteredCount; ++i)
    {
      const size_t index = m_filteredIndices->at(i);

      for (size_t j = 0; j < columnCount; ++j)
      {
        tuple[j] = columns[j]->GetAt(index);
      }

      target->Append(tuple.get());

      if (target->GetSize() >= size)
      {
        if (++i == filteredCount)
        {
          m_sourceBlock->DecRef();
          m_sourceBlock = NULL;
        }
        else
        {
          m_filteredIndex = i;
        }

        return target;
      }
    }

    m_sourceBlock->DecRef();

    while ((m_sourceBlock = m_stream.request()) != NULL)
    {
      m_filterArg = m_sourceBlock;

      Word filteredIndices;
      qp->Request(m_filter, filteredIndices);

      if (!(m_filteredIndices = (vector<size_t>*)filteredIndices.addr)->empty())
      {
        m_filteredIndex = 0;
        break;
      }
    }
  }
  while (m_sourceBlock != NULL);

  return target;
}