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

#include "hillshade.h"
#include "../Raster2/sint.h"
#include "../Raster2/sreal.h"

#ifndef M_PI
const double M_PI = acos( -1.0 );
#endif

namespace GISAlgebra {

  template <typename T>
  int hillshadeFun(Word* args, Word& result, 
                     int message, Word& local, Supplier s)
  {
    result = qp->ResultStorage(s);

    CcReal* factor = static_cast<CcReal*>(args[1].addr);
    CcReal* azimuth = static_cast<CcReal*>(args[2].addr);
    CcReal* angle = static_cast<CcReal*>(args[3].addr);
    typename T::this_type* s_out =
          static_cast<typename T::this_type*>(result.addr);
    typename T::this_type* s_in =
          static_cast<typename T::this_type*>(args[0].addr);

    raster2::grid2 grid = s_in->getGrid();

    double zFactor = factor->GetValue();
    double light_azimuth = azimuth->GetValue();
    double light_angle = angle->GetValue();
    double cellsize = grid.getLength();

    s_out->setGrid(grid);

    Rectangle<2> bbox = s_in->bbox();

    raster2::RasterIndex<2> from = grid.getIndex(bbox.MinD(0), bbox.MinD(1));
    raster2::RasterIndex<2> to = grid.getIndex(bbox.MaxD(0), bbox.MaxD(1));

    for (raster2::RasterIndex<2> index=from; index < to; 
                                             index.increment(from, to))
    {
	// Mitte
        double e = s_in->get(index);
        if (s_in->isUndefined(e)){e = 0;}

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

        // Delta bestimmen
        double dzdx = ((c + 2*f + i) - (a + 2*d + g)) / (8*cellsize*zFactor);
        double dzdy = ((g + 2*h + i) - (a + 2*b + c)) / (8*cellsize*zFactor);

        // Gradwerte umrechnen
        double zenith = (90.0 - light_angle) * M_PI / 180.0;
        double azimuth = (360.0 - light_azimuth + 90);
        if ( azimuth > 360 )
        {
          azimuth = azimuth - 360.0;
        }
        azimuth = azimuth * M_PI / 180.0;

        // Slope berechnen
        double slope = atan(sqrt(dzdx * dzdx + dzdy * dzdy));

        // Aspect berechnen
        double aspect = 0;

        if ( dzdx != 0 )
        {
          aspect = atan2(dzdy, -dzdx);
          if ( aspect < 0 )
          {
            aspect = 2 * M_PI + aspect;
          }
        }

        if ( dzdx == 0 )
        {
          if ( dzdy > 0 )
          {
            aspect = M_PI / 2;
          }
          else if ( dzdy < 0 )
          {
            aspect = 2 * M_PI - M_PI / 2;
          }
          else
          {
            aspect = aspect;
          }
        }

        // hillshade berechnen
        double hillshade = 255.0*((cos(zenith) * cos(slope)) + 
                           (sin(zenith) * sin(slope) * cos(azimuth - aspect)));

        if ( hillshade < 0 )
        {
          hillshade = 0;
        }

        s_out->set(index, hillshade);
    }

    return 0;
  }

  ValueMapping hillshadeFuns[] =
  {
    hillshadeFun<raster2::sint>,
    hillshadeFun<raster2::sreal>,
    0
  };

  int hillshadeSelectFun(ListExpr args)
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

  ListExpr hillshadeTypeMap(ListExpr args)
  {
    NList type(args);

    if(type.length() != 4)
    {
      return NList::typeError("four arguments required");
    }

    if (type == NList(raster2::sint::BasicType(), CcReal::BasicType(),
                      CcReal::BasicType(), CcReal::BasicType())) 
    {
      return NList(raster2::sint::BasicType()).listExpr();
    }

    else if(type == NList(raster2::sreal::BasicType(), CcReal::BasicType(),
                          CcReal::BasicType(), CcReal::BasicType())) 
    {
      return NList(raster2::sreal::BasicType()).listExpr();
    }
    
    return NList::typeError("Expecting an sint or sreal and three double "
                             "(zfactor, azimuth and angle).");
  }
}
