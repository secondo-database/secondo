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

//paragraph [1] Title: [{\Large \bf \begin {center}] [\end {center}}]
//[TOC] [\tableofcontents]

[1] The file is originally provided by Thomas Wolle, and integrated into SECONDO
by Mahmoud Sakr

June, 2009 Mahmoud Sakr

[TOC]

*/


#ifndef OCTREE_PARSER_HEADER
#define OCTREE_PARSER_HEADER


#include <fstream>
#include <vector>
#include <map>
#include <cstdlib>
#include "OctreeElements.h"

class OctreeCell;


class OctreeDatParser{
  public:
    OctreeDatParser(char* inputFileName);
    OctreeDatParser(char* inputFileName, int startCol, int endCol);
    //a column represents a point coordinates (e.g. x:y for 2D coords). The 
    //index of the first column is zero.  
    ~OctreeDatParser();
    ::std::vector<OctreePoint*>* getPointset();
    ::std::map<int, OctreeCell*>* getSkiptree();
    OctreeCell* getOctree();
    ::std::map<int, OctreeCell*>* getSkiptree4Dim();
    ::std::map<int, OctreeCell*>* //OctreeDatParser::
    getSkiptreeFromSet(::std::vector<OctreePoint*>* pointSet, 
        int pointDimensions);
    int activeTrajectoriesCount();
    int getPointCount() {return this->pointData->size();}
  private:
    int getNextNumber();
    double getNextDouble();
    bool getNextCoord(double& res);
    OctreePoint* createPoint();
    OctreePoint* createPoint(int startCol, int endCol); 
    //a column represents a point coordinates (e.g. x:y for 2D coords). The 
    //index of the first column is zero.  
    double calcHalfSideLength();
    void calcSkipTreeCoords(double* res);
    fstream inputFile;
    int activeTrajectories;
    OctreeCell* tmpCell;
    ::std::vector<OctreePoint*> *pointData;
    char* tmpArray;    
    int dimensions, timesteps;
//    double width;
    double *extent;
};

#endif // OCTREE_PARSER_HEADER
