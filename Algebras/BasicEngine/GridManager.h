/*
----
This file is part of SECONDO.

Copyright (C) 2021,
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
#ifndef _GRID_MANAGER_H_
#define _GRID_MANAGER_H_

#include "BasicEngineControl.h"
#include "ConnectionGeneric.h"

namespace BasicEngine {

class GridManager {

public:
  /**
  1.1 Create a grid with the given parameter

  */
  static void createGrid(BasicEngineControl *basicEngineControl,
                         const std::string &gridName, double startX,
                         double startY, double cellSize, size_t cellsX,
                         size_t cellsY);

  /**
  1.2 Delete the given grid

  */
  static void deleteGrid(BasicEngineControl *basicEngineControl,
                         std::string &gridName);

  /**
  1.3 The prefix of the grid tables

  */
   inline static const std::string GRID_TABLE_PREFIX = "grid_";
};

} // namespace BasicEngine

#endif