/*
This file is part of SECONDO.

Copyright (C) 2011, University in Hagen, Department of Computer Science,
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

#include "inst.h"
#include "../isint.h"
#include "../isreal.h"
#include "../isbool.h"
#include "../isstring.h"

namespace raster2
{
  ValueMapping instFuns[] =
  {
    instFun<int, istype_helper<int> >,
    instFun<double, istype_helper<double> >,
    instFun<char, istype_helper<char> >,
    instFun<string, istype_helper<string> >,
    0
  };

  int instSelectFun(ListExpr args)
  {
    int nSelection = -1;
    
    NList type(args);

    if(type.first().isSymbol(isint::BasicType()))
    {
      nSelection = 0;
    }
    
    else if(type.first().isSymbol(isreal::BasicType()))
    {
      nSelection = 1;
    }
    
    else if(type.first().isSymbol(isbool::BasicType()))
    {
      nSelection = 2;
    }
    
    else if(type.first().isSymbol(isstring::BasicType()))
    {
      nSelection = 3;
    }

    return nSelection;
  }

  ListExpr instTypeMap(ListExpr args)
  {
    ListExpr instTypeMapListExpr = NList::typeError("Expecting a isType.");
    
    NList types(args);
    
    if(types.first() == NList(isint::BasicType()) ||
       types.first() == NList(isreal::BasicType()) ||
       types.first() == NList(isbool::BasicType()) ||
       types.first() == NList(isstring::BasicType()))
    {
      instTypeMapListExpr = NList(DateTime::BasicType()).listExpr();
    }

    return instTypeMapListExpr;
  }
}
