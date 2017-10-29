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

[1] Header file of the Region

September, 20017 Torsten Weidmann

[TOC]

1 Overview

This header file defines the class Region.

2 Defines and includes

*/

#ifndef SECONDO_REGION_H
#define SECONDO_REGION_H

#include "../../Tools/Flob/DbArray.h"
#include "Attribute.h"
#include "GenericTC.h"
#include "Curve.h"
#include "Line.h"
#include "RectangleBB.h"
#include "RegionT.h"
#include "PointsT.h"

#include <vector>

namespace salr {

  typedef RegionT<DbArray>Region;
  typedef PointsT<DbArray>Points;

  class Region2 : public Attribute {
  public:

    friend class Line2;

    inline Region2() {};
    Region2(const Line2 &line);
    Region2(const Region2 &other);
    Region2(const Region &r);
    ~Region2();

    static const std::string BasicType();
    static const bool checkType(const ListExpr type);

    // Attribute methods
    int NumOfFLOBs() const;
    Flob *GetFLOB(const int i);
    int Compare(const Attribute *arg) const;
    bool Adjacent(const Attribute *arg) const;
    size_t Sizeof() const;
    size_t HashValue() const;
    void CopyFrom(const Attribute *arg);
    Attribute *Clone() const;

    static ListExpr Property() {
      return gentc::GenProperty("-> DATA",
                                BasicType(),
                                "int int DbArry<int> DbArry<real>",
                                "((0 1) (1.5 2.5 2.0 3.0))");
    }

    static bool CheckKind(ListExpr type, ListExpr &errorInfo);
    bool ReadFrom(ListExpr LE, const ListExpr typeInfo);
    ListExpr ToListExpr(ListExpr typeInfo) const;

    RectangleBB* getBounds();
    bool intersects(RectangleBB *bbox);
    Region2* union1(Region2 *rhs);
    Region2* minus1(Region2 *rhs);
    Region2* intersects1(Region2 *rhs);
    Region2* xor1(Region2 *rhs);

  private:
    std::vector<Curve*> curves;

    DbArray<double> coords;
    DbArray<int> pointTypes;

    void createCurves();
    void updateFLOBs();
    void nextSegment(int offset, int pointType, double *result) const;

    void appendCoord(double x) {
      coords.Append(x);
    }

    void appendType(int x) {
      pointTypes.Append(x);
    }

    int getPointType(int i) const {
      int pointType;
      pointTypes.Get(i, &pointType);
      return pointType;
    }

    double getCoord(int i) const {
      double coord;
      coords.Get(i, &coord);
      return coord;
    }
  };

}

#endif //SECONDO_REGION_H
