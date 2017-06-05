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

#include "ItHashJoin.h"

#include <algorithm>
#include <iostream>
#include "ListExprUtils.h"
#include "LogMsg.h"
#include "OperatorUtils.h"
#include "Project.h"
#include "QueryProcessor.h"
#include <set>
#include "StandardTypes.h"
#include "StreamValueMapping.h"
#include <string>
#include "Symbols.h"
#include "TypeUtils.h"

using namespace CRelAlgebra;
using namespace CRelAlgebra::Operators;

using std::copy;
using std::cout;
using std::set;
using std::string;
using std::vector;

extern NestedList *nl;
extern QueryProcessor *qp;

//ItHashJoin--------------------------------------------------------------------

ItHashJoin::ItHashJoin() :
  Operator(info, valueMappings, SelectValueMapping, TypeMapping)
{
  SetUsesArgsInTypeMapping();
  SetUsesMemory();
}

ValueMapping ItHashJoin::valueMappings[] =
{
  StreamValueMapping<State<false>, CreateState<false>>,
  StreamValueMapping<State<true>, CreateState<true>>,
  nullptr
};

const OperatorInfo ItHashJoin::info = OperatorInfo(
  "itHashJoin",
  "stream(tblock(ma, ((na0, ca0) ... (nai, cai)))) x "
  "stream(tblock(mb, ((nb0, cb0) ... (nbi, cbi)))) x "
  "jna x jnb x (pn0 ... pni) x bc x ml x bs "
  "-> stream(tblock(bs, ((pn0 c0) ... (pni ci)))) with:\n\n"

  "jna / jnb: symbol. the column names from block type a / b to join on.\n\n"

  "(pn0 ... pni): symbol(s). optional. column names to project the result on."
  "omiting -> (na0 ... nai, nb0 ... nbi).\n\n"

  "bc: int. optional. bucket count of the used hash map. omitting -> 1048576."
  "\n\n"

  "ml: int. optional. main memory limit for this operator in MiB. omitting -> "
  "provided by Secondo.\n\n"

  "bs: int. optional. block size. omitting -> ma.",
  "_ _ itHashJoin[_, _, [list], _, _, _]",
  "Executes a iterative hash join algorithm over two streams of tuple blocks."
  "The second stream arument will be used for index creation and should "
  "contain less entries than the first one. Optionally the resulting tuple "
  "blocks can be projected on a selected subset of columns. This avoids "
  "copying workload and is therefore recommended.", "");

ListExpr ItHashJoin::TypeMapping(ListExpr args)
{
  const uint64_t argCount = nl->ListLength(args);

  if (argCount < 4 || argCount > 8)
  {
    return GetTypeError("Expected four to seven arguments.");
  }

  //Check 'stream a' argument

  string error;

  TBlockTI blockAInfo = TBlockTI(false);

  if (!IsBlockStream(nl->First(nl->First(args)), blockAInfo, error))
  {
    return GetTypeError(0, "stream a", error);
  }

  //Check 'stream b' argument

  TBlockTI blockBInfo = TBlockTI(false);

  if (!IsBlockStream(nl->First(nl->Second(args)), blockBInfo, error))
  {
    return GetTypeError(1, "stream b", error);
  }

  //Check 'column-name a'

  string nameA;

  if (!TryGetSymbolValue(nl->First(nl->Third(args)), nameA, error))
  {
    return GetTypeError(2, "column-name a", error);
  }

  uint64_t nameAIndex;

  if (!GetIndexOfColumn(blockAInfo, nameA, nameAIndex))
  {
    return GetTypeError(2, "column-name a",
                        "Colum named '" + nameA + "' not found.");
  }

  //Check 'column-name b'

  string nameB;

  if (!TryGetSymbolValue(nl->First(nl->Fourth(args)), nameB, error))
  {
    return GetTypeError(3, "column-name b", error);
  }

  uint64_t nameBIndex;

  if (!GetIndexOfColumn(blockBInfo, nameB, nameBIndex))
  {
    return GetTypeError(3, "column-name b",
                        "Colum named '" + nameB + "' not found.");
  }

  if (!nl->Equal(blockAInfo.columnInfos[nameAIndex].type,
                 blockBInfo.columnInfos[nameBIndex].type))
  {
    return GetTypeError("The columns to join on have different types.");
  }

  //Initialize the result type from both block-types
  //Check for duplicate column names

  TBlockTI resultBlockInfo = TBlockTI(false);

  set<string> columnNames;

  for (const TBlockTI::ColumnInfo &columnInfo : blockAInfo.columnInfos)
  {
    columnNames.insert(columnInfo.name).second;

    resultBlockInfo.columnInfos.push_back(columnInfo);
  }

  for (const TBlockTI::ColumnInfo &columnInfo : blockBInfo.columnInfos)
  {
    if (!columnNames.insert(columnInfo.name).second)
    {
      return GetTypeError(1, "stream b", "Column name " + columnInfo.name +
                          " allready exists in stream a.");
    }

    resultBlockInfo.columnInfos.push_back(columnInfo);
  }

  //Check optional arguments

  uint64_t argNo = 5;

  ListExpr appendArgs = nl->TwoElemList(nl->IntAtom(nameAIndex),
                                        nl->IntAtom(nameBIndex));

  //Check 'projection' argument

  if (argCount >= argNo)
  {
    Project::Info info(resultBlockInfo, nl->Second(nl->Nth(argNo, args)));

    if (!info.HasError())
    {
      resultBlockInfo = info.GetBlockTypeInfo();

      appendArgs = nl->ThreeElemList(ToIntListExpr(info.GetIndices()),
                                     nl->IntAtom(nameAIndex),
                                     nl->IntAtom(nameBIndex));

      ++argNo;
    }
    else
    {
      error = info.GetError();

      if (argCount == 8)
      {
        return GetTypeError(argNo - 1, "projection", error);
      }
    }
  }

  //Check 'bucket count' argument

  if (argCount >= argNo)
  {
    if (!CcInt::checkType(nl->First(nl->Nth(argNo, args))))
    {
      return GetTypeError(argNo - 1, "projection | bucket count",
                          error + "Not a int.");
    }

    ++argNo;
  }

  //Check 'memory limit' argument

  if (argCount >= argNo)
  {
    if (!CcInt::checkType(nl->First(nl->Nth(argNo, args))))
    {
      return GetTypeError(argNo - 1, "memory limit", "Not a int.");
    }

    ++argNo;
  }

  //Check 'block size' argument

  resultBlockInfo.SetDesiredBlockSize(blockAInfo.GetDesiredBlockSize());

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
      resultBlockInfo.SetDesiredBlockSize(blockSize);
    }

    ++argNo;
  }

  return nl->ThreeElemList(nl->SymbolAtom(Symbol::APPEND()), appendArgs,
                           resultBlockInfo.GetTypeExpr(true));
}

int ItHashJoin::SelectValueMapping(ListExpr args)
{
  if (nl->HasMinLength(args, 5) && !CcInt::checkType(nl->Fifth(args)))
  {
    return 1;
  }

  return 0;
}

template<bool project>
ItHashJoin::State<project> *ItHashJoin::CreateState(ArgVector args, Supplier s)
{
  static const uint64_t defaultBucketCount = 1024 * 1024;

  const uint64_t argCount = qp->GetNoSons(s);

  Supplier streamA = args[0].addr,
    streamB = args[1].addr;

  uint64_t joinIndexA = ((CcInt*)args[argCount - 2].addr)->GetValue(),
    joinIndexB = ((CcInt*)args[argCount - 1].addr)->GetValue();

  const TBlockTI blockTypeInfo = TBlockTI(qp->GetType(s), false);

  uint64_t columnCountA =
    TBlockTI(qp->GetType(streamA), false).columnInfos.size(),
    columnCountB = TBlockTI(qp->GetType(streamB), false).columnInfos.size();

  IndexProjection *projectionsA,
    *projectionsB;

  long bucketCount,
    memLimit;

  if (project)
  {
    bucketCount = argCount > 8 ? ((CcInt*)args[5].addr)->GetValue() : 0;

    memLimit = (argCount > 9 ? ((CcInt*)args[6].addr)->GetValue() : 0) *
               1024 * 1024;

    vector<IndexProjection> tmpProjectionsA,
      tmpProjectionsB;

    uint64_t index = 0;

    for (const Word &subArg : GetSubArgvector(args[argCount - 3].addr))
    {
      const uint64_t projectedIndex = ((CcInt*)subArg.addr)->GetValue();

      if (projectedIndex < columnCountA)
      {
        tmpProjectionsA.push_back(IndexProjection(projectedIndex, index));
      }
      else
      {
        tmpProjectionsB.push_back(IndexProjection(projectedIndex - columnCountA,
                                                  index));
      }

      ++index;
    }

    columnCountA = tmpProjectionsA.size();
    columnCountB = tmpProjectionsB.size();

    projectionsA = new IndexProjection[columnCountA];
    projectionsB = new IndexProjection[columnCountB];

    copy(tmpProjectionsA.begin(), tmpProjectionsA.end(), projectionsA);
    copy(tmpProjectionsB.begin(), tmpProjectionsB.end(), projectionsB);
  }
  else
  {
    bucketCount = argCount > 6 ? ((CcInt*)args[4].addr)->GetValue() : 0;

    memLimit = (argCount > 7 ? ((CcInt*)args[5].addr)->GetValue() : 0) *
               1024 * 1024;

    projectionsA = nullptr;
    projectionsB = nullptr;
  }

  if (bucketCount <= 0)
  {
    bucketCount = defaultBucketCount;
  }

  if (memLimit <= 0)
  {
    memLimit = qp->GetMemorySize(s) * 1024 * 1024;
  }

  return new State<project>(streamA, streamB, joinIndexA, joinIndexB,
                            columnCountA, columnCountB, projectionsA,
                            projectionsB, bucketCount, memLimit, blockTypeInfo);
}

//ItHashJoin::State-------------------------------------------------------------

template<bool project>
ItHashJoin::State<project>::State(Supplier streamA, Supplier streamB,
                                  uint64_t joinIndexA, uint64_t joinIndexB,
                                  uint64_t columnCountA, uint64_t columnCountB,
                                  IndexProjection *projectionsA,
                                  IndexProjection *projectionsB,
                                  uint64_t bucketCount, uint64_t memLimit,
                                  const TBlockTI &blockTypeInfo) :
  m_joinIndexA(joinIndexA),
  m_joinIndexB(joinIndexB),
  m_columnCountA(columnCountA),
  m_columnCountB(columnCountB),
  m_memLimit(memLimit),
  m_blockSize(blockTypeInfo.GetDesiredBlockSize() * TBlockTI::blockSizeFactor),
  m_map(bucketCount),
  m_blockA(nullptr),
  m_isBExhausted(false),
  m_streamA(streamA),
  m_streamB(streamB),
  m_blockInfo(blockTypeInfo.GetBlockInfo()),
  m_tuple(new AttrArrayEntry[blockTypeInfo.columnInfos.size()]),
  m_projectionsA(projectionsA),
  m_projectionsB(projectionsB),
  m_iterations(0)
{
  m_streamA.open();
  m_streamB.open();
}

template<bool project>
ItHashJoin::State<project>::~State()
{
  if (m_blockA != nullptr)
  {
    m_blockA->DecRef();
  }

  for (TBlock *block : m_blocksB)
  {
    block->DecRef();
  }

  delete[] m_tuple;

  if (m_projectionsA != nullptr)
  {
    delete[] m_projectionsA;
  }

  if (m_projectionsB != nullptr)
  {
    delete[] m_projectionsB;
  }

  m_streamA.close();
  m_streamB.close();

  cout << "iterative hash join finished with " << m_iterations
       << " iterations\n";
}

template<bool project>
TBlock *ItHashJoin::State<project>::Request()
{
  if (m_blocksB.empty() && !ProceedStreamB())
  {
    return nullptr;
  }

  const uint64_t columnCountA = m_columnCountA,
    columnCountB = m_columnCountB;

  if (m_blockA == nullptr)
  {
    if ((m_blockA = m_streamA.request()) == nullptr)
    {
      m_streamA.close();
      m_streamA.open();

      if ((m_blockA = m_streamA.request()) == nullptr || !ProceedStreamB())
      {
        return nullptr;
      }
    }

    m_blockAIterator = m_blockA->GetFilteredIterator();

    if (m_blockAIterator.IsValid())
    {
      const TBlockEntry &tuple = m_blockAIterator.Get();

      m_mapResult = m_map.Get(tuple[m_joinIndexA]);

      if (m_mapResult.IsValid())
      {
        for (uint64_t i = 0; i < columnCountA; ++i)
        {
          if (project)
          {
            const IndexProjection &projection = m_projectionsA[i];

            m_tuple[projection.projection] = tuple[projection.index];
          }
          else
          {
            m_tuple[i] = tuple[i];
          }
        }
      }
    }
  }

  TBlock *block = new TBlock(m_blockInfo, 0, 0);

  do
  {
    while (!m_mapResult.IsValid())
    {
      if (!m_blockAIterator.IsValid() || !m_blockAIterator.MoveToNext())
      {
        do
        {
          m_blockA->DecRef();

          if ((m_blockA = m_streamA.request()) == nullptr)
          {
            if (!ProceedStreamB())
            {
              break;
            }

            m_streamA.close();
            m_streamA.open();

            if ((m_blockA = m_streamA.request()) == nullptr)
            {
              break;
            }
          }

          m_blockAIterator = m_blockA->GetFilteredIterator();
        }
        while (!m_blockAIterator.IsValid());
      }

      if (m_blockA == nullptr || m_blocksB.empty())
      {
        break;
      }

      const TBlockEntry &tuple = m_blockAIterator.Get();

      m_mapResult = m_map.Get(tuple[m_joinIndexA]);

      if (m_mapResult.IsValid())
      {
        for (uint64_t i = 0; i < columnCountA; ++i)
        {
          if (project)
          {
            const IndexProjection &projection = m_projectionsA[i];

            m_tuple[projection.projection] = tuple[projection.index];
          }
          else
          {
            m_tuple[i] = tuple[i];
          }
        }

        break;
      }
    }

    if (!m_mapResult.IsValid())
    {
      break;
    }

    const TBlockEntry tupleB = m_mapResult.GetValue();

    for (uint64_t i = 0; i < columnCountB; ++i)
    {
      if (project)
      {
        const IndexProjection &projection = m_projectionsB[i];

        m_tuple[projection.projection] = tupleB[projection.index];
      }
      else
      {
        m_tuple[columnCountA + i] = tupleB[i];
      }
    }

    block->Append(m_tuple);

    m_mapResult.MoveToNext();
  }
  while (block->GetSize() < m_blockSize);

  return block;
}

template<bool project>
uint64_t ItHashJoin::State<project>::HashKey(const AttrArrayEntry &entry)
{
  return entry.GetHash();
}

template<bool project>
bool ItHashJoin::State<project>::CompareKey(const AttrArrayEntry &a,
                                            const AttrArrayEntry &b)
{
  return a == b;
}

template<bool project>
bool ItHashJoin::State<project>::ProceedStreamB()
{
  m_map.Clear();

  for (TBlock *block : m_blocksB)
  {
    block->DecRef();
  }

  m_blocksB.clear();

  if (!m_isBExhausted)
  {
    ++m_iterations;

    uint64_t size = sizeof(ItHashJoin::State<project>),
      lastBlockSize = 0;

    do
    {
      TBlock *block = m_streamB.request();

      if (block == nullptr)
      {
        m_isBExhausted = true;
        break;
      }
      else
      {
        m_blocksB.push_back(block);

        for (const TBlockEntry &tuple : block->GetFilter())
        {
          m_map.Add(tuple[m_joinIndexB], tuple);
        }

        lastBlockSize = block->GetSize();

        size += lastBlockSize;
      }
    }
    while (size + m_map.GetSize() + lastBlockSize < m_memLimit);
  }

  return !m_blocksB.empty();
}