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
#include "TBlockTI.h"

namespace CRelAlgebra
{
  class CRelTI : public TBlockTI
  {
  public:
    static bool Check(ListExpr typeExpr, std::string &error);

    CRelTI();

    CRelTI(const TBlockTI &info, size_t cacheSize, size_t blockSize);

    CRelTI(ListExpr typeExpr);

    size_t GetCacheSize() const;

    void SetCacheSize(size_t value);

    size_t GetDesiredBlockSize() const;

    void SetDesiredBlockSize(size_t value);

    ListExpr GetTypeInfo() const;

  private:
    size_t m_cacheSize,
      m_desiredBlockSize;
  };
}