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

#include "ruggedness.h"
#include "../Raster2/sint.h"
#include "../Raster2/sreal.h"

namespace GISAlgebra {

  template <typename T>
  int ruggednessFun(Word* args, Word& result, 
                     int message, Word& local, Supplier s)
  {
    result = qp->ResultStorage(s);

    typename T::this_type* s_out =
          static_cast<typename T::this_type*>(result.addr);
    typename T::this_type* s_in =
          static_cast<typename T::this_type*>(args[0].addr);

    raster2::grid2 grid = s_in->getGrid();

    s_out->setGrid(grid);

    Rectangle<2> bbox = s_in->bbox();

    raster2::RasterIndex<2> from = grid.getIndex(bbox.MinD(0), bbox.MinD(1));
    raster2::RasterIndex<2> to = grid.getIndex(bbox.MaxD(0), bbox.MaxD(1));

    for (raster2::RasterIndex<2> index=from; index < to; 
                                             index.increment(from, to))
    {
	// Mitte
        double e = s_in->get(index);

        double a = s_in->get((int[]){index[0] - 1, index[1] + 1});
        if (s_in->isUndefined(a)){a = e;}
        double b = s_in->get((int[]){index[0], index[1] + 1});
        if (s_in->isUndefined(b)){b = e;}
        double c = s_in->get((int[]){index[0] + 1, index[1] + 1});
        if (s_in->isUndefined(c)){c = e;}
        double d = s_in->get((int[]){index[0] - 1, index[1]});
        if (s_in->isUndefined(d)){d = e;}
        double f = s_in->get((int[]){index[0] + 1, index[1]});
        if (s_in->isUndefined(f)){f = e;}
        double g = s_in->get((int[]){index[0] - 1, index[1] - 1});
        if (s_in->isUndefined(g)){g = e;}
        double h = s_in->get((int[]){index[0], index[1] - 1});
        if (s_in->isUndefined(h)){h = e;}
        double i = s_in->get((int[]){index[0] + 1, index[1] - 1});
        if (s_in->isUndefined(i)){i = e;}

        // Differenz zwischen Mitte und umgebenden Zellen bilden, quadrieren
        double diff1 = (a - e) * (a - e);
        double diff2 = (b - e) * (b - e);
        double diff3 = (c - e) * (c - e);
        double diff4 = (d - e) * (d - e);
        double diff5 = (f - e) * (f - e);
        double diff6 = (g - e) * (g - e);
        double diff7 = (h - e) * (h - e);
        double diff8 = (i - e) * (i - e);

        // Diffs addieren und aus der Summe die Wurzel ziehen
        double ruggedness = sqrt(diff1 + diff2 + diff3 + diff4 + 
                                 diff5 + diff6 + diff7 + diff8);

        s_out->set(index, ruggedness);
    }

    return 0;
  }

  ValueMapping ruggednessFuns[] =
  {
    ruggednessFun<raster2::sint>,
    ruggednessFun<raster2::sreal>,
    0
  };

  int ruggednessSelectFun(ListExpr args)
  {
    int nSelection = -1;
    
    NList type(args);

    if (type.first() == NList(raster2::sint::BasicType()))
    {
      nSelection = 0;
    }
    
    else if (type.first() == NList(raster2::sreal::BasicType()))
    {
      nSelection = 1;
    }

    return nSelection;
  }

  ListExpr ruggednessTypeMap(ListExpr args)
  {
    NList type(args);

    if(type.length() != 1)
    {
      return NList::typeError("one argument required");
    }

    if (type.first() == NList(raster2::sint::BasicType()))
    {
      return NList(raster2::sint::BasicType()).listExpr();
    }

    else if(type.first() == NList(raster2::sreal::BasicType())) 
    {
      return NList(raster2::sreal::BasicType()).listExpr();
    }
    
    return NList::typeError
           ("Expecting an sint or sreal");
  }
}
