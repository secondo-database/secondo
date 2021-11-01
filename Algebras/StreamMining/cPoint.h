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

#ifndef CPOINT_H
#define CPOINT_H

#include "Attribute.h"
#include "StandardTypes.h"

namespace eschbach {
    class cPoint
{
  public:
  cPoint(int id, int coordinates);
  ~cPoint(){};
    //Getter und Setter
    int getDimensions();
    int getCluster();
    void setCluster(int clusterIndex);
    int getId();
    int getVal(int index);

  private: 
    cPoint() {}
    //The Id of a Point used for k-means
    //ids will be created progressively
    //When new elements arrive from a stream
    int pointId;
    //The Id of the CLuster the point is currently
    //assigned to
    int clusterId;
    //The Dimensions of a Point. For int and real
    //Values this will be 1
    int dimensions;
    //The values of a point. Will also only be 1 
    //for int and real values
    std::vector<int> values;

    //Auxiliary
    void clearClusters(); 
    //Will calculate the closest clusterID to assign the 
    //point
    int getClosestClusterId(cPoint point); 




  };
}
#endif