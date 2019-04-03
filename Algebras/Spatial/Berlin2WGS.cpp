
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
//[_] [\_]

*/

#include "Berlin2WGS.h"

using namespace std;

/*
This file is applied to convert BBBike coordinates into WGS84 coordinates.

The formula is taken from the BBBike sources.

http://bbbike.sourceforge.net/index.de.html

*/
Berlin2WGS::Berlin2WGS() {
  x0 = -780761.760862528;
  x1 = 67978.2421158527;
  x2 = -2285.59137120724;
  y0 = -5844741.03397902;
  y1 = 1214.24447469596;
  y2 = 111217.945663725;
}

void Berlin2WGS::convert(const Point* source, Point* result) {
  result->Set(((source->GetX()-x0)*y2-(source->GetY()-y0)*x2)/(x1*y2-y1*x2),
              ((source->GetX()-x0)*y1-(source->GetY()-y0)*x1)/(x2*y1-x1*y2));
}

void Berlin2WGS::convert(const Rectangle<2>* source, Rectangle<2>* result) {
  double minX = source->MinD(0);
  double minY = source->MinD(1);
  double maxX = source->MaxD(0);
  double maxY = source->MaxD(1);
  Point src1(true, minX, minY), src2(true, maxX, maxY), res1(true), res2(true);
  convert(&src1, &res1);
  convert(&src2, &res2);
  double *min = new double[2];
  min[0] = res1.GetX();
  min[1] = res1.GetY();
  double *max = new double[2];
  max[0] = res2.GetX();
  max[1] = res2.GetY();
  if (res1.IsDefined() && res2.IsDefined()) {
    result->Set(true, min, max);
  }
  else {
    result->SetDefined(false);
  }
  delete[] min;
  delete[] max;
}

template<template<typename T>class Array>
void Berlin2WGS::convert(const LineT<Array>* source, LineT<Array>* result) {
  result->Clear();
  HalfSegment src;
  result->Resize(source->Size());
  for (int i = 0; i < source->Size(); i++) {
    source->Get(i, src);
    HalfSegment res = b2wgs(src);
    result->Put(i, res);
  }
}

template<template<typename T>class Array>
void Berlin2WGS::convert(const RegionT<Array>* source, RegionT<Array>* result) {
  result->Clear();
  HalfSegment src;
  result->Resize(source->Size());
  for (int i = 0; i < source->Size(); i++) {
    source->Get(i, src);
    HalfSegment res = b2wgs(src);
    result->Put(i, res);
  }
}

pair<double, double> Berlin2WGS::b2wgs(const double& x, const double& y) {
  return make_pair(((x-x0)*y2-(y-y0)*x2)/(x1*y2-y1*x2),
                   ((x-x0)*y1-(y-y0)*x1)/(x2*y1-x1*y2));
}

HalfSegment Berlin2WGS::b2wgs(const HalfSegment& source) {
  Point lp = source.GetLeftPoint();
  Point rp = source.GetRightPoint();
  Point newlp(true), newrp(true);
  convert(&lp, &newlp);
  convert(&rp, &newrp);
  HalfSegment result(source);
  result.Set(source.IsLeftDomPoint(), newlp, newrp);
  return result;
}


// Instantiations


template void Berlin2WGS::convert<DbArray>(const LineT<DbArray>* source, 
                                           LineT<DbArray>* result);
template void Berlin2WGS::convert<MMDbArray>(const LineT<MMDbArray>* source, 
                                             LineT<MMDbArray>* result);

template void Berlin2WGS::convert<DbArray>(const RegionT<DbArray>* source, 
                                           RegionT<DbArray>* result);
template void Berlin2WGS::convert<MMDbArray>(const RegionT<MMDbArray>* source, 
                                             RegionT<MMDbArray>* result);
