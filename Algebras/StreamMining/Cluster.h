/*
----
This file is part of SECONDO.

Copyright (C) 2004, University in Hagen, Department of Computer Science,
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

#ifndef CLUSTER_H
#define CLUSTER_H

#include "Attribute.h"
#include "StandardTypes.h"
#include "cPoint.h"

namespace eschbach {
    class Cluster
{
  public:
  Cluster(int id, cPoint centroid);
  ~Cluster(){};
    //Getter und Setter
    int getId();
    cPoint getPoint(int index);
    std::vector<cPoint> getAllPoints();
    int getSize();
    double getCentroidByPos(int pos);
    void setCentroidByPos(int pos, double val);

    //Auxiliary
    //Add a point to this cluster
    void addPoint(cPoint point);
    //remove a point from this cluster
    bool removePoint(int pointId);
    //clear cluster
    void removeAllPoints();


  private: 
    Cluster(){}
    //a Clusters Id
    int clusterId;
    //saving the value of the current cluster centroid for 
    //calculations
    std::vector<double> clusterCentroid; 
    //Points belonging to this cluster
    std::vector<cPoint> points; 
  };
}
#endif