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

[1] Header file of the Crossings

October, 2017 Torsten Weidmann

[TOC]

1 Overview

This header file defines the class Crossings.

2 Defines and includes

*/

#ifndef SECONDO_CROSSINGS_H
#define SECONDO_CROSSINGS_H

#include <vector>

namespace salr {

  class Curve;

  class Crossings {
  public:

    Crossings(double x1, double y1, double x2, double y2);

    double getXLo();
    double getYLo();
    double getXHi();
    double getYHi();

    bool isEmpty();

    static bool findCrossings(std::vector<Curve*> *curves,
                            double xlo, double ylo,
                            double xhi, double yhi);

    bool covers(double ystart, double yend);

    void record(double ystart, double yend, int direction);

  private:
    int limit;
    std::vector<double> yranges;
    double xlo, ylo, xhi, yhi;
  };

}

#endif //SECONDO_CROSSINGS_H
