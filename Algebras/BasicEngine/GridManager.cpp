
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

#include "GridManager.h"

using namespace std;

namespace BasicEngine {

/**
1.1 Create a grid with the given parameter

*/
void GridManager::createGrid(BasicEngineControl *basicEngineControl,
                             const std::string &gridName, double startX,
                             double startY, double cellSize, size_t cellsX,
                             size_t cellsY) {

  string gridTable = GRID_TABLE_PREFIX + gridName;
  ConnectionGeneric *dbms_connection = basicEngineControl->getDBMSConnection();

  // 1st step: Create a grid table
  bool creationResult = dbms_connection->createGridTable(gridTable);

  if (!creationResult) {
    BOOST_LOG_TRIVIAL(error) << "Creation of the grid table has failed";
    throw SecondoException("Creation of the grid table has failed");
  }

  dbms_connection->beginTransaction();

  // 2nd step: insert cells
  for (size_t ix = 0; ix < cellsX; ix++) {
    double cellX = startX + (ix * cellSize);

    for (size_t iy = 0; iy < cellsY; iy++) {
      double cellY = startY + (iy * cellSize);

      dbms_connection->insertRectangle(gridTable, cellX, cellY, cellSize,
                                       cellSize);
    }
  }

  // 3rd step: Share the grid will all workers
  bool shareResult = basicEngineControl->shareTable(gridTable);

  if (!shareResult) {
    BOOST_LOG_TRIVIAL(error) << "Unable to share grid with workers";
    throw SecondoException("Unable to share grid with workers");
  }

  dbms_connection->commitTransaction();
}

/**
1.2 Delete the grid with the given name

*/
void GridManager::deleteGrid(BasicEngineControl *basicEngineControl,
                             std::string &gridName) {

  string gridTable = GRID_TABLE_PREFIX + gridName;
  string sqlQuery = "DROP TABLE " + gridTable;

  // Delete grid on master
  ConnectionGeneric *dbms_connection = basicEngineControl->getDBMSConnection();
  bool masterResult = dbms_connection->sendCommand(sqlQuery, true);

  if (!masterResult) {
    BOOST_LOG_TRIVIAL(error) << "Unable to delete grid on master";
    throw SecondoException("Unable to delete grid on master");
  }

  bool workerResult = basicEngineControl->mcommand(sqlQuery);

  if (!workerResult) {
    BOOST_LOG_TRIVIAL(error) << "Unable to delete grid on worker";
    throw SecondoException("Unable to delete grid on worker");
  }
}

} // namespace BasicEngine