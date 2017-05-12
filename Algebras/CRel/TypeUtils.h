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
#include "NestedList.h"
#include <stdint.h>
#include <string>
#include "TypeConstructor.h"

namespace CRelAlgebra
{
  bool ResolveType(const ListExpr typeExpr, int &algebraId, int &typeId);

  bool ResolveType(const ListExpr typeExpr, std::string &name);

  bool ResolveType(const ListExpr typeExpr, std::string &name, int &algebraId,
                  int &typeId);

  void ResolveTypeOrThrow(const ListExpr typeExpr, int &algebraId, int &typeId);

  void ResolveTypeOrThrow(const ListExpr typeExpr, std::string &name);

  void ResolveTypeOrThrow(const ListExpr typeExpr, std::string &name,
                          int &algebraId, int &typeId);

  ListExpr GetStreamType(const ListExpr typeExpr, bool allowNonStream = false);

  ListExpr GetNumericType(const std::string &name);

  TypeConstructor *GetTypeConstructor(const ListExpr typeExpr);

  bool CheckKind(const std::string &kind, const ListExpr typeExpr);

  bool GetSizeTValue(ListExpr typeExpr, ListExpr valueExpr, size_t &value);

  bool GetIntValue(ListExpr typeExpr, ListExpr valueExpr, int32_t &value);

  bool GetLongIntValue(ListExpr typeExpr, ListExpr valueExpr, int64_t &value);

  InObject GetInFunction(const ListExpr typeExpr);

  OutObject GetOutFunction(const ListExpr typeExpr);
}