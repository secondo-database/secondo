/*
 ----
 This file is part of SECONDO.

 Copyright (C) 2004-2007, University in Hagen, Department of Computer Science,
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

*/

#include "TinForRoutePlanning.h"
#include "LCompose.h"

using namespace tin;

/*
Implementation

*/
tin::Tin *
routeplanningalgebra::TinForRoutePlanning::createTinFrom3DCoordinates(
    double coordinates[][3], unsigned long number, const int partSize,
    const bool isTemp) {
  assert(coordinates);

  Vertex v, priorv;

  VertexContainerSet *vc = new VertexContainerSet(
      VERTEX_CONTAINER_BIG_SIZE);

  Tin *resultTin = new Tin(isTemp);

  resultTin->config.maxSizePart = partSize;

  DEBUG << "Attempting to make tin from " << number << " points" << "\r\n";

  resultTin->setMemoryStateGradual();

  for (unsigned long i = 0; i < number; i++) {
    v.setX(coordinates[i][0]);
    v.setY(coordinates[i][1]);
    v.setZ(coordinates[i][2]);

    if (i > 0) {
      if (v.getY() > priorv.getY())
        throw std::runtime_error(E_CREATETIN_VM);

      if (v.getY() != priorv.getY()
          && resultTin->calculateNoRows(*vc) >= 1) {
        resultTin->triangulateSection(vc);
        vc = new VertexContainerSet(VERTEX_CONTAINER_BIG_SIZE);
      }
    }

    vc->insertVertex_p(&v);
    priorv = v;

    cout.precision(8);
    DETAIL << "Point (" << v.getX() << ", " << v.getY() << ", " << v.getZ()
           << ") added to Tin." << "\r\n";
  }

  LOGP
  resultTin->triangulateSection(vc);
  resultTin->finishTriangulation();

  resultTin->setDefined();
  DEBUG << "Proper Tin created." << "\r\n";

  LOGP
  return resultTin;
}
