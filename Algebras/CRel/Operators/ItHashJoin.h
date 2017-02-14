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
#include <cstddef>
#include "HashMap.h"
#include "NestedList.h"
#include "Operator.h"
#include "SecondoSMI.h"
#include "Stream.h"
#include "TBlock.h"
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
      class State
      {
      public:
        State(ArgVector args, Supplier s);

        ~State();

        TBlock *Request();

      private:
        static size_t HashKey(const ArrayAttribute &attribute);

        static int CompareKey(const ArrayAttribute &a, const ArrayAttribute &b);

        typedef HashMap<ArrayAttribute, BlockTuple, HashKey, CompareKey> Map;

        const size_t m_joinIndexA,
          m_joinIndexB,
          m_memLimit,
          m_blockSize;

        Map m_map;

        Map::EqualRangeIterator m_mapResult;

        TBlock *m_blockA;

        std::vector<TBlock*> m_blocksB;

        bool m_isBExhausted;

        TBlock::Iterator m_blockAIterator;

        Stream<TBlock> m_streamA,
          m_streamB;

        TBlock::PInfo m_blockInfo;

        ArrayAttribute *m_tuple;

        bool ProceedStreamB();
      };

      static const OperatorInfo info;

      static ListExpr TypeMapping(ListExpr args);
    };
  }
}