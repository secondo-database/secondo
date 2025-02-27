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

#ifndef RASTER2_ISREAL_H
#define RASTER2_ISREAL_H

#include "istype.h"
#include "sreal.h"

namespace raster2
{
  typedef istype<double> isreal;
  
  template <>
  struct istype_helper<double>
  {
    typedef isreal implementation_type;
    typedef sreal spatial_type;
    typedef CcReal wrapper_type;
    static std::string name();
    
    static bool check(const NList& nl)
    {
      return (nl.isReal() || nl.isInt());
    }
    
    static double parse(const NList& nl)
    {
      double value = 0.0;
      
      if(nl.isInt())
      {
        value = nl.intval();
      }
      
      else
      {
        value = nl.realval();
      }
      
      return value;
    }
    
    static bool isUndefined(const double& rdouble)
    {
      return rdouble == UNDEFINED_REAL();
    }
    
    static double getUndefined()
    {
      return UNDEFINED_REAL();
    }
    
    static std::string BasicType()
    {
      return CcReal::BasicType();
    }
    
    static ListExpr listExpr(const double& rdouble)
    {
      return NList(rdouble).listExpr();
    }
  };
}

#endif // RASTER2_ISREAL_H
