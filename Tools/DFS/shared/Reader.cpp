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
#include "io.h"
#include <stdio.h>
#include <stdlib.h>

using namespace dfs;
using namespace dfs::io::file;

Reader::Reader(const Str &filename) {
  this->filename = filename;
}

bool Reader::open() {
  char *s = filename.cstr();
  fp = fopen(s, "r");
  delete[] s;
  return fp;
}

void Reader::close() {
  fclose(fp);
}

int Reader::readInt() {
  int i = 0;
  fread(&i, sizeof(int), 1, fp);
  return i;
}

Str Reader::readStr(int len) {
  char *buf = new char[len];
  fread(buf, len, 1, fp);
  Str s = Str(buf, len);
  delete[] buf;
  return s;
}

Str Reader::readWithLengthInfo(short lenlen) {
  char *lengthBuf = new char[lenlen];
  fread(lengthBuf, lenlen, 1, fp);
  int realStrLen = atoi(lengthBuf);
  delete[] lengthBuf;
  char *buf = new char[realStrLen];
  fread(buf, realStrLen, 1, fp);
  Str s = Str(buf, realStrLen);
  delete[] buf;
  return s;
}
