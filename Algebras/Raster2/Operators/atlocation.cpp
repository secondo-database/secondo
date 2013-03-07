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
#include "../util/types.h"

namespace raster2 {


  template <typename T>
  int atlocationSFun(Word* args, Word& result, 
                     int message, Word& local, Supplier s)
  {
      result = qp->ResultStorage(s);
      typename T::wrapper_type* pResult = 
          static_cast<typename T::wrapper_type*>(result.addr);

      typename T::this_type* praster =
          static_cast<typename T::this_type*>(args[0].addr);

      Point* pPoint = static_cast<Point*>(args[1].addr);

      if(!pPoint->IsDefined() || !praster->isDefined()){
         pResult->SetDefined(false);
      } else {
        (*pResult) = T::wrap(praster->atlocation(pPoint->GetX(),
                                                 pPoint->GetY()));
      }
    return 0;
  }

  template <typename T>
  int atlocationMSFun(Word* args, Word& result, 
                      int message, Word& local, Supplier s)
  {
      result = qp->ResultStorage(s);
      typename T::moving_type* pResult = 
          static_cast<typename T::moving_type*>(result.addr);

      typename T::this_type* praster =
          static_cast<typename T::this_type*>(args[0].addr);

      Point* pPoint = static_cast<Point*>(args[1].addr);

      if(!pPoint->IsDefined() || !praster->isDefined()){
         pResult->SetDefined(false);
      } else {
        typename T::moving_type* tmp = 
             praster->atlocation(pPoint->GetX(), pPoint->GetY());
         pResult->CopyFrom(tmp);
         delete tmp;
      }
    return 0;
  }


    int atlocationSStringFun
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


    int atlocationMSStringFun
        (Word* args, Word& result, int message, Word& local, Supplier s)
    {
      result = qp->ResultStorage(s);
      MString* pResult = static_cast<MString*>(result.addr);
      cout << "AtLOCATION not implemented for msstring" << endl;
      pResult->SetDefined(false);
      return 0;
    }


    ValueMapping atlocationFuns[] = {
       atlocationSFun<sbool>,
       atlocationSFun<sint>,
       atlocationSFun<sreal>,
       atlocationSStringFun,
       atlocationMSFun<msbool>,
       atlocationMSFun<msint>,
       atlocationMSFun<msreal>,
       atlocationMSStringFun,
       0
    };

    int atlocationSelectFun(ListExpr args) {
       string rtype = nl->ToString(nl->First(args));
       int offset = util::isSType(rtype)?0:4;
       string vtype = util::getValueBasicType(rtype);
       int pos = 0;
       if(vtype==CcBool::BasicType()){
          pos = 0;
       } else if(vtype==CcInt::BasicType()){
          pos = 1;
       } else if(vtype==CcReal::BasicType()){
          pos = 2;
       } else if(vtype==CcString::BasicType()){
          pos = 3;
       }
       return offset + pos;
    }

    ListExpr atlocationTypeMap(ListExpr args)
    {
        string err = "rastertype x point expected";
        if(!nl->HasLength(args,2)){
           return listutils::typeError("two arguments required");
        }     
        if(!Point::checkType(nl->Second(args))){
          return listutils::typeError(err);
        }
        string raster = nl->ToString(nl->First(args));
        if(util::isSType(raster)){
          return nl->SymbolAtom(util::getValueBasicType(raster));
        }
        if(util::isMSType(raster)){
          return nl->SymbolAtom(util::getMovingBasicType(raster));
        }
        return listutils::typeError(err);

    }

}
