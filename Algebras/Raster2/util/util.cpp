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

#include <cmath>
#include <limits>

#include "util.h"

namespace raster2
{
  namespace util
  {
    bool equals(double d1, double d2, int ulp /* = 4 */) {
      if (d1 == d2) return true;

      if (std::numeric_limits<double>::has_quiet_NaN)
      {
        if (d1 != d1 || d2 != d2) return false;
      }

      if (std::numeric_limits<double>::has_infinity) {
        if (d1 == std::numeric_limits<double>::infinity()  ||
            d2 == std::numeric_limits<double>::infinity()  ||
            d1 == -std::numeric_limits<double>::infinity() ||
            d2 == -std::numeric_limits<double>::infinity())
        {
          return d1 == d2;
        }
      }

      double abs1 = std::abs(d1);
      double abs2 = std::abs(d2);
      double scale = abs1 > abs2 ? abs1 : abs2;

      abs1 /= scale;
      abs2 /= scale;
      double diff = abs1 < abs2 ? abs2 - abs1 : abs1 - abs2;

      return diff <= ulp * std::numeric_limits<double>::epsilon();
    }
  }
}
