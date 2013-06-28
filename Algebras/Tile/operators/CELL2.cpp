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

#include "CELL2.h"

namespace TileAlgebra
{

/*
definition of CELL2 type mapping function

*/

ListExpr CELL2TypeMappingFunction(ListExpr arguments)
{
  ListExpr type = NList::typeError("Operator CELL2 expects "
                                   "two t types or mt types.");

  NList argumentsList(arguments);

  if(argumentsList.hasLength(2))
  {
    std::string argument2 = argumentsList.second().str();

    if(IstType(argument2) ||
       IsmtType(argument2))
    {
      type = NList(GetValueWrapperType(argument2)).listExpr();
    }
  }

  return type;
}

}
