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
#ifndef NUMBERUTILS_H
#define NUMBERUTILS_H

#include "../define.h"
#include <stdio.h>
#include <stdlib.h>

class numberUtils {
public:

  static bool hasIntersect(UI64 s0, UI64 e0, UI64 s1, UI64 e1) {
    return inInterval(s0, e0, s1) || inInterval(s0, e0, e1) ||
           inInterval(s1, e1, s0) || inInterval(s1, e1, e0);
  }

  static bool inInterval(UI64 s, UI64 e, UI64 x) {
    return x >= s && x <= e;
  }

  static int randInt(int min, int max) {
    return min + rand() % (max - min + 1);
  }

  static bool
  containsIntAlready(int *list, int listSize, int maxIndexToLook, int number) {
    for (int i = 0; i < listSize && i < maxIndexToLook; i++) {
      if (list[i] == number) return true;
    }
    return false;
  }

  static int *findPermutationOfListIndices(int listLength) {
    return findUniqueRandomInts(0, listLength - 1, listLength);
  }

  static int *findUniqueRandomInts(int min, int max, int amount) {
    int *list = new int[amount];
    for (int i = 0; i < amount; i++) {

      int number = 0;
      do {
        number = randInt(min, max);
      } while (containsIntAlready(list, amount, i, number));
      list[i] = number;
    }
    return list;
  }


};

#endif
