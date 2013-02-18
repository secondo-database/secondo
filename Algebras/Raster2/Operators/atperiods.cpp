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

#include "atperiods.h"

namespace raster2 {

  ValueMapping atperiodsFuns[] =
  {
    atperiodsFun<int, mstype_helper<int> >,
    atperiodsFun<double, mstype_helper<double> >,
    atperiodsFun<char, msbool_helper >,
    atperiodsFun<string, mstype_helper<string> >,
    0
  };


    int atperiodsSelectFun(ListExpr args) {
        NList type(args);

        // The selection function should not have been called if the second
        // argument does not meet the criteria in the type mapping
        assert(type.second().isSymbol(Periods::BasicType()));

        if (type.first().isSymbol(msint::BasicType())) {
            return 0;
        }
        else if (type.first().isSymbol(msreal::BasicType())) {
            return 1;
        }
        else if (type.first().isSymbol(msbool::BasicType())) {
            return 2;
        }
        else if(type.first().isSymbol(msstring::BasicType())) {
            return 3;
        }

        return -1;
    }

	ListExpr atperiodsTypeMap(ListExpr args)
	{
	  NList type(args);

	  if (type.length() != 2) {
	    return type.typeError("Expect two arguments."); 
	  }

       if (type.second() != NList(Periods::BasicType())) {
	     return type.typeError("Expect sec arg periods."); 
	   }
	   
      if (type.first() == NList(msbool::BasicType()))
      {
	     return NList(msbool::BasicType()).listExpr();
	  }

      if (type.first() == NList(msint::BasicType()))
      {
	     return NList(msint::BasicType()).listExpr();
	  }

      if (type.first() == NList(msreal::BasicType()))
      {
	     return NList(msreal::BasicType()).listExpr();
	  }

      if (type.first() == NList(msstring::BasicType()))
      {
	     return NList(msstring::BasicType()).listExpr();
	  }

	  return NList::typeError("Expected msT.");
	}
}
