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
#include <cstdint>
#include "HashMap.h"
#include "NestedList.h"
#include "Operator.h"
#include "SecondoSMI.h"
#include "Algebras/Stream/Stream.h"
#include "../TBlock.h"
#include "../TypeConstructors/TBlockTC.h"
#include <vector>

namespace CRelAlgebra
{
  namespace Operators
  {
    class ItHashJoin : public Operator
    {
    public:
      ItHashJoin();

    private:
      class IndexProjection
      {
      public:
        uint64_t index,
          projection;

        IndexProjection()
        {
        }

        IndexProjection(uint64_t index, uint64_t projection) :
          index(index),
          projection(projection)
        {
        }
      };

      template<bool project>
      class State
      {
      public:
        State(Supplier streamA, Supplier streamB, uint64_t joinIndexA,
              uint64_t joinIndexB, uint64_t columnCountA, uint64_t columnCountB,
              IndexProjection *projectionsA, IndexProjection *projectionsB,
              uint64_t bucketCount, uint64_t memLimit,
              const TBlockTI &blockTypeInfo);

        ~State();

        TBlock *Request();

      private:
        static uint64_t HashKey(const AttrArrayEntry &entry);

        static bool CompareKey(const AttrArrayEntry &a,
                               const AttrArrayEntry &b);

        typedef HashMap<AttrArrayEntry, TBlockEntry, HashKey, CompareKey> Map;

        const uint64_t m_joinIndexA,
          m_joinIndexB,
          m_columnCountA,
          m_columnCountB,
          m_memLimit,
          m_blockSize;

        Map m_map;

        typename Map::EqualRangeIterator m_mapResult;

        TBlock *m_blockA;

        std::vector<TBlock*> m_blocksB;

        bool m_isBExhausted;

        FilteredTBlockIterator m_blockAIterator;

        Stream<TBlock> m_streamA,
          m_streamB;

        const PTBlockInfo m_blockInfo;

        AttrArrayEntry * const m_tuple;

        const IndexProjection * const m_projectionsA,
          * const m_projectionsB;

        uint64_t m_iterations;

        bool ProceedStreamB();
      };

      static const OperatorInfo info;

      static ValueMapping valueMappings[];

      static ListExpr TypeMapping(ListExpr args);

      static int SelectValueMapping(ListExpr args);

      template<bool project>
      static State<project> *CreateState(ArgVector args, Supplier s);
    };
  }
}
