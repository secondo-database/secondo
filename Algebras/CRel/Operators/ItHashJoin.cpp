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

#include <exception>
#include "ListUtils.h"
#include "LogMsg.h"
#include "QueryProcessor.h"
#include <set>
#include "StandardTypes.h"
#include "StreamValueMapping.h"
#include <string>
#include "Symbols.h"
#include "TBlockTI.h"
#include "Utility.h"

using namespace CRelAlgebra;
using namespace CRelAlgebra::Operators;
using namespace listutils;

using std::exception;
using std::set;
using std::string;

extern NestedList *nl;
extern QueryProcessor *qp;

//ItHashJoin--------------------------------------------------------------------

ItHashJoin::ItHashJoin() :
  Operator(info, StreamValueMapping<State>, TypeMapping)
{
  SetUsesArgsInTypeMapping();
}

const OperatorInfo ItHashJoin::info = OperatorInfo(
  "itHashJoin", "crel(c, m, (A)) -> stream(tblock(m, (A)))",
  "_ feed",
  "Produces a stream of tuple-blocks from a column-oriented relation.",
  "query cities feed cconsume");

ListExpr ItHashJoin::TypeMapping(ListExpr args)
{
  if (!nl->HasLength(args, 6))
  {
    return listutils::typeError("Expected five arguments.");
  }

  ListExpr stream = nl->First(nl->First(args));

  if (!nl->HasLength(stream, 2) ||
      !nl->IsEqual(nl->First(stream), Symbols::STREAM()))
  {
    return listutils::typeError("First argument isn't a stream.");
  }

  ListExpr tblock = nl->Second(stream);
  string typeError;

  if (!TBlockTI::Check(tblock, typeError))
  {
    return listutils::typeError("First argument isn't a stream of tblock: " +
                                typeError);
  }

  const TBlockTI blockAInfo(tblock);

  stream = nl->First(nl->Second(args));

  if(!nl->HasLength(stream, 2) ||
     !nl->IsEqual(nl->First(stream), Symbols::STREAM()))
  {
    return listutils::typeError("Second argument isn't a stream.");
  }

  tblock = nl->Second(stream);

  if (!TBlockTI::Check(tblock, typeError))
  {
    return listutils::typeError("Second argument isn't a stream of tblock: " +
                                typeError);
  }

  const TBlockTI blockBInfo(tblock);

  const ListExpr nameAExpr = nl->First(nl->Third(args));
  if (!nl->IsNodeType(SymbolType, nameAExpr))
  {
    return listutils::typeError("Third argument isn't a symbol.");
  }

  const string nameA = nl->SymbolValue(nameAExpr);

  const ListExpr nameBExpr = nl->First(nl->Fourth(args));
  if (!nl->IsNodeType(SymbolType, nameBExpr))
  {
    return listutils::typeError("Fourth argument isn't a symbol.");
  }

  const string nameB = nl->SymbolValue(nameBExpr);

  set<string> attributeNames;

  TBlockTI resultBlockInfo;

  const size_t blockAAttributeCount = blockAInfo.attributeInfos.size();
  size_t nameAIndex = blockAAttributeCount;

  for (size_t i = 0; i < blockAAttributeCount; ++i)
  {
    if (!attributeNames.insert(blockAInfo.attributeInfos[i].name).second)
    {
      return listutils::typeError("Not unique attribute name: " +
                                  blockAInfo.attributeInfos[i].name);
    }

    resultBlockInfo.attributeInfos.push_back(blockAInfo.attributeInfos[i]);

    if (nameAIndex == blockAAttributeCount &&
        nameA == blockAInfo.attributeInfos[i].name)
    {
      nameAIndex = i;
    }
  }

  if (nameAIndex == blockAAttributeCount)
  {
    return listutils::typeError("Third argument doesn't match any attribute "
                                "name in the first argument's tblock.");
  }

  const size_t blockBAttributeCount = blockBInfo.attributeInfos.size();
  size_t nameBIndex = blockBAttributeCount;

  for (size_t i = 0; i < blockBAttributeCount; ++i)
  {
    if (!attributeNames.insert(blockBInfo.attributeInfos[i].name).second)
    {
      return listutils::typeError("Not unique attribute name: " +
                                  blockBInfo.attributeInfos[i].name);
    }

    resultBlockInfo.attributeInfos.push_back(blockBInfo.attributeInfos[i]);

    if (nameBIndex == blockBAttributeCount &&
        nameB == blockBInfo.attributeInfos[i].name)
    {
      nameBIndex = i;
    }
  }

  if (nameBIndex == blockBAttributeCount)
  {
    return listutils::typeError("Fourth argument doesn't match any attribute "
                                "name in the second argument's tblock.");
  }

  const ListExpr memLimitExpr = nl->Fifth(args);

  if (!CcInt::checkType(nl->First(memLimitExpr)))
  {
    return listutils::typeError("Fifth argument (mem limit) isn't an int.");
  }

  const ListExpr blockSizeExpr = nl->Sixth(args);

  if (!CcInt::checkType(nl->First(blockSizeExpr)))
  {
    return listutils::typeError("Sixth argument (block size) isn't an int.");
  }

  return nl->ThreeElemList(nl->SymbolAtom(Symbol::APPEND()),
                           nl->TwoElemList(nl->IntAtom(nameAIndex),
                                           nl->IntAtom(nameBIndex)),
                           nl->TwoElemList(nl->SymbolAtom(Symbol::STREAM()),
                                           resultBlockInfo.GetTypeInfo()));
}

//ItHashJoin::State-------------------------------------------------------------

ItHashJoin::State::State(ArgVector args, Supplier s) :
  m_joinIndexA(((CcInt*)args[6].addr)->GetValue()),
  m_joinIndexB(((CcInt*)args[7].addr)->GetValue()),
  m_memLimit(((CcInt*)args[4].addr)->GetValue()),
  m_blockSize(((CcInt*)args[5].addr)->GetValue()),
  m_map(((CcInt*)args[4].addr)->GetValue()),
  m_blockA(NULL),
  m_isBExhausted(false),
  m_streamA(args[0]),
  m_streamB(args[1])
{
  qp->DeleteResultStorage(s);

  m_blockInfo = TBlockTI(qp->GetType(s)).GetBlockInfo();

  m_tuple = new ArrayAttribute[m_blockInfo->columnCount];

  m_streamA.open();
  m_streamB.open();
}

ItHashJoin::State::~State()
{
  if (m_blockA != NULL)
  {
    m_blockA->DecRef();
  }

  for (TBlock *block : m_blocksB)
  {
    block->DecRef();
  }

  delete[] m_tuple;

  m_streamA.close();
  m_streamB.close();
}

TBlock *ItHashJoin::State::Request()
{
  if (m_blocksB.empty() && !ProceedStreamB())
  {
    return NULL;
  }

  if (m_blockA == NULL)
  {
    if ((m_blockA = m_streamA.request()) == NULL)
    {
      m_streamA.close();
      m_streamA.open();

      if ((m_blockA = m_streamA.request()) == NULL || !ProceedStreamB())
      {
        return NULL;
      }
    }

    m_blockAIterator = m_blockA->GetIterator();

    if (m_blockAIterator.IsValid())
    {
      m_mapResult = m_map.Get(m_blockAIterator.GetAttribute(m_joinIndexA));

      if (m_mapResult.IsValid())
      {
        const size_t columnCount = m_blockA->GetColumnCount();

        for (size_t i = 0; i < columnCount; ++i)
        {
          m_tuple[i] = m_blockAIterator.GetAttribute(i);
        }
      }
    }
  }

  TBlock *block = new TBlock(m_blockInfo, 0, 0);

  const size_t blockAColumnCount = m_blockA->GetColumnCount(),
    blockBColumnCount = m_blocksB[0]->GetColumnCount();

  do
  {
    while (!m_mapResult.IsValid())
    {
      if (!m_blockAIterator.IsValid() || !m_blockAIterator.MoveToNext())
      {
        do
        {
          m_blockA->DecRef();

          if ((m_blockA = m_streamA.request()) == NULL)
          {
            m_streamA.close();
            m_streamA.open();

            if (!ProceedStreamB())
            {
              break;
            }

            if ((m_blockA = m_streamA.request()) == NULL)
            {
              break;
            }
          }

          m_blockAIterator = m_blockA->GetIterator();
        }
        while (!m_blockAIterator.IsValid());
      }

      if (m_blockA == NULL || m_blocksB.empty())
      {
        break;
      }

      m_mapResult = m_map.Get(m_blockAIterator.GetAttribute(m_joinIndexA));

      if (m_mapResult.IsValid())
      {
        for (size_t i = 0; i < blockAColumnCount; ++i)
        {
          m_tuple[i] = m_blockAIterator.GetAttribute(i);
        }

        break;
      }
    }

    if (!m_mapResult.IsValid())
    {
      break;
    }

    const BlockTuple tupleB = m_mapResult.GetValue();

    for (size_t i = 0; i < blockBColumnCount; ++i)
    {
      m_tuple[blockAColumnCount + i] = tupleB[i];
    }

    block->Append(m_tuple);

    m_mapResult.MoveToNext();
  }
  while (block->GetSize() < m_blockSize);

  return block;
}

size_t ItHashJoin::State::HashKey(const ArrayAttribute &attribute)
{
  return attribute.GetHash();
}

int ItHashJoin::State::CompareKey(const ArrayAttribute &a,
                                  const ArrayAttribute &b)
{
  return a.Compare(b);
}

bool ItHashJoin::State::ProceedStreamB()
{
  m_map.Clear();

  for (TBlock *block : m_blocksB)
  {
    block->DecRef();
  }

  m_blocksB.clear();

  if (!m_isBExhausted)
  {
    size_t size = 0;

    while (size < m_memLimit)
    {
      TBlock *block = m_streamB.request();

      if (block == NULL)
      {
        m_isBExhausted = true;
        break;
      }
      else
      {
        m_blocksB.push_back(block);

        for (const BlockTuple &tuple : *block)
        {
          m_map.Add(tuple[m_joinIndexB], tuple);
        }

        size += block->GetSize();
      }
    }
  }

  return !m_blocksB.empty();
}