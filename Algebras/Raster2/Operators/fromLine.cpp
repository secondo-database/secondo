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

#include <unordered_set>

#include "fromLine.h"

namespace raster2 {

  void drawLine(
      sbool& sb, std::unordered_set<RasterIndex<2> >& regions,
      RasterIndex<2> from, RasterIndex<2> to);

  int fromLineFun(Word* args, Word& result, int message, Word& local,
      Supplier s)
  {

    
    result = qp->ResultStorage(s);
    sbool* sb = static_cast<sbool*>(result.addr);
    Line* line = static_cast<Line*>(args[0].addr);
    grid2* grid = static_cast<grid2*>(args[1].addr);

    sb->setGrid(*grid);

    bool falsify = false;
    if(qp->GetNoSons(s)==3){
      CcBool* falsifyA = (CcBool*) args[2].addr;
      if(falsifyA->IsDefined() && falsifyA->GetValue()){
        falsify=true;
      }
    }

    if(!line->IsDefined() || grid->getLength()<=0){
        sb->setDefined(false);
        return 0;
    }
    sb->clear();

    int lineLength = line->Size();

    HalfSegment hs;

    std::unordered_set<sbool::index_type> regions;

    for (int i = 0; i < lineLength; i++) {
      line->Get(i, hs);

      Point from = hs.GetLeftPoint();
      Point to = hs.GetRightPoint();
      drawLine(*sb, regions,
          grid->getIndex(from.GetX(), from.GetY()),
          grid->getIndex(to.GetX(), to.GetY()));
    }


    if(falsify){
      const sbool::index_type& region_size = sbool::storage_type::region_size;

      for (std::unordered_set<sbool::index_type>::iterator
              it = regions.begin(), e = regions.end();
           it != e; ++it)
      {
        for (RasterIndex<2> i = *it, e = *it + region_size; i != e;
             i.increment(*it, e))
        {
          if (sbool_helper::isUndefined(sb->get(i))) {
            sb->set(i, false);
          }
        }
      }
    }
    return 0;
  }

  ListExpr fromLineTypeMap(ListExpr args)
  {
    NList type(args);

    if (type.length() != 2 && type.length()!=3) {
       return type.typeError("Expect two ior three arguments.");
    }

    if (type == NList(Line::BasicType(), grid2::BasicType())) {
      return NList(sbool::BasicType()).listExpr();
    }
    if (type == NList(Line::BasicType(), grid2::BasicType(),
        CcBool::BasicType())) {
      return NList(sbool::BasicType()).listExpr();
    }

    return NList::typeError("Expecting a line and a grid2.");
  }

  void drawLine(
      sbool& sb, std::unordered_set<RasterIndex<2> >& regions,
      RasterIndex<2> from, RasterIndex<2> to)
  {
    if (from[0] > to[0]) {
      std::swap(from, to);
    }

    if (from[0] == to[0]) {
      // Line is vertical
      if (from[1] > to[1]) {
        std::swap(from, to);
      }

      for (RasterIndex<2> current = from; current[1] <= to[1]; ++current[1]) {
        sb.set(current, true);
        regions.insert(sbool::storage_type::getRegion(current));
      }
    } else if (from[1] == to[1]) {
      // Line is horizontal
      for (RasterIndex<2> current = from; current[0] <= to[0]; ++current[0]) {
        sb.set(current, true);
        regions.insert(sbool::storage_type::getRegion(current));
      }
    } else {
      int dx = to[0] - from[0];
      int dy = to[1] - from[1];
      int dx2 = 2 * dx;
      int dy2 = 2 * dy;
      int dy2_minus_dx2 = dy2 - dx2;
      int dy2_plus_dx2 = dy2 + dx2;

      if (dy >= 0) {
        // m >= 0
        if (dy <= dx) {
          // 0 <= m <= 1
          int F = dy2 - dx;
          for (RasterIndex<2> current = from; current[0] <= to[0]; ++current[0])
          {
            sb.set(current, true);
            regions.insert(sbool::storage_type::getRegion(current));
            if (F <= 0) {
              F += dy2;
            } else {
              ++current[1];
              F += dy2_minus_dx2;
            }
          }
        }
        else {
          // 1 < m
          int F = dx2 - dy;
          for (RasterIndex<2> current = from; current[1] <= to[1]; ++current[1])
          {
            sb.set(current, true);
            regions.insert(sbool::storage_type::getRegion(current));
            if (F <= 0) {
              F += dx2;
            } else {
              ++current[0];
              F -= dy2_minus_dx2;
            }
          }
        }
      } else {
        // m < 0
        if (dx >= -dy) {
          // -1 <= m < 0
          int F = -dy2 - dx;
          for (RasterIndex<2> current = from; current[0] <= to[0]; ++current[0])
          {
            sb.set(current, true);
            regions.insert(sbool::storage_type::getRegion(current));
            if (F <= 0) {
              F -= dy2;
            } else {
              --current[1];
              F -= dy2_plus_dx2;
            }
          }
        } else {
          // m < -1
          int F = dx2 + dy;
          for (RasterIndex<2> current = from; current[1] >= to[1]; --current[1])
          {
            sb.set(current, true);
            regions.insert(sbool::storage_type::getRegion(current));
            if (F <= 0) {
              F += dx2;
            } else {
              ++current[0];
              F += dy2_plus_dx2;
            }
          }
        }
      }
    }
  }
}
