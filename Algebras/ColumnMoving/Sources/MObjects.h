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

1 MObjects.h

*/

#pragma once

#include <memory>
#include "Algebras/CRel/AttrArray.h"
#include "Array.h"
#include "Algebras/CRel/Ints.h"
#include "DefTimes.h"

namespace ColumnMovingAlgebra
{

/*
1.1 Declaration of ~MObjects~

~MObjects~ is the base class for all moving object attribute arrays.
The term moving objects in this case means all data types, that
are mappings from time to a certain base type so it includes moving real,
moving boolean, moving integer and moving string.

*/
  class MObjects : public CRelAlgebra::AttrArray
  {
  public:

/*
1.1.1 Constructors

*/
    MObjects();
    MObjects(const MObjects &array, 
      const CRelAlgebra::SharedArray<const uint64_t> &filter);
    virtual ~MObjects() { }

/*
1.1.2 Comparing Functions

The crel algebra requires all attribute arrays to implement eight 
different comparison functions. To ease the implementation of our
attribute arrays we provide six of the eight required functions in
this base class. They are generic and call the two left functions,
which still have to be implemented by attribute arrays derived
from this class.

*/
    virtual int  CompareAlmost(size_t rowA, const AttrArray &arrayB, 
      size_t rowB) const;
    virtual bool Equals(size_t rowA, const AttrArray &arrayB, 
      size_t rowB) const;
    virtual bool EqualsAlmost(size_t rowA, const AttrArray &arrayB, 
      size_t rowB) const;

    virtual int  CompareAlmost(size_t row, Attribute &value) const;
    virtual bool Equals(size_t row, Attribute &value) const;
    virtual bool EqualsAlmost(size_t row, Attribute &value) const;

/*
1.1.3 Operators

All moving objects share the operator present which checks whether the 
moving objects exists at a given instant, or is ever
present during a given set of time intervals. So we will implement the
operator present in this base class.

*/
    void present(Instant instant, CRelAlgebra::LongInts & result);
    void present(temporalalgebra::Periods periods, 
      CRelAlgebra::LongInts & result);

  protected:
/*
~mDefTimes~ represents the definition intervals of all moving object entries
in the attribute array.

*/
    std::shared_ptr<DefTimes> m_DefTimes;
  };

/*
1.2 Implementation of inline functions for ~MObjects~

1.2.1 Constructors

*/
  inline MObjects::MObjects() :
    m_DefTimes(std::make_shared<DefTimes>())
  {
  }

  inline MObjects::MObjects(const MObjects &array, 
    const CRelAlgebra::SharedArray<const uint64_t> &filter) :
    AttrArray(filter),
    m_DefTimes(array.m_DefTimes)
  {
  }


/*
1.2.2 Operators

*/
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
