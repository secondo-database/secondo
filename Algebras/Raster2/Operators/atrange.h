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

#ifndef RASTER2_ATRANGE_H
#define RASTER2_ATRANGE_H

#include "../stype.h"
#include "../mstype.h"

namespace raster2
{
  extern ValueMapping atrangeFuns[];
  ListExpr atrangeTypeMap(ListExpr args);
  int atrangeSelectFun(ListExpr args);

  struct atrangeInfo : OperatorInfo
  {
    atrangeInfo()
    {
      name      = "atrange";
      signature = "sType x " + Rect::BasicType() + " -> sType";
      appendSignature("msType x Instant x Instant " + Rect::BasicType() +
                      " -> msType");
      syntax    = "_ atrange [_,_,_]";
      meaning   = "Returns the values at range of the rect.";
    }
  };

  template <typename T, typename Helper>
  int atrangeFun(Word* args, Word& result, int message, Word& local, Supplier s)
  {
    typename Helper::implementation_type* pImplementationType =
      static_cast<typename Helper::implementation_type*>(args[0].addr);
    Rect* pRect = static_cast<Rect*>(args[1].addr);
    result = qp->ResultStorage(s);
    typename Helper::implementation_type* pResult =
        static_cast<typename Helper::implementation_type*>(result.addr);

    if((pImplementationType != 0) &&
       (pRect != 0) && 
        pImplementationType->isDefined() &&
        pRect->IsDefined()) {

      if(pResult != 0)
      {
        typename Helper::implementation_type* patrangeResult = 
          pImplementationType->atrange(*pRect);
          
        if(patrangeResult != 0)
        {
          std::swap(*pResult, *patrangeResult);
          patrangeResult->destroy();
          delete patrangeResult;
        }
      }
    } else {
       pResult->setDefined(false);
    }

    return 0;
  }

  template <typename MSType>
  int atrangeMFun
      (Word* args, Word& result, int message, Word& local, Supplier s)
  {
      result = qp->ResultStorage(s);
      MSType& msin = *static_cast<MSType*>(args[0].addr);
      Rect* pRect = static_cast<Rect*>(args[1].addr);
      MSType* res = static_cast<MSType*>(result.addr);

      if(   !msin.isDefined()
         || !pRect->IsDefined() ){
        res->setDefined(false);
        return 0;
      }


      MSType* msout =0;
      if(qp->GetNoSons(s) == 4){
         Instant* start = static_cast<Instant*>(args[2].addr);
         Instant* end = static_cast<Instant*>(args[3].addr);
         if( !start->IsDefined() || !end->IsDefined()){
            res->setDefined(false);
            return 0;
         }
         Interval<DateTime> lookupinterval(*start, *end, true, false);

         if(!lookupinterval.IsValid()) {
            res->setDefined(false);
            return 0;
         }
         //Correct time offset when using negative instant values
         const DateTime* mSec = new DateTime(0.00000001);

         if(start->LessThanZero())
           start->Minus(mSec);

         if(end->LessThanZero())
           end->Minus(mSec);

         delete mSec;
         mSec = 0;
         res->clear();
         msout = msin.atrange(*pRect, *start, *end);
      } else {
         msout = msin.atrange(*pRect);
      }

      if(msout != 0)
      {
         std::swap(*res, *msout);
         delete msout;
      }

      return 0;
  }
}

#endif /* #ifndef RASTER2_ATRANGE_H */
