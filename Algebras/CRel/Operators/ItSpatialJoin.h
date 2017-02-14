/*
----
This file is part of SECONDO.

Copyright (C) 2004-2009, University in Hagen, Faculty of Mathematics
and Computer Science, Database Systems for New Applications.

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

#pragma once

#include "AlgebraTypes.h"
#include <memory>
#include "MMRTree.h"
#include "NestedList.h"
#include "Operator.h"
#include "Shared.h"
#include "Stream.h"
#include "TBlock.h"
#include <vector>

namespace CRelAlgebra
{
  namespace Operators
  {
    class ItSpatialJoin : public Operator
    {
    public:
      ItSpatialJoin();

    private:
      static const OperatorInfo info;

      static ValueMapping valueMappings[];

      static ListExpr TypeMapping(ListExpr args);

      static int SelectValueMapping(ListExpr args);

      template<int dimA, int dimB>
      class State
      {
      public:
        State(ArgVector args, Supplier s);

        ~State();

        TBlock *Request();

      private:
        static const int minDim = dimA > dimB ? dimB : dimA;

        class MapEntry : public BlockTuple
        {
        public:
          MapEntry(int);

          MapEntry(const BlockTuple &tuple);
        };

        class MapResultIterator
        {
        public:
          MapResultIterator();

          MapResultIterator(
            typename mmrtree::RtreeT<minDim, MapEntry>::iterator* iterator);

          ~MapResultIterator();

          bool IsValid() const;

          bool MoveToNext();

          const MapEntry &GetValue() const;

          MapResultIterator &operator=(
            typename mmrtree::RtreeT<minDim, MapEntry>::iterator* iterator);

        private:
          typename mmrtree::RtreeT<minDim, MapEntry>::iterator* m_iterator;

          const MapEntry *m_current;
        };

        const size_t m_joinIndexA,
          m_joinIndexB,
          m_memLimit,
          m_blockSize;

        mmrtree::RtreeT<minDim, MapEntry> m_map;

        MapResultIterator m_pendingMapResult;

        TBlock *m_blockA;

        std::vector<TBlock*> m_blocksB;

        bool m_isBExhausted;

        TBlock::Iterator m_blockAIterator;

        Stream<TBlock> m_streamA,
          m_streamB;

        TBlock::PInfo m_blockInfo;

        std::unique_ptr<ArrayAttribute[]> m_tuple;

        bool ProceedStreamB();
      };
    };
  }
}