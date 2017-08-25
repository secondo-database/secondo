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

#ifndef RASTER2_CREATE_HGT_INDEX_H
#define RASTER2_CREATE_HGT_INDEX_H

#include <NList.h>
#include "../stype.h"
#include "../sint.h"
#include "../../Hash/HashAlgebra.h"

namespace raster2 {
  ListExpr createHgtIndexTM(ListExpr args);
  int createHgtIndexVM(Word* args, Word& result, int message, Word& local,
                       Supplier s);
  struct createHgtIndexInfo : OperatorInfo {
    createHgtIndexInfo() {
      name      = "createHgtIndex";
      signature = sint::BasicType() + " -> " + Hash::BasicType();
      syntax    = "createHgtIndex( _ )";
      meaning   = "Creates a hash file from an sint object. All tiles with "
                  "a certain value can then be retrieved efficiently. An "
                  "optional integer can be specified to use rougher values, "
                  "e.g., 220 for all values between 220 and 229.";
    }
  };
}
#endif /* #ifndef RASTER2_CREATE_HGT_INDEX_H */
