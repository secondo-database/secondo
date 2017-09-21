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

1 MObjects.cpp

*/

#include "AlmostEqual.h"
#include "MObjects.h"

namespace ColumnMovingAlgebra
{
/*
1.1 Implementation of virtual functions for ~MObjects~

The crel algebra requires all attribute arrays to implement eight 
different comparison functions. To ease the implementation of our
attribute arrays we provide six of the eight required functions in
this base class. They are generic and call the two left functions,
which still have to be implemented by attribute arrays derived
from this class.

*/
  int MObjects::CompareAlmost(size_t rowA, const CRelAlgebra::AttrArray 
    &arrayB, size_t rowB) const
  {
    return Compare(rowA, arrayB, rowB);
  }

  int MObjects::CompareAlmost(size_t row, Attribute &value) const
  {
    return Compare(row, value);
  }

  bool MObjects::Equals(size_t rowA, const CRelAlgebra::AttrArray &arrayB,
    size_t rowB) const
  {
    return Compare(rowA, arrayB, rowB) == 0;
  }

  bool MObjects::Equals(size_t row, Attribute &value) const
  {
    return Compare(row, value) == 0;
  }

  bool MObjects::EqualsAlmost(size_t rowA, 
    const CRelAlgebra::AttrArray &arrayB, size_t rowB) const
  {
    return Compare(rowA, arrayB, rowB) == 0;
  }

  bool MObjects::EqualsAlmost(size_t row, Attribute &value) const
  {
    return Compare(row, value) == 0;
  }

}
