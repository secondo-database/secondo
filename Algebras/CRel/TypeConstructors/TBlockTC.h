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

#include <cstdint>
#include "NestedList.h"
#include "../ReadWrite.h"
#include "SecondoSMI.h"
#include <string>
#include "../TBlock.h"
#include "TypeConstructor.h"

namespace CRelAlgebra
{
  class TBlockTI
  {
  public:
    class ColumnInfo
    {
    public:
      std::string name;

      ListExpr type;
    };

    static const uint64_t blockSizeFactor;

    static bool Check(ListExpr typeExpr);

    static bool Check(ListExpr typeExpr, std::string &error);

    std::vector<ColumnInfo> columnInfos;

    TBlockTI(bool numeric);

    TBlockTI(ListExpr typeExpr, bool numeric);

    bool IsNumeric() const;

    uint64_t GetDesiredBlockSize() const;

    void SetDesiredBlockSize(uint64_t value);

    void AppendColumnInfos(ListExpr columnList);

    ListExpr GetColumnList() const;

    ListExpr GetTypeExpr(bool stream = false) const;

    ListExpr GetTupleTypeExpr(bool stream = false) const;

    const PTBlockInfo &GetBlockInfo() const;

  private:
    bool m_isNumeric;

    uint64_t m_desiredBlockSize;

    mutable PTBlockInfo m_info;
  };

  class TBlockTC : public TypeConstructor
  {
  public:
    static const std::string name;

    static ListExpr TypeProperty();

    static bool CheckType(ListExpr typeInfo, ListExpr &errorInfo);

    static Word In(ListExpr typeInfo, ListExpr value, int errorPos,
                    ListExpr &errorInfo, bool &correct);

    static ListExpr Out(ListExpr typeInfo, Word value);

    static Word Create(const ListExpr typeInfo);

    static void Delete(const ListExpr typeInfo, Word &value);

    static bool Open(SmiRecord &valueRecord, uint64_t &offset,
                    const ListExpr typeInfo, Word &value);

    static bool Save(SmiRecord &valueRecord, uint64_t &offset,
                    const ListExpr typeInfo, Word &value);

    static void Close(const ListExpr typeInfo, Word &value);

    static void *Cast(void *addr);

    static int SizeOf();

    static Word Clone(const ListExpr typeInfo, const Word &w);

    TBlockTC();
  };
}
