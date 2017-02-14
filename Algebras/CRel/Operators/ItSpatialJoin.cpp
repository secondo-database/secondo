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

#include <cstddef>
#include <exception>
#include "ListUtils.h"
#include "LogMsg.h"
#include "QueryProcessor.h"
#include "RectangleAlgebra.h"
#include <set>
#include "SpatialAttrArray.h"
#include "StandardTypes.h"
#include "StreamValueMapping.h"
#include <string>
#include "Symbols.h"
#include "TBlockTI.h"

using namespace CRelAlgebra;
using namespace CRelAlgebra::Operators;

using std::exception;
using std::set;
using std::string;
using std::unique_ptr;
using mmrtree::RtreeT;

extern NestedList *nl;
extern QueryProcessor *qp;

//ItSpatialJoin-----------------------------------------------------------------

ItSpatialJoin::ItSpatialJoin() :
  Operator(info, valueMappings, SelectValueMapping, TypeMapping)
{
  SetUsesArgsInTypeMapping();
}

const OperatorInfo ItSpatialJoin::info = OperatorInfo(
  "itSpatialJoin",
  "",
  "",
  "",
  "");

ValueMapping ItSpatialJoin::valueMappings[] =
{
  StreamValueMapping<State<2, 2>>,
  StreamValueMapping<State<2, 3>>,
  StreamValueMapping<State<3, 2>>,
  StreamValueMapping<State<3, 3>>,
  NULL
};

ListExpr ItSpatialJoin::TypeMapping(ListExpr args)
{
  if (!nl->HasLength(args, 8))
  {
    return listutils::typeError("Expected eight arguments.");
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


  ListExpr error;

  //check attributeA type
  const ListExpr typeExprA = blockAInfo.attributeInfos[nameAIndex].type;

  if(!am->CheckKind(Kind::SPATIAL2D(), typeExprA,
                    error = nl->OneElemList(nl->Empty())) &&
     !am->CheckKind(Kind::SPATIAL3D(), typeExprA,
                    error = nl->OneElemList(nl->Empty())))
  {
    return listutils::typeError("Attribute " + nameA + "is not of kind "
                                "Spatial2D or Spatial3D");
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

  //check attributeB type
  const ListExpr typeExprB = blockBInfo.attributeInfos[nameBIndex].type;

  if(!am->CheckKind(Kind::SPATIAL2D(), typeExprB,
                    error = nl->OneElemList(nl->Empty())) &&
     !am->CheckKind(Kind::SPATIAL3D(), typeExprB,
                    error = nl->OneElemList(nl->Empty())))
  {
    return listutils::typeError("Attribute " + nameA + "is not of kind "
                                "Spatial2D or Spatial3D");
  }

  //Check nodeMin argument
  const ListExpr nodeMinExpr = nl->Fifth(args);

  if (!CcInt::checkType(nl->First(nodeMinExpr)))
  {
    return listutils::typeError("Fifth argument (node min) isn't an int.");
  }

  const int nodeMin = nl->IntValue(nl->Second(nodeMinExpr));

  if (nodeMin < 1)
  {
    return listutils::typeError("Fifth argument (node min) isn't >= 1.");
  }

  //Check nodeMax argument

  const ListExpr nodeMaxExpr = nl->Sixth(args);

  if (!CcInt::checkType(nl->First(nodeMaxExpr)))
  {
    return listutils::typeError("Sixth argument (node max) isn't an int.");
  }

  const int nodeMax = nl->IntValue(nl->Second(nodeMaxExpr));

  if (nodeMax < 2 || nodeMax < nodeMin)
  {
    return listutils::typeError("Sixth argument (node max) "
                                "isn't >= 2 && >= node min.");
  }

  //Check memLimit argument

  const ListExpr memLimitExpr = nl->Seventh(args);

  if (!CcInt::checkType(nl->First(memLimitExpr)))
  {
    return listutils::typeError("Seventh argument (mem limit) isn't an int.");
  }

  const ListExpr blockSizeExpr = nl->Eigth(args);

  if (!CcInt::checkType(nl->First(blockSizeExpr)))
  {
    return listutils::typeError("Eigth argument (block size) isn't an int.");
  }

  //---

  return nl->ThreeElemList(nl->SymbolAtom(Symbol::APPEND()),
                           nl->TwoElemList(nl->IntAtom(nameAIndex),
                                           nl->IntAtom(nameBIndex)),
                           nl->TwoElemList(nl->SymbolAtom(Symbol::STREAM()),
                                           resultBlockInfo.GetTypeInfo()));
}

int ItSpatialJoin::SelectValueMapping(ListExpr args)
{
  int result = 0;

  const TBlockTI blockAInfo(nl->Second(nl->First(args))),
    blockBInfo(nl->Second(nl->Second(args)));

  const string nameA = nl->SymbolValue(nl->Third(args)),
    nameB = nl->SymbolValue(nl->Fourth(args));

  const size_t blockAAttributeCount = blockAInfo.attributeInfos.size();
  for (size_t i = 0; i < blockAAttributeCount; ++i)
  {
    if (nameA == blockAInfo.attributeInfos[i].name)
    {
      const ListExpr typeExprA = blockAInfo.attributeInfos[i].type;
      ListExpr error = nl->Empty();

      if(am->CheckKind(Kind::SPATIAL2D(), typeExprA, error))
      {
        result = 0;
      }
      else
      {
        result = 2;
      }

      break;
    }
  }

  const size_t blockBAttributeCount = blockBInfo.attributeInfos.size();
  for (size_t i = 0; i < blockBAttributeCount; ++i)
  {
    if (nameB == blockBInfo.attributeInfos[i].name)
    {
      const ListExpr typeExprB = blockBInfo.attributeInfos[i].type;
      ListExpr error = nl->Empty();

      if(am->CheckKind(Kind::SPATIAL2D(), typeExprB, error))
      {
        return result;
      }
      else
      {
        return result + 1;
      }
    }
  }

  return -1;
}

//ItSpatialJoin::State----------------------------------------------------------

template<int dimA, int dimB>
ItSpatialJoin::State<dimA, dimB>::State(ArgVector args, Supplier s) :
  m_joinIndexA(((CcInt*)args[8].addr)->GetValue()),
  m_joinIndexB(((CcInt*)args[9].addr)->GetValue()),
  m_memLimit(((CcInt*)args[6].addr)->GetValue()),
  m_blockSize(((CcInt*)args[7].addr)->GetValue()),
  m_map(((CcInt*)args[4].addr)->GetValue(), ((CcInt*)args[5].addr)->GetValue()),
  m_blockA(NULL),
  m_isBExhausted(false),
  m_streamA(args[0]),
  m_streamB(args[1])
{
  qp->DeleteResultStorage(s);

  m_blockInfo = TBlockTI(qp->GetType(s)).GetBlockInfo();

  m_tuple.reset(new ArrayAttribute[m_blockInfo->columnCount]);

  m_streamA.open();
  m_streamB.open();
}

template<int dimA, int dimB>
ItSpatialJoin::State<dimA, dimB>::~State()
{
  if (m_blockA != NULL)
  {
    m_blockA->DecRef();
  }

  for (TBlock *block : m_blocksB)
  {
    block->DecRef();
  }

  m_streamA.close();
  m_streamB.close();
}

template<int dimA, int dimB>
TBlock *ItSpatialJoin::State<dimA, dimB>::Request()
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
      SpatialArrayAttribute<dimA> attribute =
        m_blockAIterator.GetAttribute(m_joinIndexA);

      Rectangle<dimA> bbox = attribute.GetBoundingBox();

      if (minDim != dimA)
      {
        m_pendingMapResult = m_map.find(bbox. template project<minDim>());
      }
      else
      {
        m_pendingMapResult = m_map.find(*(Rectangle<minDim>*)&bbox);
      }

      if (m_pendingMapResult.IsValid())
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
    while (!m_pendingMapResult.IsValid())
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

      SpatialArrayAttribute<dimA> attribute =
        m_blockAIterator.GetAttribute(m_joinIndexA);

      Rectangle<dimA> bbox = attribute.GetBoundingBox();

      if (minDim != dimA)
      {
        m_pendingMapResult = m_map.find(bbox. template project<minDim>());
      }
      else
      {
        m_pendingMapResult = m_map.find(*(Rectangle<minDim>*)&bbox);
      }

      if (m_pendingMapResult.IsValid())
      {
        for (size_t i = 0; i < blockAColumnCount; ++i)
        {
          m_tuple[i] = m_blockAIterator.GetAttribute(i);
        }

        break;
      }
    }

    if (!m_pendingMapResult.IsValid())
    {
      break;
    }

    const MapEntry entry = m_pendingMapResult.GetValue();

    for (size_t i = 0; i < blockBColumnCount; ++i)
    {
      m_tuple[blockAColumnCount + i] = entry[i];
    }

    block->Append(m_tuple.get());

    m_pendingMapResult.MoveToNext();
  }
  while (block->GetSize() < m_blockSize);

  return block;
}

template<int dimA, int dimB>
bool ItSpatialJoin::State<dimA, dimB>::ProceedStreamB()
{
  m_map = RtreeT<minDim, MapEntry>(4, 8);

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
          const SpatialArrayAttribute<dimB> attribute = tuple[m_joinIndexB];
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

        size += block->GetSize();
      }
    }
  }

  return !m_blocksB.empty();
}

template<int dimA, int dimB>
ItSpatialJoin::State<dimA, dimB>::MapEntry::MapEntry(int)
{
}

template<int dimA, int dimB>
ItSpatialJoin::State<dimA, dimB>::MapEntry::MapEntry(const BlockTuple &tuple) :
BlockTuple(tuple)
{
}

template<int dimA, int dimB>
ItSpatialJoin::State<dimA, dimB>::MapResultIterator::MapResultIterator() :
  m_iterator(NULL),
  m_current(NULL)
{
}

template<int dimA, int dimB>
ItSpatialJoin::State<dimA, dimB>::MapResultIterator::MapResultIterator(
  typename mmrtree::RtreeT<minDim, MapEntry>::iterator* iterator) :
  m_iterator(iterator),
  m_current(iterator->next())
{
}

template<int dimA, int dimB>
ItSpatialJoin::State<dimA, dimB>::MapResultIterator::~MapResultIterator()
{
  if (m_iterator != NULL)
  {
    delete m_iterator;
  }
}

template<int dimA, int dimB>
bool ItSpatialJoin::State<dimA, dimB>::MapResultIterator::IsValid() const
{
  return m_current != NULL;
}

template<int dimA, int dimB>
bool ItSpatialJoin::State<dimA, dimB>::MapResultIterator::MoveToNext()
{
  if (m_current != NULL)
  {
    m_current = m_iterator->next();

    return m_current != NULL;
  }

  return false;
}

template<int dimA, int dimB>
const typename ItSpatialJoin::State<dimA, dimB>::MapEntry&
ItSpatialJoin::State<dimA, dimB>::MapResultIterator::GetValue() const
{
  return *m_current;
}

template<int dimA, int dimB>
typename ItSpatialJoin::State<dimA, dimB>::MapResultIterator &
ItSpatialJoin::State<dimA, dimB>::MapResultIterator::operator=(
  typename mmrtree::RtreeT<minDim, MapEntry>::iterator* iterator)
{
  if (m_iterator != NULL)
  {
    delete m_iterator;
  }

  m_iterator = iterator;

  m_current = m_iterator != NULL ? m_iterator->next() : NULL;

  return *this;
}