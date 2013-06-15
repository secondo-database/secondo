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

#include "t2mt.h"
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
definition of t2mt functions

*/

ValueMapping t2mtFunctions[] =
{
  t2mtFunction<tint, mtProperties<int> >,
  t2mtFunction<treal, mtProperties<double> >,
  t2mtFunction<tbool, mtProperties<char> >,
  t2mtFunction<tstring, mtProperties<string> >,
  0
};

/*
definition of t2mt select function

*/

int t2mtSelectFunction(ListExpr arguments)
{
  int nSelection = -1;

  if(arguments != 0)
  {
    NList argumentsList(arguments);

    if(argumentsList.hasLength(4))
    {
      NList argument1 = argumentsList.first();
      NList argument2 = argumentsList.second();
      NList argument3 = argumentsList.third();
      NList argument4 = argumentsList.fourth();

      if(argument2.isSymbol(Duration::BasicType()) &&
         argument3.isSymbol(Instant::BasicType()) &&
         argument4.isSymbol(Instant::BasicType()))
      {
        const int TYPE_NAMES = 4;
        const std::string TYPE_NAMES_ARRAY[TYPE_NAMES] =
        {
          tint::BasicType(),
          treal::BasicType(),
          tbool::BasicType(),
          tstring::BasicType()
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
definition of t2mt type mapping function

*/

ListExpr t2mtTypeMapping(ListExpr arguments)
{
  ListExpr type = NList::typeError("Expecting a t type, a duration, "
                                   "an instant and an instant.");

  NList argumentsList(arguments);

  if(argumentsList.hasLength(4))
  {
    NList argument1 = argumentsList.first();
    NList argument2 = argumentsList.second();
    NList argument3 = argumentsList.third();
    NList argument4 = argumentsList.fourth();

    if(argument2.isSymbol(Duration::BasicType()) &&
       argument3.isSymbol(Instant::BasicType()) &&
       argument4.isSymbol(Instant::BasicType()))
    {
      if(argument1 == NList(tint::BasicType()))
      {
        type = NList(mtint::BasicType()).listExpr();
      }

      else if(argument1 == NList(treal::BasicType()))
      {
        type = NList(mtreal::BasicType()).listExpr();
      }

      else if(argument1 == NList(tbool::BasicType()))
      {
        type = NList(mtbool::BasicType()).listExpr();
      }

      else if(argument1 == NList(tstring::BasicType()))
      {
        type = NList(mtstring::BasicType()).listExpr();
      }
    }
  }

  return type;
}

}
