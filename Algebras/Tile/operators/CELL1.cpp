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

/*
TileAlgebra includes

*/

#include "CELL1.h"

/*
declaration of namespace TileAlgebra

*/

namespace TileAlgebra
{

/*
Method CELL1TypeMappingFunction returns the return value type
of CELL1 operator in the form of a ListExpr.

author: Dirk Zacher
parameters: arguments - arguments of CELL1 operator
return value: return value type of CELL1 operator
exceptions: -

*/

ListExpr CELL1TypeMappingFunction(ListExpr arguments)
{
  ListExpr type = NList::typeError("Operator CELL1 expects "
                                   "a t type or a mt type.");

  NList argumentsList(arguments);

  if(argumentsList.hasLength(1))
  {
    std::string argument1 = argumentsList.first().str();
    
    if(IstType(argument1) ||
       IsmtType(argument1))
    {
      std::string valueWrapperType = GetValueWrapperType(argument1);
      
      if(valueWrapperType.empty() == false)
      {
        type = NList(valueWrapperType).listExpr();
      }
    }
  }

  return type;
}

}
