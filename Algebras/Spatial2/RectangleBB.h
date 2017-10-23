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

[1] Header file of the RectangleBB

October, 2017 Torsten Weidmann

[TOC]

1 Overview

This header file defines the class RectangleBB.

2 Defines and includes

*/
#ifndef SECONDO_RECTANGLEBB_H
#define SECONDO_RECTANGLEBB_H

#include "Attribute.h"
#include "GenericTC.h"
#include "NestedList.h"

#include <string>

namespace salr {

  class RectangleBB : public Attribute {
  public:
    double x, y, width, height;

    inline RectangleBB() {};

    RectangleBB(int init);

    RectangleBB(double x, double y, double width, double height);

    ~RectangleBB();

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
                                "double double double double",
                                "(1.0 1.0 1.0 1.0)");
    }

    static bool CheckKind(ListExpr type, ListExpr &errorInfo);

    bool ReadFrom(ListExpr LE, const ListExpr typeInfo);

    ListExpr ToListExpr(ListExpr typeInfo) const;

    // Custom methods
    bool contains(double x1, double y1);
    bool isEmpty();
    void add(double newx, double newy);
    void setRect(double x1, double y1, double w, double h);
    bool intersects(double x1, double y1, double w, double h);

  };
}

#endif //SECONDO_RECTANGLEBB_H
