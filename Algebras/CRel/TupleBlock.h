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

#ifndef TUPLEBLOCK
#define TUPLEBLOCK

#include "Algebra.h"
#include "AlgebraTypes.h"
#include "Attribute.h"
#include "NestedList.h"
#include "SecondoSMI.h"
#include <string>
#include "TupleBlockType.h"

class TupleBlock
{
public:
  static TypeConstructor& GetTypeConstructor();

  static std::string GetBasicType();

  ~TupleBlock();

private:
  static TypeConstructor typeConstructor;

  static ListExpr Prop();
  static bool Check(ListExpr type, ListExpr &errorInfo);
  static Word In(ListExpr typeInfo, ListExpr value, int errorPos,
                       ListExpr &errorInfo, bool &correct);
  static ListExpr Out(ListExpr typeInfo, Word value);
  static ListExpr SaveToList(ListExpr typeInfo, Word value);
  static Word RestoreFromList(ListExpr typeInfo, ListExpr value,
                                    int errorPos, ListExpr &errorInfo,
                                    bool &correct);
  static Word Create(const ListExpr typeInfo);
  static Word Clone(const ListExpr typeInfo, const Word &w);
  static bool Save(SmiRecord &valueRecord, size_t &offset,
                         const ListExpr typeInfo, Word &value);
  static void Delete(const ListExpr typeInfo, Word &w);
  static bool Open(SmiRecord &valueRecord, size_t &offset,
                         const ListExpr typeInfo, Word &value);
  static void Close(const ListExpr typeInfo, Word &w);
  static void *Cast(void *addr);
  static int SizeOf();

  PTupleBlockType m_type;

  Address** m_attributes;

  TupleBlock();
};

#endif