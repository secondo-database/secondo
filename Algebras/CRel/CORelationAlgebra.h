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

#ifndef CORELATIONALGEBRA
#define CORELATIONALGEBRA

#include "Algebra.h"
#include "AlgebraTypes.h"
#include "NestedList.h"
#include "SecondoSMI.h"

class CORelationAlgebra : public Algebra
{
public:
  CORelationAlgebra();

private:
  static ListExpr CRelProp();

  static ListExpr CRelOut(ListExpr typeInfo, Word value);

  static ListExpr SaveCRelToList(ListExpr typeInfo, Word value);

  static Word CreateCRel(const ListExpr typeInfo);

  static Word CRelIn(ListExpr typeInfo, ListExpr value, int errorPos,
                     ListExpr &errorInfo, bool &correct);

  static Word RestoreCRelFromList(ListExpr typeInfo, ListExpr value,
                                  int errorPos, ListExpr &errorInfo,
                                  bool &correct);

  static void DeleteCRel(const ListExpr typeInfo, Word &w);

  static bool CheckCRel(ListExpr type, ListExpr &errorInfo);

  static void *CastCRel(void *addr);

  static void CloseCRel(const ListExpr typeInfo, Word &w);

  static bool OpenCRel(SmiRecord &valueRecord, size_t &offset,
                       const ListExpr typeInfo, Word &value);

  static bool SaveCRel(SmiRecord &valueRecord, size_t &offset,
                       const ListExpr typeInfo, Word &value);

  static int SizeOfCRel();

  static Word CloneCRel(const ListExpr typeInfo, const Word &w);

  static ListExpr TBlockProp();

  static ListExpr TBlockOut(ListExpr typeInfo, Word value);

  static ListExpr SaveTBlockToList(ListExpr typeInfo, Word value);

  static Word CreateTBlock(const ListExpr typeInfo);

  static Word TBlockIn(ListExpr typeInfo, ListExpr value, int errorPos,
                       ListExpr &errorInfo, bool &correct);

  static Word RestoreTBlockFromList(ListExpr typeInfo, ListExpr value,
                                    int errorPos, ListExpr &errorInfo,
                                    bool &correct);

  static void DeleteTBlock(const ListExpr typeInfo, Word &w);

  static bool CheckTBlock(ListExpr type, ListExpr &errorInfo);

  static void *CastTBlock(void *addr);

  static void CloseTBlock(const ListExpr typeInfo, Word &w);

  static bool OpenTBlock(SmiRecord &valueRecord, size_t &offset,
                         const ListExpr typeInfo, Word &value);

  static bool SaveTBlock(SmiRecord &valueRecord, size_t &offset,
                         const ListExpr typeInfo, Word &value);

  static int SizeOfTBlock();

  static Word CloneTBlock(const ListExpr typeInfo, const Word &w);
};

#endif