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

#include "atinstant.h"
#include "../mt/mtint.h"
#include "../mt/mtreal.h"
#include "../mt/mtbool.h"
#include "../mt/mtstring.h"
#include "../it/itint.h"
#include "../it/itreal.h"
#include "../it/itbool.h"
#include "../it/itstring.h"

namespace TileAlgebra
{

/*
definition of atinstant functions

*/

ValueMapping atinstantFunctions[] =
{
  atinstantFunction<mtint, mtProperties<int> >,
  atinstantFunction<mtreal, mtProperties<double> >,
  atinstantFunction<mtbool, mtProperties<char> >,
  atinstantFunction<mtstring, mtProperties<std::string> >,
  0
};

/*
definition of atinstant select function

*/

int atinstantSelectFunction(ListExpr arguments)
{
  int nSelection = -1;

  if(arguments != 0)
  {
    NList argumentsList(arguments);

    if(argumentsList.hasLength(2))
    {
      NList argument1 = argumentsList.first();
      NList argument2 = argumentsList.second();

      if(argument2.isSymbol(Instant::BasicType()))
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
definition of atinstant type mapping function

*/

ListExpr atinstantTypeMapping(ListExpr arguments)
{
  ListExpr type = NList::typeError("Expecting a mt type and an instant.");

  NList argumentsList(arguments);

  if(argumentsList.hasLength(2))
  {
    NList argument1 = argumentsList.first();
    NList argument2 = argumentsList.second();

    if(argument2 == NList(Instant::BasicType()))
    {
      if(argument1 == NList(mtint::BasicType()))
      {
        type = NList(itint::BasicType()).listExpr();
      }

      else if(argument1 == NList(mtreal::BasicType()))
      {
        type = NList(itreal::BasicType()).listExpr();
      }

      else if(argument1 == NList(mtbool::BasicType()))
      {
        type = NList(itbool::BasicType()).listExpr();
      }

      else if(argument1 == NList(mtstring::BasicType()))
      {
        type = NList(itstring::BasicType()).listExpr();
      }
    }
  }

  return type;
}

}
