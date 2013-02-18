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


#include "../mstype.h"
#include "../msint.h"
#include "../msreal.h"
#include "../msbool.h"
#include "../msstring.h"
#include "../stype.h"
#include "../sint.h"
#include "../sreal.h"
#include "../sbool.h"
#include "../sstring.h"
#include "../util/parse_error.h"

#include "CELL2.h"

namespace raster2
{
  ListExpr cell2TypeMap (ListExpr args)
  {    
    ListExpr cell2TypeMapListExpr = 
       NList::typeError("Cell2 expects two xType.");
    NList types(args);
    
    if(types.length()<2) {
      return cell2TypeMapListExpr;
    }
    
    /* Return CcInt                                */
    if (types.second() == NList(sint::BasicType()) ||
        types.second() == NList(msint::BasicType())  ) {
       cell2TypeMapListExpr = 
          NList(sint::wrapper_type::BasicType()).listExpr();
    }
    else if (types.second() == NList(sreal::BasicType()) ||
             types.second() == NList(msreal::BasicType())  ) {
       cell2TypeMapListExpr = 
          NList(sreal::wrapper_type::BasicType()).listExpr();
    }
    else if (types.second() == NList(sbool::BasicType()) ||
             types.second() == NList(msbool::BasicType())  ) {
       cell2TypeMapListExpr = 
          NList(sbool::wrapper_type::BasicType()).listExpr();
    }
    else if (types.second() == NList(sstring::BasicType()) ||
             types.second() == NList(msstring::BasicType())  )
    {
      cell2TypeMapListExpr = 
         NList(stype_helper<string>::wrapper_type::BasicType()).listExpr();
    } 
    
    return cell2TypeMapListExpr;
  } 
}
