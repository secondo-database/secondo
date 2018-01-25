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

7.3 Implementation of ~Crossings~ methods

Defines and includes.

*/

#include "Crossings.h"
#include "Curve.h"

#include <vector>

using namespace std;

namespace salr {

/*
Implementation of constructor

*/
  Crossings::Crossings(double x1, double y1, double x2, double y2) :
      xlo(x1), ylo(y1), xhi(x2), yhi(y2) {
    limit = 0;
  }

/*
Implementation of access methods.

*/
  double Crossings::getXLo() {
    return xlo;
  }

  double Crossings::getYLo() {
    return ylo;
  }

  double Crossings::getXHi() {
    return xhi;
  }

  double Crossings::getYHi() {
    return yhi;
  }

  bool Crossings::isEmpty() {
    return (limit == 0);
  }

/*
Finds the number of intersections between all curves inside the vector and
 the rectangle defined by xlo, ylo, xhi and yhi.

*/
  bool Crossings::findCrossings(vector<Curve*> *curves,
                           double xlo, double ylo,
                           double xhi, double yhi) {
    Crossings *cross = new Crossings(xlo, ylo, xhi, yhi);
    for(unsigned int i = 0; i < curves->size(); i++) {
      if(curves->at(i)->accumulateCrossings(cross)) {
        delete cross;
        return true;
      }
    }
    bool result = !cross->isEmpty();
    delete cross;
    return result;
  }

/*
Checks if this ~Crossings~ is inside the range defined by ystart and yend.

*/
  bool Crossings::covers(double ystart, double yend) {
    return (limit == 2 && yranges[0] <= ystart && yranges[1] >= yend);
  }

/*
Adds a new pair of yrange to this ~Crossings~.

*/
  void Crossings::record(double ystart, double yend, int direction) {
    if (ystart >= yend) {
      return;
    }
    int from = 0;
    // Quickly jump over all pairs that are completely "above"
    while (from < limit && ystart > yranges[from+1]) {
      from += 2;
    }
    int to = from;
    while (from < limit) {
      double yrlo = yranges[from++];
      double yrhi = yranges[from++];
      if (yend < yrlo) {
        // Quickly handle insertion of the new range
        yranges[to++] = ystart;
        yranges[to++] = yend;
        ystart = yrlo;
        yend = yrhi;
        continue;
      }
      // The ranges overlap - sort, collapse, insert, iterate
      double yll, ylh, yhl, yhh;
      if (ystart < yrlo) {
        yll = ystart;
        ylh = yrlo;
      } else {
        yll = yrlo;
        ylh = ystart;
      }
      if (yend < yrhi) {
        yhl = yend;
        yhh = yrhi;
      } else {
        yhl = yrhi;
        yhh = yend;
      }
      if (ylh == yhl) {
        ystart = yll;
        yend = yhh;
      } else {
        if (ylh > yhl) {
          ystart = yhl;
          yhl = ylh;
          ylh = ystart;
        }
        if (yll != ylh) {
          yranges[to++] = yll;
          yranges[to++] = ylh;
        }
        ystart = yhl;
        yend = yhh;
      }
      if (ystart >= yend) {
        break;
      }
    }
    if (to < from && from < limit) {
      if(((int)yranges.max_size()) < to+(limit-from)){
        yranges.resize(to+(limit-from));
      }
      for(int i = 0; i < limit-from; i++) {
        yranges[to+i] = yranges[from+i];
      }
    }
    to += (limit-from);
    if (ystart < yend) {
      if (to >= ((int)yranges.max_size())) {
        yranges.resize(to);
      }
      yranges[to++] = ystart;
      yranges[to++] = yend;
    }
    limit = to;
  }

}
