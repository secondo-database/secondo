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
      Point* pPoint = static_cast<Point*>(args[1].addr);
      typename T::this_type* praster =
          static_cast<typename T::this_type*>(args[0].addr);

      if(qp->GetNoSons(s)==2){
         typename T::moving_type* pResult = 
             static_cast<typename T::moving_type*>(result.addr);

         if(!pPoint->IsDefined() || !praster->isDefined()){
            pResult->SetDefined(false);
         } else {
           typename T::moving_type* tmp = 
                praster->atlocation(pPoint->GetX(), pPoint->GetY());
            pResult->CopyFrom(tmp);
            delete tmp;
         }
     } else {
         typename T::wrapper_type* pResult = 
             static_cast<typename T::wrapper_type*>(result.addr);
         DateTime* instant = static_cast<DateTime*>(args[2].addr);
         if(    !pPoint->IsDefined() || !praster->isDefined() 
             || !instant->IsDefined()){
            pResult->SetDefined(false);
         } else {
           typename T::cell_type  tmp = 
                praster->atlocation(pPoint->GetX(), pPoint->GetY(),
                              instant->ToDouble());
            (*pResult) = T::wrap(tmp);
         }
     }
     return 0;
  }


    ValueMapping atlocationFuns[] = {
       atlocationSFun<sbool>,
       atlocationSFun<sint>,
       atlocationSFun<sreal>,
       atlocationSFun<sstring>,
       atlocationMSFun<msbool>,
       atlocationMSFun<msint>,
       atlocationMSFun<msreal>,
       atlocationMSFun<msstring>,
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
        string err = "{stype, mstype} x point  "
                     "or mstype x point x instant expected";
        if(!nl->HasLength(args,2) && !nl->HasLength(args,3)){
           return listutils::typeError("two arguments required");
        }     
        if(!Point::checkType(nl->Second(args))){
          return listutils::typeError(err);
        }
        string raster = nl->ToString(nl->First(args));
        if(util::isSType(raster)){
          if(!nl->HasLength(args,2)){
            return listutils::typeError(err);
          }
          return nl->SymbolAtom(util::getValueBasicType(raster));
        }
        if(util::isMSType(raster)){
          if(nl->HasLength(args,2)){
             return nl->SymbolAtom(util::getMovingBasicType(raster));
          } else { // three arguments
             if(!DateTime::checkType(nl->Third(args))){
                return listutils::typeError(err);
             }
             return nl->SymbolAtom(util::getValueBasicType(raster));
          }
        }
        return listutils::typeError(err);

    }

}
