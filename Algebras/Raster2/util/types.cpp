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

#include <StandardTypes.h>

#include "types.h"

#include "../sint.h"
#include "../sreal.h"
#include "../sbool.h"
#include "../sstring.h"
#include "../msint.h"
#include "../msreal.h"
#include "../msbool.h"
#include "../msstring.h"


namespace raster2
{
  namespace util
  {
    bool isMSType(const std::string& type) {
      return type == msint::BasicType()
          || type == msreal::BasicType()
          || type == msbool::BasicType()
          || type == msstring::BasicType();
    }

    bool isSType(const std::string& type) {
      return type == sint::BasicType()
          || type == sreal::BasicType()
          || type == sbool::BasicType()
          || type == sstring::BasicType();
    }

    std::string getValueBasicType(const std::string& type) {
      if (type == sint::BasicType() || type == msint::BasicType()) {
        return CcInt::BasicType();
      }
      if (type == sreal::BasicType() || type == msreal::BasicType()) {
        return CcReal::BasicType();
      }
      if (type == sbool::BasicType() || type == msbool::BasicType()) {
        return CcBool::BasicType();
      }
      if (type == sstring::BasicType() || type == msstring::BasicType()) {
        return CcString::BasicType();
      }
      return "";
    }

    std::string getSpatialBasicType(const std::string& type) {
      if (type == CcInt::BasicType()) {
        return sint::BasicType();
      }
      if (type == CcReal::BasicType()) {
        return sreal::BasicType();
      }
      if (type == CcBool::BasicType()) {
        return sbool::BasicType();
      }
      if (type == CcString::BasicType()) {
        return sstring::BasicType();
      }
      return "";
    }

    std::string getMovingSpatialBasicType(const std::string& type) {
      if (type == CcInt::BasicType()) {
        return msint::BasicType();
      }
      if (type == CcReal::BasicType()) {
        return msreal::BasicType();
      }
      if (type == CcBool::BasicType()) {
        return msbool::BasicType();
      }
      if (type == CcString::BasicType()) {
        return msstring::BasicType();
      }
      return "";
    }
  }
}
