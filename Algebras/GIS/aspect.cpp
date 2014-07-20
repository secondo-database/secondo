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

/*
GIS includes

*/
#include "aspect.h"

/*
Raster2 and Tile includes

*/
#include "../Raster2/sint.h"
#include "../Raster2/sreal.h"
#include "../Tile/t/tint.h"
#include "../Tile/t/treal.h"

/*
Define of PI

*/
#ifndef M_PI
const double M_PI = acos( -1.0 );
#endif

/*
declaration of namespace GISAlgebra

*/
namespace GISAlgebra {

/*
Method aspectFuns: calculated the aspect value for each cell of a sint or 
                   sreal. For each cell the value and the values of the eight 
                   neighbour cells is read. 

Return value: sint or sreal

*/
  template <typename T>
  int aspectFun(Word* args, Word& result, 
                     int message, Word& local, Supplier s)
  {
    result = qp->ResultStorage(s);

/*
factor needed for adjustments between different scale units

*/
    CcReal* factor = static_cast<CcReal*>(args[1].addr);
    typename T::this_type* s_out =
          static_cast<typename T::this_type*>(result.addr);
    typename T::this_type* s_in =
          static_cast<typename T::this_type*>(args[0].addr);

    raster2::grid2 grid = s_in->getGrid();

    double zFactor = factor->GetValue();
    double cellsize = grid.getLength();

    s_out->setGrid(grid);

    Rectangle<2> bbox = s_in->bbox();

    raster2::RasterIndex<2> from = grid.getIndex(bbox.MinD(0), bbox.MinD(1));
    raster2::RasterIndex<2> to = grid.getIndex(bbox.MaxD(0), bbox.MaxD(1));

    for (raster2::RasterIndex<2> index=from; index < to; 
                                             index.increment(from, to))
    {
	// central cell
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

        // calculate delta
        double dzdx = ((c + 2*f + i) - (a + 2*d + g)) / (8*cellsize*zFactor);
        double dzdy = ((g + 2*h + i) - (a + 2*b + c)) / (8*cellsize*zFactor);

        // calculate aspect
        double aspect = atan2(dzdy, -dzdx) * 180/M_PI;

        if (aspect < 0)
        {
            aspect = 90 - aspect;
        }
        else if (aspect > 90)
        {
            aspect = 360 - aspect + 90;
        }
        else
        {
            aspect = 90 - aspect;
        }

        s_out->set(index, aspect);
    }

    return 0;
  }

/*
Method aspectFunsTile: calculated the aspect value for each cell of a tint or 
                       treal. For each cell the value and the values of the 
                       eight neighbour cells is read

Return value: stream of tint or treal

*/
  template <typename T, typename SourceTypeProperties>
  int aspectFunTile(Word* args, Word& result, 
                     int message, Word& local, Supplier s)
  {
    int returnValue = FAILURE;

    T* s_in = static_cast<T*>(args[0].addr);

/*
factor needed for adjustments between different scale units

*/
    CcReal* factor = static_cast<CcReal*>(args[1].addr);

    struct ResultInfo
    {
      double zFactor;
      double fromX;
      double fromY;
      double toX;
      double toY;
      double tileX;
      double tileY;
    };

    int xDimensionSize = TileAlgebra::tProperties<char>::GetXDimensionSize();
    int yDimensionSize = TileAlgebra::tProperties<char>::GetYDimensionSize();

    switch(message)
    {
      case OPEN:
      {
        // initialize the local storage
        ResultInfo* info = new ResultInfo;

        TileAlgebra::Index<2> from;
        TileAlgebra::Index<2> to;

        s_in->GetBoundingBoxIndexes(from, to);
        info->fromX = from[0];
        info->fromY = from[1];
        info->toX = to[0];
        info->toY = to[1];

        info->zFactor = factor->GetValue();

        info->tileX = info->fromX;
        info->tileY = info->fromY;

        local.addr = info;

        return 0;
      }
    
      case REQUEST:
      {
        if(local.addr != 0)
        {
          ResultInfo* info = static_cast<ResultInfo*>(local.addr);

          T* s_out = new T(true);

          if(info->tileX <= info->toX || info->tileY <= info->toY)
          {
            TileAlgebra::tgrid grid;
            s_in->getgrid(grid);
            s_out->SetGrid(grid);

            double cellsize = grid.GetLength();

            bool bHasDefinedValue = false;

            for(int row = info->tileY; row < info->toY; row++)
            {
              for(int column = info->tileX; column < info->toX; column++)
              {
                TileAlgebra::Index<2> index((int[]){column, row});

                // central cell
                double e = s_in->GetValue(index);

                if(SourceTypeProperties::TypeProperties::
                                         IsUndefinedValue(e) == false)
                {
                  bHasDefinedValue = true;

                  double a = s_in->GetValue((int[]){column - 1, row + 1});
                  if (SourceTypeProperties::TypeProperties::
                                            IsUndefinedValue(a)){a = e;}
                  double b = s_in->GetValue((int[]){column, row + 1});
                  if (SourceTypeProperties::TypeProperties::
                                            IsUndefinedValue(b)){b = e;}
                  double c = s_in->GetValue((int[]){column + 1, row + 1});
                  if (SourceTypeProperties::TypeProperties::
                                            IsUndefinedValue(c)){c = e;}
                  double d = s_in->GetValue((int[]){column - 1, row});
                  if (SourceTypeProperties::TypeProperties::
                                            IsUndefinedValue(d)){d = e;}
                  double f = s_in->GetValue((int[]){column + 1, row});
                  if (SourceTypeProperties::TypeProperties::
                                            IsUndefinedValue(f)){f = e;}
                  double g = s_in->GetValue((int[]){column - 1, row - 1});
                  if (SourceTypeProperties::TypeProperties::
                                            IsUndefinedValue(g)){g = e;}
                  double h = s_in->GetValue((int[]){column, row - 1});
                  if (SourceTypeProperties::TypeProperties::
                                            IsUndefinedValue(h)){h = e;}
                  double i = s_in->GetValue((int[]){column + 1, row - 1});
                  if (SourceTypeProperties::TypeProperties::
                                            IsUndefinedValue(i)){i = e;}
         
                  // calculate delta
                  double dzdx = ((c + 2*f + i) - (a + 2*d + g)) / 
                                                 (8*cellsize*info->zFactor);
                  double dzdy = ((g + 2*h + i) - (a + 2*b + c)) / 
                                                 (8*cellsize*info->zFactor);
         
                  // calculate aspect
                  double aspect = atan2(dzdy, -dzdx) * 180/M_PI;

                  if (aspect < 0)
                  {
                      aspect = 90 - aspect;
                  }
                  else if (aspect > 90)
                  {
                      aspect = 360 - aspect + 90;
                  }
                  else
                  {
                      aspect = 90 - aspect;
                  }

                  s_out->SetValue(index, aspect, true);
                }
              }
            }

            // One tile to the right
            info->tileX += xDimensionSize * cellsize;

            // if on right edge
            if(info->tileX >= info->toX)
            {
              // one tile to the top
              info->tileY += yDimensionSize * cellsize;

              // If not top row
              if(info->tileY < info->toY)
              {
                // back to first tile from right
                info->tileX = info->fromX;
              }
            }
                  
            if(bHasDefinedValue == true)
            {
              // return the next stream element
              result.addr = s_out;
              return YIELD;
            }
            else
            {
              delete s_out;
              s_out = 0;

              // always set the result to null before return CANCEL
              result.addr = 0;
              return CANCEL;
            }
          }
        } // if local.addr
        else
        {
          // always set the result to null before return CANCEL
          result.addr = 0;
          return CANCEL;
        }
      } //REQUEST

      case CLOSE:
      {
        if(local.addr != 0)
        {
          ResultInfo* info = static_cast<ResultInfo*>(local.addr);

          if(info != 0)
          {
            delete info;
            local.addr = 0;
          }
        }

        return 0;        
      }

      default:
      {
        assert(false);
      }
    }
 
    return returnValue;
  }

/*
definition of aspectFuns array

*/
  ValueMapping aspectFuns[] =
  {
    aspectFun<raster2::sint>,
    aspectFun<raster2::sreal>,
    aspectFunTile<TileAlgebra::tint, TileAlgebra::tProperties<int>>,
    aspectFunTile<TileAlgebra::treal, TileAlgebra::tProperties<double>>,
    0
  };

/*
Value Mapping 

*/
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

    else if (type.first() == NList(TileAlgebra::tint::BasicType()))
    {
      nSelection = 2;
    }

    else if (type.first() == NList(TileAlgebra::treal::BasicType()))
    {
      nSelection = 3;
    }

    return nSelection;
  }

/*
Type Mapping

*/
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
    
    else if(type == NList(TileAlgebra::tint::BasicType(), 
                          CcReal::BasicType())) 
    {
      return nl->TwoElemList(
             nl->SymbolAtom(Stream<TileAlgebra::tint>::BasicType()),
             nl->SymbolAtom(TileAlgebra::tint::BasicType()));
    }

    else if(type == NList(TileAlgebra::treal::BasicType(), 
                          CcReal::BasicType())) 
    {
      return nl->TwoElemList(
             nl->SymbolAtom(Stream<TileAlgebra::treal>::BasicType()),
             nl->SymbolAtom(TileAlgebra::treal::BasicType()));
    }

    return NList::typeError
           ("Expecting an sint, sreal, tint or treal and a double (zfactor).");
  }
}
