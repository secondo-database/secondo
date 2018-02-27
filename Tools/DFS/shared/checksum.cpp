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
#include "checksum.h"
#include <iostream>

using namespace std;
using namespace dfs::checksum;

UI64 generator = 0xC96C5795D7870F42;

void crc64::generateLookupTable() {

  for (short i = 0; i < 256; i++) {

    UI64 crc = i;

    for (short j = 0; j < 8; j++) {
      if (crc & 1) {
        crc >>= 1;
        crc ^= generator;
      } else {
        crc >>= 1;
      }
    }
    table[i] = crc;
  }
}

UI64 crc64::checksum(UI8 *buf, int len) {
  UI64 crc = 0;
  for (int i = 0; i < len; i++) {
    UI8 index = buf[i] ^crc;
    UI64 lookup = table[index];
    crc >>= 8;
    crc ^= lookup;
  }
  return crc;
}
