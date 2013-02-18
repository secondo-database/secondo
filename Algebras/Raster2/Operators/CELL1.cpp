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

#include "CELL1.h"

namespace raster2
{
  ListExpr cell1TypeMap (ListExpr args)
  {    
    ListExpr cell1TypeMapListExpr = NList::typeError("Expecting a xType.");
    NList types(args);
    /* Return CcInt                                */
    if (types.first() == NList(sint::BasicType()) ||
        types.first() == NList(msint::BasicType())  ) {
       cell1TypeMapListExpr = 
          NList(sint::wrapper_type::BasicType()).listExpr();
    }
    else if (types.first() == NList(sreal::BasicType()) ||
             types.first() == NList(msreal::BasicType())  ) {
       cell1TypeMapListExpr = 
          NList(sreal::wrapper_type::BasicType()).listExpr();
    }
    else if (types.first() == NList(sbool::BasicType()) ||
             types.first() == NList(msbool::BasicType())  ) {
       cell1TypeMapListExpr = 
          NList(sbool::wrapper_type::BasicType()).listExpr();
    }
    else if (types.first() == NList(sstring::BasicType()) ||
             types.first() == NList(msstring::BasicType())  )
    {
      cell1TypeMapListExpr = 
         NList(stype_helper<string>::wrapper_type::BasicType()).listExpr();
    } 
    
    return cell1TypeMapListExpr;
  } 
}
