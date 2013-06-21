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

#include "minimum.h"
#include "../Types.h"
#include "../t/tint.h"
#include "../t/treal.h"
#include "../t/tbool.h"
#include "../t/tstring.h"
#include "../mt/mtint.h"
#include "../mt/mtreal.h"
#include "../mt/mtbool.h"
#include "../mt/mtstring.h"


namespace TileAlgebra
{

/*
definition of minimum functions

*/

ValueMapping minimumFunctions[] =
{
  minimumFunction<tint, tProperties<int> >,
  minimumFunction<treal, tProperties<double> >,
  minimumFunction<tbool, tProperties<char> >,
  minimumFunction<tstring, tProperties<std::string> >,
  minimumFunction<mtint, mtProperties<int> >,
  minimumFunction<mtreal, mtProperties<double> >,
  minimumFunction<mtbool, mtProperties<char> >,
  minimumFunction<mtstring, mtProperties<std::string> >,
  0
};

/*
definition of minimum select function

*/

int minimumSelectFunction(ListExpr arguments)
{
  int nSelection = -1;

  if(arguments != 0)
  {
    NList argumentsList(arguments);

    if(argumentsList.hasLength(1))
    {
      NList argument1 = argumentsList.first();
      const int TYPE_NAMES = 8;
      const std::string TYPE_NAMES_ARRAY[TYPE_NAMES] =
      {
        tint::BasicType(),
        treal::BasicType(),
        tbool::BasicType(),
        tstring::BasicType(),
        mtint::BasicType(),
        mtreal::BasicType(),
        mtbool::BasicType(),
        mtstring::BasicType()
      };

      for(int i = 0; i < TYPE_NAMES; i++)
      {
        if(argument1.isSymbol(TYPE_NAMES_ARRAY[i]))
        {
          nSelection = i;
          break;
        }
      }
    }
  }

  return nSelection;
}

/*
definition of minimum type mapping function

*/

ListExpr minimumTypeMappingFunction(ListExpr arguments)
{
  ListExpr type = NList::typeError("Expecting a t type or a mt type.");

  NList argumentsList(arguments);

  if(argumentsList.hasLength(1))
  {
    std::string argument1 = argumentsList.first().str();

    if(IstType(argument1) ||
       IsmtType(argument1))
    {
      type = NList(GetValueWrapperType(argument1)).listExpr();
    }
  }

  return type;
}

}
