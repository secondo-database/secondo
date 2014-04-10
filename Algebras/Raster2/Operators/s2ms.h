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

#ifndef RASTER2_S2MS_H
#define RASTER2_S2MS_H

#include <unordered_set>

#include <AlgebraTypes.h>
#include <DateTime.h>
#include <NestedList.h>

#include "../grid3.h"

namespace raster2
{
    extern ValueMapping s2msFuns[];
    ListExpr s2msTypeMap(ListExpr args);
    int s2msSelectFun(ListExpr args);

    struct s2msInfo : OperatorInfo
    {
      s2msInfo()
      {
        name      = "s2ms";
        signature = "sType x duration x instant1 x instant2 -> msType";
        syntax    = "s2ms(_, _, _, _)";
        meaning   = "Adds a time component to an sType.";
      }
    };

    template <typename SType, typename MSType>
    int s2msFun
        (Word* args, Word& result, int message, Word& local, Supplier s)
    {
        result = qp->ResultStorage(s);

        SType& sin = *static_cast<SType*>(args[0].addr);
        DateTime& duration = *static_cast<DateTime*>(args[1].addr);
        Instant& start = *static_cast<Instant*>(args[2].addr);
        Instant& finish = *static_cast<Instant*>(args[3].addr);
        MSType& msout = *static_cast<MSType*>(result.addr);

        if(!sin.isDefined() ||
           !duration.IsDefined() ||
           !start.IsDefined() ||
           !finish.IsDefined()){
           msout.setDefined(false); 
        }
        msout.clear();




        grid3 g(sin.getGrid().getOriginX(),
                sin.getGrid().getOriginY(),
                sin.getGrid().getLength(),
                duration.ToDouble());
        msout.setGrid(g);

        int t_from = int(start.ToDouble()/duration.ToDouble());
        int t_to = int(finish.ToDouble()/duration.ToDouble());

        if (t_from == t_to) {
          if ((finish.ToDouble() - start.ToDouble())/duration.ToDouble() >= 0.5)
          {
            ++t_to;
          }
        } else {
          if (start.ToDouble() - t_from * duration.ToDouble() >
              duration.ToDouble()/2)
          {
            ++t_from;
          }
          if (finish.ToDouble() - t_to * duration.ToDouble() >=
              duration.ToDouble()/2)
          {
            ++t_to;
          }
        }

        if (t_to <= t_from) {
          return 0;
        }

        const typename SType::index_type& s_region_size
          = SType::riter_type::region_size;

        const typename MSType::index_type& m_region_size
          = MSType::riter_type::region_size;

        int cache_size = 1;
        cache_size *= s_region_size[0]/m_region_size[0] + 1;
        cache_size *= s_region_size[1]/m_region_size[1] + 1;
        cache_size *= (t_to - t_from)/m_region_size[2] + 1;

        msout.setCacheSize(cache_size);

        for (typename SType::riter_type rit = sin.begin_regions(),
                                         re = sin.end_regions();
             rit != re; ++rit)
        {
          for (typename SType::index_type i = *rit, e = *rit + s_region_size;
               i < e; i.increment(*rit, e))
          {
            for (int t = t_from; t < t_to; ++t) {
                msout.set((int[]){i[0], i[1], t}, sin.get(i));
            }
          }
        }

        return 0;
    }
}

#endif /* #ifndef RASTER2_S2MS_H */
