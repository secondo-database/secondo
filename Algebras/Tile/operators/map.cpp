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

#include "map.h"
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
definition of map functions

*/

ValueMapping mapFunctions[] =
{
  mapFunctiont<tint, tProperties<int>,
               tint, tProperties<int> >,
  mapFunctiont<tint, tProperties<int>,
               treal, tProperties<double> >,
  mapFunctiont<tint, tProperties<int>,
               tbool, tProperties<char> >,
  mapFunctiont<tint, tProperties<int>,
               tstring, tProperties<std::string> >,
  
  mapFunctiont<treal, tProperties<double>,
               tint, tProperties<int> >,
  mapFunctiont<treal, tProperties<double>,
               treal, tProperties<double> >,
  mapFunctiont<treal, tProperties<double>,
               tbool, tProperties<char> >,
  mapFunctiont<treal, tProperties<double>,
               tstring, tProperties<std::string> >,
          
  mapFunctiont<tbool, tProperties<char>,
               tint, tProperties<int> >,
  mapFunctiont<tbool, tProperties<char>,
               treal, tProperties<double> >,
  mapFunctiont<tbool, tProperties<char>,
               tbool, tProperties<char> >,
  mapFunctiont<tbool, tProperties<char>,
               tstring, tProperties<std::string> >,

  mapFunctiont<tstring, tProperties<std::string>,
               tint, tProperties<int> >,
  mapFunctiont<tstring, tProperties<std::string>,
               treal, tProperties<double> >,
  mapFunctiont<tstring, tProperties<std::string>,
               tbool, tProperties<char> >,
  mapFunctiont<tstring, tProperties<std::string>,
               tstring, tProperties<std::string> >,
  
  mapFunctionmt<mtint, mtProperties<int>,
                mtint, mtProperties<int> >,
  mapFunctionmt<mtint, mtProperties<int>,
                mtreal, mtProperties<double> >,
  mapFunctionmt<mtint, mtProperties<int>,
                mtbool, mtProperties<char> >,
  mapFunctionmt<mtint, mtProperties<int>,
                mtstring, mtProperties<std::string> >,

  mapFunctionmt<mtreal, mtProperties<double>,
                mtint, mtProperties<int> >,
  mapFunctionmt<mtreal, mtProperties<double>,
                mtreal, mtProperties<double> >,
  mapFunctionmt<mtreal, mtProperties<double>,
                mtbool, mtProperties<char> >,
  mapFunctionmt<mtreal, mtProperties<double>,
                mtstring, mtProperties<std::string> >,

  mapFunctionmt<mtbool, mtProperties<char>,
                mtint, mtProperties<int> >,
  mapFunctionmt<mtbool, mtProperties<char>,
                mtreal, mtProperties<double> >,
  mapFunctionmt<mtbool, mtProperties<char>,
                mtbool, mtProperties<char> >,
  mapFunctionmt<mtbool, mtProperties<char>,
                mtstring, mtProperties<std::string> >,
                
  mapFunctionmt<mtstring, mtProperties<std::string>,
                mtint, mtProperties<int> >,
  mapFunctionmt<mtstring, mtProperties<std::string>,
                mtreal, mtProperties<double> >,
  mapFunctionmt<mtstring, mtProperties<std::string>,
                mtbool, mtProperties<char> >,
  mapFunctionmt<mtstring, mtProperties<std::string>,
                mtstring, mtProperties<std::string> >,
  0
};

/*
definition of map select function

*/

int mapSelectFunction(ListExpr arguments)
{
  int nSelection = -1;

  if(arguments != 0)
  {
    NList argumentsList(arguments);

    if(argumentsList.hasLength(2))
    {
      NList argument1 = argumentsList.first();
      std::string argument2= argumentsList.second().third().str();
      
      int argument1Index = -1;
      int argument2Index = -1;

      const int TYPE_NAMES = 12;
      const std::string TYPE_NAMES_ARRAY[TYPE_NAMES] =
      {
        tint::BasicType(),
        treal::BasicType(),
        tbool::BasicType(),
        tstring::BasicType(),
        mtint::BasicType(),
        mtreal::BasicType(),
        mtbool::BasicType(),
        mtstring::BasicType(),
        CcInt::BasicType(),
        CcReal::BasicType(),
        CcBool::BasicType(),
        CcString::BasicType()
      };

      for(int i = 0; i < TYPE_NAMES; i++)
      {
        if(argument1.isSymbol(TYPE_NAMES_ARRAY[i]))
        {
          argument1Index = i;
        }
        
        if(argument2 == TYPE_NAMES_ARRAY[i])
        {
          argument2Index = i;
        }
      }
      
      if(argument1Index >= 0 &&
         argument2Index >= 0)
      {
        nSelection = (argument1Index * 4) +
                     (argument2Index % 4);
      }
    }
  }

  return nSelection;
}

/*
definition of map type mapping function

*/

ListExpr mapTypeMappingFunction(ListExpr arguments)
{
  ListExpr type = NList::typeError("Expecting two arguments.");

  if(arguments != 0)
  {
    NList argumentsList(arguments);
    
    if(argumentsList.hasLength(2))
    {
      std::string argument1 = argumentsList.first().str();
      
      if(IstType(argument1) ||
         IsmtType(argument1))
      {
        NList argument2 = argumentsList.second();

        if(listutils::isMap<1>(argument2.listExpr()))
        {
          std::string valueWrapperType = GetValueWrapperType(argument1);
          std::string functionArgument = argument2.second().str();
  
          if(valueWrapperType == functionArgument)
          {
            std::string functionResult = argument2.third().str();
            std::string typeName;
  
            if(IstType(argument1))
            {
              typeName = GettType(functionResult);
            }
  
            else
            {
              typeName = GetmtType(functionResult);
            }
  
            if(typeName.empty() == false)
            {
              type = NList(typeName).listExpr();
            }
  
            else
            {
              type = NList::typeError("Parameter function result "
                                      "is not a valid value type");
            }
          }
  
          else
          {
            type = NList::typeError("Function argument does not fit the type");
          }
        }
  
        else
        {
          type = NList::typeError("Second argument must be a function "
                                  "with 1 argument");
        }
      }
  
      else
      {
        type = NList::typeError("First argument must be a t type or "
                                "a mt type.");
      }
    }
  }

  return type;
}

}
