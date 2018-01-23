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

3 Region2

3.1 Overview

This file defines the class ~Region2~ and its methods.

Defines and includes.

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

/*
Forward declarations and typedef.

*/
  typedef RegionT<DbArray>Region;
  typedef PointsT<DbArray>Points;

/*
2.2 Class ~Region2~

Can be used as an alternative to ~Region~ from ~SpatialAlgebra~ for some use
 cases.

*/
  class Region2 : public Attribute {
  public:

    friend class Line2;

/*
Declaration of constructors.

*/
    inline Region2() {};
    Region2(const Line2 &line);
    Region2(const Region2 &other);
    Region2(const Region &r);
/*
Declaration of destructor.

*/
    ~Region2();

/*
Declaration of system methods.

*/
    static const std::string BasicType();
    static const bool checkType(const ListExpr type);
    int NumOfFLOBs() const;
    Flob *GetFLOB(const int i);
    int Compare(const Attribute *arg) const;
    bool Adjacent(const Attribute *arg) const;
    size_t Sizeof() const;
    size_t HashValue() const;
    void CopyFrom(const Attribute *arg);
    Attribute *Clone() const;
    static ListExpr Property();
    static bool CheckKind(ListExpr type, ListExpr &errorInfo);
    bool ReadFrom(ListExpr LE, const ListExpr typeInfo);
    ListExpr ToListExpr(ListExpr typeInfo) const;

/*
Declaration of custom methods.

*/
    RectangleBB* getBounds();
    bool intersects(RectangleBB *bbox);
    Region2* union1(Region2 *rhs);
    Region2* minus1(Region2 *rhs);
    Region2* intersection1(Region2 *rhs);
    Region2* xor1(Region2 *rhs);
    void updateCurves();

  private:
/*
Declaration of fields.

  * ~curves~: Stores all segements in a vector containing pointer to ~Curve~.
  * ~coords~: Stores all coordinates that make up this region.
  * ~pointTypes~: Stores the types of segments.

*/
    std::vector<Curve*> curves;

    DbArray<double> coords;
    DbArray<int> pointTypes;

/*
Declaration of private methods.

*/
    void createCurves();
    void updateFLOBs();
    void nextSegment(int offset, int pointType, double *result) const;
    void appendCoord(double x);
    void appendType(int x);
    int getPointType(int i) const;
    double getCoord(int i) const;
  };

}

#endif //SECONDO_REGION_H
