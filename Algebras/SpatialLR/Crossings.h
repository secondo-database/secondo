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

7 Crossings

7.1 Overview

This file defines the class ~Crossings~ and its methods.

Defines and includes.

*/

#ifndef SECONDO_CROSSINGS_H
#define SECONDO_CROSSINGS_H

#include <vector>

namespace salr {

/*
Forward declaration.

*/
  class Curve;

/*
7.2 Class ~Crossings~

Used to find intersections between segments and store information about them.

*/
  class Crossings {
  public:

/*
Declaration of constructor.

*/
    Crossings(double x1, double y1, double x2, double y2);

/*
Declaration of custom methods.

*/
    double getXLo();
    double getYLo();
    double getXHi();
    double getYHi();
    bool isEmpty();
    static Crossings findCrossings(std::vector<Curve*> *curves,
                            double xlo, double ylo,
                            double xhi, double yhi);

    bool covers(double ystart, double yend);
    void record(double ystart, double yend, int direction);

  private:
/*
Fields used to store information about the crossings found so far by this
 instance.

*/
    int limit;
    std::vector<double> yranges;
    double xlo, ylo, xhi, yhi;
  };

}

#endif //SECONDO_CROSSINGS_H
