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
#include "maschine.h"
#include "str.h"
#include "checksum.h"
#include <stdio.h>

using namespace dfs;


Str dfs::Maschine::volatileId(const Str &appendix) {

  dfs::checksum::crc64 c;

  Str input;
  FILE *p = popen("ip link | grep link", "r");

  if (p) {
    char buffer[1024];
    int read = fread(buffer, 1, 1024, p);
    if (read > 0) input = input.append(Str(buffer, read));
    fclose(p);
  }

  CStr cs(input);
  UI64 ii = c.checksum((UI8 *) cs.cstr(), input.len());

  return Str::toHex((const char *) &ii, 8).append(appendix);

}
