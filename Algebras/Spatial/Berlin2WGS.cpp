
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

void Berlin2WGS::convert(const Points* source, Points* result) {
  result->Clear();
  Point src, res;
  for (int i = 0; i < source->Size(); i++) {
    source->Get(i, src);
    convert(&src, &res);
    *result += res;
  }
}

void Berlin2WGS::convert(const Line* source, Line* result) {
  result->Clear();
  HalfSegment src;
  result->Resize(source->Size());
  for (int i = 0; i < source->Size(); i++) {
    source->Get(i, src);
    HalfSegment res = b2wgs(src);
    result->Put(i, res);
  }
}

void Berlin2WGS::convert(const Region* source, Region* result) {
  result->Clear();
  HalfSegment src;
  result->Resize(source->Size());
  for (int i = 0; i < source->Size(); i++) {
    source->Get(i, src);
    HalfSegment res = b2wgs(src);
    result->Put(i, res);
  }
}

void Berlin2WGS::convert(const IPoint* source, IPoint* result) {
  Point x;
  convert(&source->value, &x);
  *result = IPoint(source->instant, x);
}

void Berlin2WGS::convert(const UPoint* source, UPoint* result) {
  Point x, y;
  convert(&source->p0, &x);
  convert(&source->p1, &y);
  *result = UPoint(source->timeInterval, x, y);
}

void Berlin2WGS::convert(const MPoint* source, MPoint* result) {
  UPoint src(1), res(1);
  for (int i = 0; i < source->GetNoComponents(); i++) {
    source->Get(i, src);
    convert(&src, &res);
    result->Add(res);
  }
}


pair<double, double> Berlin2WGS::b2wgs(const double& x, const double& y) {
  return make_pair(((x-x0)*y2-(y-y0)*x2)/(x1*y2-y1*x2),
                   ((x-x0)*y1-(y-y0)*x1)/(x2*y1-x1*y2));
}

HalfSegment Berlin2WGS::b2wgs(const HalfSegment& source) {
  Point lp = source.GetLeftPoint();
  Point rp = source.GetRightPoint();
  Point newlp, newrp;
  convert(&lp, &newlp);
  convert(&rp, &newrp);
  HalfSegment result(source);
  result.Set(source.IsLeftDomPoint(), newlp, newrp);
  return result;
}


