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
#include "LongIntsTC.h"
#include "LongIntsTI.h"
#include "ListExprUtils.h"
#include "ListUtils.h"
#include "LogMsg.h"
#include "OperatorUtils.h"
#include "Project.h"
#include "QueryProcessor.h"
#include "Shared.h"
#include "StreamValueMapping.h"
#include <string>
#include "Symbols.h"
#include "TypeUtils.h"

using namespace CRelAlgebra;
using namespace CRelAlgebra::Operators;

using listutils::isStream;
using std::string;
using std::vector;

extern NestedList *nl;
extern QueryProcessor *qp;

//Filter------------------------------------------------------------------------

template<bool project>
Filter<project>::Filter() :
  Operator(info, StreamValueMapping<State, CreateState>,
           TypeMapping)
{
  SetUsesArgsInTypeMapping();
}

template<bool project>
const OperatorInfo Filter<project>::info = project ?
  OperatorInfo("cfilter", "stream(tblock) x (map tblock longints) x symbol x "
               "symbol* x int -> stream(tblock)",
               "_ cfilter[ fun , [ list ], _]",
               "Transforms a stream of tuple blocks into one wich:\n\n"
               "1) Only contains the entries which's indices are returned by "
               "the filter function."
               "\n2) Is projected to the passed column names.\n"
               "3) Has the specified block size if it's > 0 or the input block "
               "streams block size otherwise.",
               "query plz feed filter[.PLZ = 5000, [Ort], 1] consume") :
  OperatorInfo("filter", "stream(tblock) x (map tblock longints) -> "
               "stream(tblock)",
               "_ filter[ fun ]",
               "Transforms a stream of tuple blocks into one wich only "
               "contains the entries which's indices are returned by the "
               "filter function.",
               "query plz feed filter[.PLZ = 5000] consume");

template<bool project>
ListExpr Filter<project>::TypeMapping(ListExpr args)
{
  const size_t argCount = nl->ListLength(args);

  if (project)
  {
    if (argCount < 3 || argCount > 4)
    {
      return listutils::typeError("Expected three to four arguments!");
    }
  }
  else
  {
    if (argCount != 2)
    {
      return listutils::typeError("Expected two to four arguments!");
    }
  }

  //Check 'block stream' argument

  string error;

  ListExpr blockType;

  if (!IsBlockStream(nl->First(nl->First(args)), blockType, error))
  {
    return GetTypeError(0, "block stream", error);
  }

  TBlockTI blockInfo = TBlockTI(blockType, false);

  //Check 'predicate' argument

  const ListExpr mapType = nl->First(nl->Second(args));

  if (!nl->HasMinLength(mapType, 2) ||
      !nl->IsEqual(nl->First(mapType), Symbols::MAP()))
  {
    return GetTypeError(1, "predicate", "Isn't a map.");
  }

  if(!nl->HasLength(mapType, 3) ||
     !nl->Equal(nl->Second(mapType), blockType))
  {
    return GetTypeError(1, "predicate", "Doesn't accept the first argument's "
                        "(block stream) block type");
  }

  if(!LongIntsTI::Check(nl->Third(mapType)))
  {
    return GetTypeError(1, "predicate", "Doesn't evaluate to " +
                        LongIntsTC::name);
  }

  //Check optional arguments

  size_t argNo = 3;
  ListExpr appendArgs = nl->Empty();

  //Check 'projection' argument

  if (argCount >= argNo)
  {
    const Project::Info info(blockInfo, nl->First(nl->Third(args)));

    if (!info.HasError())
    {
      blockInfo = info.GetBlockTypeInfo();

      appendArgs = nl->OneElemList(ToIntListExpr(info.GetIndices()));

      ++argNo;
    }
    else
    {
      error = info.GetError();

      if (argCount == 4)
      {
        return GetTypeError(argNo - 1, "projection", error);
      }
    }
  }

  if (argCount >= argNo)
  {
    const ListExpr arg = nl->Nth(argNo, args);

    if (!CcInt::checkType(nl->First(arg)))
    {
      return GetTypeError(argNo - 1, "block size", "Not a int.");
    }

    const long blockSize = nl->IntValue(nl->Second(arg));

    if (blockSize > 0)
    {
      blockInfo.SetDesiredBlockSize(blockSize);
    }
  }

  const ListExpr result = nl->TwoElemList(nl->SymbolAtom(Symbol::STREAM()),
                                          blockInfo.GetTypeExpr());

  if (nl->IsEmpty(appendArgs))
  {
    return result;
  }

  return nl->ThreeElemList(nl->SymbolAtom(Symbols::APPEND()), appendArgs,
                           result);
}

template<bool project>
typename Filter<project>::State *Filter<project>::CreateState(ArgVector args,
                                                              Supplier s)
{
  qp->DeleteResultStorage(s);

  const TBlockTI blockType = TBlockTI(qp->GetType(s), false);

  size_t *projection;

  if (project)
  {
    const vector<Word> subArgVector =
      GetSubArgvector(args[qp->GetNoSons(s) - 1].addr);

    projection = new size_t[subArgVector.size()];

    size_t i = 0;

    for (const Word &subArg : subArgVector)
    {
      projection[i++] = ((CcInt*)subArg.addr)->GetValue();
    }
  }
  else
  {
    projection = nullptr;
  }

  return new State(args[0].addr, args[1].addr, blockType, projection);
}

//Filter::State-----------------------------------------------------------------

template<bool project>
Filter<project>::State::State(Supplier stream, Supplier filter,
                              const TBlockTI &blockType, size_t *projection) :
  m_sourceBlock(nullptr),
  m_filteredIndices(nullptr),
  m_desiredBlockSize(blockType.GetDesiredBlockSize() *
                     TBlockTI::blockSizeFactor),
  m_filteredIndex(0),
  m_stream(stream),
  m_filter(filter),
  m_filterArg((*qp->Argument(m_filter))[0].addr),
  m_blockInfo(blockType.GetBlockInfo()),
  m_projection(projection)
{
  m_stream.open();
}

template<bool project>
Filter<project>::State::~State()
{
  m_stream.close();

  if (m_sourceBlock != nullptr)
  {
    m_sourceBlock->DecRef();
  }

  if (m_projection != nullptr)
  {
    delete[] m_projection;
  }
}

template<bool project>
TBlock *Filter<project>::State::Request()
{
  while (m_sourceBlock == nullptr)
  {
    if ((m_sourceBlock = m_stream.request()) != nullptr)
    {
      m_filterArg = m_sourceBlock;

      Word filteredIndices;
      qp->Request(m_filter, filteredIndices);

      if ((m_filteredIndices = (LongInts*)filteredIndices.addr)->GetCount() > 0)
      {
        m_filteredIndex = 0;
      }
      else
      {
        m_sourceBlock->DecRef();
        m_sourceBlock = nullptr;
      }
    }
    else
    {
      return nullptr;
    }
  }

  const size_t desiredBlockSize = m_desiredBlockSize,
    columnCount = m_blockInfo->columnCount;

  TBlock *target = new TBlock(m_blockInfo, 0, 0);

  SharedArray<AttrArrayEntry> tuple(columnCount);

  do
  {
    AttrArray *columns[columnCount];

    for (size_t i = 0; i < columnCount; ++i)
    {
      columns[i] = &m_sourceBlock->GetAt(project ? m_projection[i] : i);
    }

    const size_t filteredCount = m_filteredIndices->GetCount();

    for (size_t i = m_filteredIndex; i < filteredCount; ++i)
    {
      const size_t index = m_filteredIndices->GetAt(i).value;

      for (size_t j = 0; j < columnCount; ++j)
      {
        tuple[j] = columns[j]->GetAt(index);
      }

      target->Append(tuple.GetPointer());

      if (target->GetSize() >= desiredBlockSize)
      {
        if (++i == filteredCount)
        {
          m_sourceBlock->DecRef();
          m_sourceBlock = nullptr;
        }
        else
        {
          m_filteredIndex = i;
        }

        return target;
      }
    }

    m_sourceBlock->DecRef();

    while ((m_sourceBlock = m_stream.request()) != nullptr)
    {
      m_filterArg = m_sourceBlock;

      Word filteredIndices;
      qp->Request(m_filter, filteredIndices);

      if ((m_filteredIndices = (LongInts*)filteredIndices.addr)->GetCount() > 0)
      {
        m_filteredIndex = 0;
        break;
      }

      m_sourceBlock->DecRef();
    }
  }
  while (m_sourceBlock != nullptr);

  return target;
}

template class Filter<true>;
template class Filter<false>;