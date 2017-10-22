/*
----
This file is part of SECONDO.

Copyright (C) 2016, University in Hagen,
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


*/

#include "RectangleBB.h"

#include <algorithm>

using namespace std;

namespace salr {

  RectangleBB::RectangleBB(double x, double y, double width, double height) :
    x(x), y(y), width(width), height(height) {
  }

  bool RectangleBB::contains(double x1, double y1) {
    return (x1 >= x && y1 >= y &&
            x1 < x + width && y1 < y + height);
  }

  bool RectangleBB::isEmpty() {
    return (width <= 0.0) || (height <= 0.0);
  }

  void RectangleBB::setRect(double x1, double y1, double w, double h) {
    x = x1;
    y = y1;
    width = w;
    height = h;
  }

  void RectangleBB::add(double newx, double newy) {
    double x1 = min(x, newx);
    double x2 = max(x + width, newx);
    double y1 = min(y, newy);
    double y2 = max(y + height, newy);
    setRect(x1, y1, x2 - x1, y2 - y1);
  }

  bool RectangleBB::intersects(double x1, double y1, double w, double h) {
    if (isEmpty() || w <= 0 || h <= 0) {
      return false;
    }
    return (x1 + w > x && y1 + h > y &&
            x1 < x + width && y1 < y + height);
  }

}
