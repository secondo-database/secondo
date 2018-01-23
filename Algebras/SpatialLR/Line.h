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

2 Line2

2.1 Overview

This file defines the class ~Line2~ and its methods.

Defines and includes.

*/

#ifndef SECONDO_LINE_H
#define SECONDO_LINE_H

#include "../../Tools/Flob/DbArray.h"
#include "Attribute.h"
#include "GenericTC.h"
#include "LineT.h"

namespace salr {

/*
Forward declarations and typedef.

*/
  class RectangleBB;
  class Region2;
  typedef LineT<DbArray>Line;

/*
2.2 Class ~Line2~

Can be used as an alternative to ~Line~ from ~SpatialAlgebra~ for some use
 cases.

*/
  class Line2 : public Attribute {
  public:

    friend class Region2;

/*
Declaration of constructors.

*/
    inline Line2() {};

    Line2(const Line2 &other);
    Line2(int initialCapacity);
    Line2(const Line &l);
    Line2(const Region2 &r);
/*
Declaration of destructor.

*/
    ~Line2();

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
    void moveTo(double x, double y);
    void lineTo(double x, double y);
    void quadTo(double x1, double y1, double x2, double y2);
    void closeLine();
    RectangleBB* getBounds();
    bool intersects(RectangleBB *bbox);
    void nextSegment(int offset, int pointType, double *result) const;
    Line* toLine();
    double getCoord(int i) const;
    int getPointType(int i) const;

  private:
/*
Declaration of fields.

  * ~coords~: Stores all coordinates that make up this line.
  * ~pointTypes~: Stores the types of line segments.
  * ~hasQuads~: Indicates if this line contains a quad segment.

*/
    DbArray<double> coords;
    DbArray<int> pointTypes;

    bool hasQuads;

/*
Declaration of private methods.

*/
    void appendCoord(double x);
    void appendType(int x);
    void hasInitialMove();
    int rectCrossings(double rxmin, double rymin, double rxmax, double rymax);
  };

}

#endif //SECONDO_LINE_H
