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

#include "Curve.h"
#include "RectangleBB.h"
#include "Crossings.h"

#include <algorithm>
#include <cmath>
#include <string>
#include <sstream>
#include <iostream>
#include <stdexcept>

using namespace std;

namespace salr {

  void Curve::insertMove(vector<Curve*> *curves, double x, double y) {
    curves->push_back(new MoveCurve(x, y));
  }

  void Curve::insertLine(vector<Curve*> *curves, double x0, double y0,
                                      double x1, double y1) {
    if (y0<y1) {
      curves->push_back(new LineCurve(x0, y0, x1, y1, INCREASING));
    } else if (y0 > y1) {
      curves->push_back(new LineCurve(x1, y1, x0, y0, DECREASING));
    } else {
      // Do not add horizontal lines
    }
  }

void Curve::copyCurves(const vector<Curve *> *source, vector<Curve *> *target) {
  for (unsigned int i = 0; i < source->size(); i++) {
    Curve *sc = source->at(i);
    Curve *tc;
    switch (sc->getOrder()) {
      case SEG_MOVETO:
        tc = new MoveCurve(sc->getX0(), sc->getY0());
        break;
      case SEG_LINETO:
        tc = new LineCurve(sc->getX0(), sc->getY0(),
                           sc->getX1(), sc->getY1(),
                           sc->getDirection());
        break;
    }
    target->push_back(tc);
  }
}


int Curve::rectCrossingsForLine(int crossings,
                                double rxmin, double rymin,
                                double rxmax, double rymax,
                                double x0, double y0,
                                double x1, double y1) {
  if (y0 >= rymax && y1 >= rymax) return crossings;
  if (y0 <= rymin && y1 <= rymin) return crossings;
  if (x0 <= rxmin && x1 <= rxmin) return crossings;
  if (x0 >= rxmax && x1 >= rxmax) {
    // Line is entirely to the right of the rect
    // and the vertical ranges of the two overlap by a non-empty amount
    // Thus, this line segment is partially in the "right-shadow"
    // Path may have done a complete crossing
    // Or path may have entered or exited the right-shadow
    if (y0 < y1) {
      // y-increasing line segment...
      // We know that y0 < rymax and y1 > rymin
      if (y0 <= rymin) crossings++;
      if (y1 >= rymax) crossings++;
    } else if (y1 < y0) {
      // y-decreasing line segment...
      // We know that y1 < rymax and y0 > rymin
      if (y1 <= rymin) crossings--;
      if (y0 >= rymax) crossings--;
    }
    return crossings;
  }
  // Remaining case:
  // Both x and y ranges overlap by a non-empty amount
  // First do trivial INTERSECTS rejection of the cases
  // where one of the endpoints is inside the rectangle.
  if ((x0 > rxmin && x0 < rxmax && y0 > rymin && y0 < rymax) ||
      (x1 > rxmin && x1 < rxmax && y1 > rymin && y1 < rymax)) {
    return RECT_INTERSECTS;
  }
  // Otherwise calculate the y intercepts and see where
  // they fall with respect to the rectangle
  double xi0 = x0;
  if (y0 < rymin) {
    xi0 += ((rymin - y0) * (x1 - x0) / (y1 - y0));
  } else if (y0 > rymax) {
    xi0 += ((rymax - y0) * (x1 - x0) / (y1 - y0));
  }
  double xi1 = x1;
  if (y1 < rymin) {
    xi1 += ((rymin - y1) * (x0 - x1) / (y0 - y1));
  } else if (y1 > rymax) {
    xi1 += ((rymax - y1) * (x0 - x1) / (y0 - y1));
  }
  if (xi0 <= rxmin && xi1 <= rxmin) return crossings;
  if (xi0 >= rxmax && xi1 >= rxmax) {
    if (y0 < y1) {
      // y-increasing line segment...
      // We know that y0 < rymax and y1 > rymin
      if (y0 <= rymin) crossings++;
      if (y1 >= rymax) crossings++;
    } else if (y1 < y0) {
      // y-decreasing line segment...
      // We know that y1 < rymax and y0 > rymin
      if (y1 <= rymin) crossings--;
      if (y0 >= rymax) crossings--;
    }
    return crossings;
  }
  return RECT_INTERSECTS;
}

int Curve::rectCrossingsForQuad(int crossings,
                                double rxmin, double rymin,
                                double rxmax, double rymax,
                                double x0, double y0,
                                double xc, double yc,
                                double x1, double y1,
                                int level) {
  if (y0 >= rymax && yc >= rymax && y1 >= rymax) return crossings;
  if (y0 <= rymin && yc <= rymin && y1 <= rymin) return crossings;
  if (x0 <= rxmin && xc <= rxmin && x1 <= rxmin) return crossings;
  if (x0 >= rxmax && xc >= rxmax && x1 >= rxmax) {
    // Quad is entirely to the right of the rect
    // and the vertical range of the 3 Y coordinates of the quad
    // overlaps the vertical range of the rect by a non-empty amount
    // We now judge the crossings solely based on the line segment
    // connecting the endpoints of the quad.
    // Note that we may have 0, 1, or 2 crossings as the control
    // point may be causing the Y range intersection while the
    // two endpoints are entirely above or below.
    if (y0 < y1) {
      // y-increasing line segment...
      if (y0 <= rymin && y1 > rymin) crossings++;
      if (y0 < rymax && y1 >= rymax) crossings++;
    } else if (y1 < y0) {
      // y-decreasing line segment...
      if (y1 <= rymin && y0 > rymin) crossings--;
      if (y1 < rymax && y0 >= rymax) crossings--;
    }
    return crossings;
  }
  // The intersection of ranges is more complicated
  // First do trivial INTERSECTS rejection of the cases
  // where one of the endpoints is inside the rectangle.
  if ((x0 < rxmax && x0 > rxmin && y0 < rymax && y0 > rymin) ||
      (x1 < rxmax && x1 > rxmin && y1 < rymax && y1 > rymin)) {
    return RECT_INTERSECTS;
  }
  // Otherwise, subdivide and look for one of the cases above.
  // double precision only has 52 bits of mantissa
  if (level > 52) {
    return rectCrossingsForLine(crossings,
                                rxmin, rymin, rxmax, rymax,
                                x0, y0, x1, y1);
  }
  double x0c = (x0 + xc) / 2;
  double y0c = (y0 + yc) / 2;
  double xc1 = (xc + x1) / 2;
  double yc1 = (yc + y1) / 2;
  xc = (x0c + xc1) / 2;
  yc = (y0c + yc1) / 2;
  if (xc != xc || yc != yc) {
    // [xy]c are NaN if any of [xy]0c or [xy]c1 are NaN
    // [xy]0c or [xy]c1 are NaN if any of [xy][0c1] are NaN
    // These values are also NaN if opposing infinities are added
    return 0;
  }
  crossings = rectCrossingsForQuad(crossings,
                                   rxmin, rymin, rxmax, rymax,
                                   x0, y0, x0c, y0c, xc, yc,
                                   level + 1);
  if (crossings != RECT_INTERSECTS) {
    crossings = rectCrossingsForQuad(crossings,
                                     rxmin, rymin, rxmax, rymax,
                                     xc, yc, xc1, yc1, x1, y1,
                                     level + 1);
  }
  return crossings;
}

int Curve::getDirection() {
  return direction;
}

Curve *Curve::getWithDirection(int direction) {
  return (this->direction == direction ? this : getReversedCurve());
}

int Curve::orderof(double x1, double x2) {
  if (x1 < x2) {
    return -1;
  }
  if (x1 > x2) {
    return 1;
  }
  return 0;
}

double Curve::refineTforY(double t0, double yt0, double y0) {
  double t1 = 1;
  while (true) {
    double th = (t0 + t1) / 2;
    if (th == t0 || th == t1) {
      return t1;
    }
    double y = YforT(th);
    if (y < y0) {
      t0 = th;
      yt0 = y;
    } else if (y > y0) {
      t1 = th;
    } else {
      return t1;
    }
  }
}

bool Curve::fairlyClose(double v1, double v2) {
  return (abs(v1 - v2) <
          max(abs(v1), abs(v2)) * 1E-10);
}

bool Curve::findIntersect(Curve *that, double *yrange, double ymin,
                          int slevel, int tlevel,
                          double s0, double xs0, double ys0,
                          double s1, double xs1, double ys1,
                          double t0, double xt0, double yt0,
                          double t1, double xt1, double yt1) {
  if (ys0 > yt1 || yt0 > ys1) {
    return false;
  }
  if (min(xs0, xs1) > max(xt0, xt1) ||
      max(xs0, xs1) < min(xt0, xt1)) {
    return false;
  }
  // Bounding boxes intersect - back off the larger of
  // the two subcurves by half until they stop intersecting
  // (or until they get small enough to switch to a more
  //  intensive algorithm).
  if (s1 - s0 > TMIN) {
    double s = (s0 + s1) / 2;
    double xs = this->XforT(s);
    double ys = this->YforT(s);
    if (s == s0 || s == s1) {
      throw runtime_error("no s progress!");
    }
    if (t1 - t0 > TMIN) {
      double t = (t0 + t1) / 2;
      double xt = that->XforT(t);
      double yt = that->YforT(t);
      if (t == t0 || t == t1) {
        throw runtime_error("no t progress!");
      }
      if (ys >= yt0 && yt >= ys0) {
        if (findIntersect(that, yrange, ymin, slevel + 1, tlevel + 1,
                          s0, xs0, ys0, s, xs, ys,
                          t0, xt0, yt0, t, xt, yt)) {
          return true;
        }
      }
      if (ys >= yt) {
        if (findIntersect(that, yrange, ymin, slevel + 1, tlevel + 1,
                          s0, xs0, ys0, s, xs, ys,
                          t, xt, yt, t1, xt1, yt1)) {
          return true;
        }
      }
      if (yt >= ys) {
        if (findIntersect(that, yrange, ymin, slevel + 1, tlevel + 1,
                          s, xs, ys, s1, xs1, ys1,
                          t0, xt0, yt0, t, xt, yt)) {
          return true;
        }
      }
      if (ys1 >= yt && yt1 >= ys) {
        if (findIntersect(that, yrange, ymin, slevel + 1, tlevel + 1,
                          s, xs, ys, s1, xs1, ys1,
                          t, xt, yt, t1, xt1, yt1)) {
          return true;
        }
      }
    } else {
      if (ys >= yt0) {
        if (findIntersect(that, yrange, ymin, slevel + 1, tlevel,
                          s0, xs0, ys0, s, xs, ys,
                          t0, xt0, yt0, t1, xt1, yt1)) {
          return true;
        }
      }
      if (yt1 >= ys) {
        if (findIntersect(that, yrange, ymin, slevel + 1, tlevel,
                          s, xs, ys, s1, xs1, ys1,
                          t0, xt0, yt0, t1, xt1, yt1)) {
          return true;
        }
      }
    }
  } else if (t1 - t0 > TMIN) {
    double t = (t0 + t1) / 2;
    double xt = that->XforT(t);
    double yt = that->YforT(t);
    if (t == t0 || t == t1) {
      throw runtime_error("no t progress!");
    }
    if (yt >= ys0) {
      if (findIntersect(that, yrange, ymin, slevel, tlevel + 1,
                        s0, xs0, ys0, s1, xs1, ys1,
                        t0, xt0, yt0, t, xt, yt)) {
        return true;
      }
    }
    if (ys1 >= yt) {
      if (findIntersect(that, yrange, ymin, slevel, tlevel + 1,
                        s0, xs0, ys0, s1, xs1, ys1,
                        t, xt, yt, t1, xt1, yt1)) {
        return true;
      }
    }
  } else {
    // No more subdivisions
    double xlk = xs1 - xs0;
    double ylk = ys1 - ys0;
    double xnm = xt1 - xt0;
    double ynm = yt1 - yt0;
    double xmk = xt0 - xs0;
    double ymk = yt0 - ys0;
    double det = xnm * ylk - ynm * xlk;
    if (det != 0) {
      double detinv = 1 / det;
      double s = (xnm * ymk - ynm * xmk) * detinv;
      double t = (xlk * ymk - ylk * xmk) * detinv;
      if (s >= 0 && s <= 1 && t >= 0 && t <= 1) {
        s = s0 + s * (s1 - s0);
        t = t0 + t * (t1 - t0);
        if (s < 0 || s > 1 || t < 0 || t > 1) {
          cerr << "Error subdiv" << endl;
        }
        double y = (this->YforT(s) + that->YforT(t)) / 2;
        if (y <= yrange[1] && y > yrange[0]) {
          yrange[1] = y;
          return true;
        }
      }
    }
  }
  return false;
}

int Curve::compareTo(Curve *that, double *yrange) {
  double y0 = yrange[0];
  double y1 = yrange[1];
  y1 = min(min(y1, this->getYBot()), that->getYBot());
  if (y1 <= yrange[0]) {
    stringstream sstm;
    sstm << "backstepping from " << yrange[0] << " to " << y1;
    throw runtime_error(sstm.str());
  }
  yrange[1] = y1;
  if (this->getXMax() <= that->getXMin()) {
    if (this->getXMin() == that->getXMax()) {
      return 0;
    }
    return -1;
  }
  if (this->getXMin() >= that->getXMax()) {
    return 1;
  }
  // Parameter s for thi(s) curve and t for tha(t) curve
  // [st]0 = parameters for top of current section of interest
  // [st]1 = parameters for bottom of valid range
  // [st]h = parameters for hypothesis point
  // [d][xy]s = valuations of thi(s) curve at sh
  // [d][xy]t = valuations of tha(t) curve at th
  double s0 = this->TforY(y0);
  double ys0 = this->YforT(s0);
  if (ys0 < y0) {
    s0 = refineTforY(s0, ys0, y0);
    ys0 = this->YforT(s0);
  }
  double s1 = this->TforY(y1);
  if (this->YforT(s1) < y0) {
    s1 = refineTforY(s1, this->YforT(s1), y0);
  }
  double t0 = that->TforY(y0);
  double yt0 = that->YforT(t0);
  if (yt0 < y0) {
    t0 = that->refineTforY(t0, yt0, y0);
    yt0 = that->YforT(t0);
  }
  double t1 = that->TforY(y1);
  if (that->YforT(t1) < y0) {
    t1 = that->refineTforY(t1, that->YforT(t1), y0);
  }
  double xs0 = this->XforT(s0);
  double xt0 = that->XforT(t0);
  double scale = max(abs(y0), abs(y1));
  double ymin = max(scale * 1E-14, 1E-300);
  if (fairlyClose(xs0, xt0)) {
    double bump = ymin;
    double maxbump = min(ymin * 1E13, (y1 - y0) * .1);
    double y = y0 + bump;
    while (y <= y1) {
      if (fairlyClose(this->XforY(y), that->XforY(y))) {
        if ((bump *= 2) > maxbump) {
          bump = maxbump;
        }
      } else {
        y -= bump;
        while (true) {
          bump /= 2;
          double newy = y + bump;
          if (newy <= y) {
            break;
          }
          if (fairlyClose(this->XforY(newy), that->XforY(newy))) {
            y = newy;
          }
        }
        break;
      }
      y += bump;
    }
    if (y > y0) {
      if (y < y1) {
        yrange[1] = y;
      }
      return 0;
    }
  }
  while (s0 < s1 && t0 < t1) {
    double sh = this->nextVertical(s0, s1);
    double xsh = this->XforT(sh);
    double ysh = this->YforT(sh);
    double th = that->nextVertical(t0, t1);
    double xth = that->XforT(th);
    double yth = that->YforT(th);
    try {
      if (findIntersect(that, yrange, ymin, 0, 0,
                        s0, xs0, ys0, sh, xsh, ysh,
                        t0, xt0, yt0, th, xth, yth)) {
        break;
      }
    } catch (...) {
      cerr << "Error: " << endl;
      cerr << "y range was " << yrange[0] << "=>" << yrange[1] << endl;
      cerr << "s y range is " << ys0 << "=>" << ysh << endl;
      cerr << "t y range is " << yt0 << "=>" << yth << endl;
      cerr << "ymin is " << ymin << endl;
      return 0;
    }
    if (ysh < yth) {
      if (ysh > yrange[0]) {
        if (ysh < yrange[1]) {
          yrange[1] = ysh;
        }
        break;
      }
      s0 = sh;
      xs0 = xsh;
      ys0 = ysh;
    } else {
      if (yth > yrange[0]) {
        if (yth < yrange[1]) {
          yrange[1] = yth;
        }
        break;
      }
      t0 = th;
      xt0 = xth;
      yt0 = yth;
    }
  }
  double ymid = (yrange[0] + yrange[1]) / 2;
  return orderof(this->XforY(ymid), that->XforY(ymid));
}

MoveCurve::MoveCurve(double x, double y) :
  Curve(INCREASING),
  x(x), y(y) {
}

int MoveCurve::getOrder() {
  return SEG_MOVETO;
}

double MoveCurve::getXTop() {
  return x;
}

double MoveCurve::getYTop() {
  return y;
}

double MoveCurve::getXBot() {
  return x;
}

double MoveCurve::getYBot() {
  return y;
}

double MoveCurve::getXMin() {
  return x;
}

double MoveCurve::getXMax() {
  return x;
}

double MoveCurve::getX0() {
  return x;
}

double MoveCurve::getY0() {
  return y;
}

double MoveCurve::getX1() {
  return x;
}

double MoveCurve::getY1() {
  return y;
}

double MoveCurve::XforY(double y) {
  return y;
}

double MoveCurve::TforY(double y) {
  return 0;
}

double MoveCurve::XforT(double t) {
  return x;
}

double MoveCurve::YforT(double t) {
  return y;
}

double MoveCurve::dXforT(double t, int deriv) {
  return 0;
}

double MoveCurve::dYforT(double t, int deriv) {
  return 0;
}

double MoveCurve::nextVertical(double t0, double t1) {
  return t1;
}

Curve *MoveCurve::getSubCurve(double ystart, double yend, int dir) {
  return this;
}

Curve *MoveCurve::getReversedCurve() {
  return this;
}

void MoveCurve::enlarge(RectangleBB *r) {
  r->add(x, y);
}

bool MoveCurve::accumulateCrossings(Crossings *c) {
  return (x > c->getXLo() &&
          x < c->getXHi() &&
          y > c->getYLo() &&
          y < c->getYHi());
}

int MoveCurve::getSegment(double *coords) {
  coords[0] = x;
  coords[1] = y;
  return Curve::SEG_MOVETO;
}

LineCurve::LineCurve(double x0, double y0, double x1, double y1,
                     int direction) :
  Curve(direction),
  x0(x0), y0(y0), x1(x1), y1(y1) {
  if (x0 < x1) {
    this->xmin = x0;
    this->xmax = x1;
  } else {
    this->xmin = x1;
    this->xmax = x0;
  }
}

int LineCurve::getOrder() {
  return SEG_LINETO;
}

double LineCurve::getXTop() {
  return x0;
}

double LineCurve::getYTop() {
  return y0;
}

double LineCurve::getXBot() {
  return x1;
}

double LineCurve::getYBot() {
  return y1;
}

double LineCurve::getXMin() {
  return xmin;
}

double LineCurve::getXMax() {
  return xmax;
}

double LineCurve::getX0() {
  return (direction == INCREASING) ? x0 : x1;
}

double LineCurve::getY0() {
  return (direction == INCREASING) ? y0 : y1;
}

double LineCurve::getX1() {
  return (direction == DECREASING) ? x0 : x1;
}

double LineCurve::getY1() {
  return (direction == DECREASING) ? y0 : y1;
}

double LineCurve::XforY(double y) {
  if (x0 == x1 || y <= y0) {
    return x0;
  }
  if (y >= y1) {
    return x1;
  }
  // assert(y0 != y1);  No horizontal lines...
  return (x0 + (y - y0) * (x1 - x0) / (y1 - y0));
}

double LineCurve::TforY(double y) {
  if (y <= y0) {
    return 0;
  }
  if (y >= y1) {
    return 1;
  }
  return (y - y0) / (y1 - y0);
}

double LineCurve::XforT(double t) {
  return x0 + t * (x1 - x0);
}

double LineCurve::YforT(double t) {
  return y0 + t * (y1 - y0);
}

double LineCurve::dXforT(double t, int deriv) {
  switch (deriv) {
    case 0:
      return x0 + t * (x1 - x0);
    case 1:
      return (x1 - x0);
    default:
      return 0;
  }
}

double LineCurve::dYforT(double t, int deriv) {
  switch (deriv) {
    case 0:
      return y0 + t * (y1 - y0);
    case 1:
      return (y1 - y0);
    default:
      return 0;
  }
}

double LineCurve::nextVertical(double t0, double t1) {
  return t1;
}

Curve *LineCurve::getSubCurve(double ystart, double yend, int dir) {
  if (ystart == y0 && yend == y1) {
    return getWithDirection(dir);
  }
  if (x0 == x1) {
    return new LineCurve(x0, ystart, x1, yend, dir);
  }
  double num = x0 - x1;
  double denom = y0 - y1;
  double xstart = (x0 + (ystart - y0) * num / denom);
  double xend = (x0 + (yend - y0) * num / denom);
  return new LineCurve(xstart, ystart, xend, yend, dir);
}

Curve *LineCurve::getReversedCurve() {
  return new LineCurve(x0, y0, x1, y1, -direction);
}

int LineCurve::compareTo(Curve *other, double *yrange) {
  if (other == NULL || other->getOrder() != 1) {
    return Curve::compareTo(other, yrange);
  }
  LineCurve *c1 = dynamic_cast<LineCurve *>(other);
  if (c1 == NULL) {
    return Curve::compareTo(other, yrange);
  }
  if (yrange[1] <= yrange[0]) {
    throw invalid_argument("yrange already screwed up...");
  }
  yrange[1] = min(min(yrange[1], y1), c1->y1);
  if (yrange[1] <= yrange[0]) {
    stringstream sstm;
    sstm << "backstepping from " << yrange[0] << " to " << y1;
    throw runtime_error(sstm.str());
  }
  if (xmax <= c1->xmin) {
    return (xmin == c1->xmax) ? 0 : -1;
  }
  if (xmin >= c1->xmax) {
    return 1;
  }
  double dxa = x1 - x0;
  double dya = y1 - y0;
  double dxb = c1->x1 - c1->x0;
  double dyb = c1->y1 - c1->y0;
  double denom = dxb * dya - dxa * dyb;
  double y;
  if (denom != 0) {
    double num = ((x0 - c1->x0) * dya * dyb
                  - y0 * dxa * dyb
                  + c1->y0 * dxb * dya);
    y = num / denom;
    if (y <= yrange[0]) {
      // intersection is above us
      // Use bottom-most common y for comparison
      y = min(y1, c1->y1);
    } else {
      // intersection is below the top of our range
      if (y < yrange[1]) {
        // If intersection is in our range, adjust valid range
        yrange[1] = y;
      }
      // Use top-most common y for comparison
      y = max(y0, c1->y0);
    }
  } else {
    // lines are parallel, choose any common y for comparison
    // Note - prefer an endpoint for speed of calculating the X
    // (see shortcuts in Order1.XforY())
    y = max(y0, c1->y0);
  }
  return orderof(XforY(y), c1->XforY(y));
}

void LineCurve::enlarge(RectangleBB *r) {
  r->add(x0, y0);
  r->add(x1, y1);
}

bool LineCurve::accumulateCrossings(Crossings *c) {
  double xlo = c->getXLo();
  double ylo = c->getYLo();
  double xhi = c->getXHi();
  double yhi = c->getYHi();
  if (xmin >= xhi) {
    return false;
  }
  double xstart, ystart, xend, yend;
  if (y0 < ylo) {
    if (y1 <= ylo) {
      return false;
    }
    ystart = ylo;
    xstart = XforY(ylo);
  } else {
    if (y0 >= yhi) {
      return false;
    }
    ystart = y0;
    xstart = x0;
  }
  if (y1 > yhi) {
    yend = yhi;
    xend = XforY(yhi);
  } else {
    yend = y1;
    xend = x1;
  }
  if (xstart >= xhi && xend >= xhi) {
    return false;
  }
  if (xstart > xlo || xend > xlo) {
    return true;
  }
  c->record(ystart, yend, direction);
  return false;
}

int LineCurve::getSegment(double *coords) {
  if (direction == INCREASING) {
    coords[0] = x1;
    coords[1] = y1;
  } else {
    coords[0] = x0;
    coords[1] = y0;
  }
  return Curve::SEG_LINETO;
}

}
