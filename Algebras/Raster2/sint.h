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

#ifndef RASTER2_SINT_H
#define RASTER2_SINT_H

#include "stype.h"

namespace raster2
{
  typedef stype<int> sint;
  
  template <>
  struct stype_helper<int>
  {
    typedef sint implementation_type;
    typedef CcInt wrapper_type;
    typedef MInt moving_type;
    typedef UInt unit_type;
    static const char* name;
    static bool check(const NList& nl) { return nl.isInt(); }
    static int parse(const NList& nl) { return nl.intval(); }
    static NList print(const int& i)
        { return isUndefined(i) ? NList(Symbol::UNDEFINED()) : NList(i); }
    static bool isUndefined(const int& t) { return t == UNDEFINED_INT; }
    static int getUndefined() { return UNDEFINED_INT; }
    static std::string BasicType() { return CcInt::BasicType(); }
    static wrapper_type wrap(const int& t) {
        return CcInt(!isUndefined(t), t);
    }
    static int unwrap(const CcInt& i) {
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
  template<> inline void swap<raster2::sint>
    (raster2::sint& a, raster2::sint& b)
  {
      raster2::swap(a, b);
  }
}

#endif
