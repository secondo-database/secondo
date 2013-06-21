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

#include "atperiods.h"
#include "../Types.h"
#include "../mt/mtint.h"
#include "../mt/mtreal.h"
#include "../mt/mtbool.h"
#include "../mt/mtstring.h"

namespace TileAlgebra
{

/*
definition of atperiods functions

*/

ValueMapping atperiodsFunctions[] =
{
  atperiodsFunction<mtint>,
  atperiodsFunction<mtreal>,
  atperiodsFunction<mtbool>,
  atperiodsFunction<mtstring>,
  0
};

/*
definition of atperiods select function

*/

int atperiodsSelectFunction(ListExpr arguments)
{
  int nSelection = -1;

  if(arguments != 0)
  {
    NList argumentsList(arguments);

    if(argumentsList.hasLength(2))
    {
      NList argument1 = argumentsList.first();
      NList argument2 = argumentsList.second();

      if(argument2.isSymbol(Periods::BasicType()))
      {
        const int TYPE_NAMES = 4;
        const std::string TYPE_NAMES_ARRAY[TYPE_NAMES] =
        {
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
  }

  return nSelection;
}

/*
definition of atperiods type mapping function

*/

ListExpr atperiodsTypeMappingFunction(ListExpr arguments)
{
  ListExpr type = NList::typeError("Expecting a mt type and a periods.");

  NList argumentsList(arguments);

  if(argumentsList.hasLength(2))
  {
    std::string argument1 = argumentsList.first().str();
    std::string argument2 = argumentsList.second().str();

    if(IsmtType(argument1) &&
       argument2 == Periods::BasicType())
    {
      type = NList(argument1).listExpr();
    }
  }

  return type;
}

}
