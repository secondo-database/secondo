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

#include <memory>
#include "AttrArray.h"
#include "Array.h"
#include "Ints.h"
#include "DefTimes.h"

namespace ColumnMovingAlgebra
{
  class MObjects : public CRelAlgebra::AttrArray
  {
  public:
    MObjects();
    MObjects(const MObjects &array, 
      const CRelAlgebra::SharedArray<const size_t> &filter);
    virtual ~MObjects() { }

    virtual int  CompareAlmost(size_t rowA, const AttrArray &arrayB, 
      size_t rowB) const;
    virtual bool Equals(size_t rowA, const AttrArray &arrayB, 
      size_t rowB) const;
    virtual bool EqualsAlmost(size_t rowA, const AttrArray &arrayB, 
      size_t rowB) const;

    virtual int  CompareAlmost(size_t row, Attribute &value) const;
    virtual bool Equals(size_t row, Attribute &value) const;
    virtual bool EqualsAlmost(size_t row, Attribute &value) const;

    void present(Instant instant, CRelAlgebra::LongInts & result);
    void present(temporalalgebra::Periods periods, 
      CRelAlgebra::LongInts & result);

  protected:
    std::shared_ptr<DefTimes> m_DefTimes;
  };

  inline MObjects::MObjects() :
    m_DefTimes(std::make_shared<DefTimes>())
  {
  }

  inline MObjects::MObjects(const MObjects &array, 
    const CRelAlgebra::SharedArray<const size_t> &filter) :
    AttrArray(filter),
    m_DefTimes(array.m_DefTimes)
  {
  }


  inline void MObjects::present(Instant instant, CRelAlgebra::LongInts & result)
  {
    result.Clear();
    
    for (auto & iterator : GetFilter()) {
      int i = iterator.GetRow();
      
      if (m_DefTimes->present(i, instant))
        result.Append(i);
    }
  }

  inline void MObjects::present(temporalalgebra::Periods periods, 
    CRelAlgebra::LongInts & result)
  {
    result.Clear();

    for (auto & iterator : GetFilter()) {
      int i = iterator.GetRow();
      
      if (m_DefTimes->present(i, periods))
        result.Append(i);
    }
  }

}
