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

#include <string>

#include "atlocation.h"
#include "../sint.h"
#include "../msint.h"
#include "../sreal.h"
#include "../sstring.h"
#include "../msreal.h"
#include "../sbool.h"
#include "../msbool.h"
#include "../msstring.h"

namespace raster2 {
    int atlocationFunSString(Word*, Word&, int, Word&, Supplier);

    ValueMapping atlocationFuns[] = {
        atlocationFun<int, stype_helper<int> >,
        atlocationFun<double, stype_helper<double> >,
        atlocationFun<char, sbool_helper >,
        atlocationFunSString,
        atlocationFunMType<msint, MInt, int>,
//        atlocationFunMType<double, mstype_helper<double> >,
//        atlocationFunMType<char, msbool_helper >,
//        atlocationFunMType<std::string, mstype_helper<string> >,
        0
    };

    int atlocationSelectFun(ListExpr args) {
        NList type(args);

        // The selection function should not have been called if the second
        // argument does not meet the criteria in the type mapping
        assert(type.second().isSymbol(Point::BasicType()));

        if (type.first().isSymbol(sint::BasicType())) {
            return 0;
        }
        else if (type.first().isSymbol(sreal::BasicType())) {
            return 1;
        }
        else if (type.first().isSymbol(sbool::BasicType())) {
            return 2;
        }
        else if(type.first().isSymbol(sstring::BasicType())) {
            return 3;
        }
        else if (type.first().isSymbol(msint::BasicType())) {
            return 4;
        }
//        else if (type.first().isSymbol(msreal::BasicType())) {
//            return 5;
//        }
//        else if (type.first().isSymbol(msbool::BasicType())) {
//            return 6;
//        }
//        else if (type.first().isSymbol(msstring::BasicType())) {
//            return 7;
//        }


        return 5;
    }

    ListExpr atlocationTypeMap(ListExpr args)
    {
        NList types(args);

        if (types.second() == NList(Point::BasicType())) {
            if(types.first() == NList(sint::BasicType())) {
                return NList(sint::wrapper_type::BasicType()).listExpr();
            }
            else if(types.first() == NList(sreal::BasicType())) {
                return NList
                    (sreal::wrapper_type::BasicType()).listExpr();
            }
            else if(types.first() == NList(sbool::BasicType())) {
                return NList
                    (sbool::wrapper_type::BasicType()).listExpr();
            }
            else if(types.first() == NList(sstring::BasicType())) {
                return NList
                    (CcString::BasicType()).listExpr();
            }
            else if(types.first() == NList(msint::BasicType())) {
                return NList(MInt::BasicType()).listExpr();
            }
            else if(types.first() == NList(msreal::BasicType())) {
                return NList(MReal::BasicType()).listExpr();
            }
            else if(types.first() == NList(msbool::BasicType())) {
                return NList(MBool::BasicType()).listExpr();
            }
//          else if(types.first() == NList(msstring::BasicType())) {
//          return NList(mstype_helper<string>::wrapper_type::BasicType()).
//                  listExpr();
//          }

        }
        return NList::typeError
                ("Expecting an sType or msType and a " +
                       Point::BasicType() + ".");
    

      
    }

    int atlocationFunSString
        (Word* args, Word& result, int message, Word& local, Supplier s)
    {
      sstring* pSString = static_cast<sstring*>(args[0].addr);
      Point* pPoint = static_cast<Point*>(args[1].addr);
      result = qp->ResultStorage(s);
      CcString* pResult = static_cast<CcString*>(result.addr);

      assert(pSString != 0);
      assert(pPoint != 0);
      assert(pResult != 0);

      pResult->SetDefined(false);

      if (pPoint->IsDefined()) {
          std::string value =
                  pSString->atlocation(pPoint->GetX(), pPoint->GetY());
          pResult->Set(UNDEFINED_STRING != value, value);
      }

      return 0;
    }
}
