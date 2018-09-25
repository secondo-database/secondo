/*

This file is part of SECONDO.

Copyright (C) 2018,
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

*/

#ifndef BINARYTUPLE_H_
#define BINARYTUPLE_H_

#include <cstdint>
#include <iostream>

namespace csj {

// the goal is to use exactly 64 bytes, since cache-line size is 64 bytes
  struct binaryTuple {
    
    binaryTuple() :
      blockNum{0}, row{0},
      xMin{0}, xMax{0},
      yMin{0}, yMax{0},
      zMin{0}, zMax{0} {}
      
    uint64_t blockNum; // exactly 8 Bytes, beginnt always with 1
    uint64_t row; // exactly 8 Bytes
    double xMin; // exactly 8 Bytes
    double xMax; // exactly 8 Bytes
    double yMin; // exactly 8 Bytes
    double yMax; // exactly 8 Bytes
    double zMin; // exactly 8 Bytes
    double zMax; // exactly 8 Bytes
  };

  bool tuplesAreEqual(binaryTuple t1, binaryTuple t2) {
    return ((t1.blockNum == t2.blockNum) && (t1.row == t2.row));
  }
} // end of namespace csj

#endif // BINARYTUPLE_H_
