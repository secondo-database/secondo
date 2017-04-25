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
#include "LongInts.h"
#include "NestedList.h"
#include "Operator.h"
#include "SecondoSMI.h"
#include "Stream.h"
#include "TBlock.h"
#include "TBlockTI.h"
#include <vector>

namespace CRelAlgebra
{
  namespace Operators
  {
    template<bool project>
    class Filter : public Operator
    {
    public:
      Filter();

    private:
      class State
      {
      public:
        State(Supplier stream, Supplier filter, const TBlockTI &blockType,
              size_t *projection);

        ~State();

        TBlock *Request();

      private:
        TBlock *m_sourceBlock;

        LongInts *m_filteredIndices;

        const size_t m_desiredBlockSize;

        size_t m_filteredIndex;

        Stream<TBlock> m_stream;

        Supplier m_filter;

        Address &m_filterArg;

        const PTBlockInfo m_blockInfo;

        const size_t * const m_projection;
      };

      static const OperatorInfo info;

      static ListExpr TypeMapping(ListExpr args);

      static State *CreateState(ArgVector args, Supplier s);
    };
  }
}