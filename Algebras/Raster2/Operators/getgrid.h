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

#ifndef RASTER2_GETGRID_H
#define RASTER2_GETGRID_H

#include <NList.h>

#include "../stype.h"
#include "../mstype.h"

namespace raster2
{
  extern ValueMapping getgridFuns[];
  ListExpr getgridTypeMap(ListExpr args);
  int getgridSelectFun(ListExpr args);

  struct getgridInfo : OperatorInfo
  {
    getgridInfo()
    {
      name      = "getgrid";
      signature = "sType -> grid";
      appendSignature("msType -> grid");
      syntax    = "getgrid(_)";
      meaning   = "Returns the grid.";
    }
  };

  template <typename T, typename Helper>
  int getgridFun(Word* args, Word& result, int message, Word& local, Supplier s)
  {
    typename Helper::implementation_type* pImplementationType =
      static_cast<typename Helper::implementation_type*>(args[0].addr);

    result = qp->ResultStorage(s);
      
    grid2* pResult = static_cast<grid2*>(result.addr);
    if((pImplementationType != 0) && pImplementationType->isDefined())
    {

      if(pResult != 0)
      {
        *pResult = pImplementationType->getGrid();
      }
    } else {
        pResult->set(0,0,1);
    }

    return 0;
  }
  
  template <typename T, typename Helper>
  int getgridFun3
      (Word* args, Word& result, int message, Word& local, Supplier s)
  {
    typename Helper::implementation_type* pImplementationType =
      static_cast<typename Helper::implementation_type*>(args[0].addr);

    result = qp->ResultStorage(s);

    grid3* pResult = static_cast<grid3*>(result.addr);

    if((pImplementationType != 0) && pImplementationType->isDefined())
    {
      if(pResult != 0)
      {
        *pResult = pImplementationType->getGrid();
      }
    } else {
        datetime::DateTime dt(datetime::durationtype);
        pResult->set(0,0,1,dt);
    }

    return 0;
  }
}

#endif /* #ifndef RASTER2_GETGRID_H */
