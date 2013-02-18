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

#ifndef RASTER2_GRID3_H
#define RASTER2_GRID3_H

#include "grid2.h"
#include "DateTime.h"
#include "RasterStorage.h"

namespace raster2
{

class grid3 : public grid2
{
  public:
  typedef RasterIndex<3> index_type;
  typedef RasterRegion<3> region_type;

  /*
  constructors
  
  */
  grid3();
  
  grid3(const double& rOriginX,
          const double& rOriginY,
          const double& rLength,
          const datetime::DateTime& rDuration);

  /*
  destructor
  
  */
  
  virtual ~grid3();
  
  /*
  getter
  
  */
  
  const datetime::DateTime& getDuration() const;

  index_type getIndex(double, double, double) const;
  region_type getRegion(const Rectangle<3>&) const;
  Rectangle<3> getCell(const index_type&) const;
  Rectangle<3> getBBox(const region_type&) const;
  Rectangle<3> getBBox(const index_type&, const index_type&) const;

  /*
  The following functions are used to integrate the ~grid3~ datatype
  into secondo.

  */

  static std::string BasicType();
  static void* Cast(void* pVoid);
  static Word Clone(const ListExpr typeInfo,
                    const Word& w);
  static void Close(const ListExpr typeInfo,
                    Word& w);
  static Word Create(const ListExpr typeInfo);
  static void Delete(const ListExpr typeInfo,
                     Word& w);
  static TypeConstructor getTypeConstructor();
  static Word In(const ListExpr typeInfo,
                 const ListExpr instance,
                 const int errorPos,
                 ListExpr& errorInfo,
                 bool& correct);
  static bool KindCheck(ListExpr type, ListExpr& errorInfo);
  static ListExpr Out(ListExpr typeInfo, Word value);
  static ListExpr Property();
  static int SizeOfObj();

  private:
  /*
  members
   
  */ 
  
  datetime::DateTime m_Duration;
};

}

#endif // RASTER2_GRID3_H
 
