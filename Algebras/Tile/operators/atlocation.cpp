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

#include "atlocation.h"
#include "../t/tint.h"
#include "../t/treal.h"
#include "../t/tbool.h"
#include "../t/tstring.h"
#include "../mt/mtint.h"
#include "../mt/mtreal.h"
#include "../mt/mtbool.h"
#include "../mt/mtstring.h"
#include "TemporalAlgebra.h"

namespace TileAlgebra
{

/*
definition of atlocation functions

*/

ValueMapping atlocationFunctions[] =
{
  atlocationFunction<tint, tProperties<int> >,
  atlocationFunction<treal, tProperties<double> >,
  atlocationFunction<tbool, tProperties<char> >,
  atlocationFunction<tstring, tProperties<std::string> >,
  atlocationFunction<mtint, mtProperties<int> >,
  atlocationFunction<mtreal, mtProperties<double> >,
  atlocationFunction<mtbool, mtProperties<char> >,
  atlocationFunction<mtstring, mtProperties<std::string> >,
  0
};

/*
definition of atlocation select function

*/

int atlocationSelectFunction(ListExpr arguments)
{
  int nSelection = -1;

  if(arguments != 0)
  {
    NList argumentsList(arguments);
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

  return nSelection;
}

/*
definition of atlocation type mapping function

*/

ListExpr atlocationTypeMapping(ListExpr arguments)
{
  ListExpr type = NList::typeError("Expecting a t type or a mt type "
                                   "and a point or a point and an instant.");

  NList argumentsList(arguments);

  if(argumentsList.hasLength(2))
  {
    NList argument1 = argumentsList.first();
    NList argument2 = argumentsList.second();

    if(argument1 == NList(tint::BasicType()) &&
       argument2 == NList(Point::BasicType()))
    {
      type = NList(CcInt::BasicType()).listExpr();
    }

    else if(argument1 == NList(treal::BasicType()) &&
            argument2 == NList(Point::BasicType()))
    {
      type = NList(CcReal::BasicType()).listExpr();
    }

    else if(argument1 == NList(tbool::BasicType()) &&
            argument2 == NList(Point::BasicType()))
    {
      type = NList(CcBool::BasicType()).listExpr();
    }

    else if(argument1 == NList(tstring::BasicType()) &&
            argument2 == NList(Point::BasicType()))
    {
      type = NList(CcString::BasicType()).listExpr();
    }

    else if(argument1 == NList(mtint::BasicType()) &&
            argument2 == NList(Point::BasicType()))
    {
      type = NList(MInt::BasicType()).listExpr();
    }

    else if(argument1 == NList(mtreal::BasicType()) &&
            argument2 == NList(Point::BasicType()))
    {
      type = NList(MReal::BasicType()).listExpr();
    }

    else if(argument1 == NList(mtbool::BasicType()) &&
            argument2 == NList(Point::BasicType()))
    {
      type = NList(MBool::BasicType()).listExpr();
    }

    else if(argument1 == NList(mtstring::BasicType()) &&
            argument2 == NList(Point::BasicType()))
    {
      type = NList(MString::BasicType()).listExpr();
    }
  }

  else if(argumentsList.hasLength(3))
  {
    NList argument1 = argumentsList.first();
    NList argument2 = argumentsList.second();
    NList argument3 = argumentsList.third();

    if(argument1 == NList(mtint::BasicType()) &&
       argument2 == NList(Point::BasicType()) &&
       argument2 == NList(Point::BasicType()))
    {
      type = NList(CcInt::BasicType()).listExpr();
    }

    else if(argument1 == NList(mtreal::BasicType()) &&
            argument2 == NList(Point::BasicType()) &&
            argument3 == NList(datetime::DateTime::BasicType()))
    {
      type = NList(CcReal::BasicType()).listExpr();
    }

    else if(argument1 == NList(mtbool::BasicType()) &&
            argument2 == NList(Point::BasicType()) &&
            argument3 == NList(datetime::DateTime::BasicType()))
    {
      type = NList(CcBool::BasicType()).listExpr();
    }

    else if(argument1 == NList(mtstring::BasicType()) &&
            argument2 == NList(Point::BasicType()) &&
            argument3 == NList(datetime::DateTime::BasicType()))
    {
      type = NList(CcString::BasicType()).listExpr();
    }
  }

  return type;
}

}
