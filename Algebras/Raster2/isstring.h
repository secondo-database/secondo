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

#ifndef RASTER2_ISSTRING_H
#define RASTER2_ISSTRING_H

#include "istype.h"
#include "sstring.h"

namespace raster2
{
  typedef istype<string> isstring;
  
  template <>
  struct istype_helper<string>
  {
    typedef isstring implementation_type;
    typedef sstring spatial_type;
    typedef CcString wrapper_type;
    static const char* name;
    
    static bool check(const NList& nl)
    {
      return nl.isString();
    }
    
    static string parse(const NList& nl)
    {
      return nl.str();
    }
    
    static bool isUndefined(const string& rString)
    {
      return rString == UNDEFINED_STRING;
    }
    
    static string getUndefined()
    {
      return UNDEFINED_STRING;
    }
    
    static std::string BasicType()
    {
      return CcString::BasicType();
    }
    
    static ListExpr listExpr(const string& rString)
    {
      return NList(rString).listExpr();
    }
  };
}

#endif // RASTER2_ISSTRING_H
