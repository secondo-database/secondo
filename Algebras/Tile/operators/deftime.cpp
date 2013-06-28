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

#include "deftime.h"
#include "../mt/mtint.h"
#include "../mt/mtreal.h"
#include "../mt/mtbool.h"
#include "../mt/mtstring.h"

namespace TileAlgebra
{

/*
definition of template deftimeFunction

*/

template <typename Type>
int deftimeFunction(Word* pArguments,
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
        Periods* pResult = static_cast<Periods*>(rResult.addr);

        if(pResult != 0)
        {
          if(pType->IsDefined())
          {
            pType->deftime(*pResult);
          }

          else
          {
            pResult->SetDefined(false);
          }
        }
      }
    }
  }

  return nRetVal;
}

/*
definition of deftime functions

*/

ValueMapping deftimeFunctions[] =
{
  deftimeFunction<mtint>,
  deftimeFunction<mtreal>,
  deftimeFunction<mtbool>,
  deftimeFunction<mtstring>,
  0
};

/*
definition of deftime select function

*/

int deftimeSelectFunction(ListExpr arguments)
{
  int nSelection = -1;

  if(arguments != 0)
  {
    NList argumentsList(arguments);

    if(argumentsList.hasLength(1))
    {
      NList argument1 = argumentsList.first();
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

  return nSelection;
}

/*
definition of deftime type mapping function

*/

ListExpr deftimeTypeMappingFunction(ListExpr arguments)
{
  ListExpr type = NList::typeError("Operator deftime expects a mt type.");

  NList argumentsList(arguments);

  if(argumentsList.hasLength(1))
  {
    std::string argument1 = argumentsList.first().str();

    if(IsmtType(argument1))
    {
      type = NList(Periods::BasicType()).listExpr();
    }
  }

  return type;
}

}
