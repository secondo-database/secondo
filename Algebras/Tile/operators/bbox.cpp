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
definition of template bboxFunction

*/

template <typename Type, typename Properties>
int bboxFunction(Word* pArguments,
                 Word& rResult,
                 int message,
                 Word& rLocal,
                 Supplier supplier)
{
  int nRetVal = 0;

  if(qp != 0 &&
     pArguments != 0)
  {
    Type* pType = static_cast<Type*>(pArguments[0].addr);

    if(pType != 0)
    {
      rResult = qp->ResultStorage(supplier);

      if(rResult.addr != 0)
      {
        typename Properties::RectangleType* pResult =
        static_cast<typename Properties::RectangleType*>(rResult.addr);

        if(pResult != 0)
        {
          pType->bbox(*pResult);
        }
      }
    }
  }

  return nRetVal;
}

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
definition of bbox type mapping function

*/

ListExpr bboxTypeMappingFunction(ListExpr arguments)
{
  ListExpr type = NList::typeError("Operator bbox expects "
                                   "a t type or a mt type.");

  NList argumentsList(arguments);

  if(argumentsList.hasLength(1))
  {
    std::string argument1 = argumentsList.first().str();

    if(IstType(argument1))
    {
      type = NList(Rectangle<2>::BasicType()).listExpr();
    }
    
    else if(IsmtType(argument1))
    {
      type = NList(Rectangle<3>::BasicType()).listExpr();
    }
  }

  return type;
}

}
