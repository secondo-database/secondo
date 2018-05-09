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
#include "FigureSystem.h"

using namespace dfs;

const char *figures = "0123456789abcdefghijklmnopqrstuvwxyz";

FigureSystem::FigureSystem(short base, short length) {
  this->base = base;
  this->length = length;
  values = new char[length];
  for (int i = 0; i < length; i++) values[i] = 0;
}

FigureSystem::~FigureSystem() {
  delete[] values;
}

void FigureSystem::inc() {
  values[0]++;
  refactor();
}

void FigureSystem::inc(int value) {
  if (value == 0) return;
  values[0] += value;
  refactor();
}


void FigureSystem::refactor() {
  for (int i = 0; i < length; i++) {
    char v = values[i];
    char x = v / base;
    char n = v % base;
    if (x > 0) {
      if (i == length - 1) throw "overflow";
      values[i + 1] += x;
      values[i] = n;
    }
  }
}

Str FigureSystem::toStr() {
  char tmp[base];
  for (int i = 0; i < length; i++) {
    char v = values[i];
    tmp[length - i - 1] = figures[v];
  }
  return Str(tmp, length);
}

void FigureSystem::fromStr(const Str &strValue) {
  //000A --> values[0]=10
  if (strValue.len() != length) throw "invalid length of figure system";
  int max = length - 1;
  for (int i = max; i >= 0; i--) {

    char figureChar = strValue[i];
    int figureValue = 0;

    for (int j = 0; j < base; j++) {
      if (figures[j] == figureChar) {
        figureValue = j;
        break;
      }
    }

    values[length - i - 1] = figureValue;
  }
}

void FigureSystem::resetToZero() {
  for (int i = 0; i < length; i++) values[i] = 0;
}
