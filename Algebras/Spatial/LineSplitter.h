/*
----
This file is part of SECONDO.

Copyright (C) 2017, 
University in Hagen, 
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

//[_] [\_]

*/

#ifndef LineSplitter_H
#define LineSplitter_H

#include "LineT.h"

template<template<typename T>class Array>
class LineSplitter {
 public:
  LineSplitter(LineT<Array>* line, bool ignoreCriticalPoints, bool allowCycles,
               PointsT<Array>* points = 0);

  ~LineSplitter();

  LineT<Array>* NextLine(std::list<Point> *pointlist = 0);
  
 private:
  bool isCriticalPoint(int index);

  bool* used;
  LineT<Array>* theLine;
  int lastPos;
  int size;
  bool ignoreCriticalPoints;
  PointsT<Array>* points;
  bool allowCycles;
};

#include "LineSplitterImpl.h"


#endif

