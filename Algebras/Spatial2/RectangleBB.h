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

4 RectangleBB

4.1 Overview

This file defines the class ~RectangleBB~ and its methods.

Defines and includes.

*/
#ifndef SECONDO_RECTANGLEBB_H
#define SECONDO_RECTANGLEBB_H

#include "Attribute.h"
#include "NestedList.h"

#include <string>

namespace salr {

/*
4.2 Class ~RectangleBB~

*/
  class RectangleBB : public Attribute {
  public:

/*
Declaration of fields.

  * ~x~: x-coordinate of the point most to the left.
  * ~y~: y-coordinate of the point most to the bottom.
  * ~width~: Width of this rectangle.
  * ~height~: Height of this rectangle.

*/
    double x, y, width, height;

/*
Declaration of constructors.

*/
    inline RectangleBB() {};
    RectangleBB(int init);
    RectangleBB(double x, double y, double width, double height);
/*
Declaration of destructor.

*/
    ~RectangleBB();

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
    bool contains(double x1, double y1);
    bool isEmpty();
    void add(double newx, double newy);
    void setRect(double x1, double y1, double w, double h);
    bool intersects(double x1, double y1, double w, double h);

  };
}

#endif //SECONDO_RECTANGLEBB_H
