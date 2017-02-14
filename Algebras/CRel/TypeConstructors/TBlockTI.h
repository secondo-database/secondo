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

#include <cstddef>
#include "NestedList.h"
#include <string>
#include "TBlock.h"

namespace CRelAlgebra
{
  class TBlockTI
  {
  public:
    class AttributeInfo
    {
    public:
      std::string name;

      ListExpr type;
    };

    static bool Check(ListExpr typeExpr, std::string &error);

    std::vector<AttributeInfo> attributeInfos;

    TBlockTI();

    TBlockTI(ListExpr typeExpr);

    void AppendAttributeInfos(ListExpr attributeList);

    ListExpr GetTypeInfo() const;

    TBlock::PInfo GetBlockInfo() const;

  protected:
    static AttributeInfo GetAttributeInfo(ListExpr value);

    static ListExpr GetListExpr(const AttributeInfo &value);

    mutable TBlock::PInfo m_info;
  };
}