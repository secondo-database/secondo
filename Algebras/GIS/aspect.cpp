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

#include "aspect.h"
#include "../Raster2/sint.h"
#include "../Raster2/sreal.h"

#ifndef M_PI
const double M_PI = acos( -1.0 );
#endif

namespace GISAlgebra {

  template <typename T>
  int aspectFun(Word* args, Word& result, 
                     int message, Word& local, Supplier s)
  {
    result = qp->ResultStorage(s);

    CcReal* factor = static_cast<CcReal*>(args[1].addr);
    typename T::this_type* s_out =
          static_cast<typename T::this_type*>(result.addr);
    typename T::this_type* s_in =
          static_cast<typename T::this_type*>(args[0].addr);

    raster2::grid2 grid = sint_in->getGrid();

    double zFactor = factor->GetValue();
    double cellsize = grid.getLength();

    sint_out->setGrid(grid);

    Rectangle<2> bbox = sint_in->bbox();

    raster2::RasterIndex<2> from = grid.getIndex(bbox.MinD(0), bbox.MinD(1));
    raster2::RasterIndex<2> to = grid.getIndex(bbox.MaxD(0), bbox.MaxD(1));

    for (raster2::RasterIndex<2> index=from; index < to; 
                                             index.increment(from, to))
    {
	// Mitte
        double e = sint_in->get(index);

        double a = sint_in->get((int[]){index[0] - 1, index[1] + 1});
        if (sint_in->isUndefined(a)){a = e;}
        double b = sint_in->get((int[]){index[0], index[1] + 1});
        if (sint_in->isUndefined(b)){b = e;}
        double c = sint_in->get((int[]){index[0] + 1, index[1] + 1});
        if (sint_in->isUndefined(c)){c = e;}
        double d = sint_in->get((int[]){index[0] - 1, index[1]});
        if (sint_in->isUndefined(d)){d = e;}
        double f = sint_in->get((int[]){index[0] + 1, index[1]});
        if (sint_in->isUndefined(f)){f = e;}
        double g = sint_in->get((int[]){index[0] - 1, index[1] - 1});
        if (sint_in->isUndefined(g)){g = e;}
        double h = sint_in->get((int[]){index[0], index[1] - 1});
        if (sint_in->isUndefined(h)){h = e;}
        double i = sint_in->get((int[]){index[0] + 1, index[1] - 1});
        if (sint_in->isUndefined(i)){i = e;}

        // Delta bestimmen
        double dzdx = ((c + 2*f + i) - (a + 2*d + g)) / (8*cellsize*zFactor);
        double dzdy = ((g + 2*h + i) - (a + 2*b + c)) / (8*cellsize*zFactor);

        // aspect berechnen
        double aspect = atan(dzdx / dzdy) * 180/M_PI;

        if (dzdx > 0)
        {
            aspect = 90 - aspect;
        }
        else if (dzdx < 0)
        {
            aspect = 270 - aspect;
        }
        else if (dzdy > 0)
        {
            aspect = 0;
        }
        else if (dzdx < 0)	
        {
            aspect = 180;
        }
        else
        {
            aspect = raster2::UNDEFINED_REAL;
        }

        sint_out->set(index, aspect);
    }

    return 0;
  }

  ValueMapping aspectFuns[] =
  {
    aspectFun<raster2::sint>,
    aspectFun<raster2::sreal>,
    0
  };

  int aspectSelectFun(ListExpr args)
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

  ListExpr aspectTypeMap(ListExpr args)
  {
    NList type(args);

    if(type.length() != 2)
    {
      return NList::typeError("two arguments required");
    }

    if (type == NList(raster2::sint::BasicType(), CcReal::BasicType())) 
    {
      return NList(raster2::sint::BasicType()).listExpr();
    }

    else if(type == NList(raster2::sreal::BasicType(), 
                          CcReal::BasicType())) 
    {
      return NList(raster2::sreal::BasicType()).listExpr();
    }
    
    return NList::typeError
           ("Expecting an sint or sreal and a double (zfactor).");
  }
}
