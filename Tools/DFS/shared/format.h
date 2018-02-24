/*
----
This file is part of SECONDO.
Realizing a simple distributed filesystem for master thesis of stephan scheide

Copyright (C) 2015,
Faculty of Mathematics and Computer Science,
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
----


//[$][\$]

*/
#ifndef FORMAT_H
#define FORMAT_H

#include "str.h"

using namespace dfs;

namespace dfs {

  namespace format {

    /**
     * helps formatting integer as mib, kib, B
     * @param f
     * @return
     */
    Str sizei(int f) {
      if (f > 1024 * 1024) {
        return Str(f / (1024 * 1024)).append(" MiB");
      } else if (f > 1024) {
        return Str(f / 1024).append(" KiB");
      } else {
        return Str(f).append(" B");
      }
    }

  }
}

#endif

