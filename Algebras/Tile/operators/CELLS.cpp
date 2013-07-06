/*
This file is part of SECONDO.

Copyright (C) 2013, University in Hagen, Department of Computer Science,
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

#include "CELLS.h"

namespace TileAlgebra
{

/*
definition of CELLS type mapping function

*/

ListExpr CELLSTypeMappingFunction(ListExpr arguments)
{
  ListExpr type = NList::typeError("Operator CELLS expects "
                                   "a t type or a mt type.");

  NList argumentsList(arguments);

  if(argumentsList.length() >= 1)
  {
    std::string argument1 = argumentsList.first().str();
    std::string valueWrapperType;

    if(IstType(argument1) ||
       IsmtType(argument1))
    {
      valueWrapperType = GetValueWrapperType(argument1);
    }

    if(valueWrapperType.empty() == false)
    {
      type = NList(NList("rel"),
                   NList(NList("tuple"),
                         NList(NList("Elem"),
                         NList(valueWrapperType)).enclose()
                        )
                  ).listExpr();
    }

    else
    {
      type = NList::typeError("Operator CELLS cannot determine value type of " +
                               argument1 + ".");
    }
  }

  else
  {
    type = NList::typeError("Operator CELLS expects at least one argument.");
  }

  return type;
}

}
