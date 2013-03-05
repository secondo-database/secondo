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

#ifndef RASTER2_MSREAL_H
#define RASTER2_MSREAL_H

#include "mstype.h"
#include "sreal.h"

namespace raster2 
{
    typedef mstype<double> msreal;
 
    template <>
    struct mstype_helper<double> 
    {
        typedef msreal implementation_type;
        typedef MReal moving_type;
        typedef CcReal wrapper_type;
        typedef UReal unit_type;
        typedef sreal spatial_type;
        static const char* name;
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
        
        static NList print(const double& d)
            { return isUndefined(d) ? NList(Symbol::UNDEFINED()) : NList(d); }
        static bool isUndefined(const double& t) { return t != t; }
        static double getUndefined() { return UNDEFINED_REAL; }
        static std::string BasicType() { return CcReal::BasicType(); }
        static wrapper_type wrap(const double& t) {
            return CcReal(!isUndefined(t), t);
        }
        static double unwrap(const CcReal& i) {
            if (i.IsDefined()) {
              return i.GetValue();
            } else {
              return getUndefined();
            }
        }
    };
}

namespace std
{
    template<> inline void swap<raster2::msreal>
        (raster2::msreal& a, raster2::msreal& b)
    {
        raster2::swap(a, b);
    }
}

#endif
