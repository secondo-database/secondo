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

#include "deftime.h"

#include "../msint.h"
#include "../msreal.h"
#include "../msbool.h"
#include "../msstring.h"

namespace raster2 {

  ValueMapping deftimeFuns[] =
  {
    deftimeFun<msint>,
    deftimeFun<msreal>,
    deftimeFun<msbool>,
    deftimeFun<msstring>,
    0
  };

  int deftimeSelectFun(ListExpr args) {
      NList type(args);

      if (type.first().isSymbol(msint::BasicType())) {
          return 0;
      } else if (type.first().isSymbol(msreal::BasicType())) {
          return 1;
      } else if (type.first().isSymbol(msbool::BasicType())) {
          return 2;
      } else if (type.first().isSymbol(msstring::BasicType())) {
          return 3;
      } else {
          return 4;
      }
  }

    ListExpr deftimeTypeMap(ListExpr args)
    {
      NList type(args);

      if (type.length() != 1) {
        return type.typeError("Expect one argument."); 
      }

      if (type.first() == NList(msbool::BasicType())
          || type.first() == NList(msreal::BasicType())
          || type.first() == NList(msint::BasicType())
          || type.first() == NList(msstring::BasicType()))
      {
         return NList(Periods::BasicType()).listExpr();
      }

      return NList::typeError("Expected msT.");
    }
}
