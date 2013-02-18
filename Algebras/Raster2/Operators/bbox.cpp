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

#include "bbox.h"
#include "../sint.h"
#include "../sreal.h"
#include "../sbool.h"
#include "../sstring.h"
#include "../msint.h"
#include "../msreal.h"
#include "../msbool.h"
#include "../msstring.h"

namespace raster2
{
  ValueMapping bboxFuns[] =
  {
    bboxFun<int, stype_helper<int> >,
    bboxFun<double, stype_helper<double> >,
    bboxFun<char, sbool_helper >,
    bboxFun<string, stype_helper<string> >,
    bboxFun3<int, mstype_helper<int> >,
    bboxFun3<double, mstype_helper<double> >,
    bboxFun3<char, msbool_helper >,
    bboxFun3<string, mstype_helper<string> >,
    0
  };

  int bboxSelectFun(ListExpr args)
  {
    int nSelection = -1;
    
    NList type(args);

    if(type.first().isSymbol(sint::BasicType()))
    {
      nSelection = 0;
    }
    
    else if(type.first().isSymbol(sreal::BasicType()))
    {
      nSelection = 1;
    }
    
    else if(type.first().isSymbol(sbool::BasicType()))
    {
      nSelection = 2;
    }
    
    else if(type.first().isSymbol(sstring::BasicType()))
    {
      nSelection = 3;
    }
    
    else if(type.first().isSymbol(msint::BasicType()))
    {
      nSelection = 4;
    }
    
    else if(type.first().isSymbol(msreal::BasicType()))
    {
      nSelection = 5;
    }
    
    else if(type.first().isSymbol(msbool::BasicType()))
    {
      nSelection = 6;
    }
    
    else if(type.first().isSymbol(msstring::BasicType()))
    {
      nSelection = 7;
    }

    return nSelection;
  }

  ListExpr bboxTypeMap(ListExpr args)
  {
    NList types(args);

    if(types.first() == NList(sint::BasicType()) ||
       types.first() == NList(sreal::BasicType()) ||
       types.first() == NList(sbool::BasicType()) ||
       types.first() == NList(sstring::BasicType()))
    {
      return NList(Rect::BasicType()).listExpr();
    }

    if(types.first() == NList(msint::BasicType()) ||
       types.first() == NList(msreal::BasicType()) ||
       types.first() == NList(msbool::BasicType()) ||
       types.first() == NList(msstring::BasicType()))
    {
      return NList(Rectangle<3>::BasicType()).listExpr();
    }

    return NList::typeError("Expecting an sType or msType.");
  }
}
