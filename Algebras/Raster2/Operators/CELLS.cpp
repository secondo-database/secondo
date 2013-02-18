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

#include "CELLS.h"

#include "../util/parse_error.h"
#include "../util/types.h"

#include <NestedList.h>
#include <Operator.h>

namespace raster2
{
  ListExpr cellsTypeMap(ListExpr args)
  {
    NList types = args;
    std::string attribute_type_name;

    try {
      std::ostringstream oss;
      if (types.length() < 1) {
        throw util::parse_error("CELLS expects at least one argument.");
      }

      std::string raster_type = types.first().convertToString();
      if (!(util::isSType(raster_type) || util::isMSType(raster_type))) {
        throw util::parse_error(
          "CELLS expects sT or msT as first argument, got " + raster_type + "."
        );
      }

      attribute_type_name = util::getValueBasicType(raster_type);
      if (attribute_type_name.empty()) {
        throw util::parse_error(
          "CELLS cannot determine value type of " + raster_type + "."
        );
      }

    } catch (util::parse_error& e) {
      return NList::typeError(e.what());
    }

    return NList(
        NList("rel"),
        NList(
            NList("tuple"),
            NList(NList("Elem"), NList(attribute_type_name)).enclose()
        )
    ).listExpr();
  }
}
