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

#include "NestedList.h"
#include <stdint.h>
#include <string>
#include "TypeConstructor.h"

namespace CRelAlgebra
{
  class IterableListExpr
  {
  public:
    IterableListExpr() :
      IterableListExpr(nl->Empty(), nl->Empty())
    {
    }

    IterableListExpr(ListExpr list) :
      IterableListExpr(list, nl->Empty())
    {
    }

    IterableListExpr(ListExpr list, ListExpr end) :
      m_list(list),
      m_end(end)
    {
    }

    bool IsValid() const
    {
      return m_list != m_end;
    }

    ListExpr Get() const
    {
      return m_list;
    }

    bool MoveToNext()
    {
      if (m_list != m_end)
      {
        m_list = nl->Rest(m_list);

        return m_list != m_end;
      }

      return false;
    }

    ListExpr operator * ()
    {
      return m_list;
    }

    IterableListExpr &operator ++ ()
    {
      m_list = nl->Rest(m_list);

      return *this;
    }

    bool operator == (const IterableListExpr &other) const
    {
      return !(*this != other);
    }

    bool operator != (const IterableListExpr &other) const
    {
      if (m_list != m_end)
      {
        if (other.m_list != other.m_end)
        {
          return m_list != other.m_list;
        }

        return true;
      }

      return other.m_list != other.m_end;
    }

    IterableListExpr begin() const
    {
      return *this;
    }

    IterableListExpr end() const
    {
      return IterableListExpr(m_end, m_end);
    }

  private:
    ListExpr m_list,
      m_end;
  };

  template<class T>
  ListExpr ToIntListExpr(const T &source, ListExpr listEnd)
  {
    ListExpr end = listEnd;

    for (long value : source)
    {
      end = nl->Append(end, nl->IntAtom(value));
    }

    return end;
  }

  template<class T>
  ListExpr ToIntListExpr(const T &source)
  {
    ListExpr list = nl->Empty(),
      end = list;

    for (long value : source)
    {
      if (nl->IsEmpty(list))
      {
        list = nl->OneElemList(nl->IntAtom(value));
        end = list;
      }
      else
      {
        end = nl->Append(end, nl->IntAtom(value));
      }
    }

    return list;
  }

  inline void ToIntVector(ListExpr source, std::vector<long> &target)
  {
    for (ListExpr value : IterableListExpr(source))
    {
      target.push_back(nl->IntValue(value));
    }
  }
}