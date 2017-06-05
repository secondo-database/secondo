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

#include "ItSpatialJoin.h"

#include <cstdint>
#include <iostream>
#include "ListExprUtils.h"
#include "LogMsg.h"
#include "OperatorUtils.h"
#include "Project.h"
#include "QueryProcessor.h"
#include "RectangleAlgebra.h"
#include <set>
#include "SpatialAttrArray.h"
#include "StandardTypes.h"
#include "StreamValueMapping.h"
#include "StringUtils.h"
#include <string>
#include "Symbols.h"
#include "TypeUtils.h"

using namespace CRelAlgebra;
using namespace CRelAlgebra::Operators;

using std::cout;
using std::set;
using std::string;
using std::vector;
using stringutils::any2str;
using mmrtree::RtreeT;

extern NestedList *nl;
extern QueryProcessor *qp;

//ItSpatialJoin-----------------------------------------------------------------

ItSpatialJoin::ItSpatialJoin() :
  Operator(info, valueMappings, SelectValueMapping, TypeMapping)
{
  SetUsesArgsInTypeMapping();
  SetUsesMemory();
}

const long ItSpatialJoin::defaultNodeMin = 4;

const OperatorInfo ItSpatialJoin::info = OperatorInfo(
  "itSpatialJoin",
  "stream(tblock(ma, ((na0, ca0) ... (nai, cai)))) x "
  "stream(tblock(mb, ((nb0, cb0) ... (nbi, cbi)))) x "
  "jna x jnb x (pn0 ... pni) x nmin x nmax x ml x bs "
  "-> stream(tblock(bs, ((pn0 c0) ... (pni ci)))) with:\n\n"

  "jna / jnb: symbol. the column names from block type a / b to join on. "
  "(jna, c) || (jnb, c) => c is of kind SPATIALATTRARRAY1D ... "
  "SPATIALATTRARRAY8D\n\n"

  "(pn0 ... pni): symbol(s). optional. column names to project the result on."
  "omiting -> (na0 ... nai, nb0 ... nbi).\n\n"

  "nmin: int. optional. min number of entries within the nodes of the used "
  "rtree. omitting -> 4.\n\n"

  "nmin: int. optional. max number of entries within the nodes of the used "
  "rtree. omitting -> 2 * nmin.\n\n"

  "ml: int. optional. main memory limit for this operator in MiB. omitting -> "
  "provided by Secondo.\n\n"

  "bs: int. optional. block size. omitting -> ma",
  "_ _ itSpatialJoin[_, _, [list], _, _, _, _]",
  "Executes a iterative spatial join algorithm over two streams of tuple "
  "blocks using an rtree index. The second stream arument will be used for "
  "index creation and should contain less entries than the first one. "
  "Optionally the resulting tuple blocks can be projected on a selected subset "
  "of columns. This avoids copying workload and is therefore recommended.", "");

ValueMapping ItSpatialJoin::valueMappings[] =
{
  StreamValueMapping<State<1, 1, false>, CreateState<1, 1, false>>,
  StreamValueMapping<State<1, 2, false>, CreateState<1, 2, false>>,
  StreamValueMapping<State<1, 3, false>, CreateState<1, 3, false>>,
  StreamValueMapping<State<1, 4, false>, CreateState<1, 4, false>>,
  StreamValueMapping<State<1, 8, false>, CreateState<1, 8, false>>,

  StreamValueMapping<State<2, 1, false>, CreateState<2, 1, false>>,
  StreamValueMapping<State<2, 2, false>, CreateState<2, 2, false>>,
  StreamValueMapping<State<2, 3, false>, CreateState<2, 3, false>>,
  StreamValueMapping<State<2, 4, false>, CreateState<2, 4, false>>,
  StreamValueMapping<State<2, 8, false>, CreateState<2, 8, false>>,

  StreamValueMapping<State<3, 1, false>, CreateState<3, 1, false>>,
  StreamValueMapping<State<3, 2, false>, CreateState<3, 2, false>>,
  StreamValueMapping<State<3, 3, false>, CreateState<3, 3, false>>,
  StreamValueMapping<State<3, 4, false>, CreateState<3, 4, false>>,
  StreamValueMapping<State<3, 8, false>, CreateState<3, 8, false>>,

  StreamValueMapping<State<4, 1, false>, CreateState<4, 1, false>>,
  StreamValueMapping<State<4, 2, false>, CreateState<4, 2, false>>,
  StreamValueMapping<State<4, 3, false>, CreateState<4, 3, false>>,
  StreamValueMapping<State<4, 4, false>, CreateState<4, 4, false>>,
  StreamValueMapping<State<4, 8, false>, CreateState<4, 8, false>>,

  StreamValueMapping<State<8, 1, false>, CreateState<8, 1, false>>,
  StreamValueMapping<State<8, 2, false>, CreateState<8, 2, false>>,
  StreamValueMapping<State<8, 3, false>, CreateState<8, 3, false>>,
  StreamValueMapping<State<8, 4, false>, CreateState<8, 4, false>>,
  StreamValueMapping<State<8, 8, false>, CreateState<8, 8, false>>,

  StreamValueMapping<State<1, 1, true>, CreateState<1, 1, true>>,
  StreamValueMapping<State<1, 2, true>, CreateState<1, 2, true>>,
  StreamValueMapping<State<1, 3, true>, CreateState<1, 3, true>>,
  StreamValueMapping<State<1, 4, true>, CreateState<1, 4, true>>,
  StreamValueMapping<State<1, 8, true>, CreateState<1, 8, true>>,

  StreamValueMapping<State<2, 1, true>, CreateState<2, 1, true>>,
  StreamValueMapping<State<2, 2, true>, CreateState<2, 2, true>>,
  StreamValueMapping<State<2, 3, true>, CreateState<2, 3, true>>,
  StreamValueMapping<State<2, 4, true>, CreateState<2, 4, true>>,
  StreamValueMapping<State<2, 8, true>, CreateState<2, 8, true>>,

  StreamValueMapping<State<3, 1, true>, CreateState<3, 1, true>>,
  StreamValueMapping<State<3, 2, true>, CreateState<3, 2, true>>,
  StreamValueMapping<State<3, 3, true>, CreateState<3, 3, true>>,
  StreamValueMapping<State<3, 4, true>, CreateState<3, 4, true>>,
  StreamValueMapping<State<3, 8, true>, CreateState<3, 8, true>>,

  StreamValueMapping<State<4, 1, true>, CreateState<4, 1, true>>,
  StreamValueMapping<State<4, 2, true>, CreateState<4, 2, true>>,
  StreamValueMapping<State<4, 3, true>, CreateState<4, 3, true>>,
  StreamValueMapping<State<4, 4, true>, CreateState<4, 4, true>>,
  StreamValueMapping<State<4, 8, true>, CreateState<4, 8, true>>,

  StreamValueMapping<State<8, 1, true>, CreateState<8, 1, true>>,
  StreamValueMapping<State<8, 2, true>, CreateState<8, 2, true>>,
  StreamValueMapping<State<8, 3, true>, CreateState<8, 3, true>>,
  StreamValueMapping<State<8, 4, true>, CreateState<8, 4, true>>,
  StreamValueMapping<State<8, 8, true>, CreateState<8, 8, true>>,
  nullptr
};

ListExpr ItSpatialJoin::TypeMapping(ListExpr args)
{
  const uint64_t argCount = nl->ListLength(args);

  if (argCount < 4 || argCount > 9)
  {
    return GetTypeError("Expected four to nine arguments.");
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

  TypeConstructor *typeConstructorA =
    GetTypeConstructor(blockAInfo.columnInfos[nameAIndex].type);

  if (!typeConstructorA->MemberOf(Kind::SPATIALATTRARRAY1D()) &&
      !typeConstructorA->MemberOf(Kind::SPATIALATTRARRAY2D()) &&
      !typeConstructorA->MemberOf(Kind::SPATIALATTRARRAY3D()) &&
      !typeConstructorA->MemberOf(Kind::SPATIALATTRARRAY4D()) &&
      !typeConstructorA->MemberOf(Kind::SPATIALATTRARRAY8D()))
  {
    return GetTypeError(2, "column-name a",
                        "Colum named '" + nameA + "' is not of kind "
                        "SPATIALATTRARRAY1D, SPATIALATTRARRAY2D, "
                        "SPATIALATTRARRAY3D, SPATIALATTRARRAY4D, or "
                        "SPATIALATTRARRAY8D.");
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

  TypeConstructor *typeConstructorB =
    GetTypeConstructor(blockBInfo.columnInfos[nameBIndex].type);

  if (!typeConstructorB->MemberOf(Kind::SPATIALATTRARRAY1D()) &&
      !typeConstructorA->MemberOf(Kind::SPATIALATTRARRAY2D()) &&
      !typeConstructorA->MemberOf(Kind::SPATIALATTRARRAY3D()) &&
      !typeConstructorA->MemberOf(Kind::SPATIALATTRARRAY4D()) &&
      !typeConstructorA->MemberOf(Kind::SPATIALATTRARRAY8D()))
  {
    return GetTypeError(3, "column-name b",
                        "Colum named '" + nameB + "' is not of kind "
                        "SPATIALATTRARRAY1D, SPATIALATTRARRAY2D, "
                        "SPATIALATTRARRAY3D, SPATIALATTRARRAY4D, or "
                        "SPATIALATTRARRAY8D.");
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

      if (argCount == 9)
      {
        return GetTypeError(argNo - 1, "projection", error);
      }
    }
  }

  //Check 'node min' argument

  if (argCount >= argNo)
  {
    const ListExpr nodeMinExpr = nl->Nth(argNo, args);

    if (!CcInt::checkType(nl->First(nodeMinExpr)))
    {
      return GetTypeError(argNo - 1, "node min", "Not a int.");
    }

    const long nodeMin = nl->IntValue(nl->Second(nodeMinExpr));

    //Check 'node max' argument

    if (argCount >= ++argNo)
    {
      const ListExpr nodeMaxExpr = nl->Nth(argNo, args);

      if (!CcInt::checkType(nl->First(nodeMaxExpr)))
      {
        return GetTypeError(argNo - 1, "node min", "Not a int.");
      }

      const long nodeMax = nl->IntValue(nl->Second(nodeMinExpr));

      if (nodeMax > 0)
      {
        if (nodeMin > 0)
        {
          if (nodeMax < nodeMin)
          {
            return GetTypeError(argNo - 1, "node max", "Less than node min (" +
                                any2str(nodeMin) + ").");
          }
        }
        else
        {
          if (nodeMax < defaultNodeMin)
          {
            return GetTypeError(argNo - 1, "node max", "Less than default node "
                                "min (" + any2str(defaultNodeMin) + ").");
          }
        }
      }

      ++argNo;
    }
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

int ItSpatialJoin::SelectValueMapping(ListExpr args)
{
  const vector<string> kinds = { Kind::SPATIALATTRARRAY1D(),
                                 Kind::SPATIALATTRARRAY2D(),
                                 Kind::SPATIALATTRARRAY3D(),
                                 Kind::SPATIALATTRARRAY4D(),
                                 Kind::SPATIALATTRARRAY8D() };

  int result = nl->HasMinLength(args, 5) && !CcInt::checkType(nl->Fifth(args)) ?
    25 : 0;

  for (char i = 0; i < 2; ++i)
  {
    const TBlockTI blockInfo = TBlockTI(GetStreamType(nl->Nth(1 + i, args)),
                                        false);

    uint64_t nameIndex;

    GetIndexOfColumn(blockInfo, nl->SymbolValue(nl->Nth(3 + i, args)),
                     nameIndex);

    TypeConstructor *typeConstructor =
      GetTypeConstructor(blockInfo.columnInfos[nameIndex].type);

    const uint64_t inc = i == 0 ? 5 : 1;

    for (const string &kind : kinds)
    {
      if (typeConstructor->MemberOf(kind))
      {
        break;
      }

      result += inc;
    }
  }

  return result;
}

template<int dimA, int dimB, bool project>
ItSpatialJoin::State<dimA, dimB, project> *ItSpatialJoin::CreateState(
  ArgVector args, Supplier s)
{
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

  long nodeMin,
    nodeMax,
    memLimit;

  if (project)
  {
    nodeMin = argCount > 8 ? ((CcInt*)args[5].addr)->GetValue() : 0;

    nodeMax = argCount > 9 ? ((CcInt*)args[6].addr)->GetValue() : 0;

    memLimit = (argCount > 10 ? ((CcInt*)args[7].addr)->GetValue() : 0) *
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
    nodeMin = argCount > 6 ? ((CcInt*)args[4].addr)->GetValue() : 0;

    nodeMax = argCount > 7 ? ((CcInt*)args[5].addr)->GetValue() : 0;

    memLimit = (argCount > 8 ? ((CcInt*)args[6].addr)->GetValue() : 0) *
               1024 * 1024;

    projectionsA = nullptr;
    projectionsB = nullptr;
  }

  if (nodeMin <= 0)
  {
    nodeMin = defaultNodeMin;
  }

  if (nodeMax <= 0)
  {
    nodeMax = nodeMin * 2;
  }

  if (memLimit <= 0)
  {
    memLimit = qp->GetMemorySize(s) * 1024 * 1024;
  }

  qp->DeleteResultStorage(s);

  return new State<dimA, dimB, project>(streamA, streamB, joinIndexA,
                                        joinIndexB, columnCountA, columnCountB,
                                        projectionsA, projectionsB, nodeMin,
                                        nodeMax, memLimit, blockTypeInfo);
}

//ItSpatialJoin::State----------------------------------------------------------

template<int dimA, int dimB, bool project>
ItSpatialJoin::State<dimA, dimB, project>::State(
  Supplier streamA, Supplier streamB, uint64_t joinIndexA, uint64_t joinIndexB,
  uint64_t columnCountA, uint64_t columnCountB, IndexProjection *projectionsA,
  IndexProjection *projectionsB, uint64_t nodeMin, uint64_t nodeMax,
  uint64_t memLimit, const TBlockTI &blockTypeInfo) :
  m_joinIndexA(joinIndexA),
  m_joinIndexB(joinIndexB),
  m_columnCountA(columnCountA),
  m_columnCountB(columnCountB),
  m_memLimit(memLimit),
  m_blockSize(blockTypeInfo.GetDesiredBlockSize() * TBlockTI::blockSizeFactor),
  m_map(nodeMin, nodeMax),
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

template<int dimA, int dimB, bool project>
ItSpatialJoin::State<dimA, dimB, project>::~State()
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

  cout << "iterative spatial join finished with " << m_iterations
       << " iterations\n";
}

template<int dimA, int dimB, bool project>
TBlock *ItSpatialJoin::State<dimA, dimB, project>::Request()
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

    m_blockAIterator = m_blockA->GetIterator();

    if (m_blockAIterator.IsValid())
    {
      const TBlockEntry &tuple = m_blockAIterator.Get();

      SpatialAttrArrayEntry<dimA> attribute = tuple[m_joinIndexA];

      Rectangle<dimA> bbox = attribute.GetBoundingBox();

      if (minDim != dimA)
      {
        m_mapResult = m_map.find(bbox. template project<minDim>());
      }
      else
      {
        m_mapResult = m_map.find(*(Rectangle<minDim>*)&bbox);
      }

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

          m_blockAIterator = m_blockA->GetIterator();
        }
        while (!m_blockAIterator.IsValid());
      }

      if (m_blockA == nullptr || m_blocksB.empty())
      {
        break;
      }

      const TBlockEntry &tuple = m_blockAIterator.Get();

      SpatialAttrArrayEntry<dimA> attribute = tuple[m_joinIndexA];

      Rectangle<dimA> bbox = attribute.GetBoundingBox();

      if (minDim != dimA)
      {
        m_mapResult = m_map.find(bbox. template project<minDim>());
      }
      else
      {
        m_mapResult = m_map.find(*(Rectangle<minDim>*)&bbox);
      }

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

    const MapEntry tupleB = m_mapResult.GetValue();

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

template<int dimA, int dimB, bool project>
bool ItSpatialJoin::State<dimA, dimB, project>::ProceedStreamB()
{
  m_map.~RtreeT<minDim, MapEntry>();

  new (&m_map) RtreeT<minDim, MapEntry>(4, 8);

  for (TBlock *block : m_blocksB)
  {
    block->DecRef();
  }

  m_blocksB.clear();

  if (!m_isBExhausted)
  {
    ++m_iterations;

    uint64_t size = sizeof(ItSpatialJoin::State<dimA, dimB, project>),
      rowCount = 0,
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
        size += block->GetSize();
        rowCount += block->GetFilter().GetRowCount();

        m_blocksB.push_back(block);

        for (const TBlockEntry &tuple : block->GetFilter())
        {
          const SpatialAttrArrayEntry<dimB> attribute = tuple[m_joinIndexB];
          Rectangle<dimB> bbox = attribute.GetBoundingBox();

          if (minDim != dimB)
          {
            m_map.insert(bbox. template project<minDim>(), MapEntry(tuple));
          }
          else
          {
            m_map.insert(*(Rectangle<minDim>*)&bbox, MapEntry(tuple));
          }
        }

        lastBlockSize = block->GetSize();

        size += lastBlockSize;
      }
    }
    while (size + m_map.guessSize(rowCount, true) + lastBlockSize < m_memLimit);
  }

  return !m_blocksB.empty();
}

template<int dimA, int dimB, bool project>
ItSpatialJoin::State<dimA, dimB, project>::MapEntry::MapEntry(int)
{
}

template<int dimA, int dimB, bool project>
ItSpatialJoin::State<dimA, dimB, project>::MapEntry::MapEntry(
  const TBlockEntry &tuple) :
  TBlockEntry(tuple)
{
}

template<int dimA, int dimB, bool project>
ItSpatialJoin::State<dimA, dimB, project>::MapResultIterator::
MapResultIterator() :
  m_iterator(nullptr),
  m_current(nullptr)
{
}

template<int dimA, int dimB, bool project>
ItSpatialJoin::State<dimA, dimB, project>::MapResultIterator::
MapResultIterator(
  typename mmrtree::RtreeT<minDim, MapEntry>::iterator* iterator) :
  m_iterator(iterator),
  m_current(iterator->next())
{
}

template<int dimA, int dimB, bool project>
ItSpatialJoin::State<dimA, dimB, project>::MapResultIterator::
~MapResultIterator()
{
  if (m_iterator != nullptr)
  {
    delete m_iterator;
  }
}

template<int dimA, int dimB, bool project>
bool ItSpatialJoin::State<dimA, dimB, project>::MapResultIterator::IsValid()
  const
{
  return m_current != nullptr;
}

template<int dimA, int dimB, bool project>
bool ItSpatialJoin::State<dimA, dimB, project>::MapResultIterator::MoveToNext()
{
  if (m_current != nullptr)
  {
    m_current = m_iterator->next();

    return m_current != nullptr;
  }

  return false;
}

template<int dimA, int dimB, bool project>
const typename ItSpatialJoin::State<dimA, dimB, project>::MapEntry&
ItSpatialJoin::State<dimA, dimB, project>::MapResultIterator::GetValue() const
{
  return *m_current;
}

template<int dimA, int dimB, bool project>
typename ItSpatialJoin::State<dimA, dimB, project>::MapResultIterator &
ItSpatialJoin::State<dimA, dimB, project>::MapResultIterator::operator=(
  typename mmrtree::RtreeT<minDim, MapEntry>::iterator* iterator)
{
  if (m_iterator != nullptr)
  {
    delete m_iterator;
  }

  m_iterator = iterator;

  m_current = m_iterator != nullptr ? m_iterator->next() : nullptr;

  return *this;
}