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

#ifndef RASTER2_COMPOSE_H
#define RASTER2_COMPOSE_H

#include <NList.h>

#include "../sbool.h"
#include "../sreal.h"
#include "../sint.h"
#include "../sstring.h"
#include "TemporalAlgebra.h"
#include "RTreeAlgebra.h"
#include "CellIterator.h"

namespace raster2 {
  extern ValueMapping composeFuns[];
  ListExpr composeTypeMap(ListExpr args);
  int composeSelectFun(ListExpr args);

  template <typename T, typename Helper>
  int composeFun
      (Word* args, Word& result, int message, Word& local, Supplier s)
  {

      // storage for the result
      result = qp->ResultStorage(s);
      
      // the moving point
      MPoint* movingPoint = static_cast<MPoint*>(args[0].addr);
      
      // the sT object
      typename Helper::implementation_type* raster =
          static_cast<typename Helper::implementation_type*>(args[1].addr);

      // The result of the compose
      typename Helper::moving_type* pResult =
            static_cast<typename Helper::moving_type*>(result.addr);

      pResult->Clear();

      if (!movingPoint->IsDefined()) {
        pResult->SetDefined(false);
        return 0;
      }

      pResult->StartBulkLoad();

      // get the number of components
      int num = movingPoint->GetNoComponents();

      UPoint unit(0);
      grid2 grid = raster->getGrid();
      grid2::index_type cell1;
      grid2::index_type cell2;

      for (int i = 0; i < num; i++)
      {
          movingPoint->Get(i, unit);
          
          // get the coordinates
          double xStart = unit.p0.GetX();
          double yStart = unit.p0.GetY();
          double xEnd = unit.p1.GetX();
          double yEnd = unit.p1.GetY();

          cell1 = grid.getIndex(xStart,yStart);
          cell2 = grid.getIndex(xEnd, yEnd);

          if(cell1==cell2){ // only a constant unit in result
            T v = raster->atlocation(xStart,yStart);  
            if(!raster->isUndefined(v)){  
                typename Helper::wrapper_type v1(true,v);
                pResult->MergeAdd(typename Helper::unit_type(
                                   unit.timeInterval,v1,v1));
            }
          } else {
            DateTime t1 = unit.timeInterval.start;
            DateTime t2 = unit.timeInterval.end;
            DateTime dur = t2 - t1;
            CellIterator it(grid,xStart,yStart,xEnd,yEnd);
            double dx = xEnd - xStart;
            double dy = yEnd - yStart; 
            while(it.hasNext()){
               pair<double,double> p = it.next();
               DateTime s = t1 + (dur*p.first);
               DateTime e = t1 + (dur*p.second);
               if(e>s){
                  Interval<Instant> iv(s,e,true,false);
		  double delta  =(p.first + p.second) / 2.0;
                  double x = xStart + delta*dx;
                  double y = yStart + delta*dy;
                  T v = raster->atlocation(x,y);  
                  if(!raster->isUndefined(v)){  
                     typename Helper::wrapper_type v1(true,v);
                     pResult->MergeAdd(typename Helper::unit_type(iv,v1,v1));
                  }
               } else {
                  assert(e==s);
               } 
            }  
          }
      }
       
      pResult->EndBulkLoad();

    return 0;
  }


    struct composeInfo : OperatorInfo 
    {
      composeInfo()
      { 
        name      = "compose";
        signature = MPoint::BasicType()
            + " compose "
            + sbool::BasicType() + "-> " 
            + MBool::BasicType();
        appendSignature(MPoint::BasicType() + " compose "
            + sreal::BasicType() + "-> " 
            + MReal::BasicType());
        appendSignature(MPoint::BasicType() + " compose "
            + sint::BasicType() + "-> " 
            + MInt::BasicType());
        appendSignature(MPoint::BasicType() + " compose "
            + sstring::BasicType() + "-> " 
            + MString::BasicType());

        syntax    = "compose(_)";
        meaning   = "merges mpoint and sT into mT";
      }          
    };
}

#endif /* #define RASTER2_COMPOSE_H */

