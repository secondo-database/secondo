/*
This file is part of SECONDO.

Copyright (C) 2011, University in Hagen, Department of Computer Science,
Database Systems for New Applications.

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

*/

#ifndef TILEALGEBRA_LOAD_H
#define TILEALGEBRA_LOAD_H

#include "AlgebraTypes.h"
#include "Operator.h"
#include "QueryProcessor.h"

namespace TileAlgebra
{

extern ValueMapping loadFunctions[];
int loadSelectFunction(ListExpr args);
ListExpr loadTypeMapping(ListExpr args);

struct loadInfo : OperatorInfo
{
  loadInfo()
  {
    name      = "load";
    signature = "tintArray";
    appendSignature("tintFlob");
    syntax    = "load(_)";
    meaning   = "Loads the values of a tintArray or a tintFlob.";
  }
};

template <typename Type>
int loadFunction(Word* pArguments,
                 Word& rResult,
                 int message,
                 Word& rLocal,
                 Supplier supplier)
{
  Type* pImplementationType = static_cast<Type*>(pArguments[0].addr);

  if(pImplementationType != 0)
  {
    // rResult = qp->ResultStorage(supplier);

    pImplementationType->Load();
  }

  return 0;
}

}

#endif /* #ifndef TILEALGEBRA_LOAD_H */
