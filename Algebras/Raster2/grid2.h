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

#ifndef RASTER2_GRID2_H
#define RASTER2_GRID2_H

#include <utility>

#include <AlgebraTypes.h>
#include <StandardTypes.h>
#include <NestedList.h>
#include <HalfSegment.h>

#include "Defines.h"
#include "RasterStorage.h"

namespace raster2 {
/*
The ~grid2~ class provides information about the origin of a raster object and
the size of a raster cell.

*/
  class grid2
  {
    public:

      typedef RasterIndex<2> index_type;
      typedef RasterRegion<2> region_type;

      grid2();

      grid2(double ax, double ay, double alength);
      grid2(const grid2& rgrid2);

      ~grid2();
      
      double getOriginX() const;
      double getOriginY() const;
      double getLength() const;

      std::ostream& print(ostream& os) const;
      void Reset();

      index_type getIndex(double xcoord, double ycoord) const;
      region_type getRegion(const Rectangle<2>& bbox) const;
      Rectangle<2> getCell(const index_type& i) const;
      Rectangle<2> getBBox(const region_type& r) const;
      Rectangle<2> getBBox(const index_type& from, const index_type& to) const;

/*
The function ~intersect~ calculates the edges of a cell, over which a line
(given by start end endpoint) enters and leaves a cell.

*/
      std::pair<index_type, index_type> intersect
          (const HalfSegment&, const index_type&) const;

      static const index_type None;
/*
The following functions are used to integrate the ~grid2~ datatype into secondo.

*/

      static TypeConstructor getTypeConstructor();

      static std::string BasicType();

      static Word In(const ListExpr typeInfo, const ListExpr instance,
                     const int errorPos, ListExpr& errorInfo,
                     bool& correct);

      static ListExpr Out(ListExpr typeInfo, Word value);

      static Word Create(const ListExpr typeInfo);

      static void Delete(const ListExpr typeInfo, Word& w);

      static void Close(const ListExpr typeInfo, Word& w);

      static Word Clone(const ListExpr typeInfo, const Word& w);

      static bool KindCheck(ListExpr type, ListExpr& errorInfo);

      static void* Cast(void* placement);

      static int SizeOfObj();

      static ListExpr Property();

    protected:
      double x;
      double y;

      double length;
  };

  std::ostream& operator<<(std::ostream& os, const grid2& grid);
}



#endif // RASTER2_GRID2_H
