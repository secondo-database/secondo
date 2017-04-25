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

#include "GAttrArrayTI.h"
#include <cstddef>
#include "NestedList.h"
#include <string>

namespace CRelAlgebra
{
  template <int dim>
  class GSpatialAttrArrayTI
  {
  public:
    static bool Check(ListExpr typeExpr, std::string &error);

    GSpatialAttrArrayTI(bool numeric);
    GSpatialAttrArrayTI(ListExpr typeExpr, bool numeric);

    bool IsNumeric() const;

    ListExpr GetAttributeType() const;

    void SetAttributeType(ListExpr value);

    const PGAttrArrayInfo &GetPInfo() const;

    ListExpr GetTypeExpr() const;

  private:
    GAttrArrayTI m_typeInfo;
  };
}