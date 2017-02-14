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
#include "Attribute.h"
#include <cstddef>
#include "NestedList.h"
#include "Operator.h"
#include "RelationAlgebra.h"
#include "Stream.h"

namespace CRelAlgebra
{
  namespace Operators
  {
    class Repeat : public Operator
    {
    public:
      Repeat();

    private:
      static const OperatorInfo info;

      static ValueMapping valueMappings[];

      static ListExpr TypeMapping(ListExpr args);

      static int SelectValueMapping(ListExpr args);

      template<class T>
      static int TValueMapping(ArgVector args, Word &result, int message,
                               Word &local, Supplier s);

      class AttributeState
      {
      public:
        AttributeState(Word *args, Supplier s);

        Attribute *Request();

      private:
        const size_t m_count;

        size_t m_index;

        Attribute &m_attribute;
      };

      class TupleState
      {
      public:
        TupleState(Word *args, Supplier s);

        Tuple *Request();

      private:
        const size_t m_count;

        size_t m_index;

        Tuple &m_tuple;
      };

      template<class T>
      class StreamState
      {
      public:
        StreamState(Word *args, Supplier s);

        ~StreamState();

        T *Request();

      private:
        const size_t m_count;

        size_t m_index;

        Stream<T> m_stream;
      };
    };
  }
}