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

#include "compose.h"
#include "../util/types.h"

namespace raster2 {



  template <typename T>
  int composeFun
      (Word* args, Word& result, int message, Word& local, Supplier s)
  {

      // storage for the result
      result = qp->ResultStorage(s);
      
      // the moving point
      MPoint* movingPoint = static_cast<MPoint*>(args[0].addr);
      
      // the sT object
      typename T::this_type* raster =
          static_cast<typename T::this_type*>(args[1].addr);

      // The result of the compose
      typename T::moving_type* pResult =
            static_cast<typename T::moving_type*>(result.addr);


      if (!movingPoint->IsDefined() || !raster->isDefined()) {
        pResult->SetDefined(false);
        return 0;
      }

      pResult->Clear();
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
            typename T::cell_type v = raster->atlocation(xStart,yStart);  
            if(!raster->isUndefined(v)){  
                typename T::wrapper_type v1(true,v);
                pResult->MergeAdd(typename T::unit_type(
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
                  typename T::cell_type v = raster->atlocation(x,y);  
                  if(!raster->isUndefined(v)){  
                     typename T::wrapper_type v1(true,v);
                     pResult->MergeAdd(typename T::unit_type(iv,v1,v1));
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


  ValueMapping composeFuns[] =
  {
    composeFun<sint>,
    composeFun<sreal>,
    composeFun<sbool>,
    composeFun<sstring>,
    0
  };

  ListExpr composeTypeMap(ListExpr args)
  {
     if(!nl->HasLength(args,2)){
        return listutils::typeError("2 arguments expected");
     }
     ListExpr arg1 = nl->First(args);
     ListExpr arg2 = nl->Second(args);
 
     string err = "mpoint x stype expected";
    if(!MPoint::checkType(arg1)){
        return listutils::typeError(err + " (first arg is not an mpoint)");
    }
     if(!util::isSType(arg2)){
        return listutils::typeError(err + " (second arg is not an stype)");
     }

    std::string vname = nl->SymbolValue(arg2);
    std::string mname = util::getMovingBasicType(vname);
    return nl->SymbolAtom(mname);
  }

    int composeSelectFun(ListExpr args) {
        NList type(args);

        assert(type.first().isSymbol(MPoint::BasicType()));
        
        if (type.second().isSymbol(sint::BasicType())) {
            return 0;
        }
        
        if (type.second().isSymbol(sreal::BasicType())) {
            return 1;
        }
        
        if (type.second().isSymbol(sbool::BasicType())) {
            return 2;
        }
        
        if(type.second().isSymbol(sstring::BasicType())) {
            return 3;
        }
        
        return -1;
    }


}
