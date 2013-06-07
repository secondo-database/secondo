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

#include "bbox.h"
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
definition of bbox functions

*/

ValueMapping bboxFunctions[] =
{
  bboxFunction<tint, tProperties<int> >,
  bboxFunction<treal, tProperties<double> >,
  bboxFunction<tbool, tProperties<char> >,
  bboxFunction<tstring, tProperties<std::string> >,
  bboxFunction<mtint, mtProperties<int> >,
  bboxFunction<mtreal, mtProperties<double> >,
  bboxFunction<mtbool, mtProperties<char> >,
  bboxFunction<mtstring, mtProperties<std::string> >,
  0
};

/*
definition of bbox select function

*/

int bboxSelectFunction(ListExpr arguments)
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
definition of bbox type mapping function

*/

ListExpr bboxTypeMapping(ListExpr arguments)
{
  ListExpr type = NList::typeError("Expecting a t type or a mt type.");

  NList argumentsList(arguments);
  NList argument1 = argumentsList.first();

  if(argument1 == NList(tint::BasicType()) ||
     argument1 == NList(treal::BasicType()) ||
     argument1 == NList(tbool::BasicType()) ||
     argument1 == NList(tstring::BasicType()))
  {
    type = NList(Rectangle<2>::BasicType()).listExpr();
  }
  
  if(argument1 == NList(mtint::BasicType()) ||
     argument1 == NList(mtreal::BasicType()) ||
     argument1 == NList(mtbool::BasicType()) ||
     argument1 == NList(mtstring::BasicType()))
  {
    type = NList(Rectangle<3>::BasicType()).listExpr();
  }

  return type;
}

}
