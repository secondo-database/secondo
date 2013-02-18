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

#ifndef RASTER2_ATINSTANT_H
#define RASTER2_ATINSTANT_H

#include "../mstype.h"
#include "../istype.h"

namespace raster2
{
  extern ValueMapping atinstantFuns[];
  ListExpr atinstantTypeMap(ListExpr args);
  int atinstantSelectFun(ListExpr args);
  
  struct atinstantInfo : OperatorInfo
  {
    atinstantInfo()
    {
      name      = "atinstant";
      signature = "msType " + DateTime::BasicType() + " -> isType";
      syntax    = "_ atinstant _";
      meaning   = "Returns the values for a time point.";
    }
  };

  template <typename T, typename Helper, typename ResultHelper>
  int atinstantFun
    (Word* args, Word& result, int message, Word& local, Supplier s)
  {
    result = qp->ResultStorage(s);
    typename ResultHelper::implementation_type* pResult =
      static_cast<typename ResultHelper::implementation_type*>(result.addr);

    typename Helper::implementation_type* mstype =
      static_cast<typename Helper::implementation_type*>(args[0].addr);
    DateTime* instant = static_cast<DateTime*>(args[1].addr);
    
    mstype->setCacheSize(16);

    grid3 msgrid = mstype->getGrid();
    grid2 sgrid(msgrid.getOriginX(), msgrid.getOriginY(), msgrid.getLength());

    double i = instant->ToDouble();
    
    typename ResultHelper::spatial_type* values =
            new typename ResultHelper::spatial_type();
    values->setGrid(sgrid);

    Rectangle<3> bbox = mstype->bbox();

    RasterIndex<3> msfrom = msgrid.getIndex(bbox.MinD(0), bbox.MinD(1), i);
    RasterIndex<3> msto = msgrid.getIndex(bbox.MaxD(0), bbox.MaxD(1), i);
    
    RasterIndex<2> from = ResultHelper::spatial_type::storage_type::
                          getRegion((int[]){msfrom[0], msfrom[1]});
    RasterIndex<2> to = (int[]){msto[0], msto[1]};

    RasterIndex<2> region_size = values->begin_regions().region_size;
    
    RasterIndex<2> current = from;
    
    while (current <= to) 
    {
       for (RasterIndex<2> index = current, e = current + region_size; 
                           index < e; index.increment(current, e))
       {
         typename Helper::implementation_type::index_type i2 = 
                              (int[]){index[0], index[1], msfrom[2]};
         values->set(index, mstype->get(i2));
       }

       current[0] += region_size[0];
       
       if (current[0] > to[0]) 
       {
         current[1] += region_size[1];

         if (current[1] < to[1]) 
         {
           current[0] = from[0];
         }
       }
    }

    pResult->setInstant(new DateTime(*instant));
    pResult->setValues(values);

    return 0;
  }
}

#endif /* #define RASTER2_ATINSTANT_H */
