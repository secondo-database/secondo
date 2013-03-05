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

#include "createRaster.h"
#include "../sint.h"
#include "../sreal.h"
#include "../sbool.h"
#include "../sstring.h"
#include "../msint.h"
#include "../msreal.h"
#include "../msbool.h"
#include "../msstring.h"
#include "../isint.h"
#include "../isreal.h"
#include "../isbool.h"
#include "../isstring.h"
#include "../util/parse_error.h"
#include "../util/types.h"
#include "StandardTypes.h"

#include <ListUtils.h>

namespace raster2
{
  ListExpr createRasterTM(ListExpr args) {
     if(!nl->HasLength(args,2)){
        return listutils::typeError("2 arguments expected");
     }
     string err = "{int, real, bool, string} x {grid2, grid3} expected";
     ListExpr a1 = nl->First(args);
     ListExpr a2 = nl->Second(args);
     if(   !CcInt::checkType(a1) 
        && !CcReal::checkType(a1)
        && !CcBool::checkType(a1)
        && !CcString::checkType(a1)){
       return listutils::typeError(err + " (first arg is not a cell type)");
     }

     if(   !grid2::checkType(a2) 
        && !grid3::checkType(a2)){
       return listutils::typeError(err + " (second arg is not a grid)");
     }
     string t = nl->ToString(a1);
     if(grid2::checkType(a2)){
       return nl->SymbolAtom( util::getSpatialBasicType(t));
     } else {
       return nl->SymbolAtom( util::getMovingSpatialBasicType(t));
     }
  }

  int createRasterSelectFun(ListExpr args) {
     int pos = 0;
     int offset = 0;
     ListExpr a1 = nl->First(args);
     ListExpr a2 = nl->Second(args);
     if(CcBool::checkType(a1)){
       pos = 0;
     } else if(CcInt::checkType(a1)){
       pos = 1;
     } else if(CcReal::checkType(a1)){
       pos = 2;
     } else if(CcString::checkType(a1)){
       pos = 3; 
     } else {
       return -1;
     }
     if(grid2::checkType(a2)){
        offset = 0;
     } else if(grid3::checkType(a2)){
        offset = 4;
     } else {
        return -1;
     }
     return offset + pos;
  }

  
  
  template <typename T>
  int createRasterFun(Word* args, Word& result, int message, 
                   Word& local, Supplier s)
  {
    result = qp->ResultStorage(s);
    T* res = static_cast<T*>(result.addr);
    typedef typename T::grid_type grid_type;
    grid_type* grid = static_cast<grid_type*>(args[1].addr);
    res->clear();
    res->setGrid(*grid);
    return 0;
  }
  

  
  
  ValueMapping createRasterFuns[] = {
     createRasterFun<sbool>,
     createRasterFun<sint>,
     createRasterFun<sreal>,
     createRasterFun<sstring>,
     createRasterFun<msbool>,
     createRasterFun<msint>,
     createRasterFun<msreal>,
     createRasterFun<msstring>,
     0
  };
}
