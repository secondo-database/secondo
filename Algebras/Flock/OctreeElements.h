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

#ifndef OCTREE_ELEMENTS_HEADER
#define OCTREE_ELEMENTS_HEADER

#include <map>
#include <vector>
#include <gmpxx.h>

#define __epsilon 0.00000001
const int OCTREE_POINT   = 0;
const int OCTREE_CELL    = 1;
const int OCTREE_ELEMENT = 2;

class OctreeCell;
class OctreePoint;

   
class OctreeElement {
public:
  OctreeElement(int dimensions, double* coordinates);
  virtual ~OctreeElement();
  virtual int getType();
  double* getCoordinates();
  int getDimensions();
  OctreeElement* getParent();
  void printCoordinates();
  void setCoordinates(double* coordinates);
  void setParent(OctreeCell* parent);
  virtual OctreeElement* getSkipTreeCopy(){return 0;};
  virtual void unmarkReported(){};
  virtual int boxedRangeQueryCounting(double* boxCoords, 
      double boxHalfSideLength, double halfSideLengthError){return 0;};
      virtual int exactRangeQuery2DimSteps(double* coords, double radius, 
          double radiusError){return 0;};      
protected:
  double* coordinates;
  int dimensions;
  OctreeCell* parent;    
};


class OctreePoint : public OctreeElement {
public:
  OctreePoint(int dimensions, double* coordinates, int identifier);
  ~OctreePoint(){};
  int getType();
  int getIdentifier() const {return this->identifier;};
  OctreeElement* getSkipTreeCopy();
  void markReported();
  void unmarkReported();
  bool isReported();
  bool isContained(double* coordinates, double halfSideLength);
  int boxedRangeQueryCounting(double* boxCoords, double boxHalfSideLength, 
      double halfSideLengthError);
  int exactRangeQuery2DimSteps(double* coords, double radius, 
      double radiusError);
  void increaseValue();      
private:
  int pointsReported;
  int pointsContained;
  int identifier;
};


class OctreeCell : public OctreeElement {
public:
  OctreeCell(int dimensions, double* coordinates, double halfSideLength);
  ~OctreeCell();
  int getType();
  void insert(OctreeElement* element);
  double getHalfSideLength(){return halfSideLength;};
  bool covers(OctreeElement* element);
  OctreeElement* getSkipTreeCopy();
  int getPointsContained(){return this->pointsContained;};
  OctreeElement* find(double* coordinates);
  int boxedRangeQueryCounting(double* boxCoords, double boxHalfSideLength, 
      double halfSideLengthError);
  int boxedRangeQueryReport(double* boxCoords, double boxHalfSideLength, 
      double halfSideLengthError, ::std::vector<OctreePoint*>* points);
  bool isCritical(double* boxCoords, 
      double boxHalfSideLength, mpf_class* vol, OctreeCell **successor);
  void printCoordinates();
  void deletePointData();
  int markReported(int amount);
  void unmarkReported();
  mpf_class* getBoxIntersection(double* boxCoords, 
      double boxHalfSideLength);
  int exactRangeQuery2DimSteps(double* coords, 
      double radius, double radiusError);      
  std::map<unsigned long long, OctreeElement*>* content;
  int pointsContained;
  void printAll();
private:
  OctreeCell* findCriticalSquare__down(double* boxCoords, 
      double boxHalfSideLength, mpf_class* vol);
  OctreeCell* findCriticalSquare__up(double* boxCoords, 
      double boxHalfSideLength, mpf_class* vol);
  int pointsReported;
  double halfSideLength;
  OctreeCell* upperTree;
  OctreeCell* lowerTree;        
  OctreeCell* largestInterestingSquare(OctreeElement* a, 
      OctreeElement* b);
};

#endif // OCTREE_ELEMENTS_HEADER
